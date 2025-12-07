# Repository Guidelines

## Project Context
- Fork of `rh-hideout/pokeemerald-expansion` (`master`). Base is the Japanese “Pocket Monsters Emerald” ROM, decompiled and extended to Gen 9 mechanics.
- Upstream changes fast; names/constants/logic can differ per tag/branch. Cross-check your target branch/changelog and consult upstream issues/forks when behavior differs.

## Upstream & Community
- Primary source: https://github.com/rh-hideout/pokeemerald-expansion — treat as canonical; watch both `master` and `upcoming` (beta, high churn/issues daily).
- Community hub: RHH Discord https://discord.gg/6CzjAG6GZk — active discussion, beta issues, and #pr-discussions threads.

## Discord #expansion-get-help Notes
- Export file (when present): `RH Hideout - ★ Expansion ★ - expansion-get-help [1077168246555430962].txt`; context spans many versions—double-check against your branch and prefer recent posts (older advice can be wrong as APIs change). File is a local reference only; do not commit.
- High-signal responders: Alex (often posts fixes/diagnoses), rainonline, asparaguseduardo (Eduardo), and mgriffin frequently deliver reliable answers—prioritize their solutions when skimming.
- Debug logging: `DebugPrintf` only works if you comment out `#define NDEBUG` in `include/config.h`; `MgbaPrintf_` is a test helper.
- Toolchain mismatch: `__getreent`/libc undefined refs reported with older devkitARM/libc; use the supported devkitARM from INSTALL.md.
- De-evolution logic: no reverse pointer in evo data; use a linear search like daycare `GetEggSpecies` rather than hacking extra evo entries.
- Mega evolution items: if swapping required items, set the item’s `holdEffect` to `HOLD_EFFECT_MEGA_STONE` or ME won’t trigger.
- Form-change sprites: multiple form changes (Minior/Cramorant/Castform) sticking on old sprite tracked in issue #8457 (seen on `upcoming`).
- Ability info flags: Water Bubble missing `breakable` flag—add it to the breakable list and tests when auditing ability data.
- DNS/window colors: palette glitches tied to window template sizing; fix the window template width when applying DNS UI palettes.
- Starting items: add key items in `data/scripts/new_game.inc` via the `additem` macro.
- Teachable learnsets by gen: remove moves from `sv.json` (teachable data) to drop Gen 9-only moves; upcoming will ease removals.
- Concrete examples from the export file (verify against your branch before using):
  - 2023-07-16: When removing EVs from `CALC_STAT`, keep the macro signature and only drop the term in the body: change the line inside the macro to `s32 n = (((2 * baseStat + iv) * level) / 100) + 5;` without deleting the `ev` parameter or call sites.
  - 2023-07-20: To make a Rare Candy-style key item not consumed, remove the `RemoveBagItem` call in `ItemUseCB_RareCandy` or guard it with `if (!ItemId_GetImportance(gSpecialVar_ItemId))` before removing the item after the out-of-battle effect runs.
  - 2023-11-01: Species checks need full comparisons; e.g. for Deoxys forms use chained comparisons:
    ```
    if (gBattleMons[gBattlerAttacker].species == SPECIES_DEOXYS
     || gBattleMons[gBattlerAttacker].species == SPECIES_DEOXYS_DEFENSE
     || gBattleMons[gBattlerAttacker].species == SPECIES_DEOXYS_SPEED)
        return TRUE;
    ```
  - 2023-11-01: The infinite repel item uses `OW_FLAG_NO_ENCOUNTER`; give the flag an ID in `include/overworld.h` (not just `flags.h`) or the toggle will no-op.
  - 2023-03-08: `B_FLAG_NO_CATCHING` lives under `include/config/overworld.h` after the split; define the flag there and fix conditionals referencing `B_FLAG_NO_CATCHING_USE` by using `#if B_FLAG_NO_CATCHING == 0` (or defining the `_USE` macro) to avoid build errors.
  - 2023-07-19: Adding the move relearner menu entry requires inserting `MENU_MOVES` into the `enum` in `src/party_menu.c` (include the trailing comma before it, and keep `MENU_FIELD_MOVES` as the last enumerator since field moves are appended by offset). Also add the `MENU_MOVES` entry to `sCursorOptions` and any preset arrays that display it.
  - 2023-02-23: `MgbaPrintf_` links only in tests; for regular debug logs comment out `#define NDEBUG` in `include/config.h` and call `DebugPrintf(...)` instead to avoid `undefined reference to 'MgbaPrintf_'`.
  - 2023-07-16 & 2023-08-19: If linking fails with `gItemIcon_*` or palette symbols (e.g. Lustrous Globe, Berserk Gene), ensure the icon/palette entries remain in `graphics/items.h` and `src/data/item_icon_table.h` (and corresponding gfx exist) when merging feature branches.
  - 2023-11-01: `MoveSelectionDisplayMoveType` needs a local `typeColor` declared and initialized at the top of the function before use; missing that line triggers `typeColor undeclared` build errors.
  - 2023-11-11: For a permanent Trick Room effect, set the timer to `-1` instead of a duration, e.g. `gFieldTimers.trickRoomTimer = -1;`.
  - 2023-12-04: Script var comparisons need the `var()` macro closed on the var name, not the value; e.g. `if (var(VAR_GURU_TRAINER_ITEMS) == 0)` or `compare VAR_GURU_TRAINER_ITEMS, 0` (not `compare VAR_GURU_TRAINER_ITEMS == 0, 0`), which otherwise throws “confusion in formal parameters.”
  - 2024-05-01: To drive dynamic level caps, set `B_LEVEL_CAP_VARIABLE` in `include/config/level_caps.h` (e.g. `#define B_LEVEL_CAP_VARIABLE VAR_UNUSED_0x404E`) and adjust caps in scripts with `setvar VAR_UNUSED_0x404E, <level>`.
  - 2024-06-01: Expanding the Hoenn Dex past Deoxys requires bumping the count macro: `#define HOENN_DEX_COUNT (HOENN_DEX_DEOXYS + 1)` (or higher) so new species indices appear.
  - 2024-06-01: When adding extra fishing slots (3/4/3 per rod) the selection logic works with chained `else if` blocks and updated thresholds; keep the lure swap logic consistent when mirroring slots.
  - 2024-09-01: If `trainerproc` errors on `src/data/battle_partners.party` under MSYS2, disable the new party syntax via `#define COMPETITIVE_PARTY_SYNTAX FALSE` in `include/config/battle.h` (or update the party file to the new syntax) before rebuilding.
  - 2024-12-30: Clearing lingering windows after switching pages in custom UIs: after removing a window, call `FillWindowPixelBuffer(windowId, 0)` or clear the tilemap so the old window’s tiles stop rendering when new templates are loaded.
  - 2025-01-01: To cap quadruple effectiveness at ×3, apply the clamp in `MulByTypeEffectiveness` rather than `GetTypeEffectiveness` (the latter is overworld-only); add a post-calculation check that converts ×4 multipliers to ×3, accounting for third-type modifiers if present.
  - 2025-01-01: Level caps driven by the flag list need `#define B_LEVEL_CAP_TYPE LEVEL_CAP_FLAG_LIST` in `include/config/level_caps.h`; then set the per-flag caps in the list and raise them via flags or scripts.
  - 2025-01-01: Cave-style warps must use a tile with the level-change collision behavior (not a blocked tile); otherwise the warp won’t trigger even if the event is defined.
  - 2025-01-01: Item icon palettes can be generated with the built-in converter: `tools/gbagfx/gbagfx <input>.png <output>.pal`; add the resulting palette alongside the icon in `graphics/items.h` and `src/data/item_icon_table.h`.
  - 2025-01-01: Summary screen page tilemaps (e.g. `graphics/summary_screen/page_info.bin`) are authored in TilemapStudio using `tiles.png` as a GBA 4bpp tileset, then exported to `.bin` and referenced from the UI source.
  - 2025-01-01: If `gbagfx` is not found on PATH, call it via its repo-local path `tools/gbagfx/gbagfx` for any palette/tileset conversions.
  - 2025-02-01: Multi-battle gimmick selection bug—`ShouldTrainerBattlerUseGimmick` misread the partner’s trainer ID. Fix by selecting `trainerId`/`partyIndexes` based on `BATTLE_TYPE_INGAME_PARTNER` and `BATTLE_TYPE_TWO_OPPONENTS`, and subtract `MULTI_PARTY_SIZE` for right-side/partner battlers when indexing `GetTrainerPartyFromId(trainerId)[partyIndexes]`.
  - 2025-02-01: `B_POSITION_LEFT_RIGHT` is not defined in some branches; use `B_POSITION_PLAYER_RIGHT` (or equivalent) in the above fix to compile.
  - 2025-03-01: After merging 1.11, regenerate `trainers.h` by touching `trainers.party` (any edit) so `trainerproc` rebuilds; unresolved merges there lead to “million errors from trainers.party.”
  - 2025-04-01: Overworlds missing post-migration often trace to a reverted custom `object_event` struct; reapply the project’s conditional-trainer additions or adopt the new built-in RHH solution instead of mixing both.
  - 2025-06-01: Some tilesets (e.g. Rustboro/Mauville) shipped with incorrect byte sizes; opening/saving the tileset in Porymap (without TLM) rewrites to the correct size as a temporary fix until the tileset is reconverted.
  - 2025-06-01: Party menu build error from `static` in `party_menu.h` presets—remove the `static` qualifier where the upstream header changed to non-static to match symbol visibility and unblock builds.
  - 2025-02-01: Adding new glyphs (e.g. å/ä/ö) requires defining them in `charmap.txt`; unknown characters throw “unknown character U+E5” until mapped.
  - 2025-03-01: Conflicts when merging ipatix audio changes with 1.11—keep the upstream deletions in `m4a_1.s`/`m4a.c` and reapply local tweaks after resolving; recompile to ensure the sound engine still builds.
  - 2025-05-01: Shields Down custom species—extend `IsShieldsDownProtected` and form-change logic to include the new species/forms, and add the unleashed form to the form table so stats/appearance swap when HP drops.
  - 2025-07-01: To lock a door until a variable/flag is set, either swap the door metatile to a non-door behavior via `setmetatile` or place an invisible NPC (see Team Aqua submarine door) to block until the condition clears.
  - 2025-07-01: New map section setup—define a `MAPSEC_*` in `include/constants/region_map_sections.h` (or via Porymap 6 “Locations” tab) and place it on the region map with the Region Map Editor to avoid missing/incorrect mapsec IDs.
  - 2025-08-01: AI sleep handling—`IsBattlerIncapacitated` only flagged Sleep Talk; add Snore (and similar) so AI scores support moves correctly in sleep (e.g. Follow Me/Helping Hand not treated as unusable).
  - 2025-08-01: New TM/HM entries need adding to `include/constants/tms_hms.h` and associated data tables; missing constants cause “additional info” prompts or build errors when introducing custom TMs.
  - 2025-08-01: Triple-layer metatile artifacts (roulette/gacha) stem from bad triple-layer definitions; verify the metatile behaviors/layers for those tiles and reconvert the tileset in Porymap to fix incorrect layer ordering.

## Reference Resources
- https://github.com/pret/pokeemerald/wiki/Tutorials — vanilla Emerald decomp (non-expansion); good for locating original behaviors and small features that may already be upstreamed here.
- https://github.com/TeamAquasHideout/Team-Aquas-Asset-Repo/wiki/Feature-Branches — “The Pit” feature branches with high-quality modules; many additions for v13 were adapted from here.
- https://github.com/TeamAquasHideout/pokeemerald — Team Aquas base repo; browse branch diffs for self-contained features.
- https://github.com/PCG06/pokeemerald/tree/move_relearner — reference for move relearner and related logic; some pieces are already in expansion.
- Expansion-based game forks worth inspecting for ideas/diffs: https://github.com/resetes12/pokeemerald , https://github.com/Pokabbie/pokeemerald-rogue/ , https://www.pokecommunity.com/threads/the-pit-v2-roguelite-style-hack.528423/ , https://www.pokecommunity.com . FireRed and RPGXP projects can also be mined for portable logic.

## Project Structure & Module Organization
- Core code in `src/` (C) and `include/`; shared constants in `constants/`; assembly overlays in `asm/`; data tables in `data/`.
- Assets: `graphics/`, `sound/`, and rules files (`graphics_file_rules.mk`, `audio_rules.mk`, `map_data_rules.mk`, `trainer_rules.mk`). Outputs land in `build/<target>/` plus `pokeemerald.gba`/`.elf`.
- Tests live in `test/`; tools and scripts in `tools/` and `dev_scripts/`. Docs in `docs/` (see `docs/tutorials/how_to_testing_system.md`).

## Build, Test, and Development Commands
- `make` — build the ROM (add `-j$(nproc)` to parallelize). Flags: `DEBUG=1`, `RELEASE=1`, `COMPARE=1`, `LTO=1`.
- `make check -j` — run mgba-based tests.
- `make check TESTS="Spikes"` — filter tests; same filter works with `make pokeemerald-test.elf` to build an mGBA test ROM.
- `make debug` — emit `pokeemerald.elf` with debug symbols for GDB/mGBA.
- `make -j12` (or `-j$(nproc)`) stability: if little/no output after ~5s, interrupt and rerun; if logs stream, let it finish. After aborted runs, delete zero-byte `.o` files (`find build/emerald -name '*.o' -size 0 -delete`) or `make clean` before retrying to avoid undefined references.

## Coding Style & Naming Conventions
- Follow `docs/STYLEGUIDE.md`: functions/structs `PascalCase`; variables/fields `camelCase`; globals `gPrefix`, statics `sPrefix`; macros/constants `CAPS_WITH_UNDERSCORES`.
- C/C headers use 4 spaces; `.s` and `.inc` use tabs. Place braces on new lines after control statements, with a space before parentheses (`if (cond)`).
- Comments should record intent/behavior rather than restating code; use `// ` for short notes and block comments for deeper context.

## Testing Guidelines
- Prefer automated coverage in `test/` using `GIVEN/WHEN/SCENE`; mirror player-visible effects. Prefix test names with the mechanic for easy filtering.
- Document prerequisites with `ASSUME(...)`; mark known regressions with `KNOWN_FAILING; // #issue`.
- For visual/debug investigation, build `pokeemerald-test.elf` and inspect in mGBA before finalizing assertions.

## Commit & Pull Request Guidelines
- Use concise commit titles (history often follows `Summary (#PR)`). Keep commits scoped.
- Target `master` for fixes; `upcoming` for new features/large changes. Work on topic branches.
- Run `make` and `make check -j` before opening a PR; note any config toggles or skipped tests. Avoid force-pushing after review begins; append commits instead.
- PR description should include scope, repro steps, linked issues/Discord context, and screenshots or notes when altering visible behavior.
