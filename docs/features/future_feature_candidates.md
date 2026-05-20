# Future Feature Candidates

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-17 |
| Baseline | `master` `c8b8e57183`; `git describe` = `expansion/1.15.2-56-gc8b8e57183` |
| Code status | Docs only / no code changes |
| Provenance | Local project planning notes and read-only source search |

## Purpose

This page tracks fully new local feature candidates that are not direct
extensions of the current runtime implementation shelf.

## Selection Principles

- Avoid touching existing implementation shelf PRs.
- Prefer features that can start as docs-only or small MVPs.
- Avoid save layout changes for first slices.
- Avoid battle hooks for first slices unless the feature explicitly owns that risk.
- Prefer features that create a clear in-game feeling without changing core progression.

## Candidate Matrix

| Priority | Feature | Theme | First MVP Size | Runtime Risk | Notes |
|---:|---|---|---|---|---|
| 1 | Jukebox / Sound Archive | Music / sound test | Low-Medium | Low | Best first new runtime feature candidate. |
| 2 | Weather Lab Terminal | Visual / map atmosphere | Low | Low-Medium | Good debug and presentation utility. |
| 3 | Bounty Board / Request Board | Script-driven side requests | Low-Medium | Low | Can start as item-delivery only. |
| 4 | Field Notes / Lore Codex | Worldbuilding / text archive | Medium | Low-Medium | Good for original ROM identity. |
| 5 | Route Mastery Passport | Exploration / completion | Medium | Medium | Needs flag / map ownership care. |
| 6 | Trainer Titles / Achievement Badges | Player identity / achievements | Medium | Medium | Avoid Trainer Card in MVP. |
| 7 | Art Gallery / Museum Archive | Visual archive | Medium-High | Medium | Asset policy needed. |
| 8 | Party / Status UI Overhaul | Core UI refresh | Medium-High | Medium-High | Candidate registered separately; party layout target is `2 x 3` (`2 / 2 / 2`) and BW Summary / DS-style party screens are external references only. |
| 9 | Battle Replay Log Lite | Battle log / analytics | High | High | Avoid until battle hooks are stable. |

## Recommended First Runtime Candidate

Recommend [Jukebox / Sound Archive](jukebox_sound_archive/README.md) as the
first new feature implementation candidate because it can avoid SaveBlock,
battle hooks, Summary, TM/HM, Move Relearner, Bag, and Champions systems while
still giving an immediate in-game feel.

## Foldered Candidates

- [Jukebox / Sound Archive](jukebox_sound_archive/README.md)
- [Weather Lab Terminal](weather_lab_terminal/README.md)
- [Bounty Board / Request Board](bounty_board/README.md)
- [Field Notes / Lore Codex](field_notes_codex/README.md)
- [Route Mastery Passport](route_mastery_passport/README.md)
- [Trainer Titles / Achievement Badges](trainer_titles_achievement_badges/README.md)
- [Party / Status UI Overhaul](party_status_ui_overhaul/README.md)

## Art Gallery / Museum Archive

Theme:

- Visual archive / background art / museum display.

First MVP idea:

- NPC opens a simple gallery list with static text descriptions only.
- No new graphics in first slice.
- Later add image / background unlocks.

Risks:

- Graphics asset policy.
- VRAM / palette / UI complexity.
- Copyright / credit tracking.
- Scope creep into a full museum map.

Recommendation:

- Keep as future candidate.
- Do not create a full folder yet unless this becomes active work.

## Battle Replay Log Lite

Theme:

- Post-battle summary log.

First MVP idea:

- After battle, show a short textual summary of turns, KOs, or MVP.
- This requires battle hook / logging ownership and should not be the first new
  feature slice.

Risks:

- Battle engine hook complexity.
- Memory buffer ownership.
- Double / multi battle cases.
- Battle Frontier / link battle exclusions.
- Text generation and timing.

Recommendation:

- Do not implement early.
- Create full docs later after battle hook policy stabilizes.
