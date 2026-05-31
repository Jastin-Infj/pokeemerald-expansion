#include "global.h"
#include "champions_run_session.h"
#include "coins.h"
#include "event_data.h"
#include "fieldmap.h"
#include "item.h"
#include "load_save.h"
#include "main.h"
#include "money.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "save.h"
#include "script_pokemon_util.h"
#include "config/save.h"
#include "constants/battle.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/pokemon.h"
#include "constants/species.h"

#define CHAMPIONS_RUN_SESSION_SIGNATURE 0x53524843 // "CHRS"
#define CHAMPIONS_RUN_SESSION_VERSION   1

#if SAVE_CHAMPIONS_RUN_SESSION == TRUE
static struct ChampionsRunSession *GetSession(void);
static bool32 HasValidSession(const struct ChampionsRunSession *session);
static void InitSession(struct ChampionsRunSession *session);
static void SnapshotNormalState(struct ChampionsRunSession *session);
static void RestoreSnapshotToLive(const struct ChampionsRunSession *session);
static void SnapshotStartLocation(struct ChampionsRunSession *session);
static void WarpToStartLocation(const struct ChampionsRunSession *session);
static void ClearSavedMapViewForRestore(void);
static void ClearLiveRunState(void);
static void PrepareLiveRunState(const struct ChampionsRunSession *session);
static void ClearCarryoverSlots(struct ChampionsRunSession *session);
static void LoadLastClearPartyIntoLiveRun(const struct ChampionsRunSession *session);
static bool32 DepositLiveRunPartyForClear(struct ChampionsRunSession *session);
static void CarryOrStripHeldItemForClear(struct ChampionsRunSession *session, struct Pokemon *mon);
static void CarryRunBagItemsIntoNormalSnapshot(struct ChampionsRunSession *session);
static void CopyBagToSnapshot(struct Bag *dst, const struct Bag *src);
static void CopyBagFromSnapshot(struct Bag *dst, const struct Bag *src);
static void CopyItemSlotsToSnapshot(struct ItemSlot *dst, const struct ItemSlot *src, u32 count);
static void CopyItemSlotsFromSnapshot(struct ItemSlot *dst, const struct ItemSlot *src, u32 count);
static bool32 AddItemToSnapshotBag(struct Bag *bag, enum Item itemId, u16 count);
static bool32 AddItemToSnapshotSlots(struct ItemSlot *slots, u32 capacity, bool32 singleStack, enum Item itemId, u16 count);
static void CarryItemSlotsIntoNormalSnapshot(struct ChampionsRunSession *session, const struct ItemSlot *slots, u32 count);
static bool32 ShouldCarryRunBagItem(enum Item itemId);
static u32 CountFreePokemonStorageSlots(void);
static bool32 IsDefeatOutcome(u8 battleOutcome);
static void RestoreNormalState(struct ChampionsRunSession *session, u8 outcome, bool32 warpToStartLocation);

static struct ChampionsRunSession *GetSession(void)
{
    return &gSaveBlock1Ptr->championsRun;
}

static bool32 HasValidSession(const struct ChampionsRunSession *session)
{
    return session->signature == CHAMPIONS_RUN_SESSION_SIGNATURE
        && session->version == CHAMPIONS_RUN_SESSION_VERSION;
}

static void InitSession(struct ChampionsRunSession *session)
{
    u8 lastClearPartyCount = 0;
    u8 lastClearBoxIds[PARTY_SIZE];
    u8 lastClearBoxPositions[PARTY_SIZE];

    if (HasValidSession(session))
    {
        lastClearPartyCount = session->lastClearPartyCount;
        memcpy(lastClearBoxIds, session->lastClearBoxIds, sizeof(lastClearBoxIds));
        memcpy(lastClearBoxPositions, session->lastClearBoxPositions, sizeof(lastClearBoxPositions));
    }
    else
    {
        memset(lastClearBoxIds, 0xFF, sizeof(lastClearBoxIds));
        memset(lastClearBoxPositions, 0xFF, sizeof(lastClearBoxPositions));
    }

    memset(session, 0, sizeof(*session));
    session->signature = CHAMPIONS_RUN_SESSION_SIGNATURE;
    session->version = CHAMPIONS_RUN_SESSION_VERSION;
    session->requiredPartyCount = PARTY_SIZE;
    session->lastClearPartyCount = lastClearPartyCount;
    memcpy(session->lastClearBoxIds, lastClearBoxIds, sizeof(session->lastClearBoxIds));
    memcpy(session->lastClearBoxPositions, lastClearBoxPositions, sizeof(session->lastClearBoxPositions));
}

static void CopyItemSlotsToSnapshot(struct ItemSlot *dst, const struct ItemSlot *src, u32 count)
{
    u32 i;
    u32 key = gSaveBlock2Ptr->encryptionKey;

    for (i = 0; i < count; i++)
    {
        dst[i].itemId = src[i].itemId;
        if (src[i].itemId == ITEM_NONE)
            dst[i].quantity = 0;
        else
            dst[i].quantity = src[i].quantity ^ key;
    }
}

static void CopyItemSlotsFromSnapshot(struct ItemSlot *dst, const struct ItemSlot *src, u32 count)
{
    u32 i;
    u32 key = gSaveBlock2Ptr->encryptionKey;

    for (i = 0; i < count; i++)
    {
        if (src[i].itemId == ITEM_NONE || src[i].quantity == 0)
        {
            dst[i].itemId = ITEM_NONE;
            dst[i].quantity = 0;
        }
        else
        {
            dst[i].itemId = src[i].itemId;
            dst[i].quantity = src[i].quantity ^ key;
        }
    }
}

static void CopyBagToSnapshot(struct Bag *dst, const struct Bag *src)
{
    CopyItemSlotsToSnapshot(dst->items, src->items, ARRAY_COUNT(dst->items));
    CopyItemSlotsToSnapshot(dst->keyItems, src->keyItems, ARRAY_COUNT(dst->keyItems));
    CopyItemSlotsToSnapshot(dst->pokeBalls, src->pokeBalls, ARRAY_COUNT(dst->pokeBalls));
    CopyItemSlotsToSnapshot(dst->TMsHMs, src->TMsHMs, ARRAY_COUNT(dst->TMsHMs));
    CopyItemSlotsToSnapshot(dst->berries, src->berries, ARRAY_COUNT(dst->berries));
}

static void CopyBagFromSnapshot(struct Bag *dst, const struct Bag *src)
{
    CopyItemSlotsFromSnapshot(dst->items, src->items, ARRAY_COUNT(dst->items));
    CopyItemSlotsFromSnapshot(dst->keyItems, src->keyItems, ARRAY_COUNT(dst->keyItems));
    CopyItemSlotsFromSnapshot(dst->pokeBalls, src->pokeBalls, ARRAY_COUNT(dst->pokeBalls));
    CopyItemSlotsFromSnapshot(dst->TMsHMs, src->TMsHMs, ARRAY_COUNT(dst->TMsHMs));
    CopyItemSlotsFromSnapshot(dst->berries, src->berries, ARRAY_COUNT(dst->berries));
}

static void SnapshotNormalState(struct ChampionsRunSession *session)
{
    CalculatePlayerPartyCount();

    SnapshotStartLocation(session);
    session->normalPartyCount = gPartiesCount[B_TRAINER_PLAYER];
    memcpy(session->normalParty, gParties[B_TRAINER_PLAYER], sizeof(session->normalParty));
    CopyBagToSnapshot(&session->normalBag, &gSaveBlock1Ptr->bag);
    memcpy(session->normalMail, gSaveBlock1Ptr->mail, sizeof(session->normalMail));
    session->normalMoney = GetMoney(&gSaveBlock1Ptr->money);
    session->normalCoins = GetCoins();
    session->normalRegisteredItem = gSaveBlock1Ptr->registeredItem;
}

static void SnapshotStartLocation(struct ChampionsRunSession *session)
{
    session->startLocation.mapGroup = gSaveBlock1Ptr->location.mapGroup;
    session->startLocation.mapNum = gSaveBlock1Ptr->location.mapNum;
    session->startLocation.warpId = WARP_ID_NONE;
    session->startLocation.x = gSaveBlock1Ptr->pos.x;
    session->startLocation.y = gSaveBlock1Ptr->pos.y;
}

static void WarpToStartLocation(const struct ChampionsRunSession *session)
{
    gSaveBlock1Ptr->dynamicWarp = session->startLocation;
    gSaveBlock1Ptr->continueGameWarp = session->startLocation;
    SetWarpDestinationToDynamicWarp(0);
    WarpIntoMap();
    ResetInitialPlayerAvatarState();
    SetContinueGameWarpStatus();
}

static void ClearSavedMapViewForRestore(void)
{
    memset(gSaveBlock1Ptr->mapView, 0, sizeof(gSaveBlock1Ptr->mapView));
}

static void RestoreSnapshotToLive(const struct ChampionsRunSession *session)
{
    memcpy(gParties[B_TRAINER_PLAYER], session->normalParty, sizeof(session->normalParty));
    gPartiesCount[B_TRAINER_PLAYER] = session->normalPartyCount;
    CopyBagFromSnapshot(&gSaveBlock1Ptr->bag, &session->normalBag);
    memcpy(gSaveBlock1Ptr->mail, session->normalMail, sizeof(session->normalMail));
    SetMoney(&gSaveBlock1Ptr->money, session->normalMoney);
    SetCoins(session->normalCoins);
    gSaveBlock1Ptr->registeredItem = session->normalRegisteredItem;
}

static void ClearLiveRunState(void)
{
    ZeroPlayerPartyMons();
    gPartiesCount[B_TRAINER_PLAYER] = 0;
    ClearBag();
    memset(gSaveBlock1Ptr->mail, 0, sizeof(gSaveBlock1Ptr->mail));
    SetMoney(&gSaveBlock1Ptr->money, 0);
    SetCoins(0);
    gSaveBlock1Ptr->registeredItem = ITEM_NONE;
}

static void PrepareLiveRunState(const struct ChampionsRunSession *session)
{
    switch (CHAMPIONS_RUN_ENTRY_PARTY_MODE)
    {
    case CHAMPIONS_RUN_START_PARTY_CURRENT:
        ClearBag();
        memset(gSaveBlock1Ptr->mail, 0, sizeof(gSaveBlock1Ptr->mail));
        SetMoney(&gSaveBlock1Ptr->money, 0);
        SetCoins(0);
        gSaveBlock1Ptr->registeredItem = ITEM_NONE;
        break;
    case CHAMPIONS_RUN_START_PARTY_LAST_CLEAR:
        ClearLiveRunState();
        LoadLastClearPartyIntoLiveRun(session);
        break;
    case CHAMPIONS_RUN_START_PARTY_EMPTY:
    default:
        ClearLiveRunState();
        break;
    }
}

static void ClearCarryoverSlots(struct ChampionsRunSession *session)
{
    session->lastClearPartyCount = 0;
    memset(session->lastClearBoxIds, 0xFF, sizeof(session->lastClearBoxIds));
    memset(session->lastClearBoxPositions, 0xFF, sizeof(session->lastClearBoxPositions));
}

static void LoadLastClearPartyIntoLiveRun(const struct ChampionsRunSession *session)
{
    u32 i;
    u32 partySlot = 0;
    u32 count = min(session->lastClearPartyCount, PARTY_SIZE);

    for (i = 0; i < count && partySlot < PARTY_SIZE; i++)
    {
        u8 boxId = session->lastClearBoxIds[i];
        u8 boxPosition = session->lastClearBoxPositions[i];

        if (boxId >= TOTAL_BOXES_COUNT || boxPosition >= IN_BOX_COUNT)
            continue;
        if (GetBoxMonDataAt(boxId, boxPosition, MON_DATA_SPECIES) == SPECIES_NONE)
            continue;

        BoxMonAtToMon(boxId, boxPosition, &gParties[B_TRAINER_PLAYER][partySlot]);
        partySlot++;
    }

    gPartiesCount[B_TRAINER_PLAYER] = partySlot;
}

static u32 CountFreePokemonStorageSlots(void)
{
    u32 boxId;
    u32 boxPosition;
    u32 count = 0;

    for (boxId = 0; boxId < TOTAL_BOXES_COUNT; boxId++)
    {
        for (boxPosition = 0; boxPosition < IN_BOX_COUNT; boxPosition++)
        {
            if (GetBoxMonDataAt(boxId, boxPosition, MON_DATA_SPECIES) == SPECIES_NONE)
                count++;
        }
    }

    return count;
}

static void CarryOrStripHeldItemForClear(struct ChampionsRunSession *session, struct Pokemon *mon)
{
    u16 noItem = ITEM_NONE;
    u16 heldItem = GetMonData(mon, MON_DATA_HELD_ITEM);

    switch (CHAMPIONS_RUN_CLEAR_HELD_ITEM_MODE)
    {
    case CHAMPIONS_RUN_HELD_ITEM_CARRY_ON_MON:
        break;
    case CHAMPIONS_RUN_HELD_ITEM_CARRY_TO_BAG:
        SetMonData(mon, MON_DATA_HELD_ITEM, &noItem);
        if (heldItem != ITEM_NONE)
            AddItemToSnapshotBag(&session->normalBag, heldItem, 1);
        break;
    case CHAMPIONS_RUN_HELD_ITEM_CARRY_NONE:
    default:
        SetMonData(mon, MON_DATA_HELD_ITEM, &noItem);
        break;
    }
}

static bool32 DepositLiveRunPartyForClear(struct ChampionsRunSession *session)
{
#if CHAMPIONS_RUN_CLEAR_DEPOSIT_PARTY == TRUE
    u32 i;
    u32 runPartyCount = 0;
    struct Pokemon mon;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_SPECIES) != SPECIES_NONE)
            runPartyCount++;
    }

    if (CountFreePokemonStorageSlots() < runPartyCount)
        return FALSE;

    ClearCarryoverSlots(session);
    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_SPECIES) == SPECIES_NONE)
            continue;

        mon = gParties[B_TRAINER_PLAYER][i];
        CarryOrStripHeldItemForClear(session, &mon);
        if (CopyMonToPC(&mon) != MON_GIVEN_TO_PC)
            return FALSE;

        session->lastClearBoxIds[session->lastClearPartyCount] = gSpecialVar_MonBoxId;
        session->lastClearBoxPositions[session->lastClearPartyCount] = gSpecialVar_MonBoxPos;
        session->lastClearPartyCount++;
    }
#else
    ClearCarryoverSlots(session);
#endif

    return TRUE;
}

static bool32 AddItemToSnapshotSlots(struct ItemSlot *slots, u32 capacity, bool32 singleStack, enum Item itemId, u16 count)
{
    u32 i;

    for (i = 0; i < capacity && count > 0; i++)
    {
        if (slots[i].itemId == itemId)
        {
            u16 space = MAX_BAG_ITEM_CAPACITY - min(slots[i].quantity, MAX_BAG_ITEM_CAPACITY);
            u16 toAdd = min(count, space);

            slots[i].quantity += toAdd;
            count -= toAdd;
            if (singleStack)
                return count == 0;
        }
    }

    for (i = 0; i < capacity && count > 0; i++)
    {
        if (slots[i].itemId == ITEM_NONE || slots[i].quantity == 0)
        {
            u16 toAdd = min(count, MAX_BAG_ITEM_CAPACITY);

            slots[i].itemId = itemId;
            slots[i].quantity = toAdd;
            count -= toAdd;
            if (singleStack)
                return count == 0;
        }
    }

    return count == 0;
}

static bool32 AddItemToSnapshotBag(struct Bag *bag, enum Item itemId, u16 count)
{
    switch (GetItemPocket(itemId))
    {
    case POCKET_ITEMS:
        return AddItemToSnapshotSlots(bag->items, ARRAY_COUNT(bag->items), FALSE, itemId, count);
    case POCKET_POKE_BALLS:
        return AddItemToSnapshotSlots(bag->pokeBalls, ARRAY_COUNT(bag->pokeBalls), FALSE, itemId, count);
    case POCKET_TM_HM:
        return AddItemToSnapshotSlots(bag->TMsHMs, ARRAY_COUNT(bag->TMsHMs), TRUE, itemId, count);
    case POCKET_BERRIES:
        return AddItemToSnapshotSlots(bag->berries, ARRAY_COUNT(bag->berries), TRUE, itemId, count);
    case POCKET_KEY_ITEMS:
        return AddItemToSnapshotSlots(bag->keyItems, ARRAY_COUNT(bag->keyItems), FALSE, itemId, count);
    default:
        return FALSE;
    }
}

static bool32 ShouldCarryRunBagItem(enum Item itemId)
{
    switch (GetItemPocket(itemId))
    {
    case POCKET_ITEMS:
        return CHAMPIONS_RUN_CLEAR_CARRY_ITEMS;
    case POCKET_POKE_BALLS:
        return CHAMPIONS_RUN_CLEAR_CARRY_POKE_BALLS;
    case POCKET_TM_HM:
        return CHAMPIONS_RUN_CLEAR_CARRY_TMS_HMS;
    case POCKET_BERRIES:
        return CHAMPIONS_RUN_CLEAR_CARRY_BERRIES;
    case POCKET_KEY_ITEMS:
        return CHAMPIONS_RUN_CLEAR_CARRY_KEY_ITEMS;
    default:
        return FALSE;
    }
}

static void CarryItemSlotsIntoNormalSnapshot(struct ChampionsRunSession *session, const struct ItemSlot *slots, u32 count)
{
    u32 i;
    u32 key = gSaveBlock2Ptr->encryptionKey;

    for (i = 0; i < count; i++)
    {
        enum Item itemId = slots[i].itemId;
        u16 quantity;

        if (itemId == ITEM_NONE)
            continue;
        if (!ShouldCarryRunBagItem(itemId))
            continue;

        quantity = slots[i].quantity ^ key;
        if (quantity == 0)
            continue;

        AddItemToSnapshotBag(&session->normalBag, itemId, quantity);
    }
}

static void CarryRunBagItemsIntoNormalSnapshot(struct ChampionsRunSession *session)
{
    CarryItemSlotsIntoNormalSnapshot(session, gSaveBlock1Ptr->bag.items, ARRAY_COUNT(gSaveBlock1Ptr->bag.items));
    CarryItemSlotsIntoNormalSnapshot(session, gSaveBlock1Ptr->bag.keyItems, ARRAY_COUNT(gSaveBlock1Ptr->bag.keyItems));
    CarryItemSlotsIntoNormalSnapshot(session, gSaveBlock1Ptr->bag.pokeBalls, ARRAY_COUNT(gSaveBlock1Ptr->bag.pokeBalls));
    CarryItemSlotsIntoNormalSnapshot(session, gSaveBlock1Ptr->bag.TMsHMs, ARRAY_COUNT(gSaveBlock1Ptr->bag.TMsHMs));
    CarryItemSlotsIntoNormalSnapshot(session, gSaveBlock1Ptr->bag.berries, ARRAY_COUNT(gSaveBlock1Ptr->bag.berries));
}

bool32 ChampionsRun_IsActive(void)
{
    struct ChampionsRunSession *session = GetSession();

    return HasValidSession(session) && session->active;
}

bool32 ChampionsRun_ShouldUseTemporarySave(u8 saveType)
{
    return saveType == SAVE_NORMAL && ChampionsRun_IsActive();
}

bool32 ChampionsRun_ShouldBlockBagUse(void)
{
    return ChampionsRun_IsActive();
}

bool32 ChampionsRun_ShouldBlockHeldItemChanges(void)
{
    return ChampionsRun_IsActive();
}

bool32 ChampionsRun_ShouldSuppressExp(void)
{
    return ChampionsRun_IsActive();
}

bool32 ChampionsRun_BeginEntry(void)
{
    struct ChampionsRunSession *session = GetSession();

    if (ChampionsRun_IsActive())
        return FALSE;

    InitSession(session);
    SnapshotNormalState(session);
    session->active = TRUE;
    session->status = CHAMPIONS_RUN_STATUS_ACTIVE;
    session->checkpointKind = CHAMPIONS_RUN_CHECKPOINT_ENTRY;
    PrepareLiveRunState(session);
    return TRUE;
}

u8 ChampionsRun_BeginEntryReport(void)
{
    u8 status;

    if (!ChampionsRun_BeginEntry())
        return SAVE_STATUS_ERROR;

    status = ChampionsRun_SaveCheckpoint(CHAMPIONS_RUN_CHECKPOINT_ENTRY);
    if (status != SAVE_STATUS_OK)
        ChampionsRun_RestoreNormalState(CHAMPIONS_RUN_STATUS_RECOVERING);

    return status;
}

u8 ChampionsRun_SaveCheckpoint(u8 checkpointKind)
{
    struct ChampionsRunSession *session = GetSession();

    if (!ChampionsRun_IsActive())
        return SAVE_STATUS_ERROR;

    session->status = checkpointKind == CHAMPIONS_RUN_CHECKPOINT_ENTRY
        ? CHAMPIONS_RUN_STATUS_ENTRY_SAVED
        : CHAMPIONS_RUN_STATUS_PAUSED;
    session->checkpointKind = checkpointKind;

    SaveMapView();
    return TrySavingData(SAVE_LINK);
}

static bool32 IsDefeatOutcome(u8 battleOutcome)
{
    switch (battleOutcome)
    {
    case B_OUTCOME_LOST:
    case B_OUTCOME_DREW:
    case B_OUTCOME_FORFEITED:
        return TRUE;
    default:
        return FALSE;
    }
}

static void RestoreNormalState(struct ChampionsRunSession *session, u8 outcome, bool32 warpToStartLocation)
{
    RestoreSnapshotToLive(session);
    if (warpToStartLocation)
    {
        ClearSavedMapViewForRestore();
        WarpToStartLocation(session);
    }
    InitSession(session);
    session->status = outcome;
    session->outcome = outcome;
}

bool32 ChampionsRun_EndByBattleOutcome(u8 battleOutcome)
{
    u8 saveStatus;
    struct ChampionsRunSession *session = GetSession();

    if (!ChampionsRun_IsActive() || !IsDefeatOutcome(battleOutcome))
        return FALSE;

    RestoreNormalState(session, CHAMPIONS_RUN_STATUS_LOST, TRUE);
    session->outcome = battleOutcome;

    saveStatus = TrySavingData(SAVE_NORMAL);
    ClearContinueGameWarpStatus();
    (void)saveStatus;
    return TRUE;
}

void ChampionsRun_RestoreNormalState(u8 outcome)
{
    struct ChampionsRunSession *session = GetSession();

    if (!ChampionsRun_IsActive())
        return;

    RestoreNormalState(session, outcome, FALSE);
}

u8 ChampionsRun_RestoreNormalStateAndSave(u8 outcome)
{
    if (!ChampionsRun_IsActive())
        return SAVE_STATUS_ERROR;

    ChampionsRun_RestoreNormalState(outcome);
    return TrySavingData(SAVE_NORMAL);
}

u8 ChampionsRun_RetireAndSave(void)
{
    u8 saveStatus;
    struct ChampionsRunSession *session = GetSession();

    if (!ChampionsRun_IsActive())
        return SAVE_STATUS_ERROR;

    RestoreNormalState(session, CHAMPIONS_RUN_STATUS_RETIRED, TRUE);
    saveStatus = TrySavingData(SAVE_NORMAL);
    ClearContinueGameWarpStatus();
    return saveStatus;
}

bool32 ChampionsRun_CompleteClear(void)
{
    struct ChampionsRunSession *session = GetSession();

    if (!ChampionsRun_IsActive())
        return FALSE;
    if (!DepositLiveRunPartyForClear(session))
        return FALSE;

    CarryRunBagItemsIntoNormalSnapshot(session);
    RestoreSnapshotToLive(session);
    InitSession(session);
    session->status = CHAMPIONS_RUN_STATUS_WON;
    session->outcome = B_OUTCOME_WON;
    return TRUE;
}

u8 ChampionsRun_CompleteClearAndSave(void)
{
    if (!ChampionsRun_CompleteClear())
        return SAVE_STATUS_ERROR;

#if CHAMPIONS_RUN_CLEAR_AUTOSAVE == TRUE
    return TrySavingData(SAVE_NORMAL);
#else
    return SAVE_STATUS_OK;
#endif
}

void ChampionsRun_HandleBootRecovery(void)
{
    struct ChampionsRunSession *session = GetSession();

    if (!ChampionsRun_IsActive())
        return;

    CalculatePlayerPartyCount();
    if (session->status == CHAMPIONS_RUN_STATUS_ENTRY_SAVED
     || session->status == CHAMPIONS_RUN_STATUS_ACTIVE)
        session->status = CHAMPIONS_RUN_STATUS_PAUSED;
}

void ChampionsRun_CanUseNormalPc(void)
{
    gSpecialVar_Result = !ChampionsRun_IsActive();
}

void ChampionsRun_ReloadMapAfterRestore(void)
{
    gMain.state = 0;
    SetMainCallback2(CB2_LoadMap);
}

void ChampionsRun_DebugBegin(void)
{
    gSpecialVar_Result = ChampionsRun_BeginEntryReport() == SAVE_STATUS_OK;
}

void ChampionsRun_DebugGiveRunMon(void)
{
    bool32 gaveMon;

    if (!ChampionsRun_IsActive())
    {
        gSpecialVar_Result = FALSE;
        return;
    }

    gaveMon = ScriptGiveMon(SPECIES_DRAGONITE, 50, ITEM_NONE) != MON_CANT_GIVE;
    gSpecialVar_Result = gaveMon
        && ChampionsRun_SaveCheckpoint(CHAMPIONS_RUN_CHECKPOINT_DEBUG) == SAVE_STATUS_OK;
}

void ChampionsRun_DebugSaveCheckpoint(void)
{
    gSpecialVar_Result = ChampionsRun_SaveCheckpoint(CHAMPIONS_RUN_CHECKPOINT_DEBUG) == SAVE_STATUS_OK;
}

void ChampionsRun_DebugRestoreNormal(void)
{
    gSpecialVar_Result = ChampionsRun_RetireAndSave() == SAVE_STATUS_OK;
}

void ChampionsRun_DebugPrepareLoseTest(void)
{
    if (!ChampionsRun_IsActive() && ChampionsRun_BeginEntryReport() != SAVE_STATUS_OK)
    {
        gSpecialVar_Result = FALSE;
        return;
    }

    ZeroPlayerPartyMons();
    CreateMon(&gParties[B_TRAINER_PLAYER][0], SPECIES_MAGIKARP, 1, 0, OTID_STRUCT_PLAYER_ID);
    CalculatePlayerPartyCount();
    gSpecialVar_Result = ChampionsRun_SaveCheckpoint(CHAMPIONS_RUN_CHECKPOINT_DEBUG) == SAVE_STATUS_OK;
}

void ChampionsRun_DebugClear(void)
{
    gSpecialVar_Result = ChampionsRun_CompleteClearAndSave() == SAVE_STATUS_OK;
}
#else
bool32 ChampionsRun_IsActive(void)
{
    return FALSE;
}

bool32 ChampionsRun_ShouldUseTemporarySave(u8 saveType)
{
    (void)saveType;
    return FALSE;
}

bool32 ChampionsRun_BeginEntry(void)
{
    return FALSE;
}

u8 ChampionsRun_BeginEntryReport(void)
{
    return SAVE_STATUS_ERROR;
}

u8 ChampionsRun_SaveCheckpoint(u8 checkpointKind)
{
    (void)checkpointKind;
    return SAVE_STATUS_ERROR;
}

bool32 ChampionsRun_EndByBattleOutcome(u8 battleOutcome)
{
    (void)battleOutcome;
    return FALSE;
}

void ChampionsRun_RestoreNormalState(u8 outcome)
{
    (void)outcome;
}

u8 ChampionsRun_RestoreNormalStateAndSave(u8 outcome)
{
    (void)outcome;
    return SAVE_STATUS_ERROR;
}

u8 ChampionsRun_RetireAndSave(void)
{
    return SAVE_STATUS_ERROR;
}

bool32 ChampionsRun_CompleteClear(void)
{
    return FALSE;
}

u8 ChampionsRun_CompleteClearAndSave(void)
{
    return SAVE_STATUS_ERROR;
}

void ChampionsRun_HandleBootRecovery(void)
{
}

void ChampionsRun_CanUseNormalPc(void)
{
    gSpecialVar_Result = TRUE;
}

void ChampionsRun_ReloadMapAfterRestore(void)
{
}

bool32 ChampionsRun_ShouldBlockBagUse(void)
{
    return FALSE;
}

bool32 ChampionsRun_ShouldBlockHeldItemChanges(void)
{
    return FALSE;
}

bool32 ChampionsRun_ShouldSuppressExp(void)
{
    return FALSE;
}

void ChampionsRun_DebugBegin(void)
{
    gSpecialVar_Result = FALSE;
}

void ChampionsRun_DebugGiveRunMon(void)
{
    gSpecialVar_Result = FALSE;
}

void ChampionsRun_DebugSaveCheckpoint(void)
{
    gSpecialVar_Result = FALSE;
}

void ChampionsRun_DebugRestoreNormal(void)
{
    gSpecialVar_Result = FALSE;
}

void ChampionsRun_DebugPrepareLoseTest(void)
{
    gSpecialVar_Result = FALSE;
}

void ChampionsRun_DebugClear(void)
{
    gSpecialVar_Result = FALSE;
}
#endif
