# Smart Gimmick AI

Smart Gimmick AI makes trainer-owned gimmicks behave like strategic resources instead of automatic first-turn buttons.

## Status

| Field | Value |
| --- | --- |
| Runtime shelf | `shelf/runtime-1.16.1/smart-gimmick-ai` at `e71c48c886` |
| Code status | Accepted interim-complete runtime snapshot; further planner work moves to a new branch |
| Completion code snapshot | `46ed9d403f` plus master handoff ancestry merge `dd55f9a45d` |
| Feature PR | Closed PR #72; retained as evidence and never merged into `master` |
| Primary docs | `docs/tutorials/ai_flags.md`, this folder |
| Main flags | `AI_FLAG_SMART_GIMMICK`, `AI_FLAG_GIMMICK_ENV_TERA_ONLY`, `AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY`, `AI_FLAG_GIMMICK_ENV_DYNAMAX_TERA`, `AI_FLAG_GIMMICK_ENV_ALL`, `AI_FLAG_GIMMICK_ENV_INVERSE_BATTLE`, `AI_FLAG_AGGRESSIVE_GIMMICK`, `AI_FLAG_READ_PLAYER_MOVE` |

## Master Handoff Scope

The `master` handoff for this feature remains documentation-only. Runtime
implementation, tests, generated data, and non-Lua helper tools stay on the
feature or a later integration branch. A feature PR targeting `master` is a
review shelf and is not permission to merge runtime files into `master`.

Before any master handoff or merge, verify that the branch contains only
Markdown documentation, `AGENTS.md` when workflow rules change, and explicitly
approved Lua automation. Source, include, data, test, graphics, generated, and
non-Lua tool changes are not eligible for the docs-only master path.

The next runtime line re-applies this completed Smart AI shelf onto a new
integration branch. It must take Box NPC Party Pool before Battle Team Boxes so
the registered teams retain their required party-copy and debug-battle base.

## Runtime Intent

- Trainer data says a gimmick is available; smart AI decides whether this turn is worth spending it.
- Large-scale runtime knowledge collection starts from the local expansion catalog, then adds Pokemon Wiki, VGC / official tournament, Champions usage, PartyGen, and observed-history adapters with explicit source and confidence tags.
- Tera should be held until there is offensive payoff, defensive payoff, or a specific target interaction.
- Dynamax should be held unless the AI has last-Pokemon pressure, survival pressure, KO conversion, or a Max Move board payoff.
- Dynamax can also be spent defensively to keep a selected damaging move live through known or predicted Fake Out-style flinch or Roar / Whirlwind-style phazing.
- Mega Evolution / Ultra Burst may be delayed for setup turns or pre-Mega ability value, but spent for immediate ability, Speed, damage, or defensive payoff.
- Z-Moves remain behind viability and smart timing checks instead of firing only because a Z-Crystal exists.
- Debug gauntlets can opt into `AI_FLAG_AGGRESSIVE_GIMMICK`, which spends legal Dynamax / Tera / Z-Move opportunities more often on real pressure turns while still rejecting status moves, immunities, and very weak attacks.
- Debug gauntlets can opt into `AI_FLAG_READ_PLAYER_MOVE`, which is stronger than ordinary `Omniscient`: after all live player-side commands are confirmed, the AI can use selected moves, selected switches, and selected defensive gimmicks as the current-turn read before returning its own move choice.
- Smart switching can use reserve Pokemon as board-control tools: weather setters, terrain setters, Tailwind, and Trick Room can justify a pivot when they flip the field or speed state.
- Smart switching can also use terrain seeds, status pressure / status prevention, status-benefit switch-ins, and Skill Swap-style ability bridges when those plans create board control.
- Smart switching can read a predicted `Taunt` as a free-positioning turn: a utility-heavy active Pokemon that cannot punish Taunt in place may pivot directly to an attacker, while Pokemon that can already attack, win the matchup, or ignore Taunt stay in.
- In read-mode doubles, an already Choice-locked attacker can pivot when it is being ignored, its locked move no longer makes progress, and the available switch-in is not punished by the confirmed player move set. If that spent Pokemon is being attacked, it can stay as a cushion instead of exposing a reserve.
- Protect is scored as a turn-gain tool, not a passive singles default. Singles Protect needs a payoff such as residual damage, recovery, choice scouting, Substitute threshold, Disable / Encore follow-up, Wish, or Explosion avoidance. Consecutive Protect is penalized for reduced success odds, but a second Protect can still be selected when the payoff remains.

## Current Mega / Ultra Burst Payoffs

`AI_FLAG_SMART_MEGA` now treats Mega Evolution as a timing decision instead of a guaranteed first-turn action. It spends Mega / Ultra Burst when one of these is true:

- The target form's ability creates immediate board value, such as `Shadow Tag` trapping or weather control from `Drought`, `Drizzle`, `Sand Stream`, `Snow Warning`, or `Delta Stream`.
- The target form's Speed is estimated to flip the current matchup.
- The target form's defensive stats are meaningfully better and the current target can otherwise KO the AI.
- The target form's attacking stat meaningfully improves the selected damaging move.

The AI may still delay Mega on setup turns, and may preserve `Air Lock` / `Cloud Nine` while weather is active if the target form does not provide a stronger immediate payoff.

## Current Dynamax Payoffs

`AI_FLAG_SMART_DYNAMAX` now recognizes these additional Max Move reasons:

- Speed control: `Max Airstream` ally Speed boost and `Max Strike` opposing Speed drop.
- Weather control: `Max Flare`, `Max Geyser`, `Max Rockfall`, and `Max Hailstorm`.
- Terrain control: `Max Lightning`, `Max Overgrowth`, `Max Starfall`, and `Max Mindstorm`.
- Side-wide stat pressure: `Max Knuckle`, `Max Ooze`, `Max Quake`, `Max Steelspike`, `Max Wyrmwind`, `Max Flutterby`, `Max Phantasm`, and `Max Darkness`.
- G-Max hazard pressure: `G-Max Stonesurge` and `G-Max Steelsurge` can justify Dynamax when the corresponding hazard is not already on the opposing side and the normal hazard setup checks agree.
- Disruption prevention: a known or predicted Fake Out-style flinch or Roar / Whirlwind-style phazing move can justify Dynamax when the AI selected a damaging move.

The weather and terrain checks reuse the existing AI field-status evaluators so this feature does not invent a separate weather / terrain opinion system.
The runtime knowledge layer also exposes Max Move side-effect categories for speed control, weather, terrain, stat control, G-Max unique effects, residual G-Max pressure, and G-Max hazards so later scoring can reuse the same move predicates.

## Current Z-Move Payoffs

`AI_FLAG_SMART_Z_MOVE` first applies the existing Z-Move viability checks. Under smart gimmick timing, damaging Z-Moves are then conserved unless one of these is true:

- The AI is on its last available Pokemon.
- The Z-Move converts the selected move into a KO.
- The Z-Move improves the damage race while the AI is under immediate KO pressure or trapped.
- The base move already has a KO line, but the Z-Move avoids a low-accuracy miss.

Status Z-Moves keep their existing tactical checks because their value is usually the Z-status effect rather than raw damage.
The runtime knowledge layer now tags status Z effects by family: stat reset, stat boost, critical boost, redirection, HP recovery, and replacement healing. This gives later arbitration and scoring work a shared predicate layer for remaining `Z-Haze`, `Z-Tailwind`, `Z-Trick Room`, and `Z-Parting Shot` tactics instead of requiring per-move ad hoc checks.

Smart defensive Tera now rejects pure defensive Tera lines that fix one large hit while introducing a different opponent move as a new large-hit weakness. KO conversion, true KO prevention, and offensive pressure remain stronger reasons.

## Current Debug Gauntlet Read Mode

`AI_FLAG_OMNISCIENT` means the AI knows the player's moves, abilities, and held items. It does not mean the AI knows which command the player selected this turn. Ordinary prediction still asks what the AI would choose if it were controlling the player position, so it can make the wrong read.

`AI_FLAG_READ_PLAYER_MOVE` lets opponent controllers wait until every live player-side command is confirmed, rebuild AI logic data, and recompute action / move choice before returning their own command. `GetPredictedMove()` / `GetIncomingMove()` then use the confirmed player move instead of the heuristic prediction. Confirmed player switches are applied as the predicted switch-in, and selected player-side defensive gimmicks are included in damage calculation. For example, if the player selects Electric Tera Miraidon, a Koraidon read should score into the Electric Tera type instead of treating Dragon Claw as if Miraidon stayed Dragon. This began as a debug / gauntlet escalation, but this branch now auto-applies it to non-link NPC trainer AI at runtime so Elite Four, Champion, normal NPC, Frontier-style, Trainer Hill, partners, and debug trainer battles can all play from known player commands without bloating `trainers.party` or partygen-owned fragments.

Read mode also treats selected single-target `Protect` as a hard warning. Ordinary single-target damage into that protected slot receives a large penalty, while a read-mode AI Pokemon with no confirmed incoming threat no longer gets a passive doubles Protect bump just because Protect is generally useful.

Read mode now also treats selected offensive setup moves, such as `Dragon Dance`, as immediate future-pressure threats. Damaging moves that can meaningfully hit that setup user are rewarded, narrow damage into the other slot is discounted when it lets the setup threat through, and `Protect` / `King's Shield` is discounted when it merely shelters one AI Pokemon while another player-side sweeper is known to be boosting.

Read mode also distinguishes "the player side has Encore" from "Encore is selected into this AI battler right now." Unselected Encore no longer suppresses valid disruption into a selected setup user, so moves such as `Spore`, `Taunt`, slower `Haze`, and slower `Clear Smog` can be scored as direct answers to confirmed `Geomancy` / Dragon Dance-style turns. Conversely, if the player has selected `Follow Me` / `Rage Powder`, ordinary redirectable single-target moves aimed at the partner slot are penalized unless the attacker or move would bypass that redirection, including powder-immunity exceptions for Rage Powder.

Read mode also applies soft penalties for ordinary slower actions into confirmed or visible `Fake Out` pressure. This uses actual move priority, so lower-priority `Extreme Speed` is still discounted against Gen 5+ `Fake Out`, while priority blockers such as `Armor Tail` / `Dazzling` / `Queenly Majesty` and Psychic Terrain remove the Fake Out threat.

The branch now keeps an in-ROM battle action ring buffer (`gBattleActionLog`) for command-buffer audits. It records confirmed move / switch / item commands for every live battler each turn, including move slot, target, selected gimmick, switch-in party index, AI reason tag, and compact AI trace fields for board threat flags, relevant risk family, short-horizon line flags, stable line family, fallback risk line family, and loss clock. It also records resolved switch-ins separately. `AI_FLAG_READ_PLAYER_MOVE` can fall back to the current battle's logged selected move when no current confirmed command is available. Normal mGBA still cannot write host files directly, so persistent external logs are exported through mGBA Live / Lua.

For manual read-quality rechecks, use `Party -> Gauntlet Battles -> Read Single` or `Read Double`. `Read Single` starts a level-50 3v3 singles battle from mirrored 6-Pokemon weighted pools with all gimmick access. `Read Double` starts a level-50 4v4 doubles battle from mirrored 8-Pokemon weighted pools with all gimmick access. Both sides can roll comparable support, speed control, field control, priority pressure, Mega, Z-Move, Dynamax / Gigantamax, and Tera candidates, while the AI side has full read-mode flags enabled.

For intentional ally-benefit checks, use `Party -> Gauntlet Battles -> Partner Double` or `Partner Tech`. `Partner Double` keeps the fixed `Rage Powder`, ally `Pollen Puff`, and `Aqua Jet` into `Steam Engine` / `Weakness Policy` board. `Partner Tech` adds a production-style pool for broader teammate-target patterns such as `Charm` into `Contrary`, `Swagger` into Lum Berry, guaranteed-critical `Frost Breath` into `Anger Point`, low-damage priority activation, and normal support / redirection pressure.

For Champion / Elite NPC-mode manual checks, use `Party -> Gauntlet Battles -> Champs Single`, `Champs Double`, `Elite Single`, or `Elite Double`. Singles in this mode are fixed 3v3 and doubles are fixed 4v4; reserve 6v6 only for explicit irregular fixtures. The Champion routes use Mega-only access, the Elite routes use no gimmick access, and the AI side selects from larger adaptive pools that can change tactic tags after reading the player party.

Use `tools/mgba_live/start_mgba_live.sh manual-ai-log 120` from WSL / Linux, or `tools\mgba_live\start_mgba_live.bat manual-ai-log 120` from Windows, to open mGBA Live with an explicit 120 FPS target. The helper defaults to 120 FPS when the second argument is omitted. The helper also starts battle action log autosave by default; the latest non-empty snapshot is written to `/tmp/<session>-battle-action-log-autosave.json` on WSL / Linux unless `BATTLE_ACTION_LOG_OUT` is set. Set `BATTLE_ACTION_LOG_AUTOSAVE=0` only when host-side log writes are not wanted.

Use `tools/mgba_live/export_battle_action_log.sh [SESSION] [OUT_JSON]` from WSL / Linux, or `tools\mgba_live\export_battle_action_log.bat [SESSION] [OUT_JSON]` from Windows when `mgba-live-cli` is on `PATH`. Both wrappers call `tools/mgba_live/battle_action_log_export.lua` against the running mGBA Live session and write JSON using schema `pokeemerald.battle_action_log.v4`. If the WSL / Linux wrapper is called without a session, it uses the active mGBA Live session when one exists. The JSON includes header state, battler positions, action names, move / item / gimmick names, target battlers, party indexes, selected gimmick markers, resolved switch-in markers, corrected switch-in markers, AI reason tags, named AI threat flags, named AI risk kinds, and named short-horizon line facts. Because `gBattleActionLog` is runtime EWRAM, export it during the current battle before starting another battle or returning through a path that reinitializes battle state.

Double-battle loss review should use exported or autosaved logs as evidence, not memory of a single loss. Treat one loss as a candidate pattern only. Prefer repeated patterns such as over-pivoting, under-protecting a pinned slot, ignoring spread pressure, or spending a gimmick into low board value before changing runtime weights, so the AI does not overfit to one player line.

`AI_FLAG_AGGRESSIVE_GIMMICK` is paired with the gauntlet fixtures. It adds an 85% spend route for Dynamax, Tera, and damaging Z-Move pressure turns once the selected gimmick move crosses the configured minimum pressure threshold. Stronger existing reasons, such as Max Move board control, disruption denial, KO conversion, or late-commit pressure, remain deterministic.

## Current Smart Switching Payoffs

`AI_FLAG_SMART_SWITCHING` now has two VGC-style switching layers in this feature branch:

- Bad-position switching preserves a Pokemon that has no useful pressure, is threatened by either opposing slot, and cannot be covered by its partner.
- Board-control switching can pivot into reserve weather, terrain, Tailwind, Trick Room, terrain seed, direct or secondary status / confusion pressure, status care, and ability-bridge roles when those effects improve pressure or replace an unfavorable field state.
- Read-mode Choice-role switching lets a locked attacker leave the field when it is not being hit and the locked move is no longer valuable, while rejecting switch-ins that would take punishing known damage. When the confirmed player turn is already hitting the spent Pokemon, the AI can keep it in as a cushion to improve the next reserve's entry.

Singles remain more conservative; the AI still needs bad odds, bad matchup, weak current pressure, a bad field state, or an immediate status-benefit switch-in before it pivots for board control. Doubles allow more proactive pivots because the partner slot and the reserve role can create pressure together.

Status-aware switching currently covers non-volatile status pressure, secondary status / freeze / frostbite effects, `Yawn`, `Toxic Spikes`, `Leech Seed`, `Swagger`, confusion pressure, `Heal Bell` / `Aromatherapy`, `Safeguard`, `Misty Terrain`, self-status item plans, `Guts`, `Quick Feet`, `Marvel Scale`, `Magic Guard`, `Poison Heal`, `Toxic Boost`, `Flare Boost`, `Facade`, and `Psycho Shift`.

Predicted-Taunt switching is treated as a positive pivot only when the incoming move is actually predicted as `Taunt`, the active Pokemon depends on important status moves, and the active Pokemon cannot punish with damage. The branch does not fire if the active Pokemon is already Taunted, can damage-race the target, is protected by `Aroma Veil`, has Gen 6+ `Oblivious`, or has an enabled Gen 5+ `Mental Herb`.

## Current Protect Payoffs

Singles Protect now needs an explicit payoff before it gets positive scoring. Valid payoff examples include target residual damage, incoming Wish recovery, Poison Heal / Leftovers / Black Sludge recovery, reaching a Substitute threshold, scouting an unrevealed Choice-locked move, setting up a next-turn Disable / Encore line, avoiding Explosion-style moves, or letting target secondary damage finish the opposing Pokemon.

The AI avoids passive Protect into boosted singles attackers when no follow-up payoff exists. This keeps Protect from wasting a turn after the opponent has already shown a setup line.

Consecutive Protect is not banned. The second Protect receives a risk penalty because the move is less likely to succeed, but singles and doubles can still keep it as a viable option when the board payoff remains. Third and later consecutive Protect attempts remain heavily discouraged.

## VGC Reference Notes

These runtime heuristics are source-derived rather than copied from a single match:

- The 2019 Worlds recap records a Rayquaza player delaying Mega Evolution to keep `Air Lock` until it was no longer needed, which maps to preserving pre-Mega weather-nullifying abilities when there is no stronger immediate Mega payoff: https://pokemonblog.com/2019/08/20/official-recap-of-pokemon-vgc-at-the-2019-pokemon-world-championships/
- The same 2019 recap describes Mega Gengar / Perish Song as a trap archetype and a knockout breaking `Shadow Tag`, supporting immediate Mega payoff for trapping and Z payoff for breaking trap pressure: https://pokemonblog.com/2019/08/20/official-recap-of-pokemon-vgc-at-the-2019-pokemon-world-championships/
- Paul Ruiz's 2018 Worlds report emphasizes Mega Salamence Speed and KO benchmarks into Mega Gengar and other targets, supporting Mega Speed / damage pressure checks: https://victoryroad.pro/2018/09/14/soaring-higher-report-paul-ruiz-2018-world-champion/
- Public 2018 Worlds recaps describe Groundium Z helping escape a Perish Trap matchup, supporting Z-Move use before last-Pokemon turns when trap pressure changes the damage race: https://thegamehaus.com/esports/this-is-for-latin-america-2018-pokemon-world-championships-recap/2018/08/28/
- The 2017 Worlds finals recap notes a Dark-type switch-in stopping Prankster-boosted Z-Nature Power, so status Z-Moves remain under existing tactical legality / viability checks instead of being blindly conserved or blindly fired: https://www.nintendolife.com/news/2017/08/feature_everything_you_need_to_know_about_the_2017_pokemon_world_championships
- Future usage / ranking adapters should prefer Pokemon Battle DataBase, official event records, Pokemon Home / Champions-style usage when available, local PartyGen catalogs, and observed battle history. Pokemon Showdown articles are not strategy source material for this branch; raw data or team examples may be inspected only with lower confidence and explicit source tags.
- PJCS2026 game-division streams are now an accepted source for manual turn-shape review. The official PJCS page lists the 6 June 2026 and 7 June 2026 game-division YouTube Live streams, and Victory Road tracks the event as the 2026 Japan Championships for `Pokemon Champions`: https://www.pokemon.co.jp/ex/pjcs/2026/news/260522_03/ and https://victoryroad.pro/2026-japan/

## Debug Fixtures

Trainer IDs `855` through `863` are reserved smart-gimmick / smart-switching validation fixtures. Use the debug trainer-battle flow, set Trainer 1 to the ID, then start `Try Battle`. These consume the remaining Emerald trainer-flag slots without increasing `MAX_TRAINERS_COUNT_EMERALD`.

`TRAINER_SMART_SWITCH_DEBUG` (`863`) is a double-battle fixture. Its lead utility `Zigzagoon` is intentionally vulnerable to predicted `Taunt` and status pressure; reserves include `Gengar` for direct attacking punishment, `Guts` `Ursaring` for burn-benefit pivots, and `Slowbro` for `Scald` / `Psybeam` board pressure.

See `docs/tutorials/ai_flags.md` for the ID table and expected first-turn behavior.

## Documents

- [Goal](goal.md)
- [Investigation](investigation.md)
- [Implementation](implementation.md)
- [AI Runtime Knowledge Audit](runtime_knowledge_audit.md)
- [Runtime Re-evaluation Spec](runtime_reevaluation_spec.md)
- [Test Plan](test_plan.md)

## Tools

- `tools/runtime_knowledge`: Rust catalog generator for local move, ability, hold-effect, item, and gimmick-policy review JSON.
- `tools/mgba_live`: reusable mGBA Live helpers, including 120 FPS session start wrappers and battle action log JSON export wrappers for WSL / Linux and Windows.
