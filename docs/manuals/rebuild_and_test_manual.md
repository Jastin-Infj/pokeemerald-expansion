# Rebuild and Test Manual

この文書は、feature branch で実装・生成・検証を進める時の入口。
具体的な test framework の使い方は `docs/tutorials/how_to_testing_system.md` を参照し、この manual では作業順と記録場所を決める。

## Basic Flow

1. `git status --short --branch` で branch と未コミット差分を確認する。
2. docs-only 作業か、source / data / tools を触る作業かを確認する。
3. feature docs の `test_plan.md` に、実行予定の build / test / manual check を書く。
4. 実装または data 生成を行う。
5. focused test を先に実行する。
6. 必要なら full rebuild を実行する。
7. 結果を feature docs の `test_plan.md` に追記する。

runtime に影響する source change は、push 前に `make -j16` 系の build / test と mGBA 確認をセットで行う。確認できない場合は、未確認理由と残リスクを `test_plan.md` に残す。詳細は `docs/tools/mgba_live_runtime_validation.md`。

## Build Commands

この repository では、実際に使う command は branch / platform / local setup によって変わる。
実行前に既存 docs と Makefile を確認する。

| Purpose | Command candidate | Notes |
|---|---|---|
| 通常 build | `make -j16 -O all` | host に余裕がある local validation では `-j16` を標準にする。 |
| clean rebuild | `make clean` then `make` | generated data や object cache の影響を疑う時に使う。 |
| debug build | `make -j16 -O debug` | debug menu / Live mGBA で確認する branch では必須。 |
| test framework / mGBA headless | `make -j16 -O check` | `mgba-rom-test-hydra` 経由で mGBA headless を走らせる。 |
| generated data check | feature-specific CLI / script | partygen などは lint / doctor / diff を build 前に通す。 |

`make` の出力で save block や free space に関する情報が出る場合は、feature docs の `investigation.md` か `test_plan.md` に残す。
SaveBlock 変更、large generated table、new option を触る branch では特に記録する。

### Parallelism

2026-05-06 の確認では、20 logical workers の local environment で `make -j16` と `make debug -j16` が成功した。`make -j4` は保守的な fallback として残し、通常の local validation は host に余裕があるなら `-j16` を使ってよい。

resource-related failure が疑わしい場合は `-j16` と `-j4` の両方を記録し、CPU count、失敗した command、failure excerpt を test result に残す。

### Runtime Validation Set

runtime に影響する source / data / config 変更では、最終確認として次を揃える。

| Check | Command / Tool | Required when |
|---|---|---|
| Normal ROM build | `rtk make -j16 -O all` | docs-only 以外の source / data change |
| Debug ROM build | `rtk make -j16 -O debug` | battle / field / menu / debug route を確認する change |
| mGBA headless tests | `rtk make -j16 -O check` or focused `TESTS=...` | battle / Pokemon / item / save logic を触る change |
| Focused mGBA Live check | mGBA Live CLI / MCP | 実画面、input、menu、field、battle end behavior が関係する change |

`make check` は mGBA headless の検証を含むが、実画面の確認ではない。
trainer battle end、party menu、held item icon、field return などの目視/操作が重要な変更は、
別途 mGBA Live で対象 behavior を確認する。

対象 behavior まで到達できない場合は、単なる未実行ではなく、必要な save / savestate、
debug route、未確認の残リスクを feature docs の `test_plan.md` に残す。

GitHub Actions の長時間 job は、agent 作業中に再度待ち続けない。
20-30 分程度かかることがあり、待機だけで作業が止まりやすい。
push 前の agent handoff では、local `rtk make -j16 -O all` /
`rtk make -j16 -O debug` / `rtk make -j16 -O check` または focused
`TESTS=...` と、mGBA Live CLI / MCP の実行結果を優先して記録する。
Actions を再待機しなかった場合は、その旨を PR / feature docs / final
summary のいずれかに明記する。

## When to Use Clean Rebuild

clean rebuild は時間がかかるため、毎回必須にはしない。
次の場合に使う。

- generated file を追加・削除した。
- constants / IDs / data table の count を変えた。
- include guard、config、Makefile、tool output を変えた。
- build は通るが runtime の表示や data が古い疑いがある。
- feature complete 前の最終確認を行う。

## Test Result Recording

feature docs の `test_plan.md` には、最低限次を残す。

| Field | Example |
|---|---|
| Date | `2026-05-05` |
| Command | `make` |
| Result | passed / failed / skipped |
| Notes | 失敗理由、未確認の runtime path、manual check の内容 |

失敗した場合は、実装ミスか設計ミスかを分ける。

| Failure type | Action |
|---|---|
| 実装ミス | code / data を修正し、同じ test を再実行する。 |
| 設計ミス | 先に `mvp_plan.md` / `risks.md` / `test_plan.md` を更新し、Planned 相当へ戻す。 |
| 環境問題 | command、platform、missing tool を記録し、再現条件を切り分ける。 |

## Manual Checks

UI、battle flow、field flow、save/load は自動 test だけで足りないことがある。
manual check を行う場合は、手順と期待結果を feature docs に残す。

| Area | Manual check |
|---|---|
| battle selection | trainer encounter、選出、battle start、battle end、party restore |
| opponent preview | preview と battle 実体が一致するか |
| no random encounters | grass / water / cave、Fishing / Sweet Scent / Rock Smash の対象外確認 |
| generated `.party` | lint、diff、trainerproc、build、runtime battle |
| option menu | new game default、save/load、既存 save compatibility |

runtime check は screenshot だけで終わらせない。対象 behavior に応じて、flag / var、callback、coords、battle state、screen のうち最低 2 種類を組み合わせる。

## Feature Complete Gate

feature complete にする前に、次を確認する。

- `make` または branch で必要な build が通っている。
- focused test / manual check の結果が `test_plan.md` に残っている。
- runtime に影響する変更は mGBA 確認済み、または skip 理由が残っている。
- 未実行 test がある場合、その理由が accepted risk / future work として残っている。
- generated data を使う feature は、入力、lint、出力、差し替え、戻し方が docs から辿れる。
- SaveBlock / ID / upstream merge に関係する file は upgrade checklist に載っている。
