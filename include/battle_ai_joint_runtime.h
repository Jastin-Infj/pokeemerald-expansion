#ifndef GUARD_BATTLE_AI_JOINT_RUNTIME_H
#define GUARD_BATTLE_AI_JOINT_RUNTIME_H

#include "global.h"

enum AiJointRuntimeStatus
{
    AI_JOINT_RUNTIME_NOT_ELIGIBLE,
    AI_JOINT_RUNTIME_PENDING,
    AI_JOINT_RUNTIME_READY,
    AI_JOINT_RUNTIME_FALLBACK,
};

// The joint planner is driven from the opponent controller's choose-action
// callback.  PENDING deliberately leaves the controller command open so the
// search can consume one bounded slice on a later frame.
void BattleAiJointRuntime_Reset(void);
// Once every controller has committed its command, the copied battle globals
// and trace own all data needed for execution. Release the search arena before
// move animations compete for the general-purpose EWRAM heap.
void BattleAiJointRuntime_ReleaseData(void);
enum AiJointRuntimeStatus BattleAiJointRuntime_Prepare(enum BattlerId battler);
bool32 BattleAiJointRuntime_ApplyAction(enum BattlerId battler);
void BattleAiJointRuntime_RestoreTraceDecision(enum BattlerId battler);

// Move selection is a pure replay of the action committed above.  No legacy
// scorer or board search is allowed to run from the choose-move callback.
bool32 BattleAiJointRuntime_ReuseMove(enum BattlerId battler);

#if TESTING
struct AiSimContext;
struct AiSimBoard;

u32 Test_BattleAiJointRuntime_GetBuildCount(void);
u32 Test_BattleAiJointRuntime_GetStepCount(void);
u16 Test_BattleAiJointRuntime_GetPlanId(void);
bool32 Test_BattleAiJointRuntime_IsAllocated(void);
u32 Test_BattleAiJointRuntime_GetFutureGimmickMask(const struct AiSimContext *context,
                                                  const struct AiSimBoard *board,
                                                  enum BattlerId actor,
                                                  bool32 *hasUnresolved);
bool32 Test_BattleAiJointRuntime_IsFutureGimmickAvailable(const struct AiSimContext *context,
                                                         const struct AiSimBoard *board,
                                                         u32 rosterIndex,
                                                         u32 gimmick);
void Test_BattleAiJointRuntime_SetLimits(u16 depth3Nodes, u16 totalNodes,
                                        u16 depth3Frames, u16 totalFrames);
void Test_BattleAiJointRuntime_ClearLimits(void);
#endif

#endif // GUARD_BATTLE_AI_JOINT_RUNTIME_H
