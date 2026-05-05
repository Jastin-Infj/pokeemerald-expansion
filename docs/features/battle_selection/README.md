# Trainer Battle Party Selection

## Status

Investigating. 現時点では実装しない。

この directory は、通常 trainer battle 前に player party から battle 参加 Pokémon を選出する機能の調査・設計メモを管理する。

## Goal

- single trainer battle では手持ち 6 匹から 3 匹を選出する。
- double trainer battle では手持ち 6 匹から 4 匹を選出する。
- battle 中は選出 Pokémon だけで一時的な `gPlayerParty` を構築する。
- battle 終了後、選出 Pokémon の battle 後状態を元 slot へ反映する。
- 非選出 Pokémon を含む元の 6 匹 party 順へ安全に復元する。

## Initial Constraint

専用 UI はまだ作らない。まず既存の `party_menu` / `choose half party` 系処理を流用できるか調査する。

## Related Docs

| Doc | Purpose |
|---|---|
| `investigation.md` | 既存コード調査結果 |
| `mvp_plan.md` | 実装する場合の最小構成案 |
| `risks.md` | 危険箇所と対策候補 |
| `test_plan.md` | 将来実装時の検証項目 |
| `opponent_party_and_randomizer.md` | 相手 party preview、Trainer Party Pools、party randomize / reorder 調査 |
| `docs/flows/trainer_battle_flow_v15.md` | trainer battle 開始前後の flow |
| `docs/flows/choose_half_party_flow_v15.md` | 既存選出 UI flow |
| `docs/flows/battle_start_end_flow_v15.md` | battle start/end と party 復元の flow |
| `docs/flows/battle_ui_flow_v15.md` | battle 開始後 UI と影響範囲 |
| `docs/flows/options_status_flow_v15.md` | option / status / summary UI |
| `docs/flows/script_inc_audit_v15.md` | `.inc` script audit |

## Non-Goals for First MVP

- Pokémon Champions 風の専用選出 UI。
- 相手 party preview。
- link battle / cable club / Battle Frontier / Union Room の挙動変更。
- two trainers / follower partner / multi battle の完全対応。
- `gEnemyParty` 作成 timing の変更。
- battle 開始後の healthbox / party status summary / action menu layout 変更。
- runtime option の追加。

## Open Questions

- 既存 `PARTY_MENU_TYPE_CHOOSE_HALF` を通常 trainer battle 用にそのまま使えるか。
- 4 匹選出が `MAX_FRONTIER_PARTY_SIZE` などの既存制限に当たらないか。
- battle 終了後の復元は `CB2_EndTrainerBattle` 前に wrapper callback で行うべきか、既存 callback 内へ統合すべきか。
- 相手 party preview を実装する場合、Trainer Party Pools / randomize / override 反映済み party をどの timing で安全に得るか。
