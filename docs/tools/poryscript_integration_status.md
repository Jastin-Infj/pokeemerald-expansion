# Poryscript Integration Status

## Purpose

外部 tool **Poryscript** ([huderlem/poryscript](https://github.com/huderlem/poryscript)) を本 fork に導入する/しない判断のため、現状 (= 未採用) の実態と、もし採用する場合の影響範囲・既存 hand-authored `.inc` との共存条件を整理する。

本書は **upstream tool への参照ドキュメント** であり、**fork して持ち込むことは想定しない**。導入の場合も release tag / binary を pin して使い、本 repo には commit しない方針。

導入する場合は `.pory` source、生成される `.inc`、Makefile / CI rule、`command_config.json` / `font_config.json` の扱いが同時に変わるため、通常の script 修正とは分けて **Poryscript 導入専用の feature branch** で開始する。

調査日: 2026-05-03。本 fork での source 改造はしておらず、`docs/` への記録のみ。

## Related Docs

| Doc | Why relevant |
|---|---|
| [overview/inc_script_pipeline_v15.md](../overview/inc_script_pipeline_v15.md) | 本書が論じる「採用しても変わらない pipeline」の本体。Poryscript は出力 `.inc` を変えないため、ここで説明された build 経路はそのまま活きる。 |
| [flows/map_script_flow_v15.md](../flows/map_script_flow_v15.md) | `MAP_SCRIPT_*` table dispatch。Poryscript の `mapscripts { }` block で表現する場合の対応関係を理解するため。 |
| [flows/map_script_flag_var_flow_v15.md](../flows/map_script_flag_var_flow_v15.md) | flag/var/visibility 編集チェックリスト。Poryscript 移行後も生きるルール集。 |
| [flows/script_inc_audit_v15.md](../flows/script_inc_audit_v15.md) | 既存 `.inc` の inventory。どの `.inc` から先に migrate するかの優先順位判断材料。 |

## Upstream Reference (do not vendor)

| Item | Value |
|---|---|
| Repository | [github.com/huderlem/poryscript](https://github.com/huderlem/poryscript) |
| README | [README.md @ master](https://github.com/huderlem/poryscript/blob/master/README.md) |
| Latest release | **v3.6.0** (2025-02-17) — multi-line string support, font_config text-replacement の追加 |
| Releases page | [github.com/huderlem/poryscript/releases](https://github.com/huderlem/poryscript/releases) |
| Author blog | [huderlem.com/blog/posts/poryscript](https://www.huderlem.com/blog/posts/poryscript/) |
| Online playground | [github.com/huderlem/poryscript-playground](https://github.com/huderlem/poryscript-playground) |
| Language | Go (binary distribution) |
| License | (要確認 — repository 直参照) |

**取り扱い方針 (本 fork ローカルルール):**

- Source code を本 repo に fork / vendor / submodule として持ち込まない。
- 採用する場合は GitHub release tag (例: `v3.6.0`) で pin した CLI binary を `tools/poryscript/` に配置するのみ (これ自体も commit せず、`Makefile` から download or 手動配置)。
- 設定ファイル (`font_config.json` / `command_config.json`) のみ本 repo に置きうる。
- 本書中の URL 記述は「参照」が目的であり、自動 mirror や snapshot 取得は行わない。

## Current State in This Fork (2026-05-03)

事実関係 — すべて grep / find で確認:

| Indicator | Result |
|---|---|
| `*.pory` files in repo | **0 件** (`find . -name '*.pory' → 空`) |
| `tools/poryscript/` directory | **存在せず** |
| `Makefile` / `*.mk` references to `poryscript` or `.pory` | **0 件** (`grep "poryscript\|\.pory" Makefile *.mk → 0 matches`) |
| `font_config.json` / `command_config.json` files | **存在せず** |
| Hand-authored `.inc` script files | 945 個の `data/maps/<MapName>/scripts.inc` 候補 + `data/scripts/*.inc` + `data/text/*.inc` |

**結論: 本 fork は Poryscript 未採用、event script はすべて raw `.inc` の手書き。**

これは upstream `rh-hideout/pokeemerald-expansion` に追従した状態であり、本 fork 固有の選択ではない (upstream pret の `pokeemerald` も同様に raw `.inc`)。

## What Poryscript Provides

upstream README より (本 fork が将来評価する観点で要約):

### Input/Output

- **Input**: `.pory` ファイル (high-level scripting language)
- **Output**: `.inc` ファイル (本 fork が現在書いているのと同じ event-script bytecode declaration)

つまり Poryscript は **bytecode runtime や macro layer を変えない**。`asm/macros/event.inc` の macro 群と `gScriptCmdTable` を一切触らずに、`.inc` を **書きやすい形で生成する** だけの transpiler。

### High-level constructs supported

- `if` / `elif` / `else`
- `while` / `do...while`
- `switch`
- `break` / `continue`
- `goto` (with labels)
- 複合 boolean expression: `&&` / `||`
- `script foo { ... }` / `text bar { ... }` / `movement baz { ... }` block 構造
- 複数 string `format()` / 複数行 string (v3.6.0)

これらは内部的に `compare` + `goto_if` + `call_if` の生 bytecode に展開される。出力 `.inc` は人間が書いた `.inc` と区別がつかない。

### Command line

```
./poryscript -i input.pory -o output.inc -fc font_config.json -cc command_config.json
```

主要 option:

- `-i` input
- `-o` output
- `-fc` font config (text width 計算 / 改行用)
- `-cc` command config (event-script command の知識を transpiler に教える)
- `-lm` line markers (debug 用)
- `-optimize` 最適化 pass

## Hypothetical Integration Plan (not yet executed)

**未実施。** 採用判断後、次の手順を踏む想定:

### 1. Tool 配置

```
tools/poryscript/
├── poryscript            # release v3.6.0 binary (gitignore)
├── font_config.json      # repo 内に置く
└── command_config.json   # repo 内に置く
```

`tools/poryscript/poryscript` 自体は build 環境ごとに調達する (CI / dev で別 binary)。Vendor しない。

### 2. Makefile 追加 (upstream README の例より)

```makefile
SCRIPT    := $(TOOLS_DIR)/poryscript/poryscript$(EXE)

AUTO_GEN_TARGETS += $(patsubst %.pory,%.inc,$(shell find data/ -type f -name '*.pory'))

%.pory: ;

data/%.inc: data/%.pory
	$(SCRIPT) -i $< -o $@ -fc tools/poryscript/font_config.json -cc tools/poryscript/command_config.json
```

### 3. 段階的 migration (upstream の慣行)

upstream の conversion script は **「`scripts.pory` がある directory は skip する」** 設計。すなわち:

- 既存 `scripts.inc` はそのまま残せる。
- ある map を migrate するときだけ、その map の `scripts.inc` を `scripts.pory` に置き換える。
- Build 後は `scripts.inc` が `.pory` から再生成される (= 元の手書き `.inc` は消える)。

これにより 945 map 全部を一括 migrate せず、**1 map ずつ opt-in** できる。

### 4. CI / build への影響

- `AUTO_GEN_TARGETS` に追加されるので、`make` の `SETUP_PREREQS` 段階で `.pory → .inc` 変換が実行される。`poryscript` binary が無いと build が落ちるため、`tools/poryscript/poryscript` の存在チェックを `make_tools.mk` 相当に追加する必要あり。
- CI cache の `tools/` cache key に `poryscript` を含めないと、CI ごとに毎回 rebuild される。

## Compatibility Matrix: poryscript vs hand-authored `.inc`

採用後の運用イメージ:

| Asset | 書く場所 | 補足 |
|---|---|---|
| **会話 NPC** の event script body | `.pory` 推奨 | `if/switch` / 複数分岐に強い。 |
| **Coord event** / **bg event** trigger script body | `.pory` 推奨 | 同上。 |
| **Movement command** sequence (`<Map>_Movement_Foo:`) | どちらでも | poryscript の `movement Foo { ... }` block でも、raw `.inc` でも書ける。 |
| **Text label** | どちらでも | poryscript の `text Foo { format("...") }` の方が改行管理が楽。 |
| **`<Map>_MapScripts::` table** (`map_script` data declaration) | `.pory` 推奨 / raw 併用可 | Poryscript 公式 README は `mapscripts` block を持ち、`MAP_SCRIPT_ON_FRAME_TABLE` のような table 型も `[]` syntax で表現できる。未対応 pattern や検証前の rare type は `raw` block または raw `.inc` に残す。 |
| **`<Map>_OnTransition:` 等の script body** | `.pory` 可 | bytecode label として通常通り出力されるため。 |
| **`object_event` / `warp_def` / `coord_event` / `bg_event` の data emission** | **raw `.inc` (mapjson 生成)** | `events.inc` は `mapjson` の管轄。poryscript の対象外。 |
| **`connection` / `map_header_flags`** | **raw `.inc` (mapjson 生成)** | 同上。 |

→ poryscript 採用後も以下は変わらず生 `.inc` 経路:

- `mapjson` 生成の `header.inc` / `events.inc` / `connections.inc` / `layouts.inc` / `headers.inc` / `groups.inc`
- `data/script_cmd_table.inc` (bytecode opcode table の single source)
- `asm/macros/*.inc` の macro 定義群
- `data/event_scripts.s` の集約 hub
- battle / contest / animation の別 bytecode 系統 (`battle_scripts_1.s` 等)

つまり poryscript は「`scripts.inc` の event script / movement / text / mart / map script table を書く部分」を高級言語化する。pipeline 全体や bytecode runtime は unchanged。

## Map-Script Implications

[map_script_flow_v15.md](../flows/map_script_flow_v15.md) で詳述した `MAP_SCRIPT_*` の意味論は poryscript 導入で **変わらない**:

- `MAP_SCRIPT_ON_LOAD` / `ON_TRANSITION` / `ON_RESUME` / `ON_RETURN_TO_FIELD` / `ON_DIVE_WARP` の 5 種類は単一 script ptr。poryscript の `script Foo { ... }` block を書けば、その label を `map_script` table から指せばよい (table 自身は raw か poryscript の `mapscripts` block で書く)。
- `MAP_SCRIPT_ON_FRAME_TABLE` / `ON_WARP_INTO_MAP_TABLE` の table 形式は `map_script_2 var, compare, script` という data declaration。poryscript の `mapscripts` block で対応する syntax を使うのが自然。
- 走査関数 `MapHeaderGetScriptTable` / `MapHeaderCheckScriptTable` は変更なし。bytecode 互換性はそのまま。

2026-05-03 の追加確認:

- 本 fork では `map_script` entry が 890 件、`map_script_2` entry が 339 件ある。
- `MAP_SCRIPT_ON_DIVE_WARP` は `data/maps/Underwater_SealedChamber/scripts.inc` の 1 件だけ。
- `MAP_SCRIPT_ON_RETURN_TO_FIELD` は 6 件だけ。
- `data/scripts/shared_secret_base.inc`、`data/scripts/battle_pike.inc`、`data/scripts/cable_club.inc`、`data/scripts/cable_club_frlg.inc`、`data/scripts/trainer_hill.inc`、`data/scripts/cave_hole.inc`、`data/scripts/trainer_tower.inc` など、map-local ではない shared `.inc` にも `map_script` / `map_script_2` がある。

したがって migration は「全 map 一括」ではなく、まず `scripts.pory` を持つ map だけを opt-in し、shared `.inc` と rare map script type は raw 併用で残す方針が安全。

つまり「Porymap で表現できない conditional logic」という境界線は poryscript 採用前後で同じ:

| Tool | 表現範囲 |
|---|---|
| Porymap (`map.json`) | Layout、event placement、connections、header field |
| `mapjson` (build tool) | 上記から `events.inc` / `header.inc` / `connections.inc` 等の data table を生成 |
| Hand-authored `.inc` | 上記以外の **すべて** (script body、map_script table、movement、text) |
| Poryscript (将来) | hand-authored `.inc` の **書きやすさを向上** させるだけ。生成物は `.inc` |

## Migration Guardrails

Poryscript 導入 branch の最初の task は tool 導入と最小変換に限定する。

推奨順序:

1. `tools/poryscript/` 配下に config と binary 調達 rule を追加する。
2. `data/%.pory -> data/%.inc` rule を追加し、手元と CI で no-op build を確認する。
3. 既存 `.inc` をまだ変換せず、空または小さい sample `.pory` で生成経路だけ確認する。
4. `MAP_SCRIPT_ON_DIVE_WARP` / `MAP_SCRIPT_ON_RETURN_TO_FIELD` / `MAP_SCRIPT_ON_FRAME_TABLE` / `MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE` の 4 系統をそれぞれ小さい map で検証する。
5. shared `.inc` は後回しにし、raw block で逃がせる pattern と逃がせない pattern を切り分ける。

生成物管理の未決定事項:

- `.pory` を source of truth にして generated `.inc` を gitignore するか。
- generated `.inc` も commit して、非 Poryscript 環境でも build できるようにするか。
- `data/event_scripts.s` の `.include` は既存 `scripts.inc` を指し続けるため、新規 `.pory` 追加時に `.include` 更新が必要か。
- `command_config.json` は expansion 独自 command / macro drift に追従する必要がある。

## Risk / Considerations

- **Build 速度**: 945 map 各々で `.pory → .inc` 変換が走ると、`make -j` でも初回 build 時間が伸びる。incremental build なら問題は小さいはずだが、要計測。
- **Diff の質**: `.pory` を編集すると `.inc` が再生成される。`.inc` 側を git に含めるか、`.gitignore` するかの選択がある。upstream は通常 `.inc` を gitignore する設計だが、本 fork は upstream `pokeemerald-expansion` を pull するため、**hand-authored `.inc` と generated `.inc` の混在期に注意**。
- **Upstream 追従**: 本 fork は upstream を pull する運用 ([branching_upgrade_policy.md](../upgrades/branching_upgrade_policy.md))。upstream が将来 poryscript を採用すると、その時点で本 fork の「未採用」状態は upstream とずれる。逆に本 fork が先行採用すると merge 時に conflict 源となる。**upstream 採用を待つのが安全側。**
- **Tool binary の供給**: `poryscript` は Go 製。`go install` か release binary download か、CI と dev 環境で揃える必要あり。Cross-platform binary なので Windows/macOS/Linux は提供されている (release page)。
- **Configuration drift**: `font_config.json` / `command_config.json` は本 fork 固有の expansion command (e.g., `dynmultichoice`、custom `callnative`) を反映する必要があり、event command が増えるたびに維持コストが発生する。

## Confirmed

- 本 fork に `.pory` ファイル / `tools/poryscript/` / Makefile rule は存在しない (= **未採用**)。
- upstream Poryscript の最新 release は v3.6.0 (2025-02-17)。
- Poryscript は **transpiler** であり、event-script bytecode runtime / macro layer / opcode table を変更しない。
- 採用後も `mapjson` 生成 `.inc` (header/events/connections/layouts/groups) と battle/animation 系の bytecode declaration は raw `.inc` のまま。
- 採用は per-map opt-in 可能 (`scripts.pory` がある map のみ変換、他は raw `.inc` のまま)。
- Poryscript 公式 README には `mapscripts` block があり、map script table 全体を常に raw `.inc` に固定する必要はない。ただし本 fork 固有 pattern は導入 branch で検証する。

## Risk

- 本 fork で先行採用すると upstream merge で conflict が増える。
- `.pory ⇄ .inc` の git 管理戦略を決めずに採用すると、generated `.inc` が PR diff を汚す。
- `command_config.json` の維持を怠ると、新規 event command (本 fork 拡張) が poryscript で `unknown command` 扱いになる。

## Open Questions

- 本 fork の採用判断 (timing / 範囲) は未決定。ユーザ側で「導入時期は決めている」とのことなので、その timing をここに追記する想定。
- `mapscripts { ... }` block の poryscript 側 syntax で、本 fork が必要とする `MAP_SCRIPT_*` 7 種すべて (特に `ON_RETURN_TO_FIELD` / `ON_DIVE_WARP`) を漏れなく表現できるかは未検証。導入 branch で小さい map ごとに要確認。
- 採用時に `data/event_scripts.s` の 1002 個の `.include` directive を自動更新する仕組みが必要か (新規 `.pory` を 増やしたら対応する `.include "data/.../scripts.inc"` 行も追加が要るが、現状 hand-authored)。
