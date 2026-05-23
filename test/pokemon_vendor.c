#include "global.h"
#include "pokemon.h"
#include "pokemon_vendor.h"
#include "test/test.h"
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
    u8 progress = 0;
    u8 threshold = 80;

    CreateMonWithIVs(&mon, SPECIES_DRAGONITE, 50, 0, OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);
    GiveMonInitialMoveset(&mon);
    SetMonData(&mon, MON_DATA_IS_EGG, &isEgg);
    SetMonData(&mon, MON_DATA_VENDOR_SEALED_ORIGIN, &origin);
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
    EXPECT_EQ(PokemonVendor_IsEditEntitled(&mon), TRUE);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_IS_EGG), FALSE);
}

TEST("Pokemon Vendor locked sealed recruit displays as the real species")
{
    struct Pokemon mon;
    struct BoxPokemon *boxMon = &mon.box;
    u8 isEgg = TRUE;
    u8 origin = TRUE;

    CreateMonWithIVs(&mon, SPECIES_DRAGONITE, 50, 0, OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);
    SetMonData(&mon, MON_DATA_IS_EGG, &isEgg);
    SetMonData(&mon, MON_DATA_VENDOR_SEALED_ORIGIN, &origin);

    EXPECT_EQ(PokemonVendor_IsLockedSealedRecruit(&mon), TRUE);
    EXPECT_EQ(PokemonVendor_IsLockedSealedBoxMon(boxMon), TRUE);
    EXPECT_EQ(PokemonVendor_ShouldDisplayMonAsEgg(&mon), FALSE);
    EXPECT_EQ(PokemonVendor_ShouldDisplayBoxMonAsEgg(boxMon), FALSE);
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
