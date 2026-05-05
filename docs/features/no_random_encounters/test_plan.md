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
