# Trainer Battle Party Selection Test Plan

## Status

Draft. 現時点では実装がないため、まだテストは実行しない。

## Manual Test Matrix

| Case | Setup | Expected Result |
|---|---|---|
| Single 6 to 3 | player party 6 匹、通常 single trainer | 3 匹選出し、その 3 匹だけで battle |
| Double 6 to 4 | player party 6 匹、double trainer | 4 匹選出し、double battle が開始 |
| Party order restore | slot 2/5/6 などを選出 | battle 後、元の 6 匹順へ戻る |
| HP/status reflect | battle 中に HP/status が変化 | 選出元 slot に変化が反映 |
| PP reflect | 技 PP を消費 | 選出元 slot の PP が減る |
| EXP/level reflect | battle で level up | 選出元 slot に EXP/level が反映 |
| Held item reflect | battle 中に item 消費 | 選出元 slot に item 変化が反映 |
| Whiteout | 選出 party が全滅 | 復元後に通常 whiteout flow へ進む |
| Win trainer flag | battle 勝利 | trainer flag が立ち、post battle script が正しく進む |
| Already beaten trainer | 既戦闘 trainer | 選出 UI は出ず、post battle script へ進む |
| Cancel behavior | 選出 UI で B | 仕様通りに禁止または encounter を継続 |
| Insufficient party | 2 匹以下で single trainer | 仕様通りの message / fallback |
| Fainted mons | fainted を含む party | 選出可否が仕様通り |
| Egg | egg を含む party | 既存 eligibility または仕様通り |
| Rematch trainer | rematch script | battle 前後 script が壊れない |
| No intro trainer | no intro mode | 選出 UI の timing が不自然でない |
| Continue script trainer | continue script mode | battle 後に元 script へ戻る |
| Battle party status summary | 選出後 battle start | 既存仕様通り 6 slot tray で表示されるか、変更仕様なら selected count と一致 |
| In-battle move reorder | `B_MOVE_REARRANGEMENT_IN_BATTLE` 有効 | battle 後、move order が選出元 slot へ反映 |
| Summary from selection UI | 選出 UI から summary を開く仕様にした場合 | 元 6 匹 party の正しい slot を表示して戻れる |

## Exclusion Regression Tests

MVP で触らない予定の領域が壊れていないことを確認する。

| Area | Expected |
|---|---|
| Battle Frontier party select | 既存の選出 UI / order 保存が変わらない |
| Cable club choose half | 既存通信施設 flow が変わらない |
| Union Room | 既存 party selection が変わらない |
| Wild battle | 選出 UI が出ない |
| Link battle | 選出 UI が出ない |
| Trainer Hill / Battle Pyramid | 既存特殊 battle flow が変わらない |
| Follower partner battle | MVP では対象外として既存挙動維持 |
| Two trainers battle | MVP では対象外として既存挙動維持 |
| Trainer party pools | 相手 party preview を MVP では出さない。既存 pool / random order battle が変わらない |

## State Integrity Checks

実装後に debug print / test hook などで確認したい項目:

- battle 前の original party copy が `PARTY_SIZE` 分保存されている。
- `selectedSlots` が `gSelectedOrderFromParty[i] - 1` と一致する。
- temporary `gPlayerParty` の count が single 3 / double 4 になっている。
- battle end restore 後の `CalculatePlayerPartyCount()` が元 count と一致する。
- non-selected slots が battle 前から変化していない。
- selected slots だけが battle 後状態に更新されている。
- restore 後に selection active flag が clear される。

## Automated Test Ideas

既存 test framework の詳細は未調査だが、将来的に以下を検討する。

| Test | Purpose |
|---|---|
| party mapping unit test | selected temporary slots を元 slot へ戻せるか |
| restore after outcome test | won/lost/forfeit で必ず復元されるか |
| validation unit test | single 3 / double 4 の min/max を満たすか |
| Frontier regression test | `gSelectedOrderFromParty` 共有による破壊がないか |
| battle UI regression test | selected party count 変更で healthbox / party status summary が壊れないか |
| trainer pool determinism test | preview を後続 phase で入れる場合、pool 生成が本戦と一致するか |
| partygen fixed party test | fixed party には `Party Size` が出ず、pool path に入らないこと |
| partygen pool party test | pool trainer だけ `Party Size` / `Pool Rules` / `Tags` が出ること |
| partygen trainerproc edge test | `Copy Pool`、`Macro`、Ball、Tera / Dynamax 排他、header preserve を検査すること |
| partygen target test | FRLG / battle partner / test `.party` を default で書き換えないこと |

## Open Questions

- 既存 repo の automated test 実行方法は未整理。
- battle 中の evolution / move learn を自動テストでどう扱うか未決定。
- cancel を許す仕様にする場合、manual test の expected result を更新する必要がある。
- party status summary を 6 slot 表示のままにするか、3/4 slot 表示へ変えるかで expected result が変わる。
- opponent party preview を入れる phase では、Trainer Party Pools / RNG / override trainer の専用 test が必要。
