#include "global.h"
#include "battle.h"
#include "battle_ai_joint_planner.h"
#include "move.h"
#include "constants/battle.h"
#include "constants/hold_effects.h"
#include "constants/items.h"

#define AI_JOINT_SCORE_INFINITY 0x3FFFFFFF
#define AI_JOINT_FRAME_ROOT 0
#define AI_JOINT_SCORE_RISK_DIVISOR 4
#define AI_JOINT_TERMINAL_UTILITY 12000
#define AI_JOINT_PRESSURE_MIN_KO_FAST 100
#define AI_JOINT_PRESSURE_MIN_KO_SLOW 50
#define AI_JOINT_PRESSURE_MEDIAN_KO_FAST 50
#define AI_JOINT_PRESSURE_MEDIAN_KO_SLOW 25
#define AI_JOINT_PRESSURE_SPEED_CONTROL 20
#define AI_JOINT_PRESSURE_COVERAGE 25
#define AI_JOINT_PRESSURE_MEANINGFUL_PERCENT 20
#define AI_JOINT_PRESSURE_COVERAGE_PERCENT 50

enum AiJointPrivatePhase
{
    AI_JOINT_PHASE_DEPTH1,
    AI_JOINT_PHASE_DEPTH3,
    AI_JOINT_PHASE_SELECT_EXTENSION,
    AI_JOINT_PHASE_DEPTH5,
    AI_JOINT_PHASE_FINISH,
};

enum AiJointPrivateFrameState
{
    AI_JOINT_FRAME_INIT,
    AI_JOINT_FRAME_MAX_START,
    AI_JOINT_FRAME_MIN_START,
    AI_JOINT_FRAME_OUTCOME,
    AI_JOINT_FRAME_WAIT_CHILD,
    AI_JOINT_FRAME_COMPLETE,
    AI_JOINT_FRAME_FAILED,
};

enum AiJointPrivateCacheFlags
{
    AI_JOINT_CACHE_VALID = 1 << 0,
};

static s16 ClampScore16(s32 value)
{
    if (value > 32767)
        return 32767;
    if (value < -32768)
        return -32768;
    return value;
}

static s16 AddPrior(s16 left, s16 right)
{
    return ClampScore16((s32)left + right);
}

static bool32 SameAction(const struct AiSimAction *left, const struct AiSimAction *right)
{
    return left->choice == right->choice
        && left->actor == right->actor
        && left->target == right->target
        && left->kind == right->kind
        && left->moveSlot == right->moveSlot
        && left->gimmick == right->gimmick
        && left->allyInteractionKind == right->allyInteractionKind
        && left->replacementRosterIndex == right->replacementRosterIndex
        && left->flags == right->flags;
}

static s32 CompareActions(const struct AiSimAction *left, const struct AiSimAction *right)
{
    if (left->actor != right->actor)
        return left->actor < right->actor ? -1 : 1;
    if (left->kind != right->kind)
        return left->kind < right->kind ? -1 : 1;
    if (left->choice != right->choice)
        return left->choice < right->choice ? -1 : 1;
    if (left->target != right->target)
        return left->target < right->target ? -1 : 1;
    if (left->moveSlot != right->moveSlot)
        return left->moveSlot < right->moveSlot ? -1 : 1;
    if (left->gimmick != right->gimmick)
        return left->gimmick < right->gimmick ? -1 : 1;
    if (left->allyInteractionKind != right->allyInteractionKind)
        return left->allyInteractionKind < right->allyInteractionKind ? -1 : 1;
    if (left->replacementRosterIndex != right->replacementRosterIndex)
        return left->replacementRosterIndex < right->replacementRosterIndex ? -1 : 1;
    if (left->flags != right->flags)
        return left->flags < right->flags ? -1 : 1;
    return 0;
}

static u32 GetReservedRosterIndex(const struct AiSimAction *action)
{
    if (action->kind == AI_SIM_ACTION_SWITCH)
        return action->choice;
    if (action->kind == AI_SIM_ACTION_MOVE
     && (action->flags & AI_SIM_ACTION_FORCED_REPLACEMENT))
        return action->replacementRosterIndex;
    return AI_SIM_ROSTER_NONE;
}

static s32 CompareAtomic(const struct AiJointAtomicCandidate *left,
                         const struct AiJointAtomicCandidate *right)
{
    s32 actionComparison;

    if (left->legacyPrior != right->legacyPrior)
        return left->legacyPrior > right->legacyPrior ? -1 : 1;
    if ((left->flags & AI_JOINT_ATOMIC_GUARANTEED_KO)
     != (right->flags & AI_JOINT_ATOMIC_GUARANTEED_KO))
        return left->flags & AI_JOINT_ATOMIC_GUARANTEED_KO ? -1 : 1;
    if (left->stableKey != right->stableKey)
        return left->stableKey < right->stableKey ? -1 : 1;
    actionComparison = CompareActions(&left->action, &right->action);
    if (actionComparison != 0)
        return actionComparison;
    if (left->family != right->family)
        return left->family < right->family ? -1 : 1;
    if (left->effectiveMove != right->effectiveMove)
        return left->effectiveMove < right->effectiveMove ? -1 : 1;
    if (left->effectKey != right->effectKey)
        return left->effectKey < right->effectKey ? -1 : 1;
    if (left->resourceFlags != right->resourceFlags)
        return left->resourceFlags < right->resourceFlags ? -1 : 1;
    return 0;
}

static bool32 SameAtomic(const struct AiJointAtomicCandidate *left,
                         const struct AiJointAtomicCandidate *right)
{
    return left->stableKey == right->stableKey
        && left->effectiveMove == right->effectiveMove
        && left->effectKey == right->effectKey
        && left->resourceFlags == right->resourceFlags
        && SameAction(&left->action, &right->action);
}

static void NormalizeAtomic(struct AiJointAtomicCandidate *candidate)
{
    candidate->reserved = 0;
    if (candidate->family >= AI_JOINT_FAMILY_COUNT)
        candidate->family = AI_JOINT_FAMILY_OTHER;
}

u32 AiJoint_RetainAtomicCandidates(const struct AiJointAtomicCandidate *input,
                                   u32 inputCount,
                                   struct AiJointAtomicCandidate *output,
                                   u32 capacity)
{
    u8 ranked[AI_JOINT_MAX_ATOMIC_INPUT];
    bool8 selected[AI_JOINT_MAX_ATOMIC_INPUT] = {0};
    bool8 familyKept[AI_JOINT_FAMILY_COUNT] = {0};
    u32 rankedCount = 0;
    u32 outputCount = 0;
    u32 i;

    if (input == NULL || output == NULL || capacity == 0)
        return 0;
    inputCount = min(inputCount, AI_JOINT_MAX_ATOMIC_INPUT);
    capacity = min(capacity, AI_JOINT_MAX_ATOMIC_PER_ACTOR);

    for (i = 0; i < inputCount; i++)
    {
        u32 insertAt;
        u32 existing;

        if (!(input[i].flags & AI_JOINT_ATOMIC_VALID)
         || input[i].legalityFlags != AI_JOINT_LEGAL)
            continue;
        for (existing = 0; existing < rankedCount; existing++)
        {
            if (SameAtomic(&input[i], &input[ranked[existing]]))
                break;
        }
        if (existing != rankedCount)
            continue;
        insertAt = rankedCount;
        while (insertAt != 0
            && CompareAtomic(&input[i], &input[ranked[insertAt - 1]]) < 0)
        {
            ranked[insertAt] = ranked[insertAt - 1];
            insertAt--;
        }
        ranked[insertAt] = i;
        rankedCount++;
    }

    // First retain the strongest member of every represented family. Scanning
    // the global stable order makes overflow deterministic when >capacity
    // families are present.
    for (i = 0; i < rankedCount && outputCount < capacity; i++)
    {
        u32 inputIndex = ranked[i];
        u32 family = input[inputIndex].family;

        if (family >= AI_JOINT_FAMILY_COUNT)
            family = AI_JOINT_FAMILY_OTHER;
        if (familyKept[family])
            continue;
        output[outputCount] = input[inputIndex];
        NormalizeAtomic(&output[outputCount]);
        selected[i] = TRUE;
        familyKept[family] = TRUE;
        outputCount++;
    }

    // Fill remaining slots by score without disturbing the family guarantee.
    for (i = 0; i < rankedCount && outputCount < capacity; i++)
    {
        if (selected[i])
            continue;
        output[outputCount] = input[ranked[i]];
        NormalizeAtomic(&output[outputCount]);
        outputCount++;
    }

    // Consumers get one canonical ranking regardless of the two-pass keep.
    for (i = 1; i < outputCount; i++)
    {
        struct AiJointAtomicCandidate candidate = output[i];
        u32 insertAt = i;

        while (insertAt != 0 && CompareAtomic(&candidate, &output[insertAt - 1]) < 0)
        {
            output[insertAt] = output[insertAt - 1];
            insertAt--;
        }
        output[insertAt] = candidate;
    }
    return outputCount;
}

enum AiJointPairConflict AiJoint_GetPairConflict(const struct AiSimContext *context,
                                                  const struct AiJointAtomicCandidate *left,
                                                  const struct AiJointAtomicCandidate *right)
{
    const u16 sharedGimmicks = AI_JOINT_RESOURCE_MEGA
                             | AI_JOINT_RESOURCE_ULTRA
                             | AI_JOINT_RESOURCE_Z_MOVE
                             | AI_JOINT_RESOURCE_DYNAMAX
                             | AI_JOINT_RESOURCE_TERA;

    if (left == NULL || right == NULL
     || !(left->flags & AI_JOINT_ATOMIC_VALID)
     || !(right->flags & AI_JOINT_ATOMIC_VALID)
     || left->legalityFlags != AI_JOINT_LEGAL
     || right->legalityFlags != AI_JOINT_LEGAL
     || (left->action.kind != AI_SIM_ACTION_NONE
      && right->action.kind != AI_SIM_ACTION_NONE
      && left->action.actor == right->action.actor))
        return AI_JOINT_CONFLICT_ILLEGAL_ACTION;
    if (GetReservedRosterIndex(&left->action) != AI_SIM_ROSTER_NONE
     && GetReservedRosterIndex(&left->action) == GetReservedRosterIndex(&right->action))
        return AI_JOINT_CONFLICT_SAME_RESERVE;
    if ((left->resourceFlags & right->resourceFlags & sharedGimmicks) != 0)
    {
        u32 leftActor = left->action.actor;
        u32 rightActor = right->action.actor;
        if (context == NULL || leftActor >= MAX_BATTLERS_COUNT || rightActor >= MAX_BATTLERS_COUNT
         || (context->gimmickShareMask[leftActor] & (1u << rightActor)))
            return AI_JOINT_CONFLICT_SHARED_GIMMICK;
    }
    if (((left->flags & AI_JOINT_ATOMIC_REQUIRES_PARTNER_ACTION)
      && (right->flags & AI_JOINT_ATOMIC_PREVENTS_PARTNER_ACTION))
     || ((right->flags & AI_JOINT_ATOMIC_REQUIRES_PARTNER_ACTION)
      && (left->flags & AI_JOINT_ATOMIC_PREVENTS_PARTNER_ACTION)))
        return AI_JOINT_CONFLICT_PARTNER_REQUIREMENT;
    if ((left->flags & AI_JOINT_ATOMIC_FIELD_SETUP)
     && (right->flags & AI_JOINT_ATOMIC_FIELD_SETUP)
     && left->effectKey != 0 && left->effectKey == right->effectKey)
        return AI_JOINT_CONFLICT_DUPLICATE_FIELD_SETUP;
    return AI_JOINT_CONFLICT_NONE;
}

static u16 HashPairKey(const struct AiJointAtomicCandidate *left,
                       const struct AiJointAtomicCandidate *right)
{
    u32 hash = 2166136261u;
    const struct AiJointAtomicCandidate *ordered[2] = {left, right};
    u32 i;

    if (CompareActions(&left->action, &right->action) > 0)
    {
        ordered[0] = right;
        ordered[1] = left;
    }
    for (i = 0; i < 2; i++)
    {
        hash = (hash ^ ordered[i]->stableKey) * 16777619u;
        hash = (hash ^ ordered[i]->action.choice) * 16777619u;
        hash = (hash ^ ordered[i]->action.actor) * 16777619u;
        hash = (hash ^ ordered[i]->action.target) * 16777619u;
        hash = (hash ^ ordered[i]->action.gimmick) * 16777619u;
        hash = (hash ^ ordered[i]->action.replacementRosterIndex) * 16777619u;
        hash = (hash ^ ordered[i]->action.flags) * 16777619u;
    }
    return (u16)(hash ^ (hash >> 16));
}

static struct AiJointPairCandidate MakePair(const struct AiJointAtomicCandidate *left,
                                            u32 leftIndex,
                                            const struct AiJointAtomicCandidate *right,
                                            u32 rightIndex)
{
    struct AiJointPairCandidate pair = {0};
    bool32 swap = CompareActions(&left->action, &right->action) > 0;
    const struct AiJointAtomicCandidate *first = swap ? right : left;
    const struct AiJointAtomicCandidate *second = swap ? left : right;

    pair.actions[0] = first->action;
    pair.actions[1] = second->action;
    pair.families[0] = first->family;
    pair.families[1] = second->family;
    pair.sourceIndices[0] = swap ? rightIndex : leftIndex;
    pair.sourceIndices[1] = swap ? leftIndex : rightIndex;
    pair.legacyPrior = AddPrior(left->legacyPrior, right->legacyPrior);
    pair.stableKey = HashPairKey(left, right);
    pair.flags = AI_JOINT_PAIR_VALID;
    pair.rejectionFlags[0] = min(UINT8_MAX, first->rejectionFlags);
    pair.rejectionFlags[1] = min(UINT8_MAX, second->rejectionFlags);
    if ((left->flags | right->flags) & AI_JOINT_ATOMIC_UNRESOLVED)
        pair.flags |= AI_JOINT_PAIR_UNRESOLVED;
    if ((left->flags & right->flags & AI_JOINT_ATOMIC_GUARANTEED_KO)
     && (left->koTargetMask & right->koTargetMask) != 0)
        pair.flags |= AI_JOINT_PAIR_OVERKILL;
    return pair;
}

static s32 ComparePairs(const struct AiJointPairCandidate *left,
                        const struct AiJointPairCandidate *right)
{
    s32 actionComparison;

    if (left->legacyPrior != right->legacyPrior)
        return left->legacyPrior > right->legacyPrior ? -1 : 1;
    if ((left->flags & AI_JOINT_PAIR_OVERKILL) != (right->flags & AI_JOINT_PAIR_OVERKILL))
        return left->flags & AI_JOINT_PAIR_OVERKILL ? 1 : -1;
    if (left->stableKey != right->stableKey)
        return left->stableKey < right->stableKey ? -1 : 1;
    actionComparison = CompareActions(&left->actions[0], &right->actions[0]);
    if (actionComparison != 0)
        return actionComparison;
    return CompareActions(&left->actions[1], &right->actions[1]);
}

static bool32 SamePair(const struct AiJointPairCandidate *left,
                       const struct AiJointPairCandidate *right)
{
    return SameAction(&left->actions[0], &right->actions[0])
        && SameAction(&left->actions[1], &right->actions[1]);
}

static u16 PairFamilyKey(const struct AiJointPairCandidate *pair)
{
    return pair->families[0] * AI_JOINT_FAMILY_COUNT + pair->families[1];
}

static void InsertPairWinner(struct AiJointPairCandidate *pairs,
                             u32 *count,
                             u32 capacity,
                             const struct AiJointPairCandidate *candidate)
{
    u32 i;
    u32 worst;
    u16 familyKey = PairFamilyKey(candidate);

    for (i = 0; i < *count; i++)
    {
        if (PairFamilyKey(&pairs[i]) != familyKey)
            continue;
        if (ComparePairs(candidate, &pairs[i]) < 0)
            pairs[i] = *candidate;
        return;
    }
    if (*count < capacity)
    {
        pairs[(*count)++] = *candidate;
        return;
    }
    worst = 0;
    for (i = 1; i < *count; i++)
    {
        if (ComparePairs(&pairs[i], &pairs[worst]) > 0)
            worst = i;
    }
    if (ComparePairs(candidate, &pairs[worst]) < 0)
        pairs[worst] = *candidate;
}

u32 AiJoint_BuildPairs(const struct AiSimContext *context,
                       const struct AiJointAtomicCandidate *left,
                       u32 leftCount,
                       const struct AiJointAtomicCandidate *right,
                       u32 rightCount,
                       struct AiJointPairCandidate *pairs,
                       u32 capacity)
{
    u32 pairCount = 0;
    u32 i;
    u32 j;

    if (left == NULL || right == NULL || pairs == NULL || capacity == 0)
        return 0;
    leftCount = min(leftCount, AI_JOINT_MAX_ATOMIC_PER_ACTOR);
    rightCount = min(rightCount, AI_JOINT_MAX_ATOMIC_PER_ACTOR);
    capacity = min(capacity, AI_JOINT_MAX_ROOT_PAIRS);

    // Pass one reserves the strongest legal pair in every family pairing.
    for (i = 0; i < leftCount; i++)
    {
        for (j = 0; j < rightCount; j++)
        {
            struct AiJointPairCandidate candidate;
            if (AiJoint_GetPairConflict(context, &left[i], &right[j]) != AI_JOINT_CONFLICT_NONE)
                continue;
            candidate = MakePair(&left[i], i, &right[j], j);
            InsertPairWinner(pairs, &pairCount, capacity, &candidate);
        }
    }

    // If family winners did not fill K, add the best remaining exact pairs.
    if (pairCount < capacity)
    {
        for (i = 0; i < leftCount; i++)
        {
            for (j = 0; j < rightCount; j++)
            {
                struct AiJointPairCandidate candidate;
                u32 existing;
                u32 worst;

                if (AiJoint_GetPairConflict(context, &left[i], &right[j]) != AI_JOINT_CONFLICT_NONE)
                    continue;
                candidate = MakePair(&left[i], i, &right[j], j);
                for (existing = 0; existing < pairCount; existing++)
                {
                    if (SamePair(&candidate, &pairs[existing]))
                        break;
                }
                if (existing != pairCount)
                    continue;
                if (pairCount < capacity)
                {
                    pairs[pairCount++] = candidate;
                    continue;
                }
                worst = 0;
                for (existing = 1; existing < pairCount; existing++)
                {
                    if (ComparePairs(&pairs[existing], &pairs[worst]) > 0)
                        worst = existing;
                }
                if (ComparePairs(&candidate, &pairs[worst]) < 0)
                    pairs[worst] = candidate;
            }
        }
    }

    for (i = 1; i < pairCount; i++)
    {
        struct AiJointPairCandidate candidate = pairs[i];
        u32 insertAt = i;
        while (insertAt != 0 && ComparePairs(&candidate, &pairs[insertAt - 1]) < 0)
        {
            pairs[insertAt] = pairs[insertAt - 1];
            insertAt--;
        }
        pairs[insertAt] = candidate;
    }
    return pairCount;
}

static u32 GetRosterSide(const struct AiSimContext *context, u32 rosterIndex)
{
    u32 battler;
    u32 trainer;

    if (rosterIndex >= AI_SIM_ROSTER_COUNT)
        return B_SIDE_PLAYER;
    trainer = context->mons[rosterIndex].trainer;
    for (battler = 0; battler < context->battlersCount; battler++)
    {
        if (context->battlerTrainer[battler] == trainer)
            return battler & BIT_SIDE;
    }
    return trainer & BIT_SIDE;
}

static u32 GetBoardMaxHp(const struct AiSimContext *context,
                         const struct AiSimBoard *board,
                         u32 rosterIndex)
{
    const struct AiSimMonTemplate *mon;
    u32 battler;
    u32 maxHp;

    if (rosterIndex >= AI_SIM_ROSTER_COUNT)
        return 1;
    mon = &context->mons[rosterIndex];
    // Persistent Mega and Ultra forms are captured wholly in transformed;
    // their normal profile is cleared.  Continue using that transformed max
    // HP after the Pokemon returns to the bench, where there is no active flag
    // from which to recover the profile choice.
    if (!(mon->normal.flags & AI_SIM_PROFILE_VALID)
     && (mon->transformed.flags & AI_SIM_PROFILE_VALID))
        maxHp = max(1, mon->transformed.maxHp);
    else
        maxHp = max(1, mon->normal.maxHp);
    for (battler = 0; battler < context->battlersCount; battler++)
    {
        if (!(board->activeMask & (1u << battler))
         || board->active[battler].rosterIndex != rosterIndex)
            continue;
        if (board->active[battler].flags & AI_SIM_ACTIVE_TRANSFORMED
         && mon->transformed.flags & AI_SIM_PROFILE_VALID)
            maxHp = max(1, mon->transformed.maxHp);
        if (board->active[battler].flags & AI_SIM_ACTIVE_DYNAMAX)
            maxHp = maxHp * max(100, mon->dynamaxHpPercent) / 100;
        break;
    }
    return maxHp;
}

static s32 GetHpPercent(const struct AiSimContext *context,
                        const struct AiSimBoard *board,
                        u32 rosterIndex)
{
    return min(100, (u32)board->party[rosterIndex].hp * 100
                          / GetBoardMaxHp(context, board, rosterIndex));
}

static bool32 EvaluationSideHasLivingMon(const struct AiSimContext *context,
                                         const struct AiSimBoard *board,
                                         u32 side)
{
    u32 rosterIndex;

    for (rosterIndex = 0; rosterIndex < AI_SIM_ROSTER_COUNT; rosterIndex++)
    {
        if ((context->mons[rosterIndex].flags & AI_SIM_MON_PRESENT)
         && GetRosterSide(context, rosterIndex) == side
         && board->party[rosterIndex].hp != 0)
            return TRUE;
    }
    return FALSE;
}

static bool32 EvaluationBattlerIsAlive(const struct AiSimBoard *board,
                                       u32 battler)
{
    u32 rosterIndex;

    if (battler >= MAX_BATTLERS_COUNT
     || !(board->activeMask & (1u << battler))
     || (board->absentMask & (1u << battler)))
        return FALSE;
    rosterIndex = board->active[battler].rosterIndex;
    return rosterIndex < AI_SIM_ROSTER_COUNT
        && board->party[rosterIndex].hp != 0;
}

static const struct AiSimCombatProfile *GetEvaluationProfile(
    const struct AiSimContext *context,
    const struct AiSimBoard *board,
    u32 battler)
{
    u32 rosterIndex;
    const struct AiSimMonTemplate *mon;

    if (!EvaluationBattlerIsAlive(board, battler))
        return NULL;
    rosterIndex = board->active[battler].rosterIndex;
    mon = &context->mons[rosterIndex];
    if (board->active[battler].flags & AI_SIM_ACTIVE_TRANSFORMED)
        return (mon->transformed.flags & AI_SIM_PROFILE_VALID)
             ? &mon->transformed : NULL;
    return (mon->normal.flags & AI_SIM_PROFILE_VALID) ? &mon->normal : NULL;
}

static bool32 PressureMoveCanTargetFoe(u32 move)
{
    if (move == MOVE_NONE || move >= MOVES_COUNT_ALL
     || gMovesInfo[move].power == 0
     || gMovesInfo[move].category == DAMAGE_CATEGORY_STATUS)
        return FALSE;

    switch (gMovesInfo[move].target)
    {
    case TARGET_SELECTED:
    case TARGET_OPPONENT:
    case TARGET_BOTH:
    case TARGET_FOES_AND_ALLY:
    case TARGET_ALL_BATTLERS:
        return TRUE;
    default:
        return FALSE;
    }
}

static bool32 PressureMovesLast(const struct AiSimContext *context,
                                const struct AiSimBoard *board,
                                u32 battler)
{
    const struct AiSimCombatProfile *profile =
        GetEvaluationProfile(context, board, battler);
    u32 rosterIndex;
    u32 item;

    if (profile == NULL)
        return FALSE;
    rosterIndex = board->active[battler].rosterIndex;
    item = board->party[rosterIndex].item;
    return profile->ability == ABILITY_STALL
        || (item != ITEM_NONE && item < ITEMS_COUNT
         && gItemsInfo[item].holdEffect == HOLD_EFFECT_LAGGING_TAIL);
}

static bool32 PressureActorMovesFirst(const struct AiSimContext *context,
                                      const struct AiSimBoard *board,
                                      u32 actor,
                                      u32 target,
                                      u32 move)
{
    s32 priority = GetMovePriority(move);
    bool32 actorLast;
    bool32 targetLast;
    u32 actorSpeed;
    u32 targetSpeed;

    // Pressure compares the represented attack with an ordinary priority-zero
    // reply. Priority brackets resolve before Stall/Lagging Tail and Speed.
    if (priority != 0)
        return priority > 0;
    actorLast = PressureMovesLast(context, board, actor);
    targetLast = PressureMovesLast(context, board, target);
    if (actorLast != targetLast)
        return !actorLast;
    actorSpeed = AiSim_GetEffectiveSpeed(context, board, actor);
    targetSpeed = AiSim_GetEffectiveSpeed(context, board, target);
    if (actorSpeed != targetSpeed)
    {
        if (board->fieldStatuses & STATUS_FIELD_TRICK_ROOM)
            return actorSpeed < targetSpeed;
        return actorSpeed > targetSpeed;
    }
    if (context->tieRank[actor] != context->tieRank[target])
        return context->tieRank[actor] < context->tieRank[target];
    return actor < target;
}

static bool32 PressureMoveIsUsable(const struct AiSimContext *context,
                                   const struct AiSimBoard *board,
                                   u32 actor,
                                   u32 moveSlot)
{
    u32 rosterIndex = board->active[actor].rosterIndex;
    u32 move;

    if (rosterIndex >= AI_SIM_ROSTER_COUNT || moveSlot >= MAX_MON_MOVES)
        return FALSE;
    move = context->mons[rosterIndex].moves[moveSlot];
    if (!PressureMoveCanTargetFoe(move)
     || board->party[rosterIndex].pp[moveSlot] == 0
     || (board->active[actor].disabledMoveMask & (1u << moveSlot))
     || (board->active[actor].choiceMoveSlot != AI_SIM_MOVE_SLOT_NONE
      && board->active[actor].choiceMoveSlot != moveSlot)
     || (gMovesInfo[move].accuracy != 0 && gMovesInfo[move].accuracy != 100)
     || board->active[actor].statStages[STAT_ACC] != DEFAULT_STAT_STAGE
     || board->active[actor].chargingMove != MOVE_NONE
     || (board->active[actor].volatileFlags & AI_SIM_VOLATILE_GEOMANCY_CHARGING)
     || (board->active[actor].flags & AI_SIM_ACTIVE_DYNAMAX)
     || (board->party[rosterIndex].status1
       & (STATUS1_SLEEP | STATUS1_PARALYSIS | STATUS1_ICY_ANY)))
        return FALSE;
    return TRUE;
}

static s32 EvaluateSidePressure(const struct AiSimContext *context,
                                const struct AiSimBoard *board,
                                u32 side)
{
    s32 pressure = 0;
    u32 coveredTargets = 0;
    u32 target;

    for (target = 0; target < context->battlersCount; target++)
    {
        u32 targetRoster;
        u32 targetMaxHp;
        u32 targetHp;
        u32 bestValue = 0;
        u32 bestDamagePercent = 0;
        u32 actor;

        if (!EvaluationBattlerIsAlive(board, target)
         || (target & BIT_SIDE) == side)
            continue;
        targetRoster = board->active[target].rosterIndex;
        targetMaxHp = GetBoardMaxHp(context, board, targetRoster);
        targetHp = board->party[targetRoster].hp;

        for (actor = 0; actor < context->battlersCount; actor++)
        {
            u32 moveSlot;

            if (!EvaluationBattlerIsAlive(board, actor)
             || (actor & BIT_SIDE) != side)
                continue;
            for (moveSlot = 0; moveSlot < MAX_MON_MOVES; moveSlot++)
            {
                struct AiSimAction action = {0};
                struct SimulatedDamage damage;
                u32 damagePercent;
                u32 value;
                bool32 movesFirst;
                bool32 minimumKo;
                bool32 medianKo;
                bool32 meaningful;

                if (!PressureMoveIsUsable(context, board, actor, moveSlot))
                    continue;
                action.actor = actor;
                action.target = target;
                action.kind = AI_SIM_ACTION_MOVE;
                action.choice = context->mons[board->active[actor].rosterIndex].moves[moveSlot];
                action.moveSlot = moveSlot;
                action.replacementRosterIndex = AI_SIM_ROSTER_NONE;
                // ProjectDamage performs the complete read-only legality and
                // rejection check. Avoid repeating that work for every leaf.
                if (!AiSim_ProjectDamage(context, board, &action, target, &damage))
                    continue;

                damagePercent = min(100, (u32)damage.median * 100 / max(1, targetMaxHp));
                value = damagePercent;
                minimumKo = damage.minimum >= targetHp;
                medianKo = !minimumKo && damage.median >= targetHp;
                meaningful = damagePercent >= AI_JOINT_PRESSURE_MEANINGFUL_PERCENT;
                // Order is irrelevant to low-pressure, non-KO damage. Avoid
                // repeated item/profile/speed work at every quiet search leaf.
                movesFirst = (minimumKo || medianKo || meaningful)
                           && PressureActorMovesFirst(context, board, actor, target,
                                                      action.choice);
                if (minimumKo)
                    value += movesFirst ? AI_JOINT_PRESSURE_MIN_KO_FAST
                                        : AI_JOINT_PRESSURE_MIN_KO_SLOW;
                else if (medianKo)
                    value += movesFirst ? AI_JOINT_PRESSURE_MEDIAN_KO_FAST
                                        : AI_JOINT_PRESSURE_MEDIAN_KO_SLOW;
                if (movesFirst && meaningful)
                    value += AI_JOINT_PRESSURE_SPEED_CONTROL;
                if (value > bestValue)
                {
                    bestValue = value;
                    bestDamagePercent = damagePercent;
                }
            }
        }
        pressure += bestValue;
        if (bestDamagePercent >= AI_JOINT_PRESSURE_COVERAGE_PERCENT)
            coveredTargets++;
    }
    if (coveredTargets >= 2)
        pressure += AI_JOINT_PRESSURE_COVERAGE;
    return pressure;
}

s32 AiJoint_CalculateBoardPressure(const struct AiSimContext *context,
                                   const struct AiSimBoard *board,
                                   u8 aiSide)
{
    if (context == NULL || board == NULL || aiSide >= NUM_BATTLE_SIDES)
        return 0;
    return EvaluateSidePressure(context, board, aiSide)
         - EvaluateSidePressure(context, board, aiSide ^ BIT_SIDE);
}

static bool32 EvaluateBoardWithPressure(const struct AiSimContext *context,
                                        const struct AiSimBoard *rootBoard,
                                        const struct AiSimBoard *board,
                                        u8 aiSide,
                                        bool32 hasRootPressure,
                                        s32 rootPressure,
                                        struct AiJointScore *score)
{
    s32 immediate = 0;
    s32 future = 0;
    s32 risk = 0;
    s32 resource = 0;
    u32 rosterIndex;
    u32 side;
    bool32 aiAlive;
    bool32 foeAlive;

    if (context == NULL || rootBoard == NULL || board == NULL || score == NULL
     || aiSide >= NUM_BATTLE_SIDES)
        return FALSE;

    for (rosterIndex = 0; rosterIndex < AI_SIM_ROSTER_COUNT; rosterIndex++)
    {
        s32 rootPercent;
        s32 boardPercent;
        s32 sign;

        if (!(context->mons[rosterIndex].flags & AI_SIM_MON_PRESENT))
            continue;
        sign = GetRosterSide(context, rosterIndex) == aiSide ? -1 : 1;
        rootPercent = GetHpPercent(context, rootBoard, rosterIndex);
        boardPercent = GetHpPercent(context, board, rosterIndex);
        immediate += sign * (rootPercent - boardPercent) * 2;
        if (rootBoard->party[rosterIndex].hp != 0 && board->party[rosterIndex].hp == 0)
            immediate += sign * 300;
        if (GetRosterSide(context, rosterIndex) == aiSide
         && board->party[rosterIndex].hp != 0 && boardPercent <= 25)
            risk += 25 - boardPercent;
        if (rootBoard->party[rosterIndex].item != ITEM_NONE
         && board->party[rosterIndex].item == ITEM_NONE)
        {
            s32 itemValue = 4;

            if (board->party[rosterIndex].flags & AI_SIM_PARTY_ITEM_REMOVED)
                itemValue = 16;
            else if (board->party[rosterIndex].flags & AI_SIM_PARTY_ITEM_CONSUMED)
                itemValue = 0;
            resource += sign * itemValue;
        }
    }

    aiAlive = EvaluationSideHasLivingMon(context, board, aiSide);
    foeAlive = EvaluationSideHasLivingMon(context, board, aiSide ^ BIT_SIDE);
    if (!aiAlive && !foeAlive)
    {
        // A simultaneous final KO is a draw regardless of the path's prior HP
        // asymmetry or which recoil source happened to resolve last.
        *score = (struct AiJointScore){0};
        return TRUE;
    }
    if (!aiAlive || !foeAlive)
    {
        if (aiAlive && !foeAlive)
            immediate += AI_JOINT_TERMINAL_UTILITY;
        else if (!aiAlive && foeAlive)
            immediate -= AI_JOINT_TERMINAL_UTILITY;
        score->immediate = ClampScore16(immediate);
        score->future = 0;
        score->risk = ClampScore16(max(0, risk));
        score->resource = ClampScore16(resource);
        score->total = (s32)score->immediate - score->risk + score->resource;
        return TRUE;
    }

    if (!hasRootPressure)
        rootPressure = AiJoint_CalculateBoardPressure(context, rootBoard, aiSide);
    future += AiJoint_CalculateBoardPressure(context, board, aiSide) - rootPressure;

    for (side = 0; side < NUM_BATTLE_SIDES; side++)
    {
        s32 sign = side == aiSide ? 1 : -1;
        future += sign * (board->sides[side].tailwindTimer
                        - rootBoard->sides[side].tailwindTimer) * 10;
        future += sign * (board->sides[side].reflectTimer
                        - rootBoard->sides[side].reflectTimer) * 3;
        future += sign * (board->sides[side].lightScreenTimer
                        - rootBoard->sides[side].lightScreenTimer) * 3;
        future += sign * (board->sides[side].auroraVeilTimer
                        - rootBoard->sides[side].auroraVeilTimer) * 4;
    }
    for (side = 0; side < MAX_BATTLE_TRAINERS; side++)
    {
        u32 battlerSide = GetRosterSide(context, side * PARTY_SIZE);
        u32 newlyUsed = board->trainerGimmickUsed[side]
                      & ~rootBoard->trainerGimmickUsed[side];
        if (newlyUsed != 0)
            resource += battlerSide == aiSide ? -12 : 8;
    }

    score->immediate = ClampScore16(immediate);
    score->future = ClampScore16(future);
    score->risk = ClampScore16(max(0, risk));
    score->resource = ClampScore16(resource);
    score->total = (s32)score->immediate + score->future - score->risk + score->resource;
    return TRUE;
}

bool32 AiJoint_EvaluateBoard(const struct AiSimContext *context,
                             const struct AiSimBoard *rootBoard,
                             const struct AiSimBoard *board,
                             u8 aiSide,
                             struct AiJointScore *score)
{
    return EvaluateBoardWithPressure(context, rootBoard, board, aiSide,
                                     FALSE, 0, score);
}

bool32 AiJoint_EvaluateBoardWithRootPressure(const struct AiSimContext *context,
                                             const struct AiSimBoard *rootBoard,
                                             const struct AiSimBoard *board,
                                             u8 aiSide,
                                             s32 rootPressure,
                                             struct AiJointScore *score)
{
    return EvaluateBoardWithPressure(context, rootBoard, board, aiSide,
                                     TRUE, rootPressure, score);
}

static s32 CompareScores(const struct AiJointScore *left,
                         const struct AiJointScore *right)
{
    if (left->total != right->total)
        return left->total > right->total ? -1 : 1;
    if (left->immediate != right->immediate)
        return left->immediate > right->immediate ? -1 : 1;
    if (left->future != right->future)
        return left->future > right->future ? -1 : 1;
    if (left->risk != right->risk)
        return left->risk < right->risk ? -1 : 1;
    if (left->resource != right->resource)
        return left->resource > right->resource ? -1 : 1;
    return 0;
}

static struct AiJointScore MaximumScore(void)
{
    struct AiJointScore score = {0};
    score.total = AI_JOINT_SCORE_INFINITY;
    score.immediate = 32767;
    score.future = 32767;
    score.risk = -32768;
    score.resource = 32767;
    return score;
}

static struct AiJointScore MinimumScore(void)
{
    struct AiJointScore score = {0};
    score.total = -AI_JOINT_SCORE_INFINITY;
    score.immediate = -32768;
    score.future = -32768;
    score.risk = 32767;
    score.resource = -32768;
    return score;
}

static bool32 EvaluateBoard(const struct AiJointPlannerJob *job,
                            const struct AiSimBoard *board,
                            struct AiJointScore *score)
{
    if (job->request.evaluateBoard != NULL)
    {
        return job->request.evaluateBoard(job->request.context,
                                          job->request.rootBoard,
                                          board,
                                          job->request.aiSide,
                                          score,
                                          job->request.callbackData);
    }
    return AiJoint_EvaluateBoard(job->request.context,
                                 job->request.rootBoard,
                                 board,
                                 job->request.aiSide,
                                 score);
}

static bool32 IsRosterAlive(const struct AiSimContext *context,
                            const struct AiSimBoard *board,
                            u32 rosterIndex)
{
    return rosterIndex < AI_SIM_ROSTER_COUNT
        && (context->mons[rosterIndex].flags & AI_SIM_MON_PRESENT)
        && board->party[rosterIndex].hp != 0;
}

static bool32 SideHasLivingMon(const struct AiSimContext *context,
                               const struct AiSimBoard *board,
                               u32 side)
{
    u32 rosterIndex;

    for (rosterIndex = 0; rosterIndex < AI_SIM_ROSTER_COUNT; rosterIndex++)
    {
        if (GetRosterSide(context, rosterIndex) == side
         && IsRosterAlive(context, board, rosterIndex))
            return TRUE;
    }
    return FALSE;
}

static bool32 BoardIsTerminal(const struct AiJointPlannerJob *job,
                              const struct AiSimBoard *board)
{
    return !SideHasLivingMon(job->request.context, board, job->request.aiSide)
        || !SideHasLivingMon(job->request.context, board, job->request.aiSide ^ BIT_SIDE);
}

static u32 GetSideActors(const struct AiJointPlannerJob *job,
                         const struct AiSimBoard *board,
                         u32 side,
                         u8 actors[2])
{
    u32 count = 0;
    u32 battler;

    for (battler = 0; battler < job->request.context->battlersCount && count < 2; battler++)
    {
        if (!(board->activeMask & (1u << battler)) || (battler & BIT_SIDE) != side)
            continue;
        if ((board->active[battler].flags & AI_SIM_ACTIVE_NEEDS_REPLACEMENT)
         && !AiSim_ReplacementSlotWillBeFilled(job->request.context, board,
                                               battler))
            continue;
        actors[count++] = battler;
    }
    return count;
}

static struct AiJointAtomicCandidate MakePassCandidate(u32 actor)
{
    struct AiJointAtomicCandidate candidate = {0};
    candidate.action.actor = actor;
    candidate.action.kind = AI_SIM_ACTION_NONE;
    candidate.action.moveSlot = AI_SIM_MOVE_SLOT_NONE;
    candidate.stableKey = 0xFFFF;
    candidate.family = AI_JOINT_FAMILY_NONE;
    candidate.flags = AI_JOINT_ATOMIC_VALID;
    return candidate;
}

static u32 GenerateActorCandidates(struct AiJointPlannerJob *job,
                                   const struct AiSimBoard *board,
                                   u32 side,
                                   u32 actor,
                                   u32 turnIndex,
                                   struct AiJointAtomicCandidate *retained)
{
    struct AiJointAtomicCandidate *raw;
    u32 rawCount;
    u32 filteredCount = 0;
    u32 i;

    if (job->request.generateAtomic == NULL)
        return 0;
    // pairScratch is exactly 32 * 28 bytes and is idle while providers run.
    raw = (struct AiJointAtomicCandidate *)job->workspace->pairScratch;
    rawCount = job->request.generateAtomic(job->request.context,
                                           board,
                                           side,
                                           actor,
                                           turnIndex,
                                           raw,
                                           AI_JOINT_MAX_ATOMIC_INPUT,
                                           job->request.callbackData);
    rawCount = min(rawCount, AI_JOINT_MAX_ATOMIC_INPUT);
    for (i = 0; i < rawCount; i++)
    {
        if (raw[i].action.actor != actor)
            continue;
        raw[filteredCount++] = raw[i];
    }
    return AiJoint_RetainAtomicCandidates(raw,
                                          filteredCount,
                                          retained,
                                          AI_JOINT_MAX_ATOMIC_PER_ACTOR);
}

static u32 GeneratePairsForSide(struct AiJointPlannerJob *job,
                                const struct AiSimBoard *board,
                                u32 side,
                                u32 turnIndex,
                                u32 capacity)
{
    u8 actors[2] = {MAX_BATTLERS_COUNT, MAX_BATTLERS_COUNT};
    struct AiJointAtomicCandidate pass;
    const struct AiJointAtomicCandidate *left;
    const struct AiJointAtomicCandidate *right;
    u32 actorCount = GetSideActors(job, board, side, actors);
    u32 leftCount;
    u32 rightCount;

    if (actorCount == 0)
        return 0;
    leftCount = GenerateActorCandidates(job, board, side, actors[0], turnIndex,
                                        job->workspace->retained[0]);
    if (leftCount == 0)
        return 0;
    left = job->workspace->retained[0];

    if (actorCount == 1)
    {
        pass = MakePassCandidate(actors[0] ^ BIT_FLANK);
        right = &pass;
        rightCount = 1;
    }
    else
    {
        rightCount = GenerateActorCandidates(job, board, side, actors[1], turnIndex,
                                             job->workspace->retained[1]);
        if (rightCount == 0)
            return 0;
        right = job->workspace->retained[1];
    }
    return AiJoint_BuildPairs(job->request.context,
                              left,
                              leftCount,
                              right,
                              rightCount,
                              job->workspace->pairScratch,
                              capacity);
}

static bool32 GetGeneratedPair(struct AiJointPlannerJob *job,
                               const struct AiSimBoard *board,
                               u32 side,
                               u32 turnIndex,
                               u32 capacity,
                               u32 expectedCount,
                               u32 index,
                               struct AiJointPairCandidate *pair)
{
    u32 count = GeneratePairsForSide(job, board, side, turnIndex, capacity);

    if (count != expectedCount || index >= count)
    {
        job->stats.terminationReason = AI_JOINT_TERMINATION_PROVIDER_CHANGED;
        return FALSE;
    }
    *pair = job->workspace->pairScratch[index];
    return TRUE;
}

static struct AiJointPairCandidate GetConfirmedPlayerPair(const struct AiJointPlannerJob *job)
{
    struct AiJointPairCandidate pair = {0};
    u32 i;
    u32 count = 0;

    pair.flags = AI_JOINT_PAIR_VALID;
    pair.stableKey = 0xFFFF;
    for (i = 0; i < job->request.confirmedPlayerActionCount && i < 2; i++)
    {
        const struct AiSimAction *action = &job->request.confirmedPlayerActions[i];
        if (action->kind == AI_SIM_ACTION_NONE)
            continue;
        if (count < 2)
            pair.actions[count++] = *action;
    }
    if (count == 2 && CompareActions(&pair.actions[0], &pair.actions[1]) > 0)
    {
        struct AiSimAction swap = pair.actions[0];
        pair.actions[0] = pair.actions[1];
        pair.actions[1] = swap;
    }
    return pair;
}

static bool32 AddPairToTurn(struct AiSimJointTurn *turn,
                            const struct AiJointPairCandidate *pair,
                            bool32 confirmedPlayer)
{
    u32 i;

    for (i = 0; i < 2; i++)
    {
        const struct AiSimAction *action = &pair->actions[i];
        if (action->kind == AI_SIM_ACTION_NONE)
            continue;
        if (action->actor >= MAX_BATTLERS_COUNT
         || (turn->actionMask & (1u << action->actor)))
            return FALSE;
        turn->actions[action->actor] = *action;
        turn->actionMask |= 1u << action->actor;
        if (confirmedPlayer)
            turn->confirmedPlayerMask |= 1u << action->actor;
    }
    return TRUE;
}

static bool32 ComposeTurn(const struct AiJointPairCandidate *aiPair,
                          const struct AiJointPairCandidate *playerPair,
                          bool32 playerConfirmed,
                          struct AiSimJointTurn *turn)
{
    *turn = (struct AiSimJointTurn){0};
    turn->flags = AI_SIM_JOINT_CONSERVATIVE_ENEMY_TIES;
    return AddPairToTurn(turn, aiPair, FALSE)
        && AddPairToTurn(turn, playerPair, playerConfirmed);
}

static u32 EnumerateOutcomes(const struct AiJointPlannerJob *job,
                             const struct AiSimBoard *board,
                             const struct AiSimJointTurn *turn,
                             struct AiSimOutcomeKey *outcomes,
                             u32 capacity)
{
    if (job->request.enumerateOutcomes != NULL)
    {
        return job->request.enumerateOutcomes(job->request.context,
                                              board,
                                              turn,
                                              outcomes,
                                              capacity,
                                              job->request.callbackData);
    }
    return AiSim_EnumerateOutcomes(job->request.context, board, turn, outcomes, capacity);
}

static enum AiSimApplyStatus ApplyTurn(const struct AiJointPlannerJob *job,
                                       const struct AiSimBoard *before,
                                       const struct AiSimJointTurn *turn,
                                       const struct AiSimOutcomeKey *outcome,
                                       struct AiSimBoard *after,
                                       struct AiSimTurnResult *result)
{
    if (job->request.applyTurn != NULL)
    {
        return job->request.applyTurn(job->request.context,
                                      before,
                                      turn,
                                      outcome,
                                      after,
                                      result,
                                      job->request.callbackData);
    }
    return AiSim_ApplyJointTurn(job->request.context, before, turn, outcome, after, result);
}

static u32 HashBytes(u32 hash, const void *data, u32 size)
{
    const u8 *bytes = data;
    while (size-- != 0)
        hash = (hash ^ *bytes++) * 16777619u;
    return hash;
}

static u32 HashSearchBoard(const struct AiJointPlannerJob *job,
                           const struct AiSimBoard *board,
                           u32 remainingDepth)
{
    u32 hash = 2166136261u ^ job->request.cacheKeySalt;
    hash = HashBytes(hash, board, sizeof(*board));
    hash = (hash ^ remainingDepth) * 16777619u;
    hash = (hash ^ job->request.aiSide) * 16777619u;
    hash = (hash ^ (job->searchTargetDepth == AI_JOINT_EXTENSION_DEPTH
                  ? AI_JOINT_DEPTH5_BEAM_WIDTH
                  : AI_JOINT_DEPTH3_SEARCH_WIDTH)) * 16777619u;
    return hash == 0 ? 1 : hash;
}

static bool32 LookupTransposition(struct AiJointPlannerJob *job,
                                  const struct AiSimBoard *board,
                                  u32 remainingDepth,
                                  struct AiJointScore *score)
{
    u32 hash = HashSearchBoard(job, board, remainingDepth);
    struct AiJointTranspositionEntry *entry =
        &job->workspace->transposition[hash % AI_JOINT_TRANSPOSITION_ENTRIES];

    if (!(entry->flags & AI_JOINT_CACHE_VALID)
     || entry->hash != hash
     || entry->signature != (u16)(hash >> 16)
     || entry->remainingDepth != remainingDepth)
        return FALSE;
    *score = entry->score;
    job->stats.cacheHits++;
    return TRUE;
}

static void StoreTransposition(struct AiJointPlannerJob *job,
                               const struct AiSimBoard *board,
                               u32 remainingDepth,
                               const struct AiJointScore *score)
{
    u32 hash = HashSearchBoard(job, board, remainingDepth);
    struct AiJointTranspositionEntry *entry =
        &job->workspace->transposition[hash % AI_JOINT_TRANSPOSITION_ENTRIES];

    entry->hash = hash;
    entry->score = *score;
    entry->signature = hash >> 16;
    entry->remainingDepth = remainingDepth;
    entry->flags = AI_JOINT_CACHE_VALID;
}

static void BeginOutcomeAggregation(struct AiJointSearchFrame *frame)
{
    frame->outcomeWorst = MaximumScore();
    frame->outcomeWeightedTotal = 0;
    frame->outcomeWeightedImmediate = 0;
    frame->outcomeWeightedFuture = 0;
    frame->outcomeWeightedRisk = 0;
    frame->outcomeWeightedResource = 0;
    frame->outcomeWeight = 0;
    frame->responseUnsupported = FALSE;
}

static bool32 AddOutcomeScore(struct AiJointSearchFrame *frame,
                              const struct AiSimOutcomeKey *outcome,
                              const struct AiJointScore *score)
{
    u32 weight = outcome->probabilityWeight;

    // Custom providers which omit a weight remain deterministic and receive
    // one equal unit, matching the previous callback contract.
    if (weight == 0)
        weight = 1;
    if ((u64)frame->outcomeWeight + weight > UINT32_MAX)
        return FALSE;
    frame->outcomeWeightedTotal += (s64)score->total * weight;
    frame->outcomeWeightedImmediate += (s64)score->immediate * weight;
    frame->outcomeWeightedFuture += (s64)score->future * weight;
    frame->outcomeWeightedRisk += (s64)score->risk * weight;
    frame->outcomeWeightedResource += (s64)score->resource * weight;
    frame->outcomeWeight += weight;
    if (CompareScores(score, &frame->outcomeWorst) > 0)
        frame->outcomeWorst = *score;
    return TRUE;
}

static bool32 FinishOutcomeAggregation(struct AiJointSearchFrame *frame,
                                       struct AiJointScore *score)
{
    s32 spread;
    s32 varianceRisk;

    if (frame->outcomeWeight == 0)
        return FALSE;
    score->total = frame->outcomeWeightedTotal / frame->outcomeWeight;
    score->immediate = ClampScore16(frame->outcomeWeightedImmediate / frame->outcomeWeight);
    score->future = ClampScore16(frame->outcomeWeightedFuture / frame->outcomeWeight);
    score->risk = ClampScore16(frame->outcomeWeightedRisk / frame->outcomeWeight);
    score->resource = ClampScore16(frame->outcomeWeightedResource / frame->outcomeWeight);

    // Exact expected value handles repeat-Protect correctly. A bounded downside
    // penalty still distinguishes volatile lines without pretending that the
    // least favorable stochastic branch is guaranteed.
    spread = max(0, score->total - frame->outcomeWorst.total);
    varianceRisk = (spread + AI_JOINT_SCORE_RISK_DIVISOR - 1)
                 / AI_JOINT_SCORE_RISK_DIVISOR;
    score->risk = ClampScore16((s32)score->risk + varianceRisk);
    score->total -= varianceRisk;
    return TRUE;
}

static u32 GetSearchBeamWidth(const struct AiJointPlannerJob *job)
{
    return job->searchTargetDepth == AI_JOINT_EXTENSION_DEPTH
         ? AI_JOINT_DEPTH5_BEAM_WIDTH
         : AI_JOINT_DEPTH3_SEARCH_WIDTH;
}

static u32 GetNodeLimit(const struct AiJointPlannerJob *job)
{
    if (job->phase == AI_JOINT_PHASE_DEPTH5)
        return job->request.limits.totalNodeBudget;
    return job->request.limits.depth3NodeBudget;
}

static void CompleteFrame(struct AiJointPlannerJob *job, bool32 success)
{
    struct AiJointSearchFrame *frame = &job->workspace->frames[job->stackDepth];

    if (frame->unsupportedFlags != 0)
        job->stats.flags |= AI_JOINT_STATS_UNSUPPORTED_SEEN;
    job->returnedScore = frame->nodeBest;
    job->returnedUnsupportedFlags = success ? frame->bestUnsupportedFlags
                                            : frame->unsupportedFlags;
    job->childReturned = success;
    if (success && job->stackDepth != AI_JOINT_FRAME_ROOT)
    {
        StoreTransposition(job,
                           &job->workspace->boards[job->stackDepth],
                           frame->remainingDepth,
                           &frame->nodeBest);
    }
    if (job->stackDepth == AI_JOINT_FRAME_ROOT)
    {
        job->searchActive = FALSE;
        return;
    }
    job->stackDepth--;
}

static void PushChildFrame(struct AiJointPlannerJob *job,
                           u32 remainingDepth)
{
    struct AiJointSearchFrame *child;

    job->stackDepth++;
    child = &job->workspace->frames[job->stackDepth];
    *child = (struct AiJointSearchFrame){0};
    child->remainingDepth = remainingDepth;
    child->state = AI_JOINT_FRAME_INIT;
}

static bool32 NextSearchTransitionConsumesNode(const struct AiJointPlannerJob *job)
{
    const struct AiJointSearchFrame *frame = &job->workspace->frames[job->stackDepth];

    return frame->state == AI_JOINT_FRAME_OUTCOME
        && frame->outcomeIndex < frame->outcomeCount;
}

static void AbortSearchForUnsupportedOutcome(struct AiJointPlannerJob *job,
                                             u32 unsupportedFlags)
{
    job->stats.flags |= AI_JOINT_STATS_UNSUPPORTED_SEEN;
    job->returnedUnsupportedFlags = unsupportedFlags != 0
                                  ? unsupportedFlags
                                  : AI_SIM_UNSUPPORTED_OUTCOME;
    job->returnedScore = MinimumScore();
    job->childReturned = FALSE;
    job->searchActive = FALSE;
}

static bool32 ProcessSearch(struct AiJointPlannerJob *job, u32 nodeSlice)
{
    u32 startingNodes = job->stats.nodesVisited;
    u32 transitionGuard = 0;

    while (job->searchActive
        && (job->stats.nodesVisited - startingNodes < nodeSlice
         || !NextSearchTransitionConsumesNode(job))
        && transitionGuard++ < 100000)
    {
        struct AiJointSearchFrame *frame = &job->workspace->frames[job->stackDepth];
        struct AiSimBoard *board = &job->workspace->boards[job->stackDepth];
        u32 turnIndex = job->searchTargetDepth - frame->remainingDepth;

        switch (frame->state)
        {
        case AI_JOINT_FRAME_INIT:
            frame->nodeBest = MinimumScore();
            frame->bestUnsupportedFlags = 0;
            if (BoardIsTerminal(job, board))
            {
                if (EvaluateBoard(job, board, &frame->nodeBest))
                {
                    frame->supportedMaxPairs = 1;
                    frame->state = AI_JOINT_FRAME_COMPLETE;
                }
                else
                {
                    frame->unsupportedFlags |= AI_SIM_UNSUPPORTED_OUTCOME;
                    frame->state = AI_JOINT_FRAME_FAILED;
                }
                break;
            }
            if (job->stackDepth != AI_JOINT_FRAME_ROOT
             && LookupTransposition(job, board, frame->remainingDepth, &frame->nodeBest))
            {
                frame->supportedMaxPairs = 1;
                frame->state = AI_JOINT_FRAME_COMPLETE;
                break;
            }
            if (job->stackDepth == AI_JOINT_FRAME_ROOT)
                frame->maxCount = 1;
            else
            {
                frame->maxCount = GeneratePairsForSide(job,
                                                       board,
                                                       job->request.aiSide,
                                                       turnIndex,
                                                       GetSearchBeamWidth(job));
            }
            if (frame->maxCount == 0)
            {
                frame->unsupportedFlags |= AI_SIM_UNSUPPORTED_ACTION_KIND;
                frame->state = AI_JOINT_FRAME_FAILED;
            }
            else
                frame->state = AI_JOINT_FRAME_MAX_START;
            break;

        case AI_JOINT_FRAME_MAX_START:
            if (frame->maxIndex >= frame->maxCount)
            {
                frame->state = frame->supportedMaxPairs != 0
                             ? AI_JOINT_FRAME_COMPLETE
                             : AI_JOINT_FRAME_FAILED;
                break;
            }
            if (job->stackDepth == AI_JOINT_FRAME_ROOT)
                frame->currentMaxPair = job->workspace->rootPairs[job->currentRoot];
            else if (!GetGeneratedPair(job,
                                       board,
                                       job->request.aiSide,
                                       turnIndex,
                                       GetSearchBeamWidth(job),
                                       frame->maxCount,
                                       frame->maxIndex,
                                       &frame->currentMaxPair))
            {
                frame->unsupportedFlags |= AI_SIM_UNSUPPORTED_OUTCOME;
                frame->state = AI_JOINT_FRAME_FAILED;
                break;
            }
            frame->currentWorst = MaximumScore();
            frame->currentUnsupportedFlags = 0;
            frame->supportedResponses = 0;
            frame->maxUnsupported = FALSE;
            frame->minIndex = 0;
            if (frame->currentMaxPair.flags & (AI_JOINT_PAIR_UNRESOLVED
                                             | AI_JOINT_PAIR_UNSUPPORTED))
            {
                frame->currentUnsupportedFlags |= AI_SIM_UNSUPPORTED_OUTCOME;
                frame->unsupportedFlags |= frame->currentUnsupportedFlags;
                frame->state = AI_JOINT_FRAME_FAILED;
                break;
            }
            if (job->stackDepth == AI_JOINT_FRAME_ROOT
             && job->request.confirmedPlayerActionCount != 0)
                frame->minCount = 1;
            else
            {
                frame->minCount = GeneratePairsForSide(job,
                                                       board,
                                                       job->request.aiSide ^ BIT_SIDE,
                                                       turnIndex,
                                                       GetSearchBeamWidth(job));
            }
            if (frame->minCount == 0)
            {
                frame->maxUnsupported = TRUE;
                frame->currentUnsupportedFlags |= AI_SIM_UNSUPPORTED_ACTION_KIND;
            }
            frame->state = AI_JOINT_FRAME_MIN_START;
            break;

        case AI_JOINT_FRAME_MIN_START:
            if (frame->minIndex >= frame->minCount)
            {
                if (!frame->maxUnsupported && frame->supportedResponses != 0)
                {
                    if (frame->supportedMaxPairs == 0
                     || CompareScores(&frame->currentWorst, &frame->nodeBest) < 0)
                    {
                        frame->nodeBest = frame->currentWorst;
                        frame->bestUnsupportedFlags = frame->currentUnsupportedFlags;
                    }
                    frame->supportedMaxPairs++;
                }
                else
                {
                    frame->unsupportedFlags |= frame->currentUnsupportedFlags;
                    job->stats.prunedBranches++;
                }
                frame->maxIndex++;
                frame->state = AI_JOINT_FRAME_MAX_START;
                break;
            }
            if (job->stackDepth == AI_JOINT_FRAME_ROOT
             && job->request.confirmedPlayerActionCount != 0)
                frame->currentMinPair = GetConfirmedPlayerPair(job);
            else if (!GetGeneratedPair(job,
                                       board,
                                       job->request.aiSide ^ BIT_SIDE,
                                       turnIndex,
                                       GetSearchBeamWidth(job),
                                       frame->minCount,
                                       frame->minIndex,
                                       &frame->currentMinPair))
            {
                frame->maxUnsupported = TRUE;
                frame->currentUnsupportedFlags |= AI_SIM_UNSUPPORTED_OUTCOME;
                frame->minIndex = frame->minCount;
                break;
            }
            BeginOutcomeAggregation(frame);
            if (frame->currentMinPair.flags & (AI_JOINT_PAIR_UNRESOLVED
                                             | AI_JOINT_PAIR_UNSUPPORTED))
            {
                frame->responseUnsupported = TRUE;
                frame->currentUnsupportedFlags |= AI_SIM_UNSUPPORTED_OUTCOME;
                frame->outcomeCount = 0;
                frame->state = AI_JOINT_FRAME_OUTCOME;
                break;
            }
            {
                struct AiSimJointTurn turn;
                if (!ComposeTurn(&frame->currentMaxPair,
                                 &frame->currentMinPair,
                                 job->stackDepth == AI_JOINT_FRAME_ROOT
                              && job->request.confirmedPlayerActionCount != 0,
                                 &turn))
                {
                    frame->responseUnsupported = TRUE;
                    frame->currentUnsupportedFlags |= AI_SIM_UNSUPPORTED_OUTCOME;
                    frame->outcomeCount = 0;
                }
                else
                {
                    frame->outcomeCount = EnumerateOutcomes(job,
                                                            board,
                                                            &turn,
                                                            job->workspace->outcomes[job->stackDepth],
                                                            AI_SIM_MAX_OUTCOMES);
                    if (frame->outcomeCount == 0
                     || frame->outcomeCount > AI_SIM_MAX_OUTCOMES)
                    {
                        frame->responseUnsupported = TRUE;
                        frame->currentUnsupportedFlags |= AI_SIM_UNSUPPORTED_OUTCOME;
                        frame->unsupportedFlags |= frame->currentUnsupportedFlags;
                        frame->outcomeCount = 0;
                        AbortSearchForUnsupportedOutcome(job,
                                                         frame->currentUnsupportedFlags);
                        break;
                    }
                }
            }
            frame->outcomeIndex = 0;
            frame->state = AI_JOINT_FRAME_OUTCOME;
            break;

        case AI_JOINT_FRAME_OUTCOME:
            if (frame->outcomeIndex >= frame->outcomeCount)
            {
                struct AiJointScore responseScore;
                if (frame->responseUnsupported
                 || !FinishOutcomeAggregation(frame, &responseScore))
                {
                    frame->maxUnsupported = TRUE;
                    frame->currentUnsupportedFlags |= AI_SIM_UNSUPPORTED_OUTCOME;
                }
                else
                {
                    if (frame->supportedResponses == 0
                     || CompareScores(&responseScore, &frame->currentWorst) > 0)
                        frame->currentWorst = responseScore;
                    frame->supportedResponses++;
                }
                frame->minIndex++;
                frame->state = AI_JOINT_FRAME_MIN_START;
                break;
            }
            if (job->stats.nodesVisited >= GetNodeLimit(job))
                return FALSE;
            {
                struct AiSimJointTurn turn;
                enum AiSimApplyStatus status;
                struct AiSimBoard *after = &job->workspace->boards[job->stackDepth + 1];
                const struct AiSimOutcomeKey *outcome =
                    &job->workspace->outcomes[job->stackDepth][frame->outcomeIndex];

                if (!ComposeTurn(&frame->currentMaxPair,
                                 &frame->currentMinPair,
                                 job->stackDepth == AI_JOINT_FRAME_ROOT
                              && job->request.confirmedPlayerActionCount != 0,
                                 &turn))
                {
                    frame->responseUnsupported = TRUE;
                    frame->currentUnsupportedFlags |= AI_SIM_UNSUPPORTED_OUTCOME;
                    frame->outcomeIndex++;
                    break;
                }
                job->workspace->turnResult = (struct AiSimTurnResult){0};
                status = ApplyTurn(job,
                                   board,
                                   &turn,
                                   outcome,
                                   after,
                                   &job->workspace->turnResult);
                job->stats.nodesVisited++;
                if (status != AI_SIM_APPLY_OK)
                {
                    frame->responseUnsupported = TRUE;
                    frame->currentUnsupportedFlags |=
                        job->workspace->turnResult.unsupportedFlags != 0
                      ? job->workspace->turnResult.unsupportedFlags
                      : AI_SIM_UNSUPPORTED_OUTCOME;
                    frame->outcomeIndex++;
                    break;
                }
                if (frame->remainingDepth <= 1 || BoardIsTerminal(job, after))
                {
                    struct AiJointScore outcomeScore;
                    if (!EvaluateBoard(job, after, &outcomeScore)
                     || !AddOutcomeScore(frame, outcome, &outcomeScore))
                    {
                        frame->responseUnsupported = TRUE;
                        frame->currentUnsupportedFlags |= AI_SIM_UNSUPPORTED_OUTCOME;
                    }
                    frame->outcomeIndex++;
                }
                else
                {
                    frame->state = AI_JOINT_FRAME_WAIT_CHILD;
                    PushChildFrame(job, frame->remainingDepth - 1);
                }
            }
            break;

        case AI_JOINT_FRAME_WAIT_CHILD:
        {
            const struct AiSimOutcomeKey *outcome =
                &job->workspace->outcomes[job->stackDepth][frame->outcomeIndex];
            if (!job->childReturned
             || !AddOutcomeScore(frame, outcome, &job->returnedScore))
            {
                frame->responseUnsupported = TRUE;
                frame->currentUnsupportedFlags |= job->returnedUnsupportedFlags != 0
                    ? job->returnedUnsupportedFlags
                    : AI_SIM_UNSUPPORTED_OUTCOME;
            }
            frame->outcomeIndex++;
            frame->state = AI_JOINT_FRAME_OUTCOME;
            break;
        }

        case AI_JOINT_FRAME_COMPLETE:
            CompleteFrame(job, TRUE);
            break;

        case AI_JOINT_FRAME_FAILED:
            CompleteFrame(job, FALSE);
            break;
        }
    }
    return !job->searchActive;
}

static s32 CompareRootIndices(const struct AiJointPlannerJob *job, u32 left, u32 right)
{
    const struct AiJointRootResult *leftResult = &job->workspace->rootResults[left];
    const struct AiJointRootResult *rightResult = &job->workspace->rootResults[right];
    s32 comparison;

    if (leftResult->completedDepth != rightResult->completedDepth)
        return leftResult->completedDepth > rightResult->completedDepth ? -1 : 1;
    comparison = CompareScores(&leftResult->score, &rightResult->score);
    if (comparison != 0)
        return comparison;
    return ComparePairs(&job->workspace->rootPairs[left],
                        &job->workspace->rootPairs[right]);
}

static void RankRoots(struct AiJointPlannerJob *job)
{
    u32 i;

    for (i = 0; i < job->rootCount; i++)
        job->rankedRoots[i] = i;
    for (i = 1; i < job->rootCount; i++)
    {
        u8 root = job->rankedRoots[i];
        u32 insertAt = i;
        while (insertAt != 0
            && CompareRootIndices(job, root, job->rankedRoots[insertAt - 1]) < 0)
        {
            job->rankedRoots[insertAt] = job->rankedRoots[insertAt - 1];
            insertAt--;
        }
        job->rankedRoots[insertAt] = root;
    }
}

static void LaunchRootSearch(struct AiJointPlannerJob *job, u32 root, u32 depth)
{
    struct AiJointSearchFrame *frame;

    job->currentRoot = root;
    job->searchTargetDepth = depth;
    job->phaseStartNodes = job->stats.nodesVisited;
    job->stackDepth = AI_JOINT_FRAME_ROOT;
    job->searchActive = TRUE;
    job->childReturned = FALSE;
    job->workspace->boards[0] = *job->request.rootBoard;
    frame = &job->workspace->frames[0];
    *frame = (struct AiJointSearchFrame){0};
    frame->remainingDepth = depth;
    frame->state = AI_JOINT_FRAME_INIT;
}

static void StoreCompletedRootSearch(struct AiJointPlannerJob *job)
{
    struct AiJointRootResult *root = &job->workspace->rootResults[job->currentRoot];
    u32 usedNodes = job->stats.nodesVisited - job->phaseStartNodes;
    u32 depth = job->searchTargetDepth;

    root->nodesVisited = min(0xFFFF, (u32)root->nodesVisited + usedNodes);
    if (!job->childReturned)
    {
        // A deeper partial/unsupported attempt is evidence only. It must never
        // overwrite an already completed shallower result.
        if (root->completedDepth == 0)
        {
            root->unsupportedFlags = job->returnedUnsupportedFlags;
            root->flags |= AI_JOINT_ROOT_UNSUPPORTED;
        }
        return;
    }

    root->score = job->returnedScore;
    root->unsupportedFlags = job->returnedUnsupportedFlags;
    root->completedDepth = depth;
    root->flags &= ~AI_JOINT_ROOT_UNSUPPORTED;
    if (depth == 1)
    {
        root->flags |= AI_JOINT_ROOT_DEPTH1_COMPLETE;
        job->stats.depth1CompletedRoots++;
    }
    else if (depth == AI_JOINT_STANDARD_DEPTH)
    {
        root->flags |= AI_JOINT_ROOT_DEPTH3_COMPLETE;
        job->stats.depth3CompletedRoots++;
    }
    else if (depth == AI_JOINT_EXTENSION_DEPTH)
    {
        root->flags |= AI_JOINT_ROOT_DEPTH5_COMPLETE;
        job->stats.depth5CompletedRoots++;
    }
    job->stats.completedDepth = max(job->stats.completedDepth, depth);
}

static const struct AiJointPairCandidate *GetSelectedDepth3Pair(
    const struct AiJointPlannerJob *job,
    u32 selectionIndex)
{
    u32 root = job->selectedDepth3[selectionIndex];
    u32 pairIndex = job->workspace->rootResults[root].pairIndex;

    return &job->workspace->rootPairs[pairIndex];
}

static bool32 IsDepth3OffenseFamily(u32 family)
{
    return family == AI_JOINT_FAMILY_CLEAN_KO
        || family == AI_JOINT_FAMILY_CLEAN_DAMAGE
        || family == AI_JOINT_FAMILY_ALT_TARGET;
}

static bool32 PairHasDamagingAction(const struct AiJointPairCandidate *pair)
{
    for (u32 actor = 0; actor < ARRAY_COUNT(pair->actions); actor++)
    {
        const struct AiSimAction *action = &pair->actions[actor];

        if (action->kind == AI_SIM_ACTION_MOVE
         && action->choice != MOVE_NONE
         && action->choice < MOVES_COUNT_ALL
         && GetMovePower(action->choice) != 0)
            return TRUE;
    }
    return FALSE;
}

static bool32 IsDepth3TacticalFamily(u32 family)
{
    switch (family)
    {
    case AI_JOINT_FAMILY_PROTECT:
    case AI_JOINT_FAMILY_SWITCH:
    case AI_JOINT_FAMILY_SETUP:
    case AI_JOINT_FAMILY_DISRUPTION:
    case AI_JOINT_FAMILY_SPEED_FIELD:
    case AI_JOINT_FAMILY_SUPPORT:
    case AI_JOINT_FAMILY_APPROVED_ALLY:
    case AI_JOINT_FAMILY_GIMMICK:
        return TRUE;
    default:
        return FALSE;
    }
}

static void OrderSelectedDepth3ForAnytime(struct AiJointPlannerJob *job)
{
    bool8 baselineFamilies[AI_JOINT_FAMILY_COUNT] = {0};
    const struct AiJointPairCandidate *baseline;
    bool32 baselineHasOffense = FALSE;
    u32 promote;

    if (job->depth3SelectionCount < 2)
        return;

    baseline = GetSelectedDepth3Pair(job, 0);
    baselineHasOffense = PairHasDamagingAction(baseline);
    for (u32 actor = 0; actor < ARRAY_COUNT(baseline->families); actor++)
    {
        u32 family = baseline->families[actor];

        if (family < AI_JOINT_FAMILY_COUNT)
            baselineFamilies[family] = TRUE;
        if (IsDepth3OffenseFamily(family))
            baselineHasOffense = TRUE;
    }
    if (!baselineHasOffense)
        return;

    // The strongest shallow root remains first.  If it already supplies an
    // offensive line, search one already-selected, tactically novel root next
    // so an anytime cutoff compares distinct plans instead of two attacks.
    // This is a stable reorder only; it never changes depth-three membership.
    for (promote = 1; promote < job->depth3SelectionCount; promote++)
    {
        const struct AiJointPairCandidate *pair = GetSelectedDepth3Pair(job, promote);
        bool32 addsNovelTacticalFamily = FALSE;

        for (u32 actor = 0; actor < ARRAY_COUNT(pair->families); actor++)
        {
            u32 family = pair->families[actor];

            if (family < AI_JOINT_FAMILY_COUNT
             && IsDepth3TacticalFamily(family)
             && !baselineFamilies[family])
                addsNovelTacticalFamily = TRUE;
        }
        if (addsNovelTacticalFamily)
        {
            u8 root = job->selectedDepth3[promote];

            while (promote > 1)
            {
                job->selectedDepth3[promote] = job->selectedDepth3[promote - 1];
                promote--;
            }
            job->selectedDepth3[1] = root;
            break;
        }
    }
}

static void SelectDepth3Roots(struct AiJointPlannerJob *job)
{
    static const u8 sFamilyPriority[] =
    {
        AI_JOINT_FAMILY_CLEAN_KO,
        AI_JOINT_FAMILY_PROTECT,
        AI_JOINT_FAMILY_SETUP,
        AI_JOINT_FAMILY_SPEED_FIELD,
        AI_JOINT_FAMILY_DISRUPTION,
        AI_JOINT_FAMILY_SWITCH,
        AI_JOINT_FAMILY_SUPPORT,
        AI_JOINT_FAMILY_APPROVED_ALLY,
        AI_JOINT_FAMILY_GIMMICK,
        AI_JOINT_FAMILY_CLEAN_DAMAGE,
        AI_JOINT_FAMILY_ALT_TARGET,
        AI_JOINT_FAMILY_OTHER,
    };
    bool8 selected[AI_JOINT_MAX_ROOT_PAIRS] = {0};
    bool8 coveredFamily[AI_JOINT_FAMILY_COUNT] = {0};
    u32 i;

    RankRoots(job);
    job->depth3SelectionCount = 0;

    // Always preserve the strongest completed shallow root. The two remaining
    // standard-search slots then cover the highest-priority tactical families
    // that root does not already contain. This prevents several superficially
    // similar damage pairs from crowding a Power Herb setup, Protect, switch,
    // or other strategically distinct line out of the only real lookahead.
    for (i = 0; i < job->rootCount; i++)
    {
        u32 root = job->rankedRoots[i];
        const struct AiJointPairCandidate *pair;

        if (job->workspace->rootResults[root].completedDepth != 1)
            continue;
        job->selectedDepth3[job->depth3SelectionCount++] = root;
        selected[root] = TRUE;
        pair = &job->workspace->rootPairs[job->workspace->rootResults[root].pairIndex];
        for (u32 actor = 0; actor < ARRAY_COUNT(pair->families); actor++)
        {
            if (pair->families[actor] < AI_JOINT_FAMILY_COUNT)
                coveredFamily[pair->families[actor]] = TRUE;
        }
        break;
    }

    for (i = 0; i < ARRAY_COUNT(sFamilyPriority)
             && job->depth3SelectionCount < AI_JOINT_DEPTH3_SEARCH_WIDTH; i++)
    {
        u32 family = sFamilyPriority[i];
        u32 rank;

        if (coveredFamily[family])
            continue;
        for (rank = 0; rank < job->rootCount; rank++)
        {
            u32 root = job->rankedRoots[rank];
            const struct AiJointPairCandidate *pair;
            bool32 hasFamily = FALSE;

            if (selected[root]
             || job->workspace->rootResults[root].completedDepth != 1)
                continue;
            pair = &job->workspace->rootPairs[job->workspace->rootResults[root].pairIndex];
            for (u32 actor = 0; actor < ARRAY_COUNT(pair->families); actor++)
            {
                if (pair->families[actor] == family)
                    hasFamily = TRUE;
            }
            if (!hasFamily)
                continue;
            job->selectedDepth3[job->depth3SelectionCount++] = root;
            selected[root] = TRUE;
            for (u32 actor = 0; actor < ARRAY_COUNT(pair->families); actor++)
            {
                if (pair->families[actor] < AI_JOINT_FAMILY_COUNT)
                    coveredFamily[pair->families[actor]] = TRUE;
            }
            break;
        }
    }

    // If fewer than three distinct tactical families exist, fill the remaining
    // slots in exact depth-one score order.
    for (i = 0; i < job->rootCount
             && job->depth3SelectionCount < AI_JOINT_DEPTH3_SEARCH_WIDTH; i++)
    {
        u32 root = job->rankedRoots[i];

        if (selected[root]
         || job->workspace->rootResults[root].completedDepth != 1)
            continue;
        job->selectedDepth3[job->depth3SelectionCount++] = root;
        selected[root] = TRUE;
    }

    OrderSelectedDepth3ForAnytime(job);

    for (i = 0; i < job->rootCount; i++)
    {
        if (!selected[i])
            job->workspace->rootResults[i].flags |= AI_JOINT_ROOT_PRUNED;
    }
}

static bool32 PairWarrantsExtension(const struct AiJointPairCandidate *pair)
{
    u32 i;

    if (pair->flags & (AI_JOINT_PAIR_PROTECT_PARTNER_KO | AI_JOINT_PAIR_OVERKILL))
        return TRUE;
    for (i = 0; i < ARRAY_COUNT(pair->families); i++)
    {
        switch (pair->families[i])
        {
        case AI_JOINT_FAMILY_CLEAN_KO:
        case AI_JOINT_FAMILY_PROTECT:
        case AI_JOINT_FAMILY_SETUP:
        case AI_JOINT_FAMILY_DISRUPTION:
        case AI_JOINT_FAMILY_SPEED_FIELD:
        case AI_JOINT_FAMILY_GIMMICK:
            return TRUE;
        default:
            break;
        }
    }
    return FALSE;
}

static void SelectExtensionRoots(struct AiJointPlannerJob *job)
{
    s32 bestScore = -AI_JOINT_SCORE_INFINITY;
    u32 i;

    RankRoots(job);
    job->selectedExtensionCount = 0;
    for (i = 0; i < job->rootCount; i++)
    {
        u32 root = job->rankedRoots[i];
        const struct AiJointRootResult *rootResult = &job->workspace->rootResults[root];
        if (rootResult->completedDepth != AI_JOINT_STANDARD_DEPTH)
            continue;
        if (!PairWarrantsExtension(&job->workspace->rootPairs[root]))
            continue;
        if (bestScore == -AI_JOINT_SCORE_INFINITY)
            bestScore = rootResult->score.total;
        if (bestScore - rootResult->score.total > job->request.limits.extensionScoreWindow)
            continue;
        if (job->selectedExtensionCount >= job->request.limits.maxExtensionRoots)
            break;
        job->selectedExtensions[job->selectedExtensionCount++] = root;
        job->workspace->rootResults[root].flags |= AI_JOINT_ROOT_EXTENSION_SELECTED;
    }
    if (job->selectedExtensionCount != 0)
        job->stats.requestedDepth = AI_JOINT_EXTENSION_DEPTH;
}

static void BuildChosenAfterBoard(struct AiJointPlannerJob *job,
                                  u32 root,
                                  struct AiSimBoard *chosenAfter)
{
    struct AiJointPairCandidate playerPair;
    struct AiSimJointTurn turn;
    u32 outcomeCount;
    u32 outcomeIndex;
    u32 bestWeight = 0;
    bool32 found = FALSE;

    *chosenAfter = *job->request.rootBoard;
    if (job->request.confirmedPlayerActionCount == 0)
        return;
    playerPair = GetConfirmedPlayerPair(job);
    if (!ComposeTurn(&job->workspace->rootPairs[root], &playerPair, TRUE, &turn))
        return;
    outcomeCount = EnumerateOutcomes(job,
                                     job->request.rootBoard,
                                     &turn,
                                     job->workspace->outcomes[0],
                                     AI_SIM_MAX_OUTCOMES);
    if (outcomeCount == 0 || outcomeCount > AI_SIM_MAX_OUTCOMES)
        return;
    for (outcomeIndex = 0; outcomeIndex < outcomeCount; outcomeIndex++)
    {
        const struct AiSimOutcomeKey *outcome = &job->workspace->outcomes[0][outcomeIndex];
        struct AiSimBoard *candidate = &job->workspace->boards[1];
        struct AiSimTurnResult turnResult = {0};
        u32 weight = outcome->probabilityWeight == 0 ? 1 : outcome->probabilityWeight;

        if (ApplyTurn(job,
                      job->request.rootBoard,
                      &turn,
                      outcome,
                      candidate,
                      &turnResult) != AI_SIM_APPLY_OK)
            continue;
        if (!found || weight > bestWeight)
        {
            *chosenAfter = *candidate;
            bestWeight = weight;
            found = TRUE;
        }
    }
}

static void FillResult(struct AiJointPlannerJob *job, struct AiJointPlannerResult *result)
{
    u32 root;

    if (result == NULL)
        return;
    *result = (struct AiJointPlannerResult){0};
    RankRoots(job);
    result->stats = job->stats;
    result->state = job->state;
    result->rootCount = job->rootCount;
    result->chosenAfterBoard = *job->request.rootBoard;
    if (job->rootCount == 0)
        return;
    root = job->rankedRoots[0];
    if (job->workspace->rootResults[root].completedDepth == 0)
        return;
    result->chosenPair = job->workspace->rootPairs[root];
    result->score = job->workspace->rootResults[root].score;
    result->unsupportedFlags = job->workspace->rootResults[root].unsupportedFlags;
    if (job->stats.flags & AI_JOINT_STATS_UNSUPPORTED_SEEN)
    {
        for (u32 i = 0; i < job->rootCount; i++)
            result->unsupportedFlags |= job->workspace->rootResults[i].unsupportedFlags;
        if (result->unsupportedFlags == 0)
            result->unsupportedFlags = AI_SIM_UNSUPPORTED_OUTCOME;
    }
    result->completedDepth = job->workspace->rootResults[root].completedDepth;
    result->chosenRank = 0;
    if (job->state == AI_JOINT_JOB_READY)
        BuildChosenAfterBoard(job, root, &result->chosenAfterBoard);
}

static bool32 CandidateWasRetained(const struct AiJointAtomicCandidate *candidate,
                                   const struct AiJointAtomicCandidate *retained,
                                   u32 retainedCount)
{
    u32 i;
    for (i = 0; i < retainedCount; i++)
    {
        if (SameAtomic(candidate, &retained[i]))
            return TRUE;
    }
    return FALSE;
}

static void FindForcedRejectedCandidate(struct AiJointPlannerJob *job,
                                        const struct AiJointPlannerRequest *request,
                                        const u8 retainedCounts[2])
{
    u32 actorIndex;
    for (actorIndex = 0; actorIndex < 2; actorIndex++)
    {
        u32 i;
        u32 count = min(request->rootCandidateCounts[actorIndex], AI_JOINT_MAX_ATOMIC_INPUT);
        for (i = 0; i < count; i++)
        {
            const struct AiJointAtomicCandidate *candidate =
                &request->rootCandidates[actorIndex][i];
            if (CandidateWasRetained(candidate,
                                     job->workspace->retained[actorIndex],
                                     retainedCounts[actorIndex]))
                continue;
            if (!(candidate->flags & AI_JOINT_ATOMIC_VALID)
             && candidate->rejectionFlags == 0)
                continue;
            if (!job->hasForcedRejectedCandidate
             || CompareAtomic(candidate, &job->forcedRejectedCandidate) < 0)
            {
                job->forcedRejectedCandidate = *candidate;
                NormalizeAtomic(&job->forcedRejectedCandidate);
                job->hasForcedRejectedCandidate = TRUE;
            }
        }
    }
}

enum AiJointPlannerState AiJointPlanner_InitJob(struct AiJointPlannerJob *job,
                                                 struct AiJointPlannerWorkspace *workspace,
                                                 const struct AiJointPlannerRequest *request)
{
    u8 retainedCounts[2] = {0};
    u32 actorIndex;

    if (job == NULL || workspace == NULL || request == NULL
     || request->context == NULL || request->rootBoard == NULL
     || request->aiSide >= NUM_BATTLE_SIDES
     || request->confirmedPlayerActionCount > 2)
        return AI_JOINT_JOB_FALLBACK;
    *job = (struct AiJointPlannerJob){0};
    *workspace = (struct AiJointPlannerWorkspace){0};
    job->request = *request;
    job->workspace = workspace;
    if (job->request.limits.depth3NodeBudget == 0)
        job->request.limits.depth3NodeBudget = AI_JOINT_DEFAULT_DEPTH3_NODES;
    if (job->request.limits.totalNodeBudget < job->request.limits.depth3NodeBudget)
        job->request.limits.totalNodeBudget = AI_JOINT_DEFAULT_TOTAL_NODES;
    if (job->request.limits.depth3FrameBudget == 0)
        job->request.limits.depth3FrameBudget = AI_JOINT_DEFAULT_DEPTH3_FRAMES;
    if (job->request.limits.totalFrameBudget < job->request.limits.depth3FrameBudget)
        job->request.limits.totalFrameBudget = AI_JOINT_DEFAULT_TOTAL_FRAMES;
    if (job->request.limits.extensionScoreWindow == 0)
        job->request.limits.extensionScoreWindow = AI_JOINT_DEFAULT_CLOSE_SCORE;
    if (job->request.limits.maxExtensionRoots == 0
     || job->request.limits.maxExtensionRoots > AI_JOINT_MAX_EXTENSION_ROOTS)
        job->request.limits.maxExtensionRoots = AI_JOINT_MAX_EXTENSION_ROOTS;
    job->stats.nodeBudget = job->request.limits.totalNodeBudget;
    job->stats.requestedDepth = AI_JOINT_STANDARD_DEPTH;
    job->stats.terminationReason = AI_JOINT_TERMINATION_COMPLETE;

    for (actorIndex = 0; actorIndex < 2; actorIndex++)
    {
        struct AiJointAtomicCandidate *raw =
            (struct AiJointAtomicCandidate *)workspace->pairScratch;
        u32 count = min(request->rootCandidateCounts[actorIndex], AI_JOINT_MAX_ATOMIC_INPUT);
        u32 filtered = 0;
        u32 i;

        if (request->rootCandidates[actorIndex] == NULL)
            continue;
        for (i = 0; i < count; i++)
        {
            if (request->rootCandidates[actorIndex][i].action.actor
             != request->rootActors[actorIndex])
                continue;
            raw[filtered++] = request->rootCandidates[actorIndex][i];
        }
        retainedCounts[actorIndex] = AiJoint_RetainAtomicCandidates(
            raw, filtered, workspace->retained[actorIndex], AI_JOINT_MAX_ATOMIC_PER_ACTOR);
    }
    FindForcedRejectedCandidate(job, request, retainedCounts);
    job->rootCount = AiJoint_BuildPairs(request->context,
                                        workspace->retained[0],
                                        retainedCounts[0],
                                        workspace->retained[1],
                                        retainedCounts[1],
                                        workspace->rootPairs,
                                        AI_JOINT_MAX_ROOT_PAIRS);
    for (actorIndex = 0; actorIndex < job->rootCount; actorIndex++)
    {
        workspace->rootResults[actorIndex].pairIndex = actorIndex;
        workspace->rootResults[actorIndex].flags = AI_JOINT_ROOT_VALID;
        workspace->rootResults[actorIndex].score = MinimumScore();
        job->rankedRoots[actorIndex] = actorIndex;
    }
    if (job->rootCount == 0)
    {
        job->state = AI_JOINT_JOB_FALLBACK;
        job->stats.terminationReason = AI_JOINT_TERMINATION_NO_ROOT;
        return job->state;
    }
    job->state = AI_JOINT_JOB_BUILDING;
    job->phase = AI_JOINT_PHASE_DEPTH1;
    return job->state;
}

static void FinishJob(struct AiJointPlannerJob *job)
{
    RankRoots(job);
    job->searchActive = FALSE;
    if (job->stats.flags & AI_JOINT_STATS_UNSUPPORTED_SEEN)
    {
        job->state = AI_JOINT_JOB_FALLBACK;
        job->stats.terminationReason = AI_JOINT_TERMINATION_UNSUPPORTED;
    }
    else if (job->rootCount != 0
     && job->workspace->rootResults[job->rankedRoots[0]].completedDepth != 0)
        job->state = AI_JOINT_JOB_READY;
    else
    {
        job->state = AI_JOINT_JOB_FALLBACK;
        if (job->stats.terminationReason == AI_JOINT_TERMINATION_COMPLETE)
            job->stats.terminationReason = AI_JOINT_TERMINATION_UNSUPPORTED;
    }
}

enum AiJointPlannerState AiJointPlanner_Step(struct AiJointPlannerJob *job,
                                              u32 nodeSlice,
                                              struct AiJointPlannerResult *result)
{
    u32 startingNodes;

    if (job == NULL || job->workspace == NULL)
        return AI_JOINT_JOB_FALLBACK;
    if (job->state == AI_JOINT_JOB_READY || job->state == AI_JOINT_JOB_FALLBACK)
    {
        FillResult(job, result);
        return job->state;
    }
    if (nodeSlice == 0)
        nodeSlice = AI_JOINT_DEFAULT_STEP_NODES;
    job->stats.elapsedFrames++;
    if ((job->phase != AI_JOINT_PHASE_DEPTH5
      && job->stats.elapsedFrames > job->request.limits.depth3FrameBudget)
     || job->stats.elapsedFrames > job->request.limits.totalFrameBudget)
    {
        job->stats.terminationReason = AI_JOINT_TERMINATION_FRAME_BUDGET;
        FinishJob(job);
        FillResult(job, result);
        return job->state;
    }

    startingNodes = job->stats.nodesVisited;
    // A slice limits search nodes, not zero-node phase bookkeeping. If the
    // final node completes a root exactly at the boundary, drain transitions
    // through selection/finish now instead of spending another frame only to
    // discover that the completed result has crossed the frame deadline.
    while ((job->stats.nodesVisited - startingNodes < nodeSlice
         || !job->searchActive)
        && job->state != AI_JOINT_JOB_READY
        && job->state != AI_JOINT_JOB_FALLBACK)
    {
        if (job->searchActive)
        {
            u32 remainingSlice = nodeSlice - (job->stats.nodesVisited - startingNodes);
            ProcessSearch(job, remainingSlice);
            if (job->stats.terminationReason == AI_JOINT_TERMINATION_PROVIDER_CHANGED)
            {
                FinishJob(job);
                break;
            }
            if (job->searchActive)
            {
                if (job->stats.nodesVisited >= GetNodeLimit(job))
                {
                    job->stats.terminationReason = AI_JOINT_TERMINATION_NODE_BUDGET;
                    FinishJob(job);
                }
                break;
            }
            StoreCompletedRootSearch(job);
            // Unsupported is a whole-side integrity boundary.  Once any
            // retained search has proved it, no later completed root may be
            // selected, so continuing the remaining roots cannot affect the
            // exact result and only burns the frame budget.
            if (job->stats.flags & AI_JOINT_STATS_UNSUPPORTED_SEEN)
            {
                FinishJob(job);
                break;
            }
            job->phaseIndex++;
            continue;
        }

        switch (job->phase)
        {
        case AI_JOINT_PHASE_DEPTH1:
            if (job->phaseIndex < job->rootCount)
                LaunchRootSearch(job, job->phaseIndex, 1);
            else
            {
                SelectDepth3Roots(job);
                job->phase = AI_JOINT_PHASE_DEPTH3;
                job->phaseIndex = 0;
            }
            break;
        case AI_JOINT_PHASE_DEPTH3:
            if (job->phaseIndex < job->depth3SelectionCount)
                LaunchRootSearch(job, job->selectedDepth3[job->phaseIndex], AI_JOINT_STANDARD_DEPTH);
            else
            {
                job->state = AI_JOINT_JOB_DEPTH3_READY;
                job->phase = AI_JOINT_PHASE_SELECT_EXTENSION;
                job->phaseIndex = 0;
            }
            break;
        case AI_JOINT_PHASE_SELECT_EXTENSION:
            SelectExtensionRoots(job);
            if (job->selectedExtensionCount == 0)
                job->phase = AI_JOINT_PHASE_FINISH;
            else
            {
                job->state = AI_JOINT_JOB_EXTENDING5;
                job->phase = AI_JOINT_PHASE_DEPTH5;
                job->phaseIndex = 0;
            }
            break;
        case AI_JOINT_PHASE_DEPTH5:
            if (job->phaseIndex < job->selectedExtensionCount)
                LaunchRootSearch(job,
                                 job->selectedExtensions[job->phaseIndex],
                                 AI_JOINT_EXTENSION_DEPTH);
            else
                job->phase = AI_JOINT_PHASE_FINISH;
            break;
        case AI_JOINT_PHASE_FINISH:
            FinishJob(job);
            break;
        }
        if (job->stats.nodesVisited >= GetNodeLimit(job) && job->searchActive)
        {
            job->stats.terminationReason = AI_JOINT_TERMINATION_NODE_BUDGET;
            FinishJob(job);
        }
    }
    FillResult(job, result);
    return job->state;
}

enum AiJointPlannerState AiJointPlanner_Run(struct AiJointPlannerJob *job,
                                             struct AiJointPlannerWorkspace *workspace,
                                             const struct AiJointPlannerRequest *request,
                                             struct AiJointPlannerResult *result)
{
    enum AiJointPlannerState state = AiJointPlanner_InitJob(job, workspace, request);
    while (state != AI_JOINT_JOB_READY && state != AI_JOINT_JOB_FALLBACK)
        state = AiJointPlanner_Step(job, AI_JOINT_DEFAULT_STEP_NODES, result);
    FillResult(job, result);
    return state;
}

u32 AiJointPlanner_GetRootCount(const struct AiJointPlannerJob *job)
{
    return job == NULL ? 0 : job->rootCount;
}

bool32 AiJointPlanner_GetRankedRoot(const struct AiJointPlannerJob *job,
                                    u32 rank,
                                    struct AiJointRootSummary *summary)
{
    u32 root;
    if (job == NULL || summary == NULL || rank >= job->rootCount)
        return FALSE;
    root = job->rankedRoots[rank];
    *summary = (struct AiJointRootSummary){0};
    summary->pair = job->workspace->rootPairs[root];
    summary->score = job->workspace->rootResults[root].score;
    summary->unsupportedFlags = job->workspace->rootResults[root].unsupportedFlags;
    summary->completedDepth = job->workspace->rootResults[root].completedDepth;
    summary->flags = job->workspace->rootResults[root].flags;
    summary->rank = rank;
    return TRUE;
}

bool32 AiJointPlanner_GetForcedRejectedCandidate(const struct AiJointPlannerJob *job,
                                                  struct AiJointAtomicCandidate *candidate)
{
    if (job == NULL || candidate == NULL || !job->hasForcedRejectedCandidate)
        return FALSE;
    *candidate = job->forcedRejectedCandidate;
    return TRUE;
}
