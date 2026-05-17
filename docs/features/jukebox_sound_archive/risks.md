# Jukebox / Sound Archive Risks

## Runtime Risks

- Failing to restore map BGM after exit.
- Starting music from a menu state that does not expect BGM changes.
- Debug menu entry point conflicts.
- Held input / repeated A press causing rapid BGM restart.
- Track names not matching song constants.
- Certain tracks may be fanfare / ME / SE rather than looping BGM.

## Risk Controls

- Start with existing BGM constants only.
- No new audio assets.
- No save layout changes.
- Small fixed track list.
- Use debug-only entry first if map placement is not needed.
- Treat map BGM restore failure as known risk if not solved in MVP.
