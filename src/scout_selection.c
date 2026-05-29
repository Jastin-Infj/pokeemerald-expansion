#include "global.h"
#include "scout_selection.h"
#include "bg.h"
#include "data.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "main.h"
#include "menu.h"
#include "menu_helpers.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "pokemon_storage_system.h"
#include "pokemon_summary_screen.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/abilities.h"
#include "constants/characters.h"
#include "constants/form_change_types.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/pokeball.h"
#include "constants/pokemon.h"
#include "constants/rgb.h"
#include "constants/scout_selection.h"
#include "constants/songs.h"
#include "constants/species.h"

#define SCOUT_MAX_CANDIDATES 12
#define SCOUT_VISIBLE_COUNT  6
#define SCOUT_COLUMNS        2
#define SCOUT_ROWS           3
#define SCOUT_MAX_PICK_COUNT PARTY_SIZE

#define SCOUT_WIN_LIST 0
#define SCOUT_WIN_HELP 1

#define SCOUT_CELL_WIDTH  120
#define SCOUT_CELL_HEIGHT 40
#define SCOUT_CELL_TOP    18
#define SCOUT_CELL_INNER_X 6
#define SCOUT_CELL_INNER_Y 3
#define SCOUT_CELL_INNER_WIDTH 109
#define SCOUT_CELL_INNER_HEIGHT 34

#define SCOUT_KEY_REPEAT_START_DELAY    16
#define SCOUT_KEY_REPEAT_CONTINUE_DELAY 5

#define SCOUT_COLOR_BG       TEXT_COLOR_LIGHT_GRAY
#define SCOUT_COLOR_PANEL    TEXT_COLOR_WHITE
#define SCOUT_COLOR_BORDER   TEXT_COLOR_LIGHT_BLUE
#define SCOUT_COLOR_CURSOR   TEXT_DYNAMIC_COLOR_6
#define SCOUT_COLOR_SELECTED TEXT_COLOR_LIGHT_GREEN
#define SCOUT_COLOR_BAR      TEXT_DYNAMIC_COLOR_6
#define SCOUT_COLOR_SHADOW   TEXT_COLOR_DARK_GRAY

struct ScoutMonSpec
{
    u16 species;
    u8 level;
    enum Item item;
    enum PokeBall ball;
    u8 nature;
    enum Ability ability;
    u8 gender;
    enum Move moves[MAX_MON_MOVES];
    u8 ivs[NUM_STATS];
    u8 evs[NUM_STATS];
};

struct ScoutPool
{
    const struct ScoutMonSpec *mons;
    u8 count;
};

struct ScoutSelection
{
    u8 poolId;
    u8 candidateCount;
    u8 pickCount;
    u8 cursor;
    u8 scrollOffset;
    u8 selectedCount;
    bool8 fromSummary;
    bool8 helpTextNeedsRefresh;
    bool8 keyRepeatSaved;
    u16 savedKeyRepeatStartDelay;
    u16 savedKeyRepeatContinueDelay;
    u8 selectedOrder[SCOUT_MAX_CANDIDATES];
    u8 iconSpriteIds[SCOUT_VISIBLE_COUNT];
    struct Pokemon mons[SCOUT_MAX_CANDIDATES];
};

static EWRAM_DATA struct ScoutSelection sScoutSelectionState = {0};
static EWRAM_DATA struct ScoutSelection *sScoutSelection = NULL;

static void CB2_InitScoutSelectionScreen(void);
static void CB2_ScoutSelectionScreen(void);
static void VBlankCB_ScoutSelectionScreen(void);
static void Task_ScoutSelectionInput(u8 taskId);
static void Task_ScoutSelectionOpenSummary(u8 taskId);
static void Task_ScoutSelectionExit(u8 taskId);
static void ScoutSelection_Draw(bool8 refreshIcons);
static void ScoutSelection_DrawHeader(void);
static void ScoutSelection_DrawCandidate(u8 index);
static void ScoutSelection_DrawCard(u8 index, u8 x, u8 y);
static void ScoutSelection_DrawHelpText(void);
static void ScoutSelection_RedrawCursor(u8 oldCursor);
static void ScoutSelection_CopyCardRowToVram(u8 index);
static void ScoutSelection_RestoreHelpTextIfNeeded(void);
static void ScoutSelection_CreateIcons(void);
static void ScoutSelection_DestroyIcons(void);
static void ScoutSelection_DestroyUi(void);
static void ScoutSelection_FreeState(void);
static void ScoutSelection_InitIconIds(void);
static void ScoutSelection_ApplyKeyRepeat(void);
static void ScoutSelection_RestoreKeyRepeat(void);
static void ScoutSelection_BuildCandidates(const struct ScoutPool *pool, u8 count);
static void ScoutSelection_CreateMon(struct Pokemon *mon, const struct ScoutMonSpec *spec);
static void ScoutSelection_SetAbilityFromId(struct Pokemon *mon, u16 species, enum Ability ability);
static void ScoutSelection_EnsureCursorVisible(void);
static u8 ScoutSelection_GetMaxScrollOffset(void);
static void ScoutSelection_MoveCursor(s8 dx, s8 dy);
static void ScoutSelection_ToggleSelected(void);
static bool8 ScoutSelection_CanConfirm(void);
static u16 ScoutSelection_CountAvailableStorageSlots(void);
static const struct ScoutPool *ScoutSelection_GetPool(u16 poolId);

static const struct BgTemplate sBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0,
    },
};

static const struct WindowTemplate sWindowTemplates[] =
{
    [SCOUT_WIN_LIST] =
    {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 30,
        .height = 18,
        .paletteNum = 15,
        .baseBlock = 1,
    },
    [SCOUT_WIN_HELP] =
    {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 18,
        .width = 30,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 541,
    },
    DUMMY_WIN_TEMPLATE,
};

static const u8 sTextColors[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
static const u8 sTextColorsSelected[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE};
static const u8 sTextColorsOnBar[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_BLUE};
static const u8 sTextColorsCursor[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_BLUE};
static const u8 sText_ScoutTitle[] = _("SCOUT SELECTION");
static const u8 sText_Help[] = _("A Pick  SELECT Summary  START Done");
static const u8 sText_NotEnough[] = _("Pick the requested number first.");
static const u8 sText_Level[] = _("Lv");
static const u8 sText_Pick[] = _("Pick ");
static const u8 sText_Selector[] = _(">");
static const u8 sText_ScrollUp[] = _("UP");
static const u8 sText_ScrollDown[] = _("DN");
static const u8 sText_Slash[] = _("/");

static const u8 sScoutIvMonDataIds[NUM_STATS] =
{
    MON_DATA_HP_IV,
    MON_DATA_ATK_IV,
    MON_DATA_DEF_IV,
    MON_DATA_SPEED_IV,
    MON_DATA_SPATK_IV,
    MON_DATA_SPDEF_IV,
};

static const u8 sScoutEvMonDataIds[NUM_STATS] =
{
    MON_DATA_HP_EV,
    MON_DATA_ATK_EV,
    MON_DATA_DEF_EV,
    MON_DATA_SPEED_EV,
    MON_DATA_SPATK_EV,
    MON_DATA_SPDEF_EV,
};

#include "data/scout_selection_pools.h"

void InitScoutSelection(void)
{
    const struct ScoutPool *pool;
    u16 poolId = VarGet(VAR_0x8004);
    u16 candidateCount = VarGet(VAR_0x8005);
    u16 pickCount = VarGet(VAR_0x8006);

    ScoutSelection_FreeState();

    pool = ScoutSelection_GetPool(poolId);
    if (pool == NULL || pool->count == 0)
    {
        gSpecialVar_Result = FALSE;
        return;
    }

    if (candidateCount == 0 || candidateCount > pool->count)
        candidateCount = pool->count;
    if (candidateCount > SCOUT_MAX_CANDIDATES)
        candidateCount = SCOUT_MAX_CANDIDATES;

    if (pickCount == 0)
        pickCount = 1;
    if (pickCount > candidateCount || pickCount > SCOUT_MAX_PICK_COUNT)
    {
        gSpecialVar_Result = FALSE;
        return;
    }

    sScoutSelection = &sScoutSelectionState;
    memset(sScoutSelection, 0, sizeof(*sScoutSelection));

    sScoutSelection->poolId = poolId;
    sScoutSelection->candidateCount = candidateCount;
    sScoutSelection->pickCount = pickCount;
    ScoutSelection_InitIconIds();
    ScoutSelection_BuildCandidates(pool, candidateCount);

    gSpecialVar_Result = TRUE;
}

void OpenScoutSelection(void)
{
    if (sScoutSelection == NULL || sScoutSelection->candidateCount == 0)
    {
        gSpecialVar_Result = SCOUT_RESULT_CANCEL;
        return;
    }

    SetMainCallback2(CB2_InitScoutSelectionScreen);
}

void GiveSelectedScoutMons(void)
{
    u8 order;
    u8 i;
    u32 giveResult = MON_GIVEN_TO_PARTY;

    if (sScoutSelection == NULL || !ScoutSelection_CanConfirm())
    {
        gSpecialVar_Result = MON_CANT_GIVE;
        ScoutSelection_FreeState();
        return;
    }

    if (ScoutSelection_CountAvailableStorageSlots() < sScoutSelection->selectedCount)
    {
        gSpecialVar_Result = MON_CANT_GIVE;
        ScoutSelection_FreeState();
        return;
    }

    for (order = 1; order <= sScoutSelection->selectedCount; order++)
    {
        for (i = 0; i < sScoutSelection->candidateCount; i++)
        {
            if (sScoutSelection->selectedOrder[i] == order)
            {
                u32 result = GiveScriptedMonToPlayer(&sScoutSelection->mons[i], PARTY_SIZE);
                if (result == MON_CANT_GIVE)
                {
                    gSpecialVar_Result = MON_CANT_GIVE;
                    ScoutSelection_FreeState();
                    return;
                }
                if (result == MON_GIVEN_TO_PC)
                    giveResult = MON_GIVEN_TO_PC;
                break;
            }
        }
    }

    gSpecialVar_Result = giveResult;
    ScoutSelection_FreeState();
}

static const struct ScoutPool *ScoutSelection_GetPool(u16 poolId)
{
    if (poolId >= ARRAY_COUNT(sScoutPools))
        return NULL;
    return &sScoutPools[poolId];
}

static void ScoutSelection_BuildCandidates(const struct ScoutPool *pool, u8 count)
{
    u8 i;

    for (i = 0; i < count; i++)
        ScoutSelection_CreateMon(&sScoutSelection->mons[i], &pool->mons[i]);
}

static void ScoutSelection_CreateMon(struct Pokemon *mon, const struct ScoutMonSpec *spec)
{
    u8 i;
    bool32 defaultMoves = TRUE;
    u32 personality = GetMonPersonality(spec->species, spec->gender, spec->nature, RANDOM_UNOWN_LETTER);

    CreateMonWithIVs(mon, spec->species, spec->level, personality, OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);

    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (spec->moves[i] != MOVE_DEFAULT)
        {
            defaultMoves = FALSE;
            break;
        }
    }

    if (defaultMoves)
    {
        GiveMonInitialMoveset(mon);
    }
    else
    {
        for (i = 0; i < MAX_MON_MOVES; i++)
        {
            if (spec->moves[i] == MOVE_NONE)
                break;
            if (spec->moves[i] == MOVE_DEFAULT)
                GiveMonDefaultMove(mon, i);
            else
                SetMonMoveSlot(mon, spec->moves[i], i);
        }
    }

    ScoutSelection_SetAbilityFromId(mon, spec->species, spec->ability);

    for (i = 0; i < NUM_STATS; i++)
    {
        SetMonData(mon, sScoutIvMonDataIds[i], &spec->ivs[i]);
        SetMonData(mon, sScoutEvMonDataIds[i], &spec->evs[i]);
    }

    SetMonData(mon, MON_DATA_POKEBALL, &spec->ball);
    SetMonData(mon, MON_DATA_HELD_ITEM, &spec->item);
    TryFormChange(mon, FORM_CHANGE_ITEM_HOLD);
    CalculateMonStats(mon);
}

static void ScoutSelection_SetAbilityFromId(struct Pokemon *mon, u16 species, enum Ability ability)
{
    u8 i;

    if (ability == ABILITY_NONE)
        return;

    for (i = 0; i < NUM_ABILITY_SLOTS; i++)
    {
        if (GetSpeciesAbility(species, i) == ability)
        {
            SetMonData(mon, MON_DATA_ABILITY_NUM, &i);
            return;
        }
    }
}

static void CB2_InitScoutSelectionScreen(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankCallback(NULL);
        ResetVramOamAndBgCntRegs();
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
        InitWindows(sWindowTemplates);
        DeactivateAllTextPrinters();
        ResetPaletteFade();
        ResetSpriteData();
        ResetTasks();
        FreeAllSpritePalettes();
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        gMain.state++;
        break;
    case 1:
        Menu_LoadStdPalAt(BG_PLTT_ID(15));
        LoadMonIconPalettes();
        if (sScoutSelection->fromSummary)
        {
            sScoutSelection->cursor = gLastViewedMonIndex;
            sScoutSelection->fromSummary = FALSE;
            ScoutSelection_EnsureCursorVisible();
        }
        ScoutSelection_ApplyKeyRepeat();
        ScoutSelection_Draw(TRUE);
        ShowBg(0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_BG0_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        SetVBlankCallback(VBlankCB_ScoutSelectionScreen);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        CreateTask(Task_ScoutSelectionInput, 0);
        SetMainCallback2(CB2_ScoutSelectionScreen);
        break;
    }
}

static void CB2_ScoutSelectionScreen(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void VBlankCB_ScoutSelectionScreen(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void Task_ScoutSelectionInput(u8 taskId)
{
    if (gPaletteFade.active)
        return;

    if (JOY_REPEAT(DPAD_LEFT))
        ScoutSelection_MoveCursor(-1, 0);
    else if (JOY_REPEAT(DPAD_RIGHT))
        ScoutSelection_MoveCursor(1, 0);
    else if (JOY_REPEAT(DPAD_UP))
        ScoutSelection_MoveCursor(0, -1);
    else if (JOY_REPEAT(DPAD_DOWN))
        ScoutSelection_MoveCursor(0, 1);
    else if (JOY_NEW(A_BUTTON))
    {
        ScoutSelection_ToggleSelected();
        ScoutSelection_Draw(FALSE);
    }
    else if (JOY_NEW(SELECT_BUTTON))
    {
        PlaySE(SE_SELECT);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_ScoutSelectionOpenSummary;
    }
    else if (JOY_NEW(START_BUTTON))
    {
        if (ScoutSelection_CanConfirm())
        {
            PlaySE(SE_SELECT);
            gTasks[taskId].data[0] = SCOUT_RESULT_SELECTED;
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            gTasks[taskId].func = Task_ScoutSelectionExit;
        }
        else
        {
            PlaySE(SE_BOO);
            FillWindowPixelBuffer(SCOUT_WIN_HELP, PIXEL_FILL(SCOUT_COLOR_BAR));
            AddTextPrinterParameterized3(SCOUT_WIN_HELP, FONT_NORMAL, 6, 1, sTextColorsOnBar, TEXT_SKIP_DRAW, sText_NotEnough);
            CopyWindowToVram(SCOUT_WIN_HELP, COPYWIN_GFX);
            sScoutSelection->helpTextNeedsRefresh = TRUE;
        }
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_EXIT);
        gTasks[taskId].data[0] = SCOUT_RESULT_CANCEL;
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_ScoutSelectionExit;
    }
}

static void Task_ScoutSelectionOpenSummary(u8 taskId)
{
    if (gPaletteFade.active)
        return;

    DestroyTask(taskId);
    ScoutSelection_DestroyUi();
    sScoutSelection->fromSummary = TRUE;
    ShowPokemonSummaryScreen(SUMMARY_MODE_LOCK_MOVES, sScoutSelection->mons, sScoutSelection->cursor, sScoutSelection->candidateCount - 1, CB2_InitScoutSelectionScreen);
}

static void Task_ScoutSelectionExit(u8 taskId)
{
    if (gPaletteFade.active)
        return;

    gSpecialVar_Result = gTasks[taskId].data[0];
    DestroyTask(taskId);
    ScoutSelection_DestroyUi();
    if (gSpecialVar_Result == SCOUT_RESULT_CANCEL)
        ScoutSelection_FreeState();
    SetMainCallback2(CB2_ReturnToFieldContinueScript);
}

static void ScoutSelection_Draw(bool8 refreshIcons)
{
    u8 i;

    FillWindowPixelBuffer(SCOUT_WIN_LIST, PIXEL_FILL(SCOUT_COLOR_BG));
    PutWindowTilemap(SCOUT_WIN_LIST);
    ScoutSelection_DrawHeader();

    if (sScoutSelection->scrollOffset > 0)
        AddTextPrinterParameterized3(SCOUT_WIN_LIST, FONT_SMALL, 226, 22, sTextColors, TEXT_SKIP_DRAW, sText_ScrollUp);
    if (sScoutSelection->scrollOffset < ScoutSelection_GetMaxScrollOffset())
        AddTextPrinterParameterized3(SCOUT_WIN_LIST, FONT_SMALL, 226, 126, sTextColors, TEXT_SKIP_DRAW, sText_ScrollDown);

    for (i = 0; i < SCOUT_VISIBLE_COUNT; i++)
    {
        u8 index = sScoutSelection->scrollOffset + i;
        ScoutSelection_DrawCandidate(index);
    }

    CopyWindowToVram(SCOUT_WIN_LIST, COPYWIN_FULL);
    ScoutSelection_DrawHelpText();
    sScoutSelection->helpTextNeedsRefresh = FALSE;
    if (refreshIcons)
        ScoutSelection_CreateIcons();
}

static void ScoutSelection_DrawHeader(void)
{
    u8 start;
    u8 end;
    u8 *txtPtr;

    FillWindowPixelRect(SCOUT_WIN_LIST, PIXEL_FILL(SCOUT_COLOR_BAR), 0, 0, 240, 16);
    FillWindowPixelRect(SCOUT_WIN_LIST, PIXEL_FILL(SCOUT_COLOR_BORDER), 0, 16, 240, 2);
    AddTextPrinterParameterized3(SCOUT_WIN_LIST, FONT_NORMAL, 6, 1, sTextColorsOnBar, TEXT_SKIP_DRAW, sText_ScoutTitle);

    start = sScoutSelection->scrollOffset + 1;
    end = sScoutSelection->scrollOffset + SCOUT_VISIBLE_COUNT;
    if (end > sScoutSelection->candidateCount)
        end = sScoutSelection->candidateCount;
    txtPtr = ConvertIntToDecimalStringN(gStringVar4, start, STR_CONV_MODE_LEFT_ALIGN, 2);
    txtPtr = StringAppend(txtPtr, sText_Slash);
    ConvertIntToDecimalStringN(txtPtr, end, STR_CONV_MODE_LEFT_ALIGN, 2);
    AddTextPrinterParameterized3(SCOUT_WIN_LIST, FONT_NORMAL, 188, 1, sTextColorsOnBar, TEXT_SKIP_DRAW, gStringVar4);

    txtPtr = ConvertIntToDecimalStringN(gStringVar4, sScoutSelection->selectedCount, STR_CONV_MODE_LEFT_ALIGN, 1);
    txtPtr = StringAppend(txtPtr, sText_Slash);
    ConvertIntToDecimalStringN(txtPtr, sScoutSelection->pickCount, STR_CONV_MODE_LEFT_ALIGN, 1);
    AddTextPrinterParameterized3(SCOUT_WIN_LIST, FONT_NORMAL, 220, 1, sTextColorsOnBar, TEXT_SKIP_DRAW, gStringVar4);
}

static void ScoutSelection_DrawCandidate(u8 index)
{
    u8 visibleIndex;
    u8 row;
    u8 col;
    u8 x;
    u8 y;
    u8 *txtPtr;
    const u8 *colors = sTextColors;

    if (index >= sScoutSelection->candidateCount)
        return;
    if (index < sScoutSelection->scrollOffset || index >= sScoutSelection->scrollOffset + SCOUT_VISIBLE_COUNT)
        return;

    visibleIndex = index - sScoutSelection->scrollOffset;
    row = visibleIndex / SCOUT_COLUMNS;
    col = visibleIndex % SCOUT_COLUMNS;
    x = col * SCOUT_CELL_WIDTH;
    y = SCOUT_CELL_TOP + row * SCOUT_CELL_HEIGHT;

    ScoutSelection_DrawCard(index, x, y);

    if (sScoutSelection->selectedOrder[index] != 0)
        colors = sTextColorsSelected;

    if (index == sScoutSelection->cursor)
        AddTextPrinterParameterized3(SCOUT_WIN_LIST, FONT_NORMAL, x + 7, y + 9, sTextColorsCursor, TEXT_SKIP_DRAW, sText_Selector);

    StringCopyN(gStringVar4, GetSpeciesName(GetMonData(&sScoutSelection->mons[index], MON_DATA_SPECIES)), 10);
    gStringVar4[10] = EOS;
    AddTextPrinterParameterized3(SCOUT_WIN_LIST, FONT_NARROW, x + 44, y + 4, colors, TEXT_SKIP_DRAW, gStringVar4);

    AddTextPrinterParameterized3(SCOUT_WIN_LIST, FONT_SMALL, x + 44, y + 19, colors, TEXT_SKIP_DRAW, sText_Level);
    ConvertIntToDecimalStringN(gStringVar4, GetMonData(&sScoutSelection->mons[index], MON_DATA_LEVEL), STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized3(SCOUT_WIN_LIST, FONT_SMALL, x + 60, y + 19, colors, TEXT_SKIP_DRAW, gStringVar4);

    if (sScoutSelection->selectedOrder[index] != 0)
    {
        txtPtr = StringCopy(gStringVar4, sText_Pick);
        ConvertIntToDecimalStringN(txtPtr, sScoutSelection->selectedOrder[index], STR_CONV_MODE_LEFT_ALIGN, 1);
        AddTextPrinterParameterized3(SCOUT_WIN_LIST, FONT_SMALL, x + 84, y + 19, colors, TEXT_SKIP_DRAW, gStringVar4);
    }
}

static void ScoutSelection_DrawCard(u8 index, u8 x, u8 y)
{
    u8 border = SCOUT_COLOR_BORDER;
    u8 fill = SCOUT_COLOR_PANEL;
    u8 innerX = x + SCOUT_CELL_INNER_X;
    u8 innerY = y + SCOUT_CELL_INNER_Y;

    if (sScoutSelection->selectedOrder[index] != 0)
    {
        border = SCOUT_COLOR_SELECTED;
        fill = SCOUT_COLOR_SELECTED;
    }

    if (index == sScoutSelection->cursor)
        border = SCOUT_COLOR_CURSOR;

    FillWindowPixelRect(SCOUT_WIN_LIST, PIXEL_FILL(SCOUT_COLOR_SHADOW), innerX + 2, innerY + 2, SCOUT_CELL_INNER_WIDTH, SCOUT_CELL_INNER_HEIGHT);
    FillWindowPixelRect(SCOUT_WIN_LIST, PIXEL_FILL(border), innerX, innerY, SCOUT_CELL_INNER_WIDTH, SCOUT_CELL_INNER_HEIGHT);
    FillWindowPixelRect(SCOUT_WIN_LIST, PIXEL_FILL(fill), innerX + 2, innerY + 2, SCOUT_CELL_INNER_WIDTH - 4, SCOUT_CELL_INNER_HEIGHT - 4);

    if (index == sScoutSelection->cursor)
        FillWindowPixelRect(SCOUT_WIN_LIST, PIXEL_FILL(border), innerX + 2, innerY + 2, 5, SCOUT_CELL_INNER_HEIGHT - 4);
    else if (sScoutSelection->selectedOrder[index] != 0)
        FillWindowPixelRect(SCOUT_WIN_LIST, PIXEL_FILL(SCOUT_COLOR_PANEL), innerX + 2, innerY + 2, 5, SCOUT_CELL_INNER_HEIGHT - 4);
}

static void ScoutSelection_DrawHelpText(void)
{
    FillWindowPixelBuffer(SCOUT_WIN_HELP, PIXEL_FILL(SCOUT_COLOR_BAR));
    PutWindowTilemap(SCOUT_WIN_HELP);
    AddTextPrinterParameterized3(SCOUT_WIN_HELP, FONT_SMALL, 6, 2, sTextColorsOnBar, TEXT_SKIP_DRAW, sText_Help);
    CopyWindowToVram(SCOUT_WIN_HELP, COPYWIN_FULL);
}

static void ScoutSelection_RedrawCursor(u8 oldCursor)
{
    ScoutSelection_DrawCandidate(oldCursor);
    ScoutSelection_DrawCandidate(sScoutSelection->cursor);
    ScoutSelection_CopyCardRowToVram(oldCursor);
    if ((oldCursor - sScoutSelection->scrollOffset) / SCOUT_COLUMNS != (sScoutSelection->cursor - sScoutSelection->scrollOffset) / SCOUT_COLUMNS)
        ScoutSelection_CopyCardRowToVram(sScoutSelection->cursor);
    ScoutSelection_RestoreHelpTextIfNeeded();
}

static void ScoutSelection_CopyCardRowToVram(u8 index)
{
    u8 visibleIndex;
    u8 row;
    u8 y;

    if (index < sScoutSelection->scrollOffset || index >= sScoutSelection->scrollOffset + SCOUT_VISIBLE_COUNT)
        return;

    visibleIndex = index - sScoutSelection->scrollOffset;
    row = visibleIndex / SCOUT_COLUMNS;
    y = (SCOUT_CELL_TOP + row * SCOUT_CELL_HEIGHT) / 8;
    CopyWindowRectToVram(SCOUT_WIN_LIST, COPYWIN_GFX, 0, y, 30, 6);
}

static void ScoutSelection_RestoreHelpTextIfNeeded(void)
{
    if (sScoutSelection->helpTextNeedsRefresh)
    {
        ScoutSelection_DrawHelpText();
        sScoutSelection->helpTextNeedsRefresh = FALSE;
    }
}

static void ScoutSelection_CreateIcons(void)
{
    u8 i;

    ScoutSelection_DestroyIcons();
    for (i = 0; i < SCOUT_VISIBLE_COUNT; i++)
    {
        u8 index = sScoutSelection->scrollOffset + i;
        u8 row = i / SCOUT_COLUMNS;
        u8 col = i % SCOUT_COLUMNS;
        s16 x = col * SCOUT_CELL_WIDTH + 27;
        s16 y = SCOUT_CELL_TOP + row * SCOUT_CELL_HEIGHT + 18;

        if (index >= sScoutSelection->candidateCount)
            continue;

        sScoutSelection->iconSpriteIds[i] = CreateMonIcon(
            GetMonData(&sScoutSelection->mons[index], MON_DATA_SPECIES_OR_EGG),
            SpriteCallbackDummy,
            x,
            y,
            0,
            GetMonData(&sScoutSelection->mons[index], MON_DATA_PERSONALITY));
    }
}

static void ScoutSelection_DestroyIcons(void)
{
    u8 i;

    if (sScoutSelection == NULL)
        return;

    for (i = 0; i < SCOUT_VISIBLE_COUNT; i++)
    {
        if (sScoutSelection->iconSpriteIds[i] != SPRITE_NONE)
        {
            FreeAndDestroyMonIconSprite(&gSprites[sScoutSelection->iconSpriteIds[i]]);
            sScoutSelection->iconSpriteIds[i] = SPRITE_NONE;
        }
    }
}

static void ScoutSelection_DestroyUi(void)
{
    ScoutSelection_DestroyIcons();
    FreeMonIconPalettes();
    FreeAllWindowBuffers();
    ScoutSelection_RestoreKeyRepeat();
    SetVBlankCallback(NULL);
}

static void ScoutSelection_FreeState(void)
{
    if (sScoutSelection != NULL)
    {
        memset(sScoutSelection, 0, sizeof(*sScoutSelection));
        sScoutSelection = NULL;
    }
}

static void ScoutSelection_InitIconIds(void)
{
    u8 i;

    for (i = 0; i < SCOUT_VISIBLE_COUNT; i++)
        sScoutSelection->iconSpriteIds[i] = SPRITE_NONE;
}

static void ScoutSelection_ApplyKeyRepeat(void)
{
    if (!sScoutSelection->keyRepeatSaved)
    {
        sScoutSelection->savedKeyRepeatStartDelay = gKeyRepeatStartDelay;
        sScoutSelection->savedKeyRepeatContinueDelay = gKeyRepeatContinueDelay;
        sScoutSelection->keyRepeatSaved = TRUE;
    }

    gKeyRepeatStartDelay = SCOUT_KEY_REPEAT_START_DELAY;
    gKeyRepeatContinueDelay = SCOUT_KEY_REPEAT_CONTINUE_DELAY;
    gMain.keyRepeatCounter = SCOUT_KEY_REPEAT_START_DELAY;
}

static void ScoutSelection_RestoreKeyRepeat(void)
{
    if (sScoutSelection != NULL && sScoutSelection->keyRepeatSaved)
    {
        gKeyRepeatStartDelay = sScoutSelection->savedKeyRepeatStartDelay;
        gKeyRepeatContinueDelay = sScoutSelection->savedKeyRepeatContinueDelay;
        gMain.keyRepeatCounter = gKeyRepeatStartDelay;
    }
}

static void ScoutSelection_MoveCursor(s8 dx, s8 dy)
{
    u8 oldCursor = sScoutSelection->cursor;
    u8 oldScrollOffset = sScoutSelection->scrollOffset;
    s16 cursor = sScoutSelection->cursor;

    if (dx < 0)
    {
        if ((cursor % SCOUT_COLUMNS) != 0)
            cursor--;
    }
    else if (dx > 0)
    {
        if ((cursor % SCOUT_COLUMNS) != SCOUT_COLUMNS - 1 && cursor + 1 < sScoutSelection->candidateCount)
            cursor++;
    }
    else if (dy < 0)
    {
        if (cursor >= SCOUT_COLUMNS)
            cursor -= SCOUT_COLUMNS;
    }
    else if (dy > 0)
    {
        if (cursor + SCOUT_COLUMNS < sScoutSelection->candidateCount)
            cursor += SCOUT_COLUMNS;
    }

    if (cursor != oldCursor)
    {
        sScoutSelection->cursor = cursor;
        ScoutSelection_EnsureCursorVisible();
        PlaySE(SE_SELECT);
        if (sScoutSelection->scrollOffset != oldScrollOffset)
            ScoutSelection_Draw(TRUE);
        else
            ScoutSelection_RedrawCursor(oldCursor);
    }
}

static void ScoutSelection_EnsureCursorVisible(void)
{
    u8 maxOffset = ScoutSelection_GetMaxScrollOffset();

    if (sScoutSelection->cursor < sScoutSelection->scrollOffset)
    {
        sScoutSelection->scrollOffset = (sScoutSelection->cursor / SCOUT_COLUMNS) * SCOUT_COLUMNS;
    }
    else if (sScoutSelection->cursor >= sScoutSelection->scrollOffset + SCOUT_VISIBLE_COUNT)
    {
        sScoutSelection->scrollOffset = ((sScoutSelection->cursor / SCOUT_COLUMNS) - (SCOUT_ROWS - 1)) * SCOUT_COLUMNS;
    }

    if (sScoutSelection->scrollOffset > maxOffset)
        sScoutSelection->scrollOffset = maxOffset;
}

static u8 ScoutSelection_GetMaxScrollOffset(void)
{
    u8 rows;

    if (sScoutSelection->candidateCount <= SCOUT_VISIBLE_COUNT)
        return 0;

    rows = (sScoutSelection->candidateCount + SCOUT_COLUMNS - 1) / SCOUT_COLUMNS;
    return (rows - SCOUT_ROWS) * SCOUT_COLUMNS;
}

static void ScoutSelection_ToggleSelected(void)
{
    u8 i;
    u8 cursor = sScoutSelection->cursor;
    u8 selectedOrder = sScoutSelection->selectedOrder[cursor];

    if (selectedOrder != 0)
    {
        sScoutSelection->selectedOrder[cursor] = 0;
        sScoutSelection->selectedCount--;
        for (i = 0; i < sScoutSelection->candidateCount; i++)
        {
            if (sScoutSelection->selectedOrder[i] > selectedOrder)
                sScoutSelection->selectedOrder[i]--;
        }
        PlaySE(SE_SELECT);
    }
    else if (sScoutSelection->selectedCount < sScoutSelection->pickCount)
    {
        sScoutSelection->selectedCount++;
        sScoutSelection->selectedOrder[cursor] = sScoutSelection->selectedCount;
        PlaySE(SE_SELECT);
    }
    else
    {
        PlaySE(SE_BOO);
    }
}

static bool8 ScoutSelection_CanConfirm(void)
{
    return sScoutSelection != NULL && sScoutSelection->selectedCount == sScoutSelection->pickCount;
}

static u16 ScoutSelection_CountAvailableStorageSlots(void)
{
    u8 i;
    u8 j;
    u16 count = 0;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) == SPECIES_NONE)
            count++;
    }

    for (i = 0; i < TOTAL_BOXES_COUNT; i++)
    {
        for (j = 0; j < IN_BOX_COUNT; j++)
        {
            if (GetBoxMonDataAt(i, j, MON_DATA_SPECIES) == SPECIES_NONE)
                count++;
        }
    }

    return count;
}
