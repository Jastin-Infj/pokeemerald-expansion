# Battle Item Restore Policy Test Plan

## Unit / Battle Tests

追加したい test:

- Oran Berry / Sitrus Berry is consumed during battle but restored after battle under the new policy.
- Leppa Berry can still be restored by Recycle during battle.
- Harvest still restores a consumed berry during battle.
- Cud Chew still reuses the consumed berry on the next turn.
- Pickup can still pick up another battler's consumed item during battle.
- G-Max Replenish still restores target berries during battle.
- Belch remains usable after eating a berry, including after Recycle.
- Natural Gift consumes the berry during battle and battle-end policy restores the original item.
- Knock Off / Corrosive Gas / Air Balloon keep their intended non-restorable behavior if that policy is retained.

## Scenario Tests

- Trainer battle win with consumed berry.
- Trainer battle loss with `B_FLAG_NO_WHITEOUT`.
- Wild battle with consumed berry.
- Battle Frontier / Factory / Tent if the policy is enabled there.
- Battle selection enabled, selected subset consumes berry, original party slot receives restored item.

## Manual Checks

- Bag quantity is not changed by held item restore.
- Held item icon in party menu is correct after battle.
- Summary screen held item is correct after battle.
- Link / recorded battle behavior is explicitly accepted or excluded.
