#include "global.h"
#include "box_npc_party_pool.h"
#include "battle.h"
#include "gba/isagbprint.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "random.h"

static EWRAM_DATA struct BoxNpcPartyPoolResult sLastResult = {0};
static EWRAM_DATA enum BoxNpcPartyPoolError sLastError = BOX_NPC_PARTY_POOL_ERROR_NONE;
static EWRAM_DATA bool8 sPendingBattleInitPolicy = FALSE;
static EWRAM_DATA u8 sPendingBattleCount = 0;
static EWRAM_DATA enum BoxNpcOpponentGimmickPolicy sPendingGimmickPolicy = BOX_NPC_GIMMICK_NATURAL;

static void InitResult(struct BoxNpcPartyPoolResult *result, const struct BoxNpcPartyPoolConfig *config)
{
    u32 i;

    result->boxId = BOX_NPC_POOL_BOX_ID;
    result->candidateCount = 0;
    result->battleCount = 0;
    result->poolMode = config->poolMode;
    result->battleFormat = config->battleFormat;
    result->memberMode = config->memberMode;
    result->gimmickPolicy = config->gimmickPolicy;

    for (i = 0; i < ARRAY_COUNT(result->candidateSlots); i++)
        result->candidateSlots[i] = BOX_NPC_SLOT_NONE;
    for (i = 0; i < ARRAY_COUNT(result->finalSlots); i++)
        result->finalSlots[i] = BOX_NPC_SLOT_NONE;
}

static u8 GetBattleCount(enum BoxNpcBattleFormat battleFormat)
{
    if (battleFormat == BOX_NPC_BATTLE_DOUBLE_4)
        return BOX_NPC_DOUBLE_BATTLE_SIZE;
    return BOX_NPC_SINGLE_BATTLE_SIZE;
}

static bool32 TryFillFixedCandidateRoster(struct BoxNpcPartyPoolResult *result)
{
    u32 i;

    for (i = 0; i < BOX_NPC_CANDIDATE_ROSTER_SIZE; i++)
    {
        if (!CheckBoxMonSanityAt(BOX_NPC_POOL_BOX_ID, i))
        {
            sLastError = BOX_NPC_PARTY_POOL_ERROR_FIXED_SLOT_INVALID;
            return FALSE;
        }
        result->candidateSlots[i] = i;
    }

    result->candidateCount = BOX_NPC_CANDIDATE_ROSTER_SIZE;
    return TRUE;
}

static u8 GatherValidBoxSlots(u8 *validSlots)
{
    u32 boxPosition;
    u8 count = 0;

    for (boxPosition = 0; boxPosition < IN_BOX_COUNT; boxPosition++)
    {
        if (CheckBoxMonSanityAt(BOX_NPC_POOL_BOX_ID, boxPosition))
            validSlots[count++] = boxPosition;
    }

    return count;
}

static bool32 TryFillFirstValidCandidateRoster(struct BoxNpcPartyPoolResult *result)
{
    u32 boxPosition;

    for (boxPosition = 0; boxPosition < IN_BOX_COUNT && result->candidateCount < BOX_NPC_CANDIDATE_ROSTER_SIZE; boxPosition++)
    {
        if (CheckBoxMonSanityAt(BOX_NPC_POOL_BOX_ID, boxPosition))
            result->candidateSlots[result->candidateCount++] = boxPosition;
    }

    if (result->candidateCount < BOX_NPC_CANDIDATE_ROSTER_SIZE)
    {
        sLastError = BOX_NPC_PARTY_POOL_ERROR_NOT_ENOUGH_VALID_MONS;
        return FALSE;
    }

    return TRUE;
}

static bool32 TryFillRandomCandidateRoster(struct BoxNpcPartyPoolResult *result)
{
    u8 validSlots[IN_BOX_COUNT];
    u8 validCount = GatherValidBoxSlots(validSlots);
    u32 i;

    if (validCount < BOX_NPC_CANDIDATE_ROSTER_SIZE)
    {
        sLastError = BOX_NPC_PARTY_POOL_ERROR_NOT_ENOUGH_VALID_MONS;
        return FALSE;
    }

    for (i = 0; i < BOX_NPC_CANDIDATE_ROSTER_SIZE; i++)
    {
        u32 selected = RandomUniform(RNG_BOX_NPC_PARTY_POOL_CANDIDATE, i, validCount - 1);
        u8 slot = validSlots[selected];

        validSlots[selected] = validSlots[i];
        validSlots[i] = slot;
        result->candidateSlots[i] = slot;
    }

    result->candidateCount = BOX_NPC_CANDIDATE_ROSTER_SIZE;
    return TRUE;
}

static bool32 TryFillCandidateRoster(struct BoxNpcPartyPoolResult *result)
{
    switch (result->poolMode)
    {
    case BOX_NPC_POOL_BOX1_SLOTS_1_TO_6:
        return TryFillFixedCandidateRoster(result);
    case BOX_NPC_POOL_BOX1_FIRST_VALID_6:
        return TryFillFirstValidCandidateRoster(result);
    case BOX_NPC_POOL_BOX1_RANDOM_VALID_6:
        return TryFillRandomCandidateRoster(result);
    }

    sLastError = BOX_NPC_PARTY_POOL_ERROR_NOT_ENOUGH_VALID_MONS;
    return FALSE;
}

static void FillBattleMemberIndices(const struct BoxNpcPartyPoolResult *result, u8 *memberIndices)
{
    u32 i;

    for (i = 0; i < BOX_NPC_CANDIDATE_ROSTER_SIZE; i++)
        memberIndices[i] = i;

    if (result->memberMode == BOX_NPC_BATTLE_MEMBERS_RANDOM_N)
    {
        for (i = 0; i < result->battleCount; i++)
        {
            u32 selected = RandomUniform(RNG_BOX_NPC_PARTY_POOL_BATTLE_MEMBER, i, BOX_NPC_CANDIDATE_ROSTER_SIZE - 1);
            u8 index = memberIndices[selected];

            memberIndices[selected] = memberIndices[i];
            memberIndices[i] = index;
        }
    }
}

static void CopySelectedMonsToOpponentParty(struct BoxNpcPartyPoolResult *result)
{
    u8 memberIndices[BOX_NPC_CANDIDATE_ROSTER_SIZE];
    u32 i;

    FillBattleMemberIndices(result, memberIndices);
    ZeroEnemyPartyMons();

    for (i = 0; i < result->battleCount; i++)
    {
        u8 boxSlot = result->candidateSlots[memberIndices[i]];

        result->finalSlots[i] = boxSlot;
        BoxMonAtToMon(BOX_NPC_POOL_BOX_ID, boxSlot, &gParties[B_TRAINER_OPPONENT_A][i]);
        HealPokemon(&gParties[B_TRAINER_OPPONENT_A][i]);
    }
}

static void SetPendingBattleInitPolicy(const struct BoxNpcPartyPoolResult *result)
{
    sPendingBattleInitPolicy = TRUE;
    sPendingBattleCount = result->battleCount;
    sPendingGimmickPolicy = result->gimmickPolicy;
}

static void LogSelection(const struct BoxNpcPartyPoolConfig *config, const struct BoxNpcPartyPoolResult *result)
{
    DebugPrintf(
        "BoxNPC fmt=%d pool=%d members=%d gimmick=%d aiLo=0x%08x candidates=%d,%d,%d,%d,%d,%d final=%d,%d,%d,%d",
        config->battleFormat,
        config->poolMode,
        config->memberMode,
        config->gimmickPolicy,
        (u32)config->aiFlags,
        result->candidateSlots[0],
        result->candidateSlots[1],
        result->candidateSlots[2],
        result->candidateSlots[3],
        result->candidateSlots[4],
        result->candidateSlots[5],
        result->finalSlots[0],
        result->finalSlots[1],
        result->finalSlots[2],
        result->finalSlots[3]);
}

bool32 BoxNpcPartyPool_TryBuildOpponentParty(
    const struct BoxNpcPartyPoolConfig *config,
    struct BoxNpcPartyPoolResult *result)
{
    BoxNpcPartyPool_ClearPendingBattleInitPolicy();
    InitResult(&sLastResult, config);
    sLastError = BOX_NPC_PARTY_POOL_ERROR_NONE;

    if (gPokemonStoragePtr == NULL)
    {
        sLastError = BOX_NPC_PARTY_POOL_ERROR_STORAGE_UNAVAILABLE;
        return FALSE;
    }

    if (!TryFillCandidateRoster(&sLastResult))
        return FALSE;

    sLastResult.battleCount = GetBattleCount(config->battleFormat);
    CopySelectedMonsToOpponentParty(&sLastResult);
    SetPendingBattleInitPolicy(&sLastResult);

    if (result != NULL)
        *result = sLastResult;

    LogSelection(config, &sLastResult);
    return TRUE;
}

void BoxNpcPartyPool_ApplyPendingBattleInitPolicy(void)
{
    u32 i;

    if (!sPendingBattleInitPolicy || gBattleStruct == NULL)
        return;

    if (sPendingGimmickPolicy == BOX_NPC_GIMMICK_ALLOW_TERA_DYNAMAX_ALL_FINAL_MEMBERS)
    {
        for (i = 0; i < sPendingBattleCount; i++)
        {
            gBattleStruct->opponentMonCanTera |= 1u << i;
            gBattleStruct->opponentMonCanDynamax |= 1u << i;
        }
    }

    BoxNpcPartyPool_ClearPendingBattleInitPolicy();
}

void BoxNpcPartyPool_ClearPendingBattleInitPolicy(void)
{
    sPendingBattleInitPolicy = FALSE;
    sPendingBattleCount = 0;
    sPendingGimmickPolicy = BOX_NPC_GIMMICK_NATURAL;
}

const struct BoxNpcPartyPoolResult *BoxNpcPartyPool_GetLastResult(void)
{
    return &sLastResult;
}

enum BoxNpcPartyPoolError BoxNpcPartyPool_GetLastError(void)
{
    return sLastError;
}

const u8 *BoxNpcPartyPool_GetLastErrorText(void)
{
    switch (sLastError)
    {
    case BOX_NPC_PARTY_POOL_ERROR_STORAGE_UNAVAILABLE:
        return COMPOUND_STRING("Pokemon Storage is unavailable.");
    case BOX_NPC_PARTY_POOL_ERROR_FIXED_SLOT_INVALID:
        return COMPOUND_STRING("Box 1 slots 1-6 need valid Pokemon.");
    case BOX_NPC_PARTY_POOL_ERROR_NOT_ENOUGH_VALID_MONS:
        return COMPOUND_STRING("Box 1 needs six valid Pokemon.");
    case BOX_NPC_PARTY_POOL_ERROR_NONE:
    default:
        return COMPOUND_STRING("Box NPC party pool is ready.");
    }
}
