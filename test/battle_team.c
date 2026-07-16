#include "global.h"
#include "battle_team.h"
#include "box_npc_party_pool.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "test/test.h"
#include "constants/items.h"

static void PutBattleTeamTestMon(u8 boxId, u8 boxPosition, enum Species species, enum Item item)
{
    struct Pokemon mon;

    CreateMon(&mon, species, 50, 0, OTID_STRUCT_PRESET(0));
    SetMonData(&mon, MON_DATA_HELD_ITEM, &item);
    SetBoxMonAt(boxId, boxPosition, &mon.box);
}

TEST("Battle Team slots reference Box Pokemon and enforce team-local uniqueness")
{
    struct BattleTeamSlot slot;

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

    {
        enum Item item = ITEM_SITRUS_BERRY;

        SetBoxMonDataAt(0, 0, MON_DATA_HELD_ITEM, &item);
        EXPECT_EQ(GetBoxMonDataAt(slot.boxId, slot.boxPosition, MON_DATA_HELD_ITEM), ITEM_SITRUS_BERRY);
    }
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

TEST("Registered Battle Team builds a healed NPC party without mutating Box sources")
{
    static const enum Species species[BATTLE_TEAM_MEMBER_COUNT] =
    {
        SPECIES_BULBASAUR,
        SPECIES_CHARMANDER,
        SPECIES_SQUIRTLE,
        SPECIES_PIKACHU,
        SPECIES_EEVEE,
        SPECIES_SNORLAX,
    };
    static const struct BattleTeamSlot sources[BATTLE_TEAM_MEMBER_COUNT] =
    {
        {0, 0},
        {1, 3},
        {2, 6},
        {3, 9},
        {4, 12},
        {5, 15},
    };
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
    for (i = 0; i < BATTLE_TEAM_MEMBER_COUNT; i++)
    {
        PutBattleTeamTestMon(sources[i].boxId, sources[i].boxPosition, species[i], ITEM_ORAN_BERRY);
        EXPECT(BattleTeam_TryRegister(0, i, sources[i].boxId, sources[i].boxPosition));
    }
    SetBoxMonDataAt(sources[0].boxId, sources[0].boxPosition, MON_DATA_HP_LOST, &hpLost);

    EXPECT(BoxNpcPartyPool_TryBuildOpponentParty(&config, &result));
    EXPECT_EQ(result.candidateCount, BATTLE_TEAM_MEMBER_COUNT);
    EXPECT_EQ(result.battleCount, BOX_NPC_SINGLE_BATTLE_SIZE);
    for (i = 0; i < BOX_NPC_SINGLE_BATTLE_SIZE; i++)
    {
        EXPECT_EQ(result.finalSources[i].boxId, sources[i].boxId);
        EXPECT_EQ(result.finalSources[i].boxPosition, sources[i].boxPosition);
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][i], MON_DATA_SPECIES), species[i]);
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][i], MON_DATA_HP),
                  GetMonData(&gParties[B_TRAINER_OPPONENT_A][i], MON_DATA_MAX_HP));
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][i], MON_DATA_HELD_ITEM), ITEM_ORAN_BERRY);
    }

    EXPECT_EQ(GetBoxMonDataAt(sources[0].boxId, sources[0].boxPosition, MON_DATA_HP_LOST), hpLost);
    EXPECT_EQ(GetBoxMonDataAt(sources[0].boxId, sources[0].boxPosition, MON_DATA_HELD_ITEM), ITEM_ORAN_BERRY);
    BoxNpcPartyPool_ClearPendingBattleInitPolicy();
}
