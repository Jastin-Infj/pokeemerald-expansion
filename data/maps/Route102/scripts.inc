# 1 "data/maps/Route102/scripts.pory"

# 2 "data/maps/Route102/scripts.pory"
Route102_MapScripts::
# 3 "data/maps/Route102/scripts.pory"
	.byte 0
# 4 "data/maps/Route102/scripts.pory"

# 5 "data/maps/Route102/scripts.pory"
Route102_EventScript_LittleBoy::
# 6 "data/maps/Route102/scripts.pory"
	msgbox Route102_Text_ImNotVeryTall, MSGBOX_NPC
# 7 "data/maps/Route102/scripts.pory"
	end
# 8 "data/maps/Route102/scripts.pory"

# 9 "data/maps/Route102/scripts.pory"
Route102_EventScript_RouteSignOldale::
# 10 "data/maps/Route102/scripts.pory"
	msgbox Route102_Text_RouteSignOldale, MSGBOX_SIGN
# 11 "data/maps/Route102/scripts.pory"
	end
# 12 "data/maps/Route102/scripts.pory"

# 13 "data/maps/Route102/scripts.pory"
Route102_EventScript_RouteSignPetalburg::
# 14 "data/maps/Route102/scripts.pory"
	msgbox Route102_Text_RouteSignPetalburg, MSGBOX_SIGN
# 15 "data/maps/Route102/scripts.pory"
	end
# 16 "data/maps/Route102/scripts.pory"

# 17 "data/maps/Route102/scripts.pory"
Route102_EventScript_Boy::
# 18 "data/maps/Route102/scripts.pory"
	msgbox Route102_Text_CatchWholeBunchOfPokemon, MSGBOX_NPC
# 19 "data/maps/Route102/scripts.pory"
	end
# 20 "data/maps/Route102/scripts.pory"

# 21 "data/maps/Route102/scripts.pory"
Route102_EventScript_Calvin::
# 22 "data/maps/Route102/scripts.pory"
 	lock
# 23 "data/maps/Route102/scripts.pory"
	giveitem ITEM_BLAZIKENITE
# 24 "data/maps/Route102/scripts.pory"
  giveitem ITEM_RARE_CANDY, 50
# 25 "data/maps/Route102/scripts.pory"
	giveitem ITEM_DYNAMAX_CANDY , 50
# 26 "data/maps/Route102/scripts.pory"
  giveitem ITEM_MEGA_RING
# 27 "data/maps/Route102/scripts.pory"
  giveitem ITEM_Z_POWER_RING
# 28 "data/maps/Route102/scripts.pory"
  giveitem ITEM_DYNAMAX_BAND
# 29 "data/maps/Route102/scripts.pory"
	setflag FLAG_ENABLE_TERA_BATTLE
# 30 "data/maps/Route102/scripts.pory"
	setflag FLAG_ENABLE_TERA_ORB_CHARGED
# 31 "data/maps/Route102/scripts.pory"
	giveitem ITEM_TERA_ORB
# 32 "data/maps/Route102/scripts.pory"
	release
# 33 "data/maps/Route102/scripts.pory"
	end
# 34 "data/maps/Route102/scripts.pory"

# 35 "data/maps/Route102/scripts.pory"
Route102_EventScript_CalvinRegisterMatchCallAfterBattle::
# 36 "data/maps/Route102/scripts.pory"
	special PlayerFaceTrainerAfterBattle
# 37 "data/maps/Route102/scripts.pory"
	waitmovement 0
# 38 "data/maps/Route102/scripts.pory"
	goto_if_set FLAG_HAS_MATCH_CALL, Route102_EventScript_CalvinRegisterMatchCall
# 39 "data/maps/Route102/scripts.pory"
	release
# 40 "data/maps/Route102/scripts.pory"
	end
# 41 "data/maps/Route102/scripts.pory"

# 42 "data/maps/Route102/scripts.pory"
Route102_EventScript_CalvinRegisterMatchCall::
# 43 "data/maps/Route102/scripts.pory"
	msgbox Route102_Text_CalvinRegisterShort, MSGBOX_DEFAULT
# 44 "data/maps/Route102/scripts.pory"
	register_matchcall TRAINER_CALVIN_1
# 45 "data/maps/Route102/scripts.pory"
	release
# 46 "data/maps/Route102/scripts.pory"
	end
# 47 "data/maps/Route102/scripts.pory"

# 48 "data/maps/Route102/scripts.pory"
Route102_EventScript_CalvinTryRegister::
# 49 "data/maps/Route102/scripts.pory"
	goto_if_set FLAG_HAS_MATCH_CALL, Route102_EventScript_CalvinRegister
# 50 "data/maps/Route102/scripts.pory"
	msgbox Route102_Text_CalvinPostBattle, MSGBOX_DEFAULT
# 51 "data/maps/Route102/scripts.pory"
	release
# 52 "data/maps/Route102/scripts.pory"
	end
# 53 "data/maps/Route102/scripts.pory"

# 54 "data/maps/Route102/scripts.pory"
Route102_EventScript_CalvinRegister::
# 55 "data/maps/Route102/scripts.pory"
	msgbox Route102_Text_CalvinRegister, MSGBOX_DEFAULT
# 56 "data/maps/Route102/scripts.pory"
	register_matchcall TRAINER_CALVIN_1
# 57 "data/maps/Route102/scripts.pory"
	release
# 58 "data/maps/Route102/scripts.pory"
	end
# 59 "data/maps/Route102/scripts.pory"

# 60 "data/maps/Route102/scripts.pory"
Route102_EventScript_CalvinRematch::
# 61 "data/maps/Route102/scripts.pory"
	trainerbattle_rematch TRAINER_CALVIN_1, Route102_Text_CalvinRematchIntro, Route102_Text_CalvinRematchDefeated
# 62 "data/maps/Route102/scripts.pory"
	msgbox Route102_Text_CalvinRematchPostBattle, MSGBOX_AUTOCLOSE
# 63 "data/maps/Route102/scripts.pory"
	end
# 64 "data/maps/Route102/scripts.pory"

# 65 "data/maps/Route102/scripts.pory"
Route102_EventScript_Rick::
# 66 "data/maps/Route102/scripts.pory"
	trainerbattle_single TRAINER_RICK, Route102_Text_RickIntro, Route102_Text_RickDefeated
# 67 "data/maps/Route102/scripts.pory"
	msgbox Route102_Text_RickPostBattle, MSGBOX_AUTOCLOSE
# 68 "data/maps/Route102/scripts.pory"
	end
# 69 "data/maps/Route102/scripts.pory"

# 70 "data/maps/Route102/scripts.pory"
Route102_EventScript_Tiana::
# 71 "data/maps/Route102/scripts.pory"
	trainerbattle_single TRAINER_TIANA, Route102_Text_TianaIntro, Route102_Text_TianaDefeated
# 72 "data/maps/Route102/scripts.pory"
	msgbox Route102_Text_TianaPostBattle, MSGBOX_AUTOCLOSE
# 73 "data/maps/Route102/scripts.pory"
	end
# 74 "data/maps/Route102/scripts.pory"

# 75 "data/maps/Route102/scripts.pory"
Route102_EventScript_Allen::
# 76 "data/maps/Route102/scripts.pory"
	trainerbattle_single TRAINER_ALLEN, Route102_Text_AllenIntro, Route102_Text_AllenDefeated
# 77 "data/maps/Route102/scripts.pory"
	msgbox Route102_Text_AllenPostBattle, MSGBOX_AUTOCLOSE
# 78 "data/maps/Route102/scripts.pory"
	end
# 79 "data/maps/Route102/scripts.pory"

# 80 "data/maps/Route102/scripts.pory"
Route102_Text_WatchMeCatchPokemon:
# 81 "data/maps/Route102/scripts.pory"
	.string "WALLY: {PLAYER}…\n"
# 82 "data/maps/Route102/scripts.pory"
	.string "POKéMON hide in tall grass like this,\l"
# 83 "data/maps/Route102/scripts.pory"
	.string "don't they?\p"
# 84 "data/maps/Route102/scripts.pory"
	.string "Please watch me and see if I can\n"
# 85 "data/maps/Route102/scripts.pory"
	.string "catch one properly.\p"
# 86 "data/maps/Route102/scripts.pory"
	.string "…Whoa!$"
# 87 "data/maps/Route102/scripts.pory"

# 88 "data/maps/Route102/scripts.pory"
Route102_Text_WallyIDidIt:
# 89 "data/maps/Route102/scripts.pory"
	.string "WALLY: I did it… It's my…\n"
# 90 "data/maps/Route102/scripts.pory"
	.string "My POKéMON!$"
# 91 "data/maps/Route102/scripts.pory"

# 92 "data/maps/Route102/scripts.pory"
Route102_Text_LetsGoBack:
# 93 "data/maps/Route102/scripts.pory"
	.string "{PLAYER}, thank you!\n"
# 94 "data/maps/Route102/scripts.pory"
	.string "Let's go back to the GYM!$"
# 95 "data/maps/Route102/scripts.pory"

# 96 "data/maps/Route102/scripts.pory"
Route102_Text_ImNotVeryTall:
# 97 "data/maps/Route102/scripts.pory"
	.string "I'm…not very tall, so I sink right\n"
# 98 "data/maps/Route102/scripts.pory"
	.string "into tall grass.\p"
# 99 "data/maps/Route102/scripts.pory"
	.string "The grass goes up my nose and…\n"
# 100 "data/maps/Route102/scripts.pory"
	.string "Fwafwafwafwafwa…\p"
# 101 "data/maps/Route102/scripts.pory"
	.string "Fwatchoo!$"
# 102 "data/maps/Route102/scripts.pory"

# 103 "data/maps/Route102/scripts.pory"
Route102_Text_CatchWholeBunchOfPokemon:
# 104 "data/maps/Route102/scripts.pory"
	.string "I'm going to catch a whole bunch of\n"
# 105 "data/maps/Route102/scripts.pory"
	.string "POKéMON!$"
# 106 "data/maps/Route102/scripts.pory"

# 107 "data/maps/Route102/scripts.pory"
Route102_Text_RouteSignOldale:
# 108 "data/maps/Route102/scripts.pory"
	.string "ROUTE 102\n"
# 109 "data/maps/Route102/scripts.pory"
	.string "{RIGHT_ARROW} OLDALE TOWN$"
# 110 "data/maps/Route102/scripts.pory"

# 111 "data/maps/Route102/scripts.pory"
Route102_Text_RouteSignPetalburg:
# 112 "data/maps/Route102/scripts.pory"
	.string "ROUTE 102\n"
# 113 "data/maps/Route102/scripts.pory"
	.string "{LEFT_ARROW} PETALBURG CITY$"

Test_Route102_EventScript_Calvin::
	return

