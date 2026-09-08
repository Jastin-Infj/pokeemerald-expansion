# UI style switch verification

Verified 2026-09-09 on the local feature worktree.

## Rounded HP endpoint follow-up

- `rtk make -j16 -O debug` and `rtk make -j16 -O all`: exit 0;
  existing RWX linker warning only. `git diff --check` passed.
- Matching debug ROM in script-capable mGBA: single player/opponent and all four
  doubles endpoints visually inspected. Evidence: `evidence/hp-cap/single.png`
  and `full.png`, `yellow.png`, `red.png`, `empty.png`, `almost-full.png`.
- Doubles samples set party HP/max HP to 100/100, 40/100, 10/100, 0/100 and
  98/100, then called the production HUD redraw using the existing
  `ui_contrast_fixture.py` probe. Corners stay transparent and the dark endpoint
  stays closed in all samples. These are display fixtures, not damage simulation.
- Real B/START/START input restored the bars after numeric display;
  `evidence/hp-cap/toggle-restored.png` retains the rounded ends.
- MCP tools were unavailable; used the project mGBA Live CLI. Both sessions
  stopped successfully; `status --all` returned `[]`.
- HP arithmetic and source graphics are unchanged. Default rendering is excluded
  by `IsUiStyleXY()`; this follow-up did not repeat default/Safari playthroughs
  or damage/EXP logic tests. Long GitHub Actions runs were not awaited.
- Final normal ROM SHA256:
  `7620e249261d8d0edb5458ce51bd79860d5a22acba34b8800502492e04cce700`.

| Requirement | Result / evidence |
|---|---|
| Original and XY assets coexist | All 15 default PNGs match `2c51798fe1^` byte-for-byte, and all 15 `xy_` PNGs match the corresponding original XY commit sheets. |
| Normal ROM | Initial `rtk make -j16 -O all` exit 0. Final `rtk make -j16 -O all` also exit 0 (`/tmp/ui-style-final-all.log`). |
| Debug ROM | `rtk make -j16 -O debug` exit 0. `/tmp/ui-style-debug.log`. |
| Save layout / adjacent options | `rtk make -j16 -O check TESTS="UI style"`: 1 passed / 1 total. Checks low 12 bits, Pokedex offset 0x18, both styles and unsupported value fallback. |
| Option layout | Native 240x160 inspected; all eight rows visible. `evidence/option-default.png`, `option-xy.png`. |
| Input and accept | UP wraps to CANCEL, then UI STYLE; RIGHT selects XY; B accepts. Reopening shows XY (`option-reopened-xy.png`). |
| Persistence | Normal in-game SAVE performed on disposable debug save. Sector 15, save counter 1, option word at +0x14 = `0x1001`. After `emu:reset()` and CONTINUE, RAM option word remains `0x1001`, and OPTION shows XY (`option-after-reset-xy.png`). |
| Switch back | LEFT selects DEFAULT, B accepts (`option-back-default.png`). Next single battle renders original panels (`single-default.png`). |
| XY single | `single-xy.png`: nickname, level, HP text/bar and transparent panels visible. |
| Both doubles styles | `double-default-bars.png` / `double-xy-bars.png`: four battlers, four distinct statuses, long partner nickname. |
| HP numbers/bar toggle | Real START input changes both player HP displays. `double-default-numbers.png`, `double-xy-numbers.png`, `double-xy-restored.png`; XY HP labels reappear with bars. |

## Method and scope

MCP mGBA tools were not exposed. Used the project direct mGBA Live CLI and the
script-capable mGBA build at 120fps with videoSync=1. Initial normal session
confirmed ROM boot and OPTION input. Main runtime validation used a copied,
matching debug ROM and ELF under `.cache/ui-style/`, isolated from user saves.

Title SELECT quickstart created the disposable game. In-game OPTION and SAVE
used ordinary inputs. After reset/CONTINUE, R+START > Party > Start Debug Battle
entered the existing debug trainer fixture. The singles prove both styles selected
through OPTION. The doubles use `tools/mgba_live/xy_status_double_fixture.lua`;
this fixture injects a second player mon, four statuses and the double flag.
It does not validate battle balance or alter production party data.

For identical double comparisons, a savestate at the Start Debug Battle menu was
reloaded. The XY comparison sets the option nibble to 1 before battle starts;
menu selection and persistence were independently exercised above. This is not
a hot switch during an active battle. At this debug ELF: gSaveBlock2Ptr=
0x03005204, gBattleTypeFlags=0x020000dc, gParties=0x02031c04, and
player party count byte=0x02031bf4 (matching retained fixture).

Safari special counter behavior was source-reviewed, not separately played.
HP damage/EXP arithmetic is unchanged; this turn tests selection, loading and
redraw, not every weather, language, or animation. The preexisting HUD evidence
remains in `xy_status_hud_test_plan.md` and is not counted as a current ROM run.
No claim is made that bag, party, dialogue or other UI layouts have been redesigned.

## Environment notes

The first debug launch used `/tmp` and stalled before the bridge appeared.
The process remained alive. X11 window inspection and mGBA's CoreManager.cpp
identified the `Temporary file loaded` dialog. Keyboard dismissal did not work;
the session was explicitly stopped and its ROM/ELF copied into the worktree
cache. The subsequent launch succeeded. This was a diagnosed modal, not a ROM
crash or a restart based solely on an observation timeout.

Builds retain the inherited RWX linker warning and initial asset conversion
libpng warnings. Long GitHub Actions runs were not waited on. No push or merge
has been performed.

Debug ROM SHA256: `e5b64992a445311cf6850c6afc2fa3747c76e5bcd89945c68e3a18737abac6c4`.

Bag opened and inspected (`evidence/bag.png`); B returned to battle with the
selected XY sheets, colors and four statuses intact (`evidence/xy-after-bag.png`).
Both the incremental battle entry and all-at-once battle reload path were thus
exercised in XY mode.

Cleanup: ui-style-normal, ui-style-debug (modal-blocked launch), and
ui-style-runtime explicitly stopped; final project CLI status --all returned [].

Final normal ROM SHA256: `85b74df9be21c51a434c1c3eb23f036976ed77732e722940859a4df8e40f6f38`.

## Background contrast and outline

Follow-up verified 2026-09-09:

- Final debug and normal builds passed (`/tmp/ui-contrast-outline-debug.log`,
  `/tmp/ui-contrast-all.log`). First compile exposed an include-order dependency
  on battle.h; corrected before successful builds.
- Real emulator screenshots of all 23 implemented backgrounds, with four distinct
  statuses and a 10-character partner name: `evidence/contrast/00-*.png` through
  `22-*.png`, and `all-backgrounds.png`. Native images were visually inspected
  together. This verifies the loaded graphics and selected text colors, not
  natural travel to every map or every battle-entry animation.
- The final outline crosses the healthbox sprite seam. Repeated redraws retain
  a one-pixel outline. A long partner name was changed to A and redrawn:
  `nickname-short.png` shows no previous name or outline left behind.
- DEFAULT was selected in the disposable in-memory options and a fresh single
  battle entered: `default-regression.png` retains the original panels/colors.
- UI-style save regression passed before the outline addition (1/1); the outline
  does not touch the save layout. Final focused battle regression is recorded
  below after completion.

The controlled environment probe is reproducible with
`tools/mgba_live/ui_contrast_fixture.py`. After `make debug`, run it with devkitARM
on PATH. It reads matching ELF symbols, asserts that a small reserved tail of the
32MB ROM is unused, and writes a separate `.cache/ui-contrast/contrast.gba`.
The inserted Thumb probe calls the unmodified production background loader and
four production healthbox redraw functions. It does not change the normal ROM.
Enter the existing full doubles fixture, then use `--capture SESSION`; each
background is selected in memory, drawn, and normal main callback restored.
The existing double fixture addresses were resolved against this ELF (EWRAM
shifted eight bytes because of the new cached choices). All saves used here were
copies of the prior disposable UI-style test save.

The contact sheet is a labeled assembly of native screenshots, not generated
artwork. HP/EXP arithmetic, animated weather overlays and all languages were not
retested by this color correction. Palette choice deliberately remains stable
during transient animation backgrounds; the outline provides contrast there.

Final normal ROM SHA256: `ce20a192363133c52fe02c9931d807a046f192ec68b27664179243b51338a62c`.
Probe ROM SHA256: `9b0938ea3f33a2baacbe7f45cfa63f9d3d82120c9f33e60627aa13b1307ed875`.

Final focused regression: `rtk make -j16 -O check TESTS="Paralyze Heal heals a
battler from being paralyzed"` passed 1/1 (`/tmp/ui-contrast-regression.log`).
This is a battle regression check, not a pixel comparison assertion.
Cleanup: ui-contrast and ui-contrast-final stopped; final CLI `status --all` = [].
No push/merge or long GitHub Actions wait was performed.
