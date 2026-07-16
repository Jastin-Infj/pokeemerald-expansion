#include "global.h"
#include "test/test.h"
#include "battle.h"
#include "battle_main.h"
#include "data.h"
#include "malloc.h"
#include "pokemon.h"
#include "random.h"
#include "string_util.h"
#include "trainer_pools.h"
#include "constants/item.h"
#include "constants/abilities.h"
#include "constants/trainers.h"
#include "constants/battle.h"
#include "constants/battle_ai.h"
#include "config/battle.h"

TEST("CreateNPCTrainerPartyForTrainer generates customized Pokémon")
{
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 3;
    u8 nickBuffer[20];
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(IsMonShiny(&testParty[0]));
    EXPECT(!IsMonShiny(&testParty[1]));

    EXPECT(GetMonData(&testParty[0], MON_DATA_POKEBALL, 0) == BALL_MASTER);
    EXPECT(GetMonData(&testParty[1], MON_DATA_POKEBALL, 0) == BALL_POKE);

    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES, 0) == SPECIES_WOBBUFFET);
    EXPECT(GetMonData(&testParty[1], MON_DATA_SPECIES, 0) == SPECIES_WOBBUFFET);

    EXPECT(GetMonAbility(&testParty[0]) == ABILITY_TELEPATHY);
    EXPECT(GetMonAbility(&testParty[1]) == ABILITY_SHADOW_TAG);
    EXPECT(GetMonAbility(&testParty[2]) == ABILITY_SHADOW_TAG);

    EXPECT(GetMonData(&testParty[0], MON_DATA_FRIENDSHIP, 0) == 42);
    EXPECT(GetMonData(&testParty[1], MON_DATA_FRIENDSHIP, 0) == 0);

    EXPECT(GetMonData(&testParty[0], MON_DATA_HELD_ITEM, 0) == ITEM_ASSAULT_VEST);
    EXPECT(GetMonData(&testParty[1], MON_DATA_HELD_ITEM, 0) == ITEM_NONE);

    EXPECT(GetMonData(&testParty[0], MON_DATA_HP_IV, 0) == 25);
    EXPECT(GetMonData(&testParty[0], MON_DATA_ATK_IV, 0) == 26);
    EXPECT(GetMonData(&testParty[0], MON_DATA_DEF_IV, 0) == 27);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPEED_IV, 0) == 28);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPATK_IV, 0) == 29);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPDEF_IV, 0) == 30);

    EXPECT(GetMonData(&testParty[1], MON_DATA_HP_IV, 0) == 0);
    EXPECT(GetMonData(&testParty[1], MON_DATA_ATK_IV, 0) == 0);
    EXPECT(GetMonData(&testParty[1], MON_DATA_DEF_IV, 0) == 0);
    EXPECT(GetMonData(&testParty[1], MON_DATA_SPEED_IV, 0) == 0);
    EXPECT(GetMonData(&testParty[1], MON_DATA_SPATK_IV, 0) == 0);
    EXPECT(GetMonData(&testParty[1], MON_DATA_SPDEF_IV, 0) == 0);

    EXPECT(GetMonData(&testParty[0], MON_DATA_HP_EV, 0) == 252);
    EXPECT(GetMonData(&testParty[0], MON_DATA_ATK_EV, 0) == 0);
    EXPECT(GetMonData(&testParty[0], MON_DATA_DEF_EV, 0) == 0);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPEED_EV, 0) == 252);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPATK_EV, 0) == 4);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPDEF_EV, 0) == 0);

    EXPECT(GetMonData(&testParty[1], MON_DATA_HP_EV, 0) == 0);
    EXPECT(GetMonData(&testParty[1], MON_DATA_ATK_EV, 0) == 0);
    EXPECT(GetMonData(&testParty[1], MON_DATA_DEF_EV, 0) == 0);
    EXPECT(GetMonData(&testParty[1], MON_DATA_SPEED_EV, 0) == 0);
    EXPECT(GetMonData(&testParty[1], MON_DATA_SPATK_EV, 0) == 0);
    EXPECT(GetMonData(&testParty[1], MON_DATA_SPDEF_EV, 0) == 0);

    EXPECT(GetMonData(&testParty[0], MON_DATA_LEVEL, 0) == 67);
    EXPECT(GetMonData(&testParty[1], MON_DATA_LEVEL, 0) == 5);

    EXPECT(GetMonData(&testParty[0], MON_DATA_MOVE1, 0) == MOVE_AIR_SLASH);
    EXPECT(GetMonData(&testParty[0], MON_DATA_MOVE2, 0) == MOVE_BARRIER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_MOVE3, 0) == MOVE_SOLAR_BEAM);
    EXPECT(GetMonData(&testParty[0], MON_DATA_MOVE4, 0) == MOVE_EXPLOSION);

    GetMonData(&testParty[0], MON_DATA_NICKNAME, nickBuffer);
    EXPECT(StringCompare(nickBuffer, COMPOUND_STRING("Bubbles")) == 0);

    GetMonData(&testParty[1], MON_DATA_NICKNAME, nickBuffer);
    EXPECT(StringCompare(nickBuffer, COMPOUND_STRING("Wobbuffet")) == 0);

    EXPECT(GetMonGender(&testParty[0]) == MON_FEMALE);
    EXPECT(GetNature(&testParty[0]) == NATURE_HASTY);
    EXPECT(GetNature(&testParty[1]) == NATURE_HARDY);

    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_DYNAMAX_LEVEL), 5);
    EXPECT_EQ(GetMonData(&testParty[1], MON_DATA_DYNAMAX_LEVEL), 10);

    Free(testParty);
}

TEST("CreateNPCTrainerPartyForTrainer generates different personalities for different mons")
{
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 3;
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(testParty[0].box.personality != testParty[1].box.personality);
    Free(testParty);
}

TEST("ModifyPersonalityForNature can set any nature")
{
    u32 personality = 0, nature = 0, j = 0, k = 0;
    for (j = 0; j < 64; j++)
    {
        for (k = 0; k < NUM_NATURES; k++)
        {
            PARAMETRIZE { personality = Random32(); nature = k; }
        }
    }
    ModifyPersonalityForNature(&personality, nature);
    EXPECT_EQ(GetNatureFromPersonality(personality), nature);
}

TEST("Trainer Class Balls apply to the entire party")
{
    ASSUME(B_TRAINER_CLASS_POKE_BALLS >= GEN_8);
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 j;
    u32 currTrainer = 14;
    const struct Trainer *trainer = GetTrainerStructFromId(currTrainer);
    CreateNPCTrainerPartyFromTrainer(testParty, trainer, FALSE, BATTLE_TYPE_TRAINER);
    for(j = 0; j < 6; j++)
    {
        EXPECT(GetMonData(&testParty[j], MON_DATA_POKEBALL, 0) == gTrainerClasses[trainer->trainerClass].ball);
    }
    Free(testParty);
}

TEST("Difficulty default to Normal if the trainer doesn't have a member for the current difficulty")
{
    SetCurrentDifficultyLevel(DIFFICULTY_EASY);
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 4;
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_MEWTWO);
    Free(testParty);
    SetCurrentDifficultyLevel(DIFFICULTY_NORMAL);
}

TEST("Difficulty changes which party is used for enemy trainer if defined for the difficulty (EASY)")
{
    SetCurrentDifficultyLevel(DIFFICULTY_EASY);
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 5;
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_METAPOD);
    EXPECT(GetMonData(&testParty[0], MON_DATA_LEVEL) == 1);
    Free(testParty);
    SetCurrentDifficultyLevel(DIFFICULTY_NORMAL);
}

TEST("Difficulty changes which party is used for enemy trainer if defined for the difficulty (HARD)")
{
    SetCurrentDifficultyLevel(DIFFICULTY_HARD);
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 5;
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_ARCEUS);
    EXPECT(GetMonData(&testParty[0], MON_DATA_LEVEL) == 99);
    Free(testParty);
    SetCurrentDifficultyLevel(DIFFICULTY_NORMAL);
}

TEST("Difficulty changes which party is used for enemy trainer if defined for the difficulty (NORMAL)")
{
    SetCurrentDifficultyLevel(DIFFICULTY_NORMAL);
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 5;
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_MEWTWO);
    EXPECT(GetMonData(&testParty[0], MON_DATA_LEVEL) == 50);
    Free(testParty);
}

TEST("Difficulty default to Normal if the partner doesn't have a member for the current difficulty")
{
    SetCurrentDifficultyLevel(DIFFICULTY_TEST);
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = TRAINER_PARTNER(1);
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_METANG);
    EXPECT(GetMonData(&testParty[0], MON_DATA_LEVEL) == 42);
    Free(testParty);
    SetCurrentDifficultyLevel(DIFFICULTY_NORMAL);
}

TEST("Difficulty changes which party is used for partner if defined for the difficulty (EASY)")
{
    SetCurrentDifficultyLevel(DIFFICULTY_EASY);
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = TRAINER_PARTNER(1);
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_METAPOD);
    EXPECT(GetMonData(&testParty[0], MON_DATA_LEVEL) == 1);
    Free(testParty);
    SetCurrentDifficultyLevel(DIFFICULTY_NORMAL);
}

TEST("Difficulty changes which party is used for partner if defined for the difficulty (HARD)")
{
    SetCurrentDifficultyLevel(DIFFICULTY_HARD);
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = TRAINER_PARTNER(1);
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_ARCEUS);
    EXPECT(GetMonData(&testParty[0], MON_DATA_LEVEL) == 99);
    Free(testParty);
    SetCurrentDifficultyLevel(DIFFICULTY_NORMAL);
}

TEST("Difficulty changes which party is used for partner if defined for the difficulty (NORMAL)")
{
    SetCurrentDifficultyLevel(DIFFICULTY_NORMAL);
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = TRAINER_PARTNER(1);
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_METANG);
    EXPECT(GetMonData(&testParty[0], MON_DATA_LEVEL) == 42);
    Free(testParty);
}

TEST("Trainer Party Pool generates a party from the trainer pool")
{
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 6;
    SeedRng(0);
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_EEVEE);
    Free(testParty);
}

TEST("Trainer Party Pool picks a random lead and a random ace if tags exist in the pool")
{
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 7;
    SeedRng(0);
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_WOBBUFFET
        || GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_ARON);    //  Lead
    EXPECT(GetMonData(&testParty[1], MON_DATA_SPECIES) == SPECIES_WYNAUT
        || GetMonData(&testParty[1], MON_DATA_SPECIES) == SPECIES_MEW);     //  Not Lead or Ace
    EXPECT(GetMonData(&testParty[2], MON_DATA_SPECIES) == SPECIES_EEVEE
        || GetMonData(&testParty[2], MON_DATA_SPECIES) == SPECIES_ODDISH);  //  Ace
    Free(testParty);
}

TEST("Trainer Party Pool picks according to custom rules")
{
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 8;
    SeedRng(0);
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_TORKOAL);    //  Lead + Weather Setter
    EXPECT(GetMonData(&testParty[1], MON_DATA_SPECIES) == SPECIES_BULBASAUR);  //  Lead + Weather Abuser
    EXPECT(GetMonData(&testParty[2], MON_DATA_SPECIES) == SPECIES_EEVEE);      //  Anything else
    Free(testParty);
}

TEST("Trainer Party Pool varies across runtime RNG seeds")
{
    ASSUME(B_POOL_SETTING_CONSISTENT_RNG == FALSE);
    ASSUME(B_POOL_SETTING_USE_FIXED_SEED == FALSE);

    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 8;
    u32 firstSpecies[3];
    bool32 sawDifferent = FALSE;

    SeedRng(1);
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE);
    for (u32 i = 0; i < ARRAY_COUNT(firstSpecies); i++)
        firstSpecies[i] = GetMonData(&testParty[i], MON_DATA_SPECIES);

    for (u32 seed = 2; seed < 32 && !sawDifferent; seed++)
    {
        SeedRng(seed);
        CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE);
        for (u32 i = 0; i < ARRAY_COUNT(firstSpecies); i++)
        {
            if (GetMonData(&testParty[i], MON_DATA_SPECIES) != firstSpecies[i])
            {
                sawDifferent = TRUE;
                break;
            }
        }
    }

    EXPECT(sawDifferent);
    Free(testParty);
}

TEST("Trainer Party Pool weights bias eligible tagged candidates")
{
    ASSUME(B_POOL_SETTING_CONSISTENT_RNG == FALSE);
    ASSUME(B_POOL_SETTING_USE_FIXED_SEED == FALSE);

    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 15;
    u32 lowWeightPicks = 0;
    u32 highWeightPicks = 0;

    for (u32 seed = 1; seed <= 64; seed++)
    {
        SeedRng(seed);
        CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
        switch (GetMonData(&testParty[0], MON_DATA_SPECIES))
        {
        case SPECIES_WYNAUT:
            lowWeightPicks++;
            break;
        case SPECIES_EEVEE:
            highWeightPicks++;
            break;
        default:
            EXPECT(FALSE);
            break;
        }
    }

    EXPECT(highWeightPicks > lowWeightPicks);
    Free(testParty);
}

TEST("Trainer Party Pool weights do not override slot tags")
{
    ASSUME(B_POOL_SETTING_CONSISTENT_RNG == FALSE);
    ASSUME(B_POOL_SETTING_USE_FIXED_SEED == FALSE);

    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 16;

    for (u32 seed = 1; seed <= 16; seed++)
    {
        SeedRng(seed);
        CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
        EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_WYNAUT);
    }

    Free(testParty);
}

TEST("Trainer Party Pool uses standard party creation if pool is illegal")
{
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 9;
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_WYNAUT);
    EXPECT(GetMonData(&testParty[1], MON_DATA_SPECIES) == SPECIES_WOBBUFFET);
    Free(testParty);
}

TEST("Trainer Party Pool can be pruned before picking")
{
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 10;
    SeedRng(0);
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_EEVEE);
    EXPECT(GetMonData(&testParty[1], MON_DATA_SPECIES) == SPECIES_WYNAUT);
    Free(testParty);
}

TEST("Trainer Party Pool can choose which functions to use for picking mons")
{
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 11;
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_WYNAUT);
    EXPECT(GetMonData(&testParty[1], MON_DATA_SPECIES) == SPECIES_WOBBUFFET);
    Free(testParty);
}

TEST("Trainer Party Pool can adapt pruning to the opponent party")
{
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 17;

    ZeroPartyMons(gParties[B_TRAINER_PLAYER]);
    CreateMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_JOLTEON, 100, 0, OTID_STRUCT_PRESET(0), 31);
    CreateMonWithIVs(&gParties[B_TRAINER_PLAYER][1], SPECIES_AERODACTYL, 100, 0, OTID_STRUCT_PRESET(1), 31);
    gPartiesCount[B_TRAINER_PLAYER] = 2;

    SeedRng(0);
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), FALSE, BATTLE_TYPE_TRAINER);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_SPECIES), SPECIES_BULBASAUR);
    EXPECT_EQ(GetMonData(&testParty[1], MON_DATA_SPECIES), SPECIES_CHARMANDER);
    EXPECT_EQ(GetMonData(&testParty[2], MON_DATA_SPECIES), SPECIES_SQUIRTLE);

    ZeroPartyMons(gParties[B_TRAINER_PLAYER]);
    CreateMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_KYOGRE, 100, 0, OTID_STRUCT_PRESET(0), 31);
    CreateMonWithIVs(&gParties[B_TRAINER_PLAYER][1], SPECIES_GROUDON, 100, 0, OTID_STRUCT_PRESET(1), 31);
    gPartiesCount[B_TRAINER_PLAYER] = 2;

    SeedRng(0);
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), FALSE, BATTLE_TYPE_TRAINER);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_SPECIES), SPECIES_TORKOAL);
    EXPECT_EQ(GetMonData(&testParty[1], MON_DATA_SPECIES), SPECIES_CHERRIM);
    EXPECT_EQ(GetMonData(&testParty[2], MON_DATA_SPECIES), SPECIES_MEW);

    ZeroPartyMons(gParties[B_TRAINER_PLAYER]);
    CreateMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_DRAGONITE, 100, 0, OTID_STRUCT_PRESET(0), 31);
    CreateMonWithIVs(&gParties[B_TRAINER_PLAYER][1], SPECIES_FARIGIRAF, 100, 0, OTID_STRUCT_PRESET(1), 31);
    SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][0], MOVE_DRAGON_DANCE, 0);
    SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][1], MOVE_TRICK_ROOM, 0);
    gPartiesCount[B_TRAINER_PLAYER] = 2;

    SeedRng(0);
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), FALSE, BATTLE_TYPE_TRAINER);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_SPECIES), SPECIES_WYNAUT);
    EXPECT_EQ(GetMonData(&testParty[1], MON_DATA_SPECIES), SPECIES_WOBBUFFET);
    EXPECT_EQ(GetMonData(&testParty[2], MON_DATA_SPECIES), SPECIES_EEVEE);

    ZeroPartyMons(gParties[B_TRAINER_PLAYER]);
    CreateMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_METAGROSS, 50, 0, OTID_STRUCT_PRESET(0), 31);
    SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][0], MOVE_IRON_HEAD, 0);
    SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][0], MOVE_ZEN_HEADBUTT, 1);
    SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][0], MOVE_BULLET_PUNCH, 2);
    SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][0], MOVE_EARTHQUAKE, 3);
    CreateMonWithIVs(&gParties[B_TRAINER_PLAYER][1], SPECIES_MILOTIC, 50, 0, OTID_STRUCT_PRESET(1), 31);
    SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][1], MOVE_SCALD, 0);
    SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][1], MOVE_ICE_BEAM, 1);
    SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][1], MOVE_RECOVER, 2);
    SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][1], MOVE_PROTECT, 3);
    CreateMonWithIVs(&gParties[B_TRAINER_PLAYER][2], SPECIES_BRELOOM, 50, 0, OTID_STRUCT_PRESET(2), 31);
    SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][2], MOVE_SPORE, 0);
    SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][2], MOVE_MACH_PUNCH, 1);
    SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][2], MOVE_BULLET_SEED, 2);
    SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][2], MOVE_PROTECT, 3);
    gPartiesCount[B_TRAINER_PLAYER] = 3;

    SeedRng(0);
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), FALSE, BATTLE_TYPE_TRAINER);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_SPECIES), SPECIES_WYNAUT);
    EXPECT_EQ(GetMonData(&testParty[1], MON_DATA_SPECIES), SPECIES_WOBBUFFET);
    EXPECT_EQ(GetMonData(&testParty[2], MON_DATA_SPECIES), SPECIES_EEVEE);

    ZeroPartyMons(gParties[B_TRAINER_PLAYER]);
    Free(testParty);
}

TEST("trainerproc supports both Double Battle: Yes and Battle Type: Doubles")
{
    u32 currTrainer;
    PARAMETRIZE { currTrainer = 12; }
    PARAMETRIZE { currTrainer = 13; }
    const struct Trainer *trainer = GetTrainerStructFromId(currTrainer);
    EXPECT(trainer->battleType == TRAINER_BATTLE_TYPE_DOUBLES);
}

TEST("CreateNPCTrainerPartyForTrainer generates default moves if no moves are specified")
{
    u32 currTrainer = 1;
    const struct Trainer *trainer = GetTrainerStructFromId(currTrainer);
    ASSUME(trainer->party[0].moves[0] == MOVE_NONE);
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    CreateNPCTrainerPartyFromTrainer(testParty, trainer, TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_MOVE1) != MOVE_NONE);
    Free(testParty);
}
