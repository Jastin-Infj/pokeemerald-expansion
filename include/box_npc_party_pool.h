#ifndef GUARD_BOX_NPC_PARTY_POOL_H
#define GUARD_BOX_NPC_PARTY_POOL_H

#define BOX_NPC_POOL_BOX_ID 0
#define BOX_NPC_CANDIDATE_ROSTER_SIZE 6
#define BOX_NPC_SINGLE_BATTLE_SIZE 3
#define BOX_NPC_DOUBLE_BATTLE_SIZE 4
#define BOX_NPC_SLOT_NONE 0xFF

enum BoxNpcPartyPoolMode
{
    BOX_NPC_POOL_BOX1_SLOTS_1_TO_6,
    BOX_NPC_POOL_BOX1_FIRST_VALID_6,
    BOX_NPC_POOL_BOX1_RANDOM_VALID_6,
    BOX_NPC_POOL_REGISTERED_BATTLE_TEAM,
};

enum BoxNpcBattleFormat
{
    BOX_NPC_BATTLE_SINGLE_3,
    BOX_NPC_BATTLE_DOUBLE_4,
};

enum BoxNpcBattleMemberMode
{
    BOX_NPC_BATTLE_MEMBERS_FIRST_N,
    BOX_NPC_BATTLE_MEMBERS_RANDOM_N,
};

enum BoxNpcOpponentGimmickPolicy
{
    BOX_NPC_GIMMICK_NATURAL,
    BOX_NPC_GIMMICK_ALLOW_TERA_DYNAMAX_ALL_FINAL_MEMBERS,
};

enum BoxNpcPartyPoolError
{
    BOX_NPC_PARTY_POOL_ERROR_NONE,
    BOX_NPC_PARTY_POOL_ERROR_STORAGE_UNAVAILABLE,
    BOX_NPC_PARTY_POOL_ERROR_FIXED_SLOT_INVALID,
    BOX_NPC_PARTY_POOL_ERROR_NOT_ENOUGH_VALID_MONS,
    BOX_NPC_PARTY_POOL_ERROR_INVALID_BATTLE_TEAM,
    BOX_NPC_PARTY_POOL_ERROR_BATTLE_TEAM_INCOMPLETE,
    BOX_NPC_PARTY_POOL_ERROR_NOT_ENOUGH_PLAYER_MONS,
};

struct BoxNpcPartyPoolConfig
{
    enum BoxNpcPartyPoolMode poolMode;
    enum BoxNpcBattleFormat battleFormat;
    enum BoxNpcBattleMemberMode memberMode;
    enum BoxNpcOpponentGimmickPolicy gimmickPolicy;
    u8 battleTeamId;
    u64 aiFlags;
};

struct BoxNpcPartyPoolResult
{
    struct BattleTeamSlot candidateSources[BOX_NPC_CANDIDATE_ROSTER_SIZE];
    struct BattleTeamSlot finalSources[PARTY_SIZE];
    u8 candidateCount;
    u8 battleCount;
    u8 battleTeamId;
    enum BoxNpcPartyPoolMode poolMode;
    enum BoxNpcBattleFormat battleFormat;
    enum BoxNpcBattleMemberMode memberMode;
    enum BoxNpcOpponentGimmickPolicy gimmickPolicy;
};

bool32 BoxNpcPartyPool_TryBuildOpponentParty(
    const struct BoxNpcPartyPoolConfig *config,
    struct BoxNpcPartyPoolResult *result);
bool32 BoxNpcPartyPool_TryStagePlayerParty(u8 battleCount);
bool32 BoxNpcPartyPool_RestorePlayerParty(void);
bool32 BoxNpcPartyPool_IsPlayerPartyStaged(void);
void BoxNpcPartyPool_ApplyPendingBattleInitPolicy(void);
void BoxNpcPartyPool_ClearPendingBattleInitPolicy(void);
const struct BoxNpcPartyPoolResult *BoxNpcPartyPool_GetLastResult(void);
enum BoxNpcPartyPoolError BoxNpcPartyPool_GetLastError(void);
const u8 *BoxNpcPartyPool_GetLastErrorText(void);

#endif // GUARD_BOX_NPC_PARTY_POOL_H
