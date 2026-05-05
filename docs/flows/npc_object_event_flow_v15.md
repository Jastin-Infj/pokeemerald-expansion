# NPC / Object Event Flow v15

## Purpose

NPC の配置、通常歩行、script からの一時移動、非表示 flag、条件付き map layout / metatile 変更を、`.inc` 側から追うための調査メモ。

調査日: 2026-05-04。source 改造はしておらず、`docs/` への記録のみ。

## Key Files

| File | Role |
|---|---|
| [data/maps/MapName/events.inc](../../data/maps/) | `object_event` / `warp_def` / `coord_event` / `bg_*_event` の生成物。`map.json` から生成されるため直接編集しない。 |
| [data/maps/MapName/scripts.inc](../../data/maps/) | NPC 会話、cutscene、`applymovement`、`setobjectxyperm`、`setmetatile` などの手書き script。 |
| [asm/macros/map.inc](../../asm/macros/map.inc) | `object_event`、`map_script`、`map_script_2` など map data macro。 |
| [asm/macros/event.inc](../../asm/macros/event.inc) | `applymovement`、`waitmovement`、`setobjectxyperm`、`setobjectmovementtype`、`setmetatile`、`setmaplayoutindex` など script macro。 |
| [include/constants/event_objects.h](../../include/constants/event_objects.h) | `OBJ_EVENT_GFX_*` と `MOVEMENT_TYPE_*` 定数。 |
| [src/event_object_movement.c](../../src/event_object_movement.c) | `MOVEMENT_TYPE_*` を runtime callback に接続する本体。 |
| [src/scrcmd.c](../../src/scrcmd.c) | script opcode 実装。`ScrCmd_setmetatile` / `ScrCmd_setmaplayoutindex` など。 |
| [src/field_camera.c](../../src/field_camera.c) | `DrawWholeMapView` / `CurrentMapDrawMetatileAt`。metatile 変更後の画面反映。 |

## Static NPC Definition

`events.inc` の `object_event` は、Porymap / `map.json` から生成される配置データ。

例: [BattleFrontier_ReceptionGate/events.inc](../../data/maps/BattleFrontier_ReceptionGate/events.inc)

```asm
object_event 1, OBJ_EVENT_GFX_TEALA, 0, 11, 0, MOVEMENT_TYPE_FACE_RIGHT, 1, 1, TRAINER_TYPE_NONE, 0, BattleFrontier_ReceptionGate_EventScript_Greeter, 0
object_event 4, OBJ_EVENT_GFX_SCOTT, 4, 5, 3, MOVEMENT_TYPE_FACE_DOWN, 1, 1, TRAINER_TYPE_NONE, 0, 0x0, FLAG_HIDE_BATTLE_FRONTIER_RECEPTION_GATE_SCOTT
```

重要な列:

| Column | Meaning |
|---|---|
| local id | map 内で script が参照する NPC id。`LOCALID_*` 定数がある map もある。 |
| gfx | overworld sprite graphics。`OBJ_EVENT_GFX_*`。 |
| x / y / elevation | 初期配置。 |
| movement type | 通常時の自動挙動。`MOVEMENT_TYPE_FACE_*`、`MOVEMENT_TYPE_WANDER_*`、`MOVEMENT_TYPE_WALK_SEQUENCE_*` など。 |
| movement range | wander / walk sequence 系の範囲。 |
| trainer type / sight | trainer 視線判定。 |
| script | 話しかけた時や trainer 接触時に走る script label。 |
| event flag | set されていると非表示になる flag。 |

`events.inc` の先頭には `DO NOT MODIFY THIS FILE` が付く。NPC の座標や初期 movement type は Porymap / `map.json` 側、会話や cutscene は `scripts.inc` 側で扱う。

## Runtime Movement Type

`MOVEMENT_TYPE_*` は [src/event_object_movement.c](../../src/event_object_movement.c) の `sMovementTypeCallbacks` に接続される。

代表例:

| Movement type | Runtime callback | Notes |
|---|---|---|
| `MOVEMENT_TYPE_FACE_UP/DOWN/LEFT/RIGHT` | `MovementType_FaceDirection` | その場で固定方向を向く。 |
| `MOVEMENT_TYPE_LOOK_AROUND` | `MovementType_LookAround` | ときどき向きを変える。 |
| `MOVEMENT_TYPE_WANDER_*` | `MovementType_Wander*` | `object_event` の range を使う。 |
| `MOVEMENT_TYPE_WALK_*` | `MovementType_WalkBackAndForth` | 範囲内を往復する。 |
| `MOVEMENT_TYPE_WALK_SEQUENCE_*` | `MovementType_WalkSequence*` | 範囲付きの順序歩行。 |
| `MOVEMENT_TYPE_COPY_PLAYER*` | `MovementType_CopyPlayer*` | player の移動に追従する puzzle 系。 |
| `MOVEMENT_TYPE_INVISIBLE` | `MovementType_Invisible` | 表示しない object。 |
| `MOVEMENT_TYPE_FOLLOW_PLAYER` | `MovementType_FollowPlayer` | follower 用。 |

`sMovementTypeHasRange` が `TRUE` の type は、`object_event` の x/y range が意味を持つ。range を大きくしすぎると macro 側でも overflow warning / error の対象になる。

## Scripted Movement

一時的な cutscene 移動は `scripts.inc` の `applymovement` と movement label で行う。

例: [BattleFrontier_ReceptionGate/scripts.inc](../../data/maps/BattleFrontier_ReceptionGate/scripts.inc)

```asm
applymovement LOCALID_FRONTIER_RECEPTION_GREETER, Common_Movement_ExclamationMark
waitmovement 0
applymovement LOCALID_PLAYER, BattleFrontier_ReceptionGate_Movement_PlayerApproachCounter
waitmovement 0

BattleFrontier_ReceptionGate_Movement_PlayerApproachCounter:
    walk_up
    walk_up
    walk_left
    walk_left
    step_end
```

基本ルール:

| Command | Use |
|---|---|
| `lock` / `lockall` | player / NPC の通常入力と通常移動を止める。会話は `lock`、cutscene は `lockall` が多い。 |
| `faceplayer` | 選択中 NPC を player 方向へ向ける。 |
| `applymovement localId, label` | 指定 object に movement script を投げる。 |
| `waitmovement 0` | 最後に動かした object の movement 完了待ち。複数同時に動かした時は最後の object 基準になる点に注意。 |
| `waitmovementall` | 全 movement の完了待ち。複数 object を同時に動かす cutscene で安全。 |
| `setobjectxyperm` | object template の座標を恒久的に変える。画面外に出ても戻らない配置変更に使う。 |
| `copyobjectxytoperm` | live object の現在座標を template へ反映する。 |
| `setobjectmovementtype` | object template の通常 movement type を変える。 |
| `removeobject` | object を消し、visibility flag があれば set する。 |
| `addobject` | object を spawn する。ただし visibility flag は変更しない。 |

`applymovement` は一時動作、`setobjectxyperm` / `copyobjectxytoperm` は template 更新、`removeobject` は visibility flag 更新を伴う。ここを混ぜると「画面外に出たら戻る」「再入場で消える/復活する」の原因になる。

## Map Script Interaction

NPC の条件付き配置は `MAP_SCRIPT_ON_TRANSITION` / `MAP_SCRIPT_ON_FRAME_TABLE` とよく組み合わさる。

例: [SevenIsland_House_Room1_Frlg/scripts.inc](../../data/maps/SevenIsland_House_Room1_Frlg/scripts.inc)

```asm
SevenIsland_House_Room1_OnTransition::
    special ValidateEReaderTrainer
    call_if_eq VAR_RESULT, 0, SevenIsland_House_Room1_EventScript_SetTrainerVisitingLayout
    call_if_ne VAR_MAP_SCENE_SEVEN_ISLAND_HOUSE_ROOM1, 0, SevenIsland_House_Room1_EventScript_MoveOldWomanToDoor
    end

SevenIsland_House_Room1_EventScript_SetTrainerVisitingLayout::
    setvar TRAINER_VISITING, TRUE
    setobjectxyperm LOCALID_SEVEN_ISLAND_HOUSE_OLD_WOMAN, 4, 2
    setobjectmovementtype LOCALID_SEVEN_ISLAND_HOUSE_OLD_WOMAN, MOVEMENT_TYPE_FACE_DOWN
    setmaplayoutindex LAYOUT_SEVEN_ISLAND_HOUSE_ROOM1_DOOR_OPEN
    return
```

ここでは transition 中に:

- runtime condition を見て、NPC の template 座標と向きを変える。
- `setmaplayoutindex` で map layout 自体を差し替える。
- OnFrame table で battle 後コメント cutscene を起動する。

`ON_TRANSITION` は描画前に状態を組み替える用途、`ON_FRAME_TABLE` は入場後に player control を止めて cutscene を開始する用途に向く。

## Conditional Tile / Layout Changes

条件付きの見た目変更は 2 種類ある。

| Method | Timing | Use |
|---|---|---|
| `setmaplayoutindex LAYOUT_*` | layout load 前。典型的には `MAP_SCRIPT_ON_TRANSITION`。 | 潮位、開いた扉など、map layout 全体を差し替えたい時。 |
| `setmetatile x, y, METATILE_*, impassable` | layout load 後にも可能。`ON_LOAD` や event script。 | 1 tile / 小範囲だけ変えたい時。 |

例: [ShoalCave_LowTideInnerRoom/scripts.inc](../../data/maps/ShoalCave_LowTideInnerRoom/scripts.inc)

```asm
ShoalCave_LowTideInnerRoom_OnTransition:
    goto_if_set FLAG_SYS_SHOAL_TIDE, ShoalCave_LowTideInnerRoom_EventScript_SetHighTide
    goto ShoalCave_LowTideInnerRoom_EventScript_SetLowTide

ShoalCave_LowTideInnerRoom_EventScript_SetHighTide::
    setmaplayoutindex LAYOUT_SHOAL_CAVE_HIGH_TIDE_INNER_ROOM
    end

ShoalCave_LowTideInnerRoom_OnLoad:
    call ShoalCave_LowTideInnerRoom_EventScript_SetShoalItemMetatiles
    end
```

同じ map で:

- `ON_TRANSITION` が layout 全体を選ぶ。
- `ON_LOAD` が未取得 item の metatile を追加する。
- item 取得 event は `setmetatile` 後に `special DrawWholeMapView` を呼び、画面へ反映する。

`ScrCmd_setmetatile` は `x/y` に `MAP_OFFSET` を足して `MapGridSetMetatileIdAt` を呼ぶ。script から即時見た目を変えたら `DrawWholeMapView`、C 側で単一 tile だけなら `CurrentMapDrawMetatileAt` が必要になる。

## Town Map / Fly Link

Town Map / wall-mounted Region Map から Fly に進む導線は [src/field_region_map.c](../../src/field_region_map.c) が入口。

確認済みの流れ:

1. `FieldInitRegionMap` が region map UI 用 BG/window/sprite state を初期化する。
2. `DoRegionMapInputCallback` で cursor / A / B / R を処理する。
3. A/B close path は fade out 後に `FreeRegionMapIconResources`、`SetMainCallback2(callback)`、handler free、`FreeAllWindowBuffers` へ進む。
4. R Fly path は flyable mapsec、`OW_FLAG_POKE_RIDER`、`Overworld_MapTypeAllowsTeleportAndFly` を確認し、`SetFlyDestination`、`gSkipShowMonAnim = TRUE`、`ReturnToFieldFromFlyMapSelect` へ直接進む。
5. Fly 後の map name popup は [src/map_name_popup.c](../../src/map_name_popup.c) の `ShowMapNamePopup` / `Task_MapNamePopUpWindow` が BG0 offset と window tilemap を使って描画する。

懸念点:

| Risk | Why |
|---|---|
| R Fly path が A/B close path と同じ cleanup を通らない | region map の window / icon / BG state が Fly 後の field state に残る可能性がある。 |
| map name popup は BG0 offset / window buffer に依存する | region map UI の BG0/window state が残ると、popup の slide-in や位置に見えるズレが出る可能性がある。 |
| 既存候補は設計対象のみ | 現時点では field_region_map の code は変更しない。次に実装するなら A/B close path と R Fly path の cleanup 統一を別タスクで設計する。 |

## Investigation Checklist

NPC や map 表示の不具合を見る時は、次の順で追う。

1. `data/maps/TargetMap/events.inc` の `object_event` で local id、gfx、movement type、event flag を確認する。
2. 同じ map の `scripts.inc` で `MapScripts::`、`OnTransition`、`OnLoad`、`OnFrame` を確認する。
3. `applymovement` 先の movement label が `step_end` で終わるか確認する。
4. 複数 object が動く cutscene は `waitmovement 0` で足りるか、`waitmovementall` が必要か確認する。
5. NPC を恒久移動するなら `setobjectxyperm` / `copyobjectxytoperm` があるか確認する。
6. 非表示が絡むなら `removeobject` と object event flag の関係を見る。
7. 条件付き tile は `setmaplayoutindex` か `setmetatile` かを分ける。
8. `setmetatile` で即時見た目を変える場合、`special DrawWholeMapView` があるか確認する。
9. Town Map / Fly 後の popup は `field_region_map.c` と `map_name_popup.c` の BG/window state を分けて見る。

## Open Questions

- Town Map / wall region map の R Fly path は、通常 close path と同じ cleanup を通してから `ReturnToFieldFromFlyMapSelect` へ進めるべきか。
- Fly 後の map name popup ズレが BG0 offset、window buffer、sprite resource、palette のどれ由来かは、mGBA で R Fly path と party menu Fly path を比較して切り分ける必要がある。
- NPC movement の不具合報告がある場合、該当 map 名と local id を特定してから個別に `.inc` を読む。

