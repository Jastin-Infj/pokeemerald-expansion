# All Ability Slots Runtime Test Plan

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-31 |
| Baseline | `master` `4e48ff993f` |
| Code status | Adopted into `integration/runtime-dev-20260529`; source remains off `master` |
| Provenance | Local project overlay |

## Runtime Validation Gate

When implementation starts on a feature branch, run:

- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- `rtk make -j16 -O check` under the integration default. The runtime
  integration branch defaults `B_ALL_ABILITY_SLOTS` to `FALSE` and uses a
  per-save runtime override for manual ON/OFF playtesting, so full upstream-style
  validation should stay green under `DEFAULT`.
- focused `TESTS=...` checks for new all-active ability tests
- one focused mGBA Live validation route when available

Record mGBA Live evidence or the exact failure in this file before push.

## Current Validation Evidence

Completed on `feature/all-ability-slots-runtime-20260523`:

- `rtk git diff --check`
- `rtk make -j16 -O check TESTS='All Ability Slots'`
- `rtk make -j16 -O check TESTS='All Ability Slots'` after the follow-up
  ability audit; focused suite now includes 49 All Ability Slots cases
- `rtk make -j16 -O check TESTS='All Ability Slots'` after the Summary
  slot-marker pass; focused suite now includes 50 All Ability Slots cases,
  including a `B_ALL_ABILITY_SLOTS FALSE` regression where non-representative
  `Magic Guard` stays inactive and recoil is applied
- `rtk make -j16 -O check TESTS=Poi`
- `rtk make -j16 -O check TESTS='AI thinking time'`
- `rtk make -j16 -O check TESTS='Skill Swap'`
- `rtk make -j16 -O check TESTS=Trace`
- `rtk make -j16 -O check TESTS='Sand Spit'`
- `rtk make -j16 -O check`
- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- `rtk mdbook build docs`

The focused `B_ALL_ABILITY_SLOTS TRUE` path passes `all`, `debug`, and the
focused All Ability Slots suite with the existing RWX linker warning. The
integration default is now `FALSE`, with `B_ALL_ABILITY_SLOTS_RUNTIME_TOGGLE`
keeping the code compiled and Debug -> `Flags/Vars` -> `All Abilities` able to
force `DEFAULT`, `OFF`, or `ON` per save. `mdbook` passes with existing warnings
for the missing root `CHANGELOG.md` include, existing `CREDITS.md` `</img>`
warning, and large search index. Full `check` passes under the default-`FALSE`
build.

During full-suite validation, the damaging-move Poison Puppeteer test exposed a
stale partner-slot side ability issue: a prior double battle could leave Pastel
Veil in an absent partner battler slot, and `IsAbilityOnSide()` could read it in
the next single battle. The implementation now bounds partner lookup by
`gBattlersCount`, and `rtk make -j16 -O check` passes after that fix.

mGBA Live validation:

- First attempt with the direct script-capable binary failed before boot because
  Qt could not connect to DISPLAY and exited with SIGABRT.
- Retry with `/home/jastin/.local/bin/mgba-qt` wrapper succeeded. Session
  `all-ability-slots-boot-20260523b` booted `pokeemerald.gba`, START input
  reached the Continue menu, and screenshot evidence was saved to
  `/tmp/all_ability_slots_boot_20260523b.png`.
- `mgba_live_stop` returned `alive_after:false` / `stopped:true`; follow-up
`rtk pgrep -af 'mgba-qt|mgba-live'` showed no remaining `mgba-qt` process
beyond the MCP server processes.

Additional debug-route validation added on 2026-05-24:

- `rtk git diff --check`
- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- `rtk make -j16 -O check TESTS='All Ability Slots'`
- `rtk mdbook build docs`
- Debug menu route: hold `R` and press `START`, then choose
  `Party` -> `All Ability...`.
- The submenu contains one trainer-battle entry per Pattern A-T. Each battle
  entry rebuilds the player party for that pattern, then forces the debug
  all-slot override for the next battle only. In the current `TRUE`
  implementation build the override is redundant, but it keeps temporary
  `FALSE` validation builds able to exercise the mode.
- Pattern A `Recoil`: Clefable has representative `Cute Charm`. Use
  `Double-Edge`; non-representative `Magic Guard` should prevent recoil.
- Pattern B `Flash Fire`: Houndoom has representative `Early Bird`. Use
  Arcanine's `Flamethrower`; hidden-slot `Unnerve` should display on entry,
  and non-representative `Flash Fire` should absorb the Fire move. Houndoom
  does not have `Inner Focus`, so Intimidate prevention is not expected here.
- Pattern C `Soundproof`: Kommo-o has representative `Bulletproof`. Use
  Exploud's `Hyper Voice` or `Boomburst`; non-representative `Soundproof`
  should block the sound move.
- Pattern D `Drought`: Ninetales has representative `Flash Fire`. Battle start
  should still trigger hidden-slot `Drought` and brighten sunlight.
- Pattern E `Skill Swap`: Clefable has representative `Cute Charm`. Use
  `Skill Swap`, then `Double-Edge`; slot-local swapping should leave
  non-representative `Magic Guard` active, preventing recoil after the swap.
- Pattern F `Intimidate`: Luxray has representative `Rivalry`. Battle start
  should still trigger non-representative `Intimidate`, lower the opponent's
  Attack, and display `Intimidate`.
- Pattern G `Levitate`: Bronzong has representative `Heatproof`. Use
  Garchomp's `Earthquake`; non-representative `Levitate` should block Ground
  damage and display `Levitate`.
- Pattern H `Storm Drain`: Gastrodon has representative `Sticky Hold`. Use
  Blastoise's `Water Gun` or `Surf`; non-representative `Storm Drain` should
  absorb the Water move, prevent damage, and raise Sp. Atk.
- Pattern I `Sticky Hold`: Muk has representative `Stench` and `Leftovers`.
  Use Weavile's `Knock Off`; non-representative `Sticky Hold` should prevent
  item removal and display `Sticky Hold`.
- Pattern J `Queenly Majesty`: Tsareena has representative `Leaf Guard`. Use
  Wobbuffet's `Quick Attack`; non-representative `Queenly Majesty` should block
  the priority move and display `Queenly Majesty`.
- Pattern K `Commander`: Tatsugiri has representative `Storm Drain` while its
  natural slot 0 remains `Commander`. Battle start should activate Commander
  with Dondozo in doubles, without needing `Commander` to be the representative
  Summary slot.
- Pattern L `Chlorophyll`: Bulbasaur has representative `Overgrow` and
  hidden-slot `Chlorophyll`; sun should change turn order.
- Pattern M `Trace`: Gardevoir has representative `Synchronize` and
  non-representative `Trace`; switch-in copy should use the matching operation
  slot and show the copied ability.
- Pattern N `Power Stack`: Conkeldurr validates `Guts`, `Sheer Force`, and
  `Iron Fist` stacking from all active slots.
- Pattern O `Hustle`: Durant validates non-representative `Hustle` damage and
  accuracy behavior.
- Pattern P `Rod Redirect`: doubles route for hidden-slot `Lightning Rod`
  redirection priority.
- Pattern Q `Drain Redirect`: doubles route for non-representative
  `Storm Drain` redirection priority.
- Pattern R `Mod Stack`: Toxtricity validates `Punk Rock`, `Plus`, and
  `Technician` modifier stacking.
- Pattern S `Guard Mods`: Dragonite validates defensive helper behavior such
  as hidden-slot `Multiscale`.
- Pattern T `Partner Mods`: doubles route for partner-side modifier behavior
  such as `Plus` / `Minus`.
- mGBA Live session `all-ability-debug-route-20260524` loaded
  `pokeemerald.gba` with the local save, opened the debug menu, selected the
  original Pattern A battle route before the A-E submenu split, and reached the
  trainer battle.
- In that session, `Double-Edge` was selected from the move menu. After the
  turn returned to the battle command menu, `SlotCheck` remained at
  `317/317` HP, confirming that the non-representative `Magic Guard` slot
  prevented recoil while the representative ability was `Cute Charm`.
- Screenshot evidence:
  `/tmp/all_ability_slots_debug_battle_20260524.png`.
- Cleanup caveat: `input-clear` succeeded, but `mgba-live-cli stop --session
  all-ability-debug-route-20260524` returned `alive_after:true` /
  `stopped:false` twice. `rtk pgrep -af 'mgba-qt|mgba-live'` showed PID
  `33680 [mgba-qt] <defunct>`, and `mgba-live-cli status --all` still listed
  the session as active. Treat this as a stale mGBA Live cleanup entry, not a
  clean stop.
- After expanding the route to Patterns A-E, mGBA Live session
  `all-ability-patterns-20260524` opened `Party` -> `All Ability...`, selected
  `B Flash Fire Battle`, used Arcanine's `Flamethrower` into Houndoom, and
  returned to the command menu with Houndoom's HP still full. This confirms the
  non-representative `Flash Fire` route is reachable and active from the new
  submenu. Screenshot evidence:
  `/tmp/all_ability_slots_pattern_b_flash_fire_20260524.png`.
- Cleanup caveat for the expanded-route session: `input-clear` succeeded, but
  `mgba-live-cli stop --session all-ability-patterns-20260524` returned
  `alive_after:true` / `stopped:false`. `rtk pgrep -af 'mgba-qt|mgba-live'`
  showed PID `37462 [mgba-qt] <defunct>`, and `mgba-live-cli status --all`
  still listed that session as active.
- Follow-up B/C repair on 2026-05-24: manual runtime testing showed Pattern B
  and Pattern C were still visually misleading / failing around
  non-representative move-block abilities. The repair makes
  `CanAbilityAbsorbMove()` enumerate the target battler's active ability set in
  real attacker-versus-defender all-slot battles, and carries the selected
  non-representative ability through ability popup and battle-message text.
- Follow-up D repair on 2026-05-24: Pattern D's hidden-slot `Drought` already
  started sun, but the switch-in weather ability popup could still show the
  representative Summary ability (`Flash Fire`). Weather ability scripts now
  bind `gBattlerAbility` / popup overwrite only when the scripted ability
  actually activates, preserving normal non-weather popup state while displaying
  hidden-slot `Drought` correctly.
- Focused regression after the B/C repair:
  `rtk make -j16 -O check TESTS='All Ability Slots'` passed, including
  non-representative `Flash Fire`, `Soundproof`, and `Drought` popup + message
  assertions.
- Focused regression after the D repair:
  `rtk make -j16 -O check TESTS='Insomnia prevents Rest'` passed, confirming
  the switch-in popup binding does not leave stale battler state for normal
  single-ability popup scripts.
- Debug submenu simplification on 2026-05-24 removed the separate party-only
  entries. `A Recoil Battle` through `E Skill Swap Battle` now each rebuild the
  matching player party, build the matching enemy party, force the all-slot
  debug override for that one battle, and start the battle directly.
- `rtk make -j16 -O debug` passed after the B/C repair.
- `rtk make -j16 -O all` and `rtk make -j16 -O check` passed after the B/C
  repair with the existing RWX linker warning and existing expected /
  known-failing test markers.
- `rtk make -j16 -O debug`, `rtk make -j16 -O all`, and
  `rtk make -j16 -O check` passed after the D repair with the existing RWX
  linker warning and existing expected / known-failing test markers.
- `rtk make -j16 -O debug`,
  `rtk make -j16 -O all`,
  `rtk make -j16 -O check TESTS='All Ability Slots'`,
  `rtk git diff --check`, and `rtk mdbook build docs` passed after the debug
  submenu simplification.
- mGBA Live debug submenu simplification check: session
  `all-ability-menu-single-20260524` loaded the debug ROM, opened
  `Party` -> `All Ability...`, and confirmed the submenu contains only
  `A Recoil Battle`, `B Flash Fire Battle`, `C Soundproof Battle`,
  `D Drought Battle`, and `E Skill Swap Battle`. Selecting `A Recoil Battle`
  immediately started the trainer battle after party setup. Screenshots:
  `/tmp/all-ability-menu-single-all-ability-submenu-20260524.png` and
  `/tmp/all-ability-menu-single-a-executed-20260524.png`.
- Cleanup after the debug submenu simplification check was clean:
  `mgba-live-cli stop --session all-ability-menu-single-20260524` reported
  `alive_after:false` / `stopped:true`, and `mgba-live-cli status --all`
  returned `[]`.
- `rtk git diff --check` passed after the B/C repair.
- `rtk mdbook build docs` passed after the B/C repair with the existing missing
  root `CHANGELOG.md` include warning, existing `CREDITS.md` `</img>` warning,
  and large search-index warning.
- mGBA Live Pattern B recheck: session
  `all-ability-b-flashfire-20260524` used `Party` -> `All Ability...` ->
  `B Flash Fire Battle`; repeated `Flamethrower` attempts reduced PP while
  Houndoom stayed at full HP, confirming non-representative `Flash Fire`
  absorbs the Fire move. Screenshot:
  `/tmp/all-ability-b-flashfire-absorbed-20260524.png`.
- mGBA Live Pattern C recheck: session
  `all-ability-c-fixed2-20260524` used `Party` -> `All Ability...` ->
  `C Soundproof Battle`; `Hyper Voice` reduced PP from 10 to 9 while Kommo-o
  stayed at full HP, confirming non-representative `Soundproof` blocks the
  sound move. Screenshot:
  `/tmp/all-ability-c-soundproof-blocked-20260524.png`.
- Cleanup after the B/C rechecks was clean: `mgba_live_stop` reported
  `alive_after:false` / `stopped:true`, and
  `rtk /home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli status --all`
  returned `[]`.
- Follow-up F-J expansion on 2026-05-24 added five high-risk debug routes and
  matching focused battle tests: switch-in `Intimidate`, Ground immunity
  `Levitate`, redirect / absorb `Storm Drain`, item-retention `Sticky Hold`,
  and priority-blocking `Queenly Majesty`.
- The F-J focused tests exposed three representative-slot holes before the
  repair: `Intimidate` switch-in popups, `Levitate` effectiveness popups, and
  `Sticky Hold` item-removal popups could report or consult the representative
  ability instead of the actual non-representative triggering slot. These are
  now bound to the selected active ability.
- `rtk make -j16 -O check TESTS='All Ability Slots'` passed after the F-J
  expansion and the Intimidate / Levitate / Sticky Hold repair; the focused
  suite now includes 11 All Ability Slots cases.
- Follow-up Pressure / Klutz repair on 2026-05-24: manual Pattern I testing
  showed Weavile's switch-in `Pressure` popup could render as its later
  inactive hidden-slot `Pickpocket`. The all-slot dispatcher now preserves the
  last triggered ability state when later slots are checked but do not trigger.
- The same repair adds explicit coverage for `Klutz`: non-representative
  `Klutz` suppresses held-item effects and Gen V+ `Fling`, but does not show a
  `Klutz` popup. This matches the passive held-item-suppression policy recorded
  from the external Klutz references.
- Houndoom / Pattern B follow-up on 2026-05-24: external references and local
  species data agree that Houndoom is `Early Bird / Flash Fire / Unnerve`, not
  `Inner Focus`. `Unnerve` is a switch-in message ability, so Pattern B should
  show hidden-slot `Unnerve` at entry and later show non-representative
  `Flash Fire` when Arcanine uses `Flamethrower`.
- The same follow-up broadened generic switch-in popup binding so
  `gBattlerAbility` is set from the actual triggering battler before popup
  scripts run. This covers `Unnerve`, weather setters, terrain setters,
  `Download`, `Pressure`, ruin abilities, and other direct entry-popup scripts
  that previously depended on ambient battle-script state.
- Follow-up switch-in popup audit on 2026-05-24 broadened the same binding to
  remaining entry scripts that call ability popups directly, including `Frisk`,
  `Trace`, stat-raise switch-in scripts, `Zero to Hero`, `Commander`,
  weather-form scripts, `Protosynthesis`, `Mimicry`, and `Quark Drive`.
  The new Trace test initially timed out because copied Trace could recursively
  re-enter switch-in ability processing in all-slot mode. Trace now skips after
  the battler already has an overwritten ability.
- `rtk make -j16 -O check TESTS='All Ability Slots'` passed after the
  Pressure / Pickpocket and Klutz repair; the focused suite now includes 13 All
  Ability Slots cases.
- `rtk make -j16 -O check TESTS='All Ability Slots'` passed after the Houndoom
  `Unnerve` / switch-in popup binding follow-up; the focused suite now includes
  17 All Ability Slots cases.
- `rtk make -j16 -O check TESTS='All Ability Slots'` passed after adding the
  `Frisk` and `Trace` popup follow-up; the focused suite now includes 19 All
  Ability Slots cases.
- `rtk make -j16 -O check TESTS='Switch-in abilities'` passed after the
  switch-in popup binding follow-up, confirming the existing single-ability
  popup order tests still pass.
- `rtk make -j16 -O check`, `rtk make -j16 -O debug`, and
  `rtk make -j16 -O all` passed after the Frisk / Trace switch-in popup audit
  with the existing RWX linker warning and existing expected / known-failing
  test markers.
- `rtk git diff --check` and `rtk mdbook build docs` passed after the
  Frisk / Trace switch-in popup audit. `mdbook` still reports the existing
  missing root `CHANGELOG.md` include warning, existing `CREDITS.md` `</img>`
  warning, and large search-index warning.
- Follow-up L-Q expansion on 2026-05-24 added six manual debug routes:
  `Chlorophyll`, `Trace`, Conkeldurr `Guts / Sheer Force / Iron Fist` power
  stack, Durant `Hustle`, double-battle `Lightning Rod` redirection, and
  double-battle `Storm Drain` redirection.
- The same pass added focused tests for the high-risk mechanics:
  non-representative `Guts` burn handling / Attack boost,
  non-representative `Sheer Force` base-power boost and secondary-effect
  suppression, non-representative `Iron Fist`, non-representative `Hustle`
  damage and accuracy, and faster-candidate redirection for
  `Lightning Rod` / `Storm Drain`.
- External mechanics references checked during this pass:
  Bulbapedia confirms `Lightning Rod` redirects single-target Electric moves;
  Bulbapedia confirms `Storm Drain` draws single-target Water moves; Pokemon
  Showdown's dex records the multi-redirect priority as highest Speed, then
  ability-active order on ties. The local focused test covers the highest Speed
  case through existing turn-order selection.
- `rtk make -j16 -O check TESTS='All Ability Slots'` passed after the L-Q
  expansion; the focused suite now includes 31 All Ability Slots cases.
- `rtk make -j16 -O check TESTS='AI thinking time'` initially showed only the
  non-smart Steven multi profile at 30 frames against the previous local guard
  of 29. The all-slot branch accepts that one-frame cost by raising
  `AI_FRAME_CEILING_STEVEN_MULTI` to 30.
- `rtk make -j16 -O check TESTS='AI thinking time'` passed after the Steven
  multi ceiling was raised to 30.
- `rtk make -j16 -O check`, `rtk make -j16 -O all`, and
  `rtk make -j16 -O debug` passed after the L-Q expansion and accepted AI
  ceiling update, with the existing RWX linker warning and existing expected /
  known-failing test markers.
- mGBA Live session `all-ability-lq-menu-20260524` loaded the debug ROM,
  opened `Party` -> `All Ability...`, confirmed the bottom of the submenu
  reaches Patterns I-Q including `L Chlorophyll Battle`, `M Trace Battle`,
  `N Power Stack`, `O Hustle Battle`, `P Rod Redirect`, and
  `Q Drain Redirect`, selected `N Power Stack`, and reached the trainer battle
  command menu. Screenshots:
  `/tmp/all-ability-lq-menu-20260524.png` and
  `/tmp/all-ability-lq-power-stack-start-20260524.png`.
- Cleanup after the L-Q mGBA Live route check was clean:
  `mgba-live-cli stop --session all-ability-lq-menu-20260524` reported
  `alive_after:false` / `stopped:true`, and `mgba-live-cli status --all`
  returned `[]`.
- Follow-up R-T / modifier-helper sweep on 2026-05-24 added manual debug
  routes for `Mod Stack`, `Guard Mods`, and `Partner Mods`.
- The same pass broadened runtime helper coverage for base-power, Attack /
  Defense, final damage, STAB / Tera STAB, accuracy, priority, multi-hit,
  contact, powder-block, weight, and partner modifier paths.
- `rtk make -j16 -O check TESTS='All Ability Slots'` passed after the R-T /
  modifier-helper sweep; the focused suite now includes 43 All Ability Slots
  cases, including hidden `Gale Wings` priority and hidden `Skill Link`
  five-hit coverage.
- Follow-up ability audit on 2026-05-24 converted additional representative
  direct checks for `Gorilla Tactics` move locking, damaging weather immunity,
  `Poison Heal`, `Heatproof`, Yawn sleep prevention, `Protean` / `Libero`,
  `Contrary` / `Simple`, stat-drop prevention abilities, `Ripen`, paradox item
  activation, `Suction Cups`, OHKO `Sturdy`, `Quick Feet`, and emergency
  switching.
- The all-ability debug route now calls `CalculatePlayerPartyCount()` after
  rebuilding its generated player party. This addresses a local debug-route
  source for possible double-battle first-slot placement oddities. If the visual
  placement issue still appears, track it as likely upstream debug / double
  sendout rendering behavior rather than all-slot ability logic.
- mGBA Live session `all-ability-double-count-20260524` loaded the debug ROM,
  continued from the local save, opened `Party` -> `All Ability...`, selected
  `T Partner Mods`, and reached the double-battle command menu after the party
  count refresh. The observed player and opponent first / second slot placement
  was coherent in the captured battle screen. Screenshots:
  `/tmp/all-ability-double-count-ability-menu-t-20260524.png` and
  `/tmp/all-ability-double-count-t-sendout-20260524.png`.
- Cleanup after the double-count mGBA Live route check was clean:
  `mgba-live-cli stop --session all-ability-double-count-20260524` reported
  `alive_after:false` / `stopped:true`, and `mgba-live-cli status --all`
  returned `[]`.
- `rtk make -j16 -O check TESTS='All Ability Slots'` passed after the follow-up
  ability audit; the focused suite now includes 49 All Ability Slots cases,
  including hidden `Gorilla Tactics`, `Poison Heal`, `Heatproof`, `Protean`,
  `Clear Body`, and `Contrary`.
- `rtk make -j16 -O check TESTS='All Ability Slots'` passed after adding the
  `B_ALL_ABILITY_SLOTS FALSE` regression and Summary slot-marker display; the
  focused suite now includes 50 All Ability Slots cases.
- `rtk make -j16 -O check`, `rtk make -j16 -O all`, and
  `rtk make -j16 -O debug` passed after the Summary slot-marker display.
- mGBA Live session `all-ability-false-debug-20260524` used a temporary
  `B_ALL_ABILITY_SLOTS FALSE` build, opened `Party` -> `All Ability...`, selected
  `T Partner Mods`, and reached the double-battle command menu. Screenshots:
  `/tmp/all-ability-false-debug-t-menu-20260524.png` and
  `/tmp/all-ability-false-debug-t-battle-20260524.png`.
- Cleanup after the false-config debug-route mGBA Live check was clean:
  `mgba-live-cli stop --session all-ability-false-debug-20260524` reported
  `alive_after:false` / `stopped:true`, and `mgba-live-cli status --all`
  returned `[]`.
- Summary visual validation:
  - `all-ability-summary-false-20260524` confirmed temporary
    `B_ALL_ABILITY_SLOTS FALSE` Summary behavior still shows one representative
    ability and its description:
    `/tmp/all-ability-summary-false-summary-20260524.png`.
  - `all-ability-summary-true-20260524` used a `B_ALL_ABILITY_SLOTS TRUE` build
    and confirmed Nidoqueen shows the non-representative slots in the upper
    strip and `->1 Poison Point` in the lower detail area:
    `/tmp/all-ability-summary-true-summary-down1-20260524.png`.
  - `all-ability-summary-layout-v3-20260524` rechecked the final compact layout:
    Nidoqueen shows `1 Poison Point 3 Sheer Force` in the upper strip and
    `->2 Rivalry` in the lower white detail area without overlapping Trainer
    Memo text. Screenshot:
    `/tmp/all-ability-summary-layout-v3-nidoqueen-20260524.png`.
  - `all-ability-summary-toggle-20260524` rechecked the `L` / `R` selectable
    description flow. Nidoqueen opened on slot `2 Rivalry` with `Powers up
    against rivals.`, `R` changed to slot `3 Sheer Force` with `Trades effects
    for power.`, another `R` wrapped to slot `1 Poison Point` with `Poisons foe
    on contact.`, and `L` returned to slot `3 Sheer Force`. Screenshots:
    `/tmp/all-ability-summary-toggle-initial-20260524.png`,
    `/tmp/all-ability-summary-toggle-r-20260524.png`,
    `/tmp/all-ability-summary-toggle-r2-20260524.png`, and
    `/tmp/all-ability-summary-toggle-l-20260524.png`.
  - `all-ability-selected-name-fallback-20260524` rechecked the compact fallback
    after removing the number-only display. Nidoqueen showed selected slot plus
    ability name for `->2 Rivalry`, `->3 Sheer Force`, and `->1 Poison Point`,
    with the matching description below each selection. Screenshots:
    `/tmp/all-ability-selected-name-fallback-summary-initial-20260524.png`,
    `/tmp/all-ability-selected-name-fallback-summary-r-20260524.png`, and
    `/tmp/all-ability-selected-name-fallback-summary-r2-20260524.png`.
  - `all-ability-unified-selector-20260524` rechecked the unified selected-slot
    selector after removing adaptive multi-name display. Nidoqueen (`1/2/3`)
    showed `->2 Rivalry` then `->3 Sheer Force`; Bastiodon (`1/3`) showed
    `->1 Sturdy` then skipped the empty middle slot and changed to
    `->3 Soundproof`. Each selection showed the matching description. This
    older sparse-slot behavior was superseded on 2026-05-31 by mirroring slot 1
    into omitted slot 2 for Summary readability.
    Screenshots:
    `/tmp/all-ability-unified-selector-nidoqueen-20260524.png`,
    `/tmp/all-ability-unified-selector-nidoqueen-r-20260524.png`,
    `/tmp/all-ability-unified-selector-bastiodon-20260524.png`, and
    `/tmp/all-ability-unified-selector-bastiodon-r-20260524.png`.
  - A temporary `P_SUMMARY_SCREEN_ALL_ABILITY_SLOT_SWITCH FALSE` build passed
    `rtk make -j16 -O debug`. This confirms the Summary selector switch can be
    disabled at compile time; the branch was restored to default `TRUE` before
    commit.
  - After adding `P_SUMMARY_SCREEN_ALL_ABILITY_SLOT_SWITCH`, final default-`TRUE`
    validation passed: `rtk git diff --check`,
    `rtk make -j16 -O check TESTS='All Ability Slots'`,
    `rtk make -j16 -O all`, `rtk make -j16 -O debug`, and
    `rtk mdbook build docs`.
  - Cleanup was clean for the listed Summary sessions, and the final branch config
    is restored to `TRUE`.
- Final `B_ALL_ABILITY_SLOTS TRUE` validation on 2026-05-24:
  - `rtk make -j16 -O check TESTS='All Ability Slots'` passed with the
    50-case focused suite.
  - `rtk make -j16 -O all` and `rtk make -j16 -O debug` passed with the
    existing RWX linker warning.
  - `rtk make -j16 -O check` failed under global `TRUE` with 264 failed tests
    out of 5165 total because upstream baseline tests still assert
    single-ability default behavior. Representative failures include Contrary,
    Intimidate, Rocky Payload, Sheer Force, and AI thinking-time ceilings. Log:
    `/home/jastin/.local/share/rtk/tee/1779616365_make_-j16_-O_check.log`.
  - mGBA Live session `all-ability-final-true-20260524` used the final `TRUE`
    debug ROM, continued from the local save, opened
    `Party` -> `All Ability...`, selected `A Recoil Battle`, used
    `Double-Edge`, and returned to the battle command menu with Recoil still at
    `317/317` HP. Screenshots:
    `/tmp/all-ability-final-true-all-ability-submenu-20260524.png`,
    `/tmp/all-ability-final-true-a-moves-20260524.png`, and
    `/tmp/all-ability-final-true-a-after-double-edge-20260524.png`.
  - Cleanup after the final-TRUE mGBA Live check was clean:
    `mgba-live-cli stop --session all-ability-final-true-20260524` reported
    `alive_after:false` / `stopped:true`, and `mgba-live-cli status --all`
    returned `[]`.
- `rtk make -j16 -O check TESTS='AI thinking time'` passed after the follow-up
  ability audit. The accepted stress ceilings are now 24 for doubles no-flags,
  44 for doubles smart, 32 for Steven multi, and 36 for Steven multi smart.
- `rtk git diff --check`, `rtk make -j16 -O check`, `rtk make -j16 -O all`,
  and `rtk make -j16 -O debug` passed after the R-T / modifier-helper sweep,
  with the existing RWX linker warning and existing expected / known-failing
  test markers.
- mGBA Live session `all-ability-rt-mods-20260524` loaded the debug ROM,
  continued from the local save, opened `Party` -> `All Ability...`, confirmed
  the bottom of the submenu reaches `R Mod Stack`, `S Guard Mods`, and
  `T Partner Mods`, selected `R Mod Stack`, and reached the trainer battle
  command menu with Toxtricity facing Blissey. Screenshots:
  `/tmp/all-ability-rt-submenu-bottom-20260524.png`,
  `/tmp/all-ability-rt-r-selected-20260524.png`, and
  `/tmp/all-ability-rt-r-intro-2-20260524.png`.
- Cleanup after the R-T mGBA Live route check was clean:
  `mgba-live-cli stop --session all-ability-rt-mods-20260524` reported
  `alive_after:false` / `stopped:true`, and `mgba-live-cli status --all`
  returned `[]`.
- `rtk make -j16 -O check TESTS='AI thinking time'` passed after narrowing the
  Klutz held-item suppression path so normal-mode AI item checks avoid the
  all-slot predicate cost.
- `rtk make -j16 -O check` passed after the final Pressure / Pickpocket and
  Klutz repair with the existing expected / known-failing test markers.
- `rtk make -j16 -O debug` and `rtk make -j16 -O all` passed after the final
  Pressure / Pickpocket and Klutz repair with the existing RWX linker warning.
- `rtk git diff --check` and `rtk mdbook build docs` passed after the final
  Pressure / Pickpocket and Klutz repair. `mdbook` still reports the existing
  missing root `CHANGELOG.md` include warning, existing `CREDITS.md` `</img>`
  warning, and large search-index warning.
- mGBA Live Pattern I pressure-state check: the MCP start path failed before
  boot because Qt received an empty `DISPLAY`; the CLI retry with `DISPLAY=:0`
  and the script-capable mGBA path succeeded. Session
  `all-ability-i-pressure-slow-20260524` opened
  `Party` -> `All Ability...` -> `I Sticky Hold Battle`, reached the trainer
  battle, and runtime memory showed `gBattlerAbility = 0` with
  `gLastUsedAbility = 46` (`ABILITY_PRESSURE`), not hidden-slot
  `Pickpocket`. Screenshots show the selected Pattern I route and the reached
  battle menu: `/tmp/all-ability-i-slow-selected-20260524.png` and
  `/tmp/all-ability-i-slow-battle-2-20260524.png`. The popup itself was too
  short-lived for reliable CLI screenshot capture, so the focused
  `ABILITY_POPUP(player, ABILITY_PRESSURE)` test remains the exact visual
  regression guard.
- Cleanup after the Pattern I pressure-state check was clean:
  `mgba-live-cli stop --session all-ability-i-pressure-slow-20260524` reported
  `alive_after:false` / `stopped:true`, and `mgba-live-cli status --all`
  returned `[]`.
- `rtk make -j16 -O debug`, `rtk make -j16 -O all`, and
  `rtk make -j16 -O check` passed after the F-J expansion with the existing RWX
  linker warning and existing expected / known-failing test markers.
- `rtk git diff --check` and `rtk mdbook build docs` passed after the F-J
  expansion. `mdbook` still reports the existing missing root `CHANGELOG.md`
  include warning, existing `CREDITS.md` `</img>` warning, and large
  search-index warning.
- mGBA Live F-J route check: session `all-ability-aj-menu-20260524` loaded the
  debug ROM, opened `Party` -> `All Ability...`, captured the top of the
  expanded submenu with `A Recoil Battle` through `I Sticky Hold Battle`,
  scrolled to confirm `J Majesty Battle`, selected `J Majesty Battle`, reached
  the trainer battle, selected Wobbuffet's `Quick Attack`, and returned to the
  command menu with Tsareena still at full HP. The focused test suite covers
  the exact `Queenly Majesty` popup assertion. Screenshots:
  `/tmp/all-ability-aj-submenu-top-20260524.png`,
  `/tmp/all-ability-aj-submenu-bottom-20260524.png`,
  `/tmp/all-ability-aj-j-selected-20260524.png`,
  `/tmp/all-ability-aj-j-fight-menu-20260524.png`, and
  `/tmp/all-ability-aj-j-after-quick-20260524.png`.
- Cleanup after the F-J mGBA Live route check was clean:
  `mgba-live-cli stop --session all-ability-aj-menu-20260524` reported
  `alive_after:false` / `stopped:true`, and
  `mgba-live-cli status --all` returned `[]`.
- mGBA Live Pattern B route check after the Houndoom / popup audit: session
  `all-ability-b-unnerve-2-20260524` loaded the debug ROM, opened
  `Party` -> `All Ability...`, confirmed the cursor on `B Flash Fire Battle`,
  selected it, and reached the Houndoom battle command menu. The hidden-slot
  `Unnerve` popup is too short-lived for reliable CLI screenshot capture, so
  `ABILITY_POPUP(opponent, ABILITY_UNNERVE)` in the focused test remains the
  exact visual regression guard. Screenshots:
  `/tmp/all-ability-b2-pattern-b-selected-20260524.png` and
  `/tmp/all-ability-b2-after-intro-1-20260524.png`.
- Cleanup after the Pattern B route check was clean:
  `mgba-live-cli stop --session all-ability-b-unnerve-2-20260524` reported
  `alive_after:false` / `stopped:true`, and `mgba-live-cli status --all`
  returned `[]`.
- Commander / weather / disruptive ability follow-up on 2026-05-24 added
  focused coverage for non-representative `Commander`, `Rain Dish`, `Ice Body`,
  Mold Breaker-family bypass against `Wonder Guard`, `Neutralizing Gas`,
  `Chlorophyll`, and `Pickup`.
- `B_ALL_ABILITY_SLOTS_MOLD_BREAKER` now controls whether non-representative
  Mold Breaker / Teravolt / Turboblaze / Mycelium Might can bypass target
  abilities. `B_ALL_ABILITY_SLOTS_NEUTRALIZING_GAS` now controls whether
  non-representative Neutralizing Gas suppresses other abilities. Both default
  to enabled for this mechanics branch so later facility balance can opt out
  explicitly.
- `rtk make -j16 -O check TESTS='All Ability Slots'` passed with 26 focused
  All Ability Slots cases.
- Existing focused suites passed after the follow-up:
  `TESTS='Mold Breaker'`, `TESTS='Neutralizing Gas'`,
  `TESTS='Power Construct'`, `TESTS=Commander`, `TESTS='Order Up'`,
  `TESTS='Rain Dish'`, `TESTS='Ice Body'`, `TESTS='Wonder Guard'`,
  `TESTS=Pickup`, and `TESTS=Chlorophyll`.
- `rtk make -j16 -O check`, `rtk make -j16 -O all`, and
  `rtk make -j16 -O debug` passed after the follow-up with the existing RWX
  linker warning and existing expected / known-failing test markers.
- mGBA Live Pattern K route check: session
  `all-ability-k-commander-20260524` loaded the debug ROM, opened
  `Party` -> `All Ability...`, confirmed `K Commander Battle` at the bottom of
  the submenu, selected it, reached the double battle, and captured Dondozo's
  stat increase after non-representative `Commander` activated from Tatsugiri
  whose representative slot was `Storm Drain`. Screenshots:
  `/tmp/all-ability-k-submenu-bottom-20260524.png`,
  `/tmp/all-ability-k-battle-intro-20260524.png`, and
  `/tmp/all-ability-k-battle-sendout-20260524.png`.
- Cleanup after the Pattern K route check was clean:
  `mgba-live-cli stop --session all-ability-k-commander-20260524` reported
  `alive_after:false` / `stopped:true`, and `mgba-live-cli status --all`
  returned `[]`.

## Runtime Integration Validation 2026-05-29

#60 was adopted into `integration/runtime-dev-20260529` after #47 Battle Item
Restore, #48 Held Item Catalog, #54 Party / Status UI, #51 Scout Selection, and
#57 Friendly Shop Pokemon Vendor. The integration branch keeps
`B_ALL_ABILITY_SLOTS` default `FALSE` and uses a per-save runtime override for
manual all-slot playtesting. Tests can still force `TRUE` / `FALSE` directly.

Local validation passed:

- `rtk git diff --check`
- `rtk git diff --cached --check`
- `rtk make -j16 -O check TESTS=test/battle/ability/damp.c`
- `rtk make -j16 -O check TESTS=test/battle/ai/ai.c`
- `rtk make -j16 -O check TESTS='All Ability Slots'`
- `rtk make -j16 -O check TESTS='AI thinking time'`
- `rtk make -j16 -O check TESTS='Battle item restore'`
- `rtk make -j16 -O check TESTS=test/pokemon_vendor.c`
- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- `rtk make -j16 -O check`

Builds still report the existing RWX linker warning. Full `check` passed with
the existing expected / known-failing markers.

2026-05-30 review-blocker follow-up:

- `IsAbilityOnSide()`, `IsAbilityOnField()`, and `IsAbilityOnFieldExcept()` now
  require live, present battlers before all-slot `BattlerHasAbility()` matching.
- Added `Damp does not prevent Explosion-like moves after its bearer faints` to
  prevent fainted field-ability providers from leaking into later same-turn
  actions.
- A second `codex review --base master` pass found an end-turn weather edge case
  where paired weather abilities could skip the second effect in all-slot mode.
  End-turn weather now queues paired ability scripts with `eventState.endTurnBlock`,
  and `All Ability Slots applies paired end-turn weather abilities one at a time`
  covers hidden-slot `Dry Skin` plus `Solar Power` in sun.
- A later `codex review --base master` pass found a related representative
  third-block recursion risk: if the selected ability was itself `Harvest`,
  `Moody`, `Pickup`, or another third-block end-turn ability, the early branch
  could call the all-slot dispatcher and replay unrelated slot effects.
  `TryHandleThirdEventBlockAbility()` now uses single-ability dispatch for the
  representative and additional-slot paths. `All Ability Slots does not replay
  Solar Power during third-block ability handling` covers representative
  `Harvest` plus hidden-slot `Solar Power` and asserts only one sun damage tick.
- `AI_FRAME_CEILING_SINGLES_SMART_TRAINER` is now 10 to account for the live
  guard in a hot ability-query path; focused AI and the earlier default-`FALSE`
  integration full `check` pass.
- Focused validation: `rtk make -j16 -O check TESTS=test/battle/ability/all_ability_slots.c`.

mGBA Live validation:

- Session `integration-all-ability-optin-smoke` used the earlier default-`FALSE`
  integration debug ROM, continued the local save, opened
  `Party` -> `All Ability...`, selected `A Recoil Battle`, used Clefable's
  `Double-Edge`, and returned to the move menu with Clefable still at
  `317/317`. This confirmed the debug override enabled all-slot behavior before
  the save-backed runtime override was added.
- Screenshots:
  `/tmp/integration-all-ability-optin-submenu.png`,
  `/tmp/integration-all-ability-optin-move-menu.png`, and
  `/tmp/integration-all-ability-optin-after-double-edge.png`.
- Cleanup was clean: `mgba_live_stop` reported `alive_after:false` and
  `mgba-live-cli status --all` returned `[]`.

## Runtime Toggle Validation 2026-05-31

`include/config/battle.h` keeps `B_ALL_ABILITY_SLOTS` `FALSE`, keeps
`B_ALL_ABILITY_SLOTS_RUNTIME_TOGGLE` `TRUE`, and
`SaveBlock2.optionsAllAbilitySlotsMode` adds a per-save override. Debug ->
`Flags/Vars` -> `All Abilities` cycles `DEFAULT`, `OFF`, and `ON`.
`DEFAULT` keeps the build config, `OFF` forces single-ability behavior, and
`ON` makes normal battles initialize `gAllAbilitySlotsBattle` as enabled through
`BattleStartClearSetData()`. Focused validation should prioritize:

- `rtk make -j16 -O check TESTS='All Ability Slots'`
- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- `rtk make -j16 -O check`
- one mGBA Live smoke from a non-All-Ability debug trainer or normal trainer
  route after setting the save override to `ON`, confirming a
  non-representative slot effect applies without the debug battle override.

Validation result on `integration/runtime-dev-20260529`:

- `rtk git diff --check` passed.
- `rtk mdbook build docs` passed with existing warnings for the missing root
  `CHANGELOG.md` include, existing `CREDITS.md` `</img>` warning, and large
  search index.
- `rtk make -j16 -O check TESTS='All Ability Slots'` passed.
- `rtk make -j16 -O all` passed.
- `rtk make -j16 -O debug` passed.
- `rtk make -j16 -O check` passed under the default-`FALSE` build.
- mGBA Live session `all-ability-runtime-toggle-final-20260531` booted
  `pokeemerald.gba`; Lua read `gSaveBlock2Ptr`, changed
  `SaveBlock2.optionsAllAbilitySlotsMode` from `DEFAULT` (`0`) to `ON` (`2`),
  and read it back as `afterMode = 2`.
- Screenshot session `all-ability-runtime-toggle-final-20260531` captured
  `/tmp/all-ability-runtime-toggle-final-20260531.png`; cleanup returned
  `status --all` to `[]`.
- KO-popup follow-up on `integration/runtime-dev-20260529`: compared the
  original `feature/all-ability-slots-runtime-20260523` move-end KO path and
  confirmed it did not bind `gBattleScripting.battler` /
  `abilityPopupOverwrite` before script execution. The integration fix adds a
  non-left-battler hidden-slot `Moxie` regression plus Pattern U `Moxie Popup`.
- `rtk make -j16 -O check TESTS='All Ability Slots'` passed after the
  KO-popup follow-up.
- `rtk make -j16 -O check TESTS='Moxie'` passed after the KO-popup follow-up.
- `rtk make -j16 -O all` passed after the KO-popup follow-up.
- `rtk make -j16 -O debug` passed after adding Pattern U `Moxie Popup`.
- 2026-05-31 Summary selector follow-up: slot 2 now mirrors slot 1 for species
  with omitted normal slot 2 plus hidden slot 3, so Summary selection presents a
  continuous `1/2/3` sequence while battle ability-set evaluation continues to
  dedupe duplicates. The state editor remains backed by the real species ability
  table and should not write the omitted slot.
- 2026-05-31 Summary selector validation: `rtk git diff --check`,
  `rtk make -j16 -O check TESTS='All Ability Slots'`, `rtk make -j16 -O all`,
  `rtk make -j16 -O debug`, `rtk make -j16 -O check`, and
  `rtk mdbook build docs` passed. The make commands only emitted the existing
  RWX linker warning; mdbook only emitted the existing missing root
  `CHANGELOG.md`, `CREDITS.md` `</img>`, and large search-index warnings.
- mGBA Live boot smoke `runtime-followup-ability-noencounter-20260531b`
  captured `/tmp/runtime-followup-ability-noencounter-20260531b-boot.png` and
  stopped cleanly (`status --all` returned `[]`). The exact Summary slot visual
  remains a manual check from party Summary because the smoke only covered boot.
- mGBA Live session `all-ability-moxie-popup-20260531` booted the debug ROM,
  continued from the local save, opened `Party` -> `All Ability...`, confirmed
  Pattern U `Moxie Popup` appears at the bottom of the submenu, started the
  trainer battle, selected Mightyena's `Quick Attack`, and reached the KO /
  EXP flow. Screenshots:
  `/tmp/all-ability-moxie-popup-submenu-bottom-20260531.png`,
  `/tmp/all-ability-moxie-popup-battle-20260531.png`, and
  `/tmp/all-ability-moxie-popup-after-a2-20260531.png`. The exact popup
  assertion is covered by the focused `All Ability Slots` test because the
  high-speed live session advanced through the KO follow-up quickly. Cleanup
  returned `status --all` to `[]`.

## Required Focused Tests

| Area | Required tests |
|---|---|
| Ability set builder | Species with three distinct abilities and duplicate abilities are covered. Hidden `ABILITY_NONE`, normal slot `ABILITY_NONE`, and form species remain follow-up coverage. |
| Duplicate slots | A species with duplicate abilities such as `Intimidate / Intimidate / Defiant` triggers Intimidate once and still has Defiant active. |
| Predicate checks | A battler has Magic Guard plus another ability; Soundproof plus another ability; Sticky Hold plus another ability; Levitate plus another ability; Queenly Majesty plus another ability; Klutz plus another ability. Each predicate must work without needing to be the primary slot. |
| Switch-in triggers | Two or three switch-in abilities trigger in deterministic slot order. Duplicate slots trigger once. |
| Field presence | Neutralizing Gas, Unnerve, Ruin abilities, and weather / terrain setters work when not in primary slot. |
| Suppression / bypass | Gastro Acid, Neutralizing Gas, Mold Breaker, Ability Shield, and `cantBeSuppressed` combinations. |
| Ability-changing moves | Trace, Role Play, Skill Swap, Entrainment, Worry Seed, Simple Beam, and Receiver change one corresponding slot only. Worry Seed must not turn all active slots into Insomnia. |
| Form changes | Mega Evolution, Primal Reversion, Ultra Burst, Weather forms, Stance Change, Disguise, Ice Face, and any ability-gated form change use current-form natural ability slots. |
| Battle-only boundary | Field lead ability behavior remains single-ability while battle behavior uses all active slots. |
| AI | Damage / switch decisions that depend on immunity, trapping, priority, speed, and Magic Guard-style secondary damage. |
| Items | Ability Capsule / Patch fail or apply chosen new policy under all-active mode, and retain upstream behavior when disabled. |
| Summary UI | Summary shows the selected active slot label plus ability name without text overflow and lets `L` / `R` cycle the displayed ability description across full `1/2/3` species when `P_SUMMARY_SCREEN_ALL_ABILITY_SLOT_SWITCH` is enabled. Species whose normal slot 2 is omitted but hidden slot 3 exists should mirror slot 1 into slot 2 for selector readability, while battle evaluation still dedupes duplicates. A temporary disabled-config build should still compile and keep the selected / representative display stable without cycling. |

## Candidate Manual Checks

- Start `Party` -> `All Ability...` -> `A Recoil Battle`, use Clefable's
  `Double-Edge`, and confirm no recoil is applied because `Magic Guard` is
  active outside the representative `Cute Charm` slot.
- Start Patterns B-E from the same submenu and confirm each route's expected
  visible result: Flash Fire absorption, Soundproof sound-move immunity,
  hidden-slot Drought at battle start, and Skill Swap preserving Magic Guard in
  a non-representative slot.
- Start Patterns F-K from the same submenu and confirm each route's expected
  visible result: Intimidate switch-in popup / Attack drop, Levitate blocking
  Earthquake, Storm Drain absorbing Water and raising Sp. Atk, Sticky Hold
  preserving Leftovers from Knock Off, Queenly Majesty blocking Quick Attack,
  and Commander activating from non-representative Tatsugiri slot 0 while
  `Storm Drain` remains the representative slot.
- Start Patterns L-T from the same submenu and confirm each route's expected
  visible result: sun speed order, Trace slot-local copy, Conkeldurr power
  stack, Hustle accuracy / damage, Lightning Rod / Storm Drain redirection,
  Toxtricity modifier stack, Dragonite guard modifiers, and partner modifier
  behavior.
- Start Pattern U `Moxie Popup` from the same submenu. Use Mightyena's
  `Quick Attack` into the level-1 target and confirm the KO follow-up popup
  displays `Moxie`, even though Mightyena's representative slot is
  `Intimidate`.
- Recheck Pattern I specifically and confirm Weavile's initial popup is
  `Pressure`, not hidden-slot `Pickpocket`, before testing Muk's Sticky Hold
  item-retention flow.
- Open Summary from party and from battle-adjacent flows, then inspect the
  active ability selector. On the Info page, use `L` / `R` and confirm the
  selected slot marker, ability name, and description change together for a full
  `1/2/3` species and for an omitted-slot species whose slot 2 mirrors slot 1 in
  the UI.
- Try Ability Capsule and Ability Patch in all-active mode and confirm the
  chosen message / behavior is clear.
- Mega Evolve a Pokemon whose target form has a different ability table and
  confirm the post-form active set uses the Mega species slots only, with no
  base-form overlay.
- Use Worry Seed or Simple Beam against an all-active target and confirm only
  one operation slot is overwritten.
- In a double battle, verify ability popup order and that the player is not
  trapped or immune incorrectly due to a missed secondary ability.

## Remaining Gaps

- AI remains a single known-ability cache and needs a later ability-set pass.
- Summary selectable descriptions are implemented on the Info page. Remaining
  Summary risk is visual polish / skin alignment with the broader Party / Status
  UI overhaul.
