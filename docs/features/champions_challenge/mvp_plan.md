# Champions Challenge MVP Plan

## Status

Draft. 実装はまだ行わない。

## MVP Scope

対象:

- 専用受付 NPC / script から開始する single battle 連勝施設。
- 開始時に通常 party / bag を退避する。
- 挑戦中 party は 0 匹から始める。
- ランダム生成または script / NPC 付与で 6 匹をそろえる。
- egg 以外は参加可能。Frontier ban を使う mode は rule id として用意する。
- battle 中は Lv.50。
- EXP は入らない。
- 敗北時は挑戦中 party を削除し、通常 party / bag を復元して終了する。

対象外:

- PC box / remote box。
- 通信 / link / Union Room。
- 既存 Battle Frontier の置き換え。
- challenge party の施設外持ち出し。
- Champions 風 battle menu skin。
- 複数 battle mode / doubles / multi。
- custom ban table editor。

## Generator-First Start

実装順は challenge runtime より generator を先にする。

理由:

- generator は ROM runtime state を壊しにくく、map / NPC / save state が未確定でも進められる。
- 旅順、difficulty、trainer role、pool rule は後で変わっても catalog を差し替えれば応用しやすい。
- 生成物を copy-paste 可能な `.party` fragment にしておけば、build integration 前でも人間が review できる。
- validation / diff / deterministic seed を先に固めると、後で challenge facility に接続する時の事故が減る。

最初の generator MVP:

1. `--rom-repo` でこの repo の constants と `src/data/trainers.party` を読む。
2. `doctor` で repo root、config、trainerproc、constants、output path を確認する。
3. 仮 catalog から stage / trainer role / level band / party style を読む。
4. global set library から trainer blueprint に合う候補を選び、local pool に materialize する。通常 trainer は party size 3-4、pool 6-12 程度。party size 6 trainer は明示した場合だけ pool 20 程度まで許可する。
5. deterministic seed で generated `.party` fragment を出す。
6. species / move / item / ability / trainer constants の存在を検査する。
7. role / archetype / constraint の concept validation を report する。
8. `trainerproc` が読める DSL として validation report / diff report を出す。

入力側 MVP:

- `src/data/trainers.party` は既存 trainer id / header / baseline / source order / diff 対象として読む。
- 人間が主に編集する file は `catalog/journey.json`、`catalog/groups/*.json`、`catalog/blueprints/*.json`、`catalog/sets/*.json`、`catalog/rulesets.*`、`catalog/overrides/*.json`、`weights/*.json`、`notes/species_roles.*`、`sources/*.json`。
- default model は `global_materialized_pool`。global set library の stable slug は tool 側 trace 用で、ROM には local pool として materialize した `Pokemon` block / `Party Size` / `Tags` だけを出す。
- 重要 trainer や concept が崩れやすい trainer は `pinned_materialized_pool` にして、global set から候補を pin / lock して Trainer Party Pool へ出す。完全手書き pool は primary にしない。
- 個別 override の前に group profile / rank band / availability を適用する。個別 override が多い group は report し、group profile へ昇格できないか見る。
- 既存 `.party` block を丸ごと copy-paste する運用は primary にしない。参考 template として使う場合も、preset / override / role note に変換する。
- UI が無い間は `partygen explain`、`partygen render-one`、`partygen validate`、`partygen diff` で入力変更を確認する。
- 起動は CLI を正にする。Linux / WSL は `tools/champions_partygen/partygen.sh`、Windows は `tools\champions_partygen\partygen.cmd` の thin wrapper を用意する。
- `config.example.toml` はコメント付きの基準 config として commit し、個人用の `config.local.toml` は local override として扱う。

この段階では generated file を ROM build に自動 include しない。`src/data/generated/champions_trainers.party` 相当を予約出力にし、設計確定後に build integration へ進む。

追加制約:

- MVP は `src/data/trainers.party` の既存 `TRAINER_*` block を置き換えるだけにする。
- 新規 `TRAINER_*` ID 追加は、trainer defeated flag / `TRAINERS_COUNT` / `MAX_TRAINERS_COUNT` / SaveBlock flag 領域に影響するため別 task にする。
- generated fragment を `trainers.party` から include する方式は、`trainer_rules.mk` の dependency と `clean-generated` policy を決めてから行う。
- Makefile target は CLI / wrapper / config / generated drift check が安定してから追加する。初期の通常 ROM build には partygen を依存させない。
- `src/data/trainers_frlg.party`、`src/data/battle_partners.party`、`test/battle/*.party` は default target にしない。
- source 順固定の trainer には `Party Size` を出さない。候補数と同数でも pool ordering を意図する trainer には `Party Size` を出してよい。
- `Ball` は Pokeball enum、Tera と Dynamax / Gmax は排他として validate する。
- `roles` / `archetypes` / `constraints` は tool 側 vocabulary とし、Trainer Party Pool の `Tags:` へ直接全流ししない。出力直前に Lead / Ace / Support など少数へ map する。
- lint / validation は MVP から strict にする。unknown label、archetype 逆向き採用、required slot 不足、weather / terrain / Trick Room の片欠け、local pool simulation fallback、`minLocalPoolSize` / `maxLocalPoolSize` 範囲外、valid combination count 不足は error に寄せ、file:line と fix hint を出す。

## Phase 1: Challenge State

専用 state を定義する。

| Field | Purpose |
|---|---|
| `active` | 挑戦中か |
| `status` | none / preparing / battling / won / lost / paused |
| `normalPartySaved` | 通常 party を退避済みか |
| `normalBagSaved` | 通常 bag を退避済みか |
| `partyCountRequired` | MVP は 6 |
| `eligibilityRuleId` | egg-only-ban / frontier-ban |
| `bagMode` | MVP は empty challenge bag |
| `battleLevel` | MVP は 50 |
| `disableExp` | TRUE |
| `releaseOnLoss` | TRUE |

既存 `SavePlayerParty` / `LoadPlayerParty` と `SavePlayerBag` / `LoadPlayerBag` は参考にする。ただし、通常 save buffer と challenge runtime buffer の責務が混ざらないよう、専用 helper を挟む。

## Phase 2: Start / Report Flow

受付 script の基本形:

```asm
ChampionsChallenge_Reception::
    lock
    faceplayer
    msgbox ChampionsChallenge_Text_Enter, MSGBOX_YESNO
    goto_if_eq VAR_RESULT, NO, ChampionsChallenge_End
    special ChampionsChallenge_InitRun
    special ChampionsChallenge_SaveAndClearPartyBag
    special SaveGameFrontierLike
    goto ChampionsChallenge_PrepareParty
ChampionsChallenge_End::
    release
    end
```

実装時の方針:

- 「レポートしてから挑戦開始」は Frontier の `challengeStatus` pattern を参考にする。
- ただし Frontier の既存 status に直接乗せるかは慎重に決める。
- power cut / soft reset 復帰時は、通常 party / bag を必ず戻せる状態を先に作る。

## Phase 3: Empty Party / Challenge Bag

開始時:

1. 通常 party を snapshot。
2. 通常 bag を snapshot。
3. `ZeroPlayerPartyMons()` で party を 0 匹にする。
4. challenge bag を空にする。
5. `CalculatePlayerPartyCount()` を呼ぶ。

MVP では held item 移動は不要。作成される challenge Pokemon の持ち物は candidate generator 側が決める。

Battle Pyramid 風に「通常 bag を預けて challenge bag を受け取る」演出は入れてよいが、処理は専用 helper にまとめる。

## Phase 4: Create Six Mons

候補:

| Source | MVP handling |
|---|---|
| Random rental | C の candidate generator で 6 匹を直接作成 |
| NPC create | script / special で species / moves / EV / IV を指定して作成 |
| Training UI | MVP 対象外。後続で `allowTrainingEdit` を有効化 |
| Scout selection | 既存調査 `scout_selection_and_battlefield_status_v15.md` の candidate UI を後続採用 |

battle 開始 gate:

- `CalculatePlayerPartyCount() == 6` でなければ battle 開始不可。
- egg が混ざっていたら不可。
- `eligibilityRuleId == FRONTIER_BAN` の時だけ `isFrontierBanned` を見る。

## Phase 5: Battle Start

battle rule:

| Rule | MVP |
|---|---|
| Level | effective Lv.50 |
| EXP | none |
| Bag in battle | challenge bag mode に従う。最初は no bag 推奨 |
| Held item restore | `docs/features/battle_item_restore_policy/` の方針に従う |
| Whiteout | challenge aftercare 側で処理し、通常 whiteout は避ける |

Lv.50 は actual party data を直接書き換えない。`docs/flows/battle_frontier_level_scaling_flow_v15.md` の通り、battle-only scaling hook を先に設計する。

## Phase 6: Win / Loss Aftercare

勝利:

1. EXP が入っていないことを保証。
2. HP / PP / status / held item を rule に従って回復 / 復元。
3. streak を増やす。
4. 次 battle 前 menu へ戻る。
5. party count が必要数未満なら run end。

敗北:

1. challenge party を削除。
2. challenge bag を破棄するか、報酬変換する。
3. 通常 party / bag を復元。
4. challenge state を clear。
5. lobby / reception へ戻す。

## Phase 7: Test Harness

実装が入ったら、最初に以下を automated / manual で押さえる。

- 開始前 party / bag が終了後に完全復元される。
- 0 匹開始から 6 匹作成まで battle が始まらない。
- egg は不可。
- Frontier-ban mode で banned species が不可。
- Lv.1 / Lv.49 / Lv.50 が battle 中 Lv.50 表示 / stats になる。
- EXP が増えない。
- 敗北後に challenge party が残らない。

## Open Questions

- `SaveGameFrontierLike` 相当を新規に作るか、既存 `SaveGameFrontier` を汎用化するか。
- no bag と empty challenge bag のどちらを MVP default にするか。
- 勝利時の HP / PP 回復は毎戦全回復か、消耗を残すか。
