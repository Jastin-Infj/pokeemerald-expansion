#include "global.h"
#include "test/battle.h"
#include "battle_ai_main.h"
#include "battle_ai_util.h"
#include "battle_gimmick.h"
#include "move.h"

AI_SINGLE_BATTLE_TEST("AI will not try to lower opposing stats if target is protected by it's ability")
{
    enum Ability ability;
    u32 species, move;

    PARAMETRIZE { ability = ABILITY_SPEED_BOOST;  species = SPECIES_TORCHIC; move = MOVE_SCARY_FACE; }
    PARAMETRIZE { ability = ABILITY_HYPER_CUTTER; species = SPECIES_KRABBY;  move = MOVE_GROWL; }
    PARAMETRIZE { ability = ABILITY_BIG_PECKS;    species = SPECIES_PIDGEY;  move = MOVE_SCREECH; }
    PARAMETRIZE { ability = ABILITY_ILLUMINATE;   species = SPECIES_STARYU;  move = MOVE_SAND_ATTACK; }
    PARAMETRIZE { ability = ABILITY_KEEN_EYE;     species = SPECIES_PIDGEY;  move = MOVE_SAND_ATTACK; }
    PARAMETRIZE { ability = ABILITY_CONTRARY;     species = SPECIES_SNIVY;   move = MOVE_NOBLE_ROAR; }
    PARAMETRIZE { ability = ABILITY_CLEAR_BODY;   species = SPECIES_BELDUM;  move = MOVE_NOBLE_ROAR; }

    GIVEN {
        WITH_CONFIG(B_ILLUMINATE_EFFECT, GEN_9);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_OMNISCIENT);
        PLAYER(species) { Ability(ability); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE, move); }
    } WHEN {
        TURN { SCORE_LT_VAL(opponent, move, AI_SCORE_DEFAULT); }
    }
}

AI_DOUBLE_BATTLE_TEST("AI will not try to lower opposing stats if target is protected by Flower Veil")
{
    enum Move move;

    PARAMETRIZE { move = MOVE_SCARY_FACE; }
    PARAMETRIZE { move = MOVE_GROWL; }
    PARAMETRIZE { move = MOVE_SCREECH; }
    PARAMETRIZE { move = MOVE_SAND_ATTACK; }
    PARAMETRIZE { move = MOVE_SAND_ATTACK; }
    PARAMETRIZE { move = MOVE_NOBLE_ROAR; }
    PARAMETRIZE { move = MOVE_NOBLE_ROAR; }

    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_COMFEY) { Ability(ABILITY_FLOWER_VEIL); }
        PLAYER(SPECIES_BULBASAUR);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE, move); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SCORE_LT_VAL(opponentLeft, move, AI_SCORE_DEFAULT, target: playerRight); }
    }
}

AI_SINGLE_BATTLE_TEST("AI sees No Guard affects semi-invulnerable moves")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_PHANTOM_FORCE) == EFFECT_SEMI_INVULNERABLE);
        ASSUME(GetMovePower(MOVE_PHANTOM_FORCE) == GetMovePower(MOVE_SPECTRAL_THIEF));
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_GOLURK) { Ability(ABILITY_NO_GUARD); Moves(MOVE_DYNAMIC_PUNCH, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_SMEARGLE) { Moves(MOVE_PHANTOM_FORCE, MOVE_SPECTRAL_THIEF); }
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_SPECTRAL_THIEF); }
    }
}

AI_SINGLE_BATTLE_TEST("AI predicts semi-invulnerable entry and chooses a move that can still hit")
{
    enum Move playerMove, expectedMove = MOVE_NONE;

    PARAMETRIZE { playerMove = MOVE_WATER_GUN; expectedMove = MOVE_THUNDERBOLT; }
    PARAMETRIZE { playerMove = MOVE_DIVE;      expectedMove = MOVE_SURF; } // Faster Dive should make AI avoid moves that miss underwater

    PASSES_RANDOMLY(PREDICT_MOVE_CHANCE, 100, RNG_AI_PREDICT_MOVE);
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_DIVE) == EFFECT_SEMI_INVULNERABLE);
        ASSUME(GetMoveTwoTurnAttackStatus(MOVE_DIVE) == STATE_UNDERWATER);
        ASSUME(!MoveDamagesUnderWater(MOVE_THUNDERBOLT));
        ASSUME(MoveDamagesUnderWater(MOVE_SURF));
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_PREDICT_MOVE | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_MAGIKARP) { Speed(2); Moves(playerMove); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_THUNDERBOLT, MOVE_SURF); }
    } WHEN {
        TURN {
            MOVE(player, playerMove);
            EXPECT_MOVE(opponent, expectedMove);
        }
    }
}

AI_SINGLE_BATTLE_TEST("Protect: AI avoids Protect vs Unseen Fist contact (Single)")
{
    static const enum Move protectMoves[] =
    {
        MOVE_PROTECT,
        MOVE_DETECT,
        MOVE_SPIKY_SHIELD,
        MOVE_KINGS_SHIELD,
        MOVE_BANEFUL_BUNKER,
        MOVE_BURNING_BULWARK,
        MOVE_OBSTRUCT,
        MOVE_SILK_TRAP,
    };
    u32 species = SPECIES_NONE;
    enum Ability ability = ABILITY_NONE;
    enum Move protectMove = MOVE_NONE;
    bool32 shouldProtect = FALSE;

    for (u32 paramIdx = 0; paramIdx < ARRAY_COUNT(protectMoves); paramIdx++)
    {
        PARAMETRIZE { species = SPECIES_PIKACHU; ability = ABILITY_STATIC;      shouldProtect = TRUE;  protectMove = protectMoves[paramIdx]; }
        PARAMETRIZE { species = SPECIES_URSHIFU; ability = ABILITY_UNSEEN_FIST; shouldProtect = FALSE; protectMove = protectMoves[paramIdx]; }
    }

    PASSES_RANDOMLY(PREDICT_MOVE_CHANCE, 100, RNG_AI_PREDICT_MOVE);
    GIVEN {
        ASSUME(GetMoveEffect(protectMove) == EFFECT_PROTECT);
        ASSUME(MoveMakesContact(MOVE_TACKLE));
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_PREDICT_MOVE | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(species) { Ability(ability); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(protectMove, MOVE_SCRATCH, MOVE_DISABLE); }
    } WHEN {
        if (shouldProtect)
        {
            TURN {
                MOVE(player, MOVE_TACKLE);
                SCORE_GT(opponent, protectMove, MOVE_SCRATCH);
            }
        }
        else
        {
            TURN {
                MOVE(player, MOVE_TACKLE);
                SCORE_LT(opponent, protectMove, MOVE_SCRATCH);
            }
        }
    }
}

AI_DOUBLE_BATTLE_TEST("Protect: AI avoids Protect vs Unseen Fist contact (Doubles)")
{
    static const enum Move protectMoves[] =
    {
        MOVE_PROTECT,
        MOVE_DETECT,
        MOVE_SPIKY_SHIELD,
        MOVE_KINGS_SHIELD,
        MOVE_BANEFUL_BUNKER,
        MOVE_BURNING_BULWARK,
        MOVE_OBSTRUCT,
        MOVE_SILK_TRAP,
    };
    u32 species = SPECIES_NONE;
    enum Ability ability = ABILITY_NONE;
    enum Move protectMove = MOVE_NONE;
    bool32 shouldProtect = FALSE;

    for (u32 paramIdx = 0; paramIdx < ARRAY_COUNT(protectMoves); paramIdx++)
    {
        PARAMETRIZE { species = SPECIES_PIKACHU; ability = ABILITY_STATIC;      shouldProtect = TRUE;  protectMove = protectMoves[paramIdx]; }
        PARAMETRIZE { species = SPECIES_URSHIFU; ability = ABILITY_UNSEEN_FIST; shouldProtect = FALSE; protectMove = protectMoves[paramIdx]; }
    }

    PASSES_RANDOMLY(PREDICT_MOVE_CHANCE, 100, RNG_AI_PREDICT_MOVE);
    GIVEN {
        ASSUME(GetMoveEffect(protectMove) == EFFECT_PROTECT);
        ASSUME(MoveMakesContact(MOVE_TACKLE));
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_PREDICT_MOVE);
        PLAYER(species) { Ability(ability); Moves(MOVE_TACKLE); }
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(protectMove, MOVE_SCRATCH); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_SCRATCH); }
    } WHEN {
        if (shouldProtect)
        {
            TURN {
                MOVE(playerLeft, MOVE_TACKLE, target: opponentLeft);
                MOVE(playerRight, MOVE_CELEBRATE);
                SCORE_GT(opponentLeft, protectMove, MOVE_SCRATCH, target: playerLeft);
            }
        }
        else
        {
            TURN {
                MOVE(playerLeft, MOVE_TACKLE, target: opponentLeft);
                MOVE(playerRight, MOVE_CELEBRATE);
                SCORE_LT(opponentLeft, protectMove, MOVE_SCRATCH, target: playerLeft);
            }
        }
    }
}

AI_SINGLE_BATTLE_TEST("Protect: AI avoids Protect vs moves that ignore protection (Single)")
{
    enum Move move = MOVE_NONE;
    bool32 shouldProtect = FALSE;

    PARAMETRIZE { move = MOVE_TACKLE; shouldProtect = TRUE; }
    PARAMETRIZE { move = MOVE_FEINT; shouldProtect = FALSE; }
    PARAMETRIZE { move = MOVE_SHADOW_FORCE; shouldProtect = FALSE; }
    PARAMETRIZE { move = MOVE_PHANTOM_FORCE; shouldProtect = FALSE; }
    PARAMETRIZE { move = MOVE_HYPERSPACE_HOLE; shouldProtect = FALSE; }
    PARAMETRIZE { move = MOVE_HYPERSPACE_FURY; shouldProtect = FALSE; }

    PASSES_RANDOMLY(PREDICT_MOVE_CHANCE, 100, RNG_AI_PREDICT_MOVE);
    GIVEN {
        if (shouldProtect)
            ASSUME(!MoveIgnoresProtect(move));
        else
            ASSUME(MoveIgnoresProtect(move));
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_PREDICT_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(move); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_PROTECT, MOVE_SCRATCH, MOVE_DISABLE); }
    } WHEN {
        TURN {
            MOVE(player, move);
            if (shouldProtect)
                SCORE_GT(opponent, MOVE_PROTECT, MOVE_SCRATCH);
            else
                SCORE_LT(opponent, MOVE_PROTECT, MOVE_SCRATCH);
        }
    }
}

AI_SINGLE_BATTLE_TEST("Protect: AI avoids passive Protect in singles without a turn-gain payoff")
{
    PASSES_RANDOMLY(PREDICT_MOVE_CHANCE, 100, RNG_AI_PREDICT_MOVE);
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_PREDICT_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_PROTECT, MOVE_SCRATCH); }
    } WHEN {
        TURN {
            MOVE(player, MOVE_TACKLE);
            SCORE_LT(opponent, MOVE_PROTECT, MOVE_SCRATCH);
        }
    }
}

AI_SINGLE_BATTLE_TEST("Protect: AI avoids passive Protect against a boosted singles attacker")
{
    PASSES_RANDOMLY(PREDICT_MOVE_CHANCE, 100, RNG_AI_PREDICT_MOVE);
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_PREDICT_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_SWORDS_DANCE, MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_PROTECT, MOVE_SCRATCH); }
    } WHEN {
        TURN { MOVE(player, MOVE_SWORDS_DANCE); EXPECT_MOVE(opponent, MOVE_SCRATCH); }
        TURN {
            MOVE(player, MOVE_TACKLE);
            SCORE_LT(opponent, MOVE_PROTECT, MOVE_SCRATCH);
        }
    }
}

AI_SINGLE_BATTLE_TEST("Protect: AI values singles Protect when residual damage creates payoff")
{
    PASSES_RANDOMLY(PREDICT_MOVE_CHANCE, 100, RNG_AI_PREDICT_MOVE);
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_PREDICT_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { Status1(STATUS1_TOXIC_POISON); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_PROTECT, MOVE_SCRATCH); }
    } WHEN {
        TURN {
            MOVE(player, MOVE_TACKLE);
            SCORE_GT_VAL(opponent, MOVE_PROTECT, AI_SCORE_DEFAULT);
        }
    }
}

AI_SINGLE_BATTLE_TEST("Protect: AI values singles Protect when Grassy Terrain recovery creates payoff")
{
    PASSES_RANDOMLY(PREDICT_MOVE_CHANCE, 100, RNG_AI_PREDICT_MOVE);
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_GRASSY_TERRAIN) == EFFECT_GRASSY_TERRAIN);
        SetStartingStatus(STARTING_STATUS_GRASSY_TERRAIN_TEMPORARY);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_PREDICT_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(160); HP(80); Moves(MOVE_PROTECT, MOVE_SCRATCH); }
    } WHEN {
        TURN {
            MOVE(player, MOVE_TACKLE);
            SCORE_GT_VAL(opponent, MOVE_PROTECT, AI_SCORE_DEFAULT);
        }
    }
}

AI_SINGLE_BATTLE_TEST("Protect: AI values singles Protect when Leftovers recovery creates payoff")
{
    PASSES_RANDOMLY(PREDICT_MOVE_CHANCE, 100, RNG_AI_PREDICT_MOVE);
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_PREDICT_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(160); HP(80); Item(ITEM_LEFTOVERS); Moves(MOVE_PROTECT, MOVE_SCRATCH); }
    } WHEN {
        TURN {
            MOVE(player, MOVE_TACKLE);
            SCORE_GT_VAL(opponent, MOVE_PROTECT, AI_SCORE_DEFAULT);
        }
    }
}

AI_SINGLE_BATTLE_TEST("Protect: AI values singles Protect when Sitrus Berry threshold follows end-turn chip")
{
    PASSES_RANDOMLY(PREDICT_MOVE_CHANCE, 100, RNG_AI_PREDICT_MOVE);
    GIVEN {
        ASSUME(gItemsInfo[ITEM_SITRUS_BERRY].holdEffect == HOLD_EFFECT_RESTORE_PCT_HP);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_PREDICT_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(100); HP(51); Status1(STATUS1_POISON); Item(ITEM_SITRUS_BERRY); Moves(MOVE_PROTECT, MOVE_SCRATCH); }
    } WHEN {
        TURN {
            MOVE(player, MOVE_TACKLE);
            SCORE_GT_VAL(opponent, MOVE_PROTECT, AI_SCORE_DEFAULT);
        }
    }
}

AI_SINGLE_BATTLE_TEST("Protect: AI recovery estimate includes Leech Seed drain from a seeded target")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_LEECH_SEED) == EFFECT_LEECH_SEED);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(160); HP(160); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(100); HP(40); Moves(MOVE_PROTECT, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); FORCED_MOVE(opponent); }
    } THEN {
        enum BattlerId aiBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);

        gBattleMons[playerBattler].volatiles.leechSeed = LEECHSEEDED_BY(aiBattler);
        gBattleMons[aiBattler].hp = 40;

        EXPECT_EQ(Test_GetProtectEndTurnRecovery(aiBattler), 20);
    }
}

AI_SINGLE_BATTLE_TEST("Protect: AI values singles Protect to burn the last opposing Tailwind turn")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_TAILWIND) == EFFECT_TAILWIND);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(999); HP(999); Speed(80); Moves(MOVE_DRAGON_RAGE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(120); HP(40); Speed(120); Moves(MOVE_PROTECT, MOVE_SCRATCH, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); FORCED_MOVE(opponent); }
    } THEN {
        enum BattlerId aiBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);

        gSideStatuses[B_SIDE_PLAYER] |= SIDE_STATUS_TAILWIND;
        gSideTimers[B_SIDE_PLAYER].tailwindTimer = 1;
        gBattleMons[aiBattler].hp = 40;

        EXPECT(ShouldUseSinglesProtect(aiBattler, playerBattler, MOVE_DRAGON_RAGE));
        EXPECT(ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_DRAGON_RAGE) > 0);
    }
}

AI_SINGLE_BATTLE_TEST("Protect: AI values singles Protect to burn the last Trick Room turn")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_TRICK_ROOM) == EFFECT_TRICK_ROOM);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(999); HP(999); Speed(80); Moves(MOVE_DRAGON_RAGE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(120); HP(40); Speed(120); Moves(MOVE_PROTECT, MOVE_SCRATCH, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); FORCED_MOVE(opponent); }
    } THEN {
        enum BattlerId aiBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);

        gFieldStatuses |= STATUS_FIELD_TRICK_ROOM;
        gFieldTimers.trickRoomTimer = 1;
        gBattleMons[aiBattler].hp = 40;

        EXPECT(ShouldUseSinglesProtect(aiBattler, playerBattler, MOVE_DRAGON_RAGE));
        EXPECT(ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_DRAGON_RAGE) > 0);
    }
}

AI_DOUBLE_BATTLE_TEST("Protect: AI values doubles Protect when the partner can punish final opposing Tailwind")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_TAILWIND) == EFFECT_TAILWIND);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE | AI_FLAG_DOUBLE_BATTLE);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(40); HP(40); Speed(80); Moves(MOVE_DRAGON_RAGE, MOVE_CELEBRATE); }
        PLAYER(SPECIES_SHUCKLE) { Speed(20); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(120); HP(40); Speed(60); Moves(MOVE_PROTECT, MOVE_SCRATCH, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(120); HP(120); Speed(120); Moves(MOVE_DRAGON_RAGE, MOVE_CELEBRATE); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_CELEBRATE);
            MOVE(playerRight, MOVE_CELEBRATE);
            FORCED_MOVE(opponentLeft);
            FORCED_MOVE(opponentRight);
        }
    } THEN {
        enum BattlerId aiBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        s32 scoreWithTimer;
        s32 scoreWithoutTimer;

        gSideStatuses[B_SIDE_PLAYER] |= SIDE_STATUS_TAILWIND;
        gSideTimers[B_SIDE_PLAYER].tailwindTimer = 1;
        BattleAI_SetupAIData(0xF, aiBattler);
        scoreWithTimer = ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_DRAGON_RAGE);

        gSideStatuses[B_SIDE_PLAYER] &= ~SIDE_STATUS_TAILWIND;
        gSideTimers[B_SIDE_PLAYER].tailwindTimer = 0;
        BattleAI_SetupAIData(0xF, aiBattler);
        scoreWithoutTimer = ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_DRAGON_RAGE);

        EXPECT_GT(scoreWithTimer, scoreWithoutTimer);
        EXPECT_GT(scoreWithTimer, 0);
    }
}

AI_DOUBLE_BATTLE_TEST("Protect: AI values doubles Protect when the partner can punish final Trick Room")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_TRICK_ROOM) == EFFECT_TRICK_ROOM);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE | AI_FLAG_DOUBLE_BATTLE);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(40); HP(40); Speed(80); Moves(MOVE_DRAGON_RAGE, MOVE_CELEBRATE); }
        PLAYER(SPECIES_SHUCKLE) { Speed(20); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(120); HP(40); Speed(60); Moves(MOVE_PROTECT, MOVE_SCRATCH, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(120); HP(120); Speed(120); Moves(MOVE_DRAGON_RAGE, MOVE_CELEBRATE); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_CELEBRATE);
            MOVE(playerRight, MOVE_CELEBRATE);
            FORCED_MOVE(opponentLeft);
            FORCED_MOVE(opponentRight);
        }
    } THEN {
        enum BattlerId aiBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        s32 scoreWithTimer;
        s32 scoreWithoutTimer;

        gFieldStatuses |= STATUS_FIELD_TRICK_ROOM;
        gFieldTimers.trickRoomTimer = 1;
        BattleAI_SetupAIData(0xF, aiBattler);
        scoreWithTimer = ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_DRAGON_RAGE);

        gFieldStatuses &= ~STATUS_FIELD_TRICK_ROOM;
        gFieldTimers.trickRoomTimer = 0;
        BattleAI_SetupAIData(0xF, aiBattler);
        scoreWithoutTimer = ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_DRAGON_RAGE);

        EXPECT_GT(scoreWithTimer, scoreWithoutTimer);
        EXPECT_GT(scoreWithTimer, 0);
    }
}

AI_DOUBLE_BATTLE_TEST("Protect: AI does not waste doubles final Tailwind while burning Trick Room without partner payoff")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_TAILWIND) == EFFECT_TAILWIND);
        ASSUME(GetMoveEffect(MOVE_TRICK_ROOM) == EFFECT_TRICK_ROOM);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE | AI_FLAG_DOUBLE_BATTLE);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(40); HP(40); Speed(80); Moves(MOVE_DRAGON_RAGE, MOVE_CELEBRATE); }
        PLAYER(SPECIES_SHUCKLE) { Speed(20); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(120); HP(40); Speed(60); Moves(MOVE_PROTECT, MOVE_SCRATCH, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(120); HP(120); Speed(20); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_CELEBRATE);
            MOVE(playerRight, MOVE_CELEBRATE);
            FORCED_MOVE(opponentLeft);
            FORCED_MOVE(opponentRight);
        }
    } THEN {
        enum BattlerId aiBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);

        gFieldStatuses |= STATUS_FIELD_TRICK_ROOM;
        gFieldTimers.trickRoomTimer = 1;
        gSideStatuses[B_SIDE_OPPONENT] |= SIDE_STATUS_TAILWIND;
        gSideTimers[B_SIDE_OPPONENT].tailwindTimer = 1;
        BattleAI_SetupAIData(0xF, aiBattler);

        EXPECT_EQ(ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_DRAGON_RAGE), NO_DAMAGE_OR_FAILS);
    }
}

AI_DOUBLE_BATTLE_TEST("Protect: AI values doubles Protect when the partner can punish final opposing screen")
{
    GIVEN {
        ASSUME(GetMoveCategory(MOVE_DOUBLE_EDGE) == DAMAGE_CATEGORY_PHYSICAL);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE | AI_FLAG_DOUBLE_BATTLE);
        PLAYER(SPECIES_WOBBUFFET) { Level(50); MaxHP(180); HP(180); Defense(100); Speed(80); Moves(MOVE_DRAGON_RAGE, MOVE_CELEBRATE); }
        PLAYER(SPECIES_SHUCKLE) { Speed(20); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Level(50); MaxHP(120); HP(40); Defense(100); Speed(120); Moves(MOVE_PROTECT, MOVE_SCRATCH, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Level(50); MaxHP(120); HP(120); Attack(240); Speed(120); Moves(MOVE_DOUBLE_EDGE, MOVE_CELEBRATE); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_CELEBRATE);
            MOVE(playerRight, MOVE_CELEBRATE);
            FORCED_MOVE(opponentLeft);
            FORCED_MOVE(opponentRight);
        }
    } THEN {
        enum BattlerId aiBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        s32 scoreWithScreen;
        s32 scoreWithoutScreen;

        gSideStatuses[B_SIDE_PLAYER] |= SIDE_STATUS_REFLECT;
        gSideTimers[B_SIDE_PLAYER].reflectTimer = 1;
        BattleAI_SetupAIData(0xF, aiBattler);
        scoreWithScreen = ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_DRAGON_RAGE);

        gSideStatuses[B_SIDE_PLAYER] &= ~SIDE_STATUS_REFLECT;
        gSideTimers[B_SIDE_PLAYER].reflectTimer = 0;
        BattleAI_SetupAIData(0xF, aiBattler);
        scoreWithoutScreen = ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_DRAGON_RAGE);

        EXPECT_GT(scoreWithScreen, scoreWithoutScreen);
        EXPECT_GT(scoreWithScreen, 0);
    }
}

AI_DOUBLE_BATTLE_TEST("Protect: AI does not burn final opposing screen when the partner can break it")
{
    GIVEN {
        ASSUME(MoveHasAdditionalEffect(MOVE_BRICK_BREAK, MOVE_EFFECT_BREAK_SCREEN));
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE | AI_FLAG_DOUBLE_BATTLE);
        PLAYER(SPECIES_WOBBUFFET) { Level(50); MaxHP(120); HP(120); Defense(100); Speed(80); Moves(MOVE_DRAGON_RAGE, MOVE_CELEBRATE); }
        PLAYER(SPECIES_SHUCKLE) { Speed(20); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Level(50); MaxHP(120); HP(40); Defense(100); Speed(120); Moves(MOVE_PROTECT, MOVE_SCRATCH, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Level(50); MaxHP(120); HP(120); Attack(240); Speed(120); Moves(MOVE_BRICK_BREAK, MOVE_CELEBRATE); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_CELEBRATE);
            MOVE(playerRight, MOVE_CELEBRATE);
            FORCED_MOVE(opponentLeft);
            FORCED_MOVE(opponentRight);
        }
    } THEN {
        enum BattlerId aiBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        s32 scoreWithScreen;
        s32 scoreWithoutScreen;

        gSideStatuses[B_SIDE_PLAYER] |= SIDE_STATUS_REFLECT;
        gSideTimers[B_SIDE_PLAYER].reflectTimer = 1;
        BattleAI_SetupAIData(0xF, aiBattler);
        scoreWithScreen = ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_DRAGON_RAGE);

        gSideStatuses[B_SIDE_PLAYER] &= ~SIDE_STATUS_REFLECT;
        gSideTimers[B_SIDE_PLAYER].reflectTimer = 0;
        BattleAI_SetupAIData(0xF, aiBattler);
        scoreWithoutScreen = ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_DRAGON_RAGE);

        EXPECT_LE(scoreWithScreen, scoreWithoutScreen);
    }
}

AI_DOUBLE_BATTLE_TEST("Protect: AI values doubles Protect when final Electric Terrain lets the partner punish")
{
    GIVEN {
        ASSUME(GetMoveType(MOVE_THUNDERBOLT) == TYPE_ELECTRIC);
        ASSUME(GetMoveCategory(MOVE_THUNDERBOLT) == DAMAGE_CATEGORY_SPECIAL);
        ASSUME(GetMoveCategory(MOVE_DOUBLE_EDGE) == DAMAGE_CATEGORY_PHYSICAL);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE | AI_FLAG_DOUBLE_BATTLE);
        PLAYER(SPECIES_WOBBUFFET) { Level(50); MaxHP(120); HP(120); Defense(100); SpAttack(240); SpDefense(100); Speed(80); Moves(MOVE_THUNDERBOLT, MOVE_CELEBRATE); }
        PLAYER(SPECIES_SHUCKLE) { Speed(20); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Level(50); MaxHP(120); HP(80); Defense(100); SpDefense(100); Speed(120); Moves(MOVE_PROTECT, MOVE_SCRATCH, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Level(50); MaxHP(120); HP(120); Attack(240); Speed(120); Moves(MOVE_DOUBLE_EDGE, MOVE_CELEBRATE); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_CELEBRATE);
            MOVE(playerRight, MOVE_CELEBRATE);
            FORCED_MOVE(opponentLeft);
            FORCED_MOVE(opponentRight);
        }
    } THEN {
        enum BattlerId aiBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        s32 scoreWithFinalTerrain;
        s32 scoreWithoutTerrain;

        gFieldStatuses |= STATUS_FIELD_ELECTRIC_TERRAIN;
        gFieldTimers.terrainTimer = 1;
        BattleAI_SetupAIData(0xF, aiBattler);
        scoreWithFinalTerrain = ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_THUNDERBOLT);

        gFieldStatuses &= ~STATUS_FIELD_ELECTRIC_TERRAIN;
        gFieldTimers.terrainTimer = 0;
        BattleAI_SetupAIData(0xF, aiBattler);
        scoreWithoutTerrain = ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_THUNDERBOLT);

        EXPECT_GT(scoreWithFinalTerrain, scoreWithoutTerrain);
        EXPECT_GT(scoreWithFinalTerrain, 0);
    }
}

AI_DOUBLE_BATTLE_TEST("Protect: AI does not burn nonfinal Electric Terrain for a partner attack")
{
    GIVEN {
        ASSUME(GetMoveType(MOVE_THUNDERBOLT) == TYPE_ELECTRIC);
        ASSUME(GetMoveCategory(MOVE_THUNDERBOLT) == DAMAGE_CATEGORY_SPECIAL);
        ASSUME(GetMoveCategory(MOVE_DOUBLE_EDGE) == DAMAGE_CATEGORY_PHYSICAL);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE | AI_FLAG_DOUBLE_BATTLE);
        PLAYER(SPECIES_WOBBUFFET) { Level(50); MaxHP(120); HP(120); Defense(100); SpAttack(240); SpDefense(100); Speed(80); Moves(MOVE_THUNDERBOLT, MOVE_CELEBRATE); }
        PLAYER(SPECIES_SHUCKLE) { Speed(20); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Level(50); MaxHP(120); HP(80); Defense(100); SpDefense(100); Speed(120); Moves(MOVE_PROTECT, MOVE_SCRATCH, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Level(50); MaxHP(120); HP(120); Attack(240); Speed(120); Moves(MOVE_DOUBLE_EDGE, MOVE_CELEBRATE); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_CELEBRATE);
            MOVE(playerRight, MOVE_CELEBRATE);
            FORCED_MOVE(opponentLeft);
            FORCED_MOVE(opponentRight);
        }
    } THEN {
        enum BattlerId aiBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        s32 scoreWithNonfinalTerrain;
        s32 scoreWithoutTerrain;

        gFieldStatuses |= STATUS_FIELD_ELECTRIC_TERRAIN;
        gFieldTimers.terrainTimer = 2;
        BattleAI_SetupAIData(0xF, aiBattler);
        scoreWithNonfinalTerrain = ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_THUNDERBOLT);

        gFieldStatuses &= ~STATUS_FIELD_ELECTRIC_TERRAIN;
        gFieldTimers.terrainTimer = 0;
        BattleAI_SetupAIData(0xF, aiBattler);
        scoreWithoutTerrain = ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_THUNDERBOLT);

        EXPECT_EQ(scoreWithNonfinalTerrain, scoreWithoutTerrain);
    }
}

AI_DOUBLE_BATTLE_TEST("Protect: AI values doubles Protect while opposing Perish count expires")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE | AI_FLAG_DOUBLE_BATTLE);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(40); HP(40); Speed(80); Moves(MOVE_DRAGON_RAGE, MOVE_CELEBRATE); }
        PLAYER(SPECIES_SHUCKLE) { Speed(20); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(120); HP(40); Speed(120); Moves(MOVE_PROTECT, MOVE_SCRATCH, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_SHUCKLE) { Speed(20); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_CELEBRATE);
            MOVE(playerRight, MOVE_CELEBRATE);
            FORCED_MOVE(opponentLeft);
            FORCED_MOVE(opponentRight);
        }
    } THEN {
        enum BattlerId aiBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        s32 scoreWithPerish;
        s32 scoreWithoutPerish;

        gBattleMons[playerBattler].volatiles.perishSong = TRUE;
        gBattleMons[playerBattler].volatiles.perishSongTimer = 0;
        BattleAI_SetupAIData(0xF, aiBattler);
        scoreWithPerish = ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_DRAGON_RAGE);

        gBattleMons[playerBattler].volatiles.perishSong = FALSE;
        gBattleMons[playerBattler].volatiles.perishSongTimer = 0;
        BattleAI_SetupAIData(0xF, aiBattler);
        scoreWithoutPerish = ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_DRAGON_RAGE);

        EXPECT_GT(scoreWithPerish, scoreWithoutPerish);
        EXPECT_GT(scoreWithPerish, 0);
    }
}

AI_DOUBLE_BATTLE_TEST("Protect: AI does not value doubles Protect when both Perish counts expire")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE | AI_FLAG_DOUBLE_BATTLE);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(40); HP(40); Speed(80); Moves(MOVE_DRAGON_RAGE, MOVE_CELEBRATE); }
        PLAYER(SPECIES_SHUCKLE) { Speed(20); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(120); HP(40); Speed(120); Moves(MOVE_PROTECT, MOVE_SCRATCH, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_SHUCKLE) { Speed(20); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_CELEBRATE);
            MOVE(playerRight, MOVE_CELEBRATE);
            FORCED_MOVE(opponentLeft);
            FORCED_MOVE(opponentRight);
        }
    } THEN {
        enum BattlerId aiBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        s32 scoreWithBothPerish;
        s32 scoreWithoutPerish;

        gBattleMons[playerBattler].volatiles.perishSong = TRUE;
        gBattleMons[playerBattler].volatiles.perishSongTimer = 0;
        gBattleMons[aiBattler].volatiles.perishSong = TRUE;
        gBattleMons[aiBattler].volatiles.perishSongTimer = 0;
        BattleAI_SetupAIData(0xF, aiBattler);
        scoreWithBothPerish = ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_DRAGON_RAGE);

        gBattleMons[playerBattler].volatiles.perishSong = FALSE;
        gBattleMons[aiBattler].volatiles.perishSong = FALSE;
        BattleAI_SetupAIData(0xF, aiBattler);
        scoreWithoutPerish = ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_DRAGON_RAGE);

        EXPECT_LE(scoreWithBothPerish, scoreWithoutPerish);
    }
}

AI_SINGLE_BATTLE_TEST("Protect: AI values singles Protect to burn the last opposing screen turn")
{
    enum Move aiMove;
    u32 screenStatus;

    PARAMETRIZE { aiMove = MOVE_DOUBLE_EDGE; screenStatus = SIDE_STATUS_REFLECT; }
    PARAMETRIZE { aiMove = MOVE_PSYCHIC;     screenStatus = SIDE_STATUS_LIGHTSCREEN; }
    PARAMETRIZE { aiMove = MOVE_DOUBLE_EDGE; screenStatus = SIDE_STATUS_AURORA_VEIL; }

    GIVEN {
        ASSUME(GetMoveCategory(MOVE_DOUBLE_EDGE) == DAMAGE_CATEGORY_PHYSICAL);
        ASSUME(GetMoveCategory(MOVE_PSYCHIC) == DAMAGE_CATEGORY_SPECIAL);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { Level(50); MaxHP(80); HP(80); Defense(100); SpDefense(100); Speed(80); Moves(MOVE_DRAGON_RAGE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Level(50); MaxHP(120); HP(40); Attack(200); SpAttack(200); Speed(120); Moves(MOVE_PROTECT, aiMove, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); FORCED_MOVE(opponent); }
    } THEN {
        enum BattlerId aiBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);

        gSideStatuses[B_SIDE_PLAYER] |= screenStatus;
        if (screenStatus == SIDE_STATUS_REFLECT)
            gSideTimers[B_SIDE_PLAYER].reflectTimer = 1;
        else if (screenStatus == SIDE_STATUS_LIGHTSCREEN)
            gSideTimers[B_SIDE_PLAYER].lightscreenTimer = 1;
        else
            gSideTimers[B_SIDE_PLAYER].auroraVeilTimer = 1;

        gBattleMons[aiBattler].hp = 40;

        EXPECT(ShouldUseSinglesProtect(aiBattler, playerBattler, MOVE_DRAGON_RAGE));
        EXPECT(ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_DRAGON_RAGE) > 0);
    }
}

AI_SINGLE_BATTLE_TEST("Protect: AI does not burn the last opposing screen turn when it can break screens")
{
    GIVEN {
        ASSUME(MoveHasAdditionalEffect(MOVE_BRICK_BREAK, MOVE_EFFECT_BREAK_SCREEN));
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { Level(50); MaxHP(80); HP(80); Defense(100); Speed(80); Moves(MOVE_DRAGON_RAGE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Level(50); MaxHP(120); HP(40); Attack(200); Speed(120); Moves(MOVE_PROTECT, MOVE_BRICK_BREAK, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); FORCED_MOVE(opponent); }
    } THEN {
        enum BattlerId aiBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);

        gSideStatuses[B_SIDE_PLAYER] |= SIDE_STATUS_REFLECT;
        gSideTimers[B_SIDE_PLAYER].reflectTimer = 1;
        gBattleMons[aiBattler].hp = 40;

        EXPECT(!ShouldUseSinglesProtect(aiBattler, playerBattler, MOVE_DRAGON_RAGE));
        EXPECT(ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_DRAGON_RAGE) <= 0);
    }
}

AI_SINGLE_BATTLE_TEST("Protect: AI does not burn opposing screens when its own final screen is needed")
{
    GIVEN {
        ASSUME(GetMoveCategory(MOVE_DOUBLE_EDGE) == DAMAGE_CATEGORY_PHYSICAL);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { Level(50); MaxHP(120); HP(120); Attack(500); Defense(100); Speed(120); Moves(MOVE_DOUBLE_EDGE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Level(50); MaxHP(200); HP(150); Attack(220); Defense(100); Speed(80); Moves(MOVE_PROTECT, MOVE_DOUBLE_EDGE, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); FORCED_MOVE(opponent); }
    } THEN {
        enum BattlerId aiBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);

        gSideStatuses[B_SIDE_PLAYER] |= SIDE_STATUS_REFLECT;
        gSideTimers[B_SIDE_PLAYER].reflectTimer = 1;
        gSideStatuses[B_SIDE_OPPONENT] |= SIDE_STATUS_REFLECT;
        gSideTimers[B_SIDE_OPPONENT].reflectTimer = 1;
        gBattleMons[playerBattler].hp = 80;

        EXPECT(!ShouldUseSinglesProtect(aiBattler, playerBattler, MOVE_DOUBLE_EDGE));
        EXPECT(ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_DOUBLE_EDGE) <= 0);
    }
}

AI_SINGLE_BATTLE_TEST("Protect: AI values singles Protect to burn final rain before a boosted attack")
{
    GIVEN {
        ASSUME(GetMoveType(MOVE_SURF) == TYPE_WATER);
        ASSUME(GetMoveCategory(MOVE_SURF) == DAMAGE_CATEGORY_SPECIAL);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { Level(50); MaxHP(100); HP(100); SpAttack(200); SpDefense(100); Speed(120); Moves(MOVE_SURF, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Level(50); MaxHP(120); HP(100); SpAttack(200); SpDefense(100); Speed(80); Moves(MOVE_PROTECT, MOVE_PSYCHIC, MOVE_CELEBRATE); }
    } WHEN {
        TURN {
            gBattleWeather = B_WEATHER_RAIN_NORMAL;
            gBattleStruct->weatherDuration = 1;
            MOVE(player, MOVE_SURF);
            EXPECT_MOVE(opponent, MOVE_PROTECT);
        }
    }
}

AI_SINGLE_BATTLE_TEST("Protect: AI does not burn final rain if Swift Swim speed expires too")
{
    GIVEN {
        ASSUME(GetMoveType(MOVE_SURF) == TYPE_WATER);
        ASSUME(GetMoveCategory(MOVE_SURF) == DAMAGE_CATEGORY_SPECIAL);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { Level(50); MaxHP(100); HP(100); SpAttack(300); SpDefense(100); Speed(120); Moves(MOVE_SURF, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Level(50); MaxHP(120); HP(40); SpAttack(300); SpDefense(100); Speed(80); Ability(ABILITY_SWIFT_SWIM); Moves(MOVE_PROTECT, MOVE_PSYCHIC, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); FORCED_MOVE(opponent); }
    } THEN {
        enum BattlerId aiBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);

        gBattleWeather = B_WEATHER_RAIN_NORMAL;
        gBattleStruct->weatherDuration = 1;
        gAiLogicData->speedStats[aiBattler] = GetBattlerTotalSpeedStat(aiBattler, gAiLogicData->abilities[aiBattler], gAiLogicData->holdEffects[aiBattler]);
        gAiLogicData->speedStats[playerBattler] = GetBattlerTotalSpeedStat(playerBattler, gAiLogicData->abilities[playerBattler], gAiLogicData->holdEffects[playerBattler]);
        gBattleMons[playerBattler].hp = 40;

        EXPECT(!ShouldUseSinglesProtect(aiBattler, playerBattler, MOVE_SURF));
        EXPECT(ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_SURF) <= 0);
    }
}

AI_SINGLE_BATTLE_TEST("Protect: AI values singles Protect to burn final Electric Terrain before a boosted attack")
{
    GIVEN {
        ASSUME(GetMoveType(MOVE_THUNDERBOLT) == TYPE_ELECTRIC);
        ASSUME(GetMoveCategory(MOVE_THUNDERBOLT) == DAMAGE_CATEGORY_SPECIAL);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { Level(50); MaxHP(100); HP(100); SpAttack(200); SpDefense(100); Speed(120); Moves(MOVE_THUNDERBOLT, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Level(50); MaxHP(120); HP(100); SpAttack(200); SpDefense(100); Speed(80); Moves(MOVE_PROTECT, MOVE_PSYCHIC, MOVE_CELEBRATE); }
    } WHEN {
        TURN {
            gFieldStatuses |= STATUS_FIELD_ELECTRIC_TERRAIN;
            gFieldTimers.terrainTimer = 1;
            MOVE(player, MOVE_THUNDERBOLT);
            EXPECT_MOVE(opponent, MOVE_PROTECT);
        }
    }
}

AI_SINGLE_BATTLE_TEST("Protect: AI values Max Guard to burn the last opposing Dynamax turn")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_MAX_GUARD) == EFFECT_PROTECT);
        ASSUME(GetMoveProtectMethod(MOVE_MAX_GUARD) == PROTECT_MAX_GUARD);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { Level(50); DynamaxLevel(10); MaxHP(200); HP(200); Attack(240); Defense(100); SpDefense(100); Speed(80); Moves(MOVE_FLARE_BLITZ, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Level(50); DynamaxLevel(10); MaxHP(220); HP(220); Attack(240); Defense(100); SpDefense(100); Speed(120); Moves(MOVE_GIGA_IMPACT, MOVE_PROTECT, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); FORCED_MOVE(opponent); }
    } THEN {
        enum BattlerId aiBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);

        SetActiveGimmick(playerBattler, GIMMICK_DYNAMAX);
        SetActiveGimmick(aiBattler, GIMMICK_DYNAMAX);
        gBattleStruct->dynamax.dynamaxTurns[playerBattler] = 1;
        gBattleStruct->dynamax.dynamaxTurns[aiBattler] = 2;
        gBattleStruct->chosenMovePositions[playerBattler] = 0;
        gBattleStruct->chosenMovePositions[aiBattler] = 0;

        EXPECT_GT(ProtectChecks(aiBattler, playerBattler, MOVE_MAX_GUARD, MOVE_FLARE_BLITZ), 0);
    }
}

AI_SINGLE_BATTLE_TEST("Protect: AI values Protect to burn final opposing Dynamax when chip damage is survivable")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { Level(50); DynamaxLevel(10); MaxHP(200); HP(200); Attack(240); Defense(100); SpDefense(100); Speed(80); Moves(MOVE_FLARE_BLITZ, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Level(50); MaxHP(300); HP(300); Attack(500); Defense(100); SpDefense(100); Speed(120); Moves(MOVE_GIGA_IMPACT, MOVE_PROTECT, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); FORCED_MOVE(opponent); }
    } THEN {
        enum BattlerId aiBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);

        SetActiveGimmick(playerBattler, GIMMICK_DYNAMAX);
        gBattleStruct->dynamax.dynamaxTurns[playerBattler] = 1;
        gBattleStruct->chosenMovePositions[playerBattler] = 0;
        gBattleStruct->chosenMovePositions[aiBattler] = 0;

        EXPECT_GT(ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_FLARE_BLITZ), 0);
    }
}

AI_SINGLE_BATTLE_TEST("Protect: AI does not spend its own final Max Guard only to burn opposing Dynamax")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_MAX_GUARD) == EFFECT_PROTECT);
        ASSUME(GetMoveProtectMethod(MOVE_MAX_GUARD) == PROTECT_MAX_GUARD);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { Level(50); DynamaxLevel(10); MaxHP(200); HP(200); Attack(240); Defense(100); SpDefense(100); Speed(80); Moves(MOVE_FLARE_BLITZ, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Level(50); DynamaxLevel(10); MaxHP(220); HP(220); Attack(240); Defense(100); SpDefense(100); Speed(120); Moves(MOVE_GIGA_IMPACT, MOVE_PROTECT, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); FORCED_MOVE(opponent); }
    } THEN {
        enum BattlerId aiBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);

        SetActiveGimmick(playerBattler, GIMMICK_DYNAMAX);
        SetActiveGimmick(aiBattler, GIMMICK_DYNAMAX);
        gBattleStruct->dynamax.dynamaxTurns[playerBattler] = 1;
        gBattleStruct->dynamax.dynamaxTurns[aiBattler] = 1;
        gBattleStruct->chosenMovePositions[playerBattler] = 0;
        gBattleStruct->chosenMovePositions[aiBattler] = 0;

        EXPECT(ProtectChecks(aiBattler, playerBattler, MOVE_MAX_GUARD, MOVE_FLARE_BLITZ) <= 0);
    }
}

AI_SINGLE_BATTLE_TEST("Protect: AI values singles Protect while opposing Perish count expires")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(999); HP(999); Speed(80); Moves(MOVE_DRAGON_RAGE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(120); HP(40); Speed(120); Moves(MOVE_PROTECT, MOVE_SCRATCH, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); FORCED_MOVE(opponent); }
    } THEN {
        enum BattlerId aiBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);

        gBattleMons[playerBattler].volatiles.perishSong = TRUE;
        gBattleMons[playerBattler].volatiles.perishSongTimer = 0;
        gBattleMons[aiBattler].hp = 40;

        EXPECT(ShouldUseSinglesProtect(aiBattler, playerBattler, MOVE_DRAGON_RAGE));
        EXPECT(ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_DRAGON_RAGE) > 0);
    }
}

AI_SINGLE_BATTLE_TEST("Protect: AI does not value singles Protect when both Perish counts expire")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(999); HP(999); Speed(80); Moves(MOVE_DRAGON_RAGE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(120); HP(40); Speed(120); Moves(MOVE_PROTECT, MOVE_SCRATCH, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); FORCED_MOVE(opponent); }
    } THEN {
        enum BattlerId aiBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);

        gBattleMons[playerBattler].volatiles.perishSong = TRUE;
        gBattleMons[playerBattler].volatiles.perishSongTimer = 0;
        gBattleMons[aiBattler].volatiles.perishSong = TRUE;
        gBattleMons[aiBattler].volatiles.perishSongTimer = 0;
        gBattleMons[aiBattler].hp = 40;

        EXPECT(!ShouldUseSinglesProtect(aiBattler, playerBattler, MOVE_DRAGON_RAGE));
        EXPECT(ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_DRAGON_RAGE) <= 0);
    }
}

AI_SINGLE_BATTLE_TEST("Protect: AI does not waste its own final Tailwind while burning Trick Room")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_TRICK_ROOM) == EFFECT_TRICK_ROOM);
        ASSUME(GetMoveEffect(MOVE_TAILWIND) == EFFECT_TAILWIND);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(999); HP(999); Speed(120); Moves(MOVE_DRAGON_RAGE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(120); HP(40); Speed(80); Moves(MOVE_PROTECT, MOVE_SCRATCH, MOVE_TAILWIND, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); FORCED_MOVE(opponent); }
    } THEN {
        enum BattlerId aiBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);

        gFieldStatuses |= STATUS_FIELD_TRICK_ROOM;
        gFieldTimers.trickRoomTimer = 1;
        gSideStatuses[B_SIDE_OPPONENT] |= SIDE_STATUS_TAILWIND;
        gSideTimers[B_SIDE_OPPONENT].tailwindTimer = 1;
        gBattleMons[aiBattler].hp = 40;

        EXPECT(!ShouldUseSinglesProtect(aiBattler, playerBattler, MOVE_DRAGON_RAGE));
        EXPECT_EQ(ProtectChecks(aiBattler, playerBattler, MOVE_PROTECT, MOVE_DRAGON_RAGE), NO_DAMAGE_OR_FAILS);
    }
}

AI_SINGLE_BATTLE_TEST("Protect: AI can still value a second singles Protect when payoff remains")
{
    PASSES_RANDOMLY(PREDICT_MOVE_CHANCE, 100, RNG_AI_PREDICT_MOVE);
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_PREDICT_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { Status1(STATUS1_TOXIC_POISON); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_PROTECT, MOVE_SCRATCH); }
    } WHEN {
        TURN {
            gBattleMons[B_POSITION_OPPONENT_LEFT].volatiles.consecutiveMoveUses = 1;
            MOVE(player, MOVE_TACKLE);
            SCORE_GT_VAL(opponent, MOVE_PROTECT, AI_SCORE_DEFAULT);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("Protect: AI does not treat a second double Protect as impossible")
{
    PASSES_RANDOMLY(PREDICT_MOVE_CHANCE, 100, RNG_AI_PREDICT_MOVE);
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_PREDICT_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { Status1(STATUS1_TOXIC_POISON); Moves(MOVE_TACKLE); }
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_PROTECT, MOVE_SCRATCH); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN {
            gBattleMons[B_POSITION_OPPONENT_LEFT].volatiles.consecutiveMoveUses = 1;
            MOVE(playerLeft, MOVE_TACKLE, target: opponentLeft);
            MOVE(playerRight, MOVE_CELEBRATE);
            SCORE_GT_VAL(opponentLeft, MOVE_PROTECT, AI_SCORE_DEFAULT + WORST_EFFECT, target: playerLeft);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("Protect: AI avoids Protect vs moves that ignore protection (Doubles)")
{
    enum Move move = MOVE_NONE;
    bool32 shouldProtect = FALSE;

    PARAMETRIZE { move = MOVE_TACKLE; shouldProtect = TRUE; }
    PARAMETRIZE { move = MOVE_FEINT; shouldProtect = FALSE; }
    PARAMETRIZE { move = MOVE_SHADOW_FORCE; shouldProtect = FALSE; }
    PARAMETRIZE { move = MOVE_PHANTOM_FORCE; shouldProtect = FALSE; }
    PARAMETRIZE { move = MOVE_HYPERSPACE_HOLE; shouldProtect = FALSE; }
    PARAMETRIZE { move = MOVE_HYPERSPACE_FURY; shouldProtect = FALSE; }

    PASSES_RANDOMLY(PREDICT_MOVE_CHANCE, 100, RNG_AI_PREDICT_MOVE);
    GIVEN {
        if (shouldProtect)
            ASSUME(!MoveIgnoresProtect(move));
        else
            ASSUME(MoveIgnoresProtect(move));
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_PREDICT_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(move); }
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_PROTECT, MOVE_SCRATCH); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN {
            MOVE(playerLeft, move, target: opponentLeft);
            MOVE(playerRight, MOVE_CELEBRATE);
            if (shouldProtect)
                SCORE_GT(opponentLeft, MOVE_PROTECT, MOVE_SCRATCH, target: playerLeft);
            else
                SCORE_LT(opponentLeft, MOVE_PROTECT, MOVE_SCRATCH, target: playerLeft);
        }
    }
}

AI_SINGLE_BATTLE_TEST("AI penalizes Yawn when target can self-status with Flame/Toxic Orb")
{
    u32 heldItem = ITEM_NONE;
    bool32 shouldYawn = FALSE;

    PARAMETRIZE { heldItem = ITEM_NONE;      shouldYawn = TRUE; }
    PARAMETRIZE { heldItem = ITEM_FLAME_ORB; shouldYawn = FALSE; }
    PARAMETRIZE { heldItem = ITEM_TOXIC_ORB; shouldYawn = FALSE; }

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_YAWN) == EFFECT_YAWN);
        ASSUME(gItemsInfo[ITEM_FLAME_ORB].holdEffect == HOLD_EFFECT_FLAME_ORB);
        ASSUME(gItemsInfo[ITEM_TOXIC_ORB].holdEffect == HOLD_EFFECT_TOXIC_ORB);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_WOBBUFFET) { Item(heldItem); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_YAWN, MOVE_SCRATCH); }
    } WHEN {
        TURN {
            if (shouldYawn)
                SCORE_GT(opponent, MOVE_YAWN, MOVE_SCRATCH);
            else
                SCORE_LT(opponent, MOVE_YAWN, MOVE_SCRATCH);
        }
    }
}

AI_SINGLE_BATTLE_TEST("AI avoids Thunder Wave when it can not paralyse target")
{
    u32 species;
    enum Ability ability;

    PARAMETRIZE { species = SPECIES_HITMONLEE; ability = ABILITY_LIMBER; }
    PARAMETRIZE { species = SPECIES_KOMALA; ability = ABILITY_COMATOSE; }
    PARAMETRIZE { species = SPECIES_NACLI; ability = ABILITY_PURIFYING_SALT; }
    PARAMETRIZE { species = SPECIES_PIKACHU; ability = ABILITY_STATIC; }

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_THUNDER_WAVE) == EFFECT_NON_VOLATILE_STATUS);
        ASSUME(GetMoveNonVolatileStatus(MOVE_THUNDER_WAVE) == MOVE_EFFECT_PARALYSIS);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(species) { Ability(ability); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE, MOVE_THUNDER_WAVE); }
    } WHEN {
        TURN { SCORE_EQ(opponent, MOVE_CELEBRATE, MOVE_THUNDER_WAVE); } // Both get -10
    }
}

AI_SINGLE_BATTLE_TEST("AI avoids Will-o-Wisp when it can not burn target")
{
    u32 species;
    enum Ability ability;

    PARAMETRIZE { species = SPECIES_BUIZEL; ability = ABILITY_WATER_VEIL; }
    PARAMETRIZE { species = SPECIES_DEWPIDER; ability = ABILITY_WATER_BUBBLE; }
    PARAMETRIZE { species = SPECIES_KOMALA; ability = ABILITY_COMATOSE; }
    PARAMETRIZE { species = SPECIES_ARCTIBAX; ability = ABILITY_THERMAL_EXCHANGE; }
    PARAMETRIZE { species = SPECIES_NACLI; ability = ABILITY_PURIFYING_SALT; }
    PARAMETRIZE { species = SPECIES_CHARMANDER; ability = ABILITY_BLAZE; }

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_WILL_O_WISP) == EFFECT_NON_VOLATILE_STATUS);
        ASSUME(GetMoveNonVolatileStatus(MOVE_WILL_O_WISP) == MOVE_EFFECT_BURN);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(species) { Ability(ability); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE, MOVE_WILL_O_WISP); }
    } WHEN {
        TURN { SCORE_EQ(opponent, MOVE_CELEBRATE, MOVE_WILL_O_WISP); } // Both get -10
    }
}

AI_SINGLE_BATTLE_TEST("AI avoids hypnosis when it can not put target to sleep")
{
    u32 species;
    enum Ability ability;

    PARAMETRIZE { species = SPECIES_HOOTHOOT; ability = ABILITY_INSOMNIA; }
    PARAMETRIZE { species = SPECIES_MANKEY; ability = ABILITY_VITAL_SPIRIT; }
    PARAMETRIZE { species = SPECIES_KOMALA; ability = ABILITY_COMATOSE; }
    PARAMETRIZE { species = SPECIES_NACLI; ability = ABILITY_PURIFYING_SALT; }

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_HYPNOSIS) == EFFECT_NON_VOLATILE_STATUS);
        ASSUME(GetMoveNonVolatileStatus(MOVE_HYPNOSIS) == MOVE_EFFECT_SLEEP);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(species) { Ability(ability); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE, MOVE_HYPNOSIS); }
    } WHEN {
        TURN { SCORE_EQ(opponent, MOVE_CELEBRATE, MOVE_HYPNOSIS); } // Both get -10
    }
}

AI_SINGLE_BATTLE_TEST("AI avoids toxic when it can not poison target")
{
    u32 species;
    enum Ability ability;

    PARAMETRIZE { species = SPECIES_SNORLAX; ability = ABILITY_IMMUNITY; }
    PARAMETRIZE { species = SPECIES_KOMALA; ability = ABILITY_COMATOSE; }
    PARAMETRIZE { species = SPECIES_NACLI; ability = ABILITY_PURIFYING_SALT; }
    PARAMETRIZE { species = SPECIES_BULBASAUR; ability = ABILITY_OVERGROW; }

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_TOXIC) == EFFECT_NON_VOLATILE_STATUS);
        ASSUME(GetMoveNonVolatileStatus(MOVE_TOXIC) == MOVE_EFFECT_TOXIC);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(species) { Ability(ability); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE, MOVE_TOXIC); }
    } WHEN {
        TURN { SCORE_EQ(opponent, MOVE_CELEBRATE, MOVE_TOXIC); } // Both get -10
    }
}
