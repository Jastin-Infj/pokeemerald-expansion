#ifndef GUARD_UI_STYLE_H
#define GUARD_UI_STYLE_H

#include "global.h"

#define OPTIONS_UI_STYLE_DEFAULT 0
#define OPTIONS_UI_STYLE_XY      1
#define OPTIONS_BATTLE_MENU_DEFAULT 0
#define OPTIONS_BATTLE_MENU_SM      1
#define OPTIONS_SM_DETAILS_SOFT 0
#define OPTIONS_SM_DETAILS_RICH 1

// Unknown values from older saves fall back to the original appearance.
static inline bool32 IsUiStyleXY(void)
{
    return gSaveBlock2Ptr->optionsUiStyle == OPTIONS_UI_STYLE_XY;
}

static inline bool32 IsBattleMenuSM(void)
{
    return gSaveBlock2Ptr->optionsBattleMenu == OPTIONS_BATTLE_MENU_SM;
}

// Unset and unknown values use the softer interior.
static inline bool32 IsSmDetailsRich(void)
{
    return gSaveBlock2Ptr->optionsSmDetailsTone == OPTIONS_SM_DETAILS_RICH;
}

#endif
