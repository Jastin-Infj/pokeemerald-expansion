#ifndef GUARD_TRAINER_BATTLE_SELECTION_H
#define GUARD_TRAINER_BATTLE_SELECTION_H

#include "global.h"
#include "main.h"

bool32 TrainerBattleSelection_Begin(u8 selectedCount, MainCallback callback);
void TrainerBattleSelection_StartBattleFromSelection(void);
void TrainerBattleSelection_RestoreIfActive(void);

#endif // GUARD_TRAINER_BATTLE_SELECTION_H
