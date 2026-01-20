#ifndef GUARD_CONFIG_RANDOMIZER_H
#define GUARD_CONFIG_RANDOMIZER_H
#include "item.h"

// Global control. If FALSE, no randomizer functionality will be enabled.
// If this is TRUE, that doesn't necessarily mean that a particular part of the randomizer
// will be enabled.
#define RANDOMIZER_AVAILABLE                   TRUE
#define RANDOMIZER_SEED_IS_TRAINER_ID          TRUE

#if RANDOMIZER_AVAILABLE == TRUE

// If TRUE, the trainer ID (including secret ID) will be the randomizer seed.
#define RZ_TRAINER_ID_IS_SEED       TRUE

// If TRUE, dynamically generated randomization tables stored in EWRAM are used.
// This consumes 6 bytes for each species present.
#define RANDOMIZER_DYNAMIC_SPECIES    TRUE

#if RANDOMIZER_DYNAMIC_SPECIES == TRUE

// If the longest evolutionary chain (excluding babies) is longer than this,
// the dynamic evolutionary stage randomization table will be generated
// incorrectly.
#define RANDOMIZER_MAX_EVO_STAGES   5

#endif // RANDOMIZER_DYNAMIC_SPECIES

#define RANDOMIZER_MAX_TM           ITEM_TM50

// If TRUE, allow a randomizer item pool with zero entries.
#define ALLOW_EMPTY_ITEM_POOL       FALSE

// Vars and features

// These features allow you to force enable or disable individual randomization
// features.
// If undefined, the feature will be enabled if one of the flags below is set.
// If defined and set to TRUE, the feature will always be enabled.
// If defined and set to FALSE, the feature will always be disabled.
//#define FORCE_RANDOMIZE_WILD_MON      TRUE
//#define FORCE_RANDOMIZE_FIELD_ITEMS   TRUE
//#define FORCE_RANDOMIZE_TRAINER_MON   TRUE
//#define FORCE_RANDOMIZE_FIXED_MON     TRUE
//#define FORCE_RANDOMIZE_STARTERS      TRUE

// These flags control whether a particular randomization feature is active.
// They are ignored and disabled if the flags above are set.
#ifndef FORCE_RANDOMIZE_WILD_MON
#define RANDOMIZER_FLAG_WILD_MON            FLAG_RANDOMIZER_AREA_WL
#endif

#ifndef FORCE_RANDOMIZE_FIELD_ITEMS
#define RANDOMIZER_FLAG_FIELD_ITEMS         FLAG_UNUSED_0x021
#endif

#ifndef FORCE_RANDOMIZE_TRAINER_MON
#define RANDOMIZER_FLAG_TRAINER_MON         FLAG_UNUSED_0x022
#endif

#ifndef FORCE_RANDOMIZE_FIXED_MON
#define RANDOMIZER_FLAG_FIXED_MON           FLAG_UNUSED_0x023
#endif

#ifndef FORCE_RANDOMIZE_STARTERS
#define RANDOMIZER_FLAG_STARTERS            FLAG_UNUSED_0x024
#endif

#define RANDOMIZER_VAR_SPECIES_MODE         VAR_UNUSED_0x404E

#if RANDOMIZER_SEED_IS_TRAINER_ID == FALSE
#define RANDOMIZER_VAR_SEED_L               VAR_UNUSED_0x40FA
#define RANDOMIZER_VAR_SEED_H               VAR_UNUSED_0x40FB
#endif

// Special overrides mode: ANDならカテゴリ重複は全て許可されている必要がある。ORならどれか1つ許可で通る。
#define RANDOMIZER_SPECIAL_MODE_OR   0
#define RANDOMIZER_SPECIAL_MODE_AND  1
#define RANDOMIZER_SPECIAL_OVERRIDES_MODE_DEFAULT RANDOMIZER_SPECIAL_MODE_AND
// 0=AND,1=OR を入れる可変スロット（未使用ならデフォルトを使う）
#define VAR_RANDOMIZER_SPECIAL_MODE  VAR_UNUSED_0x40F7

// 本番で例外マップ/バニラ許可を無効化するスイッチ（TRUEなら例外リストを無視）。
#define RANDOMIZER_DISABLE_EXCEPTION_MAPS   FALSE // デバッグのみ例外マップを使う想定。本番は必ずTRUE。
// 例外ヒット時はバニラ遭遇をそのまま許可する（カテゴリフィルタも無視）。
#define RANDOMIZER_EXCEPTION_BYPASS_SPECIAL_FILTER TRUE

#endif // RANDOMIZER_AVAILABLE

#endif // GUARD_CONFIG_RANDOMIZER_H
