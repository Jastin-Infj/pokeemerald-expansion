#include "global.h"
#include "battle.h"
#include "battle_controllers.h"
#include "battle_dynamax.h"
#include "battle_gimmick.h"
#include "battle_main.h"
#include "battle_message.h"
#include "data.h"
#include "menu.h"
#include "palette.h"
#include "pokemon.h"
#include "sm_battle_menu.h"
#include "string_util.h"
#include "strings.h"
#include "text.h"
#include "ui_style.h"
#include "constants/rgb.h"
#include "constants/moves.h"

static EWRAM_DATA u32 sBattler = 0;
static EWRAM_DATA u8 sEffectiveness[8] = {0};
static EWRAM_DATA u8 sPartnerPrompt[96] = {0};
// The controller clears zmove.viewing before entering target selection. Keep
// the displayed page until ordinary move names or actions are requested again.
static EWRAM_DATA bool32 sZDisplay = FALSE;
static const u8 sFightIcon[] = INCGFX_U8("graphics/battle_interface/sm_icon_fight.png", ".4bpp");
static const u8 sPokemonIcon[] = INCGFX_U8("graphics/battle_interface/sm_icon_pokemon.png", ".4bpp");
static const u8 sBagIcon[] = INCGFX_U8("graphics/battle_interface/sm_icon_bag.png", ".4bpp");
static const u8 sRunIcon[] = INCGFX_U8("graphics/battle_interface/sm_icon_run.png", ".4bpp");

// Palette indices are shared by the hand-drawn geometry and the text renderer.
static const u16 sColors[16] = {
    RGB(21, 29, 26), RGB(3, 8, 9), RGB(31, 31, 29), RGB(13, 25, 22),
    RGB(3, 19, 17), RGB(7, 16, 22), RGB(17, 27, 29), RGB(28, 31, 28),
    RGB(23, 15, 4), RGB(31, 26, 12), RGB(11, 19, 18), RGB(24, 30, 26),
    RGB(22, 5, 8), RGB(31, 15, 14), RGB(7, 21, 11), RGB(22, 30, 15),
};

bool32 SmBattleMenuEnabled(void)
{
    return IsBattleMenuSM() && gBattleScripting.windowsType == B_WIN_TYPE_NORMAL
        && !(gBattleTypeFlags & (BATTLE_TYPE_SAFARI | BATTLE_TYPE_CATCH_TUTORIAL | BATTLE_TYPE_POKEDUDE));
}

void SmBattleMenuSetBattler(u32 battler)
{
    sBattler = battler;
    sEffectiveness[0] = EOS;
    sPartnerPrompt[0] = EOS;
    sZDisplay = FALSE;
}

void SmBattleMenuTemplates(struct WindowTemplate *t)
{
    u32 i;
    t[B_WIN_MSG] = (struct WindowTemplate){0, 0, 14, 30, 6, 13, 0x90};
    t[B_WIN_ACTION_MENU] = (struct WindowTemplate){0, 0, 34, 30, 6, 12, 0x190};
    // The prompt is rendered inside the center of the action canvas instead.
    // Details overlay the battle field, reusing the hidden action-page storage.
    t[B_WIN_MOVE_DESCRIPTION] = (struct WindowTemplate){0, 0, 44, 30, 10, 6, 0x170};
    for (i = 0; i < MAX_MON_MOVES; i++)
        t[B_WIN_MOVE_NAME_1 + i] = (struct WindowTemplate){0, (i & 1) * 15, 54 + (i / 2) * 3, 15, 3, 12 + i, 0x300 + i * 45};
}

static void Rect(u32 win, u32 x, u32 y, u32 w, u32 h, u32 color)
{
    FillWindowPixelRect(win, PIXEL_FILL(color), x, y, w, h);
}

static void Text(u32 win, u32 x, u32 y, const u8 *str, u32 color, u32 width)
{
    u8 colors[] = {0, color, 0};
    u32 font = GetFontIdToFit(str, FONT_SMALL, 0, width);
    AddTextPrinterParameterized4(win, font, x, y, 0, 0, colors, TEXT_SKIP_DRAW, str);
}

static void Copy(u32 win)
{
    PutWindowTilemap(win);
    CopyWindowToVram(win, COPYWIN_FULL);
}

static void Button(u32 win, u32 x, u32 y, u32 w, u32 h, u32 dark, u32 light, const u8 *label, bool32 selected)
{
    u32 row;
    for (row = 1; row < h - 1; row++)
    {
        u32 inset = row < 4 ? 4 - row : row > h - 5 ? row - (h - 5) : 0;
        Rect(win, x + inset, y + row, w - inset * 2, 1, 1);
        if (row > 1 && row < h - 2)
            Rect(win, x + 1 + inset, y + row, w - 2 - inset * 2, 1, selected ? 2 : light);
        if (row > 2 && row < h - 3)
        {
            u32 sweep = min(w - 8 - inset * 2, w / 3 + row / 2);
            Rect(win, x + 3 + inset, y + row, w - 6 - inset * 2, 1, dark);
            Rect(win, x + 3 + inset, y + row, sweep, 1, light);
            if (row == 3)
                Rect(win, x + 4 + inset, y + row, w - 8 - inset * 2, 1, 2);
        }
    }
    // Small illuminated corners make focus visible on every button color.
    if (selected)
    {
        Rect(win, x + 5, y + 1, 9, 1, 9);
        Rect(win, x + w - 14, y + h - 2, 9, 1, 9);
    }
    Text(win, x + (w - GetStringWidth(FONT_SMALL, label, 0)) / 2, y + h / 2 - 6, label, 1, w - 8);
}

static void Icon(u32 win, const u8 *tiles, u32 left, u32 top)
{
    u32 x, y;
    for (y = 0; y < 16; y++)
        for (x = 0; x < 16; x++)
        {
            u32 offset = (y / 8 * 2 + x / 8) * 32 + y % 8 * 4 + x % 8 / 2;
            u32 pixel = (tiles[offset] >> ((x & 1) * 4)) & 15;
            if (pixel)
                Rect(win, left + x, top + y, 1, 1, pixel);
        }
}

static void DrawActions(u32 selected)
{
    u32 x, y;
    u8 name[POKEMON_NAME_LENGTH + 1];
    u32 win = B_WIN_ACTION_MENU;
    LoadPalette(sColors, BG_PLTT_ID(12), sizeof(sColors));
    FillWindowPixelBuffer(win, PIXEL_FILL(11));
    for (y = 0; y < 48; y++)
        for (x = 0; x < 240; x++)
        {
            s32 dx = (s32)x - 120, dy = (s32)y - 14;
            s32 r = dx * dx + dy * dy * 3;
            if ((r > 225 && r < 289) || (r > 1764 && r < 1936) || (x + y) % 40 < 2)
                Rect(win, x, y, 1, 1, 3);
        }
    Rect(win, 0, 0, 240, 1, 2);
    Rect(win, 0, 47, 240, 1, 4);
    Button(win, 0, 0, 72, 24, 14, 15, gText_EmptyString2, selected == B_ACTION_SWITCH);
    Button(win, 0, 24, 72, 24, 8, 9, gText_EmptyString2, selected == B_ACTION_USE_ITEM);
    Button(win, 178, 0, 62, 48, 12, 13, gText_EmptyString2, selected == B_ACTION_USE_MOVE);
    Button(win, 80, 29, 90, 19, 5, 6, gText_EmptyString2, selected == B_ACTION_RUN);
    Icon(win, sPokemonIcon, 5, 4);
    Icon(win, sBagIcon, 5, 28);
    Icon(win, sFightIcon, 201, 6);
    Icon(win, sRunIcon, 94, 30);
    // Layered flame rays and a lower nameplate give the large FIGHT panel
    // depth at native resolution without covering the neighboring commands.
    for (y = 7; y < 25; y++)
    {
        Rect(win, 187 + (y / 4), y, 2, 1, 13);
        Rect(win, 226 - (y / 4), y, 2, 1, 9);
    }
    Rect(win, 188, 27, 43, 14, 12);
    Rect(win, 191, 40, 36, 1, 13);
    Text(win, 24, 6, COMPOUND_STRING("POKEMON"), 1, 44);
    Text(win, 31, 30, COMPOUND_STRING("BAG"), 1, 35);
    Text(win, 196, 28, COMPOUND_STRING("FIGHT"), 2, 39);
    Text(win, 114, 32, COMPOUND_STRING("RUN"), 2, 43);
    GetMonData(GetBattlerMon(sBattler), MON_DATA_NICKNAME, name);
    Text(win, 80, 4, name, 1, 90);
    Text(win, 80, 15, sPartnerPrompt[0] == EOS ? COMPOUND_STRING("CHOOSE ACTION") : sPartnerPrompt, 4, 90);
    Copy(win);
}

void SmBattleMenuMessageBackground(u32 win)
{
    u32 y;
    LoadPalette(sColors, BG_PLTT_ID(13), sizeof(sColors));
    FillWindowPixelBuffer(win, PIXEL_FILL(11));
    for (y = 2; y < 46; y++)
    {
        u32 inset = y < 7 ? 7 - y : y > 40 ? y - 40 : 0;
        Rect(win, 2 + inset, y, 236 - 2 * inset, 1, 4);
        if (y > 3 && y < 44)
            Rect(win, 4 + inset, y, 232 - 2 * inset, 1, 1);
    }
}

static void DrawMove(u32 index, const u8 *name, bool32 selected)
{
    u32 y;
    u32 win = B_WIN_MOVE_NAME_1 + index;
    struct ChooseMoveStruct *info = (struct ChooseMoveStruct *)&gBattleResources->bufferA[sBattler][4];
    enum Type type = SmBattleMoveType(sBattler, index);
    u16 colors[16];
    u32 tint = gTypesInfo[type].teraTypeRGBValue;
    u32 r = tint & 31, g = (tint >> 5) & 31, b = (tint >> 10) & 31;
    u8 pp[16];
    u8 *end;
    // Each card owns a palette, allowing four different move types at once.
    memcpy(colors, sColors, sizeof(colors));
    colors[5] = RGB((r + 62) / 3, (g + 62) / 3, (b + 62) / 3);
    colors[7] = RGB(29, 31, 28);
    colors[8] = RGB(r / 3, g / 3, b / 3);
    LoadPalette(colors, BG_PLTT_ID(12 + index), sizeof(colors));
    FillWindowPixelBuffer(win, PIXEL_FILL(11));
    for (y = 1; y <= 22; y++)
    {
        u32 inset = y < 5 ? 5 - y : y > 18 ? y - 18 : 0;
        Rect(win, 2 + inset, y, 116 - inset * 2, 1, selected == 2 ? 9 : selected ? 4 : 1);
        if (y > 1 && y < 22)
            Rect(win, 4 + inset, y, 112 - inset * 2, 1, y < 11 ? 7 : 5);
    }
    Text(win, 10, 0, name, 1, 100);
    if (info->moves[index] != MOVE_NONE)
    {
        Rect(win, 9, 12, 43, 9, 8);
        Text(win, 11, 9, gTypesInfo[type].name, 2, 39);
        end = StringCopy(pp, COMPOUND_STRING("PP "));
        end = ConvertIntToDecimalStringN(end, info->currentPp[index], STR_CONV_MODE_LEFT_ALIGN, 2);
        *end++ = CHAR_SLASH;
        ConvertIntToDecimalStringN(end, info->maxPp[index], STR_CONV_MODE_LEFT_ALIGN, 2);
        // Gold identifies the source card while the cyan cursor picks a target.
        if (selected == 2)
            Text(win, 57, 9, COMPOUND_STRING("SWAP"), 8, 54);
        else if (!gBattleResources->bufferA[sBattler][2])
            Text(win, 57, 9, pp, info->currentPp[index] == 0 ? 12 : 1, 54);
        if (selected)
            Text(win, 111, 0, sEffectiveness, 1, 9);
    }
    Copy(win);
}

static const u8 *MoveName(u32 index)
{
    struct ChooseMoveStruct *info = (struct ChooseMoveStruct *)&gBattleResources->bufferA[sBattler][4];
    enum Move move = info->moves[index];
    if (IsGimmickSelected(sBattler, GIMMICK_DYNAMAX) || GetActiveGimmick(sBattler) == GIMMICK_DYNAMAX)
        move = GetMaxMove(sBattler, move);
    return GetMoveName(move);
}

bool32 SmBattleMenuPrint(const u8 *text, u32 windowId)
{
    if (!SmBattleMenuEnabled())
        return FALSE;
    windowId &= ~B_WIN_COPYTOVRAM;
    // Reordering is explained on the gold source card. The original prompt
    // occupies the old PP pane, which overlaps our right-hand move cards.
    if (windowId == B_WIN_SWITCH_PROMPT)
        return TRUE;
    if (windowId == B_WIN_MOVE_DESCRIPTION)
    {
        u32 y;
        LoadPalette(sColors, BG_PLTT_ID(6), sizeof(sColors));
        FillWindowPixelBuffer(windowId, PIXEL_FILL(0));
        for (y = 2; y < 78; y++)
        {
            u32 inset = y < 7 ? 7 - y : y > 72 ? y - 72 : 0;
            Rect(windowId, 2 + inset, y, 236 - 2 * inset, 1, 6);
            if (y > 3 && y < 76)
                Rect(windowId, 4 + inset, y, 232 - 2 * inset, 1, 5);
        }
        Text(windowId, 8, 4, text, 2, 224);
        Copy(windowId);
        return TRUE;
    }
    if (gBattleStruct->zmove.viewing)
        sZDisplay = TRUE;
    if (sZDisplay && windowId >= B_WIN_MOVE_NAME_1 && windowId <= B_WIN_MOVE_TYPE)
    {
        // Z selection uses two title/effect cells and two metadata cells.
        // Do not redraw ordinary move names over the Z title when its cursor moves.
        u32 win = windowId;
        if (windowId == B_WIN_PP || windowId == B_WIN_DUMMY)
            return TRUE;
        if (windowId == B_WIN_PP_REMAINING)
            win = B_WIN_MOVE_NAME_2;
        if (windowId == B_WIN_MOVE_TYPE)
            win = B_WIN_MOVE_NAME_4;
        LoadPalette(sColors, BG_PLTT_ID(12 + win - B_WIN_MOVE_NAME_1), sizeof(sColors));
        FillWindowPixelBuffer(win, PIXEL_FILL(7));
        Rect(win, 2, 1, 116, 1, 8);
        Rect(win, 2, 22, 116, 1, 9);
        Text(win, 8, 5, text, 1, 104);
        Copy(win);
        return TRUE;
    }
    if (windowId == B_WIN_ACTION_MENU)
        DrawActions(gActionSelectionCursor[sBattler]);
    else if (windowId == B_WIN_ACTION_PROMPT)
    {
        // Keep the existing partner move/target preview beneath the actor.
        if ((gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER) && text[0] == CHAR_P)
        {
            const u8 *line = text;
            while (*line != EOS && *line != CHAR_NEWLINE)
                line++;
            if (*line == CHAR_NEWLINE)
            {
                u32 length = min(StringLength(++line), sizeof(sPartnerPrompt) - 1);
                StringCopyN(sPartnerPrompt, line, length);
                sPartnerPrompt[length] = EOS;
            }
        }
        DrawActions(gActionSelectionCursor[sBattler]);
    }
    else if (windowId >= B_WIN_MOVE_NAME_1 && windowId <= B_WIN_MOVE_NAME_4)
        DrawMove(windowId - B_WIN_MOVE_NAME_1, text, windowId - B_WIN_MOVE_NAME_1 == gMoveSelectionCursor[sBattler]);
    else if (windowId == B_WIN_PP || windowId == B_WIN_PP_REMAINING || windowId == B_WIN_MOVE_TYPE)
    {
        if (windowId == B_WIN_PP)
        {
            u32 length = min(StringLength(text), sizeof(sEffectiveness) - 1);
            StringCopyN(sEffectiveness, text, length);
            sEffectiveness[length] = EOS;
            if (text[0] == CHAR_P)
                sEffectiveness[0] = EOS;
        }
        DrawMove(gMoveSelectionCursor[sBattler], MoveName(gMoveSelectionCursor[sBattler]), TRUE);
    }
    else
        return FALSE;
    return TRUE;
}

bool32 SmBattleMenuCursor(u32 position, bool32 move, bool32 selected)
{
    if (!SmBattleMenuEnabled())
        return FALSE;
    if (move)
    {
        if (gBattleStruct->zmove.viewing || sZDisplay)
            return TRUE;
        DrawMove(position, MoveName(position), selected);
    }
    else if (selected)
        DrawActions(position);
    return TRUE;
}
