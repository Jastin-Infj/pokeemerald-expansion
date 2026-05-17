# Battle BGM Selector / Sound Archive

## Status

- Status: Implemented draft
- Code status: Runtime source exists on `feature/battle-bgm-selector-mvp-20260517`
- Docs status: Updated with implementation evidence
- Feature type: Fully new local feature candidate
- Split from: [Jukebox / Sound Archive](../jukebox_sound_archive/README.md)

## Goal

Create a small, readable way to preview and choose battle BGM choices without
digging through numeric debug music IDs.

The player-facing goal is to make trainer / wild / Gym / Elite Four / Champion
battles feel more intentional. The developer-facing goal is to make battle BGM
selection easy to test before importing new music assets.

## Current Decision

This feature is separate from the generic Jukebox.

- Jukebox owns simple existing-track preview.
- Battle BGM Selector owns battle BGM choice selection and battle-audio
  routing.
- External BGM import started as a small BW/BW2 draft slice, then expanded to a
  selected DPPt / HGSS battle import after the sound bank dependencies were
  audited.
- New battle BGM import should be tracked in this feature folder because the
  selector is the first runtime surface used to preview and validate imported
  battle tracks.
- Trainer / NPC sprite expansion is tracked separately and is not part of this
  runtime branch.

## First Runtime MVP

- Use existing `MUS_*` constants only.
- No new music assets.
- No new sound engine work.
- No Pokemon, move, TM/HM, Bag, Summary, or battle-rule changes.
- Add a readable selector entry from debug menu or a safe test NPC.
- Show readable battle BGM choices.
- Keep Trainer BGM and Wild BGM as separate settings.
- Allow previewing the selected track.
- Apply a temporary battle BGM override for focused testing.
- Keep the default choice as current vanilla behavior.
- Do not require SaveBlock layout changes.

## External BGM Import Follow-up

The current feature branch also imports Modern Emerald battle tracks as a
sound-bank compatibility proof:

- `BW2 Iris` -> `MUS_BW_VS_IRIS`
- `BW Legend` -> `MUS_BW_VS_LEGEND`
- DPPt Wild / Trainer / Gym / Elite Four / Champion / Legend / Rival
- DPPt Galactic / Commander / Cyrus / Lake Trio / Dialga-Palkia / Arceus
- Platinum Giratina / Frontier Brain / Regi
- HGSS Wild / Trainer / Gym / Champion / Rocket / Rival / Lugia / Ho-Oh
- HGSS beasts / Kanto battle tracks / Frontier Brain / Kyogre-Groudon / Arceus

This follow-up stays inside the Battle BGM Selector feature because the selector
is the current runtime surface for previewing and validating imported battle
music. It does not add SaveBlock state, player-facing Options UI, trainer data
rewrites, or new story map dependencies.

Source and credit status are recorded in
[Asset Sources and Import Plan](asset_sources.md). The Modern Emerald source
commit was audited, but the repository did not expose a top-level license file
during the audit; keep that as a master-adoption risk.

Persistent choice storage is not required for the first runtime slice. If a
persistent setting is needed, prefer an explicitly allocated event var / runtime
option owner rather than adding new SaveBlock fields.

## Implemented MVP Choices

Trainer and Wild BGM are configured independently.

Current choices include existing local battle songs plus imported BW/BW2, DPPt,
Platinum, and HGSS draft tracks:

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

## Non-goals

- Do not import additional BGM without a source, license / credit, voicegroup,
  and mGBA validation pass.
- Do not copy audio from external repos without credit and license review.
- Do not replace every trainer's data entry.
- Do not change trainer prize money.
- Do not change encounter music unless this is explicitly selected.
- Do not add a full Options menu page in the first slice.
- Do not combine this with trainer / NPC sprite expansion.

## Why This Feature

The current debug sound menu exists, but it is ID-driven and not organized by
gameplay use. Battle BGM is also selected by battle code, not map music, so a
simple map or script BGM change does not solve the actual request.

This feature gives a focused implementation target: make battle BGM easy to
preview and override first, then decide whether to import new battle tracks.

## Related Docs

- [Investigation](investigation.md)
- [MVP Plan](mvp_plan.md)
- [Implementation](implementation.md)
- [Risks](risks.md)
- [Asset Sources and Import Plan](asset_sources.md)
- [Test Plan](test_plan.md)
- [Future Runtime Handoff](future_runtime_handoff.md)
- [Trainer Battle Reward and Audio Flow](../../flows/trainer_battle_reward_audio_flow_v15.md)
- [How to add or change BGM](../../tutorials/how_to_add_bgm.md)
