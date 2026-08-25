#include "global.h"
#include "battle.h"
#include "test/battle.h"
#include "battle_ai_joint_planner.h"
#include "battle_ai_joint_runtime.h"
#include "battle_ai_main.h"
#include "battle_ai_util.h"
#include "battle_gimmick.h"
#include "malloc.h"

#define TEST_IVS_PHYSICAL() HPIV(31); AttackIV(31); DefenseIV(31); SpAttackIV(31); SpDefenseIV(31); SpeedIV(31)
#define TEST_IVS_SPECIAL() HPIV(31); AttackIV(0); DefenseIV(31); SpAttackIV(31); SpDefenseIV(31); SpeedIV(31)
#define TEST_IVS_TRICK_ROOM() HPIV(31); AttackIV(0); DefenseIV(31); SpAttackIV(31); SpDefenseIV(31); SpeedIV(0)

struct AiSmartGimmickSimFixture
{
    struct AiSimContext context;
    struct AiSimBoard before;
    struct AiSimBoard after;
    struct AiSimJointTurn turn;
    struct AiSimTurnResult result;
};

static EWRAM_DATA struct AiSmartGimmickSimFixture *sSmartGimmickSimFixture;

static struct AiSmartGimmickSimFixture *GetSmartGimmickSimFixture(void)
{
    if (sSmartGimmickSimFixture == NULL)
        sSmartGimmickSimFixture = AllocZeroed(sizeof(*sSmartGimmickSimFixture));
    return sSmartGimmickSimFixture;
}

static void FreeSmartGimmickSimFixture(void)
{
    TRY_FREE_AND_SET_NULL(sSmartGimmickSimFixture);
}

#define PLAYER_INCINEROAR(...) \
    PLAYER(SPECIES_INCINEROAR) { \
        Level(50); Item(ITEM_SITRUS_BERRY); Ability(ABILITY_INTIMIDATE); Nature(NATURE_CAREFUL); \
        TEST_IVS_PHYSICAL(); \
        MaxHP(202); HP(202); Attack(135); Defense(120); SpDefense(146); Speed(80); \
        Moves(__VA_ARGS__); \
    }

#define PLAYER_RILLABOOM(...) \
    PLAYER(SPECIES_RILLABOOM) { \
        Level(50); Item(ITEM_ASSAULT_VEST); Ability(ABILITY_GRASSY_SURGE); Nature(NATURE_ADAMANT); \
        TEST_IVS_PHYSICAL(); \
        MaxHP(207); HP(207); Attack(187); Defense(110); SpDefense(98); Speed(105); \
        Moves(__VA_ARGS__); \
    }

#define PLAYER_TORNADUS(...) \
    PLAYER(SPECIES_TORNADUS) { \
        Level(50); Item(ITEM_COVERT_CLOAK); Ability(ABILITY_PRANKSTER); Nature(NATURE_TIMID); \
        TEST_IVS_SPECIAL(); \
        MaxHP(155); HP(155); Defense(90); SpAttack(177); SpDefense(100); Speed(179); \
        Moves(__VA_ARGS__); \
    }

#define PLAYER_TORNADUS_LOW_HP(...) \
    PLAYER(SPECIES_TORNADUS) { \
        Level(50); Item(ITEM_FOCUS_SASH); Ability(ABILITY_PRANKSTER); Nature(NATURE_TIMID); \
        TEST_IVS_SPECIAL(); \
        MaxHP(155); HP(1); Defense(90); SpAttack(177); SpDefense(100); Speed(179); \
        Moves(__VA_ARGS__); \
    }

#define PLAYER_TAPU_KOKO_Z(...) \
    PLAYER(SPECIES_TAPU_KOKO) { \
        Level(50); Item(ITEM_ELECTRIUM_Z); Ability(ABILITY_ELECTRIC_SURGE); Nature(NATURE_TIMID); \
        TEST_IVS_SPECIAL(); \
        MaxHP(146); HP(146); Defense(105); SpAttack(147); SpDefense(95); Speed(200); \
        Moves(__VA_ARGS__); \
    }

#define PLAYER_LANDORUS(...) \
    PLAYER(SPECIES_LANDORUS_THERIAN) { \
        Level(50); Item(ITEM_GROUNDIUM_Z); Ability(ABILITY_INTIMIDATE); Nature(NATURE_JOLLY); \
        TEST_IVS_PHYSICAL(); \
        MaxHP(165); HP(165); Attack(197); Defense(110); SpDefense(100); Speed(157); \
        Moves(__VA_ARGS__); \
    }

#define PLAYER_FARIGIRAF(...) \
    PLAYER(SPECIES_FARIGIRAF) { \
        Level(50); Item(ITEM_MENTAL_HERB); Ability(ABILITY_ARMOR_TAIL); Nature(NATURE_SASSY); \
        TEST_IVS_TRICK_ROOM(); \
        MaxHP(227); HP(227); Defense(105); SpAttack(130); SpDefense(110); Speed(58); \
        Moves(__VA_ARGS__); \
    }

#define PLAYER_MIRAIDON_ICE_TERA(...) \
    PLAYER(SPECIES_MIRAIDON) { \
        Level(50); Item(ITEM_CHOICE_SPECS); Nature(NATURE_TIMID); TeraType(TYPE_ICE); \
        TEST_IVS_SPECIAL(); \
        MaxHP(176); HP(176); Defense(120); SpAttack(205); SpDefense(135); Speed(205); \
        Moves(__VA_ARGS__); \
    }

#define PLAYER_GYARADOS(...) \
    PLAYER(SPECIES_GYARADOS) { \
        Level(50); Item(ITEM_SITRUS_BERRY); Ability(ABILITY_INTIMIDATE); Nature(NATURE_ADAMANT); \
        TEST_IVS_PHYSICAL(); \
        MaxHP(202); HP(202); Attack(194); Defense(99); SpDefense(120); Speed(133); \
        Moves(__VA_ARGS__); \
    }

#define PLAYER_DUGTRIO(...) \
    PLAYER(SPECIES_DUGTRIO) { \
        Level(50); Item(ITEM_FOCUS_SASH); Nature(NATURE_JOLLY); \
        TEST_IVS_PHYSICAL(); \
        MaxHP(111); HP(111); Attack(167); Defense(70); SpDefense(90); Speed(189); \
        Moves(__VA_ARGS__); \
    }

#define PLAYER_ARCANINE(...) \
    PLAYER(SPECIES_ARCANINE) { \
        Level(50); Item(ITEM_SITRUS_BERRY); Ability(ABILITY_INTIMIDATE); Nature(NATURE_CAREFUL); \
        TEST_IVS_PHYSICAL(); \
        MaxHP(197); HP(197); Attack(130); Defense(100); SpDefense(145); Speed(115); \
        Moves(__VA_ARGS__); \
    }

#define PLAYER_MARSHADOW_Z(...) \
    PLAYER(SPECIES_MARSHADOW) { \
        Level(50); Item(ITEM_MARSHADIUM_Z); Nature(NATURE_JOLLY); \
        TEST_IVS_PHYSICAL(); \
        MaxHP(166); HP(166); Attack(177); Defense(100); SpAttack(99); SpDefense(110); Speed(194); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_TORNADUS(...) \
    OPPONENT(SPECIES_TORNADUS) { \
        Level(50); Item(ITEM_COVERT_CLOAK); Ability(ABILITY_PRANKSTER); Nature(NATURE_TIMID); \
        TEST_IVS_SPECIAL(); \
        MaxHP(155); HP(155); Defense(90); SpAttack(177); SpDefense(100); Speed(179); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_WHIMSICOTT(...) \
    OPPONENT(SPECIES_WHIMSICOTT) { \
        Level(50); Item(ITEM_FOCUS_SASH); Ability(ABILITY_PRANKSTER); Nature(NATURE_TIMID); \
        TEST_IVS_SPECIAL(); \
        MaxHP(135); HP(135); Defense(105); SpAttack(129); SpDefense(95); Speed(184); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_FARIGIRAF(...) \
    OPPONENT(SPECIES_FARIGIRAF) { \
        Level(50); Item(ITEM_MENTAL_HERB); Ability(ABILITY_ARMOR_TAIL); Nature(NATURE_SASSY); \
        TEST_IVS_TRICK_ROOM(); \
        MaxHP(227); HP(227); Defense(105); SpAttack(130); SpDefense(110); Speed(58); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_KORAIDON(...) \
    OPPONENT(SPECIES_KORAIDON) { \
        Level(50); Item(ITEM_CLEAR_AMULET); Nature(NATURE_JOLLY); \
        TEST_IVS_PHYSICAL(); \
        MaxHP(176); HP(176); Attack(205); Defense(135); SpDefense(120); Speed(205); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_KORAIDON_DMAX(...) \
    OPPONENT(SPECIES_KORAIDON) { \
        Level(50); Item(ITEM_CLEAR_AMULET); Nature(NATURE_JOLLY); DynamaxLevel(10); \
        TEST_IVS_PHYSICAL(); \
        MaxHP(176); HP(176); Attack(205); Defense(135); SpDefense(120); Speed(205); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_KORAIDON_DMAX_LOW_HP(...) \
    OPPONENT(SPECIES_KORAIDON) { \
        Level(50); Item(ITEM_CLEAR_AMULET); Nature(NATURE_JOLLY); DynamaxLevel(10); \
        TEST_IVS_PHYSICAL(); \
        MaxHP(176); HP(44); Attack(205); Defense(135); SpDefense(120); Speed(205); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_KORAIDON_TERA(...) \
    OPPONENT(SPECIES_KORAIDON) { \
        Level(50); Item(ITEM_CLEAR_AMULET); Nature(NATURE_JOLLY); TeraType(TYPE_FIRE); \
        TEST_IVS_PHYSICAL(); \
        MaxHP(176); HP(176); Attack(205); Defense(135); SpDefense(120); Speed(205); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_MIRAIDON(...) \
    OPPONENT(SPECIES_MIRAIDON) { \
        Level(50); Item(ITEM_CHOICE_SPECS); Nature(NATURE_TIMID); \
        TEST_IVS_SPECIAL(); \
        MaxHP(176); HP(176); Defense(120); SpAttack(205); SpDefense(135); Speed(205); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_TAPU_KOKO_Z(...) \
    OPPONENT(SPECIES_TAPU_KOKO) { \
        Level(50); Item(ITEM_ELECTRIUM_Z); Ability(ABILITY_ELECTRIC_SURGE); Nature(NATURE_TIMID); \
        TEST_IVS_SPECIAL(); \
        MaxHP(146); HP(146); Defense(105); SpAttack(147); SpDefense(95); Speed(200); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_TAPU_KOKO_Z_LOW_HP(...) \
    OPPONENT(SPECIES_TAPU_KOKO) { \
        Level(50); Item(ITEM_ELECTRIUM_Z); Ability(ABILITY_ELECTRIC_SURGE); Nature(NATURE_TIMID); \
        TEST_IVS_SPECIAL(); \
        MaxHP(146); HP(36); Defense(105); SpAttack(147); SpDefense(95); Speed(200); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_KYOGRE(...) \
    OPPONENT(SPECIES_KYOGRE) { \
        Level(50); Item(ITEM_CHOICE_SPECS); Nature(NATURE_MODEST); DynamaxLevel(10); \
        TEST_IVS_SPECIAL(); \
        MaxHP(176); HP(176); Defense(110); SpAttack(222); SpDefense(160); Speed(140); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_KYOGRE_MYSTIC_WATER(...) \
    OPPONENT(SPECIES_KYOGRE) { \
        Level(50); Item(ITEM_MYSTIC_WATER); Nature(NATURE_MODEST); DynamaxLevel(10); \
        TEST_IVS_SPECIAL(); \
        MaxHP(176); HP(176); Defense(110); SpAttack(222); SpDefense(160); Speed(140); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_CHARIZARD_DMAX(...) \
    OPPONENT(SPECIES_CHARIZARD) { \
        Level(50); Item(ITEM_LIFE_ORB); Ability(ABILITY_SOLAR_POWER); Nature(NATURE_TIMID); DynamaxLevel(10); \
        TEST_IVS_SPECIAL(); \
        MaxHP(153); HP(153); Defense(99); SpAttack(177); SpDefense(105); Speed(167); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_CHARIZARD_GMAX(...) \
    OPPONENT(SPECIES_CHARIZARD) { \
        Level(50); Item(ITEM_LIFE_ORB); Ability(ABILITY_SOLAR_POWER); Nature(NATURE_TIMID); DynamaxLevel(10); GigantamaxFactor(TRUE); \
        TEST_IVS_SPECIAL(); \
        MaxHP(153); HP(153); Defense(99); SpAttack(177); SpDefense(105); Speed(167); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_DREDNAW_GMAX(...) \
    OPPONENT(SPECIES_DREDNAW) { \
        Level(50); Item(ITEM_MYSTIC_WATER); Nature(NATURE_ADAMANT); DynamaxLevel(10); GigantamaxFactor(TRUE); \
        TEST_IVS_PHYSICAL(); \
        MaxHP(165); HP(165); Attack(167); Defense(110); SpDefense(90); Speed(94); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_LANDORUS(...) \
    OPPONENT(SPECIES_LANDORUS_THERIAN) { \
        Level(50); Item(ITEM_CLEAR_AMULET); Ability(ABILITY_INTIMIDATE); Nature(NATURE_JOLLY); DynamaxLevel(10); \
        TEST_IVS_PHYSICAL(); \
        MaxHP(165); HP(165); Attack(197); Defense(110); SpDefense(100); Speed(157); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_GENGAR(...) \
    OPPONENT(SPECIES_GENGAR) { \
        Level(50); Item(ITEM_GENGARITE); Ability(ABILITY_CURSED_BODY); Nature(NATURE_TIMID); \
        TEST_IVS_SPECIAL(); \
        MaxHP(136); HP(136); Defense(80); SpAttack(182); SpDefense(95); Speed(178); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_GENGAR_MEGA_ACTIVE(currentHp, ...) \
    OPPONENT(SPECIES_GENGAR_MEGA) { \
        Level(50); Item(ITEM_GENGARITE); Ability(ABILITY_SHADOW_TAG); Nature(NATURE_TIMID); \
        TEST_IVS_SPECIAL(); \
        MaxHP(136); HP(currentHp); Defense(100); SpAttack(222); SpDefense(115); Speed(200); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_INCINEROAR(...) \
    OPPONENT(SPECIES_INCINEROAR) { \
        Level(50); Item(ITEM_SITRUS_BERRY); Ability(ABILITY_INTIMIDATE); Nature(NATURE_CAREFUL); \
        TEST_IVS_PHYSICAL(); \
        MaxHP(202); HP(202); Attack(135); Defense(120); SpDefense(146); Speed(80); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_INCINEROAR_DMAX_LEVEL_100(...) \
    OPPONENT(SPECIES_INCINEROAR) { \
        Level(100); Item(ITEM_SITRUS_BERRY); Ability(ABILITY_INTIMIDATE); Nature(NATURE_CAREFUL); DynamaxLevel(10); \
        TEST_IVS_PHYSICAL(); \
        MaxHP(394); HP(394); Attack(266); Defense(236); SpDefense(288); Speed(156); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_RILLABOOM(...) \
    OPPONENT(SPECIES_RILLABOOM) { \
        Level(50); Item(ITEM_ASSAULT_VEST); Ability(ABILITY_GRASSY_SURGE); Nature(NATURE_ADAMANT); \
        TEST_IVS_PHYSICAL(); \
        MaxHP(207); HP(207); Attack(187); Defense(110); SpDefense(98); Speed(105); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_VENUSAUR(currentHp, ...) \
    OPPONENT(SPECIES_VENUSAUR) { \
        Level(50); Item(ITEM_VENUSAURITE); Ability(ABILITY_CHLOROPHYLL); Nature(NATURE_MODEST); \
        TEST_IVS_SPECIAL(); \
        MaxHP(187); HP(currentHp); Defense(103); SpAttack(152); SpDefense(120); Speed(100); \
        Moves(__VA_ARGS__); \
    }

#define OPPONENT_DRAGONITE(...) \
    OPPONENT(SPECIES_DRAGONITE) { \
        Level(50); Item(ITEM_CHOICE_BAND); Ability(ABILITY_MULTISCALE); Nature(NATURE_ADAMANT); \
        TEST_IVS_PHYSICAL(); \
        MaxHP(167); HP(167); Attack(204); Defense(115); SpDefense(120); Speed(132); \
        Moves(__VA_ARGS__); \
    }

static void SetupConfirmedFakeOutBadMoveTest(enum BattlerId source, enum BattlerId battler)
{
    gChosenActionByBattler[source] = B_ACTION_USE_MOVE;
    gChosenMoveByBattler[source] = MOVE_FAKE_OUT;
    gBattleStruct->chosenMovePositions[source] = 0;
    gBattleStruct->moveTarget[source] = battler;
    gBattleStruct->battlerState[source].isFirstTurn = 1;
    gAiThinkingStruct->aiFlags[battler] |= AI_FLAG_READ_PLAYER_MOVE;
    SetAiLogicDataForTurn(gAiLogicData);
    BattleAI_SetupAIData(0xF, battler);
    gAiLogicData->partnerMove = MOVE_NONE;
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY: AI conserves Dynamax when it has no immediate payoff")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY);
        PLAYER_FARIGIRAF(MOVE_TRICK_ROOM, MOVE_PSYCHIC, MOVE_HELPING_HAND, MOVE_PROTECT);
        OPPONENT_KORAIDON_DMAX(MOVE_DRAGON_CLAW, MOVE_PROTECT);
        OPPONENT_CHARIZARD_DMAX(MOVE_HEAT_WAVE, MOVE_AIR_SLASH, MOVE_SOLAR_BEAM, MOVE_PROTECT);
    } WHEN {
        TURN { MOVE(player, MOVE_PROTECT); EXPECT_MOVE(opponent, MOVE_DRAGON_CLAW, gimmick: GIMMICK_NONE); }
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY: AI can spend Dynamax on its last Pokemon")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY);
        PLAYER_FARIGIRAF(MOVE_TRICK_ROOM, MOVE_PSYCHIC, MOVE_HELPING_HAND, MOVE_PROTECT);
        OPPONENT_KORAIDON_DMAX(MOVE_DRAGON_CLAW, MOVE_PROTECT);
    } WHEN {
        TURN { MOVE(player, MOVE_PROTECT); EXPECT_MOVE(opponent, MOVE_DRAGON_CLAW, gimmick: GIMMICK_DYNAMAX); }
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY: AI can spend Dynamax with one reserve when low on HP")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY);
        PLAYER_FARIGIRAF(MOVE_TRICK_ROOM, MOVE_PSYCHIC, MOVE_HELPING_HAND, MOVE_PROTECT);
        OPPONENT_KORAIDON_DMAX_LOW_HP(MOVE_DRAGON_CLAW, MOVE_PROTECT);
        OPPONENT_CHARIZARD_DMAX(MOVE_HEAT_WAVE, MOVE_AIR_SLASH, MOVE_SOLAR_BEAM, MOVE_PROTECT);
    } WHEN {
        TURN { MOVE(player, MOVE_PROTECT); EXPECT_MOVE(opponent, MOVE_DRAGON_CLAW, gimmick: GIMMICK_DYNAMAX); }
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY: AI can spend Dynamax to set rain with Max Geyser")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        OPPONENT_KYOGRE(MOVE_ORIGIN_PULSE, MOVE_ICE_BEAM);
        OPPONENT_CHARIZARD_DMAX(MOVE_HEAT_WAVE, MOVE_AIR_SLASH, MOVE_SOLAR_BEAM, MOVE_PROTECT);
    } WHEN {
        TURN { MOVE(player, MOVE_KNOCK_OFF); EXPECT_MOVE(opponent, MOVE_ORIGIN_PULSE, gimmick: GIMMICK_DYNAMAX); }
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY: AI can spend Dynamax to block Fake Out disruption")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        OPPONENT_CHARIZARD_DMAX(MOVE_AIR_SLASH, MOVE_PROTECT);
        OPPONENT_KYOGRE(MOVE_ORIGIN_PULSE, MOVE_ICE_BEAM);
    } WHEN {
        TURN { MOVE(player, MOVE_FAKE_OUT); EXPECT_MOVE(opponent, MOVE_AIR_SLASH, gimmick: GIMMICK_DYNAMAX); }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI does not spend Dynamax on Protect")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_PROTECT) == EFFECT_PROTECT);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE | AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY);
        PLAYER_INCINEROAR(MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF, MOVE_FAKE_OUT, MOVE_PROTECT);
        PLAYER_RILLABOOM(MOVE_PROTECT, MOVE_WOOD_HAMMER, MOVE_GRASSY_GLIDE, MOVE_FAKE_OUT);
        OPPONENT(SPECIES_AMOONGUSS) {
            Level(50); Item(ITEM_ROCKY_HELMET); Ability(ABILITY_REGENERATOR); Nature(NATURE_SASSY); DynamaxLevel(10);
            TEST_IVS_SPECIAL();
            MaxHP(221); HP(40); Defense(120); SpAttack(105); SpDefense(145); Speed(31);
            Moves(MOVE_PROTECT);
        }
        OPPONENT_RILLABOOM(MOVE_FAKE_OUT, MOVE_WOOD_HAMMER, MOVE_GRASSY_GLIDE, MOVE_PROTECT);
        OPPONENT_CHARIZARD_DMAX(MOVE_HEAT_WAVE, MOVE_AIR_SLASH, MOVE_SOLAR_BEAM, MOVE_PROTECT);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_FLARE_BLITZ, target: opponentLeft);
            MOVE(playerRight, MOVE_WOOD_HAMMER, target: opponentRight);
            EXPECT_MOVE(opponentLeft, MOVE_PROTECT, gimmick: GIMMICK_NONE);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI does not spend Dynamax on Tailwind")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_TAILWIND) == EFFECT_TAILWIND);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE | AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY);
        PLAYER_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_KNOCK_OFF, MOVE_FLARE_BLITZ, MOVE_PROTECT);
        OPPONENT(SPECIES_TORNADUS) {
            Level(50); Item(ITEM_COVERT_CLOAK); Ability(ABILITY_PRANKSTER); Nature(NATURE_TIMID); DynamaxLevel(10);
            TEST_IVS_SPECIAL();
            MaxHP(155); HP(1); Defense(90); SpAttack(177); SpDefense(100); Speed(179);
            Moves(MOVE_TAILWIND);
        }
        OPPONENT_INCINEROAR(MOVE_FAKE_OUT, MOVE_KNOCK_OFF, MOVE_FLARE_BLITZ, MOVE_PARTING_SHOT);
        OPPONENT_RILLABOOM(MOVE_FAKE_OUT, MOVE_WOOD_HAMMER, MOVE_GRASSY_GLIDE, MOVE_PROTECT);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_BLEAKWIND_STORM, target: opponentLeft);
            MOVE(playerRight, MOVE_KNOCK_OFF, target: opponentRight);
            EXPECT_MOVE(opponentLeft, MOVE_TAILWIND, gimmick: GIMMICK_NONE);
        }
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI can spend Dynamax against an already chosen Fake Out")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        OPPONENT_CHARIZARD_DMAX(MOVE_AIR_SLASH, MOVE_PROTECT);
        OPPONENT_KYOGRE(MOVE_ORIGIN_PULSE, MOVE_ICE_BEAM);
    } WHEN {
        TURN { MOVE(player, MOVE_FAKE_OUT); EXPECT_MOVE(opponent, MOVE_AIR_SLASH, gimmick: GIMMICK_DYNAMAX); }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI does not mirror a selected Trick Room")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_TRICK_ROOM) == EFFECT_TRICK_ROOM);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_POWERFUL_STATUS | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_FARIGIRAF(MOVE_TRICK_ROOM, MOVE_PSYCHIC, MOVE_HELPING_HAND, MOVE_PROTECT);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        OPPONENT_FARIGIRAF(MOVE_TRICK_ROOM, MOVE_PSYCHIC, MOVE_HELPING_HAND, MOVE_PROTECT);
        OPPONENT_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_TRICK_ROOM);
            MOVE(playerRight, MOVE_KNOCK_OFF, target: opponentRight);
            EXPECT_MOVES(opponentRight, MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        }
    } THEN {
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        const struct BattleActionLogEntry *entry = BattleActionLog_GetLastEntry(battler, 1u << B_ACTION_USE_MOVE);

        EXPECT(entry != NULL);
        if (entry != NULL)
            EXPECT_NE(entry->move, MOVE_TRICK_ROOM);
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI scores against selected player Tera type")
{
    GIVEN {
        ASSUME(GetMoveType(MOVE_DRAGON_CLAW) == TYPE_DRAGON);
        ASSUME(GetMoveType(MOVE_COLLISION_COURSE) == TYPE_FIGHTING);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_MIRAIDON_ICE_TERA(MOVE_DRACO_METEOR, MOVE_THUNDERBOLT, MOVE_ELECTRO_DRIFT, MOVE_VOLT_SWITCH);
        PLAYER_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        OPPONENT_KORAIDON(MOVE_COLLISION_COURSE, MOVE_DRAGON_CLAW, MOVE_FLARE_BLITZ, MOVE_PROTECT);
        OPPONENT_MIRAIDON(MOVE_ELECTRO_DRIFT, MOVE_THUNDERBOLT, MOVE_DRACO_METEOR, MOVE_VOLT_SWITCH);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_DRACO_METEOR, target: opponentRight, gimmick: GIMMICK_TERA);
            MOVE(playerRight, MOVE_TAILWIND);
            EXPECT_MOVE(opponentLeft, MOVE_COLLISION_COURSE, target: playerLeft);
        }
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI scores against the actual selected switch-in")
{
    GIVEN {
        ASSUME(GetMoveType(MOVE_THUNDERBOLT) == TYPE_ELECTRIC);
        ASSUME(GetMoveType(MOVE_GRASS_KNOT) == TYPE_GRASS);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_PREDICTION | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_GYARADOS(MOVE_WATERFALL, MOVE_CRUNCH, MOVE_DRAGON_DANCE, MOVE_PROTECT);
        PLAYER_DUGTRIO(MOVE_HIGH_HORSEPOWER, MOVE_ROCK_SLIDE, MOVE_SUCKER_PUNCH, MOVE_PROTECT);
        OPPONENT_TAPU_KOKO_Z(MOVE_THUNDERBOLT, MOVE_GRASS_KNOT, MOVE_DAZZLING_GLEAM, MOVE_VOLT_SWITCH);
    } WHEN {
        TURN {
            SWITCH(player, 1);
            EXPECT_MOVE(opponent, MOVE_GRASS_KNOT);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: command log records all double battle choices")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        OPPONENT_KORAIDON(MOVE_DRAGON_CLAW, MOVE_PROTECT);
        OPPONENT_MIRAIDON(MOVE_THUNDERBOLT, MOVE_PROTECT);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_TAILWIND);
            MOVE(playerRight, MOVE_PARTING_SHOT, target: opponentLeft);
            EXPECT_MOVE(opponentLeft, MOVE_DRAGON_CLAW, target: playerLeft);
            EXPECT_MOVE(opponentRight, MOVE_THUNDERBOLT, target: playerRight);
        }
    } THEN {
        enum BattlerId opponentLeftBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId opponentRightBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT);
        const struct BattleActionLogEntry *leftLog = BattleActionLog_GetLastEntry(opponentLeftBattler, 1u << B_ACTION_USE_MOVE);
        const struct BattleActionLogEntry *rightLog = BattleActionLog_GetLastEntry(opponentRightBattler, 1u << B_ACTION_USE_MOVE);

        EXPECT_EQ(gBattleActionLog.count, 4);
        EXPECT(leftLog != NULL);
        EXPECT(rightLog != NULL);
        EXPECT_EQ(leftLog->move, MOVE_DRAGON_CLAW);
        EXPECT_EQ(leftLog->target, GetBattlerAtPosition(B_POSITION_PLAYER_LEFT));
        EXPECT_EQ(rightLog->move, MOVE_THUNDERBOLT);
        EXPECT_EQ(rightLog->target, GetBattlerAtPosition(B_POSITION_PLAYER_RIGHT));
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI retargets single-target damage away from selected Protect")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_PROTECT) == EFFECT_PROTECT);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_RILLABOOM(MOVE_FAKE_OUT, MOVE_GRASSY_GLIDE, MOVE_WOOD_HAMMER, MOVE_KNOCK_OFF);
        PLAYER_TORNADUS_LOW_HP(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        OPPONENT_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        OPPONENT_KORAIDON(MOVE_DRAGON_CLAW, MOVE_COLLISION_COURSE, MOVE_FLARE_BLITZ);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_GRASSY_GLIDE, target: opponentRight);
            MOVE(playerRight, MOVE_PROTECT);
            EXPECT_MOVES(opponentLeft, MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
            EXPECT_MOVE(opponentRight, MOVE_FLARE_BLITZ, target: playerLeft);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI does not attack Protect or passively Protect into setup")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_PROTECT) == EFFECT_PROTECT);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_SMART_GIMMICK | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_DRAGONITE) { Level(50); Item(ITEM_DRAGONINITE); Ability(ABILITY_MULTISCALE); MaxHP(198); HP(198); Attack(186); Defense(115); SpDefense(120); Speed(100); Moves(MOVE_DRAGON_DANCE, MOVE_EXTREME_SPEED, MOVE_DRAGON_CLAW, MOVE_PROTECT); }
        PLAYER(SPECIES_GARDEVOIR) { Level(50); Item(ITEM_FOCUS_SASH); Ability(ABILITY_TRACE); MaxHP(143); HP(143); Defense(85); SpDefense(135); Speed(145); Moves(MOVE_DAZZLING_GLEAM, MOVE_MOONBLAST, MOVE_PSYCHIC, MOVE_PROTECT); }
        OPPONENT(SPECIES_LUCARIO) { Level(50); Item(ITEM_LUCARIONITE); Ability(ABILITY_INNER_FOCUS); SpAttack(167); Speed(156); Moves(MOVE_AURA_SPHERE, MOVE_FLASH_CANNON, MOVE_VACUUM_WAVE, MOVE_PROTECT); }
        OPPONENT(SPECIES_CHARIZARD) { Level(50); Item(ITEM_LIFE_ORB); Ability(ABILITY_SOLAR_POWER); SpAttack(161); Speed(167); Moves(MOVE_HEAT_WAVE, MOVE_AIR_SLASH, MOVE_SOLAR_BEAM, MOVE_PROTECT); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_DRAGON_DANCE);
            MOVE(playerRight, MOVE_PROTECT);
            EXPECT_MOVE(opponentLeft, MOVE_FLASH_CANNON, target: playerLeft);
            EXPECT_MOVES(opponentRight, MOVE_HEAT_WAVE, MOVE_AIR_SLASH, MOVE_SOLAR_BEAM);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI pressures a selected offensive setup threat")
{
    GIVEN {
        ASSUME(IsOffensiveStatRaisingMove(MOVE_DRAGON_DANCE));
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_SMART_GIMMICK | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_DRAGONITE) { Level(50); Item(ITEM_DRAGONINITE); Ability(ABILITY_MULTISCALE); MaxHP(198); HP(198); Attack(186); Defense(115); SpDefense(120); Speed(100); Moves(MOVE_DRAGON_DANCE, MOVE_EXTREME_SPEED, MOVE_DRAGON_CLAW, MOVE_PROTECT); }
        PLAYER(SPECIES_GARDEVOIR) { Level(50); Item(ITEM_FOCUS_SASH); Ability(ABILITY_TRACE); MaxHP(143); HP(143); Defense(85); SpDefense(135); Speed(145); Moves(MOVE_DAZZLING_GLEAM, MOVE_MOONBLAST, MOVE_PSYCHIC, MOVE_PROTECT); }
        OPPONENT(SPECIES_LUCARIO) { Level(50); Item(ITEM_LUCARIONITE); Ability(ABILITY_INNER_FOCUS); SpAttack(167); Speed(156); Moves(MOVE_AURA_SPHERE, MOVE_FLASH_CANNON, MOVE_VACUUM_WAVE, MOVE_PROTECT); }
        OPPONENT(SPECIES_CHARIZARD) { Level(50); Item(ITEM_LIFE_ORB); Ability(ABILITY_SOLAR_POWER); SpAttack(161); Speed(167); Moves(MOVE_HEAT_WAVE, MOVE_AIR_SLASH, MOVE_SOLAR_BEAM, MOVE_PROTECT); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_DRAGON_DANCE);
            MOVE(playerRight, MOVE_MOONBLAST, target: opponentLeft);
            EXPECT_MOVE(opponentLeft, MOVE_FLASH_CANNON, target: playerLeft);
            EXPECT_MOVE(opponentRight, MOVE_AIR_SLASH, target: playerLeft);
        }
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: Mega Lucario attacks selected Dragon Dance instead of setting up")
{
    GIVEN {
        ASSUME(IsOffensiveStatRaisingMove(MOVE_DRAGON_DANCE));
        ASSUME_STAT_CHANGE(MOVE_NASTY_PLOT, spAtk: +2);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_SMART_GIMMICK | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_DRAGONITE) {
            Level(50); Item(ITEM_DRAGONINITE); Ability(ABILITY_MULTISCALE); Nature(NATURE_ADAMANT);
            TEST_IVS_PHYSICAL();
            MaxHP(198); HP(198); Attack(186); Defense(115); SpDefense(120); Speed(100);
            Moves(MOVE_DRAGON_DANCE, MOVE_EXTREME_SPEED, MOVE_DRAGON_CLAW, MOVE_PROTECT);
        }
        OPPONENT(SPECIES_LUCARIO) {
            Level(50); Item(ITEM_LUCARIONITE); Ability(ABILITY_INNER_FOCUS); Nature(NATURE_TIMID);
            TEST_IVS_SPECIAL();
            MaxHP(146); HP(146); Defense(90); SpAttack(167); SpDefense(90); Speed(156);
            Moves(MOVE_AURA_SPHERE, MOVE_FLASH_CANNON, MOVE_VACUUM_WAVE, MOVE_NASTY_PLOT);
        }
        OPPONENT(SPECIES_CHARIZARD) { Level(50); Item(ITEM_LIFE_ORB); Ability(ABILITY_SOLAR_POWER); SpAttack(161); Speed(167); Moves(MOVE_HEAT_WAVE, MOVE_AIR_SLASH, MOVE_SOLAR_BEAM, MOVE_PROTECT); }
        OPPONENT(SPECIES_SAMUROTT_HISUI) { Level(50); Item(ITEM_CLEAR_AMULET); Ability(ABILITY_SHARPNESS); Attack(160); Speed(150); Moves(MOVE_CEASELESS_EDGE, MOVE_AQUA_CUTTER, MOVE_SUCKER_PUNCH, MOVE_PROTECT); }
    } WHEN {
        TURN {
            MOVE(player, MOVE_DRAGON_DANCE);
            EXPECT_MOVE(opponent, MOVE_FLASH_CANNON);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI does not hide behind King's Shield while a setup threat is selected")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_KINGS_SHIELD) == EFFECT_PROTECT);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_SMART_GIMMICK | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_DRAGONITE) { Level(50); Item(ITEM_DRAGONINITE); Ability(ABILITY_MULTISCALE); MaxHP(198); HP(198); Attack(186); Defense(115); SpDefense(120); Speed(100); Moves(MOVE_DRAGON_DANCE, MOVE_EXTREME_SPEED, MOVE_DRAGON_CLAW, MOVE_PROTECT); }
        PLAYER(SPECIES_GARDEVOIR) { Level(50); Item(ITEM_FOCUS_SASH); Ability(ABILITY_TRACE); MaxHP(143); HP(143); Defense(85); SpDefense(135); Speed(145); Moves(MOVE_DAZZLING_GLEAM, MOVE_MOONBLAST, MOVE_PSYCHIC, MOVE_PROTECT); }
        OPPONENT(SPECIES_LUCARIO) { Level(50); Item(ITEM_LUCARIONITE); Ability(ABILITY_INNER_FOCUS); SpAttack(167); Speed(156); Moves(MOVE_AURA_SPHERE, MOVE_FLASH_CANNON, MOVE_VACUUM_WAVE, MOVE_PROTECT); }
        OPPONENT(SPECIES_AEGISLASH_BLADE) { Level(50); Ability(ABILITY_STANCE_CHANGE); MaxHP(150); HP(150); Defense(70); SpAttack(222); Speed(80); Moves(MOVE_KINGS_SHIELD, MOVE_FLASH_CANNON, MOVE_SHADOW_BALL, MOVE_SACRED_SWORD); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_DRAGON_DANCE);
            MOVE(playerRight, MOVE_MOONBLAST, target: opponentRight);
            NOT_EXPECT_MOVE(opponentRight, MOVE_KINGS_SHIELD);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI disrupts selected Geomancy instead of fearing unselected Encore")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_GEOMANCY) == EFFECT_GEOMANCY);
        ASSUME(GetMoveNonVolatileStatus(MOVE_SPORE) == MOVE_EFFECT_SLEEP);
        ASSUME(GetMoveEffect(MOVE_ENCORE) == EFFECT_ENCORE);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_SMART_GIMMICK | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WHIMSICOTT) { Level(50); Item(ITEM_FOCUS_SASH); Ability(ABILITY_PRANKSTER); MaxHP(135); HP(135); Defense(90); SpAttack(129); SpDefense(95); Speed(184); Moves(MOVE_TAILWIND, MOVE_ENCORE, MOVE_MOONBLAST, MOVE_PROTECT); }
        PLAYER(SPECIES_XERNEAS) { Level(50); Item(ITEM_POWER_HERB); MaxHP(241); HP(241); Defense(135); SpAttack(183); SpDefense(150); Speed(119); Moves(MOVE_GEOMANCY, MOVE_MOONBLAST, MOVE_DAZZLING_GLEAM, MOVE_PROTECT); }
        OPPONENT(SPECIES_KYOGRE) { Level(50); Ability(ABILITY_DRIZZLE); Item(ITEM_CHOICE_SPECS); MaxHP(205); HP(205); Defense(120); SpAttack(220); SpDefense(160); Speed(90); Moves(MOVE_WATER_SPOUT, MOVE_ORIGIN_PULSE, MOVE_THUNDER, MOVE_ICE_BEAM); }
        OPPONENT(SPECIES_AMOONGUSS) { Level(50); Item(ITEM_ROCKY_HELMET); Ability(ABILITY_REGENERATOR); MaxHP(221); HP(221); Defense(120); SpAttack(105); SpDefense(145); Speed(31); Moves(MOVE_SPORE, MOVE_RAGE_POWDER, MOVE_POLLEN_PUFF, MOVE_PROTECT); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_PROTECT);
            MOVE(playerRight, MOVE_GEOMANCY);
            EXPECT_MOVE(opponentRight, MOVE_SPORE, target: playerRight);
            SCORE_GT(opponentRight, MOVE_SPORE, MOVE_POLLEN_PUFF, target: playerRight);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: T2 joint runtime simulates Power Herb Geomancy before selected slower Knock Off")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_GEOMANCY) == EFFECT_GEOMANCY);
        ASSUME(GetMoveEffect(MOVE_KNOCK_OFF) == EFFECT_KNOCK_OFF);
        AI_FLAGS(AI_FLAG_SMART_TRAINER | AI_FLAG_PREDICTION | AI_FLAG_SMART_GIMMICK | AI_FLAG_SMART_SWITCHING
               | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT | AI_FLAG_KNOW_OPPONENT_PARTY
               | AI_FLAG_POWERFUL_STATUS | AI_FLAG_AGGRESSIVE_GIMMICK | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_BLISSEY) {
            Level(50); Item(ITEM_POWER_HERB); Ability(ABILITY_PRESSURE); Nature(NATURE_CALM); TEST_IVS_SPECIAL();
            MaxHP(330); HP(330); Defense(60); SpAttack(95); SpDefense(120); Speed(55);
            Moves(MOVE_GEOMANCY);
        }
        PLAYER(SPECIES_INCINEROAR) {
            Level(50); Item(ITEM_SITRUS_BERRY); Ability(ABILITY_GUTS); Nature(NATURE_CAREFUL);
            TEST_IVS_PHYSICAL();
            MaxHP(202); HP(202); Attack(1); Defense(120); SpDefense(146); Speed(80);
            Moves(MOVE_KNOCK_OFF);
        }
        OPPONENT(SPECIES_XERNEAS) {
            Level(50); Item(ITEM_POWER_HERB); Ability(ABILITY_GUTS); Nature(NATURE_MODEST); TEST_IVS_SPECIAL();
            MaxHP(223); HP(223); Defense(115); SpAttack(201); SpDefense(118); Speed(130);
            Moves(MOVE_GEOMANCY);
        }
        OPPONENT(SPECIES_MARSHADOW) {
            Level(50); Item(ITEM_POWER_HERB); Ability(ABILITY_TECHNICIAN); Nature(NATURE_JOLLY); TEST_IVS_PHYSICAL();
            MaxHP(166); HP(1); Attack(177); Defense(100); SpAttack(99); SpDefense(110); Speed(194);
            Moves(MOVE_GEOMANCY);
        }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_GEOMANCY);
            MOVE(playerRight, MOVE_KNOCK_OFF, target: opponentLeft);
            EXPECT_MOVE(opponentLeft, MOVE_GEOMANCY);
        }
    } THEN {
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        const struct BattleActionLogEntry *entry = BattleActionLog_GetLastEntry(battler, 1u << B_ACTION_USE_MOVE);
        const struct BattleAiTracePlan *plan;
        const struct BattleAiTraceCandidate *candidate;

        EXPECT(entry != NULL);
        EXPECT_EQ(Test_BattleAiJointRuntime_GetBuildCount(), 1);
        EXPECT_GT(Test_BattleAiJointRuntime_GetStepCount(), 0);
        EXPECT_NE(Test_BattleAiJointRuntime_GetPlanId(), BATTLE_AI_TRACE_ID_NONE);
        if (entry == NULL)
            return;
        EXPECT_EQ(entry->aiPlanId, Test_BattleAiJointRuntime_GetPlanId());
        plan = BattleAiTrace_GetPlan(entry->aiPlanId);
        EXPECT(plan != NULL);
        if (plan != NULL)
        {
            EXPECT(plan->flags & BATTLE_AI_TRACE_PLAN_JOINT);
            EXPECT(!(plan->flags & BATTLE_AI_TRACE_PLAN_LEGACY_EVALUATOR));
            EXPECT_GE(plan->completedDepth, AI_JOINT_STANDARD_DEPTH);
            EXPECT_EQ(entry->move, MOVE_GEOMANCY);
            candidate = BattleAiTrace_GetCandidate(plan->firstCandidateSequence + entry->aiCandidateRank);
            EXPECT(candidate != NULL);
            if (candidate != NULL)
                EXPECT(candidate->flags & BATTLE_AI_TRACE_CANDIDATE_JOINT);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: T2 unsupported spread action preflights to coherent legacy AI")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_SMART_TRAINER | AI_FLAG_PREDICTION | AI_FLAG_SMART_GIMMICK | AI_FLAG_SMART_SWITCHING
               | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT | AI_FLAG_KNOW_OPPONENT_PARTY
               | AI_FLAG_POWERFUL_STATUS | AI_FLAG_AGGRESSIVE_GIMMICK | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WOBBUFFET) {
            Item(ITEM_POWER_HERB); Ability(ABILITY_GUTS); Speed(180); Moves(MOVE_GEOMANCY);
        }
        PLAYER(SPECIES_WOBBUFFET) {
            Ability(ABILITY_GUTS); Speed(80); Moves(MOVE_KNOCK_OFF);
        }
        OPPONENT(SPECIES_XERNEAS) {
            Level(50); Item(ITEM_NONE); Ability(ABILITY_GUTS); Nature(NATURE_MODEST); TEST_IVS_SPECIAL();
            MaxHP(223); HP(223); Defense(115); SpAttack(201); SpDefense(118); Speed(130);
            Moves(MOVE_DAZZLING_GLEAM);
        }
        OPPONENT(SPECIES_WOBBUFFET) {
            Item(ITEM_POWER_HERB); Ability(ABILITY_GUTS); Speed(40); Moves(MOVE_GEOMANCY);
        }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_GEOMANCY);
            MOVE(playerRight, MOVE_KNOCK_OFF, target: opponentLeft);
            EXPECT_MOVE(opponentLeft, MOVE_DAZZLING_GLEAM);
            EXPECT_MOVE(opponentRight, MOVE_GEOMANCY);
        }
    } THEN {
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        const struct BattleActionLogEntry *entry =
            BattleActionLog_GetLastEntry(battler, 1u << B_ACTION_USE_MOVE);
        const struct BattleAiTracePlan *plan;

        EXPECT(entry != NULL);
        EXPECT_EQ(Test_BattleAiJointRuntime_GetBuildCount(), 0);
        EXPECT_EQ(Test_BattleAiJointRuntime_GetStepCount(), 0);
        EXPECT_EQ(Test_BattleAiJointRuntime_GetPlanId(), BATTLE_AI_TRACE_ID_NONE);
        EXPECT(!Test_BattleAiJointRuntime_IsAllocated());
        if (entry == NULL)
            return;
        EXPECT_EQ(entry->move, MOVE_DAZZLING_GLEAM);
        EXPECT_NE(entry->aiPlanId, BATTLE_AI_TRACE_ID_NONE);
        plan = BattleAiTrace_GetPlan(entry->aiPlanId);
        EXPECT(plan != NULL);
        if (plan != NULL)
        {
            EXPECT(plan->flags & BATTLE_AI_TRACE_PLAN_VALID);
            EXPECT(plan->flags & BATTLE_AI_TRACE_PLAN_LEGACY_EVALUATOR);
            EXPECT(!(plan->flags & BATTLE_AI_TRACE_PLAN_JOINT));
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: T2 Power Herb Geomancy gets no denial value when Knock Off targets its partner")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_GEOMANCY) == EFFECT_GEOMANCY);
        ASSUME(GetMoveEffect(MOVE_KNOCK_OFF) == EFFECT_KNOCK_OFF);
        AI_FLAGS(AI_FLAG_SMART_TRAINER | AI_FLAG_PREDICTION | AI_FLAG_SMART_GIMMICK | AI_FLAG_SMART_SWITCHING
               | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT | AI_FLAG_KNOW_OPPONENT_PARTY
               | AI_FLAG_POWERFUL_STATUS | AI_FLAG_AGGRESSIVE_GIMMICK | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_MARSHADOW_Z(MOVE_SPECTRAL_THIEF, MOVE_CLOSE_COMBAT, MOVE_SHADOW_SNEAK, MOVE_PROTECT);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        OPPONENT(SPECIES_XERNEAS) {
            Level(50); Item(ITEM_POWER_HERB); Nature(NATURE_MODEST); TEST_IVS_SPECIAL();
            MaxHP(223); HP(223); Defense(115); SpAttack(201); SpDefense(118); Speed(130);
            Moves(MOVE_GEOMANCY, MOVE_MOONBLAST, MOVE_DAZZLING_GLEAM, MOVE_PROTECT);
        }
        OPPONENT(SPECIES_MARSHADOW) {
            Level(50); Item(ITEM_MARSHADIUM_Z); Nature(NATURE_JOLLY); TEST_IVS_PHYSICAL();
            MaxHP(166); HP(166); Attack(177); Defense(100); SpAttack(99); SpDefense(110); Speed(194);
            Moves(MOVE_SPECTRAL_THIEF, MOVE_CLOSE_COMBAT, MOVE_SHADOW_SNEAK, MOVE_PROTECT);
        }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_SHADOW_SNEAK, target: opponentRight);
            MOVE(playerRight, MOVE_KNOCK_OFF, target: opponentRight);
            EXPECT_MOVES(opponentLeft, MOVE_MOONBLAST, MOVE_DAZZLING_GLEAM);
            SCORE_GT(opponentLeft, MOVE_MOONBLAST, MOVE_GEOMANCY, target: playerLeft);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: T2 Power Herb Geomancy gets no denial value against faster selected Knock Off")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_GEOMANCY) == EFFECT_GEOMANCY);
        ASSUME(GetMoveEffect(MOVE_KNOCK_OFF) == EFFECT_KNOCK_OFF);
        AI_FLAGS(AI_FLAG_SMART_TRAINER | AI_FLAG_PREDICTION | AI_FLAG_SMART_GIMMICK | AI_FLAG_SMART_SWITCHING
               | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT | AI_FLAG_KNOW_OPPONENT_PARTY
               | AI_FLAG_POWERFUL_STATUS | AI_FLAG_AGGRESSIVE_GIMMICK | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_MARSHADOW_Z(MOVE_SPECTRAL_THIEF, MOVE_KNOCK_OFF, MOVE_SHADOW_SNEAK, MOVE_PROTECT);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        OPPONENT(SPECIES_XERNEAS) {
            Level(50); Item(ITEM_POWER_HERB); Nature(NATURE_MODEST); TEST_IVS_SPECIAL();
            MaxHP(223); HP(223); Defense(115); SpAttack(201); SpDefense(118); Speed(130);
            Moves(MOVE_GEOMANCY, MOVE_MOONBLAST, MOVE_DAZZLING_GLEAM, MOVE_PROTECT);
        }
        OPPONENT(SPECIES_MARSHADOW) {
            Level(50); Item(ITEM_MARSHADIUM_Z); Nature(NATURE_JOLLY); TEST_IVS_PHYSICAL();
            MaxHP(166); HP(166); Attack(177); Defense(100); SpAttack(99); SpDefense(110); Speed(194);
            Moves(MOVE_SPECTRAL_THIEF, MOVE_CLOSE_COMBAT, MOVE_SHADOW_SNEAK, MOVE_PROTECT);
        }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_KNOCK_OFF, target: opponentLeft);
            MOVE(playerRight, MOVE_KNOCK_OFF, target: opponentRight);
            EXPECT_MOVES(opponentLeft, MOVE_MOONBLAST, MOVE_DAZZLING_GLEAM);
            SCORE_GT(opponentLeft, MOVE_MOONBLAST, MOVE_GEOMANCY, target: playerLeft);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI does not aim redirectable single-target pressure into selected Rage Powder")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_RAGE_POWDER) == EFFECT_FOLLOW_ME);
        ASSUME(GetMoveTarget(MOVE_PSYSTRIKE) == TARGET_SELECTED);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_SMART_GIMMICK | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_VOLCARONA) { Level(50); Item(ITEM_SITRUS_BERRY); Ability(ABILITY_FLAME_BODY); MaxHP(191); HP(191); Defense(220); SpAttack(170); SpDefense(135); Speed(120); Moves(MOVE_RAGE_POWDER, MOVE_HEAT_WAVE, MOVE_STRUGGLE_BUG, MOVE_PROTECT); }
        PLAYER(SPECIES_XERNEAS) { Level(50); Item(ITEM_POWER_HERB); MaxHP(241); HP(241); Defense(90); SpAttack(183); SpDefense(150); Speed(119); Moves(MOVE_GEOMANCY, MOVE_MOONBLAST, MOVE_DAZZLING_GLEAM, MOVE_PROTECT); }
        OPPONENT(SPECIES_MEWTWO) { Level(50); Item(ITEM_MEWTWONITE_Y); MaxHP(181); HP(181); Defense(110); SpAttack(206); SpDefense(110); Speed(200); Moves(MOVE_PSYSTRIKE, MOVE_ICE_BEAM, MOVE_AURA_SPHERE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_KYOGRE) { Level(50); Ability(ABILITY_DRIZZLE); Item(ITEM_CHOICE_SPECS); MaxHP(205); HP(205); Defense(120); SpAttack(220); SpDefense(160); Speed(90); Moves(MOVE_WATER_SPOUT, MOVE_ORIGIN_PULSE, MOVE_THUNDER, MOVE_ICE_BEAM); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_RAGE_POWDER);
            MOVE(playerRight, MOVE_DAZZLING_GLEAM);
            EXPECT_MOVE(opponentLeft, MOVE_PSYSTRIKE, target: playerLeft);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI switches Mega Gengar to Incineroar against a known cross-slot Grassy Glide KO")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_GRASSY_GLIDE) == EFFECT_GRASSY_GLIDE);
        ASSUME(GetMoveTarget(MOVE_GRASSY_GLIDE) == TARGET_SELECTED);
        ASSUME(GetMoveCategory(MOVE_GRASSY_GLIDE) == DAMAGE_CATEGORY_PHYSICAL);
        ASSUME(GetMoveEffect(MOVE_TAILWIND) == EFFECT_TAILWIND);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_RILLABOOM(MOVE_FAKE_OUT, MOVE_GRASSY_GLIDE, MOVE_WOOD_HAMMER, MOVE_KNOCK_OFF);
        PLAYER_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        OPPONENT_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        OPPONENT_GENGAR_MEGA_ACTIVE(35, MOVE_SHADOW_BALL, MOVE_SLUDGE_BOMB, MOVE_FOCUS_BLAST, MOVE_PROTECT);
        OPPONENT_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_GRASSY_GLIDE, target: opponentRight);
            MOVE(playerRight, MOVE_TAILWIND);
            EXPECT_SWITCH(opponentRight, 2);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI sacrifices a spent active instead of chipping its unused Dynamax win condition")
{
    GIVEN {
        ASSUME(GetMoveTarget(MOVE_TACKLE) == TARGET_SELECTED);
        ASSUME(GetMoveCategory(MOVE_TACKLE) == DAMAGE_CATEGORY_PHYSICAL);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_RILLABOOM(MOVE_TACKLE, MOVE_GRASSY_GLIDE, MOVE_WOOD_HAMMER, MOVE_KNOCK_OFF);
        PLAYER_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        OPPONENT_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        OPPONENT(SPECIES_MAGIKARP) {
            Level(5); Item(ITEM_NONE); Ability(ABILITY_SWIFT_SWIM);
            MaxHP(18); HP(1); Attack(10); Defense(10); SpDefense(10); Speed(20);
            Moves(MOVE_TACKLE);
        }
        OPPONENT_INCINEROAR_DMAX_LEVEL_100(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_TACKLE, target: opponentRight);
            MOVE(playerRight, MOVE_TAILWIND);
            EXPECT_MOVE(opponentRight, MOVE_TACKLE, target: playerLeft);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI accepts reserve chip when preserving the active is worth more")
{
    GIVEN {
        ASSUME(GetMoveTarget(MOVE_TACKLE) == TARGET_SELECTED);
        ASSUME(GetMoveCategory(MOVE_TACKLE) == DAMAGE_CATEGORY_PHYSICAL);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_RILLABOOM(MOVE_TACKLE, MOVE_GRASSY_GLIDE, MOVE_WOOD_HAMMER, MOVE_KNOCK_OFF);
        PLAYER_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        OPPONENT_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        OPPONENT_GENGAR_MEGA_ACTIVE(35, MOVE_SHADOW_BALL, MOVE_SLUDGE_BOMB, MOVE_FOCUS_BLAST, MOVE_PROTECT);
        OPPONENT_INCINEROAR_DMAX_LEVEL_100(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_TACKLE, target: opponentRight);
            MOVE(playerRight, MOVE_TAILWIND);
            EXPECT_SWITCH(opponentRight, 2);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI exposes a lower-value cushion before its unused Dynamax win condition")
{
    GIVEN {
        ASSUME(GetMoveTarget(MOVE_TACKLE) == TARGET_SELECTED);
        ASSUME(GetMoveCategory(MOVE_TACKLE) == DAMAGE_CATEGORY_PHYSICAL);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_RILLABOOM(MOVE_TACKLE, MOVE_GRASSY_GLIDE, MOVE_WOOD_HAMMER, MOVE_KNOCK_OFF);
        PLAYER_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        OPPONENT_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        OPPONENT_GENGAR_MEGA_ACTIVE(35, MOVE_SHADOW_BALL, MOVE_SLUDGE_BOMB, MOVE_FOCUS_BLAST, MOVE_PROTECT);
        OPPONENT_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        OPPONENT_INCINEROAR_DMAX_LEVEL_100(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_TACKLE, target: opponentRight);
            MOVE(playerRight, MOVE_TAILWIND);
            EXPECT_SWITCH(opponentRight, 2);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI sacrifices Whimsicott to preserve Choice Kyogre from selected Z plus Fake Out focus")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        ASSUME(GetMoveTarget(MOVE_THUNDERBOLT) == TARGET_SELECTED);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_TAPU_KOKO_Z(MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_VOLT_SWITCH, MOVE_PROTECT);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        OPPONENT_TAPU_KOKO_Z(MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_VOLT_SWITCH, MOVE_PROTECT);
        OPPONENT_KYOGRE(MOVE_WATER_SPOUT, MOVE_ORIGIN_PULSE, MOVE_THUNDER, MOVE_ICE_BEAM);
        OPPONENT_WHIMSICOTT(MOVE_TAILWIND, MOVE_ENCORE, MOVE_MOONBLAST, MOVE_PROTECT);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_THUNDERBOLT, gimmick: GIMMICK_Z_MOVE, target: opponentRight);
            MOVE(playerRight, MOVE_FAKE_OUT, target: opponentRight);
            EXPECT_SWITCH(opponentRight, 2);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI prefers a nonfatal cushion over a doomed support sacrifice")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        ASSUME(GetMoveTarget(MOVE_THUNDERBOLT) == TARGET_SELECTED);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_TAPU_KOKO_Z(MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_VOLT_SWITCH, MOVE_PROTECT);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        OPPONENT_TAPU_KOKO_Z(MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_VOLT_SWITCH, MOVE_PROTECT);
        OPPONENT(SPECIES_KYOGRE) {
            Level(50); Item(ITEM_CHOICE_SPECS); Ability(ABILITY_DRIZZLE); Nature(NATURE_MODEST);
            TEST_IVS_SPECIAL();
            MaxHP(176); HP(35); Defense(110); SpAttack(222); SpDefense(160); Speed(140);
            Moves(MOVE_WATER_SPOUT, MOVE_ORIGIN_PULSE, MOVE_THUNDER, MOVE_ICE_BEAM);
        }
        OPPONENT_WHIMSICOTT(MOVE_TAILWIND, MOVE_ENCORE, MOVE_MOONBLAST, MOVE_PROTECT);
        OPPONENT(SPECIES_HARIYAMA) {
            Level(50); MaxHP(500); HP(500); Defense(80); SpDefense(80); Speed(40);
            Moves(MOVE_CELEBRATE);
        }
        OPPONENT(SPECIES_MAGIKARP) { Level(50); Speed(1); Moves(MOVE_SPLASH); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_THUNDERBOLT, gimmick: GIMMICK_Z_MOVE, target: opponentRight);
            MOVE(playerRight, MOVE_FAKE_OUT, target: opponentRight);
            EXPECT_SWITCH(opponentRight, 3);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI values a pivot-capable sacrifice in focused collapse")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        ASSUME(GetMoveEffect(MOVE_U_TURN) == EFFECT_HIT_ESCAPE);
        ASSUME(GetMoveTarget(MOVE_THUNDERBOLT) == TARGET_SELECTED);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_TAPU_KOKO_Z(MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_VOLT_SWITCH, MOVE_PROTECT);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        OPPONENT_TAPU_KOKO_Z(MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_VOLT_SWITCH, MOVE_PROTECT);
        OPPONENT(SPECIES_KYOGRE) {
            Level(50); Item(ITEM_CHOICE_SPECS); Ability(ABILITY_DRIZZLE); Nature(NATURE_MODEST);
            TEST_IVS_SPECIAL();
            MaxHP(176); HP(35); Defense(110); SpAttack(222); SpDefense(160); Speed(140);
            Moves(MOVE_WATER_SPOUT, MOVE_ORIGIN_PULSE, MOVE_THUNDER, MOVE_ICE_BEAM);
        }
        OPPONENT(SPECIES_CROBAT) { Level(50); Item(ITEM_FOCUS_SASH); Speed(40); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_CROBAT) { Level(50); Item(ITEM_FOCUS_SASH); Speed(40); Moves(MOVE_U_TURN); }
        OPPONENT(SPECIES_MAGIKARP) { Level(50); Speed(1); Moves(MOVE_SPLASH); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_THUNDERBOLT, gimmick: GIMMICK_Z_MOVE, target: opponentRight);
            MOVE(playerRight, MOVE_FAKE_OUT, target: opponentRight);
            EXPECT_SWITCH(opponentRight, 3);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI does not trust Protect when selected Feint breaks the pinned Kyogre line")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_PROTECT) == EFFECT_PROTECT);
        ASSUME(MoveIgnoresProtect(MOVE_FEINT));
        ASSUME(GetMoveTarget(MOVE_THUNDERBOLT) == TARGET_SELECTED);
        ASSUME(GetMoveTarget(MOVE_FEINT) == TARGET_SELECTED);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_TAPU_KOKO_Z(MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_VOLT_SWITCH, MOVE_PROTECT);
        PLAYER(SPECIES_MIENSHAO) {
            Level(50); Item(ITEM_FOCUS_SASH); Ability(ABILITY_INNER_FOCUS); Nature(NATURE_JOLLY);
            TEST_IVS_PHYSICAL();
            MaxHP(140); HP(140); Attack(177); Defense(80); SpDefense(80); Speed(172);
            Moves(MOVE_FAKE_OUT, MOVE_FEINT, MOVE_CLOSE_COMBAT, MOVE_WIDE_GUARD);
        }
        OPPONENT_TAPU_KOKO_Z(MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_VOLT_SWITCH, MOVE_PROTECT);
        OPPONENT_KYOGRE_MYSTIC_WATER(MOVE_WATER_SPOUT, MOVE_ORIGIN_PULSE, MOVE_ICE_BEAM, MOVE_PROTECT);
        OPPONENT_WHIMSICOTT(MOVE_TAILWIND, MOVE_ENCORE, MOVE_MOONBLAST, MOVE_PROTECT);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_THUNDERBOLT, gimmick: GIMMICK_Z_MOVE, target: opponentRight);
            MOVE(playerRight, MOVE_FEINT, target: opponentRight);
            EXPECT_SWITCH(opponentRight, 2);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI does not trust Protect when selected Z chip KOs through it")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_PROTECT) == EFFECT_PROTECT);
        ASSUME(GetMoveTarget(MOVE_THUNDERBOLT) == TARGET_SELECTED);
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_TAPU_KOKO_Z(MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_VOLT_SWITCH, MOVE_PROTECT);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        OPPONENT_TAPU_KOKO_Z(MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_VOLT_SWITCH, MOVE_PROTECT);
        OPPONENT(SPECIES_KYOGRE) {
            Level(50); Item(ITEM_MYSTIC_WATER); Nature(NATURE_MODEST); DynamaxLevel(10);
            TEST_IVS_SPECIAL();
            MaxHP(176); HP(35); Defense(110); SpAttack(222); SpDefense(160); Speed(140);
            Moves(MOVE_WATER_SPOUT, MOVE_ORIGIN_PULSE, MOVE_ICE_BEAM, MOVE_PROTECT);
        }
        OPPONENT_WHIMSICOTT(MOVE_TAILWIND, MOVE_ENCORE, MOVE_MOONBLAST, MOVE_PROTECT);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_THUNDERBOLT, gimmick: GIMMICK_Z_MOVE, target: opponentRight);
            MOVE(playerRight, MOVE_FAKE_OUT, target: opponentRight);
            EXPECT_SWITCH(opponentRight, 2);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI accepts a 15-of-16 survival switch when focus pressure leaves no clean pivot")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        ASSUME(GetMoveTarget(MOVE_THUNDERBOLT) == TARGET_SELECTED);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_TAPU_KOKO_Z(MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_VOLT_SWITCH, MOVE_PROTECT);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        OPPONENT_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        OPPONENT(SPECIES_KYOGRE) {
            Level(50); Item(ITEM_CHOICE_SPECS); Ability(ABILITY_DRIZZLE); Nature(NATURE_MODEST);
            TEST_IVS_SPECIAL();
            MaxHP(176); HP(35); Defense(110); SpAttack(222); SpDefense(160); Speed(140);
            Moves(MOVE_WATER_SPOUT, MOVE_ORIGIN_PULSE, MOVE_THUNDER, MOVE_ICE_BEAM);
        }
        OPPONENT(SPECIES_KORAIDON) {
            Level(50); Item(ITEM_CHOICE_BAND); Nature(NATURE_JOLLY);
            TEST_IVS_PHYSICAL();
            MaxHP(176); HP(38); Attack(205); Defense(135); SpDefense(120); Speed(205);
            Moves(MOVE_COLLISION_COURSE, MOVE_DRAGON_CLAW, MOVE_FLARE_BLITZ, MOVE_PROTECT);
        }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_THUNDERBOLT, target: opponentRight);
            MOVE(playerRight, MOVE_FAKE_OUT, target: opponentRight);
            EXPECT_SWITCH(opponentRight, 2);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI rejects a doomed sacrifice when it does not improve next board")
{
    GIVEN {
        ASSUME(GetMoveTarget(MOVE_THUNDERBOLT) == TARGET_SELECTED);
        ASSUME(GetMoveTarget(MOVE_BLEAKWIND_STORM) == TARGET_BOTH);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_TAPU_KOKO_Z(MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_VOLT_SWITCH, MOVE_PROTECT);
        PLAYER_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        OPPONENT_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        OPPONENT(SPECIES_KYOGRE) {
            Level(50); Item(ITEM_CHOICE_SPECS); Ability(ABILITY_DRIZZLE); Nature(NATURE_MODEST);
            TEST_IVS_SPECIAL();
            MaxHP(176); HP(35); Defense(110); SpAttack(222); SpDefense(160); Speed(140);
            Moves(MOVE_WATER_SPOUT, MOVE_ORIGIN_PULSE, MOVE_THUNDER, MOVE_ICE_BEAM);
        }
        OPPONENT(SPECIES_MAGIKARP) {
            Level(50); Item(ITEM_NONE); Nature(NATURE_TIMID);
            MaxHP(40); HP(40); Defense(20); SpDefense(20); Speed(80);
            Moves(MOVE_SPLASH);
        }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_THUNDERBOLT, target: opponentRight);
            MOVE(playerRight, MOVE_BLEAKWIND_STORM);
            EXPECT_MOVES(opponentRight, MOVE_WATER_SPOUT, MOVE_ORIGIN_PULSE, MOVE_THUNDER, MOVE_ICE_BEAM);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: AI does not switch Mega Gengar to Incineroar against a punishing known cross-slot hit")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_PROTECT) == EFFECT_PROTECT);
        ASSUME(GetMoveTarget(MOVE_HIGH_HORSEPOWER) == TARGET_SELECTED);
        ASSUME(GetMoveCategory(MOVE_HIGH_HORSEPOWER) == DAMAGE_CATEGORY_PHYSICAL);
        ASSUME(GetMoveEffect(MOVE_TAILWIND) == EFFECT_TAILWIND);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_RILLABOOM) {
            Level(50); Item(ITEM_CHOICE_BAND); Ability(ABILITY_GRASSY_SURGE); Nature(NATURE_ADAMANT);
            TEST_IVS_PHYSICAL();
            MaxHP(207); HP(207); Attack(187); Defense(110); SpDefense(98); Speed(105);
            Moves(MOVE_FAKE_OUT, MOVE_GRASSY_GLIDE, MOVE_WOOD_HAMMER, MOVE_HIGH_HORSEPOWER);
        }
        PLAYER_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        OPPONENT_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        OPPONENT_GENGAR_MEGA_ACTIVE(35, MOVE_SHADOW_BALL, MOVE_SLUDGE_BOMB, MOVE_FOCUS_BLAST, MOVE_PROTECT);
        OPPONENT_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_HIGH_HORSEPOWER, target: opponentRight);
            MOVE(playerRight, MOVE_TAILWIND);
            EXPECT_MOVES(opponentRight, MOVE_SHADOW_BALL, MOVE_SLUDGE_BOMB, MOVE_FOCUS_BLAST, MOVE_PROTECT);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: selected Fake Out discounts slower ordinary actions")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_PREDICTION | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        PLAYER_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        OPPONENT_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        OPPONENT_KORAIDON(MOVE_DRAGON_CLAW, MOVE_PROTECT);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_FAKE_OUT, target: opponentRight);
            MOVE(playerRight, MOVE_TAILWIND);
            EXPECT_MOVES(opponentLeft, MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
            SCORE_LT_VAL(opponentRight, MOVE_DRAGON_CLAW, AI_SCORE_DEFAULT, target: playerLeft);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: T0 Whimsicott protects instead of losing Tailwind to confirmed Fake Out")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        ASSUME(GetMoveEffect(MOVE_PROTECT) == EFFECT_PROTECT);
        ASSUME(GetMoveEffect(MOVE_TAILWIND) == EFFECT_TAILWIND);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_PREDICTION | AI_FLAG_POWERFUL_STATUS | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_MARSHADOW_Z(MOVE_SPECTRAL_THIEF, MOVE_CLOSE_COMBAT, MOVE_SHADOW_SNEAK, MOVE_PROTECT);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        OPPONENT_TAPU_KOKO_Z(MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_VOLT_SWITCH, MOVE_PROTECT);
        OPPONENT_WHIMSICOTT(MOVE_TAILWIND, MOVE_ENCORE, MOVE_MOONBLAST, MOVE_PROTECT);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_SPECTRAL_THIEF, gimmick: GIMMICK_Z_MOVE, target: opponentLeft);
            MOVE(playerRight, MOVE_FAKE_OUT, target: opponentRight);
            EXPECT_MOVES(opponentRight, MOVE_PROTECT, MOVE_TAILWIND);
        }
    } THEN {
        enum BattlerId source = GetBattlerAtPosition(B_POSITION_PLAYER_RIGHT);
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT);
        const struct BattleActionLogEntry *entry = BattleActionLog_GetLastEntry(battler, 1u << B_ACTION_USE_MOVE);
        const struct BattleAiTracePlan *plan;
        const struct BattleAiTraceCandidate *candidate;

        EXPECT(entry != NULL);
        EXPECT_EQ(entry->move, MOVE_PROTECT);
        EXPECT_EQ(entry->aiReason, AI_DECISION_REASON_KNOWN_COMMAND_ANSWER);
        EXPECT_NE(entry->aiPlanId, BATTLE_AI_TRACE_ID_NONE);
        plan = BattleAiTrace_GetPlan(entry->aiPlanId);
        EXPECT(plan != NULL);
        if (plan != NULL)
        {
            candidate = BattleAiTrace_GetCandidate(plan->firstCandidateSequence + entry->aiCandidateRank);
            EXPECT(candidate != NULL);
            if (candidate != NULL)
            {
                EXPECT(candidate->readInteractionFlags[0] & AI_READ_INTERACTION_PROTECT);
                EXPECT(candidate->readInteractionFlags[0] & AI_READ_INTERACTION_FAKE_OUT);
            }
        }

        gChosenActionByBattler[source] = B_ACTION_USE_MOVE;
        gChosenMoveByBattler[source] = MOVE_FAKE_OUT;
        gBattleStruct->chosenMovePositions[source] = 0;
        gBattleStruct->moveTarget[source] = battler;
        gBattleStruct->battlerState[source].isFirstTurn = 1;
        gAiThinkingStruct->aiFlags[battler] |= AI_FLAG_READ_PLAYER_MOVE;
        SetAiLogicDataForTurn(gAiLogicData);
        BattleAI_SetupAIData(0xF, battler);
        gAiLogicData->partnerMove = MOVE_NONE;

        EXPECT_EQ(Test_AI_CheckBadMove(battler, battler, MOVE_TAILWIND, 0, AI_SCORE_DEFAULT), 0);
        EXPECT_GT(Test_AI_CheckBadMove(battler, battler, MOVE_PROTECT, 3, AI_SCORE_DEFAULT), 0);
        EXPECT(gAiBattleData->candidateRejectionFlags[battler][battler][0] & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH);
        EXPECT(gAiBattleData->candidateReadInteractionFlags[battler][battler][0] & AI_READ_INTERACTION_FAKE_OUT);

        gAiBattleData->candidateReadInteractionFlags[battler][battler][1] = AI_READ_INTERACTION_NONE;
        Test_AI_CheckBadMove(battler, battler, MOVE_WIDE_GUARD, 1, AI_SCORE_DEFAULT);
        EXPECT(!(gAiBattleData->candidateReadInteractionFlags[battler][battler][1] & AI_READ_INTERACTION_PROTECT));
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: selected Dynamax action bypasses the confirmed Fake Out hard gate")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PROTECT);
        OPPONENT_CHARIZARD_DMAX(MOVE_AIR_SLASH, MOVE_PROTECT);
    } WHEN {
        TURN {
            MOVE(player, MOVE_PROTECT);
            EXPECT_MOVES(opponent, MOVE_AIR_SLASH, MOVE_PROTECT);
        }
    } THEN {
        enum BattlerId source = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);

        SetupConfirmedFakeOutBadMoveTest(source, battler);
        SetActiveGimmick(battler, GIMMICK_NONE);
        gBattleStruct->gimmick.usableGimmick[battler] = GIMMICK_DYNAMAX;
        gBattleStruct->gimmick.toActivate |= 1u << battler;
        SetAIUsingGimmick(battler, USE_GIMMICK);
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_EQ(GetActiveGimmick(battler), GIMMICK_NONE);
        EXPECT_GT(Test_AI_CheckBadMove(battler, source, MOVE_AIR_SLASH, 0, AI_SCORE_DEFAULT), 0);
        EXPECT(!(gAiBattleData->candidateRejectionFlags[battler][source][0] & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH));
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: selected Ghost Tera action bypasses Normal Fake Out hard gate")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        ASSUME(GetMoveType(MOVE_FAKE_OUT) == TYPE_NORMAL);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PROTECT);
        OPPONENT(SPECIES_WHIMSICOTT) {
            Level(50); Item(ITEM_FOCUS_SASH); Ability(ABILITY_PRANKSTER); Nature(NATURE_TIMID); TeraType(TYPE_GHOST);
            TEST_IVS_SPECIAL();
            MaxHP(135); HP(135); Defense(105); SpAttack(129); SpDefense(95); Speed(184);
            Moves(MOVE_MOONBLAST, MOVE_PROTECT);
        }
    } WHEN {
        TURN {
            MOVE(player, MOVE_PROTECT);
            EXPECT_MOVES(opponent, MOVE_MOONBLAST, MOVE_PROTECT);
        }
    } THEN {
        enum BattlerId source = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);

        SetupConfirmedFakeOutBadMoveTest(source, battler);
        SetActiveGimmick(battler, GIMMICK_NONE);
        gBattleStruct->gimmick.usableGimmick[battler] = GIMMICK_TERA;
        gBattleStruct->gimmick.toActivate |= 1u << battler;
        SetAIUsingGimmick(battler, USE_GIMMICK);
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_EQ(GetActiveGimmick(battler), GIMMICK_NONE);
        EXPECT_EQ(GetBattlerTeraType(battler), TYPE_GHOST);
        EXPECT_EQ(Test_AI_CheckBadMove(battler, source, MOVE_MOONBLAST, 0, AI_SCORE_DEFAULT), AI_SCORE_DEFAULT);
        EXPECT(!(gAiBattleData->candidateRejectionFlags[battler][source][0] & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH));
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: selected non-Ghost Tera exposes a natural Ghost to confirmed Fake Out")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        ASSUME(GetMoveType(MOVE_FAKE_OUT) == TYPE_NORMAL);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PROTECT);
        OPPONENT(SPECIES_GENGAR) {
            Item(ITEM_FOCUS_SASH); Ability(ABILITY_CURSED_BODY); Speed(184); TeraType(TYPE_STEEL);
            Moves(MOVE_SHADOW_BALL, MOVE_PROTECT);
        }
    } WHEN {
        TURN {
            MOVE(player, MOVE_PROTECT);
            EXPECT_MOVES(opponent, MOVE_SHADOW_BALL, MOVE_PROTECT);
        }
    } THEN {
        enum BattlerId source = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);

        SetupConfirmedFakeOutBadMoveTest(source, battler);
        SetActiveGimmick(battler, GIMMICK_NONE);
        gBattleStruct->gimmick.toActivate &= ~(1u << battler);
        SetAIUsingGimmick(battler, NO_GIMMICK);
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT(IS_BATTLER_OF_TYPE(battler, TYPE_GHOST));
        EXPECT_GT(Test_AI_CheckBadMove(battler, source, MOVE_SHADOW_BALL, 0, AI_SCORE_DEFAULT), 0);
        EXPECT(!(gAiBattleData->candidateRejectionFlags[battler][source][0]
              & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH));

        gBattleStruct->gimmick.usableGimmick[battler] = GIMMICK_TERA;
        gBattleStruct->gimmick.toActivate |= 1u << battler;
        SetAIUsingGimmick(battler, USE_GIMMICK);
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_EQ(GetBattlerTeraType(battler), TYPE_STEEL);
        EXPECT_EQ(Test_AI_CheckBadMove(battler, source, MOVE_SHADOW_BALL, 0, AI_SCORE_DEFAULT), 0);
        EXPECT(gAiBattleData->candidateRejectionFlags[battler][source][0]
             & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH);
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: type-changing ability Fake Out is not exempted by selected Ghost Tera")
{
    u32 ability;

    PARAMETRIZE { ability = ABILITY_AERILATE; }
    PARAMETRIZE { ability = ABILITY_DRAGONIZE; }
    PARAMETRIZE { ability = ABILITY_GALVANIZE; }
    PARAMETRIZE { ability = ABILITY_PIXILATE; }
    PARAMETRIZE { ability = ABILITY_REFRIGERATE; }

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        ASSUME(GetMoveType(MOVE_FAKE_OUT) == TYPE_NORMAL);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WOBBUFFET) {
            Level(50); Ability(ability); Speed(200);
            Moves(MOVE_FAKE_OUT, MOVE_PROTECT);
        }
        OPPONENT(SPECIES_WHIMSICOTT) {
            Level(50); Item(ITEM_FOCUS_SASH); Ability(ABILITY_PRANKSTER); Nature(NATURE_TIMID); TeraType(TYPE_GHOST);
            TEST_IVS_SPECIAL();
            MaxHP(135); HP(135); Defense(105); SpAttack(129); SpDefense(95); Speed(184);
            Moves(MOVE_MOONBLAST, MOVE_PROTECT);
        }
    } WHEN {
        TURN {
            MOVE(player, MOVE_PROTECT);
            EXPECT_MOVES(opponent, MOVE_MOONBLAST, MOVE_PROTECT);
        }
    } THEN {
        enum BattlerId source = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);

        SetupConfirmedFakeOutBadMoveTest(source, battler);
        SetActiveGimmick(battler, GIMMICK_NONE);
        gBattleStruct->gimmick.usableGimmick[battler] = GIMMICK_TERA;
        gBattleStruct->gimmick.toActivate |= 1u << battler;
        SetAIUsingGimmick(battler, USE_GIMMICK);
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_EQ(GetActiveGimmick(battler), GIMMICK_NONE);
        EXPECT_EQ(GetBattlerTeraType(battler), TYPE_GHOST);
        EXPECT_EQ(gAiLogicData->abilities[source], ability);
        EXPECT_EQ(Test_AI_CheckBadMove(battler, source, MOVE_MOONBLAST, 0, AI_SCORE_DEFAULT), 0);
        EXPECT(gAiBattleData->candidateRejectionFlags[battler][source][0] & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH);
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: Scrappy and Mind's Eye Fake Out still gate selected Ghost Tera")
{
    u32 ability;

    PARAMETRIZE { ability = ABILITY_SCRAPPY; }
    PARAMETRIZE { ability = ABILITY_MINDS_EYE; }

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        ASSUME(GetMoveType(MOVE_FAKE_OUT) == TYPE_NORMAL);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ability); Speed(200); Moves(MOVE_FAKE_OUT, MOVE_PROTECT); }
        OPPONENT(SPECIES_WHIMSICOTT) {
            Item(ITEM_FOCUS_SASH); Ability(ABILITY_PRANKSTER); Speed(184); TeraType(TYPE_GHOST);
            Moves(MOVE_MOONBLAST, MOVE_PROTECT);
        }
    } WHEN {
        TURN {
            MOVE(player, MOVE_PROTECT);
            EXPECT_MOVES(opponent, MOVE_MOONBLAST, MOVE_PROTECT);
        }
    } THEN {
        enum BattlerId source = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);

        SetupConfirmedFakeOutBadMoveTest(source, battler);
        SetActiveGimmick(battler, GIMMICK_NONE);
        gBattleStruct->gimmick.usableGimmick[battler] = GIMMICK_TERA;
        gBattleStruct->gimmick.toActivate |= 1u << battler;
        SetAIUsingGimmick(battler, USE_GIMMICK);
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_EQ(gAiLogicData->abilities[source], ability);
        EXPECT_EQ(Test_AI_CheckBadMove(battler, source, MOVE_MOONBLAST, 0, AI_SCORE_DEFAULT), 0);
        EXPECT(gAiBattleData->candidateRejectionFlags[battler][source][0]
             & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH);
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: Ring Target and Foresight keep Normal Fake Out live through selected Ghost Tera")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        ASSUME(GetMoveType(MOVE_FAKE_OUT) == TYPE_NORMAL);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Speed(200); Moves(MOVE_FAKE_OUT, MOVE_PROTECT); }
        OPPONENT(SPECIES_WHIMSICOTT) {
            Item(ITEM_RING_TARGET); Ability(ABILITY_PRANKSTER); Speed(184); TeraType(TYPE_GHOST);
            Moves(MOVE_MOONBLAST, MOVE_PROTECT);
        }
    } WHEN {
        TURN {
            MOVE(player, MOVE_PROTECT);
            EXPECT_MOVES(opponent, MOVE_MOONBLAST, MOVE_PROTECT);
        }
    } THEN {
        enum BattlerId source = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);

        SetupConfirmedFakeOutBadMoveTest(source, battler);
        SetActiveGimmick(battler, GIMMICK_NONE);
        gBattleStruct->gimmick.usableGimmick[battler] = GIMMICK_TERA;
        gBattleStruct->gimmick.toActivate |= 1u << battler;
        SetAIUsingGimmick(battler, USE_GIMMICK);
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_EQ(gAiLogicData->holdEffects[battler], HOLD_EFFECT_RING_TARGET);
        EXPECT_EQ(Test_AI_CheckBadMove(battler, source, MOVE_MOONBLAST, 0, AI_SCORE_DEFAULT), 0);
        EXPECT(gAiBattleData->candidateRejectionFlags[battler][source][0]
             & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH);

        gAiLogicData->holdEffects[battler] = HOLD_EFFECT_NONE;
        gBattleMons[battler].volatiles.foresight = TRUE;
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;
        EXPECT_EQ(Test_AI_CheckBadMove(battler, source, MOVE_MOONBLAST, 0, AI_SCORE_DEFAULT), 0);
        EXPECT(gAiBattleData->candidateRejectionFlags[battler][source][0]
             & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH);
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: selected Mega ability Fake Out still gates selected Ghost Tera")
{
    u32 sourceSpecies;
    u32 sourceItem;
    u32 sourceAbility;
    u32 megaSpecies;
    u32 megaAbility;

    PARAMETRIZE {
        sourceSpecies = SPECIES_LOPUNNY;
        sourceItem = ITEM_LOPUNNITE;
        sourceAbility = ABILITY_LIMBER;
        megaSpecies = SPECIES_LOPUNNY_MEGA;
        megaAbility = ABILITY_SCRAPPY;
    }
    PARAMETRIZE {
        sourceSpecies = SPECIES_FERALIGATR;
        sourceItem = ITEM_FERALIGITE;
        sourceAbility = ABILITY_TORRENT;
        megaSpecies = SPECIES_FERALIGATR_MEGA;
        megaAbility = ABILITY_DRAGONIZE;
    }

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        ASSUME(GetMoveType(MOVE_FAKE_OUT) == TYPE_NORMAL);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(sourceSpecies) {
            Item(sourceItem); Ability(sourceAbility); Speed(200); Moves(MOVE_FAKE_OUT, MOVE_PROTECT);
        }
        OPPONENT(SPECIES_WHIMSICOTT) {
            Item(ITEM_FOCUS_SASH); Ability(ABILITY_PRANKSTER); Speed(184); TeraType(TYPE_GHOST);
            Moves(MOVE_MOONBLAST, MOVE_PROTECT);
        }
    } WHEN {
        TURN {
            MOVE(player, MOVE_PROTECT);
            EXPECT_MOVES(opponent, MOVE_MOONBLAST, MOVE_PROTECT);
        }
    } THEN {
        enum BattlerId source = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);

        SetupConfirmedFakeOutBadMoveTest(source, battler);
        gBattleStruct->gimmick.usableGimmick[source] = GIMMICK_MEGA;
        gBattleStruct->gimmick.toActivate |= 1u << source;
        SetActiveGimmick(battler, GIMMICK_NONE);
        gBattleStruct->gimmick.usableGimmick[battler] = GIMMICK_TERA;
        gBattleStruct->gimmick.toActivate |= 1u << battler;
        SetAIUsingGimmick(battler, USE_GIMMICK);
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_EQ(gAiLogicData->abilities[source], sourceAbility);
        EXPECT_EQ(GetAbilityBySpecies(megaSpecies, gBattleMons[source].abilityNum), megaAbility);
        EXPECT_EQ(Test_AI_CheckBadMove(battler, source, MOVE_MOONBLAST, 0, AI_SCORE_DEFAULT), 0);
        EXPECT(gAiBattleData->candidateRejectionFlags[battler][source][0]
             & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH);
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: planned Mega Scrappy or Dragonize Fake Out gates a natural Ghost")
{
    u32 sourceSpecies;
    u32 sourceItem;
    u32 sourceAbility;
    u32 megaSpecies;
    u32 megaAbility;

    PARAMETRIZE {
        sourceSpecies = SPECIES_LOPUNNY;
        sourceItem = ITEM_LOPUNNITE;
        sourceAbility = ABILITY_LIMBER;
        megaSpecies = SPECIES_LOPUNNY_MEGA;
        megaAbility = ABILITY_SCRAPPY;
    }
    PARAMETRIZE {
        sourceSpecies = SPECIES_FERALIGATR;
        sourceItem = ITEM_FERALIGITE;
        sourceAbility = ABILITY_TORRENT;
        megaSpecies = SPECIES_FERALIGATR_MEGA;
        megaAbility = ABILITY_DRAGONIZE;
    }

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        ASSUME(GetMoveType(MOVE_FAKE_OUT) == TYPE_NORMAL);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(sourceSpecies) {
            Item(sourceItem); Ability(sourceAbility); Speed(200); Moves(MOVE_FAKE_OUT, MOVE_PROTECT);
        }
        OPPONENT(SPECIES_GENGAR) {
            Item(ITEM_FOCUS_SASH); Ability(ABILITY_CURSED_BODY); Speed(184);
            Moves(MOVE_SHADOW_BALL, MOVE_PROTECT);
        }
    } WHEN {
        TURN {
            MOVE(player, MOVE_PROTECT);
            EXPECT_MOVES(opponent, MOVE_SHADOW_BALL, MOVE_PROTECT);
        }
    } THEN {
        enum BattlerId source = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);

        SetupConfirmedFakeOutBadMoveTest(source, battler);
        gBattleStruct->gimmick.toActivate &= ~(1u << source);
        SetAIUsingGimmick(battler, NO_GIMMICK);
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT(IS_BATTLER_OF_TYPE(battler, TYPE_GHOST));
        EXPECT_EQ(gAiLogicData->abilities[source], sourceAbility);
        EXPECT_GT(Test_AI_CheckBadMove(battler, source, MOVE_SHADOW_BALL, 0, AI_SCORE_DEFAULT), 0);
        EXPECT(!(gAiBattleData->candidateRejectionFlags[battler][source][0]
              & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH));

        gBattleStruct->gimmick.usableGimmick[source] = GIMMICK_MEGA;
        gBattleStruct->gimmick.toActivate |= 1u << source;
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_EQ(GetAbilityBySpecies(megaSpecies, gBattleMons[source].abilityNum), megaAbility);
        EXPECT_EQ(Test_AI_CheckBadMove(battler, source, MOVE_SHADOW_BALL, 0, AI_SCORE_DEFAULT), 0);
        EXPECT(gAiBattleData->candidateRejectionFlags[battler][source][0]
             & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH);
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: current Sheer Force removes both confirmed Fake Out penalties")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_HARIYAMA) {
            Ability(ABILITY_SHEER_FORCE); Speed(200); Moves(MOVE_FAKE_OUT, MOVE_PROTECT);
        }
        OPPONENT(SPECIES_WHIMSICOTT) {
            Item(ITEM_FOCUS_SASH); Ability(ABILITY_PRANKSTER); Speed(184);
            Moves(MOVE_MOONBLAST, MOVE_PROTECT);
        }
    } WHEN {
        TURN {
            MOVE(player, MOVE_PROTECT);
            EXPECT_MOVES(opponent, MOVE_MOONBLAST, MOVE_PROTECT);
        }
    } THEN {
        enum BattlerId source = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);

        SetupConfirmedFakeOutBadMoveTest(source, battler);
        SetAIUsingGimmick(battler, NO_GIMMICK);
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_EQ(gAiLogicData->abilities[source], ABILITY_SHEER_FORCE);
        EXPECT_EQ(Test_AI_CheckBadMove(battler, source, MOVE_MOONBLAST, 0, AI_SCORE_DEFAULT), AI_SCORE_DEFAULT);
        EXPECT(!(gAiBattleData->candidateRejectionFlags[battler][source][0]
              & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH));
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: planned Mega Sheer Force removes both confirmed Fake Out penalties")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_CAMERUPT) {
            Item(ITEM_CAMERUPTITE); Ability(ABILITY_MAGMA_ARMOR); Speed(200);
            Moves(MOVE_FAKE_OUT, MOVE_PROTECT);
        }
        OPPONENT(SPECIES_WHIMSICOTT) {
            Item(ITEM_FOCUS_SASH); Ability(ABILITY_PRANKSTER); Speed(184);
            Moves(MOVE_MOONBLAST, MOVE_PROTECT);
        }
    } WHEN {
        TURN {
            MOVE(player, MOVE_PROTECT);
            EXPECT_MOVES(opponent, MOVE_MOONBLAST, MOVE_PROTECT);
        }
    } THEN {
        enum BattlerId source = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);

        SetupConfirmedFakeOutBadMoveTest(source, battler);
        gBattleStruct->gimmick.toActivate &= ~(1u << source);
        SetAIUsingGimmick(battler, NO_GIMMICK);
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_EQ(gAiLogicData->abilities[source], ABILITY_MAGMA_ARMOR);
        EXPECT_EQ(Test_AI_CheckBadMove(battler, source, MOVE_MOONBLAST, 0, AI_SCORE_DEFAULT), 0);
        EXPECT(gAiBattleData->candidateRejectionFlags[battler][source][0]
             & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH);

        gBattleStruct->gimmick.usableGimmick[source] = GIMMICK_MEGA;
        gBattleStruct->gimmick.toActivate |= 1u << source;
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_EQ(GetAbilityBySpecies(SPECIES_CAMERUPT_MEGA, gBattleMons[source].abilityNum), ABILITY_SHEER_FORCE);
        EXPECT_EQ(Test_AI_CheckBadMove(battler, source, MOVE_MOONBLAST, 0, AI_SCORE_DEFAULT), AI_SCORE_DEFAULT);
        EXPECT(!(gAiBattleData->candidateRejectionFlags[battler][source][0]
              & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH));
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: Neutralizing Gas suppresses planned Mega Fake Out abilities")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        ASSUME(GetMoveType(MOVE_FAKE_OUT) == TYPE_NORMAL);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_LOPUNNY) {
            Item(ITEM_LOPUNNITE); Ability(ABILITY_LIMBER); Speed(200);
            Moves(MOVE_FAKE_OUT, MOVE_PROTECT);
        }
        PLAYER(SPECIES_WEEZING) { Ability(ABILITY_NEUTRALIZING_GAS); Speed(50); Moves(MOVE_PROTECT); }
        OPPONENT(SPECIES_GENGAR) {
            Item(ITEM_FOCUS_SASH); Ability(ABILITY_CURSED_BODY); Speed(184);
            Moves(MOVE_MOONBLAST, MOVE_PROTECT);
        }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_SHADOW_TAG); Speed(40); Moves(MOVE_PROTECT); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_PROTECT);
            MOVE(playerRight, MOVE_PROTECT);
            EXPECT_MOVES(opponentLeft, MOVE_MOONBLAST, MOVE_PROTECT);
            EXPECT_MOVE(opponentRight, MOVE_PROTECT);
        }
    } THEN {
        enum BattlerId source = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId gasSource = GetBattlerAtPosition(B_POSITION_PLAYER_RIGHT);
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);

        SetupConfirmedFakeOutBadMoveTest(source, battler);
        gBattleStruct->gimmick.usableGimmick[source] = GIMMICK_MEGA;
        gBattleStruct->gimmick.toActivate |= 1u << source;
        SetAIUsingGimmick(battler, NO_GIMMICK);
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT(gBattleMons[gasSource].volatiles.neutralizingGas);
        EXPECT_EQ(gAiLogicData->abilities[source], ABILITY_NONE);
        EXPECT_EQ(GetAbilityBySpecies(SPECIES_LOPUNNY_MEGA, gBattleMons[source].abilityNum), ABILITY_SCRAPPY);
        EXPECT_EQ(Test_AI_CheckBadMove(battler, source, MOVE_MOONBLAST, 0, AI_SCORE_DEFAULT), AI_SCORE_DEFAULT);
        EXPECT(!(gAiBattleData->candidateRejectionFlags[battler][source][0]
              & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH));

        gBattleMons[gasSource].volatiles.neutralizingGas = FALSE;
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_EQ(Test_AI_CheckBadMove(battler, source, MOVE_MOONBLAST, 0, AI_SCORE_DEFAULT), 0);
        EXPECT(gAiBattleData->candidateRejectionFlags[battler][source][0]
             & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH);
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: Ability Shield preserves a Fake Out target flinch immunity")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        ASSUME(GetItemHoldEffect(ITEM_ABILITY_SHIELD) == HOLD_EFFECT_ABILITY_SHIELD);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_PANGORO) {
            Ability(ABILITY_MOLD_BREAKER); Speed(200); Moves(MOVE_FAKE_OUT, MOVE_PROTECT);
        }
        OPPONENT(SPECIES_LUCARIO) {
            Item(ITEM_ABILITY_SHIELD); Ability(ABILITY_INNER_FOCUS); Speed(184);
            Moves(MOVE_MOONBLAST, MOVE_PROTECT);
        }
    } WHEN {
        TURN {
            MOVE(player, MOVE_PROTECT);
            EXPECT_MOVES(opponent, MOVE_MOONBLAST, MOVE_PROTECT);
        }
    } THEN {
        enum BattlerId source = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);

        SetupConfirmedFakeOutBadMoveTest(source, battler);
        SetAIUsingGimmick(battler, NO_GIMMICK);
        gAiLogicData->holdEffects[battler] = HOLD_EFFECT_NONE;
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_EQ(gAiLogicData->abilities[source], ABILITY_MOLD_BREAKER);
        EXPECT_EQ(gAiLogicData->abilities[battler], ABILITY_INNER_FOCUS);
        EXPECT_EQ(Test_AI_CheckBadMove(battler, source, MOVE_MOONBLAST, 0, AI_SCORE_DEFAULT), 0);
        EXPECT(gAiBattleData->candidateRejectionFlags[battler][source][0]
             & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH);

        gAiLogicData->holdEffects[battler] = HOLD_EFFECT_ABILITY_SHIELD;
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_EQ(Test_AI_CheckBadMove(battler, source, MOVE_MOONBLAST, 0, AI_SCORE_DEFAULT), AI_SCORE_DEFAULT);
        EXPECT(!(gAiBattleData->candidateRejectionFlags[battler][source][0]
              & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH));
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: Ability Shield preserves a partner priority blocker")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        ASSUME(GetItemHoldEffect(ITEM_ABILITY_SHIELD) == HOLD_EFFECT_ABILITY_SHIELD);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_PANGORO) {
            Ability(ABILITY_MOLD_BREAKER); Speed(200); Moves(MOVE_FAKE_OUT, MOVE_PROTECT);
        }
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_SHADOW_TAG); Speed(50); Moves(MOVE_PROTECT); }
        OPPONENT(SPECIES_WHIMSICOTT) {
            Item(ITEM_FOCUS_SASH); Ability(ABILITY_PRANKSTER); Speed(184); Moves(MOVE_MOONBLAST);
        }
        OPPONENT(SPECIES_TSAREENA) {
            Item(ITEM_ABILITY_SHIELD); Ability(ABILITY_QUEENLY_MAJESTY); Speed(100); Moves(MOVE_PROTECT);
        }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_PROTECT);
            MOVE(playerRight, MOVE_PROTECT);
            EXPECT_MOVE(opponentLeft, MOVE_MOONBLAST);
            EXPECT_MOVE(opponentRight, MOVE_PROTECT);
        }
    } THEN {
        enum BattlerId source = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId partner = BATTLE_PARTNER(battler);

        SetupConfirmedFakeOutBadMoveTest(source, battler);
        SetAIUsingGimmick(battler, NO_GIMMICK);
        gAiLogicData->holdEffects[partner] = HOLD_EFFECT_NONE;
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_EQ(gAiLogicData->abilities[source], ABILITY_MOLD_BREAKER);
        EXPECT_EQ(gAiLogicData->abilities[partner], ABILITY_QUEENLY_MAJESTY);
        EXPECT_EQ(Test_AI_CheckBadMove(battler, source, MOVE_MOONBLAST, 0, AI_SCORE_DEFAULT), 0);
        EXPECT(gAiBattleData->candidateRejectionFlags[battler][source][0]
             & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH);

        gAiLogicData->holdEffects[partner] = HOLD_EFFECT_ABILITY_SHIELD;
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_EQ(Test_AI_CheckBadMove(battler, source, MOVE_MOONBLAST, 0, AI_SCORE_DEFAULT), AI_SCORE_DEFAULT);
        EXPECT(!(gAiBattleData->candidateRejectionFlags[battler][source][0]
              & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH));
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: faster planned player Mega Fake Out gates a same-priority AI action")
{
    GIVEN {
        WITH_CONFIG(B_MEGA_EVO_TURN_ORDER, GEN_7);
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_LOPUNNY) {
            Level(50); Item(ITEM_LOPUNNITE); Ability(ABILITY_LIMBER); Nature(NATURE_JOLLY);
            TEST_IVS_PHYSICAL(); Moves(MOVE_FAKE_OUT, MOVE_PROTECT);
        }
        OPPONENT(SPECIES_WEAVILE) {
            Level(50); Item(ITEM_FOCUS_SASH); Ability(ABILITY_PRESSURE); Nature(NATURE_JOLLY);
            TEST_IVS_PHYSICAL(); Moves(MOVE_FAKE_OUT, MOVE_PROTECT);
        }
    } WHEN {
        TURN {
            MOVE(player, MOVE_PROTECT);
            EXPECT_MOVES(opponent, MOVE_FAKE_OUT, MOVE_PROTECT);
        }
    } THEN {
        enum BattlerId source = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);

        SetupConfirmedFakeOutBadMoveTest(source, battler);
        gBattleMons[source].speed = 137;
        gBattleMons[battler].speed = 150;
        gBattleStruct->battlerState[battler].isFirstTurn = 1;
        gBattleStruct->gimmick.toActivate &= ~(1u << source);
        SetAIUsingGimmick(battler, NO_GIMMICK);
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_LT(gBattleMons[source].speed, gBattleMons[battler].speed);
        EXPECT_GT(Test_AI_CheckBadMove(battler, source, MOVE_FAKE_OUT, 0, AI_SCORE_DEFAULT), 0);
        EXPECT(!(gAiBattleData->candidateRejectionFlags[battler][source][0]
              & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH));

        gBattleStruct->gimmick.usableGimmick[source] = GIMMICK_MEGA;
        gBattleStruct->gimmick.toActivate |= 1u << source;
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_EQ(GetConfig(B_MEGA_EVO_TURN_ORDER), GEN_7);
        EXPECT_GT(GetSpeciesBaseSpeed(SPECIES_LOPUNNY_MEGA), GetSpeciesBaseSpeed(SPECIES_WEAVILE));
        EXPECT_EQ(Test_AI_CheckBadMove(battler, source, MOVE_FAKE_OUT, 0, AI_SCORE_DEFAULT), 0);
        EXPECT(gAiBattleData->candidateRejectionFlags[battler][source][0]
             & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH);
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: Gen 6 order keeps planned player Mega speed out of the current turn")
{
    GIVEN {
        WITH_CONFIG(B_MEGA_EVO_TURN_ORDER, GEN_6);
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_LOPUNNY) {
            Level(50); Item(ITEM_LOPUNNITE); Ability(ABILITY_LIMBER); Nature(NATURE_JOLLY);
            TEST_IVS_PHYSICAL(); Moves(MOVE_FAKE_OUT, MOVE_PROTECT);
        }
        OPPONENT(SPECIES_WEAVILE) {
            Level(50); Item(ITEM_FOCUS_SASH); Ability(ABILITY_PRESSURE); Nature(NATURE_JOLLY);
            TEST_IVS_PHYSICAL(); Moves(MOVE_FAKE_OUT, MOVE_PROTECT);
        }
    } WHEN {
        TURN {
            MOVE(player, MOVE_PROTECT);
            EXPECT_MOVES(opponent, MOVE_FAKE_OUT, MOVE_PROTECT);
        }
    } THEN {
        enum BattlerId source = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);

        SetupConfirmedFakeOutBadMoveTest(source, battler);
        gBattleMons[source].speed = 137;
        gBattleMons[battler].speed = 150;
        gBattleStruct->battlerState[battler].isFirstTurn = 1;
        gBattleStruct->gimmick.usableGimmick[source] = GIMMICK_MEGA;
        gBattleStruct->gimmick.toActivate |= 1u << source;
        SetAIUsingGimmick(battler, NO_GIMMICK);
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_EQ(GetConfig(B_MEGA_EVO_TURN_ORDER), GEN_6);
        EXPECT_LT(gBattleMons[source].speed, gBattleMons[battler].speed);
        EXPECT_GT(GetSpeciesBaseSpeed(SPECIES_LOPUNNY_MEGA), GetSpeciesBaseSpeed(SPECIES_WEAVILE));
        EXPECT_GT(Test_AI_CheckBadMove(battler, source, MOVE_FAKE_OUT, 0, AI_SCORE_DEFAULT), 0);
        EXPECT(!(gAiBattleData->candidateRejectionFlags[battler][source][0]
              & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH));
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: faster selected AI Mega Fake Out escapes the same-priority hard gate")
{
    GIVEN {
        WITH_CONFIG(B_MEGA_EVO_TURN_ORDER, GEN_7);
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WEAVILE) {
            Level(50); Item(ITEM_FOCUS_SASH); Ability(ABILITY_PRESSURE); Nature(NATURE_JOLLY);
            TEST_IVS_PHYSICAL(); Moves(MOVE_FAKE_OUT, MOVE_PROTECT);
        }
        OPPONENT(SPECIES_LOPUNNY) {
            Level(50); Item(ITEM_FOCUS_SASH); Ability(ABILITY_LIMBER); Nature(NATURE_JOLLY);
            TEST_IVS_PHYSICAL(); Moves(MOVE_FAKE_OUT, MOVE_PROTECT);
        }
    } WHEN {
        TURN {
            MOVE(player, MOVE_PROTECT);
            EXPECT_MOVES(opponent, MOVE_FAKE_OUT, MOVE_PROTECT);
        }
    } THEN {
        enum BattlerId source = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);

        gBattleMons[battler].item = ITEM_LOPUNNITE;
        SetupConfirmedFakeOutBadMoveTest(source, battler);
        gBattleMons[source].speed = 140;
        gBattleMons[battler].speed = 137;
        gBattleStruct->battlerState[battler].isFirstTurn = 1;
        SetActiveGimmick(battler, GIMMICK_NONE);
        gBattleStruct->gimmick.toActivate &= ~(1u << battler);
        SetAIUsingGimmick(battler, NO_GIMMICK);
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_LT(gBattleMons[battler].speed, gBattleMons[source].speed);
        EXPECT_EQ(Test_AI_CheckBadMove(battler, source, MOVE_FAKE_OUT, 0, AI_SCORE_DEFAULT), 0);
        EXPECT(gAiBattleData->candidateRejectionFlags[battler][source][0]
             & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH);

        gBattleStruct->gimmick.usableGimmick[battler] = GIMMICK_MEGA;
        gBattleStruct->gimmick.toActivate |= 1u << battler;
        SetAIUsingGimmick(battler, USE_GIMMICK);
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_GT(GetSpeciesBaseSpeed(SPECIES_LOPUNNY_MEGA), GetSpeciesBaseSpeed(SPECIES_WEAVILE));
        EXPECT_GT(Test_AI_CheckBadMove(battler, source, MOVE_FAKE_OUT, 0, AI_SCORE_DEFAULT), 0);
        EXPECT(!(gAiBattleData->candidateRejectionFlags[battler][source][0]
              & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH));
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: faster repeated Quick Guard prevents both confirmed Fake Out penalties")
{
    GIVEN {
        WITH_CONFIG(B_QUICK_GUARD, GEN_6);
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        ASSUME(GetMoveProtectMethod(MOVE_QUICK_GUARD) == PROTECT_QUICK_GUARD);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_INCINEROAR) { Ability(ABILITY_INTIMIDATE); Speed(150); Moves(MOVE_FAKE_OUT, MOVE_PROTECT); }
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_SHADOW_TAG); Speed(50); Moves(MOVE_PROTECT); }
        OPPONENT(SPECIES_WHIMSICOTT) { Ability(ABILITY_PRANKSTER); Speed(80); Moves(MOVE_MOONBLAST); }
        OPPONENT(SPECIES_GALLADE) { Ability(ABILITY_STEADFAST); Speed(200); Moves(MOVE_QUICK_GUARD); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_PROTECT);
            MOVE(playerRight, MOVE_PROTECT);
            EXPECT_MOVE(opponentLeft, MOVE_MOONBLAST);
            EXPECT_MOVE(opponentRight, MOVE_QUICK_GUARD);
        }
    } THEN {
        enum BattlerId source = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId partner = BATTLE_PARTNER(battler);

        SetupConfirmedFakeOutBadMoveTest(source, battler);
        gAiLogicData->partnerMove = MOVE_QUICK_GUARD;
        gBattleMons[partner].volatiles.consecutiveMoveUses = 1;
        SetAIUsingGimmick(partner, NO_GIMMICK);
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_GT(gBattleMons[partner].speed, gBattleMons[source].speed);
        EXPECT_EQ(Test_AI_CheckBadMove(battler, source, MOVE_MOONBLAST, 0, AI_SCORE_DEFAULT), AI_SCORE_DEFAULT);
        EXPECT(!(gAiBattleData->candidateRejectionFlags[battler][source][0]
              & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH));

        gBattleStruct->gimmick.usableGimmick[partner] = GIMMICK_DYNAMAX;
        SetAIUsingGimmick(partner, USE_GIMMICK);
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_EQ(Test_AI_CheckBadMove(battler, source, MOVE_MOONBLAST, 0, AI_SCORE_DEFAULT), 0);
        EXPECT(gAiBattleData->candidateRejectionFlags[battler][source][0]
             & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH);
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: slower Quick Guard cannot excuse a confirmed Fake Out hard gate")
{
    GIVEN {
        WITH_CONFIG(B_QUICK_GUARD, GEN_6);
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        ASSUME(GetMoveProtectMethod(MOVE_QUICK_GUARD) == PROTECT_QUICK_GUARD);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_INCINEROAR) { Ability(ABILITY_INTIMIDATE); Speed(150); Moves(MOVE_FAKE_OUT, MOVE_PROTECT); }
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_SHADOW_TAG); Speed(50); Moves(MOVE_PROTECT); }
        OPPONENT(SPECIES_WHIMSICOTT) { Ability(ABILITY_PRANKSTER); Speed(80); Moves(MOVE_MOONBLAST); }
        OPPONENT(SPECIES_GALLADE) { Ability(ABILITY_STEADFAST); Speed(100); Moves(MOVE_QUICK_GUARD); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_PROTECT);
            MOVE(playerRight, MOVE_PROTECT);
            EXPECT_MOVE(opponentLeft, MOVE_MOONBLAST);
            EXPECT_MOVE(opponentRight, MOVE_QUICK_GUARD);
        }
    } THEN {
        enum BattlerId source = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId partner = BATTLE_PARTNER(battler);

        SetupConfirmedFakeOutBadMoveTest(source, battler);
        gAiLogicData->partnerMove = MOVE_QUICK_GUARD;
        gBattleMons[partner].volatiles.consecutiveMoveUses = 1;
        SetAIUsingGimmick(partner, NO_GIMMICK);
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_LT(gBattleMons[partner].speed, gBattleMons[source].speed);
        EXPECT_EQ(Test_AI_CheckBadMove(battler, source, MOVE_MOONBLAST, 0, AI_SCORE_DEFAULT), 0);
        EXPECT(gAiBattleData->candidateRejectionFlags[battler][source][0]
             & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH);
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: speed-tied Quick Guard is not a guaranteed Fake Out answer")
{
    GIVEN {
        WITH_CONFIG(B_QUICK_GUARD, GEN_6);
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        ASSUME(GetMoveProtectMethod(MOVE_QUICK_GUARD) == PROTECT_QUICK_GUARD);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_INCINEROAR) { Ability(ABILITY_INTIMIDATE); Speed(150); Moves(MOVE_FAKE_OUT, MOVE_PROTECT); }
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_SHADOW_TAG); Speed(50); Moves(MOVE_PROTECT); }
        OPPONENT(SPECIES_WHIMSICOTT) { Ability(ABILITY_PRANKSTER); Speed(80); Moves(MOVE_MOONBLAST); }
        OPPONENT(SPECIES_GALLADE) { Ability(ABILITY_STEADFAST); Speed(150); Moves(MOVE_QUICK_GUARD); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_PROTECT);
            MOVE(playerRight, MOVE_PROTECT);
            EXPECT_MOVE(opponentLeft, MOVE_MOONBLAST);
            EXPECT_MOVE(opponentRight, MOVE_QUICK_GUARD);
        }
    } THEN {
        enum BattlerId source = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId partner = BATTLE_PARTNER(battler);

        SetupConfirmedFakeOutBadMoveTest(source, battler);
        gAiLogicData->partnerMove = MOVE_QUICK_GUARD;
        SetAIUsingGimmick(partner, NO_GIMMICK);
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_EQ(gBattleMons[partner].speed, gBattleMons[source].speed);
        EXPECT_EQ(Test_AI_CheckBadMove(battler, source, MOVE_MOONBLAST, 0, AI_SCORE_DEFAULT), 0);
        EXPECT(gAiBattleData->candidateRejectionFlags[battler][source][0]
             & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH);
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: faster Quick Guard wins within the shared moves-last bracket")
{
    GIVEN {
        WITH_CONFIG(B_QUICK_GUARD, GEN_6);
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        ASSUME(GetMoveProtectMethod(MOVE_QUICK_GUARD) == PROTECT_QUICK_GUARD);
        ASSUME(GetItemHoldEffect(ITEM_LAGGING_TAIL) == HOLD_EFFECT_LAGGING_TAIL);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_SABLEYE) { Ability(ABILITY_STALL); Speed(150); Moves(MOVE_FAKE_OUT, MOVE_PROTECT); }
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_SHADOW_TAG); Speed(50); Moves(MOVE_PROTECT); }
        OPPONENT(SPECIES_WHIMSICOTT) { Ability(ABILITY_PRANKSTER); Speed(80); Moves(MOVE_MOONBLAST); }
        OPPONENT(SPECIES_GALLADE) {
            Item(ITEM_LAGGING_TAIL); Ability(ABILITY_STEADFAST); Speed(200); Moves(MOVE_QUICK_GUARD);
        }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_PROTECT);
            MOVE(playerRight, MOVE_PROTECT);
            EXPECT_MOVE(opponentLeft, MOVE_MOONBLAST);
            EXPECT_MOVE(opponentRight, MOVE_QUICK_GUARD);
        }
    } THEN {
        enum BattlerId source = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId partner = BATTLE_PARTNER(battler);

        SetupConfirmedFakeOutBadMoveTest(source, battler);
        gAiLogicData->partnerMove = MOVE_QUICK_GUARD;
        SetAIUsingGimmick(partner, NO_GIMMICK);
        gAiBattleData->candidateRejectionFlags[battler][source][0] = AI_CANDIDATE_REJECTION_NONE;

        EXPECT_EQ(gAiLogicData->abilities[source], ABILITY_STALL);
        EXPECT_EQ(gAiLogicData->holdEffects[partner], HOLD_EFFECT_LAGGING_TAIL);
        EXPECT_GT(gBattleMons[partner].speed, gBattleMons[source].speed);
        EXPECT_EQ(Test_AI_CheckBadMove(battler, source, MOVE_MOONBLAST, 0, AI_SCORE_DEFAULT), AI_SCORE_DEFAULT);
        EXPECT(!(gAiBattleData->candidateRejectionFlags[battler][source][0]
              & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH));
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: Joint runtime composes faster Quick Guard with an action behind confirmed Fake Out")
{
    GIVEN {
        WITH_CONFIG(B_QUICK_GUARD, GEN_6);
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        ASSUME(GetMoveProtectMethod(MOVE_QUICK_GUARD) == PROTECT_QUICK_GUARD);
        AI_FLAGS(AI_FLAG_SMART_TRAINER | AI_FLAG_PREDICTION | AI_FLAG_SMART_GIMMICK | AI_FLAG_SMART_SWITCHING
               | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT | AI_FLAG_KNOW_OPPONENT_PARTY
               | AI_FLAG_POWERFUL_STATUS | AI_FLAG_AGGRESSIVE_GIMMICK | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WOBBUFFET) {
            Ability(ABILITY_GUTS); MaxHP(200); HP(200); Attack(1); Defense(100); SpDefense(100); Speed(150);
            Moves(MOVE_FAKE_OUT, MOVE_PROTECT);
        }
        PLAYER(SPECIES_WOBBUFFET) {
            Item(ITEM_POWER_HERB); Ability(ABILITY_GUTS); MaxHP(200); HP(200); Speed(40);
            Moves(MOVE_GEOMANCY);
        }
        // Opponent-left is evaluated before its higher-id partner.  With no
        // previous turn, legacy GetAllyChosenMove therefore exposes
        // lastUsedMove == MOVE_NONE rather than the later Quick Guard choice.
        OPPONENT(SPECIES_XERNEAS) {
            Item(ITEM_POWER_HERB); Ability(ABILITY_GUTS); MaxHP(223); HP(1); Defense(115); SpDefense(118); Speed(100);
            Moves(MOVE_GEOMANCY);
        }
        OPPONENT(SPECIES_WOBBUFFET) {
            Ability(ABILITY_GUTS); MaxHP(200); HP(200); Speed(200);
            Moves(MOVE_QUICK_GUARD);
        }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_FAKE_OUT, target: opponentLeft);
            MOVE(playerRight, MOVE_GEOMANCY);
            EXPECT_MOVE(opponentLeft, MOVE_GEOMANCY);
            EXPECT_MOVE(opponentRight, MOVE_QUICK_GUARD);
        }
    } THEN {
        enum BattlerId victim = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId guard = GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT);
        const struct BattleActionLogEntry *victimAction =
            BattleActionLog_GetLastEntry(victim, 1u << B_ACTION_USE_MOVE);
        const struct BattleActionLogEntry *guardAction =
            BattleActionLog_GetLastEntry(guard, 1u << B_ACTION_USE_MOVE);
        const struct BattleAiTracePlan *plan = NULL;
        const struct BattleAiTraceCandidate *chosen = NULL;
        bool32 foundComposedPair = FALSE;

        EXPECT(victimAction != NULL);
        EXPECT(guardAction != NULL);
        EXPECT_EQ(Test_BattleAiJointRuntime_GetBuildCount(), 1);
        EXPECT_GT(Test_BattleAiJointRuntime_GetStepCount(), 0);
        EXPECT_NE(Test_BattleAiJointRuntime_GetPlanId(), BATTLE_AI_TRACE_ID_NONE);
        if (victimAction != NULL && guardAction != NULL)
        {
            EXPECT_NE(victimAction->aiPlanId, BATTLE_AI_TRACE_ID_NONE);
            EXPECT_EQ(victimAction->aiPlanId, guardAction->aiPlanId);
            EXPECT_EQ(victimAction->aiCandidateRank, guardAction->aiCandidateRank);
            EXPECT_EQ(victimAction->aiPlanId, Test_BattleAiJointRuntime_GetPlanId());
            plan = BattleAiTrace_GetPlan(victimAction->aiPlanId);
        }
        EXPECT(plan != NULL);
        if (plan != NULL)
        {
            EXPECT(plan->flags & BATTLE_AI_TRACE_PLAN_JOINT);
            EXPECT(!(plan->flags & BATTLE_AI_TRACE_PLAN_LEGACY_EVALUATOR));
            EXPECT_GE(plan->completedDepth, AI_JOINT_STANDARD_DEPTH);
            EXPECT(plan->chosenRank < plan->candidateCount);
            chosen = BattleAiTrace_GetCandidate(plan->firstCandidateSequence + plan->chosenRank);
        }
        EXPECT(chosen != NULL);
        if (chosen != NULL)
            EXPECT(chosen->flags & BATTLE_AI_TRACE_CANDIDATE_CHOSEN);
        for (u32 rank = 0; plan != NULL && rank < plan->candidateCount; rank++)
        {
            const struct BattleAiTraceCandidate *candidate =
                BattleAiTrace_GetCandidate(plan->firstCandidateSequence + rank);
            bool32 hasVictim = FALSE;
            bool32 hasGuard = FALSE;
            u32 victimActionIndex = ARRAY_COUNT(candidate->actions);

            if (candidate == NULL)
                continue;
            for (u32 actionIndex = 0; actionIndex < ARRAY_COUNT(candidate->actions); actionIndex++)
            {
                const struct BattleAiTraceAction *action = &candidate->actions[actionIndex];
                enum BattlerId actor;

                if (!(action->actorMeta & BATTLE_AI_TRACE_ACTION_VALID))
                    continue;
                actor = action->actorMeta & BATTLE_AI_TRACE_ACTION_ACTOR_MASK;
                if (actor == victim && action->choice == MOVE_GEOMANCY)
                {
                    hasVictim = TRUE;
                    victimActionIndex = actionIndex;
                }
                else if (actor == guard && action->choice == MOVE_QUICK_GUARD)
                    hasGuard = TRUE;
            }
            if (hasVictim && hasGuard)
            {
                foundComposedPair = TRUE;
                EXPECT(candidate->flags & BATTLE_AI_TRACE_CANDIDATE_JOINT);
                EXPECT(!(candidate->flags & BATTLE_AI_TRACE_CANDIDATE_FORCED_TRACE));
                EXPECT_GE(candidate->completedDepth, 1);
                EXPECT(victimActionIndex < ARRAY_COUNT(candidate->actions));
                if (victimActionIndex < ARRAY_COUNT(candidate->actions))
                {
                    EXPECT(candidate->readInteractionFlags[victimActionIndex]
                           & AI_READ_INTERACTION_FAKE_OUT);
                    EXPECT(!(candidate->rejectionFlags[victimActionIndex]
                           & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH));
                }
            }
        }
        EXPECT(foundComposedPair);
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: Joint runtime composes faster partner Fake Out before the confirmed player Fake Out")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        AI_FLAGS(AI_FLAG_SMART_TRAINER | AI_FLAG_PREDICTION | AI_FLAG_SMART_GIMMICK | AI_FLAG_SMART_SWITCHING
               | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT | AI_FLAG_KNOW_OPPONENT_PARTY
               | AI_FLAG_POWERFUL_STATUS | AI_FLAG_AGGRESSIVE_GIMMICK | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WOBBUFFET) {
            Ability(ABILITY_GUTS); MaxHP(200); HP(1); Attack(1); Defense(100); SpDefense(100); Speed(150);
            Moves(MOVE_FAKE_OUT, MOVE_PROTECT);
        }
        // Ghost typing leaves only the selected Fake Out user as a useful
        // target for the AI partner's own Fake Out.
        PLAYER(SPECIES_GENGAR) {
            Item(ITEM_POWER_HERB); Ability(ABILITY_GUTS); MaxHP(160); HP(160); Speed(40);
            Moves(MOVE_GEOMANCY);
        }
        OPPONENT(SPECIES_XERNEAS) {
            Item(ITEM_POWER_HERB); Ability(ABILITY_GUTS); MaxHP(223); HP(1); Defense(115); SpDefense(118); Speed(100);
            Moves(MOVE_GEOMANCY);
        }
        OPPONENT(SPECIES_WOBBUFFET) {
            Ability(ABILITY_GUTS); MaxHP(200); HP(200); Attack(100); Speed(200);
            Moves(MOVE_FAKE_OUT, MOVE_PROTECT);
        }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_FAKE_OUT, target: opponentLeft);
            MOVE(playerRight, MOVE_GEOMANCY);
            EXPECT_MOVE(opponentLeft, MOVE_GEOMANCY);
            EXPECT_MOVES(opponentRight, MOVE_FAKE_OUT, MOVE_PROTECT);
        }
    } THEN {
        enum BattlerId victim = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId stopper = GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT);
        const struct BattleActionLogEntry *victimAction =
            BattleActionLog_GetLastEntry(victim, 1u << B_ACTION_USE_MOVE);
        const struct BattleActionLogEntry *stopperAction =
            BattleActionLog_GetLastEntry(stopper, 1u << B_ACTION_USE_MOVE);
        const struct BattleAiTracePlan *plan = NULL;
        const struct BattleAiTraceCandidate *chosen = NULL;
        bool32 foundComposedPair = FALSE;
        enum BattlerId playerFakeOutUser = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);

        EXPECT(victimAction != NULL);
        EXPECT(stopperAction != NULL);
        EXPECT_EQ(Test_BattleAiJointRuntime_GetBuildCount(), 1);
        EXPECT_GT(Test_BattleAiJointRuntime_GetStepCount(), 0);
        if (victimAction != NULL && stopperAction != NULL)
        {
            EXPECT_NE(victimAction->aiPlanId, BATTLE_AI_TRACE_ID_NONE);
            EXPECT_EQ(victimAction->aiPlanId, stopperAction->aiPlanId);
            EXPECT_EQ(victimAction->aiCandidateRank, stopperAction->aiCandidateRank);
            EXPECT_EQ(victimAction->aiPlanId, Test_BattleAiJointRuntime_GetPlanId());
            plan = BattleAiTrace_GetPlan(victimAction->aiPlanId);
        }
        EXPECT(plan != NULL);
        if (plan != NULL)
        {
            EXPECT(plan->flags & BATTLE_AI_TRACE_PLAN_JOINT);
            EXPECT(!(plan->flags & BATTLE_AI_TRACE_PLAN_LEGACY_EVALUATOR));
            EXPECT_GE(plan->completedDepth, AI_JOINT_STANDARD_DEPTH);
            EXPECT(plan->chosenRank < plan->candidateCount);
            chosen = BattleAiTrace_GetCandidate(plan->firstCandidateSequence + plan->chosenRank);
        }
        EXPECT(chosen != NULL);
        if (chosen != NULL)
            EXPECT(chosen->flags & BATTLE_AI_TRACE_CANDIDATE_CHOSEN);
        for (u32 rank = 0; plan != NULL && rank < plan->candidateCount; rank++)
        {
            const struct BattleAiTraceCandidate *candidate =
                BattleAiTrace_GetCandidate(plan->firstCandidateSequence + rank);
            bool32 hasVictim = FALSE;
            bool32 hasStopper = FALSE;
            u32 victimActionIndex = ARRAY_COUNT(candidate->actions);

            if (candidate == NULL)
                continue;
            for (u32 actionIndex = 0; actionIndex < ARRAY_COUNT(candidate->actions); actionIndex++)
            {
                const struct BattleAiTraceAction *action = &candidate->actions[actionIndex];
                enum BattlerId actor;
                enum BattlerId target;

                if (!(action->actorMeta & BATTLE_AI_TRACE_ACTION_VALID))
                    continue;
                actor = action->actorMeta & BATTLE_AI_TRACE_ACTION_ACTOR_MASK;
                target = action->targetMeta & BATTLE_AI_TRACE_ACTION_TARGET_MASK;
                if (actor == victim && action->choice == MOVE_GEOMANCY)
                {
                    hasVictim = TRUE;
                    victimActionIndex = actionIndex;
                }
                else if (actor == stopper
                      && action->choice == MOVE_FAKE_OUT
                      && target == playerFakeOutUser)
                    hasStopper = TRUE;
            }
            if (hasVictim && hasStopper)
            {
                foundComposedPair = TRUE;
                EXPECT(candidate->flags & BATTLE_AI_TRACE_CANDIDATE_JOINT);
                EXPECT(!(candidate->flags & BATTLE_AI_TRACE_CANDIDATE_FORCED_TRACE));
                EXPECT_GE(candidate->completedDepth, 1);
                EXPECT(victimActionIndex < ARRAY_COUNT(candidate->actions));
                if (victimActionIndex < ARRAY_COUNT(candidate->actions))
                {
                    EXPECT(candidate->readInteractionFlags[victimActionIndex]
                           & AI_READ_INTERACTION_FAKE_OUT);
                    EXPECT(!(candidate->rejectionFlags[victimActionIndex]
                           & AI_CANDIDATE_REJECTION_CONFIRMED_FLINCH));
                }
            }
        }
        EXPECT(foundComposedPair);
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: Joint runtime composed simulator still skips an unguarded confirmed Fake Out victim")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); MaxHP(200); HP(200); Speed(150); Moves(MOVE_PROTECT); }
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); MaxHP(200); HP(200); Speed(40); Moves(MOVE_PROTECT); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); MaxHP(200); HP(200); Speed(50); Moves(MOVE_PROTECT); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); MaxHP(200); HP(200); Speed(30); Moves(MOVE_PROTECT); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_PROTECT);
            MOVE(playerRight, MOVE_PROTECT);
            EXPECT_MOVE(opponentLeft, MOVE_PROTECT);
            EXPECT_MOVE(opponentRight, MOVE_PROTECT);
        }
    } THEN {
        enum BattlerId source = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId victim = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        struct AiSmartGimmickSimFixture *fixture = GetSmartGimmickSimFixture();
        enum AiSimApplyStatus applyStatus;
        u32 sourceRoster;
        u32 victimRoster;

        EXPECT(fixture != NULL);
        if (fixture == NULL)
            return;
        memset(fixture, 0, sizeof(*fixture));
        SetAiLogicDataForTurn(gAiLogicData);
        EXPECT(AiSim_CaptureKnownBoard(victim, gAiLogicData,
                                      &fixture->context, &fixture->before));
        sourceRoster = fixture->before.active[source].rosterIndex;
        victimRoster = fixture->before.active[victim].rosterIndex;
        EXPECT_LT(sourceRoster, AI_SIM_ROSTER_COUNT);
        EXPECT_LT(victimRoster, AI_SIM_ROSTER_COUNT);
        if (sourceRoster >= AI_SIM_ROSTER_COUNT || victimRoster >= AI_SIM_ROSTER_COUNT)
        {
            FreeSmartGimmickSimFixture();
            return;
        }

        fixture->context.mons[sourceRoster].moves[0] = MOVE_FAKE_OUT;
        fixture->context.mons[victimRoster].moves[0] = MOVE_GEOMANCY;
        // The preceding live Protect turn may leave command-cycle bookkeeping
        // in gBattleMons that capture correctly reports as an unsupported
        // volatile.  This is a synthetic clean board: the represented active
        // states are reset below, so remove only that capture-boundary marker.
        fixture->context.flags &= ~AI_SIM_CONTEXT_VOLATILE_STATE;
        fixture->before.party[sourceRoster].hp = fixture->context.mons[sourceRoster].normal.maxHp;
        fixture->before.party[sourceRoster].pp[0] = 10;
        fixture->before.party[sourceRoster].status1 = 0;
        fixture->before.party[victimRoster].hp = fixture->context.mons[victimRoster].normal.maxHp;
        fixture->before.party[victimRoster].item = ITEM_POWER_HERB;
        fixture->before.party[victimRoster].pp[0] = 10;
        fixture->before.party[victimRoster].status1 = 0;
        fixture->before.active[source].firstTurn = TRUE;
        fixture->before.active[source].consecutiveMoveUses = 0;
        fixture->before.active[source].chargingMove = MOVE_NONE;
        fixture->before.active[source].volatileFlags = 0;
        fixture->before.active[victim].consecutiveMoveUses = 0;
        fixture->before.active[victim].chargingMove = MOVE_NONE;
        fixture->before.active[victim].volatileFlags = 0;
        for (u32 stat = 0; stat < NUM_BATTLE_STATS; stat++)
            fixture->before.active[victim].statStages[stat] = DEFAULT_STAT_STAGE;

        fixture->turn.actions[source] = (struct AiSimAction) {
            .actor = source,
            .target = victim,
            .kind = AI_SIM_ACTION_MOVE,
            .choice = MOVE_FAKE_OUT,
            .moveSlot = 0,
        };
        fixture->turn.actions[victim] = (struct AiSimAction) {
            .actor = victim,
            .target = victim,
            .kind = AI_SIM_ACTION_MOVE,
            .choice = MOVE_GEOMANCY,
            .moveSlot = 0,
        };
        fixture->turn.actionMask = (1u << source) | (1u << victim);
        fixture->turn.confirmedPlayerMask = 1u << source;
        fixture->turn.flags = AI_SIM_JOINT_CONSERVATIVE_ENEMY_TIES;

        applyStatus = AiSim_ApplyJointTurn(&fixture->context, &fixture->before,
                                          &fixture->turn, NULL,
                                          &fixture->after, &fixture->result);
        EXPECT_EQ(fixture->result.unsupportedFlags, AI_SIM_UNSUPPORTED_NONE);
        EXPECT_EQ(applyStatus, AI_SIM_APPLY_OK);
        EXPECT(fixture->result.events & AI_SIM_EVENT_FLINCH);
        EXPECT(fixture->result.executedMask & (1u << source));
        EXPECT(fixture->result.skippedMask & (1u << victim));
        EXPECT(!(fixture->result.executedMask & (1u << victim)));
        EXPECT(fixture->result.readInteractionFlags[victim] & AI_READ_INTERACTION_FAKE_OUT);
        EXPECT_EQ(fixture->after.party[victimRoster].item, ITEM_POWER_HERB);
        EXPECT_EQ(fixture->after.party[victimRoster].pp[0], fixture->before.party[victimRoster].pp[0]);
        EXPECT_EQ(fixture->after.active[victim].statStages[STAT_SPATK], DEFAULT_STAT_STAGE);
        FreeSmartGimmickSimFixture();
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: T6 ordinary Moonblast is never an eligible direct ally target")
{
    GIVEN {
        ASSUME(GetMoveTarget(MOVE_MOONBLAST) == TARGET_SELECTED);
        AI_FLAGS(AI_FLAG_SMART_TRAINER | AI_FLAG_PREDICTION | AI_FLAG_SMART_GIMMICK | AI_FLAG_SMART_SWITCHING
               | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT | AI_FLAG_KNOW_OPPONENT_PARTY
               | AI_FLAG_POWERFUL_STATUS | AI_FLAG_AGGRESSIVE_GIMMICK | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        PLAYER(SPECIES_MEWTWO) {
            Level(50); Item(ITEM_LIFE_ORB); Nature(NATURE_TIMID); TEST_IVS_SPECIAL();
            MaxHP(181); HP(181); Defense(110); SpAttack(206); SpDefense(110); Speed(200);
            Moves(MOVE_PSYSTRIKE, MOVE_EXPANDING_FORCE, MOVE_ICE_BEAM, MOVE_PROTECT);
        }
        OPPONENT(SPECIES_XERNEAS) {
            Level(50); Item(ITEM_POWER_HERB); Nature(NATURE_MODEST); TEST_IVS_SPECIAL();
            MaxHP(201); HP(201); Defense(115); SpAttack(183); SpDefense(118); Speed(119);
            Moves(MOVE_GEOMANCY, MOVE_MOONBLAST, MOVE_DAZZLING_GLEAM, MOVE_PROTECT);
        }
        OPPONENT(SPECIES_MEWTWO) {
            Level(50); Item(ITEM_LIFE_ORB); Nature(NATURE_TIMID); TEST_IVS_SPECIAL();
            MaxHP(181); HP(181); Defense(110); SpAttack(206); SpDefense(110); Speed(200);
            Moves(MOVE_PSYSTRIKE, MOVE_EXPANDING_FORCE, MOVE_ICE_BEAM, MOVE_PROTECT);
        }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_KNOCK_OFF, target: opponentRight);
            MOVE(playerRight, MOVE_EXPANDING_FORCE, target: opponentLeft);
            SCORE_EQ_VAL(opponentLeft, MOVE_MOONBLAST, 0, target: opponentRight);
        }
    } THEN {
        enum BattlerId opponentLeftBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId opponentRightBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT);
        const struct BattleActionLogEntry *entry = BattleActionLog_GetLastEntry(opponentLeftBattler, 1u << B_ACTION_USE_MOVE);
        const struct BattleAiTracePlan *plan;
        bool32 foundRejectedAllyTarget = FALSE;

        EXPECT(Test_ApplyBattleModeAiFlags(AI_FLAG_BASIC_TRAINER) & AI_FLAG_DOUBLE_BATTLE);
        EXPECT_EQ(Test_ApplyBattleModeAiFlags(0), 0);
        EXPECT(entry != NULL);
        EXPECT_NE(entry->target, opponentRightBattler);
        EXPECT_NE(entry->aiReason, AI_DECISION_REASON_KNOWN_COMMAND_ANSWER);
        EXPECT_NE(entry->aiPlanId, BATTLE_AI_TRACE_ID_NONE);
        plan = BattleAiTrace_GetPlan(entry->aiPlanId);
        EXPECT(plan != NULL);
        for (u32 rank = 0; plan != NULL && rank < plan->candidateCount; rank++)
        {
            const struct BattleAiTraceCandidate *candidate = BattleAiTrace_GetCandidate(plan->firstCandidateSequence + rank);

            if (candidate != NULL && candidate->rejectionFlags[0] & AI_CANDIDATE_REJECTION_UNAPPROVED_ALLY_TARGET)
                foundRejectedAllyTarget = TRUE;
        }
        EXPECT(foundRejectedAllyTarget);
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: T6 joint runtime rejects ordinary ally Moonblast while preserving a legal pair")
{
    GIVEN {
        ASSUME(GetMoveTarget(MOVE_MOONBLAST) == TARGET_SELECTED);
        AI_FLAGS(AI_FLAG_SMART_TRAINER | AI_FLAG_PREDICTION | AI_FLAG_SMART_GIMMICK | AI_FLAG_SMART_SWITCHING
               | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT | AI_FLAG_KNOW_OPPONENT_PARTY
               | AI_FLAG_POWERFUL_STATUS | AI_FLAG_AGGRESSIVE_GIMMICK | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { HP(1); Item(ITEM_POWER_HERB); Ability(ABILITY_GUTS); Speed(180); Moves(MOVE_GEOMANCY); }
        PLAYER(SPECIES_WOBBUFFET) { HP(1); Item(ITEM_POWER_HERB); Ability(ABILITY_GUTS); Speed(160); Moves(MOVE_GEOMANCY); }
        OPPONENT(SPECIES_XERNEAS) {
            Item(ITEM_NONE); Ability(ABILITY_GUTS); Speed(200); Moves(MOVE_MOONBLAST);
        }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_POWER_HERB); Ability(ABILITY_GUTS); Speed(40); Moves(MOVE_GEOMANCY); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_GEOMANCY);
            MOVE(playerRight, MOVE_GEOMANCY);
            EXPECT_MOVE(opponentLeft, MOVE_MOONBLAST);
            EXPECT_MOVE(opponentRight, MOVE_GEOMANCY);
        }
    } THEN {
        enum BattlerId opponentLeftBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId opponentRightBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT);
        const struct BattleActionLogEntry *leftAction =
            BattleActionLog_GetLastEntry(opponentLeftBattler, 1u << B_ACTION_USE_MOVE);
        const struct BattleActionLogEntry *rightAction =
            BattleActionLog_GetLastEntry(opponentRightBattler, 1u << B_ACTION_USE_MOVE);
        const struct BattleAiTracePlan *plan = NULL;
        const struct BattleAiTraceCandidate *chosen = NULL;
        bool32 foundChosenXerneasAction = FALSE;
        bool32 foundRejectedMoonblastAlly = FALSE;

        EXPECT(leftAction != NULL);
        EXPECT(rightAction != NULL);
        EXPECT_EQ(Test_BattleAiJointRuntime_GetBuildCount(), 1);
        EXPECT(Test_BattleAiJointRuntime_GetStepCount() > 0);
        if (leftAction != NULL && rightAction != NULL)
        {
            EXPECT_NE(leftAction->aiPlanId, BATTLE_AI_TRACE_ID_NONE);
            EXPECT_EQ(leftAction->aiPlanId, rightAction->aiPlanId);
            EXPECT_EQ(leftAction->aiCandidateRank, rightAction->aiCandidateRank);
            EXPECT_EQ(Test_BattleAiJointRuntime_GetPlanId(), leftAction->aiPlanId);
            if (leftAction->move == MOVE_MOONBLAST)
                EXPECT_NE(leftAction->target, opponentRightBattler);

            plan = BattleAiTrace_GetPlan(leftAction->aiPlanId);
        }
        EXPECT(plan != NULL);
        if (plan != NULL)
        {
            EXPECT(plan->flags & BATTLE_AI_TRACE_PLAN_VALID);
            EXPECT(plan->flags & BATTLE_AI_TRACE_PLAN_JOINT);
            EXPECT(!(plan->flags & BATTLE_AI_TRACE_PLAN_LEGACY_EVALUATOR));
            EXPECT_EQ(plan->actorMask,
                      (1u << opponentLeftBattler) | (1u << opponentRightBattler));
            EXPECT(plan->chosenRank < plan->candidateCount);
            if (leftAction != NULL)
                EXPECT_EQ(plan->chosenRank, leftAction->aiCandidateRank);

            chosen = BattleAiTrace_GetCandidate(plan->firstCandidateSequence + plan->chosenRank);
            EXPECT(chosen != NULL);
            if (chosen != NULL)
            {
                EXPECT(chosen->flags & BATTLE_AI_TRACE_CANDIDATE_CHOSEN);
                for (u32 actionIndex = 0; actionIndex < ARRAY_COUNT(chosen->actions); actionIndex++)
                {
                    const struct BattleAiTraceAction *action = &chosen->actions[actionIndex];
                    enum BattlerId actor;
                    enum BattlerId target;

                    if (!(action->actorMeta & BATTLE_AI_TRACE_ACTION_VALID))
                        continue;
                    actor = action->actorMeta & BATTLE_AI_TRACE_ACTION_ACTOR_MASK;
                    target = action->targetMeta & BATTLE_AI_TRACE_ACTION_TARGET_MASK;
                    EXPECT(!(chosen->rejectionFlags[actionIndex]
                           & AI_CANDIDATE_REJECTION_UNAPPROVED_ALLY_TARGET));
                    if (target == BATTLE_PARTNER(actor))
                        EXPECT_NE(chosen->allyInteractionKinds[actionIndex], AI_ALLY_INTERACTION_NONE);
                    if (actor == opponentLeftBattler)
                    {
                        foundChosenXerneasAction = TRUE;
                        if (leftAction != NULL)
                        {
                            EXPECT_EQ(action->choice, leftAction->move);
                            EXPECT_EQ(target, leftAction->target);
                        }
                        if (action->choice == MOVE_MOONBLAST)
                            EXPECT_NE(target, opponentRightBattler);
                    }
                }
            }

            for (u32 rank = 0; rank < plan->candidateCount; rank++)
            {
                const struct BattleAiTraceCandidate *candidate =
                    BattleAiTrace_GetCandidate(plan->firstCandidateSequence + rank);

                if (candidate == NULL)
                    continue;
                for (u32 actionIndex = 0; actionIndex < ARRAY_COUNT(candidate->actions); actionIndex++)
                {
                    const struct BattleAiTraceAction *action = &candidate->actions[actionIndex];
                    enum BattlerId actor;
                    enum BattlerId target;

                    if (!(action->actorMeta & BATTLE_AI_TRACE_ACTION_VALID))
                        continue;
                    actor = action->actorMeta & BATTLE_AI_TRACE_ACTION_ACTOR_MASK;
                    target = action->targetMeta & BATTLE_AI_TRACE_ACTION_TARGET_MASK;
                    if (actor != opponentLeftBattler
                     || action->choice != MOVE_MOONBLAST
                     || target != opponentRightBattler)
                        continue;

                    foundRejectedMoonblastAlly = TRUE;
                    EXPECT(candidate->flags & BATTLE_AI_TRACE_CANDIDATE_FORCED_TRACE);
                    EXPECT(candidate->flags & BATTLE_AI_TRACE_CANDIDATE_PRUNED);
                    EXPECT(!(candidate->flags & BATTLE_AI_TRACE_CANDIDATE_CHOSEN));
                    EXPECT(candidate->rejectionFlags[actionIndex]
                           & AI_CANDIDATE_REJECTION_UNAPPROVED_ALLY_TARGET);
                    EXPECT_EQ(candidate->allyInteractionKinds[actionIndex], AI_ALLY_INTERACTION_NONE);
                }
            }
        }
        EXPECT(foundChosenXerneasAction);
        EXPECT(foundRejectedMoonblastAlly);
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: selected Fake Out still discounts lower-priority Extreme Speed")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        ASSUME(GetMovePriority(MOVE_FAKE_OUT) > GetMovePriority(MOVE_EXTREME_SPEED));
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_PREDICTION | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        PLAYER_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        OPPONENT_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        OPPONENT_DRAGONITE(MOVE_EXTREME_SPEED, MOVE_DRAGON_CLAW, MOVE_EARTHQUAKE, MOVE_ROOST);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_FAKE_OUT, target: opponentRight);
            MOVE(playerRight, MOVE_TAILWIND);
            EXPECT_MOVES(opponentLeft, MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
            SCORE_LT_VAL(opponentRight, MOVE_EXTREME_SPEED, AI_SCORE_DEFAULT, target: playerLeft);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: blocked selected Fake Out does not discount ordinary actions")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_PREDICTION | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        PLAYER_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        OPPONENT_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        OPPONENT_FARIGIRAF(MOVE_PSYCHIC, MOVE_TRICK_ROOM, MOVE_HELPING_HAND, MOVE_PROTECT);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_FAKE_OUT, target: opponentRight);
            MOVE(playerRight, MOVE_TAILWIND);
            EXPECT_MOVES(opponentLeft, MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
            SCORE_GT_VAL(opponentRight, MOVE_PSYCHIC, AI_SCORE_DEFAULT, target: playerRight);
        }
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY: AI can spend Dynamax to block phazing disruption")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_ROAR) == EFFECT_ROAR);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY);
        PLAYER_ARCANINE(MOVE_ROAR, MOVE_FLARE_BLITZ, MOVE_SNARL, MOVE_PROTECT);
        OPPONENT_CHARIZARD_DMAX(MOVE_AIR_SLASH, MOVE_PROTECT);
        OPPONENT_KYOGRE(MOVE_ORIGIN_PULSE, MOVE_ICE_BEAM);
    } WHEN {
        TURN { MOVE(player, MOVE_ROAR); EXPECT_MOVE(opponent, MOVE_AIR_SLASH, gimmick: GIMMICK_DYNAMAX); }
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY: AI can spend Dynamax on a low-HP pressure attacker with a reserve")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY);
        PLAYER_RILLABOOM(MOVE_FAKE_OUT, MOVE_GRASSY_GLIDE, MOVE_WOOD_HAMMER, MOVE_KNOCK_OFF);
        OPPONENT_KORAIDON_DMAX_LOW_HP(MOVE_FLARE_BLITZ, MOVE_COLLISION_COURSE, MOVE_DRAGON_CLAW, MOVE_PROTECT);
        OPPONENT_CHARIZARD_DMAX(MOVE_HEAT_WAVE, MOVE_AIR_SLASH, MOVE_SOLAR_BEAM, MOVE_PROTECT);
    } WHEN {
        TURN { MOVE(player, MOVE_KNOCK_OFF); EXPECT_MOVE(opponent, MOVE_FLARE_BLITZ, gimmick: GIMMICK_DYNAMAX); }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY: AI can spend Dynamax for Max Airstream speed control")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        PLAYER_RILLABOOM(MOVE_FAKE_OUT, MOVE_GRASSY_GLIDE, MOVE_WOOD_HAMMER, MOVE_KNOCK_OFF);
        OPPONENT_LANDORUS(MOVE_FLY, MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_PROTECT);
        OPPONENT_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        OPPONENT_CHARIZARD_DMAX(MOVE_HEAT_WAVE, MOVE_AIR_SLASH, MOVE_SOLAR_BEAM, MOVE_PROTECT);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_KNOCK_OFF, target: opponentLeft);
            MOVE(playerRight, MOVE_KNOCK_OFF, target: opponentLeft);
            EXPECT_MOVE(opponentLeft, MOVE_FLY, gimmick: GIMMICK_DYNAMAX);
        }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: G-Max Charizard values Wildfire over redundant Airstream under Tailwind")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE | AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY);
        SetStartingStatus(STARTING_STATUS_TAILWIND_OPPONENT_TEMPORARY);
        PLAYER_TAPU_KOKO_Z(MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_VOLT_SWITCH, MOVE_PROTECT);
        PLAYER_LANDORUS(MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_FLY, MOVE_PROTECT);
        OPPONENT_WHIMSICOTT(MOVE_TAILWIND, MOVE_SUNNY_DAY, MOVE_MOONBLAST, MOVE_PROTECT);
        OPPONENT_CHARIZARD_GMAX(MOVE_HEAT_WAVE, MOVE_AIR_SLASH, MOVE_SOLAR_BEAM, MOVE_PROTECT);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_THUNDERBOLT, target: opponentRight);
            MOVE(playerRight, MOVE_ROCK_SLIDE, target: opponentLeft);
            EXPECT_MOVE(opponentRight, MOVE_HEAT_WAVE, gimmick: GIMMICK_DYNAMAX);
        }
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY: G-Max Stonesurge is a hazard payoff")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_G_MAX_STONESURGE, MOVE_EFFECT_STEALTH_ROCK));
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(500); HP(500); Defense(200); Speed(30); Moves(MOVE_CELEBRATE); }
        PLAYER(SPECIES_WYNAUT) { MaxHP(200); HP(200); Speed(30); Moves(MOVE_CELEBRATE); }
        OPPONENT_DREDNAW_GMAX(MOVE_LIQUIDATION, MOVE_PROTECT);
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(500); HP(500); Speed(30); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); EXPECT_MOVE(opponent, MOVE_LIQUIDATION, gimmick: GIMMICK_DYNAMAX); }
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY: G-Max Stonesurge is conserved when rocks are already set")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY);
        ASSUME(MoveHasAdditionalEffect(MOVE_G_MAX_STONESURGE, MOVE_EFFECT_STEALTH_ROCK));
        SetStartingStatus(STARTING_STATUS_STEALTH_ROCK_PLAYER);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(500); HP(500); Defense(200); Speed(30); Moves(MOVE_CELEBRATE); }
        PLAYER(SPECIES_WYNAUT) { MaxHP(200); HP(200); Speed(30); Moves(MOVE_CELEBRATE); }
        OPPONENT_DREDNAW_GMAX(MOVE_LIQUIDATION, MOVE_PROTECT);
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(500); HP(500); Speed(30); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); EXPECT_MOVE(opponent, MOVE_LIQUIDATION, gimmick: GIMMICK_NONE); }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: smart Dynamax answers selected off-slot KO pressure")
{
    GIVEN {
        ASSUME(GetMoveType(MOVE_DRACO_METEOR) == TYPE_DRAGON);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE | AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        PLAYER_MIRAIDON_ICE_TERA(MOVE_DRACO_METEOR, MOVE_THUNDERBOLT, MOVE_ELECTRO_DRIFT, MOVE_VOLT_SWITCH);
        OPPONENT_KORAIDON_DMAX(MOVE_COLLISION_COURSE, MOVE_FLARE_BLITZ, MOVE_CLOSE_COMBAT);
        OPPONENT_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_FAKE_OUT, target: opponentRight);
            MOVE(playerRight, MOVE_DRACO_METEOR, target: opponentLeft);
            EXPECT_MOVE(opponentLeft, MOVE_COLLISION_COURSE, gimmick: GIMMICK_DYNAMAX);
        }
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_READ_PLAYER_MOVE: smart Dynamax answers selected single KO pressure")
{
    GIVEN {
        ASSUME(GetMoveType(MOVE_DRACO_METEOR) == TYPE_DRAGON);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_READ_PLAYER_MOVE | AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY);
        PLAYER_MIRAIDON_ICE_TERA(MOVE_DRACO_METEOR, MOVE_THUNDERBOLT, MOVE_ELECTRO_DRIFT, MOVE_VOLT_SWITCH);
        OPPONENT_KORAIDON_DMAX(MOVE_COLLISION_COURSE, MOVE_FLARE_BLITZ, MOVE_CLOSE_COMBAT);
        OPPONENT_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
    } WHEN {
        TURN {
            MOVE(player, MOVE_DRACO_METEOR);
            EXPECT_MOVE(opponent, MOVE_COLLISION_COURSE, gimmick: GIMMICK_DYNAMAX);
        }
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_GIMMICK_ENV_TERA_ONLY: AI can use Tera for an offensive boost")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_GIMMICK_ENV_TERA_ONLY);
        PLAYER_RILLABOOM(MOVE_FAKE_OUT, MOVE_GRASSY_GLIDE, MOVE_WOOD_HAMMER, MOVE_KNOCK_OFF);
        OPPONENT_KORAIDON_TERA(MOVE_FLARE_BLITZ, MOVE_COLLISION_COURSE, MOVE_DRAGON_CLAW, MOVE_PROTECT);
    } WHEN {
        TURN { MOVE(player, MOVE_KNOCK_OFF); EXPECT_MOVE(opponent, MOVE_FLARE_BLITZ, gimmick: GIMMICK_TERA); }
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_SMART_Z_MOVE: AI conserves a damaging Z-Move when it has no immediate payoff")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_SMART_GIMMICK_TIMING | AI_FLAG_SMART_Z_MOVE);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        OPPONENT_TAPU_KOKO_Z(MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_VOLT_SWITCH, MOVE_PROTECT);
        OPPONENT_RILLABOOM(MOVE_FAKE_OUT, MOVE_GRASSY_GLIDE, MOVE_WOOD_HAMMER, MOVE_KNOCK_OFF);
    } WHEN {
        TURN { MOVE(player, MOVE_KNOCK_OFF); EXPECT_MOVE(opponent, MOVE_THUNDERBOLT, gimmick: GIMMICK_NONE); }
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_SMART_Z_MOVE: AI can spend a damaging Z-Move on its last Pokemon")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_SMART_GIMMICK_TIMING | AI_FLAG_SMART_Z_MOVE);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        OPPONENT_TAPU_KOKO_Z(MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_VOLT_SWITCH, MOVE_PROTECT);
    } WHEN {
        TURN { MOVE(player, MOVE_KNOCK_OFF); EXPECT_MOVE(opponent, MOVE_THUNDERBOLT, gimmick: GIMMICK_Z_MOVE); }
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_SMART_Z_MOVE: AI can spend a damaging Z-Move with one reserve when low on HP")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_SMART_GIMMICK_TIMING | AI_FLAG_SMART_Z_MOVE);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        OPPONENT_TAPU_KOKO_Z_LOW_HP(MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_VOLT_SWITCH, MOVE_PROTECT);
        OPPONENT_RILLABOOM(MOVE_FAKE_OUT, MOVE_GRASSY_GLIDE, MOVE_WOOD_HAMMER, MOVE_KNOCK_OFF);
    } WHEN {
        TURN { MOVE(player, MOVE_KNOCK_OFF); EXPECT_MOVE(opponent, MOVE_THUNDERBOLT, gimmick: GIMMICK_Z_MOVE); }
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_SMART_Z_MOVE: AI can spend a damaging Z-Move from a low-HP pressure attacker with a reserve")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_SMART_GIMMICK_TIMING | AI_FLAG_SMART_Z_MOVE);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        OPPONENT_TAPU_KOKO_Z_LOW_HP(MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_VOLT_SWITCH, MOVE_PROTECT);
        OPPONENT_RILLABOOM(MOVE_FAKE_OUT, MOVE_GRASSY_GLIDE, MOVE_WOOD_HAMMER, MOVE_KNOCK_OFF);
    } WHEN {
        TURN { MOVE(player, MOVE_KNOCK_OFF); EXPECT_MOVE(opponent, MOVE_THUNDERBOLT, gimmick: GIMMICK_Z_MOVE); }
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_GIMMICK_ENV_ALL: AI can spend a Z-Move from a held Z-Crystal")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_GIMMICK_ENV_ALL);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        OPPONENT_TAPU_KOKO_Z(MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_VOLT_SWITCH, MOVE_PROTECT);
    } WHEN {
        TURN { MOVE(player, MOVE_KNOCK_OFF); EXPECT_MOVE(opponent, MOVE_THUNDERBOLT, gimmick: GIMMICK_Z_MOVE); }
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_GIMMICK_ENV_ALL: AI can Mega Evolve with one reserve when low on HP")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_GIMMICK_ENV_ALL);
        PLAYER_FARIGIRAF(MOVE_TRICK_ROOM, MOVE_PSYCHIC, MOVE_HELPING_HAND, MOVE_PROTECT);
        OPPONENT_VENUSAUR(47, MOVE_SLUDGE_BOMB, MOVE_GIGA_DRAIN, MOVE_EARTH_POWER, MOVE_PROTECT);
        OPPONENT_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
    } WHEN {
        TURN { MOVE(player, MOVE_PROTECT); EXPECT_MOVE(opponent, MOVE_SLUDGE_BOMB, gimmick: GIMMICK_MEGA); }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_MEGA_EVOLUTION, opponent);
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_SMART_Z_MOVE: AI conserves a damaging Z-Move against a trapping target without a damage-race payoff")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_SMART_GIMMICK_TIMING | AI_FLAG_SMART_Z_MOVE);
        PLAYER(SPECIES_GOTHITELLE) {
            Level(50); Item(ITEM_SITRUS_BERRY); Ability(ABILITY_SHADOW_TAG); Nature(NATURE_BOLD);
            TEST_IVS_SPECIAL();
            MaxHP(177); HP(177); Defense(150); SpAttack(115); SpDefense(130); Speed(85);
            Moves(MOVE_PSYCHIC, MOVE_HELPING_HAND, MOVE_TRICK_ROOM, MOVE_PROTECT);
        }
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        OPPONENT_TAPU_KOKO_Z(MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_VOLT_SWITCH, MOVE_PROTECT);
        OPPONENT_RILLABOOM(MOVE_FAKE_OUT, MOVE_GRASSY_GLIDE, MOVE_WOOD_HAMMER, MOVE_KNOCK_OFF);
    } WHEN {
        TURN { MOVE(player, MOVE_PROTECT); EXPECT_MOVE(opponent, MOVE_THUNDERBOLT, gimmick: GIMMICK_NONE); }
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_GIMMICK_ENV_DYNAMAX_TERA: AI can use Tera before a Dynamax backup")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_GIMMICK_ENV_DYNAMAX_TERA);
        PLAYER_RILLABOOM(MOVE_FAKE_OUT, MOVE_GRASSY_GLIDE, MOVE_WOOD_HAMMER, MOVE_KNOCK_OFF);
        OPPONENT_KORAIDON_TERA(MOVE_FLARE_BLITZ, MOVE_COLLISION_COURSE, MOVE_DRAGON_CLAW, MOVE_PROTECT);
        OPPONENT_CHARIZARD_DMAX(MOVE_HEAT_WAVE, MOVE_AIR_SLASH, MOVE_SOLAR_BEAM, MOVE_PROTECT);
    } WHEN {
        TURN { MOVE(player, MOVE_KNOCK_OFF); EXPECT_MOVE(opponent, MOVE_FLARE_BLITZ, gimmick: GIMMICK_TERA); }
    }
}

AI_DOUBLE_BATTLE_TEST("AI_FLAG_GIMMICK_ENV_ALL: AI can use Tera in doubles")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_GIMMICK_ENV_ALL);
        PLAYER_RILLABOOM(MOVE_FAKE_OUT, MOVE_GRASSY_GLIDE, MOVE_WOOD_HAMMER, MOVE_KNOCK_OFF);
        PLAYER_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
        OPPONENT_KORAIDON_TERA(MOVE_FLARE_BLITZ, MOVE_COLLISION_COURSE, MOVE_DRAGON_CLAW, MOVE_PROTECT);
        OPPONENT_TORNADUS(MOVE_TAILWIND, MOVE_TAUNT, MOVE_BLEAKWIND_STORM, MOVE_PROTECT);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_KNOCK_OFF, target: opponentLeft);
            MOVE(playerRight, MOVE_TAILWIND);
            EXPECT_MOVE(opponentLeft, MOVE_FLARE_BLITZ, gimmick: GIMMICK_TERA);
        }
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_SMART_MEGA: AI can Mega Evolve for Shadow Tag board control")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_SMART_GIMMICK_TIMING | AI_FLAG_SMART_MEGA);
        PLAYER_RILLABOOM(MOVE_FAKE_OUT, MOVE_GRASSY_GLIDE, MOVE_WOOD_HAMMER, MOVE_KNOCK_OFF);
        PLAYER_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
        OPPONENT_GENGAR(MOVE_SLUDGE_BOMB, MOVE_SHADOW_BALL, MOVE_FOCUS_BLAST, MOVE_PROTECT);
    } WHEN {
        TURN { MOVE(player, MOVE_KNOCK_OFF); EXPECT_MOVE(opponent, MOVE_SLUDGE_BOMB, gimmick: GIMMICK_MEGA); }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_MEGA_EVOLUTION, opponent);
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_SMART_MEGA: AI preserves weather suppression during active weather")
{
    enum Ability ability;

    PARAMETRIZE { ability = ABILITY_AIR_LOCK; }
    PARAMETRIZE { ability = ABILITY_CLOUD_NINE; }

    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_SMART_GIMMICK_TIMING | AI_FLAG_SMART_MEGA);
        PLAYER(SPECIES_KYOGRE) { Item(ITEM_BLUE_ORB); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_VENUSAUR) { Ability(ability); Item(ITEM_VENUSAURITE); Moves(MOVE_GIGA_DRAIN); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); EXPECT_MOVE(opponent, MOVE_GIGA_DRAIN, gimmick: GIMMICK_NONE); }
    } SCENE {
        NOT ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_MEGA_EVOLUTION, opponent);
    }
}

AI_SINGLE_BATTLE_TEST("AI_FLAG_GIMMICK_ENV_ALL: AI can delay Mega Evolution for a setup turn")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_FORCE_SETUP_FIRST_TURN | AI_FLAG_OMNISCIENT | AI_FLAG_GIMMICK_ENV_ALL);
        PLAYER_RILLABOOM(MOVE_FAKE_OUT, MOVE_GRASSY_GLIDE, MOVE_WOOD_HAMMER, MOVE_KNOCK_OFF);
        OPPONENT_VENUSAUR(187, MOVE_GROWTH, MOVE_SLUDGE_BOMB, MOVE_GIGA_DRAIN, MOVE_PROTECT);
        OPPONENT_INCINEROAR(MOVE_FAKE_OUT, MOVE_PARTING_SHOT, MOVE_FLARE_BLITZ, MOVE_KNOCK_OFF);
    } WHEN {
        TURN { MOVE(player, MOVE_GRASSY_GLIDE); EXPECT_MOVE(opponent, MOVE_GROWTH, gimmick: GIMMICK_NONE); }
    } SCENE {
        NOT ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_MEGA_EVOLUTION, opponent);
    }
}
