#include "global.h"
#include "data.h"
#include "item.h"
#include "malloc.h"
#include "pokemon.h"
#include "random.h"
#include "trainer_pools.h"
#include "constants/battle.h"
#include "constants/battle_ai.h"
#include "constants/moves.h"
#include "constants/items.h"
#include "constants/species.h"

#include "data/battle_pool_rules.h"

#define TRAINER_POOL_DEFAULT_WEIGHT 1
#define TRAINER_POOL_MAX_WEIGHT 15

static void HasRequiredTag(const struct Trainer *trainer, u8* poolIndexArray, struct PoolRules *rules, u32 *arrayIndex, bool32 *foundRequiredTag, u32 currIndex)
{
    //  Start from index 2, since lead and ace has special handling
    for (u32 currTag = 2; currTag < POOL_NUM_TAGS; currTag++)
    {
        if (rules->tagRequired[currTag]
         && trainer->party[poolIndexArray[currIndex]].tags & (1u << currTag))
        {
            *arrayIndex = currIndex;
            *foundRequiredTag = TRUE;
            break;
        }
    }
}

static u32 DefaultLeadPickFunction(const struct Trainer *trainer, u8 *poolIndexArray, u32 partyIndex, u32 monsCount, u32 battleTypeFlags, struct PoolRules *rules)
{
    u32 arrayIndex = 0;
    u32 monIndex = POOL_SLOT_DISABLED;
    //  monIndex is set to 255 if nothing has been chosen yet, this gives an upper limit on pool size of 255
    if ((partyIndex == 0)
     || (partyIndex == 1 && (battleTypeFlags & BATTLE_TYPE_DOUBLE)))
    {
        //  Find required + lead tags
        bool32 foundRequiredTag = FALSE;
        u32 firstLeadIndex = POOL_SLOT_DISABLED;
        for (u32 currIndex = 0; currIndex < trainer->poolSize; currIndex++)
        {
            if ((poolIndexArray[currIndex] != POOL_SLOT_DISABLED)
             && (trainer->party[poolIndexArray[currIndex]].tags & (1u << POOL_TAG_LEAD)))
            {
                if (firstLeadIndex == POOL_SLOT_DISABLED)
                    firstLeadIndex = currIndex;
                //  Start from index 2, since lead and ace has special handling
                HasRequiredTag(trainer, poolIndexArray, rules, &arrayIndex, &foundRequiredTag, currIndex);
            }
            if (foundRequiredTag)
                break;
        }
        //  If a combination of required + lead wasn't found, apply the first found lead
        if (foundRequiredTag)
        {
            monIndex = poolIndexArray[arrayIndex];
            poolIndexArray[arrayIndex] = POOL_SLOT_DISABLED;
        }
        else if (firstLeadIndex != POOL_SLOT_DISABLED)
        {
            monIndex = poolIndexArray[firstLeadIndex];
            poolIndexArray[firstLeadIndex] = POOL_SLOT_DISABLED;
        }
    }
    return monIndex;
}

static u32 DefaultAcePickFunction(const struct Trainer *trainer, u8 *poolIndexArray, u32 partyIndex, u32 monsCount, u32 battleTypeFlags, struct PoolRules *rules)
{
    u32 arrayIndex = 0;
    u32 monIndex = POOL_SLOT_DISABLED;
    //  monIndex is set to 255 if nothing has been chosen yet, this gives an upper limit on pool size of 255
    if (((partyIndex == monsCount - 1) || (partyIndex == monsCount - 2 && battleTypeFlags & BATTLE_TYPE_DOUBLE))
     && (rules->tagMaxMembers[1] == POOL_MEMBER_COUNT_UNLIMITED || rules->tagMaxMembers[1] >= 1))
    {
        //  Find required + ace tags
        bool32 foundRequiredTag = FALSE;
        u32 firstAceIndex = POOL_SLOT_DISABLED;
        for (u32 currIndex = 0; currIndex < trainer->poolSize; currIndex++)
        {
            if ((poolIndexArray[currIndex] != POOL_SLOT_DISABLED)
             && (trainer->party[poolIndexArray[currIndex]].tags & (1u << POOL_TAG_ACE)))
            {
                if (firstAceIndex == POOL_SLOT_DISABLED)
                    firstAceIndex = currIndex;
                HasRequiredTag(trainer, poolIndexArray, rules, &arrayIndex, &foundRequiredTag, currIndex);
            }
            if (foundRequiredTag)
                break;
        }
        //  If a combination of required + ace wasn't found, apply the first found lead
        if (foundRequiredTag)
        {
            monIndex = poolIndexArray[arrayIndex];
            poolIndexArray[arrayIndex] = POOL_SLOT_DISABLED;
        }
        else if (firstAceIndex != POOL_SLOT_DISABLED)
        {
            monIndex = poolIndexArray[firstAceIndex];
            poolIndexArray[firstAceIndex] = POOL_SLOT_DISABLED;
        }
    }
    return monIndex;
}

static u32 DefaultOtherPickFunction(const struct Trainer *trainer, u8 *poolIndexArray, u32 partyIndex, u32 monsCount, u32 battleTypeFlags, struct PoolRules *rules)
{
    u32 arrayIndex = 0;
    u32 monIndex = POOL_SLOT_DISABLED;
    //  monIndex is set to 255 if nothing has been chosen yet, this gives an upper limit on pool size of 255
    //  Find required tag
    bool32 foundRequiredTag = FALSE;
    u32 firstUnpickedIndex = POOL_SLOT_DISABLED;
    for (u32 currIndex = 0; currIndex < trainer->poolSize; currIndex++)
    {
        if (poolIndexArray[currIndex] != POOL_SLOT_DISABLED
         && !(trainer->party[poolIndexArray[currIndex]].tags & (1u << POOL_TAG_LEAD))
         && !(trainer->party[poolIndexArray[currIndex]].tags & (1u << POOL_TAG_ACE)))
        {
            if (firstUnpickedIndex == POOL_SLOT_DISABLED)
                firstUnpickedIndex = currIndex;
            HasRequiredTag(trainer, poolIndexArray, rules, &arrayIndex, &foundRequiredTag, currIndex);
        }
        if (foundRequiredTag)
            break;
    }
    //  If a combination of required + ace wasn't found, apply the first found lead
    if (foundRequiredTag)
    {
        monIndex = poolIndexArray[arrayIndex];
        poolIndexArray[arrayIndex] = POOL_SLOT_DISABLED;
    }
    else if (firstUnpickedIndex != POOL_SLOT_DISABLED)
    {
        monIndex = poolIndexArray[firstUnpickedIndex];
        poolIndexArray[firstUnpickedIndex] = POOL_SLOT_DISABLED;
    }
    return monIndex;
}

static u32 PickLowest(const struct Trainer *trainer, u8 *poolIndexArray, u32 partyIndex, u32 monsCount, u32 battleTypeFlags, struct PoolRules *rules)
{
    u32 monIndex = POOL_SLOT_DISABLED;
    u32 lowestIndex = POOL_SLOT_DISABLED;
    for (u32 i = 0; i < trainer->poolSize; i++)
    {
        if (poolIndexArray[i] < monIndex)
        {
            lowestIndex = i;
            monIndex = poolIndexArray[i];
        }
    }
    if (lowestIndex == POOL_SLOT_DISABLED)
        return POOL_SLOT_DISABLED;
    poolIndexArray[lowestIndex] = POOL_SLOT_DISABLED;
    return monIndex;
}

static u32 PickMonFromPool(const struct Trainer *trainer, u8 *poolIndexArray, u32 partyIndex, u32 monsCount, u32 battleTypeFlags, struct PoolRules *rules, struct PickFunctions pickFunctions)
{
    u32 monIndex = POOL_SLOT_DISABLED;
    //  Pick Lead
    if (monIndex == POOL_SLOT_DISABLED)
        monIndex = pickFunctions.LeadFunction(trainer, poolIndexArray, partyIndex, monsCount, battleTypeFlags, rules);
    //  Pick Ace
    if (monIndex == POOL_SLOT_DISABLED)
        monIndex = pickFunctions.AceFunction(trainer, poolIndexArray, partyIndex, monsCount, battleTypeFlags, rules);
    //  If no mon has been found yet continue looking
    if (monIndex == POOL_SLOT_DISABLED)
        monIndex = pickFunctions.OtherFunction(trainer, poolIndexArray, partyIndex, monsCount, battleTypeFlags, rules);
    //  If a mon still hasn't been found, return POOL_SLOT_DISABLED which makes party generation default to regular party generation
    if (monIndex == POOL_SLOT_DISABLED)
        return monIndex;

    u32 chosenTags = trainer->party[monIndex].tags;
    enum Species chosenSpecies = trainer->party[monIndex].species;
    enum Item chosenItem = trainer->party[monIndex].heldItem;
    enum NationalDexOrder chosenNatDex = gSpeciesInfo[chosenSpecies].natDexNum;
    //  If tag was required, change pool rule to account for the required tag already being picked
    u32 tagsToEliminate = 0;
    for (u32 currTag = 0; currTag < POOL_NUM_TAGS; currTag++)
    {
        if (chosenTags & (1u << currTag)
         && rules->tagMaxMembers[currTag] != POOL_MEMBER_COUNT_UNLIMITED)
        {
            if (rules->tagMaxMembers[currTag] == 1)
                rules->tagMaxMembers[currTag] = POOL_MEMBER_COUNT_NONE;
            else
                rules->tagMaxMembers[currTag]--;
        }
        if (chosenTags & (1u << currTag))
            rules->tagRequired[currTag] = FALSE;
        if (rules->tagMaxMembers[currTag] == POOL_MEMBER_COUNT_NONE)
            tagsToEliminate |= 1u << currTag;
    }
    //  If species clause, remove picked species from pool
    //  If item clause, remove all mons with same held item from pool
    //  If matching a tag that's been exhausted, remove from pool
    for (u32 currIndex = 0; currIndex < trainer->poolSize; currIndex++)
    {
        if (poolIndexArray[currIndex] != POOL_SLOT_DISABLED)
        {
            u32 currentTags = trainer->party[poolIndexArray[currIndex]].tags;
            enum Species currentSpecies = trainer->party[poolIndexArray[currIndex]].species;
            enum Item currentItem = trainer->party[poolIndexArray[currIndex]].heldItem;
            enum NationalDexOrder currentNatDex = gSpeciesInfo[currentSpecies].natDexNum;
            if (currentTags & tagsToEliminate)
            {
                poolIndexArray[currIndex] = POOL_SLOT_DISABLED;
            }
            if (rules->speciesClause && chosenSpecies == currentSpecies)
                poolIndexArray[currIndex] = POOL_SLOT_DISABLED;
            if (!rules->excludeForms && chosenNatDex == currentNatDex)
                poolIndexArray[currIndex] = POOL_SLOT_DISABLED;
            if (rules->itemClause && currentItem != ITEM_NONE)
            {
                if (rules->itemClauseExclusions)
                {
                    bool32 isExcluded = FALSE;
                    for (u32 i = 0; i < ARRAY_COUNT(poolItemClauseExclusions); i++)
                    {
                        if (chosenItem == poolItemClauseExclusions[i])
                        {
                            isExcluded = TRUE;
                            break;
                        }
                    }
                    if (!isExcluded)
                        poolIndexArray[currIndex] = POOL_SLOT_DISABLED;
                }
                else if (chosenItem == currentItem)
                {
                    poolIndexArray[currIndex] = POOL_SLOT_DISABLED;
                }
            }
            if (rules->megaStoneClause && gItemsInfo[currentItem].sortType == ITEM_TYPE_MEGA_STONE && gItemsInfo[chosenItem].sortType == ITEM_TYPE_MEGA_STONE)
                poolIndexArray[currIndex] = POOL_SLOT_DISABLED;
            if (rules->zCrystalClause && gItemsInfo[currentItem].sortType == ITEM_TYPE_Z_CRYSTAL && gItemsInfo[chosenItem].sortType == ITEM_TYPE_Z_CRYSTAL)
                poolIndexArray[currIndex] = POOL_SLOT_DISABLED;
        }
    }
    return monIndex;
}

static u32 GetPoolSeed(const struct Trainer *trainer)
{
    u32 seed;
    if (B_POOL_SETTING_USE_FIXED_SEED)
        seed = B_POOL_SETTING_FIXED_SEED;
    else
        seed = READ_OTID_FROM_SAVE;
    seed ^= (u32)trainer;
    return seed;
}

static u32 GetPoolWeight(const struct Trainer *trainer, u32 monIndex)
{
    u32 weight = trainer->party[monIndex].poolWeight;
    if (weight == 0)
        return TRAINER_POOL_DEFAULT_WEIGHT;
    if (weight > TRAINER_POOL_MAX_WEIGHT)
        return TRAINER_POOL_MAX_WEIGHT;
    return weight;
}

static bool32 PoolUsesWeights(const struct Trainer *trainer)
{
    for (u32 i = 0; i < trainer->poolSize; i++)
    {
        u32 weight = GetPoolWeight(trainer, i);
        if (weight != TRAINER_POOL_DEFAULT_WEIGHT)
            return TRUE;
    }
    return FALSE;
}

static u32 NextPoolRandom(bool32 useConsistentRng, rng_value_t *localRngState)
{
    if (useConsistentRng)
        return LocalRandom32(localRngState);
    return Random32();
}

static void RandomizeWeightedPoolIndices(const struct Trainer *trainer, u8 *poolIndexArray)
{
    rng_value_t localRngState;
    bool32 useConsistentRng = B_POOL_SETTING_CONSISTENT_RNG;
    if (useConsistentRng)
        localRngState = LocalRandomSeed(GetPoolSeed(trainer));

    for (u32 i = 0; i < trainer->poolSize - 1; i++)
    {
        u32 weightSum = 0;
        for (u32 j = i; j < trainer->poolSize; j++)
            weightSum += GetPoolWeight(trainer, poolIndexArray[j]);

        u32 selectedWeight = NextPoolRandom(useConsistentRng, &localRngState) % weightSum;
        u32 selectedIndex = i;
        for (u32 j = i; j < trainer->poolSize; j++)
        {
            u32 weight = GetPoolWeight(trainer, poolIndexArray[j]);
            if (selectedWeight < weight)
            {
                selectedIndex = j;
                break;
            }
            selectedWeight -= weight;
        }

        u32 tempValue = poolIndexArray[i];
        poolIndexArray[i] = poolIndexArray[selectedIndex];
        poolIndexArray[selectedIndex] = tempValue;
    }
}

static void RandomizePoolIndices(const struct Trainer *trainer, u8 *poolIndexArray)
{
    //  Basically the modern (Durstenfield's) Fisher-Yates shuffle
    //  Reducing the amount of calls to random needed by only using as many bits as needed per shuffle
    u32 poolSize = trainer->poolSize;
    for (u32 i = 0; i < poolSize; i++)
        poolIndexArray[i] = i;

    if (poolSize <= 1)
        return;

    if (PoolUsesWeights(trainer))
    {
        RandomizeWeightedPoolIndices(trainer, poolIndexArray);
        return;
    }

    u32 rnd;
    rng_value_t localRngState;
    if (B_POOL_SETTING_CONSISTENT_RNG)
    {
        u32 seed = GetPoolSeed(trainer);
        localRngState = LocalRandomSeed(seed);
        //  Replace the LocalRandom with LocalRandom32 when implemented
        rnd = LocalRandom32(&localRngState);
    }
    else
    {
        rnd = Random32();
    }
    u32 usedBits = 0;
    for (u32 i = 0; i < poolSize - 1; i++)
    {
        u32 numBits = 1;
        if (poolSize - i > 127)
            numBits = 8;
        else if (poolSize - i > 63)
            numBits = 7;
        else if (poolSize - i > 31)
            numBits = 6;
        else if (poolSize - i > 15)
            numBits = 5;
        else if (poolSize - i > 7)
            numBits = 4;
        else if (poolSize - i > 3)
            numBits = 3;
        else if (poolSize - i > 1)
            numBits = 2;
        if (usedBits + numBits > 32)
        {
            if (B_POOL_SETTING_CONSISTENT_RNG)
                rnd = LocalRandom32(&localRngState);
            else
                rnd = Random32();
            usedBits = 0;
        }
        u32 currIndex = (rnd & ((1u << numBits) - 1)) % (poolSize - i);
        rnd = rnd >> numBits;
        usedBits += numBits;
        u32 tempValue = poolIndexArray[poolSize - 1 - i];
        poolIndexArray[poolSize - 1 - i] = poolIndexArray[currIndex];
        poolIndexArray[currIndex] = tempValue;
    }
}

static struct PickFunctions GetPickFunctions(const struct Trainer *trainer)
{
    struct PickFunctions pickFunctions;
    switch (trainer->poolPickIndex)
    {
        //  Repeats, but better to have the safety
    case POOL_PICK_DEFAULT:
        pickFunctions.LeadFunction = &DefaultLeadPickFunction;
        pickFunctions.AceFunction = &DefaultAcePickFunction;
        pickFunctions.OtherFunction = &DefaultOtherPickFunction;
        break;
    case POOL_PICK_LOWEST:
        pickFunctions.LeadFunction = &PickLowest;
        pickFunctions.AceFunction = &PickLowest;
        pickFunctions.OtherFunction = &PickLowest;
        break;
    default:
        pickFunctions.LeadFunction = &DefaultLeadPickFunction;
        pickFunctions.AceFunction = &DefaultAcePickFunction;
        pickFunctions.OtherFunction = &DefaultOtherPickFunction;
        break;
    }
    return pickFunctions;
}

static void TestPrune(const struct Trainer *trainer, u8 *poolIndexArray, const struct PoolRules *rules)
{
    //  Test function to demonstrate pruning
    for (u32 i = 0; i < trainer->poolSize; i++)
        if (trainer->party[poolIndexArray[i]].species == SPECIES_WOBBUFFET)
            poolIndexArray[i] = POOL_SLOT_DISABLED;
}

static void RandomTagPrune(const struct Trainer *trainer, u8 *poolIndexArray, const struct PoolRules *rules)
{
    u32 tagToUse = trainer->party[poolIndexArray[0]].tags;
    for (u32 i = 0; i < trainer->poolSize; i++)
        if (!(trainer->party[poolIndexArray[i]].tags & tagToUse))
            poolIndexArray[i] = POOL_SLOT_DISABLED;
}

static bool32 TryPrunePoolToTag(const struct Trainer *trainer, u8 *poolIndexArray, const struct PoolRules *rules, u32 tagMask, u8 monsCount, u32 battleTypeFlags)
{
    u32 members = 0;
    u8 *testPoolIndexArray;
    bool32 canPickParty = TRUE;

    for (u32 i = 0; i < trainer->poolSize; i++)
        if (poolIndexArray[i] != POOL_SLOT_DISABLED && (trainer->party[poolIndexArray[i]].tags & tagMask))
            members++;

    if (members < monsCount)
        return FALSE;

    testPoolIndexArray = Alloc(trainer->poolSize);
    if (testPoolIndexArray == NULL)
        return FALSE;

    for (u32 i = 0; i < trainer->poolSize; i++)
    {
        if (poolIndexArray[i] != POOL_SLOT_DISABLED && (trainer->party[poolIndexArray[i]].tags & tagMask))
            testPoolIndexArray[i] = poolIndexArray[i];
        else
            testPoolIndexArray[i] = POOL_SLOT_DISABLED;
    }

    struct PoolRules testRules = *rules;
    struct PickFunctions pickFunctions = GetPickFunctions(trainer);
    for (u32 i = 0; i < monsCount; i++)
    {
        if (PickMonFromPool(trainer, testPoolIndexArray, i, monsCount, battleTypeFlags, &testRules, pickFunctions) == POOL_SLOT_DISABLED)
        {
            canPickParty = FALSE;
            break;
        }
    }
    Free(testPoolIndexArray);

    if (!canPickParty)
        return FALSE;

    for (u32 i = 0; i < trainer->poolSize; i++)
        if (poolIndexArray[i] != POOL_SLOT_DISABLED && !(trainer->party[poolIndexArray[i]].tags & tagMask))
            poolIndexArray[i] = POOL_SLOT_DISABLED;

    return TRUE;
}

static bool32 PlayerPartyHasMove(u32 partyIndex, enum Move move)
{
    for (u32 i = 0; i < MAX_MON_MOVES; i++)
        if (GetMonData(&gParties[B_TRAINER_PLAYER][partyIndex], MON_DATA_MOVE1 + i) == move)
            return TRUE;

    return FALSE;
}

static bool32 SpeciesAppliesHighLegendaryPressure(enum Species species)
{
    switch (species)
    {
    case SPECIES_MIRAIDON:
    case SPECIES_KORAIDON:
    case SPECIES_KYOGRE:
    case SPECIES_GROUDON:
    case SPECIES_RAYQUAZA:
    case SPECIES_ZACIAN:
    case SPECIES_ZACIAN_CROWNED:
    case SPECIES_ZAMAZENTA:
    case SPECIES_ZAMAZENTA_CROWNED:
    case SPECIES_ETERNATUS:
    case SPECIES_CALYREX:
    case SPECIES_CALYREX_ICE:
    case SPECIES_CALYREX_SHADOW:
        return TRUE;
    default:
        return FALSE;
    }
}

static enum PoolTags GetOpponentAdaptivePruneTag(void)
{
    u32 fastPressure = 0;
    u32 weatherOrLegendPressure = 0;
    u32 setupOrSupportPressure = 0;
    u32 partyCount = min(gPartiesCount[B_TRAINER_PLAYER], PARTY_SIZE);

    for (u32 i = 0; i < partyCount; i++)
    {
        enum Species species = GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_SPECIES_OR_EGG);

        if (species == SPECIES_NONE || species == SPECIES_EGG)
            continue;

        if (GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_SPEED) >= 170
         || PlayerPartyHasMove(i, MOVE_TAILWIND)
         || PlayerPartyHasMove(i, MOVE_ICY_WIND)
         || PlayerPartyHasMove(i, MOVE_ELECTROWEB))
            fastPressure++;

        if (SpeciesAppliesHighLegendaryPressure(species)
         || PlayerPartyHasMove(i, MOVE_RAIN_DANCE)
         || PlayerPartyHasMove(i, MOVE_SUNNY_DAY)
         || PlayerPartyHasMove(i, MOVE_SANDSTORM)
         || PlayerPartyHasMove(i, MOVE_SNOWSCAPE))
            weatherOrLegendPressure++;

        if (PlayerPartyHasMove(i, MOVE_TRICK_ROOM)
         || PlayerPartyHasMove(i, MOVE_DRAGON_DANCE)
         || PlayerPartyHasMove(i, MOVE_SWORDS_DANCE)
         || PlayerPartyHasMove(i, MOVE_NASTY_PLOT)
         || PlayerPartyHasMove(i, MOVE_FOLLOW_ME)
         || PlayerPartyHasMove(i, MOVE_RAGE_POWDER)
         || PlayerPartyHasMove(i, MOVE_WIDE_GUARD))
            setupOrSupportPressure++;
    }

    if (weatherOrLegendPressure >= 2)
        return POOL_TAG_TAG7;
    if (fastPressure >= 2)
        return POOL_TAG_TAG6;
    if (setupOrSupportPressure >= 2)
        return POOL_TAG_TAG8;
    if (weatherOrLegendPressure >= 1)
        return POOL_TAG_TAG7;
    if (fastPressure >= 1)
        return POOL_TAG_TAG6;
    if (setupOrSupportPressure >= 1)
        return POOL_TAG_TAG8;

    return POOL_TAG_TAG6 + (Random() % 3);
}

static void OpponentAdaptivePrune(const struct Trainer *trainer, u8 *poolIndexArray, const struct PoolRules *rules, u8 monsCount, u32 battleTypeFlags)
{
    enum PoolTags preferredTag = GetOpponentAdaptivePruneTag();

    if (TryPrunePoolToTag(trainer, poolIndexArray, rules, 1u << preferredTag, monsCount, battleTypeFlags))
        return;

    for (u32 i = 0; i < trainer->poolSize; i++)
    {
        if (poolIndexArray[i] == POOL_SLOT_DISABLED)
            continue;

        for (enum PoolTags tag = POOL_TAG_TAG6; tag <= POOL_TAG_TAG8; tag++)
            if (trainer->party[poolIndexArray[i]].tags & (1u << tag)
             && TryPrunePoolToTag(trainer, poolIndexArray, rules, 1u << tag, monsCount, battleTypeFlags))
                return;
    }
}

static void PrunePool(const struct Trainer *trainer, u8 *poolIndexArray, const struct PoolRules *rules, u8 monsCount, u32 battleTypeFlags)
{
    //  Use defined pruning functions go here
    switch (trainer->poolPruneIndex)
    {
    case POOL_PRUNE_NONE:
        break;
    case POOL_PRUNE_TEST:
        TestPrune(trainer, poolIndexArray, rules);
        break;
    case POOL_PRUNE_RANDOM_TAG:
        RandomTagPrune(trainer, poolIndexArray, rules);
        break;
    case POOL_PRUNE_OPPONENT_ADAPTIVE:
        OpponentAdaptivePrune(trainer, poolIndexArray, rules, monsCount, battleTypeFlags);
        break;
    default:
        break;
    }
}

void DoTrainerPartyPool(const struct Trainer *trainer, u32 *monIndices, u8 monsCount, u32 battleTypeFlags)
{
        bool32 usingPool = FALSE;
        struct PoolRules rules = defaultPoolRules;
        struct Trainer tempTrainer;
        if (trainer->poolSize == 0 && (trainer->aiFlags & AI_FLAG_RANDOMIZE_PARTY_INDICES))
        {
            tempTrainer = *trainer;
            tempTrainer.poolSize = tempTrainer.partySize;
            trainer = &tempTrainer;
        }

        if (trainer->poolSize != 0)
        {
            usingPool = TRUE;
            rules = gPoolRulesetsList[trainer->poolRuleIndex];
            u8 *poolIndexArray = Alloc(trainer->poolSize);
            RandomizePoolIndices(trainer, poolIndexArray);

            struct PickFunctions pickFunctions = GetPickFunctions(trainer);

            PrunePool(trainer, poolIndexArray, &rules, monsCount, battleTypeFlags);

            for (u32 i = 0; i < monsCount; i++)
            {
                monIndices[i] = PickMonFromPool(trainer, poolIndexArray, i, monsCount, battleTypeFlags, &rules, pickFunctions);
                //  If the slot doesn't have a proper value, the pool creation failed, fall back to normal mon pick process
                if (monIndices[i] == POOL_SLOT_DISABLED)
                {
                    usingPool = FALSE;
                    break;
                }
            }
            Free(poolIndexArray);
        }

        if (!usingPool)
            for (u32 i = 0; i < monsCount; i++)
                monIndices[i] = i;
}
