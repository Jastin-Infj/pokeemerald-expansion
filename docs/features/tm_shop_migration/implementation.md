# TM Shop Migration Implementation

更新日: 2026-06-02

## Summary

This implementation retires the legacy Gen 3 TM acquisition routes and Emerald
normal-progression HM item grants while keeping core TM/HM item machinery
intact.

Implemented:

- Removed Emerald NPC / gym / story `giveitem ITEM_TM_*` reward paths and their
  legacy TM received-flag gates.
- Removed Emerald visible TM item balls and the Route 113 hidden TM pickup from
  `map.json` owners.
- Retired Emerald TM shop / prize sources: Lilycove Department Store 4F,
  Slateport Power TM counter, Mauville Game Corner TM counter, Trainer Hill TM
  prizes, Lilycove Quiz Lady Hyper Beam prize, and Trick House Taunt rewards.
- Renamed now-unreferenced Emerald legacy TM flags to `FLAG_UNUSED_0x...`
  names without changing values.
- Added a replacement story flag, `FLAG_RETURNED_METEORITE_TO_COZMO`, at the
  previously unused `0xE9` slot so Prof. Cozmo no longer depends on
  `FLAG_RECEIVED_TM_RETURN`.
- Preserved old-save compatibility for the Meteorite return path by keeping
  the retired `0xE5` bit as `FLAG_LEGACY_RECEIVED_TM_RETURN`. Cozmo and his
  wife migrate that bit to `FLAG_RETURNED_METEORITE_TO_COZMO` on contact, but
  the removed TM Return reward is not restored.
- Added a debug-only `TM Shop Test` route under Debug menu `Scripts...` using
  `pokemart` and `.2byte ITEM_TM_*` entries. This route is for item-id-width /
  shop UI validation only and is not normal progression.
- Set `I_REUSABLE_TMS` to `TRUE`, so any retained physical TM item behaves like
  a reusable Gen 5+ TM.
- Removed Emerald normal-progression `giveitem ITEM_HM_*` / unused
  `finditem ITEM_HM_DIVE` grants.
- Removed Emerald runtime references to old HM receive flags and renamed those
  flag values to `FLAG_UNUSED_0x...`. Required story branches now use existing
  story vars / flags such as `VAR_PETALBURG_CITY_STATE`,
  `VAR_SOOTOPOLIS_WALLACE_STATE`, `FLAG_RUSTURF_TUNNEL_OPENED`, and
  `FLAG_OMIT_DIVE_FROM_STEVEN_LETTER`.

Not implemented in this slice:

- Core `FOREACH_TM`, `ITEM_TM*`, TM/HM pocket, teachability, and HM field-move
  behavior are otherwise unchanged.
- Gen 9 / 200+ reusable physical TM implementation is not part of this slice.
- Full HM modernization / field-move unlock redesign is not part of this slice.
- FRLG-specific legacy TM / HM sources using `ITEM_TM03`, `ITEM_HM01`, and
  `FLAG_GOT_TM*` / `FLAG_GOT_HM*` patterns remain as a separate follow-up
  scope.

## Files Changed

| Area | Files |
|---|---|
| Debug shop route | `src/debug.c`, `data/scripts/debug.inc` |
| Reusable TM config | `include/config/item.h` |
| Emerald gym / NPC scripts | Rustboro, Dewford, Mauville, Lavaridge, Petalburg, Fortree, Mossdeep, Sootopolis gym scripts plus affected NPC / story scripts under `data/maps/` and `data/scripts/secret_power_tm.inc` |
| Emerald HM grants | `RustboroCity_CuttersHouse`, `GraniteCave_1F`, `MauvilleCity_House1`, `RusturfTunnel`, `PetalburgCity_WallysHouse`, `Route119`, `MossdeepCity_StevensHouse`, `SootopolisCity` scripts |
| Emerald field pickups | `data/maps/*/map.json` owners for 14 visible TM balls and Route 113 hidden Double Team |
| Emerald shops / prizes | `data/maps/LilycoveCity_DepartmentStore_4F/scripts.inc`, `data/maps/SlateportCity/scripts.inc`, `data/maps/MauvilleCity_GameCorner/scripts.inc`, `src/trainer_hill.c`, `src/data/lilycove_lady.h` |
| Flags | `include/constants/flags.h`, `include/constants/flags_frlg.h` |

## Validation

Passed:

- `rtk git diff --check`
- `rtk mdbook build docs` with existing warnings for missing root
  `CHANGELOG.md` include, existing `CREDITS.md` `</img>`, and large search
  index
- JSON owner validation for edited `map.json` files with `python3 -m json.tool`
- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- `rtk make -j16 -O check`
- Static search found no Emerald `giveitem ITEM_HM_*` / `finditem ITEM_HM_*`
  grants. Remaining direct HM grants are FRLG-specific follow-up scope.
- Static search found no old HM receive flag references in `include`, `data`,
  `src`, or `docs`; old HM receive flag values are now `FLAG_UNUSED_0x...`.
- Static search confirmed `I_REUSABLE_TMS TRUE`.

mGBA Live:

- Initial validation started `pokeemerald.gba` with
  `/home/jastin/.local/bin/mgba-qt` and reached the Pokemon Emerald title
  screen, but input did not advance during that earlier session. Stop returned
  `stopped: false`; the process was killed manually and remained visible as a
  defunct child (`[mgba-qt] <defunct>`) in `pgrep`.
- Follow-up validation after enabling reusable TMs / removing Emerald HM item
  grants started `tm-shop-migration-hm-followup`, reached title, continued into
  an existing save, and opened the in-game Start menu. The session stopped
  cleanly with `stopped: true`.
- No new mGBA route was run after renaming old HM receive flags to unused; this
  was covered by static search plus `all` / `debug` / `check` rebuilds.
- Feature-specific Rustboro Cutter / HM source NPC behavior was not confirmed
  in mGBA because the loaded save was not positioned on that route.

## Integration Adoption: 2026-05-30

Runtime branch: `integration/runtime-dev-20260529`

Source shelf: `feature/tm-shop-migration`, commit
`eb486f6356 Implement TM shop migration`

Conflict choices:

- Kept Pokemon Vendor on `Debug_EventScript_Script_1`.
- Moved the TM debug shop route to `Debug_EventScript_TmShopTest`.
- Renamed the pokemart item list to `Debug_EventScript_TmShopTestItems` so the
  route label and list label do not collide.
- Added a named `TM Shop Test` entry to Debug menu `Scripts...`.
- Preserved `FLAG_NO_ENCOUNTER` from the no-random integration while applying
  the TM/HM legacy flag retirements.

Validation on the integration branch:

| Check | Result |
|---|---|
| `rtk git diff --check` / `rtk git diff --cached --check` | Passed. |
| Edited `map.json` parse check | Passed with `python3 -m json.tool` over the staged map JSON files. |
| Static TM/HM search | Passed for Emerald normal progression. Remaining `ITEM_TM_*` / `ITEM_HM_*` routes are FRLG follow-up scope or the debug-only `TM Shop Test`. |
| `rtk make -j16 -O all` | Passed with existing RWX linker warning. |
| `rtk make -j16 -O debug` | Passed with existing RWX linker warning. |
| `rtk make -j16 -O check` | Passed with existing EXPECTED_FAIL / KNOWN_FAILING markers. |
| mGBA Live smoke | Passed. Session `integration-tm-shop-smoke` booted `pokeemerald.gba`, continued the local save, opened Debug menu, navigated to `Scripts...`, selected the new `TM Shop Test` route, and captured screenshots. |
| mGBA Live cleanup | Passed. `mgba-live-cli stop` returned `alive_after:false`; final `status --all` returned `[]`. |

Screenshots:

- `/tmp/integration-tm-shop-debug-menu.png`
- `/tmp/integration-tm-shop-scripts-menu.png`
- `/tmp/integration-tm-shop-open.png`

The integration smoke validates the debug route entry and script handoff. It
does not replace the broader static audit for every retired NPC, map pickup,
shop, and prize source.

## 16.0 Replay Adoption: 2026-05-31

Runtime branch: `integration/runtime-dev-16-20260531`

The 16.0 replay keeps the upstream 16.0 script baseline authoritative, then
reapplies the Emerald TM / HM source retirement. `codex review --base master`
found one New Mauville flow regression from the replay: moving Wattson back to
the Gym during `MauvilleCity_OnTransition` made the post-generator city
completion conversation unreachable. The transition hook was removed; the
existing `MauvilleCity_EventScript_CompletedNewMauville` conversation remains
the only place that flips Wattson back to the Gym.

Validation after the fix:

| Check | Result |
|---|---|
| `rtk make -j16 -O all` | Passed with existing RWX linker warning. |
| `rtk make -j16 -O debug` | Passed with existing RWX linker warning. |
| `rtk make -j16 -O check` | Passed with existing expected / known-failing markers. |

`codex review --base master` on 2026-06-02 found one additional 16.0 replay
compatibility issue: saves that had already returned the Meteorite in 15.3 used
the old `0xE5` TM Return reward flag, while the replay only checked the new
`FLAG_RETURNED_METEORITE_TO_COZMO` bit. The script now checks the legacy alias
first and sets the new story flag before continuing to the normal "Meteorite
already returned" dialogue. This keeps 16.0's new story flag authoritative
without sending old saves back into the removed reward branch.

Validation after the Meteorite compatibility fix:

| Check | Result |
|---|---|
| `rtk git diff --check` | Passed. |
| `rtk make -j16 -O all` | Passed with existing RWX linker warning. |
| `rtk make -j16 -O debug` | Passed with existing RWX linker warning. |
| `rtk make -j16 -O check` | Passed with existing expected / known-failing markers. |
| `rtk mdbook build docs` | Passed with existing `CHANGELOG.md` include, `CREDITS.md </img>`, and large search-index warnings. |
| mGBA Live boot smoke | Passed. Session `runtime-dev-16-audit-20260602` booted `pokeemerald.gba`, returned `true` through Lua, captured `/tmp/runtime-dev-16-audit-20260602.png`, stopped cleanly, and final `status --all` returned `[]`. |

The exact legacy Cozmo dialogue branch was source-audited but not walked in
mGBA; validating that path directly requires a save fixture with old flag
`0xE5` set and `FLAG_RETURNED_METEORITE_TO_COZMO` clear.

## Remaining Risks

- FRLG-specific legacy TM acquisition remains. If this feature should cover
  FireRed / LeafGreen maps too, retire those `ITEM_TM03` style sources in a
  separate pass and validate with a FRLG build route.
- The debug-only shop still provides TMs by design. Static searches must exclude
  `data/scripts/debug.inc` when checking normal progression.
- Field-move capability flags are now owned by the field-move feature. Do not
  reuse the retired HM receive flag values without a deliberate save migration
  decision.
- FRLG-specific HM gifts still exist and must be retired separately if FRLG maps
  enter scope.
- Several old TM explanation text labels remain unreferenced in scripts. They do
  not grant items, but can be removed in a cleanup pass if text size or audit
  noise matters.
