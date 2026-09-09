# Lower battle menu verification

The earlier sections are chronological checkpoints, not current pending work.
The final verification and limits are recorded at the end.

## 2026-09-09 feasibility pass

- Aseprite 1.3.18.2 batch ran `tools/sm_battle_menu.lua` with the existing
  `xy_hud_font.lua` adapter. Native-size action/move/message/party/detail boards
  and editable sources are in `design/`.
- Read PNG IHDR/PLTE bytes: all five are indexed, 16 colors, dimensions divisible
  by 8. `gbagfx` converted all successfully: each 240x48 board is 5760 bytes;
  240x160 party is 19200 bytes; 240x80 details is 9600 bytes.
- Viewed each native-size PNG. Names, type labels, PP, party HP text, message and
  detail text fit the sampled layout. This does not prove all translated text or
  all move names. Font fitting uses actual game glyph widths, with no scaling.
- The first details generation failed because the font adapter cannot map an
  ASCII apostrophe from its charmap parser. Rephrased the sample description;
  generation then succeeded. Runtime text must use the normal game encoding.
- These images are layout studies, not final decorative assets: action glyphs
  (flame/ball/bag/footprints), party Pokemon icons and richer source-style sweeps
  remain to be designed. Do not present the current plain cards as the requested
  finished visual design.
- No ROM source changes yet, no new build/runtime verification, no push.
  Prior HP-HUD tests do not validate the lower menu changes.

## Initial runtime integration checkpoint

`make -j16 -O debug` passed after fixing an EWRAM nonzero initializer and adding
the missing strings declaration include. Build emits the existing RWX linker
warning. Used the matching copied debug ROM/ELF in `.cache/sm-menu/`, not a user
save. The copied save is the prior disposable XY test save.

In script-capable mGBA Live, entered the debug single trainer battle and viewed
the new message frame. `evidence/initial-runtime/` contains inspected action
states, move selection, details, details closed, action return, and the still
original party menu. LEFT/DOWN/RIGHT/UP selects Pokemon/Bag/Run/Fight; A enters
moves; RIGHT/DOWN changes the highlighted card; L opens/closes details; B returns
to the action canvas with intact graphics. A selected move was also executed
and the following battle message appeared in the new frame. These observations
do not validate party redesign, all gimmicks or doubles.

An earlier blind input sequence advanced from move selection into battle and
misnamed its captures. Those exploratory images were moved to
`.cache/sm-menu/exploratory-captures`; they are not claimed as named test evidence.
The accepted captures were taken again from the visibly confirmed action menu.
Both test sessions stopped successfully; `status --all` returned `[]`.

The final Safari/tutorial guard was source-added after this cached-ROM run.
No commit/push yet. Complete the remaining matrix before handoff.

Final normal `make -j16 -O all` passed, and `git diff --check` passed.
Normal ROM SHA256: `94e3cb8cc46447aab3764febd1bc74984a78a4a1f79f0365a59d6548858e1f43`.
Cached debug ROM used for the screenshots:
`54e65c09ca1ccf2721d87f48ed39b6b2862e36763ec609a6eb3f8e89ebb5dc46`.

## Six-card party runtime checkpoint

- Matching cached debug ROM `.cache/sm-menu/party-fixed.gba`, SHA256
  `af41ba21cb1b3c25b4ff04dfc04f7ba64619659296ff05f9fbdb26ea08b34a94`.
  Entered Party > Start Debug Battle with six copied test Pokemon and controlled
  HP/status values. This is a display fixture, not battle-balance validation.
- Inspected `evidence/party/`: six cards, directional selection of slots 1..6,
  sleep/paralysis/poison, fainted pink card, HP green/yellow/red/zero, long names,
  normal action submenu, and the fainted SHIFT rejection. A valid SHIFT returned
  to battle and then the custom action canvas.
- The initial screen was hidden by an opaque tile 0 on BG2. Restoring tile 0 in
  RAM isolated the cause. Source then moved background tiles to 1/2, rebuilt,
  and a fresh matching-ROM session proved the source fix. Diagnostic-only RAM
  screenshots are not counted as evidence of the fixed build.
- The first nickname fixture changed ten header bytes and incorrectly wrote EOS
  into language/nature. The project uses 12-character names, with two encrypted
  characters. Replaced the fixture with proper bit-field/checksum writes and
  inspected `twelve-character-name.png` after menu reload. The apparent extra
  suffix was fixture data, not a renderer terminator bug.
- Both party test sessions stopped; `status --all` returned `[]`.
- Current normal `make -j16 -O all` passed (existing RWX warning), and
  `git diff --check` passed. Normal ROM SHA256:
  `870037b7136238d96ef6cae911ccb971250b9cffee63ee4fcf4e3b16b0e65df0`.
  This normal build also includes partner-preview/no-PP source changes; those
  were not exercised by the cached party ROM above.

## Final verification — 2026-09-09

- `rtk make -j16 -O all`: passed. Normal ROM SHA256:
  `c8ed45902187508473b6abf1d42cfb9eabe137883d327e4534055fa5153fe7c7`.
- `rtk make -j16 -O debug`: passed. Cached final debug ROM SHA256:
  `e10915cefd3f431dcf184a7cf89beedd929c611cad3d715aa735c3e2a5ce1e8e`.
- `rtk make -j16 -O check TESTS='UI style'`: 1 passed / 1 total.
  Existing RWX linker warnings remain. No save layout or battle-rule changes.
- mGBA MCP tools were not exposed; used the script-capable direct Live CLI,
  disposable saves and matching ELF symbols. Long GitHub Actions runs were not
  waited on. Screens in `evidence/review/` are native 240x160 captures.

| Area | Evidence and scope |
| --- | --- |
| Final XY menus | `final-action`, `final-moves`, `final-party`: double battle, asymmetric colored controls, six cards, rounded borders, status/fainted colors and dark footer |
| Move details | `final-details`, `final-details-closed`: L opens the slate panel above the cards and closes without leftover border/tile corruption |
| DEFAULT regression | `default-action`, `default-moves`, `default-party`: final debug build, option word's high four bits cleared before battle; original HUD and menus appear |
| Double targeting | `double-target`: selected Flamethrower entered `HandleInputChooseTarget`; confirmation advanced to battler 2's action and move callbacks in `second-actor-*` |
| Bag and party return | `double-bag`, `bag-return`, `moves-after-bag`, `double-party-return`: real menu input and return to battle controls |
| PP and empty slots | `pp-zero-empty-slot`: red 0/10 PP, empty fourth card without type/PP; `no-pp`: all four PP values suppressed |
| Gimmick display | `max-moves`, `tera-blast`, `z-move`: production drawing functions invoked on controlled fixture data; Max names/Max Guard, Tera Blast type, Z title/type/PP fit |
| Optional move reorder | `reorder-source-target`, `reorder-cancel`, `reorder-confirm`: production cursors and `HandleMoveSwitching` invoked by fixture, then actual B/A input; gold SWAP source, cyan target, restored cards and exchanged names/PP. Current GEN_LATEST setting does not enable SELECT entry |
| Partner preview | `partner-preview`: display fixture feeds the existing two-line partner message into the action prompt; move name remains visible below actor name. This does not exercise partner AI |
| Party eligibility | Earlier `evidence/party/` checkpoint: six directions, fainted SHIFT rejection, valid SHIFT return, green/yellow/red/zero HP and correctly encoded 12-character nickname |
| Messages | Initial single-battle execution and return transitions exercised the framed text printer; text speed and scrolling remain the original engine printer |

The `double-*`, bag return and second-actor captures used the pre-reorder final
review build, SHA256
`e5fc13202102b6f7238a0c466c3c617ecc9071fdb31122e95a4c845cd61fd079`.
The `final-*`, `default-*` and re-taken PP/Max/Tera/Z images use the final debug
build above. Gimmick images are display fixtures, not complete activation or
combat-balance tests.

The display probe inserts a small Thumb call trampoline only in unused ROM tail
at 0x09FF0000; production ROM bytes before that tail are unchanged. Arguments
use scratch EWRAM 0x0203FF00, outside the allocated range ending 0x0203758C.
Symbols come from the matching ELF. `BattleResources.bufferA` is an inline
array: the initial diagnostic accidentally treated it as a pointer and did not
alter move data. Those PP/type captures were replaced after fixing the fixture.
Likewise, the UI option occupies the high nibble of a 16-bit word at +0x14,
not the high nibble of the first byte; DEFAULT evidence was re-taken correctly.

## Remaining manual coverage

Safari/catch tutorial/Pokedude, Arena and other non-normal battle windows use
the original lower menu path, but were not individually replayed. Multi-partner
party layouts and out-of-battle party screens retain the existing layout.
All localized fonts, every long move description, all item-driven party dialogs,
and complete Z/Max/Tera activation sequences are not exhaustively tested.
These are coverage limits; the captured menu flows and default fallback passed.

Final `git diff --check` passed. All Live sessions, including both call-probe
sessions, were stopped; `status --all` returned `[]`.

## Palette, gimmick and independent-option refinement — 2026-09-09

The earlier build hashes and coupled UI STYLE behavior above are historical.
The current controls are independent: UI STYLE selects the upper HUD and
BATTLE MENU selects DEFAULT / SM for battle controls and in-battle party.

- Normal `make -j16 -O all` and debug `make -j16 -O debug` passed (existing
  RWX warnings). Final normal SHA256:
  `fea5e2f0916ef129917a41b64afb3b4b93bb7a5a5394272660e03d2605f75ccc`.
  Final debug SHA256:
  `99bdb6faaf0f00846a5199ac88ac111f375a55265971310c8484059f757272af`.
- `make -j16 -O check TESTS='UI style'`: 2 passed / 2 total. Tests cover the
  original option word, all four HUD/menu combinations, unknown-value fallback,
  menu field at +0x16 and unchanged Pokedex at +0x18.
- Actual OPTION inputs selected BATTLE MENU: SM, scrolled to CANCEL and back,
  and exited with B. After normal in-game SAVE, reset and CONTINUE, RAM contained
  option words `0x1001 0x0001`: XY HUD and SM menu persisted independently.
  `option-sm`, `option-scroll-cancel` and `option-scroll-up` capture the UI.
- `evidence/polish/actions`, `moves`, `party`, `party-submenu`, `bag`,
  `bag-return`, `details`, and `details-close` were inspected. POKEMON opens the
  six-card menu and its SHIFT/SUMMARY/CANCEL submenu; BAG opens the original bag
  and B returns to battle. The footer reads “Choose a POKéMON.”, followed by the
  selected-mon prompt in its submenu. Party backgrounds are brighter green.
- Initial new mint canvases used palette index 0 and appeared black because BG
  color 0 is transparent. Source changed them to index 11, then rebuilt and
  re-took the menu captures. Every ELF symbol address was compared and identical
  between these two debug builds; only drawing constants changed, so matching
  checkpoints could be reused with a full menu redraw. Normal ROMs contain no
  call trampoline; diagnostic probes only modify unused tail bytes.
- `max-available/selected/guard/off` and `tera-available/selected/off` used real
  START and D-pad input with controlled usable-gimmick state. Max names persist
  when moving to Max Guard; Tera Blast switches Normal/Fire correctly. B returns
  to actions. The project currently has Dynamax and Tera activation flags set
  to 0, so this fixture supplies availability without changing project config.
- For Z input tests the fixture supplies a real Z-Power Ring and matching held
  crystal, then calls the ordinary viability assignment. Real START opens the
  damaging or status Z page; A enters targets; D-pad changes targets; B restores
  normal moves/actions. `z-target` and `z-target-right` retain the complete Z
  title/PP/type page, confirming the `sZDisplay` fix; `z-status` shows Z-Celebrate
  and its + All Stats description.
- `activate-4-return` and `activate-5-return` follow actual move confirmations
  and a completed double-battle turn. Live function checks reported active
  gimmick 4/5 and used=1, respectively, with the controller back at action input.
- Z activation initially returned used=0 because the Lv100 debug Pokemon
  disobeyed on the zero-badge disposable save; the existing obedience routine
  clears Z use in that case and can make the Pokemon fall asleep. Clearing only
  sleep did not resolve it. A fixture with the Rain Badge set obeyed, executed
  the Z attack, fainted Metang, and returned to action input after A advanced
  the faint message. `activate-3-return` and `z-activation-message` capture this;
  `HasTrainerUsedGimmick(0, GIMMICK_Z_MOVE)` returned 1. No obedience or badge
  rules were changed in production.
- Real OPTION input switched BATTLE MENU back to DEFAULT while retaining XY:
  `option-original`, `xy-original-actions/moves/party` show the original menus
  under the transparent XY HUD. The reverse combination was selected through
  OPTION too: `option-default-hud-sm`, `default-hud-sm-actions/moves` show the
  original upper health boxes with the SM controls. This verifies independent
  routing beyond the save-field unit test.
- Final Live cleanup returned `status --all = []`. `git diff --check` passed.
  Long GitHub Actions waits were skipped; these results are local builds/tests
  and the inspected script-capable mGBA session.

## Full-card type colors — 2026-09-09

- Normal and debug builds passed with the existing RWX warning. Normal SHA256:
  `ccfa388ac14843365dcebf280edaeba9debf13179f7ca700d509da6a981938c9`.
  Debug SHA256:
  `d1d86ca014b519586af3ea1b924f360f59902c8e6153ede2bd55a583909c4c89`.
- A fresh single battle on that debug build was inspected in mGBA. The
  disposable probe adds its call trampoline only in unused ROM tail space;
  RAM move/PP/type fixtures call the production menu redraw functions.
- Inspected `evidence/type-colors/types-1` through `types-6`: all 18 ordinary
  type hues cover both title and metadata surfaces. The sixth capture matches
  the reference moves Metal Claw, Magnitude, Astonish and Bulldoze.
- `stellar` verifies the selected Stellar Tera Blast type color; `types-5`
  includes an empty move slot; `pp-zero` retains distinct red PP 0/10.
  `reference-cursor`, `reference-details` and `reference-details-close` verify
  D-pad focus and L detail open/close retain the card colors.
- This change only adjusts draw palette constants. Save/routing logic and
  battle mechanics are unchanged; the earlier 2/2 option tests and full-turn
  gimmick checks were not repeated. This run verifies rendering, not all moves
  or all languages.
- The sm-types Live session was stopped after inspection.

## Type-colored SM move details — 2026-09-09

- `make -j16 -O debug` and `make -j16 -O all` passed with the existing RWX
  warning. Debug SHA256:
  `36595d14965af4b09395c377aab449ead07884afc733ec7ca7ebb675e55ba5e9`.
  Normal SHA256:
  `94a8194a53b0d08f05935d9227ebf32b92c090ffeea48d22c1e3c170b97817b6`.
- Focused `make -j16 -O check TESTS='UI style'` passed 2/2. These tests cover
  option/save isolation, not visual layout.
- No mGBA MCP tool was exposed. Used the script-capable direct Live CLI,
  DISPLAY=:0, 120fps target, videoSync=1. Fresh boot and debug single battle
  use the matching debug ELF/ROM; only the disposable probe's unused tail
  contains a function-call trampoline. No old-build state was loaded.
- `evidence/details/natural.png` shows Earthquake opened through L.
  `type-01` through `type-18` cover the 18 ordinary type colors via controlled
  move fields and the production detail renderer; PP fields deliberately
  retain fixture values, so they do not claim each move's natural base PP.
- Real D-pad input while details are open updates Metal Claw, Magnitude,
  Bulldoze and Astonish (`steel`, `ground-variable`, `ground`, `ghost`).
  Verified type-colored surfaces, name, PP, badge, category icon, power and
  accuracy, including variable-power `-`. `closed` and `back-actions` show
  L close and B return without a residual sheet/category icon.
- `status`, `pp-zero`, `max`, `max-guard`, `tera` and `stellar` cover status
  category, pale-red zero PP, Max names/power/descriptions, Max Guard's
  category and dashes, and dynamic Tera Blast type/palette. Gimmick availability
  and Stellar type were provided by RAM fixtures; this is display validation,
  not another complete gimmick battle simulation.
- Rebooted and set BATTLE MENU DEFAULT in disposable save RAM before entering
  a new battle. `default-details` preserves the original panel and category
  position; `default-close` shows return to original actions after L/B.
- Initial one-frame automated taps could be missed while menu DMA settled.
  Repeated the reference traversal with four-frame taps and longer settling
  intervals; only the verified captures are retained as evidence.
- The source descriptions use two lines (conditional branches initially
  appeared to add extra lines in a textual count). The body has room for
  three small-font lines; future longer/translatable descriptions still need
  runtime review. Double/special battle replays and all translations were not
  repeated for this detail-only change. Long GitHub Actions were not awaited.
- Stopped sm-details; final `status --all` returned `[]`. Diff whitespace
  validation passed before commit.

## Soft interior and saved SM DETAILS option — 2026-09-09

- Normal and debug builds passed (existing RWX linker warning). Normal SHA256:
  `4e4a68fe7d128be83d6b88502348498ebed89b642b5a5adb5c26b3a3721d894b`.
  Debug SHA256:
  `18bbc2b962bcd683ca35020c384bde3b96e9c543219d584c9bbe68885e978d76`.
- `make -j16 -O check TESTS='UI style'` passed. Re-ran the cached headless
  test binary to obtain untruncated output: 3/3 passed. The new test covers
  tone default/unknown fallback, independent flags, and +0x90/+0x98 offsets.
- No Live MCP tools were exposed; used direct script-capable mGBA Live at
  DISPLAY=:0, 120fps target and videoSync=1. Booted the final matching debug
  build afresh. The comparison-only cream study and intermediate build were
  discarded; final evidence uses the requested unchanged rim.
- Actual OPTION input scrolled to SM DETAILS, selected RICH, reached CANCEL
  and exited. Normal in-game SAVE displayed success. After reset/CONTINUE,
  the existing option word, battle-menu field and tone byte read
  `0x1001 / 0x0001 / 0x01`; the option screen still selected RICH.
  Actual OPTION input then selected SOFT and exited with B. A fresh debug
  battle opened SOFT details through L (`soft-natural`).
- `evidence/light-details/soft-01..18` and `rich-01..18` exercise all 18 type
  palettes through the production renderer with controlled move/PP RAM.
  PP values belong to the fixture rather than each move's natural base PP.
  Visually inspected SOFT names, numbers, badges, category and description.
  `soft-pp-zero`, `soft-status`, `soft-closed` and `soft-actions` verify red
  zero PP, status dashes, L close and B return without leftover panel pixels.
- `rim-verification.json` records a pixel comparison for every type case:
  240 columns x four upper/lower rim pixels x 18 cases = 17,280 pixels.
  SOFT and RICH match each other and the previous `evidence/details/type-*`
  rim exactly. Body samples differ for all types.
- The implementation changes palette/geometry inside the sheet plus one
  saved preference. Default-menu, Max/Tera/Z and double-battle playthroughs
  from the preceding validation were not repeated; their routing/data
  producers are unchanged. New option independence is covered by the three
  tests and actual save/reload. All languages/descriptions remain unexhausted.
- Stopped the study and final sessions. Final `status --all` returned `[]`;
  `git diff --check` passed. Long GitHub Actions were not awaited.
