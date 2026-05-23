#include "global.h"
#include "dynamic_placeholder_text_util.h"
#include "event_data.h"
#include "international_string_util.h"
#include "item.h"
#include "list_menu.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "menu_helpers.h"
#include "money.h"
#include "move.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "pokemon_vendor.h"
#include "random.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "constants/game_stat.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/pokedex.h"
#include "constants/pokeball.h"
#include "constants/rgb.h"
#include "constants/songs.h"

#define MAX_PRODUCTS_SHOWN 6
#define VENDOR_LIST_NAME_LENGTH 18
#define VENDOR_BOND_MAX 255

enum {
    WIN_MONEY,
    WIN_LIST,
    WIN_INFO,
    WIN_MESSAGE,
    WIN_COUNT,
};

enum {
    COLORID_NORMAL,
    COLORID_GRAY,
};

enum {
    PRODUCT_SELECT_OK,
    PRODUCT_SELECT_LOCKED,
    PRODUCT_SELECT_SOLD_OUT,
    PRODUCT_SELECT_NO_MONEY,
    PRODUCT_SELECT_NO_ROOM,
};

struct PokemonVendorMenu
{
    const struct PokemonVendorProduct *products;
    struct ListMenuItem *items;
    u8 (*names)[VENDOR_LIST_NAME_LENGTH + 1];
    u16 *productIndexes;
    u16 productCount;
    u16 visibleCount;
    u16 selectedProductIndex;
    u16 scrollOffset;
    u16 selectedRow;
    u8 listTaskId;
    u8 windowIds[WIN_COUNT];
};

static EWRAM_DATA struct PokemonVendorMenu *sPokemonVendorMenu = NULL;

static void Task_PokemonVendorWaitForFade(u8 taskId);
static void Task_PokemonVendorHandleInput(u8 taskId);
static void Task_PokemonVendorReturnToList(u8 taskId);
static void Task_PokemonVendorClose(u8 taskId);
static void PokemonVendorBuildList(void);
static void PokemonVendorFree(void);
static void PokemonVendorInitWindows(void);
static void PokemonVendorDrawWindows(void);
static void PokemonVendorPrintMoney(void);
static void PokemonVendorPrintProductInfo(s32 item, bool8 onInit, struct ListMenu *list);
static void PokemonVendorPrintPrice(u8 windowId, u32 item, u8 y);
static void PokemonVendorConfirmPurchase(u8 taskId);
static void PokemonVendorTryPurchase(u8 taskId);
static void PokemonVendorCancelPurchase(u8 taskId);
static void PokemonVendorFinishPurchase(u8 taskId);
static u8 PokemonVendorGetProductSelectState(const struct PokemonVendorProduct *product);
static bool32 PokemonVendorProductIsSoldOut(const struct PokemonVendorProduct *product);
static bool32 PokemonVendorProductIsUnlocked(const struct PokemonVendorProduct *product);
static bool32 PokemonVendorTryDeliverProduct(const struct PokemonVendorProduct *product);
static void PokemonVendorCreateMon(const struct PokemonVendorProduct *product, struct Pokemon *mon);
static void PokemonVendorMarkSealedRecruit(const struct PokemonVendorProduct *product, struct Pokemon *mon);
static u8 PokemonVendorClampBondThreshold(u16 threshold);
static void PokemonVendorUnlockSealedRecruit(struct Pokemon *mon);

static const u8 sText_Sealed[] = _("SEALED");
static const u8 sText_Normal[] = _("NORMAL");
static const u8 sText_Repeat[] = _("Repeat");
static const u8 sText_OneTime[] = _("One-time");
static const u8 sText_LockedRow[] = _("LOCKED");
static const u8 sText_QuestionMarks[] = _("?????");
static const u8 sText_QuitVendor[] = _("Close the vendor.");
static const u8 sText_ProductLocked[] = _("This recruit is still locked.{PAUSE_UNTIL_PRESS}");
static const u8 sText_SoldOut[] = _("I'm sorry, but that recruit is sold out.{PAUSE_UNTIL_PRESS}");
static const u8 sText_NoRoomForPokemon[] = _("There is no room for this POKéMON.{PAUSE_UNTIL_PRESS}");
static const u8 sText_NoRoomForSealed[] = _("A sealed recruit must join your party.{PAUSE_UNTIL_PRESS}");
static const u8 sText_HereYouGo[] = _("Here you go! Take good care of it.{PAUSE_UNTIL_PRESS}");
static const u8 sText_ConfirmPurchase[] = _("You wanted {STR_VAR_1}?\nThat'll be ¥{STR_VAR_2}. Okay?");
static const u8 sText_InfoNormal[] = _("{DYNAMIC 0}  {LV_2}{DYNAMIC 1}\n{DYNAMIC 2}\nReady to use.");
static const u8 sText_InfoSealed[] = _("{DYNAMIC 0}  {LV_2}{DYNAMIC 1}\n{DYNAMIC 2}\nBond: {DYNAMIC 3}");
static const u8 sText_BondProgress[] = _("{STR_VAR_1}/{STR_VAR_2}");

static const u8 sVendorTextColors[][3] =
{
    [COLORID_NORMAL] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY},
    [COLORID_GRAY] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_LIGHT_GRAY, TEXT_COLOR_DARK_GRAY},
};

static const struct WindowTemplate sPokemonVendorWindowTemplates[WIN_COUNT] =
{
    [WIN_MONEY] = {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 1,
        .width = 12,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0x001,
    },
    [WIN_LIST] = {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 4,
        .width = 16,
        .height = 14,
        .paletteNum = 15,
        .baseBlock = 0x019,
    },
    [WIN_INFO] = {
        .bg = 0,
        .tilemapLeft = 17,
        .tilemapTop = 4,
        .width = 12,
        .height = 8,
        .paletteNum = 15,
        .baseBlock = 0x0F9,
    },
    [WIN_MESSAGE] = {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 15,
        .width = 26,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 0x159,
    },
};

static const struct WindowTemplate sPokemonVendorYesNoWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 23,
    .tilemapTop = 9,
    .width = 5,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = 0x1C1,
};

static const struct ListMenuTemplate sPokemonVendorListTemplate =
{
    .items = NULL,
    .moveCursorFunc = PokemonVendorPrintProductInfo,
    .itemPrintFunc = PokemonVendorPrintPrice,
    .totalItems = 0,
    .maxShowed = 0,
    .windowId = WIN_LIST,
    .header_X = 0,
    .item_X = 8,
    .cursor_X = 0,
    .upText_Y = 1,
    .cursorPal = 2,
    .fillValue = 0,
    .cursorShadowPal = 3,
    .lettersSpacing = 0,
    .itemVerticalPadding = 1,
    .scrollMultiple = LIST_MULTIPLE_SCROLL_DPAD,
    .fontId = FONT_NARROW,
    .cursorKind = CURSOR_BLACK_ARROW,
    .textNarrowWidth = 88,
};

static const struct YesNoFuncTable sPokemonVendorPurchaseYesNoFuncs =
{
    PokemonVendorTryPurchase,
    PokemonVendorCancelPurchase
};

void CreatePokemonVendorMenu(const struct PokemonVendorProduct *productsForSale)
{
    u8 taskId;

    if (productsForSale == NULL)
    {
        ScriptContext_Enable();
        return;
    }

    sPokemonVendorMenu = AllocZeroed(sizeof(*sPokemonVendorMenu));
    if (sPokemonVendorMenu == NULL)
    {
        ScriptContext_Enable();
        return;
    }

    sPokemonVendorMenu->products = productsForSale;
    sPokemonVendorMenu->listTaskId = TASK_NONE;
    PokemonVendorInitWindows();
    PokemonVendorBuildList();
    PokemonVendorDrawWindows();

    taskId = CreateTask(Task_PokemonVendorWaitForFade, 8);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0, RGB_BLACK);
    gTasks[taskId].data[0] = ListMenuInit(&gMultiuseListMenuTemplate, 0, 0);
    sPokemonVendorMenu->listTaskId = gTasks[taskId].data[0];
}

static void PokemonVendorBuildList(void)
{
    u16 i;
    u16 visible = 0;
    u16 count = 0;

    while (sPokemonVendorMenu->products[count].species != SPECIES_NONE)
        count++;

    sPokemonVendorMenu->productCount = count;
    sPokemonVendorMenu->items = Alloc((count + 1) * sizeof(*sPokemonVendorMenu->items));
    sPokemonVendorMenu->names = Alloc((count + 1) * sizeof(*sPokemonVendorMenu->names));
    sPokemonVendorMenu->productIndexes = Alloc((count + 1) * sizeof(*sPokemonVendorMenu->productIndexes));

    for (i = 0; i < count; i++)
    {
        const struct PokemonVendorProduct *product = &sPokemonVendorMenu->products[i];

        if (PokemonVendorProductIsSoldOut(product))
            continue;

        if (product->revealPolicy == POKEMON_VENDOR_REVEAL_LOCKED && !PokemonVendorProductIsUnlocked(product))
            StringCopy(sPokemonVendorMenu->names[visible], sText_QuestionMarks);
        else
            StringCopy(sPokemonVendorMenu->names[visible], GetSpeciesName(product->species));

        sPokemonVendorMenu->items[visible].name = sPokemonVendorMenu->names[visible];
        sPokemonVendorMenu->items[visible].id = visible;
        sPokemonVendorMenu->productIndexes[visible] = i;
        visible++;
    }

    StringCopy(sPokemonVendorMenu->names[visible], gText_Cancel2);
    sPokemonVendorMenu->items[visible].name = sPokemonVendorMenu->names[visible];
    sPokemonVendorMenu->items[visible].id = LIST_CANCEL;
    sPokemonVendorMenu->visibleCount = visible;

    gMultiuseListMenuTemplate = sPokemonVendorListTemplate;
    gMultiuseListMenuTemplate.items = sPokemonVendorMenu->items;
    gMultiuseListMenuTemplate.totalItems = visible + 1;
    gMultiuseListMenuTemplate.maxShowed = min(MAX_PRODUCTS_SHOWN, visible + 1);
    gMultiuseListMenuTemplate.windowId = sPokemonVendorMenu->windowIds[WIN_LIST];
}

static void PokemonVendorInitWindows(void)
{
    u8 i;

    LoadMessageBoxAndBorderGfx();
    Menu_LoadStdPal();

    for (i = 0; i < WIN_COUNT; i++)
        sPokemonVendorMenu->windowIds[i] = AddWindow(&sPokemonVendorWindowTemplates[i]);

    DeactivateAllTextPrinters();
}

static void PokemonVendorDrawWindows(void)
{
    u8 i;

    for (i = 0; i < WIN_COUNT; i++)
    {
        FillWindowPixelBuffer(sPokemonVendorMenu->windowIds[i], PIXEL_FILL(0));
        DrawStdWindowFrame(sPokemonVendorMenu->windowIds[i], FALSE);
        PutWindowTilemap(sPokemonVendorMenu->windowIds[i]);
        CopyWindowToVram(sPokemonVendorMenu->windowIds[i], COPYWIN_FULL);
    }

    PokemonVendorPrintMoney();
    ScheduleBgCopyTilemapToVram(0);
}

static void PokemonVendorPrintMoney(void)
{
    u8 windowId = sPokemonVendorMenu->windowIds[WIN_MONEY];

    FillWindowPixelBuffer(windowId, PIXEL_FILL(0));
    DrawStdWindowFrame(windowId, FALSE);
    PrintMoneyAmount(windowId, 8, 1, GetMoney(&gSaveBlock1Ptr->money), TEXT_SKIP_DRAW);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

static void Task_PokemonVendorWaitForFade(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_PokemonVendorHandleInput;
}

static void Task_PokemonVendorHandleInput(u8 taskId)
{
    s32 itemId = ListMenu_ProcessInput(sPokemonVendorMenu->listTaskId);

    ListMenuGetScrollAndRow(sPokemonVendorMenu->listTaskId, &sPokemonVendorMenu->scrollOffset, &sPokemonVendorMenu->selectedRow);

    switch (itemId)
    {
    case LIST_NOTHING_CHOSEN:
        break;
    case LIST_CANCEL:
        PlaySE(SE_SELECT);
        Task_PokemonVendorClose(taskId);
        break;
    default:
    {
        const struct PokemonVendorProduct *product;
        u8 state;

        PlaySE(SE_SELECT);
        sPokemonVendorMenu->selectedProductIndex = sPokemonVendorMenu->productIndexes[itemId];
        product = &sPokemonVendorMenu->products[sPokemonVendorMenu->selectedProductIndex];
        state = PokemonVendorGetProductSelectState(product);

        switch (state)
        {
        case PRODUCT_SELECT_LOCKED:
            DisplayMessageAndContinueTask(taskId, sPokemonVendorMenu->windowIds[WIN_MESSAGE], 0xA, 0xE, FONT_NORMAL, GetPlayerTextSpeedDelay(), sText_ProductLocked, Task_PokemonVendorReturnToList);
            break;
        case PRODUCT_SELECT_SOLD_OUT:
            DisplayMessageAndContinueTask(taskId, sPokemonVendorMenu->windowIds[WIN_MESSAGE], 0xA, 0xE, FONT_NORMAL, GetPlayerTextSpeedDelay(), sText_SoldOut, Task_PokemonVendorReturnToList);
            break;
        case PRODUCT_SELECT_NO_MONEY:
            DisplayMessageAndContinueTask(taskId, sPokemonVendorMenu->windowIds[WIN_MESSAGE], 0xA, 0xE, FONT_NORMAL, GetPlayerTextSpeedDelay(), gText_YouDontHaveMoney, Task_PokemonVendorReturnToList);
            break;
        case PRODUCT_SELECT_NO_ROOM:
            DisplayMessageAndContinueTask(taskId, sPokemonVendorMenu->windowIds[WIN_MESSAGE], 0xA, 0xE, FONT_NORMAL, GetPlayerTextSpeedDelay(),
                                          product->kind == POKEMON_VENDOR_PRODUCT_SEALED ? sText_NoRoomForSealed : sText_NoRoomForPokemon,
                                          Task_PokemonVendorReturnToList);
            break;
        default:
            StringCopy(gStringVar1, GetSpeciesName(product->species));
            ConvertIntToDecimalStringN(gStringVar2, product->price, STR_CONV_MODE_LEFT_ALIGN, MAX_MONEY_DIGITS);
            StringExpandPlaceholders(gStringVar4, sText_ConfirmPurchase);
            DisplayMessageAndContinueTask(taskId, sPokemonVendorMenu->windowIds[WIN_MESSAGE], 0xA, 0xE, FONT_NORMAL, GetPlayerTextSpeedDelay(), gStringVar4, PokemonVendorConfirmPurchase);
            break;
        }
        break;
    }
    }
}

static u8 PokemonVendorGetProductSelectState(const struct PokemonVendorProduct *product)
{
    if (!PokemonVendorProductIsUnlocked(product))
        return PRODUCT_SELECT_LOCKED;
    if (PokemonVendorProductIsSoldOut(product))
        return PRODUCT_SELECT_SOLD_OUT;
    if (!IsEnoughMoney(&gSaveBlock1Ptr->money, product->price))
        return PRODUCT_SELECT_NO_MONEY;
    if (product->kind == POKEMON_VENDOR_PRODUCT_SEALED)
    {
        if (gPlayerPartyCount >= PARTY_SIZE)
            return PRODUCT_SELECT_NO_ROOM;
    }
    else if (IsPlayerPartyAndPokemonStorageFull())
    {
        return PRODUCT_SELECT_NO_ROOM;
    }

    return PRODUCT_SELECT_OK;
}

static void PokemonVendorConfirmPurchase(u8 taskId)
{
    CreateYesNoMenuWithCallbacks(taskId, &sPokemonVendorYesNoWindowTemplate, 1, 0, 0, 0x1, 0xE, &sPokemonVendorPurchaseYesNoFuncs);
}

static void PokemonVendorTryPurchase(u8 taskId)
{
    const struct PokemonVendorProduct *product = &sPokemonVendorMenu->products[sPokemonVendorMenu->selectedProductIndex];

    if (PokemonVendorTryDeliverProduct(product))
    {
        if (product->purchaseMode == POKEMON_VENDOR_PURCHASE_ONCE && product->oneTimeFlag != POKEMON_VENDOR_NO_FLAG)
            FlagSet(product->oneTimeFlag);
        RemoveMoney(&gSaveBlock1Ptr->money, product->price);
        IncrementGameStat(GAME_STAT_SHOPPED);
        PlaySE(SE_SHOP);
        PokemonVendorPrintMoney();
        DisplayMessageAndContinueTask(taskId, sPokemonVendorMenu->windowIds[WIN_MESSAGE], 0xA, 0xE, FONT_NORMAL, GetPlayerTextSpeedDelay(), sText_HereYouGo, PokemonVendorFinishPurchase);
    }
    else
    {
        DisplayMessageAndContinueTask(taskId, sPokemonVendorMenu->windowIds[WIN_MESSAGE], 0xA, 0xE, FONT_NORMAL, GetPlayerTextSpeedDelay(),
                                      product->kind == POKEMON_VENDOR_PRODUCT_SEALED ? sText_NoRoomForSealed : sText_NoRoomForPokemon,
                                      Task_PokemonVendorReturnToList);
    }
}

static void PokemonVendorCancelPurchase(u8 taskId)
{
    Task_PokemonVendorReturnToList(taskId);
}

static void PokemonVendorFinishPurchase(u8 taskId)
{
    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        PokemonVendorFree();
        PokemonVendorBuildList();
        DrawStdWindowFrame(sPokemonVendorMenu->windowIds[WIN_LIST], FALSE);
        PutWindowTilemap(sPokemonVendorMenu->windowIds[WIN_LIST]);
        CopyWindowToVram(sPokemonVendorMenu->windowIds[WIN_LIST], COPYWIN_FULL);
        sPokemonVendorMenu->listTaskId = ListMenuInit(&gMultiuseListMenuTemplate, sPokemonVendorMenu->scrollOffset, min(sPokemonVendorMenu->selectedRow, sPokemonVendorMenu->visibleCount));
        Task_PokemonVendorReturnToList(taskId);
    }
}

static void Task_PokemonVendorReturnToList(u8 taskId)
{
    FillWindowPixelBuffer(sPokemonVendorMenu->windowIds[WIN_MESSAGE], PIXEL_FILL(0));
    ClearStdWindowAndFrameToTransparent(sPokemonVendorMenu->windowIds[WIN_MESSAGE], FALSE);
    ClearWindowTilemap(sPokemonVendorMenu->windowIds[WIN_MESSAGE]);
    PutWindowTilemap(sPokemonVendorMenu->windowIds[WIN_LIST]);
    PutWindowTilemap(sPokemonVendorMenu->windowIds[WIN_INFO]);
    RedrawListMenu(sPokemonVendorMenu->listTaskId);
    CopyWindowToVram(sPokemonVendorMenu->windowIds[WIN_MESSAGE], COPYWIN_FULL);
    ScheduleBgCopyTilemapToVram(0);
    gTasks[taskId].func = Task_PokemonVendorHandleInput;
}

static void Task_PokemonVendorClose(u8 taskId)
{
    u8 i;

    if (sPokemonVendorMenu != NULL)
    {
        PokemonVendorFree();
        for (i = 0; i < WIN_COUNT; i++)
        {
            ClearStdWindowAndFrameToTransparent(sPokemonVendorMenu->windowIds[i], FALSE);
            ClearWindowTilemap(sPokemonVendorMenu->windowIds[i]);
            RemoveWindow(sPokemonVendorMenu->windowIds[i]);
        }
        ScheduleBgCopyTilemapToVram(0);
        Free(sPokemonVendorMenu);
        sPokemonVendorMenu = NULL;
    }

    ScriptContext_Enable();
    DestroyTask(taskId);
}

static void PokemonVendorFree(void)
{
    if (sPokemonVendorMenu == NULL)
        return;

    if (sPokemonVendorMenu->listTaskId != TASK_NONE)
    {
        DestroyListMenuTask(sPokemonVendorMenu->listTaskId, NULL, NULL);
        sPokemonVendorMenu->listTaskId = TASK_NONE;
    }
    Free(sPokemonVendorMenu->items);
    Free(sPokemonVendorMenu->names);
    Free(sPokemonVendorMenu->productIndexes);
    sPokemonVendorMenu->items = NULL;
    sPokemonVendorMenu->names = NULL;
    sPokemonVendorMenu->productIndexes = NULL;
}

static void PokemonVendorPrintProductInfo(s32 item, bool8 onInit, struct ListMenu *list)
{
    const struct PokemonVendorProduct *product;
    u8 windowId = sPokemonVendorMenu->windowIds[WIN_INFO];
    u8 thresholdText[8];
    u8 levelText[4];

    if (onInit != TRUE)
        PlaySE(SE_SELECT);

    FillWindowPixelBuffer(windowId, PIXEL_FILL(0));
    DrawStdWindowFrame(windowId, FALSE);

    if (item == LIST_CANCEL)
    {
        AddTextPrinterParameterized4(windowId, FONT_NARROW, 4, 1, 0, 0, sVendorTextColors[COLORID_NORMAL], TEXT_SKIP_DRAW, sText_QuitVendor);
        CopyWindowToVram(windowId, COPYWIN_FULL);
        return;
    }

    product = &sPokemonVendorMenu->products[sPokemonVendorMenu->productIndexes[item]];
    ConvertIntToDecimalStringN(levelText, product->level, STR_CONV_MODE_LEFT_ALIGN, 3);
    DynamicPlaceholderTextUtil_Reset();
    DynamicPlaceholderTextUtil_SetPlaceholderPtr(0, product->kind == POKEMON_VENDOR_PRODUCT_SEALED ? sText_Sealed : sText_Normal);
    DynamicPlaceholderTextUtil_SetPlaceholderPtr(1, levelText);
    DynamicPlaceholderTextUtil_SetPlaceholderPtr(2, product->purchaseMode == POKEMON_VENDOR_PURCHASE_ONCE ? sText_OneTime : sText_Repeat);

    if (product->kind == POKEMON_VENDOR_PRODUCT_SEALED)
    {
        ConvertIntToDecimalStringN(gStringVar1, 0, STR_CONV_MODE_LEFT_ALIGN, 3);
        ConvertIntToDecimalStringN(gStringVar2, PokemonVendorClampBondThreshold(product->bondThreshold), STR_CONV_MODE_LEFT_ALIGN, 3);
        StringExpandPlaceholders(thresholdText, sText_BondProgress);
        DynamicPlaceholderTextUtil_SetPlaceholderPtr(3, thresholdText);
        DynamicPlaceholderTextUtil_ExpandPlaceholders(gStringVar4, sText_InfoSealed);
    }
    else
    {
        DynamicPlaceholderTextUtil_ExpandPlaceholders(gStringVar4, sText_InfoNormal);
    }

    AddTextPrinterParameterized4(windowId, FONT_NARROW, 4, 1, 0, 0, sVendorTextColors[COLORID_NORMAL], TEXT_SKIP_DRAW, gStringVar4);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

static void PokemonVendorPrintPrice(u8 windowId, u32 item, u8 y)
{
    const struct PokemonVendorProduct *product;
    u8 colorId;
    u8 x;

    if (item == LIST_CANCEL)
        return;

    product = &sPokemonVendorMenu->products[sPokemonVendorMenu->productIndexes[item]];
    if (!PokemonVendorProductIsUnlocked(product))
    {
        x = GetStringRightAlignXOffset(FONT_NARROW, sText_LockedRow, 112);
        AddTextPrinterParameterized4(windowId, FONT_NARROW, x, y, 0, 0, sVendorTextColors[COLORID_GRAY], TEXT_SKIP_DRAW, sText_LockedRow);
        return;
    }

    ConvertIntToDecimalStringN(gStringVar1, product->price, STR_CONV_MODE_LEFT_ALIGN, MAX_MONEY_DIGITS);
    x = GetStringRightAlignXOffset(FONT_NARROW, gStringVar1, 112);
    colorId = IsEnoughMoney(&gSaveBlock1Ptr->money, product->price) ? COLORID_NORMAL : COLORID_GRAY;
    AddTextPrinterParameterized4(windowId, FONT_NARROW, x, y, 0, 0, sVendorTextColors[colorId], TEXT_SKIP_DRAW, gStringVar1);
}

static bool32 PokemonVendorProductIsSoldOut(const struct PokemonVendorProduct *product)
{
    return product->purchaseMode == POKEMON_VENDOR_PURCHASE_ONCE
        && product->oneTimeFlag != POKEMON_VENDOR_NO_FLAG
        && FlagGet(product->oneTimeFlag);
}

static bool32 PokemonVendorProductIsUnlocked(const struct PokemonVendorProduct *product)
{
    return product->unlockFlag == POKEMON_VENDOR_NO_FLAG || FlagGet(product->unlockFlag);
}

static bool32 PokemonVendorTryDeliverProduct(const struct PokemonVendorProduct *product)
{
    struct Pokemon mon;

    PokemonVendorCreateMon(product, &mon);

    if (product->kind == POKEMON_VENDOR_PRODUCT_SEALED)
    {
        if (gPlayerPartyCount >= PARTY_SIZE)
            return FALSE;
        CopyMon(&gPlayerParty[gPlayerPartyCount], &mon, sizeof(mon));
        gPlayerPartyCount++;
        HandleSetPokedexFlagFromMon(&mon, FLAG_SET_SEEN);
        HandleSetPokedexFlagFromMon(&mon, FLAG_SET_CAUGHT);
        return TRUE;
    }

    return GiveScriptedMonToPlayer(&mon, PARTY_SIZE) != MON_CANT_GIVE;
}

static void PokemonVendorCreateMon(const struct PokemonVendorProduct *product, struct Pokemon *mon)
{
    u32 i;
    u32 personality;
    u16 move;
    u8 fixedIvs = product->ivs;

    if (fixedIvs > USE_RANDOM_IVS)
        fixedIvs = USE_RANDOM_IVS;

    personality = Random32();
    CreateMonWithIVs(mon, product->species, product->level, personality, OTID_STRUCT_PLAYER_ID, fixedIvs);

    if (product->moves[0] == MOVE_NONE)
    {
        GiveMonInitialMoveset(mon);
    }
    else
    {
        for (i = 0; i < MAX_MON_MOVES; i++)
        {
            move = product->moves[i];
            if (move == MOVE_NONE)
                break;
            if (move == MOVE_DEFAULT)
                GiveMonDefaultMove(mon, i);
            else if (move < MOVES_COUNT)
                SetMonMoveSlot(mon, move, i);
        }
    }

    if (product->heldItem != ITEM_NONE)
        SetMonData(mon, MON_DATA_HELD_ITEM, &product->heldItem);
    if (product->ball < POKEBALL_COUNT)
        SetMonData(mon, MON_DATA_POKEBALL, &product->ball);

    CalculateMonStats(mon);

    if (product->kind == POKEMON_VENDOR_PRODUCT_SEALED)
        PokemonVendorMarkSealedRecruit(product, mon);
}

static void PokemonVendorMarkSealedRecruit(const struct PokemonVendorProduct *product, struct Pokemon *mon)
{
    u8 isEgg = TRUE;
    u8 origin = TRUE;
    u8 progress = 0;
    u8 threshold = PokemonVendorClampBondThreshold(product->bondThreshold);

    SetMonData(mon, MON_DATA_IS_EGG, &isEgg);
    SetMonData(mon, MON_DATA_VENDOR_SEALED_ORIGIN, &origin);
    SetMonData(mon, MON_DATA_FRIENDSHIP, &progress);
    SetMonData(mon, MON_DATA_SHEEN, &threshold);
}

static u8 PokemonVendorClampBondThreshold(u16 threshold)
{
    if (threshold == 0)
        threshold = POKEMON_VENDOR_DEFAULT_BOND_THRESHOLD;
    if (threshold > VENDOR_BOND_MAX)
        threshold = VENDOR_BOND_MAX;
    return threshold;
}

bool32 PokemonVendor_IsSealedOriginMon(struct Pokemon *mon)
{
    return GetMonData(mon, MON_DATA_VENDOR_SEALED_ORIGIN);
}

bool32 PokemonVendor_IsLockedSealedRecruit(struct Pokemon *mon)
{
    return PokemonVendor_IsSealedOriginMon(mon) && GetMonData(mon, MON_DATA_IS_EGG);
}

bool32 PokemonVendor_IsEditEntitled(struct Pokemon *mon)
{
    return PokemonVendor_IsSealedOriginMon(mon);
}

u8 PokemonVendor_GetSealedRecruitBondProgress(struct Pokemon *mon)
{
    if (!PokemonVendor_IsLockedSealedRecruit(mon))
        return 0;
    return GetMonData(mon, MON_DATA_FRIENDSHIP);
}

u8 PokemonVendor_GetSealedRecruitBondThreshold(struct Pokemon *mon)
{
    if (!PokemonVendor_IsLockedSealedRecruit(mon))
        return 0;
    return PokemonVendorClampBondThreshold(GetMonData(mon, MON_DATA_SHEEN));
}

bool32 PokemonVendor_AddBondExp(struct Pokemon *mon, u8 amount)
{
    u8 progress;
    u8 threshold;

    if (!PokemonVendor_IsLockedSealedRecruit(mon))
        return FALSE;

    progress = PokemonVendor_GetSealedRecruitBondProgress(mon);
    threshold = PokemonVendor_GetSealedRecruitBondThreshold(mon);

    if (VENDOR_BOND_MAX - progress < amount)
        progress = VENDOR_BOND_MAX;
    else
        progress += amount;

    SetMonData(mon, MON_DATA_FRIENDSHIP, &progress);

    if (progress >= threshold)
    {
        PokemonVendorUnlockSealedRecruit(mon);
        return TRUE;
    }

    return FALSE;
}

static void PokemonVendorUnlockSealedRecruit(struct Pokemon *mon)
{
    u8 isEgg = FALSE;
    u8 sheen = 0;
    u8 friendship = 70;

    SetMonData(mon, MON_DATA_IS_EGG, &isEgg);
    SetMonData(mon, MON_DATA_SHEEN, &sheen);
    SetMonData(mon, MON_DATA_FRIENDSHIP, &friendship);
    CalculateMonStats(mon);
}

void PokemonVendor_AddBondExpToParty(void)
{
    u8 i;
    u8 amount = gSpecialVar_0x8004;
    u16 unlockedCount = 0;

    if (amount == 0)
        amount = 1;

    for (i = 0; i < gPlayerPartyCount; i++)
    {
        if (PokemonVendor_AddBondExp(&gPlayerParty[i], amount))
            unlockedCount++;
    }

    gSpecialVar_Result = unlockedCount;
}

void PokemonVendor_IsSelectedMonSealedOrigin(void)
{
    gSpecialVar_Result = FALSE;
    if (gSpecialVar_0x8004 < PARTY_SIZE)
        gSpecialVar_Result = PokemonVendor_IsSealedOriginMon(&gPlayerParty[gSpecialVar_0x8004]);
}

void PokemonVendor_IsSelectedMonLockedSealed(void)
{
    gSpecialVar_Result = FALSE;
    if (gSpecialVar_0x8004 < PARTY_SIZE)
        gSpecialVar_Result = PokemonVendor_IsLockedSealedRecruit(&gPlayerParty[gSpecialVar_0x8004]);
}
