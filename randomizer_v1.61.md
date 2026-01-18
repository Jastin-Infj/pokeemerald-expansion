# ランダマイザー v1.61 仕様（タグ抽選）
将来の見返し用に、タグ抽選の設計を整理する。  
目的は「トレーナー側のスロットにタグ条件を付与し、共有/専用プールの中からタグ一致のみを重み抽選する」こと。

## 背景 / 目的
- 既存の `RANK_*` だけでは戦略性が弱い。  
  例: 「Water タグのみ抽選」「Support タグのみ抽選」など、タグで絞り込んだ構成を作れるようにしたい。
- タグに一致しない候補は除外し、残りの重み合計から抽選する（重み計算は現状のまま）。

## 仕様確定（v1.61）
### タグの定義と参照
- **タグの定義場所**: pool 側（`data/trainer_rank_party/*.party`）に `Tags:` を記述する。  
  例: `Tags: WATER/FIRE`（ここでは **OR/AND は書かない**）
- **タグの参照/フィルタ**: trainers.party のスロット側で `Tags:` を使う。  
  例: `Tags: OR:WATER/FIRE` / `Tags: AND:WATER/FIRE`
- **OR/AND は必須**（省略NG）。タグ区切りは `/` を使う。
- タグ名は pool 定義を正とし、ビルド時にタグ一覧を生成して管理する（上限は `u64` 想定）。

### 記述ルール（スロット内のみ）
- `Tags:` / `LeadTags:` / `AceTags:` は **スロット内にのみ記述**する。  
  トレーナーヘッダーでの記述は **ビルドエラー**。
- **1スロットに指定できるタグ系は1種類のみ**。  
  例: `Tags` と `LeadTags` の併用、`Tags` と `AceTags` の併用は **ビルドエラー**。
- **タグ付きの非ランク/非専用スロットはビルドエラー**。  
  タグは「RANK_*」「専用プール」「MultiRandom」などの抽選対象スロットにのみ使う。

### 抽選ロジック
- 入力: rank pool の候補リスト + スロットのタグ条件  
- **タグ一致の候補のみ残す**  
- **残った候補の重み合計を再計算** し、その合計から重み抽選  
- **タグ一致が0件の場合はビルドエラーを優先**。  
  例外的にランタイムへ残った場合は **そのスロットはバニラ固定**。

### LEAD/ACE の扱い
- `Lead` / `Ace` は **予約タグ**として扱う。  
  `Tags:` に `Lead/Ace` を含めるのは **ビルドエラー**。
- pool 側（`data/trainer_rank_party/*.party`）は `Tags: Lead` / `Tags: Ace` を **定義として許可**。  
  これらは「Lead/Ace 候補にしたいポケモン」印として使う。
- trainers.party 側は **`LeadTags:` / `AceTags:` 専用で参照**する（OR/AND 必須）。  
  例: `LeadTags: OR:Lead` / `AceTags: AND:Boss/Fire`
- **順序は自動調整**（Lead は先頭、Ace は最後へ回す）。  
  これにより `LeadTags` / `AceTags` はフィルタ用途に限定できる。

## 記述例
`src/data/trainers.party`
```plain
RANK_C
Tags: OR:WATER/FIRE

RANK_C
LeadTags: OR:Lead

RANK_C
AceTags: OR:DRAGON/BOSS
```
`data/trainer_rank_party/rank_c.party`
```plain
=== RANK_C_SHARED ===
WeightTotal: 100

Lanturn
Level: 33
Weight: 10
Tags: Lead/Water
```

## 注意点 / バリデーション
- `Tags: OR:...` / `Tags: AND:...` は **trainers.party 側のみ**。pool 側に OR/AND が書かれていたら **ビルドエラー**。
- `Tags` に `Lead/Ace` を含めるのは **ビルドエラー**（`LeadTags/AceTags` でのみ参照）。
- タグ名の未定義（pool 側に存在しない）・タグ一致0件・タグ併用違反は **ビルドエラー**。

## 実装ポイント
### タグ抽選（randomizer.c）
- `ChooseTrainerRankSpecies` で **タグ一致のみ抽出**して重み抽選する。
  - `OR` はいずれか一致、`AND` は全一致。
  - 一致0件は `SPECIES_NONE` を返し、呼び出し側で **バニラ固定**にフォールバック。
- `RandomizeTrainerMon` の引数に `slotTags` を追加し、スロット側のタグ条件でフィルタ。
- `LeadTags/AceTags` は **slot=0 / slot=last の時だけ**フィルタとして適用。

### Lead/Ace の順序制御（battle_main.c）
- `DoTrainerPartyPool` 後の `monIndices` を **Lead は先頭、Ace は最後**に並び替える。
- ランダマイザーON時は **Ace スロットを先に抽選**してから他スロットを抽選。
- デバッグ用に `TRand order` を **スロットごとに出力**。

### AI フラグの自動付与（battle_ai_main.c）
- トレーナーパーティ内に `MON_POOL_TAG_ACE` がある場合、  
  AI フラグ未設定なら **`AI_FLAG_ACE_POKEMON` / `AI_FLAG_DOUBLE_ACE_POKEMON` を自動付与**。
- これにより AI が Ace を最後まで温存する挙動になる。

### ツール/生成（build_trainer_rank_parties.py, trainerproc）
- `LeadTags/AceTags` の **OR/AND 必須**、**Tags と併用不可**をチェック。
- `Tags` に `Lead/Ace` が入っていたら **ビルドエラー**（予約タグ扱い）。
- タグ一覧から `generated/trainer_rank_tags.h` を生成（`u64` ビットマスク）。
- `src/data/trainers.party` は `trainerproc` が `src/data/trainers.h` に変換。

## 変更ファイル一覧（v1.61）
### 手動で編集したファイル
- `Makefile`  
  `trainer_rank_tags.h` の生成を追加（`build_trainer_rank_parties.py` の `--tags-out`）。
- `data/trainer_rank_party/rank_c.party`  
  Lead 候補のタグ付け例を追加。
- `dev_scripts/build_trainer_rank_parties.py`  
  タグ解析/検証、Lead/Ace 予約タグの排除、タグヘッダ生成。
- `include/data.h`  
  `TrainerMon.tags` を `u64` に変更。
- `include/randomizer.h`  
  `RandomizeTrainerMon(..., u64 slotTags)` に更新。
- `include/trainer_pools.h`  
  `u64` タグ定義、`MON_POOL_TAGMODE_AND` を追加、`trainer_rank_tags.h` を取り込み。
- `include/trainer_rank.h`  
  `TrainerRankMon.tags` と `TrainerRankSpecView.leadTags/aceTags` を追加。
- `src/battle_ai_main.c`  
  Ace タグに応じた AI フラグの自動付与。
- `src/battle_main.c`  
  Lead/Ace の順序調整、Ace 先行抽選、`TRand order` ログ追加、`slotTags` 受け渡し。
- `src/data/trainers.party`  
  `LeadTags`/`AceTags` を使ったスロット指定例を追加。
- `src/match_call.c`  
  `RandomizeTrainerMon` に `slotTags` を渡すよう変更。
- `src/randomizer.c`  
  タグ一致抽選、Lead/Ace フィルタ、`slotTags` 対応、キャッシュ初期化条件の更新。
- `src/trainer_pools.c`  
  タグを `u64` で扱うよう修正。
- `src/trainer_rank.c`  
  `leadTags/aceTags` を `TrainerRankSpecView` に反映。
- `tools/trainerproc/main.c`  
  `Tags/LeadTags/AceTags` の検証と `Lead/Ace` 予約タグの禁止。
- `todo ver1.61.md`  
  進捗整理の更新。

### 生成ファイル（自動生成）
- `generated/trainer_rank_parties.h`  
  pool/RankSpec/タグ情報を含む生成ヘッダ。
- `generated/trainer_rank_tags.h`  
  タグの `u64` 定義ヘッダ。
- `generated/randomizer_area_rules.h`  
  既存ルールの生成物（環境依存）。
- `src/data/trainers.h`  
  `trainerproc` により `trainers.party` から生成される中間ヘッダ。

## ビルド/再生成手順
1. `python3 dev_scripts/build_trainer_rank_parties.py --dir data/trainer_rank_party --trainer src/data/trainers.party --skip src/data/randomizer/trainer_skip_list.h --out generated/trainer_rank_parties.h --tags-out generated/trainer_rank_tags.h --mode 100`
2. `CPPFLAGS_EXTRA="-UNDEBUG" CFLAGS_EXTRA="-UNDEBUG" make -j4`
3. `FLAG_RANDOMIZER_DEBUG_LOG` を立てて `TRand` ログで確認
