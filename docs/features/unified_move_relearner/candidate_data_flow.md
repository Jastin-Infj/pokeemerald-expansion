# Unified Move Relearner Candidate Data Flow

## Purpose

この文書は、Unified Move Relearner の候補技がどこから来て、build 時に
どう変換され、runtime でどう表示されるかを追うための workflow memo である。

今回の実装で新しく外部から learnset data を持ってきたわけではない。
既に repo にあった `tools/learnset_helpers/porymoves_files/*.json` を再利用し、
既存の `all_learnables.json` とは別に、source label 付きの relearner 用 table を
build 時に生成する。配布 / XD / Ranger / form-specific 系の special move は
`tools/learnset_helpers/special_relearner_moves.json` に project-owned runtime
data として持ち、porymoves regeneration とは別に保つ。

## Existing Source Data

| Path | Role |
|---|---|
| `tools/learnset_helpers/porymoves_files/*.json` | 既存の世代 / version 別 move data。各 species に `LevelMoves`, `EggMoves`, `TMMoves`, `TutorMoves` が入っている。 |
| `tools/learnset_helpers/special_relearner_moves.json` | 配布、XD purification、Ranger transfer、form-specific move など、通常 learnset に残らない special move 候補。 |
| `src/data/pokemon/all_learnables.json` | 既存 helper が porymoves JSON を全 version union して作る平坦な move set。source / generation metadata は残らない。 |
| `tools/learnset_helpers/build/all_teaching_types.json` | repo の `species_info` から、teachable learnset を持つ species 名と teaching type を抽出した build intermediate。 |
| `src/data/pokemon/teachable_learnsets.h` | 既存 helper が physical TM/HM registry、script tutor、universal moves から作る current teachable table。 |
| `src/data/tutor_moves.h` | 既存 helper が script tutor と extra tutor から作る current tutor list。 |

`all_learnables.json` は 90k 行前後の大きい JSON だが、既存の teachable
learnset 生成用に前からあるもの。これは「この species が過去作を含めて
この move を何らかの方法で覚えたことがある」ための union であり、
`TM` なのか `Tutor` なのか、どの generation なのかは失われる。

今回の Unified Relearner では、同じ move を `TM` と `Tutor` の両方に出す必要がある。
そのため、`all_learnables.json` だけを使うと source duplicate を保持できない。

## Existing Helper Flow

既存の helper は次の流れで動く。

```text
tools/learnset_helpers/porymoves_files/*.json
  -> make_learnables.py
  -> src/data/pokemon/all_learnables.json

src/data/pokemon/species_info/*_families.h
  -> make_teaching_types.py
  -> tools/learnset_helpers/build/all_teaching_types.json

data/**/*.inc + special_movesets.json
  -> make_tutors.py / make_teachables.py
  -> src/data/tutor_moves.h

all_learnables.json
include/constants/tms_hms.h
all_tutors.json
all_teaching_types.json
special_movesets.json
  -> make_teachables.py
  -> src/data/pokemon/teachable_learnsets.h
```

ここで作られる `teachable_learnsets.h` は current physical TM/HM と current
script tutor を中心にした compatibility table なので、Gen 1-9 の historical TM/TR
を item なしで広く出す今回の relearner pool とは役割が違う。

## New Unified Relearner Flow

今回追加した flow は既存 flow に横付けしている。

```text
tools/learnset_helpers/porymoves_files/*.json
tools/learnset_helpers/special_relearner_moves.json
tools/learnset_helpers/build/all_teaching_types.json
  -> make_relearner_learnsets.py
  -> src/data/pokemon/unified_relearner_learnsets.h
  -> included by src/move_relearner.c
```

`src/data/pokemon/unified_relearner_learnsets.h` は generated header で、commit しない。
`.gitignore` に入れてあり、build 時に再生成される。

Generator のルール:

- porymoves JSON は version order (`rgb`, `y`, `gs`, ..., `sv`, `za`) で読む。
- `EggMoves`, `TMMoves`, `TutorMoves` だけを relearner source table にする。
- special JSON は `species + moves` だけを runtime table に使い、source refs や
  region / distribution metadata は audit 用に保持する。
- `LevelMoves` は generated historical table には入れない。runtime の current
  level-up learnset を `MAX_LEVEL` まで見る。
- 同じ source 内の同じ move は重複排除する。
- `TM` と `Tutor` のように source が違う場合は、同じ move でも別候補として残す。
- repo の species 名は `all_teaching_types.json` に出てくるものをまず基準にする。
- `all_teaching_types.json` に出ない form / supplemental species でも、porymoves
  または special JSON に候補があり、`include/constants/species.h` の
  `SPECIES_*` constant に解決でき、かつ同じ numeric species slot がまだ emitted
  されていなければ runtime table に出す。
- `SPECIES_HOOPA` のような alias constant は、resolved numeric slot が既に
  `SPECIES_HOOPA_CONFINED` などで emitted 済みなら重複 emit しない。

## Runtime Candidate Build

runtime では `src/move_relearner.c` の unified builder が候補を組み立てる。

```text
Move Relearner entry
  -> gMoveRelearnerState = MOVE_RELEARNER_UNIFIED
  -> GetUnifiedRelearnerMoves(mon)
     -> Level: current compiled level-up learnset, all levels up to MAX_LEVEL
     -> Egg: current egg table + generated historical EggMoves
     -> TM: generated historical TMMoves
     -> Tutor: generated historical TutorMoves + current gTutorMoves compatibility fallback
     -> Special: generated special event / XD / Ranger-transfer / form-specific candidates
     -> hide moves the Pokemon already knows
     -> de-dupe only within the same source
     -> preserve cross-source duplicates
     -> cap at MAX_RELEARNER_MOVES
  -> list rows use candidate index as menu id
  -> row name is move name; right label is Lv / Eg / TM / Tu / Sp
```

重要な分担:

- Level source は generated porymoves `LevelMoves` ではなく、repo の current
  compiled level-up data を使う。
- TM source は physical TM item を増やさず、historical virtual candidate として出す。
- Tutor source は historical tutor と current script tutor の両方を見る。
- Special source は配布、XD purification、Ranger transfer、form-specific move などを
  まとめて扱う。
  MVP 表示は `Sp` の共通 label で、個別の `EV` / `XD` / `RG` badge や unlock
  group は runtime gating の follow-up に残している。

## Coverage Snapshot

2026-05-16 時点の local build intermediate では次の通り。

| Check | Count / Result |
|---|---|
| tracked `porymoves_files/*.json` | 22 files |
| repo teaching species in `all_teaching_types.json` | 1100 |
| porymoves union species keys | 1110 |
| repo teaching species matched in porymoves | 1097 |
| repo teaching species missing from porymoves | Cosmog, Ditto, Smeargle |
| repo teaching species with porymoves Egg/TM/Tutor entries | 1097 |
| repo species without porymoves Egg/TM/Tutor entries | Cosmog, Ditto, Smeargle |
| supplemental species emitted from porymoves / special JSON | 13 species: Pikachu Rock Star, Pikachu Belle, Pikachu Pop Star, Pikachu PhD, Pikachu Libre, Rotom Heat/Wash/Frost/Fan/Mow, Kyurem White/Black, Oinkologne F |
| special runtime candidate blocks | 174 blocks / 216 moves |

この数字は「全 `SPECIES_*` enum を網羅」という意味ではない。
`make_teaching_types.py` が repo の `species_info` から teachable learnset を持つ species
を拾い、それを generated table の基準にしている。

したがって、今の実装は「現在の repo が teachable table 対象にしている 1100 species」
を主軸にしつつ、Rotom form や Kyurem form のように `all_teaching_types.json`
から落ちるが porymoves / special data に存在する numeric species slot も追加 emit
する。Cosplay Pikachu は porymoves 側に form key がないため、現状は
special JSON の form-specific move だけを supplemental emit する。

## Why This Is Separate From `all_learnables.json`

`all_learnables.json` を直接使わなかった理由:

- source label (`TM`, `Tutor`, `Egg`) が分からない。
- `TM` と `Tutor` の duplicate rows を残せない。
- Gen 1 OK / Gen 2 OK のような allow-list を後から足すときに generation metadata がない。
- LevelMoves まで平坦化されるが、Unified Relearner の level source は repo の current
  compiled learnset を使う方が安全。

つまり `all_learnables.json` は既存の compatibility helper には合っているが、
source-aware relearner UI には情報が足りない。

## Known Follow-Ups

- Gen 1-9 allow-list config を runtime で本当に使うなら、generated table に version /
  generation metadata を残す必要がある。
- form species が base species の historical TM / tutor pool も継承するべきかを決める。
  現状、porymoves に form key がある Rotom / Kyurem などは form-specific pool を
  emit し、porymoves に form key がない Cosplay Pikachu は special-only pool になる。
- Special source の個別 unlock group (`special_event_movie`, `special_xd`,
  `special_ranger` など) を story flag / rank gating に接続する。
- `Sp` の共通 label を、候補が増えた段階で `EV` / `XD` / `RG` などの
  per-entry label に分けるか決める。
- 600+ candidate target に備えて、source tab / search / chunk UX を追加検討する。
