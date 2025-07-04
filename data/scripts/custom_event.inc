EventScript_AdventureSetup::
# 3 "data/scripts/custom_event.pory"
	setflag FLAG_DYNAMAX_BATTLE
# 4 "data/scripts/custom_event.pory"
	setflag FLAG_TERA_ORB_CHARGED
# 5 "data/scripts/custom_event.pory"
	setflag FLAG_USE_EXP_SHARE
# 7 "data/scripts/custom_event.pory"
	setflag DN_FLAG_DEXNAV_GET
# 8 "data/scripts/custom_event.pory"
	giveitem ITEM_CLOCK
# 9 "data/scripts/custom_event.pory"
	giveitem ITEM_POFFIN_CASE
# 10 "data/scripts/custom_event.pory"
	giveitem ITEM_FLYING
# 11 "data/scripts/custom_event.pory"
	giveitem ITEM_EXP_SHARE
# 14 "data/scripts/custom_event.pory"
	release
	end


EventScript_BirchCase::
# 20 "data/scripts/custom_event.pory"
	msgbox PlayersHouse_2F_Text_ItsAGameCube, MSGBOX_DEFAULT
# 21 "data/scripts/custom_event.pory"
	callnative StartNewPokeballCaseUI
# 22 "data/scripts/custom_event.pory"
	waitstate
# 23 "data/scripts/custom_event.pory"
	releaseall
	end

