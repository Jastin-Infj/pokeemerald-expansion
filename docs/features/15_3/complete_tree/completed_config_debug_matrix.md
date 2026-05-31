# 1.15.3 Runtime Config / Debug Matrix

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-31 |
| Branch | `integration/runtime-dev-20260529` |
| Purpose | 1.15.3 runtime integration で追加・変更した config、runtime toggle、debug command の索引 |
| Related visual summary | [html/index.html](html/index.html) |

## Config Matrix

### Battle / Ability / Team Viewer

| Token | Value | Type | Owner | Notes |
|---|---:|---|---|---|
| `B_ALL_ABILITY_SLOTS` | `FALSE` | build-time default | All Ability Slots | 通常 ROM / full check は single representative ability を維持する。 |
| `B_ALL_ABILITY_SLOTS_RUNTIME_TOGGLE` | `TRUE` | build-time compile gate | All Ability Slots | Debug menu の `All Abilities` mode で per-save override できる。 |
| `B_ALL_ABILITY_SLOTS_MOLD_BREAKER` | `TRUE` | build-time balance knob | All Ability Slots | non-representative Mold Breaker-family ability も target ability bypass 可能にする。 |
| `B_ALL_ABILITY_SLOTS_NEUTRALIZING_GAS` | `TRUE` | build-time balance knob | All Ability Slots | non-representative Neutralizing Gas も ability suppression 可能にする。 |
| `B_RESTORE_HELD_BATTLE_BERRIES` | `TRUE` | build-time | Battle Item Restore | berry を battle 後に戻す。 |
| `B_TRAINER_BATTLE_AFTERCARE` | `TRUE` | build-time | Trainer Battle Aftercare | 通常 trainer win 後に party heal。Frontier / Pyramid / Trainer Hill / Secret Base / partner / forfeit は除外。 |
| `B_TRAINER_BATTLE_SELECTION` | `TRUE` | build-time | Battle Selection | 通常 trainer battle 前に選出 / viewer route を有効化。 |
| `B_TRAINER_BATTLE_SELECTION_ALLOW_SHORT_PARTY` | `TRUE` | build-time | Battle Selection | 通常 3/4 未満でも最低数を満たせば縮小選出を許可。 |
| `B_TRAINER_BATTLE_SELECTION_SHORT_PARTY_MIN_COUNT` | `1` | build-time | Battle Selection | Single は 1 匹から `1/1` selection が可能。 |
| `B_TRAINER_BATTLE_SELECTION_SHORT_DOUBLE_MIN_COUNT` | `2` | build-time | Battle Selection | Double は default 2 匹から。1v2 実験には `1` と `OW_DOUBLE_APPROACH_WITH_ONE_MON TRUE` が必要。 |
| `B_PREBATTLE_TEAM_VIEWER` | `TRUE` | build-time | Team Viewer | Battle 前の opponent preview / selection surface。 |
| `B_IN_BATTLE_TEAM_VIEWER` | `TRUE` | build-time | Team Viewer | Battle action menu から read-only viewer を開く。 |
| `B_TEAM_VIEWER_BUTTON` | `R_BUTTON` | build-time button | Team Viewer | In-battle viewer shortcut。 |
| `B_TEAM_VIEWER_DETAILS_BUTTON` | `SELECT_BUTTON` | build-time button | Team Viewer | Champions の Y 相当。details / Summary check に使う。 |
| `AI_FRAME_CEILING_SINGLES_SMART_TRAINER` | `10` | test threshold | AI / All Ability | all-slot helper 影響を受けた AI frame ceiling を更新。 |
| `AI_FRAME_CEILING_DOUBLES_NO_FLAGS` | `24` | test threshold | AI / All Ability | 同上。 |
| `AI_FRAME_CEILING_DOUBLES_SMART_TRAINER` | `44` | test threshold | AI / All Ability | 同上。 |
| `AI_FRAME_CEILING_STEVEN_MULTI` | `32` | test threshold | AI / All Ability | 同上。 |
| `AI_FRAME_CEILING_STEVEN_MULTI_SMART_TRAINER` | `36` | test threshold | AI / All Ability | 同上。 |

### Item / TM / Field Move / Encounter

| Token | Value | Type | Owner | Notes |
|---|---:|---|---|---|
| `I_HELD_ITEM_CATALOG_ASSIGNMENT` | `TRUE` | build-time | Nonconsumable Held Items | non-mail held-effect item は Bag 1 個を所有権 token として扱う。 |
| `I_REUSABLE_TMS` | `TRUE` | build-time | TM Shop Migration | TM を再使用可能にする。 |
| `OW_FIELD_MOVE_MODERNIZATION` | `TRUE` | build-time | Field Move Modernization | HM 技を覚えた Pokemon 不要の modern unlock flow。 |
| `OW_FIELD_MOVE_TOOLKIT_REQUIRED` | `TRUE` | build-time | Field Kit | Field Kit key item と capability flag を要求する。 |
| `OW_FIELD_MOVE_TOOLKIT_BADGES` | `TRUE` | build-time | Field Kit | 元の badge gate を pace control として残す。 |
| `OW_FIELD_MOVE_SHOW_MON_EFFECT` | `FALSE` | build-time | Field Move Modernization | Field move 使用時の Pokemon banner/cut-in を省略。 |
| `OW_FLAG_NO_ENCOUNTER` | `FLAG_NO_ENCOUNTER` | event flag id | No Random Encounters | Debug menu の `Encounter OFF` が実際に step encounter を止める。 |

### Pokemon / Summary / Move Relearner

| Token | Value | Type | Owner | Notes |
|---|---:|---|---|---|
| `P_EVOLUTIONS_ENABLED` | `FALSE` | build-time | Pokemon Vendor / game policy | 通常進化を全体停止。Mega など battle form-change は別扱い。 |
| `P_SHOW_TERA_TYPE` | `GEN_9` | build-time | Summary Tera Badge | Summary で Tera Type を表示する前提。 |
| `P_SUMMARY_SCREEN_ALL_ABILITY_SLOT_SWITCH` | `TRUE` | build-time | All Ability Slots | all-slot mode 時だけ Summary ability description を `L/R` で切替。 |
| `P_SUMMARY_TERA_TYPE_ICON_X` | `205` | layout | Summary Tera Badge | Tera badge X 座標。 |
| `P_SUMMARY_TERA_TYPE_ICON_Y` | `48` | layout | Summary Tera Badge | Tera badge Y 座標。 |
| `P_SUMMARY_SCREEN_STATE_EDITOR` | `TRUE` | build-time | Pokemon State Editor | Summary Skills page から State Editor を開く。 |
| `P_SUMMARY_STATE_EDITOR_LEVEL_EDIT` | `TRUE` | build-time | Pokemon State Editor | State Editor で level editing を許可。 |
| `P_SUMMARY_STATE_EDITOR_LEVEL_CAP` | `TRUE` | build-time | Pokemon State Editor | level edit は current cap を尊重する。 |
| `P_UNIFIED_MOVE_RELEARNER` | `TRUE` | build-time | Unified Move Relearner | Summary / common relearner entry を unified list にする。 |
| `P_UNIFIED_RELEARNER_LEVEL_MOVES` | `TRUE` | build-time | Unified Move Relearner | level-up candidates を含める。 |
| `P_UNIFIED_RELEARNER_EGG_MOVES` | `TRUE` | build-time | Unified Move Relearner | current / historical egg move candidates を含める。 |
| `P_UNIFIED_RELEARNER_TM_MOVES` | `TRUE` | build-time | Unified Move Relearner | historical TM/TR candidates を physical TM 所持と切り離して含める。 |
| `P_UNIFIED_RELEARNER_TUTOR_MOVES` | `TRUE` | build-time | Unified Move Relearner | tutor / tower candidates を含める。 |
| `P_UNIFIED_RELEARNER_SPECIAL_MOVES` | `TRUE` | build-time | Unified Move Relearner | event / distribution / XD candidates を含める。 |

### Champions Run / Save

| Token | Value | Type | Owner | Notes |
|---|---:|---|---|---|
| `SAVE_CHAMPIONS_RUN_SESSION` | `TRUE` | build-time save feature | Champions Run Session | `SaveBlock1.championsRun` を追加する。 |
| `CHAMPIONS_RUN_ENTRY_PARTY_MODE` | `CHAMPIONS_RUN_START_PARTY_EMPTY` | build-time mode | Champions Run Session | run 開始時は party 0 匹。 |
| `CHAMPIONS_RUN_CLEAR_DEPOSIT_PARTY` | `TRUE` | build-time | Champions Run Session | clear party を PC box に預ける。 |
| `CHAMPIONS_RUN_CLEAR_HELD_ITEM_MODE` | `CHAMPIONS_RUN_HELD_ITEM_CARRY_NONE` | build-time mode | Champions Run Session | clear party の held item は strip / carry なし。 |
| `CHAMPIONS_RUN_CLEAR_CARRY_ITEMS` | `TRUE` | build-time | Champions Run Session | Items pocket の run reward を通常 bag に merge。 |
| `CHAMPIONS_RUN_CLEAR_CARRY_POKE_BALLS` | `FALSE` | build-time | Champions Run Session | Poke Balls pocket は merge しない。 |
| `CHAMPIONS_RUN_CLEAR_CARRY_TMS_HMS` | `TRUE` | build-time | Champions Run Session | TM/HM pocket の run reward を merge。 |
| `CHAMPIONS_RUN_CLEAR_CARRY_BERRIES` | `FALSE` | build-time | Champions Run Session | Berries pocket は merge しない。 |
| `CHAMPIONS_RUN_CLEAR_CARRY_KEY_ITEMS` | `FALSE` | build-time | Champions Run Session | Key Items pocket は merge しない。 |
| `CHAMPIONS_RUN_CLEAR_AUTOSAVE` | `TRUE` | build-time | Champions Run Session | clear 後に normal save を書く。 |
| `FREE_MYSTERY_EVENT_BUFFERS` | `TRUE` | save space | Champions Run Session | run snapshot を収めるために free。 |
| `FREE_MYSTERY_GIFT` | `TRUE` | save space | Champions Run Session | run snapshot を収めるために free。 |

## Runtime Debug Toggles

| Menu path | Command | State changed | Notes |
|---|---|---|---|
| `Flags & Vars...` | `All Abilities: Default/OFF/ON` | save-backed all ability mode | `Default` は `B_ALL_ABILITY_SLOTS` に従う。`ON` / `OFF` は debug runtime override。 |
| `Flags & Vars...` | `Toggle Encounter OFF` | `FLAG_NO_ENCOUNTER` | No Random Encounters の runtime check。 |
| `Flags & Vars...` | `Toggle Bag Use OFF` | no-bag var | Champions / challenge rule validation の補助。 |
| `Flags & Vars...` | `Toggle Fly Flags` | visited / Fly flags | map / Fly validation の補助。 |

## Debug Command Matrix

### Party Menu

| Menu path | Command | Purpose | Expected feature |
|---|---|---|---|
| `Party...` | `Move Relearner` | Common relearner entry. Integrationでは unified route と共存。 | Unified Move Relearner |
| `Party...` | `Selection Battle` | Single trainer selection + Team Viewer debug route. | Battle Selection / Team Viewer |
| `Party...` | `Selection Double` | Double trainer selection + Team Viewer debug route. | Battle Selection / Team Viewer |
| `Party... > All Ability...` | `A Recoil Battle` | Magic Guard / Rock Head-like recoil behavior. | All Ability Slots |
| `Party... > All Ability...` | `B Flash Fire Battle` | Fire immunity / popup route. | All Ability Slots |
| `Party... > All Ability...` | `C Soundproof Battle` | Sound move immunity. | All Ability Slots |
| `Party... > All Ability...` | `D Drought Battle` | Weather popup name and ability-slot attribution. | All Ability Slots |
| `Party... > All Ability...` | `E Skill Swap Battle` | Slot-targeted ability replacement. | All Ability Slots |
| `Party... > All Ability...` | `F Intimidate Battle` | Popup / slot attribution. | All Ability Slots |
| `Party... > All Ability...` | `G Levitate Battle` | Ground immunity. | All Ability Slots |
| `Party... > All Ability...` | `H Storm Drain Battle` | Redirect / immunity. | All Ability Slots |
| `Party... > All Ability...` | `I Sticky Hold Battle` | Item removal protection. | All Ability Slots |
| `Party... > All Ability...` | `J Majesty Battle` | Priority-block style behavior. | All Ability Slots |
| `Party... > All Ability...` | `K Commander Battle` | Tatsugiri / Commander edge route. | All Ability Slots |
| `Party... > All Ability...` | `L Chlorophyll Battle` | Weather speed modifier. | All Ability Slots |
| `Party... > All Ability...` | `M Trace Battle` | Ability copy behavior. | All Ability Slots |
| `Party... > All Ability...` | `N Power Stack` | Multiple offensive modifier stack. | All Ability Slots |
| `Party... > All Ability...` | `O Hustle Battle` | Power / accuracy modifier. | All Ability Slots |
| `Party... > All Ability...` | `P Rod Redirect` | Lightning Rod-style redirect. | All Ability Slots |
| `Party... > All Ability...` | `Q Drain Redirect` | Storm Drain-style redirect. | All Ability Slots |
| `Party... > All Ability...` | `R Mod Stack` | Multiple modifier stack. | All Ability Slots |
| `Party... > All Ability...` | `S Guard Mods` | Guard / defensive modifier interactions. | All Ability Slots |
| `Party... > All Ability...` | `T Partner Mods` | Partner ability modifier interactions. | All Ability Slots |
| `Party... > All Ability...` | `U Moxie Popup` | Post-KO popup / ability name route. | All Ability Slots |

### Script Menu

| Menu path | Command | Purpose | Expected feature |
|---|---|---|---|
| `Scripts...` | `Scout Selection` | `SCOUT_POOL_PARTYGEN_DEMO`, 12 candidate, 1 pick. | Scout Selection |
| `Scripts...` | `Scout Pick 6` | Same pool, 6 picks. | Scout Selection |
| `Scripts...` | `Pokemon Vendor` | Opens 10-row Pokemon Vendor demo. | Friendly Shop Pokemon Vendor |
| `Scripts...` | `Vendor Reward` | Starts trainer battle and queues 20 sealed bond EXP in win text. | Pokemon Vendor |
| `Scripts...` | `TM Shop Test` | Opens a TM shop with Thunderbolt / Ice Beam / Flamethrower / Earthquake / Secret Power. | TM Shop Migration |
| `Scripts...` | `Champs: Start` | Start Champions run; clears party / bag / money. | Champions Run Session |
| `Scripts...` | `Champs: Give Mon` | Adds debug Dragonite and checkpoints. | Champions Run Session |
| `Scripts...` | `Champs: Checkpoint` | Writes temporary report / checkpoint. | Champions Run Session |
| `Scripts...` | `Champs: Retire` | Restores normal state and reloads map. | Champions Run Session |
| `Scripts...` | `Champs: Lose Test` | Prepares loss route against Steven. | Champions Run Session |
| `Scripts...` | `Champs: Clear` | Deposits clear party and performs clear restore / carryover. | Champions Run Session |
| `Scripts...` | `Field Kit Full` | Gives Field Kit, HM capability flags, and all badges. | Field Move Modernization |
| `Scripts...` | `Field Kit Item` | Gives Field Kit only. | Field Move Modernization |
| `Scripts...` | `Field Kit Flags` | Sets HM capability flags only. | Field Move Modernization |
| `Scripts...` | `Field Kit Clear` | Removes Field Kit and clears HM capability flags. | Field Move Modernization |
| `Scripts...` | `Script 4` to `Script 8` | Empty fallback slots. | Reserved |

### Sound Menu

| Menu path | Command | Purpose | Expected feature |
|---|---|---|---|
| `Sound...` | `Trainer BGM...` | Select / preview trainer battle BGM. | Battle BGM Selector |
| `Sound...` | `Wild BGM...` | Select / preview wild battle BGM. | Battle BGM Selector |
| `Sound...` | `Music...` / `SFX...` | Existing sound debug route; useful for imported asset checks. | Sound tooling |

### ROM Info / Utility

| Menu path | Command | Purpose |
|---|---|---|
| `ROM Info...` | `Save Block space` | Shows SaveBlock1 / 2 / 3 and Pokemon Storage size. |
| `ROM Info...` | `ROM space` | Shows current ROM size and free space. |
| `PC/Bag...` | `Access PC` | Important for Pokemon Vendor sealed recruit and Champions PC block checks. |

## Tooling Commands / Scripts

| Tool | Path | Purpose |
|---|---|---|
| Map relinker CUI | `tools/map_asset_relinker/map_relink.sh` | Python-compatible map / layout / mapsec relink operation. |
| Map relinker Rust core | `tools/map_asset_relinker_core/` | Fast scanner / dry-run / apply core for many map files. |
| Map relinker GUI | `tools/map_asset_relinker_gui/` | Tauri desktop GUI that calls the same core behavior. |
| Map relinker release workflow | `.github/workflows/map-asset-relinker-desktop.yml` | Builds Windows portable zip and uploads Release asset on `map-asset-relinker-v*` tag. |
| Trainer partygen | `tools/champions_partygen/partygen.sh` / `.cmd` | Catalog / generated trainer party pool support for Champions / Scout tuning. |
| Scout pool generator | `tools/scout_selection/make_scout_pools.py` | Generates scout pool data from partygen JSON, with species de-duplication. |
| Unified relearner generator | `tools/learnset_helpers/make_relearner_learnsets.py` | Generates all-source relearner candidate header. |
| AIF tooling | `tools/aif2pcm/` | Imported battle BGM sample conversion; odd-length compressed sample handling fixed. |
| MIDI tooling | `tools/mid2agb/` | Imported battle BGM MIDI flow; `-Q` handling adjusted in integration. |

## Validation Pointers

| Area | Owning evidence |
|---|---|
| Runtime integration queue | [completed_runtime_integration_checklist_ja_2026_05_30.md](completed_runtime_integration_checklist_ja_2026_05_30.md) |
| Full branch audit | [completed_branch_audit_2026_05_29.md](completed_branch_audit_2026_05_29.md) |
| Config / flag ledger | [local_config_and_flag_ledger.md](../../../manuals/local_config_and_flag_ledger.md) |
| Validation evidence matrix | [validation_evidence_matrix.md](../../../manuals/validation_evidence_matrix.md) |
