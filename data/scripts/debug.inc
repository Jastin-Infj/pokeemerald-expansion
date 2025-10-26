# 1 "data/scripts/debug.pory"

# 2 "data/scripts/debug.pory"
Debug_MessageEnd:
# 3 "data/scripts/debug.pory"
	waitmessage
# 4 "data/scripts/debug.pory"
	waitbuttonpress
# 5 "data/scripts/debug.pory"
	releaseall
# 6 "data/scripts/debug.pory"
	end
# 7 "data/scripts/debug.pory"

# 8 "data/scripts/debug.pory"
Debug_ShowFieldMessageStringVar4::
# 9 "data/scripts/debug.pory"
	special ShowFieldMessageStringVar4
# 10 "data/scripts/debug.pory"
	goto Debug_MessageEnd
# 11 "data/scripts/debug.pory"

# 12 "data/scripts/debug.pory"
Debug_CheatStart::
# 13 "data/scripts/debug.pory"
	setflag FLAG_SYS_POKEMON_GET
# 14 "data/scripts/debug.pory"
	setflag FLAG_RESCUED_BIRCH
# 15 "data/scripts/debug.pory"
	setflag FLAG_HIDE_ROUTE_101_BIRCH_ZIGZAGOON_BATTLE
# 16 "data/scripts/debug.pory"
	setflag FLAG_ADVENTURE_STARTED
# 17 "data/scripts/debug.pory"
	clearflag FLAG_HIDE_LITTLEROOT_TOWN_BIRCHS_LAB_BIRCH
# 18 "data/scripts/debug.pory"
	setflag FLAG_HIDE_ROUTE_101_BIRCH_STARTERS_BAG
# 19 "data/scripts/debug.pory"
	setvar VAR_BIRCH_LAB_STATE, 2
# 20 "data/scripts/debug.pory"
	setvar VAR_ROUTE101_STATE, 3
# 21 "data/scripts/debug.pory"
	givemon SPECIES_TREECKO, 20, ITEM_NONE
# 22 "data/scripts/debug.pory"
	givemon SPECIES_TORCHIC, 20, ITEM_NONE
# 23 "data/scripts/debug.pory"
	givemon SPECIES_MUDKIP, 20, ITEM_NONE
# 24 "data/scripts/debug.pory"
	setflag FLAG_SYS_POKEDEX_GET
# 25 "data/scripts/debug.pory"
	special SetUnlockedPokedexFlags
# 26 "data/scripts/debug.pory"
	setflag FLAG_RECEIVED_POKEDEX_FROM_BIRCH
# 27 "data/scripts/debug.pory"
	setvar VAR_CABLE_CLUB_TUTORIAL_STATE, 2
# 28 "data/scripts/debug.pory"
	setflag FLAG_SYS_NATIONAL_DEX
# 29 "data/scripts/debug.pory"
	special EnableNationalPokedex
# 30 "data/scripts/debug.pory"
	setflag FLAG_RECEIVED_RUNNING_SHOES
# 31 "data/scripts/debug.pory"
	setflag FLAG_SYS_B_DASH
# 32 "data/scripts/debug.pory"
	setvar VAR_LITTLEROOT_TOWN_STATE, 4 @ 4: Received Running Shoes
# 33 "data/scripts/debug.pory"
	setvar VAR_LITTLEROOT_INTRO_STATE, 7 @ 7: Told to go meet rival
# 34 "data/scripts/debug.pory"
	setvar VAR_LITTLEROOT_HOUSES_STATE_BRENDAN, 2 @ 2: Met Rival's Mom (and is corresponding gender)
# 35 "data/scripts/debug.pory"
	setvar VAR_LITTLEROOT_HOUSES_STATE_MAY, 2 @ 2: Met Rival's Mom (and is corresponding gender)
# 36 "data/scripts/debug.pory"
	setvar VAR_LITTLEROOT_RIVAL_STATE, 4 @ 4: Received Pokedex
# 37 "data/scripts/debug.pory"
	setflag FLAG_RECEIVED_BIKE
# 38 "data/scripts/debug.pory"
	additem ITEM_ACRO_BIKE
# 39 "data/scripts/debug.pory"
	setvar VAR_BRINEY_HOUSE_STATE, 1
# 40 "data/scripts/debug.pory"
	setvar VAR_ROUTE116_STATE, 2
# 41 "data/scripts/debug.pory"
	setflag FLAG_HIDE_ROUTE_116_MR_BRINEY
# 42 "data/scripts/debug.pory"
	setflag FLAG_BADGE01_GET
# 43 "data/scripts/debug.pory"
	setflag FLAG_BADGE02_GET
# 44 "data/scripts/debug.pory"
	setflag FLAG_BADGE03_GET
# 45 "data/scripts/debug.pory"
	setflag FLAG_BADGE04_GET
# 46 "data/scripts/debug.pory"
	setflag FLAG_BADGE05_GET
# 47 "data/scripts/debug.pory"
	setflag FLAG_BADGE06_GET
# 48 "data/scripts/debug.pory"
	setflag FLAG_BADGE07_GET
# 49 "data/scripts/debug.pory"
	setflag FLAG_BADGE08_GET
# 50 "data/scripts/debug.pory"
	setflag FLAG_VISITED_LITTLEROOT_TOWN
# 51 "data/scripts/debug.pory"
	setflag FLAG_VISITED_OLDALE_TOWN
# 52 "data/scripts/debug.pory"
	setflag FLAG_VISITED_DEWFORD_TOWN
# 53 "data/scripts/debug.pory"
	setflag FLAG_VISITED_LAVARIDGE_TOWN
# 54 "data/scripts/debug.pory"
	setflag FLAG_VISITED_FALLARBOR_TOWN
# 55 "data/scripts/debug.pory"
	setflag FLAG_VISITED_VERDANTURF_TOWN
# 56 "data/scripts/debug.pory"
	setflag FLAG_VISITED_PACIFIDLOG_TOWN
# 57 "data/scripts/debug.pory"
	setflag FLAG_VISITED_PETALBURG_CITY
# 58 "data/scripts/debug.pory"
	setflag FLAG_VISITED_SLATEPORT_CITY
# 59 "data/scripts/debug.pory"
	setflag FLAG_VISITED_MAUVILLE_CITY
# 60 "data/scripts/debug.pory"
	setflag FLAG_VISITED_RUSTBORO_CITY
# 61 "data/scripts/debug.pory"
	setflag FLAG_VISITED_FORTREE_CITY
# 62 "data/scripts/debug.pory"
	setflag FLAG_VISITED_LILYCOVE_CITY
# 63 "data/scripts/debug.pory"
	setflag FLAG_VISITED_MOSSDEEP_CITY
# 64 "data/scripts/debug.pory"
	setflag FLAG_VISITED_SOOTOPOLIS_CITY
# 65 "data/scripts/debug.pory"
	setflag FLAG_VISITED_EVER_GRANDE_CITY
# 66 "data/scripts/debug.pory"
	setflag FLAG_LANDMARK_POKEMON_LEAGUE
# 67 "data/scripts/debug.pory"
	setflag FLAG_LANDMARK_BATTLE_FRONTIER
# 68 "data/scripts/debug.pory"
	clearflag FLAG_HIDE_BRINEYS_HOUSE_MR_BRINEY
# 69 "data/scripts/debug.pory"
	clearflag FLAG_HIDE_BRINEYS_HOUSE_PEEKO
# 70 "data/scripts/debug.pory"
	release
# 71 "data/scripts/debug.pory"
	end
# 72 "data/scripts/debug.pory"

# 73 "data/scripts/debug.pory"
Debug_FlagsNotSetOverworldConfigMessage::
# 74 "data/scripts/debug.pory"
	message Debug_FlagsNotSetOverworldConfigMessage_Text
# 75 "data/scripts/debug.pory"
	goto Debug_MessageEnd
# 76 "data/scripts/debug.pory"

# 77 "data/scripts/debug.pory"
Debug_FlagsNotSetOverworldConfigMessage_Text:
# 78 "data/scripts/debug.pory"
	.string "Feature unavailable!\n"
# 79 "data/scripts/debug.pory"
	.string "Please define a usable flag in:\l"
# 80 "data/scripts/debug.pory"
	.string "'include/config/overworld.h'!$"
# 81 "data/scripts/debug.pory"

# 82 "data/scripts/debug.pory"
Debug_FlagsNotSetBattleConfigMessage::
# 83 "data/scripts/debug.pory"
	message Debug_FlagsNotSetBattleConfigMessage_Text
# 84 "data/scripts/debug.pory"
	goto Debug_MessageEnd
# 85 "data/scripts/debug.pory"

# 86 "data/scripts/debug.pory"
Debug_FlagsNotSetBattleConfigMessage_Text:
# 87 "data/scripts/debug.pory"
	.string "Feature unavailable!\n"
# 88 "data/scripts/debug.pory"
	.string "Please define a usable flag in:\l"
# 89 "data/scripts/debug.pory"
	.string "'include/config/battle.h'!$"
# 90 "data/scripts/debug.pory"

# 91 "data/scripts/debug.pory"
Debug_BoxFilledMessage::
# 92 "data/scripts/debug.pory"
	message Debug_BoxFilledMessage_Text
# 93 "data/scripts/debug.pory"
	goto Debug_MessageEnd
# 94 "data/scripts/debug.pory"

# 95 "data/scripts/debug.pory"
Debug_BoxFilledMessage_Text:
# 96 "data/scripts/debug.pory"
	.string "Storage boxes filled!$"
# 97 "data/scripts/debug.pory"

# 98 "data/scripts/debug.pory"
Debug_EventScript_CheckEVs::
# 99 "data/scripts/debug.pory"
	lockall
# 100 "data/scripts/debug.pory"
	getpartysize
# 101 "data/scripts/debug.pory"
	goto_if_eq VAR_RESULT, 0, Debug_NoPokemon
# 102 "data/scripts/debug.pory"
	special ChoosePartyMon
# 103 "data/scripts/debug.pory"
	waitstate
# 104 "data/scripts/debug.pory"
	goto_if_ge VAR_0x8004, PARTY_SIZE, Debug_EventScript_CheckEVs_End
# 105 "data/scripts/debug.pory"
	callnative Script_GetChosenMonOffensiveEVs
# 106 "data/scripts/debug.pory"
	msgbox Debug_EventScript_Text_OffensiveEVs, MSGBOX_DEFAULT
# 107 "data/scripts/debug.pory"
	callnative Script_GetChosenMonDefensiveEVs
# 108 "data/scripts/debug.pory"
	msgbox Debug_EventScript_Text_DefensiveEVs, MSGBOX_DEFAULT
# 109 "data/scripts/debug.pory"
Debug_EventScript_CheckEVs_End::
# 110 "data/scripts/debug.pory"
	releaseall
# 111 "data/scripts/debug.pory"
	end
# 112 "data/scripts/debug.pory"

# 113 "data/scripts/debug.pory"
Debug_EventScript_Text_OffensiveEVs:
# 114 "data/scripts/debug.pory"
	.string "ATK EVs: {STR_VAR_1}, SPATK EVs: {STR_VAR_2}, SPEED EVs: {STR_VAR_3}$"
# 115 "data/scripts/debug.pory"

# 116 "data/scripts/debug.pory"
Debug_EventScript_Text_DefensiveEVs:
# 117 "data/scripts/debug.pory"
	.string "HP EVs: {STR_VAR_1}, DEF EVs: {STR_VAR_2}, SPDEF EVs: {STR_VAR_3}$"
# 118 "data/scripts/debug.pory"

# 119 "data/scripts/debug.pory"
Debug_EventScript_CheckIVs::
# 120 "data/scripts/debug.pory"
	lockall
# 121 "data/scripts/debug.pory"
	getpartysize
# 122 "data/scripts/debug.pory"
	goto_if_eq VAR_RESULT, 0, Debug_NoPokemon
# 123 "data/scripts/debug.pory"
	special ChoosePartyMon
# 124 "data/scripts/debug.pory"
	waitstate
# 125 "data/scripts/debug.pory"
	goto_if_ge VAR_0x8004, PARTY_SIZE, Debug_EventScript_CheckIVs_End
# 126 "data/scripts/debug.pory"
	callnative Script_GetChosenMonOffensiveIVs
# 127 "data/scripts/debug.pory"
	msgbox Debug_EventScript_Text_OffensiveIVs, MSGBOX_DEFAULT
# 128 "data/scripts/debug.pory"
	callnative Script_GetChosenMonDefensiveIVs
# 129 "data/scripts/debug.pory"
	msgbox Debug_EventScript_Text_DefensiveIVs, MSGBOX_DEFAULT
# 130 "data/scripts/debug.pory"
Debug_EventScript_CheckIVs_End::
# 131 "data/scripts/debug.pory"
	releaseall
# 132 "data/scripts/debug.pory"
	end
# 133 "data/scripts/debug.pory"

# 134 "data/scripts/debug.pory"
Debug_EventScript_Text_OffensiveIVs:
# 135 "data/scripts/debug.pory"
	.string "ATK IVs: {STR_VAR_1}, SPATK IVs: {STR_VAR_2}, SPEED IVs: {STR_VAR_3}$"
# 136 "data/scripts/debug.pory"

# 137 "data/scripts/debug.pory"
Debug_EventScript_Text_DefensiveIVs:
# 138 "data/scripts/debug.pory"
	.string "HP IVs: {STR_VAR_1}, DEF IVs: {STR_VAR_2}, SPDEF IVs: {STR_VAR_3}$"
# 139 "data/scripts/debug.pory"

# 140 "data/scripts/debug.pory"
@ Debug_EventScript_Script_1::
# 141 "data/scripts/debug.pory"
@	release
# 142 "data/scripts/debug.pory"
@	end
# 143 "data/scripts/debug.pory"

# 144 "data/scripts/debug.pory"
Debug_EventScript_Script_2::
# 145 "data/scripts/debug.pory"
	release
# 146 "data/scripts/debug.pory"
	end
# 147 "data/scripts/debug.pory"

# 148 "data/scripts/debug.pory"
Debug_EventScript_Script_3::
# 149 "data/scripts/debug.pory"
	release
# 150 "data/scripts/debug.pory"
	end
# 151 "data/scripts/debug.pory"

# 152 "data/scripts/debug.pory"
Debug_EventScript_Script_4::
# 153 "data/scripts/debug.pory"
	release
# 154 "data/scripts/debug.pory"
	end
# 155 "data/scripts/debug.pory"

# 156 "data/scripts/debug.pory"
Debug_EventScript_Script_5::
# 157 "data/scripts/debug.pory"
	release
# 158 "data/scripts/debug.pory"
	end
# 159 "data/scripts/debug.pory"

# 160 "data/scripts/debug.pory"
Debug_EventScript_Script_6::
# 161 "data/scripts/debug.pory"
	release
# 162 "data/scripts/debug.pory"
	end
# 163 "data/scripts/debug.pory"

# 164 "data/scripts/debug.pory"
Debug_EventScript_Script_7::
# 165 "data/scripts/debug.pory"
	release
# 166 "data/scripts/debug.pory"
	end
# 167 "data/scripts/debug.pory"

# 168 "data/scripts/debug.pory"
Debug_EventScript_Script_8::
# 169 "data/scripts/debug.pory"
	release
# 170 "data/scripts/debug.pory"
	end
# 171 "data/scripts/debug.pory"

# 172 "data/scripts/debug.pory"
Debug_CheckSaveBlock::
# 173 "data/scripts/debug.pory"
	callnative CheckSaveBlock1Size
# 174 "data/scripts/debug.pory"
	msgbox Debug_SaveBlock1Size, MSGBOX_DEFAULT
# 175 "data/scripts/debug.pory"
	callnative CheckSaveBlock2Size
# 176 "data/scripts/debug.pory"
	msgbox Debug_SaveBlock2Size, MSGBOX_DEFAULT
# 177 "data/scripts/debug.pory"
	callnative CheckSaveBlock3Size
# 178 "data/scripts/debug.pory"
	msgbox Debug_SaveBlock3Size, MSGBOX_DEFAULT
# 179 "data/scripts/debug.pory"
	callnative CheckPokemonStorageSize
# 180 "data/scripts/debug.pory"
	msgbox Debug_PokemonStorageSize, MSGBOX_DEFAULT
# 181 "data/scripts/debug.pory"
	release
# 182 "data/scripts/debug.pory"
	end
# 183 "data/scripts/debug.pory"

# 184 "data/scripts/debug.pory"
Debug_SaveBlock1Size::
# 185 "data/scripts/debug.pory"
	.string "SaveBlock1 size: {STR_VAR_1}b/{STR_VAR_2}b.\n"
# 186 "data/scripts/debug.pory"
	.string "Free space: {STR_VAR_3}b.$"
# 187 "data/scripts/debug.pory"

# 188 "data/scripts/debug.pory"
Debug_SaveBlock2Size::
# 189 "data/scripts/debug.pory"
	.string "SaveBlock2 size: {STR_VAR_1}b/{STR_VAR_2}b.\n"
# 190 "data/scripts/debug.pory"
	.string "Free space: {STR_VAR_3}b.$"
# 191 "data/scripts/debug.pory"

# 192 "data/scripts/debug.pory"
Debug_SaveBlock3Size::
# 193 "data/scripts/debug.pory"
	.string "SaveBlock3 size: {STR_VAR_1}b/{STR_VAR_2}b.\n"
# 194 "data/scripts/debug.pory"
	.string "Free space: {STR_VAR_3}b.$"
# 195 "data/scripts/debug.pory"

# 196 "data/scripts/debug.pory"
Debug_PokemonStorageSize::
# 197 "data/scripts/debug.pory"
	.string "{PKMN}Storage size: {STR_VAR_1}b/{STR_VAR_2}b.\n"
# 198 "data/scripts/debug.pory"
	.string "Free space: {STR_VAR_3}b.$"
# 199 "data/scripts/debug.pory"

# 200 "data/scripts/debug.pory"
Debug_CheckROMSpace::
# 201 "data/scripts/debug.pory"
	callnative CheckROMSize
# 202 "data/scripts/debug.pory"
	msgbox Debug_ROMSize, MSGBOX_DEFAULT
# 203 "data/scripts/debug.pory"
	release
# 204 "data/scripts/debug.pory"
	end
# 205 "data/scripts/debug.pory"

# 206 "data/scripts/debug.pory"
Debug_ROMSize::
# 207 "data/scripts/debug.pory"
	.string "ROM size: {STR_VAR_1}MB/32MB.\n"
# 208 "data/scripts/debug.pory"
	.string "Free space: {STR_VAR_2}MB.$"
# 209 "data/scripts/debug.pory"

# 210 "data/scripts/debug.pory"
Debug_HatchAnEgg::
# 211 "data/scripts/debug.pory"
	lockall
# 212 "data/scripts/debug.pory"
	getpartysize
# 213 "data/scripts/debug.pory"
	goto_if_eq VAR_RESULT, 0, Debug_NoPokemon
# 214 "data/scripts/debug.pory"
	special ChoosePartyMon
# 215 "data/scripts/debug.pory"
	waitstate
# 216 "data/scripts/debug.pory"
	goto_if_ge VAR_0x8004, PARTY_SIZE, Debug_HatchAnEgg_End
# 217 "data/scripts/debug.pory"
	specialvar VAR_RESULT, ScriptGetPartyMonSpecies
# 218 "data/scripts/debug.pory"
	goto_if_ne VAR_RESULT, SPECIES_EGG, DebugScript_HatchAnEgg_CantForceHatch
# 219 "data/scripts/debug.pory"
	special EggHatch
# 220 "data/scripts/debug.pory"
	waitstate
# 221 "data/scripts/debug.pory"
Debug_HatchAnEgg_End::
# 222 "data/scripts/debug.pory"
	releaseall
# 223 "data/scripts/debug.pory"
	end
# 224 "data/scripts/debug.pory"

# 225 "data/scripts/debug.pory"
Debug_NoPokemon::
# 226 "data/scripts/debug.pory"
	msgbox DebugScript_Text_EmptyParty, MSGBOX_DEFAULT
# 227 "data/scripts/debug.pory"
	releaseall
# 228 "data/scripts/debug.pory"
	end
# 229 "data/scripts/debug.pory"

# 230 "data/scripts/debug.pory"
DebugScript_HatchAnEgg_CantForceHatch::
# 231 "data/scripts/debug.pory"
	msgbox DebugScript_HatchAnEgg_Text_NotAnEgg, MSGBOX_DEFAULT
# 232 "data/scripts/debug.pory"
	releaseall
# 233 "data/scripts/debug.pory"
	end
# 234 "data/scripts/debug.pory"

# 235 "data/scripts/debug.pory"
DebugScript_Text_EmptyParty::
# 236 "data/scripts/debug.pory"
	.string "You have no Pokémon nor Eggs.$"
# 237 "data/scripts/debug.pory"

# 238 "data/scripts/debug.pory"
DebugScript_HatchAnEgg_Text_NotAnEgg::
# 239 "data/scripts/debug.pory"
	.string "That's not a Pokémon Egg.$"
# 240 "data/scripts/debug.pory"

# 241 "data/scripts/debug.pory"
DebugScript_ZeroDaycareMons::
# 242 "data/scripts/debug.pory"
	msgbox DebugText_DaycareNoPokemon, MSGBOX_DEFAULT
# 243 "data/scripts/debug.pory"
	releaseall
# 244 "data/scripts/debug.pory"
	end
# 245 "data/scripts/debug.pory"

# 246 "data/scripts/debug.pory"
DebugScript_OneDaycareMons::
# 247 "data/scripts/debug.pory"
	msgbox DebugText_DaycareOnePokemon, MSGBOX_DEFAULT
# 248 "data/scripts/debug.pory"
	releaseall
# 249 "data/scripts/debug.pory"
	end
# 250 "data/scripts/debug.pory"

# 251 "data/scripts/debug.pory"
DebugScript_DaycareMonsNotCompatible::
# 252 "data/scripts/debug.pory"
	msgbox DebugText_DaycarePokemonNotCompatible, MSGBOX_DEFAULT
# 253 "data/scripts/debug.pory"
	releaseall
# 254 "data/scripts/debug.pory"
	end
# 255 "data/scripts/debug.pory"

# 256 "data/scripts/debug.pory"
DebugText_DaycareNoPokemon:
# 257 "data/scripts/debug.pory"
	.string "You have no Pokémon at Daycare.$"
# 258 "data/scripts/debug.pory"

# 259 "data/scripts/debug.pory"
DebugText_DaycareOnePokemon:
# 260 "data/scripts/debug.pory"
	.string "You have only one Pokémon at Daycare.$"
# 261 "data/scripts/debug.pory"

# 262 "data/scripts/debug.pory"
DebugText_DaycarePokemonNotCompatible:
# 263 "data/scripts/debug.pory"
	.string "Your Pokémon at Daycare can't\nhave babies together!$"
# 264 "data/scripts/debug.pory"

# 265 "data/scripts/debug.pory"
Debug_ShowExpansionVersion::
# 266 "data/scripts/debug.pory"
	callnative BufferExpansionVersion
# 267 "data/scripts/debug.pory"
	msgbox Debug_ExpansionVersion, MSGBOX_DEFAULT
# 268 "data/scripts/debug.pory"
	release
# 269 "data/scripts/debug.pory"
	end
# 270 "data/scripts/debug.pory"

# 271 "data/scripts/debug.pory"
Debug_ExpansionVersion:
# 272 "data/scripts/debug.pory"
	.string "pokeemerald-expansion {STR_VAR_1}$"
# 273 "data/scripts/debug.pory"

# 274 "data/scripts/debug.pory"
Debug_EventScript_Steven_Multi::
# 275 "data/scripts/debug.pory"
	call MossdeepCity_SpaceCenter_2F_EventScript_ChoosePartyForMultiBattle
# 276 "data/scripts/debug.pory"
	release
# 277 "data/scripts/debug.pory"
	end
# 278 "data/scripts/debug.pory"

# 279 "data/scripts/debug.pory"
Debug_BerryPestsDisabled::
# 280 "data/scripts/debug.pory"
	msgbox DebugText_BerryPestsDisabled, MSGBOX_DEFAULT
# 281 "data/scripts/debug.pory"
	release
# 282 "data/scripts/debug.pory"
	end
# 283 "data/scripts/debug.pory"

# 284 "data/scripts/debug.pory"
DebugText_BerryPestsDisabled:
# 285 "data/scripts/debug.pory"
	.string "OW_BERRY_PESTS is disabled.\n"
# 286 "data/scripts/debug.pory"
	.string "Unable to force pests onto berry trees.$"
# 287 "data/scripts/debug.pory"

# 288 "data/scripts/debug.pory"
Debug_BerryWeedsDisabled::
# 289 "data/scripts/debug.pory"
	msgbox DebugText_BerryWeedsDisabled, MSGBOX_DEFAULT
# 290 "data/scripts/debug.pory"
	release
# 291 "data/scripts/debug.pory"
	end
# 292 "data/scripts/debug.pory"

# 293 "data/scripts/debug.pory"
DebugText_BerryWeedsDisabled:
# 294 "data/scripts/debug.pory"
	.string "OW_BERRY_WEEDS is disabled.\n"
# 295 "data/scripts/debug.pory"
	.string "Unable to force weeds onto berry trees.$"
# 296 "data/scripts/debug.pory"

# 297 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1::
# 298 "data/scripts/debug.pory"
	lockall
# 299 "data/scripts/debug.pory"
	getpartysize
# 300 "data/scripts/debug.pory"
	goto_if_eq VAR_RESULT, 0, Debug_NoPokemon
# 301 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Single, 0
# 302 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_PartyWide, 1
# 303 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Close, 2
# 304 "data/scripts/debug.pory"
	dynmultistack 0, 0, FALSE, 3 FALSE, 0, NULL
# 305 "data/scripts/debug.pory"
	switch VAR_RESULT
# 306 "data/scripts/debug.pory"
	case 0, Debug_EventScript_InflictStatus1_Single
# 307 "data/scripts/debug.pory"
	case 1, Debug_EventScript_InflictStatus1_Party
# 308 "data/scripts/debug.pory"
	case 2, Debug_EventScript_InflictStatus1_Close
# 309 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Close:
# 310 "data/scripts/debug.pory"
	releaseall
# 311 "data/scripts/debug.pory"
	end
# 312 "data/scripts/debug.pory"

# 313 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Single:
# 314 "data/scripts/debug.pory"
	special ChoosePartyMon
# 315 "data/scripts/debug.pory"
	waitstate
# 316 "data/scripts/debug.pory"
	goto_if_ge VAR_0x8004, PARTY_SIZE, Debug_EventScript_InflictStatus1_Close
# 317 "data/scripts/debug.pory"
	specialvar VAR_RESULT, ScriptGetPartyMonSpecies
# 318 "data/scripts/debug.pory"
	goto_if_eq VAR_RESULT, SPECIES_EGG, Debug_EventScript_InflictStatus1_Close
# 319 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Poison, 0
# 320 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Paralysis, 1
# 321 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Sleep, 2
# 322 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Burn, 3
# 323 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Freeze, 4
# 324 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Frostbite, 5
# 325 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Close, 6
# 326 "data/scripts/debug.pory"
	dynmultistack 0, 0, FALSE, 7, FALSE, 0, NULL
# 327 "data/scripts/debug.pory"
	switch VAR_RESULT
# 328 "data/scripts/debug.pory"
	case 0, Debug_EventScript_InflictStatus1_Single_Poison
# 329 "data/scripts/debug.pory"
	case 1, Debug_EventScript_InflictStatus1_Single_Paralysis
# 330 "data/scripts/debug.pory"
	case 2, Debug_EventScript_InflictStatus1_Single_Sleep
# 331 "data/scripts/debug.pory"
	case 3, Debug_EventScript_InflictStatus1_Single_Burn
# 332 "data/scripts/debug.pory"
	case 4, Debug_EventScript_InflictStatus1_Single_Freeze
# 333 "data/scripts/debug.pory"
	case 5, Debug_EventScript_InflictStatus1_Single_Frostbite
# 334 "data/scripts/debug.pory"
	case 6, Debug_EventScript_InflictStatus1_Close
# 335 "data/scripts/debug.pory"
	case MULTI_B_PRESSED, Debug_EventScript_InflictStatus1_Close
# 336 "data/scripts/debug.pory"
	releaseall
# 337 "data/scripts/debug.pory"
	end
# 338 "data/scripts/debug.pory"

# 339 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Single_Poison:
# 340 "data/scripts/debug.pory"
	setstatus1 STATUS1_POISON, VAR_0x8004
# 341 "data/scripts/debug.pory"
	releaseall
# 342 "data/scripts/debug.pory"
	end
# 343 "data/scripts/debug.pory"

# 344 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Single_Paralysis:
# 345 "data/scripts/debug.pory"
	setstatus1 STATUS1_PARALYSIS, VAR_0x8004
# 346 "data/scripts/debug.pory"
	releaseall
# 347 "data/scripts/debug.pory"
	end
# 348 "data/scripts/debug.pory"

# 349 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Single_Sleep:
# 350 "data/scripts/debug.pory"
	setstatus1 STATUS1_SLEEP, VAR_0x8004
# 351 "data/scripts/debug.pory"
	releaseall
# 352 "data/scripts/debug.pory"
	end
# 353 "data/scripts/debug.pory"

# 354 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Single_Burn:
# 355 "data/scripts/debug.pory"
	setstatus1 STATUS1_BURN, VAR_0x8004
# 356 "data/scripts/debug.pory"
	releaseall
# 357 "data/scripts/debug.pory"
	end
# 358 "data/scripts/debug.pory"

# 359 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Single_Freeze:
# 360 "data/scripts/debug.pory"
	setstatus1 STATUS1_FREEZE, VAR_0x8004
# 361 "data/scripts/debug.pory"
	releaseall
# 362 "data/scripts/debug.pory"
	end
# 363 "data/scripts/debug.pory"

# 364 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Single_Frostbite:
# 365 "data/scripts/debug.pory"
	setstatus1 STATUS1_FROSTBITE, VAR_0x8004
# 366 "data/scripts/debug.pory"
	releaseall
# 367 "data/scripts/debug.pory"
	end
# 368 "data/scripts/debug.pory"

# 369 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Party:
# 370 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Poison, 0
# 371 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Paralysis, 1
# 372 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Sleep, 2
# 373 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Burn, 3
# 374 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Freeze, 4
# 375 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Frostbite, 5
# 376 "data/scripts/debug.pory"
	dynmultipush Debug_EventScript_InflictStatus1_Text_Close, 6
# 377 "data/scripts/debug.pory"
	dynmultistack 0, 0, FALSE, 7, FALSE, 0, NULL
# 378 "data/scripts/debug.pory"
	switch VAR_RESULT
# 379 "data/scripts/debug.pory"
	case 0, Debug_EventScript_InflictStatus1_Party_Poison
# 380 "data/scripts/debug.pory"
	case 1, Debug_EventScript_InflictStatus1_Party_Paralysis
# 381 "data/scripts/debug.pory"
	case 2, Debug_EventScript_InflictStatus1_Party_Sleep
# 382 "data/scripts/debug.pory"
	case 3, Debug_EventScript_InflictStatus1_Party_Burn
# 383 "data/scripts/debug.pory"
	case 4, Debug_EventScript_InflictStatus1_Party_Freeze
# 384 "data/scripts/debug.pory"
	case 5, Debug_EventScript_InflictStatus1_Party_Frostbite
# 385 "data/scripts/debug.pory"
	case 6, Debug_EventScript_InflictStatus1_Close
# 386 "data/scripts/debug.pory"
	case MULTI_B_PRESSED, Debug_EventScript_InflictStatus1_Close
# 387 "data/scripts/debug.pory"
	releaseall
# 388 "data/scripts/debug.pory"
	end
# 389 "data/scripts/debug.pory"

# 390 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Party_Poison:
# 391 "data/scripts/debug.pory"
	setstatus1 STATUS1_POISON, PARTY_SIZE
# 392 "data/scripts/debug.pory"
	releaseall
# 393 "data/scripts/debug.pory"
	end
# 394 "data/scripts/debug.pory"

# 395 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Party_Paralysis:
# 396 "data/scripts/debug.pory"
	setstatus1 STATUS1_PARALYSIS, PARTY_SIZE
# 397 "data/scripts/debug.pory"
	releaseall
# 398 "data/scripts/debug.pory"
	end
# 399 "data/scripts/debug.pory"

# 400 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Party_Sleep:
# 401 "data/scripts/debug.pory"
	setstatus1 STATUS1_SLEEP, PARTY_SIZE
# 402 "data/scripts/debug.pory"
	releaseall
# 403 "data/scripts/debug.pory"
	end
# 404 "data/scripts/debug.pory"

# 405 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Party_Burn:
# 406 "data/scripts/debug.pory"
	setstatus1 STATUS1_BURN, PARTY_SIZE
# 407 "data/scripts/debug.pory"
	releaseall
# 408 "data/scripts/debug.pory"
	end
# 409 "data/scripts/debug.pory"

# 410 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Party_Freeze:
# 411 "data/scripts/debug.pory"
	setstatus1 STATUS1_FREEZE, PARTY_SIZE
# 412 "data/scripts/debug.pory"
	releaseall
# 413 "data/scripts/debug.pory"
	end
# 414 "data/scripts/debug.pory"

# 415 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Party_Frostbite:
# 416 "data/scripts/debug.pory"
	setstatus1 STATUS1_FROSTBITE, PARTY_SIZE
# 417 "data/scripts/debug.pory"
	releaseall
# 418 "data/scripts/debug.pory"
	end
# 419 "data/scripts/debug.pory"

# 420 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_Single:
# 421 "data/scripts/debug.pory"
	.string "Single$"
# 422 "data/scripts/debug.pory"

# 423 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_PartyWide:
# 424 "data/scripts/debug.pory"
	.string "Party-wide$"
# 425 "data/scripts/debug.pory"

# 426 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_Close:
# 427 "data/scripts/debug.pory"
	.string "Close$"
# 428 "data/scripts/debug.pory"

# 429 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_Poison:
# 430 "data/scripts/debug.pory"
	.string "Poison$"
# 431 "data/scripts/debug.pory"

# 432 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_Paralysis:
# 433 "data/scripts/debug.pory"
	.string "Paralysis$"
# 434 "data/scripts/debug.pory"

# 435 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_Sleep:
# 436 "data/scripts/debug.pory"
	.string "Sleep$"
# 437 "data/scripts/debug.pory"

# 438 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_Burn:
# 439 "data/scripts/debug.pory"
	.string "Burn$"
# 440 "data/scripts/debug.pory"

# 441 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_Freeze:
# 442 "data/scripts/debug.pory"
	.string "Freeze$"
# 443 "data/scripts/debug.pory"

# 444 "data/scripts/debug.pory"
Debug_EventScript_InflictStatus1_Text_Frostbite:
# 445 "data/scripts/debug.pory"
	.string "Frostbite$"
# 446 "data/scripts/debug.pory"

# 447 "data/scripts/debug.pory"
Debug_EventScript_EWRAMCounters::
# 448 "data/scripts/debug.pory"
	callnative CheckEWRAMCounters
# 449 "data/scripts/debug.pory"
	msgbox Debug_EventScript_EWRAMCounters_Text, MSGBOX_DEFAULT
# 450 "data/scripts/debug.pory"
	release
# 451 "data/scripts/debug.pory"
	end
# 452 "data/scripts/debug.pory"

# 453 "data/scripts/debug.pory"
Debug_EventScript_EWRAMCounters_Text::
# 454 "data/scripts/debug.pory"
	.string "Follower Steps: {STR_VAR_1}.\n"
# 455 "data/scripts/debug.pory"
	.string "Fishing Chain: {STR_VAR_2}.$"
# 456 "data/scripts/debug.pory"

# 457 "data/scripts/debug.pory"
Debug_EventScript_FontTest_Text_1::
# 458 "data/scripts/debug.pory"
	.string "{FONT_SMALL_NARROWER}"                 @ Edit this to test your font
# 459 "data/scripts/debug.pory"
	.string "Angel Adept Blind Bodice Clique\n"
# 460 "data/scripts/debug.pory"
	.string "Coast Dunce Docile Enact Eosin\l"
# 461 "data/scripts/debug.pory"
	.string "Furlong Focal Gnome Gondola Human\l"
# 462 "data/scripts/debug.pory"
	.string "Hoist Inlet Iodine Justin Jocose\l"
# 463 "data/scripts/debug.pory"
	.string "Knoll Koala Linden Loads Milliner\l"
# 464 "data/scripts/debug.pory"
	.string "Modal Number Nodule Onset Oddball\l"
# 465 "data/scripts/debug.pory"
	.string "Pneumo Poncho Quanta Qophs Rhone\l"
# 466 "data/scripts/debug.pory"
	.string "Roman Snout Sodium Tundra Tocsin\l"
# 467 "data/scripts/debug.pory"
	.string "Uncle Udder Vulcan Vocal Whale\l"
# 468 "data/scripts/debug.pory"
	.string "Woman Xmas Xenon Yunnan Young\l"
# 469 "data/scripts/debug.pory"
	.string "Zloty Zodiac.$"
# 470 "data/scripts/debug.pory"

# 471 "data/scripts/debug.pory"
Debug_EventScript_FontTest_Text_2::
# 472 "data/scripts/debug.pory"
	.string "{FONT_SMALL_NARROWER}"                 @ Edit this to test your font
# 473 "data/scripts/debug.pory"
	.string "Angel angel adept for the nuance loads\n"
# 474 "data/scripts/debug.pory"
	.string "of the arena cocoa and quaalude. Blind\l"
# 475 "data/scripts/debug.pory"
	.string "blind bodice for the submit oboe of the\l"
# 476 "data/scripts/debug.pory"
	.string "club snob and abbot. Clique clique\l"
# 477 "data/scripts/debug.pory"
	.string "coast for the pouch loco of the franc\l"
# 478 "data/scripts/debug.pory"
	.string "assoc and accede. Dunce dunce docile\l"
# 479 "data/scripts/debug.pory"
	.string "for the loudness mastodon of the\l"
# 480 "data/scripts/debug.pory"
	.string "loud statehood and huddle.$"
# 481 "data/scripts/debug.pory"

# 482 "data/scripts/debug.pory"
Debug_EventScript_FontTest_Text_3::
# 483 "data/scripts/debug.pory"
	.string "{FONT_SMALL_NARROWER}"                 @ Edit this to test your font
# 484 "data/scripts/debug.pory"
	.string "Enact enact eosin for the quench coed\n"
# 485 "data/scripts/debug.pory"
	.string "of the pique canoe and bleep. Furlong\l"
# 486 "data/scripts/debug.pory"
	.string "furlong focal for the genuflect\l"
# 487 "data/scripts/debug.pory"
	.string "profound of the motif aloof and offers.\l"
# 488 "data/scripts/debug.pory"
	.string "Gnome gnome gondola for the impugn\l"
# 489 "data/scripts/debug.pory"
	.string "logos of the unplug analog and smuggle.\l"
# 490 "data/scripts/debug.pory"
	.string "Human human hoist for the buddhist\l"
# 491 "data/scripts/debug.pory"
	.string "alcohol of the riyadh caliph and\l"
# 492 "data/scripts/debug.pory"
	.string "bathhouse.$"
# 493 "data/scripts/debug.pory"

# 494 "data/scripts/debug.pory"
Debug_EventScript_FontTest_Text_4::
# 495 "data/scripts/debug.pory"
	.string "{FONT_SMALL_NARROWER}"                 @ Edit this to test your font
# 496 "data/scripts/debug.pory"
	.string "Inlet inlet iodine for the quince\n"
# 497 "data/scripts/debug.pory"
	.string "champion of the ennui scampi and shiite.\l"
# 498 "data/scripts/debug.pory"
	.string "Justin justin jocose for the djibouti\l"
# 499 "data/scripts/debug.pory"
	.string "sojourn of the oranj raj and hajjis.\l"
# 500 "data/scripts/debug.pory"
	.string "Knoll knoll koala for the banknote\l"
# 501 "data/scripts/debug.pory"
	.string "lookout of the dybbuk outlook and\l"
# 502 "data/scripts/debug.pory"
	.string "trekked. Linden linden loads for the\l"
# 503 "data/scripts/debug.pory"
	.string "ulna monolog of the consul menthol and\l"
# 504 "data/scripts/debug.pory"
	.string "shallot.$"
# 505 "data/scripts/debug.pory"

# 506 "data/scripts/debug.pory"
Debug_EventScript_FontTest_Text_5::
# 507 "data/scripts/debug.pory"
	.string "{FONT_SMALL_NARROWER}"                 @ Edit this to test your font
# 508 "data/scripts/debug.pory"
	.string "Milliner milliner modal for the alumna\n"
# 509 "data/scripts/debug.pory"
	.string "solomon of the album custom and summon.\l"
# 510 "data/scripts/debug.pory"
	.string "Number number nodule for the unmade\l"
# 511 "data/scripts/debug.pory"
	.string "economic of the shotgun bison and\l"
# 512 "data/scripts/debug.pory"
	.string "tunnel. Onset onset oddball for the\l"
# 513 "data/scripts/debug.pory"
	.string "abandon podium of the antiquo tempo\l"
# 514 "data/scripts/debug.pory"
	.string "and moonlit. Pneumo pneumo poncho for\l"
# 515 "data/scripts/debug.pory"
	.string "the dauphin opossum of the holdup\l"
# 516 "data/scripts/debug.pory"
	.string "bishop and supplies.$"
# 517 "data/scripts/debug.pory"

# 518 "data/scripts/debug.pory"
Debug_EventScript_FontTest_Text_6::
# 519 "data/scripts/debug.pory"
	.string "{FONT_SMALL_NARROWER}"                 @ Edit this to test your font
# 520 "data/scripts/debug.pory"
	.string "Quanta quanta qophs for the inquest\n"
# 521 "data/scripts/debug.pory"
	.string "sheqel of the cinq coq and suqqu. Rhone\l"
# 522 "data/scripts/debug.pory"
	.string "rhone roman for the burnt porous of the\l"
# 523 "data/scripts/debug.pory"
	.string "lemur clamor and carrot. Snout snout\l"
# 524 "data/scripts/debug.pory"
	.string "sodium for the ensnare bosom of the\l"
# 525 "data/scripts/debug.pory"
	.string "genus pathos and missing. Tundra\l"
# 526 "data/scripts/debug.pory"
	.string "tundra tocsin for the nutmeg isotope\l"
# 527 "data/scripts/debug.pory"
	.string "of the peasant ingot and ottoman.$"
# 528 "data/scripts/debug.pory"

# 529 "data/scripts/debug.pory"
Debug_EventScript_FontTest_Text_7::
# 530 "data/scripts/debug.pory"
	.string "{FONT_SMALL_NARROWER}"                 @ Edit this to test your font
# 531 "data/scripts/debug.pory"
	.string "Uncle uncle udder for the dunes cloud\n"
# 532 "data/scripts/debug.pory"
	.string "of the hindu thou and continuum. Vulcan\l"
# 533 "data/scripts/debug.pory"
	.string "vulcan vocal for the alluvial ovoid of\l"
# 534 "data/scripts/debug.pory"
	.string "the yugoslav chekhov and revved. Whale\l"
# 535 "data/scripts/debug.pory"
	.string "whale woman for the meanwhile blowout\l"
# 536 "data/scripts/debug.pory"
	.string "of the forepaw meadow and glowworm.\l"
# 537 "data/scripts/debug.pory"
	.string "Xmas xmas xenon for the bauxite\l"
# 538 "data/scripts/debug.pory"
	.string "doxology of the tableaux equinox and\l"
# 539 "data/scripts/debug.pory"
	.string "exxon.$"
# 540 "data/scripts/debug.pory"

# 541 "data/scripts/debug.pory"
Debug_EventScript_FontTest_Text_8::
# 542 "data/scripts/debug.pory"
	.string "{FONT_SMALL_NARROWER}"                 @ Edit this to test your font
# 543 "data/scripts/debug.pory"
	.string "Yunnan yunnan young for the dynamo\n"
# 544 "data/scripts/debug.pory"
	.string "coyote of the obloquy employ and\l"
# 545 "data/scripts/debug.pory"
	.string "sayyid. Zloty zloty zodiac for the gizmo\l"
# 546 "data/scripts/debug.pory"
	.string "ozone of the franz laissez and buzzing.$"
# 547 "data/scripts/debug.pory"

# 548 "data/scripts/debug.pory"

# 549 "data/scripts/debug.pory"
Debug_EventScript_FontTest_Text_9::                 @ Special thanks to Nintendo for this nice pangram to test other glyphs
# 550 "data/scripts/debug.pory"
	.string "{FONT_SMALL_NARROWER}"                 @ Edit this to test your font
# 551 "data/scripts/debug.pory"
	.string "42 × 138 = 5796.\n"
# 552 "data/scripts/debug.pory"
	.string "Mr Jock, TV quiz PhD: bags 20% fewer\l"
# 553 "data/scripts/debug.pory"
	.string "lynx at a café; voilà, they're “worth”\l"
# 554 "data/scripts/debug.pory"
	.string "♂1/♀1 = ¥1. That's 10 + 9 - 8 = 11\l"
# 555 "data/scripts/debug.pory"
	.string "Nintendo GBA can connect to a Nintendo\l"
# 556 "data/scripts/debug.pory"
	.string "GameCube console! He claimed-though I don't\l"
# 557 "data/scripts/debug.pory"
	.string "believe him-to have done so in an eyes-shut\l"
# 558 "data/scripts/debug.pory"
	.string "state…?$"
# 559 "data/scripts/debug.pory"

# 560 "data/scripts/debug.pory"
Debug_PrintFontTest::
# 561 "data/scripts/debug.pory"
	msgbox Debug_EventScript_FontTest_Text_1, MSGBOX_DEFAULT
# 562 "data/scripts/debug.pory"
	msgbox Debug_EventScript_FontTest_Text_2, MSGBOX_DEFAULT
# 563 "data/scripts/debug.pory"
	msgbox Debug_EventScript_FontTest_Text_3, MSGBOX_DEFAULT
# 564 "data/scripts/debug.pory"
	msgbox Debug_EventScript_FontTest_Text_4, MSGBOX_DEFAULT
# 565 "data/scripts/debug.pory"
	msgbox Debug_EventScript_FontTest_Text_5, MSGBOX_DEFAULT
# 566 "data/scripts/debug.pory"
	msgbox Debug_EventScript_FontTest_Text_6, MSGBOX_DEFAULT
# 567 "data/scripts/debug.pory"
	msgbox Debug_EventScript_FontTest_Text_7, MSGBOX_DEFAULT
# 568 "data/scripts/debug.pory"
	msgbox Debug_EventScript_FontTest_Text_8, MSGBOX_DEFAULT
# 569 "data/scripts/debug.pory"
	msgbox Debug_EventScript_FontTest_Text_9, MSGBOX_DEFAULT
# 570 "data/scripts/debug.pory"
	releaseall
# 571 "data/scripts/debug.pory"
	end
# 572 "data/scripts/debug.pory"

# 573 "data/scripts/debug.pory"
Debug_EventScript_FontTest::
# 574 "data/scripts/debug.pory"
	lockall
# 575 "data/scripts/debug.pory"
	goto Debug_PrintFontTest
# 576 "data/scripts/debug.pory"
@	goto_if_eq VAR_RESULT, 0, Debug_NoPokemon
# 577 "data/scripts/debug.pory"
@	dynmultipush Debug_EventScript_InflictStatus1_Text_Single, 0
# 578 "data/scripts/debug.pory"
@	dynmultipush Debug_EventScript_InflictStatus1_Text_PartyWide, 1
# 579 "data/scripts/debug.pory"
@	dynmultipush Debug_EventScript_InflictStatus1_Text_Close, 2
# 580 "data/scripts/debug.pory"
@	dynmultistack 0, 0, FALSE, 3 FALSE, 0, NULL
# 581 "data/scripts/debug.pory"
@	switch VAR_RESULT
# 582 "data/scripts/debug.pory"
@	case 0, Debug_EventScript_InflictStatus1_Single
# 583 "data/scripts/debug.pory"
@	case 1, Debug_EventScript_InflictStatus1_Party
# 584 "data/scripts/debug.pory"
@	case 2, Debug_EventScript_InflictStatus1_Close
# 585 "data/scripts/debug.pory"
@Debug_EventScript_InflictStatus1_Close:
# 586 "data/scripts/debug.pory"
@	releaseall
# 587 "data/scripts/debug.pory"
@	end
# 588 "data/scripts/debug.pory"

# 589 "data/scripts/debug.pory"
Debug_EventScript_TellTheTime::
# 590 "data/scripts/debug.pory"
	callnative DebugMenu_CalculateTime
# 591 "data/scripts/debug.pory"
	msgbox Debug_EventScript_TellTheTime_Text_0, MSGBOX_DEFAULT
# 592 "data/scripts/debug.pory"
	waitmessage
# 593 "data/scripts/debug.pory"
	closemessage
# 594 "data/scripts/debug.pory"
	end
# 595 "data/scripts/debug.pory"

# 596 "data/scripts/debug.pory"
Debug_EventScript_PrintTimeOfDay::
# 597 "data/scripts/debug.pory"
	callnative DebugMenu_CalculateTimeOfDay
# 598 "data/scripts/debug.pory"
	msgbox DebugEventScript_PrintWeekday_Text_0, MSGBOX_DEFAULT
# 599 "data/scripts/debug.pory"
	waitmessage
# 600 "data/scripts/debug.pory"
	closemessage
# 601 "data/scripts/debug.pory"
	end
# 602 "data/scripts/debug.pory"

# 603 "data/scripts/debug.pory"
Debug_EventScript_FakeRTCNotEnabled::
# 604 "data/scripts/debug.pory"
	msgbox Debug_EventScript_FakeRTCNotEnabled_Text_0, MSGBOX_DEFAULT
# 605 "data/scripts/debug.pory"
	waitmessage
# 606 "data/scripts/debug.pory"
	closemessage
# 607 "data/scripts/debug.pory"
	return
# 608 "data/scripts/debug.pory"

# 609 "data/scripts/debug.pory"
Debug_EventScript_FakeRTCNotEnabled_Text_0:
# 610 "data/scripts/debug.pory"
	.string "You currently do not have Fake RTC\nenabled. Please enable it in include/\lconfig/overworld.h$"
# 611 "data/scripts/debug.pory"

# 612 "data/scripts/debug.pory"

# 613 "data/scripts/debug.pory"
Debug_EventScript_TellTheTime_Text_0:
# 614 "data/scripts/debug.pory"
	.string "Time and date:\n"
# 615 "data/scripts/debug.pory"
	.string "{STR_VAR_1}, {STR_VAR_2}:{STR_VAR_3}$"
# 616 "data/scripts/debug.pory"

# 617 "data/scripts/debug.pory"
DebugEventScript_PrintWeekday_Text_0:
# 618 "data/scripts/debug.pory"
	.string "Time of day: {STR_VAR_1}$"

Debug_EventScript_Script_1::
# 623 "data/scripts/debug.pory"
	lock
# 624 "data/scripts/debug.pory"
	faceplayer
# 626 "data/scripts/debug.pory"
	special SavePlayerParty
# 629 "data/scripts/debug.pory"
	setvar VAR_0x8005, 4
# 631 "data/scripts/debug.pory"
	special ChoosePartyForBattleFrontier
# 632 "data/scripts/debug.pory"
	waitstate
# 634 "data/scripts/debug.pory"
	msgbox BattleFrontier_BattleTowerLobby_Text_OkayToSaveBeforeEntering, MSGBOX_YESNO
# 635 "data/scripts/debug.pory"
	switch VAR_RESULT
# 637 "data/scripts/debug.pory"
	case NO, Debug_EventScript_Script_1_3
# 639 "data/scripts/debug.pory"
	case YES, Debug_EventScript_Script_1_4
# 679 "data/scripts/debug.pory"
	case MULTI_B_PRESSED, Debug_EventScript_Script_1_5
Debug_EventScript_Script_1_1:
# 683 "data/scripts/debug.pory"
	releaseall
	end

Debug_EventScript_Script_1_3:
# 638 "data/scripts/debug.pory"
	goto BattleFrontier_BattleTowerLobby_EventScript_LoadPartyCancelChallenge
	goto Debug_EventScript_Script_1_1

Debug_EventScript_Script_1_4:
# 646 "data/scripts/debug.pory"
	special LoadPlayerParty
# 647 "data/scripts/debug.pory"
	closemessage
# 649 "data/scripts/debug.pory"
	delay 2
# 650 "data/scripts/debug.pory"
	call Common_EventScript_SaveGame
# 656 "data/scripts/debug.pory"
	special SavePlayerParty
# 657 "data/scripts/debug.pory"
	setvar VAR_0x8004, FRONTIER_UTIL_FUNC_SET_PARTY_ORDER
# 660 "data/scripts/debug.pory"
	setvar VAR_0x8005, FRONTIER_PARTY_SIZE
# 662 "data/scripts/debug.pory"
	special CallFrontierUtilFunc
# 664 "data/scripts/debug.pory"
	setvar VAR_RESULT, 0
	goto Debug_EventScript_Script_1_1

Debug_EventScript_Script_1_5:
# 680 "data/scripts/debug.pory"
	goto BattleFrontier_BattleTowerLobby_EventScript_LoadPartyCancelChallenge
	goto Debug_EventScript_Script_1_1

