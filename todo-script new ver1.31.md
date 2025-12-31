# todo-script new ver1.31.md — 詳細設計（timeSlotsマージ拡張）

## スコープ / 目的
- timeSlots で `any` と個別時間帯（morning/day/evening/night）を併記可能にし、`any` をベース設定として特定時間帯が上書きできるようにする。
- allowEmpty の自動 false 落ちロジックを any→時間帯→rod で統一適用（fishing 含む）。
- バリデーションを「マージ後の確定値」で実行し、エラー粒度を明確化する。

## 要件
1) timeSlots パターン
   - 有効パターンは 3 種類のみ：
     - `any` のみ
     - `any` + 一部時間帯（例: morning / morning+night など任意組み合わせ）
     - morning/day/evening/night の 4 キー全部（any なし）
   - 上記以外（any なしで 4 キー未満）はエラー。
2) 継承順と上書き
   - any → 時間帯 → rod（fishing）でマージ。書いたフィールドだけ上書き、未記載は継承。
   - 対象フィールド: default/rare（apply/remove/rareRate/rareSlots）、slotMode/maxSpecies/maxRerolls/specialOverrides/allowEmpty、fishing ブロック内の rod 別設定。
3) allowEmpty の扱い
   - any 側が true でも、時間帯/rod 側で有効データ（default/rare/fishing.*）が書かれていれば、allowEmpty 未記載なら自動で false（遭遇あり）に落とす。
   - 明示的に true を書いた時だけ遭遇なしを許容。allowEmpty=true で WL が非空ならエラー（従来どおり）。
4) バリデーション（マージ後の値で実行）
   - slotMode=uniform で rareRate/rareSlots 指定はエラー。
   - rareRate は 0–100、rareSlots は 0 以上、maxSpecies/slotLimit 超過はエラー。
   - missing rod/timeSlot はエラー（allowEmpty=true で rod が欠けている場合はゼロルールを自動挿入する現行仕様を継承）。
   - deprecated キー allowLegendOverride 検出でエラー（既存仕様を維持）。
5) エラー文例（デバッグしやすく）
   - timeSlots pattern: `<area>/timeSlots: invalid pattern (use 'any', 'any+partial times', or all morning/day/evening/night)`
   - allowEmpty 矛盾: `<area>/<time>/[fishing/rod]/allowEmpty=true but whitelist is not empty`
   - maxSpecies/rareSlots: `<area>/<time>/... maxSpecies must be >0 (or allowEmpty true)`, `rareSlots X exceeds maxSpecies Y or slot limit`
   - slotMode/rareRate: `slotMode=uniform cannot have rareRate/rareSlots`, `rareRate must be 0-100`
   - missing rod/timeSlot: `<area>/<time>: missing rod 'good'`
   - deprecated: `allowLegendOverride is deprecated; use specialOverrides.legend`

## 実装タスク
1) スクリプト修正 — `dev_scripts/build_randomizer_area_rules.py`
   - timeSlots 正規化を any+個別許容に変更し、マージ処理（any→time→rod）を導入。
   - allowEmpty 自動 false 落ちの判定を any/time/rod 共通ロジックに実装。
   - バリデーション順を「マージ後」に変更し、上記エラー文を出力。
   - 既存の any と個別の排他チェックを撤廃。
2) データ整備（必要に応じて）
   - `data/randomizer/area_rules.yml` にパターン違反がないか確認。any+個別を使う場合は新仕様で記述。
3) ドキュメント
   - `randomizer_v1.30.md` 後継として timeSlots マージ仕様の追記（v1.31 相当）。

## 事前読み取りメモ（修正前の現状）
- `dev_scripts/build_randomizer_area_rules.py`
  - 現状は any と個別 timeSlot の併記を禁止する排他チェックが残っている。
  - allowEmpty は自動 false 落ち未実装、マージ/バリデーション順も現仕様のまま。
- `include/config/randomizer.h`
  - RANDOMIZER_AVAILABLE など既存設定。今回の timeSlots 拡張では直接の変更なし。

## 影響ファイル / 役割
- `dev_scripts/build_randomizer_area_rules.py`: timeSlots/allowEmpty マージ＆バリデーションロジックの変更。
- `data/randomizer/area_rules.yml`: 新スキーマでの記述確認・調整（any+個別を使う場合）。
- `generated/randomizer_area_rules.h`: スクリプト実行で再生成。
- `randomizer_v1.30.md`（または新v1.31記録用md）: 仕様ドキュメント更新。

## テスト観点
- any のみ / any+morning / morningだけ（エラー） / morning+day+evening+night（OK）をそれぞれスクリプトで検証。
- allowEmpty=true かつ WL 非空でエラーになること。
- slotMode=uniform + rareRate/rareSlots でエラーになること。
- rod が欠けている場合の挙動（allowEmpty=true ならゼロルール挿入、それ以外はエラー）が維持されていること。
