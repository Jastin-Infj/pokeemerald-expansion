# Friendly Shop Pokemon Vendor Implementation

## Document Metadata

| Field | Value |
|---|---|
| Last updated | 2026-05-30 |
| Branch | `integration/runtime-dev-20260529`; source shelf `feature/global-no-evolution-20260523` / PR #57 |
| Scope | Runtime implementation: fixed-species rule, Pokemon vendor, sealed recruit lock / unlock |

## Summary

This branch now contains the first full runtime slice for the Friendly Shop
Pokemon Vendor.

2026-05-29 integration note: the #57 source slice has been re-applied onto
`integration/runtime-dev-20260529` after #47 Battle Item Restore, #48 Held Item
Catalog, #54 Party / Status UI, and #51 Scout Selection. The integration keeps
the Scout debug routes and assigns vendor validation routes as:
`Scripts... -> Script 1` for the vendor, `Script 2` for Scout Selection pick-6,
and `Script 3` for the vendor queued trainer-battle bond reward.

- Global evolution is disabled through `P_EVOLUTIONS_ENABLED FALSE`.
- A new `pokemonvendor products` script command opens a Pokemon-product shop
  without routing through the item `pokemart` / `AddBagItem()` path.
- Product records support species, level, price, product kind, repeat /
  one-time policy, one-time flag, unlock flag, held item, ball, custom moves,
  edit policy, reveal policy, bond threshold, bond yield metadata, and up to
  four optional random-species candidates for mystery products.
- Debug `Script 1` opens a 10-product sample vendor with normal products,
  named sealed recruits, and a purchasable mystery sealed recruit.
- On the original #57 shelf, debug `Script 2` used the shared
  `pokemonvendorawardbond 40` macro to award sealed bond progress to carried
  locked recruits. In the integration branch, `Script 2` remains the Scout
  Selection six-pick route, so standalone vendor bond-award checks should use a
  feature-local test script or the queued `Script 3` trainer route.
- Debug `Script 3` queues 20 sealed bond EXP before starting a normal trainer
  battle. The queued reward is paid and displayed by the trainer victory battle
  script after the money message, before returning to the field.
- Normal Pokemon purchases can deliver to party or PC.
- Integration review follow-up: vendor list row allocation is now checked before
  the list is populated. If a long product table or fragmented heap prevents
  allocation, the vendor unwinds to script instead of dereferencing partial
  row buffers. Purchase-result list rebuild also closes the vendor safely if
  a rebuild allocation fails.
- Sealed recruits deliver to party when there is room and fall back to PC when
  the party is full and storage has room.
- When carried in party, locked sealed recruits use Egg battle restrictions so
  the player has one fewer usable battler.
- Sealed recruits are excluded from vanilla Egg-cycle hatching.
- Vendor sealed-origin Pokemon keep a per-mon origin marker through unlock.
- Named sealed recruits keep their real species nickname / icon / sprite while
  locked, show a `LOCKED` label, and show sealed bond progress in Summary.
  They still keep Egg battle restrictions and do not play the species cry while
  locked.
- Concealed `?????` sealed recruits keep generic Egg nickname / icon / sprite
  treatment while locked so the mystery product does not reveal its actual
  species, including through species-specific Egg art, before unlock.
- Pokemon Storage now separates locked display identity from the actual Egg
  restriction bit. Named locked recruits show their real species art, and the
  left info panel prints `LOCKED`; concealed locked recruits show Egg visuals
  with `LOCKED`.
- The vendor UI now owns its bottom message window instead of reusing the field
  dialogue printer, clears any prior field message box before drawing, and
  restores the list / info windows after Yes / No overlays are dismissed.
- The final layout polish keeps the money box, separate list / detail panels,
  and the bottom message band on separate tile rows. The list and detail areas
  are two framed windows with a one-tile gutter, preserving visual separation
  without reintroducing frame overlap or repeated-open frame dirt.
- The integration review follow-up clamps the vendor list scroll offset and
  selected row after one-time purchases rebuild the visible product list. This
  prevents scrolled long lists from returning to an out-of-range list row after
  a sold-out product disappears.

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
- are purchased first, then enter the locked sealed-recruit state in party or
  PC depending on available space;
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

The current progress source is script-driven. Field / room clear scripts use
`PokemonVendor_AddBondExpToParty` through the `pokemonvendorawardbond` macro.
Trainer setup scripts can use `pokemonvendorqueuebattlebond amount`, which
stores a one-battle reward and lets the normal trainer victory script call
`BS_PokemonVendorAwardQueuedBattleBondExp` after the money message. That is
deliberate: `.inc` scripts can tune values per NPC or challenge without
turning every trainer win into an automatic vendor reward. The product table
still carries bond-yield metadata for later balancing.

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

SomeTrainer_PreBattle:
	pokemonvendorqueuebattlebond 20
	special PokemonVendor_StartDebugBondTrainerBattle
	waitstate
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
and can suppress messages with `FALSE`. `pokemonvendorqueuebattlebond amount`
queues the amount for the next trainer victory; the battle script prints the
progress message only if at least one locked sealed recruit gained progress and
prints the unlock message only if one or more recruits unlocked.

## Known Limitations

- The vendor UI is intentionally functional and compact. It uses text rows,
  Summary-visible status, and a selected-product Pokemon icon in the detail
  pane. Named products show their real species icon; concealed `?????` products
  show the generic Egg icon. The list keeps the normal shop font / row spacing
  and shows up / down scroll arrows when the product table is longer than the
  visible rows.
  Per-row Pokemon icon rendering remains a later UI pass because the standard
  list row height is too small for 32x32 mon icons without a custom list layout.
- The delivery path now supports PC fallback for sealed recruits, but the
  Gen 7 / Gen 8-style prompt to add the new Pokemon to party and choose a party
  member to send to PC is not implemented in this slice. That belongs in a
  broader gift / capture / vendor delivery feature.
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
  Locked sealed recruits sent to PC do not gain the current party-carried bond
  progress until moved into the party.
- Bond-yield metadata is stored in product rows but the first progress sources
  use script-provided amounts. Queued battle rewards are intentionally opt-in
  per script, not global for every trainer battle.

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
| 2026-05-23 | Queued battle-win reward build pass | Pass | `rtk git diff --check`, `rtk make -j16 -O all`, `rtk make -j16 -O debug`, `rtk make -j16 -O check`, and `rtk mdbook build docs` passed after moving debug `Script 3` to `pokemonvendorqueuebattlebond 20` and printing the reward from the trainer victory battle script. Existing RWX linker warning, expected test markers, and existing mdbook warnings only. The `data/script_cmd_table.inc` comments were converted from `@` to `//` so C-test inline asm can include `asm/macros/event.inc` after the macro edit. |
| 2026-05-23 | mGBA Live queued battle-win reward route | Pass | Bought a locked `?????` sealed recruit through debug `Script 1`, ran debug `Script 3`, confirmed the normal trainer battle against `YOUNGSTER CALVIN`, won it, saw `You got ¥80 for winning!`, then saw `Sealed bond EXP increased by 20.` while still in battle before field return. No extra bond message appeared after returning to the field. Screenshots: `/tmp/pokemon-vendor-battlemsg-script3-start-20260523.png`, `/tmp/pokemon-vendor-battlemsg-battle-intro-20260523.png`, `/tmp/pokemon-vendor-battlemsg-money-20260523.png`, `/tmp/pokemon-vendor-battlemsg-bond-inbattle-20260523.png`, `/tmp/pokemon-vendor-battlemsg-field-return-20260523.png`. Session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |
| 2026-05-23 | Full-party sealed PC fallback / locked display build pass | Pass | `rtk git diff --check`, `rtk make -j16 -O all`, `rtk make -j16 -O debug`, `rtk make -j16 -O check`, and `rtk mdbook build docs` passed after adding sealed PC fallback and actual-species locked display helpers. Existing RWX linker warning, expected test markers, and existing mdbook warnings only. |
| 2026-05-23 | mGBA Live full-party sealed PC fallback / locked display route | Pass | Continued the existing save, used Lua only to raise money and set up space/full-party states, then used debug `Scripts... -> Script 1`. With party count 6, buying one-time Dragonite sealed showed `It was sent to a PC BOX.` Screenshot: `/tmp/pokemon-vendor-sealed-pc-delivery-20260523.png`. Then a repeat mystery sealed recruit was bought into party; Party menu showed the actual Pokemon icon plus `LOCKED`, and Summary showed the actual sprite/type plus `LOCKED` and `Locked bond: 0/180`. Screenshots: `/tmp/pokemon-vendor-locked-party-real-icon-20260523.png`, `/tmp/pokemon-vendor-locked-summary-real-sprite-20260523.png`. Session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |
| 2026-05-23 | Named vs concealed locked display build pass | Pass | `rtk git diff --check`, `rtk make -j16 -O all`, `rtk make -j16 -O debug`, `rtk make -j16 -O check`, and `rtk mdbook build docs` passed after adding the concealed marker, named locked nickname preservation, and Storage `LOCKED` display. Existing RWX linker warning, expected test markers, and existing mdbook warnings only. |
| 2026-05-23 | mGBA Live concealed locked display route | Pass | Bought a `?????` sealed recruit through debug `Scripts... -> Script 1`. Party showed generic Egg icon, `EGG`, and `LOCKED`; Summary showed generic Egg art plus `LOCKED` and bond progress; a Lua-only validation copy to Box 1 slot 1 confirmed Pokemon Storage shows generic Egg art, `EGG`, and `LOCKED` in the left info panel. Screenshots: `/tmp/pokemon-vendor-final-party-concealed-egg-20260523.png`, `/tmp/pokemon-vendor-final-summary-concealed-egg-20260523.png`, `/tmp/pokemon-vendor-final-storage-box-concealed-20260523.png`. Session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |
| 2026-05-23 | Storage empty-slot locked-state reset | Pass | `SetDisplayMonData()` now clears display-only Egg / vendor-lock flags before reading the current cursor target, so empty Box slots cannot inherit `LOCKED` from the previously inspected sealed recruit. |
| 2026-05-23 | mGBA Live Storage empty-slot reset route | Pass | Bought a `?????` sealed recruit through debug `Scripts... -> Script 1`, copied it to Box 1 slot 1 for focused validation, cleared Box 1 slot 2, opened Pokemon Storage, confirmed slot 1 shows Egg art / `EGG` / `LOCKED`, then moved right to the empty slot and confirmed the left info panel clears instead of retaining `LOCKED`. Screenshot: `/tmp/pokemon-vendor-storage-empty-reset-blank-slot-20260523.png`. Session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |
| 2026-05-23 | Vendor selected-product icon build pass | Pass | `rtk git diff --check`, `rtk make -j16 -O all`, `rtk make -j16 -O debug`, `rtk make -j16 -O check`, and `rtk mdbook build docs` passed after adding selected-product icon rendering to the vendor detail pane. Existing RWX linker warning, expected test markers, and existing mdbook warnings only. |
| 2026-05-23 | mGBA Live vendor selected-product icon route | Pass | Booted the debug ROM, continued the existing save, opened debug `Scripts... -> Script 1`, and confirmed the detail pane shows Pikachu's icon for the Pikachu product, Dragonite's icon for the Dragonite product, a generic Egg icon for the concealed `?????` product, and no stale icon on Cancel. Screenshots: `/tmp/pokemon-vendor-icon-pikachu-20260523.png`, `/tmp/pokemon-vendor-icon-dragonite-20260523.png`, `/tmp/pokemon-vendor-icon-mystery-20260523.png`, `/tmp/pokemon-vendor-icon-cancel-20260523.png`. Session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |
| 2026-05-23 | Vendor long-list build pass | Pass | `rtk git diff --check`, `rtk make -j16 -O all`, `rtk make -j16 -O debug`, `rtk make -j16 -O check`, and `rtk mdbook build docs` passed after keeping the normal shop font / row spacing, adding scroll arrows, and expanding the debug sample table to 10 products. Existing RWX linker warning, expected test markers, and existing mdbook warnings only. |
| 2026-05-23 | mGBA Live vendor long-list route | Pass | Booted the debug ROM, continued the existing save, opened debug `Scripts... -> Script 1`, and confirmed the normal-size four-row list with a visible down arrow, scrolling to later products and Cancel, up / down arrow visibility updates, detail icon / price updates, and Cancel icon clearing. Screenshots: `/tmp/pokemon-vendor-arrow-list-open-20260523.png`, `/tmp/pokemon-vendor-arrow-list-scrolled-20260523.png`, `/tmp/pokemon-vendor-arrow-list-cancel-20260523.png`. Session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |
| 2026-05-29 | Runtime integration apply | Pass | Re-applied #57 source / tests onto `integration/runtime-dev-20260529` after #47/#48/#54/#51. `data/scripts/debug.inc` was manually resolved so vendor remains `Script 1`, Scout pick-6 remains `Script 2`, and vendor queued trainer reward remains `Script 3`. |
| 2026-05-29 | `rtk git diff --check` / `--cached --check` | Pass | No whitespace issues after resolving the debug route overlap. |
| 2026-05-29 | `rtk make -j16 -O check TESTS=test/pokemon_vendor.c` | Pass | Vendor ABI, locked display, concealed Egg visuals, and sealed unlock helpers passed on the integration stack. |
| 2026-05-29 | `rtk make -j16 -O all` | Pass | Existing RWX linker warning only. |
| 2026-05-29 | `rtk make -j16 -O debug` | Pass | Existing RWX linker warning only; debug route merge compiled. |
| 2026-05-29 | `rtk make -j16 -O check TESTS=test/random.c` | Pass | Rechecked after full-suite jitter. Random tests passed when isolated. |
| 2026-05-29 | `rtk make -j16 -O check` | Not clean | Full suite reached runtime tests but failed the existing timing-sensitive `test/random.c` RandomUniform faster-than-mod benchmarks in hydra parallel context. The same file passed focused immediately after; no vendor / Pokemon failure was reported. This is recorded as accepted non-feature validation risk for this adoption. |
| 2026-05-29 | mGBA Live `integration-pokemon-vendor-smoke` | Pass | Booted `pokeemerald.gba`, continued the local save, opened `Debug Menu > Scripts... > Script 1`, confirmed the vendor list / detail icon pane, opened the Pikachu purchase prompt, bought Pikachu, and saw `Here you go! Take good care of it.` Screenshots: `/tmp/integration-pokemon-vendor-open.png`, `/tmp/integration-pokemon-vendor-buy-prompt.png`, `/tmp/integration-pokemon-vendor-after-buy.png`. Session stopped cleanly and `mgba-live-cli status --all` returned `[]`. |
| 2026-05-30 | Integration review cursor clamp | Pass | `codex review --base master` found that buying a one-time product from a scrolled long list could rebuild the list with an out-of-range scroll offset. `PokemonVendorClampListCursor()` now clamps scroll offset and selected row before `ListMenuInit()`. `rtk git diff --check` and `rtk make -j16 -O check TESTS=test/pokemon_vendor.c` pass. |
| 2026-05-30 | Integration review allocation guard | Pass | Second `codex review --base master` reported unchecked row allocations in `PokemonVendorBuildList()`. The build path now validates `items`, `names`, and `productIndexes`, frees partial allocations, and returns to script / closes the vendor instead of crashing. Focused `rtk make -j16 -O check TESTS=test/pokemon_vendor.c`, full `all` / `debug` / `check`, docs build, and final mGBA smoke passed on the integration branch. |

GitHub Actions were not re-waited; local build, test, and mGBA evidence are the
handoff evidence for this implementation update.

## Integration Handoff

- Integration commit target: `integration/runtime-dev-20260529`.
- Source shelf retained for history: PR #57 / `feature/global-no-evolution-20260523`.
- This adoption depends on the already-applied item and party baseline:
  #47 Battle Item Restore, #48 Held Item Catalog, #54 Party / Status UI, and
  #51 Scout Selection.
- Do not merge this runtime source into `master`. For `master`, cherry-pick or
  re-apply docs / Lua-only handoff content only.
- Next likely conflict is #60 All Ability Slots because it also touches
  `src/party_menu.c`, `src/pokemon_summary_screen.c`, `include/pokemon.h`,
  `src/pokemon.c`, and `test/pokemon.c`.
