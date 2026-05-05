# How to add or change BGM

この tutorial は BGM の差し替えと新規曲追加の入口です。現時点では docs-only の調査メモで、source 改造はしていません。

## Existing Ownership

| Area | File |
|---|---|
| Song ID constants | [include/constants/songs.h](../../include/constants/songs.h) |
| Song table | [sound/song_table.inc](../../sound/song_table.inc) |
| MIDI source | `sound/songs/midi/*.mid` |
| Generated song asm | `sound/songs/midi/*.s` |
| Voicegroups | `sound/voicegroups/*.inc` |
| Map default music | `data/maps/MapName/header.inc` generated from `map.json` |
| Script music commands | `playbgm`, `savebgm`, `fadenewbgm`, `playfanfare` in `data/maps/*/scripts.inc` |
| Runtime playback | `PlayBGM`, `PlayNewMapMusic`, `FadeOutAndPlayNewMapMusic`, `Overworld_PlaySpecialMapMusic` |

The build uses `mid2agb`; `Makefile` sets `MID_SUBDIR = sound/songs/midi` and builds `*.mid` into object files.

## Changing Existing Map Music

For a simple map BGM change:

1. Find the map header or `map.json` owner for the target map.
2. Change the music field to an existing `MUS_*`.
3. Check scripts for `playbgm`, `savebgm`, or `fadenewbgm` that override the map default.

Examples:

```asm
.2byte MUS_B_FRONTIER
```

```asm
playbgm MUS_RG_ENCOUNTER_DEOXYS, FALSE
savebgm MUS_LINK_CONTEST_P1
fadenewbgm MUS_RG_SURF
```

If a script calls `playbgm`, the map header value may be correct but not audible during that event.

## Adding a New BGM

High-level steps:

1. Add a new `MUS_*` constant in [include/constants/songs.h](../../include/constants/songs.h), before `END_MUS` if it is normal BGM.
2. Add a `.mid` under `sound/songs/midi/` using the same snake_case symbol name style, for example `mus_custom_theme.mid`.
3. Ensure generated asm uses `.global mus_custom_theme` and a group symbol like `mus_custom_theme_grp`.
4. Add a `song mus_custom_theme, MUSIC_PLAYER_BGM, 0` entry to [sound/song_table.inc](../../sound/song_table.inc) at the same numeric position as the new constant.
5. Choose or create a voicegroup in `sound/voicegroups/*.inc`.
6. Point the generated song asm group to that voicegroup.
7. Use the new `MUS_CUSTOM_THEME` in map data or scripts.
8. Build and verify the song starts, loops, and does not break sound effects.

The numeric ID in `songs.h` and the row order in `gSongTable` must stay synchronized. A mismatch plays the wrong song or invalid data.

## Voicegroup Notes

Existing generated song asm starts like:

```asm
.include "MPlayDef.s"

.equ mus_route110_grp, voicegroup_route110
.equ mus_route110_pri, 0
.equ mus_route110_rev, reverb_set+50
.equ mus_route110_mvl, 80
```

The `_grp` symbol selects the voicegroup. If a MIDI uses instruments that the voicegroup does not define well, the song can sound wrong even when it builds.

For a first pass, reuse a nearby voicegroup:

| Desired style | Existing examples |
|---|---|
| Hoenn route/city | `voicegroup_route110`, `voicegroup_rustboro`, `voicegroup_lilycove` |
| Frontier | `voicegroup_b_frontier`, `voicegroup_b_tower`, `voicegroup_b_pike` |
| FRLG route/city | `voicegroup_rg_route3`, `voicegroup_rg_celadon`, `voicegroup_rg_pallet` |
| Battle | `voicegroup_vs_trainer`, `voicegroup_vs_wild`, `voicegroup_vs_frontier_brain` |
| Fanfare | `voicegroup_fanfare` |

## Map vs Event Music

| Need | Use |
|---|---|
| Default music for a map | map header / `map.json` music field. |
| Temporarily play music during a cutscene | `playbgm MUS_*, FALSE`. |
| Replace current map music after fade | `fadenewbgm MUS_*`. |
| Remember a BGM for later restoration | `savebgm MUS_*`. |
| Short fanfare | `playfanfare MUS_*` and `waitfanfare`. |

Battle music has separate selection paths, so changing a map header does not change trainer battle BGM.

## Verification Checklist

- `MUS_*` constant value and `sound/song_table.inc` row order match.
- The song symbol name in `song_table.inc` exists in generated asm.
- The `_grp` voicegroup symbol exists.
- Map header music and script overrides do not fight each other.
- Entering the map from warp, Fly, and return-to-field all produce expected music.
- Sound effects still play over the BGM.
- The song loops or ends intentionally.
