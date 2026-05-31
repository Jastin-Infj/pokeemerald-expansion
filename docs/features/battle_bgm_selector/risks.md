# Battle BGM Selector Risks

## Runtime Risks

- `GetBattleBGM()` is not the only path: some battle setup paths pass explicit
  song IDs. The current draft routes known explicit trainer / wild battle song
  IDs through `ApplyBattleBgmSelection()`, but future special paths need review.
- Changing `.encounterMusic` will not change battle BGM, which can lead to a
  false-positive implementation.
- Persistent settings need an owner. The current integration uses existing
  SaveBlock2 filler plus a magic marker for the two debug-selected choices; a
  player-facing Options UI still needs separate design.
- Random choice mode can make mGBA validation harder unless seeded or logged.
- Some `MUS_*` entries are fanfares or special-case tracks rather than safe
  looping battle BGM.
- Stopping / starting BGM from debug or menu callbacks can fail to restore map
  music if the exit path is not tested.
- Victory BGM is separate and should not be accidentally changed while touching
  battle BGM.
- Link / recorded / Frontier / Trainer Hill / Battle Dome paths may have special
  assumptions.

## Asset Risks

- New music import requires synchronized updates to `include/constants/songs.h`
  and `sound/song_table.inc`.
- The normal BGM pipeline is MIDI-driven. MP3 / OGG / AAC / streamed WAV files
  are reference audio only unless a reviewed `.mid` or compatible GBA song
  assembly exists.
- Automatic MP3-to-MIDI conversion is likely to need manual cleanup,
  voicegroup mapping, loop authoring, and listening validation before it can be
  considered usable.
- A track without original / maintained MIDI source can stall the import before
  code work starts.
- Imported MIDI / song assembly may use incompatible voicegroups.
- Imported MIDI can also contain high `PRIO` controller events. In battle this
  can starve SE / move SFX / cries because DirectSound channel allocation uses
  priority across BGM and SE. Use `mid2agb -Q` for affected BGM imports unless
  there is a deliberate reason to preserve track-priority events.
- Third-party battle tracks can depend on an entire sound bank, not just a
  `.mid` file. Modern Emerald's DPPt / HGSS tracks require large numeric
  voicegroup, keysplit, and `.aif` sample stacks.
- Modern Emerald's BW `voicegroup274` / `voicegroup275` are smaller, but they
  still need numeric voicegroup and `KeySplitTable` compatibility work.
- This branch imports `tools/aif2pcm`; keep its license and source provenance
  with the feature evidence before any integration handoff.
- Third-party music repositories can have unclear permissions.
- Credit-only is not always enough; asset license / source traceability must be
  recorded before importing.

## Scope Controls

- Existing-BGM selector was validated first; new imports must remain small,
  audited slices.
- First slice keeps vanilla as the default choice.
- Trainer / NPC sprite expansion is tracked separately.
- Any additional new BGM asset import must have MIDI / song assembly,
  voicegroup, source, credit, and mGBA evidence.
- Player-facing persistent settings must either use the existing SaveBlock2
  filler owner or a separately documented runtime option owner.

## Accepted MVP Risk

- The original choice state was session-only. The integration branch now
  persists Trainer/Wild choices after save/load, while old saves without the
  magic marker still read as `Default`.
- The selector does not own trainer eye-contact cues or victory music. Those
  should become separate settings if needed.
- The imported BW/BW2 and DPPt / Platinum / HGSS tracks have build evidence,
  but source repository licensing / permission remains unresolved for master
  adoption.
- Expanded DPPt / Platinum / HGSS battle-start behavior still needs focused
  mGBA evidence beyond debug selector preview.
- HGSS imports and `mus_pl_vs_regi` use local `mid2agb -Q` to prevent
  high-priority BGM tracks from starving battle SE. The tradeoff is that SE may
  steal BGM notes during noisy battle animations, which is preferable to losing
  SE.
- XY / ORAS MIDI candidates found outside GitHub are not GBA-ready. They need
  manual voicegroup selection, loop review, credit checks, and listening
  validation before import.
- PokéRogue MP3 assets are useful references, but they are not a normal GBA BGM
  source and carry separate fair-use / redistribution risk.
