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
	delay 30
# 64 "data/scripts/players_house.pory"
	setvar VAR_LITTLEROOT_INTRO_STATE, 6
# 65 "data/scripts/players_house.pory"
	setflag FLAG_SET_WALL_CLOCK
# 66 "data/scripts/players_house.pory"
	setflag FLAG_HIDE_LITTLEROOT_TOWN_PLAYERS_HOUSE_VIGOROTH_1
# 67 "data/scripts/players_house.pory"
	setflag FLAG_HIDE_LITTLEROOT_TOWN_PLAYERS_HOUSE_VIGOROTH_2
# 68 "data/scripts/players_house.pory"
	checkplayergender
# 69 "data/scripts/players_house.pory"
	call_if_eq VAR_RESULT, MALE, PlayersHouse_2F_EventScript_MomComesUpstairsMale
# 70 "data/scripts/players_house.pory"
	call_if_eq VAR_RESULT, FEMALE, PlayersHouse_2F_EventScript_MomComesUpstairsFemale
# 71 "data/scripts/players_house.pory"
	playse SE_EXIT
# 72 "data/scripts/players_house.pory"
	removeobject VAR_0x8008
# 73 "data/scripts/players_house.pory"
	releaseall
# 74 "data/scripts/players_house.pory"
	end
# 75 "data/scripts/players_house.pory"

# 76 "data/scripts/players_house.pory"
PlayersHouse_2F_EventScript_MomComesUpstairsMale::
# 77 "data/scripts/players_house.pory"
	setvar VAR_0x8008, LOCALID_MOM_2F
# 78 "data/scripts/players_house.pory"
	addobject VAR_0x8008
# 79 "data/scripts/players_house.pory"
	applymovement VAR_0x8008, PlayersHouse_2F_Movement_MomEntersMale
# 80 "data/scripts/players_house.pory"
	waitmovement 0
# 81 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFasterRight
# 82 "data/scripts/players_house.pory"
	waitmovement 0
# 83 "data/scripts/players_house.pory"
	msgbox PlayersHouse_2F_Text_HowDoYouLikeYourRoom, MSGBOX_DEFAULT
# 84 "data/scripts/players_house.pory"
	closemessage
# 85 "data/scripts/players_house.pory"
	applymovement VAR_0x8008, PlayersHouse_2F_Movement_MomExitsMale
# 86 "data/scripts/players_house.pory"
	waitmovement 0
# 87 "data/scripts/players_house.pory"
	return
# 88 "data/scripts/players_house.pory"

# 89 "data/scripts/players_house.pory"
PlayersHouse_2F_EventScript_MomComesUpstairsFemale::
# 90 "data/scripts/players_house.pory"
	setvar VAR_0x8008, LOCALID_MOM_2F
# 91 "data/scripts/players_house.pory"
	addobject VAR_0x8008
# 92 "data/scripts/players_house.pory"
	applymovement VAR_0x8008, PlayersHouse_2F_Movement_MomEntersFemale
# 93 "data/scripts/players_house.pory"
	waitmovement 0
# 94 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
# 95 "data/scripts/players_house.pory"
	waitmovement 0
# 96 "data/scripts/players_house.pory"
	msgbox PlayersHouse_2F_Text_HowDoYouLikeYourRoom, MSGBOX_DEFAULT
# 97 "data/scripts/players_house.pory"
	closemessage
# 98 "data/scripts/players_house.pory"
	applymovement VAR_0x8008, PlayersHouse_2F_Movement_MomExitsFemale
# 99 "data/scripts/players_house.pory"
	waitmovement 0
# 100 "data/scripts/players_house.pory"
	return
# 101 "data/scripts/players_house.pory"

# 102 "data/scripts/players_house.pory"
PlayersHouse_2F_EventScript_CheckWallClock::
# 103 "data/scripts/players_house.pory"
	incrementgamestat GAME_STAT_CHECKED_CLOCK
# 104 "data/scripts/players_house.pory"
	fadescreen FADE_TO_BLACK
# 105 "data/scripts/players_house.pory"
	special Special_ViewWallClock
# 106 "data/scripts/players_house.pory"
	waitstate
# 107 "data/scripts/players_house.pory"
	releaseall
# 108 "data/scripts/players_house.pory"
	end
# 109 "data/scripts/players_house.pory"

# 110 "data/scripts/players_house.pory"
PlayersHouse_2F_EventScript_SetWallClock::
# 111 "data/scripts/players_house.pory"
	fadescreen FADE_TO_BLACK
# 112 "data/scripts/players_house.pory"
	special StartWallClock
# 113 "data/scripts/players_house.pory"
	waitstate
# 114 "data/scripts/players_house.pory"
	return
# 115 "data/scripts/players_house.pory"

# 116 "data/scripts/players_house.pory"
PlayersHouse_2F_Movement_MomEntersMale:
# 117 "data/scripts/players_house.pory"
	delay_8
# 118 "data/scripts/players_house.pory"
	walk_down
# 119 "data/scripts/players_house.pory"
	walk_in_place_faster_left
# 120 "data/scripts/players_house.pory"
	delay_16
# 121 "data/scripts/players_house.pory"
	delay_8
# 122 "data/scripts/players_house.pory"
	walk_left
# 123 "data/scripts/players_house.pory"
	step_end
# 124 "data/scripts/players_house.pory"

# 125 "data/scripts/players_house.pory"
PlayersHouse_2F_Movement_MomExitsMale:
# 126 "data/scripts/players_house.pory"
	walk_right
# 127 "data/scripts/players_house.pory"
	walk_up
# 128 "data/scripts/players_house.pory"
	delay_8
# 129 "data/scripts/players_house.pory"
	step_end
# 130 "data/scripts/players_house.pory"

# 131 "data/scripts/players_house.pory"
PlayersHouse_2F_Movement_MomEntersFemale:
# 132 "data/scripts/players_house.pory"
	delay_8
# 133 "data/scripts/players_house.pory"
	walk_down
# 134 "data/scripts/players_house.pory"
	walk_in_place_faster_right
# 135 "data/scripts/players_house.pory"
	delay_16
# 136 "data/scripts/players_house.pory"
	delay_8
# 137 "data/scripts/players_house.pory"
	walk_right
# 138 "data/scripts/players_house.pory"
	step_end
# 139 "data/scripts/players_house.pory"

# 140 "data/scripts/players_house.pory"
PlayersHouse_2F_Movement_MomExitsFemale:
# 141 "data/scripts/players_house.pory"
	walk_left
# 142 "data/scripts/players_house.pory"
	walk_up
# 143 "data/scripts/players_house.pory"
	delay_8
# 144 "data/scripts/players_house.pory"
	step_end
# 145 "data/scripts/players_house.pory"

# 146 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_SetWatchedBroadcast::
# 147 "data/scripts/players_house.pory"
	setvar VAR_LITTLEROOT_INTRO_STATE, 7
# 148 "data/scripts/players_house.pory"
	releaseall
# 149 "data/scripts/players_house.pory"
	end
# 150 "data/scripts/players_house.pory"

# 151 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_PetalburgGymReportMale::
# 152 "data/scripts/players_house.pory"
	applymovement VAR_0x8005, Common_Movement_WalkInPlaceFasterRight
# 153 "data/scripts/players_house.pory"
	waitmovement 0
# 154 "data/scripts/players_house.pory"
	call PlayersHouse_1F_EventScript_MomNoticeGymBroadcast
# 155 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, PlayersHouse_1F_Movement_PlayerApproachTVForGymMale
# 156 "data/scripts/players_house.pory"
	waitmovement 0
# 157 "data/scripts/players_house.pory"
	playbgm MUS_ENCOUNTER_INTERVIEWER, FALSE
# 158 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_MaybeDadWillBeOn, MSGBOX_DEFAULT
# 159 "data/scripts/players_house.pory"
	closemessage
# 160 "data/scripts/players_house.pory"
	applymovement VAR_0x8005, PlayersHouse_1F_Movement_MomMakeRoomToSeeTVMale
# 161 "data/scripts/players_house.pory"
	waitmovement 0
# 162 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, PlayersHouse_1F_Movement_PlayerMoveToTVMale
# 163 "data/scripts/players_house.pory"
	waitmovement 0
# 164 "data/scripts/players_house.pory"
	call PlayersHouse_1F_EventScript_WatchGymBroadcast
# 165 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
# 166 "data/scripts/players_house.pory"
	waitmovement 0
# 167 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_ItsOverWeMissedHim, MSGBOX_DEFAULT
# 168 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_GoIntroduceYourselfNextDoor, MSGBOX_DEFAULT
# 169 "data/scripts/players_house.pory"
	closemessage
# 170 "data/scripts/players_house.pory"
	setvar VAR_TEMP_1, 1
# 171 "data/scripts/players_house.pory"
	applymovement VAR_0x8005, PlayersHouse_1F_Movement_MomReturnToSeatMale
# 172 "data/scripts/players_house.pory"
	waitmovement 0
# 173 "data/scripts/players_house.pory"
	goto PlayersHouse_1F_EventScript_SetWatchedBroadcast
# 174 "data/scripts/players_house.pory"
	end
# 175 "data/scripts/players_house.pory"

# 176 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_PetalburgGymReportFemale::
# 177 "data/scripts/players_house.pory"
	applymovement VAR_0x8005, Common_Movement_WalkInPlaceFasterLeft
# 178 "data/scripts/players_house.pory"
	waitmovement 0
# 179 "data/scripts/players_house.pory"
	call PlayersHouse_1F_EventScript_MomNoticeGymBroadcast
# 180 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, PlayersHouse_1F_Movement_PlayerApproachTVForGymFemale
# 181 "data/scripts/players_house.pory"
	waitmovement 0
# 182 "data/scripts/players_house.pory"
	playbgm MUS_ENCOUNTER_INTERVIEWER, FALSE
# 183 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_MaybeDadWillBeOn, MSGBOX_DEFAULT
# 184 "data/scripts/players_house.pory"
	closemessage
# 185 "data/scripts/players_house.pory"
	applymovement VAR_0x8005, PlayersHouse_1F_Movement_MomMakeRoomToSeeTVFemale
# 186 "data/scripts/players_house.pory"
	waitmovement 0
# 187 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, PlayersHouse_1F_Movement_PlayerMoveToTVFemale
# 188 "data/scripts/players_house.pory"
	waitmovement 0
# 189 "data/scripts/players_house.pory"
	call PlayersHouse_1F_EventScript_WatchGymBroadcast
# 190 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFasterRight
# 191 "data/scripts/players_house.pory"
	waitmovement 0
# 192 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_ItsOverWeMissedHim, MSGBOX_DEFAULT
# 193 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_GoIntroduceYourselfNextDoor, MSGBOX_DEFAULT
# 194 "data/scripts/players_house.pory"
	closemessage
# 195 "data/scripts/players_house.pory"
	setvar VAR_TEMP_1, 1
# 196 "data/scripts/players_house.pory"
	applymovement VAR_0x8005, PlayersHouse_1F_Movement_MomReturnToSeatFemale
# 197 "data/scripts/players_house.pory"
	waitmovement 0
# 198 "data/scripts/players_house.pory"
	goto PlayersHouse_1F_EventScript_SetWatchedBroadcast
# 199 "data/scripts/players_house.pory"
	end
# 200 "data/scripts/players_house.pory"

# 201 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_MomNoticeGymBroadcast::
# 202 "data/scripts/players_house.pory"
	playse SE_PIN
# 203 "data/scripts/players_house.pory"
	applymovement VAR_0x8005, Common_Movement_ExclamationMark
# 204 "data/scripts/players_house.pory"
	waitmovement 0
# 205 "data/scripts/players_house.pory"
	applymovement VAR_0x8005, Common_Movement_Delay48
# 206 "data/scripts/players_house.pory"
	waitmovement 0
# 207 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_OhComeQuickly, MSGBOX_DEFAULT
# 208 "data/scripts/players_house.pory"
	closemessage
# 209 "data/scripts/players_house.pory"
	return
# 210 "data/scripts/players_house.pory"

# 211 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_WatchGymBroadcast::
# 212 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFasterUp
# 213 "data/scripts/players_house.pory"
	waitmovement 0
# 214 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_ReportFromPetalburgGym, MSGBOX_DEFAULT
# 215 "data/scripts/players_house.pory"
	fadedefaultbgm
# 216 "data/scripts/players_house.pory"
	special TurnOffTVScreen
# 217 "data/scripts/players_house.pory"
	setflag FLAG_SYS_TV_HOME
# 218 "data/scripts/players_house.pory"
	delay 35
# 219 "data/scripts/players_house.pory"
	return
# 220 "data/scripts/players_house.pory"

# 221 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomApproachDadMale:
# 222 "data/scripts/players_house.pory"
	walk_up
# 223 "data/scripts/players_house.pory"
	walk_right
# 224 "data/scripts/players_house.pory"
	walk_right
# 225 "data/scripts/players_house.pory"
	walk_right
# 226 "data/scripts/players_house.pory"
	walk_right
# 227 "data/scripts/players_house.pory"
	walk_down
# 228 "data/scripts/players_house.pory"
	walk_in_place_faster_right
# 229 "data/scripts/players_house.pory"
	step_end
# 230 "data/scripts/players_house.pory"

# 231 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomApproachDadFemale:
# 232 "data/scripts/players_house.pory"
	walk_up
# 233 "data/scripts/players_house.pory"
	walk_left
# 234 "data/scripts/players_house.pory"
	walk_left
# 235 "data/scripts/players_house.pory"
	walk_left
# 236 "data/scripts/players_house.pory"
	walk_left
# 237 "data/scripts/players_house.pory"
	walk_down
# 238 "data/scripts/players_house.pory"
	walk_in_place_faster_left
# 239 "data/scripts/players_house.pory"
	step_end
# 240 "data/scripts/players_house.pory"

# 241 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomApproachPlayerMale:
# 242 "data/scripts/players_house.pory"
	walk_right
# 243 "data/scripts/players_house.pory"
	step_end
# 244 "data/scripts/players_house.pory"

# 245 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomApproachPlayerFemale:
# 246 "data/scripts/players_house.pory"
	walk_left
# 247 "data/scripts/players_house.pory"
	step_end
# 248 "data/scripts/players_house.pory"

# 249 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomNoticesLatiBroadcastMale:
# 250 "data/scripts/players_house.pory"
	walk_in_place_faster_left
# 251 "data/scripts/players_house.pory"
	delay_16
# 252 "data/scripts/players_house.pory"
	delay_16
# 253 "data/scripts/players_house.pory"
	delay_16
# 254 "data/scripts/players_house.pory"
	delay_16
# 255 "data/scripts/players_house.pory"
	walk_in_place_faster_right
# 256 "data/scripts/players_house.pory"
	delay_16
# 257 "data/scripts/players_house.pory"
	delay_16
# 258 "data/scripts/players_house.pory"
	step_end
# 259 "data/scripts/players_house.pory"

# 260 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomNoticesLatiBroadcastFemale:
# 261 "data/scripts/players_house.pory"
	walk_in_place_faster_right
# 262 "data/scripts/players_house.pory"
	delay_16
# 263 "data/scripts/players_house.pory"
	delay_16
# 264 "data/scripts/players_house.pory"
	delay_16
# 265 "data/scripts/players_house.pory"
	delay_16
# 266 "data/scripts/players_house.pory"
	walk_in_place_faster_left
# 267 "data/scripts/players_house.pory"
	delay_16
# 268 "data/scripts/players_house.pory"
	delay_16
# 269 "data/scripts/players_house.pory"
	step_end
# 270 "data/scripts/players_house.pory"

# 271 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomApproachPlayerAfterTVMale:
# 272 "data/scripts/players_house.pory"
	walk_up
# 273 "data/scripts/players_house.pory"
	walk_left
# 274 "data/scripts/players_house.pory"
	walk_left
# 275 "data/scripts/players_house.pory"
	step_end
# 276 "data/scripts/players_house.pory"

# 277 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomApproachPlayerAfterTVFemale:
# 278 "data/scripts/players_house.pory"
	walk_up
# 279 "data/scripts/players_house.pory"
	walk_right
# 280 "data/scripts/players_house.pory"
	walk_right
# 281 "data/scripts/players_house.pory"
	step_end
# 282 "data/scripts/players_house.pory"

# 283 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomMakeRoomToSeeTVMale:
# 284 "data/scripts/players_house.pory"
	walk_left
# 285 "data/scripts/players_house.pory"
	walk_in_place_faster_right
# 286 "data/scripts/players_house.pory"
	step_end
# 287 "data/scripts/players_house.pory"

# 288 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomMakeRoomToSeeTVFemale:
# 289 "data/scripts/players_house.pory"
	walk_right
# 290 "data/scripts/players_house.pory"
	walk_in_place_faster_left
# 291 "data/scripts/players_house.pory"
	step_end
# 292 "data/scripts/players_house.pory"

# 293 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomReturnToSeatMale:
# 294 "data/scripts/players_house.pory"
	walk_left
# 295 "data/scripts/players_house.pory"
	walk_down
# 296 "data/scripts/players_house.pory"
	walk_in_place_faster_right
# 297 "data/scripts/players_house.pory"
	step_end
# 298 "data/scripts/players_house.pory"

# 299 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomReturnToSeatFemale:
# 300 "data/scripts/players_house.pory"
	walk_right
# 301 "data/scripts/players_house.pory"
	walk_down
# 302 "data/scripts/players_house.pory"
	walk_in_place_faster_left
# 303 "data/scripts/players_house.pory"
	step_end
# 304 "data/scripts/players_house.pory"

# 305 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_Mom::
# 306 "data/scripts/players_house.pory"
	lock
# 307 "data/scripts/players_house.pory"
	faceplayer
# 308 "data/scripts/players_house.pory"
	goto_if_eq VAR_LITTLEROOT_HOUSES_STATE_MAY, 4, PlayersHouse_1F_EventScript_DontPushYourselfTooHard
# 309 "data/scripts/players_house.pory"
	goto_if_eq VAR_LITTLEROOT_HOUSES_STATE_BRENDAN, 4, PlayersHouse_1F_EventScript_DontPushYourselfTooHard
# 310 "data/scripts/players_house.pory"
	goto_if_set FLAG_HAS_MATCH_CALL, PlayersHouse_1F_EventScript_TryRegisterMom
# 311 "data/scripts/players_house.pory"
	goto_if_set FLAG_RESCUED_BIRCH, PlayersHouse_1F_EventScript_MomHealsParty
# 312 "data/scripts/players_house.pory"
	goto_if_eq VAR_TEMP_1, 1, PlayersHouse_1F_EventScript_SeeYouHoney
# 313 "data/scripts/players_house.pory"
	goto_if_eq VAR_LITTLEROOT_INTRO_STATE, 7, PlayersHouse_1F_EventScript_DidYouMeetProfBirch
# 314 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_IsntItNiceInHere, MSGBOX_DEFAULT
# 315 "data/scripts/players_house.pory"
	release
# 316 "data/scripts/players_house.pory"
	end
# 317 "data/scripts/players_house.pory"

# 318 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_DontPushYourselfTooHard::
# 319 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_DontPushYourselfTooHard, MSGBOX_DEFAULT
# 320 "data/scripts/players_house.pory"
	release
# 321 "data/scripts/players_house.pory"
	end
# 322 "data/scripts/players_house.pory"

# 323 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_TryRegisterMom::
# 324 "data/scripts/players_house.pory"
	goto_if_set FLAG_ENABLE_MOM_MATCH_CALL, PlayersHouse_1F_EventScript_CheckGiveAmuletCoin
# 325 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_IsThatAPokenav, MSGBOX_DEFAULT
# 326 "data/scripts/players_house.pory"
	closemessage
# 327 "data/scripts/players_house.pory"
	delay 30
# 328 "data/scripts/players_house.pory"
	playfanfare MUS_REGISTER_MATCH_CALL
# 329 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_RegisteredMom, MSGBOX_DEFAULT
# 330 "data/scripts/players_house.pory"
	waitfanfare
# 331 "data/scripts/players_house.pory"
	closemessage
# 332 "data/scripts/players_house.pory"
	delay 30
# 333 "data/scripts/players_house.pory"
	setflag FLAG_ENABLE_MOM_MATCH_CALL
# 334 "data/scripts/players_house.pory"
	release
# 335 "data/scripts/players_house.pory"
	end
# 336 "data/scripts/players_house.pory"

# 337 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_CheckGiveAmuletCoin::
# 338 "data/scripts/players_house.pory"
	goto_if_set FLAG_BADGE05_GET, PlayersHouse_1F_EventScript_TryGiveAmuletCoin
# 339 "data/scripts/players_house.pory"
	goto PlayersHouse_1F_EventScript_MomHealsParty
# 340 "data/scripts/players_house.pory"

# 341 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_TryGiveAmuletCoin::
# 342 "data/scripts/players_house.pory"
	goto_if_set FLAG_RECEIVED_AMULET_COIN, PlayersHouse_1F_EventScript_MomHealsParty
# 343 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_GotDadsBadgeHeresSomethingFromMom, MSGBOX_DEFAULT
# 344 "data/scripts/players_house.pory"
	giveitem ITEM_AMULET_COIN
# 345 "data/scripts/players_house.pory"
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
# 346 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_DontPushYourselfTooHard, MSGBOX_DEFAULT
# 347 "data/scripts/players_house.pory"
	setflag FLAG_RECEIVED_AMULET_COIN
# 348 "data/scripts/players_house.pory"
	release
# 349 "data/scripts/players_house.pory"
	end
# 350 "data/scripts/players_house.pory"

# 351 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_MomHealsParty::
# 352 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_YouShouldRestABit, MSGBOX_DEFAULT
# 353 "data/scripts/players_house.pory"
	goto PlayersHouse_1F_EventScript_HealParty
# 354 "data/scripts/players_house.pory"
	end
# 355 "data/scripts/players_house.pory"

# 356 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_SeeYouHoney::
# 357 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_SeeYouHoney, MSGBOX_DEFAULT
# 358 "data/scripts/players_house.pory"
	release
# 359 "data/scripts/players_house.pory"
	end
# 360 "data/scripts/players_house.pory"

# 361 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_HealParty::
# 362 "data/scripts/players_house.pory"
	closemessage
# 363 "data/scripts/players_house.pory"
	call Common_EventScript_OutOfCenterPartyHeal
# 364 "data/scripts/players_house.pory"
	incrementgamestat GAME_STAT_RESTED_AT_HOME
# 365 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_TakeCareHoney, MSGBOX_DEFAULT
# 366 "data/scripts/players_house.pory"
	release
# 367 "data/scripts/players_house.pory"
	end
# 368 "data/scripts/players_house.pory"

# 369 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_DidYouMeetProfBirch::
# 370 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_DidYouMeetProfBirch, MSGBOX_DEFAULT
# 371 "data/scripts/players_house.pory"
	release
# 372 "data/scripts/players_house.pory"
	end
# 373 "data/scripts/players_house.pory"

# 374 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_Vigoroth1::
# 375 "data/scripts/players_house.pory"
	lock
# 376 "data/scripts/players_house.pory"
	faceplayer
# 377 "data/scripts/players_house.pory"
	waitse
# 378 "data/scripts/players_house.pory"
	playmoncry SPECIES_VIGOROTH, CRY_MODE_NORMAL
# 379 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_Vigoroth1, MSGBOX_DEFAULT
# 380 "data/scripts/players_house.pory"
	waitmoncry
# 381 "data/scripts/players_house.pory"
	release
# 382 "data/scripts/players_house.pory"
	end
# 383 "data/scripts/players_house.pory"

# 384 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_Vigoroth2::
# 385 "data/scripts/players_house.pory"
	lock
# 386 "data/scripts/players_house.pory"
	faceplayer
# 387 "data/scripts/players_house.pory"
	waitse
# 388 "data/scripts/players_house.pory"
	playmoncry SPECIES_VIGOROTH, CRY_MODE_NORMAL
# 389 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_Vigoroth2, MSGBOX_DEFAULT
# 390 "data/scripts/players_house.pory"
	waitmoncry
# 391 "data/scripts/players_house.pory"
	release
# 392 "data/scripts/players_house.pory"
	end
# 393 "data/scripts/players_house.pory"

# 394 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_PlayerApproachTVForGymMale:
# 395 "data/scripts/players_house.pory"
	walk_down
# 396 "data/scripts/players_house.pory"
	walk_down
# 397 "data/scripts/players_house.pory"
	walk_left
# 398 "data/scripts/players_house.pory"
	walk_left
# 399 "data/scripts/players_house.pory"
	walk_left
# 400 "data/scripts/players_house.pory"
	step_end
# 401 "data/scripts/players_house.pory"

# 402 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_PlayerMoveToTVMale:
# 403 "data/scripts/players_house.pory"
	walk_left
# 404 "data/scripts/players_house.pory"
	step_end
# 405 "data/scripts/players_house.pory"

# 406 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_PlayerApproachTVForGymFemale:
# 407 "data/scripts/players_house.pory"
	walk_down
# 408 "data/scripts/players_house.pory"
	walk_down
# 409 "data/scripts/players_house.pory"
	walk_right
# 410 "data/scripts/players_house.pory"
	walk_right
# 411 "data/scripts/players_house.pory"
	walk_right
# 412 "data/scripts/players_house.pory"
	step_end
# 413 "data/scripts/players_house.pory"

# 414 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_PlayerMoveToTVFemale:
# 415 "data/scripts/players_house.pory"
	walk_right
# 416 "data/scripts/players_house.pory"
	step_end
# 417 "data/scripts/players_house.pory"

# 418 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MovePlayerAwayFromDoor:
# 419 "data/scripts/players_house.pory"
	walk_up
# 420 "data/scripts/players_house.pory"
	step_end
# 421 "data/scripts/players_house.pory"

# 422 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_GetSSTicketAndSeeLatiTV::
# 423 "data/scripts/players_house.pory"
	lockall
# 424 "data/scripts/players_house.pory"
	checkplayergender
# 425 "data/scripts/players_house.pory"
	call_if_eq VAR_RESULT, MALE, PlayersHouse_1F_EventScript_SetUpObjectEventVarsMale
# 426 "data/scripts/players_house.pory"
	call_if_eq VAR_RESULT, FEMALE, PlayersHouse_1F_EventScript_SetUpObjectEventVarsFemale
# 427 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, MALE, PlayersHouse_1F_EventScript_PlayerEnterRoomMale
# 428 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, FEMALE, PlayersHouse_1F_EventScript_PlayerEnterRoomFemale
# 429 "data/scripts/players_house.pory"
	applymovement VAR_0x8009, Common_Movement_FacePlayer
# 430 "data/scripts/players_house.pory"
	waitmovement 0
# 431 "data/scripts/players_house.pory"
	playse SE_PIN
# 432 "data/scripts/players_house.pory"
	applymovement VAR_0x8009, Common_Movement_ExclamationMark
# 433 "data/scripts/players_house.pory"
	waitmovement 0
# 434 "data/scripts/players_house.pory"
	applymovement VAR_0x8009, Common_Movement_Delay48
# 435 "data/scripts/players_house.pory"
	waitmovement 0
# 436 "data/scripts/players_house.pory"
	delay 20
# 437 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, MALE, PlayersHouse_1F_EventScript_DadApproachPlayerMale
# 438 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, FEMALE, PlayersHouse_1F_EventScript_DadApproachPlayerFemale
# 439 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_TicketFromBrineyCameForYou, MSGBOX_DEFAULT
# 440 "data/scripts/players_house.pory"
	giveitem ITEM_SS_TICKET
# 441 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_PortsInSlateportLilycove, MSGBOX_DEFAULT
# 442 "data/scripts/players_house.pory"
	closemessage
# 443 "data/scripts/players_house.pory"
	delay 20
# 444 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, MALE, PlayersHouse_1F_EventScript_MomApproachDadMale
# 445 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, FEMALE, PlayersHouse_1F_EventScript_MomApproachDadFemale
# 446 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_BetterGetBackToGym, MSGBOX_DEFAULT
# 447 "data/scripts/players_house.pory"
	closemessage
# 448 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, MALE, PlayersHouse_1F_EventScript_DadExitsMale
# 449 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, FEMALE, PlayersHouse_1F_EventScript_DadExitsFemale
# 450 "data/scripts/players_house.pory"
	playse SE_DOOR
# 451 "data/scripts/players_house.pory"
	removeobject VAR_0x8009
# 452 "data/scripts/players_house.pory"
	setflag FLAG_RECEIVED_SS_TICKET
# 453 "data/scripts/players_house.pory"
	delay 30
# 454 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, MALE, PlayersHouse_1F_EventScript_MomApproachPlayerMale
# 455 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, FEMALE, PlayersHouse_1F_EventScript_MomApproachPlayerFemale
# 456 "data/scripts/players_house.pory"
	delay 20
# 457 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_DadShouldStayLonger, MSGBOX_DEFAULT
# 458 "data/scripts/players_house.pory"
	closemessage
# 459 "data/scripts/players_house.pory"
	setflag FLAG_SYS_TV_LATIAS_LATIOS
# 460 "data/scripts/players_house.pory"
	special TurnOnTVScreen
# 461 "data/scripts/players_house.pory"
	delay 60
# 462 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, MALE, PlayersHouse_1F_EventScript_MomNoticesLatiBroadcastMale
# 463 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, FEMALE, PlayersHouse_1F_EventScript_MomNoticesLatiBroadcastFemale
# 464 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_IsThatABreakingStory, MSGBOX_DEFAULT
# 465 "data/scripts/players_house.pory"
	closemessage
# 466 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, MALE, PlayersHouse_1F_EventScript_PlayerApproachTVForLatiMale
# 467 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, FEMALE, PlayersHouse_1F_EventScript_PlayerApproachTVForLatiFemale
# 468 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_LatiEmergencyNewsFlash, MSGBOX_DEFAULT
# 469 "data/scripts/players_house.pory"
	closemessage
# 470 "data/scripts/players_house.pory"
	clearflag FLAG_SYS_TV_LATIAS_LATIOS
# 471 "data/scripts/players_house.pory"
	setflag FLAG_LATIOS_OR_LATIAS_ROAMING
# 472 "data/scripts/players_house.pory"
	special TurnOffTVScreen
# 473 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, MALE, PlayersHouse_1F_EventScript_MomApproachPlayerAfterTVMale
# 474 "data/scripts/players_house.pory"
	call_if_eq VAR_0x8008, FEMALE, PlayersHouse_1F_EventScript_MomApproachPlayerAfterTVFemale
# 475 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_WhatColorDidTheySay, MSGBOX_DEFAULT
# 476 "data/scripts/players_house.pory"
	multichoice 22, 8, MULTI_TV_LATI, TRUE
# 477 "data/scripts/players_house.pory"
	copyvar VAR_0x8004, VAR_RESULT
# 478 "data/scripts/players_house.pory"
	special InitRoamer
# 479 "data/scripts/players_house.pory"
	copyvar VAR_ROAMER_POKEMON, VAR_RESULT
# 480 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_StillUnknownPokemon, MSGBOX_DEFAULT
# 481 "data/scripts/players_house.pory"
	closemessage
# 482 "data/scripts/players_house.pory"
	setvar VAR_LITTLEROOT_HOUSES_STATE_MAY, 4
# 483 "data/scripts/players_house.pory"
	setvar VAR_LITTLEROOT_HOUSES_STATE_BRENDAN, 4
# 484 "data/scripts/players_house.pory"
	releaseall
# 485 "data/scripts/players_house.pory"
	end
# 486 "data/scripts/players_house.pory"

# 487 "data/scripts/players_house.pory"
@ Never called.
# 488 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_AirLatiBroadcast::
# 489 "data/scripts/players_house.pory"
	setflag FLAG_SYS_TV_LATIAS_LATIOS
# 490 "data/scripts/players_house.pory"
	return
# 491 "data/scripts/players_house.pory"

# 492 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_SetUpObjectEventVarsMale::
# 493 "data/scripts/players_house.pory"
	setvar VAR_0x8008, MALE
# 494 "data/scripts/players_house.pory"
	setvar VAR_0x8009, LOCALID_DAD
# 495 "data/scripts/players_house.pory"
	setvar VAR_0x800A, LOCALID_MOM
# 496 "data/scripts/players_house.pory"
	return
# 497 "data/scripts/players_house.pory"

# 498 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_SetUpObjectEventVarsFemale::
# 499 "data/scripts/players_house.pory"
	setvar VAR_0x8008, FEMALE
# 500 "data/scripts/players_house.pory"
	setvar VAR_0x8009, LOCALID_DAD
# 501 "data/scripts/players_house.pory"
	setvar VAR_0x800A, LOCALID_MOM
# 502 "data/scripts/players_house.pory"
	return
# 503 "data/scripts/players_house.pory"

# 504 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_DadApproachPlayerMale::
# 505 "data/scripts/players_house.pory"
	applymovement VAR_0x8009, PlayersHouse_1F_Movement_DadApproachPlayerMale
# 506 "data/scripts/players_house.pory"
	waitmovement 0
# 507 "data/scripts/players_house.pory"
	return
# 508 "data/scripts/players_house.pory"

# 509 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_DadApproachPlayerFemale::
# 510 "data/scripts/players_house.pory"
	applymovement VAR_0x8009, PlayersHouse_1F_Movement_DadApproachPlayerFemale
# 511 "data/scripts/players_house.pory"
	waitmovement 0
# 512 "data/scripts/players_house.pory"
	return
# 513 "data/scripts/players_house.pory"

# 514 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_MomApproachDadMale::
# 515 "data/scripts/players_house.pory"
	applymovement VAR_0x800A, PlayersHouse_1F_Movement_MomApproachDadMale
# 516 "data/scripts/players_house.pory"
	waitmovement 0
# 517 "data/scripts/players_house.pory"
	applymovement VAR_0x8009, PlayersHouse_1F_Movement_DadFaceMomMale
# 518 "data/scripts/players_house.pory"
	waitmovement 0
# 519 "data/scripts/players_house.pory"
	return
# 520 "data/scripts/players_house.pory"

# 521 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_MomApproachDadFemale::
# 522 "data/scripts/players_house.pory"
	applymovement VAR_0x800A, PlayersHouse_1F_Movement_MomApproachDadFemale
# 523 "data/scripts/players_house.pory"
	waitmovement 0
# 524 "data/scripts/players_house.pory"
	applymovement VAR_0x8009, PlayersHouse_1F_Movement_DadFaceMomFemale
# 525 "data/scripts/players_house.pory"
	waitmovement 0
# 526 "data/scripts/players_house.pory"
	return
# 527 "data/scripts/players_house.pory"

# 528 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_DadExitsMale::
# 529 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, PlayersHouse_1F_Movement_MomAndPlayerWatchDadExit
# 530 "data/scripts/players_house.pory"
	applymovement VAR_0x800A, PlayersHouse_1F_Movement_MomAndPlayerWatchDadExit
# 531 "data/scripts/players_house.pory"
	applymovement VAR_0x8009, PlayersHouse_1F_Movement_DadExitsMale
# 532 "data/scripts/players_house.pory"
	waitmovement 0
# 533 "data/scripts/players_house.pory"
	return
# 534 "data/scripts/players_house.pory"

# 535 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_DadExitsFemale::
# 536 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, PlayersHouse_1F_Movement_MomAndPlayerWatchDadExit
# 537 "data/scripts/players_house.pory"
	applymovement VAR_0x800A, PlayersHouse_1F_Movement_MomAndPlayerWatchDadExit
# 538 "data/scripts/players_house.pory"
	applymovement VAR_0x8009, PlayersHouse_1F_Movement_DadExitsFemale
# 539 "data/scripts/players_house.pory"
	waitmovement 0
# 540 "data/scripts/players_house.pory"
	return
# 541 "data/scripts/players_house.pory"

# 542 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_PlayerEnterRoomMale::
# 543 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, PlayersHouse_1F_Movement_PlayerEnterRoomMale
# 544 "data/scripts/players_house.pory"
	waitmovement 0
# 545 "data/scripts/players_house.pory"
	return
# 546 "data/scripts/players_house.pory"

# 547 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_PlayerEnterRoomFemale::
# 548 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, PlayersHouse_1F_Movement_PlayerEnterRoomFemale
# 549 "data/scripts/players_house.pory"
	waitmovement 0
# 550 "data/scripts/players_house.pory"
	return
# 551 "data/scripts/players_house.pory"

# 552 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_PlayerApproachTVForLatiMale::
# 553 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, PlayersHouse_1F_Movement_PlayerApproachTVForLatiMale
# 554 "data/scripts/players_house.pory"
	waitmovement 0
# 555 "data/scripts/players_house.pory"
	applymovement VAR_0x800A, Common_Movement_WalkInPlaceFasterLeft
# 556 "data/scripts/players_house.pory"
	waitmovement 0
# 557 "data/scripts/players_house.pory"
	return
# 558 "data/scripts/players_house.pory"

# 559 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_PlayerApproachTVForLatiFemale::
# 560 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, PlayersHouse_1F_Movement_PlayerApproachTVForLatiFemale
# 561 "data/scripts/players_house.pory"
	waitmovement 0
# 562 "data/scripts/players_house.pory"
	applymovement VAR_0x800A, Common_Movement_WalkInPlaceFasterRight
# 563 "data/scripts/players_house.pory"
	waitmovement 0
# 564 "data/scripts/players_house.pory"
	return
# 565 "data/scripts/players_house.pory"

# 566 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_MomApproachPlayerMale::
# 567 "data/scripts/players_house.pory"
	applymovement VAR_0x800A, PlayersHouse_1F_Movement_MomApproachPlayerMale
# 568 "data/scripts/players_house.pory"
	waitmovement 0
# 569 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
# 570 "data/scripts/players_house.pory"
	waitmovement 0
# 571 "data/scripts/players_house.pory"
	return
# 572 "data/scripts/players_house.pory"

# 573 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_MomApproachPlayerFemale::
# 574 "data/scripts/players_house.pory"
	applymovement VAR_0x800A, PlayersHouse_1F_Movement_MomApproachPlayerFemale
# 575 "data/scripts/players_house.pory"
	waitmovement 0
# 576 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFasterRight
# 577 "data/scripts/players_house.pory"
	waitmovement 0
# 578 "data/scripts/players_house.pory"
	return
# 579 "data/scripts/players_house.pory"

# 580 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_MomNoticesLatiBroadcastMale::
# 581 "data/scripts/players_house.pory"
	applymovement VAR_0x800A, PlayersHouse_1F_Movement_MomNoticesLatiBroadcastMale
# 582 "data/scripts/players_house.pory"
	waitmovement 0
# 583 "data/scripts/players_house.pory"
	return
# 584 "data/scripts/players_house.pory"

# 585 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_MomNoticesLatiBroadcastFemale::
# 586 "data/scripts/players_house.pory"
	applymovement VAR_0x800A, PlayersHouse_1F_Movement_MomNoticesLatiBroadcastFemale
# 587 "data/scripts/players_house.pory"
	waitmovement 0
# 588 "data/scripts/players_house.pory"
	return
# 589 "data/scripts/players_house.pory"

# 590 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_MomApproachPlayerAfterTVMale::
# 591 "data/scripts/players_house.pory"
	applymovement VAR_0x800A, PlayersHouse_1F_Movement_MomApproachPlayerAfterTVMale
# 592 "data/scripts/players_house.pory"
	waitmovement 0
# 593 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFasterRight
# 594 "data/scripts/players_house.pory"
	waitmovement 0
# 595 "data/scripts/players_house.pory"
	return
# 596 "data/scripts/players_house.pory"

# 597 "data/scripts/players_house.pory"
PlayersHouse_1F_EventScript_MomApproachPlayerAfterTVFemale::
# 598 "data/scripts/players_house.pory"
	applymovement VAR_0x800A, PlayersHouse_1F_Movement_MomApproachPlayerAfterTVFemale
# 599 "data/scripts/players_house.pory"
	waitmovement 0
# 600 "data/scripts/players_house.pory"
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
# 601 "data/scripts/players_house.pory"
	waitmovement 0
# 602 "data/scripts/players_house.pory"
	return
# 603 "data/scripts/players_house.pory"

# 604 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_DadApproachPlayerMale:
# 605 "data/scripts/players_house.pory"
	walk_right
# 606 "data/scripts/players_house.pory"
	walk_right
# 607 "data/scripts/players_house.pory"
	step_end
# 608 "data/scripts/players_house.pory"

# 609 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_DadApproachPlayerFemale:
# 610 "data/scripts/players_house.pory"
	walk_left
# 611 "data/scripts/players_house.pory"
	walk_left
# 612 "data/scripts/players_house.pory"
	step_end
# 613 "data/scripts/players_house.pory"

# 614 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_DadFaceMomMale:
# 615 "data/scripts/players_house.pory"
	face_left
# 616 "data/scripts/players_house.pory"
	step_end
# 617 "data/scripts/players_house.pory"

# 618 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_DadFaceMomFemale:
# 619 "data/scripts/players_house.pory"
	face_right
# 620 "data/scripts/players_house.pory"
	step_end
# 621 "data/scripts/players_house.pory"

# 622 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_DadExitsMale:
# 623 "data/scripts/players_house.pory"
	walk_down
# 624 "data/scripts/players_house.pory"
	walk_right
# 625 "data/scripts/players_house.pory"
	walk_down
# 626 "data/scripts/players_house.pory"
	delay_8
# 627 "data/scripts/players_house.pory"
	step_end
# 628 "data/scripts/players_house.pory"

# 629 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_DadExitsFemale:
# 630 "data/scripts/players_house.pory"
	walk_down
# 631 "data/scripts/players_house.pory"
	walk_left
# 632 "data/scripts/players_house.pory"
	walk_down
# 633 "data/scripts/players_house.pory"
	delay_8
# 634 "data/scripts/players_house.pory"
	step_end
# 635 "data/scripts/players_house.pory"

# 636 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_PlayerEnterRoomMale:
# 637 "data/scripts/players_house.pory"
	delay_16
# 638 "data/scripts/players_house.pory"
	walk_down
# 639 "data/scripts/players_house.pory"
	walk_down
# 640 "data/scripts/players_house.pory"
	walk_down
# 641 "data/scripts/players_house.pory"
	walk_in_place_faster_left
# 642 "data/scripts/players_house.pory"
	step_end
# 643 "data/scripts/players_house.pory"

# 644 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_MomAndPlayerWatchDadExit:
# 645 "data/scripts/players_house.pory"
	delay_8
# 646 "data/scripts/players_house.pory"
	delay_16
# 647 "data/scripts/players_house.pory"
	delay_16
# 648 "data/scripts/players_house.pory"
	walk_in_place_faster_down
# 649 "data/scripts/players_house.pory"
	step_end
# 650 "data/scripts/players_house.pory"

# 651 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_PlayerEnterRoomFemale:
# 652 "data/scripts/players_house.pory"
	delay_16
# 653 "data/scripts/players_house.pory"
	walk_down
# 654 "data/scripts/players_house.pory"
	walk_down
# 655 "data/scripts/players_house.pory"
	walk_down
# 656 "data/scripts/players_house.pory"
	walk_in_place_faster_right
# 657 "data/scripts/players_house.pory"
	step_end
# 658 "data/scripts/players_house.pory"

# 659 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_PlayerApproachTVForLatiMale:
# 660 "data/scripts/players_house.pory"
	walk_up
# 661 "data/scripts/players_house.pory"
	walk_left
# 662 "data/scripts/players_house.pory"
	walk_left
# 663 "data/scripts/players_house.pory"
	walk_left
# 664 "data/scripts/players_house.pory"
	walk_left
# 665 "data/scripts/players_house.pory"
	walk_in_place_faster_up
# 666 "data/scripts/players_house.pory"
	step_end
# 667 "data/scripts/players_house.pory"

# 668 "data/scripts/players_house.pory"
PlayersHouse_1F_Movement_PlayerApproachTVForLatiFemale:
# 669 "data/scripts/players_house.pory"
	walk_up
# 670 "data/scripts/players_house.pory"
	walk_right
# 671 "data/scripts/players_house.pory"
	walk_right
# 672 "data/scripts/players_house.pory"
	walk_right
# 673 "data/scripts/players_house.pory"
	walk_right
# 674 "data/scripts/players_house.pory"
	walk_in_place_faster_up
# 675 "data/scripts/players_house.pory"
	step_end
# 676 "data/scripts/players_house.pory"

# 677 "data/scripts/players_house.pory"
EventScript_RunningShoesManual::
# 678 "data/scripts/players_house.pory"
	msgbox PlayersHouse_1F_Text_RunningShoesManual, MSGBOX_SIGN
# 679 "data/scripts/players_house.pory"
	end
