# Trainer Battle Party Selection

## Status

Validated branch on `feature/battle-selection-mvp`.

現行 `master` ベースの branch で、通常 trainer battle 前に既存
`choose half` party menu を流用して player party から battle 参加 Pokémon を
選出する最小実装を入れた。`feature/party-select-ui` の専用 UI branch は
`vanilla/v14_1` ベースで古いため、差分は参考に留め、現行 code へ丸ごと
取り込んでいない。

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
| `implementation.md` | 現行 MVP 実装、validation、残リスク |
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

## Current MVP Contract

| Case | Behavior |
|---|---|
| `B_TRAINER_BATTLE_SELECTION == FALSE` | 通常 trainer battle flow を維持する no-op。 |
| normal single trainer battle | eligible party が 4 匹以上なら 3 匹を選出してから battle を開始する。 |
| normal double trainer battle | eligible party が 5 匹以上なら 4 匹を選出してから battle を開始する。 |
| egg / fainted / empty slot | 通常 trainer battle selection では選出不可。 |
| selected mons | battle 中だけ `gPlayerParty[0..selectedCount-1]` に詰める。 |
| Cancel / B button | 通常 trainer encounter の script 復帰先が曖昧なため、trainer battle selection 中は無効。 |
| battle end | battle 後状態を元 slot へ反映し、元 party 順へ復元する。 |
| Frontier / link / Union Room / partner / two trainers / Pyramid / Hill | MVP 対象外として既存 flow を維持する。 |

この branch では feature を試せるように `B_TRAINER_BATTLE_SELECTION` を `TRUE`
にしている。最終統合時に default を `FALSE` に戻すかどうかは integration branch
で判断する。

## Manual

設定、処理 flow、flag / save policy は
[Trainer Battle Selection Manual](../../manuals/trainer_battle_selection_manual.md) を参照する。

## Open Questions

- battle transition animation 中に player / NPC trainer sprite が一瞬黒い影のように見える cosmetic issue を polish するか。
- 相手 party preview を実装する場合、Trainer Party Pools / randomize / override 反映済み party をどの timing で安全に得るか。
- custom selection UI を作る場合、既存 choose-half UI からどこまで置き換えるか。

## Cross-Feature Notes

Champions partygen が置き換えた Elite Four / Wallace は、現時点では通常
trainer battle として開始する。Battle selection MVP は player party の
選出と復元だけを扱い、相手 party pool の preview や再抽選は扱わない。

guard 方針:

| Case | Selection MVP |
|---|---|
| config off | No-op。通常 trainer battle flow を維持。 |
| normal trainer battle | 対象候補。single 3 / double 4 を選出。 |
| partygen-owned trainer | runtime 上は normal trainer と同じ。相手 preview は出さない。 |
| future Champions runtime active | challenge 専用 pre-battle menu / roster policy が優先。通常 selection は bypass。 |
| Frontier / cable / Union Room / link | 既存 choose-half flow を壊さないため除外。 |

`partygen_owned` tag は ROM runtime guard に使わない。将来 Champions runtime
が入った時だけ `ChampionsChallenge_IsActive()` のような explicit helper を
判定に使う。
