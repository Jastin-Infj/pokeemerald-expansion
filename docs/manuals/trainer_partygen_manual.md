# Trainer Partygen Manual

This manual covers day-to-day use of `tools/champions_partygen` and how it relates to `src/data/trainers.party`.

For trainer battle prize money, encounter music, battle BGM, victory BGM,
mugshot transition, and battle background routing, read
[Trainer Battle Reward and Audio Flow](../flows/trainer_battle_reward_audio_flow_v15.md).

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
- `src/data/champions_partygen/trainers.party.inc`

## ROM Config Gate

The 16.0 PartyGen integration is config-gated and disabled by default.

`include/config/champions_partygen.h` owns the ROM-side switches. The file is
also included by `include/config/battle.h` for C runtime use:

- `B_CHAMPIONS_PARTYGEN_TRAINERS`: when `1`, the vanilla Elite Four /
  Wallace blocks in `src/data/trainers.party` are skipped and
  `src/data/champions_partygen/trainers.party.inc` is included instead.
- `B_CHAMPIONS_PARTYGEN_LEVEL`: documented target level for the generated
  Champions catalog, currently 50.
- `B_CHAMPIONS_PARTYGEN_EXP_MODE`: set to
  `B_CHAMPIONS_PARTYGEN_EXP_NONE` to suppress normal trainer battle EXP while
  PartyGen trainers are enabled.
- `B_CHAMPIONS_PARTYGEN_BADGE_BOOSTS`: when `0`, PartyGen-enabled builds
  zero the Gen3 badge stat boost flags even if global badge-boost mechanics are
  set back to Gen3.
- `B_CHAMPIONS_PARTYGEN_OBEDIENCE_CHECKS`: when `0`, PartyGen-enabled
  builds skip player-side obedience checks for the Lv50 challenge path.

Catalog set files may declare `"defaultExp": "none"` and individual sets may
override with `"exp": "normal"` or `"exp": "none"`. This is tool-side metadata
and validation. The ROM behavior comes from `B_CHAMPIONS_PARTYGEN_EXP_MODE`.

When `B_CHAMPIONS_PARTYGEN_TRAINERS` is `0`, normal builds use the
upstream fixed Elite Four / Wallace party blocks and the PartyGen include is
not compiled into trainer data.

## Basic Workflow

1. Check the catalog and constants:

```sh
rtk tools/champions_partygen/partygen.sh doctor
```

2. Render the generated fragment:

```sh
rtk tools/champions_partygen/partygen.sh generate --seed 1234 --out src/data/champions_partygen/trainers.party.inc
```

3. Validate the fragment:

```sh
rtk tools/champions_partygen/partygen.sh validate --input src/data/champions_partygen/trainers.party.inc
```

4. Inspect the latest audit log if lint output needs detail:

```sh
rtk tools/champions_partygen/partygen.sh audit list
rtk tools/champions_partygen/partygen.sh audit show --run RUN_ID
```

5. Review the diff:

```sh
rtk tools/champions_partygen/partygen.sh diff --input src/data/champions_partygen/trainers.party.inc --against src/data/trainers.party
```

6. Build and runtime-check:

```sh
rtk make -j16 -O all
rtk make -j16 -O debug
```

Use mGBA Live when generated party data changes battle behavior. A memory check of `gTrainers[DIFFICULTY_NORMAL][TRAINER_*]` is acceptable for confirming that trainerproc output reached ROM data.

The older direct replacement flow still exists for analysis branches:
`partygen apply --input FRAGMENT --target src/data/trainers.party --out OUT`.
Do not use it for the current 16.0 config-gated branch unless the branch
explicitly stops using the include gate.

## Catalog Editing

`journey.json` decides which trainers are generated.

Each journey trainer must point to:

- an existing `TRAINER_*` constant;
- a `blueprintId`.

Journey trainer entries may also include ownership tags. Use tags such as
`champions_challenge` and `partygen_owned` for trainer slots that this feature
is allowed to replace. These tags are not emitted to `trainers.party`; they are
written to audit logs for review.

Blueprints define:

- `partySize`: number of mons selected at runtime by Trainer Party Pool;
- `poolSize`: number of candidate mons materialized into `trainers.party`;
- `rulesetId`: existing pool rule, for example `POOL_RULESET_BASIC`;
- `mode`: `single` or `double`;
- `rank`: `early`, `mid`, `late`, or `champion`;
- `setGroups`: catalog-only pool groups that this blueprint may draw from;
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
- `groups`: catalog-only pool groups such as `pool.champion_demo`;
- `lintTags`: catalog-only tags for lint concepts such as `Weather Setter:
  Snow` or `Terrain Abuser: Electric`;
- `minRank` and `maxRank` for rank-band filtering;
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

Do not dump every role, archetype, weather, or terrain concept into `Tags`.
Roles, archetypes, and `lintTags` are tool vocabulary; `Tags` are ROM pool
selection vocabulary.

## Singles And Doubles

Keep singles and doubles as separate blueprint families.

Singles blueprints should usually require at most one `Lead` and one `Ace`.

Doubles blueprints should explicitly model:

- two lead slots;
- spread move availability;
- support / setup balance;
- partner anti-synergy;
- `Double Battle: Yes` in the trainer header.

Partygen now enforces header-vs-blueprint mode with `DBL001` during
`generate`. It also checks spread move presence for doubles pools when
`requireSpreadMove` is true.

The full lint design and implementation status (including doubles consistency
check ids `DBL001`-`DBL006`, rank band, weather / terrain / pledge battlefield
pairs, item duplication, cross-trainer checks, and the audit log format) lives in
`docs/features/champions_challenge/partygen_lint_spec.md`.

## Battlefield Pair Lint

`catalog/lint/battlefield_pairs.json` reserves matched setter / abuser pairs
for weather, terrain, and pledge-side effects. The file currently covers sun,
rain, sand, snow, Electric / Grassy / Misty / Psychic Terrain, Rainbow, Sea of
Fire, and Swamp.

Use `lintTags` for these concepts unless the tag must be emitted to ROM pool
selection. Example: a Glacia snow set can carry `lintTags: ["Weather Setter:
Snow", "Weather Abuser: Snow"]` while still emitting only `Lead / Support`.

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

For the full source map, including `gTrainerClasses`, encounter music, battle
BGM, victory BGM, mugshot color, and Leader / Champion battle background
effects, see
[Trainer Battle Reward and Audio Flow](../flows/trainer_battle_reward_audio_flow_v15.md).

## EXP

Partygen catalog data does not directly award or suppress EXP. The current
16.0 branch adds a ROM-side no-EXP config for the generated Lv50 challenge
path.

The current trainer multiplier is in `src/battle_script_commands.c`, `Cmd_getexp`:

```text
base = species.expYield * faintedLevel
base /= 5 or 7 depending on scaled EXP config
if trainer battle and B_TRAINER_EXP_MULTIPLIER <= GEN_7:
    base = base * 150 / 100
```

`include/config/battle.h` currently sets `B_TRAINER_EXP_MULTIPLIER` to `GEN_LATEST`.

When `B_CHAMPIONS_PARTYGEN_TRAINERS` is `1` and
`B_CHAMPIONS_PARTYGEN_EXP_MODE` is `B_CHAMPIONS_PARTYGEN_EXP_NONE`,
`Cmd_getexp` skips EXP for trainer battles. Wild battle EXP and normal builds
remain unchanged.

Challenge half-EXP, double-EXP, or profile-based EXP rules remain future
runtime work. Partygen may report expected EXP pressure, but only the explicit
ROM config should change actual EXP behavior.

## Player Style Logs

Player style logging is related to partygen because it can affect weights, but it should not be stored in `trainers.party`.

Implemented staged flow:

1. raw log: mGBA / debug / battle summary text under `tools/champions_partygen/local/logs/raw/`;
2. normalized log: JSONL rows built with `partygen logs normalize --input RAW --out LOGS.jsonl`;
3. profile summary: compact JSON built with `partygen profile build --input LOGS.jsonl --out PROFILE.json`;
4. generator input: catalog-pinned weights via `partygen generate --profile PROFILE.json`.

Do not put raw logs in SaveBlock. Profile input is opt-in; no-log generation
stays deterministic from `--seed`.

The full design and implementation status (file layout under
`tools/champions_partygen/local/`, JSONL schema, profile schema,
`logs normalize` / `profile build` / `generate --profile` CLI surface,
defaults for `minimum_adaptation_runs`, `exploration_rate`, and friends, and
privacy boundaries) lives in
`docs/features/champions_challenge/partygen_player_style_logging.md`.

## Review Checklist

Before committing generated trainer data:

- `partygen doctor` passes.
- `partygen validate` passes.
- `partygen diff` is reviewed.
- `Party Size` is present only for intended pool trainers.
- `B_CHAMPIONS_PARTYGEN_TRAINERS` is intentionally `1` or `0` for the
  branch being built.
- singles / doubles header matches intended blueprint.
- generated species, move, item, ability, and nature constants exist.
- `defaultExp` / set `exp` values are `normal` or `none`.
- prize money impact is understood for major trainers.
- EXP, badge-boost, and obedience config impact is understood for the branch.
- `rtk make -j16 -O all` and `rtk make -j16 -O debug` pass.
- mGBA Live confirms ROM data or battle behavior when party behavior changed.
