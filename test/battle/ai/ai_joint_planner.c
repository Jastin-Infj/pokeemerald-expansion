#include "global.h"
#include "test/battle.h"
#include "battle.h"
#include "battle_ai_joint_planner.h"
#include "malloc.h"
#include "constants/battle.h"
#include "constants/species.h"

enum PlannerTestMode
{
    PLANNER_TEST_FIXED,
    PLANNER_TEST_MINIMAX,
    PLANNER_TEST_WEIGHTED,
    PLANNER_TEST_RANKED,
    PLANNER_TEST_PARTIAL,
    PLANNER_TEST_TIE,
    PLANNER_TEST_CACHE,
    PLANNER_TEST_UNRESOLVED_PLAYER,
    PLANNER_TEST_UNRESOLVED_AI,
    PLANNER_TEST_CAPACITY_OVERFLOW,
    PLANNER_TEST_MIXED_OUTCOME_OVERFLOW,
    PLANNER_TEST_FORCED_REPLACEMENT,
};

struct PlannerTestData
{
    u16 generateCalls;
    u16 enumerateCalls;
    u16 applyCalls;
    u8 mode;
    u8 forcedReplacementActorMask;
};

struct PlannerTestFixture
{
    struct AiSimContext context;
    struct AiSimBoard board;
    struct AiSimBoard scratchBoard;
    struct AiJointPlannerWorkspace workspace;
    struct AiJointPlannerJob job;
    struct AiJointPlannerRequest request;
    struct AiJointPlannerResult result;
    struct AiJointAtomicCandidate rootCandidates[2][8];
    struct PlannerTestData data;
};

static EWRAM_DATA struct PlannerTestFixture *sPlannerFixture;

#define sContext        (sPlannerFixture->context)
#define sBoard          (sPlannerFixture->board)
#define sScratchBoard   (sPlannerFixture->scratchBoard)
#define sWorkspace      (sPlannerFixture->workspace)
#define sJob            (sPlannerFixture->job)
#define sRequest        (sPlannerFixture->request)
#define sResult         (sPlannerFixture->result)
#define sRootCandidates (sPlannerFixture->rootCandidates)
#define sData           (sPlannerFixture->data)

static const u8 sRosterByBattler[MAX_BATTLERS_COUNT] = {0, 6, 1, 7};

static void FreePlannerFixture(void)
{
    TRY_FREE_AND_SET_NULL(sPlannerFixture);
}

static struct AiSimAction MakeAction(u32 actor, u32 choice)
{
    struct AiSimAction action = {0};

    action.choice = choice;
    action.actor = actor;
    action.target = actor ^ BIT_SIDE;
    action.kind = AI_SIM_ACTION_MOVE;
    action.moveSlot = 0;
    action.gimmick = GIMMICK_NONE;
    action.replacementRosterIndex = AI_SIM_ROSTER_NONE;
    action.flags = 0;
    return action;
}

static struct AiJointAtomicCandidate MakeCandidate(u32 actor,
                                                    u32 choice,
                                                    s32 prior,
                                                    u32 family,
                                                    u32 stableKey)
{
    struct AiJointAtomicCandidate candidate = {0};

    candidate.action = MakeAction(actor, choice);
    candidate.effectiveMove = choice;
    candidate.legacyPrior = prior;
    candidate.stableKey = stableKey;
    candidate.family = family;
    candidate.flags = AI_JOINT_ATOMIC_VALID;
    return candidate;
}

static void InitPlannerFixture(enum PlannerTestMode mode)
{
    u32 battler;

    if (sPlannerFixture == NULL)
        sPlannerFixture = AllocZeroed(sizeof(*sPlannerFixture));
    memset(sPlannerFixture, 0, sizeof(*sPlannerFixture));

    sData.mode = mode;
    sContext.battleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE;
    sContext.battlersCount = MAX_BATTLERS_COUNT;
    sContext.activeMask = 0xF;
    sBoard.activeMask = 0xF;
    // The callbacks encode signed utility around this midpoint.
    sBoard.stellarBoostFlags[0] = 0x8000;

    for (battler = 0; battler < MAX_BATTLERS_COUNT; battler++)
    {
        u32 rosterIndex = sRosterByBattler[battler];
        struct AiSimMonTemplate *mon = &sContext.mons[rosterIndex];

        sContext.battlerTrainer[battler] = battler & BIT_SIDE;
        sContext.tieRank[battler] = battler;
        sContext.gimmickShareMask[battler] = (battler & BIT_SIDE) == B_SIDE_PLAYER ? 0x5 : 0xA;
        mon->normal.species = SPECIES_WOBBUFFET;
        mon->normal.maxHp = 100;
        mon->normal.flags = AI_SIM_PROFILE_VALID;
        mon->trainer = battler & BIT_SIDE;
        mon->partyIndex = battler >> 1;
        mon->flags = AI_SIM_MON_PRESENT | AI_SIM_MON_PROFILE_KNOWN;
        sBoard.party[rosterIndex].hp = 100;
        sBoard.active[battler].rosterIndex = rosterIndex;
        sBoard.active[battler].choiceMoveSlot = AI_SIM_MOVE_SLOT_NONE;
    }
}

static void InitPressureFixture(void)
{
    u32 battler;

    InitPlannerFixture(PLANNER_TEST_FIXED);
    for (battler = 0; battler < MAX_BATTLERS_COUNT; battler++)
    {
        u32 rosterIndex = sRosterByBattler[battler];
        struct AiSimMonTemplate *mon = &sContext.mons[rosterIndex];
        struct AiSimCombatProfile *profile = &mon->normal;

        profile->maxHp = 200;
        profile->attack = 100;
        profile->defense = 100;
        profile->speed = 100;
        profile->spAttack = 100;
        profile->spDefense = 100;
        profile->ability = ABILITY_NONE;
        profile->types[0] = TYPE_NORMAL;
        profile->types[1] = TYPE_NONE;
        profile->types[2] = TYPE_NONE;
        profile->flags = AI_SIM_PROFILE_VALID
                       | AI_SIM_PROFILE_ABILITY_KNOWN
                       | AI_SIM_PROFILE_TYPES_KNOWN;
        mon->level = 50;
        mon->moves[0] = MOVE_PROTECT;
        mon->flags = AI_SIM_MON_PRESENT
                   | AI_SIM_MON_MOVES_KNOWN
                   | AI_SIM_MON_PROFILE_KNOWN;
        sBoard.party[rosterIndex].hp = 200;
        sBoard.party[rosterIndex].item = ITEM_NONE;
        sBoard.party[rosterIndex].pp[0] = 10;
        sBoard.party[rosterIndex].flags = AI_SIM_PARTY_ITEM_KNOWN
                                        | AI_SIM_PARTY_STATUS_KNOWN;
        for (u32 stat = 0; stat < NUM_BATTLE_STATS; stat++)
            sBoard.active[battler].statStages[stat] = DEFAULT_STAT_STAGE;
        sBoard.active[battler].choiceMoveSlot = AI_SIM_MOVE_SLOT_NONE;
    }
}

static u32 AddGeneratedCandidate(struct AiJointAtomicCandidate *candidates,
                                 u32 capacity,
                                 u32 count,
                                 u32 actor,
                                 u32 choice)
{
    if (count < capacity)
        candidates[count] = MakeCandidate(actor, choice, 0, AI_JOINT_FAMILY_OTHER,
                                          actor * 100 + choice);
    return count + 1;
}

static u32 GenerateAtomic(const struct AiSimContext *context,
                          const struct AiSimBoard *board,
                          u8 side,
                          enum BattlerId actor,
                          u8 turnIndex,
                          struct AiJointAtomicCandidate *candidates,
                          u32 capacity,
                          void *callbackData)
{
    struct PlannerTestData *data = callbackData;
    u32 count = 0;

    (void)context;
    (void)board;
    data->generateCalls++;

    if (data->mode == PLANNER_TEST_FORCED_REPLACEMENT)
    {
        count = AddGeneratedCandidate(candidates, capacity, count, actor,
                                      MOVE_TACKLE);
        if (board->active[actor].flags & AI_SIM_ACTIVE_NEEDS_REPLACEMENT)
        {
            data->forcedReplacementActorMask |= 1u << actor;
            candidates[0].action.flags |= AI_SIM_ACTION_FORCED_REPLACEMENT;
            candidates[0].action.replacementRosterIndex = 8;
            candidates[0].family = AI_JOINT_FAMILY_SWITCH;
        }
    }
    else if (data->mode == PLANNER_TEST_UNRESOLVED_PLAYER
     || data->mode == PLANNER_TEST_UNRESOLVED_AI)
    {
        bool32 unresolvedSide = (data->mode == PLANNER_TEST_UNRESOLVED_PLAYER
                              && side == B_SIDE_PLAYER)
                           || (data->mode == PLANNER_TEST_UNRESOLVED_AI
                              && side == B_SIDE_OPPONENT);

        count = AddGeneratedCandidate(candidates, capacity, count, actor,
                                      100 + actor);
        if (unresolvedSide && turnIndex != 0 && (actor & BIT_FLANK) == 0)
        {
            count = AddGeneratedCandidate(candidates, capacity, count, actor,
                                          200 + actor);
            if (count <= capacity)
            {
                candidates[count - 1].family = AI_JOINT_FAMILY_SETUP;
                candidates[count - 1].flags |= AI_JOINT_ATOMIC_UNRESOLVED;
                candidates[count - 1].legacyPrior = -1;
            }
        }
    }
    else if (data->mode == PLANNER_TEST_CAPACITY_OVERFLOW)
    {
        u32 i;

        for (i = 0; i < capacity; i++)
        {
            count = AddGeneratedCandidate(candidates, capacity, count, actor,
                                          300 + actor * 40 + i);
            candidates[i].legacyPrior = capacity - i;
        }
        return capacity + 7;
    }
    else if (data->mode == PLANNER_TEST_MINIMAX)
    {
        if (actor == 1)
        {
            count = AddGeneratedCandidate(candidates, capacity, count, actor, 10);
            count = AddGeneratedCandidate(candidates, capacity, count, actor, 20);
        }
        else if (actor == 0)
        {
            count = AddGeneratedCandidate(candidates, capacity, count, actor, 30);
            count = AddGeneratedCandidate(candidates, capacity, count, actor, 40);
        }
        else
        {
            count = AddGeneratedCandidate(candidates, capacity, count, actor,
                                          actor == 3 ? 21 : 41);
        }
    }
    else if (data->mode == PLANNER_TEST_CACHE)
    {
        if (actor == 1 || actor == 0)
        {
            count = AddGeneratedCandidate(candidates, capacity, count, actor,
                                          actor == 1 ? 10 : 30);
            count = AddGeneratedCandidate(candidates, capacity, count, actor,
                                          actor == 1 ? 20 : 40);
        }
        else
        {
            count = AddGeneratedCandidate(candidates, capacity, count, actor,
                                          actor == 3 ? 21 : 41);
        }
    }
    else
    {
        count = AddGeneratedCandidate(candidates, capacity, count, actor,
                                      100 + actor);
    }
    return min(count, capacity);
}

static u32 EnumerateOutcomes(const struct AiSimContext *context,
                             const struct AiSimBoard *board,
                             const struct AiSimJointTurn *turn,
                             struct AiSimOutcomeKey *outcomes,
                             u32 capacity,
                             void *callbackData)
{
    struct PlannerTestData *data = callbackData;

    (void)context;
    (void)board;
    (void)turn;
    data->enumerateCalls++;
    if (capacity == 0)
        return 0;
    memset(outcomes, 0, sizeof(*outcomes) * capacity);
    if (data->mode == PLANNER_TEST_MIXED_OUTCOME_OVERFLOW
     && turn->actions[1].choice == 10)
        return capacity + 1;
    if (data->mode == PLANNER_TEST_WEIGHTED)
    {
        if (capacity < 2)
            return 0;
        outcomes[0].flags = 1;
        outcomes[0].probabilityWeight = 1;
        outcomes[1].flags = 2;
        outcomes[1].probabilityWeight = 3;
        return 2;
    }
    outcomes[0].probabilityWeight = 1;
    return 1;
}

static enum AiSimApplyStatus ApplyTurn(const struct AiSimContext *context,
                                       const struct AiSimBoard *before,
                                       const struct AiSimJointTurn *turn,
                                       const struct AiSimOutcomeKey *outcome,
                                       struct AiSimBoard *after,
                                       struct AiSimTurnResult *result,
                                       void *callbackData)
{
    struct PlannerTestData *data = callbackData;

    (void)context;
    data->applyCalls++;
    *after = *before;
    *result = (struct AiSimTurnResult){0};
    after->turn++;

    switch (data->mode)
    {
    case PLANNER_TEST_MINIMAX:
        if (before->turn != 0)
        {
            s32 delta;
            u32 aiChoice = turn->actions[1].choice;
            u32 playerChoice = turn->actions[0].choice;

            if (aiChoice == 10)
                delta = playerChoice == 30 ? 100 : -100;
            else
                delta = 20;
            after->stellarBoostFlags[0] += delta;
        }
        break;
    case PLANNER_TEST_WEIGHTED:
        after->stellarBoostFlags[0] = outcome != NULL && outcome->flags == 1
                                    ? 0x8000 + 100
                                    : 0x8000;
        break;
    case PLANNER_TEST_RANKED:
        if (before->turn == 0)
            after->weather = 110 - turn->actions[1].choice;
        break;
    case PLANNER_TEST_FORCED_REPLACEMENT:
        if (before->turn == 0)
        {
            after->party[sRosterByBattler[1]].hp = 0;
            after->party[sRosterByBattler[3]].hp = 0;
            after->active[1].flags |= AI_SIM_ACTIVE_NEEDS_REPLACEMENT;
            after->active[3].flags |= AI_SIM_ACTIVE_NEEDS_REPLACEMENT;
        }
        break;
    default:
        break;
    }
    return AI_SIM_APPLY_OK;
}

static bool32 EvaluateBoard(const struct AiSimContext *context,
                            const struct AiSimBoard *rootBoard,
                            const struct AiSimBoard *board,
                            u8 aiSide,
                            struct AiJointScore *score,
                            void *callbackData)
{
    struct PlannerTestData *data = callbackData;
    s32 value;

    (void)context;
    (void)rootBoard;
    (void)aiSide;
    switch (data->mode)
    {
    case PLANNER_TEST_MINIMAX:
    case PLANNER_TEST_WEIGHTED:
        value = (s32)board->stellarBoostFlags[0] - 0x8000;
        break;
    case PLANNER_TEST_RANKED:
        value = board->weather;
        break;
    case PLANNER_TEST_TIE:
        value = 42;
        break;
    default:
        value = board->turn;
        break;
    }
    *score = (struct AiJointScore){0};
    score->total = value;
    score->immediate = value;
    return TRUE;
}

static void SetupRequest(u32 leftCount, u32 rightCount)
{
    sRequest = (struct AiJointPlannerRequest){0};
    sRequest.context = &sContext;
    sRequest.rootBoard = &sBoard;
    sRequest.rootCandidates[0] = sRootCandidates[0];
    sRequest.rootCandidates[1] = sRootCandidates[1];
    sRequest.rootCandidateCounts[0] = leftCount;
    sRequest.rootCandidateCounts[1] = rightCount;
    sRequest.rootActors[0] = 1;
    sRequest.rootActors[1] = 3;
    sRequest.confirmedPlayerActions[0] = MakeAction(0, 200);
    sRequest.confirmedPlayerActions[1] = MakeAction(2, 202);
    sRequest.confirmedPlayerActionCount = 2;
    sRequest.aiSide = B_SIDE_OPPONENT;
    sRequest.activeAiMask = (1u << 1) | (1u << 3);
    sRequest.cacheKeySalt = 0xC0DEF00D;
    sRequest.limits.depth3NodeBudget = 1000;
    sRequest.limits.totalNodeBudget = 2000;
    sRequest.limits.depth3FrameBudget = 60000;
    sRequest.limits.totalFrameBudget = 60000;
    sRequest.limits.extensionScoreWindow = 1000;
    sRequest.limits.maxExtensionRoots = AI_JOINT_MAX_EXTENSION_ROOTS;
    sRequest.generateAtomic = GenerateAtomic;
    sRequest.evaluateBoard = EvaluateBoard;
    sRequest.enumerateOutcomes = EnumerateOutcomes;
    sRequest.applyTurn = ApplyTurn;
    sRequest.callbackData = &sData;
}

static void SetupSingleRoot(void)
{
    sRootCandidates[0][0] = MakeCandidate(1, 1, 0, AI_JOINT_FAMILY_SETUP, 1);
    sRootCandidates[1][0] = MakeCandidate(3, 2, 0, AI_JOINT_FAMILY_OTHER, 2);
    SetupRequest(1, 1);
}

SINGLE_BATTLE_TEST("AI joint planner retains family diversity and rejects illegal atomic candidates")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        struct AiJointAtomicCandidate *input;
        struct AiJointAtomicCandidate *output;
        u32 count;

        InitPlannerFixture(PLANNER_TEST_FIXED);
        input = sRootCandidates[0];
        output = sWorkspace.retained[0];
        input[0] = MakeCandidate(1, 10, 100, AI_JOINT_FAMILY_CLEAN_DAMAGE, 10);
        input[1] = MakeCandidate(1, 11, 99, AI_JOINT_FAMILY_CLEAN_DAMAGE, 11);
        input[2] = MakeCandidate(1, 12, 1, AI_JOINT_FAMILY_PROTECT, 12);
        input[3] = MakeCandidate(1, 13, 0, AI_JOINT_FAMILY_SETUP, 13);
        input[4] = MakeCandidate(1, 14, -1, AI_JOINT_FAMILY_SWITCH, 14);
        input[5] = MakeCandidate(1, 15, 300, AI_JOINT_FAMILY_SUPPORT, 15);
        input[5].flags = 0;
        input[6] = MakeCandidate(1, 16, 300, AI_JOINT_FAMILY_SPEED_FIELD, 16);
        input[6].legalityFlags = AI_JOINT_ILLEGAL_TARGET;

        count = AiJoint_RetainAtomicCandidates(input, 7, output, 4);
        EXPECT_EQ(count, 4);
        EXPECT_EQ(output[0].family, AI_JOINT_FAMILY_CLEAN_DAMAGE);
        EXPECT_EQ(output[0].action.choice, 10);
        EXPECT_EQ(output[1].family, AI_JOINT_FAMILY_PROTECT);
        EXPECT_EQ(output[2].family, AI_JOINT_FAMILY_SETUP);
        EXPECT_EQ(output[3].family, AI_JOINT_FAMILY_SWITCH);

        input[0] = MakeCandidate(1, 20, 10, AI_JOINT_FAMILY_CLEAN_DAMAGE, 20);
        input[1] = input[0];
        input[0].action.flags = AI_SIM_ACTION_FORCED_REPLACEMENT;
        input[0].action.replacementRosterIndex = 4;
        input[1].action.flags = AI_SIM_ACTION_FORCED_REPLACEMENT;
        input[1].action.replacementRosterIndex = 5;
        count = AiJoint_RetainAtomicCandidates(input, 2, output, 2);
        EXPECT_EQ(count, 2);
        EXPECT_EQ(output[0].action.replacementRosterIndex, 4);
        EXPECT_EQ(output[1].action.replacementRosterIndex, 5);
        FreePlannerFixture();
    }
}

SINGLE_BATTLE_TEST("AI joint planner rejects pair conflicts and shared side resources")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        struct AiJointAtomicCandidate left;
        struct AiJointAtomicCandidate right;
        u32 count;

        InitPlannerFixture(PLANNER_TEST_FIXED);
        left = MakeCandidate(1, 10, 10, AI_JOINT_FAMILY_SWITCH, 10);
        right = MakeCandidate(3, 10, 10, AI_JOINT_FAMILY_SWITCH, 11);
        left.action.kind = AI_SIM_ACTION_SWITCH;
        right.action.kind = AI_SIM_ACTION_SWITCH;
        EXPECT_EQ(AiJoint_GetPairConflict(&sContext, &left, &right), AI_JOINT_CONFLICT_SAME_RESERVE);

        left = MakeCandidate(1, 20, 10, AI_JOINT_FAMILY_CLEAN_DAMAGE, 12);
        right = MakeCandidate(3, 5, 10, AI_JOINT_FAMILY_SWITCH, 13);
        left.action.flags = AI_SIM_ACTION_FORCED_REPLACEMENT;
        left.action.replacementRosterIndex = 5;
        right.action.kind = AI_SIM_ACTION_SWITCH;
        EXPECT_EQ(AiJoint_GetPairConflict(&sContext, &left, &right), AI_JOINT_CONFLICT_SAME_RESERVE);

        right = MakeCandidate(3, 21, 10, AI_JOINT_FAMILY_CLEAN_DAMAGE, 13);
        right.action.flags = AI_SIM_ACTION_FORCED_REPLACEMENT;
        right.action.replacementRosterIndex = 5;
        EXPECT_EQ(AiJoint_GetPairConflict(&sContext, &left, &right), AI_JOINT_CONFLICT_SAME_RESERVE);
        right.action.replacementRosterIndex = 6;
        EXPECT_EQ(AiJoint_GetPairConflict(&sContext, &left, &right), AI_JOINT_CONFLICT_NONE);

        left = MakeCandidate(1, 10, 10, AI_JOINT_FAMILY_GIMMICK, 10);
        right = MakeCandidate(3, 11, 10, AI_JOINT_FAMILY_GIMMICK, 11);
        left.resourceFlags = AI_JOINT_RESOURCE_MEGA;
        right.resourceFlags = AI_JOINT_RESOURCE_MEGA;
        EXPECT_EQ(AiJoint_GetPairConflict(&sContext, &left, &right), AI_JOINT_CONFLICT_SHARED_GIMMICK);

        left.resourceFlags = 0;
        right.resourceFlags = 0;
        left.flags |= AI_JOINT_ATOMIC_REQUIRES_PARTNER_ACTION;
        right.flags |= AI_JOINT_ATOMIC_PREVENTS_PARTNER_ACTION;
        EXPECT_EQ(AiJoint_GetPairConflict(&sContext, &left, &right), AI_JOINT_CONFLICT_PARTNER_REQUIREMENT);

        left.flags = AI_JOINT_ATOMIC_VALID | AI_JOINT_ATOMIC_FIELD_SETUP;
        right.flags = AI_JOINT_ATOMIC_VALID | AI_JOINT_ATOMIC_FIELD_SETUP;
        left.effectKey = 7;
        right.effectKey = 7;
        EXPECT_EQ(AiJoint_GetPairConflict(&sContext, &left, &right), AI_JOINT_CONFLICT_DUPLICATE_FIELD_SETUP);

        sRootCandidates[0][0] = MakeCandidate(1, 20, 20, AI_JOINT_FAMILY_GIMMICK, 20);
        sRootCandidates[0][0].resourceFlags = AI_JOINT_RESOURCE_MEGA;
        sRootCandidates[0][1] = MakeCandidate(1, 21, 10, AI_JOINT_FAMILY_CLEAN_DAMAGE, 21);
        sRootCandidates[1][0] = MakeCandidate(3, 30, 20, AI_JOINT_FAMILY_GIMMICK, 30);
        sRootCandidates[1][0].resourceFlags = AI_JOINT_RESOURCE_MEGA;
        count = AiJoint_BuildPairs(&sContext,
                                   sRootCandidates[0], 2,
                                   sRootCandidates[1], 1,
                                   sWorkspace.pairScratch,
                                   ARRAY_COUNT(sWorkspace.pairScratch));
        EXPECT_EQ(count, 1);
        EXPECT_EQ(sWorkspace.pairScratch[0].actions[0].choice, 21);
        EXPECT_EQ(sWorkspace.pairScratch[0].actions[1].choice, 30);
        FreePlannerFixture();
    }
}

SINGLE_BATTLE_TEST("AI joint planner retains four tactical pair families before the three-wide depth-three search")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        struct AiJointAtomicCandidate left[5];
        struct AiJointAtomicCandidate right;
        bool8 keptFamily[AI_JOINT_FAMILY_COUNT] = {0};
        u32 count;
        u32 i;

        InitPlannerFixture(PLANNER_TEST_FIXED);
        left[0] = MakeCandidate(1, 10, 1000, AI_JOINT_FAMILY_CLEAN_KO, 10);
        left[0].flags |= AI_JOINT_ATOMIC_GUARANTEED_KO;
        left[1] = MakeCandidate(1, 11, 999, AI_JOINT_FAMILY_CLEAN_KO, 11);
        left[1].flags |= AI_JOINT_ATOMIC_GUARANTEED_KO;
        left[2] = MakeCandidate(1, 12, 100, AI_JOINT_FAMILY_PROTECT, 12);
        left[3] = MakeCandidate(1, 13, 90, AI_JOINT_FAMILY_SETUP, 13);
        left[4] = MakeCandidate(1, 4, 80, AI_JOINT_FAMILY_SWITCH, 14);
        left[4].action.kind = AI_SIM_ACTION_SWITCH;
        right = MakeCandidate(3, 20, 0, AI_JOINT_FAMILY_OTHER, 20);

        EXPECT_EQ(AI_JOINT_DEPTH3_RETENTION_WIDTH, 4);
        EXPECT_EQ(AI_JOINT_DEPTH3_SEARCH_WIDTH, 3);
        count = AiJoint_BuildPairs(&sContext,
                                   left, ARRAY_COUNT(left),
                                   &right, 1,
                                   sWorkspace.pairScratch,
                                   AI_JOINT_DEPTH3_RETENTION_WIDTH);
        EXPECT_EQ(count, AI_JOINT_DEPTH3_RETENTION_WIDTH);
        for (i = 0; i < count; i++)
            keptFamily[sWorkspace.pairScratch[i].families[0]] = TRUE;
        EXPECT(keptFamily[AI_JOINT_FAMILY_CLEAN_KO]);
        EXPECT(keptFamily[AI_JOINT_FAMILY_PROTECT]);
        EXPECT(keptFamily[AI_JOINT_FAMILY_SETUP]);
        EXPECT(keptFamily[AI_JOINT_FAMILY_SWITCH]);
        EXPECT_EQ(sWorkspace.pairScratch[0].actions[0].choice, 10);

        memset(keptFamily, 0, sizeof(keptFamily));
        count = AiJoint_BuildPairs(&sContext,
                                   left, ARRAY_COUNT(left),
                                   &right, 1,
                                   sWorkspace.pairScratch,
                                   AI_JOINT_DEPTH3_SEARCH_WIDTH);
        EXPECT_EQ(count, AI_JOINT_DEPTH3_SEARCH_WIDTH);
        for (i = 0; i < count; i++)
            keptFamily[sWorkspace.pairScratch[i].families[0]] = TRUE;
        EXPECT(keptFamily[AI_JOINT_FAMILY_CLEAN_KO]);
        EXPECT(keptFamily[AI_JOINT_FAMILY_PROTECT]);
        EXPECT(keptFamily[AI_JOINT_FAMILY_SETUP]);
        EXPECT(!keptFamily[AI_JOINT_FAMILY_SWITCH]);
        FreePlannerFixture();
    }
}

SINGLE_BATTLE_TEST("AI joint planner maximizes its pair against the player's minimizing pair")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        InitPlannerFixture(PLANNER_TEST_MINIMAX);
        SetupSingleRoot();
        EXPECT_EQ(AiJointPlanner_Run(&sJob, &sWorkspace, &sRequest, &sResult), AI_JOINT_JOB_READY);
        EXPECT_EQ(sResult.completedDepth, AI_JOINT_EXTENSION_DEPTH);
        // Four future turns each choose the safe +20 line. The tempting +100
        // line has a -100 minimizing response and must never be selected.
        EXPECT_EQ(sResult.score.total, 80);
        EXPECT_EQ(sResult.score.immediate, 80);
        FreePlannerFixture();
    }
}

SINGLE_BATTLE_TEST("AI joint planner falls back on an unresolved legal player response")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        struct AiJointRootSummary summary;

        InitPlannerFixture(PLANNER_TEST_UNRESOLVED_PLAYER);
        SetupSingleRoot();
        EXPECT_EQ(AiJointPlanner_Run(&sJob, &sWorkspace, &sRequest, &sResult), AI_JOINT_JOB_FALLBACK);
        EXPECT_EQ(sResult.stats.terminationReason, AI_JOINT_TERMINATION_UNSUPPORTED);
        EXPECT(sResult.stats.flags & AI_JOINT_STATS_UNSUPPORTED_SEEN);
        EXPECT_EQ(sResult.completedDepth, 1);
        EXPECT_EQ(sResult.stats.requestedDepth, AI_JOINT_STANDARD_DEPTH);
        EXPECT_EQ(sResult.stats.depth1CompletedRoots, 1);
        EXPECT_EQ(sResult.stats.depth3CompletedRoots, 0);
        EXPECT_GT(sResult.stats.prunedBranches, 0);
        EXPECT(AiJointPlanner_GetRankedRoot(&sJob, 0, &summary));
        EXPECT(summary.flags & AI_JOINT_ROOT_DEPTH1_COMPLETE);
        EXPECT(!(summary.flags & AI_JOINT_ROOT_DEPTH3_COMPLETE));
        FreePlannerFixture();
    }
}

SINGLE_BATTLE_TEST("AI joint planner falls back on an unresolved legal AI action")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        struct AiJointRootSummary summary;

        InitPlannerFixture(PLANNER_TEST_UNRESOLVED_AI);
        SetupSingleRoot();
        EXPECT_EQ(AiJointPlanner_Run(&sJob, &sWorkspace, &sRequest, &sResult), AI_JOINT_JOB_FALLBACK);
        EXPECT_EQ(sResult.stats.terminationReason, AI_JOINT_TERMINATION_UNSUPPORTED);
        EXPECT(sResult.stats.flags & AI_JOINT_STATS_UNSUPPORTED_SEEN);
        EXPECT_EQ(sResult.completedDepth, 1);
        EXPECT_EQ(sResult.stats.requestedDepth, AI_JOINT_STANDARD_DEPTH);
        EXPECT_EQ(sResult.stats.depth1CompletedRoots, 1);
        EXPECT_EQ(sResult.stats.depth3CompletedRoots, 0);
        EXPECT_GT(sResult.stats.prunedBranches, 0);
        EXPECT(AiJointPlanner_GetRankedRoot(&sJob, 0, &summary));
        EXPECT(summary.flags & AI_JOINT_ROOT_DEPTH1_COMPLETE);
        EXPECT(!(summary.flags & AI_JOINT_ROOT_DEPTH3_COMPLETE));
        FreePlannerFixture();
    }
}

SINGLE_BATTLE_TEST("AI joint planner uses exact stochastic weights with bounded downside risk")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        InitPlannerFixture(PLANNER_TEST_WEIGHTED);
        SetupSingleRoot();
        sRequest.limits.depth3NodeBudget = 2;
        sRequest.limits.totalNodeBudget = 2;

        EXPECT_EQ(AiJointPlanner_Run(&sJob, &sWorkspace, &sRequest, &sResult), AI_JOINT_JOB_READY);
        EXPECT_EQ(sResult.completedDepth, 1);
        EXPECT_EQ(sResult.stats.terminationReason, AI_JOINT_TERMINATION_NODE_BUDGET);
        // 1/4 * 100 + 3/4 * 0 = 25, then ceil((25 - 0) / 4) = 7 risk.
        EXPECT_EQ(sResult.score.immediate, 25);
        EXPECT_EQ(sResult.score.risk, 7);
        EXPECT_EQ(sResult.score.total, 18);
        FreePlannerFixture();
    }
}

SINGLE_BATTLE_TEST("AI joint planner completes depth three and selectively extends only the top three roots")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        struct AiJointRootSummary summary;
        u32 i;

        InitPlannerFixture(PLANNER_TEST_RANKED);
        for (i = 0; i < 4; i++)
            sRootCandidates[0][i] = MakeCandidate(1, 10 + i * 10, 0,
                                                  AI_JOINT_FAMILY_SETUP, 10 + i);
        sRootCandidates[1][0] = MakeCandidate(3, 2, 0, AI_JOINT_FAMILY_OTHER, 2);
        SetupRequest(4, 1);
        sRequest.limits.maxExtensionRoots = 3;

        EXPECT_EQ(AiJointPlanner_Run(&sJob, &sWorkspace, &sRequest, &sResult), AI_JOINT_JOB_READY);
        EXPECT_EQ(sResult.rootCount, 4);
        EXPECT_EQ(sResult.stats.depth1CompletedRoots, 4);
        EXPECT_EQ(sResult.stats.depth3CompletedRoots, AI_JOINT_DEPTH3_SEARCH_WIDTH);
        EXPECT_EQ(sResult.stats.depth5CompletedRoots, 3);
        EXPECT_EQ(sResult.completedDepth, AI_JOINT_EXTENSION_DEPTH);
        for (i = 0; i < 4; i++)
        {
            EXPECT(AiJointPlanner_GetRankedRoot(&sJob, i, &summary));
            EXPECT_EQ(summary.score.total, 100 - i * 10);
            EXPECT_EQ(summary.completedDepth,
                      i < 3 ? AI_JOINT_EXTENSION_DEPTH : 1);
            if (i < 3)
                EXPECT(summary.flags & AI_JOINT_ROOT_EXTENSION_SELECTED);
            else
                EXPECT(!(summary.flags & AI_JOINT_ROOT_EXTENSION_SELECTED));
        }
        FreePlannerFixture();
    }
}

SINGLE_BATTLE_TEST("AI joint planner reserves depth-three slots for Protect and setup over duplicate damage")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        static const u8 expectedChoices[] = {10, 20, 30, 40};
        static const u8 expectedFamilies[] =
        {
            AI_JOINT_FAMILY_CLEAN_KO,
            AI_JOINT_FAMILY_CLEAN_DAMAGE,
            AI_JOINT_FAMILY_PROTECT,
            AI_JOINT_FAMILY_SETUP,
        };
        struct AiJointRootSummary summary;
        bool8 found[ARRAY_COUNT(expectedChoices)] = {0};

        InitPlannerFixture(PLANNER_TEST_RANKED);
        for (u32 i = 0; i < ARRAY_COUNT(expectedChoices); i++)
            sRootCandidates[0][i] = MakeCandidate(1, expectedChoices[i], 0,
                                                  expectedFamilies[i],
                                                  expectedChoices[i]);
        sRootCandidates[1][0] = MakeCandidate(3, 2, 0,
                                              AI_JOINT_FAMILY_OTHER, 2);
        SetupRequest(ARRAY_COUNT(expectedChoices), 1);

        EXPECT_EQ(AiJointPlanner_Run(&sJob, &sWorkspace, &sRequest, &sResult),
                  AI_JOINT_JOB_READY);
        EXPECT_EQ(sResult.stats.depth3CompletedRoots,
                  AI_JOINT_DEPTH3_SEARCH_WIDTH);
        for (u32 rank = 0; rank < ARRAY_COUNT(expectedChoices); rank++)
        {
            EXPECT(AiJointPlanner_GetRankedRoot(&sJob, rank, &summary));
            for (u32 i = 0; i < ARRAY_COUNT(expectedChoices); i++)
            {
                if (summary.pair.actions[0].choice == expectedChoices[i])
                    found[i] = TRUE;
            }
            if (summary.pair.actions[0].choice == 20)
                EXPECT_EQ(summary.completedDepth, 1);
            else
                EXPECT(summary.completedDepth >= AI_JOINT_STANDARD_DEPTH);
        }
        for (u32 i = 0; i < ARRAY_COUNT(found); i++)
            EXPECT(found[i]);
        FreePlannerFixture();
    }
}

SINGLE_BATTLE_TEST("AI joint planner searches selected setup before duplicate offense after an offensive baseline")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        static const u16 expectedOrder[] = {10, 30, 20};
        bool8 selectedChoices[4] = {0};
        u32 steps = 0;

        InitPlannerFixture(PLANNER_TEST_RANKED);
        sRootCandidates[0][0] = MakeCandidate(1, 10, 40,
                                              AI_JOINT_FAMILY_CLEAN_DAMAGE, 10);
        sRootCandidates[0][1] = MakeCandidate(1, 20, 30,
                                              AI_JOINT_FAMILY_CLEAN_KO, 20);
        sRootCandidates[0][2] = MakeCandidate(1, 30, 20,
                                              AI_JOINT_FAMILY_SETUP, 30);
        sRootCandidates[0][3] = MakeCandidate(1, 40, 10,
                                              AI_JOINT_FAMILY_OTHER, 40);
        sRootCandidates[1][0] = MakeCandidate(3, 2, 0,
                                              AI_JOINT_FAMILY_PROTECT, 2);
        SetupRequest(4, 1);

        EXPECT_EQ(AiJointPlanner_InitJob(&sJob, &sWorkspace, &sRequest),
                  AI_JOINT_JOB_BUILDING);
        while (sJob.depth3SelectionCount == 0 && steps++ < 100)
            AiJointPlanner_Step(&sJob, 1, &sResult);

        EXPECT_EQ(sJob.depth3SelectionCount, ARRAY_COUNT(expectedOrder));
        for (u32 slot = 0; slot < sJob.depth3SelectionCount; slot++)
        {
            u32 root = sJob.selectedDepth3[slot];
            u32 pairIndex = sWorkspace.rootResults[root].pairIndex;
            u32 choice = sWorkspace.rootPairs[pairIndex].actions[0].choice;

            EXPECT_EQ(choice, expectedOrder[slot]);
            if (choice == 10)
                selectedChoices[0] = TRUE;
            else if (choice == 20)
                selectedChoices[1] = TRUE;
            else if (choice == 30)
                selectedChoices[2] = TRUE;
            else if (choice == 40)
                selectedChoices[3] = TRUE;
        }
        EXPECT(selectedChoices[0]);
        EXPECT(selectedChoices[1]);
        EXPECT(selectedChoices[2]);
        EXPECT(!selectedChoices[3]);

        while (sResult.stats.depth3CompletedRoots < 2
            && sJob.state != AI_JOINT_JOB_READY
            && sJob.state != AI_JOINT_JOB_FALLBACK
            && steps++ < 200)
            AiJointPlanner_Step(&sJob, 1, &sResult);
        EXPECT_EQ(sResult.stats.depth3CompletedRoots, 2);

        // Expire before the third selected root can complete.  Membership
        // still includes the clean-KO root, while the completed anytime set
        // contains the offensive baseline and the promoted setup root.
        sJob.request.limits.depth3FrameBudget = sJob.stats.elapsedFrames;
        EXPECT_EQ(AiJointPlanner_Step(&sJob, 1, &sResult), AI_JOINT_JOB_READY);
        EXPECT_EQ(sResult.stats.terminationReason,
                  AI_JOINT_TERMINATION_FRAME_BUDGET);
        for (u32 root = 0; root < sJob.rootCount; root++)
        {
            u32 pairIndex = sWorkspace.rootResults[root].pairIndex;
            u32 choice = sWorkspace.rootPairs[pairIndex].actions[0].choice;
            u32 depth = sWorkspace.rootResults[root].completedDepth;

            if (choice == 10 || choice == 30)
                EXPECT_EQ(depth, AI_JOINT_STANDARD_DEPTH);
            else
                EXPECT_EQ(depth, 1);
        }
        FreePlannerFixture();
    }
}

SINGLE_BATTLE_TEST("AI joint planner treats a damaging gimmick baseline as offense for anytime ordering")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        static const u16 expectedOrder[] = {MOVE_MOONBLAST, MOVE_GEOMANCY, MOVE_TACKLE};
        u32 steps = 0;

        InitPlannerFixture(PLANNER_TEST_RANKED);
        sRootCandidates[0][0] = MakeCandidate(1, MOVE_MOONBLAST, 40,
                                              AI_JOINT_FAMILY_GIMMICK, 10);
        sRootCandidates[0][1] = MakeCandidate(1, MOVE_TACKLE, 30,
                                              AI_JOINT_FAMILY_CLEAN_KO, 20);
        sRootCandidates[0][2] = MakeCandidate(1, MOVE_GEOMANCY, 20,
                                              AI_JOINT_FAMILY_SETUP, 30);
        sRootCandidates[1][0] = MakeCandidate(3, MOVE_PROTECT, 0,
                                              AI_JOINT_FAMILY_PROTECT, 2);
        SetupRequest(3, 1);

        EXPECT_EQ(AiJointPlanner_InitJob(&sJob, &sWorkspace, &sRequest),
                  AI_JOINT_JOB_BUILDING);
        while (sJob.depth3SelectionCount == 0 && steps++ < 100)
            AiJointPlanner_Step(&sJob, 1, &sResult);

        EXPECT_EQ(sJob.depth3SelectionCount, ARRAY_COUNT(expectedOrder));
        for (u32 slot = 0; slot < sJob.depth3SelectionCount; slot++)
        {
            u32 root = sJob.selectedDepth3[slot];
            u32 pairIndex = sWorkspace.rootResults[root].pairIndex;

            EXPECT_EQ(sWorkspace.rootPairs[pairIndex].actions[0].choice,
                      expectedOrder[slot]);
        }
        FreePlannerFixture();
    }
}

SINGLE_BATTLE_TEST("AI joint planner assigns a lone reserve to the earlier double-KO slot")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        InitPlannerFixture(PLANNER_TEST_FORCED_REPLACEMENT);
        sContext.mons[8] = sContext.mons[sRosterByBattler[1]];
        sContext.mons[8].trainer = B_SIDE_OPPONENT;
        sContext.mons[8].partyIndex = 2;
        sContext.mons[8].moves[0] = MOVE_TACKLE;
        sBoard.party[8] = sBoard.party[sRosterByBattler[1]];
        sBoard.party[8].hp = 100;
        SetupSingleRoot();

        EXPECT_EQ(AiJointPlanner_Run(&sJob, &sWorkspace, &sRequest, &sResult),
                  AI_JOINT_JOB_READY);
        EXPECT_GE(sResult.completedDepth, AI_JOINT_STANDARD_DEPTH);
        EXPECT_EQ(sResult.unsupportedFlags, AI_SIM_UNSUPPORTED_NONE);
        EXPECT_GT(sData.generateCalls, 0);
        EXPECT(sData.forcedReplacementActorMask & (1u << 1));
        EXPECT(!(sData.forcedReplacementActorMask & (1u << 3)));
        FreePlannerFixture();
    }
}

SINGLE_BATTLE_TEST("AI joint planner stops a quiet root at completed depth three")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        struct AiJointRootSummary summary;
        u32 exactNodes;

        InitPlannerFixture(PLANNER_TEST_FIXED);
        sRootCandidates[0][0] = MakeCandidate(1, 1, 0, AI_JOINT_FAMILY_OTHER, 1);
        sRootCandidates[1][0] = MakeCandidate(3, 2, 0, AI_JOINT_FAMILY_OTHER, 2);
        SetupRequest(1, 1);

        EXPECT_EQ(AiJointPlanner_Run(&sJob, &sWorkspace, &sRequest, &sResult), AI_JOINT_JOB_READY);
        EXPECT_EQ(sResult.completedDepth, AI_JOINT_STANDARD_DEPTH);
        EXPECT_EQ(sResult.stats.requestedDepth, AI_JOINT_STANDARD_DEPTH);
        EXPECT_EQ(sResult.stats.depth3CompletedRoots, 1);
        EXPECT_EQ(sResult.stats.depth5CompletedRoots, 0);
        EXPECT(AiJointPlanner_GetRankedRoot(&sJob, 0, &summary));
        EXPECT(summary.flags & AI_JOINT_ROOT_DEPTH3_COMPLETE);
        EXPECT(!(summary.flags & AI_JOINT_ROOT_EXTENSION_SELECTED));
        EXPECT(!(summary.flags & AI_JOINT_ROOT_DEPTH5_COMPLETE));

        // Repeating the same quiet search with one exact-sized slice must
        // drain the zero-node root/phase completion transitions immediately.
        exactNodes = sResult.stats.nodesVisited;
        InitPlannerFixture(PLANNER_TEST_FIXED);
        sRootCandidates[0][0] = MakeCandidate(1, 1, 0, AI_JOINT_FAMILY_OTHER, 1);
        sRootCandidates[1][0] = MakeCandidate(3, 2, 0, AI_JOINT_FAMILY_OTHER, 2);
        SetupRequest(1, 1);
        EXPECT_EQ(AiJointPlanner_InitJob(&sJob, &sWorkspace, &sRequest),
                  AI_JOINT_JOB_BUILDING);
        EXPECT_EQ(AiJointPlanner_Step(&sJob, exactNodes, &sResult),
                  AI_JOINT_JOB_READY);
        EXPECT_EQ(sResult.stats.nodesVisited, exactNodes);
        FreePlannerFixture();
    }
}

SINGLE_BATTLE_TEST("AI joint planner keeps completed depth three when depth five exhausts its budget")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        struct AiJointRootSummary summary;

        InitPlannerFixture(PLANNER_TEST_PARTIAL);
        SetupSingleRoot();
        sRequest.limits.depth3NodeBudget = 4;
        sRequest.limits.totalNodeBudget = 6;
        sRequest.limits.maxExtensionRoots = 1;

        EXPECT_EQ(AiJointPlanner_Run(&sJob, &sWorkspace, &sRequest, &sResult), AI_JOINT_JOB_READY);
        EXPECT_EQ(sResult.stats.terminationReason, AI_JOINT_TERMINATION_NODE_BUDGET);
        EXPECT_EQ(sResult.completedDepth, AI_JOINT_STANDARD_DEPTH);
        EXPECT_EQ(sResult.score.total, 3);
        EXPECT_EQ(sResult.stats.depth3CompletedRoots, 1);
        EXPECT_EQ(sResult.stats.depth5CompletedRoots, 0);
        EXPECT(AiJointPlanner_GetRankedRoot(&sJob, 0, &summary));
        EXPECT(summary.flags & AI_JOINT_ROOT_DEPTH3_COMPLETE);
        EXPECT(!(summary.flags & AI_JOINT_ROOT_DEPTH5_COMPLETE));
        EXPECT_EQ(summary.score.total, 3);
        FreePlannerFixture();
    }
}

SINGLE_BATTLE_TEST("AI joint planner falls back when node or frame budget expires before any root completes")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        InitPlannerFixture(PLANNER_TEST_WEIGHTED);
        SetupSingleRoot();
        sRequest.limits.depth3NodeBudget = 1;
        sRequest.limits.totalNodeBudget = 1;
        EXPECT_EQ(AiJointPlanner_Run(&sJob, &sWorkspace, &sRequest, &sResult), AI_JOINT_JOB_FALLBACK);
        EXPECT_EQ(sResult.stats.terminationReason, AI_JOINT_TERMINATION_NODE_BUDGET);
        EXPECT_EQ(sResult.completedDepth, 0);

        InitPlannerFixture(PLANNER_TEST_WEIGHTED);
        SetupSingleRoot();
        sRequest.limits.depth3FrameBudget = 1;
        sRequest.limits.totalFrameBudget = 1;
        EXPECT_EQ(AiJointPlanner_InitJob(&sJob, &sWorkspace, &sRequest), AI_JOINT_JOB_BUILDING);
        EXPECT_EQ(AiJointPlanner_Step(&sJob, 1, &sResult), AI_JOINT_JOB_BUILDING);
        EXPECT_EQ(AiJointPlanner_Step(&sJob, 1, &sResult), AI_JOINT_JOB_FALLBACK);
        EXPECT_EQ(sResult.stats.terminationReason, AI_JOINT_TERMINATION_FRAME_BUDGET);
        EXPECT_EQ(sResult.completedDepth, 0);
        FreePlannerFixture();
    }
}

SINGLE_BATTLE_TEST("AI joint planner is stable across input order and resumable node slices")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        struct AiJointPairCandidate firstChosen;
        struct AiJointScore uninterruptedScore;
        u16 uninterruptedNodes;
        u32 steps = 0;

        InitPlannerFixture(PLANNER_TEST_TIE);
        sRootCandidates[0][0] = MakeCandidate(1, 10, 0, AI_JOINT_FAMILY_OTHER, 10);
        sRootCandidates[0][1] = MakeCandidate(1, 20, 0, AI_JOINT_FAMILY_OTHER, 20);
        sRootCandidates[1][0] = MakeCandidate(3, 2, 0, AI_JOINT_FAMILY_OTHER, 2);
        SetupRequest(2, 1);
        EXPECT_EQ(AiJointPlanner_Run(&sJob, &sWorkspace, &sRequest, &sResult), AI_JOINT_JOB_READY);
        firstChosen = sResult.chosenPair;

        InitPlannerFixture(PLANNER_TEST_TIE);
        sRootCandidates[0][0] = MakeCandidate(1, 20, 0, AI_JOINT_FAMILY_OTHER, 20);
        sRootCandidates[0][1] = MakeCandidate(1, 10, 0, AI_JOINT_FAMILY_OTHER, 10);
        sRootCandidates[1][0] = MakeCandidate(3, 2, 0, AI_JOINT_FAMILY_OTHER, 2);
        SetupRequest(2, 1);
        EXPECT_EQ(AiJointPlanner_Run(&sJob, &sWorkspace, &sRequest, &sResult), AI_JOINT_JOB_READY);
        EXPECT_EQ(sResult.chosenPair.stableKey, firstChosen.stableKey);
        EXPECT_EQ(sResult.chosenPair.actions[0].choice, firstChosen.actions[0].choice);
        EXPECT_EQ(sResult.chosenPair.actions[1].choice, firstChosen.actions[1].choice);

        InitPlannerFixture(PLANNER_TEST_CACHE);
        SetupSingleRoot();
        EXPECT_EQ(AiJointPlanner_Run(&sJob, &sWorkspace, &sRequest, &sResult), AI_JOINT_JOB_READY);
        uninterruptedScore = sResult.score;
        uninterruptedNodes = sResult.stats.nodesVisited;
        EXPECT_GT(sResult.stats.cacheHits, 0);

        memset(&sWorkspace, 0, sizeof(sWorkspace));
        memset(&sJob, 0, sizeof(sJob));
        memset(&sResult, 0, sizeof(sResult));
        sData.generateCalls = 0;
        sData.enumerateCalls = 0;
        sData.applyCalls = 0;
        EXPECT_EQ(AiJointPlanner_InitJob(&sJob, &sWorkspace, &sRequest), AI_JOINT_JOB_BUILDING);
        while (sJob.state != AI_JOINT_JOB_READY && sJob.state != AI_JOINT_JOB_FALLBACK && steps < 1000)
        {
            u32 beforeNodes = sJob.stats.nodesVisited;
            AiJointPlanner_Step(&sJob, 1, &sResult);
            EXPECT_LE(sJob.stats.nodesVisited - beforeNodes, 1);
            steps++;
        }
        EXPECT_EQ(sJob.state, AI_JOINT_JOB_READY);
        EXPECT_GT(steps, 1);
        EXPECT_EQ(sResult.completedDepth, AI_JOINT_EXTENSION_DEPTH);
        EXPECT_EQ(sResult.score.total, uninterruptedScore.total);
        EXPECT_EQ(sResult.score.immediate, uninterruptedScore.immediate);
        EXPECT_EQ(sResult.stats.nodesVisited, uninterruptedNodes);
        EXPECT_GT(sResult.stats.cacheHits, 0);
        FreePlannerFixture();
    }
}

SINGLE_BATTLE_TEST("AI joint planner clamps a supported overflowing provider deterministically")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        struct AiJointPairCandidate firstChosen;
        struct AiJointScore firstScore;
        u16 firstNodes;

        InitPlannerFixture(PLANNER_TEST_CAPACITY_OVERFLOW);
        sRootCandidates[0][0] = MakeCandidate(1, 1, 0, AI_JOINT_FAMILY_OTHER, 1);
        sRootCandidates[1][0] = MakeCandidate(3, 2, 0, AI_JOINT_FAMILY_OTHER, 2);
        SetupRequest(1, 1);
        EXPECT_EQ(AiJointPlanner_Run(&sJob, &sWorkspace, &sRequest, &sResult), AI_JOINT_JOB_READY);
        EXPECT_EQ(sResult.completedDepth, AI_JOINT_STANDARD_DEPTH);
        EXPECT_EQ(sResult.stats.requestedDepth, AI_JOINT_STANDARD_DEPTH);
        EXPECT_EQ(sResult.stats.terminationReason, AI_JOINT_TERMINATION_COMPLETE);
        EXPECT_EQ(sResult.unsupportedFlags, 0);
        EXPECT_GT(sData.generateCalls, 0);
        firstChosen = sResult.chosenPair;
        firstScore = sResult.score;
        firstNodes = sResult.stats.nodesVisited;

        InitPlannerFixture(PLANNER_TEST_CAPACITY_OVERFLOW);
        sRootCandidates[0][0] = MakeCandidate(1, 1, 0, AI_JOINT_FAMILY_OTHER, 1);
        sRootCandidates[1][0] = MakeCandidate(3, 2, 0, AI_JOINT_FAMILY_OTHER, 2);
        SetupRequest(1, 1);
        EXPECT_EQ(AiJointPlanner_Run(&sJob, &sWorkspace, &sRequest, &sResult), AI_JOINT_JOB_READY);
        EXPECT_EQ(sResult.completedDepth, AI_JOINT_STANDARD_DEPTH);
        EXPECT_EQ(sResult.stats.terminationReason, AI_JOINT_TERMINATION_COMPLETE);
        EXPECT_EQ(sResult.unsupportedFlags, 0);
        EXPECT_EQ(sResult.chosenPair.stableKey, firstChosen.stableKey);
        EXPECT_EQ(sResult.chosenPair.actions[0].choice, firstChosen.actions[0].choice);
        EXPECT_EQ(sResult.chosenPair.actions[1].choice, firstChosen.actions[1].choice);
        EXPECT_EQ(sResult.score.total, firstScore.total);
        EXPECT_EQ(sResult.score.immediate, firstScore.immediate);
        EXPECT_EQ(sResult.stats.nodesVisited, firstNodes);
        FreePlannerFixture();
    }
}

SINGLE_BATTLE_TEST("AI joint planner falls back when one retained root exceeds the exact outcome capacity")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        InitPlannerFixture(PLANNER_TEST_MIXED_OUTCOME_OVERFLOW);
        sRootCandidates[0][0] = MakeCandidate(1, 10, 0,
                                              AI_JOINT_FAMILY_CLEAN_DAMAGE, 10);
        sRootCandidates[0][1] = MakeCandidate(1, 20, 1,
                                              AI_JOINT_FAMILY_SETUP, 20);
        sRootCandidates[1][0] = MakeCandidate(3, 2, 0,
                                              AI_JOINT_FAMILY_OTHER, 2);
        SetupRequest(2, 1);

        EXPECT_EQ(AiJointPlanner_Run(&sJob, &sWorkspace, &sRequest, &sResult),
                  AI_JOINT_JOB_FALLBACK);
        EXPECT_EQ(sResult.stats.terminationReason,
                  AI_JOINT_TERMINATION_UNSUPPORTED);
        EXPECT(sResult.stats.flags & AI_JOINT_STATS_UNSUPPORTED_SEEN);
        EXPECT(sResult.unsupportedFlags & AI_SIM_UNSUPPORTED_OUTCOME);
        EXPECT_EQ(sResult.rootCount, 2);
        EXPECT_GE(sResult.stats.depth1CompletedRoots, 1);
        FreePlannerFixture();
    }
}

SINGLE_BATTLE_TEST("AI joint board evaluation makes terminal outcomes dominant and suppresses future pressure")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        struct AiJointScore winScore;
        struct AiJointScore nearWinScore;
        struct AiJointScore lossScore;
        struct AiJointScore lowAiDrawScore;
        struct AiJointScore lowFoeDrawScore;

        InitPressureFixture();
        sScratchBoard = sBoard;
        sScratchBoard.party[sRosterByBattler[0]].hp = 0;
        sScratchBoard.party[sRosterByBattler[2]].hp = 0;
        sScratchBoard.active[1].statStages[STAT_SPATK] = MAX_STAT_STAGE;
        sScratchBoard.active[1].statStages[STAT_SPEED] = MAX_STAT_STAGE;
        sScratchBoard.sides[B_SIDE_OPPONENT].tailwindTimer = 4;
        EXPECT(AiJoint_EvaluateBoard(&sContext, &sBoard, &sScratchBoard,
                                     B_SIDE_OPPONENT, &winScore));

        sScratchBoard = sBoard;
        sScratchBoard.party[sRosterByBattler[0]].hp = 1;
        sScratchBoard.party[sRosterByBattler[2]].hp = 1;
        sScratchBoard.active[1].statStages[STAT_SPATK] = MAX_STAT_STAGE;
        sScratchBoard.active[1].statStages[STAT_SPEED] = MAX_STAT_STAGE;
        sScratchBoard.sides[B_SIDE_OPPONENT].tailwindTimer = 4;
        EXPECT(AiJoint_EvaluateBoard(&sContext, &sBoard, &sScratchBoard,
                                     B_SIDE_OPPONENT, &nearWinScore));

        sScratchBoard = sBoard;
        sScratchBoard.party[sRosterByBattler[1]].hp = 0;
        sScratchBoard.party[sRosterByBattler[3]].hp = 0;
        sScratchBoard.active[0].statStages[STAT_SPATK] = MAX_STAT_STAGE;
        sScratchBoard.sides[B_SIDE_PLAYER].tailwindTimer = 4;
        EXPECT(AiJoint_EvaluateBoard(&sContext, &sBoard, &sScratchBoard,
                                     B_SIDE_OPPONENT, &lossScore));

        sScratchBoard = sBoard;
        for (u32 battler = 0; battler < MAX_BATTLERS_COUNT; battler++)
            sScratchBoard.party[sRosterByBattler[battler]].hp = 0;
        sBoard.party[sRosterByBattler[1]].hp = 1;
        EXPECT(AiJoint_EvaluateBoard(&sContext, &sBoard, &sScratchBoard,
                                     B_SIDE_OPPONENT, &lowAiDrawScore));
        sBoard.party[sRosterByBattler[1]].hp = 200;
        sBoard.party[sRosterByBattler[0]].hp = 1;
        EXPECT(AiJoint_EvaluateBoard(&sContext, &sBoard, &sScratchBoard,
                                     B_SIDE_OPPONENT, &lowFoeDrawScore));
        EXPECT_EQ(winScore.future, 0);
        EXPECT_EQ(lossScore.future, 0);
        EXPECT_GT(winScore.total, nearWinScore.total);
        EXPECT_GT(nearWinScore.total, lossScore.total);
        EXPECT_GT(winScore.total, 12000);
        EXPECT_LT(lossScore.total, -12000);
        EXPECT_EQ(lowAiDrawScore.total, 0);
        EXPECT_EQ(lowAiDrawScore.immediate, 0);
        EXPECT_EQ(lowAiDrawScore.future, 0);
        EXPECT_EQ(lowFoeDrawScore.total, lowAiDrawScore.total);
        FreePlannerFixture();
    }
}

SINGLE_BATTLE_TEST("AI joint board evaluation keeps persistent-form max HP after it returns to the bench")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        struct AiJointScore score;
        struct AiSimMonTemplate *persistentMon;
        u32 persistentRoster;

        InitPressureFixture();
        persistentRoster = sRosterByBattler[0];
        persistentMon = &sContext.mons[persistentRoster];
        persistentMon->transformed = persistentMon->normal;
        memset(&persistentMon->normal, 0, sizeof(persistentMon->normal));
        sBoard.active[0].flags |= AI_SIM_ACTIVE_TRANSFORMED;

        sScratchBoard = sBoard;
        sScratchBoard.activeMask &= ~(1u << 0);
        sScratchBoard.party[persistentRoster].hp = 100;
        EXPECT(AiJoint_EvaluateBoardWithRootPressure(&sContext, &sBoard,
                                                     &sScratchBoard,
                                                     B_SIDE_OPPONENT, 0,
                                                     &score));
        EXPECT_EQ(score.immediate, 100);
        EXPECT_EQ(score.future, 0);
        EXPECT_EQ(score.total, 100);
        FreePlannerFixture();
    }
}

SINGLE_BATTLE_TEST("AI joint board evaluation values realized setup pressure and exact speed control")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        s32 rootPressure;
        s32 boostedPressure;
        s32 trickRootPressure;
        s32 trickBoostedPressure;
        u32 aiRoster;

        InitPressureFixture();
        aiRoster = sRosterByBattler[1];
        sContext.mons[aiRoster].moves[0] = MOVE_MOONBLAST;
        sContext.mons[aiRoster].normal.spAttack = 120;
        sContext.mons[aiRoster].normal.speed = 80;
        sBoard.party[aiRoster].pp[0] = 10;
        sScratchBoard = sBoard;
        sScratchBoard.active[1].statStages[STAT_SPATK] += 2;
        sScratchBoard.active[1].statStages[STAT_SPEED] += 2;

        rootPressure = AiJoint_CalculateBoardPressure(&sContext, &sBoard,
                                                      B_SIDE_OPPONENT);
        boostedPressure = AiJoint_CalculateBoardPressure(&sContext,
                                                         &sScratchBoard,
                                                         B_SIDE_OPPONENT);
        EXPECT_GT(boostedPressure, rootPressure);

        // A pure status set gains no fictitious attack pressure from stages.
        sContext.mons[aiRoster].moves[0] = MOVE_GEOMANCY;
        EXPECT_EQ(AiJoint_CalculateBoardPressure(&sContext, &sScratchBoard,
                                                 B_SIDE_OPPONENT), 0);

        // With the damaging move restored, +2 Speed crosses the ordinary
        // order matchup. Under Trick Room the same crossing is a liability.
        sContext.mons[aiRoster].moves[0] = MOVE_MOONBLAST;
        sScratchBoard = sBoard;
        sScratchBoard.fieldStatuses |= STATUS_FIELD_TRICK_ROOM;
        trickRootPressure = AiJoint_CalculateBoardPressure(&sContext,
                                                           &sScratchBoard,
                                                           B_SIDE_OPPONENT);
        sScratchBoard.active[1].statStages[STAT_SPEED] += 2;
        trickBoostedPressure = AiJoint_CalculateBoardPressure(&sContext,
                                                              &sScratchBoard,
                                                              B_SIDE_OPPONENT);
        EXPECT_GT(boostedPressure - rootPressure,
                  trickBoostedPressure - trickRootPressure);
        EXPECT_LT(trickBoostedPressure, trickRootPressure);
        FreePlannerFixture();
    }
}

SINGLE_BATTLE_TEST("AI joint board evaluation distinguishes special bulk and productive item consumption")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        struct AiJointScore consumedScore;
        struct AiJointScore removedScore;
        s32 rootSpecialPressure;
        s32 bulkSpecialPressure;
        s32 rootPhysicalPressure;
        s32 bulkPhysicalPressure;
        u32 playerRoster;
        u32 aiRoster;

        InitPressureFixture();
        playerRoster = sRosterByBattler[0];
        aiRoster = sRosterByBattler[1];
        sContext.mons[playerRoster].moves[0] = MOVE_MOONBLAST;
        sContext.mons[playerRoster].normal.spAttack = 140;
        sScratchBoard = sBoard;
        sScratchBoard.active[1].statStages[STAT_SPDEF] += 2;
        rootSpecialPressure = AiJoint_CalculateBoardPressure(&sContext, &sBoard,
                                                             B_SIDE_OPPONENT);
        bulkSpecialPressure = AiJoint_CalculateBoardPressure(&sContext,
                                                             &sScratchBoard,
                                                             B_SIDE_OPPONENT);
        EXPECT_GT(bulkSpecialPressure, rootSpecialPressure);

        sContext.mons[playerRoster].moves[0] = MOVE_CLOSE_COMBAT;
        sContext.mons[playerRoster].normal.attack = 140;
        rootPhysicalPressure = AiJoint_CalculateBoardPressure(&sContext, &sBoard,
                                                              B_SIDE_OPPONENT);
        bulkPhysicalPressure = AiJoint_CalculateBoardPressure(&sContext,
                                                              &sScratchBoard,
                                                              B_SIDE_OPPONENT);
        EXPECT_EQ(bulkPhysicalPressure, rootPhysicalPressure);

        sBoard.party[aiRoster].item = ITEM_POWER_HERB;
        sScratchBoard = sBoard;
        sScratchBoard.party[aiRoster].item = ITEM_NONE;
        sScratchBoard.party[aiRoster].flags |= AI_SIM_PARTY_ITEM_CONSUMED;
        EXPECT(AiJoint_EvaluateBoard(&sContext, &sBoard, &sScratchBoard,
                                     B_SIDE_OPPONENT, &consumedScore));
        sScratchBoard = sBoard;
        sScratchBoard.party[aiRoster].item = ITEM_NONE;
        sScratchBoard.party[aiRoster].flags |= AI_SIM_PARTY_ITEM_REMOVED;
        EXPECT(AiJoint_EvaluateBoard(&sContext, &sBoard, &sScratchBoard,
                                     B_SIDE_OPPONENT, &removedScore));
        EXPECT_EQ(consumedScore.resource, 0);
        EXPECT_EQ(removedScore.resource, -16);
        EXPECT_GT(consumedScore.total, removedScore.total);
        FreePlannerFixture();
    }
}

SINGLE_BATTLE_TEST("AI joint board evaluation counts only legally resolving damage pressure")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        s32 fakeOutLater;
        s32 fakeOutFirst;
        s32 prioritySlow;
        s32 priorityFast;
        s32 priorityBlocked;
        s32 plainKoPressure;
        s32 sashPressure;
        s32 sturdyPressure;
        s32 negativeSlow;
        s32 negativeFast;
        u32 aiRoster;
        u32 targetRoster;

        InitPressureFixture();
        aiRoster = sRosterByBattler[1];
        targetRoster = sRosterByBattler[0];
        sBoard.party[sRosterByBattler[2]].hp = 0;
        sContext.mons[aiRoster].moves[0] = MOVE_FAKE_OUT;
        fakeOutLater = AiJoint_CalculateBoardPressure(&sContext, &sBoard,
                                                       B_SIDE_OPPONENT);
        sScratchBoard = sBoard;
        sScratchBoard.active[1].firstTurn = TRUE;
        fakeOutFirst = AiJoint_CalculateBoardPressure(&sContext, &sScratchBoard,
                                                       B_SIDE_OPPONENT);
        EXPECT_EQ(fakeOutLater, 0);
        EXPECT_GT(fakeOutFirst, fakeOutLater);

        sContext.mons[aiRoster].moves[0] = MOVE_SHADOW_SNEAK;
        sContext.mons[targetRoster].normal.types[0] = TYPE_PSYCHIC;
        sContext.mons[aiRoster].normal.attack = 180;
        sContext.mons[aiRoster].normal.speed = 20;
        sContext.mons[targetRoster].normal.speed = 200;
        prioritySlow = AiJoint_CalculateBoardPressure(&sContext, &sBoard,
                                                       B_SIDE_OPPONENT);
        sContext.mons[aiRoster].normal.speed = 240;
        priorityFast = AiJoint_CalculateBoardPressure(&sContext, &sBoard,
                                                       B_SIDE_OPPONENT);
        sScratchBoard = sBoard;
        sScratchBoard.fieldStatuses |= STATUS_FIELD_PSYCHIC_TERRAIN;
        priorityBlocked = AiJoint_CalculateBoardPressure(&sContext, &sScratchBoard,
                                                          B_SIDE_OPPONENT);
        EXPECT_GT(prioritySlow, 0);
        EXPECT_EQ(priorityFast, prioritySlow);
        EXPECT_EQ(priorityBlocked, 0);

        sContext.mons[aiRoster].moves[0] = MOVE_CLOSE_COMBAT;
        sContext.mons[aiRoster].normal.attack = 1000;
        sContext.mons[targetRoster].normal.defense = 10;
        sContext.mons[targetRoster].normal.ability = ABILITY_NONE;
        sBoard.party[targetRoster].item = ITEM_NONE;
        plainKoPressure = AiJoint_CalculateBoardPressure(&sContext, &sBoard,
                                                          B_SIDE_OPPONENT);
        sScratchBoard = sBoard;
        sScratchBoard.party[targetRoster].item = ITEM_FOCUS_SASH;
        sashPressure = AiJoint_CalculateBoardPressure(&sContext, &sScratchBoard,
                                                       B_SIDE_OPPONENT);
        sContext.mons[targetRoster].normal.ability = ABILITY_STURDY;
        sturdyPressure = AiJoint_CalculateBoardPressure(&sContext, &sBoard,
                                                         B_SIDE_OPPONENT);
        EXPECT_GT(plainKoPressure, sashPressure);
        EXPECT_EQ(sturdyPressure, sashPressure);

        sContext.mons[targetRoster].normal.ability = ABILITY_NONE;
        sContext.mons[aiRoster].moves[0] = MOVE_VITAL_THROW;
        sContext.mons[aiRoster].normal.speed = 20;
        negativeSlow = AiJoint_CalculateBoardPressure(&sContext, &sBoard,
                                                       B_SIDE_OPPONENT);
        sContext.mons[aiRoster].normal.speed = 240;
        negativeFast = AiJoint_CalculateBoardPressure(&sContext, &sBoard,
                                                       B_SIDE_OPPONENT);
        EXPECT_GT(negativeSlow, 0);
        EXPECT_EQ(negativeFast, negativeSlow);
        FreePlannerFixture();
    }
}
