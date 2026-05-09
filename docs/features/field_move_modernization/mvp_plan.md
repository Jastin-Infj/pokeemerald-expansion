# Field Move Modernization MVP Plan

Status: Initial MVP implemented
Code status: Runtime slice on `feature/field-move-modernization-mvp`
最終更新: 2026-05-09

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

## Implemented MVP Policy

2026-05-09 の初期実装では、以下を runtime 方針として採用した。

| Topic | Decision |
|---|---|
| Unlock source | HM field move は既存 badge flag を維持し、`MonKnowsMove` は要求しない。 |
| Script return | 既存互換のため `VAR_RESULT` は party slot 形を維持する。 |
| Animation | Pokemon show-mon banner / 黒背景 cut-in は `OW_FIELD_MOVE_SHOW_MON_EFFECT` で無効化する。 |
| Party menu | 現行の move-owned action を残す。key item / ride UI は後段。 |
| HM moves | 忘却不可、Move Deleter、release、catch-swap policy はこの slice では未変更。 |
| Obstacles | Cut tree / Rock Smash rock / Strength boulder の map object と removal/session flag は維持。 |
| Success messages | Cut / Rock Smash / Strength / Surf / Waterfall は成功時 prompt / 使用メッセージを省略する。Dive / Surface は誤操作防止の確認を残す。 |
| Dive controls | Dive down は A button、underwater Surface は B button。party menu 入口は Dive / Surface 両対応。 |
| Flash | unlock 済みなら cave map load 時に `FLAG_SYS_USE_FLASH` を自動 set する。manual Flash animation は map load 競合を避けるため今回は起動しない。 |

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
- badge-only から story flag / key item / field toolkit power-up へ移行できる unlock source layer。
- 長期推奨は単一 key item + capability flags。key item は UI / story 上の所持物、実際の使用可否は field move ごとの flag で見る。

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

初期実装済み:

1. `checkfieldmove` の HM path を badge unlock 化する。
2. `EventScript_CutTree` / Rock Smash / Strength / Surf / Waterfall / Dive の visible behavior を維持する。
3. party move 所持ではなく unlock flag で許可する。
4. show-mon banner は表示しない。待機 task は active list 即解除で進める。
5. map object removal / Rock Smash encounter / Strength session flag は既存挙動を維持する。

## Open Questions

- field move 表示用 Pokemon を「先頭」「選択」「ride partner」「表示なし」のどれにするか。
- Cut / Rock Smash obstacle を全撤去する場合、map JSON 編集をどの単位で行うか。
- HM item を完全削除するか、店売り/通常技 machine として残すか。
- HM 解禁を per-HM key item にするか、単一 field toolkit / power-up item に集約するか。
- Key Items pocket は現状 `BAG_KEYITEMS_COUNT 30` なので、key item 方式の前に bag capacity / save layout / debug grant の改修が必要。bag 拡張は別 feature の大型改修として分離する。

## Recommended Unlock Direction

Hidden Machine という概念は長期的には field progression の所持条件から外す。badge-only は暫定実装として安全だが、ジムバッジ以外の story event や任意順進行で解禁しづらい。per-HM key item は直感的だが Key Items pocket と item list を増やしすぎる。

推奨方針:

- 単一 key item を player に持たせる。
- key item の内部 upgrade / capability flags として Cut / Surf / Dive などを解禁する。
- field move 判定は badge ではなく capability flags を見る。
- badge は必要なら capability flag を立てる story trigger の一つとして扱う。
- bag 拡張は別 feature の大型改修で扱い、この feature は単一 key item 以上の bag pressure を前提にしない。
