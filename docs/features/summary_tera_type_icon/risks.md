# Summary Tera Type Icon Risks

## Risks

| Risk | Severity | Impact | Mitigation |
|---|---|---|---|
| Icon overlap | Low | The 16x16 Tera badge may touch nearby Summary UI if the coordinate is moved too far right. | Keep coordinates in config and validate with mGBA screenshot. |
| Stale icon on eggs | Low | Switching from a normal Pokemon to an egg could leave the previous Tera badge visible. | Explicitly hide the Tera badge for eggs. |
| Render priority mismatch | Low | The Tera sprites could appear behind Summary UI if their OAM priority is unsuitable. | Use the same priority as existing Summary type icons and validate with mGBA screenshot. |
| Config scope confusion | Low | This should not become a State Editor option. | Keep the feature display-only and document it separately from State Editor controls. |
| External asset provenance | Medium | The imported RavePossum BW Summary icon set is credited to Zatsu and adopted by public Summary implementations, but the inspected source does not expose a root license for the PNGs. | Keep source branch, commit, and credit in `CREDITS.md` and `asset_references.md`. Treat future replacement art as a new credited import. |
| External asset conversion | Low | The imported icons are already 16x16 indexed PNGs, but the combined 4bpp sheet still depends on local graphics rules. | Validate dimensions / palette in build output, then re-run Summary screenshot validation. |

## Accepted Deferrals

- No icon scaling.
- No locally redrawn Tera-specific badge art; the imported RavePossum / Zatsu
  badge set is used as-is.
- No battle UI behavior changes.
