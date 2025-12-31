# todo ver1.31: timeSlots マージ仕様拡張

## 目的
- timeSlots で `any` と特定時間帯（morning/day/evening/night）を併記可能にし、`any` をベース設定として扱い、特定時間帯で上書きできるようにする。
- 例: `any` で `allowEmpty: true`（遭遇なし）をベースにしつつ、`morning` だけ `default/rare` を書けば、morning では遭遇ありにしたい。

## 仕様案
- timeSlots キー併記を許可: `any` + 各 timeSlot の同時記述を許容する（従来は排他）。
- マージルール:
  - `any` をベース。特定時間帯は「書いたフィールドだけ」上書き。書かなかったフィールドは `any` を継承。
  - 対象フィールド: default/rare ブロック（apply/remove/rareRate/rareSlots）、slotMode/maxSpecies/maxRerolls/specialOverrides/allowEmpty/fishing ブロック（rod別の中身含む）。
  - `allowEmpty` の再解釈: 特定時間帯で default/rare/fishing に有効なデータが書かれている場合、`allowEmpty` が未記載なら自動で false（遭遇あり）扱いに落とす。明示的に true と書いたときだけ遭遇なしを許容。ベース `any` が true でも、時間帯側でデータを書いたら false に転ぶ想定。fishing の timeSlots/rod でも同じロジックを適用（rodにデータがあれば未記載allowEmptyはfalse）。
- バリデーション:
  - `any` と個別キー併記を許容するが、個別キーが無い場合は従来どおり morning/day/evening/night 全て必須。
  - 正当パターンは次の3つのみ:
    1) `any` のみ
    2) `any` + 特定時間帯（例: morning だけ、morning+night など任意組み合わせ）
    3) morning/day/evening/night 全て記載（`any` なし）
    → これ以外（any なしで4時間帯が揃っていない等）はエラー。
  - `allowEmpty=true` で non-empty WL を同時指定したらエラー（従来どおり）。時間帯側でデータを書いたら allowEmpty 未記載でも false になるので遭遇可。
  - 生成物は継承後の値を展開してからバリデーション（maxSpecies/rareSlots など）を行う。
  - エラーメッセージの粒度: 
    - timeSlotsパターン違反: 「<area>/timeSlots: invalid pattern (use 'any', 'any+partial times', or all morning/day/evening/night)」
    - allowEmpty矛盾: 「<area>/<time>/[fishing/rod]/allowEmpty=true but whitelist is not empty」
    - maxSpecies/rareSlots範囲: 「<area>/<time>/... maxSpecies must be >0 (or allowEmpty true)」「rareSlots X exceeds maxSpecies Y or slot limit」
    - slotMode不整合: 「slotMode=uniform cannot have rareRate/rareSlots」
    - rareRate範囲: 「rareRate must be 0-100」
    - missing rod/timeSlot: 「<area>/<time>: missing rod 'good'」等
    - deprecatedキー検出: 「allowLegendOverride is deprecated; use specialOverrides.legend」

## 実装メモ
- `dev_scripts/build_randomizer_area_rules.py`
  - timeSlots 正規化時に `any` + 個別キーを許容し、継承ロジックを追加（any→時間帯→rodの順でマージ）。fishing ブロックも同様に継承。
  - allowEmpty の扱いを「時間帯/rodに有効データがあれば未記載でも false」に変更。
  - 既存の排他チェック（anyと個別を同時禁止）を撤廃し、マージ後にルール生成。
  - バリデーションは「マージ後の確定値」で実行（maxSpecies/rareSlots/rareRate/slotMode/allowEmpty/WL空チェックなど）。
- ドキュメント更新: randomizer_v1.30 の後続版に timeSlots 併記とマージルールを明文化。
