# TM Shop Migration Test Plan v15

調査日: 2026-05-02
再確認日: 2026-05-16

この test plan は legacy Gen 3 TM acquisition retirement 実装後の確認用。
2026-05-16 の first cut では Emerald normal-progression の TM / HM item 入手導線を
退役し、TM を reusable 化した。build / static checks / mGBA boot まで確認対象。

## Static Checks

| Check | Purpose |
|---|---|
| `rg -n "FLAG_RECEIVED_TM_|FLAG_ITEM_.*TM_|FLAG_HIDDEN_ITEM_.*TM_" include data src` | 残存取得元と flag 参照を確認する。 |
| `rg -n "ITEM_TM_" data/maps data/scripts src include` | shop、gift、prize、field item の全 TM 参照を確認する。Normal progression audit では `data/scripts/debug.inc` と FRLG-specific maps を別扱いにする。 |
| `rg -n "giveitem .*ITEM_TM|additem .*ITEM_TM|checkitemspace .*ITEM_TM|bufferitemname .*ITEM_TM|setvar VAR_0x8004, ITEM_TM|\\.2byte ITEM_TM" data src` | TM reward / prize scripts が残っていないか確認する。Expected remaining refs: debug-only `TM Shop Test` and FRLG follow-up scope. |
| `rg -n "\"trainer_sight_or_berry_tree_id\": \"ITEM_TM_|\"item\": \"ITEM_TM_" data/maps -g map.json` | map.json owner 側に TM pickup / hidden item が残っていないか確認する。 |
| `rg -n "giveitem ITEM_HM_|finditem ITEM_HM_|checkitemspace ITEM_HM_|giveitem_msg .*ITEM_HM" data/maps data/scripts -g '!data/maps/**/events.inc'` | Emerald normal-progression に HM item grant が残っていないか確認する。FRLG-specific routes are follow-up scope. |
| `rg -n "ITEM_HM_|FOREACH_HM|CannotForgetMove|checkfieldmove" include data src` | HM core / field move / cannot-forget rule の意図しない削除がないか確認する。 |
| `rg -n "pokemart|pokemartlistend" data/maps data/scripts asm src` | shop list と script command 参照を確認する。 |
| `rg -n "Debug_EventScript_Script_1|TM Shop Test|Script 1" src/debug.c data/scripts/debug.inc` | Debug Script 1 に検証用 shop を置いた場合、debug-only route と label を確認する。 |
| `rg -n "FOREACH_TM|NUM_TECHNICAL_MACHINES|BAG_TMHM_COUNT|I_REUSABLE_TMS|GetItemImportance" include src test tools` | TM core definition と reusable policy を確認する。Expected: `I_REUSABLE_TMS TRUE`。 |
| `git diff --name-only master..HEAD` | master docs-only policy ではなく feature branch 実装として source / data 変更を明示確認する。 |

## Build Checks

| Check | Required when |
|---|---|
| `rtk make -j16 -O all` | shop / item / map data / script 変更を入れたら必須。 |
| `rtk make -j16 -O debug` | debug route、debug menu、mGBA validation route を使う場合。 |
| `rtk make -j16 -O check` | item / save / script / party teach flow に触れた場合。 |

## Runtime Checks

| Area | Expected result |
|---|---|
| Removed NPC rewards | reward 取得済み / 未取得どちらの save でも会話が詰まらず、TM を渡さず、legacy TM flag を set しない。 |
| Removed gym rewards | Gym leader post-battle / rematch-style branch が TM gift なしで自然に終わる。Badge / story flags は壊さない。 |
| Removed item balls | map に不要な TM item ball object が残らない、または代替 item / future hook として取得できる。 |
| Removed hidden items | Dowsing / hidden item interaction が該当座標で不自然に残らない、または代替 item として動く。 |
| Existing shops | Department Store、Slateport Power TMs、FRLG Celadon など existing TM shop stock が退役方針通り。 |
| Optional debug TM shop | If implemented, Debug menu `Scripts... > TM Shop Test` / `Script 1` opens a `pokemart` list using current `u16` item IDs and does not set legacy TM received flags. |
| Game Corner / facility prizes | Mauville Game Corner、Trainer Hill、Lilycove Lady が TM を出さない。 |
| Removed HM item grants | Rustboro Cut, Granite Cave Flash, Mauville Rock Smash, Rusturf Strength, Petalburg Surf, Route119 Fly, Mossdeep Dive, Sootopolis Waterfall が HM item を渡さず、会話と story flag が詰まらない。 |
| TM/HM core mostly untouched | Bag TM/HM UI、teach flow、standard relearner、HM field move が意図せず壊れていない。Expected intentional change: TMs are reusable. |

## Save Compatibility Checks

| Scenario | Expected result |
|---|---|
| New game | Legacy TM acquisition routes が初期状態から残っていない。 |
| Existing save with `FLAG_RECEIVED_TM_*` unset | NPC/gym scripts が意図通り動き、TM gift path に入らない。 |
| Existing save with `FLAG_RECEIVED_TM_*` set | 会話分岐が壊れない。Retired flag bit が set 済みでも新 content の条件に使わない。 |
| Existing save with item ball flags set | field object の hide state が不自然に戻らない。 |
| Existing save holding a TM | TM/HM pocket の表示・使用が意図せず壊れない。 |
| Existing save holding a HM | HM item を既に持っている save は従来通り表示・使用できる。新規通常進行では HM item を渡さない。 |

## mGBA Live Focus Route

最初の runtime validation は次の最小 route でよい。

1. Debug ROM を起動し、退役させた NPC / gym TM reward の map へ入る。
2. 対象 NPC と会話し、TM を受け取らず、会話が詰まらないことを確認する。
3. 退役させた visible TM item ball の map へ入り、object が消えているか代替 item /
   future hook として動くことを確認する。
4. 退役させた hidden TM item の座標で、不自然な hidden TM 入手が残らないことを
   確認する。
5. Optional debug hook を実装した場合、Debug menu `Scripts... > TM Shop Test` /
   `Script 1` から shop が開き、商品 ID が壊れず、legacy TM flag が set されない
   ことを確認する。
6. Existing TM shop / Game Corner / facility prize を 1 つ確認し、TM が出ないことを
   確認する。
7. Bag / TM/HM pocket と HM field move が意図せず壊れていないことを軽く確認する。
8. Rustboro Cutter or another HM source NPC / story event を確認し、HM item を受け取らず、
   会話が詰まらないことを確認する。
9. mGBA Live session を stop し、stale session があればこの doc に残す。

## Open Questions

- 自動 test に落とせる範囲は未確認。
- mGBA runtime の最初の確認 map をどこに置くか未決定。
- Debug Script 1 の TM shop test は実装済み。ただし mGBA Live では title から入力が
  進まず、shop 画面までは未確認。
- 退役後の reward を no reward にするか replacement item にするかは source ごとに未決定。
- Retired flag values を `FLAG_UNUSED_0x...` へ hard-rename するタイミングは、参照が
  完全に消えた後に限定する。

## 2026-05-16 Validation Results

| Check | Result |
|---|---|
| `rtk git diff --check` | Passed |
| `rtk mdbook build docs` | Passed with existing warnings: missing root `CHANGELOG.md` include, existing `CREDITS.md` `</img>`, large search index |
| Edited `map.json` parse check | Passed with `python3 -m json.tool` over the edited owner files |
| `rtk make -j16 -O all` | Passed with existing RWX linker warning |
| `rtk make -j16 -O debug` | Passed with existing RWX linker warning |
| `rtk make -j16 -O check` | Passed |
| `I_REUSABLE_TMS` static check | Passed. `include/config/item.h` defines `I_REUSABLE_TMS TRUE`. |
| Emerald HM grant static check | Passed for Emerald. Remaining direct HM grants are FRLG-specific follow-up scope. |
| Old HM receive flag static check | Passed. No old HM receive flag refs remain in `include`, `data`, `src`, or `docs`; old values are `FLAG_UNUSED_0x...`. |
| mGBA Live boot | Follow-up run reached Pokemon Emerald title screen, continued into an existing save, and opened the in-game Start menu. |
| mGBA Live feature screen | Rustboro Cutter / HM source NPC behavior was not confirmed because the loaded save was not positioned on that route. No new mGBA route was run after the old HM flag rename; static search plus rebuilds covered that follow-up. |
| mGBA cleanup | Follow-up session `tm-shop-migration-hm-followup` stopped cleanly with `stopped: true`. An older `[mgba-qt] <defunct>` entry from the previous validation remains visible in `pgrep`. |
