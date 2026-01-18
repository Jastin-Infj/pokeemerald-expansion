## タグ抽選（案A）調査結果メモ

### 調査した関連ファイル（役割）
- `src/data/trainers.party`: トレーナーのスロット定義。`RANK_*` センチネルや `Tags:` を記述する場所。
- `data/trainer_rank_party/*.party`: 共有/専用プール。`WeightTotal/Weight/Tags` を記述する場所。
- `tools/trainerproc/main.c`: trainers.party パーサ。`Tags:` の書式制限や `MON_POOL_TAG_*` への変換を行う。
- `include/trainer_pools.h`: タグ定義（`POOL_TAG_*` / `MON_POOL_TAG_*`）と上限（`POOL_NUM_TAGS`）。
- `src/trainer_pools.c`: LEAD/ACE タグの特別扱い（先頭/最後の選出ロジック）。
- `include/data.h`: `struct TrainerMon` に `tags` フィールドあり（`u32`）。
- `dev_scripts/build_trainer_rank_parties.py`: rank pool 生成。現状は `Tags:` を無視。
- `generated/trainer_rank_parties.h`: 生成物。`TrainerRankMon` は species/level/weight のみ。
- `include/trainer_rank.h` / `src/trainer_rank.c`: ランクセンチネル変換/判定。
- `src/randomizer.c`: トレーナー抽選（ランク/専用プール）本体。
- `src/battle_main.c`: `RandomizeTrainerMon` 呼び出し元。
- `include/randomizer.h`: ランダマイザーAPI（関数シグネチャ変更の影響先）。

### 現状の制約 / 影響ポイント
- `tools/trainerproc/main.c` の `Tags:` パースは **`:` を許容していない**ため、`Tags: OR:WATER/FIRE` の形式はそのままだと読めない。
- `Tags:` は `MON_POOL_TAG_*` へ変換されるため、**新タグは `include/trainer_pools.h` で定義必須**。
- `TrainerMon` の `tags` はあるが、**RandomizeTrainerMon にタグ情報が渡っていない**（引数は trainerId/slot/totalMons/species のみ）。
- `build_trainer_rank_parties.py` は `Tags:` を読み取らないため、**rank pool 側のタグが抽選に反映されない**。
- `generated/trainer_rank_parties.h` の `TrainerRankMon` に **タグを保持するフィールドが無い**。
- LEAD/ACE は `trainer_pools.c` で **先頭/最後の選出ロジックに使われる**ため、フィルタに使うと副作用の可能性。
- タグ一致0件を **ビルドエラー化**するには、buildスクリプト側で trainers.party の **各スロットタグ × rank pool タグ** を突き合わせる実装が必要。

### 実装に向けた作業候補（案A）
1. **タグモード（OR/AND）パース**
   - `tools/trainerproc/main.c` の `Tags:` パースを拡張し、`Tags: OR:...` / `Tags: AND:...` を許容。
   - モードを保持するため `TrainerMon` に `tagMode` を追加するか、`tags` の上位ビットでエンコードするかを決める。

2. **rank pool 側のタグ取り込み**
   - `dev_scripts/build_trainer_rank_parties.py` で `Tags:` を読み取り、`TrainerRankMon` にタグbitmaskを持たせる。
   - `generated/trainer_rank_parties.h` と `include/trainer_rank.h` の構造体を拡張。

3. **抽選ロジックにタグフィルタを追加**
   - `src/randomizer.c` の `ChooseTrainerRankSpecies` を拡張し、タグ一致候補のみで重み合計→抽選。
   - OR/AND の判定ロジックを追加。
   - タグ一致0件時はランタイムで **バニラ固定**。

4. **ビルド時エラー（タグ一致0件）**
   - `build_trainer_rank_parties.py` で trainers.party の各スロットを解析し、
     rank pool 側に一致タグ候補が存在しない場合はエラー。
   - 既存の RankSpec コメント解析と整合させる必要がある。

5. **タグ定義の追加**
   - `include/trainer_pools.h` の `POOL_NUM_TAGS` と `MON_POOL_TAG_*` を拡張。
   - LEAD/ACE も抽選フィルタに使う前提だが、**順序制御への影響**は注意。

### 仕様確定事項（v1.61）
- 実装は **案A（既存 Tags 再利用）**。
- `Tags: OR:...` / `Tags: AND:...` を採用。
- タグ一致0件は **ビルドエラーを優先**（ランタイムはバニラ固定）。
- **タグ付きの非ランク/非専用スロットはビルドエラー**。
- **LEAD/ACE もフィルタ対象として利用可**（既存LEAD/ACEが優先、上限超過分は除外）。
