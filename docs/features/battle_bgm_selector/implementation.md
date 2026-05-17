# Battle BGM Selector Implementation

## Status

- Status: Implemented draft
- Branch: `feature/battle-bgm-selector-mvp-20260517`
- Master status: Not merged
- Runtime scope: Existing BGM plus BW/BW2, DPPt, Platinum, and HGSS battle import slices
- Save layout: No changes

## Implemented Files

| File | Purpose |
|---|---|
| `include/battle_bgm.h` | Public Trainer/Wild target and BGM choice declarations. |
| `src/battle_bgm.c` | Session-only Trainer/Wild choice state and song mapping helpers. |
| `src/pokemon.c` | Wraps vanilla `GetBattleBGM()` and explicit song IDs with selected choices. |
| `src/debug.c` | Adds `Sound -> Trainer BGM...` and `Sound -> Wild BGM...` selector UIs. |
| `test/battle_bgm.c` | Focused Trainer/Wild choice mapping tests. |
| `include/constants/songs.h` | Adds imported BW/BW2, DPPt, Platinum, and HGSS song IDs and keeps phoneme constants after `END_MUS`. |
| `sound/song_table.inc` | Adds imported BW/BW2, DPPt, Platinum, and HGSS song table rows. |
| `sound/songs/midi/midi.cfg` | Adds `mid2agb` options for the imported BW/BW2, DPPt, Platinum, and HGSS MIDI files; HGSS tracks and `mus_pl_vs_regi` use `-Q` to suppress source MIDI track-priority events. |
| `sound/songs/midi/mus_bw_vs_iris.mid` | Imported BW2-style Champion battle MIDI from Modern Emerald. |
| `sound/songs/midi/mus_bw_vs_legend.mid` | Imported BW-style legendary battle MIDI from Modern Emerald. |
| `sound/voice_groups.inc` | Includes the numeric Modern Emerald voicegroups needed by the imported songs. |
| `sound/voicegroups/voicegroup001.inc` through `voicegroup009.inc` | Modern Emerald numeric dependency voicegroups used by the BW/BW2 banks. |
| `sound/voicegroups/voicegroup274.inc` and `voicegroup275.inc` | Modern Emerald BW/BW2 song voicegroups. |
| `sound/keysplit_tables_modern_bw.inc` | Full Modern Emerald keysplit compatibility block, kept under its original branch filename. |
| `data/sound_data.s` | Includes the Modern keysplit compatibility block after the local keysplit tables. |
| `tools/aif2pcm/` | Imported Modern-compatible `.aif` to direct sound sample `.bin` converter used by DPPt / HGSS banks. |
| `tools/mid2agb/` | Adds the local `-Q` conversion option for suppressing MIDI `PRIO` controller output on high-priority imported battle MIDI. |
| `Makefile`, `make_tools.mk`, `audio_rules.mk` | Build integration for `tools/aif2pcm` and `sound/*.aif` direct sound sample conversion. |
| `sound/direct_sound_samples/*.aif` | Imported DPPt / HGSS direct sound source samples required by the selected bank closure. |
| `sound/direct_sound_data.inc` | Adds `DirectSoundWaveData_*` labels for the imported `.aif` sample sources. |
| `sound/voicegroups/voicegroup191.inc` through `voicegroup273.inc` | Modern Emerald DPPt / HGSS numeric voicegroup closure. |
| `sound/songs/midi/mus_dp_vs_*.mid` | Imported DPPt-style battle MIDI files from Modern Emerald. |
| `sound/songs/midi/mus_hg_vs_*.mid` | Imported HGSS-style battle MIDI files from Modern Emerald. |

## Runtime Behavior

- `Default` returns vanilla battle BGM behavior.
- Trainer BGM and Wild BGM are stored as separate session-only choices.
- The choices are stored in EWRAM only and reset when the ROM/session resets.
- `GetBattleBGM()` applies the selected Trainer or Wild choice based on the
  vanilla song category.
- `PlayMapChosenOrBattleBGM(songId)` now applies the selected choice to nonzero
  explicit battle song IDs too. This covers special battle-start paths that do
  not rely only on `GetBattleBGM()`.
- Wild BGM choice covers normal wild and legendary/special wild battle songs.
- Trainer BGM choice covers normal trainer and boss-style trainer songs.
- `BW2 Iris`, DPPt Trainer/Gym/Elite Four/Champion/Galactic/Cyrus/Rival,
  Platinum Frontier Brain, and HGSS Trainer/Gym/Champion/Rocket/Rival/Kanto
  Trainer/Kanto Gym/Frontier Brain are classified as Trainer/Boss battle BGM
  choices.
- `BW Legend`, DPPt Wild/Legend/Lake Trio/Dialga-Palkia/Arceus, Platinum
  Giratina/Regi, and HGSS Wild/Lugia/Ho-Oh/Suicune/Entei/Raikou/Kanto
  Wild/Kyogre-Groudon/Arceus are classified as Wild/Legendary battle BGM
  choices.

## Imported Battle Tracks

The external music import slices use selected battle tracks from Modern
Emerald:

| Choice | Song constant | Source track | Voicegroup | `mid2agb` options |
|---|---|---|---:|---|
| `BW2 Iris` | `MUS_BW_VS_IRIS` | `mus_bw_vs_iris.mid` | `274` | `-E -R0 -G274 -V090` |
| `BW Legend` | `MUS_BW_VS_LEGEND` | `mus_bw_vs_legend.mid` | `275` | `-E -R0 -G275 -V090` |
| `DPPt Wild` | `MUS_DP_VS_WILD` | `mus_dp_vs_wild.mid` | `191` | `-E -R0 -G191 -V088` |
| `DPPt Trainer` | `MUS_DP_VS_TRAINER` | `mus_dp_vs_trainer.mid` | `191` | `-E -R0 -G191 -V088` |
| `DPPt Gym` | `MUS_DP_VS_GYM_LEADER` | `mus_dp_vs_gym_leader.mid` | `191` | `-E -R0 -G191 -V088` |
| `DPPt Champion` | `MUS_DP_VS_CHAMPION` | `mus_dp_vs_champion.mid` | `191` | `-E -R0 -G191 -V090` |
| `DPPt Legend` | `MUS_DP_VS_LEGEND` | `mus_dp_vs_legend.mid` | `191` | `-E -R0 -G191 -V092` |
| `DPPt Elite Four` | `MUS_DP_VS_ELITE_FOUR` | `mus_dp_vs_elite_four.mid` | `191` | `-E -R0 -G191 -V094` |
| `DPPt Galactic` | `MUS_DP_VS_GALACTIC` | `mus_dp_vs_galactic.mid` | `191` | `-E -R0 -G191 -V090` |
| `DPPt Commander` | `MUS_DP_VS_GALACTIC_COMMANDER` | `mus_dp_vs_galactic_commander.mid` | `191` | `-E -R0 -G191 -V090` |
| `DPPt Cyrus` | `MUS_DP_VS_GALACTIC_BOSS` | `mus_dp_vs_galactic_boss.mid` | `191` | `-E -R0 -G191 -V090` |
| `DPPt Rival` | `MUS_DP_VS_RIVAL` | `mus_dp_vs_rival.mid` | `191` | `-E -R0 -G191 -V088` |
| `DPPt Lake Trio` | `MUS_DP_VS_UXIE_MESPRIT_AZELF` | `mus_dp_vs_uxie_mesprit_azelf.mid` | `191` | `-E -R0 -G191 -V078` |
| `DPPt Dialga` | `MUS_DP_VS_DIALGA_PALKIA` | `mus_dp_vs_dialga_palkia.mid` | `191` | `-E -R0 -G191 -V090` |
| `DPPt Arceus` | `MUS_DP_VS_ARCEUS` | `mus_dp_vs_arceus.mid` | `191` | `-E -R0 -G191 -V092` |
| `Plt Giratina` | `MUS_PL_VS_GIRATINA` | `mus_pl_vs_giratina.mid` | `191` | `-E -R0 -G191 -V105` |
| `Plt Frontier` | `MUS_PL_VS_FRONTIER_BRAIN` | `mus_pl_vs_frontier_brain.mid` | `191` | `-E -R0 -G191 -V120` |
| `Plt Regi` | `MUS_PL_VS_REGI` | `mus_pl_vs_regi.mid` | `191` | `-E -R0 -G191 -V090 -Q` |
| `HGSS Wild` | `MUS_HG_VS_WILD` | `mus_hg_vs_wild.mid` | `229` | `-E -R0 -G229 -V110 -Q` |
| `HGSS Trainer` | `MUS_HG_VS_TRAINER` | `mus_hg_vs_trainer.mid` | `229` | `-E -R0 -G229 -V111 -Q` |
| `HGSS Gym` | `MUS_HG_VS_GYM_LEADER` | `mus_hg_vs_gym_leader.mid` | `229` | `-E -R0 -G229 -V108 -Q` |
| `HGSS Champion` | `MUS_HG_VS_CHAMPION` | `mus_hg_vs_champion.mid` | `229` | `-E -R0 -G229 -V113 -Q` |
| `HGSS Lugia` | `MUS_HG_VS_LUGIA` | `mus_hg_vs_lugia.mid` | `229` | `-E -R0 -G229 -V102 -Q` |
| `HGSS Ho-Oh` | `MUS_HG_VS_HO_OH` | `mus_hg_vs_ho_oh.mid` | `229` | `-E -R0 -G229 -V079 -Q` |
| `HGSS Rocket` | `MUS_HG_VS_ROCKET` | `mus_hg_vs_rocket.mid` | `229` | `-E -R0 -G229 -V102 -Q` |
| `HGSS Rival` | `MUS_HG_VS_RIVAL` | `mus_hg_vs_rival.mid` | `229` | `-E -R0 -G229 -V084 -Q` |
| `HGSS Suicune` | `MUS_HG_VS_SUICUNE` | `mus_hg_vs_suicune.mid` | `229` | `-E -R0 -G229 -V098 -Q` |
| `HGSS Entei` | `MUS_HG_VS_ENTEI` | `mus_hg_vs_entei.mid` | `229` | `-E -R0 -G229 -V098 -Q` |
| `HGSS Raikou` | `MUS_HG_VS_RAIKOU` | `mus_hg_vs_raikou.mid` | `229` | `-E -R0 -G229 -V098 -Q` |
| `HGSS Kanto Wild` | `MUS_HG_VS_WILD_KANTO` | `mus_hg_vs_wild_kanto.mid` | `229` | `-E -R0 -G229 -V103 -Q` |
| `HGSS Kanto Trainer` | `MUS_HG_VS_TRAINER_KANTO` | `mus_hg_vs_trainer_kanto.mid` | `229` | `-E -R0 -G229 -V119 -Q` |
| `HGSS Kanto Gym` | `MUS_HG_VS_GYM_LEADER_KANTO` | `mus_hg_vs_gym_leader_kanto.mid` | `229` | `-E -R0 -G229 -V075 -Q` |
| `HGSS Frontier` | `MUS_HG_VS_FRONTIER_BRAIN` | `mus_hg_vs_frontier_brain.mid` | `229` | `-E -R0 -G229 -V100 -Q` |
| `HGSS Grou/Kyo` | `MUS_HG_VS_KYOGRE_GROUDON` | `mus_hg_vs_kyogre_groudon.mid` | `229` | `-E -R0 -G229 -V110 -Q` |
| `HGSS Arceus` | `MUS_HG_VS_ARCEUS` | `mus_hg_vs_arceus.mid` | `229` | `-E -R0 -G229 -V099 -Q` |

## SE Coexistence Fix

Initial HGSS battle imports could preview in the selector but were unsafe in
real battles: their source MIDI emitted high `PRIO` controller events, commonly
`PRIO 64`. The local m4a mixer starts with five DirectSound channels, and a
high-priority BGM can keep those channels from being stolen by lower-priority
SE / move SFX / cry playback.

This branch adds a local `mid2agb -Q` option that suppresses MIDI `PRIO`
controller output while preserving song-header priority, voicegroup, volume,
and timing options. The selected HGSS battle tracks and `mus_pl_vs_regi` use
`-Q`; generated imported battle `.s` files should have no `PRIO` commands
after regeneration. This keeps SE eligible to steal DirectSound channels during
battle, at the cost of possible BGM note stealing while heavy battle effects
are playing.

Source repository:

- Modern Emerald: <https://github.com/resetes12/pokeemerald>
- Audited commit: `7ca11b375bc68caee94791d294af75ee71d1ddba`

The GitHub license API did not report a top-level license file for that commit,
and `LICENSE` was not present at the repository root during the docs-only audit.
Keep this as an explicit credit / permission risk before any master adoption.

## Available Choices

- Default
- Hoenn Wild / Kanto Wild
- Hoenn Trainer / Kanto Trainer
- Hoenn Gym / Kanto Gym
- Hoenn Champion / Kanto Champion
- Elite Four
- Frontier Brain
- Rival
- Aqua/Magma and Aqua/Magma Leader
- Regi
- Groudon/Kyogre
- Rayquaza
- Mew
- Kanto Legend
- Mewtwo
- Deoxys
- Legend Beast
- BW2 Iris
- BW Legend
- DPPt Wild / Trainer / Gym / Elite Four / Champion / Legend / Rival
- DPPt Galactic / Commander / Cyrus / Lake Trio / Dialga-Palkia / Arceus
- Platinum Giratina / Frontier Brain / Regi
- HGSS Wild / Trainer / Gym / Champion / Rocket / Rival / Lugia / Ho-Oh
- HGSS Suicune / Entei / Raikou / Kanto Wild / Kanto Trainer / Kanto Gym
- HGSS Frontier Brain / Kyogre-Groudon / Arceus

## Debug UI

Path:

```text
Debug Menu -> Sound... -> Trainer BGM...
Debug Menu -> Sound... -> Wild BGM...
```

Controls:

- D-Pad left/right/up/down: cycle choice.
- A: set selected choice and preview its track.
- START: restore current map BGM.
- B: restore current map BGM and close the debug UI.

## Validation

| Check | Result |
|---|---|
| `rtk git diff --check` | Pass |
| `rtk make -j16 -O all` | Pass with existing RWX linker warning |
| `rtk make -j16 -O debug` | Pass with existing RWX linker warning |
| `rtk make -j16 -O check` | Pass with existing RWX linker warning; includes `test/battle_bgm.c` |
| `rtk mdbook build docs` | Pass with existing warnings: missing root `CHANGELOG.md` include, existing `CREDITS.md` `</img>` warning, and large search index. |
| mGBA Live existing selector pass | Pass on 2026-05-17 booted ROM, continued existing save, opened debug `Sound -> Trainer BGM...` and `Sound -> Wild BGM...`, set Trainer to `Kanto Trainer`, set Wild to `Kanto Wild`, started `Start Debug Battle`, and confirmed `gMPlayInfo_BGM.songHeader` pointed to `mus_rg_vs_trainer`. |
| mGBA Live BW/BW2 import pass | Pass on 2026-05-17 booted debug ROM, continued existing save, opened debug `Sound -> Trainer BGM...` / `Wild BGM...`, selected and previewed `BW2 Iris` and `BW Legend`, and confirmed `gMPlayInfo_BGM.songHeader` pointed to `mus_bw_vs_iris` (`0x09893A9C`) and `mus_bw_vs_legend` (`0x09896074`). |
| mGBA Live DPPt / HGSS import pass | Pass on 2026-05-17 booted debug ROM, continued existing save, opened debug `Sound -> Wild BGM...` / `Trainer BGM...`, selected and previewed `HGSS Ho-Oh`, `DPPt Legend`, `DPPt Champ`, `HGSS Rocket`, and `DPPt Cyrus`, and confirmed `gMPlayInfo_BGM.songHeader` pointed to `mus_hg_vs_ho_oh` (`0x099E58D4`), `mus_dp_vs_legend` (`0x099CA334`), `mus_dp_vs_champion` (`0x099C7BD8`), `mus_hg_vs_rocket` (`0x099F01C8`), and `mus_dp_vs_galactic_boss` (`0x099CD0FC`). |
| mGBA Live HGSS Rocket battle SE-priority pass | Pass on 2026-05-17 after `mid2agb -Q`: booted debug ROM, set Trainer BGM choice to `HGSS Rocket`, started `Party -> Start Debug Battle`, confirmed `gMPlayInfo_BGM.songHeader` pointed to regenerated `mus_hg_vs_rocket` (`0x099E5C68`), and sampled DirectSound channel priorities as `0,0,0,0,0` during the battle opening. MCP cannot hear audio, but this confirms the imported BGM no longer holds `PRIO 64` channels that would prevent lower-priority battle SE from stealing a channel. |
| Expanded DPPt / Platinum / HGSS import build pass | Pass on 2026-05-17 after adding 17 more imported battle tracks. `debug`, `all`, and `check` passed; generated imported battle `.s` files were scanned and no `PRIO` commands remain in the newly added tracks after `mus_pl_vs_regi` was moved to `-Q`. |
| mGBA Live expanded import preview pass | Pass on 2026-05-17 booted debug ROM, continued existing save, opened debug `Sound -> Trainer BGM...` / `Wild BGM...`, selected and previewed `Plt Frontier`, `HGSS Arceus`, and `Plt Regi`, and confirmed `gMPlayInfo_BGM.songHeader` pointed to `mus_pl_vs_frontier_brain` (`0x09A24114`), `mus_hg_vs_arceus` (`0x099EBAA0`), and `mus_pl_vs_regi` (`0x09A28880`). |

## mGBA Cleanup Note

The earlier combined-selector pass left a defunct `mgba-qt` child after stop.
The split-selector rerun on 2026-05-17 stopped cleanly with
`alive_after: false`; `pgrep -af mgba` showed only the MCP service processes.
The DPPt / HGSS playback pass also stopped cleanly with `alive_after: false`;
`pgrep -af mgba` showed only the MCP service processes plus the `pgrep`
command itself.
The Rocket/Galactic follow-up playback pass also stopped cleanly with
`alive_after: false`; `pgrep -af mgba` again showed only MCP service processes
plus the `pgrep` command itself.
The HGSS Rocket battle SE-priority pass also stopped cleanly with
`alive_after: false`; `pgrep -af mgba` showed only the MCP service processes
plus the `pgrep` command itself.
The expanded DPPt / Platinum / HGSS preview pass also stopped cleanly with
`alive_after: false`; `pgrep -af mgba` showed only the MCP service processes
plus the `pgrep` command itself.

## Known Gaps

- No persistent choice setting.
- No player-facing Options / PokeNav page.
- No random choice mode.
- No separate encounter cue or victory BGM setting.
- Actual trainer battle BGM was confirmed with the split selector. Actual wild
  step encounter remains to be confirmed manually; helper tests cover wild and
  legendary song ID routing.
- The imported BW/BW2 and first DPPt / HGSS batch have focused debug selector
  preview evidence. The expanded DPPt / Platinum / HGSS batch has build,
  routing-test, and generated-PRIO-scan evidence; focused mGBA preview remains
  useful.
- HGSS tracks and `mus_pl_vs_regi` now suppress MIDI track-priority events with
  `mid2agb -Q` so battle SE can preempt BGM DirectSound notes.
- Source repository licensing / permission remains unresolved for master
  adoption.
- Actual battle start was confirmed for the `HGSS Rocket` imported trainer BGM
  after the `mid2agb -Q` fix. Additional imported DPPt / HGSS representative
  battles remain useful manual listening checks, especially for subjective mix
  balance.
