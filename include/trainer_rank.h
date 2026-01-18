#pragma once

#include "global.h"

struct TrainerRankSpecView
{
    bool8 hasNormal;
    char normalRank;   // 'S'〜'E'
    u8 normalCount;    // 0 if unset
    bool8 hasRare;
    char rareRank;     // 'S'〜'E'
    u8 rareCount;      // 0 if unset
    bool8 allowDuplicates;
    u8 maxSame;        // 0 = unlimited (when allowDuplicates)
    bool8 useTrainerPool;
    u8 trainerPoolCount; // 0 if unset
    u16 trainerPoolIndex; // 0xFFFF if unset
    u64 leadTags;
    u64 aceTags;
};

struct TrainerRankMon
{
    u16 species;
    u8 level;
    u16 weight;
    u64 tags;
};

struct TrainerRankPoolDef
{
    const char *key;
    const struct TrainerRankMon *mons;
    u16 count;
    u16 weightTotal;
    bool8 isTrainerSpecific;
};

struct TrainerRankTrainerPoolMap
{
    u16 trainerId;
    u16 poolIndex;
};

// Returns TRUE if trainerId is in the rank skip list (completely fixed).
bool32 TrainerRank_IsBlacklisted(u16 trainerId);

// Fills out with the rank spec if present. Returns TRUE if found.
bool32 TrainerRank_GetSpec(u16 trainerId, struct TrainerRankSpecView *out);

// Generated tables (trainer_rank_parties.h)
extern const struct TrainerRankPoolDef gTrainerRankPools[];
extern const struct TrainerRankMon gTrainerRankMonTable[];
extern const u16 gTrainerRankSharedMap[6]; // index by rank letter S/A/B/C/D/E
extern const struct TrainerRankTrainerPoolMap gTrainerRankTrainerPools[];
extern const u16 gTrainerRankTrainerPoolCount;

#define TRAINER_RANK_SENTINEL_BASE 0xF100
#define TRAINER_RANK_SENTINEL_S (TRAINER_RANK_SENTINEL_BASE + 0)
#define TRAINER_RANK_SENTINEL_A (TRAINER_RANK_SENTINEL_BASE + 1)
#define TRAINER_RANK_SENTINEL_B (TRAINER_RANK_SENTINEL_BASE + 2)
#define TRAINER_RANK_SENTINEL_C (TRAINER_RANK_SENTINEL_BASE + 3)
#define TRAINER_RANK_SENTINEL_D (TRAINER_RANK_SENTINEL_BASE + 4)
#define TRAINER_RANK_SENTINEL_E (TRAINER_RANK_SENTINEL_BASE + 5)

char TrainerRank_SentinelToRank(u16 species);
bool32 TrainerRank_IsRankSentinel(u16 species);
u16 TrainerRank_RankToSentinel(char rank);
