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
- `diff`: compare generated trainer blocks against a `.party` source.
- `apply`: replace existing trainer blocks in a target `.party` file.

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
- player playstyle logging.
- adaptive difficulty.
- new trainer ID allocation.
- new NPC placement or new map trainer event flow.
- generated drift checking in CI.

## Remaining Work

Catalog completeness:

- Fill real journey stages.
- Add group profiles for route trainers, Gym Leaders, Elite Four, Champion, rivals, grunts, and special fights.
- Expand curated set libraries beyond the current Hoenn demo sets.
- Add source notes for why each set exists.

Battle mode separation:

- Add explicit singles and doubles blueprint families.
- Enforce `Double Battle: Yes` / `No` consistency against blueprint mode.
- Map doubles-specific tags and constraints separately from singles.
- Add lint for required lead count, support count, spread move mix, and double battle partner interactions.

Power / quality control:

- Add rank bands and power budgets.
- Add lint for underbuilt sets, empty coverage, illegal move constants, item duplication, and missing synergy.
- Add explain output that shows why a set was chosen and which rule accepted it.

Player style logging:

- Store raw play logs outside save data first.
- Normalize logs into JSONL or CSV.
- Feed only summarized profile data into generator weights.
- Keep adaptation weak until enough runs exist.

Data management:

- Keep `config.local.toml`, raw logs, generated reports, and generated fragments out of commits unless they are intended review artifacts.
- Commit source catalog changes and applied `.party` changes together when the data is meant to ship.
- Prefer `partygen diff` before `partygen apply`.

Manual / onboarding:

- Keep `docs/manuals/trainer_partygen_manual.md` as the day-to-day guide.
- Link new trainer / NPC addition work back to trainer ID capacity and defeated flag usage.
- Document mGBA verification whenever generated trainer data is applied to ROM data.

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

- Should partygen own only Champions Challenge trainers, or also offer batch replacement for normal route trainers?
- Should generated trainer blocks always replace existing blocks, or should Plan B generated includes be revisited after drift checks exist?
- How much player style data is enough before adaptive weighting stops being noise?
- Should prize money be reported as a lint warning when generated final pool level changes a major trainer reward?
- Should new trainer IDs be avoided entirely for Champions MVP, using existing trainer slots only?
