# Pokemon Vendor Manual

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-23 |
| Baseline | `feature/global-no-evolution-20260523`; implementation PR #57 |
| Code status | Runtime implementation lives on feature branch; master handoff is docs-only |
| Provenance | Local project overlay |

This manual explains how to add a Friendly Shop style Pokemon vendor once the
runtime branch is integrated. The vendor sells Pokemon products through a shop
flow without pretending that Pokemon are bag items.

## When To Use

Use the Pokemon vendor when an NPC, debug script, or challenge room should sell
one or more Pokemon products.

Use ordinary `pokemart` only for item lists. Do not put fake Pokemon item ids in
normal marts; normal marts end in the item purchase path.

## Main Files

| Purpose | File |
|---|---|
| Script command macro | `asm/macros/event.inc` |
| Script command table | `data/script_cmd_table.inc` |
| Vendor runtime | `src/pokemon_vendor.c` |
| Public product struct / helpers | `include/pokemon_vendor.h` |
| Product constants | `include/constants/pokemon_vendor.h` |
| Debug validation route | `data/scripts/debug.inc` |
| Feature docs | `docs/features/friendly_shop_pokemon_vendor/` |

## Basic Script

Put the product table on a 2-byte boundary and terminate it with
`pokemonvendorlistend`.

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

## Product Row

The macro accepts required fields first, followed by optional fields with safe
defaults.

```asm
pokemonvendorproduct productId, species, level, price, kind, purchaseMode, oneTimeFlag, unlockFlag, heldItem, ball, bondThreshold, bondYield, editPolicy, revealPolicy, ivs, move1, move2, move3, move4, randomSpecies1, randomSpecies2, randomSpecies3, randomSpecies4
```

| Field | Use |
|---|---|
| `productId` | Stable nonzero product id for docs / balancing. |
| `species` | Base species. Mystery products pick from this plus optional candidates. |
| `level` | Created Pokemon level. |
| `price` | Money price. Charged only after successful delivery. |
| `kind` | `POKEMON_VENDOR_PRODUCT_NORMAL` or `POKEMON_VENDOR_PRODUCT_SEALED`. |
| `purchaseMode` | `POKEMON_VENDOR_PURCHASE_REPEAT` or `POKEMON_VENDOR_PURCHASE_ONCE`. |
| `oneTimeFlag` | Set after successful one-time purchase. Use `POKEMON_VENDOR_NO_FLAG` for repeat products. |
| `unlockFlag` | Optional pre-purchase row gate. This is not the same as a post-purchase locked Pokemon. |
| `heldItem` | Optional held item. Default `ITEM_NONE`. |
| `ball` | Pokeball id. Debug products use `27` for Precious Ball. |
| `bondThreshold` | Sealed recruit unlock threshold. Defaults to `POKEMON_VENDOR_DEFAULT_BOND_THRESHOLD`. |
| `bondYield` | Balancing metadata for future reward tables. |
| `editPolicy` | `POKEMON_VENDOR_EDIT_NORMAL` or `POKEMON_VENDOR_EDIT_ALWAYS`. |
| `revealPolicy` | `POKEMON_VENDOR_REVEAL_SPECIES`, `POKEMON_VENDOR_REVEAL_GATED`, or `POKEMON_VENDOR_REVEAL_HIDDEN`. |
| `ivs` | `USE_RANDOM_IVS` or a fixed-IV count policy. |
| `move1..move4` | Optional explicit moves. `MOVE_DEFAULT` keeps default slot behavior. |
| `randomSpecies1..4` | Optional mystery candidates. `SPECIES_NONE` means unused. |

## Vendor UI

- The list keeps normal shop font and row spacing.
- Four rows are visible at once.
- If more rows exist, up / down scroll arrows are shown.
- The right detail pane shows the selected product kind, level, repeat /
  one-time state, sealed bond progress target, and selected-product icon.
- Named products show the real species icon.
- Concealed `?????` products show a generic Egg icon.
- Cancel clears the selected-product icon so stale sprites do not remain.

## Delivery Rules

- Normal products use the scripted gift Pokemon delivery path and can fall back
  to PC when the party is full.
- Sealed products are created as locked vendor-origin Pokemon.
- Sealed products go to party when there is room and to PC when the party is
  full but storage has room.
- If neither party nor PC has room, the purchase fails and money is not removed.
- One-time flags are set only after successful delivery.

## Sealed Recruits

Sealed recruits are Egg-like locked Pokemon, but they are not biological eggs.
They may be final evolutions or legendary Pokemon if the product table says so.

Named sealed products keep the Pokemon identity visible while locked. Concealed
products remain hidden as `?????` in the shop and use Egg visuals after purchase
until unlock.

Locked sealed recruits:

- occupy a party slot when carried,
- cannot battle like ordinary party Pokemon,
- gain bond progress only through explicit script rewards,
- become normal usable Pokemon when bond progress reaches the threshold,
- preserve their vendor sealed-origin marker after unlock.

## Bond Reward Scripts

Use `pokemonvendorawardbond` for field / room-clear rewards.

```asm
SomeRoomClearReward:
	pokemonvendorawardbond 40
	return

SomeQuietReward:
	pokemonvendorawardbond 80, FALSE
	return
```

Use `pokemonvendorqueuebattlebond` before a trainer battle when the reward
message should appear in the battle victory flow after the money message.

```asm
SomeTrainer_PreBattle:
	pokemonvendorqueuebattlebond 20
	special PokemonVendor_StartDebugBondTrainerBattle
	waitstate
	return
```

`pokemonvendorawardbond amount[, showMessage]` writes:

| Output | Meaning |
|---|---|
| `VAR_0x8005` | Number of locked sealed recruits that gained progress. |
| `VAR_RESULT` | Number of recruits unlocked by this reward. |

## Checklist

- Product table has `.align 2`.
- Product table ends with `pokemonvendorlistend`.
- Every `productId` is nonzero and unique within the table.
- One-time products use a unique event flag.
- Gated rows use `unlockFlag`; post-purchase lock state is handled by sealed
  product data.
- Concealed products use `POKEMON_VENDOR_REVEAL_HIDDEN`.
- Mystery products include only intended species candidates.
- Full party and full storage failure paths are tested.
- Long lists are tested by scrolling to the last product and Cancel.
- For trainer rewards, confirm the queued reward appears in battle victory text
  rather than again on field return.

## Validation

For runtime changes, run:

```bash
rtk git diff --check
rtk make -j16 -O all
rtk make -j16 -O debug
rtk make -j16 -O check
rtk mdbook build docs
```

Then use mGBA Live or manual play to confirm:

- debug `Scripts... -> Script 1` opens the vendor,
- named products show real icons,
- concealed products show Egg icon,
- long lists show scroll arrows and reach Cancel,
- purchase success updates money and party / PC state,
- no-room failure does not charge money.
