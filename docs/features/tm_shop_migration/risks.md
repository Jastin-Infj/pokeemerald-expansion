# TM Shop Migration Risks v15

調査日: 2026-05-02

## High Risks

| Risk | Details | Safer approach |
|---|---|---|
| Removing TM core definitions | `FOREACH_TM`、`ITEM_TM*`、`gTMHMItemMoveIds` は TM/HM pocket、teachability、item menu、party menu に直結する。 | 取得元だけを整理し、core TM definitions は維持する。 |
| Removing or re-numbering flags | `include/constants/flags.h` は save block の flag bitset と upstream diff に関係する。 | まず constants は残し、script / map data 側の取得元を変更する。 |
| Editing generated `events.inc` directly | `data/maps/*/events.inc` は `mapjson` 生成物。 | `map.json` か `scripts.inc` を編集対象にする。 |
| Confusing item ball flag and received flag | `FLAG_ITEM_*_TM_*` は object hide、`FLAG_RECEIVED_TM_*` は NPC/gym gift gate。 | 取得元 category ごとに変更する。 |
| FRLG/Emerald flag mismatch | `flags_frlg.h` では Emerald TM flags が `0` 定義の箇所がある。 | version branch / config 別に参照を確認する。 |

## Medium Risks

| Risk | Details |
|---|---|
| Bag capacity | `BAG_TMHM_COUNT 64` は現状 50 TM + 8 HM には足りるが、TM 追加時は capacity / save layout も見る。 |
| Shop list length | `src/shop.c` は item list を動的に build するが、UI 表示、scroll、price、money flow を確認する必要がある。 |
| Alternative rewards | Gym leader reward script を変える場合、badge / story flag / dialogue flow とセットで確認が必要。 |
| Hidden item UX | hidden item を撤去すると Dowsing / treasure-related scripts の期待とずれる可能性がある。 |

## Open Questions

- 全 TM を買えるようにした後、既存 TM を拾えないことをプレイヤーにどう伝えるか未決定。
- 既存 item ball を削除する場合、map 上の object count / local id 参照に副作用がないか map ごとに確認が必要。
- 既存 save で既に TM 入手済み flag が立っている場合の移行方針は未決定。
