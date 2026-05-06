# mGBA Live Init And Build Validation

## Purpose

この文書は、clean save 状態から mGBA Live CLI で起動し、debug command を使って wild battle / trainer battle まで到達できるかを確認した記録である。

runtime feature の検証では、既存 save がある前提で始めない。save が必要な場合は user に save / savestate を求め、save がない場合は new game setup から検証時間に含める。

## 2026-05-06 Result

Environment:

- `nproc`: `20`
- Branch: `docs/mgba-init-build-validation`
- ROM under test: debug build output copied to `/tmp/mgba-init-build-validation.bQ3jtl/pokeemerald-nosave.gba`
- mGBA binary: `.cache/mgba-script-build-master/qt/mgba-qt`
- Live CLI: `/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli`
- Session: `codex-mgba-init-nosave`
- FPS config: `--fps-target 120 --config videoSync=1`

The direct MCP start attempt failed because Qt/xcb could not connect to a display when `DISPLAY` was not present in that launch environment. The working path was the CLI with explicit display env:

```bash
rtk env DISPLAY=:0 /home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli start \
  --rom /tmp/mgba-init-build-validation.bQ3jtl/pokeemerald-nosave.gba \
  --session-id codex-mgba-init-nosave \
  --mgba-path /home/jastin/dev/pokeemerald-expansion/.cache/mgba-script-build-master/qt/mgba-qt \
  --fps-target 120 \
  --ready-timeout 20 \
  --config videoSync=1
```

| Check | Result | Evidence / notes |
|---|---|---|
| CPU headroom | passed | Host reported 20 logical workers. |
| `make -j16` | passed | Existing linker warning: `LOAD segment with RWX permissions`. |
| `make debug -j16` | passed | Same existing linker warning. The debug ROM was used for mGBA validation. |
| no adjacent `.sav` boot | passed | Fresh `/tmp` ROM copy had no sibling `.sav`. Title opened with `NEW GAME` / `OPTION`, not Continue. Screenshot: `/tmp/mgba-init-build-validation.bQ3jtl/boot-nosave-title.png`. |
| new game setup | passed | Went through Birch intro, entered name `A`, reached truck, exited truck, then reached field control in the player's house. |
| debug menu open | passed | `R + START` opened the overworld debug menu after field control was available. |
| `Utilities... > Cheat start` | passed | `Debug_CheatStart` ran from `data/scripts/debug.inc`. |
| valid party after cheat start | passed | Party screen showed Treecko Lv20, Torchic Lv20, Mudkip Lv20. No `????` / invalid placeholder was observed. Screenshot: `/tmp/mgba-init-build-validation.bQ3jtl/cheat-start-party.png`. |
| wild battle | passed | Route 101 grass produced Wurmple Lv2. `gBattleTypeFlags` read `4`; `gBattleMons[0].species` was `252` (Treecko), `gBattleMons[1].species` was `265` (Wurmple). Screenshot: `/tmp/mgba-init-build-validation.bQ3jtl/wild-wurmple-battle.png`. |
| trainer battle | passed | `Party... > Start Debug Battle` started battle against `PKMN TRAINER Debugger`. Screenshot: `/tmp/mgba-init-build-validation.bQ3jtl/debug-trainer-battle.png`. |
| cleanup | passed | `mgba-live-cli status --all` returned `[]` after stopping the session. |

## Build Parallelism

This workspace has enough worker headroom for `make -j16`. Use it for normal local build and debug build when the host has around 20 logical workers.

Recommended local commands:

```bash
make -j16
make debug -j16
```

In a Codex session for this workspace, keep the repository's shell wrapper rule and run the same commands through `rtk`:

```bash
rtk make -j16
rtk make debug -j16
```

`make -j4` remains the conservative fallback for weak machines, thermal throttling, or when compiler/linker failures look resource-related. If `-j16` fails and `-j4` passes, record both results with the host CPU count and failure excerpt.

## No-Save Startup Procedure

Do not delete or rename user saves in the repository root unless the exact file is known to be the active emulator save for the ROM being tested. In this workspace, `git ls-files "*.sav"` returned empty and the root `.sav` files are ignored by `.gitignore`; they were not touched.

Safer clean-save setup:

1. Create a fresh directory under `/tmp`.
2. Copy the target ROM there with a unique basename.
3. Confirm there is no sibling `.sav` for that basename.
4. Start mGBA Live CLI with that ROM copy.
5. Keep screenshots, ROM copies, save files, and Lua scratch files in `/tmp`; do not commit them.

If an exact sibling save does exist and must be detached, rename only that artifact in the temporary validation directory, for example `pokeemerald-nosave.sav.disabled`. Avoid renaming unrelated saves such as `pokeemerald1.sav` or copied backup saves.

No in-game save was performed in this validation, so the temporary directory still had no `.sav` after cleanup. If a future check needs save/load behavior, force an in-game save in the temporary ROM directory and record that the generated `.sav` is a local artifact only.

## New Game And Debug Setup Notes

From a clean save, mGBA does not start in a useful field state. The observed route was:

1. Title screen.
2. `NEW GAME`.
3. Birch intro text.
4. Name entry. For smoke validation, entering a one-letter name (`A`) was enough.
5. Truck scene.
6. Exit truck.
7. Mom intro inside the player's house.
8. Field control.
9. Open overworld debug menu with `R + START`.

The debug menu is available only after field control. Do not assume it can bypass the earliest intro text or the truck setup. If a feature needs a late-game map and no save is provided, include this setup cost in the validation plan.

## Debug Commands Used

`Utilities... > Cheat start` runs `Debug_CheatStart` from `data/scripts/debug.inc`. It sets early-game progress flags, gives Treecko / Torchic / Mudkip at level 20, enables Pokédex / National Dex flags, adds travel conveniences, and marks all badges.

`Party... > Start Debug Battle` does not reuse the current cheat-start party. It runs `DebugAction_Party_BattleSingle` in `src/debug.c`, which:

- clears player and enemy parties;
- creates the player party from `DEBUG_TRAINER_PLAYER`;
- creates the enemy party from `DEBUG_TRAINER_AI`;
- starts battle against `PKMN TRAINER Debugger`.

The source party data is `src/data/debug_trainers.party`. The observed debug trainer battle used `DEBUG_TRAINER_PLAYER` Wobbuffet and `DEBUG_TRAINER_AI` Metang / Skarmory / Aggron, so this path is suitable for smoke-checking trainer battle startup even when the clean save has no starter.

## Runtime Evidence Tips

Screenshots alone are not enough for behavior checks, but they are useful when paired with memory or route state.

For this validation, the useful memory anchors were:

- `gBattleTypeFlags`: `0x020000b8`
- `gBattleMons`: `0x02000424`
- `gMain`: `0x03006704`
- `gSaveBlock1Ptr`: `0x03005208`

`gBattleMons` occupied `0x220` bytes for 4 battlers in this build, so the observed stride was `0x88`.

Do not use `gBattleTypeFlags` alone as final proof after multiple sequential battles. In this session, the debug trainer battle screen was reliable evidence that trainer battle started, while bit state should be treated as supporting data and checked with the visible battle screen or callback state.

## Cleanup

Always clear input and stop the session before finishing:

```bash
mgba-live-cli input-clear --session codex-mgba-init-nosave
mgba-live-cli stop --session codex-mgba-init-nosave
mgba-live-cli status --all
```

Expected final status:

```json
[]
```
