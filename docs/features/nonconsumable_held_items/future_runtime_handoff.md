# Future Runtime Handoff: Nonconsumable Held Items

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-19 |
| Baseline | `master` `25731e81a0`; implementation branch `feature/held-item-catalog-current-master-20260519` |
| Code status | Historical restore handoff plus catalog implementation note |
| Provenance | Local source read and feature planning |

## Future Use Only

Do not use this prompt during the docs-only task. It is for a future runtime
branch after the user explicitly asks for implementation.

```text
runtime implementation task.

目的:
Nonconsumable Held Items の最初の runtime slice を実装する。
戦闘中に消費された player party の held item を、戦闘後に元の party slot へ
復元する。戦闘中の item 消費処理は変更しない。

branch:
feature/nonconsumable-held-items-restore-mvp

必ず最初に実行:
rtk git status --short --branch
rtk git describe --tags --always --dirty
rtk gh pr list --state open --json number,title,isDraft,headRefName,baseRefName,updatedAt,mergeStateStatus,statusCheckRollup

参照docs:
- docs/features/nonconsumable_held_items/README.md
- docs/features/nonconsumable_held_items/investigation.md
- docs/features/nonconsumable_held_items/mvp_plan.md
- docs/features/nonconsumable_held_items/risks.md
- docs/features/nonconsumable_held_items/test_plan.md
- docs/features/battle_item_restore_policy/README.md
- docs/features/battle_item_restore_policy/impact_scope.md

実装方針:
- current master から fresh runtime branch を切る。
- old branch を直接 merge しない。
- 戦闘中の held item 消費、usedHeldItem、canPickupItem、Unburden、Recycle、
  Pickup、Harvest、Cud Chew の状態を壊さない。
- battle start で保存済みの itemLost[B_SIDE_PLAYER][slot].originalItem を
  battle-end restore の source of truth にする。
- first slice は battle-end restore のみ。
- Bag / Party / Storage の catalog assignment はこの slice では実装しない。
- Mail / Storage / duplicate assignment / item clause はこの slice では触らない。

候補ファイル:
- include/config/battle.h
- src/battle_util.c
- src/battle_main.c
- test/battle_item_restore.c または focused battle test
- docs/features/nonconsumable_held_items/*
- docs/features/battle_item_restore_policy/*
- docs/manuals/local_config_and_flag_ledger.md
- docs/manuals/validation_evidence_matrix.md

禁止:
- SaveBlock を変更しない。
- Bag Expansion と混ぜない。
- Party / Bag / Storage catalog assignment をこの slice に入れない。
- during-battle consumption を無効化しない。
- usedHeldItem を早期 clear しない。
- Frontier / Tent / link / recorded battle の rule を推測で変更しない。
- TM Shop / Unified Move Relearner / Summary / State Editor を触らない。

検証:
rtk mdbook build docs
rtk make -j16 -O all
rtk make -j16 -O debug
rtk make -j16 -O check

mGBA:
- berry を持った party Pokemon で battle に入る。
- battle 中に berry が消費される。
- battle 中は berry が消費済みとして扱われる。
- battle 終了後に同じ party slot の held item が元の berry に戻る。
- non-berry single-use held item の既存 restore も壊れていないことを確認する。
- skipped link / facility cases は test_plan.md に残す。
```

## Catalog Runtime Note

Catalog / unlimited-assignment runtime is now implemented on
`feature/held-item-catalog-current-master-20260519`. It owns Party / Bag /
Storage quantity-drift helpers as a separate branch from battle-end restore.
Use `implementation.md` and `test_plan.md` for the current handoff.
