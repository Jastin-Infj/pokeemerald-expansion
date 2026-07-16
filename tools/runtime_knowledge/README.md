# Runtime Knowledge Catalog Tools

This directory contains offline tooling for the Smart Gimmick AI knowledge
catalog.

The catalog is intentionally generated from the local pokeemerald-expansion
runtime first. Pokemon Wiki, VGC usage, Champions usage, and trainer-party JSON
are reference or inference sources, but local source and config gates decide
what this branch can actually do.

## Phase 1: local runtime catalog

Run:

```sh
cargo run --manifest-path tools/runtime_knowledge/Cargo.toml -- --out /tmp/runtime_knowledge_catalog --pretty
```

The command writes:

- `moves.json`: move constants, move info fields, move-shape tags, AI move
  knowledge flags, and generation-gated flag expressions.
- `abilities.json`: ability constants plus AI ability knowledge flags parsed
  from `AI_GetAbilityKnowledgeFlags`.
- `hold_effects.json`: hold-effect constants plus AI hold-effect knowledge
  flags parsed from `AI_GetHoldEffectKnowledgeFlags`.
- `items.json`: item constants, hold effects / params, and inherited item
  knowledge flags.
- `gimmicks.json`: the current high-level resource policy for Mega / Z-Move /
  Dynamax / Tera arbitration.
- `summary.json`: counts and source file references.

The generated files are review artifacts. Do not commit generated JSON unless a
feature explicitly needs a checked-in fixture.

## Later phases

The next phases should add adapters that merge this local catalog with:

- Champions PartyGen output.
- VGC / official tournament usage snapshots.
- Pokemon Wiki category audits.
- Observed in-battle history.

Those adapters must keep source and confidence tags. Inferred information is
not omniscient information.
