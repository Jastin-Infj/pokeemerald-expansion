# All Ability Slots Ability Audit

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-24 |
| Branch | `feature/all-ability-slots-runtime-20260523` |
| Baseline | `master` `0407f6daf7` |
| Code status | Runtime implementation validated locally |
| Provenance | Local project overlay |

## Audit Method

This audit is source-driven. The pass searched for representative-only ability
patterns in battle code, especially direct `GetBattlerAbility() == ABILITY_*`,
local `ability == ABILITY_*`, and `gBattleMons[].ability` checks in runtime
paths. Each hit was triaged into one of these groups:

- already covered by `AbilityBattleEffects()` all-slot iteration;
- already covered by `BattlerHasAbility()`, `IsAbilityAndRecord()`, or an
  all-slot-aware calc helper;
- converted in this branch because it changes battle behavior;
- intentionally left representative-only because it is AI prediction,
  off-field / party behavior, UI display, or an unresolved policy edge.

## Additional 2026-05-24 Sweep

The follow-up after the R-T modifier pass converted more direct checks that
were still representative-only:

- `Gorilla Tactics` move-choice locking now records and limits moves when the
  ability is in a non-representative slot.
- End-turn damaging weather immunity now sees hidden-slot `Sand Veil`,
  `Sand Force`, `Sand Rush`, `Snow Cloak`, and `Overcoat`.
- End-turn poison / burn handlers now see hidden-slot `Poison Heal` and
  `Heatproof`, including the correct `Poison Heal` popup.
- Yawn's end-turn sleep application now sees hidden-slot `Vital Spirit` and
  `Insomnia`.
- Berserk Gene confusion prevention now sees hidden-slot `Own Tempo`.
- King's Rock / Razor Fang flinch chance now sees hidden-slot `Serene Grace`
  and `Stench`.
- Berry and consumable-item paths now see hidden-slot `Ripen` for HP, PP, and
  stat berries.
- Stat-change runtime now sees hidden-slot `Contrary`, `Simple`, `Clear Body`,
  `White Smoke`, `Full Metal Body`, `Keen Eye`, `Mind's Eye`, `Illuminate`,
  `Hyper Cutter`, `Big Pecks`, and `Mirror Armor`.
- `Protean` / `Libero` type changes now use the hidden slot and bind the popup
  to the actual ability.
- `Booster Energy` and paradox reset paths now see hidden-slot
  `Protosynthesis` and `Quark Drive`.
- Wind-move scripts now see hidden-slot `Wind Rider` and `Wind Power`.
- `Soul-Heart`, `Suction Cups`, OHKO `Sturdy`, `Quick Feet` paralysis speed,
  `Mummy` / `Lingering Aroma` contact guards, `Emergency Exit` / `Wimp Out`,
  and `Serene Grace` secondary-effect chance now use all-slot-aware checks.
- Primal weather persistence checks now see hidden-slot `Desolate Land`,
  `Primordial Sea`, and `Delta Stream`.

Focused tests were added for hidden-slot `Gorilla Tactics`, `Poison Heal`,
`Heatproof`, `Protean`, `Clear Body`, and `Contrary`.

## Full Ability Scan Status

The follow-up review also re-ran a broad direct-reference scan across battle,
Pokemon, party, Summary, and include files. The remaining direct references are
not treated as automatically wrong; they fall into one of these buckets:

- Representative operation-slot state: ability-changing effects, copied ability
  storage, Gastro Acid / Ability Shield suppression state, and battle message
  identity still need a single slot as the mutation target.
- AI prediction and switch-scoring caches: runtime behavior is authoritative,
  but AI can still reason from a known representative ability for performance
  and prediction quality.
- Display / form / animation paths: `Disguise`, `Ice Face`, `Illusion`, Summary
  rendering, and related visual helpers are only broadened when a runtime result
  mismatch is demonstrated.
- Off-field party and reward helpers: field-lead behavior, party-list Heal Bell
  `Soundproof`, Pickup, and Honey Gather remain upstream single-ability policy.
- Already-wrapped helper calls: many remaining call sites pass
  `GetBattlerAbility()` into helpers such as `IsAbilityAndRecord()`,
  `BattlerHasAbilityForCalc()`, `BattlerHasAdditionalAbility()`,
  `CanBePoisoned()`, `CanBeBurned()`, and stat / accuracy / damage calculators,
  which are now all-slot-aware where battle results require it.

## Debug Double Placement Note

Manual double-battle testing raised a possible first-slot placement glitch. The
all-ability debug route was missing a player party count refresh after replacing
the debug party. This branch now calls `CalculatePlayerPartyCount()` after
building the all-ability debug player party.

After that fix, mGBA Live session `all-ability-double-count-20260524` selected
`T Partner Mods` and reached the double-battle command menu with coherent
first / second slot placement on both sides. If a similar placement issue still
appears later, treat it as likely upstream debug / double sendout rendering
behavior rather than all-slot ability logic. The feature docs keep it as a
manual visual risk, not as a blocker for the battle mechanics tests.

## Remaining Representative-Only / Policy Areas

These areas are still intentionally not blanket-converted:

- AI prediction caches and switch scoring still mostly reason from one known
  ability. Runtime behavior is authoritative; AI quality remains follow-up
  balance / performance work.
- Off-field party and field-lead ability behavior remains upstream
  single-ability behavior unless a later feature opts into all-slot field
  rules.
- Heal Bell / Aromatherapy party-list `Soundproof` checks can only see stored
  off-field ability data for non-active party members.
- Ability Shield, Gastro Acid, and some suppression / unsuppression display
  paths still need a dedicated multi-slot policy review before broadening every
  script branch.
- Form-display and animation-only paths such as `Disguise`, `Ice Face`,
  `Illusion`, and related visual helpers are left conservative unless runtime
  tests show a battle-result mismatch.
- Post-battle reward helpers such as Pickup / Honey Gather are not treated as
  facility battle-runtime blockers in this branch.

## Validation

- `rtk make -j16 -O check TESTS='All Ability Slots'` passed after this audit
  sweep with the existing RWX linker warning.
