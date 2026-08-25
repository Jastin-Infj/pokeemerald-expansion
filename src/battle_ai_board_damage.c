#include "global.h"
#include "battle.h"
#include "battle_ai_board_sim.h"
#include "battle_main.h"
#include "battle_z_move.h"
#include "fpmath.h"
#include "item.h"
#include "move.h"
#include "constants/abilities.h"
#include "constants/battle.h"
#include "constants/battle_move_effects.h"
#include "constants/items.h"
#include "constants/hold_effects.h"
#include "constants/moves.h"
#include "constants/pokemon.h"

static const struct AiSimCombatProfile *GetProfile(const struct AiSimContext *context,
                                                   const struct AiSimBoard *board,
                                                   enum BattlerId battler)
{
    u32 rosterIndex;
    const struct AiSimMonTemplate *mon;

    if (context == NULL || board == NULL || battler >= MAX_BATTLERS_COUNT)
        return NULL;
    if (!(board->activeMask & (1u << battler)))
        return NULL;

    rosterIndex = board->active[battler].rosterIndex;
    if (rosterIndex >= AI_SIM_ROSTER_COUNT)
        return NULL;
    mon = &context->mons[rosterIndex];
    if (!(mon->flags & AI_SIM_MON_PRESENT))
        return NULL;

    if (board->active[battler].flags & AI_SIM_ACTIVE_TRANSFORMED)
    {
        if (!(mon->transformed.flags & AI_SIM_PROFILE_VALID))
            return NULL;
        return &mon->transformed;
    }

    if (!(mon->normal.flags & AI_SIM_PROFILE_VALID))
        return NULL;
    return &mon->normal;
}

static const struct AiSimPartyState *GetPartyState(const struct AiSimBoard *board,
                                                   enum BattlerId battler)
{
    u32 rosterIndex;

    if (board == NULL || battler >= MAX_BATTLERS_COUNT)
        return NULL;
    rosterIndex = board->active[battler].rosterIndex;
    if (rosterIndex >= AI_SIM_ROSTER_COUNT)
        return NULL;
    return &board->party[rosterIndex];
}

static u32 ApplyStatStage(u32 stat, s32 stage)
{
    stage = min(MAX_STAT_STAGE, max(MIN_STAT_STAGE, stage));
    if (stage >= DEFAULT_STAT_STAGE)
        return stat * (2 + stage - DEFAULT_STAT_STAGE) / 2;
    return stat * 2 / (2 + DEFAULT_STAT_STAGE - stage);
}

static bool32 IsSupportedDamageAbility(u32 ability)
{
    switch (ability)
    {
    case ABILITY_NONE:
    case ABILITY_GUTS:
    case ABILITY_FAIRY_AURA:
    case ABILITY_PRESSURE:
    case ABILITY_PRANKSTER:
    case ABILITY_PRISM_ARMOR:
    case ABILITY_NEUROFORCE:
    case ABILITY_STALL:
    case ABILITY_STURDY:
    case ABILITY_TECHNICIAN:
    case ABILITY_INTIMIDATE:
        return TRUE;
    default:
        return FALSE;
    }
}

static bool32 IsSupportedDamageItem(u32 item)
{
    u32 holdEffect;

    if (item == ITEM_NONE)
        return TRUE;
    if (item >= ITEMS_COUNT)
        return FALSE;
    holdEffect = gItemsInfo[item].holdEffect;
    switch (holdEffect)
    {
    case HOLD_EFFECT_NONE:
    case HOLD_EFFECT_POWER_HERB:
    case HOLD_EFFECT_LIFE_ORB:
    case HOLD_EFFECT_LAGGING_TAIL:
    case HOLD_EFFECT_FOCUS_SASH:
    case HOLD_EFFECT_WEAKNESS_POLICY:
    case HOLD_EFFECT_MEGA_STONE:
    case HOLD_EFFECT_PRIMAL_ORB:
    case HOLD_EFFECT_Z_CRYSTAL:
    case HOLD_EFFECT_WIDE_LENS:
    case HOLD_EFFECT_ZOOM_LENS:
    case HOLD_EFFECT_PROTECTIVE_PADS:
    case HOLD_EFFECT_SAFETY_GOGGLES:
    case HOLD_EFFECT_SHED_SHELL:
    case HOLD_EFFECT_HEAVY_DUTY_BOOTS:
    case HOLD_EFFECT_CHOICE_BAND:
    case HOLD_EFFECT_CHOICE_SCARF:
    case HOLD_EFFECT_CHOICE_SPECS:
        return TRUE;
    case HOLD_EFFECT_RESTORE_PCT_HP:
        return item == ITEM_SITRUS_BERRY;
    default:
        return FALSE;
    }
}

static bool32 IsSupportedFixedPowerMove(u32 move)
{
    u32 effect;

    if (move == MOVE_NONE || move >= MOVES_COUNT_ALL || gMovesInfo[move].power == 0)
        return FALSE;
    if (gMovesInfo[move].multiHit)
        return FALSE;
    if (move >= FIRST_Z_MOVE && move <= LAST_Z_MOVE)
        return TRUE;
    if (move >= FIRST_MAX_MOVE && move <= LAST_MAX_MOVE)
        return FALSE;

    effect = gMovesInfo[move].effect;
    return effect == EFFECT_HIT
        || effect == EFFECT_PSYSHOCK
        || effect == EFFECT_EARTHQUAKE
        || effect == EFFECT_KNOCK_OFF
        || (effect == EFFECT_RECOIL
         && move == MOVE_FLARE_BLITZ
         && gMovesInfo[move].argument.recoilPercentage == 33)
        || (effect == EFFECT_TERRAIN_BOOST
         && move == MOVE_EXPANDING_FORCE
         && gMovesInfo[move].argument.terrainBoost.terrain == STATUS_FIELD_PSYCHIC_TERRAIN
         && gMovesInfo[move].argument.terrainBoost.groundCheck == GROUND_CHECK_USER
         && gMovesInfo[move].argument.terrainBoost.hitsBothFoes)
        || (effect == EFFECT_FIRST_TURN_ONLY && move == MOVE_FAKE_OUT);
}

static bool32 ProfileHasType(const struct AiSimCombatProfile *profile, u32 type)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(profile->types); i++)
    {
        if (profile->types[i] == type && type != TYPE_NONE)
            return TRUE;
    }
    return FALSE;
}

static bool32 IsSimBattlerGrounded(const struct AiSimContext *context,
                                   const struct AiSimBoard *board,
                                   enum BattlerId battler)
{
    const struct AiSimCombatProfile *profile = GetProfile(context, board, battler);
    const struct AiSimMonTemplate *mon;

    if (profile == NULL)
        return FALSE;
    mon = &context->mons[board->active[battler].rosterIndex];
    if (board->active[battler].flags & AI_SIM_ACTIVE_TERA)
        return mon->teraType != TYPE_FLYING && mon->teraType != TYPE_STELLAR;
    return !ProfileHasType(profile, TYPE_FLYING);
}

static bool32 IsTerrainBoostActive(const struct AiSimContext *context,
                                   const struct AiSimBoard *board,
                                   const struct AiSimAction *action)
{
    const struct MoveInfo *move = &gMovesInfo[action->choice];

    if (move->effect != EFFECT_TERRAIN_BOOST
     || !(board->fieldStatuses & move->argument.terrainBoost.terrain))
        return FALSE;
    if (move->argument.terrainBoost.groundCheck == GROUND_CHECK_USER)
        return IsSimBattlerGrounded(context, board, action->actor);
    return move->argument.terrainBoost.groundCheck == GROUND_CHECK_NONE;
}

static uq4_12_t GetSimTypeModifier(const struct AiSimContext *context,
                                   const struct AiSimBoard *board,
                                   enum BattlerId target,
                                   u32 moveType)
{
    const struct AiSimCombatProfile *profile = GetProfile(context, board, target);
    const struct AiSimMonTemplate *mon;
    uq4_12_t modifier = UQ_4_12(1.0);
    u32 i;

    if (profile == NULL || moveType >= NUMBER_OF_MON_TYPES)
        return UQ_4_12(0.0);
    mon = &context->mons[board->active[target].rosterIndex];

    if (board->active[target].flags & AI_SIM_ACTIVE_TERA)
    {
        if (mon->teraType >= NUMBER_OF_MON_TYPES || mon->teraType == TYPE_STELLAR)
            return UQ_4_12(0.0);
        return gTypeEffectivenessTable[moveType][mon->teraType];
    }

    for (i = 0; i < ARRAY_COUNT(profile->types); i++)
    {
        u32 type = profile->types[i];
        if (type != TYPE_NONE && type < NUMBER_OF_MON_TYPES)
            modifier = uq4_12_multiply(modifier, gTypeEffectivenessTable[moveType][type]);
    }
    return modifier;
}

static bool32 IsSpreadTarget(u32 target)
{
    return target == TARGET_BOTH
        || target == TARGET_FOES_AND_ALLY
        || target == TARGET_ALL_BATTLERS;
}

static bool32 IsActionSpread(const struct AiSimContext *context,
                             const struct AiSimBoard *board,
                             const struct AiSimAction *action)
{
    const struct MoveInfo *move = &gMovesInfo[action->choice];

    return IsSpreadTarget(move->target)
        || (move->argument.terrainBoost.hitsBothFoes
         && IsTerrainBoostActive(context, board, action));
}

static u32 CountSpreadTargets(const struct AiSimContext *context,
                              const struct AiSimBoard *board,
                              const struct AiSimAction *action)
{
    u32 battler;
    u32 count = 0;
    u32 targetType = gMovesInfo[action->choice].target;
    bool32 terrainSpread = !IsSpreadTarget(targetType)
        && IsActionSpread(context, board, action);

    for (battler = 0; battler < MAX_BATTLERS_COUNT; battler++)
    {
        u32 rosterIndex;
        bool32 selected = FALSE;

        if (battler == action->actor || !(board->activeMask & (1u << battler)))
            continue;
        rosterIndex = board->active[battler].rosterIndex;
        if (rosterIndex >= AI_SIM_ROSTER_COUNT || board->party[rosterIndex].hp == 0)
            continue;

        if (terrainSpread || targetType == TARGET_BOTH)
            selected = ((battler ^ action->actor) & BIT_SIDE) != 0;
        else if (targetType == TARGET_FOES_AND_ALLY || targetType == TARGET_ALL_BATTLERS)
            selected = TRUE;
        if (selected)
            count++;
    }
    return count;
}

u32 AiSim_GetEffectiveSpeed(const struct AiSimContext *context,
                            const struct AiSimBoard *board,
                            enum BattlerId battler)
{
    const struct AiSimCombatProfile *profile;
    const struct AiSimPartyState *party;
    u32 speed;

    if (context == NULL || board == NULL
     || context->battlersCount == 0
     || context->battlersCount > MAX_BATTLERS_COUNT
     || battler >= MAX_BATTLERS_COUNT
     || battler >= context->battlersCount)
        return 0;
    profile = GetProfile(context, board, battler);
    party = GetPartyState(board, battler);
    if (profile == NULL || party == NULL || party->item >= ITEMS_COUNT)
        return 0;

    speed = ApplyStatStage(profile->speed, board->active[battler].statStages[STAT_SPEED]);
    if (party->status1 & STATUS1_PARALYSIS)
        speed /= 2;
    if (board->sides[battler & BIT_SIDE].statuses & SIDE_STATUS_TAILWIND)
        speed *= 2;
    if (!(board->active[battler].flags & AI_SIM_ACTIVE_DYNAMAX)
     && party->item != ITEM_NONE
     && gItemsInfo[party->item].holdEffect == HOLD_EFFECT_CHOICE_SCARF)
        speed = speed * 3 / 2;
    return max(1, speed);
}

bool32 AiSim_CalcDamage(const struct AiSimContext *context,
                        const struct AiSimBoard *board,
                        const struct AiSimAction *action,
                        enum BattlerId target,
                        struct SimulatedDamage *damage)
{
    const struct AiSimCombatProfile *attacker;
    const struct AiSimCombatProfile *defender;
    const struct AiSimMonTemplate *attackerMon;
    const struct AiSimMonTemplate *defenderMon;
    const struct AiSimPartyState *attackerParty;
    const struct AiSimPartyState *defenderParty;
    const struct MoveInfo *move;
    const struct MoveInfo *baseMove;
    uq4_12_t movePowerModifier;
    uq4_12_t typeModifier;
    u32 attackStat;
    u32 defenseStat;
    u32 baseDamage;
    u32 modified;
    u32 movePower;
    u32 moveType;
    enum Move baseMoveId;
    u32 stage;
    bool32 physical;
    bool32 targetsPhysicalDefense;
    bool32 tera;
    bool32 originalStab;
    bool32 isZMove;

    if (damage != NULL)
        *damage = (struct SimulatedDamage){0};
    if (context == NULL || board == NULL || action == NULL || damage == NULL
     || context->battlersCount == 0
     || context->battlersCount > MAX_BATTLERS_COUNT
     || action->kind != AI_SIM_ACTION_MOVE || action->actor >= MAX_BATTLERS_COUNT
     || target >= MAX_BATTLERS_COUNT
     || action->actor >= context->battlersCount
     || target >= context->battlersCount
     || !IsSupportedFixedPowerMove(action->choice))
        return FALSE;

    attacker = GetProfile(context, board, action->actor);
    defender = GetProfile(context, board, target);
    attackerParty = GetPartyState(board, action->actor);
    defenderParty = GetPartyState(board, target);
    if (attacker == NULL || defender == NULL || attackerParty == NULL || defenderParty == NULL)
        return FALSE;
    attackerMon = &context->mons[board->active[action->actor].rosterIndex];
    defenderMon = &context->mons[board->active[target].rosterIndex];
    if (!(attackerMon->flags & AI_SIM_MON_MOVES_KNOWN)
     || !(attackerMon->flags & AI_SIM_MON_PROFILE_KNOWN)
     || !(defenderMon->flags & AI_SIM_MON_PROFILE_KNOWN)
     || !(attacker->flags & AI_SIM_PROFILE_ABILITY_KNOWN)
     || !(attacker->flags & AI_SIM_PROFILE_TYPES_KNOWN)
     || !(defender->flags & AI_SIM_PROFILE_ABILITY_KNOWN)
     || !(defender->flags & AI_SIM_PROFILE_TYPES_KNOWN)
     || !(attackerParty->flags & AI_SIM_PARTY_ITEM_KNOWN)
     || !(attackerParty->flags & AI_SIM_PARTY_STATUS_KNOWN)
     || !(defenderParty->flags & AI_SIM_PARTY_ITEM_KNOWN)
     || !(defenderParty->flags & AI_SIM_PARTY_STATUS_KNOWN))
        return FALSE;
    if (!IsSupportedDamageAbility(attacker->ability)
     || !IsSupportedDamageAbility(defender->ability)
     || !IsSupportedDamageItem(attackerParty->item)
     || !IsSupportedDamageItem(defenderParty->item))
        return FALSE;

    move = &gMovesInfo[action->choice];
    isZMove = action->choice >= FIRST_Z_MOVE && action->choice <= LAST_Z_MOVE;
    baseMoveId = action->choice;
    if (isZMove)
    {
        if (action->gimmick != GIMMICK_Z_MOVE || action->moveSlot >= MAX_MON_MOVES)
            return FALSE;
        baseMoveId = attackerMon->moves[action->moveSlot];
        if (baseMoveId == MOVE_NONE || baseMoveId >= MOVES_COUNT
         || gMovesInfo[baseMoveId].category == DAMAGE_CATEGORY_STATUS)
            return FALSE;
    }
    baseMove = &gMovesInfo[baseMoveId];
    moveType = move->type;
    tera = (board->active[action->actor].flags & AI_SIM_ACTIVE_TERA)
        || action->gimmick == GIMMICK_TERA;
    if (moveType >= NUMBER_OF_MON_TYPES || moveType == TYPE_STELLAR
     || (tera && attackerMon->teraType == TYPE_STELLAR)
     || ((board->active[target].flags & AI_SIM_ACTIVE_TERA) && defenderMon->teraType == TYPE_STELLAR))
        return FALSE;

    physical = baseMove->category == DAMAGE_CATEGORY_PHYSICAL;
    if (!physical && baseMove->category != DAMAGE_CATEGORY_SPECIAL)
        return FALSE;
    // Psyshock/Psystrike keep their special category for the user's stat,
    // Choice Specs, Frostbite, and screen modifiers, but read the target's
    // physical Defense and Defense stage.
    targetsPhysicalDefense = physical || move->effect == EFFECT_PSYSHOCK;

    stage = physical ? STAT_ATK : STAT_SPATK;
    attackStat = ApplyStatStage(physical ? attacker->attack : attacker->spAttack,
                                board->active[action->actor].statStages[stage]);
    if (physical && attacker->ability == ABILITY_GUTS && attackerParty->status1 & STATUS1_ANY)
        attackStat = attackStat * 3 / 2;
    if (!(board->active[action->actor].flags & AI_SIM_ACTIVE_DYNAMAX)
     && attackerParty->item != ITEM_NONE)
    {
        u32 holdEffect = gItemsInfo[attackerParty->item].holdEffect;
        if ((physical && holdEffect == HOLD_EFFECT_CHOICE_BAND)
         || (!physical && holdEffect == HOLD_EFFECT_CHOICE_SPECS))
            attackStat = attackStat * 3 / 2;
    }
    stage = targetsPhysicalDefense ? STAT_DEF : STAT_SPDEF;
    defenseStat = ApplyStatStage(targetsPhysicalDefense ? defender->defense : defender->spDefense,
                                 board->active[target].statStages[stage]);
    if (defenseStat == 0)
        return FALSE;

    movePower = isZMove ? GetZMovePower(baseMoveId) : move->power;
    if (!isZMove && IsTerrainBoostActive(context, board, action))
        movePower = movePower * (100 + move->argument.terrainBoost.percent) / 100;

    // Live damage accumulates these supported base-power modifiers in one
    // uq4.12 product, rounds the resulting power half-down, and only then
    // applies the Terastal 60-BP floor.  Keeping them after the +2 base-damage
    // term changes both rounding and the floor boundary.
    movePowerModifier = UQ_4_12(1.0);
    if (B_KNOCK_OFF_DMG >= GEN_6
     && move->effect == EFFECT_KNOCK_OFF
     && AiSim_IsItemRemovable(context, board, target))
        movePowerModifier = uq4_12_multiply(movePowerModifier, UQ_4_12(1.5));
    if ((board->fieldStatuses & STATUS_FIELD_PSYCHIC_TERRAIN)
     && moveType == TYPE_PSYCHIC
     && IsSimBattlerGrounded(context, board, action->actor))
    {
#if B_TERRAIN_TYPE_BOOST >= GEN_8
        movePowerModifier = uq4_12_multiply(movePowerModifier, UQ_4_12(1.3));
#else
        movePowerModifier = uq4_12_multiply(movePowerModifier, UQ_4_12(1.5));
#endif
    }
    if (attacker->ability == ABILITY_TECHNICIAN && movePower <= 60)
        movePowerModifier = uq4_12_multiply(movePowerModifier, UQ_4_12(1.5));
    if (moveType == TYPE_FAIRY)
    {
        u32 battler;

        for (battler = 0; battler < MAX_BATTLERS_COUNT; battler++)
        {
            const struct AiSimCombatProfile *profile;
            u32 rosterIndex;

            if (!(board->activeMask & (1u << battler)))
                continue;
            rosterIndex = board->active[battler].rosterIndex;
            if (rosterIndex >= AI_SIM_ROSTER_COUNT
             || board->party[rosterIndex].hp == 0)
                continue;
            profile = GetProfile(context, board, battler);
            if (profile != NULL && profile->ability == ABILITY_FAIRY_AURA)
            {
                movePowerModifier = uq4_12_multiply(movePowerModifier,
                                                     UQ_4_12(1.33));
                break;
            }
        }
    }
    movePower = uq4_12_multiply_by_int_half_down(movePowerModifier, movePower);
    if (tera
     && attackerMon->teraType == moveType
     && movePower < 60
     && move->power > 1
     && move->strikeCount < 2
     && !move->multiHit
     && move->effect != EFFECT_POWER_BASED_ON_USER_HP
     && move->effect != EFFECT_POWER_BASED_ON_TARGET_HP
     && move->priority == 0)
        movePower = 60;

    baseDamage = (((2 * max(1, attackerMon->level) / 5 + 2) * movePower * attackStat / defenseStat) / 50) + 2;
    if (attackerParty->item != ITEM_NONE
     && gItemsInfo[attackerParty->item].holdEffect == HOLD_EFFECT_LIFE_ORB)
        baseDamage = baseDamage * 13 / 10;
    if (IsActionSpread(context, board, action)
     && CountSpreadTargets(context, board, action) > 1)
        baseDamage = baseDamage * 3 / 4;

    if ((board->weather & B_WEATHER_RAIN) && moveType == TYPE_WATER)
        baseDamage = baseDamage * 3 / 2;
    else if ((board->weather & B_WEATHER_RAIN) && moveType == TYPE_FIRE)
        baseDamage /= 2;
    else if ((board->weather & B_WEATHER_SUN) && moveType == TYPE_FIRE)
        baseDamage = baseDamage * 3 / 2;
    else if ((board->weather & B_WEATHER_SUN) && moveType == TYPE_WATER)
        baseDamage /= 2;

    originalStab = ProfileHasType(attacker, moveType);
    if (tera && attackerMon->teraType == moveType)
        baseDamage = originalStab ? baseDamage * 2 : baseDamage * 3 / 2;
    else if (originalStab)
        baseDamage = baseDamage * 3 / 2;

    typeModifier = GetSimTypeModifier(context, board, target, moveType);
    if (typeModifier == UQ_4_12(0.0))
        return TRUE;
    baseDamage = uq4_12_multiply_by_int_half_down(typeModifier, baseDamage);
    if (typeModifier > UQ_4_12(1.0))
    {
        if (attacker->ability == ABILITY_NEUROFORCE)
            baseDamage = baseDamage * 5 / 4;
        if (defender->ability == ABILITY_PRISM_ARMOR)
            baseDamage = baseDamage * 3 / 4;
    }

    if (physical && (attackerParty->status1 & STATUS1_BURN) && attacker->ability != ABILITY_GUTS)
        baseDamage /= 2;
    if (!physical && (attackerParty->status1 & STATUS1_FROSTBITE))
        baseDamage /= 2;

    if (physical && (board->sides[target & BIT_SIDE].statuses & (SIDE_STATUS_REFLECT | SIDE_STATUS_AURORA_VEIL)))
        baseDamage = baseDamage * (context->battlersCount > 2 ? 2 : 1) / (context->battlersCount > 2 ? 3 : 2);
    else if (!physical && (board->sides[target & BIT_SIDE].statuses & (SIDE_STATUS_LIGHTSCREEN | SIDE_STATUS_AURORA_VEIL)))
        baseDamage = baseDamage * (context->battlersCount > 2 ? 2 : 1) / (context->battlersCount > 2 ? 3 : 2);

    baseDamage = max(1, baseDamage);
    modified = max(1, baseDamage * 85 / 100);
    damage->minimum = min(UINT16_MAX, modified * max(1, move->strikeCount));
    modified = max(1, baseDamage * 93 / 100);
    damage->median = min(UINT16_MAX, modified * max(1, move->strikeCount));
    damage->maximum = min(UINT16_MAX, baseDamage * max(1, move->strikeCount));
    damage->random = damage->median;
    return TRUE;
}
