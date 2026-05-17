# Future Runtime Handoff: Battle BGM Selector

## FUTURE USE ONLY

Do not use this prompt during docs-only work.

## Runtime Prompt

runtime implementation task.

Purpose:
Implement the Battle BGM Selector / Sound Archive MVP as an independent runtime
feature. Use existing BGM first; import new music only after the asset source
gate is updated and the selected track's voicegroup/sample dependencies are
known.

Branch:
`feature/battle-bgm-selector-mvp`

Start from current `master`.

Required first commands:

```sh
rtk git status --short --branch
rtk git describe --tags --always --dirty
rtk gh pr list --state open --json number,title,isDraft,headRefName,baseRefName,updatedAt,mergeStateStatus,statusCheckRollup
```

Reference docs:

- `docs/features/battle_bgm_selector/README.md`
- `docs/features/battle_bgm_selector/investigation.md`
- `docs/features/battle_bgm_selector/mvp_plan.md`
- `docs/features/battle_bgm_selector/risks.md`
- `docs/features/battle_bgm_selector/asset_sources.md`
- `docs/features/battle_bgm_selector/test_plan.md`
- `docs/flows/trainer_battle_reward_audio_flow_v15.md`
- `docs/tutorials/how_to_add_bgm.md`

MVP:

- Add a readable battle BGM selector entry from debug menu or a safe test NPC.
- Use existing `MUS_*` constants first.
- Keep `Default` as vanilla behavior.
- Add independent Trainer BGM and Wild BGM options at minimum.
- Allow previewing selected tracks.
- Apply the selected choices for focused battle testing.
- Do not add new music assets until the source, license / credit, MIDI or GBA
  song assembly, voicegroup, keysplits, and sample dependencies are documented.
- Do not import MP3, WAV, OGG, AAC, or other streamed audio as BGM.
- Defer any new battle music until a separate branch has a reviewed `.mid` or
  compatible GBA song assembly source, voicegroup plan, license, and credits.
- Do not change SaveBlock layout.
- Do not change trainer prize money.
- Do not change trainer parties.
- Do not change trainer / NPC sprites.
- Do not touch TM Shop, Unified Move Relearner, Summary Tera, Pokemon State
  Editor, Pre-Battle Team Viewer, or Champions Challenge.

Implementation targets to investigate:

- `src/pokemon.c`, `GetBattleBGM`
- `src/pokemon.c`, `PlayBattleBGM`
- `src/pokemon.c`, `PlayMapChosenOrBattleBGM`
- `src/battle_setup.c`, `PlayTrainerEncounterMusic`
- `src/debug.c`, existing Sound menu
- `include/constants/songs.h`

Validation:

```sh
rtk mdbook build docs
rtk make -j16 -O all
rtk make -j16 -O debug
rtk make -j16 -O check
```

mGBA:

- Open selector.
- Preview at least three tracks.
- Select Default and confirm vanilla battle BGM.
- Select a non-default Trainer BGM and confirm normal trainer battle BGM changes.
- Select a non-default Wild BGM and confirm normal wild battle BGM changes.
- Confirm map BGM returns or document failure.
- Record whether legendary explicit song paths are unchanged or overridden.

## Future Additional BGM Import Prompt

Use this only after the current BW/BW2, DPPt, Platinum, and HGSS draft imports
are reviewed and the desired next source track is identified.

runtime implementation task.

Purpose:
Add one small battle BGM import slice to the existing Battle BGM Selector
feature. Do not import a whole sound bank wholesale.

Branch:
`feature/battle-bgm-selector-next-bgm-import`

Start from current `master` or the approved integration branch, not from an old
stale branch.

Required first commands:

```sh
rtk git status --short --branch
rtk git describe --tags --always --dirty
rtk gh pr list --state open --json number,title,isDraft,headRefName,baseRefName,updatedAt,mergeStateStatus,statusCheckRollup
```

Required docs before source edits:

- Update `docs/features/battle_bgm_selector/asset_sources.md` with the exact
  source repository, commit, file paths, license / permission status, MIDI /
  song assembly status, voicegroup dependencies, keysplit dependencies, and
  direct sound sample dependencies.
- If any dependency is unknown, stop at docs and do not import runtime files.

Import rules:

- Prefer `.mid` files with known `mid2agb` options or traceable GBA song
  assembly.
- Treat MP3 / OGG / AAC / FLAC / streamed WAV as reference-only unless a
  reviewed MIDI or GBA song assembly artifact exists.
- Add only the voicegroups, keysplits, and samples needed by the selected
  track.
- Do not expand direct-sample tooling unless a selected import actually needs
  it and the branch owns that toolchain change.
- DPPt / Platinum / HGSS-style tracks require the larger numeric voicegroup /
  `.aif` sample bank plan; do not treat them as MIDI-only imports.
- Do not change SaveBlock layout.
- Do not change trainer data, prize money, parties, TM/HM, Summary, Move
  Relearner, Bag, Champions, or NPC sprite systems.

Validation:

```sh
rtk mdbook build docs
rtk make -j16 -O all
rtk make -j16 -O debug
rtk make -j16 -O check
```

mGBA:

- Open `Debug Menu -> Sound... -> Trainer BGM...` or `Wild BGM...`.
- Confirm the imported track appears under the intended target.
- Press A to preview the imported track.
- Read or otherwise confirm `gMPlayInfo_BGM.songHeader` points to the imported
  song header.
- Confirm map BGM can be restored or document the failure.
- Stop mGBA Live and record cleanup state.
