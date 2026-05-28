# Map Asset Relinker Risks

## Risks

| Risk | Impact | Mitigation |
|---|---|---|
| Broad text replacement changes story text or comments | Accidental script / dialogue churn | MVP only rewrites structured JSON and exact include lines; report other matches. |
| Map group order changes map numbers | Save / warp constants become harder to review | Keep existing group order and map order unless plan explicitly requests move. |
| Layout id rename without binary path move | Build may still work but Porymap shows confusing names | Treat layout id, label name, and binary file paths as one plan unit. |
| Map points at another valid layout | Audit cannot know author intent because both ids exist | Use explicit `--no-layout-rename --set-layout-id <correct-layout-id>` anchored on the affected map. |
| Directory rename leaves stale `scripts.inc` include | Build failure or missing scripts | Update `data/event_scripts.s` exact include paths and audit missing includes. |
| Duplicate script includes after manual repair | Script block may be included twice or appear in confusing locations | Applying a plan for the map deduplicates later exact include lines while keeping the first include. |
| Map rename leaves old `*_MapScripts` label | Generated headers may reference the new map script label while `scripts.inc` still defines the old one | Use opt-in `--rewrite-script-labels`; it rewrites symbol prefixes in the selected map script and skips `.string` dialogue lines. |
| Warp / connection references keep old `MAP_*` | Runtime warps to wrong or missing map | Rewrite `dest_map` and `connections[].map` where they exactly match old map id. |
| Region Map / Fly references are renamed incorrectly | Fly / Town Map behavior changes unexpectedly | Use explicit mapsec metadata options and review dry-run. Complete Fly setup needs bounds, region-map cell, `sMapHealLocations`, `sFlyLocations`, `GetMapsecType()`, and an unlock flag path. |
| Wrong-but-valid mapsec cannot be detected automatically | Audit has no way to know author intent when the id exists | Use explicit `--rename-mapsec OLD:NEW` and review dry-run before apply. |
| Correct id but wrong display name | Porymap looks partly right but region-map or layout labels stay confusing | Use metadata-only repairs: `--set-layout-name` and `--set-mapsec-name`. |
| Town Map cell is set without matching mapsec bounds | Cursor lookup and player icon can disagree | Apply `--set-mapsec-bounds` and `--set-region-map-cell` together, then validate in-game. |
| Fly destination row is added without an unlock flag path | Fly icon may never appear or may appear at the wrong time | `--ensure-fly-location` requires an explicit `FLAG_*`; scripts/config that set that flag remain feature-owned. |
| Fly destination row is added without `sMapHealLocations` | Fly icon appears, but selecting it can warp to the wrong fallback map | Pair `--ensure-fly-location` with `--ensure-mapsec-map MAPSEC_ID:MAP_ID[:HEAL_LOCATION_ID]`. |
| Fly destination row is added without `GetMapsecType()` | Fly icon may draw, but pressing A on the cursor does not select the destination | Pair `--ensure-fly-location` with `--ensure-fly-mapsec-type MAPSEC_ID:FLAG`. |
| New Fly destination has no marker in the static region-map artwork | Selected Fly icon appears to blink off completely instead of color-flickering like stock cities | Either edit the region-map art to add a base marker, or add the mapsec to the source-side custom palette-blink path used by Route301. |
| Fly icon style is set without matching bounds | Icon style is correct but the visual shape is wrong | Use `--set-mapsec-bounds MAPSEC_ID:X:Y:WIDTH:HEIGHT`; `1x1` selects 8x8, `2x1` selects 16x8, and `1x2` selects 8x16. |
| Temporary flag claim collides with future upstream use | Save flag meaning may change after an upstream merge | Prefer local feature flags for production. `--claim-unused-flag` is acceptable for controlled branch validation, but should be reviewed before long-lived integration. |
| Visit flag is never set in script | Destination remains locked until manually flagged | Use `--set-transition-setflag FLAG_*` for simple map scripts or add the flag to the owning script logic. |
| Town Map display name differs from intended text | Map popup / Fly text appears wrong even when mapsec id is correct | Use `--set-mapsec-name MAPSEC_ID:NAME`; this is the current "transcription" / display-name repair path for Porymap-created mapsecs. |
| Region map layout header edit targets the wrong region | Cursor lookup changes the wrong world map | Use explicit region names: `hoenn`, `kanto`, `sevii123`, `sevii45`, or `sevii67`; dry-run before apply. |
| Bad group deletion removes real maps | Accidental data loss from a cleanup option | `--drop-group` refuses non-empty groups and only removes explicit group names. |
| Porymap writes while tool applies changes | Conflicting file state | Require clean target files and document "close Porymap before apply". |
| Backup directory is treated as map data | Porymap could scan backup copies if they live under `data/maps` | Store archives under repo-level `.map_asset_relinker_backups/*.bak.tar`, not inside map/layout source directories. |
| JSON formatting churn | Large noisy diffs | Use stable indentation and avoid reordering arrays except target entries. |
| Binary `map.bin` / `border.bin` move mishandled | Layout breaks in Porymap or build | Move files only as paths; do not parse or rewrite binary data in MVP. |
| Correct layout identity still fails build because of tileset choice | Link failure from a generated layout referencing a build-incompatible tileset | Use explicit `--set-primary-tileset` / `--set-secondary-tileset` while preserving the layout id/name/path. |
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
- Should Fly / Town Map setup gain a manifest command that applies bounds,
  region cell, display name, visit flag, mapsec type, and destination row from
  one declarative block instead of separate CLI flags?
