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
| runtime patch 方式で ROM address が壊れる | High | built ROM, data tables, save compatibility | exe で既存 ROM を直接 patch せず、build-time generation + rebuild を基本にする。 |
| 別 project tool と ROM repo の契約が曖昧になる | Medium | partygen, build integration, generated files | `--rom-repo` と file contract を固定し、tool は ROM repo を vendored copy しない。 |
| raw log が巨大化して扱えなくなる | Medium | mGBA logs, parser, profiles | raw / normalized / profile を分け、generator は raw log を直接読まない。 |
| generator 先行で仕様が散らばる | Medium | docs, catalog, generated output | 未確定案は docs-first parking lot に置き、contract / MVP / risk / test へ分類してから実装する。 |
| copy-paste fragment と build integration がずれる | Medium | generated `.party`, trainerproc, make rules | 予約出力段階でも `trainerproc` validation と diff report を必須にし、自動 include は設計確定後に行う。 |

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

source data の扱い:

- Victory Road / Pokemon Battle DataBase / Pokemon HOME / articles / videos / wiki notes は role / trend / rationale の入力。
- species stats、move effect、ability behavior、item behavior は repo-local data が正。
- ポケWiki / 対戦考察 wiki は個別 Pokemon の立ち位置を読む source であり、構築実績 data ではない。

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

MVP では、別 project tool が `--rom-repo` でこの repo を読み、generated `.party` fragment と validation report だけを書き出す。log は raw text -> normalized JSONL -> profile の順に変換し、generator は profile だけを読む。

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
