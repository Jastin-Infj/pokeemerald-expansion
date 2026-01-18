# ランダマイザー v1.61 仕様ドラフト（タグ抽選）
将来の見返し用に、タグ抽選の設計案を整理する。  
目的は「トレーナー側のスロットにタグを付与し、共有/専用プールの中からタグ一致のみを重み抽選する」こと。

## 背景 / 目的
- 既存の `RANK_*` だけでは戦略性が弱い。  
  例: 「Water タグのみ抽選」「Support タグのみ抽選」など、タグで絞り込んだ構成を作れるようにしたい。
- タグに一致しない候補は除外し、残りの重み合計から抽選する（重み計算は現状のまま）。

## 実装方針（決定: 案A）
### 案A: 既存の Tags / MON_POOL_TAG を再利用（最小改修）
**概要**  
trainers.party でも `Tags:` を使い、rank pool の .party にも `Tags:` を付ける。  
ランタイムは **タグ一致だけ抽選** し、**先にフィルタ→残った重み合計で抽選** する。

**メリット**  
- 既存の trainerproc の `Tags:` パーサがそのまま使える  
- 新しいタグ管理の仕組みを増やさない

**デメリット**  
- タグ名は `MON_POOL_TAG_*`（`include/trainer_pools.h`）で管理する必要がある  
- タグの自由度は増やすが、定数追加が必要

**変更箇所**  
- `include/trainer_pools.h`: `MON_POOL_TAG_WATER` など新規タグを追加  
- `dev_scripts/build_trainer_rank_parties.py`: rank pool .party の `Tags:` を読み、`TrainerRankMon` にタグ(bitmask)を持たせる  
- `src/randomizer.c`: 抽選時に tag フィルタ → 重み合計 → 抽選  

**記述例（複数タグは `/` 区切り、モードは `OR:` / `AND:` を先頭に付与）**  
`src/data/trainers.party`
```plain
RANK_C
Level: 5
Tags: OR:WATER/FIRE
```
`data/trainer_rank_party/rank_c.party`
```plain
=== RANK_C_SHARED ===
WeightTotal: 100

Lanturn
Level: 33
Weight: 10
Tags: WATER
```

---

### 案B: 専用フィールド（RankTags）を新設（自由タグ）
**概要**  
trainers.party に `RankTags: WATER/FAST` のような専用フィールドを追加。  
rank pool の .party も `Tags:` を自由文字列で管理し、  
`generated/trainer_rank_tags.h` 等でタグIDを自動生成する。

**メリット**  
- タグ名を自由に追加できる  
- `MON_POOL_TAG_*` の制約に縛られない

**デメリット**  
- 自動タグ管理/生成の実装が必要  
- 変更範囲が大きい

**変更箇所**  
- `tools/trainerproc/main.c`: `RankTags:` のパース追加  
- `dev_scripts/build_trainer_rank_parties.py`: タグ辞書を生成し、rank pool の `Tags:` と突合  
- `generated/trainer_rank_tags.h`: タグIDの列挙とマップを出力  
- `src/randomizer.c`: タグIDでフィルタ

---

## 抽選ロジック
- 入力: rank pool の候補リスト + 抽選タグ  
- **タグ一致の候補のみ残す**  
- **残った候補の重み合計を再計算** し、その合計から重み抽選  
- タグ一致が0件の場合の扱いは未確定（下記参照）

## 未確定事項（決める必要あり）
1) **タグ名のルール**  
   - 既存 `MON_POOL_TAG_*` を拡張するか  
   - 自由タグを許容するか

## まとめ
- 実装は **案A（既存 Tags 再利用）** で進める  
- タグは trainers.party / rank .party の双方で `Tags:` を使う（`Tags: OR:WATER/FIRE` のようにモード指定）  
- フィルタ後に重み合計を再計算して抽選する（重み無視はしない）  
- タグ一致が0件の場合は **ビルド時エラーを優先**。万一ランタイムで発生した場合は **そのスロットはバニラ固定** として扱う。  
- タグ名は **`MON_POOL_TAG_*` を拡張して管理**（`include/trainer_pools.h`）。必要に応じて `POOL_NUM_TAGS` を拡張して対応する。  
- **タグ付きの非ランク/非専用スロットはビルドエラー**（タグは無効扱いにせず、誤設定を止める）。  
- **LEAD/ACE タグも抽選フィルタに利用可**（ただしプールの「先頭/最後」選出ロジックにも影響する点に注意）。  
- **LEAD/ACE の上限運用**: 既存（元定義）で LEAD/ACE がある場合はそれを優先し、ランダム抽選で入ってきた LEAD/ACE は **上限（既存分が埋まっている場合）なら除外**。上限に達していなければ許可。  

## 懸念点 / 注意点
- `Tags: OR:...` / `Tags: AND:...` の `:` を既存パーサが許容していないため、trainerproc 側の `Tags` パース拡張が必要。  
- `data/trainer_rank_party/*.party` の `Tags:` を現在の生成スクリプトが無視しているため、rank pool モンにタグbitmaskを付与する実装が必要。  
- **タグ一致0件のビルドエラー化**を実現するには、`build_trainer_rank_parties.py` で trainers.party の各ポケモンスロットも解析する必要があり、実装範囲が大きい。  
- タグ上限は `POOL_NUM_TAGS` に依存（`TrainerMon.tags` は `u32`）。タグ追加時は上限拡張の設計を忘れない。  
- OR/AND の定義を明文化して固定する必要がある（OR=いずれか一致、AND=全一致）。  
- MultiRandom で増えるスロットは「最後のポケモンのタグ」も複製される。意図に合わない場合は別仕様が必要。  
- `POOL_TAG_LEAD/ACE` は pool 特別扱いがあるため、抽選フィルタに使うと **順序制御の副作用** が出る可能性がある。  
- タグを付けた **非ランク/非専用スロット** をどう扱うか（無視 or エラー）を明確化する必要がある。  
