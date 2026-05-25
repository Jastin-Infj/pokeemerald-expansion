# Map Asset Relinker Risks

## Risks

| Risk | Impact | Mitigation |
|---|---|---|
| Broad text replacement changes story text or comments | Accidental script / dialogue churn | MVP only rewrites structured JSON and exact include lines; report other matches. |
| Map group order changes map numbers | Save / warp constants become harder to review | Keep existing group order and map order unless plan explicitly requests move. |
| Layout id rename without binary path move | Build may still work but Porymap shows confusing names | Treat layout id, label name, and binary file paths as one plan unit. |
| Directory rename leaves stale `scripts.inc` include | Build failure or missing scripts | Update `data/event_scripts.s` exact include paths and audit missing includes. |
| Warp / connection references keep old `MAP_*` | Runtime warps to wrong or missing map | Rewrite `dest_map` and `connections[].map` where they exactly match old map id. |
| Region Map / Fly references are renamed incorrectly | Fly / Town Map behavior changes unexpectedly | MVP audits `MAPSEC_*` / Fly tables but does not auto-create or rewrite them by default. |
| Porymap writes while tool applies changes | Conflicting file state | Require clean target files and document "close Porymap before apply". |
| JSON formatting churn | Large noisy diffs | Use stable indentation and avoid reordering arrays except target entries. |
| Binary `map.bin` / `border.bin` move mishandled | Layout breaks in Porymap or build | Move files only as paths; do not parse or rewrite binary data in MVP. |
| Generated include files are edited directly | Changes are lost on next generated build | Tool should refuse to edit known generated outputs. |

## Open Questions

- Should the tool support batch floor naming, e.g. `RougeCave`, `RougeCave_B1F`,
  `RougeCave_B2F`, from one manifest?
- Should script label rename be default-on or opt-in? Opt-in is safer because
  labels may be referenced by other scripts.
- Should the tool preserve Porymap-specific ordering exactly, or normalize JSON
  fields? MVP should preserve existing order.
- Should a future Rust implementation share schema with `tools/mapjson`, or stay
  a standalone Python helper?
- Should generated files be regenerated automatically, or should the tool only
  tell the user to run `rtk make generated`?
