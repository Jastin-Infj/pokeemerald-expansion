# Trainer Titles / Achievement Badges Investigation

## Questions

- Existing Trainer Card code location?
- Existing event flags suitable for one-time achievements?
- Existing NPC scripts that check flags and display text?
- How to avoid needing a current-title save field?
- Which achievements can be represented by existing flags?

## Source Search Notes

Read-only search commands used during docs-only investigation:

```sh
rg "Trainer Card|trainer card|setflag|checkflag|achievement|title|PLAYER|StringCopy|nickname" src include data/maps data/scripts
rg "TrainerCard|trainer_card|card" src include | head -n 120
```

## Candidate Symbols / Areas

| Area | Candidate file / symbol | Confidence | Notes |
|---|---|---|---|
| Trainer Card API | `include/trainer_card.h` / `ShowPlayerTrainerCard`, `GetTrainerCardStars` | High | Existing Trainer Card exists, but MVP should avoid modifying it. |
| Trainer Card implementation | `src/trainer_card.c` | High | Heavy UI / save-facing area; not MVP. |
| Start menu Trainer Card entry | `src/start_menu.c` | Medium | Existing start menu opens Trainer Card; do not hook for MVP. |
| Flag commands | `src/scrcmd.c` / `ScrCmd_setflag`, `ScrCmd_checkflag` | High | Good for one-time title ownership. |
| NPC scripts | `data/maps/*/scripts.inc`, `data/scripts/*.inc` | High | Existing flag-check and text-display patterns are common. |

## Open Questions

- Current title selection requires persistent state and is not MVP unless a safe existing flag-only model is chosen.
- Real achievement conditions should be tied to existing flags only, not invented as fake checks.
