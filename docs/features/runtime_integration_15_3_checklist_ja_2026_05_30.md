# 1.15.3 Runtime 統合チェックリスト 2026-05-30

## 結論

`integration/runtime-dev-20260529` / PR #68 は、1.15.3 期間に実装・検証してきた主要 runtime feature をほぼ取り込み済みです。

`codex review --base master` で出た統合ブロッカー、追加の All Ability Slots
天候エンドターン指摘、追加 review の Pokemon Vendor long-list cursor / row
allocation 指摘は、2026-05-30 の follow-up で実装修正済みです。現時点の判定は「runtime 棚は
おおむね集約済み。PR #68 は runtime integration candidate として ready」です。

`master` 方針は変えません。runtime source / data / graphics / tools 変更は `master` に入れず、この integration branch / PR #68 側で扱います。

## 監査スナップショット

| 項目 | 内容 |
|---|---|
| 監査日 | 2026-05-30 JST |
| 対象 branch | `integration/runtime-dev-20260529` |
| 対象 PR | #68 `[codex] Runtime integration staging` |
| base | `master` |
| master baseline | `4e48ff993f` |
| integration head | review-blocker follow-up included in this integration update |
| runtime 方針 | `master` は docs / Lua-only。runtime 実装は integration lane に保持 |
| review command | `codex review --base master` 完了。追加 P2 群も follow-up 修正済み |

## ステータス凡例

| 表記 | 意味 |
|---|---|
| `[x] 取り込み済み` | PR #68 / integration branch に採用済み |
| `[!] 採用済み・修正必須` | 採用済みだが、統合レビューで修正ゲートがある |
| `[-] 別 lane` | runtime-dev ではなく tooling / randomizer / map 実験として分離 |
| `[~] 吸収済み` | 新しい feature に吸収済み、または旧ブランチとして参照のみ |

## 取り込み済み Runtime Checklist

| 状態 | Feature | 元 PR / branch | 統合証跡 | 判定 |
|---|---|---|---|---|
| [x] 取り込み済み | Battle Item Restore | #47 / `feature/battle-item-restore-current-master-20260519` | `f32387021a` | battle-end held item / berry restore policy の土台として採用済み |
| [x] 取り込み済み | Held Item Catalog | #48 / `feature/held-item-catalog-current-master-20260519` | `bd43370056` | held item 所有トークン化を採用済み |
| [x] 取り込み済み | Party / Status UI 2x3 | #54 / `feature/party-status-ui-overhaul-20260521` | `4eadc80062` | 2x3 party menu baseline として採用済み |
| [x] 取り込み済み | Scout Selection | #51 / `feature/scout-selection-runtime-20260520` | `96366478fd` | Pokemon Champions 型の最大 12 枠 scout UI を採用済み |
| [x] 取り込み済み | Friendly Shop Pokemon Vendor / Global No Evolution | #57 / `feature/global-no-evolution-20260523` | `7f15d175c7` | Pokemon vendor、sealed recruit、global no evolution policy を採用済み |
| [x] 取り込み済み | All Ability Slots | #60 / `feature/all-ability-slots-runtime-20260523` | `3161fb963e` | default `FALSE` の opt-in で採用済み。field / side ability helper と end-turn weather paired ability queue は修正済み |
| [x] 取り込み済み | Champions Run Session | #62 / `feature/champions-run-session-runtime-20260524` | `29f5f11ca8` | run session save / restore MVP を採用済み |
| [x] 取り込み済み | No Random Encounters | #41 / `feature/no-random-encounters-step-only-runtime-20260517` | `6d20de9357` | step-only random encounter suppression を採用済み |
| [x] 取り込み済み | TM Shop Migration | #31 / `feature/tm-shop-migration` | `dc352ebd34` | TM/HM acquisition retirement と reusable TM policy を採用済み |
| [x] 取り込み済み | Unified Move Relearner | #28 / `feature/unified-move-relearner` | `d28f21f8c7` | Summary-first unified move list を採用済み |
| [x] 取り込み済み | Battle Selection / Team Viewer phase2 | branch-only / `feature/battle-selection-mvp`, `feature/prebattle-team-viewer`, `feature/prebattle-team-viewer-phase2` | `b082725110`, `07e13e49c5` | 選出・pre-battle / in-battle viewer は採用済み。restore 前 defeat snapshot と in-battle hint gate を修正済み |
| [x] 取り込み済み | Pokemon State Editor | #23 / `feature/pokemon-state-editor-expansion` | `292a75704b`, `831296f770` | Summary-launched editor と polish を採用済み |
| [x] 取り込み済み | Summary Tera Type Badge | #26 / `feature/summary-tera-type-badge` | `c045b0876c` | Summary Info badge と State Editor coexistence を採用済み |
| [x] 取り込み済み | Trainer Battle Aftercare | #10 / `feature/trainer-battle-aftercare-heal` | `bc7237a568` | default `FALSE` の heal hook として採用済み |
| [x] 取り込み済み | Field Move Modernization / Field Kit | branch-only / `feature/field-move-modernization-mvp`, `feature/field-move-toolkit-item` | `1312cdb528`, `068bf06a78`, `105a6083a5`, `afc48be574`, `74b876f7f1` | HM-free field move と Field Kit は採用済み。mandatory Surf / Dive handoff の bag-full retry を修正済み |
| [x] 取り込み済み | Trainer Partygen Catalog | #7 / `feature/trainer-partygen-catalog-expansion` | `391bd527d7` | Champions / Scout pool 調整用の partygen catalog を採用済み |
| [x] 取り込み済み | Battle BGM Selector / Sound Archive | #39 / `feature/battle-bgm-selector-mvp-20260517` | `3f9e1a4a85` | BGM selector、imported battle tracks、aif2pcm 修正を採用済み |

## 未取り込み / 別 Lane Checklist

| 状態 | Branch / PR | 理由 | 次の扱い |
|---|---|---|---|
| [-] 別 lane | #65 / `feature/map-asset-relinker-20260525` | ROM runtime ではなく map tooling / GUI / Rust core lane | runtime-dev には混ぜない。tooling PR として継続 |
| [-] 別 lane | `feature/EX/ex-rz-upstream1` | randomizer / trainer rank / generated headers / graphics / tools を含む大規模 lane | 別の randomizer integration として再計画 |
| [-] 別 lane | `feature/ex-rz-upstream1` | 旧 randomizer branch | `feature/EX/ex-rz-upstream1` 優先。参照のみ |
| [-] 別 lane | `feature/new-map`, `feature/new-map-test-v15` | map / region map / Fly / route draft 実験 | map relinker / map tooling lane で扱う |
| [~] 吸収済み | `feature/qol_field_moves`, `feature/modern-qol-field-moves`, `feature/field-surfboard` | Field Move Modernization / Field Kit に吸収済み | 旧 evidence branch として保持 |
| [~] 吸収済み | `feature/party-select-ui` | Battle Selection / Team Viewer phase2 に吸収済み | 旧 evidence branch として保持 |
| [~] 吸収済み | `feature/no-random-encounters`, `feature/no-random-encounters-step-only`, `feature/no-random-encounters-step-only-adopt-20260517` | `feature/no-random-encounters-step-only-runtime-20260517` 採用済み | 旧 evidence branch として保持 |
| [~] 参照のみ | `feature/bag-expansion`, `feature/summary-first-relearner-runtime-20260522`, `feature/champions-partygen-next-slice` | 現時点の runtime-dev 採用対象ではない、または docs / reference 色が強い | 必要になった時点で再監査 |

## Codex Review 結果

### 実行結果

| Command | 結果 |
|---|---|
| `codex review --base master "<prompt>"` | CLI 制約で失敗。`--base` と prompt positional は併用不可 |
| `codex review --base master` | 完了。レビュー内で `rtk make -j16 -O check` も実行され、既存の expected / known-failing marker込みで exit 0 |

### Review Findings

| 優先度 | 場所 | 内容 | 次アクション |
|---|---|---|---|
| P1 | `src/battle_setup.c:1582` | Trainer Battle Selection が有効な状態で、選出 3/4 体が全滅しても restore が先に走り、未選出の元手持ちが健康だと敗北判定が false になりうる | 敗北判定を restore 前の compressed battle party 基準にするか、whiteout decision 後に restore を遅らせる |
| P2 | `src/battle_util.c:5384` | `BattlerHasAbility` に置き換えた field / side ability query が `IsBattlerAlive` guard を失い、瀕死 battler の Damp / Aroma Veil / aura 系などが同一ターン後続処理に残る可能性 | `IsAbilityOnField`, `IsAbilityOnFieldExcept` など field / side helper に alive guard を戻す |
| P2 | `data/maps/PetalburgCity_WallysHouse/scripts.inc:23` | mandatory Surf handoff で Field Kit が Key Items pocket に入らない場合、on-frame state が進まず bag-full message loop になりうる。Steven Dive handoff も同種 | mandatory grant を non-failing にするか、失敗時に on-frame state へ留まらない専用 failure path にする |
| P3 | `src/battle_controller_player.c:380` | in-battle Team Viewer cache がない wild battle などでも `R / TEAM / INFO` hint が出るが、実際には viewer が開けない | hint 表示を `PreBattleTeamViewer_TryOpenInBattle` 相当の eligibility に合わせる |
| P2 | `src/battle_end_turn.c:109` | All Ability Slots ON 時、天候エンドターンで `Dry Skin` と `Solar Power` など同じ天候に紐づく複数特性の片方だけが処理されうる | paired weather ability scripts を battler ごとに queue し、明示 ability 呼び出しで all-slot dispatcher へ再入しない |
| P2 | `src/pokemon_vendor.c:546` | one-time 商品を scrolled long-list から購入して sold-out 行が消えたあと、旧 scroll offset のまま `ListMenuInit()` され、範囲外行を描画する可能性 | list rebuild 後に visible count + Cancel に合わせて scroll offset / selected row を clamp する |
| P2 | `src/pokemon_vendor.c:285` | long product table / fragmented heap で `items`, `names`, `productIndexes` のいずれかの確保に失敗した場合、直後に null row buffer へ書き込む可能性 | row allocation を全件検証し、部分確保を解放して初回は script 復帰、購入後 rebuild は vendor close へ unwind する |

### Follow-up Fixes

| 元優先度 | 対応 | 検証 |
|---|---|---|
| P1 | trainer battle selection の敗北分岐は、restore 前の選出 party 全滅状態を snapshot してから元 party を restore する | `rtk make -j16 -O all`, `rtk make -j16 -O debug`, `rtk make -j16 -O check` |
| P2 | `IsAbilityOnSide()`, `IsAbilityOnField()`, `IsAbilityOnFieldExcept()` は HP 0 / absent battler を除外してから all-slot ability predicate を見る | `rtk make -j16 -O check TESTS=test/battle/ability/damp.c`, full `check` |
| P2 | Wally's Dad Surf / Steven Dive の mandatory Field Kit handoff は bag full 時に retry state へ進み、on-frame loop を止める | `rtk make -j16 -O all`, `rtk make -j16 -O debug`, full `check` |
| P3 | Team Viewer action hint は `PreBattleTeamViewer_CanOpenInBattle()` で actual open eligibility と同じ条件に揃えた | `rtk make -j16 -O all`, `rtk make -j16 -O debug`, full `check` |
| P2 | end-turn weather は `eventState.endTurnBlock` で paired ability scripts を 1 つずつ queue し、hidden-slot `Dry Skin` + `Solar Power` のような組み合わせを両方処理する | `rtk make -j16 -O check TESTS=test/battle/ability/all_ability_slots.c` |
| P2 | Pokemon Vendor は list rebuild 後に `PokemonVendorClampListCursor()` で scroll offset / selected row を clamp してから `ListMenuInit()` する | `rtk make -j16 -O check TESTS=test/pokemon_vendor.c` |
| P2 | Pokemon Vendor は `PokemonVendorBuildList()` の row allocations を検証し、失敗時は部分確保を解放して script 復帰 / vendor close する | `rtk make -j16 -O check TESTS=test/pokemon_vendor.c`, full `all` / `debug` / `check`, docs build, mGBA final smoke |

追加で、live-battler guard の hot path 影響に合わせて
`AI_FRAME_CEILING_SINGLES_SMART_TRAINER` を 9 から 10 に更新した。
`rtk make -j16 -O check TESTS=test/battle/ai/ai.c` と full `check` は通過済み。

### Runtime Smoke

| Check | 結果 |
|---|---|
| mGBA Live `integration-review-blocker-smoke` | Pass。debug ROM boot、START input、continue menu screenshot `/tmp/integration-review-blocker-smoke.png` |
| mGBA Live `integration-review-final-smoke` | Pass。ROM boot、START input、continue menu screenshot `/tmp/integration-review-final-smoke.png` |
| mGBA Live cleanup | Pass。`stop` は `alive_after:false`、`status --all` は `[]` |

### Final Validation

| Command | 結果 |
|---|---|
| `rtk git diff --check` | Pass |
| `rtk make -j16 -O check TESTS=test/pokemon_vendor.c` | Pass。vendor ABI / locked display / concealed Egg / unlock helper tests が通過 |
| `rtk make -j16 -O all` | Pass。既存 RWX linker warning のみ |
| `rtk make -j16 -O debug` | Pass。既存 RWX linker warning のみ |
| `rtk make -j16 -O check` | Pass。既存の `EXPECTED_FAIL` / `KNOWN_FAILING` marker を含み exit 0 |
| `rtk mdbook build docs` | Pass。既知の root `CHANGELOG.md` include 警告、`CREDITS.md` `</img>` 警告、large search index 警告のみ |
| mGBA Live `integration-review-final-smoke` | Pass。`pokeemerald.gba` boot、START input、screenshot、clean stop、`status --all` `[]` |

## 現在の判定

| 観点 | 判定 |
|---|---|
| 主要 1.15.3 runtime feature の集約 | ほぼ完了 |
| 未取り込み runtime shelf | 明確な主要 runtime 残りは見当たらない |
| 別 lane | Map Asset Relinker、randomizer、map / Fly 実験は分離継続 |
| PR #68 merge readiness | P1/P2/P2/P3 と追加 P2 群は修正済み。full validation と mGBA Live smoke は Pass。runtime integration candidate として ready |
| `master` 反映 | 不可。runtime 実装は docs-only master policy の対象外 |

## 次の推奨順

1. PR #68 を実装 integration branch として review する。
2. `master` 反映が必要な場合は、別途 docs / Lua-only branch で対象 docs だけを cherry-pick する。
3. Map Asset Relinker / randomizer / map-Fly 実験は runtime-dev に混ぜず、それぞれの lane で継続する。

## 残リスク

- Battle BGM は mGBA Live で selector / preview path は確認済みだが、最終的な audible listening check は手動確認が必要。
- All Ability Slots は opt-in default-off だが、ON にした場合の edge ability 組み合わせはまだ広い。
- Champions Run Session は save / bag / PC / checkpoint の blast radius が大きいため、integration merge 前に combined route の再実機確認が必要。
- Runtime branch の PR は source / graphics / audio assets を含む。docs-only master PR とは絶対に混ぜない。
