#include "global.h"
#include "battle.h"
#include "test/battle.h"
#include "battle_ai_util.h"

#define TEST_IVS_PHYSICAL() HPIV(31); AttackIV(31); DefenseIV(31); SpAttackIV(31); SpDefenseIV(31); SpeedIV(31)
#define TEST_IVS_SPECIAL() HPIV(31); AttackIV(0); DefenseIV(31); SpAttackIV(31); SpDefenseIV(31); SpeedIV(31)

#define PARTNER_SYNERGY_AI_FLAGS (AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_HP_AWARE | AI_FLAG_OMNISCIENT | AI_FLAG_DOUBLE_BATTLE)

#define PLAYER_MIRAIDON_THREAT(...) \
    PLAYER(SPECIES_MIRAIDON) { \
        Level(50); Item(ITEM_CHOICE_SPECS); Nature(NATURE_TIMID); \
        TEST_IVS_SPECIAL(); \
        MaxHP(176); HP(176); Defense(120); SpAttack(205); SpDefense(135); Speed(205); \
        Moves(__VA_ARGS__); \
    }

#define PLAYER_RILLABOOM_BULKY(...) \
    PLAYER(SPECIES_RILLABOOM) { \
        Level(50); Item(ITEM_MIRACLE_SEED); Ability(ABILITY_GRASSY_SURGE); Nature(NATURE_ADAMANT); \
        TEST_IVS_PHYSICAL(); \
        MaxHP(207); HP(207); Attack(187); Defense(130); SpDefense(120); Speed(105); \
        Moves(__VA_ARGS__); \
    }

#define PLAYER_INCINEROAR_BULKY(...) \
    PLAYER(SPECIES_INCINEROAR) { \
        Level(50); Item(ITEM_SITRUS_BERRY); Ability(ABILITY_INTIMIDATE); Nature(NATURE_CAREFUL); \
        TEST_IVS_PHYSICAL(); \
        MaxHP(202); HP(202); Attack(135); Defense(120); SpDefense(146); Speed(80); \
        Moves(__VA_ARGS__); \
    }

#define PLAYER_ARCANINE_BULKY(...) \
    PLAYER(SPECIES_ARCANINE) { \
        Level(50); Item(ITEM_SITRUS_BERRY); Ability(ABILITY_INTIMIDATE); Nature(NATURE_CAREFUL); \
        TEST_IVS_PHYSICAL(); \
        MaxHP(197); HP(197); Attack(130); Defense(110); SpDefense(145); Speed(115); \
        Moves(__VA_ARGS__); \
    }

#define PLAYER_VENUSAUR_BULKY(...) \
    PLAYER(SPECIES_VENUSAUR) { \
        Level(50); Item(ITEM_COVERT_CLOAK); Ability(ABILITY_CHLOROPHYLL); Nature(NATURE_CALM); \
        TEST_IVS_SPECIAL(); \
        MaxHP(187); HP(187); Defense(110); SpAttack(120); SpDefense(145); Speed(100); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_AMOONGUSS_SUPPORT(...) \
    OPPONENT(SPECIES_AMOONGUSS) { \
        Level(50); Item(ITEM_ROCKY_HELMET); Ability(ABILITY_REGENERATOR); Nature(NATURE_SASSY); \
        TEST_IVS_SPECIAL(); \
        MaxHP(221); HP(221); Defense(120); SpAttack(105); SpDefense(145); Speed(31); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_AMOONGUSS_DAMAGED_SUPPORT(...) \
    OPPONENT(SPECIES_AMOONGUSS) { \
        Level(50); Item(ITEM_ROCKY_HELMET); Ability(ABILITY_REGENERATOR); Nature(NATURE_SASSY); \
        TEST_IVS_SPECIAL(); \
        MaxHP(221); HP(60); Defense(120); SpAttack(105); SpDefense(145); Speed(31); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_XERNEAS_DAMAGED(...) \
    OPPONENT(SPECIES_XERNEAS) { \
        Level(50); Item(ITEM_POWER_HERB); Nature(NATURE_MODEST); \
        TEST_IVS_SPECIAL(); \
        MaxHP(201); HP(40); Defense(115); SpAttack(183); SpDefense(118); Speed(119); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_XERNEAS_LOW_HP(...) \
    OPPONENT(SPECIES_XERNEAS) { \
        Level(50); Item(ITEM_POWER_HERB); Nature(NATURE_MODEST); \
        TEST_IVS_SPECIAL(); \
        MaxHP(201); HP(30); Defense(115); SpAttack(183); SpDefense(118); Speed(119); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_XERNEAS_SETUP(...) \
    OPPONENT(SPECIES_XERNEAS) { \
        Level(50); Item(ITEM_POWER_HERB); Nature(NATURE_MODEST); \
        TEST_IVS_SPECIAL(); \
        MaxHP(201); HP(201); Defense(115); SpAttack(183); SpDefense(118); Speed(119); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_PELIPPER_SUPPORT(...) \
    OPPONENT(SPECIES_PELIPPER) { \
        Level(50); Item(ITEM_FOCUS_SASH); Ability(ABILITY_KEEN_EYE); Nature(NATURE_CALM); \
        TEST_IVS_SPECIAL(); \
        MaxHP(167); HP(167); Defense(120); SpAttack(50); SpDefense(120); Speed(85); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_PRIMARINA_AQUA_JET_SUPPORT(...) \
    OPPONENT(SPECIES_PRIMARINA) { \
        Level(50); Item(ITEM_COVERT_CLOAK); Ability(ABILITY_LIQUID_VOICE); Nature(NATURE_MODEST); \
        TEST_IVS_SPECIAL(); \
        MaxHP(187); HP(187); Attack(70); Defense(105); SpAttack(195); SpDefense(135); Speed(80); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_ARCANINE_SUPPORT(...) \
    OPPONENT(SPECIES_ARCANINE) { \
        Level(50); Item(ITEM_SITRUS_BERRY); Ability(ABILITY_INTIMIDATE); Nature(NATURE_CAREFUL); \
        TEST_IVS_PHYSICAL(); \
        MaxHP(197); HP(197); Attack(90); Defense(110); SpDefense(145); Speed(115); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_COALOSSAL_POLICY(...) \
    OPPONENT(SPECIES_COALOSSAL) { \
        Level(50); Item(ITEM_WEAKNESS_POLICY); Ability(ABILITY_STEAM_ENGINE); Nature(NATURE_MODEST); \
        TEST_IVS_SPECIAL(); \
        MaxHP(217); HP(217); Defense(140); SpAttack(145); SpDefense(120); Speed(50); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_WEAVILE_FAST(...) \
    OPPONENT(SPECIES_WEAVILE) { \
        Level(50); Item(ITEM_FOCUS_SASH); Ability(ABILITY_PRESSURE); Nature(NATURE_JOLLY); \
        TEST_IVS_PHYSICAL(); \
        MaxHP(145); HP(145); Attack(172); Defense(85); SpDefense(105); Speed(194); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_ARCHALUDON_STAMINA(...) \
    OPPONENT(SPECIES_ARCHALUDON) { \
        Level(50); Item(ITEM_ASSAULT_VEST); Ability(ABILITY_STAMINA); Nature(NATURE_MODEST); \
        TEST_IVS_SPECIAL(); \
        MaxHP(197); HP(197); Defense(165); SpAttack(160); SpDefense(100); Speed(105); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_FROSLASS_FAST(...) \
    OPPONENT(SPECIES_FROSLASS) { \
        Level(50); Item(ITEM_FOCUS_SASH); Ability(ABILITY_CURSED_BODY); Nature(NATURE_TIMID); \
        TEST_IVS_SPECIAL(); \
        MaxHP(145); HP(145); Defense(85); SpAttack(95); SpDefense(95); Speed(178); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_TAUROS_ANGER_POINT(...) \
    OPPONENT(SPECIES_TAUROS) { \
        Level(50); Item(ITEM_LUM_BERRY); Ability(ABILITY_ANGER_POINT); Nature(NATURE_JOLLY); \
        TEST_IVS_PHYSICAL(); \
        MaxHP(181); HP(181); Attack(160); Defense(115); SpDefense(100); Speed(150); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_FARIGIRAF_SUPPORT(...) \
    OPPONENT(SPECIES_FARIGIRAF) { \
        Level(50); Item(ITEM_MENTAL_HERB); Ability(ABILITY_ARMOR_TAIL); Nature(NATURE_MODEST); \
        TEST_IVS_SPECIAL(); \
        MaxHP(227); HP(227); Defense(105); SpAttack(130); SpDefense(110); Speed(80); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_CROBAT_HAZE(...) \
    OPPONENT(SPECIES_CROBAT) { \
        Level(50); Item(ITEM_COVERT_CLOAK); Ability(ABILITY_INNER_FOCUS); Nature(NATURE_TIMID); \
        TEST_IVS_SPECIAL(); \
        MaxHP(175); HP(175); Defense(100); SpAttack(90); SpDefense(100); Speed(200); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_MIRAIDON_DROP(...) \
    OPPONENT(SPECIES_MIRAIDON) { \
        Level(50); Item(ITEM_LIFE_ORB); Nature(NATURE_TIMID); \
        TEST_IVS_SPECIAL(); \
        MaxHP(176); HP(176); Defense(120); SpAttack(205); SpDefense(135); Speed(205); \
        Moves(__VA_ARGS__); \
    }

AI_DOUBLE_BATTLE_TEST("AI heals a low HP setup ally with Pollen Puff")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_POLLEN_PUFF) == EFFECT_HIT_ENEMY_HEAL_ALLY);
        AI_FLAGS(PARTNER_SYNERGY_AI_FLAGS);
        PLAYER_RILLABOOM_BULKY(MOVE_FAKE_OUT, MOVE_WOOD_HAMMER, MOVE_KNOCK_OFF, MOVE_PROTECT);
        PLAYER_VENUSAUR_BULKY(MOVE_SLEEP_POWDER, MOVE_SLUDGE_BOMB, MOVE_EARTH_POWER, MOVE_PROTECT);
        OPPONENT_AMOONGUSS_SUPPORT(MOVE_POLLEN_PUFF, MOVE_SPORE, MOVE_RAGE_POWDER, MOVE_PROTECT);
        OPPONENT_XERNEAS_DAMAGED(MOVE_MOONBLAST, MOVE_DAZZLING_GLEAM, MOVE_GEOMANCY, MOVE_PROTECT);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_PROTECT);
            MOVE(playerRight, MOVE_PROTECT);
            EXPECT_MOVE(opponentLeft, MOVE_POLLEN_PUFF, target: opponentRight);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI does not target itself with Pollen Puff even when it wants healing")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_POLLEN_PUFF) == EFFECT_HIT_ENEMY_HEAL_ALLY);
        ASSUME(GetMoveTarget(MOVE_POLLEN_PUFF) == TARGET_SELECTED);
        AI_FLAGS(PARTNER_SYNERGY_AI_FLAGS | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_INCINEROAR_BULKY(MOVE_PROTECT, MOVE_FAKE_OUT, MOVE_FLARE_BLITZ, MOVE_PARTING_SHOT);
        PLAYER_VENUSAUR_BULKY(MOVE_DAZZLING_GLEAM, MOVE_SLEEP_POWDER, MOVE_SLUDGE_BOMB, MOVE_PROTECT);
        OPPONENT_XERNEAS_SETUP(MOVE_MOONBLAST, MOVE_DAZZLING_GLEAM, MOVE_GEOMANCY, MOVE_PROTECT);
        OPPONENT_AMOONGUSS_DAMAGED_SUPPORT(MOVE_SPORE, MOVE_RAGE_POWDER, MOVE_POLLEN_PUFF, MOVE_PROTECT);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_PROTECT);
            MOVE(playerRight, MOVE_DAZZLING_GLEAM);
            SCORE_LT_VAL(opponentRight, MOVE_POLLEN_PUFF, AI_SCORE_DEFAULT, target: opponentRight);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI uses Rage Powder to protect an ally from a selected single-target KO")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_RAGE_POWDER) == EFFECT_FOLLOW_ME);
        ASSUME(GetMoveTarget(MOVE_RAGE_POWDER) == TARGET_USER);
        ASSUME(GetMoveTarget(MOVE_THUNDERBOLT) == TARGET_SELECTED);
        AI_FLAGS(PARTNER_SYNERGY_AI_FLAGS | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_MIRAIDON_THREAT(MOVE_THUNDERBOLT, MOVE_DRACO_METEOR, MOVE_ELECTRO_DRIFT, MOVE_PROTECT);
        PLAYER_INCINEROAR_BULKY(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_PROTECT);
        OPPONENT_AMOONGUSS_SUPPORT(MOVE_RAGE_POWDER, MOVE_SPORE, MOVE_POLLEN_PUFF, MOVE_PROTECT);
        OPPONENT_XERNEAS_LOW_HP(MOVE_MOONBLAST, MOVE_DAZZLING_GLEAM, MOVE_GEOMANCY, MOVE_PROTECT);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_THUNDERBOLT, target: opponentRight);
            MOVE(playerRight, MOVE_PROTECT);
            EXPECT_MOVE(opponentLeft, MOVE_RAGE_POWDER);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI triggers ally Steam Engine and Weakness Policy with priority Aqua Jet")
{
    GIVEN {
        ASSUME(gItemsInfo[ITEM_WEAKNESS_POLICY].holdEffect == HOLD_EFFECT_WEAKNESS_POLICY);
        ASSUME(GetMoveType(MOVE_AQUA_JET) == TYPE_WATER);
        ASSUME(GetMoveTarget(MOVE_AQUA_JET) == TARGET_SELECTED);
        ASSUME(GetMovePriority(MOVE_AQUA_JET) > 0);
        AI_FLAGS(PARTNER_SYNERGY_AI_FLAGS);
        PLAYER_RILLABOOM_BULKY(MOVE_WOOD_HAMMER, MOVE_KNOCK_OFF, MOVE_FAKE_OUT, MOVE_PROTECT);
        PLAYER_VENUSAUR_BULKY(MOVE_SLEEP_POWDER, MOVE_SLUDGE_BOMB, MOVE_EARTH_POWER, MOVE_PROTECT);
        OPPONENT_PRIMARINA_AQUA_JET_SUPPORT(MOVE_AQUA_JET, MOVE_SPARKLING_ARIA, MOVE_MOONBLAST, MOVE_PROTECT);
        OPPONENT_COALOSSAL_POLICY(MOVE_HEAT_WAVE, MOVE_ROCK_SLIDE, MOVE_EARTH_POWER, MOVE_BODY_PRESS);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_PROTECT);
            MOVE(playerRight, MOVE_PROTECT);
            EXPECT_MOVE(opponentLeft, MOVE_AQUA_JET, target: opponentRight);
        }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_AQUA_JET, opponentLeft);
        ABILITY_POPUP(opponentRight, ABILITY_STEAM_ENGINE);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_EFFECT, opponentRight);
    } THEN {
        EXPECT_EQ(opponentRight->statStages[STAT_SPEED], MAX_STAT_STAGE);
        EXPECT_EQ(opponentRight->statStages[STAT_ATK], DEFAULT_STAT_STAGE + 2);
        EXPECT_EQ(opponentRight->statStages[STAT_SPATK], DEFAULT_STAT_STAGE + 2);
    }
}

AI_DOUBLE_BATTLE_TEST("AI can score a harmless non-priority Fire hit to trigger ally Steam Engine")
{
    GIVEN {
        ASSUME(GetMoveType(MOVE_FLAME_CHARGE) == TYPE_FIRE);
        ASSUME(GetMoveTarget(MOVE_FLAME_CHARGE) == TARGET_SELECTED);
        ASSUME(GetMovePriority(MOVE_FLAME_CHARGE) == 0);
        AI_FLAGS(PARTNER_SYNERGY_AI_FLAGS);
        PLAYER_RILLABOOM_BULKY(MOVE_WOOD_HAMMER, MOVE_KNOCK_OFF, MOVE_FAKE_OUT, MOVE_PROTECT);
        PLAYER_VENUSAUR_BULKY(MOVE_SLEEP_POWDER, MOVE_SLUDGE_BOMB, MOVE_EARTH_POWER, MOVE_PROTECT);
        OPPONENT_ARCANINE_SUPPORT(MOVE_FLAME_CHARGE, MOVE_SNARL, MOVE_PROTECT);
        OPPONENT_COALOSSAL_POLICY(MOVE_HEAT_WAVE, MOVE_ROCK_SLIDE, MOVE_EARTH_POWER, MOVE_BODY_PRESS);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_PROTECT);
            MOVE(playerRight, MOVE_PROTECT);
            SCORE_GT_VAL(opponentLeft, MOVE_FLAME_CHARGE, AI_SCORE_DEFAULT, target: opponentRight);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI does not use unsafe non-priority Water Gun just to trigger ally Steam Engine")
{
    GIVEN {
        ASSUME(gItemsInfo[ITEM_WEAKNESS_POLICY].holdEffect == HOLD_EFFECT_WEAKNESS_POLICY);
        ASSUME(GetMoveType(MOVE_WATER_GUN) == TYPE_WATER);
        ASSUME(GetMoveTarget(MOVE_WATER_GUN) == TARGET_SELECTED);
        ASSUME(GetMovePriority(MOVE_WATER_GUN) == 0);
        AI_FLAGS(PARTNER_SYNERGY_AI_FLAGS);
        PLAYER_RILLABOOM_BULKY(MOVE_WOOD_HAMMER, MOVE_KNOCK_OFF, MOVE_FAKE_OUT, MOVE_PROTECT);
        PLAYER_VENUSAUR_BULKY(MOVE_SLEEP_POWDER, MOVE_SLUDGE_BOMB, MOVE_EARTH_POWER, MOVE_PROTECT);
        OPPONENT_PELIPPER_SUPPORT(MOVE_WATER_GUN, MOVE_PROTECT);
        OPPONENT_COALOSSAL_POLICY(MOVE_HEAT_WAVE, MOVE_ROCK_SLIDE, MOVE_EARTH_POWER, MOVE_BODY_PRESS);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_PROTECT);
            MOVE(playerRight, MOVE_PROTECT);
            SCORE_LT_VAL(opponentLeft, MOVE_WATER_GUN, AI_SCORE_DEFAULT, target: opponentRight);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI triggers ally Stamina when the ally can cash out with Body Press")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_BODY_PRESS) == EFFECT_BODY_PRESS);
        ASSUME(GetMoveTarget(MOVE_QUICK_ATTACK) == TARGET_SELECTED);
        AI_FLAGS(PARTNER_SYNERGY_AI_FLAGS);
        PLAYER_INCINEROAR_BULKY(MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF, MOVE_PARTING_SHOT, MOVE_PROTECT);
        PLAYER_ARCANINE_BULKY(MOVE_FLARE_BLITZ, MOVE_EXTREME_SPEED, MOVE_SNARL, MOVE_PROTECT);
        OPPONENT_WEAVILE_FAST(MOVE_QUICK_ATTACK, MOVE_PROTECT);
        OPPONENT_ARCHALUDON_STAMINA(MOVE_BODY_PRESS, MOVE_FLASH_CANNON, MOVE_DRACO_METEOR, MOVE_SNARL);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_PROTECT);
            MOVE(playerRight, MOVE_PROTECT);
            EXPECT_MOVE(opponentLeft, MOVE_QUICK_ATTACK, target: opponentRight);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI uses a guaranteed critical hit on an Anger Point ally")
{
    GIVEN {
        ASSUME(MoveAlwaysCrits(MOVE_FROST_BREATH));
        ASSUME(GetMoveTarget(MOVE_FROST_BREATH) == TARGET_SELECTED);
        AI_FLAGS(PARTNER_SYNERGY_AI_FLAGS);
        PLAYER_INCINEROAR_BULKY(MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF, MOVE_PARTING_SHOT, MOVE_PROTECT);
        PLAYER_ARCANINE_BULKY(MOVE_FLARE_BLITZ, MOVE_EXTREME_SPEED, MOVE_SNARL, MOVE_PROTECT);
        OPPONENT_FROSLASS_FAST(MOVE_FROST_BREATH, MOVE_PROTECT);
        OPPONENT_TAUROS_ANGER_POINT(MOVE_CLOSE_COMBAT, MOVE_ROCK_SLIDE, MOVE_PROTECT);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_PROTECT);
            MOVE(playerRight, MOVE_PROTECT);
            EXPECT_MOVE(opponentLeft, MOVE_FROST_BREATH, target: opponentRight);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI uses Psych Up on a boosted ally instead of treating the ally boost as a bad target")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_GEOMANCY) == EFFECT_GEOMANCY);
        ASSUME(GetMoveEffect(MOVE_PSYCH_UP) == EFFECT_PSYCH_UP);
        AI_FLAGS(PARTNER_SYNERGY_AI_FLAGS | AI_FLAG_FORCE_SETUP_FIRST_TURN | AI_FLAG_POWERFUL_STATUS);
        PLAYER_INCINEROAR_BULKY(MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF, MOVE_PARTING_SHOT, MOVE_PROTECT);
        PLAYER_ARCANINE_BULKY(MOVE_FLARE_BLITZ, MOVE_EXTREME_SPEED, MOVE_SNARL, MOVE_PROTECT);
        OPPONENT_FARIGIRAF_SUPPORT(MOVE_PSYCH_UP, MOVE_PSYCHIC, MOVE_PROTECT);
        OPPONENT_XERNEAS_SETUP(MOVE_GEOMANCY, MOVE_MOONBLAST, MOVE_DAZZLING_GLEAM, MOVE_PROTECT);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_PROTECT);
            MOVE(playerRight, MOVE_PROTECT);
            EXPECT_MOVE(opponentRight, MOVE_GEOMANCY);
        }
        TURN {
            MOVE(playerLeft, MOVE_PROTECT);
            MOVE(playerRight, MOVE_PROTECT);
            EXPECT_MOVE(opponentLeft, MOVE_PSYCH_UP);
        }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_PSYCH_UP, opponentLeft);
    } THEN {
        EXPECT_EQ(opponentLeft->statStages[STAT_SPATK], DEFAULT_STAT_STAGE + 2);
        EXPECT_EQ(opponentLeft->statStages[STAT_SPDEF], DEFAULT_STAT_STAGE + 2);
        EXPECT_EQ(opponentLeft->statStages[STAT_SPEED], DEFAULT_STAT_STAGE + 2);
    }
}

AI_DOUBLE_BATTLE_TEST("AI uses Haze to reset an ally's Draco Meteor drops")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_HAZE) == EFFECT_HAZE);
        ASSUME_MOVE_EFFECT_STAT_CHANGE(MOVE_DRACO_METEOR, self: TRUE, spAtk: -2);
        AI_FLAGS(PARTNER_SYNERGY_AI_FLAGS | AI_FLAG_READ_PLAYER_MOVE | AI_FLAG_POWERFUL_STATUS);
        PLAYER_INCINEROAR_BULKY(MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF, MOVE_PARTING_SHOT, MOVE_PROTECT);
        PLAYER_ARCANINE_BULKY(MOVE_FLARE_BLITZ, MOVE_EXTREME_SPEED, MOVE_SNARL, MOVE_PROTECT);
        OPPONENT_CROBAT_HAZE(MOVE_HAZE, MOVE_AIR_SLASH, MOVE_PROTECT);
        OPPONENT_MIRAIDON_DROP(MOVE_DRACO_METEOR, MOVE_PROTECT);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_KNOCK_OFF, target: opponentLeft);
            MOVE(playerRight, MOVE_SNARL);
            EXPECT_MOVE(opponentRight, MOVE_DRACO_METEOR, target: playerLeft);
        }
        TURN {
            MOVE(playerLeft, MOVE_PROTECT);
            MOVE(playerRight, MOVE_PROTECT);
            EXPECT_MOVE(opponentLeft, MOVE_HAZE);
        }
    } THEN {
        EXPECT_EQ(opponentRight->statStages[STAT_SPATK], DEFAULT_STAT_STAGE);
    }
}

AI_DOUBLE_BATTLE_TEST("AI can score Clear Smog on a dropped ally when Haze is unavailable")
{
    GIVEN {
        ASSUME(MoveHasAdditionalEffect(MOVE_CLEAR_SMOG, MOVE_EFFECT_CLEAR_SMOG));
        ASSUME_MOVE_EFFECT_STAT_CHANGE(MOVE_DRACO_METEOR, self: TRUE, spAtk: -2);
        AI_FLAGS(PARTNER_SYNERGY_AI_FLAGS | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_INCINEROAR_BULKY(MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF, MOVE_PARTING_SHOT, MOVE_PROTECT);
        PLAYER_ARCANINE_BULKY(MOVE_FLARE_BLITZ, MOVE_EXTREME_SPEED, MOVE_SNARL, MOVE_PROTECT);
        OPPONENT_AMOONGUSS_SUPPORT(MOVE_CLEAR_SMOG, MOVE_PROTECT);
        OPPONENT_MIRAIDON_DROP(MOVE_DRACO_METEOR, MOVE_PROTECT);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_KNOCK_OFF, target: opponentLeft);
            MOVE(playerRight, MOVE_SNARL);
            EXPECT_MOVE(opponentRight, MOVE_DRACO_METEOR, target: playerLeft);
        }
        TURN {
            MOVE(playerLeft, MOVE_PROTECT);
            MOVE(playerRight, MOVE_PROTECT);
            SCORE_GT_VAL(opponentLeft, MOVE_CLEAR_SMOG, AI_SCORE_DEFAULT, target: opponentRight);
        }
    }
}
