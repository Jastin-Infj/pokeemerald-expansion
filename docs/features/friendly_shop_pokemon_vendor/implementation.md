# Friendly Shop Pokemon Vendor Implementation

## Document Metadata

| Field | Value |
|---|---|
| Last updated | 2026-05-23 |
| Branch | `feature/global-no-evolution-20260523` |
| Scope | Runtime implementation: fixed-species rule, Pokemon vendor, sealed recruit lock / unlock |

## Summary

This branch now contains the first full runtime slice for the Friendly Shop
Pokemon Vendor.

- Global evolution is disabled through `P_EVOLUTIONS_ENABLED FALSE`.
- A new `pokemonvendor products` script command opens a Pokemon-product shop
  without routing through the item `pokemart` / `AddBagItem()` path.
- Product records support species, level, price, product kind, repeat /
  one-time policy, one-time flag, unlock flag, held item, ball, custom moves,
  edit policy, reveal policy, bond threshold, bond yield metadata, and up to
  four optional random-species candidates for mystery products.
- Debug `Script 1` opens a sample vendor with a repeat normal Pokemon, a
  one-time sealed recruit, and a purchasable mystery sealed recruit.
- Debug `Script 2` uses the shared `pokemonvendorawardbond 40` macro to award
  sealed bond progress to carried locked recruits and show progress / unlock
  messages.
- Debug `Script 3` starts a normal trainer battle and then runs
  `pokemonvendorawardbond 20` after the battle returns to field, giving a
  focused route for post-battle reward validation.
- Normal Pokemon purchases can deliver to party or PC.
- Sealed recruits require an empty party slot, occupy that slot while locked,
  and use Egg battle restrictions so the player has one fewer usable battler.
- Sealed recruits are excluded from vanilla Egg-cycle hatching.
- Vendor sealed-origin Pokemon keep a per-mon origin marker through unlock.
- Summary shows sealed bond progress while locked and a vendor-origin memo
  after unlock.
- The vendor UI now owns its bottom message window instead of reusing the field
  dialogue printer, clears any prior field message box before drawing, and
  restores the list / info windows after Yes / No overlays are dismissed.
- The final layout polish keeps the money box, separate list / detail panels,
  and the bottom message band on separate tile rows. The list and detail areas
  are two framed windows with a one-tile gutter, preserving visual separation
  without reintroducing frame overlap or repeated-open frame dirt.

## Changed Files

| Area | Files |
|---|---|
| Vendor constants / API | `include/constants/pokemon_vendor.h`, `include/pokemon_vendor.h` |
| Vendor runtime | `src/pokemon_vendor.c` |
| Script command / macros | `src/scrcmd.c`, `data/script_cmd_table.inc`, `asm/macros/event.inc` |
| Debug route | `data/event_scripts.s`, `data/scripts/debug.inc`, `data/specials.inc` |
| Pokemon data marker | `include/pokemon.h`, `src/pokemon.c` |
| Sealed Egg handling | `src/daycare.c` |
| Summary UX | `src/pokemon_summary_screen.c`, `include/strings.h`, `src/strings.c` |
| Tests | `test/pokemon.c`, `test/pokemon_vendor.c` |

## Runtime Contract

Normal products:

- use explicit product species and level;
- can carry configured held item / ball / move payload;
- charge money only after delivery succeeds;
- can be repeatable or one-time through an event flag.

Sealed products:

- can target final evolutions, legendary Pokemon, or any explicit species;
- can hide their species as `?????` and choose from a configured random species
  pool at purchase time;
- are purchased first, then enter the locked sealed-recruit state in party;
- are created as locked vendor-origin Pokemon using the Egg lock for battle
  exclusion;
- store bond progress in the locked recruit's feature-owned vendor helpers,
  not normal level EXP;
- skip normal Egg-cycle decrement / hatch logic;
- clear the Egg lock when bond progress reaches threshold;
- keep the vendor sealed-origin marker after unlock, allowing future editor
  surfaces to recognize the entitlement.

Shop unlock flags are pre-purchase availability gates only. They can hide a row
or make it unavailable until a script condition is met, but they are not the
same state as a locked sealed recruit after acquisition.

The current progress source is script-driven through
`PokemonVendor_AddBondExpToParty` and the `pokemonvendorawardbond` macro. That
is deliberate: trainer post-battle scripts, challenge rooms, or clear scripts
can choose different values without coupling this first slice to all battle-win
paths. The product table still carries bond-yield metadata for later balancing.
The debug trainer route uses the same script macro after a normal trainer battle
returns, so it validates the intended map-script integration point without
turning every trainer win into an automatic vendor reward.

## Script API

```asm
	lockall
	message SomeVendorGreeting
	waitmessage
	pokemonvendor SomeVendorProducts
	msgbox SomeVendorThanks, MSGBOX_DEFAULT
	releaseall
	end

	.align 2
SomeVendorProducts:
	pokemonvendorproduct 1, SPECIES_PIKACHU, 50, 3000, POKEMON_VENDOR_PRODUCT_NORMAL, POKEMON_VENDOR_PURCHASE_REPEAT
	pokemonvendorproduct 2, SPECIES_DRAGONITE, 50, 12000, POKEMON_VENDOR_PRODUCT_SEALED, POKEMON_VENDOR_PURCHASE_ONCE, FLAG_UNUSED_0x020
	pokemonvendorproduct 3, SPECIES_MEW, 50, 3000, POKEMON_VENDOR_PRODUCT_SEALED, POKEMON_VENDOR_PURCHASE_REPEAT, POKEMON_VENDOR_NO_FLAG, POKEMON_VENDOR_NO_FLAG, ITEM_NONE, 27, 180, 40, POKEMON_VENDOR_EDIT_ALWAYS, POKEMON_VENDOR_REVEAL_HIDDEN, USE_RANDOM_IVS, MOVE_NONE, MOVE_NONE, MOVE_NONE, MOVE_NONE, SPECIES_MEWTWO, SPECIES_CELEBI, SPECIES_JIRACHI, SPECIES_DEOXYS
	pokemonvendorlistend

SomeTrainer_PostBattle:
	pokemonvendorawardbond 20
	return

SomeQuietRoomReward:
	pokemonvendorawardbond 80, FALSE
	return
```

`pokemonvendorlistend` terminates the product table. Product rows may also pass
availability-gate flags, held items, ball ids, bond thresholds, edit policy,
reveal policy, IV policy, up to four explicit moves, and up to four extra
random-species candidates. `pokemonvendorawardbond amount` writes the number of
affected locked recruits to `VAR_0x8005`, the number unlocked to `VAR_RESULT`,
and can suppress messages with `FALSE`.

## Known Limitations

- The vendor UI is intentionally functional and compact. It uses text rows and
  Summary-visible status, not Pokemon icons or a custom art skin. The current
  repair validates clean standard-window rendering, two-window middle layout,
  and repeated-open stability; Pokemon icon rows remain a later UI pass because
  they need sprite lifecycle and scroll handling.
- Trainer item / TM drops are intentionally deferred to a separate reward
  feature so money, vendor products, and post-battle reward economy can be
  balanced together.
- Sealed unlock currently happens immediately when script-driven bond progress
  reaches the threshold. A later UX branch can add a dedicated release
  animation / confirmation.
- `PokemonVendor_IsEditEntitled()` exists and returns true only after a
  vendor-origin sealed recruit has been unlocked, but Status Editor / move
  editor surfaces are not wired to it in this slice.
- PC / daycare / trade policy relies on Egg restrictions while locked; unlocked
  vendor-origin Pokemon behave as ordinary Pokemon with a preserved origin bit.
- Bond-yield metadata is stored in product rows but the first progress source
  uses the script-provided amount.

## Validation

| Date | Check | Result | Notes |
|---|---|---|---|
| 2026-05-23 | `rtk git diff --check` | Pass | No whitespace issues before validation. |
| 2026-05-23 | `rtk make -j16 -O all` | Pass | Existing RWX linker warning only. |
| 2026-05-23 | `rtk make -j16 -O debug` | Pass | Existing RWX linker warning only. |
| 2026-05-23 | `rtk make -j16 -O check` | Pass | New `test/pokemon_vendor.c` passed; suite still includes expected `EXPECTED_FAIL` / `KNOWN_FAILING` markers and exits 0. |
| 2026-05-23 | mGBA Live vendor route | Pass | Booted with `DISPLAY=:0` `mgba-live-cli`, continued an existing save, opened debug `Scripts... -> Script 1`, confirmed the vendor list, bought the repeat Pikachu product, saw money drop from `¥3000` to `¥0`, and captured `/tmp/pokemon-vendor-purchase-success-20260523.png`. Session stopped cleanly. |
| 2026-05-23 | mGBA Live vendor UI repair route | Pass | Revalidated debug `Scripts... -> Script 1` after the UI repair. Initial vendor display no longer leaves the field dialogue box behind, purchase confirmation uses loaded standard frame tiles, success text is clean, the money box updates from `¥3000` to `¥0`, and the list / info panes are restored after Yes / No dismissal. Screenshots: `/tmp/pokemon-vendor-ui-fix-final-open-20260523.png`, `/tmp/pokemon-vendor-ui-fix-final-confirm-20260523.png`, `/tmp/pokemon-vendor-ui-fix-final-success-20260523.png`, `/tmp/pokemon-vendor-ui-fix-final-return-20260523.png`. Session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |
| 2026-05-23 | mGBA Live vendor layout polish route | Pass | Revalidated after the tile-row separation and combined middle-panel polish. Checked initial display, confirmation, success, field return, and re-open. The center list / detail seam no longer redraws as two competing frames, the bottom message band has a visible bottom border, and repeated open did not show stale white panel / frame corruption. Screenshots: `/tmp/pokemon-vendor-layout-polish3-open-20260523.png`, `/tmp/pokemon-vendor-layout-polish3-confirm-20260523.png`, `/tmp/pokemon-vendor-layout-polish3-success-20260523.png`, `/tmp/pokemon-vendor-layout-polish3-reopen-20260523.png`. Session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |
| 2026-05-23 | Vendor two-window build pass | Pass | `rtk git diff --check`, `rtk make -j16 -O all`, `rtk make -j16 -O debug`, and `rtk make -j16 -O check` passed after restoring the middle area to separate list / detail windows. Existing RWX linker warning and expected test markers only. |
| 2026-05-23 | mGBA Live vendor two-window route | Pass | Revalidated after restoring the middle area to two framed windows. Checked initial display, confirmation, success, field return, and a second open. The list and detail panes are separated by a one-tile gutter, the bottom message band remains independent, and repeated open did not show stale white panel / frame corruption. Screenshots: `/tmp/pokemon-vendor-two-window-open-20260523.png`, `/tmp/pokemon-vendor-two-window-confirm-20260523.png`, `/tmp/pokemon-vendor-two-window-success-20260523.png`, `/tmp/pokemon-vendor-two-window-reopen-20260523.png`. Session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |
| 2026-05-23 | Vendor gated-row / edit-entitlement build pass | Pass | `rtk git diff --check`, `rtk make -j16 -O all`, `rtk make -j16 -O debug`, `rtk make -j16 -O check`, and `rtk mdbook build docs` passed after separating pre-purchase gated rows from post-acquisition locked sealed recruits. Existing RWX linker warning, expected test markers, and existing mdbook warnings only. |
| 2026-05-23 | mGBA Live vendor gated-row route | Pass | Revalidated debug `Scripts... -> Script 1` after the terminology / entitlement repair. The concealed unavailable product row displayed `?????` and `GATED`; selecting it showed `This recruit is not available yet.` Screenshots: `/tmp/pokemon-vendor-gated-open-20260523.png`, `/tmp/pokemon-vendor-gated-message-20260523.png`. Session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |
| 2026-05-23 | Mystery sealed / bond reward build pass | Pass | `rtk git diff --check`, `rtk make -j16 -O all`, `rtk make -j16 -O debug`, and `rtk make -j16 -O check` passed after adding mystery random species candidates and the `pokemonvendorawardbond` reward macro. Existing RWX linker warning and expected test markers only. |
| 2026-05-23 | mGBA Live mystery sealed / bond reward route | Pass | Revalidated debug `Scripts... -> Script 1` and `Script 2`. The mystery product displayed `?????` with price `3000`, confirmation kept `?????`, purchase succeeded and deducted money, then `Script 2` displayed `Sealed bond EXP increased by 40.` Screenshots: `/tmp/pokemon-vendor-mystery-open-20260523.png`, `/tmp/pokemon-vendor-mystery-confirm-20260523.png`, `/tmp/pokemon-vendor-mystery-success-20260523.png`, `/tmp/pokemon-vendor-mystery-bond-message-20260523.png`. Session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |
| 2026-05-23 | Debug normal-trainer reward build pass | Pass | `rtk git diff --check`, `rtk make -j16 -O all`, `rtk make -j16 -O debug`, `rtk make -j16 -O check`, and `rtk mdbook build docs` passed after adding debug `Script 3`. Existing RWX linker warning, expected test markers, and existing mdbook warnings only. |
| 2026-05-23 | mGBA Live debug normal-trainer reward route | Pass | Bought a locked `?????` sealed recruit through debug `Script 1`, ran debug `Script 3`, confirmed a normal trainer battle against `YOUNGSTER CALVIN`, won the battle, saw the debug defeat line, then saw `Sealed bond EXP increased by 20.` Screenshots: `/tmp/pokemon-vendor-debug-battle-script3-start-20260523.png`, `/tmp/pokemon-vendor-debug-battle-intro-20260523.png`, `/tmp/pokemon-vendor-debug-battle-defeat-text-20260523.png`, `/tmp/pokemon-vendor-debug-battle-bond-message2-20260523.png`. Session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |

GitHub Actions were not re-waited; local build, test, and mGBA evidence are the
handoff evidence for this implementation update.
