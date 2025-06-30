EventScript_PicnicBasket::
# 4 "data/scripts/custom.pory"
	playse SE_USE_ITEM
# 5 "data/scripts/custom.pory"
	msgbox gText_UsePicnicBasket, MSGBOX_DEFAULT
# 6 "data/scripts/custom.pory"
	special HealPlayerParty
# 7 "data/scripts/custom.pory"
	release
	end


gText_UsePicnicBasket::
# 11 "data/scripts/custom.pory"
	.string "{PLAYER} had a picnic with the patry.\n"
	.string "All Pokémon were fully restored!$"
