# TM Shop Migration Risks v15

調査日: 2026-05-02
再確認日: 2026-05-16

## High Risks

| Risk | Details | Safer approach |
|---|---|---|
| Removing TM core definitions | `FOREACH_TM`、`ITEM_TM*`、`gTMHMItemMoveIds` は TM/HM pocket、teachability、item menu、party menu に直結する。 | 取得元だけを整理し、core TM definitions は維持する。 |
| Removing or re-numbering flags | `include/constants/flags.h` は save block の flag bitset と upstream diff に関係する。`FLAG_RECEIVED_TM_*` と `FLAG_ITEM_*_TM_*` は用途が違う。 | まず constants は残し、script / map data 側の取得元を変更する。参照が消えた後も save 互換方針が決まるまで番号詰めしない。 |
| Reusing retired TM flag bits | Existing saves may already have legacy TM received / item flags set. Reusing those bits for new story flags can start new content in the wrong state. | Rename to `FLAG_UNUSED_0x...` only after references are gone, but do not immediately reuse those IDs. Allocate new gameplay flags separately. |
| Editing generated `events.inc` directly | `data/maps/*/events.inc` は `mapjson` 生成物。 | `map.json` か `scripts.inc` を編集対象にする。 |
| Confusing item ball flag and received flag | `FLAG_ITEM_*_TM_*` は object hide、`FLAG_RECEIVED_TM_*` は NPC/gym gift gate。 | 取得元 category ごとに変更する。 |
| FRLG/Emerald flag mismatch | `flags_frlg.h` では Emerald TM flags が `0` 定義の箇所がある。 | version branch / config 別に参照を確認する。 |
| Physical 200+ TM item model | `ITEM_TM51`..`ITEM_TM100` は予約済みだが、200+ TM を物理 item として持たせるには item ID、`BAG_TMHM_COUNT`、SaveBlock1、UI count、ROM header count が絡む。 | This feature does not implement physical 200+ TMs. Future Gen 9 reusable TM work must be a separate feature. |
| HM coupling while retiring TMs | HM は field move、badge unlock、party slot、cannot-forget rule、script animation と結合している。`src/data/items.h` では `.price = 0`、`.importance = 1`。 | Emerald の HM item grant は撤去するが、core HM move / field-move behavior は別 feature まで維持する。 |
| Reusable TM switch side effects | `I_REUSABLE_TMS TRUE` は teach 消費だけでなく shop sold-out、sell/toss、Fling、standard TM relearner consumption に影響する。 | This branch enables it intentionally. Validate bag use, shop/debug shop behavior, and relearner interaction before handoff. |
| Retired HM receive flags | Old HM receive flag values may already be set in existing saves, but the branch now treats them as unused. | Do not reuse these values for new gameplay without an explicit save migration decision. Field-move capability flags should be owned by the field-move feature. |

## Medium Risks

| Risk | Details | Safer approach |
|---|---|---|
| Bag capacity | `BAG_TMHM_COUNT 64` は現状 50 TM + 8 HM には足りるが、TM 追加時は capacity / save layout も見る。 | First cut では TM item count を増やさない。 |
| Existing shop text drift | Removing TM stock from Lilycove / Slateport / FRLG shop lists may leave NPC text or floor identity saying TMs are sold. | Shop stock と同じ commit で text / floor identity を確認する。 |
| Debug shop accidentally becoming progression | A Debug Script 1 TM shop is useful for validation, but it can confuse scope if treated as the replacement acquisition model. | Debug menu `Script 1` route に限定し、normal map scripts には置かない。 |
| Alternative rewards | Gym leader reward script を変える場合、badge / story flag / dialogue flow とセットで確認が必要。 | One isolated NPC pattern を先に作り、gym は badge / story flag を触らず展開する。 |
| Hidden item UX | hidden item を撤去すると Dowsing / treasure-related scripts の期待とずれる可能性がある。 | map ごとに no reward / replacement / future hook を明示してから編集する。 |
| Existing save behavior | 既存 save で TM gift flag / item ball hide flag が立っている場合、script を消しても flag は残る。通常は無害だが、flag reuse は危険。 | Retired values はすぐ再利用せず、既存 save with flags set を runtime 確認する。 |
| Text drift | TM を渡さなくしても text が「TM をあげる」と言うと UX が壊れる。 | script 変更と同じ commit で text を更新する。 |
| Existing TM shops / prizes | Lilycove, Slateport, Mauville Game Corner, Trainer Hill, Lilycove Lady は単純な `FLAG_RECEIVED_TM_*` ではない。 | search-driven inventory で個別に分類する。 |
| Random item future work | static TM balls を消すと、将来の random item / TM lottery design の pool definition が必要になる。 | 今回は field object を no reward / replacement / future random hook のどれにするか docs に残す。 |

## Current Blocking Decisions

| Decision | Why it matters |
|---|---|
| First cut target | Current 50 TM acquisition retirement, reusable TM behavior, and Emerald HM item grant retirement. No Gen 9 200+ physical item expansion or full HM modernization unless explicitly split in. |
| Flag retirement style | Soft-retire named constants with no refs, or hard-rename to `FLAG_UNUSED_0x...` after refs are gone. User preference is hard-rename eventually. |
| Old reward replacement | Gym / NPC / facility reward を消す、代替 item、virtual TM unlock placeholder、shop hint のどれにするか。 |
| Field item replacement | Visible / hidden TM pickups を完全撤去、代替 item、future random-item hook のどれにするか。 |
| Debug test shop | Debug menu `Script 1` に `TM Shop Test` を実装済み。current `pokemart` / `u16` item list に限定し、normal progression には含めない。 |
| Physical vs virtual ownership | 200+ TM は physical bag item にしない方針が現状の docs では安全。future Gen 9 TM revival must choose physical, virtual, or mixed ownership separately. |

## Open Questions

- Legacy TM acquisition を退役させた後、プレイヤーへどの文脈で説明するかは後続 story / unlock feature で決める。
- 既存 item ball を削除する場合、map 上の object count / local id 参照に副作用がないか map ごとに確認が必要。
- 既存 save で既に TM 入手済み flag が立っている場合の移行方針は未決定。
- Retired TM flag values can be renamed to `FLAG_UNUSED_0x...`, but when they become safe to reuse for new gameplay is still undecided.
- Unified Move Relearner の virtual TM unlock と、将来 Gen 9 reusable TM を復活させる場合の物理 / 仮想 ownership の関係は未決定。
- Field-move unlock 用の capability flag 名 / 値は field-move feature 側で決める。
