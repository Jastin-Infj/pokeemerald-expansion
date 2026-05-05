# No Random Encounters

Status: Planned
Code status: No code changes

## Goal

任意の rule / debug / option から、通常移動中のランダム野生エンカウントを止める機能を整理する。

この feature は wild encounter randomizer とは別件として扱う。目的は encounter table の中身を変えることではなく、「移動中に勝手に野生 battle が始まらない」状態を作ること。

## Current Baseline

現行 repo にはすでに `OW_FLAG_NO_ENCOUNTER` がある。

| Area | Existing behavior |
|---|---|
| Config | `include/config/overworld.h` の `OW_FLAG_NO_ENCOUNTER`。default は `0` なので未割り当て。 |
| Step encounter gate | `src/field_control_avatar.c` の `CheckStandardWildEncounter` が `FlagGet(OW_FLAG_NO_ENCOUNTER)` を見て `FALSE` を返す。 |
| Debug menu | `src/debug.c` に toggle がある。ただし `OW_FLAG_NO_ENCOUNTER != 0` のときだけ有効。 |
| Wild tables | `src/data/wild_encounters.json` / `.h` は変更不要。 |

MVP は既存 flag を使う。新しい encounter table や randomizer と混ぜない。

## Recommended Shape

最初の実装方針:

1. `OW_FLAG_NO_ENCOUNTER` に未使用 flag を割り当てる。
2. script / debug menu / facility start からその flag を set / clear する。
3. まずは通常移動中の land / water step encounters を止める。
4. Sweet Scent、Fishing、Rock Smash、static `setwildbattle` / `dowildbattle` を止めるかは別 mode として決める。

`OW_FLAG_NO_ENCOUNTER` は bool config ではなく flag id config。`TRUE` や `1` を入れると「flag 1 を使う」意味になり得るため、実装時は `FLAG_UNUSED_*` などの明示 ID を割り当てる。

## Scope Split

| Mode | Stops | Does not stop | Notes |
|---|---|---|---|
| `step-only` | 草むら / 洞窟 / 水上の通常歩行 encounter | Fishing、Sweet Scent、Rock Smash、scripted wild battle | 既存 `OW_FLAG_NO_ENCOUNTER` に近い。MVP 推奨。 |
| `broad-wild` | step-only + Fishing / Sweet Scent / Rock Smash | static story encounter、trainer battle | 追加 hook が必要。後続候補。 |
| `all-wild` | broad-wild + scripted `dowildbattle` | trainer battle | story / legendary / Snorlax などを壊しやすいので非推奨。 |

MVP は `step-only`。ゲーム進行に必要な static wild battle まで止めると、Regi、Snorlax、scripted tutorial、施設イベントの調査が必要になる。

## Option / Debug Position

option UI は後続でよい。

| Entry | Recommended handling |
|---|---|
| Dev / debug | `OW_FLAG_NO_ENCOUNTER` を割り当て、debug menu toggle を使う。 |
| Script / facility | challenge 開始時に `setflag`、終了時に `clearflag`。 |
| Compile-time default | new game 初期化や facility script で flag を立てる。config だけで常時 ON にするより復旧しやすい。 |
| Option menu | multi-page option / runtime rule state の方針が固まってから接続する。 |

option menu に直接 SaveBlock2 field を足すのは後回し。既存 docs の方針通り、まず event flag / runtime rule state を source of truth にする。

## Related Docs

- `docs/features/no_random_encounters/investigation.md`
- `docs/features/no_random_encounters/mvp_plan.md`
- `docs/features/no_random_encounters/risks.md`
- `docs/features/no_random_encounters/test_plan.md`
- `docs/flows/options_status_flow_v15.md`
- `docs/overview/runtime_rule_options_feasibility_v15.md`
- `docs/flows/dexnav_flow_v15.md`
- `docs/overview/wild_moveset_randomization_v15.md`
