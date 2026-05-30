#ifndef GUARD_CHAMPIONS_RUN_SESSION_H
#define GUARD_CHAMPIONS_RUN_SESSION_H

enum ChampionsRunStatus
{
    CHAMPIONS_RUN_STATUS_NONE,
    CHAMPIONS_RUN_STATUS_ENTRY_SAVED,
    CHAMPIONS_RUN_STATUS_ACTIVE,
    CHAMPIONS_RUN_STATUS_PAUSED,
    CHAMPIONS_RUN_STATUS_WON,
    CHAMPIONS_RUN_STATUS_LOST,
    CHAMPIONS_RUN_STATUS_RETIRED,
    CHAMPIONS_RUN_STATUS_RECOVERING,
};

enum ChampionsRunCheckpoint
{
    CHAMPIONS_RUN_CHECKPOINT_NONE,
    CHAMPIONS_RUN_CHECKPOINT_ENTRY,
    CHAMPIONS_RUN_CHECKPOINT_DEBUG,
    CHAMPIONS_RUN_CHECKPOINT_MANUAL,
};

bool32 ChampionsRun_IsActive(void);
bool32 ChampionsRun_ShouldUseTemporarySave(u8 saveType);
bool32 ChampionsRun_BeginEntry(void);
u8 ChampionsRun_BeginEntryReport(void);
u8 ChampionsRun_SaveCheckpoint(u8 checkpointKind);
bool32 ChampionsRun_EndByBattleOutcome(u8 battleOutcome);
void ChampionsRun_RestoreNormalState(u8 outcome);
u8 ChampionsRun_RestoreNormalStateAndSave(u8 outcome);
u8 ChampionsRun_RetireAndSave(void);
bool32 ChampionsRun_CompleteClear(void);
u8 ChampionsRun_CompleteClearAndSave(void);
void ChampionsRun_HandleBootRecovery(void);
void ChampionsRun_CanUseNormalPc(void);
void ChampionsRun_ReloadMapAfterRestore(void);
bool32 ChampionsRun_ShouldBlockBagUse(void);
bool32 ChampionsRun_ShouldBlockHeldItemChanges(void);
bool32 ChampionsRun_ShouldSuppressExp(void);

void ChampionsRun_DebugBegin(void);
void ChampionsRun_DebugGiveRunMon(void);
void ChampionsRun_DebugSaveCheckpoint(void);
void ChampionsRun_DebugRestoreNormal(void);
void ChampionsRun_DebugPrepareLoseTest(void);
void ChampionsRun_DebugClear(void);

#endif // GUARD_CHAMPIONS_RUN_SESSION_H
