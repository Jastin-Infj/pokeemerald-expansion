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
