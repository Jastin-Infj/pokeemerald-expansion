# Champions Challenge Test Plan

## Manual Tests

| Test | Setup | Expected |
|---|---|---|
| Start challenge | Normal party 6, bag has several items | Reception saves state, challenge party becomes 0, normal bag is unavailable or replaced by empty challenge bag. |
| Cancel before start | Decline at reception | Party / bag unchanged. |
| Create 5 mons | Add only 5 challenge Pokemon | Battle start is blocked. |
| Create 6 mons | Add 6 valid non-egg Pokemon | Battle start is allowed. |
| Egg rejected | Include egg in challenge party | Battle start / eligibility check rejects it. |
| Frontier ban optional | Set rule to `FRONTIER_BAN`, include banned species | Banned species rejected only in that rule. |
| Egg-only default | Set default rule, include legendary / Frontier banned non-egg | Allowed if not egg. |
| Lv.50 scaling | Use Lv.1, Lv.49, Lv.50 challenge mons | Battle display / stats follow effective Lv.50 rule, actual level remains unchanged after battle. |
| No EXP | Win a battle against EXP-yielding opponent | EXP, level, Exp Share recipients do not change. |
| Win aftercare | Win first battle | Streak increments; rule-selected HP / PP / item restoration occurs; next battle menu appears. |
| Loss aftercare | Lose battle | Challenge party is cleared; normal party / bag is restored; run status clears. |
| Power cut recovery | Save after challenge start, reload | Game restores or resumes according to challenge status, never strands player with 0 normal party. |
| Bag restore | Use / gain challenge bag items, then lose | Normal bag is exactly pre-run state unless reward policy explicitly says otherwise. |
| PC disabled | Try to open PC during challenge | PC is unavailable or only challenge-approved mode opens. |
| No leakage | Leave challenge, start normal trainer battle | Normal battle uses normal party, normal bag, normal EXP behavior. |

## Automated Test Candidates

| Test | Target |
|---|---|
| Challenge state init/clear | New challenge state helper |
| Party snapshot/restore | Dedicated party helper around `gPlayerParty` |
| Bag snapshot/restore | Dedicated bag helper around `gSaveBlock1Ptr->bag` and `gLoadedSaveData.bag` |
| Eligibility egg-only | New `Challenge_IsMonEligible` helper |
| Eligibility frontier-ban | `isFrontierBanned` rule path |
| Required party count | `Challenge_CanStartBattle` |
| Disable EXP | EXP command / battle outcome hook |
| Loss cleanup | aftercare helper clears challenge party and restores normal state |

## Regression Tests

| Existing area | Expected |
|---|---|
| Battle Frontier lobby | Existing ineligible messages and entry rules unchanged. |
| Battle Pyramid | Pyramid bag and held item storage still work. |
| Cable Club / Union Room | `SavePlayerBag` / `LoadPlayerBag` behavior unchanged. |
| Trainer battle aftercare | Normal trainer battles still whiteout / return as before. |
| Bag menu | Normal bag contents and sort order survive challenge start/end. |
| Party menu | Normal choose-half validation unchanged outside Champions mode. |

## Test Data Needed

- A banned species with `isFrontierBanned = TRUE`.
- A non-banned ordinary species.
- An egg.
- A low-level Pokemon below 50.
- A held single-use item and a Berry for restore policy checks.
- A bag with at least one item in each pocket.

## Open Questions

- Automated battle tests can assert EXP unchanged, but Lv.50 effective stats may need a battle mon setup helper test.
- If challenge bag is runtime-only, reload behavior needs a save/load integration test rather than a simple unit test.
