Trainer固有名・ファイル名の正規化サンプル（ビルド未使用のメモ用）

- キー（ID）は ASCII 大文字と数字とアンダースコアのみを許可。入力文字列をトリムし、`[A-Z0-9]` 以外は以下で処理する。
  - スペース/ハイフン/スラッシュはアンダースコアに変換。
  - アポストロフィやピリオドなどその他の記号は削除。
  - 連続するアンダースコアは1つに圧縮し、先頭末尾のアンダースコアは除去。
  - 文字がなくなった場合はエラー。
- ファイル名はキーをローワーケースにした snake 形式＋`.party` を基本形とする（例: `LEADER_ROXANNE` → `leader_roxanne.party`）。大文字ファイルも許容する場合は tolower をかませる前提で一致判定する。
- ファイル内のラベル行 `=== ... ===` はキーそのもの（大文字）を使う。
- 例: 5ケース

| 元の表記                | 正規化キー            | 推奨ファイル名                         |
|-------------------------|-----------------------|----------------------------------------|
| Roxanne                 | ROXANNE              | data/trainer_rank_party/roxanne.party  |
| Leader Roxanne          | LEADER_ROXANNE       | data/trainer_rank_party/leader_roxanne.party |
| Team Magma Grunt (F)    | TEAM_MAGMA_GRUNT_F   | data/trainer_rank_party/team_magma_grunt_f.party |
| Wally-Rematch 2         | WALLY_REMATCH_2      | data/trainer_rank_party/wally_rematch_2.party |
| Elite Four (Phoebe)     | ELITE_FOUR_PHOEBE    | data/trainer_rank_party/elite_four_phoebe.party |

- 文字数のハード上限は当面なし（参考: 現行 `TRAINER_*` ID の最長は接頭辞除き30文字）。極端に長いものは将来制限を検討。重複キーはビルド時にエラーにする。
