#include "global.h"
#include "ui_style.h"
#include "test/test.h"

TEST("UI style uses save padding without changing neighboring options")
{
    struct SaveBlock2 save = {0};
    struct SaveBlock2 *previous = gSaveBlock2Ptr;
    u16 *options = (u16 *)((u8 *)&save + 0x14);

    // Existing settings and region map zoom occupy the low twelve bits.
    *options = 0x0A55;
    gSaveBlock2Ptr = &save;
    EXPECT(!IsUiStyleXY());
    save.optionsUiStyle = OPTIONS_UI_STYLE_XY;
    EXPECT(IsUiStyleXY());
    EXPECT_EQ(*options, 0x1A55);
    EXPECT_EQ((u8 *)&save.pokedex - (u8 *)&save, 0x18);
    save.optionsUiStyle = OPTIONS_UI_STYLE_DEFAULT;
    EXPECT(!IsUiStyleXY());
    EXPECT_EQ(*options, 0x0A55);
    save.optionsUiStyle = 15;
    EXPECT(!IsUiStyleXY());
    gSaveBlock2Ptr = previous;
}

TEST("UI style and battle menu independently preserve existing save offsets")
{
    struct SaveBlock2 save = {0};
    struct SaveBlock2 *previous = gSaveBlock2Ptr;
    u16 *options = (u16 *)((u8 *)&save + 0x14);
    gSaveBlock2Ptr = &save;
    *options = 0x0A55;
    EXPECT_EQ((u8 *)&save.optionsBattleMenu - (u8 *)&save, 0x16);
    EXPECT_EQ((u8 *)&save.pokedex - (u8 *)&save, 0x18);
    EXPECT(!IsBattleMenuSM());
    save.optionsBattleMenu = OPTIONS_BATTLE_MENU_SM;
    EXPECT(IsBattleMenuSM());
    EXPECT(!IsUiStyleXY());
    EXPECT_EQ(*options, 0x0A55);
    save.optionsUiStyle = OPTIONS_UI_STYLE_XY;
    EXPECT(IsUiStyleXY());
    EXPECT(IsBattleMenuSM());
    save.optionsBattleMenu = OPTIONS_BATTLE_MENU_DEFAULT;
    EXPECT(IsUiStyleXY());
    EXPECT(!IsBattleMenuSM());
    EXPECT_EQ(*options, 0x1A55);
    save.optionsBattleMenu = 0xFFFF;
    EXPECT(!IsBattleMenuSM());
    EXPECT_EQ(*options, 0x1A55);
    gSaveBlock2Ptr = previous;
}
