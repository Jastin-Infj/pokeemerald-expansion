# Route Mastery Passport Investigation

## Questions

- How are trainer defeat flags represented?
- How are item ball flags represented?
- How are hidden item flags represented?
- Is there a simple current map section ID?
- Which route is safest for MVP?
- Can visited rely on existing flags or should it be omitted in first slice?

## Source Search Notes

Read-only search commands used during docs-only investigation:

```sh
rg "FLAG_.*TRAINER|trainerflag|itemball|hidden item|MAP_|GetCurrentMap|visited|RegionMap|mapsec" src include data/maps data/scripts
rg "FLAG_HIDE|ITEMBALL|hiddenitem|trainerbattle" data/maps data/scripts | head -n 120
```

## Candidate Symbols / Areas

| Area | Candidate file / symbol | Confidence | Notes |
|---|---|---|---|
| Trainer flags | `data/scripts/set_gym_trainers.inc` / `settrainerflag`; `data/scripts/trainer_battle.inc` | Medium | Trainer flag commands exist, but route-specific ownership needs careful investigation. |
| Trainer battles | `data/maps/*/scripts.inc` / `trainerbattle_*` | High | Route scripts include trainer battle definitions. |
| Object hide flags | `FLAG_HIDE_*` in `data/scripts/*.inc` and map scripts | High | Useful for NPC / object ownership but not automatically route mastery. |
| Item delivery / object examples | map `scripts.inc` files | Medium | Item balls / hidden items need route-specific flag audit before use. |
| Map constants | `MAP_*` constants in scripts | Medium | Can identify candidate route maps; exact current-map API must be investigated in runtime branch. |

## Open Questions

- Do not assume trainer rematch flags mean "route cleared".
- Hidden item flags need route-specific manual audit.
- Full region coverage is out of scope.
