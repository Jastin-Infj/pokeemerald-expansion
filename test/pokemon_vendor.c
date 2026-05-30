#include "global.h"
#include "event_data.h"
#include "party_menu.h"
#include "pokemon.h"
#include "pokemon_vendor.h"
#include "string_util.h"
#include "strings.h"
#include "test/test.h"
#include "trainer_battle_selection.h"
#include "constants/pokemon_vendor.h"
#include "constants/species.h"

TEST("Pokemon Vendor product ABI matches script macro layout")
{
    EXPECT_EQ(sizeof(struct PokemonVendorProduct), 42);
}

TEST("Pokemon Vendor sealed recruit keeps origin and unlocks by bond")
{
    struct Pokemon mon;
    u8 isEgg = TRUE;
    u8 origin = TRUE;
    u8 concealed = TRUE;
    u8 progress = 0;
    u8 threshold = 80;

    CreateMonWithIVs(&mon, SPECIES_DRAGONITE, 50, 0, OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);
    GiveMonInitialMoveset(&mon);
    SetMonData(&mon, MON_DATA_IS_EGG, &isEgg);
    SetMonData(&mon, MON_DATA_VENDOR_SEALED_ORIGIN, &origin);
    SetMonData(&mon, MON_DATA_VENDOR_SEALED_CONCEALED, &concealed);
    SetMonData(&mon, MON_DATA_FRIENDSHIP, &progress);
    SetMonData(&mon, MON_DATA_SHEEN, &threshold);

    EXPECT_EQ(PokemonVendor_IsSealedOriginMon(&mon), TRUE);
    EXPECT_EQ(PokemonVendor_IsLockedSealedRecruit(&mon), TRUE);
    EXPECT_EQ(PokemonVendor_IsEditEntitled(&mon), FALSE);
    EXPECT_EQ(PokemonVendor_GetSealedRecruitBondThreshold(&mon), threshold);

    EXPECT_EQ(PokemonVendor_AddBondExp(&mon, 40), FALSE);
    EXPECT_EQ(PokemonVendor_IsLockedSealedRecruit(&mon), TRUE);
    EXPECT_EQ(PokemonVendor_GetSealedRecruitBondProgress(&mon), 40);

    EXPECT_EQ(PokemonVendor_AddBondExp(&mon, 40), TRUE);
    EXPECT_EQ(PokemonVendor_IsSealedOriginMon(&mon), TRUE);
    EXPECT_EQ(PokemonVendor_IsLockedSealedRecruit(&mon), FALSE);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_VENDOR_SEALED_CONCEALED), FALSE);
    EXPECT_EQ(PokemonVendor_IsEditEntitled(&mon), TRUE);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_IS_EGG), FALSE);
}

TEST("Pokemon Vendor locked sealed recruit displays as the real species")
{
    struct Pokemon mon;
    struct BoxPokemon *boxMon = &mon.box;
    u8 nickname[POKEMON_NAME_LENGTH + 1];
    u8 isEgg = TRUE;
    u8 origin = TRUE;
    u8 concealed = FALSE;

    CreateMonWithIVs(&mon, SPECIES_DRAGONITE, 50, 0, OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);
    SetMonData(&mon, MON_DATA_IS_EGG, &isEgg);
    SetMonData(&mon, MON_DATA_VENDOR_SEALED_ORIGIN, &origin);
    SetMonData(&mon, MON_DATA_VENDOR_SEALED_CONCEALED, &concealed);

    EXPECT_EQ(PokemonVendor_IsLockedSealedRecruit(&mon), TRUE);
    EXPECT_EQ(PokemonVendor_IsLockedSealedBoxMon(boxMon), TRUE);
    EXPECT_EQ(PokemonVendor_IsConcealedSealedRecruit(&mon), FALSE);
    EXPECT_EQ(PokemonVendor_ShouldDisplayMonAsEgg(&mon), FALSE);
    EXPECT_EQ(PokemonVendor_ShouldDisplayBoxMonAsEgg(boxMon), FALSE);

    GetMonData(&mon, MON_DATA_NICKNAME, nickname);
    EXPECT(StringCompare(nickname, GetSpeciesName(SPECIES_DRAGONITE)) == 0);
}

TEST("Pokemon Vendor concealed locked sealed recruit keeps Egg visuals")
{
    struct Pokemon mon;
    struct BoxPokemon *boxMon = &mon.box;
    u8 nickname[POKEMON_NAME_LENGTH + 1];
    u8 isEgg = TRUE;
    u8 origin = TRUE;
    u8 concealed = TRUE;

    CreateMonWithIVs(&mon, SPECIES_MEW, 50, 0, OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);
    SetMonData(&mon, MON_DATA_IS_EGG, &isEgg);
    SetMonData(&mon, MON_DATA_VENDOR_SEALED_ORIGIN, &origin);
    SetMonData(&mon, MON_DATA_VENDOR_SEALED_CONCEALED, &concealed);

    EXPECT_EQ(PokemonVendor_IsLockedSealedRecruit(&mon), TRUE);
    EXPECT_EQ(PokemonVendor_IsLockedSealedBoxMon(boxMon), TRUE);
    EXPECT_EQ(PokemonVendor_IsConcealedSealedRecruit(&mon), TRUE);
    EXPECT_EQ(PokemonVendor_ShouldDisplayMonAsEgg(&mon), TRUE);
    EXPECT_EQ(PokemonVendor_ShouldDisplayBoxMonAsEgg(boxMon), TRUE);

    GetMonData(&mon, MON_DATA_NICKNAME, nickname);
    EXPECT(StringCompare(nickname, gText_EggNickname) == 0);
}

TEST("Pokemon Vendor display helper preserves ordinary Egg display")
{
    struct Pokemon mon;
    u8 isEgg = TRUE;

    CreateMonWithIVs(&mon, SPECIES_PICHU, 1, 0, OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);
    SetMonData(&mon, MON_DATA_IS_EGG, &isEgg);

    EXPECT_EQ(PokemonVendor_IsLockedSealedRecruit(&mon), FALSE);
    EXPECT_EQ(PokemonVendor_ShouldDisplayMonAsEgg(&mon), TRUE);
    EXPECT_EQ(PokemonVendor_ShouldDisplayBoxMonAsEgg(&mon.box), TRUE);
}

TEST("Pokemon Vendor bond rewards apply to the original party during trainer battle selection")
{
    u8 isEgg = TRUE;
    u8 origin = TRUE;
    u8 concealed = FALSE;
    u8 progress = 0;
    u8 threshold = 20;

    for (u32 i = 0; i < PARTY_SIZE; i++)
        ZeroMonData(&gPlayerParty[i]);

    CreateMonWithIVs(&gPlayerParty[0], SPECIES_TREECKO, 50, 0, OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);
    CreateMonWithIVs(&gPlayerParty[1], SPECIES_DRAGONITE, 50, 0, OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);
    SetMonData(&gPlayerParty[1], MON_DATA_IS_EGG, &isEgg);
    SetMonData(&gPlayerParty[1], MON_DATA_VENDOR_SEALED_ORIGIN, &origin);
    SetMonData(&gPlayerParty[1], MON_DATA_VENDOR_SEALED_CONCEALED, &concealed);
    SetMonData(&gPlayerParty[1], MON_DATA_FRIENDSHIP, &progress);
    SetMonData(&gPlayerParty[1], MON_DATA_SHEEN, &threshold);
    CreateMonWithIVs(&gPlayerParty[2], SPECIES_MUDKIP, 50, 0, OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);
    CalculatePlayerPartyCount();

    ClearSelectedPartyOrder();
    gSelectedOrderFromParty[0] = 1;
    TrainerBattleSelection_StartBattleFromSelection();
    EXPECT_EQ(gPlayerPartyCount, 1);

    gSpecialVar_0x8004 = 20;
    PokemonVendor_AddBondExpToParty();
    EXPECT_EQ(gSpecialVar_0x8005, 1);
    EXPECT_EQ(gSpecialVar_Result, 1);
    EXPECT_EQ(PokemonVendor_IsLockedSealedRecruit(&gPlayerParty[0]), FALSE);

    TrainerBattleSelection_RestoreIfActive();
    EXPECT_EQ(gPlayerPartyCount, 3);
    EXPECT_EQ(PokemonVendor_IsSealedOriginMon(&gPlayerParty[1]), TRUE);
    EXPECT_EQ(PokemonVendor_IsLockedSealedRecruit(&gPlayerParty[1]), FALSE);
    EXPECT_EQ(PokemonVendor_IsEditEntitled(&gPlayerParty[1]), TRUE);

    ClearSelectedPartyOrder();
}

TEST("Pokemon Vendor bond rewards survive restoring a selected trainer battle mon")
{
    u8 isEgg = TRUE;
    u8 origin = TRUE;
    u8 concealed = FALSE;
    u8 progress = 0;
    u8 threshold = 20;

    for (u32 i = 0; i < PARTY_SIZE; i++)
        ZeroMonData(&gPlayerParty[i]);

    CreateMonWithIVs(&gPlayerParty[0], SPECIES_DRAGONITE, 50, 0, OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);
    SetMonData(&gPlayerParty[0], MON_DATA_IS_EGG, &isEgg);
    SetMonData(&gPlayerParty[0], MON_DATA_VENDOR_SEALED_ORIGIN, &origin);
    SetMonData(&gPlayerParty[0], MON_DATA_VENDOR_SEALED_CONCEALED, &concealed);
    SetMonData(&gPlayerParty[0], MON_DATA_FRIENDSHIP, &progress);
    SetMonData(&gPlayerParty[0], MON_DATA_SHEEN, &threshold);
    CreateMonWithIVs(&gPlayerParty[1], SPECIES_TREECKO, 50, 0, OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);
    CalculatePlayerPartyCount();

    ClearSelectedPartyOrder();
    gSelectedOrderFromParty[0] = 1;
    TrainerBattleSelection_StartBattleFromSelection();
    EXPECT_EQ(gPlayerPartyCount, 1);
    EXPECT_EQ(PokemonVendor_IsLockedSealedRecruit(&gPlayerParty[0]), TRUE);

    gSpecialVar_0x8004 = 20;
    PokemonVendor_AddBondExpToParty();
    EXPECT_EQ(gSpecialVar_0x8005, 1);
    EXPECT_EQ(gSpecialVar_Result, 1);
    EXPECT_EQ(PokemonVendor_IsLockedSealedRecruit(&gPlayerParty[0]), FALSE);

    TrainerBattleSelection_RestoreIfActive();
    EXPECT_EQ(gPlayerPartyCount, 2);
    EXPECT_EQ(PokemonVendor_IsSealedOriginMon(&gPlayerParty[0]), TRUE);
    EXPECT_EQ(PokemonVendor_IsLockedSealedRecruit(&gPlayerParty[0]), FALSE);
    EXPECT_EQ(PokemonVendor_IsEditEntitled(&gPlayerParty[0]), TRUE);

    ClearSelectedPartyOrder();
}
