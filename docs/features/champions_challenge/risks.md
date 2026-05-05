# Champions Challenge Risks

## Risk Matrix

| Risk | Severity | Affected area | Mitigation |
|---|---|---|---|
| 通常 party が復元されない | High | `gPlayerParty`, save buffer, aftercare | start/end helper を一箇所に集約し、power-cut recovery test を作る。 |
| 通常 bag が破壊される | High | `gSaveBlock1Ptr->bag`, `gLoadedSaveData.bag`, mail, encryption key | `SavePlayerBag` / `LoadPlayerBag` pattern を尊重し、独自 memcpy を散らさない。 |
| Challenge bag が save layout を壊す | High | `struct Bag`, SaveBlock1/2/3 | MVP は runtime-only または既存 spare area を調査してから。bag capacity 変更は避ける。 |
| Lv.50 化で actual EXP / level が壊れる | High | Pokemon data, battle mon setup | actual `MON_DATA_LEVEL` を書き換えず battle-only scaling にする。 |
| EXP 無効化漏れ | Medium | battle script / EXP command / Exp Share | battle rule flag で EXP 分配入口を止め、Exp Share も含めて検証する。 |
| 既存 Frontier を壊す | High | `frontier_util.c`, party menu, Pyramid bag | Frontier globals を直接 reuse しない。新規 mode / helper を分ける。 |
| eligibility が厳しすぎる | Medium | `GetBattleEntryEligibility`, `AppendIfValid` | Champions 専用 rule id を作り、egg-only-ban を default にする。 |
| duplicate species / item rule が混ざる | Medium | `CheckBattleEntriesAndGetMessage` | duplicate validation は separate flag にする。default off。 |
| PC box で rule を迂回される | Medium | storage system, party count | MVP は PC box disabled。remote box は後続設計。 |
| release helper が PC UI に依存する | Medium | `pokemon_storage_system.c` static helpers | 挑戦終了時は UI release ではなく challenge party clear helper を作る。 |
| no-whiteout と通常 whiteout が衝突 | High | `CB2_EndTrainerBattle`, `B_FLAG_NO_WHITEOUT` | challenge aftercare が自分で立てた flag だけ clear する。 |
| held item restore と release が衝突 | Medium | held item restore, challenge bag | loss 時は「復元してから削除」か「削除優先」かを固定する。 |
| soft reset / power cut で 0 匹状態から戻れない | High | save / continue warp / challenge status | 通常 party / bag の snapshot 完了後にだけ active status を立てる。 |
| external 構築 data と ROM 定義がずれる | High | party generator, custom species / moves / abilities / items | 外部 source は trend / role weight のみにし、legality / stats / moves / abilities / items は repo-local data で再検証する。 |
| 強い構築がそのまま楽しい challenge にならない | Medium | generator scoring, intensity, rewards | intensity / variance / cooldown を持たせ、同じ counter や過剰最適化が続かないようにする。 |
| 構築 source の信頼度が混ざる | Medium | Victory Road, PokeDB, articles, videos, wiki notes | Singles / Doubles で source priority を分け、source kind を metadata として保持する。 |
| player log adaptation が露骨すぎる | Medium | mGBA logs, player_profile weights | player weakness は `adaptationWeight` で薄め、低 intensity ではほぼ使わない。 |
| 小サンプルで過学習する | High | player_profile, adaptive scoring | minimum samples、uncertainty penalty、decay、manual profile を入れ、序盤 run は観測中心にする。 |
| 対策順が読まれる | Medium | journey order, generated groups | stage 内 archetype を shuffle し、cooldown / exploration rate / diversity floor を持たせる。 |
| generated party の review ができない | High | generated `.party`, diff, validation | seed / config / source revision を header comment に残し、`partygen diff` と validation report を必須にする。 |
| global set library の label が破綻する | High | `catalog/sets/*.json`, `rule_dictionary.*`, materializer | `roles` / `archetypes` / `constraints` / `tppTags` を分離し、rule dictionary と strict lint で unknown label を error にする。 |
| trainer concept と逆向きの候補が混ざる | High | trainer blueprint, materializer, local TPP | blueprint に required slots / banned archetypes / identity anchors / power budget を持たせ、strict lint の error で止める。 |
| 個別 override が増えすぎる | Medium | `catalog/overrides/*.json`, group profile, review | NPC / Gym / Rival / Elite / rank band の group profile を先に使い、override ratio を report する。 |
| rank / availability で絞りすぎる | Medium | group profile, materializer, lint | hard filter と soft weight を分け、候補ゼロ / valid combination 不足は error にする。 |
| global set id 管理が ROM 側に漏れる | Medium | generated `.party`, constants, review tooling | set id は tool-side trace 用に留め、ROM には materialized local pool だけを出す。 |
| runtime patch 方式で ROM address が壊れる | High | built ROM, data tables, save compatibility | exe で既存 ROM を直接 patch せず、build-time generation + rebuild を基本にする。 |
| 別 project tool と ROM repo の契約が曖昧になる | Medium | partygen, build integration, generated files | `--rom-repo` と file contract を固定し、tool は ROM repo を vendored copy しない。 |
| wrapper script が tool logic を持つ | Medium | `partygen.sh`, `partygen.cmd`, config loading | wrapper は repo root / config 補完だけにし、lint / generate logic は CLI core に置く。 |
| Windows / WSL / Linux で起動方法が割れる | Medium | CLI invocation, Porymap workflow, docs | command surface を共通化し、`.sh` / `.cmd` は同じ subcommand を呼ぶ thin wrapper にする。 |
| config 基準が曖昧になる | Medium | `config.example.toml`, `config.local.toml`, profiles | コメント付き sample を commit し、local override は review / CI で読まない。 |
| raw log が巨大化して扱えなくなる | Medium | mGBA logs, parser, profiles | raw / normalized / profile を分け、generator は raw log を直接読まない。 |
| generator 先行で仕様が散らばる | Medium | docs, catalog, generated output | 未確定案は docs-first parking lot に置き、contract / MVP / risk / test へ分類してから実装する。 |
| copy-paste fragment と build integration がずれる | Medium | generated `.party`, trainerproc, make rules | 予約出力段階でも `trainerproc` validation と diff report を必須にし、自動 include は設計確定後に行う。 |
| trainer ID 追加で flag 領域が溢れる | High | `include/constants/opponents.h`, `TRAINER_FLAGS_START`, SaveBlock flags | MVP は既存 `TRAINER_*` の置き換えに限定する。新規 ID 追加は別 task。 |
| generated fragment の Make dependency が漏れる | Medium | `trainer_rules.mk`, `src/data/generated/*.party` | include integration 前に dependency / clean / CI drift check を追加する。 |
| fixed-order trainer が pool path に入る | Medium | `Party Size`, `trainerproc`, `DoTrainerPartyPool` | source 順固定を意図する trainer には `Party Size` を出さない。候補数同数でも pool ordering を意図する trainer は許可する。 |
| gimmick / ball field が無視または不正になる | Medium | `Ball`, `Tera Type`, `Dynamax Level`, `Gigantamax` | `BALL_*` validation と Tera / Dynamax 排他を入れる。 |
| generator-only と team display 要件が混ざる | High | partygen, opponent preview, UI, RNG | MVP は display 変更なし。preview / team display は別 phase にし、source path / seed / post-pool result の扱いを決めてから実装する。 |

## Specific Notes

### Bag Freeze

Battle Pyramid のような bag 退避は有用だが、通常 bag と challenge bag の owner を混ぜると破壊リスクが高い。

MVP 方針:

- 通常 bag は snapshot して触らない。
- challenge bag は空で開始。
- 終了時に challenge bag は破棄。
- 通常 bag 復元失敗時は run end より復元を優先する。

### Eligibility

既存 Frontier eligibility は「対戦施設の制限」を多く含むため、Champions Challenge default には向かない。

MVP 方針:

- default は egg だけ不可。
- `isFrontierBanned` は optional rule。
- duplicate species / held item は optional clause。
- level cap は eligibility ではなく battle-only Lv.50 rule で扱う。

### Party Count

ユーザー仕様では「6 体そろえたら battle」。そのため MVP の battle start condition は `partyCount == 6` とする。

「6 体以下で終了」は文脈上「6 体未満で次 battle 不可」の可能性が高い。実装前に wording を確定する。

### Party Generator

build-time generation を基本にするなら、ROM runtime の安全性はかなり上がる。一方で、generator 側の validation が重要になる。

MVP で必須にする検査:

- `SPECIES_*`, `MOVE_*`, `ITEM_*`, `ABILITY_*`, `TRAINER_*` が repo 内に存在する。
- explicit ability が species の合法 ability slot と合う。
- move count が 4 以下で、`MOVE_NONE` sentinel と競合しない。
- level / nature / ball / gender / Tera / Dynamax field が `trainerproc` DSL と合う。
- challenge rule の ban list / duplicate species / duplicate item clause を通る。
- generated party が `trainerproc` で header 化できる。

validation の強さは段階化する。copy-paste 後に build が落ちるもの、または現行 `trainerproc` が黙って無視するものは hard error にする。balance / duplicate / source 信頼度のような調整項目は warning / report-only に落とす。

特に注意する `.party` semantics:

- `Party Size` が候補数と同じでも `.poolSize` が出て pool path に入る。これは必ずしも不正ではない。source 順固定の trainer だけ禁止する。
- `Ball` は item ではなく `include/constants/pokeball.h` の Pokeball enum。`ITEM_POKE_BALL` ではなく `BALL_POKE` / human ball 名を出す。
- 現行 `trainerproc` は `Dynamax Level` / `Gigantamax` がある mon では `Tera Type` を C 出力しない。意図しない無視を避けるため、generator は同時指定を hard error にする。
- pool 選出が失敗すると runtime は通常順 party へ fallback する。crash ではないが、生成意図と違うため strict lint では error にする。

source data の扱い:

- Victory Road / Pokemon Battle DataBase / Pokemon HOME / articles / videos / wiki notes は role / trend / rationale の入力。
- species stats、move effect、ability behavior、item behavior は repo-local data が正。
- ポケWiki / 対戦考察 wiki は個別 Pokemon の立ち位置を読む source であり、構築実績 data ではない。

hybrid generator の扱い:

- default は `global_materialized_pool`。global set library から trainer blueprint に合う候補を選び、local Trainer Party Pool を出す。通常 trainer は party size 3-4、pool 6-12 程度。party size 6 trainer は明示した場合だけ pool 20 程度まで許可する。
- boss や concept が強い trainer は `pinned_materialized_pool` を許可する。完全手書きではなく、global set から include / exclude / lock した候補を local pool として出す。
- global set id は ROM に入れない。tool 側の stable slug / hash は report と optional comment 用にし、runtime は `Pokemon` block / `Party Size` / `Tags` だけを見る。
- `roles` は tool 側の細かい役割、`archetypes` は party concept、`constraints` は相性・禁止・必須、`tppTags` は engine 出力用の少数 tag に分ける。
- materializer は採用理由と落選理由を report する。人間が concept inversion を見つけたら、blueprint / override / rule dictionary に戻して直す。

concept validation で拾うべきもの:

- unknown role / archetype / constraint。
- `rain-offense` に sun-only abuser、`trick-room` に高速 attacker だけ、`stall` と `hyper-offense` の同居など、主軸と逆向きの採用。
- weather setter / abuser、terrain setter / abuser、Trick Room setter / slow attacker の片欠け。
- required slot (`lead`, `ace`, `speed-control`, `defensive-glue` など) の不足。
- 候補数は足りているが、条件を通した valid combination が少なすぎる状態。
- identity anchor が local pool に残らない。
- `tppTags` への mapping が多すぎる、または Lead / Ace の一意性が崩れる。

play experience の懸念:

- high intensity で毎回 player counter ばかり出ると理不尽になる。
- low intensity でも弱すぎる party ばかりだと challenge にならない。
- 報酬倍率と難易度が合わないと、最適解だけを選ぶ作業になる。

このため、`intensity` だけでなく `variance`, `adaptationWeight`, `cooldown`, `archetype diversity` を generator config に持たせる。

adaptive scoring の懸念:

- 1-5 run では sample が少なく、偶然の matchup を弱点と誤認しやすい。
- player が対策を変えた直後に generator がまた別 counter を出すと、読まれている感が強すぎる。
- 旅順に対策が並ぶと、逆に攻略順が固定される。

MVP では `minimum_adaptation_runs`, `uncertainty_penalty`, `decay`, `exploration_rate`, `archetype_cooldown` を config に置き、log adaptation は十分な sample が貯まるまで弱くする。

tool boundary の懸念:

- 別 project 側に ROM repo を取り込むと、改造中の定義と同期しにくい。
- ROM repo 側に scraping / UI / log parser を入れすぎると、game build と tool 実験が絡みすぎる。
- raw log を generator が直接読むと、log format 変更で scoring が壊れる。
- `.cmd` / `.sh` wrapper に logic を持たせると、OS ごとに挙動がズレる。
- Makefile に早く接続しすぎると、partygen の環境不備で通常 ROM build まで止まる。
- generated fragment を `trainers.party` に include しても、現行 `trainer_rules.mk` は include 先を dependency として追わない。
- 通常 trainer ID を増やすと defeated trainer flag 領域を消費する。現行 Emerald では `TRAINERS_COUNT_EMERALD = 855`、`MAX_TRAINERS_COUNT_EMERALD = 864` で、追加余地は 9 件のみ。
- `Party Size` 行があると `trainerproc` は `.poolSize` を出すため、fixed-order trainer には出さない。ただし「全候補を pool path で出す」意図なら候補数同数でも許可する。
- v15 の `.party` ball は Pokeball enum であり、Tera と Dynamax / Gmax は同じ mon に同時出力しない。

MVP では、別 project tool が `--rom-repo` でこの repo を読み、generated `.party` fragment と validation report だけを書き出す。log は raw text -> normalized JSONL -> profile の順に変換し、generator は profile だけを読む。

通常 trainer の大量追加は MVP では行わない。多数の generated roster が必要になった場合は、既存 trainer slot の置き換え、virtual trainer、または roster index + 専用 bitfield を検討する。

### Generated Source Integration

現行 build は `src/data/trainers.h` を `src/data/trainers.party` から作る。別名 generated file を build source にするには Makefile 変更が必要。

候補:

| Option | Risk |
|---|---|
| `src/data/trainers.party` を canonical generated source にし、original を退避 | build source が分かりやすいが、upstream 更新時に original / generated / current の merge 手順が必要。 |
| `src/data/generated/trainers.party` があればそちらを使う | 便利だが hidden switch になりやすい。build log に source path を出す必要がある。 |
| 明示変数 `TRAINERS_PARTY_SOURCE` で切替 | source-of-truth が明示的。Makefile rule を追加する必要がある。 |
| copy-paste / `partygen apply --mode replace-blocks` | 初期検証向き。手順漏れを防ぐため diff と validate を必須にする。 |

MVP は copy-paste / apply で十分。自動化するなら hidden fallback より、`TRAINERS_PARTY_SOURCE` を明示する方が事故が少ない。

team display / opponent preview を同時にやる場合は、generated source integration だけでは足りない。UI が raw `TrainerMon` を表示するのか、Trainer Party Pool 選出後の result を表示するのかを決める必要がある。MVP は generator-only とし、表示系は別 phase に分ける。

### SaveBlock Capacity

partygen MVP は SaveBlock を変更しない。log、profile、weights、generated party contents は save に入れない。

現在の目安:

| Area | Current free |
|---|---:|
| SaveBlock1 | 304 bytes |
| SaveBlock2 | 84 bytes |
| SaveBlock3 | 1620 bytes |

SaveBlock1/2 は少し足せるが、無理に詰めると後続 feature と migration が厳しくなる。保存が必要な場合でも、full party ではなく seed / stage / roster index / defeated bitfield のような小さい state に限定する。

## Upstream Migration Watch

upstream 更新時に差分確認する file / symbol:

- `src/load_save.c` `SavePlayerParty`, `LoadPlayerParty`, `SavePlayerBag`, `LoadPlayerBag`
- `include/global.h` `struct Bag`, `struct PyramidBag`, SaveBlock layout
- `src/battle_setup.c` `CB2_EndTrainerBattle`
- `src/battle_util.c` EXP / fainted action / held item restore
- `src/battle_script_commands.c` EXP command
- `src/party_menu.c` `GetBattleEntryEligibility`, `CheckBattleEntriesAndGetMessage`
- `src/frontier_util.c` `CheckPartyIneligibility`, `AppendIfValid`
- `src/battle_pyramid_bag.c` `TryStoreHeldItemsInPyramidBag`
