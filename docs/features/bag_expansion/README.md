# Bag Expansion

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-09 |
| Baseline | `master` `835520e444`; `git describe` = `expansion/1.15.2-32-g835520e444` |
| Code status | Docs-only feature kickoff |
| Provenance | Local project feature docs |

Status: Investigating
Code status: No source changes

## Goal

通常 bag の pocket 容量を安全に拡張する。

最初の対象は、Field Kit / future field move unlocks で圧迫されやすい
Key Items pocket と、350 TM 構想で不足する TM/HM pocket。

## Current Decision

この feature はまだ実装しない。まず `master` の現状から、bag 拡張が触る
SaveBlock、UI、debug、test、下流 feature を固定する。

通常 bag は `SaveBlock3` ではなく `SaveBlock1` 内の `struct Bag` にある。
`BAG_*_COUNT` を増やすと `struct SaveBlock1` の layout が変わるため、
save compatibility / migration / `test/save.c` 更新を伴う runtime feature として扱う。

現状の `test/save.c` では `SaveBlock1` は `15568` bytes、最大 `15872` bytes、
余りは `304` bytes。`struct ItemSlot` は 4 bytes 相当なので、単純な pocket
拡張は 1 slot あたり約 4 bytes の SaveBlock1 増加になる。

2026-05-09 の catalog 確認では、正の item count は
`include/constants/items.h` の `ITEMS_COUNT` 基準。現行は
`ITEM_GLIMMORANITE = 873` / `ITEMS_COUNT = 874` なので、`ITEM_NONE` を除く
実アイテムは 873 件。

設計目標を全 pocket 合計 1000 slots、TM/HM 350 slots と見ると、現在の
186 slots から +814 slots / 約 3256 bytes の SaveBlock1 増加になる。
SaveBlock1 の現余り 304 bytes と SaveBlock1 `FREE_*` 全部の 2516 bytes を
合わせても 436 bytes 足りない。`SAVE_BLOCK_3_CHUNK_SIZE` をやめて
1 sector あたり 116 bytes を SaveBlock1 側へ戻す save-format 変更まで含めると、
4 sectors 分 +464 bytes で 1000 slots は約 28 bytes 余りで成立する。
ただし DexNav search levels は現行 `NUM_SPECIES = 1573` bytes を SaveBlock3 に
要求するため、DexNav を使う build では SaveBlock3 chunk reclaim はほぼ使えない。
その場合は Hall of Fame sectors を通常 save slot に回して 14 -> 15 sectors にする、
PokemonStorage を縮める、または special sector へ custom storage を作る、といった
より大きい save-format 方針が必要。

ホールドアイテム、バトルアイテム、メガストーン、Z クリスタル等は現状では
別 pocket ではなく `POCKET_ITEMS` 内の `sortType` 分類。見た目や絞り込みだけ
なら Items pocket 内の filter として実装できるが、本当に別 pocket にする場合は
`struct Bag` / `POCKETS_COUNT` / UI / debug / save migration まで含む別設計になる。

## Scope

### In Scope

- `BAG_ITEMS_COUNT`, `BAG_KEYITEMS_COUNT`, `BAG_POKEBALLS_COUNT`, `BAG_TMHM_COUNT`, `BAG_BERRIES_COUNT` の変更方針。
- `struct Bag` / `struct SaveBlock1` の save layout impact。
- bag menu list buffer、sort、debug fill、ROM header bag count、save compatibility tests。
- Field Kit、TM/HM expansion、Champions Challenge bag snapshot への影響整理。

### Out of Scope

- この kickoff では source / generated data / ROM runtime を変更しない。
- item ID の大規模再配置はこの feature の初期 MVP から外す。
- Battle Pyramid bag の容量変更は通常 bag とは別扱いにする。
- Champions Challenge の challenge bag runtime 実装は別 feature 側で扱う。

## Docs

- [Investigation](investigation.md)
- [MVP Plan](mvp_plan.md)
- [Risks](risks.md)
- [Test Plan](test_plan.md)

## Cross-Feature Notes

| Feature | Impact |
|---|---|
| Field Move Modernization | `ITEM_FIELD_KIT` は単一 Key Item なので現時点では bag 拡張を要求しない。per-HM key item 化する場合はこの feature が先行する。 |
| TM Shop Migration / TM Expansion | `BAG_TMHM_COUNT 64` は 350 TM を全部 item slot として持つには不足する。350 slots は `BagPocket.capacity:10` には収まるが、SaveBlock1、`u8` ROM header count、bag UI の `u8` 件数キャッシュを直す必要がある。 |
| Champions Challenge | 通常 bag snapshot / restore は `struct Bag` サイズに連動する。challenge state を SaveBlock1 に置く案はこの feature の capacity decision と競合する。 |
| Save Data / Runtime Flags | bag 拡張は SaveBlock3 ではなく SaveBlock1 の layout change。SaveBlock3 の空きがあっても通常 bag には使わない。 |

## Open Questions

- MVP の pocket target は Key Items だけか、TM/HM も同時に拡張するか。
- 既存 save を migration するか、save-breaking branch として扱うか。
- `BAG_TMHM_COUNT` を 350 へ増やすか、TM ownership を bag から virtual registry に分離するか。
- held item / battle item / Mega Stone 系を Items pocket 内 filter にするか、真の別 pocket にするか。
- `rom_header_gf.c` の bag count fields は `u8` なので、255 を超える pocket count を許可するか。
- SaveBlock1 `FREE_*` だけで 891 slots に抑えるか、SaveBlock3 chunk reclaim / 15-sector normal save / special-sector storage まで含めて 1000 slots にするか。
