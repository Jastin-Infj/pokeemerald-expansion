#include "global.h"
#include "battle.h"
#include "battle_util.h"
#include "malloc.h"
#include "pokemon.h"
#include "test/test.h"
#include "constants/items.h"
#include "constants/species.h"

static void SetupPlayerMonHeldItem(enum Item originalItem, enum Item currentItem)
{
    ZeroPlayerPartyMons();
    CreateMon(&gPlayerParty[0], SPECIES_WOBBUFFET, 50, 0, OTID_STRUCT_PRESET(0));
    CalculateMonStats(&gPlayerParty[0]);
    CalculatePlayerPartyCount();
    SetMonData(&gPlayerParty[0], MON_DATA_HELD_ITEM, &currentItem);

    gBattleStruct = AllocZeroed(sizeof(*gBattleStruct));
    gBattleStruct->itemLost[B_SIDE_PLAYER][0].originalItem = originalItem;
}

static void TearDownHeldItemRestoreTest(void)
{
    Free(gBattleStruct);
    gBattleStruct = NULL;
    gBattleTypeFlags = 0;
    ZeroPlayerPartyMons();
}

TEST("Battle item restore includes berries when configured")
{
    ASSUME(B_RESTORE_HELD_BATTLE_BERRIES == TRUE);

    SetupPlayerMonHeldItem(ITEM_ORAN_BERRY, ITEM_NONE);
    TryRestoreHeldItems();

    EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_HELD_ITEM), ITEM_ORAN_BERRY);
    TearDownHeldItemRestoreTest();
}

TEST("Battle item restore keeps existing non-berry restore behavior")
{
    ASSUME(B_RESTORE_HELD_BATTLE_ITEMS >= GEN_9);

    SetupPlayerMonHeldItem(ITEM_WHITE_HERB, ITEM_NONE);
    TryRestoreHeldItems();

    EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_HELD_ITEM), ITEM_WHITE_HERB);
    TearDownHeldItemRestoreTest();
}
