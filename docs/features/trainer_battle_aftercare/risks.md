# Trainer Battle Aftercare Risks

## Very High Risk

| Risk | Why |
|---|---|
| Whiteout order | `DoWhiteOut` は `HealPlayerParty()` を先に呼ぶ。release を whiteout 後に回すと対象が曖昧になる。 |
| Battle selection interaction | 一時 `gPlayerParty` と元 party の復元順を間違えると、release/heal が消える。 |
| Static PC release code | `Task_ReleaseMon`, `ReleaseMon`, `PurgeMonOrBoxMon` は static / UI state 依存。battle end から直接呼べない。 |
| Last usable mon | 強制 release で party が空になると、field / menu / battle entry が壊れる可能性がある。 |
| Held items | release 時に item を返すか失うかで economy と UI が変わる。 |

## High Risk

| Risk | Why |
|---|---|
| `CB2_EndTrainerBattle` branch coverage | early rival、secret base、forfeit、Pyramid、Trainer Hill、follower partner など特殊 branch がある。 |
| `NoAliveMonsForPlayer` semantics | multi battle、Arena、egg、saved party 参照などがある。 |
| Evolution / learn move timing | battle 終了後の evolution flow と party mutation の順序を確認する必要がある。 |
| Bad poison downgrade | win path の `DowngradeBadPoison()` と loss path の扱い。 |
| Battle Frontier / facilities | 既存で `HealPlayerParty` を script から呼ぶ箇所が多い。通常 trainer battle と分離が必要。 |

## Medium Risk

| Risk | Why |
|---|---|
| Follower partner battles | `RestorePartyAfterFollowerNPCBattle` と optional heal が既にある。 |
| `B_FLAG_NO_WHITEOUT` reset | `Overworld_ResetBattleFlagsAndVars` が flag を clear する。 |
| PC heal config | `OW_PC_HEAL >= GEN_8` なら `HealPlayerParty` が boxes も回復。 |
| Tera Orb recharge | `HealPlayerParty` が Tera Orb flag も扱う。 |

## Mitigations

- 最初は heal-only を実装し、release は別 feature flag で分ける。
- `CB2_EndTrainerBattle` の分岐表を test plan に落としてから実装する。
- forced release は PC UI の static function を呼ばず、専用 helper を作る。
- battle selection restore 後に aftercare を行う。
- Battle Frontier / Trainer Hill / Pyramid / link は MVP では明示除外する。
- Champions partygen 管理 trainer は、challenge runtime が入るまでは通常
  trainer として扱う。将来 `ChampionsChallenge_IsActive()` が入ったら
  normal aftercare を bypass して challenge policy へ渡す。
- `CB2_EndTrainerBattle` へ長い条件式を直接散らさず、
  `TrainerBattleAftercare_ShouldApply()` のような helper に集約する。

## Cross-Feature Risks

| Risk | Severity | Notes |
|---|---|---|
| Champions partygen trainer が意図せず heal-only 対象になる | Medium | 現在の E4 / Wallace は通常 trainer battle として動く。config default off なら無影響。config on では通常 trainer と同じ扱いにするか、明示 exclude flag を追加する。 |
| 将来の Champions Challenge runtime と通常 aftercare が二重適用される | High | `ChampionsChallenge_IsActive()` を aftercare guard の先頭で見て、challenge policy に一本化する。 |
| Battle item restore と release の順序が混ざる | Medium | heal-only MVP では release しない。release phase では item restore policy と challenge loss policy を別 table にする。 |
| Battle selection restore 前に aftercare が走る | High | 選出 party の一時 slot に heal/release してから元 party を復元すると変更が消える。restore-first を contract にする。 |
