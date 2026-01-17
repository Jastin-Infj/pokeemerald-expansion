#include "global.h"
#include "trainer_rank.h"
#include "trainer_rank_parties.h"

static const struct TrainerRankSpec *FindSpec(u16 trainerId)
{
    for (int i = 0; i < TRAINER_RANK_SPEC_COUNT; i++)
    {
        if (gTrainerRankSpecs[i].trainerId == trainerId)
            return &gTrainerRankSpecs[i];
    }
    return NULL;
}

bool32 TrainerRank_IsBlacklisted(u16 trainerId)
{
    for (int i = 0; i < TRAINER_RANK_SKIP_COUNT; i++)
    {
        if (gTrainerRankSkipList[i] == trainerId)
            return TRUE;
    }
    return FALSE;
}

bool32 TrainerRank_GetSpec(u16 trainerId, struct TrainerRankSpecView *out)
{
    if (out == NULL)
        return FALSE;

    const struct TrainerRankSpec *spec = FindSpec(trainerId);
    if (spec == NULL)
    {
        memset(out, 0, sizeof(*out));
        return FALSE;
    }

    memset(out, 0, sizeof(*out));

    if (spec->normalRank != 0 && spec->normalCount > 0)
    {
        out->hasNormal = TRUE;
        out->normalRank = spec->normalRank;
        out->normalCount = spec->normalCount;
    }
    if (spec->rareRank != 0 && spec->rareCount > 0)
    {
        out->hasRare = TRUE;
        out->rareRank = spec->rareRank;
        out->rareCount = spec->rareCount;
    }
    out->allowDuplicates = spec->allowDuplicates;
    out->maxSame = spec->maxSame;
    out->useTrainerPool = spec->useTrainerPool;
    out->trainerPoolCount = spec->trainerPoolCount;
    out->trainerPoolIndex = spec->trainerPoolIndex;
    return TRUE;
}

char TrainerRank_SentinelToRank(u16 species)
{
    switch (species)
    {
    case TRAINER_RANK_SENTINEL_S:
        return 'S';
    case TRAINER_RANK_SENTINEL_A:
        return 'A';
    case TRAINER_RANK_SENTINEL_B:
        return 'B';
    case TRAINER_RANK_SENTINEL_C:
        return 'C';
    case TRAINER_RANK_SENTINEL_D:
        return 'D';
    case TRAINER_RANK_SENTINEL_E:
        return 'E';
    default:
        return 0;
    }
}

bool32 TrainerRank_IsRankSentinel(u16 species)
{
    return TrainerRank_SentinelToRank(species) != 0;
}

u16 TrainerRank_RankToSentinel(char rank)
{
    switch (rank)
    {
    case 'S':
        return TRAINER_RANK_SENTINEL_S;
    case 'A':
        return TRAINER_RANK_SENTINEL_A;
    case 'B':
        return TRAINER_RANK_SENTINEL_B;
    case 'C':
        return TRAINER_RANK_SENTINEL_C;
    case 'D':
        return TRAINER_RANK_SENTINEL_D;
    case 'E':
        return TRAINER_RANK_SENTINEL_E;
    default:
        return 0;
    }
}
