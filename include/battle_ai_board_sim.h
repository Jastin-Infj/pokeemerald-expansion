#ifndef GUARD_BATTLE_AI_BOARD_SIM_H
#define GUARD_BATTLE_AI_BOARD_SIM_H

#include "global.h"
#include "constants/battle.h"
#include "constants/global.h"
#include "constants/pokemon.h"

#define AI_SIM_ROSTER_COUNT (MAX_BATTLE_TRAINERS * PARTY_SIZE)
#define AI_SIM_MAX_ACTIONS MAX_BATTLERS_COUNT
#define AI_SIM_MAX_OUTCOMES 32
#define AI_SIM_MAX_OUTCOME_APPLICATIONS 4096
#define AI_SIM_BOARD_FRAMES 6
#define AI_SIM_ROSTER_NONE 0xFF
#define AI_SIM_MOVE_SLOT_NONE MAX_MON_MOVES

// These layouts are copied by the search. Keep them pointer-free and keep all
// reserved bytes zero so whole-object hashes and comparisons remain stable.
struct AiSimCombatProfile
{
    u16 species;
    u16 maxHp;
    u16 attack;
    u16 defense;
    u16 speed;
    u16 spAttack;
    u16 spDefense;
    u16 ability;
    u8 types[3];
    u8 flags;
};

struct AiSimMonTemplate
{
    struct AiSimCombatProfile normal;
    struct AiSimCombatProfile transformed;
    u16 moves[MAX_MON_MOVES];
    u8 level;
    u8 teraType;
    u8 trainer;
    u8 partyIndex;
    u8 eligibleGimmicks;
    u8 transformationGimmick;
    u8 flags;
    u8 dynamaxHpPercent; // Shedinja 100, otherwise 150..200; 0 if unavailable.
};

struct AiSimContext
{
    struct AiSimMonTemplate mons[AI_SIM_ROSTER_COUNT];
    u32 battleTypeFlags;
    u8 battlerTrainer[MAX_BATTLERS_COUNT];
    u8 tieRank[MAX_BATTLERS_COUNT];
    u8 gimmickShareMask[MAX_BATTLERS_COUNT];
    u8 battlersCount;
    u8 activeMask;
    u8 flags;
    u8 reserved;
};

struct AiSimPartyState
{
    u32 status1;
    u16 hp;
    u16 item;
    u8 pp[MAX_MON_MOVES];
    u8 activeGimmick;
    u8 flags;
    u8 reserved[2];
};

struct __attribute__((packed, aligned(2))) AiSimActiveState
{
    u16 lastMove;
    u16 chargingMove;
    u16 volatileFlags;
    u8 rosterIndex;
    s8 statStages[NUM_BATTLE_STATS];
    u8 consecutiveMoveUses;
    u8 dynamaxTurns;
    u8 firstTurn;
    u8 disabledMoveMask;
    u8 choiceMoveSlot;
    u8 perishTimer;
    u8 flags;
};

struct AiSimSideState
{
    u32 statuses;
    u8 tailwindTimer;
    u8 reflectTimer;
    u8 lightScreenTimer;
    u8 auroraVeilTimer;
    u8 safeguardTimer;
    u8 mistTimer;
    u8 followMeTarget;
    u8 followMeTimer;
    u8 spikes;
    u8 toxicSpikes;
    u8 flags;
    u8 hazardsMask;
};

struct AiSimBoard
{
    struct AiSimPartyState party[AI_SIM_ROSTER_COUNT];
    struct AiSimActiveState active[MAX_BATTLERS_COUNT];
    struct AiSimSideState sides[NUM_BATTLE_SIDES];
    u32 fieldStatuses;
    u32 stellarBoostFlags[MAX_BATTLE_TRAINERS];
    u16 weather;
    u16 turn;
    u8 weatherTimer;
    u8 trickRoomTimer;
    u8 terrainTimer;
    u8 gravityTimer;
    u8 magicRoomTimer;
    u8 wonderRoomTimer;
    u8 activeMask;
    u8 absentMask;
    u8 trainerGimmickUsed[MAX_BATTLE_TRAINERS];
};

struct AiSimAction
{
    u16 choice; // Move ID, or flattened roster index for a switch.
    u8 actor;
    u8 target;
    u8 kind;
    u8 moveSlot;
    u8 gimmick;
    u8 allyInteractionKind;
    // A forced replacement is part of the following move command, rather
    // than a standalone turn.  The flag keeps ordinary zero-initialized
    // actions backwards compatible while making the reserve explicit.
    u8 replacementRosterIndex;
    u8 flags;
};

struct AiSimJointTurn
{
    struct AiSimAction actions[MAX_BATTLERS_COUNT];
    u8 actionMask;
    u8 confirmedPlayerMask;
    u8 flags;
    u8 reserved;
};

struct AiSimOutcomeKey
{
    // All emitted keys use the same implicit denominator.  The exact branch
    // probability is therefore probabilityWeight / sum(probabilityWeight),
    // after equivalent post-turn boards have had their weights added.  The
    // joint planner scores only that immutable board; transient apply-result
    // diagnostics are deliberately not part of its outcome state.
    // This avoids multiplying whole-turn rational denominators while keeping
    // the fixed search arena small.
    u32 probabilityWeight;
    // Four independent 0..15 damage rolls, packed in battler-id order.
    u16 damageRolls;
    // Low battler nibble: the actor's configured move secondary succeeds.
    // High battler nibble: the battler thaws before attempting its action.
    u8 flags;
    u8 protectSuccessMask;
    u8 criticalMask;
    // Lexicographic permutation of the actors used only when priority,
    // Stall/Lagging Tail, and effective Speed are tied.  The context sentinel
    // preserves the deterministic direct-Apply helper used by focused tests;
    // enumerated planner outcomes always carry an exact random permutation.
    u8 speedTieOrder;
    u16 reserved;
};

struct AiSimTurnResult
{
    u32 events;
    u16 damage[MAX_BATTLERS_COUNT];
    u16 rejectionFlags[MAX_BATTLERS_COUNT];
    u8 readInteractionFlags[MAX_BATTLERS_COUNT];
    u8 actionOrder[MAX_BATTLERS_COUNT];
    u8 resolvedTargets[MAX_BATTLERS_COUNT];
    u8 executedMask;
    u8 skippedMask;
    u8 protectSuccessMask;
    u8 actionCount;
    u32 unsupportedFlags;
};

enum AiSimApplyStatus
{
    AI_SIM_APPLY_OK,
    AI_SIM_APPLY_INVALID,
    AI_SIM_APPLY_UNSUPPORTED,
};

enum AiSimActionKind
{
    AI_SIM_ACTION_NONE,
    AI_SIM_ACTION_MOVE,
    AI_SIM_ACTION_SWITCH,
};

enum AiSimActionFlags
{
    AI_SIM_ACTION_FORCED_REPLACEMENT         = 1 << 0,
};

enum AiSimDamageRoll
{
    AI_SIM_DAMAGE_ROLL_MINIMUM = 0,
    AI_SIM_DAMAGE_ROLL_MEDIAN = 8,
    AI_SIM_DAMAGE_ROLL_MAXIMUM = 15,
};

#define AI_SIM_SPEED_TIE_ORDER_CONTEXT 0xFF

#define AI_SIM_OUTCOME_SECONDARY_MASK ((1u << MAX_BATTLERS_COUNT) - 1)
#define AI_SIM_OUTCOME_THAW_SHIFT MAX_BATTLERS_COUNT
#define AI_SIM_OUTCOME_THAW_MASK (AI_SIM_OUTCOME_SECONDARY_MASK << AI_SIM_OUTCOME_THAW_SHIFT)
#define AI_SIM_MODELED_RESIDUAL_STATUSES (STATUS1_BURN | STATUS1_FROSTBITE)

enum AiSimUnsupportedFlags
{
    AI_SIM_UNSUPPORTED_NONE                 = 0,
    AI_SIM_UNSUPPORTED_BATTLE_TYPE          = 1 << 0,
    AI_SIM_UNSUPPORTED_ACTION_KIND          = 1 << 1,
    AI_SIM_UNSUPPORTED_MOVE_EFFECT          = 1 << 2,
    AI_SIM_UNSUPPORTED_DYNAMIC_POWER        = 1 << 3,
    AI_SIM_UNSUPPORTED_SECONDARY_EFFECT     = 1 << 4,
    AI_SIM_UNSUPPORTED_DAMAGE_MODIFIER      = 1 << 5,
    AI_SIM_UNSUPPORTED_GIMMICK              = 1 << 6,
    AI_SIM_UNSUPPORTED_SWITCH_IN_EFFECT     = 1 << 7,
    AI_SIM_UNSUPPORTED_OUTCOME              = 1 << 8,
    AI_SIM_UNSUPPORTED_KNOWLEDGE            = 1 << 9,
    AI_SIM_UNSUPPORTED_REDIRECTION          = 1 << 10,
    AI_SIM_UNSUPPORTED_SUBSTITUTE           = 1 << 11,
    AI_SIM_UNSUPPORTED_RESIDUAL             = 1 << 12,
    AI_SIM_UNSUPPORTED_FIELD_STATE          = 1 << 13,
    AI_SIM_UNSUPPORTED_VOLATILE_STATE       = 1 << 14,
};

enum AiSimEventFlags
{
    AI_SIM_EVENT_DAMAGE                     = 1 << 0,
    AI_SIM_EVENT_KO                         = 1 << 1,
    AI_SIM_EVENT_FLINCH                     = 1 << 2,
    AI_SIM_EVENT_PROTECTED                  = 1 << 3,
    AI_SIM_EVENT_ITEM_CONSUMED              = 1 << 4,
    AI_SIM_EVENT_ITEM_REMOVED               = 1 << 5,
    AI_SIM_EVENT_STAT_CHANGE                = 1 << 6,
    AI_SIM_EVENT_FIELD_CHANGE               = 1 << 7,
    AI_SIM_EVENT_SWITCH                     = 1 << 8,
    AI_SIM_EVENT_FOCUS_SASH                 = 1 << 9,
    AI_SIM_EVENT_STURDY                     = 1 << 10,
    AI_SIM_EVENT_WEAKNESS_POLICY            = 1 << 11,
    AI_SIM_EVENT_PP_SPENT                   = 1 << 12,
    AI_SIM_EVENT_CHOICE_LOCK                = 1 << 13,
    AI_SIM_EVENT_STATUS_CHANGE              = 1 << 14,
    AI_SIM_EVENT_RECOIL                     = 1 << 15,
};

enum AiSimProfileFlags
{
    AI_SIM_PROFILE_VALID                    = 1 << 0,
    AI_SIM_PROFILE_ABILITY_KNOWN            = 1 << 1,
    AI_SIM_PROFILE_TYPES_KNOWN              = 1 << 2,
};

enum AiSimMonFlags
{
    AI_SIM_MON_PRESENT                      = 1 << 0,
    AI_SIM_MON_MOVES_KNOWN                  = 1 << 1,
    AI_SIM_MON_PROFILE_KNOWN                = 1 << 2,
};

enum AiSimPartyFlags
{
    AI_SIM_PARTY_ITEM_KNOWN                 = 1 << 0,
    AI_SIM_PARTY_STATUS_KNOWN               = 1 << 1,
    AI_SIM_PARTY_ITEM_CONSUMED              = 1 << 2,
    AI_SIM_PARTY_ITEM_REMOVED               = 1 << 3,
};

enum AiSimActiveFlags
{
    AI_SIM_ACTIVE_DYNAMAX                   = 1 << 0,
    AI_SIM_ACTIVE_TERA                      = 1 << 1,
    AI_SIM_ACTIVE_TRANSFORMED               = 1 << 2,
    AI_SIM_ACTIVE_NEEDS_REPLACEMENT         = 1 << 3,
};

enum AiSimVolatileFlags
{
    AI_SIM_VOLATILE_PROTECTED               = 1 << 0,
    AI_SIM_VOLATILE_MAX_GUARD               = 1 << 1,
    AI_SIM_VOLATILE_FLINCHED                = 1 << 2,
    AI_SIM_VOLATILE_GEOMANCY_CHARGING       = 1 << 3,
};

enum AiSimSideFlags
{
    AI_SIM_SIDE_QUICK_GUARD                 = 1 << 0,
    AI_SIM_SIDE_WIDE_GUARD                  = 1 << 1,
};

enum AiSimContextFlags
{
    AI_SIM_CONTEXT_OMNISCIENT               = 1 << 0,
    AI_SIM_CONTEXT_PARTIAL_KNOWLEDGE         = 1 << 1,
    AI_SIM_CONTEXT_REDIRECTION_STATE         = 1 << 2,
    AI_SIM_CONTEXT_SUBSTITUTE_STATE          = 1 << 3,
    AI_SIM_CONTEXT_RESIDUAL_STATE            = 1 << 4,
    AI_SIM_CONTEXT_FIELD_STATE               = 1 << 5,
    AI_SIM_CONTEXT_VOLATILE_STATE            = 1 << 6,
};

enum AiSimJointTurnFlags
{
    AI_SIM_JOINT_CONSERVATIVE_ENEMY_TIES    = 1 << 0,
};

STATIC_ASSERT(sizeof(struct AiSimCombatProfile) == 20, AiSimCombatProfileSizeChanged)
STATIC_ASSERT(sizeof(struct AiSimMonTemplate) == 56, AiSimMonTemplateSizeChanged)
STATIC_ASSERT(sizeof(struct AiSimContext) == 1364, AiSimContextSizeChanged)
STATIC_ASSERT(sizeof(struct AiSimPartyState) == 16, AiSimPartyStateSizeChanged)
STATIC_ASSERT(sizeof(struct AiSimActiveState) == 22, AiSimActiveStateSizeChanged)
STATIC_ASSERT(sizeof(struct AiSimSideState) == 16, AiSimSideStateSizeChanged)
STATIC_ASSERT(sizeof(struct AiSimBoard) == 540, AiSimBoardSizeChanged)
STATIC_ASSERT(sizeof(struct AiSimAction) == 12, AiSimActionSizeChanged)
STATIC_ASSERT(sizeof(struct AiSimJointTurn) == 52, AiSimJointTurnSizeChanged)
STATIC_ASSERT(sizeof(struct AiSimOutcomeKey) == 12, AiSimOutcomeKeySizeChanged)
STATIC_ASSERT(sizeof(struct AiSimTurnResult) == 40, AiSimTurnResultSizeChanged)

struct AiLogicData;
struct SimulatedDamage;

bool32 AiSim_CaptureKnownBoard(enum BattlerId planningBattler,
                               const struct AiLogicData *aiData,
                               struct AiSimContext *context,
                               struct AiSimBoard *board);

enum AiSimApplyStatus AiSim_CheckJointTurn(const struct AiSimContext *context,
                                           const struct AiSimBoard *board,
                                           const struct AiSimJointTurn *turn,
                                           u32 *unsupportedFlags);
u32 AiSim_EnumerateOutcomes(const struct AiSimContext *context,
                            const struct AiSimBoard *board,
                            const struct AiSimJointTurn *turn,
                            struct AiSimOutcomeKey *outcomes,
                            u32 capacity);
enum AiSimApplyStatus AiSim_ApplyJointTurn(const struct AiSimContext *context,
                                           const struct AiSimBoard *before,
                                           const struct AiSimJointTurn *turn,
                                           const struct AiSimOutcomeKey *outcome,
                                           struct AiSimBoard *after,
                                           struct AiSimTurnResult *result);
u32 AiSim_GetEffectiveSpeed(const struct AiSimContext *context,
                            const struct AiSimBoard *board,
                            enum BattlerId battler);
bool32 AiSim_IsItemRemovable(const struct AiSimContext *context,
                             const struct AiSimBoard *board,
                             enum BattlerId battler);
bool32 AiSim_CalcDamage(const struct AiSimContext *context,
                        const struct AiSimBoard *board,
                        const struct AiSimAction *action,
                        enum BattlerId target,
                        struct SimulatedDamage *damage);
// Projects only damage that the immutable simulator can legally resolve on
// this board.  Unlike AiSim_CalcDamage, this applies blocking terrain/guards
// and full-HP survival effects without mutating the board.
bool32 AiSim_ProjectDamage(const struct AiSimContext *context,
                           const struct AiSimBoard *board,
                           const struct AiSimAction *action,
                           enum BattlerId target,
                           struct SimulatedDamage *damage);
// Replacement prompts are resolved in battler-id order.  This keeps two
// fainted slots from both claiming the same final reserve.
bool32 AiSim_ReplacementSlotWillBeFilled(const struct AiSimContext *context,
                                         const struct AiSimBoard *board,
                                         enum BattlerId battler);
u32 AiSim_GetProtectSuccessDenominator(const struct AiSimBoard *board,
                                       enum BattlerId battler);
u16 AiSim_GetActionRejectionFlags(const struct AiSimContext *context,
                                  const struct AiSimBoard *board,
                                  const struct AiSimAction *action);
#if TESTING
u32 Test_AiSim_GetLastOutcomeApplicationCount(void);
bool32 Test_AiSim_OutcomeScratchCanariesIntact(void);
#endif

#endif // GUARD_BATTLE_AI_BOARD_SIM_H
