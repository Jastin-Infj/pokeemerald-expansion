#ifndef GUARD_POKEMON_VENDOR_H
#define GUARD_POKEMON_VENDOR_H

#include "constants/global.h"
#include "constants/pokemon_vendor.h"

struct Pokemon;

struct PokemonVendorProduct
{
    u16 productId;
    u16 species;
    u32 price;
    u16 oneTimeFlag;
    u16 unlockFlag;
    u16 heldItem;
    u16 ball;
    u16 bondThreshold;
    u16 bondYield;
    u16 moves[MAX_MON_MOVES];
    u8 level;
    u8 kind;
    u8 purchaseMode;
    u8 editPolicy;
    u8 revealPolicy;
    u8 ivs;
} __attribute__((packed));

void CreatePokemonVendorMenu(const struct PokemonVendorProduct *productsForSale);
bool32 PokemonVendor_IsSealedOriginMon(struct Pokemon *mon);
bool32 PokemonVendor_IsLockedSealedRecruit(struct Pokemon *mon);
bool32 PokemonVendor_IsEditEntitled(struct Pokemon *mon);
u8 PokemonVendor_GetSealedRecruitBondProgress(struct Pokemon *mon);
u8 PokemonVendor_GetSealedRecruitBondThreshold(struct Pokemon *mon);
bool32 PokemonVendor_AddBondExp(struct Pokemon *mon, u8 amount);
void PokemonVendor_AddBondExpToParty(void);
void PokemonVendor_IsSelectedMonSealedOrigin(void);
void PokemonVendor_IsSelectedMonLockedSealed(void);

#endif // GUARD_POKEMON_VENDOR_H
