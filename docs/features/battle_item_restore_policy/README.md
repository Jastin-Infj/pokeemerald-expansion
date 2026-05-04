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
