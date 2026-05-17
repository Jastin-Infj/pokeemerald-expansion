# Bounty Board Investigation

## Questions

- Which script commands are used for item checks?
- Which commands remove items?
- Which commands give rewards?
- Which event flag range is safe for local completion flags?
- Which map is safest for a board NPC / signpost?
- How are multichoice menus built?

## Source Search Notes

Read-only search commands used during docs-only investigation:

```sh
rg "checkitem|removeitem|giveitem|setflag|checkflag|multichoice|message|msgbox" data/maps data/scripts src include
rg "giveitem|checkitem|removeitem" data/maps data/scripts | head -n 80
rg "multichoice|MULTI" data/maps data/scripts include src | head -n 80
```

## Candidate Symbols / Areas

| Area | Candidate file / symbol | Confidence | Notes |
|---|---|---|---|
| Script command table | `data/script_cmd_table.inc` | High | Maps `checkitem`, `removeitem`, `checkitemspace`, `setflag`, `checkflag`, and `multichoice` script commands. |
| Item script commands | `src/scrcmd.c` / `ScrCmd_checkitem`, `ScrCmd_removeitem`, `ScrCmd_checkitemspace` | High | Runtime item check / remove behavior lives here. |
| Flag script commands | `src/scrcmd.c` / `ScrCmd_setflag`, `ScrCmd_checkflag` | High | Completion flags should use existing event flag flow. |
| Menu commands | `src/scrcmd.c` / `ScrCmd_multichoice`, `ScrCmd_multichoicedefault`, `ScrCmd_multichoicegrid` | High | Useful for a script-only request board. |
| Item delivery example | `data/maps/FiveIsland_MemorialPillar_Frlg/scripts.inc` | Medium | Uses `checkitem`, `removeitem`, and reward flow for Lemonade / TM42. |
| Variable item reward example | `data/scripts/lilycove_lady.inc` | Medium | Uses variable item rewards; good reference only. |

## Open Questions

- Exact completion flag ids must not be chosen in docs-only work.
- Bag full behavior must be decided in runtime implementation.
- Safe map placement is TBD.
