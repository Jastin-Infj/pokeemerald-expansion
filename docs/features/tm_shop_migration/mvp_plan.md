# TM Shop Migration MVP Plan v15

調査日: 2026-05-02

この計画は実装案であり、現時点では未実装。

## MVP Scope

最初の MVP は、TM 定義や save flag 定義を削除せず、取得元を整理して shop へ寄せる方針が安全。

| Step | Proposed work | Code status |
|---|---|---|
| 1 | 全 TM の販売リストを docs で確定する。 | Not started |
| 2 | 既存 `pokemart` list に TM shop を追加するか、既存 shop list を拡張する。 | Not started |
| 3 | `FLAG_RECEIVED_TM_*` を削除せず、該当 NPC/gym scripts の TM 贈与を代替報酬または会話に変える。 | Not started |
| 4 | visible item ball の `map.json` entry を別 item に置換するか撤去する。 | Not started |
| 5 | hidden item の `bg_hidden_item_event` を別 item に置換するか撤去する。 | Not started |
| 6 | Game Corner / Trainer Hill / quiz prize / special script の扱いを個別決定する。 | Not started |

## Suggested First Cut

1. `FOREACH_TM` と `ITEM_TM*` は変更しない。
2. `include/constants/flags.h` の TM 関連 flag は削除しない。
3. TM 販売は既存 `pokemart` flow を使う。
4. field item ball / hidden item は一旦 docs inventory を作り、実装時に map 単位で置換する。
5. source code 変更前に `docs/features/tm_shop_migration/test_plan.md` の確認項目を build / runtime 手順へ落とす。

## Why This Order

- flag 定義削除は save compatibility と upstream merge の両方に影響する。
- `FOREACH_TM` は item / move / UI の中心なので、取得元整理のために触るべきではない。
- `pokemart` は `ScrCmd_pokemart` と `src/shop.c` の既存 UI があり、最初の販売導線として低リスク。

## Open Questions

- Medley Shop を既存 `pokemart` list として実装するか、動的 provider を C 側に追加するか未決定。
- 全 TM を序盤から販売するか、badge / story progress / price tier を使うか未決定。
- NPC/gym の代替報酬を item にするか、会話だけにするか未決定。
