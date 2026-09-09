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
