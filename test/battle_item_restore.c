#include "global.h"
#include "battle.h"
#include "battle_util.h"
#include "malloc.h"
#include "pokemon.h"
#include "test/test.h"
#include "constants/battle.h"
#include "constants/items.h"
#include "constants/species.h"

static void SetupPlayerMonHeldItem(enum Item originalItem, enum Item currentItem)
{
    ZeroPlayerPartyMons();
    CreateMon(&gParties[B_TRAINER_PLAYER][0], SPECIES_WOBBUFFET, 50, 0, OTID_STRUCT_PRESET(0));
    CalculateMonStats(&gParties[B_TRAINER_PLAYER][0]);
    CalculatePlayerPartyCount();
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HELD_ITEM, &currentItem);

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

    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HELD_ITEM), ITEM_ORAN_BERRY);
    TearDownHeldItemRestoreTest();
}

TEST("Battle item restore keeps existing non-berry restore behavior")
{
    ASSUME(B_RESTORE_HELD_BATTLE_ITEMS >= GEN_9);

    SetupPlayerMonHeldItem(ITEM_WHITE_HERB, ITEM_NONE);
    TryRestoreHeldItems();

    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HELD_ITEM), ITEM_WHITE_HERB);
    TearDownHeldItemRestoreTest();
}

#if B_RETURN_STOLEN_NPC_ITEMS >= GEN_5 && B_RESTORE_HELD_BATTLE_ITEMS < GEN_9
TEST("Battle item restore clears trainer items when original item is not restored")
{
    SetupPlayerMonHeldItem(ITEM_WHITE_HERB, ITEM_LEFTOVERS);
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    TryRestoreHeldItems();

    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HELD_ITEM), ITEM_NONE);
    TearDownHeldItemRestoreTest();
}
#endif
