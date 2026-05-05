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
| follower interactions | `isfollowerfieldmoveuser`, `EventScript_FollowerFieldMove`, `CheckFollowerNPCFlag`。 |
| Secret Power | `FIELD_MOVE_SECRET_POWER` は HM ではないが同じ field move table を使う。 |
| Rock Smash encounters | `RockSmashWildEncounter` は wild encounter randomizer と関係。 |
| Poke Rider | `OW_FLAG_POKE_RIDER` は Fly 系の別導線候補だが、初期値 0 で未設定。 |

## Specific Failure Modes

- `VAR_RESULT = PARTY_SIZE` のまま `setfieldeffectargument 0, VAR_RESULT` すると party out-of-range の危険がある。
- show-mon を残したまま HM 技所持だけ不要にすると、表示 Pokemon の選定規則が曖昧になる。
- Cut tree / Rock Smash object を消すと NPC movement や item access が変わる。
- Strength を常時有効にすると boulder puzzle の state reset と衝突する可能性がある。
- Surf を技不要にしても `PartyHasMonWithSurf()` が残ると水面 A ボタンでは使えない。
- `P_CAN_FORGET_HIDDEN_MOVE` だけ変更すると、field move softlock prevention と矛盾する可能性がある。

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
