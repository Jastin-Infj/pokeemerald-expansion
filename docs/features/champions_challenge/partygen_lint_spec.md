# Champions Partygen Lint Spec

Date: 2026-05-06

Branch: `feature/trainer-party-generator`

Status: partially implemented in `tools/champions_partygen`

## Purpose

The original MVP `validate` step only checked constants existence, basic stat
shape, and pool tag whitelist. The current branch now implements the first
quality-lint pass, audit logs, catalog lint config files, and cross-trainer
drift checks so human review does not have to catch the same classes of
mistake by eye every time.

This doc fixes the lint surface, severities, and exit behavior for the current
implementation and the remaining checks that will follow as the catalog grows.

## Lint Layers

Lints split by what they need to read.

| Layer | Reads | Runs in |
|---|---|---|
| L0 schema | catalog JSON only | `doctor`, `generate` |
| L1 constants | catalog + `include/constants/*` | `doctor`, `generate` |
| L2 source-vs-blueprint | catalog + `src/data/trainers.party` headers | `doctor`, `generate` |
| L3 generated-pool | rendered `.party` fragment | `validate`, `generate` (pre-write), `apply` (pre-write) |
| L4 cross-trainer | aggregate over all generated trainers | `generate` |

L0 and L1 already exist in the MVP. The current branch implements the first
L2 / L3 / L4 pass.

## Severity Model

Three levels:

- `error` — exits non-zero. `apply` refuses to write. CI rejects.
- `warning` — exits zero by default; `--strict` promotes warnings to errors.
- `note` — informational only, never affects exit code.

Severity per check is fixed in this doc. Catalog cannot override severity.
A check that needs to be muted for a specific trainer must be expressed as
an explicit blueprint constraint flag (e.g. `allowSoloLead: true`).

## Doubles vs Singles

Goal: every generated trainer has a battle mode that matches the source
trainer's `Double Battle: Yes/No` header field, and doubles trainers carry
the structural pieces a doubles fight needs.

Required catalog additions:

- Blueprint gains `mode: "single" | "double"`. Catalog entries should set it
  explicitly; the CLI keeps a backward-compatible `"single"` default for old
  fixtures.
- Doubles blueprints can declare `requireSpreadMove: true` (default true).
- Sets gain `doublesSpreadMove: true | false` (default false). Used by L4.

### Checks

| ID | Layer | Severity | Description |
|---|---|---|---|
| `DBL001` | L2 | error | Source trainer has `Double Battle: Yes` but blueprint `mode == "single"`, or vice versa. |
| `DBL002` | L3 | error | Generated trainer has `Party Size: N` with `mode == "double"` but `N < 2`. |
| `DBL003` | L4 | error | Doubles pool with `requireSpreadMove == true` has zero sets that declare a spread move (Surf, Earthquake, Heat Wave, Discharge, Rock Slide, Muddy Water, Eruption, Blizzard, Dazzling Gleam, Make It Rain, ...; full versioned list lives in `tools/champions_partygen/catalog/lint/spread_moves.json`). |
| `DBL004` | L4 | warning | Doubles pool has fewer than 2 `Lead`-tagged sets. Doubles fights need 2 active mons on turn 1; one Lead is allowed (with explanation in catalog) but flagged. |
| `DBL005` | L4 | warning | Doubles pool ace count > 1. Doubles win-conditions usually share reward, multiple Aces split focus. |
| `DBL006` | L4 | note | Doubles pool ground-move user (Earthquake / Bulldoze) shares pool with non-Flying / non-Levitate / non-Air-Balloon partner candidates. Surface for review only; partner pairing is runtime, not pool-time. |

Mode mismatch in either direction is the main bug class this group catches.

## Rank Band / Power Budget

Goal: a Gym 1 trainer pool does not pull from the same set library as the
Champion. Without a band check, a fully-evolved Salamence in the Petalburg
trainer pool depends entirely on author memory.

Required catalog additions:

- Blueprint gains `rank: "early" | "mid" | "late" | "champion"`. Catalog
  entries should set it explicitly; the CLI keeps a backward-compatible `"mid"`
  default for old fixtures.
- Blueprint may set `setGroups: ["pool.foo"]` to restrict selection to sets
  whose catalog-only `groups` array intersects those values. This keeps
  Sidney / Phoebe / Champion libraries from leaking into each other when they
  share the same rank band.
- Set gains optional `minRank` and `maxRank` strings using the same band
  values. Default: `minRank = "early"`, `maxRank = "champion"`.
- Optional set field `bst` (base stat total override) for sets that come
  from a forme variant; defaults to species BST scraped from
  `src/data/pokemon/species_info/`.
- Optional blueprint field `bstBudget: {min: N, max: M, mean: K}` for hard
  band caps.

### Checks

| ID | Layer | Severity | Description |
|---|---|---|---|
| `RNK001` | L2 | error | Set's `[minRank, maxRank]` does not intersect blueprint `rank`. Materializing the set into the pool is rejected. |
| `RNK002` | L4 | error | Pool BST mean is outside blueprint `bstBudget.mean ± bstBudget.tolerance` (default tolerance 35). |
| `RNK003` | L4 | warning | Pool contains > 1 fully evolved pseudo-legendary BST ≥ 600 in `mid` blueprints. |
| `RNK004` | L3 | warning | A set's `Level` is more than `+5` over the source trainer's existing party top level on `mid` rank (catches accidental Lv60 mid-game). |
| `RNK005` | L4 | note | Pool contains zero fully evolved Pokemon for `late` or `champion` rank. Surface only; some champion gimmicks are intentional. |

Rank band data lives in catalog. The default species BST table is generated
once at startup by reading `src/data/pokemon/species_info/` and is cached
in-memory. There is no separate JSON to maintain because the species file is
already authoritative.

## Weather And Terrain Pair

Goal: a pool that sets weather but has no abuser is a dead concept; the
opposite (abusers with no setter) is also dead. Same for terrain.

### Checks

| ID | Layer | Severity | Description |
|---|---|---|---|
| `WTH001` | L4 | error | Pool has any set tagged `Weather Setter` but zero sets tagged `Weather Abuser`. |
| `WTH002` | L4 | error | Pool has any set tagged `Weather Abuser` but zero sets tagged `Weather Setter`. |
| `WTH003` | L4 | warning | Pool has setter / abuser of incompatible weathers (e.g. rain dance setter + Chlorophyll abuser). Compatibility table at `tools/champions_partygen/catalog/lint/weather_pairs.json`. |
| `WTH004` | L4 | warning | Pool has terrain setter (Electric / Grassy / Misty / Psychic Terrain user) and zero matched terrain abuser. Terrain has no dedicated tags, detection is by move/ability presence. |

`Weather Setter` and `Weather Abuser` are already in the engine pool tag
allowlist. The catalog uses them as-is; lint just enforces that they appear
in pairs.

## Item Duplication

Goal: a pool with three Life Orbs and two Choice Scarves looks generated.

### Checks

| ID | Layer | Severity | Description |
|---|---|---|---|
| `ITM001` | L3 | warning | Same non-`ITEM_NONE` item appears more than `itemDuplicationLimit` times in a single pool (default 1; common berries default 2). Originally specced as error; demoted to warning so the existing Hoenn demo Wallace (Sitrus x4) does not block default `generate`. `--strict` upgrades to error. |
| `ITM002` | L3 | warning | Pool relies on a single hard-counter item type (e.g. all Choice items). (Not yet implemented.) |
| `ITM003` | L3 | note | Pool uses an item explicitly listed in `tools/champions_partygen/catalog/lint/items_blocklist.json` (e.g. Mega Stones for species without a Mega form, Z-Crystals in non-Z-context). |

Items default to one per pool. Berries that the catalog explicitly marks as
`shareable` (Sitrus, Lum, Leftovers when intentional) raise the cap to 2.
Mega Stones and signature items always default to 1.

## Required Slot / Pool Health

Goal: catch dead pools that the deterministic selector would still happily
emit.

### Checks

| ID | Layer | Severity | Description |
|---|---|---|---|
| `SLT001` | L4 | error | Pool has zero `Lead`-tagged sets when blueprint `required` declares a `lead` slot. (MVP `select_sets` errors on selection failure; this lint catches it during `validate` without rerunning the selector.) |
| `SLT002` | L4 | error | Same for `ace`. |
| `SLT003` | L4 | warning | Pool size > 12 for a non-pool-rule trainer (`Party Size` not present). Currently allowed but probably accidental. |
| `SLT004` | L4 | warning | Pool has identical species to another set inside the same pool (e.g. two SPECIES_TAUROS sets) without `allowSpeciesDuplicate: true` in the blueprint. |

## Move Coverage (light)

Goal: not a full type-coverage solver; just guard against pools whose
attacking coverage is one type only.

### Checks

| ID | Layer | Severity | Description |
|---|---|---|---|
| `CVR001` | L4 | warning | Pool's combined offensive type set has fewer than 3 distinct types. |
| `CVR002` | L4 | note | Pool has zero status moves at all. Some hyper-offensive concepts are fine; surface only. |

Move-to-type table is read from `src/data/moves_info/`. No new JSON file is
required.

## Cross-Trainer Aggregate

Goal: spot generation drift across the whole journey, not just one trainer.

### Checks

| ID | Layer | Severity | Description |
|---|---|---|---|
| `XTR001` | L4 | warning | Same set id appears in N different generated pools (default N=3). Catalog set reuse is fine; over-reuse hides repetition. |
| `XTR002` | L4 | warning | Two adjacent journey trainers in the same journey stage share more than 50% of the smaller pool's set ids. Adjacency is `journey_index + 1` within the same `stageId`; rank band is not used for adjacency. |
| `XTR003` | L4 | note | Pool BST mean does not increase monotonically across journey rank. Surface only. |

## Audit Log Output

Generation accepts `--seed`. To make lint output reproducible and
trail-able, `partygen generate` writes an audit log:

- Default path: `tools/champions_partygen/local/audit/<run_id>.json`
  (gitignored).
- One file per `generate` invocation.
- Schema:

```jsonc
{
  "schemaVersion": 1,
  "runId": "2026-05-06T11-22-31Z-7f2b",
  "seed": 1234,
  "catalogPath": "tools/champions_partygen/catalog",
  "catalogHash": "fnv64:9a4e...",
  "profilePath": null,
  "profileStatus": null,
  "profileCatalogVersion": null,
  "profileWarnings": [],
  "trainers": [
    {
      "trainerConst": "TRAINER_WALLACE",
      "tags": ["champions_challenge", "partygen_owned"],
      "stageId": "stage.champion_demo",
      "journeyIndex": 0,
      "blueprintId": "blueprint.champion_demo",
      "rank": "champion",
      "mode": "single",
      "lints": [
        {"id": "WTH001", "severity": "error", "msg": "..."},
        {"id": "ITM001", "severity": "warning", "msg": "..."}
      ],
      "selections": [
        {"slot": "lead", "setId": "set.mightyena.intimidate", "rule": "required", "weight": 1.0},
        {"slot": "ace", "setId": "set.aggron.physical", "rule": "required", "weight": 1.0},
        {"slot": "fill", "setId": "set.skarmory.spdef", "rule": "preferred:role.support_pivot", "weight": 1.2}
      ]
    }
  ]
}
```

`partygen audit show --run RUN_ID` prints this in human-readable form.
This audit log is also the surface that the player style logging design
(`partygen_player_style_logging.md`) writes profile-driven weight changes
into. Both designs share the same file format on purpose.

## Configuration

Lint behavior is controlled in `config.example.toml` under a `[lint]`
section. Defaults shown:

```toml
[lint]
strict = false
item_duplication_limit = 1
shareable_items = ["ITEM_LUM_BERRY", "ITEM_LEFTOVERS", "ITEM_SITRUS_BERRY"]
bst_budget_tolerance = 35
xtr_set_reuse_threshold = 3
spread_moves_path = "catalog/lint/spread_moves.json"
weather_pairs_path = "catalog/lint/weather_pairs.json"
items_blocklist_path = "catalog/lint/items_blocklist.json"
```

`config.local.toml` may override `[lint]` for personal experiments. The
defaults are conservative; tuning down lints in catalog pull requests should
require a justification line in the PR description.

## Exit Behavior Summary

| Command | Behavior |
|---|---|
| `partygen doctor` | runs L0+L1 only; exits non-zero on any error. |
| `partygen validate` | runs generated-fragment validation against constants; exits non-zero on any error. With `--strict`, also non-zero on any warning. |
| `partygen generate` | runs L0+L1+L2+L3+L4; writes audit log; exits non-zero on any error. With `--strict`, also non-zero on any warning. With `--lint-only`, skips writing the .party output and only writes the audit log. |
| `partygen apply` | re-runs L3 on the generated input before writing. Refuses to apply on any error regardless of `--strict`. |
| `partygen audit show` | reads existing audit log; never produces lint output of its own. |

## CLI Surface

| Command / Flag | Status | Purpose |
|---|---|---|
| `partygen generate --strict` | new flag | promote warnings to errors |
| `partygen generate --lint-only` | new flag | run lint without writing .party |
| `partygen generate --audit-out PATH` | new flag | redirect audit log path |
| `partygen audit show --run RUN_ID` | new | print audit log human-readably |
| `partygen audit list` | new | list run ids in the audit dir |
| `partygen lint` | optional | thin alias for `validate --strict` |

## Implementation Status

Implemented now:

- severity model and audit log schema;
- `DBL001`-`DBL005`;
- rank-band hard filtering through `minRank` / `maxRank`;
- `WTH001` / `WTH002`;
- `ITM001` / `ITM003`;
- `SLT001` / `SLT002` / `SLT004`;
- lightweight `CVR001` using catalog offensive type archetypes;
- `XTR001` and `XTR002`;
- `schemaVersion` enforcement for `spread_moves.json`,
  `weather_pairs.json`, and `items_blocklist.json`.

Still planned:

- BST budget checks (`RNK002`-`RNK005`);
- weather / terrain compatibility beyond the basic setter-abuser pair;
- `DBL006`, `ITM002`, `SLT003`, `CVR002`, and `XTR003`.

## Test Plan (for the implementation PR)

- Unit tests per check id with a tiny catalog fixture that only triggers that
  check.
- Golden test: a known-good Hoenn demo generates with zero errors and a
  documented warning count.
- Negative test: a deliberately broken catalog (rain setter only, identical
  items, mode mismatch) produces the expected list of error ids in stable
  order.
- Audit log determinism: same seed, same catalog hash, no profile → byte
  identical audit log.

## Resolved Decisions

- The lint and player-style work stay in one implementation branch for now
  because they share audit log fields.
- `XTR002` adjacency is stage-local journey order, not rank-band adjacency.
- `spread_moves.json` is versioned with `schemaVersion`, and partygen rejects
  unsupported lint config schema versions instead of silently falling back.

## Open Questions

- Should L2 / L3 / L4 also run inside `apply`, or is re-running `validate`
  before `apply` enough? Running everything inside `apply` makes apply slower
  but harder to bypass; running only `validate` makes apply fast but
  trustable only if reviewers actually ran validate.
- Should `BST` be hard-cached at first read or recomputed on every run?
  Hard-cached needs an invalidation rule when species_info changes.
- Should the `items_blocklist` ship with curated entries (Mega Stones for
  non-mega species), or stay empty until evidence of misuse?
- Is there value in adding a `partygen lint diff` that compares two audit
  logs (e.g. "what changed about lint counts since the last seed")?
