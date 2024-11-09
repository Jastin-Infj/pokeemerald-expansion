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
	release
# 148 "data/scripts/debug.pory"
	end
# 149 "data/scripts/debug.pory"

# 150 "data/scripts/debug.pory"
Debug_EventScript_Script_3::
# 151 "data/scripts/debug.pory"
	release
# 152 "data/scripts/debug.pory"
	end
# 153 "data/scripts/debug.pory"

# 154 "data/scripts/debug.pory"
Debug_EventScript_Script_4::
# 155 "data/scripts/debug.pory"
	release
# 156 "data/scripts/debug.pory"
	end
# 157 "data/scripts/debug.pory"

# 158 "data/scripts/debug.pory"
Debug_EventScript_Script_5::
# 159 "data/scripts/debug.pory"
	release
# 160 "data/scripts/debug.pory"
	end
# 161 "data/scripts/debug.pory"

# 162 "data/scripts/debug.pory"
Debug_EventScript_Script_6::
# 163 "data/scripts/debug.pory"
	release
# 164 "data/scripts/debug.pory"
	end
# 165 "data/scripts/debug.pory"

# 166 "data/scripts/debug.pory"
Debug_EventScript_Script_7::
# 167 "data/scripts/debug.pory"
	release
# 168 "data/scripts/debug.pory"
	end
# 169 "data/scripts/debug.pory"

# 170 "data/scripts/debug.pory"
Debug_EventScript_Script_8::
# 171 "data/scripts/debug.pory"
	release
# 172 "data/scripts/debug.pory"
	end
# 173 "data/scripts/debug.pory"

# 174 "data/scripts/debug.pory"
Debug_CheckSaveBlock::
# 175 "data/scripts/debug.pory"
	callnative CheckSaveBlock1Size
# 176 "data/scripts/debug.pory"
	msgbox Debug_SaveBlock1Size, MSGBOX_DEFAULT
# 177 "data/scripts/debug.pory"
	callnative CheckSaveBlock2Size
# 178 "data/scripts/debug.pory"
	msgbox Debug_SaveBlock2Size, MSGBOX_DEFAULT
# 179 "data/scripts/debug.pory"
	callnative CheckSaveBlock3Size
# 180 "data/scripts/debug.pory"
	msgbox Debug_SaveBlock3Size, MSGBOX_DEFAULT
# 181 "data/scripts/debug.pory"
	callnative CheckPokemonStorageSize
# 182 "data/scripts/debug.pory"
	msgbox Debug_PokemonStorageSize, MSGBOX_DEFAULT
# 183 "data/scripts/debug.pory"
	release
# 184 "data/scripts/debug.pory"
	end
# 185 "data/scripts/debug.pory"

# 186 "data/scripts/debug.pory"
Debug_SaveBlock1Size::
# 187 "data/scripts/debug.pory"
	.string "SaveBlock1 size: {STR_VAR_1}b/{STR_VAR_2}b.\n"
# 188 "data/scripts/debug.pory"
	.string "Free space: {STR_VAR_3}b.$"
# 189 "data/scripts/debug.pory"

# 190 "data/scripts/debug.pory"
Debug_SaveBlock2Size::
# 191 "data/scripts/debug.pory"
	.string "SaveBlock2 size: {STR_VAR_1}b/{STR_VAR_2}b.\n"
# 192 "data/scripts/debug.pory"
	.string "Free space: {STR_VAR_3}b.$"
# 193 "data/scripts/debug.pory"

# 194 "data/scripts/debug.pory"
Debug_SaveBlock3Size::
# 195 "data/scripts/debug.pory"
	.string "SaveBlock3 size: {STR_VAR_1}b/{STR_VAR_2}b.\n"
# 196 "data/scripts/debug.pory"
	.string "Free space: {STR_VAR_3}b.$"
# 197 "data/scripts/debug.pory"

# 198 "data/scripts/debug.pory"
Debug_PokemonStorageSize::
# 199 "data/scripts/debug.pory"
	.string "{PKMN}Storage size: {STR_VAR_1}b/{STR_VAR_2}b.\n"
# 200 "data/scripts/debug.pory"
	.string "Free space: {STR_VAR_3}b.$"
# 201 "data/scripts/debug.pory"

# 202 "data/scripts/debug.pory"
Debug_CheckROMSpace::
# 203 "data/scripts/debug.pory"
	callnative CheckROMSize
# 204 "data/scripts/debug.pory"
	msgbox Debug_ROMSize, MSGBOX_DEFAULT
# 205 "data/scripts/debug.pory"
	release
# 206 "data/scripts/debug.pory"
	end
# 207 "data/scripts/debug.pory"

# 208 "data/scripts/debug.pory"
Debug_ROMSize::
# 209 "data/scripts/debug.pory"
	.string "ROM size: {STR_VAR_1}MB/32MB.\n"
# 210 "data/scripts/debug.pory"
	.string "Free space: {STR_VAR_2}MB.$"
# 211 "data/scripts/debug.pory"

# 212 "data/scripts/debug.pory"
Debug_HatchAnEgg::
# 213 "data/scripts/debug.pory"
	lockall
# 214 "data/scripts/debug.pory"
	getpartysize
# 215 "data/scripts/debug.pory"
	goto_if_eq VAR_RESULT, 0, Debug_NoPokemon
# 216 "data/scripts/debug.pory"
	special ChoosePartyMon
# 217 "data/scripts/debug.pory"
	waitstate
# 218 "data/scripts/debug.pory"
	goto_if_ge VAR_0x8004, PARTY_SIZE, Debug_HatchAnEgg_End
# 219 "data/scripts/debug.pory"
	specialvar VAR_RESULT, ScriptGetPartyMonSpecies
# 220 "data/scripts/debug.pory"
	goto_if_ne VAR_RESULT, SPECIES_EGG, DebugScript_HatchAnEgg_CantForceHatch
# 221 "data/scripts/debug.pory"
	special EggHatch
# 222 "data/scripts/debug.pory"
	waitstate
# 223 "data/scripts/debug.pory"
Debug_HatchAnEgg_End::
# 224 "data/scripts/debug.pory"
	releaseall
# 225 "data/scripts/debug.pory"
	end
# 226 "data/scripts/debug.pory"

# 227 "data/scripts/debug.pory"
Debug_NoPokemon::
# 228 "data/scripts/debug.pory"
	msgbox DebugScript_Text_EmptyParty, MSGBOX_DEFAULT
# 229 "data/scripts/debug.pory"
	releaseall
# 230 "data/scripts/debug.pory"
	end
# 231 "data/scripts/debug.pory"

# 232 "data/scripts/debug.pory"
DebugScript_HatchAnEgg_CantForceHatch::
# 233 "data/scripts/debug.pory"
	msgbox DebugScript_HatchAnEgg_Text_NotAnEgg, MSGBOX_DEFAULT
# 234 "data/scripts/debug.pory"
	releaseall
# 235 "data/scripts/debug.pory"
	end
# 236 "data/scripts/debug.pory"

# 237 "data/scripts/debug.pory"
DebugScript_Text_EmptyParty::
# 238 "data/scripts/debug.pory"
	.string "You have no Pokémon nor Eggs.$"
# 239 "data/scripts/debug.pory"

# 240 "data/scripts/debug.pory"
DebugScript_HatchAnEgg_Text_NotAnEgg::
# 241 "data/scripts/debug.pory"
	.string "That's not a Pokémon Egg.$"
# 242 "data/scripts/debug.pory"

# 243 "data/scripts/debug.pory"
DebugScript_ZeroDaycareMons::
# 244 "data/scripts/debug.pory"
	msgbox DebugText_DaycareNoPokemon, MSGBOX_DEFAULT
# 245 "data/scripts/debug.pory"
	releaseall
# 246 "data/scripts/debug.pory"
	end
# 247 "data/scripts/debug.pory"

# 248 "data/scripts/debug.pory"
DebugScript_OneDaycareMons::
# 249 "data/scripts/debug.pory"
	msgbox DebugText_DaycareOnePokemon, MSGBOX_DEFAULT
# 250 "data/scripts/debug.pory"
	releaseall
# 251 "data/scripts/debug.pory"
	end
# 252 "data/scripts/debug.pory"

# 253 "data/scripts/debug.pory"
DebugScript_DaycareMonsNotCompatible::
# 254 "data/scripts/debug.pory"
	msgbox DebugText_DaycarePokemonNotCompatible, MSGBOX_DEFAULT
# 255 "data/scripts/debug.pory"
	releaseall
# 256 "data/scripts/debug.pory"
	end
# 257 "data/scripts/debug.pory"

# 258 "data/scripts/debug.pory"
DebugText_DaycareNoPokemon:
# 259 "data/scripts/debug.pory"
	.string "You have no Pokémon at Daycare.$"
# 260 "data/scripts/debug.pory"

# 261 "data/scripts/debug.pory"
DebugText_DaycareOnePokemon:
# 262 "data/scripts/debug.pory"
	.string "You have only one Pokémon at Daycare.$"
# 263 "data/scripts/debug.pory"

# 264 "data/scripts/debug.pory"
DebugText_DaycarePokemonNotCompatible:
# 265 "data/scripts/debug.pory"
	.string "Your Pokémon at Daycare can't\nhave babies together!$"
# 266 "data/scripts/debug.pory"

# 267 "data/scripts/debug.pory"
Debug_ShowExpansionVersion::
# 268 "data/scripts/debug.pory"
	callnative BufferExpansionVersion
# 269 "data/scripts/debug.pory"
	msgbox Debug_ExpansionVersion, MSGBOX_DEFAULT
# 270 "data/scripts/debug.pory"
	release
# 271 "data/scripts/debug.pory"
	end
# 272 "data/scripts/debug.pory"

# 273 "data/scripts/debug.pory"
Debug_ExpansionVersion:
# 274 "data/scripts/debug.pory"
	.string "pokeemerald-expansion {STR_VAR_1}$"
# 275 "data/scripts/debug.pory"

# 276 "data/scripts/debug.pory"
Debug_BerryPestsDisabled::
# 277 "data/scripts/debug.pory"
	msgbox DebugText_BerryPestsDisabled, MSGBOX_DEFAULT
# 278 "data/scripts/debug.pory"
	release
# 279 "data/scripts/debug.pory"
	end
# 280 "data/scripts/debug.pory"

# 281 "data/scripts/debug.pory"
DebugText_BerryPestsDisabled:
# 282 "data/scripts/debug.pory"
	.string "OW_BERRY_PESTS is disabled.\n"
# 283 "data/scripts/debug.pory"
	.string "Unable to force pests onto berry trees.$"
# 284 "data/scripts/debug.pory"

# 285 "data/scripts/debug.pory"
Debug_BerryWeedsDisabled::
# 286 "data/scripts/debug.pory"
	msgbox DebugText_BerryWeedsDisabled, MSGBOX_DEFAULT
# 287 "data/scripts/debug.pory"
	release
# 288 "data/scripts/debug.pory"
	end
# 289 "data/scripts/debug.pory"

# 290 "data/scripts/debug.pory"
DebugText_BerryWeedsDisabled:
# 291 "data/scripts/debug.pory"
	.string "OW_BERRY_WEEDS is disabled.\n"
# 292 "data/scripts/debug.pory"
	.string "Unable to force weeds onto berry trees.$"
# 293 "data/scripts/debug.pory"

# 294 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1::
# 295 "data/scripts/debug.pory"
	lockall
# 296 "data/scripts/debug.pory"
	getpartysize
# 297 "data/scripts/debug.pory"
	goto_if_eq VAR_RESULT, 0, Debug_NoPokemon
# 298 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Single, 0
# 299 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_PartyWide, 1
# 300 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Close, 2
# 301 "data/scripts/debug.pory"
	dynmultistack 0, 0, FALSE, 3 FALSE, 0, NULL
# 302 "data/scripts/debug.pory"
	switch VAR_RESULT
# 303 "data/scripts/debug.pory"
	case 0, Debug_EventScript_InflictStatus1_Single
# 304 "data/scripts/debug.pory"
	case 1, Debug_EventScript_InflictStatus1_Party
# 305 "data/scripts/debug.pory"
	case 2, Debug_EventScript_InflictStatus1_Close
# 306 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Close:
# 307 "data/scripts/debug.pory"
	releaseall
# 308 "data/scripts/debug.pory"
	end
# 309 "data/scripts/debug.pory"

# 310 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Single:
# 311 "data/scripts/debug.pory"
	special ChoosePartyMon
# 312 "data/scripts/debug.pory"
	waitstate
# 313 "data/scripts/debug.pory"
	goto_if_ge VAR_0x8004, PARTY_SIZE, Debug_EventScript_InflictStatus1_Close
# 314 "data/scripts/debug.pory"
	specialvar VAR_RESULT, ScriptGetPartyMonSpecies
# 315 "data/scripts/debug.pory"
	goto_if_eq VAR_RESULT, SPECIES_EGG, Debug_EventScript_InflictStatus1_Close
# 316 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Poison, 0
# 317 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Paralysis, 1
# 318 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Sleep, 2
# 319 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Burn, 3
# 320 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Freeze, 4
# 321 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Frostbite, 5
# 322 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Close, 6
# 323 "data/scripts/debug.pory"
	dynmultistack 0, 0, FALSE, 7, FALSE, 0, NULL
# 324 "data/scripts/debug.pory"
	switch VAR_RESULT
# 325 "data/scripts/debug.pory"
	case 0, Debug_EventScript_InflictStatus1_Single_Poison
# 326 "data/scripts/debug.pory"
	case 1, Debug_EventScript_InflictStatus1_Single_Paralysis
# 327 "data/scripts/debug.pory"
	case 2, Debug_EventScript_InflictStatus1_Single_Sleep
# 328 "data/scripts/debug.pory"
	case 3, Debug_EventScript_InflictStatus1_Single_Burn
# 329 "data/scripts/debug.pory"
	case 4, Debug_EventScript_InflictStatus1_Single_Freeze
# 330 "data/scripts/debug.pory"
	case 5, Debug_EventScript_InflictStatus1_Single_Frostbite
# 331 "data/scripts/debug.pory"
	case 6, Debug_EventScript_InflictStatus1_Close
# 332 "data/scripts/debug.pory"
	case MULTI_B_PRESSED, Debug_EventScript_InflictStatus1_Close
# 333 "data/scripts/debug.pory"
	releaseall
# 334 "data/scripts/debug.pory"
	end
# 335 "data/scripts/debug.pory"

# 336 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Single_Poison:
# 337 "data/scripts/debug.pory"
	setstatus1 STATUS1_POISON, VAR_0x8004
# 338 "data/scripts/debug.pory"
	releaseall
# 339 "data/scripts/debug.pory"
	end
# 340 "data/scripts/debug.pory"

# 341 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Single_Paralysis:
# 342 "data/scripts/debug.pory"
	setstatus1 STATUS1_PARALYSIS, VAR_0x8004
# 343 "data/scripts/debug.pory"
	releaseall
# 344 "data/scripts/debug.pory"
	end
# 345 "data/scripts/debug.pory"

# 346 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Single_Sleep:
# 347 "data/scripts/debug.pory"
	setstatus1 STATUS1_SLEEP, VAR_0x8004
# 348 "data/scripts/debug.pory"
	releaseall
# 349 "data/scripts/debug.pory"
	end
# 350 "data/scripts/debug.pory"

# 351 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Single_Burn:
# 352 "data/scripts/debug.pory"
	setstatus1 STATUS1_BURN, VAR_0x8004
# 353 "data/scripts/debug.pory"
	releaseall
# 354 "data/scripts/debug.pory"
	end
# 355 "data/scripts/debug.pory"

# 356 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Single_Freeze:
# 357 "data/scripts/debug.pory"
	setstatus1 STATUS1_FREEZE, VAR_0x8004
# 358 "data/scripts/debug.pory"
	releaseall
# 359 "data/scripts/debug.pory"
	end
# 360 "data/scripts/debug.pory"

# 361 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Single_Frostbite:
# 362 "data/scripts/debug.pory"
	setstatus1 STATUS1_FROSTBITE, VAR_0x8004
# 363 "data/scripts/debug.pory"
	releaseall
# 364 "data/scripts/debug.pory"
	end
# 365 "data/scripts/debug.pory"

# 366 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Party:
# 367 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Poison, 0
# 368 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Paralysis, 1
# 369 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Sleep, 2
# 370 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Burn, 3
# 371 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Freeze, 4
# 372 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Frostbite, 5
# 373 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Close, 6
# 374 "data/scripts/debug.pory"
	dynmultistack 0, 0, FALSE, 7, FALSE, 0, NULL
# 375 "data/scripts/debug.pory"
	switch VAR_RESULT
# 376 "data/scripts/debug.pory"
	case 0, Debug_EventScript_InflictStatus1_Party_Poison
# 377 "data/scripts/debug.pory"
	case 1, Debug_EventScript_InflictStatus1_Party_Paralysis
# 378 "data/scripts/debug.pory"
	case 2, Debug_EventScript_InflictStatus1_Party_Sleep
# 379 "data/scripts/debug.pory"
	case 3, Debug_EventScript_InflictStatus1_Party_Burn
# 380 "data/scripts/debug.pory"
	case 4, Debug_EventScript_InflictStatus1_Party_Freeze
# 381 "data/scripts/debug.pory"
	case 5, Debug_EventScript_InflictStatus1_Party_Frostbite
# 382 "data/scripts/debug.pory"
	case 6, Debug_EventScript_InflictStatus1_Close
# 383 "data/scripts/debug.pory"
	case MULTI_B_PRESSED, Debug_EventScript_InflictStatus1_Close
# 384 "data/scripts/debug.pory"
	releaseall
# 385 "data/scripts/debug.pory"
	end
# 386 "data/scripts/debug.pory"

# 387 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Party_Poison:
# 388 "data/scripts/debug.pory"
	setstatus1 STATUS1_POISON, PARTY_SIZE
# 389 "data/scripts/debug.pory"
	releaseall
# 390 "data/scripts/debug.pory"
	end
# 391 "data/scripts/debug.pory"

# 392 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Party_Paralysis:
# 393 "data/scripts/debug.pory"
	setstatus1 STATUS1_PARALYSIS, PARTY_SIZE
# 394 "data/scripts/debug.pory"
	releaseall
# 395 "data/scripts/debug.pory"
	end
# 396 "data/scripts/debug.pory"

# 397 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Party_Sleep:
# 398 "data/scripts/debug.pory"
	setstatus1 STATUS1_SLEEP, PARTY_SIZE
# 399 "data/scripts/debug.pory"
	releaseall
# 400 "data/scripts/debug.pory"
	end
# 401 "data/scripts/debug.pory"

# 402 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Party_Burn:
# 403 "data/scripts/debug.pory"
	setstatus1 STATUS1_BURN, PARTY_SIZE
# 404 "data/scripts/debug.pory"
	releaseall
# 405 "data/scripts/debug.pory"
	end
# 406 "data/scripts/debug.pory"

# 407 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Party_Freeze:
# 408 "data/scripts/debug.pory"
	setstatus1 STATUS1_FREEZE, PARTY_SIZE
# 409 "data/scripts/debug.pory"
	releaseall
# 410 "data/scripts/debug.pory"
	end
# 411 "data/scripts/debug.pory"

# 412 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Party_Frostbite:
# 413 "data/scripts/debug.pory"
	setstatus1 STATUS1_FROSTBITE, PARTY_SIZE
# 414 "data/scripts/debug.pory"
	releaseall
# 415 "data/scripts/debug.pory"
	end
# 416 "data/scripts/debug.pory"

# 417 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_Single:
# 418 "data/scripts/debug.pory"
	.string "Single$"
# 419 "data/scripts/debug.pory"

# 420 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_PartyWide:
# 421 "data/scripts/debug.pory"
	.string "Party-wide$"
# 422 "data/scripts/debug.pory"

# 423 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_Close:
# 424 "data/scripts/debug.pory"
	.string "Close$"
# 425 "data/scripts/debug.pory"

# 426 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_Poison:
# 427 "data/scripts/debug.pory"
	.string "Poison$"
# 428 "data/scripts/debug.pory"

# 429 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_Paralysis:
# 430 "data/scripts/debug.pory"
	.string "Paralysis$"
# 431 "data/scripts/debug.pory"

# 432 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_Sleep:
# 433 "data/scripts/debug.pory"
	.string "Sleep$"
# 434 "data/scripts/debug.pory"

# 435 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_Burn:
# 436 "data/scripts/debug.pory"
	.string "Burn$"
# 437 "data/scripts/debug.pory"

# 438 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_Freeze:
# 439 "data/scripts/debug.pory"
	.string "Freeze$"
# 440 "data/scripts/debug.pory"

# 441 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_Frostbite:
# 442 "data/scripts/debug.pory"
	.string "Frostbite$"
# 443 "data/scripts/debug.pory"

# 444 "data/scripts/debug.pory"
Debug_EventScript_EWRAMCounters::
# 445 "data/scripts/debug.pory"
	callnative CheckEWRAMCounters
# 446 "data/scripts/debug.pory"
	msgbox Debug_EventScript_EWRAMCounters_Text, MSGBOX_DEFAULT
# 447 "data/scripts/debug.pory"
	release
# 448 "data/scripts/debug.pory"
	end
# 449 "data/scripts/debug.pory"

# 450 "data/scripts/debug.pory"
Debug_EventScript_EWRAMCounters_Text::
# 451 "data/scripts/debug.pory"
	.string "Follower Steps: {STR_VAR_1}.\n"
# 452 "data/scripts/debug.pory"
	.string "Fishing Chain: {STR_VAR_2}.$"
# 453 "data/scripts/debug.pory"

# 454 "data/scripts/debug.pory"
.endif

FactoryTest::
# 459 "data/scripts/debug.pory"
	setvar VAR_FRONTIER_FACILITY, FRONTIER_FACILITY_FACTORY
# 460 "data/scripts/debug.pory"
	setvar VAR_FRONTIER_BATTLE_MODE, FRONTIER_MODE_DOUBLES
# 461 "data/scripts/debug.pory"
	special SavePlayerParty
# 469 "data/scripts/debug.pory"
	waitmessage
# 471 "data/scripts/debug.pory"
	multichoice 17, 6, MULTI_CHALLENGEINFO, FALSE
# 473 "data/scripts/debug.pory"
	switch VAR_RESULT
# 474 "data/scripts/debug.pory"
	case 0, FactoryTest_2
# 476 "data/scripts/debug.pory"
	case 1, FactoryTest_3
# 478 "data/scripts/debug.pory"
	case 2, FactoryTest_4
# 481 "data/scripts/debug.pory"
	case MULTI_B_PRESSED, FactoryTest_5
	return

FactoryTest_2:
# 475 "data/scripts/debug.pory"
	goto FactoryTest1
	return

FactoryTest_3:
# 477 "data/scripts/debug.pory"
	goto BattleFrontier_BattleFactoryLobby_EventScript_ExplainChallenge
	return

FactoryTest_4:
# 480 "data/scripts/debug.pory"
	goto BattleFrontier_BattleFactoryLobby_EventScript_CancelChallenge
	return

FactoryTest_5:
# 483 "data/scripts/debug.pory"
	goto BattleFrontier_BattleFactoryLobby_EventScript_CancelChallenge
	return


FactoryTest1::
# 493 "data/scripts/debug.pory"
	message BattleFrontier_BattleFactoryLobby_Text_WhichLevelMode
# 494 "data/scripts/debug.pory"
	waitmessage
# 500 "data/scripts/debug.pory"
	multichoice 17, 6, MULTI_LEVEL_MODE, FALSE
# 509 "data/scripts/debug.pory"
	frontier_set FRONTIER_DATA_LVL_MODE, VAR_RESULT
# 519 "data/scripts/debug.pory"
	msgbox SearchVar
	return


SearchVar::
# 522 "data/scripts/debug.pory"
	.string "test$"
