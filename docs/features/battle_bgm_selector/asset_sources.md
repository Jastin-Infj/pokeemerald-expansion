# Battle BGM Selector Asset Sources

## Status

This document records external reference candidates and imported draft assets.
It does not by itself grant permission to ship assets on `master`.

## Existing Local Assets

The first runtime MVP should use existing local tracks:

- Hoenn battle tracks in `include/constants/songs.h`, such as `MUS_VS_WILD`,
  `MUS_VS_TRAINER`, `MUS_VS_GYM_LEADER`, `MUS_VS_ELITE_FOUR`, and
  `MUS_VS_CHAMPION`.
- FRLG / Kanto battle tracks in `include/constants/songs.h`, such as
  `MUS_RG_VS_WILD`, `MUS_RG_VS_TRAINER`, `MUS_RG_VS_GYM_LEADER`,
  `MUS_RG_VS_CHAMPION`, `MUS_RG_VS_MEWTWO`, and `MUS_RG_VS_LEGEND`.

## New BGM Format Gate

New battle BGM import is only actionable when the source provides one of:

- a real `.mid` file suitable for the repository's `mid2agb` pipeline;
- already converted GBA m4a song assembly with clear provenance, song symbol,
  song table position, and compatible voicegroup expectations;
- a documented conversion source that can reproduce the `.mid` / song assembly.

Audio-only files such as MP3, OGG, AAC, FLAC, or a normal streamed WAV are not
enough for the normal BGM pipeline. They may be useful as listening references,
but they should not be treated as implementation-ready assets.

Important terminology: in this codebase, "m4a" usually means the GBA m4a /
mp2k sound engine and its sequenced song data, not an MPEG-4 `.m4a` audio
container. FFmpeg can decode MP3 / AAC / `.m4a` containers to PCM formats such
as WAV when installed, but that does not create a playable GBA m4a sequence.
For normal looping BGM, the actionable source is still MIDI or traceable GBA
song assembly plus a compatible voicegroup.

The current build discovers `sound/songs/midi/*.mid` through
`MID_SUBDIR = sound/songs/midi`, converts those MIDI files with
`tools/mid2agb`, and links the generated song objects through
`sound/song_table.inc`. `tools/wav2agb` exists for WAV sample conversion, and
this branch imports `tools/aif2pcm` for `.aif` direct sound samples used by
later-generation sound banks. Neither tool replaces the MIDI / song-table path
for normal BGM.

Automatic MP3-to-MIDI transcription is not a reliable import path for this
project. Even if a tool produces a `.mid`, the result still needs manual cleanup,
GBA voicegroup mapping, loop setup, build validation, and mGBA listening checks.
If the original or maintained MIDI source cannot be found, defer the track.

## FFmpeg / MP3 / M4A Conversion Design

`ffmpeg` was checked locally during this branch and was not installed
(`ffmpeg: command not found`). If it is added later, it should be treated as an
offline helper, not as part of the default ROM build.

Acceptable future use:

- Decode MP3 / AAC / `.m4a` reference audio to WAV for listening, waveform
  inspection, or manual transcription.
- Decode short source audio to WAV / AIFF only for a branch that deliberately
  owns direct sound sample import.
- Record the command, source file, license, and resulting sample path in the
  feature docs.

Not acceptable as a normal BGM path:

- MP3 / `.m4a` -> FFmpeg -> WAV -> "battle BGM" without MIDI / GBA song
  assembly.
- Automatic MP3-to-MIDI transcription as an unattended import. Any transcribed
  MIDI still needs manual cleanup, looping, voicegroup mapping, build checks,
  and mGBA listening validation.
- Adding compressed audio assets to the ROM tree as source BGM.

## Local Conversion Smoke Check

Docs-only verification on 2026-05-17 confirmed that local MIDI conversion works:

```sh
tools/mid2agb/mid2agb sound/songs/midi/mus_vs_trainer.mid /tmp/mus_vs_trainer.s -E -R50 -G_vs_trainer -V080 -P1
cmp -s /tmp/mus_vs_trainer.s sound/songs/midi/mus_vs_trainer.s
```

The comparison matched when the output basename was `mus_vs_trainer.s`. This
confirms that the repository can reproduce existing song assembly from a valid
`.mid` plus the matching `midi.cfg` options.

This does not prove that MP3 / streamed audio can be imported. It proves the
usable pipeline starts from MIDI or equivalent GBA song assembly.

## External Reference Candidates

| Source | URL | Relevance | Import status | Notes |
|---|---|---|---|---|
| Modern Emerald | <https://github.com/resetes12/pokeemerald> | Has a documented sound feature set with DPPt / HGSS / BW music and selectable wild / trainer / Frontier battle music profiles. | BW/BW2 plus selected DPPt / HGSS battle draft imports on `feature/battle-bgm-selector-mvp-20260517`. | Repository default branch was checked at `7ca11b375bc68caee94791d294af75ee71d1ddba` on 2026-05-17. GitHub API did not report a top-level license file; keep this as an explicit credit / permission risk before master adoption. |
| PokeRogue | <https://github.com/pagefaultgames/pokerogue> | Possible trainer / UI / sprite reference source, not a direct GBA BGM source. | Reference only. | Repository documents file-level license / REUSE expectations. Assets without explicit licensing must not be assumed usable. |

## 2026-05-17 Modern Emerald Sound Dependency Audit

The local repository and Modern Emerald do not use the exact same sound import
shape.

Local sound pipeline:

- `Makefile` sets `MID_SUBDIR = sound/songs/midi`.
- `audio_rules.mk` reads `sound/songs/midi/midi.cfg`.
- MIDI is converted with `tools/mid2agb/mid2agb`.
- Direct sound samples are generated from `.wav` through `tools/wav2agb`.
- This feature branch imports `tools/aif2pcm` and adds a general
  `sound/%.aif` -> generated `.bin` rule for later-generation direct sound
  sample sources.

Modern Emerald sound pipeline:

- Battle MIDI files live under `sound/songs/midi`.
- `songs.mk` uses numeric voicegroups such as `-G191`, `-G229`, `-G274`, and
  `-G275`.
- `tools/aif2pcm` exists and Modern Emerald's Makefile converts `.aif` direct
  sound samples to `.bin`.
- Modern Emerald has 436 direct sound sample basenames under
  `sound/direct_sound_samples`.

Local compatibility notes:

- Before this branch, the local repository had 105 direct sound sample
  basenames and no numeric Modern Emerald voicegroup block.
- The first BW/BW2 import only needed numeric voicegroups / keysplit labels and
  reused existing local direct sound symbols.
- The DPPt / HGSS import requires a larger numeric voicegroup stack, full
  Modern keysplit compatibility, and `.aif` direct sound samples converted by
  `tools/aif2pcm`.

This means a full DPPt / HGSS import is not a MIDI-only copy. It is a sound
bank import.

| Candidate bank | Modern voicegroup | Pre-import voicegroup closure | Pre-import keysplit closure | Pre-import direct sound sample gap | Notes |
|---|---:|---:|---:|---:|---|
| DPPt main | `voicegroup191` | 38 voicegroups | 29 keysplits | 182 `.aif` samples | Heavy. Requires the `voicegroup191` through `voicegroup228` dependency stack plus DPPt samples. |
| HGSS main | `voicegroup229` | 48 voicegroups | 30 keysplits | 195 `.aif` samples | Heavy. Requires the `voicegroup229` through `voicegroup273` dependency stack plus HGSS/DPPt samples. |
| BW Iris | `voicegroup274` | 6 | 5 | 0 | Best first import candidate. Local direct sound symbols already exist; numeric voicegroups and key split labels are missing. |
| BW Legend | `voicegroup275` | 7 | 4 | 0 | Best first import candidate. Local direct sound symbols already exist; numeric voicegroups and key split labels are missing. |

The combined DPPt / HGSS closure imported by this branch uses `voicegroup191`
through `voicegroup273`, Modern keysplits, and 324 `.aif` direct sound source
files after overlap between the banks is removed.

## Imported BGM Slices

The first import used a small BW/BW2-style slice:

| Track | Modern Emerald source | Modern options | Import reason |
|---|---|---|---|
| `mus_bw_vs_iris.mid` | `sound/songs/midi/mus_bw_vs_iris.mid` | `-E -R0 -G274 -V090` | Champion-style BW/BW2 battle track with the smallest audited dependency stack. |
| `mus_bw_vs_legend.mid` | `sound/songs/midi/mus_bw_vs_legend.mid` | `-E -R0 -G275 -V090` | Legendary BW battle track with the smallest audited dependency stack. |

The follow-up import expands the same feature branch with selected DPPt / HGSS
battle tracks:

| Track | Modern Emerald source | Modern options | Selector target |
|---|---|---|---|
| `mus_dp_vs_wild.mid` | `sound/songs/midi/mus_dp_vs_wild.mid` | `-E -R0 -G191 -V088` | Wild |
| `mus_dp_vs_trainer.mid` | `sound/songs/midi/mus_dp_vs_trainer.mid` | `-E -R0 -G191 -V088` | Trainer |
| `mus_dp_vs_gym_leader.mid` | `sound/songs/midi/mus_dp_vs_gym_leader.mid` | `-E -R0 -G191 -V088` | Trainer / boss |
| `mus_dp_vs_champion.mid` | `sound/songs/midi/mus_dp_vs_champion.mid` | `-E -R0 -G191 -V090` | Trainer / boss |
| `mus_dp_vs_legend.mid` | `sound/songs/midi/mus_dp_vs_legend.mid` | `-E -R0 -G191 -V092` | Wild / legendary |
| `mus_dp_vs_elite_four.mid` | `sound/songs/midi/mus_dp_vs_elite_four.mid` | `-E -R0 -G191 -V094` | Trainer / boss |
| `mus_dp_vs_galactic.mid` | `sound/songs/midi/mus_dp_vs_galactic.mid` | `-E -R0 -G191 -V090` | Trainer / villain |
| `mus_dp_vs_galactic_commander.mid` | `sound/songs/midi/mus_dp_vs_galactic_commander.mid` | `-E -R0 -G191 -V090` | Trainer / villain |
| `mus_dp_vs_galactic_boss.mid` | `sound/songs/midi/mus_dp_vs_galactic_boss.mid` | `-E -R0 -G191 -V090` | Trainer / villain boss |
| `mus_dp_vs_rival.mid` | `sound/songs/midi/mus_dp_vs_rival.mid` | `-E -R0 -G191 -V088` | Trainer / rival |
| `mus_dp_vs_uxie_mesprit_azelf.mid` | `sound/songs/midi/mus_dp_vs_uxie_mesprit_azelf.mid` | `-E -R0 -G191 -V078` | Wild / legendary |
| `mus_dp_vs_dialga_palkia.mid` | `sound/songs/midi/mus_dp_vs_dialga_palkia.mid` | `-E -R0 -G191 -V090` | Wild / legendary |
| `mus_dp_vs_arceus.mid` | `sound/songs/midi/mus_dp_vs_arceus.mid` | `-E -R0 -G191 -V092` | Wild / legendary |
| `mus_pl_vs_giratina.mid` | `sound/songs/midi/mus_pl_vs_giratina.mid` | `-E -R0 -G191 -V105` | Wild / legendary |
| `mus_pl_vs_frontier_brain.mid` | `sound/songs/midi/mus_pl_vs_frontier_brain.mid` | `-E -R0 -G191 -V120` | Trainer / boss |
| `mus_pl_vs_regi.mid` | `sound/songs/midi/mus_pl_vs_regi.mid` | `-E -R0 -G191 -V090 -Q` | Wild / legendary |
| `mus_hg_vs_wild.mid` | `sound/songs/midi/mus_hg_vs_wild.mid` | `-E -R0 -G229 -V110 -Q` | Wild |
| `mus_hg_vs_trainer.mid` | `sound/songs/midi/mus_hg_vs_trainer.mid` | `-E -R0 -G229 -V111 -Q` | Trainer |
| `mus_hg_vs_gym_leader.mid` | `sound/songs/midi/mus_hg_vs_gym_leader.mid` | `-E -R0 -G229 -V108 -Q` | Trainer / boss |
| `mus_hg_vs_champion.mid` | `sound/songs/midi/mus_hg_vs_champion.mid` | `-E -R0 -G229 -V113 -Q` | Trainer / boss |
| `mus_hg_vs_lugia.mid` | `sound/songs/midi/mus_hg_vs_lugia.mid` | `-E -R0 -G229 -V102 -Q` | Wild / legendary |
| `mus_hg_vs_ho_oh.mid` | `sound/songs/midi/mus_hg_vs_ho_oh.mid` | `-E -R0 -G229 -V079 -Q` | Wild / legendary |
| `mus_hg_vs_rocket.mid` | `sound/songs/midi/mus_hg_vs_rocket.mid` | `-E -R0 -G229 -V102 -Q` | Trainer / villain |
| `mus_hg_vs_rival.mid` | `sound/songs/midi/mus_hg_vs_rival.mid` | `-E -R0 -G229 -V084 -Q` | Trainer / rival |
| `mus_hg_vs_suicune.mid` | `sound/songs/midi/mus_hg_vs_suicune.mid` | `-E -R0 -G229 -V098 -Q` | Wild / legendary |
| `mus_hg_vs_entei.mid` | `sound/songs/midi/mus_hg_vs_entei.mid` | `-E -R0 -G229 -V098 -Q` | Wild / legendary |
| `mus_hg_vs_raikou.mid` | `sound/songs/midi/mus_hg_vs_raikou.mid` | `-E -R0 -G229 -V098 -Q` | Wild / legendary |
| `mus_hg_vs_wild_kanto.mid` | `sound/songs/midi/mus_hg_vs_wild_kanto.mid` | `-E -R0 -G229 -V103 -Q` | Wild |
| `mus_hg_vs_trainer_kanto.mid` | `sound/songs/midi/mus_hg_vs_trainer_kanto.mid` | `-E -R0 -G229 -V119 -Q` | Trainer |
| `mus_hg_vs_gym_leader_kanto.mid` | `sound/songs/midi/mus_hg_vs_gym_leader_kanto.mid` | `-E -R0 -G229 -V075 -Q` | Trainer / boss |
| `mus_hg_vs_frontier_brain.mid` | `sound/songs/midi/mus_hg_vs_frontier_brain.mid` | `-E -R0 -G229 -V100 -Q` | Trainer / boss |
| `mus_hg_vs_kyogre_groudon.mid` | `sound/songs/midi/mus_hg_vs_kyogre_groudon.mid` | `-E -R0 -G229 -V110 -Q` | Wild / legendary |
| `mus_hg_vs_arceus.mid` | `sound/songs/midi/mus_hg_vs_arceus.mid` | `-E -R0 -G229 -V099 -Q` | Wild / legendary |

Implemented draft runtime scope:

- Imports selected battle MIDI files only, not Modern Emerald audio wholesale.
- Adds numeric voicegroups `001`, `002`, `005` through `009`, and `191`
  through `275` to support the selected banks.
- Keeps `voicegroup191` through `voicegroup273` contiguous because HGSS
  voicegroups intentionally reference adjacent data in that stack.
- Replaces the earlier minimal keysplit compatibility block with the full
  Modern keysplit compatibility data. The file is still named
  `sound/keysplit_tables_modern_bw.inc` for branch continuity.
- Imports `tools/aif2pcm` and 324 `.aif` direct sound source samples required
  by the DPPt / HGSS bank closure.
- Uses a local `mid2agb -Q` option for HGSS tracks and `mus_pl_vs_regi` to
  suppress source MIDI `PRIO` controller events that otherwise starve battle SE
  in real battles.
- Adds `MUS_*` constants and `sound/song_table.inc` rows in matching order.
- Adds selector choices after the songs build and link.

Validation evidence for this draft slice:

- `rtk make -j16 -O all`: pass with existing RWX linker warning.
- `rtk make -j16 -O debug`: pass with existing RWX linker warning.
- `rtk make -j16 -O check`: pass with existing RWX linker warning.
- `rtk mdbook build docs`: pass with existing docs warnings.
- mGBA Live: `BW2 Iris` and `BW Legend` both rendered in the debug selector,
  previewed without lockup, and updated `gMPlayInfo_BGM.songHeader` to the
  imported song headers.
- DPPt / HGSS: `all`, `debug`, and `check` pass after the `.aif` toolchain and
  sound bank import. mGBA Live previewed `HGSS Ho-Oh`, `DPPt Legend`,
  `DPPt Champ`, `HGSS Rocket`, and `DPPt Cyrus` and confirmed their song
  headers. A later expansion added DPPt Rival/Lake Trio/Dialga-Palkia/Arceus,
  Platinum Giratina/Frontier Brain/Regi, and HGSS Rival/beasts/Kanto/
  Frontier/Kyogre-Groudon/Arceus choices; build, routing tests, and generated
  `PRIO` scan passed. mGBA Live previewed `Plt Frontier`, `HGSS Arceus`, and
  `Plt Regi` and confirmed their song headers.

## Candidate Source Search: XY / ORAS / MP3 Sources

GitHub code search on 2026-05-17 did not find a ready pokeemerald-style
`mus_xy_*`, `xerneas`, or `lysandre` sound-table implementation. Treat XY as a
manual MIDI port candidate, not as an already packaged GBA import.

Modern Emerald was also checked for additional build-ready BW/BW2 battle MIDI.
At audited commit `7ca11b375bc68caee94791d294af75ee71d1ddba`, its
`sound/songs/midi/` tree only exposed `mus_bw_vs_iris.mid` and
`mus_bw_vs_legend.mid` under the `mus_bw_*` battle naming used by this branch.
Do not assume BW Wild/Trainer/Gym/Plasma/N/Ghetsis are available there without
another source audit.

VGMusic has MIDI candidates for several requested XY / ORAS tracks. These are
not voicegrouped for GBA, so they require `mid2agb` option selection,
instrument cleanup, loop review, and mGBA listening work before importing.

| Requested theme | Candidate source | Format | Status / risk |
|---|---|---|---|
| XY wild battle | VGMusic `Pokemon_XY_Wild_Pokemon.mid` / `X-Y_Wild-1.mid` | MIDI | Possible manual port. Multiple arrangements exist; choose one after listening. |
| XY trainer battle | VGMusic `xy-trainerbattle.mid` | MIDI | Possible manual port; page asks to contact the sequencer before use. |
| XY Elite Four | VGMusic `VS_shitenno.mid` | MIDI | Possible manual port. |
| XY champion / Diantha | VGMusic `ChampionXY.mid` | MIDI | Riskier: file metadata includes an "All Rights Reserved" copyright event and extra credit note. |
| XY legendary / Xerneas-Yveltal-Zygarde | VGMusic `VS_XERNEAS_YVELTAL_ZYGARDE.mid` | MIDI | Best XY legendary candidate found so far. Needs loop and voicegroup work. |
| XY Team Flare grunt | VGMusic `flare_M.mid` | MIDI | Possible villain/Team Flare candidate, separate from Rocket/Galactic. |
| XY Lysandre | VGMusic `SEQ_BGM_VS_LYSANDRE.mid` | MIDI | Possible villain boss candidate. |
| ORAS Zinnia / Lorekeeper | VGMusic `Pokemon_ORAS_-_VS_Zinna.mid` | MIDI | Possible "Lorekeeper" candidate if this is what the request means by successor/inheritor. |

PokéRogue has many battle BGM assets in `pagefaultgames/pokerogue-assets`,
including `battle_legendary_xern_yvel.mp3`, `battle_kalos_champion.mp3`,
`battle_kalos_elite.mp3`, `battle_flare_boss.mp3`,
`battle_galactic_boss.mp3`, and `battle_rocket_grunt.mp3`. Those files are MP3,
not MIDI / GBA song assembly. Its `audio/REUSE.toml` marks many official-series
background music files as `LicenseRef-FAIR-USE` with Nintendo / Creatures /
GAME FREAK copyright attribution. Use them only as listening references unless
a separate sample/transcription experiment explicitly owns the legal and
technical risk.

Essentials-style sources are similarly useful as reference / naming guidance
but usually store ordinary audio in `Audio/BGM` rather than GBA-ready song
assembly. They do not remove the need for a MIDI / voicegroup port.

## Future Import Order

1. Finish mGBA listening and battle-start evidence for the current BW / DPPt /
   HGSS battle subset.
2. Add additional tracks only when they reuse the imported banks cleanly and the
   source / license / credit status is recorded.
3. Avoid importing whole soundtrack folders without a feature owner, menu
   surface, and ROM-size review.
4. Treat MP3 / `.m4a` / WAV-only sources as reference audio unless a separate
   conversion/transcription task produces reviewed MIDI or GBA song assembly.

Each import should update this document with:

- source repository, branch, commit, and file paths;
- exact `mid2agb` options;
- voicegroup dependencies;
- missing sample / keysplit / symbol audit;
- build evidence;
- mGBA listening evidence;
- credit / license status.

## Import Policy

Before importing any new music:

1. Identify the exact source repository, branch, commit, and file path.
2. Record license / permission and credit requirements.
3. Confirm that the source has an implementation-ready `.mid` or traceable GBA
   song assembly. If only MP3 / WAV / OGG / AAC exists, mark the track as
   reference-only.
4. Confirm voicegroup compatibility and loop behavior.
5. Add source attribution to the owning feature docs and project credits before
   runtime merge.
6. Validate the imported track in mGBA for loop behavior, volume, and sound
   effect coexistence.

## Do Not Import Yet

- Do not import Modern Emerald audio wholesale.
- Do not import DeviantArt-only music without a clear redistribution permission.
- Do not import music just because another ROM hack uses it.
- Do not convert MP3 / streamed audio and call it ready without a reviewed MIDI
  or GBA song assembly artifact.
- Do not import additional music without a fresh dependency audit and mGBA
  evidence.
