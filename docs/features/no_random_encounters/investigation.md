# No Random Encounters Investigation

調査日: 2026-05-05。実装なし。

## Existing Hooks

| File | Symbol | Finding |
|---|---|---|
| `include/config/overworld.h` | `OW_FLAG_NO_ENCOUNTER` | flag id を割り当てる config。default `0` は無効。 |
| `src/field_control_avatar.c` | `CheckStandardWildEncounter` | `FlagGet(OW_FLAG_NO_ENCOUNTER)` が TRUE なら通常歩行 encounter を止める。 |
| `src/wild_encounter.c` | `StandardWildEncounter` | land / water step encounter、roamer、mass outbreak、Battle Pike / Pyramid の通常歩行 encounter を扱う。 |
| `src/wild_encounter.c` | `RockSmashWildEncounter` | 現状 `OW_FLAG_NO_ENCOUNTER` を見ない。 |
| `src/wild_encounter.c` | `SweetScentWildEncounter` | 現状 `OW_FLAG_NO_ENCOUNTER` を見ない。 |
| `src/wild_encounter.c` | `FishingWildEncounter` | 現状 `OW_FLAG_NO_ENCOUNTER` を見ない。 |
| `src/debug.c` | `DebugAction_FlagsVars_EncounterOnOff` | `OW_FLAG_NO_ENCOUNTER != 0` なら debug menu から toggle 可能。 |

## Important Boundary

既存 `OW_FLAG_NO_ENCOUNTER` は `CheckStandardWildEncounter` で止めているため、歩行による standard encounter の前段で止まる。

このため、MVP で止まるもの:

- 草むら / 洞窟などの land step encounter。
- Surf 中の water step encounter。
- `StandardWildEncounter` path に入る Battle Pike / Battle Pyramid の歩行 encounter。

MVP では止まらないもの:

- Fishing。
- Sweet Scent。
- Rock Smash。
- static / scripted `setwildbattle` + `dowildbattle`。
- DexNav / hidden mon が独自に battle を開始する path。

これは bug ではなく、初期仕様として切る方が安全。全部止める場合は `broad-wild` mode として別 hook を追加する。

## Config Note

`OW_FLAG_NO_ENCOUNTER` は `0/1` の feature switch ではない。

```c
#define OW_FLAG_NO_ENCOUNTER        0  // If this flag is set, wild encounters will be disabled.
```

実装時は以下のように flag id を割り当てる。

```c
#define OW_FLAG_NO_ENCOUNTER        FLAG_UNUSED_0x264
```

割り当て後は script で `setflag FLAG_UNUSED_0x264` / `clearflag FLAG_UNUSED_0x264` するか、debug menu から toggle する。

## Design Fit

Champions Challenge / roguelike facility で使う場合は、facility state に直接 random encounter logic を持たせるより、開始時に no encounter flag を立て、終了時に clear する方が影響範囲が狭い。

ただし、facility 専用 map で wild encounter table 自体を置かない設計も可能。その場合は flag なしでも歩行 encounter は発生しない。no encounter flag は「既存 map を一時的に安全に歩かせたい」「debug / route review で野生に邪魔されたくない」用途に向く。
