# Field Move Modernization / HM Removal

Status: Investigating
Code status: No code changes

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

## Current Conclusion

現行 v15 では、field move は以下の 4 系統にまたがっている。

1. map script / `checkfieldmove`
2. party menu action / `CursorCb_FieldMove`
3. field effect / animation / follower
4. HM forget / release / catch-swap softlock prevention

したがって、`FOREACH_HM` だけを消す、または `ScrCmd_checkfieldmove` だけを変える改造は危険。最初の MVP は「技所持不要の判定 path」を追加し、既存 animation と script return を壊さず段階的に置き換える方が安全。

## Non-Goals for Current Investigation

- まだ実装しない。
- 既存 map object を削除しない。
- `src/`, `include/`, `data/`, `config/` は変更しない。
- Emerald Rogue など外部 project からの code copy は行わない。
