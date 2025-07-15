EventScript_PicnicBasket::
# 4 "data/scripts/custom.pory"
	playse SE_USE_ITEM
# 5 "data/scripts/custom.pory"
	msgbox gText_UsePicnicBasket, MSGBOX_DEFAULT
# 6 "data/scripts/custom.pory"
	special HealPlayerParty
# 7 "data/scripts/custom.pory"
	release
	end


EventScript_Clock::
# 19 "data/scripts/custom.pory"
	call PlayersHouse_2F_EventScript_SetWallClock
# 20 "data/scripts/custom.pory"
	goto Debug_EventScript_TellTheTime
	end


Common_EventScript_Pokemart::
# 26 "data/scripts/custom.pory"
	lock
# 27 "data/scripts/custom.pory"
	faceplayer
# 28 "data/scripts/custom.pory"
	message gText_HowMayIServeYou
# 29 "data/scripts/custom.pory"
	waitmessage
# 30 "data/scripts/custom.pory"
	dynmultichoice 0, 0, TRUE, 5, 0, DYN_MULTICHOICE_CB_NONE, gText_Shop_Medicine, gText_ShopSpec, gText_ShopLevelUp, gText_ShopPokeball, gText_ShopTMHMs, gText_ShopGimmick, gText_ShopHeldItems, gText_ShopEvolution, gText_ShopFormItems, gText_ShopBerry, gText_ShopCansel
# 44 "data/scripts/custom.pory"
	switch VAR_RESULT
# 46 "data/scripts/custom.pory"
	case 0, Common_EventScript_Pokemart_3
# 48 "data/scripts/custom.pory"
	case 1, Common_EventScript_Pokemart_4
# 50 "data/scripts/custom.pory"
	case 2, Common_EventScript_Pokemart_5
# 52 "data/scripts/custom.pory"
	case 3, Common_EventScript_Pokemart_6
# 54 "data/scripts/custom.pory"
	case 4, Common_EventScript_Pokemart_7
# 56 "data/scripts/custom.pory"
	case 5, Common_EventScript_Pokemart_8
# 58 "data/scripts/custom.pory"
	case 6, Common_EventScript_Pokemart_9
# 60 "data/scripts/custom.pory"
	case 7, Common_EventScript_Pokemart_10
# 62 "data/scripts/custom.pory"
	case 8, Common_EventScript_Pokemart_11
# 64 "data/scripts/custom.pory"
	case 9, Common_EventScript_Pokemart_12
Common_EventScript_Pokemart_1:
# 68 "data/scripts/custom.pory"
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
# 69 "data/scripts/custom.pory"
	release
	end

Common_EventScript_Pokemart_3:
# 47 "data/scripts/custom.pory"
	pokemart Common_Shop_Medicine
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_4:
# 49 "data/scripts/custom.pory"
	pokemart Common_Shop_Spec
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_5:
# 51 "data/scripts/custom.pory"
	pokemart Common_Shop_Levelup
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_6:
# 53 "data/scripts/custom.pory"
	pokemart Common_Shop_Pokeball
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_7:
# 55 "data/scripts/custom.pory"
	pokemart Common_Shop_TMs
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_8:
# 57 "data/scripts/custom.pory"
	pokemart Common_Shop_Gimmick
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_9:
# 59 "data/scripts/custom.pory"
	pokemart Common_Shop_HeldItems
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_10:
# 61 "data/scripts/custom.pory"
	pokemart Common_Shop_Evolution
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_11:
# 63 "data/scripts/custom.pory"
	pokemart Common_Shop_FormItems
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_12:
# 65 "data/scripts/custom.pory"
	pokemart Common_Shop_Berry
	goto Common_EventScript_Pokemart_1


	.align 2
# 86 "data/scripts/custom.pory"
Common_Shop_Medicine:
# 88 "data/scripts/custom.pory"
	.2byte ITEM_POTION
# 89 "data/scripts/custom.pory"
	.2byte ITEM_SUPER_POTION
# 90 "data/scripts/custom.pory"
	.2byte ITEM_HYPER_POTION
# 91 "data/scripts/custom.pory"
	.2byte ITEM_MAX_POTION
# 92 "data/scripts/custom.pory"
	.2byte ITEM_FULL_RESTORE
# 93 "data/scripts/custom.pory"
	.2byte ITEM_REVIVE
# 94 "data/scripts/custom.pory"
	.2byte ITEM_MAX_REVIVE
# 95 "data/scripts/custom.pory"
	.2byte ITEM_FRESH_WATER
# 96 "data/scripts/custom.pory"
	.2byte ITEM_SODA_POP
# 97 "data/scripts/custom.pory"
	.2byte ITEM_LEMONADE
# 98 "data/scripts/custom.pory"
	.2byte ITEM_MOOMOO_MILK
# 99 "data/scripts/custom.pory"
	.2byte ITEM_ENERGY_POWDER
# 100 "data/scripts/custom.pory"
	.2byte ITEM_ENERGY_ROOT
# 101 "data/scripts/custom.pory"
	.2byte ITEM_HEAL_POWDER
# 102 "data/scripts/custom.pory"
	.2byte ITEM_REVIVAL_HERB
# 103 "data/scripts/custom.pory"
	.2byte ITEM_ANTIDOTE
# 104 "data/scripts/custom.pory"
	.2byte ITEM_PARALYZE_HEAL
# 105 "data/scripts/custom.pory"
	.2byte ITEM_BURN_HEAL
# 106 "data/scripts/custom.pory"
	.2byte ITEM_ICE_HEAL
# 107 "data/scripts/custom.pory"
	.2byte ITEM_AWAKENING
# 108 "data/scripts/custom.pory"
	.2byte ITEM_FULL_HEAL
# 109 "data/scripts/custom.pory"
	.2byte ITEM_ETHER
# 110 "data/scripts/custom.pory"
	.2byte ITEM_MAX_ETHER
# 111 "data/scripts/custom.pory"
	.2byte ITEM_ELIXIR
# 112 "data/scripts/custom.pory"
	.2byte ITEM_MAX_ELIXIR
# 113 "data/scripts/custom.pory"
	.2byte ITEM_BERRY_JUICE
# 114 "data/scripts/custom.pory"
	.2byte ITEM_SACRED_ASH
# 115 "data/scripts/custom.pory"
	.2byte ITEM_SWEET_HEART
# 116 "data/scripts/custom.pory"
	.2byte ITEM_MAX_HONEY
	.2byte ITEM_NONE

	.align 2
# 119 "data/scripts/custom.pory"
Common_Shop_Pokeball:
# 121 "data/scripts/custom.pory"
	.2byte ITEM_POKE_BALL
# 122 "data/scripts/custom.pory"
	.2byte ITEM_GREAT_BALL
# 123 "data/scripts/custom.pory"
	.2byte ITEM_ULTRA_BALL
# 124 "data/scripts/custom.pory"
	.2byte ITEM_MASTER_BALL
# 125 "data/scripts/custom.pory"
	.2byte ITEM_HEAL_BALL
# 126 "data/scripts/custom.pory"
	.2byte ITEM_NEST_BALL
# 127 "data/scripts/custom.pory"
	.2byte ITEM_PREMIER_BALL
# 128 "data/scripts/custom.pory"
	.2byte ITEM_NET_BALL
# 129 "data/scripts/custom.pory"
	.2byte ITEM_DIVE_BALL
# 130 "data/scripts/custom.pory"
	.2byte ITEM_DUSK_BALL
# 131 "data/scripts/custom.pory"
	.2byte ITEM_TIMER_BALL
# 132 "data/scripts/custom.pory"
	.2byte ITEM_QUICK_BALL
# 133 "data/scripts/custom.pory"
	.2byte ITEM_REPEAT_BALL
# 134 "data/scripts/custom.pory"
	.2byte ITEM_LUXURY_BALL
# 135 "data/scripts/custom.pory"
	.2byte ITEM_LEVEL_BALL
# 136 "data/scripts/custom.pory"
	.2byte ITEM_LURE_BALL
# 137 "data/scripts/custom.pory"
	.2byte ITEM_MOON_BALL
# 138 "data/scripts/custom.pory"
	.2byte ITEM_FRIEND_BALL
# 139 "data/scripts/custom.pory"
	.2byte ITEM_LOVE_BALL
# 140 "data/scripts/custom.pory"
	.2byte ITEM_FAST_BALL
# 141 "data/scripts/custom.pory"
	.2byte ITEM_HEAVY_BALL
# 142 "data/scripts/custom.pory"
	.2byte ITEM_DREAM_BALL
# 143 "data/scripts/custom.pory"
	.2byte ITEM_SAFARI_BALL
# 144 "data/scripts/custom.pory"
	.2byte ITEM_SPORT_BALL
# 145 "data/scripts/custom.pory"
	.2byte ITEM_PARK_BALL
# 146 "data/scripts/custom.pory"
	.2byte ITEM_BEAST_BALL
# 147 "data/scripts/custom.pory"
	.2byte ITEM_CHERISH_BALL
	.2byte ITEM_NONE

	.align 2
# 150 "data/scripts/custom.pory"
Common_Shop_Evolution:
# 152 "data/scripts/custom.pory"
	.2byte ITEM_FIRE_STONE
# 153 "data/scripts/custom.pory"
	.2byte ITEM_WATER_STONE
# 154 "data/scripts/custom.pory"
	.2byte ITEM_THUNDER_STONE
# 155 "data/scripts/custom.pory"
	.2byte ITEM_LEAF_STONE
# 156 "data/scripts/custom.pory"
	.2byte ITEM_ICE_STONE
# 157 "data/scripts/custom.pory"
	.2byte ITEM_SUN_STONE
# 158 "data/scripts/custom.pory"
	.2byte ITEM_MOON_STONE
# 159 "data/scripts/custom.pory"
	.2byte ITEM_SHINY_STONE
# 160 "data/scripts/custom.pory"
	.2byte ITEM_DUSK_STONE
# 161 "data/scripts/custom.pory"
	.2byte ITEM_DAWN_STONE
# 162 "data/scripts/custom.pory"
	.2byte ITEM_LINKING_CORD
# 163 "data/scripts/custom.pory"
	.2byte ITEM_KINGS_ROCK
# 164 "data/scripts/custom.pory"
	.2byte ITEM_METAL_COAT
# 165 "data/scripts/custom.pory"
	.2byte ITEM_DRAGON_SCALE
# 166 "data/scripts/custom.pory"
	.2byte ITEM_UPGRADE
# 167 "data/scripts/custom.pory"
	.2byte ITEM_DEEP_SEA_SCALE
# 168 "data/scripts/custom.pory"
	.2byte ITEM_DEEP_SEA_TOOTH
# 169 "data/scripts/custom.pory"
	.2byte ITEM_OVAL_STONE
# 170 "data/scripts/custom.pory"
	.2byte ITEM_PROTECTOR
# 171 "data/scripts/custom.pory"
	.2byte ITEM_ELECTIRIZER
# 172 "data/scripts/custom.pory"
	.2byte ITEM_MAGMARIZER
# 173 "data/scripts/custom.pory"
	.2byte ITEM_DUBIOUS_DISC
# 174 "data/scripts/custom.pory"
	.2byte ITEM_REAPER_CLOTH
# 175 "data/scripts/custom.pory"
	.2byte ITEM_PRISM_SCALE
# 176 "data/scripts/custom.pory"
	.2byte ITEM_WHIPPED_DREAM
# 177 "data/scripts/custom.pory"
	.2byte ITEM_SACHET
# 178 "data/scripts/custom.pory"
	.2byte ITEM_SWEET_APPLE
# 179 "data/scripts/custom.pory"
	.2byte ITEM_TART_APPLE
# 180 "data/scripts/custom.pory"
	.2byte ITEM_SYRUPY_APPLE
# 181 "data/scripts/custom.pory"
	.2byte ITEM_CRACKED_POT
# 182 "data/scripts/custom.pory"
	.2byte ITEM_CHIPPED_POT
# 183 "data/scripts/custom.pory"
	.2byte ITEM_GALARICA_CUFF
# 184 "data/scripts/custom.pory"
	.2byte ITEM_GALARICA_WREATH
# 185 "data/scripts/custom.pory"
	.2byte ITEM_STRAWBERRY_SWEET
# 186 "data/scripts/custom.pory"
	.2byte ITEM_LOVE_SWEET
# 187 "data/scripts/custom.pory"
	.2byte ITEM_BERRY_SWEET
# 188 "data/scripts/custom.pory"
	.2byte ITEM_CLOVER_SWEET
# 189 "data/scripts/custom.pory"
	.2byte ITEM_FLOWER_SWEET
# 190 "data/scripts/custom.pory"
	.2byte ITEM_STAR_SWEET
# 191 "data/scripts/custom.pory"
	.2byte ITEM_RIBBON_SWEET
# 192 "data/scripts/custom.pory"
	.2byte ITEM_BLACK_AUGURITE
# 193 "data/scripts/custom.pory"
	.2byte ITEM_PEAT_BLOCK
# 194 "data/scripts/custom.pory"
	.2byte ITEM_SCROLL_OF_DARKNESS
# 195 "data/scripts/custom.pory"
	.2byte ITEM_SCROLL_OF_WATERS
# 196 "data/scripts/custom.pory"
	.2byte ITEM_AUSPICIOUS_ARMOR
# 197 "data/scripts/custom.pory"
	.2byte ITEM_MALICIOUS_ARMOR
# 198 "data/scripts/custom.pory"
	.2byte ITEM_GIMMIGHOUL_COIN
# 199 "data/scripts/custom.pory"
	.2byte ITEM_LEADERS_CREST
# 200 "data/scripts/custom.pory"
	.2byte ITEM_UNREMARKABLE_TEACUP
# 201 "data/scripts/custom.pory"
	.2byte ITEM_MASTERPIECE_TEACUP
# 202 "data/scripts/custom.pory"
	.2byte ITEM_METAL_ALLOY
	.2byte ITEM_NONE

	.align 2
# 205 "data/scripts/custom.pory"
Common_Shop_Spec:
# 207 "data/scripts/custom.pory"
	.2byte ITEM_ABILITY_CAPSULE
# 208 "data/scripts/custom.pory"
	.2byte ITEM_ABILITY_PATCH
# 209 "data/scripts/custom.pory"
	.2byte ITEM_LONELY_MINT
# 210 "data/scripts/custom.pory"
	.2byte ITEM_ADAMANT_MINT
# 211 "data/scripts/custom.pory"
	.2byte ITEM_NAUGHTY_MINT
# 212 "data/scripts/custom.pory"
	.2byte ITEM_BRAVE_MINT
# 213 "data/scripts/custom.pory"
	.2byte ITEM_BOLD_MINT
# 214 "data/scripts/custom.pory"
	.2byte ITEM_IMPISH_MINT
# 215 "data/scripts/custom.pory"
	.2byte ITEM_LAX_MINT
# 216 "data/scripts/custom.pory"
	.2byte ITEM_RELAXED_MINT
# 217 "data/scripts/custom.pory"
	.2byte ITEM_MODEST_MINT
# 218 "data/scripts/custom.pory"
	.2byte ITEM_MILD_MINT
# 219 "data/scripts/custom.pory"
	.2byte ITEM_RASH_MINT
# 220 "data/scripts/custom.pory"
	.2byte ITEM_QUIET_MINT
# 221 "data/scripts/custom.pory"
	.2byte ITEM_CALM_MINT
# 222 "data/scripts/custom.pory"
	.2byte ITEM_GENTLE_MINT
# 223 "data/scripts/custom.pory"
	.2byte ITEM_CAREFUL_MINT
# 224 "data/scripts/custom.pory"
	.2byte ITEM_SASSY_MINT
# 225 "data/scripts/custom.pory"
	.2byte ITEM_TIMID_MINT
# 226 "data/scripts/custom.pory"
	.2byte ITEM_HASTY_MINT
# 227 "data/scripts/custom.pory"
	.2byte ITEM_JOLLY_MINT
# 228 "data/scripts/custom.pory"
	.2byte ITEM_NAIVE_MINT
# 229 "data/scripts/custom.pory"
	.2byte ITEM_SERIOUS_MINT
	.2byte ITEM_NONE

	.align 2
# 232 "data/scripts/custom.pory"
Common_Shop_Levelup:
# 234 "data/scripts/custom.pory"
	.2byte ITEM_RARE_CANDY
# 235 "data/scripts/custom.pory"
	.2byte ITEM_EXP_CANDY_XS
# 236 "data/scripts/custom.pory"
	.2byte ITEM_EXP_CANDY_S
# 237 "data/scripts/custom.pory"
	.2byte ITEM_EXP_CANDY_M
# 238 "data/scripts/custom.pory"
	.2byte ITEM_EXP_CANDY_L
# 239 "data/scripts/custom.pory"
	.2byte ITEM_EXP_CANDY_XL
# 240 "data/scripts/custom.pory"
	.2byte ITEM_HP_UP
# 241 "data/scripts/custom.pory"
	.2byte ITEM_PROTEIN
# 242 "data/scripts/custom.pory"
	.2byte ITEM_IRON
# 243 "data/scripts/custom.pory"
	.2byte ITEM_CALCIUM
# 244 "data/scripts/custom.pory"
	.2byte ITEM_ZINC
# 245 "data/scripts/custom.pory"
	.2byte ITEM_CARBOS
# 246 "data/scripts/custom.pory"
	.2byte ITEM_HEALTH_FEATHER
# 247 "data/scripts/custom.pory"
	.2byte ITEM_MUSCLE_FEATHER
# 248 "data/scripts/custom.pory"
	.2byte ITEM_RESIST_FEATHER
# 249 "data/scripts/custom.pory"
	.2byte ITEM_GENIUS_FEATHER
# 250 "data/scripts/custom.pory"
	.2byte ITEM_CLEVER_FEATHER
# 251 "data/scripts/custom.pory"
	.2byte ITEM_SWIFT_FEATHER
# 252 "data/scripts/custom.pory"
	.2byte ITEM_PP_UP
# 253 "data/scripts/custom.pory"
	.2byte ITEM_PP_MAX
# 254 "data/scripts/custom.pory"
	.2byte ITEM_DYNAMAX_CANDY
	.2byte ITEM_NONE

	.align 2
# 257 "data/scripts/custom.pory"
Common_Shop_HeldItems:
# 259 "data/scripts/custom.pory"
	.2byte ITEM_SILK_SCARF
# 260 "data/scripts/custom.pory"
	.2byte ITEM_CHARCOAL
# 261 "data/scripts/custom.pory"
	.2byte ITEM_MYSTIC_WATER
# 262 "data/scripts/custom.pory"
	.2byte ITEM_MAGNET
# 263 "data/scripts/custom.pory"
	.2byte ITEM_MIRACLE_SEED
# 264 "data/scripts/custom.pory"
	.2byte ITEM_NEVER_MELT_ICE
# 265 "data/scripts/custom.pory"
	.2byte ITEM_BLACK_BELT
# 266 "data/scripts/custom.pory"
	.2byte ITEM_POISON_BARB
# 267 "data/scripts/custom.pory"
	.2byte ITEM_SOFT_SAND
# 268 "data/scripts/custom.pory"
	.2byte ITEM_SHARP_BEAK
# 269 "data/scripts/custom.pory"
	.2byte ITEM_TWISTED_SPOON
# 270 "data/scripts/custom.pory"
	.2byte ITEM_SILVER_POWDER
# 271 "data/scripts/custom.pory"
	.2byte ITEM_HARD_STONE
# 272 "data/scripts/custom.pory"
	.2byte ITEM_SPELL_TAG
# 273 "data/scripts/custom.pory"
	.2byte ITEM_DRAGON_FANG
# 274 "data/scripts/custom.pory"
	.2byte ITEM_BLACK_GLASSES
# 275 "data/scripts/custom.pory"
	.2byte ITEM_METAL_COAT
# 276 "data/scripts/custom.pory"
	.2byte ITEM_FAIRY_FEATHER
# 277 "data/scripts/custom.pory"
	.2byte ITEM_CHOICE_BAND
# 278 "data/scripts/custom.pory"
	.2byte ITEM_CHOICE_SPECS
# 279 "data/scripts/custom.pory"
	.2byte ITEM_CHOICE_SCARF
# 280 "data/scripts/custom.pory"
	.2byte ITEM_FLAME_ORB
# 281 "data/scripts/custom.pory"
	.2byte ITEM_TOXIC_ORB
# 282 "data/scripts/custom.pory"
	.2byte ITEM_DAMP_ROCK
# 283 "data/scripts/custom.pory"
	.2byte ITEM_HEAT_ROCK
# 284 "data/scripts/custom.pory"
	.2byte ITEM_SMOOTH_ROCK
# 285 "data/scripts/custom.pory"
	.2byte ITEM_ICY_ROCK
# 286 "data/scripts/custom.pory"
	.2byte ITEM_ELECTRIC_SEED
# 287 "data/scripts/custom.pory"
	.2byte ITEM_PSYCHIC_SEED
# 288 "data/scripts/custom.pory"
	.2byte ITEM_MISTY_SEED
# 289 "data/scripts/custom.pory"
	.2byte ITEM_GRASSY_SEED
# 290 "data/scripts/custom.pory"
	.2byte ITEM_ABSORB_BULB
# 291 "data/scripts/custom.pory"
	.2byte ITEM_CELL_BATTERY
# 292 "data/scripts/custom.pory"
	.2byte ITEM_LUMINOUS_MOSS
# 293 "data/scripts/custom.pory"
	.2byte ITEM_SNOWBALL
# 294 "data/scripts/custom.pory"
	.2byte ITEM_BRIGHT_POWDER
# 295 "data/scripts/custom.pory"
	.2byte ITEM_WHITE_HERB
# 296 "data/scripts/custom.pory"
	.2byte ITEM_EXP_SHARE
# 297 "data/scripts/custom.pory"
	.2byte ITEM_QUICK_CLAW
# 298 "data/scripts/custom.pory"
	.2byte ITEM_SOOTHE_BELL
# 299 "data/scripts/custom.pory"
	.2byte ITEM_MENTAL_HERB
# 300 "data/scripts/custom.pory"
	.2byte ITEM_KINGS_ROCK
# 301 "data/scripts/custom.pory"
	.2byte ITEM_AMULET_COIN
# 302 "data/scripts/custom.pory"
	.2byte ITEM_CLEANSE_TAG
# 303 "data/scripts/custom.pory"
	.2byte ITEM_SMOKE_BALL
# 304 "data/scripts/custom.pory"
	.2byte ITEM_FOCUS_BAND
# 305 "data/scripts/custom.pory"
	.2byte ITEM_LUCKY_EGG
# 306 "data/scripts/custom.pory"
	.2byte ITEM_SCOPE_LENS
# 307 "data/scripts/custom.pory"
	.2byte ITEM_LEFTOVERS
# 308 "data/scripts/custom.pory"
	.2byte ITEM_SHELL_BELL
# 309 "data/scripts/custom.pory"
	.2byte ITEM_WIDE_LENS
# 310 "data/scripts/custom.pory"
	.2byte ITEM_MUSCLE_BAND
# 311 "data/scripts/custom.pory"
	.2byte ITEM_WISE_GLASSES
# 312 "data/scripts/custom.pory"
	.2byte ITEM_EXPERT_BELT
# 313 "data/scripts/custom.pory"
	.2byte ITEM_LIGHT_CLAY
# 314 "data/scripts/custom.pory"
	.2byte ITEM_LIFE_ORB
# 315 "data/scripts/custom.pory"
	.2byte ITEM_POWER_HERB
# 316 "data/scripts/custom.pory"
	.2byte ITEM_FOCUS_SASH
# 317 "data/scripts/custom.pory"
	.2byte ITEM_ZOOM_LENS
# 318 "data/scripts/custom.pory"
	.2byte ITEM_METRONOME
# 319 "data/scripts/custom.pory"
	.2byte ITEM_IRON_BALL
# 320 "data/scripts/custom.pory"
	.2byte ITEM_LAGGING_TAIL
# 321 "data/scripts/custom.pory"
	.2byte ITEM_DESTINY_KNOT
# 322 "data/scripts/custom.pory"
	.2byte ITEM_BLACK_SLUDGE
# 323 "data/scripts/custom.pory"
	.2byte ITEM_GRIP_CLAW
# 324 "data/scripts/custom.pory"
	.2byte ITEM_STICKY_BARB
# 325 "data/scripts/custom.pory"
	.2byte ITEM_SHED_SHELL
# 326 "data/scripts/custom.pory"
	.2byte ITEM_BIG_ROOT
# 327 "data/scripts/custom.pory"
	.2byte ITEM_RAZOR_CLAW
# 328 "data/scripts/custom.pory"
	.2byte ITEM_RAZOR_FANG
# 329 "data/scripts/custom.pory"
	.2byte ITEM_EVIOLITE
# 330 "data/scripts/custom.pory"
	.2byte ITEM_FLOAT_STONE
# 331 "data/scripts/custom.pory"
	.2byte ITEM_ROCKY_HELMET
# 332 "data/scripts/custom.pory"
	.2byte ITEM_AIR_BALLOON
# 333 "data/scripts/custom.pory"
	.2byte ITEM_RED_CARD
# 334 "data/scripts/custom.pory"
	.2byte ITEM_RING_TARGET
# 335 "data/scripts/custom.pory"
	.2byte ITEM_BINDING_BAND
# 336 "data/scripts/custom.pory"
	.2byte ITEM_EJECT_BUTTON
# 337 "data/scripts/custom.pory"
	.2byte ITEM_WEAKNESS_POLICY
# 338 "data/scripts/custom.pory"
	.2byte ITEM_ASSAULT_VEST
# 339 "data/scripts/custom.pory"
	.2byte ITEM_SAFETY_GOGGLES
# 340 "data/scripts/custom.pory"
	.2byte ITEM_ADRENALINE_ORB
# 341 "data/scripts/custom.pory"
	.2byte ITEM_TERRAIN_EXTENDER
# 342 "data/scripts/custom.pory"
	.2byte ITEM_PROTECTIVE_PADS
# 343 "data/scripts/custom.pory"
	.2byte ITEM_THROAT_SPRAY
# 344 "data/scripts/custom.pory"
	.2byte ITEM_EJECT_PACK
# 345 "data/scripts/custom.pory"
	.2byte ITEM_HEAVY_DUTY_BOOTS
# 346 "data/scripts/custom.pory"
	.2byte ITEM_BLUNDER_POLICY
# 347 "data/scripts/custom.pory"
	.2byte ITEM_ROOM_SERVICE
# 348 "data/scripts/custom.pory"
	.2byte ITEM_UTILITY_UMBRELLA
# 349 "data/scripts/custom.pory"
	.2byte ITEM_ABILITY_SHIELD
# 350 "data/scripts/custom.pory"
	.2byte ITEM_CLEAR_AMULET
# 351 "data/scripts/custom.pory"
	.2byte ITEM_PUNCHING_GLOVE
# 352 "data/scripts/custom.pory"
	.2byte ITEM_COVERT_CLOAK
# 353 "data/scripts/custom.pory"
	.2byte ITEM_LOADED_DICE
# 354 "data/scripts/custom.pory"
	.2byte ITEM_BOOSTER_ENERGY
# 355 "data/scripts/custom.pory"
	.2byte ITEM_MIRROR_HERB
	.2byte ITEM_NONE

	.align 2
# 358 "data/scripts/custom.pory"
Common_Shop_Gimmick:
# 360 "data/scripts/custom.pory"
	.2byte ITEM_NORMAL_GEM
# 361 "data/scripts/custom.pory"
	.2byte ITEM_FIRE_GEM
# 362 "data/scripts/custom.pory"
	.2byte ITEM_WATER_GEM
# 363 "data/scripts/custom.pory"
	.2byte ITEM_ELECTRIC_GEM
# 364 "data/scripts/custom.pory"
	.2byte ITEM_GRASS_GEM
# 365 "data/scripts/custom.pory"
	.2byte ITEM_ICE_GEM
# 366 "data/scripts/custom.pory"
	.2byte ITEM_FIGHTING_GEM
# 367 "data/scripts/custom.pory"
	.2byte ITEM_POISON_GEM
# 368 "data/scripts/custom.pory"
	.2byte ITEM_GROUND_GEM
# 369 "data/scripts/custom.pory"
	.2byte ITEM_FLYING_GEM
# 370 "data/scripts/custom.pory"
	.2byte ITEM_PSYCHIC_GEM
# 371 "data/scripts/custom.pory"
	.2byte ITEM_BUG_GEM
# 372 "data/scripts/custom.pory"
	.2byte ITEM_ROCK_GEM
# 373 "data/scripts/custom.pory"
	.2byte ITEM_GHOST_GEM
# 374 "data/scripts/custom.pory"
	.2byte ITEM_DRAGON_GEM
# 375 "data/scripts/custom.pory"
	.2byte ITEM_DARK_GEM
# 376 "data/scripts/custom.pory"
	.2byte ITEM_STEEL_GEM
# 377 "data/scripts/custom.pory"
	.2byte ITEM_FAIRY_GEM
# 378 "data/scripts/custom.pory"
	.2byte ITEM_VENUSAURITE
# 379 "data/scripts/custom.pory"
	.2byte ITEM_CHARIZARDITE_X
# 380 "data/scripts/custom.pory"
	.2byte ITEM_CHARIZARDITE_Y
# 381 "data/scripts/custom.pory"
	.2byte ITEM_BLASTOISINITE
# 382 "data/scripts/custom.pory"
	.2byte ITEM_BEEDRILLITE
# 383 "data/scripts/custom.pory"
	.2byte ITEM_PIDGEOTITE
# 384 "data/scripts/custom.pory"
	.2byte ITEM_ALAKAZITE
# 385 "data/scripts/custom.pory"
	.2byte ITEM_SLOWBRONITE
# 386 "data/scripts/custom.pory"
	.2byte ITEM_GENGARITE
# 387 "data/scripts/custom.pory"
	.2byte ITEM_KANGASKHANITE
# 388 "data/scripts/custom.pory"
	.2byte ITEM_PINSIRITE
# 389 "data/scripts/custom.pory"
	.2byte ITEM_GYARADOSITE
# 390 "data/scripts/custom.pory"
	.2byte ITEM_AERODACTYLITE
# 391 "data/scripts/custom.pory"
	.2byte ITEM_MEWTWONITE_X
# 392 "data/scripts/custom.pory"
	.2byte ITEM_MEWTWONITE_Y
# 393 "data/scripts/custom.pory"
	.2byte ITEM_AMPHAROSITE
# 394 "data/scripts/custom.pory"
	.2byte ITEM_STEELIXITE
# 395 "data/scripts/custom.pory"
	.2byte ITEM_SCIZORITE
# 396 "data/scripts/custom.pory"
	.2byte ITEM_HERACRONITE
# 397 "data/scripts/custom.pory"
	.2byte ITEM_HOUNDOOMINITE
# 398 "data/scripts/custom.pory"
	.2byte ITEM_TYRANITARITE
# 399 "data/scripts/custom.pory"
	.2byte ITEM_SCEPTILITE
# 400 "data/scripts/custom.pory"
	.2byte ITEM_BLAZIKENITE
# 401 "data/scripts/custom.pory"
	.2byte ITEM_SWAMPERTITE
# 402 "data/scripts/custom.pory"
	.2byte ITEM_GARDEVOIRITE
# 403 "data/scripts/custom.pory"
	.2byte ITEM_SABLENITE
# 404 "data/scripts/custom.pory"
	.2byte ITEM_MAWILITE
# 405 "data/scripts/custom.pory"
	.2byte ITEM_AGGRONITE
# 406 "data/scripts/custom.pory"
	.2byte ITEM_MEDICHAMITE
# 407 "data/scripts/custom.pory"
	.2byte ITEM_MANECTITE
# 408 "data/scripts/custom.pory"
	.2byte ITEM_SHARPEDONITE
# 409 "data/scripts/custom.pory"
	.2byte ITEM_CAMERUPTITE
# 410 "data/scripts/custom.pory"
	.2byte ITEM_ALTARIANITE
# 411 "data/scripts/custom.pory"
	.2byte ITEM_BANETTITE
# 412 "data/scripts/custom.pory"
	.2byte ITEM_ABSOLITE
# 413 "data/scripts/custom.pory"
	.2byte ITEM_GLALITITE
# 414 "data/scripts/custom.pory"
	.2byte ITEM_SALAMENCITE
# 415 "data/scripts/custom.pory"
	.2byte ITEM_METAGROSSITE
# 416 "data/scripts/custom.pory"
	.2byte ITEM_LATIASITE
# 417 "data/scripts/custom.pory"
	.2byte ITEM_LATIOSITE
# 418 "data/scripts/custom.pory"
	.2byte ITEM_LOPUNNITE
# 419 "data/scripts/custom.pory"
	.2byte ITEM_GARCHOMPITE
# 420 "data/scripts/custom.pory"
	.2byte ITEM_LUCARIONITE
# 421 "data/scripts/custom.pory"
	.2byte ITEM_ABOMASITE
# 422 "data/scripts/custom.pory"
	.2byte ITEM_GALLADITE
# 423 "data/scripts/custom.pory"
	.2byte ITEM_AUDINITE
# 424 "data/scripts/custom.pory"
	.2byte ITEM_DIANCITE
# 425 "data/scripts/custom.pory"
	.2byte ITEM_RED_ORB
# 426 "data/scripts/custom.pory"
	.2byte ITEM_BLUE_ORB
# 427 "data/scripts/custom.pory"
	.2byte ITEM_NORMALIUM_Z
# 428 "data/scripts/custom.pory"
	.2byte ITEM_FIRIUM_Z
# 429 "data/scripts/custom.pory"
	.2byte ITEM_WATERIUM_Z
# 430 "data/scripts/custom.pory"
	.2byte ITEM_ELECTRIUM_Z
# 431 "data/scripts/custom.pory"
	.2byte ITEM_GRASSIUM_Z
# 432 "data/scripts/custom.pory"
	.2byte ITEM_ICIUM_Z
# 433 "data/scripts/custom.pory"
	.2byte ITEM_FIGHTINIUM_Z
# 434 "data/scripts/custom.pory"
	.2byte ITEM_POISONIUM_Z
# 435 "data/scripts/custom.pory"
	.2byte ITEM_GROUNDIUM_Z
# 436 "data/scripts/custom.pory"
	.2byte ITEM_FLYINIUM_Z
# 437 "data/scripts/custom.pory"
	.2byte ITEM_PSYCHIUM_Z
# 438 "data/scripts/custom.pory"
	.2byte ITEM_BUGINIUM_Z
# 439 "data/scripts/custom.pory"
	.2byte ITEM_ROCKIUM_Z
# 440 "data/scripts/custom.pory"
	.2byte ITEM_GHOSTIUM_Z
# 441 "data/scripts/custom.pory"
	.2byte ITEM_DRAGONIUM_Z
# 442 "data/scripts/custom.pory"
	.2byte ITEM_DARKINIUM_Z
# 443 "data/scripts/custom.pory"
	.2byte ITEM_STEELIUM_Z
# 444 "data/scripts/custom.pory"
	.2byte ITEM_FAIRIUM_Z
# 445 "data/scripts/custom.pory"
	.2byte ITEM_PIKANIUM_Z
# 446 "data/scripts/custom.pory"
	.2byte ITEM_EEVIUM_Z
# 447 "data/scripts/custom.pory"
	.2byte ITEM_SNORLIUM_Z
# 448 "data/scripts/custom.pory"
	.2byte ITEM_MEWNIUM_Z
# 449 "data/scripts/custom.pory"
	.2byte ITEM_DECIDIUM_Z
# 450 "data/scripts/custom.pory"
	.2byte ITEM_INCINIUM_Z
# 451 "data/scripts/custom.pory"
	.2byte ITEM_PRIMARIUM_Z
# 452 "data/scripts/custom.pory"
	.2byte ITEM_LYCANIUM_Z
# 453 "data/scripts/custom.pory"
	.2byte ITEM_MIMIKIUM_Z
# 454 "data/scripts/custom.pory"
	.2byte ITEM_KOMMONIUM_Z
# 455 "data/scripts/custom.pory"
	.2byte ITEM_TAPUNIUM_Z
# 456 "data/scripts/custom.pory"
	.2byte ITEM_SOLGANIUM_Z
# 457 "data/scripts/custom.pory"
	.2byte ITEM_LUNALIUM_Z
# 458 "data/scripts/custom.pory"
	.2byte ITEM_MARSHADIUM_Z
# 459 "data/scripts/custom.pory"
	.2byte ITEM_ALORAICHIUM_Z
# 460 "data/scripts/custom.pory"
	.2byte ITEM_PIKASHUNIUM_Z
# 461 "data/scripts/custom.pory"
	.2byte ITEM_ULTRANECROZIUM_Z
# 462 "data/scripts/custom.pory"
	.2byte ITEM_BUG_TERA_SHARD
# 463 "data/scripts/custom.pory"
	.2byte ITEM_DARK_TERA_SHARD
# 464 "data/scripts/custom.pory"
	.2byte ITEM_DRAGON_TERA_SHARD
# 465 "data/scripts/custom.pory"
	.2byte ITEM_ELECTRIC_TERA_SHARD
# 466 "data/scripts/custom.pory"
	.2byte ITEM_FAIRY_TERA_SHARD
# 467 "data/scripts/custom.pory"
	.2byte ITEM_FIGHTING_TERA_SHARD
# 468 "data/scripts/custom.pory"
	.2byte ITEM_FIRE_TERA_SHARD
# 469 "data/scripts/custom.pory"
	.2byte ITEM_FLYING_TERA_SHARD
# 470 "data/scripts/custom.pory"
	.2byte ITEM_GHOST_TERA_SHARD
# 471 "data/scripts/custom.pory"
	.2byte ITEM_GRASS_TERA_SHARD
# 472 "data/scripts/custom.pory"
	.2byte ITEM_GROUND_TERA_SHARD
# 473 "data/scripts/custom.pory"
	.2byte ITEM_ICE_TERA_SHARD
# 474 "data/scripts/custom.pory"
	.2byte ITEM_NORMAL_TERA_SHARD
# 475 "data/scripts/custom.pory"
	.2byte ITEM_POISON_TERA_SHARD
# 476 "data/scripts/custom.pory"
	.2byte ITEM_PSYCHIC_TERA_SHARD
# 477 "data/scripts/custom.pory"
	.2byte ITEM_ROCK_TERA_SHARD
# 478 "data/scripts/custom.pory"
	.2byte ITEM_STEEL_TERA_SHARD
# 479 "data/scripts/custom.pory"
	.2byte ITEM_WATER_TERA_SHARD
# 480 "data/scripts/custom.pory"
	.2byte ITEM_STELLAR_TERA_SHARD
	.2byte ITEM_NONE

	.align 2
# 483 "data/scripts/custom.pory"
Common_Shop_FormItems:
# 485 "data/scripts/custom.pory"
	.2byte ITEM_LIGHT_BALL
# 486 "data/scripts/custom.pory"
	.2byte ITEM_LEEK
# 487 "data/scripts/custom.pory"
	.2byte ITEM_THICK_CLUB
# 488 "data/scripts/custom.pory"
	.2byte ITEM_LUCKY_PUNCH
# 489 "data/scripts/custom.pory"
	.2byte ITEM_METAL_POWDER
# 490 "data/scripts/custom.pory"
	.2byte ITEM_QUICK_POWDER
# 491 "data/scripts/custom.pory"
	.2byte ITEM_DEEP_SEA_SCALE
# 492 "data/scripts/custom.pory"
	.2byte ITEM_DEEP_SEA_TOOTH
# 493 "data/scripts/custom.pory"
	.2byte ITEM_SOUL_DEW
# 494 "data/scripts/custom.pory"
	.2byte ITEM_METEORITE
# 495 "data/scripts/custom.pory"
	.2byte ITEM_ROTOM_CATALOG
# 496 "data/scripts/custom.pory"
	.2byte ITEM_ADAMANT_ORB
# 497 "data/scripts/custom.pory"
	.2byte ITEM_LUSTROUS_ORB
# 498 "data/scripts/custom.pory"
	.2byte ITEM_GRISEOUS_ORB
# 499 "data/scripts/custom.pory"
	.2byte ITEM_ADAMANT_CRYSTAL
# 500 "data/scripts/custom.pory"
	.2byte ITEM_LUSTROUS_GLOBE
# 501 "data/scripts/custom.pory"
	.2byte ITEM_GRISEOUS_CORE
# 502 "data/scripts/custom.pory"
	.2byte ITEM_GRACIDEA
# 503 "data/scripts/custom.pory"
	.2byte ITEM_FLAME_PLATE
# 504 "data/scripts/custom.pory"
	.2byte ITEM_SPLASH_PLATE
# 505 "data/scripts/custom.pory"
	.2byte ITEM_ZAP_PLATE
# 506 "data/scripts/custom.pory"
	.2byte ITEM_MEADOW_PLATE
# 507 "data/scripts/custom.pory"
	.2byte ITEM_ICICLE_PLATE
# 508 "data/scripts/custom.pory"
	.2byte ITEM_FIST_PLATE
# 509 "data/scripts/custom.pory"
	.2byte ITEM_TOXIC_PLATE
# 510 "data/scripts/custom.pory"
	.2byte ITEM_EARTH_PLATE
# 511 "data/scripts/custom.pory"
	.2byte ITEM_SKY_PLATE
# 512 "data/scripts/custom.pory"
	.2byte ITEM_MIND_PLATE
# 513 "data/scripts/custom.pory"
	.2byte ITEM_INSECT_PLATE
# 514 "data/scripts/custom.pory"
	.2byte ITEM_STONE_PLATE
# 515 "data/scripts/custom.pory"
	.2byte ITEM_SPOOKY_PLATE
# 516 "data/scripts/custom.pory"
	.2byte ITEM_DRACO_PLATE
# 517 "data/scripts/custom.pory"
	.2byte ITEM_DREAD_PLATE
# 518 "data/scripts/custom.pory"
	.2byte ITEM_IRON_PLATE
# 519 "data/scripts/custom.pory"
	.2byte ITEM_PIXIE_PLATE
# 520 "data/scripts/custom.pory"
	.2byte ITEM_REVEAL_GLASS
# 521 "data/scripts/custom.pory"
	.2byte ITEM_DNA_SPLICERS
# 522 "data/scripts/custom.pory"
	.2byte ITEM_DOUSE_DRIVE
# 523 "data/scripts/custom.pory"
	.2byte ITEM_SHOCK_DRIVE
# 524 "data/scripts/custom.pory"
	.2byte ITEM_BURN_DRIVE
# 525 "data/scripts/custom.pory"
	.2byte ITEM_CHILL_DRIVE
# 526 "data/scripts/custom.pory"
	.2byte ITEM_ZYGARDE_CUBE
# 527 "data/scripts/custom.pory"
	.2byte ITEM_PRISON_BOTTLE
# 528 "data/scripts/custom.pory"
	.2byte ITEM_RED_NECTAR
# 529 "data/scripts/custom.pory"
	.2byte ITEM_YELLOW_NECTAR
# 530 "data/scripts/custom.pory"
	.2byte ITEM_PINK_NECTAR
# 531 "data/scripts/custom.pory"
	.2byte ITEM_PURPLE_NECTAR
# 532 "data/scripts/custom.pory"
	.2byte ITEM_FIRE_MEMORY
# 533 "data/scripts/custom.pory"
	.2byte ITEM_WATER_MEMORY
# 534 "data/scripts/custom.pory"
	.2byte ITEM_ELECTRIC_MEMORY
# 535 "data/scripts/custom.pory"
	.2byte ITEM_GRASS_MEMORY
# 536 "data/scripts/custom.pory"
	.2byte ITEM_ICE_MEMORY
# 537 "data/scripts/custom.pory"
	.2byte ITEM_FIGHTING_MEMORY
# 538 "data/scripts/custom.pory"
	.2byte ITEM_POISON_MEMORY
# 539 "data/scripts/custom.pory"
	.2byte ITEM_GROUND_MEMORY
# 540 "data/scripts/custom.pory"
	.2byte ITEM_FLYING_MEMORY
# 541 "data/scripts/custom.pory"
	.2byte ITEM_PSYCHIC_MEMORY
# 542 "data/scripts/custom.pory"
	.2byte ITEM_BUG_MEMORY
# 543 "data/scripts/custom.pory"
	.2byte ITEM_ROCK_MEMORY
# 544 "data/scripts/custom.pory"
	.2byte ITEM_GHOST_MEMORY
# 545 "data/scripts/custom.pory"
	.2byte ITEM_DRAGON_MEMORY
# 546 "data/scripts/custom.pory"
	.2byte ITEM_DARK_MEMORY
# 547 "data/scripts/custom.pory"
	.2byte ITEM_STEEL_MEMORY
# 548 "data/scripts/custom.pory"
	.2byte ITEM_FAIRY_MEMORY
# 549 "data/scripts/custom.pory"
	.2byte ITEM_N_SOLARIZER
# 550 "data/scripts/custom.pory"
	.2byte ITEM_N_LUNARIZER
# 551 "data/scripts/custom.pory"
	.2byte ITEM_RUSTED_SWORD
# 552 "data/scripts/custom.pory"
	.2byte ITEM_RUSTED_SHIELD
# 553 "data/scripts/custom.pory"
	.2byte ITEM_REINS_OF_UNITY
# 554 "data/scripts/custom.pory"
	.2byte ITEM_WELLSPRING_MASK
# 555 "data/scripts/custom.pory"
	.2byte ITEM_HEARTHFLAME_MASK
# 556 "data/scripts/custom.pory"
	.2byte ITEM_CORNERSTONE_MASK
	.2byte ITEM_NONE

	.align 2
# 559 "data/scripts/custom.pory"
Common_Shop_Berry:
# 561 "data/scripts/custom.pory"
	.2byte ITEM_CHERI_BERRY
# 562 "data/scripts/custom.pory"
	.2byte ITEM_CHESTO_BERRY
# 563 "data/scripts/custom.pory"
	.2byte ITEM_PECHA_BERRY
# 564 "data/scripts/custom.pory"
	.2byte ITEM_RAWST_BERRY
# 565 "data/scripts/custom.pory"
	.2byte ITEM_ASPEAR_BERRY
# 566 "data/scripts/custom.pory"
	.2byte ITEM_LEPPA_BERRY
# 567 "data/scripts/custom.pory"
	.2byte ITEM_ORAN_BERRY
# 568 "data/scripts/custom.pory"
	.2byte ITEM_PERSIM_BERRY
# 569 "data/scripts/custom.pory"
	.2byte ITEM_LUM_BERRY
# 570 "data/scripts/custom.pory"
	.2byte ITEM_SITRUS_BERRY
# 571 "data/scripts/custom.pory"
	.2byte ITEM_FIGY_BERRY
# 572 "data/scripts/custom.pory"
	.2byte ITEM_WIKI_BERRY
# 573 "data/scripts/custom.pory"
	.2byte ITEM_MAGO_BERRY
# 574 "data/scripts/custom.pory"
	.2byte ITEM_AGUAV_BERRY
# 575 "data/scripts/custom.pory"
	.2byte ITEM_IAPAPA_BERRY
# 576 "data/scripts/custom.pory"
	.2byte ITEM_RAZZ_BERRY
# 577 "data/scripts/custom.pory"
	.2byte ITEM_BLUK_BERRY
# 578 "data/scripts/custom.pory"
	.2byte ITEM_NANAB_BERRY
# 579 "data/scripts/custom.pory"
	.2byte ITEM_WEPEAR_BERRY
# 580 "data/scripts/custom.pory"
	.2byte ITEM_PINAP_BERRY
# 581 "data/scripts/custom.pory"
	.2byte ITEM_POMEG_BERRY
# 582 "data/scripts/custom.pory"
	.2byte ITEM_KELPSY_BERRY
# 583 "data/scripts/custom.pory"
	.2byte ITEM_QUALOT_BERRY
# 584 "data/scripts/custom.pory"
	.2byte ITEM_HONDEW_BERRY
# 585 "data/scripts/custom.pory"
	.2byte ITEM_GREPA_BERRY
# 586 "data/scripts/custom.pory"
	.2byte ITEM_TAMATO_BERRY
# 587 "data/scripts/custom.pory"
	.2byte ITEM_CORNN_BERRY
# 588 "data/scripts/custom.pory"
	.2byte ITEM_MAGOST_BERRY
# 589 "data/scripts/custom.pory"
	.2byte ITEM_RABUTA_BERRY
# 590 "data/scripts/custom.pory"
	.2byte ITEM_NOMEL_BERRY
# 591 "data/scripts/custom.pory"
	.2byte ITEM_SPELON_BERRY
# 592 "data/scripts/custom.pory"
	.2byte ITEM_PAMTRE_BERRY
# 593 "data/scripts/custom.pory"
	.2byte ITEM_WATMEL_BERRY
# 594 "data/scripts/custom.pory"
	.2byte ITEM_DURIN_BERRY
# 595 "data/scripts/custom.pory"
	.2byte ITEM_BELUE_BERRY
# 596 "data/scripts/custom.pory"
	.2byte ITEM_CHILAN_BERRY
# 597 "data/scripts/custom.pory"
	.2byte ITEM_OCCA_BERRY
# 598 "data/scripts/custom.pory"
	.2byte ITEM_PASSHO_BERRY
# 599 "data/scripts/custom.pory"
	.2byte ITEM_WACAN_BERRY
# 600 "data/scripts/custom.pory"
	.2byte ITEM_RINDO_BERRY
# 601 "data/scripts/custom.pory"
	.2byte ITEM_YACHE_BERRY
# 602 "data/scripts/custom.pory"
	.2byte ITEM_CHOPLE_BERRY
# 603 "data/scripts/custom.pory"
	.2byte ITEM_KEBIA_BERRY
# 604 "data/scripts/custom.pory"
	.2byte ITEM_SHUCA_BERRY
# 605 "data/scripts/custom.pory"
	.2byte ITEM_COBA_BERRY
# 606 "data/scripts/custom.pory"
	.2byte ITEM_PAYAPA_BERRY
# 607 "data/scripts/custom.pory"
	.2byte ITEM_TANGA_BERRY
# 608 "data/scripts/custom.pory"
	.2byte ITEM_CHARTI_BERRY
# 609 "data/scripts/custom.pory"
	.2byte ITEM_KASIB_BERRY
# 610 "data/scripts/custom.pory"
	.2byte ITEM_HABAN_BERRY
# 611 "data/scripts/custom.pory"
	.2byte ITEM_COLBUR_BERRY
# 612 "data/scripts/custom.pory"
	.2byte ITEM_BABIRI_BERRY
# 613 "data/scripts/custom.pory"
	.2byte ITEM_ROSELI_BERRY
# 614 "data/scripts/custom.pory"
	.2byte ITEM_LIECHI_BERRY
# 615 "data/scripts/custom.pory"
	.2byte ITEM_GANLON_BERRY
# 616 "data/scripts/custom.pory"
	.2byte ITEM_SALAC_BERRY
# 617 "data/scripts/custom.pory"
	.2byte ITEM_PETAYA_BERRY
# 618 "data/scripts/custom.pory"
	.2byte ITEM_APICOT_BERRY
# 619 "data/scripts/custom.pory"
	.2byte ITEM_LANSAT_BERRY
# 620 "data/scripts/custom.pory"
	.2byte ITEM_STARF_BERRY
# 621 "data/scripts/custom.pory"
	.2byte ITEM_ENIGMA_BERRY
# 622 "data/scripts/custom.pory"
	.2byte ITEM_MICLE_BERRY
# 623 "data/scripts/custom.pory"
	.2byte ITEM_CUSTAP_BERRY
# 624 "data/scripts/custom.pory"
	.2byte ITEM_JABOCA_BERRY
# 625 "data/scripts/custom.pory"
	.2byte ITEM_ROWAP_BERRY
# 626 "data/scripts/custom.pory"
	.2byte ITEM_KEE_BERRY
# 627 "data/scripts/custom.pory"
	.2byte ITEM_MARANGA_BERRY
	.2byte ITEM_NONE

	.align 2
# 630 "data/scripts/custom.pory"
Common_Shop_TMs:
# 632 "data/scripts/custom.pory"
	.2byte ITEM_TM01
# 633 "data/scripts/custom.pory"
	.2byte ITEM_TM02
# 634 "data/scripts/custom.pory"
	.2byte ITEM_TM03
# 635 "data/scripts/custom.pory"
	.2byte ITEM_TM04
# 636 "data/scripts/custom.pory"
	.2byte ITEM_TM05
# 637 "data/scripts/custom.pory"
	.2byte ITEM_TM06
# 638 "data/scripts/custom.pory"
	.2byte ITEM_TM07
# 639 "data/scripts/custom.pory"
	.2byte ITEM_TM08
# 640 "data/scripts/custom.pory"
	.2byte ITEM_TM09
# 641 "data/scripts/custom.pory"
	.2byte ITEM_TM10
# 642 "data/scripts/custom.pory"
	.2byte ITEM_TM11
# 643 "data/scripts/custom.pory"
	.2byte ITEM_TM12
# 644 "data/scripts/custom.pory"
	.2byte ITEM_TM13
# 645 "data/scripts/custom.pory"
	.2byte ITEM_TM14
# 646 "data/scripts/custom.pory"
	.2byte ITEM_TM15
# 647 "data/scripts/custom.pory"
	.2byte ITEM_TM16
# 648 "data/scripts/custom.pory"
	.2byte ITEM_TM17
# 649 "data/scripts/custom.pory"
	.2byte ITEM_TM18
# 650 "data/scripts/custom.pory"
	.2byte ITEM_TM19
# 651 "data/scripts/custom.pory"
	.2byte ITEM_TM20
# 652 "data/scripts/custom.pory"
	.2byte ITEM_TM21
# 653 "data/scripts/custom.pory"
	.2byte ITEM_TM22
# 654 "data/scripts/custom.pory"
	.2byte ITEM_TM23
# 655 "data/scripts/custom.pory"
	.2byte ITEM_TM24
# 656 "data/scripts/custom.pory"
	.2byte ITEM_TM25
# 657 "data/scripts/custom.pory"
	.2byte ITEM_TM26
# 658 "data/scripts/custom.pory"
	.2byte ITEM_TM27
# 659 "data/scripts/custom.pory"
	.2byte ITEM_TM28
# 660 "data/scripts/custom.pory"
	.2byte ITEM_TM29
# 661 "data/scripts/custom.pory"
	.2byte ITEM_TM30
# 662 "data/scripts/custom.pory"
	.2byte ITEM_TM31
# 663 "data/scripts/custom.pory"
	.2byte ITEM_TM32
# 664 "data/scripts/custom.pory"
	.2byte ITEM_TM33
# 665 "data/scripts/custom.pory"
	.2byte ITEM_TM34
# 666 "data/scripts/custom.pory"
	.2byte ITEM_TM35
# 667 "data/scripts/custom.pory"
	.2byte ITEM_TM36
# 668 "data/scripts/custom.pory"
	.2byte ITEM_TM37
# 669 "data/scripts/custom.pory"
	.2byte ITEM_TM38
# 670 "data/scripts/custom.pory"
	.2byte ITEM_TM39
# 671 "data/scripts/custom.pory"
	.2byte ITEM_TM40
# 672 "data/scripts/custom.pory"
	.2byte ITEM_TM41
# 673 "data/scripts/custom.pory"
	.2byte ITEM_TM42
# 674 "data/scripts/custom.pory"
	.2byte ITEM_TM43
# 675 "data/scripts/custom.pory"
	.2byte ITEM_TM44
# 676 "data/scripts/custom.pory"
	.2byte ITEM_TM45
# 677 "data/scripts/custom.pory"
	.2byte ITEM_TM46
# 678 "data/scripts/custom.pory"
	.2byte ITEM_TM47
# 679 "data/scripts/custom.pory"
	.2byte ITEM_TM48
# 680 "data/scripts/custom.pory"
	.2byte ITEM_TM49
# 681 "data/scripts/custom.pory"
	.2byte ITEM_TM50
# 682 "data/scripts/custom.pory"
	.2byte ITEM_TM51
# 683 "data/scripts/custom.pory"
	.2byte ITEM_TM52
# 684 "data/scripts/custom.pory"
	.2byte ITEM_TM53
# 685 "data/scripts/custom.pory"
	.2byte ITEM_TM54
# 686 "data/scripts/custom.pory"
	.2byte ITEM_TM55
# 687 "data/scripts/custom.pory"
	.2byte ITEM_TM56
# 688 "data/scripts/custom.pory"
	.2byte ITEM_TM57
# 689 "data/scripts/custom.pory"
	.2byte ITEM_TM58
# 690 "data/scripts/custom.pory"
	.2byte ITEM_TM59
# 691 "data/scripts/custom.pory"
	.2byte ITEM_TM60
# 692 "data/scripts/custom.pory"
	.2byte ITEM_TM61
# 693 "data/scripts/custom.pory"
	.2byte ITEM_TM62
# 694 "data/scripts/custom.pory"
	.2byte ITEM_TM63
# 695 "data/scripts/custom.pory"
	.2byte ITEM_TM64
# 696 "data/scripts/custom.pory"
	.2byte ITEM_TM65
# 697 "data/scripts/custom.pory"
	.2byte ITEM_TM66
# 698 "data/scripts/custom.pory"
	.2byte ITEM_TM67
# 699 "data/scripts/custom.pory"
	.2byte ITEM_TM68
# 700 "data/scripts/custom.pory"
	.2byte ITEM_TM69
# 701 "data/scripts/custom.pory"
	.2byte ITEM_TM70
# 702 "data/scripts/custom.pory"
	.2byte ITEM_TM71
# 703 "data/scripts/custom.pory"
	.2byte ITEM_TM72
# 704 "data/scripts/custom.pory"
	.2byte ITEM_TM73
# 705 "data/scripts/custom.pory"
	.2byte ITEM_TM74
# 706 "data/scripts/custom.pory"
	.2byte ITEM_TM75
# 707 "data/scripts/custom.pory"
	.2byte ITEM_TM76
# 708 "data/scripts/custom.pory"
	.2byte ITEM_TM77
# 709 "data/scripts/custom.pory"
	.2byte ITEM_TM78
# 710 "data/scripts/custom.pory"
	.2byte ITEM_TM79
# 711 "data/scripts/custom.pory"
	.2byte ITEM_TM80
# 712 "data/scripts/custom.pory"
	.2byte ITEM_TM81
# 713 "data/scripts/custom.pory"
	.2byte ITEM_TM82
# 714 "data/scripts/custom.pory"
	.2byte ITEM_TM83
# 715 "data/scripts/custom.pory"
	.2byte ITEM_TM84
# 716 "data/scripts/custom.pory"
	.2byte ITEM_TM85
# 717 "data/scripts/custom.pory"
	.2byte ITEM_TM86
# 718 "data/scripts/custom.pory"
	.2byte ITEM_TM87
# 719 "data/scripts/custom.pory"
	.2byte ITEM_TM88
# 720 "data/scripts/custom.pory"
	.2byte ITEM_TM89
# 721 "data/scripts/custom.pory"
	.2byte ITEM_TM90
# 722 "data/scripts/custom.pory"
	.2byte ITEM_TM91
# 723 "data/scripts/custom.pory"
	.2byte ITEM_TM92
# 724 "data/scripts/custom.pory"
	.2byte ITEM_TM93
# 725 "data/scripts/custom.pory"
	.2byte ITEM_TM94
# 726 "data/scripts/custom.pory"
	.2byte ITEM_TM95
# 727 "data/scripts/custom.pory"
	.2byte ITEM_TM96
# 728 "data/scripts/custom.pory"
	.2byte ITEM_TM97
# 729 "data/scripts/custom.pory"
	.2byte ITEM_TM98
# 730 "data/scripts/custom.pory"
	.2byte ITEM_TM99
# 731 "data/scripts/custom.pory"
	.2byte ITEM_TM100
# 732 "data/scripts/custom.pory"
	.2byte ITEM_TM101
# 733 "data/scripts/custom.pory"
	.2byte ITEM_TM102
# 734 "data/scripts/custom.pory"
	.2byte ITEM_TM103
# 735 "data/scripts/custom.pory"
	.2byte ITEM_TM104
# 736 "data/scripts/custom.pory"
	.2byte ITEM_TM105
# 737 "data/scripts/custom.pory"
	.2byte ITEM_TM106
# 738 "data/scripts/custom.pory"
	.2byte ITEM_TM107
# 739 "data/scripts/custom.pory"
	.2byte ITEM_TM108
# 740 "data/scripts/custom.pory"
	.2byte ITEM_TM109
# 741 "data/scripts/custom.pory"
	.2byte ITEM_TM110
# 742 "data/scripts/custom.pory"
	.2byte ITEM_TM111
# 743 "data/scripts/custom.pory"
	.2byte ITEM_TM112
# 744 "data/scripts/custom.pory"
	.2byte ITEM_TM113
# 745 "data/scripts/custom.pory"
	.2byte ITEM_TM114
# 746 "data/scripts/custom.pory"
	.2byte ITEM_TM115
# 747 "data/scripts/custom.pory"
	.2byte ITEM_TM116
# 748 "data/scripts/custom.pory"
	.2byte ITEM_TM117
# 749 "data/scripts/custom.pory"
	.2byte ITEM_TM118
# 750 "data/scripts/custom.pory"
	.2byte ITEM_TM119
# 751 "data/scripts/custom.pory"
	.2byte ITEM_TM120
# 752 "data/scripts/custom.pory"
	.2byte ITEM_TM121
# 753 "data/scripts/custom.pory"
	.2byte ITEM_TM122
# 754 "data/scripts/custom.pory"
	.2byte ITEM_TM123
# 755 "data/scripts/custom.pory"
	.2byte ITEM_TM124
# 756 "data/scripts/custom.pory"
	.2byte ITEM_TM125
# 757 "data/scripts/custom.pory"
	.2byte ITEM_TM126
# 758 "data/scripts/custom.pory"
	.2byte ITEM_TM127
# 759 "data/scripts/custom.pory"
	.2byte ITEM_TM128
# 760 "data/scripts/custom.pory"
	.2byte ITEM_TM129
# 761 "data/scripts/custom.pory"
	.2byte ITEM_TM130
# 762 "data/scripts/custom.pory"
	.2byte ITEM_TM131
# 763 "data/scripts/custom.pory"
	.2byte ITEM_TM132
# 764 "data/scripts/custom.pory"
	.2byte ITEM_TM133
# 765 "data/scripts/custom.pory"
	.2byte ITEM_TM134
# 766 "data/scripts/custom.pory"
	.2byte ITEM_TM135
# 767 "data/scripts/custom.pory"
	.2byte ITEM_TM136
# 768 "data/scripts/custom.pory"
	.2byte ITEM_TM137
# 769 "data/scripts/custom.pory"
	.2byte ITEM_TM138
# 770 "data/scripts/custom.pory"
	.2byte ITEM_TM139
# 771 "data/scripts/custom.pory"
	.2byte ITEM_TM140
# 772 "data/scripts/custom.pory"
	.2byte ITEM_TM141
# 773 "data/scripts/custom.pory"
	.2byte ITEM_TM142
# 774 "data/scripts/custom.pory"
	.2byte ITEM_TM143
# 775 "data/scripts/custom.pory"
	.2byte ITEM_TM144
# 776 "data/scripts/custom.pory"
	.2byte ITEM_TM145
# 777 "data/scripts/custom.pory"
	.2byte ITEM_TM146
# 778 "data/scripts/custom.pory"
	.2byte ITEM_TM147
# 779 "data/scripts/custom.pory"
	.2byte ITEM_TM148
# 780 "data/scripts/custom.pory"
	.2byte ITEM_TM149
# 781 "data/scripts/custom.pory"
	.2byte ITEM_TM150
# 782 "data/scripts/custom.pory"
	.2byte ITEM_TM151
# 783 "data/scripts/custom.pory"
	.2byte ITEM_TM152
# 784 "data/scripts/custom.pory"
	.2byte ITEM_TM153
# 785 "data/scripts/custom.pory"
	.2byte ITEM_TM154
# 786 "data/scripts/custom.pory"
	.2byte ITEM_TM155
# 787 "data/scripts/custom.pory"
	.2byte ITEM_TM156
# 788 "data/scripts/custom.pory"
	.2byte ITEM_TM157
# 789 "data/scripts/custom.pory"
	.2byte ITEM_TM158
# 790 "data/scripts/custom.pory"
	.2byte ITEM_TM159
# 791 "data/scripts/custom.pory"
	.2byte ITEM_TM160
# 792 "data/scripts/custom.pory"
	.2byte ITEM_TM161
# 793 "data/scripts/custom.pory"
	.2byte ITEM_TM162
# 794 "data/scripts/custom.pory"
	.2byte ITEM_TM163
# 795 "data/scripts/custom.pory"
	.2byte ITEM_TM164
# 796 "data/scripts/custom.pory"
	.2byte ITEM_TM165
# 797 "data/scripts/custom.pory"
	.2byte ITEM_TM166
# 798 "data/scripts/custom.pory"
	.2byte ITEM_TM167
# 799 "data/scripts/custom.pory"
	.2byte ITEM_TM168
# 800 "data/scripts/custom.pory"
	.2byte ITEM_TM169
# 801 "data/scripts/custom.pory"
	.2byte ITEM_TM170
# 802 "data/scripts/custom.pory"
	.2byte ITEM_TM171
# 803 "data/scripts/custom.pory"
	.2byte ITEM_TM172
# 804 "data/scripts/custom.pory"
	.2byte ITEM_TM173
# 805 "data/scripts/custom.pory"
	.2byte ITEM_TM174
# 806 "data/scripts/custom.pory"
	.2byte ITEM_TM175
# 807 "data/scripts/custom.pory"
	.2byte ITEM_TM176
# 808 "data/scripts/custom.pory"
	.2byte ITEM_TM177
# 809 "data/scripts/custom.pory"
	.2byte ITEM_TM178
# 810 "data/scripts/custom.pory"
	.2byte ITEM_TM179
# 811 "data/scripts/custom.pory"
	.2byte ITEM_TM180
# 812 "data/scripts/custom.pory"
	.2byte ITEM_TM181
# 813 "data/scripts/custom.pory"
	.2byte ITEM_TM182
# 814 "data/scripts/custom.pory"
	.2byte ITEM_TM183
# 815 "data/scripts/custom.pory"
	.2byte ITEM_TM184
# 816 "data/scripts/custom.pory"
	.2byte ITEM_TM185
# 817 "data/scripts/custom.pory"
	.2byte ITEM_TM186
# 818 "data/scripts/custom.pory"
	.2byte ITEM_TM187
# 819 "data/scripts/custom.pory"
	.2byte ITEM_TM188
# 820 "data/scripts/custom.pory"
	.2byte ITEM_TM189
# 821 "data/scripts/custom.pory"
	.2byte ITEM_TM190
# 822 "data/scripts/custom.pory"
	.2byte ITEM_TM191
# 823 "data/scripts/custom.pory"
	.2byte ITEM_TM192
# 824 "data/scripts/custom.pory"
	.2byte ITEM_TM193
# 825 "data/scripts/custom.pory"
	.2byte ITEM_TM194
# 826 "data/scripts/custom.pory"
	.2byte ITEM_TM195
# 827 "data/scripts/custom.pory"
	.2byte ITEM_TM196
# 828 "data/scripts/custom.pory"
	.2byte ITEM_TM197
# 829 "data/scripts/custom.pory"
	.2byte ITEM_TM198
# 830 "data/scripts/custom.pory"
	.2byte ITEM_TM199
# 831 "data/scripts/custom.pory"
	.2byte ITEM_TM200
# 832 "data/scripts/custom.pory"
	.2byte ITEM_TM201
# 833 "data/scripts/custom.pory"
	.2byte ITEM_TM202
# 834 "data/scripts/custom.pory"
	.2byte ITEM_TM203
# 835 "data/scripts/custom.pory"
	.2byte ITEM_TM204
# 836 "data/scripts/custom.pory"
	.2byte ITEM_TM205
# 837 "data/scripts/custom.pory"
	.2byte ITEM_TM206
# 838 "data/scripts/custom.pory"
	.2byte ITEM_TM207
# 839 "data/scripts/custom.pory"
	.2byte ITEM_TM208
# 840 "data/scripts/custom.pory"
	.2byte ITEM_TM209
# 841 "data/scripts/custom.pory"
	.2byte ITEM_TM210
# 842 "data/scripts/custom.pory"
	.2byte ITEM_TM211
# 843 "data/scripts/custom.pory"
	.2byte ITEM_TM212
# 844 "data/scripts/custom.pory"
	.2byte ITEM_TM213
# 845 "data/scripts/custom.pory"
	.2byte ITEM_TM214
# 846 "data/scripts/custom.pory"
	.2byte ITEM_TM215
# 847 "data/scripts/custom.pory"
	.2byte ITEM_TM216
# 848 "data/scripts/custom.pory"
	.2byte ITEM_TM217
# 849 "data/scripts/custom.pory"
	.2byte ITEM_TM218
# 850 "data/scripts/custom.pory"
	.2byte ITEM_TM219
# 851 "data/scripts/custom.pory"
	.2byte ITEM_TM220
# 852 "data/scripts/custom.pory"
	.2byte ITEM_TM221
# 853 "data/scripts/custom.pory"
	.2byte ITEM_TM222
# 854 "data/scripts/custom.pory"
	.2byte ITEM_TM223
# 855 "data/scripts/custom.pory"
	.2byte ITEM_TM224
# 856 "data/scripts/custom.pory"
	.2byte ITEM_TM225
# 857 "data/scripts/custom.pory"
	.2byte ITEM_TM226
# 858 "data/scripts/custom.pory"
	.2byte ITEM_TM227
# 859 "data/scripts/custom.pory"
	.2byte ITEM_TM228
# 860 "data/scripts/custom.pory"
	.2byte ITEM_TM229
# 861 "data/scripts/custom.pory"
	.2byte ITEM_HM01
# 862 "data/scripts/custom.pory"
	.2byte ITEM_HM02
# 863 "data/scripts/custom.pory"
	.2byte ITEM_HM03
# 864 "data/scripts/custom.pory"
	.2byte ITEM_HM04
# 865 "data/scripts/custom.pory"
	.2byte ITEM_HM05
# 866 "data/scripts/custom.pory"
	.2byte ITEM_HM06
# 867 "data/scripts/custom.pory"
	.2byte ITEM_HM07
# 868 "data/scripts/custom.pory"
	.2byte ITEM_HM08
	.2byte ITEM_NONE

Common_EventScript_Money::
# 873 "data/scripts/custom.pory"
	lock
# 874 "data/scripts/custom.pory"
	faceplayer
# 875 "data/scripts/custom.pory"
	showmoneybox 0, 0
# 877 "data/scripts/custom.pory"
	msgbox Common_Text_MoneyStart, MSGBOX_YESNO
# 878 "data/scripts/custom.pory"
	compare VAR_RESULT, 0
	goto_if_ne Common_EventScript_Money_2
# 899 "data/scripts/custom.pory"
	msgbox Common_Text_MoneyCansel, MSGBOX_DEFAULT
# 900 "data/scripts/custom.pory"
	closemessage
Common_EventScript_Money_1:
# 902 "data/scripts/custom.pory"
	hidemoneybox
# 903 "data/scripts/custom.pory"
	waitmovement 0
# 904 "data/scripts/custom.pory"
	releaseall
	end

Common_EventScript_Money_2:
	checkitem ITEM_RELIC_SILVER
# 0 "data/scripts/custom.pory"
	compare VAR_RESULT, TRUE
	goto_if_eq Common_EventScript_Money_5
# 893 "data/scripts/custom.pory"
	msgbox Common_Text_MoneyHasNone, MSGBOX_DEFAULT
# 894 "data/scripts/custom.pory"
	closemessage
	goto Common_EventScript_Money_1

Common_EventScript_Money_5:
# 883 "data/scripts/custom.pory"
	addmoney 5000
# 884 "data/scripts/custom.pory"
	updatemoneybox
# 885 "data/scripts/custom.pory"
	playse SE_SHOP
# 886 "data/scripts/custom.pory"
	removeitem ITEM_RELIC_SILVER, 1
# 888 "data/scripts/custom.pory"
	msgbox Common_Text_MoneyChange, MSGBOX_AUTOCLOSE
	goto Common_EventScript_Money_1


gText_UsePicnicBasket::
# 11 "data/scripts/custom.pory"
	.string "{PLAYER} had a picnic with the patry.\n"
	.string "All Pokémon were fully restored!$"

gText_Shop_Medicine::
# 73 "data/scripts/custom.pory"
	.string "Medicine$"

gText_ShopSpec::
# 74 "data/scripts/custom.pory"
	.string "Ability/Nature$"

gText_ShopLevelUp::
# 75 "data/scripts/custom.pory"
	.string "Level up$"

gText_ShopPokeball::
# 76 "data/scripts/custom.pory"
	.string "Poke ball$"

gText_ShopTMHMs::
# 77 "data/scripts/custom.pory"
	.string "TM / HMs$"

gText_ShopGimmick::
# 78 "data/scripts/custom.pory"
	.string "Gimmicks$"

gText_ShopHeldItems::
# 79 "data/scripts/custom.pory"
	.string "Held Items$"

gText_ShopEvolution::
# 80 "data/scripts/custom.pory"
	.string "Evolution$"

gText_ShopFormItems::
# 81 "data/scripts/custom.pory"
	.string "Form Items$"

gText_ShopBerry::
# 82 "data/scripts/custom.pory"
	.string "Berry$"

gText_ShopCansel::
# 83 "data/scripts/custom.pory"
	.string "   Cansel$"

Common_Text_MoneyStart::
# 908 "data/scripts/custom.pory"
	.string "If you have RELIC_SILVER I will buy it\n"
	.string "from you for 5,000$.$"

Common_Text_MoneyHasNone::
# 913 "data/scripts/custom.pory"
	.string "Don't lie to me.$"

Common_Text_MoneyChange::
# 918 "data/scripts/custom.pory"
	.string "Negotiations were concluded! If I see\n"
	.string "it again, I'll buy it.$"

Common_Text_MoneyCansel::
# 923 "data/scripts/custom.pory"
	.string "Okay. Let me know if you need anything\n"
	.string "else.$"
