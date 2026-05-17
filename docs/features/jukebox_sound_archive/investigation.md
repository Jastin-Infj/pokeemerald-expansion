# Jukebox / Sound Archive Investigation

## Questions

- Where are BGM song constants defined?
- Which functions safely start BGM playback?
- Which functions restore map music?
- Is there an existing debug menu entry point?
- Is a test NPC safer than debug menu integration?
- Does the engine track current map music in a way that can be restored after a menu closes?

## Source Search Notes

Read-only search commands used during docs-only investigation:

```sh
rg "MUS_ROUTE101|MUS_POKE_CENTER|MUS_TITLE|PlayBGM|Fade.*BGM|StopMapMusic|debug|sound" src include data
rg "PlayBGM|FadeOutBGM|FadeInBGM|StopMapMusic|GetCurrentMapMusic|Overworld_PlaySpecialMapMusic" src include
rg "MUS_" include/constants data src | head -n 80
```

## Candidate Symbols / Areas

| Area | Candidate file / symbol | Confidence | Notes |
|---|---|---|---|
| Song constants | `include/constants/songs.h` | High | Defines `MUS_TITLE`, `MUS_ROUTE101`, `MUS_POKE_CENTER`, `MUS_SURF`, `MUS_GYM`, `MUS_VICTORY_*`, and more. |
| BGM API | `include/sound.h` | High | Declares `GetCurrentMapMusic`, `PlayNewMapMusic`, `StopMapMusic`, `FadeOutAndPlayNewMapMusic`, `FadeOutBGMTemporarily`, `FadeInBGM`, and `PlayBGM`. |
| BGM implementation | `src/sound.c` | High | Implements current / next map music tracking and direct BGM start / fade functions. |
| Map music restore | `src/overworld.c` / `Overworld_PlaySpecialMapMusic` | Medium | Existing map music restore path; must be tested from the chosen menu callback. |
| Debug entry point | `src/debug.c` / `Debug_ShowMainMenu` | High | Existing debug menu uses `ListMenu_ProcessInput` and includes sound / music test patterns. |
| Existing music backup example | `src/berry_blender.c` | Medium | Backs up `GetCurrentMapMusic`, plays `MUS_CYCLING`, then later `PlayBGM(savedMusic)`. Good reference only, not proof for a Jukebox menu. |

## Confirmed Example Track Constants

These constants were found in `include/constants/songs.h`:

- `MUS_TITLE`
- `MUS_ROUTE101`
- `MUS_POKE_CENTER`
- `MUS_SURF`
- `MUS_GYM`
- `MUS_CAVE_OF_ORIGIN`
- `MUS_SLATEPORT`
- `MUS_VICTORY_WILD`
- `MUS_VICTORY_TRAINER`
- `MUS_B_PYRAMID`

Do not assume every `MUS_*` constant is a looping BGM. Some entries are fanfare,
intro, or special-case tracks.
