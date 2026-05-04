# Roguelike Runtime Party Generation v15

調査日: 2026-05-04。実装は未着手。`docs/` への記録のみ。

## Purpose

Rogue モードの「対戦相手のパーティを runtime で生成する」設計を、既存コードのどこに乗せるかという観点で整理する。具体的には:

- `struct Trainer` / `.party` / `gTrainers[][]` のパイプライン
- `TRAINER_BATTLE_PARAM.opponentA` 起点の battle 開始経路
- `gIsDebugBattle` / `GetDebugAiTrainer()` という既存の **runtime trainer 差し替え** hook
- Battle Frontier / Battle Tower / Battle Factory が使う facility roster 経路
- 上記をどう Rogue に流用するかの 3 案比較

関連 doc: [Roguelike NPC Capacity Impact v15](roguelike_npc_capacity_v15.md)、[Roguelike Party Policy Impact v15](roguelike_party_policy_impact_v15.md)、[Trainer Battle Flow v15](../flows/trainer_battle_flow_v15.md)、[Battle Frontier Level Scaling Flow v15](../flows/battle_frontier_level_scaling_flow_v15.md)、[Battle AI Decision Flow v15](../flows/battle_ai_decision_flow_v15.md)。

## TL;DR

1. **`GetTrainerStructFromId()` には既に runtime hook が入っている**。debug menu はここで `gIsDebugBattle` をフックして、EWRAM 上の `struct Trainer` を返している ([include/data.h:253-269](../../include/data.h#L253-L269), [src/debug.c:4897-4900](../../src/debug.c#L4897-L4900))。Rogue も**同じ穴に乗せれば**、battle 開始経路を一切再発明せずに「runtime で組んだ Trainer」を AI / battle UI / party generator に流せる。
2. **`struct Trainer.poolSize` / `poolRuleIndex` / `poolPickIndex` / `poolPruneIndex` が既に roguelike 的な pool 抽選を持っている** ([include/data.h:115-135](../../include/data.h#L115-L135), [src/trainer_pools.c:370](../../src/trainer_pools.c#L370))。Trainer の `.party[]` を「使う pool」とし、`partySize < poolSize` で「pool から partySize 体を抽選」できる。Rogue 用の prune 関数 (`POOL_PRUNE_*`) を 1 つ足すだけで、難度別 / wave 別の選抜を表現できる。
3. **Battle Frontier 経路も併用できる**。`TRAINER_BATTLE_PARAM.opponentA < FRONTIER_TRAINERS_COUNT (=300)` の値が来ると、battle_frontier.c 側の `FillTrainerParty` が `gFacilityTrainers[]` + `gFacilityTrainerMons[]` から動的に組む ([src/battle_frontier.c:202-307](../../src/battle_frontier.c#L202-L307))。ただし Frontier 経路は species/held item の duplicate ban、固定 IV、3 体パーティといった独自ルールが付随するので、Rogue 用 rule に引きずられる場合は不向き。
4. **gTrainers[][] は 3 difficulty × 855 trainer = 2565 entries**。Rogue NPC を全員ここに登録するのは現実的でない。Trainer struct ベースの `overrideTrainer` か、上記 Trainer pool、または GetTrainerStructFromId hook で virtual slot 1 個に集約すべき。

推奨: **GetTrainerStructFromId hook + Trainer pool の組み合わせ**。Rogue 専用 SaveBlock のキャパシティ doc と整合的で、`gTrainers[][]` に登録する slot は 1 個 (sentinel として使う) に抑えられる。

## Trainer Data Pipeline

### 静的データソース

```text
src/data/trainers.party
src/data/trainers_frlg.party
src/data/battle_partners.party
src/data/debug_trainers.party
```

これらが `tools/trainerproc` でコンパイル時に変換され:

```text
src/data/trainers.h            (gTrainers[DIFFICULTY_NORMAL][...])
src/data/trainers_frlg.h
src/data/debug_trainers.h      (sDebugTrainers[][])
src/data/battle_partners.h     (gBattlePartners[][])
```

`.party` の例 ([src/data/debug_trainers.party](../../src/data/debug_trainers.party)):

```text
=== DEBUG_TRAINER_AI ===
Name: Debugger
AI: Smart Trainer
Class: Rival
Battle Type: Singles
Pic: Steven
Gender: Male
Music: Male

Metang
Brave Nature
Level: 42
IVs: 31 HP / 31 Atk / 31 Def / 31 SpA / 31 SpD / 31 Spe
EVs: 252 Atk / 252 Def / 6 SpA
- Light Screen
- Psychic
...
```

これは **手書き保守は重い**。Rogue で 100〜300 体に同じ format で書くと:

- 1 trainer ≒ 30〜60 行 → 300 trainer = 9000〜18000 行
- 微調整 (level curve、AI flag set、pool rule) を全 trainer に流し込む作業が膨大
- `gTrainers[3][TRAINERS_COUNT]` で 3 difficulty 分が ROM に乗るので、メモリ消費も等比

### `struct Trainer` の主要フィールド

[include/data.h:115-135](../../include/data.h#L115-L135):

| Field | 用途 | Rogue 観点 |
|---|---|---|
| `aiFlags` (u64) | AI 動作 (Smart / Risky / Acepokemon / RANDOMIZE_PARTY_INDICES …) | wave 帯ごとに切り替えたい。ここを runtime で書ければ AI 強度を Rogue が制御できる。 |
| `party` (`const struct TrainerMon *`) | 静的なモンスター配列 | Rogue 側は **const ROM の pool 配列** を指させる。 |
| `partySize` | 実際に使う体数 | 普段は 1〜6。Rogue は wave に応じて変える。 |
| `poolSize` | `party[]` の総サイズ | `poolSize > partySize` ならランタイム抽選。Rogue にぴったり。 |
| `poolRuleIndex` | `gPoolRulesetsList[]` から rule set を選ぶ | Rogue 用 rule set を `trainer_pools.c` 側に追加する形。 |
| `poolPickIndex` | lead / ace / other の選び方 | デフォルト or LOWEST。Rogue で別を作ってもよい。 |
| `poolPruneIndex` | pool 抽選前の pruning | `POOL_PRUNE_RANDOM_TAG` のように **runtime 風の挙動** が既に実装済み。 |
| `overrideTrainer` | `partySize == 0` の時に他 trainer の party を借りる | 同じ pool を多数の Trainer entry で共有できる。 |
| `trainerClass` / `trainerPic` / `trainerName` / `mugshotColor` | 表示メタデータ | per-NPC で変えたいので runtime 差し替え対象。 |

### `gTrainers[][TRAINERS_COUNT]` のサイズ

| Constant | 値 | Source |
|---|---|---|
| `DIFFICULTY_COUNT` | 3 (`EASY`, `NORMAL`, `HARD`) | [include/constants/difficulty.h:4-10](../../include/constants/difficulty.h#L4-L10) |
| `TRAINERS_COUNT` (Emerald) | 855 | [include/constants/opponents.h:867](../../include/constants/opponents.h#L867) |
| `MAX_TRAINERS_COUNT` | 864 | 末尾 9 が unused |
| `gTrainers[][]` 実体 | 3 × 855 = **2565 Trainer entries** | [include/data.h:208](../../include/data.h#L208) |
| `FRONTIER_TRAINERS_COUNT` | 300 | [include/constants/battle_frontier_trainers.h:305](../../include/constants/battle_frontier_trainers.h#L305) (偶然 Rogue 目標と一致) |

300 体ぶん `.party` を書く負担は、Frontier の trainer / mon set 表 (300 trainer × monSet) に近い。Frontier はこれを「facilityClass + name + speech + monSet ポインタ」の小さな struct + 共有 mon table で対処している。Rogue も同方向で組むのが自然。

## Battle Entry Path (TRAINER_BATTLE_PARAM)

### TRAINER_BATTLE_PARAM の正体

[include/battle_setup.h:20-51](../../include/battle_setup.h#L20-L51):

```c
typedef union PACKED TrainerBattleParameter
{
    struct PACKED _TrainerBattleParameter
    {
        u8 isDoubleBattle:1;
        u8 isRematch:1;
        u8 playMusicA:1;
        u8 playMusicB:1;
        u8 mode:4;
        u8 objEventLocalIdA;
        u16 opponentA;          // ← ここに gTrainers[] の index が入る
        ...
        u16 opponentB;
        ...
    } params;
    u8 data[sizeof(struct _TrainerBattleParameter)];
} TrainerBattleParameter;

#define TRAINER_BATTLE_PARAM gTrainerBattleParameter.params
```

スクリプトの `trainerbattle TRAINER_BATTLE_SINGLE, ..., \trainer, ...` は最終的に `BattleSetup_ConfigureTrainerBattle` ([src/battle_setup.c:1115](../../src/battle_setup.c#L1115)) を経由して `TRAINER_BATTLE_PARAM.opponentA` を埋めるだけ。Rogue が独自スクリプトコマンド (`callnative` あるいは新規 `setrogueopponent`) で `opponentA` を直接書ければ、battle 開始経路は既存の `BattleSetup_StartTrainerBattle` をそのまま使える。

### Battle 開始の主経路

[src/battle_setup.c:1300](../../src/battle_setup.c#L1300) の `BattleSetup_StartTrainerBattle()`:

1. `gNoOfApproachingTrainers` から double / single を判定し `gBattleTypeFlags` を組む。
2. Battle Pyramid / Trainer Hill 中なら `FillFrontierTrainerParty` 系で `gEnemyParty[]` を埋める。
3. それ以外なら `DoTrainerBattle()` → `CB2_HandleStartBattle` → battle_main.c へ。
4. battle_main.c::`InitBattle` の中で `CreateNPCTrainerParty(&gEnemyParty[0], TRAINER_BATTLE_PARAM.opponentA, TRUE)` ([battle_main.c:600](../../src/battle_main.c#L600)) が走り、`GetTrainerStructFromId()` 経由で `struct Trainer` を取り出してパーティを組む。

このルートに乗ると、`.opponentA` が `gTrainers[][]` の index として解釈される。Rogue が独自 trainer struct を渡したい場合の選択肢は:

- A) `gIsDebugBattle = TRUE` 相当のフラグを立て、`GetTrainerStructFromId()` が EWRAM の trainer struct を返すよう hook する (debug 経路と同じ)。
- B) battle 開始前に手動で `CreateNPCTrainerPartyFromTrainer(gEnemyParty, customTrainer, ...)` を呼んで `gEnemyParty[]` を構築し、`battle_main.c::InitBattle` の `CreateNPCTrainerParty` 呼び出しを `gIsDebugBattle` でスキップさせる ([battle_main.c:596-606](../../src/battle_main.c#L596-L606))。

debug 経路はまさに B) を組み合わせて使っている: `DebugAction_Party_BattleSingle` ([src/debug.c:4910](../../src/debug.c#L4910)) が `CreateNPCTrainerPartyFromTrainer(gEnemyParty, GetDebugAiTrainer(), FALSE, BATTLE_TYPE_TRAINER)` を呼んで `gIsDebugBattle = TRUE` にしてから `BattleSetup_StartTrainerBattle_Debug()` を呼ぶ。

### `BattleSetup_StartTrainerBattle_Debug` の役割

[src/battle_setup.c:1392](../../src/battle_setup.c#L1392) は通常版から `gNoOfApproachingTrainers` 周りをスキップして直接 `DoTrainerBattle` を呼ぶ debug 専用路。Rogue でも overworld の "trainer が近づく" 演出を省いて即 battle に入る局面 (e.g. 即時 reroll 戦) では、これに似た自前経路を用意したくなる。

## Frontier / Tower / Factory Roster Pattern

Frontier 系は `gTrainers[][]` を経由せずに別パイプラインで動く。学べる構造が多いので整理する。

### `gFacilityTrainers[]` と `gFacilityTrainerMons[]`

[include/battle_frontier.h:7-25](../../include/battle_frontier.h#L7-L25):

```c
struct BattleFrontierTrainer
{
    u8 facilityClass;
    u8 filler1[3];
    u8 trainerName[PLAYER_NAME_LENGTH + 1];
    u16 speechBefore[EASY_CHAT_BATTLE_WORDS_COUNT];
    u16 speechWin[EASY_CHAT_BATTLE_WORDS_COUNT];
    u16 speechLose[EASY_CHAT_BATTLE_WORDS_COUNT];
    const u16 *monSet;          // ← gFacilityTrainerMons[] のインデックス配列。0xFFFF 終端
};

extern const struct BattleFrontierTrainer *gFacilityTrainers;
extern const struct TrainerMon *gFacilityTrainerMons;
```

特徴:

- `struct Trainer` よりずっと軽量 (約 36 byte)。300 個でも約 11 KB ROM。
- `monSet` は `const u16[]` の 0xFFFF 終端配列。「この trainer はこの pool から選ぶ」を表現する。
- `gFacilityTrainerMons[]` は species / moves / item / nature / IV を持つ TrainerMon 配列。pool は **複数の trainer で共有** されている。例えば `FRONTIER_MONS_YOUNGSTER_LASS_1` を多数の Youngster trainer が共有 ([src/data/battle_frontier/battle_frontier_trainers.h](../../src/data/battle_frontier/battle_frontier_trainers.h))。

### `FillTrainerParty` の抽選ループ

[src/battle_frontier.c:202](../../src/battle_frontier.c#L202) は概ね次:

```c
for (bfMonCount = 0; monSet[bfMonCount] != 0xFFFF; bfMonCount++) ;
// bfMonCount = pool size
i = 0;
while (i != monCount)
{
    u16 monId = monSet[Random() % bfMonCount];

    // open level only filter
    if ((level == FRONTIER_MAX_LEVEL_50 || level == 20) && monId > FRONTIER_MONS_HIGH_TIER) continue;

    // species duplicate check
    // held item duplicate check
    // index duplicate check

    chosenMonIndices[i] = monId;
    CreateFacilityMon(&gFacilityTrainerMons[monId], level, fixedIV, otID, 0, &gEnemyParty[i + firstMonId]);
    i++;
}
```

Rogue が再利用する時の注意点:

| Rule | Frontier の振る舞い | Rogue で同じにする? |
|---|---|---|
| Species duplicate ban | 同じ species は採用しない | Rogue は wave で同じ species が出ても OK にしたいかも (例: ボス前の "群れ" 演出)。 |
| Held item duplicate ban | 同じ held item を 2 体持たせない | Rogue は item rule policy で切り替えたい。 |
| Tier filter (`FRONTIER_MONS_HIGH_TIER`) | level 50 / 20 では high tier 不採用 | Rogue は wave 進行で解禁する独自テーブルが要る。 |
| Fixed IV (`GetFrontierTrainerFixedIvs`) | trainer ID 帯で IV を分ける | Rogue は run seed + wave で計算したい。 |
| Level | `SetFacilityPtrsGetLevel()` | Rogue 側で wave 単位に決める。 |
| Friendship / nature / shiny | `CreateFacilityMon` 内で固定処理 | だいたい流用可能。Frustration / Return swap も同居。 |

### Battle Tent / Battle Factory との違い

- **Battle Factory**: `gFacilityTrainerMons[]` を player にも貸し出す (rental)。`CreateFacilityMon(..., FLAG_FRONTIER_MON_FACTORY, ...)` の flag で Frustration / Return swap が変わる。Rogue が rental を含むなら学ぶべきパターン。
- **Battle Tent (Slateport / Verdanturf / Fallarbor)**: 短期版 Frontier。`gSlateportBattleTentMons[]` などの table を使う。
- **Battle Tower**: より単純な multi-fight。e-reader trainer や record-mixed trainer は `TRAINER_EREADER` / `TRAINER_RECORD_MIXING_FRIEND` のような特殊 ID を `opponentA` に置き、`FillTrainerParty` の if 分岐で別パスへ ([battle_frontier.c:218-249](../../src/battle_frontier.c#L218-L249))。

### Trainer ID 範囲設計

Frontier は `TRAINER_BATTLE_PARAM.opponentA` の **値域** で経路を切り分けている:

| Range | Source |
|---|---|
| `< FRONTIER_TRAINERS_COUNT` (300) | `gFacilityTrainers[]` の通常 frontier trainer |
| `TRAINER_EREADER` | e-reader trainer |
| `TRAINER_FRONTIER_BRAIN` | Frontier brain |
| `< TRAINER_RECORD_MIXING_APPRENTICE` | Record mixed friend |
| それ以上 | Apprentice |

Rogue 用に独自 ID 帯 (例 `ROGUE_TRAINER_BASE = 0x4000`) を `opponentA` に置く設計はこの分岐パターンと相性がよい。ただし `opponentA` は u16 なので、`gTrainers[][]` の TRAINERS_COUNT (855) と衝突しないなら自由に使える。

## `gIsDebugBattle` Hook ― そのまま流用できる入口

[include/data.h:253-269](../../include/data.h#L253-L269):

```c
static inline const struct Trainer *GetTrainerStructFromId(u16 trainerId)
{
    u32 sanitizedTrainerId = 0;
    if (gIsDebugBattle) return GetDebugAiTrainer();
    sanitizedTrainerId = SanitizeTrainerId(trainerId);

    if (IsPartnerTrainerId(trainerId))
    {
        enum DifficultyLevel difficulty = GetBattlePartnerDifficultyLevel(sanitizedTrainerId);
        return &gBattlePartners[difficulty][sanitizedTrainerId - TRAINER_PARTNER(PARTNER_NONE)];
    }
    else
    {
        enum DifficultyLevel difficulty = GetTrainerDifficultyLevel(sanitizedTrainerId);
        return &gTrainers[difficulty][sanitizedTrainerId];
    }
}
```

ここに 1 行 hook を足すだけで、Rogue でも同じように runtime trainer を返せる:

```c
// 仮設計
if (gRogueBattle.active) return RogueBuildTrainerStruct();
```

`RogueBuildTrainerStruct()` は EWRAM 上に `static struct Trainer rogueRuntimeTrainer;` を 1 個確保して、そこを毎回上書き返すだけでよい。Battle 中ずっと同じポインタが返るので、AI / battle UI / `CreateNPCTrainerPartyFromTrainer` が一貫した struct を見られる。

debug 経路は実はもう 1 段の最適化を入れている。`battle_main.c:596` の:

```c
if (!DEBUG_OVERWORLD_MENU || (DEBUG_OVERWORLD_MENU && !gIsDebugBattle))
{
    if (!(gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED)))
    {
        CreateNPCTrainerParty(&gEnemyParty[0], TRAINER_BATTLE_PARAM.opponentA, TRUE);
        ...
    }
}
```

`gIsDebugBattle` が TRUE なら `CreateNPCTrainerParty` は **呼ばれない**。party は debug 側が事前に `CreateNPCTrainerPartyFromTrainer(gEnemyParty, GetDebugAiTrainer(), ...)` で組んだ結果がそのまま使われる。Rogue でも同じ手順を踏めば、(1) Rogue が `gEnemyParty[]` を組む、(2) `gIsRogueBattle = TRUE` を立てる、(3) `BattleSetup_StartTrainerBattle_Debug` 系の経路を呼ぶ、で足りる。

## Design Options for Rogue

### Option 1: GetTrainerStructFromId hook + 1 virtual slot

仕組み:

1. `gTrainers[DIFFICULTY_NORMAL][TRAINER_ROGUE_VIRTUAL]` を sentinel として 1 個確保。`partySize = 0`, `party = NULL` で OK (実体は使わない)。
2. Rogue 用に `static struct Trainer sRogueRuntimeTrainer;` を EWRAM に置く。
3. `GetTrainerStructFromId` に `if (gRogueBattle.active) return &sRogueRuntimeTrainer;` を 1 行追加。
4. battle 開始前に Rogue の roster table から `sRogueRuntimeTrainer` を埋める (name, class, pic, party pool, partySize, aiFlags, poolRuleIndex …)。
5. `TRAINER_BATTLE_PARAM.opponentA = TRAINER_ROGUE_VIRTUAL` を立てて、通常の `BattleSetup_StartTrainerBattle` を呼ぶ。
6. battle_main.c の `CreateNPCTrainerParty` が `GetTrainerStructFromId` 経由で sRogueRuntimeTrainer を取得し、`CreateNPCTrainerPartyFromTrainer` で `gEnemyParty[]` を組む。

メリット:

- battle 開始経路 (transition、music、intro text、AI、battle UI、勝敗 flag) を一切再発明しない。
- `struct Trainer.poolSize / poolRuleIndex / poolPruneIndex` がそのまま機能するので、Rogue 用 prune 関数を 1 個追加するだけで「per-wave / per-difficulty で違う pool 抽選」を表現できる。
- TRAINER flag は `TRAINER_FLAGS_START + TRAINER_ROGUE_VIRTUAL` の 1 個しか使わない。これは battle が走るたび `ClearTrainerFlag(TRAINER_ROGUE_VIRTUAL)` でリセットでき、勝敗の永続記録は Rogue SaveBlock 側 (capacity doc 参照) に書く。

デメリット:

- `gTrainers[][]` に 1 個 sentinel を入れる必要がある (ROM コスト数十 byte)。
- `GetTrainerStructFromId` は inline で多数の場所から呼ばれている。hook の cost は実質 0 だが、`gRogueBattle.active` は global EWRAM になる。
- 同時 Rogue battle が 1 つしか走らない前提。multi battle で 2 人同時に Rogue 化するなら sRogueRuntimeTrainer を 2 本 (A/B) 持つ必要がある。

### Option 2: Frontier 経路への相乗り

仕組み:

1. `TRAINER_BATTLE_PARAM.opponentA` に Rogue 用の ID 帯 (例: `ROGUE_TRAINER_BASE = 0x4000`) を入れる。
2. `FillTrainerParty` ([battle_frontier.c:202](../../src/battle_frontier.c#L202)) の if 分岐に Rogue 範囲を 1 つ追加し、Rogue 専用の `gRogueFacilityTrainers[]` + `gRogueFacilityMons[]` から party を組む。
3. Battle Pyramid / Trainer Hill のように `BATTLE_TYPE_FRONTIER` か独自 flag を立てて battle 経路を Frontier 系に流す。

メリット:

- `struct BattleFrontierTrainer` の軽量さが活きる (300 trainer で約 11 KB)。
- Battle Frontier の roster table と一緒の book-keeping。
- e-reader / record-mix のような特殊 ID 経路を踏襲しやすい。

デメリット:

- Frontier の **species/held item duplicate ban**、**fixed IV**、**3-mon party**、**FRONTIER_MAX_LEVEL_50 filter** といった rule が暗黙に効く。Rogue 用に逃すなら、Frontier コードに Rogue 専用 if 分岐を増やすことになる。
- AI flag セットや mugshot のような Trainer struct 由来の演出が `gFacilityTrainers` には入らない。Rogue で「ボス mugshot」「特殊 AI」をやりたいなら別途差し戻し処理が要る。
- `BATTLE_TYPE_FRONTIER` を立てると battle 中の挙動が変わる場所が多い (`SetWildMonHeldItem` を呼ばない、record battle disable、experience を貰わない、capture 不可、…)。Rogue が「捕獲アリ / 経験値アリ」を選べるなら相性が悪い。

### Option 3: 完全独自経路 (debug 系列を流用)

仕組み:

1. Rogue から `CreateNPCTrainerPartyFromTrainer(gEnemyParty, &sRogueRuntimeTrainer, FALSE, BATTLE_TYPE_TRAINER)` を直接呼ぶ。
2. `gIsRogueBattle = TRUE` を立てる (もしくは `gIsDebugBattle` をそのまま流用)。
3. `BattleSetup_StartTrainerBattle_Debug` と同じパターンで battle に入る。

メリット:

- battle 開始経路を Rogue 完全制御。
- gTrainers[][] に sentinel 1 個すら入れない構成も可能。

デメリット:

- 普通の trainer battle が呼ばれる場所 (rematch、TRAINER_BATTLE_REMATCH、TRAINER_BATTLE_EARLY_RIVAL、TRAINER_BATTLE_TWO_TRAINERS …) と合流できない。Rogue 用に "battle 開始" のラッパを再発明することになる。
- debug 経路は `CB2_EndDebugBattle` → `CB2_EndTrainerBattle` という戻り路を持っているが、Rogue はその後の wave 進行 / reward UI を置く場所が必要。debug を直接流用するとこれが debug 用 callback と混線する。

### 推奨: Option 1 をベースに、必要に応じて Option 3 でラッピング

- **Trainer struct と party generator は Option 1**。`struct Trainer.poolSize` 機能を活用し、Rogue 用 prune / pick 関数を `trainer_pools.c` に 1〜2 個追加する。
- **Battle 開始経路は通常の `BattleSetup_StartTrainerBattle`**。Rogue ロビーから対戦相手 NPC に話しかける流れがそのまま使える。
- **戻り処理だけ Rogue 用 callback にすり替え** (Option 3 の battle end の差し替え)。wave 進行 / reward 表示はここで処理する。

これにより、AI / mugshot / animation / intro text などの既存資産は壊さず、Rogue のロジック (roster / wave / reward) は新規ファイルに完結させられる。

## Pool Rule / Pruning Hook の活用

[src/trainer_pools.c:311-368](../../src/trainer_pools.c#L311-L368) の `GetPickFunctions` と `PrunePool` は既に拡張ポイントになっている:

```c
switch (trainer->poolPickIndex)
{
    case POOL_PICK_DEFAULT: ...
    case POOL_PICK_LOWEST: ...
    default: ...
}

switch (trainer->poolPruneIndex)
{
    case POOL_PRUNE_NONE: break;
    case POOL_PRUNE_TEST: TestPrune(trainer, poolIndexArray, rules); break;
    case POOL_PRUNE_RANDOM_TAG: RandomTagPrune(trainer, poolIndexArray, rules); break;
    default: break;
}
```

Rogue が追加できる枠例:

| 名前 (案) | 機能 |
|---|---|
| `POOL_PRUNE_ROGUE_LEVEL_BAND` | 現在 wave に対して species_info の base stat / power level が外れているものを除外 |
| `POOL_PRUNE_ROGUE_TYPE_THEME` | run seed で決まる theme type を持つ mon だけ残す |
| `POOL_PRUNE_ROGUE_DIFFICULTY` | DIFFICULTY_HARD で hard-only タグを残す |
| `POOL_PICK_ROGUE_BOSS_LAST` | ace 枠を必ず "BOSS" タグの mon にする |
| `POOL_PICK_ROGUE_RUN_SEED` | Random32() ではなく `gRogueSave.runSeed` を使って deterministic に抽選 |

`struct TrainerMon.tags` ([include/data.h:80](../../include/data.h#L80)) は **u32 の bitfield** で、既に `RandomTagPrune` が monoteam 化に使っている。Rogue 用 tag (BOSS / GIMMICK / WAVE_EARLY / WAVE_LATE …) を 5〜10 個切れば、Rogue 専用配列を別途用意せずに既存 `gFacilityTrainerMons[]` や Rogue 用 mon table と共有できる。

## Difficulty Layer との関係

[src/difficulty.c](../../src/difficulty.c) の `GetCurrentDifficultyLevel()` は global state を返す。`GetTrainerDifficultyLevel(trainerId)` がそれをそのまま使うので、`gTrainers[difficulty][trainerId]` のスロット選択は difficulty 切替に反応する。

Rogue 観点では:

- run 開始時に difficulty を fix する設計なら、`GetCurrentDifficultyLevel()` を Rogue 側で上書きする必要は無い (run 用 difficulty を `SetCurrentDifficultyLevel` で設定する)。
- run 中だけ別 difficulty に振りたいなら、Rogue battle entry の前後で difficulty を save/restore するパターンが安全。`gIsRogueBattle` 中だけ runtime trainer を返す Option 1 設計と相性がよい。
- HARD で gimmick AI flag を増やす、EASY で pool size を絞る、のような分岐は `RogueBuildTrainerStruct()` 内で完結させる。

## Risks and Open Questions

| Risk | Why | Mitigation |
|---|---|---|
| `GetTrainerStructFromId` hook が IsPartnerTrainerId と衝突 | partner battle で `gBattlePartners[][]` を見る分岐が手前にあるため、hook 位置を間違えると partner battle が壊れる。 | `if (gRogueBattle.active && !IsPartnerTrainerId(trainerId)) return ...` の順で書く。 |
| `gIsDebugBattle` を Rogue が流用 | debug menu と Rogue が同 flag を共有すると、debug battle 中に Rogue side effect が発火しうる。 | 必ず別 flag (`gIsRogueBattle`) を切る。debug の hook も残すこと。 |
| `gEnemyParty[]` の事前構築 vs 自動構築 | Option 1 では battle_main.c が自動で組む。Option 3 では事前に組む。両者を混在させると 2 重生成。 | Rogue は Option 1 だけに絞る。Option 3 は将来「reroll battle」のような特殊路でだけ使う。 |
| `BATTLE_TYPE_FRONTIER` の副作用 | Frontier flag が立つと experience / capture / record battle が変わる。Rogue がこれらを許す方針なら相性が悪い。 | Option 2 を採用しない。Option 1 で `BATTLE_TYPE_TRAINER` だけにする。 |
| AI flag を runtime で書き換え | `aiFlags` は battle 開始時に AI 側へコピーされる箇所が複数。中盤で書き換えても効かない可能性。 | Rogue は battle 開始前に確定させ、battle 中は変更しない。dynamic AI が要るなら `AI_FLAG_DYNAMIC_FUNC` + `setdynamicaifunc` の経路を別途調査する (battle_ai_decision_flow_v15.md と要連動)。 |
| `overrideTrainer` との相互作用 | `CreateNPCTrainerParty` ([src/battle_main.c:2069](../../src/battle_main.c#L2069)) は `overrideTrainer != 0` の trainer を memcpy で再構築する。Rogue runtime trainer に `overrideTrainer` を立てるとここでさらに 1 hop する。 | `sRogueRuntimeTrainer.overrideTrainer = 0` 固定で運用する。 |
| `partySize == 0` を AI が想定しない可能性 | trainer pool 機能が `partySize == 0 && AI_FLAG_RANDOMIZE_PARTY_INDICES` の特殊ケースを持つ ([trainer_pools.c:375](../../src/trainer_pools.c#L375))。Rogue が pool only で `partySize` を後付けすると初回は 0 で踏みうる。 | Rogue runtime trainer は `partySize` を **必ず** 1〜6 で初期化する。 |

### Open Questions

- Rogue battle は `BATTLE_TYPE_TRAINER` 単独でよいか、それとも独自 flag (`BATTLE_TYPE_ROGUE`?) を 1 個追加するか。後者は battle 中の experience / catch / restore policy を 1 か所で切り替えられて便利だが、`gBattleTypeFlags` の bit 予算に影響する。
- 持ち物 / 性格 / 技構成を **run seed から決定論的に生成** する場合、`Random()` を使う `FillTrainerParty` パスは決定論性を破る。Rogue は `Random32_Seeded(seed)` のような専用 RNG を新設する必要がある (see [Roguelike Party Policy Impact v15](roguelike_party_policy_impact_v15.md))。
- Rogue trainer の **mugshot / pic** を runtime に切り替える場合、`gTrainerSprites[]` / `gTrainerBacksprites[]` のロード経路が compile-time index 前提のはず。switch ROM size と起動時 decompress を要再調査。
- Battle Tower の `CreateBattleTowerMon_HandleLevel` で record-mix mon を扱う処理 ([src/battle_frontier.c:239](../../src/battle_frontier.c#L239)) は、player の record mix 友達のパーティを `gSaveBlock2Ptr->frontier.towerRecords[]` から復元する。Rogue が「他プレイヤーと共有された roster」を将来扱うなら、この経路と save migration 連携を検討する。
- `gPartnerTrainerId` (battle_setup.h:49) と Rogue の partner battle 設計の関係 — Rogue でフォロワー partner と一緒に戦うなら、partner 側も `IsPartnerTrainerId` / `gBattlePartners[][]` 経路に乗せるか、Rogue 独自経路で組むかの判断が必要。
