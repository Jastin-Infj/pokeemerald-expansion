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

u8 BattleTeam_FindSourcePosition(u8 teamId, u8 boxId, u8 boxPosition)
{
    struct BattleTeamSlot slot;
    u32 teamPosition;

    if (teamId >= BATTLE_TEAM_COUNT)
        return BATTLE_TEAM_SLOT_NONE;

    for (teamPosition = 0; teamPosition < BATTLE_TEAM_MEMBER_COUNT; teamPosition++)
    {
        if (BattleTeam_TryGetMember(teamId, teamPosition, &slot)
         && slot.boxId == boxId
         && slot.boxPosition == boxPosition)
            return teamPosition;
    }

    return BATTLE_TEAM_SLOT_NONE;
}

bool32 BattleTeam_TryRegister(u8 teamId, u8 teamPosition, u8 boxId, u8 boxPosition)
{
    struct BattleTeamRegistry *registry;
    u8 existingPosition;

    EnsureRegistryInitialized();
    if (!IsTeamAndPositionValid(teamId, teamPosition)
     || !BattleTeam_CanRegisterBoxSlot(boxId, boxPosition))
        return FALSE;

    existingPosition = BattleTeam_FindSourcePosition(teamId, boxId, boxPosition);
    if (existingPosition != BATTLE_TEAM_SLOT_NONE && existingPosition != teamPosition)
        return FALSE;

    registry = GetRegistry();
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

void BattleTeam_SwapSlots(u8 teamId, u8 firstPosition, u8 secondPosition)
{
    struct BattleTeamSlot temp;
    struct BattleTeamRegistry *registry;

    EnsureRegistryInitialized();
    if (!IsTeamAndPositionValid(teamId, firstPosition)
     || !IsTeamAndPositionValid(teamId, secondPosition)
     || firstPosition == secondPosition)
        return;

    registry = GetRegistry();
    temp = registry->teams[teamId][firstPosition];
    registry->teams[teamId][firstPosition] = registry->teams[teamId][secondPosition];
    registry->teams[teamId][secondPosition] = temp;
    registry->lastViewedTeam = teamId;
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

u8 BattleTeam_GetFirstInvalidPosition(u8 teamId)
{
    u32 teamPosition;

    if (teamId >= BATTLE_TEAM_COUNT)
        return BATTLE_TEAM_SLOT_NONE;

    for (teamPosition = 0; teamPosition < BATTLE_TEAM_MEMBER_COUNT; teamPosition++)
    {
        if (!BattleTeam_TryGetMember(teamId, teamPosition, NULL))
            return teamPosition;
    }

    return BATTLE_TEAM_SLOT_NONE;
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
    return BattleTeam_GetBoxSlotTeamMask(boxId, boxPosition) != 0;
}

u8 BattleTeam_GetBoxSlotTeamMask(u8 boxId, u8 boxPosition)
{
    struct BattleTeamSlot slot;
    u32 teamId;
    u32 teamPosition;
    u8 teamMask = 0;

    for (teamId = 0; teamId < BATTLE_TEAM_COUNT; teamId++)
    {
        for (teamPosition = 0; teamPosition < BATTLE_TEAM_MEMBER_COUNT; teamPosition++)
        {
            if (BattleTeam_TryGetMember(teamId, teamPosition, &slot)
             && slot.boxId == boxId
             && slot.boxPosition == boxPosition)
            {
                teamMask |= 1 << teamId;
                break;
            }
        }
    }
    return teamMask;
}

void BattleTeam_BuildBoxSlotTeamMasks(u8 boxId, u8 *teamMasks)
{
    struct BattleTeamRegistry *registry;
    u32 teamId;
    u32 teamPosition;

    if (teamMasks == NULL)
        return;

    memset(teamMasks, 0, IN_BOX_COUNT);
    EnsureRegistryInitialized();
    registry = GetRegistry();
    for (teamId = 0; teamId < BATTLE_TEAM_COUNT; teamId++)
    {
        for (teamPosition = 0; teamPosition < BATTLE_TEAM_MEMBER_COUNT; teamPosition++)
        {
            struct BattleTeamSlot *slot = &registry->teams[teamId][teamPosition];

            if (!BattleTeam_CanRegisterBoxSlot(slot->boxId, slot->boxPosition))
            {
                ClearSlot(slot);
            }
            else if (slot->boxId == boxId)
            {
                teamMasks[slot->boxPosition] |= 1 << teamId;
            }
        }
    }
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
