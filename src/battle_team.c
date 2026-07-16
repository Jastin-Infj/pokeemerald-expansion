#include "global.h"
#include "battle_team.h"
#include "pokemon_storage_system.h"

#define BATTLE_TEAM_REGISTRY_MAGIC 0x4D544142 // "BATM"
#define BATTLE_TEAM_REGISTRY_VERSION 1

static struct BattleTeamRegistry *GetRegistry(void)
{
    return &gSaveBlock3Ptr->battleTeamRegistry;
}

static bool32 IsTeamAndPositionValid(u8 teamId, u8 teamPosition)
{
    return teamId < BATTLE_TEAM_COUNT && teamPosition < BATTLE_TEAM_MEMBER_COUNT;
}

static void ClearSlot(struct BattleTeamSlot *slot)
{
    slot->boxId = BATTLE_TEAM_SLOT_NONE;
    slot->boxPosition = BATTLE_TEAM_SLOT_NONE;
}

void BattleTeam_Reset(void)
{
    struct BattleTeamRegistry *registry = GetRegistry();
    u32 teamId;
    u32 teamPosition;

    memset(registry, 0, sizeof(*registry));
    registry->magic = BATTLE_TEAM_REGISTRY_MAGIC;
    registry->version = BATTLE_TEAM_REGISTRY_VERSION;

    for (teamId = 0; teamId < BATTLE_TEAM_COUNT; teamId++)
    {
        for (teamPosition = 0; teamPosition < BATTLE_TEAM_MEMBER_COUNT; teamPosition++)
            ClearSlot(&registry->teams[teamId][teamPosition]);
    }
}

static void EnsureRegistryInitialized(void)
{
    struct BattleTeamRegistry *registry = GetRegistry();

    if (registry->magic != BATTLE_TEAM_REGISTRY_MAGIC
     || registry->version != BATTLE_TEAM_REGISTRY_VERSION
     || registry->lastViewedTeam >= BATTLE_TEAM_COUNT)
        BattleTeam_Reset();
}

bool32 BattleTeam_CanRegisterBoxSlot(u8 boxId, u8 boxPosition)
{
    return gPokemonStoragePtr != NULL
        && boxId < TOTAL_BOXES_COUNT
        && boxPosition < IN_BOX_COUNT
        && CheckBoxMonSanityAt(boxId, boxPosition);
}

bool32 BattleTeam_TryGetMember(u8 teamId, u8 teamPosition, struct BattleTeamSlot *slot)
{
    struct BattleTeamSlot *savedSlot;

    EnsureRegistryInitialized();
    if (!IsTeamAndPositionValid(teamId, teamPosition))
        return FALSE;

    savedSlot = &GetRegistry()->teams[teamId][teamPosition];
    if (!BattleTeam_CanRegisterBoxSlot(savedSlot->boxId, savedSlot->boxPosition))
    {
        ClearSlot(savedSlot);
        return FALSE;
    }

    if (slot != NULL)
        *slot = *savedSlot;
    return TRUE;
}

bool32 BattleTeam_TryRegister(u8 teamId, u8 teamPosition, u8 boxId, u8 boxPosition)
{
    struct BattleTeamRegistry *registry;
    u32 i;

    EnsureRegistryInitialized();
    if (!IsTeamAndPositionValid(teamId, teamPosition)
     || !BattleTeam_CanRegisterBoxSlot(boxId, boxPosition))
        return FALSE;

    registry = GetRegistry();
    for (i = 0; i < BATTLE_TEAM_MEMBER_COUNT; i++)
    {
        struct BattleTeamSlot *slot = &registry->teams[teamId][i];

        if (i != teamPosition && slot->boxId == boxId && slot->boxPosition == boxPosition)
            ClearSlot(slot);
    }

    registry->teams[teamId][teamPosition].boxId = boxId;
    registry->teams[teamId][teamPosition].boxPosition = boxPosition;
    registry->lastViewedTeam = teamId;
    return TRUE;
}

void BattleTeam_ClearSlot(u8 teamId, u8 teamPosition)
{
    EnsureRegistryInitialized();
    if (IsTeamAndPositionValid(teamId, teamPosition))
        ClearSlot(&GetRegistry()->teams[teamId][teamPosition]);
}

bool32 BattleTeam_TryGetFullRoster(u8 teamId, struct BattleTeamSlot *slots)
{
    u32 i;

    if (slots == NULL || teamId >= BATTLE_TEAM_COUNT)
        return FALSE;

    for (i = 0; i < BATTLE_TEAM_MEMBER_COUNT; i++)
    {
        if (!BattleTeam_TryGetMember(teamId, i, &slots[i]))
            return FALSE;
    }
    return TRUE;
}

u8 BattleTeam_GetRegisteredCount(u8 teamId)
{
    u32 i;
    u8 count = 0;

    for (i = 0; i < BATTLE_TEAM_MEMBER_COUNT; i++)
    {
        if (BattleTeam_TryGetMember(teamId, i, NULL))
            count++;
    }
    return count;
}

bool32 BattleTeam_IsBoxSlotRegistered(u8 boxId, u8 boxPosition)
{
    struct BattleTeamSlot slot;
    u32 teamId;
    u32 teamPosition;

    for (teamId = 0; teamId < BATTLE_TEAM_COUNT; teamId++)
    {
        for (teamPosition = 0; teamPosition < BATTLE_TEAM_MEMBER_COUNT; teamPosition++)
        {
            if (BattleTeam_TryGetMember(teamId, teamPosition, &slot)
             && slot.boxId == boxId
             && slot.boxPosition == boxPosition)
                return TRUE;
        }
    }
    return FALSE;
}

u8 BattleTeam_GetLastViewedTeam(void)
{
    EnsureRegistryInitialized();
    return GetRegistry()->lastViewedTeam;
}

void BattleTeam_SetLastViewedTeam(u8 teamId)
{
    EnsureRegistryInitialized();
    if (teamId < BATTLE_TEAM_COUNT)
        GetRegistry()->lastViewedTeam = teamId;
}
