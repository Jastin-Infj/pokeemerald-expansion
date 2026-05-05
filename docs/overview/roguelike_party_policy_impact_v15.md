# Roguelike Party Policy Impact v15

調査日: 2026-05-04。現時点では実装なし。

## Purpose

Battle Factory / Pokemon Champions / roguelike 的な施設を作る場合に、party、持ち物、技、育成値、敗北後処理がどこへ波及するか整理する。

想定:

- 100 戦到達のような long-run facility。
- Pokemon を入れ替えたり逃がしたりしながら進む。
- 持ち物を簡単に変更できない、または一時ロックする。
- 性格、EV、IV、技を簡単に編集する UI を将来的に持つ。
- battle 中の消費 held item は battle 後に復元する方向。

## Related Docs

| Doc | Why |
|---|---|
| `docs/overview/runtime_rule_options_feasibility_v15.md` | Nuzlocke、release、difficulty、EXP/catch/shiny、gimmick on/off の option 化候補。 |
| `docs/overview/champions_training_ui_feasibility_v15.md` | EV / IV / nature / moveset editor。 |
| `docs/flows/battle_ai_decision_flow_v15.md` | NPC AI、交代、double battle。 |
| `docs/features/battle_item_restore_policy/` | battle 後 held item 復元。 |
| `docs/features/trainer_battle_aftercare/` | battle 後 heal / release / no-whiteout。 |
| `docs/features/battle_selection/` | trainer battle 前選出、party 一時保存。 |
| `docs/flows/party_menu_flow_v15.md` | party menu、held item give/take、choose half。 |
| `docs/flows/move_relearner_flow_v15.md` | 技思い出し / level 無視 move candidate。 |
| [`docs/overview/roguelike_npc_capacity_v15.md`](roguelike_npc_capacity_v15.md) | Rogue NPC 100〜300 人計画の object event / flag / save 予算と推奨実装方式。 |
| [`docs/overview/roguelike_runtime_party_v15.md`](roguelike_runtime_party_v15.md) | Rogue 用 runtime party 生成。`GetTrainerStructFromId` hook + `struct Trainer.poolSize` の活用、Frontier 経路との比較。 |
| [`docs/overview/roguelike_runtime_hooks_v15.md`](roguelike_runtime_hooks_v15.md) | Battle 終了 callback、wave 帯ごとの AI flag、`SeedRng` / `LocalRandom` の決定論的 RNG、SaveBlock3 への RogueSave 追加候補。各項目で「現時点未実装」を明示。 |

## Held Item Lock

Pokemon に item を持たせる/外す flow は複数ある。

| Entry | Important functions |
|---|---|
| Party menu field action | `CursorCb_Give`, `CursorCb_TakeItem`, `CursorCb_Toss`, `GiveItemToMon`, `TryTakeMonItem` |
| Bag -> Give | `CB2_ChooseMonToGiveItem`, `TryGiveItemOrMailToSelectedMon`, `GiveItemToSelectedMon` |
| Storage item mode | `TakeItemFromMon`, `GiveItemToMon` in `src/pokemon_storage_system.c` |
| Battle Pyramid item storage | `PARTY_MENU_TYPE_STORE_PYRAMID_HELD_ITEMS` |
| Battle item restore | `TryRestoreHeldItems`, `B_RESTORE_HELD_BATTLE_ITEMS` |

「一時的に持ち物変更をロックする」なら、単に `SetMonData(MON_DATA_HELD_ITEM)` を止めるだけでは足りない。UI 入口を止める必要がある。

候補:

| Option | Scope | Notes |
|---|---|---|
| facility flag で party menu の Give/Take/Toss を隠す | Field party menu | まずここから。 |
| bag の Give action を拒否 | Bag -> party | `CB2_ChooseMonToGiveItem` 周辺。 |
| storage の item mode を拒否 | PC box | box から item 移動できるとロックを迂回できる。 |
| battle entry validation で held item 変更を拒否 | Facility start | UI を抜けた変更も検出できる。 |

MVP は「facility 中は held item を変更する UI を開けない」方式が安全。変更前の held item を snapshot して、facility 中の比較に使うと迂回検出がしやすい。

## Item Clause and One Item Rule

対戦型ルールで「同じ held item は 1 個まで」を採用する場合、既存の validation がある。

`Task_ValidateChosenHalfParty` 周辺では、選出済み party の species duplicate と held item duplicate を見ている。Battle Frontier / Factory / Tent には既に duplicate held item rule が存在する。

注意:

- Bag quantity が無限でも、held item duplicate rule は別。
- battle 中の消費 item を復元する場合、復元後にも duplicate が成り立つよう、battle 開始時の状態を source of truth にする。
- rental / forced swap / release で item を戻すか失うかを決める。

## Training UI and Facility Rule

性格、EV、IV、技編集をどこまで施設中に許すかでゲーム性が変わる。

| Policy | Pros | Risks |
|---|---|---|
| facility 外でだけ編集 | ゲーム性を固定しやすい。 | 途中で入手した Pokemon の調整ができない。 |
| wave 間だけ編集 | Champions 風。 | UI と rule state が増える。 |
| currency / item 消費で編集 | roguelike 報酬と噛み合う。 | save data / shop / balance 設計が必要。 |
| 完全自由編集 | 検証や sandbox は楽。 | 100 戦施設の難易度が崩れやすい。 |

最初の設計では、facility state に次を持たせると後から拡張しやすい。

| State | Meaning |
|---|---|
| `allowTrainingEdit` | EV/IV/nature editing を許可するか。 |
| `allowMovesetEdit` | move editor / relearner を許可するか。 |
| `lockHeldItems` | held item 変更を禁止するか。 |
| `restoreBattleItems` | battle 後に held item を復元するか。 |
| `itemClause` | duplicate held item を禁止するか。 |
| `releaseOnLoss` | 敗北時に release / forced replacement を行うか。 |

これらは compile-time config だけでなく、facility runtime state に寄せる方が long-run mode と相性がよい。

## Party / Release / Replacement

100 戦型 facility で Pokemon を逃がす、入れ替える、失う処理は高リスク。

見るべき場所:

| Area | Files |
|---|---|
| battle end | `src/battle_setup.c`, `src/battle_main.c` |
| forced release | `docs/features/trainer_battle_aftercare/`, `src/pokemon_storage_system.c` |
| party compact | `CompactPartySlots` |
| held item return/loss | `TryTakeMonItem`, storage item helpers, item restore policy |
| HM / field move softlock | `sRestrictedReleaseMoves`, `B_CATCH_SWAP_CHECK_HMS`, field move docs |
| follower / selected mon | follower NPC docs, party menu callbacks |

release policy は先に決める。

| Policy | Notes |
|---|---|
| item を bag に返す | player に優しい。bag capacity を見る。 |
| item も失う | roguelike らしいが、rare item loss と復元 rule が衝突する。 |
| facility item pool に戻す | rental / factory 風。独自 inventory が必要。 |

## Nuzlocke Relationship

Nuzlocke を option として入れる場合、`releaseOnLoss` だけでは足りない。捕獲可能エリア、捕獲済みエリア、faint/death marker、dead box / forced release、last usable mon softlock、field move softlock をまとめて扱う必要がある。

最初は次のように分けるのが安全。

| Layer | Responsibility |
|---|---|
| Runtime rule | Nuzlocke enabled / disabled。 |
| Encounter rule | 現在 map / mapsec で捕獲済みか。 |
| Catch aftercare | 捕獲成功時に area consumed を記録。 |
| Battle aftercare | battle 後に fainted mon を dead / release / box lock に反映。 |
| UI rule | dead mon を party / box / summary / battle entry で使えないようにする。 |

強制 release を先に入れるより、dead marker 方式で UI から使用禁止にする方が検証しやすい。release は held item、party compact、follower、last live mon、HM softlock まで絡むため、最後に回す。

## Battle Factory Relationship

Battle Factory 風にする場合、既存 Frontier / Factory には rental / swap / level / item rule がある。

今回の Champions-style training UI とは方向が違うため、混ぜる場合は役割を分ける。

| Layer | Suggested responsibility |
|---|---|
| Factory/rental layer | どの Pokemon を使えるか、swap できるか。 |
| Training layer | 許可されたタイミングで EV/IV/nature/moves を調整。 |
| Item policy layer | held item lock、item clause、battle item restore。 |
| Aftercare layer | heal、release、run end、reward。 |
| AI layer | wave / trainer type ごとの AI flags / dynamic function。 |

## Implementation Order Candidate

1. docs-only で facility policy を固定する。
2. held item lock の UI 入口一覧をテスト項目化する。
3. battle item restore policy を先に決める。
4. training UI は field-only MVP で作り、facility 中は一旦無効にする。
5. moveset editor は move relearner flow を使うか新規 UI にするか決める。
6. rogue-like release / forced swap は最後に実装する。

## Open Questions

- facility 中に EV/IV/nature/moves を自由編集できるか。
- held item lock は party menu だけか、bag / storage も含むか。
- release 時に held item を返すか失うか。
- wild 6V と training UI を両方入れる場合、育成 UI の価値をどう残すか。
- AI は全 trainer を強くするか、facility wave だけ dynamic function を使うか。
