#!/usr/bin/env python3
"""
Validate and summarize trainer rank .party files, and optional trainer rank specs.

Mode: pre-processing (does not modify trainer.party). Generates a header with metadata
for later consumption by C code.

Checks:
- WeightTotal is present and equals mode (100 or 1000).
- Weight entries: numeric > 0 or NONE. NONE is excluded from the sum and from selection.
- Sum of numeric weights must equal WeightTotal.
- Rank/count pairing: NormalRank -> NormalCount required, RareRank -> RareCount required.
- AllowDuplicates must be true/false if present (default false).

Outputs:
- generated/trainer_rank_parties.h (default) with pool metadata and trainer rank specs.
"""

from __future__ import annotations

import argparse
import pathlib
import re
import sys
from dataclasses import dataclass
from typing import Dict, List, Optional, Tuple


ROOT = pathlib.Path(__file__).resolve().parent.parent
DEFAULT_DIR = ROOT / "data" / "trainer_rank_party"
DEFAULT_TRAINER_PARTY = ROOT / "src" / "data" / "trainers.party"
DEFAULT_SKIP = ROOT / "src" / "data" / "randomizer" / "trainer_skip_list.h"
DEFAULT_OUT = ROOT / "generated" / "trainer_rank_parties.h"
DEFAULT_SPECIES = ROOT / "include" / "constants" / "species.h"
DEFAULT_TRAINER_CONST = ROOT / "include" / "constants" / "opponents.h"

WEIGHT_RE = re.compile(r"(?:/\*\s*)?Weight:\s*(?P<val>[0-9]+|NONE)\s*(?:\*/)?")
WEIGHT_TOTAL_RE = re.compile(r"^WeightTotal:\s*(\d+)\s*$")
LABEL_RE = re.compile(r"^===\s*(?P<label>[A-Z0-9_]+)\s*===\s*$")
TRAINER_LABEL_RE = re.compile(r"^===\s*(TRAINER_[A-Z0-9_]+)\s*===\s*$")
FIELD_RE = re.compile(r"^(?P<key>[A-Za-z]+):\s*(?P<val>.+?)\s*$")
SKIP_ID_RE = re.compile(r"\b(TRAINER_[A-Z0-9_]+)\b")
SKIP_LIST: List[str] = []


@dataclass
class PoolMeta:
    key: str
    weight_total: int
    weight_sum: int
    none_count: int
    path: pathlib.Path
    mons: List["PoolMon"]
    is_trainer: bool
    trainer_id: Optional[str]


@dataclass
class PoolMon:
    species: str  # SPECIES_FOO
    level: int
    weight: int


@dataclass
class RankSpec:
    trainer_id: str
    normal_rank: Optional[str]
    normal_count: Optional[int]
    rare_rank: Optional[str]
    rare_count: Optional[int]
    allow_duplicates: bool
    max_same: Optional[int]
    use_trainer_pool: bool
    trainer_pool_count: Optional[int]
    path: pathlib.Path
    line: int


def parse_pool(path: pathlib.Path, expected_total: int) -> PoolMeta:
    # read top-level label/WeightTotal
    text = path.read_text(encoding="utf-8")
    label = None
    declared_total = None
    weight_sum = 0
    none_count = 0
    blocks: List[List[str]] = []
    cur_block: List[str] = []

    for line in text.splitlines():
        if label is None:
            m = LABEL_RE.match(line.strip())
            if m:
                label = m.group("label")
        if declared_total is None:
            m = WEIGHT_TOTAL_RE.match(line.strip())
            if m:
                declared_total = int(m.group(1))
                continue
        for m in WEIGHT_RE.finditer(line):
            val = m.group("val")
            if val == "NONE":
                none_count += 1
            else:
                weight = int(val)
                if weight <= 0:
                    raise ValueError(f"{path}: weight must be >0 (got {weight})")
                weight_sum += weight
        line_stripped = line.strip()
        if line_stripped == "":
            if cur_block:
                blocks.append(cur_block)
                cur_block = []
        else:
            # Skip non-weight comments (e.g., header comments)
            if line_stripped.startswith("/*") and "Weight:" not in line_stripped:
                continue
            if LABEL_RE.match(line_stripped):
                continue
            if WEIGHT_TOTAL_RE.match(line_stripped):
                continue
            m_field = FIELD_RE.match(line_stripped)
            if m_field:
                key = m_field.group("key")
                if key not in ("Level", "Weight"):
                    # skip metadata fields (Name/Class/Pic/Gender/etc.)
                    continue
            cur_block.append(line)

    if cur_block:
        blocks.append(cur_block)

    if label is None:
        raise ValueError(f"{path}: missing label line (=== LABEL ===)")
    if declared_total is None:
        raise ValueError(f"{path}: missing WeightTotal")
    if declared_total not in (100, 1000):
        raise ValueError(f"{path}: WeightTotal must be 100 or 1000 (got {declared_total})")
    if declared_total != expected_total:
        raise ValueError(f"{path}: WeightTotal={declared_total} but mode={expected_total}")
    if weight_sum != expected_total:
        raise ValueError(f"{path}: weight sum {weight_sum} != WeightTotal {expected_total} (NONE entries: {none_count})")

    mons: List[PoolMon] = []
    for block in blocks:
        species_line = None
        level = None
        weight = None
        for line in block:
            if species_line is None and not FIELD_RE.match(line.strip()) and not line.strip().startswith("/*"):
                species_line = line.strip()
            m = re.match(r"Level:\s*(\d+)", line.strip())
            if m:
                level = int(m.group(1))
            m = WEIGHT_RE.search(line)
            if m:
                v = m.group("val")
                if v != "NONE":
                    weight = int(v)
        if species_line is None:
            raise ValueError(f"{path}: missing species line in block: {block}")
        if level is None:
            raise ValueError(f"{path}: missing Level for {species_line} in {path}")
        if weight is None:
            raise ValueError(f"{path}: missing Weight for {species_line} in {path}")
        mons.append(PoolMon(species_line, level, weight))

    is_trainer = label.startswith("TRAINER_")
    trainer_id = label if is_trainer else None
    return PoolMeta(label, declared_total, weight_sum, none_count, path, mons, is_trainer, trainer_id)


def normalize_rank(val: str) -> str:
    v = val.strip().upper()
    if v not in ("S", "A", "B", "C", "D", "E"):
        raise ValueError(f"invalid rank '{val}'")
    return v


def parse_bool(val: str) -> bool:
    v = val.strip().lower()
    if v == "true":
        return True
    if v == "false":
        return False
    raise ValueError(f"invalid boolean '{val}', use true/false")


def parse_trainer_specs(path: pathlib.Path) -> List[RankSpec]:
    specs: List[RankSpec] = []
    current_trainer: Optional[str] = None
    current_line = 0
    tmp: Dict[str, Tuple[str, int]] = {}

    def flush():
        nonlocal tmp
        if current_trainer is None:
            return
        normal_rank = tmp.get("NormalRank", (None, -1))[0]
        normal_count_raw = tmp.get("NormalCount", (None, -1))[0]
        rare_rank = tmp.get("RareRank", (None, -1))[0]
        rare_count_raw = tmp.get("RareCount", (None, -1))[0]
        allow_dup_raw = tmp.get("AllowDuplicates", ("false", -1))[0]
        max_same_raw = tmp.get("MaxSame", (None, -1))[0]
        multi_mode_raw = tmp.get("MultiRandomMode", ("false", -1))[0]
        multi_count_raw = tmp.get("MultiRandomCount", (None, -1))[0]

        normal_count = int(normal_count_raw) if normal_count_raw is not None else None
        rare_count = int(rare_count_raw) if rare_count_raw is not None else None

        # Pairing checks
        if normal_rank and normal_count is None:
            raise ValueError(f"{path}:{tmp['NormalRank'][1]} NormalRank set but NormalCount missing for {current_trainer}")
        if normal_count is not None and normal_rank is None:
            raise ValueError(f"{path}:{tmp['NormalCount'][1]} NormalCount set but NormalRank missing for {current_trainer}")
        if rare_rank and rare_count is None:
            raise ValueError(f"{path}:{tmp['RareRank'][1]} RareRank set but RareCount missing for {current_trainer}")
        if rare_count is not None and rare_rank is None:
            raise ValueError(f"{path}:{tmp['RareCount'][1]} RareCount set but RareRank missing for {current_trainer}")

        if normal_rank:
            normal_rank = normalize_rank(normal_rank)
        if rare_rank:
            rare_rank = normalize_rank(rare_rank)

        allow_dup = parse_bool(allow_dup_raw)
        max_same: Optional[int]
        if max_same_raw is None:
            max_same = None
        else:
            max_same = int(max_same_raw)
            if max_same <= 0:
                raise ValueError(f"{path}:{tmp['MaxSame'][1]} MaxSame must be >0 for {current_trainer}")
            if not allow_dup:
                raise ValueError(f"{path}:{tmp['MaxSame'][1]} MaxSame requires AllowDuplicates=true for {current_trainer}")

        use_trainer_pool = parse_bool(multi_mode_raw)
        trainer_pool_count = int(multi_count_raw) if multi_count_raw is not None else None
        if use_trainer_pool and trainer_pool_count is None:
            raise ValueError(f"{path}:{tmp['MultiRandomMode'][1]} MultiRandomMode requires MultiRandomCount for {current_trainer}")
        if not use_trainer_pool and multi_count_raw is not None:
            raise ValueError(f"{path}:{tmp['MultiRandomCount'][1]} MultiRandomCount without MultiRandomMode for {current_trainer}")

        specs.append(
            RankSpec(
                trainer_id=current_trainer,
                normal_rank=normal_rank,
                normal_count=normal_count,
                rare_rank=rare_rank,
                rare_count=rare_count,
                allow_duplicates=allow_dup,
                max_same=max_same,
                use_trainer_pool=use_trainer_pool,
                trainer_pool_count=trainer_pool_count,
                path=path,
                line=current_line,
            )
        )
        tmp = {}

    for lineno, line in enumerate(path.read_text(encoding="utf-8").splitlines(), start=1):
        line_stripped = line.strip()

        if line_stripped.startswith("/*") and "RankSpec:" in line_stripped:
            after = line_stripped.split("RankSpec:", 1)[1]
            after = after.split("*/", 1)[0]
            for token in after.strip().split():
                if "=" not in token:
                    continue
                k, v = token.split("=", 1)
                tmp[k] = (v, lineno)
            continue
        if line_stripped.startswith("/*") and "MultiRandom:" in line_stripped:
            after = line_stripped.split("MultiRandom:", 1)[1]
            after = after.split("*/", 1)[0]
            tmp["MultiRandomMode"] = ("true", lineno)
            for token in after.strip().split():
                if "=" not in token:
                    continue
                k, v = token.split("=", 1)
                if k.lower() == "count":
                    tmp["MultiRandomCount"] = (v, lineno)
            continue

        # Field-based MultiRandom
        if line_stripped.lower().startswith("multirandom"):
            tmp["MultiRandomMode"] = ("true", lineno)
            after = line_stripped.split(":", 1)[1] if ":" in line_stripped else line_stripped
            after = after.strip()
            # Support "Count=2" or just "2"
            for token in after.split():
                if "=" in token:
                    k, v = token.split("=", 1)
                    if k.lower() == "count":
                        tmp["MultiRandomCount"] = (v, lineno)
                else:
                    # bare number
                    if token.isdigit():
                        tmp["MultiRandomCount"] = (token, lineno)
            continue

        m_tr = TRAINER_LABEL_RE.match(line_stripped)
        if m_tr:
            flush()
            current_trainer = m_tr.group(1)
            current_line = lineno
            tmp = {}
            continue
        m_field = FIELD_RE.match(line_stripped)
        if m_field and current_trainer:
            key = m_field.group("key")
            val = m_field.group("val")
            if key in ("NormalRank", "NormalCount", "RareRank", "RareCount", "AllowDuplicates", "MaxSame", "MultiRandomMode", "MultiRandomCount"):
                tmp[key] = (val, lineno)
    flush()
    return [s for s in specs if any([s.normal_rank, s.rare_rank, s.allow_duplicates, s.use_trainer_pool])]


def load_species_constants(species_path: pathlib.Path) -> Dict[str, str]:
    mapping: Dict[str, str] = {}
    for line in species_path.read_text(encoding="utf-8").splitlines():
        m = re.match(r"#define\s+(SPECIES_[A-Z0-9_]+)\s+", line.strip())
        if not m:
            continue
        const = m.group(1)
        key = const[len("SPECIES_") :]
        mapping[key] = const
    return mapping


def canonicalize_species_name(raw: str) -> str:
    s = raw.strip().upper()
    # replace non-alnum with underscore
    s = re.sub(r"[^A-Z0-9]+", "_", s)
    s = re.sub(r"_+", "_", s).strip("_")
    return s


def resolve_species(raw: str, species_map: Dict[str, str]) -> str:
    if raw.startswith("SPECIES_"):
        return raw
    key = canonicalize_species_name(raw)
    if key in species_map:
        return species_map[key]
    raise ValueError(f"unknown species '{raw}' (normalized: {key})")


def load_trainer_constants(tr_path: pathlib.Path) -> Dict[str, str]:
    mapping: Dict[str, str] = {}
    for line in tr_path.read_text(encoding="utf-8").splitlines():
        m = re.match(r"#define\s+(TRAINER_[A-Z0-9_]+)\s+(\d+)", line.strip())
        if m:
            mapping[m.group(1)] = m.group(1)
    return mapping


HEADER_PREAMBLE = """// Auto-generated by build_trainer_rank_parties.py. Do not edit manually.
#pragma once
#include "global.h"
#include "constants/trainers.h"
#include "trainer_rank.h"
// Internal spec used for generated data
struct TrainerRankSpec {
    u16 trainerId;
    char normalRank; // 0 if unset
    u8 normalCount;  // 0 if unset
    char rareRank;   // 0 if unset
    u8 rareCount;    // 0 if unset
    bool8 allowDuplicates;
    u8 maxSame;      // 0 if unset/unlimited; only relevant if allowDuplicates
    bool8 useTrainerPool;
    u8 trainerPoolCount; // 0 if unset
    u16 trainerPoolIndex; // 0xFFFF if unset
};
"""


def write_header(out_path: pathlib.Path, pools: List[PoolMeta], specs: List[RankSpec], trainer_pool_map: List[Tuple[str, int]]) -> None:
    lines: List[str] = [HEADER_PREAMBLE]

    lines.append(f"#define TRAINER_RANK_POOL_COUNT {len(pools)}\n")
    total_mons = sum(len(p.mons) for p in pools)
    lines.append(f"#define TRAINER_RANK_MON_COUNT {total_mons}\n\n")

    # mon table
    lines.append("const struct TrainerRankMon gTrainerRankMonTable[] = {\n")
    for p in pools:
        for mon in p.mons:
            lines.append(f"    {{ {mon.species}, {mon.level}, {mon.weight} }},\n")
    lines.append("};\n\n")

    # pool definitions with offsets
    offset = 0
    lines.append("const struct TrainerRankPoolDef gTrainerRankPools[] = {\n")
    for p in pools:
        count = len(p.mons)
        is_trainer = "TRUE" if p.is_trainer else "FALSE"
        lines.append(
            f'    {{ "{p.key}", &gTrainerRankMonTable[{offset}], {count}, {p.weight_total}, {is_trainer} }},\n'
        )
        offset += count
    lines.append("};\n\n")

    # shared rank map (S/A/B/C/D/E)
    shared_map: List[str] = ["0xFFFF"] * 6
    for idx, p in enumerate(pools):
        if p.key.startswith("RANK_") and len(p.key) >= 6:
            rank_letter = p.key[5]
            order = ["S", "A", "B", "C", "D", "E"]
            if rank_letter in order:
                shared_map[order.index(rank_letter)] = str(idx)
    lines.append("const u16 gTrainerRankSharedMap[6] = { ")
    lines.append(", ".join(shared_map))
    lines.append(" };\n\n")

    # trainer-specific map
    lines.append(f"#define TRAINER_RANK_TRAINER_POOL_COUNT {len(trainer_pool_map)}\n")
    lines.append("const struct TrainerRankTrainerPoolMap gTrainerRankTrainerPools[] = {\n")
    for trainer_id, pool_idx in trainer_pool_map:
        lines.append(f"    {{ {trainer_id}, {pool_idx} }},\n")
    lines.append("};\n\n")
    lines.append("const u16 gTrainerRankTrainerPoolCount = TRAINER_RANK_TRAINER_POOL_COUNT;\n\n")

    lines.append(f"#define TRAINER_RANK_SPEC_COUNT {len(specs)}\n")
    lines.append("static const struct TrainerRankSpec gTrainerRankSpecs[] = {\n")
    for s in specs:
        nrank = f"'{s.normal_rank}'" if s.normal_rank else "0"
        rrank = f"'{s.rare_rank}'" if s.rare_rank else "0"
        ncount = s.normal_count or 0
        rcount = s.rare_count or 0
        dup = "TRUE" if s.allow_duplicates else "FALSE"
        max_same = s.max_same or 0
        bool_pool = "TRUE" if s.use_trainer_pool else "FALSE"
        pool_count = s.trainer_pool_count or 0
        pool_idx = "0xFFFF"
        # find pool index if any
        for tid, idx in trainer_pool_map:
            if tid == s.trainer_id:
                pool_idx = str(idx)
                break
        lines.append(
            f"    {{ {s.trainer_id}, {nrank}, {ncount}, {rrank}, {rcount}, {dup}, {max_same}, {bool_pool}, {pool_count}, {pool_idx} }},\n"
        )
    lines.append("};\n\n")

    lines.append(f"#define TRAINER_RANK_SKIP_COUNT {len(SKIP_LIST)}\n")
    lines.append("static const u16 gTrainerRankSkipList[] = {\n")
    for tid in SKIP_LIST:
        lines.append(f"    {tid},\n")
    lines.append("};\n")
    lines.append("\n")

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text("".join(lines), encoding="utf-8")


def main() -> int:
    ap = argparse.ArgumentParser(description="Validate trainer rank .party files and generate metadata header.")
    ap.add_argument("--dir", type=pathlib.Path, default=DEFAULT_DIR, help="Directory containing rank .party files")
    ap.add_argument("--trainer", type=pathlib.Path, default=DEFAULT_TRAINER_PARTY, help="Path to trainers.party")
    ap.add_argument("--out", type=pathlib.Path, default=DEFAULT_OUT, help="Output header path")
    ap.add_argument("--mode", type=int, default=100, choices=[100, 1000], help="WeightTotal/WeightScale mode")
    ap.add_argument("--skip", type=pathlib.Path, default=DEFAULT_SKIP, help="trainer skip list header (for blacklist)")
    ap.add_argument("--species", type=pathlib.Path, default=DEFAULT_SPECIES, help="constants/species.h path")
    ap.add_argument("--trainer-const", dest="trainer_const", type=pathlib.Path, default=DEFAULT_TRAINER_CONST, help="constants/trainers.h path")
    ap.add_argument("--check", action="store_true", help="Validate only; do not write header")
    args = ap.parse_args()

    errors: List[str] = []
    species_map: Dict[str, str] = {}
    if args.species.exists():
        try:
            species_map = load_species_constants(args.species)
        except Exception as e:  # noqa: BLE001
            errors.append(str(e))
    else:
        errors.append(f"species constants not found at {args.species}")
    trainer_const_map: Dict[str, str] = {}
    if args.trainer_const.exists():
        try:
            trainer_const_map = load_trainer_constants(args.trainer_const)
        except Exception as e:  # noqa: BLE001
            errors.append(str(e))
    else:
        errors.append(f"trainer constants not found at {args.trainer_const}")

    pool_files = sorted(args.dir.glob("*.party"))
    if not pool_files:
        errors.append(f"no .party files found in {args.dir}")
    pools: List[PoolMeta] = []
    for path in pool_files:
        try:
            pools.append(parse_pool(path, args.mode))
        except Exception as e:  # noqa: BLE001
            errors.append(str(e))

    specs: List[RankSpec] = []
    if args.trainer.exists():
        try:
            specs = parse_trainer_specs(args.trainer)
        except Exception as e:  # noqa: BLE001
            errors.append(str(e))
    else:
        # trainers.party missing is not fatal, but note it.
        errors.append(f"trainer.party not found at {args.trainer}")

    # skip list
    global SKIP_LIST  # noqa: PLW0603
    SKIP_LIST = []
    if args.skip.exists():
        try:
            text = args.skip.read_text(encoding="utf-8")
            for m in SKIP_ID_RE.finditer(text):
                tid = m.group(1)
                if tid not in SKIP_LIST:
                    SKIP_LIST.append(tid)
        except Exception as e:  # noqa: BLE001
            errors.append(str(e))
    else:
        errors.append(f"skip list not found at {args.skip}")

    if errors:
        for e in errors:
            print(f"[ERROR] {e}", file=sys.stderr)
        return 1

    if args.check:
        print(f"validated {len(pools)} pools, {len(specs)} trainer specs (mode={args.mode})")
        return 0

    try:
        # resolve species and build mon table before writing
        for p in pools:
            for mon in p.mons:
                mon.species = resolve_species(mon.species, species_map)
        trainer_pool_map: List[Tuple[str, int]] = []
        warnings: List[str] = []

        # filter specs to only those with known trainer constants
        valid_specs: List[RankSpec] = []
        for s in specs:
            if s.trainer_id in trainer_const_map:
                valid_specs.append(s)
            else:
                warnings.append(f"trainer spec {s.trainer_id} not found in constants; skipping")
        specs = valid_specs

        for idx, p in enumerate(pools):
            if p.is_trainer:
                if p.trainer_id not in trainer_const_map:
                    warnings.append(f"trainer pool {p.trainer_id} not found in constants; trainer_pool_index set to 0xFFFF")
                else:
                    trainer_pool_map.append((p.trainer_id, idx))
        write_header(args.out, pools, specs, trainer_pool_map)
    except Exception as e:  # noqa: BLE001
        print(f"[ERROR] failed to write header: {e}", file=sys.stderr)
        return 1
    print(f"wrote {args.out} (pools={len(pools)} specs={len(specs)} trainerPools={len(trainer_pool_map)})")
    for w in warnings:
        print(f"[WARN] {w}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
