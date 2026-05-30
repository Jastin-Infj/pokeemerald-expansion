#ifndef GUARD_TRAINER_BATTLE_SELECTION_H
#define GUARD_TRAINER_BATTLE_SELECTION_H

#include "global.h"
#include "main.h"

struct Pokemon;

typedef void (*TrainerBattleSelectionPartyMonFunc)(struct Pokemon *mon, void *context);

bool32 TrainerBattleSelection_Begin(u8 selectedCount, MainCallback callback, MainCallback backCallback);
void TrainerBattleSelection_StartBattleFromSelection(void);
void TrainerBattleSelection_RestoreIfActive(void);
bool32 TrainerBattleSelection_ForEachOriginalPartyMon(TrainerBattleSelectionPartyMonFunc func, void *context);
bool32 TrainerBattleSelection_ForEachSelectedBattleMon(TrainerBattleSelectionPartyMonFunc func, void *context);

#endif // GUARD_TRAINER_BATTLE_SELECTION_H
