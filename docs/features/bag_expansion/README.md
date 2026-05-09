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
Key Items pocket と、250 TM 構想で不足する TM/HM pocket。

## Current Decision

この feature はまだ実装しない。まず `master` の現状から、bag 拡張が触る
SaveBlock、UI、debug、test、下流 feature を固定する。

通常 bag は `SaveBlock3` ではなく `SaveBlock1` 内の `struct Bag` にある。
`BAG_*_COUNT` を増やすと `struct SaveBlock1` の layout が変わるため、
save compatibility / migration / `test/save.c` 更新を伴う runtime feature として扱う。

現状の `test/save.c` では `SaveBlock1` は `15568` bytes、最大 `15872` bytes、
余りは `304` bytes。`struct ItemSlot` は 4 bytes 相当なので、単純な pocket
拡張は 1 slot あたり約 4 bytes の SaveBlock1 増加になる。

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
| TM Shop Migration / TM Expansion | `BAG_TMHM_COUNT 64` は 250 TM を全部 item slot として持つには不足する。virtual TM registry か TM/HM pocket 拡張を決める必要がある。 |
| Champions Challenge | 通常 bag snapshot / restore は `struct Bag` サイズに連動する。challenge state を SaveBlock1 に置く案はこの feature の capacity decision と競合する。 |
| Save Data / Runtime Flags | bag 拡張は SaveBlock3 ではなく SaveBlock1 の layout change。SaveBlock3 の空きがあっても通常 bag には使わない。 |

## Open Questions

- MVP の pocket target は Key Items だけか、TM/HM も同時に拡張するか。
- 既存 save を migration するか、save-breaking branch として扱うか。
- `BAG_TMHM_COUNT` を 250 以上へ増やすか、TM ownership を bag から virtual registry に分離するか。
- `rom_header_gf.c` の bag count fields は `u8` なので、255 を超える pocket count を許可するか。
- SaveBlock1 余り 304 bytes で収めるか、`FREE_*` toggle を使って容量を空けるか。
