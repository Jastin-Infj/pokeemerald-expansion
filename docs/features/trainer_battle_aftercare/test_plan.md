# Trainer Battle Aftercare Test Plan

## Heal After Trainer Battle

- 通常 trainer battle に勝利後、HP / PP / status が回復するか。
- 通常 trainer battle に敗北後、whiteout しない mode で回復するか。
- `OW_PC_HEAL` の設定で boxes も回復するか、意図通りか。
- Tera Orb recharge が想定通りか。
- bad poison downgrade と競合しないか。

## No Whiteout

- `B_FLAG_NO_WHITEOUT` 相当の trainer loss で field script に戻るか。
- battle loss text が不自然でないか。
- `gSpecialVar_Result = gBattleOutcome` が script 側で読めるか。
- `CB2_WhiteOut` が走らないこと。
- loss 後に player controls / map music / script context が復帰すること。

## Forced Release

- 全滅時に対象 party slot が release されるか。
- release 後に `CompactPartySlots()` 相当で party が詰まるか。
- held item return / loss policy。
- last mon release policy。
- egg が対象外になるか。
- fainted participant only / all fainted / selected party only の各 policy。
- follower が release 対象だった場合の挙動。

## Battle Selection Integration

- 選出 3 匹で敗北し、元 6 匹 party に戻してから release されるか。
- 非選出 Pokemon が release 対象にならない policy の場合、正しく保護されるか。
- 選出個体の HP/status/PP 反映後に heal/release が適用されるか。

## Exclusion Tests

- Battle Frontier。
- Battle Tent。
- Trainer Hill。
- Battle Pyramid。
- link battle。
- secret base trainer。
- early rival。
- follower partner battle。

## Visual / Script Return Tests

- battle 終了後、map music が復帰するか。
- post-battle trainer script が継続するか。
- whiteout cutscene が誤って出ないか。
- release message を出す場合、window / palette / callback が壊れないか。
