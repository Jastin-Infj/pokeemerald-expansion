#include "global.h"
#include "champions_run_session.h"
#include "coins.h"
#include "event_data.h"
#include "item.h"
#include "load_save.h"
#include "money.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "save.h"
#include "config/save.h"
#include "test/test.h"
#include "constants/battle.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/species.h"

static void InitNormalStateForChampionsRunTest(void)
{
    ZeroPlayerPartyMons();
    gPlayerPartyCount = 0;
    ClearBag();

    CreateMon(&gPlayerParty[0], SPECIES_PIKACHU, 50, 0, OTID_STRUCT_PRESET(0));
    CreateMon(&gPlayerParty[1], SPECIES_DRAGONITE, 50, 1, OTID_STRUCT_PRESET(1));
    CalculatePlayerPartyCount();

    AddBagItem(ITEM_POTION, 3);
    SetMoney(&gSaveBlock1Ptr->money, 12345);
    SetCoins(123);
    gSaveBlock1Ptr->registeredItem = ITEM_POTION;
}

TEST("Champions run entry snapshots normal state and clears live challenge state")
{
    InitNormalStateForChampionsRunTest();

    EXPECT_EQ(ChampionsRun_BeginEntry(), TRUE);
    EXPECT_EQ(ChampionsRun_IsActive(), TRUE);
    EXPECT_EQ(gPlayerPartyCount, 0);
    EXPECT_EQ(CalculatePlayerPartyCount(), 0);
    EXPECT_EQ(CheckBagHasItem(ITEM_POTION, 1), FALSE);
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 0);
    EXPECT_EQ(GetCoins(), 0);

    ChampionsRun_RestoreNormalState(CHAMPIONS_RUN_STATUS_RETIRED);

    EXPECT_EQ(ChampionsRun_IsActive(), FALSE);
    EXPECT_EQ(gPlayerPartyCount, 2);
    EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_SPECIES), SPECIES_PIKACHU);
    EXPECT_EQ(GetMonData(&gPlayerParty[1], MON_DATA_SPECIES), SPECIES_DRAGONITE);
    EXPECT_EQ(CheckBagHasItem(ITEM_POTION, 3), TRUE);
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 12345);
    EXPECT_EQ(GetCoins(), 123);
    EXPECT_EQ(gSaveBlock1Ptr->registeredItem, ITEM_POTION);
}

TEST("Champions run normal snapshot survives encryption key changes")
{
    InitNormalStateForChampionsRunTest();

    EXPECT_EQ(ChampionsRun_BeginEntry(), TRUE);

    gSaveBlock2Ptr->encryptionKey = 0x12345678;
    ChampionsRun_RestoreNormalState(CHAMPIONS_RUN_STATUS_RETIRED);

    EXPECT_EQ(CheckBagHasItem(ITEM_POTION, 3), TRUE);
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 12345);
    EXPECT_EQ(GetCoins(), 123);
}

TEST("Champions run maps normal save requests to temporary report saves while active")
{
    EXPECT_EQ(ChampionsRun_ShouldUseTemporarySave(SAVE_NORMAL), FALSE);

    InitNormalStateForChampionsRunTest();
    EXPECT_EQ(ChampionsRun_BeginEntry(), TRUE);

    EXPECT_EQ(ChampionsRun_ShouldUseTemporarySave(SAVE_NORMAL), TRUE);
    EXPECT_EQ(ChampionsRun_ShouldUseTemporarySave(SAVE_LINK), FALSE);

    ChampionsRun_RestoreNormalState(CHAMPIONS_RUN_STATUS_RETIRED);
}

TEST("Champions run blocks normal PC access while active")
{
    ChampionsRun_CanUseNormalPc();
    EXPECT_EQ(gSpecialVar_Result, TRUE);

    InitNormalStateForChampionsRunTest();
    EXPECT_EQ(ChampionsRun_BeginEntry(), TRUE);

    ChampionsRun_CanUseNormalPc();
    EXPECT_EQ(gSpecialVar_Result, FALSE);

    ChampionsRun_RestoreNormalState(CHAMPIONS_RUN_STATUS_RETIRED);
}

TEST("Champions run enables run restrictions only while active")
{
    EXPECT_EQ(ChampionsRun_ShouldBlockBagUse(), FALSE);
    EXPECT_EQ(ChampionsRun_ShouldBlockHeldItemChanges(), FALSE);
    EXPECT_EQ(ChampionsRun_ShouldSuppressExp(), FALSE);

    InitNormalStateForChampionsRunTest();
    EXPECT_EQ(ChampionsRun_BeginEntry(), TRUE);

    EXPECT_EQ(ChampionsRun_ShouldBlockBagUse(), TRUE);
    EXPECT_EQ(ChampionsRun_ShouldBlockHeldItemChanges(), TRUE);
    EXPECT_EQ(ChampionsRun_ShouldSuppressExp(), TRUE);
    EXPECT_EQ(ChampionsRun_EndByBattleOutcome(B_OUTCOME_WON), FALSE);
    EXPECT_EQ(ChampionsRun_IsActive(), TRUE);

    ChampionsRun_RestoreNormalState(CHAMPIONS_RUN_STATUS_RETIRED);

    EXPECT_EQ(ChampionsRun_ShouldBlockBagUse(), FALSE);
    EXPECT_EQ(ChampionsRun_ShouldBlockHeldItemChanges(), FALSE);
    EXPECT_EQ(ChampionsRun_ShouldSuppressExp(), FALSE);
}

TEST("Champions run defeat restores the normal party before clearing the session")
{
    InitNormalStateForChampionsRunTest();
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_LITTLEROOT_TOWN);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_LITTLEROOT_TOWN);
    gSaveBlock1Ptr->location.warpId = WARP_ID_NONE;
    gSaveBlock1Ptr->location.x = 7;
    gSaveBlock1Ptr->location.y = 8;
    gSaveBlock1Ptr->pos.x = 7;
    gSaveBlock1Ptr->pos.y = 8;
    EXPECT_EQ(ChampionsRun_BeginEntry(), TRUE);

    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_ROUTE101);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_ROUTE101);
    gSaveBlock1Ptr->location.warpId = WARP_ID_NONE;
    gSaveBlock1Ptr->location.x = 10;
    gSaveBlock1Ptr->location.y = 10;
    gSaveBlock1Ptr->pos.x = 10;
    gSaveBlock1Ptr->pos.y = 10;
    CreateMon(&gPlayerParty[0], SPECIES_MAGIKARP, 1, 0, OTID_STRUCT_PRESET(2));
    CalculatePlayerPartyCount();
    EXPECT_EQ(gPlayerPartyCount, 1);
    EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_SPECIES), SPECIES_MAGIKARP);

    EXPECT_EQ(ChampionsRun_EndByBattleOutcome(B_OUTCOME_LOST), TRUE);

    EXPECT_EQ(ChampionsRun_IsActive(), FALSE);
    EXPECT_EQ(gPlayerPartyCount, 2);
    EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_SPECIES), SPECIES_PIKACHU);
    EXPECT_EQ(GetMonData(&gPlayerParty[1], MON_DATA_SPECIES), SPECIES_DRAGONITE);
    EXPECT_EQ(CheckBagHasItem(ITEM_POTION, 3), TRUE);
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 12345);
    EXPECT_EQ(GetCoins(), 123);
    EXPECT_EQ(gSaveBlock1Ptr->location.mapGroup, MAP_GROUP(MAP_LITTLEROOT_TOWN));
    EXPECT_EQ(gSaveBlock1Ptr->location.mapNum, MAP_NUM(MAP_LITTLEROOT_TOWN));
    EXPECT_EQ(gSaveBlock1Ptr->pos.x, 7);
    EXPECT_EQ(gSaveBlock1Ptr->pos.y, 8);
    EXPECT_EQ(gSaveBlock1Ptr->continueGameWarp.mapGroup, MAP_GROUP(MAP_LITTLEROOT_TOWN));
    EXPECT_EQ(gSaveBlock1Ptr->continueGameWarp.mapNum, MAP_NUM(MAP_LITTLEROOT_TOWN));
    EXPECT_EQ(UseContinueGameWarp(), FALSE);
}

TEST("Champions run retire restores the normal state and clears the run")
{
    u32 i;

    InitNormalStateForChampionsRunTest();
    EXPECT_EQ(ChampionsRun_BeginEntry(), TRUE);

    CreateMon(&gPlayerParty[0], SPECIES_MAGIKARP, 1, 0, OTID_STRUCT_PRESET(2));
    CalculatePlayerPartyCount();
    EXPECT_EQ(AddBagItem(ITEM_RARE_CANDY, 2), TRUE);
    for (i = 0; i < ARRAY_COUNT(gSaveBlock1Ptr->mapView); i++)
        gSaveBlock1Ptr->mapView[i] = 0x1234;

    EXPECT_EQ(ChampionsRun_RetireAndSave(), SAVE_STATUS_OK);

    EXPECT_EQ(ChampionsRun_IsActive(), FALSE);
    EXPECT_EQ(gPlayerPartyCount, 2);
    EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_SPECIES), SPECIES_PIKACHU);
    EXPECT_EQ(GetMonData(&gPlayerParty[1], MON_DATA_SPECIES), SPECIES_DRAGONITE);
    EXPECT_EQ(UseContinueGameWarp(), FALSE);
    EXPECT_EQ(CheckBagHasItem(ITEM_POTION, 3), TRUE);
    EXPECT_EQ(CheckBagHasItem(ITEM_RARE_CANDY, 1), FALSE);
    for (i = 0; i < ARRAY_COUNT(gSaveBlock1Ptr->mapView); i++)
        EXPECT_EQ(gSaveBlock1Ptr->mapView[i], 0);
}

TEST("Champions run clear deposits run party and follows configured next-start carryover")
{
    u16 heldItem = ITEM_LEFTOVERS;

    ResetPokemonStorageSystem();
    InitNormalStateForChampionsRunTest();
    EXPECT_EQ(ChampionsRun_BeginEntry(), TRUE);

    CreateMon(&gPlayerParty[0], SPECIES_MAGIKARP, 10, 0, OTID_STRUCT_PRESET(2));
    CreateMon(&gPlayerParty[1], SPECIES_DRAGONITE, 50, 0, OTID_STRUCT_PRESET(3));
    SetMonData(&gPlayerParty[1], MON_DATA_HELD_ITEM, &heldItem);
    CalculatePlayerPartyCount();
    EXPECT_EQ(gPlayerPartyCount, 2);
    EXPECT_EQ(AddBagItem(ITEM_RARE_CANDY, 2), TRUE);

    EXPECT_EQ(ChampionsRun_CompleteClearAndSave(), SAVE_STATUS_OK);

    EXPECT_EQ(ChampionsRun_IsActive(), FALSE);
    EXPECT_EQ(gPlayerPartyCount, 2);
    EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_SPECIES), SPECIES_PIKACHU);
    EXPECT_EQ(GetMonData(&gPlayerParty[1], MON_DATA_SPECIES), SPECIES_DRAGONITE);
    EXPECT_EQ(CountAllStorageMons(), 2);
#if CHAMPIONS_RUN_CLEAR_CARRY_ITEMS == TRUE
    EXPECT_EQ(CheckBagHasItem(ITEM_RARE_CANDY, 2), TRUE);
#endif
#if CHAMPIONS_RUN_CLEAR_HELD_ITEM_MODE == CHAMPIONS_RUN_HELD_ITEM_CARRY_ON_MON
    EXPECT_EQ(GetBoxMonDataAt(0, 1, MON_DATA_HELD_ITEM), ITEM_LEFTOVERS);
#elif CHAMPIONS_RUN_CLEAR_HELD_ITEM_MODE == CHAMPIONS_RUN_HELD_ITEM_CARRY_TO_BAG
    EXPECT_EQ(GetBoxMonDataAt(0, 1, MON_DATA_HELD_ITEM), ITEM_NONE);
    EXPECT_EQ(CheckBagHasItem(ITEM_LEFTOVERS, 1), TRUE);
#else
    EXPECT_EQ(GetBoxMonDataAt(0, 1, MON_DATA_HELD_ITEM), ITEM_NONE);
    EXPECT_EQ(CheckBagHasItem(ITEM_LEFTOVERS, 1), FALSE);
#endif

    EXPECT_EQ(ChampionsRun_BeginEntry(), TRUE);
#if CHAMPIONS_RUN_ENTRY_PARTY_MODE == CHAMPIONS_RUN_START_PARTY_LAST_CLEAR
    EXPECT_EQ(gPlayerPartyCount, 2);
    EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_SPECIES), SPECIES_MAGIKARP);
    EXPECT_EQ(GetMonData(&gPlayerParty[1], MON_DATA_SPECIES), SPECIES_DRAGONITE);
#elif CHAMPIONS_RUN_ENTRY_PARTY_MODE == CHAMPIONS_RUN_START_PARTY_CURRENT
    EXPECT_EQ(gPlayerPartyCount, 2);
    EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_SPECIES), SPECIES_PIKACHU);
    EXPECT_EQ(GetMonData(&gPlayerParty[1], MON_DATA_SPECIES), SPECIES_DRAGONITE);
#else
    EXPECT_EQ(gPlayerPartyCount, 0);
#endif

    ChampionsRun_RestoreNormalState(CHAMPIONS_RUN_STATUS_RETIRED);
}
