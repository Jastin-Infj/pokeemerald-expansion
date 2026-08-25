#ifndef GUARD_BATTLE_AI_JOINT_PLANNER_H
#define GUARD_BATTLE_AI_JOINT_PLANNER_H

#include "global.h"
#include "battle_ai_board_sim.h"

#define AI_JOINT_MAX_ATOMIC_INPUT        32
#define AI_JOINT_MAX_ATOMIC_PER_ACTOR    12
#define AI_JOINT_MAX_ROOT_PAIRS          32
#define AI_JOINT_MAX_FUTURE_PAIRS        16
#define AI_JOINT_MAX_TRACE_ROOTS          8
#define AI_JOINT_MAX_SEARCH_DEPTH         5
#define AI_JOINT_STANDARD_DEPTH           3
#define AI_JOINT_EXTENSION_DEPTH          5
#define AI_JOINT_DEPTH3_RETENTION_WIDTH   4
#define AI_JOINT_DEPTH3_SEARCH_WIDTH      3
#define AI_JOINT_DEPTH5_BEAM_WIDTH        3
#define AI_JOINT_TRANSPOSITION_ENTRIES   16
#define AI_JOINT_DEFAULT_STEP_NODES       12
#define AI_JOINT_DEFAULT_DEPTH3_NODES   2048
#define AI_JOINT_DEFAULT_TOTAL_NODES    3072
#define AI_JOINT_DEFAULT_DEPTH3_FRAMES  160
#define AI_JOINT_DEFAULT_TOTAL_FRAMES   220
#define AI_JOINT_DEFAULT_CLOSE_SCORE     24
#define AI_JOINT_MAX_EXTENSION_ROOTS      3

enum AiJointCandidateFamily
{
    AI_JOINT_FAMILY_NONE,
    AI_JOINT_FAMILY_CLEAN_KO,
    AI_JOINT_FAMILY_CLEAN_DAMAGE,
    AI_JOINT_FAMILY_ALT_TARGET,
    AI_JOINT_FAMILY_PROTECT,
    AI_JOINT_FAMILY_SWITCH,
    AI_JOINT_FAMILY_SETUP,
    AI_JOINT_FAMILY_DISRUPTION,
    AI_JOINT_FAMILY_SPEED_FIELD,
    AI_JOINT_FAMILY_SUPPORT,
    AI_JOINT_FAMILY_APPROVED_ALLY,
    AI_JOINT_FAMILY_GIMMICK,
    AI_JOINT_FAMILY_OTHER,
    AI_JOINT_FAMILY_COUNT,
};

enum AiJointAtomicFlags
{
    AI_JOINT_ATOMIC_VALID                    = 1 << 0,
    AI_JOINT_ATOMIC_GUARANTEED_KO            = 1 << 1,
    AI_JOINT_ATOMIC_REQUIRES_PARTNER_ACTION  = 1 << 2,
    AI_JOINT_ATOMIC_PREVENTS_PARTNER_ACTION  = 1 << 3,
    AI_JOINT_ATOMIC_SUPPORTS_PARTNER         = 1 << 4,
    AI_JOINT_ATOMIC_FIELD_SETUP              = 1 << 5,
    AI_JOINT_ATOMIC_SPREAD                   = 1 << 6,
    AI_JOINT_ATOMIC_UNRESOLVED               = 1 << 7,
};

enum AiJointLegalityFlags
{
    AI_JOINT_LEGAL                           = 0,
    AI_JOINT_ILLEGAL_TARGET                  = 1 << 0,
    AI_JOINT_ILLEGAL_UNAPPROVED_ALLY         = 1 << 1,
    AI_JOINT_ILLEGAL_NO_PP                    = 1 << 2,
    AI_JOINT_ILLEGAL_GIMMICK                  = 1 << 3,
    AI_JOINT_ILLEGAL_SWITCH                   = 1 << 4,
    AI_JOINT_ILLEGAL_OTHER                    = 1 << 5,
};

enum AiJointResourceFlags
{
    AI_JOINT_RESOURCE_NONE                   = 0,
    AI_JOINT_RESOURCE_MEGA                   = 1 << 0,
    AI_JOINT_RESOURCE_ULTRA                  = 1 << 1,
    AI_JOINT_RESOURCE_Z_MOVE                 = 1 << 2,
    AI_JOINT_RESOURCE_DYNAMAX                = 1 << 3,
    AI_JOINT_RESOURCE_TERA                   = 1 << 4,
    AI_JOINT_RESOURCE_HELD_ITEM              = 1 << 5,
};

enum AiJointPairFlags
{
    AI_JOINT_PAIR_VALID                      = 1 << 0,
    AI_JOINT_PAIR_OVERKILL                   = 1 << 1,
    AI_JOINT_PAIR_PROTECT_PARTNER_KO         = 1 << 2,
    AI_JOINT_PAIR_UNRESOLVED                 = 1 << 3,
    AI_JOINT_PAIR_FALLBACK                   = 1 << 4,
    AI_JOINT_PAIR_UNSUPPORTED                = 1 << 5,
};

enum AiJointPairConflict
{
    AI_JOINT_CONFLICT_NONE,
    AI_JOINT_CONFLICT_ILLEGAL_ACTION,
    AI_JOINT_CONFLICT_SAME_RESERVE,
    AI_JOINT_CONFLICT_SHARED_GIMMICK,
    AI_JOINT_CONFLICT_PARTNER_REQUIREMENT,
    AI_JOINT_CONFLICT_DUPLICATE_FIELD_SETUP,
};

enum AiJointPlannerState
{
    AI_JOINT_JOB_EMPTY,
    AI_JOINT_JOB_BUILDING,
    AI_JOINT_JOB_DEPTH3_READY,
    AI_JOINT_JOB_EXTENDING5,
    AI_JOINT_JOB_READY,
    AI_JOINT_JOB_FALLBACK,
};

enum AiJointTerminationReason
{
    AI_JOINT_TERMINATION_COMPLETE,
    AI_JOINT_TERMINATION_NODE_BUDGET,
    AI_JOINT_TERMINATION_FRAME_BUDGET,
    AI_JOINT_TERMINATION_NO_ROOT,
    AI_JOINT_TERMINATION_UNSUPPORTED,
    AI_JOINT_TERMINATION_PROVIDER_CHANGED,
};

enum AiJointPlannerStatsFlags
{
    // At least one retained search branch reached a categorical simulator or
    // provider boundary.  A different completed root must not hide it.
    AI_JOINT_STATS_UNSUPPORTED_SEEN          = 1 << 0,
};

enum AiJointRootFlags
{
    AI_JOINT_ROOT_VALID                      = 1 << 0,
    AI_JOINT_ROOT_DEPTH1_COMPLETE            = 1 << 1,
    AI_JOINT_ROOT_DEPTH3_COMPLETE            = 1 << 2,
    AI_JOINT_ROOT_DEPTH5_COMPLETE            = 1 << 3,
    AI_JOINT_ROOT_EXTENSION_SELECTED         = 1 << 4,
    AI_JOINT_ROOT_UNSUPPORTED                = 1 << 5,
    AI_JOINT_ROOT_PRUNED                     = 1 << 6,
    AI_JOINT_ROOT_TERMINAL                   = 1 << 7,
};

struct AiJointAtomicCandidate
{
    struct AiSimAction action;
    u16 effectiveMove;
    s16 legacyPrior;
    u16 stableKey;
    u16 legalityFlags;
    u16 readFlags;
    u16 resourceFlags;
    u16 effectKey;
    u16 rejectionFlags;
    u8 family;
    u8 flags;
    u8 koTargetMask;
    u8 reserved;
};

struct AiJointPairCandidate
{
    struct AiSimAction actions[2];
    s16 legacyPrior;
    u16 stableKey;
    u16 flags;
    u8 families[2];
    u8 sourceIndices[2];
    u8 rejectionFlags[2];
};

struct AiJointScore
{
    s32 total;
    s16 immediate;
    s16 future;
    s16 risk;
    s16 resource;
};

struct AiJointPlannerStats
{
    u16 nodesVisited;
    u16 nodeBudget;
    u16 cacheHits;
    u16 elapsedFrames;
    u16 depth1CompletedRoots;
    u16 depth3CompletedRoots;
    u16 depth5CompletedRoots;
    u16 prunedBranches;
    u8 requestedDepth;
    u8 completedDepth;
    u8 terminationReason;
    u8 flags;
};

struct AiJointPlannerLimits
{
    u16 depth3NodeBudget;
    u16 totalNodeBudget;
    u16 depth3FrameBudget;
    u16 totalFrameBudget;
    s16 extensionScoreWindow;
    u8 maxExtensionRoots;
    u8 reserved;
};

typedef u32 (*AiJointGenerateAtomicFunc)(const struct AiSimContext *context,
                                         const struct AiSimBoard *board,
                                         u8 side,
                                         enum BattlerId actor,
                                         u8 turnIndex,
                                         struct AiJointAtomicCandidate *candidates,
                                         u32 capacity,
                                         void *data);
typedef bool32 (*AiJointEvaluateBoardFunc)(const struct AiSimContext *context,
                                           const struct AiSimBoard *rootBoard,
                                           const struct AiSimBoard *board,
                                           u8 aiSide,
                                           struct AiJointScore *score,
                                           void *data);
typedef u32 (*AiJointEnumerateOutcomesFunc)(const struct AiSimContext *context,
                                            const struct AiSimBoard *board,
                                            const struct AiSimJointTurn *turn,
                                            struct AiSimOutcomeKey *outcomes,
                                            u32 capacity,
                                            void *data);
typedef enum AiSimApplyStatus (*AiJointApplyTurnFunc)(const struct AiSimContext *context,
                                                       const struct AiSimBoard *before,
                                                       const struct AiSimJointTurn *turn,
                                                       const struct AiSimOutcomeKey *outcome,
                                                       struct AiSimBoard *after,
                                                       struct AiSimTurnResult *result,
                                                       void *data);

struct AiJointPlannerRequest
{
    const struct AiSimContext *context;
    const struct AiSimBoard *rootBoard;
    const struct AiJointAtomicCandidate *rootCandidates[2];
    u8 rootCandidateCounts[2];
    u8 rootActors[2];
    struct AiSimAction confirmedPlayerActions[2];
    u8 confirmedPlayerActionCount;
    u8 aiSide;
    u8 activeAiMask;
    u8 reserved;
    u32 cacheKeySalt;
    struct AiJointPlannerLimits limits;
    AiJointGenerateAtomicFunc generateAtomic;
    AiJointEvaluateBoardFunc evaluateBoard;
    AiJointEnumerateOutcomesFunc enumerateOutcomes;
    AiJointApplyTurnFunc applyTurn;
    void *callbackData;
};

struct AiJointRootResult
{
    struct AiJointScore score;
    u32 unsupportedFlags;
    u16 nodesVisited;
    u8 pairIndex;
    u8 completedDepth;
    u8 flags;
    u8 reserved[3];
};

struct AiJointRootSummary
{
    struct AiJointPairCandidate pair;
    struct AiJointScore score;
    u32 unsupportedFlags;
    u8 completedDepth;
    u8 flags;
    u8 rank;
    u8 reserved;
};

struct AiJointPlannerResult
{
    struct AiJointPairCandidate chosenPair;
    struct AiJointScore score;
    struct AiJointPlannerStats stats;
    struct AiSimBoard chosenAfterBoard;
    u32 unsupportedFlags;
    u8 state;
    u8 chosenRank;
    u8 rootCount;
    u8 completedDepth;
};

struct AiJointSearchFrame
{
    struct AiJointPairCandidate currentMaxPair;
    struct AiJointPairCandidate currentMinPair;
    struct AiJointScore nodeBest;
    struct AiJointScore currentWorst;
    struct AiJointScore outcomeWorst;
    s64 outcomeWeightedTotal;
    s64 outcomeWeightedImmediate;
    s64 outcomeWeightedFuture;
    s64 outcomeWeightedRisk;
    s64 outcomeWeightedResource;
    u32 unsupportedFlags;
    u32 currentUnsupportedFlags;
    u32 bestUnsupportedFlags;
    u32 outcomeWeight;
    u8 state;
    u8 remainingDepth;
    u8 maxIndex;
    u8 maxCount;
    u8 minIndex;
    u8 minCount;
    u8 outcomeIndex;
    u8 outcomeCount;
    u8 supportedResponses;
    u8 supportedMaxPairs;
    u8 responseUnsupported;
    u8 maxUnsupported;
};

struct AiJointTranspositionEntry
{
    u32 hash;
    struct AiJointScore score;
    u16 signature;
    u8 remainingDepth;
    u8 flags;
};

struct AiJointPlannerWorkspace
{
    struct AiSimBoard boards[AI_SIM_BOARD_FRAMES];
    struct AiJointAtomicCandidate providerScratch[AI_JOINT_MAX_ATOMIC_PER_ACTOR];
    struct AiJointAtomicCandidate retained[2][AI_JOINT_MAX_ATOMIC_PER_ACTOR];
    struct AiJointPairCandidate pairScratch[AI_JOINT_MAX_ROOT_PAIRS];
    struct AiJointPairCandidate rootPairs[AI_JOINT_MAX_ROOT_PAIRS];
    struct AiJointRootResult rootResults[AI_JOINT_MAX_ROOT_PAIRS];
    struct AiJointTranspositionEntry transposition[AI_JOINT_TRANSPOSITION_ENTRIES];
    struct AiJointSearchFrame frames[AI_JOINT_MAX_SEARCH_DEPTH];
    struct AiSimOutcomeKey outcomes[AI_JOINT_MAX_SEARCH_DEPTH][AI_SIM_MAX_OUTCOMES];
    struct AiSimTurnResult turnResult;
};

struct AiJointPlannerJob
{
    struct AiJointPlannerRequest request;
    struct AiJointPlannerWorkspace *workspace;
    struct AiJointPlannerStats stats;
    struct AiJointAtomicCandidate forcedRejectedCandidate;
    struct AiJointScore returnedScore;
    u32 returnedUnsupportedFlags;
    u16 phaseStartNodes;
    u8 state;
    u8 phase;
    u8 phaseIndex;
    u8 rootCount;
    u8 currentRoot;
    u8 stackDepth;
    u8 searchTargetDepth;
    u8 depth3SelectionCount;
    u8 selectedDepth3[AI_JOINT_DEPTH3_SEARCH_WIDTH];
    u8 selectedExtensionCount;
    u8 currentExtension;
    u8 selectedExtensions[AI_JOINT_MAX_EXTENSION_ROOTS];
    u8 rankedRoots[AI_JOINT_MAX_ROOT_PAIRS];
    u8 hasForcedRejectedCandidate;
    u8 searchActive;
    u8 childReturned;
    u8 reserved[2];
};

STATIC_ASSERT(sizeof(struct AiJointAtomicCandidate) == 32, AiJointAtomicCandidateSizeChanged)
STATIC_ASSERT(sizeof(struct AiJointPairCandidate) == 36, AiJointPairCandidateSizeChanged)
STATIC_ASSERT(sizeof(struct AiJointScore) == 12, AiJointScoreSizeChanged)
STATIC_ASSERT(sizeof(struct AiJointTranspositionEntry) == 20, AiJointTranspositionEntrySizeChanged)

u32 AiJoint_RetainAtomicCandidates(const struct AiJointAtomicCandidate *input,
                                   u32 inputCount,
                                   struct AiJointAtomicCandidate *output,
                                   u32 capacity);
enum AiJointPairConflict AiJoint_GetPairConflict(const struct AiSimContext *context,
                                                  const struct AiJointAtomicCandidate *left,
                                                  const struct AiJointAtomicCandidate *right);
u32 AiJoint_BuildPairs(const struct AiSimContext *context,
                       const struct AiJointAtomicCandidate *left,
                       u32 leftCount,
                       const struct AiJointAtomicCandidate *right,
                       u32 rightCount,
                       struct AiJointPairCandidate *pairs,
                       u32 capacity);
bool32 AiJoint_EvaluateBoard(const struct AiSimContext *context,
                             const struct AiSimBoard *rootBoard,
                             const struct AiSimBoard *board,
                             u8 aiSide,
                             struct AiJointScore *score);
s32 AiJoint_CalculateBoardPressure(const struct AiSimContext *context,
                                   const struct AiSimBoard *board,
                                   u8 aiSide);
bool32 AiJoint_EvaluateBoardWithRootPressure(const struct AiSimContext *context,
                                             const struct AiSimBoard *rootBoard,
                                             const struct AiSimBoard *board,
                                             u8 aiSide,
                                             s32 rootPressure,
                                             struct AiJointScore *score);

enum AiJointPlannerState AiJointPlanner_InitJob(struct AiJointPlannerJob *job,
                                                 struct AiJointPlannerWorkspace *workspace,
                                                 const struct AiJointPlannerRequest *request);
enum AiJointPlannerState AiJointPlanner_Step(struct AiJointPlannerJob *job,
                                              u32 nodeSlice,
                                              struct AiJointPlannerResult *result);
enum AiJointPlannerState AiJointPlanner_Run(struct AiJointPlannerJob *job,
                                             struct AiJointPlannerWorkspace *workspace,
                                             const struct AiJointPlannerRequest *request,
                                             struct AiJointPlannerResult *result);
u32 AiJointPlanner_GetRootCount(const struct AiJointPlannerJob *job);
bool32 AiJointPlanner_GetRankedRoot(const struct AiJointPlannerJob *job,
                                    u32 rank,
                                    struct AiJointRootSummary *summary);
bool32 AiJointPlanner_GetForcedRejectedCandidate(const struct AiJointPlannerJob *job,
                                                  struct AiJointAtomicCandidate *candidate);

#endif // GUARD_BATTLE_AI_JOINT_PLANNER_H
