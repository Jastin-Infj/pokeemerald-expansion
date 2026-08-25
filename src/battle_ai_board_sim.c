#include "global.h"
#include "battle.h"
#include "battle_ai_board_sim.h"
#include "battle_gimmick.h"
#include "battle_main.h"
#include "battle_z_move.h"
#include "fpmath.h"
#include "item.h"
#include "mail.h"
#include "move.h"
#include "pokemon.h"
#include "constants/abilities.h"
#include "constants/battle.h"
#include "constants/battle_move_effects.h"
#include "constants/hold_effects.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/pokemon.h"

#define AI_SIM_SCREEN_TURNS 5
#define AI_SIM_ROOM_TURNS 5

#if TESTING
static EWRAM_DATA u32 sAiSimLastOutcomeApplicationCount;

u32 Test_AiSim_GetLastOutcomeApplicationCount(void)
{
    return sAiSimLastOutcomeApplicationCount;
}
#endif

static u16 GetCurrentMaxHp(const struct AiSimContext *context,
                           const struct AiSimBoard *board,
                           enum BattlerId battler);
static s32 GetSimMovePriority(const struct AiSimContext *context,
                              const struct AiSimBoard *board,
                              const struct AiSimAction *action);
static bool32 MovesLastInBracket(const struct AiSimContext *context,
                                 const struct AiSimBoard *board,
                                 enum BattlerId battler);
static uq4_12_t GetSimTypeModifier(const struct AiSimContext *context,
                                   const struct AiSimBoard *board,
                                   const struct AiSimAction *action,
                                   enum BattlerId target);
static u32 SelectDamageRoll(const struct AiSimContext *context,
                            const struct AiSimBoard *board,
                            const struct AiSimAction *action,
                            enum BattlerId target,
                            const struct AiSimOutcomeKey *outcome,
                            const struct SimulatedDamage *damage);

static const struct AiSimCombatProfile *GetProfile(const struct AiSimContext *context,
                                                   const struct AiSimBoard *board,
                                                   enum BattlerId battler)
{
    u32 rosterIndex;
    const struct AiSimMonTemplate *mon;

    if (context == NULL || board == NULL || battler >= MAX_BATTLERS_COUNT
     || !(board->activeMask & (1u << battler)))
        return NULL;
    rosterIndex = board->active[battler].rosterIndex;
    if (rosterIndex >= AI_SIM_ROSTER_COUNT)
        return NULL;
    mon = &context->mons[rosterIndex];
    if (!(mon->flags & AI_SIM_MON_PRESENT))
        return NULL;
    if (board->active[battler].flags & AI_SIM_ACTIVE_TRANSFORMED)
        return (mon->transformed.flags & AI_SIM_PROFILE_VALID) ? &mon->transformed : NULL;
    return (mon->normal.flags & AI_SIM_PROFILE_VALID) ? &mon->normal : NULL;
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

static bool32 IsSimBattlerOfType(const struct AiSimContext *context,
                                 const struct AiSimBoard *board,
                                 enum BattlerId battler,
                                 u32 type)
{
    const struct AiSimCombatProfile *profile = GetProfile(context, board, battler);
    const struct AiSimMonTemplate *mon;

    if (profile == NULL)
        return FALSE;
    mon = &context->mons[board->active[battler].rosterIndex];
    if (board->active[battler].flags & AI_SIM_ACTIVE_TERA)
        return mon->teraType == type;
    return ProfileHasType(profile, type);
}

static struct AiSimPartyState *GetPartyState(struct AiSimBoard *board, enum BattlerId battler)
{
    u32 rosterIndex;

    if (board == NULL || battler >= MAX_BATTLERS_COUNT)
        return NULL;
    rosterIndex = board->active[battler].rosterIndex;
    if (rosterIndex >= AI_SIM_ROSTER_COUNT)
        return NULL;
    return &board->party[rosterIndex];
}

static const struct AiSimPartyState *GetConstPartyState(const struct AiSimBoard *board,
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

static bool32 IsSimBattlerAlive(const struct AiSimBoard *board, enum BattlerId battler)
{
    const struct AiSimPartyState *party;

    if (battler >= MAX_BATTLERS_COUNT || !(board->activeMask & (1u << battler)))
        return FALSE;
    party = GetConstPartyState(board, battler);
    return party != NULL && party->hp != 0;
}

static bool32 IsSpreadTarget(u32 target)
{
    return target == TARGET_BOTH
        || target == TARGET_FOES_AND_ALLY
        || target == TARGET_ALL_BATTLERS;
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

static bool32 IsActionSpread(const struct AiSimContext *context,
                             const struct AiSimBoard *board,
                             const struct AiSimAction *action)
{
    const struct MoveInfo *move = &gMovesInfo[action->choice];

    return IsSpreadTarget(move->target)
        || (move->argument.terrainBoost.hitsBothFoes
         && IsTerrainBoostActive(context, board, action));
}

static bool32 IsAffectedTarget(const struct AiSimContext *context,
                               const struct AiSimBoard *board,
                               const struct AiSimAction *action,
                               enum BattlerId target)
{
    u32 targetType = gMovesInfo[action->choice].target;

    if (!IsSpreadTarget(targetType) && IsActionSpread(context, board, action))
        return ((target ^ action->actor) & BIT_SIDE) != 0;
    if (target == action->actor)
        return targetType == TARGET_ALL_BATTLERS;
    if (targetType == TARGET_BOTH)
        return ((target ^ action->actor) & BIT_SIDE) != 0;
    if (targetType == TARGET_FOES_AND_ALLY || targetType == TARGET_ALL_BATTLERS)
        return TRUE;
    return target == action->target;
}

static bool32 IsSupportedSimAbility(u32 ability)
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
    case ABILITY_INTIMIDATE: // Its completed switch-in drop is in stat stages.
        return TRUE;
    default:
        return FALSE;
    }
}

static bool32 IsSupportedSimItem(u32 item)
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

static u32 GetSnapshotUnsupportedFlags(const struct AiSimContext *context,
                                       const struct AiSimBoard *board)
{
    const u32 supportedSideStatuses = SIDE_STATUS_SCREEN_ANY
                                    | SIDE_STATUS_SAFEGUARD
                                    | SIDE_STATUS_MIST
                                    | SIDE_STATUS_TAILWIND;
    u32 unsupported = 0;
    u32 battler;
    u32 side;

    if (context->flags & AI_SIM_CONTEXT_PARTIAL_KNOWLEDGE)
        unsupported |= AI_SIM_UNSUPPORTED_KNOWLEDGE;
    if (context->flags & AI_SIM_CONTEXT_REDIRECTION_STATE)
        unsupported |= AI_SIM_UNSUPPORTED_REDIRECTION;
    if (context->flags & AI_SIM_CONTEXT_SUBSTITUTE_STATE)
        unsupported |= AI_SIM_UNSUPPORTED_SUBSTITUTE;
    if (context->flags & AI_SIM_CONTEXT_RESIDUAL_STATE)
        unsupported |= AI_SIM_UNSUPPORTED_RESIDUAL;
    if (context->flags & AI_SIM_CONTEXT_FIELD_STATE)
        unsupported |= AI_SIM_UNSUPPORTED_FIELD_STATE;
    if (context->flags & AI_SIM_CONTEXT_VOLATILE_STATE)
        unsupported |= AI_SIM_UNSUPPORTED_VOLATILE_STATE;

    if (board->fieldStatuses & ~(STATUS_FIELD_TRICK_ROOM | STATUS_FIELD_PSYCHIC_TERRAIN))
        unsupported |= AI_SIM_UNSUPPORTED_FIELD_STATE;
    if (board->gravityTimer != 0 || board->magicRoomTimer != 0 || board->wonderRoomTimer != 0)
        unsupported |= AI_SIM_UNSUPPORTED_FIELD_STATE;
    if (board->weather & B_WEATHER_DAMAGING_ANY)
        unsupported |= AI_SIM_UNSUPPORTED_RESIDUAL;
    if (board->weather & ~(B_WEATHER_RAIN_NORMAL | B_WEATHER_SUN_NORMAL | B_WEATHER_DAMAGING_ANY))
        unsupported |= AI_SIM_UNSUPPORTED_FIELD_STATE;

    for (side = 0; side < NUM_BATTLE_SIDES; side++)
    {
        if (board->sides[side].followMeTimer != 0)
            unsupported |= AI_SIM_UNSUPPORTED_REDIRECTION;
        if (board->sides[side].statuses & ~supportedSideStatuses)
            unsupported |= AI_SIM_UNSUPPORTED_FIELD_STATE;
    }

    for (battler = 0; battler < MAX_BATTLERS_COUNT; battler++)
    {
        const struct AiSimCombatProfile *profile;
        const struct AiSimMonTemplate *mon;
        const struct AiSimPartyState *party;
        u32 rosterIndex;

        if (!(board->activeMask & (1u << battler)))
            continue;
        if (!IsSimBattlerAlive(board, battler)
         && (board->active[battler].flags & AI_SIM_ACTIVE_NEEDS_REPLACEMENT))
            continue;
        rosterIndex = board->active[battler].rosterIndex;
        if (rosterIndex >= AI_SIM_ROSTER_COUNT)
            continue;
        mon = &context->mons[rosterIndex];
        party = &board->party[rosterIndex];
        profile = GetProfile(context, board, battler);
        // Stellar preserves the Pokemon's defensive base types while using a
        // separate offensive boost ledger.  Neither component is encoded in
        // the compact board, so an already-active Stellar form must fail at
        // the snapshot boundary just like a prospective Stellar activation.
        if ((board->active[battler].flags & AI_SIM_ACTIVE_TERA)
         && mon->teraType == TYPE_STELLAR)
            unsupported |= AI_SIM_UNSUPPORTED_GIMMICK
                         | AI_SIM_UNSUPPORTED_DAMAGE_MODIFIER;
        if (!(mon->flags & AI_SIM_MON_MOVES_KNOWN)
         || !(mon->flags & AI_SIM_MON_PROFILE_KNOWN)
         || profile == NULL
         || !(profile->flags & AI_SIM_PROFILE_ABILITY_KNOWN)
         || !(profile->flags & AI_SIM_PROFILE_TYPES_KNOWN)
         || !(party->flags & AI_SIM_PARTY_ITEM_KNOWN)
         || !(party->flags & AI_SIM_PARTY_STATUS_KNOWN))
            unsupported |= AI_SIM_UNSUPPORTED_KNOWLEDGE;
        else if (!IsSupportedSimAbility(profile->ability)
              || !IsSupportedSimItem(party->item))
            unsupported |= AI_SIM_UNSUPPORTED_DAMAGE_MODIFIER;
        if (party->status1 & (STATUS1_DAMAGING & ~AI_SIM_MODELED_RESIDUAL_STATUSES))
            unsupported |= AI_SIM_UNSUPPORTED_RESIDUAL;
    }
    return unsupported;
}

static bool32 IsProtectionMove(u32 move)
{
    return move != MOVE_NONE && move < MOVES_COUNT_ALL
        && gMovesInfo[move].effect == EFFECT_PROTECT;
}

static bool32 IsDirectDamagingAllyTarget(const struct AiSimContext *context,
                                         const struct AiSimBoard *board,
                                         const struct AiSimAction *action)
{
    const struct MoveInfo *move;

    if (action == NULL || action->kind != AI_SIM_ACTION_MOVE
     || action->choice == MOVE_NONE || action->choice >= MOVES_COUNT_ALL
     || action->actor >= MAX_BATTLERS_COUNT || action->target >= MAX_BATTLERS_COUNT)
        return FALSE;
    move = &gMovesInfo[action->choice];
    if (move->power == 0 || IsActionSpread(context, board, action) || action->actor == action->target)
        return FALSE;
    return ((action->actor ^ action->target) & BIT_SIDE) == 0;
}

u16 AiSim_GetActionRejectionFlags(const struct AiSimContext *context,
                                  const struct AiSimBoard *board,
                                  const struct AiSimAction *action)
{
    if (context == NULL || board == NULL || action == NULL
     || context->battlersCount == 0
     || context->battlersCount > MAX_BATTLERS_COUNT)
        return AI_CANDIDATE_REJECTION_NONE;
    if (IsDirectDamagingAllyTarget(context, board, action)
     && action->allyInteractionKind == AI_ALLY_INTERACTION_NONE)
        return AI_CANDIDATE_REJECTION_UNAPPROVED_ALLY_TARGET;
    return AI_CANDIDATE_REJECTION_NONE;
}

static bool32 IsMoveTargetValid(const struct AiSimBoard *board,
                                const struct AiSimAction *action)
{
    u32 targetType = gMovesInfo[action->choice].target;

    switch (targetType)
    {
    case TARGET_USER:
    case TARGET_FIELD:
    case TARGET_OPPONENTS_FIELD:
        return action->target == action->actor || action->target == AI_SIM_ROSTER_NONE;
    case TARGET_ALLY:
        return action->target < MAX_BATTLERS_COUNT
            && IsSimBattlerAlive(board, action->target)
            && action->target != action->actor
            && ((action->actor ^ action->target) & BIT_SIDE) == 0;
    case TARGET_USER_OR_ALLY:
        return action->target < MAX_BATTLERS_COUNT
            && IsSimBattlerAlive(board, action->target)
            && ((action->actor ^ action->target) & BIT_SIDE) == 0;
    case TARGET_USER_AND_ALLY:
    case TARGET_BOTH:
    case TARGET_FOES_AND_ALLY:
    case TARGET_ALL_BATTLERS:
        return TRUE;
    default:
        return action->target < MAX_BATTLERS_COUNT && IsSimBattlerAlive(board, action->target);
    }
}

static enum Move GetActionBaseMove(const struct AiSimContext *context,
                                   const struct AiSimBoard *board,
                                   const struct AiSimAction *action)
{
    u32 rosterIndex;

    if (action->actor >= MAX_BATTLERS_COUNT || action->moveSlot >= MAX_MON_MOVES)
        return MOVE_NONE;
    rosterIndex = board->active[action->actor].rosterIndex;
    if (rosterIndex >= AI_SIM_ROSTER_COUNT)
        return MOVE_NONE;
    return context->mons[rosterIndex].moves[action->moveSlot];
}

static enum Move GetExpectedDamagingZMove(const struct AiSimContext *context,
                                          const struct AiSimBoard *board,
                                          const struct AiSimAction *action)
{
    const struct AiSimCombatProfile *profile;
    const struct AiSimPartyState *party;
    enum Move baseMove = GetActionBaseMove(context, board, action);
    enum Move zMove;
    u32 moveType;

    if (baseMove == MOVE_NONE || baseMove >= MOVES_COUNT
     || gMovesInfo[baseMove].category == DAMAGE_CATEGORY_STATUS)
        return MOVE_NONE;
    party = GetConstPartyState(board, action->actor);
    profile = GetProfile(context, board, action->actor);
    if (party == NULL || profile == NULL || party->item == ITEM_NONE
     || party->item >= ITEMS_COUNT
     || gItemsInfo[party->item].holdEffect != HOLD_EFFECT_Z_CRYSTAL)
        return MOVE_NONE;

    zMove = GetSignatureZMove(baseMove, profile->species, party->item);
    if (zMove != MOVE_NONE)
        return zMove;
    moveType = gMovesInfo[baseMove].type;
    if (moveType >= NUMBER_OF_MON_TYPES || gItemsInfo[party->item].secondaryId != moveType)
        return MOVE_NONE;
    return gTypesInfo[moveType].zMove;
}

static bool32 IsMoveInTemplate(const struct AiSimContext *context,
                               const struct AiSimBoard *board,
                               const struct AiSimAction *action)
{
    u32 rosterIndex = board->active[action->actor].rosterIndex;

    if (rosterIndex >= AI_SIM_ROSTER_COUNT || action->moveSlot >= MAX_MON_MOVES)
        return FALSE;
    if (action->choice >= FIRST_Z_MOVE && action->choice <= LAST_Z_MOVE)
        return action->gimmick == GIMMICK_Z_MOVE
            && GetExpectedDamagingZMove(context, board, action) == action->choice;
    if (action->choice >= FIRST_MAX_MOVE && action->choice <= LAST_MAX_MOVE)
        return context->mons[rosterIndex].moves[action->moveSlot] != MOVE_NONE;
    return context->mons[rosterIndex].moves[action->moveSlot] == action->choice;
}

static bool32 IsContinuingGeomancy(const struct AiSimBoard *board,
                                   const struct AiSimAction *action)
{
    const struct AiSimActiveState *active = &board->active[action->actor];

    return action->choice == MOVE_GEOMANCY
        && (active->chargingMove == MOVE_GEOMANCY
         || (active->volatileFlags & AI_SIM_VOLATILE_GEOMANCY_CHARGING));
}

static bool32 IsSupportedProspectiveFormAbility(u32 ability)
{
    // These abilities are either passive or are already consumed directly by
    // the immutable simulator. Entry-style form-change effects such as
    // Intimidate and weather setters must fail closed until their activation
    // events are represented on the board.
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
        return TRUE;
    default:
        return FALSE;
    }
}

static enum AiSimApplyStatus CheckGimmick(const struct AiSimContext *context,
                                          const struct AiSimBoard *board,
                                          const struct AiSimAction *action,
                                          u32 *unsupportedFlags)
{
    u32 rosterIndex;
    const struct AiSimMonTemplate *mon;
    const struct AiSimPartyState *party;
    u32 trainer;

    if (action->gimmick == GIMMICK_NONE)
    {
        if (action->choice >= FIRST_Z_MOVE && action->choice <= LAST_Z_MOVE)
            return AI_SIM_APPLY_INVALID;
        if (board->active[action->actor].flags & AI_SIM_ACTIVE_DYNAMAX)
        {
            // A Max move's type/effect alone is insufficient: its power is
            // derived from the base move.  The compact action does not carry
            // that power, so never reinterpret a base move (or power-1 Max
            // move table entry) as an exact Max action.
            *unsupportedFlags |= AI_SIM_UNSUPPORTED_GIMMICK
                               | AI_SIM_UNSUPPORTED_DYNAMIC_POWER;
            return AI_SIM_APPLY_UNSUPPORTED;
        }
        if (action->choice >= FIRST_MAX_MOVE && action->choice <= LAST_MAX_MOVE)
            return AI_SIM_APPLY_INVALID;
        return AI_SIM_APPLY_OK;
    }
    if (action->gimmick >= GIMMICKS_COUNT)
    {
        *unsupportedFlags |= AI_SIM_UNSUPPORTED_GIMMICK;
        return AI_SIM_APPLY_UNSUPPORTED;
    }

    rosterIndex = board->active[action->actor].rosterIndex;
    mon = &context->mons[rosterIndex];
    party = &board->party[rosterIndex];
    trainer = mon->trainer;
    if (!(mon->eligibleGimmicks & (1u << action->gimmick))
     || trainer >= MAX_BATTLE_TRAINERS
     || (board->trainerGimmickUsed[trainer] & (1u << action->gimmick)))
        return AI_SIM_APPLY_INVALID;
    if (party->activeGimmick != GIMMICK_NONE
     && !(party->activeGimmick == GIMMICK_ULTRA_BURST
       && action->gimmick == GIMMICK_Z_MOVE))
        return AI_SIM_APPLY_INVALID;

    if ((action->gimmick == GIMMICK_MEGA || action->gimmick == GIMMICK_ULTRA_BURST)
     && (!(mon->transformed.flags & AI_SIM_PROFILE_VALID)
      || !(mon->transformed.flags & AI_SIM_PROFILE_ABILITY_KNOWN)
      || !(mon->transformed.flags & AI_SIM_PROFILE_TYPES_KNOWN)
      || mon->transformationGimmick != action->gimmick
      || !IsSupportedProspectiveFormAbility(mon->transformed.ability)))
    {
        *unsupportedFlags |= AI_SIM_UNSUPPORTED_GIMMICK;
        return AI_SIM_APPLY_UNSUPPORTED;
    }
    if (action->gimmick == GIMMICK_Z_MOVE)
    {
        if (action->choice < FIRST_Z_MOVE || action->choice > LAST_Z_MOVE)
            return AI_SIM_APPLY_INVALID;
        return AI_SIM_APPLY_OK;
    }
    if (action->gimmick == GIMMICK_DYNAMAX)
    {
        // The compact action does not yet carry the effective Max move and
        // its derived power. Never consume the resource and apply a base move.
        *unsupportedFlags |= AI_SIM_UNSUPPORTED_GIMMICK | AI_SIM_UNSUPPORTED_DYNAMIC_POWER;
        return AI_SIM_APPLY_UNSUPPORTED;
    }
    if (action->gimmick == GIMMICK_TERA
     && (mon->teraType == TYPE_NONE
      || mon->teraType == TYPE_STELLAR
      || mon->teraType >= NUMBER_OF_MON_TYPES))
    {
        *unsupportedFlags |= AI_SIM_UNSUPPORTED_GIMMICK;
        return AI_SIM_APPLY_UNSUPPORTED;
    }
    return AI_SIM_APPLY_OK;
}

static bool32 HasSupportedMoonblastSecondary(u32 moveId)
{
    const struct MoveInfo *move;
    const struct AdditionalEffect *effect;

    if (moveId != MOVE_MOONBLAST)
        return FALSE;
    move = &gMovesInfo[moveId];
    if (move->numAdditionalEffects != 1 || move->additionalEffects == NULL)
        return FALSE;
    effect = &move->additionalEffects[0];
    return effect->moveEffect == MOVE_EFFECT_STAT_MINUS
        && effect->chance == 30
        && effect->spAtk == 1
        && effect->attack == 0
        && effect->defense == 0
        && effect->spDef == 0
        && effect->speed == 0
        && effect->accuracy == 0
        && effect->evasion == 0
        && !effect->self
        && !effect->onSide;
}

static bool32 HasSupportedIceBeamSecondary(u32 moveId)
{
    const struct MoveInfo *move;
    const struct AdditionalEffect *effect;

    if (moveId != MOVE_ICE_BEAM)
        return FALSE;
    move = &gMovesInfo[moveId];
    if (move->numAdditionalEffects != 1 || move->additionalEffects == NULL)
        return FALSE;
    effect = &move->additionalEffects[0];
    return effect->moveEffect == MOVE_EFFECT_FREEZE_OR_FROSTBITE
        && effect->chance == 10
        && effect->attack == 0
        && effect->defense == 0
        && effect->spAtk == 0
        && effect->spDef == 0
        && effect->speed == 0
        && effect->accuracy == 0
        && effect->evasion == 0
        && !effect->self
        && !effect->preAttackEffect
        && !effect->onSide;
}

static bool32 HasSupportedFlareBlitzSecondary(u32 moveId)
{
    const struct MoveInfo *move;
    const struct AdditionalEffect *effect;

    if (moveId != MOVE_FLARE_BLITZ)
        return FALSE;
    move = &gMovesInfo[moveId];
    if (move->effect != EFFECT_RECOIL
     || move->argument.recoilPercentage != 33
     || !move->thawsUser
     || move->numAdditionalEffects != 1
     || move->additionalEffects == NULL)
        return FALSE;
    effect = &move->additionalEffects[0];
    return effect->moveEffect == MOVE_EFFECT_BURN
        && effect->chance == 10
        && effect->attack == 0
        && effect->defense == 0
        && effect->spAtk == 0
        && effect->spDef == 0
        && effect->speed == 0
        && effect->accuracy == 0
        && effect->evasion == 0
        && !effect->self
        && !effect->preAttackEffect
        && !effect->onSide;
}

static bool32 HasSupportedPartingShotEffect(u32 moveId)
{
    const struct MoveInfo *move;
    const struct AdditionalEffect *effect;

    if (moveId != MOVE_PARTING_SHOT)
        return FALSE;
    move = &gMovesInfo[moveId];
    if (move->effect != EFFECT_PARTING_SHOT
     || move->numAdditionalEffects != 1
     || move->additionalEffects == NULL)
        return FALSE;
    effect = &move->additionalEffects[0];
    return effect->moveEffect == STAT_CHANGE_EFFECT_MINUS
        && effect->chance == 0
        && effect->attack == 1
        && effect->spAtk == 1
        && effect->defense == 0
        && effect->spDef == 0
        && effect->speed == 0
        && effect->accuracy == 0
        && effect->evasion == 0
        && !effect->self
        && !effect->preAttackEffect
        && !effect->onSide;
}

static u32 GetSupportedSecondaryChance(u32 moveId)
{
    if (HasSupportedMoonblastSecondary(moveId))
        return 30;
    if (HasSupportedIceBeamSecondary(moveId))
        return 10;
    if (HasSupportedFlareBlitzSecondary(moveId))
        return 10;
    return 0;
}

static bool32 HasSupportedCloseCombatSecondary(u32 moveId)
{
    const struct MoveInfo *move;
    const struct AdditionalEffect *effect;

    if (moveId != MOVE_CLOSE_COMBAT)
        return FALSE;
    move = &gMovesInfo[moveId];
    if (move->numAdditionalEffects != 1 || move->additionalEffects == NULL)
        return FALSE;
    effect = &move->additionalEffects[0];
    return effect->moveEffect == MOVE_EFFECT_STAT_MINUS
        && effect->chance == 0
        && effect->defense == 1
        && effect->spDef == 1
        && effect->attack == 0
        && effect->spAtk == 0
        && effect->speed == 0
        && effect->accuracy == 0
        && effect->evasion == 0
        && effect->self
        && !effect->preAttackEffect
        && !effect->onSide;
}

static bool32 HasSupportedSpectralThiefEffect(u32 moveId)
{
    const struct MoveInfo *move;
    const struct AdditionalEffect *effect;

    if (moveId != MOVE_SPECTRAL_THIEF)
        return FALSE;
    move = &gMovesInfo[moveId];
    if (move->numAdditionalEffects != 1 || move->additionalEffects == NULL)
        return FALSE;
    effect = &move->additionalEffects[0];
    return effect->moveEffect == MOVE_EFFECT_STEAL_STATS
        && effect->chance == 0
        && effect->attack == 0
        && effect->defense == 0
        && effect->spAtk == 0
        && effect->spDef == 0
        && effect->speed == 0
        && effect->accuracy == 0
        && effect->evasion == 0
        && !effect->self
        && effect->preAttackEffect
        && !effect->onSide;
}

static bool32 IsSupportedExpandingForce(u32 moveId)
{
    const struct MoveInfo *move;

    if (moveId != MOVE_EXPANDING_FORCE)
        return FALSE;
    move = &gMovesInfo[moveId];
    return move->effect == EFFECT_TERRAIN_BOOST
        && move->argument.terrainBoost.terrain == STATUS_FIELD_PSYCHIC_TERRAIN
        && move->argument.terrainBoost.percent == 50
        && move->argument.terrainBoost.groundCheck == GROUND_CHECK_USER
        && move->argument.terrainBoost.hitsBothFoes;
}

static u32 CountLegalReserves(const struct AiSimContext *context,
                              const struct AiSimBoard *board,
                              enum BattlerId actor)
{
    u32 count = 0;
    u32 rosterIndex;
    u32 active;
    u32 trainer = context->battlerTrainer[actor];

    for (rosterIndex = 0; rosterIndex < AI_SIM_ROSTER_COUNT; rosterIndex++)
    {
        const struct AiSimMonTemplate *mon = &context->mons[rosterIndex];
        bool32 isActive = FALSE;

        if (!(mon->flags & AI_SIM_MON_PRESENT)
         || mon->trainer != trainer
         || board->party[rosterIndex].hp == 0)
            continue;
        for (active = 0; active < MAX_BATTLERS_COUNT; active++)
        {
            if ((board->activeMask & (1u << active))
             && board->active[active].rosterIndex == rosterIndex)
            {
                isActive = TRUE;
                break;
            }
        }
        if (!isActive)
            count++;
    }
    return count;
}

static bool32 HasLegalReserve(const struct AiSimContext *context,
                              const struct AiSimBoard *board,
                              enum BattlerId actor)
{
    return CountLegalReserves(context, board, actor) != 0;
}

bool32 AiSim_ReplacementSlotWillBeFilled(const struct AiSimContext *context,
                                         const struct AiSimBoard *board,
                                         enum BattlerId battler)
{
    u32 earlierNeeds = 0;
    u32 trainer;

    if (context == NULL || board == NULL
     || context->battlersCount == 0
     || context->battlersCount > MAX_BATTLERS_COUNT
     || battler >= MAX_BATTLERS_COUNT
     || battler >= context->battlersCount
     || !(board->activeMask & (1u << battler))
     || !(board->active[battler].flags & AI_SIM_ACTIVE_NEEDS_REPLACEMENT))
        return FALSE;
    trainer = context->battlerTrainer[battler];
    for (enum BattlerId earlier = 0; earlier < battler; earlier++)
    {
        if ((board->activeMask & (1u << earlier))
         && context->battlerTrainer[earlier] == trainer
         && (board->active[earlier].flags & AI_SIM_ACTIVE_NEEDS_REPLACEMENT))
            earlierNeeds++;
    }
    return earlierNeeds < CountLegalReserves(context, board, battler);
}

static enum AiSimApplyStatus CheckMoveSupport(const struct AiSimContext *context,
                                              const struct AiSimBoard *board,
                                              const struct AiSimAction *action,
                                              u32 *unsupportedFlags,
                                              enum BattlerId projectedTarget,
                                              struct SimulatedDamage *projectedDamage)
{
    const struct MoveInfo *move = &gMovesInfo[action->choice];
    u32 target;
    bool32 foundTarget = FALSE;

    // These move flags alter legality or damage rules that are not represented
    // by the compact board/damage model.  Reject them categorically instead of
    // treating the flagged move (including an effective signature Z move) as
    // an ordinary EFFECT_HIT.
    if (move->cantUseTwice
     || move->ignoresTargetAbility
     || move->ignoresTargetDefenseEvasionStages)
    {
        *unsupportedFlags |= AI_SIM_UNSUPPORTED_MOVE_EFFECT;
        return AI_SIM_APPLY_UNSUPPORTED;
    }

    if (move->target == TARGET_SMART || move->target == TARGET_DEPENDS
     || move->target == TARGET_RANDOM || move->target == TARGET_NONE)
    {
        *unsupportedFlags |= AI_SIM_UNSUPPORTED_OUTCOME;
        return AI_SIM_APPLY_UNSUPPORTED;
    }

    if (move->power != 0 && move->accuracy != 0 && move->accuracy != 100)
    {
        *unsupportedFlags |= AI_SIM_UNSUPPORTED_OUTCOME;
        return AI_SIM_APPLY_UNSUPPORTED;
    }
    if (move->power != 0 && move->accuracy != 0
     && board->active[action->actor].statStages[STAT_ACC] != DEFAULT_STAT_STAGE)
    {
        *unsupportedFlags |= AI_SIM_UNSUPPORTED_OUTCOME;
        return AI_SIM_APPLY_UNSUPPORTED;
    }
    // Generation 1 derives critical odds from base Speed, and Generation 2
    // uses X/256 odds.  The exact outcome key models modern 1/N critical
    // stages, so a pre-Generation-3 damage root must fail closed instead of
    // silently treating every hit as non-critical.
    if (move->power != 0 && GetConfig(B_CRIT_CHANCE) < GEN_3)
    {
        *unsupportedFlags |= AI_SIM_UNSUPPORTED_OUTCOME;
        return AI_SIM_APPLY_UNSUPPORTED;
    }
    // Live damage and critical RNG are drawn independently for every affected
    // defender.  The compact exact key has one roll/critical slot per actor,
    // so spread damage is a categorical root-generation boundary rather than
    // a shared-roll approximation.
    if (move->power != 0
     && (IsActionSpread(context, board, action)
      || max(1, move->strikeCount) != 1))
    {
        *unsupportedFlags |= AI_SIM_UNSUPPORTED_OUTCOME;
        return AI_SIM_APPLY_UNSUPPORTED;
    }

    switch (move->effect)
    {
    case EFFECT_PROTECT:
        switch (move->argument.protectMethod)
        {
        case PROTECT_NORMAL:
        case PROTECT_MAX_GUARD:
        case PROTECT_WIDE_GUARD:
        case PROTECT_QUICK_GUARD:
            return AI_SIM_APPLY_OK;
        default:
            *unsupportedFlags |= AI_SIM_UNSUPPORTED_MOVE_EFFECT;
            return AI_SIM_APPLY_UNSUPPORTED;
        }
    case EFFECT_TAILWIND:
        return AI_SIM_APPLY_OK;
    case EFFECT_TRICK_ROOM:
    case EFFECT_GEOMANCY:
        return AI_SIM_APPLY_OK;
    case EFFECT_REFLECT:
        return (board->sides[action->actor & BIT_SIDE].statuses & SIDE_STATUS_REFLECT)
             ? AI_SIM_APPLY_INVALID : AI_SIM_APPLY_OK;
    case EFFECT_LIGHT_SCREEN:
        return (board->sides[action->actor & BIT_SIDE].statuses & SIDE_STATUS_LIGHTSCREEN)
             ? AI_SIM_APPLY_INVALID : AI_SIM_APPLY_OK;
    case EFFECT_AURORA_VEIL:
        if (board->sides[action->actor & BIT_SIDE].statuses & SIDE_STATUS_AURORA_VEIL)
            return AI_SIM_APPLY_INVALID;
        if (!(board->weather & B_WEATHER_ICY_ANY))
            return AI_SIM_APPLY_INVALID;
        return AI_SIM_APPLY_OK;
    case EFFECT_PARTING_SHOT:
        if (!HasSupportedPartingShotEffect(action->choice))
        {
            *unsupportedFlags |= AI_SIM_UNSUPPORTED_MOVE_EFFECT;
            return AI_SIM_APPLY_UNSUPPORTED;
        }
        if (move->accuracy != 0
         && (move->accuracy != 100
          || board->active[action->actor].statStages[STAT_ACC] != DEFAULT_STAT_STAGE
          || board->active[action->target].statStages[STAT_EVASION] != DEFAULT_STAT_STAGE))
        {
            *unsupportedFlags |= AI_SIM_UNSUPPORTED_OUTCOME;
            return AI_SIM_APPLY_UNSUPPORTED;
        }
        // The compact continuation does not carry a voluntary pivot reserve.
        // A no-reserve Parting Shot can still resolve its stat drops exactly;
        // otherwise never guess which bench state should become active.
        if (HasLegalReserve(context, board, action->actor))
        {
            *unsupportedFlags |= AI_SIM_UNSUPPORTED_SWITCH_IN_EFFECT;
            return AI_SIM_APPLY_UNSUPPORTED;
        }
        return AI_SIM_APPLY_OK;
    case EFFECT_FIRST_TURN_ONLY:
        if (action->choice == MOVE_FAKE_OUT)
            break;
        *unsupportedFlags |= AI_SIM_UNSUPPORTED_MOVE_EFFECT;
        return AI_SIM_APPLY_UNSUPPORTED;
    case EFFECT_HIT:
        if (move->numAdditionalEffects != 0
         && !HasSupportedMoonblastSecondary(action->choice)
         && !HasSupportedIceBeamSecondary(action->choice)
         && !HasSupportedCloseCombatSecondary(action->choice)
         && !HasSupportedSpectralThiefEffect(action->choice))
        {
            *unsupportedFlags |= AI_SIM_UNSUPPORTED_SECONDARY_EFFECT;
            return AI_SIM_APPLY_UNSUPPORTED;
        }
        break;
    case EFFECT_TERRAIN_BOOST:
        if (!IsSupportedExpandingForce(action->choice))
        {
            *unsupportedFlags |= AI_SIM_UNSUPPORTED_DYNAMIC_POWER;
            return AI_SIM_APPLY_UNSUPPORTED;
        }
        break;
    case EFFECT_RECOIL:
        if (!HasSupportedFlareBlitzSecondary(action->choice))
        {
            *unsupportedFlags |= AI_SIM_UNSUPPORTED_MOVE_EFFECT
                               | AI_SIM_UNSUPPORTED_SECONDARY_EFFECT;
            return AI_SIM_APPLY_UNSUPPORTED;
        }
        break;
    case EFFECT_EARTHQUAKE:
    case EFFECT_KNOCK_OFF:
    case EFFECT_PSYSHOCK:
        break;
    default:
        *unsupportedFlags |= AI_SIM_UNSUPPORTED_MOVE_EFFECT;
        return AI_SIM_APPLY_UNSUPPORTED;
    }

    for (target = 0; target < MAX_BATTLERS_COUNT; target++)
    {
        struct SimulatedDamage damage;

        if (!IsSimBattlerAlive(board, target) || !IsAffectedTarget(context, board, action, target))
            continue;
        foundTarget = TRUE;
        if (move->accuracy != 0
         && board->active[target].statStages[STAT_EVASION] != DEFAULT_STAT_STAGE)
        {
            *unsupportedFlags |= AI_SIM_UNSUPPORTED_OUTCOME;
            return AI_SIM_APPLY_UNSUPPORTED;
        }
        if (!AiSim_CalcDamage(context, board, action, target, &damage))
        {
            *unsupportedFlags |= AI_SIM_UNSUPPORTED_DAMAGE_MODIFIER;
            return AI_SIM_APPLY_UNSUPPORTED;
        }
        if (projectedDamage != NULL && target == projectedTarget)
            *projectedDamage = damage;
    }
    if (!foundTarget)
        return AI_SIM_APPLY_INVALID;
    return AI_SIM_APPLY_OK;
}

static void ApplyReplacement(const struct AiSimContext *context,
                             struct AiSimBoard *board,
                             enum BattlerId actor,
                             u32 replacement,
                             struct AiSimTurnResult *result,
                             bool32 standaloneSwitch);
static void ActivateGimmick(const struct AiSimContext *context,
                            struct AiSimBoard *board,
                            const struct AiSimAction *action);

static bool32 HasForcedReplacement(const struct AiSimAction *action)
{
    return action->kind == AI_SIM_ACTION_MOVE
        && (action->flags & AI_SIM_ACTION_FORCED_REPLACEMENT);
}

static u32 GetActionReplacement(const struct AiSimAction *action)
{
    if (action->kind == AI_SIM_ACTION_SWITCH)
        return action->choice;
    if (HasForcedReplacement(action))
        return action->replacementRosterIndex;
    return AI_SIM_ROSTER_NONE;
}

static bool32 IsSupportedSwitchInAbility(u32 ability)
{
    return ability == ABILITY_NONE
        || ability == ABILITY_PRESSURE
        || ability == ABILITY_PRANKSTER
        || ability == ABILITY_PRISM_ARMOR
        || ability == ABILITY_NEUROFORCE
        || ability == ABILITY_STALL
        || ability == ABILITY_STURDY
        || ability == ABILITY_GUTS
        || ability == ABILITY_TECHNICIAN
        || ability == ABILITY_FAIRY_AURA;
}

static enum AiSimApplyStatus CheckReplacement(const struct AiSimContext *context,
                                              const struct AiSimBoard *board,
                                              const struct AiSimJointTurn *turn,
                                              enum BattlerId actor,
                                              u32 replacement,
                                              u32 *unsupportedFlags)
{
    const struct AiSimCombatProfile *replacementProfile;
    const struct AiSimMonTemplate *replacementMon;
    const struct AiSimPartyState *replacementParty;
    u32 other;

    if (replacement >= AI_SIM_ROSTER_COUNT
     || !(context->mons[replacement].flags & AI_SIM_MON_PRESENT)
     || context->mons[replacement].trainer != context->battlerTrainer[actor]
     || board->party[replacement].hp == 0)
        return AI_SIM_APPLY_INVALID;
    replacementMon = &context->mons[replacement];
    replacementParty = &board->party[replacement];
    if (replacementParty->activeGimmick == GIMMICK_TERA
     && replacementMon->teraType == TYPE_STELLAR)
    {
        *unsupportedFlags |= AI_SIM_UNSUPPORTED_GIMMICK
                           | AI_SIM_UNSUPPORTED_DAMAGE_MODIFIER;
        return AI_SIM_APPLY_UNSUPPORTED;
    }
    replacementProfile = &replacementMon->normal;
    if (replacementParty->activeGimmick == GIMMICK_MEGA
     || replacementParty->activeGimmick == GIMMICK_ULTRA_BURST)
        replacementProfile = &replacementMon->transformed;
    if (!(replacementMon->flags & AI_SIM_MON_PROFILE_KNOWN)
     || !(replacementMon->flags & AI_SIM_MON_MOVES_KNOWN)
     || !(replacementProfile->flags & AI_SIM_PROFILE_VALID)
     || !(replacementProfile->flags & AI_SIM_PROFILE_ABILITY_KNOWN)
     || !(replacementProfile->flags & AI_SIM_PROFILE_TYPES_KNOWN)
     || !(replacementParty->flags & AI_SIM_PARTY_ITEM_KNOWN)
     || !(replacementParty->flags & AI_SIM_PARTY_STATUS_KNOWN))
    {
        *unsupportedFlags |= AI_SIM_UNSUPPORTED_KNOWLEDGE;
        return AI_SIM_APPLY_UNSUPPORTED;
    }
    if (!IsSupportedSimItem(replacementParty->item)
     || (replacementParty->status1 & (STATUS1_DAMAGING & ~AI_SIM_MODELED_RESIDUAL_STATUSES)))
    {
        *unsupportedFlags |= (replacementParty->status1
                            & (STATUS1_DAMAGING & ~AI_SIM_MODELED_RESIDUAL_STATUSES))
                           ? AI_SIM_UNSUPPORTED_RESIDUAL
                           : AI_SIM_UNSUPPORTED_DAMAGE_MODIFIER;
        return AI_SIM_APPLY_UNSUPPORTED;
    }
    for (other = 0; other < MAX_BATTLERS_COUNT; other++)
    {
        if ((board->activeMask & (1u << other))
         && board->active[other].rosterIndex == replacement)
            return AI_SIM_APPLY_INVALID;
    }
    for (other = 0; other < actor; other++)
    {
        if ((turn->actionMask & (1u << other))
         && context->battlerTrainer[other] == context->battlerTrainer[actor]
         && GetActionReplacement(&turn->actions[other]) == replacement)
            return AI_SIM_APPLY_INVALID;
    }
    if (board->sides[actor & BIT_SIDE].hazardsMask != 0
     || board->sides[actor & BIT_SIDE].spikes != 0
     || board->sides[actor & BIT_SIDE].toxicSpikes != 0)
    {
        *unsupportedFlags |= AI_SIM_UNSUPPORTED_SWITCH_IN_EFFECT;
        return AI_SIM_APPLY_UNSUPPORTED;
    }
    if (!IsSupportedSwitchInAbility(replacementProfile->ability))
    {
        *unsupportedFlags |= AI_SIM_UNSUPPORTED_SWITCH_IN_EFFECT;
        return AI_SIM_APPLY_UNSUPPORTED;
    }
    return AI_SIM_APPLY_OK;
}

// Board validation is part of the same single-job/non-reentrant simulator as
// exact outcome enumeration.  A complete board here would otherwise consume
// over half of the battle-controller task's fixed 1 KiB stack before the
// enumerator or apply path calls into it.
static EWRAM_DATA struct AiSimBoard sAiSimValidationBoard;

enum AiSimApplyStatus AiSim_CheckJointTurn(const struct AiSimContext *context,
                                           const struct AiSimBoard *board,
                                           const struct AiSimJointTurn *turn,
                                           u32 *unsupportedFlags)
{
    struct AiSimBoard *validationBoard = &sAiSimValidationBoard;
    u32 actor;
    u32 unsupported = 0;
    u32 unsupportedActionMask = 0;
    bool32 replacementUnsupported = FALSE;

    if (unsupportedFlags != NULL)
        *unsupportedFlags = 0;
    if (context == NULL || board == NULL || turn == NULL)
        return AI_SIM_APPLY_INVALID;
    if (context->battlersCount == 0 || context->battlersCount > MAX_BATTLERS_COUNT
     || context->reserved != 0 || turn->reserved != 0)
        return AI_SIM_APPLY_INVALID;
    unsupported |= GetSnapshotUnsupportedFlags(context, board);
    if (context->battleTypeFlags & BATTLE_TYPE_RAID)
        unsupported |= AI_SIM_UNSUPPORTED_BATTLE_TYPE;

    *validationBoard = *board;

    for (actor = 0; actor < MAX_BATTLERS_COUNT; actor++)
    {
        u32 rosterIndex;
        u32 percent;
        if (!(board->activeMask & (1u << actor))
         || !(board->active[actor].flags & AI_SIM_ACTIVE_DYNAMAX))
            continue;
        rosterIndex = board->active[actor].rosterIndex;
        if (rosterIndex >= AI_SIM_ROSTER_COUNT)
            return AI_SIM_APPLY_INVALID;
        percent = context->mons[rosterIndex].dynamaxHpPercent;
        if ((percent != 100 && (percent < 150 || percent > 200))
         || board->active[actor].dynamaxTurns == 0)
            return AI_SIM_APPLY_INVALID;
    }

    // Switching happens before moves. Validate every chosen reserve against
    // the incoming board, then install it in a local copy so move legality,
    // speed, targeting, PP, and gimmicks use the Pokemon that will act.
    for (actor = 0; actor < MAX_BATTLERS_COUNT; actor++)
    {
        const struct AiSimAction *action;
        enum AiSimApplyStatus status;

        if (!(turn->actionMask & (1u << actor)))
            continue;
        action = &turn->actions[actor];
        if (action->actor != actor || !(board->activeMask & (1u << actor)))
            return AI_SIM_APPLY_INVALID;
        if (board->active[actor].rosterIndex >= AI_SIM_ROSTER_COUNT)
            return AI_SIM_APPLY_INVALID;
        if (IsSimBattlerAlive(board, actor)
         && (board->active[actor].chargingMove != MOVE_NONE
          || (board->active[actor].volatileFlags & AI_SIM_VOLATILE_GEOMANCY_CHARGING))
         && (action->kind != AI_SIM_ACTION_MOVE
          || HasForcedReplacement(action)
          || action->choice != board->active[actor].chargingMove))
            return AI_SIM_APPLY_INVALID;
        if (action->flags & ~AI_SIM_ACTION_FORCED_REPLACEMENT)
            return AI_SIM_APPLY_INVALID;
        if (action->kind != AI_SIM_ACTION_MOVE && action->kind != AI_SIM_ACTION_SWITCH)
        {
            unsupported |= AI_SIM_UNSUPPORTED_ACTION_KIND;
            continue;
        }
        if (action->kind == AI_SIM_ACTION_SWITCH && action->flags != 0)
            return AI_SIM_APPLY_INVALID;
        if (action->kind == AI_SIM_ACTION_SWITCH || HasForcedReplacement(action))
        {
            u32 replacement = GetActionReplacement(action);

            if (HasForcedReplacement(action)
             && (IsSimBattlerAlive(board, actor)
              || !(board->active[actor].flags & AI_SIM_ACTIVE_NEEDS_REPLACEMENT)))
                return AI_SIM_APPLY_INVALID;
            if (action->kind == AI_SIM_ACTION_SWITCH
             && !IsSimBattlerAlive(board, actor)
             && !(board->active[actor].flags & AI_SIM_ACTIVE_NEEDS_REPLACEMENT))
                return AI_SIM_APPLY_INVALID;
            status = CheckReplacement(context, board, turn, actor, replacement, &unsupported);
            if (status == AI_SIM_APPLY_INVALID)
                return status;
            if (status == AI_SIM_APPLY_UNSUPPORTED)
            {
                replacementUnsupported = TRUE;
                continue;
            }
            ApplyReplacement(context, validationBoard, actor, replacement, NULL, FALSE);
        }
        else if (!IsSimBattlerAlive(board, actor))
            return AI_SIM_APPLY_INVALID;
    }

    if (replacementUnsupported)
    {
        if (unsupportedFlags != NULL)
            *unsupportedFlags = unsupported;
        return AI_SIM_APPLY_UNSUPPORTED;
    }

    for (actor = 0; actor < MAX_BATTLERS_COUNT; actor++)
    {
        const struct AiSimAction *action;
        enum AiSimApplyStatus status;

        if (!(turn->actionMask & (1u << actor)))
            continue;
        action = &turn->actions[actor];
        if (action->kind == AI_SIM_ACTION_SWITCH)
            continue;
        if (action->kind != AI_SIM_ACTION_MOVE)
            continue;
        if (action->choice == MOVE_NONE || action->choice >= MOVES_COUNT_ALL)
            return AI_SIM_APPLY_INVALID;
        if (action->gimmick == GIMMICK_Z_MOVE)
        {
            enum Move baseMove = GetActionBaseMove(context, validationBoard, action);

            if (baseMove != MOVE_NONE && baseMove < MOVES_COUNT
             && gMovesInfo[baseMove].category == DAMAGE_CATEGORY_STATUS)
            {
                unsupported |= AI_SIM_UNSUPPORTED_MOVE_EFFECT;
                unsupportedActionMask |= 1u << actor;
                continue;
            }
        }
        if (!IsMoveInTemplate(context, validationBoard, action)
         || !IsMoveTargetValid(validationBoard, action))
            return AI_SIM_APPLY_INVALID;
        {
            const struct AiSimPartyState *party = GetConstPartyState(validationBoard, actor);
            const struct AiSimActiveState *active = &validationBoard->active[actor];
            if (party == NULL
             || (party->pp[action->moveSlot] == 0 && !IsContinuingGeomancy(validationBoard, action))
             || (active->disabledMoveMask & (1u << action->moveSlot))
             || (active->choiceMoveSlot != AI_SIM_MOVE_SLOT_NONE
              && active->choiceMoveSlot != action->moveSlot)
             || (action->choice == MOVE_FAKE_OUT && !active->firstTurn))
                return AI_SIM_APPLY_INVALID;
            if (party->status1 & (STATUS1_SLEEP | STATUS1_PARALYSIS))
            {
                unsupported |= AI_SIM_UNSUPPORTED_OUTCOME;
                unsupportedActionMask |= 1u << actor;
                continue;
            }
        }
        if (AiSim_GetActionRejectionFlags(context, validationBoard, action) != 0)
            return AI_SIM_APPLY_INVALID;

        status = CheckGimmick(context, validationBoard, action, &unsupported);
        if (status == AI_SIM_APPLY_INVALID)
            return status;
        if (status == AI_SIM_APPLY_UNSUPPORTED)
        {
            unsupportedActionMask |= 1u << actor;
            continue;
        }
    }

    // Shared per-trainer gimmick resources cannot be consumed by both members
    // of a joint candidate in the same turn.
    for (actor = 0; actor < MAX_BATTLERS_COUNT; actor++)
    {
        u32 other;
        const struct AiSimAction *action = &turn->actions[actor];

        if (!(turn->actionMask & (1u << actor)) || action->kind != AI_SIM_ACTION_MOVE
         || action->gimmick == GIMMICK_NONE)
            continue;
        for (other = actor + 1; other < MAX_BATTLERS_COUNT; other++)
        {
            const struct AiSimAction *otherAction = &turn->actions[other];

            if (!(turn->actionMask & (1u << other)) || otherAction->kind != AI_SIM_ACTION_MOVE)
                continue;
            if (action->gimmick == otherAction->gimmick
             && (context->gimmickShareMask[actor] & (1u << other)))
                return AI_SIM_APPLY_INVALID;
        }
    }

    // All gimmicks activate before move order is resolved. Install every
    // supported prospective form first so validation and damage use the same
    // transformed stats, abilities, and types that execution will use.
    for (actor = 0; actor < MAX_BATTLERS_COUNT; actor++)
    {
        const struct AiSimAction *action = &turn->actions[actor];

        if (!(turn->actionMask & (1u << actor))
         || (unsupportedActionMask & (1u << actor))
         || action->kind != AI_SIM_ACTION_MOVE)
            continue;
        ActivateGimmick(context, validationBoard, action);
    }

    for (actor = 0; actor < MAX_BATTLERS_COUNT; actor++)
    {
        const struct AiSimAction *action = &turn->actions[actor];
        enum AiSimApplyStatus status;

        if (!(turn->actionMask & (1u << actor))
         || (unsupportedActionMask & (1u << actor))
         || action->kind != AI_SIM_ACTION_MOVE)
            continue;
        status = CheckMoveSupport(context, validationBoard, action, &unsupported,
                                  MAX_BATTLERS_COUNT, NULL);
        if (status == AI_SIM_APPLY_INVALID)
            return status;
    }

    if (unsupportedFlags != NULL)
        *unsupportedFlags = unsupported;
    return unsupported == 0 ? AI_SIM_APPLY_OK : AI_SIM_APPLY_UNSUPPORTED;
}

u32 AiSim_GetProtectSuccessDenominator(const struct AiSimBoard *board,
                                       enum BattlerId battler)
{
    u32 uses;

    if (board == NULL || battler >= MAX_BATTLERS_COUNT)
        return 1;
    uses = min(3, board->active[battler].consecutiveMoveUses);
    if (uses == 0)
        return 1;
#if B_PROTECT_FAILURE_RATE < GEN_5
    return 1u << uses;
#else
    if (uses == 1)
        return 3;
    if (uses == 2)
        return 9;
    return 27;
#endif
}

static u32 GetProtectActionSuccessDenominator(const struct AiSimBoard *board,
                                              const struct AiSimAction *action)
{
    u32 method = gMovesInfo[action->choice].argument.protectMethod;

    // The incoming Pokemon has no consecutive-Protect history, regardless of
    // the fainted slot's stale active-state bytes.
    if (HasForcedReplacement(action))
        return 1;

#if B_WIDE_GUARD >= GEN_6
    if (method == PROTECT_WIDE_GUARD)
        return 1;
#endif
#if B_QUICK_GUARD >= GEN_6
    if (method == PROTECT_QUICK_GUARD)
        return 1;
#endif
    if (method == PROTECT_CRAFTY_SHIELD)
        return 1;
    return AiSim_GetProtectSuccessDenominator(board, action->actor);
}

static bool32 ActionHasInvariantCappedDamage(const struct AiSimContext *context,
                                             const struct AiSimBoard *board,
                                             const struct AiSimJointTurn *turn,
                                             enum BattlerId actor)
{
    const struct AiSimAction *action = &turn->actions[actor];
    const struct AiSimPartyState *targetParty;
    struct SimulatedDamage projectedDamage;
    u32 other;

    if (action->kind != AI_SIM_ACTION_MOVE
     || action->gimmick != GIMMICK_NONE
     || HasForcedReplacement(action)
     || gMovesInfo[action->choice].power == 0
     || IsActionSpread(context, board, action)
     || action->target >= context->battlersCount)
        return FALSE;

    targetParty = GetConstPartyState(board, action->target);
    if (targetParty != NULL
     && targetParty->hp == 1
     && targetParty->item != ITEM_SITRUS_BERRY
     && AiSim_ProjectDamage(context, board, action, action->target,
                            &projectedDamage)
     && projectedDamage.minimum != 0)
    {
        bool32 stableTargetSlot = TRUE;

        // On the supported simulator boundary, Sitrus is the only effect that
        // can raise HP during a turn.  If no selected action can replace or
        // transform a slot, every positive hit against its one remaining HP
        // therefore has the same post-hit board regardless of roll, critical,
        // or target secondary.  Stat, screen, item-removal, and aura changes
        // may lower the formula result, but the live damage floor remains one.
        // Protect, skips, thawing, and dynamic action order retain their own
        // independent outcome dimensions.
        for (other = 0; other < MAX_BATTLERS_COUNT; other++)
        {
            const struct AiSimAction *otherAction;

            if (!(turn->actionMask & (1u << other)))
                continue;
            otherAction = &turn->actions[other];
            if (otherAction->kind == AI_SIM_ACTION_SWITCH
             || otherAction->gimmick != GIMMICK_NONE
             || HasForcedReplacement(otherAction))
            {
                stableTargetSlot = FALSE;
                break;
            }
        }
        if (stableTargetSlot)
            return TRUE;
    }

    // This is a whole-turn proof, not an isolated pre-turn damage estimate.
    // Every selected action must itself be a guaranteed capped single-target
    // hit.  Consequently an earlier secondary that could change a pending
    // attacker's damage (Moonblast, burn, Spectral Thief, Knock Off, etc.) can
    // reach that attacker only as part of a hit that KOs it first.  Status,
    // switch, field, and nonlethal damage actions retain their full branches.
    for (other = 0; other < MAX_BATTLERS_COUNT; other++)
    {
        const struct AiSimAction *otherAction;
        const struct MoveInfo *otherMove;
        const struct AiSimPartyState *otherActorParty;
        const struct AiSimPartyState *otherTargetParty;
        struct SimulatedDamage otherDamage;

        if (!(turn->actionMask & (1u << other)))
            continue;
        otherAction = &turn->actions[other];
        otherMove = &gMovesInfo[otherAction->choice];
        if (otherAction->kind != AI_SIM_ACTION_MOVE
         || otherAction->gimmick != GIMMICK_NONE
         || HasForcedReplacement(otherAction)
         || otherMove->power == 0
         || max(1, otherMove->strikeCount) != 1
         || IsActionSpread(context, board, otherAction)
         || otherAction->target >= context->battlersCount)
            return FALSE;
        otherActorParty = GetConstPartyState(board, other);
        otherTargetParty = GetConstPartyState(board, otherAction->target);
        if (otherActorParty == NULL || otherTargetParty == NULL
         || (otherActorParty->status1 & STATUS1_ICY_ANY)
         || otherTargetParty->item == ITEM_SITRUS_BERRY
         || !AiSim_ProjectDamage(context, board, otherAction,
                                 otherAction->target, &otherDamage)
         || otherDamage.minimum < otherTargetParty->hp)
            return FALSE;
    }

    // Fairy Aura is a shared damage input that can disappear if its owner is
    // KO'd before this action.  A frozen/frostbitten Guts user can likewise
    // lose its pre-turn boost through a thaw.  Neither is compatible with the
    // invariant lower-bound proof.
    for (other = 0; other < MAX_BATTLERS_COUNT; other++)
    {
        const struct AiSimCombatProfile *profile;

        if (!(board->activeMask & (1u << other)))
            continue;
        profile = GetProfile(context, board, other);
        if (profile == NULL || profile->ability == ABILITY_FAIRY_AURA)
            return FALSE;
    }
    // Sitrus is excluded above because it can raise a target beyond its
    // pre-turn HP before a later hit.  Full-HP Sash/Sturdy is projected to
    // HP-1 by AiSim_ProjectDamage and therefore cannot satisfy the proof.
    return TRUE;
}

static u32 GetPotentialThawMask(const struct AiSimContext *context,
                                const struct AiSimBoard *board,
                                const struct AiSimJointTurn *turn,
                                u32 invariantDamageMask)
{
    u32 actor;
    u32 mask = 0;

    (void)context;

    for (actor = 0; actor < MAX_BATTLERS_COUNT; actor++)
    {
        const struct AiSimAction *action;
        const struct AiSimPartyState *party;

        if (!(turn->actionMask & (1u << actor)))
            continue;
        action = &turn->actions[actor];
        if (action->kind != AI_SIM_ACTION_MOVE || MoveThawsUser(action->choice))
            continue;
        party = GetConstPartyState(board, actor);
        if (party != NULL && (party->status1 & STATUS1_FREEZE))
            mask |= 1u << actor;
    }

    // A target frozen before its pending move gets the same immediate 20%
    // thaw check as a battler that began the turn frozen.  Include the branch
    // for every pending Ice Beam target because speed-changing actions can
    // reorder the turn. If the beam ultimately does not precede or freeze it,
    // the paired branches are equivalent and sum back to the live result.
    for (actor = 0; actor < MAX_BATTLERS_COUNT; actor++)
    {
        const struct AiSimAction *action;
        const struct AiSimAction *targetAction;
        u32 target;

        if (!(turn->actionMask & (1u << actor)))
            continue;
        action = &turn->actions[actor];
        if (action->kind != AI_SIM_ACTION_MOVE
         || (invariantDamageMask & (1u << actor))
         || !HasSupportedIceBeamSecondary(action->choice))
            continue;
        target = action->target;
        if (target >= MAX_BATTLERS_COUNT || !(turn->actionMask & (1u << target)))
            continue;
        targetAction = &turn->actions[target];
        if (targetAction->kind == AI_SIM_ACTION_MOVE && !MoveThawsUser(targetAction->choice))
            mask |= 1u << target;
    }
    return mask;
}

enum AiSimOutcomeDimensionKind
{
    AI_SIM_OUTCOME_DIM_DAMAGE,
    AI_SIM_OUTCOME_DIM_DAMAGE_CLASS,
    AI_SIM_OUTCOME_DIM_CRITICAL,
    AI_SIM_OUTCOME_DIM_SECONDARY,
    AI_SIM_OUTCOME_DIM_PROTECT,
    AI_SIM_OUTCOME_DIM_THAW,
    AI_SIM_OUTCOME_DIM_SPEED_TIE,
};

struct AiSimOutcomeDimension
{
    u16 radix;
    u16 failureWeight;
    u16 successWeight;
    u8 kind;
    u8 actor;
};

#define AI_SIM_MAX_OUTCOME_DIMENSIONS (MAX_BATTLERS_COUNT * 5 + 1)
#define AI_SIM_MAX_DAMAGE_CLASSES 32

struct AiSimDamageClassScratch
{
    u8 keys[MAX_BATTLERS_COUNT][AI_SIM_MAX_DAMAGE_CLASSES];
    u8 counts[MAX_BATTLERS_COUNT];
    u16 weights[MAX_BATTLERS_COUNT][AI_SIM_MAX_DAMAGE_CLASSES];
    u16 amounts[AI_SIM_MAX_DAMAGE_CLASSES];
};

struct AiSimOutcomeEnumerationScratch
{
#if TESTING
    u32 lowerCanary;
#endif
    struct AiSimOutcomeDimension dimensions[AI_SIM_MAX_OUTCOME_DIMENSIONS];
    struct AiSimOutcomeKey representatives[AI_SIM_MAX_OUTCOMES];
    u64 weights[AI_SIM_MAX_OUTCOMES];
    u32 hashes[AI_SIM_MAX_OUTCOMES];
    struct AiSimBoard after;
    struct AiSimBoard representativeAfter;
    struct AiSimTurnResult result;
    struct AiSimTurnResult representativeResult;
    u8 tieOrders[24];
    u8 tieOrderWeights[24];
    struct AiSimDamageClassScratch damageClasses;
#if TESTING
    u32 upperCanary;
#endif
};

// Runtime enumeration runs from a battle-controller task with a much smaller
// stack than the standalone test runner.  The simulator is deliberately
// single-job/non-reentrant, so keep the complete enumeration workspace in
// shared EWRAM.  In particular, nested critical calculation copies a board;
// leaving the frontier and its two comparison boards on the controller stack
// would corrupt the task table before the exact fallback can finish.
static EWRAM_DATA struct AiSimOutcomeEnumerationScratch sAiSimOutcomeScratch;

#if TESTING
#define AI_SIM_OUTCOME_LOWER_CANARY 0xA15C20E1
#define AI_SIM_OUTCOME_UPPER_CANARY 0xA15C20E2

bool32 Test_AiSim_OutcomeScratchCanariesIntact(void)
{
    return sAiSimOutcomeScratch.lowerCanary == AI_SIM_OUTCOME_LOWER_CANARY
        && sAiSimOutcomeScratch.upperCanary == AI_SIM_OUTCOME_UPPER_CANARY;
}
#endif

static u32 GetCriticalHitDenominator(const struct AiSimBoard *board,
                                     const struct AiSimAction *action)
{
    static const u8 sGen7Odds[] = {24, 8, 2, 1, 1};
    static const u8 sGen6Odds[] = {16, 8, 2, 1, 1};
    static const u8 sGen3Odds[] = {16, 8, 4, 3, 2};
    u32 stage;

    if (board->sides[action->target & BIT_SIDE].statuses & SIDE_STATUS_LUCKY_CHANT)
        return 0;
    if (MoveAlwaysCrits(action->choice))
        return 1;
    stage = min(GetMoveCriticalHitStage(action->choice), ARRAY_COUNT(sGen7Odds) - 1);
    if (GetConfig(B_CRIT_CHANCE) >= GEN_7)
        return sGen7Odds[stage];
    if (GetConfig(B_CRIT_CHANCE) == GEN_6)
        return sGen6Odds[stage];
    // Pre-Generation-3 configurations are rejected categorically before
    // outcome enumeration because their odds are not this modern 1/N table.
    if (GetConfig(B_CRIT_CHANCE) < GEN_3)
        return 0;
    return sGen3Odds[stage];
}

static u32 OutcomeActionFactorial(u32 actionCount)
{
    u32 result = 1;

    while (actionCount > 1)
        result *= actionCount--;
    return result;
}

static bool32 BuildOutcomeTieRanks(const struct AiSimJointTurn *turn,
                                   u32 code,
                                   u8 ranks[MAX_BATTLERS_COUNT])
{
    enum BattlerId available[MAX_BATTLERS_COUNT];
    u32 count = 0;
    u32 position;
    enum BattlerId battler;

    memset(ranks, 0, MAX_BATTLERS_COUNT);
    for (battler = 0; battler < MAX_BATTLERS_COUNT; battler++)
    {
        if (turn->actionMask & (1u << battler))
            available[count++] = battler;
    }
    if (code >= OutcomeActionFactorial(count))
        return FALSE;
    for (position = 0; position < count; position++)
    {
        u32 remaining = count - position;
        u32 block = OutcomeActionFactorial(remaining - 1);
        u32 selected = code / block;
        enum BattlerId selectedBattler;

        code %= block;
        selectedBattler = available[selected];
        ranks[selectedBattler] = position;
        while (selected + 1 < remaining)
        {
            available[selected] = available[selected + 1];
            selected++;
        }
    }
    return TRUE;
}

static void BuildActionOrderStartBoard(const struct AiSimContext *context,
                                       const struct AiSimBoard *board,
                                       const struct AiSimJointTurn *turn,
                                       struct AiSimBoard *orderBoard)
{
    enum BattlerId actor;

    *orderBoard = *board;
    for (actor = 0; actor < MAX_BATTLERS_COUNT; actor++)
    {
        const struct AiSimAction *action = &turn->actions[actor];

        if (!(turn->actionMask & (1u << actor)))
            continue;
        if (action->kind == AI_SIM_ACTION_SWITCH)
            ApplyReplacement(context, orderBoard, actor, action->choice, NULL, TRUE);
        else if (HasForcedReplacement(action))
            ApplyReplacement(context, orderBoard, actor,
                             action->replacementRosterIndex, NULL, FALSE);
    }
    for (actor = 0; actor < MAX_BATTLERS_COUNT; actor++)
    {
        if ((turn->actionMask & (1u << actor))
         && turn->actions[actor].kind == AI_SIM_ACTION_MOVE
         && GetConfig(B_MEGA_EVO_TURN_ORDER) >= GEN_7)
            ActivateGimmick(context, orderBoard, &turn->actions[actor]);
    }
}

static void ActivateActionOrderGimmicks(const struct AiSimContext *context,
                                        struct AiSimBoard *board,
                                        const struct AiSimJointTurn *turn)
{
    enum BattlerId actor;

    for (actor = 0; actor < MAX_BATTLERS_COUNT; actor++)
    {
        if ((turn->actionMask & (1u << actor))
         && turn->actions[actor].kind == AI_SIM_ACTION_MOVE)
            ActivateGimmick(context, board, &turn->actions[actor]);
    }
}

static void BuildActionExecutionStartBoard(const struct AiSimContext *context,
                                           const struct AiSimBoard *board,
                                           const struct AiSimJointTurn *turn,
                                           struct AiSimBoard *executionBoard)
{
    BuildActionOrderStartBoard(context, board, turn, executionBoard);
    if (GetConfig(B_MEGA_EVO_TURN_ORDER) < GEN_7)
        ActivateActionOrderGimmicks(context, executionBoard, turn);
}

static u32 GetCurrentTiePairMask(const struct AiSimContext *context,
                                 const struct AiSimBoard *board,
                                 const struct AiSimJointTurn *turn,
                                 u8 remainingActionMask)
{
    u32 mask = 0;
    u32 pairBit = 0;
    enum BattlerId left;
    enum BattlerId right;

    for (left = 0; left < MAX_BATTLERS_COUNT; left++)
    {
        if (!(turn->actionMask & (1u << left)))
            continue;
        for (right = left + 1; right < MAX_BATTLERS_COUNT; right++)
        {
            if (!(turn->actionMask & (1u << right)))
                continue;
            if ((remainingActionMask & (1u << left))
             && (remainingActionMask & (1u << right))
             && GetSimMovePriority(context, board, &turn->actions[left])
                    == GetSimMovePriority(context, board, &turn->actions[right])
             && MovesLastInBracket(context, board, left)
                    == MovesLastInBracket(context, board, right)
             && AiSim_GetEffectiveSpeed(context, board, left)
                    == AiSim_GetEffectiveSpeed(context, board, right))
                mask |= 1u << pairBit;
            pairBit++;
        }
    }
    return mask;
}

static void ApplyPotentialActionOrderEffect(const struct AiSimContext *context,
                                            struct AiSimBoard *board,
                                            const struct AiSimAction *action)
{
    struct AiSimPartyState *party;
    struct AiSimActiveState *active;
    u32 effect;

    if (action->kind != AI_SIM_ACTION_MOVE)
        return;
    party = GetPartyState(board, action->actor);
    active = &board->active[action->actor];
    effect = gMovesInfo[action->choice].effect;
    if (effect == EFFECT_TAILWIND)
        board->sides[action->actor & BIT_SIDE].statuses |= SIDE_STATUS_TAILWIND;
    else if (effect == EFFECT_GEOMANCY)
    {
        bool32 completes = active->chargingMove == MOVE_GEOMANCY
            || (active->volatileFlags & AI_SIM_VOLATILE_GEOMANCY_CHARGING);

        if (!completes && party != NULL && party->item != ITEM_NONE
         && gItemsInfo[party->item].holdEffect == HOLD_EFFECT_POWER_HERB)
        {
            party->item = ITEM_NONE;
            completes = TRUE;
        }
        if (completes)
        {
            active->chargingMove = MOVE_NONE;
            active->volatileFlags &= ~AI_SIM_VOLATILE_GEOMANCY_CHARGING;
            active->statStages[STAT_SPEED] = min(MAX_STAT_STAGE,
                active->statStages[STAT_SPEED] + 2);
        }
        else
        {
            active->chargingMove = MOVE_GEOMANCY;
            active->volatileFlags |= AI_SIM_VOLATILE_GEOMANCY_CHARGING;
        }
    }
    else if (effect == EFFECT_KNOCK_OFF)
    {
        struct AiSimPartyState *targetParty = GetPartyState(board, action->target);

        if (targetParty != NULL
         && AiSim_IsItemRemovable(context, board, action->target))
            targetParty->item = ITEM_NONE;
    }
    else if (HasSupportedSpectralThiefEffect(action->choice)
          && GetSimTypeModifier(context, board, action, action->target) != UQ_4_12(0.0))
    {
        struct AiSimActiveState *target = &board->active[action->target];
        s32 stolen = target->statStages[STAT_SPEED] - DEFAULT_STAT_STAGE;

        if (stolen > 0 && active->statStages[STAT_SPEED] != MAX_STAT_STAGE)
        {
            target->statStages[STAT_SPEED] = DEFAULT_STAT_STAGE;
            active->statStages[STAT_SPEED] = min(MAX_STAT_STAGE,
                active->statStages[STAT_SPEED] + stolen);
        }
    }
}

static u32 GetPotentialTiePairMask(const struct AiSimContext *context,
                                   const struct AiSimBoard *board,
                                   const struct AiSimJointTurn *turn)
{
    // This routine is used only by the single-job exact enumerator.  Reuse
    // its as-yet-unneeded output board instead of placing a 540-byte board on
    // the battle-controller task's fixed 1 KiB stack.
    struct AiSimBoard *possibleBoard = &sAiSimOutcomeScratch.after;
    u32 actionCount = __builtin_popcount(turn->actionMask);
    u32 permutationCount = OutcomeActionFactorial(actionCount);
    u32 mask;
    u32 code;

    BuildActionOrderStartBoard(context, board, turn, possibleBoard);
    mask = GetCurrentTiePairMask(context, possibleBoard, turn,
                                 turn->actionMask);
    if (GetConfig(B_RECALC_TURN_AFTER_ACTIONS) < GEN_8)
        return mask;

    // Before Generation 7, the first action order is fixed using the old
    // forms even though the gimmicks themselves activate before moves.  A
    // Generation-8-style recalculation after the first move therefore uses
    // the post-gimmick profiles, just like move execution does.
    for (code = 0; code < permutationCount; code++)
    {
        u8 ranks[MAX_BATTLERS_COUNT];
        u8 remainingActionMask = turn->actionMask;
        u32 position;
        bool32 processedMoveAction = FALSE;

        // Rebuild instead of retaining a second 540-byte board copy on the
        // battle-controller task stack throughout exact enumeration.
        BuildActionOrderStartBoard(context, board, turn, possibleBoard);
        if (GetConfig(B_MEGA_EVO_TURN_ORDER) < GEN_7)
            ActivateActionOrderGimmicks(context, possibleBoard, turn);
        if (!BuildOutcomeTieRanks(turn, code, ranks))
            return 0;
        for (position = 0; position < actionCount; position++)
        {
            enum BattlerId actor;

            for (actor = 0; actor < MAX_BATTLERS_COUNT; actor++)
            {
                if ((turn->actionMask & (1u << actor)) && ranks[actor] == position)
                    break;
            }
            if (actor >= MAX_BATTLERS_COUNT)
                return 0;
            remainingActionMask &= ~(1u << actor);
            ApplyPotentialActionOrderEffect(context, possibleBoard,
                                            &turn->actions[actor]);
            if (turn->actions[actor].kind == AI_SIM_ACTION_MOVE)
                processedMoveAction = TRUE;
            if (processedMoveAction)
                mask |= GetCurrentTiePairMask(context, possibleBoard, turn,
                                              remainingActionMask);
        }
    }
    return mask;
}

static u32 BuildRelevantTieOrders(const struct AiSimContext *context,
                                  const struct AiSimBoard *board,
                                  const struct AiSimJointTurn *turn,
                                  u8 orders[24],
                                  u8 orderWeights[24])
{
    u32 actionCount = __builtin_popcount(turn->actionMask);
    u32 permutationCount = OutcomeActionFactorial(actionCount);
    u32 tiePairMask = GetPotentialTiePairMask(context, board, turn);
    u32 signatures[24];
    u32 count = 0;
    u32 code;

    if (tiePairMask == 0)
        return 0;
    for (code = 0; code < permutationCount; code++)
    {
        u8 ranks[MAX_BATTLERS_COUNT];
        u32 signature = 0;
        u32 pairBit = 0;
        u32 existing;
        enum BattlerId left;
        enum BattlerId right;

        if (!BuildOutcomeTieRanks(turn, code, ranks))
            return 0;
        for (left = 0; left < MAX_BATTLERS_COUNT; left++)
        {
            if (!(turn->actionMask & (1u << left)))
                continue;
            for (right = left + 1; right < MAX_BATTLERS_COUNT; right++)
            {
                if (!(turn->actionMask & (1u << right)))
                    continue;
                if ((tiePairMask & (1u << pairBit)) && ranks[left] < ranks[right])
                    signature |= 1u << pairBit;
                pairBit++;
            }
        }
        for (existing = 0; existing < count; existing++)
        {
            if (signatures[existing] == signature)
                break;
        }
        if (existing != count)
        {
            orderWeights[existing]++;
            continue;
        }
        signatures[count] = signature;
        orders[count++] = code;
        orderWeights[count - 1] = 1;
    }
    return count;
}

static void SetOutcomeDamageRoll(struct AiSimOutcomeKey *outcome,
                                 enum BattlerId actor,
                                 u32 roll)
{
    u32 shift = actor * 4;

    outcome->damageRolls &= ~(0xFu << shift);
    outcome->damageRolls |= (roll & 0xF) << shift;
}

static bool32 CanUseExactDamageClasses(const struct AiSimContext *context,
                                        const struct AiSimBoard *board,
                                        const struct AiSimJointTurn *turn)
{
    enum BattlerId battler;

    // A class represents every (roll, critical) pair that produces the same
    // raw damage.  That substitution is exact only while no action in the
    // turn can change a later action's damage inputs.  Keep this proof narrow:
    // ordinary one-hit moves have no field, status, stat, item-removal, or
    // gimmick effects of their own.
    for (battler = 0; battler < MAX_BATTLERS_COUNT; battler++)
    {
        const struct AiSimAction *action;
        const struct MoveInfo *move;

        if (!(turn->actionMask & (1u << battler)))
            continue;
        action = &turn->actions[battler];
        if (action->kind != AI_SIM_ACTION_MOVE
         || action->gimmick != GIMMICK_NONE
         || HasForcedReplacement(action))
            return FALSE;
        move = &gMovesInfo[action->choice];
        if (move->power == 0
         || move->effect != EFFECT_HIT
         || move->numAdditionalEffects != 0
         || max(1, move->strikeCount) != 1
         || IsActionSpread(context, board, action))
            return FALSE;
    }

    for (battler = 0; battler < MAX_BATTLERS_COUNT; battler++)
    {
        const struct AiSimPartyState *party;
        const struct AiSimCombatProfile *profile;

        if (!(board->activeMask & (1u << battler)))
            continue;
        party = GetConstPartyState(board, battler);
        profile = GetProfile(context, board, battler);
        if (party == NULL || profile == NULL)
            return FALSE;
        // Weakness Policy can change a pending attacker's offensive stages.
        // Fairy Aura can disappear when its owner is KO'd.  Freeze/frostbite
        // can disappear before a pending Guts attack through the exact thaw
        // branch or a Fire-type hit.  All three make initial-board damage an
        // invalid representative for a later action.
        if ((party->item != ITEM_NONE
          && gItemsInfo[party->item].holdEffect == HOLD_EFFECT_WEAKNESS_POLICY)
         || profile->ability == ABILITY_FAIRY_AURA
         || (party->status1 & STATUS1_ICY_ANY))
            return FALSE;
    }
    return TRUE;
}

static bool32 CanUseSingleDamageExactClasses(const struct AiSimContext *context,
                                             const struct AiSimBoard *board,
                                             const struct AiSimJointTurn *turn,
                                             u32 invariantDamageMask,
                                             u32 *damageActorMask)
{
    const struct AiSimAction *damageAction = NULL;
    const struct MoveInfo *damageMove = NULL;
    enum BattlerId battler;
    enum BattlerId damageActor = MAX_BATTLERS_COUNT;

    *damageActorMask = 0;
    for (battler = 0; battler < MAX_BATTLERS_COUNT; battler++)
    {
        const struct AiSimPartyState *party;

        if (!(board->activeMask & (1u << battler)))
            continue;
        party = GetConstPartyState(board, battler);
        if (party == NULL || (party->status1 & STATUS1_ICY_ANY))
            return FALSE;
    }

    // This secondary proof deliberately covers one ordinary hit only.  With
    // no other damage in the turn, the whitelisted actions below cannot KO an
    // aura owner, trigger Weakness Policy, or otherwise mutate damage inputs.
    for (battler = 0; battler < MAX_BATTLERS_COUNT; battler++)
    {
        const struct AiSimAction *action;
        const struct MoveInfo *move;

        if (!(turn->actionMask & (1u << battler)))
            continue;
        action = &turn->actions[battler];
        if (action->kind != AI_SIM_ACTION_MOVE)
            continue;
        move = &gMovesInfo[action->choice];
        if (move->power == 0)
            continue;
        if (damageAction != NULL
         || (invariantDamageMask & (1u << battler))
         || HasForcedReplacement(action)
         || (action->gimmick != GIMMICK_NONE
          && action->gimmick != GIMMICK_MEGA)
         || move->effect != EFFECT_HIT
         || move->numAdditionalEffects != 0
         || max(1, move->strikeCount) != 1
         || move->multiHit
         || IsActionSpread(context, board, action))
            return FALSE;
        damageAction = action;
        damageMove = move;
        damageActor = battler;
    }
    if (damageAction == NULL)
        return FALSE;

    for (battler = 0; battler < MAX_BATTLERS_COUNT; battler++)
    {
        const struct AiSimAction *action;

        if (!(turn->actionMask & (1u << battler)) || battler == damageActor)
            continue;
        action = &turn->actions[battler];
        if (action->kind == AI_SIM_ACTION_SWITCH)
        {
            const struct AiSimPartyState *replacementParty;
            u32 replacement = GetActionReplacement(action);

            if (action->gimmick != GIMMICK_NONE
             || battler == damageAction->target
             || replacement >= AI_SIM_ROSTER_COUNT
             || damageMove->type == TYPE_FAIRY)
                return FALSE;
            replacementParty = &board->party[replacement];
            if (replacementParty->status1 & STATUS1_ICY_ANY)
                return FALSE;
            continue;
        }
        if (action->kind != AI_SIM_ACTION_MOVE
         || action->gimmick != GIMMICK_NONE
         || HasForcedReplacement(action)
         || gMovesInfo[action->choice].power != 0)
            return FALSE;
        switch (gMovesInfo[action->choice].effect)
        {
        case EFFECT_TAILWIND:
        case EFFECT_TRICK_ROOM:
            break;
        case EFFECT_GEOMANCY:
            // Geomancy never changes physical Defense.  A special hit is
            // safe only when its target is not the Geomancy user whose
            // Sp. Def can rise before the hit lands.
            if (damageMove->category == DAMAGE_CATEGORY_SPECIAL
             && battler == damageAction->target)
                return FALSE;
            break;
        default:
            return FALSE;
        }
    }

    // Only Fairy Aura is a supported switch-time global damage input.  A
    // Fairy move with any switch was rejected above; non-Fairy damage is
    // invariant whether the passive switch adds or removes that aura.
    *damageActorMask = 1u << damageActor;
    return TRUE;
}

static u32 BuildExactDamageClasses(const struct AiSimContext *context,
                                   const struct AiSimBoard *board,
                                   const struct AiSimAction *action,
                                   u8 keys[AI_SIM_MAX_DAMAGE_CLASSES],
                                   u16 weights[AI_SIM_MAX_DAMAGE_CLASSES],
                                   u16 amounts[AI_SIM_MAX_DAMAGE_CLASSES])
{
    struct SimulatedDamage damage;
    u32 denominator = GetCriticalHitDenominator(board, action);
    u32 classCount = 0;
    u32 roll;
    bool32 critical;

    if (!AiSim_CalcDamage(context, board, action, action->target, &damage))
        return 0;
    memset(weights, 0, sizeof(*weights) * AI_SIM_MAX_DAMAGE_CLASSES);
    for (roll = 0; roll < 16; roll++)
    {
        for (critical = FALSE; critical <= TRUE; critical++)
        {
            struct AiSimOutcomeKey outcome =
            {
                .probabilityWeight = 1,
                .speedTieOrder = AI_SIM_SPEED_TIE_ORDER_CONTEXT,
            };
            u32 branchWeight;
            u32 amount;
            u32 existing;

            if ((denominator == 0 && critical)
             || (denominator == 1 && !critical))
                continue;
            SetOutcomeDamageRoll(&outcome, action->actor, roll);
            if (critical)
                outcome.criticalMask |= 1u << action->actor;
            branchWeight = denominator <= 1
                         ? 1
                         : (critical ? 1 : denominator - 1);
            amount = SelectDamageRoll(context, board, action, action->target,
                                      &outcome, &damage);
            for (existing = 0; existing < classCount; existing++)
            {
                if (amounts[existing] == amount)
                    break;
            }
            if (existing == classCount)
            {
                if (classCount >= AI_SIM_MAX_DAMAGE_CLASSES)
                    return 0;
                amounts[classCount] = amount;
                keys[classCount] = roll | (critical ? 0x10 : 0);
                classCount++;
            }
            if ((u32)weights[existing] + branchWeight > UINT16_MAX)
                return 0;
            weights[existing] += branchWeight;
        }
    }
    return classCount;
}

static u32 HashOutcomeBytes(u32 hash, const void *data, u32 size)
{
    const u8 *bytes = data;

    while (size-- != 0)
        hash = (hash ^ *bytes++) * 16777619u;
    return hash;
}

static u64 GreatestCommonDivisor64(u64 left, u64 right)
{
    while (right != 0)
    {
        u64 remainder = left % right;
        left = right;
        right = remainder;
    }
    return left;
}

u32 AiSim_EnumerateOutcomes(const struct AiSimContext *context,
                            const struct AiSimBoard *board,
                            const struct AiSimJointTurn *turn,
                            struct AiSimOutcomeKey *outcomes,
                            u32 capacity)
{
    struct AiSimOutcomeEnumerationScratch *scratch = &sAiSimOutcomeScratch;
    struct AiSimOutcomeDimension *dimensions = scratch->dimensions;
    struct AiSimOutcomeKey *representatives = scratch->representatives;
    u64 *weights = scratch->weights;
    u32 *hashes = scratch->hashes;
    struct AiSimBoard *after = &scratch->after;
    struct AiSimBoard *representativeAfter = &scratch->representativeAfter;
    struct AiSimTurnResult *result = &scratch->result;
    struct AiSimTurnResult *representativeResult = &scratch->representativeResult;
    u32 actor;
    u32 uncertainMask = 0;
    u32 deterministicSuccessMask = 0;
    u32 secondaryMask = 0;
    u32 invariantDamageMask = 0;
    u32 exactDamageCandidateMask = 0;
    u32 exactDamageClassMask = 0;
    u32 thawMask;
    u32 dimensionCount = 0;
    u32 uniqueCount = 0;
    u64 combinationCount = 1;
    u64 rawIndex;
    u32 tieOrderCount;
    u8 *tieOrders = scratch->tieOrders;
    u8 *tieOrderWeights = scratch->tieOrderWeights;
    u32 index;
    u32 applicationCount = 0;
    u32 lastHash = 0;
    u32 lastUnique = 0;
    const struct AiSimBoard *damageClassBoard = board;
    bool32 hasLast = FALSE;

#if TESTING
    sAiSimLastOutcomeApplicationCount = 0;
#endif
    if (AiSim_CheckJointTurn(context, board, turn, NULL) != AI_SIM_APPLY_OK)
        return 0;
    memset(scratch, 0, sizeof(*scratch));
#if TESTING
    scratch->lowerCanary = AI_SIM_OUTCOME_LOWER_CANARY;
    scratch->upperCanary = AI_SIM_OUTCOME_UPPER_CANARY;
#endif
    for (actor = 0; actor < MAX_BATTLERS_COUNT; actor++)
    {
        const struct AiSimAction *action;
        if (!(turn->actionMask & (1u << actor)))
            continue;
        action = &turn->actions[actor];
        if (action->kind != AI_SIM_ACTION_MOVE)
            continue;
        if (gMovesInfo[action->choice].power != 0
         && ActionHasInvariantCappedDamage(context, board, turn, actor))
            invariantDamageMask |= 1u << actor;
        if (GetSupportedSecondaryChance(action->choice) != 0
         && !(invariantDamageMask & (1u << actor)))
            secondaryMask |= 1u << actor;
        if (!IsProtectionMove(action->choice))
            continue;
        if (GetProtectActionSuccessDenominator(board, action) == 1)
            deterministicSuccessMask |= 1u << actor;
        else
            uncertainMask |= 1u << actor;
    }

    thawMask = GetPotentialThawMask(context, board, turn, invariantDamageMask);

    if (CanUseExactDamageClasses(context, board, turn))
    {
        for (actor = 0; actor < MAX_BATTLERS_COUNT; actor++)
        {
            const struct AiSimAction *action;

            if (!(turn->actionMask & (1u << actor))
             || (invariantDamageMask & (1u << actor)))
                continue;
            action = &turn->actions[actor];
            if (action->kind == AI_SIM_ACTION_MOVE
             && gMovesInfo[action->choice].power != 0)
                exactDamageCandidateMask |= 1u << actor;
        }
    }
    else if (CanUseSingleDamageExactClasses(context, board, turn,
                                             invariantDamageMask,
                                             &exactDamageCandidateMask))
    {
        BuildActionExecutionStartBoard(context, board, turn,
                                       representativeAfter);
        damageClassBoard = representativeAfter;
    }
    for (actor = 0; actor < MAX_BATTLERS_COUNT; actor++)
    {
        const struct AiSimAction *action;

        if (!(exactDamageCandidateMask & (1u << actor)))
            continue;
        action = &turn->actions[actor];
        scratch->damageClasses.counts[actor] = BuildExactDamageClasses(
            context, damageClassBoard, action,
            scratch->damageClasses.keys[actor],
            scratch->damageClasses.weights[actor],
            scratch->damageClasses.amounts);
        if (scratch->damageClasses.counts[actor] == 0)
            return 0;
        exactDamageClassMask |= 1u << actor;
    }

    // Mixed-radix enumeration changes the first dimension fastest.  Put the
    // dimensions that distinguish independent actors first so a genuinely
    // over-cap frontier proves its 33rd unique post-turn result promptly.
    // This changes only traversal order: every Cartesian-product branch and
    // its exact weight is still visited whenever the merged frontier fits.
    // Damage classes preserve the exact roll/critical probability mass while
    // collapsing only pairs with identical raw damage.  Unsafe turns retain
    // the original independent roll and critical dimensions.
    for (actor = 0; actor < MAX_BATTLERS_COUNT; actor++)
    {
        const struct AiSimAction *action;

        if (!(turn->actionMask & (1u << actor)))
            continue;
        action = &turn->actions[actor];
        if (action->kind == AI_SIM_ACTION_MOVE
         && gMovesInfo[action->choice].power != 0
         && !(invariantDamageMask & (1u << actor)))
        {
            dimensions[dimensionCount++] = (struct AiSimOutcomeDimension)
            {
                .radix = (exactDamageClassMask & (1u << actor))
                       ? scratch->damageClasses.counts[actor] : 16,
                .failureWeight = 1,
                .successWeight = 1,
                .kind = (exactDamageClassMask & (1u << actor))
                      ? AI_SIM_OUTCOME_DIM_DAMAGE_CLASS
                      : AI_SIM_OUTCOME_DIM_DAMAGE,
                .actor = actor,
            };
        }
    }

    // Board-equivalent speed orders are cheap after exact damage classes have
    // already exposed each independent HP result.  This ordering also lets a
    // true over-cap pure-hit frontier reach its 33rd board without traversing
    // all equivalent permutations first.
    tieOrderCount = BuildRelevantTieOrders(context, board, turn, tieOrders,
                                           tieOrderWeights);
    if (tieOrderCount != 0)
    {
        dimensions[dimensionCount++] = (struct AiSimOutcomeDimension)
        {
            .radix = tieOrderCount,
            .kind = AI_SIM_OUTCOME_DIM_SPEED_TIE,
            .actor = 0,
        };
    }

    // Raw damage rolls for every unsafe actor precede critical toggles.
    // Interleaving a single actor's critical dimension would exhaust all of
    // that actor's locally equivalent branches before exposing another
    // actor's independently changing HP result.
    for (actor = 0; actor < MAX_BATTLERS_COUNT; actor++)
    {
        const struct AiSimAction *action;
        u32 criticalDenominator;

        if (!(turn->actionMask & (1u << actor)))
            continue;
        action = &turn->actions[actor];
        if (action->kind != AI_SIM_ACTION_MOVE
         || gMovesInfo[action->choice].power == 0
         || (invariantDamageMask & (1u << actor))
         || (exactDamageClassMask & (1u << actor)))
            continue;
        criticalDenominator = GetCriticalHitDenominator(board, action);
        if (criticalDenominator > 1)
        {
            dimensions[dimensionCount++] = (struct AiSimOutcomeDimension)
            {
                .radix = 2,
                .failureWeight = criticalDenominator - 1,
                .successWeight = 1,
                .kind = AI_SIM_OUTCOME_DIM_CRITICAL,
                .actor = actor,
            };
        }
    }
    for (actor = 0; actor < MAX_BATTLERS_COUNT; actor++)
    {
        const struct AiSimAction *action = &turn->actions[actor];

        if (secondaryMask & (1u << actor))
        {
            dimensions[dimensionCount++] = (struct AiSimOutcomeDimension)
            {
                .radix = 2,
                .failureWeight = (100 - GetSupportedSecondaryChance(action->choice)) / 10,
                .successWeight = GetSupportedSecondaryChance(action->choice) / 10,
                .kind = AI_SIM_OUTCOME_DIM_SECONDARY,
                .actor = actor,
            };
        }
    }
    for (actor = 0; actor < MAX_BATTLERS_COUNT; actor++)
    {
        const struct AiSimAction *action = &turn->actions[actor];

        if (uncertainMask & (1u << actor))
        {
            dimensions[dimensionCount++] = (struct AiSimOutcomeDimension)
            {
                .radix = 2,
                .failureWeight = GetProtectActionSuccessDenominator(board, action) - 1,
                .successWeight = 1,
                .kind = AI_SIM_OUTCOME_DIM_PROTECT,
                .actor = actor,
            };
        }
    }
    for (actor = 0; actor < MAX_BATTLERS_COUNT; actor++)
    {
        if (thawMask & (1u << actor))
        {
            dimensions[dimensionCount++] = (struct AiSimOutcomeDimension)
            {
                .radix = 2,
                .failureWeight = 4,
                .successWeight = 1,
                .kind = AI_SIM_OUTCOME_DIM_THAW,
                .actor = actor,
            };
        }
    }
    for (index = 0; index < dimensionCount; index++)
    {
        if (dimensions[index].radix == 0
         || combinationCount > UINT64_MAX / dimensions[index].radix)
            return 0;
        combinationCount *= dimensions[index].radix;
    }

    for (rawIndex = 0; rawIndex < combinationCount; rawIndex++)
    {
        struct AiSimOutcomeKey raw =
        {
            .probabilityWeight = 1,
            .flags = 0,
            .protectSuccessMask = deterministicSuccessMask,
            .criticalMask = 0,
            .speedTieOrder = tieOrderCount != 0 ? tieOrders[0] : AI_SIM_SPEED_TIE_ORDER_CONTEXT,
        };
        u64 branch = rawIndex;
        u64 rawWeight = 1;
        u32 hash;
        bool32 merged = FALSE;

        for (actor = 0; actor < MAX_BATTLERS_COUNT; actor++)
        {
            SetOutcomeDamageRoll(&raw, actor, AI_SIM_DAMAGE_ROLL_MEDIAN);
            if ((turn->actionMask & (1u << actor))
             && turn->actions[actor].kind == AI_SIM_ACTION_MOVE
             && gMovesInfo[turn->actions[actor].choice].power != 0
             && GetCriticalHitDenominator(board, &turn->actions[actor]) == 1
             && !(invariantDamageMask & (1u << actor))
             && !(exactDamageClassMask & (1u << actor)))
                raw.criticalMask |= 1u << actor;
        }
        for (index = 0; index < dimensionCount; index++)
        {
            const struct AiSimOutcomeDimension *dimension = &dimensions[index];
            u32 choice = branch % dimension->radix;
            u32 factor = 1;

            branch /= dimension->radix;
            switch (dimension->kind)
            {
            case AI_SIM_OUTCOME_DIM_DAMAGE:
                SetOutcomeDamageRoll(&raw, dimension->actor, choice);
                break;
            case AI_SIM_OUTCOME_DIM_DAMAGE_CLASS:
            {
                u32 key = scratch->damageClasses.keys[dimension->actor][choice];

                SetOutcomeDamageRoll(&raw, dimension->actor, key & 0xF);
                if (key & 0x10)
                    raw.criticalMask |= 1u << dimension->actor;
                factor = scratch->damageClasses.weights[dimension->actor][choice];
                break;
            }
            case AI_SIM_OUTCOME_DIM_CRITICAL:
                if (choice != 0)
                    raw.criticalMask |= 1u << dimension->actor;
                factor = choice == 0 ? dimension->failureWeight : dimension->successWeight;
                break;
            case AI_SIM_OUTCOME_DIM_SECONDARY:
            {
                if (choice != 0)
                    raw.flags |= 1u << dimension->actor;
                factor = choice == 0 ? dimension->failureWeight : dimension->successWeight;
                break;
            }
            case AI_SIM_OUTCOME_DIM_PROTECT:
                if (choice != 0)
                    raw.protectSuccessMask |= 1u << dimension->actor;
                factor = choice == 0 ? dimension->failureWeight : dimension->successWeight;
                break;
            case AI_SIM_OUTCOME_DIM_THAW:
                if (choice != 0)
                    raw.flags |= 1u << (dimension->actor + AI_SIM_OUTCOME_THAW_SHIFT);
                factor = choice == 0 ? dimension->failureWeight : dimension->successWeight;
                break;
            case AI_SIM_OUTCOME_DIM_SPEED_TIE:
                raw.speedTieOrder = tieOrders[choice];
                factor = tieOrderWeights[choice];
                break;
            }
            if (factor != 0 && rawWeight > UINT64_MAX / factor)
                return 0;
            rawWeight *= factor;
        }

        // Merging cannot itself prove that a huge raw Cartesian product is
        // cheap: many branches can collapse onto the same <=32 boards.  Keep
        // exact enumeration inside an explicit supported envelope and fail
        // closed before one call can monopolize a battle-controller task.
        if (applicationCount >= AI_SIM_MAX_OUTCOME_APPLICATIONS)
            return AI_SIM_MAX_OUTCOMES + 1;
        applicationCount++;
#if TESTING
        sAiSimLastOutcomeApplicationCount = applicationCount;
#endif
        if (AiSim_ApplyJointTurn(context, board, turn, &raw, after, result) != AI_SIM_APPLY_OK)
            return 0;
        hash = HashOutcomeBytes(2166136261u, after, sizeof(*after));
        if (hasLast && hash == lastHash
         && memcmp(after, representativeAfter, sizeof(*after)) == 0)
        {
            if (weights[lastUnique] > UINT64_MAX - rawWeight)
                return 0;
            weights[lastUnique] += rawWeight;
            merged = TRUE;
        }
        for (index = 0; index < uniqueCount; index++)
        {
            if (merged)
                break;
            if (hashes[index] != hash)
                continue;
            if (applicationCount >= AI_SIM_MAX_OUTCOME_APPLICATIONS)
                return AI_SIM_MAX_OUTCOMES + 1;
            applicationCount++;
#if TESTING
            sAiSimLastOutcomeApplicationCount = applicationCount;
#endif
            if (AiSim_ApplyJointTurn(context, board, turn, &representatives[index],
                                     representativeAfter, representativeResult) != AI_SIM_APPLY_OK)
                return 0;
            if (memcmp(after, representativeAfter, sizeof(*after)) != 0)
                continue;
            if (weights[index] > UINT64_MAX - rawWeight)
                return 0;
            weights[index] += rawWeight;
            lastUnique = index;
            merged = TRUE;
            break;
        }
        if (merged)
        {
            *representativeAfter = *after;
            lastHash = hash;
            hasLast = TRUE;
            continue;
        }
        if (uniqueCount >= AI_SIM_MAX_OUTCOMES)
            return AI_SIM_MAX_OUTCOMES + 1;
        representatives[uniqueCount] = raw;
        weights[uniqueCount] = rawWeight;
        hashes[uniqueCount] = hash;
        lastUnique = uniqueCount;
        uniqueCount++;
        *representativeAfter = *after;
        lastHash = hash;
        hasLast = TRUE;
    }

    if (uniqueCount != 0)
    {
        u64 divisor = weights[0];
        u64 totalWeight = 0;

        for (index = 1; index < uniqueCount; index++)
            divisor = GreatestCommonDivisor64(divisor, weights[index]);
        divisor = max(1, divisor);
        for (index = 0; index < uniqueCount; index++)
        {
            weights[index] /= divisor;
            if (weights[index] > UINT32_MAX || totalWeight > UINT32_MAX - weights[index])
                return 0;
            representatives[index].probabilityWeight = weights[index];
            totalWeight += weights[index];
        }
    }
    if (outcomes != NULL && capacity >= uniqueCount)
        memcpy(outcomes, representatives, sizeof(*outcomes) * uniqueCount);
    return uniqueCount;
}

static s32 GetSimMovePriority(const struct AiSimContext *context,
                              const struct AiSimBoard *board,
                              const struct AiSimAction *action)
{
    const struct AiSimCombatProfile *profile;
    enum Move priorityMove = action->choice;
    s32 priority;

    if (action->kind == AI_SIM_ACTION_SWITCH)
        return 127;
    if (priorityMove == MOVE_NONE || priorityMove >= MOVES_COUNT_ALL)
        return 0;
    priority = gMovesInfo[priorityMove].priority;
    profile = GetProfile(context, board, action->actor);
    if (profile != NULL && profile->ability == ABILITY_PRANKSTER
     && gMovesInfo[priorityMove].category == DAMAGE_CATEGORY_STATUS)
        priority++;
    return priority;
}

static bool32 MovesLastInBracket(const struct AiSimContext *context,
                                 const struct AiSimBoard *board,
                                 enum BattlerId battler)
{
    const struct AiSimCombatProfile *profile = GetProfile(context, board, battler);
    const struct AiSimPartyState *party = GetConstPartyState(board, battler);

    if (profile == NULL || party == NULL)
        return FALSE;
    return profile->ability == ABILITY_STALL
        || (party->item != ITEM_NONE && gItemsInfo[party->item].holdEffect == HOLD_EFFECT_LAGGING_TAIL);
}

static bool32 ActionComesBefore(const struct AiSimContext *context,
                                const struct AiSimBoard *board,
                                const struct AiSimJointTurn *turn,
                                const struct AiSimOutcomeKey *outcome,
                                enum BattlerId lhs,
                                enum BattlerId rhs)
{
    const struct AiSimAction *left = &turn->actions[lhs];
    const struct AiSimAction *right = &turn->actions[rhs];
    s32 leftPriority = GetSimMovePriority(context, board, left);
    s32 rightPriority = GetSimMovePriority(context, board, right);
    bool32 leftLast;
    bool32 rightLast;
    u32 leftSpeed;
    u32 rightSpeed;

    if (leftPriority != rightPriority)
        return leftPriority > rightPriority;
    leftLast = MovesLastInBracket(context, board, lhs);
    rightLast = MovesLastInBracket(context, board, rhs);
    if (leftLast != rightLast)
        return !leftLast;

    leftSpeed = AiSim_GetEffectiveSpeed(context, board, lhs);
    rightSpeed = AiSim_GetEffectiveSpeed(context, board, rhs);
    if (leftSpeed != rightSpeed)
    {
        if (board->fieldStatuses & STATUS_FIELD_TRICK_ROOM)
            return leftSpeed < rightSpeed;
        return leftSpeed > rightSpeed;
    }

    if (outcome->speedTieOrder != AI_SIM_SPEED_TIE_ORDER_CONTEXT)
    {
        u8 ranks[MAX_BATTLERS_COUNT] = {0};

        if (!BuildOutcomeTieRanks(turn, outcome->speedTieOrder, ranks))
            return lhs < rhs;
        return ranks[lhs] < ranks[rhs];
    }
    if (turn->flags & AI_SIM_JOINT_CONSERVATIVE_ENEMY_TIES)
    {
        bool32 leftConfirmed = turn->confirmedPlayerMask & (1u << lhs);
        bool32 rightConfirmed = turn->confirmedPlayerMask & (1u << rhs);
        if (leftConfirmed != rightConfirmed)
            return leftConfirmed;
    }
    if (context->tieRank[lhs] != context->tieRank[rhs])
        return context->tieRank[lhs] < context->tieRank[rhs];
    return lhs < rhs;
}

static bool32 BuildInitialActionOrder(const struct AiSimContext *context,
                                      const struct AiSimBoard *board,
                                      const struct AiSimJointTurn *turn,
                                      const struct AiSimOutcomeKey *outcome,
                                      struct AiSimBoard *workspace,
                                      u8 order[MAX_BATTLERS_COUNT])
{
    u8 remainingActionMask = turn->actionMask;
    u32 actionCount = __builtin_popcount(turn->actionMask);
    u32 position;

    BuildActionOrderStartBoard(context, board, turn, workspace);
    for (position = 0; position < actionCount; position++)
    {
        enum BattlerId actor = MAX_BATTLERS_COUNT;
        enum BattlerId candidate;

        for (candidate = 0; candidate < MAX_BATTLERS_COUNT; candidate++)
        {
            if (!(remainingActionMask & (1u << candidate)))
                continue;
            if (actor == MAX_BATTLERS_COUNT
             || ActionComesBefore(context, workspace, turn, outcome,
                                  candidate, actor))
                actor = candidate;
        }
        if (actor >= MAX_BATTLERS_COUNT)
            return FALSE;
        remainingActionMask &= ~(1u << actor);
        order[position] = actor;
    }
    return TRUE;
}

static u16 GetCurrentMaxHp(const struct AiSimContext *context,
                           const struct AiSimBoard *board,
                           enum BattlerId battler)
{
    const struct AiSimCombatProfile *profile = GetProfile(context, board, battler);
    const struct AiSimMonTemplate *mon;
    u32 maxHp;

    if (profile == NULL)
        return 0;
    mon = &context->mons[board->active[battler].rosterIndex];
    maxHp = profile->maxHp;
    if (board->active[battler].flags & AI_SIM_ACTIVE_DYNAMAX)
    {
        if (mon->dynamaxHpPercent < 100)
            return 0;
        maxHp = (maxHp * mon->dynamaxHpPercent + 50) / 100;
    }
    return min(0xFFFF, maxHp);
}

static s8 ClampStage(s32 stage)
{
    return min(MAX_STAT_STAGE, max(MIN_STAT_STAGE, stage));
}

static void RaiseStage(struct AiSimBoard *board, enum BattlerId battler, u32 stat, s32 amount)
{
    board->active[battler].statStages[stat] = ClampStage(board->active[battler].statStages[stat] + amount);
}

static bool32 LowerStage(struct AiSimBoard *board, enum BattlerId battler, u32 stat, s32 amount)
{
    s8 before = board->active[battler].statStages[stat];

    board->active[battler].statStages[stat] = ClampStage(before - amount);
    return board->active[battler].statStages[stat] != before;
}

static bool32 SpeciesUsesHoldItemToChangeForm(enum Species species, u32 item)
{
    const struct FormChange *formChanges = GetSpeciesFormChanges(species);
    u32 i;

    for (i = 0;
         formChanges != NULL
      && formChanges[i].method != FORM_CHANGE_TERMINATOR;
         i++)
    {
        switch (formChanges[i].method)
        {
        case FORM_CHANGE_BATTLE_MEGA_EVOLUTION_ITEM:
        case FORM_CHANGE_BATTLE_PRIMAL_REVERSION:
        case FORM_CHANGE_BATTLE_ULTRA_BURST:
        case FORM_CHANGE_ITEM_HOLD:
        case FORM_CHANGE_BEGIN_BATTLE:
            if (formChanges[i].param1 == item)
                return TRUE;
            break;
        default:
            break;
        }
    }
    return FALSE;
}

bool32 AiSim_IsItemRemovable(const struct AiSimContext *context,
                             const struct AiSimBoard *board,
                             enum BattlerId battler)
{
    const struct AiSimCombatProfile *profile = GetProfile(context, board,
                                                           battler);
    const struct AiSimPartyState *party = GetConstPartyState(board, battler);
    u32 item;

    if (profile == NULL || party == NULL)
        return FALSE;
    item = party->item;
    if (item == ITEM_NONE || item >= ITEMS_COUNT
     || ItemIsMail(item)
     || item == ITEM_ENIGMA_BERRY_E_READER
     || SpeciesUsesHoldItemToChangeForm(profile->species, item)
     || gItemsInfo[item].holdEffect == HOLD_EFFECT_Z_CRYSTAL)
        return FALSE;
    return TRUE;
}

static uq4_12_t GetSimTypeModifier(const struct AiSimContext *context,
                                   const struct AiSimBoard *board,
                                   const struct AiSimAction *action,
                                   enum BattlerId target)
{
    const struct AiSimCombatProfile *profile = GetProfile(context, board, target);
    const struct AiSimMonTemplate *mon;
    uq4_12_t modifier = UQ_4_12(1.0);
    u32 moveType = gMovesInfo[action->choice].type;
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

static bool32 IsMoveBlocked(const struct AiSimContext *context,
                            const struct AiSimBoard *board,
                            const struct AiSimAction *action,
                            enum BattlerId target,
                            bool32 *chip)
{
    const struct AiSimActiveState *targetState = &board->active[target];
    u32 targetSide = target & BIT_SIDE;
    bool32 maxOrZ = action->gimmick == GIMMICK_Z_MOVE
        || action->choice >= FIRST_Z_MOVE
        || action->choice >= FIRST_MAX_MOVE;

    *chip = FALSE;
    if (gMovesInfo[action->choice].ignoresProtect)
        return FALSE;
    if (targetState->volatileFlags & AI_SIM_VOLATILE_MAX_GUARD)
        return TRUE;
    if (targetState->volatileFlags & AI_SIM_VOLATILE_PROTECTED)
    {
        if (maxOrZ)
        {
            *chip = TRUE;
            return FALSE;
        }
        return TRUE;
    }
    // Z-Moves and Max Moves bypass side guards in the live engine. Ordinary
    // single-target Protect still reduces Z/Max damage above, while Max Guard
    // blocks it completely.
    if (maxOrZ)
        return FALSE;
    if ((board->sides[targetSide].flags & AI_SIM_SIDE_QUICK_GUARD)
     && GetSimMovePriority(context, board, action) > 0
     && ((action->actor ^ target) & BIT_SIDE) != 0)
        return TRUE;
    if ((board->sides[targetSide].flags & AI_SIM_SIDE_WIDE_GUARD)
     && IsActionSpread(context, board, action)
     && ((action->actor ^ target) & BIT_SIDE) != 0)
        return TRUE;
    return FALSE;
}

static bool32 IsPsychicTerrainPriorityBlocked(const struct AiSimContext *context,
                                              const struct AiSimBoard *board,
                                              const struct AiSimAction *action,
                                              enum BattlerId target)
{
    u32 moveTarget = gMovesInfo[action->choice].target;

    return (board->fieldStatuses & STATUS_FIELD_PSYCHIC_TERRAIN)
        && ((action->actor ^ target) & BIT_SIDE) != 0
        && GetSimMovePriority(context, board, action) > 0
        && moveTarget != TARGET_ALL_BATTLERS
        && moveTarget != TARGET_OPPONENTS_FIELD
        && IsSimBattlerGrounded(context, board, target);
}

static bool32 IsPranksterDarkTypeBlocked(const struct AiSimContext *context,
                                         const struct AiSimBoard *board,
                                         const struct AiSimAction *action,
                                         enum BattlerId target)
{
    const struct AiSimCombatProfile *actorProfile = GetProfile(context, board, action->actor);
    u32 moveTarget = gMovesInfo[action->choice].target;

    return GetConfig(B_PRANKSTER_DARK_TYPES) >= GEN_7
        && actorProfile != NULL
        && actorProfile->ability == ABILITY_PRANKSTER
        && gMovesInfo[action->choice].category == DAMAGE_CATEGORY_STATUS
        && GetSimMovePriority(context, board, action) > 0
        && ((action->actor ^ target) & BIT_SIDE) != 0
        && moveTarget != TARGET_DEPENDS
        && moveTarget != TARGET_OPPONENTS_FIELD
        && IsSimBattlerOfType(context, board, target, TYPE_DARK);
}

static bool32 ApplySpectralThiefPreDamage(const struct AiSimContext *context,
                                          struct AiSimBoard *board,
                                          const struct AiSimAction *action,
                                          enum BattlerId target,
                                          struct AiSimTurnResult *result)
{
    bool32 stoleStats = FALSE;
    u32 stat;

    if (!HasSupportedSpectralThiefEffect(action->choice))
        return TRUE;
    // Spectral Thief's pre-attack effect does not run into an immune target.
    if (GetSimTypeModifier(context, board, action, target) == UQ_4_12(0.0))
        return FALSE;
    for (stat = STAT_ATK; stat < NUM_BATTLE_STATS; stat++)
    {
        s8 targetStage = board->active[target].statStages[stat];

        // The live engine leaves this target boost in place when the thief is
        // already at the maximum stage for that stat.
        if (targetStage > DEFAULT_STAT_STAGE
         && board->active[action->actor].statStages[stat] != MAX_STAT_STAGE)
        {
            board->active[target].statStages[stat] = DEFAULT_STAT_STAGE;
            RaiseStage(board, action->actor, stat, targetStage - DEFAULT_STAT_STAGE);
            stoleStats = TRUE;
        }
    }
    if (stoleStats && result != NULL)
        result->events |= AI_SIM_EVENT_STAT_CHANGE;
    return TRUE;
}

static u16 ProjectSurvivalDamage(u16 amount, u32 hp)
{
    if (amount >= hp)
        return hp - 1;
    return amount;
}

bool32 AiSim_ProjectDamage(const struct AiSimContext *context,
                           const struct AiSimBoard *board,
                           const struct AiSimAction *action,
                           enum BattlerId target,
                           struct SimulatedDamage *damage)
{
    struct AiSimBoard gimmickBoard;
    struct SimulatedDamage projectedDamage = {0};
    const struct AiSimBoard *damageBoard = board;
    const struct AiSimPartyState *actorParty;
    const struct AiSimPartyState *targetParty;
    const struct AiSimActiveState *actorState;
    const struct AiSimCombatProfile *targetProfile;
    enum AiSimApplyStatus status;
    bool32 chip;
    bool32 spectralThief;
    u32 unsupportedFlags = AI_SIM_UNSUPPORTED_NONE;
    u32 maxHp;
    u32 strikeCount;

    if (damage != NULL)
        *damage = (struct SimulatedDamage){0};
    if (context == NULL || board == NULL || action == NULL || damage == NULL
     || context->battlersCount == 0
     || context->battlersCount > MAX_BATTLERS_COUNT
     || action->kind != AI_SIM_ACTION_MOVE
     || action->actor >= MAX_BATTLERS_COUNT
     || target >= MAX_BATTLERS_COUNT
     || action->actor >= context->battlersCount
     || target >= context->battlersCount
     || action->choice == MOVE_NONE || action->choice >= MOVES_COUNT_ALL
     || !IsSimBattlerAlive(board, action->actor)
     || !IsSimBattlerAlive(board, target)
     || !IsAffectedTarget(context, board, action, target)
     || !IsMoveInTemplate(context, board, action)
     || !IsMoveTargetValid(board, action))
        return FALSE;

    actorParty = GetConstPartyState(board, action->actor);
    actorState = &board->active[action->actor];
    targetParty = GetConstPartyState(board, target);
    targetProfile = GetProfile(context, board, target);
    if (actorParty == NULL || targetParty == NULL || targetProfile == NULL
     || action->moveSlot >= MAX_MON_MOVES
     || actorParty->pp[action->moveSlot] == 0
     || (actorState->disabledMoveMask & (1u << action->moveSlot))
     || (actorState->choiceMoveSlot != AI_SIM_MOVE_SLOT_NONE
      && actorState->choiceMoveSlot != action->moveSlot)
     || actorState->chargingMove != MOVE_NONE
     || (actorState->volatileFlags & AI_SIM_VOLATILE_GEOMANCY_CHARGING)
     || (actorState->volatileFlags & AI_SIM_VOLATILE_FLINCHED)
     || (actorState->flags & (AI_SIM_ACTIVE_NEEDS_REPLACEMENT | AI_SIM_ACTIVE_DYNAMAX))
     || (actorParty->status1 & (STATUS1_SLEEP | STATUS1_PARALYSIS | STATUS1_ICY_ANY))
     || (action->choice == MOVE_FAKE_OUT && !actorState->firstTurn)
     || AiSim_GetActionRejectionFlags(context, board, action) != 0)
        return FALSE;

    status = CheckGimmick(context, board, action, &unsupportedFlags);
    if (status != AI_SIM_APPLY_OK || unsupportedFlags != AI_SIM_UNSUPPORTED_NONE)
        return FALSE;
    spectralThief = HasSupportedSpectralThiefEffect(action->choice);
    if (action->gimmick != GIMMICK_NONE || spectralThief)
    {
        gimmickBoard = *board;
        if (action->gimmick != GIMMICK_NONE)
            ActivateGimmick(context, &gimmickBoard, action);
        damageBoard = &gimmickBoard;
    }
    status = CheckMoveSupport(context, damageBoard, action, &unsupportedFlags,
                              target, spectralThief ? NULL : &projectedDamage);
    if (status != AI_SIM_APPLY_OK || unsupportedFlags != AI_SIM_UNSUPPORTED_NONE)
        return FALSE;
    targetParty = GetConstPartyState(damageBoard, target);
    targetProfile = GetProfile(context, damageBoard, target);
    if (targetParty == NULL || targetProfile == NULL)
        return FALSE;
    if (IsPsychicTerrainPriorityBlocked(context, damageBoard, action, target))
        return FALSE;
    if (IsMoveBlocked(context, damageBoard, action, target, &chip) && !chip)
        return FALSE;
    if (spectralThief
     && (!ApplySpectralThiefPreDamage(context, &gimmickBoard, action, target, NULL)
      || !AiSim_CalcDamage(context, &gimmickBoard, action, target, &projectedDamage)))
        return FALSE;
    if (projectedDamage.maximum == 0)
        return FALSE;

    if (chip)
    {
        projectedDamage.minimum = max(1, projectedDamage.minimum / 4);
        projectedDamage.median = max(1, projectedDamage.median / 4);
        projectedDamage.maximum = max(1, projectedDamage.maximum / 4);
    }
    maxHp = GetCurrentMaxHp(context, damageBoard, target);
    strikeCount = max(1, gMovesInfo[action->choice].strikeCount);
    if (strikeCount == 1 && targetParty->hp == maxHp
     && ((targetParty->item != ITEM_NONE
       && gItemsInfo[targetParty->item].holdEffect == HOLD_EFFECT_FOCUS_SASH)
      || targetProfile->ability == ABILITY_STURDY))
    {
        projectedDamage.minimum = ProjectSurvivalDamage(projectedDamage.minimum, targetParty->hp);
        projectedDamage.median = ProjectSurvivalDamage(projectedDamage.median, targetParty->hp);
        projectedDamage.maximum = ProjectSurvivalDamage(projectedDamage.maximum, targetParty->hp);
    }
    projectedDamage.random = projectedDamage.median;
    *damage = projectedDamage;
    return TRUE;
}

static u32 GetOutcomeDamageRoll(const struct AiSimOutcomeKey *outcome,
                                enum BattlerId actor)
{
    return (outcome->damageRolls >> (actor * 4)) & 0xF;
}

static bool32 CalcCriticalDamage(const struct AiSimContext *context,
                                 const struct AiSimBoard *board,
                                 const struct AiSimAction *action,
                                 enum BattlerId target,
                                 struct SimulatedDamage *damage)
{
    struct AiSimBoard criticalBoard = *board;
    const struct MoveInfo *move = &gMovesInfo[action->choice];
    const struct MoveInfo *baseMove = move;
    u32 attackStage;
    u32 defenseStage;

    if (action->choice >= FIRST_Z_MOVE && action->choice <= LAST_Z_MOVE)
    {
        u32 rosterIndex = board->active[action->actor].rosterIndex;
        enum Move baseMoveId;

        if (rosterIndex >= AI_SIM_ROSTER_COUNT || action->moveSlot >= MAX_MON_MOVES)
            return FALSE;
        baseMoveId = context->mons[rosterIndex].moves[action->moveSlot];
        if (baseMoveId == MOVE_NONE || baseMoveId >= MOVES_COUNT)
            return FALSE;
        baseMove = &gMovesInfo[baseMoveId];
    }
    attackStage = baseMove->category == DAMAGE_CATEGORY_PHYSICAL ? STAT_ATK : STAT_SPATK;
    defenseStage = (baseMove->category == DAMAGE_CATEGORY_PHYSICAL
                 || move->effect == EFFECT_PSYSHOCK) ? STAT_DEF : STAT_SPDEF;
    if (criticalBoard.active[action->actor].statStages[attackStage] < DEFAULT_STAT_STAGE)
        criticalBoard.active[action->actor].statStages[attackStage] = DEFAULT_STAT_STAGE;
    if (criticalBoard.active[target].statStages[defenseStage] > DEFAULT_STAT_STAGE)
        criticalBoard.active[target].statStages[defenseStage] = DEFAULT_STAT_STAGE;
    criticalBoard.sides[target & BIT_SIDE].statuses &= ~(SIDE_STATUS_REFLECT
                                                       | SIDE_STATUS_LIGHTSCREEN
                                                       | SIDE_STATUS_AURORA_VEIL);
    if (!AiSim_CalcDamage(context, &criticalBoard, action, target, damage))
        return FALSE;
    if (damage->maximum != 0)
    {
        u32 multiplier = GetConfig(B_CRIT_MULTIPLIER) >= GEN_6 ? 3 : 4;

        damage->maximum = min(UINT16_MAX, (u32)damage->maximum * multiplier / 2);
    }
    return TRUE;
}

static u32 SelectDamageRoll(const struct AiSimContext *context,
                            const struct AiSimBoard *board,
                            const struct AiSimAction *action,
                            enum BattlerId target,
                            const struct AiSimOutcomeKey *outcome,
                            const struct SimulatedDamage *damage)
{
    struct SimulatedDamage criticalDamage;
    u32 baseDamage = damage->maximum;
    u32 roll = GetOutcomeDamageRoll(outcome, action->actor);

    if (outcome->criticalMask & (1u << action->actor))
    {
        if (!CalcCriticalDamage(context, board, action, target, &criticalDamage))
            return 0;
        baseDamage = criticalDamage.maximum;
    }
    if (baseDamage == 0)
        return 0;
    return max(1, baseDamage * (85 + roll) / 100);
}

static void TryThawIcyTarget(struct AiSimBoard *board,
                             const struct AiSimAction *action,
                             enum BattlerId target,
                             struct AiSimTurnResult *result)
{
    struct AiSimPartyState *party = GetPartyState(board, target);
    bool32 thaws = FALSE;

    if (party == NULL || party->hp == 0 || !(party->status1 & STATUS1_ICY_ANY))
        return;
    if (MoveThawsUser(action->choice))
        thaws = TRUE;
#if B_HIT_THAW >= GEN_3
    else if ((party->status1 & STATUS1_FREEZE)
          && gMovesInfo[action->choice].type == TYPE_FIRE)
        thaws = TRUE;
#endif
    if (thaws)
    {
        party->status1 &= ~STATUS1_ICY_ANY;
        result->events |= AI_SIM_EVENT_STATUS_CHANGE;
    }
}

static bool32 ApplyDamageToTarget(const struct AiSimContext *context,
                                  struct AiSimBoard *board,
                                  const struct AiSimAction *action,
                                  enum BattlerId target,
                                  const struct AiSimOutcomeKey *outcome,
                                  struct AiSimTurnResult *result,
                                  bool32 *blocked)
{
    struct SimulatedDamage damage;
    struct AiSimPartyState *party = GetPartyState(board, target);
    const struct AiSimCombatProfile *profile = GetProfile(context, board, target);
    bool32 chip;
    u32 amount;
    u32 strikeCount;
    u32 hpBefore;
    u32 maxHp;
    bool32 directAlly;

    *blocked = FALSE;
    if (IsPsychicTerrainPriorityBlocked(context, board, action, target))
        return FALSE;
    *blocked = IsMoveBlocked(context, board, action, target, &chip);
    if (*blocked)
    {
        result->events |= AI_SIM_EVENT_PROTECTED;
        result->readInteractionFlags[target] |= AI_READ_INTERACTION_PROTECT;
        if (action->choice == MOVE_FAKE_OUT)
            result->readInteractionFlags[target] |= AI_READ_INTERACTION_FAKE_OUT;
        return FALSE;
    }
    if (!ApplySpectralThiefPreDamage(context, board, action, target, result))
        return FALSE;
    if (!AiSim_CalcDamage(context, board, action, target, &damage))
        return FALSE;

    amount = SelectDamageRoll(context, board, action, target, outcome, &damage);
    // Type immunity remains zero damage through Protect; quartering a zero
    // Z/Max roll must not manufacture one point of chip.
    if (amount == 0)
        return FALSE;
    if (chip)
    {
        amount /= 4;
        amount = max(1, amount);
        result->events |= AI_SIM_EVENT_PROTECTED;
        result->readInteractionFlags[target] |= AI_READ_INTERACTION_PROTECT;
    }
    hpBefore = party->hp;
    maxHp = GetCurrentMaxHp(context, board, target);
    strikeCount = max(1, gMovesInfo[action->choice].strikeCount);
    if (strikeCount == 1 && amount >= hpBefore && hpBefore == maxHp)
    {
        if (party->item != ITEM_NONE
         && gItemsInfo[party->item].holdEffect == HOLD_EFFECT_FOCUS_SASH)
        {
            amount = hpBefore - 1;
            party->item = ITEM_NONE;
            party->flags |= AI_SIM_PARTY_ITEM_CONSUMED;
            result->events |= AI_SIM_EVENT_FOCUS_SASH | AI_SIM_EVENT_ITEM_CONSUMED;
        }
        else if (profile->ability == ABILITY_STURDY)
        {
            amount = hpBefore - 1;
            result->events |= AI_SIM_EVENT_STURDY;
        }
    }

    amount = min(amount, hpBefore);
    party->hp -= amount;
    result->damage[action->actor] = min(0xFFFF, result->damage[action->actor] + amount);
    result->events |= AI_SIM_EVENT_DAMAGE;
    if (party->hp == 0)
    {
        board->active[target].flags |= AI_SIM_ACTIVE_NEEDS_REPLACEMENT;
        result->events |= AI_SIM_EVENT_KO;
    }

    directAlly = ((action->actor ^ target) & BIT_SIDE) == 0;
    if (party->hp != 0 && party->item != ITEM_NONE
     && gItemsInfo[party->item].holdEffect == HOLD_EFFECT_WEAKNESS_POLICY
     && GetSimTypeModifier(context, board, action, target) > UQ_4_12(1.0)
     && (!directAlly || action->allyInteractionKind == AI_ALLY_INTERACTION_ITEM_TRIGGER))
    {
        party->item = ITEM_NONE;
        party->flags |= AI_SIM_PARTY_ITEM_CONSUMED;
        RaiseStage(board, target, STAT_ATK, 2);
        RaiseStage(board, target, STAT_SPATK, 2);
        result->events |= AI_SIM_EVENT_WEAKNESS_POLICY | AI_SIM_EVENT_ITEM_CONSUMED | AI_SIM_EVENT_STAT_CHANGE;
    }
    return TRUE;
}

static void ApplyMoonblastSecondary(struct AiSimBoard *board,
                                    const struct AiSimAction *action,
                                    enum BattlerId target,
                                    const struct AiSimOutcomeKey *outcome,
                                    struct AiSimTurnResult *result)
{
    if (!HasSupportedMoonblastSecondary(action->choice)
     || !(outcome->flags & (1u << action->actor))
     || !IsSimBattlerAlive(board, target))
        return;
    if (((action->actor ^ target) & BIT_SIDE) != 0
     && (board->sides[target & BIT_SIDE].statuses & SIDE_STATUS_MIST))
        return;
    if (LowerStage(board, target, STAT_SPATK, 1))
        result->events |= AI_SIM_EVENT_STAT_CHANGE;
}

static bool32 CanApplyIceBeamStatus(const struct AiSimContext *context,
                                    const struct AiSimBoard *board,
                                    enum BattlerId target)
{
    const struct AiSimPartyState *party = GetConstPartyState(board, target);

    if (party == NULL || party->status1 & STATUS1_ANY)
        return FALSE;
    if (IsSimBattlerOfType(context, board, target, TYPE_ICE))
        return FALSE;
    if (board->weather & B_WEATHER_SUN)
        return FALSE;
    if (board->sides[target & BIT_SIDE].statuses & SIDE_STATUS_SAFEGUARD)
        return FALSE;
    return TRUE;
}

static void ApplyIceBeamSecondary(const struct AiSimContext *context,
                                  struct AiSimBoard *board,
                                  const struct AiSimAction *action,
                                  enum BattlerId target,
                                  const struct AiSimOutcomeKey *outcome,
                                  struct AiSimTurnResult *result)
{
    struct AiSimPartyState *party;

    if (!HasSupportedIceBeamSecondary(action->choice)
     || !(outcome->flags & (1u << action->actor))
     || !IsSimBattlerAlive(board, target)
     || !CanApplyIceBeamStatus(context, board, target))
        return;
    party = GetPartyState(board, target);
#if B_USE_FROSTBITE == TRUE
    party->status1 |= STATUS1_FROSTBITE;
#else
    party->status1 |= STATUS1_FREEZE;
    // Applying Freeze cancels a pending two-turn move in the live engine.
    board->active[target].chargingMove = MOVE_NONE;
    board->active[target].volatileFlags &= ~AI_SIM_VOLATILE_GEOMANCY_CHARGING;
#endif
    result->events |= AI_SIM_EVENT_STATUS_CHANGE;
}

static bool32 CanApplyFlareBlitzBurn(const struct AiSimContext *context,
                                     const struct AiSimBoard *board,
                                     enum BattlerId target)
{
    const struct AiSimPartyState *party = GetConstPartyState(board, target);

    if (party == NULL || party->status1 & STATUS1_ANY)
        return FALSE;
    if (IsSimBattlerOfType(context, board, target, TYPE_FIRE))
        return FALSE;
    if (board->sides[target & BIT_SIDE].statuses & SIDE_STATUS_SAFEGUARD)
        return FALSE;
    return TRUE;
}

static void ApplyFlareBlitzSecondary(const struct AiSimContext *context,
                                     struct AiSimBoard *board,
                                     const struct AiSimAction *action,
                                     enum BattlerId target,
                                     const struct AiSimOutcomeKey *outcome,
                                     struct AiSimTurnResult *result)
{
    struct AiSimPartyState *party;

    if (!HasSupportedFlareBlitzSecondary(action->choice)
     || !(outcome->flags & (1u << action->actor))
     || !IsSimBattlerAlive(board, target)
     || !CanApplyFlareBlitzBurn(context, board, target))
        return;
    party = GetPartyState(board, target);
    party->status1 |= STATUS1_BURN;
    result->events |= AI_SIM_EVENT_STATUS_CHANGE;
}

static void TryConsumeSitrusBerry(const struct AiSimContext *context,
                                  struct AiSimBoard *board,
                                  enum BattlerId target,
                                  struct AiSimTurnResult *result)
{
    struct AiSimPartyState *party = GetPartyState(board, target);
    const struct AiSimCombatProfile *profile = GetProfile(context, board, target);
    u32 currentMaxHp;
    u32 heal;

    if (party == NULL || profile == NULL || party->hp == 0
     || party->item != ITEM_SITRUS_BERRY
     || gItemsInfo[party->item].holdEffect != HOLD_EFFECT_RESTORE_PCT_HP)
        return;
    currentMaxHp = GetCurrentMaxHp(context, board, target);
    if (currentMaxHp == 0 || party->hp > currentMaxHp / 2)
        return;

    // Sitrus checks the current (possibly Dynamaxed) HP threshold, but heals
    // from the non-Dynamax maximum just like ItemHealHp in the live battle.
    heal = profile->maxHp * gItemsInfo[party->item].holdEffectParam / 100;
    party->hp = min(currentMaxHp, party->hp + heal);
    party->item = ITEM_NONE;
    party->flags |= AI_SIM_PARTY_ITEM_CONSUMED;
    result->events |= AI_SIM_EVENT_ITEM_CONSUMED;
}

static void ApplyDamagingMove(const struct AiSimContext *context,
                              struct AiSimBoard *board,
                              const struct AiSimAction *action,
                              const struct AiSimOutcomeKey *outcome,
                              struct AiSimTurnResult *result,
                              u8 *powerHerbConsumedMask)
{
    u32 target;
    bool32 hitAny = FALSE;
    bool32 blocked;
    bool32 spread = IsActionSpread(context, board, action);
    u32 damageBefore = result->damage[action->actor];

    for (target = 0; target < MAX_BATTLERS_COUNT; target++)
    {
        struct AiSimPartyState *targetParty;
        if (!IsSimBattlerAlive(board, target)
         || (!spread && target != action->target)
         || (spread && !IsAffectedTarget(context, board, action, target)))
            continue;
        targetParty = GetPartyState(board, target);
        if (ApplyDamageToTarget(context, board, action, target, outcome, result, &blocked))
        {
            hitAny = TRUE;
            if (action->choice == MOVE_FAKE_OUT)
            {
                result->readInteractionFlags[target] |= AI_READ_INTERACTION_FAKE_OUT;
                if (!(board->active[target].flags & AI_SIM_ACTIVE_DYNAMAX) && targetParty->hp != 0)
                {
                    board->active[target].volatileFlags |= AI_SIM_VOLATILE_FLINCHED;
                    result->events |= AI_SIM_EVENT_FLINCH;
                }
            }

            if (action->choice == MOVE_KNOCK_OFF
             && AiSim_IsItemRemovable(context, board, target))
            {
                targetParty->item = ITEM_NONE;
                targetParty->flags |= AI_SIM_PARTY_ITEM_REMOVED;
                result->events |= AI_SIM_EVENT_ITEM_REMOVED;
            }
            else if (action->choice == MOVE_KNOCK_OFF && (*powerHerbConsumedMask & (1u << target)))
            {
                result->readInteractionFlags[target] |= AI_READ_INTERACTION_SETUP_ITEM_SEQUENCE;
            }

            ApplyMoonblastSecondary(board, action, target, outcome, result);
            ApplyIceBeamSecondary(context, board, action, target, outcome, result);
            ApplyFlareBlitzSecondary(context, board, action, target, outcome, result);
            // Additional effects run before live move-end thawing. A frozen
            // target therefore cannot be burned by the same Flare Blitz that
            // subsequently thaws it.
            TryThawIcyTarget(board, action, target, result);
            TryConsumeSitrusBerry(context, board, target, result);
        }
    }

    if (hitAny)
    {
        struct AiSimPartyState *actorParty = GetPartyState(board, action->actor);

        if (HasSupportedCloseCombatSecondary(action->choice))
        {
            bool32 changed = LowerStage(board, action->actor, STAT_DEF, 1);

            changed |= LowerStage(board, action->actor, STAT_SPDEF, 1);
            if (changed)
                result->events |= AI_SIM_EVENT_STAT_CHANGE;
        }
        if (gMovesInfo[action->choice].effect == EFFECT_RECOIL)
        {
            u32 damageDealt = result->damage[action->actor] - damageBefore;
            u32 recoil = max(1, damageDealt * gMovesInfo[action->choice].argument.recoilPercentage / 100);

            recoil = min(recoil, actorParty->hp);
            actorParty->hp -= recoil;
            result->events |= AI_SIM_EVENT_RECOIL;
            if (actorParty->hp == 0)
            {
                board->active[action->actor].flags |= AI_SIM_ACTIVE_NEEDS_REPLACEMENT;
                result->events |= AI_SIM_EVENT_KO;
            }
            else
            {
                TryConsumeSitrusBerry(context, board, action->actor, result);
            }
        }
        if (actorParty->hp != 0
         && actorParty->item != ITEM_NONE
         && gItemsInfo[actorParty->item].holdEffect == HOLD_EFFECT_LIFE_ORB)
        {
            u32 recoil = max(1, GetCurrentMaxHp(context, board, action->actor) / 10);
            recoil = min(recoil, actorParty->hp);
            actorParty->hp -= recoil;
            if (actorParty->hp == 0)
            {
                board->active[action->actor].flags |= AI_SIM_ACTIVE_NEEDS_REPLACEMENT;
                result->events |= AI_SIM_EVENT_KO;
            }
        }
    }
}

static bool32 IsLastPendingMoveAction(const struct AiSimBoard *board,
                                      const struct AiSimJointTurn *turn,
                                      u32 remainingActionMask)
{
    enum BattlerId battler;

    for (battler = 0; battler < MAX_BATTLERS_COUNT; battler++)
    {
        if ((remainingActionMask & (1u << battler))
         && turn->actions[battler].kind == AI_SIM_ACTION_MOVE
         && IsSimBattlerAlive(board, battler))
            return FALSE;
    }
    return TRUE;
}

static void ApplyProtect(struct AiSimBoard *board,
                         const struct AiSimAction *action,
                         const struct AiSimOutcomeKey *outcome,
                         bool32 isLastMoveAction,
                         struct AiSimTurnResult *result)
{
    u32 method = gMovesInfo[action->choice].argument.protectMethod;
    bool32 success = !isLastMoveAction
                  && (outcome->protectSuccessMask & (1u << action->actor));

    if (!success)
        return;
    result->protectSuccessMask |= 1u << action->actor;
    if (method == PROTECT_QUICK_GUARD)
        board->sides[action->actor & BIT_SIDE].flags |= AI_SIM_SIDE_QUICK_GUARD;
    else if (method == PROTECT_WIDE_GUARD)
        board->sides[action->actor & BIT_SIDE].flags |= AI_SIM_SIDE_WIDE_GUARD;
    else if (method == PROTECT_MAX_GUARD)
        board->active[action->actor].volatileFlags |= AI_SIM_VOLATILE_MAX_GUARD;
    else
        board->active[action->actor].volatileFlags |= AI_SIM_VOLATILE_PROTECTED;
}

static void ApplyGeomancy(struct AiSimBoard *board,
                          const struct AiSimAction *action,
                          struct AiSimTurnResult *result,
                          u8 *powerHerbConsumedMask)
{
    struct AiSimPartyState *party = GetPartyState(board, action->actor);
    struct AiSimActiveState *active = &board->active[action->actor];
    bool32 completes = active->chargingMove == MOVE_GEOMANCY
        || (active->volatileFlags & AI_SIM_VOLATILE_GEOMANCY_CHARGING);

    if (!completes && party->item != ITEM_NONE
     && gItemsInfo[party->item].holdEffect == HOLD_EFFECT_POWER_HERB)
    {
        party->item = ITEM_NONE;
        party->flags |= AI_SIM_PARTY_ITEM_CONSUMED;
        *powerHerbConsumedMask |= 1u << action->actor;
        result->events |= AI_SIM_EVENT_ITEM_CONSUMED;
        completes = TRUE;
    }
    if (!completes)
    {
        active->chargingMove = MOVE_GEOMANCY;
        active->volatileFlags |= AI_SIM_VOLATILE_GEOMANCY_CHARGING;
        return;
    }

    active->chargingMove = MOVE_NONE;
    active->volatileFlags &= ~AI_SIM_VOLATILE_GEOMANCY_CHARGING;
    RaiseStage(board, action->actor, STAT_SPATK, 2);
    RaiseStage(board, action->actor, STAT_SPDEF, 2);
    RaiseStage(board, action->actor, STAT_SPEED, 2);
    result->events |= AI_SIM_EVENT_STAT_CHANGE;
}

static void ApplyPartingShot(const struct AiSimContext *context,
                             struct AiSimBoard *board,
                             const struct AiSimAction *action,
                             struct AiSimTurnResult *result)
{
    bool32 chip;

    if (IsMoveBlocked(context, board, action, action->target, &chip))
    {
        result->events |= AI_SIM_EVENT_PROTECTED;
        result->readInteractionFlags[action->target] |= AI_READ_INTERACTION_PROTECT;
        return;
    }
    if (IsPsychicTerrainPriorityBlocked(context, board, action, action->target)
     || IsPranksterDarkTypeBlocked(context, board, action, action->target))
        return;
    if (((action->actor ^ action->target) & BIT_SIDE) != 0
     && (board->sides[action->target & BIT_SIDE].statuses & SIDE_STATUS_MIST))
        return;
    if (LowerStage(board, action->target, STAT_ATK, 1)
      | LowerStage(board, action->target, STAT_SPATK, 1))
        result->events |= AI_SIM_EVENT_STAT_CHANGE;
}

static void ApplyStatusMove(const struct AiSimContext *context,
                            struct AiSimBoard *board,
                            const struct AiSimAction *action,
                            const struct AiSimOutcomeKey *outcome,
                            bool32 isLastMoveAction,
                            struct AiSimTurnResult *result,
                            u8 *powerHerbConsumedMask)
{
    u32 side = action->actor & BIT_SIDE;

    switch (gMovesInfo[action->choice].effect)
    {
    case EFFECT_PROTECT:
        ApplyProtect(board, action, outcome, isLastMoveAction, result);
        break;
    case EFFECT_TAILWIND:
        if (!(board->sides[side].statuses & SIDE_STATUS_TAILWIND))
        {
            board->sides[side].statuses |= SIDE_STATUS_TAILWIND;
            board->sides[side].tailwindTimer = GetConfig(B_TAILWIND_TURNS) >= GEN_5 ? 4 : 3;
            result->events |= AI_SIM_EVENT_FIELD_CHANGE;
        }
        break;
    case EFFECT_TRICK_ROOM:
        if (board->fieldStatuses & STATUS_FIELD_TRICK_ROOM)
        {
            board->fieldStatuses &= ~STATUS_FIELD_TRICK_ROOM;
            board->trickRoomTimer = 0;
        }
        else
        {
            board->fieldStatuses |= STATUS_FIELD_TRICK_ROOM;
            board->trickRoomTimer = AI_SIM_ROOM_TURNS;
        }
        result->events |= AI_SIM_EVENT_FIELD_CHANGE;
        break;
    case EFFECT_GEOMANCY:
        ApplyGeomancy(board, action, result, powerHerbConsumedMask);
        break;
    case EFFECT_PARTING_SHOT:
        ApplyPartingShot(context, board, action, result);
        break;
    case EFFECT_REFLECT:
        board->sides[side].statuses |= SIDE_STATUS_REFLECT;
        board->sides[side].reflectTimer = AI_SIM_SCREEN_TURNS;
        result->events |= AI_SIM_EVENT_FIELD_CHANGE;
        break;
    case EFFECT_LIGHT_SCREEN:
        board->sides[side].statuses |= SIDE_STATUS_LIGHTSCREEN;
        board->sides[side].lightScreenTimer = AI_SIM_SCREEN_TURNS;
        result->events |= AI_SIM_EVENT_FIELD_CHANGE;
        break;
    case EFFECT_AURORA_VEIL:
        if (board->weather & B_WEATHER_ICY_ANY)
        {
            board->sides[side].statuses |= SIDE_STATUS_AURORA_VEIL;
            board->sides[side].auroraVeilTimer = AI_SIM_SCREEN_TURNS;
            result->events |= AI_SIM_EVENT_FIELD_CHANGE;
        }
        break;
    default:
        break;
    }
}

static void ResetActiveForSwitch(struct AiSimBoard *board,
                                 enum BattlerId battler,
                                 u32 rosterIndex)
{
    struct AiSimActiveState *active = &board->active[battler];
    u32 stat;
    u32 persistentGimmick = board->party[rosterIndex].activeGimmick;

    *active = (struct AiSimActiveState){0};
    active->rosterIndex = rosterIndex;
    active->firstTurn = TRUE;
    active->choiceMoveSlot = AI_SIM_MOVE_SLOT_NONE;
    for (stat = 0; stat < NUM_BATTLE_STATS; stat++)
        active->statStages[stat] = DEFAULT_STAT_STAGE;
    if (persistentGimmick == GIMMICK_MEGA || persistentGimmick == GIMMICK_ULTRA_BURST)
        active->flags |= AI_SIM_ACTIVE_TRANSFORMED;
    else if (persistentGimmick == GIMMICK_TERA)
        active->flags |= AI_SIM_ACTIVE_TERA;
}

static void ApplyReplacement(const struct AiSimContext *context,
                             struct AiSimBoard *board,
                             enum BattlerId actor,
                             u32 replacement,
                             struct AiSimTurnResult *result,
                             bool32 standaloneSwitch)
{
    struct AiSimPartyState *oldParty = GetPartyState(board, actor);

    if (board->active[actor].flags & AI_SIM_ACTIVE_DYNAMAX)
    {
        u32 rosterIndex = board->active[actor].rosterIndex;
        u32 percent = context->mons[rosterIndex].dynamaxHpPercent;
        oldParty->hp = (oldParty->hp * 100 + percent - 1) / percent;
        oldParty->activeGimmick = GIMMICK_NONE;
    }
    // Bad poison remains bad poison after switching, but its escalating turn
    // counter resets.
    oldParty->status1 &= ~STATUS1_TOXIC_COUNTER;
    ResetActiveForSwitch(board, actor, replacement);
    if (result != NULL)
    {
        result->events |= AI_SIM_EVENT_SWITCH;
        if (standaloneSwitch)
            result->executedMask |= 1u << actor;
    }
}

static void ActivateGimmick(const struct AiSimContext *context,
                            struct AiSimBoard *board,
                            const struct AiSimAction *action)
{
    struct AiSimPartyState *party;
    const struct AiSimMonTemplate *mon;
    u32 rosterIndex;
    u32 trainer;

    if (action->gimmick == GIMMICK_NONE)
        return;
    rosterIndex = board->active[action->actor].rosterIndex;
    party = &board->party[rosterIndex];
    mon = &context->mons[rosterIndex];
    trainer = mon->trainer;
    board->trainerGimmickUsed[trainer] |= 1u << action->gimmick;

    switch (action->gimmick)
    {
    case GIMMICK_MEGA:
    case GIMMICK_ULTRA_BURST:
        party->activeGimmick = action->gimmick;
        board->active[action->actor].flags |= AI_SIM_ACTIVE_TRANSFORMED;
        break;
    case GIMMICK_DYNAMAX:
        party->activeGimmick = GIMMICK_DYNAMAX;
        party->hp = min(0xFFFF, (party->hp * mon->dynamaxHpPercent + 50) / 100);
        board->active[action->actor].flags |= AI_SIM_ACTIVE_DYNAMAX;
        board->active[action->actor].dynamaxTurns = 3;
        break;
    case GIMMICK_TERA:
        party->activeGimmick = GIMMICK_TERA;
        board->active[action->actor].flags |= AI_SIM_ACTIVE_TERA;
        break;
    case GIMMICK_Z_MOVE:
        break;
    }
}

static bool32 IsChoiceHoldEffect(u32 holdEffect)
{
    return holdEffect == HOLD_EFFECT_CHOICE_BAND
        || holdEffect == HOLD_EFFECT_CHOICE_SCARF
        || holdEffect == HOLD_EFFECT_CHOICE_SPECS;
}

static u32 GetMovePpCost(const struct AiSimContext *context,
                         const struct AiSimBoard *board,
                         const struct AiSimAction *action)
{
    u32 target;
    u32 cost = 1;

    for (target = 0; target < MAX_BATTLERS_COUNT; target++)
    {
        const struct AiSimCombatProfile *profile;

        if (!IsSimBattlerAlive(board, target)
         || ((action->actor ^ target) & BIT_SIDE) == 0
         || !IsAffectedTarget(context, board, action, target))
            continue;
        profile = GetProfile(context, board, target);
        if (profile != NULL && profile->ability == ABILITY_PRESSURE)
            cost++;
    }
    return cost;
}

static void SpendMovePpAndApplyChoiceLock(const struct AiSimContext *context,
                                          struct AiSimBoard *board,
                                          const struct AiSimAction *action,
                                          bool32 continuingMove,
                                          struct AiSimTurnResult *result)
{
    struct AiSimPartyState *party = GetPartyState(board, action->actor);
    struct AiSimActiveState *active = &board->active[action->actor];
    u32 holdEffect;
    u32 ppCost;

    if (continuingMove)
        return;
    ppCost = GetMovePpCost(context, board, action);
    party->pp[action->moveSlot] -= min(ppCost, party->pp[action->moveSlot]);
    result->events |= AI_SIM_EVENT_PP_SPENT;

    if (party->item == ITEM_NONE || active->flags & AI_SIM_ACTIVE_DYNAMAX)
        return;
    holdEffect = gItemsInfo[party->item].holdEffect;
    if (IsChoiceHoldEffect(holdEffect) && active->choiceMoveSlot == AI_SIM_MOVE_SLOT_NONE)
    {
        active->choiceMoveSlot = action->moveSlot;
        result->events |= AI_SIM_EVENT_CHOICE_LOCK;
    }
}

static void DecrementTimer(u8 *timer, u32 *statuses, u32 status)
{
    if (*timer == 0)
        return;
    (*timer)--;
    if (*timer == 0)
        *statuses &= ~status;
}

static void ApplyEndTurn(const struct AiSimContext *context,
                         struct AiSimBoard *board,
                         u8 switchedMask,
                         struct AiSimTurnResult *result)
{
    u32 side;
    u32 battler;

    for (side = 0; side < NUM_BATTLE_SIDES; side++)
    {
        DecrementTimer(&board->sides[side].tailwindTimer, &board->sides[side].statuses, SIDE_STATUS_TAILWIND);
        DecrementTimer(&board->sides[side].reflectTimer, &board->sides[side].statuses, SIDE_STATUS_REFLECT);
        DecrementTimer(&board->sides[side].lightScreenTimer, &board->sides[side].statuses, SIDE_STATUS_LIGHTSCREEN);
        DecrementTimer(&board->sides[side].auroraVeilTimer, &board->sides[side].statuses, SIDE_STATUS_AURORA_VEIL);
        DecrementTimer(&board->sides[side].safeguardTimer, &board->sides[side].statuses, SIDE_STATUS_SAFEGUARD);
        DecrementTimer(&board->sides[side].mistTimer, &board->sides[side].statuses, SIDE_STATUS_MIST);
        board->sides[side].flags &= ~(AI_SIM_SIDE_QUICK_GUARD | AI_SIM_SIDE_WIDE_GUARD);
    }

    DecrementTimer(&board->trickRoomTimer, &board->fieldStatuses, STATUS_FIELD_TRICK_ROOM);
    DecrementTimer(&board->terrainTimer, &board->fieldStatuses, STATUS_FIELD_TERRAIN_ANY);
    DecrementTimer(&board->gravityTimer, &board->fieldStatuses, STATUS_FIELD_GRAVITY);
    DecrementTimer(&board->magicRoomTimer, &board->fieldStatuses, STATUS_FIELD_MAGIC_ROOM);
    DecrementTimer(&board->wonderRoomTimer, &board->fieldStatuses, STATUS_FIELD_WONDER_ROOM);
    if (board->weatherTimer != 0)
    {
        board->weatherTimer--;
        if (board->weatherTimer == 0)
            board->weather = B_WEATHER_NONE;
    }

    for (battler = 0; battler < MAX_BATTLERS_COUNT; battler++)
    {
        struct AiSimPartyState *party;
        if (!(board->activeMask & (1u << battler)))
            continue;
        board->active[battler].volatileFlags &= ~(AI_SIM_VOLATILE_PROTECTED
                                                | AI_SIM_VOLATILE_MAX_GUARD
                                                | AI_SIM_VOLATILE_FLINCHED);
        if (!(switchedMask & (1u << battler)))
            board->active[battler].firstTurn = FALSE;
        party = GetPartyState(board, battler);
        if (party != NULL && party->hp != 0
         && (party->status1 & AI_SIM_MODELED_RESIDUAL_STATUSES))
        {
#if B_BURN_DAMAGE >= GEN_7 || B_BURN_DAMAGE == GEN_1
            u32 amount = max(1, GetCurrentMaxHp(context, board, battler) / 16);
#else
            u32 amount = max(1, GetCurrentMaxHp(context, board, battler) / 8);
#endif

            party->hp -= min(amount, party->hp);
            result->events |= AI_SIM_EVENT_DAMAGE;
            if (party->hp == 0)
            {
                board->active[battler].flags |= AI_SIM_ACTIVE_NEEDS_REPLACEMENT;
                result->events |= AI_SIM_EVENT_KO;
            }
            else
            {
                TryConsumeSitrusBerry(context, board, battler, result);
            }
        }
        if (!(board->active[battler].flags & AI_SIM_ACTIVE_DYNAMAX)
         || board->active[battler].dynamaxTurns == 0)
            continue;
        board->active[battler].dynamaxTurns--;
        if (board->active[battler].dynamaxTurns != 0)
            continue;
        party = GetPartyState(board, battler);
        {
            u32 rosterIndex = board->active[battler].rosterIndex;
            u32 percent = context->mons[rosterIndex].dynamaxHpPercent;
            party->hp = (party->hp * 100 + percent - 1) / percent;
        }
        party->activeGimmick = GIMMICK_NONE;
        board->active[battler].flags &= ~AI_SIM_ACTIVE_DYNAMAX;
    }
    board->turn++;
    (void)context;
}

static bool32 IsOutcomeValid(const struct AiSimContext *context,
                             const struct AiSimBoard *board,
                             const struct AiSimJointTurn *turn,
                             const struct AiSimOutcomeKey *outcome)
{
    u32 actor;
    u32 protectMask = 0;
    u32 secondaryMask = 0;
    u32 criticalMask = 0;
    u32 forcedCriticalMask = 0;
    u32 invariantDamageMask = 0;
    u32 thawMask;
    u32 tieOrderCount = OutcomeActionFactorial(__builtin_popcount(turn->actionMask));

    if (outcome->probabilityWeight == 0 || outcome->reserved != 0)
        return FALSE;
    if (outcome->speedTieOrder != AI_SIM_SPEED_TIE_ORDER_CONTEXT
     && outcome->speedTieOrder >= tieOrderCount)
        return FALSE;

    for (actor = 0; actor < MAX_BATTLERS_COUNT; actor++)
    {
        const struct AiSimAction *action;

        if (!(turn->actionMask & (1u << actor)))
            continue;
        action = &turn->actions[actor];
        if (action->kind != AI_SIM_ACTION_MOVE)
            continue;
        if (gMovesInfo[action->choice].power != 0
         && ActionHasInvariantCappedDamage(context, board, turn, actor))
            invariantDamageMask |= 1u << actor;
        if (GetSupportedSecondaryChance(action->choice) != 0
         && !(invariantDamageMask & (1u << actor)))
        {
            secondaryMask |= 1u << actor;
        }
        if (gMovesInfo[action->choice].power != 0
         && !(invariantDamageMask & (1u << actor)))
        {
            u32 denominator = GetCriticalHitDenominator(board, action);

            if (denominator != 0)
                criticalMask |= 1u << actor;
            if (denominator == 1)
                forcedCriticalMask |= 1u << actor;
        }
        if (!IsProtectionMove(action->choice))
            continue;
        protectMask |= 1u << actor;
        if (GetProtectActionSuccessDenominator(board, action) == 1
         && !(outcome->protectSuccessMask & (1u << actor)))
            return FALSE;
    }
    thawMask = GetPotentialThawMask(context, board, turn, invariantDamageMask);
    return (outcome->protectSuccessMask & ~protectMask) == 0
        && (outcome->flags & ~(secondaryMask | (thawMask << AI_SIM_OUTCOME_THAW_SHIFT))) == 0
        && (outcome->criticalMask & ~criticalMask) == 0
        && (outcome->criticalMask & forcedCriticalMask) == forcedCriticalMask;
}

enum AiSimApplyStatus AiSim_ApplyJointTurn(const struct AiSimContext *context,
                                           const struct AiSimBoard *before,
                                           const struct AiSimJointTurn *turn,
                                           const struct AiSimOutcomeKey *outcome,
                                           struct AiSimBoard *after,
                                           struct AiSimTurnResult *result)
{
    struct AiSimOutcomeKey defaultOutcome =
    {
        .probabilityWeight = 1,
        .speedTieOrder = AI_SIM_SPEED_TIE_ORDER_CONTEXT,
    };
    u8 powerHerbConsumedMask = 0;
    u8 switchedMask = 0;
    u8 initialActionOrder[MAX_BATTLERS_COUNT];
    u8 remainingActionMask;
    u32 unsupported = 0;
    u32 actionCount;
    u32 i;
    bool32 processedMoveAction = FALSE;
    bool32 dynamicTurnOrder;
    enum AiSimApplyStatus status;

    if (result != NULL)
        *result = (struct AiSimTurnResult){0};
    if (context == NULL || before == NULL || turn == NULL || after == NULL || result == NULL
     || before == after)
        return AI_SIM_APPLY_INVALID;

    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
    {
        if (turn->actionMask & (1u << i))
            result->rejectionFlags[i] = AiSim_GetActionRejectionFlags(context, before, &turn->actions[i]);
    }
    status = AiSim_CheckJointTurn(context, before, turn, &unsupported);
    result->unsupportedFlags = unsupported;
    if (status != AI_SIM_APPLY_OK)
        return status;

    if (outcome == NULL)
    {
        for (i = 0; i < MAX_BATTLERS_COUNT; i++)
            SetOutcomeDamageRoll(&defaultOutcome, i, AI_SIM_DAMAGE_ROLL_MEDIAN);
        for (i = 0; i < MAX_BATTLERS_COUNT; i++)
        {
            if ((turn->actionMask & (1u << i))
             && turn->actions[i].kind == AI_SIM_ACTION_MOVE
             && GetSupportedSecondaryChance(turn->actions[i].choice) != 0)
            {
                result->unsupportedFlags = AI_SIM_UNSUPPORTED_OUTCOME;
                return AI_SIM_APPLY_UNSUPPORTED;
            }
            if ((turn->actionMask & (1u << i))
             && turn->actions[i].kind == AI_SIM_ACTION_MOVE
             && IsProtectionMove(turn->actions[i].choice))
            {
                if (GetProtectActionSuccessDenominator(before, &turn->actions[i]) != 1)
                {
                    result->unsupportedFlags = AI_SIM_UNSUPPORTED_OUTCOME;
                    return AI_SIM_APPLY_UNSUPPORTED;
                }
                defaultOutcome.protectSuccessMask |= 1u << i;
            }
        }
        if (GetPotentialThawMask(context, before, turn, 0) != 0)
        {
            result->unsupportedFlags = AI_SIM_UNSUPPORTED_OUTCOME;
            return AI_SIM_APPLY_UNSUPPORTED;
        }
        outcome = &defaultOutcome;
    }
    if (!IsOutcomeValid(context, before, turn, outcome))
    {
        result->unsupportedFlags = AI_SIM_UNSUPPORTED_OUTCOME;
        return AI_SIM_APPLY_UNSUPPORTED;
    }

    // Use the output board as temporary ordering workspace before replacing
    // it with the actual post-gimmick execution board.  This keeps a complete
    // AiSimBoard off the battle-controller task's small stack.
    if (!BuildInitialActionOrder(context, before, turn, outcome, after,
                                 initialActionOrder))
        return AI_SIM_APPLY_INVALID;

    *after = *before;
    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
    {
        const struct AiSimAction *action;

        if (!(turn->actionMask & (1u << i)))
            continue;
        action = &turn->actions[i];
        if (action->kind == AI_SIM_ACTION_SWITCH)
        {
            ApplyReplacement(context, after, i, action->choice, result, TRUE);
            switchedMask |= 1u << i;
        }
        else if (HasForcedReplacement(action))
        {
            ApplyReplacement(context, after, i, action->replacementRosterIndex, result, FALSE);
        }
    }
    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
    {
        if ((turn->actionMask & (1u << i)) && turn->actions[i].kind == AI_SIM_ACTION_MOVE)
            ActivateGimmick(context, after, &turn->actions[i]);
    }

    actionCount = __builtin_popcount(turn->actionMask);
    remainingActionMask = turn->actionMask;
    dynamicTurnOrder = GetConfig(B_RECALC_TURN_AFTER_ACTIONS) >= GEN_8;
    result->actionCount = actionCount;
    for (i = 0; i < actionCount; i++)
    {
        enum BattlerId actor = MAX_BATTLERS_COUNT;
        const struct AiSimAction *action;
        struct AiSimActiveState *active;
        struct AiSimPartyState *party;

        // Pre-Generation-8 battles consume the initially sorted order for the
        // whole turn.  Dynamic battles keep that order through the switch
        // bracket and first move, then recalculate the remaining move actions
        // after each completed move.  This also preserves the pre-form first
        // move order selected by B_MEGA_EVO_TURN_ORDER before Generation 7.
        if (!dynamicTurnOrder || !processedMoveAction)
        {
            actor = initialActionOrder[i];
        }
        else
        {
            enum BattlerId candidate;

            for (candidate = 0; candidate < MAX_BATTLERS_COUNT; candidate++)
            {
                if (!(remainingActionMask & (1u << candidate)))
                    continue;
                if (actor == MAX_BATTLERS_COUNT
                 || ActionComesBefore(context, after, turn, outcome,
                                      candidate, actor))
                    actor = candidate;
            }
        }
        if (actor >= MAX_BATTLERS_COUNT
         || !(remainingActionMask & (1u << actor)))
            return AI_SIM_APPLY_INVALID;
        remainingActionMask &= ~(1u << actor);
        result->actionOrder[i] = actor;
        action = &turn->actions[actor];
        active = &after->active[actor];
        party = GetPartyState(after, actor);

        result->resolvedTargets[actor] = action->target;
        if (action->kind == AI_SIM_ACTION_SWITCH)
            continue;
        processedMoveAction = TRUE;
        if (!IsSimBattlerAlive(after, actor))
        {
            result->skippedMask |= 1u << actor;
            active->consecutiveMoveUses = 0;
            continue;
        }
        if (party->status1 & STATUS1_ICY_ANY)
        {
            if (MoveThawsUser(action->choice))
            {
                party->status1 &= ~STATUS1_ICY_ANY;
                result->events |= AI_SIM_EVENT_STATUS_CHANGE;
            }
            else if (party->status1 & STATUS1_FREEZE)
            {
                if (outcome->flags & (1u << (actor + AI_SIM_OUTCOME_THAW_SHIFT)))
                {
                    party->status1 &= ~STATUS1_FREEZE;
                    result->events |= AI_SIM_EVENT_STATUS_CHANGE;
                }
                else
                {
                    result->skippedMask |= 1u << actor;
                    active->consecutiveMoveUses = 0;
                    continue;
                }
            }
        }
        if (active->volatileFlags & AI_SIM_VOLATILE_FLINCHED)
        {
            result->skippedMask |= 1u << actor;
            active->consecutiveMoveUses = 0;
            continue;
        }

        result->executedMask |= 1u << actor;
        {
            bool32 continuingMove = IsContinuingGeomancy(after, action);
            SpendMovePpAndApplyChoiceLock(context, after, action, continuingMove, result);
        }
        if (gMovesInfo[action->choice].power != 0)
        {
            if (action->choice == MOVE_FAKE_OUT && !active->firstTurn)
                continue;
            ApplyDamagingMove(context, after, action, outcome, result, &powerHerbConsumedMask);
        }
        else
        {
            ApplyStatusMove(context, after, action, outcome,
                            IsLastPendingMoveAction(after, turn, remainingActionMask),
                            result, &powerHerbConsumedMask);
        }

        if (IsProtectionMove(action->choice))
        {
            if (!(result->protectSuccessMask & (1u << actor)))
                active->consecutiveMoveUses = 0;
            else
                active->consecutiveMoveUses = min(3, active->consecutiveMoveUses + 1);
        }
        else
        {
            active->consecutiveMoveUses = 0;
        }
        active->lastMove = action->choice;
    }

    ApplyEndTurn(context, after, switchedMask, result);
    return AI_SIM_APPLY_OK;
}
