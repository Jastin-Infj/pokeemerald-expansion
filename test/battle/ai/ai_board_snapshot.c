#include "global.h"
#include "test/battle.h"
#include "battle.h"
#include "battle_ai_board_sim.h"
#include "battle_ai_joint_runtime.h"
#include "battle_controllers.h"
#include "battle_gimmick.h"
#include "malloc.h"
#include "pokemon.h"
#include "constants/abilities.h"
#include "constants/battle_ai.h"
#include "constants/items.h"
#include "constants/moves.h"

#define ROSTER_INDEX(trainer, partyIndex) ((trainer) * PARTY_SIZE + (partyIndex))

struct AiSimSnapshotTestFixture
{
    struct AiSimContext context;
    struct AiSimBoard board;
    u32 inputsHash;
};

static EWRAM_DATA struct AiSimSnapshotTestFixture *sSnapshotFixture;

static struct AiSimSnapshotTestFixture *GetSnapshotFixture(void)
{
    if (sSnapshotFixture == NULL)
        sSnapshotFixture = AllocZeroed(sizeof(*sSnapshotFixture));
    return sSnapshotFixture;
}

#define sContext            (GetSnapshotFixture()->context)
#define sBoard              (GetSnapshotFixture()->board)
#define sSnapshotInputsHash (GetSnapshotFixture()->inputsHash)

static void FreeSnapshotFixture(void)
{
    TRY_FREE_AND_SET_NULL(sSnapshotFixture);
}

static u32 HashSnapshotBytes(u32 hash, const void *data, u32 size)
{
    const u8 *bytes = data;

    while (size-- != 0)
    {
        hash ^= *bytes++;
        hash *= 16777619;
    }

    return hash;
}

#define HASH_SNAPSHOT_INPUT(input) hash = HashSnapshotBytes(hash, &(input), sizeof(input))

static u32 HashSnapshotInputs(void)
{
    u32 hash = 2166136261;

    HASH_SNAPSHOT_INPUT(gBattleMons);
    HASH_SNAPSHOT_INPUT(gParties);
    HASH_SNAPSHOT_INPUT(*gBattleStruct);
    HASH_SNAPSHOT_INPUT(*gAiThinkingStruct);
    HASH_SNAPSHOT_INPUT(*gAiLogicData);
    HASH_SNAPSHOT_INPUT(*gAiPartyData);
    HASH_SNAPSHOT_INPUT(*gBattleHistory);
    HASH_SNAPSHOT_INPUT(gSideTimers);
    HASH_SNAPSHOT_INPUT(gFieldTimers);
    HASH_SNAPSHOT_INPUT(gSideStatuses);
    HASH_SNAPSHOT_INPUT(gBattleTypeFlags);
    HASH_SNAPSHOT_INPUT(gBattlersCount);
    HASH_SNAPSHOT_INPUT(gAbsentBattlerFlags);
    HASH_SNAPSHOT_INPUT(gBattlerPartyIndexes);
    HASH_SNAPSHOT_INPUT(gChosenActionByBattler);
    HASH_SNAPSHOT_INPUT(gChosenMoveByBattler);
    HASH_SNAPSHOT_INPUT(gLastMoves);
    HASH_SNAPSHOT_INPUT(gProtectStructs);
    HASH_SNAPSHOT_INPUT(gFieldStatuses);
    HASH_SNAPSHOT_INPUT(gBattleWeather);
    HASH_SNAPSHOT_INPUT(gBattleTurnCounter);
    HASH_SNAPSHOT_INPUT(gBattleScripting.levelUpHP);

    return hash;
}

static void SaveSnapshotInputs(void)
{
    sSnapshotInputsHash = HashSnapshotInputs();
}

static void ExpectSnapshotInputsUnchanged(void)
{
    EXPECT_EQ(HashSnapshotInputs(), sSnapshotInputsHash);
}

AI_DOUBLE_BATTLE_TEST("AI board snapshot captures the full known roster and does not mutate battle state")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_READ_PLAYER_MOVE | AI_FLAG_OMNISCIENT);
        TIE_BREAK_SCORE(RNG_AI_SCORE_TIE_DOUBLES_MOVE, SCORE_TIE_CHOSEN, 0);
        PLAYER(SPECIES_XERNEAS) { Ability(ABILITY_FAIRY_AURA); Item(ITEM_POWER_HERB); Moves(MOVE_CELEBRATE, MOVE_MOONBLAST, MOVE_PROTECT, MOVE_GEOMANCY); }
        PLAYER(SPECIES_INCINEROAR) { Ability(ABILITY_INTIMIDATE); Item(ITEM_SITRUS_BERRY); Moves(MOVE_CELEBRATE, MOVE_FAKE_OUT, MOVE_KNOCK_OFF, MOVE_PROTECT); }
        PLAYER(SPECIES_PIKACHU) { Ability(ABILITY_STATIC); Item(ITEM_LIGHT_BALL); Moves(MOVE_THUNDERBOLT, MOVE_PROTECT); }
        OPPONENT(SPECIES_WHIMSICOTT) { Ability(ABILITY_PRANKSTER); Item(ITEM_FOCUS_SASH); Moves(MOVE_CELEBRATE, MOVE_TAILWIND, MOVE_MOONBLAST, MOVE_PROTECT); }
        OPPONENT(SPECIES_MARSHADOW) { Ability(ABILITY_TECHNICIAN); Item(ITEM_MARSHADIUM_Z); Moves(MOVE_CELEBRATE, MOVE_SPECTRAL_THIEF, MOVE_SHADOW_SNEAK, MOVE_PROTECT); }
        OPPONENT(SPECIES_XERNEAS) { Ability(ABILITY_FAIRY_AURA); Item(ITEM_POWER_HERB); Moves(MOVE_GEOMANCY, MOVE_MOONBLAST, MOVE_PROTECT); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_CELEBRATE);
            MOVE(playerRight, MOVE_CELEBRATE);
            EXPECT_MOVE(opponentLeft, MOVE_CELEBRATE);
            EXPECT_MOVE(opponentRight, MOVE_CELEBRATE);
        }
    } THEN {
        enum BattlerId planningBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerLeft = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattlerId opponentRight = GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT);
        enum BattleTrainer playerTrainer = GetBattlerTrainer(playerLeft);
        enum BattleTrainer opponentTrainer = GetBattlerTrainer(planningBattler);
        u8 playerActive = ROSTER_INDEX(playerTrainer, gBattlerPartyIndexes[playerLeft]);
        u8 opponentActive = ROSTER_INDEX(GetBattlerTrainer(opponentRight), gBattlerPartyIndexes[opponentRight]);

        // Opening Intimidate can leave this command-selection bookkeeping bit
        // set after the switch-in Eject Pack check found nothing to activate.
        // Snapshot capture must normalize only its local copy.
        gBattleMons[opponentRight].volatiles.tryEjectPack = TRUE;
        SaveSnapshotInputs();
        EXPECT(AiSim_CaptureKnownBoard(planningBattler, gAiLogicData, &sContext, &sBoard));
        ExpectSnapshotInputsUnchanged();

        EXPECT(sContext.flags & AI_SIM_CONTEXT_OMNISCIENT);
        EXPECT(!(sContext.flags & AI_SIM_CONTEXT_PARTIAL_KNOWLEDGE));
        EXPECT(!(sContext.flags & AI_SIM_CONTEXT_VOLATILE_STATE));
        EXPECT(gBattleMons[opponentRight].volatiles.tryEjectPack);
        EXPECT_EQ(sContext.battlersCount, MAX_BATTLERS_COUNT);
        EXPECT_EQ(sContext.activeMask, 0xF);
        EXPECT_EQ(sBoard.activeMask, 0xF);
        EXPECT_EQ(sBoard.active[playerLeft].rosterIndex, playerActive);
        EXPECT_EQ(sBoard.active[opponentRight].rosterIndex, opponentActive);
        EXPECT_EQ(sContext.mons[playerActive].normal.species, SPECIES_XERNEAS_ACTIVE);
        EXPECT_EQ(sContext.mons[playerActive].moves[1], MOVE_MOONBLAST);
        EXPECT_EQ(sBoard.party[playerActive].item, ITEM_POWER_HERB);
        EXPECT(sBoard.party[playerActive].flags & AI_SIM_PARTY_ITEM_KNOWN);
        EXPECT(sContext.mons[playerActive].normal.flags & AI_SIM_PROFILE_ABILITY_KNOWN);
        EXPECT_EQ(sContext.mons[ROSTER_INDEX(playerTrainer, 2)].normal.species, SPECIES_PIKACHU);
        EXPECT_EQ(sContext.mons[ROSTER_INDEX(opponentTrainer, 2)].normal.species, SPECIES_XERNEAS_ACTIVE);

        EXPECT_EQ(sContext.reserved, 0);
        for (u32 rosterIndex = 0; rosterIndex < AI_SIM_ROSTER_COUNT; rosterIndex++)
        {
            if (sContext.mons[rosterIndex].flags & AI_SIM_MON_PRESENT)
            {
                EXPECT(sContext.mons[rosterIndex].dynamaxHpPercent >= 100);
                EXPECT(sContext.mons[rosterIndex].dynamaxHpPercent <= 200);
            }
            else
                EXPECT_EQ(sContext.mons[rosterIndex].dynamaxHpPercent, 0);
            EXPECT_EQ(sBoard.party[rosterIndex].reserved[0], 0);
            EXPECT_EQ(sBoard.party[rosterIndex].reserved[1], 0);
        }
        for (enum BattleSide side = B_SIDE_PLAYER; side < NUM_BATTLE_SIDES; side++)
            EXPECT_EQ(sBoard.sides[side].hazardsMask, 0);
        FreeSnapshotFixture();
    }
}

AI_DOUBLE_BATTLE_TEST("AI board snapshot derives reserve Mega Ultra Z and Tera futures from real party data")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_READ_PLAYER_MOVE | AI_FLAG_OMNISCIENT
               | AI_FLAG_SEQUENCE_SWITCHING);
        TIE_BREAK_SCORE(RNG_AI_SCORE_TIE_DOUBLES_MOVE, SCORE_TIE_CHOSEN, 0);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Moves(MOVE_CELEBRATE); }
        PLAYER(SPECIES_WYNAUT) { Ability(ABILITY_GUTS); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WYNAUT) { Ability(ABILITY_GUTS); Moves(MOVE_CELEBRATE); }
        // Invalid-for-species abilities are forced by the test runner and are
        // reapplied after live form recalculation.  The prospective profile
        // must preserve that exact harness state across Mega Evolution.
        OPPONENT(SPECIES_SCIZOR) { Ability(ABILITY_GUTS); Item(ITEM_SCIZORITE); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_PIKACHU) { Ability(ABILITY_GUTS); Item(ITEM_NORMALIUM_Z); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_SHUCKLE) { Ability(ABILITY_STURDY); TeraType(TYPE_GHOST); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_NECROZMA_DUSK_MANE) { Ability(ABILITY_PRISM_ARMOR); Item(ITEM_ULTRANECROZIUM_Z); Moves(MOVE_TACKLE); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_CELEBRATE);
            MOVE(playerRight, MOVE_CELEBRATE);
            EXPECT_MOVE(opponentLeft, MOVE_CELEBRATE);
            EXPECT_MOVE(opponentRight, MOVE_CELEBRATE);
        }
    } THEN {
        enum BattlerId planningBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerRightBattler = GetBattlerAtPosition(B_POSITION_PLAYER_RIGHT);
        enum BattleTrainer trainer = GetBattlerTrainer(planningBattler);
        u32 activeRoster = ROSTER_INDEX(trainer, gBattlerPartyIndexes[planningBattler]);
        u32 playerRightRoster = ROSTER_INDEX(GetBattlerTrainer(playerRightBattler),
                                             gBattlerPartyIndexes[playerRightBattler]);
        u32 megaRoster = ROSTER_INDEX(trainer, 2);
        u32 zRoster = ROSTER_INDEX(trainer, 3);
        u32 teraRoster = ROSTER_INDEX(trainer, 4);
        u32 ultraRoster = ROSTER_INDEX(trainer, 5);
        bool32 futureUnresolved;
        u32 futureGimmickMask;

        // Reserve intent is normally supplied by trainer data. The test
        // runner stores the equivalent per-party selection here.
        gBattleTestRunnerState->data.chosenGimmick[trainer][4] = GIMMICK_TERA;
        SaveSnapshotInputs();
        EXPECT(AiSim_CaptureKnownBoard(planningBattler, gAiLogicData, &sContext, &sBoard));
        ExpectSnapshotInputsUnchanged();

        EXPECT(sContext.mons[megaRoster].eligibleGimmicks & (1u << GIMMICK_MEGA));
        EXPECT_EQ(sContext.mons[megaRoster].transformationGimmick, GIMMICK_MEGA);
        EXPECT_EQ(sContext.mons[megaRoster].normal.species, SPECIES_SCIZOR);
        EXPECT_EQ(sContext.mons[megaRoster].transformed.species, SPECIES_SCIZOR_MEGA);
        EXPECT_EQ(sContext.mons[megaRoster].normal.ability, ABILITY_GUTS);
        EXPECT_EQ(sContext.mons[megaRoster].transformed.ability, ABILITY_GUTS);
        EXPECT(sContext.mons[megaRoster].transformed.flags & AI_SIM_PROFILE_VALID);
        EXPECT(sContext.mons[megaRoster].transformed.attack
             > sContext.mons[megaRoster].normal.attack);

        EXPECT(sContext.mons[zRoster].eligibleGimmicks & (1u << GIMMICK_Z_MOVE));
        EXPECT(!(sContext.mons[zRoster].eligibleGimmicks & (1u << GIMMICK_MEGA)));
        EXPECT(sContext.mons[teraRoster].eligibleGimmicks & (1u << GIMMICK_TERA));
        EXPECT_EQ(sContext.mons[teraRoster].teraType, TYPE_GHOST);

        // Ultranecrozium Z first exposes Ultra Burst. The same immutable
        // loadout also records the Z resource that becomes usable only after
        // Ultra Burst is active; runtime priority enforces that ordering.
        EXPECT(sContext.mons[ultraRoster].eligibleGimmicks & (1u << GIMMICK_ULTRA_BURST));
        EXPECT(sContext.mons[ultraRoster].eligibleGimmicks & (1u << GIMMICK_Z_MOVE));
        EXPECT_EQ(sContext.mons[ultraRoster].transformationGimmick, GIMMICK_ULTRA_BURST);
        EXPECT_EQ(sContext.mons[ultraRoster].normal.species, SPECIES_NECROZMA_DUSK_MANE);
        EXPECT_EQ(sContext.mons[ultraRoster].normal.ability, ABILITY_PRISM_ARMOR);
        EXPECT_EQ(sContext.mons[ultraRoster].transformed.species, SPECIES_NECROZMA_ULTRA);
        EXPECT_EQ(sContext.mons[ultraRoster].transformed.ability, ABILITY_NEUROFORCE);
        EXPECT_EQ(sContext.mons[ultraRoster].transformed.types[0], TYPE_PSYCHIC);
        EXPECT_EQ(sContext.mons[ultraRoster].transformed.types[1], TYPE_DRAGON);
        EXPECT_EQ(sContext.mons[ultraRoster].normal.types[2], TYPE_MYSTERY);
        EXPECT_EQ(sContext.mons[ultraRoster].transformed.types[2], TYPE_MYSTERY);
        EXPECT(sContext.mons[ultraRoster].transformed.speed
             > sContext.mons[ultraRoster].normal.speed);

        // Keep this large fixture in EWRAM. A second AiSimBoard on the battle
        // test callback stack can overwrite the runner state on hardware.
        sBoard.party[activeRoster].hp = 0;
        // One canonical enemy target keeps four reserves x (base + gimmick)
        // below the provider's 12-atomic retention cap. This assertion is
        // about legal family generation, not ranking-driven pruning.
        sBoard.party[playerRightRoster].hp = 0;
        sBoard.active[planningBattler].flags = AI_SIM_ACTIVE_NEEDS_REPLACEMENT;
        futureGimmickMask = Test_BattleAiJointRuntime_GetFutureGimmickMask(
            &sContext, &sBoard, planningBattler, &futureUnresolved);
        EXPECT(futureGimmickMask & (1u << GIMMICK_MEGA));
        EXPECT(futureGimmickMask & (1u << GIMMICK_Z_MOVE));
        EXPECT(futureGimmickMask & (1u << GIMMICK_TERA));
        // The supported Tackle route proves that the exact prospective Ultra
        // profile survives switch+activation and genuinely enters joint search
        // without an unresolved sentinel forcing strict fallback.
        EXPECT(futureGimmickMask & (1u << GIMMICK_ULTRA_BURST));
        EXPECT(!futureUnresolved);

        // Once Ultra is active, live CanUseZMove permits the same mon to use
        // its still-unspent Z resource. This direct availability assertion is
        // independent of Photon Geyser's not-yet-modeled dynamic category.
        sBoard.active[planningBattler].rosterIndex = ultraRoster;
        sBoard.active[planningBattler].flags = AI_SIM_ACTIVE_TRANSFORMED;
        sBoard.party[ultraRoster].activeGimmick = GIMMICK_ULTRA_BURST;
        sBoard.trainerGimmickUsed[trainer] |= 1u << GIMMICK_ULTRA_BURST;
        EXPECT(!Test_BattleAiJointRuntime_IsFutureGimmickAvailable(
            &sContext, &sBoard, ultraRoster, GIMMICK_ULTRA_BURST));
        EXPECT(Test_BattleAiJointRuntime_IsFutureGimmickAvailable(
            &sContext, &sBoard, ultraRoster, GIMMICK_Z_MOVE));
        FreeSnapshotFixture();
    }
}

AI_DOUBLE_BATTLE_TEST("AI board snapshot future Tera form entry remains fail closed")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_READ_PLAYER_MOVE | AI_FLAG_OMNISCIENT
               | AI_FLAG_SEQUENCE_SWITCHING);
        TIE_BREAK_SCORE(RNG_AI_SCORE_TIE_DOUBLES_MOVE, SCORE_TIE_CHOSEN, 0);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Moves(MOVE_CELEBRATE); }
        PLAYER(SPECIES_WYNAUT) { Ability(ABILITY_GUTS); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_GUTS); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WYNAUT) { Ability(ABILITY_GUTS); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_OGERPON) { Ability(ABILITY_GUTS); TeraType(TYPE_GRASS); Moves(MOVE_TACKLE); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_CELEBRATE);
            MOVE(playerRight, MOVE_CELEBRATE);
            EXPECT_MOVE(opponentLeft, MOVE_CELEBRATE);
            EXPECT_MOVE(opponentRight, MOVE_CELEBRATE);
        }
    } THEN {
        enum BattlerId planningBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattleTrainer trainer = GetBattlerTrainer(planningBattler);
        u32 activeRoster = ROSTER_INDEX(trainer, gBattlerPartyIndexes[planningBattler]);
        u32 ogerponRoster = ROSTER_INDEX(trainer, 2);
        bool32 futureUnresolved;
        u32 futureGimmickMask;

        gBattleTestRunnerState->data.chosenGimmick[trainer][2] = GIMMICK_TERA;
        EXPECT(DoesSpeciesHaveFormChangeMethod(SPECIES_OGERPON,
                                               FORM_CHANGE_BATTLE_TERASTALLIZATION));
        EXPECT(AiSim_CaptureKnownBoard(planningBattler, gAiLogicData, &sContext, &sBoard));
        EXPECT(sContext.mons[ogerponRoster].eligibleGimmicks & (1u << GIMMICK_TERA));

        sBoard.party[activeRoster].hp = 0;
        // The fainted outgoing slot may still carry a persistent Mega/Ultra
        // transformed flag.  It must not select the empty transformed profile
        // of the incoming Ogerpon and bypass the Tera-form boundary.
        sBoard.party[activeRoster].activeGimmick = GIMMICK_MEGA;
        sBoard.active[planningBattler].flags = AI_SIM_ACTIVE_NEEDS_REPLACEMENT
                                             | AI_SIM_ACTIVE_TRANSFORMED;
        futureGimmickMask = Test_BattleAiJointRuntime_GetFutureGimmickMask(
            &sContext, &sBoard, planningBattler, &futureUnresolved);
        EXPECT(!(futureGimmickMask & (1u << GIMMICK_TERA)));
        EXPECT(futureUnresolved);
        FreeSnapshotFixture();
    }
}

AI_DOUBLE_BATTLE_TEST("AI board snapshot sanitizes unobserved moves items abilities and reserves")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_LIMBER); Item(ITEM_POWER_HERB); Moves(MOVE_CELEBRATE, MOVE_PROTECT, MOVE_TAILWIND, MOVE_SPLASH); }
        PLAYER(SPECIES_WYNAUT) { Ability(ABILITY_LIMBER); Item(ITEM_LEFTOVERS); Moves(MOVE_CELEBRATE, MOVE_PROTECT); }
        PLAYER(SPECIES_GENGAR) { Ability(ABILITY_LEVITATE); Item(ITEM_LIFE_ORB); Moves(MOVE_SHADOW_BALL, MOVE_SLUDGE_BOMB, MOVE_PROTECT); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_TELEPATHY); Item(ITEM_ORAN_BERRY); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WYNAUT) { Ability(ABILITY_TELEPATHY); Item(ITEM_ORAN_BERRY); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_MARSHADOW) { Ability(ABILITY_TECHNICIAN); Item(ITEM_FOCUS_SASH); Moves(MOVE_SPECTRAL_THIEF, MOVE_PROTECT); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_CELEBRATE);
            MOVE(playerRight, MOVE_CELEBRATE);
            EXPECT_MOVE(opponentLeft, MOVE_CELEBRATE);
            EXPECT_MOVE(opponentRight, MOVE_CELEBRATE);
        }
    } THEN {
        enum BattlerId planningBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattlerId playerLeft = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        enum BattleTrainer playerTrainer = GetBattlerTrainer(playerLeft);
        enum BattleTrainer opponentTrainer = GetBattlerTrainer(planningBattler);
        u8 playerActive = ROSTER_INDEX(playerTrainer, gBattlerPartyIndexes[playerLeft]);
        u8 hiddenReserve = ROSTER_INDEX(playerTrainer, 2);
        u8 ownReserve = ROSTER_INDEX(opponentTrainer, 2);

        // Retained per-party knowledge can be richer than the sparse history
        // attached to the currently occupied battler slot. Snapshot capture
        // must merge the two sources instead of erasing the retained slot.
        gAiPartyData->mons[playerTrainer][gBattlerPartyIndexes[playerLeft]].moves[1] = MOVE_PROTECT;
        gBattleHistory->usedMoves[playerLeft][0] = MOVE_CELEBRATE;
        gBattleHistory->usedMoves[playerLeft][1] = MOVE_NONE;
        EXPECT(AiSim_CaptureKnownBoard(planningBattler, gAiLogicData, &sContext, &sBoard));

        EXPECT(!(sContext.flags & AI_SIM_CONTEXT_OMNISCIENT));
        EXPECT(sContext.flags & AI_SIM_CONTEXT_PARTIAL_KNOWLEDGE);
        EXPECT_EQ(sContext.mons[playerActive].normal.species, SPECIES_WOBBUFFET);
        EXPECT_EQ(sContext.mons[playerActive].moves[0], MOVE_CELEBRATE);
        EXPECT_EQ(sContext.mons[playerActive].moves[1], MOVE_PROTECT);
        EXPECT_EQ(sContext.mons[playerActive].moves[2], MOVE_NONE);
        EXPECT_EQ(sContext.mons[playerActive].moves[3], MOVE_NONE);
        EXPECT(!(sContext.mons[playerActive].flags & AI_SIM_MON_MOVES_KNOWN));
        EXPECT_EQ(sContext.mons[playerActive].normal.ability, ABILITY_NONE);
        EXPECT(!(sContext.mons[playerActive].normal.flags & AI_SIM_PROFILE_ABILITY_KNOWN));
        EXPECT_EQ(sContext.mons[playerActive].normal.maxHp, 0);
        EXPECT_EQ(sContext.mons[playerActive].normal.attack, 0);
        EXPECT_EQ(sContext.mons[playerActive].normal.defense, 0);
        EXPECT_EQ(sContext.mons[playerActive].normal.speed, 0);
        EXPECT_EQ(sContext.mons[playerActive].normal.spAttack, 0);
        EXPECT_EQ(sContext.mons[playerActive].normal.spDefense, 0);
        EXPECT_EQ(sBoard.party[playerActive].item, ITEM_NONE);
        EXPECT(!(sBoard.party[playerActive].flags & AI_SIM_PARTY_ITEM_KNOWN));

        EXPECT_EQ(sContext.mons[hiddenReserve].flags, 0);
        EXPECT_EQ(sContext.mons[hiddenReserve].normal.species, SPECIES_NONE);
        EXPECT_EQ(sBoard.party[hiddenReserve].item, ITEM_NONE);

        EXPECT(sContext.mons[ownReserve].flags & AI_SIM_MON_PRESENT);
        EXPECT(sContext.mons[ownReserve].flags & AI_SIM_MON_MOVES_KNOWN);
        EXPECT(sContext.mons[ownReserve].flags & AI_SIM_MON_PROFILE_KNOWN);
        EXPECT_EQ(sContext.mons[ownReserve].normal.species, SPECIES_MARSHADOW);
        EXPECT_EQ(sContext.mons[ownReserve].normal.ability, ABILITY_TECHNICIAN);
        EXPECT_EQ(sBoard.party[ownReserve].item, ITEM_FOCUS_SASH);
        FreeSnapshotFixture();
    }
}

AI_DOUBLE_BATTLE_TEST("AI board snapshot knows preview species without leaking their hidden loadout")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_KNOW_OPPONENT_PARTY | AI_FLAG_READ_PLAYER_MOVE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        PLAYER(SPECIES_WYNAUT) { Moves(MOVE_CELEBRATE); }
        PLAYER(SPECIES_GENGAR) { Ability(ABILITY_LEVITATE); Item(ITEM_LIFE_ORB); Moves(MOVE_SHADOW_BALL, MOVE_SLUDGE_BOMB, MOVE_PROTECT); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WYNAUT) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_CELEBRATE);
            MOVE(playerRight, MOVE_CELEBRATE);
            EXPECT_MOVE(opponentLeft, MOVE_CELEBRATE);
            EXPECT_MOVE(opponentRight, MOVE_CELEBRATE);
        }
    } THEN {
        enum BattlerId planningBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattleTrainer playerTrainer = GetBattlerTrainer(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT));
        u8 reserve = ROSTER_INDEX(playerTrainer, 2);

        EXPECT(AiSim_CaptureKnownBoard(planningBattler, gAiLogicData, &sContext, &sBoard));
        EXPECT(sContext.mons[reserve].flags & AI_SIM_MON_PRESENT);
        EXPECT_EQ(sContext.mons[reserve].normal.species, SPECIES_GENGAR);
        EXPECT_EQ(sContext.mons[reserve].normal.types[2], TYPE_MYSTERY);
        EXPECT_EQ(sContext.mons[reserve].moves[0], MOVE_NONE);
        EXPECT_EQ(sContext.mons[reserve].normal.ability, ABILITY_NONE);
        EXPECT_EQ(sBoard.party[reserve].item, ITEM_NONE);
        EXPECT(!(sContext.mons[reserve].flags & AI_SIM_MON_MOVES_KNOWN));
        EXPECT(!(sContext.mons[reserve].flags & AI_SIM_MON_PROFILE_KNOWN));
        EXPECT(!(sBoard.party[reserve].flags & AI_SIM_PARTY_ITEM_KNOWN));
        FreeSnapshotFixture();
    }
}

AI_SINGLE_BATTLE_TEST("AI board snapshot keeps a Dynamaxed battler's base max HP and live scaled HP distinct")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(120); MaxHP(200); DynamaxLevel(7); Moves(MOVE_SCRATCH); }
    } WHEN {
        TURN {
            MOVE(player, MOVE_CELEBRATE);
            EXPECT_MOVE(opponent, MOVE_SCRATCH, gimmick: GIMMICK_DYNAMAX);
        }
    } THEN {
        enum BattlerId planningBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattleTrainer trainer = GetBattlerTrainer(planningBattler);
        u8 rosterIndex = ROSTER_INDEX(trainer, gBattlerPartyIndexes[planningBattler]);

        EXPECT(AiSim_CaptureKnownBoard(planningBattler, gAiLogicData, &sContext, &sBoard));
        EXPECT(sBoard.active[planningBattler].flags & AI_SIM_ACTIVE_DYNAMAX);
        EXPECT_EQ(sContext.mons[rosterIndex].dynamaxHpPercent, 185);
        EXPECT_EQ(sContext.mons[rosterIndex].normal.maxHp, 200);
        EXPECT_EQ(gBattleMons[planningBattler].maxHP, 370);
        EXPECT_EQ(sBoard.party[rosterIndex].hp, gBattleMons[planningBattler].hp);
        EXPECT_EQ(sBoard.party[rosterIndex].hp, 222);
        EXPECT_EQ(sBoard.active[planningBattler].choiceMoveSlot, AI_SIM_MOVE_SLOT_NONE);
        EXPECT_EQ(sBoard.active[planningBattler].disabledMoveMask, 0);
        FreeSnapshotFixture();
    }
}

AI_DOUBLE_BATTLE_TEST("AI board snapshot captures active stages locks timers field state and gimmick ownership")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_READ_PLAYER_MOVE | AI_FLAG_OMNISCIENT);
        TIE_BREAK_SCORE(RNG_AI_SCORE_TIE_DOUBLES_MOVE, SCORE_TIE_CHOSEN, 0);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        PLAYER(SPECIES_WYNAUT) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_XERNEAS) { Item(ITEM_POWER_HERB); Moves(MOVE_CELEBRATE, MOVE_GEOMANCY, MOVE_PROTECT); }
        OPPONENT(SPECIES_WHIMSICOTT) { Item(ITEM_FOCUS_SASH); Moves(MOVE_CELEBRATE, MOVE_TAILWIND, MOVE_PROTECT); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_CELEBRATE);
            MOVE(playerRight, MOVE_CELEBRATE);
            EXPECT_MOVE(opponentLeft, MOVE_CELEBRATE);
            EXPECT_MOVE(opponentRight, MOVE_CELEBRATE);
        }
    } THEN {
        enum BattlerId planningBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        enum BattleTrainer trainer = GetBattlerTrainer(planningBattler);
        u8 rosterIndex = ROSTER_INDEX(trainer, gBattlerPartyIndexes[planningBattler]);

        gBattleMons[planningBattler].statStages[STAT_SPEED] = DEFAULT_STAT_STAGE + 2;
        gBattleMons[planningBattler].volatiles.disabledMove = MOVE_PROTECT;
        gBattleMons[planningBattler].volatiles.consecutiveMoveUses = 2;
        gBattleMons[planningBattler].volatiles.perishSong = TRUE;
        gBattleMons[planningBattler].volatiles.perishSongTimer = 1;
        gBattleStruct->choicedMove[planningBattler] = MOVE_GEOMANCY;
        gBattleStruct->gimmick.activated[planningBattler][GIMMICK_Z_MOVE] = TRUE;
        gSideStatuses[B_SIDE_OPPONENT] |= SIDE_STATUS_TAILWIND | SIDE_STATUS_REFLECT;
        gSideTimers[B_SIDE_OPPONENT].tailwindTimer = 3;
        gSideTimers[B_SIDE_OPPONENT].reflectTimer = 2;
        gFieldStatuses |= STATUS_FIELD_TRICK_ROOM | STATUS_FIELD_PSYCHIC_TERRAIN;
        gFieldTimers.trickRoomTimer = 4;
        gFieldTimers.terrainTimer = 5;
        gBattleWeather = B_WEATHER_RAIN_NORMAL;
        gBattleStruct->weatherDuration = 6;

        EXPECT(AiSim_CaptureKnownBoard(planningBattler, gAiLogicData, &sContext, &sBoard));
        EXPECT_EQ(sBoard.active[planningBattler].rosterIndex, rosterIndex);
        EXPECT_EQ(sBoard.active[planningBattler].statStages[STAT_SPEED], DEFAULT_STAT_STAGE + 2);
        EXPECT(sBoard.active[planningBattler].disabledMoveMask & (1u << 2));
        EXPECT_EQ(sBoard.active[planningBattler].choiceMoveSlot, 1);
        EXPECT_EQ(sBoard.active[planningBattler].consecutiveMoveUses, 2);
        EXPECT_EQ(sBoard.active[planningBattler].perishTimer, 1);
        EXPECT_EQ(sBoard.sides[B_SIDE_OPPONENT].tailwindTimer, 3);
        EXPECT_EQ(sBoard.sides[B_SIDE_OPPONENT].reflectTimer, 2);
        EXPECT_EQ(sBoard.trickRoomTimer, 4);
        EXPECT_EQ(sBoard.terrainTimer, 5);
        EXPECT(sBoard.fieldStatuses & STATUS_FIELD_PSYCHIC_TERRAIN);
        EXPECT(!(sContext.flags & AI_SIM_CONTEXT_FIELD_STATE));
        EXPECT_EQ(sBoard.weather, B_WEATHER_RAIN_NORMAL);
        EXPECT_EQ(sBoard.weatherTimer, 6);
        EXPECT(sBoard.trainerGimmickUsed[trainer] & (1u << GIMMICK_Z_MOVE));
        FreeSnapshotFixture();
    }
}
