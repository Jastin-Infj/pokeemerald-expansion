# Battle Item Restore Policy Impact Scope

## Summary

The shipped branch changes battle-end restoration only. It does not make held
items unconsumable during battle.

This separation is the main compatibility rule:

| Timing | Policy |
|---|---|
| During battle | Keep existing item removal, `usedHeldItem`, `canPickupItem`, `ateBerry`, `isKnockedOff`, and Unburden state changes. |
| Battle end | Restore the original player party held item from `itemLost[B_SIDE_PLAYER][slot].originalItem` when config policy allows it. |

That means mechanics that depend on consumed-item state should keep working
during battle. The remaining risk is final ownership at battle end when an item
was stolen, swapped, bestowed, removed, or replaced during battle.

## Primary State Fields

| State | Role | Restore policy impact |
|---|---|---|
| `itemLost[side][slot].originalItem` | Battle-start source of truth for party held item restore. | Used only at battle end. |
| `itemLost[side][slot].stolen` | Marks original player item as stolen / removed for existing trainer item return behavior. | Existing non-berry behavior is preserved; berries can now be included when enabled. |
| `GetBattlerPartyState(battler)->usedHeldItem` | Stores consumed held item for Recycle, Pickup, Harvest, Cud Chew, berry recycle effects, etc. | Must not be cleared early by this feature. |
| `gBattleStruct->battlerState[battler].canPickupItem` | Marks whether Pickup can target the battler's last used item. | Must remain a battle-time mechanic. |
| `GetBattlerPartyState(battler)->ateBerry` | Allows Belch after berry consumption. | Must remain independent of battle-end restore. |
| `GetBattlerPartyState(battler)->isKnockedOff` | Tracks Gen 3-style Knock Off usability/removal. | Battle-end restore must not be confused with mid-battle item usability. |

## Mechanics Matrix

| Mechanic | Code path observed | Expected with berry restore on | Follow-up needed |
|---|---|---|---|
| Natural Gift | `EFFECT_NATURAL_GIFT` removes berry, sets `usedHeldItem`, sets `canPickupItem`, and triggers Unburden. | Battle still consumes the berry; battle end restores original berry if policy allows. | Add focused Natural Gift battle-end test. |
| Fling | `EFFECT_FLING` and related scripts remove held item outside normal `Cmd_removeitem` path. | Battle should consume item normally; original item restore depends on final policy. | Add Fling test for berry and non-berry original items. |
| Recycle | `Cmd_tryrecycleitem` restores from `usedHeldItem` and clears it. | Recycle remains a battle-time restore; battle-end restore should not interfere. | Add test where Recycle restores berry during battle, then battle-end state is accepted. |
| Pickup | `ABILITY_PICKUP` uses target `usedHeldItem` and `canPickupItem`. | Pickup should still be able to pick up eligible consumed items during battle. | Add double battle Pickup regression if this feature expands. |
| Harvest | `ABILITY_HARVEST` checks own `usedHeldItem` is Berry pocket. | Harvest should still restore consumed berry during battle. | Existing Harvest tests cover ability; add battle-end policy assertion only if behavior changes. |
| Cud Chew | `ABILITY_CUD_CHEW` delays and reuses `usedHeldItem`, then clears it. | Cud Chew should still reuse berry next turn; battle-end restore is separate. | Add battle-end assertion if Cud Chew holder is in player party. |
| G-Max Replenish / berry recycle effects | `BS_TryRecycleBerry` restores target berry from `usedHeldItem`. | Target berry restore remains battle-time behavior. | Add target-side test only if Dynamax/Replenish policy is used in this branch. |
| Belch | `IsBelchPreventingMove` checks `ateBerry`. | Belch permission must remain based on berry consumption, not battle-end restoration. | Add Belch after consumed/restored berry regression if movesets use it. |
| Bug Bite / Pluck | Move effects eat target berry and bypass the normal `Cmd_removeitem` current item removal path. | Battle-time berry eating should remain unchanged. | Clarify final ownership when player berry is eaten by an opponent. |
| Knock Off | `EFFECT_KNOCK_OFF` removes item or marks `isKnockedOff`; Gen 5+ also marks `itemLost[side][slot].stolen`. | Existing trainer item return behavior remains; berry restore now follows enabled policy. | Confirm intended generation/config behavior for berries knocked off in wild vs trainer battles. |
| Thief / Covet / Pickpocket / Magician | `StealTargetItem` and `TrySaveExchangedItem` transfer item and may mark original player item stolen. | Battle-time stealing remains; battle-end original restore may overwrite final ownership by policy. | Decide whether obtained item should persist in wild/trainer/facility cases. |
| Trick / Switcheroo | Swap path calls `TrySaveExchangedItem` for old items. | Battle-time swap remains; battle-end original restore can override the swap for player party. | Needs explicit design for competitive restore vs RPG item ownership. |
| Bestow / Symbiosis | `BestowItem`, `TrySymbiosis`, and `BS_TrySymbiosis` move items between battlers. | Battle-time transfer remains; battle-end original restore policy may supersede final held item. | Add scenario tests before enabling broader facility/challenge rules. |
| Corrosive Gas | `Cmd_removeitem` avoids storing `usedHeldItem` for `EFFECT_CORROSIVE_GAS`. | Corroded items should not be restored by battle-time mechanics. | Confirm whether battle-end original restore should intentionally bypass or respect this exception. |
| Air Balloon | `Cmd_removeitem` avoids storing popped Air Balloon as `usedHeldItem`. | Existing non-restorable battle-time behavior remains. | Confirm battle-end restore exception if Air Balloon restore policy is expanded. |
| Unburden | Item removal calls `CheckSetUnburden`; Recycle/Bestow can clear active state. | Battle-time speed behavior remains based on current item loss/restoration. | Add regression only if battle-end restore happens before final battle callbacks used by tests. |

## Config On / Off

| Config state | Expected behavior |
|---|---|
| `B_RESTORE_HELD_BATTLE_BERRIES == FALSE` | Existing non-berry restore behavior remains. Berry pocket items stay excluded from battle-end restore. |
| `B_RESTORE_HELD_BATTLE_BERRIES == TRUE` | Player party original held berries can be restored at battle end, using the same original item tracking as existing held item restore. |

The config does not change battle-time consumption. If a future branch wants
items to be unconsumable during battle, that is a separate rule and must not be
implemented by clearing or bypassing `usedHeldItem`.

## Next Tasks

- Add focused tests for Natural Gift, Recycle, and Pickup interactions if this
  policy is expanded beyond the current Oran Berry restore proof.
- Decide final ownership policy for Thief/Covet, Trick/Switcheroo, Bestow,
  Symbiosis, Pickpocket, and Magician in trainer, wild, facility, and challenge
  contexts.
- Decide whether Corrosive Gas and Air Balloon should remain non-restorable at
  battle end, or whether competitive-style restore intentionally restores the
  original item after battle.
- Revisit Frontier / Factory / Tent / Battle Pyramid before enabling this
  policy for facility-specific item rules.
- When battle selection or Champions Challenge runtime is added, route item
  restore through the challenge/facility policy instead of applying normal
  trainer restore twice.
