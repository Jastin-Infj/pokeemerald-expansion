# Champions Partygen Validation Report

## 2026-06-03 16.0 Config-Gated Port Addendum

Branch: `feature/champions-partygen-16-20260603`

Scope:

- Ported `tools/champions_partygen` and catalog data onto the current 16.0
  master baseline.
- Added config-gated generated trainer include:
  `src/data/champions_partygen/trainers.party.inc`.
- Added `include/config/champions_partygen.h` as the single PartyGen runtime /
  data config owner, included by `include/config/battle.h` for C runtime use
  and by `src/data/trainers.party` for trainerproc preprocessing.
- Added no-EXP / badge boost / obedience config knobs for the Lv50 challenge
  path.

Static and tool checks:

```sh
rtk cargo test --manifest-path tools/champions_partygen/Cargo.toml
rtk tools/champions_partygen/partygen.sh doctor
rtk tools/champions_partygen/partygen.sh validate --input src/data/champions_partygen/trainers.party.inc
rtk tools/champions_partygen/partygen.sh diff --input src/data/champions_partygen/trainers.party.inc --against src/data/trainers.party
rtk git diff --check
rtk mdbook build docs
```

Results:

- `cargo test`: passed, 17 tests.
- `doctor`: passed; catalog found 5 journey trainers, 5 blueprints, 31 sets,
  and 855 source trainer blocks.
- `validate`: passed with 0 errors, 0 warnings, 0 notes.
- `diff`: Sidney, Phoebe, Glacia, Drake changed from fixed 5 mons to generated
  6-mon pools; Wallace changed from fixed 6 mons to a generated 6-mon pool with
  `Party Size: 3`.
- `git diff --check`: passed.
- `mdbook build docs`: passed with existing warnings for missing root
  `CHANGELOG.md` include, `CREDITS.md` `</img>`, and large search index.

Trainerproc / config-gate checks:

- Default config (`B_CHAMPIONS_PARTYGEN_TRAINERS = 0`) preprocesses
  `src/data/trainers.party` and trainerproc emits the vanilla Sidney and Wallace
  fixed parties.
- A temporary `/tmp` override with `B_CHAMPIONS_PARTYGEN_TRAINERS = 1`
  preprocesses and trainerproc emits generated Trainer Party Pool data:
  Sidney has `.partySize = 5`, `.poolSize = 6`; Wallace has `.partySize = 3`,
  `.poolSize = 6`.
- This check was done without changing repo source by overriding
  `include/config/champions_partygen.h` from `/tmp/partygen-true-include`.

Build / test checks:

```sh
rtk make -j16 -O check TESTS=test/battle/exp.c
rtk make -j16 -O all
rtk make -j16 -O debug
rtk make -j16 -O check
```

Results:

- Focused EXP check: passed.
- Normal ROM build: passed with existing RWX linker warning.
- Debug ROM build: passed with existing RWX linker warning.
- Full check: passed with existing `EXPECTED_FAIL`, `KNOWN_FAILING`, and
  crash-resume test-runner markers.

mGBA Live evidence:

- MCP start first failed because Qt had no `DISPLAY`.
- CLI start with `DISPLAY=:0` succeeded using the script-capable mGBA build:
  `/home/jastin/dev/pokeemerald-expansion/.cache/mgba-script-build-master/qt/mgba-qt`.
- Boot screenshot: `/tmp/partygen_16_boot.png`.
- START input was accepted and the next screenshot reached the title demo:
  `/tmp/partygen_16_after_start.png`.
- `mgba-live-cli stop` returned `alive_after: false`, and
  `mgba-live-cli status --all` returned `[]`.

Remaining validation boundary:

- The committed normal ROM keeps `B_CHAMPIONS_PARTYGEN_TRAINERS = 0`, so mGBA
  runtime evidence confirms boot/input on the default build. Generated pool
  expansion for `B_CHAMPIONS_PARTYGEN_TRAINERS = 1` is confirmed through
  CPP/trainerproc output, not through a separate enabled ROM build in this
  pass.

---

Date: 2026-05-06

Branch: `feature/trainer-partygen-catalog-expansion`

## Scope

Implemented the first repo-local trainer party generator MVP at `tools/champions_partygen`.

Validated generated `TRAINER_SIDNEY`, `TRAINER_PHOEBE`, `TRAINER_GLACIA`,
`TRAINER_DRAKE`, and `TRAINER_WALLACE` output using Plan A replacement in
`src/data/trainers.party`.

## Static Checks

Commands run:

```sh
rtk cargo test --manifest-path tools/champions_partygen/Cargo.toml
rtk cargo clippy --manifest-path tools/champions_partygen/Cargo.toml -- -D warnings
rtk tools/champions_partygen/partygen.sh doctor
rtk tools/champions_partygen/partygen.sh generate --seed 1234 --out /tmp/champions_trainers.party
rtk tools/champions_partygen/partygen.sh validate --input /tmp/champions_trainers.party
rtk tools/champions_partygen/partygen.sh diff --input /tmp/champions_trainers.party --against src/data/trainers.party
rtk tools/champions_partygen/partygen.sh profile show --input tools/champions_partygen/profiles/example_profile.json
rtk tools/champions_partygen/partygen.sh generate --seed 1234 --profile tools/champions_partygen/profiles/example_profile.json --out /tmp/champions_trainers.party
rtk git diff --check
rtk make -j4
rtk make debug -j4
rtk mdbook build docs
```

Results:

- `cargo test`: passed.
- `cargo clippy`: passed with `-D warnings`.
- `doctor`: passed; catalog found 5 journey trainers, 5 blueprints, 31 sets, and 855 source trainer blocks.
- `validate`: passed.
- `diff`: `TRAINER_SIDNEY`, `TRAINER_PHOEBE`, `TRAINER_GLACIA`, and
  `TRAINER_DRAKE` changed from fixed 5 mons to generated pool 6 mons with
  `Party Size: 5`; `TRAINER_WALLACE` was unchanged.
- `diff` after apply: Sidney, Phoebe, Glacia, Drake, and Wallace all
  `unchanged`.
- `profile show`: passed with `catalog_version: fnv64:385fe0c879672609`.
- `generate --profile`: passed; still reports the documented Wallace `ITM001`
  warning.
- `git diff --check`: passed.
- `make`: passed.
- `make debug`: passed.
- `mdbook build docs`: completed and wrote HTML output; existing docs still
  report a missing `../CHANGELOG.md` include, an unexpected HTML end tag
  warning in `CREDITS.md`, and a large search index warning.

## Trainerproc Evidence

The generated Elite Four blocks expand through `tools/trainerproc` to:

- `.partySize = 5`
- `.poolSize = 6`
- `.poolRuleIndex = POOL_RULESET_BASIC`
- trainer headers preserved for class, pic, gender, music, items, AI, and
  mugshot fields
- emitted `Tags:` stay within the trainerproc pool tag allowlist; weather /
  terrain / pledge concepts stay in catalog-only `lintTags`

## mGBA Live Evidence

ROM copy: `/tmp/champions-partygen-e4-mgba/pokeemerald-partygen-e4.gba`

Session: `codex-partygen-e4-check`

Check method: mGBA Live Lua read of ROM data at
`gTrainers[DIFFICULTY_NORMAL][TRAINER_SIDNEY..TRAINER_DRAKE]`.

Observed:

- Sidney: `partySize=5`, `poolSize=6`, `poolRuleIndex=0`, species ids
  `262, 359, 332, 342, 275, 319`.
- Phoebe: `partySize=5`, `poolSize=6`, `poolRuleIndex=0`, species ids
  `354, 429, 356, 354, 477, 302`.
- Glacia: `partySize=5`, `poolSize=6`, `poolRuleIndex=0`, species ids
  `460, 365, 478, 362, 362, 364`.
- Drake: `partySize=5`, `poolSize=6`, `poolRuleIndex=0`, species ids
  `334, 373, 230, 372, 330, 445`.

These match the generated `.party` fragment and trainer pool tag mapping.

Cleanup: `mgba-live-cli status --all` returned `[]` after stopping the session.
