# Champions Partygen Player Style Logging

Date: 2026-05-06

Branch: `feature/trainer-party-generator`

Status: partially implemented in `tools/champions_partygen`

## Purpose

Capture how the player actually plays so partygen can weight set selection
toward fights that stay challenging without becoming unfair. The pipeline is
explicitly staged so generation never blocks on logs and so noisy or stale data
cannot silently change trainer rosters.

The same staging also gives later features (Champions Challenge runtime, scout
selection, adaptive intensity) a single canonical source for player profile
data instead of each feature reading raw mGBA logs on its own.

## Pipeline

```
raw mGBA / debug log  -->  normalized JSONL  -->  player_profile.json  -->  partygen weights
       (tool 1)              (tool 2)              (tool 3)               (tool 4)
```

Each step is a separate CLI subcommand under `partygen`. They run in order; the
output of one is the input of the next. There is no daemon and no in-process
shared state.

### Stage 1. Raw log

- Source: mGBA Lua hook script that writes one line per significant battle
  event. The script lives at `tools/champions_partygen/lua/battle_log.lua` and
  is not loaded automatically; the player or tester opts in via the existing
  mGBA Live MCP `mgba_live_start_with_lua` flow.
- Output path: `tools/champions_partygen/local/logs/raw/<utc_date>/<run_id>.log`
  (gitignored under `tools/champions_partygen/.gitignore` `local/`).
- Format: line-oriented text. Each line is `timestamp\tevent\tkey=value\t...`.
  Plain text on purpose so the file stays diffable and survives partial writes.
- Recommended events:
  - `battle_start` — battle id, opponent trainer id, format (single/double),
    player party species/level/item.
  - `turn_start` — turn number.
  - `move_used` — actor side, slot, move id.
  - `switch` — actor side, from species, to species.
  - `item_used` — actor side, item id, target slot.
  - `faint` — side, slot, species.
  - `battle_end` — outcome (win/loss/forfeit/draw), turns, surviving party.

Raw logs are not read by `partygen generate`. They exist only as the source
for the normalize step and as a debug artifact. The canonical committed script
stays under the partygen tree; mGBA Live tooling should load this file rather
than carrying a second copy.

### Stage 2. Normalize

- Command: `partygen logs normalize --input <raw_path_or_dir> --out <jsonl_path>`.
- Input: one raw log file or a directory of them. Multiple raw files for the
  same `run_id` collapse into a single JSONL stream.
- Output: JSON Lines, one row per battle, schema below.
- Validation: the current implementation parses and preserves logged tokens.
  Constant validation for species / move / item / ability / trainer ids is
  still planned.
- The normalize step never writes back to raw. It is idempotent: rerunning over
  the same raw produces a byte-identical JSONL (deterministic ordering by
  `run_id`, `battle_id`).

JSONL row schema, one row per battle:

```jsonc
{
  "schemaVersion": 1,
  "runId": "2026-05-06T10-22-31Z-abc123",
  "battleId": 17,
  "trainerConst": "TRAINER_WALLACE",
  "format": "single",                 // "single" | "double"
  "playerParty": [
    {"species": "SPECIES_BLAZIKEN", "level": 50, "item": "ITEM_LIFE_ORB",
     "moves": ["MOVE_FLARE_BLITZ", "MOVE_CLOSE_COMBAT", "MOVE_PROTECT", "MOVE_THUNDER_PUNCH"]}
  ],
  "outcome": "win",                   // "win" | "loss" | "forfeit" | "draw"
  "turns": 12,
  "playerFaintOrder": ["SPECIES_BLAZIKEN"],
  "opponentFaintOrder": ["SPECIES_MIGHTYENA", "SPECIES_AGGRON", "SPECIES_SKARMORY"],
  "switchesByPlayer": 2,
  "switchesByOpponent": 1,
  "itemsUsedByPlayer": ["ITEM_HYPER_POTION"],
  "keyMoves": ["MOVE_DRAGON_DANCE", "MOVE_PROTECT"],   // moves that flipped tempo
  "warnings": ["unknown:MOVE_PHANTOM_FORCE_v2"]
}
```

`schemaVersion` exists from day one so that schema bumps can run in parallel
with old normalized files without breaking the profile build step.

### Stage 3. Profile build

- Command: `partygen profile build --input <jsonl_path_or_dir> --out <profile_path>`.
- Input: one or more JSONL files.
- Output: `player_profile.json`. Default path `tools/champions_partygen/local/profiles/active.json`.
- Aggregation rules (MVP):
  - `winRate`: total wins / total battles.
  - `archetypeWinRate`: per opponent archetype tag (`weather_neutral`,
    `weather_rain`, `tr_setup`, ...) → win rate. Source archetypes come from the
    catalog set the trainer drew from, looked up by `trainerConst`.
  - `repeatedWeaknesses`: ordered list of opponent archetypes where win rate is
    below `weaknessThreshold` and battle count is above `minSampleCount`.
  - `preferredTempo`: median turns to win across `outcome == "win"` battles.
  - `switchHabit`: median `switchesByPlayer` across all battles.
  - `itemReliance`: total `itemsUsedByPlayer` per battle, averaged.
- Output includes a `confidence` block with `totalBattles`, `oldestBattleAt`,
  `newestBattleAt`. Generator weighting is only applied if
  `totalBattles >= minimum_adaptation_runs` (default 25, documented in
  `config.example.toml`; full TOML loading is still planned).

`player_profile.json` shape (MVP):

```jsonc
{
  "schemaVersion": 1,
  "catalogVersion": "fnv64:385fe0c879672609",
  "generatedAt": "2026-05-06T11:04:55Z",
  "confidence": {
    "totalBattles": 120,
    "oldestBattleAt": "2026-04-09T18:30:00Z",
    "newestBattleAt": "2026-05-06T10:55:12Z"
  },
  "winRate": 0.72,
  "archetypeWinRate": {
    "weather_neutral": 0.81,
    "weather_rain": 0.42,
    "tr_setup": 0.55
  },
  "repeatedWeaknesses": ["weather_rain"],
  "preferredTempo": 9,
  "switchHabit": 1.0,
  "itemReliance": 0.4
}
```

The profile file never contains:

- player name,
- save slot,
- in-game money,
- map / coordinate data,
- raw move-by-move sequences (those stay in JSONL).

This keeps the profile small, easy to share between testers, and free of save
state that would tie it to a single play file.

### Stage 4. Weight feedback into partygen

- New flag: `partygen generate --profile <profile_path>`.
- Behavior when the flag is omitted: identical to current MVP, fully
  deterministic from `--seed`. No silent profile reads.
- Behavior when the flag is set:
  - profile is loaded;
  - if `confidence.totalBattles < minimum_adaptation_runs`, the profile is
    logged ("profile under threshold, weights skipped") and selection runs
    deterministically;
  - if `catalogVersion` does not match the current catalog hash, the profile
    is treated as stale and weights are skipped;
  - otherwise weights are applied as soft modifiers on top of the existing
    `select_sets` flow:
    - sets whose archetypes match `repeatedWeaknesses` get a bounded weight
      bonus (`weakness_bonus`, default 1.5x within the same role);
    - cooldown penalties are not applied yet; selected set ids and weights are
      written to audit logs so cooldown can be reconstructed without a
      persistent local state file;
    - `exploration_rate` (default 0.2) is the probability that the
      profile-aware pick is overridden by the deterministic pick. This keeps
      seed-only reruns close to profile runs and prevents collapse.
- The applied weight modifiers are written into the audit log (see
  `partygen_lint_spec.md` for the audit log format) so that any unexpected
  party can be traced back to the profile snapshot that drove it.

## File Locations And Commit Policy

```
tools/champions_partygen/
  lua/
    battle_log.lua                # mGBA Lua hook (committed reference script)
  local/                          # gitignored
    logs/
      raw/<utc_date>/<run_id>.log
      normalized/<run_id>.jsonl
    profiles/
      active.json
  profiles/
    example_profile.json          # committed fixture for tests / docs
```

`tools/champions_partygen/.gitignore` ignores `target/`, `generated/`,
`local/`, and `config.local.toml`. Raw and normalized logs must not be committed.
A small fictional `example_profile.json` may be committed under `profiles/`
so docs can show real shape.

`config.example.toml` includes a `[profile]` section documenting the defaults:

```toml
[profile]
path = "local/profiles/active.json"
minimum_adaptation_runs = 25
weakness_bonus = 1.5
archetype_cooldown = 3
exploration_rate = 0.2
weakness_threshold = 0.5
min_sample_count = 5
profile_max_age_days = 60
```

`config.local.toml` overrides remain personal and are never committed.

## CLI Surface

| Command | Status | Purpose |
|---|---|---|
| `partygen logs normalize` | implemented | raw mGBA log → JSONL |
| `partygen profile build` | implemented | JSONL → player_profile.json |
| `partygen profile show` | implemented | print summary of a profile file |
| `partygen profile diff --before X --after Y` | implemented | compare two snapshots |
| `partygen generate --profile PATH` | implemented | apply catalog-pinned profile weights |
| `partygen explain --profile PATH --trainer T` | implemented | show profile status and selected weights for one trainer |
| `partygen audit show --run RUN_ID` | implemented | print audit log for a generate run (cross-references `partygen_lint_spec.md`) |

All log / profile commands must run with `partygen` doctor-clean repo state;
they do not require constants beyond what `doctor` already loads.

## Failure Modes And Safeguards

- Missing raw log: `logs normalize` returns a clear error and exit non-zero;
  it never silently produces an empty JSONL.
- Corrupt log line: the offending line is written to a sibling
  `<run_id>.errors.log` and the normalize step continues. Counts of dropped
  lines surface in stdout.
- Profile age: if `newestBattleAt` is older than `profile_max_age_days`
  (default 60), `generate --profile` should warn and fall back to deterministic
  selection. This remains planned because normalized rows do not yet retain a
  canonical battle timestamp.
- Profile / catalog mismatch: if the profile references archetype tags that no
  longer exist in the catalog, those entries are reported as profile warnings
  in stdout and the audit log. The profile schema pins `catalogVersion`, so a
  changed catalog hash disables weights until the profile is rebuilt.
- mGBA crash mid-battle: `battle_end` is missing for that battle, normalize
  marks the row as `outcome: "incomplete"` and excludes it from profile
  aggregates by default.

## Privacy / Sharing Boundaries

- Raw logs and JSONL stay in `local/`. They are excluded from commits.
- `player_profile.json` may be shared between testers as a deliberate review
  artifact. The schema is intentionally aggregate-only so two testers swapping
  profiles cannot reconstruct each other's run history.
- No profile data is written into SaveBlock.
- No profile data is written into ROM.

## Test Plan (for the implementation PR)

- Unit tests for the raw log parser (well-formed lines, malformed lines,
  partial last line, mixed CRLF).
- Unit tests for the normalize step using an in-repo fixture (a tiny
  hand-written `raw/example.log` plus the expected JSONL).
- Unit tests for the profile aggregator using a fixture JSONL that exercises
  weakness threshold, sample count gate, tempo median, switch median.
- Integration test: `partygen generate --profile fixture --seed 1234` with a
  fixture profile that pushes one weakness should select the matching set
  more often than baseline across a small sample (`run N times, count`),
  while staying within `exploration_rate` of the deterministic baseline.
- Integration test: profile under `minimum_adaptation_runs` produces output
  byte-identical to `generate --seed 1234` without the flag.

## Resolved Decisions

- `battle_log.lua` lives under `tools/champions_partygen/lua/`; mGBA Live
  should load that canonical script.
- Profiles pin `catalogVersion` using the same stable `fnv64` catalog hash
  written to audit logs. `profile_max_age_days` remains a later timestamp gate,
  not the primary stale-catalog guard.
- `archetype_cooldown` is audit-only for the current implementation. A
  persistent `local/profiles/cooldown.json` can be added later if reconstructing
  from audit logs becomes too slow.
- Champions Challenge profile data should use an explicit separate file such
  as `local/profiles/challenge_profile.json`; partygen does not silently merge
  casual-play and challenge-play profiles.

## Open Questions

- Should `exploration_rate` default to 0.2 globally, or scale per trainer
  rank (Gym vs route trainer)?
- Should `partygen profile build` accept multiple users' JSONL and emit a
  per-user profile fan-out, or stay single-user-only for MVP?
- Is mGBA the only allowed log source, or should we leave room for a future
  capture from a different emulator (no current plan, but the Lua schema is
  emulator-agnostic in principle)?
