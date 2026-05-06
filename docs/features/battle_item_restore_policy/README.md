# Battle Item Restore Policy

Status: Investigating
Code status: No code changes

## Goal

戦闘中に消費された持ち物を、対戦型ルールに合わせて戦闘後に復元する方針を整理する。

主な対象:

- `B_RESTORE_HELD_BATTLE_ITEMS`
- きのみ / Berry Juice / Gems / White Herb / Weakness Policy / Red Card / Eject Button などの single-use held items
- `Recycle`、`Pickup`、`Harvest`、`Cud Chew`、`G-Max Replenish` など、消費済み持ち物を battle 中に参照する処理
- item clause / duplicate held item rule

## Primary Docs

- `docs/features/battle_item_restore_policy/investigation.md`
- `docs/features/battle_item_restore_policy/mvp_plan.md`
- `docs/features/battle_item_restore_policy/risks.md`
- `docs/features/battle_item_restore_policy/test_plan.md`
- `docs/overview/move_item_ability_map_v15.md`
- `docs/flows/battle_effect_resolution_flow_v15.md`

## Current Conclusion

現在の実装では、`B_RESTORE_HELD_BATTLE_ITEMS >= GEN_9` でも、戦闘後に復元されるのは基本的に非きのみの single-use item だけ。`src/battle_util.c` の `TryRestoreHeldItems` は、元の持ち物が Berry pocket の場合に復元対象から外している。

ただし、きのみを戦闘中に「消費しない」扱いへ変えるのは危険。`usedHeldItem` は `Recycle`、`Pickup`、`Harvest`、`Cud Chew`、`G-Max Replenish` の runtime state として使われている。安全な方向は、battle 中は今まで通り消費済みとして扱い、battle end aftercare で元の held item を復元する設計。

## Non-goals for now

- この段階では C 実装をしない。
- Town Map / Field Region Map の R Fly cleanup もこの feature では実装しない。
- Bag item quantity の無限化や shop economy は別 feature として扱う。

## Cross-Feature Notes

Champions partygen は berry / single-use item を持つ trainer pool を生成できる。
例として現行 Hoenn demo の Wallace pool は Sitrus Berry を複数含む。
そのため battle-end item restore を通常 trainer battle 全体へ広げると、
partygen-owned trainer の連戦体験にも影響する。

初回実装では、戦闘中の消費処理は変更しない。`usedHeldItem`、
`canPickupItem`、`Recycle`、`Harvest`、`Cud Chew`、`Pickup` などの
runtime state を維持し、battle 終了時の復元 policy だけを helper に寄せる。

適用条件は aftercare と同じ考え方にする。

| Case | Restore MVP |
|---|---|
| config off | 既存 `B_RESTORE_HELD_BATTLE_ITEMS` 挙動を維持。 |
| config on, normal trainer | battle-end policy に従って player party の original held item を復元。 |
| partygen-owned trainer | Champions runtime 未実装時は normal trainer と同じ。 |
| future Champions runtime active | challenge item policy を優先し、通常 restore と二重適用しない。 |
| link / recorded / facility | MVP では除外候補。通信同期と施設ルールを別途見る。 |

`partygen_owned` tag は tool-side metadata なので、C 側の if には使わない。
将来 runtime 判定が必要な場合は `ChampionsChallenge_IsActive()` のような
explicit state helper を使う。
