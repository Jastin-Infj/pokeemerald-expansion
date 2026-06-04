#include "global.h"
#include "test/test.h"
#include "data.h"
#include "constants/items.h"
#include "constants/pokemon.h"

#if B_CHAMPIONS_PARTYGEN_TRAINERS

#define PARTYGEN_GIMMICK_FIXTURE_TRAINER 15

static bool32 IsChampionsPartyGenMegaItem(enum Item item)
{
    switch (item)
    {
    case ITEM_ALAKAZITE:
    case ITEM_GALLADITE:
    case ITEM_GARDEVOIRITE:
    case ITEM_METAGROSSITE:
        return TRUE;
    default:
        return FALSE;
    }
}

static bool32 IsChampionsPartyGenZItem(enum Item item)
{
    switch (item)
    {
    case ITEM_FAIRIUM_Z:
    case ITEM_NORMALIUM_Z:
    case ITEM_PSYCHIUM_Z:
    case ITEM_WATERIUM_Z:
        return TRUE;
    default:
        return FALSE;
    }
}

TEST("Champions PartyGen trainerproc fixture includes double gimmick coverage")
{
    const struct Trainer *trainer = GetTrainerStructFromId(PARTYGEN_GIMMICK_FIXTURE_TRAINER);
    bool32 hasMega = FALSE;
    bool32 hasZMove = FALSE;
    bool32 hasDynamax = FALSE;
    bool32 hasGigantamax = FALSE;
    bool32 hasTera = FALSE;
    bool32 hasNoGimmick = FALSE;
    bool32 hasMegaTera = FALSE;
    bool32 hasMegaDynamaxTera = FALSE;
    bool32 hasZDynamax = FALSE;
    bool32 hasZTera = FALSE;
    bool32 hasZDynamaxTera = FALSE;
    bool32 hasDynamaxTera = FALSE;
    bool32 hasGigantamaxTera = FALSE;
    bool32 hasSpreadMove = FALSE;

    EXPECT_EQ((u32)trainer->battleType, TRAINER_BATTLE_TYPE_DOUBLES);
    EXPECT_EQ((u32)trainer->partySize, 4);
    EXPECT(trainer->poolSize >= 8);

    for (u32 i = 0; i < trainer->poolSize; i++)
    {
        const struct TrainerMon *mon = &trainer->party[i];
        bool32 isMega = IsChampionsPartyGenMegaItem(mon->heldItem);
        bool32 isZMove = IsChampionsPartyGenZItem(mon->heldItem);
        bool32 hasExplicitTera = mon->teraType != TYPE_NONE;

        hasMega |= isMega;
        hasZMove |= isZMove;
        hasDynamax |= mon->shouldUseDynamax;
        hasGigantamax |= mon->gigantamaxFactor;
        hasTera |= hasExplicitTera;
        hasNoGimmick |= !isMega && !isZMove && !mon->shouldUseDynamax && !mon->gigantamaxFactor && !hasExplicitTera;
        hasMegaTera |= isMega && hasExplicitTera;
        hasMegaDynamaxTera |= isMega && mon->shouldUseDynamax && hasExplicitTera;
        hasZDynamax |= isZMove && mon->shouldUseDynamax;
        hasZTera |= isZMove && hasExplicitTera;
        hasZDynamaxTera |= isZMove && mon->shouldUseDynamax && hasExplicitTera;
        hasDynamaxTera |= mon->shouldUseDynamax && hasExplicitTera;
        hasGigantamaxTera |= mon->gigantamaxFactor && hasExplicitTera;

        for (u32 j = 0; j < MAX_MON_MOVES; j++)
        {
            switch (mon->moves[j])
            {
            case MOVE_EARTHQUAKE:
            case MOVE_MUDDY_WATER:
            case MOVE_ROCK_SLIDE:
            case MOVE_SURF:
                hasSpreadMove = TRUE;
                break;
            default:
                break;
            }
        }
    }

    EXPECT(hasMega);
    EXPECT(hasZMove);
    EXPECT(hasDynamax);
    EXPECT(hasGigantamax);
    EXPECT(hasTera);
    EXPECT(hasNoGimmick);
    EXPECT(hasMegaTera);
    EXPECT(hasMegaDynamaxTera);
    EXPECT(hasZDynamax);
    EXPECT(hasZTera);
    EXPECT(hasZDynamaxTera);
    EXPECT(hasDynamaxTera);
    EXPECT(hasGigantamaxTera);
    EXPECT(hasSpreadMove);
}

#endif
