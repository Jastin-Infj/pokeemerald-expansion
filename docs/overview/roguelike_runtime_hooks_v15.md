# Roguelike Runtime Hooks Investigation v15

調査日: 2026-05-04。実装は未着手。`docs/` への記録のみ。

## Purpose

[Roguelike Runtime Party Generation v15](roguelike_runtime_party_v15.md) の続編。Rogue の "wave 進行 / 報酬 / 決定論的 RNG / 永続セーブ" を組むときに、現状コードに「乗れる hook が既にあるか」「無いか」を 1 件ずつ確認する。各章の冒頭に **状態欄** を置き、現時点で未実装のものは `Status: 現時点未実装` と明記する。

対象トピック:

1. Battle 終了側の callback ([CB2_EndTrainerBattle](#1-battle-end-callback-cb2_endtrainerbattle))
2. AI flag を wave 帯ごとに切り替える設計 ([Trainer.aiFlags + setdynamicaifunc](#2-ai-flag-per-wave))
3. `Random32` を Rogue seed に置換する範囲 ([SeedRng / RNG_* tag system](#3-seedable-rng-and-rng-tags))
4. Rogue 専用 SaveBlock の構造案 ([SaveBlock1/2/3 layout と migration](#4-rogue-saveblock-anchor-candidate))

関連 doc:

- [Roguelike NPC Capacity Impact v15](roguelike_npc_capacity_v15.md)
- [Roguelike Party Policy Impact v15](roguelike_party_policy_impact_v15.md)
- [Roguelike Runtime Party Generation v15](roguelike_runtime_party_v15.md)
- [Trainer Battle Flow v15](../flows/trainer_battle_flow_v15.md)
- [Battle AI Decision Flow v15](../flows/battle_ai_decision_flow_v15.md)
- [Save Data Flow v15](../flows/save_data_flow_v15.md)

## 1. Battle End Callback (CB2_EndTrainerBattle)

**Status**: 既存の hook で十分対応可能。Rogue 用 callback を 1 個増やすだけで wave 進行 / 報酬 UI を差し込める。

### 既存の流れ

[src/battle_setup.c:1300](../../src/battle_setup.c#L1300) `BattleSetup_StartTrainerBattle()` の中で:

```c
gMain.savedCallback = CB2_EndTrainerBattle;
```

を立て、battle が終わると battle_main 側 (`HandleEndTurn_FinishBattle`、`BattleMainCB2` など) が `SetMainCallback2(gMain.savedCallback)` で `CB2_EndTrainerBattle` に戻ってくる。

[src/battle_setup.c:1428-1494](../../src/battle_setup.c#L1428-L1494) の `CB2_EndTrainerBattle()` は次の判定をする:

| Branch | 入る条件 | 行き先 |
|---|---|---|
| Follower NPC partner restore | `FollowerNPCIsBattlePartner()` | `RestorePartyAfterFollowerNPCBattle()` + 必要なら heal、その後通常 branch へ |
| Early Rival 敗北 | `TRAINER_BATTLE_EARLY_RIVAL` && `IsPlayerDefeated` | `RIVAL_BATTLE_HEAL_AFTER` で heal or `CB2_WhiteOut` |
| Secret Base | `opponentA == TRAINER_SECRET_BASE` | `CB2_ReturnToFieldContinueScriptPlayMapMusic` |
| Forfeit | `DidPlayerForfeitNormalTrainerBattle()` | `B_FLAG_NO_WHITEOUT` / Pyramid / Hill 中なら field 復帰、それ以外は `CB2_WhiteOut` |
| 敗北 | `IsPlayerDefeated(gBattleOutcome)` | 同上 (`B_FLAG_NO_WHITEOUT` で white-out 抑制可能) |
| 勝利 | else | field 復帰 + `RegisterTrainerInMatchCall()` + `SetBattledTrainersFlags()` |

そして post-battle に走らせたい event script は `sTrainerBattleEndScript` ([battle_setup.c:94](../../src/battle_setup.c#L94)) に格納されていて、[src/battle_setup.c:1551-1557](../../src/battle_setup.c#L1551-L1557) の `BattleSetup_GetScriptAddrAfterBattle()` がそれを返す。

```c
const u8 *BattleSetup_GetScriptAddrAfterBattle(void)
{
    if (sTrainerBattleEndScript != NULL)
        return sTrainerBattleEndScript;
    else
        return EventScript_TestSignpostMsg;
}
```

つまり通常 trainer battle は、勝敗判定後に `CB2_ReturnToFieldContinueScriptPlayMapMusic` でフィールドへ戻り、`gPostMenuFieldCallback` 系のフィールド復帰経路で `BattleSetup_GetScriptAddrAfterBattle()` が指すスクリプトを実行する設計。

### Debug battle が既にやっている差し替え

[src/battle_setup.c:1377-1390](../../src/battle_setup.c#L1377-L1390) `CB2_EndDebugBattle`:

```c
static void CB2_EndDebugBattle(void)
{
    if (gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER)
    {
        for (u32 i = 0; i < 3; i++)
        {
            u16 monId = gSaveBlock2Ptr->frontier.selectedPartyMons[i] - 1;
            if (monId < PARTY_SIZE)
                SavePlayerPartyMon(...);
        }
        LoadPlayerParty();
    }
    SetMainCallback2(CB2_EndTrainerBattle);
}
```

そして `BattleSetup_StartTrainerBattle_Debug()` は `gMain.savedCallback = CB2_EndDebugBattle;` を立てる ([battle_setup.c:1398](../../src/battle_setup.c#L1398))。**Rogue も同じパターン**で `CB2_EndRogueBattle` を 1 個用意し、そこで wave 結果の処理 → 通常 `CB2_EndTrainerBattle` に委譲、もしくは Rogue 専用 reward UI へ分岐できる。

### Rogue 用に推奨する差し込み点

| 局面 | 既存 hook | 何を入れる |
|---|---|---|
| Battle 開始直前 | `BattleSetup_StartTrainerBattle()` の savedCallback 代入 | `gMain.savedCallback = CB2_EndRogueBattle;` (通常 `CB2_EndTrainerBattle` の代わり) |
| Battle 終了直後 | `CB2_EndRogueBattle` 新設 | 勝利→wave++ / 敗北→run abort 判定。`gIsRogueBattle = FALSE` を立て、Rogue trainer struct を解除。 |
| 勝敗判定 | `IsPlayerDefeated(gBattleOutcome)` | 既存 API。Rogue では white-out させたくない場合があるので、`B_FLAG_NO_WHITEOUT` を Rogue 進入時に set / 退出時に clear する単純運用も可。 |
| 報酬 UI | Rogue 用 callback | `SetMainCallback2(CB2_RogueRewardUI)` → 完了で `SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic)` |
| Post-battle script | `sTrainerBattleEndScript` | Rogue 進入時に `BattleSetup_ConfigureTrainerBattle` を経由しないなら、`sTrainerBattleEndScript = RogueScript_AfterBattle;` を直接代入することで OK。Rogue が独自 script を走らせるルートに乗せる。 |

### Risks

- `CB2_ReturnToFieldContinueScriptPlayMapMusic` は overworld task / map music を再開する。Rogue 報酬 UI を battle 直後に出すなら、戻り処理を BG load → window allocate → fade-in の順で組まないと、field 復帰中の sprite double-free や music 二重再生を踏みやすい。debug 経路はこれを通常 `CB2_EndTrainerBattle` に委譲することで回避している。
- `RegisterTrainerInMatchCall()` ([battle_setup.c:1490](../../src/battle_setup.c#L1490)) と `SetBattledTrainersFlags()` ([battle_setup.c:1491](../../src/battle_setup.c#L1491)) は Rogue では呼びたくない (TRAINER_FLAG を消費する、match call DB を汚す)。Rogue 用 callback ではここを通らない経路にする。
- `HandleBattleVariantEndParty()` ([battle_setup.c:1430](../../src/battle_setup.c#L1430)) は Battle Frontier / Tent の party 復元処理。Rogue が独自 party 持ち込みでなければ呼ばなくてよいが、follower partner と組み合わせる場合は再確認する。

## 2. AI Flag Per Wave

**Status**: `Trainer.aiFlags` 経路は既存。`setdynamicaifunc` script command + `sDynamicAiFunc` EWRAM hook も既存。**Rogue は新規 dynamic AI function を 1 個書いて wave 帯ごとの prefer / avoid を表現できる**。

### aiFlags の読み出し経路

[src/battle_ai_main.c:200-252](../../src/battle_ai_main.c#L200-L252) `GetAiFlags(u16 trainerId, ...)`:

```c
else if (gBattleTypeFlags & BATTLE_TYPE_FACTORY)
    flags = GetAiScriptsInBattleFactory();
else if (gBattleTypeFlags & (BATTLE_TYPE_FRONTIER | BATTLE_TYPE_EREADER_TRAINER | BATTLE_TYPE_TRAINER_HILL | BATTLE_TYPE_SECRET_BASE))
    flags = AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT;
else
    flags = GetTrainerAIFlagsFromId(trainerId);
```

ここの最後の else が **Rogue が乗りたい経路**。`GetTrainerAIFlagsFromId(trainerId)` は `GetTrainerStructFromId(trainerId)->aiFlags` を返すので、Runtime Party doc の Option 1 hook (`gIsRogueBattle` 時に `&sRogueRuntimeTrainer` を返す) を使えば **Rogue trainer の aiFlags が AI 初期化に流れる**。

[src/battle_ai_main.c:254-309](../../src/battle_ai_main.c#L254-L309) `BattleAI_SetupFlags()` が battler ごとに `GetAiFlags` を呼んで `gAiThinkingStruct->aiFlags[]` に格納する。**battle 中に flags を書き換えても効かない** — battle 開始時に固定される。Rogue は wave entry の手前で `sRogueRuntimeTrainer.aiFlags` を確定させる必要がある。

### 自動付与される flag

`GetAiFlags` は最後に次を自動 OR する:

| 自動付与 | トリガ | 出典 |
|---|---|---|
| `AI_FLAG_DOUBLE_BATTLE` | `IsDoubleBattle() && flags != 0` | [battle_ai_main.c:235-238](../../src/battle_ai_main.c#L235-L238) |
| `AI_FLAG_SMART_MON_CHOICES` | `flags & AI_FLAG_SMART_SWITCHING` | [battle_ai_main.c:241-242](../../src/battle_ai_main.c#L241-L242) |
| `AI_FLAG_PREDICT_SWITCH` | `flags & AI_FLAG_PREDICT_INCOMING_MON` | [battle_ai_main.c:245-246](../../src/battle_ai_main.c#L245-L246) |
| `AI_FLAG_DYNAMIC_FUNC` | `sDynamicAiFunc != NULL` | [battle_ai_main.c:248-249](../../src/battle_ai_main.c#L248-L249) |

最後の項目が Rogue 観点では重要。**`sDynamicAiFunc` は EWRAM の関数ポインタ**で、これが non-NULL なら自動的に `AI_FLAG_DYNAMIC_FUNC` が立ち、battle 中の `AI_DynamicFunc` ([battle_ai_main.c:7115-7120](../../src/battle_ai_main.c#L7115-L7120)) が AI スコア計算の最後に呼ばれる:

```c
static s32 AI_DynamicFunc(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, s32 score)
{
    if (sDynamicAiFunc != NULL)
        score = sDynamicAiFunc(battlerAtk, battlerDef, move, score);
    return score;
}
```

つまり「pool 抽選した move の score を Rogue 側で +/- する」hook がそのまま使える。

### Setdynamicaifunc script command

[asm/macros/event.inc:2148-2149](../../asm/macros/event.inc#L2148-L2149):

```text
.macro setdynamicaifunc func:req
    callnative ScriptSetDynamicAiFunc, requests_effects=1
```

[src/battle_ai_main.c:7122-7142](../../src/battle_ai_main.c#L7122-L7142) `ScriptSetDynamicAiFunc` が EWRAM の `sDynamicAiFunc` を更新する。`ResetDynamicAiFunctions()` で NULL に戻す。

Rogue ロビーから battle に入る script は、wave 帯に応じて:

```text
setdynamicaifunc Rogue_AiScore_Wave1to3
trainerbattle ... TRAINER_ROGUE_VIRTUAL ...
```

のように切り替えるだけで、AI の挙動を wave 単位で変えられる。

### 設計案

| Rogue runtime state | aiFlags / dynamic func の決め方 |
|---|---|
| Wave 1〜3 (序盤) | `AI_FLAG_CHECK_BAD_MOVE` のみ。dynamic func で setup move を抑制。 |
| Wave 4〜7 | `AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_VIABILITY`。dynamic func は無し。 |
| Wave 8+ ボス | フル AI (`AI_FLAG_SMART_SWITCHING | AI_FLAG_OMNISCIENT | AI_FLAG_ACE_POKEMON | AI_FLAG_DYNAMIC_FUNC` 含む)。 |

`sRogueRuntimeTrainer.aiFlags` を Rogue 側 table から wave 番号で lookup して確定させる。`setdynamicaifunc` は battle entry script で必要なときだけ呼ぶ。

### Risks

- **battle 中 dynamic AI 切り替えは不可**。flag 配列は battle 開始時に固定。wave 内で AI を変えたい場合は battle 終了 → setdynamicaifunc → 次 wave entry の順しかない。
- **AI_FLAG_DYNAMIC_FUNC は score 加減算のみ**。switch / target 選択を変えるには `gDynamicAiSwitchFunc` (battle_ai_main.c:7130-7136) を別途 set する。今回 Rogue が switch logic を弄りたいかどうかは未決。
- **Frontier / Trainer Hill 系の固定 flag** は `BATTLE_TYPE_FRONTIER` で上書きされる ([battle_ai_main.c:229-230](../../src/battle_ai_main.c#L229-L230))。Rogue が Frontier flag を立てた途端 `aiFlags = AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT` で固定される。Runtime Party doc の Option 1 推奨どおり `BATTLE_TYPE_TRAINER` 単独に留めること。
- **`gDebugAIFlags` の存在**。debug menu 経路は `BattleAI_SetupFlags` 内で `gDebugAIFlags` を直接代入する分岐がある ([battle_ai_main.c:261-266](../../src/battle_ai_main.c#L261-L266))。Rogue が debug path を流用する場合、`gDebugAIFlags` を毎回上書きする必要がある。逆に通常路 (Option 1 推奨) を通せばこの分岐は触らない。

## 3. Seedable RNG and RNG Tags

**Status**: SFC32 ベースの `gRngValue` / `gRng2Value` の 2 stream 構成は既存。**`SeedRng()` / `SeedRng2()` でシード可能**で、battle 用 RNG を切り離す経路 (`Random2_32`) も用意されている。Rogue 用 deterministic RNG は **`LocalRandomSeed()` + `LocalRandom32()` を使えば既存 global を一切汚さずに済む**。RNG tag (`enum RandomTag`) も既存だが、tag 別 stream の自動切り替えは未実装。

### 既存 RNG 構造

[src/random.c](../../src/random.c) と [include/random.h](../../include/random.h):

```c
COMMON_DATA rng_value_t gRngValue = {0};   // メイン (SFC32)
COMMON_DATA rng_value_t gRng2Value = {0};  // 別 stream

u32 NAKED Random32(void);          // メイン stream を消費 (ASM 実装)
u32 Random2_32(void);              // gRng2Value を消費

void SeedRng(u32 seed);            // gRngValue 再シード
void SeedRng2(u32 seed);           // gRng2Value 再シード

rng_value_t LocalRandomSeed(u32 seed);  // 独立 state を返す
static inline u32 LocalRandom32(rng_value_t *val);   // 独立 state を進める
```

`SeedRng` 呼び出し元:

| 場所 | タイミング |
|---|---|
| [src/main.c:224](../../src/main.c#L224) `SeedRngAndSetTrainerId` | 起動時。タイマレジスタを seed に。 |
| [src/main.c:252](../../src/main.c#L252) `SeedRngWithRtc` (`#ifdef BUGFIX`) | RTC seed (Emerald で意図的に無効化されている) |
| [src/link.c:290](../../src/link.c#L290) | リンク時の同期 seed |
| [src/link_rfu_2.c:2583](../../src/link_rfu_2.c#L2583) | リンク RFU 同期 |

つまり **通常 game 中は `gRngValue` を Rogue が直接書き換えても安全だが、リンク battle 中は介入禁止**。Rogue は単独プレイ前提なので問題ない。

### Battle が消費する RNG

[include/random.h:125-262](../../include/random.h#L125-L262) の `enum RandomTag` には battle 内で使われる種別が大量にある:

| 主要な tag | 用途 |
|---|---|
| `RNG_ACCURACY` | 命中判定 |
| `RNG_CRITICAL_HIT` | 急所判定 |
| `RNG_DAMAGE_MODIFIER` | ダメージ振れ幅 |
| `RNG_HITS` | 連続技回数 |
| `RNG_SECONDARY_EFFECT` / `_2` / `_3` | 追加効果 |
| `RNG_PARALYSIS` / `RNG_FROZEN` / `RNG_INFATUATION` 等 | 状態異常解除判定 |
| `RNG_QUICK_DRAW` / `RNG_QUICK_CLAW` | 特性 / 道具 |
| `RNG_AI_SCORE_TIE_*` / `RNG_AI_SWITCH_*` | AI 判断 |

これらの tag 関数 (`RandomUniform`, `RandomWeightedArray`, `RandomElementArray`) は `__attribute__((weak, alias("...Default")))` で実装されており ([random.c:162-172](../../src/random.c#L162-L172))、test runner が tag 別に上書きできる仕組みになっている (テスト DSL で「クリティカル必中」を作るなど)。

ただし **runtime (test runner 以外) で tag 別の seed を指定する仕組みは現時点未実装**。`RandomUniformDefault` は内部で `Random()` を呼ぶだけで、tag は引数として渡されてくるが分岐に使われていない:

```c
u32 RandomUniformDefault(enum RandomTag tag, u32 lo, u32 hi)
{
    assertf(lo <= hi);
    return lo + (((hi - lo + 1) * Random()) >> 16);
}
```

### Rogue が取れる選択肢

| 選択肢 | 何をするか | 既存に乗るか |
|---|---|---|
| A. battle 開始前に `SeedRng(rogueRunSeed ^ waveIndex)` | wave 単位で全 RNG を deterministic にする | ◯ 既存 API のみ。ただし battle UI / 自然遷移用の `AdvanceRandom()` も影響を受ける。 |
| B. party 生成だけ `LocalRandomSeed` + `LocalRandom32` | Rogue ロジックが local state を持ち、`gRngValue` を一切触らない | ◎ 一番安全。party generator / pool 抽選 / loot table すべて local state で済む。 |
| C. battle 中は `gRng2Value` (`SeedRng2`) を Rogue が制御 | battle が `Random2_32` を使うように改修 + Rogue が seed | ✗ battle 側のすべての `Random()` 呼び出しを `Random2()` に書き換える大改造。**現時点未実装**。 |
| D. `enum RandomTag` 別に異なる stream を作る | 例: `RNG_AI_SCORE_TIE_*` だけ別 seed | ✗ tag 別 dispatch は実装されていない。**現時点未実装**。tag は test runner override 用に予約されている。 |

**推奨は B**。Rogue runtime は `gRogueRun.partySeed` / `gRogueRun.lootSeed` のような複数 seed を持ち、それぞれを `LocalRandomSeed` で初期化、Rogue ロジックは `LocalRandom32(&gRogueRun.partyState)` で進める。これなら通常 battle の crit / accuracy などには一切干渉しない。

### 派生注意点

- **`Random()` (16-bit) と `Random32()` (32-bit) は同じ stream**。`Random()` は `Random32() >> 16` ([random.h:60-62](../../include/random.h#L60-L62))。Rogue が `gRngValue` を再シードするなら両者が影響を受ける。
- **`AdvanceRandom()` が VBlank で呼ばれている** ([random.c:100-104](../../src/random.c#L100-L104), [src/battle_main.c:2102](../../src/battle_main.c#L2102))。link / frontier / recorded battle 中はスキップされるが、通常 battle 中は VBlank ごとに 1 つ消費される。**Rogue が seed を battle 開始時に固定しても、battle 中の VBlank 消費数によって毎回違う乱数が出る**。これは選択肢 A の致命的な欠点で、deterministic battle を作るには battle_main.c の `AdvanceRandom` 呼び出しを Rogue 中だけ抑制する別 hook が必要。
- **`B_VAR_RANDOMIZE_DAMAGE` 等の config flag** は damage 振れ幅を完全に固定する config。これらの既存 config を流用するだけで「Rogue では全 damage 固定」が手に入る場合がある。詳細は `include/config/battle.h` を別途 audit する。

### Conclusion

- **party 生成 / 抽選 / loot は LocalRandom 系で完全 deterministic**にできる (現状 API のみで実現可能)。
- **battle 中の crit / accuracy / 追加効果まで deterministic にする**のは現状 API では不可能。`AdvanceRandom` の VBlank 消費が deterministic 性を破る。やるなら `gIsRogueBattle` フラグ追加 + `AdvanceRandom` 抑制 + `SeedRng` を battle 開始時 fixed seed で呼ぶ、の **3 点改修が必要 (現時点未実装)**。
- 当面は「party / encounter / loot は固定 seed」「battle 中は normal RNG」のハイブリッドで割り切るのが工数対効果的に最良。

## 4. Rogue SaveBlock Anchor Candidate

**Status**: `RogueSave` 用の SaveBlock は **現時点未実装**。既存 SaveBlock1/2/3 / PokemonStorage の容量と migration 政策は把握済み。Rogue 用構造体を **SaveBlock3 に config-gated で追加** が最小手で、`docs/upgrades/branching_upgrade_policy.md` の "save 影響変更は config-off default" 方針とも整合する。

### 既存 SaveBlock 全体像

[include/save.h](../../include/save.h) と [src/save.c:54-83](../../src/save.c#L54-L83):

| Block | Sectors | 容量上限 | 静的 assert |
|---|---|---|---|
| `struct SaveBlock2` | 1 (`SECTOR_ID_SAVEBLOCK2 = 0`) | `SECTOR_DATA_SIZE = 3968` byte | [save.c:81](../../src/save.c#L81) |
| `struct SaveBlock1` | 4 (`SAVEBLOCK1_START..END = 1..4`) | `4 × 3968 = 15872` byte | [save.c:82](../../src/save.c#L82) |
| `struct PokemonStorage` | 9 (5..13) | `9 × 3968 = 35712` byte | [save.c:83](../../src/save.c#L83) |
| `struct SaveBlock3` | 全 14 sectors の余白に分散 (`SAVE_BLOCK_3_CHUNK_SIZE = 116` byte / sector) | `14 × 116 = 1624` byte | [save.c:80](../../src/save.c#L80) |
| Footer | 全 sector 末尾 | `SECTOR_FOOTER_SIZE = 12` byte (id+checksum+signature+counter) | [save.h:9](../../include/save.h#L9) |

[src/save.c:80](../../src/save.c#L80) の comment:

> Each 4 KiB flash sector contains 3968 bytes of actual data followed by 116 bytes of SaveBlock3 and then 12 bytes of footer.

つまり **SaveBlock3 は 14 sectors の "切れ端" を集めたブロック** で、容量こそ 1624 byte と小さいが、新規 feature 用の追加領域として使うのが expansion の慣習になっている。

### SaveBlock3 の現在の中身

[include/global.h:253-271](../../include/global.h#L253-L271):

```c
struct SaveBlock3
{
#if OW_USE_FAKE_RTC
    struct SiiRtcInfo fakeRTC;
#endif
#if FNPC_ENABLE_NPC_FOLLOWERS
    struct NPCFollower NPCfollower;
#endif
#if OW_SHOW_ITEM_DESCRIPTIONS == OW_ITEM_DESCRIPTIONS_FIRST_TIME
    u8 itemFlags[ITEM_FLAGS_COUNT];
#endif
#if USE_DEXNAV_SEARCH_LEVELS == TRUE
    u8 dexNavSearchLevels[NUM_SPECIES];
#endif
    u8 dexNavChain;
#if APRICORN_TREE_COUNT > 0
    u8 apricornTrees[NUM_APRICORN_TREE_BYTES];
#endif
}; /* max size 1624 bytes */
```

特徴:

- **すべて config-gated**。各 feature の `#if` で field を増減できる。Rogue も `#if ROGUE_ENABLE` 形式で参加するのが自然。
- **DexNav の `dexNavSearchLevels[NUM_SPECIES]`** が既に大きめ (1 byte × NUM_SPECIES)。NUM_SPECIES が GEN_LATEST に応じて伸びるので、SaveBlock3 容量は将来も詰まる方向。Rogue が大きい構造体を入れると DexNav と衝突する。
- **`STATIC_ASSERT(sizeof(struct SaveBlock3) <= 1624)`** が compile time で守ってくれる。

### Rogue が必要そうな state (再掲、capacity doc から)

```c
struct RogueSave
{
    u8 npcDefeatedThisRun[(ROGUE_ROSTER_SIZE + 7) / 8];
    u8 npcDefeatedAllTime[(ROGUE_ROSTER_SIZE + 7) / 8];
    u8 npcMet[(ROGUE_ROSTER_SIZE + 7) / 8];
    u16 currentWave;
    u16 partySize;            // current run の party size
    u32 runSeed;
    u32 lootSeed;
    u8 difficulty;
    u8 ruleset;
    /* …rewards, currency, persistent unlocks… */
};
```

サイズ目安:

| Roster size | bitfield 3 本 | + 固定領域 (~16 byte) | 合計 |
|---|---|---|---|
| 100 | 39 byte | 55 byte |  |
| 300 | 114 byte | 130 byte |  |
| 1000 | 375 byte | 391 byte |  |

300 体で約 130 byte。**SaveBlock3 (1624 byte) には収まる**が、DexNav が NUM_SPECIES 分使うと残りが 1000 byte 前後になり得る。1000 体規模 (391 byte) でもまだ収まるが、「Rogue + DexNav + apricorn + follower NPC」を全部 ON にすると後で詰まる可能性がある。

### Anchor 候補比較

| 案 | 場所 | Pros | Cons |
|---|---|---|---|
| **A. SaveBlock3 に config-gated で追加** (推奨) | `struct SaveBlock3` 内 `#if ROGUE_ENABLE` | feature off で容量 0、既存 expansion パターンに乗る、migration 影響を限定できる。 | 1624 byte 全体が他 feature と争奪。1000 体規模は厳しめ。 |
| B. SaveBlock1 に追加 | `struct SaveBlock1` 内 | 容量に余裕 (15872 byte 中現状ほぼ詰まりだが、save migration 込みで再編成すれば確保可)。 | 既存 SaveBlock1 はストーリー進行 / dex / mail / RAM script など本編 state が密集していて、変更で全体配置が変わる。compatibility 影響大。 |
| C. SaveBlock2 に追加 | `struct SaveBlock2` 内 | playerName / pokedex / frontier 等の "メタデータ" 領域。Frontier は既に同居している。 | 上限 3968 byte の 1 sector 単独。Frontier `struct BattleFrontier frontier` で既にかなり埋まっている可能性。要再採寸。 |
| D. PokemonStorage の余白を借りる | `struct PokemonStorage` 末尾 | 9 sectors × 3968 = 35712 byte と最大。Rogue 1000 体でも余裕。 | PC box の構造体に紛れ込ませるのは設計的に汚い。box 増設 PR / 互換性破壊の risk と衝突する。 |
| E. 専用 sector を新設 | 28..31 の空き ID を再利用 (現状 HoF / Trainer Hill / Recorded Battle で塞がれている) | Rogue 専用領域として明確。 | 新 sector 追加は save format 全体変更に近い。`SECTORS_COUNT` / `NUM_SECTORS_PER_SLOT` の調整。**現時点未実装で、移行コスト最大**。 |

### 推奨設計 (A 案)

```c
// include/global.h (案)
struct SaveBlock3
{
    /* …existing fields… */

#if ROGUE_ENABLE
    struct RogueSave rogue;
#endif
};
```

Rogue 側は:

- `include/config/rogue.h` (新規) に `ROGUE_ENABLE` / `ROGUE_ROSTER_SIZE` などを定義。default `FALSE` (save に影響する feature は default off の規約に準拠 — [docs/STYLEGUIDE.md](../STYLEGUIDE.md), [docs/upgrades/branching_upgrade_policy.md](../upgrades/branching_upgrade_policy.md))。
- `include/rogue/rogue_save.h` (新規) に `struct RogueSave` を定義。
- `src/rogue.c` (新規) に Rogue ロジック。

### Migration

[docs/upgrades/update_migration_notes.md](../upgrades/update_migration_notes.md) の "save block 関連" セクションは現在 **未決**で:

> save block を変更する独自 option / seed / feature flag の migration policy は未決定。

つまり **公式の policy はまだ存在しない**。expansion の既存パターンから推察するなら:

1. `#if ROGUE_ENABLE` で完全 off-able にする → off の player は save 互換性に影響なし。
2. ON にして player に配る場合、migration script を `migration_scripts/<version>/` に追加 (追加 field は zero-init で済む場合が多い)。
3. SaveBlock3 の `STATIC_ASSERT` でサイズ overflow を捕まえる。

### Risks

- **SaveBlock3 容量の他 feature との競合**。DexNav search levels (1 byte × NUM_SPECIES) が GEN_LATEST 進行で伸び続けるため、Rogue + DexNav 両方 ON は monitor 必要。
- **`encryptionKey`** ([include/global.h:608](../../include/global.h#L608)) は SaveBlock2 にあり、Pokemon の OT/ID encryption に使われる。Rogue 用に別の encryption を被せる必要は無いが、`gSaveBlock2Ptr->encryptionKey` で復号する既存資産 (Pokemon struct の `box.fields[]`) を Rogue ロジックがコピーする場合は注意。
- **Save corruption**。Rogue runtime state の更新を「battle 終了直後」に書く設計だと、書き込み中の電源 OFF で run state が壊れる。`TrySavingData(SAVE_NORMAL)` を呼ぶ頻度は通常 save 操作と同じ単位 (PC / 中断セーブ) に揃え、battle 単位で incremental save しない方が安全。
- **link/record battle で SaveBlock 全体が転送される場面**。Battle Tower record mix / Battle Frontier link で `SaveBlock1.battleTower` / `SaveBlock2.frontier` の一部が他 player に渡る経路がある。Rogue は **link 機能と接続しない**前提で SaveBlock3 に閉じ込めれば、record-mix 経路に流れずに済む。

### Conclusion

- **A 案 (SaveBlock3 config-gated 追加)** が最小コスト。`#if ROGUE_ENABLE` を default off にすれば既存 player の save に何も起きない。
- 容量予算は 300 体で 130 byte → 安全圏、1000 体でも 391 byte で収まる。`STATIC_ASSERT(sizeof(struct SaveBlock3) <= 1624)` がコンパイル時検出してくれる。
- **migration policy 自体は project 側に未確立**なので、Rogue を実装する前に [docs/upgrades/update_migration_notes.md](../upgrades/update_migration_notes.md) の policy を 1 段先に固める必要がある (open question として doc に残してある)。

## Cross-Topic Open Questions

- **Battle 中 deterministic RNG**: 必要なら `gIsRogueBattle` 経由の `AdvanceRandom` 抑制 + `SeedRng` 固定の 2 点改修が要る。run の record / replay 機能を作るかどうかで決める。**現時点未実装**。
- **wave 単位の AI 動的切替**: `setdynamicaifunc` で score 加減算は可能。switch logic を含めて切り替えるなら `gDynamicAiSwitchFunc` も同じ要領で OK だが、Rogue 用 dynamic switch func の具体的な要件は未定。
- **save migration policy**: SaveBlock 変更の公式 policy が **現時点未確立**。Rogue 実装前に project rule を 1 本固める必要がある。
- **Battle 終了時の reward UI**: フィールド復帰前に出すか、戻ってからスクリプト経由で出すか。前者は callback 直差し替え、後者は `sTrainerBattleEndScript` 経由で済む。後者の方が field cleanup を再発明せずに済む。
