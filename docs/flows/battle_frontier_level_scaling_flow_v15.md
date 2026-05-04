# Battle Frontier Level Scaling Flow v15

## Purpose

Battle Frontier / Battle Tent の level mode と、対戦用に「低レベルでも Lv.50 として扱う」仕様を追加する場合の設計点を整理する。

調査日: 2026-05-04。source 改造はしておらず、`docs/` への記録のみ。

## Current Behavior

現行の Frontier level mode は [include/constants/global.h](../../include/constants/global.h) の `enum FrontierLevelMode`。

| Mode | Meaning |
|---|---|
| `FRONTIER_LVL_50` | Lv.50 course。敵 facility mon は Lv.50。player eligibility は Lv.50 以下だけを許可する。 |
| `FRONTIER_LVL_OPEN` | Open Level。敵 level は player party の最高 level、ただし下限あり。 |
| `FRONTIER_LVL_TENT` | Battle Tent 用の特別 mode。 |

敵 level は [src/frontier_util.c](../../src/frontier_util.c) の `SetFacilityPtrsGetLevel` / `GetFrontierEnemyMonLevel` が決める。

```c
case FRONTIER_LVL_50:
    level = FRONTIER_MAX_LEVEL_50;
    break;
case FRONTIER_LVL_OPEN:
    level = GetHighestLevelInPlayerParty();
    if (level < FRONTIER_MIN_LEVEL_OPEN)
        level = FRONTIER_MIN_LEVEL_OPEN;
    break;
```

重要: `FRONTIER_LVL_50` は **player の Lv.1 や Lv.49 を battle 中だけ Lv.50 に引き上げる機能ではない**。現行の eligibility は [src/frontier_util.c](../../src/frontier_util.c) の `AppendIfValid` で、Lv.50 course では `monLevel > FRONTIER_MAX_LEVEL_50` を弾く。Lv.50 未満は参加可能だが、そのままの level で戦う。

## Where Level Is Applied

| Target | Current path | Notes |
|---|---|---|
| Enemy facility mons | `SetFacilityPtrsGetLevel` -> `CreateFacilityMon` | Tower / Factory / Pyramid などの施設側生成。 |
| Recorded / apprentice mons | `CreateBattleTowerMon_HandleLevel`, `CreateApprenticeMon` | `GetFrontierEnemyMonLevel` を使う path がある。 |
| Player party eligibility | `CheckPartyIneligibility` -> `AppendIfValid` | species duplicate、held item duplicate、ban、Lv.50 上限を見る。 |
| Player battle stats | normal party-to-battle conversion | `gPlayerParty` の actual level から battle mon が作られる。 |

このため「対戦中だけ全員 Lv.50」は enemy facility generation ではなく、player party を battle に入れる直前か battle mon 作成時の補正として設計する必要がある。

## Recommended Design

目的が対戦型ルールなら、保存データの actual level は変えず、battle 中だけ effective level を補正する。

推奨方針:

| Step | Design |
|---|---|
| 1 | 新しい rule config を追加する。例: `B_FRONTIER_SCALE_PLAYER_TO_LVL_50` か facility-specific runtime flag。 |
| 2 | `FRONTIER_LVL_50` の eligibility は「Lv.50 超過を弾く」か「全 level を許可して Lv.50 化」かを決める。 |
| 3 | player party を battle data へ変換する直前に effective level を 50 にする hook を探す。 |
| 4 | battle 後に actual party level / EXP / HP / status / held item が壊れないことを保証する。 |
| 5 | recorded battle、link battle、Battle Factory rental、Pyramid bag など特殊 path から除外するか対応する。 |

避けるべき実装:

- `SetMonData(&gPlayerParty[i], MON_DATA_LEVEL, 50)` を直接使って戻さない。
- actual EXP と level の整合性を崩す。
- Lv.50 補正を `CreateFacilityMon` だけに入れて player party に効いたつもりになる。

## Possible Hook Points

候補は設計段階。未実装。

| Hook | Pros | Risks |
|---|---|---|
| battle mon 作成時に `gBattleMons[battler].level` を補正 | save data を壊しにくい。battle-only に近い。 | stats recalculation、HP 比率、経験値、controller buffer との整合を要確認。 |
| Frontier battle 開始前に party snapshot を取り、一時的に level/stat を補正して battle 後 restore | 実装は分かりやすい。 | restore 漏れが致命的。進化、経験値、アイテム消費、whiteout、通信系でリスク大。 |
| `GetMonData(... MON_DATA_LEVEL)` の Frontier battle context だけ仮想 level を返す | 呼び出し側を広く拾える。 | global getter の条件分岐は副作用が大きく、UI や save 処理へ漏れやすい。 |

現時点の第一候補は「battle mon 作成時に effective level と stats を補正する」方式。ただし該当 call path の精査が必要。

## Script Flow

Frontier lobby scripts は level mode を `frontier_set FRONTIER_DATA_LVL_MODE, ...` で保存する。

例:

```asm
frontier_set FRONTIER_DATA_LVL_MODE, VAR_RESULT
frontier_checkineligible
frontier_set FRONTIER_DATA_SELECTED_MON_ORDER
frontier_setpartyorder FRONTIER_PARTY_SIZE
```

`FRONTIER_DATA_LVL_MODE` は [include/constants/frontier_util.h](../../include/constants/frontier_util.h) で定義され、`frontier_set` は `src/frontier_util.c` 側の utility function に接続される。

Lv.50 自動補正を入れるなら、script に単発の `setvar` を足すだけでは足りない。battle setup / party-to-battle conversion の C 側 policy が必要。

## Tests Needed

| Test | Expected |
|---|---|
| Lv.1 / Lv.49 / Lv.50 が Lv.50 course に参加 | battle 中の表示 level と stats が方針通り。 |
| Lv.51 が Lv.50 course に参加 | 弾くか、許可して Lv.50 化するか、仕様通り。 |
| battle 後 | actual level、EXP、HP、PP、status、held item が仕様通り復元。 |
| Battle Factory rental | rental mon generation と衝突しない。 |
| Battle Pyramid | Pyramid bag / wild battle / facility battle の level mode が壊れない。 |
| Recorded battle | 再生時の level 表示と damage が記録時と一致。 |
| Link / Union Room | Frontier level mode の補正が通信対戦へ漏れない。 |

## Open Questions

- 仕様は「Lv.50 以下だけ参加、Lv.50 未満は Lv.50 に引き上げ」か、「全 level 参加可、全員 Lv.50 化」か。
- 補正対象は Battle Frontier だけか、通常 trainer battle の対戦モードにも広げるか。
- enemy も player も exact Lv.50 にするのか、Open Level は現行の highest-level rule を残すのか。

