#ifndef GUARD_DEBUG_H
#define GUARD_DEBUG_H

struct BoxNpcPartyPoolConfig;

void Debug_ShowMainMenu(void);
extern const u8 Debug_FlagsAndVarNotSetBattleConfigMessage[];
const u8 *GetWeatherName(u32 weatherId);
const struct Trainer* GetDebugAiTrainer(void);
bool32 Debug_StartPreparedBoxNpcBattle(const struct BoxNpcPartyPoolConfig *config);

void DebugNative_GetAbilityNames(void);
void DebugNative_Party_SetFriendship(void);

extern EWRAM_DATA bool8 gIsDebugBattle;
extern EWRAM_DATA u64 gDebugAIFlags;
extern EWRAM_DATA u8 gDebugGimmickAccessFlags;

#endif // GUARD_DEBUG_H
