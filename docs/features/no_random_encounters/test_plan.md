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
