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

1. 未使用 flag を選ぶ。
2. `include/config/overworld.h` の `OW_FLAG_NO_ENCOUNTER` にその flag を割り当てる。
3. debug menu の Encounter toggle が表示 / 動作することを確認する。
4. 必要な script / facility start で `setflag`、終了処理で `clearflag` を入れる。
5. 通常 map で草むら / cave / surf を歩き、random encounter が発生しないことを確認する。
6. Fishing / Sweet Scent / Rock Smash / static encounter が現状どおりか確認する。

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
