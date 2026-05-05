# TM Shop Migration

調査日: 2026-05-02

将来、TM を通常入手・拾得からマート販売へ寄せるための調査メモ。現時点では docs-only で、source code、map data、scripts の変更は行っていない。

## Goal

- TM を Pokemart / 将来の Medley Shop で買えるようにする。
- NPC/gym gift、visible item ball、hidden item、施設報酬など既存の TM 取得元を整理する。
- ただし `FOREACH_TM`、TM item constants、TM/HM pocket、learnset / teachable logic は消さない前提で影響範囲を分ける。

## Docs

| Doc | Purpose |
|---|---|
| `investigation.md` | 現在の TM 定義、取得元 flag、item ball / hidden item / shop flow の調査。 |
| `mvp_plan.md` | 実装する場合の段階的な作業案。未実装。 |
| `risks.md` | flag 削除、save compatibility、FRLG/Emerald 差分、shop UI のリスク。 |
| `test_plan.md` | 実装後に確認する観点。未実行。 |

## Current Status

| Item | Status |
|---|---|
| Code changes | None |
| Data changes | None |
| Docs | Investigating |

## Related Docs

- `docs/flows/map_script_flag_var_flow_v15.md`
- `docs/flows/script_inc_audit_v15.md`
- `docs/overview/extension_impact_map_v15.md`
- `docs/overview/source_map_v15.md`
