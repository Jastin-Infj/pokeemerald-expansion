#include "global.h"
#include "battle.h"
#include "battle_team.h"
#include "box_npc_party_pool.h"
#include "malloc.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "test/test.h"
#include "constants/items.h"

static const enum Species sBattleTeamTestSpecies[BATTLE_TEAM_MEMBER_COUNT] =
{
    SPECIES_BULBASAUR,
    SPECIES_CHARMANDER,
    SPECIES_SQUIRTLE,
    SPECIES_PIKACHU,
    SPECIES_EEVEE,
    SPECIES_SNORLAX,
};

static const struct BattleTeamSlot sBattleTeamTestSources[BATTLE_TEAM_MEMBER_COUNT] =
{
    {0, 0},
    {1, 3},
    {2, 6},
    {3, 9},
    {4, 12},
    {5, 15},
};

static void PutBattleTeamTestMon(u8 boxId, u8 boxPosition, enum Species species, enum Item item)
{
    struct Pokemon mon;

    CreateMon(&mon, species, 50, 0, OTID_STRUCT_PRESET(0));
    SetMonData(&mon, MON_DATA_HELD_ITEM, &item);
    SetBoxMonAt(boxId, boxPosition, &mon.box);
}

static void RegisterBattleTeamTestRoster(u8 teamId, u8 count)
{
    u32 i;

    for (i = 0; i < count; i++)
    {
        PutBattleTeamTestMon(sBattleTeamTestSources[i].boxId,
                             sBattleTeamTestSources[i].boxPosition,
                             sBattleTeamTestSpecies[i],
                             ITEM_ORAN_BERRY);
        EXPECT(BattleTeam_TryRegister(teamId, i,
                                      sBattleTeamTestSources[i].boxId,
                                      sBattleTeamTestSources[i].boxPosition));
    }
}

TEST("Battle Team slots reference Box Pokemon and reject team-local duplicates")
{
    struct BattleTeamSlot slot;
    u8 teamMasks[IN_BOX_COUNT];

    ResetPokemonStorageSystem();
    PutBattleTeamTestMon(0, 0, SPECIES_BULBASAUR, ITEM_ORAN_BERRY);

    EXPECT(BattleTeam_TryRegister(0, 0, 0, 0));
    EXPECT(BattleTeam_TryRegister(1, 0, 0, 0));
    EXPECT(BattleTeam_TryGetMember(0, 0, &slot));
    EXPECT_EQ(slot.boxId, 0);
    EXPECT_EQ(slot.boxPosition, 0);
    EXPECT(BattleTeam_TryGetMember(1, 0, &slot));

    EXPECT(BattleTeam_TryRegister(0, 1, 0, 0));
    EXPECT(!BattleTeam_TryGetMember(0, 0, NULL));
    EXPECT(BattleTeam_TryGetMember(0, 1, &slot));
    EXPECT(BattleTeam_TryGetMember(1, 0, NULL));
    EXPECT_EQ(BattleTeam_GetRegisteredCount(0), 1);
    EXPECT(BattleTeam_IsBoxSlotRegistered(0, 0));
    EXPECT_EQ(BattleTeam_GetBoxSlotTeamMask(0, 0), (1 << 0) | (1 << 1));
    BattleTeam_BuildBoxSlotTeamMasks(0, teamMasks);
    EXPECT_EQ(teamMasks[0], (1 << 0) | (1 << 1));

    {
        enum Item item = ITEM_SITRUS_BERRY;

        SetBoxMonDataAt(0, 0, MON_DATA_HELD_ITEM, &item);
        EXPECT_EQ(GetBoxMonDataAt(slot.boxId, slot.boxPosition, MON_DATA_HELD_ITEM), ITEM_SITRUS_BERRY);
    }

    BattleTeam_ClearSlot(0, 0);
    EXPECT(BattleTeam_IsBoxSlotRegistered(0, 0));
    EXPECT_EQ(BattleTeam_GetBoxSlotTeamMask(0, 0), 1 << 1);
    BattleTeam_BuildBoxSlotTeamMasks(0, teamMasks);
    EXPECT_EQ(teamMasks[0], 1 << 1);
    BattleTeam_ClearSlot(1, 0);
    EXPECT(!BattleTeam_IsBoxSlotRegistered(0, 0));
    EXPECT_EQ(BattleTeam_GetBoxSlotTeamMask(0, 0), 0);
    BattleTeam_BuildBoxSlotTeamMasks(0, teamMasks);
    EXPECT_EQ(teamMasks[0], 0);
}

TEST("Battle Team slots can be reordered without changing their Box sources")
{
    struct BattleTeamSlot slot;

    ResetPokemonStorageSystem();
    PutBattleTeamTestMon(2, 4, SPECIES_TREECKO, ITEM_NONE);
    PutBattleTeamTestMon(7, 9, SPECIES_TORCHIC, ITEM_NONE);
    EXPECT(BattleTeam_TryRegister(2, 1, 2, 4));
    EXPECT(BattleTeam_TryRegister(2, 4, 7, 9));

    BattleTeam_SwapSlots(2, 1, 4);
    EXPECT(BattleTeam_TryGetMember(2, 1, &slot));
    EXPECT_EQ(slot.boxId, 7);
    EXPECT_EQ(slot.boxPosition, 9);
    EXPECT(BattleTeam_TryGetMember(2, 4, &slot));
    EXPECT_EQ(slot.boxId, 2);
    EXPECT_EQ(slot.boxPosition, 4);
    EXPECT_EQ(BattleTeam_GetLastViewedTeam(), 2);
}

TEST("Battle Team registration rejects invalid sources and clears stale references")
{
    bool8 isEgg = TRUE;

    ResetPokemonStorageSystem();
    EXPECT(!BattleTeam_TryRegister(0, 0, 0, 0));
    EXPECT(!BattleTeam_TryRegister(0, 0, BATTLE_TEAM_SLOT_NONE, BATTLE_TEAM_SLOT_NONE));

    PutBattleTeamTestMon(0, 0, SPECIES_TOGEPI, ITEM_NONE);
    SetBoxMonDataAt(0, 0, MON_DATA_IS_EGG, &isEgg);
    EXPECT(!BattleTeam_TryRegister(0, 0, 0, 0));

    PutBattleTeamTestMon(1, 4, SPECIES_CHARMANDER, ITEM_NONE);
    EXPECT(BattleTeam_TryRegister(0, 0, 1, 4));
    ZeroBoxMonAt(1, 4);
    EXPECT(!BattleTeam_TryGetMember(0, 0, NULL));
    EXPECT(!BattleTeam_IsBoxSlotRegistered(1, 4));
}

TEST("Battle Team registered roster builds a healed NPC party without mutating Box sources")
{
    const struct BoxNpcPartyPoolConfig config =
    {
        .poolMode = BOX_NPC_POOL_REGISTERED_BATTLE_TEAM,
        .battleFormat = BOX_NPC_BATTLE_SINGLE_3,
        .memberMode = BOX_NPC_BATTLE_MEMBERS_FIRST_N,
        .gimmickPolicy = BOX_NPC_GIMMICK_NATURAL,
        .battleTeamId = 0,
        .aiFlags = 0,
    };
    struct BoxNpcPartyPoolResult result;
    u16 hpLost = 20;
    u32 i;

    ResetPokemonStorageSystem();
    RegisterBattleTeamTestRoster(0, BATTLE_TEAM_MEMBER_COUNT);
    EXPECT_EQ(BattleTeam_GetRegisteredCount(0), BATTLE_TEAM_MEMBER_COUNT);
    EXPECT_EQ(BattleTeam_GetFirstInvalidPosition(0), BATTLE_TEAM_SLOT_NONE);
    SetBoxMonDataAt(sBattleTeamTestSources[0].boxId, sBattleTeamTestSources[0].boxPosition, MON_DATA_HP_LOST, &hpLost);

    EXPECT(BoxNpcPartyPool_TryBuildOpponentParty(&config, &result));
    EXPECT_EQ(result.candidateCount, BATTLE_TEAM_MEMBER_COUNT);
    EXPECT_EQ(result.battleCount, BOX_NPC_SINGLE_BATTLE_SIZE);
    for (i = 0; i < BOX_NPC_SINGLE_BATTLE_SIZE; i++)
    {
        EXPECT_EQ(result.finalSources[i].boxId, sBattleTeamTestSources[i].boxId);
        EXPECT_EQ(result.finalSources[i].boxPosition, sBattleTeamTestSources[i].boxPosition);
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][i], MON_DATA_SPECIES), sBattleTeamTestSpecies[i]);
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][i], MON_DATA_HP),
                  GetMonData(&gParties[B_TRAINER_OPPONENT_A][i], MON_DATA_MAX_HP));
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][i], MON_DATA_HELD_ITEM), ITEM_ORAN_BERRY);
    }

    EXPECT_EQ(GetBoxMonDataAt(sBattleTeamTestSources[0].boxId, sBattleTeamTestSources[0].boxPosition, MON_DATA_HP_LOST), hpLost);
    EXPECT_EQ(GetBoxMonDataAt(sBattleTeamTestSources[0].boxId, sBattleTeamTestSources[0].boxPosition, MON_DATA_HELD_ITEM), ITEM_ORAN_BERRY);
    BoxNpcPartyPool_ClearPendingBattleInitPolicy();
}

TEST("Battle Team Box NPC battles stage exactly three or four player mons and restore all six")
{
    struct Pokemon originalParty[PARTY_SIZE];
    u8 originalCount;
    u32 i;

    BoxNpcPartyPool_RestorePlayerParty();
    ZeroPlayerPartyMons();
    for (i = 0; i < PARTY_SIZE; i++)
        CreateMonWithIVs(&gParties[B_TRAINER_PLAYER][i], sBattleTeamTestSpecies[i], 40 + i, 0, OTID_STRUCT_PRESET(0), 0);
    originalCount = CalculatePlayerPartyCount();
    memcpy(originalParty, gParties[B_TRAINER_PLAYER], sizeof(originalParty));
    // Storage screens can leave the cached count stale; staging must preserve
    // the logical full-party count derived from the actual party data.
    gPartiesCount[B_TRAINER_PLAYER] = 0;

    EXPECT_EQ(CountPartyAliveNonEggMonsExcept(PARTY_SIZE), PARTY_SIZE);
    EXPECT(BoxNpcPartyPool_TryStagePlayerParty(BOX_NPC_SINGLE_BATTLE_SIZE));
    EXPECT(BoxNpcPartyPool_IsPlayerPartyStaged());
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], BOX_NPC_SINGLE_BATTLE_SIZE);
    for (i = 0; i < BOX_NPC_SINGLE_BATTLE_SIZE; i++)
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_SPECIES), sBattleTeamTestSpecies[i]);
    for (; i < PARTY_SIZE; i++)
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_SPECIES), SPECIES_NONE);
    ZeroMonData(&gParties[B_TRAINER_PLAYER][0]);
    EXPECT(BoxNpcPartyPool_RestorePlayerParty());
    EXPECT(!BoxNpcPartyPool_IsPlayerPartyStaged());
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], originalCount);
    EXPECT(memcmp(originalParty, gParties[B_TRAINER_PLAYER], sizeof(originalParty)) == 0);

    EXPECT(BoxNpcPartyPool_TryStagePlayerParty(BOX_NPC_DOUBLE_BATTLE_SIZE));
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], BOX_NPC_DOUBLE_BATTLE_SIZE);
    for (i = 0; i < BOX_NPC_DOUBLE_BATTLE_SIZE; i++)
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_SPECIES), sBattleTeamTestSpecies[i]);
    for (; i < PARTY_SIZE; i++)
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_SPECIES), SPECIES_NONE);
    ZeroMonData(&gParties[B_TRAINER_PLAYER][3]);
    EXPECT(BoxNpcPartyPool_RestorePlayerParty());
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], originalCount);
    EXPECT(memcmp(originalParty, gParties[B_TRAINER_PLAYER], sizeof(originalParty)) == 0);
}

TEST("Battle Team registered roster builds four unique random members for a Double battle")
{
    const struct BoxNpcPartyPoolConfig config =
    {
        .poolMode = BOX_NPC_POOL_REGISTERED_BATTLE_TEAM,
        .battleFormat = BOX_NPC_BATTLE_DOUBLE_4,
        .memberMode = BOX_NPC_BATTLE_MEMBERS_RANDOM_N,
        .gimmickPolicy = BOX_NPC_GIMMICK_NATURAL,
        .battleTeamId = 1,
        .aiFlags = 0,
    };
    struct BoxNpcPartyPoolResult result;
    bool8 selected[BATTLE_TEAM_MEMBER_COUNT] = {FALSE};
    u32 i;

    ResetPokemonStorageSystem();
    RegisterBattleTeamTestRoster(1, BATTLE_TEAM_MEMBER_COUNT);
    EXPECT(BoxNpcPartyPool_TryBuildOpponentParty(&config, &result));
    EXPECT_EQ(result.battleCount, BOX_NPC_DOUBLE_BATTLE_SIZE);
    for (i = 0; i < result.battleCount; i++)
    {
        u32 candidate;

        for (candidate = 0; candidate < result.candidateCount; candidate++)
        {
            if (result.finalSources[i].boxId == result.candidateSources[candidate].boxId
             && result.finalSources[i].boxPosition == result.candidateSources[candidate].boxPosition)
                break;
        }
        EXPECT(candidate < result.candidateCount);
        EXPECT(!selected[candidate]);
        selected[candidate] = TRUE;
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][i], MON_DATA_SPECIES),
                  GetBoxMonDataAt(result.finalSources[i].boxId, result.finalSources[i].boxPosition, MON_DATA_SPECIES));
    }
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][BOX_NPC_DOUBLE_BATTLE_SIZE], MON_DATA_SPECIES), SPECIES_NONE);
    BoxNpcPartyPool_ClearPendingBattleInitPolicy();
}

TEST("Battle Team preview rejects incomplete teams and applies or cancels gimmick policy")
{
    struct BoxNpcPartyPoolConfig config =
    {
        .poolMode = BOX_NPC_POOL_REGISTERED_BATTLE_TEAM,
        .battleFormat = BOX_NPC_BATTLE_SINGLE_3,
        .memberMode = BOX_NPC_BATTLE_MEMBERS_FIRST_N,
        .gimmickPolicy = BOX_NPC_GIMMICK_ALLOW_TERA_DYNAMAX_ALL_FINAL_MEMBERS,
        .battleTeamId = 2,
        .aiFlags = 0,
    };
    u32 teraBits;
    u32 dynamaxBits;

    ResetPokemonStorageSystem();
    RegisterBattleTeamTestRoster(2, BATTLE_TEAM_MEMBER_COUNT - 1);
    EXPECT(!BoxNpcPartyPool_TryBuildOpponentParty(&config, NULL));
    EXPECT_EQ(BoxNpcPartyPool_GetLastError(), BOX_NPC_PARTY_POOL_ERROR_BATTLE_TEAM_INCOMPLETE);

    PutBattleTeamTestMon(sBattleTeamTestSources[5].boxId,
                         sBattleTeamTestSources[5].boxPosition,
                         sBattleTeamTestSpecies[5],
                         ITEM_ORAN_BERRY);
    EXPECT(BattleTeam_TryRegister(2, 5,
                                  sBattleTeamTestSources[5].boxId,
                                  sBattleTeamTestSources[5].boxPosition));
    EXPECT(BoxNpcPartyPool_TryBuildOpponentParty(&config, NULL));
    EXPECT(gBattleStruct == NULL);
    gBattleStruct = AllocZeroed(sizeof(*gBattleStruct));
    EXPECT(gBattleStruct != NULL);
    BoxNpcPartyPool_ApplyPendingBattleInitPolicy();
    teraBits = gBattleStruct->opponentMonCanTera;
    dynamaxBits = gBattleStruct->opponentMonCanDynamax;
    FREE_AND_SET_NULL(gBattleStruct);
    EXPECT_EQ(teraBits, (1 << BOX_NPC_SINGLE_BATTLE_SIZE) - 1);
    EXPECT_EQ(dynamaxBits, (1 << BOX_NPC_SINGLE_BATTLE_SIZE) - 1);

    EXPECT(BoxNpcPartyPool_TryBuildOpponentParty(&config, NULL));
    BoxNpcPartyPool_ClearPendingBattleInitPolicy();
    gBattleStruct = AllocZeroed(sizeof(*gBattleStruct));
    EXPECT(gBattleStruct != NULL);
    BoxNpcPartyPool_ApplyPendingBattleInitPolicy();
    teraBits = gBattleStruct->opponentMonCanTera;
    dynamaxBits = gBattleStruct->opponentMonCanDynamax;
    FREE_AND_SET_NULL(gBattleStruct);
    EXPECT_EQ(teraBits, 0);
    EXPECT_EQ(dynamaxBits, 0);

    config.gimmickPolicy = BOX_NPC_GIMMICK_NATURAL;
    EXPECT(BoxNpcPartyPool_TryBuildOpponentParty(&config, NULL));
    gBattleStruct = AllocZeroed(sizeof(*gBattleStruct));
    EXPECT(gBattleStruct != NULL);
    BoxNpcPartyPool_ApplyPendingBattleInitPolicy();
    teraBits = gBattleStruct->opponentMonCanTera;
    dynamaxBits = gBattleStruct->opponentMonCanDynamax;
    FREE_AND_SET_NULL(gBattleStruct);
    EXPECT_EQ(teraBits, 0);
    EXPECT_EQ(dynamaxBits, 0);
}
