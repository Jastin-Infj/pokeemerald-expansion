# Trainer Battle Aftercare Investigation

調査日: 2026-05-02

## Confirmed Battle End Flow

| File | Facts |
|---|---|
| `src/battle_setup.c` | `BattleSetup_StartTrainerBattle` が `gMain.savedCallback = CB2_EndTrainerBattle` を設定。 |
| `src/battle_main.c` | `ReturnFromBattleToOverworld` が `gSpecialVar_Result = gBattleOutcome`、`gMain.inBattle = FALSE`、`SetMainCallback2(gMain.savedCallback)`。 |
| `src/battle_controller_player.c` | player controller end path でも `SetMainCallback2(gMain.savedCallback)` を使う。 |
| `src/battle_setup.c` | `CB2_EndTrainerBattle` が trainer battle 後の field return / whiteout / flag set を決める。 |

## `CB2_EndTrainerBattle` Details

`src/battle_setup.c` の `CB2_EndTrainerBattle` で確認した分岐:

1. `HandleBattleVariantEndParty()`
2. follower battle partner の party restore と optional heal
3. `TRAINER_BATTLE_EARLY_RIVAL`
4. `TRAINER_SECRET_BASE`
5. `DidPlayerForfeitNormalTrainerBattle()`
6. `IsPlayerDefeated(gBattleOutcome)`
7. win path

重要 symbols:

- `HandleBattleVariantEndParty`
- `SaveChangesToPlayerParty`
- `LoadPlayerParty`
- `B_FLAG_SKY_BATTLE`
- `FNPC_FLAG_HEAL_AFTER_FOLLOWER_BATTLE`
- `HealPlayerParty`
- `B_FLAG_NO_WHITEOUT`
- `CurrentBattlePyramidLocation`
- `InTrainerHillChallenge`
- `NoAliveMonsForPlayer`
- `CB2_WhiteOut`
- `CB2_ReturnToFieldContinueScriptPlayMapMusic`
- `DowngradeBadPoison`
- `RegisterTrainerInMatchCall`
- `SetBattledTrainersFlags`

## No Whiteout

`include/config/battle.h`:

- `B_FLAG_NO_WHITEOUT` は初期値 `0`。
- comment で、flag が set されていると trainer battle で whiteout できないが、party は自動回復されないと明記されている。

`src/battle_script_commands.c`:

- `BS_JumpIfNoWhiteOut` は `FlagGet(B_FLAG_NO_WHITEOUT)` を見る。
- `NoAliveMonsForPlayer` は `gPlayerParty` の HP を集計し、タマゴや Arena などの特殊条件も見る。

`src/overworld.c`:

- `DoWhiteOut` は `RunScriptImmediately(EventScript_WhiteOut)`、`HealPlayerParty()`、`Overworld_ResetStateAfterWhiteOut()`、last heal location warp を実行する。
- `Overworld_ResetBattleFlagsAndVars` は `B_FLAG_NO_WHITEOUT` を clear する。

外部参考:

- `PokemonSanFran/pokeemerald` の No Whiteout wiki は、loss 後に field へ戻すと party は倒れたままなので必要なら `special HealPlayerParty` を使う、という注意点を示している。

## Heal Flow

| File | Facts |
|---|---|
| `src/script_pokemon_util.c` | `HealPlayerParty` は `gPlayerPartyCount` 分 `HealPokemon(&gPlayerParty[i])` を呼ぶ。`OW_PC_HEAL >= GEN_8` なら boxes も回復。Tera Orb recharge もここ。 |
| `data/specials.inc` | `def_special HealPlayerParty`。 |
| `data/scripts/pkmn_center_nurse.inc` | Pokemon Center heal script が `special HealPlayerParty`。 |
| `src/battle_setup.c` | follower battle partner / early rival で `HealPlayerParty()` を使う既存例あり。 |

## Existing Subset Party Restore

`src/battle_setup.c` の Sky Battle path は、battle selection との関係で重要。

- `HandleBattleVariantEndParty` は `B_FLAG_SKY_BATTLE` が set されている時だけ実行。
- `SaveChangesToPlayerParty` は `B_VAR_SKY_BATTLE` bitfield を使い、battle に参加した一時 party の mon を saved party の元 slot へ戻す。
- その後 `LoadPlayerParty` で元 party 全体を復元する。

これは「一時 subset party から元 slot へ battle 後状態を戻す」既存 pattern。ただし Sky Battle 専用の flag / var をそのまま使うのは危険。

## Release / Purge Flow

`src/pokemon_storage_system.c` で確認した release 関連:

| Symbol | Visibility | Facts |
|---|---|---|
| `Task_ReleaseMon` | `static` | PC UI の release state machine。confirm message、release animation、display refresh、party compact を行う。 |
| `ReleaseMon` | `static` | held item return 後に `PurgeMonOrBoxMon`。 |
| `PurgeMonOrBoxMon` | `static` | party なら `ZeroMonData(&gPlayerParty[position])`、box なら `ZeroBoxMonAt`。 |
| `CompactPartySlots` | public | party の空 slot を詰め、末尾を zero。 |
| `sRestrictedReleaseMoves` | `static const` | Surf / Dive / 一部 map の Strength / Rock Smash release を softlock 防止で制限。 |
| `InitCanReleaseMonVars` | `static` | `AtLeastThreeUsableMons()` を使い、PC release 可否を判定。 |

重要:

- PC release task は UI / storage state に依存しており、battle end callback から直接使う設計には向かない。
- 強制 release は `ZeroMonData` / `CompactPartySlots` 相当の低レベル処理が必要だが、held item、follower、last live mon、battle selection restore、save state を別途設計する必要がある。

## Candidate Hook Points

| Hook | Pros | Risks |
|---|---|---|
| `CB2_EndTrainerBattle` 先頭 | battle outcome と return path を一箇所で見られる。 | Sky Battle / follower restore / battle selection restore との順序が難しい。 |
| `HandleBattleVariantEndParty` 後 | subset party restore 後に処理しやすい。 | 通常選出機能を追加する場合、その restore と順序調整が必要。 |
| `CB2_EndTrainerBattle` の defeated branch | whiteout / no-whiteout と直結。 | win 後 heal とは別処理になる。 |
| field script の post battle | script で `special HealPlayerParty` などを置ける。 | 全 trainer battle に一括適用しにくい。視線検知や special trainer を漏らしやすい。 |
| battle script | loss/win message と近い。 | party mutation、release、field callback は battle script 中でやるには重い。 |

## External Reference Notes

- `https://github.com/PokemonSanFran/pokeemerald/wiki/No-Whiteout-After-Player-Loss` を確認。No Whiteout 後の heal 注意点はこの repo の `B_FLAG_NO_WHITEOUT` comment と一致する。
- `https://github.com/Pokabbie/pokeemerald-rogue/` と `https://github.com/DepressoMocha/emerogue` は公開 repo として確認。強制 release の具体実装は未確認。

## Open Questions

- 全滅時に release する対象は、倒れた Pokemon 全員か、参加 Pokemon だけか、手持ち全員か。
- trainer battle に勝った後の heal と、負けた後の release/heal の順序。
- whiteout cutscene を完全に避けるか、whiteout 後に release するか。
- battle selection と併用時、選出個体だけを元 slot に戻してから release する必要があるか。
- held item を bag に返すか、release と一緒に失うか。
- egg / fainted / alive / boxed mons の扱い。
