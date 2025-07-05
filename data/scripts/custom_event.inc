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


EventScript_Mega::
# 20 "data/scripts/custom_event.pory"
	giveitem ITEM_MEGA_RING
# 21 "data/scripts/custom_event.pory"
	release
	end


EventScript_ZMove::
# 27 "data/scripts/custom_event.pory"
	giveitem ITEM_Z_POWER_RING
# 28 "data/scripts/custom_event.pory"
	release
	end


EventScript_Dynamax::
# 34 "data/scripts/custom_event.pory"
	giveitem ITEM_DYNAMAX_BAND
# 35 "data/scripts/custom_event.pory"
	release
	end


EventScript_TeraOrb::
# 41 "data/scripts/custom_event.pory"
	giveitem ITEM_TERA_ORB
# 42 "data/scripts/custom_event.pory"
	release
	end


EventScript_BirchCase::
# 48 "data/scripts/custom_event.pory"
	msgbox PlayersHouse_2F_Text_ItsAGameCube, MSGBOX_DEFAULT
# 49 "data/scripts/custom_event.pory"
	callnative StartNewPokeballCaseUI
# 50 "data/scripts/custom_event.pory"
	waitstate
# 51 "data/scripts/custom_event.pory"
	releaseall
	end

