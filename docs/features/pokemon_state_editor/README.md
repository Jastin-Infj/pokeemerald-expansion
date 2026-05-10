# Pokemon State Editor Expansion

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-10 |
| Baseline | `master` `8502839610`; `git describe` = `expansion/1.15.2-40-g8502839610` |
| Code status | Planned; docs-only feature log |
| Provenance | User request and local code/docs review |

## Goal

既存の state editor / Uroxido 系の個体値編集導線を土台に、既存 Pokemon の
状態を広く編集できる UI を作る。

ユーザー要望:

- IV / EV だけでなく、産地、性格、特性を編集できる。
- 性別も可能なら変更できる。
- なつき度も変更できる。
- 1 画面に詰め込まなくてよい。複数 menu / submenu に分けてよい。
- 既に他者実装済みの editor 部分があるなら、その branch / prototype を取り込む前提で影響を整理する。

この docs は実装前 feature log。実装 branch では、実際の state editor / Uroxido
source の file path と差分を `implementation.md` へ追記する。

## Current Baseline

既存 debug menu には近い部品がある。

| Area | Existing entry / symbol |
|---|---|
| Give Pokemon complex | `src/debug.c` `DebugAction_Give_PokemonComplex` |
| Give Pokemon nature | `DebugAction_Give_Pokemon_SelectNature` |
| Give Pokemon ability | `DebugAction_Give_Pokemon_SelectAbility` |
| Give Pokemon IV / EV | `DebugAction_Give_Pokemon_SelectIVs`, `DebugAction_Give_Pokemon_SelectEVs` |
| Edit Pokemon submenu | `DebugAction_ExecuteScript`, `Debug_EventScript_SetHiddenNature`, `Debug_EventScript_SetAbility`, `Debug_EventScript_SetFriendship` |
| Check EV / IV | `Debug_EventScript_CheckEVs`, `Debug_EventScript_CheckIVs` |
| Pokemon data accessors | `src/pokemon.c` `GetMonData`, `SetMonData`, `GetMonPersonality`, `GetMonGender` |
| Primary data constants | `include/pokemon.h` `MON_DATA_*` |

Relevant fields:

| Field | Storage / notes |
|---|---|
| IVs | `MON_DATA_HP_IV` ... `MON_DATA_SPDEF_IV`, `MON_DATA_IVS`; 0..31. |
| EVs | `MON_DATA_HP_EV` ... stat EVs; total and per-stat caps must be enforced. |
| nature | personality-derived via `GetNatureFromPersonality`, plus `MON_DATA_HIDDEN_NATURE` override support. |
| ability | `MON_DATA_ABILITY_NUM`; must map to valid species ability slots. |
| gender | derived from species gender ratio and personality low byte. |
| friendship / なつき度 | `MON_DATA_FRIENDSHIP`, `MAX_FRIENDSHIP 255`. |
| met data / 産地 | `MON_DATA_MET_LOCATION`, `MON_DATA_MET_LEVEL`, `MON_DATA_MET_GAME`. |
| OT gender | `MON_DATA_OT_GENDER`; separate from Pokemon gender. |
| personality | `MON_DATA_PERSONALITY`; affects nature, gender, shininess, Unown letter, Spinda spots, and some forms. |

## Proposed Shape

MVP は debug / utility feature として、既存 party Pokemon を対象にする。
PC box Pokemon 対応は Phase 2 として分けると安全。

Suggested menu split:

| Menu | Fields |
|---|---|
| Stats | IVs, EVs, level, current HP restore/recalc policy. |
| Identity | nature, ability slot, gender, personality reroll / hidden nature policy. |
| Origin | met location, met level, met game, ball if needed later. |
| Bond | friendship / なつき度. |
| Moves | existing Move Relearner / future unified move editor link. |
| Battle gimmicks | Tera type, Dynamax level, Gigantamax factor if this editor owns those later. |

Do not force all fields into one screen. A dense debug menu is acceptable if each submenu has
clear validation and cancel behavior.

## Data Policy

| Topic | Decision draft |
|---|---|
| Save layout | No new saved fields for MVP. Edit existing Pokemon substruct data only. |
| Apply model | Edit a temporary buffer, then commit on confirm. Cancel discards. |
| Party vs box | Party first. Box support needs PC selection / box summary return path review. |
| Nature editing | Prefer `MON_DATA_HIDDEN_NATURE` for simple user-facing nature change if the project treats it as canonical. Raw personality rewrite is a separate advanced option. |
| Gender editing | Requires personality reroll that satisfies species gender ratio. Genderless / fixed-gender species must reject incompatible choices. |
| Ability editing | Present valid ability slots from `gSpeciesInfo[species].abilities`; reject `ABILITY_NONE` unless the species truly has no slot. |
| IV editing | Clamp 0..31 and recalculate stats after apply. |
| EV editing | Enforce per-stat and total EV caps according to current config. |
| Friendship editing | Clamp 0..255. |
| Origin editing | Use valid mapsec / met location values; avoid raw arbitrary IDs in normal UI. |

## Impact Surface

| File / area | Impact |
|---|---|
| `src/debug.c` | Existing Give Pokemon complex and Edit Pokemon menu can be reused or refactored. |
| `src/pokemon.c` | `SetMonData`, personality generation, stat recalculation, gender / nature helpers. |
| `include/pokemon.h` | `MON_DATA_*` constants and helper prototypes. |
| `include/constants/pokemon.h` | nature, friendship, gender constants. |
| `src/party_menu.c` | Party selection entry if editor opens from party menu. |
| `src/pokemon_summary_screen.c` | Summary entry / return path if editor opens from summary. |
| `src/chooseboxmon.c` / PC storage | Needed for Phase 2 box support. |
| `docs/overview/champions_training_ui_feasibility_v15.md` | EV/IV/nature/moveset editor overlap. |
| `docs/features/unified_move_relearner/` | Moves submenu may call into unified move editor / relearner. |

## Personality Coupling

性格と性別は単純な independent field ではない。

- Nature is `personality % NUM_NATURES` unless hidden nature override is used.
- Gender is derived from species gender ratio and personality low byte.
- Shiny state depends on personality and OT id.
- Unown letter and Spinda spots depend on personality.
- Some form / visual behavior can depend on personality or gender.

Therefore there are two viable policies:

| Policy | Pros | Risks |
|---|---|---|
| Hidden nature + ability slot + friendship edits only | Simple, low risk, avoids personality side effects. | Gender / shiny / personality-driven visuals are not fully editable. |
| Controlled personality reroll | Can support gender and raw nature changes. | Must preserve or intentionally change shiny / Unown / Spinda / form expectations. |

MVP recommendation: use hidden nature for nature, and add gender editing only after a
`RerollPersonalityForSpeciesGenderNature()` helper is designed and tested.

## Risks

| Risk | Severity | Notes |
|---|---|---|
| Invalid personality rewrite | High | Can unintentionally change nature, gender, shiny, form visuals, Unown letter, or Spinda spots. |
| Invalid ability slot | Medium | Ability number must map to species ability slots and hidden ability behavior. |
| Stat desync | Medium | IV/EV/nature/level edits must recalculate max HP/current HP safely. |
| Box support corruption | Medium | Box Pokemon editing needs box data accessors and PC return path; party-only MVP is safer. |
| Origin ID confusion | Medium | Raw met location IDs are not user-friendly and can show invalid summary text. |
| Existing editor merge unknown | Medium | Uroxido/state-editor source is not identified in this branch yet. It must be inspected before implementation. |

## Validation Plan

Minimum local checks after implementation:

- `rtk git diff --check`
- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- `rtk make -j16 -O check` or focused Pokemon/stat tests if helper boundaries exist
- mGBA Live check through party editor path, summary display, and at least one battle stat check

Manual cases:

- Edit all six IVs and confirm summary/stat calculation reflects the change.
- Edit nature and confirm stat modifiers / summary text match the selected nature.
- Edit ability and enter battle to confirm the selected ability is active.
- Edit friendship to 0 and 255 and confirm summary / friendship checker behavior.
- Edit met location and confirm summary origin text remains valid.
- Try changing gender on fixed-gender and genderless species; invalid choices are rejected.
- Cancel from each submenu leaves the Pokemon unchanged.

## Open Questions

- What exact branch / files contain the existing state editor / Uroxido implementation?
- Should the feature live under debug menu only, or become a normal in-game utility menu later?
- Should nature editing use hidden nature, personality rewrite, or both as separate advanced options?
- Should gender editing preserve shiny status if the original Pokemon is shiny?
- Should box Pokemon editing be MVP or Phase 2?
- Should moves be edited through Unified Move Relearner, direct move slot editor, or both?
