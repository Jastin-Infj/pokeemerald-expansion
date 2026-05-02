# TM Shop Migration Test Plan v15

調査日: 2026-05-02

この test plan は将来実装後の確認用。現時点では未実行。

## Static Checks

| Check | Purpose |
|---|---|
| `rg -n "FLAG_RECEIVED_TM_|FLAG_ITEM_.*TM_|FLAG_HIDDEN_ITEM_.*TM_" include data src` | 残存取得元と flag 参照を確認する。 |
| `rg -n "ITEM_TM_" data/maps data/scripts src include` | shop、gift、prize、field item の全 TM 参照を確認する。 |
| `rg -n "pokemart|pokemartlistend" data/maps data/scripts asm src` | shop list と script command 参照を確認する。 |
| `rg -n "FOREACH_TM|NUM_TECHNICAL_MACHINES|BAG_TMHM_COUNT" include src test tools` | TM core definition への意図しない変更がないか確認する。 |

## Runtime Checks

| Area | Expected result |
|---|---|
| TM shop | 対象 TM が購入でき、bag の TM/HM pocket に入る。 |
| Bag TM/HM UI | TM 番号、move name、description、price display が崩れない。 |
| Teach flow | purchased TM を party menu から使える。習得可否と move replacement が動く。 |
| Removed NPC rewards | reward 取得済み / 未取得どちらの save でも会話が詰まらない。 |
| Removed item balls | map に不要な item ball object が残らない、または代替 item として取得できる。 |
| Removed hidden items | Dowsing / hidden item interaction が該当座標で不自然に残らない。 |
| Existing shops | Department Store、Slateport Power TMs、Game Corner など既存 TM source の扱いが設計通り。 |

## Save Compatibility Checks

| Scenario | Expected result |
|---|---|
| New game | TM shop progression が初期状態から正しく動く。 |
| Existing save with `FLAG_RECEIVED_TM_*` unset | NPC/gym scripts が意図通り動く。 |
| Existing save with `FLAG_RECEIVED_TM_*` set | 再入手や会話分岐が壊れない。 |
| Existing save with item ball flags set | field object の hide state が不自然に戻らない。 |

## Open Questions

- 自動 test に落とせる範囲は未確認。
- shop runtime の manual test map をどこに置くか未決定。
