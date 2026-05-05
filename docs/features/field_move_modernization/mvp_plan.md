# Field Move Modernization MVP Plan

Status: Planned (per-HM scope decided)
Code status: No code changes
最終更新: 2026-05-05

## Per-HM Decision Table

`src/field_move.c:11-64` の各 `IsFieldMoveUnlocked_*` を確認し、HM ごとの badge と modernize MVP 範囲を確定した。

| HM | Move | Unlock flag (Emerald) | Unlock flag (FRLG) | Map obstacle | MVP slice | 後段 phase | Notes |
|---|---|---|---|---|---|---|---|
| HM01 Cut | `MOVE_CUT` | `FLAG_BADGE01_GET` | `FLAG_BADGE02_GET` | Cut Tree (`EventScript_CutTreeDown`) | **Slice 1**: 技所持 check 撤去、badge flag は残す | — | 最小スコープ。`removeobject VAR_LAST_TALKED` の挙動は維持 |
| HM06 Rock Smash | `MOVE_ROCK_SMASH` | `FLAG_BADGE03_GET` | `FLAG_BADGE06_GET` | Rock (`EventScript_SmashRock`) + `RockSmashWildEncounter` | Slice 2 | — | wild encounter 起動部は別挙動として残すか option 化 |
| HM04 Strength | `MOVE_STRENGTH` | `FLAG_BADGE04_GET` | `FLAG_BADGE04_GET` | Strength Boulder + `FLAG_SYS_USE_STRENGTH` | Slice 3 | — | session flag を auto-set にできるか確認 |
| HM05 Flash | `MOVE_FLASH` | `FLAG_BADGE02_GET` | `FLAG_BADGE01_GET` | 暗洞 brightness (`FLAG_SYS_USE_FLASH`) | Slice 4 | — | `Overworld_ResetStateAfterWhiteOut` で flag clear される挙動 |
| HM03 Surf | `MOVE_SURF` | `FLAG_BADGE05_GET` | `FLAG_BADGE05_GET` | 水タイル transition | — | **Phase 2** | avatar state、follower flag、Surf blob graphics への影響大 |
| HM07 Waterfall | `MOVE_WATERFALL` | `FLAG_BADGE08_GET` | `FLAG_BADGE07_GET` | 滝タイル transition | — | Phase 2 | avatar state、map type 判定 |
| HM08 Dive | `MOVE_DIVE` | `FLAG_BADGE07_GET` | n/a (Emerald only) | 海面 ↔ 水中 map swap | — | Phase 3 | 海底 map 切替の callback 影響 |
| HM02 Fly | `MOVE_FLY` | `FLAG_BADGE06_GET` | `FLAG_BADGE03_GET` | UI のみ (region map) | — | Phase 3 | `docs/flows/map_registration_fly_region_flow_v15.md` と連動 |

**Slice 順序の根拠**:

- Slice 1-4 (Cut / Rock Smash / Strength / Flash) は object interaction 系。`EventScript_*` と `checkfieldmove` の path 変更だけで完結する。avatar state を触らない。
- Phase 2 (Surf / Waterfall) は avatar state + follower + Surf blob graphics + map type の複合。Slice 1-4 の helper / pattern が固まった後でなければ regress しやすい。
- Phase 3 (Dive / Fly) は map 切替 / region map UI の追加 surface を触る。region map 拡張 (`docs/flows/map_registration_fly_region_flow_v15.md`) と同じ branch で進めるのが効率的。

## Out-of-Scope (modernize しない field move)

`enum FieldMove` には HM 以外も含まれる。これらは modernize の対象外:

| Field move | 性格 | 扱い |
|---|---|---|
| `FIELD_MOVE_TELEPORT` | move 所持 + map type | 既存挙動を維持 |
| `FIELD_MOVE_DIG` | move 所持 + map type | 既存挙動を維持 |
| `FIELD_MOVE_SECRET_POWER` | move 所持 + secret base | 別 feature (secret base) と連動するため触らない |
| `FIELD_MOVE_MILK_DRINK` / `SOFT_BOILED` | party heal | 触らない |
| `FIELD_MOVE_SWEET_SCENT` | wild encounter trigger | `no_random_encounters` 側で扱うか別 mode |

## Principles

- まず技所持依存を外す。map design と animation の全面変更は後回し。
- `FIELD_MOVE_*` table をいきなり削除しない。
- Dig / Teleport / Secret Power / Soft-Boiled など、HM ではない field utility を巻き込まない。
- `VAR_RESULT` が party slot を返す前提を壊す場合は、animation 側の代替も同時に用意する。

## MVP Stages

### Stage 0: Policy Lock

決めること:

| Topic | Candidate |
|---|---|
| Unlock source | badge flag、story flag、key item、global config |
| Animation | 既存 Pokemon show-mon を残す、または ride/key item animation へ置換 |
| Party menu | HM field action を非表示にする、または key item UI へ移す |
| HM moves | 忘却不可を維持する、通常技化する、HM item 自体を消す |
| Obstacles | map から撤去、自動処理、unlock 後 A ボタン処理 |

### Stage 1: Central Capability Check

将来実装候補:

- `CanUseFieldMoveModern(fieldMove)` のような共通 helper。
- `MonKnowsMove` を使う path と、badge/key item unlock だけを使う path を明示分離。
- `FIELD_MOVE_SURF` など水上移動は follower flag / metatile / map type check を残す。

危険な避け方:

- `ScrCmd_checkfieldmove` を単純に常に成功させると、`VAR_RESULT` が party slot でなくなり `FldEff_FieldMoveShowMonInit` が壊れる。

### Stage 2: Script Path

候補:

| Path | Notes |
|---|---|
| 既存 `checkfieldmove` を拡張 | 影響範囲は小さく見えるが既存 script 全体に効く。 |
| 新 macro / 新 command | 古い挙動を残せる。map script 差し替え量は増える。 |
| 新 special | C 側 helper と script 変数を柔軟に扱える。 |

最初は Cut / Rock Smash / Strength の object interaction だけを対象にする方が安全。Surf / Waterfall / Dive は avatar state / warp / follower への影響が大きいため後段に分ける。

### Stage 3: Party Menu Path

現行は `SetPartyMonFieldSelectionActions` が「選択 Pokemon の技」を見て action を出す。技所持不要にするなら以下のどれかが必要。

| Option | Notes |
|---|---|
| field move action を非表示 | Gen7/Gen8 風に最も近い。A ボタンや key item 入口へ寄せる。 |
| party menu からも unlock だけで使える | Pokemon を選ぶ意味が薄いが、show-mon animation の party slot を選べる。 |
| dedicated ride/key item UI | 将来向き。MVP では重い。 |

### Stage 4: Map Object Pass

`EventScript_CutTree`, `EventScript_RockSmash`, `EventScript_StrengthBoulder` を参照する map を全確認する。

作業観点:

- story progression に必要な障害物か。
- item route / optional shortcut か。
- NPC movement / object hide flag と連動しているか。
- `removeobject` 後に再出現すべきか。
- Rock Smash wild encounter を残すか。

### Stage 5: Forget / Release / Catch Swap

HM move を field requirement から外した後、以下を見直す。

- `P_CAN_FORGET_HIDDEN_MOVE`
- `CannotForgetMove`
- Move Deleter scripts
- `IsLastMonThatKnowsSurf`
- `sRestrictedReleaseMoves`
- `B_CATCH_SWAP_CHECK_HMS`

## Suggested First Implementation Slice

未実装の提案:

1. Cut tree だけを対象にする。
2. `EventScript_CutTree` の visible behavior を維持する。
3. party move 所持ではなく unlock flag で許可する。
4. show-mon animation を残す場合は、先頭 non-egg party mon など display mon の決定規則を明文化する。
5. その後 Rock Smash、Strength、Surf の順に広げる。

## Open Questions

- field move 表示用 Pokemon を「先頭」「選択」「ride partner」「表示なし」のどれにするか。
- Cut / Rock Smash obstacle を全撤去する場合、map JSON 編集をどの単位で行うか。
- HM item を完全削除するか、店売り/通常技 machine として残すか。
