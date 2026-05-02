# Trainer Battle Aftercare MVP Plan

## MVP Variants

| Variant | Behavior |
|---|---|
| A: Heal after trainer battle | 通常 trainer battle 終了後、win / loss 条件に応じて `HealPlayerParty` 相当を実行。 |
| B: No whiteout on trainer loss | `B_FLAG_NO_WHITEOUT` 相当の behavior で field へ戻し、必要なら heal。 |
| C: Forced release on wipeout | player defeat かつ `NoAliveMonsForPlayer()` の場合、対象 Pokemon を release してから field へ戻す。 |
| D: Selection-aware aftercare | battle selection 復元後、元 slot に反映された party に対して heal/release。 |

## Recommended Order

1. Heal after trainer battle only.
2. No-whiteout trainer loss with explicit heal.
3. Forced release helper without UI.
4. Battle selection restore と統合。

## Hook Design Candidate

未実装の設計候補:

1. `CB2_EndTrainerBattle` に入った時点で battle outcome を読む。
2. `HandleBattleVariantEndParty()` と将来の battle selection restore を先に完了させる。
3. defeated / no-whiteout / whiteout branch に入る前に aftercare policy を判定する。
4. release が必要なら専用 post-battle callback へ遷移する。
5. release 完了後に `HealPlayerParty` または survivor heal を行う。
6. `CB2_ReturnToFieldContinueScriptPlayMapMusic` へ戻すか、既存 `CB2_WhiteOut` に任せる。

## Release Helper Candidate

PC storage の `Task_ReleaseMon` は static UI task なので、将来は別 helper が必要。

候補 API:

| Helper | Purpose |
|---|---|
| `CanForceReleasePartyMon(slot)` | egg / last mon / protected story mon / battle selection 状態を判定。 |
| `ForceReleasePartyMon(slot, itemPolicy)` | held item policy に従って `ZeroMonData` 相当を行う。 |
| `ForceReleaseFaintedPartyMons(policy)` | 対象 slot を走査して release。 |
| `CompactPartySlots()` | 既存 public helper を利用候補。 |

## Policy Decisions

| Topic | Options |
|---|---|
| Trigger | trainer loss only、any battle loss、wipeout only、challenge flag enabled only |
| Release target | fainted mons、battle participants、selected mons、whole party |
| Heal target | survivors only、whole remaining party、no heal |
| Items | bag return、lost、PC release と同じ `OW_PC_RELEASE_ITEM` 準拠 |
| Last mon | release しない、run failure、starter replacement、game over |
| Boxes | party only、boxes included、future optional |
| Facilities | exclude Frontier / Hill / Pyramid / link、include later |

## Integration With Battle Selection

選出機能を同時に入れる場合の安全順:

1. battle 後の一時 `gPlayerParty` から元 slot へ選出個体状態を反映。
2. 元 6 匹 party を復元。
3. aftercare policy を適用。
4. `CalculatePlayerPartyCount()`。
5. field return / whiteout decision。

順序を逆にすると、一時 party の slot を release してから元 party を復元してしまい、release が消える可能性がある。

## Open Questions

- release 後に evolution / move learn pending がある場合の順序。
- victory 後 heal を全 trainer に適用するか、flag / var / trainer class で制御するか。
- release 演出を表示するか、message のみか、無演出か。
- `B_FLAG_NO_WHITEOUT` を使うか、独自 flag を作るか。
