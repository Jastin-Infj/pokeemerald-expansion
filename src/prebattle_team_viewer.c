#include "global.h"
#include "battle.h"
#include "battle_controllers.h"
#include "battle_main.h"
#include "battle_setup.h"
#include "bg.h"
#include "config/battle.h"
#include "gpu_regs.h"
#include "item.h"
#include "main.h"
#include "menu.h"
#include "move.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "prebattle_team_viewer.h"
#include "reshow_battle_screen.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/battle_setup.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "constants/trainers.h"

#define TEAM_VIEWER_GRID_COLUMNS 3
#define TEAM_VIEWER_WINDOW_TILE_SIZE 8
#define TEAM_VIEWER_BORDER_PIXELS 2
#define TEAM_VIEWER_ICON_PLAYER_X 18
#define TEAM_VIEWER_ICON_OPPONENT_X 138
#define TEAM_VIEWER_ICON_COLUMN_SPACING 40
#define TEAM_VIEWER_ICON_TOP_Y 42
#define TEAM_VIEWER_ICON_ROW_SPACING 38
#define TEAM_VIEWER_LABEL_X 2
#define TEAM_VIEWER_LABEL_TOP_Y 44
#define TEAM_VIEWER_LABEL_ROW_SPACING 38
#define TEAM_VIEWER_PANEL_TEXT_CLEAR_Y 2

enum TeamViewerMode
{
    TEAM_VIEWER_MODE_PRE_BATTLE,
    TEAM_VIEWER_MODE_IN_BATTLE,
};

enum TeamViewerSide
{
    TEAM_VIEWER_SIDE_PLAYER,
    TEAM_VIEWER_SIDE_OPPONENT,
};

enum TeamViewerWindow
{
    WIN_TITLE,
    WIN_PLAYER,
    WIN_OPPONENT,
    WIN_FOOTER,
    WIN_COUNT,
};

struct PreBattleTeamViewerState
{
    bool8 cacheActive;
    bool8 battleStarted;
    bool8 iconSpritesInitialized;
    enum TeamViewerMode mode;
    enum TeamViewerSide side;
    bool8 showDetails;
    u8 cursor;
    u8 selectedCount;
    u8 playerCount;
    u8 enemyPartyCount;
    u16 trainerId;
    u32 battleTypeFlags;
    u16 opponentMonCanTera;
    u16 opponentMonCanDynamax;
    MainCallback callback;
    MainCallback callback1;
    struct Pokemon enemyParty[PARTY_SIZE];
    u8 iconSpriteIds[2][PARTY_SIZE];
};

static EWRAM_DATA struct PreBattleTeamViewerState sTeamViewerState = {0};

static void CB2_PreBattleTeamViewer(void);
static void VBlankCB_PreBattleTeamViewer(void);
static void DrawTeamViewer(void);
static void DrawHeader(void);
static void DrawTeamPanel(u8 windowId, struct Pokemon *party, u8 count, enum TeamViewerSide side);
static void DrawTeamLabels(u8 windowId, struct Pokemon *party, u8 count, enum TeamViewerSide side);
static void DrawFooter(void);
static void DrawSelectedMonDetails(void);
static void RefreshTeamCursor(bool8 redrawFooter);
static void DrawThinFrame(u8 windowId);
static void BuildMonIconLabel(u8 *dest, struct Pokemon *mon, u8 slot);
static void BuildBattleActionPrompt(u8 *dest);
static void BuildTypeText(u8 *dest, u16 species);
static void BuildHeldItemText(u8 *dest, enum Item item);
static void BuildMoveText(u8 *dest, struct Pokemon *mon, u8 moveSlot);
static struct Pokemon *GetSelectedMon(void);
static u8 GetCurrentSideCount(void);
static bool32 PrepareOpponentCache(void);
static bool32 IsEligibleInBattleViewer(void);
static void InitTeamIconSpriteIds(void);
static void DestroyTeamIcons(void);
static void CreateTeamIcons(void);
static void CreateTeamIcon(struct Pokemon *party, enum TeamViewerSide side, u8 slot);
static void CloseTeamViewer(void);
static void FreeTeamViewerWindows(void);
static void PrintText(u8 windowId, const u8 *str, u8 x, u8 y);

static const struct BgTemplate sBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .priority = 2,
    },
};

static const struct WindowTemplate sWindowTemplates[] =
{
    [WIN_TITLE] =
    {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 30,
        .height = 2,
        .paletteNum = STD_WINDOW_PALETTE_NUM,
        .baseBlock = 0x001,
    },
    [WIN_PLAYER] =
    {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 2,
        .width = 15,
        .height = 12,
        .paletteNum = STD_WINDOW_PALETTE_NUM,
        .baseBlock = 0x03D,
    },
    [WIN_OPPONENT] =
    {
        .bg = 0,
        .tilemapLeft = 15,
        .tilemapTop = 2,
        .width = 15,
        .height = 12,
        .paletteNum = STD_WINDOW_PALETTE_NUM,
        .baseBlock = 0x100,
    },
    [WIN_FOOTER] =
    {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 14,
        .width = 30,
        .height = 6,
        .paletteNum = STD_WINDOW_PALETTE_NUM,
        .baseBlock = 0x1C3,
    },
    DUMMY_WIN_TEMPLATE,
};

static const u8 sTextColor[] = { TEXT_COLOR_TRANSPARENT, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY };
static const u8 sTitleText[] = _("TEAM VIEWER");
static const u8 sPlayerText[] = _("YOUR TEAM");
static const u8 sOpponentText[] = _("OPPONENT");
static const u8 sHiddenText[] = _("Opponent details hidden");

bool32 PreBattleTeamViewer_Begin(u8 selectedCount, MainCallback callback)
{
#if B_PREBATTLE_TEAM_VIEWER
    if (!PrepareOpponentCache())
        return FALSE;

    sTeamViewerState.mode = TEAM_VIEWER_MODE_PRE_BATTLE;
    sTeamViewerState.side = TEAM_VIEWER_SIDE_OPPONENT;
    sTeamViewerState.cursor = 0;
    sTeamViewerState.showDetails = FALSE;
    sTeamViewerState.selectedCount = selectedCount;
    sTeamViewerState.playerCount = CalculatePlayerPartyCount();
    sTeamViewerState.callback = callback;
    SetMainCallback2(CB2_PreBattleTeamViewer);
    return TRUE;
#else
    return FALSE;
#endif
}

bool32 PreBattleTeamViewer_Reopen(u8 selectedCount, MainCallback callback)
{
#if B_PREBATTLE_TEAM_VIEWER
    if (!sTeamViewerState.cacheActive)
        return FALSE;

    sTeamViewerState.mode = TEAM_VIEWER_MODE_PRE_BATTLE;
    sTeamViewerState.side = TEAM_VIEWER_SIDE_OPPONENT;
    sTeamViewerState.cursor = 0;
    sTeamViewerState.showDetails = FALSE;
    sTeamViewerState.selectedCount = selectedCount;
    sTeamViewerState.playerCount = CalculatePlayerPartyCount();
    sTeamViewerState.callback = callback;
    InitTeamIconSpriteIds();
    SetMainCallback2(CB2_PreBattleTeamViewer);
    return TRUE;
#else
    return FALSE;
#endif
}

bool32 PreBattleTeamViewer_LoadCachedOpponentParty(void)
{
    if (!sTeamViewerState.cacheActive)
        return FALSE;
    if (sTeamViewerState.trainerId != TRAINER_BATTLE_PARAM.opponentA)
        return FALSE;
    if ((sTeamViewerState.battleTypeFlags & BATTLE_TYPE_DOUBLE) != (gBattleTypeFlags & BATTLE_TYPE_DOUBLE))
        return FALSE;

    memcpy(gEnemyParty, sTeamViewerState.enemyParty, sizeof(sTeamViewerState.enemyParty));
    if (gBattleStruct != NULL)
    {
        gBattleStruct->opponentMonCanTera = sTeamViewerState.opponentMonCanTera;
        gBattleStruct->opponentMonCanDynamax = sTeamViewerState.opponentMonCanDynamax;
    }
    sTeamViewerState.battleStarted = TRUE;
    return TRUE;
}

bool32 PreBattleTeamViewer_TryOpenInBattle(u32 battler)
{
#if B_IN_BATTLE_TEAM_VIEWER
    if (!IsEligibleInBattleViewer())
        return FALSE;

    gBattlerInMenuId = battler;
    sTeamViewerState.mode = TEAM_VIEWER_MODE_IN_BATTLE;
    sTeamViewerState.side = TEAM_VIEWER_SIDE_OPPONENT;
    sTeamViewerState.cursor = 0;
    sTeamViewerState.showDetails = FALSE;
    sTeamViewerState.playerCount = CalculatePlayerPartyCount();
    sTeamViewerState.callback = CB2_ReturnToChooseActionFromTeamViewer;
    sTeamViewerState.callback1 = gMain.callback1;
    gMain.callback1 = NULL;
    SetMainCallback2(CB2_PreBattleTeamViewer);
    return TRUE;
#else
    return FALSE;
#endif
}

void PreBattleTeamViewer_RestoreBattleCallback1(void)
{
    if (sTeamViewerState.callback1 == NULL)
        return;

    gMain.callback1 = sTeamViewerState.callback1;
    sTeamViewerState.callback1 = NULL;
}

void PreBattleTeamViewer_Clear(void)
{
    PreBattleTeamViewer_RestoreBattleCallback1();
    DestroyTeamIcons();
    memset(&sTeamViewerState, 0, sizeof(sTeamViewerState));
}

static bool32 PrepareOpponentCache(void)
{
    u8 partyCount;

    memset(&sTeamViewerState, 0, sizeof(sTeamViewerState));
    InitTeamIconSpriteIds();
    partyCount = CreateNPCTrainerPartyForPreview(sTeamViewerState.enemyParty,
                                                 TRAINER_BATTLE_PARAM.opponentA,
                                                 gBattleTypeFlags,
                                                 &sTeamViewerState.opponentMonCanTera,
                                                 &sTeamViewerState.opponentMonCanDynamax);
    if (partyCount == 0)
        return FALSE;

    sTeamViewerState.cacheActive = TRUE;
    sTeamViewerState.enemyPartyCount = CalculatePartyCount(sTeamViewerState.enemyParty);
    sTeamViewerState.trainerId = TRAINER_BATTLE_PARAM.opponentA;
    sTeamViewerState.battleTypeFlags = gBattleTypeFlags;
    return TRUE;
}

static bool32 IsEligibleInBattleViewer(void)
{
    if (!sTeamViewerState.cacheActive)
        return FALSE;
    if (!(gBattleTypeFlags & BATTLE_TYPE_TRAINER))
        return FALSE;
    if (gBattleTypeFlags & (BATTLE_TYPE_LINK
                          | BATTLE_TYPE_FRONTIER
                          | BATTLE_TYPE_MULTI
                          | BATTLE_TYPE_INGAME_PARTNER
                          | BATTLE_TYPE_TWO_OPPONENTS
                          | BATTLE_TYPE_PYRAMID
                          | BATTLE_TYPE_TRAINER_HILL
                          | BATTLE_TYPE_SECRET_BASE
                          | BATTLE_TYPE_RECORDED
                          | BATTLE_TYPE_RECORDED_LINK))
        return FALSE;
    return TRUE;
}

static void CB2_PreBattleTeamViewer(void)
{
    switch (gMain.state)
    {
    case 0:
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        SetHBlankCallback(NULL);
        SetVBlankCallback(NULL);
        ScanlineEffect_Stop();
        ResetPaletteFade();
        gPaletteFade.bufferTransferDisabled = FALSE;
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        FreeAllWindowBuffers();
        ResetSpriteData();
        FreeAllSpritePalettes();
        LoadMonIconPalettes();
        ResetTasks();
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
        ChangeBgX(0, 0, BG_COORD_SET);
        ChangeBgY(0, 0, BG_COORD_SET);
        SetGpuReg(REG_OFFSET_BG0HOFS, 0);
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
        ResetTempTileDataBuffers();
        if (!InitWindows(sWindowTemplates))
            return;
        DeactivateAllTextPrinters();
        FillBgTilemapBufferRect(0, 0, 0, 0, DISPLAY_TILE_WIDTH, DISPLAY_TILE_HEIGHT, STD_WINDOW_PALETTE_NUM);
        Menu_LoadStdPal();
        InitTeamIconSpriteIds();
        SetVBlankCallback(VBlankCB_PreBattleTeamViewer);
        gMain.state++;
        break;
    case 1:
        DrawTeamViewer();
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        ShowBg(0);
        gMain.state++;
        break;
    case 2:
        if (!UpdatePaletteFade())
            gMain.state++;
        break;
    case 3:
        RunTasks();
        AnimateSprites();
        BuildOamBuffer();
        if (sTeamViewerState.mode == TEAM_VIEWER_MODE_IN_BATTLE)
        {
            if (JOY_NEW(A_BUTTON | B_BUTTON | B_TEAM_VIEWER_BUTTON))
            {
                PlaySE(SE_SELECT);
                BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
                gMain.state++;
            }
            break;
        }
        if (JOY_NEW(DPAD_UP))
        {
            if (sTeamViewerState.cursor >= TEAM_VIEWER_GRID_COLUMNS)
            {
                bool8 redrawFooter = sTeamViewerState.showDetails;

                sTeamViewerState.cursor -= TEAM_VIEWER_GRID_COLUMNS;
                PlaySE(SE_SELECT);
                RefreshTeamCursor(redrawFooter);
            }
        }
        else if (JOY_NEW(DPAD_DOWN))
        {
            if (sTeamViewerState.cursor + TEAM_VIEWER_GRID_COLUMNS < GetCurrentSideCount())
            {
                bool8 redrawFooter = sTeamViewerState.showDetails;

                sTeamViewerState.cursor += TEAM_VIEWER_GRID_COLUMNS;
                PlaySE(SE_SELECT);
                RefreshTeamCursor(redrawFooter);
            }
        }
        else if (JOY_NEW(DPAD_LEFT))
        {
            if (sTeamViewerState.cursor % TEAM_VIEWER_GRID_COLUMNS != 0)
            {
                bool8 redrawFooter = sTeamViewerState.showDetails;

                sTeamViewerState.cursor--;
                PlaySE(SE_SELECT);
                RefreshTeamCursor(redrawFooter);
            }
            else
            {
                bool8 redrawFooter = sTeamViewerState.showDetails;

                sTeamViewerState.side ^= 1;
                if (sTeamViewerState.cursor >= GetCurrentSideCount())
                    sTeamViewerState.cursor = GetCurrentSideCount() == 0 ? 0 : GetCurrentSideCount() - 1;
                PlaySE(SE_SELECT);
                RefreshTeamCursor(redrawFooter);
            }
        }
        else if (JOY_NEW(DPAD_RIGHT))
        {
            if (sTeamViewerState.cursor % TEAM_VIEWER_GRID_COLUMNS != TEAM_VIEWER_GRID_COLUMNS - 1
             && sTeamViewerState.cursor + 1 < GetCurrentSideCount())
            {
                bool8 redrawFooter = sTeamViewerState.showDetails;

                sTeamViewerState.cursor++;
                PlaySE(SE_SELECT);
                RefreshTeamCursor(redrawFooter);
            }
            else
            {
                bool8 redrawFooter = sTeamViewerState.showDetails;

                sTeamViewerState.side ^= 1;
                if (sTeamViewerState.cursor >= GetCurrentSideCount())
                    sTeamViewerState.cursor = GetCurrentSideCount() == 0 ? 0 : GetCurrentSideCount() - 1;
                PlaySE(SE_SELECT);
                RefreshTeamCursor(redrawFooter);
            }
        }
        else if (JOY_NEW(B_TEAM_VIEWER_DETAILS_BUTTON))
        {
            sTeamViewerState.showDetails ^= 1;
            PlaySE(SE_SELECT);
            DrawFooter();
        }
        else if (JOY_NEW(A_BUTTON))
        {
            PlaySE(SE_SELECT);
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            gMain.state++;
        }
        break;
    case 4:
        RunTasks();
        AnimateSprites();
        BuildOamBuffer();
        if (!UpdatePaletteFade())
            CloseTeamViewer();
        break;
    }
}

static void VBlankCB_PreBattleTeamViewer(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CloseTeamViewer(void)
{
    MainCallback callback = sTeamViewerState.callback;

    DestroyTeamIcons();
    FreeMonIconPalettes();
    FreeTeamViewerWindows();
    SetVBlankCallback(NULL);
    if (callback == NULL)
        callback = CB2_ReturnToField;
    SetMainCallback2(callback);
}

static void FreeTeamViewerWindows(void)
{
    u8 i;

    for (i = 0; i < WIN_COUNT; i++)
    {
        ClearStdWindowAndFrame(i, FALSE);
        ClearWindowTilemap(i);
        RemoveWindow(i);
    }
    FreeAllWindowBuffers();
}

static void DrawTeamViewer(void)
{
    DrawHeader();
    DrawTeamPanel(WIN_PLAYER, gPlayerParty, sTeamViewerState.playerCount, TEAM_VIEWER_SIDE_PLAYER);
    DrawTeamPanel(WIN_OPPONENT, sTeamViewerState.enemyParty, sTeamViewerState.enemyPartyCount, TEAM_VIEWER_SIDE_OPPONENT);
    DrawFooter();
    CopyBgTilemapBufferToVram(0);
    CreateTeamIcons();
}

static void DrawHeader(void)
{
    DrawThinFrame(WIN_TITLE);
    PutWindowTilemap(WIN_TITLE);
    PrintText(WIN_TITLE, sPlayerText, 5, 2);
    PrintText(WIN_TITLE, sTitleText, 87, 2);
    PrintText(WIN_TITLE, sOpponentText, 186, 2);
    CopyWindowToVram(WIN_TITLE, COPYWIN_FULL);
}

static void DrawTeamPanel(u8 windowId, struct Pokemon *party, u8 count, enum TeamViewerSide side)
{
    DrawThinFrame(windowId);
    PutWindowTilemap(windowId);
    DrawTeamLabels(windowId, party, count, side);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

static void DrawTeamLabels(u8 windowId, struct Pokemon *party, u8 count, enum TeamViewerSide side)
{
    u8 i;
    u16 width = GetWindowAttribute(windowId, WINDOW_WIDTH) * TEAM_VIEWER_WINDOW_TILE_SIZE;
    u16 height = GetWindowAttribute(windowId, WINDOW_HEIGHT) * TEAM_VIEWER_WINDOW_TILE_SIZE;

    FillWindowPixelRect(windowId,
                        PIXEL_FILL(1),
                        TEAM_VIEWER_BORDER_PIXELS,
                        TEAM_VIEWER_PANEL_TEXT_CLEAR_Y,
                        width - TEAM_VIEWER_BORDER_PIXELS * 2,
                        height - TEAM_VIEWER_PANEL_TEXT_CLEAR_Y - TEAM_VIEWER_BORDER_PIXELS);

    for (i = 0; i < PARTY_SIZE; i++)
    {
        u8 col = i % TEAM_VIEWER_GRID_COLUMNS;
        u8 row = i / TEAM_VIEWER_GRID_COLUMNS;
        u8 x = TEAM_VIEWER_LABEL_X + col * TEAM_VIEWER_ICON_COLUMN_SPACING;
        u8 y = TEAM_VIEWER_LABEL_TOP_Y + row * TEAM_VIEWER_LABEL_ROW_SPACING;

        if (i >= count || GetMonData(&party[i], MON_DATA_SPECIES_OR_EGG) == SPECIES_NONE)
        {
            if (sTeamViewerState.mode == TEAM_VIEWER_MODE_IN_BATTLE)
            {
                gStringVar4[0] = EOS;
            }
            else
            {
                ConvertIntToDecimalStringN(gStringVar4, i + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
                StringAppend(gStringVar4, COMPOUND_STRING(" -"));
            }
        }
        else
        {
            BuildMonIconLabel(gStringVar4, &party[i], i);
        }

        if (sTeamViewerState.mode == TEAM_VIEWER_MODE_PRE_BATTLE
         && sTeamViewerState.side == side
         && sTeamViewerState.cursor == i)
            PrintText(windowId, COMPOUND_STRING(">"), x, y);
        PrintText(windowId, gStringVar4, x + 5, y);
    }
}

static void DrawFooter(void)
{
    DrawThinFrame(WIN_FOOTER);
    PutWindowTilemap(WIN_FOOTER);
    if (sTeamViewerState.showDetails)
        DrawSelectedMonDetails();
    else
    {
        BuildBattleActionPrompt(gStringVar4);
        PrintText(WIN_FOOTER, gStringVar4, 8, 4);
        if (sTeamViewerState.mode == TEAM_VIEWER_MODE_PRE_BATTLE)
        {
            PrintText(WIN_FOOTER, COMPOUND_STRING("A: Choose  Sel: Strength"), 8, 19);
            PrintText(WIN_FOOTER, COMPOUND_STRING("D-Pad: Cursor"), 8, 34);
        }
        else
        {
            PrintText(WIN_FOOTER, COMPOUND_STRING("A/B/R: Back"), 8, 19);
        }
    }
    CopyWindowToVram(WIN_FOOTER, COPYWIN_FULL);
}

static void RefreshTeamCursor(bool8 redrawFooter)
{
    DrawTeamLabels(WIN_PLAYER, gPlayerParty, sTeamViewerState.playerCount, TEAM_VIEWER_SIDE_PLAYER);
    CopyWindowToVram(WIN_PLAYER, COPYWIN_GFX);
    DrawTeamLabels(WIN_OPPONENT, sTeamViewerState.enemyParty, sTeamViewerState.enemyPartyCount, TEAM_VIEWER_SIDE_OPPONENT);
    CopyWindowToVram(WIN_OPPONENT, COPYWIN_GFX);
    if (redrawFooter)
        DrawFooter();
}

static void DrawThinFrame(u8 windowId)
{
    u16 width = GetWindowAttribute(windowId, WINDOW_WIDTH) * TEAM_VIEWER_WINDOW_TILE_SIZE;
    u16 height = GetWindowAttribute(windowId, WINDOW_HEIGHT) * TEAM_VIEWER_WINDOW_TILE_SIZE;

    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    FillWindowPixelRect(windowId, PIXEL_FILL(2), 0, 0, width, TEAM_VIEWER_BORDER_PIXELS);
    FillWindowPixelRect(windowId, PIXEL_FILL(2), 0, height - TEAM_VIEWER_BORDER_PIXELS, width, TEAM_VIEWER_BORDER_PIXELS);
    FillWindowPixelRect(windowId, PIXEL_FILL(2), 0, 0, TEAM_VIEWER_BORDER_PIXELS, height);
    FillWindowPixelRect(windowId, PIXEL_FILL(2), width - TEAM_VIEWER_BORDER_PIXELS, 0, TEAM_VIEWER_BORDER_PIXELS, height);
}

static void DrawSelectedMonDetails(void)
{
    struct Pokemon *mon = GetSelectedMon();
    u16 species;
    u8 level;
    enum Item item;
    enum Ability ability;
    u8 nickname[POKEMON_NAME_LENGTH + 1];
    u8 *txtPtr;

    if (mon == NULL)
        return;

    species = GetMonData(mon, MON_DATA_SPECIES_OR_EGG);
    level = GetMonData(mon, MON_DATA_LEVEL);
    if (sTeamViewerState.side == TEAM_VIEWER_SIDE_PLAYER)
    {
        GetMonData(mon, MON_DATA_NICKNAME, nickname);
        txtPtr = StringCopy_Nickname(gStringVar4, nickname);
    }
    else
    {
        txtPtr = StringCopy(gStringVar4, GetSpeciesName(species));
    }
    txtPtr = StringAppend(txtPtr, COMPOUND_STRING(" LV."));
    ConvertIntToDecimalStringN(txtPtr, level, STR_CONV_MODE_LEFT_ALIGN, 3);
    PrintText(WIN_FOOTER, gStringVar4, 8, 2);

    BuildTypeText(gStringVar4, species);
    PrintText(WIN_FOOTER, gStringVar4, 8, 12);

    if (sTeamViewerState.side == TEAM_VIEWER_SIDE_OPPONENT)
    {
        PrintText(WIN_FOOTER, sHiddenText, 8, 24);
        return;
    }

    item = GetMonData(mon, MON_DATA_HELD_ITEM);
    BuildHeldItemText(gStringVar4, item);
    PrintText(WIN_FOOTER, gStringVar4, 126, 2);

    ability = GetMonAbility(mon);
    txtPtr = StringCopy(gStringVar4, COMPOUND_STRING("Ab:"));
    txtPtr = StringCopy(txtPtr, gAbilitiesInfo[ability].name);
    PrintText(WIN_FOOTER, gStringVar4, 126, 12);

    BuildMoveText(gStringVar4, mon, 0);
    PrintText(WIN_FOOTER, gStringVar4, 8, 23);
    BuildMoveText(gStringVar4, mon, 1);
    PrintText(WIN_FOOTER, gStringVar4, 126, 23);
    BuildMoveText(gStringVar4, mon, 2);
    PrintText(WIN_FOOTER, gStringVar4, 8, 35);
    BuildMoveText(gStringVar4, mon, 3);
    PrintText(WIN_FOOTER, gStringVar4, 126, 35);
}

static void BuildMonIconLabel(u8 *dest, struct Pokemon *mon, u8 slot)
{
    (void)mon;

    if (sTeamViewerState.mode == TEAM_VIEWER_MODE_IN_BATTLE)
    {
        dest[0] = EOS;
    }
    else
    {
        ConvertIntToDecimalStringN(dest, slot + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
    }
}

static void BuildBattleActionPrompt(u8 *dest)
{
    u8 *txtPtr;

    if (sTeamViewerState.mode == TEAM_VIEWER_MODE_IN_BATTLE)
    {
        StringCopy(dest, COMPOUND_STRING("Read-only battle team view."));
    }
    else if (sTeamViewerState.selectedCount != 0)
    {
        txtPtr = StringCopy(dest, COMPOUND_STRING("Choose "));
        txtPtr = ConvertIntToDecimalStringN(txtPtr, sTeamViewerState.selectedCount, STR_CONV_MODE_LEFT_ALIGN, 1);
        StringCopy(txtPtr, COMPOUND_STRING(" from your team."));
    }
    else
    {
        StringCopy(dest, COMPOUND_STRING("Read-only battle team view."));
    }
}

static void BuildTypeText(u8 *dest, u16 species)
{
    enum Type type1 = gSpeciesInfo[species].types[0];
    enum Type type2 = gSpeciesInfo[species].types[1];
    u8 *txtPtr = StringCopy(dest, COMPOUND_STRING("T:"));

    txtPtr = StringCopy(txtPtr, gTypesInfo[type1].name);
    if (type2 != type1)
    {
        txtPtr = StringAppend(txtPtr, COMPOUND_STRING("/"));
        StringCopy(txtPtr, gTypesInfo[type2].name);
    }
}

static void BuildHeldItemText(u8 *dest, enum Item item)
{
    u8 *txtPtr = StringCopy(dest, COMPOUND_STRING("It:"));

    if (item == ITEM_NONE)
        StringCopy(txtPtr, COMPOUND_STRING("None"));
    else
        StringCopy(txtPtr, GetItemName(item));
}

static void BuildMoveText(u8 *dest, struct Pokemon *mon, u8 moveSlot)
{
    enum Move move = GetMonData(mon, MON_DATA_MOVE1 + moveSlot);
    u8 *txtPtr = ConvertIntToDecimalStringN(dest, moveSlot + 1, STR_CONV_MODE_LEFT_ALIGN, 1);

    txtPtr = StringAppend(txtPtr, COMPOUND_STRING(" "));
    if (move == MOVE_NONE)
        StringCopy(txtPtr, COMPOUND_STRING("-"));
    else
        StringCopy(txtPtr, GetMoveName(move));
}

static struct Pokemon *GetSelectedMon(void)
{
    struct Pokemon *party;
    u8 count = GetCurrentSideCount();

    if (count == 0 || sTeamViewerState.cursor >= count)
        return NULL;

    party = sTeamViewerState.side == TEAM_VIEWER_SIDE_PLAYER ? gPlayerParty : sTeamViewerState.enemyParty;
    if (GetMonData(&party[sTeamViewerState.cursor], MON_DATA_SPECIES_OR_EGG) == SPECIES_NONE)
        return NULL;
    return &party[sTeamViewerState.cursor];
}

static u8 GetCurrentSideCount(void)
{
    if (sTeamViewerState.side == TEAM_VIEWER_SIDE_PLAYER)
        return sTeamViewerState.playerCount;
    return sTeamViewerState.enemyPartyCount;
}

static void InitTeamIconSpriteIds(void)
{
    u8 side;
    u8 slot;

    for (side = 0; side < 2; side++)
    {
        for (slot = 0; slot < PARTY_SIZE; slot++)
            sTeamViewerState.iconSpriteIds[side][slot] = SPRITE_NONE;
    }
    sTeamViewerState.iconSpritesInitialized = TRUE;
}

static void DestroyTeamIcons(void)
{
    u8 side;
    u8 slot;

    if (!sTeamViewerState.iconSpritesInitialized)
        return;

    for (side = 0; side < 2; side++)
    {
        for (slot = 0; slot < PARTY_SIZE; slot++)
        {
            u8 spriteId = sTeamViewerState.iconSpriteIds[side][slot];

            if (spriteId != SPRITE_NONE)
            {
                FreeAndDestroyMonIconSprite(&gSprites[spriteId]);
                sTeamViewerState.iconSpriteIds[side][slot] = SPRITE_NONE;
            }
        }
    }
}

static void CreateTeamIcons(void)
{
    u8 i;

    DestroyTeamIcons();

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (i < sTeamViewerState.playerCount)
            CreateTeamIcon(gPlayerParty, TEAM_VIEWER_SIDE_PLAYER, i);
        if (i < sTeamViewerState.enemyPartyCount)
            CreateTeamIcon(sTeamViewerState.enemyParty, TEAM_VIEWER_SIDE_OPPONENT, i);
    }
}

static void CreateTeamIcon(struct Pokemon *party, enum TeamViewerSide side, u8 slot)
{
    u16 species = GetMonData(&party[slot], MON_DATA_SPECIES_OR_EGG);
    u32 personality;
    s16 x;
    s16 y;

    if (species == SPECIES_NONE)
        return;

    personality = GetMonData(&party[slot], MON_DATA_PERSONALITY);
    x = (side == TEAM_VIEWER_SIDE_PLAYER ? TEAM_VIEWER_ICON_PLAYER_X : TEAM_VIEWER_ICON_OPPONENT_X)
      + (slot % TEAM_VIEWER_GRID_COLUMNS) * TEAM_VIEWER_ICON_COLUMN_SPACING;
    y = TEAM_VIEWER_ICON_TOP_Y + (slot / TEAM_VIEWER_GRID_COLUMNS) * TEAM_VIEWER_ICON_ROW_SPACING;
    sTeamViewerState.iconSpriteIds[side][slot] = CreateMonIconIsEgg(species,
                                                                    SpriteCB_MonIcon,
                                                                    x,
                                                                    y,
                                                                    1,
                                                                    personality,
                                                                    GetMonData(&party[slot], MON_DATA_IS_EGG));
}

static void PrintText(u8 windowId, const u8 *str, u8 x, u8 y)
{
    AddTextPrinterParameterized4(windowId, FONT_SMALL, x, y, 0, 0, sTextColor, TEXT_SKIP_DRAW, str);
}
