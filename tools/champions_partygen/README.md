# Champions Party Generator

`champions_partygen` materializes curated set catalog data into the existing
`trainers.party` DSL used by `tools/trainerproc`.

The MVP is intentionally build-adjacent rather than build-required:

- `generate` writes a reviewable `.party` fragment for the generated include.
- `validate` checks constants and trainerproc-facing syntax constraints.
- `diff` reports trainer blocks changed by a generated fragment.
- `apply` replaces matching trainer blocks in a target `.party` file when a
  branch is ready to commit data changes.

The default catalog owns Champions Challenge trainer slots only. Add
journey-level tags such as `champions_challenge` and `partygen_owned` when a
trainer block is intentionally managed by partygen; the tags are recorded in
audit logs and are not emitted to `trainers.party`.

Blueprint `setGroups` and set `groups` are catalog-only filters. Use them to
keep one trainer's pool from drawing sets intended for another trainer.

Set `lintTags` are also catalog-only. Use them for weather, terrain, and
pledge-side concepts that should be linted but should not be emitted to
trainerproc `Tags`.

The default catalog currently owns the Elite Four run-up
(`TRAINER_SIDNEY` through `TRAINER_DRAKE`), the Tate & Liza double-battle
gimmick demo slot, and the Wallace demo slot. The ROM uses this data when
`B_CHAMPIONS_PARTYGEN_TRAINERS` is enabled in
`include/config/champions_partygen.h`; the current PartyGen branch commits that
flag enabled so the generated data is the active validation target.

Lv50 challenge catalogs may set top-level `"defaultExp": "none"` and per-set
`"exp": "normal"` or `"exp": "none"`. The tool validates this metadata. Actual
ROM EXP suppression is controlled by `B_CHAMPIONS_PARTYGEN_EXP_MODE`; trainer
battle EV suppression is controlled by `B_CHAMPIONS_PARTYGEN_EV_MODE`.

Set JSON may also emit trainerproc gimmick fields:

- Mega Evolution and Z-Moves are represented by held items such as
  `ITEM_GARDEVOIRITE` or `ITEM_PSYCHIUM_Z`.
- `dynamaxLevel` emits `Dynamax Level`.
- `gigantamax: true` emits `Gigantamax: Yes`.
- `teraType` emits `Tera Type`.

The tool and trainerproc allow combination test data, for example
Mega+Dynamax+Tera or Z-Move+Dynamax+Tera. Actual battle-time gimmick choice
still follows the ROM's existing runtime priority and item restrictions.

Examples:

```sh
tools/champions_partygen/partygen.sh doctor
tools/champions_partygen/partygen.sh scan
tools/champions_partygen/partygen.sh generate --seed 1234 --out src/data/champions_partygen/trainers.party.inc
tools/champions_partygen/partygen.sh validate --input src/data/champions_partygen/trainers.party.inc
tools/champions_partygen/partygen.sh diff --input src/data/champions_partygen/trainers.party.inc --against src/data/trainers.party
tools/champions_partygen/partygen.sh apply --input /tmp/champions_trainers.party --target src/data/trainers.party --out /tmp/trainers.party
tools/champions_partygen/partygen.sh audit list
tools/champions_partygen/partygen.sh logs normalize --input tools/champions_partygen/tests/fixtures/raw_log_example.log --out /tmp/partygen_logs.jsonl
tools/champions_partygen/partygen.sh profile build --input /tmp/partygen_logs.jsonl --out /tmp/player_profile.json
tools/champions_partygen/partygen.sh generate --seed 1234 --profile tools/champions_partygen/profiles/example_profile.json --out /tmp/champions_trainers.party
```
