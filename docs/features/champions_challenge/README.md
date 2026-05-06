# Champions Challenge Facility

Status: Generator MVP in progress
Code status: `tools/champions_partygen` implemented; challenge runtime not implemented

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
- `docs/features/champions_challenge/partygen_impact_and_next_steps.md`
- `docs/features/champions_challenge/partygen_validation_report.md`
- `docs/features/champions_challenge/partygen_lint_spec.md`
- `docs/features/champions_challenge/partygen_player_style_logging.md`
- `docs/manuals/trainer_partygen_manual.md`
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

## Runtime Guard Contract

現行 branch の partygen は generator / data replacement だけで、challenge
runtime はまだ存在しない。したがって C 側は `partygen_owned` tag を直接
見ることができない。

後続 runtime 実装では、他 feature が長い trainer ID 条件を持たなくて済むよう、
最小の explicit helper を用意する。

```c
bool32 ChampionsChallenge_IsActive(void);
bool32 ChampionsChallenge_UsesTrainerBattleAftercare(void);
bool32 ChampionsChallenge_UsesHeldItemRestore(void);
bool32 ChampionsChallenge_UsesTrainerBattleSelection(void);
```

runtime 未実装または inactive の間、これらは false 扱いにする。
通常 Elite Four / Wallace を partygen 管理 trainer として置き換えていても、
challenge facility 中でなければ通常 trainer battle として扱う。

他 feature の guard は以下の方針にする。

| Feature | Inactive | Active |
|---|---|---|
| Trainer Battle Aftercare | 通常 trainer policy。default off。 | challenge win/loss aftercare に一本化。通常 aftercare は bypass。 |
| Battle Item Restore | 通常 restore policy。default off または既存 Gen9 non-berry。 | challenge item policy を優先。loss 時 restore/delete 順を challenge 側で固定。 |
| Battle Selection | 通常 trainer selection。default off。 | challenge pre-battle menu / roster policy を優先。通常 selection は bypass。 |

この contract により、`CB2_EndTrainerBattle` や `TryRestoreHeldItems` に
Champions trainer ID の固定リストを増やさずに済む。

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
| Journey order | MVP は `.party` の現行 block 出現順を `sourceOrder` として使う。必要な trainer だけ `catalog/journey.json` の `order` で上書きする。 |
| MVP inputs | `catalog/journey.json`, `catalog/groups/*.json`, `catalog/blueprints/*.json`, `catalog/sets/*.json`, `catalog/rulesets.*`, `catalog/overrides/*.json`, `weights/*.json`, `notes/species_roles.*`, `sources/*.json`。 |
| Script usage | `data/maps/*/scripts.inc` / `data/scripts/*.inc` の `trainerbattle_* TRAINER_XXXX` は補助情報として scan する。 |
| MVP outputs | `generated/trainer_index.csv`, `generated/script_usage.csv`, `catalog/journey.json` skeleton / update、`champions_trainers.party`。 |
| Stack | Rust core + Rust CLI。config は TOML、catalog / report は JSON / CSV / Markdown。 |
| Invocation | CLI を正にし、Linux / WSL 用 `partygen.sh` と Windows cmd 用 `partygen.cmd` は薄い wrapper にする。 |
| UI / exe | Web UI と exe は初期対象外。必要になったら local browser UI、配布が必要になったら Tauri wrapper を検討する。 |

randomizer 方針は、完全 shuffle ではなく実戦寄りの curated generator とする。基本形は **global set library + trainer blueprint + materialized local Trainer Party Pool**。300 体前後の構築済み set を tool 側に持ち、trainer ごとの blueprint で concept / slot / anchor / ban / power budget を指定し、最終的に local pool を `trainers.party` DSL へ出す。通常 trainer は party size 3-4、pool 6-12 程度を基本にし、明示した party size 6 trainer だけ 20 候補程度から 6 体抽出する形を許可する。

global set id は ROM の管理 ID にしない。tool 側では trace 用の stable slug を持ってよいが、ROM runtime は materialized 後の `Pokemon` block、`Party Size`、`Tags` だけを見る。

speed control、weather / terrain、screens、pivot、setup、defensive glue、Tera type などの構築軸は `roles` / `archetypes` / `constraints` として扱う。これらを直接すべて Trainer Party Pool の `Tags:` に流すと tag 空間が破綻しやすいので、engine に出す `Tags:` は Lead / Ace / Support など少数へ map する。

入力側は、既存 `.party` block の丸ごと copy-paste を primary にしない。`src/data/trainers.party` は既存 trainer id、header、baseline、source order、diff 対象として読む。人間が直接触る主入力は、旅順 catalog、ruleset、個別 override、curated weight、species role note、参考 URL memo とする。UI が無い間は CLI の `explain` / `render-one` / `validate` / `diff` で入力変更が `.party` にどう反映されるか確認する。

個別 override が増えすぎないよう、NPC / Gym / Rival / Elite / High Class / Super Class のような group profile を先に置く。rank band や availability は hard filter と soft weight を分け、絞りすぎによる候補ゼロ / combo 不足は strict lint の error にする。

追加で確認した境界:

- `src/data/trainers.party` は `trainer_rules.mk` で `src/data/trainers.h` に変換される。generated fragment を include する場合、現行 rule は include 先の dependency を追わないため、build integration は Makefile 更新込みで扱う。
- 通常 trainer ID を増やすと `TRAINER_FLAGS_START + trainerId` の defeated flag 領域を消費する。現行 Emerald の `MAX_TRAINERS_COUNT_EMERALD` までの追加余地は 9 件なので、MVP は既存 `TRAINER_*` の置き換えに限定する。
- `src/data/trainers_frlg.party`、`src/data/battle_partners.party`、`test/battle/*.party` は別用途。partygen の default target は `src/data/trainers.party` のみにする。
- `Party Size` 行は pool 有効化として扱う。固定順 trainer には出さず、候補数同数でも pool ordering を意図する trainer には出してよい。
- trainer header (`Name` / `Class` / `Pic` / `Music` / `Items` / `Back Pic` など) は preserve し、MVP では party block と pool keys だけを generator 所有にする。
- partygen lint / validation は MVP から strict にする。unknown role、archetype 逆向き採用、required slot 不足、weather setter / abuser の片欠け、TPP tag mapping の過多は error に寄せ、file:line と fix hint を出す。
- MVP は generator-only で、randomizer 表示 / opponent preview / team display は変えない。これらを同時にやる場合は別 phase とし、preview と battle 本番 party の一致、RNG 二重消費、source path 表示を追加要件にする。

初期 CLI:

```text
tools/champions_partygen/partygen.sh doctor
tools/champions_partygen/partygen.sh scan
tools/champions_partygen/partygen.sh generate --seed 1234 --out /tmp/champions_trainers.party
tools/champions_partygen/partygen.sh validate --input /tmp/champions_trainers.party
tools/champions_partygen/partygen.sh diff --input /tmp/champions_trainers.party --against src/data/trainers.party
tools/champions_partygen/partygen.sh apply --input /tmp/champions_trainers.party --target src/data/trainers.party --out /tmp/trainers.party
```

Windows から使う場合は同じ command surface を `tools\champions_partygen\partygen.cmd` で呼ぶ。wrapper は repo root / config path を補完するだけで、generator logic は CLI core に置く。

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

MVP の対象は Champions Challenge の trainer slot のみとする。通常 route trainer の一括置換は別 mode / 別 feature として後回しにし、まずは既存 `TRAINER_*` ID と既存 trainer block の置き換えで進める。catalog 側では `champions_challenge` / `partygen_owned` のような tag で partygen 管理対象を明示する。

field NPC の削除・置換は partygen の範囲外とする。NPC は `events.inc` / `scripts.inc` / hide flag / defeated flag / story flag / movement script / sprite resource と結びつくため、必要になったら NPC cleanup / resource capacity 用の別 feature docs で扱う。特に Team Aqua / Team Magma 系 trainer は story script 依存を確認してから触る。

Web search、usage ranking、動画要約などの外部 trend data は、Pokemon / item / move / nature / role の weight として取り込む。ただし legality と実データは必ずこの repo を基準にする。

UI を作る場合も、1 匹ずつ button で編集するより、旅順 table、ranking panel、bulk apply、batch replace、validation panel を中心にする。

将来は mGBA / 通常 play log から player profile を作り、苦手構築・得意構築を weight に変換する。Champions Challenge 側では `intensity` を 0.0-10.0 のような連続値として持ち、値が高いほど trend / synergy / player weakness 反映を強め、報酬倍率も上げる。

基本は build-time generation とする。`config.example.toml` を基準にし、必要なら `config.local.toml` や `profiles/*.toml` で intensity / adaptation / source options を変更し、`partygen generate` 後に ROM を rebuild する。runtime で ROM data を直接 patch する方式は address 変動と save compatibility のリスクが高いため避ける。

adaptive difficulty は小サンプルで過学習しやすいため、序盤 run は観測中心にする。`minimum_adaptation_runs`, `exploration_rate`, `archetype_cooldown` を持たせ、同じ対策や順番が露骨に続かないようにする。

tooling は別 project の Rust core + Rust CLI を先に作る。tool はこの ROM repo を `--rom-repo` で読み取り対象 / 出力対象として扱い、ROM 側は generated `.party` fragment を取り込んで rebuild する。UI は必要になったら local browser UI を作り、desktop exe が必要なら Tauri wrapper を検討する。Figma MCP は design reference としては有用だが、MVP の必須依存にはしない。

mGBA log は raw text として保存し、normalized JSONL、`player_profile.json` の順に後段処理する。learning / adaptive scoring は最後段に置き、MVP は generation / validation / diff を優先する。

## Open Questions

- 「6 体以下で終了」は、実装上は「6 体未満で次 battle へ進めない」と解釈するか。
- 敗北時の release は演出付きにするか、挑戦 state の破棄として即時削除にするか。
- 挑戦中に捕獲 / スカウト / NPC 作成で 7 匹目が発生する場合、入れ替え UI を挟むか。
- PC box は完全禁止か、挑戦中専用 box だけ許可するか。
- Champions 風 battle menu / pre-battle menu を、通常 battle menu とは別 skin / mode として作るか。
