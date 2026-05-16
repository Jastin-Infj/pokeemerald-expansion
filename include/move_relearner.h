#ifndef GUARD_MOVE_RELEARNER_H
#define GUARD_MOVE_RELEARNER_H

#include "constants/move_relearner.h"

void TeachMoveRelearnerMove(void);
void MoveRelearnerShowHideHearts(s32 move);
void MoveRelearnerShowHideCategoryIcon(s32);
s32 MoveRelearnerGetMoveForMenuId(s32 menuId);
const u8 *MoveRelearnerGetSourceLabelForMenuId(s32 menuId);
bool32 MoveRelearnerUseSourceLabels(void);
bool32 MoveRelearnerUsePageScroll(void);
void CB2_InitLearnMove(void);
bool32 CanBoxMonRelearnAnyMove(struct BoxPokemon *boxMon);
bool32 CanBoxMonRelearnMoves(struct BoxPokemon *boxMon, enum MoveRelearnerStates state);

extern enum MoveRelearnerStates gMoveRelearnerState;
extern enum RelearnMode gRelearnMode;

#endif //GUARD_MOVE_RELEARNER_H
