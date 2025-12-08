# Repository Guidelines

## Project Context
- Fork of `rh-hideout/pokeemerald-expansion` (`master`). Base is the Japanese “Pocket Monsters Emerald” ROM, decompiled and extended to Gen 9 mechanics.
- Upstream changes fast; names/constants/logic can differ per tag/branch. Cross-check your target branch/changelog and consult upstream issues/forks when behavior differs.

## Upstream & Community
- Primary source: https://github.com/rh-hideout/pokeemerald-expansion — treat as canonical; watch both `master` and `upcoming` (beta, high churn/issues daily).
- Community hub: RHH Discord https://discord.gg/6CzjAG6GZk — active discussion, beta issues, and #pr-discussions threads.

## Project-specific Deviations from upstream expansion
- Scripting:
  - This project does **NOT** use `tools/pory` or `pory.config`.
  - Event scripts are edited directly as `.inc` and assembled as in upstream “modern” config.
  - When proposing new events, prefer reusing existing `.inc` patterns over introducing Poryscript.
- Branch usage in this fork:
  - `master`: tracks upstream `rh-hideout/pokeemerald-expansion` master with minimal changes.
  - `birch_case`: main feature branch for [describe: e.g. story/balance changes].
  - `dev2` (or others): experimental; large refactors or WIP features live here.
  - When generating git commands, **never push or force-push automatically**; only suggest commands.
- Config toggles:
  - Level caps, party syntax and key battle flags are controlled in:
    - `include/config/level_caps.h`
    - `include/config/battle.h`
  - This fork usually keeps:
    - `B_LEVEL_CAP_TYPE = LEVEL_CAP_FLAG_LIST` (or your actual setting)
    - `COMPETITIVE_PARTY_SYNTAX` = [TRUE/FALSE] according to current branch.

## Editing Rules for Codex
- Prefer **small, localized changes** over large refactors.
  - Do not rewrite whole files unless explicitly requested.
  - When adding new code, imitate nearby patterns and naming.
- Always show **minimal diffs**:
  - Output only the functions/structs/blocks that changed, not the entire file.
  - Use comments like `// NEW` or `// CHANGED` when helpful.
- Respect existing style and names:
  - Do not rename functions, structs, or constants unless the user explicitly asks.
  - When in doubt, keep upstream names and patterns.
- Before proposing changes:
  - Briefly restate what the user wants (e.g. “new key item that behaves like X”).
  - Point out any tradeoffs or alternative approaches.

## Balance & Difficulty Philosophy
- Target experience:
  - “Emerald++”: harder and more modern than vanilla Emerald, but not full Smogon ladder.
  - Early-game: mostly simple 2–3 move sets, limited items, little to no EV optimization.
  - Mid/late-game bosses: can use育成論-level sets (proper EV spreads, items, abilities), but avoid pure stall.
- EVs and items:
  - Early routes / minor trainers:
    - Usually no EVs or simple 252 / 252 spreads at most.
    - Basic items only (no Life Orb, Choice items, etc. unless clearly themed).
  - Boss trainers (Gym Leaders, E4, major rivals):
    - May use育成論-based EVs, abilities, and items.
    - Allow 1–2 “signature” sets per boss that closely mirror competitive builds.
- AI friendliness:
  - Avoid pure stall or heavy PP drain as default tools.
  - Gimmicks (Trick Room, weather, setup sweeps) are welcome, but should be telegraphed and answerable by standard play.

## Dangerous Areas – Do NOT touch unless explicitly asked
To avoid breaking saves or core engine behavior, do NOT modify these areas unless the user clearly requests it:
- Save data / saveblock layout:
  - `include/constants/saveblock.h`, `src/save.c`, any `gSaveBlock*` structs.
  - Do not add/remove fields or change struct order on your own.
- Core battle / overworld engine:
  - Low-level engine files such as `src/battle_main.c`, `src/fieldmap.c`, `src/overworld.c`
    should only be touched for the specific mechanic the user asks about.
  - Prefer adding small helpers or using existing hooks over refactoring these files.
- Build system and global config:
  - `Makefile`, `graphics_file_rules.mk`, `audio_rules.mk`, `map_data_rules.mk`, `trainer_rules.mk`,
    and `include/config/*.h` should not be edited “for style”.
  - Do not change optimization flags, link flags, or config toggles unless requested.
- Audio engine:
  - Files such as `sound/*`, `src/m4a.c`, `asm/m4a_1.s` are fragile.
  - Do not alter them except when the user explicitly requests a specific audio fix.
- If you believe touching these areas is necessary, **stop and explain why** first, and propose minimal, localized changes.

## Text & Localization Guidelines
- In-code language:
  - Code comments and identifiers: **English**.
  - In-game text (message boxes, signposts, NPC dialogue): **Japanese**, matching the original Emerald JP tone.
- Adding new text:
  - Add new strings to the appropriate text table or script file instead of inlining large strings in C.
  - Reuse existing message patterns (e.g. common system messages) when possible.
- Character encoding:
  - All new characters must exist in `charmap.txt`.
  - If you see “unknown character U+XXXX” errors, add the mapping in `charmap.txt` first, then use that glyph.
- Style:
  - Keep line lengths and line breaks similar to vanilla Emerald windows to avoid overflow.
  - Avoid overlong multi-line boxes; split scenes into multiple messages if needed.

## Local Dev Environment Snapshot (for debugging/build help)
- OS: Windows 11 + WSL2 (Ubuntu) as primary build environment.
- Toolchain:
  - devkitARM: use the version recommended in upstream `INSTALL.md` (do not downgrade/upgrade on your own).
  - `make`, `gcc-arm-none-eabi`, `binutils-arm-none-eabi`, `libpng-dev`, `python3` installed via apt.
- Repo location (WSL):
  - Typically under `~/dev/pokeemerald-expansion` (do not assume Windows drive paths inside WSL).
- Node / Codex:
  - Node.js managed by `fnm` (or nvm); do not suggest system-wide `apt install nodejs` unless explicitly requested.

## Discord #expansion-get-help Notes
- Export file (when present): `RH Hideout - ★ Expansion ★ - expansion-get-help [1077168246555430962].txt`; context spans many versions—double-check against your branch and prefer recent posts (older advice can be wrong as APIs change). File is a local reference only; do not commit.
- High-signal responders: Alex (often posts fixes/diagnoses), rainonline, asparaguseduardo (Eduardo), and mgriffin frequently deliver reliable answers—prioritize their solutions when skimming.
- Debug logging: `DebugPrintf` only works if you comment out `#define NDEBUG` in `include/config.h`; `MgbaPrintf_` is a test helper.
- Toolchain mismatch: `__getreent`/libc undefined refs reported with older devkitARM/libc; use the supported devkitARM from INSTALL.md.
- De-evolution logic: no reverse pointer in evo data; use a linear search like daycare `GetEggSpecies` rather than hacking extra evo entries.
- Mega evolution items: if swapping required items, set the item’s `holdEffect` to `HOLD_EFFECT_MEGA_STONE` or ME won’t trigger.
- Form-change sprites: multiple form changes (Minior/Cramorant/Castform) sticking on old sprite tracked in issue #8457 (seen on `upcoming`).
- Ability info flags: Water Bubble missing `breakable` flag—add it to the breakable list and tests when auditing ability data.
- DNS/window colors: palette glitches tied to window template sizing; fix the window template width when applying DNS UI palettes.
- Starting items: add key items in `data/scripts/new_game.inc` via the `additem` macro.
- Teachable learnsets by gen: remove moves from `sv.json` (teachable data) to drop Gen 9-only moves; upcoming will ease removals.
- Concrete examples from the export file (verify against your branch before using):
  - 2023-07-16: When removing EVs from `CALC_STAT`, keep the macro signature and only drop the term in the body: change the line inside the macro to `s32 n = (((2 * baseStat + iv) * level) / 100) + 5;` without deleting the `ev` parameter or call sites.
  - 2023-07-20: To make a Rare Candy-style key item not consumed, remove the `RemoveBagItem` call in `ItemUseCB_RareCandy` or guard it with `if (!ItemId_GetImportance(gSpecialVar_ItemId))` before removing the item after the out-of-battle effect runs.
  - 2023-11-01: Species checks need full comparisons; e.g. for Deoxys forms use chained comparisons:
    ```
    if (gBattleMons[gBattlerAttacker].species == SPECIES_DEOXYS
     || gBattleMons[gBattlerAttacker].species == SPECIES_DEOXYS_DEFENSE
     || gBattleMons[gBattlerAttacker].species == SPECIES_DEOXYS_SPEED)
        return TRUE;
    ```
  - 2023-11-01: The infinite repel item uses `OW_FLAG_NO_ENCOUNTER`; give the flag an ID in `include/overworld.h` (not just `flags.h`) or the toggle will no-op.
  - 2023-03-08: `B_FLAG_NO_CATCHING` lives under `include/config/overworld.h` after the split; define the flag there and fix conditionals referencing `B_FLAG_NO_CATCHING_USE` by using `#if B_FLAG_NO_CATCHING == 0` (or defining the `_USE` macro) to avoid build errors.
  - 2023-07-19: Adding the move relearner menu entry requires inserting `MENU_MOVES` into the `enum` in `src/party_menu.c` (include the trailing comma before it, and keep `MENU_FIELD_MOVES` as the last enumerator since field moves are appended by offset). Also add the `MENU_MOVES` entry to `sCursorOptions` and any preset arrays that display it.
  - 2023-02-23: `MgbaPrintf_` links only in tests; for regular debug logs comment out `#define NDEBUG` in `include/config.h` and call `DebugPrintf(...)` instead to avoid `undefined reference to 'MgbaPrintf_'`.
  - 2023-07-16 & 2023-08-19: If linking fails with `gItemIcon_*` or palette symbols (e.g. Lustrous Globe, Berserk Gene), ensure the icon/palette entries remain in `graphics/items.h` and `src/data/item_icon_table.h` (and corresponding gfx exist) when merging feature branches.
  - 2023-11-01: `MoveSelectionDisplayMoveType` needs a local `typeColor` declared and initialized at the top of the function before use; missing that line triggers `typeColor undeclared` build errors.
  - 2023-11-11: For a permanent Trick Room effect, set the timer to `-1` instead of a duration, e.g. `gFieldTimers.trickRoomTimer = -1;`.
  - 2023-12-04: Script var comparisons need the `var()` macro closed on the var name, not the value; e.g. `if (var(VAR_GURU_TRAINER_ITEMS) == 0)` or `compare VAR_GURU_TRAINER_ITEMS, 0` (not `compare VAR_GURU_TRAINER_ITEMS == 0, 0`), which otherwise throws “confusion in formal parameters.”
  - 2024-05-01: To drive dynamic level caps, set `B_LEVEL_CAP_VARIABLE` in `include/config/level_caps.h` (e.g. `#define B_LEVEL_CAP_VARIABLE VAR_UNUSED_0x404E`) and adjust caps in scripts with `setvar VAR_UNUSED_0x404E, <level>`.
  - 2024-06-01: Expanding the Hoenn Dex past Deoxys requires bumping the count macro: `#define HOENN_DEX_COUNT (HOENN_DEX_DEOXYS + 1)` (or higher) so new species indices appear.
  - 2024-06-01: When adding extra fishing slots (3/4/3 per rod) the selection logic works with chained `else if` blocks and updated thresholds; keep the lure swap logic consistent when mirroring slots.
  - 2024-09-01: If `trainerproc` errors on `src/data/battle_partners.party` under MSYS2, disable the new party syntax via `#define COMPETITIVE_PARTY_SYNTAX FALSE` in `include/config/battle.h` (or update the party file to the new syntax) before rebuilding.
  - 2024-12-30: Clearing lingering windows after switching pages in custom UIs: after removing a window, call `FillWindowPixelBuffer(windowId, 0)` or clear the tilemap so the old window’s tiles stop rendering when new templates are loaded.
  - 2025-01-01: To cap quadruple effectiveness at ×3, apply the clamp in `MulByTypeEffectiveness` rather than `GetTypeEffectiveness` (the latter is overworld-only); add a post-calculation check that converts ×4 multipliers to ×3, accounting for third-type modifiers if present.
  - 2025-01-01: Level caps driven by the flag list need `#define B_LEVEL_CAP_TYPE LEVEL_CAP_FLAG_LIST` in `include/config/level_caps.h`; then set the per-flag caps in the list and raise them via flags or scripts.
  - 2025-01-01: Cave-style warps must use a tile with the level-change collision behavior (not a blocked tile); otherwise the warp won’t trigger even if the event is defined.
  - 2025-01-01: Item icon palettes can be generated with the built-in converter: `tools/gbagfx/gbagfx <input>.png <output>.pal`; add the resulting palette alongside the icon in `graphics/items.h` and `src/data/item_icon_table.h`.
  - 2025-01-01: Summary screen page tilemaps (e.g. `graphics/summary_screen/page_info.bin`) are authored in TilemapStudio using `tiles.png` as a GBA 4bpp tileset, then exported to `.bin` and referenced from the UI source.
  - 2025-01-01: If `gbagfx` is not found on PATH, call it via its repo-local path `tools/gbagfx/gbagfx` for any palette/tileset conversions.
  - 2025-02-01: Multi-battle gimmick selection bug—`ShouldTrainerBattlerUseGimmick` misread the partner’s trainer ID. Fix by selecting `trainerId`/`partyIndexes` based on `BATTLE_TYPE_INGAME_PARTNER` and `BATTLE_TYPE_TWO_OPPONENTS`, and subtract `MULTI_PARTY_SIZE` for right-side/partner battlers when indexing `GetTrainerPartyFromId(trainerId)[partyIndexes]`.
  - 2025-02-01: `B_POSITION_LEFT_RIGHT` is not defined in some branches; use `B_POSITION_PLAYER_RIGHT` (or equivalent) in the above fix to compile.
  - 2025-03-01: After merging 1.11, regenerate `trainers.h` by touching `trainers.party` (any edit) so `trainerproc` rebuilds; unresolved merges there lead to “million errors from trainers.party.”
  - 2025-04-01: Overworlds missing post-migration often trace to a reverted custom `object_event` struct; reapply the project’s conditional-trainer additions or adopt the new built-in RHH solution instead of mixing both.
  - 2025-06-01: Some tilesets (e.g. Rustboro/Mauville) shipped with incorrect byte sizes; opening/saving the tileset in Porymap (without TLM) rewrites to the correct size as a temporary fix until the tileset is reconverted.
  - 2025-06-01: Party menu build error from `static` in `party_menu.h` presets—remove the `static` qualifier where the upstream header changed to non-static to match symbol visibility and unblock builds.
  - 2025-02-01: Adding new glyphs (e.g. å/ä/ö) requires defining them in `charmap.txt`; unknown characters throw “unknown character U+E5” until mapped.
  - 2025-03-01: Conflicts when merging ipatix audio changes with 1.11—keep the upstream deletions in `m4a_1.s`/`m4a.c` and reapply local tweaks after resolving; recompile to ensure the sound engine still builds.
  - 2025-05-01: Shields Down custom species—extend `IsShieldsDownProtected` and form-change logic to include the new species/forms, and add the unleashed form to the form table so stats/appearance swap when HP drops.
  - 2025-07-01: To lock a door until a variable/flag is set, either swap the door metatile to a non-door behavior via `setmetatile` or place an invisible NPC (see Team Aqua submarine door) to block until the condition clears.
  - 2025-07-01: New map section setup—define a `MAPSEC_*` in `include/constants/region_map_sections.h` (or via Porymap 6 “Locations” tab) and place it on the region map with the Region Map Editor to avoid missing/incorrect mapsec IDs.
  - 2025-08-01: AI sleep handling—`IsBattlerIncapacitated` only flagged Sleep Talk; add Snore (and similar) so AI scores support moves correctly in sleep (e.g. Follow Me/Helping Hand not treated as unusable).
  - 2025-08-01: New TM/HM entries need adding to `include/constants/tms_hms.h` and associated data tables; missing constants cause “additional info” prompts or build errors when introducing custom TMs.
  - 2025-08-01: Triple-layer metatile artifacts (roulette/gacha) stem from bad triple-layer definitions; verify the metatile behaviors/layers for those tiles and reconvert the tileset in Porymap to fix incorrect layer ordering.

## Reference Resources
- https://www.pokecommunity.com/threads/scripting-tutorial.416800/ — PokéCommunity decomposed pokéemerald scripting tutorial (inc-style, expansion-friendly): covers map `scripts.inc` structure and PoryMap label assignment, msgbox/lock/yes-no flows, string buffers for species/player names, branching with gender/random/flags/vars, hide/show objects, give/take items/Pokémon/eggs, applymovements, marts/weather/time/door/fadescreen commands, and displaying sprites/cries/SFX.
- https://www.pokecommunity.com/threads/battle-engine-upgrade.417820/ — PokéCommunity release thread for the battle engine upgrade (became `rh-hideout/pokeemerald-expansion`): feature set includes physical/special split, Fairy type, gen7 moves/abilities/items/learnsets, triple/dual-type handling, double wild battles, ability pop-ups, exp-on-catch, faster intros, Mega support, mid-battle trainer messages, custom multi battles, and config toggles in `include/constants/battle_config.h`. Thread tips: use git merging instead of manual file copy; mid-battle messages are defined in `src/battle_messages.c` `sTrainerSlides` (trainerId + messages for last mon/last mon low HP/first faint; set fields to NULL to skip); early bug reports flagged missing sandstorm SpDef boost and hail Blizzard accuracy before later fixes.
- https://www.pokecommunity.com/forums/decomp-disassembly-tutorials.475/ — PokeCommunity decomp/disassembly tutorials board; high-signal threads include Simple Modifications Directory (grab-bag patches), toolchain/UI utilities (PoryMap, Poryscript, porypal), installer/build guides (Easy Decomp Installer, DST, WSL/MSYS2/Cygwin how-tos), and portable modules/features: bag pocket expansion, TM/HM expansion/reusables, Pokémon/item expansions and USUM data files, type addition (Fairy), better RNG, move tutor expansion, custom evolutions, dynamic trainer levels/level caps/boss balancing, overworld count expansion, map section/name popup tutorial, Eon Flute soaring, affine animations, debug menu, map previews, BF prize edits, stack size tweaks, intro sprite edit, 80×80 sprites. Many threads target FireRed but concepts/patches often portable to Emerald-expansion—verify against current `master`/`upcoming` before applying.
- https://wiki.xn--rck7ae4l.com/wiki/%E3%83%9D%E3%82%B1%E3%83%A2%E3%83%B3%E3%81%AE%E5%A4%96%E5%9B%BD%E8%AA%9E%E5%90%8D%E4%B8%80%E8%A6%A7 , https://wiki.xn--rck7ae4l.com/wiki/%E3%81%A8%E3%81%8F%E3%81%9B%E3%81%84%E3%81%AE%E5%A4%96%E5%9B%BD%E8%AA%9E%E5%90%8D%E4%B8%80%E8%A6%A7 , https://wiki.xn--rck7ae4l.com/wiki/%E3%82%8F%E3%81%96%E3%81%AE%E5%A4%96%E5%9B%BD%E8%AA%9E%E5%90%8D%E4%B8%80%E8%A6%A7 — Official JP wiki foreign-name lists (Pokémon, Abilities, Moves) useful for JP→EN naming while coding.
- https://wiki.xn--rck7ae4l.com — JP official wiki hub; category lists cover moves/abilities/items/locations/trainer classes/types/etc. Use for JP→EN term lookup when specs arrive in Japanese.
- https://bulbapedia.bulbagarden.net — Bulbapedia hub; use the main site search and category indexes (moves, abilities, items, Pokémon by Dex, types, locations, mechanics) to translate JP specs and sanity-check data. Quick links: https://bulbapedia.bulbagarden.net/wiki/List_of_Pok%C3%A9mon_by_National_Pok%C3%A9dex_number , https://bulbapedia.bulbagarden.net/wiki/List_of_items .
- https://latest.pokewiki.net — JP competitive compendium with gens 1–9: per-mon singles/doubles roles, pros/cons, differentiation points, and historical notes. Navigation links to past-gen archives.
- Past-gen competitive mirrors (特に https://previous.pokewiki.net/ を最優先で参照): https://previous.pokewiki.net/ , https://1st.pokewiki.net/ , https://2st.pokewiki.net/ , https://3st.pokewiki.net/ , https://4st.pokewiki.net/ , https://5st.pokewiki.net/ , https://6st.pokewiki.net/ , https://7st.pokewiki.net/ , https://8st.pokewiki.net/ , https://9st.pokewiki.net/ for gens 1st–9th legacy snapshots (ordinal表記: 1st, 2nd, 3rd, 4th, 5th, 6th, 7th, 8th, 9th); cross-check with Smogon Dex by gen (e.g., https://www.smogon.com/dex/rs/, /dp/, /bw/, /xy/, /sm/, /ss/, /sv/pokemon/) for English move/EV set context and current SV baselines.
- https://github.com/smogon/pokemon-showdown — Showdown simulator and Smogon rulesets; useful to cross-check current competitive mechanics, learnsets, formats, and sets when aligning trainer data or mechanics with modern metas.
- https://sv.pokedb.tokyo/ — JP usage stats and builds from in-game SV seasons; singles/doubles rates shift each season with format bans and roster changes (some mons missing in Gen 9). When a target mon is absent or underrepresented, look back to Gen 8/7 usage and historical roles to understand why it fell off before adopting it; prioritize context from past metas to avoid poor fits.
- High-volume set/breeding writeups: https://yakkun.com/sv/theory/ (ポケモン徹底攻略 “PokeTei”育成論、投稿数最多クラスで不採用理由まで言及しがち。考察重視でネタ枠不要), https://www.smogon.com/dex/sv/pokemon/ (Smogon curated sets in EN). Treat PokeTei as the primary build reference—even for low-usage mons—to understand why they are or aren’t viable; then cross-check usage trends (sv.pokedb.tokyo/Showdown stats) and historical roles before locking trainer builds.
- https://github.com/pret/pokeemerald/wiki/Tutorials — vanilla Emerald decomp (non-expansion); good for locating original behaviors and small features that may already be upstreamed here.
- https://github.com/TeamAquasHideout/Team-Aquas-Asset-Repo/wiki/Feature-Branches — “The Pit” feature branches with high-quality modules; many additions for v13 were adapted from here.
- https://github.com/TeamAquasHideout/pokeemerald — Team Aquas base repo; browse branch diffs for self-contained features.
- https://github.com/PCG06/pokeemerald/tree/move_relearner — reference for move relearner and related logic; some pieces are already in expansion.
- Expansion-based game forks worth inspecting for ideas/diffs: https://github.com/resetes12/pokeemerald , https://github.com/Pokabbie/pokeemerald-rogue/ , https://www.pokecommunity.com/threads/the-pit-v2-roguelite-style-hack.528423/ , https://www.pokecommunity.com . FireRed and RPGXP projects can also be mined for portable logic.

## Data Source Priorities
- For competitive sets and trainer parties: prioritize JP 育成論 (latest/past pokewiki) + ポケ徹育成論 for roles/moves/EVs, then cross-check with EN sources (Smogon Dex + Showdown data). Use multiple sources; never rely on a single post.
- Modern stats inform intent, but adapt to Emerald pacing and repo availability. If mechanics/items/moves are missing, pick the closest equivalent and note the substitution.

## Competitive Set Research – 育成論 Wiki Deep Dive
- When designing or reviewing Pokémon sets (moves/EVs/items/roles), always mine both JP 育成論 sources and EN competitive sites, summarize, and then propose. Do not copy a single set; compare multiple guides and form your own conclusion.
- Primary 育成論 sources (JP):
  - 最新世代: https://latest.pokewiki.net/
  - 過去世代: https://1st.pokewiki.net/ , https://2st.pokewiki.net/ , https://3st.pokewiki.net/ , https://4st.pokewiki.net/ , https://5th.pokewiki.net/ , https://6st.pokewiki.net/ , https://7st.pokewiki.net/ , https://8st.pokewiki.net/ , https://9st.pokewiki.net/
  - ポケモン徹底攻略 育成論: 現行 https://yakkun.com/sv/theory/ , 過去世代例 https://yakkun.com/swsh/theory/
- Cross-check with EN meta:
  - Smogon Dex per gen: https://www.smogon.com/dex/rs/pokemon/ through /sv/pokemon/
  - Showdown rules/data: https://github.com/smogon/pokemon-showdown
- Per-Pokémon workflow (for every species touched in parties/learnsets/recommendations):
  1. Locate pages: at least one 育成考察Wiki page and one ポケ徹育成論 page for the same mon/role/gen; open the matching Smogon Dex page if it exists.
  2. Extract roles and patterns: identify major roles (examples: 受け, 起点づくり, 高速アタッカー, トリルエース, 壁サポート, 天候エース) and repeated choices (key moves, items, EV spreads, natures). Ignore ネタ/ロマン unless explicitly requested.
  3. Summarize in English for the project: for each viable role, write a short role label plus moves (with alternatives if sources differ), item, nature, EVs, ability, and any typing/tera-synergy notes. Keep this as your own summary, not copied text.
  4. Adapt to pokeemerald-expansion constraints: replace missing moves/items/mechanics with nearest equivalents and briefly note the substitution rationale; keep Emerald pacing in mind.
  5. Output format when asked for sets:
     ```
     [Role] Physically bulky Dragon Dance sweeper
     - Moves: Dragon Dance / Outrage / Earthquake / Fire Punch
     - Item: Leftovers / Lum Berry
     - Ability: Intimidate
     - Nature: Jolly
     - EVs: 252 Atk / 4 Def / 252 Spe
     - Notes (JP育成論 + Smogon summary):
       * JP育成考察Wiki: DD + dual-STAB with Fire coverage for Steels.
       * PokeTei & Smogon: Jolly max Speed to outspeed base 100s after one DD.
       * Adjust coverage if the exact move is absent in this branch.
     ```
  6. When info is sparse: check older generations’ 育成考察Wiki and Smogon for historical roles; mark the mon as niche/low-usage and state that the set is extrapolated.
- Behavior rules: never stop at a single source; prefer high-usage/high-vote builds; when given a custom idea, still run the above research and confirm alignment or warn about common pitfalls from the sources.

## Task Playbooks (trainer sets/parties)
- Follow these when responding to user requests about sets or trainer parties. Use the Data Source Priorities and Competitive Deep Dive rules above.

### Competitive Deep Dive (singles, pokeemerald-expansion adaptation)
- Inputs: Species (JP/EN name), target meta: modern singles baseline → adapt to pokeemerald-expansion.
- Steps:
  1. Check JP 育成論 sources (latest/past pokewiki + ポケ徹育成論) and summarize main viable roles only (no meme sets).
  2. Cross-check roles with Smogon Dex and Pokemon Showdown data.
  3. Reconcile differences and write a short EN summary of main roles, typical moves, EVs, natures, items, abilities.
  4. Verify which moves/items/abilities exist in this repo; note any missing.
  5. Propose 2–3 pokeemerald-expansion-friendly sets.
- Output format:
  ```
  ## 1. Sources checked
  - JP育成論: ...
  - EN competitive: ...

  ## 2. Role summary
  (日本語でもいいので役割を箇条書きで)

  ## 3. Recommended sets for pokeemerald-expansion
  (それぞれ Role / Moves / Item / Ability / Nature / EVs / Notes を書く)

  ## 4. Implementation notes
  - Files to edit in this repo
  - Any limitations or deviations from modern meta
  ```

### Trainer Party Redesign (育成論-based)
- Inputs: Trainer ID/name, current party snippet (from trainer_parties), target difficulty (early/mid/post-game boss), format (singles/doubles).
- Requirements:
  1. For each species used or added, run the 育成論 Deep Dive: JP 育成論 Wiki + ポケ徹 + Smogon/Showdown; summarize roles/typical sets; adapt to repo availability.
  2. Build a coherent party with a clear concept (sand offense, bulky balance, Trick Room, etc.), challenging but reasonable for the game stage; one spicy pick allowed if still sensible.
- Output:
  ```
  ### 1. Concept
  - Overall team concept and win condition
  - Why this fits this trainer and game stage

  ### 2. Per-Pokémon sets
  For each Pokémon:
  - Species:
  - Role:
  - Moves:
  - Item:
  - Ability:
  - Nature:
  - EVs:
  - Notes (which育成論/Smogon ideas this is based on, and what was adapted)

  ### 3. Trainer party implementation
  - Proposed trainer.parties entries or C struct snippet in code form
  - Mention any constants or files to touch

  ### 4. Balance notes
  - What kinds of player teams this beats/loses to
  - Any potential frustration points (accuracy spam, PP stall, etc.)
  ```

### Trainer Party Review (light deep dive)
- Inputs: Existing trainer party definition.
- Steps:
  1. For each Pokémon, run a light 育成論 deep dive: main viable roles from JP 育成論 + Smogon; check whether current moves/EVs/items fit a standard/reasonable role.
  2. Classify each as “Close to standard”, “Reasonable but off-meta”, or “Meme / incoherent”.
  3. Propose minimal fixes (adjust moves/EVs/items; replace species only if necessary while keeping theme/difficulty).
- Output:
  ```
  ## 1. High-level verdict
  (このトレーナーの構成がどんな印象か)

  ## 2. Per-Pokémon review
  For each:
  - Current role & set interpretation
  - Comparison with育成論/Smogon
  - Issues
  - Minimal fix proposal

  ## 3. Updated trainer party snippet
  (修正後の trainer パーティ定義をコードで出力)
  ```

## Project Structure & Module Organization
- Core code in `src/` (C) and `include/`; shared constants in `constants/`; assembly overlays in `asm/`; data tables in `data/`.
- Assets: `graphics/`, `sound/`, and rules files (`graphics_file_rules.mk`, `audio_rules.mk`, `map_data_rules.mk`, `trainer_rules.mk`). Outputs land in `build/<target>/` plus `pokeemerald.gba`/`.elf`.
- Tests live in `test/`; tools and scripts in `tools/` and `dev_scripts/`. Docs in `docs/` (see `docs/tutorials/how_to_testing_system.md`).

## Build, Test, and Development Commands
- `make` — build the ROM (add `-j$(nproc)` to parallelize). Flags: `DEBUG=1`, `RELEASE=1`, `COMPARE=1`, `LTO=1`.
- `make check -j` — run mgba-based tests.
- `make check TESTS="Spikes"` — filter tests; same filter works with `make pokeemerald-test.elf` to build an mGBA test ROM.
- `make debug` — emit `pokeemerald.elf` with debug symbols for GDB/mGBA.
- `make -j12` (or `-j$(nproc)`) stability: if little/no output after ~5s, interrupt and rerun; if logs stream, let it finish. After aborted runs, delete zero-byte `.o` files (`find build/emerald -name '*.o' -size 0 -delete`) or `make clean` before retrying to avoid undefined references.

## Coding Style & Naming Conventions
- Follow `docs/STYLEGUIDE.md`: functions/structs `PascalCase`; variables/fields `camelCase`; globals `gPrefix`, statics `sPrefix`; macros/constants `CAPS_WITH_UNDERSCORES`.
- C/C headers use 4 spaces; `.s` and `.inc` use tabs. Place braces on new lines after control statements, with a space before parentheses (`if (cond)`).
- Comments should record intent/behavior rather than restating code; use `// ` for short notes and block comments for deeper context.

## Testing Guidelines
- Prefer automated coverage in `test/` using `GIVEN/WHEN/SCENE`; mirror player-visible effects. Prefix test names with the mechanic for easy filtering.
- Document prerequisites with `ASSUME(...)`; mark known regressions with `KNOWN_FAILING; // #issue`.
- For visual/debug investigation, build `pokeemerald-test.elf` and inspect in mGBA before finalizing assertions.

## Commit & Pull Request Guidelines
- Use concise commit titles (history often follows `Summary (#PR)`). Keep commits scoped.
- Target `master` for fixes; `upcoming` for new features/large changes. Work on topic branches.
- Run `make` and `make check -j` before opening a PR; note any config toggles or skipped tests. Avoid force-pushing after review begins; append commits instead.
- PR description should include scope, repro steps, linked issues/Discord context, and screenshots or notes when altering visible behavior.
