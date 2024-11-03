# 1 "data/scripts/players_house.pory"

# 2 "data/scripts/players_house.pory"
@ Local IDs are the same for both players houses
# 3 "data/scripts/players_house.pory"
.set LOCALID_MOM, 1
# 4 "data/scripts/players_house.pory"
.set LOCALID_DAD, 5
# 5 "data/scripts/players_house.pory"
.set LOCALID_MOM_2F, 14
# 6 "data/scripts/players_house.pory"

# 7 "data/scripts/players_house.pory"
PlayersHouse_2F_EventScript_BlockStairsUntilClockIsSet::
# 8 "data/scripts/players_house.pory"
	setvar VAR_LITTLEROOT_INTRO_STATE, 5
# 9 "data/scripts/players_house.pory"
	return
# 10 "data/scripts/players_house.pory"

# 11 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_EnterHouseMovingIn::
# 12 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_IsntItNiceInHere, MSGBOX_DEFAULT
# 13 "data/scripts/players_house.pory"
	applymovement VAR_0x8004, Common_Movement_FacePlayer
# 14 "data/scripts/players_house.pory"
	waitmovement 0
# 15 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8005, MALE, PlayersHouse_1F_EventScript_MomFacePlayerMovingInMale
# 16 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8005, FEMALE, PlayersHouse_1F_EventScript_MomFacePlayerMovingInFemale
# 17 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_MoversPokemonGoSetClock, MSGBOX_DEFAULT
# 18 "data/scripts/players_house.pory"
	closemessage
# 19 "data/scripts/players_house.pory"
	setvar VAR_LITTLEROOT_INTRO_STATE, 4
# 20 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, PlayersHouse_1F_Movement_PlayerWalkIn
# 21 "data/scripts/players_house.pory"
	applymovement VAR_0x8004, Common_Movement_WalkInPlaceFasterUp
# 22 "data/scripts/players_house.pory"
	waitmovement 0
# 23 "data/scripts/players_house.pory"
	releaseall
# 24 "data/scripts/players_house.pory"
	end
# 25 "data/scripts/players_house.pory"

# 26 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_MomFacePlayerMovingInMale::
# 27 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFasterRight
# 28 "data/scripts/players_house.pory"
	waitmovement 0
# 29 "data/scripts/players_house.pory"
	return
# 30 "data/scripts/players_house.pory"

# 31 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_MomFacePlayerMovingInFemale::
# 32 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
# 33 "data/scripts/players_house.pory"
	waitmovement 0
# 34 "data/scripts/players_house.pory"
	return
# 35 "data/scripts/players_house.pory"

# 36 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_PlayerWalkIn:
# 37 "data/scripts/players_house.pory"
	walk_up
# 38 "data/scripts/players_house.pory"
	step_end
# 39 "data/scripts/players_house.pory"

# 40 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_MomGoSeeRoom::
# 41 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_ArentYouInterestedInRoom, MSGBOX_DEFAULT
# 42 "data/scripts/players_house.pory"
	closemessage
# 43 "data/scripts/players_house.pory"
	applymovement VAR_0x8004, Common_Movement_WalkInPlaceFasterUp
# 44 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, PlayersHouse_1F_Movement_MovePlayerAwayFromDoor
# 45 "data/scripts/players_house.pory"
	waitmovement 0
# 46 "data/scripts/players_house.pory"
	releaseall
# 47 "data/scripts/players_house.pory"
	end
# 48 "data/scripts/players_house.pory"

# 49 "data/scripts/players_house.pory"
LittlerootTown_BrendansHouse_2F_EventScript_WallClock::
# 50 "data/scripts/players_house.pory"
	lockall
# 51 "data/scripts/players_house.pory"
	setvar VAR_0x8004, MALE
# 52 "data/scripts/players_house.pory"
	goto PlayersHouse_2F_EventScript_WallClock
# 53 "data/scripts/players_house.pory"
	end
# 54 "data/scripts/players_house.pory"

# 55 "data/scripts/players_house.pory"
LittlerootTown_MaysHouse_2F_EventScript_WallClock::
# 56 "data/scripts/players_house.pory"
	lockall
# 57 "data/scripts/players_house.pory"
	setvar VAR_0x8004, FEMALE
# 58 "data/scripts/players_house.pory"
	goto PlayersHouse_2F_EventScript_WallClock
# 59 "data/scripts/players_house.pory"
	end
# 60 "data/scripts/players_house.pory"

# 61 "data/scripts/players_house.pory"
PlayersHouse_2F_EventScript_WallClock::
# 62 "data/scripts/players_house.pory"
	goto_if_set FLAG_SET_WALL_CLOCK, PlayersHouse_2F_EventScript_CheckWallClock
# 63 "data/scripts/players_house.pory"
	call PlayersHouse_2F_EventScript_SetWallClock
# 64 "data/scripts/players_house.pory"
	delay 30
# 65 "data/scripts/players_house.pory"
	setvar VAR_LITTLEROOT_INTRO_STATE, 6
# 66 "data/scripts/players_house.pory"
	setflag FLAG_SET_WALL_CLOCK
# 67 "data/scripts/players_house.pory"
	setflag FLAG_HIDE_LITTLEROOT_TOWN_PLAYERS_HOUSE_VIGOROTH_1
# 68 "data/scripts/players_house.pory"
	setflag FLAG_HIDE_LITTLEROOT_TOWN_PLAYERS_HOUSE_VIGOROTH_2
# 69 "data/scripts/players_house.pory"
	checkplayergender
# 70 "data/scripts/players_house.pory"
	call_if_eq VAR_RESULT, MALE, PlayersHouse_2F_EventScript_MomComesUpstairsMale
# 71 "data/scripts/players_house.pory"
	call_if_eq VAR_RESULT, FEMALE, PlayersHouse_2F_EventScript_MomComesUpstairsFemale
# 72 "data/scripts/players_house.pory"
	playse SE_EXIT
# 73 "data/scripts/players_house.pory"
	removeobject VAR_0x8008
# 74 "data/scripts/players_house.pory"
	releaseall
# 75 "data/scripts/players_house.pory"
	end
# 76 "data/scripts/players_house.pory"

# 77 "data/scripts/players_house.pory"
PlayersHouse_2F_EventScript_MomComesUpstairsMale::
# 78 "data/scripts/players_house.pory"
	setvar VAR_0x8008, LOCALID_MOM_2F
# 79 "data/scripts/players_house.pory"
	addobject VAR_0x8008
# 80 "data/scripts/players_house.pory"
	applymovement VAR_0x8008, PlayersHouse_2F_Movement_MomEntersMale
# 81 "data/scripts/players_house.pory"
	waitmovement 0
# 82 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFasterRight
# 83 "data/scripts/players_house.pory"
	waitmovement 0
# 84 "data/scripts/players_house.pory"
	msgbox PlayersHouse_2F_Text_HowDoYouLikeYourRoom, MSGBOX_DEFAULT
# 85 "data/scripts/players_house.pory"
	closemessage
# 86 "data/scripts/players_house.pory"
	applymovement VAR_0x8008, PlayersHouse_2F_Movement_MomExitsMale
# 87 "data/scripts/players_house.pory"
	waitmovement 0
# 88 "data/scripts/players_house.pory"
	return
# 89 "data/scripts/players_house.pory"

# 90 "data/scripts/players_house.pory"
PlayersHouse_2F_EventScript_MomComesUpstairsFemale::
# 91 "data/scripts/players_house.pory"
	setvar VAR_0x8008, LOCALID_MOM_2F
# 92 "data/scripts/players_house.pory"
	addobject VAR_0x8008
# 93 "data/scripts/players_house.pory"
	applymovement VAR_0x8008, PlayersHouse_2F_Movement_MomEntersFemale
# 94 "data/scripts/players_house.pory"
	waitmovement 0
# 95 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
# 96 "data/scripts/players_house.pory"
	waitmovement 0
# 97 "data/scripts/players_house.pory"
	msgbox PlayersHouse_2F_Text_HowDoYouLikeYourRoom, MSGBOX_DEFAULT
# 98 "data/scripts/players_house.pory"
	closemessage
# 99 "data/scripts/players_house.pory"
	applymovement VAR_0x8008, PlayersHouse_2F_Movement_MomExitsFemale
# 100 "data/scripts/players_house.pory"
	waitmovement 0
# 101 "data/scripts/players_house.pory"
	return
# 102 "data/scripts/players_house.pory"

# 103 "data/scripts/players_house.pory"
PlayersHouse_2F_EventScript_CheckWallClock::
# 104 "data/scripts/players_house.pory"
	incrementgamestat GAME_STAT_CHECKED_CLOCK
# 105 "data/scripts/players_house.pory"
	fadescreen FADE_TO_BLACK
# 106 "data/scripts/players_house.pory"
	special Special_ViewWallClock
# 107 "data/scripts/players_house.pory"
	waitstate
# 108 "data/scripts/players_house.pory"
	releaseall
# 109 "data/scripts/players_house.pory"
	end
# 110 "data/scripts/players_house.pory"

# 111 "data/scripts/players_house.pory"
PlayersHouse_2F_EventScript_SetWallClock::
# 112 "data/scripts/players_house.pory"
	fadescreen FADE_TO_BLACK
# 113 "data/scripts/players_house.pory"
	special StartWallClock
# 114 "data/scripts/players_house.pory"
	waitstate
# 115 "data/scripts/players_house.pory"
	return
# 116 "data/scripts/players_house.pory"

# 117 "data/scripts/players_house.pory"
PlayersHouse_2F_Movement_MomEntersMale:
# 118 "data/scripts/players_house.pory"
	delay_8
# 119 "data/scripts/players_house.pory"
	walk_down
# 120 "data/scripts/players_house.pory"
	walk_in_place_faster_left
# 121 "data/scripts/players_house.pory"
	delay_16
# 122 "data/scripts/players_house.pory"
	delay_8
# 123 "data/scripts/players_house.pory"
	walk_left
# 124 "data/scripts/players_house.pory"
	step_end
# 125 "data/scripts/players_house.pory"

# 126 "data/scripts/players_house.pory"
PlayersHouse_2F_Movement_MomExitsMale:
# 127 "data/scripts/players_house.pory"
	walk_right
# 128 "data/scripts/players_house.pory"
	walk_up
# 129 "data/scripts/players_house.pory"
	delay_8
# 130 "data/scripts/players_house.pory"
	step_end
# 131 "data/scripts/players_house.pory"

# 132 "data/scripts/players_house.pory"
PlayersHouse_2F_Movement_MomEntersFemale:
# 133 "data/scripts/players_house.pory"
	delay_8
# 134 "data/scripts/players_house.pory"
	walk_down
# 135 "data/scripts/players_house.pory"
	walk_in_place_faster_right
# 136 "data/scripts/players_house.pory"
	delay_16
# 137 "data/scripts/players_house.pory"
	delay_8
# 138 "data/scripts/players_house.pory"
	walk_right
# 139 "data/scripts/players_house.pory"
	step_end
# 140 "data/scripts/players_house.pory"

# 141 "data/scripts/players_house.pory"
PlayersHouse_2F_Movement_MomExitsFemale:
# 142 "data/scripts/players_house.pory"
	walk_left
# 143 "data/scripts/players_house.pory"
	walk_up
# 144 "data/scripts/players_house.pory"
	delay_8
# 145 "data/scripts/players_house.pory"
	step_end
# 146 "data/scripts/players_house.pory"

# 147 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_SetWatchedBroadcast::
# 148 "data/scripts/players_house.pory"
	setvar VAR_LITTLEROOT_INTRO_STATE, 7
# 149 "data/scripts/players_house.pory"
	releaseall
# 150 "data/scripts/players_house.pory"
	end
# 151 "data/scripts/players_house.pory"

# 152 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_PetalburgGymReportMale::
# 153 "data/scripts/players_house.pory"
	applymovement VAR_0x8005, Common_Movement_WalkInPlaceFasterRight
# 154 "data/scripts/players_house.pory"
	waitmovement 0
# 155 "data/scripts/players_house.pory"
	call PlayersHouse_1F_EventScript_MomNoticeGymBroadcast
# 156 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, PlayersHouse_1F_Movement_PlayerApproachTVForGymMale
# 157 "data/scripts/players_house.pory"
	waitmovement 0
# 158 "data/scripts/players_house.pory"
	playbgm MUS_ENCOUNTER_INTERVIEWER, FALSE
# 159 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_MaybeDadWillBeOn, MSGBOX_DEFAULT
# 160 "data/scripts/players_house.pory"
	closemessage
# 161 "data/scripts/players_house.pory"
	applymovement VAR_0x8005, PlayersHouse_1F_Movement_MomMakeRoomToSeeTVMale
# 162 "data/scripts/players_house.pory"
	waitmovement 0
# 163 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, PlayersHouse_1F_Movement_PlayerMoveToTVMale
# 164 "data/scripts/players_house.pory"
	waitmovement 0
# 165 "data/scripts/players_house.pory"
	call PlayersHouse_1F_EventScript_WatchGymBroadcast
# 166 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
# 167 "data/scripts/players_house.pory"
	waitmovement 0
# 168 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_ItsOverWeMissedHim, MSGBOX_DEFAULT
# 169 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_GoIntroduceYourselfNextDoor, MSGBOX_DEFAULT
# 170 "data/scripts/players_house.pory"
	closemessage
# 171 "data/scripts/players_house.pory"
	setvar VAR_TEMP_1, 1
# 172 "data/scripts/players_house.pory"
	applymovement VAR_0x8005, PlayersHouse_1F_Movement_MomReturnToSeatMale
# 173 "data/scripts/players_house.pory"
	waitmovement 0
# 174 "data/scripts/players_house.pory"
	goto PlayersHouse_1F_EventScript_SetWatchedBroadcast
# 175 "data/scripts/players_house.pory"
	end
# 176 "data/scripts/players_house.pory"

# 177 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_PetalburgGymReportFemale::
# 178 "data/scripts/players_house.pory"
	applymovement VAR_0x8005, Common_Movement_WalkInPlaceFasterLeft
# 179 "data/scripts/players_house.pory"
	waitmovement 0
# 180 "data/scripts/players_house.pory"
	call PlayersHouse_1F_EventScript_MomNoticeGymBroadcast
# 181 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, PlayersHouse_1F_Movement_PlayerApproachTVForGymFemale
# 182 "data/scripts/players_house.pory"
	waitmovement 0
# 183 "data/scripts/players_house.pory"
	playbgm MUS_ENCOUNTER_INTERVIEWER, FALSE
# 184 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_MaybeDadWillBeOn, MSGBOX_DEFAULT
# 185 "data/scripts/players_house.pory"
	closemessage
# 186 "data/scripts/players_house.pory"
	applymovement VAR_0x8005, PlayersHouse_1F_Movement_MomMakeRoomToSeeTVFemale
# 187 "data/scripts/players_house.pory"
	waitmovement 0
# 188 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, PlayersHouse_1F_Movement_PlayerMoveToTVFemale
# 189 "data/scripts/players_house.pory"
	waitmovement 0
# 190 "data/scripts/players_house.pory"
	call PlayersHouse_1F_EventScript_WatchGymBroadcast
# 191 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFasterRight
# 192 "data/scripts/players_house.pory"
	waitmovement 0
# 193 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_ItsOverWeMissedHim, MSGBOX_DEFAULT
# 194 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_GoIntroduceYourselfNextDoor, MSGBOX_DEFAULT
# 195 "data/scripts/players_house.pory"
	closemessage
# 196 "data/scripts/players_house.pory"
	setvar VAR_TEMP_1, 1
# 197 "data/scripts/players_house.pory"
	applymovement VAR_0x8005, PlayersHouse_1F_Movement_MomReturnToSeatFemale
# 198 "data/scripts/players_house.pory"
	waitmovement 0
# 199 "data/scripts/players_house.pory"
	goto PlayersHouse_1F_EventScript_SetWatchedBroadcast
# 200 "data/scripts/players_house.pory"
	end
# 201 "data/scripts/players_house.pory"

# 202 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_MomNoticeGymBroadcast::
# 203 "data/scripts/players_house.pory"
	playse SE_PIN
# 204 "data/scripts/players_house.pory"
	applymovement VAR_0x8005, Common_Movement_ExclamationMark
# 205 "data/scripts/players_house.pory"
	waitmovement 0
# 206 "data/scripts/players_house.pory"
	applymovement VAR_0x8005, Common_Movement_Delay48
# 207 "data/scripts/players_house.pory"
	waitmovement 0
# 208 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_OhComeQuickly, MSGBOX_DEFAULT
# 209 "data/scripts/players_house.pory"
	closemessage
# 210 "data/scripts/players_house.pory"
	return
# 211 "data/scripts/players_house.pory"

# 212 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_WatchGymBroadcast::
# 213 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFasterUp
# 214 "data/scripts/players_house.pory"
	waitmovement 0
# 215 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_ReportFromPetalburgGym, MSGBOX_DEFAULT
# 216 "data/scripts/players_house.pory"
	fadedefaultbgm
# 217 "data/scripts/players_house.pory"
	special TurnOffTVScreen
# 218 "data/scripts/players_house.pory"
	setflag FLAG_SYS_TV_HOME
# 219 "data/scripts/players_house.pory"
	delay 35
# 220 "data/scripts/players_house.pory"
	return
# 221 "data/scripts/players_house.pory"

# 222 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomApproachDadMale:
# 223 "data/scripts/players_house.pory"
	walk_up
# 224 "data/scripts/players_house.pory"
	walk_right
# 225 "data/scripts/players_house.pory"
	walk_right
# 226 "data/scripts/players_house.pory"
	walk_right
# 227 "data/scripts/players_house.pory"
	walk_right
# 228 "data/scripts/players_house.pory"
	walk_down
# 229 "data/scripts/players_house.pory"
	walk_in_place_faster_right
# 230 "data/scripts/players_house.pory"
	step_end
# 231 "data/scripts/players_house.pory"

# 232 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomApproachDadFemale:
# 233 "data/scripts/players_house.pory"
	walk_up
# 234 "data/scripts/players_house.pory"
	walk_left
# 235 "data/scripts/players_house.pory"
	walk_left
# 236 "data/scripts/players_house.pory"
	walk_left
# 237 "data/scripts/players_house.pory"
	walk_left
# 238 "data/scripts/players_house.pory"
	walk_down
# 239 "data/scripts/players_house.pory"
	walk_in_place_faster_left
# 240 "data/scripts/players_house.pory"
	step_end
# 241 "data/scripts/players_house.pory"

# 242 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomApproachPlayerMale:
# 243 "data/scripts/players_house.pory"
	walk_right
# 244 "data/scripts/players_house.pory"
	step_end
# 245 "data/scripts/players_house.pory"

# 246 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomApproachPlayerFemale:
# 247 "data/scripts/players_house.pory"
	walk_left
# 248 "data/scripts/players_house.pory"
	step_end
# 249 "data/scripts/players_house.pory"

# 250 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomNoticesLatiBroadcastMale:
# 251 "data/scripts/players_house.pory"
	walk_in_place_faster_left
# 252 "data/scripts/players_house.pory"
	delay_16
# 253 "data/scripts/players_house.pory"
	delay_16
# 254 "data/scripts/players_house.pory"
	delay_16
# 255 "data/scripts/players_house.pory"
	delay_16
# 256 "data/scripts/players_house.pory"
	walk_in_place_faster_right
# 257 "data/scripts/players_house.pory"
	delay_16
# 258 "data/scripts/players_house.pory"
	delay_16
# 259 "data/scripts/players_house.pory"
	step_end
# 260 "data/scripts/players_house.pory"

# 261 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomNoticesLatiBroadcastFemale:
# 262 "data/scripts/players_house.pory"
	walk_in_place_faster_right
# 263 "data/scripts/players_house.pory"
	delay_16
# 264 "data/scripts/players_house.pory"
	delay_16
# 265 "data/scripts/players_house.pory"
	delay_16
# 266 "data/scripts/players_house.pory"
	delay_16
# 267 "data/scripts/players_house.pory"
	walk_in_place_faster_left
# 268 "data/scripts/players_house.pory"
	delay_16
# 269 "data/scripts/players_house.pory"
	delay_16
# 270 "data/scripts/players_house.pory"
	step_end
# 271 "data/scripts/players_house.pory"

# 272 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomApproachPlayerAfterTVMale:
# 273 "data/scripts/players_house.pory"
	walk_up
# 274 "data/scripts/players_house.pory"
	walk_left
# 275 "data/scripts/players_house.pory"
	walk_left
# 276 "data/scripts/players_house.pory"
	step_end
# 277 "data/scripts/players_house.pory"

# 278 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomApproachPlayerAfterTVFemale:
# 279 "data/scripts/players_house.pory"
	walk_up
# 280 "data/scripts/players_house.pory"
	walk_right
# 281 "data/scripts/players_house.pory"
	walk_right
# 282 "data/scripts/players_house.pory"
	step_end
# 283 "data/scripts/players_house.pory"

# 284 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomMakeRoomToSeeTVMale:
# 285 "data/scripts/players_house.pory"
	walk_left
# 286 "data/scripts/players_house.pory"
	walk_in_place_faster_right
# 287 "data/scripts/players_house.pory"
	step_end
# 288 "data/scripts/players_house.pory"

# 289 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomMakeRoomToSeeTVFemale:
# 290 "data/scripts/players_house.pory"
	walk_right
# 291 "data/scripts/players_house.pory"
	walk_in_place_faster_left
# 292 "data/scripts/players_house.pory"
	step_end
# 293 "data/scripts/players_house.pory"

# 294 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomReturnToSeatMale:
# 295 "data/scripts/players_house.pory"
	walk_left
# 296 "data/scripts/players_house.pory"
	walk_down
# 297 "data/scripts/players_house.pory"
	walk_in_place_faster_right
# 298 "data/scripts/players_house.pory"
	step_end
# 299 "data/scripts/players_house.pory"

# 300 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomReturnToSeatFemale:
# 301 "data/scripts/players_house.pory"
	walk_right
# 302 "data/scripts/players_house.pory"
	walk_down
# 303 "data/scripts/players_house.pory"
	walk_in_place_faster_left
# 304 "data/scripts/players_house.pory"
	step_end
# 305 "data/scripts/players_house.pory"

# 306 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_Mom::
# 307 "data/scripts/players_house.pory"
	lock
# 308 "data/scripts/players_house.pory"
	faceplayer
# 309 "data/scripts/players_house.pory"
	goto_if_eq VAR_LITTLEROOT_HOUSES_STATE_MAY, 4, PlayersHouse_1F_EventScript_DontPushYourselfTooHard
# 310 "data/scripts/players_house.pory"
	goto_if_eq VAR_LITTLEROOT_HOUSES_STATE_BRENDAN, 4, PlayersHouse_1F_EventScript_DontPushYourselfTooHard
# 311 "data/scripts/players_house.pory"
	goto_if_set FLAG_HAS_MATCH_CALL, PlayersHouse_1F_EventScript_TryRegisterMom
# 312 "data/scripts/players_house.pory"
	goto_if_set FLAG_RESCUED_BIRCH, PlayersHouse_1F_EventScript_MomHealsParty
# 313 "data/scripts/players_house.pory"
	goto_if_eq VAR_TEMP_1, 1, PlayersHouse_1F_EventScript_SeeYouHoney
# 314 "data/scripts/players_house.pory"
	goto_if_eq VAR_LITTLEROOT_INTRO_STATE, 7, PlayersHouse_1F_EventScript_DidYouMeetProfBirch
# 315 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_IsntItNiceInHere, MSGBOX_DEFAULT
# 316 "data/scripts/players_house.pory"
	release
# 317 "data/scripts/players_house.pory"
	end
# 318 "data/scripts/players_house.pory"

# 319 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_DontPushYourselfTooHard::
# 320 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_DontPushYourselfTooHard, MSGBOX_DEFAULT
# 321 "data/scripts/players_house.pory"
	release
# 322 "data/scripts/players_house.pory"
	end
# 323 "data/scripts/players_house.pory"

# 324 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_TryRegisterMom::
# 325 "data/scripts/players_house.pory"
	goto_if_set FLAG_ENABLE_MOM_MATCH_CALL, PlayersHouse_1F_EventScript_CheckGiveAmuletCoin
# 326 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_IsThatAPokenav, MSGBOX_DEFAULT
# 327 "data/scripts/players_house.pory"
	closemessage
# 328 "data/scripts/players_house.pory"
	delay 30
# 329 "data/scripts/players_house.pory"
	playfanfare MUS_REGISTER_MATCH_CALL
# 330 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_RegisteredMom, MSGBOX_DEFAULT
# 331 "data/scripts/players_house.pory"
	waitfanfare
# 332 "data/scripts/players_house.pory"
	closemessage
# 333 "data/scripts/players_house.pory"
	delay 30
# 334 "data/scripts/players_house.pory"
	setflag FLAG_ENABLE_MOM_MATCH_CALL
# 335 "data/scripts/players_house.pory"
	release
# 336 "data/scripts/players_house.pory"
	end
# 337 "data/scripts/players_house.pory"

# 338 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_CheckGiveAmuletCoin::
# 339 "data/scripts/players_house.pory"
	goto_if_set FLAG_BADGE05_GET, PlayersHouse_1F_EventScript_TryGiveAmuletCoin
# 340 "data/scripts/players_house.pory"
	goto PlayersHouse_1F_EventScript_MomHealsParty
# 341 "data/scripts/players_house.pory"

# 342 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_TryGiveAmuletCoin::
# 343 "data/scripts/players_house.pory"
	goto_if_set FLAG_RECEIVED_AMULET_COIN, PlayersHouse_1F_EventScript_MomHealsParty
# 344 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_GotDadsBadgeHeresSomethingFromMom, MSGBOX_DEFAULT
# 345 "data/scripts/players_house.pory"
	giveitem ITEM_AMULET_COIN
# 346 "data/scripts/players_house.pory"
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
# 347 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_DontPushYourselfTooHard, MSGBOX_DEFAULT
# 348 "data/scripts/players_house.pory"
	setflag FLAG_RECEIVED_AMULET_COIN
# 349 "data/scripts/players_house.pory"
	release
# 350 "data/scripts/players_house.pory"
	end
# 351 "data/scripts/players_house.pory"

# 352 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_MomHealsParty::
# 353 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_YouShouldRestABit, MSGBOX_DEFAULT
# 354 "data/scripts/players_house.pory"
	goto PlayersHouse_1F_EventScript_HealParty
# 355 "data/scripts/players_house.pory"
	end
# 356 "data/scripts/players_house.pory"

# 357 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_SeeYouHoney::
# 358 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_SeeYouHoney, MSGBOX_DEFAULT
# 359 "data/scripts/players_house.pory"
	release
# 360 "data/scripts/players_house.pory"
	end
# 361 "data/scripts/players_house.pory"

# 362 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_HealParty::
# 363 "data/scripts/players_house.pory"
	closemessage
# 364 "data/scripts/players_house.pory"
	call Common_EventScript_OutOfCenterPartyHeal
# 365 "data/scripts/players_house.pory"
	incrementgamestat GAME_STAT_RESTED_AT_HOME
# 366 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_TakeCareHoney, MSGBOX_DEFAULT
# 367 "data/scripts/players_house.pory"
	release
# 368 "data/scripts/players_house.pory"
	end
# 369 "data/scripts/players_house.pory"

# 370 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_DidYouMeetProfBirch::
# 371 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_DidYouMeetProfBirch, MSGBOX_DEFAULT
# 372 "data/scripts/players_house.pory"
	release
# 373 "data/scripts/players_house.pory"
	end
# 374 "data/scripts/players_house.pory"

# 375 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_Vigoroth1::
# 376 "data/scripts/players_house.pory"
	lock
# 377 "data/scripts/players_house.pory"
	faceplayer
# 378 "data/scripts/players_house.pory"
	waitse
# 379 "data/scripts/players_house.pory"
	playmoncry SPECIES_VIGOROTH, CRY_MODE_NORMAL
# 380 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_Vigoroth1, MSGBOX_DEFAULT
# 381 "data/scripts/players_house.pory"
	waitmoncry
# 382 "data/scripts/players_house.pory"
	release
# 383 "data/scripts/players_house.pory"
	end
# 384 "data/scripts/players_house.pory"

# 385 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_Vigoroth2::
# 386 "data/scripts/players_house.pory"
	lock
# 387 "data/scripts/players_house.pory"
	faceplayer
# 388 "data/scripts/players_house.pory"
	waitse
# 389 "data/scripts/players_house.pory"
	playmoncry SPECIES_VIGOROTH, CRY_MODE_NORMAL
# 390 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_Vigoroth2, MSGBOX_DEFAULT
# 391 "data/scripts/players_house.pory"
	waitmoncry
# 392 "data/scripts/players_house.pory"
	release
# 393 "data/scripts/players_house.pory"
	end
# 394 "data/scripts/players_house.pory"

# 395 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_PlayerApproachTVForGymMale:
# 396 "data/scripts/players_house.pory"
	walk_down
# 397 "data/scripts/players_house.pory"
	walk_down
# 398 "data/scripts/players_house.pory"
	walk_left
# 399 "data/scripts/players_house.pory"
	walk_left
# 400 "data/scripts/players_house.pory"
	walk_left
# 401 "data/scripts/players_house.pory"
	step_end
# 402 "data/scripts/players_house.pory"

# 403 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_PlayerMoveToTVMale:
# 404 "data/scripts/players_house.pory"
	walk_left
# 405 "data/scripts/players_house.pory"
	step_end
# 406 "data/scripts/players_house.pory"

# 407 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_PlayerApproachTVForGymFemale:
# 408 "data/scripts/players_house.pory"
	walk_down
# 409 "data/scripts/players_house.pory"
	walk_down
# 410 "data/scripts/players_house.pory"
	walk_right
# 411 "data/scripts/players_house.pory"
	walk_right
# 412 "data/scripts/players_house.pory"
	walk_right
# 413 "data/scripts/players_house.pory"
	step_end
# 414 "data/scripts/players_house.pory"

# 415 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_PlayerMoveToTVFemale:
# 416 "data/scripts/players_house.pory"
	walk_right
# 417 "data/scripts/players_house.pory"
	step_end
# 418 "data/scripts/players_house.pory"

# 419 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MovePlayerAwayFromDoor:
# 420 "data/scripts/players_house.pory"
	walk_up
# 421 "data/scripts/players_house.pory"
	step_end
# 422 "data/scripts/players_house.pory"

# 423 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_GetSSTicketAndSeeLatiTV::
# 424 "data/scripts/players_house.pory"
	lockall
# 425 "data/scripts/players_house.pory"
	checkplayergender
# 426 "data/scripts/players_house.pory"
	call_if_eq VAR_RESULT, MALE, PlayersHouse_1F_EventScript_SetUpObjectEventVarsMale
# 427 "data/scripts/players_house.pory"
	call_if_eq VAR_RESULT, FEMALE, PlayersHouse_1F_EventScript_SetUpObjectEventVarsFemale
# 428 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, MALE, PlayersHouse_1F_EventScript_PlayerEnterRoomMale
# 429 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, FEMALE, PlayersHouse_1F_EventScript_PlayerEnterRoomFemale
# 430 "data/scripts/players_house.pory"
	applymovement VAR_0x8009, Common_Movement_FacePlayer
# 431 "data/scripts/players_house.pory"
	waitmovement 0
# 432 "data/scripts/players_house.pory"
	playse SE_PIN
# 433 "data/scripts/players_house.pory"
	applymovement VAR_0x8009, Common_Movement_ExclamationMark
# 434 "data/scripts/players_house.pory"
	waitmovement 0
# 435 "data/scripts/players_house.pory"
	applymovement VAR_0x8009, Common_Movement_Delay48
# 436 "data/scripts/players_house.pory"
	waitmovement 0
# 437 "data/scripts/players_house.pory"
	delay 20
# 438 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, MALE, PlayersHouse_1F_EventScript_DadApproachPlayerMale
# 439 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, FEMALE, PlayersHouse_1F_EventScript_DadApproachPlayerFemale
# 440 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_TicketFromBrineyCameForYou, MSGBOX_DEFAULT
# 441 "data/scripts/players_house.pory"
	giveitem ITEM_SS_TICKET
# 442 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_PortsInSlateportLilycove, MSGBOX_DEFAULT
# 443 "data/scripts/players_house.pory"
	closemessage
# 444 "data/scripts/players_house.pory"
	delay 20
# 445 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, MALE, PlayersHouse_1F_EventScript_MomApproachDadMale
# 446 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, FEMALE, PlayersHouse_1F_EventScript_MomApproachDadFemale
# 447 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_BetterGetBackToGym, MSGBOX_DEFAULT
# 448 "data/scripts/players_house.pory"
	closemessage
# 449 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, MALE, PlayersHouse_1F_EventScript_DadExitsMale
# 450 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, FEMALE, PlayersHouse_1F_EventScript_DadExitsFemale
# 451 "data/scripts/players_house.pory"
	playse SE_DOOR
# 452 "data/scripts/players_house.pory"
	removeobject VAR_0x8009
# 453 "data/scripts/players_house.pory"
	setflag FLAG_RECEIVED_SS_TICKET
# 454 "data/scripts/players_house.pory"
	delay 30
# 455 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, MALE, PlayersHouse_1F_EventScript_MomApproachPlayerMale
# 456 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, FEMALE, PlayersHouse_1F_EventScript_MomApproachPlayerFemale
# 457 "data/scripts/players_house.pory"
	delay 20
# 458 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_DadShouldStayLonger, MSGBOX_DEFAULT
# 459 "data/scripts/players_house.pory"
	closemessage
# 460 "data/scripts/players_house.pory"
	setflag FLAG_SYS_TV_LATIAS_LATIOS
# 461 "data/scripts/players_house.pory"
	special TurnOnTVScreen
# 462 "data/scripts/players_house.pory"
	delay 60
# 463 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, MALE, PlayersHouse_1F_EventScript_MomNoticesLatiBroadcastMale
# 464 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, FEMALE, PlayersHouse_1F_EventScript_MomNoticesLatiBroadcastFemale
# 465 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_IsThatABreakingStory, MSGBOX_DEFAULT
# 466 "data/scripts/players_house.pory"
	closemessage
# 467 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, MALE, PlayersHouse_1F_EventScript_PlayerApproachTVForLatiMale
# 468 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, FEMALE, PlayersHouse_1F_EventScript_PlayerApproachTVForLatiFemale
# 469 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_LatiEmergencyNewsFlash, MSGBOX_DEFAULT
# 470 "data/scripts/players_house.pory"
	closemessage
# 471 "data/scripts/players_house.pory"
	clearflag FLAG_SYS_TV_LATIAS_LATIOS
# 472 "data/scripts/players_house.pory"
	setflag FLAG_LATIOS_OR_LATIAS_ROAMING
# 473 "data/scripts/players_house.pory"
	special TurnOffTVScreen
# 474 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, MALE, PlayersHouse_1F_EventScript_MomApproachPlayerAfterTVMale
# 475 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, FEMALE, PlayersHouse_1F_EventScript_MomApproachPlayerAfterTVFemale
# 476 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_WhatColorDidTheySay, MSGBOX_DEFAULT
# 477 "data/scripts/players_house.pory"
	multichoice 22, 8, MULTI_TV_LATI, TRUE
# 478 "data/scripts/players_house.pory"
	copyvar VAR_0x8004, VAR_RESULT
# 479 "data/scripts/players_house.pory"
	special InitRoamer
# 480 "data/scripts/players_house.pory"
	copyvar VAR_ROAMER_POKEMON, VAR_RESULT
# 481 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_StillUnknownPokemon, MSGBOX_DEFAULT
# 482 "data/scripts/players_house.pory"
	closemessage
# 483 "data/scripts/players_house.pory"
	setvar VAR_LITTLEROOT_HOUSES_STATE_MAY, 4
# 484 "data/scripts/players_house.pory"
	setvar VAR_LITTLEROOT_HOUSES_STATE_BRENDAN, 4
# 485 "data/scripts/players_house.pory"
	releaseall
# 486 "data/scripts/players_house.pory"
	end
# 487 "data/scripts/players_house.pory"

# 488 "data/scripts/players_house.pory"
@ Never called.
# 489 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_AirLatiBroadcast::
# 490 "data/scripts/players_house.pory"
	setflag FLAG_SYS_TV_LATIAS_LATIOS
# 491 "data/scripts/players_house.pory"
	return
# 492 "data/scripts/players_house.pory"

# 493 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_SetUpObjectEventVarsMale::
# 494 "data/scripts/players_house.pory"
	setvar VAR_0x8008, MALE
# 495 "data/scripts/players_house.pory"
	setvar VAR_0x8009, LOCALID_DAD
# 496 "data/scripts/players_house.pory"
	setvar VAR_0x800A, LOCALID_MOM
# 497 "data/scripts/players_house.pory"
	return
# 498 "data/scripts/players_house.pory"

# 499 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_SetUpObjectEventVarsFemale::
# 500 "data/scripts/players_house.pory"
	setvar VAR_0x8008, FEMALE
# 501 "data/scripts/players_house.pory"
	setvar VAR_0x8009, LOCALID_DAD
# 502 "data/scripts/players_house.pory"
	setvar VAR_0x800A, LOCALID_MOM
# 503 "data/scripts/players_house.pory"
	return
# 504 "data/scripts/players_house.pory"

# 505 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_DadApproachPlayerMale::
# 506 "data/scripts/players_house.pory"
	applymovement VAR_0x8009, PlayersHouse_1F_Movement_DadApproachPlayerMale
# 507 "data/scripts/players_house.pory"
	waitmovement 0
# 508 "data/scripts/players_house.pory"
	return
# 509 "data/scripts/players_house.pory"

# 510 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_DadApproachPlayerFemale::
# 511 "data/scripts/players_house.pory"
	applymovement VAR_0x8009, PlayersHouse_1F_Movement_DadApproachPlayerFemale
# 512 "data/scripts/players_house.pory"
	waitmovement 0
# 513 "data/scripts/players_house.pory"
	return
# 514 "data/scripts/players_house.pory"

# 515 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_MomApproachDadMale::
# 516 "data/scripts/players_house.pory"
	applymovement VAR_0x800A, PlayersHouse_1F_Movement_MomApproachDadMale
# 517 "data/scripts/players_house.pory"
	waitmovement 0
# 518 "data/scripts/players_house.pory"
	applymovement VAR_0x8009, PlayersHouse_1F_Movement_DadFaceMomMale
# 519 "data/scripts/players_house.pory"
	waitmovement 0
# 520 "data/scripts/players_house.pory"
	return
# 521 "data/scripts/players_house.pory"

# 522 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_MomApproachDadFemale::
# 523 "data/scripts/players_house.pory"
	applymovement VAR_0x800A, PlayersHouse_1F_Movement_MomApproachDadFemale
# 524 "data/scripts/players_house.pory"
	waitmovement 0
# 525 "data/scripts/players_house.pory"
	applymovement VAR_0x8009, PlayersHouse_1F_Movement_DadFaceMomFemale
# 526 "data/scripts/players_house.pory"
	waitmovement 0
# 527 "data/scripts/players_house.pory"
	return
# 528 "data/scripts/players_house.pory"

# 529 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_DadExitsMale::
# 530 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, PlayersHouse_1F_Movement_MomAndPlayerWatchDadExit
# 531 "data/scripts/players_house.pory"
	applymovement VAR_0x800A, PlayersHouse_1F_Movement_MomAndPlayerWatchDadExit
# 532 "data/scripts/players_house.pory"
	applymovement VAR_0x8009, PlayersHouse_1F_Movement_DadExitsMale
# 533 "data/scripts/players_house.pory"
	waitmovement 0
# 534 "data/scripts/players_house.pory"
	return
# 535 "data/scripts/players_house.pory"

# 536 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_DadExitsFemale::
# 537 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, PlayersHouse_1F_Movement_MomAndPlayerWatchDadExit
# 538 "data/scripts/players_house.pory"
	applymovement VAR_0x800A, PlayersHouse_1F_Movement_MomAndPlayerWatchDadExit
# 539 "data/scripts/players_house.pory"
	applymovement VAR_0x8009, PlayersHouse_1F_Movement_DadExitsFemale
# 540 "data/scripts/players_house.pory"
	waitmovement 0
# 541 "data/scripts/players_house.pory"
	return
# 542 "data/scripts/players_house.pory"

# 543 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_PlayerEnterRoomMale::
# 544 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, PlayersHouse_1F_Movement_PlayerEnterRoomMale
# 545 "data/scripts/players_house.pory"
	waitmovement 0
# 546 "data/scripts/players_house.pory"
	return
# 547 "data/scripts/players_house.pory"

# 548 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_PlayerEnterRoomFemale::
# 549 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, PlayersHouse_1F_Movement_PlayerEnterRoomFemale
# 550 "data/scripts/players_house.pory"
	waitmovement 0
# 551 "data/scripts/players_house.pory"
	return
# 552 "data/scripts/players_house.pory"

# 553 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_PlayerApproachTVForLatiMale::
# 554 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, PlayersHouse_1F_Movement_PlayerApproachTVForLatiMale
# 555 "data/scripts/players_house.pory"
	waitmovement 0
# 556 "data/scripts/players_house.pory"
	applymovement VAR_0x800A, Common_Movement_WalkInPlaceFasterLeft
# 557 "data/scripts/players_house.pory"
	waitmovement 0
# 558 "data/scripts/players_house.pory"
	return
# 559 "data/scripts/players_house.pory"

# 560 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_PlayerApproachTVForLatiFemale::
# 561 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, PlayersHouse_1F_Movement_PlayerApproachTVForLatiFemale
# 562 "data/scripts/players_house.pory"
	waitmovement 0
# 563 "data/scripts/players_house.pory"
	applymovement VAR_0x800A, Common_Movement_WalkInPlaceFasterRight
# 564 "data/scripts/players_house.pory"
	waitmovement 0
# 565 "data/scripts/players_house.pory"
	return
# 566 "data/scripts/players_house.pory"

# 567 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_MomApproachPlayerMale::
# 568 "data/scripts/players_house.pory"
	applymovement VAR_0x800A, PlayersHouse_1F_Movement_MomApproachPlayerMale
# 569 "data/scripts/players_house.pory"
	waitmovement 0
# 570 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
# 571 "data/scripts/players_house.pory"
	waitmovement 0
# 572 "data/scripts/players_house.pory"
	return
# 573 "data/scripts/players_house.pory"

# 574 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_MomApproachPlayerFemale::
# 575 "data/scripts/players_house.pory"
	applymovement VAR_0x800A, PlayersHouse_1F_Movement_MomApproachPlayerFemale
# 576 "data/scripts/players_house.pory"
	waitmovement 0
# 577 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFasterRight
# 578 "data/scripts/players_house.pory"
	waitmovement 0
# 579 "data/scripts/players_house.pory"
	return
# 580 "data/scripts/players_house.pory"

# 581 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_MomNoticesLatiBroadcastMale::
# 582 "data/scripts/players_house.pory"
	applymovement VAR_0x800A, PlayersHouse_1F_Movement_MomNoticesLatiBroadcastMale
# 583 "data/scripts/players_house.pory"
	waitmovement 0
# 584 "data/scripts/players_house.pory"
	return
# 585 "data/scripts/players_house.pory"

# 586 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_MomNoticesLatiBroadcastFemale::
# 587 "data/scripts/players_house.pory"
	applymovement VAR_0x800A, PlayersHouse_1F_Movement_MomNoticesLatiBroadcastFemale
# 588 "data/scripts/players_house.pory"
	waitmovement 0
# 589 "data/scripts/players_house.pory"
	return
# 590 "data/scripts/players_house.pory"

# 591 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_MomApproachPlayerAfterTVMale::
# 592 "data/scripts/players_house.pory"
	applymovement VAR_0x800A, PlayersHouse_1F_Movement_MomApproachPlayerAfterTVMale
# 593 "data/scripts/players_house.pory"
	waitmovement 0
# 594 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFasterRight
# 595 "data/scripts/players_house.pory"
	waitmovement 0
# 596 "data/scripts/players_house.pory"
	return
# 597 "data/scripts/players_house.pory"

# 598 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_MomApproachPlayerAfterTVFemale::
# 599 "data/scripts/players_house.pory"
	applymovement VAR_0x800A, PlayersHouse_1F_Movement_MomApproachPlayerAfterTVFemale
# 600 "data/scripts/players_house.pory"
	waitmovement 0
# 601 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
# 602 "data/scripts/players_house.pory"
	waitmovement 0
# 603 "data/scripts/players_house.pory"
	return
# 604 "data/scripts/players_house.pory"

# 605 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_DadApproachPlayerMale:
# 606 "data/scripts/players_house.pory"
	walk_right
# 607 "data/scripts/players_house.pory"
	walk_right
# 608 "data/scripts/players_house.pory"
	step_end
# 609 "data/scripts/players_house.pory"

# 610 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_DadApproachPlayerFemale:
# 611 "data/scripts/players_house.pory"
	walk_left
# 612 "data/scripts/players_house.pory"
	walk_left
# 613 "data/scripts/players_house.pory"
	step_end
# 614 "data/scripts/players_house.pory"

# 615 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_DadFaceMomMale:
# 616 "data/scripts/players_house.pory"
	face_left
# 617 "data/scripts/players_house.pory"
	step_end
# 618 "data/scripts/players_house.pory"

# 619 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_DadFaceMomFemale:
# 620 "data/scripts/players_house.pory"
	face_right
# 621 "data/scripts/players_house.pory"
	step_end
# 622 "data/scripts/players_house.pory"

# 623 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_DadExitsMale:
# 624 "data/scripts/players_house.pory"
	walk_down
# 625 "data/scripts/players_house.pory"
	walk_right
# 626 "data/scripts/players_house.pory"
	walk_down
# 627 "data/scripts/players_house.pory"
	delay_8
# 628 "data/scripts/players_house.pory"
	step_end
# 629 "data/scripts/players_house.pory"

# 630 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_DadExitsFemale:
# 631 "data/scripts/players_house.pory"
	walk_down
# 632 "data/scripts/players_house.pory"
	walk_left
# 633 "data/scripts/players_house.pory"
	walk_down
# 634 "data/scripts/players_house.pory"
	delay_8
# 635 "data/scripts/players_house.pory"
	step_end
# 636 "data/scripts/players_house.pory"

# 637 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_PlayerEnterRoomMale:
# 638 "data/scripts/players_house.pory"
	delay_16
# 639 "data/scripts/players_house.pory"
	walk_down
# 640 "data/scripts/players_house.pory"
	walk_down
# 641 "data/scripts/players_house.pory"
	walk_down
# 642 "data/scripts/players_house.pory"
	walk_in_place_faster_left
# 643 "data/scripts/players_house.pory"
	step_end
# 644 "data/scripts/players_house.pory"

# 645 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomAndPlayerWatchDadExit:
# 646 "data/scripts/players_house.pory"
	delay_8
# 647 "data/scripts/players_house.pory"
	delay_16
# 648 "data/scripts/players_house.pory"
	delay_16
# 649 "data/scripts/players_house.pory"
	walk_in_place_faster_down
# 650 "data/scripts/players_house.pory"
	step_end
# 651 "data/scripts/players_house.pory"

# 652 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_PlayerEnterRoomFemale:
# 653 "data/scripts/players_house.pory"
	delay_16
# 654 "data/scripts/players_house.pory"
	walk_down
# 655 "data/scripts/players_house.pory"
	walk_down
# 656 "data/scripts/players_house.pory"
	walk_down
# 657 "data/scripts/players_house.pory"
	walk_in_place_faster_right
# 658 "data/scripts/players_house.pory"
	step_end
# 659 "data/scripts/players_house.pory"

# 660 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_PlayerApproachTVForLatiMale:
# 661 "data/scripts/players_house.pory"
	walk_up
# 662 "data/scripts/players_house.pory"
	walk_left
# 663 "data/scripts/players_house.pory"
	walk_left
# 664 "data/scripts/players_house.pory"
	walk_left
# 665 "data/scripts/players_house.pory"
	walk_left
# 666 "data/scripts/players_house.pory"
	walk_in_place_faster_up
# 667 "data/scripts/players_house.pory"
	step_end
# 668 "data/scripts/players_house.pory"

# 669 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_PlayerApproachTVForLatiFemale:
# 670 "data/scripts/players_house.pory"
	walk_up
# 671 "data/scripts/players_house.pory"
	walk_right
# 672 "data/scripts/players_house.pory"
	walk_right
# 673 "data/scripts/players_house.pory"
	walk_right
# 674 "data/scripts/players_house.pory"
	walk_right
# 675 "data/scripts/players_house.pory"
	walk_in_place_faster_up
# 676 "data/scripts/players_house.pory"
	step_end
# 677 "data/scripts/players_house.pory"

# 678 "data/scripts/players_house.pory"
EventScript_RunningShoesManual::
# 679 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_RunningShoesManual, MSGBOX_SIGN
# 680 "data/scripts/players_house.pory"
	end
