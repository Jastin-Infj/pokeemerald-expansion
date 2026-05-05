# Trainer Battle Party Selection Risks

## Risk Summary

| Risk | Severity | Affected Symbols / Files | Notes |
|---|---|---|---|
| 元 party を失う | Very High | `gPlayerParty`, `SavePlayerParty`, `LoadPlayerParty`, `ReducePlayerPartyToSelectedMons` | 一時 party 構築前に完全な backup が必要 |
| battle 後状態が消える | Very High | `gPlayerParty`, `CB2_EndTrainerBattle`, `HandleBattleVariantEndParty` | `LoadPlayerParty` だけでは battle 中の変化を上書きする可能性 |
| callback chain の破損 | High | `gMain.savedCallback`, `CB2_EndTrainerBattle`, party menu callbacks | party menu と battle end が同じ callback field を使う |
| script waitstate softlock | High | `ScriptContext_Stop`, `ScriptContext_Enable`, `CB2_ReturnToFieldContinueScriptPlayMapMusic` | UI 終了時に script を正しく再開できないと停止する |
| double 判定ずれ | High | `TRAINER_BATTLE_PARAM`, `GetTrainerBattleType`, `gBattleTypeFlags` | 選出数 3/4 を間違えると battle party が不正になる |
| existing facilities 破壊 | High | `VAR_FRONTIER_FACILITY`, `gSelectedOrderFromParty`, `frontier_util.c` | choose half は Frontier / cable club でも使用中 |
| 4 匹選出制限 | Medium | `MAX_FRONTIER_PARTY_SIZE`, `gSelectedOrderFromParty` | double battle 4 匹選出に既存配列長が足りるか確認必要 |
| duplicate validation が不適切 | Medium | `Task_ValidateChosenHalfParty`, `CheckBattleEntriesAndGetMessage` | Frontier ルールが通常 trainer battle に混ざる可能性 |
| cancel handling | Medium | `CB2_ReturnFromChooseHalfParty`, `gSpecialVar_Result` | 通常 trainer encounter で cancel をどう扱うか未定 |
| special vars 汚染 | Medium | `gSpecialVar_Result`, `gSpecialVar_0x8004`, `gSpecialVar_0x8005` | trainer battle scripts も `VAR_RESULT` を使う |
| battle outcome 分岐漏れ | High | `gBattleOutcome`, `CB2_EndTrainerBattle` | 勝利以外でも復元が必要 |
| form/evolution/move learn | Medium | battle end / evolution flow | 状態反映 timing が未確認 |
| battle UI 表示ずれ | High | `CreatePartyStatusSummarySprites`, `CreateBattlerHealthboxSprites`, `gPlayerPartyCount` | 選出後 party count / empty slot が party status summary に影響 |
| move reorder 反映漏れ | Medium | `B_MOVE_REARRANGEMENT_IN_BATTLE`, `HandleMoveSwitching`, `GetBattlerMon` | battle 中に move order が変わる場合、元 slot へ戻す必要 |
| opponent preview 不一致 | High | `CreateNPCTrainerPartyFromTrainer`, `DoTrainerPartyPool`, `gEnemyParty` | pool / randomize / override 反映前の party を表示すると本戦とずれる |
| RNG 二重消費 | High | `DoTrainerPartyPool`, `RandomizePoolIndices`, `Random32` | preview 用生成で本戦と違う party になる可能性 |
| option save layout 変更 | Medium | `struct SaveBlock2`, `src/option_menu.c`, `SetDefaultOptions` | runtime UI option を増やす場合に save 互換性が絡む |
| partygen SaveBlock 混入 | High | `src/data/trainers.party`, generated `.party`, `struct SaveBlock*` | generated party contents を SaveBlock に入れると容量と migration が厳しい |
| partygen 出力の無検証貼り付け | Medium | `tools/trainerproc/main.c`, `src/data/trainers.party` | DSL / constants / trainer block 境界のズレで build または trainer data が壊れる |
| partygen 手動調整不能 | Medium | `tools/champions_partygen/catalog/*.json`, generated `.party` | GUI / exe が無い状態で JSON だけを source にすると、どこを触ればよいか分からなくなる |
| global set label の破綻 | High | `catalog/sets/*.json`, `rule_dictionary.*`, materializer | set が増えるほど自由入力 label が増え、選出条件が読めなくなる |
| concept inversion | High | trainer blueprint, materialized local pool | trainer の構築思想と逆向きの set が混ざると、弱いだけでなく意図が分からない party になる |
| set id 管理の肥大化 | Medium | global set library, generated `.party`, review report | global ID を ROM 側の管理対象にすると merge / migration / trainer ID と混線する |
| 個別 override の増殖 | Medium | `catalog/overrides/*.json`, group profile | trainer ごとの例外が増えすぎると、generator ではなく手作業 DB になる |
| group / rank filter の絞りすぎ | Medium | group profile, rank band, materializer | availability や rank 条件を hard にしすぎると候補ゼロ / combo 不足になる |
| `trainerproc` key 名 drift | Medium | `tools/trainerproc/main.c`, `docs/tutorials/how_to_trainer_party_pool.md` | `Pool Pick Functions` など parser が読む key と docs / generator が出す key がズレると build 前に詰まる |
| pool fallback の見落とし | High | `DoTrainerPartyPool`, `PickMonFromPool`, `POOL_SLOT_DISABLED` | pool selection 失敗時に通常順 party へ戻るため、意図しない party が出ても分かりにくい |
| 新規 trainer ID 追加 | High | `include/constants/opponents.h`, `TRAINER_FLAGS_START`, `TRAINERS_COUNT`, `MAX_TRAINERS_COUNT` | 通常 trainer は defeated flag を消費する。現行 Emerald は追加余地 9 件のみ |
| generated fragment dependency 漏れ | Medium | `trainer_rules.mk`, `src/data/trainers.party`, `src/data/generated/*.party` | `.party` include 先は現行 rule では自動 dependency にならない |
| 対象 `.party` file の取り違え | Medium | `trainers.party`, `trainers_frlg.party`, `battle_partners.party`, test `.party` | generator が通常 trainer 以外の source を誤って書くと別 mode / tests に影響する |
| `Party Size` 行による pool 化 | Medium | `tools/trainerproc/main.c`, `src/trainer_pools.c` | `Party Size` が候補数と同じでも `.poolSize` が出て pool / shuffle path に入る。source 順固定なら避けるが、pool ordering 意図なら許可する |
| trainer metadata の意図しない変更 | Medium | `Name`, `Class`, `Pic`, `Music`, `Items`, `Back Pic`, `Starting Status` | party 以外の presentation / battle setup が変わる |
| Ball / Tera / Dynamax 出力ミス | Medium | `include/constants/pokeball.h`, `tools/trainerproc/main.c` | `ITEM_*` ball や Tera + Dynamax 同時指定で意図しない C initializer になる |
| upstream merge conflict | Medium | `src/battle_setup.c`, `src/party_menu.c`, `src/script_pokemon_util.c` | v15.x 更新で変わりやすい領域 |

## Details

### Party Restore

`ReducePlayerPartyToSelectedMons` は `gPlayerParty` を選出順に詰めるが、元 party の保存は行わない。Battle Frontier scripts では `SavePlayerParty` などと組み合わせている。

通常 trainer battle では、battle 後の選出 Pokémon の状態を元 slot へ戻す必要があるため、単純な `SavePlayerParty` -> `ReducePlayerPartyToSelectedMons` -> `LoadPlayerParty` では不十分な可能性が高い。

### Callback Chain

`ChooseHalfPartyForBattle` は `gMain.savedCallback` を party menu 終了 callback に使う。一方で trainer battle 開始時も `BattleSetup_StartTrainerBattle` が `gMain.savedCallback = CB2_EndTrainerBattle` を設定する。

同じ field を複数段階で使うため、選出 UI を battle 前に挟む場合は callback を退避するか、専用の state machine を作る必要がある。

### Existing Choose Half Rules

既存 choose half は facility 種別により:

- 選出数
- level cap
- fainted Pokémon の扱い
- species / held item duplicate validation
- 表示 message

が変わる。

通常 trainer battle 用に使う場合、Frontier ルールが混ざらないように専用 mode を検討する。

### Battle Type

`BATTLE_TYPE_DOUBLE` は `BattleSetup_StartTrainerBattle` 内で設定される。選出 UI はその前に起動する可能性が高いため、`gBattleTypeFlags` だけに依存して選出数を決める設計は危険。

`TRAINER_BATTLE_PARAM.mode` や `GetTrainerBattleType(TRAINER_BATTLE_PARAM.opponentA)` から同等の判定を行う必要がある可能性がある。

### Battle UI

`CreatePartyStatusSummarySprites` は `PARTY_SIZE` 分の ball tray を扱う。選出後に `gPlayerParty` の残り slot を空にする場合、battle 中 UI は「選出していない slot」を empty として表示する可能性がある。

この見え方を変えるには `src/battle_interface.c` の party status summary と、場合によっては healthbox / battle controller 側も触る必要があるため、MVP では既存 UI のまま許容する案が安全。

### Opponent Party Preview

Trainer Party Pools は `DoTrainerPartyPool` と `CreateNPCTrainerPartyFromTrainer` により battle init 中に反映される。battle 前 UI で相手 party を見せる場合、同じ pool / randomize / override 結果を battle 前に得る必要がある。

preview のために party を二重生成すると RNG 消費や `gEnemyParty` 汚染が起きる可能性がある。

### Partygen SaveBlock Boundary

`trainers.party` を外部 generator で生成しても、生成結果は `trainerproc` 経由で ROM data になるだけなら SaveBlock を増やさない。危険なのは、生成済み party、player log、usage weight、adaptive profile のような大きい tool-side data を runtime save に持ち込むこと。

MVP では SaveBlock を変更しない。保存が必要になった場合も、SaveBlock に入れるのは run seed、stage、roster index、defeated bitfield のような小さい state だけにする。full party contents は ROM data または tool output として扱う。

### Partygen Copy-Paste Workflow

最初の導入は `/tmp/champions_trainers.party` のような fragment を生成し、`partygen validate` と diff を見てから trainer block 単位で `src/data/trainers.party` に貼る運用にする。

`TRAINER_*` id を変えず、`=== TRAINER_XXXX ===` block の境界を壊さないことが重要。自動 apply を作る場合も、無条件上書きではなく対象 block replace と diff 表示を必須にする。

### Partygen Manual Tuning

JSON catalog は採用してよいが、GUI / exe が無い初期段階では「JSON を編集して、生成 `.party` を確認する」loop を明文化する必要がある。

MVP では、全 trainer の完全 party を trainer JSON に保存しない。人間が触る JSON は旅順、difficulty band、role、theme、ace、ban list、個別 override のような薄い intent に限定する。完全な species / move / item / tag block は global set library または generator が `.party` として出し、review は `.party` diff で行う。

現時点の default は hybrid 方式にする。tool 側に 300 体前後の global set library を持ち、trainer blueprint で concept / required slot / identity anchor / ban / power budget を指定し、最後に local Trainer Party Pool へ materialize する。通常 trainer は party size 3-4、pool 6-12 程度。party size 6 trainer は明示した場合だけ pool 20 程度まで許可する。runtime は global set library を知らず、既存 TPP の `Pokemon` block / `Party Size` / `Tags` だけを見る。

global set id は tool-side trace 用に留める。`nosepass_early_rock_ace_v1` のような stable slug は report と optional comment には使えるが、ROM constants や trainer id のような永続管理対象にしない。

最低限必要な command:

- `partygen explain --trainer TRAINER_*`
- `partygen render-one --trainer TRAINER_* --out /tmp/name.party`
- `partygen validate --input /tmp/name.party`
- `partygen diff --input /tmp/name.party --against src/data/trainers.party`

この loop が無いまま実装すると、調整担当者は JSON のどの field が trainer party に効いているか追えない。Web / desktop UI は後回しでよいが、CLI で同じ説明・単体生成・検証・差分確認ができることを MVP 条件にする。

### Tag / Concept Drift

global set library を使う場合、tag は必ず層を分ける。

| Layer | Policy |
|---|---|
| `roles` | tool 側の細かい役割。lead、ace、speed-control、weather-setter、weather-abuser、defensive-glue など。 |
| `archetypes` | party / trainer concept。rain-offense、rock-control、trick-room-balance、stall など。 |
| `constraints` | 相性、禁止、必須、stage power、type theme、duplicate clause などの rule。 |
| `tppTags` | `.party` に出す少数 tag。Lead / Ace / Support など、engine が扱いやすいものだけ。 |

`roles` を直接 `Tags:` に全出力すると、Trainer Party Pool の tag 空間が肥大化して、pool rule がどこかで詰まりやすい。materializer は role / archetype / constraint で候補を選び、最後に local pool 内で必要な tag だけを `tppTags` に map する。

validation では unknown label、required slot 不足、weather / terrain / Trick Room の片欠け、排他 archetype の同居、identity anchor の欠落、TPP tag mapping 過多、valid combination count 不足を strict lint の error にする。人間が意図して逆張り採用する場合も、generated output を手で直さず、blueprint / override / rule dictionary に例外を明示してから通す。

### Trainerproc Key Drift

`tools/trainerproc/main.c` が受ける trainer option key と docs / generator の表記は同期する必要がある。例えば pool pick は parser 上は `Pool Pick Functions` であり、別名の `Pool Pick Index` は受け付けない。

generator は human label から `.party` key を組み立てるのではなく、内部 enum から parser-compatible key へ変換する。`partygen validate` は `trainerproc` parser を通すか、同等の key whitelist を持つ。

### Pool Fallback Visibility

`DoTrainerPartyPool` は pool selection が `POOL_SLOT_DISABLED` を返すと `usingPool = FALSE` にして、通常の `0..monsCount-1` 順へ fallback する。これは runtime crash を避けるには良いが、generator の期待から見ると silent failure に近い。

partygen 側では、ruleset / tags / species clause / item clause / prune を簡易 simulation し、指定回数の seed で `partySize` 分を取れるか検査する。特に required tag、Lead / Ace、Weather、Support、`Pool Prune` を使う trainer は validation 必須にする。

### Trainer ID / Flag Capacity

通常 trainer の勝利済み状態は `TRAINER_FLAGS_START + trainerId` に保存される。`include/constants/opponents.h` は Emerald 側で `TRAINERS_COUNT_EMERALD = 855`、`MAX_TRAINERS_COUNT_EMERALD = 864` としており、flag 領域 overflow まで追加余地は 9 trainer しかない。

そのため partygen MVP は既存 `TRAINER_*` block の置き換えに限定する。新規 trainer ID を増やす場合は、`TRAINERS_COUNT`、`MAX_TRAINERS_COUNT`、`include/constants/flags.h`、map script、generated `src/data/trainers.h`、trainer slide table、debug menu の影響を別 task で見る。

多数の generated NPC / roster を扱う場合、通常 trainer ID を人数分増やさない。virtual trainer slot 1 個と roster index、または generated group selection を使い、勝敗履歴は専用 bitfield に逃がす。

### Party Build Integration

`.party` build は `trainer_rules.mk` の `%.h: %.party $(TRAINERPROC)` で、C preprocessor output を `trainerproc` に渡す。`src/data/trainers.party` から `src/data/generated/champions_trainers.party` を include する設計は可能性としてあるが、現行 rule は include 先を dependency として追わない。

自動 include に進む場合は、generated fragment を `AUTO_GEN_TARGETS` / dependency / clean policy / CI check のどれで管理するか決める。MVP では copy-paste または `partygen apply --mode replace-blocks` に留める。

partygen が触る対象も限定する。通常 trainer は `src/data/trainers.party`、FRLG は `src/data/trainers_frlg.party`、partner は `src/data/battle_partners.party`、test は `test/battle/*.party` で役割が違う。MVP は通常 trainer のみを対象にする。

### Trainerproc Field Semantics

`trainerproc` は `Party Size` 行があると `.partySize` と `.poolSize` を出す。`Party Size` が定義 Pokemon 数より小さい時だけではなく、同数でも pool path に入る。

これは必ずしも不正ではない。候補数と同数でも、`DoTrainerPartyPool` / `RandomizePoolIndices` / Lead / Ace / pick function を通して「全員出すが順番や role を pool で決める」用途なら許容する。一方、source に書いた順番のまま出す fixed-order trainer に generator が機械的に `Party Size` を足すと、randomize / pool rule の影響を受ける可能性がある。

trainer header も partygen の対象外にする。`Name`、`Class`、`Pic`、`Gender`、`Music`、`Items`、`Mugshot`、`Starting Status`、`Back Pic` は party の強さ以外の挙動に関わるため、MVP では既存値を保持する。

Pokemon field では v15 の `Ball` は item ではなく Pokeball enum。generator は `include/constants/pokeball.h` と `BALL_*` / human ball 名を正にし、`ITEM_POKE_BALL` のような item constant を出さない。

また `trainerproc` の C 出力では `Dynamax Level` / `Gigantamax` がある場合、`Tera Type` branch は出力されない。generator は同じ Pokemon に Dynamax/Gmax と Tera を同時指定しない。

## Mitigation Ideas

| Risk | Mitigation |
|---|---|
| 元 party 消失 | 専用 EWRAM state に `struct Pokemon originalParty[PARTY_SIZE]` を保存 |
| battle 後状態消失 | 復元前に一時 party の selected mons を元 slot へ copy |
| callback 破損 | `gMain.savedCallback` wrapper を明確にし、active flag を持つ |
| script softlock | `special` + `waitstate` pattern を既存に合わせ、return callback を限定 |
| double 判定ずれ | battle setup の判定 helper を共通化または同じ条件を文書化 |
| facility 破壊 | Frontier / cable / Union Room の scripts では feature を無効化 |
| validation 不一致 | 通常 trainer battle 専用 validation path を用意 |
| battle UI 表示ずれ | MVP では battle 開始後 UI を変更せず、3/4 匹 selected party + empty slot 表示を許容するか仕様化 |
| opponent preview 不一致 | MVP から除外し、後続 phase で non-mutating preview helper を設計 |
| RNG 二重消費 | consistent seed か preview/battle 共通の generated party cache を検討 |
| option save layout | 最初は compile-time config に留め、runtime option は save migration 方針確定後に実施 |
| partygen SaveBlock 混入 | generated party / logs / weights は SaveBlock に入れず、保存するなら small id / seed / bitfield に限定 |
| partygen 出力の無検証貼り付け | validate -> diff -> trainer block 単位 paste -> build の順に固定 |
| partygen 手動調整不能 | thin JSON catalog + `explain` / `render-one` / `validate` / `diff` command を MVP に含める |
| global set label の破綻 | `roles` / `archetypes` / `constraints` / `tppTags` を分離し、`rule_dictionary.*` の controlled vocabulary で unknown label を error にする |
| concept inversion | blueprint に required slots / banned archetypes / identity anchors / power budget を持たせ、逆向き採用は strict lint の error で止める |
| set id 管理の肥大化 | global set id は tool-side trace 用に留め、ROM constants / trainer ID として扱わない |
| `trainerproc` key 名 drift | generator output key を `trainerproc` whitelist に固定し、tutorial sample も同期する |
| pool fallback の見落とし | partygen validation で pool rule を simulation し、fallback しそうな trainer は strict lint の error にする |
| 新規 trainer ID 追加 | MVP は既存 trainer block の置き換えに限定し、ID 追加は SaveBlock / flags / constants task に分離 |
| generated fragment dependency 漏れ | include integration 前に `trainer_rules.mk` dependency、clean policy、CI drift check を追加 |
| 対象 `.party` file の取り違え | `--target emerald-trainers` のように対象を明示し、FRLG / partner / test source は default で触らない |
| `Party Size` 行による pool 化 | fixed-order trainer には `Party Size` を出さない。候補数同数でも pool ordering を意図する trainer は許可 |
| trainer metadata の意図しない変更 | partygen の ownership を Pokemon block + pool keys に限定し、header は preserve |
| Ball / Tera / Dynamax 出力ミス | `BALL_*` validation と、Tera / Dynamax の排他 validation を追加 |
| upstream conflict | 差し込み file を最小化し、`docs/upgrades` の checklist で毎回確認 |

## Open Questions

- `MAX_FRONTIER_PARTY_SIZE` が double 4 匹選出に十分か未確認。
- battle 後 evolution / move learn のタイミングと復元タイミングの相互作用は未確認。
- whiteout 時の復元 ordering は未確認。
- cancel を禁止する場合、既存 party menu の B button behavior をどう変えるべきか未決定。
- battle 中 party status summary の 6 slot 表示を仕様として受け入れるか未決定。
- 相手 party preview で trainer pool の randomize を正確に再現する方法は未決定。
- Champions / Rogue runtime state を保存する場合、`RogueSave` / `ChampionsSave` をどの SaveBlock に置くか未決定。
