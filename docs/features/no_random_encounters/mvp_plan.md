# No Random Encounters MVP Plan

## Status

Draft. 実装はまだ行わない。

## MVP Scope

対象:

- 通常歩行中の land / water random encounter を止める。
- `OW_FLAG_NO_ENCOUNTER` を real flag に割り当てる。
- debug menu toggle を使える状態にする。
- script / facility から set / clear できるようにする。

対象外:

- Fishing / Sweet Scent / Rock Smash の抑制。
- static / scripted wild battle の抑制。
- option menu UI。
- encounter table randomizer。
- DexNav / hidden mon 専用制御。

## Implementation Outline

1. `include/constants/flags.h` の `FLAG_UNUSED_0x8E5` を `FLAG_NO_ENCOUNTER` に rename する。新規 `#define` 追加はしない。候補と理由は `investigation.md#Candidate Flag IDs` を参照。
2. `include/config/overworld.h` の `OW_FLAG_NO_ENCOUNTER` を `FLAG_NO_ENCOUNTER` に変更する。
3. debug build (`make debug`) で起動し、Debug menu の Encounter toggle が表示 / 動作することを確認する (`OW_FLAG_NO_ENCOUNTER != 0` のときだけ表示される `DebugAction_FlagsVars_EncounterOnOff` の condition を満たす)。
4. 必要な script / facility start で `setflag FLAG_NO_ENCOUNTER`、終了処理で `clearflag FLAG_NO_ENCOUNTER` を入れる (MVP は script 追加なしで debug toggle のみで足りる)。
5. 通常 map で草むら / cave / surf を歩き、random encounter が発生しないことを確認する。
6. Fishing / Sweet Scent / Rock Smash / static encounter が現状どおり発火することを確認する。
7. Save / Load 経由で flag 状態が persist することを確認する (SYSTEM region の flag を選んでいれば persistent)。

## Future Broad Mode

`broad-wild` が必要になった場合の追加 hook:

| Hook | Change |
|---|---|
| `RockSmashWildEncounter` | 冒頭で no encounter rule を見て `gSpecialVar_Result = FALSE`。 |
| `SweetScentWildEncounter` | 冒頭で no encounter rule を見て `FALSE`。 |
| `FishingWildEncounter` | battle 開始前に no encounter rule を見て失敗 flow へ返す設計が必要。 |
| DexNav / hidden mon | DexNav flow 側で hidden encounter 開始を抑制するか、別 flag にする。 |

static `dowildbattle` は基本的に止めない。story progress を壊すリスクが高い。

## Option UI Later

option UI に入れる場合も、option item は `OW_FLAG_NO_ENCOUNTER` または runtime rule state を set / clear するだけにする。SaveBlock2 に専用 field を足すより、既存 flag / var pattern を優先する。
