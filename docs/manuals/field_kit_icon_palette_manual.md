# Field Kit Icon And Palette Manual

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-09 |
| Baseline | `feature/field-move-toolkit-item` |
| Code status | Field Kit runtime implemented; icon remains placeholder |
| Provenance | Local project overlay |

この manual は、一時的な Field Kit question mark icon を正式な icon / palette に差し替えるための handoff です。
Field Kit の runtime behavior は変更しません。

## Current State

`ITEM_FIELD_KIT` は現時点で generic question mark graphics を使っています。

- item entry: `src/data/items.h`
- icon pointer: `gItemIcon_QuestionMark`
- palette pointer: `gItemIconPalette_QuestionMark`

icon art と palette が用意できたら、この 2 つの pointer を Field Kit 専用 symbol に差し替えます。

## Asset Shape

既存 item icon は次の形です。

- 24x24 PNG
- 4-bit indexed color
- 16-color palette
- icon source under `graphics/items/icons/`
- palette source under `graphics/items/icon_palettes/`

参考にしやすい既存例:

- `graphics/items/icons/question_mark.png`
- `graphics/items/icon_palettes/question_mark.pal`
- `graphics/items/icons/town_map.png`
- `graphics/items/icon_palettes/town_map.pal`

## Recommended Names

後で検索しやすいよう、Field Kit 専用名で揃えます。

- `graphics/items/icons/field_kit.png`
- `graphics/items/icon_palettes/field_kit.pal`
- `gItemIcon_FieldKit`
- `gItemIconPalette_FieldKit`

## Implementation Steps

1. icon PNG と palette `.pal` を追加する。
2. `include/graphics.h` に extern declaration を追加する。

```c
extern const u32 gItemIcon_FieldKit[];
extern const u16 gItemIconPalette_FieldKit[];
```

3. `src/data/graphics/items.h` に graphics definition を追加する。置き場は generic / key item icon 群の近くでよい。

```c
const u32 gItemIcon_FieldKit[] = INCGFX_U32("graphics/items/icons/field_kit.png", ".4bpp.smol");
const u16 gItemIconPalette_FieldKit[] = INCGFX_U16("graphics/items/icon_palettes/field_kit.pal", ".gbapal");
```

4. `src/data/items.h` の `ITEM_FIELD_KIT` を差し替える。

```c
.iconPic = gItemIcon_FieldKit,
.iconPalette = gItemIconPalette_FieldKit,
```

## Validation

最低限これを実行します。

```sh
rtk make -j16 -O all
```

debug shortcut で item を素早く確認する場合は、debug ROM も作ります。

```sh
rtk make -j16 -O debug
```

手動確認:

1. debug ROM を起動する。
2. `Scripts... > Field Kit Full` を使う。
3. Bag -> Key Items を開く。
4. `FIELD KIT` が新しい icon / palette で表示されることを確認する。
5. Field Kit を SELECT 登録し、Field Kit menu が引き続き開くことを確認する。

## Notes

- icon 作業では Field Kit capability flags を変更しない。
- この visual pass では per-HM key item を追加しない。bag capacity 拡張は別 feature。
- Bag では正しく、dark / cave map だけで変に見える場合も、まず icon palette 自体を確認する。Field Kit menu の border palette は runtime 側で別に処理済み。
