# XY status HUD validation

Validated on 2026-09-08/09, with a 120fps target and videoSync=1.

## Results

| Check | Result and evidence |
|---|---|
| Aseprite generation | Passed with real Aseprite. Re-running committed-source candidates against installed assets produced identical pixels and palettes for 15 runtime sheets. |
| Design boards | Five 240x160 exports match regeneration; all 960x640 previews are exact nearest-neighbor 4x. Layered `.aseprite` files are included. |
| Indexed conversion | Runtime sheet dimensions and 0..15 index bounds pass; normal/debug builds consumed the generated graphics. |
| Normal build | `make -j16 -O all`, exit 0; log `/tmp/xy-status-all.log`. |
| Debug build | `make -j16 -O debug`, exit 0; log `/tmp/xy-status-debug.log`. |
| Focused existing test | `make -j16 -O check TESTS="Paralyze Heal heals a battler from being paralyzed"`: 1 passed / 1 total; `/tmp/xy-status-check.log`. This is a regression test, not a pixel-layout assertion. |
| Independent review | Found missing HP-prefix restoration after statused doubles numbers-to-bars toggle. Fixed and re-reviewed; source fix approved. No further confirmed palette/OAM defects. |
| Single final ROM | 521/521 HP, nickname, Lv100, command area and empty-at-cap EXP confirmed. `xy_evidence_single.png`. |
| Full 2v2 | Both player panels and both enemy panels visible; four independent statuses and HP labels coexist. `xy_evidence_double.png`; memory reports flags=13, battlers=4, playerParty=2. |
| Numbers/bar toggle | START enters numeric HP mode; second START restores both player bars and HP labels while statused. `xy_evidence_numbers.png` / `xy_evidence_restored.png`. |
| Status cure | Burn cleared through the ROM status update handler; no old badge/opaque rectangle remains and HP label stays visible. `xy_evidence_cured.png`. |
| HP states | Final-ROM healthbar handler exercised green, yellow, red and empty; three non-green captures are included. Numeric HP updating also inspected in single battle. |
| EXP update | Real controller EXP update exercised partial blue strip and level/HP redraw. `xy_evidence_exp.png`; fixture details below. |
| Cleanup | xy-status-runtime, xy-status-final, xy-status-single-final stopped. Final project CLI `status --all` returned `[]`. |
| Actions | Long GitHub Actions runs were not waited on. |

The final tested debug ROM SHA256 is
`74e8ae9713f03615fc69060140a9d64348ad7877df65d66f266dea6bb6c7840c`.
The running `/tmp/xy-status-final.gba` was byte-for-byte equal to the worktree ROM.
Builds retain the existing RWX linker warning; no new build failure occurred.

## Runtime method and limits

MCP mGBA tools were not exposed in this session. The project direct CLI controlled
the script-capable mGBA build. Menu inputs entered the debug trainer battle; the
single baseline used its normal Wobbuffet (Buffie) and enemy party. The Aseprite
boards use real Wailord/Metang/Skarmory assets as layout examples, not as a claim
that the runtime debug party contains Wailord.

The double fixture copied the first player Pokemon into a second party slot,
assigned four statuses and enabled the double flag before battle initialization.
It did not change production trainers or save format. This proves a full 2v2 HUD
layout, not AI or battle-balance behavior. The retained fixture source is
`tools/mgba_live/xy_status_double_fixture.lua`; addresses must be re-resolved from
the matching ELF for other builds. Do not use it with an unrelated ROM or save.

At this ELF, gBattleResources=0x02000218 (bufferA offset 0x10),
gBattlerControllerFuncs=0x0300241c and gBattleControllerExecFlags=0x020000d8.
Healthbar handler Thumb entry 0x0806a5ed exercised decreases 261, 462 and 32767
from the initial 521 HP fixture. These directly exercise production rendering;
they are not naturally inflicted damage. The status handler entry 0x0806a6c1
refreshed the party status after the fixture cleared it. START toggles used real
controller input and exposed the review regression before its fix.

For the EXP fixture, the mon-data controller set total EXP to 479600, then the EXP
controller added 14008 followed by 9000. The debug Pokemon is Wobbuffet: its actual
growth curve causes a level recalculation to 79, followed by a partial EXP strip.
The initial temporary level assignment was 71; it was a test setup value and was
not treated as the final expected level. The resulting Lv79/HP redraw and blue
strip were inspected. This is a rendering/refresh fixture, not a natural EXP
reward test or a claim about the Wailord growth table.

One final-session startup initially timed out waiting for the bridge. The same
PID remained alive, later emitted heartbeat frames and answered screenshots and
inputs; it was not restarted on the observation timeout. stderr contained EGL
driver warnings. After validation it stopped normally and no stale entry remained.

## Remaining scope

No known blocking issue remains in the requested status-area scope. Battle text
and command-area redesign are outside this change. Special Safari/tutorial
scenarios were source-reviewed for retained colors but not separately played;
all-weather and all-language coverage is not claimed. The inherited four-digit
HP formatting remains unchanged; runtime examples here use three digits.

The branch includes its image evidence with the consuming implementation and
must not be merged through a docs/Lua-only master handoff.
