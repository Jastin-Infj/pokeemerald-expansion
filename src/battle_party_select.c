#include "global.h"
#include "battle_party_select.h"
#include "bg.h"
#include "gpu_regs.h"
#include "malloc.h"
#include "main.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "party_menu.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "battle_main.h"
#include "data.h"
#include "event_data.h"
#include "constants/pokemon.h"
#include "constants/songs.h"
#include "constants/characters.h"
#include "constants/rgb.h"
#include "constants/battle.h"
#include "constants/trainers.h"
#include "strings.h"

enum
{
    PARTY_SELECT_PLAYER,
    PARTY_SELECT_OPPONENT,
};

enum
{
    WIN_PLAYER_PANEL,
    WIN_OPP_PANEL,
    WIN_FOOTER,
    WIN_COUNT,
};

#define PARTY_SELECT_COLS 2
#define PARTY_SELECT_ROWS 3
#define PARTY_SELECT_SLOTS (PARTY_SELECT_COLS * PARTY_SELECT_ROWS)

#define PANEL_WIDTH  15
#define PANEL_HEIGHT 18
#define FOOTER_HEIGHT 2
#define BASE_BLOCK 1

#define PANEL_PIXEL_WIDTH  (PANEL_WIDTH * 8)
#define PANEL_PIXEL_HEIGHT (PANEL_HEIGHT * 8)

#define SLOT_WIDTH   52
#define SLOT_HEIGHT  32
#define SLOT_X0      6
#define SLOT_X1      (SLOT_X0 + SLOT_WIDTH + 4)
#define SLOT_Y0      16
#define SLOT_Y1      (SLOT_Y0 + SLOT_HEIGHT + 4)
#define SLOT_Y2      (SLOT_Y1 + SLOT_HEIGHT + 4)

struct PartySelectConfig
{
    struct Pokemon *playerParty;
    const struct Pokemon *opponentParty;
    const u8 *playerName;
    const u8 *opponentName;
    void (*exitCallback)(void);
    u8 playerPartyCount;
    u8 opponentPartyCount;
    u8 maxSelections;
    bool8 isDoubleBattle;
    bool8 fromScript;
};

struct PartySelectSlot
{
    u8 iconSpriteId;
    u8 order;      // Selection order (1..max), 0 if not selected
    bool8 selected;
    bool8 canSelect;
};

struct PartySelectData
{
    u8 windowIds[WIN_COUNT];
    u8 cursorSide;
    u8 cursorSlot; // 0-5
    u8 selections[2];
    u8 maxSelections[2];
    bool8 isDoubleBattle;
    u8 partyCount[2];
    struct PartySelectSlot slots[2][PARTY_SIZE];
    struct Pokemon opponentParty[PARTY_SIZE];
    struct Pokemon *parties[2];
    u8 trainerNames[2][PLAYER_NAME_LENGTH + 1];
    void (*exitCallback)(void);
    u8 state;
    u8 fadeOut;
    bool8 confirmed;
    bool8 fromScript;
};

static EWRAM_DATA struct PartySelectData *sPartySelect = NULL;

static const struct BgTemplate sPartySelectBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
};

// Window baseBlocks are laid out sequentially to avoid overlap.
#define HEADER_TILES   (PANEL_WIDTH * PANEL_HEIGHT)
#define FOOTER_TILES   (30 * FOOTER_HEIGHT)
#define PLAYER_PANEL_BASE (BASE_BLOCK)
#define OPP_PANEL_BASE    (PLAYER_PANEL_BASE + HEADER_TILES)
#define FOOTER_BASE       (OPP_PANEL_BASE + HEADER_TILES)

static const struct WindowTemplate sPartySelectWindowTemplates[WIN_COUNT + 1] =
{
    [WIN_PLAYER_PANEL] = {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = PANEL_WIDTH,
        .height = PANEL_HEIGHT,
        .paletteNum = 15,
        .baseBlock = PLAYER_PANEL_BASE
    },
    [WIN_OPP_PANEL] = {
        .bg = 0,
        .tilemapLeft = PANEL_WIDTH,
        .tilemapTop = 0,
        .width = PANEL_WIDTH,
        .height = PANEL_HEIGHT,
        .paletteNum = 15,
        .baseBlock = OPP_PANEL_BASE
    },
    [WIN_FOOTER] = {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = PANEL_HEIGHT,
        .width = PANEL_WIDTH * 2,
        .height = FOOTER_HEIGHT,
        .paletteNum = 15,
        .baseBlock = FOOTER_BASE
    },
    DUMMY_WIN_TEMPLATE
};

static const u8 sSlotPosX[] = { SLOT_X0, SLOT_X1, SLOT_X0, SLOT_X1, SLOT_X0, SLOT_X1 };
static const u8 sSlotPosY[] = { SLOT_Y0, SLOT_Y0, SLOT_Y1, SLOT_Y1, SLOT_Y2, SLOT_Y2 };

static const u8 sTextColor_Normal[]   = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY};
static const u8 sTextColor_Fainted[]  = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_LIGHT_GRAY, TEXT_COLOR_DARK_GRAY};
static const u8 sTextColor_Header[]   = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY};
static const u8 sTextColor_Highlight[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_LIGHT_RED, TEXT_COLOR_WHITE};

static void CB2_PartySelect(void);
static void VBlankCB_PartySelect(void);
static void FreePartySelectResources(void);
static void PartySelect_Init(void);
static void PartySelect_DrawPanels(void);
static void PartySelect_DrawPanel(u8 side);
static void PartySelect_DrawSlot(u8 side, u8 slot);
static void PartySelect_DrawFooter(void);
static void PartySelect_CreateIcons(void);
static void PartySelect_DestroyIcons(void);
static void PartySelect_HandleInput(void);
static void PartySelect_MoveCursor(s8 deltaX, s8 deltaY);
static void PartySelect_ToggleSelection(void);
static void PartySelect_ReorderSelections(u8 side, u8 removedOrder);
static void PartySelect_AttemptConfirm(void);
static bool32 PartySelect_IsSlotSelectable(u8 side, u8 slot);
static u8 PartySelect_GetSlotIndex(u8 row, u8 col);
static bool32 PartySelect_FindFirstSelectable(u8 side, u8 *slotOut);
static bool32 PartySelect_Begin(const struct PartySelectConfig *config);
static void PartySelect_SavePlayerSelection(void);

void CB2_OpenBattlePartySelectDemo(void)
{
    struct PartySelectConfig config;
    struct Pokemon opponentParty[PARTY_SIZE];

    memset(&config, 0, sizeof(config));
    memset(opponentParty, 0, sizeof(opponentParty));

    // Use the player's party for both sides for the demo UI.
    memcpy(opponentParty, gPlayerParty, sizeof(opponentParty));

    config.playerParty = gPlayerParty;
    config.opponentParty = opponentParty;
    config.playerPartyCount = CalculatePlayerPartyCount();
    config.opponentPartyCount = config.playerPartyCount;
    config.playerName = gSaveBlock2Ptr->playerName;
    config.opponentName = COMPOUND_STRING("OPPONENT");
    config.isDoubleBattle = FALSE;
    config.maxSelections = FRONTIER_PARTY_SIZE;
    config.exitCallback = CB2_ReturnToFieldContinueScriptPlayMapMusic;
    config.fromScript = FALSE;

    PartySelect_Begin(&config);
}

u16 StartTrainerPartySelect(void)
{
    struct PartySelectConfig config;
    struct Pokemon opponentParty[PARTY_SIZE];
    const u8 *opponentName;
    u16 trainerA = gSpecialVar_0x8004;
    u16 trainerB = gSpecialVar_0x8005;
    bool8 isDouble = (gSpecialVar_0x8006 != 0);
    u32 battleTypeFlags = BATTLE_TYPE_TRAINER;

    gSpecialVar_Result = FALSE;
    memset(&config, 0, sizeof(config));
    memset(opponentParty, 0, sizeof(opponentParty));

    if (trainerA == TRAINER_NONE)
        return FALSE;

    if (isDouble)
        battleTypeFlags |= BATTLE_TYPE_DOUBLE;
    if (trainerB != TRAINER_NONE)
        battleTypeFlags |= BATTLE_TYPE_TWO_OPPONENTS;

    CreateNPCTrainerPartyFromTrainer(opponentParty, GetTrainerStructFromId(trainerA), TRUE, battleTypeFlags);
    if (trainerB != TRAINER_NONE)
        CreateNPCTrainerPartyFromTrainer(&opponentParty[MULTI_PARTY_SIZE], GetTrainerStructFromId(trainerB), FALSE, battleTypeFlags);

    config.playerParty = gPlayerParty;
    config.opponentParty = opponentParty;
    config.playerPartyCount = CalculatePlayerPartyCount();
    config.opponentPartyCount = CalculatePartyCount(opponentParty);
    config.playerName = gSaveBlock2Ptr->playerName;
    opponentName = GetTrainerNameFromId(trainerA);
    config.opponentName = opponentName;
    config.isDoubleBattle = isDouble;
    config.maxSelections = isDouble ? FRONTIER_DOUBLES_PARTY_SIZE : FRONTIER_PARTY_SIZE;
    config.exitCallback = CB2_ReturnToFieldContinueScriptPlayMapMusic;
    config.fromScript = TRUE;

    if (PartySelect_Begin(&config))
    {
        gSpecialVar_Result = TRUE;
        return TRUE;
    }

    return FALSE;
}

static bool32 PartySelect_Begin(const struct PartySelectConfig *config)
{
    u8 playerCount;
    u8 opponentCount;
    u8 maxSelect;

    if (config == NULL || config->playerParty == NULL || config->opponentParty == NULL)
        return FALSE;

    sPartySelect = AllocZeroed(sizeof(*sPartySelect));
    if (sPartySelect == NULL)
        return FALSE;

    playerCount = config->playerPartyCount;
    opponentCount = config->opponentPartyCount;
    maxSelect = config->maxSelections;

    if (playerCount > PARTY_SIZE)
        playerCount = PARTY_SIZE;
    if (opponentCount > PARTY_SIZE)
        opponentCount = PARTY_SIZE;

    if (maxSelect == 0)
        maxSelect = config->isDoubleBattle ? FRONTIER_DOUBLES_PARTY_SIZE : FRONTIER_PARTY_SIZE;
    if (maxSelect > MAX_FRONTIER_PARTY_SIZE)
        maxSelect = MAX_FRONTIER_PARTY_SIZE;

    sPartySelect->isDoubleBattle = config->isDoubleBattle;
    sPartySelect->maxSelections[PARTY_SELECT_PLAYER] = maxSelect;
    sPartySelect->maxSelections[PARTY_SELECT_OPPONENT] = maxSelect;
    sPartySelect->cursorSide = PARTY_SELECT_PLAYER;
    sPartySelect->cursorSlot = 0;
    sPartySelect->exitCallback = (config->exitCallback != NULL) ? config->exitCallback : CB2_ReturnToFieldContinueScriptPlayMapMusic;
    sPartySelect->fromScript = config->fromScript;
    sPartySelect->confirmed = FALSE;

    sPartySelect->parties[PARTY_SELECT_PLAYER] = config->playerParty;
    sPartySelect->partyCount[PARTY_SELECT_PLAYER] = playerCount;

    if (opponentCount > 0)
        memcpy(sPartySelect->opponentParty, config->opponentParty, sizeof(struct Pokemon) * opponentCount);

    sPartySelect->parties[PARTY_SELECT_OPPONENT] = sPartySelect->opponentParty;
    sPartySelect->partyCount[PARTY_SELECT_OPPONENT] = opponentCount;

    if (config->playerName != NULL)
        StringCopy(sPartySelect->trainerNames[PARTY_SELECT_PLAYER], config->playerName);
    else
        StringCopy(sPartySelect->trainerNames[PARTY_SELECT_PLAYER], gSaveBlock2Ptr->playerName);

    if (config->opponentName != NULL)
        StringCopy(sPartySelect->trainerNames[PARTY_SELECT_OPPONENT], config->opponentName);
    else
        StringCopy(sPartySelect->trainerNames[PARTY_SELECT_OPPONENT], COMPOUND_STRING("OPPONENT"));

    PartySelect_FindFirstSelectable(PARTY_SELECT_PLAYER, &sPartySelect->cursorSlot);
    memset(gSelectedOrderFromParty, 0, sizeof(gSelectedOrderFromParty));

    SetMainCallback2(PartySelect_Init);
    return TRUE;
}

static void PartySelect_Init(void)
{
    switch (sPartySelect->state)
    {
    case 0:
        SetVBlankCallback(NULL);
        ResetBgsAndClearDma3BusyFlags(0);
        ResetTasks();
        ResetSpriteData();
        FreeAllSpritePalettes();
        ResetPaletteFade();
        sPartySelect->state++;
        break;
    case 1:
        InitBgsFromTemplates(0, sPartySelectBgTemplates, ARRAY_COUNT(sPartySelectBgTemplates));
        InitWindows(sPartySelectWindowTemplates);
        DeactivateAllTextPrinters();
        LoadUserWindowBorderGfx(0, 1, BG_PLTT_ID(15));
        LoadMessageBoxAndBorderGfx();
        LoadMonIconPalettes();
        sPartySelect->windowIds[WIN_PLAYER_PANEL] = AddWindow(&sPartySelectWindowTemplates[WIN_PLAYER_PANEL]);
        sPartySelect->windowIds[WIN_OPP_PANEL] = AddWindow(&sPartySelectWindowTemplates[WIN_OPP_PANEL]);
        sPartySelect->windowIds[WIN_FOOTER] = AddWindow(&sPartySelectWindowTemplates[WIN_FOOTER]);
        sPartySelect->state++;
        break;
    case 2:
        PartySelect_DrawPanels();
        PartySelect_DrawFooter();
        PartySelect_CreateIcons();
        ShowBg(0);
        BlendPalettes(0xFFFFFFFF, 16, RGB_BLACK);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB_PartySelect);
        SetMainCallback2(CB2_PartySelect);
        sPartySelect->state++;
        break;
    }
}

static void VBlankCB_PartySelect(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_PartySelect(void)
{
    RunTasks();
    PartySelect_HandleInput();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();

    if (sPartySelect->fadeOut && !gPaletteFade.active)
    {
        void (*exitCallback)(void) = sPartySelect->exitCallback;

        if (sPartySelect->fromScript)
            gSpecialVar_Result = sPartySelect->confirmed;

        FreePartySelectResources();
        if (exitCallback != NULL)
            SetMainCallback2(exitCallback);
    }
}

static void FreePartySelectResources(void)
{
    SetVBlankCallback(NULL);
    PartySelect_DestroyIcons();
    FreeMonIconPalettes();
    FreeAllWindowBuffers();
    sPartySelect->state = 0;
    Free(sPartySelect);
    sPartySelect = NULL;
}

static void DrawRectOutline(u8 windowId, u8 x, u8 y, u8 width, u8 height, u8 color)
{
    FillWindowPixelRect(windowId, color, x, y, width, 1);
    FillWindowPixelRect(windowId, color, x, y + height - 1, width, 1);
    FillWindowPixelRect(windowId, color, x, y, 1, height);
    FillWindowPixelRect(windowId, color, x + width - 1, y, 1, height);
}

static void PartySelect_DrawPanels(void)
{
    u8 i;

    for (i = 0; i < 2; i++)
        PartySelect_DrawPanel(i);
}

static void PartySelect_DrawPanel(u8 side)
{
    u8 windowId = sPartySelect->windowIds[side];
    u8 i;

    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    DrawStdWindowFrame(windowId, FALSE);

    // Header bar inside panel
    FillWindowPixelRect(windowId, PIXEL_FILL(2), 2, 2, PANEL_PIXEL_WIDTH - 4, 12);
    DrawRectOutline(windowId, 2, 2, PANEL_PIXEL_WIDTH - 4, 12, PIXEL_FILL(15));
    AddTextPrinterParameterized4(windowId, FONT_NORMAL, 6, 4, 0, 0, sTextColor_Header, 0, sPartySelect->trainerNames[side]);

    // Draw slots
    for (i = 0; i < PARTY_SELECT_SLOTS; i++)
        PartySelect_DrawSlot(side, i);

    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

static void PartySelect_DrawFooter(void)
{
    u8 windowId = sPartySelect->windowIds[WIN_FOOTER];
    static const u8 sFooterText[] = _("A:Select  B:Back  START:Battle");

    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    DrawStdWindowFrame(windowId, FALSE);
    AddTextPrinterParameterized4(windowId, FONT_NORMAL, 4, 4, 0, 0, sTextColor_Normal, 0, sFooterText);
    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

static void PartySelect_DrawSlot(u8 side, u8 slot)
{
    u8 windowId = sPartySelect->windowIds[side];
    struct Pokemon *party = sPartySelect->parties[side];
    u8 x = sSlotPosX[slot];
    u8 y = sSlotPosY[slot];
    u16 species;
    u8 textX, textY;
    u8 fillColor;
    u8 level;
    u8 nickname[POKEMON_NAME_LENGTH + 1];
    const u8 *colors;
    bool32 isCursor = (sPartySelect->cursorSide == side && sPartySelect->cursorSlot == slot);
    struct PartySelectSlot *slotState = &sPartySelect->slots[side][slot];

    if (slot >= sPartySelect->partyCount[side])
    {
        slotState->canSelect = FALSE;
        FillWindowPixelRect(windowId, PIXEL_FILL(1), x, y, SLOT_WIDTH, SLOT_HEIGHT);
        DrawRectOutline(windowId, x, y, SLOT_WIDTH, SLOT_HEIGHT, PIXEL_FILL(15));
        CopyWindowToVram(windowId, COPYWIN_GFX);
        return;
    }

    species = GetMonData(&party[slot], MON_DATA_SPECIES_OR_EGG, NULL);
    if (species == SPECIES_NONE)
    {
        slotState->canSelect = FALSE;
        FillWindowPixelRect(windowId, PIXEL_FILL(1), x, y, SLOT_WIDTH, SLOT_HEIGHT);
        DrawRectOutline(windowId, x, y, SLOT_WIDTH, SLOT_HEIGHT, PIXEL_FILL(15));
        CopyWindowToVram(windowId, COPYWIN_GFX);
        return;
    }

    slotState->canSelect = PartySelect_IsSlotSelectable(side, slot);

    if (slotState->selected)
        fillColor = PIXEL_FILL(4);
    else if (isCursor)
        fillColor = PIXEL_FILL(2);
    else
        fillColor = PIXEL_FILL(1);

    FillWindowPixelRect(windowId, fillColor, x, y, SLOT_WIDTH, SLOT_HEIGHT);
    DrawRectOutline(windowId, x, y, SLOT_WIDTH, SLOT_HEIGHT, PIXEL_FILL(15));

    level = GetMonData(&party[slot], MON_DATA_LEVEL, NULL);
    GetMonData(&party[slot], MON_DATA_NICKNAME, nickname);
    StringGet_Nickname(nickname);

    textX = x + 4;
    textY = y + 2;

    if (GetMonData(&party[slot], MON_DATA_HP, NULL) == 0)
        colors = sTextColor_Fainted;
    else if (slotState->selected)
        colors = sTextColor_Highlight;
    else
        colors = sTextColor_Normal;

    {
        u8 buffer[16];
        StringCopy(buffer, gText_Lv);
        ConvertIntToDecimalStringN(buffer + StringLength(buffer), level, STR_CONV_MODE_LEFT_ALIGN, 3);
        AddTextPrinterParameterized4(windowId, FONT_SMALL, textX, textY, 0, 0, colors, 0, buffer);
    }

    AddTextPrinterParameterized4(windowId, FONT_NORMAL, textX, textY + 14, 0, 0, colors, 0, nickname);

    if (slotState->selected && slotState->order > 0)
    {
        u8 orderText[4];
        ConvertIntToDecimalStringN(orderText, slotState->order, STR_CONV_MODE_LEFT_ALIGN, 1);
        AddTextPrinterParameterized4(windowId, FONT_SMALL, x + SLOT_WIDTH - 10, y + 2, 0, 0, colors, 0, orderText);
    }

    CopyWindowToVram(windowId, COPYWIN_GFX);
}

static void PartySelect_CreateIcons(void)
{
    s32 side, i;

    for (side = 0; side < 2; side++)
    {
        for (i = 0; i < PARTY_SIZE; i++)
        {
            struct Pokemon *mon = &sPartySelect->parties[side][i];
            u16 species = GetMonData(mon, MON_DATA_SPECIES_OR_EGG, NULL);
            u8 spriteId = MAX_SPRITES;
            s16 x, y;

            sPartySelect->slots[side][i].iconSpriteId = MAX_SPRITES;
            sPartySelect->slots[side][i].selected = FALSE;
            sPartySelect->slots[side][i].order = 0;

            if (i >= sPartySelect->partyCount[side])
                continue;

            if (species == SPECIES_NONE)
                continue;

            x = (sPartySelectWindowTemplates[side].tilemapLeft * 8) + sSlotPosX[i] + 8;
            y = (sPartySelectWindowTemplates[side].tilemapTop * 8) + sSlotPosY[i] + 12;

            LoadMonIconPalette(species);
            spriteId = CreateMonIcon(species, SpriteCallbackDummy, x, y, 1, GetMonData(mon, MON_DATA_PERSONALITY, NULL));
            sPartySelect->slots[side][i].iconSpriteId = spriteId;
        }
    }
}

static void PartySelect_DestroyIcons(void)
{
    s32 side, i;
    for (side = 0; side < 2; side++)
    {
        for (i = 0; i < PARTY_SIZE; i++)
        {
            if (sPartySelect->slots[side][i].iconSpriteId != MAX_SPRITES)
                FreeAndDestroyMonIconSprite(&gSprites[sPartySelect->slots[side][i].iconSpriteId]);
        }
    }
}

static void PartySelect_HandleInput(void)
{
    if (gPaletteFade.active)
        return;

    if (JOY_NEW(B_BUTTON))
    {
        sPartySelect->confirmed = FALSE;
        sPartySelect->fadeOut = TRUE;
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        return;
    }

    if (JOY_NEW(START_BUTTON) || JOY_NEW(SELECT_BUTTON))
    {
        PartySelect_AttemptConfirm();
        return;
    }

    if (JOY_NEW(A_BUTTON))
    {
        PartySelect_ToggleSelection();
        return;
    }

    if (JOY_NEW(DPAD_UP))
        PartySelect_MoveCursor(0, -1);
    else if (JOY_NEW(DPAD_DOWN))
        PartySelect_MoveCursor(0, 1);
    else if (JOY_NEW(DPAD_LEFT))
        PartySelect_MoveCursor(-1, 0);
    else if (JOY_NEW(DPAD_RIGHT))
        PartySelect_MoveCursor(1, 0);
}

static u8 PartySelect_GetSlotIndex(u8 row, u8 col)
{
    return row * PARTY_SELECT_COLS + col;
}

static void PartySelect_MoveCursor(s8 deltaX, s8 deltaY)
{
    u8 row = sPartySelect->cursorSlot / PARTY_SELECT_COLS;
    u8 col = sPartySelect->cursorSlot % PARTY_SELECT_COLS;
    u8 side = sPartySelect->cursorSide;
    u8 newSide = side;
    u8 oldSide = side;
    u8 oldSlot = sPartySelect->cursorSlot;

    if (deltaY != 0)
    {
        s8 newRow = row + deltaY;
        if (newRow < 0)
            newRow = 0;
        if (newRow >= PARTY_SELECT_ROWS)
            newRow = PARTY_SELECT_ROWS - 1;
        row = newRow;
    }

    if (deltaX != 0)
    {
        s8 newCol = col + deltaX;
        if (newCol < 0)
        {
            if (side == PARTY_SELECT_OPPONENT)
            {
                newSide = PARTY_SELECT_PLAYER;
                newCol = PARTY_SELECT_COLS - 1;
            }
            else
            {
                newCol = 0;
            }
        }
        else if (newCol >= PARTY_SELECT_COLS)
        {
            if (side == PARTY_SELECT_PLAYER)
            {
                newSide = PARTY_SELECT_OPPONENT;
                newCol = 0;
            }
            else
            {
                newCol = PARTY_SELECT_COLS - 1;
            }
        }
        col = newCol;
    }

    {
        u8 newSlot = PartySelect_GetSlotIndex(row, col);

        if (!PartySelect_IsSlotSelectable(newSide, newSlot))
        {
            if (!PartySelect_FindFirstSelectable(newSide, &newSlot))
            {
                newSide = oldSide;
                newSlot = oldSlot;
            }
        }

        sPartySelect->cursorSide = newSide;
        sPartySelect->cursorSlot = newSlot;
    }

    PartySelect_DrawPanel(PARTY_SELECT_PLAYER);
    PartySelect_DrawPanel(PARTY_SELECT_OPPONENT);
}

static bool32 PartySelect_IsSlotSelectable(u8 side, u8 slot)
{
    struct Pokemon *party = sPartySelect->parties[side];
    if (slot >= sPartySelect->partyCount[side])
        return FALSE;
    if (GetMonData(&party[slot], MON_DATA_SPECIES_OR_EGG, NULL) == SPECIES_NONE)
        return FALSE;
    if (GetMonData(&party[slot], MON_DATA_HP, NULL) == 0)
        return FALSE;
    return TRUE;
}

static void PartySelect_ToggleSelection(void)
{
    u8 side = sPartySelect->cursorSide;
    u8 slot = sPartySelect->cursorSlot;
    struct PartySelectSlot *slotState = &sPartySelect->slots[side][slot];
    u8 maxSelect = sPartySelect->maxSelections[side];

    if (!PartySelect_IsSlotSelectable(side, slot))
        return;

    if (!slotState->selected)
    {
        if (sPartySelect->selections[side] >= maxSelect)
            return;

        sPartySelect->selections[side]++;
        slotState->selected = TRUE;
        slotState->order = sPartySelect->selections[side];
    }
    else
    {
        PartySelect_ReorderSelections(side, slotState->order);
        slotState->selected = FALSE;
        slotState->order = 0;
        if (sPartySelect->selections[side] > 0)
            sPartySelect->selections[side]--;
    }

    PartySelect_DrawPanel(PARTY_SELECT_PLAYER);
    PartySelect_DrawPanel(PARTY_SELECT_OPPONENT);
}

static void PartySelect_ReorderSelections(u8 side, u8 removedOrder)
{
    s32 i;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct PartySelectSlot *slotState = &sPartySelect->slots[side][i];
        if (slotState->selected && slotState->order > removedOrder)
            slotState->order--;
    }
}

static void PartySelect_AttemptConfirm(void)
{
    if (sPartySelect->selections[PARTY_SELECT_PLAYER] < sPartySelect->maxSelections[PARTY_SELECT_PLAYER])
        return;

    sPartySelect->confirmed = TRUE;
    PartySelect_SavePlayerSelection();
    sPartySelect->fadeOut = TRUE;
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
}

static void PartySelect_SavePlayerSelection(void)
{
    s32 i;

    memset(gSelectedOrderFromParty, 0, sizeof(gSelectedOrderFromParty));

    for (i = 0; i < sPartySelect->partyCount[PARTY_SELECT_PLAYER]; i++)
    {
        u8 order = sPartySelect->slots[PARTY_SELECT_PLAYER][i].order;
        if (order > 0 && order <= MAX_FRONTIER_PARTY_SIZE)
            gSelectedOrderFromParty[order - 1] = i + 1;
    }
}

static bool32 PartySelect_FindFirstSelectable(u8 side, u8 *slotOut)
{
    u8 i;
    for (i = 0; i < sPartySelect->partyCount[side]; i++)
    {
        if (PartySelect_IsSlotSelectable(side, i))
        {
            *slotOut = i;
            return TRUE;
        }
    }
    return FALSE;
}
