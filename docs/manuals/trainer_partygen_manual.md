# Trainer Partygen Manual

This manual covers day-to-day use of `tools/champions_partygen` and how it relates to `src/data/trainers.party`.

## When To Use

Use partygen when a trainer's party should be generated from curated catalog data and materialized into the existing `trainers.party` DSL.

Do not use partygen for:

- one-off fixed party edits that are easier to review by hand;
- `src/data/trainers_frlg.party`;
- `src/data/battle_partners.party`;
- `test/battle/*.party`;
- runtime party patching.

## Files

Main tool:

- `tools/champions_partygen/src/main.rs`
- `tools/champions_partygen/partygen.sh`
- `tools/champions_partygen/partygen.cmd`

Catalog input:

- `tools/champions_partygen/catalog/journey.json`
- `tools/champions_partygen/catalog/blueprints/*.json`
- `tools/champions_partygen/catalog/sets/*.json`

Generated / temporary output:

- write review fragments to `/tmp` or `tools/champions_partygen/generated/`
- do not commit `config.local.toml`
- do not commit raw play logs unless a feature doc explicitly asks for a small fixture

Applied ROM source:

- `src/data/trainers.party`

## Basic Workflow

1. Check the catalog and constants:

```sh
rtk tools/champions_partygen/partygen.sh doctor
```

2. Render the generated fragment:

```sh
rtk tools/champions_partygen/partygen.sh generate --seed 1234 --out /tmp/champions_trainers.party
```

3. Validate the fragment:

```sh
rtk tools/champions_partygen/partygen.sh validate --input /tmp/champions_trainers.party
```

4. Review the diff:

```sh
rtk tools/champions_partygen/partygen.sh diff --input /tmp/champions_trainers.party --against src/data/trainers.party
```

5. Apply to a temp file first:

```sh
rtk tools/champions_partygen/partygen.sh apply --input /tmp/champions_trainers.party --target src/data/trainers.party --out /tmp/trainers.party
```

6. Apply to source only after review:

```sh
rtk tools/champions_partygen/partygen.sh apply --input /tmp/champions_trainers.party --target src/data/trainers.party --out src/data/trainers.party
```

7. Build and runtime-check:

```sh
rtk make -j4
rtk make debug -j4
```

Use mGBA Live when generated party data changes battle behavior. A memory check of `gTrainers[DIFFICULTY_NORMAL][TRAINER_*]` is acceptable for confirming that trainerproc output reached ROM data.

## Catalog Editing

`journey.json` decides which trainers are generated.

Each journey trainer must point to:

- an existing `TRAINER_*` constant;
- a `blueprintId`.

Blueprints define:

- `partySize`: number of mons selected at runtime by Trainer Party Pool;
- `poolSize`: number of candidate mons materialized into `trainers.party`;
- `rulesetId`: existing pool rule, for example `POOL_RULESET_BASIC`;
- required slots such as lead or ace;
- preferred roles;
- pool size constraints.

Sets define:

- stable set id;
- species;
- ability;
- moves;
- item;
- IVs and EVs;
- nature;
- level;
- roles and archetypes for tool-side selection;
- final `Tags` that trainerproc understands.

Only these final pool tags are emitted to `trainers.party`:

- `Lead`
- `Ace`
- `Weather Setter`
- `Weather Abuser`
- `Support`
- `Tag 5`
- `Tag 6`
- `Tag 7`

Do not dump every role or archetype into `Tags`. Roles and archetypes are tool vocabulary; `Tags` are ROM pool selection vocabulary.

## Singles And Doubles

Keep singles and doubles as separate blueprint families.

Singles blueprints should usually require at most one `Lead` and one `Ace`.

Doubles blueprints should explicitly model:

- two lead slots;
- spread move availability;
- support / setup balance;
- partner anti-synergy;
- `Double Battle: Yes` in the trainer header.

The current MVP does not enforce header-vs-blueprint battle mode yet. Until that lint exists, manually check generated trainers before applying.

## Existing Trainer Replacement

The MVP replaces existing trainer blocks. This is safer than adding new trainer IDs because existing trainer IDs already have defeated flags and script references.

Before replacing a trainer:

- confirm the trainer appears in `include/constants/opponents.h`;
- confirm the source block exists in `src/data/trainers.party`;
- search scripts for the trainer ID;
- check whether the trainer is single, double, rematch, rival, Gym, Elite Four, or special event;
- preserve header fields unless the feature explicitly changes them.

## New Trainer IDs And NPC Battles

Adding a new trainer is broader than partygen.

A new normal trainer ID affects:

- `include/constants/opponents.h`;
- `TRAINERS_COUNT`;
- `MAX_TRAINERS_COUNT`;
- trainer defeated flag range from `TRAINER_FLAGS_START`;
- `src/data/trainers.party`;
- map script `trainerbattle_*` usage;
- object event placement in map data;
- optional match call / rematch data;
- save compatibility if counts or flag ranges change.

Current Emerald notes from the feature docs:

- `TRAINERS_COUNT_EMERALD = 855`
- `MAX_TRAINERS_COUNT_EMERALD = 864`
- only 9 additional normal trainer slots remain before the max is reached

For Champions MVP, prefer existing trainer slot replacement or a facility-specific roster mechanism over adding many normal trainer IDs.

If a new NPC trainer battle is required, treat it as a map / script task first, then connect partygen only after the trainer ID and script are stable.

## Prize Money

Prize money is not a `trainers.party` generator field.

The current win reward formula is implemented in `src/battle_script_commands.c`:

```text
normal trainer: 4 * lastTrainerMonLevel * moneyMultiplier * trainerClassMoney
single trainer double battle: 4 * lastTrainerMonLevel * moneyMultiplier * 2 * trainerClassMoney
two opponents: reward(opponentA) + reward(opponentB)
secret base: 20 * firstSecretBaseMonLevel * moneyMultiplier
```

`trainerClassMoney` is defined in `gTrainerClasses` in `src/battle_main.c`; missing class money falls back to 5.

Partygen can change the reward indirectly because it can change the last materialized party mon level. If reward stability matters, keep pool member levels consistent or add an expected money report to partygen before changing many major trainers.

## EXP

EXP is not a partygen-owned rule.

The current trainer multiplier is in `src/battle_script_commands.c`, `Cmd_getexp`:

```text
base = species.expYield * faintedLevel
base /= 5 or 7 depending on scaled EXP config
if trainer battle and B_TRAINER_EXP_MULTIPLIER <= GEN_7:
    base = base * 150 / 100
```

`include/config/battle.h` currently sets `B_TRAINER_EXP_MULTIPLIER` to `GEN_LATEST`.

Challenge no-EXP, half-EXP, double-EXP, or profile-based EXP rules should be implemented in a separate runtime rule branch. Partygen may report expected EXP pressure, but it should not silently change EXP logic.

## Player Style Logs

Player style logging is related to partygen because it can later affect weights, but it should not be stored in `trainers.party`.

Recommended staged design:

1. raw log: mGBA / debug / battle summary text in `/tmp` or a local profile directory;
2. normalized log: JSONL rows with battle id, trainer id, team, outcome, turns, faint order, switches, items, and key moves;
3. profile summary: compact JSON with archetype success rates, repeated weaknesses, preferred tempo, and difficulty tolerance;
4. generator input: weights derived from profile summary only.

Do not put raw logs in SaveBlock. Do not make generation depend on logs until no-log fallback stays deterministic.

## Review Checklist

Before committing generated trainer data:

- `partygen doctor` passes.
- `partygen validate` passes.
- `partygen diff` is reviewed.
- `Party Size` is present only for intended pool trainers.
- singles / doubles header matches intended blueprint.
- generated species, move, item, ability, and nature constants exist.
- prize money impact is understood for major trainers.
- expected EXP impact is understood for major trainers.
- `make` and `make debug` pass.
- mGBA Live confirms ROM data or battle behavior when party behavior changed.
