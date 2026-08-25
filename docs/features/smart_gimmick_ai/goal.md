# Smart Gimmick AI Goal

## Milestone Status

The current milestone is the playable smart doubles joint board-search
integration on `integration/smart-ai-board-search-20260720`. It is based on
`integration/runtime-lab-current-1.16.1` at
`4e960aecea0185912e21a28f40e00fab1b388874`. There is no feature commit or PR
for this working-tree slice yet.

This branch is snapshot-derived integration evidence, not a master-ready
runtime line. Final review-fix validation is recorded as complete locally in
`test_plan.md`; the slice remains uncommitted and has no feature PR.
The earlier `feature/smart-gimmick-ai-16-20260604` snapshot and its
`46ed9d403f` / `dd55f9a45d` handoff ancestry remain historical references for
the score-layer behavior now used as the legacy fallback; they are not the
status of the current planner milestone.

## Goal

The target is not an unbeatable AI. The target is an AI that usually loses
because the player brought a better construction, found a real matchup edge,
or created a line outside the AI's supported information and mechanics. It
should not lose because its two doubles battlers chose incompatible actions,
missed a direct board answer, ignored confirmed commands, wasted a limited
gimmick, targeted an invalid slot, or preferred a shallow local score over the
realized board.

Short version: construction and real tactical tradeoffs should matter more
than independent per-battler mistakes.

## Implemented Joint Planner

The current integration adds one side-wide planner above the preserved legacy
move and switch evaluators. On an eligible board it:

- Captures the active battlers, reserves, confirmed actions, field, side
  state, gimmick access, and relevant counters in an immutable board snapshot.
  With full knowledge, that snapshot derives real prospective Mega / Ultra
  profiles and records future gimmick eligibility for active and reserve party
  members, including Ultra Burst followed by its independent Z resource.
- Generates legal move, switch, and supported gimmick actions for each AI
  actor, then ranks joint roots instead of committing each battler separately.
- Represents forced replacement explicitly, so a fainted or replacement-bound
  slot is not treated as an ordinary optional switch.
- Simulates both sides' actions with order recomputed from the changing board,
  including priority and effective Speed rather than retaining the initial
  command order after a board change.
- Enumerates supported random outcomes with exact integer weights: all 16
  single-target damage rolls, configured critical branches, and genuine
  same-priority / same-effective-Speed order permutations, alongside supported
  Protect, secondary-effect, and thaw branches. It applies the branches first
  and merges equivalent post-turn boards instead of replacing them with one
  arbitrary roll; transient per-turn result events are excluded from the merge key.
  A narrow proof groups equal raw-damage classes before whole-turn replay when
  exactly one ordinary hit is present and every other action is proven not to
  change its inputs; unsafe mixed turns retain raw enumeration.
- Searches to depth 3 and selectively extends the leading or close roots to
  depth 5.
- Evaluates realized board pressure after the simulated actions resolve,
  including damage, survival, position, remaining resources, and supported
  field or side effects.
- Advances through resumable work slices so command selection does not monopolize
  one frame.
- Restores both AI commands from the same chosen plan and candidate rank.

The planner is a bounded tactical search, not a claim of exhaustive game-tree
solution. Unsupported mechanics are deliberately rejected rather than modeled
with invented state transitions.

## Eligibility And Side-Wide Fallback

Joint planning begins only when all of these conditions are present:

- The format is an ordinary non-link NPC trainer 2v2 double admitted by the
  runtime gate, with no trainer item inventory; recorded, Palace, Safari,
  roaming, first-battle, multi, two-opponent, and in-game-partner formats are
  excluded.
- The AI flags include `AI_FLAG_READ_PLAYER_MOVE`,
  `AI_FLAG_DOUBLE_BATTLE`, `AI_FLAG_OMNISCIENT`, and
  `AI_FLAG_SMART_MON_CHOICES`, without `AI_FLAG_ATTACKS_PARTNER`.
- Exactly two non-Commander AI-side battlers and two commandable non-Commander
  player battlers are alive and present.
- Exactly two live player-side commands are confirmed.
- Both confirmed player commands are move or switch actions, and neither player
  battler is in a forced multi-turn or recharge state.

One eligible AI side owns one shared search. It never accepts a joint decision
for one AI battler while allowing the partner to use an unrelated legacy
choice. If root generation encounters an unsupported legal action, the whole
root set is incomplete; the planner does not hide that action and optimize over
the remaining subset. Damaging spread and multi-hit moves, pre-Generation-3
critical rules, a stochastic frontier with more than 32 distinct merged post-turn
states, and a turn requiring 4,096 exact-outcome applications are explicit examples.
Both AI battlers return to the existing
legacy evaluator if generation is incomplete, if the bounded search cannot
complete any usable depth-3 root inside its budget, or if no usable completed
plan exists. That fail-closed behavior keeps command selection live and
preserves the established score-layer AI on boards outside the simulator's
current contract.

Snapshot boundaries are equally explicit: the unused third profile type is
`TYPE_MYSTERY`; an incoming forced replacement supplies its own persistent Mega /
Ultra form; and already-active or reserve-entry Stellar Tera is unsupported because
the compact board has no Stellar offensive ledger. Test-only forced abilities are
preserved across prospective transformation solely to keep the harness aligned with
the live test battler.

## Search Bounds

The default caps are part of the runtime contract:

| Limit | Default |
| --- | ---: |
| Generated atomic actions per actor | 32 |
| Actions retained per actor | 12 |
| Joint roots | 32 |
| Base depth | 3 |
| Selective depth | 5 |
| Top / close roots extended | 3 maximum |
| Normal node budget | 2,048 |
| Deep node budget | 3,072 |
| Normal frame budget | 160 |
| Deep frame budget | 220 |
| Nodes advanced per slice | 12 |
| Merged stochastic outcomes per joint turn | 32 |
| Exact-outcome applications per joint turn | 4,096 maximum |
| Transposition entries | 16 |
| Runtime arena | 16,248 bytes (below 16 KiB) |
| Static exact-enumeration scratch | 2,596 bytes of single-job EWRAM |
| Static validation scratch | 540 bytes of single-job EWRAM |
| Enumerator local stack | 240 normal / 124 debug / 244 `TESTING` bytes, plus 36 saved-register bytes |
| Joint-turn validation local stack | 56 normal / 28 debug / 56 `TESTING` bytes, plus 36 saved-register bytes |

The heap arena is allocated only while the shared decision is being built and
is released after both AI commands have been committed. Once a usable depth-3
root exists, unfinished remaining roots or a deeper continuation cannot erase
it; the trace records which completed depth actually supplied the choice. The
exact-enumeration scratch is separate from this releasable arena. The transposition
table was reduced from 64 to 16 entries to keep the heap arena under its ceiling.
This is a cache-capacity tradeoff, not permission to omit legal actions or
stochastic outcomes.

## Trace And Inspectability

The command log and board-search trace are separate, linked records:

- `gBattleActionLog` has 128 28-byte entries plus its header, for 3,592 bytes.
  It records every confirmed command and links joint AI actions by plan ID and
  candidate rank.
- The version-5 planner trace uses magic `0xA15C`, a 16-byte header, 32 32-byte
  plan records, 256 28-byte candidate records, and 96 96-byte board snapshots,
  for 17,424 bytes.
- The exporter emits `pokeemerald.battle_action_log.v5` with
  `ai_trace_header`, `ai_plans`, `ai_candidates`, and `ai_board_snapshots` when
  the signature is valid. It fails safely to the v4 action-only schema when
  the trace signature does not match.
- Each plan retains up to eight candidates total. The chosen root is forced
  into that set if it was ranked outside the retained window, and one rejected
  atomic fact can occupy or replace a non-chosen slot so legality failures
  remain visible.

The minimum useful joint trace proves that both AI commands reference the same
plan and rank, marks the selected candidate as `joint` and `chosen`, and records
the deepest completed result used rather than merely showing that a search was
started.

## Preserved Score-Layer Foundation

The joint planner does not discard the earlier Smart Gimmick AI work. Those
behaviors remain useful action-generation inputs, focused guards, tests, and
the complete fallback path:

- Full-information read mode uses confirmed player moves, switches, and
  supported defensive gimmicks for non-link trainer AI.
- Mega / Ultra Burst, Z-Move, Dynamax / Gigantamax, and Tera timing remains
  payoff-based rather than an automatic first-turn spend.
- Protect considers legality, counterplay, consecutive-use odds, recovery,
  residual pressure, and supported timer-stall payoff.
- Smart switching includes selected-command preservation, imperfect survival
  bands, board-control pivots, Taunt pivots, Choice-role completion, and
  deliberate cushion or sacrifice decisions with a next-board requirement.
- The shared risk governor suppresses low-accuracy, flinch, paralysis, freeze,
  critical-hit, OHKO, delayed-attack, partner-sacrifice, and similar comeback
  lines while stable play remains.
- Partner tactics, Soundproof / Throat Chop bridges, Commander target
  correction, exact focused damage-roll summaries, end-of-turn recovery, and
  compact threat / risk reasons retain their focused regressions.

This history matters because unsupported joint boards must remain playable and
strategically coherent rather than dropping to a generic random choice.

## Known Boundaries

1. **The tree is bounded, not exhaustive.**
   Action retention, joint-root caps, supported response families, search
   depth, nodes, frames, and arena size intentionally exclude parts of the full
   game tree. A result means best among the represented lines, not a solved
   battle.

2. **Dynamax simulation is fail-closed.**
   A board with an already active Dynamax form, or whose generated action set
   contains any newly selected Dynamax / Max candidate or dynamically resolved
   Max Move power, is unsupported by the joint simulator and uses the legacy
   evaluator for the whole AI side.

3. **Form-entry effects require an explicit profile.**
   For known active and reserve party members, the snapshot now copies the real
   party mon, resolves Mega-by-move, Mega-by-item, or Ultra Burst species, and
   recalculates the transformed stats, types, and ability. It also records
   future Mega / Ultra / Z / Tera eligibility, including Ultra-to-Z sequencing.
   Unknown profiles or unsupported entry-event abilities still reject joint root
   generation rather than guessing.

4. **Exact damage is currently single-target and single-hit.**
   Live spread targets and multi-hit strikes draw independent damage and
   critical RNG that the per-actor compact outcome key cannot encode. Those
   damaging moves, pre-Generation-3 critical rules, more than 32 distinct
   post-merge states, and the 4,096-application safety ceiling are whole-side
   fallback boundaries rather than shared-roll approximations.
   The one-hit class-compression proof is intentionally narrow: multiple damage
   actions, a special hit into a Geomancy user, Fairy damage whose aura can change
   through a switch, and other input-changing combinations use raw enumeration.

5. **Mechanics exceptions remain incomplete.**
   Commander / swallowed Tatsugiri interactions and other mechanics that
   cannot be represented by the immutable snapshot and supported transition
   set remain fallback boundaries. Existing score-layer Commander guards still
   apply on that legacy path.

6. **Legacy fallback is intentionally visible.**
   A legacy choice on an apparently eligible doubles board can mean that one
   action, gimmick, profile, or mechanic made the full shared root set unsafe to
   simulate. Trace review must distinguish that boundary from a completed joint
   plan whose chosen score was merely surprising.

## Acceptance Criteria

- Eligible read-mode doubles positions produce one shared plan for both live AI
  battlers after both player commands are confirmed.
- Legal move, switch, supported gimmick, and forced-replacement choices compete
  in the same bounded board comparison.
- Dynamic priority and Speed changes affect the simulated action order, and
  all 16 supported damage rolls, configured critical branches, and true Speed
  ties contribute their exact integer weights. Equivalent post-turn boards
  merge before the 32-outcome bound is applied.
- A searched switch can expose a known reserve's later Mega, Ultra Burst,
  Z-Move, or Tera action; Ultra Burst does not erase its later Z eligibility.
- Targeted Prankster status effects represented by the simulator respect
  grounded Psychic Terrain and Gen 7+ current-Dark-type immunity, including
  Dark Tera and Tera-away controls.
- Clean damage, disruption, switching, support, protection, board control, and
  controlled risk are compared by their realized board result rather than by
  isolated per-battler move scores.
- A completed depth-3 result remains usable when a selective depth-5 extension
  is not the deepest completed result; no partial continuation is reported as
  complete.
- Unsupported mechanics, invalid profiles, and exhausted budgets fail closed
  to two coherent legacy choices without deadlock or a mixed joint / legacy
  side.
- The planner arena is exactly 16,248 bytes, never exceeds 16 KiB, and is no
  longer allocated after both commands are committed.
- The separate exact-enumeration workspace is a 2,596-byte static EWRAM object;
  the validation board is a separate 540-byte static object. Their single-job
  ownership, object-code stack allocations, and final total EWRAM headroom are
  part of validation.
- The v5 trace can link both chosen AI commands to one plan and rank, while a
  bad or absent planner signature still exports a valid v4 action-only log.
- Every behavior change has focused regression evidence or a documented mGBA
  Live route in `test_plan.md`.

## Handoff Scope

This milestone's source, include, data, test, generated, and non-Lua tool
changes stay on a feature or integration implementation branch. Documentation
can describe the validated branch and evidence, but that evidence is not
permission to merge runtime implementation into `master`. Any publication must
first record the exact branch, base, diff scope, local checks, mGBA Live result,
manual remainder, and accepted risk in the owning implementation and test-plan
documents.
