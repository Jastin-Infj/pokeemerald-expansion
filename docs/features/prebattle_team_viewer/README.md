# Pre-Battle / In-Battle Team Viewer

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-10 |
| Baseline | `master` `7c19f56901`; `git describe` = `expansion/1.15.2-38-g7c19f56901` |
| Code status | MVP implemented; build/check and focused mGBA route passed |
| Provenance | Local project feature docs |

Status: MVP implemented; focused mGBA route validated through pre-battle viewer, selection,
battle action menu, in-battle viewer, and return to action menu.
Code status: source changes present in this branch

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

実装は current `master` から切った `feature/prebattle-team-viewer` 上で、必要な
battle selection source だけを reapply してから team viewer を追加した。
pre-battle viewer と in-battle viewer は同じ opponent cache を使う。

推奨 branch 形:

| Purpose | Branch |
|---|---|
| docs / planning | `feature/prebattle-team-viewer` |
| runtime implementation | `feature/prebattle-team-viewer` |
| dependency source | selected source files from `feature/battle-selection-mvp`; the whole old branch was not merged |

## MVP Shape

最初の実装 slice は、battle 前に専用 team preview screen を挟み、battle 中は
read-only viewer として同じ情報を開けるようにする。

1. trainer battle の `gBattleTypeFlags` が確定する。
2. 対象 battle なら opponent party を preview cache として確定する。
3. team viewer を表示する。
4. player が確認したら、既存 battle selection MVP の 3/4 匹選出へ進む。
5. 選出確定後、cached opponent party と selected player party で battle を開始する。
6. battle 中の player action menu で `R_BUTTON` を押すと read-only team viewer を開く。
7. battle 中の player action menu には `R / TEAM / INFO` の小さい 32x32 hint を
   表示し、viewer shortcut が存在することを見せる。
8. battle 中 viewer は display-only とし、D-pad / `SELECT` は無視する。
9. viewer を閉じると action menu に戻り、battle command はまだ未選択のままにする。

この slice では、選出そのものは既存 `choose half` UI を使ってよい。
同一画面で相手 team を見ながら直接 4 匹選ぶ Champions 風 UI は Phase 2 として扱う。

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
| center / footer | `A` confirm、`SELECT` strength view、single 3 / double 4 の required count。 |
| selection | MVP は viewer 後に既存 selection UI。selection 中の `B` は cached viewer へ戻る。Phase 2 は viewer と selection を統合。 |
| in-battle mode | display-only。`R` / `B` / `A` close。D-pad / `SELECT` は無視し、選択や party mutation はしない。 |

`SELECT` strength view は、選択中 Pokémon の詳細を開く。target は player side の
held item、ability、moves、nature / stat alignment、能力ポイント相当の配分表示。
実装済み MVP は space の都合で stat alignment / effort allocation 相当の表示は deferred とし、
player side では nickname/species、`LV.`、held item、type、ability、four moves を表示する。
opponent side は MVP では species / type / gender / level などの public preview に留める。
相手の moves、ability、held item、stat allocation は、公式 UI が明確に公開していると
確認できるまで表示しない。
`SELECT` で strength view に入った後は、D-pad で cursor を動かしても detail panel を
閉じず、選択中の Pokemon / side に合わせて footer detail を更新する。

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
- [Risks](risks.md)
- [Test Plan](test_plan.md)
- [Phase 2 Integrated Selection Flow Checklist](phase2_selection_flow_checklist.md)
- [Legacy Console Battle UI References](legacy_console_references.md)
- [Battle Selection](../battle_selection/README.md)
- [Opponent Party and Randomizer](../battle_selection/opponent_party_and_randomizer.md)

## Open Questions

- Phase 1 で「preview -> existing selection UI」の 2 画面構成を許容するか、初回から統合選出画面まで作るか。
- `B_POOL_SETTING_CONSISTENT_RNG == FALSE` の trainer pool を、preview 時点で固定する仕様にするか。
- 相手側に type icons / gender / item badges を出すための graphics / palette budget をどこまで確保するか。
- opponent party cache を `struct Pokemon` で持つか、species / level / gender / type などの lightweight view model で持つか。
- in-battle viewer の default button を `R_BUTTON` のままにするか、後で option 化するか。
