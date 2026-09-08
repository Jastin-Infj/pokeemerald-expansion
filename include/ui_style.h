#ifndef GUARD_UI_STYLE_H
#define GUARD_UI_STYLE_H

#include "global.h"

#define OPTIONS_UI_STYLE_DEFAULT 0
#define OPTIONS_UI_STYLE_XY      1

// Unknown values from older saves fall back to the original appearance.
static inline bool32 IsUiStyleXY(void)
{
    return gSaveBlock2Ptr->optionsUiStyle == OPTIONS_UI_STYLE_XY;
}

#endif
