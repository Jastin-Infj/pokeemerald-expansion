# Roguelike NPC Capacity Impact v15

調査日: 2026-05-04。実装は未着手。`docs/` への記録のみ。

## Purpose

Rogue (roguelike) 専用 NPC を 100〜300 人規模で追加する計画について、

- 1 マップ / 全体に置けるオブジェクトイベントの枠
- スプライト同時表示の枠
- 利用できるトレーナーフラグ・通常フラグ・var の枠
- セーブブロック / EWRAM への影響
- 実装アプローチ別の現実的な上限

を、ソースを変更しない前提で整理する。関連 doc は [Roguelike Party Policy Impact v15](roguelike_party_policy_impact_v15.md) と [NPC / Object Event Flow v15](../flows/npc_object_event_flow_v15.md)、[Map Script / Flag / Var Flow v15](../flows/map_script_flag_var_flow_v15.md)。

## TL;DR

| 制約 | 値 | Source |
|---|---|---|
| 1 マップに置けるテンプレ数 (`OBJECT_EVENT_TEMPLATES_COUNT`) | **64** | [include/constants/global.h:98](../../include/constants/global.h#L98) |
| 同時 spawn / 画面上の object event 数 (`OBJECT_EVENTS_COUNT`) | **16** (うち player + follower 含む) | [include/constants/global.h:93](../../include/constants/global.h#L93) |
| `OBJECT_EVENT_TEMPLATES_COUNT` の構造的上限 | **126** (`SPECIAL_LOCALIDS_START = LOCALID_CAMERA = 127`) | [src/event_object_movement.c:60-69](../../src/event_object_movement.c#L60-L69) |
| 1 マップの object event 数の保存型 | `u8 objectEventCount` (255 まで構造的に格納可) | [include/global.fieldmap.h:202](../../include/global.fieldmap.h#L202) |
| Saved flag 総数 (`FLAGS_COUNT`) | **2400** = `0x960` (300 byte / SaveBlock1) | [include/constants/flags.h:1642](../../include/constants/flags.h#L1642) |
| `FLAG_UNUSED_*` で名前付きの空きフラグ | **373** | `grep -c '^#define FLAG_UNUSED_'` |
| トレーナーフラグ範囲 (`TRAINER_FLAGS_START..END`) | `0x500..0x85F` = **864** スロット (Emerald, FRLG は 768) | [include/constants/flags.h:1343-1344](../../include/constants/flags.h#L1343-L1344), [include/constants/opponents.h:868](../../include/constants/opponents.h#L868) |
| トレーナーフラグの末尾未使用 | **9** | flags.h ヘッダコメント |
| Saved var 総数 (`VARS_COUNT`) | **256** u16 = 512 byte | [include/constants/vars.h:280](../../include/constants/vars.h#L280) |
| `VAR_UNUSED_*` 名前付き空き | **23** | `grep -c '^#define VAR_UNUSED_'` |
| 1 マップあたり object event template の RAM コスト | 24 byte × 64 = **1536 byte** in `gSaveBlock1Ptr->objectEventTemplates` | [include/global.fieldmap.h:132-163](../../include/global.fieldmap.h#L132-L163), [include/global.h:1127](../../include/global.h#L1127) |

要点だけ:

1. **「1 つの広場に 100 人並べる」は不可能**。テンプレ枠 64 / 同時 spawn 16 を超えられない。1 マップに収めるなら、見えてない人を `addobject` / `removeobject` で動的に出し入れする roster 方式しかない。
2. **マップを分けるなら通る**。Rogue 用の小部屋 6 室 (各 50 人前後) で 300 人ぶん配置できる。各部屋でも同時表示は 14 人ぶん (16 から player + follower を引いた数) が上限。
3. **生のフラグ / var だけなら数千の余裕がある**。300 人ぶんの NPC hide flag を新設しても全体予算 (2400) の 12.5%、現状の名前付き unused 373 にも収まる。
4. **トレーナーフラグも 300 人ぶんなら 864 中 35% で収まる**が、もし全 NPC を `gTrainers[]` に登録するとパーティ data の ROM コストと JSON 維持コストが重くなる。Rogue は **1 個か少数の "virtual trainer" スロットを使い回し**、roster は別 table で持つほうが現実的。
5. **save migration が来るまでは saveblock を膨張させない方針**。現状の枠で組めるなら踏み込まない。

## Object Event Limits

### Per-map limits

[include/constants/global.h](../../include/constants/global.h) にある:

```c
#define OBJECT_EVENTS_COUNT 16
#define OBJECT_EVENT_TEMPLATES_COUNT 64
```

- `OBJECT_EVENTS_COUNT = 16`: live object event の上限。`gObjectEvents[16]`。
  - player と follower がここに入るので、通常 NPC として使えるのは **最大 14**。
  - フォロワー無効時は player のみで 15。
  - 同時 spawn を増やしたければここを上げる必要があるが、`gObjectEvents` は IWRAM / EWRAM の両方に効き、sprite slot や OAM の空きとも干渉する。気軽には触れない。
- `OBJECT_EVENT_TEMPLATES_COUNT = 64`: 1 マップが持てるテンプレの上限。
  - hide flag が立っていれば spawn しないので、**「64 人配置 → 当該フラグで 50 人隠す → 14 人だけ見える」** は成立する。
  - `localId` は `1..OBJECT_EVENT_TEMPLATES_COUNT`。`LOCALID_CAMERA=127` などの予約 ID と被らないように、[src/event_object_movement.c:60-69](../../src/event_object_movement.c#L60-L69) が `static_assert` 相当のコンパイルエラーを掛けている。理論上 **126 まで上げられる** が、saveblock の `objectEventTemplates[64]` 配列が 24 byte × 増分だけ増える。

### Storage cost in SaveBlock1

[include/global.h:1127](../../include/global.h#L1127):

```c
/*0xC70*/ struct ObjectEventTemplate objectEventTemplates[OBJECT_EVENT_TEMPLATES_COUNT];
```

- `struct ObjectEventTemplate` は **24 byte** (`size = 0x18`)。
- 現状 64 entries で **1536 byte / save**。
- これは「**プレイヤーが今いるマップだけ**」のテンプレ。マップ数を増やしてもセーブブロックは増えない。
- マップ移動時に `LoadObjEventTemplatesFromHeader()` が ROM 側のテンプレを上書きコピーする ([flow doc 参照](../flows/map_script_flag_var_flow_v15.md#field-runtime-flow))。
- つまり **「Rogue を別マップに切り出す」だけなら save 容量は増えない**。逆に `OBJECT_EVENT_TEMPLATES_COUNT` を上げるとマップを訪れた時点で全マップに上限ぶんの save 容量を払うことになる (CLAUDE.md にある EWRAM 約 93% の現状を考えると、無闇には上げない)。

### Per-map count is u8

[include/global.fieldmap.h:202](../../include/global.fieldmap.h#L202) で `u8 objectEventCount`。`OBJECT_EVENT_TEMPLATES_COUNT` を 256 以上にしようとするとここでも詰まる。実用上は 126 が天井。

### Real-world usage today

`grep` で集計 (調査時点):

```text
940  : data/maps/*/events.inc に書かれた object_event 行の総数
46   : 単一マップ最大 (Route111)
```

トップは Route111 (46), Route120 (44), Route123/Route119 (43), Route110 / MossdeepCity_Gym (36), …。**現状ですら 64 枠を 7 割使っている map が複数ある**ので、Rogue の追加先として既存マップを選ぶのは避ける。

## Live spawn slot constraints

`gObjectEvents[16]` は overworld で同時に存在できる NPC の総数。Rogue ロビーで「100 人並んでいるように見せる」を実装する場合に効いてくる。

| Use case | 必要な同時 spawn | 現枠 (16) で足りる? |
|---|---|---|
| 自分の周囲 14 タイル以内に 5〜6 人見える「酒場」風 | 6〜8 | ◯ |
| 16 タイル四方の広場で 30〜50 人がうろつくロビー | 30+ | ✗ — `addobject` / `removeobject` で見える範囲だけ spawn する scrolling roster が必要 |
| roguelike 標準の「ボス + 取り巻き 3 名 + 商人 2 名 + 通行人 4 名」 | ~10 | ◯ |

実装上、見える範囲だけ spawn する仕組みは新規。対応するなら:

- **camera 中心からの距離で template を `addobject` / `removeobject`** する常駐 task。NPC 全員のテンプレは map 側に置くが live spawn は近接 16 人だけにする。
- このとき hide flag (`object_event` の `event_flag` 列) は使わず、`addobject` での spawn 制御だけで賄う。`removeobject` は flag を set する仕様 ([NPC flow doc 参照](../flows/npc_object_event_flow_v15.md#spawn-and-hide-flag)) なので、毎ターン set する設計にすると後で flag clear 漏れの保守事故になりやすい。代わりに **`hideobjectat` / `showobjectat`** を使う。これなら template flag を触らずに live invisibility だけ動く。
- ただし `gObjectEvents` 自体が 16 までなので、表示中 NPC は 14 が上限。

## Flag Budget

### Total budget

[include/constants/flags.h:1642](../../include/constants/flags.h#L1642):

```c
#define FLAGS_COUNT (DAILY_FLAGS_END + 1)   // = 0x960 = 2400
```

[include/global.h:143](../../include/global.h#L143) で `NUM_FLAG_BYTES = ROUND_BITS_TO_BYTES(2400) = 300`。SaveBlock1 にビット列 300 byte を持っている。

| 区域 | 範囲 | 用途 |
|---|---|---|
| Temp flags | `0x000..0x01F` | map load 毎に clear。スクリプト一時状態。NPC 配置には使えない。 |
| 一般 flag (hidden item / NPC hide / received item / 進行イベントなど) | `0x020..0x4FF` | NPC hide flag、item ball flag、進行 flag。**Rogue NPC の hide flag を新設するならここ**。 |
| Trainer flags | `0x500..0x85F` (864 個) | `trainerbattle` で勝敗を記録するスロット。 |
| System flags | `0x860..` | バッジ、Pokenav、game-clear など。触らない。 |
| Daily flags | `..0x95F` | 日付リセット系。Rogue で「日次ボス」枠を作るなら候補。 |
| Special flags | `0x4000..0x407F` (EWRAM のみ、save されない) | follower 一時 hide、map name popup 抑制など。 |

### Named-unused 空き

```text
grep -c '^#define FLAG_UNUSED_' include/constants/flags.h
=> 373
```

373 のうち多くは `0x020..0x4FF` の一般領域に散らばっている。**300 人ぶんの hide flag を確保する余裕は数値上はある。** ただし:

- 既存の Rogue 計画でも DexNav / Champions / Battle Selection 等が将来 flag を要求する。flag は早い者勝ちの希少資源として扱う。
- 全 Rogue NPC に hide flag を 1 個ずつ割り当てる素朴設計より、「**Rogue ロビーは map_script で一括 hide 制御**」のほうがフラグを食わない。具体的には:
  - NPC 配置側 (`object_event ... 0`) で hide flag を `0` (= 使わない) にする。
  - ロビーに入った時点で `MAP_SCRIPT_ON_TRANSITION` から `clearflag` で全員 visible に。
  - 個別 NPC をストーリー進行で永続消去するなら、その時点で初めて 1 個 flag を割り当てる。

これで flag 消費は「永続的に消す必要がある NPC の数」だけに圧縮できる。Rogue の通常通行人なら 0 でよい。

### Trainer flag 上限と運用

- `MAX_TRAINERS_COUNT_EMERALD = 864` ([include/constants/opponents.h:868](../../include/constants/opponents.h#L868))。各エントリは `gTrainers[]` の party / class / AI flags / sight / pic を持ち、ROM コストが大きい。
- `trainerbattle` を使うと `TRAINER_FLAGS_START + trainerId` のフラグ上で勝敗を記憶する。300 人ぜんぶに固有 trainer slot を割り振ると 35% を一気に消費し、後のストーリー追加余地を圧迫する。
- 推奨パターンは Battle Frontier / Factory と同じく、**少数の "virtual trainer" 枠 + roster table** を別途用意する方式。
  - `TRAINER_BATTLE_PARAM.opponentA` を runtime で書き換えて `BattleSetup_StartTrainerBattle` を直叩きする経路は src 側で既存。
  - 1 戦ごとに勝敗 flag を保持したいなら、roster index → SaveBlock 上の bitfield (Rogue 専用) を新設するほうがクリーン。trainer flag 領域は触らない。
  - これは `docs/overview/roguelike_party_policy_impact_v15.md` の「facility runtime state」議論と整合する。

## Var Budget

- `VARS_COUNT = 256` u16 ([include/constants/vars.h:280](../../include/constants/vars.h#L280))。
- 名前付き unused は **23 個** (`VAR_UNUSED_0x40FX` 周辺)。
- Rogue runtime state (現在 wave、roster index、報酬 RNG seed、reroll 回数 …) を全部 saved var で持つと数十 var 食う。
- 推奨は「Rogue 側で `struct RogueState` を SaveBlock に追加し、saved var は entry/exit interface だけに使う」。**1〜3 個の var だけで Rogue 開閉を扱えるはず。** 詳細は `docs/overview/roguelike_party_policy_impact_v15.md` の facility state 表を参照。

## Implementation Approach Comparison

| 方式 | 1 マップに置く NPC 数 | 同時表示 | flag コスト | save コスト | Rogue 100〜300 人達成可? |
|---|---|---|---|---|---|
| A. 単一広間 / 全員 `object_event` | ≤ 64 (拡張時 ≤ 126) | ≤ 14 | 永続消去ぶんだけ | テンプレ 24 byte × 増分 | ✗ 単独では 100 を越えられない |
| B. 複数 Rogue 部屋に分割 | 各 ≤ 64 | 各 ≤ 14 | 同上 | マップ数だけ ROM。saveblock は据え置き | ◯ 6 部屋 × 50 で 300 まで |
| C. 動的 roster (1 部屋に小プールの template、`addobject` で出し入れ) | 表示プール 16 のみ。論理 roster は別 table | ≤ 14 | 0 (flag 不要) | RogueState ぶん | ◯ 100〜300 を 1 部屋で表現可能 |
| D. NPC 全員に `gTrainers[]` slot | 上記いずれか | 上記いずれか | 1 / 人 (TRAINER_FLAG) | gTrainers[] エントリぶん ROM | △ 数値上は 864 まで可。party / pic を全部用意するコストが現実的でない |
| E. Rogue 専用 trainer roster (virtual slot) | 上記いずれか | 上記いずれか | 1 (virtual slot 1 個) + Rogue 側 bitfield | Rogue 側 SaveBlock | ◎ 推奨 |

### 推奨

C + E の組み合わせ:

1. Rogue 入口マップを **1〜数枚**作る。
2. 各マップは「**枠だけの object_event**」を 16 個ぶん用意する (graphics は generic NPC、script は `Rogue_NpcInteract`)。
3. 起動時に Rogue runtime が roster table から 14 人を選び、`setobjectxyperm` + `setobjectmovementtype` + `gObjectEventGfxId` で graphics / 配置を上書き。
4. 戦闘は virtual trainer slot 1 個 + Rogue roster index で開始。勝敗は Rogue SaveBlock の bitfield に書く。
5. 通常マップ flag / trainer flag / object event template count はいずれも **据え置きで実装可能**。

これなら Rogue 規模を 300 人から 1000 人に増やしても、ROM 上の roster table が伸びるだけで EWRAM / saveblock / flag 予算には波及しない。

## Risk Notes

| Risk | Why | Mitigation |
|---|---|---|
| `OBJECT_EVENT_TEMPLATES_COUNT` を上げる選択 | saveblock `objectEventTemplates[]` が線形に増える。EWRAM 予算がきつい (CLAUDE.md 参照)。 | C 方式で枠拡張せずに済ませる。本当に必要なら 64 → 96 程度に留め、save migration と一緒に出す。 |
| Rogue NPC ごとの hide flag を素朴に 1 個ずつ確保 | 名前付き unused 373 を一気に削る。後の DexNav / Champions / Region Map 機能が flag 不足になる。 | 通常通行人は flag 不要。永続削除が必要な NPC だけ flag を取る。 |
| `gTrainers[]` に 100 人登録 | ROM / .party file 維持負担。Rogue は roster をランダム化したいので毎エントリを書くのは目的に反する。 | virtual trainer + roster table。`docs/features/scout_selection_*` 系の roster 設計と統一する。 |
| `addobject` / `removeobject` で flag を意図せず set | `removeobject` は object hide flag を `FlagSet` する ([NPC flow doc](../flows/npc_object_event_flow_v15.md#spawn-and-hide-flag))。flag を持たないテンプレに `removeobject` するとこの set が空振りになり、後の同 localId への再 spawn が「すでに hide」の状態と区別できなくなる。 | 一時 hide には `hideobjectat` / `showobjectat`。flag は触らない。 |
| Roster runtime swap で follower / camera object と localId が衝突 | localId 上限は 64 (拡張時 126) で、127 (`LOCALID_CAMERA`)、254 (`LOCALID_FOLLOWING_POKEMON`)、255 (`LOCALID_PLAYER`) 等は予約済み。 | テンプレ pool は localId 1..16 程度に固定し、roster 側は別 index で参照する。 |

## Flag-Loop Pattern (検討案) と限界

「テンプレを N 個ループさせて、flag で NPC ID を識別する」という素朴案を考えると、現状仕様で何が起きるかを書き残しておく。

### 案

- ロビーマップに `object_event` を 16 個ぶんだけ用意し (graphics は generic)、Rogue runtime が NPC ID 1〜300 を順に template に流し込む。
- 「NPC #N に勝った」を覚えるために `FLAG_ROGUE_NPC_DEFEATED_BASE + N` のような flag を 300 個割り当てる。

### なぜ詰まるか

1. **flag は消費型で再利用が不可逆**。NPC 1 を倒した → flag set。次の run で NPC 1 を初期化したい → `clearflag` するしかない。が、その瞬間「過去 run で倒した」記録が消える。**「1 run 内の戦績」と「全 run 累計の戦績 (永続実績)」を 1 本の flag で混ぜると壊れる**。
2. **flag は global namespace**。300 個確保した時点で、後から DexNav / Champions / Region Map / Battle Selection 等の機能用に 1 個取りたいときに「Rogue が予約している領域を踏まないように」という制約が永続化する。隣接機能のバグ温床になりやすい。
3. **template のループ自体は問題ない**。問題は「NPC ID → flag」のマッピングを `flags[NUM_FLAG_BYTES]` に置くこと。マッピング先を変えれば全部回避できる。

### 逃げ道: Rogue 専用 SaveBlock 構造体

`flags[]` を一切触らずに、Rogue 機能だけが見る独自の bitfield を SaveBlock に新設する:

```c
// Rogue runtime state (案)
struct RogueSave
{
    u8 npcDefeatedThisRun[(ROGUE_ROSTER_SIZE + 7) / 8]; // run 内戦績
    u8 npcDefeatedAllTime[(ROGUE_ROSTER_SIZE + 7) / 8]; // 累計
    u8 npcMet[(ROGUE_ROSTER_SIZE + 7) / 8];             // 出会った
    u16 currentWave;
    u32 runSeed;
    /* ... */
};
```

サイズ感:

| Roster size | bitfield 1 本のコスト | 3 本 (defeated this run / all time / met) |
|---|---|---|
| 100 | 13 byte | 39 byte |
| 300 | 38 byte | 114 byte |
| 1000 | 125 byte | 375 byte |

これは `flags[]` (300 byte 全体) とは独立。Rogue を OFF にしても通常 flag 予算は無傷で、Rogue を 1000 人規模に拡張しても通常 game 進行には波及しない。ROM 上の roster table と一緒に Rogue feature 単位で完結する。

`SaveBlock1` か `SaveBlock2` のどちらに付けるかは別議論 (`docs/flows/save_data_flow_v15.md` に save migration 議論があるので、そこと合わせる)。

### Trainer flag を Rogue NPC ごとに使うべきか

同様に、`TRAINER_FLAGS_START..END` (864 スロット) を 300 個ぶん消費する案も「使わない」を推奨。理由は同じく namespace 圧迫と、`gTrainers[]` のパーティ data ROM コスト。virtual trainer slot 1 個 + Rogue roster index で勝敗は上の `RogueSave.npcDefeatedThisRun` に記録するほうが、roster を runtime ランダム化したい設計と整合する。

## Physical Capacity Walls (本当の壁)

`flags[]` と object event template を回避できたとしても、GBA ハードウェアと ROM 側に別の上限がある。Rogue 100〜300 人計画でぶつかるのはこちら。

### Sprite palette: ハードウェア 16 スロット

GBA の OBJ palette は 16 個。背景タイル / battle UI / overworld の他 sprite と共有しているので、**フィールド NPC 用に同時にロードできる palette は実用 8〜12 個**。

- [src/event_object_movement.c:489](../../src/event_object_movement.c#L489) の `sObjectEventSpritePalettes[]` は NULL-terminated table で、tag 検索 (`OBJ_EVENT_PAL_TAG_*`) でロードする。
- 同じ tag の palette を持つ NPC は palette を共有する。
- **300 人いても、見える 14 人ぶんの palette tag を 8 種類以下に揃えれば足りる**。Rogue 用に共有 palette set を 4〜8 個切り、roster の "見た目バリエーション" をその中に押し込む。
- 同時に画面上で「赤髪 NPC・青髪 NPC・金髪 NPC・…」と 12 種類超を表示しようとすると、ハードウェアスロットを使い切る。Rogue ロビーの設計上は「同時表示は 6〜8 種類に制限」が現実的。

### Graphics ID: 388 種類 + 16 dynamic slot

[include/constants/event_objects.h:426](../../include/constants/event_objects.h#L426):

```c
#define NUM_OBJ_EVENT_GFX                        388
#define OBJ_EVENT_GFX_VARS   (NUM_OBJ_EVENT_GFX + 1)
#define OBJ_EVENT_GFX_VAR_0..VAR_F                     // 16 個
```

- 388 種類は十分な数。Rogue NPC ぶんの spritesheet を 30〜50 種類追加しても余裕がある。
- `OBJ_EVENT_GFX_VAR_*` はランタイムで `VarSet(VAR_OBJ_GFX_ID_X, OBJ_EVENT_GFX_*)` 経由で「同じ template に違う見た目を入れる」ための設計済み口。Rogue 動的 roster と相性が良い。
- spritesheet 自体の ROM コストは小さくない (1 NPC 約 1.5〜3 KB)。300 人ぶんの**ユニーク sprite** はやらない。色違い (palette swap) や姿勢違いを増やす方向で見た目を膨らませる。

### Trainer party data の ROM コスト

`.party` files が `trainerproc` で `gTrainers[].party` に展開される。1 trainer の party は構造体 (各モン 16〜36 byte) × N 体。CLAUDE.md にもある通り Emerald は EWRAM/IWRAM が約 93% 使用済み。ROM はもう少し余裕があるが、

- 6 体パーティ × 36 byte = 216 byte / trainer
- 300 trainer なら 約 65 KB

これは ROM 上で吸収可能だが、それぞれを `.party` ファイルとして手書きすると保守コストが爆発する。**Rogue 用 party は roster table + 生成器で動的に組む**ほうが妥当。`gTrainers[]` には virtual slot を 1〜数個だけ置く。生成器は species pool, level curve, item pool, AI flag set を入力に取る関数。

### `gObjectEvents[16]` の同時 spawn

すでに TL;DR に書いた通り、画面上の生 NPC は 14 が上限。これは Rogue だけ拡張すると battle / cutscene 系の他処理に副作用が出るので触らない。Rogue 側は「14 人だけ見える」を前提に演出する。

### 結論: 300 体は物理的に "足りる"

| Resource | 300 体での消費 | 残り余地 |
|---|---|---|
| `flags[]` | **0** (RogueSave に隔離) | 通常機能用に 100% 残る |
| Saved vars | 0〜数個 | 通常機能用にほぼ 100% 残る |
| Trainer flags | 0 (virtual trainer 経路) | 100% 残る |
| Object event templates | 16 / map (枠のみ) | 通常マップに非干渉 |
| Sprite palette | 同時 8 種類以下に圧縮 | 安全圏 |
| Graphics ID | 30〜50 spritesheet 追加 | 388 中の小割合 |
| ROM (party data) | 〜数十 KB | 生成器化で大幅圧縮可 |
| RogueSave bitfield | 100〜400 byte | 新設、SaveBlock 影響を予測可能 |

**「物理キャパが足らない」は現状仕様のままなら正しい**が、**Rogue 機能を `flags[]` から切り離して RogueSave に閉じ込める設計**を取れば、300 体どころか 1000 体規模でも通常 game 予算を侵さずに乗ります。逆に、これをやらない (flag 1 個 = NPC 1 人) と 300 体で本編側の機能拡張が止まります。**Rogue 設計の最初の一手は「Rogue 専用 SaveBlock 構造体を切ること」**で、ここを決めないままマップ / NPC / battle entry を組むと後から剥がせなくなる、というのが本 doc の結論です。

## Open Questions

- Rogue ロビーは固定 1 マップにするか、ステージ毎にマップを切るか。後者なら flag / saveblock 影響はさらに軽い。
- Rogue 用 roster table を `data/` に置くか、ROM-only な C 配列にするか (前者なら JSON 連動の自動生成スキームが必要)。
- 既存 Battle Frontier / Tower の virtual trainer 経路 (`gTrainerBattleOpponent_A` 直書き) を流用できるか、Rogue 用に別の構造体を切るか。次回は `src/battle_setup.c` と `src/battle_tower.c` を比較する必要がある。
- マップ数を増やす場合、map group を新設するか既存 BattleFrontier 系に追加するか。`include/constants/map_groups.h` は generated なので、`map_groups.json` 側の手当てが要る。
- `RogueSave` を `SaveBlock1` に追加するか `SaveBlock2` か別の新設 block か。save migration を伴うので [save_data_flow_v15.md](../flows/save_data_flow_v15.md) と一緒に設計する必要がある。
- 「全 run 累計の NPC 撃破実績」を持つかどうか。持つなら `npcDefeatedAllTime` を増やす。持たないなら 1 本だけで足りる。
- Rogue 専用 sprite palette pool を本編 NPC 用 palette と分けるか共有するか。共有すると ROM が小さくなるが、Rogue ロビーで「Rogue らしい色合い」を作りにくい。
