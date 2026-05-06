# Champions Party Generator

`champions_partygen` materializes curated set catalog data into the existing
`trainers.party` DSL used by `tools/trainerproc`.

The MVP is intentionally build-adjacent rather than build-required:

- `generate` writes a copy-pasteable `.party` fragment.
- `validate` checks constants and trainerproc-facing syntax constraints.
- `diff` reports trainer blocks changed by a generated fragment.
- `apply` replaces matching trainer blocks in a target `.party` file when a
  branch is ready to commit data changes.

Examples:

```sh
tools/champions_partygen/partygen.sh doctor
tools/champions_partygen/partygen.sh scan
tools/champions_partygen/partygen.sh generate --seed 1234 --out /tmp/champions_trainers.party
tools/champions_partygen/partygen.sh validate --input /tmp/champions_trainers.party
tools/champions_partygen/partygen.sh diff --input /tmp/champions_trainers.party --against src/data/trainers.party
tools/champions_partygen/partygen.sh apply --input /tmp/champions_trainers.party --target src/data/trainers.party --out /tmp/trainers.party
```
