# Generated Data Workflow Manual

この文書は、外部 tool で生成した data を repository の source data に反映する時の入口。
party generator、trainer party pool、shop list、wild encounter table など、手書き source と generated output が混ざる feature で使う。

## Principle

generated data は、入力、validation、出力、反映方法、戻し方を分けて管理する。
最終的に C build が通るだけでは足りない。
生成前の入力ミスを lint で止め、失敗時にどこを直すべきかを file:line と fix hint で出せる状態を目指す。

## Recommended Layers

| Layer | Purpose | Example |
|---|---|---|
| Source catalog | 人間が管理する元データ。concept、group、set、rule を書く。 | `catalog/sets/*.json`, `catalog/groups/*.json` |
| Rule dictionary | role / archetype / constraint / tag の controlled vocabulary。 | `rule_dictionary.*` |
| Blueprint | trainer ごとの抽出条件、固定枠、party size、difficulty。 | `blueprints/*.json` |
| Lint / doctor | unknown label、候補不足、組み合わせ不足、`.party` 非対応を止める。 | `partygen lint`, `partygen doctor` |
| Materialized output | build に渡す generated source fragment。 | generated `.party` fragment |
| Integration | `trainer.party` へ直接入れるか、generated file を参照するか。 | feature ごとに決める |

## CLI First Policy

生成 tool は CLI を source of truth にする。
Web / UI は便利だが、必須にしない。

最初に必要なのは次の command 群。

| Command | Purpose |
|---|---|
| `doctor` | tool、config、input path、known constants、build prerequisite を確認する。 |
| `lint` | 入力 catalog / blueprint / rule の構文と意味を検証する。 |
| `build` | generated output を作る。 |
| `diff` | 既存 output との差分を確認する。 |
| `explain` | trainer / set / rule がなぜ採用・除外されたかを表示する。 |

Windows 用 `.cmd` と Linux / WSL 用 `.sh` は薄い wrapper に留める。
Makefile integration は、CLI、wrapper、config、drift check が安定してから検討する。

## Config Files

local path や一時出力は local config に寄せる。

| File | Commit policy | Purpose |
|---|---|---|
| `config.example.toml` | commit する | コメント付きの基準 config。 |
| `config.local.toml` | commit しない | 個人環境の path、temporary output、debug option。 |
| profile config | 必要なら commit する | challenge / gym / NPC など用途別の共有設定。 |

## Validation Policy

入力は select box だけで完全に守ろうとしない。
候補が多すぎる data は、最終的に lint で厳格化する。

error message は、少なくとも次を含める。

| Field | Example |
|---|---|
| Code | `PGN_E_UNKNOWN_ROLE` |
| Location | `catalog/sets/sets_0001_0100.json:42` |
| Message | `Unknown role 'rain_sweeper_plus'.` |
| Fix hint | `Add the role to rule_dictionary or rename it to 'rain_sweeper'.` |

strict error に寄せる項目:

- unknown constant: `TRAINER_*`, `SPECIES_*`, `MOVE_*`, `ITEM_*`, `ABILITY_*`
- unknown role / archetype / constraint / tag
- required slot 不足
- pool size が party size や max pool policy と矛盾する
- valid combination count が少なすぎる
- weather / terrain / Trick Room などの setter / abuser が片欠け
- generated `.party` が trainerproc / build で読めない

warning に留める項目:

- balance が少し弱い
- concept は成立するが coverage が薄い
- pin / override が多く、randomness が低い

## File Size and Searchability

large JSON は 1 file に集約しすぎない。
ただし、細かく分けすぎると検索性が落ちる。

現時点の推奨は、100 set 前後を 1 shard にする方式。

```text
catalog/
  sets/
    sets_0001_0100.json
    sets_0101_0200.json
    index.json
  groups/
    npc_basic.json
    gym_midgame.json
  rule_dictionary.json
```

検索性は CLI で補う。

| Command | Purpose |
|---|---|
| `sets find` | species、role、group、item で検索する。 |
| `sets show` | set ID の詳細を表示する。 |
| `sets grep` | raw text / label を横断検索する。 |

## Integration Options

generated output の反映方法は feature ごとに選ぶ。

| Option | Pros | Cons |
|---|---|---|
| 既存 source file を rename / replace する | build path が単純。既存表示や trainerproc がそのまま動きやすい。 | original data の退避、upstream merge、戻し忘れの管理が必要。 |
| generated file があればそちらを参照する | original data を残しやすい。比較しやすい。 | build path / display / tooling が generated path を理解する必要がある。 |
| copy-paste fragment を出す | MVP が軽い。review しやすい。 | 人手反映でずれやすい。drift check が必要。 |

MVP は copy-paste 可能な generated fragment と strict lint でよい。
feature complete 前に、参照方式、rename 方式、drift check のどれを採用するかを feature docs に固定する。

## Drift Check

generated data を使う feature は、drift check を持つ。

- input catalog と generated output の timestamp / hash / ruleset hash を比較する。
- generated output を手で編集した場合に検出する。
- original source と generated source のどちらが build に入ったかを表示する。
- CI / local command のどちらで止めるかを feature docs に書く。

## Related Docs

- `docs/features/battle_selection/opponent_party_and_randomizer.md`
- `docs/tutorials/how_to_trainer_party_pool.md`
- `docs/tools/investigation_tooling_guide.md`
- `docs/manuals/rebuild_and_test_manual.md`
