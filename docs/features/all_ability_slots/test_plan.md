# All Ability Slots Runtime Test Plan

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-23 |
| Baseline | `master` `de310ef9eb`; upstream `expansion/1.15.2-96-gde310ef9eb` |
| Code status | Docs-only test plan |
| Provenance | Local project overlay |

## Docs-Only Validation

For this investigation branch:

- `rtk git diff --check`
- `rtk mdbook build docs`
- `rtk git diff --name-only master..HEAD` must contain only Markdown docs.

No ROM runtime validation is required for this docs-only branch.

## Runtime Validation Gate

When implementation starts on a feature branch, run:

- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- `rtk make -j16 -O check`
- focused `TESTS=...` checks for new all-active ability tests
- one focused mGBA Live validation route when available

Record mGBA Live evidence or the exact failure in this file before push.

## Required Focused Tests

| Area | Required tests |
|---|---|
| Ability set builder | Species with three distinct abilities, duplicate abilities, hidden `ABILITY_NONE`, normal slot `ABILITY_NONE`, and form species. |
| Predicate checks | A battler has Magic Guard plus another ability; Soundproof plus another ability; Sticky Hold plus another ability. Each predicate must work without needing to be the primary slot. |
| Switch-in triggers | Two or three switch-in abilities trigger in deterministic slot order. Duplicate slots trigger once. |
| Field presence | Neutralizing Gas, Unnerve, Ruin abilities, and weather / terrain setters work when not in primary slot. |
| Suppression / bypass | Gastro Acid, Neutralizing Gas, Mold Breaker, Ability Shield, and `cantBeSuppressed` combinations. |
| Ability-changing moves | Trace, Role Play, Skill Swap, Entrainment or Worry Seed, and Receiver with the MVP one-ability override policy. |
| Form changes | Mega Evolution, Primal Reversion, Ultra Burst, Weather forms, Stance Change, Disguise, Ice Face, and any ability-gated form change. |
| AI | Damage / switch decisions that depend on immunity, trapping, priority, speed, and Magic Guard-style secondary damage. |
| Items | Ability Capsule / Patch fail or apply chosen new policy under all-active mode, and retain upstream behavior when disabled. |
| Summary UI | Summary shows all active ability names and lets the player inspect descriptions without text overflow. |

## Candidate Manual Checks

- Start a debug battle with a species that has three non-empty abilities and
  confirm all relevant abilities activate.
- Open Summary from party and from battle-adjacent flows, then inspect the
  active ability list.
- Try Ability Capsule and Ability Patch in all-active mode and confirm the
  chosen message / behavior is clear.
- Mega Evolve a Pokemon whose target form has a different ability table and
  confirm the post-form active set matches the current species policy.
- In a double battle, verify ability popup order and that the player is not
  trapped or immune incorrectly due to a missed secondary ability.

## Known Gaps Until Implementation

- No source tests exist yet for all-active ability sets.
- The current test runner's `Ability(...)` syntax forces one ability and will
  need either compatibility behavior or a new helper for natural all-slot tests.
- mGBA Live validation cannot confirm this docs-only branch because there is no
  runtime behavior change.
