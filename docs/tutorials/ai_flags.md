# What are AI Flags?
AI flags alter the behavior of AI controlled trainers.  These flags affect what moves the AI chooses to use, what Pokémon the AI sends out and when they decide to switch, overarching strategic choices the AI prefers to make, and more.

The AI flags can be found in [`include/constants/battle_ai.h`](https://github.com/rh-hideout/pokeemerald-expansion/blob/master/include/constants/battle_ai.h). Some flags have their own dedicated functions that affect how the AI scores its options when choosing what to do in battle, and those functions can be found in [`src/battle_ai_main.c`](https://github.com/rh-hideout/pokeemerald-expansion/blob/master/src/battle_ai_main.c). Other flags are used in conditional checks to gate certain behaviour behind certain flags, which you can typically find by searching the codebase for the flag name and browsing from there.

# What flags should you use?
When adding new AI flags it is recommended to use `AI_FLAG_CHECK_BAD_MOVE`, `AI_FLAG_CHECK_VIABILITY`, `AI_FLAG_TRY_TO_FAINT` to make sure the AI makes good decisions. It is especially important to use `AI_FLAG_CHECK_BAD_MOVE` in combination with any added flags otherwise the AI will use moves that can fail.

Other flags should be used with consideration to the circumstances.

# How do you use them?
Adding an AI flag to a trainer is straightforward, but the process is different depending on how trainers are being defined.

## `COMPETITIVE_PARTY_SYNTAX == TRUE`
If you are using competitive syntax parties, navigate to the trainer data in [`src/data/trainers.party`](https://github.com/rh-hideout/pokeemerald-expansion/blob/upcoming/src/data/trainers.party), find the trainer you’d like to change, and add flags like so:
AI: Check Bad Move / Try to Faint / Check Viability. The name of each flag is just the constant, but without AI_FLAG at the beginning. For example, to add `AI_FLAG_SEQUENCE_SWITCHING`, any of the following will work:
* AI_FLAG_SEQUENCE_SWITCHING
* SEQUENCE_SWITCHING
* SEQUENCE SWITCHING
* Sequence_Switching
* Sequence Switching

## `COMPETITIVE_PARTY_SYNTAX != TRUE` / Not Found
If you are not using competitive syntax parties, instead access the trainer data directly in [`src/data/trainers.h`](https://github.com/rh-hideout/pokeemerald-expansion/blob/master/src/data/trainers.h), and add flags like so, typed exactly the same as the flag names themselves:
`.aiFlags = AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_VIABILITY`

# What AI Flags does pokeemerald-expansion have?
This section lists all of expansion’s AI Flags and briefly describes the effect they have on the AI’s behaviour. In all cases, please check the corresponding function or surrounding code around their implementation for more details. Some of these functions are vanilla, some share a name with vanilla but have been modified to varying degrees, and some are completely new.

## Composite AI Flags

Expansion has a few "composite" AI flags. This means that these flags have no unique functionality themselves, and can instead be thought of as groups of other flags that are all enabled when this flag is enabled. The idea behind these flags is that if you don't care to manage the detailed behaviour of a particular trainer, you can use these as a baseline instead, and expansion will keep them updated for you.

`AI_FLAG_BASIC_TRAINER` is expansion's version of generic, normal AI behaviour. It includes `AI_FLAG_CHECK_BAD_MOVE` (don't use bad moves), `AI_FLAG_TRY_TO_FAINT` (faint the player where possible), and `AI_FLAG_CHECK_VIABILITY` (choose the most effective move to use in the current context). Trainers with this flag will still be smarter than they are in vanilla as there have been dramatic improvements made to move selection, but not incredibly so. Trainers with this flag should feel like normal trainers. In general we recommend these three flags be used in all cases, unless you specifically want a trainer who makes obvious mistakes in battle.

`AI_FLAG_SMART_TRAINER` is expansion's version of a "smart AI". It includes everything in `AI_FLAG_BASIC_TRAINER` along with `AI_FLAG_SMART_SWITCHING` (make smart decisions about when to switch), `AI_FLAG_SMART_MON_CHOICES` (make smart decisions about what mon to send in after a switch / KO), `AI_FLAG_OMNISCIENT` (awareness of what moves, items, and abilities the player's mons have to better inform decisions), and `AI_FLAG_SMART_TERA` (make smart decisions about when to terastalize). Expansion will keep this updated to represent the most objectively intelligent behaviour our flags are capable of producing.

`AI_FLAG_SMART_GIMMICK` adds smart timing for battle gimmicks. It treats trainer party gimmick data as permission to use a gimmick, not as a command to spend it immediately. This currently covers smart Tera, Dynamax conservation plus Max Move payoff checks, Mega Evolution / Ultra Burst timing for setup, ability, Speed, damage, and defensive payoff, and Z-Move usage under both the existing Z-Move viability checks and smart timing conservation.

The gimmick environment presets are convenience groups for common rulesets: `AI_FLAG_GIMMICK_ENV_TERA_ONLY`, `AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY`, `AI_FLAG_GIMMICK_ENV_DYNAMAX_TERA`, `AI_FLAG_GIMMICK_ENV_ALL`, and `AI_FLAG_GIMMICK_ENV_INVERSE_BATTLE`. These presets do not enable or disable the mechanics themselves; availability still comes from trainer data, held items, battle flags, and config.

Smart gimmick behavior also has fixed trainer-ID fixtures for debug validation. Use the debug menu's trainer battle flow, set Trainer 1 to the listed ID, then start `Try Battle`. The debug player party is still the active player party, so use a passive or non-lethal player lead when validating "conserve" or "delay" behavior. These fixtures use the remaining Emerald trainer-flag slots, IDs 855-863, so add more standard trainer fixtures only after moving trainer flags or increasing `MAX_TRAINERS_COUNT_EMERALD` intentionally.

Smart Tera conservation counts only explicit trainer-party `Tera Type` entries as future AI Tera candidates. Default generated Tera types are not treated as trainer intent. A Pokemon holding a Mega Stone or Z-Crystal cannot be the visible Tera validation slot, so all-gimmick fixtures keep Mega, Tera, Dynamax, and Z-Move candidates on separate Pokemon.

Current smart timing is still calculation-local. Tera considers explicit offensive and defensive payoff against the selected target, including doubles, but does not fully model every partner threat. Dynamax spends for last-Pokemon pressure, immediate KO pressure, when Max damage converts the chosen move into a KO, or when the selected Max Move has a strategic payoff: Speed control, weather control, terrain control, or side-wide offensive / defensive stat pressure. Mega Evolution spends for target-form ability, Speed, damage, and defensive payoff, while still allowing setup-turn delay and pre-Mega `Air Lock` / `Cloud Nine` preservation. Z-Moves can be conserved, but can also be spent early when trap pressure or immediate threat makes the damage race better.

Detailed design notes, VGC source timestamps, and validation records live in [Smart Gimmick AI](../features/smart_gimmick_ai/README.md).

| Trainer ID | Constant | Expected first-turn validation |
| --- | --- | --- |
| 855 | `TRAINER_SMART_GIMMICK_DMAX_CONSERVE` | Uses `Scratch` without Dynamax because another Dynamax user remains and the turn has no immediate payoff. |
| 856 | `TRAINER_SMART_GIMMICK_DMAX_LAST` | Uses `Scratch` with Dynamax because it is the trainer's last available Pokemon. |
| 857 | `TRAINER_SMART_GIMMICK_MEGA_SETUP` | Uses `Growth` without Mega Evolution when it is not under immediate KO pressure. |
| 858 | `TRAINER_SMART_GIMMICK_Z_MOVE` | Uses `Quick Attack` as a Normalium Z move when the Z-Move viability check accepts it. |
| 859 | `TRAINER_SMART_GIMMICK_TERA_ONLY` | Uses `Aqua Tail` with Water Tera for an offensive Tera payoff. |
| 860 | `TRAINER_SMART_GIMMICK_DMAX_TERA` | Uses Water Tera on the lead `Aqua Tail` user, then keeps a separate Dynamax `Scratch` user in reserve. |
| 861 | `TRAINER_SMART_GIMMICK_ALL_SINGLE` | Validates the all-gimmick single-battle preset with separate Mega, Tera, Dynamax, and Z-Move candidates in one party. |
| 862 | `TRAINER_SMART_GIMMICK_ALL_DOUBLE` | Validates the all-gimmick double-battle preset with an active Mega candidate and an active Water Tera candidate. |
| 863 | `TRAINER_SMART_GIMMICK_NO_GIMMICK` | Control trainer with the same AI baseline but no gimmick-bearing Pokemon. |

`AI_FLAG_PREDICTION` will enable all of the prediction flags at once, so the AI can perform as well as possible. It is best paired with the flags in `AI_FLAG_SMART_TRAINER` for optimal behaviour. This currently includes `AI_FLAG_PREDICT_SWITCH` and `AI_FLAG_PREDICT_INCOMING_MON`, but will likely be expanded in the future.

Expansion has LOADS of flags, which will be covered in the rest of this guide. If you don't want to engage with detailed trainer AI tuning though, you can just use these two composite flags, and trust that expansion will keep their contents updated to always represent the most standard and the smartest behaviour we can.

## `AI_FLAG_CHECK_BAD_MOVE`
The AI will avoid using moves that are likely to fail in the current situation. This flag helps prevent the AI from making ineffective choices, such as using moves into immunities, into invulnerable states, or when the moves are otherwise hindered by abilities, terrain, or status conditions.

## `AI_FLAG_TRY_TO_FAINT`
AI will prioritize KOing the player if able rather than using status moves. Will prioritize using a move that can OHKO the player. If the player can KO the AI’s mon and the AI’s mon is slower, prioritize priority moves (this does not prevent the AI from switching out instead).

This flag handles scoring for OHKOs but does not handle 2HKOs at all, `AI_FLAG_STRONGEST_MOVE` should be used for 2HKO scoring.

## `AI_FLAG_CHECK_VIABILITY`
This flag is divided into two components to calculate the best available move for the current context:
- **`AI_CompareDamagingMoves`**: This function compares damaging moves against each other and picks the best one.
- **`AI_CalcMoveEffectScore`**: This function checks every move effect (status or damaging move effect) and increases the score accordingly.

This is different to `AI_FLAG_CHECK_BAD_MOVE` as it calculates how poor a move is and not whether it will fail or not.

## `AI_FLAG_ATTACKS_PARTNER`
This flag is meant for double battles where both of the opponents hate each other.  They prioritize damage to their 'partner' over the player.

## `AI_FLAG_FORCE_SETUP_FIRST_TURN`
AI will prioritize using setup moves on the first turn at the expense of all else. These include stat buffs, field effects, status moves, etc. AI_FLAG_CHECK_VIABILITY will instead do this when the AI determines it makes sense.

This is just a flat increase without any consideration of whether it makes sense to use the move or not. For better move choice quality for those moves, `AI_FLAG_CHECK_VIABILITY` should be used.

## `AI_FLAG_RISKY`
AI will generally behave more recklessly. This AI enables the following behaviour:
* Always assume the highest damage roll when scoring moves
* Blindly Mirror Coat / Counter based on the player mon’s species higher attacking stat
* Moves with Recoil if they miss are not treated differently even if accuracy is lowered
* Prioritize maximizing damage from moves at the cost of accuracy
* Prioritize moves with low change strong effects (Ancient Power etc., check `AI_Risky` function for full list)
* Switch offensively mid battle rather than defensively (if using `AI_FLAG_SMART_MON_CHOICES`)
* Prioritize Explosion moves

## `AI_FLAG_TRY_TO_2HKO`
Adds score bonus to any move the AI has that either OHKOs or 2HKOs the player.

Keep in mind that this is a weaker form of `AI_FLAG_TRY_TO_FAINT` at scoring OHKOs as it does not take into account who is attacking first, it does however handle 2HKOs.

## `AI_FLAG_PREFER_BATON_PASS`
AI prefers raising its own stats if it has >= 60% HP, as well as Ingrain, Aqua Ring, and Protect. Prioritizes Baton Bass if the mon is rooted (Ingrain) or has the Aqua Ring effect, and doesn’t if it has been Leech Seeded.

## `AI_FLAG_DOUBLE_BATTLE`
This flag is automatically set in double battles, and controls much of the doubles-specific scoring. I’ll summarize some of its scoring as follows:
* Don’t use Helping Hand if partner is, don’t Perish Trap your partner, don’t change the weather if they are, don’t buff stats if partner will trigger Anger Point for us
* Collaborate with partner to Perish Trap opponent, Magnet Rise to protect partner, Dragon Cheer partner if applicable
* Prioritize using weather move if it benefits partner
* Prioritize triggering partner’s good abilities if possible (Motor Drive, Storm Drain, Beat Up -> Justified, etc.)
* Handle Skill Swap smartly, both with the partner and against the player

## `AI_FLAG_HP_AWARE`
Lets the AI make decisions based on how much remaining HP its mon(s) and the player’s mon(s) have.

With respect to the AI’s mons, in doubles:
* Allows the AI to attack its partner with a move it can absorb if its low on HP (ie. Electric move on partner with Volt Absorb)
* Prioritizes healing its partner if its HP is <= 50% if able

In both singles and doubles:
* Prioritizes not using moves that require the user fainting (Destiny Bond, Explosion etc.) and healing moves while on >= 70% HP.
* Prioritize not using moves that require the user fainting or losing significant HP (Belly Drum etc) while between 30% and 70% HP
* Prioritize not using setup moves (Light Screen etc.) and Bide while on <= 30% HP

With respect to the player’s mons:
* Prioritize not using many status moves (stat buffs, Poison, Pain Split) if the player has between 30% and 70% HP
* Prioritize not using any status moves if the player is has <= 30% HP

## `AI_FLAG_POWERFUL_STATUS`
AI prioritizes setting up field effects (Trick Room, Rain Dance, etc.) and side statuses (Tailwind, Spikes, etc.), even if it could faint the target.

## `AI_FLAG_NEGATE_UNAWARE`
AI does not understand ability suppression (Mold Breaker etc., weather suppression (Air Lock etc.), redirection abilities (Lightningrod etc.) being temporarily removed due to move effects (Sky Drop etc.), or item suppression (Magic Room etc.) and will ignore them. This is a handicap flag.

## `AI_FLAG_WILL_SUICIDE`
AI prioritizes self destruction moves (Explosion, Memento).

## `AI_FLAG_PREFER_STATUS_MOVES`
AI gets a score bonus for status moves. This should be combined with `AI_FLAG_CHECK_BAD_MOVE` to prevent using only status moves.

## `AI_FLAG_STALL`
AI prefers simple classically "stalling" behaviour. It will prioritize:
* Mean Look, Fairy Lock, and Wrap for trapping
* Increasing its defense and special defense
* Moves that inflict Poison if it also has a Protect move
* Copying defense and special defense buffs

## `AI_FLAG_SMART_SWITCHING`
Affects when the AI chooses to switch. AI will make smarter decisions about when to switch out mid-battle. Automatically enables `AI_FLAG_SMART_MON_CHOICES`, which is required as the vanilla mon selection AI is not smart enough to handle several switch-triggering situations appropriately, leading to bizarre behaviour. Many of these checks have intentional failure rates, so the AI won’t switch out 100% of the time in these cases to keep the player from being able to predict perfectly. Some of these also only apply to singles, and many of them are being simplified for the sake of brevity. This flag lets the AI trigger switches when:
* It can’t hit Wonder Guard and has another mon in the party that can (switch that mon in)
* It’s going to die to Perish Song, can’t KO the player and is affected by Yawn, is being severely affected by a status condition that switching helps (Curse, Toxic, Leech Seed)
* It has a mon that can trap the player’s mon and win the 1v1 (switch that mon in)
* It has a mon in the party that can absorb the player’s next expected attack (switch that mon in)
* It will not switch if the current mon will die to hazards on re-entry and it has no means of clearing them in its party
* All its moves are bad
* It can take advantage of Natural Cure or Regenerator
* Its Encore’d into something bad
* Its primary attacking stats are sufficiently lowered
* In double battles, one or both active Pokemon are in a bad position: the active Pokemon has no meaningful pressure into either opposing slot, is threatened by either opposing slot, has enough HP to preserve, and its partner cannot cover the threat. This check is deterministic once the strict position conditions are met, and it avoids overriding the existing Intimidate-blocker contract.
* Its "odds are bad", which is a generic "try to make smart, player-like decisions generally speaking" check. Switches can be triggered if the player has a good switchin candidate (`AI_FLAG_SMART_MON_CHOICES`), and:
* The current mon has a bad type matchup and doesn’t have a super effective move and has at least ½ HP, or ¼ HP and Regenerator, or
* The current mon loses the 1v1 quickly and has at least ½ HP, or ¼ and Regenerator

## `AI_FLAG_ACE_POKEMON`
Marks the last Pokemon in the party as the Ace Pokemon. It will not be used unless it is the last one remaining, or is forced to be switched in (Roar, U-Turn with 1 mon remaining, etc.). If you are challenged by two different trainers at the same time, only the ones with this flag will have Ace Pokémon. For example vs one trainer with `AI_FLAG_ACE_POKEMON`and the other without, there will be a total of 1 Ace Pokémon.

## `AI_FLAG_DOUBLE_ACE_POKEMON`
Marks the last two Pokémon in the party as Ace Pokémon, with the same behaviour as `AI_FLAG_ACE_POKEMON`. Intented for double battles where you battle one trainer id that represents two trainers, ie Twins, Couples. If you apply this flag to trainers outside of double battles or in cases where two trainers can challenge you at the same time, it has the same behaviour. For example vs two trainers with `AI_FLAG_DOUBLE_ACE_POKEMON` there will be a total of 4 Ace Pokémon.

## `AI_FLAG_OMNISCIENT`
AI has full knowledge of player moves, abilities, and hold items, and can use this knowledge when making decisions.

## `AI_FLAG_KNOW_OPPONENT_PARTY`
AI has full knowledge of the species in the player's party, as well as their fainted status; no other omniscient knowledge is included. Functions similarly to a team preview.

## `AI_FLAG_ASSUME_STAB`
A significantly more restricted version of `AI_FLAG_OMNISCIENT`, the AI only knows the player's STAB moves, as their existence would be reasonable to assume in almost any case.

## `AI_FLAG_ASSUME_STATUS_MOVES`
A more restricted version of `AI_FLAG_OMNISCIENT`. The AI has a _chance_ to know what status moves the player has, plus additionally Fake Out and fixed percentage moves like Super Fang. The intention is so that if the AI has a counterplay implemented, it will seem to have guessed if the player's pokemon has a move, without giving the AI perfect information. For example, with Omniscient set, the AI will not usually put a pokemon to sleep if it has Sleep Talk; with neither Assume Powerful Status nor Omniscient set, the AI will always assume the pokemon does not have Sleep Talk.

By default, there are three groups of higher likelihood status moves defined in `include/config/ai.h` under `ASSUME_STATUS_HIGH_ODDS`, `ASSUME_STATUS_MEDIUM_ODDS`, and `ASSUME_STATUS_LOW_ODDS`.  Moves are sorted in `src/battle_ai_util.c` within `ShouldRecordStatusMove()`.

Any move that is not special cased is then potentially caught by `ASSUME_ALL_STATUS_ODDS`.

## `AI_FLAG_SMART_MON_CHOICES`
Affects what the AI chooses to send out after a switch. AI will make smarter decisions when choosing which mon to send out mid-battle and after a KO, which are handled separately. Automatically included when `AI_FLAG_SMART_SWITCHING` is enabled.

With this flag enabled, the AI will prioritize choosing mons after a KO prioritizing the following criteria:
* Trapper (can trap the player’s mon and win the 1v1)
* Revenge killer (outspeeds an OHKOs / is outsped and OHKOs, is not OHKOd/ outspeeds and 2HKOs, is not OHKOd / is outsped and 2HKOs, is not 2HKOd)
* Has good type matchup and a super effective move
* Has good type matchup and does not have a super effective move
* Has Baton Pass
* If no mons meet any of the above criteria, choose the one that does the most damage

And will choose mons after a mid-battle switch prioritizing the following criteria:
* Trapper (can trap the player’s mon and win the 1v1)
* Has good type matchup and a super effective move
* Has good type matchup and does not have a super effective move
* Is not 3HKO’d by the player
* Has Baton Pass

## `AI_FLAG_CONSERVATIVE`
AI always assumes it will roll the lowest possible result when comparing damage in scoring.

## `AI_FLAG_SEQUENCE_SWITCHING`
AI will always switch out after a KO in exactly party order as defined in the trainer data (ie. slot 1, then 2, then 3, etc.). The AI will never switch out mid-battle unless forced to (Roar etc.). If the AI uses a move that requires a switch where it makes a decision about what to send in (U-Turn etc.), it will always switch out into the lowest available party index.

## `AI_FLAG_WEIGH_ABILITY_PREDICTION`
AI will predict the player's ability based to its aiRating. Without this flag the AI randomly assumes an ability with an even distribution between all possible abilities until one is confirmed. With this flag, it instead guesses proportionally to each ability's aiRating, making it far more likely to guess an ability like Water Absorb than Damp if both are options.

## `AI_FLAG_PREFER_HIGHEST_DAMAGE_MOVE`
AI will add score to its highest damaging move, regardless of accuracy or secondary effects. Replaces deprecated `AI_FLAG_PREFER_STRONGEST_MOVE`.

## `AI_FLAG_PREDICT_SWITCH`
AI will determine whether it would switch out in the player's situation or not, and predict the player to switch accordingly. In any case where the AI would consider switching, it will assume the player will switch. This is modulated by a 50% failure rate, so the behaviour is non-deterministic and can change from turn to turn to emulate the inconsistency in human predictions. This behaviour is improved significantly by using `AI_FLAG_SMART_SWITCHING` and `AI_FLAG_SMART_MON_CHOICES` as they improve the AI's ability to determine good situations to switch, and also by `AI_FLAG_OMNISCIENT` so the AI can use all its knowledge of the player's team to make the decision.

## `AI_FLAG_PREDICT_INCOMING_MON`
This flag requires `AI_FLAG_PREDICT_SWITCH` to function. If the AI predicts that the player will switch, this flag allows the AI to run its move scoring calculation against the Pokémon it expects the player to switch into, instead of the Pokémon that it expects to switch out.

## `AI_FLAG_SMART_TERA`
AI will make smarter decisions about when to terastalize (over the default behaviour to always tera when available). This considers factors such as whether tera allows the AI to KO the opponent, whether it can save itself from a KO or a big hit, and how many remaining pokemon could terastalize. This behavior is not currently supported in double battles.

## `AI_FLAG_SMART_GIMMICK_TIMING`
AI treats available gimmicks as strategic resources. Without this flag, trainer-owned gimmicks keep the older eager behavior where an available gimmick is generally selected immediately unless a gimmick-specific check cancels it. With this flag, each gimmick must also have its own smart flag enabled before the AI will spend it.

## `AI_FLAG_SMART_DYNAMAX`
AI may conserve Dynamax instead of using it immediately. It spends Dynamax when it is on the last available Pokemon, when the current target can otherwise KO it, when Dynamax turns the chosen move into a KO that the regular move would miss, or when the selected Max Move has a strong board payoff such as Max Airstream / Max Strike Speed control, weather control, terrain control, or side-wide stat boosts / drops.

## `AI_FLAG_SMART_MEGA`
AI may delay Mega Evolution or Ultra Burst on setup turns, but can still spend it immediately for target-form ability payoff, Speed flips, meaningful defensive improvement under KO pressure, or meaningful attacking-stat improvement for the selected damaging move. It can also preserve pre-Mega `Air Lock` / `Cloud Nine` during active weather if the target form has no stronger immediate payoff.

## `AI_FLAG_SMART_Z_MOVE`
AI keeps Z-Move spending under smart gimmick timing. It still uses the existing Z-Move viability checks, including avoiding Z-Moves that are unnecessary for a KO or invalid for the selected move. Under `AI_FLAG_SMART_GIMMICK_TIMING`, damaging Z-Moves are conserved unless the AI is on its last available Pokemon, the Z-Move converts the selected move into a KO, the Z-Move improves a damage race under immediate KO or trap pressure, or the Z-Move secures a low-accuracy KO line. Status Z-Moves keep their existing tactical checks.

## `AI_FLAG_ENV_INVERSE_BATTLE`
Marks an AI preset as intended for inverse-battle environments. The actual inverse type matchup still comes from `B_FLAG_INVERSE_BATTLE`; this flag is mainly useful when composing trainer AI flags for an inverse ruleset.

## `AI_FLAG_PREDICT_MOVE`
AI will predict what move the player is going to use based on what move it would use in the same situation. Generally works best if also using `AI_FLAG_OMNISCIENT`.

## `AI_FLAG_PP_STALL_PREVENTION`
This flag aims to prevent the player from PP stalling the AI by switching between immunities. The AI mon's move scores will slowly decay for absorbed moves over time, eventually making its moves unpredictable. More detailed control for this behaviour can be customized in the `ai.h` config file.

## `AI_FLAG_RANDOMIZE_SWITCHIN`
AI will randomly choose between eligible switchin candidates rather than always picking the last one in the party. For example, if the AI has two mons that can revenge kill the player's mon after a KO, by default the AI will only track the most recent eligible candidate, and will always send in the last one in party order as a result. With this flag, it will instead track all of the eligible mons, and randomly choose between them when deciding which to send out.

## `AI_FLAG_RANDOMIZE_PARTY_INDICES`
AI will randomize the order of the mons in their party before battle starts. This means that lead choice is randommized, but so is the last mon for things like Illusion or the Ace flag, so be mindful when using it.
