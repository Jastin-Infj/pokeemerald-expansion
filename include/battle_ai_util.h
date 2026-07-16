#ifndef GUARD_BATTLE_AI_UTIL_H
#define GUARD_BATTLE_AI_UTIL_H

#include "battle_ai_main.h"
#include "battle_ai_field_statuses.h"
#include "constants/items.h"

// Roll boundaries used by AI when scoring. Doesn't affect actual damage dealt.
#define MAX_ROLL_PERCENTAGE DMG_ROLL_PERCENT_HI
#define MIN_ROLL_PERCENTAGE DMG_ROLL_PERCENT_LO
#define DMG_ROLL_PERCENTAGE ((MAX_ROLL_PERCENTAGE + MIN_ROLL_PERCENTAGE + 1) / 2) // Controls the damage roll the AI sees for the median roll. By default the 9th roll is seen

enum DamageRollType
{
    DMG_ROLL_LOWEST,
    DMG_ROLL_MEDIAN,
    DMG_ROLL_HIGHEST,
    DMG_ROLL_RANDOM,
};

enum DamageCalcContext
{
    AI_DEFENDING,
    AI_ATTACKING,
    AI_SWITCHIN_DEFENDING,
    AI_SWITCHIN_ATTACKING,
    AI_SHOULD_SETUP_DEFENDING,
    AI_ATTACKING_PARTNER,
};

enum AiConsiderEndure
{
    CONSIDER_ENDURE,
    DONT_CONSIDER_ENDURE,
};

// Higher priority at the bottom; note that these are used in the formula MAX_MON_MOVES ^ AiCompareMovesPriority, which must fit within a u32.
// In expansion where MAX_MON_MOVES is 4, this means that AiCompareMovesPriority can range from 0 - 15 inclusive.
enum AiCompareMovesPriority
{
    PRIORITY_EFFECT,
    PRIORITY_ACCURACY,
    PRIORITY_GUARANTEE,
    PRIORITY_AVOID_SELF_SACRIFICE,
    PRIORITY_SPEED,
    PRIORITY_NOT_CHARGING,
    PRIORITY_RESIST_BERRY,
};

enum AIPivot
{
    DONT_PIVOT,
    CAN_TRY_PIVOT,
    SHOULD_PIVOT,
};

#define AI_MOVE_KNOWLEDGE_NONE               0
#define AI_MOVE_KNOWLEDGE_CONTACT            (1u <<  0)
#define AI_MOVE_KNOWLEDGE_SOUND              (1u <<  1)
#define AI_MOVE_KNOWLEDGE_BALLISTIC          (1u <<  2)
#define AI_MOVE_KNOWLEDGE_POWDER             (1u <<  3)
#define AI_MOVE_KNOWLEDGE_SLICING            (1u <<  4)
#define AI_MOVE_KNOWLEDGE_PUNCHING           (1u <<  5)
#define AI_MOVE_KNOWLEDGE_BITING             (1u <<  6)
#define AI_MOVE_KNOWLEDGE_PULSE              (1u <<  7)
#define AI_MOVE_KNOWLEDGE_DANCE              (1u <<  8)
#define AI_MOVE_KNOWLEDGE_WIND               (1u <<  9)
#define AI_MOVE_KNOWLEDGE_HEALING            (1u << 10)
#define AI_MOVE_KNOWLEDGE_MAGIC_COAT         (1u << 11)
#define AI_MOVE_KNOWLEDGE_SNATCH             (1u << 12)
#define AI_MOVE_KNOWLEDGE_ABILITY_CONTROL    (1u << 13)
#define AI_MOVE_KNOWLEDGE_MOVE_DENIAL        (1u << 14)
#define AI_MOVE_KNOWLEDGE_COMBO_STATE        (1u << 15)
#define AI_MOVE_KNOWLEDGE_MAX_SPEED_CONTROL  (1u << 16)
#define AI_MOVE_KNOWLEDGE_MAX_WEATHER        (1u << 17)
#define AI_MOVE_KNOWLEDGE_MAX_TERRAIN        (1u << 18)
#define AI_MOVE_KNOWLEDGE_MAX_STAT_CONTROL   (1u << 19)
#define AI_MOVE_KNOWLEDGE_GMAX_UNIQUE        (1u << 20)
#define AI_MOVE_KNOWLEDGE_GMAX_RESIDUAL      (1u << 21)
#define AI_MOVE_KNOWLEDGE_Z_STATUS           (1u << 22)
#define AI_MOVE_KNOWLEDGE_Z_STAT_RESET       (1u << 23)
#define AI_MOVE_KNOWLEDGE_Z_STAT_BOOST       (1u << 24)
#define AI_MOVE_KNOWLEDGE_Z_CRIT_BOOST       (1u << 25)
#define AI_MOVE_KNOWLEDGE_Z_REDIRECTION      (1u << 26)
#define AI_MOVE_KNOWLEDGE_Z_RECOVERY         (1u << 27)
#define AI_MOVE_KNOWLEDGE_Z_REPLACEMENT_HEAL (1u << 28)
#define AI_MOVE_KNOWLEDGE_GMAX_HAZARD        (1u << 29)

#define AI_ABILITY_KNOWLEDGE_NONE            0
#define AI_ABILITY_KNOWLEDGE_MOVE_IMMUNITY   (1u <<  0)
#define AI_ABILITY_KNOWLEDGE_MOVE_POWER      (1u <<  1)
#define AI_ABILITY_KNOWLEDGE_DAMAGE_RACE     (1u <<  2)
#define AI_ABILITY_KNOWLEDGE_STATUS          (1u <<  3)
#define AI_ABILITY_KNOWLEDGE_FIELD_CONTROL   (1u <<  4)
#define AI_ABILITY_KNOWLEDGE_POSITIONING     (1u <<  5)
#define AI_ABILITY_KNOWLEDGE_ABILITY_CONTROL (1u <<  6)
#define AI_ABILITY_KNOWLEDGE_STAT_CONTROL    (1u <<  7)
#define AI_ABILITY_KNOWLEDGE_ITEM_CONTROL    (1u <<  8)
#define AI_ABILITY_KNOWLEDGE_PRIORITY        (1u <<  9)
#define AI_ABILITY_KNOWLEDGE_FORM_STATE      (1u << 10)

#define AI_HOLD_EFFECT_KNOWLEDGE_NONE                 0
#define AI_HOLD_EFFECT_KNOWLEDGE_DAMAGE_RACE          (1u <<  0)
#define AI_HOLD_EFFECT_KNOWLEDGE_DEFENSIVE_RACE       (1u <<  1)
#define AI_HOLD_EFFECT_KNOWLEDGE_STAT_CONTROL         (1u <<  2)
#define AI_HOLD_EFFECT_KNOWLEDGE_SPEED_CONTROL        (1u <<  3)
#define AI_HOLD_EFFECT_KNOWLEDGE_RECOVERY             (1u <<  4)
#define AI_HOLD_EFFECT_KNOWLEDGE_STATUS_CURE          (1u <<  5)
#define AI_HOLD_EFFECT_KNOWLEDGE_SELF_STATUS          (1u <<  6)
#define AI_HOLD_EFFECT_KNOWLEDGE_FIELD_DURATION       (1u <<  7)
#define AI_HOLD_EFFECT_KNOWLEDGE_CONTACT_PUNISH       (1u <<  8)
#define AI_HOLD_EFFECT_KNOWLEDGE_MOVE_SHAPE           (1u <<  9)
#define AI_HOLD_EFFECT_KNOWLEDGE_ABILITY_PROTECTION   (1u << 10)
#define AI_HOLD_EFFECT_KNOWLEDGE_CHOICE_LOCK          (1u << 11)
#define AI_HOLD_EFFECT_KNOWLEDGE_POSITIONING          (1u << 12)
#define AI_HOLD_EFFECT_KNOWLEDGE_GIMMICK              (1u << 13)

enum WeatherState
{
    WEATHER_INACTIVE,
    WEATHER_ACTIVE,
    WEATHER_ACTIVE_BUT_BLOCKED,
    WEATHER_INACTIVE_AND_BLOCKED,
};

enum AIConsiderGimmick
{
    NO_GIMMICK,
    USE_GIMMICK,
};

enum ConsiderPriority
{
    DONT_CONSIDER_PRIORITY,
    CONSIDER_PRIORITY,
};

enum AiThreatClass
{
    AI_THREAT_STABLE            = 0,
    AI_THREAT_DAMAGE_RACE       = 1 << 0,
    AI_THREAT_KNOWN_KO_PRESSURE = 1 << 1,
    AI_THREAT_SETUP_CHECKMATE   = 1 << 2,
    AI_THREAT_PERISH_TRAP_CLOCK = 1 << 3,
    AI_THREAT_MODE_LOSS         = 1 << 4,
    AI_THREAT_DESPERATION       = 1 << 5,
};

enum AiRiskKind
{
    AI_RISK_HIGH_VARIANCE_COMEBACK,
    AI_RISK_LOW_ACCURACY_STATUS,
    AI_RISK_LOW_ACCURACY_DAMAGE,
    AI_RISK_SECONDARY_HAX,
    AI_RISK_OHKO_FISH,
    AI_RISK_DELAYED_ATTACK,
    AI_RISK_PARTNER_SACRIFICE,
    AI_RISK_SECOND_PROTECT,
    AI_RISK_SWITCH_SURVIVAL,
};

enum AiShortHorizonLine
{
    AI_SHORT_LINE_NONE            = 0,
    AI_SHORT_LINE_CLEAN_DAMAGE    = 1 << 0,
    AI_SHORT_LINE_SWITCH_ESCAPE   = 1 << 1,
    AI_SHORT_LINE_SETUP_DENIAL    = 1 << 2,
    AI_SHORT_LINE_MODE_CONTROL    = 1 << 3,
    AI_SHORT_LINE_RESERVE_ENTRY   = 1 << 4,
    AI_SHORT_LINE_HIGH_VARIANCE   = 1 << 5,
};

enum AiCandidateLineFamily
{
    AI_CANDIDATE_LINE_NONE,
    AI_CANDIDATE_LINE_CLEAN_DAMAGE,
    AI_CANDIDATE_LINE_SWITCH_ESCAPE,
    AI_CANDIDATE_LINE_SETUP_DENIAL,
    AI_CANDIDATE_LINE_MODE_CONTROL,
    AI_CANDIDATE_LINE_RESERVE_ENTRY,
    AI_CANDIDATE_LINE_HIGH_VARIANCE,
};

struct AiCandidateLine
{
    enum AiCandidateLineFamily family;
    enum AiShortHorizonLine line;
    u8 turn;
    bool32 stable;
};

struct AiBoardSnapshot
{
    enum BattlerId battlerAtk;
    enum BattlerId battlerDef;
    enum BattlerId partner;
    u32 aiReserveCount;
    u32 partnerReserveCount;
    u32 threatFlags;
    bool32 isValid;
    bool32 hasPartner;
    bool32 noReserve;
    bool32 perishTrapClock;
    bool32 knownKoPressure;
    bool32 opposingSetupPressure;
    bool32 targetSetupPressure;
    bool32 targetImmediateKoPressure;
    bool32 nearTermDamageClock;
    bool32 modeLoss;
};

struct AiShortHorizon
{
    struct AiBoardSnapshot snapshot;
    u32 lineFlags;
    u8 lossClock;
    bool32 isValid;
    bool32 stableLineAvailable;
    struct AiCandidateLine preferredStableLine;
    struct AiCandidateLine fallbackRiskLine;
};

static inline bool32 IsMoveUnusable(u32 moveIndex, enum Move move, u32 moveLimitations)
{
    return move == MOVE_NONE
        || move == MOVE_UNAVAILABLE
        || moveLimitations & 1u << moveIndex;
}

typedef bool32 (*MoveFlag)(enum Move move);

bool32 AI_IsFaster(enum BattlerId battlerAi, enum BattlerId battlerDef, enum Move aiMove, enum Move playerMove, enum ConsiderPriority considerPriority);
bool32 AI_IsSlower(enum BattlerId battlerAi, enum BattlerId battlerDef, enum Move aiMove, enum Move playerMove, enum ConsiderPriority considerPriority);
bool32 AI_RandLessThan(u32 val);
bool32 AI_IsBattlerGrounded(enum BattlerId battler);
enum MoveTarget AI_GetBattlerMoveTargetType(enum BattlerId battler, enum Move move);
bool32 AI_IsBattlerCommanderTatsugiri(enum BattlerId battler);
bool32 AI_ShouldAvoidCommanderTatsugiriTarget(enum BattlerId battlerDef, enum Move move);
enum Ability AI_GetMoldBreakerSanitizedAbility(enum BattlerId battlerAtk, enum Ability abilityAtk, enum Ability abilityDef, enum HoldEffect holdEffectDef, enum Move move);
u32 AI_GetDamage(enum BattlerId battlerAtk, enum BattlerId battlerDef, u32 moveIndex, enum DamageCalcContext calcContext, struct AiLogicData *aiData);
bool32 IsAiFlagPresent(u64 flag);
bool32 IsAiBattlerAware(enum BattlerId battlerId);
bool32 IsAiBattlerAssumingStab(enum BattlerId battlerId);
bool32 IsAiBattlerAssumingStatusMoves(enum BattlerId battlerId);
bool32 IsAiBattlerPredictingAbility(enum BattlerId battlerId);
bool32 ShouldRecordStatusMove(enum Move move);
void SaveBattlerData(enum BattlerId battlerId);
void SetBattlerData(enum BattlerId battlerId);
void SetBattlerAiData(enum BattlerId battler, struct AiLogicData *aiData);
void RestoreBattlerData(enum BattlerId battlerId);
enum Move GetAIChosenMove(enum BattlerId battlerId);
u32 GetTotalBaseStat(enum Species species);
bool32 IsTruantMonVulnerable(enum BattlerId battlerAI, enum BattlerId opposingBattler);
bool32 AI_BattlerAtMaxHp(enum BattlerId battler);
u32 GetHealthPercentage(enum BattlerId battler);
bool32 AI_CanBattlerEscape(enum BattlerId battler);
bool32 IsBattlerTrapped(enum BattlerId battlerAtk, enum BattlerId battlerDef);
s32 AI_WhoStrikesFirst(enum BattlerId battlerAI, enum BattlerId battler, enum Move aiMoveConsidered, enum Move playerMoveConsidered, enum ConsiderPriority considerPriority);
bool32 CanTargetFaintAi(enum BattlerId battlerDef, enum BattlerId battlerAtk);
u32 NoOfHitsForTargetToFaintBattler(enum BattlerId battlerDef, enum BattlerId battlerAtk, enum DamageCalcContext calcContext, enum AiConsiderEndure considerEndure);
void GetBestDmgMovesFromBattler(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum DamageCalcContext calcContext, enum Move *bestMoves);
u32 GetMoveIndex(enum BattlerId battler, enum Move move);
bool32 IsBestDmgMove(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum DamageCalcContext calcContext, enum Move move);
bool32 BestDmgMoveHasEffect(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum DamageCalcContext calcContext, enum BattleMoveEffects moveEffect);
u32 GetBestDmgFromBattler(enum BattlerId battler, enum BattlerId battlerTarget, enum DamageCalcContext calcContext);
bool32 CanTargetMoveFaintAi(enum Move move, enum BattlerId battlerDef, enum BattlerId battlerAtk, u32 nHits);
bool32 CanTargetFaintAiWithMod(enum BattlerId battlerDef, enum BattlerId battlerAtk, s32 hpMod, s32 dmgMod);
enum Ability AI_DecideKnownAbilityForTurn(enum BattlerId battlerId);
enum HoldEffect AI_DecideHoldEffectForTurn(enum BattlerId battlerId);
bool32 DoesBattlerIgnoreAbilityChecks(enum BattlerId battlerAtk, enum Ability atkAbility, enum Move move);
u32 AI_GetWeather(void);
u32 AI_GetSwitchinWeather(enum BattlerId battler);
u32 AI_GetSwitchinFieldStatus(enum BattlerId battler);
enum WeatherState IsWeatherActive(u32 flags);
bool32 CanAIFaintTarget(enum BattlerId battlerAtk, enum BattlerId battlerDef, u32 numHits);
bool32 CanIndexMoveFaintTarget(enum BattlerId battlerAtk, enum BattlerId battlerDef, u32 index, enum DamageCalcContext calcContext);
bool32 HasDamagingMove(enum BattlerId battler);
bool32 HasDamagingMoveOfType(enum BattlerId battler, enum Type type);
u32 GetBattlerSecondaryDamage(enum BattlerId battlerId);
bool32 BattlerWillFaintFromWeather(enum BattlerId battler, enum Ability ability);
bool32 BattlerWillFaintFromSecondaryDamage(enum BattlerId battler, enum Ability ability);
bool32 ShouldTryOHKO(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability atkAbility, enum Ability defAbility, enum Move move);
bool32 ShouldUseRecoilMove(enum BattlerId battlerAtk, enum BattlerId battlerDef, u32 recoilDmg, u32 moveIndex);
bool32 ShouldAbsorb(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move);
bool32 ShouldRecover(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, u32 healPercent);
bool32 ShouldSetScreen(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum BattleMoveEffects moveEffect);
bool32 ShouldCureStatus(enum BattlerId battlerAtk, enum BattlerId battlerDef, struct AiLogicData *aiData);
bool32 ShouldCureStatusWithItem(enum BattlerId battlerAtk, enum BattlerId battlerDef, struct AiLogicData *aiData);
enum AIPivot ShouldPivot(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move);
bool32 IsRecycleEncouragedItem(enum Item item);
bool32 ShouldRestoreHpBerry(enum BattlerId battlerAtk, enum Item item);
bool32 IsStatBoostingBerry(enum Item item);
bool32 CanKnockOffItem(enum BattlerId fromBattler, enum BattlerId battler, enum Item item);
bool32 IsAbilityOfRating(enum Ability ability, s32 rating);
bool32 AI_IsAbilityOnSide(enum BattlerId battlerId, enum Ability ability);
bool32 AI_MoveMakesContact(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability ability, enum HoldEffect holdEffect, enum Move move);
bool32 AI_CanContactBypassProtect(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move);
bool32 IsConsideringZMove(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move);
bool32 ShouldUseZMove(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move chosenMove);
void SetAIUsingGimmick(enum BattlerId battler, enum AIConsiderGimmick use);
bool32 IsAIUsingGimmick(enum BattlerId battler);
void DecideGimmickBeforeMoveSelection(enum BattlerId battler);
void ReconsiderSmartGimmick(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move);
void DecideTerastal(enum BattlerId battler);
bool32 CanEndureHit(enum BattlerId battler, enum BattlerId battlerTarget, enum Move move);
bool32 ShouldFinalGambit(enum BattlerId battlerAtk, enum BattlerId battlerDef, bool32 aiIsFaster);
bool32 ShouldConsiderSelfSacrificeDamageEffect(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, bool32 aiIsFaster);

// stat stage checks
bool32 AnyStatIsRaised(enum BattlerId battlerId);
bool32 AnyUsefulStatIsRaised(enum BattlerId battlerId);
bool32 CanLowerStat(enum BattlerId battlerAtk, enum BattlerId battlerDef, struct AiLogicData *aiData, enum Stat stat);
bool32 BattlerStatCanRise(enum BattlerId battler, enum Ability battlerAbility, enum Stat stat);
bool32 AreBattlersStatsMaxed(enum BattlerId battler);
u32 CountPositiveStatStages(enum BattlerId battlerId);
u32 CountNegativeStatStages(enum BattlerId battlerId);

// move checks
bool32 Ai_IsPriorityBlocked(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, struct AiLogicData *aiData);
bool32 AI_CanMoveBeBlockedByTarget(struct DamageContext *ctx);
bool32 MovesWithCategoryUnusable(u32 attacker, u32 target, enum DamageCategory category);
enum MoveComparisonResult CompareMoveEffects(enum Move move1, enum Move move2, enum BattlerId battlerAtk, enum BattlerId battlerDef, s32 noOfHitsToKo);
struct SimulatedDamage AI_CalcDamageSaveBattlers(enum Move move, enum BattlerId battlerAtk, enum BattlerId battlerDef, uq4_12_t *typeEffectiveness, enum AIConsiderGimmick considerGimmickAtk, enum AIConsiderGimmick considerGimmickDef);
bool32 IsAdditionalEffectBlocked(enum BattlerId battlerAtk, u32 abilityAtk, enum BattlerId battlerDef, enum Ability abilityDef);
struct SimulatedDamage AI_CalcDamage(enum Move move, enum BattlerId battlerAtk, enum BattlerId battlerDef, uq4_12_t *typeEffectiveness, enum AIConsiderGimmick considerGimmickAtk, enum AIConsiderGimmick considerGimmickDef, u32 weather, u32 fieldStatuses);
bool32 AI_IsDamagedByRecoil(enum BattlerId battler);
u32 GetNoOfHitsToKO(u32 dmg, s32 hp);
u32 GetNoOfHitsToKOBattlerDmg(u32 dmg, enum BattlerId battlerDef);
u32 GetNoOfHitsToKOBattler(enum BattlerId battlerAtk, enum BattlerId battlerDef, u32 moveIndex, enum DamageCalcContext calcContext, enum AiConsiderEndure considerEndure);
u32 GetBestNoOfHitsToKO(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum DamageCalcContext calcContext);
u32 GetCurrDamageHpPercent(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum DamageCalcContext calcContext);
uq4_12_t AI_GetMoveEffectiveness(enum Move move, enum BattlerId battlerAtk, enum BattlerId battlerDef);
enum Move *GetMovesArray(enum BattlerId battler);
bool32 IsConfusionMoveEffect(enum BattleMoveEffects moveEffect);
bool32 HasMove(enum BattlerId battlerId, enum Move move);
u32 GetBattlerMoveIndexWithEffect(enum BattlerId battler, enum BattleMoveEffects effect);
bool32 ShouldBeatUpForJustified(enum BattlerId battlerAtk, enum BattlerId battlerAtkPartner, enum Move move, enum Type moveType, bool32 wouldPartnerFaint, struct AiLogicData *aiData);
bool32 ShouldBeatUpForRageFist(enum BattlerId battlerAtkPartner, enum Move move, bool32 wouldPartnerFaint, struct AiLogicData *aiData);
bool32 HasPhysicalBestMove(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum DamageCalcContext calcContext);
bool32 HasOnlyMovesWithCategory(enum BattlerId battlerId, enum DamageCategory category, bool32 onlyOffensive);
bool32 HasMoveWithCategory(enum BattlerId battler, enum DamageCategory category);
bool32 HasMoveWithType(enum BattlerId battler, enum Type type);
bool32 HasMoveWithEffect(enum BattlerId battler, enum BattleMoveEffects moveEffect);
bool32 HasMoveWithAIEffect(enum BattlerId battler, u32 aiEffect);
bool32 HasBattlerSideMoveWithEffect(enum BattlerId battler, enum BattleMoveEffects effect);
bool32 HasBattlerSideMoveWithAIEffect(enum BattlerId battler, u32 effect);
bool32 HasBattlerSideUsedMoveWithEffect(enum BattlerId battler, enum BattleMoveEffects effect);
bool32 HasNonVolatileMoveEffect(enum BattlerId battlerId, enum MoveEffect effect);
bool32 IsPowerBasedOnStatus(enum BattlerId battlerId, enum BattleMoveEffects effect, u32 argument);
bool32 HasMoveWithAdditionalEffect(enum BattlerId battlerId, enum MoveEffect moveEffect);
bool32 HasBattlerSideMoveWithAdditionalEffect(enum BattlerId battler, enum MoveEffect moveEffect);
bool32 HasMoveWithCriticalHitChance(enum BattlerId battlerId);
bool32 HasMoveWithMoveEffectExcept(enum BattlerId battlerId, enum MoveEffect moveEffect, enum BattleMoveEffects exception);
bool32 HasMoveThatLowersOwnStats(enum BattlerId battlerId);
bool32 HasMoveWithLowAccuracy(enum BattlerId battlerAtk, enum BattlerId battlerDef, u32 accCheck, bool32 ignoreStatus);
bool32 HasAnyKnownMove(enum BattlerId battlerId);
bool32 IsAromaVeilProtectedEffect(enum BattleMoveEffects moveEffect);
bool32 IsNonVolatileStatusMove(enum Move move);
bool32 IsMoveRedirectionPrevented(enum BattlerId battlerAtk, enum Move move, enum Ability atkAbility);
bool32 IsHazardMove(enum Move move);
bool32 IsTwoTurnNotSemiInvulnerableMove(enum BattlerId battlerAtk, enum Move move);
bool32 IsBattlerDamagedByStatus(enum BattlerId battler);
bool32 BattlerHasOffensiveSetup(enum BattlerId battler);
bool32 IsReadPlayerSelectedOffensiveSetupThreat(enum BattlerId battlerAtk, enum BattlerId battlerDef);
bool32 IsOpposingSideOffensiveSetupThreat(enum BattlerId battlerAtk);
bool32 AI_BuildBoardSnapshot(enum BattlerId battlerAtk, enum BattlerId battlerDef, struct AiBoardSnapshot *snapshot);
u32 AI_ClassifyBoardThreat(const struct AiBoardSnapshot *snapshot);
bool32 AI_BoardHasThreat(const struct AiBoardSnapshot *snapshot, enum AiThreatClass threat);
bool32 AI_EvaluateShortHorizon(enum BattlerId battlerAtk, enum BattlerId battlerDef, struct AiShortHorizon *horizon);
bool32 AI_ShortHorizonHasLine(const struct AiShortHorizon *horizon, enum AiShortHorizonLine line);
bool32 AI_RiskGovernorAllows(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum AiRiskKind riskKind);
bool32 AI_IsNearTermDesperationPressure(enum BattlerId battlerAtk, enum BattlerId battlerDef);
bool32 AI_ShouldAcceptDesperationRisk(enum BattlerId battlerAtk, enum BattlerId battlerDef);
bool32 ShouldUseSinglesProtect(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move predictedMove);
u32 Test_GetProtectEndTurnRecovery(enum BattlerId battler);
s32 ProtectChecks(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, enum Move predictedMove);
bool32 ShouldRaiseAnyStat(enum BattlerId battlerAtk, enum BattlerId battlerDef);
bool32 ShouldSetWeather(enum BattlerId battler, u32 weather);
bool32 ShouldClearWeather(enum BattlerId battler, u32 weather);
bool32 ShouldSetFieldStatus(enum BattlerId battler, u32 fieldStatus);
bool32 ShouldClearFieldStatus(enum BattlerId battler, u32 fieldStatus);
bool32 HasSleepMoveWithLowAccuracy(enum BattlerId battlerAtk, enum BattlerId battlerDef);
bool32 HasHealingEffect(enum BattlerId battler);
bool32 IsTrappingMove(enum Move move);
bool32 HasTrappingMoveEffect(enum BattlerId battler);
bool32 IsFlinchGuaranteed(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move);
bool32 HasChoiceEffect(enum BattlerId battler);
bool32 HasThawingMove(enum BattlerId battler);
bool32 HasMoveUsableWhileAsleep(enum BattlerId battler);
bool32 IsStatRaisingMove(enum Move move);
bool32 IsOffensiveStatRaisingMove(enum Move move);
bool32 IsStatLoweringMove(enum Move move);
bool32 IsSwitchOutEffect(enum BattleMoveEffects effect);
bool32 IsChaseEffect(enum BattleMoveEffects effect);
bool32 IsAttackBoostMoveEffect(enum BattleMoveEffects effect);
bool32 IsUngroundingEffect(enum BattleMoveEffects effect);
bool32 HasMoveWithFlag(enum BattlerId battler, MoveFlag getFlag);
bool32 IsHazardClearingMove(enum Move move);
bool32 IsSubstituteEffect(enum BattleMoveEffects effect);
bool32 IsSelfSacrificeEffect(enum Move move);
u32 GetAIExplosionChanceFromHP(u32 hpPercent);

// status checks
bool32 AI_CanBeConfused(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, enum Ability ability);
bool32 IsBattlerIncapacitated(enum BattlerId battler, enum Ability ability);
bool32 AI_CanPutToSleep(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability defAbility, enum Move move, enum Move partnerMove);
bool32 ShouldPoison(enum BattlerId battlerAtk, enum BattlerId battlerDef);
bool32 AI_CanPoison(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability defAbility, enum Move move, enum Move partnerMove);
bool32 AI_CanParalyze(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability defAbility, enum Move move, enum Move partnerMove);
bool32 AI_CanConfuse(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability defAbility, enum BattlerId battlerAtkPartner, enum Move move, enum Move partnerMove);
bool32 ShouldBurn(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability abilityDef);
bool32 ShouldFreezeOrFrostbite(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability abilityDef);
bool32 ShouldParalyze(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability abilityDef);
bool32 AI_CanBurn(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability defAbility, enum BattlerId battlerAtkPartner, enum Move move, enum Move partnerMove);
bool32 AI_CanGiveFrostbite(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability defAbility, enum BattlerId battlerAtkPartner, enum Move move, enum Move partnerMove);
bool32 AI_CanBeInfatuated(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability defAbility);
bool32 AnyPartyMemberStatused(enum BattlerId battlerId, bool32 checkSoundproof);
bool32 ShouldTryToFlinch(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability atkAbility, enum Ability defAbility, enum Move move);
bool32 ShouldTrap(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move);
bool32 IsWakeupTurn(enum BattlerId battler);
bool32 AI_IsBattlerAsleepOrComatose(enum BattlerId battlerId);

// ability logic
bool32 IsMoxieTypeAbility(enum Ability ability);
bool32 DoesAbilityRaiseStatsWhenLowered(enum Ability ability);
bool32 DoesIntimidateRaiseStats(enum Ability ability);
bool32 ShouldTriggerAbility(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability ability);
bool32 CanEffectChangeAbility(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, struct AiLogicData *aiData);
void AbilityChangeScore(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, s32 *score, struct AiLogicData *aiData);
enum AIScore BattlerBenefitsFromAbilityScore(enum BattlerId battler, enum Ability ability, struct AiLogicData *aiData);

// partner logic
bool32 IsTargetingPartner(enum BattlerId battlerAtk, enum BattlerId battlerDef);
// IsTargetingPartner includes a check to make sure the adjacent Pokémon is truly a partner.
enum Move GetAllyChosenMove(enum BattlerId battlerId);
bool32 IsBattle1v1(void);
// IsBattle1v1 is distinct from !IsDoubleBattle. If the player is fighting Maxie and Tabitha, with Steven as their partner, and both Tabitha and Steven have run out of Pokemon, the battle is 1v1, even though mechanically it is a Double Battle for how battlers and flags are set.
// Most AI checks should be using IsBattle1v1; most engine checks should be using !IsDoubleBattle
bool32 HasTwoOpponents(enum BattlerId battler);
// HasTwoOpponents checks if the opposing side has two Pokémon. Partner state is irrelevant. e.g., Dragon Darts hits one time with two opponents and twice with one opponent.
bool32 HasPartner(enum BattlerId battler);
bool32 HasPartnerIgnoreFlags(enum BattlerId battler);
// HasPartner respects the Attacks Partner AI flag; HasPartnerIgnoreFlags checks only if a live Pokémon is adjacent.
bool32 AreMovesEquivalent(enum BattlerId battlerAtk, enum BattlerId battlerAtkPartner, enum Move move, enum Move partnerMove);
bool32 DoesPartnerHaveSameMoveEffect(enum BattlerId battlerAtkPartner, enum BattlerId battlerDef, enum Move move, enum Move partnerMove);
bool32 PartnerMoveEffectIsStatusSameTarget(enum BattlerId battlerAtkPartner, enum BattlerId battlerDef, enum Move partnerMove);
bool32 PartnerMoveEffectIs(enum BattlerId battlerAtkPartner, enum Move partnerMove, enum BattleMoveEffects effectCheck);
bool32 PartnerMoveIs(enum BattlerId battlerAtkPartner, enum Move partnerMove, enum Move moveCheck);
bool32 PartnerMoveIsSameAsAttacker(enum BattlerId battlerAtkPartner, enum BattlerId battlerDef, enum Move move, enum Move partnerMove);
bool32 PartnerMoveIsSameNoTarget(enum BattlerId battlerAtkPartner, enum Move move, enum Move partnerMove);
bool32 PartnerMoveActivatesSleepClause(enum Move partnerMove);
bool32 ShouldUseWishAromatherapy(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move);
u32 GetFriendlyFireKOThreshold(enum BattlerId battler);
bool32 IsAllyProtectingFromMove(enum BattlerId battlerAtk, enum Move attackerMove, enum Move allyMove);

// party logic
struct BattlePokemon *AllocSaveBattleMons(void);
void FreeRestoreBattleMons(struct BattlePokemon *savedBattleMons);
struct AiLogicData *AllocSaveAiLogicData(void);
void FreeRestoreAiLogicData(struct AiLogicData *savedAiLogicData);
s32 CountUsablePartyMons(enum BattlerId battlerId);
bool32 IsPartyFullyHealedExceptBattler(enum BattlerId battler);
bool32 PartyHasMoveCategory(enum BattlerId battlerId, enum DamageCategory category);
bool32 SideHasMoveCategory(enum BattlerId battlerId, enum DamageCategory category);
s32 GetAILastPartyIndex(enum BattlerId battler);
u32 GetActiveBattlerIds(enum BattlerId battler, enum BattlerId *battlerIn1, enum BattlerId *battlerIn2);
bool32 IsPartyMonOnFieldOrChosenToSwitch(enum BattlerId battler, u32 partyIndex, enum BattlerId battlerIn1, enum BattlerId battlerIn2);
bool32 IsPartyMonPlannedToBeSwitchedInByPartner(u32 partyIndex, enum BattlerId battler);
s32 GetStatChangeScore(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move);
s32 GetSelfStatChangeScore(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move);
s32 GetFoeStatChangeScore(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move);
s32 GetAllyStatChangeScore(u32 battlerAtk, u32 partner, u32 move);

// score increases
enum AIScore IncreaseStatUpScore(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Stat stat, s32 stage);
enum AIScore IncreaseStatUpScoreContrary(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Stat stat, s32 stage);
enum AIScore IncreaseStatDownScore(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Stat stat);
void IncreasePoisonScore(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, s32 *score);
void IncreaseBurnScore(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, s32 *score);
void IncreaseParalyzeScore(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, s32 *score);
void IncreaseSleepScore(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, s32 *score);
void IncreaseConfusionScore(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, s32 *score);
void IncreaseFrostbiteScore(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, s32 *score);
bool32 HasHPForDamagingSetup(enum BattlerId battlerAtk, enum BattlerId battlerDef, u32 hpThreshold);

s32 AI_TryToClearStats(enum BattlerId battlerAtk, enum BattlerId battlerDef, bool32 isDoubleBattle);
bool32 AI_ShouldCopyStatChanges(enum BattlerId battlerAtk, enum BattlerId battlerDef);
bool32 AI_ShouldSetUpHazards(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, struct AiLogicData *aiData);
void IncreaseTidyUpScore(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, s32 *score);
bool32 AI_ShouldSpicyExtract(enum BattlerId battlerAtk, enum BattlerId battlerAtkPartner, enum Move move, struct AiLogicData *aiData);
u32 IncreaseSubstituteMoveScore(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move);
bool32 IsBattlerItemEnabled(enum BattlerId battler);
bool32 IsBattlerPredictedToSwitch(enum BattlerId battler);
enum Move GetIncomingMove(enum BattlerId battler, enum BattlerId opposingBattler, struct AiLogicData *aiData);
enum Move GetPredictedMove(enum BattlerId battler, enum BattlerId opposingBattler, struct AiLogicData *aiData);
u32 AI_GetMoveKnowledgeFlags(enum Move move);
bool32 AI_MoveHasKnowledgeFlag(enum Move move, u32 flag);
bool32 AI_IsMoveAbilityControl(enum Move move);
bool32 AI_IsMoveDenial(enum Move move);
bool32 AI_IsMoveComboState(enum Move move);
u32 AI_GetAbilityKnowledgeFlags(enum Ability ability);
bool32 AI_AbilityHasKnowledgeFlag(enum Ability ability, u32 flag);
u32 AI_GetHoldEffectKnowledgeFlags(enum HoldEffect holdEffect);
bool32 AI_HoldEffectHasKnowledgeFlag(enum HoldEffect holdEffect, u32 flag);
u32 AI_GetItemKnowledgeFlags(enum Item item);
bool32 AI_ItemHasKnowledgeFlag(enum Item item, u32 flag);
bool32 AI_CanBattlerIgnorePredictedMove(enum BattlerId battlerDef, enum BattlerId battlerAtk, enum Move move);
bool32 AI_OpponentCanFaintAiWithMod(enum BattlerId battler, u32 healAmount);
bool32 ShouldInstructPartner(enum BattlerId partner, enum Move move);
bool32 CanMoveBeBouncedBack(enum BattlerId battler, enum Move move);
bool32 AI_CanAnyStatChange(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move);

// Switching and item helpers
bool32 AiExpectsToFaintPlayer(enum BattlerId battler);

// These are for the purpose of not doubling up on moves during double battles.
// Used in GetAIEffectGroup for move effects and GetAIEffectGroupFromMove for additional effects
#define AI_EFFECT_NONE                        0
#define AI_EFFECT_WEATHER              (1 <<  0)
#define AI_EFFECT_TERRAIN              (1 <<  1)
#define AI_EFFECT_CLEAR_HAZARDS        (1 <<  2)
#define AI_EFFECT_BREAK_SCREENS        (1 <<  3)
#define AI_EFFECT_RESET_STATS          (1 <<  4)
#define AI_EFFECT_FORCE_SWITCH         (1 <<  5)
#define AI_EFFECT_TORMENT              (1 <<  6)
#define AI_EFFECT_LIGHT_SCREEN         (1 <<  7)
#define AI_EFFECT_REFLECT              (1 <<  8)
#define AI_EFFECT_GRAVITY              (1 <<  9)
#define AI_EFFECT_CHANGE_ABILITY       (1 << 10)

// As Aurora Veil should almost never be used alongside the other screens, we save the bit.
#define AI_EFFECT_AURORA_VEIL          (AI_EFFECT_LIGHT_SCREEN | AI_EFFECT_REFLECT)

#endif //GUARD_BATTLE_AI_UTIL_H
