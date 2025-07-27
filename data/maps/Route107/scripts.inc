# 1 "data/maps/Route107/scripts.pory"

# 2 "data/maps/Route107/scripts.pory"
Route107_MapScripts::
# 3 "data/maps/Route107/scripts.pory"
	map_script MAP_SCRIPT_ON_LOAD, Route107_OnLoad
# 4 "data/maps/Route107/scripts.pory"
	.byte 0
# 5 "data/maps/Route107/scripts.pory"

# 6 "data/maps/Route107/scripts.pory"
Route107_OnLoad:
# 7 "data/maps/Route107/scripts.pory"
	call_if_unset FLAG_CHALLENGE2_CLEAR,Route107_WildEncounterInter
# 8 "data/maps/Route107/scripts.pory"
	end
# 9 "data/maps/Route107/scripts.pory"

# 10 "data/maps/Route107/scripts.pory"
Route107_WildEncounterInter::
# 11 "data/maps/Route107/scripts.pory"
	setmetatile  9, 11,METATILE_Dewford_SandRockOne,TRUE
# 12 "data/maps/Route107/scripts.pory"
	setmetatile 10, 11,METATILE_Dewford_SandRockOne,TRUE
# 13 "data/maps/Route107/scripts.pory"
	setmetatile 11, 11,METATILE_Dewford_SandRockOne,TRUE
# 14 "data/maps/Route107/scripts.pory"
	return
# 15 "data/maps/Route107/scripts.pory"

# 16 "data/maps/Route107/scripts.pory"
Route107_EventScript_Darrin::
# 17 "data/maps/Route107/scripts.pory"
	trainerbattle_single TRAINER_DARRIN, Route107_Text_DarrinIntro, Route107_Text_DarrinDefeated
# 18 "data/maps/Route107/scripts.pory"
	msgbox Route107_Text_DarrinPostBattle, MSGBOX_AUTOCLOSE
# 19 "data/maps/Route107/scripts.pory"
	end
# 20 "data/maps/Route107/scripts.pory"

# 21 "data/maps/Route107/scripts.pory"
Route107_EventScript_Tony::
# 22 "data/maps/Route107/scripts.pory"
	trainerbattle_single TRAINER_TONY_1, Route107_Text_TonyIntro, Route107_Text_TonyDefeated, Route107_EventScript_TonyRegisterMatchCallAfterBattle
# 23 "data/maps/Route107/scripts.pory"
	specialvar VAR_RESULT, ShouldTryRematchBattle
# 24 "data/maps/Route107/scripts.pory"
	goto_if_eq VAR_RESULT, TRUE, Route107_EventScript_TonyRematch
# 25 "data/maps/Route107/scripts.pory"
	msgbox Route107_Text_TonyPostBattle, MSGBOX_DEFAULT
# 26 "data/maps/Route107/scripts.pory"
	release
# 27 "data/maps/Route107/scripts.pory"
	end
# 28 "data/maps/Route107/scripts.pory"

# 29 "data/maps/Route107/scripts.pory"
Route107_EventScript_TonyRegisterMatchCallAfterBattle::
# 30 "data/maps/Route107/scripts.pory"
	special PlayerFaceTrainerAfterBattle
# 31 "data/maps/Route107/scripts.pory"
	waitmovement 0
# 32 "data/maps/Route107/scripts.pory"
	msgbox Route107_Text_TonyRegister, MSGBOX_DEFAULT
# 33 "data/maps/Route107/scripts.pory"
	register_matchcall TRAINER_TONY_1
# 34 "data/maps/Route107/scripts.pory"
	release
# 35 "data/maps/Route107/scripts.pory"
	end
# 36 "data/maps/Route107/scripts.pory"

# 37 "data/maps/Route107/scripts.pory"
Route107_EventScript_TonyRematch::
# 38 "data/maps/Route107/scripts.pory"
	trainerbattle_rematch TRAINER_TONY_1, Route107_Text_TonyRematchIntro, Route107_Text_TonyRematchDefeated
# 39 "data/maps/Route107/scripts.pory"
	msgbox Route107_Text_TonyRematchPostBattle, MSGBOX_AUTOCLOSE
# 40 "data/maps/Route107/scripts.pory"
	end
# 41 "data/maps/Route107/scripts.pory"

# 42 "data/maps/Route107/scripts.pory"
Route107_EventScript_Denise::
# 43 "data/maps/Route107/scripts.pory"
	trainerbattle_single TRAINER_DENISE, Route107_Text_DeniseIntro, Route107_Text_DeniseDefeated
# 44 "data/maps/Route107/scripts.pory"
	msgbox Route107_Text_DenisePostBattle, MSGBOX_AUTOCLOSE
# 45 "data/maps/Route107/scripts.pory"
	end
# 46 "data/maps/Route107/scripts.pory"

# 47 "data/maps/Route107/scripts.pory"
Route107_EventScript_Beth::
# 48 "data/maps/Route107/scripts.pory"
	trainerbattle_single TRAINER_BETH, Route107_Text_BethIntro, Route107_Text_BethDefeated
# 49 "data/maps/Route107/scripts.pory"
	msgbox Route107_Text_BethPostBattle, MSGBOX_AUTOCLOSE
# 50 "data/maps/Route107/scripts.pory"
	end
# 51 "data/maps/Route107/scripts.pory"

# 52 "data/maps/Route107/scripts.pory"
Route107_EventScript_Lisa::
# 53 "data/maps/Route107/scripts.pory"
	trainerbattle_double TRAINER_LISA_AND_RAY, Route107_Text_LisaIntro, Route107_Text_LisaDefeated, Route107_Text_LisaNotEnoughPokemon
# 54 "data/maps/Route107/scripts.pory"
	msgbox Route107_Text_LisaPostBattle, MSGBOX_AUTOCLOSE
# 55 "data/maps/Route107/scripts.pory"
	end
# 56 "data/maps/Route107/scripts.pory"

# 57 "data/maps/Route107/scripts.pory"
Route107_EventScript_Ray::
# 58 "data/maps/Route107/scripts.pory"
	trainerbattle_double TRAINER_LISA_AND_RAY, Route107_Text_RayIntro, Route107_Text_RayDefeated, Route107_Text_RayNotEnoughPokemon
# 59 "data/maps/Route107/scripts.pory"
	msgbox Route107_Text_RayPostBattle, MSGBOX_AUTOCLOSE
# 60 "data/maps/Route107/scripts.pory"
	end
# 61 "data/maps/Route107/scripts.pory"

# 62 "data/maps/Route107/scripts.pory"
Route107_EventScript_Camron::
# 63 "data/maps/Route107/scripts.pory"
	trainerbattle_single TRAINER_CAMRON, Route107_Text_CamronIntro, Route107_Text_CamronDefeated
# 64 "data/maps/Route107/scripts.pory"
	msgbox Route107_Text_CamronPostBattle, MSGBOX_AUTOCLOSE
# 65 "data/maps/Route107/scripts.pory"
	end
