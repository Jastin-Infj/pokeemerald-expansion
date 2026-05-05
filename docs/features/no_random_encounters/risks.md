# No Random Encounters Risks

## Risk Matrix

| Risk | Severity | Affected area | Mitigation |
|---|---|---|---|
| story static encounter を止める | High | `setwildbattle`, `dowildbattle`, legendary / event scripts | MVP は standard step encounter だけ止める。static battle は対象外。 |
| Fishing / Sweet Scent / Rock Smash の仕様が曖昧 | Medium | `FishingWildEncounter`, `SweetScentWildEncounter`, `RockSmashWildEncounter` | `step-only` と `broad-wild` を mode として分ける。 |
| config に `TRUE` / `1` を入れて flag id と混同する | High | `include/config/overworld.h`, flags | `OW_FLAG_NO_ENCOUNTER` は flag id config と明記し、未使用 flag を割り当てる。 |
| flag を clear し忘れる | Medium | facility end, debug, scripts | start/end helper または script macro を作り、challenge 終了時に必ず clear する。 |
| Battle Pike / Pyramid の wild room を壊す | Medium | `StandardWildEncounter`, Frontier scripts | MVP では no encounter flag が有効なら歩行 encounter は止まる。facility 中に使うかどうかを rule で分ける。 |
| DexNav / detector とズレる | Medium | DexNav, hidden encounter | MVP では DexNav を対象外にし、必要なら DexNav 側に別 validation を追加する。 |
| option menu 追加で save layout を壊す | Medium | `SaveBlock2`, option menu | 初期は debug / script / flag で運用し、option UI は runtime rule state 方針後に接続する。 |

## Policy

no encounter は「野生 encounter table を消す」機能ではない。table はそのまま残し、battle 開始 gate を止める。

初期実装では、歩行 encounter だけを止める。Fishing、Sweet Scent、Rock Smash は player が能動的に起こす encounter なので、止めるなら別 mode として扱う。

static `dowildbattle` を止める mode は原則作らない。必要な場合でも story/event map を個別に audit してからにする。
