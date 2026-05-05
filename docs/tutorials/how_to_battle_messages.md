# How to Add or Change Battle Messages

この tutorial は、battle message の文言変更、新規 `STRINGID_*` 追加、battle script からの表示追加を行う手順です。
通常の map message は [Message Text Manual](../manuals/message_text_manual.md) の field message 節を先に見てください。

## 1. 既存 message を探す

まず `STRINGID_*` と実際の text のどちらから探せるかを決めます。

```bash
rg -n "STRINGID_PKMNREGAINEDHEALTH|recovered|regained" include src data
```

見る場所:

- ID 定義: `include/constants/battle_string_ids.h`
- 文言 table: `src/battle_message.c`
- 表示 script: `data/battle_scripts_1.s` / `data/battle_scripts_2.s`
- macro 定義: `asm/macros/battle_script.inc`

`STRINGID_INTROMSG`、`STRINGID_INTROSENDOUT`、`STRINGID_RETURNMON`、`STRINGID_SWITCHINMON`、`STRINGID_USEDMOVE`、`STRINGID_BATTLEEND`、`STRINGID_TRAINERSLIDE` は special case です。
この ID を見つけたら、`src/battle_message.c` の `BufferStringBattle` 内の `switch` を読みます。

## 2. 既存文言だけを変える

文言だけなら `src/battle_message.c` の該当 entry を変えます。

例:

```c
[STRINGID_PKMNREGAINEDHEALTH] = COMPOUND_STRING("{B_DEF_NAME_WITH_PREFIX}'s HP was restored."),
```

確認すること:

- `{B_DEF_NAME_WITH_PREFIX}` などの battle placeholder を消していないか。
- `\p`、`\n`、`{PAUSE ...}`、`{WAIT_SE}` などの制御文字を意図せず消していないか。
- battle textbox は基本 2 行なので、長くしすぎていないか。

## 3. 新しい battle message を追加する

手順:

1. `include/constants/battle_string_ids.h` の `STRINGID_TABLE_START` より下、`STRINGID_COUNT` より上に新しい `STRINGID_*` を追加する。
2. `src/battle_message.c` の `gBattleStringsTable[STRINGID_COUNT]` に同じ ID の entry を追加する。
3. 必要なら `data/battle_scripts_1.s` または `data/battle_scripts_2.s` から `printstring STRINGID_NEW_MESSAGE` を呼ぶ。
4. `printstring` の後に通常は `waitmessage B_WAIT_TIME_LONG` を置く。

最小例:

```asm
	printstring STRINGID_MY_NEW_MESSAGE
	waitmessage B_WAIT_TIME_LONG
```

```c
[STRINGID_MY_NEW_MESSAGE] = COMPOUND_STRING("{B_ATK_NAME_WITH_PREFIX} is ready!"),
```

注意:

- enum と table の片方だけを変更すると、別の文言が表示されるか、空文字になる可能性があります。
- `STRINGID_COUNT` は末尾 marker なので直接値を入れません。
- new ID を table に追加する場所は、既存の関連 message の近くに置くと追いやすいです。

## 4. 値を入れて表示する

文言内に move、item、ability、number、species などを入れる場合は、C 側で battle text buffer を用意します。

よく使う macro:

| Macro | 用途 |
| --- | --- |
| `PREPARE_MOVE_BUFFER(gBattleTextBuff1, move)` | `{B_BUFF1}` に move name |
| `PREPARE_ITEM_BUFFER(gBattleTextBuff1, item)` | `{B_BUFF1}` に item name |
| `PREPARE_SPECIES_BUFFER(gBattleTextBuff1, species)` | `{B_BUFF1}` に species name |
| `PREPARE_ABILITY_BUFFER(gBattleTextBuff1, ability)` | `{B_BUFF1}` に ability name |
| `PREPARE_STAT_BUFFER(gBattleTextBuff1, statId)` | `{B_BUFF1}` に stat name |
| `PREPARE_BYTE_NUMBER_BUFFER(gBattleTextBuff1, digits, number)` | `{B_BUFF1}` に小さい数値 |
| `PREPARE_STRING_BUFFER(gBattleTextBuff1, stringId)` | `{B_BUFF1}` に別 battle string |

文言側:

```c
[STRINGID_ITEMWASUSEDUP] = COMPOUND_STRING("{B_LAST_ITEM} was used up!"),
[STRINGID_MY_MOVE_MESSAGE] = COMPOUND_STRING("{B_ATK_NAME_WITH_PREFIX}'s {B_BUFF1} shone!"),
```

buffer を詰める場所は、対象効果を処理している C helper または battle script command です。
すでに似た効果がある場合は、その handler の `PREPARE_*_BUFFER` の使い方をコピーするのが安全です。

## 5. Battle script の表示順を決める

message を置く位置は見た目に直結します。

代表的な選択:

| 置く場所 | 見え方 |
| --- | --- |
| `attackanimation` の前 | 技の演出前に説明が出る |
| `attackanimation` / `waitanimation` の後 | 演出後に結果説明が出る |
| `healthbarupdate` / `datahpupdate` の前 | HP が動く前に説明が出る |
| `healthbarupdate` / `datahpupdate` の後 | HP が動いた後に説明が出る |
| `resultmessage` の前 | 急所/効果抜群より先に出る |
| `resultmessage` の後 | 通常結果表示後に追加説明が出る |
| `moveendall` 前 | move-end ability/item hook より前 |
| move-end hook script 内 | item/ability/turn-end による追加表示 |

generic hit path の近くを触る場合は、[Battle Effect Resolution Flow v15](../flows/battle_effect_resolution_flow_v15.md) も確認します。

## 6. `waitmessage` と `flushtextbox`

`printstring`、`printfromtable`、`resultmessage`、`critmessage` は表示要求を作ります。
その後の `waitmessage` がないと、次の animation や script がすぐ進み、読めない表示になることがあります。

通常:

```asm
	printstring STRINGID_SOMETHING
	waitmessage B_WAIT_TIME_LONG
```

短くしたい場合:

```asm
	printstring STRINGID_SOMETHING
	waitmessage B_WAIT_TIME_SHORT
```

message box を空にしてから次の target / hit に進めたい場合:

```asm
	flushtextbox
```

`flushtextbox` は `printstring STRINGID_EMPTYSTRING3` と `waitmessage 1` の wrapper です。
spread move や multi-hit の再実行でよく使われます。

## 7. `printfromtable` を使う

stat 変化や miss のように、C 側が `MULTISTRING_CHOOSER` を決める場合は `printfromtable` を使います。

```asm
	printfromtable gStatUpStringIds
	waitmessage B_WAIT_TIME_LONG
```

確認すること:

- table の定義が `src/battle_message.c` または関連 source にあるか。
- index 用 enum が `include/constants/battle_string_ids.h` にあるか。
- `gBattleCommunication[MULTISTRING_CHOOSER]` を直前の command が想定通りセットしているか。

table の順番を変えると複数効果に影響するため、既存 entry の並び替えは避けます。
追加する場合は、index enum と table entry を同時に増やします。

## 8. Result message を変える

miss、効果抜群、いまひとつ、効果なし、OHKO、Endure、Sturdy、Focus Sash 系は `Cmd_resultmessage` に集約されています。

触る前に見る条件:

- single と double spread で別文言がある。
- multi-hit では各 hit ごとに効果抜群を出さない制御がある。
- Sturdy / held item / affection endurance は `BattleScriptCall` で別 script に飛ぶ。
- `MOVE_RESULT_DOESNT_AFFECT_FOE` は trainer slide の trigger にも関係する。

単に文言を変えるだけなら `gBattleStringsTable` の該当 `STRINGID_*` を変えます。
条件分岐を変える場合は `src/battle_script_commands.c` の `Cmd_resultmessage` を変更することになります。

## 9. Field message と混ぜない

map script の message:

```asm
	message SomeMap_Text_Hello
	waitmessage
	closemessage
```

battle script の message:

```asm
	printstring STRINGID_HELLO
	waitmessage B_WAIT_TIME_LONG
```

この 2 つは別物です。
field の `message` は `ShowFieldMessage`、battle の `printstring` は `PrepareStringBattle` / controller を通ります。
placeholder も別なので、battle 専用 `{B_*}` を map text に置かないでください。

## 10. 確認手順

最低限:

```bash
make -j$(nproc)
```

可能なら、対象 message が出る battle を実機 / emulator / test runner で確認します。

確認観点:

- message が期待タイミングで出る。
- textbox が空白のまま止まらない。
- message が早すぎて読めない、または長く止まりすぎない。
- HP bar、faint animation、ability popup、item activation と順番が破綻していない。
- double battle / spread move / multi-hit で重複表示しない。
- `B_WAIT_TIME_MULTIPLIER` を下げた config でも違和感が少ない。

## よくある失敗

| 症状 | 疑う場所 |
| --- | --- |
| 別の文言が出る | `enum StringID` と `gBattleStringsTable` のずれ |
| 文言が空になる | `STRINGID_COUNT` 範囲外、table entry 未追加、special case の戻り |
| 一瞬だけ表示される | `waitmessage` がない、または `MSG_DISPLAY` が立っていない |
| 何も表示されないのに待つ | `gBattleCommunication[MSG_DISPLAY]` の残り、空文字表示 |
| spread move で表示が消える | `flushtextbox` / `BattleScript_FlushMessageBox` |
| HP が減る前後の見え方が変 | `healthbarupdate` / `datahpupdate` と message の順番 |
| 効果抜群などが重複する | `Cmd_resultmessage` の double / multi-hit 分岐 |
| field では動く placeholder が battle で動かない | field placeholder と battle placeholder の取り違え |

## 関連 docs

- [Message Text Manual](../manuals/message_text_manual.md)
- [Battle Effect Resolution Flow v15](../flows/battle_effect_resolution_flow_v15.md)
- [Battle UI Flow v15](../flows/battle_ui_flow_v15.md)
- [How to add new battle script commands/macros](how_to_battle_script_command_macro.md)
