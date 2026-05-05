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
