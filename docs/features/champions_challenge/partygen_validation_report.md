# Champions Partygen Validation Report

Date: 2026-05-06

Branch: `feature/trainer-party-generator`

## Scope

Implemented the first repo-local trainer party generator MVP at `tools/champions_partygen`.

Validated generated `TRAINER_WALLACE` output using Plan A replacement in `src/data/trainers.party`.

## Static Checks

Commands run:

```sh
rtk cargo test --manifest-path tools/champions_partygen/Cargo.toml
rtk cargo clippy --manifest-path tools/champions_partygen/Cargo.toml -- -D warnings
rtk tools/champions_partygen/partygen.sh doctor
rtk tools/champions_partygen/partygen.sh generate --seed 1234 --out /tmp/champions_trainers.party
rtk tools/champions_partygen/partygen.sh validate --input /tmp/champions_trainers.party
rtk tools/champions_partygen/partygen.sh diff --input /tmp/champions_trainers.party --against src/data/trainers.party
rtk git diff --check
rtk make -j4
rtk make debug -j4
rtk mdbook build docs
```

Results:

- `cargo test`: passed.
- `cargo clippy`: passed with `-D warnings`.
- `doctor`: passed; catalog found 1 journey trainer, 1 blueprint, 7 sets, and 855 source trainer blocks.
- `validate`: passed.
- `diff`: `TRAINER_WALLACE` changed, old fixed party 6 mons to generated pool 6 mons with `Party Size: 3`.
- `diff` after apply: `TRAINER_WALLACE: unchanged`.
- `git diff --check`: passed.
- `make`: passed.
- `make debug`: passed.
- `mdbook build docs`: completed and wrote HTML output; existing docs still report a missing `../CHANGELOG.md` include and an unexpected HTML end tag warning in `CREDITS.md`.

## Trainerproc Evidence

The generated Wallace block expands through `tools/trainerproc` to:

- `.partySize = 3`
- `.poolSize = 6`
- `.poolRuleIndex = POOL_RULESET_BASIC`
- generated species order:
  `SPECIES_MIGHTYENA`, `SPECIES_AGGRON`, `SPECIES_SKARMORY`, `SPECIES_ZIGZAGOON`, `SPECIES_METANG`, `SPECIES_WOBBUFFET`
- generated tags:
  Lead, Ace, Lead/Support, none, Lead/Support, Support

## mGBA Live Evidence

ROM copy: `/tmp/champions-partygen-mgba/pokeemerald-partygen.gba`

Session: `codex-partygen-check`

Check method: mGBA Live Lua read of ROM data at `gTrainers[DIFFICULTY_NORMAL][TRAINER_WALLACE]`.

Observed:

- `partySize`: 3
- `poolSize`: 6
- `poolRuleIndex`: 0 (`POOL_RULESET_BASIC`)
- party pointer: `0x08436FEC`
- species ids: 262, 306, 227, 263, 375, 202
- item ids: 523, 523, 472, 523, 523, 472
- ability ids: 22, 69, 5, 53, 29, 23
- levels: all 50
- tags: 1, 2, 17, 0, 17, 16

These match the generated `.party` fragment and trainer pool tag mapping.

Cleanup: `mgba-live-cli status --all` returned `[]` after stopping the session.
