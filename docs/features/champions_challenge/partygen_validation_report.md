# Champions Partygen Validation Report

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
