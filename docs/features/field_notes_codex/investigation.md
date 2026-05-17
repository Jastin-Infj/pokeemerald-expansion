# Field Notes / Lore Codex Investigation

## Questions

- Which existing text viewer / menu patterns are safest?
- Is list menu support sufficient for a title list?
- Is scrollable text needed for MVP?
- Which entry point is safest?
- Can static text entries live without new data formats?

## Source Search Notes

Read-only search commands used during docs-only investigation:

```sh
rg "message|msgbox|text|menu|scroll|field note|Pokenav|easy chat" src include data/maps data/scripts
rg "ShowFieldMessage|PrintText|Menu|ListMenu|Task" src include | head -n 120
```

## Candidate Symbols / Areas

| Area | Candidate file / symbol | Confidence | Notes |
|---|---|---|---|
| Field text | `include/field_message_box.h` / `ShowFieldMessage`, `ShowFieldMessageFromBuffer` | High | Existing field message display path. |
| Field message special | `data/specials.inc` / `ShowFieldMessageStringVar4` | High | Script-facing helper found. |
| Field message implementation | `src/field_message_box.c` | High | Existing message box implementation. |
| List menu input | `include/list_menu.h` / `ListMenu_ProcessInput` | High | Existing list menu processing for custom screens. |
| Script text patterns | `data/maps/*/scripts.inc`, `data/scripts/*.inc` | High | `message`, `msgbox`, `waitmessage`, `messageautoscroll` are common. |

## Open Questions

- Long text paging should be tested before large lore entries are written.
- PokeNav integration is not an MVP entry point.
