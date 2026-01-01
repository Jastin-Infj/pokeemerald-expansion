# randomizer_v1.40 変更内容まとめ

## リリースノート風サマリ
- encounterRate/allowEmpty の厳格化（allowEmpty=true は encounterRate 未指定必須、書けばエラー）。  
- rare設定の厳格化（slotMode=rare で rareRate/rareSlots 欠落や rareWL空はエラー）。  
- maxRerolls `"auto"` 追加（静的に WL-BL 件数から min(件数,8)）。  
- 釣り用 slotSet 追加、route_103_fishing を slotSet ベースに簡素化。  
- route_102_land のレア枠を legend_unlock + specialOverrides 許可に。  
- encounterRate を各エリア/slotSet に明示、allowEmpty 空エリアは未指定。  
- ポップアップ時の allowEmpty ログは時間帯解決して1回だけ WARN。  
- ビルド/生成：`python3 dev_scripts/build_randomizer_area_rules.py`、`CPPFLAGS_EXTRA="-UNDEBUG" CFLAGS_EXTRA="-UNDEBUG" make -j4` 成功。

## データスキーマ・バリデーション
- **encounterRate必須化**: slotSet/エリア/timeSlotのいずれにも encounterRate が無い場合はビルドエラー。  
  - 例外: allowEmpty=true の空エリアは encounterRate 未指定が必須（0も書かない）。  
- **allowEmpty時の制約**: allowEmpty=true なら slotCount=0 かつ encounterRate 未指定。encounterRate を書くとエラー。  
- **rare設定の厳格化**: slotMode=rare で rareRate/rareSlots が null/欠落ならエラー。rareSlots>0 で rareRate=0 もエラー。  
  - rare枠WLが空の場合もエラー（allowEmptyでない限り）。  
  - uniform で rare 上書きする場合は rareRate/rareSlots を両方書くか、完全に未指定で slotSet 継承（片方だけはエラー）。  
- **Fishing**: allowEmpty=true のときでも timeSlot 内で allowEmpty:true を明記する必要あり（空ブロック禁止）。  
- **maxRerolls: "auto"** をサポート: WL-BL の静的件数から min(件数, 8) を自動設定（0件かつallowEmptyなら0、最低1）。  

## 主要データ変更
- **slotSets**  
  - vanilla/even_land/rare_land_sample に encounterRate:15 を付与。  
  - fishing_mixed_sample を追加（old: rareRate=15/rareSlots=1, good: 空, super: uniform）。  
  - empty_area: allowEmpty=true（encounterRate未指定）。  
- **route_102_land**  
  - slotSet: rare_land_sample (slotMode=rare, rareRate=20, rareSlots=2 継承)  
  - rare.apply に legend_unlock を追加、specialOverrides で legend系を許可。  
- **route_102_fishing**  
  - allowEmpty 空エリア (slotSet: empty_area)、timeSlots.any に allowEmpty:true を明記。  
- **route_103_fishing**  
  - slotSet: fishing_mixed_sample を使用。timeSlots.any で old/super のみ apply/remove を上書き、good は空。  
- **route_103_land**  
  - encounterRate:15 を明示（未指定だと0で遭遇しないため）。  
- **route_104_land**  
  - allowEmpty: true (any), 朝は encounterRate:0 で出現なし。夜のみ allowEmpty:false + encounterRate:15 で出現あり。  
- **他エリア**  
  - route_110_water / route_121_land / route_128_water などに encounterRate:15 を明示。  
  - Hidden/Gift サンプル (hidden_* / gift_default) に encounterRate を付与（allowEmpty 空は未指定/0）。  

## コード変更
- **randomizer.c**: allowEmpty ログを map_name_popup 側に統一（randomizer.c からは削除済み）。  
- **map_name_popup.c**: ポップアップ表示時に現在の timeSlot を解決して Land の allowEmpty ログを1回だけ WARN 出力（デバッグフラグON時）。  
- **build_randomizer_area_rules.py**:  
  - 上記バリデーション強化（encounterRate必須、allowEmptyとencounterRateの組合せ禁止、rare矛盾のエラー化）。  
  - maxRerolls "auto" 追加。  
  - Fishing: allowEmpty=true で timeSlotに allowEmpty:true 明記を強制。  

## ビルド/生成状況
- `python3 dev_scripts/build_randomizer_area_rules.py` 成功。  
- `CPPFLAGS_EXTRA="-UNDEBUG" CFLAGS_EXTRA="-UNDEBUG" make -j4` 成功。  

## 残課題 (次回TODO)
- maxRerolls をランタイムで「実効件数ベース」に自動調整する案（現在は静的 WL-BL 件数のみ）。  
- Fishing/good rod など、空時の encounterRate/allowEmpty の扱いを運用方針に合わせて再確認。  
