# Field Move Modernization Risks

## High Risk

| Risk | Why |
|---|---|
| `ScrCmd_checkfieldmove` の戻り値前提 | `VAR_RESULT` は party slot。animation は `gPlayerParty[slot]` を読む。 |
| party menu と script の二重入口 | script 側だけ変えても party menu action は技所持依存のまま。 |
| field move animation | `SetPlayerAvatarFieldMove`, `FldEff_FieldMoveShowMonInit`, indoor/outdoor streaks、Surf blob などが絡む。 |
| map object removal | Cut / Rock Smash は `removeobject`、Strength は `FLAG_SYS_USE_STRENGTH`。map design と softlock に直結。 |
| Surf / Waterfall / Dive | avatar state、follower、warp、metatile check、music、surf blob が絡む。 |
| HM forget restrictions | `CannotForgetMove` は battle learn move、evolution、summary、Move Deleter に波及する。 |
| release / catch-swap softlock | `sRestrictedReleaseMoves` と `B_CATCH_SWAP_CHECK_HMS` が HM 前提の softlock 防止をしている。 |

## Medium Risk

| Risk | Why |
|---|---|
| Flash cave state | `FLAG_SYS_USE_FLASH` と `requires_flash` map。 |
| Flash auto-use timing | map load 中に effect / script を走らせると fade、callback、script context と競合しやすい。 |
| follower interactions | `isfollowerfieldmoveuser`, `EventScript_FollowerFieldMove`, `CheckFollowerNPCFlag`。 |
| Dive input mismatch | Dive down は A、Surface は B、yes/no 決定は A。無言化すると underwater で意図しない Surface warp が起きやすい。 |
| Secret Power | `FIELD_MOVE_SECRET_POWER` は HM ではないが同じ field move table を使う。 |
| Rock Smash encounters | `RockSmashWildEncounter` は wild encounter randomizer と関係。 |
| Poke Rider | `OW_FLAG_POKE_RIDER` は Fly 系の別導線候補だが、初期値 0 で未設定。 |
| Key item unlock capacity | `BAG_KEYITEMS_COUNT 30` の固定長 pocket。per-HM key item 追加は bag / save / debug grant に波及するため、bag 拡張は別 feature の大型改修として扱う。 |
| Field Kit migration | Existing saves with HM receipt flags but no Field Kit will fail modernized HM checks until the item is granted. First-time grant scripts avoid setting capability flags if the Field Kit cannot be added. |
| Item id churn | Key Item section 途中に挿入すると後続 item ids が大きくずれる。Field Kit は `ITEMS_COUNT` 直前に追加して churn を抑える。 |

## Specific Failure Modes

- `VAR_RESULT = PARTY_SIZE` のまま `setfieldeffectargument 0, VAR_RESULT` すると party out-of-range の危険がある。
- show-mon を残したまま HM 技所持だけ不要にすると、表示 Pokemon の選定規則が曖昧になる。
- show-mon banner を単純削除すると、`FLDEFF_FIELD_MOVE_SHOW_MON` を待つ Surf / Waterfall / Dive / Fly / common field move task が進まなくなる可能性がある。初期実装では init effect が active list を即解除し、待機側の条件を満たす形にしている。
- Cut tree / Rock Smash object を消すと NPC movement や item access が変わる。
- Strength を常時有効にすると boulder puzzle の state reset と衝突する可能性がある。
- Surf を技不要にしても `PartyHasMonWithSurf()` が残ると水面 A ボタンでは使えない。
- `P_CAN_FORGET_HIDDEN_MOVE` だけ変更すると、field move softlock prevention と矛盾する可能性がある。
- Flash を manual field effect として自動起動すると、map load 直後の palette fade / cave transition / script callback が重なって固まる可能性がある。初期実装では flag set のみに留める。

## Accepted Initial MVP Limits

- party menu action はまだ move-owned のまま。HM 技を忘れる policy と Fly 代替 UI が固まるまで、party menu を badge-only にしない。
- HM forget / Move Deleter / release / catch-swap は未変更。field progression は技所持不要になったが、HM move の通常技化は別 slice。
- `VAR_RESULT` は互換のため party slot を返す。show-mon が無効でも follower check と既存 script shape を維持するため。
- unlock source は Field Kit + capability flags に移行した。初期値では既存 badge gate も維持する。
- long-term unlock は badge-only より単一 field toolkit key item + capability flags を推奨。per-HM key item は bag pressure が高い。
- Field Kit itemization は初期値で badge gate も維持する。`OW_FIELD_MOVE_TOOLKIT_BADGES == FALSE` にすると progression が capability-only になり、Cut / Flash / Rock Smash などの使用タイミングが早まる可能性がある。

## Upstream Risks

upstream v15.x / v16 追従時は以下の変更を毎回確認する。

- `struct FieldMoveInfo`
- `gFieldMoveInfo`
- `ScrCmd_checkfieldmove`
- `CursorCb_FieldMove`
- `SetPartyMonFieldSelectionActions`
- `FldEff_FieldMoveShowMonInit`
- `CreateFieldMoveTask`
- `SetPlayerAvatarFieldMove`
- `CannotForgetMove`
- `sRestrictedReleaseMoves`
- `B_CATCH_SWAP_CHECK_HMS`
