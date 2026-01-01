# TODO ver1.41: maxRerolls をランタイム実効件数ベースに自動調整する

## 背景
- 現状の `"auto"` は生成時に WL-BL の静的件数で min(件数, 8) を決めるのみ。  
- 実際のランタイムではレア分岐・カテゴリフィルタ・フォーム除外などで有効候補がさらに減るため、静的件数では十分でない場合がある（フォールバックが多発）。

## 方針案
1) ランタイムで「許可される候補数」を数えて、maxRerolls の上限を動的に決める。  
   - 例: `effective = count(allowed candidates)`; 推奨 `mr = clamp(effective, 1, 8)`; allowEmpty で 0 件なら mr=0。  
   - 探索に使うプール: WL - BL - カテゴリ不可 - フォーム不可 をざっくりチェック。  
   - 重い場合は「レア/通常どちらを抽選中か」で対象の WL/BL だけを見る簡易版にする。
2) `"auto"` のときのみ動的に置き換え。手動指定があればそのまま使う。  
3) レアヒット時と通常時で別カウントも可（rareWLとnormalWLで各々算出）。  
4) allowEmpty=true かつ 0 件なら即ブロック（現在の挙動維持）。

## 実装ポイント
- `RandomizeWildEncounterBlocked` 内で `maxRerolls` を上書きするフックを追加。  
- 有効候補カウント関数を作る（rare/normal を bool で切替）。  
- 上限/下限は現行と同じく 0〜8 を基本に検討。

## バリデーション/ログ
- `"auto"` をランタイム換算する旨をデバッグログに一度出す（WARN/FLAG_RANDOMIZER_DEBUG_LOG）。  
- ビルド時は `"auto"` を許容したまま（既存静的計算は廃止または fallback）。

## テスト項目
- WL-BL が多い場合でも最大値まで抑制されるか。  
- WL-BL 0 のとき allowEmpty でブロック、allowEmpty=false はエラーにならないか。  
- レア抽選時、rareWL が空/少数でも正しい上限に下がるか。  
- ポップアップ時ログ/遭遇ログに maxRerolls が反映されているか。

## 追加検討事項
- **レポート出力フック**: `--report` で各エリア/timeSlot/rod の有効WL/BL件数（rare/normal別）、allowEmpty、encounterRate、maxRerolls(auto換算後) を一覧出力。警告（rareWL空、allowEmptyなのにWLあり等）も含め、生成後にサマリを標準出力。  
- **ランタイム maxRerolls auto ログ**: 1回目だけ `maxRerolls(auto)=X (effective=Y)` を WARN（デバッグフラグON時、マップ/area/timeSlot単位で抑制）。  
- **rareHit統計ログ**: デバッグ限定で「rareHit率、effectiveWL、fallback率」など簡易集計を1マップ1回にまとめて出す。  
- **slotSet encounterRate 必須化**: slotSet に encounterRate を必ず持たせ、上書きはエリア/timeSlot側で。allowEmpty=true の場合は encounterRate 未指定必須（書いたらエラー）。  
- **Fishing slotSet 整備**: all-uniform/all-rare 等のテンプレを追加し、rodごとの allowEmpty 空指定の扱いをスキーマで統一。rodデータ省略時は空WL自動補完。  
- **カテゴリ整合チェック**: specialOverrides と WL の矛盾（legend_unlock なのに legend:false など）はスクリプトでエラーにする。カテゴリ未定義種を含む場合の警告も追加。  
- **テストデータ整理**: 基本パターン（空、uniform、rare、釣り）に絞った回帰用データセットを用意し、検証工数を削減。  
- **maxRerolls auto ランタイムチューニング**: rare/normal別に有効件数を数え、抽選直前に上限を動的決定（フォーム/カテゴリ除外も簡易評価）。  
- **encounterRate デフォルト明文化**: slotSet必須（allowEmpty空のみ未指定）。エリア側は上書き専用。釣り/hidden/gift で必要ならデフォルト定数を分ける。  
- **警告/エラー閾値のドキュメント化**: rareWL空→エラー、カテゴリ矛盾→エラー、allowEmpty+WLあり→エラー、encounterRate未指定（allowEmpty以外）→エラー、など優先度を明文化。  
- **CI用 dry-run**: `--check` モードで生成なしバリデーションのみ実行し、将来のスキーマ変更にも耐えられる形でCIに組み込む。

## 新規で進めるタスク案（上記から具体化）
- **Pythonレポートの粒度統一**: `--report` で警告レベルをタグ化（ERROR/ WARN/ INFO）し、生成後のサマリに件数を表示。デバッグログ(WARN)も同様に抑制キャッシュを導入。  
- **カテゴリ整合テーブル生成**: specialOverrides 用のカテゴリ定義をテーブル化し、スクリプトとランタイム両方で参照。矛盾は即エラー。  
- **Fishingスキーマ軽量化**: rod未指定時は allowEmpty=true なら自動で空WL/空weightsを補完。rodごとの allowEmpty 空指定をスキーマで明示し、バイト率(encounterRate)は slotSet必須に。  
- **上限チェック強化**: スロット上限/weights合計/levelBands長など、エラーメッセージに期待値と実測値を記載して案内。  
- **カテゴリ警告詳細化**: どのWL（rare/normal）に矛盾種がいるかを警告メッセージに含める。  
- **回帰用テストデータ最小セット**: 空/均等/レア/釣りの4〜5ケースだけの `test_scenarios.yml` を用意し、CIの `--check` で使う。  
- **maxRerolls auto ランタイム化**: rare/normal 別に実効件数を数え、抽選直前に clamp(1..8) で決定。デバッグログで一度だけ `maxRerolls(auto)=X (normal=.., rare=..)` を出す。  
- **encounterRate方針の明文化**: slotSetは必須、allowEmpty空のみ未指定可。エリア/timeSlotは上書き専用。釣り/hidden/gift で別デフォルトを持つなら定数を分ける。  
- **CIドライラン**: `--check` を追加し、生成物を書き出さずバリデーションのみ実行。警告/エラー閾値はオプションで調整可能にする。  
- **スクリプト単体テスト**: pytest等でrare/allowEmpty/encounterRate/上限チェックをユニットテスト化し、CI `--check` と併用。  
- **レポート差分比較**: `--report` をCSV/JSONで保存し、前回との差分（WL件数/encounterRate/警告数）を検知する仕組みを追加。  
- **ログレベル切替の定数化**: デバッグログのWARN/INFO切替をconfig/#defineで制御し、実機ではWARN固定にできるスイッチを用意。  
- **スキーマバージョンとマイグレーションガイド**: schemaVersionを明記し、主要変更点とマイグレーション手順を短くまとめる。  
- **例外マップの検証補助**: 例外マップ一覧をレポートに含め、意図しないバニラ化を防ぐ。  
- **フォーム/カテゴリ未定義監視**: 未分類種や新フォームをWL/BLに含めた場合の警告オプションを追加。  
- **フィールドログ抑制キー拡張**: allowEmpty以外のWARNもマップ/時間帯単位で1回に抑制するキャッシュを追加し、実機デバッグのノイズを軽減。  
- **重み0・レベル帯境界チェック**: weightsに0が含まれる/levelBands min=max などを警告。固定レベル運用ならINFO扱いにするオプションも検討。  
- **遭遇率/バイト率のデフォルト定数化**: Land/Water/Fish/Hidden/Gift 用のデフォルト定数をまとめ、レポートにも表示して整合確認を容易にする。  
- **レア枠フォールバック方針の明文化**: rareHitしたのにrareWLが空のときはエラー or 警告 or 通常WLへフォールバック、のどれかに固定しスクリプトでも警告を出す。  
- **小規模リハーサルデータ**: 空/均等/レア/釣り/hidden/gift を少数に絞ったサンドボックスYAMLを用意し、`--check`＋実機で挙動確認できるセットを作る。  
- **エラーメッセージ整形**: 用語（encounterRate/allowEmpty/rareRate等）を統一し、短い修正例を併記する。  
- **CIレポート閾値**: `--report` の警告件数や WL=0 のマップがあればCIを失敗させるゲートを追加。  
- **slotSetサンプル期待値表**: rare_land_sample や fishing_mixed_sample など主要 slotSet の期待挙動を簡潔な表にして、データ修正時の目安にする。  
- **CIでレポートをアーティファクト化**: `--report` のCSV/JSONをCIの成果物として保存し、リリース前後の差分確認を容易にする。`pre-release-checklist.md` の手順とCIジョブを紐づけ。

## 進め方とチェック
- 影響の大きい変更（maxRerollsランタイム化、ログ抑制など）は「レポート充実 → バリデーション強化 → ランタイム変更」の順で小さく分割して進める。  
- サンドボックスYAMLで `--check` + 実機確認を行うときのチェックリスト（例）  
  - allowEmptyエリアで遭遇なしを再現できるか  
  - rare枠（legend_unlock等）が意図通り出る/出ないか  
  - Fishing(old/good/super) の空/出現ありが期待通りか  
  - maxRerolls auto (静的) で過度なfallbackが出ていないか  
  - WARNログが抑制され、デバッグ時に必要十分な情報だけが見えるか  
