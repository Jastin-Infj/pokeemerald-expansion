# Field Move Modernization / HM Removal

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-09 |
| Baseline | `master` `4af2beb493`; `git describe` = `expansion/1.15.2-31-g4af2beb493` |
| Code status | Initial MVP implemented on `feature/field-move-modernization-mvp` |
| Provenance | Local project feature docs |

Status: Initial MVP implemented
Code status: Runtime slice in progress on `feature/field-move-modernization-mvp`

## Goal

Gen7 / Gen8 以降に近い設計として、フィールド進行を HM 技所持に依存しない形へ移行できるか調査する。

候補:

- Cut / Rock Smash / Strength などの障害物を撤去または key item / flag 解禁へ置き換える。
- Surf / Fly / Waterfall / Dive などを Pokemon の技から切り離す。
- HM move の忘却制限、Move Deleter、summary、learn move UI の扱いを決める。
- `"{STR_VAR_1} used {STR_VAR_2}!"` 系 message と field move animation を残すか、ride/key item 演出へ置き換える。

## Primary Docs

- `docs/flows/field_move_hm_flow_v15.md`
- `docs/features/field_move_modernization/investigation.md`
- `docs/features/field_move_modernization/mvp_plan.md`
- `docs/features/field_move_modernization/risks.md`
- `docs/features/field_move_modernization/test_plan.md`
- `docs/features/field_move_modernization/implementation.md`

## Current Conclusion

現行 v15 では、field move は以下の 4 系統にまたがっている。

1. map script / `checkfieldmove`
2. party menu action / `CursorCb_FieldMove`
3. field effect / animation / follower
4. HM forget / release / catch-swap softlock prevention

したがって、`FOREACH_HM` だけを消す、または `ScrCmd_checkfieldmove` だけを変える改造は危険。最初の MVP は「技所持不要の判定 path」を追加し、script return を壊さず段階的に置き換える方が安全。

2026-05-09 の初期実装では、HM field move の script 判定を badge unlock に寄せ、Pokemon show-mon banner / 黒背景 cut-in を effect 側で即完了化した。Cut / Rock Smash / Strength / Surf / Waterfall は成功時 prompt / 使用メッセージを出さずに実行し、Dive / Surface だけ誤操作防止の確認を残す。Dive down は A、underwater Surface は B、party menu 入口は Dive / Surface 両対応。Flash は unlock 済みなら cave 入場時に自動で明るくする。

将来は badge-only から、story flag、per-HM key item、または単一 field toolkit / power-up item へ unlock source を移す余地がある。推奨は単一 key item + capability flags。アイテムは UI / lore anchor に留め、Surf / Dive などの実解禁は flags で増やす。per-HM item 方式は Key Items pocket を圧迫し、badge-only は story pacing の自由度が低い。Key Items pocket は現状 `BAG_KEYITEMS_COUNT 30` の固定長なので、bag 拡張は別 feature の大型改修として扱い、この field move feature では実装しない。

## Non-Goals for Current Slice

- 既存 map object を削除しない。
- party menu / Fly / key item UI を完成させない。
- HM forget / PC release / catch-swap policy はまだ変更しない。
- Emerald Rogue など外部 project からの code copy は行わない。
