EventScript_BirchCase::
# 3 "data/scripts/custom_event.pory"
	msgbox PlayersHouse_2F_Text_ItsAGameCube, MSGBOX_DEFAULT
# 4 "data/scripts/custom_event.pory"
	callnative StartNewPokeballCaseUI
# 5 "data/scripts/custom_event.pory"
	waitstate
# 6 "data/scripts/custom_event.pory"
	releaseall
	end

