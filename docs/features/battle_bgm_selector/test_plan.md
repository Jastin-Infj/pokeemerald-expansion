# Battle BGM Selector Test Plan

## Docs-only Validation

- `rtk mdbook build docs`

## Current Branch Validation

Run on `feature/battle-bgm-selector-mvp-20260517`:

- `rtk git diff --check`: pass
- `rtk make -j16 -O all`: pass with existing RWX linker warning
- `rtk make -j16 -O debug`: pass with existing RWX linker warning
- `rtk make -j16 -O check`: pass with existing RWX linker warning; includes
  `test/battle_bgm.c`
- `rtk mdbook build docs`: pass with existing warnings for missing root
  `CHANGELOG.md` include, `CREDITS.md` unexpected `</img>`, and large search
  index.
- mGBA Live BW/BW2 import pass: pass. `BW2 Iris` and `BW Legend` both rendered
  in the debug selector, previewed without lockup, and updated
  `gMPlayInfo_BGM.songHeader` to the imported song headers.
- DPPt / HGSS import build pass: `all`, `debug`, and `check` pass after
  importing the selected MIDI files, numeric voicegroups, `.aif` samples, and
  `tools/aif2pcm`.
- mGBA Live DPPt / HGSS import pass: pass. `HGSS Ho-Oh`, `DPPt Legend`, and
  `DPPt Champ` rendered in the debug selector, previewed without lockup, and
  updated `gMPlayInfo_BGM.songHeader` to the imported song headers.
- mGBA Live Rocket/Galactic follow-up pass: pass. `HGSS Rocket` and
  `DPPt Cyrus` rendered in the debug selector, previewed without lockup, and
  updated `gMPlayInfo_BGM.songHeader` to the imported song headers.
- mGBA Live HGSS Rocket battle SE-priority pass: pass. After applying
  `mid2agb -Q`, started an actual debug trainer battle with Trainer BGM set to
  `HGSS Rocket`, confirmed `gMPlayInfo_BGM.songHeader` pointed to regenerated
  `mus_hg_vs_rocket` (`0x099E5C68`), and sampled DirectSound channel priorities
  as `0,0,0,0,0` during the battle opening. MCP cannot hear audio, so final
  listening remains a manual check, but the original `PRIO 64` starvation cause
  is removed.

## Focused Code Checks

- Confirm `GetBattleBGM()` returns vanilla values when Trainer/Wild choices are
  `Default`.
- Confirm selected Wild choice changes normal wild battle BGM.
- Confirm selected Trainer choice changes normal trainer battle BGM.
- Confirm Gym Leader / Elite Four / Champion categories still route correctly.
- Confirm explicit legendary song paths are overridden by the Wild choice when
  their song ID is in the known wild / legendary battle set.
- Confirm `.encounterMusic` remains the eye-contact cue unless encounter cue
  support is explicitly added as a separate setting.
- `test/battle_bgm.c` verifies Default, Trainer/Wild independence, expanded
  trainer choices, expanded wild/legendary choices, and imported BW/BW2,
  DPPt, and HGSS song routing.

## New BGM Asset Gate

Run this gate for every branch that imports new battle music:

- Confirm the source provides `.mid` or traceable GBA song assembly.
- Confirm MP3 / WAV / OGG / AAC-only sources are rejected or marked
  reference-only.
- Confirm external source repository, branch, commit, and file paths are
  recorded in `asset_sources.md`.
- Confirm credit / permission status is recorded even when the runtime branch is
  a draft.
- Confirm `MUS_*` constant order and `sound/song_table.inc` row order match.
- Confirm the song symbol and `_grp` voicegroup symbol build.
- Confirm imported voicegroup dependencies, `KeySplitTable` labels, and direct
  sound sample symbols all resolve.
- Confirm loop points, volume, and sound effect coexistence in mGBA.
- For imported MIDI that emits `PRIO` controller events, confirm the generated
  song assembly does not starve battle SE. Use `mid2agb -Q` on the affected
  tracks when BGM preview works but real battle SE / move SFX / cries cut out.

## BW/BW2 First Import Gate

For the first Modern Emerald BW/BW2-style slice:

- Imported only `mus_bw_vs_iris` and `mus_bw_vs_legend`.
- Used Modern Emerald `songs.mk` options:
  - `mus_bw_vs_iris`: `-E -R0 -G274 -V090`
  - `mus_bw_vs_legend`: `-E -R0 -G275 -V090`
- Confirmed `voicegroup274` and `voicegroup275` build without importing DPPt /
  HGSS bank stacks.
- Confirmed no new direct sound sample files are required for this slice.
- Confirmed the selector can preview both tracks.
- Confirmed `BW2 Iris` maps through the Trainer selector and changes
  `gMPlayInfo_BGM.songHeader` to `mus_bw_vs_iris` (`0x09893A9C`) during
  preview.
- Confirmed `BW Legend` maps through the Wild selector and changes
  `gMPlayInfo_BGM.songHeader` to `mus_bw_vs_legend` (`0x09896074`) during
  preview.
- Actual Trainer/Wild battle starts with the imported songs remain accepted
  follow-up mGBA checks; focused helper tests cover the routing.

## DPPt / HGSS Import Gate

For the selected Modern Emerald DPPt / HGSS battle slice:

- Imported selected battle MIDI only, not whole soundtrack folders.
- Used Modern Emerald `songs.mk` options:
  - `mus_dp_vs_wild`: `-E -R0 -G191 -V088`
  - `mus_dp_vs_trainer`: `-E -R0 -G191 -V088`
  - `mus_dp_vs_gym_leader`: `-E -R0 -G191 -V088`
  - `mus_dp_vs_champion`: `-E -R0 -G191 -V090`
  - `mus_dp_vs_legend`: `-E -R0 -G191 -V092`
  - `mus_dp_vs_elite_four`: `-E -R0 -G191 -V094`
  - `mus_dp_vs_galactic`: `-E -R0 -G191 -V090`
  - `mus_dp_vs_galactic_commander`: `-E -R0 -G191 -V090`
  - `mus_dp_vs_galactic_boss`: `-E -R0 -G191 -V090`
  - `mus_dp_vs_rival`: `-E -R0 -G191 -V088`
  - `mus_dp_vs_uxie_mesprit_azelf`: `-E -R0 -G191 -V078`
  - `mus_dp_vs_dialga_palkia`: `-E -R0 -G191 -V090`
  - `mus_dp_vs_arceus`: `-E -R0 -G191 -V092`
  - `mus_pl_vs_giratina`: `-E -R0 -G191 -V105`
  - `mus_pl_vs_frontier_brain`: `-E -R0 -G191 -V120`
  - `mus_pl_vs_regi`: `-E -R0 -G191 -V090 -Q`
  - `mus_hg_vs_wild`: `-E -R0 -G229 -V110 -Q`
  - `mus_hg_vs_trainer`: `-E -R0 -G229 -V111 -Q`
  - `mus_hg_vs_gym_leader`: `-E -R0 -G229 -V108 -Q`
  - `mus_hg_vs_champion`: `-E -R0 -G229 -V113 -Q`
  - `mus_hg_vs_lugia`: `-E -R0 -G229 -V102 -Q`
  - `mus_hg_vs_ho_oh`: `-E -R0 -G229 -V079 -Q`
  - `mus_hg_vs_rocket`: `-E -R0 -G229 -V102 -Q`
  - `mus_hg_vs_rival`: `-E -R0 -G229 -V084 -Q`
  - `mus_hg_vs_suicune`: `-E -R0 -G229 -V098 -Q`
  - `mus_hg_vs_entei`: `-E -R0 -G229 -V098 -Q`
  - `mus_hg_vs_raikou`: `-E -R0 -G229 -V098 -Q`
  - `mus_hg_vs_wild_kanto`: `-E -R0 -G229 -V103 -Q`
  - `mus_hg_vs_trainer_kanto`: `-E -R0 -G229 -V119 -Q`
  - `mus_hg_vs_gym_leader_kanto`: `-E -R0 -G229 -V075 -Q`
  - `mus_hg_vs_frontier_brain`: `-E -R0 -G229 -V100 -Q`
  - `mus_hg_vs_kyogre_groudon`: `-E -R0 -G229 -V110 -Q`
  - `mus_hg_vs_arceus`: `-E -R0 -G229 -V099 -Q`
- Confirmed `mid2agb -Q` suppresses the HGSS source MIDI and `mus_pl_vs_regi`
  `PRIO` controller events in generated imported battle `.s` files. This is
  required because high-priority battle BGM channels can prevent lower-priority
  SE from taking DirectSound channels in real battle.
- Imported `voicegroup191` through `voicegroup273` and kept the stack
  contiguous for HGSS bank semantics.
- Imported `tools/aif2pcm` and required `.aif` direct sound source samples.
- Confirmed all imported symbols build through `all`, `debug`, and `check`.
- Confirmed generated imported battle `.s` files for the expanded DPPt /
  Platinum / HGSS batch have no `PRIO` commands after `mus_pl_vs_regi` was
  switched to `-Q`.
- Focused mGBA playback confirmed HGSS wild/legendary, DPPt wild/legendary,
  DPPt trainer/boss, HGSS Rocket, and DPPt Cyrus choices can preview from the
  debug selector and update `gMPlayInfo_BGM`.

## mGBA Checks

Split selector pass on 2026-05-17:

- Booted `pokeemerald.gba` in mGBA Live.
- Continued existing `pokeemerald.sav`.
- Opened overworld debug menu with `R + START`.
- Opened `Sound...`.
- Confirmed `Trainer BGM...` and `Wild BGM...` entries render.
- Opened `Trainer BGM...`, selected `Kanto Trainer`, and pressed A to set /
  preview.
- Opened `Wild BGM...`, selected `Kanto Wild`, and pressed A to set / preview.
- Read `sBattleBgmChoices` from EWRAM and confirmed values `[4, 2]`
  (`Kanto Trainer`, `Kanto Wild`).
- Started `Party -> Start Debug Battle`.
- Read `gMPlayInfo_BGM.songHeader` during the trainer battle and confirmed it
  pointed to `mus_rg_vs_trainer`, matching the selected Trainer BGM choice.
- Stopped mGBA Live cleanly with `alive_after: false`.

BW/BW2 import pass on 2026-05-17:

- Booted the debug `pokeemerald.gba` in mGBA Live.
- Continued existing `pokeemerald.sav`.
- Opened overworld debug menu with `R + START`.
- Opened `Sound...`.
- Opened `Wild BGM...`, selected `BW Legend`, and pressed A to set / preview.
- Read `gMPlayInfo_BGM.songHeader` and confirmed it pointed to
  `mus_bw_vs_legend` at `0x09896074`.
- Reopened `Sound... -> Trainer BGM...`, selected `BW2 Iris`, and pressed A to
  set / preview.
- Read `gMPlayInfo_BGM.songHeader` and confirmed it pointed to
  `mus_bw_vs_iris` at `0x09893A9C`.
- Stopped mGBA Live cleanly with `alive_after: false`.

DPPt / HGSS import pass on 2026-05-17:

- Booted the debug `pokeemerald.gba` in mGBA Live.
- Continued existing `pokeemerald.sav`.
- Opened overworld debug menu with `R + START`.
- Opened `Sound...`.
- Opened `Wild BGM...`, selected `HGSS Ho-Oh`, and pressed A to set /
  preview.
- Read `gMPlayInfo_BGM.songHeader` and confirmed it pointed to
  `mus_hg_vs_ho_oh` at `0x099E58D4`.
- In the same Wild selector, selected `DPPt Legend` and pressed A to set /
  preview.
- Read `gMPlayInfo_BGM.songHeader` and confirmed it pointed to
  `mus_dp_vs_legend` at `0x099CA334`.
- Reopened `Sound... -> Trainer BGM...`, selected `DPPt Champ`, and pressed A
  to set / preview.
- Read `gMPlayInfo_BGM.songHeader` and confirmed it pointed to
  `mus_dp_vs_champion` at `0x099C7BD8`.
- Exported a focused screenshot to `/tmp/dphg-bgm-test-dppt-champ.png`.
- Stopped mGBA Live cleanly with `alive_after: false`; `pgrep -af mgba` showed
  only MCP service processes plus the `pgrep` command itself.

Rocket/Galactic follow-up pass on 2026-05-17:

- Booted the debug `pokeemerald.gba` in mGBA Live.
- Continued existing `pokeemerald.sav`.
- Opened overworld debug menu with `R + START`.
- Opened `Sound... -> Trainer BGM...`.
- Selected `HGSS Rocket` and pressed A to set / preview.
- Read `gMPlayInfo_BGM.songHeader` and confirmed it pointed to
  `mus_hg_vs_rocket` at `0x099F01C8`.
- Selected `DPPt Cyrus` and pressed A to set / preview.
- Read `gMPlayInfo_BGM.songHeader` and confirmed it pointed to
  `mus_dp_vs_galactic_boss` at `0x099CD0FC`.
- Exported a focused screenshot to `/tmp/rocket-galactic-bgm-test-dppt-cyrus.png`.
- Stopped mGBA Live cleanly with `alive_after: false`; `pgrep -af mgba` showed
  only MCP service processes plus the `pgrep` command itself.

HGSS Rocket battle SE-priority pass on 2026-05-17:

- Booted the debug `pokeemerald.gba` in mGBA Live.
- Continued existing `pokeemerald.sav`.
- Wrote Trainer BGM choice `39` (`HGSS Rocket`) to `sBattleBgmChoices[0]`
  (`0x02000074`) for a focused route. This was the pre-expanded enum value;
  after the later DPPt / Platinum / HGSS expansion, `HGSS Rocket` is choice
  `46`.
- Opened the overworld debug menu with `R + START`.
- Opened `Party... -> Start Debug Battle`.
- Confirmed the battle opening used regenerated `mus_hg_vs_rocket` by reading
  `gMPlayInfo_BGM.songHeader` as `0x099E5C68`.
- Sampled DirectSound channel priorities at `gSoundInfo.chans[0..4]` during
  the battle opening and confirmed `0,0,0,0,0`, not the source MIDI `PRIO 64`
  values.
- Selected a battle move and confirmed the battle advanced without lockup.
- Exported a focused screenshot to `/tmp/battle-bgm-se-prio-check-hgss-rocket.png`.
- Stopped mGBA Live cleanly with `alive_after: false`; `pgrep -af mgba` showed
  only MCP service processes plus the `pgrep` command itself.

Expanded DPPt / Platinum / HGSS preview pass on 2026-05-17:

- Booted the debug `pokeemerald.gba` in mGBA Live.
- Continued existing `pokeemerald.sav`.
- Wrote Trainer BGM choice `38` (`Plt Frontier`) and Wild BGM choice `56`
  (`HGSS Arceus`) to `sBattleBgmChoices`.
- Opened `Sound... -> Trainer BGM...`, confirmed the selector rendered
  `Plt Frontier`, pressed A to preview, and confirmed
  `gMPlayInfo_BGM.songHeader` pointed to `mus_pl_vs_frontier_brain` at
  `0x09A24114`.
- Opened `Sound... -> Wild BGM...`, confirmed the selector rendered
  `HGSS Arceus`, pressed A to preview, and confirmed
  `gMPlayInfo_BGM.songHeader` pointed to `mus_hg_vs_arceus` at `0x099EBAA0`.
- Wrote Wild BGM choice `39` (`Plt Regi`) for a focused check of the `-Q`
  converted Platinum track, reopened `Sound... -> Wild BGM...`, previewed it,
  and confirmed `gMPlayInfo_BGM.songHeader` pointed to `mus_pl_vs_regi` at
  `0x09A28880`.
- Exported focused screenshots to `/tmp/bgm-expanded-pl-frontier-preview.png`,
  `/tmp/bgm-expanded-hgss-arceus-preview2.png`, and
  `/tmp/bgm-expanded-pl-regi-preview.png`.
- Stopped mGBA Live cleanly with `alive_after: false`; `pgrep -af mgba` showed
  only MCP service processes plus the `pgrep` command itself.

Earlier combined-selector pass before the split follow-up:

- Opened the old combined `Battle BGM...` screen.
- Confirmed the `Default` screen rendered.
- Selected `Kanto` with D-Pad input.
- Pressed A to set / preview `Kanto`.
- Pressed B and returned to the field.

Not yet confirmed in mGBA:

- Starting an actual wild battle after selecting a Wild choice.
- Starting actual Trainer/Wild battles after selecting the imported BW/BW2
  choices.
- Starting actual Trainer/Wild battles after selecting the imported DPPt / HGSS
  choices, except the focused `HGSS Rocket` trainer battle SE-priority route
  above.
- Gym Leader / Elite Four / Champion representatives.
- A / B mash stress pass.

Cleanup note:

- Earlier combined-selector pass: `mgba-live-cli stop` returned
  `alive_after: true`; a defunct `mgba-qt` child remained under the MCP parent
  after manual `kill`.
- Split-selector pass: stop returned `alive_after: false`; no `mgba-qt` process
  remained.

## Known MVP Gaps To Record

- Persistent setting not implemented in first slice unless explicitly chosen.
- Encounter cue setting not implemented unless explicitly chosen.
- Victory BGM setting not implemented.
- Imported new BGM is limited to the selected BW/BW2, DPPt, and HGSS battle
  draft slices; future imports still require the MIDI / GBA song assembly asset
  gate.
