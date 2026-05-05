# TM Shop Migration Investigation v15

調査日: 2026-05-02

この文書は、TM をマート販売へ寄せる前提で、既存の TM 定義、取得元、flag / var / `.inc` の影響範囲を整理する。現時点では実装・削除は行っていない。

## Important Correction

ユーザー要望では「技マシンの取得フラグが 50 個」とあったが、今回確認した範囲では次の通り。

| Category | Confirmed count / facts |
|---|---|
| TM move list | `include/constants/tms_hms.h` の `FOREACH_TM(F)` は 50 entries。 |
| HM move list | `FOREACH_HM(F)` は 8 entries。 |
| `FLAG_RECEIVED_TM_*` | `include/constants/flags.h` では 21 definitions。 |
| visible TM item ball flags | `FLAG_ITEM_*_TM_*` は 14 definitions。 |
| hidden TM item flags | `FLAG_HIDDEN_ITEM_*_TM_*` は 1 definition。 |
| TM item constants | `include/constants/items.h` では `ITEM_TM01`..`ITEM_TM100` が予約され、`FOREACH_TM` から symbolic aliases が作られる。 |

結論: 消したい対象は「50 個の TM 定義」ではなく、既存 TM の取得元である可能性が高い。`FOREACH_TM` や item constants を削ると TM/HM pocket、item menu、teachability、move relearner、tests へ広く波及する。

## TM Core Data

| File | Confirmed symbols / behavior |
|---|---|
| `include/constants/tms_hms.h` | `FOREACH_TM(F)`、`FOREACH_HM(F)`、`FOREACH_TMHM(F)`。 |
| `include/constants/items.h` | `ITEM_TM01 = 582`、`ITEM_TM50 = 631`、`ITEM_TM100 = 681`、`ITEM_HM01 = 682` を確認。 |
| `include/item.h` | `enum TMHMIndex`、`NUM_TECHNICAL_MACHINES`、`NUM_HIDDEN_MACHINES`、`GetItemTMHMIndex`、`GetItemTMHMMoveId`、`GetTMHMItemIdFromMoveId`。 |
| `src/item.c` | `gTMHMItemMoveIds` を `FOREACH_TM` / `FOREACH_HM` から作る。 |
| `src/data/items.h` | `ITEM_TM_*` の item database。`fieldUseFunc = ItemUseOutOfBattle_TMHM`。 |
| `src/item_use.c` | `ItemUseOutOfBattle_TMHM`、`UseTMHM`。 |
| `src/item_menu.c` | TM/HM 表示。`NUM_TECHNICAL_MACHINES >= 100` で番号桁数を変える処理を確認。 |
| `src/party_menu.c` | `ItemUseCB_TMHM`、`CanTeachMove`、`GiveMoveToMon`、field move action。 |
| `include/constants/global.h` | `BAG_TMHM_COUNT 64`。 |
| `test/bag.c` | TM/HM pocket behavior test。 |

## Received TM Flags

`include/constants/flags.h` で確認した `FLAG_RECEIVED_TM_*` は 21 件。

| Flag | Value | Example script refs |
|---|---:|---|
| `FLAG_RECEIVED_TM_BRICK_BREAK` | `0x79` | `data/maps/SootopolisCity_House1/scripts.inc` |
| `FLAG_RECEIVED_TM_ROCK_TOMB` | `0xA5` | `data/maps/RustboroCity_Gym/scripts.inc` |
| `FLAG_RECEIVED_TM_BULK_UP` | `0xA6` | `data/maps/DewfordTown_Gym/scripts.inc` |
| `FLAG_RECEIVED_TM_SHOCK_WAVE` | `0xA7` | `data/maps/MauvilleCity_Gym/scripts.inc` |
| `FLAG_RECEIVED_TM_OVERHEAT` | `0xA8` | `data/maps/LavaridgeTown_Gym_1F/scripts.inc` |
| `FLAG_RECEIVED_TM_FACADE` | `0xA9` | `data/maps/PetalburgCity_Gym/scripts.inc` |
| `FLAG_RECEIVED_TM_AERIAL_ACE` | `0xAA` | `data/maps/FortreeCity_Gym/scripts.inc` |
| `FLAG_RECEIVED_TM_CALM_MIND` | `0xAB` | `data/maps/MossdeepCity_Gym/scripts.inc` |
| `FLAG_RECEIVED_TM_WATER_PULSE` | `0xAC` | `data/maps/SootopolisCity_Gym_1F/scripts.inc` |
| `FLAG_RECEIVED_TM_RETURN` | `0xE5` | `data/maps/FallarborTown_CozmosHouse/scripts.inc` |
| `FLAG_RECEIVED_TM_SLUDGE_BOMB` | `0xE6` | `data/maps/DewfordTown_Hall/scripts.inc` |
| `FLAG_RECEIVED_TM_ROAR` | `0xE7` | `data/maps/Route114/scripts.inc` |
| `FLAG_RECEIVED_TM_GIGA_DRAIN` | `0xE8` | `data/maps/Route123/scripts.inc` |
| `FLAG_RECEIVED_TM_REST` | `0xEA` | `data/maps/LilycoveCity_House2/scripts.inc` |
| `FLAG_RECEIVED_TM_ATTRACT` | `0xEB` | `data/maps/VerdanturfTown_BattleTentLobby/scripts.inc` |
| `FLAG_RECEIVED_TM_SNATCH` | `0x104` | `data/maps/SSTidalRooms/scripts.inc` |
| `FLAG_RECEIVED_TM_DIG` | `0x105` | `data/maps/Route114_FossilManiacsHouse/scripts.inc` |
| `FLAG_RECEIVED_TM_BULLET_SEED` | `0x106` | `data/maps/Route104/scripts.inc` |
| `FLAG_RECEIVED_TM_HIDDEN_POWER` | `0x108` | `data/maps/FortreeCity_House2/scripts.inc` |
| `FLAG_RECEIVED_TM_TORMENT` | `0x109` | `data/maps/SlateportCity_BattleTentLobby/scripts.inc` |
| `FLAG_RECEIVED_TM_THIEF` | `0x10D` | `data/maps/SlateportCity_OceanicMuseum_1F/scripts.inc` |

`include/constants/flags_frlg.h` では対応する `FLAG_RECEIVED_TM_*` が `0` に定義されている箇所を確認した。Emerald / FRLG の切替に関わるため、flag 削除や番号詰めは高リスク。

## Visible TM Item Ball Flags

`include/constants/flags.h` で確認した visible item ball の TM flag は 14 件。

| Flag | Example data owner |
|---|---|
| `FLAG_ITEM_ROUTE_111_TM_SANDSTORM` | `data/maps/Route111/map.json` |
| `FLAG_ITEM_ROUTE_115_TM_FOCUS_PUNCH` | `data/maps/Route115/map.json` |
| `FLAG_ITEM_METEOR_FALLS_1F_1R_TM_IRON_TAIL` | `data/maps/MeteorFalls_1F_1R/map.json` |
| `FLAG_ITEM_MT_PYRE_EXTERIOR_TM_SKILL_SWAP` | `data/maps/MtPyre_Exterior/map.json` |
| `FLAG_ITEM_SCORCHED_SLAB_TM_SUNNY_DAY` | `data/maps/ScorchedSlab/map.json` |
| `FLAG_ITEM_METEOR_FALLS_B1F_2R_TM_DRAGON_CLAW` | `data/maps/MeteorFalls_B1F_2R/map.json` |
| `FLAG_ITEM_VICTORY_ROAD_B1F_TM_PSYCHIC` | `data/maps/VictoryRoad_B1F/map.json` |
| `FLAG_ITEM_MT_PYRE_6F_TM_SHADOW_BALL` | `data/maps/MtPyre_6F/map.json` |
| `FLAG_ITEM_SEAFLOOR_CAVERN_ROOM_9_TM_EARTHQUAKE` | `data/maps/SeafloorCavern_Room9/map.json` |
| `FLAG_ITEM_FIERY_PATH_TM_TOXIC` | `data/maps/FieryPath/map.json` |
| `FLAG_ITEM_SAFARI_ZONE_NORTH_WEST_TM_SOLAR_BEAM` | `data/maps/SafariZone_Northwest/map.json` |
| `FLAG_ITEM_ABANDONED_SHIP_ROOMS_B1F_TM_ICE_BEAM` | `data/maps/AbandonedShip_Room_B1F/map.json` |
| `FLAG_ITEM_ABANDONED_SHIP_HIDDEN_FLOOR_ROOM_1_TM_RAIN_DANCE` | `data/maps/AbandonedShip_HiddenFloorRooms/map.json` |
| `FLAG_ITEM_SHOAL_CAVE_ICE_ROOM_TM_HAIL` | `data/maps/ShoalCave_LowTideIceRoom/map.json` |

visible item ball flow は `docs/flows/map_script_flag_var_flow_v15.md` に詳細を記録した。重要点は、item id が `object_event` の `trainerRange_berryTreeId` に入り、取得後は object の hide flag が set されること。

## Hidden TM Item Flags

hidden TM flag は今回確認範囲で 1 件。

| Flag | Data owner |
|---|---|
| `FLAG_HIDDEN_ITEM_ROUTE_113_TM_DOUBLE_TEAM` | `data/maps/Route113/map.json`。generated `events.inc` に出力される。 |

hidden item flow は `src/field_control_avatar.c`、`data/scripts/obtain_item.inc`、`src/field_specials.c` の `SetHiddenItemFlag()` に分かれている。

## Other TM Sources

`FLAG_RECEIVED_TM_*` 以外にも TM を渡す箇所がある。

| Source | File / symbols | Notes |
|---|---|---|
| Mauville Wattson gift | `data/maps/MauvilleCity/scripts.inc`、`ITEM_TM_THUNDERBOLT`、`FLAG_GOT_TM_THUNDERBOLT_FROM_WATTSON` | `FLAG_RECEIVED_TM_THUNDERBOLT` ではない。 |
| Steven gift | `data/maps/GraniteCave_StevensRoom/scripts.inc`、`ITEM_TM_STEEL_WING` | 一回限り入手だが received flag 名ではない。 |
| Trick House | `data/maps/Route110_TrickHouseEntrance/scripts.inc`、`data/maps/Route110_TrickHouseEnd/scripts.inc`、`ITEM_TM_TAUNT` | Trick House 報酬系。 |
| Pacifidlog Town | `data/maps/PacifidlogTown_House2/scripts.inc` | Return / Frustration 系。day-based 変数の追加確認が必要。 |
| Secret Power | `data/scripts/secret_power_tm.inc`、`ITEM_TM_SECRET_POWER` | 共通 script。 |
| Lilycove Department Store | `data/maps/LilycoveCity_DepartmentStore_4F/scripts.inc` | `ITEM_TM_FIRE_BLAST`、`ITEM_TM_THUNDER`、`ITEM_TM_BLIZZARD`、`ITEM_TM_HYPER_BEAM`、`ITEM_TM_PROTECT`、`ITEM_TM_SAFEGUARD`、`ITEM_TM_REFLECT`、`ITEM_TM_LIGHT_SCREEN` など既存販売。 |
| Mauville Game Corner | `data/maps/MauvilleCity_GameCorner/scripts.inc` | coins 交換で TM。 |
| Slateport Power TMs | `data/maps/SlateportCity/scripts.inc` | `SlateportCity_Pokemart_PowerTMs` が `ITEM_TM_HIDDEN_POWER`、`ITEM_TM_SECRET_POWER` を販売。 |
| Trainer Hill prizes | `src/trainer_hill.c` | `sPrizeList*` に複数 TM。 |
| Lilycove Lady quiz | `src/data/lilycove_lady.h` | quiz prize に `ITEM_TM_HYPER_BEAM`。 |

## Shop Flow

| File | Confirmed behavior |
|---|---|
| `asm/macros/event.inc` | `pokemart products:req` と `pokemartlistend` macro。 |
| `data/script_cmd_table.inc` | `SCR_OP_POKEMART` が `ScrCmd_pokemart` へ dispatch。 |
| `src/scrcmd.c` | `ScrCmd_pokemart` は script pointer を読み、`CreatePokemartMenu(ptr)` を呼び、`ScriptContext_Stop()` する。 |
| `src/shop.c` | `CreateShopMenu`、`SetShopItemsForSale`、`Task_ShopMenu`、`BuyMenuBuildListMenuTemplate`。 |
| `data/maps/SlateportCity/scripts.inc` | `SlateportCity_Pokemart_PowerTMs` の TM shop list 例。 |

`src/shop.c` の `SetShopItemsForSale()` は item list を sentinel まで読む。`pokemartlistend` は list 終端として使われる。

## What Not To Remove First

| Do not remove yet | Reason |
|---|---|
| `FOREACH_TM` entries | TM item to move mapping、TM/HM numbering、teachability、item display の根幹。 |
| `ITEM_TM01`..`ITEM_TM100` constants | item id range と TM alias generation に関係。 |
| `BAG_TMHM_COUNT` | save block の bag pocket layout に関係。 |
| `GetItemTMHMIndex` / `GetItemTMHMMoveId` 系 | item menu、party menu、move relearner が利用。 |
| `FLAG_RECEIVED_TM_*` constants | save flag id / FRLG compatibility / upstream diff の追跡に影響。最初は script 取得元を止める方が安全。 |

## Recommended Investigation Order Before Implementation

1. 全 TM の最終販売リストを作る。
2. 既存取得元を「残す」「削除」「別 item に置換」「NPC 会話だけ残す」に分類する。
3. visible item ball は `map.json` から object を消すのか、item を置換するのか、hide flag は残すのかを決める。
4. hidden item は `bg_hidden_item_event` を消すのか、item を置換するのかを決める。
5. NPC/gym gift は `giveitem` と `FLAG_RECEIVED_TM_*` gate をどう扱うか決める。
6. shop list は既存 `pokemart` で足りるか、将来の Medley Shop 用 provider が必要かを決める。

## Open Questions

- “Medley Shop” 固有の既存 symbol は `src/`、`include/`、`data/` で未検出。新規独自 shop として追加予定なのか、既存 `pokemart` の呼称なのか未確認。
- 全 50 TM を 1 shop に並べるか、進行度や badge で段階解放するか未決定。
- NPC/gym reward の代替報酬を何にするか未決定。
- visible item ball / hidden item を削除する場合、map の見た目として別 item に置換するか完全撤去するか未決定。
- `FLAG_RECEIVED_TM_*` を将来の別用途へ再利用するかは未決定。save compatibility を考えると再利用は避ける方が安全と思われるが未設計。
