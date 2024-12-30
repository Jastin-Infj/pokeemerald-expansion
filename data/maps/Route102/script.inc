# 1 "data/maps/Route102/script.pory"

# 2 "data/maps/Route102/script.pory"
Route102_MapScripts::
# 3 "data/maps/Route102/script.pory"
	.byte 0
# 4 "data/maps/Route102/script.pory"

# 5 "data/maps/Route102/script.pory"
Route102_EventScript_LittleBoy::
# 6 "data/maps/Route102/script.pory"
	msgbox Route102_Text_ImNotVeryTall, MSGBOX_NPC
# 7 "data/maps/Route102/script.pory"
	end
# 8 "data/maps/Route102/script.pory"

# 9 "data/maps/Route102/script.pory"
Route102_EventScript_RouteSignOldale::
# 10 "data/maps/Route102/script.pory"
	msgbox Route102_Text_RouteSignOldale, MSGBOX_SIGN
# 11 "data/maps/Route102/script.pory"
	end
# 12 "data/maps/Route102/script.pory"

# 13 "data/maps/Route102/script.pory"
Route102_EventScript_RouteSignPetalburg::
# 14 "data/maps/Route102/script.pory"
	msgbox Route102_Text_RouteSignPetalburg, MSGBOX_SIGN
# 15 "data/maps/Route102/script.pory"
	end
# 16 "data/maps/Route102/script.pory"

# 17 "data/maps/Route102/script.pory"
Route102_EventScript_Boy::
# 18 "data/maps/Route102/script.pory"
	msgbox Route102_Text_CatchWholeBunchOfPokemon, MSGBOX_NPC
# 19 "data/maps/Route102/script.pory"
	end
# 20 "data/maps/Route102/script.pory"

# 21 "data/maps/Route102/script.pory"
Route102_EventScript_Calvin::
# 22 "data/maps/Route102/script.pory"
	release
# 23 "data/maps/Route102/script.pory"
	end
# 24 "data/maps/Route102/script.pory"

# 25 "data/maps/Route102/script.pory"
Route102_EventScript_CalvinRegisterMatchCallAfterBattle::
# 26 "data/maps/Route102/script.pory"
	# special PlayerFaceTrainerAfterBattle
# 27 "data/maps/Route102/script.pory"
	# waitmovement 0
# 28 "data/maps/Route102/script.pory"
	# goto_if_set FLAG_HAS_MATCH_CALL, Route102_EventScript_CalvinRegisterMatchCall
# 29 "data/maps/Route102/script.pory"
	release
# 30 "data/maps/Route102/script.pory"
	end
# 31 "data/maps/Route102/script.pory"

# 32 "data/maps/Route102/script.pory"
Route102_EventScript_CalvinRegisterMatchCall::
# 33 "data/maps/Route102/script.pory"
	# msgbox Route102_Text_CalvinRegisterShort, MSGBOX_DEFAULT
# 34 "data/maps/Route102/script.pory"
	# register_matchcall TRAINER_CALVIN_1
# 35 "data/maps/Route102/script.pory"
	release
# 36 "data/maps/Route102/script.pory"
	end
# 37 "data/maps/Route102/script.pory"

# 38 "data/maps/Route102/script.pory"
Route102_EventScript_CalvinTryRegister::
# 39 "data/maps/Route102/script.pory"
	# goto_if_set FLAG_HAS_MATCH_CALL, Route102_EventScript_CalvinRegister
# 40 "data/maps/Route102/script.pory"
	# msgbox Route102_Text_CalvinPostBattle, MSGBOX_DEFAULT
# 41 "data/maps/Route102/script.pory"
	release
# 42 "data/maps/Route102/script.pory"
	end
# 43 "data/maps/Route102/script.pory"

# 44 "data/maps/Route102/script.pory"
Route102_EventScript_CalvinRegister::
# 45 "data/maps/Route102/script.pory"
	# msgbox Route102_Text_CalvinRegister, MSGBOX_DEFAULT
# 46 "data/maps/Route102/script.pory"
	# register_matchcall TRAINER_CALVIN_1
# 47 "data/maps/Route102/script.pory"
	release
# 48 "data/maps/Route102/script.pory"
	end
# 49 "data/maps/Route102/script.pory"

# 50 "data/maps/Route102/script.pory"
Route102_EventScript_CalvinRematch::
# 51 "data/maps/Route102/script.pory"
	# trainerbattle_rematch TRAINER_CALVIN_1, Route102_Text_CalvinRematchIntro, Route102_Text_CalvinRematchDefeated
# 52 "data/maps/Route102/script.pory"
	# msgbox Route102_Text_CalvinRematchPostBattle, MSGBOX_AUTOCLOSE
# 53 "data/maps/Route102/script.pory"
	end
# 54 "data/maps/Route102/script.pory"

# 55 "data/maps/Route102/script.pory"
Route102_EventScript_Rick::
# 56 "data/maps/Route102/script.pory"
	trainerbattle_single TRAINER_RICK, Route102_Text_RickIntro, Route102_Text_RickDefeated
# 57 "data/maps/Route102/script.pory"
	msgbox Route102_Text_RickPostBattle, MSGBOX_AUTOCLOSE
# 58 "data/maps/Route102/script.pory"
	end
# 59 "data/maps/Route102/script.pory"

# 60 "data/maps/Route102/script.pory"
Route102_EventScript_Tiana::
# 61 "data/maps/Route102/script.pory"
	trainerbattle_single TRAINER_TIANA, Route102_Text_TianaIntro, Route102_Text_TianaDefeated
# 62 "data/maps/Route102/script.pory"
	msgbox Route102_Text_TianaPostBattle, MSGBOX_AUTOCLOSE
# 63 "data/maps/Route102/script.pory"
	end
# 64 "data/maps/Route102/script.pory"

# 65 "data/maps/Route102/script.pory"
Route102_EventScript_Allen::
# 66 "data/maps/Route102/script.pory"
	trainerbattle_single TRAINER_ALLEN, Route102_Text_AllenIntro, Route102_Text_AllenDefeated
# 67 "data/maps/Route102/script.pory"
	msgbox Route102_Text_AllenPostBattle, MSGBOX_AUTOCLOSE
# 68 "data/maps/Route102/script.pory"
	end
# 69 "data/maps/Route102/script.pory"

# 70 "data/maps/Route102/script.pory"
Route102_Text_WatchMeCatchPokemon:
# 71 "data/maps/Route102/script.pory"
	.string "WALLY: {PLAYER}…\n"
# 72 "data/maps/Route102/script.pory"
	.string "POKéMON hide in tall grass like this,\l"
# 73 "data/maps/Route102/script.pory"
	.string "don't they?\p"
# 74 "data/maps/Route102/script.pory"
	.string "Please watch me and see if I can\n"
# 75 "data/maps/Route102/script.pory"
	.string "catch one properly.\p"
# 76 "data/maps/Route102/script.pory"
	.string "…Whoa!$"
# 77 "data/maps/Route102/script.pory"

# 78 "data/maps/Route102/script.pory"
Route102_Text_WallyIDidIt:
# 79 "data/maps/Route102/script.pory"
	.string "WALLY: I did it… It's my…\n"
# 80 "data/maps/Route102/script.pory"
	.string "My POKéMON!$"
# 81 "data/maps/Route102/script.pory"

# 82 "data/maps/Route102/script.pory"
Route102_Text_LetsGoBack:
# 83 "data/maps/Route102/script.pory"
	.string "{PLAYER}, thank you!\n"
# 84 "data/maps/Route102/script.pory"
	.string "Let's go back to the GYM!$"
# 85 "data/maps/Route102/script.pory"

# 86 "data/maps/Route102/script.pory"
Route102_Text_ImNotVeryTall:
# 87 "data/maps/Route102/script.pory"
	.string "I'm…not very tall, so I sink right\n"
# 88 "data/maps/Route102/script.pory"
	.string "into tall grass.\p"
# 89 "data/maps/Route102/script.pory"
	.string "The grass goes up my nose and…\n"
# 90 "data/maps/Route102/script.pory"
	.string "Fwafwafwafwafwa…\p"
# 91 "data/maps/Route102/script.pory"
	.string "Fwatchoo!$"
# 92 "data/maps/Route102/script.pory"

# 93 "data/maps/Route102/script.pory"
Route102_Text_CatchWholeBunchOfPokemon:
# 94 "data/maps/Route102/script.pory"
	.string "I'm going to catch a whole bunch of\n"
# 95 "data/maps/Route102/script.pory"
	.string "POKéMON!$"
# 96 "data/maps/Route102/script.pory"

# 97 "data/maps/Route102/script.pory"
Route102_Text_RouteSignOldale:
# 98 "data/maps/Route102/script.pory"
	.string "ROUTE 102\n"
# 99 "data/maps/Route102/script.pory"
	.string "{RIGHT_ARROW} OLDALE TOWN$"
# 100 "data/maps/Route102/script.pory"

# 101 "data/maps/Route102/script.pory"
Route102_Text_RouteSignPetalburg:
# 102 "data/maps/Route102/script.pory"
	.string "ROUTE 102\n"
# 103 "data/maps/Route102/script.pory"
	.string "{LEFT_ARROW} PETALBURG CITY$"
