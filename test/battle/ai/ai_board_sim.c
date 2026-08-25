#include "global.h"
#include "test/battle.h"
#include "battle.h"
#include "battle_ai_board_sim.h"
#include "malloc.h"
#include "constants/abilities.h"
#include "constants/battle.h"
#include "constants/battle_ai.h"
#include "constants/items.h"
#include "constants/moves.h"

struct AiSimTestFixture
{
    u32 lowerCanary;
    struct AiSimContext context;
    struct AiSimBoard before;
    struct AiSimBoard after;
    struct AiSimJointTurn turn;
    struct AiSimTurnResult result;
    struct AiSimOutcomeKey outcomes[AI_SIM_MAX_OUTCOMES];
    u32 upperCanary;
};

#define AI_SIM_FIXTURE_LOWER_CANARY 0xA15C10E1
#define AI_SIM_FIXTURE_UPPER_CANARY 0xA15C10E2

static EWRAM_DATA struct AiSimTestFixture *sSimFixture;

#define sSimContext  (sSimFixture->context)
#define sSimBefore   (sSimFixture->before)
#define sSimAfter    (sSimFixture->after)
#define sSimTurn     (sSimFixture->turn)
#define sSimResult   (sSimFixture->result)
#define sSimOutcomes (sSimFixture->outcomes)

static void FreePureFixture(void)
{
    TRY_FREE_AND_SET_NULL(sSimFixture);
}

static bool32 PureFixtureCanariesIntact(void)
{
    return sSimFixture != NULL
        && sSimFixture->lowerCanary == AI_SIM_FIXTURE_LOWER_CANARY
        && sSimFixture->upperCanary == AI_SIM_FIXTURE_UPPER_CANARY;
}

static const u8 sRosterByBattler[MAX_BATTLERS_COUNT] = {0, 6, 1, 7};

static u32 HashBytes(u32 hash, const void *data, u32 size)
{
    const u8 *bytes = data;

    while (size-- != 0)
    {
        hash ^= *bytes++;
        hash *= 16777619;
    }
    return hash;
}

static void InitPureMon(u32 rosterIndex, u32 trainer, u32 partyIndex)
{
    struct AiSimMonTemplate *mon = &sSimContext.mons[rosterIndex];
    struct AiSimPartyState *party = &sSimBefore.party[rosterIndex];

    mon->normal.species = SPECIES_WOBBUFFET;
    mon->normal.maxHp = 200;
    mon->normal.attack = 100;
    mon->normal.defense = 100;
    mon->normal.speed = 100;
    mon->normal.spAttack = 100;
    mon->normal.spDefense = 100;
    mon->normal.ability = ABILITY_NONE;
    mon->normal.types[0] = TYPE_NORMAL;
    mon->normal.types[1] = TYPE_NONE;
    mon->normal.types[2] = TYPE_NONE;
    mon->normal.flags = AI_SIM_PROFILE_VALID
                      | AI_SIM_PROFILE_ABILITY_KNOWN
                      | AI_SIM_PROFILE_TYPES_KNOWN;
    mon->level = 50;
    mon->trainer = trainer;
    mon->partyIndex = partyIndex;
    mon->flags = AI_SIM_MON_PRESENT | AI_SIM_MON_MOVES_KNOWN | AI_SIM_MON_PROFILE_KNOWN;
    mon->dynamaxHpPercent = 150;

    party->hp = 200;
    party->flags = AI_SIM_PARTY_ITEM_KNOWN | AI_SIM_PARTY_STATUS_KNOWN;
}

static void InitPureDoubles(void)
{
    u32 battler;

    if (sSimFixture == NULL)
        sSimFixture = AllocZeroed(sizeof(*sSimFixture));

    memset(&sSimContext, 0, sizeof(sSimContext));
    memset(&sSimBefore, 0, sizeof(sSimBefore));
    memset(&sSimAfter, 0, sizeof(sSimAfter));
    memset(&sSimTurn, 0, sizeof(sSimTurn));
    memset(&sSimResult, 0, sizeof(sSimResult));
    memset(sSimOutcomes, 0, sizeof(sSimOutcomes));
    sSimFixture->lowerCanary = AI_SIM_FIXTURE_LOWER_CANARY;
    sSimFixture->upperCanary = AI_SIM_FIXTURE_UPPER_CANARY;

    sSimContext.battleTypeFlags = BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TRAINER;
    sSimContext.battlersCount = MAX_BATTLERS_COUNT;
    sSimContext.activeMask = 0xF;
    sSimContext.flags = AI_SIM_CONTEXT_OMNISCIENT;
    sSimBefore.activeMask = 0xF;

    for (battler = 0; battler < MAX_BATTLERS_COUNT; battler++)
    {
        u32 trainer = battler & BIT_SIDE;
        u32 partyIndex = battler >> 1;
        u32 rosterIndex = sRosterByBattler[battler];

        InitPureMon(rosterIndex, trainer, partyIndex);
        sSimContext.battlerTrainer[battler] = trainer;
        sSimContext.tieRank[battler] = battler;
        sSimContext.gimmickShareMask[battler] = trainer == B_SIDE_PLAYER ? 0x5 : 0xA;
        sSimBefore.active[battler].rosterIndex = rosterIndex;
        sSimBefore.active[battler].firstTurn = TRUE;
        sSimBefore.active[battler].choiceMoveSlot = AI_SIM_MOVE_SLOT_NONE;
        for (u32 stat = 0; stat < NUM_BATTLE_STATS; stat++)
            sSimBefore.active[battler].statStages[stat] = DEFAULT_STAT_STAGE;
    }
}

static void SetPureMove(u32 battler, u32 slot, enum Move move)
{
    u32 rosterIndex = sSimBefore.active[battler].rosterIndex;

    sSimContext.mons[rosterIndex].moves[slot] = move;
    sSimBefore.party[rosterIndex].pp[slot] = 10;
}

static void SetPureAction(u32 battler, u32 slot, enum Move move, u32 target)
{
    SetPureMove(battler, slot, move);
    sSimTurn.actions[battler] = (struct AiSimAction)
    {
        .choice = move,
        .actor = battler,
        .target = target,
        .kind = AI_SIM_ACTION_MOVE,
        .moveSlot = slot,
        .gimmick = GIMMICK_NONE,
    };
    sSimTurn.actionMask |= 1u << battler;
}

static struct AiSimOutcomeKey MakePureOutcome(u8 flags)
{
    struct AiSimOutcomeKey outcome =
    {
        .probabilityWeight = 1,
        .flags = flags,
        .speedTieOrder = AI_SIM_SPEED_TIE_ORDER_CONTEXT,
    };
    u32 battler;

    for (battler = 0; battler < MAX_BATTLERS_COUNT; battler++)
        outcome.damageRolls |= AI_SIM_DAMAGE_ROLL_MEDIAN << (battler * 4);
    return outcome;
}

static u32 SumOutcomeWeights(const struct AiSimOutcomeKey *outcomes,
                             u32 count,
                             u8 requiredFlags,
                             u8 forbiddenFlags,
                             u8 requiredProtectMask)
{
    u32 index;
    u32 total = 0;

    for (index = 0; index < count; index++)
    {
        if ((outcomes[index].flags & requiredFlags) != requiredFlags
         || (outcomes[index].flags & forbiddenFlags) != 0
         || (outcomes[index].protectSuccessMask & requiredProtectMask) != requiredProtectMask)
            continue;
        total += outcomes[index].probabilityWeight;
    }
    return total;
}

SINGLE_BATTLE_TEST("AI board simulator immutably resolves Fake Out before Tailwind and rejects ordinary ally damage")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 contextHash;
        u32 boardHash;

        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[0]].normal.speed = 120;
        sSimContext.mons[sRosterByBattler[1]].normal.speed = 100;
        SetPureAction(0, 0, MOVE_FAKE_OUT, 1);
        SetPureAction(1, 0, MOVE_TAILWIND, 1);
        contextHash = HashBytes(2166136261, &sSimContext, sizeof(sSimContext));
        boardHash = HashBytes(2166136261, &sSimBefore, sizeof(sSimBefore));

        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT_EQ(HashBytes(2166136261, &sSimContext, sizeof(sSimContext)), contextHash);
        EXPECT_EQ(HashBytes(2166136261, &sSimBefore, sizeof(sSimBefore)), boardHash);
        EXPECT(sSimResult.events & AI_SIM_EVENT_FLINCH);
        EXPECT(sSimResult.skippedMask & (1u << 1));
        EXPECT(!(sSimAfter.sides[B_SIDE_OPPONENT].statuses & SIDE_STATUS_TAILWIND));
        EXPECT_EQ(sSimAfter.party[sRosterByBattler[0]].pp[0], 9);
        EXPECT_EQ(sSimAfter.party[sRosterByBattler[1]].pp[0], 10);

        InitPureDoubles();
        SetPureAction(0, 0, MOVE_MOONBLAST, 2);
        EXPECT_EQ(AiSim_GetActionRejectionFlags(&sSimContext, &sSimBefore, &sSimTurn.actions[0]), AI_CANDIDATE_REJECTION_UNAPPROVED_ALLY_TARGET);
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL), AI_SIM_APPLY_INVALID);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator resolves both Power Herb Geomancy and slower Knock Off sequencing")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 xerneas = sRosterByBattler[0];

        InitPureDoubles();
        sSimContext.mons[xerneas].normal.speed = 150;
        sSimBefore.party[xerneas].item = ITEM_POWER_HERB;
        SetPureAction(0, 0, MOVE_GEOMANCY, 0);
        SetPureAction(1, 0, MOVE_KNOCK_OFF, 0);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.party[xerneas].item, ITEM_NONE);
        EXPECT(sSimAfter.party[xerneas].flags & AI_SIM_PARTY_ITEM_CONSUMED);
        EXPECT_EQ(sSimAfter.active[0].statStages[STAT_SPATK], DEFAULT_STAT_STAGE + 2);
        EXPECT_EQ(sSimAfter.active[0].statStages[STAT_SPDEF], DEFAULT_STAT_STAGE + 2);
        EXPECT_EQ(sSimAfter.active[0].statStages[STAT_SPEED], DEFAULT_STAT_STAGE + 2);
        EXPECT(sSimResult.readInteractionFlags[0] & AI_READ_INTERACTION_SETUP_ITEM_SEQUENCE);

        InitPureDoubles();
        sSimContext.mons[xerneas].normal.speed = 80;
        sSimBefore.party[xerneas].item = ITEM_POWER_HERB;
        SetPureAction(0, 0, MOVE_GEOMANCY, 0);
        SetPureAction(1, 0, MOVE_KNOCK_OFF, 0);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT(sSimAfter.party[xerneas].flags & AI_SIM_PARTY_ITEM_REMOVED);
        EXPECT(sSimAfter.active[0].volatileFlags & AI_SIM_VOLATILE_GEOMANCY_CHARGING);
        EXPECT_EQ(sSimAfter.active[0].statStages[STAT_SPATK], DEFAULT_STAT_STAGE);
        EXPECT(!(sSimResult.readInteractionFlags[0] & AI_READ_INTERACTION_SETUP_ITEM_SEQUENCE));

        sSimBefore = sSimAfter;
        memset(&sSimTurn, 0, sizeof(sSimTurn));
        sSimBefore.party[xerneas].pp[0] = 0; // Completion does not spend PP twice.
        SetPureAction(0, 0, MOVE_GEOMANCY, 0);
        sSimBefore.party[xerneas].pp[0] = 0;
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.active[0].statStages[STAT_SPATK], DEFAULT_STAT_STAGE + 2);
        EXPECT_EQ(sSimAfter.party[xerneas].pp[0], 0);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator exposes exact consecutive Protect branch probabilities")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 count;
        u32 index;
        u32 failureIndex = UINT32_MAX;
        u32 successIndex = UINT32_MAX;
        u32 failureWeight = 0;
        u32 successWeight = 0;

        InitPureDoubles();
        sSimBefore.active[0].consecutiveMoveUses = 1;
        SetPureAction(0, 0, MOVE_PROTECT, 0);
        SetPureAction(1, 0, MOVE_TACKLE, 0);
#if B_PROTECT_FAILURE_RATE < GEN_5
        EXPECT_EQ(AiSim_GetProtectSuccessDenominator(&sSimBefore, 0), 2);
#else
        EXPECT_EQ(AiSim_GetProtectSuccessDenominator(&sSimBefore, 0), 3);
#endif
        count = AiSim_EnumerateOutcomes(&sSimContext, &sSimBefore, &sSimTurn,
                                        sSimOutcomes, ARRAY_COUNT(sSimOutcomes));
        EXPECT(count > 2 && count <= AI_SIM_MAX_OUTCOMES);
        for (index = 0; index < count; index++)
        {
            if (sSimOutcomes[index].protectSuccessMask & 1)
            {
                successIndex = index;
                successWeight += sSimOutcomes[index].probabilityWeight;
            }
            else
            {
                failureIndex = index;
                failureWeight += sSimOutcomes[index].probabilityWeight;
            }
        }
        EXPECT(successIndex != UINT32_MAX);
        EXPECT(failureIndex != UINT32_MAX);
#if B_PROTECT_FAILURE_RATE < GEN_5
        EXPECT_EQ(failureWeight, successWeight);
#else
        EXPECT_EQ(failureWeight, successWeight * 2);
#endif

        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &sSimOutcomes[failureIndex], &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.active[0].consecutiveMoveUses, 0);
        EXPECT_EQ(sSimResult.protectSuccessMask, 0);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &sSimOutcomes[successIndex], &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.active[0].consecutiveMoveUses, 2);
        EXPECT_EQ(sSimResult.protectSuccessMask, 1);

        sSimOutcomes[successIndex].protectSuccessMask |= 1u << 3;
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &sSimOutcomes[successIndex], &sSimAfter, &sSimResult), AI_SIM_APPLY_UNSUPPORTED);

        // Like live IsLastMonToMove, the slower of two same-priority
        // protection actions fails after the faster action succeeds.
        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[0]].normal.speed = 150;
        sSimContext.mons[sRosterByBattler[1]].normal.speed = 50;
        SetPureAction(0, 0, MOVE_PROTECT, 0);
        SetPureAction(1, 0, MOVE_PROTECT, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimResult.actionOrder[0], 0);
        EXPECT_EQ(sSimResult.actionOrder[1], 1);
        EXPECT_EQ(sSimResult.protectSuccessMask, 1u << 0);
        EXPECT_EQ(sSimAfter.active[0].consecutiveMoveUses, 1);
        EXPECT_EQ(sSimAfter.active[1].consecutiveMoveUses, 0);

        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[0]].normal.speed = 150;
        sSimContext.mons[sRosterByBattler[1]].normal.speed = 50;
        SetPureAction(0, 0, MOVE_QUICK_GUARD, 0);
        SetPureAction(1, 0, MOVE_QUICK_GUARD, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimResult.protectSuccessMask, 1u << 0);
        EXPECT_EQ(sSimAfter.active[0].consecutiveMoveUses, 1);
        EXPECT_EQ(sSimAfter.active[1].consecutiveMoveUses, 0);

        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[0]].normal.speed = 150;
        sSimContext.mons[sRosterByBattler[1]].normal.speed = 50;
        SetPureAction(0, 0, MOVE_WIDE_GUARD, 0);
        SetPureAction(1, 0, MOVE_WIDE_GUARD, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimResult.protectSuccessMask, 1u << 0);
        EXPECT_EQ(sSimAfter.active[0].consecutiveMoveUses, 1);
        EXPECT_EQ(sSimAfter.active[1].consecutiveMoveUses, 0);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator weights 16 rolls critical hits survival and speed-tie KOs exactly")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 target = sRosterByBattler[1];
        u32 count;
        u32 index;
        u32 totalWeight;
        u32 criticalWeight = 0;
        u32 ordinaryWeight = 0;
        u32 koIndex = UINT32_MAX;
        u32 survivalIndex = UINT32_MAX;

        // One ordinary hit has 16 equiprobable damage rolls and a default
        // 1/24 critical branch. Equal final damage values are merged, but the
        // common-denominator weights still sum to 16 * 24 exactly.
        InitPureDoubles();
        SetPureAction(0, 0, MOVE_TACKLE, 1);
        count = AiSim_EnumerateOutcomes(&sSimContext, &sSimBefore, &sSimTurn,
                                        sSimOutcomes, ARRAY_COUNT(sSimOutcomes));
        EXPECT(count > 1 && count < AI_SIM_MAX_OUTCOMES);
        if (count > AI_SIM_MAX_OUTCOMES)
            count = AI_SIM_MAX_OUTCOMES;
        totalWeight = SumOutcomeWeights(sSimOutcomes, count, 0, 0, 0);
        EXPECT_EQ(totalWeight, 16 * 24);
        for (index = 0; index < count; index++)
        {
            if (sSimOutcomes[index].criticalMask & 1)
            {
                criticalWeight += sSimOutcomes[index].probabilityWeight;
                koIndex = index;
            }
            else
            {
                ordinaryWeight += sSimOutcomes[index].probabilityWeight;
                if (((sSimOutcomes[index].damageRolls >> 0) & 0xF)
                    == AI_SIM_DAMAGE_ROLL_MINIMUM)
                    survivalIndex = index;
            }
        }
        EXPECT_EQ(criticalWeight, 16);
        EXPECT_EQ(ordinaryWeight, 16 * 23);
        EXPECT(koIndex != UINT32_MAX);
        EXPECT(survivalIndex != UINT32_MAX);
        sSimBefore.party[target].hp = 27;
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                        &sSimOutcomes[survivalIndex], &sSimAfter,
                                        &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT(sSimAfter.party[target].hp != 0);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                        &sSimOutcomes[koIndex], &sSimAfter,
                                        &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.party[target].hp, 0);

        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator fails closed on pre-Generation-3 critical-hit odds")
{
    u32 genConfig;

    PARAMETRIZE { genConfig = GEN_1; }
    PARAMETRIZE { genConfig = GEN_2; }
    GIVEN {
        WITH_CONFIG(B_CRIT_CHANCE, genConfig);
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 unsupportedFlags = 0;

        InitPureDoubles();
        SetPureAction(0, 0, MOVE_TACKLE, 1);
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       &unsupportedFlags),
                  AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupportedFlags & AI_SIM_UNSUPPORTED_OUTCOME);
        EXPECT_EQ(AiSim_EnumerateOutcomes(&sSimContext, &sSimBefore, &sSimTurn,
                                          sSimOutcomes, ARRAY_COUNT(sSimOutcomes)),
                  0);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator applies exact Focus Sash survival to a critical outcome")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 target = sRosterByBattler[1];
        struct AiSimOutcomeKey critical = MakePureOutcome(0);

        // Focus Sash merges every would-be KO roll/critical into the same
        // one-HP board while preserving the item-consumption event weight.
        InitPureDoubles();
        sSimContext.mons[target].normal.maxHp = 27;
        sSimBefore.party[target].hp = 27;
        sSimBefore.party[target].item = ITEM_FOCUS_SASH;
        SetPureAction(0, 0, MOVE_TACKLE, 1);
        critical.criticalMask = 1u << 0;
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                        &critical, &sSimAfter,
                                        &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT(sSimResult.events & AI_SIM_EVENT_FOCUS_SASH);
        EXPECT_EQ(sSimAfter.party[target].hp, 1);
        EXPECT_EQ(sSimAfter.party[target].item, ITEM_NONE);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator applies exact Sturdy survival to a critical outcome")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 target = sRosterByBattler[1];
        struct AiSimOutcomeKey critical = MakePureOutcome(0);

        // Sturdy uses the same full-HP survival boundary without consuming an
        // item, including on an exact critical branch.
        InitPureDoubles();
        sSimContext.mons[target].normal.maxHp = 27;
        sSimContext.mons[target].normal.ability = ABILITY_STURDY;
        sSimBefore.party[target].hp = 27;
        SetPureAction(0, 0, MOVE_TACKLE, 1);
        critical.criticalMask = 1u << 0;
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                        &critical, &sSimAfter,
                                        &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT(sSimResult.events & AI_SIM_EVENT_STURDY);
        EXPECT_EQ(sSimAfter.party[target].hp, 1);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator makes critical damage bypass adverse stages and Reflect")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        struct AiSimOutcomeKey ordinary;
        struct AiSimOutcomeKey critical;
        u32 ordinaryDamage;
        u32 criticalDamage;

        // Critical damage ignores a negative attacking stage, a positive
        // defending stage, and Reflect within the simulator's supported set.
        InitPureDoubles();
        sSimBefore.active[0].statStages[STAT_ATK] = DEFAULT_STAT_STAGE - 2;
        sSimBefore.active[1].statStages[STAT_DEF] = DEFAULT_STAT_STAGE + 2;
        sSimBefore.sides[B_SIDE_OPPONENT].statuses |= SIDE_STATUS_REFLECT;
        sSimBefore.sides[B_SIDE_OPPONENT].reflectTimer = 5;
        SetPureAction(0, 0, MOVE_TACKLE, 1);
        ordinary = MakePureOutcome(0);
        critical = ordinary;
        critical.criticalMask = 1u << 0;
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                        &ordinary, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_OK);
        ordinaryDamage = sSimResult.damage[0];
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                        &critical, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_OK);
        criticalDamage = sSimResult.damage[0];
        EXPECT(criticalDamage > ordinaryDamage);

        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator resolves same-priority effective-Speed ties before a KO action")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 count;
        u32 index;

        // With one HP on both sides, all roll/crit results are capped KOs.
        // The only two unique results are the exact 50/50 Speed-tie orders;
        // the slower result is KOed before acting and spends no PP.
        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[0]].normal.maxHp = 1;
        sSimContext.mons[sRosterByBattler[1]].normal.maxHp = 1;
        sSimBefore.party[sRosterByBattler[0]].hp = 1;
        sSimBefore.party[sRosterByBattler[1]].hp = 1;
        SetPureAction(0, 0, MOVE_TACKLE, 1);
        SetPureAction(1, 0, MOVE_TACKLE, 0);
        count = AiSim_EnumerateOutcomes(&sSimContext, &sSimBefore, &sSimTurn,
                                        sSimOutcomes, ARRAY_COUNT(sSimOutcomes));
        EXPECT_EQ(count, 2);
        EXPECT_EQ(sSimOutcomes[0].probabilityWeight, 1);
        EXPECT_EQ(sSimOutcomes[1].probabilityWeight, 1);
        for (index = 0; index < count; index++)
        {
            EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                            &sSimOutcomes[index], &sSimAfter,
                                            &sSimResult),
                      AI_SIM_APPLY_OK);
            EXPECT_EQ(__builtin_popcount(sSimResult.executedMask & 0x3), 1);
            EXPECT_EQ(__builtin_popcount(sSimResult.skippedMask & 0x3), 1);
            EXPECT_EQ(sSimResult.actionOrder[0], index == 0 ? 0 : 1);
        }
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator bounds exact all-Tackle frontier overflow")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        // Each ordinary hit first groups the 32 (roll, critical) branches by
        // identical raw damage with their exact weights.  Independent damage
        // classes still produce more than 32 distinct post-turn boards, and
        // must prove that overflow without walking the 384^4 raw product.
        InitPureDoubles();
        SetPureAction(0, 0, MOVE_TACKLE, 1);
        SetPureAction(1, 0, MOVE_TACKLE, 0);
        SetPureAction(2, 0, MOVE_TACKLE, 3);
        SetPureAction(3, 0, MOVE_TACKLE, 2);
        EXPECT_EQ(AiSim_EnumerateOutcomes(&sSimContext, &sSimBefore, &sSimTurn,
                                          sSimOutcomes, ARRAY_COUNT(sSimOutcomes)),
                  AI_SIM_MAX_OUTCOMES + 1);
        EXPECT_LE(Test_AiSim_GetLastOutcomeApplicationCount(), 64);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator compresses one invariant-input hit across status gimmick and switch actions")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 actor = sRosterByBattler[1];
        u32 count;

        // Mega Bullet Punch uses its post-gimmick Attack and Technician, but
        // Tailwind and physical-target Geomancy cannot change its damage.
        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[0]].normal.speed = 400;
        sSimContext.mons[sRosterByBattler[1]].normal.speed = 100;
        sSimContext.mons[sRosterByBattler[2]].normal.speed = 300;
        sSimContext.mons[sRosterByBattler[3]].normal.speed = 200;
        sSimBefore.party[sRosterByBattler[2]].item = ITEM_POWER_HERB;
        sSimContext.mons[actor].transformed = sSimContext.mons[actor].normal;
        sSimContext.mons[actor].transformed.attack = 120;
        sSimContext.mons[actor].transformed.speed = 110;
        sSimContext.mons[actor].transformed.ability = ABILITY_TECHNICIAN;
        sSimContext.mons[actor].transformationGimmick = GIMMICK_MEGA;
        sSimContext.mons[actor].eligibleGimmicks |= 1u << GIMMICK_MEGA;
        SetPureAction(0, 0, MOVE_TAILWIND, 0);
        SetPureAction(1, 0, MOVE_BULLET_PUNCH, 2);
        SetPureAction(2, 0, MOVE_GEOMANCY, 2);
        SetPureAction(3, 0, MOVE_TAILWIND, 3);
        sSimTurn.actions[1].gimmick = GIMMICK_MEGA;

        count = AiSim_EnumerateOutcomes(&sSimContext, &sSimBefore, &sSimTurn,
                                        sSimOutcomes,
                                        ARRAY_COUNT(sSimOutcomes));
        EXPECT(count > 1 && count <= AI_SIM_MAX_OUTCOMES);
        EXPECT_EQ(SumOutcomeWeights(sSimOutcomes, count, 0, 0, 0), 16 * 24);
        EXPECT_LT(Test_AiSim_GetLastOutcomeApplicationCount(), 32);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       &sSimOutcomes[0], &sSimAfter,
                                       &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT(sSimAfter.active[1].flags & AI_SIM_ACTIVE_TRANSFORMED);
        EXPECT(sSimAfter.party[sRosterByBattler[2]].flags
             & AI_SIM_PARTY_ITEM_CONSUMED);

        // A passive switch from Prankster to Fairy Aura is damage-neutral for
        // non-Fairy Bullet Punch and is installed before class construction.
        InitPureDoubles();
        InitPureMon(8, B_SIDE_OPPONENT, 2);
        sSimContext.mons[sRosterByBattler[0]].normal.speed = 400;
        sSimContext.mons[sRosterByBattler[1]].normal.speed = 100;
        sSimContext.mons[sRosterByBattler[2]].normal.speed = 300;
        sSimContext.mons[sRosterByBattler[3]].normal.speed = 200;
        sSimContext.mons[sRosterByBattler[3]].normal.ability = ABILITY_PRANKSTER;
        sSimContext.mons[8].normal.ability = ABILITY_FAIRY_AURA;
        sSimBefore.party[sRosterByBattler[2]].item = ITEM_POWER_HERB;
        SetPureAction(0, 0, MOVE_TAILWIND, 0);
        SetPureAction(1, 0, MOVE_BULLET_PUNCH, 2);
        SetPureAction(2, 0, MOVE_GEOMANCY, 2);
        sSimTurn.actions[3] = (struct AiSimAction)
        {
            .choice = 8,
            .actor = 3,
            .kind = AI_SIM_ACTION_SWITCH,
        };
        sSimTurn.actionMask |= 1u << 3;

        count = AiSim_EnumerateOutcomes(&sSimContext, &sSimBefore, &sSimTurn,
                                        sSimOutcomes,
                                        ARRAY_COUNT(sSimOutcomes));
        EXPECT(count > 1 && count <= AI_SIM_MAX_OUTCOMES);
        EXPECT_EQ(SumOutcomeWeights(sSimOutcomes, count, 0, 0, 0), 16 * 24);
        EXPECT_LT(Test_AiSim_GetLastOutcomeApplicationCount(), 32);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       &sSimOutcomes[0], &sSimAfter,
                                       &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.active[3].rosterIndex, 8);

        // The nearby special case is intentionally raw: an earlier Geomancy
        // raises this target's Sp. Def, so initial-board damage classes would
        // be unsound.
        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[1]].normal.speed = 100;
        sSimContext.mons[sRosterByBattler[2]].normal.speed = 300;
        sSimBefore.party[sRosterByBattler[2]].item = ITEM_POWER_HERB;
        SetPureAction(1, 0, MOVE_WATER_GUN, 2);
        SetPureAction(2, 0, MOVE_GEOMANCY, 2);

        count = AiSim_EnumerateOutcomes(&sSimContext, &sSimBefore, &sSimTurn,
                                        sSimOutcomes,
                                        ARRAY_COUNT(sSimOutcomes));
        EXPECT(count > 1 && count <= AI_SIM_MAX_OUTCOMES);
        EXPECT_EQ(SumOutcomeWeights(sSimOutcomes, count, 0, 0, 0), 16 * 24);
        EXPECT_GE(Test_AiSim_GetLastOutcomeApplicationCount(), 32);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator collapses mixed Geomancy and one-HP guaranteed KOs")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 count;

        // Geomancy disables whole-turn damage-class compression, but its
        // Sp. Def boost cannot distinguish Ice Beam rolls against one HP.
        // All four effective Speeds are distinct, so the only exact board is
        // reached with one raw simulator application rather than 64^3.
        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[3]].normal.speed = 400;
        sSimContext.mons[sRosterByBattler[0]].normal.speed = 300;
        sSimContext.mons[sRosterByBattler[1]].normal.speed = 200;
        sSimContext.mons[sRosterByBattler[2]].normal.speed = 100;
        sSimBefore.party[sRosterByBattler[3]].item = ITEM_POWER_HERB;
        sSimBefore.party[sRosterByBattler[3]].hp = 1;
        sSimBefore.party[sRosterByBattler[0]].hp = 1;
        sSimBefore.party[sRosterByBattler[1]].hp = 1;
        SetPureAction(0, 0, MOVE_ICE_BEAM, 3);
        SetPureAction(1, 0, MOVE_ICE_BEAM, 0);
        SetPureAction(2, 0, MOVE_ICE_BEAM, 1);
        SetPureAction(3, 0, MOVE_GEOMANCY, 3);

        count = AiSim_EnumerateOutcomes(&sSimContext, &sSimBefore, &sSimTurn,
                                        sSimOutcomes, ARRAY_COUNT(sSimOutcomes));
        EXPECT_EQ(count, 1);
        EXPECT_EQ(Test_AiSim_GetLastOutcomeApplicationCount(), 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                        &sSimOutcomes[0], &sSimAfter,
                                        &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimResult.executedMask & 0xF, 0xF);
        EXPECT_EQ(sSimAfter.party[sRosterByBattler[3]].hp, 0);
        EXPECT_EQ(sSimAfter.party[sRosterByBattler[0]].hp, 0);
        EXPECT_EQ(sSimAfter.party[sRosterByBattler[1]].hp, 0);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator bounds raw applications when mixed guaranteed KOs cannot use the fast proof")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        // At two HP all three Ice Beams are still guaranteed KOs, but the
        // narrow one-HP proof intentionally does not claim them and Geomancy
        // disables whole-turn damage classes.  The 64^3 raw product merges to
        // one board, so only the absolute application budget can bound it.
        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[3]].normal.speed = 400;
        sSimContext.mons[sRosterByBattler[0]].normal.speed = 300;
        sSimContext.mons[sRosterByBattler[1]].normal.speed = 200;
        sSimContext.mons[sRosterByBattler[2]].normal.speed = 100;
        sSimBefore.party[sRosterByBattler[3]].item = ITEM_POWER_HERB;
        sSimBefore.party[sRosterByBattler[3]].hp = 2;
        sSimBefore.party[sRosterByBattler[0]].hp = 2;
        sSimBefore.party[sRosterByBattler[1]].hp = 2;
        SetPureAction(0, 0, MOVE_ICE_BEAM, 3);
        SetPureAction(1, 0, MOVE_ICE_BEAM, 0);
        SetPureAction(2, 0, MOVE_ICE_BEAM, 1);
        SetPureAction(3, 0, MOVE_GEOMANCY, 3);

        EXPECT_EQ(AiSim_EnumerateOutcomes(&sSimContext, &sSimBefore, &sSimTurn,
                                          sSimOutcomes, ARRAY_COUNT(sSimOutcomes)),
                  AI_SIM_MAX_OUTCOMES + 1);
        EXPECT_EQ(Test_AiSim_GetLastOutcomeApplicationCount(),
                  AI_SIM_MAX_OUTCOME_APPLICATIONS);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator branches only reachable dynamic Tailwind ties")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 count;

        // Tailwind acts first, then makes its partner exactly tie the foe.
        // Reflect and Light Screen commute, so both exact random orders merge
        // into one identical post-turn board.
        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[0]].normal.speed = 120;
        sSimContext.mons[sRosterByBattler[1]].normal.speed = 100;
        sSimContext.mons[sRosterByBattler[2]].normal.speed = 50;
        SetPureAction(0, 0, MOVE_TAILWIND, 0);
        SetPureAction(1, 0, MOVE_REFLECT, 1);
        SetPureAction(2, 0, MOVE_LIGHT_SCREEN, 2);
        count = AiSim_EnumerateOutcomes(&sSimContext, &sSimBefore, &sSimTurn,
                                        sSimOutcomes, ARRAY_COUNT(sSimOutcomes));
        EXPECT_EQ(count, 1);
        EXPECT_EQ(sSimOutcomes[0].probabilityWeight, 1);

        // A nearby non-equal post-Tailwind Speed has no RNG dimension.
        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[0]].normal.speed = 120;
        sSimContext.mons[sRosterByBattler[1]].normal.speed = 100;
        sSimContext.mons[sRosterByBattler[2]].normal.speed = 49;
        SetPureAction(0, 0, MOVE_TAILWIND, 0);
        SetPureAction(1, 0, MOVE_REFLECT, 1);
        SetPureAction(2, 0, MOVE_LIGHT_SCREEN, 2);
        EXPECT_EQ(AiSim_EnumerateOutcomes(&sSimContext, &sSimBefore, &sSimTurn,
                                          sSimOutcomes, ARRAY_COUNT(sSimOutcomes)),
                  1);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator treats repeated Tailwind as a deterministic no-op")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        InitPureDoubles();
        sSimBefore.sides[B_SIDE_PLAYER].statuses |= SIDE_STATUS_TAILWIND;
        sSimBefore.sides[B_SIDE_PLAYER].tailwindTimer = 3;
        SetPureAction(0, 0, MOVE_TAILWIND, 0);

        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL),
                  AI_SIM_APPLY_OK);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT(sSimResult.executedMask & (1u << 0));
        EXPECT_EQ(sSimAfter.party[sRosterByBattler[0]].pp[0], 9);
        EXPECT(sSimAfter.sides[B_SIDE_PLAYER].statuses & SIDE_STATUS_TAILWIND);
        EXPECT_EQ(sSimAfter.sides[B_SIDE_PLAYER].tailwindTimer, 2);
        EXPECT(!(sSimResult.events & AI_SIM_EVENT_FIELD_CHANGE));
        EXPECT_EQ(sSimBefore.sides[B_SIDE_PLAYER].tailwindTimer, 3);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator uses the configured Tailwind duration")
{
    u32 config;

    PARAMETRIZE { config = GEN_4; }
    PARAMETRIZE { config = GEN_5; }
    GIVEN {
        WITH_CONFIG(B_TAILWIND_TURNS, config);
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        InitPureDoubles();
        SetPureAction(0, 0, MOVE_TAILWIND, 0);

        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL),
                  AI_SIM_APPLY_OK);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT(sSimResult.executedMask & (1u << 0));
        EXPECT_EQ(sSimAfter.party[sRosterByBattler[0]].pp[0], 9);
        EXPECT(sSimAfter.sides[B_SIDE_PLAYER].statuses & SIDE_STATUS_TAILWIND);
        EXPECT_EQ(sSimAfter.sides[B_SIDE_PLAYER].tailwindTimer,
                  config >= GEN_5 ? 3 : 2);
        EXPECT(sSimResult.events & AI_SIM_EVENT_FIELD_CHANGE);
        EXPECT_EQ(sSimBefore.sides[B_SIDE_PLAYER].tailwindTimer, 0);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator obeys the Generation 8 dynamic Speed boundary")
{
    u32 config;

    PARAMETRIZE { config = GEN_7; }
    PARAMETRIZE { config = GEN_8; }
    GIVEN {
        WITH_CONFIG(B_RECALC_TURN_AFTER_ACTIONS, config);
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[0]].normal.speed = 120;
        sSimContext.mons[sRosterByBattler[1]].normal.speed = 80;
        sSimContext.mons[sRosterByBattler[2]].normal.speed = 50;
        SetPureAction(0, 0, MOVE_TAILWIND, 0);
        SetPureAction(1, 0, MOVE_LIGHT_SCREEN, 1);
        SetPureAction(2, 0, MOVE_REFLECT, 2);

        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimResult.actionOrder[0], 0);
        EXPECT_EQ(sSimResult.actionOrder[1], config >= GEN_8 ? 2 : 1);
        EXPECT_EQ(sSimResult.actionOrder[2], config >= GEN_8 ? 1 : 2);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator enumerates Tailwind Speed ties only with dynamic order")
{
    u32 config;

    PARAMETRIZE { config = GEN_7; }
    PARAMETRIZE { config = GEN_8; }
    GIVEN {
        WITH_CONFIG(B_RECALC_TURN_AFTER_ACTIONS, config);
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 count;
        u32 liveBattleTypeFlags = gBattleTypeFlags;
        struct AiSimTestFixture *fixture;

        InitPureDoubles();
        fixture = sSimFixture;
        EXPECT(PureFixtureCanariesIntact());
        sSimContext.mons[sRosterByBattler[0]].normal.speed = 120;
        sSimContext.mons[sRosterByBattler[1]].normal.speed = 100;
        sSimContext.mons[sRosterByBattler[2]].normal.speed = 50;
        sSimContext.mons[sRosterByBattler[1]].normal.maxHp = 1;
        sSimContext.mons[sRosterByBattler[2]].normal.maxHp = 1;
        sSimBefore.party[sRosterByBattler[1]].hp = 1;
        sSimBefore.party[sRosterByBattler[2]].hp = 1;
        SetPureAction(0, 0, MOVE_TAILWIND, 0);
        SetPureAction(1, 0, MOVE_TACKLE, 2);
        SetPureAction(2, 0, MOVE_TACKLE, 1);

        count = AiSim_EnumerateOutcomes(&sSimContext, &sSimBefore, &sSimTurn,
                                        sSimOutcomes,
                                        ARRAY_COUNT(sSimOutcomes));
        EXPECT_EQ(sSimFixture, fixture);
        EXPECT(PureFixtureCanariesIntact());
        EXPECT(Test_AiSim_OutcomeScratchCanariesIntact());
        EXPECT_EQ(gBattleTypeFlags, liveBattleTypeFlags);
        EXPECT_EQ(count, config >= GEN_8 ? 2 : 1);
        if (config < GEN_8)
        {
            EXPECT_EQ(sSimOutcomes[0].speedTieOrder,
                      AI_SIM_SPEED_TIE_ORDER_CONTEXT);
            EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore,
                                            &sSimTurn, &sSimOutcomes[0],
                                            &sSimAfter, &sSimResult),
                      AI_SIM_APPLY_OK);
            EXPECT(PureFixtureCanariesIntact());
            EXPECT_EQ(sSimResult.actionOrder[1], 1);
            EXPECT_EQ(sSimAfter.party[sRosterByBattler[2]].hp, 0);
        }
        else
        {
            u32 index;
            u8 firstTackleMask = 0;

            for (index = 0; index < count; index++)
            {
                EXPECT_EQ(sSimOutcomes[index].probabilityWeight, 1);
                EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore,
                                                &sSimTurn,
                                                &sSimOutcomes[index],
                                                &sSimAfter, &sSimResult),
                          AI_SIM_APPLY_OK);
                EXPECT(PureFixtureCanariesIntact());
                firstTackleMask |= 1u << sSimResult.actionOrder[1];
            }
            EXPECT_EQ(firstTackleMask, (1u << 1) | (1u << 2));
        }
        EXPECT_EQ(sSimFixture, fixture);
        EXPECT(PureFixtureCanariesIntact());
        EXPECT(Test_AiSim_OutcomeScratchCanariesIntact());
        EXPECT_EQ(gBattleTypeFlags, liveBattleTypeFlags);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator obeys the Generation 7 Mega turn-order boundary")
{
    u32 config;

    PARAMETRIZE { config = GEN_6; }
    PARAMETRIZE { config = GEN_7; }
    GIVEN {
        WITH_CONFIG(B_MEGA_EVO_TURN_ORDER, config);
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 actor = sRosterByBattler[0];

        InitPureDoubles();
        sSimContext.mons[actor].normal.speed = 80;
        sSimContext.mons[actor].transformed = sSimContext.mons[actor].normal;
        sSimContext.mons[actor].transformed.speed = 120;
        sSimContext.mons[actor].transformationGimmick = GIMMICK_MEGA;
        sSimContext.mons[actor].eligibleGimmicks |= 1u << GIMMICK_MEGA;
        sSimContext.mons[sRosterByBattler[1]].normal.speed = 100;
        SetPureAction(0, 0, MOVE_REFLECT, 0);
        SetPureAction(1, 0, MOVE_LIGHT_SCREEN, 1);
        sSimTurn.actions[0].gimmick = GIMMICK_MEGA;

        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimResult.actionOrder[0], config >= GEN_7 ? 0 : 1);
        EXPECT_EQ(sSimResult.actionOrder[1], config >= GEN_7 ? 1 : 0);
        EXPECT(sSimAfter.active[0].flags & AI_SIM_ACTIVE_TRANSFORMED);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator enumerates transformed Speed ties only in Generation 7 order")
{
    u32 config;

    PARAMETRIZE { config = GEN_6; }
    PARAMETRIZE { config = GEN_7; }
    GIVEN {
        WITH_CONFIG(B_MEGA_EVO_TURN_ORDER, config);
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 actor = sRosterByBattler[0];
        u32 target = sRosterByBattler[1];
        u32 count;
        u32 liveBattleTypeFlags = gBattleTypeFlags;
        struct AiSimTestFixture *fixture;

        InitPureDoubles();
        fixture = sSimFixture;
        EXPECT(PureFixtureCanariesIntact());
        sSimContext.mons[actor].normal.speed = 80;
        sSimContext.mons[actor].normal.maxHp = 1;
        sSimContext.mons[actor].transformed = sSimContext.mons[actor].normal;
        sSimContext.mons[actor].transformed.speed = 100;
        sSimContext.mons[actor].transformationGimmick = GIMMICK_MEGA;
        sSimContext.mons[actor].eligibleGimmicks |= 1u << GIMMICK_MEGA;
        sSimContext.mons[target].normal.speed = 100;
        sSimContext.mons[target].normal.maxHp = 1;
        sSimBefore.party[actor].hp = 1;
        sSimBefore.party[target].hp = 1;
        SetPureAction(0, 0, MOVE_TACKLE, 1);
        SetPureAction(1, 0, MOVE_TACKLE, 0);
        sSimTurn.actions[0].gimmick = GIMMICK_MEGA;

        count = AiSim_EnumerateOutcomes(&sSimContext, &sSimBefore, &sSimTurn,
                                        sSimOutcomes,
                                        ARRAY_COUNT(sSimOutcomes));
        EXPECT_EQ(sSimFixture, fixture);
        EXPECT(PureFixtureCanariesIntact());
        EXPECT(Test_AiSim_OutcomeScratchCanariesIntact());
        EXPECT_EQ(gBattleTypeFlags, liveBattleTypeFlags);
        EXPECT_EQ(count, config >= GEN_7 ? 2 : 1);
        if (config < GEN_7)
        {
            EXPECT_EQ(sSimOutcomes[0].speedTieOrder,
                      AI_SIM_SPEED_TIE_ORDER_CONTEXT);
            EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore,
                                            &sSimTurn, &sSimOutcomes[0],
                                            &sSimAfter, &sSimResult),
                      AI_SIM_APPLY_OK);
            EXPECT(PureFixtureCanariesIntact());
            EXPECT_EQ(sSimResult.actionOrder[0], 1);
            EXPECT_EQ(sSimAfter.party[actor].hp, 0);
        }
        else
        {
            u32 index;
            u8 firstActorMask = 0;

            for (index = 0; index < count; index++)
            {
                EXPECT_EQ(sSimOutcomes[index].probabilityWeight, 1);
                EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore,
                                                &sSimTurn,
                                                &sSimOutcomes[index],
                                                &sSimAfter, &sSimResult),
                          AI_SIM_APPLY_OK);
                EXPECT(PureFixtureCanariesIntact());
                firstActorMask |= 1u << sSimResult.actionOrder[0];
            }
            EXPECT_EQ(firstActorMask, (1u << 0) | (1u << 1));
        }
        EXPECT_EQ(sSimFixture, fixture);
        EXPECT(PureFixtureCanariesIntact());
        EXPECT(Test_AiSim_OutcomeScratchCanariesIntact());
        EXPECT_EQ(gBattleTypeFlags, liveBattleTypeFlags);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator spends PP, applies Choice lock, and supports forced replacement after KO")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        InitPureDoubles();
        sSimBefore.party[sRosterByBattler[0]].item = ITEM_CHOICE_SCARF;
        SetPureAction(0, 0, MOVE_TACKLE, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.party[sRosterByBattler[0]].pp[0], 9);
        EXPECT_EQ(sSimAfter.active[0].choiceMoveSlot, 0);
        EXPECT(sSimResult.events & AI_SIM_EVENT_PP_SPENT);
        EXPECT(sSimResult.events & AI_SIM_EVENT_CHOICE_LOCK);

        sSimBefore = sSimAfter;
        memset(&sSimTurn, 0, sizeof(sSimTurn));
        SetPureAction(0, 1, MOVE_SCRATCH, 1);
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL), AI_SIM_APPLY_INVALID);

        InitPureDoubles();
        InitPureMon(2, B_SIDE_PLAYER, 2);
        sSimBefore.party[sRosterByBattler[0]].hp = 0;
        sSimBefore.active[0].flags |= AI_SIM_ACTIVE_NEEDS_REPLACEMENT;
        sSimContext.mons[2].moves[0] = MOVE_FAKE_OUT;
        sSimBefore.party[2].pp[0] = 10;
        sSimTurn.actions[0] = (struct AiSimAction)
        {
            .choice = MOVE_FAKE_OUT,
            .actor = 0,
            .kind = AI_SIM_ACTION_MOVE,
            .target = 1,
            .moveSlot = 0,
            .replacementRosterIndex = 2,
            .flags = AI_SIM_ACTION_FORCED_REPLACEMENT,
        };
        sSimTurn.actionMask = 1;
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.active[0].rosterIndex, 2);
        EXPECT(!sSimAfter.active[0].firstTurn);
        EXPECT(!(sSimAfter.active[0].flags & AI_SIM_ACTIVE_NEEDS_REPLACEMENT));
        EXPECT_EQ(sSimAfter.party[2].pp[0], 9);
        EXPECT(sSimResult.executedMask & (1u << 0));
        EXPECT(sSimResult.damage[0] != 0);

        sSimBefore.sides[B_SIDE_PLAYER].hazardsMask = 1u << HAZARDS_STEALTH_ROCK;
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL), AI_SIM_APPLY_UNSUPPORTED);

        sSimBefore = sSimAfter;
        memset(&sSimTurn, 0, sizeof(sSimTurn));
        SetPureAction(0, 0, MOVE_FAKE_OUT, 1);
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL), AI_SIM_APPLY_INVALID);

        // If both same-trainer slots faint, replacement prompts consume the
        // finite reserve pool in battler-id order.
        InitPureDoubles();
        InitPureMon(8, B_SIDE_OPPONENT, 2);
        sSimBefore.party[sRosterByBattler[1]].hp = 0;
        sSimBefore.party[sRosterByBattler[3]].hp = 0;
        sSimBefore.active[1].flags |= AI_SIM_ACTIVE_NEEDS_REPLACEMENT;
        sSimBefore.active[3].flags |= AI_SIM_ACTIVE_NEEDS_REPLACEMENT;
        EXPECT(AiSim_ReplacementSlotWillBeFilled(&sSimContext, &sSimBefore, 1));
        EXPECT(!AiSim_ReplacementSlotWillBeFilled(&sSimContext, &sSimBefore, 3));

        InitPureMon(9, B_SIDE_OPPONENT, 3);
        EXPECT(AiSim_ReplacementSlotWillBeFilled(&sSimContext, &sSimBefore, 1));
        EXPECT(AiSim_ReplacementSlotWillBeFilled(&sSimContext, &sSimBefore, 3));

        sSimBefore.party[8].hp = 0;
        sSimBefore.party[9].hp = 0;
        EXPECT(!AiSim_ReplacementSlotWillBeFilled(&sSimContext, &sSimBefore, 1));
        EXPECT(!AiSim_ReplacementSlotWillBeFilled(&sSimContext, &sSimBefore, 3));
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator rejects normal and forced switch-in of active Stellar Tera")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 unsupported = 0;

        InitPureDoubles();
        InitPureMon(2, B_SIDE_PLAYER, 2);
        sSimContext.mons[2].teraType = TYPE_STELLAR;
        sSimBefore.party[2].activeGimmick = GIMMICK_TERA;
        sSimTurn.actions[0] = (struct AiSimAction)
        {
            .choice = 2,
            .actor = 0,
            .kind = AI_SIM_ACTION_SWITCH,
        };
        sSimTurn.actionMask = 1;
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       &unsupported),
                  AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_GIMMICK);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_DAMAGE_MODIFIER);

        InitPureDoubles();
        InitPureMon(2, B_SIDE_PLAYER, 2);
        sSimContext.mons[2].teraType = TYPE_STELLAR;
        sSimContext.mons[2].moves[0] = MOVE_TACKLE;
        sSimBefore.party[2].activeGimmick = GIMMICK_TERA;
        sSimBefore.party[2].pp[0] = 10;
        sSimBefore.party[sRosterByBattler[0]].hp = 0;
        sSimBefore.active[0].flags |= AI_SIM_ACTIVE_NEEDS_REPLACEMENT;
        sSimTurn.actions[0] = (struct AiSimAction)
        {
            .choice = MOVE_TACKLE,
            .actor = 0,
            .kind = AI_SIM_ACTION_MOVE,
            .target = 1,
            .moveSlot = 0,
            .replacementRosterIndex = 2,
            .flags = AI_SIM_ACTION_FORCED_REPLACEMENT,
        };
        sSimTurn.actionMask = 1;
        unsupported = 0;
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       &unsupported),
                  AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_GIMMICK);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_DAMAGE_MODIFIER);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator validates every spread target and fails closed on unrepresented state")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        struct AiSimContext invalidContext;
        struct SimulatedDamage damage;
        u32 unsupported = 0;

        InitPureDoubles();
        SetPureAction(0, 0, MOVE_EARTHQUAKE, 1);
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       &unsupported),
                  AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_OUTCOME);
        // Live damage and critical RNG are independent per defender.  The
        // compact per-actor outcome key cannot claim an exact shared roll.
        EXPECT_EQ(AiSim_EnumerateOutcomes(&sSimContext, &sSimBefore, &sSimTurn,
                                          sSimOutcomes, ARRAY_COUNT(sSimOutcomes)),
                  0);

        InitPureDoubles();
        unsupported = 0;
        SetPureAction(0, 0, MOVE_DOUBLE_KICK, 1);
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       &unsupported),
                  AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_OUTCOME);

        // An already-active Stellar Tera is also a categorical snapshot
        // boundary for an otherwise supported single-target move.  Stellar
        // retains defensive base typing and needs an offensive boost ledger,
        // neither of which is represented by the compact board.
        InitPureDoubles();
        unsupported = 0;
        SetPureAction(0, 0, MOVE_TACKLE, 1);
        sSimBefore.active[1].flags |= AI_SIM_ACTIVE_TERA;
        sSimContext.mons[sRosterByBattler[1]].teraType = TYPE_STELLAR;
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       &unsupported),
                  AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_GIMMICK);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_DAMAGE_MODIFIER);
        EXPECT_EQ(AiSim_EnumerateOutcomes(&sSimContext, &sSimBefore, &sSimTurn,
                                          sSimOutcomes, ARRAY_COUNT(sSimOutcomes)),
                  0);

        InitPureDoubles();
        SetPureAction(0, 0, MOVE_EARTHQUAKE, 1);
        sSimBefore.active[3].flags |= AI_SIM_ACTIVE_TERA;
        sSimContext.mons[sRosterByBattler[3]].teraType = TYPE_STELLAR;
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &unsupported), AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_OUTCOME);

        InitPureDoubles();
        SetPureAction(0, 0, MOVE_TACKLE, 1);
        sSimContext.flags |= AI_SIM_CONTEXT_PARTIAL_KNOWLEDGE;
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &unsupported), AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_KNOWLEDGE);

        sSimContext.flags &= ~AI_SIM_CONTEXT_PARTIAL_KNOWLEDGE;
        sSimBefore.sides[B_SIDE_OPPONENT].followMeTimer = 1;
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &unsupported), AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_REDIRECTION);

        sSimBefore.sides[B_SIDE_OPPONENT].followMeTimer = 0;
        sSimBefore.party[sRosterByBattler[0]].status1 = STATUS1_POISON;
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &unsupported), AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_RESIDUAL);

        InitPureDoubles();
        SetPureAction(0, 0, MOVE_TACKLE, 1);
        invalidContext = sSimContext;
        invalidContext.battlersCount = MAX_BATTLERS_COUNT + 1;
        damage = (struct SimulatedDamage){1, 1, 1, 1};
        EXPECT(!AiSim_ProjectDamage(&invalidContext, &sSimBefore,
                                    &sSimTurn.actions[0], 1, &damage));
        EXPECT_EQ(damage.minimum, 0);
        EXPECT_EQ(damage.median, 0);
        EXPECT_EQ(damage.maximum, 0);
        EXPECT_EQ(damage.random, 0);
        EXPECT(!AiSim_ReplacementSlotWillBeFilled(&invalidContext,
                                                  &sSimBefore, 0));
        EXPECT(!AiSim_ReplacementSlotWillBeFilled(&invalidContext,
                                                  &sSimBefore,
                                                  MAX_BATTLERS_COUNT));

        SetPureAction(0, 0, MOVE_EXPANDING_FORCE, 1);
        EXPECT_EQ(AiSim_GetActionRejectionFlags(NULL, &sSimBefore,
                                                &sSimTurn.actions[0]),
                  AI_CANDIDATE_REJECTION_NONE);
        EXPECT_EQ(AiSim_GetActionRejectionFlags(&sSimContext, NULL,
                                                &sSimTurn.actions[0]),
                  AI_CANDIDATE_REJECTION_NONE);
        EXPECT_EQ(AiSim_GetActionRejectionFlags(&invalidContext, &sSimBefore,
                                                &sSimTurn.actions[0]),
                  AI_CANDIDATE_REJECTION_NONE);

        sSimBefore.party[sRosterByBattler[0]].item = ITEMS_COUNT;
        EXPECT_EQ(AiSim_GetEffectiveSpeed(&sSimContext, &sSimBefore, 0), 0);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator models T2 Sitrus timing and Moonblast secondary branches")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 target = sRosterByBattler[1];
        u32 damage;
        struct AiSimOutcomeKey noSecondary;
        struct AiSimOutcomeKey secondary;

        InitPureDoubles();
        sSimBefore.party[target].hp = 101;
        sSimBefore.party[target].item = ITEM_SITRUS_BERRY;
        SetPureAction(0, 0, MOVE_MOONBLAST, 1);
        // Damage rolls, critical hits, and the secondary produce more than
        // the bounded exact frontier here.  The node fails closed rather than
        // silently substituting a median roll.
        EXPECT_EQ(AiSim_EnumerateOutcomes(&sSimContext, &sSimBefore, &sSimTurn,
                                          sSimOutcomes, ARRAY_COUNT(sSimOutcomes)),
                  AI_SIM_MAX_OUTCOMES + 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_UNSUPPORTED);

        noSecondary = MakePureOutcome(0);
        secondary = MakePureOutcome(1u << 0);

        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &noSecondary, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        damage = sSimResult.damage[0];
        EXPECT(damage > 0 && damage < 101);
        EXPECT_EQ(sSimAfter.party[target].hp, 101 - damage + 50);
        EXPECT_EQ(sSimAfter.party[target].item, ITEM_NONE);
        EXPECT(sSimAfter.party[target].flags & AI_SIM_PARTY_ITEM_CONSUMED);
        EXPECT(sSimResult.events & AI_SIM_EVENT_ITEM_CONSUMED);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_SPATK], DEFAULT_STAT_STAGE);

        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &secondary, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_SPATK], DEFAULT_STAT_STAGE - 1);
        EXPECT(sSimResult.events & AI_SIM_EVENT_STAT_CHANGE);

        InitPureDoubles();
        sSimBefore.party[target].hp = 150;
        sSimBefore.party[target].item = ITEM_SITRUS_BERRY;
        SetPureAction(0, 0, MOVE_TACKLE, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT(sSimAfter.party[target].hp > 100);
        EXPECT_EQ(sSimAfter.party[target].item, ITEM_SITRUS_BERRY);

        InitPureDoubles();
        sSimBefore.party[target].hp = 101;
        sSimBefore.party[target].item = ITEM_SITRUS_BERRY;
        SetPureAction(0, 0, MOVE_KNOCK_OFF, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT(sSimAfter.party[target].hp <= 100);
        EXPECT_EQ(sSimAfter.party[target].item, ITEM_NONE);
        EXPECT(sSimAfter.party[target].flags & AI_SIM_PARTY_ITEM_REMOVED);
        EXPECT(!(sSimAfter.party[target].flags & AI_SIM_PARTY_ITEM_CONSUMED));
        EXPECT(sSimResult.events & AI_SIM_EVENT_ITEM_REMOVED);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator models single-target Expanding Force and rejects its spread terrain form")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 offTerrainDamage;
        u32 unsupported = 0;

        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[0]].normal.types[0] = TYPE_PSYCHIC;
        SetPureAction(0, 0, MOVE_EXPANDING_FORCE, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        offTerrainDamage = 200 - sSimAfter.party[sRosterByBattler[1]].hp;
        EXPECT(offTerrainDamage > 0);
        EXPECT_EQ(sSimAfter.party[sRosterByBattler[3]].hp, 200);

        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[0]].normal.types[0] = TYPE_PSYCHIC;
        sSimBefore.fieldStatuses |= STATUS_FIELD_PSYCHIC_TERRAIN;
        sSimBefore.terrainTimer = 5;
        SetPureAction(0, 0, MOVE_EXPANDING_FORCE, 1);
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       &unsupported),
                  AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_OUTCOME);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_UNSUPPORTED);

        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[0]].normal.types[0] = TYPE_FLYING;
        sSimBefore.fieldStatuses |= STATUS_FIELD_PSYCHIC_TERRAIN;
        sSimBefore.terrainTimer = 5;
        SetPureAction(0, 0, MOVE_EXPANDING_FORCE, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT(sSimAfter.party[sRosterByBattler[1]].hp < 200);
        EXPECT_EQ(sSimAfter.party[sRosterByBattler[3]].hp, 200);

        InitPureDoubles();
        sSimBefore.fieldStatuses |= STATUS_FIELD_PSYCHIC_TERRAIN;
        sSimBefore.terrainTimer = 5;
        SetPureAction(0, 0, MOVE_FAKE_OUT, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.party[sRosterByBattler[1]].hp, 200);
        EXPECT_EQ(sSimResult.damage[0], 0);
        EXPECT(!(sSimResult.events & AI_SIM_EVENT_FLINCH));
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator makes Psystrike use Sp. Attack against physical Defense")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 baselineDamage;
        u32 highDefenseDamage;
        u32 highSpDefenseDamage;
        u32 lightScreenDamage;
        u32 reflectDamage;

        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[0]].normal.attack = 1;
        sSimContext.mons[sRosterByBattler[0]].normal.spAttack = 200;
        SetPureAction(0, 0, MOVE_PSYSTRIKE, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        baselineDamage = sSimResult.damage[0];
        EXPECT(baselineDamage > 50);

        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[0]].normal.attack = 1;
        sSimContext.mons[sRosterByBattler[0]].normal.spAttack = 200;
        sSimContext.mons[sRosterByBattler[1]].normal.defense = 400;
        SetPureAction(0, 0, MOVE_PSYSTRIKE, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        highDefenseDamage = sSimResult.damage[0];
        EXPECT(highDefenseDamage < baselineDamage / 2);

        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[0]].normal.attack = 1;
        sSimContext.mons[sRosterByBattler[0]].normal.spAttack = 200;
        sSimContext.mons[sRosterByBattler[1]].normal.spDefense = 400;
        SetPureAction(0, 0, MOVE_PSYSTRIKE, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        highSpDefenseDamage = sSimResult.damage[0];
        EXPECT_EQ(highSpDefenseDamage, baselineDamage);

        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[0]].normal.attack = 1;
        sSimContext.mons[sRosterByBattler[0]].normal.spAttack = 200;
        sSimBefore.sides[B_SIDE_OPPONENT].statuses |= SIDE_STATUS_LIGHTSCREEN;
        sSimBefore.sides[B_SIDE_OPPONENT].lightScreenTimer = 5;
        SetPureAction(0, 0, MOVE_PSYSTRIKE, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        lightScreenDamage = sSimResult.damage[0];
        EXPECT(lightScreenDamage < baselineDamage);

        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[0]].normal.attack = 1;
        sSimContext.mons[sRosterByBattler[0]].normal.spAttack = 200;
        sSimBefore.sides[B_SIDE_OPPONENT].statuses |= SIDE_STATUS_REFLECT;
        sSimBefore.sides[B_SIDE_OPPONENT].reflectTimer = 5;
        SetPureAction(0, 0, MOVE_PSYSTRIKE, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        reflectDamage = sSimResult.damage[0];
        EXPECT_EQ(reflectDamage, baselineDamage);

        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[0]].normal.spAttack = 200;
        sSimBefore.party[sRosterByBattler[0]].status1 = STATUS1_FROSTBITE;
        SetPureAction(0, 0, MOVE_PSYSTRIKE, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT(sSimResult.damage[0] < baselineDamage);
#if B_BURN_DAMAGE >= GEN_7
        EXPECT_EQ(sSimAfter.party[sRosterByBattler[0]].hp, 200 - 200 / 16);
#else
        EXPECT_EQ(sSimAfter.party[sRosterByBattler[0]].hp, 200 - 200 / 8);
#endif
        EXPECT(sSimAfter.party[sRosterByBattler[0]].status1 & STATUS1_FROSTBITE);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator applies Neuroforce and Prism Armor only to super-effective damage")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        struct SimulatedDamage baseline;
        struct SimulatedDamage neuroforce;
        struct SimulatedDamage prismArmor;
        u32 attacker = sRosterByBattler[0];
        u32 defender = sRosterByBattler[1];

        InitPureDoubles();
        sSimContext.mons[defender].normal.types[0] = TYPE_GRASS;
        SetPureAction(0, 0, MOVE_ICE_BEAM, 1);
        EXPECT(AiSim_CalcDamage(&sSimContext, &sSimBefore,
                                &sSimTurn.actions[0], 1, &baseline));

        sSimContext.mons[attacker].normal.ability = ABILITY_NEUROFORCE;
        EXPECT(AiSim_CalcDamage(&sSimContext, &sSimBefore,
                                &sSimTurn.actions[0], 1, &neuroforce));
        EXPECT_EQ(neuroforce.maximum, baseline.maximum * 5 / 4);

        sSimContext.mons[attacker].normal.ability = ABILITY_NONE;
        sSimContext.mons[defender].normal.ability = ABILITY_PRISM_ARMOR;
        EXPECT(AiSim_CalcDamage(&sSimContext, &sSimBefore,
                                &sSimTurn.actions[0], 1, &prismArmor));
        EXPECT_EQ(prismArmor.maximum, baseline.maximum * 3 / 4);

        sSimContext.mons[defender].normal.types[0] = TYPE_NORMAL;
        EXPECT(AiSim_CalcDamage(&sSimContext, &sSimBefore,
                                &sSimTurn.actions[0], 1, &prismArmor));
        sSimContext.mons[defender].normal.ability = ABILITY_NONE;
        EXPECT(AiSim_CalcDamage(&sSimContext, &sSimBefore,
                                &sSimTurn.actions[0], 1, &baseline));
        EXPECT_EQ(prismArmor.maximum, baseline.maximum);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator branches Ice Beam status and immediate or future thaw exactly")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 frozenTarget = sRosterByBattler[1];
        struct AiSimOutcomeKey noFreezeNoThaw;
        struct AiSimOutcomeKey freezeNoThaw;
        struct AiSimOutcomeKey freezeAndThaw;
        u32 count;
        u32 index;
        u32 frozenIndex = UINT32_MAX;
        u32 thawIndex = UINT32_MAX;

        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[0]].normal.speed = 200;
        sSimContext.mons[frozenTarget].normal.speed = 100;
        SetPureAction(0, 0, MOVE_ICE_BEAM, 1);
        SetPureAction(1, 0, MOVE_TACKLE, 0);
        EXPECT_EQ(AiSim_EnumerateOutcomes(&sSimContext, &sSimBefore, &sSimTurn,
                                          sSimOutcomes, ARRAY_COUNT(sSimOutcomes)),
                  AI_SIM_MAX_OUTCOMES + 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_UNSUPPORTED);

        noFreezeNoThaw = MakePureOutcome(0);
        freezeNoThaw = MakePureOutcome(1u << 0);
        freezeAndThaw = MakePureOutcome((1u << 0) | (1u << (1 + AI_SIM_OUTCOME_THAW_SHIFT)));

        // No Ice Beam secondary means the target acts; the unused thaw branch
        // is an equivalent probability partition, not an invented thaw.
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &noFreezeNoThaw, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.party[frozenTarget].status1, 0);
        EXPECT(sSimResult.executedMask & (1u << 1));

        // Freeze succeeds before the slower action and its 80% thaw check
        // fails, so the action is skipped without PP loss and Freeze persists.
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &freezeNoThaw, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT(sSimAfter.party[frozenTarget].status1 & STATUS1_FREEZE);
        EXPECT(sSimResult.skippedMask & (1u << 1));
        EXPECT(!(sSimResult.executedMask & (1u << 1)));
        EXPECT_EQ(sSimAfter.party[frozenTarget].pp[0], 10);
        EXPECT(sSimResult.events & AI_SIM_EVENT_STATUS_CHANGE);

        // The 20% branch thaws immediately, spends PP, and executes normally.
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &freezeAndThaw, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.party[frozenTarget].status1, 0);
        EXPECT(sSimResult.executedMask & (1u << 1));
        EXPECT_EQ(sSimAfter.party[frozenTarget].pp[0], 9);

        // Recreate the persistent branch, then verify the next turn retains
        // the exact 80/20 frozen-versus-thaw distribution.
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &freezeNoThaw, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        sSimBefore = sSimAfter;
        memset(&sSimTurn, 0, sizeof(sSimTurn));
        SetPureAction(1, 0, MOVE_TACKLE, 0);
        count = AiSim_EnumerateOutcomes(&sSimContext, &sSimBefore, &sSimTurn,
                                        sSimOutcomes, ARRAY_COUNT(sSimOutcomes));
        EXPECT(count > 2 && count <= AI_SIM_MAX_OUTCOMES);
        EXPECT_EQ(SumOutcomeWeights(sSimOutcomes, count, 0,
                                    1u << (1 + AI_SIM_OUTCOME_THAW_SHIFT), 0),
                  SumOutcomeWeights(sSimOutcomes, count,
                                    1u << (1 + AI_SIM_OUTCOME_THAW_SHIFT), 0, 0) * 4);
        for (index = 0; index < count; index++)
        {
            if (sSimOutcomes[index].flags & (1u << (1 + AI_SIM_OUTCOME_THAW_SHIFT)))
                thawIndex = index;
            else
                frozenIndex = index;
        }
        EXPECT(frozenIndex != UINT32_MAX);
        EXPECT(thawIndex != UINT32_MAX);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &sSimOutcomes[frozenIndex], &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT(sSimAfter.party[frozenTarget].status1 & STATUS1_FREEZE);
        EXPECT(sSimResult.skippedMask & (1u << 1));
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &sSimOutcomes[thawIndex], &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.party[frozenTarget].status1, 0);
        EXPECT(sSimResult.executedMask & (1u << 1));

        // Four simultaneous Ice Beams have a large raw Cartesian product.
        // At one HP every executed beam is a capped KO, so damage, critical,
        // secondary, and thaw choices are post-Apply equivalent.  The 24
        // exact equal-Speed orders merge into four distinct survivor boards,
        // each with the same total probability.
        InitPureDoubles();
        for (index = 0; index < MAX_BATTLERS_COUNT; index++)
        {
            sSimContext.mons[sRosterByBattler[index]].normal.maxHp = 1;
            sSimBefore.party[sRosterByBattler[index]].hp = 1;
        }
        SetPureAction(0, 0, MOVE_ICE_BEAM, 1);
        SetPureAction(1, 0, MOVE_ICE_BEAM, 0);
        SetPureAction(2, 0, MOVE_ICE_BEAM, 3);
        SetPureAction(3, 0, MOVE_ICE_BEAM, 2);
        EXPECT_EQ(AiSim_EnumerateOutcomes(&sSimContext, &sSimBefore, &sSimTurn,
                                          sSimOutcomes, ARRAY_COUNT(sSimOutcomes)),
                  4);
        EXPECT_EQ(SumOutcomeWeights(sSimOutcomes, 4, 0, 0, 0), 4);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator resolves no-reserve Parting Shot and fails closed on an unencoded pivot")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 beforeHash;
        u32 unsupported = 0;

        InitPureDoubles();
        SetPureAction(0, 0, MOVE_PARTING_SHOT, 1);
        beforeHash = HashBytes(2166136261, &sSimBefore, sizeof(sSimBefore));
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT_EQ(HashBytes(2166136261, &sSimBefore, sizeof(sSimBefore)), beforeHash);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_ATK], DEFAULT_STAT_STAGE - 1);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_SPATK], DEFAULT_STAT_STAGE - 1);
        EXPECT_EQ(sSimAfter.active[0].rosterIndex, sRosterByBattler[0]);
        EXPECT(!(sSimResult.events & AI_SIM_EVENT_SWITCH));
        EXPECT(sSimResult.events & AI_SIM_EVENT_STAT_CHANGE);

        InitPureDoubles();
        sSimBefore.sides[B_SIDE_OPPONENT].statuses |= SIDE_STATUS_MIST;
        sSimBefore.sides[B_SIDE_OPPONENT].mistTimer = 5;
        SetPureAction(0, 0, MOVE_PARTING_SHOT, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_ATK], DEFAULT_STAT_STAGE);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_SPATK], DEFAULT_STAT_STAGE);

        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[1]].normal.speed = 200;
        SetPureAction(0, 0, MOVE_PARTING_SHOT, 1);
        SetPureAction(1, 0, MOVE_PROTECT, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT(sSimResult.events & AI_SIM_EVENT_PROTECTED);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_ATK], DEFAULT_STAT_STAGE);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_SPATK], DEFAULT_STAT_STAGE);

        InitPureDoubles();
        SetPureAction(0, 0, MOVE_PARTING_SHOT, 1);
        sSimBefore.active[0].statStages[STAT_ACC] = DEFAULT_STAT_STAGE - 1;
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &unsupported), AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_OUTCOME);

        InitPureDoubles();
        SetPureAction(0, 0, MOVE_PARTING_SHOT, 1);
        sSimBefore.active[1].statStages[STAT_EVASION] = DEFAULT_STAT_STAGE + 1;
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &unsupported), AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_OUTCOME);

        InitPureDoubles();
        InitPureMon(2, B_SIDE_PLAYER, 2);
        SetPureAction(0, 0, MOVE_PARTING_SHOT, 1);
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &unsupported), AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_SWITCH_IN_EFFECT);

        sSimBefore.party[2].hp = 0;
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &unsupported), AI_SIM_APPLY_OK);
        EXPECT_EQ(unsupported, AI_SIM_UNSUPPORTED_NONE);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator applies Psychic Terrain controls to Prankster Parting Shot")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 actor = sRosterByBattler[0];
        u32 target = sRosterByBattler[1];

        // Psychic Terrain blocks an elevated targeted status move against a
        // grounded foe without reporting a Protect interaction.
        InitPureDoubles();
        sSimContext.mons[actor].normal.ability = ABILITY_PRANKSTER;
        sSimBefore.fieldStatuses |= STATUS_FIELD_PSYCHIC_TERRAIN;
        sSimBefore.terrainTimer = 5;
        SetPureAction(0, 0, MOVE_PARTING_SHOT, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_ATK], DEFAULT_STAT_STAGE);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_SPATK], DEFAULT_STAT_STAGE);
        EXPECT(!(sSimResult.events & AI_SIM_EVENT_PROTECTED));
        EXPECT_EQ(sSimResult.readInteractionFlags[1] & AI_READ_INTERACTION_PROTECT, 0);

        // An ungrounded target remains affected by the same Prankster move.
        InitPureDoubles();
        sSimContext.mons[actor].normal.ability = ABILITY_PRANKSTER;
        sSimContext.mons[target].normal.types[0] = TYPE_FLYING;
        sSimBefore.fieldStatuses |= STATUS_FIELD_PSYCHIC_TERRAIN;
        sSimBefore.terrainTimer = 5;
        SetPureAction(0, 0, MOVE_PARTING_SHOT, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_ATK], DEFAULT_STAT_STAGE - 1);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_SPATK], DEFAULT_STAT_STAGE - 1);

        // Terrain does not block an ordinary priority-zero Parting Shot.
        InitPureDoubles();
        sSimBefore.fieldStatuses |= STATUS_FIELD_PSYCHIC_TERRAIN;
        sSimBefore.terrainTimer = 5;
        SetPureAction(0, 0, MOVE_PARTING_SHOT, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_ATK], DEFAULT_STAT_STAGE - 1);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_SPATK], DEFAULT_STAT_STAGE - 1);

        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator gates Prankster Parting Shot Dark immunity by generation")
{
    u32 genConfig;

    PARAMETRIZE { genConfig = GEN_6; }
    PARAMETRIZE { genConfig = GEN_7; }
    GIVEN {
        WITH_CONFIG(B_PRANKSTER_DARK_TYPES, genConfig);
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 actor = sRosterByBattler[0];
        u32 target = sRosterByBattler[1];
        bool32 darkTypesBlock = genConfig >= GEN_7;

        InitPureDoubles();
        sSimContext.mons[actor].normal.ability = ABILITY_PRANKSTER;
        sSimContext.mons[target].normal.types[0] = TYPE_DARK;
        SetPureAction(0, 0, MOVE_PARTING_SHOT, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_ATK],
                  darkTypesBlock ? DEFAULT_STAT_STAGE : DEFAULT_STAT_STAGE - 1);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_SPATK],
                  darkTypesBlock ? DEFAULT_STAT_STAGE : DEFAULT_STAT_STAGE - 1);

        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator uses current Tera type for Prankster Parting Shot immunity")
{
    GIVEN {
        WITH_CONFIG(B_PRANKSTER_DARK_TYPES, GEN_7);
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 actor = sRosterByBattler[0];
        u32 target = sRosterByBattler[1];

        InitPureDoubles();
        sSimContext.mons[actor].normal.ability = ABILITY_PRANKSTER;
        sSimContext.mons[target].teraType = TYPE_DARK;
        sSimBefore.active[1].flags |= AI_SIM_ACTIVE_TERA;
        SetPureAction(0, 0, MOVE_PARTING_SHOT, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_ATK], DEFAULT_STAT_STAGE);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_SPATK], DEFAULT_STAT_STAGE);

        // Terastallizing away from Dark removes the immunity.
        InitPureDoubles();
        sSimContext.mons[actor].normal.ability = ABILITY_PRANKSTER;
        sSimContext.mons[target].normal.types[0] = TYPE_DARK;
        sSimContext.mons[target].teraType = TYPE_FAIRY;
        sSimBefore.active[1].flags |= AI_SIM_ACTIVE_TERA;
        SetPureAction(0, 0, MOVE_PARTING_SHOT, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_ATK], DEFAULT_STAT_STAGE - 1);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_SPATK], DEFAULT_STAT_STAGE - 1);

        // Dark typing alone does not block a non-Prankster move.
        InitPureDoubles();
        sSimContext.mons[target].normal.types[0] = TYPE_DARK;
        SetPureAction(0, 0, MOVE_PARTING_SHOT, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_ATK], DEFAULT_STAT_STAGE - 1);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_SPATK], DEFAULT_STAT_STAGE - 1);

        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator records Protect before Prankster Dark blocking")
{
    GIVEN {
        WITH_CONFIG(B_PRANKSTER_DARK_TYPES, GEN_7);
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 actor = sRosterByBattler[0];
        u32 target = sRosterByBattler[1];

        // Protect resolves before terrain/type blockers and retains its read
        // evidence even when the target would also be Dark-immune in Gen 7.
        InitPureDoubles();
        sSimContext.mons[actor].normal.ability = ABILITY_PRANKSTER;
        sSimContext.mons[target].normal.types[0] = TYPE_DARK;
        SetPureAction(0, 0, MOVE_PARTING_SHOT, 1);
        SetPureAction(1, 0, MOVE_PROTECT, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_ATK], DEFAULT_STAT_STAGE);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_SPATK], DEFAULT_STAT_STAGE);
        EXPECT(sSimResult.events & AI_SIM_EVENT_PROTECTED);
        EXPECT(sSimResult.readInteractionFlags[1] & AI_READ_INTERACTION_PROTECT);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator branches Flare Blitz burn and applies damage recoil and residual ordering")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 target = sRosterByBattler[1];
        u32 baselineTackleDamage;
        u32 flareDamage;
        u32 burnTackleDamage;
        u32 recoil;
        u32 beforeHash;
        struct AiSimOutcomeKey noBurn;
        struct AiSimOutcomeKey burn;

        InitPureDoubles();
        SetPureAction(1, 0, MOVE_TACKLE, 0);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        baselineTackleDamage = sSimResult.damage[1];

        InitPureDoubles();
        SetPureAction(0, 0, MOVE_FLARE_BLITZ, 1);
        beforeHash = HashBytes(2166136261, &sSimBefore, sizeof(sSimBefore));
        EXPECT_EQ(AiSim_EnumerateOutcomes(&sSimContext, &sSimBefore, &sSimTurn,
                                          sSimOutcomes, ARRAY_COUNT(sSimOutcomes)),
                  AI_SIM_MAX_OUTCOMES + 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_UNSUPPORTED);

        noBurn = MakePureOutcome(0);
        burn = MakePureOutcome(1u << 0);

        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &noBurn, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT_EQ(HashBytes(2166136261, &sSimBefore, sizeof(sSimBefore)), beforeHash);
        flareDamage = sSimResult.damage[0];
        recoil = max(1, flareDamage * gMovesInfo[MOVE_FLARE_BLITZ].argument.recoilPercentage / 100);
        EXPECT(flareDamage > 0);
        EXPECT_EQ(sSimAfter.party[sRosterByBattler[0]].hp, 200 - recoil);
        EXPECT_EQ(sSimAfter.party[target].status1, 0);
        EXPECT(sSimResult.events & AI_SIM_EVENT_RECOIL);

        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &burn, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT(sSimAfter.party[target].status1 & STATUS1_BURN);
#if B_BURN_DAMAGE >= GEN_7 || B_BURN_DAMAGE == GEN_1
        EXPECT_EQ(sSimAfter.party[target].hp, 200 - flareDamage - 200 / 16);
#else
        EXPECT_EQ(sSimAfter.party[target].hp, 200 - flareDamage - 200 / 8);
#endif
        EXPECT(sSimResult.events & AI_SIM_EVENT_STATUS_CHANGE);

        sSimBefore = sSimAfter;
        memset(&sSimTurn, 0, sizeof(sSimTurn));
        SetPureAction(1, 0, MOVE_TACKLE, 0);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        burnTackleDamage = sSimResult.damage[1];
        EXPECT(burnTackleDamage < baselineTackleDamage);
        EXPECT(sSimAfter.party[target].status1 & STATUS1_BURN);

        InitPureDoubles();
        sSimBefore.party[sRosterByBattler[0]].hp = 105;
        sSimBefore.party[sRosterByBattler[0]].item = ITEM_SITRUS_BERRY;
        SetPureAction(0, 0, MOVE_FLARE_BLITZ, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &noBurn, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.party[sRosterByBattler[0]].hp, 105 - recoil + 50);
        EXPECT_EQ(sSimAfter.party[sRosterByBattler[0]].item, ITEM_NONE);
        EXPECT(sSimAfter.party[sRosterByBattler[0]].flags & AI_SIM_PARTY_ITEM_CONSUMED);

        InitPureDoubles();
        sSimBefore.party[sRosterByBattler[0]].hp = recoil;
        SetPureAction(0, 0, MOVE_FLARE_BLITZ, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &noBurn, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.party[sRosterByBattler[0]].hp, 0);
        EXPECT(sSimAfter.active[0].flags & AI_SIM_ACTIVE_NEEDS_REPLACEMENT);
        EXPECT(sSimResult.events & AI_SIM_EVENT_KO);

        InitPureDoubles();
        sSimContext.mons[target].normal.types[0] = TYPE_FIRE;
        SetPureAction(0, 0, MOVE_FLARE_BLITZ, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &burn, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.party[target].status1, 0);

        InitPureDoubles();
        sSimBefore.party[target].status1 = STATUS1_FREEZE;
        SetPureAction(0, 0, MOVE_FLARE_BLITZ, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &burn, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.party[target].status1, 0);

        InitPureDoubles();
        sSimBefore.sides[B_SIDE_OPPONENT].statuses |= SIDE_STATUS_SAFEGUARD;
        sSimBefore.sides[B_SIDE_OPPONENT].safeguardTimer = 5;
        SetPureAction(0, 0, MOVE_FLARE_BLITZ, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &burn, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.party[target].status1, 0);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator validates and resolves damaging Z actions from their base move")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 actor = sRosterByBattler[0];
        u32 ordinaryDamage;
        u32 unsupported = 0;

        InitPureDoubles();
        SetPureAction(0, 0, MOVE_TACKLE, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        ordinaryDamage = sSimResult.damage[0];

        InitPureDoubles();
        SetPureMove(0, 0, MOVE_TACKLE);
        sSimBefore.party[actor].item = ITEM_NORMALIUM_Z;
        sSimContext.mons[actor].eligibleGimmicks |= 1u << GIMMICK_Z_MOVE;
        sSimTurn.actions[0] = (struct AiSimAction)
        {
            .choice = MOVE_BREAKNECK_BLITZ,
            .actor = 0,
            .target = 1,
            .kind = AI_SIM_ACTION_MOVE,
            .moveSlot = 0,
            .gimmick = GIMMICK_Z_MOVE,
        };
        sSimTurn.actionMask = 1;
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &unsupported), AI_SIM_APPLY_OK);
        EXPECT_EQ(unsupported, AI_SIM_UNSUPPORTED_NONE);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT(sSimResult.damage[0] > ordinaryDamage * 2);
        EXPECT_EQ(sSimAfter.party[actor].pp[0], 9);
        EXPECT_EQ(sSimAfter.party[actor].item, ITEM_NORMALIUM_Z);
        EXPECT(sSimAfter.trainerGimmickUsed[B_SIDE_PLAYER] & (1u << GIMMICK_Z_MOVE));

        sSimTurn.actions[0].choice = MOVE_TWINKLE_TACKLE;
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &unsupported), AI_SIM_APPLY_INVALID);

        InitPureDoubles();
        SetPureMove(0, 0, MOVE_MOONBLAST);
        sSimContext.mons[actor].normal.attack = 1;
        sSimContext.mons[actor].normal.spAttack = 200;
        sSimContext.mons[sRosterByBattler[1]].normal.defense = 1000;
        sSimContext.mons[sRosterByBattler[1]].normal.spDefense = 100;
        sSimBefore.party[actor].item = ITEM_FAIRIUM_Z;
        sSimContext.mons[actor].eligibleGimmicks |= 1u << GIMMICK_Z_MOVE;
        sSimTurn.actions[0] = (struct AiSimAction)
        {
            .choice = MOVE_TWINKLE_TACKLE,
            .actor = 0,
            .target = 1,
            .kind = AI_SIM_ACTION_MOVE,
            .moveSlot = 0,
            .gimmick = GIMMICK_Z_MOVE,
        };
        sSimTurn.actionMask = 1;
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT(sSimResult.damage[0] > 50);

        InitPureDoubles();
        SetPureMove(0, 0, MOVE_PROTECT);
        sSimBefore.party[actor].item = ITEM_NORMALIUM_Z;
        sSimContext.mons[actor].eligibleGimmicks |= 1u << GIMMICK_Z_MOVE;
        sSimTurn.actions[0] = (struct AiSimAction)
        {
            .choice = MOVE_BREAKNECK_BLITZ,
            .actor = 0,
            .target = 1,
            .kind = AI_SIM_ACTION_MOVE,
            .moveSlot = 0,
            .gimmick = GIMMICK_Z_MOVE,
        };
        sSimTurn.actionMask = 1;
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &unsupported), AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_MOVE_EFFECT);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator projects activated Mega and Tera profiles and fails closed on unknown forms")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 actor = sRosterByBattler[0];
        struct SimulatedDamage baseDamage;
        struct SimulatedDamage gimmickDamage;
        u32 unsupported = 0;

        InitPureDoubles();
        SetPureAction(0, 0, MOVE_TACKLE, 1);
        EXPECT(AiSim_ProjectDamage(&sSimContext, &sSimBefore,
                                   &sSimTurn.actions[0], 1, &baseDamage));
        sSimContext.mons[actor].transformed = sSimContext.mons[actor].normal;
        sSimContext.mons[actor].transformed.attack = 300;
        sSimContext.mons[actor].transformationGimmick = GIMMICK_MEGA;
        sSimContext.mons[actor].eligibleGimmicks |= 1u << GIMMICK_MEGA;
        sSimTurn.actions[0].gimmick = GIMMICK_MEGA;
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       &unsupported), AI_SIM_APPLY_OK);
        EXPECT_EQ(unsupported, AI_SIM_UNSUPPORTED_NONE);
        EXPECT(AiSim_ProjectDamage(&sSimContext, &sSimBefore,
                                   &sSimTurn.actions[0], 1, &gimmickDamage));
        EXPECT_GT(gimmickDamage.median, baseDamage.median);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimResult.damage[0], gimmickDamage.median);
        EXPECT(sSimAfter.active[0].flags & AI_SIM_ACTIVE_TRANSFORMED);

        InitPureDoubles();
        SetPureAction(0, 0, MOVE_TACKLE, 1);
        sSimContext.mons[actor].transformed = sSimContext.mons[actor].normal;
        sSimContext.mons[actor].transformed.ability = ABILITY_WONDER_GUARD;
        sSimContext.mons[actor].transformationGimmick = GIMMICK_MEGA;
        sSimContext.mons[actor].eligibleGimmicks |= 1u << GIMMICK_MEGA;
        sSimTurn.actions[0].gimmick = GIMMICK_MEGA;
        unsupported = 0;
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       &unsupported), AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_GIMMICK);
        gimmickDamage = (struct SimulatedDamage){1, 1, 1, 1};
        EXPECT(!AiSim_ProjectDamage(&sSimContext, &sSimBefore,
                                    &sSimTurn.actions[0], 1, &gimmickDamage));
        EXPECT_EQ(gimmickDamage.minimum, 0);
        EXPECT_EQ(gimmickDamage.median, 0);
        EXPECT_EQ(gimmickDamage.maximum, 0);
        EXPECT_EQ(gimmickDamage.random, 0);

        // A form-change entry effect must fail closed even when the selected
        // move itself performs no damage calculation.
        InitPureDoubles();
        SetPureAction(0, 0, MOVE_PROTECT, 0);
        sSimContext.mons[actor].transformed = sSimContext.mons[actor].normal;
        sSimContext.mons[actor].transformed.ability = ABILITY_INTIMIDATE;
        sSimContext.mons[actor].transformationGimmick = GIMMICK_MEGA;
        sSimContext.mons[actor].eligibleGimmicks |= 1u << GIMMICK_MEGA;
        sSimTurn.actions[0].gimmick = GIMMICK_MEGA;
        unsupported = 0;
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       &unsupported), AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_GIMMICK);

        InitPureDoubles();
        SetPureAction(0, 0, MOVE_TACKLE, 1);
        EXPECT(AiSim_ProjectDamage(&sSimContext, &sSimBefore,
                                   &sSimTurn.actions[0], 1, &baseDamage));
        sSimContext.mons[actor].teraType = TYPE_NORMAL;
        sSimContext.mons[actor].eligibleGimmicks |= 1u << GIMMICK_TERA;
        sSimTurn.actions[0].gimmick = GIMMICK_TERA;
        unsupported = 0;
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       &unsupported), AI_SIM_APPLY_OK);
        EXPECT(AiSim_ProjectDamage(&sSimContext, &sSimBefore,
                                   &sSimTurn.actions[0], 1, &gimmickDamage));
        EXPECT_GT(gimmickDamage.median, baseDamage.median);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimResult.damage[0], gimmickDamage.median);
        EXPECT(sSimAfter.active[0].flags & AI_SIM_ACTIVE_TERA);
        EXPECT_EQ(sSimAfter.party[actor].activeGimmick, GIMMICK_TERA);

        InitPureDoubles();
        SetPureAction(0, 0, MOVE_TACKLE, 1);
        sSimContext.mons[actor].teraType = TYPE_STELLAR;
        sSimContext.mons[actor].eligibleGimmicks |= 1u << GIMMICK_TERA;
        sSimTurn.actions[0].gimmick = GIMMICK_TERA;
        unsupported = 0;
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       &unsupported), AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_GIMMICK);
        gimmickDamage = (struct SimulatedDamage){1, 1, 1, 1};
        EXPECT(!AiSim_ProjectDamage(&sSimContext, &sSimBefore,
                                    &sSimTurn.actions[0], 1, &gimmickDamage));
        EXPECT_EQ(gimmickDamage.maximum, 0);

        InitPureDoubles();
        SetPureAction(0, 0, MOVE_PROTECT, 0);
        sSimContext.mons[actor].teraType = TYPE_STELLAR;
        sSimContext.mons[actor].eligibleGimmicks |= 1u << GIMMICK_TERA;
        sSimTurn.actions[0].gimmick = GIMMICK_TERA;
        unsupported = 0;
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       &unsupported), AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_GIMMICK);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator applies the Tera power floor after Technician for planned and active Tera")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        struct SimulatedDamage baseDamage;
        struct SimulatedDamage plannedDamage;
        struct SimulatedDamage activeDamage;
        u32 actor = sRosterByBattler[0];

        EXPECT_EQ((u32)gMovesInfo[MOVE_TACKLE].power, 40);
        InitPureDoubles();
        SetPureAction(0, 0, MOVE_TACKLE, 1);
        EXPECT(AiSim_CalcDamage(&sSimContext, &sSimBefore,
                                &sSimTurn.actions[0], 1, &baseDamage));
        EXPECT_EQ(baseDamage.maximum, 28);

        sSimContext.mons[actor].teraType = TYPE_NORMAL;
        sSimContext.mons[actor].eligibleGimmicks |= 1u << GIMMICK_TERA;
        sSimTurn.actions[0].gimmick = GIMMICK_TERA;
        EXPECT(AiSim_CalcDamage(&sSimContext, &sSimBefore,
                                &sSimTurn.actions[0], 1, &plannedDamage));
        EXPECT(AiSim_ProjectDamage(&sSimContext, &sSimBefore,
                                   &sSimTurn.actions[0], 1, &activeDamage));
        EXPECT_EQ(plannedDamage.maximum, 56);
        EXPECT_EQ(activeDamage.maximum, plannedDamage.maximum);

        sSimTurn.actions[0].gimmick = GIMMICK_NONE;
        sSimBefore.active[0].flags |= AI_SIM_ACTIVE_TERA;
        sSimBefore.party[actor].activeGimmick = GIMMICK_TERA;
        EXPECT(AiSim_CalcDamage(&sSimContext, &sSimBefore,
                                &sSimTurn.actions[0], 1, &activeDamage));
        EXPECT_EQ(activeDamage.maximum, plannedDamage.maximum);

        // Technician raises 40 BP to exactly 60 before the floor check.  The
        // floor must not run first and then receive a second 1.5x boost.
        InitPureDoubles();
        sSimContext.mons[actor].normal.ability = ABILITY_TECHNICIAN;
        sSimContext.mons[actor].teraType = TYPE_NORMAL;
        SetPureAction(0, 0, MOVE_TACKLE, 1);
        EXPECT(AiSim_CalcDamage(&sSimContext, &sSimBefore,
                                &sSimTurn.actions[0], 1, &baseDamage));
        EXPECT_EQ(baseDamage.maximum, 42);
        sSimTurn.actions[0].gimmick = GIMMICK_TERA;
        EXPECT(AiSim_CalcDamage(&sSimContext, &sSimBefore,
                                &sSimTurn.actions[0], 1, &plannedDamage));
        EXPECT_EQ(plannedDamage.maximum, 56);

        // The inclusive Technician boundary is also a base-power modifier:
        // 60 BP Aerial Ace becomes 90 before the +2 damage-formula term.
        EXPECT_EQ((u32)gMovesInfo[MOVE_AERIAL_ACE].power, 60);
        InitPureDoubles();
        sSimContext.mons[actor].normal.types[0] = TYPE_FLYING;
        sSimContext.mons[actor].normal.ability = ABILITY_TECHNICIAN;
        sSimContext.mons[actor].teraType = TYPE_FLYING;
        SetPureAction(0, 0, MOVE_AERIAL_ACE, 1);
        EXPECT(AiSim_CalcDamage(&sSimContext, &sSimBefore,
                                &sSimTurn.actions[0], 1, &baseDamage));
        EXPECT_EQ(baseDamage.maximum, 61);
        sSimTurn.actions[0].gimmick = GIMMICK_TERA;
        EXPECT(AiSim_CalcDamage(&sSimContext, &sSimBefore,
                                &sSimTurn.actions[0], 1, &plannedDamage));
        EXPECT_EQ(plannedDamage.maximum, 82);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator applies Knock Off power and removal to foreign form items only")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        struct SimulatedDamage damage;
        u32 target = sRosterByBattler[1];

        EXPECT_EQ((u32)gMovesInfo[MOVE_KNOCK_OFF].power, 65);
        InitPureDoubles();
        sSimBefore.party[target].item = ITEM_SCIZORITE;
        SetPureAction(0, 0, MOVE_KNOCK_OFF, 1);
        EXPECT(AiSim_IsItemRemovable(&sSimContext, &sSimBefore, 1));
        EXPECT(AiSim_CalcDamage(&sSimContext, &sSimBefore,
                                &sSimTurn.actions[0], 1, &damage));
#if B_KNOCK_OFF_DMG >= GEN_6
        EXPECT_EQ(damage.maximum, 44);
#else
        EXPECT_EQ(damage.maximum, 30);
#endif
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.party[target].item, ITEM_NONE);

        InitPureDoubles();
        sSimContext.mons[target].normal.species = SPECIES_SCIZOR;
        sSimBefore.party[target].item = ITEM_SCIZORITE;
        SetPureAction(0, 0, MOVE_KNOCK_OFF, 1);
        EXPECT(!AiSim_IsItemRemovable(&sSimContext, &sSimBefore, 1));
        EXPECT(AiSim_CalcDamage(&sSimContext, &sSimBefore,
                                &sSimTurn.actions[0], 1, &damage));
        EXPECT_EQ(damage.maximum, 30);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.party[target].item, ITEM_SCIZORITE);

        InitPureDoubles();
        sSimBefore.party[target].item = ITEM_BLUE_ORB;
        EXPECT(AiSim_IsItemRemovable(&sSimContext, &sSimBefore, 1));
        sSimContext.mons[target].normal.species = SPECIES_KYOGRE;
        EXPECT(!AiSim_IsItemRemovable(&sSimContext, &sSimBefore, 1));
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator fails closed on cant-use-twice move legality")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 unsupported = 0;

        InitPureDoubles();
        sSimBefore.active[0].lastMove = MOVE_GIGATON_HAMMER;
        SetPureAction(0, 0, MOVE_GIGATON_HAMMER, 1);
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       &unsupported),
                  AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_MOVE_EFFECT);

        InitPureDoubles();
        unsupported = 0;
        sSimBefore.active[0].lastMove = MOVE_BLOOD_MOON;
        SetPureAction(0, 0, MOVE_BLOOD_MOON, 1);
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       &unsupported),
                  AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_MOVE_EFFECT);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator fails closed on target-ability-ignoring moves and signature Z moves")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 actor = sRosterByBattler[0];
        u32 target = sRosterByBattler[1];
        u32 unsupported = 0;

        InitPureDoubles();
        sSimContext.mons[target].normal.ability = ABILITY_STURDY;
        SetPureAction(0, 0, MOVE_SUNSTEEL_STRIKE, 1);
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       &unsupported),
                  AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_MOVE_EFFECT);

        InitPureDoubles();
        unsupported = 0;
        sSimContext.mons[actor].normal.species = SPECIES_SOLGALEO;
        sSimContext.mons[target].normal.ability = ABILITY_PRISM_ARMOR;
        SetPureMove(0, 0, MOVE_SUNSTEEL_STRIKE);
        sSimBefore.party[actor].item = ITEM_SOLGANIUM_Z;
        sSimContext.mons[actor].eligibleGimmicks |= 1u << GIMMICK_Z_MOVE;
        sSimTurn.actions[0] = (struct AiSimAction)
        {
            .choice = MOVE_SEARING_SUNRAZE_SMASH,
            .actor = 0,
            .target = 1,
            .kind = AI_SIM_ACTION_MOVE,
            .moveSlot = 0,
            .gimmick = GIMMICK_Z_MOVE,
        };
        sSimTurn.actionMask = 1;
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       &unsupported),
                  AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_MOVE_EFFECT);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator fails closed on target-defense-stage-ignoring moves")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 unsupported = 0;

        InitPureDoubles();
        sSimBefore.active[1].statStages[STAT_DEF] = MAX_STAT_STAGE;
        SetPureAction(0, 0, MOVE_SACRED_SWORD, 1);
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       &unsupported),
                  AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_MOVE_EFFECT);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator preserves Z priority protection immunity and Dynamax choice rules")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 actor = sRosterByBattler[0];
        u32 target = sRosterByBattler[1];
        struct SimulatedDamage rawDamage;
        struct SimulatedDamage protectedDamage;
        struct SimulatedDamage noChoiceDamage;
        struct SimulatedDamage choiceDamage;
        u32 hpBefore;
        u32 scarfSpeed;
        u32 unsupported;
        u32 boardHash;

        // Z-Moves do not retain their base move's priority, so neither
        // Psychic Terrain nor Quick Guard blocks Z-Quick Attack.
        InitPureDoubles();
        SetPureMove(0, 0, MOVE_QUICK_ATTACK);
        sSimBefore.party[actor].item = ITEM_NORMALIUM_Z;
        sSimContext.mons[actor].eligibleGimmicks |= 1u << GIMMICK_Z_MOVE;
        sSimTurn.actions[0] = (struct AiSimAction)
        {
            .choice = MOVE_BREAKNECK_BLITZ,
            .actor = 0,
            .target = 1,
            .kind = AI_SIM_ACTION_MOVE,
            .moveSlot = 0,
            .gimmick = GIMMICK_Z_MOVE,
        };
        sSimTurn.actionMask = 1;
        sSimBefore.fieldStatuses |= STATUS_FIELD_PSYCHIC_TERRAIN;
        sSimBefore.sides[B_SIDE_OPPONENT].flags |= AI_SIM_SIDE_QUICK_GUARD;
        EXPECT(AiSim_ProjectDamage(&sSimContext, &sSimBefore,
                                   &sSimTurn.actions[0], 1, &rawDamage));
        EXPECT_GT(rawDamage.median, 0);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT_GT(sSimResult.damage[0], 0);

        // Ordinary Protect reduces a nonimmune Z-Move to exact quarter chip.
        InitPureDoubles();
        SetPureMove(0, 0, MOVE_TACKLE);
        sSimBefore.party[actor].item = ITEM_NORMALIUM_Z;
        sSimContext.mons[actor].eligibleGimmicks |= 1u << GIMMICK_Z_MOVE;
        sSimTurn.actions[0] = (struct AiSimAction)
        {
            .choice = MOVE_BREAKNECK_BLITZ,
            .actor = 0,
            .target = 1,
            .kind = AI_SIM_ACTION_MOVE,
            .moveSlot = 0,
            .gimmick = GIMMICK_Z_MOVE,
        };
        sSimTurn.actionMask = 1;
        EXPECT(AiSim_ProjectDamage(&sSimContext, &sSimBefore,
                                   &sSimTurn.actions[0], 1, &rawDamage));
        sSimBefore.active[1].volatileFlags |= AI_SIM_VOLATILE_PROTECTED;
        EXPECT(AiSim_ProjectDamage(&sSimContext, &sSimBefore,
                                   &sSimTurn.actions[0], 1, &protectedDamage));
        EXPECT_EQ(protectedDamage.minimum, max(1, rawDamage.minimum / 4));
        EXPECT_EQ(protectedDamage.median, max(1, rawDamage.median / 4));
        EXPECT_EQ(protectedDamage.maximum, max(1, rawDamage.maximum / 4));
        EXPECT_EQ(protectedDamage.random, protectedDamage.median);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimResult.damage[0], protectedDamage.median);

        // Protection cannot manufacture chip through a type immunity.
        InitPureDoubles();
        SetPureMove(0, 0, MOVE_TACKLE);
        sSimBefore.party[actor].item = ITEM_NORMALIUM_Z;
        sSimContext.mons[actor].eligibleGimmicks |= 1u << GIMMICK_Z_MOVE;
        sSimContext.mons[target].normal.types[0] = TYPE_GHOST;
        sSimBefore.active[1].volatileFlags |= AI_SIM_VOLATILE_PROTECTED;
        sSimTurn.actions[0] = (struct AiSimAction)
        {
            .choice = MOVE_BREAKNECK_BLITZ,
            .actor = 0,
            .target = 1,
            .kind = AI_SIM_ACTION_MOVE,
            .moveSlot = 0,
            .gimmick = GIMMICK_Z_MOVE,
        };
        sSimTurn.actionMask = 1;
        protectedDamage = (struct SimulatedDamage){1, 1, 1, 1};
        EXPECT(!AiSim_ProjectDamage(&sSimContext, &sSimBefore,
                                    &sSimTurn.actions[0], 1, &protectedDamage));
        EXPECT_EQ(protectedDamage.minimum, 0);
        EXPECT_EQ(protectedDamage.median, 0);
        EXPECT_EQ(protectedDamage.maximum, 0);
        EXPECT_EQ(protectedDamage.random, 0);
        hpBefore = sSimBefore.party[target].hp;
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimResult.damage[0], 0);
        EXPECT_EQ(sSimAfter.party[target].hp, hpBefore);

        InitPureDoubles();
        SetPureAction(0, 0, MOVE_TACKLE, 1);
        sSimBefore.active[0].volatileFlags |= AI_SIM_VOLATILE_FLINCHED;
        rawDamage = (struct SimulatedDamage){1, 1, 1, 1};
        EXPECT(!AiSim_ProjectDamage(&sSimContext, &sSimBefore,
                                    &sSimTurn.actions[0], 1, &rawDamage));
        EXPECT_EQ(rawDamage.maximum, 0);

        // Choice boosts are inactive while Dynamaxed.
        InitPureDoubles();
        SetPureAction(0, 0, MOVE_TACKLE, 1);
        sSimContext.mons[actor].normal.attack = 300;
        sSimBefore.active[0].flags |= AI_SIM_ACTIVE_DYNAMAX;
        sSimBefore.active[0].dynamaxTurns = 3;
        sSimBefore.party[actor].activeGimmick = GIMMICK_DYNAMAX;
        boardHash = HashBytes(2166136261, &sSimBefore, sizeof(sSimBefore));
        unsupported = 0;
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       &unsupported), AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_DYNAMIC_POWER);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(sSimResult.unsupportedFlags & AI_SIM_UNSUPPORTED_DYNAMIC_POWER);
        EXPECT_EQ(HashBytes(2166136261, &sSimBefore, sizeof(sSimBefore)), boardHash);
        rawDamage = (struct SimulatedDamage){1, 1, 1, 1};
        EXPECT(!AiSim_ProjectDamage(&sSimContext, &sSimBefore,
                                    &sSimTurn.actions[0], 1, &rawDamage));
        EXPECT_EQ(rawDamage.maximum, 0);
        sSimTurn.actions[0].choice = MOVE_MAX_STRIKE;
        unsupported = 0;
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       &unsupported), AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_DYNAMIC_POWER);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn,
                                       NULL, &sSimAfter, &sSimResult),
                  AI_SIM_APPLY_UNSUPPORTED);
        EXPECT_EQ(HashBytes(2166136261, &sSimBefore, sizeof(sSimBefore)), boardHash);
        rawDamage = (struct SimulatedDamage){1, 1, 1, 1};
        EXPECT(!AiSim_CalcDamage(&sSimContext, &sSimBefore,
                                 &sSimTurn.actions[0], 1, &rawDamage));
        EXPECT_EQ(rawDamage.minimum, 0);
        EXPECT_EQ(rawDamage.median, 0);
        EXPECT_EQ(rawDamage.maximum, 0);
        EXPECT_EQ(rawDamage.random, 0);
        sSimTurn.actions[0].choice = MOVE_TACKLE;
        sSimBefore.party[actor].item = ITEM_NONE;
        EXPECT(AiSim_CalcDamage(&sSimContext, &sSimBefore,
                                &sSimTurn.actions[0], 1, &noChoiceDamage));
        sSimBefore.party[actor].item = ITEM_CHOICE_BAND;
        EXPECT(AiSim_CalcDamage(&sSimContext, &sSimBefore,
                                &sSimTurn.actions[0], 1, &choiceDamage));
        EXPECT_EQ(choiceDamage.minimum, noChoiceDamage.minimum);
        EXPECT_EQ(choiceDamage.median, noChoiceDamage.median);
        EXPECT_EQ(choiceDamage.maximum, noChoiceDamage.maximum);

        SetPureAction(0, 0, MOVE_MOONBLAST, 1);
        sSimContext.mons[actor].normal.spAttack = 300;
        sSimBefore.party[actor].item = ITEM_NONE;
        EXPECT(AiSim_CalcDamage(&sSimContext, &sSimBefore,
                                &sSimTurn.actions[0], 1, &noChoiceDamage));
        sSimBefore.party[actor].item = ITEM_CHOICE_SPECS;
        EXPECT(AiSim_CalcDamage(&sSimContext, &sSimBefore,
                                &sSimTurn.actions[0], 1, &choiceDamage));
        EXPECT_EQ(choiceDamage.median, noChoiceDamage.median);

        sSimBefore.party[actor].item = ITEM_CHOICE_SCARF;
        sSimBefore.active[0].flags &= ~AI_SIM_ACTIVE_DYNAMAX;
        scarfSpeed = AiSim_GetEffectiveSpeed(&sSimContext, &sSimBefore, 0);
        sSimBefore.active[0].flags |= AI_SIM_ACTIVE_DYNAMAX;
        EXPECT_GT(scarfSpeed,
                  AiSim_GetEffectiveSpeed(&sSimContext, &sSimBefore, 0));
        EXPECT_EQ(AiSim_GetEffectiveSpeed(&sSimContext, &sSimBefore, 0),
                  sSimContext.mons[actor].normal.speed);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator applies Close Combat drops only after a successful hit")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        InitPureDoubles();
        SetPureAction(0, 0, MOVE_CLOSE_COMBAT, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT(sSimResult.damage[0] > 0);
        EXPECT_EQ(sSimAfter.active[0].statStages[STAT_DEF], DEFAULT_STAT_STAGE - 1);
        EXPECT_EQ(sSimAfter.active[0].statStages[STAT_SPDEF], DEFAULT_STAT_STAGE - 1);
        EXPECT(sSimResult.events & AI_SIM_EVENT_STAT_CHANGE);

        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[1]].normal.speed = 200;
        SetPureAction(0, 0, MOVE_CLOSE_COMBAT, 1);
        SetPureAction(1, 0, MOVE_PROTECT, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT(sSimResult.events & AI_SIM_EVENT_PROTECTED);
        EXPECT_EQ(sSimResult.damage[0], 0);
        EXPECT_EQ(sSimAfter.active[0].statStages[STAT_DEF], DEFAULT_STAT_STAGE);
        EXPECT_EQ(sSimAfter.active[0].statStages[STAT_SPDEF], DEFAULT_STAT_STAGE);

        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[1]].normal.speed = 200;
        SetPureAction(0, 0, MOVE_CLOSE_COMBAT, 1);
        SetPureAction(1, 0, MOVE_FAKE_OUT, 0);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT(sSimResult.skippedMask & (1u << 0));
        EXPECT_EQ(sSimAfter.active[0].statStages[STAT_DEF], DEFAULT_STAT_STAGE);
        EXPECT_EQ(sSimAfter.active[0].statStages[STAT_SPDEF], DEFAULT_STAT_STAGE);
        FreePureFixture();
    }
}

SINGLE_BATTLE_TEST("AI board simulator steals Spectral Thief boosts before damage and remaining action order")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 baselineDamage;
        u32 beforeHash;
        u32 unsupported = 0;
        struct SimulatedDamage projectedDamage;

        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[1]].normal.types[0] = TYPE_FIGHTING;
        SetPureAction(0, 0, MOVE_SPECTRAL_THIEF, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        baselineDamage = sSimResult.damage[0];
        EXPECT(baselineDamage > 0);

        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[0]].normal.speed = 200;
        sSimContext.mons[sRosterByBattler[1]].normal.speed = 80;
        sSimContext.mons[sRosterByBattler[2]].normal.speed = 120;
        sSimContext.mons[sRosterByBattler[1]].normal.types[0] = TYPE_FIGHTING;
        sSimBefore.active[1].statStages[STAT_ATK] = DEFAULT_STAT_STAGE + 2;
        sSimBefore.active[1].statStages[STAT_DEF] = DEFAULT_STAT_STAGE + 2;
        sSimBefore.active[1].statStages[STAT_SPEED] = DEFAULT_STAT_STAGE + 2;
        SetPureAction(0, 0, MOVE_SPECTRAL_THIEF, 1);
        SetPureAction(1, 0, MOVE_TACKLE, 0);
        SetPureAction(2, 0, MOVE_TACKLE, 1);
        beforeHash = HashBytes(2166136261, &sSimBefore, sizeof(sSimBefore));
        EXPECT(AiSim_ProjectDamage(&sSimContext, &sSimBefore,
                                   &sSimTurn.actions[0], 1, &projectedDamage));
        EXPECT_EQ(HashBytes(2166136261, &sSimBefore, sizeof(sSimBefore)), beforeHash);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT_EQ(HashBytes(2166136261, &sSimBefore, sizeof(sSimBefore)), beforeHash);
        EXPECT_EQ(sSimResult.damage[0], projectedDamage.median);
        EXPECT(sSimResult.damage[0] > baselineDamage);
        EXPECT_EQ(sSimAfter.active[0].statStages[STAT_ATK], DEFAULT_STAT_STAGE + 2);
        EXPECT_EQ(sSimAfter.active[0].statStages[STAT_DEF], DEFAULT_STAT_STAGE + 2);
        EXPECT_EQ(sSimAfter.active[0].statStages[STAT_SPEED], DEFAULT_STAT_STAGE + 2);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_ATK], DEFAULT_STAT_STAGE);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_DEF], DEFAULT_STAT_STAGE);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_SPEED], DEFAULT_STAT_STAGE);
        EXPECT_EQ(sSimResult.actionOrder[0], 0);
        EXPECT_EQ(sSimResult.actionOrder[1], 2);
        EXPECT_EQ(sSimResult.actionOrder[2], 1);
        EXPECT(sSimResult.events & AI_SIM_EVENT_STAT_CHANGE);

        InitPureDoubles();
        sSimBefore.active[0].statStages[STAT_ATK] = MAX_STAT_STAGE;
        sSimBefore.active[1].statStages[STAT_ATK] = DEFAULT_STAT_STAGE + 2;
        sSimContext.mons[sRosterByBattler[1]].normal.types[0] = TYPE_FIGHTING;
        SetPureAction(0, 0, MOVE_SPECTRAL_THIEF, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimAfter.active[0].statStages[STAT_ATK], MAX_STAT_STAGE);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_ATK], DEFAULT_STAT_STAGE + 2);

        InitPureDoubles();
        sSimBefore.active[1].statStages[STAT_ATK] = DEFAULT_STAT_STAGE + 2;
        sSimBefore.active[1].statStages[STAT_DEF] = DEFAULT_STAT_STAGE + 2;
        SetPureAction(0, 0, MOVE_SPECTRAL_THIEF, 1);
        EXPECT_EQ(AiSim_ApplyJointTurn(&sSimContext, &sSimBefore, &sSimTurn, NULL, &sSimAfter, &sSimResult), AI_SIM_APPLY_OK);
        EXPECT_EQ(sSimResult.damage[0], 0);
        EXPECT_EQ(sSimAfter.active[0].statStages[STAT_ATK], DEFAULT_STAT_STAGE);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_ATK], DEFAULT_STAT_STAGE + 2);
        EXPECT_EQ(sSimAfter.active[1].statStages[STAT_DEF], DEFAULT_STAT_STAGE + 2);

        InitPureDoubles();
        sSimContext.mons[sRosterByBattler[1]].normal.types[0] = TYPE_FIGHTING;
        sSimBefore.active[1].statStages[STAT_EVASION] = DEFAULT_STAT_STAGE + 1;
        SetPureAction(0, 0, MOVE_SPECTRAL_THIEF, 1);
        EXPECT_EQ(AiSim_CheckJointTurn(&sSimContext, &sSimBefore, &sSimTurn, &unsupported), AI_SIM_APPLY_UNSUPPORTED);
        EXPECT(unsupported & AI_SIM_UNSUPPORTED_OUTCOME);
        FreePureFixture();
    }
}
