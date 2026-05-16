# TM Shop Migration Investigation v15

調査日: 2026-05-02
再確認日: 2026-05-16

この文書は、もともと TM をマート販売へ寄せる前提で始めた調査を、legacy Gen 3
TM acquisition retirement へ切り替えたもの。既存の TM 定義、取得元、flag / var /
`.inc` の影響範囲を整理する。現時点では実装・削除は行っていない。

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

## 2026-05-16 Revised Scope

ユーザー方針の更新により、この feature の主目的は「current 50 TM を shop 化する」
ことではなく、「legacy Gen 3 TM acquisition を退役させる」ことに変わった。

- Current 50 TM は古い Gen 3 machine set として扱う。
- NPC/gym gift、visible item ball、hidden item、shop、facility reward など、
  player が通常プレイ中に current 50 TM を入手する導線を撤去する。
- TM received / got / field item / hidden item flags は、参照を外した後に
  `FLAG_UNUSED_0x...` 相当へ戻す方針。ただし flag value の再番号付けや別用途への
  即時再利用はしない。
- `FOREACH_TM`、`ITEM_TM*`、TM/HM pocket、teachability、item use は first cut では
  残す。ここを消すと item / move / bag / relearner / tests に広く波及する。
- 将来 TM を復活させる場合は、Gen 5 以降の reusable 仕様で Gen 9 準拠の
  200-250 TM 程度を新規 feature として設計する。
- Gen 9 TM と TR / tutor / tower move の重複は、復活 feature 側で separate source
  表示・dedupe・priority を決める。
- Unified Move Relearner の virtual TM candidate pool は、物理 TM item を増やさずに
  broad TM compatibility を扱うための既存方針として維持する。

### Current code facts from recheck

| Area | Confirmed behavior |
|---|---|
| Physical TM registry | `include/constants/tms_hms.h` の `FOREACH_TM` は current 50 entries。`FOREACH_HM` は 8 entries。 |
| Reserved TM item IDs | `include/constants/items.h` は `ITEM_TM01`..`ITEM_TM100` を予約している。 |
| Placeholder TM items | `src/data/items.h` の `ITEM_TM51`..`ITEM_TM100` は name / price / pocket / field use を持つが、description は `sQuestionMarksDesc`、move mapping は `FOREACH_TM` に無いため実運用未接続。 |
| TM reuse switch | `include/config/item.h` の `I_REUSABLE_TMS` は `TRUE`。既存 physical TM を持つ場合は無制限使用扱い。 |
| TM item importance | Current TM item entries は `.importance = I_REUSABLE_TMS`。`TRUE` にすると shop では所持済みが `Sold out` になり、teach 後に bag から消費されず、toss / sell / Fling も important item 扱いになる。 |
| HM item importance | HM entries は `.importance = 1`、`.price = 0`。shop list にそのまま入れると所持前は 0 円で購入でき、所持後は `Sold out` になる。 |
| Shop list behavior | `src/shop.c` の `SetShopItemsForSale()` は `ITEM_NONE` / `pokemartlistend` まで読む。退役対象の existing TM shops はこの path を使う。 |
| Shop quantity behavior | Important item は数量選択なしで 1 個購入。Non-important TM/HM pocket item は move name を出して数量選択に入る。 |
| Teach consumption | `src/party_menu.c` の `Task_LearnedMove()` は `!GetItemImportance(item)` の時だけ TM/HM item を消費する。 |
| Move Relearner consumption | Standard TM relearner path は `!I_REUSABLE_TMS` の時、bag 所持 TM を消費する path がある。Unified Move Relearner の broad TM pool は virtual candidate pool として別扱い。 |
| Debug Script 1 hook | `src/debug.c` の debug script menu は `Debug_EventScript_Script_1` を `"Script 1"` として実行する。`data/scripts/debug.inc` の `Debug_EventScript_Script_1` は現在 `release` / `end` の空 script。 |

### Acquisition source inventory from recheck

| Category | Current source count / examples |
|---|---|
| NPC / gym `giveitem ITEM_TM_*` refs | 34 direct `giveitem` lines were found. Some gym scripts have duplicate post-battle / rematch-style branches for the same TM. |
| Game Corner TM prize script | 5 TM prize entries use `bufferitemname` / `setvar` / `checkitemspace` / `additem`: Double Team, Psychic, Flamethrower, Thunderbolt, Ice Beam. |
| Visible TM item balls | 14 `map.json` object hide flags: Route 111 Sandstorm, Route 115 Focus Punch, Meteor Falls / Mt. Pyre / Scorched Slab / Victory Road / Seafloor Cavern / Fiery Path / Safari Zone / Abandoned Ship / Shoal Cave examples. |
| Hidden TM item | 1 hidden item: Route 113 Double Team. |
| Existing money shops | Lilycove Department Store sells 8 TMs; Slateport Power TMs sells Hidden Power and Secret Power. |
| Facility / C data rewards | Trainer Hill prize arrays contain TM rewards; Lilycove Lady quiz prize is `ITEM_TM_HYPER_BEAM`. |

### Practical interpretation

The first implementation should remove or rewrite acquisition *references* before
renaming flags to unused:

- Keep `FLAG_RECEIVED_TM_*`, `FLAG_ITEM_*_TM_*`, and
  `FLAG_HIDDEN_ITEM_*_TM_*` constants while references still exist.
- After script / map references are gone, the desired final state is to rename
  Emerald-side legacy TM flags back to `FLAG_UNUSED_0x...` names using the same
  values. Do not renumber the flag values.
- Do not reuse those values for new gameplay flags in the same change. Existing
  saves may already have those bits set.
- Edit map owners (`map.json`) rather than generated `events.inc`.
- Edit reward scripts and text together. Removing only `giveitem` can leave
  player-facing text claiming a TM was given.
- Decide whether each old source becomes no reward, a non-TM replacement item,
  a virtual TM unlock placeholder, or a future random-item hook.

## Legacy TM Retirement Dependency Map

| Dependency | Current owners | Retirement action |
|---|---|---|
| Received / got flags | 21 `FLAG_RECEIVED_TM_*` constants plus `FLAG_GOT_TM_THUNDERBOLT_FROM_WATTSON` in `include/constants/flags.h`; FRLG aliases are already `0`. | Remove all script reads / writes first. Then rename Emerald constants to `FLAG_UNUSED_0x...` names with identical values. |
| NPC / gym scripts | `data/maps/**/scripts.inc` and `data/scripts/secret_power_tm.inc` contain direct `giveitem ITEM_TM_*` and matching text / flag gates. | Replace the reward flow and text. Do not leave a no-op branch that still says the player received a TM. |
| Visible TM item balls | 14 `map.json` object entries with `trainer_sight_or_berry_tree_id = ITEM_TM_*` and `FLAG_ITEM_*_TM_*`. | Edit `map.json`: remove object, replace item, or route to a future random-item hook. Generated `events.inc` should not be edited directly. |
| Hidden TM item | Route 113 hidden Double Team uses `ITEM_TM_DOUBLE_TEAM` and `FLAG_HIDDEN_ITEM_ROUTE_113_TM_DOUBLE_TEAM`. | Edit the hidden item entry in `data/maps/Route113/map.json`; retire the flag after the map reference is gone. |
| Existing TM shops | Lilycove Department Store 4F, Slateport Power TMs, FRLG Celadon TM shop references. | Remove TM stock or replace shop identity. If shop text mentions TMs, update it with the same change. |
| Game Corner prizes | Mauville Game Corner uses `bufferitemname`, `setvar VAR_0x8004`, `checkitemspace`, and `additem` for 5 TMs. | Replace prize table and purchase branches; this is not controlled by `FLAG_RECEIVED_TM_*`. |
| Facility / C rewards | `src/trainer_hill.c` prize arrays and `src/data/lilycove_lady.h` quiz prize contain TM rewards. | Replace with non-TM rewards or disable reward entries. Build required because these are C data. |
| HM gift scripts | Emerald normal-progression HM item grants share TM/HM pocket and field move assumptions. | Remove `ITEM_HM_*` grants from Emerald scripts, but keep required story flags until field-move unlock logic owns them. |
| Core TM/HM item machinery | `FOREACH_TM/HM`, `gTMHMItemMoveIds`, `ItemUseOutOfBattle_TMHM`, `ItemUseCB_TMHM`, `BAG_TMHM_COUNT`, item icons, item menu. | Leave intact until a separate physical-TM/HM removal feature is explicitly scoped. |
| Unified Move Relearner | Virtual TM pool handles broad TM compatibility without physical item ownership. | Keep physical TM retirement separate from virtual relearner data. Do not add 200 physical TMs just to support relearner candidates. |

Scope note after implementation: the 2026-05-16 source slice covers Emerald
normal progression. FRLG-specific sources that use `ITEM_TM03` / `ITEM_HM01`
style constants and `FLAG_GOT_TM*` / `FLAG_GOT_HM*` flags remain visible in
global searches and should be handled as a separate follow-up if FRLG maps are
in scope.

## Retirement Category Policy

| Category | Policy |
|---|---|
| Gym leader / post-battle rewards | Remove `giveitem ITEM_TM_*`, matching `setflag FLAG_RECEIVED_TM_*`, and the reward branch text that says a TM was given. Badge / story flags and rematch flow must remain intact. If a leader still needs post-battle dialogue, keep a plain conversation branch. |
| Normal NPC gifts | Remove the TM reward and received-flag gate, but avoid leaving a silent NPC. Replace with normal dialogue, a non-TM reward, a future virtual-unlock hint, or no special reward depending on the source. |
| Explanation text after TM rewards | Delete or rewrite text that only explained the removed TM. Do not leave text promising a TM, describing a TM number, or teaching the player how to use a TM item. |
| Visible TM item balls | Remove the `map.json` object entirely, replace it with a non-TM item, or convert it later to a random-item hook. Removing the object is acceptable if the map does not need the pickup visual. |
| Hidden TM items | Remove or replace the `bg_hidden_item_event` in the owning `map.json`. Do not edit generated `events.inc` directly. |
| Existing TM shops / prizes | Remove TM stock / prize entries from the current 50-TM shops, Game Corner, Trainer Hill, Lilycove Lady, and FRLG references. These are not solved by `FLAG_RECEIVED_TM_*` cleanup. |
| HM scripts | Remove Emerald HM item grants from normal progression, but do not remove core HM move definitions or field-move checks in this slice. Old HM receive flags are renamed to `FLAG_UNUSED_0x...`; required story branches use existing story vars / flags. |

## Optional Debug Script 1 TM Shop

User-approved test hook: a temporary TM shop may be installed under Debug menu
`Scripts... > Script 1` for local validation. This route is debug-only and must
not be treated as a normal progression source.

Current local facts:

| File | Fact |
|---|---|
| `src/debug.c` | `sDebugMenu_Actions_Scripts[]` maps `"Script 1"` to `Debug_EventScript_Script_1`. The label can be renamed, for example to `"TM Shop Test"`, if the route is implemented. |
| `data/scripts/debug.inc` | `Debug_EventScript_Script_1` is currently an empty `release` / `end` script. |
| `asm/macros/event.inc` | `pokemart products:req` takes a pointer to products, and product lists are documented as `.2byte item values` preceded by `.align 2`; `pokemartlistend` writes `.2byte ITEM_NONE`. |
| `src/scrcmd.c` / `include/shop.h` / `src/shop.c` | `ScrCmd_pokemart` passes the pointer to `CreatePokemartMenu(const u16 *itemsForSale)`. `struct MartInfo` stores `const u16 *itemList` and `u16 itemCount`. |

Implementation rule if this hook is used:

- Use the current `pokemart` macro with an aligned `.2byte ITEM_*` list and
  `pokemartlistend`.
- Do not use older narrow item-id give/list patterns for the debug TM shop.
- Keep the stock small and representative unless a later validation specifically
  needs a larger list.
- Do not set TM received flags from the debug shop. It exists only to confirm
  modern shop behavior and item-id width.

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

1. 既存取得元を「撤去」「別 item に置換」「virtual unlock placeholder」「future random hook」「NPC 会話だけ残す」に分類する。
2. `FLAG_RECEIVED_TM_*` / `FLAG_GOT_TM_*` を読む script と書く script を source ごとに消す。
3. visible item ball は `map.json` から object を消すのか、item を置換するのか、hide flag は残すのかを決める。
4. hidden item は `bg_hidden_item_event` を消すのか、item を置換するのかを決める。
5. NPC/gym gift は `giveitem` と `FLAG_RECEIVED_TM_*` gate をどう扱うか決める。
6. Existing TM shops / prize tables / C data rewards を退役対象として個別に扱う。
7. 参照が消えた後、legacy TM flag names を `FLAG_UNUSED_0x...` へ戻す。

## Open Questions

- NPC/gym reward の代替報酬を何にするか未決定。
- visible item ball / hidden item を削除する場合、map の見た目として別 item に置換するか完全撤去するか未決定。
- Retired flag values をいつ新規用途へ戻せるかは未決定。save compatibility を考えると即時再利用は避ける方が安全。
