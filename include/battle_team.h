#ifndef GUARD_BATTLE_TEAM_H
#define GUARD_BATTLE_TEAM_H

#define BATTLE_TEAM_COUNT 3
#define BATTLE_TEAM_MEMBER_COUNT 6
#define BATTLE_TEAM_SLOT_NONE 0xFF

struct BattleTeamSlot
{
    u8 boxId;
    u8 boxPosition;
};

struct BattleTeamRegistry
{
    u32 magic;
    u8 version;
    u8 lastViewedTeam;
    u8 padding[2];
    struct BattleTeamSlot teams[BATTLE_TEAM_COUNT][BATTLE_TEAM_MEMBER_COUNT];
};

void BattleTeam_Reset(void);
bool32 BattleTeam_TryRegister(u8 teamId, u8 teamPosition, u8 boxId, u8 boxPosition);
void BattleTeam_ClearSlot(u8 teamId, u8 teamPosition);
bool32 BattleTeam_TryGetMember(u8 teamId, u8 teamPosition, struct BattleTeamSlot *slot);
bool32 BattleTeam_TryGetFullRoster(u8 teamId, struct BattleTeamSlot *slots);
u8 BattleTeam_GetRegisteredCount(u8 teamId);
bool32 BattleTeam_IsBoxSlotRegistered(u8 boxId, u8 boxPosition);
bool32 BattleTeam_CanRegisterBoxSlot(u8 boxId, u8 boxPosition);
u8 BattleTeam_GetLastViewedTeam(void);
void BattleTeam_SetLastViewedTeam(u8 teamId);

#endif // GUARD_BATTLE_TEAM_H
