# ランダマイザー v1.60 実装メモ
将来（数年後）見返したときに迷わないように、今回入れた仕様と運用ポイントをまとめる。

## トレーナーランダム化の優先順位
1. **ブラックリスト**: `src/data/randomizer/trainer_skip_list.h` に載っている TRAINER_ID は常にバニラ固定。
2. **専用プール**: trainers.party で MultiRandom/専用プールが有効なトレーナーは、専用 .party（`TRAINER_*` ラベル）から slot < poolCount の範囲で抽選。
3. **共有ランクプール**: `RANK_S`〜`RANK_E`（ランクセンチネル）をスロットに書いた場合、そのランクの共有プールから抽選。
4. 上記に当てはまらないスロットは元の種・レベルを使用（ランダマイザーOFF時もそのまま）。

## データ記述ルール
- **ランクセンチネル**: trainers.party の species 行を `RANK_S` / `RANK_A` / … / `RANK_E` にすると、そのスロットは共有ランクプールから抽選される。  
  - 抽選したポケモンのレベルはプール定義のレベルで上書きされる（元の Level: は無視される）。
  - ランダマイザーOFF時はセンチネルがそのまま残るので、基本ON運用を想定。
- **MultiRandom: Count=N**（N=1〜6）: trainers.party にフィールドとして書く。  
  - 指定数が手持ち定義より多い場合、最後のポケモン（未定義ならダミー: species=None, Lv1）で埋めて partySize も N に拡張。  
  - Count=0 はエラー。ポケモン未定義でも Count>0 は許可（ダミーで埋める前提）。  
  - 専用プールの場合、この Count が「専用プールから引くスロット数」になる。
- **専用プール .party**: `data/trainer_rank_party/` 配下の `TRAINER_*` ラベルの .party で専用プールを定義。WeightTotal=100（または1000）必須。weights はコメントでなく `Weight:` 行で記述。
- **共有ランクプール .party**: `RANK_S_SHARED` などの .party に S〜E のプールを定義。WeightTotal=100（または1000）。
- **Skip リスト**: `src/data/randomizer/trainer_skip_list.h` に追加すると、ランダマイザーONでも固定化される。

## ツール/生成物
- `dev_scripts/build_trainer_rank_parties.py`  
  - `.party` の WeightTotal/weights/NONE をバリデーションして `generated/trainer_rank_parties.h` を生成。  
  - trainers.party から RankSpec（Normal/Rare/AllowDuplicates/MaxSame/useTrainerPool/trainerPoolCount/MultiRandom）を抽出。  
  - species/trainer 定数にないものは警告してスキップ。
- `tools/trainerproc`  
  - `MultiRandom: Count=N` をパースし、不足分を埋めて partySize を拡張。  
  - RANK_* センチネルを `TRAINER_RANK_SENTINEL_*` に変換するため、生成ヘッダ側で解決される。

## ランタイム挙動（randomizer.c）
- ブラックリスト → 専用プール（useTrainerPool + poolCount）→ ランクセンチネル → RankSpec Normal/Rare → それ以外バニラ。
- センチネル/プール経由で選んだポケモンはプール定義のレベルで上書き。重複チェックは RankSpec の AllowDuplicates/MaxSame を尊重（未設定ならデフォルト許可）。
- デバッグログ（FLAG_RANDOMIZER_DEBUG_LOG 有効時）で TRand xxx のステップを出力。

## 既知の運用メモ
- ランダマイザーOFF時は RANK_* がそのまま出るので、基本ON運用を前提に運用。OFFで使う場合はスロットを通常種に戻すこと。
- WeightTotal は 100 または 1000 に固定。NONE は抽選外/加算外として扱う。
- 専用プール/ランクプールの Weight はコメントではなく `Weight:` 行で書く。
- partySize が MultiRandom より小さいケースは自動で拡張するが、定義順は保持されない（専用プールのピック関数がシャッフルするため）。順序固定が必要なら pick/シャッフル側を別途改修すること。

## 実際に触るファイルと記述例
- トレーナー定義: `src/data/trainers.party`
  - ランク抽選スロット（共有プール）:
    ```plain
    RANK_C   # species 行をランクセンチネルにする。Level: は抽選レベルに上書きされる
    Level: 5
    ```
  - 専用プール＋複数抽選（MultiRandom）:
    ```plain
    === TRAINER_HALEY_1 ===
    Name: HALEY
    ...
    MultiRandom: Count=3   # 1〜6。定義数より多くても自動で埋め、partySizeを3に拡張

    Lotad
    Level: 6
    ...
    Shroomish
    Level: 6
    ```
    ※手持ち0でも Count>0 は許可（ダミーで埋まる）。0はエラー。

- 共有/専用プール定義: `data/trainer_rank_party/*.party`
  - 共有ランク例（WeightTotal=100必須）:
    ```plain
    === RANK_C_SHARED ===
    WeightTotal: 100

    Lanturn
    Level: 33
    Weight: 10

    Pelipper
    Level: 21
    Weight: 10
    ...
    ```
  - 専用プール例（TRAINER_* ラベルで専用化）:
    ```plain
    === TRAINER_HALEY_1 ===
    WeightTotal: 100

    Lotad
    Level: 6
    Weight: 34
    Shroomish
    Level: 6
    Weight: 33
    Seedot
    Level: 6
    Weight: 33
    ```

- ビルド生成スクリプト: `dev_scripts/build_trainer_rank_parties.py`  
  - `.party` の WeightTotal/weights/NONE を検証し、`generated/trainer_rank_parties.h` を生成。  
  - trainers.party から MultiRandom、RankSpec を取り込み。エラー時は WeightTotal や Count が規定外の場合。

- trainerproc: `tools/trainerproc/main.c`  
  - `MultiRandom: Count=N` をパースし、不足分を埋めて `partySize` を拡張。  
  - RANK_* センチネルを `TRAINER_RANK_SENTINEL_*` に変換（生成ヘッダ側で解決）。

- ランタイムコード:  
  - `src/randomizer.c` … 抽選とレベル上書き、デバッグログ、優先順位。  
  - `src/trainer_rank.c` … センチネル判定/変換ヘルパ。  
  - `src/battle_main.c` … ランダマイザーからのレベル上書きを CreateMon 前に適用。

## 触るときの手順（最小）
1. `data/trainer_rank_party/` に共有/専用プールを定義（WeightTotal=100/1000、Weight: を記載）。  
2. `src/data/trainers.party` で RANK_* や MultiRandom: Count=N を設定。  
3. `python3 dev_scripts/build_trainer_rank_parties.py --mode 100`（必要なら 1000）で再生成。  
4. `CPPFLAGS_EXTRA="-UNDEBUG" CFLAGS_EXTRA="-UNDEBUG" make -j4` でビルド。  
5. `FLAG_RANDOMIZER_DEBUG_LOG` を立てて TRand ログで経路確認。*
