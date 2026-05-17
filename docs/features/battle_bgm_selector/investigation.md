# Battle BGM Selector Investigation

## Questions

- Where is battle BGM selected?
- Is trainer `Music:` the battle BGM or only the encounter cue?
- Which existing battle tracks are already available?
- Is there already a debug sound menu?
- Can this be implemented without new save layout?
- Which paths bypass the normal battle BGM selector?

## Read-only Search Notes

Commands used during planning:

```sh
rg -n "GetBattleBGM|PlayBattleBGM|PlayMapChosenOrBattleBGM|CreateTask_PlayMapChosenOrBattleBGM|GetTrainerEncounterMusicId|encounterMusic|TRAINER_ENCOUNTER_MUSIC" src include
rg -n "MUS_RG_VS|MUS_VS_|MUS_ENCOUNTER_|MUS_RG_ENCOUNTER" include/constants/songs.h src/pokemon.c src/battle_setup.c src/debug.c
rg -n "gTrainerClasses|trainer prize|money reward|encounterMusic" docs src include
```

## Candidate Symbols / Areas

| Area | Candidate file / symbol | Confidence | Notes |
|---|---|---|---|
| Battle BGM decision | `src/pokemon.c`, `GetBattleBGM` | High | Main selector for wild, trainer, legendary, link, Frontier, Hoenn, and Kanto / FRLG battle music. |
| Battle BGM playback | `src/pokemon.c`, `PlayBattleBGM` | High | Calls `ResetMapMusic`, stops all m4a players, then plays `GetBattleBGM()`. |
| Explicit battle start song | `src/pokemon.c`, `PlayMapChosenOrBattleBGM` | High | If `songId` is nonzero, plays that song instead of `GetBattleBGM()`. |
| Delayed explicit battle start song | `src/pokemon.c`, `CreateTask_PlayMapChosenOrBattleBGM` | High | Battle Dome-style task wrapper for explicit song ID or fallback to `GetBattleBGM()`. |
| Legendary / special battle setup | `src/battle_setup.c` | High | Some special battle starts pass explicit `MUS_*` IDs to `CreateBattleStartTask`, so they may bypass a simple `GetBattleBGM` override. |
| Trainer encounter cue | `src/battle_setup.c`, `PlayTrainerEncounterMusic` | High | Maps trainer `.encounterMusic` to `MUS_ENCOUNTER_*`. This is the pre-battle eye-contact cue, not the battle BGM. |
| Trainer data field | `src/data/trainers.h`, `.encounterMusic` | High | Generated from trainer data. Do not treat this as battle BGM. |
| Song constants | `include/constants/songs.h` | High | Defines Hoenn `MUS_VS_*`, FRLG `MUS_RG_VS_*`, encounter cues, victory themes, and map tracks. |
| Debug sound menu | `src/debug.c`, `DebugAction_Sound_MUS` | High | Existing numeric music selector. Useful reference, but too ID-focused for a battle profile UI. |
| BGM tutorial | `docs/tutorials/how_to_add_bgm.md` | High | Documents song constants, song table, MIDI files, voicegroups, and map/script music ownership. |

## Existing Battle Track Families

Confirmed existing battle-oriented constants include:

| Family | Examples | Notes |
|---|---|---|
| Hoenn wild / trainer | `MUS_VS_WILD`, `MUS_VS_TRAINER` | Current default outside Kanto where no special class applies. |
| Hoenn boss | `MUS_VS_GYM_LEADER`, `MUS_VS_ELITE_FOUR`, `MUS_VS_CHAMPION`, `MUS_VS_RIVAL` | Selected by trainer class in `GetBattleBGM`. |
| Hoenn villain / legendary | `MUS_VS_AQUA_MAGMA`, `MUS_VS_AQUA_MAGMA_LEADER`, `MUS_VS_REGI`, `MUS_VS_KYOGRE_GROUDON`, `MUS_VS_RAYQUAZA`, `MUS_VS_MEW` | Some special battles are selected through battle setup code, not only the class switch. |
| Frontier | `MUS_VS_FRONTIER_BRAIN` | Used by Frontier Brain trainer classes. |
| FRLG / Kanto | `MUS_RG_VS_WILD`, `MUS_RG_VS_TRAINER`, `MUS_RG_VS_GYM_LEADER`, `MUS_RG_VS_CHAMPION`, `MUS_RG_VS_DEOXYS`, `MUS_RG_VS_MEWTWO`, `MUS_RG_VS_LEGEND` | Existing source already uses some Kanto tracks by current region or FRLG classes. |

## Important Separation

Trainer battle presentation has several separate audio decisions:

| Decision | Current owner | Notes |
|---|---|---|
| Eye-contact / encounter cue | `src/battle_setup.c`, `PlayTrainerEncounterMusic` | Uses `.encounterMusic`. |
| Battle BGM | `src/pokemon.c`, `GetBattleBGM` plus special setup paths | Main target for this feature. |
| Victory BGM | `src/battle_main.c` victory branches | Not first slice. |
| Prize money | `src/battle_main.c`, `gTrainerClasses`; `src/battle_script_commands.c`, `GetTrainerMoneyToGive` | Not part of this feature. |

See [Trainer Battle Reward and Audio Flow](../../flows/trainer_battle_reward_audio_flow_v15.md)
for the full source map.

## Initial Implementation Shape

The safest first runtime branch should:

- add a small battle BGM choice table;
- leave vanilla `GetBattleBGM()` as the default;
- add debug/test selectors that update temporary Trainer and Wild choices
  separately;
- route `GetBattleBGM()` through the selected choice when the vanilla song is a
  known trainer or wild battle song;
- route explicit `CreateBattleStartTask(..., songId)` battle songs through the
  same classifier when the song is known.

## Open Questions

- Should the first implementation store the selected choices persistently?
- If persistent, which event var or runtime rule option owns it?
- Should encounter cues be selectable separately from battle BGM?
- Should victory BGM be selectable separately later?
