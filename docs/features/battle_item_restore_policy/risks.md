# Battle Item Restore Policy Risks

| Risk | Severity | Notes |
|---|---|---|
| `Recycle` regression | High | `usedHeldItem` を早く消すと Recycle が失敗する。 |
| `Harvest` / `Cud Chew` regression | High | Berry pocket 判定と delayed berry reuse が壊れやすい。 |
| `Pickup` target mismatch | Medium | `canPickupItem` と他 battler の `usedHeldItem` を参照する。 |
| Battle end overwrite | High | battle 中に Trick / Thief / Bestow / Symbiosis で item が移動した後、original item で上書きしてよいか仕様判断が必要。 |
| Facility rules mismatch | Medium | Frontier / Factory / Tent は既に duplicate held item rule を持つ。通常 battle に混ぜると既存ルールが変わる。 |
| Link battle mismatch | High | link / recorded battle は state 同期と再現性が重要。 |
| Test expectation changes | Medium | 既存 tests は「消費された」「Recycle できる」「Pickup できる」ことを確認している。battle end restore だけを見分ける test が必要。 |

## Key Principle

Battle 中は item を消費する。Battle 後に戻す。

この分離を崩すと、消費済み item を参照する既存 mechanics が連鎖的に壊れる。

## Cross-Feature Risks

| Risk | Severity | Notes |
|---|---|---|
| partygen-owned trainer の berry-heavy pool と復元 policy が噛み合わない | Medium | Wallace demo のように berry が多い pool では、通常 trainer 戦後に berry を戻すかどうかで連戦 difficulty が変わる。config default off で導入する。 |
| Champions Challenge loss policy と item restore が二重適用される | High | loss 時に restore してから challenge party を削除するのか、削除優先かを challenge policy で固定する。通常 restore helper は `ChampionsChallenge_IsActive()` で bypass する。 |
| Trainer Party Pool item clause と battle-end restore を混同する | Medium | duplicate held item validation は partygen / battle selection 側、battle-end restore は battle_util 側で分ける。 |
