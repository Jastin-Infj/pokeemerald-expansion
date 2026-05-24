# All Ability Slots Runtime Handoff

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-24 |
| Implementation PR | #60 `[codex] Implement all ability slots runtime` |
| Implementation branch | `feature/all-ability-slots-runtime-20260523` |
| Baseline | `master` `0407f6daf7` |
| Code status | Runtime implementation validated locally; keep source off `master` |
| Master policy | Docs-only handoff branch only |

## Handoff Summary

PR #60 is the implementation shelf for the All Ability Slots Runtime. It enables
Pokemon to use all non-empty species ability slots in battle while keeping
`abilityNum` as the saved representative / operation slot. The branch is guarded
by `B_ALL_ABILITY_SLOTS`, with separate balance toggles for non-representative
Mold Breaker-family bypass and Neutralizing Gas suppression. The feature branch
currently defaults `B_ALL_ABILITY_SLOTS` to `TRUE`.

Summary UI now shows the selected active slot label plus ability name on the
Info page, and the lower ability area shows that slot's description. `L` / `R`
cycles to the next non-empty slot, so full `1/2/3` species and sparse `1/3`
species use the same flow. This cycling is separately guarded by
`P_SUMMARY_SCREEN_ALL_ABILITY_SLOT_SWITCH`, default `TRUE`, so later integration
can disable the UI selector without disabling the battle rule.

Do not merge the implementation branch into `master` while the project keeps
`master` docs-only. Use a docs-only branch from current `master` for this file
and the related All Ability Slots docs, then keep source / tests / config on
PR #60 or a later integration branch.

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

Checked against open PRs on 2026-05-24:

| PR | Branch | Overlap / action |
|---|---|---|
| #47 Battle Item Restore | `feature/battle-item-restore-current-master-20260519` | Direct battle-core overlap in `include/config/battle.h`, `src/battle_main.c`, and `src/battle_util.c`. Resolve before treating either branch as integration-ready. Re-run item restore tests plus All Ability Slots focused tests after conflict resolution. |
| #48 Held Item Ownership Tokens | `feature/held-item-catalog-current-master-20260519` | Direct overlap in `src/party_menu.c`; docs/manual ledger also overlap. Recheck Ability Capsule / Patch failure text and held item assignment menus after merge. |
| #51 Scout Selection Runtime | `feature/scout-selection-runtime-20260520` | Direct overlap in `src/debug.c` and docs registry. Runtime systems are mostly separate, but debug menus and generated party/scout Pokemon should be rechecked because All Ability Slots activates all species slots without changing `abilityNum`. |
| #54 Party / Status UI Overhaul | `feature/party-status-ui-overhaul-20260521` | Direct overlap in `src/party_menu.c` and party UI docs. Recheck Summary entry / return and party item callbacks after adopting both. |
| #57 Friendly Shop Pokemon Vendor | `feature/global-no-evolution-20260523` | Direct overlap in `include/pokemon.h`, `src/pokemon.c`, `src/party_menu.c`, `src/pokemon_summary_screen.c`, and `test/pokemon.c`. This is the largest non-battle conflict because both branches alter Summary / locked Pokemon display and Pokemon metadata helpers. |

## Validation Snapshot

Latest local validation on the implementation branch:

- `rtk git diff --check`
- `rtk make -j16 -O check TESTS='All Ability Slots'`
- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- temporary `P_SUMMARY_SCREEN_ALL_ABILITY_SLOT_SWITCH FALSE`:
  `rtk make -j16 -O debug`
- `rtk mdbook build docs`
- mGBA Live `all-ability-final-true-20260524`: final `TRUE` build,
  Pattern A Recoil route, `Magic Guard` prevented recoil.
- mGBA Live `all-ability-unified-selector-20260524`: Nidoqueen `1/2/3` and
  Bastiodon `1/3` Summary selector behavior confirmed; cleanup returned `[]`.

Known warning / caveat:

- `arm-none-eabi-ld` reports the existing RWX segment warning.
- `mdbook` reports existing warnings for missing root `CHANGELOG.md`, existing
  `CREDITS.md` `</img>`, and large search index.
- Full `rtk make -j16 -O check` is not a green gate while
  `B_ALL_ABILITY_SLOTS` defaults to `TRUE`, because upstream tests still assert
  single-ability default behavior. The focused All Ability Slots suite is the
  green logic gate for this branch.

## Integration Checklist

Before a future source integration:

- decide whether `B_ALL_ABILITY_SLOTS` remains global `TRUE`, becomes opt-in, or
  is wrapped by a Champions facility runtime rule;
- decide merge order with #47 and #57 first, because they overlap the highest-risk
  battle / Pokemon / Summary files;
- preserve `abilityNum` as the saved representative slot unless a separate save
  migration is explicitly approved;
- re-run `All Ability Slots`, AI thinking-time, `all`, `debug`, and a focused
  mGBA Summary + debug battle route after conflicts are resolved;
- recheck Summary display if Party / Status UI Overhaul is adopted before or with
  this branch;
- keep balance tuning, species ability table edits, and ability buffs / nerfs in
  a later balance feature.
