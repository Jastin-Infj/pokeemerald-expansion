# Champions Challenge Facility

Status: Planned
Code status: No code changes

## Goal

Pokemon Champions / Battle Factory / Battle Pyramid 風の、専用ルールで連勝していく挑戦施設の仕様をまとめる。

想定する基本 flow:

1. 挑戦受付でレポートし、通常の手持ちとバッグを一時退避する。
2. 挑戦中の手持ちは最初 0 匹から始める。
3. ランダム配布、または NPC / 編集 UI から 6 匹を作る。
4. 6 匹そろったら Lv.50 ルールで battle を開始する。
5. 戦闘では EXP を得ない。
6. 勝利したら次の battle / 報酬 / 編成へ進む。
7. 敗北したら release / run-end policy により挑戦中の Pokemon を失い、通常の手持ちとバッグを復元して終了する。

## Primary Docs

- `docs/features/champions_challenge/investigation.md`
- `docs/features/champions_challenge/mvp_plan.md`
- `docs/features/champions_challenge/risks.md`
- `docs/features/champions_challenge/test_plan.md`
- `docs/features/battle_selection/opponent_party_and_randomizer.md`
- `docs/overview/scout_selection_and_battlefield_status_v15.md`
- `docs/overview/roguelike_party_policy_impact_v15.md`
- `docs/features/trainer_battle_aftercare/`
- `docs/features/battle_item_restore_policy/`
- `docs/flows/battle_frontier_level_scaling_flow_v15.md`
- `docs/flows/choose_half_party_flow_v15.md`

## Design Position

この feature は、既存 Battle Frontier を直接改造するより、専用 challenge state を持つ新規 facility として作る方が安全。

既存から流用したいもの:

- Frontier の「受付で保存して挑戦開始」「途中復帰 / 敗北 / 勝利状態」の考え方。
- `SavePlayerParty` / `LoadPlayerParty` の party snapshot pattern。
- `SavePlayerBag` / `LoadPlayerBag` と Battle Pyramid bag の「通常バッグを退避して、一時バッグで遊ぶ」考え方。
- Frontier / party menu の参加資格 validation。
- Trainer battle aftercare の battle end hook。

ただし、実装時は既存 Frontier global をそのまま使い回さず、Champions Challenge 専用の rule id / runtime flag / save state を作る。

## Current Priority

先行優先は **party generator** とする。理由は、challenge state / map / NPC / battle aftercare より ROM runtime への影響が小さく、仕様が後で変わっても catalog / weight / validation の形を応用しやすいため。

最初は game build へ深く接続しない。generator は copy-paste 可能な generated `.party` fragment、validation report、diff report を出すところまでを第一到達点にする。設計が固まるまでは `src/data/trainers.party` を直接置き換えず、予約出力として `src/data/generated/champions_trainers.party` 相当を作る。

作業方針:

- 未確定仕様、思いつき、リスク、後続案は一旦 docs に入れる。
- 確定したものだけ MVP plan / contract / implementation task に移す。
- generator core は先に作れるが、map 配置、trainer ID、旅順、battle rule は docs 側で仮 catalog として扱う。
- ROM 側の runtime 実装は、generated output と challenge rule が review できる状態になってから接続する。

### Party Generator Baseline Summary

まだ実装はしないが、generator の初期方針は以下で固定する。

| Topic | Decision |
|---|---|
| First artifact | copy-paste 可能な `champions_trainers.party` を出す。 |
| Build integration | 最初は自動 include しない。generated file は予約出力として扱う。 |
| Trainer identity | `.party` の `=== TRAINER_XXXX ===` と `include/constants/opponents.h` を対応させる。 |
| Journey order | `.party` の現行定義順は信用しない。人間 review 済みの `journey_catalog.json` を正にする。 |
| Script usage | `data/maps/*/scripts.inc` / `data/scripts/*.inc` の `trainerbattle_* TRAINER_XXXX` は補助情報として scan する。 |
| MVP outputs | `trainer_index.csv`, `script_usage.csv`, `journey_catalog.json`, `champions_trainers.party`。 |
| Stack | Rust core + Rust CLI。config は TOML、catalog / report は JSON / CSV / Markdown。 |
| UI / exe | Web UI と exe は初期対象外。必要になったら local browser UI、配布が必要になったら Tauri wrapper を検討する。 |

randomizer 方針は、完全 shuffle ではなく実戦寄りの curated generator とする。speed control、weather / terrain、screens、pivot、setup、defensive glue、Tera type などの構築軸を見て候補を出し、人間が species / move / item / ability / nature / EV / IV / Tera type を lock / override して微調整できるようにする。

初期 CLI の想定:

```text
partygen scan --rom-repo /path/to/pokeemerald-expansion
partygen plan --catalog catalog/journey_catalog.json
partygen generate --seed 1234 --out champions_trainers.party
partygen validate --input champions_trainers.party
partygen diff --against src/data/trainers.party
```

詳細は `docs/features/battle_selection/opponent_party_and_randomizer.md` の `Trainer ID / Journey Order Feasibility` と `Docs-First Parking Lot` を参照する。

## Default Rule

初期仕様は以下で固定する。

| Rule | Default |
|---|---|
| Entry party | 挑戦開始時は 0 匹 |
| Required party | battle 開始には 6 匹必須 |
| Eligibility | egg 以外は参加可能 |
| Ban list | rule id で切り替え可能。default は Frontier ban を使える mode を用意 |
| Level | battle 中は Lv.50 として扱う |
| EXP | 挑戦中 battle では獲得しない |
| Bag | 通常 bag は退避。挑戦中は空 bag または challenge bag |
| PC box | MVP では使用不可。後で optional remote box を検討 |
| Loss | 挑戦中 party を失い、通常 party / bag を復元して終了 |

## Rule Flags / State Candidates

実装時に必要になる runtime state:

| State | Purpose |
|---|---|
| `active` | Champions Challenge 中か |
| `bagMode` | normal / empty challenge bag / pyramid-style bag |
| `eligibilityRuleId` | egg-only-ban / frontier-ban / custom-ban-list |
| `requiredPartySize` | MVP は 6 |
| `battleLevel` | MVP は 50 |
| `disableExp` | 挑戦中 battle の EXP を止める |
| `releaseOnLoss` | 敗北時に挑戦中 party を削除する |
| `allowPcBox` | PC / remote box を使えるか |
| `allowTrainingEdit` | EV / IV / nature / moveset 編集を許可するか |

## Party Generator Direction

Champions 用 party generator は C 実装に固定しない。MVP は Rust の repo-local CLI tool を第一候補として作り、`src/data/trainers.party` と repo 内 constants / species / move / item / ability data を正として読む。Python は prototype / analysis script として使える。

Web search、usage ranking、動画要約などの外部 trend data は、Pokemon / item / move / nature / role の weight として取り込む。ただし legality と実データは必ずこの repo を基準にする。

UI を作る場合も、1 匹ずつ button で編集するより、旅順 table、ranking panel、bulk apply、batch replace、validation panel を中心にする。

将来は mGBA / 通常 play log から player profile を作り、苦手構築・得意構築を weight に変換する。Champions Challenge 側では `intensity` を 0.0-10.0 のような連続値として持ち、値が高いほど trend / synergy / player weakness 反映を強め、報酬倍率も上げる。

基本は build-time generation とする。`config.toml` で intensity / adaptation / source options を変更し、`partygen generate` 後に ROM を rebuild する。runtime で ROM data を直接 patch する方式は address 変動と save compatibility のリスクが高いため避ける。

adaptive difficulty は小サンプルで過学習しやすいため、序盤 run は観測中心にする。`minimum_adaptation_runs`, `exploration_rate`, `archetype_cooldown` を持たせ、同じ対策や順番が露骨に続かないようにする。

tooling は別 project の Rust core + Rust CLI を先に作る。tool はこの ROM repo を `--rom-repo` で読み取り対象 / 出力対象として扱い、ROM 側は generated `.party` fragment を取り込んで rebuild する。UI は必要になったら local browser UI を作り、desktop exe が必要なら Tauri wrapper を検討する。Figma MCP は design reference としては有用だが、MVP の必須依存にはしない。

mGBA log は raw text として保存し、normalized JSONL、`player_profile.json` の順に後段処理する。learning / adaptive scoring は最後段に置き、MVP は generation / validation / diff を優先する。

## Open Questions

- 「6 体以下で終了」は、実装上は「6 体未満で次 battle へ進めない」と解釈するか。
- 敗北時の release は演出付きにするか、挑戦 state の破棄として即時削除にするか。
- 挑戦中に捕獲 / スカウト / NPC 作成で 7 匹目が発生する場合、入れ替え UI を挟むか。
- PC box は完全禁止か、挑戦中専用 box だけ許可するか。
- Champions 風 battle menu / pre-battle menu を、通常 battle menu とは別 skin / mode として作るか。
