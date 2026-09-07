#include "global.h"
#include "test/battle.h"
#include "battle_ai_main.h"
#include "battle_ai_joint_planner.h"
#include "battle_ai_joint_runtime.h"
#include "battle_ai_util.h"
#include "battle_setup.h"
#include "constants/battle.h"
#include "constants/battle_ai.h"

SINGLE_BATTLE_TEST("AI runtime knowledge maps move categories from move data")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_TACKLE); }
    } THEN {
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_TACKLE, AI_MOVE_KNOWLEDGE_CONTACT));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_BOOMBURST, AI_MOVE_KNOWLEDGE_SOUND));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_BULLET_SEED, AI_MOVE_KNOWLEDGE_BALLISTIC));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_SPORE, AI_MOVE_KNOWLEDGE_POWDER));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_AQUA_CUTTER, AI_MOVE_KNOWLEDGE_SLICING));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_DRAIN_PUNCH, AI_MOVE_KNOWLEDGE_PUNCHING));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_BITE, AI_MOVE_KNOWLEDGE_BITING));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_HEAL_PULSE, AI_MOVE_KNOWLEDGE_PULSE));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_DRAGON_DANCE, AI_MOVE_KNOWLEDGE_DANCE));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_AIR_CUTTER, AI_MOVE_KNOWLEDGE_WIND));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_RECOVER, AI_MOVE_KNOWLEDGE_HEALING));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_TAUNT, AI_MOVE_KNOWLEDGE_MAGIC_COAT));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_SWORDS_DANCE, AI_MOVE_KNOWLEDGE_SNATCH));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_SKILL_SWAP, AI_MOVE_KNOWLEDGE_ABILITY_CONTROL));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_IMPRISON, AI_MOVE_KNOWLEDGE_MOVE_DENIAL));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_DEFENSE_CURL, AI_MOVE_KNOWLEDGE_COMBO_STATE));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_MAX_AIRSTREAM, AI_MOVE_KNOWLEDGE_MAX_SPEED_CONTROL));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_MAX_FLARE, AI_MOVE_KNOWLEDGE_MAX_WEATHER));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_MAX_LIGHTNING, AI_MOVE_KNOWLEDGE_MAX_TERRAIN));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_MAX_KNUCKLE, AI_MOVE_KNOWLEDGE_MAX_STAT_CONTROL));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_G_MAX_WILDFIRE, AI_MOVE_KNOWLEDGE_GMAX_UNIQUE));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_G_MAX_WILDFIRE, AI_MOVE_KNOWLEDGE_GMAX_RESIDUAL));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_G_MAX_BEFUDDLE, AI_MOVE_KNOWLEDGE_GMAX_UNIQUE));
        EXPECT(!AI_MoveHasKnowledgeFlag(MOVE_G_MAX_BEFUDDLE, AI_MOVE_KNOWLEDGE_GMAX_RESIDUAL));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_G_MAX_STONESURGE, AI_MOVE_KNOWLEDGE_GMAX_HAZARD));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_G_MAX_STEELSURGE, AI_MOVE_KNOWLEDGE_GMAX_HAZARD));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_LEECH_SEED, AI_MOVE_KNOWLEDGE_Z_STATUS));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_LEECH_SEED, AI_MOVE_KNOWLEDGE_Z_STAT_RESET));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_HAPPY_HOUR, AI_MOVE_KNOWLEDGE_Z_STAT_BOOST));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_DETECT, AI_MOVE_KNOWLEDGE_Z_STAT_BOOST));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_TAILWIND, AI_MOVE_KNOWLEDGE_Z_CRIT_BOOST));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_DESTINY_BOND, AI_MOVE_KNOWLEDGE_Z_REDIRECTION));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_HAZE, AI_MOVE_KNOWLEDGE_Z_RECOVERY));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_PARTING_SHOT, AI_MOVE_KNOWLEDGE_Z_REPLACEMENT_HEAL));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_CURSE, AI_MOVE_KNOWLEDGE_Z_STAT_BOOST));
        EXPECT(AI_MoveHasKnowledgeFlag(MOVE_CURSE, AI_MOVE_KNOWLEDGE_Z_RECOVERY));
    }
}

SINGLE_BATTLE_TEST("Battle AI trace stores a compact plan and links its selected action")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        enum BattlerId playerBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId opponentBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        struct BattleAiTracePlan tracePlan = {0};
        struct BattleAiTraceCandidate traceCandidate = {0};
        const struct BattleAiTracePlan *loggedPlan;
        const struct BattleAiTraceCandidate *loggedCandidate;
        const struct BattleAiTraceBoard *loggedBoard;
        const struct BattleActionLogEntry *linkedAction;
        u16 planId;

        BattleAiTrace_InitAction(&tracePlan.predictedPlayerActions[0], playerBattler, B_ACTION_USE_MOVE, MOVE_CELEBRATE, opponentBattler, 0, GIMMICK_NONE, BATTLE_AI_TRACE_PREDICTION_CONFIRMED_COMMAND);
        BattleAiTrace_InitAction(&traceCandidate.actions[0], opponentBattler, B_ACTION_USE_MOVE, MOVE_CELEBRATE, playerBattler, 0, GIMMICK_NONE, BATTLE_AI_TRACE_PREDICTION_NONE);
        tracePlan.actorMask = 1u << opponentBattler;
        tracePlan.chosenRank = 0;
        tracePlan.requestedDepth = 1;
        tracePlan.completedDepth = 1;
        tracePlan.flags = BATTLE_AI_TRACE_PLAN_LEGACY_EVALUATOR | BATTLE_AI_TRACE_PLAN_COMPONENTS_PARTIAL;
        traceCandidate.totalScore = 100;
        traceCandidate.immediateScore = 100;
        traceCandidate.readInteractionFlags[0] = AI_READ_INTERACTION_KNOWN_KO;
        traceCandidate.allyInteractionKinds[0] = AI_ALLY_INTERACTION_SUPPORT;
        traceCandidate.completedDepth = 1;
        traceCandidate.flags = BATTLE_AI_TRACE_CANDIDATE_COMPLETE | BATTLE_AI_TRACE_CANDIDATE_COMPONENTS_PARTIAL;

        planId = BattleAiTrace_RecordPlan(&tracePlan, &traceCandidate, 1);
        BattleAiTrace_SetBattlerDecision(opponentBattler, planId, 0);
        EXPECT_EQ(gAiBattleData->decisionPlanId[opponentBattler], planId);
        EXPECT_EQ(gAiBattleData->decisionCandidateRank[opponentBattler], 0);
        BattleActionLog_RecordSwitchIn(opponentBattler, 0, FALSE);

        loggedPlan = BattleAiTrace_GetPlan(planId);
        EXPECT(loggedPlan != NULL);
        EXPECT_EQ(loggedPlan->candidateCount, 1);
        EXPECT_EQ(loggedPlan->chosenRank, 0);
        EXPECT(loggedPlan->flags & BATTLE_AI_TRACE_PLAN_VALID);
        loggedCandidate = BattleAiTrace_GetCandidate(loggedPlan->firstCandidateSequence);
        EXPECT(loggedCandidate != NULL);
        EXPECT_EQ(loggedCandidate->totalScore, 100);
        EXPECT_EQ(loggedCandidate->readInteractionFlags[0], AI_READ_INTERACTION_KNOWN_KO);
        EXPECT_EQ(loggedCandidate->allyInteractionKinds[0], AI_ALLY_INTERACTION_SUPPORT);
        EXPECT(loggedCandidate->flags & BATTLE_AI_TRACE_CANDIDATE_CHOSEN);
        loggedBoard = BattleAiTrace_GetBoard(loggedPlan->boardSequence);
        EXPECT(loggedBoard != NULL);
        EXPECT_EQ(loggedBoard->planId, planId);
        EXPECT_EQ(loggedBoard->meta & BATTLE_AI_TRACE_BOARD_PHASE_MASK, BATTLE_AI_TRACE_BOARD_PHASE_BEFORE);
        linkedAction = BattleActionLog_GetLastEntry(opponentBattler, 1u << B_ACTION_SWITCH);
        EXPECT(linkedAction != NULL);
        EXPECT_EQ(linkedAction->aiPlanId, planId);
        EXPECT_EQ(linkedAction->aiCandidateRank, 0);
    }
}

TEST("Battle AI trace compares predicted and actual board snapshots")
{
    struct BattleAiTraceBoard predicted = {0};
    struct BattleAiTraceBoard actual = {0};
    u16 originalWeather = gBattleWeather;
    u8 originalBattlersCount = gBattlersCount;

    predicted.planId = 1;
    predicted.meta = BATTLE_AI_TRACE_BOARD_VALID | (1 << BATTLE_AI_TRACE_BOARD_BATTLER_MASK_SHIFT);
    actual = predicted;
    EXPECT_EQ(BattleAiTrace_CompareBoards(&predicted, &actual), BATTLE_AI_TRACE_BOARD_DIFF_NONE);

    actual.weather = 1;
    EXPECT_EQ(BattleAiTrace_CompareBoards(&predicted, &actual), BATTLE_AI_TRACE_BOARD_DIFF_WEATHER);
    actual = predicted;
    actual.battlers[0].hp = 1;
    EXPECT_EQ(BattleAiTrace_CompareBoards(&predicted, &actual), BATTLE_AI_TRACE_BOARD_DIFF_BATTLER);
    actual = predicted;
    actual.tailwindTimers[0] = 1;
    EXPECT_EQ(BattleAiTrace_CompareBoards(&predicted, &actual), BATTLE_AI_TRACE_BOARD_DIFF_TIMERS);
    actual = predicted;
    actual.meta &= ~BATTLE_AI_TRACE_BOARD_BATTLER_MASK;
    EXPECT_EQ(BattleAiTrace_CompareBoards(&predicted, &actual), BATTLE_AI_TRACE_BOARD_DIFF_BATTLER_MASK);
    EXPECT(BattleAiTrace_CompareBoards(NULL, &actual) & BATTLE_AI_TRACE_BOARD_DIFF_INVALID);

    BattleAiTrace_Clear();
    gBattlersCount = 0;
    gBattleWeather = 1;
    BattleAiTrace_CaptureBoard(1, BATTLE_AI_TRACE_BOARD_PHASE_PREDICTED_AFTER);
    gBattleWeather = 2;
    BattleAiTrace_CaptureBoard(1, BATTLE_AI_TRACE_BOARD_PHASE_ACTUAL_AFTER);
    EXPECT_EQ(BattleAiTrace_ComparePlanBoards(1), BATTLE_AI_TRACE_BOARD_DIFF_WEATHER);

    BattleAiTrace_Clear();
    gBattleWeather = originalWeather;
    gBattlersCount = originalBattlersCount;
}

TEST("Battle AI trace rings preserve retained plans across cursor and sequence wrap")
{
        struct BattleAiTracePlan tracePlan = {0};
        struct BattleAiTraceCandidate traceCandidates[BATTLE_AI_TRACE_TOP_CANDIDATES] = {0};
        u16 planIds[BATTLE_AI_TRACE_PLAN_ENTRIES + 1];
        u16 firstCandidateSequences[BATTLE_AI_TRACE_PLAN_ENTRIES + 1];
        u16 beforeBoardSequences[BATTLE_AI_TRACE_PLAN_ENTRIES + 1];
        u16 predictedBoardSequences[BATTLE_AI_TRACE_PLAN_ENTRIES + 1];
        u16 actualBoardSequences[BATTLE_AI_TRACE_PLAN_ENTRIES + 1];

        BattleAiTrace_Clear();
        EXPECT_EQ(gBattleAiTraceLog.schemaVersion, BATTLE_AI_TRACE_SCHEMA_VERSION);
        EXPECT_EQ(gBattleAiTraceLog.magic, BATTLE_AI_TRACE_HEADER_MAGIC);

        // Start each counter close to rollover so this also verifies that zero,
        // the sentinel for an absent trace link, is skipped by every ring.
        gBattleAiTraceLog.planSequence = 0xFFFE;
        gBattleAiTraceLog.candidateSequence = 0xFFF8;
        gBattleAiTraceLog.boardSequence = 0xFFFC;
        tracePlan.chosenRank = 0;
        for (u32 rank = 0; rank < ARRAY_COUNT(traceCandidates); rank++)
        {
            traceCandidates[rank].totalScore = rank;
            traceCandidates[rank].completedDepth = 1;
            traceCandidates[rank].flags = BATTLE_AI_TRACE_CANDIDATE_COMPLETE;
        }

        for (u32 i = 0; i < ARRAY_COUNT(planIds); i++)
        {
            const struct BattleAiTracePlan *loggedPlan;

            planIds[i] = BattleAiTrace_RecordPlan(&tracePlan, traceCandidates, ARRAY_COUNT(traceCandidates));
            loggedPlan = BattleAiTrace_GetPlan(planIds[i]);
            EXPECT(loggedPlan != NULL);
            firstCandidateSequences[i] = loggedPlan->firstCandidateSequence;
            beforeBoardSequences[i] = loggedPlan->boardSequence;
            predictedBoardSequences[i] = BattleAiTrace_CaptureBoard(planIds[i], BATTLE_AI_TRACE_BOARD_PHASE_PREDICTED_AFTER);
            actualBoardSequences[i] = BattleAiTrace_CaptureBoard(planIds[i], BATTLE_AI_TRACE_BOARD_PHASE_ACTUAL_AFTER);
        }

        EXPECT_EQ(planIds[0], 0xFFFF);
        EXPECT_EQ(planIds[1], 1);
        EXPECT_EQ(firstCandidateSequences[0], 0xFFF9);
        EXPECT_EQ(firstCandidateSequences[1], 2);
        EXPECT_EQ(beforeBoardSequences[0], 0xFFFD);
        EXPECT_EQ(beforeBoardSequences[1], 1);
        EXPECT_EQ(gBattleAiTraceLog.planCount, BATTLE_AI_TRACE_PLAN_ENTRIES);
        EXPECT_EQ(gBattleAiTraceLog.candidateCount, BATTLE_AI_TRACE_CANDIDATE_ENTRIES);
        EXPECT_EQ(gBattleAiTraceLog.boardCount, BATTLE_AI_TRACE_BOARD_ENTRIES);
        EXPECT_EQ(gBattleAiTraceLog.planCursor, 1);
        EXPECT_EQ(gBattleAiTraceLog.candidateCursor, BATTLE_AI_TRACE_TOP_CANDIDATES);
        EXPECT_EQ(gBattleAiTraceLog.boardCursor, 3);

        // The overwritten oldest plan and all three of its board links
        // disappear together.  Every retained plan still has all candidates
        // and before/predicted/actual board snapshots available.
        EXPECT(BattleAiTrace_GetPlan(planIds[0]) == NULL);
        EXPECT(BattleAiTrace_GetCandidate(firstCandidateSequences[0]) == NULL);
        EXPECT(BattleAiTrace_GetBoard(beforeBoardSequences[0]) == NULL);
        for (u32 i = 1; i < ARRAY_COUNT(planIds); i++)
        {
            const struct BattleAiTracePlan *loggedPlan = BattleAiTrace_GetPlan(planIds[i]);
            u16 candidateSequence = firstCandidateSequences[i];

            EXPECT(loggedPlan != NULL);
            EXPECT_EQ(loggedPlan->boardSequence, beforeBoardSequences[i]);
            EXPECT(BattleAiTrace_GetBoard(beforeBoardSequences[i]) != NULL);
            EXPECT(BattleAiTrace_GetBoard(predictedBoardSequences[i]) != NULL);
            EXPECT(BattleAiTrace_GetBoard(actualBoardSequences[i]) != NULL);
            for (u32 rank = 0; rank < BATTLE_AI_TRACE_TOP_CANDIDATES; rank++)
            {
                EXPECT(BattleAiTrace_GetCandidate(candidateSequence) != NULL);
                candidateSequence++;
                if (candidateSequence == BATTLE_AI_TRACE_ID_NONE)
                    candidateSequence++;
            }
        }
}

AI_SINGLE_BATTLE_TEST("Battle AI trace links a legacy final switch instead of its scored move")
{
    PASSES_RANDOMLY(SHOULD_SWITCH_HASBADODDS_PERCENTAGE, 100, RNG_AI_SWITCH_HASBADODDS);
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_MAGNITUDE) == EFFECT_MAGNITUDE);
        ASSUME(GetMoveType(MOVE_MAGNITUDE) == TYPE_GROUND);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE
               | AI_FLAG_TRY_TO_FAINT
               | AI_FLAG_CHECK_VIABILITY
               | AI_FLAG_SMART_SWITCHING
               | AI_FLAG_SMART_MON_CHOICES
               | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_GEODUDE) { Level(15); Moves(MOVE_DEFENSE_CURL, MOVE_MAGNITUDE); }
        OPPONENT(SPECIES_TYRUNT) { Level(13); Moves(MOVE_BITE, MOVE_THUNDER_FANG, MOVE_ROCK_TOMB); }
        OPPONENT(SPECIES_ZUBAT) { Level(14); Moves(MOVE_TACKLE); }
    } WHEN {
        TURN { MOVE(player, MOVE_MAGNITUDE); EXPECT_SWITCH(opponent, 1); }
    } THEN {
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        const struct BattleActionLogEntry *entry =
            BattleActionLog_GetLastEntry(battler, 1u << B_ACTION_SWITCH);
        const struct BattleAiTracePlan *plan;
        const struct BattleAiTraceCandidate *candidate;
        u16 candidateSequence;

        EXPECT(entry != NULL);
        EXPECT_NE(entry->aiPlanId, BATTLE_AI_TRACE_ID_NONE);
        plan = BattleAiTrace_GetPlan(entry->aiPlanId);
        EXPECT(plan != NULL);
        EXPECT(plan->flags & BATTLE_AI_TRACE_PLAN_LEGACY_EVALUATOR);
        EXPECT_EQ(entry->aiCandidateRank, plan->chosenRank);

        candidateSequence = plan->firstCandidateSequence;
        for (u32 rank = 0; rank < plan->chosenRank; rank++)
        {
            candidateSequence++;
            if (candidateSequence == BATTLE_AI_TRACE_ID_NONE)
                candidateSequence++;
        }
        candidate = BattleAiTrace_GetCandidate(candidateSequence);
        EXPECT(candidate != NULL);
        EXPECT(candidate->flags & BATTLE_AI_TRACE_CANDIDATE_CHOSEN);
        EXPECT(candidate->flags & BATTLE_AI_TRACE_CANDIDATE_FORCED_TRACE);
        EXPECT(candidate->actions[0].actorMeta & BATTLE_AI_TRACE_ACTION_VALID);
        EXPECT_EQ((candidate->actions[0].actorMeta & BATTLE_AI_TRACE_ACTION_KIND_MASK)
                >> BATTLE_AI_TRACE_ACTION_KIND_SHIFT,
                  BATTLE_AI_TRACE_ACTION_SWITCH);
        EXPECT_EQ(candidate->actions[0].choice, 1);
        EXPECT_EQ(entry->partyIndex, candidate->actions[0].choice);
    }
}

AI_SINGLE_BATTLE_TEST("Battle AI trace links a legacy final move after command refresh")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE
               | AI_FLAG_TRY_TO_FAINT
               | AI_FLAG_CHECK_VIABILITY
               | AI_FLAG_READ_PLAYER_MOVE
               | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_TACKLE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); EXPECT_MOVE(opponent, MOVE_TACKLE); }
    } THEN {
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        const struct BattleActionLogEntry *entry =
            BattleActionLog_GetLastEntry(battler, 1u << B_ACTION_USE_MOVE);
        const struct BattleAiTracePlan *plan;
        const struct BattleAiTraceCandidate *candidate;
        u16 candidateSequence;

        EXPECT(entry != NULL);
        EXPECT_NE(entry->aiPlanId, BATTLE_AI_TRACE_ID_NONE);
        plan = BattleAiTrace_GetPlan(entry->aiPlanId);
        EXPECT(plan != NULL);
        EXPECT(plan->flags & BATTLE_AI_TRACE_PLAN_LEGACY_EVALUATOR);
        EXPECT_EQ(entry->aiCandidateRank, plan->chosenRank);

        candidateSequence = plan->firstCandidateSequence;
        for (u32 rank = 0; rank < plan->chosenRank; rank++)
        {
            candidateSequence++;
            if (candidateSequence == BATTLE_AI_TRACE_ID_NONE)
                candidateSequence++;
        }
        candidate = BattleAiTrace_GetCandidate(candidateSequence);
        EXPECT(candidate != NULL);
        EXPECT(candidate->flags & BATTLE_AI_TRACE_CANDIDATE_CHOSEN);
        EXPECT_EQ((candidate->actions[0].actorMeta & BATTLE_AI_TRACE_ACTION_KIND_MASK)
                >> BATTLE_AI_TRACE_ACTION_KIND_SHIFT,
                  BATTLE_AI_TRACE_ACTION_MOVE);
        EXPECT_EQ(candidate->actions[0].choice, MOVE_TACKLE);
        EXPECT_EQ(candidate->actions[0].choice, entry->move);
        EXPECT_EQ(candidate->actions[0].targetMeta & BATTLE_AI_TRACE_ACTION_TARGET_MASK,
                  entry->target);
    }
}

AI_DOUBLE_BATTLE_TEST("Joint runtime computes one shared complete plan in either opponent controller order")
{
    u32 reverseLogicChance;

    PARAMETRIZE { reverseLogicChance = 0; }
    PARAMETRIZE { reverseLogicChance = 100; }

    GIVEN {
        WITH_CONFIG(AI_REVERSE_BATTLER_LOGIC_ORDER_CHANCE, reverseLogicChance);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE
               | AI_FLAG_CHECK_VIABILITY
               | AI_FLAG_TRY_TO_FAINT
               | AI_FLAG_READ_PLAYER_MOVE
               | AI_FLAG_DOUBLE_BATTLE
               | AI_FLAG_OMNISCIENT
               | AI_FLAG_SMART_MON_CHOICES);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Item(ITEM_POWER_HERB); Speed(200); Moves(MOVE_GEOMANCY); }
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Item(ITEM_POWER_HERB); Speed(180); Moves(MOVE_GEOMANCY); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Item(ITEM_POWER_HERB); Speed(60); Moves(MOVE_GEOMANCY); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Item(ITEM_POWER_HERB); Speed(40); Moves(MOVE_GEOMANCY); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_GEOMANCY);
            MOVE(playerRight, MOVE_GEOMANCY);
            EXPECT_MOVE(opponentLeft, MOVE_GEOMANCY);
            EXPECT_MOVE(opponentRight, MOVE_GEOMANCY);
        }
    } THEN {
        enum BattlerId left = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId right = GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT);
        const struct BattleActionLogEntry *leftAction =
            BattleActionLog_GetLastEntry(left, 1u << B_ACTION_USE_MOVE);
        const struct BattleActionLogEntry *rightAction =
            BattleActionLog_GetLastEntry(right, 1u << B_ACTION_USE_MOVE);
        const struct BattleAiTracePlan *plan;

        EXPECT(leftAction != NULL);
        EXPECT(rightAction != NULL);
        EXPECT_NE(leftAction->aiPlanId, BATTLE_AI_TRACE_ID_NONE);
        EXPECT_EQ(leftAction->aiPlanId, rightAction->aiPlanId);
        EXPECT_EQ(Test_BattleAiJointRuntime_GetBuildCount(), 1);
        EXPECT(Test_BattleAiJointRuntime_GetStepCount() > 0);
        EXPECT_EQ(Test_BattleAiJointRuntime_GetPlanId(), leftAction->aiPlanId);
        EXPECT(!Test_BattleAiJointRuntime_IsAllocated());
        plan = BattleAiTrace_GetPlan(leftAction->aiPlanId);
        EXPECT(plan != NULL);
        EXPECT(plan->flags & BATTLE_AI_TRACE_PLAN_VALID);
        EXPECT(plan->flags & BATTLE_AI_TRACE_PLAN_JOINT);
        EXPECT(!(plan->flags & BATTLE_AI_TRACE_PLAN_LEGACY_EVALUATOR));
        EXPECT(!(plan->flags & BATTLE_AI_TRACE_PLAN_NODE_BUDGET_HIT));
        EXPECT(!(plan->flags & BATTLE_AI_TRACE_PLAN_FRAME_BUDGET_HIT));
        EXPECT_EQ(plan->actorMask, (1u << left) | (1u << right));
        EXPECT_GE(plan->requestedDepth, AI_JOINT_STANDARD_DEPTH);
        EXPECT_EQ(plan->completedDepth, plan->requestedDepth);
        EXPECT_EQ(plan->terminationReason, BATTLE_AI_TRACE_TERMINATION_COMPLETE);
        EXPECT(plan->nodesVisited <= AI_JOINT_DEFAULT_TOTAL_NODES);
        EXPECT(plan->elapsedFrames <= AI_JOINT_DEFAULT_TOTAL_FRAMES);
        EXPECT_EQ(plan->nodeBudget, AI_JOINT_DEFAULT_TOTAL_NODES);
        EXPECT_EQ(plan->frameBudget, AI_JOINT_DEFAULT_TOTAL_FRAMES);
        EXPECT(plan->chosenRank < plan->candidateCount);
    }
}

AI_DOUBLE_BATTLE_TEST("Joint runtime promptly falls back when the exact all-Tackle frontier exceeds capacity")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE
               | AI_FLAG_CHECK_VIABILITY
               | AI_FLAG_TRY_TO_FAINT
               | AI_FLAG_READ_PLAYER_MOVE
               | AI_FLAG_DOUBLE_BATTLE
               | AI_FLAG_OMNISCIENT
               | AI_FLAG_SMART_MON_CHOICES);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Moves(MOVE_TACKLE); }
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Moves(MOVE_TACKLE); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_TACKLE, target: opponentLeft);
            MOVE(playerRight, MOVE_TACKLE, target: opponentRight);
            EXPECT_MOVE(opponentLeft, MOVE_TACKLE);
            EXPECT_MOVE(opponentRight, MOVE_TACKLE);
        }
    } THEN {
        enum BattlerId left = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId right = GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT);
        const struct BattleActionLogEntry *leftAction =
            BattleActionLog_GetLastEntry(left, 1u << B_ACTION_USE_MOVE);
        const struct BattleActionLogEntry *rightAction =
            BattleActionLog_GetLastEntry(right, 1u << B_ACTION_USE_MOVE);
        const struct BattleAiTracePlan *leftPlan;
        const struct BattleAiTracePlan *rightPlan;

        EXPECT_EQ(Test_BattleAiJointRuntime_GetBuildCount(), 1);
        EXPECT(Test_BattleAiJointRuntime_GetStepCount() > 0);
        EXPECT_EQ(Test_BattleAiJointRuntime_GetPlanId(), BATTLE_AI_TRACE_ID_NONE);
        EXPECT(!Test_BattleAiJointRuntime_IsAllocated());
        EXPECT(leftAction != NULL);
        EXPECT(rightAction != NULL);
        EXPECT_NE(leftAction->aiPlanId, BATTLE_AI_TRACE_ID_NONE);
        EXPECT_NE(rightAction->aiPlanId, BATTLE_AI_TRACE_ID_NONE);
        leftPlan = BattleAiTrace_GetPlan(leftAction->aiPlanId);
        rightPlan = BattleAiTrace_GetPlan(rightAction->aiPlanId);
        EXPECT(leftPlan != NULL);
        EXPECT(rightPlan != NULL);
        EXPECT(leftPlan->flags & BATTLE_AI_TRACE_PLAN_LEGACY_EVALUATOR);
        EXPECT(rightPlan->flags & BATTLE_AI_TRACE_PLAN_LEGACY_EVALUATOR);
        EXPECT(!(leftPlan->flags & BATTLE_AI_TRACE_PLAN_JOINT));
        EXPECT(!(rightPlan->flags & BATTLE_AI_TRACE_PLAN_JOINT));
    }
}

AI_DOUBLE_BATTLE_TEST("Joint runtime searches a real prospective Mega profile")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE
               | AI_FLAG_CHECK_VIABILITY
               | AI_FLAG_TRY_TO_FAINT
               | AI_FLAG_READ_PLAYER_MOVE
               | AI_FLAG_DOUBLE_BATTLE
               | AI_FLAG_OMNISCIENT
               | AI_FLAG_SMART_MON_CHOICES);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Item(ITEM_POWER_HERB); Speed(200); Moves(MOVE_GEOMANCY); }
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Item(ITEM_POWER_HERB); Speed(180); Moves(MOVE_GEOMANCY); }
        OPPONENT(SPECIES_SCIZOR) { Ability(ABILITY_TECHNICIAN); Item(ITEM_SCIZORITE); Speed(100); Moves(MOVE_GEOMANCY); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Item(ITEM_POWER_HERB); Speed(40); Moves(MOVE_GEOMANCY); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_GEOMANCY);
            MOVE(playerRight, MOVE_GEOMANCY);
            EXPECT_MOVE(opponentLeft, MOVE_GEOMANCY);
            EXPECT_MOVE(opponentRight, MOVE_GEOMANCY);
        }
    } THEN {
        enum BattlerId left = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId right = GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT);
        const struct BattleActionLogEntry *leftAction =
            BattleActionLog_GetLastEntry(left, 1u << B_ACTION_USE_MOVE);
        const struct BattleActionLogEntry *rightAction =
            BattleActionLog_GetLastEntry(right, 1u << B_ACTION_USE_MOVE);
        const struct BattleAiTracePlan *plan;

        EXPECT(leftAction != NULL);
        EXPECT(rightAction != NULL);
        EXPECT_NE(leftAction->aiPlanId, BATTLE_AI_TRACE_ID_NONE);
        EXPECT_EQ(leftAction->aiPlanId, rightAction->aiPlanId);
        plan = BattleAiTrace_GetPlan(leftAction->aiPlanId);
        EXPECT(plan != NULL);
        EXPECT(plan->flags & BATTLE_AI_TRACE_PLAN_JOINT);
        EXPECT(!(plan->flags & BATTLE_AI_TRACE_PLAN_LEGACY_EVALUATOR));
    }
}

AI_DOUBLE_BATTLE_TEST("Joint runtime fails closed on an unsupported prospective Mega ability")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE
               | AI_FLAG_CHECK_VIABILITY
               | AI_FLAG_TRY_TO_FAINT
               | AI_FLAG_READ_PLAYER_MOVE
               | AI_FLAG_DOUBLE_BATTLE
               | AI_FLAG_OMNISCIENT
               | AI_FLAG_SMART_MON_CHOICES);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Moves(MOVE_TACKLE); }
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_SABLEYE) { Ability(ABILITY_PRANKSTER); Item(ITEM_SABLENITE); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Moves(MOVE_TACKLE); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_TACKLE, target: opponentLeft);
            MOVE(playerRight, MOVE_TACKLE, target: opponentRight);
            EXPECT_MOVE(opponentLeft, MOVE_TACKLE);
            EXPECT_MOVE(opponentRight, MOVE_TACKLE);
        }
    } THEN {
        enum BattlerId left = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId right = GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT);
        const struct BattleActionLogEntry *leftAction =
            BattleActionLog_GetLastEntry(left, 1u << B_ACTION_USE_MOVE);
        const struct BattleActionLogEntry *rightAction =
            BattleActionLog_GetLastEntry(right, 1u << B_ACTION_USE_MOVE);
        const struct BattleAiTracePlan *leftPlan;
        const struct BattleAiTracePlan *rightPlan;

        EXPECT(leftAction != NULL);
        EXPECT(rightAction != NULL);
        leftPlan = BattleAiTrace_GetPlan(leftAction->aiPlanId);
        rightPlan = BattleAiTrace_GetPlan(rightAction->aiPlanId);
        EXPECT(leftPlan != NULL);
        EXPECT(rightPlan != NULL);
        EXPECT(leftPlan->flags & BATTLE_AI_TRACE_PLAN_LEGACY_EVALUATOR);
        EXPECT(rightPlan->flags & BATTLE_AI_TRACE_PLAN_LEGACY_EVALUATOR);
        EXPECT(!(leftPlan->flags & BATTLE_AI_TRACE_PLAN_JOINT));
        EXPECT(!(rightPlan->flags & BATTLE_AI_TRACE_PLAN_JOINT));
    }
}

AI_DOUBLE_BATTLE_TEST("Joint runtime tiny budget fails closed to the repaired legacy actions")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE
               | AI_FLAG_CHECK_VIABILITY
               | AI_FLAG_TRY_TO_FAINT
               | AI_FLAG_READ_PLAYER_MOVE
               | AI_FLAG_DOUBLE_BATTLE
               | AI_FLAG_OMNISCIENT
               | AI_FLAG_SMART_MON_CHOICES);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Moves(MOVE_TACKLE); }
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Moves(MOVE_TACKLE); }
    } WHEN {
        Test_BattleAiJointRuntime_SetLimits(1, 1, 1, 1);
        TURN {
            MOVE(playerLeft, MOVE_TACKLE, target: opponentLeft);
            MOVE(playerRight, MOVE_TACKLE, target: opponentRight);
            EXPECT_MOVE(opponentLeft, MOVE_TACKLE);
            EXPECT_MOVE(opponentRight, MOVE_TACKLE);
        }
    } THEN {
        enum BattlerId left = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId right = GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT);
        const struct BattleActionLogEntry *leftAction =
            BattleActionLog_GetLastEntry(left, 1u << B_ACTION_USE_MOVE);
        const struct BattleActionLogEntry *rightAction =
            BattleActionLog_GetLastEntry(right, 1u << B_ACTION_USE_MOVE);
        const struct BattleAiTracePlan *leftPlan;
        const struct BattleAiTracePlan *rightPlan;

        EXPECT_EQ(Test_BattleAiJointRuntime_GetBuildCount(), 1);
        EXPECT_EQ(Test_BattleAiJointRuntime_GetPlanId(), BATTLE_AI_TRACE_ID_NONE);
        EXPECT(!Test_BattleAiJointRuntime_IsAllocated());
        EXPECT(leftAction != NULL);
        EXPECT(rightAction != NULL);
        EXPECT_NE(leftAction->aiPlanId, BATTLE_AI_TRACE_ID_NONE);
        EXPECT_NE(rightAction->aiPlanId, BATTLE_AI_TRACE_ID_NONE);
        leftPlan = BattleAiTrace_GetPlan(leftAction->aiPlanId);
        rightPlan = BattleAiTrace_GetPlan(rightAction->aiPlanId);
        EXPECT(leftPlan != NULL);
        EXPECT(rightPlan != NULL);
        if (leftPlan != NULL)
        {
            const struct BattleAiTraceCandidate *chosen = BattleAiTrace_GetCandidate(
                leftPlan->firstCandidateSequence + leftAction->aiCandidateRank);

            EXPECT(leftPlan->flags & BATTLE_AI_TRACE_PLAN_LEGACY_EVALUATOR);
            EXPECT(!(leftPlan->flags & BATTLE_AI_TRACE_PLAN_JOINT));
            EXPECT(chosen != NULL);
            if (chosen != NULL)
            {
                EXPECT_EQ(chosen->actions[0].actorMeta & BATTLE_AI_TRACE_ACTION_ACTOR_MASK,
                          left);
                EXPECT_EQ(chosen->actions[0].choice, MOVE_TACKLE);
            }
        }
        if (rightPlan != NULL)
        {
            const struct BattleAiTraceCandidate *chosen = BattleAiTrace_GetCandidate(
                rightPlan->firstCandidateSequence + rightAction->aiCandidateRank);

            EXPECT(rightPlan->flags & BATTLE_AI_TRACE_PLAN_LEGACY_EVALUATOR);
            EXPECT(!(rightPlan->flags & BATTLE_AI_TRACE_PLAN_JOINT));
            EXPECT(chosen != NULL);
            if (chosen != NULL)
            {
                EXPECT_EQ(chosen->actions[0].actorMeta & BATTLE_AI_TRACE_ACTION_ACTOR_MASK,
                          right);
                EXPECT_EQ(chosen->actions[0].choice, MOVE_TACKLE);
            }
        }
        Test_BattleAiJointRuntime_ClearLimits();
        BattleAiJointRuntime_Reset();
    }
}

AI_DOUBLE_BATTLE_TEST("Joint runtime clearing tiny limits before a search restores the default budget")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE
               | AI_FLAG_CHECK_VIABILITY
               | AI_FLAG_TRY_TO_FAINT
               | AI_FLAG_READ_PLAYER_MOVE
               | AI_FLAG_DOUBLE_BATTLE
               | AI_FLAG_OMNISCIENT
               | AI_FLAG_SMART_MON_CHOICES);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Item(ITEM_POWER_HERB); Speed(200); Moves(MOVE_GEOMANCY); }
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Item(ITEM_POWER_HERB); Speed(180); Moves(MOVE_GEOMANCY); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Item(ITEM_POWER_HERB); Speed(60); Moves(MOVE_GEOMANCY); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Item(ITEM_POWER_HERB); Speed(40); Moves(MOVE_GEOMANCY); }
    } WHEN {
        Test_BattleAiJointRuntime_SetLimits(1, 1, 1, 1);
        Test_BattleAiJointRuntime_ClearLimits();
        BattleAiJointRuntime_Reset();
        TURN {
            MOVE(playerLeft, MOVE_GEOMANCY);
            MOVE(playerRight, MOVE_GEOMANCY);
            EXPECT_MOVE(opponentLeft, MOVE_GEOMANCY);
            EXPECT_MOVE(opponentRight, MOVE_GEOMANCY);
        }
    } THEN {
        EXPECT_EQ(Test_BattleAiJointRuntime_GetBuildCount(), 1);
        EXPECT_NE(Test_BattleAiJointRuntime_GetPlanId(), BATTLE_AI_TRACE_ID_NONE);
        Test_BattleAiJointRuntime_ClearLimits();
        BattleAiJointRuntime_Reset();
    }
}

AI_DOUBLE_BATTLE_TEST("Joint runtime does not leave opponent controllers pending with one live player battler")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_MEMENTO) == EFFECT_MEMENTO);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE
               | AI_FLAG_CHECK_VIABILITY
               | AI_FLAG_TRY_TO_FAINT
               | AI_FLAG_READ_PLAYER_MOVE
               | AI_FLAG_DOUBLE_BATTLE
               | AI_FLAG_OMNISCIENT
               | AI_FLAG_SMART_MON_CHOICES);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Speed(1); Moves(MOVE_CELEBRATE); }
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Speed(100); Moves(MOVE_MEMENTO); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Speed(3); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_CELEBRATE);
            MOVE(playerRight, MOVE_MEMENTO, target: opponentLeft);
            EXPECT_MOVE(opponentLeft, MOVE_CELEBRATE);
            EXPECT_MOVE(opponentRight, MOVE_CELEBRATE);
        }
        BattleAiJointRuntime_Reset();
        TURN {
            MOVE(playerLeft, MOVE_CELEBRATE);
            EXPECT_MOVE(opponentLeft, MOVE_CELEBRATE);
            EXPECT_MOVE(opponentRight, MOVE_CELEBRATE);
        }
    } THEN {
        enum BattlerId opponent = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);

        EXPECT_EQ(BattleAiJointRuntime_Prepare(opponent), AI_JOINT_RUNTIME_NOT_ELIGIBLE);
        EXPECT_EQ(Test_BattleAiJointRuntime_GetBuildCount(), 0);
    }
}

AI_DOUBLE_BATTLE_TEST("Joint runtime is ineligible without deadlock while Commander hides a player battler")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE
               | AI_FLAG_CHECK_VIABILITY
               | AI_FLAG_TRY_TO_FAINT
               | AI_FLAG_READ_PLAYER_MOVE
               | AI_FLAG_DOUBLE_BATTLE
               | AI_FLAG_OMNISCIENT
               | AI_FLAG_SMART_MON_CHOICES);
        PLAYER(SPECIES_TATSUGIRI) { Ability(ABILITY_COMMANDER); Moves(MOVE_CELEBRATE); }
        PLAYER(SPECIES_DONDOZO) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN {
            MOVE(playerRight, MOVE_CELEBRATE);
            EXPECT_MOVE(opponentLeft, MOVE_CELEBRATE);
            EXPECT_MOVE(opponentRight, MOVE_CELEBRATE);
        }
    } THEN {
        enum BattlerId commander = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId opponent = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);

        EXPECT(gBattleStruct->battlerState[commander].commandingDondozo);
        EXPECT_EQ(BattleAiJointRuntime_Prepare(opponent), AI_JOINT_RUNTIME_NOT_ELIGIBLE);
        EXPECT_EQ(Test_BattleAiJointRuntime_GetBuildCount(), 0);
    }
}

AI_DOUBLE_BATTLE_TEST("Joint runtime shares one plan when confirmed player commands mix a move and switch")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE
               | AI_FLAG_CHECK_VIABILITY
               | AI_FLAG_TRY_TO_FAINT
               | AI_FLAG_READ_PLAYER_MOVE
               | AI_FLAG_DOUBLE_BATTLE
               | AI_FLAG_OMNISCIENT
               | AI_FLAG_SMART_MON_CHOICES);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Item(ITEM_POWER_HERB); Speed(200); Moves(MOVE_GEOMANCY); }
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Item(ITEM_POWER_HERB); Speed(180); Moves(MOVE_GEOMANCY); }
        PLAYER(SPECIES_WYNAUT) { Ability(ABILITY_GUTS); Item(ITEM_POWER_HERB); Speed(160); Moves(MOVE_GEOMANCY); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Item(ITEM_POWER_HERB); Speed(60); Moves(MOVE_GEOMANCY); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Item(ITEM_POWER_HERB); Speed(40); Moves(MOVE_GEOMANCY); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_GEOMANCY);
            SWITCH(playerRight, 2);
            EXPECT_MOVE(opponentLeft, MOVE_GEOMANCY);
            EXPECT_MOVE(opponentRight, MOVE_GEOMANCY);
        }
    } THEN {
        enum BattlerId playerMove = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId playerSwitch = GetBattlerAtPosition(B_POSITION_PLAYER_RIGHT);
        enum BattlerId opponentLeftBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId opponentRightBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT);
        const struct BattleActionLogEntry *leftAction =
            BattleActionLog_GetLastEntry(opponentLeftBattler, 1u << B_ACTION_USE_MOVE);
        const struct BattleActionLogEntry *rightAction =
            BattleActionLog_GetLastEntry(opponentRightBattler, 1u << B_ACTION_USE_MOVE);
        const struct BattleAiTracePlan *plan;

        EXPECT(leftAction != NULL);
        EXPECT(rightAction != NULL);
        EXPECT_NE(leftAction->aiPlanId, BATTLE_AI_TRACE_ID_NONE);
        EXPECT_EQ(Test_BattleAiJointRuntime_GetBuildCount(), 1);
        EXPECT_EQ(Test_BattleAiJointRuntime_GetPlanId(), leftAction->aiPlanId);
        EXPECT_EQ(leftAction->aiPlanId, rightAction->aiPlanId);
        plan = BattleAiTrace_GetPlan(leftAction->aiPlanId);
        EXPECT(plan != NULL);
        EXPECT_EQ(plan->predictedPlayerActions[0].actorMeta & BATTLE_AI_TRACE_ACTION_ACTOR_MASK,
                  playerMove);
        EXPECT_EQ((plan->predictedPlayerActions[0].actorMeta & BATTLE_AI_TRACE_ACTION_KIND_MASK)
                >> BATTLE_AI_TRACE_ACTION_KIND_SHIFT,
                  BATTLE_AI_TRACE_ACTION_MOVE);
        EXPECT_EQ(plan->predictedPlayerActions[0].choice, MOVE_GEOMANCY);
        EXPECT_EQ(plan->predictedPlayerActions[1].actorMeta & BATTLE_AI_TRACE_ACTION_ACTOR_MASK,
                  playerSwitch);
        EXPECT_EQ((plan->predictedPlayerActions[1].actorMeta & BATTLE_AI_TRACE_ACTION_KIND_MASK)
                >> BATTLE_AI_TRACE_ACTION_KIND_SHIFT,
                  BATTLE_AI_TRACE_ACTION_SWITCH);
        EXPECT_EQ(plan->predictedPlayerActions[1].choice, 2);
    }
}

SINGLE_BATTLE_TEST("NPC trainer AI automatically reads confirmed player commands outside link battles")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_TACKLE); }
    } THEN {
        u32 savedBattleTypeFlags = gBattleTypeFlags;
        u32 savedOpponentA = TRAINER_BATTLE_PARAM.opponentA;
        u64 flags;

        gBattleTypeFlags = BATTLE_TYPE_TRAINER;
        TRAINER_BATTLE_PARAM.opponentA = TRAINER_NONE;
        flags = Test_ApplyNpcTrainerReadPlayerMove(AI_FLAG_BASIC_TRAINER);
        EXPECT(flags & AI_FLAG_BASIC_TRAINER);
        EXPECT(flags & AI_FLAG_READ_PLAYER_MOVE);

        gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_FRONTIER;
        flags = Test_ApplyNpcTrainerReadPlayerMove(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        EXPECT(flags & AI_FLAG_CHECK_BAD_MOVE);
        EXPECT(flags & AI_FLAG_READ_PLAYER_MOVE);

        gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_LINK;
        flags = Test_ApplyNpcTrainerReadPlayerMove(AI_FLAG_BASIC_TRAINER);
        EXPECT(!(flags & AI_FLAG_READ_PLAYER_MOVE));

        gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_RECORDED;
        flags = Test_ApplyNpcTrainerReadPlayerMove(AI_FLAG_BASIC_TRAINER);
        EXPECT(!(flags & AI_FLAG_READ_PLAYER_MOVE));

        gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_RECORDED_LINK;
        flags = Test_ApplyNpcTrainerReadPlayerMove(AI_FLAG_BASIC_TRAINER);
        EXPECT(!(flags & AI_FLAG_READ_PLAYER_MOVE));

        gBattleTypeFlags = 0;
        flags = Test_ApplyNpcTrainerReadPlayerMove(AI_FLAG_BASIC_TRAINER);
        EXPECT(!(flags & AI_FLAG_READ_PLAYER_MOVE));

        gBattleTypeFlags = BATTLE_TYPE_TRAINER;
        flags = Test_ApplyNpcTrainerReadPlayerMove(0);
        EXPECT(!(flags & AI_FLAG_READ_PLAYER_MOVE));

        gBattleTypeFlags = savedBattleTypeFlags;
        TRAINER_BATTLE_PARAM.opponentA = savedOpponentA;
    }
}

SINGLE_BATTLE_TEST("AI runtime knowledge maps ability categories")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_TACKLE); }
    } THEN {
        EXPECT(AI_AbilityHasKnowledgeFlag(ABILITY_SOUNDPROOF, AI_ABILITY_KNOWLEDGE_MOVE_IMMUNITY));
        EXPECT(AI_AbilityHasKnowledgeFlag(ABILITY_SHARPNESS, AI_ABILITY_KNOWLEDGE_MOVE_POWER));
        EXPECT(AI_AbilityHasKnowledgeFlag(ABILITY_MULTISCALE, AI_ABILITY_KNOWLEDGE_DAMAGE_RACE));
        EXPECT(AI_AbilityHasKnowledgeFlag(ABILITY_GUTS, AI_ABILITY_KNOWLEDGE_STATUS));
        EXPECT(AI_AbilityHasKnowledgeFlag(ABILITY_DRIZZLE, AI_ABILITY_KNOWLEDGE_FIELD_CONTROL));
        EXPECT(AI_AbilityHasKnowledgeFlag(ABILITY_SHADOW_TAG, AI_ABILITY_KNOWLEDGE_POSITIONING));
        EXPECT(AI_AbilityHasKnowledgeFlag(ABILITY_NEUTRALIZING_GAS, AI_ABILITY_KNOWLEDGE_ABILITY_CONTROL));
        EXPECT(AI_AbilityHasKnowledgeFlag(ABILITY_INTIMIDATE, AI_ABILITY_KNOWLEDGE_STAT_CONTROL));
        EXPECT(AI_AbilityHasKnowledgeFlag(ABILITY_UNBURDEN, AI_ABILITY_KNOWLEDGE_ITEM_CONTROL));
        EXPECT(AI_AbilityHasKnowledgeFlag(ABILITY_PRANKSTER, AI_ABILITY_KNOWLEDGE_PRIORITY));
        EXPECT(AI_AbilityHasKnowledgeFlag(ABILITY_STANCE_CHANGE, AI_ABILITY_KNOWLEDGE_FORM_STATE));
        EXPECT(!AI_AbilityHasKnowledgeFlag(ABILITY_RUN_AWAY, AI_ABILITY_KNOWLEDGE_MOVE_POWER));
    }
}

SINGLE_BATTLE_TEST("AI runtime knowledge maps item and hold effect categories")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_TACKLE); }
    } THEN {
        EXPECT(AI_ItemHasKnowledgeFlag(ITEM_CHOICE_BAND, AI_HOLD_EFFECT_KNOWLEDGE_DAMAGE_RACE));
        EXPECT(AI_ItemHasKnowledgeFlag(ITEM_CHOICE_BAND, AI_HOLD_EFFECT_KNOWLEDGE_CHOICE_LOCK));
        EXPECT(AI_ItemHasKnowledgeFlag(ITEM_FOCUS_SASH, AI_HOLD_EFFECT_KNOWLEDGE_DEFENSIVE_RACE));
        EXPECT(AI_ItemHasKnowledgeFlag(ITEM_LEFTOVERS, AI_HOLD_EFFECT_KNOWLEDGE_RECOVERY));
        EXPECT(AI_ItemHasKnowledgeFlag(ITEM_LUM_BERRY, AI_HOLD_EFFECT_KNOWLEDGE_STATUS_CURE));
        EXPECT(AI_ItemHasKnowledgeFlag(ITEM_FLAME_ORB, AI_HOLD_EFFECT_KNOWLEDGE_SELF_STATUS));
        EXPECT(AI_ItemHasKnowledgeFlag(ITEM_LIGHT_CLAY, AI_HOLD_EFFECT_KNOWLEDGE_FIELD_DURATION));
        EXPECT(AI_ItemHasKnowledgeFlag(ITEM_ROCKY_HELMET, AI_HOLD_EFFECT_KNOWLEDGE_CONTACT_PUNISH));
        EXPECT(AI_ItemHasKnowledgeFlag(ITEM_SAFETY_GOGGLES, AI_HOLD_EFFECT_KNOWLEDGE_MOVE_SHAPE));
        EXPECT(AI_ItemHasKnowledgeFlag(ITEM_ABILITY_SHIELD, AI_HOLD_EFFECT_KNOWLEDGE_ABILITY_PROTECTION));
        EXPECT(AI_ItemHasKnowledgeFlag(ITEM_CHOICE_SCARF, AI_HOLD_EFFECT_KNOWLEDGE_SPEED_CONTROL));
        EXPECT(AI_ItemHasKnowledgeFlag(ITEM_EJECT_BUTTON, AI_HOLD_EFFECT_KNOWLEDGE_POSITIONING));
        EXPECT(AI_ItemHasKnowledgeFlag(ITEM_VENUSAURITE, AI_HOLD_EFFECT_KNOWLEDGE_GIMMICK));
        EXPECT(AI_HoldEffectHasKnowledgeFlag(HOLD_EFFECT_TERRAIN_SEED, AI_HOLD_EFFECT_KNOWLEDGE_STAT_CONTROL));
        EXPECT(!AI_ItemHasKnowledgeFlag(ITEM_POTION, AI_HOLD_EFFECT_KNOWLEDGE_DAMAGE_RACE));
    }
}

AI_SINGLE_BATTLE_TEST("AI runtime knowledge detects predicted move immunity layers")
{
    enum BattlerId aiBattler = (enum BattlerId)B_POSITION_OPPONENT_LEFT;
    enum BattlerId playerBattler = (enum BattlerId)B_POSITION_PLAYER_LEFT;

    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_ABRA) { Speed(20); Moves(MOVE_SCRATCH); }
        OPPONENT(SPECIES_ZIGZAGOON) { Ability(ABILITY_MAGIC_BOUNCE); Item(ITEM_SAFETY_GOGGLES); Speed(10); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); EXPECT_MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT(AI_CanBattlerIgnorePredictedMove(aiBattler, playerBattler, MOVE_TAUNT));
        EXPECT(AI_CanBattlerIgnorePredictedMove(aiBattler, playerBattler, MOVE_SPORE));
        EXPECT(!AI_CanBattlerIgnorePredictedMove(aiBattler, playerBattler, MOVE_SCRATCH));
    }
}

AI_SINGLE_BATTLE_TEST("AI prefers Bubble over Water Gun if it's slower")
{
    GIVEN {
        ASSUME(GetMovePower(MOVE_WATER_GUN) == GetMovePower(MOVE_BUBBLE));
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_SCIZOR) { Speed(200); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_WATER_GUN, MOVE_BUBBLE); Speed(10); }
    } WHEN {
        TURN { SCORE_GT(opponent, MOVE_BUBBLE, MOVE_WATER_GUN); }
        TURN { SCORE_GT(opponent, MOVE_BUBBLE, MOVE_WATER_GUN); }
    }
}

AI_SINGLE_BATTLE_TEST("AI prefers Water Gun over Bubble if it knows that foe has Contrary")
{
    enum Ability abilityAI;

    PARAMETRIZE { abilityAI = ABILITY_MOXIE; }
    PARAMETRIZE { abilityAI = ABILITY_MOLD_BREAKER; } // Mold Breaker ignores Contrary.
    GIVEN {
        ASSUME(GetMovePower(MOVE_BUBBLE) == GetMovePower(MOVE_WATER_GUN));
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_SHUCKLE) { Ability(ABILITY_CONTRARY); }
        OPPONENT(SPECIES_PINSIR) { Moves(MOVE_WATER_GUN, MOVE_BUBBLE); Ability(abilityAI); }
    } WHEN {
            TURN { MOVE(player, MOVE_DEFENSE_CURL); }
            TURN { MOVE(player, MOVE_DEFENSE_CURL);
                   if (abilityAI == ABILITY_MOLD_BREAKER) { SCORE_GT(opponent, MOVE_BUBBLE, MOVE_WATER_GUN); } // Bubble is a plus effect if contrary is ignored
                   else { SCORE_GT(opponent, MOVE_WATER_GUN, MOVE_BUBBLE); }}
    } SCENE {
        MESSAGE("Shuckle's Defense fell!"); // Contrary activates
    } THEN {
        EXPECT(gAiLogicData->abilities[B_POSITION_PLAYER_LEFT] == ABILITY_CONTRARY);
    }
}

AI_SINGLE_BATTLE_TEST("AI prefers moves with better accuracy, but only if they both require the same number of hits to ko")
{
    enum Move move1 = MOVE_NONE, move2 = MOVE_NONE, move3 = MOVE_NONE, move4 = MOVE_NONE;
    enum Move expectedMove, expectedMove2;
    u16 hp, turns;
    enum Ability abilityAtk;

    abilityAtk = ABILITY_NONE;
    expectedMove2 = MOVE_NONE;

    // Here it's a simple test, both Slam and Strength deal the same damage, but Strength always hits, whereas Slam often misses.
    PARAMETRIZE { move1 = MOVE_SLAM; move2 = MOVE_STRENGTH; move3 = MOVE_SCRATCH; hp = 490; expectedMove = MOVE_STRENGTH; turns = 4; }
    PARAMETRIZE { move1 = MOVE_SLAM; move2 = MOVE_STRENGTH; move3 = MOVE_SWIFT; move4 = MOVE_SCRATCH; hp = 365; expectedMove = MOVE_STRENGTH; turns = 3; }
    PARAMETRIZE { move1 = MOVE_SLAM; move2 = MOVE_STRENGTH; move3 = MOVE_SWIFT; move4 = MOVE_SCRATCH; hp = 245; expectedMove = MOVE_STRENGTH; turns = 2; }
    PARAMETRIZE { move1 = MOVE_SLAM; move2 = MOVE_STRENGTH; move3 = MOVE_SWIFT; move4 = MOVE_SCRATCH; hp = 125; expectedMove = MOVE_STRENGTH; turns = 1; }
    // Mega Kick deals more damage, but can miss more often. Here, AI should choose Mega Kick if it can faint target in less number of turns than Strength. Otherwise, it should use Strength.
    PARAMETRIZE { move1 = MOVE_MEGA_KICK; move2 = MOVE_STRENGTH; move3 = MOVE_SWIFT; move4 = MOVE_SCRATCH; hp = 170; expectedMove = MOVE_MEGA_KICK; turns = 1; }
    PARAMETRIZE { move1 = MOVE_MEGA_KICK; move2 = MOVE_STRENGTH; move3 = MOVE_SWIFT; move4 = MOVE_SCRATCH; hp = 245; expectedMove = MOVE_STRENGTH; turns = 2; }
    // Swift always hits and Guts has accuracy of 100%. Hustle lowers accuracy of all physical moves.
    PARAMETRIZE { abilityAtk = ABILITY_HUSTLE; move1 = MOVE_MEGA_KICK; move2 = MOVE_STRENGTH; move3 = MOVE_SWIFT; move4 = MOVE_SCRATCH; hp = 5; expectedMove = MOVE_SWIFT; turns = 1; }
    PARAMETRIZE { abilityAtk = ABILITY_HUSTLE; move1 = MOVE_MEGA_KICK; move2 = MOVE_STRENGTH; move3 = MOVE_GUST; move4 = MOVE_SCRATCH; hp = 5; expectedMove = MOVE_GUST; turns = 1; }
    // Mega Kick and Slam both have lower accuracy. Gust and Scratch both have 100, so AI can choose either of them.
    PARAMETRIZE { move1 = MOVE_MEGA_KICK; move2 = MOVE_SLAM; move3 = MOVE_SCRATCH; move4 = MOVE_GUST; hp = 5; expectedMove = MOVE_GUST; expectedMove2 = MOVE_SCRATCH; turns = 1; }
    // All moves hit with No guard ability
    PARAMETRIZE { move1 = MOVE_MEGA_KICK; move2 = MOVE_GUST; hp = 5; expectedMove = MOVE_MEGA_KICK; expectedMove2 = MOVE_GUST; turns = 1; }
    // Tests to compare move that always hits and a beneficial effect. A move with higher acc should be chosen in this case.
    PARAMETRIZE { move1 = MOVE_SHOCK_WAVE; move2 = MOVE_ICY_WIND; hp = 5; expectedMove = MOVE_SHOCK_WAVE; turns = 1; }
    PARAMETRIZE { move1 = MOVE_SHOCK_WAVE; move2 = MOVE_ICY_WIND; move3 = MOVE_THUNDERBOLT; hp = 5; expectedMove = MOVE_SHOCK_WAVE; expectedMove2 = MOVE_THUNDERBOLT; turns = 1; }

    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_WOBBUFFET) { HP(hp); }
        PLAYER(SPECIES_WOBBUFFET);
        ASSUME(GetMoveAccuracy(MOVE_SWIFT) == 0);
        ASSUME(GetMovePower(MOVE_SLAM) == GetMovePower(MOVE_STRENGTH));
        ASSUME(GetMovePower(MOVE_MEGA_KICK) > GetMovePower(MOVE_STRENGTH));
        ASSUME(GetMoveAccuracy(MOVE_SLAM) < GetMoveAccuracy(MOVE_STRENGTH));
        ASSUME(GetMoveAccuracy(MOVE_MEGA_KICK) < GetMoveAccuracy(MOVE_STRENGTH));
        ASSUME(GetMoveAccuracy(MOVE_SCRATCH) == 100);
        ASSUME(GetMoveAccuracy(MOVE_GUST) == 100);
        ASSUME(GetMoveAccuracy(MOVE_SHOCK_WAVE) == 0);
        ASSUME(GetMoveAccuracy(MOVE_THUNDERBOLT) == 100);
        ASSUME(GetMoveAccuracy(MOVE_ICY_WIND) != 100);
        ASSUME(GetMoveCategory(MOVE_SLAM) == DAMAGE_CATEGORY_PHYSICAL);
        ASSUME(GetMoveCategory(MOVE_STRENGTH) == DAMAGE_CATEGORY_PHYSICAL);
        ASSUME(GetMoveCategory(MOVE_SCRATCH) == DAMAGE_CATEGORY_PHYSICAL);
        ASSUME(GetMoveCategory(MOVE_MEGA_KICK) == DAMAGE_CATEGORY_PHYSICAL);
        ASSUME(GetMoveCategory(MOVE_SWIFT) == DAMAGE_CATEGORY_SPECIAL);
        ASSUME(GetMoveCategory(MOVE_SHOCK_WAVE) == DAMAGE_CATEGORY_SPECIAL);
        ASSUME(GetMoveCategory(MOVE_ICY_WIND) == DAMAGE_CATEGORY_SPECIAL);
        ASSUME(GetMoveCategory(MOVE_THUNDERBOLT) == DAMAGE_CATEGORY_SPECIAL);
        ASSUME(GetMoveCategory(MOVE_GUST) == DAMAGE_CATEGORY_SPECIAL);
        OPPONENT(SPECIES_EXPLOUD) { Moves(move1, move2, move3, move4); Ability(abilityAtk); SpAttack(50); } // Low Sp.Atk, so Swift deals less damage than Strength.
    } WHEN {
            switch (turns)
            {
            case 1:
                if (expectedMove2 != MOVE_NONE) {
                    TURN { EXPECT_MOVES(opponent, expectedMove, expectedMove2); SEND_OUT(player, 1); }
                }
                else {
                    TURN { EXPECT_MOVE(opponent, expectedMove); SEND_OUT(player, 1); }
                }
                break;
            case 2:
                TURN { EXPECT_MOVE(opponent, expectedMove); }
                TURN { EXPECT_MOVE(opponent, expectedMove); SEND_OUT(player, 1); }
                break;
            case 3:
                TURN { EXPECT_MOVE(opponent, expectedMove); }
                TURN { EXPECT_MOVE(opponent, expectedMove); }
                TURN { EXPECT_MOVE(opponent, expectedMove); SEND_OUT(player, 1); }
                break;
            case 4:
                TURN { EXPECT_MOVE(opponent, expectedMove); }
                TURN { EXPECT_MOVE(opponent, expectedMove); }
                TURN { EXPECT_MOVE(opponent, expectedMove); }
                TURN { EXPECT_MOVE(opponent, expectedMove); SEND_OUT(player, 1); }
                break;
            }
    } SCENE {
        MESSAGE("Wobbuffet fainted!");
    }
}

AI_SINGLE_BATTLE_TEST("AI prefers moves which deal more damage instead of moves which are super-effective but deal less damage")
{
    u8 turns = 0;
    enum Move move1 = MOVE_NONE, move2 = MOVE_NONE, move3 = MOVE_NONE, move4 = MOVE_NONE;
    enum Move expectedMove;
    enum Ability abilityAtk, abilityDef;

    abilityAtk = ABILITY_NONE;

    // Scald and Poison Jab take 3 hits, Waterfall takes 2.
    PARAMETRIZE { move1 = MOVE_WATERFALL; move2 = MOVE_SCALD; move3 = MOVE_POISON_JAB; move4 = MOVE_WATER_GUN; expectedMove = MOVE_WATERFALL; turns = 2; }
    // Poison Jab takes 3 hits, Water gun 5. Immunity so there's no poison chip damage.
    PARAMETRIZE { move1 = MOVE_POISON_JAB; move2 = MOVE_WATER_GUN; expectedMove = MOVE_POISON_JAB; abilityDef = ABILITY_IMMUNITY; turns = 3; }

    GIVEN {
        ASSUME(GetMoveCategory(MOVE_WATERFALL) == DAMAGE_CATEGORY_PHYSICAL);
        ASSUME(GetMoveCategory(MOVE_SCALD) == DAMAGE_CATEGORY_SPECIAL);
        ASSUME(GetMoveCategory(MOVE_POISON_JAB) == DAMAGE_CATEGORY_PHYSICAL);
        ASSUME(GetMoveCategory(MOVE_WATER_GUN) == DAMAGE_CATEGORY_SPECIAL);
        ASSUME(GetSpeciesBaseAttack(SPECIES_NIDOQUEEN) == 92); // Gen 5's 82 Base Attack causes the test to fail
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_TYPHLOSION) { Ability(abilityDef); }
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_NIDOQUEEN) { Moves(move1, move2, move3, move4); Ability(abilityAtk); }
    } WHEN {
            switch (turns)
            {
            case 2:
                TURN { EXPECT_MOVE(opponent, expectedMove); }
                TURN { EXPECT_MOVE(opponent, expectedMove); SEND_OUT(player, 1); }
                break;
            case 3:
                TURN { EXPECT_MOVE(opponent, expectedMove); }
                TURN { EXPECT_MOVE(opponent, expectedMove); }
                TURN { EXPECT_MOVE(opponent, expectedMove); SEND_OUT(player, 1); }
                break;
            }
    } SCENE {
        MESSAGE("Typhlosion fainted!");
    }
}

AI_SINGLE_BATTLE_TEST("AI prefers Earthquake over Drill Run if both require the same number of hits to ko")
{
    // Drill Run has less accuracy than E-quake, but can score a higher crit. However the chance is too small, so AI should ignore it.
    GIVEN {
        ASSUME(GetMoveCategory(MOVE_EARTHQUAKE) == DAMAGE_CATEGORY_PHYSICAL); // Added because Geodude has to KO Typhlosion
        ASSUME(GetMoveCategory(MOVE_DRILL_RUN) == DAMAGE_CATEGORY_PHYSICAL);  // Added because Geodude has to KO Typhlosion
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_TYPHLOSION);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_GEODUDE) { Moves(MOVE_EARTHQUAKE, MOVE_DRILL_RUN); }
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_EARTHQUAKE); }
        TURN { EXPECT_MOVE(opponent, MOVE_EARTHQUAKE); SEND_OUT(player, 1); }
    }
    SCENE {
        MESSAGE("Typhlosion fainted!");
    }
}

AI_SINGLE_BATTLE_TEST("AI prefers a weaker move over one with a downside effect if both require the same number of hits to ko")
{
    enum Move move1 = MOVE_NONE, move2 = MOVE_NONE, move3 = MOVE_NONE, move4 = MOVE_NONE;
    enum Move expectedMove;
    u16 hp, turns;

    // Both moves require the same number of turns but Flamethrower will be chosen over Overheat (powerful effect)
    PARAMETRIZE { move1 = MOVE_OVERHEAT; move2 = MOVE_FLAMETHROWER; hp = 320; expectedMove = MOVE_FLAMETHROWER; turns = 2; }
    // Overheat kill in least amount of turns
    PARAMETRIZE { move1 = MOVE_OVERHEAT; move2 = MOVE_FLAMETHROWER; hp = 250; expectedMove = MOVE_OVERHEAT; turns = 1; }

    GIVEN {
        ASSUME(GetMoveCategory(MOVE_FLAMETHROWER) == DAMAGE_CATEGORY_SPECIAL); // Added because Typhlosion has to KO Wobbuffet
        ASSUME(GetMoveCategory(MOVE_OVERHEAT) == DAMAGE_CATEGORY_SPECIAL);     // Added because Typhlosion has to KO Wobbuffet
        // With Gen 5 data, it chooses Overheat instead
        ASSUME(GetMovePower(MOVE_FLAMETHROWER) == 90); // In Gen 5, it's 95
        ASSUME(GetMovePower(MOVE_OVERHEAT) == 130); // In Gen 5, it's 140.
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_WOBBUFFET) { HP(hp); }
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_TYPHLOSION) { Moves(move1, move2, move3, move4); }
    } WHEN {
        switch (turns)
        {
        case 1:
            TURN { EXPECT_MOVE(opponent, expectedMove); SEND_OUT(player, 1); }
            break;
        case 2:
            TURN { EXPECT_MOVE(opponent, expectedMove); }
            TURN { EXPECT_MOVE(opponent, expectedMove); SEND_OUT(player, 1); }
            break;
        }
    } SCENE {
        MESSAGE("Wobbuffet fainted!");
    }
}

AI_SINGLE_BATTLE_TEST("AI prefers moves with the best possible score, chosen randomly if tied")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_WOBBUFFET) { HP(5); }
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_THUNDERBOLT, MOVE_SLUDGE_BOMB, MOVE_TAKE_DOWN); }
    } WHEN {
        TURN { EXPECT_MOVES(opponent, MOVE_THUNDERBOLT, MOVE_SLUDGE_BOMB); SEND_OUT(player, 1); }
    }
    SCENE {
        MESSAGE("Wobbuffet fainted!");
    }
}

AI_SINGLE_BATTLE_TEST("AI can choose a status move that boosts the attack by two")
{
    GIVEN {
        ASSUME(GetMoveCategory(MOVE_STRENGTH) == DAMAGE_CATEGORY_PHYSICAL);
        ASSUME(GetMoveCategory(MOVE_HORN_ATTACK) == DAMAGE_CATEGORY_PHYSICAL);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_WOBBUFFET) { HP(277); }
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_KANGASKHAN) { Moves(MOVE_STRENGTH, MOVE_HORN_ATTACK, MOVE_SWORDS_DANCE); }
    } WHEN {
        TURN { EXPECT_MOVES(opponent, MOVE_STRENGTH, MOVE_SWORDS_DANCE); }
        TURN { EXPECT_MOVE(opponent, MOVE_STRENGTH); SEND_OUT(player, 1); }
    }
}

AI_SINGLE_BATTLE_TEST("AI chooses the safest option to faint the target, taking into account accuracy and move effect")
{
    enum Move move1 = MOVE_NONE, move2 = MOVE_NONE, move3 = MOVE_NONE, move4 = MOVE_NONE;
    enum Move expectedMove, expectedMove2 = MOVE_NONE;
    enum Ability abilityAtk = ABILITY_NONE;
    enum Item holdItemAtk = ITEM_NONE;

    // Psychic is not very effective, but always hits. Solarbeam requires a charging turn, Double Edge has recoil and Focus Blast can miss;
    PARAMETRIZE { abilityAtk = ABILITY_STURDY; move1 = MOVE_FOCUS_BLAST; move2 = MOVE_SOLAR_BEAM; move3 = MOVE_PSYCHIC; move4 = MOVE_DOUBLE_EDGE; expectedMove = MOVE_PSYCHIC; }
    // Same as above, but ai mon has rock head ability, so it can use Double Edge without taking recoil damage. Psychic can also lower Special Defense,
    // but because it faints the target it doesn't matter.
    PARAMETRIZE { abilityAtk = ABILITY_ROCK_HEAD; move1 = MOVE_FOCUS_BLAST; move2 = MOVE_SOLAR_BEAM; move3 = MOVE_PSYCHIC; move4 = MOVE_DOUBLE_EDGE;
                  expectedMove = MOVE_PSYCHIC; expectedMove2 = MOVE_DOUBLE_EDGE; }
    // This time it's Solarbeam + Psychic, because the weather is sunny.
    PARAMETRIZE { abilityAtk = ABILITY_DROUGHT; move1 = MOVE_FOCUS_BLAST; move2 = MOVE_SOLAR_BEAM; move3 = MOVE_PSYCHIC; move4 = MOVE_DOUBLE_EDGE;
                  expectedMove = MOVE_PSYCHIC; expectedMove2 = MOVE_SOLAR_BEAM; }
    // Psychic and Solar Beam are chosen because user is holding Power Herb
    PARAMETRIZE { abilityAtk = ABILITY_STURDY; holdItemAtk = ITEM_POWER_HERB; move1 = MOVE_FOCUS_BLAST; move2 = MOVE_SOLAR_BEAM; move3 = MOVE_PSYCHIC; move4 = MOVE_DOUBLE_EDGE;
                  expectedMove = MOVE_PSYCHIC; expectedMove2 = MOVE_SOLAR_BEAM; }
    // Skull Bash is chosen because it's the most accurate and is holding Power Herb
    PARAMETRIZE { abilityAtk = ABILITY_STURDY; holdItemAtk = ITEM_POWER_HERB; move1 = MOVE_FOCUS_BLAST; move2 = MOVE_SKULL_BASH; move3 = MOVE_SLAM; move4 = MOVE_DOUBLE_EDGE;
                  expectedMove = MOVE_SKULL_BASH; }

    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_WOBBUFFET) { HP(5); }
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_GEODUDE) { Moves(move1, move2, move3, move4); Ability(abilityAtk); Item(holdItemAtk); }
    } WHEN {
        TURN {  if (expectedMove2 == MOVE_NONE) { EXPECT_MOVE(opponent, expectedMove); SEND_OUT(player, 1); }
                else { EXPECT_MOVES(opponent, expectedMove, expectedMove2); SCORE_EQ(opponent, expectedMove, expectedMove2); SEND_OUT(player, 1); }
             }
    }
    SCENE {
        MESSAGE("Wobbuffet fainted!");
    }
}

AI_SINGLE_BATTLE_TEST("AI scores KOs with two turn moves correctly, considering Power Herb")
{
    enum Item aiItem;

    PARAMETRIZE { aiItem = ITEM_POWER_HERB; }
    PARAMETRIZE { aiItem= ITEM_NONE; }

    GIVEN {
        ASSUME(GetItemHoldEffect(ITEM_POWER_HERB) == HOLD_EFFECT_POWER_HERB);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_CELEBRATE); HP(5); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_FOCUS_BLAST, MOVE_SKULL_BASH, MOVE_FIERY_DANCE, MOVE_CRABHAMMER); Item(aiItem); }
    } WHEN {
        TURN { aiItem == ITEM_POWER_HERB ? EXPECT_MOVE(opponent, MOVE_SKULL_BASH) : SCORE_EQ(opponent, MOVE_FIERY_DANCE, MOVE_SKULL_BASH); }
    }
}

AI_SINGLE_BATTLE_TEST("AI won't use Solar Beam if there is no Sun up or the user is not holding Power Herb")
{
    enum Ability abilityAtk = ABILITY_NONE;
    enum Item holdItemAtk = ITEM_NONE;

    PARAMETRIZE { abilityAtk = ABILITY_DROUGHT; }
    PARAMETRIZE { holdItemAtk = ITEM_POWER_HERB; }

    GIVEN {
        ASSUME(GetMoveCategory(MOVE_SOLAR_BEAM) == DAMAGE_CATEGORY_SPECIAL);
        ASSUME(GetMoveCategory(MOVE_GRASS_PLEDGE) == DAMAGE_CATEGORY_SPECIAL);
        ASSUME(GetMovePower(MOVE_GRASS_PLEDGE) == 80); // Gen 5's 50 power causes the test to fail
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_WOBBUFFET) { HP(211); }
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_TYPHLOSION) { Moves(MOVE_SOLAR_BEAM, MOVE_GRASS_PLEDGE); Ability(abilityAtk); Item(holdItemAtk); }
    } WHEN {
        if (abilityAtk == ABILITY_DROUGHT) {
            TURN { EXPECT_MOVES(opponent, MOVE_SOLAR_BEAM, MOVE_GRASS_PLEDGE); }
            TURN { EXPECT_MOVES(opponent, MOVE_SOLAR_BEAM, MOVE_GRASS_PLEDGE); SEND_OUT(player, 1); }
        } else if (holdItemAtk == ITEM_POWER_HERB) {
            TURN { EXPECT_MOVES(opponent, MOVE_SOLAR_BEAM, MOVE_GRASS_PLEDGE); MOVE(player, MOVE_KNOCK_OFF); }
            TURN { EXPECT_MOVE(opponent, MOVE_GRASS_PLEDGE); SEND_OUT(player, 1); }
        } else {
            TURN { EXPECT_MOVE(opponent, MOVE_GRASS_PLEDGE); }
            TURN { EXPECT_MOVE(opponent, MOVE_GRASS_PLEDGE); SEND_OUT(player, 1); }
        }
    } SCENE {
        MESSAGE("Wobbuffet fainted!");
    }
}

AI_SINGLE_BATTLE_TEST("AI accepts delayed Solar Beam when near-term loss has no clean hit")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_SOLAR_BEAM) == EFFECT_SOLAR_BEAM);
        ASSUME(GetMoveCategory(MOVE_SOLAR_BEAM) == DAMAGE_CATEGORY_SPECIAL);
        ASSUME(GetMoveCategory(MOVE_GRASS_PLEDGE) == DAMAGE_CATEGORY_SPECIAL);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_BLASTOISE) {
            Level(50); MaxHP(150); HP(150); SpAttack(80); SpDefense(100); Speed(80);
            Moves(MOVE_WATER_PULSE);
        }
        OPPONENT(SPECIES_TYPHLOSION) {
            Level(50); MaxHP(120); HP(120); SpAttack(200); SpDefense(100); Speed(100);
            Moves(MOVE_SOLAR_BEAM, MOVE_GRASS_PLEDGE);
        }
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_SOLAR_BEAM); }
    }
}

AI_SINGLE_BATTLE_TEST("AI won't use ground type attacks against flying type Pokemon unless Gravity is in effect")
{
    GIVEN {
        ASSUME(GetMoveCategory(MOVE_EARTHQUAKE) == DAMAGE_CATEGORY_PHYSICAL); // Otherwise, it doesn't KO Crobat
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_CROBAT);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_NIDOQUEEN) { Moves(MOVE_EARTHQUAKE, MOVE_SCRATCH, MOVE_POISON_STING, MOVE_GUST); }
    } WHEN {
        TURN { NOT_EXPECT_MOVE(opponent, MOVE_EARTHQUAKE); }
        TURN { MOVE(player, MOVE_GRAVITY); NOT_EXPECT_MOVE(opponent, MOVE_EARTHQUAKE); }
        TURN { EXPECT_MOVE(opponent, MOVE_EARTHQUAKE); SEND_OUT(player, 1); }
    } SCENE {
        MESSAGE("Gravity intensified!");
    }
}


AI_SINGLE_BATTLE_TEST("AI without any flags chooses moves at random - singles")
{
    GIVEN {
        AI_FLAGS(0);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_NIDOQUEEN) { Moves(MOVE_SPLASH, MOVE_EXPLOSION, MOVE_RAGE, MOVE_HELPING_HAND); }
    } WHEN {
            TURN { EXPECT_MOVES(opponent, MOVE_SPLASH, MOVE_EXPLOSION, MOVE_RAGE, MOVE_HELPING_HAND);
                   SCORE_EQ_VAL(opponent, MOVE_SPLASH, AI_SCORE_DEFAULT);
                   SCORE_EQ_VAL(opponent, MOVE_EXPLOSION, AI_SCORE_DEFAULT);
                   SCORE_EQ_VAL(opponent, MOVE_RAGE, AI_SCORE_DEFAULT);
                   SCORE_EQ_VAL(opponent, MOVE_HELPING_HAND, AI_SCORE_DEFAULT);
                }
    }
}

AI_DOUBLE_BATTLE_TEST("AI without any flags chooses moves at random - doubles")
{
    GIVEN {
        AI_FLAGS(0);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_NIDOQUEEN) { Moves(MOVE_SPLASH, MOVE_EXPLOSION, MOVE_RAGE, MOVE_HELPING_HAND); }
        OPPONENT(SPECIES_NIDOQUEEN) { Moves(MOVE_SPLASH, MOVE_EXPLOSION, MOVE_RAGE, MOVE_HELPING_HAND); }
    } WHEN {
            TURN { EXPECT_MOVES(opponentLeft, MOVE_SPLASH, MOVE_EXPLOSION, MOVE_RAGE, MOVE_HELPING_HAND);
                   EXPECT_MOVES(opponentRight, MOVE_SPLASH, MOVE_EXPLOSION, MOVE_RAGE, MOVE_HELPING_HAND);
                   SCORE_EQ_VAL(opponentLeft, MOVE_SPLASH, AI_SCORE_DEFAULT, target:playerLeft);
                   SCORE_EQ_VAL(opponentLeft, MOVE_EXPLOSION, AI_SCORE_DEFAULT, target:playerLeft);
                   SCORE_EQ_VAL(opponentLeft, MOVE_RAGE, AI_SCORE_DEFAULT, target:playerLeft);
                   SCORE_EQ_VAL(opponentLeft, MOVE_HELPING_HAND, AI_SCORE_DEFAULT, target:playerLeft);
                   SCORE_EQ_VAL(opponentRight, MOVE_SPLASH, AI_SCORE_DEFAULT, target:playerLeft);
                   SCORE_EQ_VAL(opponentRight, MOVE_EXPLOSION, AI_SCORE_DEFAULT, target:playerLeft);
                   SCORE_EQ_VAL(opponentRight, MOVE_RAGE, AI_SCORE_DEFAULT, target:playerLeft);
                   SCORE_EQ_VAL(opponentRight, MOVE_HELPING_HAND, AI_SCORE_DEFAULT, target:playerLeft);
                }
    }
}

AI_SINGLE_BATTLE_TEST("AI will choose either Rock Tomb or Bulldoze if Stat drop effect will activate and they kill with the same number of hits")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_WOBBUFFET) { HP(46); Speed(20); }
        PLAYER(SPECIES_WYNAUT) { Speed(20); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(10); Moves(MOVE_BULLDOZE, MOVE_ROCK_TOMB); }
    } WHEN {
            TURN { EXPECT_MOVES(opponent, MOVE_BULLDOZE, MOVE_ROCK_TOMB); }
            TURN { EXPECT_MOVES(opponent, MOVE_BULLDOZE, MOVE_ROCK_TOMB); SEND_OUT(player, 1); }
    } SCENE {
        MESSAGE("Wobbuffet fainted!");
    }
}

AI_SINGLE_BATTLE_TEST("First Impression is preferred on the first turn of the species if it's the best dmg move")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FIRST_IMPRESSION) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(GetMovePower(MOVE_FIRST_IMPRESSION) == 90);
        ASSUME(GetMovePower(MOVE_LUNGE) == 80);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_KANGASKHAN);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_FIRST_IMPRESSION, MOVE_LUNGE); }
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_FIRST_IMPRESSION); }
        TURN { EXPECT_MOVE(opponent, MOVE_LUNGE); }
    }
}

AI_SINGLE_BATTLE_TEST("First Impression is not chosen if it's blocked by certain abilities")
{
    u16 species;
    enum Ability ability;

    PARAMETRIZE { species = SPECIES_BRUXISH; ability = ABILITY_DAZZLING; }
    PARAMETRIZE { species = SPECIES_FARIGIRAF; ability = ABILITY_ARMOR_TAIL; }
    PARAMETRIZE { species = SPECIES_TSAREENA; ability = ABILITY_QUEENLY_MAJESTY; }

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FIRST_IMPRESSION) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(GetMovePower(MOVE_FIRST_IMPRESSION) == 90);
        ASSUME(GetMovePower(MOVE_LUNGE) == 80);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(species) { Ability(ability); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_FIRST_IMPRESSION, MOVE_LUNGE); }
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_LUNGE); }
    }
}

AI_SINGLE_BATTLE_TEST("AI will not choose Burn Up if the user lost the Fire typing")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_BURN_UP) == EFFECT_FAIL_IF_NOT_ARG_TYPE);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CYNDAQUIL) { Moves(MOVE_BURN_UP, MOVE_EXTRASENSORY, MOVE_FLAMETHROWER); }
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_BURN_UP); }
        TURN { EXPECT_MOVE(opponent, MOVE_FLAMETHROWER); }
    }
}

AI_SINGLE_BATTLE_TEST("AI will only choose Surf 1/3 times if the opposing mon has Volt Absorb")
{
    PASSES_RANDOMLY(1, 3, RNG_AI_ABILITY);
    GIVEN {
        ASSUME(GetMoveType(MOVE_THUNDERBOLT) == TYPE_ELECTRIC);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_LANTURN) { Ability(ABILITY_VOLT_ABSORB); }
        OPPONENT(SPECIES_LANTURN) { Moves(MOVE_THUNDERBOLT, MOVE_ICE_BEAM, MOVE_SURF); }
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_SURF); }
        TURN { EXPECT_MOVE(opponent, MOVE_SURF); }
    } SCENE {
        MESSAGE("The opposing Lanturn used Surf!");
        MESSAGE("The opposing Lanturn used Surf!");
    }
}

AI_SINGLE_BATTLE_TEST("AI will choose Thunderbolt then Surf 2/3 times if the opposing mon has Volt Absorb")
{
    PASSES_RANDOMLY(2, 3, RNG_AI_ABILITY);
    GIVEN {
        ASSUME(GetMoveType(MOVE_THUNDERBOLT) == TYPE_ELECTRIC);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_LANTURN) { Ability(ABILITY_VOLT_ABSORB); }
        OPPONENT(SPECIES_LANTURN) { Moves(MOVE_THUNDERBOLT, MOVE_ICE_BEAM, MOVE_SURF); }
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_THUNDERBOLT); }
        TURN { EXPECT_MOVE(opponent, MOVE_SURF); }
    } SCENE {
        MESSAGE("The opposing Lanturn used Thunderbolt!");
        MESSAGE("The opposing Lanturn used Surf!");
    }
}

AI_SINGLE_BATTLE_TEST("AI will choose Scratch over Power-up Punch with Contrary")
{
    enum Ability ability;

    PARAMETRIZE { ability = ABILITY_SUCTION_CUPS; }
    PARAMETRIZE { ability = ABILITY_CONTRARY; }
    GIVEN {
        ASSUME(GetMovePower(MOVE_SCRATCH) == 40);
        ASSUME(GetMoveType(MOVE_SCRATCH) == TYPE_NORMAL);
        ASSUME(GetMovePower(MOVE_POWER_UP_PUNCH) == 40);
        ASSUME(GetMoveType(MOVE_POWER_UP_PUNCH) == TYPE_FIGHTING);
        ASSUME(GetSpeciesType(SPECIES_SQUIRTLE, 0) == TYPE_WATER);
        ASSUME(GetSpeciesType(SPECIES_SQUIRTLE, 1) == TYPE_WATER);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_SQUIRTLE) { };
        OPPONENT(SPECIES_MALAMAR) { Ability(ability); Moves(MOVE_SCRATCH, MOVE_POWER_UP_PUNCH); }
    } WHEN {
        TURN {
            if (ability != ABILITY_CONTRARY)
                EXPECT_MOVE(opponent, MOVE_POWER_UP_PUNCH);
            else
                EXPECT_MOVE(opponent, MOVE_SCRATCH);
        }
    }
}

AI_SINGLE_BATTLE_TEST("AI will choose Superpower over Outrage with Contrary")
{
    enum Ability ability;

    PARAMETRIZE { ability = ABILITY_SUCTION_CUPS; }
    PARAMETRIZE { ability = ABILITY_CONTRARY; }
    GIVEN {
        ASSUME(GetMovePower(MOVE_SUPERPOWER) == 120);
        ASSUME(GetMoveType(MOVE_SUPERPOWER) == TYPE_FIGHTING);
        ASSUME(GetMovePower(MOVE_OUTRAGE) == 120);
        ASSUME(GetMoveType(MOVE_OUTRAGE) == TYPE_DRAGON);
        ASSUME(GetSpeciesType(SPECIES_SQUIRTLE, 0) == TYPE_WATER);
        ASSUME(GetSpeciesType(SPECIES_SQUIRTLE, 1) == TYPE_WATER);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_SQUIRTLE) { };
        OPPONENT(SPECIES_MALAMAR) { Ability(ability); Moves(MOVE_OUTRAGE, MOVE_SUPERPOWER); }
    } WHEN {
        TURN {
            if (ability != ABILITY_CONTRARY)
                EXPECT_MOVE(opponent, MOVE_OUTRAGE);
            else
                EXPECT_MOVE(opponent, MOVE_SUPERPOWER);
        }
    }
}

AI_SINGLE_BATTLE_TEST("AI calculates guaranteed criticals and detects critical immunity")
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_SWIFT_SWIM; }
    PARAMETRIZE { ability = ABILITY_SHELL_ARMOR; }

    GIVEN {
        ASSUME(MoveAlwaysCrits(MOVE_STORM_THROW));
        ASSUME(GetMovePower(MOVE_STORM_THROW) == 60);
        ASSUME(GetMovePower(MOVE_BRICK_BREAK) == 75);
        ASSUME(GetMoveType(MOVE_STORM_THROW) == GetMoveType(MOVE_BRICK_BREAK));
        ASSUME(GetMoveCategory(MOVE_STORM_THROW) == GetMoveCategory(MOVE_BRICK_BREAK));
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_OMASTAR) { Ability(ability); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_STORM_THROW, MOVE_BRICK_BREAK); }
    } WHEN {
        if (ability == ABILITY_SHELL_ARMOR)
            TURN { EXPECT_MOVE(opponent, MOVE_BRICK_BREAK); }
        else
            TURN { EXPECT_MOVE(opponent, MOVE_STORM_THROW); }
    }
}

AI_SINGLE_BATTLE_TEST("AI avoids contact moves against rocky helmet")
{
    enum Item item;

    PARAMETRIZE { item = ITEM_NONE; }
    PARAMETRIZE { item = ITEM_ROCKY_HELMET; }

    GIVEN {
        ASSUME(MoveMakesContact(MOVE_BRANCH_POKE));
        ASSUME(!MoveMakesContact(MOVE_LEAFAGE));
        ASSUME(GetMovePower(MOVE_BRANCH_POKE) == GetMovePower(MOVE_LEAFAGE));
        ASSUME(GetMoveType(MOVE_BRANCH_POKE) == GetMoveType(MOVE_LEAFAGE));
        ASSUME(GetMoveCategory(MOVE_BRANCH_POKE) == GetMoveCategory(MOVE_LEAFAGE));
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_WOBBUFFET) { Item(item); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_BRANCH_POKE, MOVE_LEAFAGE); }
    } WHEN {
        if (item == ITEM_ROCKY_HELMET)
            TURN { EXPECT_MOVE(opponent, MOVE_LEAFAGE); }
        else
            TURN { EXPECT_MOVES(opponent, MOVE_LEAFAGE, MOVE_BRANCH_POKE); }
    }
}

AI_SINGLE_BATTLE_TEST("AI uses a guaranteed KO move instead of the move with the highest expected damage")
{
    u32 flags;

    PARAMETRIZE { flags = AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY; }
    PARAMETRIZE { flags = AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT; }

    GIVEN {
        ASSUME(GetMoveCriticalHitStage(MOVE_SLASH) == 1);
        ASSUME(GetMovePower(MOVE_SLASH) == 70);
        ASSUME(GetMovePower(MOVE_STRENGTH) == 80);
        ASSUME(GetMoveType(MOVE_SLASH) == GetMoveType(MOVE_STRENGTH));
        ASSUME(GetMoveCategory(MOVE_SLASH) == GetMoveCategory(MOVE_STRENGTH));
        AI_FLAGS(flags);
        PLAYER(SPECIES_WOBBUFFET) { HP(225); }
        OPPONENT(SPECIES_ABSOL) { Ability(ABILITY_SUPER_LUCK); Moves(MOVE_SLASH, MOVE_STRENGTH); }
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_SLASH); }
        if (flags & AI_FLAG_TRY_TO_FAINT)
            TURN { EXPECT_MOVE(opponent, MOVE_STRENGTH); }
        else
            TURN { EXPECT_MOVE(opponent, MOVE_SLASH); }
    } SCENE {
        if (flags & AI_FLAG_TRY_TO_FAINT)
            MESSAGE("Wobbuffet fainted!");
        else
            NOT MESSAGE("Wobbuffet fainted!");
    }
}

AI_SINGLE_BATTLE_TEST("AI stays choice locked into moves in spite of the player's ability disabling them")
{
    u32 playerMon, aiMove;
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_DAZZLING;          playerMon = SPECIES_BRUXISH;       aiMove = MOVE_QUICK_ATTACK; }
    PARAMETRIZE { ability = ABILITY_QUEENLY_MAJESTY;   playerMon = SPECIES_TSAREENA;      aiMove = MOVE_QUICK_ATTACK; }
    PARAMETRIZE { ability = ABILITY_ARMOR_TAIL;        playerMon = SPECIES_FARIGIRAF;     aiMove = MOVE_QUICK_ATTACK; }
    PARAMETRIZE { ability = ABILITY_SOUNDPROOF;        playerMon = SPECIES_EXPLOUD;       aiMove = MOVE_BOOMBURST; }
    PARAMETRIZE { ability = ABILITY_BULLETPROOF;       playerMon = SPECIES_CHESNAUGHT;    aiMove = MOVE_BULLET_SEED; }

    GIVEN {
        ASSUME(gItemsInfo[ITEM_CHOICE_BAND].holdEffect == HOLD_EFFECT_CHOICE_BAND);
        ASSUME(GetMovePriority(MOVE_QUICK_ATTACK) == 1);
        ASSUME(IsSoundMove(MOVE_BOOMBURST));
        ASSUME(IsBallisticMove(MOVE_BULLET_SEED));
        ASSUME(GetMoveCategory(MOVE_TAIL_WHIP) == DAMAGE_CATEGORY_STATUS);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(playerMon) { Ability(ability); }
        OPPONENT(SPECIES_SMEARGLE) { Item(ITEM_CHOICE_BAND); Moves(aiMove, MOVE_SCRATCH); }
    } WHEN {
        TURN { SWITCH(player, 1); EXPECT_MOVE(opponent, aiMove); }
        TURN { EXPECT_MOVE(opponent, aiMove); }
    }
}

AI_SINGLE_BATTLE_TEST("AI won't use Sucker Punch if it expects a status move a percentage of the time")
{
    PASSES_RANDOMLY(SUCKER_PUNCH_CHANCE, 100, RNG_AI_SUCKER_PUNCH);
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_SUCKER_PUNCH) == EFFECT_SUCKER_PUNCH);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_GROWL, MOVE_SCRATCH); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_SUCKER_PUNCH, MOVE_SCRATCH); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); EXPECT_MOVE(opponent, MOVE_SUCKER_PUNCH); }
    }
}

AI_SINGLE_BATTLE_TEST("AI won't use thawing moves if target is frozen unless it is super effective or it has no other options")
{
    u32 aiFlags = 0; u32 status = 0; u32 aiMove = 0;
    PARAMETRIZE { status = STATUS1_FREEZE;      aiMove = MOVE_SCALD;    aiFlags = 0; }
    PARAMETRIZE { status = STATUS1_FREEZE;      aiMove = MOVE_SCALD;    aiFlags = AI_FLAG_CHECK_BAD_MOVE; }
    PARAMETRIZE { status = STATUS1_FROSTBITE;   aiMove = MOVE_SCALD;    aiFlags = 0; }
    PARAMETRIZE { status = STATUS1_FROSTBITE;   aiMove = MOVE_SCALD;    aiFlags = AI_FLAG_CHECK_BAD_MOVE; }
    PARAMETRIZE { status = STATUS1_FREEZE;      aiMove = MOVE_EMBER;    aiFlags = 0; }
    PARAMETRIZE { status = STATUS1_FREEZE;      aiMove = MOVE_EMBER;    aiFlags = AI_FLAG_CHECK_BAD_MOVE; }
    PARAMETRIZE { status = STATUS1_FROSTBITE;   aiMove = MOVE_EMBER;    aiFlags = 0; }
    PARAMETRIZE { status = STATUS1_FROSTBITE;   aiMove = MOVE_EMBER;    aiFlags = AI_FLAG_CHECK_BAD_MOVE; }

    GIVEN {
        WITH_CONFIG(B_HIT_THAW, GEN_6); // In Gen 5, moves that thawed the user didn't thaw the target
        ASSUME(GetMoveType(MOVE_EMBER) == TYPE_FIRE);
        ASSUME(GetMoveCategory(MOVE_TACKLE) == DAMAGE_CATEGORY_PHYSICAL);
        ASSUME(GetMoveCategory(MOVE_WATER_GUN) == DAMAGE_CATEGORY_SPECIAL);
        ASSUME(MoveThawsUser(MOVE_SCALD) == TRUE);
        AI_FLAGS(aiFlags | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); Status1(status); }
        OPPONENT(SPECIES_VULPIX) { Moves(MOVE_TACKLE, aiMove); }
    } WHEN {
        if (aiFlags == AI_FLAG_CHECK_BAD_MOVE)
            TURN { MOVE(player, MOVE_TACKLE); EXPECT_MOVE(opponent, MOVE_TACKLE); }
        else
            TURN { MOVE(player, MOVE_TACKLE); EXPECT_MOVE(opponent, aiMove); }
    }
}

AI_SINGLE_BATTLE_TEST("AI score for Mean Look will be decreased if target can escape")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_BULBASAUR) { Item(ITEM_SHED_SHELL); }
        OPPONENT(SPECIES_BULBASAUR) { Moves(MOVE_TACKLE, MOVE_MEAN_LOOK); }
    } WHEN {
        TURN { SCORE_EQ_VAL(opponent, MOVE_MEAN_LOOK, 90); }
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_SMART_SWITCHING: AI considers Focus Sash when determining if it should switch out")
{
    GIVEN {
        ASSUME(gItemsInfo[ITEM_FOCUS_SASH].holdEffect == HOLD_EFFECT_FOCUS_SASH);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_SMART_SWITCHING | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_BEAUTIFLY) { Speed(10); Moves(MOVE_AIR_SLASH); }
        OPPONENT(SPECIES_CACNEA) { Speed(1); Moves(MOVE_SCRATCH); }
        OPPONENT(SPECIES_COMBUSKEN) { Speed(1); Moves(MOVE_FLAMETHROWER); Item(ITEM_FOCUS_SASH); }
        OPPONENT(SPECIES_CROBAT) { Speed(11); Moves(MOVE_SLUDGE); }
    } WHEN {
        TURN { MOVE(player, MOVE_AIR_SLASH); EXPECT_MOVE(opponent, MOVE_SCRATCH); EXPECT_SEND_OUT(opponent, 1); }
        TURN { MOVE(player, MOVE_AIR_SLASH); EXPECT_MOVE(opponent, MOVE_FLAMETHROWER); }
    }
}

AI_SINGLE_BATTLE_TEST("AI sees popped Air Balloon")
{
    GIVEN {
        ASSUME(GetItemHoldEffect(ITEM_AIR_BALLOON) == HOLD_EFFECT_AIR_BALLOON);
        ASSUME(GetMoveType(MOVE_EARTHQUAKE) == TYPE_GROUND);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_VIABILITY | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_TORCHIC) { Item(ITEM_AIR_BALLOON); Moves(MOVE_SCRATCH); }
        OPPONENT(SPECIES_GEODUDE) { Moves(MOVE_SCRATCH, MOVE_EARTHQUAKE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); EXPECT_MOVE(opponent, MOVE_SCRATCH); }
        TURN { MOVE(player, MOVE_SCRATCH); EXPECT_MOVE(opponent, MOVE_EARTHQUAKE); }
    }
}

AI_SINGLE_BATTLE_TEST("AI sees popped Air Balloon after Air Balloon mon switches out and back in")
{
    GIVEN {
        ASSUME(GetItemHoldEffect(ITEM_AIR_BALLOON) == HOLD_EFFECT_AIR_BALLOON);
        ASSUME(GetMoveType(MOVE_EARTHQUAKE) == TYPE_GROUND);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_VIABILITY | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_TORCHIC) { Item(ITEM_AIR_BALLOON); Moves(MOVE_SCRATCH); }
        PLAYER(SPECIES_TORCHIC) { Item(ITEM_AIR_BALLOON); Moves(MOVE_SCRATCH); }
        OPPONENT(SPECIES_GEODUDE) { Moves(MOVE_SCRATCH, MOVE_EARTHQUAKE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); EXPECT_MOVE(opponent, MOVE_SCRATCH); }
        TURN { SWITCH(player, 1); EXPECT_MOVE(opponent, MOVE_EARTHQUAKE); }
        TURN { SWITCH(player, 0); EXPECT_MOVE(opponent, MOVE_SCRATCH); }
        TURN { MOVE(player, MOVE_SCRATCH); EXPECT_MOVE(opponent, MOVE_EARTHQUAKE); SEND_OUT(player, 1); }
    }
}

SINGLE_BATTLE_TEST("AI correctly records used moves")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE, MOVE_GROWL, MOVE_FLOWER_TRICK, MOVE_TORCH_SONG); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_RAGE_FIST, MOVE_PSYCHIC, MOVE_SCRATCH, MOVE_EARTHQUAKE); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE);       MOVE(opponent, MOVE_EARTHQUAKE); }
        TURN { MOVE(player, MOVE_FLOWER_TRICK); MOVE(opponent, MOVE_SCRATCH);    }
        TURN { MOVE(player, MOVE_TORCH_SONG);   MOVE(opponent, MOVE_PSYCHIC);    }
        TURN { MOVE(player, MOVE_GROWL);        MOVE(opponent, MOVE_RAGE_FIST);  }
    } THEN {
        EXPECT_EQ(gBattleHistory->usedMoves[B_POSITION_PLAYER_LEFT][0], MOVE_TACKLE);
        EXPECT_EQ(gBattleHistory->usedMoves[B_POSITION_PLAYER_LEFT][1], MOVE_GROWL);
        EXPECT_EQ(gBattleHistory->usedMoves[B_POSITION_PLAYER_LEFT][2], MOVE_FLOWER_TRICK);
        EXPECT_EQ(gBattleHistory->usedMoves[B_POSITION_PLAYER_LEFT][3], MOVE_TORCH_SONG);

        EXPECT_EQ(gBattleHistory->usedMoves[B_POSITION_OPPONENT_LEFT][0], MOVE_RAGE_FIST);
        EXPECT_EQ(gBattleHistory->usedMoves[B_POSITION_OPPONENT_LEFT][1], MOVE_PSYCHIC);
        EXPECT_EQ(gBattleHistory->usedMoves[B_POSITION_OPPONENT_LEFT][2], MOVE_SCRATCH);
        EXPECT_EQ(gBattleHistory->usedMoves[B_POSITION_OPPONENT_LEFT][3], MOVE_EARTHQUAKE);
    }
}

AI_SINGLE_BATTLE_TEST("AI won't boost stats against opponent with Unaware")
{
    GIVEN {
        ASSUME_STAT_CHANGE(MOVE_SWORDS_DANCE, attack: +2);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_VIABILITY);
        PLAYER(SPECIES_QUAGSIRE) { Ability(ABILITY_UNAWARE); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_BODY_SLAM, MOVE_SWORDS_DANCE); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); EXPECT_MOVE(opponent, MOVE_BODY_SLAM); }
    }
}

AI_SINGLE_BATTLE_TEST("AI won't use status moves against opponents that would benefit")
{
    enum Move aiMove;
    PARAMETRIZE { aiMove = MOVE_WILL_O_WISP; }
    PARAMETRIZE { aiMove = MOVE_TOXIC; }
    PARAMETRIZE { aiMove = MOVE_THUNDER_WAVE; }
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_WILL_O_WISP) == EFFECT_NON_VOLATILE_STATUS);
        ASSUME(GetMoveNonVolatileStatus(MOVE_WILL_O_WISP) == MOVE_EFFECT_BURN);
        ASSUME(GetMoveEffect(MOVE_TOXIC) == EFFECT_NON_VOLATILE_STATUS);
        ASSUME(GetMoveNonVolatileStatus(MOVE_TOXIC) == MOVE_EFFECT_TOXIC);
        ASSUME(GetMoveEffect(MOVE_THUNDER_WAVE) == EFFECT_NON_VOLATILE_STATUS);
        ASSUME(GetMoveNonVolatileStatus(MOVE_THUNDER_WAVE) == MOVE_EFFECT_PARALYSIS);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_SWELLOW) { Ability(ABILITY_GUTS); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE, aiMove); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); EXPECT_MOVE(opponent, MOVE_TACKLE); }
    }
}

AI_SINGLE_BATTLE_TEST("AI accepts a low-accuracy sleep swing when it is about to be KOed")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_HYPNOSIS) == EFFECT_NON_VOLATILE_STATUS);
        ASSUME(GetMoveNonVolatileStatus(MOVE_HYPNOSIS) == MOVE_EFFECT_SLEEP);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_STRENGTH); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(100); HP(1); Speed(100); Moves(MOVE_HYPNOSIS, MOVE_CELEBRATE); }
    } WHEN {
        TURN {
            MOVE(player, MOVE_STRENGTH);
            EXPECT_MOVE(opponent, MOVE_HYPNOSIS);
            SCORE_GT_VAL(opponent, MOVE_HYPNOSIS, AI_SCORE_DEFAULT + DECENT_EFFECT);
        }
    } THEN {
        const struct BattleActionLogEntry *opponentLog = BattleActionLog_GetLastEntry(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), 1u << B_ACTION_USE_MOVE);

        EXPECT(opponentLog != NULL);
        EXPECT_EQ(opponentLog->aiReason, AI_DECISION_REASON_DESPERATION_COMEBACK);
        EXPECT((opponentLog->aiThreatFlags & AI_THREAT_DAMAGE_RACE) != 0);
        EXPECT((opponentLog->aiThreatFlags & AI_THREAT_DESPERATION) != 0);
        EXPECT_EQ(opponentLog->aiRiskKind, BATTLE_ACTION_LOG_AI_RISK(AI_RISK_LOW_ACCURACY_STATUS));
    }
}

AI_SINGLE_BATTLE_TEST("AI fishes for flinch against a read Perish Song only without a clean damage line")
{
    enum Move playerMove;
    u16 playerHp;

    PARAMETRIZE { playerMove = MOVE_CELEBRATE;    playerHp = 300; }
    PARAMETRIZE { playerMove = MOVE_PERISH_SONG; playerHp = 300; }
    PARAMETRIZE { playerMove = MOVE_PERISH_SONG; playerHp = 55; }

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_PERISH_SONG) == EFFECT_PERISH_SONG);
        ASSUME(MoveHasAdditionalEffect(MOVE_STOMP, MOVE_EFFECT_FLINCH));
        ASSUME(GetMoveType(MOVE_STOMP) == GetMoveType(MOVE_STRENGTH));
        ASSUME(GetMoveCategory(MOVE_STOMP) == GetMoveCategory(MOVE_STRENGTH));
        ASSUME(GetMovePower(MOVE_STRENGTH) > GetMovePower(MOVE_STOMP));
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_READ_PLAYER_MOVE | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(300); HP(playerHp); Speed(1); Moves(playerMove); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_STOMP, MOVE_STRENGTH); }
    } WHEN {
        TURN {
            MOVE(player, playerMove);
            if (playerMove == MOVE_PERISH_SONG && playerHp > 1)
            {
                EXPECT_MOVE(opponent, MOVE_STOMP);
            }
            else
                EXPECT_MOVE(opponent, MOVE_STRENGTH);
        }
    }
}

AI_SINGLE_BATTLE_TEST("AI short horizon suppresses hax when clean damage answers read Perish Song")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_PERISH_SONG) == EFFECT_PERISH_SONG);
        ASSUME(MoveHasAdditionalEffect(MOVE_STOMP, MOVE_EFFECT_FLINCH));
        ASSUME(GetMovePower(MOVE_BOOMBURST) > GetMovePower(MOVE_STOMP));
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_READ_PLAYER_MOVE | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(300); HP(55); Speed(1); Moves(MOVE_PERISH_SONG); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_STOMP, MOVE_BOOMBURST); }
    } WHEN {
        TURN {
            MOVE(player, MOVE_PERISH_SONG);
            EXPECT_MOVE(opponent, MOVE_BOOMBURST);
        }
    } THEN {
        const struct BattleActionLogEntry *opponentLog = BattleActionLog_GetLastEntry(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), 1u << B_ACTION_USE_MOVE);

        EXPECT(opponentLog != NULL);
        EXPECT_EQ(opponentLog->aiReason, AI_DECISION_REASON_CLEAN_DAMAGE_PREFERRED);
        EXPECT((opponentLog->aiThreatFlags & AI_THREAT_PERISH_TRAP_CLOCK) != 0);
        EXPECT_EQ(opponentLog->aiRiskKind, BATTLE_ACTION_LOG_AI_RISK_NONE);
        EXPECT((opponentLog->aiLineFlags & AI_SHORT_LINE_CLEAN_DAMAGE) != 0);
        EXPECT((opponentLog->aiLineFlags & AI_SHORT_LINE_HIGH_VARIANCE) != 0);
        EXPECT_EQ(opponentLog->aiStableLineFamily, AI_CANDIDATE_LINE_CLEAN_DAMAGE);
        EXPECT_EQ(opponentLog->aiFallbackLineFamily, AI_CANDIDATE_LINE_HIGH_VARIANCE);
        EXPECT_EQ(opponentLog->aiLossClock, 2);
    }
}

AI_SINGLE_BATTLE_TEST("AI logs hax out reason for desperate read Perish Song flinch")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_PERISH_SONG) == EFFECT_PERISH_SONG);
        ASSUME(MoveHasAdditionalEffect(MOVE_STOMP, MOVE_EFFECT_FLINCH));
        ASSUME(GetMoveType(MOVE_STOMP) == GetMoveType(MOVE_STRENGTH));
        ASSUME(GetMoveCategory(MOVE_STOMP) == GetMoveCategory(MOVE_STRENGTH));
        ASSUME(GetMovePower(MOVE_STRENGTH) > GetMovePower(MOVE_STOMP));
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_READ_PLAYER_MOVE | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(300); HP(300); Speed(1); Moves(MOVE_PERISH_SONG); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_STOMP, MOVE_STRENGTH); }
    } WHEN {
        TURN {
            MOVE(player, MOVE_PERISH_SONG);
            EXPECT_MOVE(opponent, MOVE_STOMP);
        }
    } THEN {
        const struct BattleActionLogEntry *opponentLog = BattleActionLog_GetLastEntry(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), 1u << B_ACTION_USE_MOVE);

        EXPECT(opponentLog != NULL);
        EXPECT_EQ(opponentLog->aiReason, AI_DECISION_REASON_HAX_OUT);
        EXPECT((opponentLog->aiThreatFlags & AI_THREAT_PERISH_TRAP_CLOCK) != 0);
        EXPECT((opponentLog->aiThreatFlags & AI_THREAT_DESPERATION) != 0);
        EXPECT_EQ(opponentLog->aiRiskKind, BATTLE_ACTION_LOG_AI_RISK(AI_RISK_SECONDARY_HAX));
    }
}

AI_SINGLE_BATTLE_TEST("AI does not fish for flinch against read Perish Song when it can switch out")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_PERISH_SONG) == EFFECT_PERISH_SONG);
        ASSUME(MoveHasAdditionalEffect(MOVE_STOMP, MOVE_EFFECT_FLINCH));
        ASSUME(GetMoveType(MOVE_STOMP) == GetMoveType(MOVE_STRENGTH));
        ASSUME(GetMoveCategory(MOVE_STOMP) == GetMoveCategory(MOVE_STRENGTH));
        ASSUME(GetMovePower(MOVE_STRENGTH) > GetMovePower(MOVE_STOMP));
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_READ_PLAYER_MOVE | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(300); HP(300); Speed(1); Moves(MOVE_PERISH_SONG); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_STOMP, MOVE_STRENGTH); }
        OPPONENT(SPECIES_WYNAUT) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN {
            MOVE(player, MOVE_PERISH_SONG);
            {
                EXPECT_MOVE(opponent, MOVE_STRENGTH);
            }
        }
    }
}

AI_SINGLE_BATTLE_TEST("AI sees that Primal weather can block a move by type")
{
    GIVEN {
        ASSUME(GetMoveType(MOVE_HYDRO_PUMP) == TYPE_WATER);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_VIABILITY | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_GROUDON) { Item(ITEM_RED_ORB); Moves(MOVE_SCRATCH); }
        OPPONENT(SPECIES_BLASTOISE) { Moves(MOVE_HYDRO_PUMP, MOVE_POUND); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); EXPECT_MOVE(opponent, MOVE_POUND); }
    }
}

AI_DOUBLE_BATTLE_TEST("AI sees opposing drain ability")
{
    GIVEN {
        ASSUME(GetMoveType(MOVE_THUNDERBOLT) == TYPE_ELECTRIC);
        ASSUME(GetMoveType(MOVE_RAZOR_LEAF) != TYPE_ELECTRIC);
        ASSUME(GetMoveType(MOVE_METAL_CLAW) != TYPE_ELECTRIC);
        WITH_CONFIG(B_REDIRECT_ABILITY_IMMUNITY, GEN_5);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_RAICHU) { Ability(ABILITY_LIGHTNING_ROD); Moves(MOVE_CELEBRATE); }
        PLAYER(SPECIES_KRABBY) { Ability(ABILITY_VOLT_ABSORB); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_MAGNETON) { Moves(MOVE_THUNDERBOLT, MOVE_RAZOR_LEAF); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_THUNDERBOLT, MOVE_METAL_CLAW); }
    } WHEN {
        TURN {
            NOT_EXPECT_MOVE(opponentLeft, MOVE_THUNDERBOLT);
            NOT_EXPECT_MOVE(opponentRight, MOVE_THUNDERBOLT); }
    }
}

AI_SINGLE_BATTLE_TEST("AI will not set up Weather if it wont have any affect")
{
    enum Ability ability;

    PARAMETRIZE { ability = ABILITY_CLOUD_NINE; }
    PARAMETRIZE { ability = ABILITY_DAMP; }

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_RAIN_DANCE) == EFFECT_WEATHER);
        ASSUME(GetMoveWeatherType(MOVE_RAIN_DANCE) == BATTLE_WEATHER_RAIN);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_VIABILITY);
        PLAYER(SPECIES_GOLDUCK) { Ability(ability); Moves(MOVE_SCRATCH); }
        OPPONENT(SPECIES_KABUTOPS) { Ability(ABILITY_SWIFT_SWIM); Moves(MOVE_RAIN_DANCE, MOVE_POUND); }
    } WHEN {
        if (ability == ABILITY_CLOUD_NINE)
            TURN { MOVE(player, MOVE_SCRATCH); EXPECT_MOVE(opponent, MOVE_POUND); }
        else
            TURN { MOVE(player, MOVE_SCRATCH); EXPECT_MOVE(opponent, MOVE_RAIN_DANCE); }
    }
}

AI_SINGLE_BATTLE_TEST("Move scoring comparison properly awards bonus point to best OHKO move")
{
    GIVEN {
        ASSUME(MoveHasAdditionalEffect(MOVE_THUNDER, MOVE_EFFECT_PARALYSIS));
        ASSUME(GetMoveAdditionalEffectCount(MOVE_WATER_SPOUT) == 0);
        ASSUME(GetMoveAdditionalEffectCount(MOVE_WATER_GUN) == 0);
        ASSUME(GetMoveAdditionalEffectCount(MOVE_ORIGIN_PULSE) == 0);
        ASSUME(GetMoveAccuracy(MOVE_WATER_SPOUT) > GetMoveAccuracy(MOVE_THUNDER));
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_VIABILITY);
        PLAYER(SPECIES_WAILORD) { Level(50); }
        OPPONENT(SPECIES_WAILORD) { Moves(MOVE_THUNDER, MOVE_WATER_SPOUT, MOVE_WATER_GUN, MOVE_SURF); }
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_WATER_SPOUT); }
    }
}

AI_SINGLE_BATTLE_TEST("AI will stop setting up at +4")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_TACKLE, MOVE_IRON_DEFENSE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); EXPECT_MOVE(opponent, MOVE_IRON_DEFENSE); }
        TURN { MOVE(player, MOVE_CELEBRATE); EXPECT_MOVE(opponent, MOVE_IRON_DEFENSE); }
        TURN { MOVE(player, MOVE_CELEBRATE); EXPECT_MOVE(opponent, MOVE_TACKLE); }
    }
}

AI_SINGLE_BATTLE_TEST("Move scoring comparison properly awards bonus point to best OHKO move")
{
    GIVEN {
        ASSUME(MoveHasAdditionalEffect(MOVE_THUNDER, MOVE_EFFECT_PARALYSIS));
        ASSUME(GetMoveAdditionalEffectCount(MOVE_WATER_SPOUT) == 0);
        ASSUME(GetMoveAdditionalEffectCount(MOVE_WATER_GUN) == 0);
        ASSUME(GetMoveAdditionalEffectCount(MOVE_ORIGIN_PULSE) == 0);
        ASSUME(GetMoveAccuracy(MOVE_WATER_SPOUT) > GetMoveAccuracy(MOVE_THUNDER));
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_VIABILITY);
        PLAYER(SPECIES_WAILORD) { Level(50); }
        OPPONENT(SPECIES_WAILORD) { Moves(MOVE_THUNDER, MOVE_WATER_SPOUT, MOVE_WATER_GUN, MOVE_SURF); }
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_WATER_SPOUT); }
    }
}

AI_SINGLE_BATTLE_TEST("AI will see Magnitude damage")
{
    PASSES_RANDOMLY(SHOULD_SWITCH_HASBADODDS_PERCENTAGE, 100, RNG_AI_SWITCH_HASBADODDS);
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_MAGNITUDE) == EFFECT_MAGNITUDE);
        ASSUME(GetMoveType(MOVE_MAGNITUDE) == TYPE_GROUND);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_VIABILITY | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_GEODUDE) { Level(15); Moves(MOVE_DEFENSE_CURL, MOVE_MAGNITUDE); }
        OPPONENT(SPECIES_TYRUNT) { Level(13); Moves(MOVE_BITE, MOVE_THUNDER_FANG, MOVE_ROCK_TOMB); }
        OPPONENT(SPECIES_ZUBAT) { Level(14); Moves(MOVE_TACKLE); }
    } WHEN {
        TURN { MOVE(player, MOVE_MAGNITUDE); EXPECT_SWITCH(opponent, 1); }
    }
}

AI_SINGLE_BATTLE_TEST("AI will prefer resisted move over failing move")
{
    GIVEN {
        WITH_CONFIG(B_POWDER_GRASS, GEN_6);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_VIABILITY);
        PLAYER(SPECIES_ROSELIA) { Moves(MOVE_ABSORB); }
        OPPONENT(SPECIES_GLOOM) { Moves(MOVE_MEGA_DRAIN, MOVE_STUN_SPORE, MOVE_LEECH_SEED, MOVE_SYNTHESIS); }
    } WHEN {
        TURN { MOVE(player, MOVE_ABSORB); EXPECT_MOVE(opponent, MOVE_MEGA_DRAIN); }
    }
}

AI_SINGLE_BATTLE_TEST("AI won't setup if it can KO through Sturdy effect")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_SKARMORY) { Ability(ABILITY_STURDY); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_MOLTRES) { Moves(MOVE_FIRE_BLAST, MOVE_AGILITY); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); EXPECT_MOVE(opponent, MOVE_FIRE_BLAST); }
    }
}

AI_SINGLE_BATTLE_TEST("AI won't setup if otherwise good scenario is changed by the presence of priority")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_FLOATZEL) { Speed(2); Moves(MOVE_AQUA_JET, MOVE_SURF); }
        OPPONENT(SPECIES_DONPHAN) { Speed(5); Moves(MOVE_BULK_UP, MOVE_EARTHQUAKE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SURF); EXPECT_MOVE(opponent, MOVE_EARTHQUAKE); }
    }
}

AI_SINGLE_BATTLE_TEST("AI will use Recovery move if it outheals your damage and outspeeds")
{
    PASSES_RANDOMLY(100, 100, RNG_AI_SHOULD_RECOVER);
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_LINOONE) { Speed(2); Moves(MOVE_HEADBUTT); }
        OPPONENT(SPECIES_GASTRODON) { Speed(5); Moves(MOVE_SCALD, MOVE_RECOVER); HP(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_HEADBUTT); EXPECT_MOVE(opponent, MOVE_RECOVER); }
    }
}

AI_SINGLE_BATTLE_TEST("AI will use recovery move if it outheals your damage and is outsped")
{
    enum Move aiMove = MOVE_NONE;
    PASSES_RANDOMLY(100, 100, RNG_AI_SHOULD_RECOVER);
    PARAMETRIZE{ aiMove = MOVE_RECOVER; }
    PARAMETRIZE{ aiMove = MOVE_STRENGTH_SAP; }
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_LINOONE) { Speed(5); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_GASTRODON) { Speed(2); Moves(MOVE_SCALD, aiMove); HP(200); MaxHP(400); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); EXPECT_MOVE(opponent, aiMove); }
    }
}

AI_SINGLE_BATTLE_TEST("AI will use recovery move if is in no immediate danger beneath an HP threshold")
{
    PASSES_RANDOMLY(SHOULD_RECOVER_CHANCE, 100, RNG_AI_SHOULD_RECOVER);
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_LINOONE) { Speed(2); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_GASTRODON) { Speed(5); Moves(MOVE_SCALD, MOVE_RECOVER); HP(200); MaxHP(400); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); EXPECT_MOVE(opponent, MOVE_RECOVER); }
    }
}

AI_SINGLE_BATTLE_TEST("AI has a chance to prioritize last chance priority damage over slow KO")
{
    PASSES_RANDOMLY(PRIORITIZE_LAST_CHANCE_CHANCE, 100, RNG_AI_PRIORITIZE_LAST_CHANCE);
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_CAMERUPT) { Speed(2); Moves(MOVE_FLAMETHROWER, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_FLOATZEL) { Level(85); Speed(1); HP(1); Moves(MOVE_WAVE_CRASH, MOVE_AQUA_JET); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); EXPECT_MOVE(opponent, MOVE_AQUA_JET); }
    }
}

AI_DOUBLE_BATTLE_TEST("AI won't be confused by player's previous priority moves when evaluating KOs")
{
    PASSES_RANDOMLY(100, 100);
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_BEAUTIFLY) { Speed(1); Moves(MOVE_DETECT, MOVE_SCRATCH); }
        PLAYER(SPECIES_MASQUERAIN) { Speed(10); Moves(MOVE_DETECT, MOVE_SCRATCH); }
        OPPONENT(SPECIES_CRADILY) { Speed(5); Moves(MOVE_POWER_GEM); }
        OPPONENT(SPECIES_ZIGZAGOON) { Speed(4); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_DETECT); MOVE(playerRight, MOVE_DETECT); EXPECT_MOVE(opponentLeft, MOVE_POWER_GEM, target:playerLeft); EXPECT_MOVE(opponentRight, MOVE_CELEBRATE); }
        TURN { MOVE(playerLeft, MOVE_DETECT); MOVE(playerRight, MOVE_DETECT); EXPECT_MOVE(opponentLeft, MOVE_POWER_GEM, target:playerLeft); EXPECT_MOVE(opponentRight, MOVE_CELEBRATE); }
    }
}

AI_SINGLE_BATTLE_TEST("AI will see 2HKOs through resist berries")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_ZIGZAGOON) { HP(117); Moves(MOVE_CELEBRATE); Item(ITEM_CHOPLE_BERRY); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_JUMP_KICK, MOVE_HEADBUTT); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); EXPECT_MOVE(opponent, MOVE_JUMP_KICK); }
    }
}

AI_SINGLE_BATTLE_TEST("AI will prioritize a regular OHKO over a berry-ignoring OHKO")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_ZIGZAGOON) { HP(75); Moves(MOVE_CELEBRATE); Item(ITEM_CHOPLE_BERRY); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_BOOMBURST, MOVE_VITAL_THROW); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); EXPECT_MOVES(opponent, MOVE_BOOMBURST, MOVE_VITAL_THROW); }
        SCORE_GT(opponent, MOVE_BOOMBURST, MOVE_VITAL_THROW);
    }
}

AI_SINGLE_BATTLE_TEST("AI will not prioritize a regular OHKO over a berry-reduced OHKO")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_ZIGZAGOON) { HP(1); Moves(MOVE_CELEBRATE); Item(ITEM_CHOPLE_BERRY); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_SCRATCH, MOVE_KARATE_CHOP); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); EXPECT_MOVES(opponent, MOVE_SCRATCH, MOVE_KARATE_CHOP); }
        SCORE_EQ(opponent, MOVE_SCRATCH, MOVE_KARATE_CHOP);
    }
}

AI_SINGLE_BATTLE_TEST("AI won't increase its stats if it's about to fall asleep due to Yawn")
{
    enum Move aiMove;
    PARAMETRIZE { aiMove = MOVE_CELEBRATE; }
    PARAMETRIZE { aiMove = MOVE_SWORDS_DANCE; }
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_YAWN, MOVE_SCRATCH); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(aiMove, MOVE_SCRATCH); }
    } WHEN {
        if (aiMove == MOVE_CELEBRATE)
            TURN { MOVE(player, MOVE_YAWN); EXPECT_MOVE(opponent, MOVE_SCRATCH); }
        else
            TURN { MOVE(player, MOVE_YAWN); EXPECT_MOVE(opponent, MOVE_SWORDS_DANCE); }
        TURN { MOVE(player, MOVE_SCRATCH); EXPECT_MOVE(opponent, MOVE_SCRATCH); }
    }
}

AI_SINGLE_BATTLE_TEST("AI will consider using Explosion inversely proportional to its missing HP")
{
    u32 monHP; u32 passesChance;
    PARAMETRIZE { monHP = 20; passesChance = EXPLOSION_MINIMUM_CHANCE; }
    PARAMETRIZE { monHP = 1; passesChance = EXPLOSION_MAXIMUM_CHANCE; }
    PASSES_RANDOMLY(passesChance, 100, RNG_AI_CONSIDER_EXPLOSION);
    GIVEN {
        ASSUME(IsExplosionMove(MOVE_EXPLOSION));
        ASSUME(EXPLOSION_LOWER_HP_THRESHOLD == 10);
        ASSUME(EXPLOSION_HIGHER_HP_THRESHOLD == 90);
        ASSUME(LAST_MON_PREFERS_NOT_SACRIFICE == FALSE);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_ZIGZAGOON) { Level(5); Moves(MOVE_SCRATCH); }
        OPPONENT(SPECIES_ZIGZAGOON) { Level(5); HP(monHP); MaxHP(20); Moves(MOVE_SCRATCH, MOVE_EXPLOSION); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); EXPECT_MOVE(opponent, MOVE_EXPLOSION); }
    }
}

AI_SINGLE_BATTLE_TEST("AI will prioritize non-self-sacrificing moves if they have the same hits to KO")
{
    enum Move selfSacrificeMove;
    PARAMETRIZE { selfSacrificeMove = MOVE_EXPLOSION; }
    PARAMETRIZE { selfSacrificeMove = MOVE_FINAL_GAMBIT; }
    PASSES_RANDOMLY(100, 100, RNG_AI_CONSIDER_EXPLOSION);
    GIVEN {
        ASSUME(IsExplosionMove(MOVE_EXPLOSION));
        ASSUME(GetMoveEffect(MOVE_FINAL_GAMBIT) == EFFECT_FINAL_GAMBIT);
        ASSUME(EXPLOSION_LOWER_HP_THRESHOLD == 10);
        ASSUME(EXPLOSION_HIGHER_HP_THRESHOLD == 90);
        ASSUME(LAST_MON_PREFERS_NOT_SACRIFICE == FALSE);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_ZIGZAGOON) { Level(5); HP(1); Speed(1); Moves(MOVE_SCRATCH); }
        OPPONENT(SPECIES_ZIGZAGOON) { Level(5); HP(1); MaxHP(20); Speed(2); Moves(MOVE_SCRATCH, selfSacrificeMove); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); EXPECT_MOVE(opponent, MOVE_SCRATCH); }
    }
}

AI_SINGLE_BATTLE_TEST("AI will consider using Final Gambit if it expects to KO and outspeeds")
{
    u32 aiOmniscientFlag;
    PARAMETRIZE { aiOmniscientFlag = AI_FLAG_OMNISCIENT; }
    PARAMETRIZE { aiOmniscientFlag = 0 ;}
    PASSES_RANDOMLY(FINAL_GAMBIT_CHANCE, 100, RNG_AI_FINAL_GAMBIT);
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FINAL_GAMBIT) == EFFECT_FINAL_GAMBIT);
        ASSUME(LAST_MON_PREFERS_NOT_SACRIFICE == FALSE);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | aiOmniscientFlag);
        PLAYER(SPECIES_ZIGZAGOON) { Level(5); MaxHP(20); Speed(1); Moves(MOVE_SCRATCH); }
        OPPONENT(SPECIES_ZIGZAGOON) { Level(5); MaxHP(20); Speed(2); Moves(MOVE_SCRATCH, MOVE_FINAL_GAMBIT); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); EXPECT_MOVE(opponent, MOVE_FINAL_GAMBIT); }
    }
}

AI_SINGLE_BATTLE_TEST("AI's Explosion scoring handles multiple move effects and the Explosion defense config")
{
    u32 genConfig; u32 passesChance;
    PARAMETRIZE { genConfig = GEN_5; passesChance = 0; }
    PARAMETRIZE { genConfig = GEN_4; passesChance = EXPLOSION_MAXIMUM_CHANCE; }
    PASSES_RANDOMLY(passesChance, 100, RNG_AI_CONSIDER_EXPLOSION);
    GIVEN {
        WITH_CONFIG(B_EXPLOSION_DEFENSE, genConfig);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_VIABILITY | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_CLOYSTER) { Level(44); HP(68); Moves(MOVE_DETECT, MOVE_RAZOR_SHELL, MOVE_ICICLE_SPEAR, MOVE_ICE_SHARD); }
        OPPONENT(SPECIES_GLALIE_MEGA) { Level(44); HP(1); Ability(ABILITY_REFRIGERATE); Friendship(MAX_FRIENDSHIP); Moves(MOVE_RETURN, MOVE_EARTHQUAKE, MOVE_EXPLOSION, MOVE_CRUNCH); }
    } WHEN {
        TURN { MOVE(player, MOVE_DETECT); EXPECT_MOVE(opponent, MOVE_EXPLOSION); }
    }
}

AI_DOUBLE_BATTLE_TEST("AI won't be confused by player's one-shot-priority moves (ie. Fake Out, Detect) when comparing speed")
{
    PASSES_RANDOMLY(100, 100);
    GIVEN {
        ASSUME(GetMovePriority(MOVE_DETECT) > GetMovePriority(MOVE_AQUA_JET));
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_COMBUSKEN) { Level(20); HP(1); Ability(ABILITY_SPEED_BOOST); Item(ITEM_BRIGHT_POWDER); Moves(MOVE_DETECT, MOVE_DOUBLE_KICK); }
        PLAYER(SPECIES_QUILAVA) { Level(20); Ability(ABILITY_ADAPTABILITY); Item(ITEM_ORAN_BERRY); Moves(MOVE_MUD_SHOT, MOVE_INCINERATE); }
        OPPONENT(SPECIES_PARAS) { Level(16); Ability(ABILITY_DRY_SKIN); Item(ITEM_QUICK_CLAW); Moves(MOVE_SLEEP_POWDER, MOVE_BUG_BITE, MOVE_AERIAL_ACE, MOVE_POISON_FANG); }
        OPPONENT(SPECIES_BUIZEL) { Level(16); Ability(ABILITY_SWIFT_SWIM); Item(ITEM_DAMP_ROCK); Moves(MOVE_RAIN_DANCE, MOVE_HELPING_HAND, MOVE_ICE_FANG, MOVE_AQUA_JET); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_DETECT); MOVE(playerRight, MOVE_INCINERATE, target:opponentLeft); EXPECT_MOVE(opponentRight, MOVE_AQUA_JET, target:playerLeft); }
    }
}

TEST("AI hits to KO damage rounding works correctly")
{
    EXPECT_EQ(GetNoOfHitsToKO(4, 12), 3);
    EXPECT_EQ(GetNoOfHitsToKO(16, 50), 4);
}

AI_SINGLE_BATTLE_TEST("AI is encouraged to use pivot moves if it outspeeds and should switch")
{
    PASSES_RANDOMLY(SHOULD_SWITCH_HASBADODDS_PERCENTAGE, 100, RNG_AI_SWITCH_HASBADODDS);
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_U_TURN) == EFFECT_HIT_ESCAPE);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_VIABILITY | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_ZIGZAGOON) { Speed(1); Moves(MOVE_SCRATCH, MOVE_GROWL); }
        OPPONENT(SPECIES_ZIGZAGOON) { Speed(2); HP(3); MaxHP(5); Moves(MOVE_U_TURN, MOVE_STRENGTH); }
        OPPONENT(SPECIES_METAGROSS) { Speed(2); Moves(MOVE_METEOR_MASH); }
    } WHEN {
        TURN { MOVE(player, MOVE_GROWL); EXPECT_MOVE(opponent, MOVE_U_TURN); }
    }
}

AI_SINGLE_BATTLE_TEST("AI is encouraged to use pivot moves if it is outsped, survives a hit, and should switch")
{
    PASSES_RANDOMLY(SHOULD_SWITCH_HASBADODDS_PERCENTAGE, 100, RNG_AI_SWITCH_HASBADODDS);
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_U_TURN) == EFFECT_HIT_ESCAPE);
        ASSUME(GetMoveEffect(MOVE_DRAGON_RAGE) == EFFECT_FIXED_HP_DAMAGE);
        ASSUME(GetMoveFixedHPDamage(MOVE_DRAGON_RAGE) == 40);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_VIABILITY | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_MACHAMP) { Speed(2); Moves(MOVE_DRAGON_RAGE, MOVE_GROWL); }
        OPPONENT(SPECIES_ZIGZAGOON) { Speed(1); MaxHP(41); Moves(MOVE_U_TURN, MOVE_STRENGTH); }
        OPPONENT(SPECIES_METAGROSS) { Speed(2); Moves(MOVE_METEOR_MASH); }
    } WHEN {
        TURN { MOVE(player, MOVE_GROWL); EXPECT_MOVE(opponent, MOVE_U_TURN); }
    }
}

AI_SINGLE_BATTLE_TEST("AI is encouraged to use pivot moves if the target has a Sash or Multiscale effect and the AI has a good switchin")
{
    PASSES_RANDOMLY(SHOULD_PIVOT_BREAK_SASH_CHANCE, 100, RNG_AI_SHOULD_PIVOT_BREAK_SASH);
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_U_TURN) == EFFECT_HIT_ESCAPE);
        ASSUME(GetItemHoldEffect(ITEM_FOCUS_SASH) == HOLD_EFFECT_FOCUS_SASH);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_VIABILITY | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_ZIGZAGOON) { Item(ITEM_FOCUS_SASH); Moves(MOVE_GROWL); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_U_TURN, MOVE_STRENGTH); }
        OPPONENT(SPECIES_METAGROSS) { Moves(MOVE_METEOR_MASH); }
    } WHEN {
        TURN { MOVE(player, MOVE_GROWL); EXPECT_MOVE(opponent, MOVE_U_TURN); }
    }
}

AI_SINGLE_BATTLE_TEST("AI is encouraged to use pivot moves if it benefits from Regenerator and has a good switchin")
{
    PASSES_RANDOMLY(100, 100);
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_U_TURN) == EFFECT_HIT_ESCAPE);
        ASSUME(GetMoveEffect(MOVE_DRAGON_RAGE) == EFFECT_FIXED_HP_DAMAGE);
        ASSUME(GetMoveFixedHPDamage(MOVE_DRAGON_RAGE) == 40);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_VIABILITY | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_ZIGZAGOON) { Speed(1); Moves(MOVE_DRAGON_RAGE, MOVE_GROWL); }
        OPPONENT(SPECIES_TANGELA) { Speed(2); HP(30); MaxHP(100); Ability(ABILITY_REGENERATOR); Moves(MOVE_U_TURN, MOVE_MAGICAL_LEAF); }
        OPPONENT(SPECIES_METAGROSS) { Speed(2); Moves(MOVE_METEOR_MASH); }
    } WHEN {
        TURN { MOVE(player, MOVE_GROWL); EXPECT_MOVE(opponent, MOVE_U_TURN); }
    }
}

AI_SINGLE_BATTLE_TEST("AI is discouraged from using pivot moves if it has no good switchin and does not KO")
{
    PASSES_RANDOMLY(100, 100);
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_U_TURN) == EFFECT_HIT_ESCAPE);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_VIABILITY | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_ZIGZAGOON) { Speed(1); Moves(MOVE_SCRATCH, MOVE_GROWL); }
        OPPONENT(SPECIES_ZIGZAGOON) { Speed(2); HP(3); MaxHP(5); Moves(MOVE_U_TURN, MOVE_STRENGTH); }
        OPPONENT(SPECIES_ZIGZAGOON) { Speed(1); Level(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_GROWL); EXPECT_MOVE(opponent, MOVE_STRENGTH); }
    }
}

AI_SINGLE_BATTLE_TEST("AI will try to withstand hit with absorbing move")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_VIABILITY | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_DRAGON_RAGE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(2); HP(39); Moves(MOVE_ENERGY_BALL, MOVE_GIGA_DRAIN); }
    } WHEN {
        TURN { MOVE(player, MOVE_DRAGON_RAGE); EXPECT_MOVE(opponent, MOVE_GIGA_DRAIN); }
    }
}

AI_SINGLE_BATTLE_TEST("AI will not try to withstand hit with absorbing move if it will still be KO'd")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_VIABILITY | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_DRAGON_RAGE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(2); MaxHP(40); HP(39); Moves(MOVE_ENERGY_BALL, MOVE_GIGA_DRAIN); }
    } WHEN {
        TURN { MOVE(player, MOVE_DRAGON_RAGE); EXPECT_MOVE(opponent, MOVE_ENERGY_BALL); }
    }
}

AI_DOUBLE_BATTLE_TEST("AI can use Acupressure on its ally")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_VIABILITY | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        PLAYER(SPECIES_WYNAUT) { HP(1); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_ACUPRESSURE); }
        OPPONENT(SPECIES_WYNAUT) { Moves(MOVE_SCRATCH); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_CELEBRATE); MOVE(playerRight, MOVE_CELEBRATE); EXPECT_MOVE(opponentRight, MOVE_SCRATCH, target:playerRight); EXPECT_MOVE(opponentLeft, MOVE_ACUPRESSURE, target:opponentRight); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ACUPRESSURE, opponentLeft);
    }
}

AI_SINGLE_BATTLE_TEST("AI's comparison of damaging moves correctly reads moveset indexes for effects")
{
    u32 move = MOVE_NONE;
    PARAMETRIZE { move = MOVE_TACKLE; }
    PARAMETRIZE { move = MOVE_DUAL_CHOP; }
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_VIABILITY | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_RAPIDASH_GALAR){ Level(64); HP(1); Nature(NATURE_TIMID); Moves(MOVE_TACKLE);}
        OPPONENT(SPECIES_HAXORUS){ Level(64); Nature(NATURE_JOLLY); Ability(ABILITY_MOLD_BREAKER); Moves(move, MOVE_EARTHQUAKE, MOVE_POISON_JAB); }
    } WHEN {
        TURN {
            MOVE(player, MOVE_TACKLE);
            if (move == MOVE_TACKLE)
                SCORE_EQ_VAL(opponent, MOVE_TACKLE, 104);
            else if (move == MOVE_DUAL_CHOP)
                SCORE_EQ_VAL(opponent, MOVE_DUAL_CHOP, 60);
            SCORE_EQ_VAL(opponent, MOVE_EARTHQUAKE, 104);
            SCORE_EQ_VAL(opponent, MOVE_POISON_JAB, 105);
        }
    }
}

AI_SINGLE_BATTLE_TEST("Bolt Beak damage will be correctly seen by AI (singles)")
{
    u32 playerSpeed, aiSpeed;

    PARAMETRIZE { playerSpeed = 20; aiSpeed = 10; }
    PARAMETRIZE { playerSpeed = 10; aiSpeed = 20; }

    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_STARMIE) { Speed(playerSpeed); Moves(MOVE_PROTECT, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_ZAPDOS) { Speed(aiSpeed); Moves(MOVE_BOLT_BEAK, MOVE_THUNDER); }
    } WHEN {
        if (playerSpeed > aiSpeed) {
            TURN { MOVE(player, MOVE_PROTECT); EXPECT_MOVE(opponent, MOVE_THUNDER); }
            TURN { EXPECT_MOVE(opponent, MOVE_THUNDER); }
        } else {
            TURN { MOVE(player, MOVE_PROTECT); EXPECT_MOVE(opponent, MOVE_BOLT_BEAK); }
            TURN { EXPECT_MOVE(opponent, MOVE_BOLT_BEAK); }
        }
    }
}

AI_DOUBLE_BATTLE_TEST("Bolt Beak damage will be correctly seen by AI (doubles)")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_PROTECT, MOVE_CELEBRATE); }
        PLAYER(SPECIES_WOBBUFFET) { Speed(3); Moves(MOVE_PROTECT, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_ZAPDOS)  { Speed(2); Moves(MOVE_BOLT_BEAK, MOVE_THUNDER); }
        OPPONENT(SPECIES_ZAPDOS)  { Speed(4); Moves(MOVE_BOLT_BEAK, MOVE_THUNDER); }
        TIE_BREAK_TARGET(TARGET_TIE_HI, 0);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_PROTECT);
            MOVE(playerRight, MOVE_PROTECT);
            SCORE_EQ_VAL(opponentLeft,  MOVE_BOLT_BEAK, AI_SCORE_DEFAULT + BEST_DAMAGE_MOVE, target:playerLeft);
            SCORE_EQ_VAL(opponentLeft,  MOVE_BOLT_BEAK, AI_SCORE_DEFAULT,                    target:playerRight);
            SCORE_EQ_VAL(opponentLeft,  MOVE_THUNDER,   AI_SCORE_DEFAULT,                    target:playerLeft);
            SCORE_EQ_VAL(opponentLeft,  MOVE_THUNDER,   AI_SCORE_DEFAULT + BEST_DAMAGE_MOVE, target:playerRight);
            SCORE_EQ_VAL(opponentRight, MOVE_BOLT_BEAK, AI_SCORE_DEFAULT + BEST_DAMAGE_MOVE, target:playerLeft);
            SCORE_EQ_VAL(opponentRight, MOVE_BOLT_BEAK, AI_SCORE_DEFAULT + BEST_DAMAGE_MOVE, target:playerRight);
            SCORE_EQ_VAL(opponentRight, MOVE_THUNDER,   AI_SCORE_DEFAULT,                    target:playerLeft);
            SCORE_EQ_VAL(opponentRight, MOVE_THUNDER,   AI_SCORE_DEFAULT,                    target:playerRight);
            EXPECT_MOVE(opponentLeft,   MOVE_THUNDER,   target:playerRight);
            EXPECT_MOVE(opponentRight,  MOVE_BOLT_BEAK, target:playerRight);
        }
        TURN {
            SCORE_EQ_VAL(opponentLeft,  MOVE_BOLT_BEAK, AI_SCORE_DEFAULT + BEST_DAMAGE_MOVE, target:playerLeft);
            SCORE_EQ_VAL(opponentLeft,  MOVE_BOLT_BEAK, AI_SCORE_DEFAULT,                    target:playerRight);
            SCORE_EQ_VAL(opponentLeft,  MOVE_THUNDER,   AI_SCORE_DEFAULT,                    target:playerLeft);
            SCORE_EQ_VAL(opponentLeft,  MOVE_THUNDER,   AI_SCORE_DEFAULT + BEST_DAMAGE_MOVE, target:playerRight);
            SCORE_EQ_VAL(opponentRight, MOVE_BOLT_BEAK, AI_SCORE_DEFAULT + BEST_DAMAGE_MOVE, target:playerLeft);
            SCORE_EQ_VAL(opponentRight, MOVE_BOLT_BEAK, AI_SCORE_DEFAULT + BEST_DAMAGE_MOVE, target:playerRight);
            SCORE_EQ_VAL(opponentRight, MOVE_THUNDER,   AI_SCORE_DEFAULT,                    target:playerLeft);
            SCORE_EQ_VAL(opponentRight, MOVE_THUNDER,   AI_SCORE_DEFAULT,                    target:playerRight);
            EXPECT_MOVE(opponentLeft,   MOVE_THUNDER,   target:playerRight);
            EXPECT_MOVE(opponentRight,  MOVE_BOLT_BEAK, target:playerRight);
        }
    }
}
