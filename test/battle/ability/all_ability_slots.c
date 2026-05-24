#include "global.h"
#include "battle_util.h"
#include "test/battle.h"

SINGLE_BATTLE_TEST("All Ability Slots lets a non-representative Magic Guard prevent recoil")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_CLEFABLE, 0) == ABILITY_CUTE_CHARM);
        ASSUME(GetSpeciesAbility(SPECIES_CLEFABLE, 1) == ABILITY_MAGIC_GUARD);
        ASSUME(GetMoveRecoil(MOVE_DOUBLE_EDGE) == 33);
        PLAYER(SPECIES_CLEFABLE) { Ability(ABILITY_CUTE_CHARM); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_DOUBLE_EDGE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DOUBLE_EDGE, player);
        HP_BAR(opponent);
        NOT HP_BAR(player);
    } THEN {
        EXPECT(IsBattlerAbilityActive(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT), ABILITY_MAGIC_GUARD));
    }
}

SINGLE_BATTLE_TEST("All Ability Slots keeps non-representative slots when Worry Seed overwrites one slot")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_CLEFABLE, 0) == ABILITY_CUTE_CHARM);
        ASSUME(GetSpeciesAbility(SPECIES_CLEFABLE, 1) == ABILITY_MAGIC_GUARD);
        ASSUME(GetMoveEffect(MOVE_WORRY_SEED) == EFFECT_OVERWRITE_ABILITY);
        ASSUME(GetMoveOverwriteAbility(MOVE_WORRY_SEED) == ABILITY_INSOMNIA);
        ASSUME(GetMoveRecoil(MOVE_DOUBLE_EDGE) == 33);
        PLAYER(SPECIES_CLEFABLE) { Ability(ABILITY_CUTE_CHARM); Speed(10); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(20); Moves(MOVE_WORRY_SEED); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_WORRY_SEED); MOVE(player, MOVE_DOUBLE_EDGE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_WORRY_SEED, opponent);
        ABILITY_POPUP(player, ABILITY_CUTE_CHARM);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DOUBLE_EDGE, player);
        HP_BAR(opponent);
        NOT HP_BAR(player);
    } THEN {
        EXPECT_EQ(player->ability, ABILITY_INSOMNIA);
        EXPECT(IsBattlerAbilityActive(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT), ABILITY_MAGIC_GUARD));
    }
}

SINGLE_BATTLE_TEST("All Ability Slots swaps only the matching operation slot")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_CLEFABLE, 1) == ABILITY_MAGIC_GUARD);
        ASSUME(GetSpeciesAbility(SPECIES_EKANS, 0) == ABILITY_INTIMIDATE);
        ASSUME(GetSpeciesAbility(SPECIES_EKANS, 1) == ABILITY_SHED_SKIN);
        ASSUME(GetMoveEffect(MOVE_SKILL_SWAP) == EFFECT_SKILL_SWAP);
        PLAYER(SPECIES_CLEFABLE) { Ability(ABILITY_MAGIC_GUARD); }
        OPPONENT(SPECIES_EKANS) { Ability(ABILITY_INTIMIDATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SKILL_SWAP); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SKILL_SWAP, player);
        ABILITY_POPUP(player, ABILITY_MAGIC_GUARD);
        ABILITY_POPUP(opponent, ABILITY_SHED_SKIN);
    } THEN {
        EXPECT_EQ(player->ability, ABILITY_SHED_SKIN);
        EXPECT_EQ(opponent->ability, ABILITY_INTIMIDATE);
        EXPECT(IsBattlerAbilityActive(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), ABILITY_MAGIC_GUARD));
    }
}

SINGLE_BATTLE_TEST("All Ability Slots lets a non-representative Flash Fire absorb Fire moves")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_HOUNDOOM, 0) == ABILITY_EARLY_BIRD);
        ASSUME(GetSpeciesAbility(SPECIES_HOUNDOOM, 1) == ABILITY_FLASH_FIRE);
        PLAYER(SPECIES_ARCANINE);
        OPPONENT(SPECIES_HOUNDOOM) { Ability(ABILITY_EARLY_BIRD); }
    } WHEN {
        TURN { MOVE(player, MOVE_FLAMETHROWER); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_FLASH_FIRE);
        MESSAGE("The opposing Houndoom's Flash Fire raised the power of Fire-type moves!");
        NONE_OF {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_FLAMETHROWER, player);
            HP_BAR(opponent);
        }
    } THEN {
        EXPECT(gBattleMons[GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT)].volatiles.flashFireBoosted);
        EXPECT(IsBattlerAbilityActive(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), ABILITY_FLASH_FIRE));
    }
}

SINGLE_BATTLE_TEST("All Ability Slots shows Houndoom's hidden Unnerve before later Flash Fire")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_HOUNDOOM, 0) == ABILITY_EARLY_BIRD);
        ASSUME(GetSpeciesAbility(SPECIES_HOUNDOOM, 1) == ABILITY_FLASH_FIRE);
        ASSUME(GetSpeciesAbility(SPECIES_HOUNDOOM, 2) == ABILITY_UNNERVE);
        PLAYER(SPECIES_ARCANINE);
        OPPONENT(SPECIES_HOUNDOOM) { Ability(ABILITY_EARLY_BIRD); }
    } WHEN {
        TURN { MOVE(player, MOVE_FLAMETHROWER); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_UNNERVE);
        ABILITY_POPUP(opponent, ABILITY_FLASH_FIRE);
        NONE_OF {
            ABILITY_POPUP(opponent, ABILITY_EARLY_BIRD);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_FLAMETHROWER, player);
            HP_BAR(opponent);
        }
    } THEN {
        EXPECT(gBattleMons[GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT)].volatiles.unnerveActivated);
        EXPECT(gBattleMons[GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT)].volatiles.flashFireBoosted);
    }
}

SINGLE_BATTLE_TEST("All Ability Slots lets a non-representative Soundproof block sound moves")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_KOMMO_O, 0) == ABILITY_BULLETPROOF);
        ASSUME(GetSpeciesAbility(SPECIES_KOMMO_O, 1) == ABILITY_SOUNDPROOF);
        ASSUME(IsSoundMove(MOVE_HYPER_VOICE));
        PLAYER(SPECIES_EXPLOUD) { Ability(ABILITY_SCRAPPY); }
        OPPONENT(SPECIES_KOMMO_O) { Ability(ABILITY_BULLETPROOF); }
    } WHEN {
        TURN { MOVE(player, MOVE_HYPER_VOICE); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_SOUNDPROOF);
        MESSAGE("The opposing Kommo-o's Soundproof blocks Hyper Voice!");
        NONE_OF {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_HYPER_VOICE, player);
            HP_BAR(opponent);
        }
    } THEN {
        EXPECT(IsBattlerAbilityActive(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), ABILITY_SOUNDPROOF));
    }
}

SINGLE_BATTLE_TEST("All Ability Slots shows non-representative switch-in weather abilities")
{
    u32 species = SPECIES_NONE, representativeAbility = ABILITY_NONE, weatherAbility = ABILITY_NONE, weatherSlot = 0, expectedWeather = 0;

    PARAMETRIZE { species = SPECIES_POLITOED; representativeAbility = ABILITY_WATER_ABSORB; weatherAbility = ABILITY_DRIZZLE; weatherSlot = 2; expectedWeather = B_WEATHER_RAIN; }
    PARAMETRIZE { species = SPECIES_GIGALITH; representativeAbility = ABILITY_STURDY; weatherAbility = ABILITY_SAND_STREAM; weatherSlot = 1; expectedWeather = B_WEATHER_SANDSTORM; }
    PARAMETRIZE { species = SPECIES_VANILLUXE; representativeAbility = ABILITY_ICE_BODY; weatherAbility = ABILITY_SNOW_WARNING; weatherSlot = 1; expectedWeather = B_WEATHER_SNOW; }

    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        WITH_CONFIG(B_ABILITY_WEATHER, GEN_6);
        WITH_CONFIG(B_SNOW_WARNING, GEN_9);
        ASSUME(GetSpeciesAbility(species, 0) == representativeAbility);
        ASSUME(GetSpeciesAbility(species, weatherSlot) == weatherAbility);
        PLAYER(species) { Ability(representativeAbility); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ABILITY_POPUP(player, weatherAbility);
        NOT ABILITY_POPUP(player, representativeAbility);
    } THEN {
        EXPECT(gBattleWeather & expectedWeather);
        EXPECT(IsBattlerAbilityActive(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT), weatherAbility));
    }
}

SINGLE_BATTLE_TEST("All Ability Slots shows non-representative Electric Surge")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_PINCURCHIN, 0) == ABILITY_LIGHTNING_ROD);
        ASSUME(GetSpeciesAbility(SPECIES_PINCURCHIN, 2) == ABILITY_ELECTRIC_SURGE);
        PLAYER(SPECIES_PINCURCHIN) { Ability(ABILITY_LIGHTNING_ROD); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_ELECTRIC_SURGE);
        NOT ABILITY_POPUP(player, ABILITY_LIGHTNING_ROD);
    } THEN {
        EXPECT(gFieldStatuses & STATUS_FIELD_ELECTRIC_TERRAIN);
        EXPECT(IsBattlerAbilityActive(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT), ABILITY_ELECTRIC_SURGE));
    }
}

SINGLE_BATTLE_TEST("All Ability Slots shows non-representative Download")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_PORYGON_Z, 0) == ABILITY_ADAPTABILITY);
        ASSUME(GetSpeciesAbility(SPECIES_PORYGON_Z, 1) == ABILITY_DOWNLOAD);
        PLAYER(SPECIES_PORYGON_Z) { Ability(ABILITY_ADAPTABILITY); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_DOWNLOAD);
        NOT ABILITY_POPUP(player, ABILITY_ADAPTABILITY);
    } THEN {
        EXPECT_EQ(player->statStages[STAT_SPATK], DEFAULT_STAT_STAGE + 1);
        EXPECT(IsBattlerAbilityActive(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT), ABILITY_DOWNLOAD));
    }
}

SINGLE_BATTLE_TEST("All Ability Slots shows non-representative Frisk on switch-in")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_BANETTE, 0) == ABILITY_INSOMNIA);
        ASSUME(GetSpeciesAbility(SPECIES_BANETTE, 1) == ABILITY_FRISK);
        PLAYER(SPECIES_BANETTE) { Ability(ABILITY_INSOMNIA); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_LEFTOVERS); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_FRISK);
        NOT ABILITY_POPUP(player, ABILITY_INSOMNIA);
    } THEN {
        EXPECT(IsBattlerAbilityActive(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT), ABILITY_FRISK));
    }
}

SINGLE_BATTLE_TEST("All Ability Slots shows non-representative Trace before copying the matching slot")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_GARDEVOIR, 0) == ABILITY_SYNCHRONIZE);
        ASSUME(GetSpeciesAbility(SPECIES_GARDEVOIR, 1) == ABILITY_TRACE);
        ASSUME(GetSpeciesAbility(SPECIES_WOBBUFFET, 0) == ABILITY_SHADOW_TAG);
        PLAYER(SPECIES_GARDEVOIR) { Ability(ABILITY_SYNCHRONIZE); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_SHADOW_TAG); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_TRACE);
        NOT ABILITY_POPUP(player, ABILITY_SYNCHRONIZE);
    } THEN {
        EXPECT(IsBattlerAbilityActive(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT), ABILITY_SHADOW_TAG));
    }
}

DOUBLE_BATTLE_TEST("All Ability Slots lets non-representative Commander activate with Dondozo")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_TATSUGIRI_CURLY, 0) == ABILITY_COMMANDER);
        ASSUME(GetSpeciesAbility(SPECIES_TATSUGIRI_CURLY, 2) == ABILITY_STORM_DRAIN);
        PLAYER(SPECIES_TATSUGIRI_CURLY) { Ability(ABILITY_STORM_DRAIN); }
        PLAYER(SPECIES_DONDOZO);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN {}
    } SCENE {
        ABILITY_POPUP(playerLeft, ABILITY_COMMANDER);
        MESSAGE("Tatsugiri was swallowed by Dondozo and became Dondozo's commander!");
        NOT ABILITY_POPUP(playerLeft, ABILITY_STORM_DRAIN);
    } THEN {
        EXPECT(gBattleMons[GetBattlerAtPosition(B_POSITION_PLAYER_LEFT)].volatiles.semiInvulnerable == STATE_COMMANDER);
    }
}

SINGLE_BATTLE_TEST("All Ability Slots lets non-representative Rain Dish heal in rain")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_LOTAD, 0) == ABILITY_SWIFT_SWIM);
        ASSUME(GetSpeciesAbility(SPECIES_LOTAD, 1) == ABILITY_RAIN_DISH);
        ASSUME(GetMoveEffect(MOVE_RAIN_DANCE) == EFFECT_WEATHER);
        ASSUME(GetMoveWeatherType(MOVE_RAIN_DANCE) == BATTLE_WEATHER_RAIN);
        PLAYER(SPECIES_LOTAD) { Ability(ABILITY_SWIFT_SWIM); HP(1); MaxHP(100); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_RAIN_DANCE); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_RAIN_DISH);
        NOT ABILITY_POPUP(player, ABILITY_SWIFT_SWIM);
        HP_BAR(player, damage: -(100 / 16));
    }
}

SINGLE_BATTLE_TEST("All Ability Slots lets non-representative Ice Body heal in snow or hail")
{
    enum Move move;
    PARAMETRIZE { move = MOVE_HAIL; }
    PARAMETRIZE { move = MOVE_SNOWSCAPE; }

    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_WALREIN, 0) == ABILITY_THICK_FAT);
        ASSUME(GetSpeciesAbility(SPECIES_WALREIN, 1) == ABILITY_ICE_BODY);
        ASSUME(GetMoveEffect(move) == EFFECT_WEATHER);
        PLAYER(SPECIES_WALREIN) { Ability(ABILITY_THICK_FAT); HP(1); MaxHP(100); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, move); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_ICE_BODY);
        NOT ABILITY_POPUP(player, ABILITY_THICK_FAT);
        HP_BAR(player, damage: -(100 / 16));
    }
}

SINGLE_BATTLE_TEST("All Ability Slots can let non-representative Mold Breaker bypass Wonder Guard")
{
    bool32 moldBreakerConfig;
    PARAMETRIZE { moldBreakerConfig = TRUE; }
    PARAMETRIZE { moldBreakerConfig = FALSE; }

    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        WITH_CONFIG(B_ALL_ABILITY_SLOTS_MOLD_BREAKER, moldBreakerConfig);
        ASSUME(GetSpeciesAbility(SPECIES_PINSIR, 0) == ABILITY_HYPER_CUTTER);
        ASSUME(GetSpeciesAbility(SPECIES_PINSIR, 1) == ABILITY_MOLD_BREAKER);
        ASSUME(GetSpeciesAbility(SPECIES_SHEDINJA, 0) == ABILITY_WONDER_GUARD);
        PLAYER(SPECIES_PINSIR) { Ability(ABILITY_HYPER_CUTTER); Moves(MOVE_WATER_GUN); }
        OPPONENT(SPECIES_SHEDINJA) { Ability(ABILITY_WONDER_GUARD); }
    } WHEN {
        TURN { MOVE(player, MOVE_WATER_GUN); }
    } SCENE {
        if (moldBreakerConfig)
        {
            ABILITY_POPUP(player, ABILITY_MOLD_BREAKER);
            NOT ABILITY_POPUP(opponent, ABILITY_WONDER_GUARD);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_WATER_GUN, player);
            HP_BAR(opponent);
        }
        else
        {
            NOT ABILITY_POPUP(player, ABILITY_MOLD_BREAKER);
            ABILITY_POPUP(opponent, ABILITY_WONDER_GUARD);
            NONE_OF {
                ANIMATION(ANIM_TYPE_MOVE, MOVE_WATER_GUN, player);
                HP_BAR(opponent);
            }
        }
    }
}

SINGLE_BATTLE_TEST("All Ability Slots can let non-representative Neutralizing Gas suppress other abilities")
{
    bool32 neutralizingGasConfig;
    PARAMETRIZE { neutralizingGasConfig = TRUE; }
    PARAMETRIZE { neutralizingGasConfig = FALSE; }

    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        WITH_CONFIG(B_ALL_ABILITY_SLOTS_NEUTRALIZING_GAS, neutralizingGasConfig);
        ASSUME(GetSpeciesAbility(SPECIES_WEEZING, 0) == ABILITY_LEVITATE);
        ASSUME(GetSpeciesAbility(SPECIES_WEEZING, 1) == ABILITY_NEUTRALIZING_GAS);
        ASSUME(GetMoveEffect(MOVE_REST) == EFFECT_REST);
        PLAYER(SPECIES_DROWZEE) { Ability(ABILITY_INSOMNIA); HP(1); }
        OPPONENT(SPECIES_WEEZING) { Ability(ABILITY_LEVITATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_REST); }
    } SCENE {
        if (neutralizingGasConfig)
        {
            ABILITY_POPUP(opponent, ABILITY_NEUTRALIZING_GAS);
            NOT ABILITY_POPUP(player, ABILITY_INSOMNIA);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_REST, player);
            HP_BAR(player);
        }
        else
        {
            NOT ABILITY_POPUP(opponent, ABILITY_NEUTRALIZING_GAS);
            ABILITY_POPUP(player, ABILITY_INSOMNIA);
            NOT ANIMATION(ANIM_TYPE_MOVE, MOVE_REST, player);
        }
    }
}

SINGLE_BATTLE_TEST("All Ability Slots lets non-representative Chlorophyll affect turn order")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_BULBASAUR, 0) == ABILITY_OVERGROW);
        ASSUME(GetSpeciesAbility(SPECIES_BULBASAUR, 2) == ABILITY_CHLOROPHYLL);
        PLAYER(SPECIES_BULBASAUR) { Ability(ABILITY_OVERGROW); Speed(100); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(199); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_CELEBRATE); MOVE(player, MOVE_SUNNY_DAY); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SUNNY_DAY, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
    }
}

SINGLE_BATTLE_TEST("All Ability Slots lets non-representative Pickup restore a used item")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(gItemsInfo[ITEM_SITRUS_BERRY].holdEffect == HOLD_EFFECT_RESTORE_PCT_HP);
        ASSUME(GetSpeciesAbility(SPECIES_ZIGZAGOON, 0) == ABILITY_PICKUP);
        ASSUME(GetSpeciesAbility(SPECIES_ZIGZAGOON, 1) == ABILITY_GLUTTONY);
        PLAYER(SPECIES_ZIGZAGOON) { Ability(ABILITY_GLUTTONY); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(100); HP(51); Item(ITEM_SITRUS_BERRY); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
        ABILITY_POPUP(player, ABILITY_PICKUP);
        MESSAGE("Zigzagoon found one Sitrus Berry!");
    } THEN {
        EXPECT_EQ(player->item, ITEM_SITRUS_BERRY);
    }
}

SINGLE_BATTLE_TEST("All Ability Slots lets a non-representative Drought start sun with the correct popup")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        WITH_CONFIG(B_ABILITY_WEATHER, GEN_6);
        ASSUME(GetSpeciesAbility(SPECIES_NINETALES, 0) == ABILITY_FLASH_FIRE);
        ASSUME(GetSpeciesAbility(SPECIES_NINETALES, 2) == ABILITY_DROUGHT);
        PLAYER(SPECIES_NINETALES) { Ability(ABILITY_FLASH_FIRE); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_DROUGHT);
        NOT ABILITY_POPUP(player, ABILITY_FLASH_FIRE);
        MESSAGE("Ninetales's Drought intensified the sun's rays!");
    } THEN {
        EXPECT(gBattleWeather & B_WEATHER_SUN);
        EXPECT(IsBattlerAbilityActive(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT), ABILITY_DROUGHT));
    }
}

SINGLE_BATTLE_TEST("All Ability Slots lets a non-representative Intimidate trigger on switch-in")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_LUXRAY, 0) == ABILITY_RIVALRY);
        ASSUME(GetSpeciesAbility(SPECIES_LUXRAY, 1) == ABILITY_INTIMIDATE);
        PLAYER(SPECIES_LUXRAY) { Ability(ABILITY_RIVALRY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN {}
    } SCENE {
        ABILITY_POPUP(player, ABILITY_INTIMIDATE);
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_ATK], DEFAULT_STAT_STAGE - 1);
        EXPECT(IsBattlerAbilityActive(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT), ABILITY_INTIMIDATE));
    }
}

SINGLE_BATTLE_TEST("All Ability Slots keeps switch-in Pressure popup stable when later slots do not trigger")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_WEAVILE, 0) == ABILITY_PRESSURE);
        ASSUME(GetSpeciesAbility(SPECIES_WEAVILE, 2) == ABILITY_PICKPOCKET);
        PLAYER(SPECIES_WEAVILE) { Ability(ABILITY_PRESSURE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN {}
    } SCENE {
        ABILITY_POPUP(player, ABILITY_PRESSURE);
        NOT ABILITY_POPUP(player, ABILITY_PICKPOCKET);
    } THEN {
        EXPECT(IsBattlerAbilityActive(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT), ABILITY_PRESSURE));
        EXPECT(IsBattlerAbilityActive(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT), ABILITY_PICKPOCKET));
    }
}

SINGLE_BATTLE_TEST("All Ability Slots lets non-representative Klutz suppress held items without a popup")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        WITH_CONFIG(B_KLUTZ_FLING_INTERACTION, GEN_5);
        ASSUME(GetSpeciesAbility(SPECIES_LOPUNNY, 0) == ABILITY_CUTE_CHARM);
        ASSUME(GetSpeciesAbility(SPECIES_LOPUNNY, 1) == ABILITY_KLUTZ);
        PLAYER(SPECIES_LOPUNNY) { Ability(ABILITY_CUTE_CHARM); Item(ITEM_LEFTOVERS); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN {}
    } SCENE {
        NOT ABILITY_POPUP(player, ABILITY_KLUTZ);
    } THEN {
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        EXPECT_EQ(GetBattlerHoldEffect(battler), HOLD_EFFECT_NONE);
        EXPECT(!CanFling(battler));
    }
}

SINGLE_BATTLE_TEST("All Ability Slots lets a non-representative Levitate block Ground moves")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_BRONZONG, 0) == ABILITY_LEVITATE);
        ASSUME(GetSpeciesAbility(SPECIES_BRONZONG, 1) == ABILITY_HEATPROOF);
        ASSUME(GetMoveType(MOVE_EARTHQUAKE) == TYPE_GROUND);
        PLAYER(SPECIES_GARCHOMP);
        OPPONENT(SPECIES_BRONZONG) { Ability(ABILITY_HEATPROOF); }
    } WHEN {
        TURN { MOVE(player, MOVE_EARTHQUAKE); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_LEVITATE);
        NOT HP_BAR(opponent);
    } THEN {
        EXPECT(IsBattlerAbilityActive(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), ABILITY_LEVITATE));
    }
}

SINGLE_BATTLE_TEST("All Ability Slots lets a non-representative Storm Drain absorb Water moves")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        WITH_CONFIG(B_REDIRECT_ABILITY_IMMUNITY, GEN_5);
        ASSUME(GetSpeciesAbility(SPECIES_GASTRODON, 0) == ABILITY_STICKY_HOLD);
        ASSUME(GetSpeciesAbility(SPECIES_GASTRODON, 1) == ABILITY_STORM_DRAIN);
        ASSUME(GetMoveType(MOVE_WATER_GUN) == TYPE_WATER);
        PLAYER(SPECIES_BLASTOISE);
        OPPONENT(SPECIES_GASTRODON) { Ability(ABILITY_STICKY_HOLD); }
    } WHEN {
        TURN { MOVE(player, MOVE_WATER_GUN); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        NONE_OF {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_WATER_GUN, player);
            HP_BAR(opponent);
        }
        ABILITY_POPUP(opponent, ABILITY_STORM_DRAIN);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, opponent);
        MESSAGE("The opposing Gastrodon's Sp. Atk rose!");
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_SPATK], DEFAULT_STAT_STAGE + 1);
        EXPECT(IsBattlerAbilityActive(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), ABILITY_STORM_DRAIN));
    }
}

SINGLE_BATTLE_TEST("All Ability Slots lets a non-representative Sticky Hold keep held items")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_MUK, 0) == ABILITY_STENCH);
        ASSUME(GetSpeciesAbility(SPECIES_MUK, 1) == ABILITY_STICKY_HOLD);
        PLAYER(SPECIES_WEAVILE);
        OPPONENT(SPECIES_MUK) { Ability(ABILITY_STENCH); Item(ITEM_LEFTOVERS); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_CELEBRATE); MOVE(player, MOVE_KNOCK_OFF); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_STICKY_HOLD);
    } THEN {
        EXPECT_EQ(opponent->item, ITEM_LEFTOVERS);
        EXPECT(IsBattlerAbilityActive(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), ABILITY_STICKY_HOLD));
    }
}

SINGLE_BATTLE_TEST("All Ability Slots lets a non-representative Queenly Majesty block priority moves")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_TSAREENA, 0) == ABILITY_LEAF_GUARD);
        ASSUME(GetSpeciesAbility(SPECIES_TSAREENA, 1) == ABILITY_QUEENLY_MAJESTY);
        ASSUME(GetMovePriority(MOVE_QUICK_ATTACK) > 0);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_TSAREENA) { Ability(ABILITY_LEAF_GUARD); }
    } WHEN {
        TURN { MOVE(player, MOVE_QUICK_ATTACK); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_QUEENLY_MAJESTY);
        MESSAGE("Wobbuffet cannot use Quick Attack!");
        NONE_OF {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_QUICK_ATTACK, player);
            HP_BAR(opponent);
        }
    } THEN {
        EXPECT(IsBattlerAbilityActive(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), ABILITY_QUEENLY_MAJESTY));
    }
}

SINGLE_BATTLE_TEST("All Ability Slots stacks Conkeldurr's Guts, Sheer Force, and Iron Fist", s16 damage)
{
    bool32 allAbilitySlots;
    PARAMETRIZE { allAbilitySlots = FALSE; }
    PARAMETRIZE { allAbilitySlots = TRUE; }

    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, allAbilitySlots);
        ASSUME(GetSpeciesAbility(SPECIES_CONKELDURR, 0) == ABILITY_GUTS);
        ASSUME(GetSpeciesAbility(SPECIES_CONKELDURR, 1) == ABILITY_SHEER_FORCE);
        ASSUME(GetSpeciesAbility(SPECIES_CONKELDURR, 2) == ABILITY_IRON_FIST);
        ASSUME(IsPunchingMove(MOVE_THUNDER_PUNCH));
        ASSUME(MoveIsAffectedBySheerForce(MOVE_THUNDER_PUNCH));
        PLAYER(SPECIES_CONKELDURR) { Ability(ABILITY_GUTS); Status1(STATUS1_BURN); Moves(MOVE_THUNDER_PUNCH); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_THUNDER_PUNCH); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_GT(results[1].damage, results[0].damage);
    }
}

SINGLE_BATTLE_TEST("All Ability Slots lets non-representative Sheer Force suppress secondary effects")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_CONKELDURR, 0) == ABILITY_GUTS);
        ASSUME(GetSpeciesAbility(SPECIES_CONKELDURR, 1) == ABILITY_SHEER_FORCE);
        ASSUME(MoveIsAffectedBySheerForce(MOVE_NUZZLE));
        PLAYER(SPECIES_CONKELDURR) { Ability(ABILITY_GUTS); Moves(MOVE_NUZZLE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_NUZZLE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_NUZZLE, player);
        HP_BAR(opponent);
        NOT STATUS_ICON(opponent, paralysis: TRUE);
    } THEN {
        EXPECT(IsBattlerAbilityActive(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT), ABILITY_SHEER_FORCE));
    }
}

SINGLE_BATTLE_TEST("All Ability Slots lets non-representative Hustle boost physical damage", s16 damage)
{
    bool32 allAbilitySlots;
    PARAMETRIZE { allAbilitySlots = FALSE; }
    PARAMETRIZE { allAbilitySlots = TRUE; }

    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, allAbilitySlots);
        ASSUME(GetSpeciesAbility(SPECIES_DURANT, 0) == ABILITY_SWARM);
        ASSUME(GetSpeciesAbility(SPECIES_DURANT, 1) == ABILITY_HUSTLE);
        ASSUME(IsBattleMovePhysical(MOVE_AERIAL_ACE));
        PLAYER(SPECIES_DURANT) { Ability(ABILITY_SWARM); Moves(MOVE_AERIAL_ACE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_AERIAL_ACE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_GT(results[1].damage, results[0].damage);
    }
}

SINGLE_BATTLE_TEST("All Ability Slots lets non-representative Hustle lower physical accuracy")
{
    PASSES_RANDOMLY(80, 100, RNG_ACCURACY);
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_DURANT, 0) == ABILITY_SWARM);
        ASSUME(GetSpeciesAbility(SPECIES_DURANT, 1) == ABILITY_HUSTLE);
        ASSUME(GetMoveAccuracy(MOVE_SCRATCH) == 100);
        ASSUME(IsBattleMovePhysical(MOVE_SCRATCH));
        PLAYER(SPECIES_DURANT) { Ability(ABILITY_SWARM); Moves(MOVE_SCRATCH); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
    }
}

DOUBLE_BATTLE_TEST("All Ability Slots redirects Electric and Water moves to the fastest non-representative absorber")
{
    enum Move move;
    enum Ability ability, representativeAbility;
    u16 species;

    PARAMETRIZE { move = MOVE_THUNDERBOLT; ability = ABILITY_LIGHTNING_ROD; representativeAbility = ABILITY_STATIC; species = SPECIES_RAICHU; }
    PARAMETRIZE { move = MOVE_WATER_GUN; ability = ABILITY_STORM_DRAIN; representativeAbility = ABILITY_STICKY_HOLD; species = SPECIES_GASTRODON_EAST; }

    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        WITH_CONFIG(B_REDIRECT_ABILITY_IMMUNITY, GEN_5);
        ASSUME(GetMoveType(move) == (ability == ABILITY_LIGHTNING_ROD ? TYPE_ELECTRIC : TYPE_WATER));
        ASSUME(GetSpeciesAbility(species, 0) == representativeAbility);
        ASSUME(GetSpeciesAbility(species, ability == ABILITY_LIGHTNING_ROD ? 2 : 1) == ability);
        PLAYER(SPECIES_WOBBUFFET) { Speed(150); Moves(move); }
        PLAYER(SPECIES_WOBBUFFET) { Speed(50); }
        OPPONENT(species) { Ability(representativeAbility); Speed(200); }
        OPPONENT(species) { Ability(representativeAbility); Speed(100); }
    } WHEN {
        TURN { MOVE(playerLeft, move, target: playerRight); }
    } SCENE {
        NONE_OF {
            HP_BAR(playerRight);
            HP_BAR(opponentRight);
        }
        ABILITY_POPUP(opponentLeft, ability);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, opponentLeft);
    } THEN {
        EXPECT_EQ(opponentLeft->statStages[STAT_SPATK], DEFAULT_STAT_STAGE + 1);
        EXPECT(IsBattlerAbilityActive(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), ability));
    }
}

SINGLE_BATTLE_TEST("All Ability Slots applies non-representative Tough Claws damage", s16 damage)
{
    bool32 allAbilitySlots;
    PARAMETRIZE { allAbilitySlots = FALSE; }
    PARAMETRIZE { allAbilitySlots = TRUE; }

    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, allAbilitySlots);
        ASSUME(GetSpeciesAbility(SPECIES_BARBARACLE, 0) == ABILITY_TOUGH_CLAWS);
        ASSUME(GetSpeciesAbility(SPECIES_BARBARACLE, 1) == ABILITY_SNIPER);
        ASSUME(MoveMakesContact(MOVE_SCRATCH));
        PLAYER(SPECIES_BARBARACLE) { Ability(ABILITY_SNIPER); Moves(MOVE_SCRATCH); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_GT(results[1].damage, results[0].damage);
    }
}

SINGLE_BATTLE_TEST("All Ability Slots applies non-representative Strong Jaw damage", s16 damage)
{
    bool32 allAbilitySlots;
    PARAMETRIZE { allAbilitySlots = FALSE; }
    PARAMETRIZE { allAbilitySlots = TRUE; }

    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, allAbilitySlots);
        ASSUME(GetSpeciesAbility(SPECIES_BRUXISH, 0) == ABILITY_DAZZLING);
        ASSUME(GetSpeciesAbility(SPECIES_BRUXISH, 1) == ABILITY_STRONG_JAW);
        ASSUME(IsBitingMove(MOVE_BITE));
        PLAYER(SPECIES_BRUXISH) { Ability(ABILITY_DAZZLING); Moves(MOVE_BITE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_BITE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_GT(results[1].damage, results[0].damage);
    }
}

SINGLE_BATTLE_TEST("All Ability Slots applies non-representative Sharpness damage", s16 damage)
{
    bool32 allAbilitySlots;
    PARAMETRIZE { allAbilitySlots = FALSE; }
    PARAMETRIZE { allAbilitySlots = TRUE; }

    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, allAbilitySlots);
        ASSUME(GetSpeciesAbility(SPECIES_KLEAVOR, 0) == ABILITY_SWARM);
        ASSUME(GetSpeciesAbility(SPECIES_KLEAVOR, 2) == ABILITY_SHARPNESS);
        ASSUME(IsSlicingMove(MOVE_AERIAL_ACE));
        PLAYER(SPECIES_KLEAVOR) { Ability(ABILITY_SWARM); Moves(MOVE_AERIAL_ACE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_AERIAL_ACE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_GT(results[1].damage, results[0].damage);
    }
}

SINGLE_BATTLE_TEST("All Ability Slots stacks Toxtricity's non-representative Punk Rock and Technician", s16 damage)
{
    bool32 allAbilitySlots;
    PARAMETRIZE { allAbilitySlots = FALSE; }
    PARAMETRIZE { allAbilitySlots = TRUE; }

    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, allAbilitySlots);
        ASSUME(GetSpeciesAbility(SPECIES_TOXTRICITY_AMPED, 0) == ABILITY_PUNK_ROCK);
        ASSUME(GetSpeciesAbility(SPECIES_TOXTRICITY_AMPED, 1) == ABILITY_PLUS);
        ASSUME(GetSpeciesAbility(SPECIES_TOXTRICITY_AMPED, 2) == ABILITY_TECHNICIAN);
        ASSUME(IsSoundMove(MOVE_SNARL));
        ASSUME(GetMovePower(MOVE_SNARL) <= 60);
        PLAYER(SPECIES_TOXTRICITY_AMPED) { Ability(ABILITY_PLUS); Moves(MOVE_SNARL); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SNARL); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_GT(results[1].damage, results[0].damage);
    }
}

SINGLE_BATTLE_TEST("All Ability Slots applies non-representative Adaptability STAB", s16 damage)
{
    bool32 allAbilitySlots;
    PARAMETRIZE { allAbilitySlots = FALSE; }
    PARAMETRIZE { allAbilitySlots = TRUE; }

    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, allAbilitySlots);
        ASSUME(GetSpeciesAbility(SPECIES_CRAWDAUNT, 0) == ABILITY_HYPER_CUTTER);
        ASSUME(GetSpeciesAbility(SPECIES_CRAWDAUNT, 2) == ABILITY_ADAPTABILITY);
        ASSUME(GetSpeciesType(SPECIES_CRAWDAUNT, 0) == TYPE_WATER || GetSpeciesType(SPECIES_CRAWDAUNT, 1) == TYPE_WATER);
        PLAYER(SPECIES_CRAWDAUNT) { Ability(ABILITY_HYPER_CUTTER); Moves(MOVE_WATER_GUN); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_WATER_GUN); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_GT(results[1].damage, results[0].damage);
    }
}

SINGLE_BATTLE_TEST("All Ability Slots applies non-representative Thick Fat damage reduction", s16 damage)
{
    bool32 allAbilitySlots;
    PARAMETRIZE { allAbilitySlots = FALSE; }
    PARAMETRIZE { allAbilitySlots = TRUE; }

    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, allAbilitySlots);
        ASSUME(GetSpeciesAbility(SPECIES_WALREIN, 0) == ABILITY_THICK_FAT);
        ASSUME(GetSpeciesAbility(SPECIES_WALREIN, 1) == ABILITY_ICE_BODY);
        PLAYER(SPECIES_ARCANINE) { Moves(MOVE_FLAMETHROWER); }
        OPPONENT(SPECIES_WALREIN) { Ability(ABILITY_ICE_BODY); }
    } WHEN {
        TURN { MOVE(player, MOVE_FLAMETHROWER); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_LT(results[1].damage, results[0].damage);
    }
}

SINGLE_BATTLE_TEST("All Ability Slots applies non-representative Solid Rock damage reduction", s16 damage)
{
    bool32 allAbilitySlots;
    PARAMETRIZE { allAbilitySlots = FALSE; }
    PARAMETRIZE { allAbilitySlots = TRUE; }

    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, allAbilitySlots);
        ASSUME(GetSpeciesAbility(SPECIES_RHYPERIOR, 0) == ABILITY_LIGHTNING_ROD);
        ASSUME(GetSpeciesAbility(SPECIES_RHYPERIOR, 1) == ABILITY_SOLID_ROCK);
        PLAYER(SPECIES_BLASTOISE) { Moves(MOVE_WATER_GUN); }
        OPPONENT(SPECIES_RHYPERIOR) { Ability(ABILITY_LIGHTNING_ROD); }
    } WHEN {
        TURN { MOVE(player, MOVE_WATER_GUN); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_LT(results[1].damage, results[0].damage);
    }
}

SINGLE_BATTLE_TEST("All Ability Slots applies non-representative Multiscale damage reduction", s16 damage)
{
    bool32 allAbilitySlots;
    PARAMETRIZE { allAbilitySlots = FALSE; }
    PARAMETRIZE { allAbilitySlots = TRUE; }

    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, allAbilitySlots);
        ASSUME(GetSpeciesAbility(SPECIES_DRAGONITE, 0) == ABILITY_INNER_FOCUS);
        ASSUME(GetSpeciesAbility(SPECIES_DRAGONITE, 2) == ABILITY_MULTISCALE);
        PLAYER(SPECIES_GLALIE) { Moves(MOVE_ICE_BEAM); }
        OPPONENT(SPECIES_DRAGONITE) { Ability(ABILITY_INNER_FOCUS); HP(200); MaxHP(200); }
    } WHEN {
        TURN { MOVE(player, MOVE_ICE_BEAM); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_LT(results[1].damage, results[0].damage);
    }
}

DOUBLE_BATTLE_TEST("All Ability Slots applies hidden Plus and Minus partner boosts", s16 damage)
{
    bool32 allAbilitySlots;
    PARAMETRIZE { allAbilitySlots = FALSE; }
    PARAMETRIZE { allAbilitySlots = TRUE; }

    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, allAbilitySlots);
        ASSUME(GetSpeciesAbility(SPECIES_TOXTRICITY_AMPED, 0) == ABILITY_PUNK_ROCK);
        ASSUME(GetSpeciesAbility(SPECIES_TOXTRICITY_AMPED, 1) == ABILITY_PLUS);
        ASSUME(GetSpeciesAbility(SPECIES_TOXTRICITY_LOW_KEY, 0) == ABILITY_PUNK_ROCK);
        ASSUME(GetSpeciesAbility(SPECIES_TOXTRICITY_LOW_KEY, 1) == ABILITY_MINUS);
        PLAYER(SPECIES_TOXTRICITY_AMPED) { Ability(ABILITY_PUNK_ROCK); Moves(MOVE_THUNDERBOLT); }
        PLAYER(SPECIES_TOXTRICITY_LOW_KEY) { Ability(ABILITY_PUNK_ROCK); }
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_THUNDERBOLT, target: opponentLeft); }
    } SCENE {
        HP_BAR(opponentLeft, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_GT(results[1].damage, results[0].damage);
    }
}

DOUBLE_BATTLE_TEST("All Ability Slots applies non-representative Friend Guard partner reduction", s16 damage)
{
    bool32 allAbilitySlots;
    PARAMETRIZE { allAbilitySlots = FALSE; }
    PARAMETRIZE { allAbilitySlots = TRUE; }

    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, allAbilitySlots);
        ASSUME(GetSpeciesAbility(SPECIES_CLEFAIRY, 0) == ABILITY_CUTE_CHARM);
        ASSUME(GetSpeciesAbility(SPECIES_CLEFAIRY, 2) == ABILITY_FRIEND_GUARD);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_CLEFAIRY) { Ability(ABILITY_CUTE_CHARM); }
        OPPONENT(SPECIES_ARCANINE) { Moves(MOVE_FLAMETHROWER); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponentLeft, MOVE_FLAMETHROWER, target: playerLeft); }
    } SCENE {
        HP_BAR(playerLeft, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_LT(results[1].damage, results[0].damage);
    }
}

SINGLE_BATTLE_TEST("All Ability Slots lets non-representative Gale Wings raise Flying priority")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_TALONFLAME, 0) == ABILITY_FLAME_BODY);
        ASSUME(GetSpeciesAbility(SPECIES_TALONFLAME, 2) == ABILITY_GALE_WINGS);
        PLAYER(SPECIES_TALONFLAME) { Ability(ABILITY_FLAME_BODY); Speed(1); Moves(MOVE_WING_ATTACK); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_WING_ATTACK); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_WING_ATTACK, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
    }
}

SINGLE_BATTLE_TEST("All Ability Slots lets non-representative Skill Link force five hits")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_CLOYSTER, 0) == ABILITY_SHELL_ARMOR);
        ASSUME(GetSpeciesAbility(SPECIES_CLOYSTER, 1) == ABILITY_SKILL_LINK);
        ASSUME(IsMultiHitMove(MOVE_ICICLE_SPEAR));
        PLAYER(SPECIES_CLOYSTER) { Ability(ABILITY_SHELL_ARMOR); Moves(MOVE_ICICLE_SPEAR); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1000); MaxHP(1000); }
    } WHEN {
        TURN { MOVE(player, MOVE_ICICLE_SPEAR); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ICICLE_SPEAR, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ICICLE_SPEAR, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ICICLE_SPEAR, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ICICLE_SPEAR, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ICICLE_SPEAR, player);
        MESSAGE("The Pokémon was hit 5 time(s)!");
    }
}

SINGLE_BATTLE_TEST("All Ability Slots lets non-representative Gorilla Tactics lock move selection")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_DARMANITAN_GALAR, 0) == ABILITY_GORILLA_TACTICS);
        ASSUME(GetSpeciesAbility(SPECIES_DARMANITAN_GALAR, 2) == ABILITY_ZEN_MODE);
        PLAYER(SPECIES_DARMANITAN_GALAR) { Ability(ABILITY_ZEN_MODE); Moves(MOVE_SCRATCH, MOVE_FLAME_CHARGE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
        HP_BAR(opponent);
    } THEN {
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        u32 flameChargeSlot = GetMoveSlot(gBattleMons[battler].moves, MOVE_FLAME_CHARGE);

        EXPECT(IsBattlerAbilityActive(battler, ABILITY_GORILLA_TACTICS));
        EXPECT(CheckMoveLimitations(battler, 0, MOVE_LIMITATION_CHOICE_ITEM) & (1u << flameChargeSlot));
    }
}

SINGLE_BATTLE_TEST("All Ability Slots lets non-representative Poison Heal replace poison damage with healing")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_SHROOMISH, 0) == ABILITY_EFFECT_SPORE);
        ASSUME(GetSpeciesAbility(SPECIES_SHROOMISH, 1) == ABILITY_POISON_HEAL);
        PLAYER(SPECIES_SHROOMISH) { Ability(ABILITY_EFFECT_SPORE); Status1(STATUS1_POISON); HP(1); MaxHP(400); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_POISON_HEAL);
        HP_BAR(player, damage: -50);
    } THEN {
        EXPECT(IsBattlerAbilityActive(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT), ABILITY_POISON_HEAL));
    }
}

SINGLE_BATTLE_TEST("All Ability Slots lets non-representative Heatproof reduce burn damage", s16 damage)
{
    bool32 allAbilitySlots;
    PARAMETRIZE { allAbilitySlots = FALSE; }
    PARAMETRIZE { allAbilitySlots = TRUE; }

    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, allAbilitySlots);
        WITH_CONFIG(B_BURN_DAMAGE, GEN_7);
        ASSUME(GetSpeciesAbility(SPECIES_BRONZONG, 0) == ABILITY_LEVITATE);
        ASSUME(GetSpeciesAbility(SPECIES_BRONZONG, 1) == ABILITY_HEATPROOF);
        PLAYER(SPECIES_BRONZONG) { Ability(ABILITY_LEVITATE); Status1(STATUS1_BURN); MaxHP(320); HP(320); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN {}
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_LT(results[1].damage, results[0].damage);
    }
}

SINGLE_BATTLE_TEST("All Ability Slots lets non-representative Protean change type with the correct popup")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        WITH_CONFIG(B_PROTEAN_LIBERO, GEN_6);
        ASSUME(GetSpeciesAbility(SPECIES_GRENINJA, 0) == ABILITY_TORRENT);
        ASSUME(GetSpeciesAbility(SPECIES_GRENINJA, 2) == ABILITY_PROTEAN);
        PLAYER(SPECIES_GRENINJA) { Ability(ABILITY_TORRENT); Moves(MOVE_SCRATCH); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_PROTEAN);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
    } THEN {
        EXPECT_EQ(player->types[0], TYPE_NORMAL);
        EXPECT(IsBattlerAbilityActive(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT), ABILITY_PROTEAN));
    }
}

SINGLE_BATTLE_TEST("All Ability Slots lets non-representative Clear Body block stat drops")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_KLINKLANG, 0) == ABILITY_PLUS);
        ASSUME(GetSpeciesAbility(SPECIES_KLINKLANG, 2) == ABILITY_CLEAR_BODY);
        PLAYER(SPECIES_KLINKLANG) { Ability(ABILITY_PLUS); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_GROWL); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_CLEAR_BODY);
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("All Ability Slots lets non-representative Contrary reverse stat drops")
{
    GIVEN {
        WITH_CONFIG(B_ALL_ABILITY_SLOTS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_SERPERIOR, 0) == ABILITY_OVERGROW);
        ASSUME(GetSpeciesAbility(SPECIES_SERPERIOR, 2) == ABILITY_CONTRARY);
        PLAYER(SPECIES_SERPERIOR) { Ability(ABILITY_OVERGROW); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_GROWL); }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, player);
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE + 1);
    }
}
