#ifndef GUARD_PREBATTLE_TEAM_VIEWER_H
#define GUARD_PREBATTLE_TEAM_VIEWER_H

#include "global.h"
#include "main.h"

bool32 PreBattleTeamViewer_Begin(u8 selectedCount, MainCallback callback);
bool32 PreBattleTeamViewer_Reopen(u8 selectedCount, MainCallback callback);
bool32 PreBattleTeamViewer_LoadCachedOpponentParty(void);
bool32 PreBattleTeamViewer_TryOpenInBattle(u32 battler);
void PreBattleTeamViewer_RestoreBattleCallback1(void);
void PreBattleTeamViewer_Clear(void);

#endif // GUARD_PREBATTLE_TEAM_VIEWER_H
