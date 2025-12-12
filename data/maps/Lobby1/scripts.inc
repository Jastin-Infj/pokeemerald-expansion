Lobby1_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Lobby1_OnTransition
	.byte 0

Lobby1_OnTransition:
	setflag FLAG_VISITED_LOBBY1
	end
