#include "global.h"
#include "test/battle.h"

SINGLE_BATTLE_TEST("Damp prevents Explosion-like moves from enemies")
{
    enum Move move;
    PARAMETRIZE { move = MOVE_EXPLOSION; }
    PARAMETRIZE { move = MOVE_SELF_DESTRUCT; }
    PARAMETRIZE { move = MOVE_MIND_BLOWN; }
    PARAMETRIZE { move = MOVE_MISTY_EXPLOSION; }
    GIVEN {
        PLAYER(SPECIES_PARAS) { Ability(ABILITY_DAMP); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, move); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_DAMP);
        NONE_OF { HP_BAR(player); HP_BAR(opponent); }
    }
}

DOUBLE_BATTLE_TEST("Damp prevents Explosion-like moves from enemies in a double battle")
{
    enum Move move;
    PARAMETRIZE { move = MOVE_EXPLOSION; }
    PARAMETRIZE { move = MOVE_SELF_DESTRUCT; }
    PARAMETRIZE { move = MOVE_MIND_BLOWN; }
    PARAMETRIZE { move = MOVE_MISTY_EXPLOSION; }
    GIVEN {
        PLAYER(SPECIES_PARAS) { Ability(ABILITY_DAMP); }
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponentLeft, move); }
    } SCENE {
        ABILITY_POPUP(playerLeft, ABILITY_DAMP);
        NONE_OF { HP_BAR(playerLeft); HP_BAR(opponentLeft); HP_BAR(playerRight); HP_BAR(opponentRight); }
    }
}

DOUBLE_BATTLE_TEST("Damp does not prevent Explosion-like moves after its bearer faints")
{
    GIVEN {
        PLAYER(SPECIES_PARAS) { Ability(ABILITY_DAMP); HP(1); Speed(1); }
        PLAYER(SPECIES_WYNAUT) { HP(1); Speed(1); }
        OPPONENT(SPECIES_ABRA) { Speed(3); }
        OPPONENT(SPECIES_KADABRA) { Speed(2); }
    } WHEN {
        TURN { MOVE(opponentLeft, MOVE_SCRATCH, target: playerLeft); MOVE(opponentRight, MOVE_EXPLOSION); }
    } SCENE {
        HP_BAR(playerLeft, hp: 0);
        NOT ABILITY_POPUP(playerLeft, ABILITY_DAMP);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_EXPLOSION, opponentRight);
        HP_BAR(playerRight, hp: 0);
    }
}

SINGLE_BATTLE_TEST("Damp prevents Explosion-like moves from self")
{
    enum Move move;
    PARAMETRIZE { move = MOVE_EXPLOSION; }
    PARAMETRIZE { move = MOVE_SELF_DESTRUCT; }
    PARAMETRIZE { move = MOVE_MIND_BLOWN; }
    PARAMETRIZE { move = MOVE_MISTY_EXPLOSION; }
    GIVEN {
        PLAYER(SPECIES_PARAS) { Ability(ABILITY_DAMP); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, move); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_DAMP);
        NONE_OF { HP_BAR(player); HP_BAR(opponent); }
    }
}

SINGLE_BATTLE_TEST("Damp prevents damage from Aftermath")
{
    GIVEN {
        ASSUME(MoveMakesContact(MOVE_SCRATCH));
        PLAYER(SPECIES_PARAS) { Ability(ABILITY_DAMP); }
        OPPONENT(SPECIES_VOLTORB) { Ability(ABILITY_AFTERMATH); HP(1); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); SEND_OUT(opponent, 1); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_AFTERMATH);
        ABILITY_POPUP(player, ABILITY_DAMP);
        NONE_OF { HP_BAR(player); }
    }
}

//TO_DO_BATTLE_TEST("Damp affects non-adjacent Pokémon (triples)")
