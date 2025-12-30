場所別ランダマイザー 詳細設計（ver1.2：時間帯別WL対応）
===============================================

目的
- 時間帯別（morning/day/evening/night）にWL/BLを切り替えられるようにする。時間帯ON時のみ適用、OFFなら共通WLのみ。
- フォールバックや空WLの扱いを明確化し、バリデーション/ログ方針を固定する。

YAML拡張方針
- timeSlotsを追加: `areas.[entry].timeSlots.{morning/day/evening/night}` を必須4キーで持つ（1つでも欠けたらビルドエラー）。未知キーはエラー。YAML側で時間帯を増減させない。
- 共通＋上書き: 共通フィールドをベースに、timeSlotsに書いたフィールドだけ上書き。未記載は共通を継承。
  - 継承対象: apply/remove/maxRerolls/allowLegendOverride/slotMode/rareSlots/rareRate/maxSpecies/maxSpeciesOverrides。
- 釣りの時間帯: timeSlots内にold/good/superを全て記載（欠けたらエラー）。ロッド設定は共通→timeSlotロッドで上書きし、未記載フィールドは共通ロッド設定を継承。

時間帯ON/OFFと挙動
- 時間帯ON: 現在の時間帯のtimeSlotを使用。空ならフォールバック判定へ。
- 時間帯OFF: 共通のみを使用。共通WLが空なら警告（ビルドは通す）。

フォールバック仕様（時間帯ON）
- グローバル設定のみ使用（`OW_TIME_OF_DAY_FALLBACK`）。YAMLで個別指定はしない。
- `OW_TIME_OF_DAY_DISABLE_FALLBACK=TRUE`: timeSlotが空なら出現なし（警告のみ）。空でなければ通常使用。
- `OW_TIME_OF_DAY_DISABLE_FALLBACK=FALSE`: timeSlotが空なら `OW_TIME_OF_DAY_FALLBACK` の時間帯テーブルを使用（空でなければ通常使用）。
- 多段フォールバック: 落ち先も空なら morning→day→evening→night→morning… とTIME_COUNT回まで巡回し、最初の非空を採用。全て空なら出現なし＋警告。無限ループ防止としてTIME_COUNTで打ち切る。
- 意図的な出現なし: timeSlotを空＋`DISABLE_FALLBACK=TRUE` で明示（定義漏れと区別）。`DISABLE_FALLBACK=FALSE` では落ち先が使われるので「出現なし」を維持したい場合はTRUEにする。
- レア/スロット設定: フォールバック先のtimeSlotの設定（slotMode/rareSlots/rareRate/maxSpecies等）を丸ごと適用。差分が出ても許容（レア枠なし→あり等）。

時間帯キー（固定）
- enum: `TIME_MORNING`, `TIME_DAY`, `TIME_EVENING`, `TIME_NIGHT`。
- YAML: `morning/day/evening/night` の4キーを必須で用意。未知キーはエラー。

バリデーション（生成スクリプト）
- エラー: timeSlots欠落（時間帯ONなのに1つもなし）、未知時間帯キー、全時間帯キー未定義、maxSpecies < rareSlots、slotMode矛盾（uniform＋rare指定）、rareRate>100、rareSlots=0でrareRate>0、釣り時間帯でロッド欠け、時間帯キーなし＋時間帯ON、その他既存チェック。継承後の最終値で判定。
- 警告: 時間帯ONで全時間帯空WL（出現なし）、時間帯OFFで共通WLが空、フォールバック巡回しても全時間帯空で出現なしに確定。意図的な出現なしは空配列＋コメントで明示。
- バリデーション順: 共通読込 → timeSlot上書き →（釣りならロッド上書き）→ 最終値でチェック。

ログ方針（デバッグ）
- 時間帯ON＋空WL＋フォールバック無効などはWARNで出力して確認しやすくする（デバッグフラグON時）。フォールバック巡回で出現なし確定もWARNで通知。

サンプル（コメント付きイメージ）
```
route_102_land:
  key: { mapGroup: MAP_GROUP(MAP_ROUTE102), mapNum: MAP_NUM(MAP_ROUTE102), areaMask: Land }
  apply: [starter_land]
  remove: [starter_trim]
  timeSlots:
    morning:
      apply: [starter_land, morning_extra]   # 朝だけ追加
    day:
      apply: [starter_land]                  # 共通に近い
    evening:
      apply: [evening_only]                  # 夕方専用
    night:
      apply: []                              # 空=出現なしを意図するなら DISABLE_FALLBACK=TRUE と組み合わせる
```

備考
- 時間帯種別やフォールバック先はグローバル設定のみ（`OW_TIME_OF_DAY_FALLBACK`）。YAMLで増減・個別指定はしない。
- 時間帯ON/OFFのスイッチは `OW_TIME_OF_DAY_ENCOUNTERS`（include/config/overworld.h）。既存のまま利用。

修正予定ファイル（見込みと理由）
- `dev_scripts/build_randomizer_area_rules.py`  
  - 理由: YAML拡張（timeSlots必須4キー、釣りロッド必須、継承・バリデーション強化、WARN/ERROR出力）を反映するため。  
  - 役割: `data/randomizer/area_rules.yml` をパースし、重複除去・継承・空WL/バリデーションを行い、`generated/randomizer_area_rules.h` を生成するスクリプト。
- `data/randomizer/area_rules.yml`  
  - 理由: timeSlotsを追加し、時間帯別のapply/removeや釣りロッド設定を記述できるようにするため。意図的な空WLはコメントで明示する。  
  - 役割: WL/BLの単一ソース（kits＋areas/gifts）として、時間帯別・釣りロッド別の設定も含めて管理する。
- `generated/randomizer_area_rules.h`（生成物）  
  - 理由: スクリプト変更により出力形式が拡張されるため再生成が必要。  
  - 役割: ランタイムが参照するエリアルール・釣りルール・プール配列の定義。
- `src/randomizer.c`  
  - 理由: timeSlots対応のランタイム選択ロジックやログ追加が必要になる場合に修正。バリデーションはスクリプト側だが、空WL＋フォールバック無効などのログ出力を入れる場合に調整する。  
  - 役割: ランタイムのランダマイザ本体（WL/BLフィルタ、釣りルール適用、デバッグログなど）。
- `include/config/overworld.h`（設定確認のみ、変更は想定せず）  
  - 理由: `OW_TIME_OF_DAY_ENCOUNTERS` やフォールバック設定を利用するための確認。基本は既存のまま。  
  - 役割: 時間帯仕様のグローバル設定（時間帯ON/OFF、フォールバックON/OFF、フォールバック先など）。
- `todo ver1.2.md` / `todo-script new ver1.2.md`（本メモ）  
  - 理由: 仕様変更をドキュメントとして残し、数年後に見直す際の参照用。  
  - 役割: 設計方針・バリデーション・ログ方針・ファイル役割を明文化。
