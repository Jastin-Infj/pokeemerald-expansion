#include "global.h"
#include "battle.h"
#include "constants/battle_ai.h"
#include "battle_ai_main.h"
#include "battle_ai_switch.h"
#include "battle_ai_util.h"
#include "battle_util.h"
#include "battle_anim.h"
#include "battle_controllers.h"
#include "battle_gimmick.h"
#include "battle_main.h"
#include "battle_setup.h"
#include "data.h"
#include "item.h"
#include "party_menu.h"
#include "pokemon.h"
#include "random.h"
#include "util.h"
#include "constants/abilities.h"
#include "constants/item_effects.h"
#include "constants/battle_move_effects.h"
#include "constants/items.h"
#include "constants/moves.h"

#define AI_SWITCH_ROLL_14_OF_16_PERCENT 98
#define AI_SWITCH_ROLL_15_OF_16_PERCENT 99
#define AI_SWITCH_DAMAGE_ROLL_COUNT 16
#define AI_SWITCH_MAX_DAMAGE_ROLL_COMBINATIONS (AI_SWITCH_DAMAGE_ROLL_COUNT * AI_SWITCH_DAMAGE_ROLL_COUNT)

// this file's functions
struct IncomingHealInfo
{
    u16 healAmount:16;
    u16 wishCounter:8;
    u16 hasHealing:1;
    u16 healBeforeHazards:1;
    u16 healAfterHazards:1;
    u16 healEndOfTurn:1;
    u16 curesStatus:1;
};
struct KnownPlayerDamageRollSummary
{
    u32 minimum;
    u32 median;
    u32 roll14Of16;
    u32 roll15Of16;
    u32 maximum;
};
enum FocusedSwitchCandidateKind
{
    FOCUSED_SWITCH_CANDIDATE_NONE,
    FOCUSED_SWITCH_CANDIDATE_SAFE,
    FOCUSED_SWITCH_CANDIDATE_ROLL_SURVIVAL,
    FOCUSED_SWITCH_CANDIDATE_CUSHION,
    FOCUSED_SWITCH_CANDIDATE_SACRIFICE,
};
struct FocusedSwitchCandidate
{
    u32 monIndex;
    u32 score;
    enum FocusedSwitchCandidateKind kind;
};
static bool32 CanUseSuperEffectiveMoveAgainstOpponents(enum BattlerId battler, enum BattlerId opposingBattler);
static bool32 CanUseSuperEffectiveMoveAgainstOpponent(enum BattlerId battler, enum BattlerId opposingBattler);
static u32 GetSwitchinHazardsDamage(enum BattlerId battler);
static u32 GetSwitchinSingleUseItemHealing(enum BattlerId battler, enum BattlerId opposingBattler, s32 currentHP);
static bool32 AI_CanSwitchinAbilityTrapOpponent(enum Ability ability, enum BattlerId opposingBattler);
static uq4_12_t GetTypeMatchupAgainstTypes(enum BattlerId opposingBattler, enum Type defType1, enum Type defType2);
static enum Ability GetPartyMonAbilityForSwitchCalc(enum BattlerId battler, u32 monIndex, struct Pokemon *mon);
static uq4_12_t GetBattlerTypeMatchup(enum BattlerId opposingBattler, enum BattlerId battler);
static u32 GetSwitchinHitsToKO(s32 damageTaken, enum BattlerId battler, const struct IncomingHealInfo *healInfo, u32 originalHp);
static u32 EstimateDamageAtRollPercent(const struct SimulatedDamage *damage, u32 rollPercent);
static void GetDamageRollValues(const struct SimulatedDamage *damage, u32 *rollValues);
static void SortDamageRollSums(u32 *rollSums, u32 count);
static bool32 SetKnownPlayerDamageRollSummaryFromDistribution(const struct SimulatedDamage *damages, u32 damageCount, struct KnownPlayerDamageRollSummary *summary);
static void GetIncomingHealInfo(enum BattlerId battler, struct IncomingHealInfo *healInfo);
static u32 GetWishHealAmountForBattler(enum BattlerId battler);
static void SetBattlerStatusForSwitchin(enum BattlerId battler);
static void SetBattlerStatStagesForSwitchin(enum BattlerId battler, enum BattlerId opposingBattler, u32 fieldStatus);
static void SetBattlerHPChangeForSwitch(enum BattlerId battler, enum BattlerId opposingBattler);
static void SetBattlerVolatilesForSwitchin(enum BattlerId battler, u32 weather, u32 fieldStatus);
bool32 IsSwitchinTSpikesAffected(enum BattlerId battler);
static bool32 IsOpponentPhysicalAttacker(enum BattlerId battler, enum BattlerId opposingBattler);
static bool32 CanIntimidateLowerOpponentAtk(enum BattlerId battler, enum BattlerId opposingBattler);
static bool32 ShouldSwitchIfIntimidateBenefit(struct SwitchAiContext *switchContext);
static bool32 ShouldSwitchIfBoardControlBenefit(struct SwitchAiContext *switchContext);
static bool32 ShouldSwitchIfDoublePositionBad(struct SwitchAiContext *switchContext);
static bool32 ShouldSwitchIfPredictedTauntPunish(struct SwitchAiContext *switchContext);
static bool32 ShouldSwitchIfKnownSingleTargetKO(struct SwitchAiContext *switchContext);
static bool32 ShouldSwitchIfKnownFocusedSlotCollapse(struct SwitchAiContext *switchContext);
static bool32 ShouldSwitchIfReadChoiceRoleDone(struct SwitchAiContext *switchContext);
static bool32 ShouldStayInForKnownTeraSurvival(struct SwitchAiContext *switchContext);
static bool32 ShouldPreserveDoubleBattlerWithProtect(struct SwitchAiContext *switchContext);
static bool32 FindSoundproofSwitchinForReadPerishSong(struct SwitchAiContext *switchContext);
static bool32 FindSoundproofSwitchinForReadSoundPressure(struct SwitchAiContext *switchContext);
static bool32 DoesMostSuitableSwitchinBenefitFromWish(enum BattlerId battler);
static u32 GetSwitchinCandidate(u32 switchinCategory, enum BattlerId battler, int lastId, enum SwitchType switchType);

static enum Ability GetPartyMonAbilityForSwitchCalc(enum BattlerId battler, u32 monIndex, struct Pokemon *mon)
{
    enum Ability ability = GetMonAbility(mon);

#if TESTING
    if (gTestRunnerEnabled)
    {
        enum BattleTrainer trainer = !IsPartnerMonFromSameTrainer(battler) ? battler : GetBattlerSide(battler);
        u32 forcedAbility = TestRunner_Battle_GetForcedAbility(trainer, monIndex);
        if (forcedAbility != 0)
            ability = forcedAbility;
    }
#endif

    return ability;
}

static void InitializeSwitchinCandidate(enum BattlerId switchinBattler, u32 monIndex, struct Pokemon *mon)
{
    u32 storeCurrBattlerPartyIndex = gBattlerPartyIndexes[switchinBattler]; // Rage Fist fix
    PokemonToBattleMon(mon, &gBattleMons[switchinBattler]);
    gBattlerPartyIndexes[switchinBattler] = monIndex;
    CopyMonAbilityAndTypesToBattleMon(switchinBattler, mon);
    // Setup switchin battler data
    gAiThinkingStruct->saved[switchinBattler].saved = TRUE;
    SetBattlerAiData(switchinBattler, gAiLogicData);
    u32 switchinWeather = AI_GetSwitchinWeather(switchinBattler);
    u32 switchinFieldStatus = AI_GetSwitchinFieldStatus(switchinBattler);
    SetBattlerVolatilesForSwitchin(switchinBattler, switchinWeather, switchinFieldStatus);
    SetBattlerStatusForSwitchin(switchinBattler);
    gBattlerPartyIndexes[switchinBattler] = monIndex;
    gAiLogicData->switchInCalc = TRUE;

    for (enum BattlerId battlerIndex = 0; battlerIndex < gBattlersCount; battlerIndex++)
    {
        if (switchinBattler == battlerIndex || !IsBattlerAlive(battlerIndex))
            continue;
        SetBattlerStatStagesForSwitchin(switchinBattler, battlerIndex, switchinFieldStatus);
        SetBattlerHPChangeForSwitch(switchinBattler, battlerIndex);
        CalcBattlerAiMovesData(gAiLogicData, switchinBattler, battlerIndex, switchinWeather, switchinFieldStatus);
        CalcBattlerAiMovesData(gAiLogicData, battlerIndex, switchinBattler, switchinWeather, switchinFieldStatus);
    }

    gAiLogicData->switchInCalc = FALSE;
    gBattlerPartyIndexes[switchinBattler] = storeCurrBattlerPartyIndex;
    gAiThinkingStruct->saved[switchinBattler].saved = FALSE;
}

static u32 GetWishHealAmountForBattler(enum BattlerId battler)
{
    u32 wishHeal = 0;

    if (gBattleStruct->wish[battler].counter == 0)
        return wishHeal;

    if (B_WISH_HP_SOURCE >= GEN_5)
    {
        wishHeal = GetMonData(&GetBattlerParty(battler)[gBattleStruct->wish[battler].partyId], MON_DATA_MAX_HP) / 2;
    }
    else
    {
        wishHeal = GetNonDynamaxMaxHP(battler) / 2;
    }

    return wishHeal;
}

static void GetIncomingHealInfo(enum BattlerId battler, struct IncomingHealInfo *healInfo)
{
    memset(healInfo, 0, sizeof(*healInfo));

    // Healing Wish / Lunar Dance heal to full and clear status before hazards
    if (gBattleStruct->battlerState[battler].storedHealingWish)
    {
        healInfo->hasHealing = TRUE;
        healInfo->healBeforeHazards = TRUE;
        healInfo->curesStatus = TRUE;
    }
    if (gBattleStruct->battlerState[battler].storedLunarDance)
    {
        healInfo->hasHealing = TRUE;
        healInfo->healBeforeHazards = TRUE;
        healInfo->curesStatus = TRUE;
    }

    // Z-Parting Shot / Z-Memento heal after hazards on switch-in
    if (gBattleStruct->zmove.healReplacement)
    {
        healInfo->hasHealing = TRUE;
        healInfo->healAfterHazards = TRUE;
    }

    // Wish heals at end of turn
    if (gBattleStruct->wish[battler].counter > 0)
    {
        healInfo->hasHealing = TRUE;
        healInfo->healEndOfTurn = TRUE;
        healInfo->wishCounter = gBattleStruct->wish[battler].counter;
        healInfo->healAmount = GetWishHealAmountForBattler(battler);
    }
}

u32 GetSwitchChance(enum ShouldSwitchScenario shouldSwitchScenario)
{
    // Modify these cases if you want unique behaviour based on other data (trainer class, difficulty, etc.)
    switch (shouldSwitchScenario)
    {
    case SHOULD_SWITCH_WONDER_GUARD:
        return SHOULD_SWITCH_WONDER_GUARD_PERCENTAGE;
    case SHOULD_SWITCH_ABSORBS_MOVE:
        return SHOULD_SWITCH_ABSORBS_MOVE_PERCENTAGE;
    case SHOULD_SWITCH_TRAPPER:
        return SHOULD_SWITCH_TRAPPER_PERCENTAGE;
    case SHOULD_SWITCH_FREE_TURN:
        return SHOULD_SWITCH_FREE_TURN_PERCENTAGE;
    case SHOULD_SWITCH_TRUANT:
        return SHOULD_SWITCH_TRUANT_PERCENTAGE;
    case SHOULD_SWITCH_ALL_MOVES_BAD:
        return GetConfig(SHOULD_SWITCH_ALL_MOVES_BAD_PERCENTAGE);
    case SHOULD_SWITCH_PERISH_SONG:
        return SHOULD_SWITCH_PERISH_SONG_PERCENTAGE;
    case SHOULD_SWITCH_YAWN:
        return SHOULD_SWITCH_YAWN_PERCENTAGE;
    case SHOULD_SWITCH_BADLY_POISONED:
        return SHOULD_SWITCH_BADLY_POISONED_PERCENTAGE;
    case SHOULD_SWITCH_BADLY_POISONED_STATS_RAISED:
        return SHOULD_SWITCH_BADLY_POISONED_STATS_RAISED_PERCENTAGE;
    case SHOULD_SWITCH_CURSED:
        return SHOULD_SWITCH_CURSED_PERCENTAGE;
    case SHOULD_SWITCH_CURSED_STATS_RAISED:
        return SHOULD_SWITCH_CURSED_STATS_RAISED_PERCENTAGE;
    case SHOULD_SWITCH_NIGHTMARE:
        return SHOULD_SWITCH_NIGHTMARE_PERCENTAGE;
    case SHOULD_SWITCH_NIGHTMARE_STATS_RAISED:
        return SHOULD_SWITCH_NIGHTMARE_STATS_RAISED_PERCENTAGE;
    case SHOULD_SWITCH_SEEDED:
        return SHOULD_SWITCH_SEEDED_PERCENTAGE;
    case SHOULD_SWITCH_SEEDED_STATS_RAISED:
        return SHOULD_SWITCH_SEEDED_STATS_RAISED_PERCENTAGE;
    case SHOULD_SWITCH_INFATUATION:
        return SHOULD_SWITCH_INFATUATION_PERCENTAGE;
    case SHOULD_SWITCH_HASBADODDS:
        return SHOULD_SWITCH_HASBADODDS_PERCENTAGE;
    case SHOULD_SWITCH_NATURAL_CURE_STRONG:
        return SHOULD_SWITCH_NATURAL_CURE_STRONG_PERCENTAGE;
    case SHOULD_SWITCH_NATURAL_CURE_STRONG_STATS_RAISED:
        return SHOULD_SWITCH_NATURAL_CURE_STRONG_STATS_RAISED_PERCENTAGE;
    case SHOULD_SWITCH_NATURAL_CURE_WEAK:
        return SHOULD_SWITCH_NATURAL_CURE_WEAK_PERCENTAGE;
    case SHOULD_SWITCH_NATURAL_CURE_WEAK_STATS_RAISED:
        return SHOULD_SWITCH_NATURAL_CURE_WEAK_STATS_RAISED_PERCENTAGE;
    case SHOULD_SWITCH_REGENERATOR:
        return SHOULD_SWITCH_REGENERATOR_PERCENTAGE;
    case SHOULD_SWITCH_REGENERATOR_STATS_RAISED:
        return SHOULD_SWITCH_REGENERATOR_STATS_RAISED_PERCENTAGE;
    case SHOULD_SWITCH_INTIMIDATE:
        return SHOULD_SWITCH_INTIMIDATE_PERCENTAGE;
    case SHOULD_SWITCH_INTIMIDATE_STATS_RAISED:
        return SHOULD_SWITCH_INTIMIDATE_STATS_RAISED_PERCENTAGE;
    case SHOULD_SWITCH_ENCORE_STATUS:
        return SHOULD_SWITCH_ENCORE_STATUS_PERCENTAGE;
    case SHOULD_SWITCH_ENCORE_DAMAGE:
        return SHOULD_SWITCH_ENCORE_DAMAGE_PERCENTAGE;
    case SHOULD_SWITCH_CHOICE_LOCKED:
        return SHOULD_SWITCH_CHOICE_LOCKED_PERCENTAGE;
    case SHOULD_SWITCH_ATTACKING_STAT_MINUS_TWO:
        return SHOULD_SWITCH_ATTACKING_STAT_MINUS_TWO_PERCENTAGE;
    case SHOULD_SWITCH_ATTACKING_STAT_MINUS_THREE_PLUS:
        return SHOULD_SWITCH_ATTACKING_STAT_MINUS_THREE_PLUS_PERCENTAGE;
    case SHOULD_SWITCH_ALL_SCORES_BAD:
        return SHOULD_SWITCH_ALL_SCORES_BAD_PERCENTAGE;
    case SHOULD_SWITCH_DYN_FUNC:
        return SHOULD_SWITCH_DYN_FUNC_PERCENTAGE;
    case SHOULD_SWITCH_WISH_PASSING:
        return SHOULD_SWITCH_WISH_PASSING_PERCENTAGE;
    case SHOULD_SWITCH_LOSES_1V1:
        return GetConfig(SHOULD_SWITCH_LOSES_1V1_PERCENTAGE);
    default:
        return 100;
    }
}

bool32 IsAceMon(enum BattlerId battler, u32 monPartyId)
{
    enum BattleTrainer trainer = GetBattlerTrainer(battler);

    if (gAiThinkingStruct->aiFlags[battler] & AI_FLAG_ACE_POKEMON
     && !gProtectStructs[battler].forcedSwitch
     && monPartyId == CalculatePartyCount(trainer)-1)
    {
        return TRUE;
    }
    if (gAiThinkingStruct->aiFlags[battler] & AI_FLAG_DOUBLE_ACE_POKEMON
     && !gProtectStructs[battler].forcedSwitch
     && (monPartyId == CalculatePartyCount(trainer)-1 || monPartyId == CalculatePartyCount(trainer)-2))
    {
        return TRUE;
    }
    return FALSE;
}

static bool32 AreStatsRaised(enum BattlerId battler)
{
    u8 buffedStatsValue = 0;

    for (u32 statIndex = 0; statIndex < NUM_BATTLE_STATS; statIndex++)
    {
        if (gBattleMons[battler].statStages[statIndex] > DEFAULT_STAT_STAGE)
            buffedStatsValue += gBattleMons[battler].statStages[statIndex] - DEFAULT_STAT_STAGE;
    }

    return (buffedStatsValue > STAY_IN_STATS_RAISED);
}

bool32 IsSwitchinTSpikesAffected(enum BattlerId battler)
{
    enum Ability ability = gAiLogicData->abilities[battler];
    enum HoldEffect heldItemEffect = gAiLogicData->holdEffects[battler];
    enum BattlerId opposingBattler = GetOppositeBattler(battler);
    bool32 ignoreItem = ((gFieldStatuses & STATUS_FIELD_MAGIC_ROOM) || ability == ABILITY_KLUTZ);
    if (gBattleMons[battler].status1 & STATUS1_ANY)
        return FALSE;
    if (IS_BATTLER_ANY_TYPE(battler, TYPE_POISON, TYPE_STEEL))
        return FALSE;
    if (ability == ABILITY_IMMUNITY || AI_IsAbilityOnSide(battler, ABILITY_PASTEL_VEIL))
        return FALSE;
    if ((heldItemEffect == HOLD_EFFECT_HEAVY_DUTY_BOOTS || heldItemEffect == HOLD_EFFECT_CURE_PSN || heldItemEffect == HOLD_EFFECT_CURE_STATUS) && !ignoreItem)
        return FALSE;
    if (!AI_IsBattlerGrounded(battler))
        return FALSE;
    if (IsMistyTerrainAffected(battler, ability, heldItemEffect, gFieldStatuses))
        return FALSE;
    if (IsLeafGuardProtected(battler, ability))
        return FALSE;
    if (IsShieldsDownProtected(battler, ability))
        return FALSE;
    if (IsFlowerVeilProtected(battler))
        return FALSE;
    if (IsSafeguardProtected(opposingBattler, battler, gAiLogicData->abilities[opposingBattler]))
        return FALSE;
    return TRUE;
}

static inline bool32 SetSwitchinAndSwitch(enum BattlerId battler, u32 switchinId)
{
    gBattleStruct->AI_monToSwitchIntoId[battler] = switchinId;
    return TRUE;
}

static bool32 AI_DoesChoiceEffectBlockMove(enum BattlerId battler, enum Move move)
{
    // Choice locked into something else
    if (gAiLogicData->lastUsedMove[battler] != MOVE_NONE && gAiLogicData->lastUsedMove[battler] != move
    && (IsHoldEffectChoice(GetBattlerHoldEffect(battler) && IsBattlerItemEnabled(battler))
        || gAiLogicData->abilities[battler] == ABILITY_GORILLA_TACTICS))
        return TRUE;
    return FALSE;
}

static inline bool32 CanBattlerWin1v1(u32 hitsToKOAI, u32 hitsToKOPlayer, bool32 isBattlerFirst)
{
    // Player's best move deals 0 damage
    if (hitsToKOAI == 0 && hitsToKOPlayer > 0)
        return TRUE;

    // AI's best move deals 0 damage
    if (hitsToKOPlayer == 0 && hitsToKOAI > 0)
        return FALSE;

    // Neither mon can damage the other
    if (hitsToKOPlayer == 0 && hitsToKOAI == 0)
        return FALSE;

    // Different KO thresholds depending on who goes first
    if (isBattlerFirst)
    {
        if (hitsToKOAI >= hitsToKOPlayer)
            return TRUE;
    }
    else
    {
        if (hitsToKOAI > hitsToKOPlayer)
            return TRUE;
    }
    return FALSE;
}

static bool32 DoesMostSuitableSwitchinBenefitFromWish(enum BattlerId battler)
{
    struct Pokemon *party = GetBattlerParty(battler);
    u32 wishHealAmount = GetWishHealAmountForBattler(battler);
    u32 currentHp;
    u32 maxHp;
    s32 possibleHeal = wishHealAmount;

    if (gAiLogicData->mostSuitableMonId[battler] == PARTY_SIZE)
        return FALSE;

    maxHp = GetMonData(&party[gAiLogicData->mostSuitableMonId[battler]], MON_DATA_MAX_HP);
    currentHp = GetMonData(&party[gAiLogicData->mostSuitableMonId[battler]], MON_DATA_HP);

    if (possibleHeal > (s32)(maxHp - currentHp))
        possibleHeal = (s32)(maxHp - currentHp);

    return possibleHeal > (s32)(maxHp / AI_WISH_HEAL_THRESHOLD);
}

// Note that as many return statements as possible are INTENTIONALLY put after all of the loops;
// the function can take a max of about 0.06s to run, and this prevents the player from identifying
// whether the mon will switch or not by seeing how long the delay is before they select a move
static bool32 ShouldSwitchIfHasBadOdds(struct SwitchAiContext *switchContext)
{
    // Only use this if AI_FLAG_SMART_SWITCHING is set for the trainer
    if (!(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_SMART_SWITCHING))
        return FALSE;

    // Double Battles aren't included in AI_FLAG_SMART_MON_CHOICE. Defaults to regular switch in logic
    if (IsDoubleBattle())
        return FALSE;

    // Check if current mon can 1v1 in spite of bad matchup, and don't switch out if it can
    if (switchContext->canBattlerWin1v1)
        return FALSE;

    // If we don't have any other viable options, don't switch out
    if (gAiLogicData->mostSuitableMonId[switchContext->battler] == PARTY_SIZE)
        return FALSE;

    // Start assessing whether or not mon has bad odds
    // Jump straight to switching out in cases where mon gets OHKO'd
    if ((switchContext->battlerGetsOHKOd && !switchContext->canBattlerWin1v1) && (gBattleMons[switchContext->battler].hp >= gBattleMons[switchContext->battler].maxHP / 2 // And the current mon has at least 1/2 their HP, or 1/4 HP and Regenerator
            || (gAiLogicData->abilities[switchContext->battler] == ABILITY_REGENERATOR && gBattleMons[switchContext->battler].hp >= gBattleMons[switchContext->battler].maxHP / 4)))
    {
        // 50% chance to stay in regardless
        if (RandomPercentage(RNG_AI_SWITCH_HASBADODDS, (100 - GetSwitchChance(SHOULD_SWITCH_HASBADODDS))) && !gAiLogicData->aiPredictionInProgress)
            return FALSE;

        // Switch mon out
        return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);
    }

    // General bad type matchups have more wiggle room
    if (switchContext->typeMatchup > UQ_4_12(2.0)) // If the player has favourable offensive matchup (2.0 is neutral, this must be worse)
    {
        if (!switchContext->hasEffectiveMove // If the AI doesn't have a super effective move
        && (gBattleMons[switchContext->battler].hp >= gBattleMons[switchContext->battler].maxHP / 2 // And the current mon has at least 1/2 their HP, or 1/4 HP and Regenerator
            || (gAiLogicData->abilities[switchContext->battler] == ABILITY_REGENERATOR
            && gBattleMons[switchContext->battler].hp >= gBattleMons[switchContext->battler].maxHP / 4)))
        {
            // Then check if they have an important status move, which is worth using even in a bad matchup
            if (switchContext->hasImportantStatusMove)
                return FALSE;

            // 50% chance to stay in regardless
            if (RandomPercentage(RNG_AI_SWITCH_HASBADODDS, (100 - GetSwitchChance(SHOULD_SWITCH_HASBADODDS))) && !gAiLogicData->aiPredictionInProgress)
                return FALSE;

            // Switch mon out
            return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);
        }
    }
    return FALSE;
}

static bool32 CanDoubleBattlerMakeProgress(struct SwitchAiContext *switchContext)
{
    enum BattlerId battler = switchContext->battler;
    enum BattlerId opposingBattler = switchContext->opposingBattler;
    enum BattlerId opposingPartner = BATTLE_PARTNER(opposingBattler);

    if (CanUseSuperEffectiveMoveAgainstOpponents(battler, opposingBattler))
        return TRUE;

    if (GetBestNoOfHitsToKO(battler, opposingBattler, AI_ATTACKING) <= 2)
        return TRUE;

    if (IsBattlerAlive(opposingPartner) && GetBestNoOfHitsToKO(battler, opposingPartner, AI_ATTACKING) <= 2)
        return TRUE;

    return FALSE;
}

static bool32 IsDoubleBattlerUnderPressureFrom(enum BattlerId opposingBattler, enum BattlerId battler)
{
    if (!IsBattlerAlive(opposingBattler))
        return FALSE;

    if (GetBestNoOfHitsToKO(opposingBattler, battler, AI_DEFENDING) == 1)
        return TRUE;

    if (GetBattlerTypeMatchup(opposingBattler, battler) > UQ_4_12(2.0))
        return TRUE;

    return FALSE;
}

static bool32 IsDoubleBattlerUnderPressure(struct SwitchAiContext *switchContext)
{
    enum BattlerId opposingBattler = switchContext->opposingBattler;
    enum BattlerId opposingPartner = BATTLE_PARTNER(opposingBattler);

    if (IsDoubleBattlerUnderPressureFrom(opposingBattler, switchContext->battler))
        return TRUE;

    if (IsDoubleBattlerUnderPressureFrom(opposingPartner, switchContext->battler))
        return TRUE;

    return FALSE;
}

static bool32 CanDoublePartnerCoverPosition(struct SwitchAiContext *switchContext)
{
    enum BattlerId partner = BATTLE_PARTNER(switchContext->battler);
    enum BattlerId opposingBattler = switchContext->opposingBattler;
    enum BattlerId opposingPartner = BATTLE_PARTNER(opposingBattler);

    if (!IsBattlerAlive(partner))
        return FALSE;

    if (CanUseSuperEffectiveMoveAgainstOpponents(partner, opposingBattler))
        return TRUE;

    if (GetBestNoOfHitsToKO(partner, opposingBattler, AI_ATTACKING) <= 2)
        return TRUE;

    if (IsBattlerAlive(opposingPartner) && GetBestNoOfHitsToKO(partner, opposingPartner, AI_ATTACKING) <= 2)
        return TRUE;

    return FALSE;
}

static bool32 ShouldAvoidDoublePositionIntimidateCycle(struct SwitchAiContext *switchContext)
{
    enum BattlerId opposingBattler = switchContext->opposingBattler;
    enum BattlerId opposingPartner = BATTLE_PARTNER(opposingBattler);

    if (gAiLogicData->abilities[switchContext->battler] != ABILITY_INTIMIDATE)
        return FALSE;

    if (CanIntimidateLowerOpponentAtk(switchContext->battler, opposingBattler))
        return FALSE;

    if (IsBattlerAlive(opposingPartner) && CanIntimidateLowerOpponentAtk(switchContext->battler, opposingPartner))
        return FALSE;

    return TRUE;
}

static uq4_12_t GetWorstDoubleTypeMatchupAgainstTypes(enum BattlerId opposingBattler, enum BattlerId opposingPartner, enum Type defType1, enum Type defType2)
{
    uq4_12_t matchup = GetTypeMatchupAgainstTypes(opposingBattler, defType1, defType2);
    uq4_12_t partnerMatchup;

    if (IsBattlerAlive(opposingPartner))
    {
        partnerMatchup = GetTypeMatchupAgainstTypes(opposingPartner, defType1, defType2);
        if (partnerMatchup > matchup)
            matchup = partnerMatchup;
    }

    return matchup;
}

static u32 FindDoublePositionSwitchin(struct SwitchAiContext *switchContext)
{
    enum BattlerId opposingBattler = switchContext->opposingBattler;
    enum BattlerId opposingPartner = BATTLE_PARTNER(opposingBattler);
    uq4_12_t currentMatchup = GetWorstDoubleTypeMatchupAgainstTypes(opposingBattler, opposingPartner, gBattleMons[switchContext->battler].types[0], gBattleMons[switchContext->battler].types[1]);
    uq4_12_t bestMatchup = currentMatchup;
    u32 bestMonId = PARTY_SIZE;

    for (u32 monIndex = 0; monIndex < switchContext->lastId; monIndex++)
    {
        enum Species species;
        enum Type type1, type2;
        uq4_12_t switchinMatchup;

        if (!(switchContext->eligiblePartyMons & (1u << monIndex)))
            continue;

        species = GetMonData(&switchContext->party[monIndex], MON_DATA_SPECIES);
        type1 = GetSpeciesType(species, 0);
        type2 = GetSpeciesType(species, 1);
        switchinMatchup = GetWorstDoubleTypeMatchupAgainstTypes(opposingBattler, opposingPartner, type1, type2);

        if (switchinMatchup < bestMatchup)
        {
            bestMonId = monIndex;
            bestMatchup = switchinMatchup;
        }
    }

    return bestMonId;
}

static bool32 ShouldPreserveDoubleBattler(struct SwitchAiContext *switchContext)
{
    enum BattlerId battler = switchContext->battler;

    if (gBattleMons[battler].hp >= gBattleMons[battler].maxHP / 2)
        return TRUE;

    if (gAiLogicData->abilities[battler] == ABILITY_REGENERATOR && gBattleMons[battler].hp >= gBattleMons[battler].maxHP / 4)
        return TRUE;

    return FALSE;
}

static bool32 ShouldSwitchIfDoublePositionBad(struct SwitchAiContext *switchContext)
{
    u32 switchinId;

    if (!(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_SMART_SWITCHING))
        return FALSE;

    if (!IsDoubleBattle())
        return FALSE;

    if (ShouldPreserveDoubleBattlerWithProtect(switchContext))
        return FALSE;

    switchinId = FindDoublePositionSwitchin(switchContext);

    if (!ShouldPreserveDoubleBattler(switchContext))
        return FALSE;

    if (switchContext->hasImportantStatusMove || switchContext->hasStatRaised)
        return FALSE;

    if (ShouldAvoidDoublePositionIntimidateCycle(switchContext))
        return FALSE;

    if (CanDoubleBattlerMakeProgress(switchContext))
        return FALSE;

    if (!IsDoubleBattlerUnderPressure(switchContext))
        return FALSE;

    if (CanDoublePartnerCoverPosition(switchContext))
        return FALSE;

    return SetSwitchinAndSwitch(switchContext->battler, switchinId);
}

static bool32 PartyMonHasMove(struct Pokemon *mon, enum Move moveToFind)
{
    for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
    {
        enum Move move = GetMonData(mon, MON_DATA_MOVE1 + moveIndex);
        if (move == moveToFind)
            return TRUE;
    }

    return FALSE;
}

static bool32 PartyMonHasMoveEffect(struct Pokemon *mon, enum BattleMoveEffects effect)
{
    for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
    {
        enum Move move = GetMonData(mon, MON_DATA_MOVE1 + moveIndex);
        if (move != MOVE_NONE && GetMoveEffect(move) == effect)
            return TRUE;
    }

    return FALSE;
}

static enum BattlerId GetKnownPlayerChosenMoveTarget(enum BattlerId battler)
{
    if (gBattleStruct != NULL && gBattleStruct->moveTarget[battler] < gBattlersCount)
        return gBattleStruct->moveTarget[battler];

    if (gBattleResources != NULL
     && IsOnPlayerSide(battler)
     && gChosenActionByBattler[battler] == B_ACTION_USE_MOVE
     && gChosenMoveByBattler[battler] != MOVE_NONE
     && gChosenMoveByBattler[battler] != MOVE_UNAVAILABLE
     && gBattleResources->bufferB[battler][3] < gBattlersCount)
        return gBattleResources->bufferB[battler][3];

    return MAX_BATTLERS_COUNT;
}

static u32 GetKnownPlayerChosenMoveIndex(enum BattlerId battler, enum Move move)
{
    u32 moveIndex = gBattleStruct->chosenMovePositions[battler];

    if (moveIndex < MAX_MON_MOVES && gBattleMons[battler].moves[moveIndex] == move)
        return moveIndex;

    return GetMoveIndex(battler, move);
}

static enum AIConsiderGimmick GetKnownPlayerChosenGimmickForSwitch(enum BattlerId battler)
{
    enum Gimmick gimmick;

    if (gBattleStruct == NULL || battler >= gBattlersCount)
        return NO_GIMMICK;

    gimmick = gBattleStruct->gimmick.usableGimmick[battler];
    if (gimmick != GIMMICK_NONE && IsGimmickSelected(battler, gimmick))
        return USE_GIMMICK;

    return NO_GIMMICK;
}

static u32 EstimateDamageAtRollPercent(const struct SimulatedDamage *damage, u32 rollPercent)
{
    u32 rollRange;
    u32 spread;

    if (damage == NULL)
        return 0;
    if (rollPercent <= MIN_ROLL_PERCENTAGE)
        return damage->minimum;
    if (rollPercent >= MAX_ROLL_PERCENTAGE || damage->maximum <= damage->minimum)
        return damage->maximum;

    rollRange = MAX_ROLL_PERCENTAGE - MIN_ROLL_PERCENTAGE;
    spread = damage->maximum - damage->minimum;
    return damage->minimum + DIV_ROUND_UP(spread * (rollPercent - MIN_ROLL_PERCENTAGE), rollRange);
}

static void GetDamageRollValues(const struct SimulatedDamage *damage, u32 *rollValues)
{
    for (u32 rollIndex = 0; rollIndex < AI_SWITCH_DAMAGE_ROLL_COUNT; rollIndex++)
        rollValues[rollIndex] = EstimateDamageAtRollPercent(damage, MIN_ROLL_PERCENTAGE + rollIndex);
}

static void SortDamageRollSums(u32 *rollSums, u32 count)
{
    for (u32 i = 1; i < count; i++)
    {
        u32 value = rollSums[i];
        u32 j = i;

        while (j > 0 && rollSums[j - 1] > value)
        {
            rollSums[j] = rollSums[j - 1];
            j--;
        }
        rollSums[j] = value;
    }
}

static bool32 SetKnownPlayerDamageRollSummaryFromDistribution(const struct SimulatedDamage *damages, u32 damageCount, struct KnownPlayerDamageRollSummary *summary)
{
    u32 rollSums[AI_SWITCH_MAX_DAMAGE_ROLL_COMBINATIONS] = {0};
    u32 nextRollSums[AI_SWITCH_MAX_DAMAGE_ROLL_COMBINATIONS];
    u32 combinationCount = 1;

    if (summary == NULL)
        return FALSE;

    *summary = (struct KnownPlayerDamageRollSummary){0};

    if (damages == NULL || damageCount == 0)
        return FALSE;

    for (u32 damageIndex = 0; damageIndex < damageCount; damageIndex++)
    {
        u32 rollValues[AI_SWITCH_DAMAGE_ROLL_COUNT];
        u32 nextCombinationCount;

        if (combinationCount > AI_SWITCH_MAX_DAMAGE_ROLL_COMBINATIONS / AI_SWITCH_DAMAGE_ROLL_COUNT)
            return FALSE;

        nextCombinationCount = combinationCount * AI_SWITCH_DAMAGE_ROLL_COUNT;
        GetDamageRollValues(&damages[damageIndex], rollValues);
        for (u32 comboIndex = 0; comboIndex < combinationCount; comboIndex++)
        {
            for (u32 rollIndex = 0; rollIndex < AI_SWITCH_DAMAGE_ROLL_COUNT; rollIndex++)
                nextRollSums[comboIndex * AI_SWITCH_DAMAGE_ROLL_COUNT + rollIndex] = rollSums[comboIndex] + rollValues[rollIndex];
        }

        combinationCount = nextCombinationCount;
        for (u32 comboIndex = 0; comboIndex < combinationCount; comboIndex++)
            rollSums[comboIndex] = nextRollSums[comboIndex];
    }

    SortDamageRollSums(rollSums, combinationCount);
    summary->minimum = rollSums[0];
    summary->median = rollSums[combinationCount / 2];
    summary->roll14Of16 = rollSums[DIV_ROUND_UP(combinationCount * 14, 16) - 1];
    summary->roll15Of16 = rollSums[DIV_ROUND_UP(combinationCount * 15, 16) - 1];
    summary->maximum = rollSums[combinationCount - 1];
    return TRUE;
}

static struct SimulatedDamage GetKnownPlayerSelectedMoveDamageRollsForSwitch(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, u32 moveIndex)
{
    enum AIConsiderGimmick considerGimmickAtk = GetKnownPlayerChosenGimmickForSwitch(battlerAtk);

    if (considerGimmickAtk == USE_GIMMICK)
    {
        uq4_12_t effectiveness;
        return AI_CalcDamage(move, battlerAtk, battlerDef, &effectiveness, considerGimmickAtk, NO_GIMMICK, AI_GetWeather(), gFieldStatuses);
    }

    return gAiLogicData->simulatedDmg[battlerAtk][battlerDef][moveIndex];
}

static u32 GetKnownPlayerSelectedMoveDamageForSwitch(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, u32 moveIndex, enum DamageCalcContext damageContext)
{
    enum AIConsiderGimmick considerGimmickAtk = GetKnownPlayerChosenGimmickForSwitch(battlerAtk);

    if (considerGimmickAtk == USE_GIMMICK)
    {
        uq4_12_t effectiveness;
        struct SimulatedDamage damage = AI_CalcDamage(move, battlerAtk, battlerDef, &effectiveness, considerGimmickAtk, NO_GIMMICK, AI_GetWeather(), gFieldStatuses);

        switch (damageContext)
        {
        case AI_DEFENDING:
        case AI_SWITCHIN_DEFENDING:
        case AI_SHOULD_SETUP_DEFENDING:
            return damage.maximum;
        default:
            return damage.median;
        }
    }

    return AI_GetDamage(battlerAtk, battlerDef, moveIndex, damageContext, gAiLogicData);
}

static bool32 CanKnownMoveHitBattlerSlot(enum BattlerId battlerAtk, enum BattlerId target, enum Move move, enum BattlerId chosenTarget)
{
    enum MoveTarget moveTarget;

    if (target >= gBattlersCount || !IsBattlerAlive(target))
        return FALSE;
    if (IsBattleMoveStatus(move))
        return FALSE;
    if (AI_ShouldAvoidCommanderTatsugiriTarget(target, move))
        return FALSE;

    moveTarget = AI_GetBattlerMoveTargetType(battlerAtk, move);
    switch (moveTarget)
    {
    case TARGET_SELECTED:
    case TARGET_SMART:
    case TARGET_DEPENDS:
    case TARGET_OPPONENT:
    case TARGET_RANDOM:
    case TARGET_ALLY:
    case TARGET_USER_OR_ALLY:
        return chosenTarget == target && CanTargetBattler(battlerAtk, target, move);
    case TARGET_BOTH:
        return !IsBattlerAlly(battlerAtk, target) && CanTargetBattler(battlerAtk, target, move);
    case TARGET_FOES_AND_ALLY:
    case TARGET_ALL_BATTLERS:
        return battlerAtk != target && CanTargetBattler(battlerAtk, target, move);
    case TARGET_USER:
        return battlerAtk == target && CanTargetBattler(battlerAtk, target, move);
    case TARGET_USER_AND_ALLY:
        return IsBattlerAlly(battlerAtk, target) && CanTargetBattler(battlerAtk, target, move);
    default:
        return FALSE;
    }
}

static bool32 IsKnownPlayerSingleTargetDamageMoveTargetingBattler(enum BattlerId battler, enum BattlerId opposingBattler, enum Move move)
{
    enum MoveTarget moveTarget;
    enum BattlerId chosenTarget;

    if (IsBattleMoveStatus(move))
        return FALSE;

    moveTarget = AI_GetBattlerMoveTargetType(opposingBattler, move);
    if (IsSpreadMove(moveTarget) || moveTarget == TARGET_ALL_BATTLERS || moveTarget == TARGET_FIELD || moveTarget == TARGET_OPPONENTS_FIELD)
        return FALSE;

    switch (moveTarget)
    {
    case TARGET_SELECTED:
    case TARGET_SMART:
    case TARGET_DEPENDS:
    case TARGET_OPPONENT:
    case TARGET_RANDOM:
        chosenTarget = GetKnownPlayerChosenMoveTarget(opposingBattler);
        return CanKnownMoveHitBattlerSlot(opposingBattler, battler, move, chosenTarget);
    default:
        return FALSE;
    }
}

static bool32 GetKnownPlayerSingleTargetDamageThreat(enum BattlerId battler, enum BattlerId *threatBattler, enum Move *threatMove, u32 *threatMoveIndex)
{
    u32 bestDamage = 0;
    bool32 found = FALSE;

    if (!(gAiThinkingStruct->aiFlags[battler] & AI_FLAG_READ_PLAYER_MOVE) || !BattlerHasAi(battler))
        return FALSE;

    for (enum BattlerId opposingBattler = 0; opposingBattler < gBattlersCount; opposingBattler++)
    {
        enum Move move;
        u32 moveIndex;
        u32 damage;

        if (IsBattlerAlly(battler, opposingBattler) || !IsBattlerAlive(opposingBattler) || BattlerHasAi(opposingBattler))
            continue;
        if (gChosenActionByBattler[opposingBattler] != B_ACTION_USE_MOVE)
            continue;

        move = gChosenMoveByBattler[opposingBattler];
        if (move == MOVE_NONE || move == MOVE_UNAVAILABLE)
            continue;
        if (!IsKnownPlayerSingleTargetDamageMoveTargetingBattler(battler, opposingBattler, move))
            continue;

        moveIndex = GetKnownPlayerChosenMoveIndex(opposingBattler, move);
        if (moveIndex >= MAX_MON_MOVES)
            continue;

        damage = GetKnownPlayerSelectedMoveDamageForSwitch(opposingBattler, battler, move, moveIndex, AI_DEFENDING);
        if (damage > bestDamage)
        {
            *threatBattler = opposingBattler;
            *threatMove = move;
            *threatMoveIndex = moveIndex;
            bestDamage = damage;
            found = TRUE;
        }
    }

    return found;
}

static bool32 ShouldUseKnownIncomingMoveForSingleSwitch(struct SwitchAiContext *switchContext)
{
    if (IsDoubleBattle())
        return FALSE;
    if (!(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_READ_PLAYER_MOVE))
        return FALSE;
    if (switchContext->incomingBattler >= gBattlersCount || switchContext->incomingMoveIndex >= MAX_MON_MOVES)
        return FALSE;
    if (switchContext->incomingMove == MOVE_NONE || switchContext->incomingMove == MOVE_UNAVAILABLE)
        return FALSE;
    if (BattlerHasAi(switchContext->incomingBattler) || IsBattlerAlly(switchContext->battler, switchContext->incomingBattler))
        return FALSE;
    return gChosenActionByBattler[switchContext->incomingBattler] == B_ACTION_USE_MOVE;
}

static void SetKnownIncomingMoveForSwitchContext(struct SwitchAiContext *switchContext)
{
    enum BattlerId threatBattler = MAX_BATTLERS_COUNT;
    enum Move threatMove = MOVE_NONE;
    u32 threatMoveIndex = MAX_MON_MOVES;

    switchContext->incomingBattler = switchContext->opposingBattler;
    switchContext->incomingMove = GetIncomingMove(switchContext->battler, switchContext->opposingBattler, gAiLogicData);
    switchContext->incomingMoveIndex = GetMoveIndex(switchContext->opposingBattler, switchContext->incomingMove);

    if (GetKnownPlayerSingleTargetDamageThreat(switchContext->battler, &threatBattler, &threatMove, &threatMoveIndex))
    {
        switchContext->incomingBattler = threatBattler;
        switchContext->incomingMove = threatMove;
        switchContext->incomingMoveIndex = threatMoveIndex;
    }
}

static bool32 IsKnownPlayerMoveCommandReadyForSwitch(enum BattlerId battler)
{
    if (gBattleMons[battler].volatiles.multipleTurns || gBattleMons[battler].volatiles.rechargeTimer > 0)
        return TRUE;

    return gChosenMoveByBattler[battler] != MOVE_NONE
        && gChosenMoveByBattler[battler] != MOVE_UNAVAILABLE;
}

static bool32 IsKnownPlayerCommandReadyForSwitch(enum BattlerId battler)
{
    if (!IsOnPlayerSide(battler))
        return TRUE;
    if ((gAbsentBattlerFlags & (1u << battler)) || !IsBattlerAlive(battler))
        return TRUE;
    if (gBattleStruct->battlerState[battler].commandingDondozo)
        return TRUE;

    switch (gChosenActionByBattler[battler])
    {
    case B_ACTION_USE_MOVE:
        return IsKnownPlayerMoveCommandReadyForSwitch(battler);
    case B_ACTION_SWITCH:
        return gBattleStruct->monToSwitchIntoId[battler] < PARTY_SIZE;
    case B_ACTION_USE_ITEM:
    case B_ACTION_RUN:
    case B_ACTION_SAFARI_WATCH_CAREFULLY:
    case B_ACTION_SAFARI_BALL:
    case B_ACTION_SAFARI_POKEBLOCK:
    case B_ACTION_SAFARI_GO_NEAR:
    case B_ACTION_SAFARI_RUN:
    case B_ACTION_WALLY_THROW:
    case B_ACTION_THROW_BALL:
    case B_ACTION_DEBUG:
    case B_ACTION_EXEC_SCRIPT:
    case B_ACTION_TRY_FINISH:
    case B_ACTION_FINISHED:
    case B_ACTION_NOTHING_FAINTED:
        return TRUE;
    default:
        return FALSE;
    }
}

static bool32 AreKnownPlayerCommandsReadyForSwitch(void)
{
    for (enum BattlerId battler = 0; battler < gBattlersCount; battler++)
    {
        if (!IsKnownPlayerCommandReadyForSwitch(battler))
            return FALSE;
    }

    return TRUE;
}

static u32 GetKnownPlayerDamageSummaryIntoBattlerSlot(enum BattlerId battler, enum DamageCalcContext damageContext, u32 *hitCount, bool32 *hasFakeOut)
{
    u32 totalDamage = 0;

    if (hitCount != NULL)
        *hitCount = 0;
    if (hasFakeOut != NULL)
        *hasFakeOut = FALSE;

    if (!(gAiThinkingStruct->aiFlags[battler] & AI_FLAG_READ_PLAYER_MOVE) || !BattlerHasAi(battler))
        return 0;

    for (enum BattlerId opposingBattler = 0; opposingBattler < gBattlersCount; opposingBattler++)
    {
        enum Move move;
        enum BattlerId chosenTarget;
        u32 moveIndex;

        if (IsBattlerAlly(battler, opposingBattler) || !IsBattlerAlive(opposingBattler) || BattlerHasAi(opposingBattler))
            continue;
        if (gChosenActionByBattler[opposingBattler] != B_ACTION_USE_MOVE)
            continue;

        move = gChosenMoveByBattler[opposingBattler];
        if (move == MOVE_NONE || move == MOVE_UNAVAILABLE)
            continue;

        chosenTarget = GetKnownPlayerChosenMoveTarget(opposingBattler);
        if (!CanKnownMoveHitBattlerSlot(opposingBattler, battler, move, chosenTarget))
            continue;

        moveIndex = GetKnownPlayerChosenMoveIndex(opposingBattler, move);
        if (moveIndex >= MAX_MON_MOVES)
            continue;

        totalDamage += GetKnownPlayerSelectedMoveDamageForSwitch(opposingBattler, battler, move, moveIndex, damageContext);
        if (hitCount != NULL)
            (*hitCount)++;
        if (hasFakeOut != NULL && GetMoveEffect(move) == EFFECT_FIRST_TURN_ONLY && MoveHasAdditionalEffect(move, MOVE_EFFECT_FLINCH))
            *hasFakeOut = TRUE;
    }

    return totalDamage;
}

static void GetKnownPlayerDamageRollSummaryIntoBattlerSlot(enum BattlerId battler, struct KnownPlayerDamageRollSummary *summary)
{
    struct SimulatedDamage damages[MAX_BATTLERS_COUNT];
    u32 damageCount = 0;

    if (summary == NULL)
        return;

    *summary = (struct KnownPlayerDamageRollSummary){0};

    if (!(gAiThinkingStruct->aiFlags[battler] & AI_FLAG_READ_PLAYER_MOVE) || !BattlerHasAi(battler))
        return;

    for (enum BattlerId opposingBattler = 0; opposingBattler < gBattlersCount; opposingBattler++)
    {
        enum Move move;
        enum BattlerId chosenTarget;
        u32 moveIndex;
        struct SimulatedDamage damage;

        if (IsBattlerAlly(battler, opposingBattler) || !IsBattlerAlive(opposingBattler) || BattlerHasAi(opposingBattler))
            continue;
        if (gChosenActionByBattler[opposingBattler] != B_ACTION_USE_MOVE)
            continue;

        move = gChosenMoveByBattler[opposingBattler];
        if (move == MOVE_NONE || move == MOVE_UNAVAILABLE)
            continue;

        chosenTarget = GetKnownPlayerChosenMoveTarget(opposingBattler);
        if (!CanKnownMoveHitBattlerSlot(opposingBattler, battler, move, chosenTarget))
            continue;

        moveIndex = GetKnownPlayerChosenMoveIndex(opposingBattler, move);
        if (moveIndex >= MAX_MON_MOVES)
            continue;

        if (damageCount >= ARRAY_COUNT(damages))
            return;

        damage = GetKnownPlayerSelectedMoveDamageRollsForSwitch(opposingBattler, battler, move, moveIndex);
        damages[damageCount++] = damage;
    }

    SetKnownPlayerDamageRollSummaryFromDistribution(damages, damageCount, summary);
}

static u32 GetKnownPlayerDamageIntoBattlerSlot(enum BattlerId battler, enum DamageCalcContext damageContext)
{
    return GetKnownPlayerDamageSummaryIntoBattlerSlot(battler, damageContext, NULL, NULL);
}

static bool32 PartyMonHasDamagingMoveOfType(struct Pokemon *mon, enum Type type)
{
    for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
    {
        enum Move move = GetMonData(mon, MON_DATA_MOVE1 + moveIndex);
        if (move != MOVE_NONE && !IsBattleMoveStatus(move) && GetMoveType(move) == type)
            return TRUE;
    }

    return FALSE;
}

static bool32 PartyMonHasMoveWithCategory(struct Pokemon *mon, enum DamageCategory category)
{
    for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
    {
        enum Move move = GetMonData(mon, MON_DATA_MOVE1 + moveIndex);
        if (move != MOVE_NONE && move != MOVE_UNAVAILABLE && GetMoveCategory(move) == category)
            return TRUE;
    }

    return FALSE;
}

static bool32 PartyMonIsAnyType(struct Pokemon *mon, enum Type type)
{
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);

    return GetSpeciesType(species, 0) == type || GetSpeciesType(species, 1) == type;
}

static u32 GetSwitchinWeatherFromAbility(enum Ability ability)
{
    if (AI_GetWeather() & B_WEATHER_PRIMAL_ANY)
        return B_WEATHER_NONE;

    switch (ability)
    {
    case ABILITY_DRIZZLE:
        return B_WEATHER_RAIN;
    case ABILITY_DROUGHT:
    case ABILITY_ORICHALCUM_PULSE:
        return B_WEATHER_SUN;
    case ABILITY_SAND_STREAM:
        return B_WEATHER_SANDSTORM;
    case ABILITY_SNOW_WARNING:
        return B_WEATHER_ICY_ANY;
    default:
        return B_WEATHER_NONE;
    }
}

static u32 GetSwitchinTerrainFromAbility(enum Ability ability)
{
    if (gBattleStruct->isSkyBattle)
        return 0;

    switch (ability)
    {
    case ABILITY_ELECTRIC_SURGE:
    case ABILITY_HADRON_ENGINE:
        return STATUS_FIELD_ELECTRIC_TERRAIN;
    case ABILITY_GRASSY_SURGE:
        return STATUS_FIELD_GRASSY_TERRAIN;
    case ABILITY_MISTY_SURGE:
        return STATUS_FIELD_MISTY_TERRAIN;
    case ABILITY_PSYCHIC_SURGE:
        return STATUS_FIELD_PSYCHIC_TERRAIN;
    default:
        return 0;
    }
}

static bool32 PartyMonBenefitsFromWeather(struct Pokemon *mon, enum Ability ability, u32 weather)
{
    switch (ability)
    {
    case ABILITY_FORECAST:
        if (weather & (B_WEATHER_RAIN | B_WEATHER_SUN | B_WEATHER_ICY_ANY))
            return TRUE;
        break;
    case ABILITY_SWIFT_SWIM:
    case ABILITY_RAIN_DISH:
    case ABILITY_HYDRATION:
    case ABILITY_DRY_SKIN:
        if (weather & B_WEATHER_RAIN)
            return TRUE;
        break;
    case ABILITY_CHLOROPHYLL:
    case ABILITY_FLOWER_GIFT:
    case ABILITY_HARVEST:
    case ABILITY_LEAF_GUARD:
    case ABILITY_SOLAR_POWER:
    case ABILITY_PROTOSYNTHESIS:
    case ABILITY_ORICHALCUM_PULSE:
        if (weather & B_WEATHER_SUN)
            return TRUE;
        break;
    case ABILITY_SAND_FORCE:
    case ABILITY_SAND_RUSH:
    case ABILITY_SAND_VEIL:
        if (weather & B_WEATHER_SANDSTORM)
            return TRUE;
        break;
    case ABILITY_ICE_BODY:
    case ABILITY_ICE_FACE:
    case ABILITY_SNOW_CLOAK:
    case ABILITY_SLUSH_RUSH:
        if (weather & B_WEATHER_ICY_ANY)
            return TRUE;
        break;
    case ABILITY_MAGIC_GUARD:
    case ABILITY_OVERCOAT:
        if (weather & B_WEATHER_DAMAGING_ANY)
            return TRUE;
        break;
    default:
        break;
    }

    if (PartyMonHasMoveEffect(mon, EFFECT_WEATHER_BALL))
        return TRUE;

    if (weather & B_WEATHER_RAIN)
        return PartyMonHasDamagingMoveOfType(mon, TYPE_WATER)
            || PartyMonHasMove(mon, MOVE_THUNDER)
            || PartyMonHasMove(mon, MOVE_HURRICANE);

    if (weather & B_WEATHER_SUN)
        return PartyMonHasDamagingMoveOfType(mon, TYPE_FIRE)
            || PartyMonHasMoveEffect(mon, EFFECT_SOLAR_BEAM)
            || PartyMonHasMoveEffect(mon, EFFECT_SYNTHESIS);

    if (weather & B_WEATHER_SANDSTORM)
        return PartyMonIsAnyType(mon, TYPE_ROCK)
            || PartyMonIsAnyType(mon, TYPE_GROUND)
            || PartyMonIsAnyType(mon, TYPE_STEEL);

    if (weather & B_WEATHER_ICY_ANY)
        return PartyMonIsAnyType(mon, TYPE_ICE)
            || PartyMonHasMove(mon, MOVE_BLIZZARD)
            || PartyMonHasMoveEffect(mon, EFFECT_AURORA_VEIL);

    return FALSE;
}

static bool32 PartyMonHasTerrainSeedForField(struct Pokemon *mon, u32 terrain)
{
    enum Item item = GetMonData(mon, MON_DATA_HELD_ITEM);
    enum HoldEffect holdEffect = GetItemHoldEffect(item);
    u32 seedParam;

    if (holdEffect != HOLD_EFFECT_TERRAIN_SEED)
        return FALSE;

    seedParam = GetItemHoldEffectParam(item);
    return (seedParam == HOLD_EFFECT_PARAM_ELECTRIC_TERRAIN && terrain == STATUS_FIELD_ELECTRIC_TERRAIN)
        || (seedParam == HOLD_EFFECT_PARAM_GRASSY_TERRAIN && terrain == STATUS_FIELD_GRASSY_TERRAIN)
        || (seedParam == HOLD_EFFECT_PARAM_MISTY_TERRAIN && terrain == STATUS_FIELD_MISTY_TERRAIN)
        || (seedParam == HOLD_EFFECT_PARAM_PSYCHIC_TERRAIN && terrain == STATUS_FIELD_PSYCHIC_TERRAIN);
}

static bool32 PartyMonBenefitsFromTerrain(struct Pokemon *mon, enum Ability ability, u32 terrain)
{
    if (PartyMonHasTerrainSeedForField(mon, terrain))
        return TRUE;

    if (PartyMonHasMoveEffect(mon, EFFECT_TERRAIN_BOOST)
     || PartyMonHasMoveEffect(mon, EFFECT_TERRAIN_PULSE))
        return TRUE;

    switch (terrain)
    {
    case STATUS_FIELD_GRASSY_TERRAIN:
        return ability == ABILITY_GRASS_PELT
            || PartyMonHasDamagingMoveOfType(mon, TYPE_GRASS)
            || PartyMonHasMoveEffect(mon, EFFECT_GRASSY_GLIDE);
    case STATUS_FIELD_ELECTRIC_TERRAIN:
        return ability == ABILITY_QUARK_DRIVE
            || ability == ABILITY_HADRON_ENGINE
            || ability == ABILITY_SURGE_SURFER
            || PartyMonHasDamagingMoveOfType(mon, TYPE_ELECTRIC);
    case STATUS_FIELD_PSYCHIC_TERRAIN:
        return PartyMonHasDamagingMoveOfType(mon, TYPE_PSYCHIC);
    case STATUS_FIELD_MISTY_TERRAIN:
        return PartyMonIsAnyType(mon, TYPE_DRAGON);
    default:
        return FALSE;
    }
}

static bool32 MoveHasStatusPressureEffect(enum Move move)
{
    enum MoveEffect status;

    if (move == MOVE_NONE || move == MOVE_UNAVAILABLE)
        return FALSE;

    status = GetMoveNonVolatileStatus(move);
    if (status == MOVE_EFFECT_SLEEP
     || status == MOVE_EFFECT_POISON
     || status == MOVE_EFFECT_TOXIC
     || status == MOVE_EFFECT_BURN
     || status == MOVE_EFFECT_PARALYSIS
     || status == MOVE_EFFECT_FROSTBITE)
        return TRUE;

    if (MoveHasAdditionalEffect(move, MOVE_EFFECT_SLEEP)
     || MoveHasAdditionalEffect(move, MOVE_EFFECT_POISON)
     || MoveHasAdditionalEffect(move, MOVE_EFFECT_TOXIC)
     || MoveHasAdditionalEffect(move, MOVE_EFFECT_BURN)
     || MoveHasAdditionalEffect(move, MOVE_EFFECT_PARALYSIS)
     || MoveHasAdditionalEffect(move, MOVE_EFFECT_FREEZE_OR_FROSTBITE)
     || MoveHasAdditionalEffect(move, MOVE_EFFECT_FROSTBITE)
     || MoveHasAdditionalEffect(move, MOVE_EFFECT_CONFUSION)
     || MoveHasAdditionalEffect(move, MOVE_EFFECT_CONFUSE_SIDE)
     || MoveHasAdditionalEffect(move, MOVE_EFFECT_CONFUSE_PAY_DAY_SIDE))
        return TRUE;

    switch (GetMoveEffect(move))
    {
    case EFFECT_CONFUSE:
    case EFFECT_LEECH_SEED:
    case EFFECT_SWAGGER:
    case EFFECT_TOXIC_SPIKES:
    case EFFECT_TOXIC_THREAD:
    case EFFECT_YAWN:
        return TRUE;
    default:
        return FALSE;
    }
}

static bool32 PartyMonHasStatusPressureMove(struct Pokemon *mon)
{
    for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
    {
        enum Move move = GetMonData(mon, MON_DATA_MOVE1 + moveIndex);
        if (MoveHasStatusPressureEffect(move))
            return TRUE;
    }

    return FALSE;
}

static bool32 PartyMonCanUseAuroraVeil(struct Pokemon *mon, enum Ability ability)
{
    if (!PartyMonHasMoveEffect(mon, EFFECT_AURORA_VEIL))
        return FALSE;

    return (AI_GetWeather() & B_WEATHER_ICY_ANY)
        || (GetSwitchinWeatherFromAbility(ability) & B_WEATHER_ICY_ANY);
}

static bool32 PartyMonHasDefensiveSupportMove(struct Pokemon *mon, enum Ability ability)
{
    for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
    {
        enum Move move = GetMonData(mon, MON_DATA_MOVE1 + moveIndex);

        if (move == MOVE_NONE || move == MOVE_UNAVAILABLE)
            continue;

        switch (GetMoveEffect(move))
        {
        case EFFECT_REFLECT:
        case EFFECT_LIGHT_SCREEN:
        case EFFECT_MIST:
        case EFFECT_HAZE:
        case EFFECT_POWER_SWAP:
        case EFFECT_GUARD_SWAP:
        case EFFECT_SPEED_SWAP:
            return TRUE;
        case EFFECT_AURORA_VEIL:
            if (PartyMonCanUseAuroraVeil(mon, ability))
                return TRUE;
            break;
        default:
            break;
        }
    }

    return FALSE;
}

static bool32 BattlerHasStatusPressure(enum BattlerId battler)
{
    enum Move *moves;

    if (!IsBattlerAlive(battler))
        return FALSE;

    moves = GetMovesArray(battler);
    for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
    {
        if (MoveHasStatusPressureEffect(moves[moveIndex]))
            return TRUE;
    }

    return FALSE;
}

static bool32 OpposingSideHasStatusOpening(struct SwitchAiContext *switchContext)
{
    enum BattlerId opposingPartner = BATTLE_PARTNER(switchContext->opposingBattler);

    if (gSideStatuses[GetBattlerSide(switchContext->opposingBattler)] & SIDE_STATUS_SAFEGUARD)
        return FALSE;

    if (IsBattlerAlive(switchContext->opposingBattler) && !(gBattleMons[switchContext->opposingBattler].status1 & STATUS1_ANY))
        return TRUE;

    return IsDoubleBattle()
        && IsBattlerAlive(opposingPartner)
        && IsBattlerAlly(switchContext->opposingBattler, opposingPartner)
        && !(gBattleMons[opposingPartner].status1 & STATUS1_ANY);
}

static bool32 OpposingSideCanSpreadStatus(struct SwitchAiContext *switchContext)
{
    enum BattlerId opposingPartner = BATTLE_PARTNER(switchContext->opposingBattler);

    if (BattlerHasStatusPressure(switchContext->opposingBattler))
        return TRUE;

    return IsDoubleBattle()
        && IsBattlerAlive(opposingPartner)
        && IsBattlerAlly(switchContext->opposingBattler, opposingPartner)
        && BattlerHasStatusPressure(opposingPartner);
}

static bool32 PartySideHasStatusToCure(struct SwitchAiContext *switchContext)
{
    enum BattlerId partner = BATTLE_PARTNER(switchContext->battler);

    if (gBattleMons[switchContext->battler].status1 & STATUS1_ANY)
        return TRUE;

    if (IsDoubleBattle()
     && IsBattlerAlive(partner)
     && IsBattlerAlly(switchContext->battler, partner)
     && (gBattleMons[partner].status1 & STATUS1_ANY))
        return TRUE;

    for (u32 monIndex = 0; monIndex < switchContext->lastId; monIndex++)
    {
        if (!IsValidForBattle(&switchContext->party[monIndex]))
            continue;
        if (GetMonData(&switchContext->party[monIndex], MON_DATA_STATUS) & STATUS1_ANY)
            return TRUE;
    }

    return FALSE;
}

static bool32 IncomingMoveCanApplyStatus(enum Move move)
{
    if (move == MOVE_NONE || move == MOVE_UNAVAILABLE)
        return FALSE;

    switch (GetMoveNonVolatileStatus(move))
    {
    case MOVE_EFFECT_SLEEP:
    case MOVE_EFFECT_POISON:
    case MOVE_EFFECT_TOXIC:
    case MOVE_EFFECT_BURN:
    case MOVE_EFFECT_PARALYSIS:
    case MOVE_EFFECT_FROSTBITE:
        return TRUE;
    default:
        break;
    }

    return MoveHasAdditionalEffect(move, MOVE_EFFECT_SLEEP)
        || MoveHasAdditionalEffect(move, MOVE_EFFECT_POISON)
        || MoveHasAdditionalEffect(move, MOVE_EFFECT_TOXIC)
        || MoveHasAdditionalEffect(move, MOVE_EFFECT_BURN)
        || MoveHasAdditionalEffect(move, MOVE_EFFECT_PARALYSIS)
        || MoveHasAdditionalEffect(move, MOVE_EFFECT_FREEZE_OR_FROSTBITE)
        || MoveHasAdditionalEffect(move, MOVE_EFFECT_FROSTBITE)
        || GetMoveEffect(move) == EFFECT_YAWN;
}

static bool32 PartyMonBenefitsFromStatusPlan(struct Pokemon *mon, enum Ability ability, bool32 incomingStatusThreat)
{
    enum Item item = GetMonData(mon, MON_DATA_HELD_ITEM);
    enum HoldEffect holdEffect = GetItemHoldEffect(item);
    bool32 selfStatusItem = holdEffect == HOLD_EFFECT_FLAME_ORB || holdEffect == HOLD_EFFECT_TOXIC_ORB;

    if (!incomingStatusThreat && !selfStatusItem)
        return FALSE;

    switch (ability)
    {
    case ABILITY_GUTS:
        return PartyMonHasMoveWithCategory(mon, DAMAGE_CATEGORY_PHYSICAL);
    case ABILITY_QUICK_FEET:
    case ABILITY_MARVEL_SCALE:
    case ABILITY_MAGIC_GUARD:
        return TRUE;
    case ABILITY_POISON_HEAL:
        return incomingStatusThreat || holdEffect == HOLD_EFFECT_TOXIC_ORB;
    case ABILITY_TOXIC_BOOST:
        return (incomingStatusThreat || holdEffect == HOLD_EFFECT_TOXIC_ORB)
            && PartyMonHasMoveWithCategory(mon, DAMAGE_CATEGORY_PHYSICAL);
    case ABILITY_FLARE_BOOST:
        return (incomingStatusThreat || holdEffect == HOLD_EFFECT_FLAME_ORB)
            && PartyMonHasMoveWithCategory(mon, DAMAGE_CATEGORY_SPECIAL);
    default:
        break;
    }

    return PartyMonHasMoveEffect(mon, EFFECT_FACADE)
        || PartyMonHasMoveEffect(mon, EFFECT_PSYCHO_SHIFT);
}

static bool32 PartyMonHasStatusBoardControl(struct SwitchAiContext *switchContext, struct Pokemon *mon, enum Ability ability)
{
    if (!IsDoubleBattle())
        return FALSE;

    if (PartyMonHasStatusPressureMove(mon) && OpposingSideHasStatusOpening(switchContext))
        return TRUE;

    if (PartyMonHasMoveEffect(mon, EFFECT_HEAL_BELL) && PartySideHasStatusToCure(switchContext))
        return TRUE;

    if ((PartyMonHasMoveEffect(mon, EFFECT_SAFEGUARD) || PartyMonHasMoveEffect(mon, EFFECT_MISTY_TERRAIN))
     && OpposingSideCanSpreadStatus(switchContext))
        return TRUE;

    return PartyMonHasDefensiveSupportMove(mon, ability);
}

static bool32 IsBoardControlAbility(enum Ability ability)
{
    switch (ability)
    {
    case ABILITY_DRIZZLE:
    case ABILITY_DROUGHT:
    case ABILITY_SAND_STREAM:
    case ABILITY_SNOW_WARNING:
    case ABILITY_ELECTRIC_SURGE:
    case ABILITY_GRASSY_SURGE:
    case ABILITY_MISTY_SURGE:
    case ABILITY_PSYCHIC_SURGE:
    case ABILITY_ORICHALCUM_PULSE:
    case ABILITY_HADRON_ENGINE:
    case ABILITY_INTIMIDATE:
    case ABILITY_SWIFT_SWIM:
    case ABILITY_CHLOROPHYLL:
    case ABILITY_SAND_RUSH:
    case ABILITY_SAND_VEIL:
    case ABILITY_SLUSH_RUSH:
    case ABILITY_SURGE_SURFER:
    case ABILITY_WIND_RIDER:
    case ABILITY_WIND_POWER:
    case ABILITY_GUTS:
    case ABILITY_QUICK_FEET:
    case ABILITY_MARVEL_SCALE:
    case ABILITY_MAGIC_GUARD:
    case ABILITY_POISON_HEAL:
    case ABILITY_TOXIC_BOOST:
    case ABILITY_FLARE_BOOST:
        return TRUE;
    default:
        return FALSE;
    }
}

static bool32 PartyMonCanBridgeBoardControl(struct SwitchAiContext *switchContext, struct Pokemon *mon, enum Ability ability)
{
    enum BattlerId partner = BATTLE_PARTNER(switchContext->battler);

    if (!PartyMonHasMoveEffect(mon, EFFECT_SKILL_SWAP)
     && !PartyMonHasMoveEffect(mon, EFFECT_ROLE_PLAY)
     && !PartyMonHasMoveEffect(mon, EFFECT_ENTRAINMENT))
        return FALSE;

    if (IsBoardControlAbility(ability))
        return TRUE;

    return IsDoubleBattle()
        && IsBattlerAlive(partner)
        && IsBattlerAlly(switchContext->battler, partner)
        && IsBoardControlAbility(gAiLogicData->abilities[partner]);
}

static bool32 DoesWeatherSetterReplaceBadWeather(enum BattlerId battler, u32 weather)
{
    u32 currentWeather = AI_GetWeather() & B_WEATHER_ANY;

    if (weather == B_WEATHER_NONE || currentWeather == B_WEATHER_NONE || (currentWeather & weather))
        return FALSE;

    return ShouldClearWeather(battler, currentWeather);
}

static bool32 DoesTerrainSetterReplaceBadTerrain(enum BattlerId battler, u32 terrain)
{
    u32 currentTerrain = gFieldStatuses & STATUS_FIELD_TERRAIN_ANY;

    if (terrain == 0 || currentTerrain == 0 || (currentTerrain & terrain))
        return FALSE;

    return ShouldClearFieldStatus(battler, currentTerrain);
}

static bool32 SpeedWouldGainFromTailwind(u32 speed, enum BattlerId opposingBattler)
{
    if (!IsBattlerAlive(opposingBattler))
        return FALSE;

    return speed <= gBattleMons[opposingBattler].speed && speed * 2 > gBattleMons[opposingBattler].speed;
}

static bool32 SpeedIsBetterUnderTrickRoom(u32 speed, enum BattlerId opposingBattler)
{
    if (!IsBattlerAlive(opposingBattler))
        return FALSE;

    return speed < gBattleMons[opposingBattler].speed;
}

static bool32 PartyMonBenefitsFromTailwind(struct SwitchAiContext *switchContext, struct Pokemon *mon, enum Ability ability)
{
    enum BattlerId partner = BATTLE_PARTNER(switchContext->battler);
    enum BattlerId opposingPartner = BATTLE_PARTNER(switchContext->opposingBattler);
    u32 speed;

    if (!PartyMonHasMoveEffect(mon, EFFECT_TAILWIND))
        return FALSE;
    if (gSideStatuses[GetBattlerSide(switchContext->battler)] & SIDE_STATUS_TAILWIND)
        return FALSE;
    if ((gFieldStatuses & STATUS_FIELD_TRICK_ROOM) && gFieldTimers.trickRoomTimer > 1)
        return FALSE;

    if (ability == ABILITY_WIND_RIDER || ability == ABILITY_WIND_POWER)
        return TRUE;

    speed = GetMonData(mon, MON_DATA_SPEED);
    if (SpeedWouldGainFromTailwind(speed, switchContext->opposingBattler)
     || SpeedWouldGainFromTailwind(speed, opposingPartner))
        return TRUE;

    if (IsDoubleBattle() && IsBattlerAlive(partner))
    {
        speed = gBattleMons[partner].speed;
        if (SpeedWouldGainFromTailwind(speed, switchContext->opposingBattler)
         || SpeedWouldGainFromTailwind(speed, opposingPartner))
            return TRUE;
    }

    return FALSE;
}

static bool32 PartyMonBenefitsFromTrickRoom(struct SwitchAiContext *switchContext, struct Pokemon *mon)
{
    enum BattlerId partner = BATTLE_PARTNER(switchContext->battler);
    enum BattlerId opposingPartner = BATTLE_PARTNER(switchContext->opposingBattler);
    u32 speed;

    if (!PartyMonHasMoveEffect(mon, EFFECT_TRICK_ROOM))
        return FALSE;
    if ((gFieldStatuses & STATUS_FIELD_TRICK_ROOM) && gFieldTimers.trickRoomTimer > 1)
        return FALSE;
    if (ShouldSetFieldStatus(switchContext->battler, STATUS_FIELD_TRICK_ROOM))
        return TRUE;

    speed = GetMonData(mon, MON_DATA_SPEED);
    if (!SpeedIsBetterUnderTrickRoom(speed, switchContext->opposingBattler)
     && !SpeedIsBetterUnderTrickRoom(speed, opposingPartner))
        return FALSE;

    if (IsDoubleBattle() && IsBattlerAlive(partner))
    {
        speed = gBattleMons[partner].speed;
        if (!SpeedIsBetterUnderTrickRoom(speed, switchContext->opposingBattler)
         && !SpeedIsBetterUnderTrickRoom(speed, opposingPartner))
            return FALSE;
    }

    return TRUE;
}

static bool32 BattlerHasUsableProtectMove(enum BattlerId battler)
{
    for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
    {
        enum Move move = gBattleMons[battler].moves[moveIndex];

        if (!IsMoveUnusable(moveIndex, move, gAiLogicData->moveLimitations[battler])
         && gBattleMons[battler].pp[moveIndex] > 0
         && GetMoveEffect(move) == EFFECT_PROTECT)
            return TRUE;
    }

    return FALSE;
}

static bool32 CanKnownPlayerMoveBreakProtectAgainstBattler(enum BattlerId battler, enum BattlerId opposingBattler, enum Move move)
{
    enum BattlerId chosenTarget;
    enum Gimmick gimmick;
    u32 moveIndex;
    u32 damageThroughProtect;

    if (move == MOVE_NONE || move == MOVE_UNAVAILABLE)
        return FALSE;

    chosenTarget = GetKnownPlayerChosenMoveTarget(opposingBattler);
    if (!CanKnownMoveHitBattlerSlot(opposingBattler, battler, move, chosenTarget))
        return FALSE;

    if (MoveIgnoresProtect(move))
        return TRUE;

    if (GetActiveGimmick(battler) != GIMMICK_DYNAMAX)
    {
        gimmick = gBattleStruct->gimmick.usableGimmick[opposingBattler];
        if ((gimmick == GIMMICK_Z_MOVE || gimmick == GIMMICK_DYNAMAX)
         && IsGimmickSelected(opposingBattler, gimmick))
        {
            moveIndex = GetKnownPlayerChosenMoveIndex(opposingBattler, move);
            if (moveIndex < MAX_MON_MOVES && !IsBattleMoveStatus(move))
            {
                damageThroughProtect = max(1, GetKnownPlayerSelectedMoveDamageForSwitch(opposingBattler, battler, move, moveIndex, AI_DEFENDING) / 4);
                if (damageThroughProtect >= gBattleMons[battler].hp)
                    return TRUE;
            }
        }
    }

    return AI_CanContactBypassProtect(opposingBattler, battler, move);
}

static bool32 IsKnownProtectUnsafeForDoubleBattler(struct SwitchAiContext *switchContext)
{
    enum BattlerId battler = switchContext->battler;

    if (switchContext->incomingMove != MOVE_NONE
     && switchContext->incomingMove != MOVE_UNAVAILABLE)
    {
        if (MoveIgnoresProtect(switchContext->incomingMove))
            return TRUE;
        if (AI_CanContactBypassProtect(switchContext->incomingBattler, battler, switchContext->incomingMove))
            return TRUE;
    }

    if (!(gAiThinkingStruct->aiFlags[battler] & AI_FLAG_READ_PLAYER_MOVE))
        return FALSE;

    for (enum BattlerId opposingBattler = 0; opposingBattler < gBattlersCount; opposingBattler++)
    {
        enum Move move;

        if (IsBattlerAlly(battler, opposingBattler) || !IsBattlerAlive(opposingBattler) || BattlerHasAi(opposingBattler))
            continue;
        if (gChosenActionByBattler[opposingBattler] != B_ACTION_USE_MOVE)
            continue;

        move = gChosenMoveByBattler[opposingBattler];
        if (CanKnownPlayerMoveBreakProtectAgainstBattler(battler, opposingBattler, move))
            return TRUE;
    }

    return FALSE;
}

static bool32 ShouldPreserveDoubleBattlerWithProtect(struct SwitchAiContext *switchContext)
{
    enum BattlerId battler = switchContext->battler;

    if (!IsDoubleBattle())
        return FALSE;

    if (!BattlerHasUsableProtectMove(battler))
        return FALSE;

    if (gBattleMons[battler].status1 & STATUS1_ANY)
        return FALSE;

    if (gBattleMons[battler].volatiles.yawn
     || gBattleMons[battler].volatiles.perishSong
     || gBattleMons[battler].volatiles.cursed
     || gBattleMons[battler].volatiles.nightmare
     || gBattleMons[battler].volatiles.leechSeed
     || gBattleMons[battler].volatiles.infatuation
     || gBattleMons[battler].volatiles.encoreTimer != 0)
        return FALSE;

    if (!IsDoubleBattlerUnderPressure(switchContext))
        return FALSE;

    if (IsKnownProtectUnsafeForDoubleBattler(switchContext))
        return FALSE;

    return TRUE;
}

static bool32 ShouldPivotForBoardControl(struct SwitchAiContext *switchContext, bool32 replacesBadField, bool32 hasImmediateEntryEffect)
{
    if (switchContext->hasImportantStatusMove || switchContext->hasStatRaised)
        return FALSE;

    if (switchContext->battlerGetsOHKOd)
        return TRUE;

    if (replacesBadField)
        return TRUE;

    if (IsDoubleBattle())
    {
        if (hasImmediateEntryEffect)
            return TRUE;

        if (CanDoubleBattlerMakeProgress(switchContext))
            return FALSE;

        if (!IsDoubleBattlerUnderPressure(switchContext))
            return FALSE;

        if (ShouldPreserveDoubleBattlerWithProtect(switchContext))
            return FALSE;

        return TRUE;
    }

    if (!switchContext->canBattlerWin1v1 || switchContext->typeMatchup > UQ_4_12(2.0))
        return TRUE;

    return hasImmediateEntryEffect && !switchContext->hasEffectiveMove;
}

static u32 GetBoardControlSwitchinScore(struct SwitchAiContext *switchContext, u32 monIndex, bool32 *replacesBadField, bool32 *hasImmediateEntryEffect)
{
    struct Pokemon *mon = &switchContext->party[monIndex];
    enum Ability ability = GetPartyMonAbilityForSwitchCalc(switchContext->battler, monIndex, mon);
    u32 score = 0;

    *replacesBadField = FALSE;
    *hasImmediateEntryEffect = FALSE;

    if (!IsNeutralizingGasOnField())
    {
        u32 weather = GetSwitchinWeatherFromAbility(ability);
        u32 terrain = GetSwitchinTerrainFromAbility(ability);

        if (weather != B_WEATHER_NONE && !(AI_GetWeather() & weather))
        {
            bool32 replacesWeather = DoesWeatherSetterReplaceBadWeather(switchContext->battler, weather);

            if (ShouldSetWeather(switchContext->battler, weather)
             || PartyMonBenefitsFromWeather(mon, ability, weather)
             || replacesWeather)
            {
                score += 5;
                *hasImmediateEntryEffect = TRUE;
                *replacesBadField |= replacesWeather;
            }
        }

        if (terrain != 0 && !(gFieldStatuses & terrain))
        {
            bool32 replacesTerrain = DoesTerrainSetterReplaceBadTerrain(switchContext->battler, terrain);

            if (ShouldSetFieldStatus(switchContext->battler, terrain)
             || PartyMonBenefitsFromTerrain(mon, ability, terrain)
             || replacesTerrain)
            {
                score += 5;
                *hasImmediateEntryEffect = TRUE;
                *replacesBadField |= replacesTerrain;
            }
        }
    }

    if (PartyMonBenefitsFromTailwind(switchContext, mon, ability))
        score += 3;

    if (PartyMonBenefitsFromTrickRoom(switchContext, mon))
        score += 3;

    if (PartyMonBenefitsFromStatusPlan(mon, ability, IncomingMoveCanApplyStatus(switchContext->incomingMove)))
    {
        score += 3;
        *hasImmediateEntryEffect = TRUE;
    }

    if (PartyMonHasStatusBoardControl(switchContext, mon, ability))
        score += 2;

    if (PartyMonCanBridgeBoardControl(switchContext, mon, ability))
        score += 2;

    return score;
}

static u32 FindBoardControlSwitchin(struct SwitchAiContext *switchContext)
{
    u32 bestMonId = PARTY_SIZE;
    u32 bestScore = 0;

    for (u32 monIndex = 0; monIndex < switchContext->lastId; monIndex++)
    {
        u32 score;
        bool32 replacesBadField;
        bool32 hasImmediateEntryEffect;

        if (!(switchContext->eligiblePartyMons & (1u << monIndex)))
            continue;

        score = GetBoardControlSwitchinScore(switchContext, monIndex, &replacesBadField, &hasImmediateEntryEffect);
        if (score == 0)
            continue;
        if (!ShouldPivotForBoardControl(switchContext, replacesBadField, hasImmediateEntryEffect))
            continue;

        if (score > bestScore)
        {
            bestScore = score;
            bestMonId = monIndex;
        }
    }

    return bestMonId;
}

static bool32 ShouldSwitchIfBoardControlBenefit(struct SwitchAiContext *switchContext)
{
    u32 switchinId;

    if (!(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_SMART_SWITCHING))
        return FALSE;

    switchinId = FindBoardControlSwitchin(switchContext);
    if (switchinId == PARTY_SIZE)
        return FALSE;

    return SetSwitchinAndSwitch(switchContext->battler, switchinId);
}

static bool32 ShouldSwitchIfTruant(struct SwitchAiContext *switchContext)
{
    // Switch if mon with truant is bodied by Protect or invulnerability spam
    if (gAiLogicData->abilities[switchContext->battler] == ABILITY_TRUANT
        && IsTruantMonVulnerable(switchContext->battler, switchContext->opposingBattler)
        && gBattleMons[switchContext->battler].volatiles.truantCounter
        && gBattleMons[switchContext->battler].hp >= gBattleMons[switchContext->battler].maxHP / 2
        && gAiLogicData->mostSuitableMonId[switchContext->battler] != PARTY_SIZE)
    {
        if (RandomPercentage(RNG_AI_SWITCH_TRUANT, GetSwitchChance(SHOULD_SWITCH_TRUANT)))
            return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);
    }
    return FALSE;
}

static u32 FindMonWithMoveOfEffectiveness(struct SwitchAiContext *switchContext, uq4_12_t effectiveness)
{
    enum Move move;
    u32 superEffectiveIds = 0;

    // Find a Pokémon in the party that has a super effective move.
    for (u32 monIndex = 0; monIndex < switchContext->lastId; monIndex++)
    {
        if(!(switchContext->eligiblePartyMons & (1u << monIndex)))
            continue;

        for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
        {
            move = GetMonData(&switchContext->party[monIndex], MON_DATA_MOVE1 + moveIndex);
            if (move != MOVE_NONE && AI_GetMoveEffectiveness(move, switchContext->battler, switchContext->opposingBattler) >= effectiveness && GetMovePower(move) != 0)
            {
                superEffectiveIds |= (1u << monIndex);
            }
        }
    }

    if (superEffectiveIds != 0)
        return SetSwitchinAndSwitch(switchContext->battler, GetSwitchinCandidate(superEffectiveIds, switchContext->battler, switchContext->lastId, SWITCH_MID_BATTLE_OPTIONAL));

    return FALSE; // There is not a single Pokémon in the party that has a move with this effectiveness threshold
}

static bool32 CanMoveAffectTarget(struct DamageContext *ctx, u32 moveIndex)
{
    if (ctx->move != MOVE_NONE
        && gAiLogicData->effectiveness[ctx->battlerAtk][ctx->battlerDef][moveIndex] > UQ_4_12(0.0)
        && !AI_CanMoveBeBlockedByTarget(ctx))
        return TRUE;
    return FALSE;
}

static bool32 IsMoveBad(struct DamageContext *ctx, u32 moveIndex)
{
    if (CanMoveAffectTarget(ctx, moveIndex))
        return FALSE;
    if (!ALL_MOVES_BAD_STATUS_MOVES_BAD || GetMovePower(ctx->move) != 0) // If using ALL_MOVES_BAD_STATUS_MOVES_BAD, then need power to be non-zero
        return TRUE;
    return FALSE;
}

static bool32 ShouldSwitchIfAllMovesBad(struct SwitchAiContext *switchContext)
{
    struct DamageContext ctx = {0};
    ctx.battlerAtk = switchContext->battler;
    ctx.battlerDef = switchContext->opposingBattler;
    ctx.abilities[ctx.battlerAtk] = gAiLogicData->abilities[ctx.battlerAtk];
    ctx.abilities[ctx.battlerDef] = gAiLogicData->abilities[ctx.battlerDef];
    ctx.holdEffects[ctx.battlerAtk] = gAiLogicData->holdEffects[ctx.battlerAtk];
    ctx.holdEffects[ctx.battlerDef] = gAiLogicData->holdEffects[ctx.battlerDef];

    // Switch if no moves affect opponents
    if (IsDoubleBattle())
    {
        enum BattlerId opposingPartner = BATTLE_PARTNER(switchContext->opposingBattler);
        for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
        {
            ctx.move = ctx.chosenMove = gBattleMons[switchContext->battler].moves[moveIndex];
            ctx.moveType = GetBattleMoveType(ctx.move);
            // Check if move is bad in the context of both opposing battlers
            if (!IsMoveBad(&ctx, moveIndex))
            {
                return FALSE;
            }
            else
            {
                // Set partner data in ctx
                ctx.battlerDef = opposingPartner;
                ctx.abilities[ctx.battlerDef] = gAiLogicData->abilities[ctx.battlerDef];
                ctx.holdEffects[ctx.battlerDef] = gAiLogicData->holdEffects[ctx.battlerDef];
                if (!IsMoveBad(&ctx, moveIndex))
                    return FALSE;
            }
        }
    }
    else
    {
        for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
        {
            ctx.move = ctx.chosenMove = gBattleMons[switchContext->battler].moves[moveIndex];
            ctx.moveType = GetBattleMoveType(ctx.move);
            if (!IsMoveBad(&ctx, moveIndex))
                return FALSE;
        }
    }

    if (RandomPercentage(RNG_AI_SWITCH_ALL_MOVES_BAD, GetSwitchChance(SHOULD_SWITCH_ALL_MOVES_BAD))
        && (gAiLogicData->mostSuitableMonId[switchContext->battler] != PARTY_SIZE || !ALL_MOVES_BAD_NEEDS_GOOD_SWITCHIN))
    {
        if (ShouldPreserveDoubleBattlerWithProtect(switchContext))
            return FALSE;

        if (gAiLogicData->mostSuitableMonId[switchContext->battler] == PARTY_SIZE) // No good candidate mons, find any one that can deal damage
            return FindMonWithMoveOfEffectiveness(switchContext, UQ_4_12(1.0));
        else // Good candidate mon, send that in
            return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);
    }

    return FALSE;
}

static bool32 ShouldSwitchIfWonderGuard(struct SwitchAiContext *switchContext)
{
    if (IsDoubleBattle())
        return FALSE;

    if (gAiLogicData->abilities[switchContext->opposingBattler] != ABILITY_WONDER_GUARD)
        return FALSE;

    // Check if Pokémon has a super effective move.
    if (CanUseSuperEffectiveMoveAgainstOpponent(switchContext->battler, switchContext->opposingBattler))
        return FALSE;

    if (RandomPercentage(RNG_AI_SWITCH_WONDER_GUARD, GetSwitchChance(SHOULD_SWITCH_WONDER_GUARD)))
    {
        if (gAiLogicData->mostSuitableMonId[switchContext->battler] == PARTY_SIZE) // No good candidate mons, find any one that can deal damage
            return FindMonWithMoveOfEffectiveness(switchContext, UQ_4_12(2.0));
        else // Good candidate mon, send that in
            return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);
    }

    return FALSE;
}

static bool32 IsReadPerishSongIncoming(struct SwitchAiContext *switchContext)
{
    if (gAiLogicData->abilities[switchContext->battler] == ABILITY_SOUNDPROOF)
        return FALSE;

    if (GetMoveEffect(switchContext->incomingMove) == EFFECT_PERISH_SONG)
        return TRUE;

    if (!(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_READ_PLAYER_MOVE))
        return FALSE;

    for (enum BattlerId opposingBattler = 0; opposingBattler < gBattlersCount; opposingBattler++)
    {
        if (IsBattlerAlly(switchContext->battler, opposingBattler) || !IsBattlerAlive(opposingBattler) || BattlerHasAi(opposingBattler))
            continue;
        if (gChosenActionByBattler[opposingBattler] != B_ACTION_USE_MOVE)
            continue;
        if (GetMoveEffect(gChosenMoveByBattler[opposingBattler]) == EFFECT_PERISH_SONG)
            return TRUE;
    }

    return FALSE;
}

static bool32 CanStopReadPerishSongWithCleanMove(struct SwitchAiContext *switchContext)
{
    if (AiExpectsToFaintPlayer(switchContext->battler))
        return TRUE;

    for (enum BattlerId opposingBattler = 0; opposingBattler < gBattlersCount; opposingBattler++)
    {
        if (IsBattlerAlly(switchContext->battler, opposingBattler) || !IsBattlerAlive(opposingBattler) || BattlerHasAi(opposingBattler))
            continue;
        if (gChosenActionByBattler[opposingBattler] != B_ACTION_USE_MOVE)
            continue;
        if (GetMoveEffect(gChosenMoveByBattler[opposingBattler]) != EFFECT_PERISH_SONG)
            continue;

        for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
        {
            enum Move aiMove = gBattleMons[switchContext->battler].moves[moveIndex];

            if (aiMove == MOVE_NONE || aiMove == MOVE_UNAVAILABLE)
                continue;
            if (CanIndexMoveFaintTarget(switchContext->battler, opposingBattler, moveIndex, AI_ATTACKING)
             && AI_IsFaster(switchContext->battler, opposingBattler, aiMove, gChosenMoveByBattler[opposingBattler], CONSIDER_PRIORITY))
                return TRUE;
        }
    }

    return FALSE;
}

static bool32 FindSoundproofSwitchinForReadPerishSong(struct SwitchAiContext *switchContext)
{
    if (!(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_SMART_SWITCHING))
        return FALSE;
    if (!IsReadPerishSongIncoming(switchContext))
        return FALSE;
    if (CanStopReadPerishSongWithCleanMove(switchContext))
        return FALSE;

    for (u32 monIndex = 0; monIndex < switchContext->lastId; monIndex++)
    {
        enum Ability partyMonAbility;

        if (!(switchContext->eligiblePartyMons & (1u << monIndex)))
            continue;

        partyMonAbility = GetPartyMonAbilityForSwitchCalc(switchContext->battler, monIndex, &switchContext->party[monIndex]);
        if (partyMonAbility == ABILITY_SOUNDPROOF)
        {
            if (gAiBattleData != NULL)
                gAiBattleData->decisionReason[switchContext->battler] = AI_DECISION_REASON_PERISH_ESCAPE;
            return SetSwitchinAndSwitch(switchContext->battler, monIndex);
        }
    }

    return FALSE;
}

static bool32 IsKnownPlayerSoundMovePressureTargetingBattler(enum BattlerId battler, enum BattlerId opposingBattler, enum Move move)
{
    enum MoveTarget moveTarget;
    enum BattlerId chosenTarget;

    if (!IsSoundMove(move) || IsBattleMoveStatus(move))
        return FALSE;

    moveTarget = AI_GetBattlerMoveTargetType(opposingBattler, move);
    if (moveTarget == TARGET_FIELD || moveTarget == TARGET_OPPONENTS_FIELD)
        return FALSE;

    chosenTarget = GetKnownPlayerChosenMoveTarget(opposingBattler);
    return CanKnownMoveHitBattlerSlot(opposingBattler, battler, move, chosenTarget);
}

static bool32 GetKnownPlayerReadSoundPressure(struct SwitchAiContext *switchContext, enum BattlerId *soundBattler, enum Move *soundMove, u32 *soundMoveIndex, u32 *soundDamage, bool32 allowNonlethalPressure)
{
    bool32 found = FALSE;
    u32 bestDamage = 0;

    if (!(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_READ_PLAYER_MOVE) || !BattlerHasAi(switchContext->battler))
        return FALSE;
    if (gAiLogicData->abilities[switchContext->battler] == ABILITY_SOUNDPROOF)
        return FALSE;

    for (enum BattlerId opposingBattler = 0; opposingBattler < gBattlersCount; opposingBattler++)
    {
        enum Move move;
        u32 moveIndex;
        u32 damage;

        if (IsBattlerAlly(switchContext->battler, opposingBattler) || !IsBattlerAlive(opposingBattler) || BattlerHasAi(opposingBattler))
            continue;
        if (gChosenActionByBattler[opposingBattler] != B_ACTION_USE_MOVE)
            continue;

        move = gChosenMoveByBattler[opposingBattler];
        if (move == MOVE_NONE || move == MOVE_UNAVAILABLE)
            continue;
        if (!IsKnownPlayerSoundMovePressureTargetingBattler(switchContext->battler, opposingBattler, move))
            continue;

        moveIndex = GetKnownPlayerChosenMoveIndex(opposingBattler, move);
        if (moveIndex >= MAX_MON_MOVES)
            continue;

        damage = GetKnownPlayerSelectedMoveDamageForSwitch(opposingBattler, switchContext->battler, move, moveIndex, AI_DEFENDING);
        if (damage < gBattleMons[switchContext->battler].hp
         && (!allowNonlethalPressure || damage * 2 < gBattleMons[switchContext->battler].hp))
            continue;
        if (damage > bestDamage)
        {
            *soundBattler = opposingBattler;
            *soundMove = move;
            *soundMoveIndex = moveIndex;
            if (soundDamage != NULL)
                *soundDamage = damage;
            bestDamage = damage;
            found = TRUE;
        }
    }

    return found;
}

static bool32 CanWinKnownSoundPressureRaceInPlace(struct SwitchAiContext *switchContext, enum BattlerId soundBattler, enum Move soundMove, u32 soundMoveIndex)
{
    u32 hitsToKOAi;

    if (soundMoveIndex >= MAX_MON_MOVES)
        return FALSE;

    hitsToKOAi = GetNoOfHitsToKOBattler(soundBattler, switchContext->battler, soundMoveIndex, AI_DEFENDING, CONSIDER_ENDURE);
    if (hitsToKOAi == 0)
        return TRUE;

    for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
    {
        enum Move aiMove = gBattleMons[switchContext->battler].moves[moveIndex];
        u32 hitsToKOSoundBattler;
        bool32 isBattlerFirst;

        if (aiMove == MOVE_NONE || aiMove == MOVE_UNAVAILABLE || IsBattleMoveStatus(aiMove))
            continue;
        if (gBattleMons[switchContext->battler].pp[moveIndex] == 0 || AI_DoesChoiceEffectBlockMove(switchContext->battler, aiMove))
            continue;

        hitsToKOSoundBattler = GetNoOfHitsToKOBattler(switchContext->battler, soundBattler, moveIndex, AI_ATTACKING, CONSIDER_ENDURE);
        if (hitsToKOSoundBattler == 0)
            continue;

        isBattlerFirst = AI_IsFaster(switchContext->battler, soundBattler, aiMove, soundMove, CONSIDER_PRIORITY);
        if (hitsToKOSoundBattler < hitsToKOAi || (hitsToKOSoundBattler == hitsToKOAi && isBattlerFirst))
            return TRUE;
    }

    return FALSE;
}

static bool32 PartyMonHasSoundproofSwitchValue(struct SwitchAiContext *switchContext, struct Pokemon *mon, enum Ability ability)
{
    return PartyMonHasMoveWithCategory(mon, DAMAGE_CATEGORY_PHYSICAL)
        || PartyMonHasMoveWithCategory(mon, DAMAGE_CATEGORY_SPECIAL)
        || PartyMonHasStatusBoardControl(switchContext, mon, ability)
        || PartyMonHasMoveEffect(mon, EFFECT_TAILWIND)
        || PartyMonHasMoveEffect(mon, EFFECT_TRICK_ROOM)
        || PartyMonHasMoveEffect(mon, EFFECT_ENCORE)
        || PartyMonHasMoveEffect(mon, EFFECT_FIRST_TURN_ONLY)
        || PartyMonHasMoveEffect(mon, EFFECT_HIT_ESCAPE)
        || PartyMonHasMoveEffect(mon, EFFECT_PARTING_SHOT);
}

static bool32 FindSoundproofSwitchinForReadSoundPressure(struct SwitchAiContext *switchContext)
{
    enum BattlerId soundBattler = MAX_BATTLERS_COUNT;
    enum Move soundMove = MOVE_NONE;
    u32 soundMoveIndex = MAX_MON_MOVES;
    u32 soundDamage = 0;

    if (!(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_SMART_SWITCHING))
        return FALSE;
    if (!GetKnownPlayerReadSoundPressure(switchContext, &soundBattler, &soundMove, &soundMoveIndex, &soundDamage, TRUE))
        return FALSE;
    if (CanWinKnownSoundPressureRaceInPlace(switchContext, soundBattler, soundMove, soundMoveIndex))
        return FALSE;

    for (u32 monIndex = 0; monIndex < switchContext->lastId; monIndex++)
    {
        enum Ability partyMonAbility;

        if (!(switchContext->eligiblePartyMons & (1u << monIndex)))
            continue;

        partyMonAbility = GetPartyMonAbilityForSwitchCalc(switchContext->battler, monIndex, &switchContext->party[monIndex]);
        if (partyMonAbility == ABILITY_SOUNDPROOF
         && (soundDamage >= gBattleMons[switchContext->battler].hp
          || PartyMonHasSoundproofSwitchValue(switchContext, &switchContext->party[monIndex], partyMonAbility)))
        {
            if (gAiBattleData != NULL)
                gAiBattleData->decisionReason[switchContext->battler] = AI_DECISION_REASON_KNOWN_COMMAND_ANSWER;
            return SetSwitchinAndSwitch(switchContext->battler, monIndex);
        }
    }

    return FALSE;
}

static bool32 FindMonThatAbsorbsOpponentsMove(struct SwitchAiContext *switchContext)
{
    u8 numAbsorbingAbilities = 0;
    enum Ability absorbingTypeAbilities[8]; // Max needed for type + move property absorbers
    enum Ability partyMonAbility;
    enum Type incomingType = CheckDynamicMoveType(GetBattlerMon(switchContext->opposingBattler), switchContext->incomingMove, switchContext->opposingBattler, MON_IN_BATTLE);

    if (!(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_SMART_SWITCHING))
        return FALSE;
    if (GetMoveEffect(switchContext->incomingMove) == EFFECT_HIDDEN_POWER && RandomPercentage(RNG_AI_SWITCH_ABSORBING_HIDDEN_POWER, SHOULD_SWITCH_ABSORBS_HIDDEN_POWER_PERCENTAGE))
        return FALSE;
    if (gBattleStruct->prevTurnSpecies[switchContext->battler] != gBattleMons[switchContext->battler].species && !(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_PREDICT_MOVE)) // AI mon has changed, player's behaviour no longer reliable; override this if using AI_FLAG_PREDICT_MOVE
        return FALSE;
    if (CanUseSuperEffectiveMoveAgainstOpponents(switchContext->battler, switchContext->opposingBattler) && (RandomPercentage(RNG_AI_SWITCH_ABSORBING_STAY_IN, STAY_IN_ABSORBING_PERCENTAGE) || gAiLogicData->aiPredictionInProgress))
        return FALSE;
    if (AreStatsRaised(switchContext->battler))
        return FALSE;
    if (IsMoldBreakerTypeAbility(switchContext->opposingBattler, gAiLogicData->abilities[switchContext->opposingBattler]))
        return FALSE;
    if (switchContext->canBattlerWin1v1)
        return FALSE;

    // Create an array of possible absorb abilities so the AI considers all of them
    if (incomingType == TYPE_FIRE)
    {
        absorbingTypeAbilities[numAbsorbingAbilities++] = ABILITY_FLASH_FIRE;
        absorbingTypeAbilities[numAbsorbingAbilities++] = ABILITY_WELL_BAKED_BODY;
    }
    if (incomingType == TYPE_WATER)
    {
        absorbingTypeAbilities[numAbsorbingAbilities++] = ABILITY_WATER_ABSORB;
        absorbingTypeAbilities[numAbsorbingAbilities++] = ABILITY_DRY_SKIN;
        if (GetConfig(B_REDIRECT_ABILITY_IMMUNITY) >= GEN_5)
            absorbingTypeAbilities[numAbsorbingAbilities++] = ABILITY_STORM_DRAIN;
    }
    if (incomingType == TYPE_ELECTRIC)
    {
        absorbingTypeAbilities[numAbsorbingAbilities++] = ABILITY_VOLT_ABSORB;
        absorbingTypeAbilities[numAbsorbingAbilities++] = ABILITY_MOTOR_DRIVE;
        if (GetConfig(B_REDIRECT_ABILITY_IMMUNITY) >= GEN_5)
            absorbingTypeAbilities[numAbsorbingAbilities++] = ABILITY_LIGHTNING_ROD;
    }
    if (incomingType == TYPE_GRASS)
    {
        absorbingTypeAbilities[numAbsorbingAbilities++] = ABILITY_SAP_SIPPER;
    }
    if (incomingType == TYPE_GROUND)
    {
        absorbingTypeAbilities[numAbsorbingAbilities++] = ABILITY_EARTH_EATER;
        absorbingTypeAbilities[numAbsorbingAbilities++] = ABILITY_LEVITATE;
    }
    if (IsSoundMove(switchContext->incomingMove))
    {
        absorbingTypeAbilities[numAbsorbingAbilities++] = ABILITY_SOUNDPROOF;
    }
    if (IsBallisticMove(switchContext->incomingMove))
    {
        absorbingTypeAbilities[numAbsorbingAbilities++] = ABILITY_BULLETPROOF;
    }
    if (IsWindMove(switchContext->incomingMove))
    {
        absorbingTypeAbilities[numAbsorbingAbilities++] = ABILITY_WIND_RIDER;
    }
    if (IsPowderMove(switchContext->incomingMove))
    {
        if (GetConfig(B_POWDER_OVERCOAT) >= GEN_6)
            absorbingTypeAbilities[numAbsorbingAbilities++] = ABILITY_OVERCOAT;
    }
    if (numAbsorbingAbilities == 0)
    {
        return FALSE;
    }

    // Check current mon for all absorbing abilities
    for (u32 absorbingAbilityIndex = 0; absorbingAbilityIndex < numAbsorbingAbilities; absorbingAbilityIndex++)
    {
        if (gAiLogicData->abilities[switchContext->battler] == absorbingTypeAbilities[absorbingAbilityIndex])
            return FALSE;
    }

    // Check party for mon with ability that absorbs move
    for (u32 monIndex = 0; monIndex < switchContext->lastId; monIndex++)
    {
        if (!(switchContext->eligiblePartyMons & (1u << monIndex)))
            continue;

        partyMonAbility = GetPartyMonAbilityForSwitchCalc(switchContext->battler, monIndex, &switchContext->party[monIndex]);

        for (u32 absorbingAbilityIndex = 0; absorbingAbilityIndex < numAbsorbingAbilities; absorbingAbilityIndex++)
        {
            // Found a mon
            if (absorbingTypeAbilities[absorbingAbilityIndex] == partyMonAbility && RandomPercentage(RNG_AI_SWITCH_ABSORBING, GetSwitchChance(SHOULD_SWITCH_ABSORBS_MOVE)))
            {
                if (gAiBattleData != NULL && partyMonAbility == ABILITY_SOUNDPROOF && GetMoveEffect(switchContext->incomingMove) == EFFECT_PERISH_SONG)
                    gAiBattleData->decisionReason[switchContext->battler] = AI_DECISION_REASON_PERISH_ESCAPE;
                return SetSwitchinAndSwitch(switchContext->battler, monIndex);
            }
        }
    }
    return FALSE;
}

// Ideally this is replaced with predicted moves being factored into switchin 1v1 calcs instead, and this can be seen as a "free" switch there; future work :)
static bool32 ShouldSwitchIfOpponentChargingOrInvulnerable(struct SwitchAiContext *switchContext)
{
    enum BattleMoveEffects effect = GetMoveEffect(switchContext->incomingMove);

    if (IsDoubleBattle() || !(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_SMART_SWITCHING))
        return FALSE;

    // Two-turn attacks that charge without entering semi-invulnerable state (e.g. Solar Beam).
    // First turn of Fly/Dive/Bounce/Sky Drop: move is selected this turn but user is not yet semi-invulnerable.
    // Opponent is already semi-invulnerable.
    if (!(IsTwoTurnNotSemiInvulnerableMove(switchContext->opposingBattler, switchContext->incomingMove)
        || ((effect == EFFECT_SEMI_INVULNERABLE || effect == EFFECT_SKY_DROP) && !IsSemiInvulnerable(switchContext->opposingBattler, CHECK_ALL))
        || IsSemiInvulnerable(switchContext->opposingBattler, CHECK_ALL)))
    {
        return FALSE;
    }

    if (switchContext->canBattlerWin1v1)
        return FALSE;

    if (gAiLogicData->mostSuitableMonId[switchContext->battler] != PARTY_SIZE && RandomPercentage(RNG_AI_SWITCH_FREE_TURN, GetSwitchChance(SHOULD_SWITCH_FREE_TURN)))
        return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);

    return FALSE;
}

static bool32 ShouldSwitchIfTrapperInParty(struct SwitchAiContext *switchContext)
{
    enum Ability partyMonAbility;

    // Only use this if AI_FLAG_SMART_SWITCHING is set for the trainer
    if (!(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_SMART_SWITCHING))
        return FALSE;

    // Check if opposing battler is already trapped
    if (IsBattlerTrapped(switchContext->battler, switchContext->opposingBattler))
        return FALSE;

    for (u32 monIndex = 0; monIndex < switchContext->lastId; monIndex++)
    {
        if (!(switchContext->eligiblePartyMons & (1u << monIndex)))
            continue;

        partyMonAbility = GetPartyMonAbilityForSwitchCalc(switchContext->battler, monIndex, &switchContext->party[monIndex]);

        if (AI_CanSwitchinAbilityTrapOpponent(partyMonAbility, switchContext->opposingBattler) || (AI_CanSwitchinAbilityTrapOpponent(gAiLogicData->abilities[switchContext->opposingBattler], switchContext->opposingBattler) && partyMonAbility == ABILITY_TRACE))
        {
            // If mon in slot i is the most suitable switchin candidate, then it's a trapper than wins 1v1
            if (monIndex == gAiLogicData->mostSuitableMonId[switchContext->battler] && RandomPercentage(RNG_AI_SWITCH_TRAPPER, GetSwitchChance(SHOULD_SWITCH_TRAPPER)))
                return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);
        }
    }
    return FALSE;
}

static bool32 ShouldSwitchIfBadlyStatused(struct SwitchAiContext *switchContext)
{
    bool32 switchMon = FALSE;
    enum Ability monAbility = gAiLogicData->abilities[switchContext->battler];
    enum HoldEffect holdEffect = gAiLogicData->holdEffects[switchContext->battler];

    //Perish Song
    if (gBattleMons[switchContext->battler].volatiles.perishSong
        && gBattleMons[switchContext->battler].volatiles.perishSongTimer == 0
        && monAbility != ABILITY_SOUNDPROOF
        && RandomPercentage(RNG_AI_SWITCH_PERISH_SONG, GetSwitchChance(SHOULD_SWITCH_PERISH_SONG)))
        return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);

    if (gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_SMART_SWITCHING)
    {
        //Yawn
        if (gBattleMons[switchContext->battler].volatiles.yawn
            && CanBeSlept(switchContext->battler, switchContext->battler, monAbility, BLOCKED_BY_SLEEP_CLAUSE)
            && gBattleMons[switchContext->battler].hp > gBattleMons[switchContext->battler].maxHP / 3
            && RandomPercentage(RNG_AI_SWITCH_YAWN, GetSwitchChance(SHOULD_SWITCH_YAWN)))
        {
            switchMon = TRUE;

            // If we don't have a good switchin, not worth switching
            if (gAiLogicData->mostSuitableMonId[switchContext->battler] == PARTY_SIZE)
                switchMon = FALSE;

            // Check if Active Pokemon can KO opponent instead of switching
            // Will still fall asleep, but take out opposing Pokemon first
            if (AiExpectsToFaintPlayer(switchContext->battler))
                switchMon = FALSE;

            // Checks to see if active Pokemon can do something against sleep
            if ((monAbility == ABILITY_NATURAL_CURE
                || monAbility == ABILITY_SHED_SKIN
                || monAbility == ABILITY_EARLY_BIRD)
                || holdEffect == (HOLD_EFFECT_CURE_SLP | HOLD_EFFECT_CURE_STATUS)
                || HasMoveWithEffect(switchContext->battler, EFFECT_SLEEP_TALK)
                || (HasMoveWithEffect(switchContext->battler, EFFECT_SNORE) && gAiLogicData->effectiveness[switchContext->battler][switchContext->opposingBattler][GetBattlerMoveIndexWithEffect(switchContext->battler, EFFECT_SNORE)] >= UQ_4_12(1.0))
                || (IsBattlerGrounded(switchContext->battler, monAbility, gAiLogicData->holdEffects[switchContext->battler])
                    && (HasMove(switchContext->battler, MOVE_MISTY_TERRAIN) || HasMove(switchContext->battler, MOVE_ELECTRIC_TERRAIN)))
                )
                switchMon = FALSE;

            // Check if Active Pokemon evasion boosted and might be able to dodge until awake
            if (gBattleMons[switchContext->battler].statStages[STAT_EVASION] > (DEFAULT_STAT_STAGE + 3)
                && gAiLogicData->abilities[switchContext->opposingBattler] != ABILITY_UNAWARE
                && gAiLogicData->abilities[switchContext->opposingBattler] != ABILITY_KEEN_EYE
                && gAiLogicData->abilities[switchContext->opposingBattler] != ABILITY_MINDS_EYE
                && (GetConfig(B_ILLUMINATE_EFFECT) >= GEN_9 && gAiLogicData->abilities[switchContext->opposingBattler] != ABILITY_ILLUMINATE)
                && !gBattleMons[switchContext->battler].volatiles.foresight
                && !gBattleMons[switchContext->battler].volatiles.miracleEye)
                switchMon = FALSE;

            if (switchMon)
                return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);
        }

        // Secondary Damage
        if (monAbility != ABILITY_MAGIC_GUARD
            && !AiExpectsToFaintPlayer(switchContext->battler)
            && gAiLogicData->mostSuitableMonId[switchContext->battler] != PARTY_SIZE)
        {
            //Toxic
            if (((gBattleMons[switchContext->battler].status1 & STATUS1_TOXIC_COUNTER) >= STATUS1_TOXIC_TURN(2))
                && gBattleMons[switchContext->battler].hp >= (gBattleMons[switchContext->battler].maxHP / 3)
                && gAiLogicData->mostSuitableMonId[switchContext->battler] != PARTY_SIZE
                && (switchContext->hasStatRaised ? RandomPercentage(RNG_AI_SWITCH_BADLY_POISONED, GetSwitchChance(SHOULD_SWITCH_BADLY_POISONED_STATS_RAISED)) : RandomPercentage(RNG_AI_SWITCH_BADLY_POISONED, GetSwitchChance(SHOULD_SWITCH_BADLY_POISONED))))
                return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);

            //Cursed
            if (gBattleMons[switchContext->battler].volatiles.cursed
                && (switchContext->hasStatRaised ? RandomPercentage(RNG_AI_SWITCH_CURSED, GetSwitchChance(SHOULD_SWITCH_CURSED_STATS_RAISED)) : RandomPercentage(RNG_AI_SWITCH_CURSED, GetSwitchChance(SHOULD_SWITCH_CURSED))))
                return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);

            //Nightmare
            if (gBattleMons[switchContext->battler].volatiles.nightmare
                && (switchContext->hasStatRaised ? RandomPercentage(RNG_AI_SWITCH_NIGHTMARE, GetSwitchChance(SHOULD_SWITCH_NIGHTMARE_STATS_RAISED)) : RandomPercentage(RNG_AI_SWITCH_NIGHTMARE, GetSwitchChance(SHOULD_SWITCH_NIGHTMARE))))
                return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);

            //Leech Seed
            if (gBattleMons[switchContext->battler].volatiles.leechSeed
                && (switchContext->hasStatRaised ? RandomPercentage(RNG_AI_SWITCH_SEEDED, GetSwitchChance(SHOULD_SWITCH_SEEDED_STATS_RAISED)) : RandomPercentage(RNG_AI_SWITCH_SEEDED, GetSwitchChance(SHOULD_SWITCH_SEEDED))))
                return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);
        }

        // Infatuation
        if (gBattleMons[switchContext->battler].volatiles.infatuation
            && !AiExpectsToFaintPlayer(switchContext->battler)
            && gAiLogicData->mostSuitableMonId[switchContext->battler] != PARTY_SIZE
            && RandomPercentage(RNG_AI_SWITCH_INFATUATION, GetSwitchChance(SHOULD_SWITCH_INFATUATION)))
            return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);
    }

    return FALSE;
}

static bool32 GetHitEscapeTransformState(enum BattlerId battlerAtk, enum Move move)
{
    u32 moveIndex;
    bool32 hasValidTarget = FALSE;
    bool32 isFasterThanAll = TRUE;
    bool32 absorberOnField = FALSE;
    enum Type moveType;

    if (gBattleMons[battlerAtk].species != SPECIES_PALAFIN_ZERO
     || gAiLogicData->abilities[battlerAtk] != ABILITY_ZERO_TO_HERO)
        return FALSE;

    if (GetMoveEffect(move) != EFFECT_HIT_ESCAPE)
        return FALSE;

    moveIndex = GetMoveIndex(battlerAtk, move);
    if (moveIndex >= MAX_MON_MOVES)
        return FALSE;

    moveType = GetBattleMoveType(move);
    if ((moveType == TYPE_WATER && (AI_GetWeather() & B_WEATHER_SUN_PRIMAL))
     || (moveType == TYPE_FIRE && (AI_GetWeather() & B_WEATHER_RAIN_PRIMAL)))
        return FALSE;

    struct DamageContext ctx = {0};
    ctx.aiCalc = TRUE;
    ctx.battlerAtk = battlerAtk;
    ctx.move = ctx.chosenMove = move;
    ctx.moveType = moveType;
    ctx.holdEffects[ctx.battlerAtk] = gAiLogicData->holdEffects[battlerAtk];
    ctx.abilities[ctx.battlerAtk] = gAiLogicData->abilities[battlerAtk];


    for (enum BattlerId battlerDef = 0; battlerDef < gBattlersCount; battlerDef++)
    {
        if (!IsBattlerAlive(battlerDef) || IsBattlerAlly(battlerDef, battlerAtk))
            continue;

        enum Ability abilityDef = AI_GetMoldBreakerSanitizedAbility(
            battlerAtk,
            gAiLogicData->abilities[battlerAtk],
            gAiLogicData->abilities[battlerDef],
            gAiLogicData->holdEffects[battlerDef],
            move
        );

        ctx.battlerDef = battlerDef;
        ctx.holdEffects[ctx.battlerDef] = gAiLogicData->holdEffects[battlerDef];
        ctx.abilities[ctx.battlerDef] = abilityDef;

        if (AI_CanMoveBeBlockedByTarget(&ctx))
        {
            if ((moveType == TYPE_WATER && abilityDef == ABILITY_STORM_DRAIN)
             || (moveType == TYPE_ELECTRIC && abilityDef == ABILITY_LIGHTNING_ROD))
                absorberOnField = TRUE;
            gAiLogicData->effectiveness[battlerAtk][battlerDef][moveIndex] = UQ_4_12(0.0);
            continue;
        }

        if (gAiLogicData->effectiveness[battlerAtk][battlerDef][moveIndex] > UQ_4_12(0.0))
        {
            enum Move predictedMove = GetPredictedMove(battlerAtk, battlerDef, gAiLogicData);

            hasValidTarget = TRUE;
            if (!AI_IsFaster(battlerAtk, battlerDef, move, predictedMove, CONSIDER_PRIORITY))
                isFasterThanAll = FALSE;
        }
    }

    if (absorberOnField || !hasValidTarget)
        return FALSE; // Can't meaningfully use a hit escape move

    return isFasterThanAll;
}

static bool32 IsOpponentPhysicalAttacker(enum BattlerId battler, enum BattlerId opposingBattler)
{
    if (!IsBattlerAlive(opposingBattler))
        return FALSE;

    if (GetBestDmgFromBattler(opposingBattler, battler, AI_DEFENDING) > 0 && HasPhysicalBestMove(opposingBattler, battler, AI_DEFENDING))
        return TRUE;

    enum Move incomingMove = GetIncomingMove(battler, opposingBattler, gAiLogicData);
    return incomingMove != MOVE_NONE
        && incomingMove != MOVE_UNAVAILABLE
        && GetBattleMoveCategory(incomingMove) == DAMAGE_CATEGORY_PHYSICAL;
}

static bool32 CanIntimidateLowerOpponentAtk(enum BattlerId battler, enum BattlerId opposingBattler)
{
    enum Ability abilityDef = gAiLogicData->abilities[opposingBattler];

    // If Attack is already at -2 or lower, repeated Intimidate cycles aren't worth it.
    if (gBattleMons[opposingBattler].statStages[STAT_ATK] <= DEFAULT_STAT_STAGE - 2)
        return FALSE;

    if (gBattleMons[opposingBattler].volatiles.substitute)
        return FALSE;

    if (gAiLogicData->holdEffects[opposingBattler] == HOLD_EFFECT_CLEAR_AMULET)
        return FALSE;

    if (gSideStatuses[GetBattlerSide(opposingBattler)] & SIDE_STATUS_MIST)
        return FALSE;

    if (IS_BATTLER_OF_TYPE(opposingBattler, TYPE_GRASS) && AI_IsAbilityOnSide(opposingBattler, ABILITY_FLOWER_VEIL))
        return FALSE;

    switch (abilityDef)
    {
    case ABILITY_HYPER_CUTTER:
    case ABILITY_CLEAR_BODY:
    case ABILITY_FULL_METAL_BODY:
    case ABILITY_WHITE_SMOKE:
        return FALSE;
    default:
        break;
    }

    if (GetConfig(B_UPDATED_INTIMIDATE) >= GEN_8)
    {
        switch (abilityDef)
        {
        case ABILITY_INNER_FOCUS:
        case ABILITY_SCRAPPY:
        case ABILITY_OWN_TEMPO:
        case ABILITY_OBLIVIOUS:
            return FALSE;
        default:
            break;
        }
    }

    return TRUE;
}

static bool32 ShouldSwitchIfIntimidateBenefit(struct SwitchAiContext *switchContext)
{
    // Keep Intimidate cycling behavior restricted to smart-switching AI
    if (!(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_SMART_SWITCHING))
        return FALSE;

    enum BattlerId opposingPartner = BATTLE_PARTNER(switchContext->opposingBattler);
    bool32 hasValidTarget = FALSE;

    if (IsBattlerAlive(switchContext->opposingBattler))
    {
        enum Ability abilityDef = gAiLogicData->abilities[switchContext->opposingBattler];
        bool32 canLowerAtk = CanIntimidateLowerOpponentAtk(switchContext->battler, switchContext->opposingBattler);

        if (canLowerAtk && (DoesIntimidateRaiseStats(abilityDef) || abilityDef == ABILITY_MIRROR_ARMOR))
            return FALSE;
        if (canLowerAtk && IsOpponentPhysicalAttacker(switchContext->battler, switchContext->opposingBattler))
            hasValidTarget = TRUE;
    }

    if (IsDoubleBattle() && IsBattlerAlive(opposingPartner))
    {
        enum Ability abilityDef = gAiLogicData->abilities[opposingPartner];
        bool32 canLowerAtk = CanIntimidateLowerOpponentAtk(switchContext->battler, opposingPartner);

        if (canLowerAtk && (DoesIntimidateRaiseStats(abilityDef) || abilityDef == ABILITY_MIRROR_ARMOR))
            return FALSE;
        if (canLowerAtk && IsOpponentPhysicalAttacker(switchContext->battler, opposingPartner))
            hasValidTarget = TRUE;
    }

    return hasValidTarget;
}

static bool32 ShouldSwitchIfAbilityBenefit(struct SwitchAiContext *switchContext)
{
    //Check if ability is blocked
    if (gBattleMons[switchContext->battler].volatiles.gastroAcid
        || IsNeutralizingGasOnField())
        return FALSE;

    switch (gAiLogicData->abilities[switchContext->battler])
    {
    case ABILITY_NATURAL_CURE:
        //Attempt to cure bad ailment
        if (gBattleMons[switchContext->battler].status1 & (STATUS1_SLEEP | STATUS1_FREEZE | STATUS1_TOXIC_POISON)
            && gAiLogicData->mostSuitableMonId[switchContext->battler] != PARTY_SIZE
            && (switchContext->hasStatRaised ? RandomPercentage(RNG_AI_SWITCH_NATURAL_CURE, GetSwitchChance(SHOULD_SWITCH_NATURAL_CURE_STRONG_STATS_RAISED)) : RandomPercentage(RNG_AI_SWITCH_NATURAL_CURE, GetSwitchChance(SHOULD_SWITCH_NATURAL_CURE_STRONG))))
            break;
        //Attempt to cure lesser ailment
        if ((gBattleMons[switchContext->battler].status1 & STATUS1_ANY)
            && (gBattleMons[switchContext->battler].hp >= gBattleMons[switchContext->battler].maxHP / 2)
            && gAiLogicData->mostSuitableMonId[switchContext->battler] != PARTY_SIZE
            && (switchContext->hasStatRaised ? RandomPercentage(RNG_AI_SWITCH_NATURAL_CURE, GetSwitchChance(SHOULD_SWITCH_NATURAL_CURE_WEAK_STATS_RAISED)) : RandomPercentage(RNG_AI_SWITCH_NATURAL_CURE, GetSwitchChance(SHOULD_SWITCH_NATURAL_CURE_WEAK))))
            break;

        return FALSE;

    case ABILITY_REGENERATOR:
        //Don't switch if ailment
        if (gBattleMons[switchContext->battler].status1 & STATUS1_ANY)
            return FALSE;
        if ((gBattleMons[switchContext->battler].hp <= ((gBattleMons[switchContext->battler].maxHP * 2) / 3))
             && gAiLogicData->mostSuitableMonId[switchContext->battler] != PARTY_SIZE
             && (switchContext->hasStatRaised ? RandomPercentage(RNG_AI_SWITCH_REGENERATOR, GetSwitchChance(SHOULD_SWITCH_REGENERATOR_STATS_RAISED)) : RandomPercentage(RNG_AI_SWITCH_REGENERATOR, GetSwitchChance(SHOULD_SWITCH_REGENERATOR))))
            break;

        return FALSE;

    case ABILITY_INTIMIDATE:
        // TODO: In ShouldSwitch cleanup, gate Intimidate cycling behind "stay in instead if the current mon wins the 1v1" to avoid duplicating Bad Odds logic here.
        if (ShouldSwitchIfIntimidateBenefit(switchContext)
            && gAiLogicData->mostSuitableMonId[switchContext->battler] != PARTY_SIZE
            && (switchContext->hasStatRaised ? RandomPercentage(RNG_AI_SWITCH_INTIMIDATE, GetSwitchChance(SHOULD_SWITCH_INTIMIDATE_STATS_RAISED)) : RandomPercentage(RNG_AI_SWITCH_INTIMIDATE, GetSwitchChance(SHOULD_SWITCH_INTIMIDATE))))
            break;

        return FALSE;

    case ABILITY_ZERO_TO_HERO:
    {
        enum Move hitEscapeMove = MOVE_NONE;

        if (GetBattlerMoveIndexWithEffect(switchContext->battler, EFFECT_HIT_ESCAPE) < MAX_MON_MOVES)
            hitEscapeMove = gBattleMons[switchContext->battler].moves[GetBattlerMoveIndexWithEffect(switchContext->battler, EFFECT_HIT_ESCAPE)];

        // Prefer to use a hit escape move if Palafin will move first and can hit
        if (hitEscapeMove != MOVE_NONE && GetHitEscapeTransformState(switchContext->battler, hitEscapeMove))
            return FALSE;
        break;
    }

    default:
        return FALSE;
    }

    return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);
}

// Consider switching to pass Wish to a teammate that benefits from the heal
static bool32 ShouldSwitchIfWishPassing(struct SwitchAiContext *switchContext)
{
    // Only use with smart switching flag
    if (!(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_SMART_SWITCHING))
        return FALSE;

    // Only singles for now (consistent with ShouldSwitchIfHasBadOdds)
    if (IsDoubleBattle())
        return FALSE;

    // Check if Wish is active for this battler's position
    if (gBattleStruct->wish[switchContext->battler].counter == 0)
        return FALSE;

    if (switchContext->incomingMove != MOVE_NONE && GetMoveEffect(switchContext->incomingMove) == EFFECT_HEAL_BLOCK)
        return FALSE;

    // Current mon has good or neutral matchup - no need to switch for Wish
    if (switchContext->typeMatchup <= UQ_4_12(2.0))
        return FALSE;

    // Current mon wins 1v1 - no need to switch for Wish
    if (switchContext->canBattlerWin1v1)
        return FALSE;

    if (!DoesMostSuitableSwitchinBenefitFromWish(switchContext->battler))
        return FALSE;

    if (RandomPercentage(RNG_AI_SWITCH_WISH_PASSING, GetSwitchChance(SHOULD_SWITCH_WISH_PASSING)))
        return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);

    return FALSE;
}

static bool32 CanUseSuperEffectiveMoveAgainstOpponent(enum BattlerId battler, enum BattlerId opposingBattler)
{
    enum Move move;

    for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
    {
        move = gBattleMons[battler].moves[moveIndex];
        if (move == MOVE_NONE || AI_DoesChoiceEffectBlockMove(battler, move))
            continue;

        if (gAiLogicData->effectiveness[battler][opposingBattler][moveIndex] >= UQ_4_12(2.0))
            return TRUE;
    }
    return FALSE;
}

static bool32 CanUseSuperEffectiveMoveAgainstOpponents(enum BattlerId battler, enum BattlerId opposingBattler)
{
    if (CanUseSuperEffectiveMoveAgainstOpponent(battler, opposingBattler))
        return TRUE;

    if (IsDoubleBattle() && CanUseSuperEffectiveMoveAgainstOpponent(battler, BATTLE_PARTNER(opposingBattler)))
        return TRUE;

    return FALSE;
}

static bool32 CanMonSurviveHazardSwitchin(struct SwitchAiContext *switchContext)
{
    u32 hazardDamage = 0, battlerHp = gBattleMons[switchContext->battler].hp;
    enum Ability ability = gAiLogicData->abilities[switchContext->battler];
    enum Move aiMove;

    if (ability == ABILITY_REGENERATOR)
        battlerHp = (battlerHp * 133) / 100; // Account for Regenerator healing

    hazardDamage = GetSwitchinHazardsDamage(switchContext->battler);

    // Battler will faint to hazards, check to see if another mon can clear them
    if (hazardDamage > battlerHp)
    {
        for (u32 monIndex = 0; monIndex < switchContext->lastId; monIndex++)
        {
            if (!(switchContext->eligiblePartyMons & (1u << monIndex)))
                continue;

            for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
            {
                aiMove = GetMonData(&switchContext->party[monIndex], MON_DATA_MOVE1 + moveIndex);
                if (IsHazardClearingMove(aiMove)) // Have a mon that can clear the hazards, so switching out is okay
                    return TRUE;
            }
        }
        // Faints to hazards and party can't clear them, don't switch out
        return FALSE;
    }
    return TRUE;
}

static bool32 ShouldSwitchIfEncored(struct SwitchAiContext *switchContext)
{
    enum Move encoredMove = gBattleMons[switchContext->battler].volatiles.encoredMove;

    // Only use this if AI_FLAG_SMART_SWITCHING is set for the trainer
    if (!(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_SMART_SWITCHING))
        return FALSE;

    // If not Encore'd don't switch
    if (encoredMove == MOVE_NONE)
        return FALSE;

    // Switch out if status move
    if (GetMoveCategory(encoredMove) == DAMAGE_CATEGORY_STATUS && RandomPercentage(RNG_AI_SWITCH_ENCORE, GetSwitchChance(SHOULD_SWITCH_ENCORE_STATUS)))
        return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);

    // Stay in if effective move
    else if (gAiLogicData->effectiveness[switchContext->battler][switchContext->opposingBattler][GetMoveIndex(switchContext->battler, encoredMove)] >= UQ_4_12(2.0))
        return FALSE;

    // Switch out 50% of the time otherwise
    else if ((RandomPercentage(RNG_AI_SWITCH_ENCORE, GetSwitchChance(SHOULD_SWITCH_ENCORE_DAMAGE)) || gAiLogicData->aiPredictionInProgress) && gAiLogicData->mostSuitableMonId[switchContext->battler] != PARTY_SIZE)
        return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);

    return FALSE;
}

static bool32 ShouldSwitchIfBadChoiceLock(struct SwitchAiContext *switchContext)
{
    enum Move choicedMove = gBattleStruct->choicedMove[switchContext->battler];

    struct DamageContext ctx = {0};
    ctx.battlerAtk = switchContext->battler;
    ctx.battlerDef = switchContext->opposingBattler;
    ctx.move = ctx.chosenMove = choicedMove;
    ctx.moveType = GetBattleMoveType(choicedMove);
    ctx.abilities[ctx.battlerAtk] = gAiLogicData->abilities[ctx.battlerAtk];
    ctx.abilities[ctx.battlerDef] = gAiLogicData->abilities[ctx.battlerDef];
    ctx.holdEffects[ctx.battlerAtk] = gAiLogicData->holdEffects[ctx.battlerAtk];
    ctx.holdEffects[ctx.battlerDef] = gAiLogicData->holdEffects[ctx.battlerDef];

    // Not locked in to anything yet, or not choiced
    if (choicedMove == MOVE_NONE)
        return FALSE;

    u32 moveIndex = GetMoveIndex(switchContext->battler, choicedMove);

    if (IsDoubleBattle())
    {
        enum BattlerId opposingPartner = BATTLE_PARTNER(switchContext->opposingBattler);
        if (IsHoldEffectChoice(ctx.holdEffects[ctx.battlerAtk]) && IsBattlerItemEnabled(switchContext->battler))
        {
            if (GetMoveCategory(choicedMove) == DAMAGE_CATEGORY_STATUS || !CanMoveAffectTarget(&ctx, moveIndex))
            {
                // Set partner data in ctx
                ctx.battlerDef = opposingPartner;
                ctx.abilities[ctx.battlerDef] = gAiLogicData->abilities[ctx.battlerDef];
                ctx.holdEffects[ctx.battlerDef] = gAiLogicData->holdEffects[ctx.battlerDef];
                if (!CanMoveAffectTarget(&ctx, moveIndex) && RandomPercentage(RNG_AI_SWITCH_CHOICE_LOCKED, GetSwitchChance(SHOULD_SWITCH_CHOICE_LOCKED)))
                    return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);
            }
        }
    }
    else if (IsHoldEffectChoice(ctx.holdEffects[ctx.battlerAtk]) && IsBattlerItemEnabled(switchContext->battler))
    {
        if ((GetMoveCategory(choicedMove) == DAMAGE_CATEGORY_STATUS || !CanMoveAffectTarget(&ctx, moveIndex)) && RandomPercentage(RNG_AI_SWITCH_CHOICE_LOCKED, GetSwitchChance(SHOULD_SWITCH_CHOICE_LOCKED)))
            return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);
    }

    return FALSE;
}

// AI should switch if it's become setup fodder and has something better to switch to
static bool32 ShouldSwitchIfAttackingStatsLowered(struct SwitchAiContext *switchContext)
{
    s8 attackingStage = gBattleMons[switchContext->battler].statStages[STAT_ATK];
    s8 spAttackingStage = gBattleMons[switchContext->battler].statStages[STAT_SPATK];

    // Only use this if AI_FLAG_SMART_SWITCHING is set for the trainer
    if (!(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_SMART_SWITCHING))
        return FALSE;

    // Physical attacker
    if (gBattleMons[switchContext->battler].attack > gBattleMons[switchContext->battler].spAttack)
    {
        // Don't switch if attack isn't below -1
        if (attackingStage > DEFAULT_STAT_STAGE - 2)
            return FALSE;
        // 50% chance if attack at -2 and have a good candidate mon
        else if (attackingStage == DEFAULT_STAT_STAGE - 2)
        {
            if (gAiLogicData->mostSuitableMonId[switchContext->battler] != PARTY_SIZE && (RandomPercentage(RNG_AI_SWITCH_STATS_LOWERED, GetSwitchChance(SHOULD_SWITCH_ATTACKING_STAT_MINUS_TWO)) || gAiLogicData->aiPredictionInProgress))
                return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);
        }
        // If at -3 or worse, switch out regardless
        else if ((attackingStage < DEFAULT_STAT_STAGE - 2) && RandomPercentage(RNG_AI_SWITCH_STATS_LOWERED, GetSwitchChance(SHOULD_SWITCH_ATTACKING_STAT_MINUS_THREE_PLUS)))
            return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);
    }

    // Special attacker
    else
    {
        // Don't switch if attack isn't below -1
        if (spAttackingStage > DEFAULT_STAT_STAGE - 2)
            return FALSE;
        // 50% chance if attack at -2 and have a good candidate mon
        else if (spAttackingStage == DEFAULT_STAT_STAGE - 2)
        {
            if (gAiLogicData->mostSuitableMonId[switchContext->battler] != PARTY_SIZE && (RandomPercentage(RNG_AI_SWITCH_STATS_LOWERED, GetSwitchChance(SHOULD_SWITCH_ATTACKING_STAT_MINUS_TWO)) || gAiLogicData->aiPredictionInProgress))
                return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);
        }
        // If at -3 or worse, switch out regardless
        else if ((spAttackingStage < DEFAULT_STAT_STAGE - 2) && RandomPercentage(RNG_AI_SWITCH_STATS_LOWERED, GetSwitchChance(SHOULD_SWITCH_ATTACKING_STAT_MINUS_THREE_PLUS)))
            return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);
    }
    return FALSE;
}

bool32 ShouldSwitchIfLoses1v1(struct SwitchAiContext *switchContext)
{
    if (IsDoubleBattle())
        return FALSE;
    if (!(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_SMART_SWITCHING))
        return FALSE;
    if (gAiLogicData->mostSuitableMonId[switchContext->battler] == PARTY_SIZE)
        return FALSE;
    if (!switchContext->canBattlerWin1v1 && RandomPercentage(RNG_AI_SWITCH_LOSES_1V1, GetSwitchChance(SHOULD_SWITCH_LOSES_1V1)))
        return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);
    return FALSE;
}

bool32 ShouldSwitchDynFuncExample(struct SwitchAiContext *switchContext)
{
    // Chance to switch if trainer class is Guitarist, perhaps thematic for Jugglers
    if (GetTrainerClassFromId(TRAINER_BATTLE_PARAM.opponentA) == TRAINER_CLASS_GUITARIST
        && RandomPercentage(RNG_AI_SWITCH_DYN_FUNC, GetSwitchChance(SHOULD_SWITCH_DYN_FUNC)))
    {
        return SetSwitchinAndSwitch(switchContext->battler, PARTY_SIZE);
    }
    return FALSE;
}

static bool32 CanBattlerConsiderSwitch(enum BattlerId battler)
{
    if (gBattleMons[battler].volatiles.wrapped)
        return FALSE;
    if (gBattleMons[battler].volatiles.escapePrevention)
        return FALSE;
    if (gBattleMons[battler].volatiles.root)
        return FALSE;
    if (IsAbilityPreventingEscape(battler))
        return FALSE;
    if (gBattleStruct->battlerState[battler].commanderSpecies)
        return FALSE;
    if (gBattleTypeFlags & BATTLE_TYPE_ARENA)
        return FALSE;
    return TRUE;
}

void GetShouldSwitchMoveData(struct SwitchAiContext *switchContext)
{
    //Variable initialization
    enum Move *playerMoves = GetMovesArray(switchContext->opposingBattler);
    enum Move aiMove, playerMove, bestPlayerPriorityMove = MOVE_NONE, bestPlayerMove = MOVE_NONE, expectedMove = MOVE_NONE;
    enum BattleMoveEffects aiMoveEffect;
    u32 hitsToKOAI = 0, hitsToKOPlayer = 0, minHitsToKOAI = gBattleMons[switchContext->battler].hp, minHitsToKOAIPriority = gBattleMons[switchContext->battler].hp;
    bool32 isBattlerFirst, isBattlerFirstPriority;
    bool32 useKnownSingleMove = ShouldUseKnownIncomingMoveForSingleSwitch(switchContext);

    if (useKnownSingleMove)
    {
        bestPlayerMove = switchContext->incomingMove;
        if (!IsBattleMoveStatus(switchContext->incomingMove) && GetMoveEffect(switchContext->incomingMove) != EFFECT_FOCUS_PUNCH)
        {
            hitsToKOAI = GetNoOfHitsToKOBattler(switchContext->incomingBattler, switchContext->battler, switchContext->incomingMoveIndex, AI_DEFENDING, CONSIDER_ENDURE);
            minHitsToKOAI = hitsToKOAI;
            minHitsToKOAIPriority = 0;
            if (GetBattleMovePriority(switchContext->incomingBattler, gAiLogicData->abilities[switchContext->incomingBattler], switchContext->incomingMove) > 0)
            {
                minHitsToKOAIPriority = hitsToKOAI;
                bestPlayerPriorityMove = switchContext->incomingMove;
            }
        }
        else
        {
            minHitsToKOAI = 0;
            minHitsToKOAIPriority = 0;
        }
    }
    else
    {
        // Get max damage mon could take
        for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
        {
            playerMove = SMART_SWITCHING_OMNISCIENT ? gBattleMons[switchContext->opposingBattler].moves[moveIndex] : playerMoves[moveIndex];
            if (playerMove != MOVE_NONE && !IsBattleMoveStatus(playerMove) && GetMoveEffect(playerMove) != EFFECT_FOCUS_PUNCH && gBattleMons[switchContext->opposingBattler].pp[moveIndex] > 0)
            {
                hitsToKOAI = GetNoOfHitsToKOBattler(switchContext->opposingBattler, switchContext->battler, moveIndex, AI_DEFENDING, CONSIDER_ENDURE);
                if (hitsToKOAI < minHitsToKOAI && !AI_DoesChoiceEffectBlockMove(switchContext->opposingBattler, playerMove))
                {
                    bestPlayerMove = playerMove;
                    minHitsToKOAI = hitsToKOAI;
                }
                if (GetBattleMovePriority(switchContext->opposingBattler, gAiLogicData->abilities[switchContext->opposingBattler], playerMove) > 0 && hitsToKOAI < minHitsToKOAIPriority && !AI_DoesChoiceEffectBlockMove(switchContext->opposingBattler, playerMove))
                {
                    bestPlayerPriorityMove = playerMove;
                    minHitsToKOAIPriority = hitsToKOAI;
                }
            }
        }
    }

    switchContext->battlerGetsOHKOd = minHitsToKOAI == 1 ? TRUE : FALSE;
    expectedMove = gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_PREDICT_MOVE ? GetIncomingMove(switchContext->battler, switchContext->opposingBattler, gAiLogicData) : bestPlayerMove;

    for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
    {
        aiMove = gBattleMons[switchContext->battler].moves[moveIndex];
        aiMoveEffect = GetMoveEffect(aiMove);
        if (aiMove != MOVE_NONE && gBattleMons[switchContext->battler].pp[moveIndex] > 0)
        {
            enum MoveEffect nonVolatileStatus = GetMoveNonVolatileStatus(aiMove);
            // Check if mon has an "important" status move
            if (aiMoveEffect == EFFECT_REFLECT || aiMoveEffect == EFFECT_LIGHT_SCREEN
            || aiMoveEffect == EFFECT_SPIKES || aiMoveEffect == EFFECT_TOXIC_SPIKES || aiMoveEffect == EFFECT_STEALTH_ROCK || aiMoveEffect == EFFECT_STICKY_WEB || aiMoveEffect == EFFECT_LEECH_SEED
            || IsExplosionMove(aiMove)
            || nonVolatileStatus == MOVE_EFFECT_SLEEP
            || nonVolatileStatus == MOVE_EFFECT_TOXIC
            || nonVolatileStatus == MOVE_EFFECT_PARALYSIS
            || nonVolatileStatus == MOVE_EFFECT_BURN
            || aiMoveEffect == EFFECT_YAWN
            || aiMoveEffect == EFFECT_TRICK || aiMoveEffect == EFFECT_TRICK_ROOM || aiMoveEffect== EFFECT_WONDER_ROOM || aiMoveEffect ==  EFFECT_PSYCHO_SHIFT || aiMoveEffect == EFFECT_FIRST_TURN_ONLY)
            {
                switchContext->hasImportantStatusMove = TRUE;
            }

            // Only check damage if it's a damaging move
            if (!IsBattleMoveStatus(aiMove) && !AI_DoesChoiceEffectBlockMove(switchContext->battler, aiMove))
            {
                // Check if mon has a super effective move
                if (gAiLogicData->effectiveness[switchContext->battler][switchContext->opposingBattler][moveIndex] >= UQ_4_12(2.0))
                    switchContext->hasEffectiveMove = TRUE;

                // Check if can win 1v1
                hitsToKOPlayer = GetNoOfHitsToKOBattler(switchContext->battler, switchContext->opposingBattler, moveIndex, AI_SWITCHIN_ATTACKING, CONSIDER_ENDURE);
                if (!switchContext->canBattlerWin1v1 ) // Once we can win a 1v1 we don't need to track this, but want to run the rest of the function to keep the runtime the same regardless of when we find the winning move
                {
                    isBattlerFirst = AI_IsFaster(switchContext->battler, switchContext->opposingBattler, aiMove, expectedMove, CONSIDER_PRIORITY);
                    isBattlerFirstPriority = AI_IsFaster(switchContext->battler, switchContext->opposingBattler, aiMove, bestPlayerPriorityMove, CONSIDER_PRIORITY);
                    switchContext->canBattlerWin1v1 = CanBattlerWin1v1(minHitsToKOAI, hitsToKOPlayer, isBattlerFirst) && CanBattlerWin1v1(minHitsToKOAIPriority, hitsToKOPlayer, isBattlerFirstPriority);
                }
            }
        }
    }
}

void GetShouldSwitchPartyMonEligibility(struct SwitchAiContext *switchContext)
{
    for (u32 monIndex = 0; monIndex < switchContext->lastId; monIndex++)
    {
        if (!IsValidForBattle(&switchContext->party[monIndex]))
            continue;
        if (IsPartyMonOnFieldOrChosenToSwitch(switchContext->battler, monIndex, switchContext->battlerIn1, switchContext->battlerIn2))
            continue;
        if (IsPartyMonPlannedToBeSwitchedInByPartner(monIndex, switchContext->battler))
            continue;
        if (IsAceMon(switchContext->battler, monIndex))
            continue;
        switchContext->eligiblePartyMons |= (1u << monIndex);
    }
}

bool32 ShouldSwitch(enum BattlerId battler)
{
    struct SwitchAiContext switchContext = {0};
    switchContext.battler = battler;
    switchContext.opposingBattler = GetOppositeBattler(switchContext.battler);
    switchContext.party = GetBattlerParty(switchContext.battler);
    switchContext.lastId = GetAILastPartyIndex(switchContext.battler);
    SetKnownIncomingMoveForSwitchContext(&switchContext);
    switchContext.hasStatRaised = AnyUsefulStatIsRaised(switchContext.battler);
    switchContext.typeMatchup = GetBattlerTypeMatchup(switchContext.opposingBattler, switchContext.battler);
    GetActiveBattlerIds(switchContext.battler, &switchContext.battlerIn1, &switchContext.battlerIn2);
    GetShouldSwitchMoveData(&switchContext);
    GetShouldSwitchPartyMonEligibility(&switchContext);

    if (!CanBattlerConsiderSwitch(switchContext.battler))
        return FALSE;

    // Sequence Switching AI never switches mid-battle
    if (gAiThinkingStruct->aiFlags[switchContext.battler] & AI_FLAG_SEQUENCE_SWITCHING)
        return FALSE;

    if (switchContext.eligiblePartyMons == 0)
        return FALSE;

    // custom switching logic
    // NOTE: needs to always end with `return SetSwitchinAndSwitch` or `return FALSE`
    if (gDynamicAiSwitchFunc != NULL && gDynamicAiSwitchFunc(&switchContext)) // Create custom AI functions for specific battles via "setdynamicswitchaifunc" cmd
        return TRUE;

    // NOTE: The sequence of the below functions matter! Do not change unless you have carefully considered the outcome.
    // Since the order is sequential, and some of these functions prompt switch to specific party members.

    // FindMon functions can prompt a switch to specific party members that override GetMostSuitableMonToSwitchInto
    // The rest can prompt a switch to party member returned by GetMostSuitableMonToSwitchInto
    if (ShouldSwitchIfWonderGuard(&switchContext))
        return TRUE;
    if ((gAiThinkingStruct->aiFlags[switchContext.battler] & AI_FLAG_SMART_SWITCHING) && (CanMonSurviveHazardSwitchin(&switchContext) == FALSE))
        return FALSE;
    if (ShouldSwitchIfTrapperInParty(&switchContext))
        return TRUE;
    if (FindSoundproofSwitchinForReadPerishSong(&switchContext))
        return TRUE;
    if (FindSoundproofSwitchinForReadSoundPressure(&switchContext))
        return TRUE;
    if (FindMonThatAbsorbsOpponentsMove(&switchContext))
        return TRUE;
    if (ShouldSwitchIfOpponentChargingOrInvulnerable(&switchContext))
        return TRUE;
    if (ShouldSwitchIfTruant(&switchContext))
        return TRUE;
    if (ShouldSwitchIfPredictedTauntPunish(&switchContext))
        return TRUE;
    if (ShouldStayInForKnownTeraSurvival(&switchContext))
        return FALSE;
    if (ShouldSwitchIfKnownSingleTargetKO(&switchContext))
        return TRUE;
    if (ShouldPreserveDoubleBattlerWithProtect(&switchContext))
        return FALSE;
    if (ShouldSwitchIfKnownFocusedSlotCollapse(&switchContext))
        return TRUE;
    if (ShouldSwitchIfReadChoiceRoleDone(&switchContext))
        return TRUE;
    if (ShouldSwitchIfAllMovesBad(&switchContext))
        return TRUE;
    if (ShouldSwitchIfBadlyStatused(&switchContext))
        return TRUE;
    if (ShouldSwitchIfAbilityBenefit(&switchContext))
        return TRUE;
    if (ShouldSwitchIfBoardControlBenefit(&switchContext))
        return TRUE;
    if (ShouldSwitchIfDoublePositionBad(&switchContext))
        return TRUE;
    if (ShouldSwitchIfWishPassing(&switchContext))
        return TRUE;
    if (ShouldPreserveDoubleBattlerWithProtect(&switchContext))
        return FALSE;
    if (ShouldSwitchIfHasBadOdds(&switchContext))
        return TRUE;
    if (ShouldSwitchIfEncored(&switchContext))
        return TRUE;
    if (ShouldSwitchIfBadChoiceLock(&switchContext))
        return TRUE;
    if (ShouldSwitchIfAttackingStatsLowered(&switchContext))
        return TRUE;
    if (ShouldSwitchIfLoses1v1(&switchContext))
        return TRUE;
    return FALSE;
}

bool32 ShouldSwitchIfAllScoresBad(struct SwitchAiContext *switchContext)
{
    u32 score;
    if (!(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_SMART_SWITCHING))
        return FALSE;

    for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
    {
        if (IsDoubleBattle())
        {
            u32 score1 = gAiBattleData->finalScore[switchContext->battler][switchContext->opposingBattler][moveIndex];
            u32 score2 = gAiBattleData->finalScore[switchContext->battler][BATTLE_PARTNER(switchContext->opposingBattler)][moveIndex];
            if (score1 > AI_BAD_SCORE_THRESHOLD || score2 > AI_BAD_SCORE_THRESHOLD)
                return FALSE;
        }
        else
        {
            score = gAiBattleData->finalScore[switchContext->battler][switchContext->opposingBattler][moveIndex];
            if (score > AI_BAD_SCORE_THRESHOLD)
                return FALSE;
        }
    }
    if (RandomPercentage(RNG_AI_SWITCH_ALL_SCORES_BAD, GetSwitchChance(SHOULD_SWITCH_ALL_SCORES_BAD))
        && (gAiLogicData->mostSuitableMonId[switchContext->battler] != PARTY_SIZE || !ALL_SCORES_BAD_NEEDS_GOOD_SWITCHIN))
        return TRUE;
    return FALSE;
}

bool32 ShouldStayInToUseMove(struct SwitchAiContext *switchContext)
{
    enum Move aiMove;
    enum BattleMoveEffects aiMoveEffect;
    for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
    {
        aiMove = gBattleMons[switchContext->battler].moves[moveIndex];
        aiMoveEffect = GetMoveEffect(aiMove);
        if (aiMoveEffect == EFFECT_REVIVAL_BLESSING || IsSwitchOutEffect(aiMoveEffect))
        {
            // Palafin should not stay in for a hit escape move if it can't use it effectively (slower or no target)
            if (gBattleMons[switchContext->battler].species == SPECIES_PALAFIN_ZERO
             && gAiLogicData->abilities[switchContext->battler] == ABILITY_ZERO_TO_HERO
             && aiMoveEffect == EFFECT_HIT_ESCAPE
             && !GetHitEscapeTransformState(switchContext->battler, aiMove))
                continue;

            if (gAiBattleData->finalScore[switchContext->battler][switchContext->opposingBattler][moveIndex] > AI_GOOD_SCORE_THRESHOLD
                || (IsDoubleBattle() && gAiBattleData->finalScore[switchContext->battler][BATTLE_PARTNER(switchContext->opposingBattler)][moveIndex] > AI_GOOD_SCORE_THRESHOLD))
                return TRUE;
        }
    }
    return FALSE;
}

void ModifySwitchAfterMoveScoring(enum BattlerId battler)
{
    struct SwitchAiContext switchContext = {0};
    switchContext.battler = battler;
    switchContext.opposingBattler = GetOppositeBattler(switchContext.battler);
    switchContext.party = GetBattlerParty(switchContext.battler);
    switchContext.lastId = GetAILastPartyIndex(switchContext.battler);
    GetActiveBattlerIds(switchContext.battler, &switchContext.battlerIn1, &switchContext.battlerIn2);
    GetShouldSwitchPartyMonEligibility(&switchContext);

    if (!CanBattlerConsiderSwitch(battler))
        return;

    // Sequence Switching AI never switches mid-battle
    if (gAiThinkingStruct->aiFlags[battler] & AI_FLAG_SEQUENCE_SWITCHING)
        return;

    if (switchContext.eligiblePartyMons == 0)
        return;

    if (ShouldSwitchIfAllScoresBad(&switchContext))
        gAiLogicData->shouldSwitch |= (1u << battler);
    else if (ShouldStayInToUseMove(&switchContext))
        gAiLogicData->shouldSwitch &= ~(1u << battler);
}

bool32 IsSwitchinValid(enum BattlerId battler)
{
    // Edge case: See if partner already chose to switch into the same mon
    if (IsDoubleBattle())
    {
        enum BattlerId partner = BATTLE_PARTNER(battler);
        if (gBattleStruct->AI_monToSwitchIntoId[battler] == PARTY_SIZE) // Generic switch
        {
            if ((gAiLogicData->shouldSwitch & (1u << partner))
             && gAiLogicData->monToSwitchInId[partner] == gAiLogicData->mostSuitableMonId[battler]
             && BattlersShareParty(battler, partner))
            {
                return FALSE;
            }
        }
        else // Override switch
        {
            if ((gAiLogicData->shouldSwitch & (1u << partner))
             && gAiLogicData->monToSwitchInId[partner] == gAiLogicData->mostSuitableMonId[battler]
             && BattlersShareParty(battler, partner))
            {
                return FALSE;
            }
        }
    }
    return TRUE;
}

static u32 GetSwitchinSingleUseItemHealing(enum BattlerId battler, enum BattlerId opposingBattler, s32 currentHP)
{
    enum Item aiItem = gAiLogicData->items[battler];
    enum Ability ability = gAiLogicData->abilities[battler];
    u32 maxHP = gBattleMons[battler].maxHP;
    s32 itemHeal = 0;
    bool32 berryPocket = GetItemPocket(aiItem) == POCKET_BERRIES;

    // Check if we're at a single use healing item threshold
    if (currentHP <= 0
     || ability == ABILITY_KLUTZ
     || (gAiLogicData->abilities[opposingBattler] == ABILITY_UNNERVE && berryPocket))
        return itemHeal;

    switch (GetItemHoldEffect(aiItem))
    {
    case HOLD_EFFECT_RESTORE_HP:
        if (currentHP <= maxHP / 2 || (berryPocket && ability == ABILITY_GLUTTONY && currentHP <= maxHP / 2))
            itemHeal = GetItemHoldEffectParam(aiItem);
        break;
    case HOLD_EFFECT_RESTORE_PCT_HP:
        if (currentHP <= maxHP / 2 || (berryPocket && ability == ABILITY_GLUTTONY && currentHP <= maxHP / 2))
        {
            itemHeal = max(1, maxHP * GetItemHoldEffectParam(aiItem) / 100);
            if (itemHeal == 0)
                itemHeal = 1;
        }
        break;
    case HOLD_EFFECT_CONFUSE_SPICY:
    case HOLD_EFFECT_CONFUSE_DRY:
    case HOLD_EFFECT_CONFUSE_SWEET:
    case HOLD_EFFECT_CONFUSE_BITTER:
    case HOLD_EFFECT_CONFUSE_SOUR:
        if (currentHP <= maxHP / CONFUSE_BERRY_HP_FRACTION
         || (berryPocket && ability == ABILITY_GLUTTONY && currentHP <= maxHP / 2))
        {
            itemHeal = maxHP / GetItemHoldEffectParam(aiItem);
            if (itemHeal == 0)
                itemHeal = 1;
        }
        break;
    default:
        break;
    }

    if (itemHeal != 0 && berryPocket && ability == ABILITY_RIPEN)
        itemHeal *= 2;

    return itemHeal;
}

u32 Test_GetSwitchinSingleUseItemHealing(enum BattlerId battler, enum BattlerId opposingBattler, s32 currentHP)
{
    return GetSwitchinSingleUseItemHealing(battler, opposingBattler, currentHP);
}

#if TESTING
void Test_GetCombinedDamageRollSummary(const struct SimulatedDamage *damages, u32 damageCount, u32 *minimum, u32 *median, u32 *roll14Of16, u32 *roll15Of16, u32 *maximum)
{
    struct KnownPlayerDamageRollSummary summary;

    SetKnownPlayerDamageRollSummaryFromDistribution(damages, damageCount, &summary);
    if (minimum != NULL)
        *minimum = summary.minimum;
    if (median != NULL)
        *median = summary.median;
    if (roll14Of16 != NULL)
        *roll14Of16 = summary.roll14Of16;
    if (roll15Of16 != NULL)
        *roll15Of16 = summary.roll15Of16;
    if (maximum != NULL)
        *maximum = summary.maximum;
}
#endif

// Gets hazard damage
static u32 GetSwitchinHazardsDamage(enum BattlerId battler)
{
    u8 tSpikesLayers;
    enum HoldEffect heldItemEffect = gAiLogicData->holdEffects[battler];
    u32 maxHP = gBattleMons[battler].maxHP;
    enum Ability ability = gAiLogicData->abilities[battler];
    u32 status = gBattleMons[battler].status1;
    u32 spikesDamage = 0, tSpikesDamage = 0, hazardDamage = 0;
    enum BattleSide side = GetBattlerSide(battler);

    // Check ways mon might avoid all hazards
    if (ability != ABILITY_MAGIC_GUARD || (heldItemEffect == HOLD_EFFECT_HEAVY_DUTY_BOOTS &&
        !((gFieldStatuses & STATUS_FIELD_MAGIC_ROOM) || ability == ABILITY_KLUTZ)))
    {
        // Stealth Rock
        if (IsHazardOnSide(side, HAZARDS_STEALTH_ROCK) && heldItemEffect != HOLD_EFFECT_HEAVY_DUTY_BOOTS)
            hazardDamage += GetStealthHazardDamage(TYPE_SIDE_HAZARD_POINTED_STONES, battler);
        // G-Max Steelsurge
        if (IsHazardOnSide(side, HAZARDS_STEELSURGE) && heldItemEffect != HOLD_EFFECT_HEAVY_DUTY_BOOTS)
            hazardDamage += GetStealthHazardDamage(TYPE_SIDE_HAZARD_SHARP_STEEL, battler);
        // Spikes
        if (IsHazardOnSide(side, HAZARDS_SPIKES) && AI_IsBattlerGrounded(battler))
        {
            spikesDamage = maxHP / ((5 - gSideTimers[GetBattlerSide(battler)].spikesAmount) * 2);
            if (spikesDamage == 0)
                spikesDamage = 1;
            hazardDamage += spikesDamage;
        }

        if (IsHazardOnSide(side, HAZARDS_TOXIC_SPIKES) && (!IS_BATTLER_ANY_TYPE(battler, TYPE_POISON, TYPE_STEEL)
            && ability != ABILITY_IMMUNITY && ability != ABILITY_POISON_HEAL && ability != ABILITY_COMATOSE
            && status == 0
            && !(gSideStatuses[GetBattlerSide(battler)] & SIDE_STATUS_SAFEGUARD)
            && !IsAbilityOnSide(battler, ABILITY_PASTEL_VEIL)
            && !IsMistyTerrainAffected(battler, ability, gAiLogicData->holdEffects[battler], gFieldStatuses)
            && !IsAbilityStatusProtected(battler, ability)
            && heldItemEffect != HOLD_EFFECT_CURE_PSN && heldItemEffect != HOLD_EFFECT_CURE_STATUS
            && AI_IsBattlerGrounded(battler)))
        {
            tSpikesLayers = gSideTimers[GetBattlerSide(battler)].toxicSpikesAmount;
            if (tSpikesLayers == 1)
            {
                tSpikesDamage = maxHP / 8;
                if (tSpikesDamage == 0)
                    tSpikesDamage = 1;
            }
            else if (tSpikesLayers >= 2)
            {
                tSpikesDamage = maxHP / 16;
                if (tSpikesDamage == 0)
                    tSpikesDamage = 1;
            }
            hazardDamage += tSpikesDamage;
        }
    }
    return hazardDamage;
}

// Gets damage / healing from weather
static s32 GetSwitchinWeatherImpact(enum BattlerId battler)
{
    s32 weatherImpact = 0, maxHP = gBattleMons[battler].maxHP;
    enum Ability ability = gAiLogicData->abilities[battler];
    enum HoldEffect holdEffect = gAiLogicData->holdEffects[battler];
    u32 weather = AI_GetSwitchinWeather(battler);

    // Damage
    if (holdEffect != HOLD_EFFECT_SAFETY_GOGGLES && ability != ABILITY_MAGIC_GUARD && ability != ABILITY_OVERCOAT)
    {
        if ((weather  & B_WEATHER_HAIL)
         && !IS_BATTLER_OF_TYPE(battler, TYPE_ICE)
         && ability != ABILITY_SNOW_CLOAK && ability != ABILITY_ICE_BODY)
        {
            weatherImpact = maxHP / 16;
            if (weatherImpact == 0)
                weatherImpact = 1;
        }
        else if ((weather  & B_WEATHER_SANDSTORM)
            && !IS_BATTLER_ANY_TYPE(battler, TYPE_ROCK, TYPE_GROUND, TYPE_STEEL)
            && ability != ABILITY_SAND_VEIL && ability != ABILITY_SAND_RUSH && ability != ABILITY_SAND_FORCE)
        {
            weatherImpact = maxHP / 16;
            if (weatherImpact == 0)
                weatherImpact = 1;
        }
    }
    if ((weather  & B_WEATHER_SUN) && holdEffect != HOLD_EFFECT_UTILITY_UMBRELLA
     && (ability == ABILITY_SOLAR_POWER || ability == ABILITY_DRY_SKIN))
    {
        weatherImpact = maxHP / 8;
        if (weatherImpact == 0)
            weatherImpact = 1;
    }

    // Healing
    if (weather  & B_WEATHER_RAIN && holdEffect != HOLD_EFFECT_UTILITY_UMBRELLA)
    {
        if (ability == ABILITY_DRY_SKIN)
        {
            weatherImpact = -(maxHP / 8);
            if (weatherImpact == 0)
                weatherImpact = -1;
        }
        else if (ability == ABILITY_RAIN_DISH)
        {
            weatherImpact = -(maxHP / 16);
            if (weatherImpact == 0)
                weatherImpact = -1;
        }
    }
    if ((weather & (B_WEATHER_HAIL | B_WEATHER_SNOW)) && ability == ABILITY_ICE_BODY)
    {
        weatherImpact = -(maxHP / 16);
        if (weatherImpact == 0)
            weatherImpact = -1;
    }

    return weatherImpact;
}

// Gets one turn of recurring healing
static u32 GetSwitchinRecurringHealing(enum BattlerId battler)
{
    u32 recurringHealing = 0, maxHP = gBattleMons[battler].maxHP;
    enum Ability ability = gAiLogicData->abilities[battler];
    enum HoldEffect holdEffect = gAiLogicData->holdEffects[battler];

    // Items
    if (ability != ABILITY_KLUTZ)
    {
        if (holdEffect == HOLD_EFFECT_BLACK_SLUDGE && IS_BATTLER_OF_TYPE(battler, TYPE_POISON))
        {
            recurringHealing = maxHP / 16;
            if (recurringHealing == 0)
                recurringHealing = 1;
        }
        else if (holdEffect == HOLD_EFFECT_LEFTOVERS)
        {
            recurringHealing = maxHP / 16;
            if (recurringHealing == 0)
                recurringHealing = 1;
        }
    } // Intentionally omitting Shell Bell for its inconsistency

    // Abilities
    if (ability == ABILITY_POISON_HEAL && (gBattleMons[battler].status1 & STATUS1_POISON))
    {
        u32 healing = maxHP / 8;
        if (healing == 0)
            healing = 1;
        recurringHealing += healing;
    }
    if ((gFieldStatuses & STATUS_FIELD_GRASSY_TERRAIN) && AI_IsBattlerGrounded(battler) && !IsSemiInvulnerable(battler, CHECK_ALL))
    {
        u32 healing = maxHP / 16;
        if (healing == 0)
            healing = 1;
        recurringHealing += healing;
    }
    return recurringHealing;
}

// Gets one turn of recurring damage
static u32 GetSwitchinRecurringDamage(enum BattlerId battler)
{
    u32 passiveDamage = 0, maxHP = gBattleMons[battler].maxHP;
    enum Ability ability = gAiLogicData->abilities[battler];
    enum HoldEffect holdEffect = gAiLogicData->holdEffects[battler];

    // Items
    if (ability != ABILITY_MAGIC_GUARD && ability != ABILITY_KLUTZ)
    {
        if (holdEffect == HOLD_EFFECT_BLACK_SLUDGE && !IS_BATTLER_OF_TYPE(battler, TYPE_POISON))
        {
            passiveDamage = maxHP / 8;
            if (passiveDamage == 0)
                passiveDamage = 1;
        }
        else if (holdEffect == HOLD_EFFECT_LIFE_ORB && ability != ABILITY_SHEER_FORCE)
        {
            passiveDamage = maxHP / 10;
            if (passiveDamage == 0)
                passiveDamage = 1;
        }
        else if (holdEffect == HOLD_EFFECT_STICKY_BARB)
        {
            passiveDamage = maxHP / 8;
            if (passiveDamage == 0)
                passiveDamage = 1;
        }
    }
    return passiveDamage;
}

// Gets one turn of status damage
static u32 GetSwitchinStatusDamage(enum BattlerId battler)
{
    u8 tSpikesLayers = gSideTimers[GetBattlerSide(battler)].toxicSpikesAmount;
    u32 status = gBattleMons[battler].status1;
    enum Ability ability = gAiLogicData->abilities[battler];
    u32 maxHP = gBattleMons[battler].maxHP;
    u32 statusDamage = 0;

    // Status condition damage
    if ((status != 0) && ability != ABILITY_MAGIC_GUARD)
    {
        if (status & STATUS1_BURN)
        {
            if (GetConfig(B_BURN_DAMAGE) >= GEN_7 || GetConfig(B_BURN_DAMAGE) == GEN_1)
                statusDamage = maxHP / 16;
            else
                statusDamage = maxHP / 8;
            if (ability == ABILITY_HEATPROOF)
                statusDamage = statusDamage / 2;
            if (statusDamage == 0)
                statusDamage = 1;
        }
        else if (status & STATUS1_FROSTBITE)
        {
            if (GetConfig(B_BURN_DAMAGE) >= GEN_7 || GetConfig(B_BURN_DAMAGE) == GEN_1)
                statusDamage = maxHP / 16;
            else
                statusDamage = maxHP / 8;
            if (statusDamage == 0)
                statusDamage = 1;
        }
        else if ((status & STATUS1_POISON) && ability != ABILITY_POISON_HEAL)
        {
            statusDamage = maxHP / 8;
            if (statusDamage == 0)
                statusDamage = 1;
        }
        else if ((status & STATUS1_TOXIC_POISON) && ability != ABILITY_POISON_HEAL)
        {
            if ((status & STATUS1_TOXIC_COUNTER) != STATUS1_TOXIC_TURN(15)) // not 16 turns
                gBattleMons[battler].status1 += STATUS1_TOXIC_TURN(1);
            statusDamage = maxHP / 16;
            if (statusDamage == 0)
                statusDamage = 1;
            statusDamage *= gBattleMons[battler].status1 & STATUS1_TOXIC_COUNTER >> 8;
        }
    }

    // Apply hypothetical poisoning from Toxic Spikes, which means the first turn of damage already added in GetSwitchinHazardsDamage
    // Do this last to skip one iteration of Poison / Toxic damage, and start counting Toxic damage one turn later.
    if (tSpikesLayers != 0 && IsSwitchinTSpikesAffected(battler))
    {
        if (tSpikesLayers == 1)
        {
            gBattleMons[battler].status1 = STATUS1_POISON; // Assign "hypothetical" status to the switchin candidate so we can get the damage it would take from TSpikes
        }
        if (tSpikesLayers == 2)
        {
            gBattleMons[battler].status1 = STATUS1_TOXIC_POISON; // Assign "hypothetical" status to the switchin candidate so we can get the damage it would take from TSpikes
            gBattleMons[battler].status1 += STATUS1_TOXIC_TURN(1);
        }
    }
    return statusDamage;
}

// Gets number of hits to KO factoring in hazards, healing held items, status, weather, and incoming heals
static u32 GetSwitchinHitsToKO(s32 damageTaken, enum BattlerId battler, const struct IncomingHealInfo *healInfo, u32 originalHp)
{
    u32 hazardDamage = GetSwitchinHazardsDamage(battler);
    u32 hazardCheckHp = healInfo->healBeforeHazards ? gBattleMons[battler].maxHP : gBattleMons[battler].hp;
    u32 startingHP;

    if (healInfo->healAfterHazards)
    {
        // Heal happens after entry damage
        if (hazardDamage >= originalHp)
            return 1;
        startingHP = gBattleMons[battler].maxHP;
    }
    else
    {
        if (hazardDamage >= hazardCheckHp)
            return 1;
        startingHP = hazardCheckHp - hazardDamage;
    }

    s32 weatherImpact = GetSwitchinWeatherImpact(battler); // Signed to handle both damage and healing in the same value
    u32 recurringDamage = GetSwitchinRecurringDamage(battler);
    u32 recurringHealing = GetSwitchinRecurringHealing(battler);
    u32 statusDamage = GetSwitchinStatusDamage(battler);
    u32 hitsToKO = 0;
    u16 maxHP = gBattleMons[battler].maxHP, item = gAiLogicData->items[battler], heldItemEffect = GetItemHoldEffect(item);
    u8 weatherDuration = gBattleStruct->weatherDuration;
    enum BattlerId opposingBattler = GetOppositeBattler(battler);
    enum Ability opposingAbility = gAiLogicData->abilities[opposingBattler], ability = gAiLogicData->abilities[battler];
    bool32 usedSingleUseHealingItem = FALSE, opponentCanBreakMold = IsMoldBreakerTypeAbility(opposingBattler, opposingAbility);
    s32 currentHP = startingHP, singleUseItemHeal = 0;
    bool32 applyWishNow = healInfo->healEndOfTurn && healInfo->wishCounter == 1;

    // No damage being dealt
    if ((damageTaken + statusDamage + recurringDamage <= recurringHealing) || damageTaken + statusDamage + recurringDamage == 0)
        return hitsToKO;

    // Mon fainted to hazards
    if (startingHP == 0)
        return 1;

    // Find hits to KO
    while (currentHP > 0)
    {
        // Remove weather damage when it would run out
        if (weatherImpact != 0 && weatherDuration == 0)
            weatherImpact = 0;

        // Take attack damage for the turn
        currentHP = currentHP - damageTaken;

        // One shot prevention effects
        if (damageTaken >= maxHP && startingHP == maxHP && (heldItemEffect == HOLD_EFFECT_FOCUS_SASH || (!opponentCanBreakMold && GetConfig(B_STURDY) >= GEN_5 && ability == ABILITY_STURDY)) && hitsToKO < 1)
            currentHP = 1;

        // If mon is still alive, apply weather impact first, as it might KO the mon before it can heal with its item (order is weather -> item -> status)
        if (currentHP > 0)
            currentHP = currentHP - weatherImpact;

        // Check if we're at a single use healing item threshold
        if (usedSingleUseHealingItem == FALSE)
        {
            singleUseItemHeal = GetSwitchinSingleUseItemHealing(battler, opposingBattler, currentHP);
            // If we used one, apply it without overcapping our maxHP
            if (singleUseItemHeal > 0)
            {
                if ((currentHP + singleUseItemHeal) > maxHP)
                    currentHP = maxHP;
                else
                    currentHP = currentHP + singleUseItemHeal;
                usedSingleUseHealingItem = TRUE;
            }
        }

        // Healing from items occurs before status so we can do the rest in one line
        if (currentHP > 0)
            currentHP = currentHP + recurringHealing - recurringDamage - statusDamage;

        // Wish healing happens at the end of the turn when it is due this turn.
        if (applyWishNow && currentHP > 0)
        {
            currentHP += healInfo->healAmount;
            if (currentHP > maxHP)
                currentHP = maxHP;
            applyWishNow = FALSE;
        }

        // Recalculate toxic damage if needed
        if (gBattleMons[battler].status1 & STATUS1_TOXIC_POISON)
            statusDamage = GetSwitchinStatusDamage(battler);

        // Reduce weather duration
        if (weatherDuration != 0)
            weatherDuration--;

        hitsToKO++;
    }

    // Disguise will always add an extra hit to KO
    if (!opponentCanBreakMold && IsMimikyuDisguised(battler))
        hitsToKO++;

    return hitsToKO;
}

static uq4_12_t GetTypeMatchupAgainstTypes(enum BattlerId opposingBattler, enum Type defType1, enum Type defType2)
{
    // Check type matchup
    uq4_12_t typeEffectiveness1 = UQ_4_12(1.0), typeEffectiveness2 = UQ_4_12(1.0);
    enum Type atkType1 = gBattleMons[opposingBattler].types[0], atkType2 = gBattleMons[opposingBattler].types[1];

    // Add each independent defensive type matchup together
    typeEffectiveness1 = uq4_12_multiply(typeEffectiveness1, (GetTypeModifier(atkType1, defType1)));
    if (defType2 != defType1)
        typeEffectiveness1 = uq4_12_multiply(typeEffectiveness1, (GetTypeModifier(atkType1, defType2)));
    if (typeEffectiveness1 == 0) // Immunity
        typeEffectiveness1 = UQ_4_12(0.1);

    if (atkType2 != atkType1)
    {
        typeEffectiveness2 = uq4_12_multiply(typeEffectiveness2, (GetTypeModifier(atkType2, defType1)));
        if (defType2 != defType1)
            typeEffectiveness2 = uq4_12_multiply(typeEffectiveness2, (GetTypeModifier(atkType2, defType2)));
        if (typeEffectiveness2 == 0) // Immunity
            typeEffectiveness2 = UQ_4_12(0.1);
    }
    else
    {
        typeEffectiveness2 = typeEffectiveness1;
    }

    return typeEffectiveness1 + typeEffectiveness2;
}

static uq4_12_t GetBattlerTypeMatchup(enum BattlerId opposingBattler, enum BattlerId battler)
{
    return GetTypeMatchupAgainstTypes(opposingBattler, gBattleMons[battler].types[0], gBattleMons[battler].types[1]);
}

static u32 GetSwitchinCandidate(u32 switchinCategory, enum BattlerId battler, int lastId, enum SwitchType switchType)
{
    if (switchinCategory == 0)
        return PARTY_SIZE;

    // Randomize between eligible mons
    if (gAiThinkingStruct->aiFlags[battler] & AI_FLAG_RANDOMIZE_SWITCHIN)
    {
        // This split is necessary because the test system can't handle multiple calls with the same random tag in the same turn
        if (switchType == SWITCH_AFTER_KO)
            return RandomBitIndex(RNG_AI_RANDOM_SWITCHIN_POST_KO, switchinCategory); // Can't pass this anything with no set bits
        else
            return RandomBitIndex(RNG_AI_RANDOM_SWITCHIN_MID_BATTLE, switchinCategory); // Can't pass this anything with no set bits
    }

    // Pick last eligible mon in party order
    for (s32 monIndex = (lastId-1); monIndex >= 0; monIndex--)
    {
        if (switchinCategory & (1 << monIndex))
            return monIndex;
    }

    return PARTY_SIZE;
}

static u32 GetValidSwitchinCandidate(u32 validMonIds, enum BattlerId battler, u32 lastId, enum SwitchType switchType)
{
    if (validMonIds == 0)
        return PARTY_SIZE;

    // Randomize between valid mons
    if ((gAiThinkingStruct->aiFlags[battler] & AI_FLAG_RANDOMIZE_SWITCHIN) && RANDOMIZE_SWITCHIN_ANY_VALID)
    {
        // This split is necessary because the test system can't handle multiple calls with the same random tag in the same turn
        if (switchType == SWITCH_AFTER_KO)
            return RandomBitIndex(RNG_AI_RANDOM_VALID_SWITCHIN_POST_KO, validMonIds); // Can't pass this anything with no set bits
        else
            return RandomBitIndex(RNG_AI_RANDOM_VALID_SWITCHIN_MID_BATTLE, validMonIds); // Can't pass this anything with no set bits
    }

    // Pick last valid mon in party order
    for (s32 monIndex = (lastId-1); monIndex > 0; monIndex--)
    {
        if (validMonIds & (1 << monIndex))
            return monIndex;
    }

    return PARTY_SIZE;
}

static s32 GetMaxDamagePlayerCouldDealToSwitchin(enum BattlerId battler, enum BattlerId opposingBattler, enum Move *bestPlayerMove)
{
    enum Move playerMove;
    enum Move *playerMoves = GetMovesArray(opposingBattler);
    s32 damageTaken = 0, maxDamageTaken = 0;

    for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
    {
        playerMove = SMART_SWITCHING_OMNISCIENT ? gBattleMons[opposingBattler].moves[moveIndex] : playerMoves[moveIndex];
        if (playerMove != MOVE_NONE && !IsBattleMoveStatus(playerMove) && GetMoveEffect(playerMove) != EFFECT_FOCUS_PUNCH && gBattleMons[opposingBattler].pp[moveIndex] > 0)
        {
            damageTaken = AI_GetDamage(opposingBattler, battler, moveIndex, AI_SWITCHIN_DEFENDING, gAiLogicData);
            if (playerMove == gBattleStruct->choicedMove[opposingBattler]) // If player is choiced, only care about the choice locked move
            {
                *bestPlayerMove = playerMove;
                return damageTaken;
            }
            if (damageTaken > maxDamageTaken)
            {
                maxDamageTaken = damageTaken;
                *bestPlayerMove = playerMove;
            }
        }
    }
    return maxDamageTaken;
}

static s32 GetMaxPriorityDamagePlayerCouldDealToSwitchin(enum BattlerId battler, enum BattlerId opposingBattler, enum Move *bestPlayerPriorityMove)
{
    enum Move playerMove;
    enum Move *playerMoves = GetMovesArray(opposingBattler);
    s32 damageTaken = 0, maxDamageTaken = 0;

    for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
    {
        // If player is choiced into a non-priority move, AI understands that it can't deal priority damage
        if (gBattleStruct->choicedMove[opposingBattler] != MOVE_NONE && GetMovePriority(gBattleStruct->choicedMove[opposingBattler]) < 1)
            break;
        playerMove = SMART_SWITCHING_OMNISCIENT ? gBattleMons[opposingBattler].moves[moveIndex] : playerMoves[moveIndex];
        if (GetBattleMovePriority(opposingBattler, gAiLogicData->abilities[opposingBattler], playerMove) > 0
            && playerMove != MOVE_NONE && !IsBattleMoveStatus(playerMove) && GetMoveEffect(playerMove) != EFFECT_FOCUS_PUNCH && gBattleMons[opposingBattler].pp[moveIndex] > 0)
        {
            damageTaken = AI_GetDamage(opposingBattler, battler, moveIndex, AI_SWITCHIN_DEFENDING, gAiLogicData);
            if (playerMove == gBattleStruct->choicedMove[opposingBattler]) // If player is choiced, only care about the choice locked move
            {
                *bestPlayerPriorityMove = playerMove;
                return damageTaken;
            }
            if (damageTaken > maxDamageTaken)
            {
                maxDamageTaken = damageTaken;
                *bestPlayerPriorityMove = playerMove;
            }
        }
    }
    return maxDamageTaken;
}

static bool32 AI_CanSwitchinAbilityTrapOpponent(enum Ability ability, enum BattlerId opposingBattler)
{
    if (AI_CanBattlerEscape(opposingBattler))
        return FALSE;
    if (gBattleTypeFlags & BATTLE_TYPE_TRAINER && CountUsablePartyMons(opposingBattler) == 0)
        return FALSE;
    else if (ability == ABILITY_SHADOW_TAG)
    {
        if (B_SHADOW_TAG_ESCAPE >= GEN_4 && gAiLogicData->abilities[opposingBattler] == ABILITY_SHADOW_TAG)
            return FALSE;
        else
            return TRUE;
    }
    else if (ability == ABILITY_ARENA_TRAP && IsBattlerGrounded(opposingBattler, gAiLogicData->abilities[opposingBattler], gAiLogicData->holdEffects[opposingBattler]))
        return TRUE;
    else if (ability == ABILITY_MAGNET_PULL && IS_BATTLER_OF_TYPE(opposingBattler, TYPE_STEEL))
        return TRUE;
    else
        return FALSE;
}

static inline bool32 IsFreeSwitch(enum SwitchType switchType, enum BattlerId battlerSwitchingOut, enum BattlerId opposingBattler)
{
    bool32 movedSecond = GetBattlerTurnOrderNum(battlerSwitchingOut) > GetBattlerTurnOrderNum(opposingBattler) ? TRUE : FALSE;

    // Switch out effects
    if (!IsDoubleBattle()) // Not handling doubles' additional complexity
    {
        if (IsSwitchOutEffect(GetMoveEffect(gCurrentMove)) && movedSecond)
            return TRUE;
        if (gAiLogicData->ejectButtonSwitch)
            return TRUE;
        if (gAiLogicData->ejectPackSwitch)
        {
            enum Ability opposingAbility = gAiLogicData->abilities[opposingBattler];
            // If faster, not a free switch; likely lowered own stats
            if (!movedSecond && opposingAbility != ABILITY_INTIMIDATE && opposingAbility != ABILITY_SUPERSWEET_SYRUP) // Intimidate triggers switches before turn starts
                return FALSE;
            // Otherwise, free switch
            return TRUE;
        }
    }

    // Post KO check has to be last because the GetMostSuitableMonToSwitchInto call in OpponentHandleChoosePokemon assumes a KO rather than a forced switch choice
    if (switchType == SWITCH_AFTER_KO)
        return TRUE;
    else
        return FALSE;
}

static inline bool32 CanSwitchinWin1v1(u32 hitsToKOAI, u32 hitsToKOPlayer, bool32 isSwitchinFirst, bool32 isFreeSwitch)
{
    // Player's best move deals 0 damage
    if (hitsToKOAI == 0 && hitsToKOPlayer > 0)
        return TRUE;

    // AI's best move deals 0 damage
    if (hitsToKOPlayer == 0 && hitsToKOAI > 0)
        return FALSE;

    // Neither mon can damage the other
    if (hitsToKOPlayer == 0 && hitsToKOAI == 0)
        return FALSE;

    // Free switch, need to outspeed or take 1 extra hit
    if (isFreeSwitch)
    {
        if (hitsToKOAI > hitsToKOPlayer || (hitsToKOAI == hitsToKOPlayer && isSwitchinFirst))
            return TRUE;
    }
    // Mid battle switch, need to take 1 or 2 extra hits depending on speed
    if (hitsToKOAI > hitsToKOPlayer + 1 || (hitsToKOAI == hitsToKOPlayer + 1 && isSwitchinFirst))
        return TRUE;
    return FALSE;
}

static bool32 BattlerCanPunishTauntInPlace(struct SwitchAiContext *switchContext)
{
    if (switchContext->hasEffectiveMove || switchContext->canBattlerWin1v1)
        return TRUE;

    return GetBestNoOfHitsToKO(switchContext->battler, switchContext->opposingBattler, AI_ATTACKING) <= 2;
}

static bool32 ShouldConsiderTauntPunishSwitch(struct SwitchAiContext *switchContext)
{
    if (!(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_SMART_SWITCHING))
        return FALSE;
    if (!(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_PREDICT_MOVE))
        return FALSE;
    if (switchContext->incomingMove == MOVE_NONE || switchContext->incomingMove == MOVE_UNAVAILABLE)
        return FALSE;
    if (GetMoveEffect(switchContext->incomingMove) != EFFECT_TAUNT)
        return FALSE;
    if (gBattleMons[switchContext->battler].volatiles.tauntTimer != 0)
        return FALSE;
    if (AI_CanBattlerIgnorePredictedMove(switchContext->battler, switchContext->opposingBattler, switchContext->incomingMove))
        return FALSE;
    if (!switchContext->hasImportantStatusMove)
        return FALSE;
    if (BattlerCanPunishTauntInPlace(switchContext))
        return FALSE;

    return TRUE;
}

static u32 FindTauntPunishSwitchin(struct SwitchAiContext *switchContext)
{
    u32 bestMonId = PARTY_SIZE;
    s32 bestDamage = 0;
    struct IncomingHealInfo healInfoData;
    const struct IncomingHealInfo *healInfo = &healInfoData;
    struct AiLogicData *savedAiLogicData = AllocSaveAiLogicData();
    struct BattlePokemon *savedBattleMons = AllocSaveBattleMons();
    u32 savedNotOnField = gBattleStruct->battlerState[switchContext->battler].notOnField;

    GetIncomingHealInfo(switchContext->battler, &healInfoData);
    gBattleStruct->battlerState[switchContext->battler].notOnField = FALSE;

    for (u32 monIndex = 0; monIndex < switchContext->lastId; monIndex++)
    {
        enum Move bestPlayerMove = MOVE_NONE;
        enum Move bestPlayerPriorityMove = MOVE_NONE;
        u32 originalHp;
        u32 hitsToKOAI;
        u32 hitsToKOAIPriority;

        if (!(switchContext->eligiblePartyMons & (1u << monIndex)))
            continue;

        InitializeSwitchinCandidate(switchContext->battler, monIndex, &switchContext->party[monIndex]);
        originalHp = gBattleMons[switchContext->battler].hp;

        hitsToKOAI = GetSwitchinHitsToKO(GetMaxDamagePlayerCouldDealToSwitchin(switchContext->battler, switchContext->opposingBattler, &bestPlayerMove), switchContext->battler, healInfo, originalHp);
        hitsToKOAIPriority = GetSwitchinHitsToKO(GetMaxPriorityDamagePlayerCouldDealToSwitchin(switchContext->battler, switchContext->opposingBattler, &bestPlayerPriorityMove), switchContext->battler, healInfo, originalHp);

        for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
        {
            enum Move move = gBattleMons[switchContext->battler].moves[moveIndex];
            s32 damage;
            u32 hitsToKOPlayer;
            bool32 isSwitchinFirst;
            bool32 isSwitchinFirstPriority;

            if (move == MOVE_NONE || move == MOVE_UNAVAILABLE || IsBattleMoveStatus(move))
                continue;
            if (gBattleMons[switchContext->battler].pp[moveIndex] == 0)
                continue;

            damage = AI_GetDamage(switchContext->battler, switchContext->opposingBattler, moveIndex, AI_SWITCHIN_ATTACKING, gAiLogicData);
            hitsToKOPlayer = GetNoOfHitsToKOBattler(switchContext->battler, switchContext->opposingBattler, moveIndex, AI_SWITCHIN_ATTACKING, CONSIDER_ENDURE);
            isSwitchinFirst = AI_IsFaster(switchContext->battler, switchContext->opposingBattler, move, bestPlayerMove, CONSIDER_PRIORITY);
            isSwitchinFirstPriority = AI_IsFaster(switchContext->battler, switchContext->opposingBattler, move, bestPlayerPriorityMove, CONSIDER_PRIORITY);

            if (!CanSwitchinWin1v1(hitsToKOAI, hitsToKOPlayer, isSwitchinFirst, TRUE)
             || !CanSwitchinWin1v1(hitsToKOAIPriority, hitsToKOPlayer, isSwitchinFirstPriority, TRUE))
                continue;

            if (hitsToKOPlayer <= 2 || damage > AI_SWITCHIN_DAMAGE_THRESHOLD)
            {
                if (bestMonId == PARTY_SIZE || damage > bestDamage)
                {
                    bestMonId = monIndex;
                    bestDamage = damage;
                }
            }
        }
    }

    gBattleStruct->battlerState[switchContext->battler].notOnField = savedNotOnField;
    FreeRestoreAiLogicData(savedAiLogicData);
    FreeRestoreBattleMons(savedBattleMons);

    return bestMonId;
}

static bool32 ShouldSwitchIfPredictedTauntPunish(struct SwitchAiContext *switchContext)
{
    u32 switchinId;

    if (!ShouldConsiderTauntPunishSwitch(switchContext))
        return FALSE;

    switchinId = FindTauntPunishSwitchin(switchContext);
    if (switchinId == PARTY_SIZE)
        return FALSE;

    return SetSwitchinAndSwitch(switchContext->battler, switchinId);
}

static bool32 ShouldStayInForKnownTeraSurvival(struct SwitchAiContext *switchContext)
{
    enum BattlerId battler = switchContext->battler;
    enum BattlerId attacker = switchContext->incomingBattler;
    enum AIConsiderGimmick attackerGimmick = NO_GIMMICK;
    enum Gimmick attackerSelectedGimmick;
    struct SimulatedDamage damageWithoutTera, damageWithTera;
    uq4_12_t effectiveness;
    enum Move *aiMoves;
    u32 targetHp;
    u32 bestDamage = 0;

    if (IsDoubleBattle())
        return FALSE;
    if (!(gAiThinkingStruct->aiFlags[battler] & AI_FLAG_READ_PLAYER_MOVE))
        return FALSE;
    if (!(gAiThinkingStruct->aiFlags[battler] & AI_FLAG_SMART_TERA))
        return FALSE;
    if (attacker >= gBattlersCount || switchContext->incomingMoveIndex >= MAX_MON_MOVES)
        return FALSE;
    if (BattlerHasAi(attacker) || IsBattlerAlly(battler, attacker))
        return FALSE;
    if (gChosenActionByBattler[attacker] != B_ACTION_USE_MOVE)
        return FALSE;
    if (switchContext->incomingMove == MOVE_NONE || switchContext->incomingMove == MOVE_UNAVAILABLE || IsBattleMoveStatus(switchContext->incomingMove))
        return FALSE;
    if (!CanActivateGimmick(battler, GIMMICK_TERA))
        return FALSE;

    attackerSelectedGimmick = gBattleStruct->gimmick.usableGimmick[attacker];
    if (attackerSelectedGimmick != GIMMICK_NONE && IsGimmickSelected(attacker, attackerSelectedGimmick))
        attackerGimmick = USE_GIMMICK;

    damageWithoutTera = AI_CalcDamage(switchContext->incomingMove, attacker, battler, &effectiveness, attackerGimmick, NO_GIMMICK, AI_GetWeather(), gFieldStatuses);
    damageWithTera = AI_CalcDamage(switchContext->incomingMove, attacker, battler, &effectiveness, attackerGimmick, USE_GIMMICK, AI_GetWeather(), gFieldStatuses);
    if (damageWithoutTera.maximum < gBattleMons[battler].hp)
        return FALSE;
    if (damageWithTera.maximum >= gBattleMons[battler].hp)
        return FALSE;
    if (damageWithTera.maximum >= damageWithoutTera.maximum)
        return FALSE;

    aiMoves = GetMovesArray(battler);
    targetHp = max(1, gBattleMons[switchContext->opposingBattler].hp);
    for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
    {
        struct SimulatedDamage damage;
        bool32 isBattlerFirst;

        if (IsMoveUnusable(moveIndex, aiMoves[moveIndex], gAiLogicData->moveLimitations[battler]) || IsBattleMoveStatus(aiMoves[moveIndex]))
            continue;
        isBattlerFirst = AI_IsFaster(battler, attacker, aiMoves[moveIndex], switchContext->incomingMove, CONSIDER_PRIORITY);
        if (!isBattlerFirst)
            continue;

        damage = AI_CalcDamage(aiMoves[moveIndex], battler, switchContext->opposingBattler, &effectiveness, USE_GIMMICK, NO_GIMMICK, AI_GetWeather(), gFieldStatuses);
        if (damage.median > bestDamage)
            bestDamage = damage.median;
    }

    return bestDamage * 100 >= targetHp * 35;
}

static u32 FindSwitchinThatSurvivesKnownSingleTargetMove(struct SwitchAiContext *switchContext)
{
    struct IncomingHealInfo healInfoData;
    const struct IncomingHealInfo *healInfo = &healInfoData;
    struct AiLogicData *savedAiLogicData = AllocSaveAiLogicData();
    struct BattlePokemon *savedBattleMons = AllocSaveBattleMons();
    u32 savedNotOnField = gBattleStruct->battlerState[switchContext->battler].notOnField;
    u32 bestMonId = PARTY_SIZE;
    u32 bestScore = 1;
    bool32 bestHasFakeOut = FALSE;
    bool32 bestHasPivot = FALSE;

    GetIncomingHealInfo(switchContext->battler, &healInfoData);
    gBattleStruct->battlerState[switchContext->battler].notOnField = FALSE;

    for (u32 monIndex = 0; monIndex < switchContext->lastId; monIndex++)
    {
        u32 originalHp;
        u32 hitsToKO;
        u32 score;
        bool32 hasFakeOut;
        bool32 hasPivot;

        if (!(switchContext->eligiblePartyMons & (1u << monIndex)))
            continue;

        InitializeSwitchinCandidate(switchContext->battler, monIndex, &switchContext->party[monIndex]);
        originalHp = gBattleMons[switchContext->battler].hp;

        if (healInfo->healBeforeHazards)
        {
            gBattleMons[switchContext->battler].hp = gBattleMons[switchContext->battler].maxHP;
            if (healInfo->curesStatus)
                gBattleMons[switchContext->battler].status1 = 0;
        }

        if (gAiLogicData->abilities[switchContext->battler] == ABILITY_TRUANT && IsTruantMonVulnerable(switchContext->battler, switchContext->incomingBattler))
            continue;

        hitsToKO = GetSwitchinHitsToKO(
            GetKnownPlayerSelectedMoveDamageForSwitch(switchContext->incomingBattler, switchContext->battler, switchContext->incomingMove, switchContext->incomingMoveIndex, AI_SWITCHIN_DEFENDING),
            switchContext->battler,
            healInfo,
            originalHp);

        // This path bypasses the doubles Protect-preservation guard, so require
        // a real defensive pivot rather than a switch-in that merely survives.
        if (hitsToKO != 0 && hitsToKO <= AI_DEFENSIVE_KO_THRESHOLD)
            continue;

        score = hitsToKO == 0 ? 0xFFFF : hitsToKO;
        hasFakeOut = PartyMonHasMoveEffect(&switchContext->party[monIndex], EFFECT_FIRST_TURN_ONLY);
        hasPivot = PartyMonHasMoveEffect(&switchContext->party[monIndex], EFFECT_HIT_ESCAPE)
                || PartyMonHasMoveEffect(&switchContext->party[monIndex], EFFECT_PARTING_SHOT);
        if (hasFakeOut)
            score++;
        if (hasPivot)
            score++;

        if (score > bestScore
         || (score == bestScore && hasFakeOut && !bestHasFakeOut)
         || (score == bestScore && hasFakeOut == bestHasFakeOut && hasPivot && !bestHasPivot))
        {
            bestMonId = monIndex;
            bestScore = score;
            bestHasFakeOut = hasFakeOut;
            bestHasPivot = hasPivot;
        }
    }

    gBattleStruct->battlerState[switchContext->battler].notOnField = savedNotOnField;
    FreeRestoreAiLogicData(savedAiLogicData);
    FreeRestoreBattleMons(savedBattleMons);

    return bestMonId;
}

static bool32 ShouldSwitchIfKnownSingleTargetKO(struct SwitchAiContext *switchContext)
{
    u32 switchinId;
    u32 incomingDamage;

    if (!(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_SMART_SWITCHING))
        return FALSE;
    if (!(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_READ_PLAYER_MOVE))
        return FALSE;
    if (switchContext->incomingBattler >= gBattlersCount || switchContext->incomingMoveIndex >= MAX_MON_MOVES)
        return FALSE;
    if (IsBattleMoveStatus(switchContext->incomingMove))
        return FALSE;
    if (IsBattlerAlly(switchContext->battler, switchContext->incomingBattler))
        return FALSE;

    incomingDamage = GetKnownPlayerSelectedMoveDamageForSwitch(switchContext->incomingBattler, switchContext->battler, switchContext->incomingMove, switchContext->incomingMoveIndex, AI_DEFENDING);
    if (incomingDamage < gBattleMons[switchContext->battler].hp)
        return FALSE;
    if (CanEndureHit(switchContext->incomingBattler, switchContext->battler, switchContext->incomingMove))
        return FALSE;

    switchinId = FindSwitchinThatSurvivesKnownSingleTargetMove(switchContext);
    if (switchinId == PARTY_SIZE)
        return FALSE;

    return SetSwitchinAndSwitch(switchContext->battler, switchinId);
}

static bool32 IsBattlerWorthFocusedPreserve(struct SwitchAiContext *switchContext)
{
    enum BattlerId battler = switchContext->battler;

    if (HasChoiceEffect(battler)
     && (gAiLogicData->abilities[battler] == ABILITY_GORILLA_TACTICS || IsBattlerItemEnabled(battler)))
        return TRUE;

    if (switchContext->hasStatRaised)
        return TRUE;

    if (GetActiveGimmick(battler) != GIMMICK_NONE)
        return TRUE;

    if (CanDoubleBattlerMakeProgress(switchContext))
        return TRUE;

    return FALSE;
}

static bool32 PartyMonHasFocusedSacrificeSupportValue(struct Pokemon *mon)
{
    return PartyMonHasMoveEffect(mon, EFFECT_TAILWIND)
        || PartyMonHasMoveEffect(mon, EFFECT_TRICK_ROOM)
        || PartyMonHasMoveEffect(mon, EFFECT_ENCORE)
        || PartyMonHasMoveEffect(mon, EFFECT_FIRST_TURN_ONLY)
        || PartyMonHasMoveEffect(mon, EFFECT_HIT_ESCAPE)
        || PartyMonHasMoveEffect(mon, EFFECT_PARTING_SHOT);
}

static bool32 FocusedSacrificeHasNextBoardPayoff(bool32 consumesFakeOut, bool32 hasSupportValue, bool32 isFocusSash)
{
    return consumesFakeOut || hasSupportValue || isFocusSash;
}

static bool32 ShouldAcceptFocusedSwitchinRollSurvival(
    struct SwitchAiContext *switchContext,
    const struct KnownPlayerDamageRollSummary *damage,
    const struct IncomingHealInfo *healInfo,
    u32 originalHp,
    u32 maxHitsToKO,
    bool32 allowSwitchSurvivalRisk,
    bool32 allowDesperationRisk,
    bool32 hasMeaningfulBenefit,
    u32 *acceptedHitsToKO,
    u32 *scoreBonus)
{
    u32 acceptedDamage;
    u32 hitsToKO;
    bool32 accepted15Of16 = FALSE;

    if (acceptedHitsToKO != NULL)
        *acceptedHitsToKO = 0;
    if (scoreBonus != NULL)
        *scoreBonus = 0;

    if (damage == NULL || !allowSwitchSurvivalRisk || !hasMeaningfulBenefit)
        return FALSE;
    if (damage->maximum == 0 || damage->maximum <= damage->roll15Of16 || maxHitsToKO != 1)
        return FALSE;

    acceptedDamage = damage->roll15Of16;
    hitsToKO = GetSwitchinHitsToKO(acceptedDamage, switchContext->battler, healInfo, originalHp);
    if (hitsToKO > 1 || hitsToKO == 0)
    {
        accepted15Of16 = TRUE;
    }
    else if (allowDesperationRisk && damage->roll14Of16 < damage->roll15Of16)
    {
        acceptedDamage = damage->roll14Of16;
        hitsToKO = GetSwitchinHitsToKO(acceptedDamage, switchContext->battler, healInfo, originalHp);
    }
    else
    {
        return FALSE;
    }

    *acceptedHitsToKO = hitsToKO;
    if (*acceptedHitsToKO != 0 && *acceptedHitsToKO <= 1)
        return FALSE;

    if (scoreBonus != NULL)
        *scoreBonus = accepted15Of16 ? 600 : 300;

    return TRUE;
}

static bool32 BuildKnownFocusedSlotCollapseSwitchCandidate(
    u32 monIndex,
    u32 knownDamage,
    u32 hitsToKO,
    u32 rollHitsToKO,
    u32 rollScoreBonus,
    bool32 acceptsRollSurvival,
    bool32 consumesFakeOut,
    bool32 canMakeProgress,
    bool32 isMostSuitable,
    bool32 isFocusSash,
    bool32 isChoiceReserve,
    bool32 hasSupportValue,
    struct FocusedSwitchCandidate *candidate)
{
    bool32 isSafe = knownDamage == 0 || hitsToKO == 0 || hitsToKO > AI_DEFENSIVE_KO_THRESHOLD;

    if (candidate == NULL)
        return FALSE;

    *candidate = (struct FocusedSwitchCandidate){
        .monIndex = monIndex,
        .score = 0,
        .kind = FOCUSED_SWITCH_CANDIDATE_NONE,
    };

    if (isSafe)
    {
        candidate->kind = FOCUSED_SWITCH_CANDIDATE_SAFE;
        candidate->score = 10000
                         + (canMakeProgress ? 1000 : 0)
                         + (isMostSuitable ? 500 : 0)
                         + (hasSupportValue ? 100 : 0)
                         + (hitsToKO == 0 ? 255 : hitsToKO);
        return TRUE;
    }

    if (acceptsRollSurvival)
    {
        candidate->kind = FOCUSED_SWITCH_CANDIDATE_ROLL_SURVIVAL;
        candidate->score = 4000
                         + rollScoreBonus
                         + (canMakeProgress ? 350 : 0)
                         + (isMostSuitable ? 200 : 0)
                         + (hasSupportValue ? 150 : 0)
                         + (isFocusSash ? 100 : 0)
                         + (rollHitsToKO == 0 ? 255 : rollHitsToKO * 10);
        return TRUE;
    }

    if (hitsToKO > 1)
    {
        candidate->kind = FOCUSED_SWITCH_CANDIDATE_CUSHION;
        candidate->score = 2500
                         + (canMakeProgress ? 300 : 0)
                         + (isMostSuitable ? 150 : 0)
                         + (hasSupportValue ? 100 : 0)
                         + (!isChoiceReserve ? 80 : 0)
                         + hitsToKO * 20;
        return TRUE;
    }

    if (isChoiceReserve && !hasSupportValue && !isFocusSash)
        return FALSE;
    if (!FocusedSacrificeHasNextBoardPayoff(consumesFakeOut, hasSupportValue, isFocusSash))
        return FALSE;
    if (!hasSupportValue && !isFocusSash && canMakeProgress && isMostSuitable)
        return FALSE;

    candidate->kind = FOCUSED_SWITCH_CANDIDATE_SACRIFICE;
    candidate->score = 1000
                     + (consumesFakeOut ? 350 : 0)
                     + (isFocusSash ? 300 : 0)
                     + (hasSupportValue ? 250 : 0)
                     + (!isChoiceReserve ? 100 : 0)
                     + (!isMostSuitable ? 75 : 0);
    return TRUE;
}

static u32 FindKnownFocusedSlotCollapseSwitchin(struct SwitchAiContext *switchContext, bool32 consumesFakeOut)
{
    struct IncomingHealInfo healInfoData;
    const struct IncomingHealInfo *healInfo = &healInfoData;
    struct AiLogicData *savedAiLogicData = AllocSaveAiLogicData();
    struct BattlePokemon *savedBattleMons = AllocSaveBattleMons();
    u32 savedNotOnField = gBattleStruct->battlerState[switchContext->battler].notOnField;
    u32 bestMonId = PARTY_SIZE;
    u32 bestScore = 0;
    bool32 allowSwitchSurvivalRisk;
    bool32 allowDesperationRisk;

    GetIncomingHealInfo(switchContext->battler, &healInfoData);
    gBattleStruct->battlerState[switchContext->battler].notOnField = FALSE;
    allowSwitchSurvivalRisk = AI_RiskGovernorAllows(switchContext->battler, switchContext->opposingBattler, AI_RISK_SWITCH_SURVIVAL);
    allowDesperationRisk = AI_ShouldAcceptDesperationRisk(switchContext->battler, switchContext->opposingBattler);

    for (u32 monIndex = 0; monIndex < switchContext->lastId; monIndex++)
    {
        struct KnownPlayerDamageRollSummary damageRolls;
        u32 originalHp;
        u32 knownDamage;
        u32 hitsToKO;
        u32 rollHitsToKO = 0;
        u32 rollScoreBonus = 0;
        enum Item item;
        enum HoldEffect holdEffect;
        bool32 isFocusSash;
        bool32 isChoiceReserve;
        bool32 hasSupportValue;
        bool32 canMakeProgress;
        bool32 isMostSuitable;
        bool32 acceptsRollSurvival;
        struct FocusedSwitchCandidate candidate;

        if (!(switchContext->eligiblePartyMons & (1u << monIndex)))
            continue;

        InitializeSwitchinCandidate(switchContext->battler, monIndex, &switchContext->party[monIndex]);
        originalHp = gBattleMons[switchContext->battler].hp;

        if (healInfo->healBeforeHazards)
        {
            gBattleMons[switchContext->battler].hp = gBattleMons[switchContext->battler].maxHP;
            if (healInfo->curesStatus)
                gBattleMons[switchContext->battler].status1 = 0;
        }

        if (gAiLogicData->abilities[switchContext->battler] == ABILITY_TRUANT && IsTruantMonVulnerable(switchContext->battler, switchContext->incomingBattler))
            continue;

        GetKnownPlayerDamageRollSummaryIntoBattlerSlot(switchContext->battler, &damageRolls);
        knownDamage = damageRolls.maximum != 0 ? damageRolls.maximum : GetKnownPlayerDamageIntoBattlerSlot(switchContext->battler, AI_SWITCHIN_DEFENDING);
        hitsToKO = GetSwitchinHitsToKO(knownDamage, switchContext->battler, healInfo, originalHp);
        canMakeProgress = CanDoubleBattlerMakeProgress(switchContext);
        isMostSuitable = gAiLogicData->mostSuitableMonId[switchContext->battler] == monIndex;
        item = GetMonData(&switchContext->party[monIndex], MON_DATA_HELD_ITEM);
        holdEffect = GetItemHoldEffect(item);
        isFocusSash = holdEffect == HOLD_EFFECT_FOCUS_SASH;
        isChoiceReserve = IsHoldEffectChoice(holdEffect);
        hasSupportValue = PartyMonHasFocusedSacrificeSupportValue(&switchContext->party[monIndex]);
        acceptsRollSurvival = ShouldAcceptFocusedSwitchinRollSurvival(
            switchContext,
            &damageRolls,
            healInfo,
            originalHp,
            hitsToKO,
            allowSwitchSurvivalRisk,
            allowDesperationRisk,
            canMakeProgress || isMostSuitable || hasSupportValue || isFocusSash,
            &rollHitsToKO,
            &rollScoreBonus);

        if (!BuildKnownFocusedSlotCollapseSwitchCandidate(
            monIndex,
            knownDamage,
            hitsToKO,
            rollHitsToKO,
            rollScoreBonus,
            acceptsRollSurvival,
            consumesFakeOut,
            canMakeProgress,
            isMostSuitable,
            isFocusSash,
            isChoiceReserve,
            hasSupportValue,
            &candidate))
            continue;

        if (candidate.score > bestScore)
        {
            bestMonId = candidate.monIndex;
            bestScore = candidate.score;
        }
    }

    gBattleStruct->battlerState[switchContext->battler].notOnField = savedNotOnField;
    FreeRestoreAiLogicData(savedAiLogicData);
    FreeRestoreBattleMons(savedBattleMons);

    return bestMonId;
}

static bool32 ShouldSwitchIfKnownFocusedSlotCollapse(struct SwitchAiContext *switchContext)
{
    u32 hitCount = 0;
    u32 knownDamage;
    u32 switchinId;
    bool32 hasFakeOut = FALSE;

    if (!(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_SMART_SWITCHING))
        return FALSE;
    if (!(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_READ_PLAYER_MOVE))
        return FALSE;
    if (!IsDoubleBattle())
        return FALSE;
    if (!AreKnownPlayerCommandsReadyForSwitch())
        return FALSE;

    knownDamage = GetKnownPlayerDamageSummaryIntoBattlerSlot(switchContext->battler, AI_DEFENDING, &hitCount, &hasFakeOut);
    if (knownDamage < gBattleMons[switchContext->battler].hp)
        return FALSE;
    if (hitCount < 2 && !hasFakeOut)
        return FALSE;
    if (!IsBattlerWorthFocusedPreserve(switchContext))
        return FALSE;

    switchinId = FindKnownFocusedSlotCollapseSwitchin(switchContext, hasFakeOut);
    if (switchinId == PARTY_SIZE)
        return FALSE;

    return SetSwitchinAndSwitch(switchContext->battler, switchinId);
}

static bool32 IsBattlerChoiceLockedForSwitch(enum BattlerId battler, enum Move *lockedMove)
{
    *lockedMove = MOVE_NONE;

    if (gBattleStruct == NULL)
        return FALSE;
    if (!HasChoiceEffect(battler))
        return FALSE;
    if (gAiLogicData->abilities[battler] != ABILITY_GORILLA_TACTICS && !IsBattlerItemEnabled(battler))
        return FALSE;

    *lockedMove = gBattleStruct->choicedMove[battler];
    return *lockedMove != MOVE_NONE && *lockedMove != MOVE_UNAVAILABLE;
}

static bool32 ChoiceLockedMoveCanMakeDoubleProgress(struct SwitchAiContext *switchContext, enum Move lockedMove)
{
    enum BattlerId battler = switchContext->battler;
    enum BattlerId opposingBattler = switchContext->opposingBattler;
    enum BattlerId opposingPartner = BATTLE_PARTNER(opposingBattler);
    enum BattlerId targets[2] = {opposingBattler, opposingPartner};
    enum MoveTarget moveTarget;
    u32 moveIndex;
    u32 totalDamage = 0;
    u32 totalMaxHp = 0;

    if (IsBattleMoveStatus(lockedMove))
        return FALSE;

    moveIndex = GetMoveIndex(battler, lockedMove);
    if (moveIndex >= MAX_MON_MOVES)
        return FALSE;

    moveTarget = AI_GetBattlerMoveTargetType(battler, lockedMove);

    for (u32 targetIndex = 0; targetIndex < ARRAY_COUNT(targets); targetIndex++)
    {
        enum BattlerId target = targets[targetIndex];
        u32 damage;

        if (!IsBattlerAlive(target))
            continue;
        if (!CanKnownMoveHitBattlerSlot(battler, target, lockedMove, target))
            continue;

        damage = AI_GetDamage(battler, target, moveIndex, AI_ATTACKING, gAiLogicData);
        totalDamage += damage;
        totalMaxHp += gBattleMons[target].maxHP;

        if (GetNoOfHitsToKOBattler(battler, target, moveIndex, AI_ATTACKING, CONSIDER_ENDURE) <= 2)
            return TRUE;
    }

    if ((IsSpreadMove(moveTarget) || moveTarget == TARGET_ALL_BATTLERS) && totalMaxHp != 0 && totalDamage * 100 >= totalMaxHp * 30)
        return TRUE;

    return FALSE;
}

static u32 FindReadChoiceRoleDoneSwitchin(struct SwitchAiContext *switchContext)
{
    struct IncomingHealInfo healInfoData;
    const struct IncomingHealInfo *healInfo = &healInfoData;
    struct AiLogicData *savedAiLogicData = AllocSaveAiLogicData();
    struct BattlePokemon *savedBattleMons = AllocSaveBattleMons();
    u32 savedNotOnField = gBattleStruct->battlerState[switchContext->battler].notOnField;
    u32 bestMonId = PARTY_SIZE;
    u32 bestScore = 0;

    GetIncomingHealInfo(switchContext->battler, &healInfoData);
    gBattleStruct->battlerState[switchContext->battler].notOnField = FALSE;

    for (u32 monIndex = 0; monIndex < switchContext->lastId; monIndex++)
    {
        u32 originalHp;
        u32 knownDamage;
        u32 hitsToKO;
        u32 score;
        bool32 canMakeProgress;
        bool32 isMostSuitable;

        if (!(switchContext->eligiblePartyMons & (1u << monIndex)))
            continue;

        InitializeSwitchinCandidate(switchContext->battler, monIndex, &switchContext->party[monIndex]);
        originalHp = gBattleMons[switchContext->battler].hp;

        if (healInfo->healBeforeHazards)
        {
            gBattleMons[switchContext->battler].hp = gBattleMons[switchContext->battler].maxHP;
            if (healInfo->curesStatus)
                gBattleMons[switchContext->battler].status1 = 0;
        }

        if (gAiLogicData->abilities[switchContext->battler] == ABILITY_TRUANT && IsTruantMonVulnerable(switchContext->battler, switchContext->incomingBattler))
            continue;

        knownDamage = GetKnownPlayerDamageIntoBattlerSlot(switchContext->battler, AI_SWITCHIN_DEFENDING);
        hitsToKO = GetSwitchinHitsToKO(knownDamage, switchContext->battler, healInfo, originalHp);
        if (knownDamage != 0 && hitsToKO <= AI_DEFENSIVE_KO_THRESHOLD)
            continue;

        canMakeProgress = CanDoubleBattlerMakeProgress(switchContext);
        isMostSuitable = gAiLogicData->mostSuitableMonId[switchContext->battler] == monIndex;
        if (!canMakeProgress && !isMostSuitable)
            continue;

        score = (canMakeProgress ? 1000 : 0)
              + (isMostSuitable ? 500 : 0)
              + (hitsToKO == 0 ? 255 : hitsToKO);
        if (score > bestScore)
        {
            bestMonId = monIndex;
            bestScore = score;
        }
    }

    gBattleStruct->battlerState[switchContext->battler].notOnField = savedNotOnField;
    FreeRestoreAiLogicData(savedAiLogicData);
    FreeRestoreBattleMons(savedBattleMons);

    return bestMonId;
}

static bool32 ShouldSwitchIfReadChoiceRoleDone(struct SwitchAiContext *switchContext)
{
    enum Move lockedMove;
    u32 switchinId;

    if (!(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_SMART_SWITCHING))
        return FALSE;
    if (!(gAiThinkingStruct->aiFlags[switchContext->battler] & AI_FLAG_READ_PLAYER_MOVE))
        return FALSE;
    if (!IsDoubleBattle())
        return FALSE;
    if (!AreKnownPlayerCommandsReadyForSwitch())
        return FALSE;
    if (!IsBattlerChoiceLockedForSwitch(switchContext->battler, &lockedMove))
        return FALSE;
    // A spent locked battler can still be the correct cushion. Do not expose a
    // reserve when the confirmed player turn is already spending damage here.
    if (GetKnownPlayerDamageIntoBattlerSlot(switchContext->battler, AI_DEFENDING) != 0)
        return FALSE;
    if (ChoiceLockedMoveCanMakeDoubleProgress(switchContext, lockedMove))
        return FALSE;

    switchinId = FindReadChoiceRoleDoneSwitchin(switchContext);
    if (switchinId == PARTY_SIZE)
        return FALSE;

    return SetSwitchinAndSwitch(switchContext->battler, switchinId);
}

// This function splits switching behaviour depending on whether the switch is free.
// Everything runs in the same loop to minimize computation time. This makes it harder to read, but hopefully the comments can guide you!
static u32 GetBestMonIntegrated(struct Pokemon *party, int lastId, enum BattlerId battler, enum BattlerId opposingBattler, enum BattlerId battlerIn1, enum BattlerId battlerIn2, enum SwitchType switchType)
{
    struct IncomingHealInfo healInfoData;
    const struct IncomingHealInfo *healInfo = &healInfoData;
    u32 revengeKillerIds = 0, slowRevengeKillerIds = 0, fastThreatenIds = 0, slowThreatenIds = 0, damageMonIds = 0, generic1v1MonIds = 0;
    u32 batonPassIds = 0, typeMatchupIds = 0, typeMatchupEffectiveIds = 0, defensiveMonIds = 0, trapperIds = 0, healingCandidateIds = 0;
    u32 bestDefensiveMonId = PARTY_SIZE, bestTypeMatchupId = PARTY_SIZE, bestTypeMatchupEffectiveId = PARTY_SIZE, bestDamageMonId = PARTY_SIZE, bestHealGainId = PARTY_SIZE;
    u32 aceMonId = PARTY_SIZE, aceMonCount = 0;
    s32 playerMonHP = gBattleMons[opposingBattler].hp, maxDamageDealt = AI_SWITCHIN_DAMAGE_THRESHOLD, damageDealt = 0, bestHealGain = 0;
    enum Move aiMove, bestPlayerMove = MOVE_NONE, bestPlayerPriorityMove = MOVE_NONE;
    u32 hitsToKOAI, hitsToKOPlayer, hitsToKOAIPriority, maxHitsToKO = AI_DEFENSIVE_KO_THRESHOLD;
    uq4_12_t bestResist = AI_TYPE_MATCHUP_THRESHOLD, bestResistEffective = AI_TYPE_MATCHUP_THRESHOLD, typeMatchup; // 2.0 is the default "Neutral" matchup from GetBattlerTypeMatchup
    bool32 isFreeSwitch = IsFreeSwitch(switchType, battlerIn1, opposingBattler), isSwitchinFirst, isSwitchinFirstPriority, canSwitchinWin1v1;
    u32 validMonIds = 0;

    GetIncomingHealInfo(battler, &healInfoData);

    // Save existing battler data
    struct AiLogicData *savedAiLogicData = AllocSaveAiLogicData();
    struct BattlePokemon *savedBattleMons = AllocSaveBattleMons();
    u32 savedNotOnField = gBattleStruct->battlerState[battler].notOnField;

    gBattleStruct->battlerState[battler].notOnField = FALSE;
    // Iterate through mons
    for (u32 monIndex = 0; monIndex < lastId; monIndex++)
    {
        // Check mon validity
        if (!IsValidForBattle(&party[monIndex]))
            continue;
        // Check if mon is already in play or being sent in
        if (IsPartyMonOnFieldOrChosenToSwitch(battler, monIndex, battlerIn1, battlerIn2))
            continue;
        // Check if partner wants to use this mon already
        if (IsPartyMonPlannedToBeSwitchedInByPartner(monIndex, battler))
            continue;
        // Save Ace Pokemon for last
        if (IsAceMon(battler, monIndex))
        {
            aceMonId = monIndex;
            aceMonCount++;
            continue;
        }

        validMonIds |= (1u << monIndex);
        InitializeSwitchinCandidate(battler, monIndex, &party[monIndex]);

        u32 originalHp = gBattleMons[battler].hp;

        if (healInfo->healBeforeHazards)
        {
            gBattleMons[battler].hp = gBattleMons[battler].maxHP;
            if (healInfo->curesStatus)
                gBattleMons[battler].status1 = 0;
        }

        // While not really invalid per se, not really wise to switch into this mon
        if (gAiLogicData->abilities[battler] == ABILITY_TRUANT && IsTruantMonVulnerable(battler, opposingBattler))
            continue;

        // Get max number of hits for player to KO AI mon and type matchup for defensive switching
        hitsToKOAI = GetSwitchinHitsToKO(GetMaxDamagePlayerCouldDealToSwitchin(battler, opposingBattler, &bestPlayerMove), battler, healInfo, originalHp);
        hitsToKOAIPriority = GetSwitchinHitsToKO(GetMaxPriorityDamagePlayerCouldDealToSwitchin(battler, opposingBattler, &bestPlayerPriorityMove), battler, healInfo, originalHp);
        typeMatchup = GetBattlerTypeMatchup(opposingBattler, battler);
        canSwitchinWin1v1 = FALSE;
        bool32 anyMoveCanWin1v1 = FALSE;

        // Check through current mon's moves
        for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
        {
            // Check that move has PP remaining before running calcs
            if (gBattleMons[battler].pp[moveIndex] < 1)
                continue;

            aiMove = gBattleMons[battler].moves[moveIndex];
            damageDealt = AI_GetDamage(battler, opposingBattler, moveIndex, AI_SWITCHIN_ATTACKING, gAiLogicData);
            hitsToKOPlayer = GetNoOfHitsToKOBattler(battler, opposingBattler, moveIndex, AI_SWITCHIN_ATTACKING, CONSIDER_ENDURE);

            // Offensive switchin decisions are based on which whether switchin moves first and whether it can win a 1v1
            isSwitchinFirst = AI_IsFaster(battler, opposingBattler, aiMove, bestPlayerMove, CONSIDER_PRIORITY);
            isSwitchinFirstPriority = AI_IsFaster(battler, opposingBattler, aiMove, bestPlayerPriorityMove, CONSIDER_PRIORITY);
            canSwitchinWin1v1 = CanSwitchinWin1v1(hitsToKOAI, hitsToKOPlayer, isSwitchinFirst, isFreeSwitch) && CanSwitchinWin1v1(hitsToKOAIPriority, hitsToKOPlayer, isSwitchinFirstPriority, isFreeSwitch); // AI must successfully 1v1 with and without priority to be considered a good option
            anyMoveCanWin1v1 |= canSwitchinWin1v1;

            // Check for Baton Pass; hitsToKO requirements mean mon can boost and BP without dying whether it's slower or not
            if (GetMoveEffect(aiMove) == EFFECT_BATON_PASS)
            {
                if ((isSwitchinFirst && hitsToKOAI > 1) || hitsToKOAI > 2) // Need to take an extra hit if slower
                    batonPassIds |= (1u << monIndex);
            }

            // Check that good type matchups get at least two turns and set best type matchup mon
            if (typeMatchup < AI_TYPE_MATCHUP_THRESHOLD)
            {
                if (canSwitchinWin1v1)
                {
                    typeMatchupIds |= (1u << monIndex);
                    if (typeMatchup < bestResist)
                    {
                        bestResist = typeMatchup;
                        bestTypeMatchupId = monIndex;
                    }
                }
            }

            // Track max hits to KO and set defensive mon
            if (hitsToKOAI > AI_DEFENSIVE_KO_THRESHOLD)
            {
                if (canSwitchinWin1v1 || gAiThinkingStruct->aiFlags[battler] & AI_FLAG_STALL)
                {
                    defensiveMonIds |= (1u << monIndex);
                    if (hitsToKOAI > maxHitsToKO)
                    {
                        maxHitsToKO = hitsToKOAI;
                        bestDefensiveMonId = monIndex;
                    }
                }
            }

            if (canSwitchinWin1v1)
                generic1v1MonIds |= (1u << monIndex);

            // Check for mon with resistance and super effective move for best type matchup mon with effective move
            if (aiMove != MOVE_NONE && !IsBattleMoveStatus(aiMove))
            {
                if (typeMatchup < AI_TYPE_MATCHUP_THRESHOLD)
                {
                    if (gAiLogicData->effectiveness[battler][opposingBattler][moveIndex] >= UQ_4_12(2.0))
                    {
                        if (canSwitchinWin1v1)
                        {
                            typeMatchupEffectiveIds |= (1u << monIndex);
                            if (typeMatchup < bestResistEffective)
                            {
                                bestResistEffective = typeMatchup;
                                bestTypeMatchupEffectiveId = monIndex;
                            }
                        }
                    }
                }

                // If a self destruction move doesn't OHKO, don't factor it into revenge killing
                if (IsExplosionMove(aiMove) && damageDealt < playerMonHP)
                    continue;

                // Check that mon isn't one shot and set best damage mon
                if (damageDealt > AI_SWITCHIN_DAMAGE_THRESHOLD)
                {
                    if ((isFreeSwitch && hitsToKOAI > 1) || hitsToKOAI > 2) // This is a "default", we have uniquely low standards
                    {
                        damageMonIds |= (1u << monIndex);
                        if (damageDealt > maxDamageDealt)
                        {
                            maxDamageDealt = damageDealt;
                            bestDamageMonId = monIndex;
                        }
                    }
                }

                // Check if current mon can revenge kill in some capacity
                // If AI mon can one shot
                if (damageDealt >= playerMonHP)
                {
                    if (canSwitchinWin1v1)
                    {
                        if (isSwitchinFirst)
                            revengeKillerIds |= (1u << monIndex);
                        else
                            slowRevengeKillerIds |= (1u << monIndex);
                    }
                }

                // If AI mon can two shot
                if (damageDealt >= (playerMonHP / 2 + playerMonHP % 2)) // Modulo to handle odd numbers in non-decimal division
                {
                    if (canSwitchinWin1v1)
                    {
                        if (isSwitchinFirst)
                            fastThreatenIds |= (1u << monIndex);
                        else
                            slowThreatenIds |= (1u << monIndex);
                    }
                }

                // If mon can trap
                if ((AI_CanSwitchinAbilityTrapOpponent(gAiLogicData->abilities[battler], opposingBattler)
                    || (AI_CanSwitchinAbilityTrapOpponent(gAiLogicData->abilities[opposingBattler], opposingBattler) && gAiLogicData->abilities[battler] == ABILITY_TRACE))
                    && canSwitchinWin1v1)
                    trapperIds |= (1u << monIndex);
            }
        }

        // Check for healing candidate - mon that benefits significantly from incoming heal
        if (healInfo->hasHealing)
        {
            u32 maxHP = gBattleMons[battler].maxHP;
            s32 healGain = (s32)maxHP - (s32)originalHp;

            // For Wish, heal gain is the wish amount (capped at what can actually be gained)
            if (healInfo->healEndOfTurn && !healInfo->healBeforeHazards && !healInfo->healAfterHazards)
            {
                healGain = healInfo->healAmount;
                if (healGain > (s32)(maxHP - originalHp))
                    healGain = (s32)(maxHP - originalHp);
            }

            // Require significant heal gain (at least 25% of max HP), must win 1v1 with at least one move, and pick the best
            if (anyMoveCanWin1v1 && healGain > (s32)(maxHP / AI_WISH_HEAL_THRESHOLD))
            {
                healingCandidateIds |= (1u << monIndex);
                if (healGain > bestHealGain)
                {
                    bestHealGain = healGain;
                    bestHealGainId = monIndex;
                }
            }
        }
    }

    // Restore battler data
    gBattleStruct->battlerState[battler].notOnField = savedNotOnField;
    FreeRestoreAiLogicData(savedAiLogicData);
    FreeRestoreBattleMons(savedBattleMons);

    // GetSwitchinCandidate returns either the *last* party mon that met a threshold (without AI_FLAG_RANDOMIZE_SWITCHIN), or a random one that met a threshold
    // If we aren't using AI_FLAG_RANDOMIZE_SWITCHIN there are cases where we don't want the *last* mon, we want the *best* mon
    // Last revenge killer is fine, but if we're picking based on type matchup, we want the best one; so we track that and return accordingly
    bool32 getRandom = (gAiThinkingStruct->aiFlags[battler] & AI_FLAG_RANDOMIZE_SWITCHIN) ? TRUE : FALSE;

    // Different switching priorities depending on switching mid battle vs switching after a KO or slow switch
    if (isFreeSwitch)
    {
        // Return Trapper > Revenge Killer > Type Matchup > Healing Candidate > Baton Pass > Best Damage
        if (trapperIds != 0)                    return GetSwitchinCandidate(trapperIds, battler, lastId, switchType);
        else if (revengeKillerIds != 0)         return GetSwitchinCandidate(revengeKillerIds, battler, lastId, switchType);
        else if (slowRevengeKillerIds != 0)     return GetSwitchinCandidate(slowRevengeKillerIds, battler, lastId, switchType);
        else if (fastThreatenIds != 0)          return GetSwitchinCandidate(fastThreatenIds, battler, lastId, switchType);
        else if (slowThreatenIds != 0)          return GetSwitchinCandidate(slowThreatenIds, battler, lastId, switchType);
        else if (typeMatchupEffectiveIds != 0)  return getRandom ? GetSwitchinCandidate(typeMatchupEffectiveIds, battler, lastId, switchType) : bestTypeMatchupEffectiveId;
        else if (typeMatchupIds != 0)           return getRandom ? GetSwitchinCandidate(typeMatchupIds, battler, lastId, switchType) : bestTypeMatchupId;
        else if (healingCandidateIds != 0)      return getRandom ? GetSwitchinCandidate(healingCandidateIds, battler, lastId, switchType) : bestHealGainId;
        else if (batonPassIds != 0)             return GetSwitchinCandidate(batonPassIds, battler, lastId, switchType);
        else if (generic1v1MonIds != 0)         return GetSwitchinCandidate(generic1v1MonIds, battler, lastId, switchType);
        else if (damageMonIds != 0)             return getRandom ? GetSwitchinCandidate(damageMonIds, battler, lastId, switchType) : bestDamageMonId;
    }
    else
    {
        // Return Trapper > Type Matchup > Best Defensive > Healing Candidate > Baton Pass
        if (trapperIds != 0)                    return GetSwitchinCandidate(trapperIds, battler, lastId, switchType);
        else if (typeMatchupEffectiveIds != 0)  return getRandom ? GetSwitchinCandidate(typeMatchupEffectiveIds, battler, lastId, switchType) : bestTypeMatchupEffectiveId;
        else if (typeMatchupIds != 0)           return getRandom ? GetSwitchinCandidate(typeMatchupIds, battler, lastId, switchType) : bestTypeMatchupId;
        else if (defensiveMonIds != 0)          return getRandom ? GetSwitchinCandidate(defensiveMonIds, battler, lastId, switchType) : bestDefensiveMonId;
        else if (healingCandidateIds != 0)      return getRandom ? GetSwitchinCandidate(healingCandidateIds, battler, lastId, switchType) : bestHealGainId;
        else if (batonPassIds != 0)             return GetSwitchinCandidate(batonPassIds, battler, lastId, switchType);
        else if (generic1v1MonIds != 0)         return GetSwitchinCandidate(generic1v1MonIds, battler, lastId, switchType);
    }

    // Not required to switch here and no good candidates, bail
    if (switchType == SWITCH_MID_BATTLE_OPTIONAL)
        return PARTY_SIZE;

    // Fallback
    if (validMonIds != 0)
        return GetValidSwitchinCandidate(validMonIds, battler, lastId, switchType);

    // If ace mon is the last available Pokemon and U-Turn/Volt Switch or Eject Pack/Button was used - switch to the mon.
    if (aceMonId != PARTY_SIZE && CountUsablePartyMons(battler) <= aceMonCount)
        return aceMonId;

    return PARTY_SIZE;
}

static u32 GetBestMonVanilla(struct Pokemon *party, int lastId, enum BattlerId battler, enum BattlerId opposingBattler, enum BattlerId battlerIn1, enum BattlerId battlerIn2, enum SwitchType switchType)
{
    s32 aceMonCount = 0;
    u32 validMonIds = 0, batonPassIds = 0, typeMatchupIds = 0, bestDamageId = PARTY_SIZE, aceMonId = PARTY_SIZE;
    u32 bestResist = UQ_4_12(2.0), typeMatchup, bestDamage = 0;

    // Save existing battler data
    struct AiLogicData *savedAiLogicData = AllocSaveAiLogicData();
    struct BattlePokemon *savedBattleMons = AllocSaveBattleMons();
    u32 savedNotOnField = gBattleStruct->battlerState[battler].notOnField;

    gBattleStruct->battlerState[battler].notOnField = FALSE;

    // Iterate through mons
    for (u32 monIndex = 0; monIndex < lastId; monIndex++)
    {
        // Check mon validity
        if (!IsValidForBattle(&party[monIndex]))
            continue;
        // Check if mon is already in play or being sent in
        if (IsPartyMonOnFieldOrChosenToSwitch(battler, monIndex, battlerIn1, battlerIn2))
            continue;
        // Check if partner wants to use this mon already
        if (IsPartyMonPlannedToBeSwitchedInByPartner(monIndex, battler))
            continue;
        // Save Ace Pokemon for last
        if (IsAceMon(battler, monIndex))
        {
            aceMonId = monIndex;
            aceMonCount++;
            continue;
        }
        validMonIds |= (1u << monIndex);
        InitializeSwitchinCandidate(battler, monIndex, &party[monIndex]);

        // While not really invalid per se, not really wise to switch into this mon
        if (gAiLogicData->abilities[battler] == ABILITY_TRUANT && IsTruantMonVulnerable(battler, opposingBattler))
            continue;

        typeMatchup = GetBattlerTypeMatchup(opposingBattler, battler);

        for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
        {
            // Check that move has PP remaining before running calcs
            if (gBattleMons[battler].pp[moveIndex] < 1)
                continue;

            enum Move aiMove = gBattleMons[battler].moves[moveIndex];

            // Baton Pass
            if (GetMoveEffect(aiMove) == EFFECT_BATON_PASS)
            {
                batonPassIds |= (1u << monIndex);
            }

            // Type Matchup
            if (typeMatchup < bestResist && gAiLogicData->effectiveness[battler][opposingBattler][moveIndex] >= UQ_4_12(2.0))
            {
                bestResist = typeMatchup;
                typeMatchupIds |= (1u << monIndex);
            }

            // Best damage
            if (aiMove != MOVE_NONE && !IsBattleMoveStatus(aiMove))
            {
                u32 aiDmg = AI_GetDamage(battler, opposingBattler, moveIndex, AI_SWITCHIN_ATTACKING, gAiLogicData);
                if (aiDmg > bestDamage)
                {
                    bestDamage = aiDmg;
                    bestDamageId = monIndex;
                }
            }
        }
    }

    // Restore battler data
    gBattleStruct->battlerState[battler].notOnField = savedNotOnField;
    FreeRestoreAiLogicData(savedAiLogicData);
    FreeRestoreBattleMons(savedBattleMons);
    SetBattlerAiData(battler, gAiLogicData);

    // Baton Pass > Type Matchup > Best Damage
    if (batonPassIds != 0)                  return GetSwitchinCandidate(batonPassIds, battler, lastId, switchType);
    else if (typeMatchupIds != 0)           return GetSwitchinCandidate(typeMatchupIds, battler, lastId, switchType);
    else if (bestDamageId != PARTY_SIZE)    return bestDamageId;

    // Not required to switch here and no good candidates, bail
    if (switchType == SWITCH_MID_BATTLE_OPTIONAL)
        return PARTY_SIZE;

    // Fallback
    if (validMonIds != 0)
        return GetValidSwitchinCandidate(validMonIds, battler, lastId, switchType);

    if (aceMonId != PARTY_SIZE && CountUsablePartyMons(battler) <= aceMonCount)
        return aceMonId;

    return PARTY_SIZE;
}

static u32 GetNextMonInParty(struct Pokemon *party, int lastId, enum BattlerId battler, enum BattlerId battlerIn1, enum BattlerId battlerIn2)
{
    // Iterate through mons
    for (u32 monIndex = 0; monIndex < lastId; monIndex++)
    {
        // Check mon validity
        if (!IsValidForBattle(&party[monIndex]))
        {
            continue;
        }
        // Check if mon is already in play or being sent in
        if (IsPartyMonOnFieldOrChosenToSwitch(battler, monIndex, battlerIn1, battlerIn2))
        {
            continue;
        }
        // Check if partner wants to use this mon already
        if (IsPartyMonPlannedToBeSwitchedInByPartner(monIndex, battler))
        {
            continue;
        }
        return monIndex;
    }
    return PARTY_SIZE;
}

u32 GetMostSuitableMonToSwitchInto(enum BattlerId battler, enum SwitchType switchType)
{
    enum BattlerId opposingBattler = 0;
    u32 bestMonId = PARTY_SIZE;
    enum BattlerId battlerIn1 = 0, battlerIn2 = 0;
    s32 lastId = GetAILastPartyIndex(battler); // + 1
    struct Pokemon *party;

    if (gBattleStruct->monToSwitchIntoId[battler] != PARTY_SIZE)
        return gBattleStruct->monToSwitchIntoId[battler];
    if (gBattleTypeFlags & BATTLE_TYPE_ARENA)
        return gBattlerPartyIndexes[battler] + 1;

    opposingBattler = GetActiveBattlerIds(battler, &battlerIn1, &battlerIn2);
    party = GetBattlerParty(battler);

    if (gAiThinkingStruct->aiFlags[battler] & AI_FLAG_SEQUENCE_SWITCHING)
    {
        bestMonId = GetNextMonInParty(party, lastId, battler, battlerIn1, battlerIn2);
        return bestMonId;
    }

    // Only use better mon selection if AI_FLAG_SMART_MON_CHOICES is set for the trainer.
    if (gAiThinkingStruct->aiFlags[battler] & AI_FLAG_SMART_MON_CHOICES && !IsDoubleBattle()) // Double Battles aren't included in AI_FLAG_SMART_MON_CHOICE. Defaults to regular switch in logic
    {
        bestMonId = GetBestMonIntegrated(party, lastId, battler, opposingBattler, battlerIn1, battlerIn2, switchType);
        return bestMonId;
    }

    // This all handled by the GetBestMonIntegrated function if the AI_FLAG_SMART_MON_CHOICES flag is set
    else
    {
        bestMonId = GetBestMonVanilla(party, lastId, battler, opposingBattler, battlerIn1, battlerIn2, switchType);
        return bestMonId;
    }
}

u32 AI_SelectRevivalBlessingMon(enum BattlerId battler)
{
    s32 lastId = GetAILastPartyIndex(battler); // + 1
    enum BattlerId opposingBattler = 0;
    struct Pokemon *party = GetBattlerParty(battler);
    u32 bestMonId = PARTY_SIZE;
    s32 bestScore = -1;

    if (IsDoubleBattle())
    {
        opposingBattler = BATTLE_OPPOSITE(battler);
        if (gAbsentBattlerFlags & (1u << opposingBattler))
            opposingBattler ^= BIT_FLANK;
    }
    else
    {
        opposingBattler = GetOppositeBattler(battler);
    }

    // Save existing battler data
    struct AiLogicData *savedAiLogicData = AllocSaveAiLogicData();
    struct BattlePokemon *savedBattleMons = AllocSaveBattleMons();
    u32 savedNotOnField = gBattleStruct->battlerState[battler].notOnField;

    gBattleStruct->battlerState[battler].notOnField = FALSE;

    for (u32 monIndex = 0; monIndex < lastId; monIndex++)
    {
        if (GetMonData(&party[monIndex], MON_DATA_HP) != 0)
            continue; // Only consider fainted mons

        bool32 isAceMon = IsAceMon(battler, monIndex);

        InitializeSwitchinCandidate(battler, monIndex, &party[monIndex]);
        gBattleMons[battler].hp = gBattleMons[battler].maxHP / 2; // Revival Blessing restores half HP
        gBattleMons[battler].status1 = 0;

        // Avoid reviving something that will immediately faint to hazards
        if (GetSwitchinHazardsDamage(battler) >= gBattleMons[battler].hp)
            continue;

        s32 bestDamage = 0;
        for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
        {
            enum Move aiMove = gBattleMons[battler].moves[moveIndex];
            if (aiMove == MOVE_NONE || gBattleMons[battler].pp[moveIndex] == 0)
                continue;

            s32 damage = AI_GetDamage(battler, opposingBattler, moveIndex, AI_SWITCHIN_ATTACKING, gAiLogicData);
            if (damage > bestDamage)
                bestDamage = damage;
        }

        uq4_12_t typeMatchup = GetBattlerTypeMatchup(opposingBattler, battler);
        s32 score = bestDamage + gBattleMons[battler].maxHP;

        // Reviving the ace is usually high value. give it a small bonus but still let matchup/coverage decide
        if (isAceMon)
            score += gBattleMons[battler].maxHP / 3;

        // Prefer mons that don't face a terrible defensive matchup on entry
        if (typeMatchup < UQ_4_12(1.0))
            score += gBattleMons[battler].maxHP / 4;
        else if (typeMatchup > UQ_4_12(4.0))
            score -= gBattleMons[battler].maxHP / 4;

        if (score > bestScore)
        {
            bestScore = score;
            bestMonId = monIndex;
        }
    }

    // Restore battler data
    gBattleStruct->battlerState[battler].notOnField = savedNotOnField;
    FreeRestoreAiLogicData(savedAiLogicData);
    FreeRestoreBattleMons(savedBattleMons);
    SetBattlerAiData(battler, gAiLogicData);

    if (bestMonId == PARTY_SIZE)
        bestMonId = GetFirstFaintedPartyIndex(battler);

    return bestMonId;
}

static void SetBattlerStatusForSwitchin(enum BattlerId battler)
{
    u32 tSpikesLayers = gSideTimers[GetBattlerSide(battler)].toxicSpikesAmount;
    if (tSpikesLayers != 0 && IsSwitchinTSpikesAffected(battler))
    {
        if (tSpikesLayers == 1)
            gBattleMons[battler].status1 = STATUS1_POISON;
        if (tSpikesLayers == 2)
        {
            gBattleMons[battler].status1 = STATUS1_TOXIC_POISON;
            gBattleMons[battler].status1 += STATUS1_TOXIC_TURN(1);
        }
    }
}

static void SetBattlerStatStagesForSwitchin(enum BattlerId battler, enum BattlerId opposingBattler, u32 fieldStatus)
{
    u32 aiAbility = gAiLogicData->abilities[battler];
    enum Item aiItem = gAiLogicData->items[battler];
    bool32 isStickyWebsAffected = (IsHazardOnSide(GetBattlerSide(battler), HAZARDS_STICKY_WEB) && IsBattlerAffectedByHazards(battler, GetItemHoldEffect(aiItem), FALSE) && IsBattlerGrounded(battler, gAiLogicData->abilities[battler], GetItemHoldEffect(aiItem)));
    bool32 opponentStatDrop = FALSE;

    // Ability stat changes
    switch(aiAbility)
    {
    case ABILITY_INTREPID_SWORD:
        gBattleMons[battler].statStages[STAT_ATK] += 1;
        break;
    case ABILITY_DAUNTLESS_SHIELD:
        gBattleMons[battler].statStages[STAT_DEF] += 1;
        break;
    case ABILITY_SUPREME_OVERLORD:
        break;
    case ABILITY_DOWNLOAD:
        gBattleMons[battler].statStages[GetDownloadStat(battler)] += 1;
        break;
    case ABILITY_INTIMIDATE:
        if (CanLowerStat(battler, opposingBattler, gAiLogicData, STAT_ATK))
        {
            if (gAiLogicData->abilities[opposingBattler] == ABILITY_CONTRARY)
            {
                gBattleMons[opposingBattler].statStages[STAT_ATK] += 1;
            }
            else
            {
                opponentStatDrop = TRUE;
                gBattleMons[opposingBattler].statStages[STAT_ATK] -= 1;
                if (gAiLogicData->abilities[opposingBattler] == ABILITY_DEFIANT)
                    gBattleMons[opposingBattler].statStages[STAT_ATK] += 2;
                if (gAiLogicData->abilities[opposingBattler] == ABILITY_COMPETITIVE)
                    gBattleMons[opposingBattler].statStages[STAT_SPATK] += 2;
            }
        }
        break;
    case ABILITY_SUPERSWEET_SYRUP:
        if (CanLowerStat(battler, opposingBattler, gAiLogicData, STAT_EVASION))
        {
            if (gAiLogicData->abilities[opposingBattler] == ABILITY_CONTRARY)
            {
                gBattleMons[opposingBattler].statStages[STAT_EVASION] += 1;
            }
            else
            {
                opponentStatDrop = TRUE;
                gBattleMons[opposingBattler].statStages[STAT_EVASION] -= 1;
                if (gAiLogicData->abilities[opposingBattler] == ABILITY_DEFIANT)
                    gBattleMons[opposingBattler].statStages[STAT_ATK] += 2;
                if (gAiLogicData->abilities[opposingBattler] == ABILITY_COMPETITIVE)
                    gBattleMons[opposingBattler].statStages[STAT_SPATK] += 2;
            }
        }
        break;
    case ABILITY_WIND_RIDER:
        if (gSideStatuses[GetBattlerSide(battler)] & SIDE_STATUS_TAILWIND)
            gBattleMons[battler].statStages[STAT_ATK] += 1;
        break;
    case ABILITY_DEFIANT:
        if (isStickyWebsAffected)
            gBattleMons[battler].statStages[STAT_ATK] += 2;
        break;
    case ABILITY_COMPETITIVE:
        if (isStickyWebsAffected)
            gBattleMons[battler].statStages[STAT_SPATK] += 2;
        break;
    case ABILITY_CONTRARY:
        if (isStickyWebsAffected)
            gBattleMons[battler].statStages[STAT_SPEED] += 1;
    default:
        break;
    }

    // Item stat changes
    switch(GetItemHoldEffect(aiItem))
    {
    case HOLD_EFFECT_TERRAIN_SEED:
    {
        u32 seedParam = GetItemHoldEffectParam(aiItem);
        if ((seedParam == HOLD_EFFECT_PARAM_ELECTRIC_TERRAIN && (fieldStatus & STATUS_FIELD_ELECTRIC_TERRAIN))
         || (seedParam == HOLD_EFFECT_PARAM_GRASSY_TERRAIN && (fieldStatus & STATUS_FIELD_GRASSY_TERRAIN))
         || (seedParam == HOLD_EFFECT_PARAM_MISTY_TERRAIN && (fieldStatus & STATUS_FIELD_MISTY_TERRAIN))
         || (seedParam == HOLD_EFFECT_PARAM_PSYCHIC_TERRAIN && (fieldStatus & STATUS_FIELD_PSYCHIC_TERRAIN)))
            gBattleMons[battler].statStages[STAT_DEF] += 1;
        break;
    }
    case HOLD_EFFECT_ATTACK_UP:
        if (HasEnoughHpToEatBerry(battler, aiAbility, GetItemHoldEffectParam(aiItem), aiItem))
            gBattleMons[battler].statStages[STAT_ATK] += 1;
        break;
    case HOLD_EFFECT_DEFENSE_UP:
        if (HasEnoughHpToEatBerry(battler, aiAbility, GetItemHoldEffectParam(aiItem), aiItem))
            gBattleMons[battler].statStages[STAT_DEF] += 1;
        break;
    case HOLD_EFFECT_SPEED_UP:
        if (HasEnoughHpToEatBerry(battler, aiAbility, GetItemHoldEffectParam(aiItem), aiItem))
            gBattleMons[battler].statStages[STAT_SPEED] += 1;
        break;
    case HOLD_EFFECT_SP_ATTACK_UP:
        if (HasEnoughHpToEatBerry(battler, aiAbility, GetItemHoldEffectParam(aiItem), aiItem))
            gBattleMons[battler].statStages[STAT_SPATK] += 1;
        break;
    case HOLD_EFFECT_SP_DEFENSE_UP:
        if (HasEnoughHpToEatBerry(battler, aiAbility, GetItemHoldEffectParam(aiItem), aiItem))
            gBattleMons[battler].statStages[STAT_SPDEF] += 1;
        break;
    case HOLD_EFFECT_ROOM_SERVICE:
        if (gFieldStatuses & STATUS_FIELD_TRICK_ROOM)
            gBattleMons[battler].statStages[STAT_SPEED] -= 1;
    case HOLD_EFFECT_MIRROR_HERB:
        if (opponentStatDrop && gAiLogicData->abilities[opposingBattler] == ABILITY_DEFIANT)
            gBattleMons[battler].statStages[STAT_ATK] += 2;
        if (opponentStatDrop && gAiLogicData->abilities[opposingBattler] == ABILITY_COMPETITIVE)
            gBattleMons[battler].statStages[STAT_SPATK] += 2;
        break;
    default:
        break;
    }

    // Hazard stat changes
    if (isStickyWebsAffected && GetItemHoldEffect(aiItem) != HOLD_EFFECT_WHITE_HERB)
        gBattleMons[battler].statStages[STAT_SPEED] -= 1;
}

static void SetBattlerHPChangeForSwitch(enum BattlerId battler, enum BattlerId opposingBattler)
{
    s32 maxHP = gBattleMons[battler].maxHP;
    s32 currentHP = gBattleMons[battler].hp - GetSwitchinHazardsDamage(battler);
    s32 itemHeal = GetSwitchinSingleUseItemHealing(battler, opposingBattler, currentHP);

    if (itemHeal > 0)
    {
        if ((currentHP + itemHeal) > maxHP)
            currentHP = maxHP;
        else
            currentHP = currentHP + itemHeal;
    }
    gBattleMons[battler].hp = currentHP;
}

// Set potential field effect from ability for switch in
static void SetBattlerVolatilesForSwitchin(enum BattlerId battler, u32 weather, u32 fieldStatus)
{
    enum Item aiItem = gAiLogicData->items[battler];
    switch (gAiLogicData->abilities[battler])
    {
    case ABILITY_VESSEL_OF_RUIN:
        gBattleMons[battler].volatiles.vesselOfRuin = TRUE;
        break;
    case ABILITY_SWORD_OF_RUIN:
        gBattleMons[battler].volatiles.swordOfRuin = TRUE;
        break;
    case ABILITY_TABLETS_OF_RUIN:
        gBattleMons[battler].volatiles.tabletsOfRuin = TRUE;
        break;
    case ABILITY_BEADS_OF_RUIN:
        gBattleMons[battler].volatiles.beadsOfRuin = TRUE;
        break;
    case ABILITY_QUARK_DRIVE:
        if ((fieldStatus & STATUS_FIELD_ELECTRIC_TERRAIN) || GetItemHoldEffect(aiItem) == HOLD_EFFECT_BOOSTER_ENERGY)
            gBattleMons[battler].volatiles.boosterEnergyActivated = TRUE;
        break;
    case ABILITY_PROTOSYNTHESIS:
        if ((weather & B_WEATHER_SUN) || GetItemHoldEffect(aiItem) == HOLD_EFFECT_BOOSTER_ENERGY)
            gBattleMons[battler].volatiles.boosterEnergyActivated = TRUE;
        break;
    case ABILITY_WIND_POWER:
        if (gSideStatuses[GetBattlerSide(battler)] & SIDE_STATUS_TAILWIND)
            gBattleMons[battler].volatiles.chargeTimer = 2;
        break;
    default:
        break;
    }
}
