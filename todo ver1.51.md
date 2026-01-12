### DexNav Register (R押下) が揺れない
- **対応済**: R押下時のDN_FLAG_SEARCHINGが幽霊状態で立ちっぱなしだったため入力側で弾かれていた。R入力時にDexNavタスク有無を確認し、タスクが無ければフラグをクリアして再試行するよう修正。  
- **ログ強化**: R入力経路・検索開始成否・TryStartHiddenMonFieldEffect失敗時に常時ログを出すよう追加。地形（beh）、mapType、タイル座標も出力されるので原因特定が容易に。  
- **現状**: フィールドでRを押下すると登録ポケモンが揺れることを確認済み。  
- **残課題**: TryStartHiddenMonFieldEffectが失敗するケースはまだ発生しうる（選定タイルが草判定でないなど）。必要なら小スキャンfallback等を検討。

### The Pokemon got away! の扱い
- 現仕様のまま（タイムアウト/離脱で必ず出す）。要変更なら別途方針を決める。
