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
| Gated shop row | Open vendor before the row's unlock condition is met. | Row shows `GATED` / unavailable text and cannot be purchased. |
| Party full sealed purchase | Fill party and try to buy a sealed recruit while PC storage has room. | Recruit is sent to PC, money is subtracted only after successful delivery, and a PC delivery message is shown. |
| Fully full sealed purchase | Fill party and all PC boxes, then try to buy a sealed recruit. | Purchase is rejected and money is not subtracted. |
| Money failure | Try to buy any product without enough money. | Purchase is rejected and no delivery occurs. |
| PC delivery policy | Buy normal Pokemon with a full party if PC delivery is enabled. | Pokemon goes to PC and money is subtracted only after successful delivery. |
| Unlock path | Unlock / hatch or force-resolve the vendor sealed recruit. | Resulting Pokemon has expected species, nickname behavior, met data policy, and level policy, including final evolution / legendary products if configured. |
| Vendor origin bit preservation | Buy vendor sealed recruit, confirm marker while locked, unlock it, then save/load. | Marker remains on the unlocked Pokemon and does not appear on ordinary Eggs or ordinary gift Pokemon. |
| Summary marker | Open Summary for ordinary Egg, named locked sealed recruit, concealed `?????` locked sealed recruit, ordinary unlocked Pokemon, and sealed-origin Pokemon. | Named locked recruit shows its real species sprite / type identity, `LOCKED`, and bond progress without playing its cry; concealed locked recruit keeps Egg sprite / nickname treatment plus `LOCKED`; ordinary Egg keeps Egg visuals; unlocked sealed-origin Pokemon shows the dedicated origin label / badge. |
| Bond EXP progress | Carry sealed recruit through selected progress source. | Bond / seal EXP advances only while the recruit is in party and only from allowed events. Normal `MON_DATA_EXP` does not change unless explicitly designed. |
| Trainer bond reward macro | Add `pokemonvendorawardbond 20` to a post-battle script. | Locked sealed recruits gain 20 bond EXP, optional progress message appears, and unlock message appears when threshold is met. |
| Queued trainer bond reward macro | Add `pokemonvendorqueuebattlebond 20` before starting a normal trainer battle. | Locked sealed recruits gain 20 bond EXP from the trainer victory script; the progress message appears in battle after the money message, before field return. |
| Debug normal trainer reward route | Buy or carry a locked sealed recruit, then run debug `Scripts... -> Script 3`. | A normal trainer battle starts; after victory, the battle win text displays `Sealed bond EXP increased by 20.` after the money message and before field return. |
| Quiet bond reward macro | Add `pokemonvendorawardbond 20, FALSE` to a room clear script. | Bond EXP is awarded without showing progress / unlock messages. |
| Mystery sealed purchase | Buy a `POKEMON_VENDOR_REVEAL_HIDDEN` product with random species candidates. | Shop displays `?????`, charges money, delivers a locked sealed recruit, and chooses the actual species at purchase time. |
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
| Summary origin UX | Open Summary before and after vendor sealed unlock. | Locked recruit uses actual Pokemon visuals plus `LOCKED` / bond text and is not confused with ordinary hatch memo text. |
| PC icon UX | Send or move named and concealed locked sealed recruits to PC, then open Pokemon Storage. | Named locked recruits show real species icon / front sprite plus `LOCKED`; concealed `?????` recruits show Egg icon / front sprite plus `LOCKED`; Storage restrictions still treat both as Eggs while locked. |
| Vendor product icon UX | Open debug `Scripts... -> Script 1`, move through Pikachu / Dragonite / concealed `?????` products, then move to Cancel. | The detail pane shows the selected named product's real Pokemon icon, shows a generic Egg icon for concealed `?????`, and clears the icon on Cancel / close without stale sprites. |
| Vendor long-list UX | Open debug `Scripts... -> Script 1` with the 10-product sample table and scroll past the first visible page. | The vendor keeps the normal shop font / row spacing, shows up / down scroll arrows when more rows exist, scrolls to the remaining products plus Cancel, and keeps the detail icon / price text in sync. |
| Shadow-like tone | Watch progress / release text in Summary or release message. | Text communicates bond deepening / release without reusing real Shadow Pokemon state. |

## Results

| Date | Command / check | Result | Notes |
|---|---|---|---|
| 2026-05-22 | Docs-only investigation | In progress | No runtime source changes in this branch. Runtime build / mGBA checks are not applicable until implementation branch. |
| 2026-05-23 | `rtk git diff --check` | Pass | No whitespace issues before validation. |
| 2026-05-23 | `rtk make -j16 -O all` | Pass | Existing RWX linker warning only. |
| 2026-05-23 | `rtk make -j16 -O debug` | Pass | Existing RWX linker warning only; debug Script 1 / Script 2 route is available. |
| 2026-05-23 | `rtk make -j16 -O check` | Pass | New `test/pokemon_vendor.c` passed; suite still includes expected `EXPECTED_FAIL` / `KNOWN_FAILING` markers and exits 0. |
| 2026-05-23 | mGBA Live vendor route | Pass | `DISPLAY=:0` `mgba-live-cli` booted, continued an existing save, opened debug `Scripts... -> Script 1`, displayed the vendor, bought Pikachu, and confirmed money changed from `¥3000` to `¥0`. Screenshot saved to `/tmp/pokemon-vendor-purchase-success-20260523.png`; session stopped cleanly. |
| 2026-05-23 | Vendor UI repair build pass | Pass | `rtk git diff --check`, `rtk make -j16 -O all`, `rtk make -j16 -O debug`, and `rtk make -j16 -O check` passed after the message-window / Yes-No overlay repair. Existing RWX linker warning and expected test markers only. |
| 2026-05-23 | mGBA Live vendor UI repair route | Pass | Reopened debug `Scripts... -> Script 1` and checked initial list, purchase confirmation, success message, money update, and post-message list return. No leftover field dialogue box, no unloaded `#` / rough frame glyphs, and no right-side info-pane clearing after Yes / No dismissal. Screenshots: `/tmp/pokemon-vendor-ui-fix-final-open-20260523.png`, `/tmp/pokemon-vendor-ui-fix-final-confirm-20260523.png`, `/tmp/pokemon-vendor-ui-fix-final-success-20260523.png`, `/tmp/pokemon-vendor-ui-fix-final-return-20260523.png`; session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |
| 2026-05-23 | Vendor layout polish build pass | Pass | `rtk git diff --check`, `rtk make -j16 -O all`, `rtk make -j16 -O debug`, and `rtk make -j16 -O check` passed after separating all framed tile rows and combining the middle list / info panel. Existing RWX linker warning and expected test markers only. |
| 2026-05-23 | mGBA Live repeated-open layout route | Pass | Opened debug `Scripts... -> Script 1`, checked initial display, confirmation, success message, field return, and a second open. The middle panel no longer shows a competing center border, the bottom message band no longer shares tile rows with the list / info panel, and repeated open did not show stale white panel / frame corruption. Screenshots: `/tmp/pokemon-vendor-layout-polish3-open-20260523.png`, `/tmp/pokemon-vendor-layout-polish3-confirm-20260523.png`, `/tmp/pokemon-vendor-layout-polish3-success-20260523.png`, `/tmp/pokemon-vendor-layout-polish3-reopen-20260523.png`; session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |
| 2026-05-23 | Vendor two-window layout build pass | Pass | `rtk git diff --check`, `rtk make -j16 -O all`, `rtk make -j16 -O debug`, and `rtk make -j16 -O check` passed after restoring the middle area to separate list / detail windows with a one-tile gutter. Existing RWX linker warning and expected test markers only. |
| 2026-05-23 | mGBA Live two-window layout route | Pass | Opened debug `Scripts... -> Script 1`, checked initial display, confirmation, success message, field return, and a second open. The list and detail windows are visually separated, the bottom message band stays independent, and repeated open did not show stale white panel / frame corruption. Screenshots: `/tmp/pokemon-vendor-two-window-open-20260523.png`, `/tmp/pokemon-vendor-two-window-confirm-20260523.png`, `/tmp/pokemon-vendor-two-window-success-20260523.png`, `/tmp/pokemon-vendor-two-window-reopen-20260523.png`; session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |
| 2026-05-23 | Gated-row / edit-entitlement build pass | Pass | `rtk git diff --check`, `rtk make -j16 -O all`, `rtk make -j16 -O debug`, `rtk make -j16 -O check`, and `rtk mdbook build docs` passed after making `PokemonVendor_IsEditEntitled()` false for locked sealed recruits and true after unlock. Existing RWX linker warning, expected test markers, and existing mdbook warnings only. |
| 2026-05-23 | mGBA Live gated-row route | Pass | Opened debug `Scripts... -> Script 1`, confirmed the unavailable concealed row displays `?????` plus `GATED`, and selected it to confirm `This recruit is not available yet.` Screenshots: `/tmp/pokemon-vendor-gated-open-20260523.png`, `/tmp/pokemon-vendor-gated-message-20260523.png`; session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |
| 2026-05-23 | Mystery sealed / bond reward build pass | Pass | `rtk git diff --check`, `rtk make -j16 -O all`, `rtk make -j16 -O debug`, and `rtk make -j16 -O check` passed after adding random-species mystery products and the trainer / room reward macro. Existing RWX linker warning and expected test markers only. |
| 2026-05-23 | mGBA Live mystery sealed / bond reward route | Pass | Opened debug `Scripts... -> Script 1`, bought the `?????` mystery sealed product for `3000`, then ran `Scripts... -> Script 2` and confirmed the bond reward message. Screenshots: `/tmp/pokemon-vendor-mystery-open-20260523.png`, `/tmp/pokemon-vendor-mystery-confirm-20260523.png`, `/tmp/pokemon-vendor-mystery-success-20260523.png`, `/tmp/pokemon-vendor-mystery-bond-message-20260523.png`; session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |
| 2026-05-23 | Debug normal-trainer reward build pass | Pass | `rtk git diff --check`, `rtk make -j16 -O all`, `rtk make -j16 -O debug`, `rtk make -j16 -O check`, and `rtk mdbook build docs` passed after adding debug `Script 3`. Existing RWX linker warning, expected test markers, and existing mdbook warnings only. |
| 2026-05-23 | mGBA Live debug normal-trainer reward route | Pass | Bought a locked `?????` sealed recruit through debug `Scripts... -> Script 1`, ran debug `Scripts... -> Script 3`, confirmed the normal trainer battle against `YOUNGSTER CALVIN`, won it, and saw `Sealed bond EXP increased by 20.` Screenshots: `/tmp/pokemon-vendor-debug-battle-script3-start-20260523.png`, `/tmp/pokemon-vendor-debug-battle-intro-20260523.png`, `/tmp/pokemon-vendor-debug-battle-defeat-text-20260523.png`, `/tmp/pokemon-vendor-debug-battle-bond-message2-20260523.png`; session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |
| 2026-05-23 | `rtk mdbook build docs` | Pass | Existing warnings: missing root `CHANGELOG.md` include, `CREDITS.md` `</img>`, large search index. |
| 2026-05-23 | Queued battle-win reward build pass | Pass | `rtk git diff --check`, `rtk make -j16 -O all`, `rtk make -j16 -O debug`, `rtk make -j16 -O check`, and `rtk mdbook build docs` passed after moving debug `Script 3` to the queued battle-win reward path. Existing RWX linker warning, expected test markers, and existing mdbook warnings only. `data/script_cmd_table.inc` now uses `//` comments so test inline asm can include `asm/macros/event.inc` after macro edits. |
| 2026-05-23 | mGBA Live queued battle-win reward route | Pass | Bought a locked `?????` sealed recruit through debug `Scripts... -> Script 1`, ran debug `Scripts... -> Script 3`, confirmed the normal trainer battle against `YOUNGSTER CALVIN`, won it, saw `You got ¥80 for winning!`, then saw `Sealed bond EXP increased by 20.` in the battle screen before field return. Field return had no extra bond message. Screenshots: `/tmp/pokemon-vendor-battlemsg-script3-start-20260523.png`, `/tmp/pokemon-vendor-battlemsg-battle-intro-20260523.png`, `/tmp/pokemon-vendor-battlemsg-money-20260523.png`, `/tmp/pokemon-vendor-battlemsg-bond-inbattle-20260523.png`, `/tmp/pokemon-vendor-battlemsg-field-return-20260523.png`; session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |
| 2026-05-23 | Full-party sealed PC fallback / locked display build pass | Pass | `rtk git diff --check`, `rtk make -j16 -O all`, `rtk make -j16 -O debug`, `rtk make -j16 -O check`, and `rtk mdbook build docs` passed after the delivery / locked display update. Existing RWX linker warning, expected test markers, and existing mdbook warnings only. |
| 2026-05-23 | mGBA Live full-party sealed PC fallback / locked display route | Pass | Continued the existing save, used Lua only to raise money and set up party count for the focused route, then used debug `Scripts... -> Script 1`. Full-party Dragonite sealed purchase displayed `It was sent to a PC BOX.` A party-carried mystery sealed recruit displayed its actual icon with `LOCKED` in Party and its actual sprite/type with `LOCKED` plus `Locked bond: 0/180` in Summary. Screenshots: `/tmp/pokemon-vendor-sealed-pc-delivery-20260523.png`, `/tmp/pokemon-vendor-locked-party-real-icon-20260523.png`, `/tmp/pokemon-vendor-locked-summary-real-sprite-20260523.png`; session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |
| 2026-05-23 | Named vs concealed locked display build pass | Pass | `rtk git diff --check`, `rtk make -j16 -O all`, `rtk make -j16 -O debug`, `rtk make -j16 -O check`, and `rtk mdbook build docs` passed after splitting named locked recruits from concealed `?????` locked recruits. New tests verify named locked nickname / real-species display helpers and concealed locked Egg visuals. Existing RWX linker warning, expected test markers, and existing mdbook warnings only. |
| 2026-05-23 | mGBA Live concealed locked display route | Pass | Bought a `?????` sealed recruit through debug `Scripts... -> Script 1`. Party showed generic Egg icon, `EGG`, and `LOCKED`; Summary showed generic Egg art plus `LOCKED` and bond progress; a Lua-only validation copy to Box 1 slot 1 confirmed Pokemon Storage shows generic Egg art, `EGG`, and `LOCKED` in the left info panel. Screenshots: `/tmp/pokemon-vendor-final-party-concealed-egg-20260523.png`, `/tmp/pokemon-vendor-final-summary-concealed-egg-20260523.png`, `/tmp/pokemon-vendor-final-storage-box-concealed-20260523.png`; session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |
| 2026-05-23 | Storage empty-slot locked-state reset | Pass | `rtk git diff --check`, `rtk make -j16 -O all`, `rtk make -j16 -O debug`, and `rtk make -j16 -O check` passed after resetting display-only Egg / vendor-lock flags before reading the current Storage cursor target. Existing RWX linker warning and expected test markers only. |
| 2026-05-23 | mGBA Live Storage empty-slot reset route | Pass | Bought a `?????` sealed recruit through debug `Scripts... -> Script 1`, copied it to Box 1 slot 1 for focused validation, cleared Box 1 slot 2, opened Pokemon Storage, confirmed slot 1 shows Egg art / `EGG` / `LOCKED`, then moved right to the empty slot and confirmed the left info panel clears instead of retaining `LOCKED`. Screenshot: `/tmp/pokemon-vendor-storage-empty-reset-blank-slot-20260523.png`; session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |
| 2026-05-23 | Vendor selected-product icon build pass | Pass | `rtk git diff --check`, `rtk make -j16 -O all`, `rtk make -j16 -O debug`, `rtk make -j16 -O check`, and `rtk mdbook build docs` passed after adding selected-product icon rendering to the vendor detail pane. Existing RWX linker warning, expected test markers, and existing mdbook warnings only. |
| 2026-05-23 | mGBA Live vendor selected-product icon route | Pass | Opened debug `Scripts... -> Script 1` and confirmed Pikachu shows Pikachu icon, Dragonite shows Dragonite icon, concealed `?????` shows the generic Egg icon, and Cancel clears the sprite without stale art. Screenshots: `/tmp/pokemon-vendor-icon-pikachu-20260523.png`, `/tmp/pokemon-vendor-icon-dragonite-20260523.png`, `/tmp/pokemon-vendor-icon-mystery-20260523.png`, `/tmp/pokemon-vendor-icon-cancel-20260523.png`; session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |
| 2026-05-23 | Vendor long-list build pass | Pass | `rtk git diff --check`, `rtk make -j16 -O all`, `rtk make -j16 -O debug`, `rtk make -j16 -O check`, and `rtk mdbook build docs` passed after keeping the normal shop font / row spacing, adding scroll arrows, and expanding debug `Script 1` to 10 products. Existing RWX linker warning, expected test markers, and existing mdbook warnings only. |
| 2026-05-23 | mGBA Live vendor long-list route | Pass | Opened debug `Scripts... -> Script 1`, confirmed the normal-size four-row list with a visible down arrow, scrolled to later products and Cancel, and confirmed up / down arrow visibility, detail icon / price updates, and Cancel icon clearing. Screenshots: `/tmp/pokemon-vendor-arrow-list-open-20260523.png`, `/tmp/pokemon-vendor-arrow-list-scrolled-20260523.png`, `/tmp/pokemon-vendor-arrow-list-cancel-20260523.png`; session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |

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
- Trainer / room scripts can award tuned bond amounts with optional field
  messages, and trainer setup scripts can queue tuned battle-win rewards.
- Debug `Script 3` validates the normal-trainer battle-win reward route with
  `pokemonvendorqueuebattlebond 20`.
- Mystery sealed products can be purchased while species-hidden and can resolve
  to a random configured species at purchase time.
- Sealed products can fall back to PC when the party is full and storage has
  room. The Gen 7 / Gen 8-style party swap prompt remains a separate delivery
  UX feature.
- No Pokemon can evolve through normal or special evolution triggers while the
  no-evolution runtime rule is enabled.
- Locked row UI and vendor sealed-origin Summary marker are visible and tested.
- Shop row gates are treated as pre-purchase availability only; the `LOCKED`
  state belongs to sealed recruits after they are bought and placed in party.
- Edit entitlement helper is implemented; actual Status Editor / relearner
  surface wiring is deferred to the next editor-focused slice.
- `test_plan.md` records local make results, mGBA Live evidence, skipped long
  GitHub Actions waits, and accepted remaining risk.

## Open Questions

- Which non-debug map / NPC should host the first real vendor?
- Should sealed release remain immediate at threshold or show a confirmation /
  release animation?
- Which editor / relearner entry point should consume
  `PokemonVendor_IsEditEntitled()` first?
