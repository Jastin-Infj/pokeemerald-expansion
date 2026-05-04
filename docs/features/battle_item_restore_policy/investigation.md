# Battle Item Restore Policy Investigation

調査日: 2026-05-04

## Current Configs

| Config | File | Current meaning |
|---|---|---|
| `B_RESTORE_HELD_BATTLE_ITEMS` | `include/config/battle.h` | Gen9+ では battle 後に non-berry single-use item を復元する。 |
| `B_TRAINERS_KNOCK_OFF_ITEMS` | `include/config/battle.h` | trainer が player item を steal/swap/remove できるか。 |
| `B_RETURN_STOLEN_NPC_ITEMS` | `include/config/battle.h` | NPC 側から盗んだ item を返す世代設定。 |
| `B_POOL_RULE_ITEM_CLAUSE` | `include/config/battle.h` | trainer party pool 生成時の duplicate held item 制限。 |

## Current Restore Flow

Confirmed symbols:

| File | Symbol | Fact |
|---|---|---|
| `src/battle_main.c` | battle init | `itemLost[side][slot].originalItem` に battle 開始時の held item を保存する。 |
| `src/battle_main.c` | battle end | `B_TRAINERS_KNOCK_OFF_ITEMS == TRUE` または `B_RESTORE_HELD_BATTLE_ITEMS >= GEN_9` なら `TryRestoreHeldItems()` を呼ぶ。 |
| `src/battle_util.c` | `TryRestoreHeldItems` | player party の original item を見て、条件を満たす item を held item に戻す。 |
| `src/battle_script_commands.c` | remove item command | held item 消費時に `GetBattlerPartyState(battler)->usedHeldItem` へ記録する。 |
| `src/battle_move_resolution.c` | `EFFECT_NATURAL_GIFT` | Natural Gift は berry を消費し、`usedHeldItem` に記録する。 |

`TryRestoreHeldItems` の現状:

- `B_RESTORE_HELD_BATTLE_ITEMS >= GEN_9` なら復元処理へ入る。
- original item が Berry pocket かつ現在その mon が同じ item を持っていなければ、`lostItem = ITEM_NONE` にする。
- 最終的に Berry pocket の item は `SetMonData(... MON_DATA_HELD_ITEM ...)` の対象外になる。

このため、現在は「きのみだけ意図的に戦闘後復元から外れている」挙動になる。

## Why berries are sensitive

現時点で upstream の意図は断定しない。コード上の事実として、きのみは単なる消費物ではなく、消費後 state を参照する処理が多い。

| Mechanic | Dependency |
|---|---|
| `Recycle` | `usedHeldItem` から item を復元する。 |
| `Pickup` | 他 battler の `usedHeldItem` と `canPickupItem` を見て拾う。 |
| `Harvest` | 自分の `usedHeldItem` が Berry pocket の場合に復元候補にする。 |
| `Cud Chew` | 次ターンに `usedHeldItem` の berry effect を再利用する。 |
| `G-Max Replenish` | target の `usedHeldItem` が Berry pocket の場合に berry を戻す。 |
| `Belch` | berry を食べた事実が move 使用可否に関係する。 |

したがって、対戦型ルールで「戦闘後にきのみを戻したい」場合でも、battle 中の `usedHeldItem` 記録は残す必要がある。

## Recommended Direction

安全な設計は次の分離。

| Timing | Behavior |
|---|---|
| During battle | 今まで通り held item を消費し、`usedHeldItem` / `canPickupItem` を更新する。 |
| During battle item effects | `Recycle`、`Pickup`、`Harvest`、`Cud Chew` などは既存 state を使う。 |
| Battle end | 対戦型ルールが有効なら、battle 開始時の `originalItem` を party slot に復元する。 |

この方針なら、battle 中の公式的な処理を壊さず、battle 後だけ Champions-style / competitive-style の「持ち物が失われない」ルールに寄せられる。

## Item Clause

既存の duplicate held item 制限は複数箇所にある。

| Area | File | Fact |
|---|---|---|
| Battle Frontier generated party | `src/battle_frontier.c`, `src/battle_tower.c`, `src/battle_factory.c`, `src/battle_tent.c` | enemy / rental generation で duplicate species/item を避ける。 |
| Choose-half-party UI | `src/party_menu.c` `CheckBattleEntriesAndGetMessage` | Frontier 系 facility では duplicate species/item を弾く。Union Room / Multi or E-reader は除外。 |
| Trainer party pools | `src/trainer_pools.c`, `src/data/battle_pool_rules.h` | `B_POOL_RULE_ITEM_CLAUSE` と exclusion list がある。 |

通常 trainer battle 全体へ item clause を広げる場合は、Frontier 固有 validation をそのまま流用するのではなく、battle selection feature 側の validation policy と分ける必要がある。

## Open Questions

- 新しい復元 config は `B_RESTORE_HELD_BATTLE_ITEMS` の拡張にするか、別 config にするか。
- きのみ復元を常時有効にするか、対戦型ルール時だけ有効にするか。
- player party だけ復元するか、NPC / link / battle facility / test battle も同じ扱いにするか。
- Knock Off / Thief / Trick / Bestow / Symbiosis で battle 終了時にどの original item を優先するか。
- `Harvest` や `Pickup` で battle 中に別 item を得た場合、battle end で original item に戻すか、得た item を保持するか。
