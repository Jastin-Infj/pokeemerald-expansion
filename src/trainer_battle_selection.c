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

bool32 TrainerBattleSelection_Begin(u8 selectedCount, MainCallback callback)
{
    if (selectedCount == 0 || selectedCount > MAX_FRONTIER_PARTY_SIZE)
        return FALSE;

    ClearTrainerBattleSelectionState();
    InitChooseHalfPartyForTrainerBattleSelection(selectedCount, callback);
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
        CopyMon(&sTrainerBattleSelectionState.originalParty[i], &gPlayerParty[i], sizeof(struct Pokemon));

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
        CopyMon(&selectedParty[i], &gPlayerParty[sTrainerBattleSelectionState.selectedSlots[i]], sizeof(struct Pokemon));
        sTrainerBattleSelectionState.selectedCount++;
    }

    if (sTrainerBattleSelectionState.selectedCount == 0)
    {
        ClearTrainerBattleSelectionState();
        ClearSelectedPartyOrder();
        return;
    }

    for (i = 0; i < PARTY_SIZE; i++)
        ZeroMonData(&gPlayerParty[i]);

    for (i = 0; i < sTrainerBattleSelectionState.selectedCount; i++)
        CopyMon(&gPlayerParty[i], &selectedParty[i], sizeof(struct Pokemon));

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
            CopyMon(&sTrainerBattleSelectionState.originalParty[originalSlot], &gPlayerParty[i], sizeof(struct Pokemon));
    }

    for (i = 0; i < PARTY_SIZE; i++)
        ZeroMonData(&gPlayerParty[i]);

    for (i = 0; i < sTrainerBattleSelectionState.originalPartyCount && i < PARTY_SIZE; i++)
        CopyMon(&gPlayerParty[i], &sTrainerBattleSelectionState.originalParty[i], sizeof(struct Pokemon));

    ClearTrainerBattleSelectionState();
    CalculatePlayerPartyCount();
}
