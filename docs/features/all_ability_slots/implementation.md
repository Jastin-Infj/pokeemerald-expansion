# All Ability Slots Runtime Implementation

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-24 |
| Branch | `feature/all-ability-slots-runtime-20260523` |
| Baseline | `master` `0407f6daf7` |
| Code status | Runtime implementation validated locally |
| Provenance | Local project overlay |

## Dependency / Merge Notes

There is no known runtime blocker left for this branch after the 2026-05-24
ability audit. The remaining risk is merge ordering with other staged feature
branches, not an unmet all-ability requirement.

Open PR file-level conflicts observed on 2026-05-24:

- #47 battle item restore overlaps `include/config/battle.h`,
  `src/battle_main.c`, and `src/battle_util.c`.
- #48 held item token work overlaps docs registry files and `src/party_menu.c`.
- #51 scout selection overlaps docs registry files and `src/debug.c`.
- #54 party/status UI overlaps docs registry files and `src/party_menu.c`.
- #57 friendly shop Pokemon vendor overlaps docs registry files,
  `include/pokemon.h`, `src/party_menu.c`, `src/pokemon.c`,
  `src/pokemon_summary_screen.c`, and `test/pokemon.c`.

The safest integration path is to keep this branch as the implementation
candidate, then create a separate docs-only branch from current `master` for
master-facing status / handoff. Do not merge this implementation branch into
`master` while the project policy keeps master docs-only.

## Implementation Summary

This branch adds a guarded battle runtime mode behind `B_ALL_ABILITY_SLOTS`.
The implementation branch currently sets the config to `TRUE`, so the feature
PR build uses all ability slots by default. The focused tests still include a
`B_ALL_ABILITY_SLOTS FALSE` regression to prove upstream single-ability
behavior can be restored by config.

A debug-menu override exists only for validation: `Party` -> `All Ability...`
contains one direct trainer-battle entry per Pattern A-T. Each entry forces
`gAllAbilitySlotsBattle` for the next debug trainer battle and then clears the
override when battle setup consumes it. This allows mGBA/manual verification
from a debug ROM even when a temporary local build has the global config off.

Core behavior:

- `abilityNum` remains the representative / operation slot for save and party
  compatibility.
- Battle ability evaluation can enumerate all non-empty direct species ability
  slots in slot order.
- Duplicate ability entries are deduped before trigger / predicate evaluation.
- Slot-local runtime overrides are stored in
  `gBattleStruct->abilitySlotOverrides[battler][slot]`.
- `GetBattlerAbility()` remains a single representative ability API for legacy
  compatibility.
- New helpers cover the all-slot path:
  `GetSpeciesAbilitySet`, `GetMonAbilitySet`, `GetBattlerAbilitySlot`,
  `GetBattlerAbilitySet`, `BattlerHasAbility`, and
  `IsBattlerAbilityActive`.
- `AbilityBattleEffects()` iterates the effective active ability set once per
  trigger when all-slot mode is enabled, so switch-in / move-end / weather /
  terrain effect families that already route through it can reuse existing
  ability cases.
- Ability-changing effects are slot-local. Skill Swap, Trace, Role Play /
  Doodle, Entrainment, Worry Seed / Simple Beam, and Receiver / Power of
  Alchemy operate on the representative operation slot instead of replacing the
  whole active set.
- `B_ALL_ABILITY_SLOTS_MOLD_BREAKER` and
  `B_ALL_ABILITY_SLOTS_NEUTRALIZING_GAS` are separate balance knobs. They let
  the mechanics branch support non-representative Mold Breaker-family bypass
  and Neutralizing Gas suppression, while preserving an easy future switch if
  those categories prove too strong for a facility ruleset.

Additional battle predicate coverage was added for common direct checks that do
not naturally pass through `AbilityBattleEffects`, including Magic Guard,
Pressure, Truant, Soundproof, Sticky Hold, Dancer, Run Away, Mycelium Might,
Cheek Pouch, Liquid Ooze, Pickpocket, Mega Launcher, Early Bird, Opportunist,
Ripen, Symbiosis, and trapping abilities.

Move-block / absorption checks now enumerate the defender's active ability set
in all-slot battles. This covers non-representative `Flash Fire`,
`Soundproof`, `Water Absorb`, `Volt Absorb`, stat-raising absorb abilities, and
similar target-side immunity abilities. The selected ability is also propagated
to ability popup and battle-message text so a representative-slot ability such
as `Early Bird` or `Bulletproof` is not displayed when the actual triggering
slot was `Flash Fire` or `Soundproof`.
The all-slot enumeration is limited to real attacker-versus-defender battle
contexts so party / overworld effectiveness helpers that reuse the same helper
do not accidentally consume stale battle state.

Single-target `Lightning Rod` and `Storm Drain` redirection now checks the
target and candidate battlers through `BattlerHasAbility()`, so hidden-slot /
non-representative redirect abilities participate in the same path as
representative abilities. External references describe these abilities as
redirecting single-target Electric / Water moves, and Showdown / Smogon-style
mechanics note that when multiple Pokemon can redirect, the faster Pokemon gets
the move, with ties resolved by ability-active order. The local engine already
selects the lowest turn-order redirector, so the new all-slot path preserves the
existing speed-order behavior while testing the faster candidate case.

Switch-in weather ability scripts now bind the ability popup to the actual
triggering slot before the battle script runs. Pattern D's Ninetales can keep
representative `Flash Fire` while hidden-slot `Drought` starts sun and displays
`Drought`, not the representative Summary ability.

Switch-in and move-block follow-up coverage exposed three additional
representative-slot display / predicate holes:

- non-representative `Intimidate` now binds its switch-in ability popup to the
  actual triggering slot;
- non-representative `Levitate` now becomes the effective immunity ability for
  Ground-type move effectiveness and popup text;
- non-representative `Sticky Hold` now keeps items through Knock Off / steal
  paths while displaying `Sticky Hold`, not the representative ability.

Priority-blocking abilities now use the active ability set in all-slot battles,
so non-representative `Dazzling`, `Queenly Majesty`, and `Armor Tail` can block
priority moves while still reporting the actual blocking ability.

The all-slot dispatcher now preserves the last actually-triggered ability state
when later slots are checked but do not trigger. This fixes switch-in popup
pollution such as Weavile `Pressure / none / Pickpocket`, where `Pressure`
queued the switch-in script but the later inactive `Pickpocket` slot could
overwrite `gLastUsedAbility` before the popup was rendered.

Switch-in scripts that render an ability popup now bind both the triggering
battler and the triggering ability before the script runs. This removes
dependence on stale `gBattlerAbility` state for non-representative entry effects
such as `Unnerve`, `Drizzle`, `Sand Stream`, `Snow Warning`, terrain setters,
`Download`, `Frisk`, `Trace`, `Pressure`, ruin abilities, and stat-raise /
form-change switch-in scripts.

End-turn weather and third-block ability handlers now have all-slot entry
points for non-representative abilities that would otherwise be skipped when
the representative ability is unrelated. This covers `Rain Dish`, `Ice Body`,
and end-turn families such as `Pickup`, `Harvest`, `Moody`, and `Speed Boost`.

Battle speed calculation now consults the active ability set when the caller is
using the battler's representative ability in all-slot mode. Non-representative
weather speed abilities such as `Chlorophyll` therefore affect turn order
without changing `abilityNum`.

The damage / accuracy pass covers the high-risk offensive stacks called out
during manual review:

- `Guts` now suppresses burn's physical damage penalty and applies its Attack
  modifier from a non-representative slot;
- `Hustle` now applies both its physical Attack boost and its physical accuracy
  penalty from a non-representative slot;
- `Iron Fist`, `Sheer Force`, `Technician`, `Tough Claws`, `Strong Jaw`,
  `Sharpness`, `Punk Rock`, and other base-power modifiers now contribute from
  non-representative slots;
- non-representative `Sheer Force` also suppresses secondary effects in the
  real move-effect path.

This validates the Conkeldurr-style `Guts / Sheer Force / Iron Fist` stack and
Durant-style `Swarm / Hustle / Truant` stack without changing the stored
representative slot. The follow-up modifier sweep also covers all-slot
`Adaptability` / Tera STAB, `Thick Fat`, `Solid Rock`, `Multiscale`,
`Friend Guard`, `Plus` / `Minus`, `Gale Wings`, `Skill Link`, `Overcoat`,
`Shield Dust`, `No Guard`, `Heavy Metal`, and `Light Metal` paths.

The added all-slot predicate checks increase the stress AI thinking-time guards
by one frame in the test ROM after the broad modifier sweep. The branch raises
the local ceilings for doubles smart, Steven multi, and Steven multi smart to
the observed passing values while keeping the earlier large regression fixed.

`Trace` needs one extra all-slot guard. After Trace copies an ability, the
operation slot is overwritten and the battle script immediately re-runs
switch-in abilities for that battler. In all-slot mode the natural hidden Trace
slot would otherwise still exist and could recurse. Trace now skips if the
battler already has an overwritten ability, preserving the one-shot switch-in
semantics.

`Klutz` is treated as a passive held-item suppression predicate, not as a popup
ability. External references describe it as disabling the holder's held-item
effects rather than as an announced switch-in or trigger event. In all-slot
mode, non-representative `Klutz` suppresses held-item effects and Gen V+
`Fling`, while still avoiding a `Klutz` popup.

`IsAbilityOnSide()` now ignores absent partner slots in singles by bounding the
partner lookup to `gBattlersCount`. This prevents stale double-battle partner
data, such as Pastel Veil in `gBattleMons[3]`, from blocking status effects in
the next single battle.

The follow-up ability audit converted remaining representative-only behavior
checks found in high-impact runtime paths: `Gorilla Tactics` move locking,
damaging weather immunity, `Poison Heal`, `Heatproof`, Yawn sleep prevention,
`Protean` / `Libero`, `Contrary` / `Simple`, stat-drop prevention abilities,
`Ripen`, `Booster Energy`, `Soul-Heart`, `Suction Cups`, OHKO `Sturdy`,
`Quick Feet`, `Emergency Exit` / `Wimp Out`, and related popup binding.

Remaining concerns are now narrower: AI prediction caches, off-field party /
field ability behavior, Ability Shield / Gastro Acid multi-slot policy,
animation-only paths, form-display side effects, and item / form edge cases
that still intentionally accept one representative ability argument. See
[Ability Audit](ability_audit.md) for the current triage.

## UI / Item Policy

Summary now uses a compact all-slot layout when `B_ALL_ABILITY_SLOTS` is
enabled. It uses direct slot labels `1`, `2`, and `3` instead of a deduped set.
The non-representative slots are printed as a single upper strip. That strip
uses the normal Summary font when it fits, falls back to the small font when the
names would otherwise overflow, and then falls back to showing only the
currently selected slot label plus ability name. The selected slot is marked
with a right-arrow marker in the upper strip. `L` / `R` on the Info page cycles
the selected ability slot, and the lower white ability-detail area shows that
slot's ability description. The single-ability name / description block is still
used when the mode is disabled.

Ability Capsule and Ability Patch fail with the usual "It won't have any
effect" message while all-slot mode is enabled. They still keep upstream
behavior when the mode is disabled.

## Feature / PR Dependency Notes

Dependency here includes other local feature branches, not only function-level
source dependencies.

| Related feature / PR | Branch | Impact |
|---|---|---|
| Friendly Shop Pokemon Vendor / locked Pokemon (#57) | `feature/global-no-evolution-20260523` | Touches `src/pokemon.c`, `src/pokemon_summary_screen.c`, `src/party_menu.c`, `src/pokemon_storage_system.c`, battle setup / message, and Pokemon tests. Merge order will need conflict resolution around Summary, party menu item callbacks, and Pokemon metadata helpers. |
| Party / Status UI Overhaul (#54) | `feature/party-status-ui-overhaul-20260521` | Touches `src/party_menu.c`, `src/data/party_menu.h`, and party layout constants. This branch only makes small item-policy edits in `party_menu.c`, but UI merge should verify menu text and Ability Capsule / Patch failure flow after #54. |
| Scout Selection Runtime (#51) | `feature/scout-selection-runtime-20260520` | Mostly separate runtime, but it adds debug routes and party generation tooling. If scout-generated Pokemon rely on `abilityNum`, all-slot battle behavior will activate all species slots without changing the scout pool format. |
| Held Item Ownership Tokens (#48) | `feature/held-item-catalog-current-master-20260519` | Touches `src/item.c`, `src/item_menu.c`, `src/party_menu.c`, and storage. Merge should verify Ability Capsule / Patch behavior and item menu text after item-policy changes. |
| Battle Item Restore Policy (#47) | `feature/battle-item-restore-current-master-20260519` | Touches `include/config/battle.h`, `src/battle_main.c`, `src/battle_util.c`, and battle item tests. This is the largest battle-core conflict risk because all-slot mode also changes config, battle utility helpers, and run / item effect predicates. |

## Tests Added

- `test/battle/ability/all_ability_slots.c`
  - non-representative Magic Guard prevents recoil;
  - Worry Seed overwrites only one operation slot while Magic Guard remains
    active in another slot;
  - Skill Swap swaps only the matching operation slot;
  - non-representative Flash Fire absorbs Fire moves with the correct popup and
    message ability;
  - non-representative Soundproof blocks sound moves with the correct popup and
    message ability;
  - Houndoom's hidden-slot `Unnerve` displays before the later
    non-representative `Flash Fire` absorption check in the Pattern B shape;
  - non-representative Drought starts sun with the correct popup and message
    ability;
  - non-representative `Drizzle`, `Sand Stream`, and `Snow Warning` show the
    correct switch-in weather popup;
  - non-representative `Electric Surge` shows the correct terrain popup;
  - non-representative `Download` shows the correct stat-raise popup;
  - non-representative `Frisk` shows the correct switch-in popup;
  - non-representative `Trace` shows the correct popup and does not recurse
    after copying the matching operation slot;
  - non-representative `Commander` activates with Dondozo while the
    representative Tatsugiri slot remains `Storm Drain`;
  - non-representative `Rain Dish` and `Ice Body` run their weather end-turn
    recovery with the correct popup;
  - non-representative Mold Breaker-family bypass can be enabled or disabled
    through `B_ALL_ABILITY_SLOTS_MOLD_BREAKER`, including Wonder Guard bypass;
  - non-representative Neutralizing Gas suppression can be enabled or disabled
    through `B_ALL_ABILITY_SLOTS_NEUTRALIZING_GAS`;
  - non-representative `Chlorophyll` affects turn order in sun;
  - non-representative `Pickup` restores a used item at end turn;
  - non-representative Intimidate triggers on switch-in with the correct popup;
  - non-representative Levitate blocks Ground moves with the correct popup;
  - non-representative Storm Drain absorbs Water moves and raises Sp. Atk;
  - non-representative Sticky Hold prevents item removal;
  - non-representative Queenly Majesty blocks priority moves;
  - representative Pressure popup stays stable when later inactive slots such
    as Pickpocket are also active;
  - non-representative Klutz suppresses held-item effects / Gen V+ Fling
    without showing a Klutz popup;
  - Conkeldurr-style `Guts / Sheer Force / Iron Fist` stacks increase damage
    when all-slot mode is enabled;
  - non-representative `Sheer Force` suppresses a damaging move's secondary
    effect;
  - Durant-style non-representative `Hustle` increases physical damage and
    lowers physical accuracy;
  - non-representative `Lightning Rod` and `Storm Drain` redirect single-target
    moves to the faster eligible redirector in doubles.
- `test/pokemon.c`
  - `GetSpeciesAbilitySet` returns three distinct abilities in slot order;
  - repeated ability slots are deduped.
- `test/battle/ability/poison_puppeteer.c`
  - the damaging-move Poison Puppeteer case now forces Poison Sting's secondary
    effect, making the existing test independent of full-suite RNG order.
- Debug trainer route:
  - `src/debug.c` adds `Party` -> `All Ability...`, a submenu with one
    trainer-battle entry per Pattern A-T. Each battle entry rebuilds the player
    party for that pattern before starting the battle, so separate party-only
    setup commands are not needed.
  - `src/data/debug_trainers.party` adds Pattern A `Recoil`
    (Clefable `Cute Charm` plus hidden `Magic Guard`), Pattern B `Flash Fire`
    (Houndoom `Early Bird` plus non-representative `Flash Fire`), Pattern C
    `Soundproof` (Kommo-o `Bulletproof` plus non-representative `Soundproof`),
    Pattern D `Drought` (Ninetales `Flash Fire` plus hidden-slot `Drought`),
    and Pattern E `Skill Swap` (Clefable keeps non-representative
    `Magic Guard` after representative-slot swapping).
  - The expanded route adds Pattern F `Intimidate` (Luxray `Rivalry` plus
    non-representative `Intimidate`), Pattern G `Levitate` (Bronzong
    `Heatproof` plus non-representative `Levitate`), Pattern H `Storm Drain`
    (Gastrodon `Sticky Hold` plus non-representative `Storm Drain`), Pattern I
    `Sticky Hold` (Muk `Stench` plus non-representative `Sticky Hold`), and
    Pattern J `Queenly Majesty` (Tsareena `Leaf Guard` plus
    non-representative `Queenly Majesty`).
  - Pattern K `Commander` adds a double battle with Tatsugiri's representative
    slot forced to `Storm Drain` while natural slot 0 `Commander` still
    activates with Dondozo.
  - Pattern L `Chlorophyll` provides a manual sun / speed-order route using
    Bulbasaur `Overgrow` plus hidden-slot `Chlorophyll`.
  - Pattern M `Trace` provides a manual switch-in popup / copy route using
    Gardevoir `Synchronize` plus non-representative `Trace`.
  - Pattern N `Power Stack` provides a Conkeldurr route for
    `Guts / Sheer Force / Iron Fist`, with `Flame Orb` and punching moves.
  - Pattern O `Hustle` provides a Durant route for non-representative
    `Hustle` damage / accuracy checks.
  - Pattern P `Rod Redirect` provides a doubles route with two Raichu whose
    hidden-slot `Lightning Rod` should redirect to the faster candidate.
  - Pattern Q `Drain Redirect` provides the same doubles shape for Gastrodon's
    non-representative `Storm Drain`.
  - Pattern R `Mod Stack` provides a Toxtricity route for `Punk Rock`, `Plus`,
    and `Technician` damage / sound-modifier checks.
  - Pattern S `Guard Mods` provides a Dragonite route for defensive modifiers
    such as hidden-slot `Multiscale`.
  - Pattern T `Partner Mods` provides a doubles route for partner-side
    `Plus` / `Minus` style modifier checks.
  - Battle entries use the debug all-slot override for the next battle only.
    In the current `TRUE` implementation build the override is redundant, but
    it keeps temporary `FALSE` validation builds able to exercise the mode.

## Validation

Completed on `feature/all-ability-slots-runtime-20260523`:

- `rtk git diff --check`
- `rtk make -j16 -O check TESTS='All Ability Slots'`
- `rtk make -j16 -O check TESTS=Poi`
- `rtk make -j16 -O check TESTS='AI thinking time'`
- `rtk make -j16 -O check TESTS='Skill Swap'`
- `rtk make -j16 -O check TESTS=Trace`
- `rtk make -j16 -O check TESTS='Sand Spit'`
- `rtk make -j16 -O check`
- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- `rtk make -j16 -O debug` after adding the manual debug trainer route
- `rtk make -j16 -O debug` after expanding the manual debug trainer route to
  Patterns A-E
- `rtk make -j16 -O all` after expanding the manual debug trainer route to
  Patterns A-E
- `rtk make -j16 -O check TESTS='All Ability Slots'` after adding the manual
  debug trainer route
- `rtk make -j16 -O check TESTS='All Ability Slots'` after expanding the
  manual debug trainer route to Patterns A-E
- `rtk make -j16 -O check TESTS='All Ability Slots'` after repairing
  non-representative Flash Fire / Soundproof move-block display and runtime
  behavior
- `rtk make -j16 -O debug` after repairing non-representative Flash Fire /
  Soundproof move-block display and runtime behavior
- `rtk make -j16 -O all` after repairing non-representative Flash Fire /
  Soundproof move-block display and runtime behavior
- `rtk make -j16 -O check` after repairing non-representative Flash Fire /
  Soundproof move-block display and runtime behavior
- `rtk make -j16 -O check TESTS='All Ability Slots'` after repairing
  non-representative Drought switch-in popup / message behavior
- `rtk make -j16 -O check TESTS='Insomnia prevents Rest'` after limiting
  switch-in popup battler binding to actual scripted ability activations
- `rtk make -j16 -O debug` after repairing non-representative Drought switch-in
  popup / message behavior
- `rtk make -j16 -O all` after repairing non-representative Drought switch-in
  popup / message behavior
- `rtk make -j16 -O check` after repairing non-representative Drought switch-in
  popup / message behavior
- `rtk make -j16 -O debug` after simplifying the debug submenu to one
  battle-and-party-setup command per Pattern A-E
- `rtk make -j16 -O all` after simplifying the debug submenu to one
  battle-and-party-setup command per Pattern A-E
- `rtk make -j16 -O check TESTS='All Ability Slots'` after simplifying the
  debug submenu to one battle-and-party-setup command per Pattern A-E
- `rtk git diff --check` after simplifying the debug submenu to one
  battle-and-party-setup command per Pattern A-E
- `rtk mdbook build docs` after simplifying the debug submenu to one
  battle-and-party-setup command per Pattern A-E
- mGBA Live session `all-ability-menu-single-20260524` after simplifying the
  debug submenu: `Party` -> `All Ability...` showed only the five battle
  entries, and selecting `A Recoil Battle` started the trainer battle directly.
- `rtk git diff --check` after the B/C repair
- `rtk mdbook build docs` after the B/C repair
- `rtk make -j16 -O check TESTS='All Ability Slots'` after expanding the debug
  submenu to Patterns A-J and repairing the Intimidate / Levitate / Sticky Hold
  popup and predicate paths
- `rtk make -j16 -O check TESTS='All Ability Slots'` after repairing
  Pressure / Pickpocket popup pollution and adding the non-representative Klutz
  no-popup held-item suppression check
- `rtk make -j16 -O check TESTS='All Ability Slots'` after broadening
  switch-in popup binding and adding Houndoom `Unnerve`, non-representative
  weather, terrain, and `Download` popup coverage
- `rtk make -j16 -O check TESTS='All Ability Slots'` after extending the
  switch-in popup audit to `Frisk` and `Trace`; the focused suite now includes
  19 All Ability Slots cases
- `rtk make -j16 -O check TESTS='Switch-in abilities'` after broadening
  switch-in popup binding, confirming upstream single-ability popup order still
  passes
- `rtk make -j16 -O check` after the Frisk / Trace switch-in popup audit
- `rtk make -j16 -O debug` after the Frisk / Trace switch-in popup audit
- `rtk make -j16 -O all` after the Frisk / Trace switch-in popup audit
- `rtk git diff --check` after the Frisk / Trace switch-in popup audit
- `rtk mdbook build docs` after the Frisk / Trace switch-in popup audit
- `rtk make -j16 -O check TESTS='AI thinking time'` after the Klutz held-item
  suppression path was narrowed so normal-mode AI hot paths stay close to
  upstream behavior
- `rtk make -j16 -O check` after the final Pressure / Pickpocket and Klutz
  repair
- `rtk make -j16 -O debug` after the final Pressure / Pickpocket and Klutz
  repair
- `rtk make -j16 -O all` after the final Pressure / Pickpocket and Klutz
  repair
- `rtk git diff --check` after the final Pressure / Pickpocket and Klutz
  repair
- `rtk mdbook build docs` after the final Pressure / Pickpocket and Klutz
  repair
- `rtk make -j16 -O debug` after expanding the debug submenu to Patterns A-J
- `rtk make -j16 -O all` after expanding the debug submenu to Patterns A-J
- `rtk make -j16 -O check` after expanding the debug submenu to Patterns A-J
- `rtk git diff --check` after expanding the debug submenu to Patterns A-J
- `rtk mdbook build docs` after expanding the debug submenu to Patterns A-J
- mGBA Live session `all-ability-aj-menu-20260524`: opened
  `Party` -> `All Ability...`, confirmed the expanded debug submenu reaches
  Patterns A-J, selected `J Majesty Battle`, reached the trainer battle, chose
  Wobbuffet's `Quick Attack`, and returned to the battle command menu with
  Tsareena still at full HP. Popup text is asserted by the focused test suite.
- mGBA Live session `all-ability-i-pressure-slow-20260524`: opened
  `Party` -> `All Ability...` -> `I Sticky Hold Battle`, reached the trainer
  battle, and confirmed runtime battle state after switch-in had
  `gBattlerAbility = 0` and `gLastUsedAbility = 46` (`ABILITY_PRESSURE`), not
  hidden-slot `Pickpocket`. The visual popup was too brief to capture reliably
  through CLI screenshot timing, so the focused battle test remains the exact
  popup assertion.
- `rtk make -j16 -O check TESTS='All Ability Slots'` after adding Commander,
  Rain Dish, Ice Body, Mold Breaker / Wonder Guard, Neutralizing Gas,
  Chlorophyll, and Pickup coverage; the focused suite now includes 26 All
  Ability Slots cases.
- `rtk make -j16 -O check TESTS='Mold Breaker'`
- `rtk make -j16 -O check TESTS='Neutralizing Gas'`
- `rtk make -j16 -O check TESTS='Power Construct'`
- `rtk make -j16 -O check TESTS=Commander`
- `rtk make -j16 -O check TESTS='Order Up'`
- `rtk make -j16 -O check TESTS='Rain Dish'`
- `rtk make -j16 -O check TESTS='Ice Body'`
- `rtk make -j16 -O check TESTS='Wonder Guard'`
- `rtk make -j16 -O check TESTS=Pickup`
- `rtk make -j16 -O check TESTS=Chlorophyll`
- `rtk make -j16 -O check`
- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- mGBA Live session `all-ability-k-commander-20260524`: loaded the debug ROM,
  opened `Party` -> `All Ability...`, confirmed `K Commander Battle` in the
  submenu, selected it, reached the double battle, and captured Dondozo's stat
  increase after non-representative `Commander` activated from Tatsugiri whose
  representative slot was `Storm Drain`. Screenshots:
  `/tmp/all-ability-k-submenu-bottom-20260524.png`,
  `/tmp/all-ability-k-battle-intro-20260524.png`, and
  `/tmp/all-ability-k-battle-sendout-20260524.png`.
- Cleanup after the Pattern K Commander route was clean:
  `mgba-live-cli stop --session all-ability-k-commander-20260524` reported
  `alive_after:false` / `stopped:true`, and `mgba-live-cli status --all`
  returned `[]`.
- `rtk make -j16 -O check TESTS='All Ability Slots'` after expanding the
  manual debug route to Patterns L-Q and adding focused `Chlorophyll`,
  `Trace`, Conkeldurr `Guts / Sheer Force / Iron Fist`, Durant `Hustle`, and
  `Lightning Rod` / `Storm Drain` redirection coverage; the focused suite now
  includes 31 All Ability Slots cases.
- `rtk make -j16 -O check TESTS='AI thinking time'` after accepting the
  Steven multi one-frame all-slot overhead by raising
  `AI_FRAME_CEILING_STEVEN_MULTI` from 29 to 30.
- `rtk make -j16 -O check`
- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- mGBA Live session `all-ability-lq-menu-20260524`: opened
  `Party` -> `All Ability...`, confirmed the bottom of the expanded submenu
  reaches Patterns I-Q, selected `N Power Stack`, and reached the trainer
  battle command menu. Screenshots:
  `/tmp/all-ability-lq-menu-20260524.png` and
  `/tmp/all-ability-lq-power-stack-start-20260524.png`.
- Cleanup after the Pattern L-Q route check was clean:
  `mgba-live-cli stop --session all-ability-lq-menu-20260524` reported
  `alive_after:false` / `stopped:true`, and `mgba-live-cli status --all`
  returned `[]`.
- `rtk make -j16 -O check TESTS='All Ability Slots'` after the R-T /
  modifier-helper sweep; the focused suite now includes 43 All Ability Slots
  cases, including hidden-slot `Gale Wings` priority and hidden-slot
  `Skill Link` five-hit coverage.
- Follow-up ability audit on 2026-05-24 converted additional representative
  direct checks for move locking, end-turn weather / status effects, stat-change
  prevention, berry / item helpers, Protean / Libero, paradox item activation,
  phazing prevention, and emergency switching.
- `rtk make -j16 -O check TESTS='All Ability Slots'` passed after the follow-up
  ability audit; the focused suite now includes 49 All Ability Slots cases,
  including hidden-slot `Gorilla Tactics`, `Poison Heal`, `Heatproof`,
  `Protean`, `Clear Body`, and `Contrary`.
- `rtk make -j16 -O check TESTS='All Ability Slots'` passed after the
  Summary slot-marker pass; the focused suite now includes 50 All Ability Slots
  cases and covers `B_ALL_ABILITY_SLOTS FALSE` returning non-representative
  abilities to inactive normal behavior.
- `rtk make -j16 -O check`, `rtk make -j16 -O all`, and
  `rtk make -j16 -O debug` passed after the Summary slot-marker pass, with the
  existing RWX linker warning and existing expected / known-failing test
  markers.
- mGBA Live session `all-ability-false-debug-20260524` used a temporary
  `B_ALL_ABILITY_SLOTS FALSE` build, opened `Party` -> `All Ability...`,
  selected `T Partner Mods`, and reached the double-battle command menu. This
  confirms the debug validation route still forces the runtime override when
  the global config is disabled. Screenshots:
  `/tmp/all-ability-false-debug-t-menu-20260524.png` and
  `/tmp/all-ability-false-debug-t-battle-20260524.png`. Cleanup was clean:
  `alive_after:false` / `stopped:true`, and `mgba-live-cli status --all`
  returned `[]`.
- mGBA Live Summary display checks:
  - `all-ability-summary-false-20260524` used a temporary
    `B_ALL_ABILITY_SLOTS FALSE` build and confirmed Summary still shows a
    single representative ability name plus description:
    `/tmp/all-ability-summary-false-summary-20260524.png`.
  - `all-ability-summary-true-20260524` used a
    `B_ALL_ABILITY_SLOTS TRUE` build and confirmed a multi-ability species shows
    direct slot labels and representative marking:
    `2 Rivalry  3 Sheer Force` in the upper strip and `->1 Poison Point` in
    the lower detail area on Nidoqueen at
    `/tmp/all-ability-summary-true-summary-down1-20260524.png`.
  - The final branch config is restored to `TRUE` after the visual checks.
- `rtk make -j16 -O check TESTS='AI thinking time'` passed after the follow-up
  ability audit. The accepted stress ceilings are now 24 for doubles no-flags,
  44 for doubles smart, 32 for Steven multi, and 36 for Steven multi smart.
- Final `B_ALL_ABILITY_SLOTS TRUE` validation on 2026-05-24:
  `rtk make -j16 -O check TESTS='All Ability Slots'` passed with the 50-case
  focused suite, and `rtk make -j16 -O all` / `rtk make -j16 -O debug` passed
  with the existing RWX linker warning.
- The full `rtk make -j16 -O check` suite is not a green gate with global
  `B_ALL_ABILITY_SLOTS TRUE` because many upstream tests assert the old
  single-ability default semantics. The final-TRUE run failed with 264 failed
  tests out of 5165 total, including representative single-ability assumptions
  in Contrary, Intimidate, Rocky Payload, Sheer Force, and AI thinking-time
  ceilings. Log:
  `/home/jastin/.local/share/rtk/tee/1779616365_make_-j16_-O_check.log`.
- mGBA Live session `all-ability-final-true-20260524` used the final `TRUE`
  debug ROM, continued from the local save, opened
  `Party` -> `All Ability...`, selected `A Recoil Battle`, used `Double-Edge`,
  and returned to the battle command menu with Recoil still at `317/317` HP.
  This confirms non-representative `Magic Guard` is active in the final `TRUE`
  build. Screenshots:
  `/tmp/all-ability-final-true-all-ability-submenu-20260524.png`,
  `/tmp/all-ability-final-true-a-moves-20260524.png`, and
  `/tmp/all-ability-final-true-a-after-double-edge-20260524.png`. Cleanup was
  clean: `alive_after:false` / `stopped:true`, and `mgba-live-cli status --all`
  returned `[]`.
- mGBA Live session `all-ability-summary-layout-v3-20260524` rechecked the
  compact Summary ability layout on the final `TRUE` build. Nidoqueen shows the
  non-representative slots as `1 Poison Point 3 Sheer Force` in the upper strip
  and the representative slot as `->2 Rivalry` in the lower white detail area,
  with no overlap into Trainer Memo text:
  `/tmp/all-ability-summary-layout-v3-nidoqueen-20260524.png`. Cleanup was
  clean: `alive_after:false` / `stopped:true`, and `mgba-live-cli status --all`
  returned `[]`.
- mGBA Live session `all-ability-summary-toggle-20260524` rechecked the
  selectable description flow on the final `TRUE` build. Nidoqueen opened on
  selected slot `2 Rivalry` with description `Powers up against rivals.`, `R`
  changed to `3 Sheer Force` with `Trades effects for power.`, another `R`
  wrapped to `1 Poison Point` with `Poisons foe on contact.`, and `L` returned
  to `3 Sheer Force`. Screenshots:
  `/tmp/all-ability-summary-toggle-initial-20260524.png`,
  `/tmp/all-ability-summary-toggle-r-20260524.png`,
  `/tmp/all-ability-summary-toggle-r2-20260524.png`, and
  `/tmp/all-ability-summary-toggle-l-20260524.png`. Cleanup was clean:
  `alive_after:false` / `stopped:true`, and `mgba-live-cli status --all`
  returned `[]`.
- mGBA Live session `all-ability-selected-name-fallback-20260524` rechecked the
  compact selected-name fallback after removing the number-only fallback.
  Nidoqueen opened on `->2 Rivalry`, `R` changed to `->3 Sheer Force`, and
  another `R` wrapped to `->1 Poison Point`, with the matching description shown
  below each selected slot. Screenshots:
  `/tmp/all-ability-selected-name-fallback-summary-initial-20260524.png`,
  `/tmp/all-ability-selected-name-fallback-summary-r-20260524.png`, and
  `/tmp/all-ability-selected-name-fallback-summary-r2-20260524.png`. Cleanup was
  clean: `alive_after:false` / `stopped:true`, and `mgba-live-cli status --all`
  returned `[]`.
- mGBA Live session `all-ability-double-count-20260524` selected
  `T Partner Mods` after the debug-party count refresh and reached the
  double-battle command menu. The captured battle screen showed coherent
  first / second slot placement on both sides:
  `/tmp/all-ability-double-count-t-sendout-20260524.png`. Cleanup was clean:
  `alive_after:false` / `stopped:true`, and `mgba-live-cli status --all`
  returned `[]`.
- `rtk git diff --check`, `rtk make -j16 -O check`, `rtk make -j16 -O all`,
  and `rtk make -j16 -O debug` passed after the R-T / modifier-helper sweep,
  with the existing RWX linker warning and existing expected / known-failing
  test markers.
- mGBA Live session `all-ability-rt-mods-20260524` loaded the debug ROM,
  opened `Party` -> `All Ability...`, confirmed `R Mod Stack`,
  `S Guard Mods`, and `T Partner Mods` at the bottom of the submenu, selected
  `R Mod Stack`, and reached the trainer battle command menu. Screenshots:
  `/tmp/all-ability-rt-submenu-bottom-20260524.png`,
  `/tmp/all-ability-rt-r-selected-20260524.png`, and
  `/tmp/all-ability-rt-r-intro-2-20260524.png`.
- Cleanup after the R-T mGBA Live route check was clean:
  `mgba-live-cli stop --session all-ability-rt-mods-20260524` reported
  `alive_after:false` / `stopped:true`, and `mgba-live-cli status --all`
  returned `[]`.
- `rtk make -j16 -O check` after adding the manual debug trainer route
- `rtk mdbook build docs`

Builds pass with the existing linker warning:

- `arm-none-eabi-ld: warning: ../../pokeemerald-test.elf has a LOAD segment with RWX permissions`
- `arm-none-eabi-ld: warning: ../../pokeemerald.elf has a LOAD segment with RWX permissions`

mGBA Live boot validation, debug-route validation, and docs build are recorded in
[Test Plan](test_plan.md).

## Remaining Risks

- AI caches still store one known ability per battler. Core battle behavior is
  improved, but AI prediction may under-value secondary / hidden slot abilities
  until a follow-up AI pass converts `AiLogicData.abilities` to an ability-set
  view.
- Some helpers still accept one `enum Ability` argument for compatibility.
  Major direct predicates were converted, but obscure item / form / animation
  paths should remain on the manual regression list.
- Transform and some form-change edge cases may need a dedicated follow-up for
  copying slot-local overrides rather than deriving non-representative slots
  from the current species.
- Field / overworld lead ability behavior is intentionally unchanged.
- Summary now provides selectable per-slot descriptions on the Info page, but a
  later UI polish pass may still improve the presentation if the broader
  Party / Status UI overhaul changes the visual skin.
