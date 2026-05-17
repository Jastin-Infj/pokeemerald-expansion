# Trainer Battle Aftercare Implementation

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-17 |
| Baseline | `master` `9376760f68`; `git describe` = `expansion/1.15.2-54-g9376760f68` |
| Code status | Branch evidence only; not on `master` |
| Provenance | Local feature docs and closed PR evidence |

## Branch Evidence

Trainer Battle Aftercare has a heal-only branch implementation on
`feature/trainer-battle-aftercare-heal`, but the related PR #10 was closed
unmerged. Treat this as branch evidence, not a direct merge candidate.

The branch shape described by the feature docs:

| Area | Branch behavior |
|---|---|
| Config | Adds `B_TRAINER_BATTLE_AFTERCARE`, default `FALSE`. |
| Hook | Calls an aftercare helper from `CB2_EndTrainerBattle` after battle-variant / follower partner restoration. |
| MVP action | Heals the player party only after normal trainer battle wins when config is enabled. |
| Exclusions | Loss, Frontier, Pyramid, Trainer Hill, link, recorded link, secret base, early rival, follower partner, and forfeit are excluded for MVP. |
| Future work | No-whiteout, forced release, battle selection integration, and challenge-specific aftercare are not implemented in the heal-only MVP. |

## Adoption Guidance

- Do not merge the closed PR branch directly.
- If resumed, create a fresh branch from current `master`.
- Add focused tests or mGBA scenarios for config-off, normal win, and excluded
  paths before adoption.
- Keep this separate from Battle Item Restore Policy unless the user explicitly
  asks for a combined battle-end integration branch.

## Validation Status

Current docs do not record a complete local make + mGBA validation pass for a
fresh current-master aftercare branch. The feature should remain
`Planned / branch implementation` until revalidated.

## Open Questions

- Should heal-only aftercare default remain `FALSE` in integration?
- Should forced release be a child feature instead of extending the heal-only branch?
- Should battle selection restore run before or inside the final aftercare helper?

