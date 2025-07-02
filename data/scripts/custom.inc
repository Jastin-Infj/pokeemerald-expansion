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
	dynmultichoice 0, 0, TRUE, 5, 0, DYN_MULTICHOICE_CB_NONE, gText_ShopSpec, gText_ShopLevelUp, gText_ShopPokeball, gText_ShopTMHMs, gText_ShopGimmick, gText_ShopHeldItems, gText_ShopEvolution, gText_ShopFormItems, gText_ShopBerry, gText_ShopCansel
# 43 "data/scripts/custom.pory"
	switch VAR_RESULT
# 45 "data/scripts/custom.pory"
	case 0, Common_EventScript_Pokemart_3
# 47 "data/scripts/custom.pory"
	case 1, Common_EventScript_Pokemart_4
# 49 "data/scripts/custom.pory"
	case 2, Common_EventScript_Pokemart_5
# 51 "data/scripts/custom.pory"
	case 3, Common_EventScript_Pokemart_6
# 53 "data/scripts/custom.pory"
	case 4, Common_EventScript_Pokemart_7
# 55 "data/scripts/custom.pory"
	case 5, Common_EventScript_Pokemart_8
# 57 "data/scripts/custom.pory"
	case 6, Common_EventScript_Pokemart_9
# 59 "data/scripts/custom.pory"
	case 7, Common_EventScript_Pokemart_10
# 61 "data/scripts/custom.pory"
	case 8, Common_EventScript_Pokemart_11
Common_EventScript_Pokemart_1:
# 65 "data/scripts/custom.pory"
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
# 66 "data/scripts/custom.pory"
	release
	end

Common_EventScript_Pokemart_3:
# 46 "data/scripts/custom.pory"
	pokemart Common_Shop_Spec
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_4:
# 48 "data/scripts/custom.pory"
	pokemart Common_Shop_Levelup
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_5:
# 50 "data/scripts/custom.pory"
	pokemart Common_Shop_Pokeball
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_6:
# 52 "data/scripts/custom.pory"
	pokemart Common_Shop_TMs
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_7:
# 54 "data/scripts/custom.pory"
	pokemart Common_Shop_Gimmick
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_8:
# 56 "data/scripts/custom.pory"
	pokemart Common_Shop_HeldItems
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_9:
# 58 "data/scripts/custom.pory"
	pokemart Common_Shop_Evolution
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_10:
# 60 "data/scripts/custom.pory"
	pokemart Common_Shop_FormItems
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_11:
# 62 "data/scripts/custom.pory"
	pokemart Common_Shop_Berry
	goto Common_EventScript_Pokemart_1


	.align 2
# 81 "data/scripts/custom.pory"
Common_Shop_Pokeball:
# 83 "data/scripts/custom.pory"
	.2byte ITEM_POKE_BALL
# 84 "data/scripts/custom.pory"
	.2byte ITEM_GREAT_BALL
# 85 "data/scripts/custom.pory"
	.2byte ITEM_ULTRA_BALL
# 86 "data/scripts/custom.pory"
	.2byte ITEM_MASTER_BALL
# 87 "data/scripts/custom.pory"
	.2byte ITEM_HEAL_BALL
# 88 "data/scripts/custom.pory"
	.2byte ITEM_NEST_BALL
# 89 "data/scripts/custom.pory"
	.2byte ITEM_PREMIER_BALL
# 90 "data/scripts/custom.pory"
	.2byte ITEM_NET_BALL
# 91 "data/scripts/custom.pory"
	.2byte ITEM_DIVE_BALL
# 92 "data/scripts/custom.pory"
	.2byte ITEM_DUSK_BALL
# 93 "data/scripts/custom.pory"
	.2byte ITEM_TIMER_BALL
# 94 "data/scripts/custom.pory"
	.2byte ITEM_QUICK_BALL
# 95 "data/scripts/custom.pory"
	.2byte ITEM_REPEAT_BALL
# 96 "data/scripts/custom.pory"
	.2byte ITEM_LUXURY_BALL
# 97 "data/scripts/custom.pory"
	.2byte ITEM_LEVEL_BALL
# 98 "data/scripts/custom.pory"
	.2byte ITEM_LURE_BALL
# 99 "data/scripts/custom.pory"
	.2byte ITEM_MOON_BALL
# 100 "data/scripts/custom.pory"
	.2byte ITEM_FRIEND_BALL
# 101 "data/scripts/custom.pory"
	.2byte ITEM_LOVE_BALL
# 102 "data/scripts/custom.pory"
	.2byte ITEM_FAST_BALL
# 103 "data/scripts/custom.pory"
	.2byte ITEM_HEAVY_BALL
# 104 "data/scripts/custom.pory"
	.2byte ITEM_DREAM_BALL
# 105 "data/scripts/custom.pory"
	.2byte ITEM_SAFARI_BALL
# 106 "data/scripts/custom.pory"
	.2byte ITEM_SPORT_BALL
# 107 "data/scripts/custom.pory"
	.2byte ITEM_PARK_BALL
# 108 "data/scripts/custom.pory"
	.2byte ITEM_BEAST_BALL
# 109 "data/scripts/custom.pory"
	.2byte ITEM_CHERISH_BALL
	.2byte ITEM_NONE

	.align 2
# 112 "data/scripts/custom.pory"
Common_Shop_Evolution:
# 114 "data/scripts/custom.pory"
	.2byte ITEM_FIRE_STONE
# 115 "data/scripts/custom.pory"
	.2byte ITEM_WATER_STONE
# 116 "data/scripts/custom.pory"
	.2byte ITEM_THUNDER_STONE
# 117 "data/scripts/custom.pory"
	.2byte ITEM_LEAF_STONE
# 118 "data/scripts/custom.pory"
	.2byte ITEM_ICE_STONE
# 119 "data/scripts/custom.pory"
	.2byte ITEM_SUN_STONE
# 120 "data/scripts/custom.pory"
	.2byte ITEM_MOON_STONE
# 121 "data/scripts/custom.pory"
	.2byte ITEM_SHINY_STONE
# 122 "data/scripts/custom.pory"
	.2byte ITEM_DUSK_STONE
# 123 "data/scripts/custom.pory"
	.2byte ITEM_DAWN_STONE
# 124 "data/scripts/custom.pory"
	.2byte ITEM_LINKING_CORD
# 125 "data/scripts/custom.pory"
	.2byte ITEM_KINGS_ROCK
# 126 "data/scripts/custom.pory"
	.2byte ITEM_METAL_COAT
# 127 "data/scripts/custom.pory"
	.2byte ITEM_DRAGON_SCALE
# 128 "data/scripts/custom.pory"
	.2byte ITEM_UPGRADE
# 129 "data/scripts/custom.pory"
	.2byte ITEM_DEEP_SEA_SCALE
# 130 "data/scripts/custom.pory"
	.2byte ITEM_DEEP_SEA_TOOTH
# 131 "data/scripts/custom.pory"
	.2byte ITEM_OVAL_STONE
# 132 "data/scripts/custom.pory"
	.2byte ITEM_PROTECTOR
# 133 "data/scripts/custom.pory"
	.2byte ITEM_ELECTIRIZER
# 134 "data/scripts/custom.pory"
	.2byte ITEM_MAGMARIZER
# 135 "data/scripts/custom.pory"
	.2byte ITEM_DUBIOUS_DISC
# 136 "data/scripts/custom.pory"
	.2byte ITEM_REAPER_CLOTH
# 137 "data/scripts/custom.pory"
	.2byte ITEM_PRISM_SCALE
# 138 "data/scripts/custom.pory"
	.2byte ITEM_WHIPPED_DREAM
# 139 "data/scripts/custom.pory"
	.2byte ITEM_SACHET
# 140 "data/scripts/custom.pory"
	.2byte ITEM_SWEET_APPLE
# 141 "data/scripts/custom.pory"
	.2byte ITEM_TART_APPLE
# 142 "data/scripts/custom.pory"
	.2byte ITEM_SYRUPY_APPLE
# 143 "data/scripts/custom.pory"
	.2byte ITEM_CRACKED_POT
# 144 "data/scripts/custom.pory"
	.2byte ITEM_CHIPPED_POT
# 145 "data/scripts/custom.pory"
	.2byte ITEM_GALARICA_CUFF
# 146 "data/scripts/custom.pory"
	.2byte ITEM_GALARICA_WREATH
# 147 "data/scripts/custom.pory"
	.2byte ITEM_STRAWBERRY_SWEET
# 148 "data/scripts/custom.pory"
	.2byte ITEM_LOVE_SWEET
# 149 "data/scripts/custom.pory"
	.2byte ITEM_BERRY_SWEET
# 150 "data/scripts/custom.pory"
	.2byte ITEM_CLOVER_SWEET
# 151 "data/scripts/custom.pory"
	.2byte ITEM_FLOWER_SWEET
# 152 "data/scripts/custom.pory"
	.2byte ITEM_STAR_SWEET
# 153 "data/scripts/custom.pory"
	.2byte ITEM_RIBBON_SWEET
# 154 "data/scripts/custom.pory"
	.2byte ITEM_BLACK_AUGURITE
# 155 "data/scripts/custom.pory"
	.2byte ITEM_PEAT_BLOCK
# 156 "data/scripts/custom.pory"
	.2byte ITEM_SCROLL_OF_DARKNESS
# 157 "data/scripts/custom.pory"
	.2byte ITEM_SCROLL_OF_WATERS
# 158 "data/scripts/custom.pory"
	.2byte ITEM_AUSPICIOUS_ARMOR
# 159 "data/scripts/custom.pory"
	.2byte ITEM_MALICIOUS_ARMOR
# 160 "data/scripts/custom.pory"
	.2byte ITEM_GIMMIGHOUL_COIN
# 161 "data/scripts/custom.pory"
	.2byte ITEM_LEADERS_CREST
# 162 "data/scripts/custom.pory"
	.2byte ITEM_UNREMARKABLE_TEACUP
# 163 "data/scripts/custom.pory"
	.2byte ITEM_MASTERPIECE_TEACUP
# 164 "data/scripts/custom.pory"
	.2byte ITEM_METAL_ALLOY
	.2byte ITEM_NONE

	.align 2
# 167 "data/scripts/custom.pory"
Common_Shop_Spec:
# 169 "data/scripts/custom.pory"
	.2byte ITEM_ABILITY_CAPSULE
# 170 "data/scripts/custom.pory"
	.2byte ITEM_ABILITY_PATCH
# 171 "data/scripts/custom.pory"
	.2byte ITEM_LONELY_MINT
# 172 "data/scripts/custom.pory"
	.2byte ITEM_ADAMANT_MINT
# 173 "data/scripts/custom.pory"
	.2byte ITEM_NAUGHTY_MINT
# 174 "data/scripts/custom.pory"
	.2byte ITEM_BRAVE_MINT
# 175 "data/scripts/custom.pory"
	.2byte ITEM_BOLD_MINT
# 176 "data/scripts/custom.pory"
	.2byte ITEM_IMPISH_MINT
# 177 "data/scripts/custom.pory"
	.2byte ITEM_LAX_MINT
# 178 "data/scripts/custom.pory"
	.2byte ITEM_RELAXED_MINT
# 179 "data/scripts/custom.pory"
	.2byte ITEM_MODEST_MINT
# 180 "data/scripts/custom.pory"
	.2byte ITEM_MILD_MINT
# 181 "data/scripts/custom.pory"
	.2byte ITEM_RASH_MINT
# 182 "data/scripts/custom.pory"
	.2byte ITEM_QUIET_MINT
# 183 "data/scripts/custom.pory"
	.2byte ITEM_CALM_MINT
# 184 "data/scripts/custom.pory"
	.2byte ITEM_GENTLE_MINT
# 185 "data/scripts/custom.pory"
	.2byte ITEM_CAREFUL_MINT
# 186 "data/scripts/custom.pory"
	.2byte ITEM_SASSY_MINT
# 187 "data/scripts/custom.pory"
	.2byte ITEM_TIMID_MINT
# 188 "data/scripts/custom.pory"
	.2byte ITEM_HASTY_MINT
# 189 "data/scripts/custom.pory"
	.2byte ITEM_JOLLY_MINT
# 190 "data/scripts/custom.pory"
	.2byte ITEM_NAIVE_MINT
# 191 "data/scripts/custom.pory"
	.2byte ITEM_SERIOUS_MINT
	.2byte ITEM_NONE

	.align 2
# 194 "data/scripts/custom.pory"
Common_Shop_Levelup:
# 196 "data/scripts/custom.pory"
	.2byte ITEM_RARE_CANDY
# 197 "data/scripts/custom.pory"
	.2byte ITEM_EXP_CANDY_XS
# 198 "data/scripts/custom.pory"
	.2byte ITEM_EXP_CANDY_S
# 199 "data/scripts/custom.pory"
	.2byte ITEM_EXP_CANDY_M
# 200 "data/scripts/custom.pory"
	.2byte ITEM_EXP_CANDY_L
# 201 "data/scripts/custom.pory"
	.2byte ITEM_EXP_CANDY_XL
# 202 "data/scripts/custom.pory"
	.2byte ITEM_HP_UP
# 203 "data/scripts/custom.pory"
	.2byte ITEM_PROTEIN
# 204 "data/scripts/custom.pory"
	.2byte ITEM_IRON
# 205 "data/scripts/custom.pory"
	.2byte ITEM_CALCIUM
# 206 "data/scripts/custom.pory"
	.2byte ITEM_ZINC
# 207 "data/scripts/custom.pory"
	.2byte ITEM_CARBOS
# 208 "data/scripts/custom.pory"
	.2byte ITEM_HEALTH_FEATHER
# 209 "data/scripts/custom.pory"
	.2byte ITEM_MUSCLE_FEATHER
# 210 "data/scripts/custom.pory"
	.2byte ITEM_RESIST_FEATHER
# 211 "data/scripts/custom.pory"
	.2byte ITEM_GENIUS_FEATHER
# 212 "data/scripts/custom.pory"
	.2byte ITEM_CLEVER_FEATHER
# 213 "data/scripts/custom.pory"
	.2byte ITEM_SWIFT_FEATHER
# 214 "data/scripts/custom.pory"
	.2byte ITEM_PP_UP
# 215 "data/scripts/custom.pory"
	.2byte ITEM_PP_MAX
# 216 "data/scripts/custom.pory"
	.2byte ITEM_DYNAMAX_CANDY
	.2byte ITEM_NONE

	.align 2
# 219 "data/scripts/custom.pory"
Common_Shop_HeldItems:
# 221 "data/scripts/custom.pory"
	.2byte ITEM_SILK_SCARF
# 222 "data/scripts/custom.pory"
	.2byte ITEM_CHARCOAL
# 223 "data/scripts/custom.pory"
	.2byte ITEM_MYSTIC_WATER
# 224 "data/scripts/custom.pory"
	.2byte ITEM_MAGNET
# 225 "data/scripts/custom.pory"
	.2byte ITEM_MIRACLE_SEED
# 226 "data/scripts/custom.pory"
	.2byte ITEM_NEVER_MELT_ICE
# 227 "data/scripts/custom.pory"
	.2byte ITEM_BLACK_BELT
# 228 "data/scripts/custom.pory"
	.2byte ITEM_POISON_BARB
# 229 "data/scripts/custom.pory"
	.2byte ITEM_SOFT_SAND
# 230 "data/scripts/custom.pory"
	.2byte ITEM_SHARP_BEAK
# 231 "data/scripts/custom.pory"
	.2byte ITEM_TWISTED_SPOON
# 232 "data/scripts/custom.pory"
	.2byte ITEM_SILVER_POWDER
# 233 "data/scripts/custom.pory"
	.2byte ITEM_HARD_STONE
# 234 "data/scripts/custom.pory"
	.2byte ITEM_SPELL_TAG
# 235 "data/scripts/custom.pory"
	.2byte ITEM_DRAGON_FANG
# 236 "data/scripts/custom.pory"
	.2byte ITEM_BLACK_GLASSES
# 237 "data/scripts/custom.pory"
	.2byte ITEM_METAL_COAT
# 238 "data/scripts/custom.pory"
	.2byte ITEM_FAIRY_FEATHER
# 239 "data/scripts/custom.pory"
	.2byte ITEM_CHOICE_BAND
# 240 "data/scripts/custom.pory"
	.2byte ITEM_CHOICE_SPECS
# 241 "data/scripts/custom.pory"
	.2byte ITEM_CHOICE_SCARF
# 242 "data/scripts/custom.pory"
	.2byte ITEM_FLAME_ORB
# 243 "data/scripts/custom.pory"
	.2byte ITEM_TOXIC_ORB
# 244 "data/scripts/custom.pory"
	.2byte ITEM_DAMP_ROCK
# 245 "data/scripts/custom.pory"
	.2byte ITEM_HEAT_ROCK
# 246 "data/scripts/custom.pory"
	.2byte ITEM_SMOOTH_ROCK
# 247 "data/scripts/custom.pory"
	.2byte ITEM_ICY_ROCK
# 248 "data/scripts/custom.pory"
	.2byte ITEM_ELECTRIC_SEED
# 249 "data/scripts/custom.pory"
	.2byte ITEM_PSYCHIC_SEED
# 250 "data/scripts/custom.pory"
	.2byte ITEM_MISTY_SEED
# 251 "data/scripts/custom.pory"
	.2byte ITEM_GRASSY_SEED
# 252 "data/scripts/custom.pory"
	.2byte ITEM_ABSORB_BULB
# 253 "data/scripts/custom.pory"
	.2byte ITEM_CELL_BATTERY
# 254 "data/scripts/custom.pory"
	.2byte ITEM_LUMINOUS_MOSS
# 255 "data/scripts/custom.pory"
	.2byte ITEM_SNOWBALL
# 256 "data/scripts/custom.pory"
	.2byte ITEM_BRIGHT_POWDER
# 257 "data/scripts/custom.pory"
	.2byte ITEM_WHITE_HERB
# 258 "data/scripts/custom.pory"
	.2byte ITEM_EXP_SHARE
# 259 "data/scripts/custom.pory"
	.2byte ITEM_QUICK_CLAW
# 260 "data/scripts/custom.pory"
	.2byte ITEM_SOOTHE_BELL
# 261 "data/scripts/custom.pory"
	.2byte ITEM_MENTAL_HERB
# 262 "data/scripts/custom.pory"
	.2byte ITEM_KINGS_ROCK
# 263 "data/scripts/custom.pory"
	.2byte ITEM_AMULET_COIN
# 264 "data/scripts/custom.pory"
	.2byte ITEM_CLEANSE_TAG
# 265 "data/scripts/custom.pory"
	.2byte ITEM_SMOKE_BALL
# 266 "data/scripts/custom.pory"
	.2byte ITEM_FOCUS_BAND
# 267 "data/scripts/custom.pory"
	.2byte ITEM_LUCKY_EGG
# 268 "data/scripts/custom.pory"
	.2byte ITEM_SCOPE_LENS
# 269 "data/scripts/custom.pory"
	.2byte ITEM_LEFTOVERS
# 270 "data/scripts/custom.pory"
	.2byte ITEM_SHELL_BELL
# 271 "data/scripts/custom.pory"
	.2byte ITEM_WIDE_LENS
# 272 "data/scripts/custom.pory"
	.2byte ITEM_MUSCLE_BAND
# 273 "data/scripts/custom.pory"
	.2byte ITEM_WISE_GLASSES
# 274 "data/scripts/custom.pory"
	.2byte ITEM_EXPERT_BELT
# 275 "data/scripts/custom.pory"
	.2byte ITEM_LIGHT_CLAY
# 276 "data/scripts/custom.pory"
	.2byte ITEM_LIFE_ORB
# 277 "data/scripts/custom.pory"
	.2byte ITEM_POWER_HERB
# 278 "data/scripts/custom.pory"
	.2byte ITEM_FOCUS_SASH
# 279 "data/scripts/custom.pory"
	.2byte ITEM_ZOOM_LENS
# 280 "data/scripts/custom.pory"
	.2byte ITEM_METRONOME
# 281 "data/scripts/custom.pory"
	.2byte ITEM_IRON_BALL
# 282 "data/scripts/custom.pory"
	.2byte ITEM_LAGGING_TAIL
# 283 "data/scripts/custom.pory"
	.2byte ITEM_DESTINY_KNOT
# 284 "data/scripts/custom.pory"
	.2byte ITEM_BLACK_SLUDGE
# 285 "data/scripts/custom.pory"
	.2byte ITEM_GRIP_CLAW
# 286 "data/scripts/custom.pory"
	.2byte ITEM_STICKY_BARB
# 287 "data/scripts/custom.pory"
	.2byte ITEM_SHED_SHELL
# 288 "data/scripts/custom.pory"
	.2byte ITEM_BIG_ROOT
# 289 "data/scripts/custom.pory"
	.2byte ITEM_RAZOR_CLAW
# 290 "data/scripts/custom.pory"
	.2byte ITEM_RAZOR_FANG
# 291 "data/scripts/custom.pory"
	.2byte ITEM_EVIOLITE
# 292 "data/scripts/custom.pory"
	.2byte ITEM_FLOAT_STONE
# 293 "data/scripts/custom.pory"
	.2byte ITEM_ROCKY_HELMET
# 294 "data/scripts/custom.pory"
	.2byte ITEM_AIR_BALLOON
# 295 "data/scripts/custom.pory"
	.2byte ITEM_RED_CARD
# 296 "data/scripts/custom.pory"
	.2byte ITEM_RING_TARGET
# 297 "data/scripts/custom.pory"
	.2byte ITEM_BINDING_BAND
# 298 "data/scripts/custom.pory"
	.2byte ITEM_EJECT_BUTTON
# 299 "data/scripts/custom.pory"
	.2byte ITEM_WEAKNESS_POLICY
# 300 "data/scripts/custom.pory"
	.2byte ITEM_ASSAULT_VEST
# 301 "data/scripts/custom.pory"
	.2byte ITEM_SAFETY_GOGGLES
# 302 "data/scripts/custom.pory"
	.2byte ITEM_ADRENALINE_ORB
# 303 "data/scripts/custom.pory"
	.2byte ITEM_TERRAIN_EXTENDER
# 304 "data/scripts/custom.pory"
	.2byte ITEM_PROTECTIVE_PADS
# 305 "data/scripts/custom.pory"
	.2byte ITEM_THROAT_SPRAY
# 306 "data/scripts/custom.pory"
	.2byte ITEM_EJECT_PACK
# 307 "data/scripts/custom.pory"
	.2byte ITEM_HEAVY_DUTY_BOOTS
# 308 "data/scripts/custom.pory"
	.2byte ITEM_BLUNDER_POLICY
# 309 "data/scripts/custom.pory"
	.2byte ITEM_ROOM_SERVICE
# 310 "data/scripts/custom.pory"
	.2byte ITEM_UTILITY_UMBRELLA
# 311 "data/scripts/custom.pory"
	.2byte ITEM_ABILITY_SHIELD
# 312 "data/scripts/custom.pory"
	.2byte ITEM_CLEAR_AMULET
# 313 "data/scripts/custom.pory"
	.2byte ITEM_PUNCHING_GLOVE
# 314 "data/scripts/custom.pory"
	.2byte ITEM_COVERT_CLOAK
# 315 "data/scripts/custom.pory"
	.2byte ITEM_LOADED_DICE
# 316 "data/scripts/custom.pory"
	.2byte ITEM_BOOSTER_ENERGY
# 317 "data/scripts/custom.pory"
	.2byte ITEM_MIRROR_HERB
	.2byte ITEM_NONE

	.align 2
# 320 "data/scripts/custom.pory"
Common_Shop_Gimmick:
# 322 "data/scripts/custom.pory"
	.2byte ITEM_NORMAL_GEM
# 323 "data/scripts/custom.pory"
	.2byte ITEM_FIRE_GEM
# 324 "data/scripts/custom.pory"
	.2byte ITEM_WATER_GEM
# 325 "data/scripts/custom.pory"
	.2byte ITEM_ELECTRIC_GEM
# 326 "data/scripts/custom.pory"
	.2byte ITEM_GRASS_GEM
# 327 "data/scripts/custom.pory"
	.2byte ITEM_ICE_GEM
# 328 "data/scripts/custom.pory"
	.2byte ITEM_FIGHTING_GEM
# 329 "data/scripts/custom.pory"
	.2byte ITEM_POISON_GEM
# 330 "data/scripts/custom.pory"
	.2byte ITEM_GROUND_GEM
# 331 "data/scripts/custom.pory"
	.2byte ITEM_FLYING_GEM
# 332 "data/scripts/custom.pory"
	.2byte ITEM_PSYCHIC_GEM
# 333 "data/scripts/custom.pory"
	.2byte ITEM_BUG_GEM
# 334 "data/scripts/custom.pory"
	.2byte ITEM_ROCK_GEM
# 335 "data/scripts/custom.pory"
	.2byte ITEM_GHOST_GEM
# 336 "data/scripts/custom.pory"
	.2byte ITEM_DRAGON_GEM
# 337 "data/scripts/custom.pory"
	.2byte ITEM_DARK_GEM
# 338 "data/scripts/custom.pory"
	.2byte ITEM_STEEL_GEM
# 339 "data/scripts/custom.pory"
	.2byte ITEM_FAIRY_GEM
# 340 "data/scripts/custom.pory"
	.2byte ITEM_VENUSAURITE
# 341 "data/scripts/custom.pory"
	.2byte ITEM_CHARIZARDITE_X
# 342 "data/scripts/custom.pory"
	.2byte ITEM_CHARIZARDITE_Y
# 343 "data/scripts/custom.pory"
	.2byte ITEM_BLASTOISINITE
# 344 "data/scripts/custom.pory"
	.2byte ITEM_BEEDRILLITE
# 345 "data/scripts/custom.pory"
	.2byte ITEM_PIDGEOTITE
# 346 "data/scripts/custom.pory"
	.2byte ITEM_ALAKAZITE
# 347 "data/scripts/custom.pory"
	.2byte ITEM_SLOWBRONITE
# 348 "data/scripts/custom.pory"
	.2byte ITEM_GENGARITE
# 349 "data/scripts/custom.pory"
	.2byte ITEM_KANGASKHANITE
# 350 "data/scripts/custom.pory"
	.2byte ITEM_PINSIRITE
# 351 "data/scripts/custom.pory"
	.2byte ITEM_GYARADOSITE
# 352 "data/scripts/custom.pory"
	.2byte ITEM_AERODACTYLITE
# 353 "data/scripts/custom.pory"
	.2byte ITEM_MEWTWONITE_X
# 354 "data/scripts/custom.pory"
	.2byte ITEM_MEWTWONITE_Y
# 355 "data/scripts/custom.pory"
	.2byte ITEM_AMPHAROSITE
# 356 "data/scripts/custom.pory"
	.2byte ITEM_STEELIXITE
# 357 "data/scripts/custom.pory"
	.2byte ITEM_SCIZORITE
# 358 "data/scripts/custom.pory"
	.2byte ITEM_HERACRONITE
# 359 "data/scripts/custom.pory"
	.2byte ITEM_HOUNDOOMINITE
# 360 "data/scripts/custom.pory"
	.2byte ITEM_TYRANITARITE
# 361 "data/scripts/custom.pory"
	.2byte ITEM_SCEPTILITE
# 362 "data/scripts/custom.pory"
	.2byte ITEM_BLAZIKENITE
# 363 "data/scripts/custom.pory"
	.2byte ITEM_SWAMPERTITE
# 364 "data/scripts/custom.pory"
	.2byte ITEM_GARDEVOIRITE
# 365 "data/scripts/custom.pory"
	.2byte ITEM_SABLENITE
# 366 "data/scripts/custom.pory"
	.2byte ITEM_MAWILITE
# 367 "data/scripts/custom.pory"
	.2byte ITEM_AGGRONITE
# 368 "data/scripts/custom.pory"
	.2byte ITEM_MEDICHAMITE
# 369 "data/scripts/custom.pory"
	.2byte ITEM_MANECTITE
# 370 "data/scripts/custom.pory"
	.2byte ITEM_SHARPEDONITE
# 371 "data/scripts/custom.pory"
	.2byte ITEM_CAMERUPTITE
# 372 "data/scripts/custom.pory"
	.2byte ITEM_ALTARIANITE
# 373 "data/scripts/custom.pory"
	.2byte ITEM_BANETTITE
# 374 "data/scripts/custom.pory"
	.2byte ITEM_ABSOLITE
# 375 "data/scripts/custom.pory"
	.2byte ITEM_GLALITITE
# 376 "data/scripts/custom.pory"
	.2byte ITEM_SALAMENCITE
# 377 "data/scripts/custom.pory"
	.2byte ITEM_METAGROSSITE
# 378 "data/scripts/custom.pory"
	.2byte ITEM_LATIASITE
# 379 "data/scripts/custom.pory"
	.2byte ITEM_LATIOSITE
# 380 "data/scripts/custom.pory"
	.2byte ITEM_LOPUNNITE
# 381 "data/scripts/custom.pory"
	.2byte ITEM_GARCHOMPITE
# 382 "data/scripts/custom.pory"
	.2byte ITEM_LUCARIONITE
# 383 "data/scripts/custom.pory"
	.2byte ITEM_ABOMASITE
# 384 "data/scripts/custom.pory"
	.2byte ITEM_GALLADITE
# 385 "data/scripts/custom.pory"
	.2byte ITEM_AUDINITE
# 386 "data/scripts/custom.pory"
	.2byte ITEM_DIANCITE
# 387 "data/scripts/custom.pory"
	.2byte ITEM_RED_ORB
# 388 "data/scripts/custom.pory"
	.2byte ITEM_BLUE_ORB
# 389 "data/scripts/custom.pory"
	.2byte ITEM_NORMALIUM_Z
# 390 "data/scripts/custom.pory"
	.2byte ITEM_FIRIUM_Z
# 391 "data/scripts/custom.pory"
	.2byte ITEM_WATERIUM_Z
# 392 "data/scripts/custom.pory"
	.2byte ITEM_ELECTRIUM_Z
# 393 "data/scripts/custom.pory"
	.2byte ITEM_GRASSIUM_Z
# 394 "data/scripts/custom.pory"
	.2byte ITEM_ICIUM_Z
# 395 "data/scripts/custom.pory"
	.2byte ITEM_FIGHTINIUM_Z
# 396 "data/scripts/custom.pory"
	.2byte ITEM_POISONIUM_Z
# 397 "data/scripts/custom.pory"
	.2byte ITEM_GROUNDIUM_Z
# 398 "data/scripts/custom.pory"
	.2byte ITEM_FLYINIUM_Z
# 399 "data/scripts/custom.pory"
	.2byte ITEM_PSYCHIUM_Z
# 400 "data/scripts/custom.pory"
	.2byte ITEM_BUGINIUM_Z
# 401 "data/scripts/custom.pory"
	.2byte ITEM_ROCKIUM_Z
# 402 "data/scripts/custom.pory"
	.2byte ITEM_GHOSTIUM_Z
# 403 "data/scripts/custom.pory"
	.2byte ITEM_DRAGONIUM_Z
# 404 "data/scripts/custom.pory"
	.2byte ITEM_DARKINIUM_Z
# 405 "data/scripts/custom.pory"
	.2byte ITEM_STEELIUM_Z
# 406 "data/scripts/custom.pory"
	.2byte ITEM_FAIRIUM_Z
# 407 "data/scripts/custom.pory"
	.2byte ITEM_PIKANIUM_Z
# 408 "data/scripts/custom.pory"
	.2byte ITEM_EEVIUM_Z
# 409 "data/scripts/custom.pory"
	.2byte ITEM_SNORLIUM_Z
# 410 "data/scripts/custom.pory"
	.2byte ITEM_MEWNIUM_Z
# 411 "data/scripts/custom.pory"
	.2byte ITEM_DECIDIUM_Z
# 412 "data/scripts/custom.pory"
	.2byte ITEM_INCINIUM_Z
# 413 "data/scripts/custom.pory"
	.2byte ITEM_PRIMARIUM_Z
# 414 "data/scripts/custom.pory"
	.2byte ITEM_LYCANIUM_Z
# 415 "data/scripts/custom.pory"
	.2byte ITEM_MIMIKIUM_Z
# 416 "data/scripts/custom.pory"
	.2byte ITEM_KOMMONIUM_Z
# 417 "data/scripts/custom.pory"
	.2byte ITEM_TAPUNIUM_Z
# 418 "data/scripts/custom.pory"
	.2byte ITEM_SOLGANIUM_Z
# 419 "data/scripts/custom.pory"
	.2byte ITEM_LUNALIUM_Z
# 420 "data/scripts/custom.pory"
	.2byte ITEM_MARSHADIUM_Z
# 421 "data/scripts/custom.pory"
	.2byte ITEM_ALORAICHIUM_Z
# 422 "data/scripts/custom.pory"
	.2byte ITEM_PIKASHUNIUM_Z
# 423 "data/scripts/custom.pory"
	.2byte ITEM_ULTRANECROZIUM_Z
# 424 "data/scripts/custom.pory"
	.2byte ITEM_BUG_TERA_SHARD
# 425 "data/scripts/custom.pory"
	.2byte ITEM_DARK_TERA_SHARD
# 426 "data/scripts/custom.pory"
	.2byte ITEM_DRAGON_TERA_SHARD
# 427 "data/scripts/custom.pory"
	.2byte ITEM_ELECTRIC_TERA_SHARD
# 428 "data/scripts/custom.pory"
	.2byte ITEM_FAIRY_TERA_SHARD
# 429 "data/scripts/custom.pory"
	.2byte ITEM_FIGHTING_TERA_SHARD
# 430 "data/scripts/custom.pory"
	.2byte ITEM_FIRE_TERA_SHARD
# 431 "data/scripts/custom.pory"
	.2byte ITEM_FLYING_TERA_SHARD
# 432 "data/scripts/custom.pory"
	.2byte ITEM_GHOST_TERA_SHARD
# 433 "data/scripts/custom.pory"
	.2byte ITEM_GRASS_TERA_SHARD
# 434 "data/scripts/custom.pory"
	.2byte ITEM_GROUND_TERA_SHARD
# 435 "data/scripts/custom.pory"
	.2byte ITEM_ICE_TERA_SHARD
# 436 "data/scripts/custom.pory"
	.2byte ITEM_NORMAL_TERA_SHARD
# 437 "data/scripts/custom.pory"
	.2byte ITEM_POISON_TERA_SHARD
# 438 "data/scripts/custom.pory"
	.2byte ITEM_PSYCHIC_TERA_SHARD
# 439 "data/scripts/custom.pory"
	.2byte ITEM_ROCK_TERA_SHARD
# 440 "data/scripts/custom.pory"
	.2byte ITEM_STEEL_TERA_SHARD
# 441 "data/scripts/custom.pory"
	.2byte ITEM_WATER_TERA_SHARD
# 442 "data/scripts/custom.pory"
	.2byte ITEM_STELLAR_TERA_SHARD
	.2byte ITEM_NONE

	.align 2
# 445 "data/scripts/custom.pory"
Common_Shop_FormItems:
# 447 "data/scripts/custom.pory"
	.2byte ITEM_LIGHT_BALL
# 448 "data/scripts/custom.pory"
	.2byte ITEM_LEEK
# 449 "data/scripts/custom.pory"
	.2byte ITEM_THICK_CLUB
# 450 "data/scripts/custom.pory"
	.2byte ITEM_LUCKY_PUNCH
# 451 "data/scripts/custom.pory"
	.2byte ITEM_METAL_POWDER
# 452 "data/scripts/custom.pory"
	.2byte ITEM_QUICK_POWDER
# 453 "data/scripts/custom.pory"
	.2byte ITEM_DEEP_SEA_SCALE
# 454 "data/scripts/custom.pory"
	.2byte ITEM_DEEP_SEA_TOOTH
# 455 "data/scripts/custom.pory"
	.2byte ITEM_SOUL_DEW
# 456 "data/scripts/custom.pory"
	.2byte ITEM_METEORITE
# 457 "data/scripts/custom.pory"
	.2byte ITEM_ROTOM_CATALOG
# 458 "data/scripts/custom.pory"
	.2byte ITEM_ADAMANT_ORB
# 459 "data/scripts/custom.pory"
	.2byte ITEM_LUSTROUS_ORB
# 460 "data/scripts/custom.pory"
	.2byte ITEM_GRISEOUS_ORB
# 461 "data/scripts/custom.pory"
	.2byte ITEM_ADAMANT_CRYSTAL
# 462 "data/scripts/custom.pory"
	.2byte ITEM_LUSTROUS_GLOBE
# 463 "data/scripts/custom.pory"
	.2byte ITEM_GRISEOUS_CORE
# 464 "data/scripts/custom.pory"
	.2byte ITEM_GRACIDEA
# 465 "data/scripts/custom.pory"
	.2byte ITEM_FLAME_PLATE
# 466 "data/scripts/custom.pory"
	.2byte ITEM_SPLASH_PLATE
# 467 "data/scripts/custom.pory"
	.2byte ITEM_ZAP_PLATE
# 468 "data/scripts/custom.pory"
	.2byte ITEM_MEADOW_PLATE
# 469 "data/scripts/custom.pory"
	.2byte ITEM_ICICLE_PLATE
# 470 "data/scripts/custom.pory"
	.2byte ITEM_FIST_PLATE
# 471 "data/scripts/custom.pory"
	.2byte ITEM_TOXIC_PLATE
# 472 "data/scripts/custom.pory"
	.2byte ITEM_EARTH_PLATE
# 473 "data/scripts/custom.pory"
	.2byte ITEM_SKY_PLATE
# 474 "data/scripts/custom.pory"
	.2byte ITEM_MIND_PLATE
# 475 "data/scripts/custom.pory"
	.2byte ITEM_INSECT_PLATE
# 476 "data/scripts/custom.pory"
	.2byte ITEM_STONE_PLATE
# 477 "data/scripts/custom.pory"
	.2byte ITEM_SPOOKY_PLATE
# 478 "data/scripts/custom.pory"
	.2byte ITEM_DRACO_PLATE
# 479 "data/scripts/custom.pory"
	.2byte ITEM_DREAD_PLATE
# 480 "data/scripts/custom.pory"
	.2byte ITEM_IRON_PLATE
# 481 "data/scripts/custom.pory"
	.2byte ITEM_PIXIE_PLATE
# 482 "data/scripts/custom.pory"
	.2byte ITEM_REVEAL_GLASS
# 483 "data/scripts/custom.pory"
	.2byte ITEM_DNA_SPLICERS
# 484 "data/scripts/custom.pory"
	.2byte ITEM_DOUSE_DRIVE
# 485 "data/scripts/custom.pory"
	.2byte ITEM_SHOCK_DRIVE
# 486 "data/scripts/custom.pory"
	.2byte ITEM_BURN_DRIVE
# 487 "data/scripts/custom.pory"
	.2byte ITEM_CHILL_DRIVE
# 488 "data/scripts/custom.pory"
	.2byte ITEM_ZYGARDE_CUBE
# 489 "data/scripts/custom.pory"
	.2byte ITEM_PRISON_BOTTLE
# 490 "data/scripts/custom.pory"
	.2byte ITEM_RED_NECTAR
# 491 "data/scripts/custom.pory"
	.2byte ITEM_YELLOW_NECTAR
# 492 "data/scripts/custom.pory"
	.2byte ITEM_PINK_NECTAR
# 493 "data/scripts/custom.pory"
	.2byte ITEM_PURPLE_NECTAR
# 494 "data/scripts/custom.pory"
	.2byte ITEM_FIRE_MEMORY
# 495 "data/scripts/custom.pory"
	.2byte ITEM_WATER_MEMORY
# 496 "data/scripts/custom.pory"
	.2byte ITEM_ELECTRIC_MEMORY
# 497 "data/scripts/custom.pory"
	.2byte ITEM_GRASS_MEMORY
# 498 "data/scripts/custom.pory"
	.2byte ITEM_ICE_MEMORY
# 499 "data/scripts/custom.pory"
	.2byte ITEM_FIGHTING_MEMORY
# 500 "data/scripts/custom.pory"
	.2byte ITEM_POISON_MEMORY
# 501 "data/scripts/custom.pory"
	.2byte ITEM_GROUND_MEMORY
# 502 "data/scripts/custom.pory"
	.2byte ITEM_FLYING_MEMORY
# 503 "data/scripts/custom.pory"
	.2byte ITEM_PSYCHIC_MEMORY
# 504 "data/scripts/custom.pory"
	.2byte ITEM_BUG_MEMORY
# 505 "data/scripts/custom.pory"
	.2byte ITEM_ROCK_MEMORY
# 506 "data/scripts/custom.pory"
	.2byte ITEM_GHOST_MEMORY
# 507 "data/scripts/custom.pory"
	.2byte ITEM_DRAGON_MEMORY
# 508 "data/scripts/custom.pory"
	.2byte ITEM_DARK_MEMORY
# 509 "data/scripts/custom.pory"
	.2byte ITEM_STEEL_MEMORY
# 510 "data/scripts/custom.pory"
	.2byte ITEM_FAIRY_MEMORY
# 511 "data/scripts/custom.pory"
	.2byte ITEM_N_SOLARIZER
# 512 "data/scripts/custom.pory"
	.2byte ITEM_N_LUNARIZER
# 513 "data/scripts/custom.pory"
	.2byte ITEM_RUSTED_SWORD
# 514 "data/scripts/custom.pory"
	.2byte ITEM_RUSTED_SHIELD
# 515 "data/scripts/custom.pory"
	.2byte ITEM_REINS_OF_UNITY
# 516 "data/scripts/custom.pory"
	.2byte ITEM_WELLSPRING_MASK
# 517 "data/scripts/custom.pory"
	.2byte ITEM_HEARTHFLAME_MASK
# 518 "data/scripts/custom.pory"
	.2byte ITEM_CORNERSTONE_MASK
	.2byte ITEM_NONE

	.align 2
# 521 "data/scripts/custom.pory"
Common_Shop_Berry:
# 523 "data/scripts/custom.pory"
	.2byte ITEM_CHERI_BERRY
# 524 "data/scripts/custom.pory"
	.2byte ITEM_CHESTO_BERRY
# 525 "data/scripts/custom.pory"
	.2byte ITEM_PECHA_BERRY
# 526 "data/scripts/custom.pory"
	.2byte ITEM_RAWST_BERRY
# 527 "data/scripts/custom.pory"
	.2byte ITEM_ASPEAR_BERRY
# 528 "data/scripts/custom.pory"
	.2byte ITEM_LEPPA_BERRY
# 529 "data/scripts/custom.pory"
	.2byte ITEM_ORAN_BERRY
# 530 "data/scripts/custom.pory"
	.2byte ITEM_PERSIM_BERRY
# 531 "data/scripts/custom.pory"
	.2byte ITEM_LUM_BERRY
# 532 "data/scripts/custom.pory"
	.2byte ITEM_SITRUS_BERRY
# 533 "data/scripts/custom.pory"
	.2byte ITEM_FIGY_BERRY
# 534 "data/scripts/custom.pory"
	.2byte ITEM_WIKI_BERRY
# 535 "data/scripts/custom.pory"
	.2byte ITEM_MAGO_BERRY
# 536 "data/scripts/custom.pory"
	.2byte ITEM_AGUAV_BERRY
# 537 "data/scripts/custom.pory"
	.2byte ITEM_IAPAPA_BERRY
# 538 "data/scripts/custom.pory"
	.2byte ITEM_RAZZ_BERRY
# 539 "data/scripts/custom.pory"
	.2byte ITEM_BLUK_BERRY
# 540 "data/scripts/custom.pory"
	.2byte ITEM_NANAB_BERRY
# 541 "data/scripts/custom.pory"
	.2byte ITEM_WEPEAR_BERRY
# 542 "data/scripts/custom.pory"
	.2byte ITEM_PINAP_BERRY
# 543 "data/scripts/custom.pory"
	.2byte ITEM_POMEG_BERRY
# 544 "data/scripts/custom.pory"
	.2byte ITEM_KELPSY_BERRY
# 545 "data/scripts/custom.pory"
	.2byte ITEM_QUALOT_BERRY
# 546 "data/scripts/custom.pory"
	.2byte ITEM_HONDEW_BERRY
# 547 "data/scripts/custom.pory"
	.2byte ITEM_GREPA_BERRY
# 548 "data/scripts/custom.pory"
	.2byte ITEM_TAMATO_BERRY
# 549 "data/scripts/custom.pory"
	.2byte ITEM_CORNN_BERRY
# 550 "data/scripts/custom.pory"
	.2byte ITEM_MAGOST_BERRY
# 551 "data/scripts/custom.pory"
	.2byte ITEM_RABUTA_BERRY
# 552 "data/scripts/custom.pory"
	.2byte ITEM_NOMEL_BERRY
# 553 "data/scripts/custom.pory"
	.2byte ITEM_SPELON_BERRY
# 554 "data/scripts/custom.pory"
	.2byte ITEM_PAMTRE_BERRY
# 555 "data/scripts/custom.pory"
	.2byte ITEM_WATMEL_BERRY
# 556 "data/scripts/custom.pory"
	.2byte ITEM_DURIN_BERRY
# 557 "data/scripts/custom.pory"
	.2byte ITEM_BELUE_BERRY
# 558 "data/scripts/custom.pory"
	.2byte ITEM_CHILAN_BERRY
# 559 "data/scripts/custom.pory"
	.2byte ITEM_OCCA_BERRY
# 560 "data/scripts/custom.pory"
	.2byte ITEM_PASSHO_BERRY
# 561 "data/scripts/custom.pory"
	.2byte ITEM_WACAN_BERRY
# 562 "data/scripts/custom.pory"
	.2byte ITEM_RINDO_BERRY
# 563 "data/scripts/custom.pory"
	.2byte ITEM_YACHE_BERRY
# 564 "data/scripts/custom.pory"
	.2byte ITEM_CHOPLE_BERRY
# 565 "data/scripts/custom.pory"
	.2byte ITEM_KEBIA_BERRY
# 566 "data/scripts/custom.pory"
	.2byte ITEM_SHUCA_BERRY
# 567 "data/scripts/custom.pory"
	.2byte ITEM_COBA_BERRY
# 568 "data/scripts/custom.pory"
	.2byte ITEM_PAYAPA_BERRY
# 569 "data/scripts/custom.pory"
	.2byte ITEM_TANGA_BERRY
# 570 "data/scripts/custom.pory"
	.2byte ITEM_CHARTI_BERRY
# 571 "data/scripts/custom.pory"
	.2byte ITEM_KASIB_BERRY
# 572 "data/scripts/custom.pory"
	.2byte ITEM_HABAN_BERRY
# 573 "data/scripts/custom.pory"
	.2byte ITEM_COLBUR_BERRY
# 574 "data/scripts/custom.pory"
	.2byte ITEM_BABIRI_BERRY
# 575 "data/scripts/custom.pory"
	.2byte ITEM_ROSELI_BERRY
# 576 "data/scripts/custom.pory"
	.2byte ITEM_LIECHI_BERRY
# 577 "data/scripts/custom.pory"
	.2byte ITEM_GANLON_BERRY
# 578 "data/scripts/custom.pory"
	.2byte ITEM_SALAC_BERRY
# 579 "data/scripts/custom.pory"
	.2byte ITEM_PETAYA_BERRY
# 580 "data/scripts/custom.pory"
	.2byte ITEM_APICOT_BERRY
# 581 "data/scripts/custom.pory"
	.2byte ITEM_LANSAT_BERRY
# 582 "data/scripts/custom.pory"
	.2byte ITEM_STARF_BERRY
# 583 "data/scripts/custom.pory"
	.2byte ITEM_ENIGMA_BERRY
# 584 "data/scripts/custom.pory"
	.2byte ITEM_MICLE_BERRY
# 585 "data/scripts/custom.pory"
	.2byte ITEM_CUSTAP_BERRY
# 586 "data/scripts/custom.pory"
	.2byte ITEM_JABOCA_BERRY
# 587 "data/scripts/custom.pory"
	.2byte ITEM_ROWAP_BERRY
# 588 "data/scripts/custom.pory"
	.2byte ITEM_KEE_BERRY
# 589 "data/scripts/custom.pory"
	.2byte ITEM_MARANGA_BERRY
	.2byte ITEM_NONE

	.align 2
# 592 "data/scripts/custom.pory"
Common_Shop_TMs:
# 594 "data/scripts/custom.pory"
	.2byte ITEM_TM01
# 595 "data/scripts/custom.pory"
	.2byte ITEM_TM02
# 596 "data/scripts/custom.pory"
	.2byte ITEM_TM03
# 597 "data/scripts/custom.pory"
	.2byte ITEM_TM04
# 598 "data/scripts/custom.pory"
	.2byte ITEM_TM05
# 599 "data/scripts/custom.pory"
	.2byte ITEM_TM06
# 600 "data/scripts/custom.pory"
	.2byte ITEM_TM07
# 601 "data/scripts/custom.pory"
	.2byte ITEM_TM08
# 602 "data/scripts/custom.pory"
	.2byte ITEM_TM09
# 603 "data/scripts/custom.pory"
	.2byte ITEM_TM10
# 604 "data/scripts/custom.pory"
	.2byte ITEM_TM11
# 605 "data/scripts/custom.pory"
	.2byte ITEM_TM12
# 606 "data/scripts/custom.pory"
	.2byte ITEM_TM13
# 607 "data/scripts/custom.pory"
	.2byte ITEM_TM14
# 608 "data/scripts/custom.pory"
	.2byte ITEM_TM15
# 609 "data/scripts/custom.pory"
	.2byte ITEM_TM16
# 610 "data/scripts/custom.pory"
	.2byte ITEM_TM17
# 611 "data/scripts/custom.pory"
	.2byte ITEM_TM18
# 612 "data/scripts/custom.pory"
	.2byte ITEM_TM19
# 613 "data/scripts/custom.pory"
	.2byte ITEM_TM20
# 614 "data/scripts/custom.pory"
	.2byte ITEM_TM21
# 615 "data/scripts/custom.pory"
	.2byte ITEM_TM22
# 616 "data/scripts/custom.pory"
	.2byte ITEM_TM23
# 617 "data/scripts/custom.pory"
	.2byte ITEM_TM24
# 618 "data/scripts/custom.pory"
	.2byte ITEM_TM25
# 619 "data/scripts/custom.pory"
	.2byte ITEM_TM26
# 620 "data/scripts/custom.pory"
	.2byte ITEM_TM27
# 621 "data/scripts/custom.pory"
	.2byte ITEM_TM28
# 622 "data/scripts/custom.pory"
	.2byte ITEM_TM29
# 623 "data/scripts/custom.pory"
	.2byte ITEM_TM30
# 624 "data/scripts/custom.pory"
	.2byte ITEM_TM31
# 625 "data/scripts/custom.pory"
	.2byte ITEM_TM32
# 626 "data/scripts/custom.pory"
	.2byte ITEM_TM33
# 627 "data/scripts/custom.pory"
	.2byte ITEM_TM34
# 628 "data/scripts/custom.pory"
	.2byte ITEM_TM35
# 629 "data/scripts/custom.pory"
	.2byte ITEM_TM36
# 630 "data/scripts/custom.pory"
	.2byte ITEM_TM37
# 631 "data/scripts/custom.pory"
	.2byte ITEM_TM38
# 632 "data/scripts/custom.pory"
	.2byte ITEM_TM39
# 633 "data/scripts/custom.pory"
	.2byte ITEM_TM40
# 634 "data/scripts/custom.pory"
	.2byte ITEM_TM41
# 635 "data/scripts/custom.pory"
	.2byte ITEM_TM42
# 636 "data/scripts/custom.pory"
	.2byte ITEM_TM43
# 637 "data/scripts/custom.pory"
	.2byte ITEM_TM44
# 638 "data/scripts/custom.pory"
	.2byte ITEM_TM45
# 639 "data/scripts/custom.pory"
	.2byte ITEM_TM46
# 640 "data/scripts/custom.pory"
	.2byte ITEM_TM47
# 641 "data/scripts/custom.pory"
	.2byte ITEM_TM48
# 642 "data/scripts/custom.pory"
	.2byte ITEM_TM49
# 643 "data/scripts/custom.pory"
	.2byte ITEM_TM50
# 644 "data/scripts/custom.pory"
	.2byte ITEM_TM51
# 645 "data/scripts/custom.pory"
	.2byte ITEM_TM52
# 646 "data/scripts/custom.pory"
	.2byte ITEM_TM53
# 647 "data/scripts/custom.pory"
	.2byte ITEM_TM54
# 648 "data/scripts/custom.pory"
	.2byte ITEM_TM55
# 649 "data/scripts/custom.pory"
	.2byte ITEM_TM56
# 650 "data/scripts/custom.pory"
	.2byte ITEM_TM57
# 651 "data/scripts/custom.pory"
	.2byte ITEM_TM58
# 652 "data/scripts/custom.pory"
	.2byte ITEM_TM59
# 653 "data/scripts/custom.pory"
	.2byte ITEM_TM60
# 654 "data/scripts/custom.pory"
	.2byte ITEM_TM61
# 655 "data/scripts/custom.pory"
	.2byte ITEM_TM62
# 656 "data/scripts/custom.pory"
	.2byte ITEM_TM63
# 657 "data/scripts/custom.pory"
	.2byte ITEM_TM64
# 658 "data/scripts/custom.pory"
	.2byte ITEM_TM65
# 659 "data/scripts/custom.pory"
	.2byte ITEM_TM66
# 660 "data/scripts/custom.pory"
	.2byte ITEM_TM67
# 661 "data/scripts/custom.pory"
	.2byte ITEM_TM68
# 662 "data/scripts/custom.pory"
	.2byte ITEM_TM69
# 663 "data/scripts/custom.pory"
	.2byte ITEM_TM70
# 664 "data/scripts/custom.pory"
	.2byte ITEM_TM71
# 665 "data/scripts/custom.pory"
	.2byte ITEM_TM72
# 666 "data/scripts/custom.pory"
	.2byte ITEM_TM73
# 667 "data/scripts/custom.pory"
	.2byte ITEM_TM74
# 668 "data/scripts/custom.pory"
	.2byte ITEM_TM75
# 669 "data/scripts/custom.pory"
	.2byte ITEM_TM76
# 670 "data/scripts/custom.pory"
	.2byte ITEM_TM77
# 671 "data/scripts/custom.pory"
	.2byte ITEM_TM78
# 672 "data/scripts/custom.pory"
	.2byte ITEM_TM79
# 673 "data/scripts/custom.pory"
	.2byte ITEM_TM80
# 674 "data/scripts/custom.pory"
	.2byte ITEM_TM81
# 675 "data/scripts/custom.pory"
	.2byte ITEM_TM82
# 676 "data/scripts/custom.pory"
	.2byte ITEM_TM83
# 677 "data/scripts/custom.pory"
	.2byte ITEM_TM84
# 678 "data/scripts/custom.pory"
	.2byte ITEM_TM85
# 679 "data/scripts/custom.pory"
	.2byte ITEM_TM86
# 680 "data/scripts/custom.pory"
	.2byte ITEM_TM87
# 681 "data/scripts/custom.pory"
	.2byte ITEM_TM88
# 682 "data/scripts/custom.pory"
	.2byte ITEM_TM89
# 683 "data/scripts/custom.pory"
	.2byte ITEM_TM90
# 684 "data/scripts/custom.pory"
	.2byte ITEM_TM91
# 685 "data/scripts/custom.pory"
	.2byte ITEM_TM92
# 686 "data/scripts/custom.pory"
	.2byte ITEM_TM93
# 687 "data/scripts/custom.pory"
	.2byte ITEM_TM94
# 688 "data/scripts/custom.pory"
	.2byte ITEM_TM95
# 689 "data/scripts/custom.pory"
	.2byte ITEM_TM96
# 690 "data/scripts/custom.pory"
	.2byte ITEM_TM97
# 691 "data/scripts/custom.pory"
	.2byte ITEM_TM98
# 692 "data/scripts/custom.pory"
	.2byte ITEM_TM99
# 693 "data/scripts/custom.pory"
	.2byte ITEM_TM100
# 694 "data/scripts/custom.pory"
	.2byte ITEM_TM101
# 695 "data/scripts/custom.pory"
	.2byte ITEM_TM102
# 696 "data/scripts/custom.pory"
	.2byte ITEM_TM103
# 697 "data/scripts/custom.pory"
	.2byte ITEM_TM104
# 698 "data/scripts/custom.pory"
	.2byte ITEM_TM105
# 699 "data/scripts/custom.pory"
	.2byte ITEM_TM106
# 700 "data/scripts/custom.pory"
	.2byte ITEM_TM107
# 701 "data/scripts/custom.pory"
	.2byte ITEM_TM108
# 702 "data/scripts/custom.pory"
	.2byte ITEM_TM109
# 703 "data/scripts/custom.pory"
	.2byte ITEM_TM110
# 704 "data/scripts/custom.pory"
	.2byte ITEM_TM111
# 705 "data/scripts/custom.pory"
	.2byte ITEM_TM112
# 706 "data/scripts/custom.pory"
	.2byte ITEM_TM113
# 707 "data/scripts/custom.pory"
	.2byte ITEM_TM114
# 708 "data/scripts/custom.pory"
	.2byte ITEM_TM115
# 709 "data/scripts/custom.pory"
	.2byte ITEM_TM116
# 710 "data/scripts/custom.pory"
	.2byte ITEM_TM117
# 711 "data/scripts/custom.pory"
	.2byte ITEM_TM118
# 712 "data/scripts/custom.pory"
	.2byte ITEM_TM119
# 713 "data/scripts/custom.pory"
	.2byte ITEM_TM120
# 714 "data/scripts/custom.pory"
	.2byte ITEM_TM121
# 715 "data/scripts/custom.pory"
	.2byte ITEM_TM122
# 716 "data/scripts/custom.pory"
	.2byte ITEM_TM123
# 717 "data/scripts/custom.pory"
	.2byte ITEM_TM124
# 718 "data/scripts/custom.pory"
	.2byte ITEM_TM125
# 719 "data/scripts/custom.pory"
	.2byte ITEM_TM126
# 720 "data/scripts/custom.pory"
	.2byte ITEM_TM127
# 721 "data/scripts/custom.pory"
	.2byte ITEM_TM128
# 722 "data/scripts/custom.pory"
	.2byte ITEM_TM129
# 723 "data/scripts/custom.pory"
	.2byte ITEM_TM130
# 724 "data/scripts/custom.pory"
	.2byte ITEM_TM131
# 725 "data/scripts/custom.pory"
	.2byte ITEM_TM132
# 726 "data/scripts/custom.pory"
	.2byte ITEM_TM133
# 727 "data/scripts/custom.pory"
	.2byte ITEM_TM134
# 728 "data/scripts/custom.pory"
	.2byte ITEM_TM135
# 729 "data/scripts/custom.pory"
	.2byte ITEM_TM136
# 730 "data/scripts/custom.pory"
	.2byte ITEM_TM137
# 731 "data/scripts/custom.pory"
	.2byte ITEM_TM138
# 732 "data/scripts/custom.pory"
	.2byte ITEM_TM139
# 733 "data/scripts/custom.pory"
	.2byte ITEM_TM140
# 734 "data/scripts/custom.pory"
	.2byte ITEM_TM141
# 735 "data/scripts/custom.pory"
	.2byte ITEM_TM142
# 736 "data/scripts/custom.pory"
	.2byte ITEM_TM143
# 737 "data/scripts/custom.pory"
	.2byte ITEM_TM144
# 738 "data/scripts/custom.pory"
	.2byte ITEM_TM145
# 739 "data/scripts/custom.pory"
	.2byte ITEM_TM146
# 740 "data/scripts/custom.pory"
	.2byte ITEM_TM147
# 741 "data/scripts/custom.pory"
	.2byte ITEM_TM148
# 742 "data/scripts/custom.pory"
	.2byte ITEM_TM149
# 743 "data/scripts/custom.pory"
	.2byte ITEM_TM150
# 744 "data/scripts/custom.pory"
	.2byte ITEM_TM151
# 745 "data/scripts/custom.pory"
	.2byte ITEM_TM152
# 746 "data/scripts/custom.pory"
	.2byte ITEM_TM153
# 747 "data/scripts/custom.pory"
	.2byte ITEM_TM154
# 748 "data/scripts/custom.pory"
	.2byte ITEM_TM155
# 749 "data/scripts/custom.pory"
	.2byte ITEM_TM156
# 750 "data/scripts/custom.pory"
	.2byte ITEM_TM157
# 751 "data/scripts/custom.pory"
	.2byte ITEM_TM158
# 752 "data/scripts/custom.pory"
	.2byte ITEM_TM159
# 753 "data/scripts/custom.pory"
	.2byte ITEM_TM160
# 754 "data/scripts/custom.pory"
	.2byte ITEM_TM161
# 755 "data/scripts/custom.pory"
	.2byte ITEM_TM162
# 756 "data/scripts/custom.pory"
	.2byte ITEM_TM163
# 757 "data/scripts/custom.pory"
	.2byte ITEM_TM164
# 758 "data/scripts/custom.pory"
	.2byte ITEM_TM165
# 759 "data/scripts/custom.pory"
	.2byte ITEM_TM166
# 760 "data/scripts/custom.pory"
	.2byte ITEM_TM167
# 761 "data/scripts/custom.pory"
	.2byte ITEM_TM168
# 762 "data/scripts/custom.pory"
	.2byte ITEM_TM169
# 763 "data/scripts/custom.pory"
	.2byte ITEM_TM170
# 764 "data/scripts/custom.pory"
	.2byte ITEM_TM171
# 765 "data/scripts/custom.pory"
	.2byte ITEM_TM172
# 766 "data/scripts/custom.pory"
	.2byte ITEM_TM173
# 767 "data/scripts/custom.pory"
	.2byte ITEM_TM174
# 768 "data/scripts/custom.pory"
	.2byte ITEM_TM175
# 769 "data/scripts/custom.pory"
	.2byte ITEM_TM176
# 770 "data/scripts/custom.pory"
	.2byte ITEM_TM177
# 771 "data/scripts/custom.pory"
	.2byte ITEM_TM178
# 772 "data/scripts/custom.pory"
	.2byte ITEM_TM179
# 773 "data/scripts/custom.pory"
	.2byte ITEM_TM180
# 774 "data/scripts/custom.pory"
	.2byte ITEM_TM181
# 775 "data/scripts/custom.pory"
	.2byte ITEM_TM182
# 776 "data/scripts/custom.pory"
	.2byte ITEM_TM183
# 777 "data/scripts/custom.pory"
	.2byte ITEM_TM184
# 778 "data/scripts/custom.pory"
	.2byte ITEM_TM185
# 779 "data/scripts/custom.pory"
	.2byte ITEM_TM186
# 780 "data/scripts/custom.pory"
	.2byte ITEM_TM187
# 781 "data/scripts/custom.pory"
	.2byte ITEM_TM188
# 782 "data/scripts/custom.pory"
	.2byte ITEM_TM189
# 783 "data/scripts/custom.pory"
	.2byte ITEM_TM190
# 784 "data/scripts/custom.pory"
	.2byte ITEM_TM191
# 785 "data/scripts/custom.pory"
	.2byte ITEM_TM192
# 786 "data/scripts/custom.pory"
	.2byte ITEM_TM193
# 787 "data/scripts/custom.pory"
	.2byte ITEM_TM194
# 788 "data/scripts/custom.pory"
	.2byte ITEM_TM195
# 789 "data/scripts/custom.pory"
	.2byte ITEM_TM196
# 790 "data/scripts/custom.pory"
	.2byte ITEM_TM197
# 791 "data/scripts/custom.pory"
	.2byte ITEM_TM198
# 792 "data/scripts/custom.pory"
	.2byte ITEM_TM199
# 793 "data/scripts/custom.pory"
	.2byte ITEM_TM200
# 794 "data/scripts/custom.pory"
	.2byte ITEM_TM201
# 795 "data/scripts/custom.pory"
	.2byte ITEM_TM202
# 796 "data/scripts/custom.pory"
	.2byte ITEM_TM203
# 797 "data/scripts/custom.pory"
	.2byte ITEM_TM204
# 798 "data/scripts/custom.pory"
	.2byte ITEM_TM205
# 799 "data/scripts/custom.pory"
	.2byte ITEM_TM206
# 800 "data/scripts/custom.pory"
	.2byte ITEM_TM207
# 801 "data/scripts/custom.pory"
	.2byte ITEM_TM208
# 802 "data/scripts/custom.pory"
	.2byte ITEM_TM209
# 803 "data/scripts/custom.pory"
	.2byte ITEM_TM210
# 804 "data/scripts/custom.pory"
	.2byte ITEM_TM211
# 805 "data/scripts/custom.pory"
	.2byte ITEM_TM212
# 806 "data/scripts/custom.pory"
	.2byte ITEM_TM213
# 807 "data/scripts/custom.pory"
	.2byte ITEM_TM214
# 808 "data/scripts/custom.pory"
	.2byte ITEM_TM215
# 809 "data/scripts/custom.pory"
	.2byte ITEM_TM216
# 810 "data/scripts/custom.pory"
	.2byte ITEM_TM217
# 811 "data/scripts/custom.pory"
	.2byte ITEM_TM218
# 812 "data/scripts/custom.pory"
	.2byte ITEM_TM219
# 813 "data/scripts/custom.pory"
	.2byte ITEM_TM220
# 814 "data/scripts/custom.pory"
	.2byte ITEM_TM221
# 815 "data/scripts/custom.pory"
	.2byte ITEM_TM222
# 816 "data/scripts/custom.pory"
	.2byte ITEM_TM223
# 817 "data/scripts/custom.pory"
	.2byte ITEM_TM224
# 818 "data/scripts/custom.pory"
	.2byte ITEM_TM225
# 819 "data/scripts/custom.pory"
	.2byte ITEM_TM226
# 820 "data/scripts/custom.pory"
	.2byte ITEM_TM227
# 821 "data/scripts/custom.pory"
	.2byte ITEM_TM228
# 822 "data/scripts/custom.pory"
	.2byte ITEM_TM229
# 823 "data/scripts/custom.pory"
	.2byte ITEM_HM01
# 824 "data/scripts/custom.pory"
	.2byte ITEM_HM02
# 825 "data/scripts/custom.pory"
	.2byte ITEM_HM03
# 826 "data/scripts/custom.pory"
	.2byte ITEM_HM04
# 827 "data/scripts/custom.pory"
	.2byte ITEM_HM05
# 828 "data/scripts/custom.pory"
	.2byte ITEM_HM06
# 829 "data/scripts/custom.pory"
	.2byte ITEM_HM07
# 830 "data/scripts/custom.pory"
	.2byte ITEM_HM08
	.2byte ITEM_NONE

gText_UsePicnicBasket::
# 11 "data/scripts/custom.pory"
	.string "{PLAYER} had a picnic with the patry.\n"
	.string "All Pokémon were fully restored!$"

gText_ShopSpec::
# 70 "data/scripts/custom.pory"
	.string "Ability/Nature$"

gText_ShopLevelUp::
# 71 "data/scripts/custom.pory"
	.string "Level up$"

gText_ShopPokeball::
# 72 "data/scripts/custom.pory"
	.string "Poke ball$"

gText_ShopTMHMs::
# 73 "data/scripts/custom.pory"
	.string "TM / HMs$"

gText_ShopGimmick::
# 74 "data/scripts/custom.pory"
	.string "Gimmicks$"

gText_ShopHeldItems::
# 75 "data/scripts/custom.pory"
	.string "Held Items$"

gText_ShopEvolution::
# 76 "data/scripts/custom.pory"
	.string "Evolution$"

gText_ShopFormItems::
# 77 "data/scripts/custom.pory"
	.string "Form Items$"

gText_ShopBerry::
# 78 "data/scripts/custom.pory"
	.string "Berry$"

gText_ShopCansel::
# 79 "data/scripts/custom.pory"
	.string "   Cansel$"
