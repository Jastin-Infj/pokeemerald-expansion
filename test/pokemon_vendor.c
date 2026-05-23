#include "global.h"
#include "pokemon.h"
#include "pokemon_vendor.h"
#include "test/test.h"
#include "constants/pokemon_vendor.h"
#include "constants/species.h"

TEST("Pokemon Vendor product ABI matches script macro layout")
{
    EXPECT_EQ(sizeof(struct PokemonVendorProduct), 34);
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
    EXPECT_EQ(PokemonVendor_IsEditEntitled(&mon), TRUE);
    EXPECT_EQ(PokemonVendor_GetSealedRecruitBondThreshold(&mon), threshold);

    EXPECT_EQ(PokemonVendor_AddBondExp(&mon, 40), FALSE);
    EXPECT_EQ(PokemonVendor_IsLockedSealedRecruit(&mon), TRUE);
    EXPECT_EQ(PokemonVendor_GetSealedRecruitBondProgress(&mon), 40);

    EXPECT_EQ(PokemonVendor_AddBondExp(&mon, 40), TRUE);
    EXPECT_EQ(PokemonVendor_IsSealedOriginMon(&mon), TRUE);
    EXPECT_EQ(PokemonVendor_IsLockedSealedRecruit(&mon), FALSE);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_IS_EGG), FALSE);
}
