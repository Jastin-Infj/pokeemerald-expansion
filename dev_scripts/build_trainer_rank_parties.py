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
DEFAULT_TAGS_OUT = ROOT / "generated" / "trainer_rank_tags.h"
DEFAULT_SPECIES = ROOT / "include" / "constants" / "species.h"
DEFAULT_TRAINER_CONST = ROOT / "include" / "constants" / "opponents.h"

WEIGHT_RE = re.compile(r"(?:/\*\s*)?Weight:\s*(?P<val>[0-9]+|NONE)\s*(?:\*/)?")
WEIGHT_TOTAL_RE = re.compile(r"^WeightTotal:\s*(\d+)\s*$")
LABEL_RE = re.compile(r"^===\s*(?P<label>[A-Z0-9_]+)\s*===\s*$")
TRAINER_LABEL_RE = re.compile(r"^===\s*(TRAINER_[A-Z0-9_]+)\s*===\s*$")
FIELD_RE = re.compile(r"^(?P<key>[A-Za-z]+):\s*(?P<val>.+?)\s*$")
SKIP_ID_RE = re.compile(r"\b(TRAINER_[A-Z0-9_]+)\b")
SKIP_LIST: List[str] = []
POOL_NUM_TAGS = 8  # must match include/trainer_pools.h


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
    tags: List[str]


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
    lead_tags: List[str]
    lead_mode_and: bool
    ace_tags: List[str]
    ace_mode_and: bool
    path: pathlib.Path
    line: int


@dataclass
class LeadAceTags:
    lead_tags: List[str]
    lead_mode_and: bool
    lead_line: int
    ace_tags: List[str]
    ace_mode_and: bool
    ace_line: int


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
                if key not in ("Level", "Weight", "Tags"):
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
        tags: List[str] = []
        for line in block:
            if species_line is None and not FIELD_RE.match(line.strip()) and not line.strip().startswith("/*"):
                species_line = line.strip()
            m = re.match(r"Level:\s*(\d+)", line.strip())
            if m:
                level = int(m.group(1))
            m = re.match(r"Tags:\s*(.+)", line.strip(), flags=re.IGNORECASE)
            if m:
                if re.match(r"^Tags:\s*(OR:|AND:)", line.strip(), flags=re.IGNORECASE):
                    raise ValueError(f"{path}: Tags in pool definitions must not include OR:/AND: (use plain tag list)")
                try:
                    tags, _ = parse_tags_line(line, require_mode=False)
                except ValueError as e:
                    raise ValueError(f"{path}: invalid Tags line: {e}") from e
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
        mons.append(PoolMon(species_line, level, weight, tags))

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

        lead_tags: List[str] = []
        ace_tags: List[str] = []
        lead_mode_and = False
        ace_mode_and = False

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
                lead_tags=lead_tags,
                lead_mode_and=lead_mode_and,
                ace_tags=ace_tags,
                ace_mode_and=ace_mode_and,
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


def normalize_tag_name(raw: str) -> str:
    s = raw.strip().upper()
    s = re.sub(r"[^A-Z0-9]+", "_", s)
    s = re.sub(r"_+", "_", s).strip("_")
    return s


def tags_expr(tags: List[str]) -> str:
    if not tags:
        return "0"
    parts = [f"MON_POOL_TAG_{normalize_tag_name(t)}" for t in tags]
    return " | ".join(parts)


def tags_expr_with_mode(tags: List[str], mode_and: bool) -> str:
    expr = tags_expr(tags)
    if expr == "0":
        return "0"
    if mode_and:
        return f"MON_POOL_TAGMODE_AND | {expr}"
    return expr


def parse_tags_line(raw: str, require_mode: bool = True) -> Tuple[List[str], bool]:
    text = raw.strip()
    if text.lower().startswith("tags:"):
        text = text.split(":", 1)[1].strip()
    parts = [p.strip() for p in text.split("/") if p.strip()]
    if not parts:
        raise ValueError("Tags must include at least one tag")
    mode_and = False
    head = parts[0]
    if head.upper().startswith("OR:"):
        parts[0] = head[3:].strip()
    elif head.upper().startswith("AND:"):
        mode_and = True
        parts[0] = head[4:].strip()
    elif require_mode:
        raise ValueError("Tags must start with OR: or AND:")
    tags = [normalize_tag_name(p) for p in parts if p]
    if not tags:
        raise ValueError("Tags must include at least one tag")
    return tags, mode_and


@dataclass
class TagSlot:
    trainer_id: str
    slot_index: int
    tags: List[str]
    mode_and: bool
    rank_letter: Optional[str]
    multi_random_count: int
    line: int
    kind: str


def load_trainer_constants(tr_path: pathlib.Path) -> Dict[str, str]:
    mapping: Dict[str, str] = {}
    for line in tr_path.read_text(encoding="utf-8").splitlines():
        m = re.match(r"#define\s+(TRAINER_[A-Z0-9_]+)\s+(\d+)", line.strip())
        if m:
            mapping[m.group(1)] = m.group(1)
    return mapping


def extract_rank_letter(line: str) -> Optional[str]:
    for token in re.findall(r"[A-Za-z0-9_]+", line.strip()):
        token = token.upper()
        if len(token) == 6 and token.startswith("RANK_") and token[5] in "SABCDE":
            return token[5]
    return None


def parse_trainer_tag_slots(path: pathlib.Path) -> Tuple[List[TagSlot], Dict[str, int], Dict[str, LeadAceTags]]:
    slots: List[TagSlot] = []
    counts: Dict[str, int] = {}
    lead_ace: Dict[str, LeadAceTags] = {}
    current_trainer: Optional[str] = None
    multi_count = 0
    slot_index = 0
    current_species: Optional[str] = None
    current_tags: List[str] = []
    current_mode_and = False
    current_tags_line = 0
    current_tags_kind: Optional[str] = None
    in_block_comment = False

    def flush_mon():
        nonlocal slot_index, current_species, current_tags, current_mode_and, current_tags_line, current_tags_kind
        if current_species is not None:
            if current_tags:
                rank_letter = extract_rank_letter(current_species)
                slots.append(
                    TagSlot(
                        trainer_id=current_trainer or "",
                        slot_index=slot_index,
                        tags=current_tags,
                        mode_and=current_mode_and,
                        rank_letter=rank_letter,
                        multi_random_count=multi_count,
                        line=current_tags_line,
                        kind=current_tags_kind or "tags",
                    )
                )
                if current_trainer is not None and current_tags_kind in ("lead", "ace"):
                    entry = lead_ace.get(
                        current_trainer,
                        LeadAceTags([], False, 0, [], False, 0),
                    )
                    if current_tags_kind == "lead":
                        if entry.lead_tags:
                            raise ValueError(f"{path}:{current_tags_line} duplicate LeadTags for {current_trainer}")
                        entry.lead_tags = current_tags
                        entry.lead_mode_and = current_mode_and
                        entry.lead_line = current_tags_line
                    else:
                        if entry.ace_tags:
                            raise ValueError(f"{path}:{current_tags_line} duplicate AceTags for {current_trainer}")
                        entry.ace_tags = current_tags
                        entry.ace_mode_and = current_mode_and
                        entry.ace_line = current_tags_line
                    lead_ace[current_trainer] = entry
            slot_index += 1
        current_species = None
        current_tags = []
        current_mode_and = False
        current_tags_line = 0
        current_tags_kind = None

    for lineno, raw in enumerate(path.read_text(encoding="utf-8").splitlines(), start=1):
        line = raw.strip()
        if in_block_comment:
            if "*/" in line:
                in_block_comment = False
            continue
        if line.startswith("/*"):
            if "*/" not in line:
                in_block_comment = True
            continue
        if not line:
            if current_species is not None:
                flush_mon()
            continue

        m_tr = TRAINER_LABEL_RE.match(line)
        if m_tr:
            if current_trainer is not None:
                if current_species is not None:
                    flush_mon()
                counts[current_trainer] = slot_index
            current_trainer = m_tr.group(1)
            multi_count = 0
            slot_index = 0
            continue

        if current_trainer is None:
            continue

        if line.lower().startswith("multirandom"):
            # Support "MultiRandom: Count=2" or "MultiRandom: 2"
            count = None
            after = line.split(":", 1)[1].strip() if ":" in line else ""
            for token in after.split():
                if "=" in token:
                    k, v = token.split("=", 1)
                    if k.lower() == "count" and v.isdigit():
                        count = int(v)
                elif token.isdigit():
                    count = int(token)
            if count is not None:
                multi_count = count
            continue

        if line.lower().startswith(("tags:", "leadtags:", "acetags:")):
            if current_species is None:
                raise ValueError(f"{path}:{lineno} Tags must be inside a Pokemon slot (trainer header is not allowed)")
            if current_tags_kind is not None:
                raise ValueError(f"{path}:{lineno} only one of Tags/LeadTags/AceTags is allowed per slot")
            kind = "tags"
            if line.lower().startswith("leadtags:"):
                kind = "lead"
                raw = line.split(":", 1)[1].strip()
            elif line.lower().startswith("acetags:"):
                kind = "ace"
                raw = line.split(":", 1)[1].strip()
            else:
                raw = line
            try:
                current_tags, current_mode_and = parse_tags_line(raw, require_mode=True)
            except ValueError as e:
                label = "LeadTags" if kind == "lead" else "AceTags" if kind == "ace" else "Tags"
                raise ValueError(f"{path}:{lineno} invalid {label} line: {e}") from e
            if kind == "tags" and any(t in ("LEAD", "ACE") for t in current_tags):
                raise ValueError(f"{path}:{lineno} Tags cannot include Lead/Ace; use LeadTags/AceTags")
            current_tags_line = lineno
            current_tags_kind = kind
            continue

        if current_species is None:
            if ":" in line:
                # trainer header field
                continue
            current_species = line
            current_tags = []
            current_mode_and = False
            current_tags_line = 0
            continue

        # ignore other pokemon fields

    if current_trainer is not None:
        if current_species is not None:
            flush_mon()
        counts[current_trainer] = slot_index

    return slots, counts, lead_ace


def pool_match_count(pool: PoolMeta, required_tags: List[str], mode_and: bool) -> int:
    if not required_tags:
        return 0
    required = set(required_tags)
    count = 0
    for mon in pool.mons:
        mon_tags = set(mon.tags)
        if mode_and:
            if not required.issubset(mon_tags):
                continue
        else:
            if mon_tags.isdisjoint(required):
                continue
        count += 1
    return count


def validate_tag_slots(
    tag_slots: List[TagSlot],
    pools: List[PoolMeta],
    trainer_counts: Dict[str, int],
) -> List[str]:
    errors: List[str] = []
    rank_pools: Dict[str, PoolMeta] = {}
    trainer_pools: Dict[str, PoolMeta] = {}
    for p in pools:
        if p.key.startswith("RANK_") and len(p.key) >= 6:
            rank_letter = p.key[5]
            if rank_letter in "SABCDE":
                rank_pools[rank_letter] = p
        if p.is_trainer and p.trainer_id:
            trainer_pools[p.trainer_id] = p

    for slot in tag_slots:
        label = "Tags"
        if slot.kind == "lead":
            label = "LeadTags"
        elif slot.kind == "ace":
            label = "AceTags"
        pool = None
        if slot.rank_letter is not None:
            pool = rank_pools.get(slot.rank_letter)
            if pool is None:
                errors.append(f"{slot.trainer_id}:{slot.line} missing shared pool for rank {slot.rank_letter}")
                continue
        else:
            if slot.slot_index < slot.multi_random_count:
                pool = trainer_pools.get(slot.trainer_id)
                if pool is None:
                    errors.append(f"{slot.trainer_id}:{slot.line} {label} require trainer pool for MultiRandom slot")
                    continue
            else:
                errors.append(f"{slot.trainer_id}:{slot.line} {label} require RANK_* or MultiRandom slot")
                continue

        match_count = pool_match_count(pool, slot.tags, slot.mode_and)
        if match_count == 0:
            tag_list = "/".join(slot.tags)
            mode = "AND" if slot.mode_and else "OR"
            errors.append(f"{slot.trainer_id}:{slot.line} {label} {mode}:{tag_list} match 0 mons in pool {pool.key}")

    return errors


def collect_custom_tags(pools: List[PoolMeta], tag_slots: List[TagSlot], specs: List[RankSpec]) -> List[str]:
    reserved = {
        "LEAD",
        "ACE",
        "WEATHER_SETTER",
        "WEATHER_ABUSER",
        "SUPPORT",
        "TAG6",
        "TAG7",
        "TAG8",
    }
    tags: set[str] = set()
    for p in pools:
        for mon in p.mons:
            tags.update(mon.tags)
    for slot in tag_slots:
        tags.update(slot.tags)
    for spec in specs:
        tags.update(spec.lead_tags)
        tags.update(spec.ace_tags)
    custom = sorted(t for t in tags if t and t not in reserved)
    max_custom = 63 - POOL_NUM_TAGS  # reserve top bit for tag mode
    if len(custom) > max_custom:
        raise ValueError(f"too many custom tags: {len(custom)} (max {max_custom})")
    return custom


def write_tags_header(out_path: pathlib.Path, custom_tags: List[str]) -> None:
    lines: List[str] = [
        "// Auto-generated by build_trainer_rank_parties.py. Do not edit manually.\n",
        "#pragma once\n",
        "\n",
        "// Custom tags start after core pool tags.\n",
        "#define MON_POOL_TAG_CUSTOM_BASE POOL_NUM_TAGS\n",
        f"#define MON_POOL_TAG_CUSTOM_COUNT {len(custom_tags)}\n",
    ]
    for idx, name in enumerate(custom_tags):
        lines.append(f"#define MON_POOL_TAG_{name} (1ULL << (MON_POOL_TAG_CUSTOM_BASE + {idx}))\n")
    lines.append("\n")
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text("".join(lines), encoding="utf-8")


HEADER_PREAMBLE = """// Auto-generated by build_trainer_rank_parties.py. Do not edit manually.
#pragma once
#include "global.h"
#include "constants/trainers.h"
#include "trainer_rank.h"
#include "trainer_pools.h"
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
    u64 leadTags;
    u64 aceTags;
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
            lines.append(f"    {{ {mon.species}, {mon.level}, {mon.weight}, {tags_expr(mon.tags)} }},\n")
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
        lead_tags = tags_expr_with_mode(s.lead_tags, s.lead_mode_and)
        ace_tags = tags_expr_with_mode(s.ace_tags, s.ace_mode_and)
        lines.append(
            f"    {{ {s.trainer_id}, {nrank}, {ncount}, {rrank}, {rcount}, {dup}, {max_same}, {bool_pool}, {pool_count}, {pool_idx}, {lead_tags}, {ace_tags} }},\n"
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
    ap.add_argument("--tags-out", dest="tags_out", type=pathlib.Path, default=DEFAULT_TAGS_OUT, help="Output tag header path")
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
    tag_slots: List[TagSlot] = []
    lead_ace_map: Dict[str, LeadAceTags] = {}
    if args.trainer.exists():
        try:
            specs = parse_trainer_specs(args.trainer)
        except Exception as e:  # noqa: BLE001
            errors.append(str(e))
        try:
            tag_slots, trainer_counts, lead_ace_map = parse_trainer_tag_slots(args.trainer)
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

    if lead_ace_map:
        specs_by_trainer: Dict[str, RankSpec] = {s.trainer_id: s for s in specs}
        for trainer_id, entry in lead_ace_map.items():
            spec = specs_by_trainer.get(trainer_id)
            if spec is None:
                line = entry.lead_line or entry.ace_line or 0
                spec = RankSpec(
                    trainer_id=trainer_id,
                    normal_rank=None,
                    normal_count=None,
                    rare_rank=None,
                    rare_count=None,
                    allow_duplicates=False,
                    max_same=None,
                    use_trainer_pool=False,
                    trainer_pool_count=None,
                    lead_tags=[],
                    lead_mode_and=False,
                    ace_tags=[],
                    ace_mode_and=False,
                    path=args.trainer,
                    line=line,
                )
                specs.append(spec)
                specs_by_trainer[trainer_id] = spec
            if entry.lead_tags:
                spec.lead_tags = entry.lead_tags
                spec.lead_mode_and = entry.lead_mode_and
            if entry.ace_tags:
                spec.ace_tags = entry.ace_tags
                spec.ace_mode_and = entry.ace_mode_and

    tag_errors = validate_tag_slots(
        tag_slots,
        pools,
        trainer_counts if "trainer_counts" in locals() else {},
    )
    if tag_errors:
        for e in tag_errors:
            print(f"[ERROR] {e}", file=sys.stderr)
        return 1

    try:
        custom_tags = collect_custom_tags(pools, tag_slots, specs)
    except Exception as e:  # noqa: BLE001
        print(f"[ERROR] {e}", file=sys.stderr)
        return 1

    if args.check:
        print(f"validated {len(pools)} pools, {len(specs)} trainer specs (mode={args.mode})")
        return 0

    try:
        write_tags_header(args.tags_out, custom_tags)

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
