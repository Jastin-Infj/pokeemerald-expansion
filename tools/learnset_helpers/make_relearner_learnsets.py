#!/usr/bin/env python3

"""
Usage: python3 make_relearner_learnsets.py INPUTS_DIR BUILD_DIR OUTPUT_FILE

Build source-tagged historical move relearner tables from the porymoves JSON
files. These tables intentionally do not create physical TM items.
"""

import json
import pathlib
import re
import sys
from collections import defaultdict


SNAKIFY_PAT = re.compile(r"(?<!^)(?=[A-Z])")
NORMALIZE_PAT = re.compile(r"[^A-Z0-9]+")
SPECIES_DEFINE_PAT = re.compile(r"#define\s+SPECIES_([A-Z0-9_]+)\s+([A-Z0-9_]+|SPECIES_[A-Z0-9_]+)")

PORYMOVES_ORDER = [
    "rgb",
    "y",
    "gs",
    "c",
    "rse",
    "frlg",
    "xd",
    "dp",
    "pt",
    "hgss",
    "bw",
    "b2w2",
    "xy",
    "oras",
    "sm",
    "usum",
    "lgpe",
    "swsh",
    "bdsp",
    "la",
    "sv",
    "za",
]

SPECIAL_MOVES_FILE = "special_relearner_moves.json"


def species_to_upper(name: str) -> str:
    return SNAKIFY_PAT.sub("_", name).upper()


def upper_to_pascal(name: str) -> str:
    return "".join(part.title() for part in name.split("_"))


def normalize_pory_species(name: str) -> str:
    name = name.replace("Â€™", "").replace("’", "").replace("'", "")
    normalized = NORMALIZE_PAT.sub("_", name.upper()).strip("_")
    normalized = normalized.replace("FARFETCH_D", "FARFETCHD")
    return normalized


def normalize_species_constant(name: str) -> str:
    normalized = name.strip().upper()
    if normalized.startswith("SPECIES_"):
        normalized = normalized[len("SPECIES_"):]
    return normalized


def normalize_move(name: str) -> str:
    return name.replace("Â€™", "").replace("’", "").replace("'", "")


def ordered_input_files(inputs_dir: pathlib.Path) -> list[pathlib.Path]:
    by_stem = {path.stem: path for path in inputs_dir.glob("*.json")}
    ordered = [by_stem.pop(stem) for stem in PORYMOVES_ORDER if stem in by_stem]
    ordered.extend(path for _, path in sorted(by_stem.items()))
    return ordered


def append_unique(dst: list[str], seen: set[str], move: str) -> None:
    if move not in seen:
        dst.append(move)
        seen.add(move)


def collect_source_moves(inputs_dir: pathlib.Path) -> dict[str, dict[str, list[str]]]:
    moves = defaultdict(lambda: {"EggMoves": [], "TMMoves": [], "TutorMoves": []})
    seen = defaultdict(lambda: {"EggMoves": set(), "TMMoves": set(), "TutorMoves": set()})

    for path in ordered_input_files(inputs_dir):
        with open(path, "r", encoding="utf-8") as fp:
            data = json.load(fp)
        for species, by_method in data.items():
            species_key = normalize_pory_species(species)
            for method in ("EggMoves", "TMMoves", "TutorMoves"):
                for move in by_method.get(method, []):
                    append_unique(moves[species_key][method], seen[species_key][method], normalize_move(move))

    return moves


def collect_special_moves(special_moves_file: pathlib.Path) -> dict[str, list[str]]:
    moves = defaultdict(list)
    seen = defaultdict(set)

    if not special_moves_file.is_file():
        return moves

    with open(special_moves_file, "r", encoding="utf-8") as fp:
        data = json.load(fp)

    for candidate in data.get("candidates", []):
        species_key = normalize_species_constant(candidate["species"])
        for move in candidate.get("moves", []):
            append_unique(moves[species_key], seen[species_key], normalize_move(move))

    return moves


def collect_species_values(repo_root: pathlib.Path) -> dict[str, int]:
    constants_file = repo_root / "include" / "constants" / "species.h"
    raw_values = {}

    with open(constants_file, "r", encoding="utf-8") as fp:
        for line in fp:
            match = SPECIES_DEFINE_PAT.match(line)
            if match is None:
                continue

            name, value = match.groups()
            raw_values[name] = value[len("SPECIES_"):] if value.startswith("SPECIES_") else int(value)

    def resolve(name: str) -> int | None:
        seen = set()
        while name not in seen:
            seen.add(name)
            value = raw_values.get(name)
            if isinstance(value, int):
                return value
            if isinstance(value, str):
                name = value
                continue
            return None
        return None

    return {
        name: value
        for name in raw_values
        if (value := resolve(name)) is not None
    }


def write_move_array(lines: list[str], name: str, source: str, moves: list[str]) -> str:
    if not moves:
        return "sNoneUnifiedRelearnerMoves"

    symbol = f"s{name}UnifiedRelearner{source}Moves"
    lines.append(f"static const u16 {symbol}[] = {{")
    for move in moves:
        lines.append(f"    {move},")
    lines.append("    MOVE_UNAVAILABLE,")
    lines.append("};")
    return symbol


def main() -> None:
    if len(sys.argv) != 4:
        print("Invalid number of arguments", file=sys.stderr)
        print(__doc__, file=sys.stderr)
        raise SystemExit(1)

    inputs_dir = pathlib.Path(sys.argv[1])
    build_dir = pathlib.Path(sys.argv[2])
    output_file = pathlib.Path(sys.argv[3])
    teaching_types_file = build_dir / "all_teaching_types.json"

    assert inputs_dir.is_dir(), f"{inputs_dir=} is not a directory"
    assert teaching_types_file.is_file(), f"{teaching_types_file=} is not a file"
    assert output_file.parent.is_dir(), f"parent of {output_file=} is not a directory"

    source_moves = collect_source_moves(inputs_dir)
    special_moves = collect_special_moves(inputs_dir.parent / SPECIAL_MOVES_FILE)
    species_values = collect_species_values(pathlib.Path(__file__).resolve().parents[2])

    with open(teaching_types_file, "r", encoding="utf-8") as fp:
        repo_species_data = json.load(fp)

    array_lines = [
        "//",
        "// DO NOT MODIFY THIS FILE! It is auto-generated by tools/learnset_helpers/make_relearner_learnsets.py",
        "//",
        "",
        "static const u16 sNoneUnifiedRelearnerMoves[] = {",
        "    MOVE_UNAVAILABLE,",
        "};",
        "",
    ]
    entries = [
        "static const struct UnifiedRelearnerLearnset gUnifiedMoveRelearnerLearnsets[NUM_SPECIES] = {",
        "    [SPECIES_NONE] = {",
        "        .eggMoves = sNoneUnifiedRelearnerMoves,",
        "        .tmMoves = sNoneUnifiedRelearnerMoves,",
        "        .tutorMoves = sNoneUnifiedRelearnerMoves,",
        "        .specialMoves = sNoneUnifiedRelearnerMoves,",
        "    },",
    ]
    emitted_species_values = {species_values.get("NONE", 0)}

    for species_data in repo_species_data:
        if isinstance(species_data, str):
            array_lines.append(species_data.rstrip("\n"))
            entries.append(species_data.rstrip("\n"))
            continue

        name = species_data["name"]
        species_upper = species_to_upper(name)
        if species_upper in species_values:
            emitted_species_values.add(species_values[species_upper])

        species_moves = source_moves.get(species_upper, {})
        egg_symbol = write_move_array(array_lines, name, "Egg", species_moves.get("EggMoves", []))
        tm_symbol = write_move_array(array_lines, name, "Tm", species_moves.get("TMMoves", []))
        tutor_symbol = write_move_array(array_lines, name, "Tutor", species_moves.get("TutorMoves", []))
        special_symbol = write_move_array(array_lines, name, "Special", special_moves.get(species_upper, []))

        entries.extend([
            f"    [SPECIES_{species_upper}] = {{",
            f"        .eggMoves = {egg_symbol},",
            f"        .tmMoves = {tm_symbol},",
            f"        .tutorMoves = {tutor_symbol},",
            f"        .specialMoves = {special_symbol},",
            "    },",
        ])

    supplemental_species = []
    for species_upper in sorted(set(source_moves) | set(special_moves), key=lambda key: species_values.get(key, 0)):
        species_value = species_values.get(species_upper)
        if species_value is None or species_value in emitted_species_values:
            continue
        supplemental_species.append(species_upper)
        emitted_species_values.add(species_value)

    for species_upper in supplemental_species:
        name = upper_to_pascal(species_upper)
        species_moves = source_moves.get(species_upper, {})
        egg_symbol = write_move_array(array_lines, name, "Egg", species_moves.get("EggMoves", []))
        tm_symbol = write_move_array(array_lines, name, "Tm", species_moves.get("TMMoves", []))
        tutor_symbol = write_move_array(array_lines, name, "Tutor", species_moves.get("TutorMoves", []))
        special_symbol = write_move_array(array_lines, name, "Special", special_moves.get(species_upper, []))

        entries.extend([
            f"    [SPECIES_{species_upper}] = {{",
            f"        .eggMoves = {egg_symbol},",
            f"        .tmMoves = {tm_symbol},",
            f"        .tutorMoves = {tutor_symbol},",
            f"        .specialMoves = {special_symbol},",
            "    },",
        ])

    entries.append("};")
    entries.append("")

    with open(output_file, "w", encoding="utf-8") as fp:
        fp.write("\n".join(array_lines + [""] + entries))


if __name__ == "__main__":
    main()
