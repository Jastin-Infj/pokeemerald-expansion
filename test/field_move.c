#include "global.h"
#include "config/overworld.h"
#include "event_data.h"
#include "field_move.h"
#include "item.h"
#include "test/test.h"
#include "constants/field_move.h"
#include "constants/flags.h"
#include "constants/items.h"

TEST("Modern Field Kit migrates old HM receipt saves")
{
    ASSUME(OW_FIELD_MOVE_MODERNIZATION);
    ASSUME(OW_FIELD_MOVE_TOOLKIT_REQUIRED);

    FlagSet(FLAG_BADGE05_GET);
    FlagSet(FLAG_RECEIVED_HM_SURF);

    EXPECT_EQ(CheckBagHasItem(ITEM_FIELD_KIT, 1), FALSE);
    EXPECT_EQ(IsFieldMoveUnlocked(FIELD_MOVE_SURF), TRUE);
    EXPECT_EQ(CheckBagHasItem(ITEM_FIELD_KIT, 1), TRUE);
}

TEST("Modern Field Kit does not unlock old HM receipt saves if migration cannot fit")
{
    u32 i;

    ASSUME(OW_FIELD_MOVE_MODERNIZATION);
    ASSUME(OW_FIELD_MOVE_TOOLKIT_REQUIRED);

    for (i = 0; i < BAG_KEYITEMS_COUNT; i++)
        EXPECT_EQ(AddBagItem((enum Item)(ITEM_OVAL_CHARM + i), 1), TRUE);

    FlagSet(FLAG_BADGE05_GET);
    FlagSet(FLAG_RECEIVED_HM_SURF);

    EXPECT_EQ(CheckBagHasItem(ITEM_FIELD_KIT, 1), FALSE);
    EXPECT_EQ(AddBagItem(ITEM_FIELD_KIT, 1), FALSE);
    EXPECT_EQ(IsFieldMoveUnlocked(FIELD_MOVE_SURF), FALSE);
    EXPECT_EQ(CheckBagHasItem(ITEM_FIELD_KIT, 1), FALSE);
}
