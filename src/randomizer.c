#include "randomizer.h"

#if RANDOMIZER_AVAILABLE == TRUE
#include "main.h"
#include "new_game.h"
#include "item.h"
#include "event_data.h"
#include "field_control_avatar.h"
#include "gba/isagbprint.h"
#include "pokemon.h"
#include "script.h"
#include "data.h"
#include "data/randomizer/special_form_tables.h"
#include "data/randomizer/trainer_skip_list.h"
#include "data/randomizer/trainer_dup_rules.h"
#include "randomizer_area_rules.h"
#include "randomizer_exceptions.h"
#include "rtc.h"

#define SLOT_MODE_UNIFORM RANDOMIZER_SLOT_MODE_UNIFORM
#define SLOT_MODE_RARE    RANDOMIZER_SLOT_MODE_RARE

#define AREA_MASK_LAND   (1 << 0)
#define AREA_MASK_WATER  (1 << 1)
#define AREA_MASK_FISH   (1 << 2)
#define AREA_MASK_ROCKS  (1 << 3)
#define AREA_MASK_HIDDEN (1 << 4)
#define AREA_MASK_GIFT   (1 << 5)
#define SPECIAL_MASK_ALL (RANDOMIZER_SPECIAL_LEGEND | RANDOMIZER_SPECIAL_MYTHICAL | RANDOMIZER_SPECIAL_ULTRA_BEAST | RANDOMIZER_SPECIAL_PARADOX | RANDOMIZER_SPECIAL_SUB_LEGEND)

static bool32 IsSpeciesPermitted(u16 species);
static u16 GetRuleWhitelistLimit(const struct RandomizerAreaRule *rule, const struct RandomizerFishingRule *fishingRule);
static const struct RandomizerFishingRule *SelectFishingRule(const struct RandomizerAreaRule *rule, u8 rodType);

#define TRAINER_DUP_MAX_SAME_DEFAULT 255
#define TRAINER_DUP_MIN_DISTINCT_DEFAULT 0
#define TRAINER_DUP_MAX_ATTEMPTS 8

static u16 sTrainerPartyCache[PARTY_SIZE];
static u16 sTrainerPartyCacheTrainerId;
static u8 sTrainerPartyCacheFilled;
static u8 sTrainerPartyCacheDistinct;
static u8 sTrainerPartyCacheTotal;
static bool8 sAutoMaxRerollsLogValid = FALSE;
static u8 sAutoMaxRerollsLogMapGroup;
static u8 sAutoMaxRerollsLogMapNum;
static u8 sAutoMaxRerollsLogArea;
static u8 sAutoMaxRerollsLogTime;
static u8 sAutoMaxRerollsLogRod;

static u8 ClampAutoMaxRerolls(u16 count)
{
    if (count == 0)
        return 0;
    if (count > 8)
        return 8;
    return (u8)count;
}

static bool8 ShouldLogAutoMaxRerolls(u8 mapGroup, u8 mapNum, u8 area, u8 timeSlot, u8 rodType)
{
    if (!sAutoMaxRerollsLogValid
        || sAutoMaxRerollsLogMapGroup != mapGroup
        || sAutoMaxRerollsLogMapNum != mapNum
        || sAutoMaxRerollsLogArea != area
        || sAutoMaxRerollsLogTime != timeSlot
        || sAutoMaxRerollsLogRod != rodType)
    {
        sAutoMaxRerollsLogValid = TRUE;
        sAutoMaxRerollsLogMapGroup = mapGroup;
        sAutoMaxRerollsLogMapNum = mapNum;
        sAutoMaxRerollsLogArea = area;
        sAutoMaxRerollsLogTime = timeSlot;
        sAutoMaxRerollsLogRod = rodType;
        return TRUE;
    }
    return FALSE;
}

bool32 RandomizerFeatureEnabled(enum RandomizerFeature feature)
{
    switch(feature)
    {
        case RANDOMIZE_WILD_MON:
            #ifdef FORCE_RANDOMIZE_WILD_MON
                return FORCE_RANDOMIZE_WILD_MON;
            #else
                return FlagGet(RANDOMIZER_FLAG_WILD_MON);
            #endif
        case RANDOMIZE_FIELD_ITEMS:
            #ifdef FORCE_RANDOMIZE_FIELD_ITEMS
                return FORCE_RANDOMIZE_FIELD_ITEMS;
            #else
                return FlagGet(RANDOMIZER_FLAG_FIELD_ITEMS);
            #endif
        case RANDOMIZE_TRAINER_MON:
            #ifdef FORCE_RANDOMIZE_TRAINER_MON
                return FORCE_RANDOMIZE_TRAINER_MON;
            #else
                return FlagGet(RANDOMIZER_FLAG_TRAINER_MON);
            #endif
        case RANDOMIZE_FIXED_MON:
            #ifdef FORCE_RANDOMIZE_FIXED_MON
                return FORCE_RANDOMIZE_FIXED_MON;
            #else
                return FlagGet(RANDOMIZER_FLAG_FIXED_MON);
            #endif
        case RANDOMIZE_STARTERS:
            #ifdef FORCE_RANDOMIZE_STARTERS
                return FORCE_RANDOMIZE_STARTERS;
            #else
                return FlagGet(RANDOMIZER_FLAG_STARTERS);
            #endif
        default:
            return FALSE;
    }
}

static bool8 RandomizerAreaRulesEnabled(void)
{
    return FlagGet(FLAG_RANDOMIZER_AREA_WL);
}

// Forward declarations for helper functions defined later
static u8 GetAreaMaskFromWildArea(enum WildPokemonArea area);
static const struct RandomizerAreaRule *FindAreaRule(u8 mapGroup, u8 mapNum, u8 areaMask, u8 timeSlot);
static bool8 IsExceptionMap(u8 mapGroup, u8 mapNum, u8 areaMask, u8 timeSlot, u8 rodType);
static void GetRulePools(const struct RandomizerAreaRule *rule, const struct RandomizerFishingRule *fishingRule, bool8 useRare, const u16 **wl, u16 *wlCount, const u16 **bl, u16 *blCount, u8 *specialMask, bool8 *allowEmpty);

static const u16 *GetRuleWeights(const struct RandomizerAreaRule *rule, const struct RandomizerFishingRule *fishingRule, bool8 useRare, u16 *count)
{
    const u16 *weights = NULL;
    *count = 0;

    if (useRare && rule->rareWeightCount > 0)
    {
        weights = &sRandomizerAreaRareWeightPool[rule->rareWeightOffset];
        *count = rule->rareWeightCount;
    }
    else if (!useRare && rule->weightCount > 0)
    {
        weights = &sRandomizerAreaWeightPool[rule->weightOffset];
        *count = rule->weightCount;
    }
    return weights;
}

static u16 ChooseWeightedIndex(const u16 *weights, u16 count, struct Sfc32State *state)
{
    u32 total = 0;
    u16 i;
    for (i = 0; i < count; i++)
        total += weights[i];
    if (total == 0)
        return 0;
    u32 r = RandomizerNextRange(state, total);
    u32 acc = 0;
    for (i = 0; i < count; i++)
    {
        acc += weights[i];
        if (r < acc)
            return i;
    }
    return count - 1;
}

#ifndef NDEBUG
static bool8 RandomizerDebugLoggingEnabled(void)
{
    return FlagGet(FLAG_RANDOMIZER_DEBUG_LOG);
}

static const struct RandomizerAreaRule *FindAreaRuleForContext(u8 mapGroup, u8 mapNum, enum WildPokemonArea area, u8 timeSlot, u8 rodType, const struct RandomizerFishingRule **outFishing)
{
    const struct RandomizerAreaRule *areaRule = FindAreaRule(mapGroup, mapNum, GetAreaMaskFromWildArea(area), timeSlot);
    const struct RandomizerFishingRule *fishingRule = NULL;
    if (outFishing)
        *outFishing = NULL;
    if (areaRule == NULL)
        return NULL;
    if (area == WILD_AREA_FISHING)
        fishingRule = SelectFishingRule(areaRule, rodType);
    if (outFishing)
        *outFishing = fishingRule;
    return areaRule;
}

// 新しい参照用API：weights/levelBands/encounterRate等を返す。見つからなければFALSE。
bool8 RandomizerGetAreaRuleView(u8 mapGroup, u8 mapNum, enum WildPokemonArea area, u8 rodType, u8 timeSlot, struct RandomizerRuleView *out)
{
#if RANDOMIZER_AVAILABLE != TRUE
    return FALSE;
#else
    const struct RandomizerAreaRule *areaRule;
    const struct RandomizerFishingRule *fishingRule;
    const u16 *wl = NULL, *bl = NULL;
    u16 wlCount = 0, blCount = 0;
    u8 specialMask = SPECIAL_MASK_ALL;
    bool8 allowEmpty = FALSE;
    const u16 *weights = NULL;
    u16 weightCount = 0;

    if (out == NULL)
        return FALSE;

    if (FlagGet(FLAG_RANDOMIZER_VANILLA_ENCOUNTER) || IsExceptionMap(mapGroup, mapNum, GetAreaMaskFromWildArea(area), timeSlot, rodType))
        return FALSE;
    if (!RandomizerFeatureEnabled(RANDOMIZE_WILD_MON))
        return FALSE;

    areaRule = FindAreaRuleForContext(mapGroup, mapNum, area, timeSlot, rodType, &fishingRule);
    if (areaRule == NULL)
        return FALSE;

    GetRulePools(areaRule, fishingRule, FALSE, &wl, &wlCount, &bl, &blCount, &specialMask, &allowEmpty);
    weights = GetRuleWeights(areaRule, fishingRule, FALSE, &weightCount);

    out->areaRule = areaRule;
    out->fishingRule = fishingRule;
    out->wl = wl;
    out->wlCount = wlCount;
    out->bl = bl;
    out->blCount = blCount;
    out->weights = weights;
    out->weightCount = weightCount;
    out->rareWeights = NULL;
    out->rareWeightCount = 0;
    if (areaRule->rareWeightCount > 0)
    {
        out->rareWeights = &sRandomizerAreaRareWeightPool[areaRule->rareWeightOffset];
        out->rareWeightCount = areaRule->rareWeightCount;
    }
    out->levelBands = NULL;
    out->levelBandCount = 0;
    if (areaRule->levelBandCount > 0 && RANDOMIZER_LEVEL_BAND_POOL_COUNT > 0)
    {
        out->levelBands = &sRandomizerAreaLevelBandPool[areaRule->levelBandOffset];
        out->levelBandCount = areaRule->levelBandCount;
    }
    out->rareLevelBands = NULL;
    out->rareLevelBandCount = 0;
    if (areaRule->rareLevelBandCount > 0 && RANDOMIZER_RARE_LEVEL_BAND_POOL_COUNT > 0)
    {
        out->rareLevelBands = &sRandomizerAreaRareLevelBandPool[areaRule->rareLevelBandOffset];
        out->rareLevelBandCount = areaRule->rareLevelBandCount;
    }
    out->slotCount = fishingRule ? fishingRule->maxSpecies : areaRule->slotCount;
    out->slotMode = fishingRule ? fishingRule->slotMode : areaRule->slotMode;
    out->rareSlots = fishingRule ? fishingRule->rareSlots : areaRule->rareSlots;
    out->rareRate = fishingRule ? fishingRule->rareRate : areaRule->rareRate;
    out->encounterRate = areaRule->encounterRate;
    if (out->slotCount == 0 && out->weightCount > 0)
        out->slotCount = out->weightCount;
    if (out->slotCount == 0 && out->levelBandCount > 0)
        out->slotCount = out->levelBandCount;
    out->allowEmpty = allowEmpty;
    out->specialOverrides = specialMask;
    return TRUE;
#endif
}

static void DebugLogWildRare(u8 mapGroup, u8 mapNum, enum WildPokemonArea area, u8 slot, const struct RandomizerAreaRule *rule, const struct RandomizerFishingRule *fishingRule, bool8 rareHit, u16 targetCount, u16 wlLimit)
{
    if (!RandomizerDebugLoggingEnabled())
        return;

    DebugPrintfLevel(MGBA_LOG_WARN, // WARNに統一（他のINFOログに埋もれないようにする）
                     "[INFO] RandR wild m=%d/%d area=%d slot=%d mode=%d rareSlots=%d rareRate=%d wl=%d targetCount=%d rareHit=%d fishing=%d",
                     mapGroup,
                     mapNum,
                     area,
                     slot,
                     fishingRule ? fishingRule->slotMode : rule->slotMode,
                     fishingRule ? fishingRule->rareSlots : rule->rareSlots,
                     fishingRule ? fishingRule->rareRate : rule->rareRate,
                     wlLimit,
                     targetCount,
                     rareHit,
                     fishingRule ? 1 : 0);
}
#endif

static u8 GetAreaMaskFromWildArea(enum WildPokemonArea area)
{
    switch (area)
    {
        case WILD_AREA_WATER:
            return AREA_MASK_WATER;
        case WILD_AREA_FISHING:
            return AREA_MASK_FISH;
        case WILD_AREA_ROCKS:
            return AREA_MASK_ROCKS;
        case WILD_AREA_HIDDEN:
            return AREA_MASK_HIDDEN;
        case WILD_AREA_LAND:
        default:
            return AREA_MASK_LAND;
    }
}

static void GetRulePools(const struct RandomizerAreaRule *rule, const struct RandomizerFishingRule *fishingRule, bool8 useRare, const u16 **wl, u16 *wlCount, const u16 **bl, u16 *blCount, u8 *specialMask, bool8 *allowEmpty)
{
    const u16 *wlPool = sRandomizerAreaWhitelistPool;
    const u16 *blPool = sRandomizerAreaBlacklistPool;
    u16 wlOffset, blOffset;
    if (wl)
        *wl = NULL;
    if (bl)
        *bl = NULL;
    if (wlCount)
        *wlCount = 0;
    if (blCount)
        *blCount = 0;
    if (specialMask)
        *specialMask = SPECIAL_MASK_ALL;
    if (allowEmpty)
        *allowEmpty = 0;

    if (fishingRule != NULL)
    {
        wlOffset = useRare ? fishingRule->rareWlOffset : fishingRule->normalWlOffset;
        blOffset = useRare ? fishingRule->rareBlOffset : fishingRule->normalBlOffset;
        if (wl)
            *wl = wlPool + wlOffset;
        if (bl)
            *bl = blPool + blOffset;
        if (wlCount)
            *wlCount = useRare ? fishingRule->rareWlCount : fishingRule->normalWlCount;
        if (blCount)
            *blCount = useRare ? fishingRule->rareBlCount : fishingRule->normalBlCount;
        if (specialMask)
            *specialMask = fishingRule->specialOverrides;
        if (allowEmpty)
            *allowEmpty = fishingRule->allowEmpty;
    }
    else if (rule != NULL)
    {
        wlOffset = useRare ? rule->rareWlOffset : rule->normalWlOffset;
        blOffset = useRare ? rule->rareBlOffset : rule->normalBlOffset;
        if (wl)
            *wl = wlPool + wlOffset;
        if (bl)
            *bl = blPool + blOffset;
        if (wlCount)
            *wlCount = useRare ? rule->rareWlCount : rule->normalWlCount;
        if (blCount)
            *blCount = useRare ? rule->rareBlCount : rule->normalBlCount;
        if (specialMask)
            *specialMask = rule->specialOverrides;
        if (allowEmpty)
            *allowEmpty = rule->allowEmpty;
    }
}

static bool8 SpeciesAllowedByRule(u16 species, const struct RandomizerAreaRule *rule, const struct RandomizerFishingRule *fishingRule, bool8 useRare, u16 limit)
{
    u16 i;
    const u16 *wl, *bl;
    u16 wlCount, blCount;
    u8 specialMask;
    bool8 allowEmpty;
    u8 catMask = 0;
    u8 mode = RANDOMIZER_SPECIAL_OVERRIDES_MODE_DEFAULT;

    if (!IsSpeciesPermitted(species))
        return FALSE;

    GetRulePools(rule, fishingRule, useRare, &wl, &wlCount, &bl, &blCount, &specialMask, &allowEmpty);

    if (wlCount == 0)
        return FALSE;
    if (limit > 0 && limit < wlCount)
        wlCount = limit;

    {
        u16 varMode = VarGet(VAR_RANDOMIZER_SPECIAL_MODE);
        if (varMode == RANDOMIZER_SPECIAL_MODE_OR || varMode == RANDOMIZER_SPECIAL_MODE_AND)
            mode = (u8)varMode;
    }

    if (gSpeciesInfo[species].isLegendary)
        catMask |= RANDOMIZER_SPECIAL_LEGEND;
    if (gSpeciesInfo[species].isMythical)
        catMask |= RANDOMIZER_SPECIAL_MYTHICAL;
    if (gSpeciesInfo[species].isUltraBeast)
        catMask |= RANDOMIZER_SPECIAL_ULTRA_BEAST;
    if (gSpeciesInfo[species].isParadox)
        catMask |= RANDOMIZER_SPECIAL_PARADOX;
    if (gSpeciesInfo[species].isSubLegendary)
        catMask |= RANDOMIZER_SPECIAL_SUB_LEGEND;

    if (catMask != 0)
    {
        if (mode == RANDOMIZER_SPECIAL_MODE_AND)
        {
            if ((catMask & specialMask) != catMask)
                return FALSE;
        }
        else
        {
            if ((catMask & specialMask) == 0)
                return FALSE;
        }
    }

    for (i = 0; i < blCount; i++)
    {
        if (bl[i] == species)
            return FALSE;
    }

    for (i = 0; i < wlCount; i++)
    {
        if (wl[i] == species)
            return TRUE;
    }

    return FALSE;
}

static const struct RandomizerFishingRule *SelectFishingRule(const struct RandomizerAreaRule *rule, u8 rodType)
{
    if (rule == NULL)
        return NULL;
    if (rule->fishingRuleCount == 0 || rule->fishingRules == NULL)
        return NULL;

    switch (rodType)
    {
    case 0: // Old Rod
        return &rule->fishingRules[RANDOMIZER_ROD_OLD];
    case 1: // Good Rod
        if (rule->fishingRuleCount > RANDOMIZER_ROD_GOOD)
            return &rule->fishingRules[RANDOMIZER_ROD_GOOD];
        break;
    case 2: // Super Rod
        if (rule->fishingRuleCount > RANDOMIZER_ROD_SUPER)
            return &rule->fishingRules[RANDOMIZER_ROD_SUPER];
        break;
    default:
        break;
    }
    // fallback
    return &rule->fishingRules[0];
}

static bool8 TimeSlotMatches(u8 ruleSlot, u8 requestSlot)
{
    if (ruleSlot == 0xFF)
        return TRUE;          // ルール側ANYは常に許可
    if (requestSlot == 0xFF)
        return FALSE;         // 要求がANYなら、ANYルールのみを許可（時間帯付きルールにはマッチさせない）
    return ruleSlot == requestSlot;
}

static const struct RandomizerAreaRule *FindAreaRule(u8 mapGroup, u8 mapNum, u8 areaMask, u8 timeSlot)
{
    u16 i;
    const u8 targetMask = areaMask;

    if (!RandomizerAreaRulesEnabled())
        return NULL;

    for (i = 0; i < RANDOMIZER_AREA_RULE_COUNT; i++)
    {
        const struct RandomizerAreaRule *rule = &sRandomizerAreaRules[i];
        if (rule->mapGroup == mapGroup && rule->mapNum == mapNum)
        {
            if (rule->areaMask == targetMask && TimeSlotMatches(rule->timeSlot, timeSlot))
                return rule;
        }
    }

    return NULL;
}

#ifndef NDEBUG
static void DebugLogBlockedEncounter(u8 mapGroup, u8 mapNum, enum WildPokemonArea area, const struct RandomizerFishingRule *fishingRule)
{
    if (!RandomizerDebugLoggingEnabled())
        return;

    DebugPrintfLevel(MGBA_LOG_WARN,
                     "[INFO] RandR blocked map=%d/%d mask=%d fishing=%d",
                     mapGroup,
                     mapNum,
                     GetAreaMaskFromWildArea(area),
                     fishingRule != NULL);
}
#endif

static bool8 IsExceptionMap(u8 mapGroup, u8 mapNum, u8 areaMask, u8 timeSlot, u8 rodType)
{
#if RANDOMIZER_DISABLE_EXCEPTION_MAPS == TRUE
    return FALSE;
#else
    u16 i;
    for (i = 0; i < RANDOMIZER_EXCEPTION_COUNT; i++)
    {
        const struct RandomizerException *ex = &sRandomizerExceptions[i];
        if (ex->mapGroup == mapGroup && ex->mapNum == mapNum)
        {
            if (ex->areaMask != areaMask)
                continue;
            if (!TimeSlotMatches(ex->timeSlot, timeSlot))
                continue;
            if (ex->rodType != 0xFF && ex->rodType != rodType)
                continue;
            return TRUE;
        }
    }
    return FALSE;
#endif
}

static bool8 IsRuleBlocked(const struct RandomizerAreaRule *areaRule, const struct RandomizerFishingRule *fishingRule, const u16 **wlOut, u16 *wlCountOut, u8 mapGroup, u8 mapNum, enum WildPokemonArea area, u8 timeSlot, u8 rodType, bool8 logBlocked)
{
    const u16 *wl = NULL;
    u16 wlCount = 0;
    bool8 allowEmpty = FALSE;

    if (areaRule == NULL)
        return FALSE;

    GetRulePools(areaRule, fishingRule, FALSE, &wl, &wlCount, NULL, NULL, NULL, &allowEmpty);

    if (wlOut != NULL)
        *wlOut = wl;
    if (wlCountOut != NULL)
        *wlCountOut = wlCount;

    if (wlCount == 0 && allowEmpty)
    {
#ifndef NDEBUG
        if (logBlocked)
            DebugLogBlockedEncounter(mapGroup, mapNum, area, fishingRule);
#endif
        if (FlagGet(FLAG_RANDOMIZER_DEBUG_LOG))
        {
            DebugPrintfLevel(MGBA_LOG_WARN,
                             "[INFO] RandR blocked (allowEmpty) map=%d/%d mask=%d time=%d rod=%d",
                             mapGroup,
                             mapNum,
                             GetAreaMaskFromWildArea(area),
                             timeSlot,
                             rodType);
        }
        return TRUE;
    }

    return FALSE;
}

u8 RandomizerResolveTimeSlot(u8 defaultSlot)
{
    if (OW_TIME_OF_DAY_ENCOUNTERS)
        return GenConfigTimeOfDay(GetTimeOfDay());
    return defaultSlot;
}

bool8 RandomizerIsEncounterBlocked(u8 mapGroup, u8 mapNum, enum WildPokemonArea area, u8 rodType, u8 timeSlot)
{
    const struct RandomizerAreaRule *areaRule;
    const struct RandomizerFishingRule *fishingRule = NULL;

    if (!RandomizerFeatureEnabled(RANDOMIZE_WILD_MON))
        return FALSE;

    if (FlagGet(FLAG_RANDOMIZER_VANILLA_ENCOUNTER))
        return FALSE;

    if (IsExceptionMap(mapGroup, mapNum, GetAreaMaskFromWildArea(area), timeSlot, rodType))
        return FALSE; // デバッグ例外: 完全バニラ、カテゴリフィルタも無視（ログ抑制）

    areaRule = FindAreaRule(mapGroup, mapNum, GetAreaMaskFromWildArea(area), timeSlot);
    if (areaRule == NULL)
    {
        DebugPrintfLevel(MGBA_LOG_WARN,
                         "[INFO] RandR blocked undef map=%d/%d mask=%d time=%d rod=%d",
                         mapGroup,
                         mapNum,
                         GetAreaMaskFromWildArea(area),
                         timeSlot,
                         rodType);
        return TRUE; // 未定義エリアは遭遇させない
    }

    if (area == WILD_AREA_FISHING)
        fishingRule = SelectFishingRule(areaRule, rodType);

    return IsRuleBlocked(areaRule, fishingRule, NULL, NULL, mapGroup, mapNum, area, timeSlot, rodType, TRUE);
}

#ifndef NDEBUG
static void DebugLogRandomization(enum RandomizerReason reason, u32 seed, u16 original, u16 result, const struct RandomizerAreaRule *rule, const struct RandomizerFishingRule *fishingRule, bool8 useRare, u8 attempts, bool8 usedFallback)
{
    if (!RandomizerDebugLoggingEnabled())
        return;

    {
        const u16 *wl, *bl;
        u16 wlCount, blCount;
        u8 dummyMask;
        bool8 allowEmpty;
        GetRulePools(rule, fishingRule, useRare, &wl, &wlCount, &bl, &blCount, &dummyMask, &allowEmpty);

    DebugPrintfLevel(MGBA_LOG_WARN, // WARNに統一し、他のINFOログに埋もれないようにする
                     "[INFO] RandR reason=%d map=%d/%d mask=%d wl=%d bl=%d attempts=%d%s seed=%lu %d->%d",
                     reason,
                     rule != NULL ? rule->mapGroup : -1,
                     rule != NULL ? rule->mapNum : -1,
                     rule != NULL ? rule->areaMask : -1,
                     wlCount,
                     blCount,
                     attempts,
                     usedFallback ? " (fallback)" : "",
                     (unsigned long)seed,
                     original,
                     result);
    }
}
#endif

static void ResetTrainerPartyCache(u16 trainerId, u8 totalMons)
{
    sTrainerPartyCacheTrainerId = trainerId;
    sTrainerPartyCacheTotal = totalMons;
    sTrainerPartyCacheFilled = 0;
    sTrainerPartyCacheDistinct = 0;
}

static void GetTrainerDupRule(u16 trainerId, u8 *maxSame, u8 *minDistinct)
{
    u16 i;
    *maxSame = TRAINER_DUP_MAX_SAME_DEFAULT;
    *minDistinct = TRAINER_DUP_MIN_DISTINCT_DEFAULT;

    for (i = 0; gRandomizerTrainerDupRules[i].trainerId != RANDOMIZER_TRAINER_ID_END; i++)
    {
        if (gRandomizerTrainerDupRules[i].trainerId == trainerId)
        {
            *maxSame = gRandomizerTrainerDupRules[i].maxSame;
            *minDistinct = gRandomizerTrainerDupRules[i].minDistinct;
            break;
        }
    }
}

static bool8 TrainerPartyHasSpecies(u16 species)
{
    u8 i;
    for (i = 0; i < sTrainerPartyCacheFilled; i++)
    {
        if (sTrainerPartyCache[i] == species)
            return TRUE;
    }
    return FALSE;
}

static u8 TrainerPartySpeciesCount(u16 species)
{
    u8 i, count = 0;
    for (i = 0; i < sTrainerPartyCacheFilled; i++)
    {
        if (sTrainerPartyCache[i] == species)
            count++;
    }
    return count;
}

static void TrainerPartyCacheAdd(u16 species)
{
    bool8 alreadyPresent = TrainerPartyHasSpecies(species);
    if (sTrainerPartyCacheFilled < PARTY_SIZE)
    {
        sTrainerPartyCache[sTrainerPartyCacheFilled++] = species;
        if (!alreadyPresent)
            sTrainerPartyCacheDistinct++;
    }
}

static u16 RandomizeWithAreaRule(enum RandomizerReason reason, enum RandomizerSpeciesMode mode, u32 seed, u16 species, const struct RandomizerAreaRule *rule, const struct RandomizerFishingRule *fishingRule, bool8 useRare, u8 maxRerolls)
{
    u8 attempt;
    u16 candidate = species;
    u16 wlLimit = GetRuleWhitelistLimit(rule, fishingRule);
    const u16 *wl = NULL, *bl = NULL;
    u16 wlCount = 0, blCount = 0;
    const u16 *weights = NULL;
    u16 weightCount = 0;
    u8 specialMask = SPECIAL_MASK_ALL;
    bool8 allowEmpty = FALSE;

    GetRulePools(rule, fishingRule, useRare, &wl, &wlCount, &bl, &blCount, &specialMask, &allowEmpty);
    weights = GetRuleWeights(rule, fishingRule, useRare, &weightCount);

    if (wlCount == 0)
    {
        if (allowEmpty)
            return species;
        return species;
    }

    for (attempt = 0; attempt <= maxRerolls; attempt++)
    {
        u32 adjustedSeed = seed ^ attempt;
        candidate = RandomizeMon(reason, mode, adjustedSeed, species);
        if (SpeciesAllowedByRule(candidate, rule, fishingRule, useRare, wlLimit))
        {
#ifndef NDEBUG
            DebugLogRandomization(reason, seed, species, candidate, rule, fishingRule, useRare, attempt, FALSE);
#endif
            return candidate;
        }
    }

    {
        struct Sfc32State state = RandomizerRandSeed(reason, seed, species);
        u16 idx;
        if (weights != NULL && weightCount == wlCount && weightCount > 0)
            idx = ChooseWeightedIndex(weights, weightCount, &state);
        else
            idx = RandomizerNextRange(&state, wlLimit);
        candidate = wl[idx];
#ifndef NDEBUG
        DebugLogRandomization(reason, seed, species, candidate, rule, fishingRule, useRare, maxRerolls + 1, TRUE);
#endif
        return candidate;
    }
}

bool32 ShouldRandomizeTrainer(u16 trainerId)
{
#if RANDOMIZER_AVAILABLE != TRUE
    return FALSE;
#else
    u32 i;
    if (trainerId == RANDOMIZER_TRAINER_ID_UNKNOWN)
        return TRUE;

    for (i = 0; gRandomizerTrainerSkipList[i] != RANDOMIZER_TRAINER_ID_END; i++)
    {
        if (gRandomizerTrainerSkipList[i] == trainerId)
            return FALSE;
    }
    return TRUE;
#endif
}

u32 GetRandomizerSeed(void)
{
    #if RANDOMIZER_SEED_IS_TRAINER_ID == TRUE
        return GetTrainerId(gSaveBlock2Ptr->playerTrainerId);
    #else
        u32 result;
        result = ((u32)VarGet(RANDOMIZER_VAR_SEED_H) << 16) | VarGet(RANDOMIZER_VAR_SEED_L);
        return result;
    #endif
}

// Sets the seed that will be used for the randomizer if doing so is possible.
bool32 SetRandomizerSeed(u32 newSeed)
{
    #if RANDOMIZER_SEED_IS_TRAINER_ID == TRUE
        // It isn't possible to set the randomizer seed in this case.
        return FALSE;
    #else
        VarSet(RANDOMIZER_VAR_SEED_L, (u16)newSeed);
        VarSet(RANDOMIZER_VAR_SEED_H, (u16)(newSeed >> 16));
        return TRUE;
    #endif
}

static bool32 IsSpeciesPermitted(u16 species)
{
    // SPECIES_NONE is never valid.
    if (species == SPECIES_NONE)
        return FALSE;
    // This is used to indicate a disabled species.
    if (gSpeciesInfo[species].baseHP == 0)
        return FALSE;
    if (gSpeciesInfo[species].randomizerMode == MON_RANDOMIZER_INVALID)
        return FALSE;

    return TRUE;
};

static u16 GetRuleWhitelistLimit(const struct RandomizerAreaRule *rule, const struct RandomizerFishingRule *fishingRule)
{
    const u16 *wl;
    u16 wlCount = 0;
    bool8 allowEmpty;
    u16 limit = 0;

    GetRulePools(rule, fishingRule, FALSE, &wl, &wlCount, NULL, NULL, NULL, &allowEmpty);
    if (wlCount == 0)
        return 0;

    if (fishingRule != NULL && fishingRule->maxSpecies > 0)
        limit = fishingRule->maxSpecies;
    else if (rule != NULL && rule->maxSpecies > 0)
        limit = rule->maxSpecies;
    else
        limit = wlCount;

    if (limit > wlCount)
        limit = wlCount;

    return limit;
}

u32 GenerateSeedForRandomizer(void)
{
    u32 data;
    const u32 vblankCounter = gMain.vblankCounter1;
    #if HQ_RANDOM == TRUE
        data = Random32();
    #else
        data = gRngValue;
        Random();
    #endif
    return data ^ vblankCounter;
}

u16 GetRandomizerOption(enum RandomizerOption option)
{
    switch(option) {
        case RANDOMIZER_OPTION_SPECIES_MODE:
            return VarGet(RANDOMIZER_VAR_SPECIES_MODE);
        default: // Unknown option.
            return 0;
    }
}

/* Seeds an SFC32 random number generator state for the randomizer.

SFC32 can be seeded with up to 96 bits of data.
32 are used for the randomizer reason, which is mixed with the seed.
data1 and data2 are 64 bits of data that a caller can use for
any purpose they wish. Certain groups of functions will assign a purpose to
these: for example, RandomizeMon uses data2 for the original species number.
data2 is also mixed with the seed.
*/
struct Sfc32State RandomizerRandSeed(enum RandomizerReason reason, u32 data1, u32 data2)
{
    struct Sfc32State state;
    u32 i;
    const u32 randomizerSeed = GetRandomizerSeed();

    state.a = randomizerSeed + (u32)reason;
    state.b = randomizerSeed ^ data2;
    state.c = data1;
    state.ctr = RANDOMIZER_STREAM;

    for (i = 0; i < 10; i++)
    {
        _SFC32_Next_Stream(&state, RANDOMIZER_STREAM);
    }

    return state;
}


// This uses a slightly accelerated bitmasking method.
u32 RandomizerNextRange(struct Sfc32State* state, u32 range)
{
    u32 next_power_of_two, mask, result;
    if (range < 2)
        return 0;
    else if (range == UINT32_MAX)
        return _SFC32_Next_Stream(state, RANDOMIZER_STREAM);

    next_power_of_two = range;
    --next_power_of_two;
    next_power_of_two |= next_power_of_two >> 1;
    next_power_of_two |= next_power_of_two >> 2;
    next_power_of_two |= next_power_of_two >> 4;
    next_power_of_two |= next_power_of_two >> 8;
    ++next_power_of_two;

    mask = next_power_of_two - 1;

    do
    {
        result = _SFC32_Next_Stream(state, RANDOMIZER_STREAM) & mask;
    } while (result >= range);

    return result;
}

// Functions for producing single seeded random numbers.
u16 RandomizerRand(enum RandomizerReason reason, u32 data1, u32 data2)
{
    struct Sfc32State state;
    state = RandomizerRandSeed(reason, data1, data2);
    return RandomizerNext(&state);
}

u16 RandomizerRandRange(enum RandomizerReason reason, u32 data1, u32 data2, u16 range)
{
    struct Sfc32State state;
    state = RandomizerRandSeed(reason, data1, data2);
    return RandomizerNextRange(&state, range);
}

// Utility functions for the field item randomizer.
static inline bool32 IsItemTMHM(u16 itemId)
{
    return GetItemPocket(itemId) == POCKET_TM_HM;
}

static inline bool32 IsItemHM(u16 itemId)
{
    return itemId >= ITEM_HM01 && IsItemTMHM(itemId);
}

static inline bool32 IsKeyItem(u16 itemId)
{
    return GetItemPocket(itemId) == POCKET_KEY_ITEMS;
}

// Don't randomize HMs or key items, that can make the game unwinnable.
// ITEM_NONE also should not be randomized as it is invalid.
static inline bool32 ShouldRandomizeItem(u16 itemId)
{
    return !(IsItemHM(itemId) || IsKeyItem(itemId) || itemId == ITEM_NONE);
}

#include "data/randomizer/item_whitelist.h"

// Given a found item and its location in the game, returns a replacement for that item.
u16 RandomizeFoundItem(u16 itemId, u8 mapNum, u8 mapGroup, u8 localId)
{
    struct Sfc32State state;
    u16 result;
    u32 mapSeed;

    if (!ShouldRandomizeItem(itemId))
        return itemId;

    // Seed the generator using the original item and the object event that led up
    // to this call.
    mapSeed = ((u32)mapGroup) << 16;
    mapSeed |= ((u32)mapNum) << 8;
    mapSeed |= localId;

    state = RandomizerRandSeed(RANDOMIZER_REASON_FIELD_ITEM, mapSeed, itemId);

    // Randomize TMs to TMs. Because HMs shouldn't be randomized, we can assume
    // this is a TM.
    if (IsItemTMHM(itemId))
        return RandomizerNextRange(&state, RANDOMIZER_MAX_TM - ITEM_TM01 + 1) + ITEM_TM01;

    // Randomize everything else to everything else.
    do {
        result = sRandomizerItemWhitelist[RandomizerNextRange(&state, ITEM_WHITELIST_SIZE)];
    } while(!ShouldRandomizeItem(result) || IsItemTMHM(result));

    return result;

}

// Takes a SpecialVar as an argument to simplify handling separate scripts.
static inline void RandomizeFoundItemScript(u16 *scriptVar)
{
    if (RandomizerFeatureEnabled(RANDOMIZE_FIELD_ITEMS))
    {
        // Pull the object event information from the current object event.
        u8 objEvent = gSelectedObjectEvent;
        *scriptVar = RandomizeFoundItem(
            *scriptVar,
            gObjectEvents[objEvent].mapGroup,
            gObjectEvents[objEvent].mapNum,
            gObjectEvents[objEvent].localId);
    }
}

// These functions are invoked by the scripts that handle found items and
// write the results of the randomization to the correct script variable.
void FindItemRandomize_NativeCall(struct ScriptContext *ctx)
{
    RandomizeFoundItemScript(&gSpecialVar_0x8000);
}

void FindHiddenItemRandomize_NativeCall(struct ScriptContext *ctx)
{
    RandomizeFoundItemScript(&gSpecialVar_0x8005);
}

// Both legendary and mythical Pokémon are included in this category.
static inline bool32 IsRandomizerLegendary(u16 species)
{
    return gSpeciesInfo[species].isLegendary
        || gSpeciesInfo[species].isMythical
        || gSpeciesInfo[species].isUltraBeast;
}

struct SpeciesTable
{
    // Stores the group records for each species.
    u16 groupData[RANDOMIZER_SPECIES_COUNT];
    u16 speciesToGroupIndex[RANDOMIZER_SPECIES_COUNT];
    // Maps a group data index to a species.
    u16 groupIndexToSpecies[RANDOMIZER_SPECIES_COUNT];
};

#define GROUP_INVALID   0xFFFF

static inline u16 GetSpeciesGroup(const struct SpeciesTable* table, u16 species)
{
    return table->groupData[table->speciesToGroupIndex[species]];

}

static void GetGroupRange(u16 group, enum RandomizerSpeciesMode mode, u16 *resultMin, u16 *resultMax)
{
    // This should never be called on a GROUP_INVALID mon, but if it happens,
    // GROUP_INVALID should be the only valid group.
    if (group == GROUP_INVALID)
    {
        *resultMax = *resultMin = group;
        return;
    }

    // BST mode: species can randomize to species with similar BST.
    if (mode == MON_RANDOM_BST)
    {
        // Choose a 10.24% range around the base BST.
        s32 base, minScaled, maxScaled;
        base = group * 1024;
        minScaled = (base - group * 100) / 1024;
        maxScaled = (base + group * 100) / 1024;
        *resultMin = (u16)max(minScaled, 0);
        *resultMax =(u16)min(maxScaled, GROUP_INVALID-1);
    }
    // Species in the same category can randomize to each other.
    else
    {
        *resultMax = *resultMin = group;
    }
}

//
static void GetIndicesFromGroupRange(const struct SpeciesTable *table, u16 minGroup, u16 maxGroup, u16 *start, u16 *end)
{
    u16 index, leftBound, rightBound, maxRightBound;
    maxRightBound = RANDOMIZER_SPECIES_COUNT-1;
    maxGroup = min(0xFFFEu, maxGroup);
    minGroup = min(0xFFFEu, minGroup);
    leftBound = 0;
    rightBound = RANDOMIZER_SPECIES_COUNT-1;
    // Do leftmost binary search to find the lower limit.
    while (leftBound < rightBound)
    {
        u16 leftFoundGroup;
        index = (leftBound + rightBound) / 2;
        leftFoundGroup = table->groupData[index];
        if (leftFoundGroup < minGroup)
            leftBound = index + 1;
        else
        {
            if (leftFoundGroup > maxGroup)
                maxRightBound = index;
            rightBound = index;
        }
    }
    *start = leftBound;

    rightBound = maxRightBound;

    // Do rightmost binary search to find the upper limit.
    while (leftBound < rightBound)
    {
        index = (leftBound + rightBound) / 2;
        if (table->groupData[index] > maxGroup)
            rightBound = index;
        else
            leftBound = index + 1;
    }
    *end = rightBound - 1;
}

#if RANDOMIZER_DYNAMIC_SPECIES == TRUE

struct RamSpeciesTable
{
    enum RandomizerSpeciesMode mode;
    bool16 tableInitialized;
    struct SpeciesTable speciesTable;
};

EWRAM_DATA static struct RamSpeciesTable sRamSpeciesTable = {0};

static void FillSpeciesGroupsRandom(struct SpeciesTable* entries)
{
    u16 i;
    for (i = 0; i < RANDOMIZER_SPECIES_COUNT; i++)
    {
        entries->groupIndexToSpecies[i] = i;
        if (IsSpeciesPermitted(i))
            entries->groupData[i] = 0;
        else
            entries->groupData[i] = GROUP_INVALID;
    }
}

static void FillSpeciesGroupsBST(struct SpeciesTable* entries)
{
    u16 i;
    for(i = 0; i < RANDOMIZER_SPECIES_COUNT; i++)
    {
        const struct SpeciesInfo *curSpeciesInfo;
        u16 group;

        entries->groupIndexToSpecies[i] = i;

        if (IsSpeciesPermitted(i))
        {
            curSpeciesInfo = &gSpeciesInfo[i];

            group = curSpeciesInfo->baseAttack;
            group += curSpeciesInfo->baseDefense;
            group += curSpeciesInfo->baseSpAttack;
            group += curSpeciesInfo->baseSpDefense;
            group += curSpeciesInfo->baseHP;
            group += curSpeciesInfo->baseSpeed;

        }
        else
            group = GROUP_INVALID;

        entries->groupData[i] = group;
    }
}

static void FillSpeciesGroupsLegendary(struct SpeciesTable* entries)
{
    u16 i;
    for(i = 0; i < RANDOMIZER_SPECIES_COUNT; i++)
    {
        entries->groupIndexToSpecies[i] = i;
        if (!IsSpeciesPermitted(i))
            entries->groupData[i] = GROUP_INVALID;
        else
            entries->groupData[i] = IsRandomizerLegendary(i);
    }
}

// This is a list of baby Pokémon that should not cause their evolution
// to count as an evolved pokemon.
// XXX: put this somewhere else?
static const u16 sPreevolutionBabyMons[] =
{
    SPECIES_PICHU,
    SPECIES_CLEFFA,
    SPECIES_IGGLYBUFF,
    SPECIES_TYROGUE,
    SPECIES_SMOOCHUM,
    SPECIES_ELEKID,
    SPECIES_MAGBY,
    SPECIES_AZURILL,
    SPECIES_WYNAUT,
    SPECIES_BUDEW,
    SPECIES_CHINGLING,
    SPECIES_BONSLY,
    SPECIES_MIME_JR,
    SPECIES_HAPPINY,
    SPECIES_MUNCHLAX,
    SPECIES_MANTYKE,
};

static void MarkEvolutions(struct SpeciesTable *entries, u16 species, u16 stage)
{
    const struct Evolution *evos;
    if (stage == RANDOMIZER_MAX_EVO_STAGES)
        return;

    evos = GetSpeciesEvolutions(species);
    if (evos != NULL)
    {
        u32 i;
        for (i = 0; evos[i].method != 0xFFFF; i++)
        {
            if(entries->groupData[species-1] <= stage)
                MarkEvolutions(entries, evos[i].targetSpecies, stage+1);
        }
    }
    entries->groupIndexToSpecies[species] = species;
    entries->groupData[species] = stage;
}

static void FillSpeciesGroupsEvolution(struct SpeciesTable* entries)
{
    u16 i;
    static const u8 EVO_GROUP_LEGENDARY = 0x81;
    static const u8 EVO_GROUP_NO_EVO = RANDOMIZER_MAX_EVO_STAGES+1;

    // Step 0: zero everything
    memset(entries, 0, sizeof(sRamSpeciesTable.speciesTable));

    // Step 1: pre-visit the special babies, and mark them as basic mons.
    for (i = 0; i < ARRAY_COUNT(sPreevolutionBabyMons); i++)
    {
        u16 babyMonIndex = sPreevolutionBabyMons[i];
        entries->groupIndexToSpecies[babyMonIndex] = babyMonIndex;
        if(IsSpeciesPermitted(babyMonIndex))
            entries->groupData[babyMonIndex] = 0;
        else
            entries->groupData[babyMonIndex] = GROUP_INVALID;
    }

    for(i = 0; i < RANDOMIZER_SPECIES_COUNT; i++)
    {
        if (entries->groupIndexToSpecies[i] == 0)
        {
            // This entry hasn't been visited yet, so we don't know if it evolves.
            const struct Evolution *evos = GetSpeciesEvolutions(i);
            entries->groupIndexToSpecies[i] = i;
            if (!IsSpeciesPermitted(i)) // This shouldn't show up in randomization.
                entries->groupData[i] = GROUP_INVALID;
            else if (IsRandomizerLegendary(i)) // Legendaries get their own group.
                entries->groupData[i] = EVO_GROUP_LEGENDARY;
            else if (evos == NULL || evos->method == 0xFFFF)
                entries->groupData[i] = EVO_GROUP_NO_EVO;
            else // There are evolutions! Let's check it out.
                MarkEvolutions(entries, i, 0);
        }
    }
}

static inline u16 LeftChildIndex(u16 index)
{
    return 2*index + 1;
}

static inline void SwapSpeciesAndGroup(struct SpeciesTable* table, u16 indexA, u16 indexB)
{
    u16 temp;
    SWAP(table->groupData[indexA], table->groupData[indexB], temp);
    SWAP(table->groupIndexToSpecies[indexA], table->groupIndexToSpecies[indexB], temp);
}

static void BuildRandomizerSpeciesTable(enum RandomizerSpeciesMode mode)
{
    u16 i, start, end;
    struct SpeciesTable* speciesTable;

    sRamSpeciesTable.tableInitialized = TRUE;
    sRamSpeciesTable.mode = mode;
    speciesTable = &sRamSpeciesTable.speciesTable;

    switch(mode)
    {
        case MON_RANDOM_LEGEND_AWARE:
            FillSpeciesGroupsLegendary(speciesTable);
            break;
        case MON_RANDOM_BST:
            FillSpeciesGroupsBST(speciesTable);
            break;
        case MON_EVOLUTION:
            FillSpeciesGroupsEvolution(speciesTable);
            break;
        case MON_RANDOM:
        default:
            FillSpeciesGroupsRandom(speciesTable);
    }

    // Heap sort the table.
    start = RANDOMIZER_SPECIES_COUNT/2;
    end = RANDOMIZER_SPECIES_COUNT-1;

    while (end > 1)
    {
        u16 root;
        if (start > 0)
            start = start - 1;
        else
        {
            end = end - 1;
            SwapSpeciesAndGroup(speciesTable, end, 0);
        }
        root = start;
        while(LeftChildIndex(root) < end)
        {
            u16 child;
            child = LeftChildIndex(root);

            if (child+1 < end
                && speciesTable->groupData[child] < speciesTable->groupData[child+1])
            {
                child = child + 1;
            }

            if (speciesTable->groupData[root] < speciesTable->groupData[child])
            {
                SwapSpeciesAndGroup(speciesTable, root, child);
                root = child;
            }
            else
                break;
        }
    }


    // Build the species index. This is needed for getting a group from a species.
    for (i = 0; i < RANDOMIZER_SPECIES_COUNT; i++)
    {
        u16 targetIndex = speciesTable->groupIndexToSpecies[i];
        speciesTable->speciesToGroupIndex[targetIndex] = i;
    }
}

static const struct SpeciesTable* GetSpeciesTable(enum RandomizerSpeciesMode mode)
{
    if (!sRamSpeciesTable.tableInitialized || mode != sRamSpeciesTable.mode )
        BuildRandomizerSpeciesTable(mode);

    return &sRamSpeciesTable.speciesTable;
}

void PreloadRandomizationTables(void)
{
    GetSpeciesTable(GetRandomizerOption(RANDOMIZER_OPTION_SPECIES_MODE));
}

#endif

static u16 RandomizeMonTableLookup(struct Sfc32State* state, enum RandomizerSpeciesMode mode, u16 species)
{
    u16 minGroup, maxGroup, originalGroup, resultIndex;
    u16 minIndex, maxIndex;
    const struct SpeciesTable *table;

    table = GetSpeciesTable(mode);
    originalGroup = GetSpeciesGroup(table, species);

    if (originalGroup == GROUP_INVALID)
        return species;

    GetGroupRange(originalGroup, mode, &minGroup, &maxGroup);
    GetIndicesFromGroupRange(table, minGroup, maxGroup, &minIndex, &maxIndex);
    resultIndex = RandomizerNextRange(state, maxIndex - minIndex + 1) + minIndex;
    return table->groupIndexToSpecies[resultIndex];
}

static u16 RandomizeMonFromSeed(struct Sfc32State *state, enum RandomizerSpeciesMode mode, u16 species)
{
    if (!IsSpeciesPermitted(species))
        return species;

    if (mode >= MAX_MON_MODE)
        mode = MON_RANDOM;

    return RandomizeMonTableLookup(state, mode, species);

}

// Fills an array with count Pokémon, with no repeats.
void GetUniqueMonList(enum RandomizerReason reason, enum RandomizerSpeciesMode mode, u32 seed1, u16 seed2, u8 count, const u16 *originalSpecies, u16 *resultSpecies)
{
    u32 i, curMon;
    u32 seenMonBitVector[(RANDOMIZER_SPECIES_COUNT-1)/32+1] = {};
    struct Sfc32State state = RandomizerRandSeed(reason, seed1, seed2);

    for (i = 0; i < count; i++)
    {
        u16 curOriginal = originalSpecies[i];
        bool32 foundNextMon = FALSE;
        if (!IsSpeciesPermitted(curOriginal))
        {
            // If there's non-permitted Pokémon in here, something is wrong.
            // Just pass them through without marking.
            curMon = curOriginal;
            continue;
        }

        // Find the next mon.
        while (!foundNextMon)
        {
            u16 wordIndex, adjustedCurMon;
            u32 bitVectorWord;
            u8 bitIndex;

            // Generate a Pokémon. If it has already been generated, keep generating new ones
            // until one that hasn't been seen is picked.

            curMon = RandomizeMonFromSeed(&state, mode, curOriginal);

            // Compute the bit address of this mon.
            adjustedCurMon = curMon - 1;
            wordIndex = adjustedCurMon / 32;
            bitIndex = adjustedCurMon & 31;
            bitVectorWord = seenMonBitVector[wordIndex];

            // If set, this mon has been seen already.
            if (bitVectorWord & (1 << bitIndex))
                continue;

            bitVectorWord |= 1 << bitIndex;
            seenMonBitVector[wordIndex] = bitVectorWord;
            foundNextMon = TRUE;
        }
        resultSpecies[i] = curMon;
    }
}

u16 RandomizeMonBaseForm(enum RandomizerReason reason, enum RandomizerSpeciesMode mode, u32 seed, u16 species)
{
    struct Sfc32State state;
    state = RandomizerRandSeed(reason, seed, species);
    return RandomizeMonFromSeed(&state, mode, species);
}

static u16 ChooseRandomForm(struct Sfc32State *state, const u16 baseSpecies)
{
    const u16 *formsTable = gSpeciesInfo[baseSpecies].formSpeciesIdTable;
    if (formsTable)
    {
        u32 formCount = 0;
        while (formsTable[formCount] != FORM_SPECIES_END)
        {
            formCount++;
        }
        return formsTable[RandomizerNextRange(state, formCount)];
    }

    return baseSpecies;
}

static u16 GetFormFromRareFormInfo(struct Sfc32State *state, const struct RandomizerRareFormInfo *info)
{
    if (RandomizerNextRange(state, info->inverseRareFormChance) > 0)
        return info->commonForm;
    else
        return info->rareForm;
}

#define RANDOM_FROM_ARRAY(arr)  (arr[RandomizerNextRange(state, ARRAY_COUNT(arr))])
#define RARE_FORM(infoStruct)   (GetFormFromRareFormInfo(state, &infoStruct))
static u16 ChooseFormSpecial(struct Sfc32State *state, const u16 baseSpecies)
{
    switch (baseSpecies) {
        // These species do almost ordinary ordinary random form selection processes.
        // However, their form tables include forms that shouldn't normally be
        // selected, so they need to have special hard-coded form tables.
        case SPECIES_FLOETTE:
            return RANDOM_FROM_ARRAY(sFloetteFormChoices);
        case SPECIES_TAUROS_PALDEA_COMBAT:
            return RANDOM_FROM_ARRAY(sPaldeanTaurosFormChoices);
        case SPECIES_MINIOR:
            return RANDOM_FROM_ARRAY(sMiniorFormChoices);
        // These are species, first appearing in Gen 8, that have one common
        // form and one rare form.
        // Note that as Maushold can only appear in raid battles in Gen 9, it
        // normally does not behave this way in the wild, but for a randomizer
        // this seems like a reasonable choice.
        case SPECIES_MAUSHOLD:
            return RARE_FORM(sMausholdRareFormInfo);
        case SPECIES_SINISTEA:
            return RARE_FORM(sSinisteaRareFormInfo);
        case SPECIES_SINISTCHA:
            return RARE_FORM(sSinistchaRareFormInfo);
        case SPECIES_POLTEAGEIST:
            return RARE_FORM(sPolteageistRareFormInfo);
        case SPECIES_DUDUNSPARCE:
            return RARE_FORM(sDudunsparceRareFormInfo);
        default:
            return baseSpecies;
    }

}
#undef RANDOM_FROM_ARRAY
#undef RARE_FORM

u16 RandomizeMon(enum RandomizerReason reason, enum RandomizerSpeciesMode mode, u32 seed, u16 species)
{
    u32 speciesMode;
    u16 resultSpecies;
    struct Sfc32State state;

    if (!IsSpeciesPermitted(species))
        return species;

    state = RandomizerRandSeed(reason, seed, species);

    resultSpecies = RandomizeMonFromSeed(&state, mode, species);
    speciesMode = gSpeciesInfo[resultSpecies].randomizerMode;

    switch (speciesMode)
    {
        case MON_RANDOMIZER_RANDOM_FORM:
            return ChooseRandomForm(&state, resultSpecies);
        case MON_RANDOMIZER_SPECIAL_FORM:
            return ChooseFormSpecial(&state, resultSpecies);
        case MON_RANDOMIZER_NORMAL:
        default:
            return resultSpecies;
    }
}

// This is used in the Pokédex area map code.
bool32 IsRandomizationPossible(u16 originalSpecies, u16 targetSpecies)
{
    const enum RandomizerSpeciesMode mode = GetRandomizerOption(RANDOMIZER_OPTION_SPECIES_MODE);
    if (!IsSpeciesPermitted(targetSpecies) || !IsSpeciesPermitted(originalSpecies))
    {
        // For a species that is not permitted, randomization is disabled.
        // Therefore, if the species are the same, they will "randomize".
        return originalSpecies == targetSpecies;
    }

    if (mode != MON_RANDOM && mode < MAX_MON_MODE)
    {
        u16 minGroupOriginal, maxGroupOriginal, minGroupTarget, maxGroupTarget,
            originalGroup, targetGroup;
        const struct SpeciesTable* table;
        table = GetSpeciesTable(mode);
        originalGroup = GetSpeciesGroup(table, originalSpecies);
        targetGroup = GetSpeciesGroup(table, targetSpecies);
        GetGroupRange(originalGroup, mode, &minGroupOriginal, &maxGroupOriginal);
        GetGroupRange(targetGroup, mode, &minGroupTarget, &maxGroupTarget);

        // If the group ranges intersect, randomization is possible.
        return maxGroupOriginal >= minGroupTarget && minGroupOriginal <= maxGroupTarget;
    }

    return TRUE;
}

u16 RandomizeTrainerMon(u16 trainerId, u8 slot, u8 totalMons, u16 species)
{
    if (RandomizerFeatureEnabled(RANDOMIZE_TRAINER_MON))
    {
        // The seed is based on the internal trainer number, the number of
        // Pokémon in that trainer's party, and which party position it is in.
        u32 seed;
        seed = (u32)trainerId << 16;
        seed |= (u32)totalMons << 8;
        seed |= slot;

        {
            u8 maxSame, minDistinct;
            GetTrainerDupRule(trainerId, &maxSame, &minDistinct);

            if (trainerId != sTrainerPartyCacheTrainerId || totalMons != sTrainerPartyCacheTotal || slot == 0)
                ResetTrainerPartyCache(trainerId, totalMons);

            if (maxSame == TRAINER_DUP_MAX_SAME_DEFAULT && minDistinct == TRAINER_DUP_MIN_DISTINCT_DEFAULT)
            {
                species = RandomizeMon(RANDOMIZER_REASON_TRAINER_PARTY, GetRandomizerOption(RANDOMIZER_OPTION_SPECIES_MODE), seed, species);
            }
            else
            {
                u8 attempt;
                u8 remainingSlots = totalMons - sTrainerPartyCacheFilled;
                u8 neededDistinct = (minDistinct > sTrainerPartyCacheDistinct) ? (minDistinct - sTrainerPartyCacheDistinct) : 0;
                bool8 mustBeNew = (neededDistinct > 0 && remainingSlots <= neededDistinct);
                u16 fallbackSpecies = species;

                for (attempt = 0; attempt < TRAINER_DUP_MAX_ATTEMPTS; attempt++)
                {
                    u32 adjustedSeed = seed ^ (0x9E3779B9u * attempt);
                    u16 candidate = RandomizeMon(RANDOMIZER_REASON_TRAINER_PARTY, GetRandomizerOption(RANDOMIZER_OPTION_SPECIES_MODE), adjustedSeed, species);

                    fallbackSpecies = candidate;

                    if (mustBeNew && TrainerPartyHasSpecies(candidate))
                        continue;
                    if (TrainerPartySpeciesCount(candidate) >= maxSame)
                        continue;

                    species = candidate;
                    break;
                }

                if (attempt == TRAINER_DUP_MAX_ATTEMPTS)
                {
                    species = fallbackSpecies;
                }
            }
        }

        TrainerPartyCacheAdd(species);
        return species;
    }

    return species;
}

u16 RandomizeFixedEncounterMon(u16 species, u8 mapNum, u8 mapGroup, u8 localId)
{
    if (!RandomizerFeatureEnabled(RANDOMIZE_FIXED_MON))
        return species;

    // The seed is based on the location of the object event.
    u32 seed;
    seed = (u32)mapNum << 16;
    seed |= (u32)mapGroup << 8;
    seed |= localId;

    // Hiddenマスクでルール取得（固定/ギフト想定）
    {
        struct RandomizerRuleView view;
        if (RandomizerGetAreaRuleView(mapGroup, mapNum, WILD_AREA_HIDDEN, 0xFF, 0xFF, &view))
        {
            // allowEmptyかつWL空ならスキップ
            if (view.wlCount == 0 && view.allowEmpty)
                return species;

            // WL/weights/levelBandsを用いたランダム化
            const struct RandomizerAreaRule *areaRule = view.areaRule;
            const struct RandomizerFishingRule *fishRule = view.fishingRule; // 常にNULLのはず
            u8 maxRerolls = areaRule ? areaRule->maxRerolls : 0;
            return RandomizeWithAreaRule(RANDOMIZER_REASON_FIXED_ENCOUNTER,
                                         GetRandomizerOption(RANDOMIZER_OPTION_SPECIES_MODE),
                                         seed,
                                         species,
                                         areaRule,
                                         fishRule,
                                         FALSE,
                                         maxRerolls);
        }
    }

    return species;
}

EWRAM_DATA static u32 sLastStarterRandomizerSeed = 0;
EWRAM_DATA static u16 sRandomizedStarters[3] = {0};

u16 RandomizeStarter(u16 starterSlot, const u16* originalStarters)
{
    if (RandomizerFeatureEnabled(RANDOMIZE_STARTERS))
    {
        if (sLastStarterRandomizerSeed != GetRandomizerSeed() || sRandomizedStarters[0] == SPECIES_NONE)
        {
            // The randomized starter table is stale or uninitialized. Fix that!

            // Hash the starter list so that which starters there are influences the seed.
            u32 starterHash = 5381;
            u32 i;
            for (i = 0; i < 3; i++)
            {
                u16 originalStarter = originalStarters[i];
                starterHash = ((starterHash << 5) + starterHash) ^ (u8)originalStarter;
                starterHash = ((starterHash << 5) + starterHash) ^ (u8)(originalStarter >> 8);
            }

            GetUniqueMonList(RANDOMIZER_REASON_STARTER, GetRandomizerOption(RANDOMIZER_OPTION_SPECIES_MODE),
                starterHash, 0, 3, originalStarters, sRandomizedStarters);
        }
        return sRandomizedStarters[starterSlot];
    }

    return originalStarters[starterSlot];
}

// ラッパ: blocked out-paramを返す
bool8 RandomizeWildEncounterBlocked(u16 species, u8 mapNum, u8 mapGroup, enum WildPokemonArea area, u8 slot, u8 rodType, u8 timeSlot, u16 *outSpecies)
{
    // TODO: 将来的にレベル/遭遇率/スロット選択まで返す拡張APIを作る。現状は種族のみ。
    const struct RandomizerAreaRule *areaRule;
    bool8 blocked = FALSE;

    if (outSpecies == NULL)
        return FALSE;

    if (FlagGet(FLAG_RANDOMIZER_VANILLA_ENCOUNTER) || IsExceptionMap(mapGroup, mapNum, GetAreaMaskFromWildArea(area), timeSlot, rodType))
    {
        *outSpecies = species; // デバッグバニラ／例外マップ時は完全バニラ（カテゴリフィルタも無視）
        return FALSE;
    }

    if (RandomizerFeatureEnabled(RANDOMIZE_WILD_MON))
    {
        u32 seed;
        seed = ((u32)mapGroup) << 24;
        seed |= ((u32)mapNum) << 16;
        seed |= ((u32)area) << 8;
        seed |= slot;

        areaRule = FindAreaRule(mapGroup, mapNum, GetAreaMaskFromWildArea(area), timeSlot);
        if (areaRule != NULL)
        {
            const struct RandomizerFishingRule *fishingRule = NULL;
            const u16 *wlNormal, *wlRare;
            u16 wlNormalCount, wlRareCount;
            u8 slotMode;
            u8 rareSlots;
            u8 rareRate;
            u16 rareCount = 0;
            u16 commonCount = 0;
            u8 maxRerolls;

            if (area == WILD_AREA_FISHING)
                fishingRule = SelectFishingRule(areaRule, rodType);

            slotMode = fishingRule ? fishingRule->slotMode : areaRule->slotMode;
            rareSlots = fishingRule ? fishingRule->rareSlots : areaRule->rareSlots;
            rareRate = fishingRule ? fishingRule->rareRate : areaRule->rareRate;
            maxRerolls = (fishingRule && fishingRule->maxRerolls > 0) ? fishingRule->maxRerolls : areaRule->maxRerolls;

            if (IsRuleBlocked(areaRule, fishingRule, &wlNormal, &wlNormalCount, mapGroup, mapNum, area, timeSlot, rodType, TRUE))
            {
                blocked = TRUE;
                *outSpecies = species;
                return blocked;
            }
            GetRulePools(areaRule, fishingRule, TRUE, &wlRare, &wlRareCount, NULL, NULL, NULL, NULL);

            commonCount = wlNormalCount;
            rareCount = 0;
            if (slotMode == SLOT_MODE_RARE && rareRate > 0 && rareSlots > 0 && wlRareCount > 0)
            {
                if (rareSlots > wlRareCount)
                    rareSlots = wlRareCount;
                rareCount = rareSlots;
            }

            // maxRerolls=auto (0xFF) はランタイムで有効候補数に基づいて決定する
            u8 maxRerollsNormal = maxRerolls;
            u8 maxRerollsRare = maxRerolls;
            if (maxRerolls == RANDOMIZER_MAX_REROLLS_AUTO)
            {
                maxRerollsNormal = ClampAutoMaxRerolls(commonCount);
                maxRerollsRare = ClampAutoMaxRerolls(rareCount);
#ifndef NDEBUG
                if (FlagGet(FLAG_RANDOMIZER_DEBUG_LOG) && ShouldLogAutoMaxRerolls(mapGroup, mapNum, area, timeSlot, rodType))
                {
                    DebugPrintfLevel(MGBA_LOG_WARN,
                                     "[INFO] RandR auto maxRerolls map=%d/%d area=%d time=%d rod=%d normal=%d rare=%d -> n=%d r=%d",
                                     mapGroup,
                                     mapNum,
                                     area,
                                     timeSlot,
                                     rodType,
                                     commonCount,
                                     rareCount,
                                     maxRerollsNormal,
                                     maxRerollsRare);
                }
#endif
            }
            maxRerolls = maxRerollsNormal;

            if (slotMode == SLOT_MODE_UNIFORM || rareCount == 0 || rareRate == 0)
            {
                *outSpecies = RandomizeWithAreaRule(RANDOMIZER_REASON_WILD_ENCOUNTER, GetRandomizerOption(RANDOMIZER_OPTION_SPECIES_MODE), seed, species, areaRule, fishingRule, FALSE, maxRerolls);
                return blocked;
            }
            else
            {
                struct Sfc32State state = RandomizerRandSeed(RANDOMIZER_REASON_WILD_ENCOUNTER, seed, species);
                bool8 rareHit = (RandomizerNextRange(&state, 100) < rareRate);
                u8 attempt;
                u16 candidate = species;
                u16 targetCount = rareHit ? rareCount : commonCount;
                const u16 *wlTarget = rareHit ? wlRare : wlNormal;
                u8 maxRerollsHit = rareHit ? maxRerollsRare : maxRerollsNormal;

#ifndef NDEBUG
                DebugLogWildRare(mapGroup, mapNum, area, slot, areaRule, fishingRule, rareHit, targetCount, rareHit ? rareCount : commonCount);
#endif

                for (attempt = 0; attempt <= maxRerollsHit; attempt++)
                {
                    u32 adjustedSeed = seed ^ attempt;
                    candidate = RandomizeMon(RANDOMIZER_REASON_WILD_ENCOUNTER, GetRandomizerOption(RANDOMIZER_OPTION_SPECIES_MODE), adjustedSeed, species);
                    if (SpeciesAllowedByRule(candidate, areaRule, fishingRule, rareHit, targetCount))
                    {
#ifndef NDEBUG
                        DebugLogRandomization(RANDOMIZER_REASON_WILD_ENCOUNTER, seed, species, candidate, areaRule, fishingRule, rareHit, attempt, FALSE);
#endif
                        *outSpecies = candidate;
                        return blocked;
                    }
                }

                {
                    u16 idx = RandomizerNextRange(&state, targetCount);
                    candidate = wlTarget[idx];
#ifndef NDEBUG
                    DebugLogRandomization(RANDOMIZER_REASON_WILD_ENCOUNTER, seed, species, candidate, areaRule, fishingRule, rareHit, maxRerollsHit + 1, TRUE);
#endif
                    *outSpecies = candidate;
                    return blocked;
                }
            }
        }
        else
        {
#ifndef NDEBUG
            DebugPrintfLevel(MGBA_LOG_WARN,
                             "[INFO] RandR blocked undef map=%d/%d mask=%d time=%d rod=%d",
                             mapGroup,
                             mapNum,
                             GetAreaMaskFromWildArea(area),
                             timeSlot,
                             rodType);
#endif
            blocked = TRUE;
            *outSpecies = species;
            return blocked;
        }

        *outSpecies = RandomizeMon(RANDOMIZER_REASON_WILD_ENCOUNTER, GetRandomizerOption(RANDOMIZER_OPTION_SPECIES_MODE), seed, species);
        return blocked;
    }

    *outSpecies = species;
    return blocked;
}

// 互換用: blocked無視でspeciesのみ返す
u16 RandomizeWildEncounter(u16 species, u8 mapNum, u8 mapGroup, enum WildPokemonArea area, u8 slot, u8 rodType, u8 timeSlot)
{
    u16 out;
    RandomizeWildEncounterBlocked(species, mapNum, mapGroup, area, slot, rodType, timeSlot, &out);
    return out;
}

#endif // RANDOMIZER_AVAILABLE
