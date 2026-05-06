# Champions Partygen Impact And Next Steps

Date: 2026-05-06

Branch: `feature/trainer-party-generator`

## What Was Implemented

`tools/champions_partygen` is now the first repo-local trainer party generator MVP.

The tool reads:

- `src/data/trainers.party` for existing trainer IDs, headers, and source order.
- `include/constants/*` for trainer, species, move, item, ability, and nature validation.
- `tools/champions_partygen/catalog/*` for journey, blueprint, and curated set input.

The tool writes trainerproc-compatible `.party` DSL and supports:

- `doctor`: repo / catalog / constants sanity check.
- `scan`: trainer and script usage scan.
- `generate`: render generated `.party` fragment.
- `render-one`: render one trainer.
- `explain`: show selected sets and roles.
- `validate`: validate generated fragment before applying.
- `lint`: alias for strict fragment validation.
- `diff`: compare generated trainer blocks against a `.party` source.
- `apply`: replace existing trainer blocks in a target `.party` file.
- `audit show` / `audit list`: inspect generated audit logs.
- `logs normalize`: convert raw mGBA-style battle logs to normalized JSONL.
- `profile build` / `profile show` / `profile diff`: build and inspect
  catalog-pinned player profiles.

The first applied data change is `TRAINER_WALLACE`, converted from a fixed six-Pokemon party to a Trainer Party Pool:

- `Party Size: 3`
- `Pool Rules: Basic`
- pool size 6
- Lead / Ace / Support tags emitted through existing trainerproc syntax

## Impact

The ROM build path is still the normal `trainers.party -> trainerproc -> trainers.h -> ROM` path. No runtime patching and no SaveBlock change were introduced.

The generated output is intentionally Plan A replacement. `src/data/trainers.party` remains the source consumed by Makefile. This keeps normal ROM builds independent of partygen; partygen failure does not block `make` unless someone chooses to regenerate and apply a block.

`Party Size` has behavioral meaning. For `trainers.party`, adding `Party Size` makes trainerproc emit pool data. Do not add it to fixed-order trainers unless pool selection is intended.

Trainer header fields still matter. `Name`, `Class`, `Pic`, `Gender`, `Music`, `Items`, `Double Battle`, `AI`, and `Mugshot` are preserved by the MVP and continue to control non-party battle behavior.

## Scope Decisions

Partygen owns Champions Challenge opponent party data first. It does not try
to batch-replace normal route trainers in the MVP. Normal route trainer
replacement can be a later catalog mode after the Champions flow is stable.

MVP data changes use existing trainer IDs and replace existing trainer blocks.
New trainer IDs are avoided for now because each new trainer tends to pull in
NPC placement, defeated flags, script references, story gates, and map cleanup
work. The catalog marks owned trainers with journey-level tags such as
`champions_challenge` and `partygen_owned`; these tags are written to audit logs
so reviewers can tell which generated blocks are intentionally managed by this
feature.

Generated includes remain deferred. `apply` keeps doing direct block
replacement into `src/data/trainers.party`, with `diff`, audit logs, and mGBA
checks as the review path. Generated includes can be revisited only after drift
checks exist and reviewers can prove that included data still matches the ROM
source expected by scripts and constants.

NPC deletion / replacement is a separate feature boundary. Removing or
replacing field NPCs requires a map-script audit of `events.inc`,
`scripts.inc`, object hide flags, movement scripts, story flags, defeated
trainer flags, and map-specific state. Team Aqua / Team Magma trainers need
extra care because their trainer constants and map scripts are easy to confuse
and may be tied to story progression. Partygen may point at existing trainer
slots, but it does not delete field objects or allocate new NPC sprite/image
resources.

New external NPC art, sprite capacity, palette capacity, and object-event slot
capacity are future resource work. The current partygen branch uses existing
trainer/NPC assets and keeps sprite expansion or cleanup in a separate
capacity/asset feature.

## Confirmed Behavior

Validation report: `docs/features/champions_challenge/partygen_validation_report.md`.

Confirmed checks:

- Rust unit tests and clippy passed.
- `partygen doctor`, `generate`, `validate`, and `diff` passed.
- `make` and `make debug` passed.
- mGBA Live Lua read confirmed ROM data for `gTrainers[DIFFICULTY_NORMAL][TRAINER_WALLACE]`.
- User-side manual check also confirmed the generated party behavior worked.

## Current Boundaries

This PR is trainer party generation only. It does not implement:

- Champions Challenge runtime state.
- challenge party / bag save-restore.
- no-EXP challenge mode.
- reward / prize policy.
- Champions Challenge runtime player profile state.
- ROM-side adaptive difficulty.
- new trainer ID allocation.
- new NPC placement or new map trainer event flow.
- NPC removal, route cleanup, or story flag rewiring.
- generated drift checking in CI.

## Remaining Work

The next implementation phase is now tracked by two design documents. The
current branch implements their first code pass; each doc now marks what is
implemented and what is still planned.

- Lint and audit log: [partygen_lint_spec.md](partygen_lint_spec.md).
  Covers doubles vs singles consistency, rank band / power budget, weather
  setter / abuser pairing, item duplication, required slot health, light
  coverage check, cross-trainer drift, audit log schema, severity model, and
  exit behavior per CLI.
- Player style logging: [partygen_player_style_logging.md](partygen_player_style_logging.md).
  Covers raw mGBA log capture, normalize step (JSONL), profile build step,
  weight feedback into `partygen generate`, file locations, commit policy,
  privacy boundaries, and failure modes.

Catalog completeness:

- Keep each trainer / stage library isolated with blueprint `setGroups` and
  set `groups` before adding more Elite Four or route trainer sets.
- Fill real journey stages.
- Add group profiles for route trainers, Gym Leaders, Elite Four, Champion, rivals, grunts, and special fights.
- Expand curated set libraries beyond the current Hoenn demo sets.
- Add source notes for why each set exists.
- Author against the new `mode`, `rank`, `minRank`, `maxRank`, and
  `bstBudget` fields introduced in `partygen_lint_spec.md` so catalog growth
  does not have to be re-shaped after the lint lands.

Battle mode separation:

- See `partygen_lint_spec.md` "Doubles vs Singles" for required catalog
  fields (`mode`, `requireSpreadMove`, `doublesSpreadMove`) and the `DBL00x`
  check ids. Implementation work for this section is grouped with the lint
  layer below.

Power / quality control:

- See `partygen_lint_spec.md` "Rank Band / Power Budget", "Item Duplication",
  "Move Coverage (light)", and "Cross-Trainer Aggregate" for catalog fields,
  check ids, and severity defaults.
- `explain` enrichment (why a set was chosen) is folded into the audit log
  schema in `partygen_lint_spec.md` rather than a separate command surface.

Player style logging:

- See `partygen_player_style_logging.md` for the four-stage pipeline (raw →
  normalized JSONL → profile → weight feedback), the new CLI subcommands
  (`logs normalize`, `profile build`, `profile show`, `profile diff`,
  `generate --profile`, `audit show --run`), and the `[profile]` config
  block.
- `minimum_adaptation_runs`, `weakness_bonus`, `archetype_cooldown`,
  `exploration_rate`, `weakness_threshold`, and `min_sample_count` defaults
  live in that doc and in `config.example.toml`.

Data management:

- Keep `config.local.toml`, raw logs, generated reports, and generated fragments out of commits unless they are intended review artifacts.
- Commit source catalog changes and applied `.party` changes together when the data is meant to ship.
- Prefer `partygen diff` before `partygen apply`.
- `tools/champions_partygen/local/` is gitignored for logs, audit files, and
  active profiles.

Manual / onboarding:

- Keep `docs/manuals/trainer_partygen_manual.md` as the day-to-day guide.
- Link new trainer / NPC addition work back to trainer ID capacity and defeated flag usage.
- Document mGBA verification whenever generated trainer data is applied to ROM data.
- Keep lint and player-style CLI examples in the manual as the implementation
  changes.

## Prize Money Note

Trainer win money is not controlled directly by `trainers.party` party fields.

Current formula is in `src/battle_script_commands.c`, `GetTrainerMoneyToGive`:

- Secret Base: `20 * firstSecretBaseMonLevel * moneyMultiplier`
- Normal trainer battle: `4 * lastTrainerMonLevel * moneyMultiplier * trainerClassMoney`
- two separate opponents: reward is calculated for both opponents and added
- double battle with one trainer: formula has an extra `* 2`

`trainerClassMoney` comes from `gTrainerClasses` in `src/battle_main.c`. If a class has no money value, the fallback is 5.

Partygen can affect money indirectly because generated party order and `Party Size` change which mon is treated as the last party mon. In the Wallace MVP, the last pool entry is level 50, so Champion money uses level 50 rather than the old fixed Milotic level 58.

If prize money needs challenge-specific behavior, implement it in a separate reward / economy branch. Partygen should at most report expected money impact.

## EXP Note

Trainer EXP is not controlled by `trainers.party` directly.

Current EXP flow is in `src/battle_script_commands.c`, `Cmd_getexp`:

- EXP starts from `species expYield * faintedMonLevel`.
- It is divided by 5 or 7 depending on scaled EXP config.
- If `B_TRAINER_EXP_MULTIPLIER <= GEN_7` and battle type is trainer, it applies `* 150 / 100`.
- Split EXP and Exp. Share rules are handled after that.
- `BattleTypeAllowsExp` disables EXP for Frontier, Trainer Hill, link, Safari, Battle Tower, and related special battles.

`include/config/battle.h` currently sets `B_TRAINER_EXP_MULTIPLIER` to `GEN_LATEST`, whose comment says Gen 7+ no longer gives the 1.5 trainer EXP multiplier.

Challenge-specific EXP on/off or EXP multiplier should be a separate runtime battle-rule branch. Partygen can still include expected EXP impact in reports because generated levels and species affect reward volume.

## Open Questions

- How much player style data is enough before adaptive weighting stops being noise? (Tracked alongside `minimum_adaptation_runs` default in `partygen_player_style_logging.md`.)
- Should reward / economy reporting get its own lint family outside partygen
  once Champions reward policy is designed?
