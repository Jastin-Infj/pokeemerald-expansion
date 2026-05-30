RustboroCity_CuttersHouse_MapScripts::
	.byte 0

RustboroCity_CuttersHouse_EventScript_Cutter::
	lock
	faceplayer
	msgbox RustboroCity_CuttersHouse_Text_YouCanPutThisHMToGoodUse, MSGBOX_DEFAULT
	msgbox RustboroCity_CuttersHouse_Text_ExplainCut, MSGBOX_DEFAULT
	release
	end

RustboroCity_CuttersHouse_EventScript_Lass::
	msgbox RustboroCity_CuttersHouse_Text_DadHelpedClearLandOfTrees, MSGBOX_NPC
	end

RustboroCity_CuttersHouse_Text_YouCanPutThisHMToGoodUse:
	.string "That determined expression…\n"
	.string "That limber way you move…\l"
	.string "And your well-trained POKéMON…\p"
	.string "You're obviously a skilled TRAINER!\p"
	.string "No, wait, don't say a word.\n"
	.string "I can tell just by looking at you.\p"
	.string "I'm sure that you can put the\n"
	.string "CUT technique to good use.\p"
	.string "No need to be modest or shy.\n"
	.string "Let me tell you about it!$"

RustboroCity_CuttersHouse_Text_ExplainCut:
	.string "CUT can be used by POKéMON\n"
	.string "outside of battle.\p"
	.string "Any POKéMON that knows CUT can\n"
	.string "chop down thin trees if the TRAINER\l"
	.string "has earned the STONE BADGE.$"

RustboroCity_CuttersHouse_Text_DadHelpedClearLandOfTrees:
	.string "When they were expanding the city of\n"
	.string "RUSTBORO, my dad helped out.\p"
	.string "He made his POKéMON use CUT to clear\n"
	.string "the land of trees.$"
