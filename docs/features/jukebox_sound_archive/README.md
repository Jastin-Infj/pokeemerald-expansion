# Jukebox / Sound Archive

## Status

- Status: Planned
- Code status: No code changes on master
- Docs status: Initial design
- Feature type: Fully new local feature candidate
- Recommended first runtime candidate: Yes

## Goal

Provide an in-game sound archive / sound test that lets the player or developer
preview existing BGM tracks from a small menu.

## First MVP

- Use existing BGM only.
- No new music assets.
- No save data changes.
- No unlock tracking.
- No favorites.
- No PokeNav integration.
- Open from debug menu or safe test NPC.
- Fixed track list of around 8 to 12 songs.
- A plays the selected track.
- B exits.
- On exit, attempt to restore the current map BGM.

## Non-goals

- Do not add new songs.
- Do not add custom audio engine changes.
- Do not add unlock rules.
- Do not add save data.
- Do not integrate into PokeNav in the first slice.
- Do not touch battle, Summary, Move Relearner, TM/HM, Bag, or Champions Challenge features.

## Why This Feature

This is a low-risk new feature that creates immediate game feel without touching
the existing runtime implementation shelf.

## Related Docs

- [Investigation](investigation.md)
- [MVP Plan](mvp_plan.md)
- [Risks](risks.md)
- [Test Plan](test_plan.md)
- [Future Runtime Handoff](future_runtime_handoff.md)
