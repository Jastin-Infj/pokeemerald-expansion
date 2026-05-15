# Summary Tera Type Icon Asset References

## Purpose

Track external Tera type icon candidates before any art is imported into the
repository. The first implementation used existing local battle interface
indicator art; the current implementation imports the RavePossum BW Summary
Screen Expansion Tera icons and keeps source / credit notes here.

## Candidates

| Reference | Author / handle | Observed asset | Reuse status | Credit / license note | Verification |
|---|---|---|---|---|---|
| <https://ravepossum.github.io/#bw_summary_screen_expansion> | RavePossum | Public feature page for `bw_summary_screen_expansion`, with screenshots and a wiki link for details / credits. | Probable feature page remembered by the user. Treat as the adoption/source page for the BW Summary implementation, not as a raw asset download. | Page says to see the linked wiki for more information and credits. The wiki link is also referenced by TARC2 credits. | Verified in `ravepossum/ravepossum.github.io` README; the page links to `github.com/ravepossum/pokeemerald-expansion/wiki#bw_summary_screen_expansion`. |
| <https://github.com/ravepossum/pokeemerald-expansion/tree/bw_summary_screen_expansion/graphics/types_bw/tera> | RavePossum branch, Tera icon credit tracked to Zatsu | Per-type 16x16 indexed PNGs under `graphics/types_bw/tera/`, including normal through stellar plus none / mystery. | Imported to `graphics/types/tera/` for this feature. These are already GBA-sized and match the BW Summary Tera icon pipeline. | The branch has no root `LICENSE` file. Its root `CREDITS.md` includes RavePossum and Zatsu as contributors; the specific per-asset credit is supplied by the BW Summary credit line in TARC2. The project accepted credited community reuse for this feature and records the source commit here and in `CREDITS.md`. | Verified and copied from branch `bw_summary_screen_expansion` at commit `83ab2da05eddeddcd85c9426ddee174b9e7067f7`; `src/bw_summary_screen.c` includes `graphics/types_bw/tera/tera_types_bw.4bpp.smol` and creates a `SPRITE_ARR_ID_TERA_TYPE` sprite. |
| <https://github.com/fakuzatsu/verdant/tree/bc970ab71088cd0009441c767d206be35c537b83/graphics/types_bw/tera> | Zatsu / `fakuzatsu` | Same-style per-type 16x16 Tera PNGs under `graphics/types_bw/tera/`. The sampled file SHAs match the RavePossum branch assets. | Attribution / adoption corroboration. Useful because Zatsu is the credited Tera icon author and this public project carries the same icon set. | No root `LICENSE` or `CREDITS.md` was found at the inspected commit, so copying still needs explicit permission or a confirmed upstream usage rule. Credit should include Zatsu / `fakuzatsu` if used. | Verified via GitHub file listing. `src/bw_summary_screen.c` in the same repository includes `graphics/types_bw/tera/tera_types_bw.4bpp.fastSmol` and creates a `SPRITE_ARR_ID_TERA_TYPE` sprite. |
| <https://github.com/jschoeny/TARC2/blob/9a34907512099052a686f575850607d49d8b1679/HACK_CREDITS.md> | TARC2 / BW Summary Screen credits | Credit list explicitly names Zatsu for Tera type icons under BW Summary Screen. | Adoption evidence only; not an asset source. | Confirms the credit name to preserve if the BW Summary icon set is adopted. | Exact GitHub code search for `Zatsu` and `Tera type icons` found this credit entry. |
| <https://github.com/search?q=%22graphics%2Ftypes_bw%2Ftera%2Ftera_types_bw%22&type=code> | Multiple public Emerald/BW Summary forks | Public code references to `graphics/types_bw/tera/tera_types_bw`. | Adoption evidence. Shows this asset path is used by more than one Summary implementation. | Search evidence does not establish redistribution permission by itself. | GitHub code search returned 21 public matches for the exact asset path on 2026-05-15. |
| <https://www.deviantart.com/makiavelik20/art/teratypes-icons-by-Makiavelik20-946210479> | Makiavelik20 | `teratypes icons`; 470x486 RGBA sheet with 1x and 2x type-specific Tera badges. | Visual reference / fallback asset candidate. Suitable visual source, but needs slicing / resizing / GBA palette conversion. | Page and image state "Free use with credits"; credit Makiavelik20 if used. This is an informal usage note, not a formal license. | Downloaded locally for inspection only; not imported. |
| <https://eeveeexpo.com/resources/1139/> | Sonicover | `icon_teraTypes.png` Sol Emerald-style edit for wrigty12's Terastal Phenomenon. | Reference / possible asset candidate. The page says it can be used or repurposed, but the original image availability should be rechecked before import. | Page requests credit to Banjo2015 on DeviantArt for the base Sol Emerald sprite; Sonicover credit is not required but appreciated. | Web reference inspected; not imported. |
| <https://eeveeexpo.com/resources/987/> | wrigty12 | Terastal Phenomenon resource that introduced an `icon_TeraTypes`-style flow for Pokemon Essentials. | Discovery reference only unless the downloadable package and its credits are inspected. | Credit / license details for any bundled graphics must be confirmed from the package before copying. | Web reference inspected; not imported. |
| <https://www.pokeharbor.com/2025/09/pokemon-spirits-of-the-storm/> | Runawayturtle1 / project credits | Emerald ROM hack credit list mentions Zatsu for Tera type icons in a BW Summary Screen Expansion context. | Adoption clue only; not an asset source. | Use this to confirm real project adoption of the BW Summary credit line; do not copy from the hack or mirror. | Cross-checks the TARC2 credit wording on a released hack mirror. |

## Import Record

The RavePossum BW Summary Screen Expansion icon set, credited to Zatsu in later
BW Summary credit lists, was imported because it is already type-specific,
16x16, indexed, and wired through an Emerald-style Summary implementation. This
is a better fit for the Summary Info page than the first implementation's 8x16
battle indicator.

The inspected RavePossum branch and `fakuzatsu/verdant` commit do not expose a
root license covering these PNGs. The accepted use in this project is credited
community reuse; keep the source branch, commit, and credit line with the
feature so the origin is not lost.

## Import Requirements

For any later replacement or derived art:

- Confirm source URL, author, and exact required credit line.
- Record whether resizing, slicing, palette conversion, and repository
  redistribution are allowed by the source text or by explicit permission.
- Keep source and credit notes in `CREDITS.md` and this file.
- Re-run `rtk git diff --check`, `rtk make -j16 -O all`,
  `rtk make -j16 -O debug`, `rtk make -j16 -O check`, and focused Summary
  mGBA validation after wiring the new art.
