# Wild Moveset Randomization Feasibility v15

調査日: 2026-05-04。source / data / include / tools は読み取りのみ。実装はまだ行わず、`docs/` への調査メモだけを追加する。

## Purpose

野生 Pokemon の初期技を、現行の「その level までに覚える level-up 技の最後 4 つ」から、重み付き random / TM / tutor 候補込みへ拡張できるか確認する。

結論: **可能**。ただし全技へ手作業で重みを付ける方式は保守が重い。MVP は mode 化し、最初は level-up 技だけの重み付き random、次に TM / tutor を低確率で混ぜる段階実装が安全。

## Current Wild Move Flow

確認した主な files:

| File | Role |
|---|---|
| [src/wild_encounter.c](../../src/wild_encounter.c) | `CreateWildMon()` が wild encounter 用 Pokemon を作る。 |
| [src/pokemon.c](../../src/pokemon.c) | `GiveMonInitialMoveset()` / `GiveBoxMonInitialMoveset()` が初期技を入れる。 |
| [src/data/pokemon/level_up_learnsets/gen_*.h](../../src/data/pokemon/level_up_learnsets/) | species ごとの level-up learnset。 |
| [src/move_relearner.c](../../src/move_relearner.c) | level-up / TM / tutor 候補生成の既存参考実装。 |
| [src/data/pokemon/teachable_learnsets.h](../../src/data/pokemon/teachable_learnsets.h) | TM / tutor 互換性。 |
| [src/data/tutor_moves.h](../../src/data/tutor_moves.h) | tutor move list。 |
| [src/data/moves_info.h](../../src/data/moves_info.h) | 技の威力、命中、type、category、effect など。 |

`CreateWildMon()` は `CreateMonWithIVs(..., USE_RANDOM_IVS)` の後に `GiveMonInitialMoveset(&gEnemyParty[0])` を呼ぶ。`GiveBoxMonInitialMoveset()` は learnset を順に見て、mon の level 以下の非重複技を入れ、4 枠を超えたら古い技を左へずらして新しい技を末尾に入れる。

つまり現行の野生初期技は、ほぼ **level-up learnset のうち、現在 level 以下で最後に覚える 4 技**になる。level 100 の野生個体で高 level 技が最後に並ぶのはこの仕様による。

## Candidate Move Sources

| Source | Feasibility | Notes |
|---|---|---|
| Level-up moves up to current level | High | 現行 flow の延長で最も安全。低 level 個体が不自然な高火力技を持ちにくい。 |
| Level-up moves above current level | Medium | Champions-style / special mode なら可能。通常野生に入れると捕獲価値と難易度が大きく変わる。 |
| TM/HM compatible moves | Medium | `GetRelearnerTMMoves()` が `NUM_ALL_MACHINES`、`GetTMHMMoveId()`、`CanLearnTeachableMove()` を使うため参考にできる。 |
| Tutor moves | Medium | `GetRelearnerTutorMoves()` が `gTutorMoves` と `CanLearnTeachableMove()` を使う。解禁 flag と bag 所持条件を野生生成に持ち込むかは別途仕様化が必要。 |
| Egg moves | Low / risky | 強力な技が混ざりやすく、species ごとの差が大きい。初回 MVP では除外推奨。 |
| All compatible custom table | High for control, high maintenance | CSV/JSON などから生成すれば強さ定義を外部化できるが、table 管理が必要。 |

## Config Mode Proposal

直書きではなく mode 化する。

```c
#define WILD_MOVESET_MODE_DEFAULT 0
#define WILD_MOVESET_MODE_WEIGHTED_LEVELUP 1
#define WILD_MOVESET_MODE_WEIGHTED_PLUS_TEACHABLE 2
#define WILD_MOVESET_MODE_VAR 3

#define P_WILD_MOVESET_MODE WILD_MOVESET_MODE_DEFAULT
#define P_VAR_WILD_MOVESET_MODE 0
```

| Mode | Behavior |
|---|---|
| `DEFAULT` | 現行の `GiveMonInitialMoveset()`。 |
| `WEIGHTED_LEVELUP` | current level 以下の level-up 技から重み付きで最大 4 技を選ぶ。 |
| `WEIGHTED_PLUS_TEACHABLE` | level-up 技を主候補にし、level band に応じて TM / tutor 候補を一部混ぜる。 |
| `VAR` | event var で mode を切り替える。facility / route / story progress で変えたい場合の逃げ道。 |

野生 6V と同じく、将来の facility / roguelike mode では compile-time default + flag/var override の組み合わせが扱いやすい。

## Weighting Options

| Weighting | Pros | Risks |
|---|---|---|
| Learn level proximity | 実装が軽い。現在 level に近い技ほど出やすい。 | 威力のない補助技や古い強技を正しく評価できない。 |
| Move power / accuracy / priority | `gMovesInfo` を参照して自動化しやすい。 | status 技、weather、hazard、setup、recovery の価値が見えない。 |
| Source weight by level band | Lv1-30 は level-up 中心、Lv31-60 で TM 少量、Lv61+ で TM/tutor 増加、のように調整しやすい。 | species ごとの個性は弱い。 |
| Per-move generated weight table | 「強い技」の定義を最も制御しやすい。外部 service と相性が良い。 | table 作成と更新が重い。技追加時のメンテが必要。 |
| Tag-based move roles | `hazard`, `setup`, `stab`, `coverage`, `recovery` などを混ぜられる。 | tag 設計が必要。AI や trainer party generator と共通化しないと管理が増える。 |

推奨は **level band + source weight + optional generated move weight table**。全技に最初から手作業重みを付けるより、まずは source と level band で大枠を作り、危険な技だけ blacklist / penalty table で調整する。

## Example Selection Policy

MVP の考え方:

| Level band | Candidate mix |
|---|---|
| 1-20 | level-up の current level 以下のみ。 |
| 21-40 | level-up 中心。低確率で低 weight の TM 候補を 1 枠まで許可。 |
| 41-70 | level-up + TM。species が tutor compatible なら tutor も低確率。 |
| 71-100 | level-up + TM/tutor。高 weight 技が出やすいが、同系統の技だけで 4 枠を埋めない。 |

技選出時は、同じ技を重複させない。PP は現行と同じく `GetMovePP()` で初期化する。4 枠未満しか候補がない species では現行 default へ fallback する。

## Runtime vs External Generation

| Approach | Pros | Risks |
|---|---|---|
| Runtime wild moveset randomization | 捕獲ごとに差が出る。facility / flag / var と連動しやすい。 | RNG、性能、デバッグ再現性、balance test が重くなる。 |
| Build-time generated table | ROM 内の候補 table を安定化できる。レビューしやすい。 | ルールを変えるたびに生成物更新が必要。 |
| External service output | Pokemon Champions 風の rule tuning や randomizer と相性が良い。 | engine 側の入力 format を固定する必要がある。 |

野生 encounter は生成回数が多いため、runtime で毎回重い candidate scan を行うより、species ごとの候補 list / weight を生成 table 化する方が最終的には安定する。

## Risks

- 「強い技」の定義を power だけにすると、status / hazard / recovery / setup 技が弱く評価される。
- TM / tutor を混ぜると、捕獲した野生個体が通常育成より強くなり、move relearner / TM shop / tutor 解禁の価値が変わる。
- Double battle で全体技や partner damage 技が増えると難易度が跳ねる。
- Legendary / static / gift / roamer を対象に含めるかで balance が変わる。
- Reproducible debug が必要なら seed と mode を記録できる設計にする。
- `MAX_RELEARNER_MOVES` のような UI list 上限とは別に、野生生成用の候補上限を決める必要がある。

## Open Questions

- 通常野生、facility 野生、DexNav、Safari、static encounter を同じ mode にするか。
- TM / tutor は「その species が覚えられる」だけで許可するか、player の解禁状況も見るか。
- Per-move weight table を手で書くか、外部 generator で作るか。
- 捕獲後もその技を保持する仕様でよいか。
- Level 100 wild でも完全 random にするか、高 level 技を優先する現行感を残すか。
