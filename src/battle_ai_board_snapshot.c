#include "global.h"
#include "battle.h"
#include "battle_ai_board_sim.h"
#include "battle_controllers.h"
#include "battle_dynamax.h"
#include "battle_gimmick.h"
#include "battle_terastal.h"
#include "battle_util.h"
#include "event_data.h"
#include "item.h"
#include "move.h"
#include "pokemon.h"
#include "test_runner.h"
#include "constants/abilities.h"
#include "constants/battle_ai.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/moves.h"

static u8 AiSim_CompactTimer(u16 timer)
{
    return min(timer, (u16)UINT8_MAX);
}

static u8 AiSim_GetRosterIndex(enum BattleTrainer trainer, u32 partyIndex)
{
    if (trainer >= MAX_BATTLE_TRAINERS || partyIndex >= PARTY_SIZE)
        return AI_SIM_ROSTER_NONE;

    return trainer * PARTY_SIZE + partyIndex;
}

static u8 AiSim_GetDynamaxHpPercent(struct Pokemon *mon)
{
    if (HasShedinjaHPHandling(GetMonData(mon, MON_DATA_SPECIES)))
        return 100;

    return 150 + 5 * min(GetMonData(mon, MON_DATA_DYNAMAX_LEVEL), (u32)MAX_DYNAMAX_LEVEL);
}

static bool32 AiSim_HasFullKnowledge(enum BattlerId planningBattler, enum BattleTrainer trainer)
{
    if (GetBattlerSide(planningBattler) == (trainer & BIT_SIDE))
        return TRUE;

    return (gAiThinkingStruct->aiFlags[planningBattler] & AI_FLAG_OMNISCIENT) != 0;
}

static bool32 AiSim_KnowsOpponentPartySpecies(enum BattlerId planningBattler)
{
    return (gAiThinkingStruct->aiFlags[planningBattler] & (AI_FLAG_OMNISCIENT | AI_FLAG_KNOW_OPPONENT_PARTY)) != 0;
}

static void AiSim_FillPartyCombatProfile(struct AiSimCombatProfile *profile, struct Pokemon *mon, enum Ability ability, bool32 abilityKnown)
{
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);

    profile->species = species;
    profile->maxHp = GetMonData(mon, MON_DATA_MAX_HP);
    profile->attack = GetMonData(mon, MON_DATA_ATK);
    profile->defense = GetMonData(mon, MON_DATA_DEF);
    profile->speed = GetMonData(mon, MON_DATA_SPEED);
    profile->spAttack = GetMonData(mon, MON_DATA_SPATK);
    profile->spDefense = GetMonData(mon, MON_DATA_SPDEF);
    profile->ability = abilityKnown ? ability : ABILITY_NONE;
    profile->types[0] = GetSpeciesType(species, 0);
    profile->types[1] = GetSpeciesType(species, 1);
    // SpeciesInfo has two defensive type slots.  BattlePokemon reserves the
    // third slot for temporary effects and initializes it to TYPE_MYSTERY;
    // asking GetSpeciesType for slot 2 would read the following catch-rate
    // byte as a phantom type.
    profile->types[2] = TYPE_MYSTERY;
    profile->flags = AI_SIM_PROFILE_VALID | AI_SIM_PROFILE_TYPES_KNOWN;
    if (abilityKnown)
        profile->flags |= AI_SIM_PROFILE_ABILITY_KNOWN;
}

static enum Species AiSim_GetPartyFormChangeTarget(struct Pokemon *mon,
                                                   enum FormChanges method)
{
    struct FormChangeContext context =
    {
        .method = method,
        .currentSpecies = GetMonData(mon, MON_DATA_SPECIES),
        .heldItem = GetMonData(mon, MON_DATA_HELD_ITEM),
        .ability = GetMonAbility(mon),
        .status = GetMonData(mon, MON_DATA_STATUS),
        .hp = GetMonData(mon, MON_DATA_HP),
        .maxHP = GetMonData(mon, MON_DATA_MAX_HP),
        .gmaxFactor = GetMonData(mon, MON_DATA_GIGANTAMAX_FACTOR),
        .teraType = GetMonData(mon, MON_DATA_TERA_TYPE),
        .level = GetMonData(mon, MON_DATA_LEVEL),
    };

    for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
        context.moves[moveIndex] = GetMonData(mon, MON_DATA_MOVE1 + moveIndex);
    return GetFormChangeTargetSpecies_Internal(context);
}

static bool32 AiSim_FillProspectiveTransformation(struct AiSimMonTemplate *template,
                                                  struct Pokemon *mon,
                                                  enum Item currentItem,
                                                  enum Gimmick gimmick)
{
    struct Pokemon transformedMon = *mon;
    enum Species currentSpecies;
    enum Species targetSpecies;
    enum Ability targetAbility;
    s32 savedLevelUpHp;

    SetMonData(&transformedMon, MON_DATA_HELD_ITEM, &currentItem);
    currentSpecies = GetMonData(&transformedMon, MON_DATA_SPECIES);
    if (gimmick == GIMMICK_MEGA)
    {
        // ActivateMegaEvolution tries the move route first (Rayquaza), then
        // the held-item route. Mirror that ordering on the immutable copy.
        targetSpecies = AiSim_GetPartyFormChangeTarget(
            &transformedMon, FORM_CHANGE_BATTLE_MEGA_EVOLUTION_MOVE);
        if (targetSpecies == currentSpecies)
            targetSpecies = AiSim_GetPartyFormChangeTarget(
                &transformedMon, FORM_CHANGE_BATTLE_MEGA_EVOLUTION_ITEM);
    }
    else if (gimmick == GIMMICK_ULTRA_BURST)
    {
        targetSpecies = AiSim_GetPartyFormChangeTarget(
            &transformedMon, FORM_CHANGE_BATTLE_ULTRA_BURST);
    }
    else
    {
        return FALSE;
    }
    if (targetSpecies == SPECIES_NONE || targetSpecies == currentSpecies)
        return FALSE;

    SetMonData(&transformedMon, MON_DATA_SPECIES, &targetSpecies);
    // CalculateMonStats writes this battle-script scratch field in addition
    // to the supplied Pokemon. Restore it so snapshot capture remains
    // observational even while deriving exact IV/EV/nature-dependent stats.
    savedLevelUpHp = gBattleScripting.levelUpHP;
    CalculateMonStats(&transformedMon);
    gBattleScripting.levelUpHP = savedLevelUpHp;
    targetAbility = GetMonAbility(&transformedMon);
    // The battle-test harness reapplies an explicitly forced ability whenever
    // RecalcBattlerStats copies a new form into BattlePokemon.  Mirror that
    // test-only runtime state in prospective profiles; production parties
    // return ABILITY_NONE here and retain the transformed species' ability.
    if (TestRunner_Battle_GetForcedAbility(template->trainer,
                                           template->partyIndex) != ABILITY_NONE)
    {
        targetAbility = TestRunner_Battle_GetForcedAbility(template->trainer,
                                                            template->partyIndex);
    }
    AiSim_FillPartyCombatProfile(&template->transformed,
                                 &transformedMon,
                                 targetAbility,
                                 TRUE);
    template->transformationGimmick = gimmick;
    return TRUE;
}

static bool32 AiSim_HasFutureGimmickAccess(enum BattlerId planningBattler,
                                           enum BattleTrainer trainer,
                                           enum Gimmick gimmick)
{
#if TESTING
    // The test runner deliberately bypasses bag/key-item and orb-charge
    // checks in each live CanActivate function.
    (void)planningBattler;
    (void)trainer;
    (void)gimmick;
    return TRUE;
#else
    // NPC trainers do not consume the local player's key-item access. The
    // current joint runtime excludes multi battles, so B_TRAINER_PLAYER is
    // the only player-controlled party that needs the local access checks.
    if (trainer != B_TRAINER_PLAYER)
        return TRUE;
    if (!HasGimmickAccess(planningBattler, gimmick))
        return FALSE;
    if (gimmick == GIMMICK_DYNAMAX && !IsDynamaxBattleEnabled(planningBattler))
        return FALSE;
    if (gimmick == GIMMICK_TERA
     && !HasGimmickAccessOverride(GIMMICK_TERA)
     && !(B_FLAG_TERA_ORB_NO_COST != 0 && FlagGet(B_FLAG_TERA_ORB_NO_COST))
     && !(B_FLAG_TERA_ORB_CHARGED != 0 && FlagGet(B_FLAG_TERA_ORB_CHARGED)))
        return FALSE;
    return TRUE;
#endif
}

static bool32 AiSim_IsTrainerMonIntendedForGimmick(enum BattleTrainer trainer,
                                                    u32 partyIndex,
                                                    enum Gimmick gimmick)
{
#if TESTING
    return TestRunner_Battle_GetChosenGimmick(trainer, partyIndex) == gimmick;
#else
    if (trainer == B_TRAINER_PLAYER)
        return TRUE;
    if (gimmick == GIMMICK_TERA)
        return (gBattleStruct->opponentMonCanTera & (1u << partyIndex)) != 0;
    if (gimmick == GIMMICK_DYNAMAX)
        return (gBattleStruct->opponentMonCanDynamax & (1u << partyIndex)) != 0;
    return TRUE;
#endif
}

static bool32 AiSim_CanEventuallyUseZMove(enum BattlerId planningBattler,
                                          enum BattleTrainer trainer,
                                          const struct AiSimPartyState *party)
{
    return !(gBattleTypeFlags & (BATTLE_TYPE_SAFARI | BATTLE_TYPE_CATCH_TUTORIAL))
        && party->item < ITEMS_COUNT
        && GetItemHoldEffect(party->item) == HOLD_EFFECT_Z_CRYSTAL
        && AiSim_HasFutureGimmickAccess(planningBattler, trainer, GIMMICK_Z_MOVE);
}

static void AiSim_CaptureProspectiveGimmicks(enum BattlerId planningBattler,
                                             enum BattleTrainer trainer,
                                             u32 partyIndex,
                                             struct Pokemon *mon,
                                             struct AiSimMonTemplate *template,
                                             struct AiSimPartyState *party)
{
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);
    enum HoldEffect holdEffect = party->item < ITEMS_COUNT
                               ? GetItemHoldEffect(party->item)
                               : HOLD_EFFECT_NONE;
    enum Gimmick activeGimmick = party->activeGimmick;
    bool32 canUseZMove;

    if (!(template->flags & AI_SIM_MON_PROFILE_KNOWN)
     || !(template->flags & AI_SIM_MON_MOVES_KNOWN)
     || !(party->flags & AI_SIM_PARTY_ITEM_KNOWN))
        return;

    canUseZMove = AiSim_CanEventuallyUseZMove(planningBattler, trainer, party);
    if (activeGimmick != GIMMICK_NONE)
    {
        template->eligibleGimmicks |= 1u << activeGimmick;
        // Ultra Necrozma remains eligible to spend its Z resource after the
        // higher-priority Ultra Burst resource has been activated.
        if (activeGimmick == GIMMICK_ULTRA_BURST && canUseZMove)
            template->eligibleGimmicks |= 1u << GIMMICK_Z_MOVE;
        return;
    }

    if (holdEffect != HOLD_EFFECT_Z_CRYSTAL
     && AiSim_HasFutureGimmickAccess(planningBattler, trainer, GIMMICK_MEGA)
     && AiSim_FillProspectiveTransformation(template, mon, party->item,
                                            GIMMICK_MEGA))
    {
        template->eligibleGimmicks |= 1u << GIMMICK_MEGA;
    }
    else if (holdEffect == HOLD_EFFECT_Z_CRYSTAL
          && AiSim_HasFutureGimmickAccess(planningBattler, trainer, GIMMICK_ULTRA_BURST)
          && AiSim_FillProspectiveTransformation(template, mon, party->item,
                                                 GIMMICK_ULTRA_BURST))
    {
        template->eligibleGimmicks |= 1u << GIMMICK_ULTRA_BURST;
    }

    if (canUseZMove)
        template->eligibleGimmicks |= 1u << GIMMICK_Z_MOVE;

    if (!(gBattleTypeFlags & BATTLE_TYPE_FIRST_BATTLE && (trainer & BIT_SIDE) == B_SIDE_OPPONENT)
     && GET_BASE_SPECIES_ID(species) != SPECIES_ZACIAN
     && GET_BASE_SPECIES_ID(species) != SPECIES_ZAMAZENTA
     && GET_BASE_SPECIES_ID(species) != SPECIES_ETERNATUS
     && holdEffect != HOLD_EFFECT_Z_CRYSTAL
     && holdEffect != HOLD_EFFECT_MEGA_STONE
     && AiSim_HasFutureGimmickAccess(planningBattler, trainer, GIMMICK_DYNAMAX)
     && AiSim_IsTrainerMonIntendedForGimmick(trainer, partyIndex, GIMMICK_DYNAMAX))
        template->eligibleGimmicks |= 1u << GIMMICK_DYNAMAX;

    if (!(gBattleTypeFlags & BATTLE_TYPE_FIRST_BATTLE && (trainer & BIT_SIDE) == B_SIDE_OPPONENT)
     && holdEffect != HOLD_EFFECT_Z_CRYSTAL
     && holdEffect != HOLD_EFFECT_MEGA_STONE
     && AiSim_HasFutureGimmickAccess(planningBattler, trainer, GIMMICK_TERA)
     && AiSim_IsTrainerMonIntendedForGimmick(trainer, partyIndex, GIMMICK_TERA))
        template->eligibleGimmicks |= 1u << GIMMICK_TERA;
}

static void AiSim_FillActiveCombatProfile(struct AiSimCombatProfile *profile, enum BattlerId battler, enum Ability ability, bool32 abilityKnown)
{
    const struct BattlePokemon *mon = &gBattleMons[battler];

    profile->species = mon->species;
    profile->maxHp = mon->maxHP;
    profile->attack = mon->attack;
    profile->defense = mon->defense;
    profile->speed = mon->speed;
    profile->spAttack = mon->spAttack;
    profile->spDefense = mon->spDefense;
    profile->ability = abilityKnown ? ability : ABILITY_NONE;
    memcpy(profile->types, mon->types, sizeof(profile->types));
    profile->flags = AI_SIM_PROFILE_VALID | AI_SIM_PROFILE_TYPES_KNOWN;
    if (abilityKnown)
        profile->flags |= AI_SIM_PROFILE_ABILITY_KNOWN;
}

static enum Ability AiSim_GetRecordedAbility(enum BattlerId battler, enum BattleTrainer trainer, u32 partyIndex)
{
    if (gBattleMons[battler].volatiles.overwrittenAbility != ABILITY_NONE)
        return gBattleMons[battler].volatiles.overwrittenAbility;
    if (gBattleHistory->abilities[battler] != ABILITY_NONE)
        return gBattleHistory->abilities[battler];
    if (gAiPartyData->mons[trainer][partyIndex].ability != ABILITY_NONE)
        return gAiPartyData->mons[trainer][partyIndex].ability;

    return ABILITY_NONE;
}

static void AiSim_CopyKnownMoves(enum BattlerId planningBattler, enum BattleTrainer trainer, u32 partyIndex, struct AiSimMonTemplate *template)
{
    const struct AiPartyMon *knownMon = &gAiPartyData->mons[trainer][partyIndex];

    memcpy(template->moves, knownMon->moves, sizeof(template->moves));

    for (enum BattlerId battler = 0; battler < gBattlersCount; battler++)
    {
        if (GetBattlerTrainer(battler) != trainer || gBattlerPartyIndexes[battler] != partyIndex)
            continue;

        // Party knowledge survives a switch. Battle history can be sparse for
        // the currently occupied battler slot, so merge observed slots instead
        // of replacing already known slots with MOVE_NONE.
        for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
        {
            if (gBattleHistory->usedMoves[battler][moveIndex] != MOVE_NONE)
                template->moves[moveIndex] = gBattleHistory->usedMoves[battler][moveIndex];
        }
        if ((gAiThinkingStruct->aiFlags[planningBattler] & AI_FLAG_READ_PLAYER_MOVE)
         && gChosenActionByBattler[battler] == B_ACTION_USE_MOVE
         && gChosenMoveByBattler[battler] != MOVE_NONE
         && gChosenMoveByBattler[battler] != MOVE_UNAVAILABLE
         && gBattleStruct->chosenMovePositions[battler] < MAX_MON_MOVES)
            template->moves[gBattleStruct->chosenMovePositions[battler]] = gChosenMoveByBattler[battler];
        break;
    }
}

static void AiSim_CaptureRosterMon(enum BattlerId planningBattler, enum BattleTrainer trainer, u32 partyIndex, struct AiSimContext *context, struct AiSimBoard *board)
{
    struct Pokemon *mon = &gParties[trainer][partyIndex];
    const struct AiPartyMon *knownMon = &gAiPartyData->mons[trainer][partyIndex];
    struct AiSimMonTemplate *template = &context->mons[AiSim_GetRosterIndex(trainer, partyIndex)];
    struct AiSimPartyState *party = &board->party[AiSim_GetRosterIndex(trainer, partyIndex)];
    enum Species actualSpecies = GetMonData(mon, MON_DATA_SPECIES);
    bool32 fullKnowledge = AiSim_HasFullKnowledge(planningBattler, trainer);
    bool32 speciesKnown = fullKnowledge || AiSim_KnowsOpponentPartySpecies(planningBattler) || knownMon->species != SPECIES_NONE;

    template->trainer = trainer;
    template->partyIndex = partyIndex;
    template->teraType = TYPE_NONE;

    if (actualSpecies == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
        return;
    if (!speciesKnown)
    {
        context->flags |= AI_SIM_CONTEXT_PARTIAL_KNOWLEDGE;
        return;
    }

    template->flags |= AI_SIM_MON_PRESENT;
    party->activeGimmick = gBattleStruct->gimmick.activeGimmick[trainer][partyIndex];
    template->level = fullKnowledge ? GetMonData(mon, MON_DATA_LEVEL) : knownMon->level;
    template->normal.species = fullKnowledge ? actualSpecies : knownMon->species;
    template->normal.types[0] = GetSpeciesType(template->normal.species, 0);
    template->normal.types[1] = GetSpeciesType(template->normal.species, 1);
    template->normal.types[2] = TYPE_MYSTERY;
    template->normal.flags = AI_SIM_PROFILE_TYPES_KNOWN;

    if (fullKnowledge)
    {
        enum Ability ability = GetMonAbility(mon);
        u32 forcedAbility = TestRunner_Battle_GetForcedAbility(trainer, partyIndex);

        // Battle tests apply forced abilities when a party member is sent
        // out. Capture the same override while it is still benched so switch
        // lines simulate the board the test runner will actually create.
        if (forcedAbility != ABILITY_NONE)
            ability = forcedAbility;

        AiSim_FillPartyCombatProfile(&template->normal, mon, ability, TRUE);
        template->dynamaxHpPercent = AiSim_GetDynamaxHpPercent(mon);
        for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
        {
            template->moves[moveIndex] = GetMonData(mon, MON_DATA_MOVE1 + moveIndex);
            party->pp[moveIndex] = GetMonData(mon, MON_DATA_PP1 + moveIndex);
        }
        template->teraType = GetMonData(mon, MON_DATA_TERA_TYPE);
        template->flags |= AI_SIM_MON_MOVES_KNOWN | AI_SIM_MON_PROFILE_KNOWN;
        party->hp = GetMonData(mon, MON_DATA_HP);
        party->item = GetMonData(mon, MON_DATA_HELD_ITEM);
        party->status1 = GetMonData(mon, MON_DATA_STATUS);
        party->flags |= AI_SIM_PARTY_ITEM_KNOWN | AI_SIM_PARTY_STATUS_KNOWN;
    }
    else
    {
        AiSim_CopyKnownMoves(planningBattler, trainer, partyIndex, template);
        party->status1 = knownMon->status;
        if (knownMon->wasSentInBattle || knownMon->isFainted)
            party->flags |= AI_SIM_PARTY_STATUS_KNOWN;
        if (knownMon->isFainted)
            party->hp = 0;
        if (knownMon->item != ITEM_NONE)
        {
            party->item = knownMon->item;
            party->flags |= AI_SIM_PARTY_ITEM_KNOWN;
        }
        context->flags |= AI_SIM_CONTEXT_PARTIAL_KNOWLEDGE;
    }

    if (gBattleStruct->partyState[trainer][partyIndex].usedHeldItem != ITEM_NONE)
    {
        party->flags |= AI_SIM_PARTY_ITEM_CONSUMED;
        party->item = ITEM_NONE;
    }
    if (gBattleStruct->partyState[trainer][partyIndex].isKnockedOff)
    {
        party->flags |= AI_SIM_PARTY_ITEM_REMOVED;
        party->item = ITEM_NONE;
    }

    if (fullKnowledge)
    {
        // A Mega/Ultra form persists in the party after switching out. Its
        // current party stats therefore belong in the transformed profile,
        // which ResetActiveForSwitch selects when the mon returns.
        if (party->activeGimmick == GIMMICK_MEGA
         || party->activeGimmick == GIMMICK_ULTRA_BURST)
        {
            template->transformed = template->normal;
            memset(&template->normal, 0, sizeof(template->normal));
            template->transformationGimmick = party->activeGimmick;
        }
        AiSim_CaptureProspectiveGimmicks(planningBattler, trainer, partyIndex,
                                         mon, template, party);
    }
}

static void AiSim_CaptureActiveBattler(enum BattlerId planningBattler, enum BattlerId battler, struct AiSimContext *context, struct AiSimBoard *board)
{
    enum BattleTrainer trainer = GetBattlerTrainer(battler);
    u32 partyIndex = gBattlerPartyIndexes[battler];
    u8 rosterIndex = AiSim_GetRosterIndex(trainer, partyIndex);
    struct AiSimMonTemplate *template;
    struct AiSimPartyState *party;
    struct AiSimActiveState *active = &board->active[battler];
    enum Gimmick activeGimmick;
    enum Ability knownAbility;
    bool32 fullKnowledge;
    bool32 abilityKnown;

    active->rosterIndex = AI_SIM_ROSTER_NONE;
    if (rosterIndex == AI_SIM_ROSTER_NONE || (gAbsentBattlerFlags & (1u << battler)))
        return;

    template = &context->mons[rosterIndex];
    party = &board->party[rosterIndex];
    fullKnowledge = AiSim_HasFullKnowledge(planningBattler, trainer);
    knownAbility = fullKnowledge ? gBattleMons[battler].ability : AiSim_GetRecordedAbility(battler, trainer, partyIndex);
    abilityKnown = knownAbility != ABILITY_NONE;
    activeGimmick = GetActiveGimmick(battler);

    template->flags |= AI_SIM_MON_PRESENT;
    template->level = gBattleMons[battler].level;
    template->trainer = trainer;
    template->partyIndex = partyIndex;
    if (fullKnowledge && (activeGimmick == GIMMICK_MEGA || activeGimmick == GIMMICK_ULTRA_BURST))
    {
        AiSim_FillActiveCombatProfile(&template->transformed, battler, knownAbility, abilityKnown);
        template->transformationGimmick = activeGimmick;
        active->flags |= AI_SIM_ACTIVE_TRANSFORMED;
    }
    else if (fullKnowledge)
    {
        AiSim_FillActiveCombatProfile(&template->normal, battler, knownAbility, abilityKnown);
    }
    else
    {
        struct AiSimCombatProfile *profile = &template->normal;

        // Exact live stats and PP are hidden information. Keep only visible
        // identity/type data plus abilities that battle history has actually
        // revealed; partial snapshots are deliberately fail-closed by the
        // pure simulator.
        if (activeGimmick == GIMMICK_MEGA || activeGimmick == GIMMICK_ULTRA_BURST)
        {
            profile = &template->transformed;
            template->transformationGimmick = activeGimmick;
            active->flags |= AI_SIM_ACTIVE_TRANSFORMED;
        }
        memset(profile, 0, sizeof(*profile));
        profile->species = gBattleMons[battler].species;
        memcpy(profile->types, gBattleMons[battler].types, sizeof(profile->types));
        profile->ability = abilityKnown ? knownAbility : ABILITY_NONE;
        profile->flags = AI_SIM_PROFILE_TYPES_KNOWN;
        if (abilityKnown)
            profile->flags |= AI_SIM_PROFILE_ABILITY_KNOWN;
    }
    if (activeGimmick == GIMMICK_DYNAMAX)
    {
        if (fullKnowledge)
        {
            template->dynamaxHpPercent = AiSim_GetDynamaxHpPercent(&gParties[trainer][partyIndex]);
            template->normal.maxHp = GetNonDynamaxMaxHP(battler);
        }
        else
        {
            template->dynamaxHpPercent = 0;
            template->normal.maxHp = 0;
        }
    }
    if (fullKnowledge)
        template->flags |= AI_SIM_MON_PROFILE_KNOWN;

    if (fullKnowledge)
    {
        memcpy(template->moves, gBattleMons[battler].moves, sizeof(template->moves));
        template->flags |= AI_SIM_MON_MOVES_KNOWN;
    }
    else
    {
        AiSim_CopyKnownMoves(planningBattler, trainer, partyIndex, template);
        context->flags |= AI_SIM_CONTEXT_PARTIAL_KNOWLEDGE;
    }
    for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
    {
        if (template->moves[moveIndex] != MOVE_NONE)
        {
            if (fullKnowledge)
                party->pp[moveIndex] = gBattleMons[battler].pp[moveIndex];
            else
                party->pp[moveIndex] = GetMovePP(template->moves[moveIndex]);
        }
    }

    party->hp = gBattleMons[battler].hp;
    party->status1 = gBattleMons[battler].status1;
    party->flags |= AI_SIM_PARTY_STATUS_KNOWN;
    if (fullKnowledge || (party->flags & AI_SIM_PARTY_ITEM_KNOWN))
    {
        party->item = gBattleMons[battler].item;
        party->flags |= AI_SIM_PARTY_ITEM_KNOWN;
    }
    party->activeGimmick = activeGimmick;

    active->rosterIndex = rosterIndex;
    active->lastMove = gLastMoves[battler] == MOVE_UNAVAILABLE ? MOVE_NONE : gLastMoves[battler];
    if (gBattleMons[battler].volatiles.multipleTurns)
    {
        active->chargingMove = gChosenMoveByBattler[battler];
        if (active->chargingMove == MOVE_NONE || active->chargingMove == MOVE_UNAVAILABLE)
            active->chargingMove = active->lastMove;
        if (GetMoveEffect(active->chargingMove) == EFFECT_GEOMANCY)
            active->volatileFlags |= AI_SIM_VOLATILE_GEOMANCY_CHARGING;
    }
    if (gProtectStructs[battler].protected != PROTECT_NONE)
        active->volatileFlags |= AI_SIM_VOLATILE_PROTECTED;
    if (gProtectStructs[battler].protected == PROTECT_MAX_GUARD)
        active->volatileFlags |= AI_SIM_VOLATILE_MAX_GUARD;
    if (gBattleMons[battler].volatiles.flinched)
        active->volatileFlags |= AI_SIM_VOLATILE_FLINCHED;

    memcpy(active->statStages, gBattleMons[battler].statStages, sizeof(active->statStages));
    active->consecutiveMoveUses = min(gBattleMons[battler].volatiles.consecutiveMoveUses, (u32)UINT8_MAX);
    active->dynamaxTurns = AiSim_CompactTimer(gBattleStruct->dynamax.dynamaxTurns[battler]);
    active->firstTurn = gBattleStruct->battlerState[battler].isFirstTurn;
    active->choiceMoveSlot = MAX_MON_MOVES;
    for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
    {
        if (gBattleMons[battler].volatiles.disabledMove != MOVE_NONE
         && gBattleMons[battler].volatiles.disabledMove != MOVE_UNAVAILABLE
         && gBattleMons[battler].volatiles.disabledMove == gBattleMons[battler].moves[moveIndex])
            active->disabledMoveMask |= 1u << moveIndex;
        if ((fullKnowledge || (party->flags & AI_SIM_PARTY_ITEM_KNOWN))
         && gBattleStruct->choicedMove[battler] != MOVE_NONE
         && gBattleStruct->choicedMove[battler] != MOVE_UNAVAILABLE
         && gBattleStruct->choicedMove[battler] == gBattleMons[battler].moves[moveIndex])
            active->choiceMoveSlot = moveIndex;
    }
    if (gBattleMons[battler].volatiles.perishSong)
        active->perishTimer = gBattleMons[battler].volatiles.perishSongTimer;

    if (activeGimmick == GIMMICK_DYNAMAX)
        active->flags |= AI_SIM_ACTIVE_DYNAMAX;
    else if (activeGimmick == GIMMICK_TERA)
        active->flags |= AI_SIM_ACTIVE_TERA;

    if (activeGimmick != GIMMICK_NONE)
        template->eligibleGimmicks |= 1u << activeGimmick;
    if (gBattleStruct->gimmick.usableGimmick[battler] != GIMMICK_NONE)
        template->eligibleGimmicks |= 1u << gBattleStruct->gimmick.usableGimmick[battler];
    if (fullKnowledge || activeGimmick == GIMMICK_TERA || IsGimmickSelected(battler, GIMMICK_TERA))
        template->teraType = GetBattlerTeraType(battler);

    context->activeMask |= 1u << battler;
    board->activeMask |= 1u << battler;
}

static void AiSim_CaptureSide(enum BattleSide side, struct AiSimBoard *board)
{
    struct AiSimSideState *state = &board->sides[side];

    state->statuses = gSideStatuses[side];
    state->tailwindTimer = AiSim_CompactTimer(gSideTimers[side].tailwindTimer);
    state->reflectTimer = AiSim_CompactTimer(gSideTimers[side].reflectTimer);
    state->lightScreenTimer = AiSim_CompactTimer(gSideTimers[side].lightscreenTimer);
    state->auroraVeilTimer = AiSim_CompactTimer(gSideTimers[side].auroraVeilTimer);
    state->safeguardTimer = AiSim_CompactTimer(gSideTimers[side].safeguardTimer);
    state->mistTimer = AiSim_CompactTimer(gSideTimers[side].mistTimer);
    state->followMeTarget = gSideTimers[side].followmeTarget;
    state->followMeTimer = gSideTimers[side].followmeTimer;
    state->spikes = gSideTimers[side].spikesAmount;
    state->toxicSpikes = gSideTimers[side].toxicSpikesAmount;
    for (enum Hazards hazard = HAZARDS_SPIKES; hazard < HAZARDS_MAX_COUNT; hazard++)
    {
        if (IsHazardOnSide(side, hazard))
            state->hazardsMask |= 1u << hazard;
    }

    for (enum BattlerId battler = 0; battler < gBattlersCount; battler++)
    {
        if (GetBattlerSide(battler) != side)
            continue;
        if (gProtectStructs[battler].protected == PROTECT_QUICK_GUARD)
            state->flags |= AI_SIM_SIDE_QUICK_GUARD;
        if (gProtectStructs[battler].protected == PROTECT_WIDE_GUARD)
            state->flags |= AI_SIM_SIDE_WIDE_GUARD;
    }
}

static bool32 AiSim_HasUnsupportedVolatile(enum BattlerId battler)
{
    struct Volatiles state = gBattleMons[battler].volatiles;
    const struct Volatiles zero = {0};
    enum Move chargingMove = gChosenMoveByBattler[battler];

    // These values are represented explicitly in AiSimActiveState or do not
    // affect a future transition once their visible result has been captured.
    state.flinched = FALSE;
    state.substitute = FALSE; // Reported separately for a precise reason flag.
    state.consecutiveMoveUses = 0;
    state.disabledMove = MOVE_NONE;
    state.usedMoves = 0;
    // The live counter advances after ordinary moves even when the battler is
    // not holding Metronome. That hold effect is outside the supported item
    // envelope, so the counter has no transition effect on a supported board.
    state.metronomeItemCounter = 0;
    state.overwrittenAbility = ABILITY_NONE;
    state.weatherAbilityDone = FALSE;
    state.terrainAbilityDone = FALSE;
    state.unnerveActivated = FALSE;
    state.traceActivated = FALSE;
    // Command selection runs only after switch-in item processing. A stat
    // drop such as opening Intimidate can leave this bookkeeping bit set when
    // no Eject Pack activates, but it has no pending transition by this point.
    state.tryEjectPack = FALSE;

    if (state.multipleTurns
     && (chargingMove == MOVE_NONE || chargingMove == MOVE_UNAVAILABLE
      || GetMoveEffect(chargingMove) != EFFECT_GEOMANCY))
        return TRUE;
    state.multipleTurns = FALSE;

    return memcmp(&state, &zero, sizeof(state)) != 0;
}

static void AiSim_CaptureUnsupportedState(struct AiSimContext *context, const struct AiSimBoard *board)
{
    const u32 supportedSideStatuses = SIDE_STATUS_SCREEN_ANY
                                    | SIDE_STATUS_SAFEGUARD
                                    | SIDE_STATUS_MIST
                                    | SIDE_STATUS_TAILWIND;
    const u32 supportedFieldStatuses = STATUS_FIELD_TRICK_ROOM
                                     | STATUS_FIELD_PSYCHIC_TERRAIN;

    for (enum BattleSide side = B_SIDE_PLAYER; side < NUM_BATTLE_SIDES; side++)
    {
        if (board->sides[side].followMeTimer != 0)
            context->flags |= AI_SIM_CONTEXT_REDIRECTION_STATE;
        if (board->sides[side].statuses & ~supportedSideStatuses)
            context->flags |= AI_SIM_CONTEXT_FIELD_STATE;
    }

    if (board->fieldStatuses & ~supportedFieldStatuses)
        context->flags |= AI_SIM_CONTEXT_FIELD_STATE;
    if (board->gravityTimer != 0 || board->magicRoomTimer != 0 || board->wonderRoomTimer != 0)
        context->flags |= AI_SIM_CONTEXT_FIELD_STATE;
    if (board->weather & B_WEATHER_DAMAGING_ANY)
        context->flags |= AI_SIM_CONTEXT_RESIDUAL_STATE;
    if (board->weather & ~(B_WEATHER_RAIN_NORMAL | B_WEATHER_SUN_NORMAL | B_WEATHER_DAMAGING_ANY))
        context->flags |= AI_SIM_CONTEXT_FIELD_STATE;

    for (enum BattlerId battler = 0; battler < gBattlersCount; battler++)
    {
        if (!(board->activeMask & (1u << battler)))
            continue;
        if (gBattleMons[battler].volatiles.substitute)
            context->flags |= AI_SIM_CONTEXT_SUBSTITUTE_STATE;
        if (board->party[board->active[battler].rosterIndex].status1
          & (STATUS1_DAMAGING & ~AI_SIM_MODELED_RESIDUAL_STATUSES))
            context->flags |= AI_SIM_CONTEXT_RESIDUAL_STATE;
        if (AiSim_HasUnsupportedVolatile(battler))
            context->flags |= AI_SIM_CONTEXT_VOLATILE_STATE;
    }
}

static void AiSim_CaptureGimmickOwnership(struct AiSimContext *context, struct AiSimBoard *board)
{
    for (enum BattlerId battler = 0; battler < gBattlersCount; battler++)
    {
        u8 shareMask = 0;
        enum BattleTrainer trainer = GetBattlerTrainer(battler);

        for (enum BattlerId other = 0; other < gBattlersCount; other++)
        {
            if (GetBattlerTrainer(other) == trainer)
                shareMask |= 1u << other;
        }
        context->gimmickShareMask[battler] = shareMask;

        for (enum Gimmick gimmick = GIMMICK_MEGA; gimmick < GIMMICKS_COUNT; gimmick++)
        {
            if (gBattleStruct->gimmick.activated[battler][gimmick])
                board->trainerGimmickUsed[trainer] |= 1u << gimmick;
        }
    }
}

bool32 AiSim_CaptureKnownBoard(enum BattlerId planningBattler, const struct AiLogicData *aiData, struct AiSimContext *context, struct AiSimBoard *board)
{
    if (planningBattler >= gBattlersCount
     || aiData == NULL
     || context == NULL
     || board == NULL
     || gAiThinkingStruct == NULL
     || gAiPartyData == NULL
     || gBattleHistory == NULL
     || !BattlerHasAi(planningBattler))
        return FALSE;

    memset(context, 0, sizeof(*context));
    memset(board, 0, sizeof(*board));

    context->battleTypeFlags = gBattleTypeFlags;
    context->battlersCount = gBattlersCount;
    if (gAiThinkingStruct->aiFlags[planningBattler] & AI_FLAG_OMNISCIENT)
        context->flags |= AI_SIM_CONTEXT_OMNISCIENT;

    for (enum BattleTrainer trainer = B_TRAINER_PLAYER; trainer < MAX_BATTLE_TRAINERS; trainer++)
    {
        if (!TrainerHasParty(trainer))
            continue;
        for (u32 partyIndex = 0; partyIndex < PARTY_SIZE; partyIndex++)
            AiSim_CaptureRosterMon(planningBattler, trainer, partyIndex, context, board);
    }

    for (enum BattlerId battler = 0; battler < gBattlersCount; battler++)
    {
        context->battlerTrainer[battler] = GetBattlerTrainer(battler);
        context->tieRank[battler] = battler;
        AiSim_CaptureActiveBattler(planningBattler, battler, context, board);
    }

    for (enum BattleSide side = B_SIDE_PLAYER; side < NUM_BATTLE_SIDES; side++)
        AiSim_CaptureSide(side, board);

    AiSim_CaptureGimmickOwnership(context, board);
    board->fieldStatuses = gFieldStatuses;
    memcpy(board->stellarBoostFlags, gBattleStruct->stellarBoostFlags, sizeof(board->stellarBoostFlags));
    board->weather = gBattleWeather;
    board->turn = gBattleTurnCounter;
    board->weatherTimer = gBattleStruct->weatherDuration;
    board->trickRoomTimer = AiSim_CompactTimer(gFieldTimers.trickRoomTimer);
    board->terrainTimer = AiSim_CompactTimer(gFieldTimers.terrainTimer);
    board->gravityTimer = AiSim_CompactTimer(gFieldTimers.gravityTimer);
    board->magicRoomTimer = AiSim_CompactTimer(gFieldTimers.magicRoomTimer);
    board->wonderRoomTimer = AiSim_CompactTimer(gFieldTimers.wonderRoomTimer);
    board->absentMask = gAbsentBattlerFlags;
    AiSim_CaptureUnsupportedState(context, board);

    return TRUE;
}
