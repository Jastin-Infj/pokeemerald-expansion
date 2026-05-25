Route301_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Route301_OnTransition
	.byte 0

Route301_OnTransition:
	setflag FLAG_VISITED_ROUTE301
	end
