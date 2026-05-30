#ifndef GUARD_CONFIG_SAVE_H
#define GUARD_CONFIG_SAVE_H

// Menu configs
#define SKIP_SAVE_CONFIRMATION              FALSE   // If TRUE, skips the "There is already a saved file" confirmation when overwriting a save.

// Feature save configs
#define SAVE_CHAMPIONS_RUN_SESSION          TRUE    // If TRUE, stores Champions challenge normal-state snapshots in SaveBlock1.

#define CHAMPIONS_RUN_START_PARTY_EMPTY       0     // Start each run with no Pokemon.
#define CHAMPIONS_RUN_START_PARTY_LAST_CLEAR  1     // Start with the last clear party copied from Pokemon Storage.
#define CHAMPIONS_RUN_START_PARTY_CURRENT     2     // Start with the player's current normal party copied into the run.
#define CHAMPIONS_RUN_ENTRY_PARTY_MODE        CHAMPIONS_RUN_START_PARTY_EMPTY

#define CHAMPIONS_RUN_HELD_ITEM_CARRY_NONE       0  // Strip held items from clear-party Pokemon and do not carry them.
#define CHAMPIONS_RUN_HELD_ITEM_CARRY_ON_MON     1  // Keep held items on deposited clear-party Pokemon.
#define CHAMPIONS_RUN_HELD_ITEM_CARRY_TO_BAG     2  // Strip held items from deposited Pokemon and merge them into the restored normal bag if there is room.

#define CHAMPIONS_RUN_CLEAR_DEPOSIT_PARTY   TRUE    // If TRUE, completing a Champions run copies the live run party into Pokemon Storage before restoring normal state.
#define CHAMPIONS_RUN_CLEAR_HELD_ITEM_MODE  CHAMPIONS_RUN_HELD_ITEM_CARRY_NONE
#define CHAMPIONS_RUN_CLEAR_CARRY_ITEMS     TRUE    // If TRUE, regular Items-pocket rewards found during a cleared run are merged into the restored normal bag.
#define CHAMPIONS_RUN_CLEAR_CARRY_POKE_BALLS FALSE  // If TRUE, Poke Balls-pocket rewards found during a cleared run are merged into the restored normal bag.
#define CHAMPIONS_RUN_CLEAR_CARRY_TMS_HMS   TRUE    // If TRUE, TM/HM-pocket rewards found during a cleared run are merged into the restored normal bag.
#define CHAMPIONS_RUN_CLEAR_CARRY_BERRIES   FALSE   // If TRUE, Berries-pocket rewards found during a cleared run are merged into the restored normal bag.
#define CHAMPIONS_RUN_CLEAR_CARRY_KEY_ITEMS FALSE   // If TRUE, Key Items-pocket rewards found during a cleared run are merged into the restored normal bag.
#define CHAMPIONS_RUN_CLEAR_AUTOSAVE        TRUE    // If TRUE, completing a Champions run writes a full normal save after carryover and restore.

#if CHAMPIONS_RUN_ENTRY_PARTY_MODE < CHAMPIONS_RUN_START_PARTY_EMPTY || CHAMPIONS_RUN_ENTRY_PARTY_MODE > CHAMPIONS_RUN_START_PARTY_CURRENT
#error "CHAMPIONS_RUN_ENTRY_PARTY_MODE must be CHAMPIONS_RUN_START_PARTY_EMPTY, CHAMPIONS_RUN_START_PARTY_LAST_CLEAR, or CHAMPIONS_RUN_START_PARTY_CURRENT."
#endif

#if CHAMPIONS_RUN_CLEAR_HELD_ITEM_MODE < CHAMPIONS_RUN_HELD_ITEM_CARRY_NONE || CHAMPIONS_RUN_CLEAR_HELD_ITEM_MODE > CHAMPIONS_RUN_HELD_ITEM_CARRY_TO_BAG
#error "CHAMPIONS_RUN_CLEAR_HELD_ITEM_MODE must be CHAMPIONS_RUN_HELD_ITEM_CARRY_NONE, CHAMPIONS_RUN_HELD_ITEM_CARRY_ON_MON, or CHAMPIONS_RUN_HELD_ITEM_CARRY_TO_BAG."
#endif

// SaveBlock1 configs
#define FREE_EXTRA_SEEN_FLAGS_SAVEBLOCK1    FALSE   // Free up unused Pokédex seen flags (52 bytes).
#define FREE_TRAINER_HILL                   FALSE   // Frees up Trainer Hill data (28 bytes).
#define FREE_TRAINER_TOWER                  FALSE   // Frees up Trainer Tower data (x bytes).
#define FREE_MYSTERY_EVENT_BUFFERS          TRUE    // Frees up ramScript (1104 bytes).
#define FREE_MATCH_CALL                     FALSE   // Frees up match call and rematch / VS Seeker data. (104 bytes).
#define FREE_UNION_ROOM_CHAT                FALSE   // Frees up union room chat (212 bytes).
#define FREE_ENIGMA_BERRY                   FALSE   // Frees up E-Reader Enigma Berry data (52 bytes).
#define FREE_LINK_BATTLE_RECORDS            FALSE   // Frees up link battle record data (88 bytes).
#define FREE_MYSTERY_GIFT                   TRUE    // Frees up Mystery Gift data (876 bytes).
                                            // SaveBlock1 total: 2516 bytes

#if SAVE_CHAMPIONS_RUN_SESSION == TRUE && (FREE_MYSTERY_EVENT_BUFFERS == FALSE || FREE_MYSTERY_GIFT == FALSE)
#error "SAVE_CHAMPIONS_RUN_SESSION requires FREE_MYSTERY_EVENT_BUFFERS and FREE_MYSTERY_GIFT to fit the normal-state snapshot."
#endif
// SaveBlock2 configs
#define FREE_BATTLE_TOWER_E_READER          FALSE   // Frees up Battle Tower E-Reader data (188 bytes).
#define FREE_POKEMON_JUMP                   FALSE   // Frees up Pokémon Jump data (16 bytes).
#define FREE_RECORD_MIXING_HALL_RECORDS     FALSE   // Frees up hall records for record mixing (1032 bytes).
#define FREE_EXTRA_SEEN_FLAGS_SAVEBLOCK2    FALSE   // Free up unused Pokédex seen flags (108 bytes).
                                            // SaveBlock2 total: 1274 bytes

                                            // Grand Total: 3790

#endif // GUARD_CONFIG_SAVE_H
