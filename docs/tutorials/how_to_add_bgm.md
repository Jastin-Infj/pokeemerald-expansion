# How to add or change BGM

この tutorial は BGM の差し替えと新規曲追加の入口です。通常BGMは
MIDI / GBA song assembly / voicegroup / song table の作業であり、MP3や
`.m4a`音声ファイルをそのまま入れる作業ではありません。

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
| Direct sound sample conversion | `tools/wav2agb`, `tools/aif2pcm` |
| Runtime playback | `PlayBGM`, `PlayNewMapMusic`, `FadeOutAndPlayNewMapMusic`, `Overworld_PlaySpecialMapMusic` |

The build uses `mid2agb`; `Makefile` sets `MID_SUBDIR = sound/songs/midi` and
builds `*.mid` into object files. The Battle BGM Selector branch also imports
`tools/aif2pcm` for `.aif` direct sound samples used by later-generation sound
banks.

## Source Format Gate

Normal BGM in this project should start from MIDI or traceable GBA m4a song
assembly. In this context, "m4a" means the GBA m4a / mp2k sound engine and its
sequenced song data, not an MPEG-4 `.m4a` audio container. MP3, OGG, AAC, FLAC,
or normal streamed WAV files are not implementation-ready BGM sources for this
pipeline.

The practical gate is:

1. Prefer an original or maintained `.mid` source.
2. Accept already converted GBA song assembly only when its provenance,
   matching song symbol, song-table position, and voicegroup expectations are
   clear.
3. Treat audio-only files as listening references, not importable assets.
4. Do not rely on automatic MP3-to-MIDI transcription without manual cleanup,
   loop setup, voicegroup mapping, build validation, and mGBA listening checks.

`tools/wav2agb` and `tools/aif2pcm` exist in the build tooling, but they are not
the default path for normal battle / map BGM. They convert direct sound sample
sources used by voicegroups. Use the MIDI -> `mid2agb` -> song table path
unless a separate sound-engine task explicitly owns sample-based audio.

## MP3 / M4A / FFmpeg Notes

FFmpeg can be useful outside the ROM build if it is installed locally, but it
does not solve normal GBA BGM import by itself.

Acceptable helper use:

```sh
ffmpeg -i source.mp3 -ar 13379 -ac 1 reference.wav
ffmpeg -i source.m4a -ar 13379 -ac 1 reference.wav
```

Use those outputs only as listening / transcription references, or as direct
sound sample sources when a branch explicitly owns sample import. Do not commit
MP3 / `.m4a` containers as BGM sources.

Not enough for BGM:

```text
MP3 / .m4a -> FFmpeg -> WAV -> normal battle BGM
```

That path produces audio samples, not a sequenced GBA song. To become normal
BGM, the music still needs MIDI or GBA song assembly, loop data, a compatible
voicegroup, song table registration, and mGBA listening validation. Automatic
MP3-to-MIDI transcription is possible as an external experiment, but the result
must be treated as a hand-authored MIDI candidate, not a trusted conversion.

Local smoke verification confirmed that an existing MIDI can be regenerated:

```sh
tools/mid2agb/mid2agb sound/songs/midi/mus_vs_trainer.mid /tmp/mus_vs_trainer.s -E -R50 -G_vs_trainer -V080 -P1
cmp -s /tmp/mus_vs_trainer.s sound/songs/midi/mus_vs_trainer.s
```

The command matched the checked-in generated assembly when the output basename
was the same. This is the expected conversion route for future BGM imports.

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

## Importing From Another Decomp Repo

Do not assume a song can be copied by moving only its `.mid`.

Audit these layers before importing:

1. The `.mid` or traceable generated song assembly.
2. The source repository's `mid2agb` options. In this repo those options live in
   `sound/songs/midi/midi.cfg`; some source repos use `songs.mk` instead.
3. The target `MUS_*` constant and matching `sound/song_table.inc` row.
4. The voicegroup symbol used by the generated song assembly.
5. Any nested voicegroup dependencies.
6. Any required key split tables.
7. Any direct sound sample symbols and their `.bin` / source sample files.
8. Credit, source URL, branch, commit, and license / permission status.

Modern Emerald is a useful reference, but its later-generation sound banks use
numeric voicegroups and `.aif` sample conversion through `tools/aif2pcm`.
This local repo mostly uses named Gen 3 / FRLG voicegroups and `.wav` sample
conversion through `tools/wav2agb`. A DPPt / HGSS import is therefore a sound
bank migration, not a simple MIDI import.

The first battle BGM import started with a small BW/BW2-style slice and then
expanded to selected DPPt / HGSS battle tracks under
[Battle BGM Selector Asset Sources](../features/battle_bgm_selector/asset_sources.md):

- `mus_bw_vs_iris.mid`
- `mus_bw_vs_legend.mid`
- `mus_dp_vs_wild.mid`, `mus_dp_vs_trainer.mid`,
  `mus_dp_vs_gym_leader.mid`, `mus_dp_vs_champion.mid`,
  `mus_dp_vs_legend.mid`, `mus_dp_vs_rival.mid`,
  `mus_dp_vs_uxie_mesprit_azelf.mid`, `mus_dp_vs_dialga_palkia.mid`,
  `mus_dp_vs_arceus.mid`
- `mus_pl_vs_giratina.mid`, `mus_pl_vs_frontier_brain.mid`,
  `mus_pl_vs_regi.mid`
- `mus_hg_vs_wild.mid`, `mus_hg_vs_trainer.mid`,
  `mus_hg_vs_gym_leader.mid`, `mus_hg_vs_champion.mid`,
  `mus_hg_vs_lugia.mid`, `mus_hg_vs_ho_oh.mid`,
  `mus_hg_vs_rocket.mid`, `mus_hg_vs_rival.mid`,
  `mus_hg_vs_suicune.mid`, `mus_hg_vs_entei.mid`,
  `mus_hg_vs_raikou.mid`, `mus_hg_vs_wild_kanto.mid`,
  `mus_hg_vs_trainer_kanto.mid`, `mus_hg_vs_gym_leader_kanto.mid`,
  `mus_hg_vs_frontier_brain.mid`, `mus_hg_vs_kyogre_groudon.mid`,
  `mus_hg_vs_arceus.mid`

The BW/BW2 tracks use Modern Emerald `voicegroup274` and `voicegroup275`. The
DPPt / HGSS tracks use `voicegroup191` / `voicegroup229` and require the larger
numeric voicegroup, keysplit, and `.aif` direct sound sample closure. Keep
later imports small and audited; do not copy whole soundtrack folders by
default.

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

Some imported MIDI files contain GBA `PRIO` controller events. These can be
dangerous for battle BGM: high-priority BGM notes may occupy the limited
DirectSound channel pool and prevent lower-priority SE / move SFX / cries from
playing. If a track previews correctly but SE cuts out during real battles,
regenerate it with the local `mid2agb -Q` option and confirm the generated
song assembly contains no `PRIO` commands.

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

Battle music has separate selection paths, so changing a map header does not
change trainer battle BGM. For the local battle profile feature, see
[Battle BGM Selector / Sound Archive](../features/battle_bgm_selector/README.md).

## Verification Checklist

- `MUS_*` constant value and `sound/song_table.inc` row order match.
- The song symbol name in `song_table.inc` exists in generated asm.
- The `_grp` voicegroup symbol exists.
- Map header music and script overrides do not fight each other.
- Entering the map from warp, Fly, and return-to-field all produce expected music.
- Sound effects still play over the BGM.
- The song loops or ends intentionally.
