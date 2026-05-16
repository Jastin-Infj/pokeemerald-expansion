# TM Shop Migration

調査日: 2026-05-02
再確認日: 2026-05-16

Legacy Gen 3 TM acquisition を退役させるための feature handoff。2026-05-16
時点では `feature/tm-shop-migration` で implementation を入れ、Emerald
normal-progression の TM / HM item 入手導線を退役済み。

以前の目的は current 50 TM を Friendly Shop へ寄せることだったが、方針を変更する。
current 50 TM は Gen 3 世代の古い machine set として扱い、NPC / gym reward /
field item / facility prize からの通常取得導線を撤去する。将来 TM を復活させる
場合は、Gen 5 以降の reusable 仕様で、Gen 9 準拠の 200-250 TM 程度を別 feature
として設計する。

2026-05-16 follow-up で `I_REUSABLE_TMS` は `TRUE` に変更済み。既存 50 TM を
入手した場合は無制限使用扱いにする。HM は item として通常進行では渡さず、field
move 解放は別 feature / story flag 側へ寄せる。

## Goal

- Current 50 TM の NPC/gym gift、visible item ball、hidden item、shop、facility
  reward などの既存入手導線を退役させる。
- TM received / got / item-ball / hidden-item flag は参照を外した後、最終的には
  `FLAG_UNUSED_0x...` 相当へ戻す。ただし value の再番号付けはしない。
- `FOREACH_TM`、TM item constants、TM/HM pocket、learnset / teachable logic は
  first cut では削除しない。runtime 依存が広く、Unified Move Relearner の
  virtual TM 方針と衝突しやすい。
- Gym leader などの「TM item 報酬」は、将来必要なら Unified Move Relearner の
  virtual TM unlock / story unlock で賄う。
- 将来 TM を再実装する場合は、Gen 9 準拠の reusable TM set を新規設計する。
  TR / tutor と重複する move は、その時点で separate source / dedupe policy を
  決める。
- Debug route で一時的な TM shop を置く場合は、通常進行ではなく Debug menu
  `Scripts... > Script 1` の検証用に限定する。実装するなら legacy narrow item-id
  path ではなく、現行 `pokemart` macro / `u16` item list を使う。
- Emerald の HM gift / story reward は item を渡さない。既存進行を止めないため、
  必要な分岐は既存 story var / story flag へ寄せ、旧 HM receive flags は
  `FLAG_UNUSED_0x...` に戻す。

## 2026-05-16 Recheck Summary

| Area | Current finding |
|---|---|
| Current physical registry | `FOREACH_TM` は 50 entries、`FOREACH_HM` は 8 entries。 |
| Reserved TM items | `ITEM_TM01`..`ITEM_TM100` は存在するが、`ITEM_TM51`..`ITEM_TM100` は placeholder item data で `FOREACH_TM` には未登録。 |
| Reusable TM switch | `include/config/item.h` の `I_REUSABLE_TMS` は `TRUE`。既存 TM を入手した場合は無制限使用扱い。 |
| HM items | HM は `.importance = 1`、`.price = 0` の core item 定義を残すが、Emerald normal-progression の HM item grant は撤去済み。 |
| Existing TM shops | Lilycove Department Store に 8 TM、Slateport Power TMs に 2 TM、Mauville Game Corner に 5 TM coin prizes がある。 |
| Existing TM acquisition sources | `giveitem ITEM_TM_*` 系、visible item ball 14 件、hidden item 1 件、Trainer Hill / Lilycove Lady / Secret Power / Game Corner などが残る。 |
| Optional debug shop | `data/scripts/debug.inc` の `Debug_EventScript_Script_1` を `TM Shop Test` として実装済み。`pokemart` / `.2byte ITEM_*` / `pokemartlistend` で開く debug-only route。 |
| Legacy flag retirement | Emerald 側の 21 `FLAG_RECEIVED_TM_*`、1 `FLAG_GOT_TM_THUNDERBOLT_FROM_WATTSON`、14 visible item flags、1 hidden item flag が退役候補。FRLG 側は既に 0 alias。 |
| 200+ TM expansion | 物理 item として 200 以上増やす予定は現時点ではない。やるなら Bag Expansion / save layout / item ID ceiling を伴う別 feature。Move Relearner 側の broad TM pool は virtual candidate pool として分離済み。 |

## Docs

| Doc | Purpose |
|---|---|
| `investigation.md` | 現在の TM 定義、取得元 flag、item ball / hidden item / shop flow の調査。 |
| `mvp_plan.md` | 実装する場合の段階的な作業案。 |
| `implementation.md` | 実装内容、検証結果、残リスク、mGBA Live handoff。 |
| `risks.md` | flag 削除、save compatibility、FRLG/Emerald 差分、shop / bag / HM のリスク。 |
| `test_plan.md` | 実装後に確認する観点。 |

## Current Status

| Item | Status |
|---|---|
| Branch | `feature/tm-shop-migration` |
| Code changes | Emerald normal-progression legacy TM acquisition retired; reusable TMs enabled |
| Data changes | Emerald TM item balls / hidden item removed from `map.json`; Emerald HM item grants removed from scripts; debug-only TM shop added |
| Docs | Implementation handoff updated |

## Scope Note

This implementation covers Emerald normal progression. FRLG-specific maps still
contain separate legacy TM / HM routes such as `ITEM_TM03`, `ITEM_HM01`, and
`FLAG_GOT_TM*`; keep those as a follow-up slice unless the branch scope is
explicitly expanded.

## Related Docs

- `docs/flows/map_script_flag_var_flow_v15.md`
- `docs/flows/script_inc_audit_v15.md`
- `docs/flows/pokemon_learnset_flow_v15.md`
- `docs/features/bag_expansion/README.md`
- `docs/features/unified_move_relearner/implementation.md`
- `docs/manuals/tm_hm_manual.md`
- `docs/overview/tm_hm_expansion_250_v15.md`
- `docs/overview/extension_impact_map_v15.md`
- `docs/overview/source_map_v15.md`
