# All Ability Slots Runtime Risks

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-23 |
| Baseline | `master` `de310ef9eb`; upstream `expansion/1.15.2-96-gde310ef9eb` |
| Code status | Docs-only risk assessment |
| Provenance | Local project overlay |

## Risks

| Risk | Severity | Impact | Mitigation |
|---|---|---|---|
| Singular API misuse | High | Leaving equality checks on `GetBattlerAbility()` will make only one slot work, creating inconsistent behavior. | Add set helpers and migrate by call-site family. Keep legacy API only for primary display / compatibility. |
| Trigger spam | High | A Pokemon with multiple switch-in abilities can produce too many popups, weather changes, stat changes, or form checks. | Stable slot order, duplicate suppression, and tests for multiple switch-in abilities. Consider grouping later. |
| Ability-changing moves are ambiguous | High | Trace, Role Play, Doodle, Skill Swap, Entrainment, Worry Seed, Simple Beam, Receiver, and Power of Alchemy currently copy or overwrite one ability. | Use a conservative one-ability temporary override set for MVP; document any user-facing divergence. |
| Suppression rules become wrong | High | Gastro Acid, Neutralizing Gas, Mold Breaker, Ability Shield, and `cantBeSuppressed` can accidentally suppress too much or too little. | Apply suppression and bypass per ability. Add tests with one suppressible plus one unsuppressable ability. |
| AI underestimates threats | High | AI caches one ability and may ignore immunities, trapping, priority, speed, or damage modifiers from other active slots. | Add AI ability-set helpers and focused tests for trapping, Magic Guard, priority, speed, and immunity decisions. |
| Mega / form behavior changes | High | Mega overlay can create up to six active ability entries before dedupe, and ability-gated form changes currently receive one ability argument. | Implement explicit base-plus-Mega overlay logic, dedupe, and tests. Decide Primal / Ultra / non-Mega form policy separately. |
| Ability Capsule / Patch confusion | Medium | Items appear to work but no longer change battle behavior. | Disable cleanly under all-active mode unless a display-slot or hidden-unlock policy is chosen. |
| Summary / party UI overflow | Medium | Three names and descriptions do not fit the current one-ability Summary layout. | Make Summary the first complete display surface with a compact list and selected description. Defer party cards if needed. |
| Field ability behavior drift | Medium | Outside-battle lead ability checks will intentionally stay single-ability while battles use all slots. | Document battle-only MVP and leave global field behavior to a separate feature. |
| Test runner assumptions | Medium | Existing test DSL and forced ability paths assume one ability. | Keep forced ability as a one-ability override in tests, then add explicit all-active test helpers later. |
| Balance explosion | High | Some species combinations become far stronger than expected, especially Neutralizing Gas-style suppression plus additional abilities. | Treat runtime implementation as mechanics first. Follow with a dedicated ability / trainer balance branch. |
| Upstream conflict risk | Medium | Battle utility, script commands, AI, Summary, and party item-use files are high-churn upstream areas. | Keep branch small by phases and update docs before broad conversions. |

## Blockers Before Runtime Work

- Decide Ability Capsule / Patch behavior under all-active mode.
- Decide temporary ability-copy / overwrite semantics.
- Decide whether Primal Reversion, Ultra Burst, and non-Mega form changes use
  Mega-style overlay or current-species-only ability sets.

## Accepted First-Branch Risks

- It is acceptable for the first branch to implement Summary as the only fully
  complete multi-ability UI surface, as long as battle behavior is tested.
- It is acceptable for forced test-runner abilities to remain one-ability
  overrides.
- It is acceptable to leave some rare ability interactions as documented gaps if
  the major predicate, trigger, suppression, and form-change categories are
  covered by tests.

## Future Balance Risks

- Slot 0 + slot 1 + hidden combinations can stack several damage multipliers.
- Abilities that remove weaknesses, block status, or trap opponents become much
  stronger when combined with offensive abilities.
- Mega forms with multiple filled slots may become the largest balance swing.
- Neutralizing Gas and similar field-wide suppression abilities may become format
  defining if the holder also keeps offensive or defensive abilities.
- Species with duplicate or `ABILITY_NONE` slots will be less affected, which can
  widen the gap between species.
- Ability and trainer balance should be tracked as a separate feature so the
  mechanics branch can remain focused.
