# pre-release-checklist (ランダマイザー用)

1) スクリプト/バリデーション
   - [ ] `python3 dev_scripts/build_randomizer_area_rules.py --check`（生成なしモードがあれば）を実行し、エラー/警告ゼロを確認  
   - [ ] `--report` で WARN/ERROR 件数・WL=0 マップがないか確認（CIゲートがあれば通過）  
   - [ ] pytest などユニットテスト（rare/allowEmpty/encounterRate/上限チェック）が通ること

2) サンドボックスYAMLでの挙動確認
   - [ ] 空/均等/レア/釣り/hidden/gift を少数に絞ったサンドボックスで `--check` 実行  
   - [ ] 実機デバッグ（フラグON）で以下を目視確認  
       - allowEmpty エリアは遭遇なし  
       - rare枠（legend_unlock 等）が意図通り出る/出ない  
       - Fishing (old/good/super) の空/出現ありが期待通り  
       - maxRerolls(auto) で過度な fallback が出ていない  
       - WARN ログが抑制され必要十分な情報のみ

3) 本番データのレポート確認
   - [ ] `--report` の差分を前回と比較し、意図しない WL/BL/encounterRate の変更がないか確認  
   - [ ] カテゴリ整合（specialOverrides と WL の矛盾がない）  
   - [ ] 例外マップ一覧を確認し、意図しないバニラ化がない

4) ドキュメント更新
   - [ ] randomizer_vX.Y.md にリリースノート風サマリを追記  
   - [ ] schemaVersion/マイグレーションガイドを最新に更新

5) ビルド
   - [ ] `CPPFLAGS_EXTRA="-UNDEBUG" CFLAGS_EXTRA="-UNDEBUG" make -j4` 成功  
   - [ ] 実機用 ROM 差し替えを忘れずに
