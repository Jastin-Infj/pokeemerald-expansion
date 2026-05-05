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

実装時は以下のように **既存 unused flag id を 1 つ rename して** 割り当てる。新規 flag macro を追加するのではない。

```c
// 例: include/constants/flags.h で
// #define FLAG_UNUSED_0x8E5    (SYSTEM_FLAGS + 0x85)
// を
// #define FLAG_NO_ENCOUNTER    (SYSTEM_FLAGS + 0x85)
// に rename。include/config/overworld.h の側を:
#define OW_FLAG_NO_ENCOUNTER        FLAG_NO_ENCOUNTER
```

debug menu / script からは `FlagSet(FLAG_NO_ENCOUNTER)` / `FlagClear(FLAG_NO_ENCOUNTER)`。

## Candidate Flag IDs

`include/constants/flags.h` を 1.15.2 時点で確認した結果、以下の region に未使用 flag が並んでいる。
すべて save-persistent 領域に属する。

| Region | Range (offset from region start) | Persistence | Notes |
|---|---|---|---|
| `TEMP_FLAGS` (`0x000`–`0x01F`) | `FLAG_TEMP_5`–`FLAG_TEMP_10` (10 件), `FLAG_TEMP_F`/`0x10` 含む | Map 切り替えで clear | challenge 中だけ単一 map で使うなら可だが、歩いて map 跨ぎする MVP には不向き。 |
| `SYSTEM_FLAGS` (`0x860`–`0x91F`) | `FLAG_UNUSED_0x881`–`0x887`, `0x88E`–`0x8F`, `0x8E3`, `0x8E5`–`0x91F` (連番で 60 件以上) | Save persistent | 推奨。`FLAG_UNUSED_0x8E5` 以降のブロックは「Unused Flag」とのみ書かれ、debug 残りでもない。 |
| `DAILY_FLAGS` (`0x920`–`0x95F`) | `FLAG_UNUSED_0x920`, `0x923`–`0x929`, `0x933`, `0x935`–`0x95F` (40 件以上) | Save persistent だが daily reset で clear される可能性あり | encounter rule のように「セットしたら維持」で運用する flag には不向き。 |

**MVP 推奨**: `FLAG_UNUSED_0x8E5` (`SYSTEM_FLAGS + 0x85`)。

理由:

- SYSTEM region は save persistent。debug toggle / facility state / option どの起点で立てても map 跨ぎで保持される。
- `0x8E5`–`0x91F` 連番は他に使われていない。upstream 追従の場合も衝突確率が低い。
- DAILY 領域は daily reset 系処理で意図せず clear される可能性が残るので避ける。
- TEMP 領域は map 退出で clear されるため、歩行型 encounter 抑制の用途と合わない。

別候補として `FLAG_UNUSED_0x8E6`–`0x8EE` 付近の連続帯を取り、後続 feature (challenge state、no scripted encounter mode、option page など) でも同じブロックから割り当てると、debug build / save migration の追跡が楽になる。

割り当てる flag は `include/constants/flags.h` で **macro を rename する**。新規 `#define` を追加して既存 `FLAG_UNUSED_*` と二重定義にしない。FLAGS_COUNT を破壊しないため、領域追加 (`FLAGS_COUNT += N`) は MVP では行わない。

## Design Fit

Champions Challenge / roguelike facility で使う場合は、facility state に直接 random encounter logic を持たせるより、開始時に no encounter flag を立て、終了時に clear する方が影響範囲が狭い。

ただし、facility 専用 map で wild encounter table 自体を置かない設計も可能。その場合は flag なしでも歩行 encounter は発生しない。no encounter flag は「既存 map を一時的に安全に歩かせたい」「debug / route review で野生に邪魔されたくない」用途に向く。
