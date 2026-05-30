# Trainer Battle Aftercare Implementation

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-30 |
| Baseline | `integration/runtime-dev-20260529` |
| Code status | Adopted into runtime integration branch; not on `master` |
| Provenance | Closed PR #10 / `feature/trainer-battle-aftercare-heal`, re-applied as a small hook |

## Branch Evidence

Trainer Battle Aftercare has a heal-only branch implementation on
`feature/trainer-battle-aftercare-heal`; related PR #10 was closed unmerged.
The runtime integration branch re-applied only the default-off heal hook from
that shelf. The later berry-restore work from the same old branch lineage was
not re-applied here because Battle Item Restore is already owned by #47.

The branch shape described by the feature docs:

| Area | Branch behavior |
|---|---|
| Config | Adds `B_TRAINER_BATTLE_AFTERCARE`, default `FALSE`. |
| Hook | Calls an aftercare helper from `CB2_EndTrainerBattle` after battle-variant / follower partner restoration. |
| MVP action | Heals the player party only after normal trainer battle wins when config is enabled. |
| Exclusions | Loss, Frontier, Pyramid, Trainer Hill, link, recorded link, secret base, early rival, follower partner, and forfeit are excluded for MVP. |
| Future work | No-whiteout, forced release, battle selection integration, and challenge-specific aftercare are not implemented in the heal-only MVP. |

## Runtime Integration 2026-05-30

Adopted commit: `bc7237a568 integration: adopt trainer battle aftercare hook`.

Integration resolution:

- Added `B_TRAINER_BATTLE_AFTERCARE` to `include/config/battle.h`, default
  `FALSE`.
- Added `TrainerBattleAftercare_ShouldApply()` and
  `TrainerBattleAftercare_ApplyIfEnabled()` to `src/battle_setup.c`.
- Hooked aftercare in `CB2_EndTrainerBattle()` after Champions run-session loss
  handling, so Champions defeat restore still intercepts first.
- Kept the hook out of loss, Frontier, Pyramid, Trainer Hill, link, recorded
  link, secret base, early rival, follower partner, and forfeit paths.
- Kept the feature separate from #47 held-item restore and #57 Pokemon Vendor
  bond/reward messaging.

Validation:

- `rtk git diff --cached --check` before commit.
- `rtk make -j16 -O debug` passed with the existing RWX linker warning.
- `rtk make -j16 -O all` passed with the existing RWX linker warning.
- `rtk make -j16 -O check` passed with the existing RWX linker warning.
- mGBA Live smoke booted `pokeemerald.gba` to the title splash and exported
  `/tmp/integration-trainer-aftercare-boot.png`.
- mGBA Live cleanup was clean; CLI `status --all` returned `[]`.

Because the config remains `FALSE`, integration validation proves that the
runtime branch still builds and boots with the aftercare hook present but
inactive. A future config-on pass should cover normal trainer win healing and
the exclusion paths before enabling this behavior by default.

## Open Questions

- Should forced release be a child feature instead of extending the heal-only branch?
- Should battle selection restore run before or inside the final aftercare helper?
