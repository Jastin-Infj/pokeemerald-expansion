# Repository Guidelines

## Project Context
- Fork of `rh-hideout/pokeemerald-expansion` (`master`). Base is the Japanese “Pocket Monsters Emerald” ROM, decompiled and extended to Gen 9 mechanics.
- Upstream changes fast; names/constants/logic can differ per tag/branch. Cross-check your target branch/changelog and consult upstream issues/forks when behavior differs.

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
