# All Ability Slots Runtime Handoff

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-31 |
| Implementation PR | #60 `[codex] Implement all ability slots runtime` |
| Implementation branch | `feature/all-ability-slots-runtime-20260523` |
| Integration branch | `integration/runtime-dev-20260529` |
| Baseline | `master` `4e48ff993f` |
| Code status | Adopted into runtime integration; keep source off `master` |
| Master policy | Docs / Lua-only handoff branch only |

## Handoff Summary

PR #60 is the implementation shelf for the All Ability Slots Runtime. It enables
Pokemon to use all non-empty species ability slots in battle while keeping
`abilityNum` as the saved representative / operation slot. The branch is guarded
by `B_ALL_ABILITY_SLOTS`, with separate balance toggles for non-representative
Mold Breaker-family bypass and Neutralizing Gas suppression. The original
feature branch defaulted `B_ALL_ABILITY_SLOTS` to `TRUE`; the runtime
integration branch now defaults it to `FALSE` and adds a per-save runtime
override in Debug -> `Flags/Vars` -> `All Abilities`. The override cycles
`DEFAULT`, `OFF`, and `ON`, so normal battles can use all active ability slots
without rebuilding while full upstream-style validation remains single-ability
by default.

Summary UI now shows the selected active slot label plus ability name on the
Info page, and the lower ability area shows that slot's description. `L` / `R`
cycles to the next non-empty slot, so full `1/2/3` species and sparse `1/3`
species use the same flow. This cycling is separately guarded by
`P_SUMMARY_SCREEN_ALL_ABILITY_SLOT_SWITCH`, default `TRUE`, so later integration
can disable the UI selector without disabling the battle rule.

Do not merge the implementation branch into `master` while the project keeps
`master` docs / Lua-only. Keep source / tests / config on PR #60 or the runtime
integration branch, and use a docs-only branch from current `master` for this
file and the related All Ability Slots docs.

## Implementation Diff Areas

The runtime branch touches these source-like areas:

- battle ability set helpers and direct predicates:
  `include/battle.h`, `include/battle_util.h`, `src/battle_util.c`,
  `src/battle_main.c`, `src/battle_switch_in.c`, `src/battle_move_resolution.c`,
  `src/battle_end_turn.c`, `src/battle_hold_effects.c`,
  `src/battle_script_commands.c`, `src/battle_terastal.c`, and
  `src/battle_util2.c`;
- config and generated config mapping:
  `include/config/battle.h`, `include/config/summary_screen.h`,
  `include/config/ai.h`, and `include/constants/generational_changes.h`;
- Pokemon / UI / debug surfaces:
  `include/pokemon.h`, `include/debug.h`, `src/pokemon.c`, `src/party_menu.c`,
  `src/pokemon_summary_screen.c`, `src/debug.c`, and
  `src/data/debug_trainers.party`;
- tests:
  `test/battle/ability/all_ability_slots.c`,
  `test/battle/ability/poison_puppeteer.c`, and `test/pokemon.c`.

## Current Dependencies

Checked against the runtime integration order on 2026-05-29:

| PR | Branch | Overlap / action |
|---|---|---|
| #47 Battle Item Restore | `feature/battle-item-restore-current-master-20260519` | Resolved before #60 adoption. Rechecked `TESTS='Battle item restore'` after #60 and during the default-`FALSE` integration point; the 2026-05-31 follow-up keeps the integration default `FALSE` and adds the per-save runtime override. |
| #48 Held Item Ownership Tokens | `feature/held-item-catalog-current-master-20260519` | Resolved before #60 adoption. `src/party_menu.c` retains held-item catalog assignment hooks; future Ability Capsule / Patch UX changes should still recheck Give / Take paths. |
| #51 Scout Selection Runtime | `feature/scout-selection-runtime-20260520` | Resolved before #60 adoption. `src/debug.c` menu entries coexist; Scout remains `Script 2` and All Ability debug battles live under `Party` -> `All Ability...`. |
| #54 Party / Status UI Overhaul | `feature/party-status-ui-overhaul-20260521` | Resolved before #60 adoption. Summary entry / return remains the key manual UI regression point. |
| #57 Friendly Shop Pokemon Vendor | `feature/global-no-evolution-20260523` | Resolved before #60 adoption. Pokemon metadata helpers, locked / sealed Summary display, and vendor tests still pass after #60. |

## Validation Snapshot

Latest local validation on `integration/runtime-dev-20260529`:

- `rtk git diff --check`
- `rtk git diff --cached --check`
- `rtk make -j16 -O check TESTS='All Ability Slots'`
- `rtk make -j16 -O check TESTS='AI thinking time'`
- `rtk make -j16 -O check TESTS='Battle item restore'`
- `rtk make -j16 -O check TESTS=test/pokemon_vendor.c`
- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- earlier default-`FALSE` integration run: `rtk make -j16 -O check`
- `rtk mdbook build docs`
- mGBA Live `integration-all-ability-optin-smoke`: default-`FALSE` integration
  build, `Party` -> `All Ability...` -> `A Recoil Battle`, `Double-Edge`;
  Clefable stayed at `317/317`, confirming the debug override still enables
  non-representative `Magic Guard`. Screenshots:
  `/tmp/integration-all-ability-optin-submenu.png`,
  `/tmp/integration-all-ability-optin-move-menu.png`, and
  `/tmp/integration-all-ability-optin-after-double-edge.png`. Cleanup returned
  `[]`.

Known warning / caveat:

- `arm-none-eabi-ld` reports the existing RWX segment warning.
- `mdbook` reports existing warnings for missing root `CHANGELOG.md`, existing
  `CREDITS.md` `</img>`, and large search index.
- The integration branch uses the default-`FALSE` build plus a runtime override.
  Full `check` stays green under `DEFAULT`; focused All Ability tests and debug
  battle routes still force the mode on for validation.
- 2026-05-31 runtime-toggle validation passes focused All Ability, `all`,
  `debug`, full `check`, docs, and mGBA Live boot.
- 2026-05-31 KO-popup follow-up fixes non-representative move-end KO ability
  popup binding for `Moxie` / neigh / `Beast Boost` / `Battle Bond`, adds the
  hidden-slot `Moxie` regression, and adds debug Pattern U `Moxie Popup`.

## Integration Checklist

Before a future source integration:

- if Champions needs the rule globally during a facility run, add a later
  runtime toggle around the current config/default rather than widening Pokemon
  save data;
- preserve the #47 / #48 / #54 / #51 / #57 conflict resolutions when rebasing
  or refreshing the integration branch;
- preserve `abilityNum` as the saved representative slot unless a separate save
  migration is explicitly approved;
- re-run `All Ability Slots`, AI thinking-time, `all`, `debug`, and a focused
  mGBA Summary + debug battle route after conflicts are resolved;
- recheck Summary display if Party / Status UI Overhaul is adopted before or with
  this branch;
- keep balance tuning, species ability table edits, and ability buffs / nerfs in
  a later balance feature.
