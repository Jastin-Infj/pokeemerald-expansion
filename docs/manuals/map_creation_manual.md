# 新規マップ作成マニュアル

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-17 |
| Baseline | `master` `36fa0b2dc4`; `git describe` = `expansion/1.15.2-61-g36fa0b2dc4` |
| Code status | Docs-only manual |
| Provenance | Local source read and existing map docs |

## 目的

この manual は、新しい map を追加するときに「どこを作るか」「どこを編集してはいけないか」「どの命名を揃えるか」を先に固定する入口です。

特に、Porymap で見える `map.json` と、build が実際に読む generated `.inc` / hand-written `scripts.inc` の境界を明確にします。

## 先に読むもの

| Doc | 役割 |
|---|---|
| [Map Creation Flow v15](../flows/map_creation_flow_v15.md) | 新規 map の作成順と依存関係。 |
| [Map Script Flow v15](../flows/map_script_flow_v15.md) | `MAP_SCRIPT_*` table と `scripts.inc` の実行順。 |
| [Map Script / Flag / Var Flow v15](../flows/map_script_flag_var_flow_v15.md) | flag / var / item ball / hidden item / coord event の扱い。 |
| [NPC / Object Event Flow v15](../flows/npc_object_event_flow_v15.md) | NPC 配置、movement、hide flag、条件付き metatile。 |
| [Map Registration / Region Map / Fly Flow v15](../flows/map_registration_fly_region_flow_v15.md) | `MAPSEC_*`、Town Map、Fly destination、FRLG world map flag。 |
| [Inc Script Pipeline v15](../overview/inc_script_pipeline_v15.md) | `.inc` script と generated map data の build pipeline。 |

## 最重要ルール

### 編集するファイル

| 目的 | 編集元 |
|---|---|
| map の header field、event 配置、warp、connection | `data/maps/<MapName>/map.json` |
| map の script 本体、会話、movement、map script table | `data/maps/<MapName>/scripts.inc` |
| map script を ROM に入れる | `data/event_scripts.s` の `.include` |
| map group / `MAP_*` 定数生成元 | `data/maps/map_groups.json` |
| layout / `LAYOUT_*` 定数生成元 | `data/layouts/layouts.json` と `data/layouts/<LayoutName>/` |
| Region Map 上の名前と座標 | `src/data/region_map/region_map_sections.json` |
| Region Map の tile layout | `src/data/region_map/region_map_layout*.h` |
| Fly icon / destination | `src/region_map.c` |
| FRLG preview / world map 初訪問 | `src/map_preview_screen.c`, `include/map_preview_screen.h`, `setworldmapflag` |

### 直接編集しないファイル

| Generated file | Source |
|---|---|
| `data/maps/<MapName>/header.inc` | `map.json` + `layouts.json` |
| `data/maps/<MapName>/events.inc` | `map.json` |
| `data/maps/<MapName>/connections.inc` | `map.json` |
| `data/maps/headers.inc` | `map_groups.json` + map list |
| `data/maps/events.inc` | `map_groups.json` + map list |
| `data/maps/groups.inc` | `map_groups.json` |
| `data/maps/connections.inc` | `map_groups.json` + map list |
| `data/layouts/layouts.inc` | `layouts.json` |
| `data/layouts/layouts_table.inc` | `layouts.json` |
| `include/constants/map_groups.h` | `map_groups.json` + `map.json` ids |
| `include/constants/map_event_ids.h` | `map.json` の `local_id` / `warp_id` |
| `include/constants/layouts.h` | `layouts.json` |
| `include/constants/region_map_sections.h` | `region_map_sections.json` |

生成物に手を入れると、次の build / clean で消えるか、Porymap と不一致になります。

## 命名規則

新規 map は、既存の近い map を 1 つ選び、その命名規則をコピーします。

| 種別 | 例 | ルール |
|---|---|---|
| Directory | `data/maps/LittlerootTown_ProfessorBirchsLab/` | `map.json` の `name` と一致させる。 |
| Map name | `"name": "LittlerootTown_ProfessorBirchsLab"` | CamelCase + `_` 区切り。directory 名と一致。 |
| Map id | `"id": "MAP_LITTLEROOT_TOWN_PROFESSOR_BIRCHS_LAB"` | `MAP_` + upper snake case。 |
| Layout id | `"layout": "LAYOUT_LITTLEROOT_TOWN_PROFESSOR_BIRCHS_LAB"` | `LAYOUT_` + upper snake case。 |
| Layout folder | `data/layouts/LittlerootTown_ProfessorBirchsLab/` | 基本は map directory と同じ。共有 layout なら既存名を使う。 |
| Map scripts label | `LittlerootTown_ProfessorBirchsLab_MapScripts::` | `<MapName>_MapScripts::`。 |
| Event script label | `LittlerootTown_ProfessorBirchsLab_EventScript_Aide` | `<MapName>_EventScript_<用途>`。 |
| Text label | `LittlerootTown_ProfessorBirchsLab_Text_Aide` | `<MapName>_Text_<用途>`。 |
| Movement label | `LittlerootTown_ProfessorBirchsLab_Movement_PlayerWalksIn` | `<MapName>_Movement_<用途>`。 |
| Local id | `LOCALID_BIRCHS_LAB_AIDE` | `include/constants/map_event_ids.h` に自動生成される。短くても map 内で意味が分かる名前にする。 |
| Hide flag | `FLAG_HIDE_LITTLEROOT_TOWN_BIRCHS_LAB_BIRCH` | object を恒久的に隠す用途。`0` なら常時表示。 |
| Mapsec | `MAPSEC_LITTLEROOT_TOWN` | 屋内は親 town/city の `MAPSEC_*` を共有することが多い。 |

## 最小構成

完全な新規 map には最低限これが必要です。

```text
data/maps/<MapName>/
  map.json
  scripts.inc

data/layouts/<LayoutName>/
  border.bin
  map.bin

data/maps/map_groups.json
data/layouts/layouts.json
data/event_scripts.s
```

`map.json` に `object_events` が 0 件でも、`scripts.inc` には最低限 `MapName_MapScripts::` を置きます。

```asm
MapName_MapScripts::
	.byte 0
```

`mapjson` は `header.inc` の `mapScripts` field に `<MapName>_MapScripts` を出します。`scripts.inc` に label が無い、または `data/event_scripts.s` に include されていない場合、build は symbol 解決で失敗します。

## Porymap との境界

| Porymap / `map.json` で扱う | `scripts.inc` で扱う |
|---|---|
| layout の参照 | `MapName_MapScripts::` table |
| music / weather / map type / mapsec | `MAP_SCRIPT_ON_TRANSITION` / `ON_LOAD` / `ON_FRAME_TABLE` |
| object / warp / coord / bg event の配置 | NPC 会話、cutscene、movement、trainer battle |
| object の初期 graphic / movement / flag | `setflag` / `clearflag` / `setvar` / `compare` |
| connections | `setmetatile`, `setmaplayoutindex`, `applymovement` |
| wild encounter editing | 実際に coord / object / bg event で呼ばれる script body |

Porymap は event の「置き場所」と「呼ぶ label」を持ちます。label の中身は `scripts.inc` に手書きします。

## 地域と build 対象

`tools/mapjson/mapjson.cpp` は `map.json` の `region` を見て、Emerald build と FRLG build の対象 map を分けます。

| Build | 必要な region | 注意 |
|---|---|---|
| Emerald | `REGION_HOENN` | `region` field が無い場合は `REGION_HOENN` 扱い。 |
| FireRed / LeafGreen | `REGION_KANTO` | FRLG map は `map.json` に `"region": "REGION_KANTO"` を入れる。 |

FRLG map を作る場合は [How to use FireRed/LeafGreen](../tutorials/how_to_frlg.md) の Porymap 設定も確認します。

## 作成順

1. 既存の近い map を選ぶ。town / route / indoor / cave / facility を混ぜない。
2. directory、`name`、`id`、`layout` の命名を決める。
3. `data/layouts/<LayoutName>/` と `data/layouts/layouts.json` を用意する。
4. `data/maps/<MapName>/map.json` を作る。
5. `data/maps/map_groups.json` の適切な group に `<MapName>` を追加する。
6. `data/maps/<MapName>/scripts.inc` を作り、`<MapName>_MapScripts::` を定義する。
7. `data/event_scripts.s` に `.include "data/maps/<MapName>/scripts.inc"` を追加する。
8. `map.json` の object / warp / coord / bg event が参照する script label を `scripts.inc` に作る。
9. 屋外 map なら connection、mapsec、Region Map、Fly 対応が必要かを決める。
10. item ball / hidden item / NPC hide flag がある場合、flag owner を決める。
11. `rtk make -j16 -O all` で mapjson 生成と build を確認する。
12. Porymap で開き、map が list に出ること、layout と event が表示されることを確認する。
13. mGBA で warp in / warp out / map name popup / NPC / collision / save-load を確認する。

## よくある失敗

| 症状 | 疑う場所 |
|---|---|
| Porymap に map が出ない | `data/maps/map_groups.json` に無い、`map.json` の `name` と directory が違う、FRLG map なのに `region` が無い。 |
| build で `<MapName>_MapScripts` が undefined | `scripts.inc` に label が無い、または `data/event_scripts.s` に include していない。 |
| build で event script label が undefined | `map.json` の `script` field が `scripts.inc` に存在しない。 |
| `MAP_*` が見つからない | `map_groups.json` に追加していない、または generated constants が stale。 |
| `LAYOUT_*` が見つからない | `layouts.json` に追加していない、layout id が違う。 |
| NPC が消える / 出ない | object `flag` が set されている、`removeobject` が hide flag を set している、`addobject` 前に `clearflag` していない。 |
| 話しかけても何も起きない | object の `script` が `0x0`、または label 未定義。 |
| warp 先がおかしい | `dest_map` / `dest_warp_id` と相手 map の `warp_events` 番号が合っていない。 |
| outdoor connection がずれる | connection の `direction` / `offset` が片側だけ正しくない。 |
| map name が出ない | `show_map_name` が false、または `region_map_section` が想定外。 |
| Fly icon / destination がズレる | `MAPSEC_*`, region map layout, `sFlyLocations`, `GetMapsecType`, destination table が揃っていない。 |
| FRLG map preview が変 | `setflag` と `setworldmapflag` を混同している。 |

## 実装 branch の検証基準

新規 map を runtime branch で追加したら、最低限次を記録します。

| Check | Command / evidence |
|---|---|
| generated map data | `rtk make -j16 -O all` |
| debug route / debug warp を使う場合 | `rtk make -j16 -O debug` |
| script / save / battle / item logic を触る場合 | `rtk make -j16 -O check` |
| docs | `rtk mdbook build docs` |
| Porymap | map list に出る、layout が開ける、event が見える |
| mGBA | boot、map in/out、warp、NPC 会話、collision、map name popup |
| 追加要素 | Fly、Region Map、wild encounter、item ball、hidden item、trainer battle、weather |

## Open Questions

- 新規 local map は Hoenn 側に寄せるか、FRLG 側に寄せるか、独自 region へ切るか。
- Porymap で作った map の追加差分を自動 lint する project-local tool が必要か。
- `data/event_scripts.s` の include 追加忘れを CI で検出する軽量 check を作るか。
