#include "global.h"
#include "test/battle.h"
#include "battle_ai_util.h"

AI_SINGLE_BATTLE_TEST("AI_FLAG_SMART_TERA: AI will tera if it enables a ko")
{
    GIVEN {
        ASSUME(GetMovePower(MOVE_SEED_BOMB) == 80);
        ASSUME(GetMovePower(MOVE_AQUA_TAIL) == 90);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_SMART_TERA);
        PLAYER(SPECIES_WOBBUFFET) { HP(47); Speed(100); }
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_AQUA_TAIL, MOVE_SEED_BOMB); TeraType(TYPE_GRASS); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); Speed(100); TeraType(TYPE_FIRE); }
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_SEED_BOMB, gimmick: GIMMICK_TERA); SEND_OUT(player, 1); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet terastallized into the Grass type!");
        MESSAGE("Wobbuffet fainted!");
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_SMART_TERA: AI will not tera if it gets outsped and ko'd")
{
    GIVEN {
        ASSUME(GetMovePower(MOVE_SEED_BOMB) == 80);
        ASSUME(GetMovePower(MOVE_FLAMETHROWER) == 90);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_SMART_TERA | AI_FLAG_OMNISCIENT );
        PLAYER(SPECIES_WOBBUFFET) { HP(47); Speed(100); Moves(MOVE_FLAMETHROWER, MOVE_CELEBRATE); }
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(60); Speed(1); Moves(MOVE_SEED_BOMB); TeraType(TYPE_GRASS); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); Speed(100); TeraType(TYPE_FIRE); }
    } WHEN {
        TURN {}
    } SCENE {
        NOT MESSAGE("The opposing Wobbuffet terastallized into the Grass type!");
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_SMART_TERA: AI will not tera if it gets ko'd by priority")
{
    GIVEN {
        ASSUME(GetMovePower(MOVE_SEED_BOMB) == 80);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_SMART_TERA | AI_FLAG_OMNISCIENT );
        PLAYER(SPECIES_WOBBUFFET) { HP(47); Speed(1); Moves(MOVE_QUICK_ATTACK, MOVE_CELEBRATE); }
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); Speed(100); Moves(MOVE_SEED_BOMB, MOVE_AQUA_TAIL); TeraType(TYPE_GRASS); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); Speed(100); TeraType(TYPE_FIRE); }
    } WHEN {
        TURN {  }
    } SCENE {
        NOT MESSAGE("The opposing Wobbuffet terastallized into the Grass type!");
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_SMART_TERA: AI will tera if it gets saved from a ko")
{
    GIVEN {
        ASSUME(GetMovePower(MOVE_SEED_BOMB) == 80);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_SMART_TERA | AI_FLAG_OMNISCIENT );
        PLAYER(SPECIES_WOBBUFFET) { HP(47); Speed(100); Moves(MOVE_SEED_BOMB); }
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); MaxHP(30); HP(30); Moves(MOVE_SEED_BOMB); TeraType(TYPE_FIRE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); TeraType(TYPE_FIRE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SEED_BOMB); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet terastallized into the Fire type!");
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_SMART_TERA: AI avoids defensive Tera that introduces a new major weakness")
{
    GIVEN {
        ASSUME(GetMoveType(MOVE_FLAMETHROWER) == TYPE_FIRE);
        ASSUME(GetMoveType(MOVE_THUNDERBOLT) == TYPE_ELECTRIC);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_SMART_TERA | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_WOBBUFFET) { Level(50); SpAttack(115); Speed(90); Moves(MOVE_FLAMETHROWER, MOVE_THUNDERBOLT); }
        OPPONENT(SPECIES_VENUSAUR) { Level(50); MaxHP(180); HP(180); SpDefense(120); Speed(100); Moves(MOVE_GIGA_DRAIN); TeraType(TYPE_WATER); }
    } WHEN {
        TURN { MOVE(player, MOVE_FLAMETHROWER); EXPECT_MOVE(opponent, MOVE_GIGA_DRAIN, gimmick: GIMMICK_NONE); }
    } SCENE {
        NOT MESSAGE("The opposing Venusaur terastallized into the Water type!");
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI stays and teras instead of switching from a selected KO it can outpace")
{
    GIVEN {
        ASSUME(GetMoveType(MOVE_SHADOW_BALL) == TYPE_GHOST);
        ASSUME(GetSpeciesType(SPECIES_FLUTTER_MANE, 0) == TYPE_GHOST);
        ASSUME(GetSpeciesType(SPECIES_FLUTTER_MANE, 1) == TYPE_FAIRY);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_SMART_TERA | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_FLUTTER_MANE) { Level(50); MaxHP(130); HP(130); SpAttack(200); SpDefense(155); Speed(180); Moves(MOVE_SHADOW_BALL, MOVE_MOONBLAST); }
        OPPONENT(SPECIES_FLUTTER_MANE) { Level(50); MaxHP(130); HP(130); SpAttack(200); SpDefense(155); Speed(220); Moves(MOVE_MOONBLAST, MOVE_SHADOW_BALL); TeraType(TYPE_FAIRY); }
        OPPONENT(SPECIES_CHI_YU) { Level(50); MaxHP(130); HP(130); SpAttack(200); SpDefense(155); Speed(150); Moves(MOVE_DARK_PULSE); TeraType(TYPE_FIRE); }
    } WHEN {
        TURN {
            MOVE(player, MOVE_SHADOW_BALL);
            EXPECT_MOVE(opponent, MOVE_SHADOW_BALL, gimmick: GIMMICK_TERA);
        }
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI may switch if tera still loses to the selected KO roll")
{
    GIVEN {
        ASSUME(GetMoveType(MOVE_SHADOW_BALL) == TYPE_GHOST);
        ASSUME(GetSpeciesType(SPECIES_FLUTTER_MANE, 0) == TYPE_GHOST);
        ASSUME(GetSpeciesType(SPECIES_FLUTTER_MANE, 1) == TYPE_FAIRY);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_SMART_TERA | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_FLUTTER_MANE) { Level(50); MaxHP(130); HP(130); SpAttack(255); SpDefense(155); Speed(180); Moves(MOVE_SHADOW_BALL, MOVE_MOONBLAST); }
        OPPONENT(SPECIES_FLUTTER_MANE) { Level(50); MaxHP(60); HP(60); SpAttack(200); SpDefense(120); Speed(220); Moves(MOVE_MOONBLAST, MOVE_SHADOW_BALL); TeraType(TYPE_FAIRY); }
        OPPONENT(SPECIES_CHI_YU) { Level(50); MaxHP(130); HP(130); SpAttack(200); SpDefense(155); Speed(150); Moves(MOVE_DARK_PULSE); TeraType(TYPE_FIRE); }
    } WHEN {
        TURN {
            MOVE(player, MOVE_SHADOW_BALL);
            EXPECT_SWITCH(opponent, 1);
        }
    }
}
