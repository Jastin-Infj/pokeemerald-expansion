# Map Asset Relinker

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-25 |
| Baseline | `master` `bbafc56b46` |
| Code status | Docs-only investigation; tool not implemented |
| Provenance | Local source read, Porymap-shaped map data, and `feature/new-map-test-v15` audit |

## Goal

Porymap で仮名の map / layout を作った後に、後から安全に正式名へ戻すための
repo-side tool を作る。

対象は Porymap 自体の改造ではなく、Porymap が使う project files を整合した
状態へリネーム / 再紐付けする補助 CLI。

主なユースケース:

- `RougeCave_2` のような仮 map 名を `RougeCave_1F` / `RougeCave_2F`
  のような正式名へ戻す。
- `LAYOUT_ROUGE_CAVE_2`、`RougeCave_2_Layout`、
  `data/layouts/RougeCave_2/` のような layout 周辺名を同時にそろえる。
- `map_groups.json`、`layouts.json`、map `id` / `name` / `layout`、
  `data/event_scripts.s` の include、warp / connection 参照をまとめて更新する。
- 変更前に dry-run / diff preview を出し、Porymap で再度開ける状態を保つ。

## Position

Porymap は map editor として正しく、source-of-truth の JSON / binary を
きれいに扱う。ただし「後から名前体系を変える」「仮 layout を正式 layout に
差し替える」「関連参照を漏れなく書き換える」は、Porymap より repo 専用
refactor tool の方が安全。

この feature は `tools/map_asset_relinker/` などに置く Python CLI として
設計するのが第一候補。理由は、既存の map / layout source が JSON と file
rename 中心で、標準ライブラリだけで dry-run と atomic-ish な更新を組みやすい
ため。Rust は後で schema / CLI が固まってからで十分。

## Primary Docs

- `docs/features/map_asset_relinker/investigation.md`
- `docs/features/map_asset_relinker/mvp_plan.md`
- `docs/features/map_asset_relinker/risks.md`
- `docs/features/map_asset_relinker/test_plan.md`
- `docs/flows/map_creation_flow_v15.md`
- `docs/flows/map_registration_fly_region_flow_v15.md`

## Non-Goals

- Porymap 本体や Porymap project format を変更しない。
- Generated files (`include/constants/map_groups.h`,
  `include/constants/layouts.h`, `data/maps/*/header.inc` など) を直接編集しない。
- Region Map / Fly / wild encounter / trainer / story flag を自動で新設しない。
  それらは audit して警告するだけにする。
- 初期 MVP では binary map data の内容を解析しない。`map.bin` / `border.bin`
  は rename / move 対象として扱う。
