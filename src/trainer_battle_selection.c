#include "global.h"
#include "party_menu.h"
#include "pokemon.h"
#include "trainer_battle_selection.h"

struct TrainerBattleSelectionState
{
    bool8 active;
    u8 originalPartyCount;
    u8 selectedCount;
    u8 selectedSlots[MAX_FRONTIER_PARTY_SIZE];
    struct Pokemon originalParty[PARTY_SIZE];
};

static EWRAM_DATA struct TrainerBattleSelectionState sTrainerBattleSelectionState = {0};

static void ClearTrainerBattleSelectionState(void)
{
    memset(&sTrainerBattleSelectionState, 0, sizeof(sTrainerBattleSelectionState));
}

bool32 TrainerBattleSelection_Begin(u8 selectedCount, MainCallback callback, MainCallback backCallback)
{
    if (selectedCount == 0 || selectedCount > MAX_FRONTIER_PARTY_SIZE)
        return FALSE;

    ClearTrainerBattleSelectionState();
    InitChooseHalfPartyForTrainerBattleSelection(selectedCount, callback, backCallback);
    return TRUE;
}

void TrainerBattleSelection_StartBattleFromSelection(void)
{
    u8 i;
    struct Pokemon selectedParty[MAX_FRONTIER_PARTY_SIZE];

    ClearChooseHalfPartyTrainerBattleSelection();
    sTrainerBattleSelectionState.originalPartyCount = CalculatePlayerPartyCount();
    sTrainerBattleSelectionState.selectedCount = 0;

    for (i = 0; i < PARTY_SIZE; i++)
        CopyMon(&sTrainerBattleSelectionState.originalParty[i], &gParties[B_TRAINER_PLAYER][i], sizeof(struct Pokemon));

    for (i = 0; i < MAX_FRONTIER_PARTY_SIZE && gSelectedOrderFromParty[i] != 0; i++)
    {
        u8 selectedSlot = gSelectedOrderFromParty[i] - 1;

        if (selectedSlot >= PARTY_SIZE)
        {
            ClearTrainerBattleSelectionState();
            ClearSelectedPartyOrder();
            return;
        }

        sTrainerBattleSelectionState.selectedSlots[i] = selectedSlot;
        CopyMon(&selectedParty[i], &gParties[B_TRAINER_PLAYER][sTrainerBattleSelectionState.selectedSlots[i]], sizeof(struct Pokemon));
        sTrainerBattleSelectionState.selectedCount++;
    }

    if (sTrainerBattleSelectionState.selectedCount == 0)
    {
        ClearTrainerBattleSelectionState();
        ClearSelectedPartyOrder();
        return;
    }

    for (i = 0; i < PARTY_SIZE; i++)
        ZeroMonData(&gParties[B_TRAINER_PLAYER][i]);

    for (i = 0; i < sTrainerBattleSelectionState.selectedCount; i++)
        CopyMon(&gParties[B_TRAINER_PLAYER][i], &selectedParty[i], sizeof(struct Pokemon));

    sTrainerBattleSelectionState.active = TRUE;
    ClearSelectedPartyOrder();
    CalculatePlayerPartyCount();
}

void TrainerBattleSelection_RestoreIfActive(void)
{
    u8 i;

    if (!sTrainerBattleSelectionState.active)
        return;

    for (i = 0; i < sTrainerBattleSelectionState.selectedCount; i++)
    {
        u8 originalSlot = sTrainerBattleSelectionState.selectedSlots[i];

        if (originalSlot < PARTY_SIZE)
            CopyMon(&sTrainerBattleSelectionState.originalParty[originalSlot], &gParties[B_TRAINER_PLAYER][i], sizeof(struct Pokemon));
    }

    for (i = 0; i < PARTY_SIZE; i++)
        ZeroMonData(&gParties[B_TRAINER_PLAYER][i]);

    for (i = 0; i < sTrainerBattleSelectionState.originalPartyCount && i < PARTY_SIZE; i++)
        CopyMon(&gParties[B_TRAINER_PLAYER][i], &sTrainerBattleSelectionState.originalParty[i], sizeof(struct Pokemon));

    ClearTrainerBattleSelectionState();
    CalculatePlayerPartyCount();
}

bool32 TrainerBattleSelection_ForEachOriginalPartyMon(TrainerBattleSelectionPartyMonFunc func, void *context)
{
    u8 i;

    if (!sTrainerBattleSelectionState.active || func == NULL)
        return FALSE;

    for (i = 0; i < sTrainerBattleSelectionState.originalPartyCount && i < PARTY_SIZE; i++)
        func(&sTrainerBattleSelectionState.originalParty[i], context);

    return TRUE;
}

bool32 TrainerBattleSelection_ForEachSelectedBattleMon(TrainerBattleSelectionPartyMonFunc func, void *context)
{
    u8 i;

    if (!sTrainerBattleSelectionState.active || func == NULL)
        return FALSE;

    for (i = 0; i < sTrainerBattleSelectionState.selectedCount && i < PARTY_SIZE; i++)
        func(&gParties[B_TRAINER_PLAYER][i], context);

    return TRUE;
}
