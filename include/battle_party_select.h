#ifndef GUARD_BATTLE_PARTY_SELECT_H
#define GUARD_BATTLE_PARTY_SELECT_H

// Temporary demo entrypoint for the in-battle party selection UI.
// Opens a two-sided selection screen using the player's party on both sides.
// UI-only for now; selections are discarded on exit.
void CB2_OpenBattlePartySelectDemo(void);

// Opens the party select UI for a trainer battle using the trainer ids and mode
// provided in gSpecialVar_0x8004 (trainer A), gSpecialVar_0x8005 (trainer B or TRAINER_NONE),
// and gSpecialVar_0x8006 (0 = single, non-zero = double).
// Sets gSpecialVar_Result to TRUE/FALSE based on whether the UI opened successfully,
// and again on exit based on whether the player confirmed or canceled.
// On confirm, writes the chosen order into gSelectedOrderFromParty.
u16 StartTrainerPartySelect(void);

#endif // GUARD_BATTLE_PARTY_SELECT_H
