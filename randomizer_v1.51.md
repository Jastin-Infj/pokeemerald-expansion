# randomizer v1.51 メモ

- DexNav: R押下でRegisterしたランダマイザー種が揺れない問題を修正。DN_FLAG_SEARCHINGの幽霊立ちを検出しクリアして再試行。
- DexNav: R入力〜検索開始〜フィールドエフェクト開始失敗までのログを強化（種/環境/タイル座標/beh/mapTypeを出力）。
- DexNav: ランダマイザーでレベル帯が無い場合もレベルを算出するフォールバックを追加済み（v1.51系で導入）。
- 注意: TryStartHiddenMonFieldEffectが地形判定で落ちる場合はログを確認し、小スキャンfallback等を検討する。
