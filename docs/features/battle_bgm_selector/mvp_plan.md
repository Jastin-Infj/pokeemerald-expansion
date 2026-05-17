# Battle BGM Selector MVP Plan

## MVP Scope

The MVP is a battle BGM target selector, not a full music import feature.

| Item | MVP decision |
|---|---|
| Track source | Existing `MUS_*` constants only. |
| Entry point | Debug menu entry preferred; safe test NPC acceptable. |
| UI | Small readable list menu, not numeric ID entry. |
| Preview | A button previews selected choice / track. |
| Apply | Selector applies independent Trainer and Wild choices for future battles in the current session. |
| Persistence | Not required in first slice. |
| Save layout | No SaveBlock layout changes. |
| New music assets | Not MVP. |
| Encounter cue | Not MVP; trainer eye-contact music should remain a separate setting. |
| Victory BGM | Not MVP. |

## Implemented Choice Direction

The implemented MVP moved from broad profiles to independent Trainer/Wild BGM
choices. Each target can choose from a curated list of existing battle tracks.

Current selectable tracks include Hoenn, FRLG/Kanto, Frontier, villain, rival,
legendary, unused Legend Beast battle tracks that already exist locally, and
the imported BW/BW2, DPPt, Platinum, and HGSS draft tracks.

## External Import Follow-up Scope

The first new-BGM follow-up began with BW/BW2, then expanded to selected DPPt,
Platinum, and HGSS battle subsets after the larger bank dependencies were
mapped:

| Item | Decision |
|---|---|
| Imported tracks | `mus_bw_vs_iris`, `mus_bw_vs_legend`, DPPt battle / Galactic / rival / legendary tracks, Platinum battle tracks, and HGSS Johto/Kanto/legendary battle tracks. |
| Source | Modern Emerald commit `7ca11b375bc68caee94791d294af75ee71d1ddba`. |
| Voicegroups | Numeric Modern voicegroups `001`, `002`, `005`-`009`, and `191`-`275`. |
| Direct samples | Adds the `.aif` direct sound source samples required by the DPPt / HGSS bank closure. |
| Tooling | Uses local `mid2agb` plus imported `tools/aif2pcm` for `.aif` direct sound samples. |
| Selector surface | Add BW/BW2, DPPt, Platinum, and HGSS choices to the existing Trainer/Wild selector categories. |
| Save layout | No changes. |
| Master adoption risk | Source repository license / permission status is unresolved. |

## Entry Point Options

| Option | Pros | Cons | Decision |
|---|---|---|---|
| Debug menu entry | No story map edits; fast testing. | Debug UI ownership. | Preferred first slice. |
| Safe test NPC | Closer to player-facing feature. | Requires map/script placement. | Acceptable second path. |
| Options menu | Real setting location. | UI and persistent save ownership are bigger. | Not MVP. |
| PokeNav / Sound Archive page | Polished. | Too much UI for first battle selector. | Future work. |

## Runtime Implementation Notes

- Keep vanilla `GetBattleBGM()` behavior available as `Default`.
- Do not rewrite trainer data just to change BGM.
- Do not import more BGM without a separate MIDI / GBA song assembly asset gate
  and voicegroup dependency audit.
- Add helper functions around battle BGM choice selection rather than spreading checks
  across battle setup code.
- Apply the selector to explicit song IDs passed to battle transition tasks when
  the song belongs to the known trainer or wild/legendary battle set.
- If persistence is added later, update
  [Local Config And Flag Ledger](../../manuals/local_config_and_flag_ledger.md)
  with the chosen var / flag owner.

## Later Phases

1. Persistent Trainer/Wild choice setting.
2. Separate encounter cue setting.
3. Separate victory BGM setting.
4. Additional imported battle tracks only after a fresh dependency and
   permission audit.
5. Expanded DPPt / Platinum / HGSS battle-start validation and loop / volume
   listening passes.
6. Player-facing Sound Archive page.
7. Facility-specific BGM rules for Champions / Frontier-style content.
