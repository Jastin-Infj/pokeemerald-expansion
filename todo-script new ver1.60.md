## トレーナーランク .party 実装タスク（v1.60 用）

### 目的
- ランク共通/固有 `.party` をビルド前に検証し、トレーナーへ割り当てる生成パスを作る。
- 重み/合計の厳密バリデーションと、優先順位（ブラックリスト→固有→Normal/Rare指定→共通）を実装する。

### 前提・仕様抜粋
- WeightTotal は 100/1000 のみ許可。0 禁止。`NONE` は合計から除外し抽選対象外。合計が WeightTotal と一致しない場合はビルドエラー。
- ランク共通 .party は Class/Pic/Gender 省略。固有 .party は保持。
- `trainer.party` 拡張フィールド: `NormalRank`, `NormalCount`, `RareRank`, `RareCount`, `AllowDuplicates`(true/false, 省略時 false)。
  - NormalRank→NormalCount 必須、RareRank→RareCount 必須。Count だけ／Rank だけはエラー。
  - 重複禁止時に所定数を埋められなければビルドエラー。
- 優先順位: 1) ブラックリスト → 2) 固有キー → 3) Normal/Rare+Count 指定 → 4) 共通デフォルト。

### 実装手順案（スクリプト＋生成）
1. バリデーション兼生成スクリプトを追加（例: `dev_scripts/build_trainer_rank_parties.py`）  
   - 入力: `data/trainer_rank_party/*.party`（共通/固有）と `trainer.party`。  
   - チェック: WeightTotal=100/1000、0禁止、NONE除外、合計一致。Rank/Count の整合。AllowDuplicates 値。  
   - ブラックリストは既存 `src/data/randomizer/trainer_skip_list.h` を読み込むか、別入力で受ける（サンプルリストも受け付け可能に）。  
   - 優先順位を解決し、トレーナーID→使用キー/ランク/Count/AllowDuplicates をまとめた生成ヘッダを出力（未決だが `generated/trainer_rank_parties.h` 等を想定）。
2. Makefile への組み込み  
   - 上記スクリプトをビルド前フックに追加し、失敗時はビルドを止める。  
   - WeightTotal のモードは環境変数またはスクリプト引数で 100/1000 を切替（それ以外はエラー）。
3. ログ出力  
   - バリデーション失敗時に「ファイル名/キー/合計/不足・過剰/none件数」を表示。  
   - 優先順位解決時にブラックリスト命中や固有使用を情報ログとして出せるようにする（デバッグ用オプション）。  
4. サンプル/テスト  
   - 現行ドラフト `data/trainer_rank_party/` を入力にテスト実行し、ビルドが通ることを確認。  
   - ブラックリスト用のサンプル一覧を別途テキストで用意して確認を依頼する。

### 実装方法の選択肢（どれでも可、任意に選ぶ）
1. 前処理スクリプト方式（推奨）  
   - `trainer.party` は無改造のまま読み込み、拡張フィールド（NormalRank/Count, RareRank/Count, AllowDuplicates）を別YAML/JSONで受け取るか、コメントタグで埋め込んで前処理スクリプトで吸収し、Cの生成入力に変換する。  
   - メリット: `tools/trainerproc` を最小限の変更で済ませられる。  
   - デメリット: 変換レイヤーが増える。
2. `tools/trainerproc` を拡張  
   - 直接 `main.c` のパーサにフィールドを追加し、trainer.party に記述できるようにする。  
   - メリット: 単一ソースで完結。  
   - デメリット: パーサ改修の影響範囲が広い。
3. C 側で static_assert 的に検証（補助）  
   - 生成ヘッダに合計や件数を埋め込み、C 側で追加の検証をかける。  
   - 単独では不足なので、1 or 2 と併用。

### 追加決定待ち
- ブラックリストに入れる具体的なトレーナーIDリスト（後で指示を受けて反映）。

### 参照ファイルと役割（実装時に読む想定）
- `src/data/trainers.party` … 既存トレーナー定義（Showdown互換）。拡張フィールドをどこに挿すか検討が必要（未対応だと parser が弾く）。
- `tools/trainerproc/main.c` … `trainer.party` のパーサ/ジェネレータ。新フィールド（NormalRank/Count, RareRank/Count, AllowDuplicates）を通すにはここを拡張するか、別プリプロセスで吸収する必要あり。マクロや pool_* フィールドの扱いに注意。
- `include/constants/trainers.h` … トレーナーID定義。正規化キー衝突チェックやブラックリストの参照元。
- `src/data/randomizer/trainer_skip_list.h` … 既存ブラックリストのデータソース。追加IDをここへ入れる前提。
- `data/trainer_rank_party/*.party` … 共通/固有ランクプール（WeightTotalヘッダあり）。共通は Class/Pic/Gender なし、固有は保持。
- `data/trainer_rank_party_samples/*.party` … ビルド対象外のサンプル。フォーマット確認用。

### 予想される実装上の懸念点
- `trainer.party` パーサが新フィールドを未対応のままだとビルドが落ちるため、拡張か前処理が必須。
- Rank/Count のペア必須、NONE の除外、WeightTotal=100/1000 厳守の複合チェックが増えるので、エラーメッセージを明確にしないとデバッグが難しい。
- 優先順位ロジックをどこで解決するか（生成スクリプト側が現実的）を決めておかないと、C 側での条件分岐が増えがち。
- ブラックリストの確定待ち。仮リストで実装すると後で差し替えが必要になる。
