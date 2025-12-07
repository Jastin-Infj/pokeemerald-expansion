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
