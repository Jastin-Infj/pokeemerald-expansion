#ifndef GUARD_SM_BATTLE_MENU_H
#define GUARD_SM_BATTLE_MENU_H

#include "global.h"
#include "window.h"

bool32 SmBattleMenuEnabled(void);
void SmBattleMenuTemplates(struct WindowTemplate *templates);
void SmBattleMenuSetBattler(u32 battler);
bool32 SmBattleMenuPrint(const u8 *text, u32 windowId);
bool32 SmBattleMenuCursor(u32 position, bool32 move, bool32 selected);
void SmBattleMenuMessageBackground(u32 windowId);
u32 SmBattleMoveType(u32 battler, u32 moveIndex);

#endif
