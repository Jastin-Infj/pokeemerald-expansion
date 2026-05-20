# Party / Status UI External References

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-20 |
| Baseline | `master` `4125f7c4d5`; docs-only investigation branch `docs/champions-run-session-restore-20260520` |
| Code status | External reference registry; no assets or source copied |
| Provenance | GitHub API search, repository README / file listing, local source inspection |

## Reference Table

| Reference URL | Author / handle | Feature observed | Reuse type | License / credit | Verification status |
|---|---|---|---|---|---|
| <https://github.com/ravepossum/emeraldextra> | RavePossum | Emerald Extra hack base with BW Summary code, BW Summary graphics, party menu graphics, and README credits. | Primary code / visual reference candidate. | GitHub API reports no top-level license. README "Use and Credits" says free use is allowed and asks to credit RavePossum plus original authors where mentioned. Do not copy assets/code without carrying this credit note. | Confirmed repository root and README on 2026-05-20. |
| <https://github.com/ravepossum/emeraldextra/blob/main/src/bw_summary_screen.c> | RavePossum, with README credits to Buffel saft, Zeturic, and others for related Summary features | Standalone BW-style Summary implementation guarded by `BW_SUMMARY_SCREEN`. Includes IV/EV display modes, type icons, friendship/shiny/status icons, move selectors, and Summary callbacks. | Code reference, not direct import yet. | Same as repo. Needs compatibility review against current `src/pokemon_summary_screen.c` before copying. | Confirmed path and source listing on 2026-05-20. |
| <https://github.com/ravepossum/emeraldextra/blob/main/include/bw_summary_screen.h> | RavePossum | BW Summary config surface: `BW_SUMMARY_SCREEN`, IV/EV display, nature arrows/colors, category icons, friendship, BW type icons. | Config / API reference. | Same as repo. | Confirmed path and source listing on 2026-05-20. |
| <https://github.com/ravepossum/emeraldextra/tree/main/graphics/summary_screen/bw> | RavePossum / credited original asset authors as applicable | BW Summary graphics: page tilemaps, buttons, move selector, shiny / Pokerus / friendship / stat grade icons, category icons. | Visual / asset reference only until credit is finalized. | No top-level license. Must keep source URL and credit line if imported. | Confirmed file listing on 2026-05-20. |
| <https://github.com/ravepossum/emeraldextra/tree/main/graphics/party_menu> | RavePossum, README credits Xaman for DS-style party screen | Party menu graphics in the same project; README credit line points to Xaman for DS-style party screen. | Visual reference for party screen direction. | Treat Xaman credit as relevant if any DS-style party art / code lineage is copied. | Confirmed file listing and README credit on 2026-05-20. |
| <https://github.com/search?q=bw_summary_screen+in%3Apath+language%3AC&type=code> | Multiple public forks | Public adoption evidence for `bw_summary_screen.c` / `.h` style files. | Discovery only. | Public search matches do not prove permission. Use direct repo / README for source of record. | GitHub code search returned multiple public matches on 2026-05-20, including RavePossum, Erkey830, RubyRaven6, PCG06, and others. |

## Notes For Adoption

- The current project already carries RavePossum / Zatsu credit for the Summary
  Tera Type Icon slice, but that does not automatically cover a full BW Summary
  screen import.
- A real implementation will need image assets, not just C changes. Party grid
  work needs new party background / slot frame assets; BW Summary work needs
  the BW Summary graphics set or a new locally-created equivalent.
- If the BW Summary implementation is copied, update `CREDITS.md` with the
  wider RavePossum / Emerald Extra source note and any specific original-author
  credits that are still present in the imported files or assets.
- If only used as visual reference, keep the links here and do not add source /
  asset credits as if code was imported.
- The party screen layout requirement is local: `2 columns x 3 rows`. External
  DS/BW party screens are reference material, not the exact required layout.

## Asset Intake Checklist

Before a runtime branch imports images:

1. Choose external import vs new local asset creation.
2. Record each source file URL and commit SHA.
3. Record author / handle and credit line.
4. Confirm indexed PNG / palette / tilemap conversion path.
5. Keep copied image / BIN / palette files on the implementation branch, not a
   docs-only master branch.
6. Add mGBA Live screenshots for field party, choose-half, and Summary return.
