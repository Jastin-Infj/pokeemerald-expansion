# TM Shop Migration MVP Plan v15

調査日: 2026-05-02
再確認日: 2026-05-16

この計画は実装案として作成したもの。2026-05-16 の first cut では、Emerald
normal-progression の legacy TM acquisition retirement と HM item grant
retirement を実装済み。

2026-05-16 の再確認では、first cut は「current 50 TM の入手導線を退役させる」
ことに絞っていた。その後の follow-up で `I_REUSABLE_TMS TRUE` と Emerald HM
item grant retirement も同じ branch に含めた。Friendly Shop 化、Gen 9 200+
physical TM、full HM modernization はこの MVP から外す。

## MVP Scope

最初の MVP は、TM 定義や save flag value を壊さず、取得元を整理して legacy TM
flag を unused-like に戻せる状態へ寄せる方針が安全。

| Step | Proposed work | Code status |
|---|---|---|
| 1 | Legacy TM acquisition inventory を確定する: NPC/gym, field item, hidden item, existing shop, facility prize, Game Corner, Secret Power。 | Done for Emerald |
| 2 | `FLAG_RECEIVED_TM_*` / `FLAG_GOT_TM_*` reads and writes を scripts から外す。 | Done for Emerald |
| 3 | NPC/gym TM reward を no reward / replacement item / virtual unlock placeholder / shop hint のどれかへ置換し、text も合わせて直す。 | Done for Emerald |
| 4 | Visible TM item ball の `map.json` entry を撤去 / replacement / future random hook に分類して変更する。 | Done for Emerald visible TM balls |
| 5 | Hidden TM item の `bg_hidden_item_event` を撤去 / replacement / future random hook に分類して変更する。 | Done for Route 113 hidden Double Team |
| 6 | Existing TM shops and prizes: Lilycove, Slateport, Mauville Game Corner, Trainer Hill, Lilycove Lady, FRLG shop references を退役させる。 | Done for Emerald; FRLG remains follow-up |
| 7 | 参照が消えた後、legacy TM flags を同じ value の `FLAG_UNUSED_0x...` 名へ戻す。 | Done for Emerald legacy TM flags |
| 8 | Static search, build, and mGBA route で legacy TM acquisition が残っていないことを確認する。 | Static/build passed; mGBA boot passed, feature screen not confirmed |
| 9 | `I_REUSABLE_TMS` を `TRUE` にして既存 TM を無制限化する。 | Done |
| 10 | Emerald normal-progression の HM item grants を撤去し、story flags only にする。 | Done for Emerald; FRLG remains follow-up |

## Suggested First Cut

1. `FOREACH_TM` と `ITEM_TM*` は変更しない。
2. `include/constants/flags.h` の TM 関連 flag は削除しない。
3. `I_REUSABLE_TMS` は `TRUE` にする。既存 physical TM を入手した場合は無制限使用。
4. 通常進行用の TM shop は新規追加しない。既存 TM shop / prize は退役対象として扱う。
5. Debug menu `Scripts... > Script 1` に検証用 TM shop を置く場合だけ、
   current `pokemart` / `u16` item list を使う。通常進行の shop 導線としては
   扱わない。
6. Full HM modernization は同時にやらない。Emerald の HM item grant は撤去し、
   field-move unlock は別 feature / story flag 側へ寄せる。
7. field item ball / hidden item は map 単位で置換する。generated `events.inc`
   ではなく `map.json` を編集する。
8. NPC/gym gift は `giveitem` だけでなく text と flag gate を合わせて調整する。
9. source code 変更前に `docs/features/tm_shop_migration/test_plan.md` の確認項目を
   build / runtime 手順へ落とす。First cut 後の実績は `implementation.md` と
   `test_plan.md` に記録済み。

## Why This Order

- flag 定義削除は save compatibility と upstream merge の両方に影響する。
- `FOREACH_TM` は item / move / UI の中心なので、取得元整理のために触るべきではない。
- Debug-only shop を使う場合、`pokemart` は `u16` item list と既存 shop UI を通るため
  検証 route として低リスク。ただし通常進行の TM 販売導線ではない。
- 200+ physical TM は item ID、bag save layout、UI count の問題が先に来るため、
  legacy acquisition retirement と同時に扱うべきではない。
- Unified Move Relearner は broad TM compatibility を virtual candidate pool で
  扱えるため、古い Gen 3 TM item を維持する理由が弱い。
- HM item grant を消しても core HM move / field-move logic は残る。旧 HM receive
  flags は unused に戻し、必要な story 分岐は既存 story var / flag へ寄せる。

## Proposed Implementation Order

1. Rewrite one isolated NPC TM gift and text as a pattern, then build and mGBA
   verify that the conversation no longer grants a TM or sets a TM flag.
2. Optional: rename Debug menu `"Script 1"` to a TM shop test route and add a
   small `pokemart` list in `data/scripts/debug.inc` for local validation only.
3. Apply the same pattern to gym leader TM reward scripts.
4. Replace / remove visible TM item balls from `map.json`, then verify one map
   object route in mGBA.
5. Replace / remove the Route 113 hidden TM entry.
6. Migrate nonstandard sources: Game Corner, Trainer Hill, Lilycove Lady,
   Secret Power, existing Department Store / Slateport / FRLG TM shops.
7. Run full static search for `ITEM_TM_`, `FLAG_RECEIVED_TM_`,
   `FLAG_ITEM_.*TM_`, and `FLAG_HIDDEN_ITEM_.*TM_` under `data/` and `src/`.
8. Rename now-unreferenced Emerald legacy TM flags to `FLAG_UNUSED_0x...`
   names with unchanged values.
9. Enable reusable TM behavior with `I_REUSABLE_TMS TRUE`.
10. Remove Emerald `ITEM_HM_*` grants from normal progression scripts and
    update player-facing text so no NPC claims to hand out an HM.

## Open Questions

- NPC/gym の代替報酬を item にするか、会話だけにするか未決定。
- Gym / NPC reward を Unified Move Relearner の virtual TM unlock に変える場合、
  runtime branch でその unlock state が既に使えるか、後続 feature まで placeholder
  text にするか未決定。
- Static TM pickup を future random item hook にする場合、今回の feature で hook だけ
  作るか、単純撤去 / replacement に留めるか未決定。
- Retired TM flag values をいつ新規 flag pool へ戻すかは未決定。既存 save で set 済み
  になり得るため、即時再利用は避ける。
- Debug Script 1 shop は `TM Shop Test` として実装済み。通常進行の入手導線には
  数えず、runtime 検証用 hook として扱う。
