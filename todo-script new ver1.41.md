# TODO-SCRIPT new ver1.41.md — maxRerollsランタイム自動化＆周辺強化 詳細設計

## ゴール
- `"auto"` 指定の maxRerolls をランタイムの実効候補数ベースで決定し、フォールバック過多を抑制する。
- レポート/バリデーションを拡張し、カテゴリ矛盾や rare 設定漏れを早期検出する。
- slotSet 周りの必須項目（encounterRate など）を明文化・強制し、テストデータを整理する。
- 既存挙動を極力維持しつつ段階的に導入できるよう、実装フローと検証手順を細分化する。

## 対象ファイル・役割
- `dev_scripts/build_randomizer_area_rules.py`
  - maxRerolls=auto の解釈変更（静的→ランタイム算出前提）。既存の静的 min(WL-BL,8) は fallback/警告に移行。
  - レポート/チェックモード（`--report`, `--check`）の整備、警告/エラー粒度の明示。
  - encounterRate 必須化（slotSet）と allowEmpty=true 時の未指定強制。rare 設定の矛盾検出強化。カテゴリ整合チェック。
  - fishing slotSet のテンプレ補強、rod 未指定時の自動補完（allowEmpty=true のみ）などスキーマ軽量化。
  - テスト/サンプル YML の最小セット整理（空/均等/レア/釣り/hidden/gift）。
- `data/randomizer/area_rules.yml`（および slotSet テンプレ）
  - 必須フィールド整合（encounterRate、rareRate/rareSlots 両立など）を反映。新テンプレを追加。
- `generated/randomizer_area_rules.h`
  - maxRerolls フィールドは現状維持。auto 指定が来た場合に runtime 上書きできるようコメント/期待値を明記。
- `src/randomizer.c`
  - ランタイムで maxRerolls を決定するフックを追加（rare/normal 別カウント）。デバッグログに `maxRerolls(auto)=X (normal=Y, rare=Z)` を一度だけ出力（FLAG_RANDOMIZER_DEBUG_LOG）。
  - rareHit 統計の簡易ログ（1マップ1回抑制）。allowEmpty/undef ログ抑制キャッシュは既存の仕組みに合わせる。
- `src/wild_encounter.c` / `src/map_name_popup.c`
  - 必要ならシグネチャ調整やログの抑制キー拡張（マップ/area/timeSlot 単位で1回）を検討。既存のマップポップアップ連携は維持。
- `docs/randomizer_v1.41.md`（新規）
  - 変更概要、バリデーション方針、ログ/レポート仕様、テスト手順を記載。
- `pre-release-checklist.md`（必要に応じ更新）
  - `python3 dev_scripts/build_randomizer_area_rules.py --check --report` 実行をチェック項目に追加。

## 実装フロー（段階的）
1) **スクリプト強化（安全な read-only 作業）**
   - maxRerolls=auto の解釈を「ランタイム計算する」前提に変更し、静的算出はレポート用 fallback として残すか警告に格下げ。
   - バリデーション拡張：encounterRate 必須（allowEmpty=true なら未指定必須）、rareRate/rareSlots 片側欠落エラー、slotMode=uniform なのに rare 設定がある場合のエラー、カテゴリ矛盾（legend_unlock なのに legend:false）をエラー化。
   - レポート/`--check` 実装：警告/エラー件数、各エリアの有効候補数（静的）、encounterRate、allowEmpty、maxRerolls(auto想定) を一覧出力。
   - fishing slotSet テンプレ補強と rod 未指定時の補完ロジック（allowEmpty=true のみ自動空補完）を実装。
   - テスト用 YML 最小セット追加（空/均等/レア/釣り/hidden/gift）。CI 用に `test_scenarios.yml` などを用意。
2) **データ調整**
   - 既存 slotSet に encounterRate を明示（allowEmpty=true の空セットは未指定のまま）。rare 設定の整合を取り、カテゴリ矛盾を解消。
   - サンプル/テスト YML を `--check` で通るよう修正。エラー例はコメントでのみ提示。
3) **ランタイム maxRerolls 自動化**
   - `RandomizeWildEncounterBlocked` 直前で有効候補数を算出（rare/normal 別）。`maxRerolls=auto` のときだけ `clamp(1..8)` で上書き。allowEmpty で0件なら0を許可。
   - デバッグログ（FLAG_RANDOMIZER_DEBUG_LOG 時のみ、マップ/area/timeSlot/rod 単位で1回）を WARN で出力。
   - rareHit 統計の簡易ログを WARN で1マップ1回出すオプション（ログレベル切替用の #define を追加）。
4) **フィールドログ抑制拡張**
   - allowEmpty 等の WARN をマップ/area/timeSlot/rod 単位で1回にキャッシュ。既存抑制キーに timeSlot/rod を含める。
5) **検証**
   - `python3 dev_scripts/build_randomizer_area_rules.py --check --report`
   - 代表マップ実機チェック（空WL/rare/釣り・rod別）。maxRerolls(auto) のログ確認。
   - 回帰: route_101 allowEmpty、route_102 rare、route_103 fishing、route_104 time帯など。
6) **ドキュメント**
   - randomizer_v1.41.md に変更点と手順、バリデーション一覧、ログ仕様を記載。
   - pre-release-checklist 更新（必要なら）。

## バリデーション方針（要反映）
- slotSet encounterRate: 必須。allowEmpty=true の空セットのみ未指定可（指定したらエラー）。
- rareRate/rareSlots: 片方欠落・0/負数はエラー。slotMode=uniform で rare設定があればエラー。
- slotMode=rare なのに rareRate/rareSlots/rareWL 不足はエラー。rareHit 先が空ならエラーまたは警告＋フォールバックを固定（設計で決定）。
- allowEmpty=true で WL/weights が非空ならエラー。maxRerolls/slotCount/encounterRate 記載もエラー。
- カテゴリ整合: WL に legend_unlock 等があるのに specialOverrides.legend=false ならエラー。未分類フォームは警告。
- encounterRate 未指定（allowEmpty でない）: エラー。
- weights/rareWeights 合計: 100固定（バラつきはエラー）。levelBands の要素数と weights 長さが不一致ならエラー。

## ログ/レポート仕様（予定）
- ランタイム: `maxRerolls(auto)=X (normal=Y rare=Z)` を WARN で1回/キー。
- rareHit統計: `rareStats: hits=.. miss=.. effectiveWL=.. fallback=..` を WARN 1回/マップ。
- allowEmpty/undef: 既存同様 WARN 1回/キー。抑制キーに timeSlot/rod を含める。
- レポート: `--report` で JSON/CSV 出力（警告タグ付き）。`--check` は生成無しのバリデーション専用。

## テスト観点
- allowEmpty 空セット: 遭遇なし＋WARN 1回。encounterRate 未指定必須が効いているか。
- rare+legend_unlock: specialOverrides legend=true 時のみ出現。false ならエラー/警告にかかるか。
- fishing rod 別 allowEmpty 混在: old 出現、good/super 空などが期待通りか。
- maxRerolls(auto): WL 少数時に1〜少数、通常時に上限8で動くか。rare/normal で別計算されるか。
- weights/levelBands 整合: 合計100、要素数一致チェックが効くか。

## 実行コマンド例
- バリデーション＋レポート: `python3 dev_scripts/build_randomizer_area_rules.py --check --report`
- 生成のみ: `python3 dev_scripts/build_randomizer_area_rules.py`
- ビルド（デバッグログ有効）: `CPPFLAGS_EXTRA=\"-UNDEBUG\" CFLAGS_EXTRA=\"-UNDEBUG\" make -j4`

