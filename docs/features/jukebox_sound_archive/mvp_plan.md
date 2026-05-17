# Jukebox / Sound Archive MVP Plan

## MVP Track List

Use a small fixed table. Do not hardcode exact constants during planning unless
they were confirmed by source search.

Intended categories:

- Title
- Route
- Pokemon Center
- Gym
- Surf
- Cave / Victory Road
- Battle / victory
- Frontier

## UI Contract

- List menu with track names.
- A: play selected track.
- B: close.
- No save state.
- No unlock state.
- No category UI in first slice.

## Entry Point Options

| Option | Pros | Cons | Decision |
|---|---|---|---|
| Debug menu entry | Does not affect story maps. | Needs debug menu ownership. | Preferred if low-risk. |
| Test NPC | Simple script access. | Requires safe map placement. | Acceptable fallback. |
| PokeNav page | Feels polished. | Too much UI ownership for MVP. | Not MVP. |

## Future Work

- Unlock songs after hearing them.
- Favorites.
- Category filtering.
- PokeNav integration.
- Custom track support.
- Composer / developer notes.
