# Future Runtime Handoff: Field Notes / Lore Codex

FUTURE USE ONLY.

```text
runtime implementation task.

Goal:
Implement a text-only Field Notes / Lore Codex MVP from a safe NPC or test
object. Use 3 fixed entries. Do not add save data, unlock state, or PokeNav
integration in the first slice.

branch:
feature/field-notes-codex-mvp

Docs:
- docs/features/field_notes_codex/*

Validation:
rtk mdbook build docs
rtk make -j16 -O all
rtk make -j16 -O debug
rtk make -j16 -O check

mGBA:
- Open viewer.
- Select all 3 entries.
- Close with B.
- Reopen.
- Confirm no text/window artifacts or lockup.
```
