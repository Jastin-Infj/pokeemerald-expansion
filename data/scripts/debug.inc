# 1 "data/scripts/debug.pory"

# 2 "data/scripts/debug.pory"
.if DEBUG_OVERWORLD_MENU == TRUE
# 3 "data/scripts/debug.pory"
Debug_MessageEnd:
# 4 "data/scripts/debug.pory"
	waitmessage
# 5 "data/scripts/debug.pory"
	waitbuttonpress
# 6 "data/scripts/debug.pory"
	releaseall
# 7 "data/scripts/debug.pory"
	end
# 8 "data/scripts/debug.pory"

# 9 "data/scripts/debug.pory"
Debug_ShowFieldMessageStringVar4::
# 10 "data/scripts/debug.pory"
	special ShowFieldMessageStringVar4
# 11 "data/scripts/debug.pory"
	goto Debug_MessageEnd
# 12 "data/scripts/debug.pory"

# 13 "data/scripts/debug.pory"
Debug_CheatStart::
# 14 "data/scripts/debug.pory"
	setflag FLAG_SYS_POKEMON_GET
# 15 "data/scripts/debug.pory"
	setflag FLAG_RESCUED_BIRCH
# 16 "data/scripts/debug.pory"
	setflag FLAG_HIDE_ROUTE_101_BIRCH_ZIGZAGOON_BATTLE
# 17 "data/scripts/debug.pory"
	setflag FLAG_ADVENTURE_STARTED
# 18 "data/scripts/debug.pory"
	clearflag FLAG_HIDE_LITTLEROOT_TOWN_BIRCHS_LAB_BIRCH
# 19 "data/scripts/debug.pory"
	setflag FLAG_HIDE_ROUTE_101_BIRCH_STARTERS_BAG
# 20 "data/scripts/debug.pory"
	setvar VAR_BIRCH_LAB_STATE, 2
# 21 "data/scripts/debug.pory"
	setvar VAR_ROUTE101_STATE, 3
# 22 "data/scripts/debug.pory"
	givemon SPECIES_TREECKO, 20, ITEM_NONE
# 23 "data/scripts/debug.pory"
	givemon SPECIES_TORCHIC, 20, ITEM_NONE
# 24 "data/scripts/debug.pory"
	givemon SPECIES_MUDKIP, 20, ITEM_NONE
# 25 "data/scripts/debug.pory"
	setflag FLAG_SYS_POKEDEX_GET
# 26 "data/scripts/debug.pory"
	special SetUnlockedPokedexFlags
# 27 "data/scripts/debug.pory"
	setflag FLAG_RECEIVED_POKEDEX_FROM_BIRCH
# 28 "data/scripts/debug.pory"
	setvar VAR_CABLE_CLUB_TUTORIAL_STATE, 2
# 29 "data/scripts/debug.pory"
	setflag FLAG_SYS_NATIONAL_DEX
# 30 "data/scripts/debug.pory"
	special EnableNationalPokedex
# 31 "data/scripts/debug.pory"
	setflag FLAG_RECEIVED_RUNNING_SHOES
# 32 "data/scripts/debug.pory"
	setflag FLAG_SYS_B_DASH
# 33 "data/scripts/debug.pory"
	setvar VAR_LITTLEROOT_TOWN_STATE, 4 @ 4: Received Running Shoes
# 34 "data/scripts/debug.pory"
	setvar VAR_LITTLEROOT_INTRO_STATE, 7 @ 7: Told to go meet rival
# 35 "data/scripts/debug.pory"
	setvar VAR_LITTLEROOT_HOUSES_STATE_BRENDAN, 2 @ 2: Met Rival's Mom (and is corresponding gender)
# 36 "data/scripts/debug.pory"
	setvar VAR_LITTLEROOT_HOUSES_STATE_MAY, 2 @ 2: Met Rival's Mom (and is corresponding gender)
# 37 "data/scripts/debug.pory"
	setvar VAR_LITTLEROOT_RIVAL_STATE, 4 @ 4: Received Pokedex
# 38 "data/scripts/debug.pory"
	setflag FLAG_RECEIVED_BIKE
# 39 "data/scripts/debug.pory"
	additem ITEM_ACRO_BIKE
# 40 "data/scripts/debug.pory"
	setvar VAR_BRINEY_HOUSE_STATE, 1
# 41 "data/scripts/debug.pory"
	setvar VAR_ROUTE116_STATE, 2
# 42 "data/scripts/debug.pory"
	setflag FLAG_HIDE_ROUTE_116_MR_BRINEY
# 43 "data/scripts/debug.pory"
	setflag FLAG_BADGE01_GET
# 44 "data/scripts/debug.pory"
	setflag FLAG_BADGE02_GET
# 45 "data/scripts/debug.pory"
	setflag FLAG_BADGE03_GET
# 46 "data/scripts/debug.pory"
	setflag FLAG_BADGE04_GET
# 47 "data/scripts/debug.pory"
	setflag FLAG_BADGE05_GET
# 48 "data/scripts/debug.pory"
	setflag FLAG_BADGE06_GET
# 49 "data/scripts/debug.pory"
	setflag FLAG_BADGE07_GET
# 50 "data/scripts/debug.pory"
	setflag FLAG_BADGE08_GET
# 51 "data/scripts/debug.pory"
	setflag FLAG_VISITED_LITTLEROOT_TOWN
# 52 "data/scripts/debug.pory"
	setflag FLAG_VISITED_OLDALE_TOWN
# 53 "data/scripts/debug.pory"
	setflag FLAG_VISITED_DEWFORD_TOWN
# 54 "data/scripts/debug.pory"
	setflag FLAG_VISITED_LAVARIDGE_TOWN
# 55 "data/scripts/debug.pory"
	setflag FLAG_VISITED_FALLARBOR_TOWN
# 56 "data/scripts/debug.pory"
	setflag FLAG_VISITED_VERDANTURF_TOWN
# 57 "data/scripts/debug.pory"
	setflag FLAG_VISITED_PACIFIDLOG_TOWN
# 58 "data/scripts/debug.pory"
	setflag FLAG_VISITED_PETALBURG_CITY
# 59 "data/scripts/debug.pory"
	setflag FLAG_VISITED_SLATEPORT_CITY
# 60 "data/scripts/debug.pory"
	setflag FLAG_VISITED_MAUVILLE_CITY
# 61 "data/scripts/debug.pory"
	setflag FLAG_VISITED_RUSTBORO_CITY
# 62 "data/scripts/debug.pory"
	setflag FLAG_VISITED_FORTREE_CITY
# 63 "data/scripts/debug.pory"
	setflag FLAG_VISITED_LILYCOVE_CITY
# 64 "data/scripts/debug.pory"
	setflag FLAG_VISITED_MOSSDEEP_CITY
# 65 "data/scripts/debug.pory"
	setflag FLAG_VISITED_SOOTOPOLIS_CITY
# 66 "data/scripts/debug.pory"
	setflag FLAG_VISITED_EVER_GRANDE_CITY
# 67 "data/scripts/debug.pory"
	setflag FLAG_LANDMARK_POKEMON_LEAGUE
# 68 "data/scripts/debug.pory"
	setflag FLAG_LANDMARK_BATTLE_FRONTIER
# 69 "data/scripts/debug.pory"
	clearflag FLAG_HIDE_BRINEYS_HOUSE_MR_BRINEY
# 70 "data/scripts/debug.pory"
	clearflag FLAG_HIDE_BRINEYS_HOUSE_PEEKO
# 71 "data/scripts/debug.pory"
	release
# 72 "data/scripts/debug.pory"
	end
# 73 "data/scripts/debug.pory"

# 74 "data/scripts/debug.pory"
Debug_FlagsNotSetOverworldConfigMessage::
# 75 "data/scripts/debug.pory"
	message Debug_FlagsNotSetOverworldConfigMessage_Text
# 76 "data/scripts/debug.pory"
	goto Debug_MessageEnd
# 77 "data/scripts/debug.pory"

# 78 "data/scripts/debug.pory"
Debug_FlagsNotSetOverworldConfigMessage_Text:
# 79 "data/scripts/debug.pory"
	.string "Feature unavailable!\n"
# 80 "data/scripts/debug.pory"
	.string "Please define a usable flag in:\l"
# 81 "data/scripts/debug.pory"
	.string "'include/config/overworld.h'!$"
# 82 "data/scripts/debug.pory"

# 83 "data/scripts/debug.pory"
Debug_FlagsNotSetBattleConfigMessage::
# 84 "data/scripts/debug.pory"
	message Debug_FlagsNotSetBattleConfigMessage_Text
# 85 "data/scripts/debug.pory"
	goto Debug_MessageEnd
# 86 "data/scripts/debug.pory"

# 87 "data/scripts/debug.pory"
Debug_FlagsNotSetBattleConfigMessage_Text:
# 88 "data/scripts/debug.pory"
	.string "Feature unavailable!\n"
# 89 "data/scripts/debug.pory"
	.string "Please define a usable flag in:\l"
# 90 "data/scripts/debug.pory"
	.string "'include/config/battle.h'!$"
# 91 "data/scripts/debug.pory"

# 92 "data/scripts/debug.pory"
Debug_BoxFilledMessage::
# 93 "data/scripts/debug.pory"
	message Debug_BoxFilledMessage_Text
# 94 "data/scripts/debug.pory"
	goto Debug_MessageEnd
# 95 "data/scripts/debug.pory"

# 96 "data/scripts/debug.pory"
Debug_BoxFilledMessage_Text:
# 97 "data/scripts/debug.pory"
	.string "Storage boxes filled!$"
# 98 "data/scripts/debug.pory"

# 99 "data/scripts/debug.pory"
Debug_EventScript_CheckEVs::
# 100 "data/scripts/debug.pory"
	lockall
# 101 "data/scripts/debug.pory"
	getpartysize
# 102 "data/scripts/debug.pory"
	goto_if_eq VAR_RESULT, 0, Debug_NoPokemon
# 103 "data/scripts/debug.pory"
	special ChoosePartyMon
# 104 "data/scripts/debug.pory"
	waitstate
# 105 "data/scripts/debug.pory"
	goto_if_ge VAR_0x8004, PARTY_SIZE, Debug_EventScript_CheckEVs_End
# 106 "data/scripts/debug.pory"
	callnative Script_GetChosenMonOffensiveEVs
# 107 "data/scripts/debug.pory"
	msgbox Debug_EventScript_Text_OffensiveEVs, MSGBOX_DEFAULT
# 108 "data/scripts/debug.pory"
	callnative Script_GetChosenMonDefensiveEVs
# 109 "data/scripts/debug.pory"
	msgbox Debug_EventScript_Text_DefensiveEVs, MSGBOX_DEFAULT
# 110 "data/scripts/debug.pory"
Debug_EventScript_CheckEVs_End::
# 111 "data/scripts/debug.pory"
	releaseall
# 112 "data/scripts/debug.pory"
	end
# 113 "data/scripts/debug.pory"

# 114 "data/scripts/debug.pory"
Debug_EventScript_Text_OffensiveEVs:
# 115 "data/scripts/debug.pory"
	.string "ATK EVs: {STR_VAR_1}, SPATK EVs: {STR_VAR_2}, SPEED EVs: {STR_VAR_3}$"
# 116 "data/scripts/debug.pory"

# 117 "data/scripts/debug.pory"
Debug_EventScript_Text_DefensiveEVs:
# 118 "data/scripts/debug.pory"
	.string "HP EVs: {STR_VAR_1}, DEF EVs: {STR_VAR_2}, SPDEF EVs: {STR_VAR_3}$"
# 119 "data/scripts/debug.pory"

# 120 "data/scripts/debug.pory"
Debug_EventScript_CheckIVs::
# 121 "data/scripts/debug.pory"
	lockall
# 122 "data/scripts/debug.pory"
	getpartysize
# 123 "data/scripts/debug.pory"
	goto_if_eq VAR_RESULT, 0, Debug_NoPokemon
# 124 "data/scripts/debug.pory"
	special ChoosePartyMon
# 125 "data/scripts/debug.pory"
	waitstate
# 126 "data/scripts/debug.pory"
	goto_if_ge VAR_0x8004, PARTY_SIZE, Debug_EventScript_CheckIVs_End
# 127 "data/scripts/debug.pory"
	callnative Script_GetChosenMonOffensiveIVs
# 128 "data/scripts/debug.pory"
	msgbox Debug_EventScript_Text_OffensiveIVs, MSGBOX_DEFAULT
# 129 "data/scripts/debug.pory"
	callnative Script_GetChosenMonDefensiveIVs
# 130 "data/scripts/debug.pory"
	msgbox Debug_EventScript_Text_DefensiveIVs, MSGBOX_DEFAULT
# 131 "data/scripts/debug.pory"
Debug_EventScript_CheckIVs_End::
# 132 "data/scripts/debug.pory"
	releaseall
# 133 "data/scripts/debug.pory"
	end
# 134 "data/scripts/debug.pory"

# 135 "data/scripts/debug.pory"
Debug_EventScript_Text_OffensiveIVs:
# 136 "data/scripts/debug.pory"
	.string "ATK IVs: {STR_VAR_1}, SPATK IVs: {STR_VAR_2}, SPEED IVs: {STR_VAR_3}$"
# 137 "data/scripts/debug.pory"

# 138 "data/scripts/debug.pory"
Debug_EventScript_Text_DefensiveIVs:
# 139 "data/scripts/debug.pory"
	.string "HP IVs: {STR_VAR_1}, DEF IVs: {STR_VAR_2}, SPDEF IVs: {STR_VAR_3}$"
# 140 "data/scripts/debug.pory"

# 141 "data/scripts/debug.pory"
Debug_EventScript_Script_1::
# 142 "data/scripts/debug.pory"
	call FactoryTest
# 143 "data/scripts/debug.pory"
	release
# 144 "data/scripts/debug.pory"
	end
# 145 "data/scripts/debug.pory"

# 146 "data/scripts/debug.pory"
Debug_EventScript_Script_2::
# 147 "data/scripts/debug.pory"
	call TowerTest
# 148 "data/scripts/debug.pory"
	release
# 149 "data/scripts/debug.pory"
	end
# 150 "data/scripts/debug.pory"

# 151 "data/scripts/debug.pory"
Debug_EventScript_Script_3::
# 152 "data/scripts/debug.pory"
	release
# 153 "data/scripts/debug.pory"
	end
# 154 "data/scripts/debug.pory"

# 155 "data/scripts/debug.pory"
Debug_EventScript_Script_4::
# 156 "data/scripts/debug.pory"
	release
# 157 "data/scripts/debug.pory"
	end
# 158 "data/scripts/debug.pory"

# 159 "data/scripts/debug.pory"
Debug_EventScript_Script_5::
# 160 "data/scripts/debug.pory"
	release
# 161 "data/scripts/debug.pory"
	end
# 162 "data/scripts/debug.pory"

# 163 "data/scripts/debug.pory"
Debug_EventScript_Script_6::
# 164 "data/scripts/debug.pory"
	release
# 165 "data/scripts/debug.pory"
	end
# 166 "data/scripts/debug.pory"

# 167 "data/scripts/debug.pory"
Debug_EventScript_Script_7::
# 168 "data/scripts/debug.pory"
	release
# 169 "data/scripts/debug.pory"
	end
# 170 "data/scripts/debug.pory"

# 171 "data/scripts/debug.pory"
Debug_EventScript_Script_8::
# 172 "data/scripts/debug.pory"
	release
# 173 "data/scripts/debug.pory"
	end
# 174 "data/scripts/debug.pory"

# 175 "data/scripts/debug.pory"
Debug_CheckSaveBlock::
# 176 "data/scripts/debug.pory"
	callnative CheckSaveBlock1Size
# 177 "data/scripts/debug.pory"
	msgbox Debug_SaveBlock1Size, MSGBOX_DEFAULT
# 178 "data/scripts/debug.pory"
	callnative CheckSaveBlock2Size
# 179 "data/scripts/debug.pory"
	msgbox Debug_SaveBlock2Size, MSGBOX_DEFAULT
# 180 "data/scripts/debug.pory"
	callnative CheckSaveBlock3Size
# 181 "data/scripts/debug.pory"
	msgbox Debug_SaveBlock3Size, MSGBOX_DEFAULT
# 182 "data/scripts/debug.pory"
	callnative CheckPokemonStorageSize
# 183 "data/scripts/debug.pory"
	msgbox Debug_PokemonStorageSize, MSGBOX_DEFAULT
# 184 "data/scripts/debug.pory"
	release
# 185 "data/scripts/debug.pory"
	end
# 186 "data/scripts/debug.pory"

# 187 "data/scripts/debug.pory"
Debug_SaveBlock1Size::
# 188 "data/scripts/debug.pory"
	.string "SaveBlock1 size: {STR_VAR_1}b/{STR_VAR_2}b.\n"
# 189 "data/scripts/debug.pory"
	.string "Free space: {STR_VAR_3}b.$"
# 190 "data/scripts/debug.pory"

# 191 "data/scripts/debug.pory"
Debug_SaveBlock2Size::
# 192 "data/scripts/debug.pory"
	.string "SaveBlock2 size: {STR_VAR_1}b/{STR_VAR_2}b.\n"
# 193 "data/scripts/debug.pory"
	.string "Free space: {STR_VAR_3}b.$"
# 194 "data/scripts/debug.pory"

# 195 "data/scripts/debug.pory"
Debug_SaveBlock3Size::
# 196 "data/scripts/debug.pory"
	.string "SaveBlock3 size: {STR_VAR_1}b/{STR_VAR_2}b.\n"
# 197 "data/scripts/debug.pory"
	.string "Free space: {STR_VAR_3}b.$"
# 198 "data/scripts/debug.pory"

# 199 "data/scripts/debug.pory"
Debug_PokemonStorageSize::
# 200 "data/scripts/debug.pory"
	.string "{PKMN}Storage size: {STR_VAR_1}b/{STR_VAR_2}b.\n"
# 201 "data/scripts/debug.pory"
	.string "Free space: {STR_VAR_3}b.$"
# 202 "data/scripts/debug.pory"

# 203 "data/scripts/debug.pory"
Debug_CheckROMSpace::
# 204 "data/scripts/debug.pory"
	callnative CheckROMSize
# 205 "data/scripts/debug.pory"
	msgbox Debug_ROMSize, MSGBOX_DEFAULT
# 206 "data/scripts/debug.pory"
	release
# 207 "data/scripts/debug.pory"
	end
# 208 "data/scripts/debug.pory"

# 209 "data/scripts/debug.pory"
Debug_ROMSize::
# 210 "data/scripts/debug.pory"
	.string "ROM size: {STR_VAR_1}MB/32MB.\n"
# 211 "data/scripts/debug.pory"
	.string "Free space: {STR_VAR_2}MB.$"
# 212 "data/scripts/debug.pory"

# 213 "data/scripts/debug.pory"
Debug_HatchAnEgg::
# 214 "data/scripts/debug.pory"
	lockall
# 215 "data/scripts/debug.pory"
	getpartysize
# 216 "data/scripts/debug.pory"
	goto_if_eq VAR_RESULT, 0, Debug_NoPokemon
# 217 "data/scripts/debug.pory"
	special ChoosePartyMon
# 218 "data/scripts/debug.pory"
	waitstate
# 219 "data/scripts/debug.pory"
	goto_if_ge VAR_0x8004, PARTY_SIZE, Debug_HatchAnEgg_End
# 220 "data/scripts/debug.pory"
	specialvar VAR_RESULT, ScriptGetPartyMonSpecies
# 221 "data/scripts/debug.pory"
	goto_if_ne VAR_RESULT, SPECIES_EGG, DebugScript_HatchAnEgg_CantForceHatch
# 222 "data/scripts/debug.pory"
	special EggHatch
# 223 "data/scripts/debug.pory"
	waitstate
# 224 "data/scripts/debug.pory"
Debug_HatchAnEgg_End::
# 225 "data/scripts/debug.pory"
	releaseall
# 226 "data/scripts/debug.pory"
	end
# 227 "data/scripts/debug.pory"

# 228 "data/scripts/debug.pory"
Debug_NoPokemon::
# 229 "data/scripts/debug.pory"
	msgbox DebugScript_Text_EmptyParty, MSGBOX_DEFAULT
# 230 "data/scripts/debug.pory"
	releaseall
# 231 "data/scripts/debug.pory"
	end
# 232 "data/scripts/debug.pory"

# 233 "data/scripts/debug.pory"
DebugScript_HatchAnEgg_CantForceHatch::
# 234 "data/scripts/debug.pory"
	msgbox DebugScript_HatchAnEgg_Text_NotAnEgg, MSGBOX_DEFAULT
# 235 "data/scripts/debug.pory"
	releaseall
# 236 "data/scripts/debug.pory"
	end
# 237 "data/scripts/debug.pory"

# 238 "data/scripts/debug.pory"
DebugScript_Text_EmptyParty::
# 239 "data/scripts/debug.pory"
	.string "You have no Pokémon nor Eggs.$"
# 240 "data/scripts/debug.pory"

# 241 "data/scripts/debug.pory"
DebugScript_HatchAnEgg_Text_NotAnEgg::
# 242 "data/scripts/debug.pory"
	.string "That's not a Pokémon Egg.$"
# 243 "data/scripts/debug.pory"

# 244 "data/scripts/debug.pory"
DebugScript_ZeroDaycareMons::
# 245 "data/scripts/debug.pory"
	msgbox DebugText_DaycareNoPokemon, MSGBOX_DEFAULT
# 246 "data/scripts/debug.pory"
	releaseall
# 247 "data/scripts/debug.pory"
	end
# 248 "data/scripts/debug.pory"

# 249 "data/scripts/debug.pory"
DebugScript_OneDaycareMons::
# 250 "data/scripts/debug.pory"
	msgbox DebugText_DaycareOnePokemon, MSGBOX_DEFAULT
# 251 "data/scripts/debug.pory"
	releaseall
# 252 "data/scripts/debug.pory"
	end
# 253 "data/scripts/debug.pory"

# 254 "data/scripts/debug.pory"
DebugScript_DaycareMonsNotCompatible::
# 255 "data/scripts/debug.pory"
	msgbox DebugText_DaycarePokemonNotCompatible, MSGBOX_DEFAULT
# 256 "data/scripts/debug.pory"
	releaseall
# 257 "data/scripts/debug.pory"
	end
# 258 "data/scripts/debug.pory"

# 259 "data/scripts/debug.pory"
DebugText_DaycareNoPokemon:
# 260 "data/scripts/debug.pory"
	.string "You have no Pokémon at Daycare.$"
# 261 "data/scripts/debug.pory"

# 262 "data/scripts/debug.pory"
DebugText_DaycareOnePokemon:
# 263 "data/scripts/debug.pory"
	.string "You have only one Pokémon at Daycare.$"
# 264 "data/scripts/debug.pory"

# 265 "data/scripts/debug.pory"
DebugText_DaycarePokemonNotCompatible:
# 266 "data/scripts/debug.pory"
	.string "Your Pokémon at Daycare can't\nhave babies together!$"
# 267 "data/scripts/debug.pory"

# 268 "data/scripts/debug.pory"
Debug_ShowExpansionVersion::
# 269 "data/scripts/debug.pory"
	callnative BufferExpansionVersion
# 270 "data/scripts/debug.pory"
	msgbox Debug_ExpansionVersion, MSGBOX_DEFAULT
# 271 "data/scripts/debug.pory"
	release
# 272 "data/scripts/debug.pory"
	end
# 273 "data/scripts/debug.pory"

# 274 "data/scripts/debug.pory"
Debug_ExpansionVersion:
# 275 "data/scripts/debug.pory"
	.string "pokeemerald-expansion {STR_VAR_1}$"
# 276 "data/scripts/debug.pory"

# 277 "data/scripts/debug.pory"
Debug_BerryPestsDisabled::
# 278 "data/scripts/debug.pory"
	msgbox DebugText_BerryPestsDisabled, MSGBOX_DEFAULT
# 279 "data/scripts/debug.pory"
	release
# 280 "data/scripts/debug.pory"
	end
# 281 "data/scripts/debug.pory"

# 282 "data/scripts/debug.pory"
DebugText_BerryPestsDisabled:
# 283 "data/scripts/debug.pory"
	.string "OW_BERRY_PESTS is disabled.\n"
# 284 "data/scripts/debug.pory"
	.string "Unable to force pests onto berry trees.$"
# 285 "data/scripts/debug.pory"

# 286 "data/scripts/debug.pory"
Debug_BerryWeedsDisabled::
# 287 "data/scripts/debug.pory"
	msgbox DebugText_BerryWeedsDisabled, MSGBOX_DEFAULT
# 288 "data/scripts/debug.pory"
	release
# 289 "data/scripts/debug.pory"
	end
# 290 "data/scripts/debug.pory"

# 291 "data/scripts/debug.pory"
DebugText_BerryWeedsDisabled:
# 292 "data/scripts/debug.pory"
	.string "OW_BERRY_WEEDS is disabled.\n"
# 293 "data/scripts/debug.pory"
	.string "Unable to force weeds onto berry trees.$"
# 294 "data/scripts/debug.pory"

# 295 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1::
# 296 "data/scripts/debug.pory"
	lockall
# 297 "data/scripts/debug.pory"
	getpartysize
# 298 "data/scripts/debug.pory"
	goto_if_eq VAR_RESULT, 0, Debug_NoPokemon
# 299 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Single, 0
# 300 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_PartyWide, 1
# 301 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Close, 2
# 302 "data/scripts/debug.pory"
	dynmultistack 0, 0, FALSE, 3 FALSE, 0, NULL
# 303 "data/scripts/debug.pory"
	switch VAR_RESULT
# 304 "data/scripts/debug.pory"
	case 0, Debug_EventScript_InflictStatus1_Single
# 305 "data/scripts/debug.pory"
	case 1, Debug_EventScript_InflictStatus1_Party
# 306 "data/scripts/debug.pory"
	case 2, Debug_EventScript_InflictStatus1_Close
# 307 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Close:
# 308 "data/scripts/debug.pory"
	releaseall
# 309 "data/scripts/debug.pory"
	end
# 310 "data/scripts/debug.pory"

# 311 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Single:
# 312 "data/scripts/debug.pory"
	special ChoosePartyMon
# 313 "data/scripts/debug.pory"
	waitstate
# 314 "data/scripts/debug.pory"
	goto_if_ge VAR_0x8004, PARTY_SIZE, Debug_EventScript_InflictStatus1_Close
# 315 "data/scripts/debug.pory"
	specialvar VAR_RESULT, ScriptGetPartyMonSpecies
# 316 "data/scripts/debug.pory"
	goto_if_eq VAR_RESULT, SPECIES_EGG, Debug_EventScript_InflictStatus1_Close
# 317 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Poison, 0
# 318 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Paralysis, 1
# 319 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Sleep, 2
# 320 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Burn, 3
# 321 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Freeze, 4
# 322 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Frostbite, 5
# 323 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Close, 6
# 324 "data/scripts/debug.pory"
	dynmultistack 0, 0, FALSE, 7, FALSE, 0, NULL
# 325 "data/scripts/debug.pory"
	switch VAR_RESULT
# 326 "data/scripts/debug.pory"
	case 0, Debug_EventScript_InflictStatus1_Single_Poison
# 327 "data/scripts/debug.pory"
	case 1, Debug_EventScript_InflictStatus1_Single_Paralysis
# 328 "data/scripts/debug.pory"
	case 2, Debug_EventScript_InflictStatus1_Single_Sleep
# 329 "data/scripts/debug.pory"
	case 3, Debug_EventScript_InflictStatus1_Single_Burn
# 330 "data/scripts/debug.pory"
	case 4, Debug_EventScript_InflictStatus1_Single_Freeze
# 331 "data/scripts/debug.pory"
	case 5, Debug_EventScript_InflictStatus1_Single_Frostbite
# 332 "data/scripts/debug.pory"
	case 6, Debug_EventScript_InflictStatus1_Close
# 333 "data/scripts/debug.pory"
	case MULTI_B_PRESSED, Debug_EventScript_InflictStatus1_Close
# 334 "data/scripts/debug.pory"
	releaseall
# 335 "data/scripts/debug.pory"
	end
# 336 "data/scripts/debug.pory"

# 337 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Single_Poison:
# 338 "data/scripts/debug.pory"
	setstatus1 STATUS1_POISON, VAR_0x8004
# 339 "data/scripts/debug.pory"
	releaseall
# 340 "data/scripts/debug.pory"
	end
# 341 "data/scripts/debug.pory"

# 342 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Single_Paralysis:
# 343 "data/scripts/debug.pory"
	setstatus1 STATUS1_PARALYSIS, VAR_0x8004
# 344 "data/scripts/debug.pory"
	releaseall
# 345 "data/scripts/debug.pory"
	end
# 346 "data/scripts/debug.pory"

# 347 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Single_Sleep:
# 348 "data/scripts/debug.pory"
	setstatus1 STATUS1_SLEEP, VAR_0x8004
# 349 "data/scripts/debug.pory"
	releaseall
# 350 "data/scripts/debug.pory"
	end
# 351 "data/scripts/debug.pory"

# 352 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Single_Burn:
# 353 "data/scripts/debug.pory"
	setstatus1 STATUS1_BURN, VAR_0x8004
# 354 "data/scripts/debug.pory"
	releaseall
# 355 "data/scripts/debug.pory"
	end
# 356 "data/scripts/debug.pory"

# 357 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Single_Freeze:
# 358 "data/scripts/debug.pory"
	setstatus1 STATUS1_FREEZE, VAR_0x8004
# 359 "data/scripts/debug.pory"
	releaseall
# 360 "data/scripts/debug.pory"
	end
# 361 "data/scripts/debug.pory"

# 362 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Single_Frostbite:
# 363 "data/scripts/debug.pory"
	setstatus1 STATUS1_FROSTBITE, VAR_0x8004
# 364 "data/scripts/debug.pory"
	releaseall
# 365 "data/scripts/debug.pory"
	end
# 366 "data/scripts/debug.pory"

# 367 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Party:
# 368 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Poison, 0
# 369 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Paralysis, 1
# 370 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Sleep, 2
# 371 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Burn, 3
# 372 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Freeze, 4
# 373 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Frostbite, 5
# 374 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Close, 6
# 375 "data/scripts/debug.pory"
	dynmultistack 0, 0, FALSE, 7, FALSE, 0, NULL
# 376 "data/scripts/debug.pory"
	switch VAR_RESULT
# 377 "data/scripts/debug.pory"
	case 0, Debug_EventScript_InflictStatus1_Party_Poison
# 378 "data/scripts/debug.pory"
	case 1, Debug_EventScript_InflictStatus1_Party_Paralysis
# 379 "data/scripts/debug.pory"
	case 2, Debug_EventScript_InflictStatus1_Party_Sleep
# 380 "data/scripts/debug.pory"
	case 3, Debug_EventScript_InflictStatus1_Party_Burn
# 381 "data/scripts/debug.pory"
	case 4, Debug_EventScript_InflictStatus1_Party_Freeze
# 382 "data/scripts/debug.pory"
	case 5, Debug_EventScript_InflictStatus1_Party_Frostbite
# 383 "data/scripts/debug.pory"
	case 6, Debug_EventScript_InflictStatus1_Close
# 384 "data/scripts/debug.pory"
	case MULTI_B_PRESSED, Debug_EventScript_InflictStatus1_Close
# 385 "data/scripts/debug.pory"
	releaseall
# 386 "data/scripts/debug.pory"
	end
# 387 "data/scripts/debug.pory"

# 388 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Party_Poison:
# 389 "data/scripts/debug.pory"
	setstatus1 STATUS1_POISON, PARTY_SIZE
# 390 "data/scripts/debug.pory"
	releaseall
# 391 "data/scripts/debug.pory"
	end
# 392 "data/scripts/debug.pory"

# 393 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Party_Paralysis:
# 394 "data/scripts/debug.pory"
	setstatus1 STATUS1_PARALYSIS, PARTY_SIZE
# 395 "data/scripts/debug.pory"
	releaseall
# 396 "data/scripts/debug.pory"
	end
# 397 "data/scripts/debug.pory"

# 398 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Party_Sleep:
# 399 "data/scripts/debug.pory"
	setstatus1 STATUS1_SLEEP, PARTY_SIZE
# 400 "data/scripts/debug.pory"
	releaseall
# 401 "data/scripts/debug.pory"
	end
# 402 "data/scripts/debug.pory"

# 403 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Party_Burn:
# 404 "data/scripts/debug.pory"
	setstatus1 STATUS1_BURN, PARTY_SIZE
# 405 "data/scripts/debug.pory"
	releaseall
# 406 "data/scripts/debug.pory"
	end
# 407 "data/scripts/debug.pory"

# 408 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Party_Freeze:
# 409 "data/scripts/debug.pory"
	setstatus1 STATUS1_FREEZE, PARTY_SIZE
# 410 "data/scripts/debug.pory"
	releaseall
# 411 "data/scripts/debug.pory"
	end
# 412 "data/scripts/debug.pory"

# 413 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Party_Frostbite:
# 414 "data/scripts/debug.pory"
	setstatus1 STATUS1_FROSTBITE, PARTY_SIZE
# 415 "data/scripts/debug.pory"
	releaseall
# 416 "data/scripts/debug.pory"
	end
# 417 "data/scripts/debug.pory"

# 418 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_Single:
# 419 "data/scripts/debug.pory"
	.string "Single$"
# 420 "data/scripts/debug.pory"

# 421 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_PartyWide:
# 422 "data/scripts/debug.pory"
	.string "Party-wide$"
# 423 "data/scripts/debug.pory"

# 424 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_Close:
# 425 "data/scripts/debug.pory"
	.string "Close$"
# 426 "data/scripts/debug.pory"

# 427 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_Poison:
# 428 "data/scripts/debug.pory"
	.string "Poison$"
# 429 "data/scripts/debug.pory"

# 430 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_Paralysis:
# 431 "data/scripts/debug.pory"
	.string "Paralysis$"
# 432 "data/scripts/debug.pory"

# 433 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_Sleep:
# 434 "data/scripts/debug.pory"
	.string "Sleep$"
# 435 "data/scripts/debug.pory"

# 436 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_Burn:
# 437 "data/scripts/debug.pory"
	.string "Burn$"
# 438 "data/scripts/debug.pory"

# 439 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_Freeze:
# 440 "data/scripts/debug.pory"
	.string "Freeze$"
# 441 "data/scripts/debug.pory"

# 442 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_Frostbite:
# 443 "data/scripts/debug.pory"
	.string "Frostbite$"
# 444 "data/scripts/debug.pory"

# 445 "data/scripts/debug.pory"
Debug_EventScript_EWRAMCounters::
# 446 "data/scripts/debug.pory"
	callnative CheckEWRAMCounters
# 447 "data/scripts/debug.pory"
	msgbox Debug_EventScript_EWRAMCounters_Text, MSGBOX_DEFAULT
# 448 "data/scripts/debug.pory"
	release
# 449 "data/scripts/debug.pory"
	end
# 450 "data/scripts/debug.pory"

# 451 "data/scripts/debug.pory"
Debug_EventScript_EWRAMCounters_Text::
# 452 "data/scripts/debug.pory"
	.string "Follower Steps: {STR_VAR_1}.\n"
# 453 "data/scripts/debug.pory"
	.string "Fishing Chain: {STR_VAR_2}.$"
# 454 "data/scripts/debug.pory"

# 455 "data/scripts/debug.pory"
.endif

FactoryTest::
# 460 "data/scripts/debug.pory"
	setvar VAR_FRONTIER_FACILITY, FRONTIER_FACILITY_FACTORY
# 461 "data/scripts/debug.pory"
	setvar VAR_FRONTIER_BATTLE_MODE, FRONTIER_MODE_DOUBLES
# 462 "data/scripts/debug.pory"
	special SavePlayerParty
# 470 "data/scripts/debug.pory"
	waitmessage
# 472 "data/scripts/debug.pory"
	multichoice 17, 6, MULTI_CHALLENGEINFO, FALSE
# 474 "data/scripts/debug.pory"
	switch VAR_RESULT
# 475 "data/scripts/debug.pory"
	case 0, FactoryTest_3
# 477 "data/scripts/debug.pory"
	case 1, FactoryTest_4
# 479 "data/scripts/debug.pory"
	case 2, FactoryTest_5
# 482 "data/scripts/debug.pory"
	case MULTI_B_PRESSED, FactoryTest_6
FactoryTest_1:
# 488 "data/scripts/debug.pory"
	setvar VAR_0x8004, BATTLE_FACTORY_FUNC_INIT
# 489 "data/scripts/debug.pory"
	special CallBattleFactoryFunction
	return

FactoryTest_3:
# 476 "data/scripts/debug.pory"
	goto FactoryTest1
	goto FactoryTest_1

FactoryTest_4:
# 478 "data/scripts/debug.pory"
	goto BattleFrontier_BattleFactoryLobby_EventScript_ExplainChallenge
	goto FactoryTest_1

FactoryTest_5:
# 481 "data/scripts/debug.pory"
	goto BattleFrontier_BattleFactoryLobby_EventScript_CancelChallenge
	goto FactoryTest_1

FactoryTest_6:
# 484 "data/scripts/debug.pory"
	goto BattleFrontier_BattleFactoryLobby_EventScript_CancelChallenge
	goto FactoryTest_1


FactoryTest1::
# 495 "data/scripts/debug.pory"
	message BattleFrontier_BattleFactoryLobby_Text_WhichLevelMode
# 496 "data/scripts/debug.pory"
	waitmessage
# 502 "data/scripts/debug.pory"
	multichoice 17, 6, MULTI_LEVEL_MODE, FALSE
# 511 "data/scripts/debug.pory"
	frontier_set FRONTIER_DATA_LVL_MODE, VAR_RESULT
# 512 "data/scripts/debug.pory"
	msgbox BattleFrontier_BattleFactoryLobby_Text_OkayToSaveBeforeChallenge, MSGBOX_YESNO
# 514 "data/scripts/debug.pory"
	switch VAR_RESULT
# 515 "data/scripts/debug.pory"
	case NO, FactoryTest1_3
# 517 "data/scripts/debug.pory"
	case YES, FactoryTest1_4
# 519 "data/scripts/debug.pory"
	case MULTI_B_PRESSED, FactoryTest1_5
FactoryTest1_1:
# 522 "data/scripts/debug.pory"
	msgbox SearchVar
	return

FactoryTest1_3:
# 516 "data/scripts/debug.pory"
	goto BattleFrontier_BattleFactoryLobby_EventScript_LoadPartyAndCancelChallenge
	goto FactoryTest1_1

FactoryTest1_4:
# 518 "data/scripts/debug.pory"
	goto FactoryTest2
	goto FactoryTest1_1

FactoryTest1_5:
# 520 "data/scripts/debug.pory"
	goto BattleFrontier_BattleFactoryLobby_EventScript_LoadPartyAndCancelChallenge
	goto FactoryTest1_1


FactoryTest2::
# 526 "data/scripts/debug.pory"
	setvar VAR_TEMP_CHALLENGE_STATUS, 0
# 528 "data/scripts/debug.pory"
	factory_init
# 530 "data/scripts/debug.pory"
	frontier_set FRONTIER_DATA_CHALLENGE_STATUS, CHALLENGE_STATUS_SAVING
# 531 "data/scripts/debug.pory"
	factory_set FACTORY_DATA_WIN_STREAK_ACTIVE, TRUE
# 532 "data/scripts/debug.pory"
	frontier_set FRONTIER_DATA_PAUSED, FALSE
# 533 "data/scripts/debug.pory"
	closemessage
# 534 "data/scripts/debug.pory"
	delay 2
# 536 "data/scripts/debug.pory"
	call Common_EventScript_SaveGame
# 538 "data/scripts/debug.pory"
	setvar VAR_TEMP_CHALLENGE_STATUS, 255
# 541 "data/scripts/debug.pory"
	compare VAR_RESULT, 0
	goto_if_eq FactoryTest2_2
FactoryTest2_1:
# 545 "data/scripts/debug.pory"
	setvar VAR_0x8006, 0
# 548 "data/scripts/debug.pory"
	special SavePlayerParty
# 550 "data/scripts/debug.pory"
	closemessage
# 556 "data/scripts/debug.pory"
	waitmovement 0
# 557 "data/scripts/debug.pory"
	closemessage
# 560 "data/scripts/debug.pory"
	setvar VAR_TEMP_CHALLENGE_STATUS, 0
# 561 "data/scripts/debug.pory"
	goto FactoryTest3
	end

FactoryTest2_2:
# 542 "data/scripts/debug.pory"
	goto BattleFrontier_BattleFactoryLobby_EventScript_CancelChallengeSaveFailed
	goto FactoryTest2_1


FactoryTest3::
# 569 "data/scripts/debug.pory"
	compare VAR_TEMP_0, 0
	goto_if_eq FactoryTest3_1
	return

FactoryTest3_1:
# 570 "data/scripts/debug.pory"
	compare VAR_0x8006, 1
	goto_if_eq FactoryTest3_4
FactoryTest3_3:
# 574 "data/scripts/debug.pory"
	setvar VAR_TEMP_0, 1
# 580 "data/scripts/debug.pory"
	compare VAR_0x8006, 2
	goto_if_eq FactoryTest3_7
FactoryTest3_6:
# 585 "data/scripts/debug.pory"
	factory_generaterentalmons
# 588 "data/scripts/debug.pory"
	setvar VAR_0x8004, BATTLE_FACTORY_FUNC_GENERATE_OPPONENT_MONS
# 589 "data/scripts/debug.pory"
	special CallBattleFactoryFunction
# 606 "data/scripts/debug.pory"
	fadescreen FADE_TO_BLACK
# 607 "data/scripts/debug.pory"
	factory_setswapped
# 611 "data/scripts/debug.pory"
	setvar VAR_0x8004, BATTLE_FACTORY_FUNC_SELECT_RENT_MONS
# 612 "data/scripts/debug.pory"
	special CallBattleFactoryFunction
# 614 "data/scripts/debug.pory"
	waitstate
# 630 "data/scripts/debug.pory"
	goto FactoryTest4
# 631 "data/scripts/debug.pory"
	waitstate
	end

FactoryTest3_4:
# 571 "data/scripts/debug.pory"
	goto BattleFrontier_BattleFactoryPreBattleRoom_EventScript_ReturnToRoomFromBattle
	goto FactoryTest3_3

FactoryTest3_7:
# 581 "data/scripts/debug.pory"
	goto BattleFrontier_BattleFactoryPreBattleRoom_EventScript_ResumeChallenge
	goto FactoryTest3_6


FactoryTest4::
# 638 "data/scripts/debug.pory"
	compare VAR_TEMP_F, FRONTIER_BRAIN_NOT_READY
	goto_if_ne FactoryTest4_2
FactoryTest4_1:
# 648 "data/scripts/debug.pory"
	call BattleFrontier_BattleFactoryBattleRoom_EventScript_ScientistsFaceBattle
# 649 "data/scripts/debug.pory"
	factory_setopponentgfx
# 650 "data/scripts/debug.pory"
	removeobject LOCALID_OPPONENT
# 652 "data/scripts/debug.pory"
	setobjectxyperm LOCALID_OPPONENT, 7, 1
# 653 "data/scripts/debug.pory"
	addobject LOCALID_OPPONENT
# 654 "data/scripts/debug.pory"
	applymovement LOCALID_OPPONENT, BattleFrontier_BattleFactoryBattleRoom_Movement_OpponentEnter
# 655 "data/scripts/debug.pory"
	waitmovement 0
# 658 "data/scripts/debug.pory"
	compare VAR_TEMP_F, FRONTIER_BRAIN_NOT_READY
	goto_if_ne FactoryTest4_5
FactoryTest4_4:
# 661 "data/scripts/debug.pory"
	palace_getopponentintro
# 662 "data/scripts/debug.pory"
	lockall
# 670 "data/scripts/debug.pory"
	frontier_set FRONTIER_DATA_RECORD_DISABLED, FALSE
# 671 "data/scripts/debug.pory"
	special HealPlayerParty
# 673 "data/scripts/debug.pory"
	setvar VAR_0x8004, SPECIAL_BATTLE_FACTORY
# 674 "data/scripts/debug.pory"
	setvar VAR_0x8005, 0
# 676 "data/scripts/debug.pory"
	special DoSpecialTrainerBattle
# 677 "data/scripts/debug.pory"
	waitstate
# 680 "data/scripts/debug.pory"
	switch VAR_RESULT
# 681 "data/scripts/debug.pory"
	case 1, FactoryTest4_9
FactoryTest4_7:
	end

FactoryTest4_2:
# 639 "data/scripts/debug.pory"
	goto BattleFrontier_BattleFactoryBattleRoom_EventScript_EnterRoomFactoryHeadBattle
	goto FactoryTest4_1

FactoryTest4_5:
# 659 "data/scripts/debug.pory"
	goto BattleFrontier_BattleFactoryBattleRoom_EventScript_BattleNoland
	goto FactoryTest4_4

FactoryTest4_9:
# 682 "data/scripts/debug.pory"
	goto BattleFrontier_BattleFactoryBattleRoom_EventScript_DefeatedOpponent
	goto FactoryTest4_7


TowerTest::
# 694 "data/scripts/debug.pory"
	lock
# 695 "data/scripts/debug.pory"
	faceplayer
# 696 "data/scripts/debug.pory"
	setvar VAR_FRONTIER_FACILITY, FRONTIER_FACILITY_TOWER
# 698 "data/scripts/debug.pory"
	special SavePlayerParty
# 699 "data/scripts/debug.pory"
	msgbox BattleFrontier_BattleTowerLobby_Text_WelcomeDoubleBattle, MSGBOX_DEFAULT
# 701 "data/scripts/debug.pory"
	message BattleFrontier_BattleTowerLobby_Text_TakeDoublesChallenge
# 702 "data/scripts/debug.pory"
	waitmessage
# 703 "data/scripts/debug.pory"
	multichoice 17, 6, MULTI_CHALLENGEINFO, FALSE
# 706 "data/scripts/debug.pory"
	switch VAR_RESULT
# 707 "data/scripts/debug.pory"
	case 0, TowerTest_2
# 709 "data/scripts/debug.pory"
	case 1, TowerTest_3
# 711 "data/scripts/debug.pory"
	case 2, TowerTest_4
# 713 "data/scripts/debug.pory"
	case MULTI_B_PRESSED, TowerTest_5
	return

TowerTest_2:
# 708 "data/scripts/debug.pory"
	goto TowerTest1
	return

TowerTest_3:
# 710 "data/scripts/debug.pory"
	goto BattleFrontier_BattleTowerLobby_EventScript_ExplainDoublesChallenge
	return

TowerTest_4:
# 712 "data/scripts/debug.pory"
	goto BattleFrontier_BattleTowerLobby_EventScript_CancelChallenge
	return

TowerTest_5:
# 714 "data/scripts/debug.pory"
	goto BattleFrontier_BattleTowerLobby_EventScript_CancelChallenge
	return


TowerTest1::
# 721 "data/scripts/debug.pory"
	setvar VAR_FRONTIER_BATTLE_MODE, FRONTIER_MODE_DOUBLES
# 722 "data/scripts/debug.pory"
	message BattleFrontier_BattleTowerLobby_Text_WhichLevelMode
# 723 "data/scripts/debug.pory"
	waitmessage
# 724 "data/scripts/debug.pory"
	multichoice 17, 6, MULTI_LEVEL_MODE, FALSE
# 725 "data/scripts/debug.pory"
	switch VAR_RESULT
# 726 "data/scripts/debug.pory"
	case FRONTIER_LVL_TENT, TowerTest1_3
# 728 "data/scripts/debug.pory"
	case MULTI_B_PRESSED, TowerTest1_4
TowerTest1_1:
# 733 "data/scripts/debug.pory"
	frontier_checkineligible
# 735 "data/scripts/debug.pory"
	compare VAR_0x8004, TRUE
	goto_if_eq TowerTest1_6
TowerTest1_5:
# 738 "data/scripts/debug.pory"
	frontier_set FRONTIER_DATA_LVL_MODE, VAR_RESULT
# 740 "data/scripts/debug.pory"
	msgbox BattleFrontier_BattleTowerLobby_Text_PleaseSelectFourMons, MSGBOX_DEFAULT
# 742 "data/scripts/debug.pory"
	fadescreen FADE_TO_BLACK
# 744 "data/scripts/debug.pory"
	callnative RemoveFollowingPokemon
# 746 "data/scripts/debug.pory"
	call BattleFrontier_EventScript_GetLvlMode
# 747 "data/scripts/debug.pory"
	copyvar VAR_0x8004, VAR_RESULT
# 749 "data/scripts/debug.pory"
	setvar VAR_0x8005, FRONTIER_DOUBLES_PARTY_SIZE
# 751 "data/scripts/debug.pory"
	special ChoosePartyForBattleFrontier
# 752 "data/scripts/debug.pory"
	waitstate
# 755 "data/scripts/debug.pory"
	compare VAR_RESULT, 0
	goto_if_eq TowerTest1_9
TowerTest1_8:
# 760 "data/scripts/debug.pory"
	msgbox BattleFrontier_BattleTowerLobby_Text_OkayToSaveBeforeEntering, MSGBOX_YESNO
# 761 "data/scripts/debug.pory"
	switch VAR_RESULT
# 762 "data/scripts/debug.pory"
	case NO, TowerTest1_12
# 764 "data/scripts/debug.pory"
	case YES, TowerTest1_13
# 766 "data/scripts/debug.pory"
	case MULTI_B_PRESSED, TowerTest1_14
	return

TowerTest1_3:
# 727 "data/scripts/debug.pory"
	goto BattleFrontier_BattleTowerLobby_EventScript_CancelChallenge
	goto TowerTest1_1

TowerTest1_4:
# 729 "data/scripts/debug.pory"
	goto BattleFrontier_BattleTowerLobby_EventScript_CancelChallenge
	goto TowerTest1_1

TowerTest1_6:
# 736 "data/scripts/debug.pory"
	goto BattleFrontier_BattleTowerLobby_EventScript_NotEnoughValidMons
	goto TowerTest1_5

TowerTest1_9:
# 756 "data/scripts/debug.pory"
	goto BattleFrontier_BattleTowerLobby_EventScript_LoadPartyCancelChallenge
	goto TowerTest1_8

TowerTest1_12:
# 763 "data/scripts/debug.pory"
	goto BattleFrontier_BattleTowerLobby_EventScript_LoadPartyCancelChallenge
	return

TowerTest1_13:
# 765 "data/scripts/debug.pory"
	goto TowerTest2
	return

TowerTest1_14:
# 767 "data/scripts/debug.pory"
	goto BattleFrontier_BattleTowerLobby_EventScript_LoadPartyCancelChallenge
	return


TowerTest2::
# 772 "data/scripts/debug.pory"
	frontier_set FRONTIER_DATA_SELECTED_MON_ORDER
# 773 "data/scripts/debug.pory"
	setvar VAR_TEMP_CHALLENGE_STATUS, 0
# 776 "data/scripts/debug.pory"
	tower_init
# 777 "data/scripts/debug.pory"
	tower_set TOWER_DATA_WIN_STREAK_ACTIVE, TRUE
# 778 "data/scripts/debug.pory"
	frontier_set FRONTIER_DATA_PAUSED, FALSE
# 781 "data/scripts/debug.pory"
	special LoadPlayerParty
# 782 "data/scripts/debug.pory"
	closemessage
# 784 "data/scripts/debug.pory"
	delay_2
# 785 "data/scripts/debug.pory"
	call Common_EventScript_SaveGame
# 788 "data/scripts/debug.pory"
	setvar VAR_TEMP_CHALLENGE_STATUS, 255
# 789 "data/scripts/debug.pory"
	compare VAR_RESULT, 0
	goto_if_eq TowerTest2_2
TowerTest2_1:
# 792 "data/scripts/debug.pory"
	incrementgamestat GAME_STAT_ENTERED_BATTLE_TOWER
# 793 "data/scripts/debug.pory"
	setvar VAR_BRAVO_TRAINER_BATTLE_TOWER_ON, FALSE
# 794 "data/scripts/debug.pory"
	goto TowerTest3
	return

TowerTest2_2:
# 790 "data/scripts/debug.pory"
	goto BattleFrontier_BattleTowerLobby_EventScript_CancelChallengeSaveFailed
	goto TowerTest2_1


TowerTest3::
# 798 "data/scripts/debug.pory"
	special SavePlayerParty
# 799 "data/scripts/debug.pory"
	setvar VAR_0x8004, FRONTIER_UTIL_FUNC_SET_PARTY_ORDER
# 800 "data/scripts/debug.pory"
	call BattleFrontier_BattleTowerLobby_EventScript_GetPartySize
# 801 "data/scripts/debug.pory"
	special CallFrontierUtilFunc
# 803 "data/scripts/debug.pory"
	setvar VAR_RESULT, 0
# 805 "data/scripts/debug.pory"
	tower_loadlinkopponents
# 806 "data/scripts/debug.pory"
	delay 1
# 807 "data/scripts/debug.pory"
	compare VAR_RESULT, 6
	goto_if_eq TowerTest3_2
TowerTest3_1:
# 810 "data/scripts/debug.pory"
	call BattleFrontier_BattleTowerLobby_EventScript_ShowYouToBattleRoom
# 811 "data/scripts/debug.pory"
	clearflag FLAG_CANCEL_BATTLE_ROOM_CHALLENGE
# 814 "data/scripts/debug.pory"
	warp MAP_BATTLE_FRONTIER_BATTLE_TOWER_ELEVATOR, 1, 6
# 815 "data/scripts/debug.pory"
	setvar VAR_TEMP_CHALLENGE_STATUS, 0
# 816 "data/scripts/debug.pory"
	waitstate
	end

TowerTest3_2:
# 808 "data/scripts/debug.pory"
	goto BattleFrontier_BattleTowerLobby_EventScript_WaitForLinkOpponentLoad
	goto TowerTest3_1


SearchVar::
# 687 "data/scripts/debug.pory"
	.string "test$"
