# Friendly Shop Pokemon Vendor Test Plan

## Build / Lint

| Test | Command | Expected |
|---|---|---|
| Diff check | `rtk git diff --check` | No whitespace or patch errors. |
| Normal build | `rtk make -j16 -O all` | Passes when runtime source exists. |
| Debug build | `rtk make -j16 -O debug` | Passes if debug menu or debug route is added. |
| Focused checks | `rtk make -j16 -O check` or focused `TESTS=...` | Passes for shop / Pokemon / save / script tests touched by implementation. |
| Docs | `rtk mdbook build docs` | Passes with only known existing warnings. |

## Focused Runtime Tests

| Test | Steps | Expected |
|---|---|---|
| Repeat normal Pokemon purchase | Buy the same repeatable Pokemon twice. | Both purchases deliver Pokemon and subtract money only after success. |
| One-time normal Pokemon purchase | Buy one-time product, reopen vendor. | Product is hidden, sold out, or disabled according to selected policy. |
| Repeat Egg purchase | Buy repeatable Egg with an empty party slot. | Egg is delivered, money is subtracted, and the party has one fewer usable battle Pokemon. |
| One-time Egg purchase | Buy one-time Egg and reopen vendor. | One-time state is set only after successful delivery. |
| Party full Egg purchase | Fill party and try to buy an Egg if MVP requires party slot. | Purchase is rejected and money is not subtracted. |
| Money failure | Try to buy any product without enough money. | Purchase is rejected and no delivery occurs. |
| PC delivery policy | Buy normal Pokemon with a full party if PC delivery is enabled. | Pokemon goes to PC and money is subtracted only after successful delivery. |
| Hatch path | Hatch or force-resolve the vendor Egg. | Resulting Pokemon has expected species, nickname behavior, met data policy, and level policy. |
| Egg progress | Carry Egg through selected progress source. | Progress advances only while the Egg is in party and only from allowed events. |
| Edit entitlement | Open move / item editor for normal purchased Pokemon and Egg-origin Pokemon. | Normal Pokemon follows area restriction; Egg-origin Pokemon follows always-editable policy if implemented. |

## mGBA Live Checks

| Check | Steps | Expected |
|---|---|---|
| Boot and vendor entry | Boot debug ROM, warp to test vendor, interact. | Vendor opens and returns to field cleanly. |
| Purchase success | Buy debug normal Pokemon and debug Egg. | UI messages, money box, party state, and script resume are correct. |
| Failure paths | Test no money, full party, and one-time already bought. | No softlock, no incorrect money subtraction, no bad party data. |
| Battle slot risk | Enter a battle while carrying Egg. | Egg cannot be selected as a normal battler and usable party count is reduced. |

## Results

| Date | Command / check | Result | Notes |
|---|---|---|---|
| 2026-05-22 | Docs-only investigation | In progress | No runtime source changes in this branch. Runtime build / mGBA checks are not applicable until implementation branch. |

## Feature Complete Gate

- Existing item shops still work.
- Pokemon vendor success and failure paths are locally validated.
- One-time state cannot charge the player twice.
- Egg risk is visible and does not require hidden battle penalties.
- Egg progress source is documented and tested.
- Edit entitlement behavior is documented and either implemented or explicitly
  deferred.
- `test_plan.md` records local make results, mGBA Live evidence, skipped long
  GitHub Actions waits, and accepted remaining risk.

## Open Questions

- Which map / NPC should host the first debug vendor?
- Which source should be used for the first Egg progress proof: trainer win,
  challenge clear, or script event?
