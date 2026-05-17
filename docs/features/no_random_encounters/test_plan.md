# No Random Encounters Test Plan

## Manual Tests

| Test | Expected |
|---|---|
| `OW_FLAG_NO_ENCOUNTER` 未割り当てで debug toggle を開く | config 未設定 message が出る。 |
| `OW_FLAG_NO_ENCOUNTER` に real flag を割り当て、debug toggle ON | flag が set される。 |
| grass / cave を 200 steps 程度歩く | random wild battle が始まらない。 |
| Surf 中に 200 steps 程度移動する | random water battle が始まらない。 |
| debug toggle OFF 後に grass / cave を歩く | 通常どおり random wild battle が発生する。 |
| Fishing | MVP では通常どおり wild battle が始まる。 |
| Sweet Scent | MVP では通常どおり wild battle が始まる。 |
| Rock Smash | MVP では通常どおり wild battle 判定が走る。 |
| static `setwildbattle` / `dowildbattle` event | MVP では通常どおり battle が始まる。 |
| Battle Pyramid / Pike wild room で flag ON | 歩行 encounter が止まる。これを許容するか facility rule で確認する。 |

## Regression Points

- Repel と no encounter flag の併用。
- Roamer encounter が no encounter flag ON で始まらないこと。
- Mass outbreak encounter が no encounter flag ON で始まらないこと。
- Flag clear 後に encounter immunity steps が不自然に残らないこと。
- Debug menu の ON/OFF 表示が flag 状態と一致すること。

## Docs-Only Adoption Review: 2026-05-17

Current decision:

- This pass updates docs only. No source / include runtime file remains changed.
- The future runtime branch should be
  `feature/no-random-encounters-step-only-adopt-20260517` or a newer fresh branch
  from current `master`.
- Runtime adoption must re-apply only:
  - `include/config/overworld.h`
  - `include/constants/flags.h`
  - `include/constants/flags_frlg.h`

Required validation when runtime source is applied:

| Check | Required result |
|---|---|
| `rtk mdbook build docs` | Passes with only known baseline warnings. |
| `rtk make -j16 -O all` | Passes for the normal ROM. |
| `rtk make -j16 -O debug` | Passes because the existing debug toggle is the intended validation route. |
| `rtk make -j16 -O check` | Passes; no encounter logic regression should be introduced by flag allocation. |
| mGBA flag OFF | Route 101 or equivalent grass still starts a normal wild encounter. |
| mGBA flag ON | Standard walking encounters do not start after the debug toggle / flag set. |
| mGBA flag OFF restored | Clearing the flag restores normal walking encounters. |

Out-of-scope for this slice and should be recorded as unvalidated unless a later
feature explicitly expands scope:

- Fishing
- Sweet Scent
- Rock Smash
- scripted `setwildbattle` / `dowildbattle`
- option menu / runtime rule UI

## Validation Record: `feature/no-random-encounters`

Date: 2026-05-05.

Branch / commit checked: `feature/no-random-encounters` at `545989b695`.

Build:

| Command | Result | Notes |
|---|---|---|
| `make -j4` | passed | Existing linker warning: `LOAD segment with RWX permissions`. |
| `make debug -j4` | passed | Same existing linker warning. |

mGBA runtime:

| Test | Result | Evidence |
|---|---|---|
| `FLAG_NO_ENCOUNTER=false`, repel var clear, Route 101 grass movement | passed | Wild Wurmple battle started. Screenshot: `/tmp/mgba-noencounter-flag-off-wild-battle.png`. |
| `FLAG_NO_ENCOUNTER=true`, repel var clear, Route 101 grass movement for 2160 frames | passed | Stayed on field; no wild encounter. Screenshot: `/tmp/mgba-noencounter-flag-on-after-2160frames.png`. |

Important note: after the OFF battle returned to field, `gBattleTypeFlags` still held `4`. Do not use `gBattleTypeFlags` alone as the ON-test oracle. Use `gMain.callback2`, screenshot state, map/coords, and flag bytes together.

## Validation Record: `feature/no-random-encounters-step-only`

Date: 2026-05-09.

Branch / base checked: `feature/no-random-encounters-step-only` from `master`
`5591163a09`.

Implementation slice:

- `include/config/overworld.h`
- `include/constants/flags.h`
- `include/constants/flags_frlg.h`

Build / static checks:

| Command | Result | Notes |
|---|---|---|
| `rtk git diff --check` | passed | No whitespace errors. |
| `rtk make -j16 -O debug` | passed | Existing linker warning: `LOAD segment with RWX permissions`. |
| `rtk make -j16 -O all` | passed | Existing linker warning: `LOAD segment with RWX permissions`. |
| `rtk make -j16 -O check` | passed | Existing linker warning: `LOAD segment with RWX permissions`. |

mGBA runtime:

| Test | Result | Evidence |
|---|---|---|
| `FLAG_NO_ENCOUNTER=false`, repel var clear, Route 101 grass movement | passed | Debug menu warp to `MAP_ROUTE101` (`map_group=0`, `map_num=16`), moved into grass at `x=12`, `y=10`, wild Wurmple battle started. Screenshot: `/tmp/mgba-noencounter-fresh-off-wild-battle-20260509.png`. |
| `FLAG_NO_ENCOUNTER=true`, repel var clear, Route 101 grass movement for 2400 macro frames | passed | Lua set `FLAG_NO_ENCOUNTER` bit 5 at SaveBlock1 flags byte, confirmed `no_encounter=true`, `repel=0`; after walking, `macro_hit=false`, `callback2=0x0819133D` (`CB2_Overworld`), `battle_flags=0`, map stayed `0/16`. Screenshot: `/tmp/mgba-noencounter-fresh-on-after-2400frames-20260509.png`. |
| mGBA Live cleanup | passed | `mgba_live_stop` reported `alive_after:false` for both sessions; CLI `status --all` returned `[]`. |

Runtime symbols used for the fresh branch:

| Symbol | Address / offset |
|---|---|
| `gSaveBlock1Ptr` | `0x030051D0` |
| `gMain` | `0x030066CC` |
| `gMain.callback2` | `gMain + 0x04` |
| `SaveBlock1.flags` | `save1 + 0x1270` |
| `FLAG_NO_ENCOUNTER` | `0x8E5`, bit `5` in the target byte |

Remaining manual checks:

- Surf / cave step encounters were not separately walked in the fresh branch
  runtime pass. The implementation uses the existing shared
  `CheckStandardWildEncounter` gate, so Route 101 grass was used as the focused
  runtime oracle.
- Fishing, Sweet Scent, Rock Smash, static `setwildbattle` / `dowildbattle`,
  DexNav, and option UI remain out of MVP scope.

## Docs-Only Master Record: 2026-05-09

Current decision:

- Do not copy the implementation files into `master`.
- Keep the historical validation evidence above as branch evidence.
- Treat `feature/no-random-encounters` commit `545989b695` as the source slice
  to re-apply on a fresh feature / integration branch when runtime work resumes.

Non-invasive checks performed from `master`:

| Check | Result | Notes |
|---|---|---|
| `git branch --all --list *no-random* *encounter*` | passed | Local `feature/no-random-encounters` exists. |
| `git log --all --grep=no-random --grep=NO_ENCOUNTER` | passed | Found `545989b695 config: enable no random encounters flag`. |
| `git diff master..feature/no-random-encounters -- include/config/overworld.h include/constants/flags.h include/constants/flags_frlg.h` | passed | Runtime slice is limited to three files. |
| `rg OW_FLAG_NO_ENCOUNTER include src docs/features` | passed | `master` has the encounter gate and debug toggle, but config remains `0`. |

No ROM build or mGBA runtime run was repeated for this docs-only update because
no source / data / config file changed on this branch. Before opening an
implementation PR, re-apply the three-file slice on a current-master feature
branch and rerun `rtk make -j16 -O all`, `rtk make -j16 -O debug`, and one
focused mGBA runtime check.

## Docs-Only Adoption Record: 2026-05-17

Non-invasive checks performed from current `master` before this docs update:

| Check | Result | Notes |
|---|---|---|
| `rtk git status --short --branch` | passed | Clean `master` before creating the docs/adoption review branch. |
| `rtk git describe --tags --always --dirty` | passed | `expansion/1.15.2-56-gc8b8e57183`. |
| `rtk gh pr list --state open ...` | passed | Open runtime shelves rechecked before the fresh runtime PR was opened; later opened and closed as completed shelf #41 after CI success. |
| `rtk git diff master..feature/no-random-encounters-step-only -- include/config/overworld.h include/constants/flags.h include/constants/flags_frlg.h` | passed | Historical runtime slice remains limited to the expected three files. |
| `rg OW_FLAG_NO_ENCOUNTER ...` | passed | Current `master` still has the existing gate / debug toggle and config remains `0`. |

No mGBA run was performed in this docs-only pass.

## Runtime Validation Record: 2026-05-17 Fresh Branch

Branch / base checked:

- Branch: `feature/no-random-encounters-step-only-runtime-20260517`
- Base: `master` `788191a7cd`; `git describe` = `expansion/1.15.2-65-g788191a7cd`

Implementation slice:

- `include/config/overworld.h`
- `include/constants/flags.h`
- `include/constants/flags_frlg.h`

Build / static checks:

| Command | Result | Notes |
|---|---|---|
| `rtk git diff --check` | passed | No whitespace errors. |
| `rtk make -j16 -O all` | passed | Existing linker warning: `LOAD segment with RWX permissions`. |
| `rtk make -j16 -O debug` | passed | Existing linker warning: `LOAD segment with RWX permissions`. |
| `rtk make -j16 -O check` | passed | Existing linker warning: `LOAD segment with RWX permissions`. |

mGBA runtime:

| Test | Result | Evidence |
|---|---|---|
| `FLAG_NO_ENCOUNTER=false`, repel var clear, Route 101 grass movement | passed | Debug menu warp to `MAP_ROUTE101` (`map_group=0`, `map_num=16`). Macro started at `x=10`, `y=10`; wild Wurmple battle started after 163 macro frames with `hit_cb=0x0808FBB5`. Screenshot: `/tmp/no-random-encounters-20260517/off-wild-battle.png`. |
| `FLAG_NO_ENCOUNTER=true`, repel var clear, Route 101 grass movement for 2400 macro frames | passed | Lua set `FLAG_NO_ENCOUNTER` bit 5 at SaveBlock1 flags byte, confirmed `flag_on=true`, `repel=0`; after walking, `hit=false`, `callback2=0x081A3BE5` (`CB2_Overworld`), map stayed `0/16`, final coords `x=12`, `y=11`. Screenshot: `/tmp/no-random-encounters-20260517/on-after-2400frames.png`. |
| `FLAG_NO_ENCOUNTER=false` restored, repel var clear, Route 101 grass movement | passed | Lua cleared the same flag bit; wild Poochyena battle started after 394 macro frames with `hit_cb=0x0808FBB5`. Screenshot: `/tmp/no-random-encounters-20260517/off-restored-wild-battle.png`. |
| mGBA Live cleanup | passed after retry | CLI `input-clear` succeeded. CLI `stop` returned `alive_after:true` while `pgrep` showed `[mgba-qt] <defunct>`; MCP `mgba_live_stop` then cleared the stale session and final CLI `status --all` returned `[]`. |

Runtime setup:

| Item | Value |
|---|---|
| ROM copy | `/tmp/no-random-encounters-20260517/no-random.gba` |
| Save copy | `/tmp/no-random-encounters-20260517/no-random.sav`, copied from ignored local `pokeemerald.sav` |
| Session | `codex-no-random-20260517` |
| mGBA path | `/home/jastin/.local/bin/mgba-qt` wrapper |
| `gSaveBlock1Ptr` | `0x03005208` |
| `gMain.callback2` | `gMain + 0x04` |
| `SaveBlock1.flags` | `save1 + 0x1270` |
| `FLAG_NO_ENCOUNTER` | `0x8E5`, bit `5` in the target byte |

Important note: `gBattleTypeFlags` retained `0x4` after the OFF battle returned
to field. The ON-test oracle is `callback2`, screenshot / field state, map /
coords, and flag bytes, not `gBattleTypeFlags` alone.

Remaining manual checks remain the same as the 2026-05-09 validation: Surf /
cave step encounters were not separately walked, and Fishing, Sweet Scent, Rock
Smash, static `setwildbattle` / `dowildbattle`, DexNav, and option UI are out of
MVP scope.
