#include "global.h"
#include "battle.h"
#include "battle_ai_board_sim.h"
#include "battle_ai_joint_planner.h"
#include "battle_ai_joint_runtime.h"
#include "battle_ai_main.h"
#include "battle_ai_util.h"
#include "battle_controllers.h"
#include "battle_dynamax.h"
#include "battle_gimmick.h"
#include "battle_util.h"
#include "battle_z_move.h"
#include "item.h"
#include "main.h"
#include "malloc.h"
#include "move.h"
#include "pokemon.h"
#include "constants/battle_ai.h"
#include "constants/battle_move_effects.h"
#include "constants/hold_effects.h"
#include "constants/items.h"
#include "constants/moves.h"

#define AI_JOINT_RUNTIME_REQUIRED_FLAGS (AI_FLAG_READ_PLAYER_MOVE     \
                                       | AI_FLAG_DOUBLE_BATTLE       \
                                       | AI_FLAG_OMNISCIENT          \
                                       | AI_FLAG_SMART_MON_CHOICES)

#define AI_JOINT_RUNTIME_UNSUPPORTED_TYPES_BASE (BATTLE_TYPE_LINK     \
                                           | BATTLE_TYPE_RECORDED_LINK\
                                           | BATTLE_TYPE_PALACE        \
                                           | BATTLE_TYPE_SAFARI        \
                                           | BATTLE_TYPE_ROAMER        \
                                           | BATTLE_TYPE_FIRST_BATTLE  \
                                           | BATTLE_TYPE_MULTI         \
                                           | BATTLE_TYPE_TWO_OPPONENTS \
                                           | BATTLE_TYPE_INGAME_PARTNER)

// AI battle tests use BATTLE_TYPE_RECORDED to make RNG reproducible while
// still exercising the live opponent controller.  Real recorded playback
// remains outside the runtime planner envelope.
#if TESTING
#define AI_JOINT_RUNTIME_UNSUPPORTED_TYPES AI_JOINT_RUNTIME_UNSUPPORTED_TYPES_BASE
#else
#define AI_JOINT_RUNTIME_UNSUPPORTED_TYPES (AI_JOINT_RUNTIME_UNSUPPORTED_TYPES_BASE \
                                           | BATTLE_TYPE_RECORDED)
#endif

enum AiJointRuntimeInternalState
{
    AI_JOINT_RUNTIME_STATE_EMPTY,
    AI_JOINT_RUNTIME_STATE_SEARCHING,
    AI_JOINT_RUNTIME_STATE_READY,
    AI_JOINT_RUNTIME_STATE_FALLBACK,
};

struct AiJointRuntimeData
{
    struct AiSimContext context;
    struct AiSimBoard rootBoard;
    struct AiJointAtomicCandidate rootCandidates[2][AI_JOINT_MAX_ATOMIC_INPUT];
    struct AiJointAtomicCandidate retainedRoots[2][AI_JOINT_MAX_ATOMIC_PER_ACTOR];
    struct AiJointPlannerWorkspace workspace;
    struct AiJointPlannerJob job;
    struct AiJointPlannerResult result;
    struct AiJointPlannerLimits limitOverride;
    struct AiSimAction legacyActions[2];
    u32 commandSignature;
    u32 lastStepFrame;
    u32 searchStartFrame;
    u32 buildCount;
    u32 stepCount;
    s16 rootPressure;
    u16 planId;
    u16 turn;
    u8 actors[2];
    u8 rootCounts[2];
    u8 legacyValidMask;
    u8 appliedMask;
    u8 reusedMoveMask;
    u8 state;
    u8 hasLimitOverride;
    u8 steppedThisFrame;
    u8 traceChosenRank;
    u8 pairGlobalsPrepared;
    u8 rootGenerationIncomplete;
    u8 delayRecorded;
};

STATIC_ASSERT(sizeof(struct AiJointRuntimeData) <= 16 * 1024,
              AiJointRuntimeDataExceededEwramBudget)

// The search arena is needed only by ordinary smart 2v2 trainer battles.  It
// lives in the existing general-purpose EWRAM heap instead of permanently
// shrinking every battle/test configuration's free EWRAM by roughly 15 KiB.
// Allocation failure is a normal fail-closed path to the legacy evaluator.
static EWRAM_DATA struct AiJointRuntimeData *sAiJointRuntime = NULL;

#if TESTING
static EWRAM_DATA struct AiJointPlannerLimits sAiJointRuntimeTestLimits = {0};
static EWRAM_DATA bool8 sAiJointRuntimeHasTestLimits = FALSE;
static EWRAM_DATA struct AiJointAtomicCandidate
    sAiJointRuntimeTestCandidates[AI_JOINT_MAX_ATOMIC_PER_ACTOR] = {0};
static EWRAM_DATA struct
{
    u32 buildCount;
    u32 stepCount;
    u16 planId;
} sAiJointRuntimeReleasedStats = {0};
#endif

static bool32 AiJointRuntime_EnsureData(void)
{
    if (sAiJointRuntime != NULL)
        return TRUE;
    sAiJointRuntime = AllocZeroed(sizeof(*sAiJointRuntime));
    if (sAiJointRuntime == NULL)
        return FALSE;
    sAiJointRuntime->planId = BATTLE_AI_TRACE_ID_NONE;
#if TESTING
    if (sAiJointRuntimeHasTestLimits)
    {
        sAiJointRuntime->limitOverride = sAiJointRuntimeTestLimits;
        sAiJointRuntime->hasLimitOverride = TRUE;
    }
#endif
    return TRUE;
}

static u32 AiJointRuntime_HashByte(u32 hash, u8 value)
{
    hash ^= value;
    return hash * 16777619u;
}

static u32 AiJointRuntime_HashHalfword(u32 hash, u16 value)
{
    hash = AiJointRuntime_HashByte(hash, value);
    return AiJointRuntime_HashByte(hash, value >> 8);
}

static bool32 AiJointRuntime_IsAliveOnBoard(const struct AiSimBoard *board, enum BattlerId battler)
{
    u32 rosterIndex;

    if (battler >= MAX_BATTLERS_COUNT
     || !(board->activeMask & (1u << battler))
     || (board->absentMask & (1u << battler)))
        return FALSE;
    rosterIndex = board->active[battler].rosterIndex;
    return rosterIndex < AI_SIM_ROSTER_COUNT && board->party[rosterIndex].hp != 0;
}

static u32 AiJointRuntime_GetPlayerCommandSignature(bool32 *commandsReady,
                                                    bool32 *knownUnsupported)
{
    u32 hash = 2166136261u;
    u32 commandCount = 0;

    *commandsReady = FALSE;
    *knownUnsupported = FALSE;
    hash = AiJointRuntime_HashHalfword(hash, gBattleTurnCounter);
    for (enum BattlerId battler = 0; battler < gBattlersCount; battler++)
    {
        if (!IsOnPlayerSide(battler)
         || (gAbsentBattlerFlags & (1u << battler))
         || !IsBattlerAlive(battler)
         || gBattleStruct->battlerState[battler].commandingDondozo)
            continue;

        hash = AiJointRuntime_HashByte(hash, battler);
        hash = AiJointRuntime_HashByte(hash, gChosenActionByBattler[battler]);
        if (gBattleMons[battler].volatiles.multipleTurns
         || gBattleMons[battler].volatiles.rechargeTimer > 0)
        {
            *knownUnsupported = TRUE;
            return hash;
        }
        switch (gChosenActionByBattler[battler])
        {
        case B_ACTION_USE_MOVE:
            if (gChosenMoveByBattler[battler] == MOVE_NONE
             || gChosenMoveByBattler[battler] == MOVE_UNAVAILABLE
             || gBattleStruct->chosenMovePositions[battler] >= MAX_MON_MOVES
             || gBattleStruct->moveTarget[battler] >= gBattlersCount)
                return hash;
            hash = AiJointRuntime_HashHalfword(hash, gChosenMoveByBattler[battler]);
            hash = AiJointRuntime_HashByte(hash, gBattleStruct->chosenMovePositions[battler]);
            hash = AiJointRuntime_HashByte(hash, gBattleStruct->moveTarget[battler]);
            hash = AiJointRuntime_HashByte(hash, (gBattleStruct->gimmick.toActivate >> battler) & 1);
            break;
        case B_ACTION_SWITCH:
            if (gBattleStruct->monToSwitchIntoId[battler] >= PARTY_SIZE)
                return hash;
            hash = AiJointRuntime_HashByte(hash, gBattleStruct->monToSwitchIntoId[battler]);
            break;
        default:
            // The immutable simulator intentionally has no trainer-item/run
            // action.  A known but unsupported command must take the repaired
            // legacy path rather than being silently approximated.
            if (gChosenActionByBattler[battler] != B_ACTION_NONE)
                *knownUnsupported = TRUE;
            return hash;
        }
        commandCount++;
    }

    // This initial integration is deliberately restricted to an ordinary 2v2
    // board.  Replacement and asymmetric boards remain on the legacy path.
    *commandsReady = commandCount == 2;
    return hash;
}

static bool32 AiJointRuntime_IsEligible(enum BattlerId battler)
{
    enum BattlerId actors[2];
    u32 count = 0;
    u32 commandablePlayerCount = 0;

    if (battler >= gBattlersCount
     || GetBattlerSide(battler) != B_SIDE_OPPONENT
     || !IsDoubleBattle()
     || (gBattleTypeFlags & AI_JOINT_RUNTIME_UNSUPPORTED_TYPES)
     || gBattleHistory == NULL
     || gBattleHistory->itemsNo != 0)
        return FALSE;

    for (enum BattlerId actor = 0; actor < gBattlersCount; actor++)
    {
        if (IsOnPlayerSide(actor)
         && IsBattlerAlive(actor)
         && !(gAbsentBattlerFlags & (1u << actor))
         && !gBattleStruct->battlerState[actor].commandingDondozo)
            commandablePlayerCount++;
    }
    if (commandablePlayerCount != 2)
        return FALSE;

    for (enum BattlerId actor = 0; actor < gBattlersCount; actor++)
    {
        u64 flags;

        if (GetBattlerSide(actor) != GetBattlerSide(battler))
            continue;
        if (!BattlerHasAi(actor)
         || !IsBattlerAlive(actor)
         || (gAbsentBattlerFlags & (1u << actor))
         || gBattleStruct->battlerState[actor].commandingDondozo
         || count >= ARRAY_COUNT(actors))
            return FALSE;
        flags = gAiThinkingStruct->aiFlags[actor];
        if ((flags & AI_JOINT_RUNTIME_REQUIRED_FLAGS) != AI_JOINT_RUNTIME_REQUIRED_FLAGS
         || (flags & AI_FLAG_ATTACKS_PARTNER))
            return FALSE;
        actors[count++] = actor;
    }

    return count == 2;
}

static u8 AiJointRuntime_GetCandidateFamily(enum Move move)
{
    u32 effect = GetMoveEffect(move);

    switch (effect)
    {
    case EFFECT_PROTECT:
        return AI_JOINT_FAMILY_PROTECT;
    case EFFECT_TAILWIND:
    case EFFECT_TRICK_ROOM:
        return AI_JOINT_FAMILY_SPEED_FIELD;
    case EFFECT_GEOMANCY:
        return AI_JOINT_FAMILY_SETUP;
    case EFFECT_REFLECT:
    case EFFECT_LIGHT_SCREEN:
    case EFFECT_AURORA_VEIL:
        return AI_JOINT_FAMILY_SUPPORT;
    case EFFECT_FIRST_TURN_ONLY:
    case EFFECT_KNOCK_OFF:
        return AI_JOINT_FAMILY_DISRUPTION;
    default:
        if (GetMovePower(move) != 0)
            return AI_JOINT_FAMILY_CLEAN_DAMAGE;
        return AI_JOINT_FAMILY_OTHER;
    }
}

static u16 AiJointRuntime_GetResourceFlags(enum Gimmick gimmick)
{
    switch (gimmick)
    {
    case GIMMICK_MEGA:
        return AI_JOINT_RESOURCE_MEGA;
    case GIMMICK_ULTRA_BURST:
        return AI_JOINT_RESOURCE_ULTRA;
    case GIMMICK_Z_MOVE:
        return AI_JOINT_RESOURCE_Z_MOVE;
    case GIMMICK_DYNAMAX:
        return AI_JOINT_RESOURCE_DYNAMAX;
    case GIMMICK_TERA:
        return AI_JOINT_RESOURCE_TERA;
    default:
        return AI_JOINT_RESOURCE_NONE;
    }
}

static enum Move AiJointRuntime_GetBoardZMove(const struct AiSimContext *context,
                                              const struct AiSimBoard *board,
                                              enum BattlerId actor,
                                              enum Move baseMove)
{
    const struct AiSimMonTemplate *mon;
    const struct AiSimCombatProfile *profile;
    enum Item item;
    enum Move zMove;
    u32 rosterIndex;

    if (context == NULL || board == NULL || actor >= context->battlersCount
     || baseMove == MOVE_NONE || baseMove == MOVE_UNAVAILABLE)
        return MOVE_NONE;
    rosterIndex = board->active[actor].rosterIndex;
    if (rosterIndex >= AI_SIM_ROSTER_COUNT)
        return MOVE_NONE;
    mon = &context->mons[rosterIndex];
    profile = &mon->normal;
    if (board->active[actor].flags & AI_SIM_ACTIVE_TRANSFORMED)
        profile = &mon->transformed;
    item = board->party[rosterIndex].item;
    if (item >= ITEMS_COUNT || GetItemHoldEffect(item) != HOLD_EFFECT_Z_CRYSTAL)
        return MOVE_NONE;
    zMove = GetSignatureZMove(baseMove, profile->species, item);
    if (zMove != MOVE_NONE)
        return zMove;
    if (GetMoveType(baseMove) != GetItemSecondaryId(item))
        return MOVE_NONE;
    if (GetMoveCategory(baseMove) == DAMAGE_CATEGORY_STATUS)
        return MOVE_Z_STATUS;
    return GetTypeBasedZMove(baseMove);
}

static enum Move AiJointRuntime_GetEffectiveMove(const struct AiSimContext *context,
                                                 const struct AiSimBoard *board,
                                                 enum BattlerId actor,
                                                 enum Move baseMove,
                                                 enum Gimmick gimmick)
{
    if (gimmick == GIMMICK_Z_MOVE)
    {
        enum Move zMove = AiJointRuntime_GetBoardZMove(context, board, actor, baseMove);

        return zMove == MOVE_Z_STATUS ? baseMove : zMove;
    }
    if (gimmick == GIMMICK_DYNAMAX)
        return MOVE_NONE;
    return baseMove;
}

static enum Move AiJointRuntime_GetEffectiveRootMove(enum BattlerId actor,
                                                     enum Move baseMove,
                                                     enum Gimmick gimmick)
{
    if (gimmick == GIMMICK_DYNAMAX
     || (gimmick == GIMMICK_NONE && GetActiveGimmick(actor) == GIMMICK_DYNAMAX))
        return GetMaxMove(actor, baseMove);
    return AiJointRuntime_GetEffectiveMove(&sAiJointRuntime->context,
                                           &sAiJointRuntime->rootBoard,
                                           actor, baseMove, gimmick);
}

static enum BattlerId AiJointRuntime_NormalizeRootTarget(enum BattlerId actor,
                                                         enum BattlerId scoreTarget,
                                                         enum Move move)
{
    switch (AI_GetBattlerMoveTargetType(actor, move))
    {
    case TARGET_USER:
    case TARGET_USER_AND_ALLY:
    case TARGET_FIELD:
    case TARGET_OPPONENTS_FIELD:
    case TARGET_ALL_BATTLERS:
        return actor;
    case TARGET_ALLY:
        return BATTLE_PARTNER(actor);
    case TARGET_USER_OR_ALLY:
        return scoreTarget;
    case TARGET_BOTH:
    case TARGET_FOES_AND_ALLY:
        return IsBattlerAlive(LEFT_FOE(actor)) ? LEFT_FOE(actor) : RIGHT_FOE(actor);
    default:
        return scoreTarget;
    }
}

static void AiJointRuntime_InitMoveCandidate(struct AiJointAtomicCandidate *candidate,
                                             enum BattlerId actor,
                                             enum BattlerId target,
                                             enum Move move,
                                             u32 moveSlot,
                                             enum Gimmick gimmick,
                                             s32 prior,
                                             u8 readFlags,
                                             u8 allyInteractionKind)
{
    memset(candidate, 0, sizeof(*candidate));
    candidate->action.actor = actor;
    candidate->action.target = target;
    candidate->action.kind = AI_SIM_ACTION_MOVE;
    candidate->action.choice = move;
    candidate->action.moveSlot = moveSlot;
    candidate->action.gimmick = gimmick;
    candidate->action.allyInteractionKind = allyInteractionKind;
    candidate->effectiveMove = move;
    candidate->legacyPrior = min(32767, max(-32768, prior));
    candidate->stableKey = (moveSlot << 8) | (target << 4) | gimmick;
    candidate->readFlags = readFlags;
    candidate->resourceFlags = AiJointRuntime_GetResourceFlags(gimmick);
    candidate->effectKey = GetMoveEffect(move);
    candidate->family = allyInteractionKind != AI_ALLY_INTERACTION_NONE
                      ? AI_JOINT_FAMILY_APPROVED_ALLY
                      : AiJointRuntime_GetCandidateFamily(move);
    candidate->flags = AI_JOINT_ATOMIC_VALID;
    if (candidate->family == AI_JOINT_FAMILY_SPEED_FIELD
     || candidate->effectKey == EFFECT_REFLECT
     || candidate->effectKey == EFFECT_LIGHT_SCREEN
     || candidate->effectKey == EFFECT_AURORA_VEIL)
        candidate->flags |= AI_JOINT_ATOMIC_FIELD_SETUP;
    if (IsSpreadMove(GetMoveTarget(move)))
        candidate->flags |= AI_JOINT_ATOMIC_SPREAD;
}

static enum AiSimApplyStatus AiJointRuntime_CheckAction(const struct AiSimContext *context,
                                                        const struct AiSimBoard *board,
                                                        const struct AiSimAction *action,
                                                        u32 *unsupportedFlags)
{
    struct AiSimJointTurn turn = {0};
    u32 rosterIndex;
    const struct AiSimCombatProfile *profile;

    if (action == NULL || action->actor >= MAX_BATTLERS_COUNT)
        return AI_SIM_APPLY_INVALID;
    if ((action->flags & AI_SIM_ACTION_FORCED_REPLACEMENT)
     && action->replacementRosterIndex < AI_SIM_ROSTER_COUNT)
        rosterIndex = action->replacementRosterIndex;
    else
        rosterIndex = board->active[action->actor].rosterIndex;
    if (action->gimmick == GIMMICK_TERA && rosterIndex < AI_SIM_ROSTER_COUNT)
    {
        profile = &context->mons[rosterIndex].normal;
        // A forced replacement must inspect the incoming party member, not
        // the fainted slot's stale transformed flag.  Mega and Ultra forms
        // persist while benched, so their party-level active gimmick selects
        // the transformed profile before a later Tera action is checked.
        if (board->party[rosterIndex].activeGimmick == GIMMICK_MEGA
         || board->party[rosterIndex].activeGimmick == GIMMICK_ULTRA_BURST
         || (!(action->flags & AI_SIM_ACTION_FORCED_REPLACEMENT)
          && board->active[action->actor].rosterIndex == rosterIndex
          && (board->active[action->actor].flags & AI_SIM_ACTIVE_TRANSFORMED)))
            profile = &context->mons[rosterIndex].transformed;
        // Ogerpon/Terapagos-style Tera entry changes species, stats, ability,
        // and may trigger an ability event. The compact simulator currently
        // models only the type overlay, so preserve the legal action as an
        // unresolved branch instead of silently simulating the wrong form.
        if ((profile->flags & AI_SIM_PROFILE_VALID)
         && DoesSpeciesHaveFormChangeMethod(profile->species,
                                            FORM_CHANGE_BATTLE_TERASTALLIZATION))
        {
            *unsupportedFlags |= AI_SIM_UNSUPPORTED_GIMMICK
                               | AI_SIM_UNSUPPORTED_SWITCH_IN_EFFECT;
            return AI_SIM_APPLY_UNSUPPORTED;
        }
    }
    turn.actions[action->actor] = *action;
    turn.actionMask = 1u << action->actor;
    return AiSim_CheckJointTurn(context, board, &turn, unsupportedFlags);
}

static u16 AiJointRuntime_GetRootRejectionFlags(enum BattlerId actor,
                                                enum BattlerId scoreTarget,
                                                u32 moveSlot,
                                                const struct AiSimAction *action,
                                                bool32 *deferredConfirmedFlinch)
{
    u16 rejectionFlags =
        gAiBattleData->candidateRejectionFlags[actor][scoreTarget][moveSlot];

    *deferredConfirmedFlinch =
        (rejectionFlags & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH) != 0;
    // Confirmed Fake Out is a composed-turn interaction, not an atomic
    // legality failure.  The legacy evaluator cannot see a paired Quick
    // Guard or a partner action that flinches the Fake Out user first, and
    // its mutable partnerMove may describe a stale independent decision.
    // Preserve the read below, then let AiSimJointTurn resolve ordering,
    // guards, immunities, and flinch against the complete four-action turn.
    rejectionFlags &= ~AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH;

    rejectionFlags |= AiSim_GetActionRejectionFlags(&sAiJointRuntime->context,
                                                     &sAiJointRuntime->rootBoard,
                                                     action);
    return rejectionFlags;
}

static void AiJointRuntime_RecordDeferredConfirmedFlinch(
    struct AiJointAtomicCandidate *candidate,
    bool32 deferredConfirmedFlinch)
{
    if (!deferredConfirmedFlinch)
        return;

    candidate->readFlags |= AI_READ_INTERACTION_FAKE_OUT;
    // AI_CheckBadMove returns exactly zero at the legacy hard gate.  Restore
    // only that sentinel to a neutral root prior so the otherwise legal
    // action can survive bounded atomic retention.  The prior is not part of
    // board evaluation; the composed simulator still scores a line where the
    // action is actually flinched as skipped.
    if (candidate->legacyPrior == 0
     && candidate->rejectionFlags == AI_CANDIDATE_REJECTION_NONE)
        candidate->legacyPrior = AI_SCORE_DEFAULT;
}

static bool32 AiJointRuntime_IsRootActionSupported(const struct AiSimAction *action,
                                                   u16 rejectionFlags)
{
    u32 unsupportedFlags = 0;
    enum AiSimApplyStatus status;

    if (rejectionFlags != AI_CANDIDATE_REJECTION_NONE)
        return FALSE;
    status = AiJointRuntime_CheckAction(&sAiJointRuntime->context,
                                        &sAiJointRuntime->rootBoard,
                                        action,
                                        &unsupportedFlags);
    if (status == AI_SIM_APPLY_UNSUPPORTED || unsupportedFlags != AI_SIM_UNSUPPORTED_NONE)
        sAiJointRuntime->rootGenerationIncomplete = TRUE;
    return status == AI_SIM_APPLY_OK && unsupportedFlags == AI_SIM_UNSUPPORTED_NONE;
}

static u32 AiJointRuntime_BuildRootCandidates(enum BattlerId actor,
                                              struct AiJointAtomicCandidate *output,
                                              u32 capacity)
{
    u32 count = 0;
    u32 moveCapacity = capacity > PARTY_SIZE ? capacity - PARTY_SIZE : capacity;
    enum Gimmick usableGimmick = gBattleStruct->gimmick.usableGimmick[actor];

    for (enum BattlerId scoreTarget = 0; scoreTarget < gBattlersCount; scoreTarget++)
    {
        if (!IsBattlerAlive(scoreTarget))
            continue;
        for (u32 moveSlot = 0; moveSlot < MAX_MON_MOVES && count < moveCapacity; moveSlot++)
        {
            struct AiJointAtomicCandidate *candidate;
            enum Move move = gBattleMons[actor].moves[moveSlot];
            enum BattlerId target;
            u16 rejectionFlags;
            bool32 deferredConfirmedFlinch;

            if (move == MOVE_NONE || move == MOVE_UNAVAILABLE
             || !CanTargetBattler(actor, scoreTarget, move))
                continue;
            target = AiJointRuntime_NormalizeRootTarget(actor, scoreTarget, move);
            candidate = &output[count++];
            AiJointRuntime_InitMoveCandidate(candidate, actor, target, move, moveSlot, GIMMICK_NONE,
                                             gAiBattleData->finalScore[actor][scoreTarget][moveSlot],
                                             gAiBattleData->candidateReadInteractionFlags[actor][scoreTarget][moveSlot],
                                             gAiBattleData->candidateAllyInteractionKinds[actor][scoreTarget][moveSlot]);
            rejectionFlags = AiJointRuntime_GetRootRejectionFlags(actor,
                                                                  scoreTarget,
                                                                  moveSlot,
                                                                  &candidate->action,
                                                                  &deferredConfirmedFlinch);
            candidate->rejectionFlags = rejectionFlags;
            AiJointRuntime_RecordDeferredConfirmedFlinch(candidate,
                                                         deferredConfirmedFlinch);
            if (!AiJointRuntime_IsRootActionSupported(&candidate->action,
                                                       rejectionFlags))
            {
                candidate->legalityFlags |= AI_JOINT_ILLEGAL_OTHER;
                candidate->flags &= ~AI_JOINT_ATOMIC_VALID;
            }
            if (GetMovePower(move) != 0
             && gAiLogicData->simulatedDmg[actor][scoreTarget][moveSlot].minimum >= gBattleMons[scoreTarget].hp)
            {
                candidate->family = AI_JOINT_FAMILY_CLEAN_KO;
                candidate->flags |= AI_JOINT_ATOMIC_GUARANTEED_KO;
                candidate->koTargetMask |= 1u << scoreTarget;
            }

            // Gimmick use is part of the atomic action, not a decision inherited
            // from the independent legacy winner.  Keep a base route and a
            // bounded usable-gimmick variant for every relevant move/target.
            if (count < moveCapacity && usableGimmick != GIMMICK_NONE)
            {
                struct AiJointAtomicCandidate *gimmickCandidate = &output[count];
                enum Move effectiveMove = AiJointRuntime_GetEffectiveRootMove(actor,
                                                                               move,
                                                                               usableGimmick);

                // A Z crystal only enables moves of its type (or its matching
                // signature move). Other slots have no legal Z action.
                if (effectiveMove == MOVE_NONE)
                    continue;

                *gimmickCandidate = *candidate;
                gimmickCandidate->action.gimmick = usableGimmick;
                gimmickCandidate->action.choice = effectiveMove;
                gimmickCandidate->action.target = AiJointRuntime_NormalizeRootTarget(
                    actor, scoreTarget, effectiveMove);
                gimmickCandidate->effectiveMove = effectiveMove;
                gimmickCandidate->effectKey = GetMoveEffect(effectiveMove);
                gimmickCandidate->legalityFlags = 0;
                gimmickCandidate->rejectionFlags = rejectionFlags;
                gimmickCandidate->flags |= AI_JOINT_ATOMIC_VALID;
                gimmickCandidate->flags &= ~AI_JOINT_ATOMIC_SPREAD;
                if (IsSpreadMove(GetMoveTarget(effectiveMove)))
                    gimmickCandidate->flags |= AI_JOINT_ATOMIC_SPREAD;
                gimmickCandidate->resourceFlags = AiJointRuntime_GetResourceFlags(usableGimmick);
                gimmickCandidate->stableKey = (moveSlot << 8)
                                             | (gimmickCandidate->action.target << 4)
                                             | usableGimmick;
                gimmickCandidate->family = AI_JOINT_FAMILY_GIMMICK;
                rejectionFlags = AiJointRuntime_GetRootRejectionFlags(actor,
                                                                      scoreTarget,
                                                                      moveSlot,
                                                                      &gimmickCandidate->action,
                                                                      &deferredConfirmedFlinch);
                gimmickCandidate->rejectionFlags = rejectionFlags;
                AiJointRuntime_RecordDeferredConfirmedFlinch(
                    gimmickCandidate, deferredConfirmedFlinch);
                if (!AiJointRuntime_IsRootActionSupported(&gimmickCandidate->action,
                                                           rejectionFlags))
                {
                    gimmickCandidate->legalityFlags |= rejectionFlags != 0
                                                     ? AI_JOINT_ILLEGAL_OTHER
                                                     : AI_JOINT_ILLEGAL_GIMMICK;
                    gimmickCandidate->flags &= ~AI_JOINT_ATOMIC_VALID;
                }
                else
                {
                    count++;
                }
            }
        }
    }

    for (u32 partyIndex = 0; partyIndex < PARTY_SIZE && count < capacity; partyIndex++)
    {
        u32 reserve = GetBattlerTrainer(actor) * PARTY_SIZE + partyIndex;
        u32 legacyPartyIndex = gBattleStruct->AI_monToSwitchIntoId[actor];
        struct AiJointAtomicCandidate *candidate;
        bool32 active = FALSE;

        if (legacyPartyIndex >= PARTY_SIZE)
            legacyPartyIndex = gAiLogicData->mostSuitableMonId[actor];
        if (reserve >= AI_SIM_ROSTER_COUNT
         || !(sAiJointRuntime->context.mons[reserve].flags & AI_SIM_MON_PRESENT)
         || sAiJointRuntime->rootBoard.party[reserve].hp == 0)
            continue;
        for (enum BattlerId battler = 0; battler < gBattlersCount; battler++)
        {
            if ((sAiJointRuntime->rootBoard.activeMask & (1u << battler))
             && sAiJointRuntime->rootBoard.active[battler].rosterIndex == reserve)
            {
                active = TRUE;
                break;
            }
        }
        if (active)
            continue;
        candidate = &output[count++];
        memset(candidate, 0, sizeof(*candidate));
        candidate->action.actor = actor;
        candidate->action.target = AI_SIM_ROSTER_NONE;
        candidate->action.kind = AI_SIM_ACTION_SWITCH;
        candidate->action.choice = reserve;
        candidate->action.moveSlot = AI_SIM_MOVE_SLOT_NONE;
        candidate->legacyPrior = partyIndex == legacyPartyIndex
                               ? AI_SCORE_DEFAULT + 20
                               : AI_SCORE_DEFAULT - 10;
        candidate->stableKey = 0x8000 | reserve;
        candidate->family = AI_JOINT_FAMILY_SWITCH;
        candidate->flags = AI_JOINT_ATOMIC_VALID;
        if (!AiJointRuntime_IsRootActionSupported(&candidate->action,
                                                   AI_CANDIDATE_REJECTION_NONE))
        {
            candidate->legalityFlags |= AI_JOINT_ILLEGAL_SWITCH;
            candidate->flags &= ~AI_JOINT_ATOMIC_VALID;
        }
    }

    return count;
}

static bool32 AiJointRuntime_TargetAllowed(const struct AiSimContext *context,
                                           const struct AiSimBoard *board,
                                           enum BattlerId actor,
                                           enum BattlerId target,
                                           enum MoveTarget targetType)
{
    bool32 ally = ((actor ^ target) & BIT_SIDE) == 0;

    if (!AiJointRuntime_IsAliveOnBoard(board, target)
     && (!(board->activeMask & (1u << target))
      || !(board->active[target].flags & AI_SIM_ACTIVE_NEEDS_REPLACEMENT)
      || !AiSim_ReplacementSlotWillBeFilled(context, board, target)))
        return FALSE;
    switch (targetType)
    {
    case TARGET_USER:
    case TARGET_FIELD:
    case TARGET_OPPONENTS_FIELD:
    case TARGET_USER_AND_ALLY:
    case TARGET_ALL_BATTLERS:
        return target == actor;
    case TARGET_ALLY:
        return ally && target != actor;
    case TARGET_USER_OR_ALLY:
        return ally;
    case TARGET_FOES_AND_ALLY:
    case TARGET_BOTH:
    case TARGET_SELECTED:
    case TARGET_SMART:
    case TARGET_DEPENDS:
    case TARGET_OPPONENT:
    case TARGET_RANDOM:
    default:
        return !ally;
    }
}

static bool32 AiJointRuntime_ActionWaitsForReplacement(const struct AiSimBoard *board,
                                                       const struct AiSimAction *action)
{
    enum MoveTarget targetType;

    if (action->kind != AI_SIM_ACTION_MOVE)
        return FALSE;
    targetType = GetMoveTarget(action->choice);
    for (enum BattlerId target = 0; target < MAX_BATTLERS_COUNT; target++)
    {
        bool32 affected = target == action->target;

        if (!(board->activeMask & (1u << target))
         || !(board->active[target].flags & AI_SIM_ACTIVE_NEEDS_REPLACEMENT))
            continue;
        if (targetType == TARGET_BOTH)
            affected = ((target ^ action->actor) & BIT_SIDE) != 0;
        else if (targetType == TARGET_FOES_AND_ALLY || targetType == TARGET_ALL_BATTLERS)
            affected = target != action->actor;
        if (affected)
            return TRUE;
    }
    return FALSE;
}

static enum AiSimApplyStatus AiJointRuntime_CheckFutureAction(const struct AiSimContext *context,
                                                              const struct AiSimBoard *board,
                                                              const struct AiSimAction *action)
{
    u32 unsupportedFlags = 0;
    enum AiSimApplyStatus status;

    if (AiSim_GetActionRejectionFlags(context, board, action) != 0)
        return AI_SIM_APPLY_UNSUPPORTED;
    status = AiJointRuntime_CheckAction(context, board, action, &unsupportedFlags);
    if (status == AI_SIM_APPLY_OK && unsupportedFlags == AI_SIM_UNSUPPORTED_NONE)
        return AI_SIM_APPLY_OK;
    // The atomic generator does not yet know which opposing reserve will
    // occupy a fainted slot. The composed four-action turn does, and the
    // immutable simulator performs the definitive support check there.
    if (status == AI_SIM_APPLY_INVALID
     && unsupportedFlags == AI_SIM_UNSUPPORTED_NONE
     && AiJointRuntime_ActionWaitsForReplacement(board, action))
        return AI_SIM_APPLY_OK;
    return status == AI_SIM_APPLY_INVALID ? AI_SIM_APPLY_INVALID : AI_SIM_APPLY_UNSUPPORTED;
}

static bool32 AiJointRuntime_ActionAffectsTarget(const struct AiSimAction *action,
                                                 enum BattlerId target)
{
    enum MoveTarget targetType = GetMoveTarget(action->choice);

    if (target == action->actor)
        return targetType == TARGET_ALL_BATTLERS;
    if (targetType == TARGET_BOTH)
        return ((target ^ action->actor) & BIT_SIDE) != 0;
    if (targetType == TARGET_FOES_AND_ALLY || targetType == TARGET_ALL_BATTLERS)
        return TRUE;
    return target == action->target;
}

static void AiJointRuntime_SetFuturePrior(const struct AiSimContext *context,
                                          const struct AiSimBoard *board,
                                          struct AiJointAtomicCandidate *candidate)
{
    s32 prior = 0;

    switch (candidate->family)
    {
    case AI_JOINT_FAMILY_PROTECT:
        prior = 90;
        break;
    case AI_JOINT_FAMILY_SETUP:
    case AI_JOINT_FAMILY_SPEED_FIELD:
        prior = 80;
        break;
    case AI_JOINT_FAMILY_DISRUPTION:
    case AI_JOINT_FAMILY_SUPPORT:
        prior = 70;
        break;
    case AI_JOINT_FAMILY_SWITCH:
        prior = 30;
        break;
    default:
        prior = 50;
        break;
    }
    if (candidate->action.kind == AI_SIM_ACTION_MOVE
     && GetMovePower(candidate->action.choice) != 0)
    {
        prior = 100;
        for (enum BattlerId target = 0; target < context->battlersCount; target++)
        {
            struct SimulatedDamage damage;
            u32 hp;

            if (!AiJointRuntime_IsAliveOnBoard(board, target)
             || !AiJointRuntime_ActionAffectsTarget(&candidate->action, target)
             || !AiSim_CalcDamage(context, board, &candidate->action, target, &damage))
                continue;
            hp = board->party[board->active[target].rosterIndex].hp;
            if (hp == 0)
                continue;
            if (((candidate->action.actor ^ target) & BIT_SIDE) == 0)
            {
                prior -= min(300, damage.median * 150 / hp);
            }
            else
            {
                prior += min(200, damage.median * 100 / hp);
                if (damage.minimum >= hp)
                {
                    prior += 400;
                    candidate->family = AI_JOINT_FAMILY_CLEAN_KO;
                    candidate->flags |= AI_JOINT_ATOMIC_GUARANTEED_KO;
                    candidate->koTargetMask |= 1u << target;
                }
            }
        }
        if (AiJointRuntime_ActionWaitsForReplacement(board, &candidate->action))
            prior += 250;
    }
    if (candidate->action.gimmick != GIMMICK_NONE)
        prior += 25;
    candidate->legacyPrior = min(32767, prior);
}

static void AiJointRuntime_ConsiderFuture(const struct AiSimContext *context,
                                          const struct AiSimBoard *supportBoard,
                                          const struct AiSimBoard *scoreBoard,
                                          struct AiJointAtomicCandidate *candidate,
                                          struct AiJointAtomicCandidate *candidates,
                                          u32 capacity,
                                          u32 *count,
                                          bool32 *unresolved)
{
    enum AiSimApplyStatus status = AiJointRuntime_CheckFutureAction(context,
                                                                   supportBoard,
                                                                   &candidate->action);

    if (status == AI_SIM_APPLY_UNSUPPORTED)
    {
        *unresolved = TRUE;
        return;
    }
    if (status == AI_SIM_APPLY_INVALID)
        return;
    AiJointRuntime_SetFuturePrior(context, scoreBoard, candidate);
    if (*count < capacity)
        candidates[(*count)++] = *candidate;
    else if (capacity != 0)
    {
        u32 worst = 0;

        for (u32 i = 1; i < capacity; i++)
        {
            if (candidates[i].legacyPrior < candidates[worst].legacyPrior
             || (candidates[i].legacyPrior == candidates[worst].legacyPrior
              && candidates[i].stableKey > candidates[worst].stableKey))
                worst = i;
        }
        if (candidate->legacyPrior > candidates[worst].legacyPrior
         || (candidate->legacyPrior == candidates[worst].legacyPrior
          && candidate->stableKey < candidates[worst].stableKey))
            candidates[worst] = *candidate;
    }
}

static void AiJointRuntime_AppendUnresolved(enum BattlerId actor,
                                           struct AiJointAtomicCandidate *candidates,
                                           u32 capacity,
                                           u32 *count)
{
    struct AiJointAtomicCandidate unresolved = {0};

    if (capacity == 0)
        return;
    unresolved.action.actor = actor;
    unresolved.action.target = AI_SIM_ROSTER_NONE;
    unresolved.action.kind = AI_SIM_ACTION_NONE;
    unresolved.action.moveSlot = AI_SIM_MOVE_SLOT_NONE;
    unresolved.action.replacementRosterIndex = AI_SIM_ROSTER_NONE;
    unresolved.legacyPrior = 32767;
    unresolved.stableKey = 0;
    unresolved.family = AI_JOINT_FAMILY_OTHER;
    unresolved.flags = AI_JOINT_ATOMIC_VALID | AI_JOINT_ATOMIC_UNRESOLVED;
    if (*count < capacity)
        candidates[(*count)++] = unresolved;
    else
        candidates[capacity - 1] = unresolved;
}

static void AiJointRuntime_PreviewReplacement(struct AiSimBoard *board,
                                              enum BattlerId actor,
                                              u32 reserve)
{
    u32 stat;
    u32 activeGimmick = board->party[reserve].activeGimmick;

    board->active[actor] = (struct AiSimActiveState){0};
    board->active[actor].rosterIndex = reserve;
    board->active[actor].firstTurn = TRUE;
    board->active[actor].choiceMoveSlot = AI_SIM_MOVE_SLOT_NONE;
    for (stat = 0; stat < NUM_BATTLE_STATS; stat++)
        board->active[actor].statStages[stat] = DEFAULT_STAT_STAGE;
    if (activeGimmick == GIMMICK_MEGA || activeGimmick == GIMMICK_ULTRA_BURST)
        board->active[actor].flags |= AI_SIM_ACTIVE_TRANSFORMED;
    else if (activeGimmick == GIMMICK_TERA)
        board->active[actor].flags |= AI_SIM_ACTIVE_TERA;
}

static bool32 AiJointRuntime_IsGimmickStateLegal(const struct AiSimBoard *board,
                                                 u32 rosterIndex,
                                                 enum Gimmick gimmick)
{
    enum Gimmick activeGimmick = board->party[rosterIndex].activeGimmick;

    if (gimmick == GIMMICK_Z_MOVE)
        return activeGimmick == GIMMICK_NONE
            || activeGimmick == GIMMICK_ULTRA_BURST;
    return activeGimmick == GIMMICK_NONE;
}

static bool32 AiJointRuntime_IsGimmickAvailable(const struct AiSimContext *context,
                                                const struct AiSimBoard *board,
                                                u32 rosterIndex,
                                                enum Gimmick gimmick)
{
    const struct AiSimMonTemplate *mon;
    u32 trainer;

    if (context == NULL || board == NULL || rosterIndex >= AI_SIM_ROSTER_COUNT
     || gimmick <= GIMMICK_NONE || gimmick >= GIMMICKS_COUNT)
        return FALSE;
    mon = &context->mons[rosterIndex];
    trainer = mon->trainer;

    if (!(mon->eligibleGimmicks & (1u << gimmick))
     || trainer >= MAX_BATTLE_TRAINERS
     || (board->trainerGimmickUsed[trainer] & (1u << gimmick))
     || !AiJointRuntime_IsGimmickStateLegal(board, rosterIndex, gimmick))
        return FALSE;

    // AssignUsableGimmicks exposes only the first currently legal gimmick.
    // Keep the full intrinsic mask in the snapshot for future turns, then
    // reproduce that live priority here. This also permits Z only after an
    // eligible Ultra Burst has become active (or otherwise unavailable).
    for (enum Gimmick prior = GIMMICK_MEGA; prior < gimmick; prior++)
    {
        if ((mon->eligibleGimmicks & (1u << prior))
         && !(board->trainerGimmickUsed[trainer] & (1u << prior))
         && AiJointRuntime_IsGimmickStateLegal(board, rosterIndex, prior))
            return FALSE;
    }
    return TRUE;
}

static bool32 AiJointRuntime_IsCanonicalFutureTarget(const struct AiSimContext *context,
                                                     const struct AiSimBoard *board,
                                                     enum BattlerId actor,
                                                     enum BattlerId target,
                                                     enum MoveTarget targetType)
{
    if (!AiJointRuntime_TargetAllowed(context, board, actor, target, targetType))
        return FALSE;
    if (!IsSpreadMove(targetType))
        return TRUE;
    for (enum BattlerId earlier = 0; earlier < target; earlier++)
    {
        if (AiJointRuntime_TargetAllowed(context, board, actor, earlier, targetType))
            return FALSE;
    }
    return TRUE;
}

static u32 AiJointRuntime_GenerateFuture(const struct AiSimContext *context,
                                         const struct AiSimBoard *board,
                                         u8 side,
                                         enum BattlerId actor,
                                         u8 turnIndex,
                                         struct AiJointAtomicCandidate *candidates,
                                         u32 capacity,
                                         void *data)
{
    u32 rosterIndex;
    u32 count = 0;
    bool32 needsReplacement;
    bool32 unresolved = FALSE;

    (void)turnIndex;
    (void)data;
    if (context == NULL || board == NULL || candidates == NULL
     || actor >= context->battlersCount || (actor & BIT_SIDE) != side)
        return 0;
    rosterIndex = board->active[actor].rosterIndex;
    if (rosterIndex >= AI_SIM_ROSTER_COUNT)
        return 0;
    needsReplacement = (board->active[actor].flags & AI_SIM_ACTIVE_NEEDS_REPLACEMENT) != 0;
    if (!AiJointRuntime_IsAliveOnBoard(board, actor) && !needsReplacement)
        return 0;
    if (!needsReplacement
     && board->party[rosterIndex].activeGimmick == GIMMICK_DYNAMAX)
    {
        // Max/G-Max move derivation depends on live battle-only form/type
        // state that is not yet fully captured in the immutable context.
        AiJointRuntime_AppendUnresolved(actor, candidates, capacity, &count);
        return count;
    }

    if (!needsReplacement)
    {
        const struct AiSimMonTemplate *mon = &context->mons[rosterIndex];

        for (u32 moveSlot = 0; moveSlot < MAX_MON_MOVES; moveSlot++)
        {
            enum Move move = mon->moves[moveSlot];
            enum MoveTarget targetType;

            if (move == MOVE_NONE || move == MOVE_UNAVAILABLE
             || board->party[rosterIndex].pp[moveSlot] == 0)
                continue;
            targetType = GetMoveTarget(move);
            for (enum BattlerId target = 0;
                 target < context->battlersCount;
                 target++)
            {
                struct AiJointAtomicCandidate candidate;

                if (AiJointRuntime_IsCanonicalFutureTarget(context, board, actor, target, targetType))
                {
                    AiJointRuntime_InitMoveCandidate(&candidate, actor, target, move, moveSlot,
                                                     GIMMICK_NONE, 0, 0,
                                                     AI_ALLY_INTERACTION_NONE);
                    AiJointRuntime_ConsiderFuture(context, board, board, &candidate,
                                                  candidates, capacity, &count, &unresolved);
                }

                // Future turns can still spend an unused independently legal
                // transformation. Damaging Z moves use their effective move
                // while preserving the base slot; Max remains fail-closed.
                for (enum Gimmick gimmick = GIMMICK_MEGA;
                     gimmick < GIMMICKS_COUNT;
                     gimmick++)
                {
                    enum Move effectiveMove;

                    if (!AiJointRuntime_IsGimmickAvailable(context, board, rosterIndex, gimmick))
                        continue;
                    effectiveMove = AiJointRuntime_GetEffectiveMove(context, board, actor,
                                                                    move, gimmick);
                    if (effectiveMove == MOVE_NONE)
                    {
                        if (gimmick == GIMMICK_DYNAMAX)
                            unresolved = TRUE;
                        continue;
                    }
                    if (!AiJointRuntime_IsCanonicalFutureTarget(context, board, actor, target,
                                                                GetMoveTarget(effectiveMove)))
                        continue;
                    AiJointRuntime_InitMoveCandidate(&candidate, actor, target, effectiveMove,
                                                     moveSlot, gimmick, 0, 0,
                                                     AI_ALLY_INTERACTION_NONE);
                    candidate.family = AI_JOINT_FAMILY_GIMMICK;
                    AiJointRuntime_ConsiderFuture(context, board, board, &candidate,
                                                  candidates, capacity, &count, &unresolved);
                }
            }
        }
    }

    for (u32 reserve = 0; reserve < AI_SIM_ROSTER_COUNT; reserve++)
    {
        const struct AiSimMonTemplate *reserveMon = &context->mons[reserve];

        if (!(reserveMon->flags & AI_SIM_MON_PRESENT)
         || reserveMon->trainer != context->battlerTrainer[actor]
         || board->party[reserve].hp == 0)
            continue;
        bool32 isActive = FALSE;
        for (enum BattlerId active = 0; active < context->battlersCount; active++)
        {
            if ((board->activeMask & (1u << active)) && board->active[active].rosterIndex == reserve)
            {
                isActive = TRUE;
                break;
            }
        }
        if (isActive)
            continue;
        if (needsReplacement)
        {
            struct AiSimBoard previewBoard = *board;

            AiJointRuntime_PreviewReplacement(&previewBoard, actor, reserve);
            for (u32 moveSlot = 0; moveSlot < MAX_MON_MOVES; moveSlot++)
            {
                enum Move move = reserveMon->moves[moveSlot];
                enum MoveTarget targetType;

                if (move == MOVE_NONE || move == MOVE_UNAVAILABLE
                 || board->party[reserve].pp[moveSlot] == 0)
                    continue;
                targetType = GetMoveTarget(move);
                for (enum BattlerId target = 0;
                     target < context->battlersCount;
                     target++)
                {
                    struct AiJointAtomicCandidate candidate;

                    if (AiJointRuntime_IsCanonicalFutureTarget(context, &previewBoard, actor,
                                                               target, targetType))
                    {
                        AiJointRuntime_InitMoveCandidate(&candidate, actor, target, move,
                                                         moveSlot, GIMMICK_NONE, 0, 0,
                                                         AI_ALLY_INTERACTION_NONE);
                        candidate.action.replacementRosterIndex = reserve;
                        candidate.action.flags |= AI_SIM_ACTION_FORCED_REPLACEMENT;
                        candidate.stableKey = 0x4000 | (reserve << 9)
                                            | (moveSlot << 7) | (target << 4);
                        AiJointRuntime_ConsiderFuture(context, board, &previewBoard, &candidate,
                                                      candidates, capacity, &count, &unresolved);
                    }

                    for (enum Gimmick gimmick = GIMMICK_MEGA;
                         gimmick < GIMMICKS_COUNT;
                         gimmick++)
                    {
                        enum Move effectiveMove;

                        if (!AiJointRuntime_IsGimmickAvailable(context, board, reserve, gimmick))
                            continue;
                        effectiveMove = AiJointRuntime_GetEffectiveMove(context, &previewBoard,
                                                                        actor, move, gimmick);
                        if (effectiveMove == MOVE_NONE)
                        {
                            if (gimmick == GIMMICK_DYNAMAX)
                                unresolved = TRUE;
                            continue;
                        }
                    if (!AiJointRuntime_IsCanonicalFutureTarget(context, &previewBoard, actor,
                                                                target,
                                                                GetMoveTarget(effectiveMove)))
                            continue;
                        AiJointRuntime_InitMoveCandidate(&candidate, actor, target, effectiveMove,
                                                         moveSlot, gimmick, 0, 0,
                                                         AI_ALLY_INTERACTION_NONE);
                        candidate.action.replacementRosterIndex = reserve;
                        candidate.action.flags |= AI_SIM_ACTION_FORCED_REPLACEMENT;
                        candidate.stableKey = 0x4000 | (reserve << 9)
                                            | (moveSlot << 7) | (target << 4) | gimmick;
                        candidate.family = AI_JOINT_FAMILY_GIMMICK;
                        AiJointRuntime_ConsiderFuture(context, board, &previewBoard, &candidate,
                                                      candidates, capacity, &count, &unresolved);
                    }
                }
            }
        }
        else
        {
            struct AiJointAtomicCandidate candidate;

            memset(&candidate, 0, sizeof(candidate));
            candidate.action.actor = actor;
            candidate.action.target = AI_SIM_ROSTER_NONE;
            candidate.action.kind = AI_SIM_ACTION_SWITCH;
            candidate.action.choice = reserve;
            candidate.action.moveSlot = AI_SIM_MOVE_SLOT_NONE;
            candidate.stableKey = 0x8000 | reserve;
            candidate.family = AI_JOINT_FAMILY_SWITCH;
            candidate.flags = AI_JOINT_ATOMIC_VALID;
            AiJointRuntime_ConsiderFuture(context, board, board, &candidate,
                                          candidates, capacity, &count, &unresolved);
        }
    }
    if (unresolved)
        AiJointRuntime_AppendUnresolved(actor, candidates, capacity, &count);
    return count;
}

static bool32 AiJointRuntime_Evaluate(const struct AiSimContext *context,
                                      const struct AiSimBoard *rootBoard,
                                      const struct AiSimBoard *board,
                                      u8 aiSide,
                                      struct AiJointScore *score,
                                      void *data)
{
    const struct AiJointRuntimeData *runtime = data;

    if (runtime == NULL)
        return AiJoint_EvaluateBoard(context, rootBoard, board, aiSide, score);
    return AiJoint_EvaluateBoardWithRootPressure(context, rootBoard, board,
                                                  aiSide,
                                                  runtime->rootPressure,
                                                  score);
}

static u32 AiJointRuntime_Enumerate(const struct AiSimContext *context,
                                    const struct AiSimBoard *board,
                                    const struct AiSimJointTurn *turn,
                                    struct AiSimOutcomeKey *outcomes,
                                    u32 capacity,
                                    void *data)
{
    (void)data;
    return AiSim_EnumerateOutcomes(context, board, turn, outcomes, capacity);
}

static enum AiSimApplyStatus AiJointRuntime_Apply(const struct AiSimContext *context,
                                                   const struct AiSimBoard *before,
                                                   const struct AiSimJointTurn *turn,
                                                   const struct AiSimOutcomeKey *outcome,
                                                   struct AiSimBoard *after,
                                                   struct AiSimTurnResult *result,
                                                   void *data)
{
    (void)data;
    return AiSim_ApplyJointTurn(context, before, turn, outcome, after, result);
}

static bool32 AiJointRuntime_BuildConfirmedActions(struct AiJointPlannerRequest *request)
{
    u32 count = 0;

    for (enum BattlerId battler = 0; battler < gBattlersCount; battler++)
    {
        struct AiSimAction *action;

        if (!IsOnPlayerSide(battler)
         || !IsBattlerAlive(battler)
         || (gAbsentBattlerFlags & (1u << battler)))
            continue;
        if (count >= ARRAY_COUNT(request->confirmedPlayerActions))
            return FALSE;
        action = &request->confirmedPlayerActions[count++];
        memset(action, 0, sizeof(*action));
        action->actor = battler;
        action->target = AI_SIM_ROSTER_NONE;
        action->moveSlot = AI_SIM_MOVE_SLOT_NONE;
        if (gChosenActionByBattler[battler] == B_ACTION_USE_MOVE)
        {
            enum Move baseMove = gChosenMoveByBattler[battler];

            action->kind = AI_SIM_ACTION_MOVE;
            action->moveSlot = gBattleStruct->chosenMovePositions[battler];
            action->target = gBattleStruct->moveTarget[battler];
            if (gBattleStruct->gimmick.toActivate & (1u << battler))
                action->gimmick = gBattleStruct->gimmick.usableGimmick[battler];
            action->choice = AiJointRuntime_GetEffectiveRootMove(battler,
                                                                 baseMove,
                                                                 action->gimmick);
            if (action->choice == MOVE_NONE)
                return FALSE;
        }
        else if (gChosenActionByBattler[battler] == B_ACTION_SWITCH)
        {
            action->kind = AI_SIM_ACTION_SWITCH;
            action->choice = GetBattlerTrainer(battler) * PARTY_SIZE
                           + gBattleStruct->monToSwitchIntoId[battler];
        }
        else
        {
            return FALSE;
        }
    }
    request->confirmedPlayerActionCount = count;
    return count == 2;
}

static void AiJointRuntime_SaveLegacyAction(u32 actorIndex)
{
    enum BattlerId actor = sAiJointRuntime->actors[actorIndex];
    struct AiSimAction *action = &sAiJointRuntime->legacyActions[actorIndex];
    u32 partyIndex;

    memset(action, 0, sizeof(*action));
    action->actor = actor;
    action->target = AI_SIM_ROSTER_NONE;
    action->moveSlot = AI_SIM_MOVE_SLOT_NONE;
    if (gAiLogicData->shouldSwitch & (1u << actor))
    {
        partyIndex = gBattleStruct->AI_monToSwitchIntoId[actor];
        if (partyIndex >= PARTY_SIZE)
            partyIndex = gAiLogicData->mostSuitableMonId[actor];
        if (partyIndex < PARTY_SIZE)
        {
            action->kind = AI_SIM_ACTION_SWITCH;
            action->choice = GetBattlerTrainer(actor) * PARTY_SIZE + partyIndex;
            sAiJointRuntime->legacyValidMask |= 1u << actor;
            return;
        }
    }
    if (gAiBattleData->chosenMoveIndex[actor] < MAX_MON_MOVES)
    {
        action->kind = AI_SIM_ACTION_MOVE;
        action->moveSlot = gAiBattleData->chosenMoveIndex[actor];
        action->choice = gBattleMons[actor].moves[action->moveSlot];
        action->target = gAiBattleData->chosenTarget[actor];
        if (IsAIUsingGimmick(actor))
            action->gimmick = gBattleStruct->gimmick.usableGimmick[actor];
        if (action->choice != MOVE_NONE && action->choice != MOVE_UNAVAILABLE)
            sAiJointRuntime->legacyValidMask |= 1u << actor;
    }
}

static bool32 AiJointRuntime_StartSearch(u32 signature)
{
    struct AiJointPlannerRequest request;
    u32 actorCount = 0;

    memset(&request, 0, sizeof(request));
    memset(&sAiJointRuntime->context, 0, sizeof(sAiJointRuntime->context));
    memset(&sAiJointRuntime->rootBoard, 0, sizeof(sAiJointRuntime->rootBoard));
    memset(sAiJointRuntime->rootCandidates, 0, sizeof(sAiJointRuntime->rootCandidates));
    memset(sAiJointRuntime->legacyActions, 0, sizeof(sAiJointRuntime->legacyActions));
    sAiJointRuntime->legacyValidMask = 0;
    sAiJointRuntime->rootGenerationIncomplete = FALSE;
    sAiJointRuntime->searchStartFrame = gMain.vblankCounter1;
    sAiJointRuntime->delayRecorded = FALSE;

    for (enum BattlerId actor = 0; actor < gBattlersCount; actor++)
    {
        if (GetBattlerSide(actor) == B_SIDE_OPPONENT && IsBattlerAlive(actor))
            sAiJointRuntime->actors[actorCount++] = actor;
    }
    if (actorCount != 2)
        return FALSE;

    // This is the one mutable-global capture for a side-wide plan.  The two
    // legacy evaluations below only seed root priors and provenance; every
    // future node is generated from context+board by the callback above.
    SetAiLogicDataForTurn(gAiLogicData);
    if (!AiSim_CaptureKnownBoard(sAiJointRuntime->actors[0], gAiLogicData,
                                 &sAiJointRuntime->context,
                                 &sAiJointRuntime->rootBoard))
        return FALSE;
    sAiJointRuntime->rootPressure = min(32767, max(-32768,
        AiJoint_CalculateBoardPressure(&sAiJointRuntime->context,
                                       &sAiJointRuntime->rootBoard,
                                       B_SIDE_OPPONENT)));

    for (u32 i = 0; i < ARRAY_COUNT(sAiJointRuntime->actors); i++)
    {
        enum BattlerId actor = sAiJointRuntime->actors[i];

        ComputeAiBattlerDecisions(actor);
        AiJointRuntime_SaveLegacyAction(i);
        sAiJointRuntime->rootCounts[i] = AiJointRuntime_BuildRootCandidates(
            actor, sAiJointRuntime->rootCandidates[i],
            ARRAY_COUNT(sAiJointRuntime->rootCandidates[i]));
        if (sAiJointRuntime->rootCounts[i] == 0)
            return FALSE;
        memset(sAiJointRuntime->retainedRoots[i], 0,
               sizeof(sAiJointRuntime->retainedRoots[i]));
        if (AiJoint_RetainAtomicCandidates(sAiJointRuntime->rootCandidates[i],
                                           sAiJointRuntime->rootCounts[i],
                                           sAiJointRuntime->retainedRoots[i],
                                           ARRAY_COUNT(sAiJointRuntime->retainedRoots[i])) == 0)
            return FALSE;
    }
    if (sAiJointRuntime->rootGenerationIncomplete)
        return FALSE;

    request.context = &sAiJointRuntime->context;
    request.rootBoard = &sAiJointRuntime->rootBoard;
    request.rootCandidates[0] = sAiJointRuntime->rootCandidates[0];
    request.rootCandidates[1] = sAiJointRuntime->rootCandidates[1];
    request.rootCandidateCounts[0] = sAiJointRuntime->rootCounts[0];
    request.rootCandidateCounts[1] = sAiJointRuntime->rootCounts[1];
    request.rootActors[0] = sAiJointRuntime->actors[0];
    request.rootActors[1] = sAiJointRuntime->actors[1];
    request.aiSide = B_SIDE_OPPONENT;
    request.activeAiMask = (1u << sAiJointRuntime->actors[0]) | (1u << sAiJointRuntime->actors[1]);
    request.cacheKeySalt = signature;
    request.generateAtomic = AiJointRuntime_GenerateFuture;
    request.evaluateBoard = AiJointRuntime_Evaluate;
    request.enumerateOutcomes = AiJointRuntime_Enumerate;
    request.applyTurn = AiJointRuntime_Apply;
    request.callbackData = sAiJointRuntime;
    if (sAiJointRuntime->hasLimitOverride)
        request.limits = sAiJointRuntime->limitOverride;
    if (!AiJointRuntime_BuildConfirmedActions(&request))
        return FALSE;

    memset(&sAiJointRuntime->workspace, 0, sizeof(sAiJointRuntime->workspace));
    memset(&sAiJointRuntime->job, 0, sizeof(sAiJointRuntime->job));
    memset(&sAiJointRuntime->result, 0, sizeof(sAiJointRuntime->result));
    sAiJointRuntime->commandSignature = signature;
    sAiJointRuntime->turn = gBattleTurnCounter;
    sAiJointRuntime->appliedMask = 0;
    sAiJointRuntime->reusedMoveMask = 0;
    sAiJointRuntime->planId = BATTLE_AI_TRACE_ID_NONE;
    sAiJointRuntime->buildCount++;
    sAiJointRuntime->lastStepFrame = UINT32_MAX;
    sAiJointRuntime->steppedThisFrame = FALSE;
    if (AiJointPlanner_InitJob(&sAiJointRuntime->job, &sAiJointRuntime->workspace, &request)
     == AI_JOINT_JOB_FALLBACK)
        return FALSE;
    sAiJointRuntime->state = AI_JOINT_RUNTIME_STATE_SEARCHING;
    return TRUE;
}

static s16 AiJointRuntime_ClampTraceScore(s32 score, bool32 *clamped)
{
    if (score > 32767)
    {
        *clamped = TRUE;
        return 32767;
    }
    if (score < -32768)
    {
        *clamped = TRUE;
        return -32768;
    }
    return score;
}

static void AiJointRuntime_InitTraceAction(struct BattleAiTraceAction *traceAction,
                                           const struct AiSimAction *action)
{
    if (action->kind == AI_SIM_ACTION_MOVE)
    {
        BattleAiTrace_InitAction(traceAction, action->actor, B_ACTION_USE_MOVE,
                                 action->choice, action->target, action->moveSlot,
                                 action->gimmick, BATTLE_AI_TRACE_PREDICTION_NONE);
    }
    else if (action->kind == AI_SIM_ACTION_SWITCH)
    {
        u32 partyIndex = action->choice < AI_SIM_ROSTER_COUNT
                       ? sAiJointRuntime->context.mons[action->choice].partyIndex
                       : PARTY_SIZE;
        BattleAiTrace_InitAction(traceAction, action->actor, B_ACTION_SWITCH,
                                 partyIndex, MAX_BATTLERS_COUNT, MAX_MON_MOVES,
                                 GIMMICK_NONE, BATTLE_AI_TRACE_PREDICTION_NONE);
    }
}

static const struct AiJointAtomicCandidate *AiJointRuntime_GetRootAtomic(
    const struct AiJointPairCandidate *pair, u32 actionIndex)
{
    u32 sourceIndex;

    if (pair == NULL || actionIndex >= ARRAY_COUNT(pair->actions))
        return NULL;
    sourceIndex = pair->sourceIndices[actionIndex];
    if (sourceIndex >= AI_JOINT_MAX_ATOMIC_PER_ACTOR)
        return NULL;
    for (u32 group = 0; group < ARRAY_COUNT(sAiJointRuntime->actors); group++)
    {
        if (sAiJointRuntime->actors[group] == pair->actions[actionIndex].actor)
            return &sAiJointRuntime->retainedRoots[group][sourceIndex];
    }
    return NULL;
}

static const struct AiJointAtomicCandidate *AiJointRuntime_GetChosenAtomic(enum BattlerId battler)
{
    for (u32 actionIndex = 0;
         actionIndex < ARRAY_COUNT(sAiJointRuntime->result.chosenPair.actions);
         actionIndex++)
    {
        if (sAiJointRuntime->result.chosenPair.actions[actionIndex].actor == battler)
            return AiJointRuntime_GetRootAtomic(&sAiJointRuntime->result.chosenPair,
                                                actionIndex);
    }
    return NULL;
}

static void AiJointRuntime_FillTraceCandidate(const struct AiJointRootSummary *summary,
                                              struct BattleAiTraceCandidate *candidate)
{
    bool32 clamped = FALSE;

    memset(candidate, 0, sizeof(*candidate));
    for (u32 i = 0; i < 2; i++)
    {
        const struct AiJointAtomicCandidate *atomic =
            AiJointRuntime_GetRootAtomic(&summary->pair, i);

        AiJointRuntime_InitTraceAction(&candidate->actions[i],
                                       &summary->pair.actions[i]);
        candidate->rejectionFlags[i] = summary->pair.rejectionFlags[i];
        if (atomic != NULL)
        {
            candidate->readInteractionFlags[i] = atomic->readFlags;
            candidate->allyInteractionKinds[i] = atomic->action.allyInteractionKind;
        }
    }
    candidate->totalScore = AiJointRuntime_ClampTraceScore(summary->score.total, &clamped);
    candidate->immediateScore = summary->score.immediate;
    candidate->futureScore = summary->score.future;
    candidate->riskScore = summary->score.risk;
    candidate->resourceScore = summary->score.resource;
    candidate->completedDepth = summary->completedDepth;
    candidate->flags = BATTLE_AI_TRACE_CANDIDATE_JOINT;
    if (summary->flags & AI_JOINT_ROOT_DEPTH3_COMPLETE)
        candidate->flags |= BATTLE_AI_TRACE_CANDIDATE_COMPLETE;
    if (summary->flags & (AI_JOINT_ROOT_PRUNED | AI_JOINT_ROOT_UNSUPPORTED))
        candidate->flags |= BATTLE_AI_TRACE_CANDIDATE_PRUNED;
    if (clamped)
        candidate->flags |= BATTLE_AI_TRACE_CANDIDATE_SCORE_CLAMPED;
}

static void AiJointRuntime_FillRejectedTraceCandidate(
    const struct AiJointAtomicCandidate *atomic,
    struct BattleAiTraceCandidate *candidate)
{
    bool32 clamped = FALSE;

    memset(candidate, 0, sizeof(*candidate));
    AiJointRuntime_InitTraceAction(&candidate->actions[0], &atomic->action);
    candidate->totalScore = AiJointRuntime_ClampTraceScore(atomic->legacyPrior, &clamped);
    candidate->immediateScore = candidate->totalScore;
    candidate->rejectionFlags[0] = min(UINT8_MAX, atomic->rejectionFlags);
    candidate->readInteractionFlags[0] = min(UINT8_MAX, atomic->readFlags);
    candidate->allyInteractionKinds[0] = atomic->action.allyInteractionKind;
    candidate->flags = BATTLE_AI_TRACE_CANDIDATE_JOINT
                     | BATTLE_AI_TRACE_CANDIDATE_PRUNED
                     | BATTLE_AI_TRACE_CANDIDATE_FORCED_TRACE
                     | BATTLE_AI_TRACE_CANDIDATE_COMPONENTS_PARTIAL;
    if (clamped)
        candidate->flags |= BATTLE_AI_TRACE_CANDIDATE_SCORE_CLAMPED;
}

static void AiJointRuntime_RecordTrace(void)
{
    struct BattleAiTracePlan plan = {0};
    struct BattleAiTraceCandidate candidates[BATTLE_AI_TRACE_TOP_CANDIDATES];
    u32 candidateCount = 0;
    bool32 traceTruncated = FALSE;

    for (u32 rank = 0; rank < BATTLE_AI_TRACE_TOP_CANDIDATES; rank++)
    {
        struct AiJointRootSummary summary;

        if (!AiJointPlanner_GetRankedRoot(&sAiJointRuntime->job, rank, &summary))
            break;
        AiJointRuntime_FillTraceCandidate(&summary, &candidates[candidateCount]);
        candidateCount++;
    }

    {
        struct AiJointRootSummary omittedSummary;

        traceTruncated = AiJointPlanner_GetRankedRoot(
            &sAiJointRuntime->job, BATTLE_AI_TRACE_TOP_CANDIDATES,
            &omittedSummary);
    }

    if (candidateCount == 0)
        return;
    sAiJointRuntime->traceChosenRank = sAiJointRuntime->result.chosenRank;
    if (sAiJointRuntime->traceChosenRank >= candidateCount)
    {
        struct AiJointRootSummary chosenSummary;

        if (!AiJointPlanner_GetRankedRoot(&sAiJointRuntime->job,
                                          sAiJointRuntime->result.chosenRank,
                                          &chosenSummary))
            return;
        sAiJointRuntime->traceChosenRank = candidateCount - 1;
        AiJointRuntime_FillTraceCandidate(&chosenSummary,
                                          &candidates[sAiJointRuntime->traceChosenRank]);
        candidates[sAiJointRuntime->traceChosenRank].flags |= BATTLE_AI_TRACE_CANDIDATE_FORCED_TRACE;
    }
    {
        struct AiJointAtomicCandidate rejected;

        if (AiJointPlanner_GetForcedRejectedCandidate(&sAiJointRuntime->job, &rejected)
         && rejected.rejectionFlags != AI_CANDIDATE_REJECTION_NONE)
        {
            u32 rejectedRank;

            if (candidateCount < BATTLE_AI_TRACE_TOP_CANDIDATES)
                rejectedRank = candidateCount++;
            else
            {
                rejectedRank = BATTLE_AI_TRACE_TOP_CANDIDATES - 1;
                if (rejectedRank == sAiJointRuntime->traceChosenRank && rejectedRank != 0)
                    rejectedRank--;
                traceTruncated = TRUE;
            }
            AiJointRuntime_FillRejectedTraceCandidate(&rejected,
                                                       &candidates[rejectedRank]);
        }
    }
    plan.actorMask = (1u << sAiJointRuntime->actors[0]) | (1u << sAiJointRuntime->actors[1]);
    plan.chosenRank = sAiJointRuntime->traceChosenRank;
    plan.nodesVisited = sAiJointRuntime->result.stats.nodesVisited;
    plan.nodeBudget = sAiJointRuntime->result.stats.nodeBudget;
    plan.cacheHits = sAiJointRuntime->result.stats.cacheHits;
    plan.elapsedFrames = sAiJointRuntime->result.stats.elapsedFrames;
    plan.requestedDepth = sAiJointRuntime->result.stats.requestedDepth;
    plan.completedDepth = sAiJointRuntime->result.completedDepth;
    plan.terminationReason = BATTLE_AI_TRACE_TERMINATION_COMPLETE;
    plan.flags = BATTLE_AI_TRACE_PLAN_JOINT | BATTLE_AI_TRACE_PLAN_DEEPEST_COMPLETE_USED;
    if (traceTruncated)
        plan.flags |= BATTLE_AI_TRACE_PLAN_TRUNCATED;
    plan.frameBudget = min(UINT8_MAX, sAiJointRuntime->job.request.limits.totalFrameBudget);
    if (sAiJointRuntime->result.stats.terminationReason == AI_JOINT_TERMINATION_NODE_BUDGET)
    {
        plan.terminationReason = BATTLE_AI_TRACE_TERMINATION_NODE_BUDGET;
        plan.flags |= BATTLE_AI_TRACE_PLAN_NODE_BUDGET_HIT;
    }
    else if (sAiJointRuntime->result.stats.terminationReason == AI_JOINT_TERMINATION_FRAME_BUDGET)
    {
        plan.terminationReason = BATTLE_AI_TRACE_TERMINATION_FRAME_BUDGET;
        plan.flags |= BATTLE_AI_TRACE_PLAN_FRAME_BUDGET_HIT;
    }
    for (u32 i = 0; i < 2; i++)
    {
        const struct AiSimAction *action = &sAiJointRuntime->job.request.confirmedPlayerActions[i];
        if (action->kind == AI_SIM_ACTION_MOVE)
            BattleAiTrace_InitAction(&plan.predictedPlayerActions[i], action->actor,
                                     B_ACTION_USE_MOVE, action->choice, action->target,
                                     action->moveSlot, action->gimmick,
                                     BATTLE_AI_TRACE_PREDICTION_CONFIRMED_COMMAND);
        else
            BattleAiTrace_InitAction(&plan.predictedPlayerActions[i], action->actor,
                                     B_ACTION_SWITCH,
                                     sAiJointRuntime->context.mons[action->choice].partyIndex,
                                     MAX_BATTLERS_COUNT, MAX_MON_MOVES, GIMMICK_NONE,
                                     BATTLE_AI_TRACE_PREDICTION_CONFIRMED_COMMAND);
    }
    sAiJointRuntime->planId = BattleAiTrace_RecordPlan(&plan, candidates, candidateCount);
    if (sAiJointRuntime->planId != BATTLE_AI_TRACE_ID_NONE)
    {
        for (u32 i = 0; i < 2; i++)
        {
            enum BattlerId actor = sAiJointRuntime->actors[i];
            const struct AiJointAtomicCandidate *atomic =
                AiJointRuntime_GetChosenAtomic(actor);
            BattleAI_ClearDecisionMetadata(actor);
            BattleAiTrace_SetBattlerDecision(actor, sAiJointRuntime->planId,
                                             sAiJointRuntime->traceChosenRank);
            if (atomic != NULL && atomic->readFlags != 0)
                gAiBattleData->decisionReason[actor] = AI_DECISION_REASON_KNOWN_COMMAND_ANSWER;
        }
        BattleAiTrace_CaptureSimBoard(sAiJointRuntime->planId,
                                      &sAiJointRuntime->context,
                                      &sAiJointRuntime->result.chosenAfterBoard);
    }
}

void BattleAiJointRuntime_Reset(void)
{
    if (sAiJointRuntime != NULL)
        FREE_AND_SET_NULL(sAiJointRuntime);
#if TESTING
    memset(&sAiJointRuntimeReleasedStats, 0, sizeof(sAiJointRuntimeReleasedStats));
#endif
}

void BattleAiJointRuntime_ReleaseData(void)
{
    if (sAiJointRuntime == NULL)
        return;
#if TESTING
    // Runtime tests inspect the completed search after the turn. Preserve only
    // the scalar evidence while exercising the same heap lifecycle as a ROM.
    sAiJointRuntimeReleasedStats.buildCount = sAiJointRuntime->buildCount;
    sAiJointRuntimeReleasedStats.stepCount = sAiJointRuntime->stepCount;
    sAiJointRuntimeReleasedStats.planId = sAiJointRuntime->planId;
#endif
    FREE_AND_SET_NULL(sAiJointRuntime);
}

static u16 AiJointRuntime_GetElapsedFrames(void)
{
    u32 elapsed;

    if (sAiJointRuntime == NULL)
        return 0;
    elapsed = gMain.vblankCounter1 - sAiJointRuntime->searchStartFrame;
    return min(UINT16_MAX, elapsed);
}

static void AiJointRuntime_SyncElapsedFrames(void)
{
    u16 elapsed;

    if (sAiJointRuntime == NULL)
        return;
    elapsed = AiJointRuntime_GetElapsedFrames();
    sAiJointRuntime->job.stats.elapsedFrames = max(
        sAiJointRuntime->job.stats.elapsedFrames, elapsed);
}

static void AiJointRuntime_RecordDelay(void)
{
    if (sAiJointRuntime == NULL || sAiJointRuntime->delayRecorded)
        return;
    if ((TESTING && sAiJointRuntime->turn == 0) || DEBUG_AI_DELAY_TIMER)
        gBattleStruct->aiDelayFrames += AiJointRuntime_GetElapsedFrames();
    sAiJointRuntime->delayRecorded = TRUE;
}

enum AiJointRuntimeStatus BattleAiJointRuntime_Prepare(enum BattlerId battler)
{
    bool32 commandsReady;
    bool32 knownUnsupported;
    u32 signature;
    enum AiJointPlannerState plannerState;

    if (!AiJointRuntime_IsEligible(battler))
        return AI_JOINT_RUNTIME_NOT_ELIGIBLE;
    signature = AiJointRuntime_GetPlayerCommandSignature(&commandsReady,
                                                          &knownUnsupported);
    if (!commandsReady)
    {
        if (knownUnsupported)
        {
            BattleAiJointRuntime_Reset();
            return AI_JOINT_RUNTIME_FALLBACK;
        }
        return AI_JOINT_RUNTIME_PENDING;
    }

    if (!AiJointRuntime_EnsureData())
        return AI_JOINT_RUNTIME_FALLBACK;

    if (sAiJointRuntime->state != AI_JOINT_RUNTIME_STATE_EMPTY
     && (sAiJointRuntime->turn != gBattleTurnCounter
      || sAiJointRuntime->commandSignature != signature))
    {
        BattleAiJointRuntime_Reset();
        if (!AiJointRuntime_EnsureData())
            return AI_JOINT_RUNTIME_FALLBACK;
    }
    if (sAiJointRuntime->state == AI_JOINT_RUNTIME_STATE_EMPTY)
    {
        if (!AiJointRuntime_StartSearch(signature))
        {
            sAiJointRuntime->commandSignature = signature;
            sAiJointRuntime->turn = gBattleTurnCounter;
            sAiJointRuntime->state = AI_JOINT_RUNTIME_STATE_FALLBACK;
            AiJointRuntime_RecordDelay();
            return AI_JOINT_RUNTIME_FALLBACK;
        }
    }
    if (sAiJointRuntime->state == AI_JOINT_RUNTIME_STATE_READY)
    {
        AiJointRuntime_RecordDelay();
        return AI_JOINT_RUNTIME_READY;
    }
    if (sAiJointRuntime->state == AI_JOINT_RUNTIME_STATE_FALLBACK)
    {
        AiJointRuntime_RecordDelay();
        return AI_JOINT_RUNTIME_FALLBACK;
    }

    // Both opponent callbacks can run in one engine pass.  Keying the slice
    // to vblankCounter1 makes exactly one of them advance the shared job.
    if (sAiJointRuntime->lastStepFrame == gMain.vblankCounter1
     && sAiJointRuntime->steppedThisFrame)
        return AI_JOINT_RUNTIME_PENDING;
    sAiJointRuntime->lastStepFrame = gMain.vblankCounter1;
    sAiJointRuntime->steppedThisFrame = TRUE;
    sAiJointRuntime->stepCount++;
    AiJointRuntime_SyncElapsedFrames();
    plannerState = AiJointPlanner_Step(&sAiJointRuntime->job,
                                       AI_JOINT_DEFAULT_STEP_NODES,
                                       &sAiJointRuntime->result);
    if (plannerState == AI_JOINT_JOB_READY)
    {
        // A partial depth-five route may never displace a fully completed
        // standard route.  Conversely, no depth-three result means the
        // standard search contract was not met and must fail closed.
        if (sAiJointRuntime->result.completedDepth < AI_JOINT_STANDARD_DEPTH
         || sAiJointRuntime->result.rootCount == 0
         || sAiJointRuntime->result.unsupportedFlags != AI_SIM_UNSUPPORTED_NONE
         || !(sAiJointRuntime->result.chosenPair.flags & AI_JOINT_PAIR_VALID))
        {
            sAiJointRuntime->state = AI_JOINT_RUNTIME_STATE_FALLBACK;
            AiJointRuntime_RecordDelay();
            return AI_JOINT_RUNTIME_FALLBACK;
        }
        sAiJointRuntime->state = AI_JOINT_RUNTIME_STATE_READY;
        AiJointRuntime_RecordTrace();
        AiJointRuntime_RecordDelay();
        return AI_JOINT_RUNTIME_READY;
    }
    if (plannerState == AI_JOINT_JOB_FALLBACK)
    {
        sAiJointRuntime->state = AI_JOINT_RUNTIME_STATE_FALLBACK;
        AiJointRuntime_RecordDelay();
        return AI_JOINT_RUNTIME_FALLBACK;
    }
    return AI_JOINT_RUNTIME_PENDING;
}

static const struct AiSimAction *AiJointRuntime_GetAction(enum BattlerId battler)
{
    if (sAiJointRuntime == NULL)
        return NULL;
    if (sAiJointRuntime->state == AI_JOINT_RUNTIME_STATE_READY)
    {
        for (u32 i = 0; i < ARRAY_COUNT(sAiJointRuntime->result.chosenPair.actions); i++)
        {
            if (sAiJointRuntime->result.chosenPair.actions[i].actor == battler)
                return &sAiJointRuntime->result.chosenPair.actions[i];
        }
        return NULL;
    }
    if (sAiJointRuntime->state == AI_JOINT_RUNTIME_STATE_FALLBACK
     && (sAiJointRuntime->legacyValidMask & (1u << battler)))
    {
        for (u32 i = 0; i < ARRAY_COUNT(sAiJointRuntime->actors); i++)
        {
            if (sAiJointRuntime->actors[i] == battler)
                return &sAiJointRuntime->legacyActions[i];
        }
    }
    return NULL;
}

static bool32 AiJointRuntime_CanApplyStoredAction(enum BattlerId battler,
                                                  const struct AiSimAction *action)
{
    enum Move baseMove;

    if (sAiJointRuntime == NULL || action == NULL || action->actor != battler)
        return FALSE;
    if (action->kind == AI_SIM_ACTION_SWITCH)
    {
        if (action->choice >= AI_SIM_ROSTER_COUNT)
            return FALSE;
        return sAiJointRuntime->context.mons[action->choice].partyIndex < PARTY_SIZE;
    }
    if (action->kind != AI_SIM_ACTION_MOVE || action->moveSlot >= MAX_MON_MOVES)
        return FALSE;
    baseMove = gBattleMons[battler].moves[action->moveSlot];
    if (baseMove == action->choice)
        return TRUE;
    if (action->gimmick == GIMMICK_Z_MOVE)
        return GetUsableZMove(battler, baseMove) == action->choice;
    if (action->gimmick == GIMMICK_DYNAMAX
     || GetActiveGimmick(battler) == GIMMICK_DYNAMAX)
        return GetMaxMove(battler, baseMove) == action->choice;
    return FALSE;
}

static bool32 AiJointRuntime_ApplyStoredAction(enum BattlerId battler,
                                               const struct AiSimAction *action)
{
    if (!AiJointRuntime_CanApplyStoredAction(battler, action))
        return FALSE;
    gAiLogicData->shouldSwitch &= ~(1u << battler);
    gBattleStruct->AI_monToSwitchIntoId[battler] = PARTY_SIZE;
    gAiLogicData->monToSwitchInId[battler] = PARTY_SIZE;
    if (action->kind == AI_SIM_ACTION_SWITCH)
    {
        u32 partyIndex = sAiJointRuntime->context.mons[action->choice].partyIndex;

        gAiLogicData->shouldSwitch |= 1u << battler;
        gAiLogicData->mostSuitableMonId[battler] = partyIndex;
        gAiLogicData->monToSwitchInId[battler] = partyIndex;
        gBattleStruct->AI_monToSwitchIntoId[battler] = partyIndex;
        SetAIUsingGimmick(battler, NO_GIMMICK);
        return TRUE;
    }
    gAiBattleData->chosenMoveIndex[battler] = action->moveSlot;
    gAiBattleData->chosenTarget[battler] = action->target;
    SetAIUsingGimmick(battler,
                      action->gimmick == GIMMICK_NONE ? NO_GIMMICK : USE_GIMMICK);
    return TRUE;
}

static bool32 AiJointRuntime_PreparePairGlobals(void)
{
    if (sAiJointRuntime == NULL)
        return FALSE;
    if (sAiJointRuntime->pairGlobalsPrepared)
        return TRUE;
    for (u32 i = 0; i < ARRAY_COUNT(sAiJointRuntime->actors); i++)
    {
        enum BattlerId actor = sAiJointRuntime->actors[i];

        if (!AiJointRuntime_CanApplyStoredAction(actor,
                                                 AiJointRuntime_GetAction(actor)))
            return FALSE;
    }
    for (u32 i = 0; i < ARRAY_COUNT(sAiJointRuntime->actors); i++)
    {
        enum BattlerId actor = sAiJointRuntime->actors[i];

        if (!AiJointRuntime_ApplyStoredAction(actor,
                                              AiJointRuntime_GetAction(actor)))
            return FALSE;
    }
    sAiJointRuntime->pairGlobalsPrepared = TRUE;
    return TRUE;
}

bool32 BattleAiJointRuntime_ApplyAction(enum BattlerId battler)
{
    const struct AiSimAction *action = AiJointRuntime_GetAction(battler);

    if (!AiJointRuntime_PreparePairGlobals()
     || !AiJointRuntime_CanApplyStoredAction(battler, action))
        return FALSE;
    sAiJointRuntime->appliedMask |= 1u << battler;
    return TRUE;
}

void BattleAiJointRuntime_RestoreTraceDecision(enum BattlerId battler)
{
    const struct AiJointAtomicCandidate *atomic;

    if (sAiJointRuntime == NULL
     || sAiJointRuntime->state != AI_JOINT_RUNTIME_STATE_READY
     || sAiJointRuntime->planId == BATTLE_AI_TRACE_ID_NONE
     || !(sAiJointRuntime->appliedMask & (1u << battler)))
        return;

    BattleAI_ClearDecisionMetadata(battler);
    BattleAiTrace_SetBattlerDecision(battler, sAiJointRuntime->planId,
                                     sAiJointRuntime->traceChosenRank);
    atomic = AiJointRuntime_GetChosenAtomic(battler);
    if (atomic != NULL && atomic->readFlags != 0)
        gAiBattleData->decisionReason[battler] = AI_DECISION_REASON_KNOWN_COMMAND_ANSWER;
}

bool32 BattleAiJointRuntime_ReuseMove(enum BattlerId battler)
{
    const struct AiSimAction *action;

    if (sAiJointRuntime == NULL
     || !(sAiJointRuntime->appliedMask & (1u << battler)))
        return FALSE;
    action = AiJointRuntime_GetAction(battler);
    if (action == NULL || action->kind != AI_SIM_ACTION_MOVE)
        return FALSE;
    if (!AiJointRuntime_ApplyStoredAction(battler, action))
        return FALSE;
    sAiJointRuntime->reusedMoveMask |= 1u << battler;
    return TRUE;
}

#if TESTING
u32 Test_BattleAiJointRuntime_GetBuildCount(void)
{
    return sAiJointRuntime == NULL
         ? sAiJointRuntimeReleasedStats.buildCount
         : sAiJointRuntime->buildCount;
}

u32 Test_BattleAiJointRuntime_GetStepCount(void)
{
    return sAiJointRuntime == NULL
         ? sAiJointRuntimeReleasedStats.stepCount
         : sAiJointRuntime->stepCount;
}

u16 Test_BattleAiJointRuntime_GetPlanId(void)
{
    return sAiJointRuntime == NULL
         ? sAiJointRuntimeReleasedStats.planId
         : sAiJointRuntime->planId;
}

bool32 Test_BattleAiJointRuntime_IsAllocated(void)
{
    return sAiJointRuntime != NULL;
}

u32 Test_BattleAiJointRuntime_GetFutureGimmickMask(const struct AiSimContext *context,
                                                  const struct AiSimBoard *board,
                                                  enum BattlerId actor,
                                                  bool32 *hasUnresolved)
{
    u32 count;
    u32 gimmickMask = 0;

    if (hasUnresolved != NULL)
        *hasUnresolved = FALSE;
    if (context == NULL || board == NULL || actor >= context->battlersCount)
        return 0;
    count = AiJointRuntime_GenerateFuture(context, board, actor & BIT_SIDE,
                                          actor, 1, sAiJointRuntimeTestCandidates,
                                          ARRAY_COUNT(sAiJointRuntimeTestCandidates), NULL);
    for (u32 i = 0; i < count; i++)
    {
        const struct AiJointAtomicCandidate *candidate = &sAiJointRuntimeTestCandidates[i];

        if ((candidate->flags & AI_JOINT_ATOMIC_UNRESOLVED) && hasUnresolved != NULL)
            *hasUnresolved = TRUE;
        if (candidate->action.kind == AI_SIM_ACTION_MOVE
         && candidate->action.gimmick > GIMMICK_NONE
         && candidate->action.gimmick < GIMMICKS_COUNT)
            gimmickMask |= 1u << candidate->action.gimmick;
    }
    return gimmickMask;
}

bool32 Test_BattleAiJointRuntime_IsFutureGimmickAvailable(const struct AiSimContext *context,
                                                         const struct AiSimBoard *board,
                                                         u32 rosterIndex,
                                                         u32 gimmick)
{
    return AiJointRuntime_IsGimmickAvailable(context, board, rosterIndex, gimmick);
}

void Test_BattleAiJointRuntime_SetLimits(u16 depth3Nodes, u16 totalNodes,
                                        u16 depth3Frames, u16 totalFrames)
{
    memset(&sAiJointRuntimeTestLimits, 0, sizeof(sAiJointRuntimeTestLimits));
    sAiJointRuntimeTestLimits.depth3NodeBudget = depth3Nodes;
    sAiJointRuntimeTestLimits.totalNodeBudget = totalNodes;
    sAiJointRuntimeTestLimits.depth3FrameBudget = depth3Frames;
    sAiJointRuntimeTestLimits.totalFrameBudget = totalFrames;
    sAiJointRuntimeTestLimits.extensionScoreWindow = AI_JOINT_DEFAULT_CLOSE_SCORE;
    sAiJointRuntimeTestLimits.maxExtensionRoots = AI_JOINT_MAX_EXTENSION_ROOTS;
    sAiJointRuntimeHasTestLimits = TRUE;
    if (sAiJointRuntime != NULL)
    {
        sAiJointRuntime->limitOverride = sAiJointRuntimeTestLimits;
        sAiJointRuntime->hasLimitOverride = TRUE;
    }
}

void Test_BattleAiJointRuntime_ClearLimits(void)
{
    memset(&sAiJointRuntimeTestLimits, 0, sizeof(sAiJointRuntimeTestLimits));
    sAiJointRuntimeHasTestLimits = FALSE;
    if (sAiJointRuntime != NULL)
    {
        memset(&sAiJointRuntime->limitOverride, 0, sizeof(sAiJointRuntime->limitOverride));
        sAiJointRuntime->hasLimitOverride = FALSE;
    }
}

#endif
