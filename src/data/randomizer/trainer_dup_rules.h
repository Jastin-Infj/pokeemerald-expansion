#pragma once

#include "constants/trainers.h"

struct RandomizerTrainerDupRule
{
    u16 trainerId;
    u8 maxSame;      // 同じ種族を何匹まで許容するか（255で無制限）  重複不可（同種1匹 1で定義)
    u8 minDistinct;  // 確保したいユニーク種の最低数（0で要求なし）
};

// ここに重複制御したいトレーナーだけ列挙する。
// 例: { TRAINER_ROXANNE_1, 1, 3 }, { TRAINER_ROXANNE_2, 1, 3 },
static const struct RandomizerTrainerDupRule gRandomizerTrainerDupRules[] = {
    { TRAINER_TIANA, 1, 2 },
    { RANDOMIZER_TRAINER_ID_END, 0, 0 },
};
