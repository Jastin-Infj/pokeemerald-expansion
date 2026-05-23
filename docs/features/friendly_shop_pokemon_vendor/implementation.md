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
  edit policy, reveal policy, bond threshold, and bond yield metadata.
- Debug `Script 1` opens a sample vendor with a repeat normal Pokemon, a
  one-time sealed recruit, and a locked concealed row.
- Debug `Script 2` awards sealed bond progress to carried locked recruits.
- Normal Pokemon purchases can deliver to party or PC.
- Sealed recruits require an empty party slot, occupy that slot while locked,
  and use Egg battle restrictions so the player has one fewer usable battler.
- Sealed recruits are excluded from vanilla Egg-cycle hatching.
- Vendor sealed-origin Pokemon keep a per-mon origin marker through unlock.
- Summary shows sealed bond progress while locked and a vendor-origin memo
  after unlock.

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
- are created as locked vendor-origin Pokemon using the Egg lock for battle
  exclusion;
- store bond progress in the locked recruit's feature-owned vendor helpers,
  not normal level EXP;
- skip normal Egg-cycle decrement / hatch logic;
- clear the Egg lock when bond progress reaches threshold;
- keep the vendor sealed-origin marker after unlock, allowing future editor
  surfaces to recognize the entitlement.

The current progress source is script-driven through
`PokemonVendor_AddBondExpToParty`. That is deliberate: challenge rooms can call
the special after a clear without coupling this first slice to all battle-win
paths. The product table still carries bond-yield metadata for later balancing.

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
	pokemonvendorlistend
```

`pokemonvendorlistend` terminates the product table. Product rows may also pass
unlock flags, held items, ball ids, bond thresholds, edit policy, reveal policy,
IV policy, and up to four explicit moves.

## Known Limitations

- The vendor UI is intentionally functional and compact. It uses text rows and
  Summary-visible status, not Pokemon icons or a custom art skin.
- Sealed unlock currently happens immediately when script-driven bond progress
  reaches the threshold. A later UX branch can add a dedicated release
  animation / confirmation.
- `PokemonVendor_IsEditEntitled()` exists, but Status Editor / move editor
  surfaces are not wired to it in this slice.
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

GitHub Actions were not re-waited; local build, test, and mGBA evidence are the
handoff evidence for this implementation update.
