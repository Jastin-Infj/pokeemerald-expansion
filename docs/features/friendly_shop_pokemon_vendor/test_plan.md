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
| Repeat sealed purchase | Buy repeatable sealed recruit with an empty party slot. | Locked recruit is delivered, money is subtracted, and the party has one fewer usable battle Pokemon. |
| One-time sealed purchase | Buy one-time sealed recruit and reopen vendor. | One-time state is set only after successful delivery. |
| Locked shop row | Open vendor before the row's unlock condition is met. | Row shows `LOCKED` / still-locked text and cannot be purchased. |
| Party full sealed purchase | Fill party and try to buy a sealed recruit if MVP requires party slot. | Purchase is rejected and money is not subtracted. |
| Money failure | Try to buy any product without enough money. | Purchase is rejected and no delivery occurs. |
| PC delivery policy | Buy normal Pokemon with a full party if PC delivery is enabled. | Pokemon goes to PC and money is subtracted only after successful delivery. |
| Unlock path | Unlock / hatch or force-resolve the vendor sealed recruit. | Resulting Pokemon has expected species, nickname behavior, met data policy, and level policy, including final evolution / legendary products if configured. |
| Vendor origin bit preservation | Buy vendor sealed recruit, confirm marker while locked, unlock it, then save/load. | Marker remains on the unlocked Pokemon and does not appear on ordinary Eggs or ordinary gift Pokemon. |
| Summary marker | Open Summary for ordinary Egg, locked sealed recruit, ordinary unlocked Pokemon, and sealed-origin Pokemon. | Locked recruit shows locked-state UI; unlocked sealed-origin Pokemon shows the dedicated origin label / badge. |
| Bond EXP progress | Carry sealed recruit through selected progress source. | Bond / seal EXP advances only while the recruit is in party and only from allowed events. Normal `MON_DATA_EXP` does not change unless explicitly designed. |
| Release threshold | Fill bond EXP to the configured threshold. | Recruit enters ready / release state, then unlocks through the selected automatic or confirmation flow. |
| EXP table normalization | Generate or inspect a product whose growth table has high cumulative EXP. | Raw cumulative EXP is not used; generated threshold is clamped into the feature-owned bond EXP range. |
| Missing usage fallback | Add a sealed product for a Pokemon with no usage data. | Product uses explicit threshold or neutral species-tier fallback; it is not blocked by missing usage stats. |
| Formula exception override | Add products with absolute threshold overrides for known outliers. | Override wins over tier, usage, and EXP curve formula. |
| Global evolution block | Try level-up, item, trade/link, friendship, script-trigger, overworld-special, battle-end, and any configured special evolution on ordinary Pokemon and sealed-origin Pokemon. | No Pokemon evolves; separate product species are required for alternate stages/forms. |
| Stone evolution block | Use Moon Stone on a Clefairy-style fixed-species product. | Evolution is rejected or no-ops with a clear fixed-species message. |
| Edit entitlement | Open Status Editor for normal purchased Pokemon, ordinary hatched Pokemon, locked sealed recruit, and sealed-origin Pokemon. | Only non-locked sealed-origin Pokemon can edit anywhere; normal Pokemon and ordinary hatch Pokemon follow area restriction; locked recruits reject editor entry. |

## mGBA Live Checks

| Check | Steps | Expected |
|---|---|---|
| Boot and vendor entry | Boot debug ROM, warp to test vendor, interact. | Vendor opens and returns to field cleanly. |
| Purchase success | Buy debug normal Pokemon and debug sealed recruit. | UI messages, money box, party state, and script resume are correct. |
| Failure paths | Test no money, full party, and one-time already bought. | No softlock, no incorrect money subtraction, no bad party data. |
| Battle slot risk | Enter a battle while carrying locked sealed recruit. | Locked recruit cannot be selected as a normal battler and usable party count is reduced. |
| Summary origin UX | Open Summary before and after vendor sealed unlock. | Locked and sealed-origin status is visible and not confused with ordinary hatch memo text. |
| Shadow-like tone | Watch progress / release text in Summary or release message. | Text communicates bond deepening / release without reusing real Shadow Pokemon state. |

## Results

| Date | Command / check | Result | Notes |
|---|---|---|---|
| 2026-05-22 | Docs-only investigation | In progress | No runtime source changes in this branch. Runtime build / mGBA checks are not applicable until implementation branch. |
| 2026-05-23 | `rtk git diff --check` | Pass | No whitespace issues before validation. |
| 2026-05-23 | `rtk make -j16 -O all` | Pass | Existing RWX linker warning only. |
| 2026-05-23 | `rtk make -j16 -O debug` | Pass | Existing RWX linker warning only. |
| 2026-05-23 | `rtk make -j16 -O check` | Pass | Existing RWX linker warning only; test suite exits 0. |
| 2026-05-23 | `rtk mdbook build docs` | Pass | Existing warnings: missing root `CHANGELOG.md` include, `CREDITS.md` `</img>`, large search index. |
| 2026-05-23 | mGBA Live boot | Pass | Direct MCP start failed with unset `DISPLAY`; `DISPLAY=:0` `mgba-live-cli` retry booted and captured the Game Freak screen. Session stopped cleanly. |

## Feature Complete Gate

- Existing item shops still work.
- Pokemon vendor success and failure paths are locally validated.
- One-time state cannot charge the player twice.
- Sealed risk is visible and does not require hidden battle penalties.
- Bond / sealed progress source is documented and tested.
- Normal level EXP is not mutated by bond progress unless explicitly enabled.
- Raw cumulative EXP table values are never used directly as unlock thresholds.
- Thresholds are explicit or generated from species tier with usage as a
  modifier only; missing usage has a neutral fallback.
- No Pokemon can evolve through normal or special evolution triggers while the
  no-evolution runtime rule is enabled.
- Locked row UI and vendor sealed-origin Summary marker are visible and tested.
- Edit entitlement behavior is documented and either implemented or explicitly
  deferred; always-available Status Editor access is limited to non-locked
  vendor sealed-origin Pokemon.
- `test_plan.md` records local make results, mGBA Live evidence, skipped long
  GitHub Actions waits, and accepted remaining risk.

## Open Questions

- Which map / NPC should host the first debug vendor?
- Which source should be used for the first sealed progress proof: trainer win,
  challenge clear, or script event?
