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


EventScript_Clock::
# 19 "data/scripts/custom.pory"
	call PlayersHouse_2F_EventScript_SetWallClock
# 20 "data/scripts/custom.pory"
	goto Debug_EventScript_TellTheTime
	end


Common_EventScript_Pokemart::
# 26 "data/scripts/custom.pory"
	lock
# 27 "data/scripts/custom.pory"
	faceplayer
# 28 "data/scripts/custom.pory"
	message gText_HowMayIServeYou
# 29 "data/scripts/custom.pory"
	waitmessage
# 30 "data/scripts/custom.pory"
	dynmultichoice 0, 0, TRUE, 5, 0, DYN_MULTICHOICE_CB_NONE, gText_ShopHeldItems, gText_ShopEvolution, gText_ShopGimmick, gText_ShopTMHMs, gText_ShopSpec, gText_ShopLevelUp, gText_ShopPokeball, gText_ShopFormItems, gText_ShopBerry, gText_ShopCansel
# 43 "data/scripts/custom.pory"
	switch VAR_RESULT
# 45 "data/scripts/custom.pory"
	case 0, Common_EventScript_Pokemart_3
# 47 "data/scripts/custom.pory"
	case 1, Common_EventScript_Pokemart_4
# 49 "data/scripts/custom.pory"
	case 2, Common_EventScript_Pokemart_5
# 51 "data/scripts/custom.pory"
	case 3, Common_EventScript_Pokemart_6
# 53 "data/scripts/custom.pory"
	case 4, Common_EventScript_Pokemart_7
# 55 "data/scripts/custom.pory"
	case 5, Common_EventScript_Pokemart_8
# 57 "data/scripts/custom.pory"
	case 6, Common_EventScript_Pokemart_9
# 59 "data/scripts/custom.pory"
	case 7, Common_EventScript_Pokemart_10
# 61 "data/scripts/custom.pory"
	case 8, Common_EventScript_Pokemart_11
Common_EventScript_Pokemart_1:
# 65 "data/scripts/custom.pory"
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
# 66 "data/scripts/custom.pory"
	release
	end

Common_EventScript_Pokemart_3:
# 46 "data/scripts/custom.pory"
	pokemart Common_Shop_HeldItems
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_4:
# 48 "data/scripts/custom.pory"
	pokemart Common_Shop_Evolution
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_5:
# 50 "data/scripts/custom.pory"
	pokemart Common_Shop_Gimmick
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_6:
# 52 "data/scripts/custom.pory"
	pokemart Common_Shop_TMs
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_7:
# 54 "data/scripts/custom.pory"
	pokemart Common_Shop_Spec
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_8:
# 56 "data/scripts/custom.pory"
	pokemart Common_Shop_Levelup
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_9:
# 58 "data/scripts/custom.pory"
	pokemart Common_Shop_Pokeball
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_10:
# 60 "data/scripts/custom.pory"
	pokemart Common_Shop_FormItems
	goto Common_EventScript_Pokemart_1

Common_EventScript_Pokemart_11:
# 62 "data/scripts/custom.pory"
	pokemart Common_Shop_Berry
	goto Common_EventScript_Pokemart_1


gText_UsePicnicBasket::
# 11 "data/scripts/custom.pory"
	.string "{PLAYER} had a picnic with the patry.\n"
	.string "All Pokémon were fully restored!$"

gText_ShopGimmick::
# 70 "data/scripts/custom.pory"
	.string "Gimmicks$"

gText_ShopEvolution::
# 71 "data/scripts/custom.pory"
	.string "Evolution$"

gText_ShopHeldItems::
# 72 "data/scripts/custom.pory"
	.string "Held Items$"

gText_ShopTMHMs::
# 73 "data/scripts/custom.pory"
	.string "TM / HMs$"

gText_ShopSpec::
# 74 "data/scripts/custom.pory"
	.string "Ability/Nature$"

gText_ShopLevelUp::
# 75 "data/scripts/custom.pory"
	.string "Level up$"

gText_ShopPokeball::
# 76 "data/scripts/custom.pory"
	.string "Poke ball$"

gText_ShopFormItems::
# 77 "data/scripts/custom.pory"
	.string "Form items$"

gText_ShopBerry::
# 78 "data/scripts/custom.pory"
	.string "Berry$"

gText_ShopCansel::
# 79 "data/scripts/custom.pory"
	.string "   Cansel$"
