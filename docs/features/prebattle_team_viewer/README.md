# Pre-Battle / In-Battle Team Viewer

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-17 |
| Baseline | `master` `7c19f56901`; `git describe` = `expansion/1.15.2-38-g7c19f56901` |
| Code status | Phase 2 integrated selection implemented; build/check and focused mGBA routes passed |
| Provenance | Local project feature docs |

Status: Phase 2 implemented. Focused mGBA routes validated single 3-of-6,
double 4-of-6, player Summary entry, in-battle viewer, and action-menu return.
Code status: preserved as completed shelf #20 (`feature/prebattle-team-viewer`),
closed 2026-05-17 after CI success; docs-only master handoff should use docs-only branch / PR. Older references to
`feature/prebattle-team-viewer-phase2` below are historical implementation notes
from the branch that supplied the current PR contents.

## Goal

通常 trainer battle の開始前に、相手の手持ち 6 匹と自分の手持ち 6 匹を
確認できる team viewer を出す。
さらに battle 中の action menu からもボタンで同じ team view を開けるようにする。

最終的な理想は Pokémon Champions 風の選出画面に近い UI で、相手 team を見ながら
single なら 3 匹、double なら 4 匹を選出できる状態。

## Current Decision

この feature は `feature/battle-selection-mvp` の上へ直接積まない。
`feature/battle-selection-mvp` は現 `master` より古い docs baseline を持つため、
その branch を base にすると bag expansion / field move docs などを落とす差分が出る。

実装は current `master` から切った `feature/prebattle-team-viewer-phase2` 上で、必要な
battle selection source だけを reapply してから team viewer と統合選出を追加した。
pre-battle viewer と in-battle viewer は同じ opponent cache を使う。

推奨 branch 形:

| Purpose | Branch |
|---|---|
| docs / planning | `feature/prebattle-team-viewer-phase2` |
| runtime implementation | `feature/prebattle-team-viewer-phase2` |
| dependency source | selected source files from `feature/battle-selection-mvp`; the whole old branch was not merged |

## Implemented Shape

Phase 2 の実装は、battle 前に専用 team preview / selection screen を挟み、
battle 中は read-only viewer として同じ情報を開けるようにする。

1. trainer battle の `gBattleTypeFlags` が確定する。
2. 対象 battle なら opponent party を preview cache として確定する。
3. team viewer を表示する。
4. player-side `A` で 3 / 4 匹を pick / unpick する。
5. 必要数に達したら `START` で確定し、既存 battle selection restore path へ選出順を渡す。
6. 選出確定後、cached opponent party と selected player party で battle を開始する。
7. battle 中の player action menu で `R_BUTTON` を押すと read-only team viewer を開く。
8. battle 中の player action menu には `R / TEAM / INFO` の小さい 32x32 hint を
   表示し、viewer shortcut が存在することを見せる。
9. battle 中 viewer は display-only とし、D-pad / `SELECT` は無視する。
10. viewer を閉じると action menu に戻り、battle command はまだ未選択のままにする。

Implemented note: GBA has no physical `Y_BUTTON`, so the Champions-style strength view is
mapped to `SELECT_BUTTON` by `B_TEAM_VIEWER_DETAILS_BUTTON`.

## Official Reference Check

2026-05-10 に公式 site を確認した。

| Source | Confirmed point |
|---|---|
| https://www.pokemonchampions.jp/ja/ | Pokémon Champions は type、ability、move selection を戦略要素として扱う。 |
| https://www.pokemonchampions.jp/ja/battle/ | Battle modes include single battle and double battle. |
| https://www.pokemonchampions.jp/ja/pokemon/ | Team building includes recruited Pokémon and Pokémon HOME visitors; HOME visitors may need move changes through training. |
| https://www.pokemonchampions.jp/ja/training/ | Training can raise stats and change abilities / moves. |
| https://champions.pokemon.com/en-us/?pubDate=20250228 | Official page metadata includes a team menu screenshot and a training menu screenshot with stat points, stat alignment, moves, and Ability categories. |

Exact `Y` button behavior for the battle selection screen was not found as text on the
official pages. Treat the user-provided reference image / official footage as the UI
reference, and verify exact button label / behavior during implementation research before
locking the final interaction copy.

Older console battle titles were also checked as secondary references. Colosseum / XD /
Battle Revolution support the idea of a formal team card / pass, visible 6-Pokémon teams,
and 3v3 singles / 4v4 doubles chosen from 6, but they are not the primary UI target.
Details are in [Legacy Console Battle UI References](legacy_console_references.md).

## UI Direction

GBA の 240x160 screen では添付 reference の Switch 画面をそのまま再現できない。
ただし構造は寄せる。

| Area | MVP display |
|---|---|
| player side | 自分の最大 6 匹。Pokemon icon と slot number を 3x2 grid で表示。 |
| opponent side | 相手の最大 6 匹。Pokemon icon と slot number を 3x2 grid で表示。 |
| center / footer | `A` pick / unpick、`START` confirm、player-side `SELECT` Summary、single 3 / double 4 の required count。 |
| selection | Phase 2 は viewer と selection を統合。player side で `A` pick / unpick、`START` confirm。 |
| in-battle mode | display-only。`R` / `B` / `A` close。D-pad / `SELECT` は無視し、選択や party mutation はしない。 |

player-side `SELECT` は独自 detail ではなく通常の Pokemon Summary を開き、
`POKEMON SKILLS` page から開始する。`TEAM_VIEWER_SUMMARY_ALLOW_MOVE_REORDER`
を `1` にすると通常 Summary と同じ技並び替えを許可し、既定の `0` では
`SUMMARY_MODE_LOCK_MOVES` で技並び替えを抑止する。opponent side は MVP では
species / type / gender / level などの public preview footer に留める。
相手の moves、ability、held item、stat allocation は、公式 UI が明確に公開していると
確認できるまで表示しない。

2026-05-10 follow-up: 初回 manual check で下部 screen corruption が見つかった。
原因は viewer BG の `charBaseIndex = 3` と screenblock 31 の重なりで、footer window
tile が BG map を上書きしていたこと。実装は `charBaseIndex = 0` に移し、同時に
名前主体の縦リストを Pokemon icon grid に置き換えた。

同日の追加 follow-up で、icon label は出るが Pokemon icon が出ない問題と、
後続 battle path の malloc blue screen を修正した。icon は viewer が OBJ 表示を
有効化していなかったことが原因で、`DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP` を立て、
BG priority も下げて icon OBJ を window 背景より前に出す。
malloc 側は viewer へ入る前の field / battle window buffers を `InitWindows()` 前に
解放していなかったことが原因で、viewer 初期化時に `FreeAllWindowBuffers()` を呼ぶ。

さらに同日の layout polish で、cursor 移動時に full redraw と icon 再生成を走らせていた
処理を text label だけの redraw に分離した。これにより D-pad 移動中の divider / lower
bar の一瞬抜けと icon の再生成ちらつきを抑える。UI は標準 frame ではなく thin white
window に変え、detail panel の level は `LV.` 表記、in-battle viewer は slot number なしにした。

追加 polish では、battle 中 viewer が battle BG0 scroll を引き継いで白 window を下へ
ずらし、上側が黒背景に見える問題を避けるため、viewer entry で BG0 scroll を明示的に
reset する。`YOUR TEAM` / `OPPONENT` は icon grid 内ではなく top header に移し、
main grid は pre-battle では slot number のみ、in-battle では label なしにする。
team/footer の境界は 1 tile 下げ、footer prompt は少し下寄せにする。

追加の manual validation で、battle action menu 表示直後に viewer を開くと赤く tint される
ケースと、viewer を `B` で閉じた後に battle textbox が空のまま action menu が戻らない
ケースが見つかった。赤 tint の最終原因は viewer window が palette 15 を使っていたこと。
standard menu palette は palette 14 にロードされるため、viewer window は
`STD_WINDOW_PALETTE_NUM` に合わせた。in-battle viewer close は battle screen reshow 後に
action prompt/menu を再描画する player-controller redraw flag を使う。

2026-05-10 の focused mGBA validation では、debug-only `Party -> Team Viewer Battle`
route で pre-battle viewer、3体選出、trainer battle、action menu、`R` in-battle viewer、
`B` return to visible action menu まで確認した。証跡 screenshot は
`/tmp/prebattle-team-viewer-real-prebattle.png`、
`/tmp/prebattle-team-viewer-real-action-menu.png`、
`/tmp/prebattle-team-viewer-real-inbattle.png`、
`/tmp/prebattle-team-viewer-real-return.png`。

追加 runtime check で、battle 中 viewer の裏で `callback1` が動き続け、D-pad /
`SELECT` / `A` が action / move / target 入力へ漏れる問題を確認した。in-battle viewer
open 中は `gMain.callback1` を退避して NULL にし、battle screen reshow 後に戻す。
さらに player controller は全キー release まで入力受付に戻らない。`R` viewer 内で
D-pad / `SELECT` を押しても display のまま維持され、`A` 押しっぱなし close でも
Fight / move selection へ進まないことを
`/tmp/prebattle-team-viewer-readonly-after-dpad-select.png` と
`/tmp/prebattle-team-viewer-readonly-held-a-return.png` で確認済み。

追加の focused mGBA validation では、pre-battle footer の縦余白を増やした layout、
battle action menu の `R / TEAM / INFO` hint、in-battle viewer footer の read-only copy、
D-pad / `SELECT` 無視、`B` return、`A` close 後に Fight / move selection へ暴発しない
ことを `prebattle-team-viewer-team-info-hint` で確認した。証跡 screenshot は
`/tmp/prebattle-team-viewer-team-info-prebattle.png`、
`/tmp/prebattle-team-viewer-team-info-action-hint.png`、
`/tmp/prebattle-team-viewer-team-info-opened.png`、
`/tmp/prebattle-team-viewer-team-info-inbattle.png`。
`TEAM INFO` 文字色は `prebattle-team-viewer-team-info-dark` で Move Info と同じ暗い
palette index に寄せたことを `/tmp/prebattle-team-viewer-team-info-dark-action-hint.png`
で再確認した。

Team Info hint は Move Info と同じ slide model に合わせた。X=-14 から X=14 へ
表示され、通常 action / Bag / Run / debug / last-used-ball などでは X=-14 へ戻ってから
sprite tiles / palette を解放する。Bag などの menu 遷移で sprite system が reset され、
hide callback が最後まで走らない場合は、次の action menu restore 時に sprite tile tag を
見て stale active flag を落とし、hint を再生成する。
`prebattle-team-viewer-team-info-slide-v4` では action menu 表示、Bag 内で残留しないこと、
Bag から戻ると再表示されること、Fight 遷移後は Move Info だけが表示されること、
R viewer に入っても hint residue がないことを確認した。
`prebattle-team-viewer-details-persist` では、pre-battle viewer の `SELECT` detail が
右 / 左移動と player / opponent side 切替後も開いたまま更新されることを確認した。

W / double battle 用に debug-only `Party -> Team Viewer W` route を追加した。この
route は Amy & Liv の通常 double trainer を使い、viewer 内で 4 匹選出してから
double battle action menu へ入る。W action menu では
`TEAM_VIEWER_ACTION_HINT_Y_DOUBLE = 102` を維持し、double 用 `MOVE INFO` と同じ
Y 座標系に合わせる。single の `TEAM_VIEWER_ACTION_HINT_Y_SINGLE = 92` も維持する。
`prebattle-team-viewer-w-debug3` では 4/4 選出、double battle action menu の
`R / TEAM / INFO` hint、`R` からの in-battle viewer 表示を確認した。証跡 screenshot は
`/tmp/prebattle-team-viewer-w-debug-route-start.png`、
`/tmp/prebattle-team-viewer-w-debug-four-selected.png`、
`/tmp/prebattle-team-viewer-w-debug-battle-intro.png`、
`/tmp/prebattle-team-viewer-w-debug-action-hint.png`、
`/tmp/prebattle-team-viewer-w-debug-inbattle-viewer.png`。MoveInfo 座標合わせ後の
action hint 再確認は
`/tmp/prebattle-team-viewer-w-debug-moveinfo-aligned-action-hint.png`。

Phase 2 では、pre-battle viewer から既存 choose-half 画面へ遷移せず、viewer 内で
選出まで行う。`A` で player-side Pokemon を pick / unpick し、必要数に達したら
`START` で battle へ進む。選出済みの player label は元 slot number を選出順の数字へ
置き換え、低コントラストのクリーム背景とオレンジブラウン文字で表示する。
marker rectangle と text render offset は `src/prebattle_team_viewer.c` の
`TEAM_VIEWER_SELECTED_MARKER_*` / `TEAM_VIEWER_SELECTED_TEXT_*` で管理する。
座標は slot label origin からの相対値で、現行は番号の視認性を優先して少し右上へ寄せている。
marker の塗りと text printer の背景は分離し、番号テキスト自体は透明背景 / 透明 shadow で描画する。

player-side の `SELECT` は簡易 footer ではなく、通常の Pokemon Summary を
`POKEMON SKILLS` page で開く。技並び替えの許可 / 抑止は
`TEAM_VIEWER_SUMMARY_ALLOW_MOVE_REORDER` で切り替える。`prebattle-team-viewer-summary-marker` では
Summary へ遷移して戻れること、slot 6 を最初に選ぶと表示が `6` ではなく `1` に
変わること、3 匹選出後に trainer battle へ進むことを確認した。証跡 screenshot は
`/tmp/prebattle-team-viewer-summary-marker-start.png`、
`/tmp/prebattle-team-viewer-summary-skills-page.png`、
`/tmp/prebattle-team-viewer-summary-marker-slot6-first.png`、
`/tmp/prebattle-team-viewer-summary-marker-three-selected.png`、
`/tmp/prebattle-team-viewer-summary-marker-battle-start.png`。
marker 座標調整後の追加確認は `prebattle-team-viewer-marker-adjust` で行い、
`/tmp/prebattle-team-viewer-marker-adjust-slot6-first.png` と
`/tmp/prebattle-team-viewer-marker-adjust-summary-skills.png` を残した。
text 背景分離後の確認は `prebattle-team-viewer-transparent-text` で行い、
`/tmp/prebattle-team-viewer-transparent-text-slot6-first.png` を残した。
Summary 初回 `SELECT` の layout 崩れ修正後は `prebattle-team-viewer-summary-initial-layout`
で確認し、`/tmp/prebattle-team-viewer-summary-initial-layout-fixed.png` を残した。
その後、`SKILLS` 直入り用に `SKILLS` tilemap を左半分へコピーすると `INFO` に戻った
時に `SKILLS` 背景が残ることを確認したため、通常 Summary の横スクロール構造に合わせて
BG X 座標で右半分を表示する修正へ変更した。`prebattle-team-viewer-summary-info-layout-fixed`
では `SKILLS -> INFO -> SKILLS -> viewer return` を確認し、
`/tmp/prebattle-team-viewer-summary-info-layout-skills-fixed.png` と
`/tmp/prebattle-team-viewer-summary-info-layout-info-fixed.png` を残した。

button assignment は build-time config にする想定。第一候補は trainer battle の
action menu 中だけ `R_BUTTON`。default config では `L_BUTTON` は move description、
`R_BUTTON` は last used ball、`START_BUTTON` は HP 表示切替 / gimmick、
`SELECT_BUTTON` は debug / move rearrange と競合するため、in-battle viewer では
`SELECT` を無効化し、action menu の trainer battle に限定して差し込む。

## Scope

### In Scope

- 通常 trainer battle 前の team viewer。
- 自分の手持ち 6 匹と相手の最大 6 匹の表示。
- battle selection MVP との接続。
- battle 中 action menu からの read-only team viewer。
- preview と本戦 opponent party の一致。
- Trainer Party Pools / randomize / override trainer の扱い。
- Pokemon icon sprite / palette lifetime の専用画面設計。

### Out of Scope

- Battle Frontier / link / Union Room / Pyramid / Trainer Hill / two opponents / follower partner の対応。
- battle 中 UI、healthbox、party status summary の変更。
- move menu、target selection、bag、party switch menu から viewer を開く対応。
- 相手 moves / ability / held item の公開。
- runtime option menu。
- Champions challenge 専用 roster / facility runtime。

## Docs

- [Investigation](investigation.md)
- [MVP Plan](mvp_plan.md)
- [Implementation](implementation.md)
- [Dependencies](dependencies.md)
- [Risks](risks.md)
- [Test Plan](test_plan.md)
- [Phase 2 Integrated Selection Flow Checklist](phase2_selection_flow_checklist.md)
- [Legacy Console Battle UI References](legacy_console_references.md)
- [Pre-Battle Team Viewer Manual](../../manuals/prebattle_team_viewer_manual.md)
- [Battle Selection](../battle_selection/README.md)
- [Opponent Party and Randomizer](../battle_selection/opponent_party_and_randomizer.md)

## Open Questions

- `B_POOL_SETTING_CONSISTENT_RNG == FALSE` の trainer pool を、preview 時点で固定する仕様にするか。現在の実装契約では encounter start 前後で cached party を固定して battle に渡す。
- 相手側に type icons / gender / item badges を出すための graphics / palette budget をどこまで確保するか。
- opponent party cache を今後も `struct Pokemon` で持つか、species / level / gender / type などの lightweight view model へ分けるか。
- in-battle viewer の default button を `R_BUTTON` のままにするか、後で option 化するか。
