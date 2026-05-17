# Future Runtime Handoff: Jukebox / Sound Archive

FUTURE USE ONLY.

Do not use this prompt during the docs-only task. Start from current `master`,
create a fresh runtime feature branch, and keep the first slice away from save
layout, new music assets, and PokeNav integration.

```text
runtime implementation task.

Goal:
Implement the Jukebox / Sound Archive MVP. Create a small Sound Test that can
select and play existing BGM in-game. Do not add new music assets, unlock
tracking, save fields, favorites, or PokeNav integration.

branch:
feature/jukebox-sound-archive-mvp

First commands:
rtk git status --short --branch
rtk git describe --tags --always --dirty
rtk gh pr list --state open --json number,title,isDraft,headRefName,baseRefName,updatedAt,mergeStateStatus,statusCheckRollup

Reference docs:
- docs/features/jukebox_sound_archive/README.md
- docs/features/jukebox_sound_archive/investigation.md
- docs/features/jukebox_sound_archive/mvp_plan.md
- docs/features/jukebox_sound_archive/risks.md
- docs/features/jukebox_sound_archive/test_plan.md

MVP:
- Existing songs only.
- Fixed table of 8 to 12 tracks.
- A plays selected track.
- B exits.
- Restore map BGM after exit if possible.
- No save data.
- No unlocks.
- No favorites.
- No new music assets.

Entry point:
- Use a debug-only entry if safe.
- Otherwise use a safe test NPC / script entry.
- Avoid normal story map changes for the first slice.

Do not touch:
- TM Shop Migration
- Unified Move Relearner
- Summary Tera / Pokemon State Editor
- Pre-Battle Team Viewer
- Battle hooks
- Field Move / HM
- SaveBlock
- Bag
- New BGM assets
- PokeNav integration

Validation:
rtk mdbook build docs
rtk make -j16 -O all
rtk make -j16 -O debug
rtk make -j16 -O check

mGBA:
- Open from Jukebox entry.
- Play at least 3 tracks.
- Close with B.
- Confirm map BGM returns.
- Confirm held / repeated input does not lock up.
- Reopen without breaking state.
```
