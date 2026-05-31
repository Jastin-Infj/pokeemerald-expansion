#!/usr/bin/env python3
"""Audit and relink Porymap-style map/layout assets.

This tool intentionally works on repo source files, not on Porymap internals.
It updates structured map JSON, layout JSON, map group JSON, exact script
include paths, and map references in warps/connections.
"""

from __future__ import annotations

import argparse
import datetime as dt
import io
import json
import os
import re
import shutil
import subprocess
import sys
import tarfile
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable


MAPS_DIR = Path("data/maps")
LAYOUTS_DIR = Path("data/layouts")
MAP_GROUPS = MAPS_DIR / "map_groups.json"
LAYOUTS_JSON = LAYOUTS_DIR / "layouts.json"
EVENT_SCRIPTS = Path("data/event_scripts.s")
REGION_MAP_SECTIONS_JSON = Path("src/data/region_map/region_map_sections.json")
REGION_MAP_C = Path("src/region_map.c")
FLAGS_H = Path("include/constants/flags.h")
BACKUP_ROOT = Path(".map_asset_relinker_backups")
SPECIAL_MAP_IDS = {"MAP_DYNAMIC", "MAP_NONE"}
SPECIAL_MAPSECS = {"MAPSEC_DYNAMIC", "MAPSEC_NONE"}
REGION_MAP_LAYOUT_FILES = {
    "hoenn": Path("src/data/region_map/region_map_layout.h"),
    "kanto": Path("src/data/region_map/region_map_layout_kanto.h"),
    "sevii123": Path("src/data/region_map/region_map_layout_sevii123.h"),
    "sevii45": Path("src/data/region_map/region_map_layout_sevii45.h"),
    "sevii67": Path("src/data/region_map/region_map_layout_sevii67.h"),
}
REGION_MAP_TYPES = {
    "hoenn": "REGION_MAP_HOENN",
    "kanto": "REGION_MAP_KANTO",
    "sevii123": "REGION_MAP_SEVII123",
    "sevii45": "REGION_MAP_SEVII45",
    "sevii67": "REGION_MAP_SEVII67",
}
FLY_ICON_STYLE_ALIASES = {
    "blue": "palette-blink",
    "blue-blink": "palette-blink",
    "default": "stock",
    "stock": "stock",
    "palette": "palette-blink",
    "palette-blink": "palette-blink",
    "red-outline": "red-outline",
}

KNOWN_GENERATED_OUTPUTS = {
    Path("include/constants/map_groups.h"),
    Path("include/constants/layouts.h"),
    Path("include/constants/map_event_ids.h"),
    Path("data/maps/groups.inc"),
    Path("data/maps/headers.inc"),
    Path("data/maps/events.inc"),
    Path("data/maps/connections.inc"),
    Path("data/layouts/layouts.inc"),
    Path("data/layouts/layouts_table.inc"),
    Path("src/data/map_group_count.h"),
}


@dataclass
class Diagnostic:
    level: str
    message: str


@dataclass
class MapRef:
    name: str
    path: Path
    data: dict[str, Any]
    groups: list[str]


@dataclass
class LayoutRef:
    layout_id: str
    data: dict[str, Any]
    index: int


class RelinkError(RuntimeError):
    pass


def repo_path(root: Path, relpath: Path | str) -> Path:
    return root / Path(relpath)


def rel(path: Path, root: Path) -> str:
    return path.relative_to(root).as_posix()


def display_path(path: Path, root: Path) -> str:
    try:
        return rel(path, root)
    except ValueError:
        return path.as_posix()


def load_json(path: Path) -> Any:
    with path.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def write_json(path: Path, data: Any) -> None:
    with path.open("w", encoding="utf-8") as handle:
        json.dump(data, handle, indent=2, ensure_ascii=False)
        handle.write("\n")


def read_text(path: Path) -> str:
    with path.open("r", encoding="utf-8") as handle:
        return handle.read()


def write_text(path: Path, text: str) -> None:
    with path.open("w", encoding="utf-8", newline="\n") as handle:
        handle.write(text)


def camel_to_upper_snake(name: str) -> str:
    # Handles both RougeCave_2F and LittlerootTown.
    parts: list[str] = []
    for token in name.split("_"):
        split = re.sub(r"(?<=[a-z])(?=[A-Z])", "_", token)
        parts.append(split.upper())
    return "_".join(filter(None, parts))


def title_identifier(name: str) -> str:
    parts = [part for part in re.split(r"[^A-Za-z0-9]+", name) if part]
    return "_".join(part[:1].upper() + part[1:] for part in parts)


def title_identifier_from_mapsec(mapsec_id: str) -> str:
    suffix = mapsec_id.removeprefix("MAPSEC_")
    return title_identifier(suffix)


def normalize_mapsec_id(mapsec_id: str) -> str:
    if not mapsec_id.startswith("MAPSEC_"):
        return mapsec_id
    suffix = mapsec_id.removeprefix("MAPSEC_")
    suffix = re.sub(r"(?<=[a-z0-9])(?=[A-Z])", "_", suffix)
    suffix = re.sub(r"[^A-Za-z0-9]+", "_", suffix).strip("_").upper()
    return f"MAPSEC_{suffix}" if suffix else mapsec_id


def display_name_from_mapsec(mapsec_id: str) -> str:
    suffix = mapsec_id.removeprefix("MAPSEC_")
    return re.sub(r"_+", " ", suffix).strip()


def safe_temp_name(name: str) -> str:
    value = re.sub(r"[^A-Za-z0-9_]+", "_", name).strip("_").lower()
    return value or "map"


def parse_pair(raw: str, label: str) -> tuple[str, str]:
    if ":" not in raw:
        raise RelinkError(f"{label} must use OLD:NEW format.")
    old, new = raw.split(":", 1)
    if not old or not new:
        raise RelinkError(f"{label} must use non-empty OLD:NEW values.")
    return old, new


def parse_bool(raw: str, label: str) -> bool:
    normalized = raw.strip().lower()
    if normalized in {"1", "true", "yes", "on"}:
        return True
    if normalized in {"0", "false", "no", "off"}:
        return False
    raise RelinkError(f"{label} must be true or false.")


def parse_mapsec_bounds(raw: str) -> dict[str, Any]:
    parts = raw.split(":")
    if len(parts) != 5:
        raise RelinkError("--set-mapsec-bounds must use MAPSEC_ID:X:Y:WIDTH:HEIGHT format.")
    mapsec_id, x, y, width, height = parts
    try:
        bounds = {
            "mapsec": mapsec_id,
            "x": int(x),
            "y": int(y),
            "width": int(width),
            "height": int(height),
        }
    except ValueError as err:
        raise RelinkError("--set-mapsec-bounds coordinates must be integers.") from err
    if bounds["width"] <= 0 or bounds["height"] <= 0:
        raise RelinkError("--set-mapsec-bounds width and height must be positive.")
    return bounds


def parse_region_map_cell(raw: str) -> dict[str, Any]:
    parts = raw.split(":")
    if len(parts) != 4:
        raise RelinkError("--set-region-map-cell must use REGION:X:Y:MAPSEC_ID format.")
    region, x, y, mapsec_id = parts
    region = region.lower()
    if region not in REGION_MAP_LAYOUT_FILES:
        valid = ", ".join(sorted(REGION_MAP_LAYOUT_FILES))
        raise RelinkError(f"--set-region-map-cell region must be one of: {valid}.")
    try:
        x_int = int(x)
        y_int = int(y)
    except ValueError as err:
        raise RelinkError("--set-region-map-cell coordinates must be integers.") from err
    return {"region": region, "x": x_int, "y": y_int, "mapsec": mapsec_id}


def parse_fly_location(raw: str) -> dict[str, str]:
    parts = raw.split(":")
    if len(parts) != 3:
        raise RelinkError("--ensure-fly-location must use REGION:MAPSEC_ID:FLAG format.")
    region, mapsec_id, flag = parts
    region = region.lower()
    if region not in REGION_MAP_TYPES:
        valid = ", ".join(sorted(REGION_MAP_TYPES))
        raise RelinkError(f"--ensure-fly-location region must be one of: {valid}.")
    if not flag.startswith("FLAG_"):
        raise RelinkError("--ensure-fly-location flag must start with FLAG_.")
    return {"region": region, "regionMapType": REGION_MAP_TYPES[region], "mapsec": mapsec_id, "flag": flag}


def parse_fly_icon_style(raw: str) -> dict[str, str | None]:
    parts = raw.split(":")
    if len(parts) not in (2, 3):
        raise RelinkError("--set-fly-icon-style must use MAPSEC_ID:STYLE[:FLAG] format.")
    mapsec_id, style = parts[0], parts[1].lower()
    if style not in FLY_ICON_STYLE_ALIASES:
        valid = ", ".join(sorted(FLY_ICON_STYLE_ALIASES))
        raise RelinkError(f"--set-fly-icon-style style must be one of: {valid}.")
    flag = parts[2] if len(parts) == 3 else None
    if flag is not None and not flag.startswith("FLAG_"):
        raise RelinkError("--set-fly-icon-style optional flag must start with FLAG_.")
    return {"mapsec": mapsec_id, "style": FLY_ICON_STYLE_ALIASES[style], "flag": flag}


def parse_flag_pair(raw: str, option: str) -> dict[str, str]:
    old_flag, new_flag = parse_pair(raw, option)
    if not old_flag.startswith("FLAG_") or not new_flag.startswith("FLAG_"):
        raise RelinkError(f"{option} values must be FLAG_* identifiers.")
    if old_flag == new_flag:
        raise RelinkError(f"{option} old and new flags must differ.")
    return {"oldFlag": old_flag, "newFlag": new_flag}


def parse_mapsec_flag(raw: str, option: str) -> dict[str, str]:
    mapsec_id, flag = parse_pair(raw, option)
    if not mapsec_id.startswith("MAPSEC_"):
        raise RelinkError(f"{option} mapsec must be a MAPSEC_* identifier.")
    if not flag.startswith("FLAG_"):
        raise RelinkError(f"{option} flag must be a FLAG_* identifier.")
    return {"mapsec": mapsec_id, "flag": flag}


def parse_mapsec_map(raw: str) -> dict[str, str]:
    parts = raw.split(":")
    if len(parts) not in {2, 3}:
        raise RelinkError("--ensure-mapsec-map must use MAPSEC_ID:MAP_ID[:HEAL_LOCATION_ID] format.")
    mapsec_id, map_id = parts[0], parts[1]
    heal_location = parts[2] if len(parts) == 3 else "HEAL_LOCATION_NONE"
    if not mapsec_id.startswith("MAPSEC_"):
        raise RelinkError("--ensure-mapsec-map mapsec must be a MAPSEC_* identifier.")
    if not map_id.startswith("MAP_"):
        raise RelinkError("--ensure-mapsec-map map must be a MAP_* identifier.")
    if not heal_location.startswith("HEAL_LOCATION_"):
        raise RelinkError("--ensure-mapsec-map heal location must be a HEAL_LOCATION_* identifier.")
    return {"mapsec": mapsec_id, "map": map_id, "healLocation": heal_location}


def iter_map_dirs(root: Path) -> Iterable[Path]:
    maps_dir = repo_path(root, MAPS_DIR)
    if not maps_dir.exists():
        return []
    return sorted(path for path in maps_dir.iterdir() if (path / "map.json").is_file())


def load_map_groups(root: Path) -> dict[str, Any]:
    path = repo_path(root, MAP_GROUPS)
    if not path.exists():
        raise RelinkError(f"Missing {MAP_GROUPS}.")
    return load_json(path)


def load_layouts(root: Path) -> dict[str, Any]:
    path = repo_path(root, LAYOUTS_JSON)
    if not path.exists():
        raise RelinkError(f"Missing {LAYOUTS_JSON}.")
    return load_json(path)


def build_map_index(root: Path) -> dict[str, MapRef]:
    groups_data = load_map_groups(root)
    map_to_groups: dict[str, list[str]] = {}
    for group in groups_data.get("group_order", []):
        for map_name in groups_data.get(group, []):
            map_to_groups.setdefault(map_name, []).append(group)

    maps: dict[str, MapRef] = {}
    for map_dir in iter_map_dirs(root):
        map_json = map_dir / "map.json"
        data = load_json(map_json)
        name = data.get("name", map_dir.name)
        maps[name] = MapRef(name=name, path=map_json, data=data, groups=map_to_groups.get(name, []))
    return maps


def build_layout_index(root: Path) -> dict[str, LayoutRef]:
    layouts_data = load_layouts(root)
    layouts: dict[str, LayoutRef] = {}
    for index, layout in enumerate(layouts_data.get("layouts", [])):
        if not isinstance(layout, dict):
            continue
        layout_id = layout.get("id")
        if layout_id:
            layouts[layout_id] = LayoutRef(layout_id=layout_id, data=layout, index=index)
    return layouts


def load_mapsec_ids(root: Path) -> set[str]:
    ids = set(SPECIAL_MAPSECS)
    path = repo_path(root, REGION_MAP_SECTIONS_JSON)
    if not path.exists():
        return ids
    data = load_json(path)
    for section in data.get("map_sections", []):
        if isinstance(section, dict) and isinstance(section.get("id"), str):
            ids.add(section["id"])
    return ids


def load_mapsec_entries(root: Path) -> dict[str, dict[str, Any]]:
    path = repo_path(root, REGION_MAP_SECTIONS_JSON)
    if not path.exists():
        return {}
    data = load_json(path)
    entries: dict[str, dict[str, Any]] = {}
    for section in data.get("map_sections", []):
        if isinstance(section, dict) and isinstance(section.get("id"), str):
            entries[section["id"]] = section
    return entries


def add_duplicate_diagnostics(
    diagnostics: list[Diagnostic],
    values: dict[str, list[str]],
    label: str,
    level: str = "error",
) -> None:
    for value, locations in sorted(values.items()):
        if len(locations) > 1:
            diagnostics.append(Diagnostic(level, f"{label} {value!r} appears multiple times: {', '.join(locations)}."))


def collect_audit(root: Path, target: str | None = None) -> list[Diagnostic]:
    diagnostics: list[Diagnostic] = []
    groups_data = load_map_groups(root)
    layouts_data = load_layouts(root)
    mapsec_ids = load_mapsec_ids(root)
    mapsec_entries = load_mapsec_entries(root)

    group_maps: dict[str, list[str]] = {}
    group_order_raw = groups_data.get("group_order")
    if not isinstance(group_order_raw, list):
        diagnostics.append(Diagnostic("error", "map_groups.json field 'group_order' must be a list."))
        group_order: list[Any] = []
    else:
        group_order = group_order_raw

    seen_groups: dict[str, list[str]] = {}
    for index, group in enumerate(group_order):
        if not isinstance(group, str):
            diagnostics.append(Diagnostic("error", f"map_groups.json group_order[{index}] must be a string."))
            continue
        if not re.fullmatch(r"gMapGroup_[A-Z][A-Za-z0-9_]*", group):
            diagnostics.append(Diagnostic("warning", f"map_groups.json group {group!r} has suspicious naming; expected gMapGroup_<UpperCamelOrUpperSnake>."))
        seen_groups.setdefault(group, []).append(f"group_order[{index}]")
        maps = groups_data.get(group)
        if not isinstance(maps, list):
            diagnostics.append(Diagnostic("error", f"map_groups.json lists group {group!r}, but that field is missing or not a list."))
            continue
        if not maps:
            diagnostics.append(Diagnostic("warning", f"map_groups.json group {group!r} is empty."))
        seen_in_group: set[str] = set()
        for map_index, map_name in enumerate(maps):
            if not isinstance(map_name, str):
                diagnostics.append(Diagnostic("error", f"map_groups.json {group}[{map_index}] must be a string."))
                continue
            if map_name in seen_in_group:
                diagnostics.append(Diagnostic("error", f"map_groups.json group {group!r} lists map {map_name!r} more than once."))
            seen_in_group.add(map_name)
            group_maps.setdefault(map_name, []).append(group)
    add_duplicate_diagnostics(diagnostics, seen_groups, "map group")

    for key, value in sorted(groups_data.items()):
        if key == "group_order":
            continue
        if isinstance(value, list) and key not in group_order:
            diagnostics.append(Diagnostic("warning", f"map_groups.json has group {key!r}, but it is not listed in group_order."))

    add_duplicate_diagnostics(diagnostics, group_maps, "map group entry")

    mapsec_id_locations: dict[str, list[str]] = {}
    mapsec_path = repo_path(root, REGION_MAP_SECTIONS_JSON)
    if mapsec_path.exists():
        mapsec_data = load_json(mapsec_path)
        sections = mapsec_data.get("map_sections")
        if not isinstance(sections, list):
            diagnostics.append(Diagnostic("error", f"{REGION_MAP_SECTIONS_JSON} field 'map_sections' must be a list."))
        else:
            for index, section in enumerate(sections):
                if not isinstance(section, dict):
                    diagnostics.append(Diagnostic("error", f"{REGION_MAP_SECTIONS_JSON} map_sections[{index}] must be an object."))
                    continue
                mapsec_id = section.get("id")
                if not isinstance(mapsec_id, str) or not mapsec_id:
                    diagnostics.append(Diagnostic("error", f"{REGION_MAP_SECTIONS_JSON} map_sections[{index}] must have a non-empty id."))
                    continue
                mapsec_id_locations.setdefault(mapsec_id, []).append(f"{REGION_MAP_SECTIONS_JSON}:map_sections[{index}]")
    add_duplicate_diagnostics(diagnostics, mapsec_id_locations, "region map section id")

    for mapsec_id, section in sorted(mapsec_entries.items()):
        if not re.fullmatch(r"MAPSEC_[A-Z0-9_]+", mapsec_id):
            diagnostics.append(Diagnostic("warning", f"region map section id {mapsec_id!r} has suspicious naming; expected uppercase MAPSEC_*."))
        name = section.get("name")
        if isinstance(name, str) and re.fullmatch(r"MAPSEC_ROUTE_\d+", mapsec_id) and name != mapsec_id.removeprefix("MAPSEC_").replace("_", " "):
            diagnostics.append(Diagnostic("warning", f"region map section {mapsec_id!r} has route-like id but display name {name!r}."))

    map_dirs = {path.name: path for path in iter_map_dirs(root)}
    map_ids: set[str] = set(SPECIAL_MAP_IDS)
    map_id_locations: dict[str, list[str]] = {}
    map_name_locations: dict[str, list[str]] = {}
    layout_ids = {
        layout.get("id")
        for layout in layouts_data.get("layouts", [])
        if isinstance(layout, dict) and layout.get("id")
    }

    for map_name in sorted(map_dirs):
        map_path = map_dirs[map_name] / "map.json"
        map_data = load_json(map_path)
        map_ref = MapRef(name=map_data.get("name", map_name), path=map_path, data=map_data, groups=group_maps.get(map_name, []))
        json_name = map_ref.data.get("name")
        if not isinstance(json_name, str) or not json_name:
            diagnostics.append(Diagnostic("error", f"{rel(map_ref.path, root)} must have a non-empty string name."))
        else:
            map_name_locations.setdefault(json_name, []).append(rel(map_ref.path, root))
            if re.fullmatch(r"(?i:test\d*|temp(?:orary)?[_-]?\d*)", json_name):
                diagnostics.append(Diagnostic("warning", f"{rel(map_ref.path, root)} has temporary-looking map name {json_name!r}."))
        if json_name != map_name:
            diagnostics.append(Diagnostic("error", f"{rel(map_ref.path, root)} name is {json_name!r}, expected {map_name!r}."))
        if map_name not in group_maps:
            diagnostics.append(Diagnostic("warning", f"{MAPS_DIR / map_name} exists but is not listed in map_groups.json."))
        map_id = map_ref.data.get("id")
        if not isinstance(map_id, str) or not map_id.startswith("MAP_"):
            diagnostics.append(Diagnostic("error", f"{rel(map_ref.path, root)} must have an id string starting with MAP_."))
        else:
            map_ids.add(map_id)
            map_id_locations.setdefault(map_id, []).append(rel(map_ref.path, root))
        layout_id = map_ref.data.get("layout")
        if layout_id not in layout_ids:
            diagnostics.append(Diagnostic("error", f"{rel(map_ref.path, root)} references missing layout {layout_id!r}."))
        mapsec = map_ref.data.get("region_map_section")
        if not isinstance(mapsec, str) or mapsec not in mapsec_ids:
            diagnostics.append(Diagnostic("error", f"{rel(map_ref.path, root)} references missing region_map_section {mapsec!r}."))

    add_duplicate_diagnostics(diagnostics, map_name_locations, "map json name")
    add_duplicate_diagnostics(diagnostics, map_id_locations, "map id")

    for map_name in sorted(group_maps):
        if map_name not in map_dirs:
            diagnostics.append(Diagnostic("error", f"map_groups.json lists {map_name!r}, but {MAPS_DIR / map_name / 'map.json'} is missing."))

    layout_id_counts: dict[str, int] = {}
    layout_name_locations: dict[str, list[str]] = {}
    for layout in layouts_data.get("layouts", []):
        if not isinstance(layout, dict):
            continue
        layout_id = layout.get("id")
        if layout_id:
            layout_id_counts[layout_id] = layout_id_counts.get(layout_id, 0) + 1
        else:
            diagnostics.append(Diagnostic("error", f"{LAYOUTS_JSON} has a layout entry without an id."))

        for field in ("border_filepath", "blockdata_filepath"):
            filepath = layout.get(field)
            if not isinstance(filepath, str) or not filepath:
                diagnostics.append(Diagnostic("error", f"layout {layout_id!r} {field} must be a non-empty string."))
            elif not repo_path(root, filepath).exists():
                diagnostics.append(Diagnostic("error", f"layout {layout_id!r} {field} points to missing {filepath}."))

        name = layout.get("name", "")
        if isinstance(name, str) and name:
            layout_name_locations.setdefault(name, []).append(str(layout_id or "<missing id>"))
        if "LAYOUT_" in name or name.endswith("_Layout_Layout"):
            diagnostics.append(Diagnostic("warning", f"layout {layout_id!r} has suspicious label name {name!r}."))

    for layout_id, count in sorted(layout_id_counts.items()):
        if count > 1:
            diagnostics.append(Diagnostic("error", f"layout id {layout_id!r} appears {count} times."))
    add_duplicate_diagnostics(diagnostics, layout_name_locations, "layout name")

    for map_json in iter_all_map_json(root):
        data = load_json(map_json)
        for index, connection in enumerate(data.get("connections") or []):
            if not isinstance(connection, dict):
                continue
            target_id = connection.get("map")
            if isinstance(target_id, str) and target_id not in map_ids:
                diagnostics.append(Diagnostic("error", f"{rel(map_json, root)} connection[{index}] references missing map id {target_id!r}."))
        for index, warp in enumerate(data.get("warp_events") or []):
            if not isinstance(warp, dict):
                continue
            target_id = warp.get("dest_map")
            if isinstance(target_id, str) and target_id not in map_ids:
                diagnostics.append(Diagnostic("error", f"{rel(map_json, root)} warp_events[{index}] references missing map id {target_id!r}."))

    event_scripts = repo_path(root, EVENT_SCRIPTS)
    if event_scripts.exists():
        text = read_text(event_scripts)
        include_locations: dict[str, list[str]] = {}
        for line_number, line in enumerate(text.splitlines(), start=1):
            match = re.search(r'\.include\s+"data/maps/([^"]+)/scripts\.inc"', line)
            if match:
                include_locations.setdefault(match.group(1), []).append(f"{EVENT_SCRIPTS}:{line_number}")
        add_duplicate_diagnostics(diagnostics, include_locations, "script include")
        included_maps = set(include_locations)
        for map_name in sorted(map_dirs):
            if (map_dirs[map_name] / "scripts.inc").exists() and map_name not in included_maps:
                diagnostics.append(Diagnostic("warning", f"{MAPS_DIR / map_name / 'scripts.inc'} is not included by {EVENT_SCRIPTS}."))
        for map_name in sorted(included_maps):
            if not repo_path(root, MAPS_DIR / map_name / "scripts.inc").exists():
                diagnostics.append(Diagnostic("error", f"{EVENT_SCRIPTS} includes missing {MAPS_DIR / map_name / 'scripts.inc'}."))

    if target:
        for path in scan_text_refs(root, [target]):
            diagnostics.append(Diagnostic("info", f"target text {target!r} appears in {path}."))

    return diagnostics


def print_diagnostics(diagnostics: list[Diagnostic]) -> int:
    for diagnostic in diagnostics:
        print(f"{diagnostic.level.upper()}: {diagnostic.message}")
    errors = sum(1 for diagnostic in diagnostics if diagnostic.level == "error")
    warnings = sum(1 for diagnostic in diagnostics if diagnostic.level == "warning")
    print(f"Audit complete: {errors} error(s), {warnings} warning(s).")
    return 1 if errors else 0


def map_groups_for_names(root: Path, names: set[str]) -> list[str]:
    groups_data = load_map_groups(root)
    groups: list[str] = []
    for group in groups_data.get("group_order", []):
        maps = groups_data.get(group, [])
        if any(map_name in names for map_name in maps):
            groups.append(group)
    return groups


def find_map_ref(root: Path, old_value: str, match_by: str) -> MapRef:
    matches: list[MapRef] = []
    for map_dir in iter_map_dirs(root):
        map_json = map_dir / "map.json"
        data = load_json(map_json)
        json_name = data.get("name")
        map_id = data.get("id")
        if (
            (match_by == "dir" and map_dir.name == old_value)
            or (match_by == "name" and json_name == old_value)
            or (match_by == "id" and map_id == old_value)
        ):
            group_names = {map_dir.name}
            if isinstance(json_name, str):
                group_names.add(json_name)
            matches.append(MapRef(name=json_name or map_dir.name, path=map_json, data=data, groups=map_groups_for_names(root, group_names)))
    if not matches:
        raise RelinkError(f"Map {old_value!r} was not found by {match_by} in {MAPS_DIR}.")
    if len(matches) > 1:
        locations = ", ".join(rel(match.path, root) for match in matches)
        raise RelinkError(f"Map {old_value!r} matched multiple maps by {match_by}: {locations}.")
    return matches[0]


def find_layout_ref(root: Path, layout_id: str) -> LayoutRef:
    layouts = build_layout_index(root)
    if layout_id not in layouts:
        raise RelinkError(f"Layout {layout_id!r} was not found in {LAYOUTS_JSON}.")
    return layouts[layout_id]


def default_layout_dir(layout: dict[str, Any]) -> str | None:
    border = layout.get("border_filepath")
    block = layout.get("blockdata_filepath")
    if not border or not block:
        return None
    border_dir = str(Path(border).parent)
    block_dir = str(Path(block).parent)
    if border_dir == block_dir:
        return border_dir
    return None


def make_plan(args: argparse.Namespace) -> dict[str, Any]:
    root = Path(args.root).resolve()
    old_map, new_map = parse_pair(args.map, "--map")
    map_ref = find_map_ref(root, old_map, args.match_by)
    old_dir_name = map_ref.path.parent.name
    new_dir_name = args.new_map_dir or new_map
    old_map_id = map_ref.data.get("id")
    old_map_ids = {map_id for map_id in (old_map_id, args.old_map_id) if isinstance(map_id, str) and map_id}
    if args.match_by != "id":
        old_map_ids.add(f"MAP_{camel_to_upper_snake(old_map)}")
    else:
        old_map_ids.add(old_map)
    new_map_id = args.new_map_id or f"MAP_{camel_to_upper_snake(new_map)}"
    if args.set_map_type and not args.set_map_type.startswith("MAP_TYPE_"):
        raise RelinkError("--set-map-type must be a MAP_TYPE_* identifier.")
    old_layout_id = args.old_layout_id or map_ref.data.get("layout")
    needs_layout_plan = (not args.no_layout_rename and not args.set_layout_id) or args.set_layout_name or args.set_primary_tileset or args.set_secondary_tileset
    layout_ref = find_layout_ref(root, old_layout_id) if needs_layout_plan else None
    if args.set_layout_id:
        find_layout_ref(root, args.set_layout_id)

    mapsec_plans = []
    new_mapsec = args.new_mapsec
    if args.rename_mapsec:
        if args.set_mapsec_name:
            raise RelinkError("--set-mapsec-name cannot be combined with --rename-mapsec; use --new-mapsec-name for rename plans.")
        old_mapsec, renamed_mapsec = parse_pair(args.rename_mapsec, "--rename-mapsec")
        mapsec_entries = load_mapsec_entries(root)
        if old_mapsec not in mapsec_entries:
            raise RelinkError(f"Region map section {old_mapsec!r} was not found.")
        if renamed_mapsec in mapsec_entries and renamed_mapsec != old_mapsec:
            raise RelinkError(f"Region map section {renamed_mapsec!r} already exists.")
        if args.new_mapsec and args.new_mapsec != renamed_mapsec:
            raise RelinkError("--new-mapsec must match the NEW side of --rename-mapsec when both are provided.")
        new_mapsec = renamed_mapsec
        mapsec_plans.append({
            "oldId": old_mapsec,
            "newId": renamed_mapsec,
            "oldName": mapsec_entries[old_mapsec].get("name"),
            "newName": args.new_mapsec_name,
        })
    elif new_mapsec and new_mapsec not in load_mapsec_ids(root):
        raise RelinkError(f"Region map section {new_mapsec!r} was not found.")
    if args.set_mapsec_name:
        mapsec_id, mapsec_name = parse_pair(args.set_mapsec_name, "--set-mapsec-name")
        mapsec_entries = load_mapsec_entries(root)
        if mapsec_id not in mapsec_entries:
            raise RelinkError(f"Region map section {mapsec_id!r} was not found.")
        mapsec_plans.append({
            "oldId": mapsec_id,
            "newId": mapsec_id,
            "oldName": mapsec_entries[mapsec_id].get("name"),
            "newName": mapsec_name,
        })
    known_mapsecs = load_mapsec_ids(root) | {plan["newId"] for plan in mapsec_plans}
    for bounds in [parse_mapsec_bounds(raw) for raw in args.set_mapsec_bounds or []]:
        if bounds["mapsec"] not in known_mapsecs:
            raise RelinkError(f"Region map section {bounds['mapsec']!r} was not found.")
        mapsec_plans.append({
            "oldId": bounds["mapsec"],
            "newId": bounds["mapsec"],
            "oldName": load_mapsec_entries(root).get(bounds["mapsec"], {}).get("name"),
            "newName": None,
            "bounds": {
                "x": bounds["x"],
                "y": bounds["y"],
                "width": bounds["width"],
                "height": bounds["height"],
            },
        })
    region_map_cells = [parse_region_map_cell(raw) for raw in args.set_region_map_cell or []]
    for cell in region_map_cells:
        if cell["mapsec"] not in known_mapsecs:
            raise RelinkError(f"Region map section {cell['mapsec']!r} was not found.")
    fly_locations = [parse_fly_location(raw) for raw in args.ensure_fly_location or []]
    for fly_location in fly_locations:
        if fly_location["mapsec"] not in known_mapsecs:
            raise RelinkError(f"Region map section {fly_location['mapsec']!r} was not found.")
    fly_mapsec_types = [parse_mapsec_flag(raw, "--ensure-fly-mapsec-type") for raw in args.ensure_fly_mapsec_type or []]
    for fly_mapsec_type in fly_mapsec_types:
        if fly_mapsec_type["mapsec"] not in known_mapsecs:
            raise RelinkError(f"Region map section {fly_mapsec_type['mapsec']!r} was not found.")
    fly_icon_styles = [parse_fly_icon_style(raw) for raw in args.set_fly_icon_style or []]
    for icon_style in fly_icon_styles:
        if icon_style["mapsec"] not in known_mapsecs:
            raise RelinkError(f"Region map section {icon_style['mapsec']!r} was not found.")
    mapsec_maps = [parse_mapsec_map(raw) for raw in args.ensure_mapsec_map or []]
    for mapsec_map in mapsec_maps:
        if mapsec_map["mapsec"] not in known_mapsecs:
            raise RelinkError(f"Region map section {mapsec_map['mapsec']!r} was not found.")
    flag_claims = [parse_flag_pair(raw, "--claim-unused-flag") for raw in args.claim_unused_flag or []]
    if args.set_transition_setflag and not args.set_transition_setflag.startswith("FLAG_"):
        raise RelinkError("--set-transition-setflag must be a FLAG_* identifier.")

    layout_plan = None
    if needs_layout_plan:
        assert layout_ref is not None
        new_layout_id = old_layout_id if args.no_layout_rename else (args.new_layout_id or f"LAYOUT_{camel_to_upper_snake(new_map)}")
        old_layout_name = layout_ref.data.get("name")
        if args.set_layout_name:
            new_layout_name = args.set_layout_name
        elif args.no_layout_rename:
            new_layout_name = old_layout_name
        else:
            new_layout_name = args.new_layout_name or f"{new_map}_Layout"
        old_layout_dir = None if args.no_layout_rename else (args.old_layout_dir or default_layout_dir(layout_ref.data))
        new_layout_dir = None if args.no_layout_rename else (args.new_layout_dir or f"{LAYOUTS_DIR.as_posix()}/{new_map}")
        layout_plan = {
            "oldId": old_layout_id,
            "newId": new_layout_id,
            "oldName": old_layout_name,
            "newName": new_layout_name,
            "oldDir": old_layout_dir,
            "newDir": new_layout_dir,
            "primaryTileset": args.set_primary_tileset,
            "secondaryTileset": args.set_secondary_tileset,
        }

    old_group_names = {old_dir_name, old_map}
    json_name = map_ref.data.get("name")
    if isinstance(json_name, str) and json_name:
        old_group_names.add(json_name)
    for old_group_map_name in args.old_group_map_name or []:
        old_group_names.add(old_group_map_name)
    groups = sorted(set(map_ref.groups + map_groups_for_names(root, old_group_names)))
    from_group = args.from_group
    to_group = args.to_group
    if from_group and from_group not in groups:
        raise RelinkError(f"Map {old_map!r} is not listed in source group {from_group!r}.")
    group = to_group or args.group or from_group or (groups[0] if groups else None)
    script_old_prefixes = sorted(set(args.old_script_prefix or []) | ({old_map, old_dir_name} if args.rewrite_script_labels else set()))
    plan: dict[str, Any] = {
        "version": 1,
        "maps": [
            {
                "oldName": old_map,
                "newName": new_map,
                "oldDirName": old_dir_name,
                "newDirName": new_dir_name,
                "oldId": old_map_id,
                "oldIds": sorted(old_map_ids),
                "newId": new_map_id,
                "newMapsec": new_mapsec,
                "newLayoutId": args.set_layout_id or (layout_plan["newId"] if layout_plan else None),
                "mapType": args.set_map_type,
                "showMapName": parse_bool(args.set_show_map_name, "--set-show-map-name") if args.set_show_map_name is not None else None,
                "transitionSetFlag": args.set_transition_setflag,
                "group": group,
                "fromGroup": from_group,
                "toGroup": to_group,
                "groupOldNames": sorted(old_group_names),
                "matchBy": args.match_by,
                "scriptOldPrefixes": script_old_prefixes,
            }
        ],
        "layouts": [layout_plan] if layout_plan else [],
        "mapsecs": mapsec_plans,
        "regionMapCells": region_map_cells,
        "flyLocations": fly_locations,
        "flyMapsecTypes": fly_mapsec_types,
        "flyIconStyles": fly_icon_styles,
        "mapsecMaps": mapsec_maps,
        "flagClaims": flag_claims,
        "dropGroups": args.drop_group or [],
        "options": {
            "rewriteScriptLabels": args.rewrite_script_labels,
        },
    }
    return plan


def save_plan(plan: dict[str, Any], out_path: str | None) -> None:
    if out_path:
        path = Path(out_path)
        write_json(path, plan)
        print(f"Wrote plan: {path}")
    else:
        print(json.dumps(plan, indent=2))


def load_plan(path: Path) -> dict[str, Any]:
    plan = load_json(path)
    if plan.get("version") != 1:
        raise RelinkError("Only plan version 1 is supported.")
    return plan


def target_paths_from_plan(root: Path, plan: dict[str, Any]) -> list[Path]:
    paths = [repo_path(root, MAP_GROUPS), repo_path(root, LAYOUTS_JSON), repo_path(root, EVENT_SCRIPTS)]
    if plan.get("mapsecs"):
        paths.append(repo_path(root, REGION_MAP_SECTIONS_JSON))
    if plan.get("flagClaims"):
        paths.append(repo_path(root, FLAGS_H))
    for cell in plan.get("regionMapCells", []):
        layout_file = REGION_MAP_LAYOUT_FILES.get(cell.get("region"))
        if layout_file:
            paths.append(repo_path(root, layout_file))
    if plan.get("flyLocations") or plan.get("flyMapsecTypes") or plan.get("flyIconStyles") or plan.get("mapsecMaps"):
        paths.append(repo_path(root, REGION_MAP_C))
    for layout in plan.get("layouts", []):
        for key in ("oldDir", "newDir"):
            if layout.get(key):
                paths.append(repo_path(root, layout[key]))
    for map_plan in plan.get("maps", []):
        old_dir_name = map_plan.get("oldDirName", map_plan["oldName"])
        new_dir_name = map_plan.get("newDirName", map_plan["newName"])
        paths.append(repo_path(root, MAPS_DIR / old_dir_name))
        paths.append(repo_path(root, MAPS_DIR / new_dir_name))
        paths.append(repo_path(root, MAPS_DIR / old_dir_name / "map.json"))
        paths.append(repo_path(root, MAPS_DIR / new_dir_name / "map.json"))
        paths.append(repo_path(root, MAPS_DIR / old_dir_name / "scripts.inc"))
        paths.append(repo_path(root, MAPS_DIR / new_dir_name / "scripts.inc"))
    return paths


def backup_paths_from_plan(root: Path, plan: dict[str, Any]) -> list[Path]:
    existing: list[Path] = []
    seen: set[Path] = set()
    for path in target_paths_from_plan(root, plan):
        if not path.exists():
            continue
        resolved = path.resolve()
        if resolved in seen:
            continue
        seen.add(resolved)
        existing.append(path)

    selected: list[Path] = []
    for path in sorted(existing, key=lambda item: len(item.relative_to(root).parts)):
        if any(path == parent or parent in path.parents for parent in selected):
            continue
        selected.append(path)
    return selected


def resolve_backup_root(root: Path, backup_root: str | None) -> Path:
    path = Path(backup_root) if backup_root else BACKUP_ROOT
    return path if path.is_absolute() else root / path


def create_backup_archive(root: Path, plan: dict[str, Any], backup_root: str | None) -> Path | None:
    paths = backup_paths_from_plan(root, plan)
    if not paths:
        print("BACKUP skipped; no existing apply targets.")
        return None

    backup_dir = resolve_backup_root(root, backup_root)
    backup_dir.mkdir(parents=True, exist_ok=True)
    timestamp = dt.datetime.now().strftime("%Y%m%d_%H%M%S_%f")
    archive = backup_dir / f"map_relink_{timestamp}.bak.tar"
    suffix = 1
    while archive.exists():
        archive = backup_dir / f"map_relink_{timestamp}_{suffix}.bak.tar"
        suffix += 1

    manifest = {
        "version": 1,
        "createdAt": dt.datetime.now(dt.timezone.utc).isoformat(),
        "format": "tar",
        "paths": [rel(path, root) for path in paths],
        "plan": plan,
    }
    manifest_bytes = json.dumps(manifest, indent=2, ensure_ascii=False).encode("utf-8")
    with tarfile.open(archive, "w") as tar:
        for path in paths:
            tar.add(path, arcname=rel(path, root), recursive=True)
        info = tarfile.TarInfo("MANIFEST.json")
        info.size = len(manifest_bytes)
        info.mtime = int(dt.datetime.now().timestamp())
        tar.addfile(info, io.BytesIO(manifest_bytes))

    print(f"BACKUP {display_path(archive, root)}")
    return archive


def check_dirty(root: Path, paths: list[Path]) -> None:
    relpaths = [rel(path, root) for path in paths if path.exists()]
    if not relpaths:
        return
    result = subprocess.run(
        ["git", "status", "--porcelain", "--", *relpaths],
        cwd=root,
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    if result.returncode != 0:
        raise RelinkError(result.stderr.strip() or "git status failed.")
    if result.stdout.strip():
        raise RelinkError("Target files have uncommitted changes. Re-run with --allow-dirty if intentional.")


def ensure_not_generated(path: Path, root: Path) -> None:
    try:
        relative = path.relative_to(root)
    except ValueError:
        return
    if relative in KNOWN_GENERATED_OUTPUTS:
        raise RelinkError(f"Refusing to edit generated output {relative}.")


def ensure_map_group(groups_data: dict[str, Any], group: str) -> list[Any]:
    group_order = groups_data.setdefault("group_order", [])
    if group not in group_order:
        group_order.append(group)
    return groups_data.setdefault(group, [])


def drop_empty_map_groups(groups_data: dict[str, Any], groups: Iterable[str]) -> bool:
    changed = False
    group_order = groups_data.get("group_order", [])
    if not isinstance(group_order, list):
        raise RelinkError("map_groups.json field 'group_order' must be a list.")
    for group in groups:
        if not group:
            continue
        maps = groups_data.get(group)
        if maps is not None and maps:
            raise RelinkError(f"Refusing to drop non-empty map group {group!r}.")
        if group in group_order:
            group_order.remove(group)
            changed = True
        if group in groups_data:
            del groups_data[group]
            changed = True
    return changed


def remove_map_from_group(maps: list[Any], map_names: set[str]) -> tuple[bool, int | None]:
    changed = False
    first_index: int | None = None
    index = 0
    while index < len(maps):
        if maps[index] in map_names:
            if first_index is None:
                first_index = index
            del maps[index]
            changed = True
            continue
        index += 1
    return changed, first_index


def update_map_groups(
    groups_data: dict[str, Any],
    old_names: Iterable[str],
    new_name: str,
    from_group: str | None,
    to_group: str | None,
    preferred_group: str | None,
) -> bool:
    before = json.dumps(groups_data, sort_keys=True)
    changed = False
    found = False
    old_name_set = {name for name in old_names if name}
    if to_group:
        insert_index: int | None = None
        for group in groups_data.get("group_order", []):
            if from_group and group != from_group:
                continue
            maps = groups_data.get(group, [])
            removed, first_index = remove_map_from_group(maps, old_name_set)
            if removed:
                found = True
                changed = True
                if group == to_group and insert_index is None:
                    insert_index = first_index

        target_maps = ensure_map_group(groups_data, to_group)
        if new_name not in target_maps:
            if insert_index is not None and insert_index <= len(target_maps):
                target_maps.insert(insert_index, new_name)
            else:
                target_maps.append(new_name)
            changed = True
        return changed and json.dumps(groups_data, sort_keys=True) != before

    for group in groups_data.get("group_order", []):
        if from_group and group != from_group:
            continue
        maps = groups_data.get(group, [])
        for index, map_name in enumerate(maps):
            if map_name in old_name_set:
                found = True
                if map_name != new_name:
                    maps[index] = new_name
                    changed = True
    if not found and preferred_group:
        maps = ensure_map_group(groups_data, preferred_group)
        if new_name not in maps:
            maps.append(new_name)
            changed = True
    return changed and json.dumps(groups_data, sort_keys=True) != before


def update_map_json(
    data: dict[str, Any],
    old_ids: set[str],
    new_id: str,
    new_name: str,
    new_layout: str | None,
    new_mapsec: str | None,
    map_type: str | None,
    show_map_name: bool | None,
) -> bool:
    changed = False
    if data.get("id") != new_id:
        data["id"] = new_id
        changed = True
    if data.get("name") != new_name:
        data["name"] = new_name
        changed = True
    if new_layout and data.get("layout") != new_layout:
        data["layout"] = new_layout
        changed = True
    if new_mapsec and data.get("region_map_section") != new_mapsec:
        data["region_map_section"] = new_mapsec
        changed = True
    if map_type and data.get("map_type") != map_type:
        data["map_type"] = map_type
        changed = True
    if show_map_name is not None and data.get("show_map_name") != show_map_name:
        data["show_map_name"] = show_map_name
        changed = True
    return changed


def update_mapsec_refs(data: dict[str, Any], old_id: str, new_id: str) -> bool:
    if data.get("region_map_section") == old_id and old_id != new_id:
        data["region_map_section"] = new_id
        return True
    return False


def update_mapsecs_json(data: dict[str, Any], mapsec_plan: dict[str, Any]) -> bool:
    changed = False
    old_id = mapsec_plan["oldId"]
    for section in data.get("map_sections", []):
        if not isinstance(section, dict) or section.get("id") != old_id:
            continue
        if section.get("id") != mapsec_plan["newId"]:
            section["id"] = mapsec_plan["newId"]
            changed = True
        if mapsec_plan.get("newName") and section.get("name") != mapsec_plan["newName"]:
            section["name"] = mapsec_plan["newName"]
            changed = True
        bounds = mapsec_plan.get("bounds")
        if isinstance(bounds, dict):
            for field in ("x", "y", "width", "height"):
                if section.get(field) != bounds[field]:
                    section[field] = bounds[field]
                    changed = True
        break
    return changed


def update_map_refs(data: dict[str, Any], old_ids: set[str], new_id: str) -> bool:
    changed = False
    connections = data.get("connections")
    if isinstance(connections, list):
        for connection in connections:
            if isinstance(connection, dict) and connection.get("map") in old_ids and connection.get("map") != new_id:
                connection["map"] = new_id
                changed = True
    warps = data.get("warp_events")
    if isinstance(warps, list):
        for warp in warps:
            if isinstance(warp, dict) and warp.get("dest_map") in old_ids and warp.get("dest_map") != new_id:
                warp["dest_map"] = new_id
                changed = True
    return changed


def update_layout_refs(data: dict[str, Any], layout_renames: dict[str, str]) -> bool:
    layout = data.get("layout")
    if layout in layout_renames:
        data["layout"] = layout_renames[layout]
        return True
    return False


def update_layouts_json(layouts_data: dict[str, Any], layout_plan: dict[str, Any]) -> bool:
    changed = False
    old_id = layout_plan["oldId"]
    for layout in layouts_data.get("layouts", []):
        if not isinstance(layout, dict) or layout.get("id") != old_id:
            continue
        layout["id"] = layout_plan["newId"]
        if layout_plan.get("newName"):
            layout["name"] = layout_plan["newName"]
        if layout_plan.get("primaryTileset"):
            layout["primary_tileset"] = layout_plan["primaryTileset"]
        if layout_plan.get("secondaryTileset"):
            layout["secondary_tileset"] = layout_plan["secondaryTileset"]
        old_dir = layout_plan.get("oldDir")
        new_dir = layout_plan.get("newDir")
        if old_dir and new_dir:
            for field in ("border_filepath", "blockdata_filepath"):
                value = layout.get(field)
                if isinstance(value, str) and value.startswith(old_dir.rstrip("/") + "/"):
                    layout[field] = new_dir.rstrip("/") + value[len(old_dir.rstrip("/")):]
        changed = True
        break
    return changed


def update_region_map_layout(text: str, cell_plan: dict[str, Any]) -> tuple[str, bool]:
    row_index = 0
    changed = False
    updated_lines: list[str] = []
    target_x = cell_plan["x"]
    target_y = cell_plan["y"]
    if target_x < 0 or target_y < 0:
        raise RelinkError("--set-region-map-cell coordinates must be non-negative.")

    for line in text.splitlines(keepends=True):
        match = re.match(r"^(\s*)\{(.+)\}(,?\s*)$", line.rstrip("\n"))
        if not match:
            updated_lines.append(line)
            continue

        if row_index == target_y:
            cells = [cell.strip() for cell in match.group(2).split(",")]
            if target_x >= len(cells):
                raise RelinkError(f"Region map x={target_x} is outside row width {len(cells)}.")
            if cells[target_x] != cell_plan["mapsec"]:
                cells[target_x] = cell_plan["mapsec"]
                newline = "\n" if line.endswith("\n") else ""
                updated_lines.append(f"{match.group(1)}{{{', '.join(cells)}}}{match.group(3).rstrip()}{newline}")
                changed = True
            else:
                updated_lines.append(line)
        else:
            updated_lines.append(line)
        row_index += 1

    if target_y >= row_index:
        raise RelinkError(f"Region map y={target_y} is outside row count {row_index}.")
    return "".join(updated_lines), changed


def fly_location_entry(region_map_type: str, mapsec: str, flag: str) -> str:
    return (
        "    {\n"
        f"        .regionMapType = {region_map_type},\n"
        f"        .mapsec = {mapsec},\n"
        f"        .flag = {flag},\n"
        "    },"
    )


def update_fly_locations(text: str, fly_plan: dict[str, str]) -> tuple[str, bool]:
    array_match = re.search(r"static const struct FlyLocation sFlyLocations\[\] =\n\{\n(?P<body>.*?)\n\};", text, re.DOTALL)
    if not array_match:
        raise RelinkError("Could not find sFlyLocations array in src/region_map.c.")

    body = array_match.group("body")
    entry_pattern = re.compile(
        r"    \{\n"
        r"        \.regionMapType = (?P<region>[^,]+),\n"
        r"        \.mapsec = (?P<mapsec>[^,]+),\n"
        r"        \.flag = (?P<flag>[^,]+),\n"
        r"    \},"
    )
    new_entry = fly_location_entry(fly_plan["regionMapType"], fly_plan["mapsec"], fly_plan["flag"])
    for match in entry_pattern.finditer(body):
        if match.group("mapsec") != fly_plan["mapsec"]:
            continue
        old_entry = match.group(0)
        if old_entry == new_entry:
            return text, False
        new_body = body[:match.start()] + new_entry + body[match.end():]
        return text[:array_match.start("body")] + new_body + text[array_match.end("body"):], True

    separator = "" if body.endswith("\n") else "\n"
    new_body = body + separator + new_entry
    return text[:array_match.start("body")] + new_body + text[array_match.end("body"):], True


def mapsec_map_entry(mapsec: str, map_id: str, heal_location: str) -> str:
    return f"    [{mapsec}] = {{MAP_GROUP({map_id}), MAP_NUM({map_id}), {heal_location}}},"


def update_mapsec_map_locations(text: str, mapsec_map: dict[str, str]) -> tuple[str, bool]:
    array_match = re.search(r"static const u8 sMapHealLocations\[\]\[3\] =\n\{\n(?P<body>.*?)\n\};", text, re.DOTALL)
    if not array_match:
        raise RelinkError("Could not find sMapHealLocations array in src/region_map.c.")

    body = array_match.group("body")
    mapsec = re.escape(mapsec_map["mapsec"])
    entry_pattern = re.compile(rf"    \[{mapsec}\] = \{{MAP_GROUP\([^)]+\), MAP_NUM\([^)]+\), [^}}]+\}},")
    new_entry = mapsec_map_entry(mapsec_map["mapsec"], mapsec_map["map"], mapsec_map["healLocation"])
    entry_match = entry_pattern.search(body)
    if entry_match:
        if entry_match.group(0) == new_entry:
            return text, False
        new_body = body[:entry_match.start()] + new_entry + body[entry_match.end():]
        return text[:array_match.start("body")] + new_body + text[array_match.end("body"):], True

    separator = "" if body.endswith("\n") else "\n"
    new_body = body + separator + new_entry
    return text[:array_match.start("body")] + new_body + text[array_match.end("body"):], True


def update_get_mapsec_type(text: str, type_plan: dict[str, str]) -> tuple[str, bool]:
    func_match = re.search(
        r"static u8 GetMapsecType\(mapsec_u16_t mapSecId\)\n\{\n(?P<body>.*?)\n\}\n\nmapsec_u16_t GetRegionMapSecIdAt",
        text,
        re.DOTALL,
    )
    if not func_match:
        raise RelinkError("Could not find GetMapsecType in src/region_map.c.")

    body = func_match.group("body")
    mapsec = re.escape(type_plan["mapsec"])
    case_pattern = re.compile(rf"    case {mapsec}:\n(?P<body>.*?)(?=\n    case |\n    default:)", re.DOTALL)
    new_case = (
        f"    case {type_plan['mapsec']}:\n"
        f"        return FlagGet({type_plan['flag']}) ? MAPSECTYPE_CITY_CANFLY : MAPSECTYPE_CITY_CANTFLY;"
    )
    case_match = case_pattern.search(body)
    if case_match:
        old_case = case_match.group(0)
        if old_case == new_case:
            return text, False
        new_body = body[:case_match.start()] + new_case + body[case_match.end():]
        return text[:func_match.start("body")] + new_body + text[func_match.end("body"):], True

    default_marker = "    default:\n        return MAPSECTYPE_ROUTE;"
    if default_marker not in body:
        raise RelinkError("Could not find GetMapsecType default route case.")
    new_body = body.replace(default_marker, f"{new_case}\n{default_marker}", 1)
    return text[:func_match.start("body")] + new_body + text[func_match.end("body"):], True


def find_fly_location_flag(text: str, mapsec_id: str) -> str | None:
    array_match = re.search(r"static const struct FlyLocation sFlyLocations\[\] =\n\{\n(?P<body>.*?)\n\};", text, re.DOTALL)
    if not array_match:
        return None
    entry_pattern = re.compile(
        r"    \{\n"
        r"        \.regionMapType = [^,]+,\n"
        rf"        \.mapsec = {re.escape(mapsec_id)},\n"
        r"        \.flag = (?P<flag>[^,]+),\n"
        r"    \},"
    )
    match = entry_pattern.search(array_match.group("body"))
    return match.group("flag").strip() if match else None


def update_simple_mapsec_array(text: str, array_name: str, mapsec_id: str, enabled: bool) -> tuple[str, bool]:
    array_match = re.search(rf"static const mapsec_u16_t {array_name}\[\] =\n\{{\n(?P<body>.*?)\n\}};", text, re.DOTALL)
    if not array_match:
        if enabled:
            raise RelinkError(f"Could not find {array_name} array in src/region_map.c.")
        return text, False

    body = array_match.group("body")
    entry_pattern = re.compile(rf"^    {re.escape(mapsec_id)},\n?", re.MULTILINE)
    entry_match = entry_pattern.search(body)
    if enabled:
        if entry_match:
            return text, False
        sentinel = re.search(r"^    MAPSEC_NONE,?\n?", body, re.MULTILINE)
        if not sentinel:
            raise RelinkError(f"Could not find MAPSEC_NONE sentinel in {array_name}.")
        new_body = body[:sentinel.start()] + f"    {mapsec_id},\n" + body[sentinel.start():]
        return text[:array_match.start("body")] + new_body + text[array_match.end("body"):], True

    if not entry_match:
        return text, False
    new_body = body[:entry_match.start()] + body[entry_match.end():]
    return text[:array_match.start("body")] + new_body + text[array_match.end("body"):], True


def red_outline_entry(flag: str, mapsec_id: str) -> str:
    return (
        "    {\n"
        f"        {flag},\n"
        f"        {mapsec_id}\n"
        "    },"
    )


def update_red_outline_destinations(text: str, mapsec_id: str, flag: str | None, enabled: bool) -> tuple[str, bool]:
    array_match = re.search(r"static const mapsec_u16_t sRedOutlineFlyDestinations\[\]\[2\] =\n\{\n(?P<body>.*?)\n\};", text, re.DOTALL)
    if not array_match:
        if enabled:
            raise RelinkError("Could not find sRedOutlineFlyDestinations array in src/region_map.c.")
        return text, False

    body = array_match.group("body")
    entry_pattern = re.compile(
        r"    \{\n"
        r"        (?P<flag>[^,]+),\n"
        rf"        {re.escape(mapsec_id)}\n"
        r"    \},"
    )
    entry_match = entry_pattern.search(body)
    if enabled:
        if flag is None:
            flag = find_fly_location_flag(text, mapsec_id)
        if flag is None:
            raise RelinkError(f"Cannot infer red-outline flag for {mapsec_id}; pass MAPSEC_ID:red-outline:FLAG_*.")
        new_entry = red_outline_entry(flag, mapsec_id)
        if entry_match:
            if entry_match.group(0) == new_entry:
                return text, False
            new_body = body[:entry_match.start()] + new_entry + body[entry_match.end():]
            return text[:array_match.start("body")] + new_body + text[array_match.end("body"):], True
        sentinel = re.search(r"    \{\n        -1,\n        MAPSEC_NONE\n    \},?", body)
        if not sentinel:
            raise RelinkError("Could not find sRedOutlineFlyDestinations sentinel.")
        new_body = body[:sentinel.start()] + new_entry + "\n" + body[sentinel.start():]
        return text[:array_match.start("body")] + new_body + text[array_match.end("body"):], True

    if not entry_match:
        return text, False
    new_body = body[:entry_match.start()] + body[entry_match.end():]
    return text[:array_match.start("body")] + new_body + text[array_match.end("body"):], True


def update_fly_icon_style(text: str, style_plan: dict[str, str | None]) -> tuple[str, bool]:
    mapsec_id = str(style_plan["mapsec"])
    style = style_plan["style"]
    changed = False

    if style == "palette-blink":
        text, step_changed = update_simple_mapsec_array(text, "sPaletteBlinkFlyDestinations", mapsec_id, True)
        changed |= step_changed
        text, step_changed = update_red_outline_destinations(text, mapsec_id, None, False)
        changed |= step_changed
    elif style == "red-outline":
        text, step_changed = update_simple_mapsec_array(text, "sPaletteBlinkFlyDestinations", mapsec_id, False)
        changed |= step_changed
        text, step_changed = update_red_outline_destinations(text, mapsec_id, style_plan.get("flag"), True)
        changed |= step_changed
    elif style == "stock":
        text, step_changed = update_simple_mapsec_array(text, "sPaletteBlinkFlyDestinations", mapsec_id, False)
        changed |= step_changed
        text, step_changed = update_red_outline_destinations(text, mapsec_id, None, False)
        changed |= step_changed
    else:
        raise RelinkError(f"Unsupported Fly icon style {style!r}.")
    return text, changed


def update_flag_claim(text: str, claim: dict[str, str]) -> tuple[str, bool]:
    new_flag_exists = re.search(rf"^#define\s+{re.escape(claim['newFlag'])}\b", text, re.MULTILINE)
    pattern = re.compile(rf"^#define\s+{re.escape(claim['oldFlag'])}\s+(?P<value>\([^)]+\)|\S+)(?:\s*//.*)?$", re.MULTILINE)
    match = pattern.search(text)
    if not match:
        if new_flag_exists:
            return text, False
        raise RelinkError(f"Could not find unused flag {claim['oldFlag']} in include/constants/flags.h.")
    if new_flag_exists and claim["oldFlag"] != claim["newFlag"]:
        raise RelinkError(f"Cannot claim {claim['oldFlag']} as {claim['newFlag']}; {claim['newFlag']} already exists.")
    replacement = f"#define {claim['newFlag']:<45} {match.group('value')} // Claimed by map_asset_relinker"
    return text[:match.start()] + replacement + text[match.end():], True


def update_transition_setflag(text: str, map_name: str, flag: str, fallback_prefixes: Iterable[str] = ()) -> tuple[str, bool]:
    prefixes = [prefix for prefix in [map_name, *fallback_prefixes] if prefix]
    header = ""
    header_index = -1
    for prefix in dict.fromkeys(prefixes):
        header = f"{prefix}_MapScripts::"
        header_index = text.find(header)
        if header_index != -1:
            map_name = prefix
            break

    if header_index == -1:
        match = re.search(r"^([A-Za-z0-9_]+)_MapScripts::", text, re.MULTILINE)
        if not match:
            expected = ", ".join(f"{prefix}_MapScripts::" for prefix in dict.fromkeys(prefixes))
            raise RelinkError(f"Could not find map script header; expected one of: {expected}.")
        map_name = match.group(1)
        header = match.group(0)
        header_index = match.start()

    byte_index = text.find("\t.byte 0", header_index)
    if byte_index == -1:
        raise RelinkError(f"Could not find map script terminator for {map_name}.")

    script_table = text[header_index:byte_index]
    transition_match = re.search(r"^\tmap_script\s+MAP_SCRIPT_ON_TRANSITION,\s*(?P<label>[A-Za-z0-9_]+)\s*$", script_table, re.MULTILINE)
    if transition_match:
        label = transition_match.group("label")
    else:
        label = f"{map_name}_OnTransition"
        insert_at = text.find("\n", header_index) + 1
        text = text[:insert_at] + f"\tmap_script MAP_SCRIPT_ON_TRANSITION, {label}\n" + text[insert_at:]

    label_pattern = re.compile(rf"^{re.escape(label)}::?\n", re.MULTILINE)
    label_match = label_pattern.search(text)
    if label_match:
        insert_at = label_match.end()
        next_label = re.search(r"^[A-Za-z0-9_]+::?\n", text[insert_at:], re.MULTILINE)
        label_end = insert_at + next_label.start() if next_label else len(text)
        if f"setflag {flag}" in text[insert_at:label_end]:
            return text, False
        text = text[:insert_at] + f"\tsetflag {flag}\n" + text[insert_at:]
    else:
        if not text.endswith("\n"):
            text += "\n"
        text += f"\n{label}:\n\tsetflag {flag}\n\tend\n"
    return text, True


def update_event_scripts(text: str, old_name: str, new_name: str) -> tuple[str, bool]:
    old = f'.include "data/maps/{old_name}/scripts.inc"'
    new = f'.include "data/maps/{new_name}/scripts.inc"'
    changed = False
    if old != new and old in text:
        text = text.replace(old, new)
        changed = True

    lines = text.splitlines(keepends=True)
    if not lines:
        return text, changed

    seen = False
    normalized_lines: list[str] = []
    for line in lines:
        if line.strip() == new:
            if seen:
                changed = True
                continue
            seen = True
        normalized_lines.append(line)
    new_text = "".join(normalized_lines)
    return new_text, changed


def update_script_label_prefixes(text: str, old_prefixes: Iterable[str], new_prefix: str) -> tuple[str, bool]:
    changed = False
    for old_prefix in sorted({prefix for prefix in old_prefixes if prefix and prefix != new_prefix}, key=len, reverse=True):
        pattern = re.compile(rf"(?<![A-Za-z0-9_]){re.escape(old_prefix)}(?=_)")
        updated_lines: list[str] = []
        for line in text.splitlines(keepends=True):
            # Do not rewrite literal dialogue strings when script labels share
            # a prefix with user-visible text.
            if line.lstrip().startswith(".string"):
                updated_lines.append(line)
                continue
            updated_line = pattern.sub(new_prefix, line)
            if updated_line != line:
                changed = True
            updated_lines.append(updated_line)
        text = "".join(updated_lines)
    return text, changed


def iter_all_map_json(root: Path) -> Iterable[Path]:
    for path in iter_map_dirs(root):
        yield path / "map.json"


def scan_text_refs(root: Path, tokens: list[str]) -> list[str]:
    matches: list[str] = []
    patterns = [
        re.compile(rf"(?<![A-Za-z0-9]){re.escape(token)}(?![A-Za-z0-9])")
        for token in tokens
    ]
    search_roots = [repo_path(root, "data/maps"), repo_path(root, "data/layouts"), repo_path(root, "data/event_scripts.s")]
    for search_root in search_roots:
        if not search_root.exists():
            continue
        files = [search_root] if search_root.is_file() else search_root.rglob("*")
        for path in files:
            if not path.is_file() or path.suffix not in {".json", ".inc", ".s"}:
                continue
            if is_generated_source(path, root):
                continue
            try:
                text = read_text(path)
            except UnicodeDecodeError:
                continue
            if any(pattern.search(text) for pattern in patterns):
                matches.append(rel(path, root))
    return sorted(set(matches))


def is_generated_source(path: Path, root: Path) -> bool:
    try:
        relative = path.relative_to(root)
    except ValueError:
        return False
    if relative in KNOWN_GENERATED_OUTPUTS:
        return True
    if relative.parts[:2] == ("data", "maps") and relative.name in {"header.inc", "events.inc", "connections.inc"}:
        return True
    return False


def move_path(root: Path, src_rel: str, dst_rel: str, dry_run: bool) -> None:
    if not validate_move_path(root, src_rel, dst_rel):
        return
    print(f"MOVE {src_rel} -> {dst_rel}")
    if dry_run:
        return
    src = repo_path(root, src_rel)
    dst = repo_path(root, dst_rel)
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.move(str(src), str(dst))


def validate_move_path(root: Path, src_rel: str, dst_rel: str) -> bool:
    src = repo_path(root, src_rel)
    dst = repo_path(root, dst_rel)
    if src == dst:
        return False
    if not src.exists():
        raise RelinkError(f"Cannot move missing path {src_rel}.")
    if dst.exists():
        raise RelinkError(f"Destination already exists: {dst_rel}.")
    return True


def planned_move_paths(plan: dict[str, Any]) -> list[tuple[str, str]]:
    moves: list[tuple[str, str]] = []
    for layout in plan.get("layouts", []):
        old_dir = layout.get("oldDir")
        new_dir = layout.get("newDir")
        if old_dir and new_dir and old_dir != new_dir:
            moves.append((old_dir, new_dir))
    for map_plan in plan.get("maps", []):
        old_dir_name = map_plan.get("oldDirName", map_plan["oldName"])
        new_dir_name = map_plan.get("newDirName", map_plan["newName"])
        old_dir = (MAPS_DIR / old_dir_name).as_posix()
        new_dir = (MAPS_DIR / new_dir_name).as_posix()
        if old_dir != new_dir:
            moves.append((old_dir, new_dir))
    return moves


def validate_planned_moves(root: Path, plan: dict[str, Any]) -> None:
    destinations: set[str] = set()
    for src_rel, dst_rel in planned_move_paths(plan):
        if dst_rel in destinations:
            raise RelinkError(f"Multiple moves target the same destination: {dst_rel}.")
        destinations.add(dst_rel)
        validate_move_path(root, src_rel, dst_rel)


def apply_plan(root: Path, plan: dict[str, Any], dry_run: bool, allow_dirty: bool, backup_root: str | None) -> None:
    if not allow_dirty and not dry_run:
        check_dirty(root, target_paths_from_plan(root, plan))

    for path in target_paths_from_plan(root, plan):
        ensure_not_generated(path, root)

    validate_planned_moves(root, plan)

    if not dry_run:
        create_backup_archive(root, plan, backup_root)

    layout_by_old_id = {layout["oldId"]: layout for layout in plan.get("layouts", [])}
    layout_renames = {
        layout["oldId"]: layout["newId"]
        for layout in plan.get("layouts", [])
        if layout.get("oldId") and layout.get("newId") and layout["oldId"] != layout["newId"]
    }

    # Moves first, so written JSON lands in final paths.
    for layout in plan.get("layouts", []):
        old_dir = layout.get("oldDir")
        new_dir = layout.get("newDir")
        if old_dir and new_dir:
            move_path(root, old_dir, new_dir, dry_run)

    for map_plan in plan.get("maps", []):
        old_dir_name = map_plan.get("oldDirName", map_plan["oldName"])
        new_dir_name = map_plan.get("newDirName", map_plan["newName"])
        move_path(root, (MAPS_DIR / old_dir_name).as_posix(), (MAPS_DIR / new_dir_name).as_posix(), dry_run)

    groups_path = repo_path(root, MAP_GROUPS)
    groups_data = load_json(groups_path)
    groups_changed = False
    for map_plan in plan.get("maps", []):
        groups_changed |= update_map_groups(
            groups_data,
            map_plan.get("groupOldNames", [map_plan["oldName"]]),
            map_plan["newName"],
            map_plan.get("fromGroup"),
            map_plan.get("toGroup"),
            map_plan.get("group"),
        )
    groups_changed |= drop_empty_map_groups(groups_data, plan.get("dropGroups", []))
    if groups_changed:
        print(f"EDIT {MAP_GROUPS}")
        if not dry_run:
            write_json(groups_path, groups_data)

    mapsec_renames = {mapsec["oldId"]: mapsec["newId"] for mapsec in plan.get("mapsecs", [])}
    if plan.get("mapsecs"):
        mapsecs_path = repo_path(root, REGION_MAP_SECTIONS_JSON)
        mapsecs_data = load_json(mapsecs_path)
        mapsecs_changed = False
        for mapsec_plan in plan.get("mapsecs", []):
            mapsecs_changed |= update_mapsecs_json(mapsecs_data, mapsec_plan)
        if mapsecs_changed:
            print(f"EDIT {REGION_MAP_SECTIONS_JSON}")
            if not dry_run:
                write_json(mapsecs_path, mapsecs_data)

    layouts_path = repo_path(root, LAYOUTS_JSON)
    layouts_data = load_json(layouts_path)
    layouts_changed = False
    for layout_plan in plan.get("layouts", []):
        layouts_changed |= update_layouts_json(layouts_data, layout_plan)
    if layouts_changed:
        print(f"EDIT {LAYOUTS_JSON}")
        if not dry_run:
            write_json(layouts_path, layouts_data)

    for map_plan in plan.get("maps", []):
        new_name = map_plan["newName"]
        old_dir_name = map_plan.get("oldDirName", map_plan["oldName"])
        new_dir_name = map_plan.get("newDirName", new_name)
        old_ids = set(map_plan.get("oldIds", [map_plan["oldId"]]))
        new_id = map_plan["newId"]
        map_json_path = repo_path(root, MAPS_DIR / (new_dir_name if not dry_run else old_dir_name) / "map.json")
        map_data = load_json(map_json_path)
        new_layout = map_plan.get("newLayoutId")
        if new_layout is None:
            old_layout = map_data.get("layout")
            if old_layout in layout_by_old_id:
                new_layout = layout_by_old_id[old_layout]["newId"]
        map_changed = update_map_json(
            map_data,
            old_ids,
            new_id,
            new_name,
            new_layout,
            map_plan.get("newMapsec"),
            map_plan.get("mapType"),
            map_plan.get("showMapName"),
        )
        map_changed |= update_map_refs(map_data, old_ids, new_id)
        for old_mapsec, new_mapsec in mapsec_renames.items():
            map_changed |= update_mapsec_refs(map_data, old_mapsec, new_mapsec)
        if map_changed:
            print(f"EDIT {MAPS_DIR / new_dir_name / 'map.json'}")
            if not dry_run:
                write_json(repo_path(root, MAPS_DIR / new_dir_name / "map.json"), map_data)

        if plan.get("options", {}).get("rewriteScriptLabels") or map_plan.get("transitionSetFlag"):
            script_path = repo_path(root, MAPS_DIR / (new_dir_name if not dry_run else old_dir_name) / "scripts.inc")
            if script_path.exists():
                script_text = read_text(script_path)
                script_changed = False
                if plan.get("options", {}).get("rewriteScriptLabels"):
                    script_text, changed = update_script_label_prefixes(
                        script_text,
                        map_plan.get("scriptOldPrefixes", [old_dir_name, map_plan["oldName"]]),
                        new_name,
                    )
                    script_changed |= changed
                if map_plan.get("transitionSetFlag"):
                    script_text, changed = update_transition_setflag(
                        script_text,
                        new_name,
                        map_plan["transitionSetFlag"],
                        [old_dir_name, map_plan["oldName"]],
                    )
                    script_changed |= changed
                if script_changed:
                    print(f"EDIT {MAPS_DIR / new_dir_name / 'scripts.inc'}")
                    if not dry_run:
                        write_text(repo_path(root, MAPS_DIR / new_dir_name / "scripts.inc"), script_text)

        event_scripts = repo_path(root, EVENT_SCRIPTS)
        if event_scripts.exists():
            new_text, changed = update_event_scripts(read_text(event_scripts), old_dir_name, new_dir_name)
            if changed:
                print(f"EDIT {EVENT_SCRIPTS}")
                if not dry_run:
                    write_text(event_scripts, new_text)

        for map_json in iter_all_map_json(root):
            if map_json.parent.name == (old_dir_name if dry_run else new_dir_name):
                continue
            data = load_json(map_json)
            refs_changed = update_map_refs(data, old_ids, new_id)
            refs_changed |= update_layout_refs(data, layout_renames)
            for old_mapsec, new_mapsec in mapsec_renames.items():
                refs_changed |= update_mapsec_refs(data, old_mapsec, new_mapsec)
            if refs_changed:
                print(f"EDIT {rel(map_json, root)}")
                if not dry_run:
                    write_json(map_json, data)

    region_layouts: dict[str, str] = {}
    region_layout_changed: dict[str, bool] = {}
    for cell_plan in plan.get("regionMapCells", []):
        region = cell_plan["region"]
        layout_path = REGION_MAP_LAYOUT_FILES[region]
        if region not in region_layouts:
            region_layouts[region] = read_text(repo_path(root, layout_path))
            region_layout_changed[region] = False
        region_layouts[region], changed = update_region_map_layout(region_layouts[region], cell_plan)
        region_layout_changed[region] |= changed
    for region, changed in region_layout_changed.items():
        if changed:
            layout_path = REGION_MAP_LAYOUT_FILES[region]
            print(f"EDIT {layout_path}")
            if not dry_run:
                write_text(repo_path(root, layout_path), region_layouts[region])

    if plan.get("flagClaims"):
        flags_path = repo_path(root, FLAGS_H)
        flags_text = read_text(flags_path)
        flags_changed = False
        for claim in plan.get("flagClaims", []):
            flags_text, changed = update_flag_claim(flags_text, claim)
            flags_changed |= changed
        if flags_changed:
            print(f"EDIT {FLAGS_H}")
            if not dry_run:
                write_text(flags_path, flags_text)

    if plan.get("flyLocations") or plan.get("flyMapsecTypes") or plan.get("flyIconStyles") or plan.get("mapsecMaps"):
        region_map_c = repo_path(root, REGION_MAP_C)
        region_map_text = read_text(region_map_c)
        fly_changed = False
        for mapsec_map in plan.get("mapsecMaps", []):
            region_map_text, changed = update_mapsec_map_locations(region_map_text, mapsec_map)
            fly_changed |= changed
        for fly_plan in plan.get("flyLocations", []):
            region_map_text, changed = update_fly_locations(region_map_text, fly_plan)
            fly_changed |= changed
        for type_plan in plan.get("flyMapsecTypes", []):
            region_map_text, changed = update_get_mapsec_type(region_map_text, type_plan)
            fly_changed |= changed
        for style_plan in plan.get("flyIconStyles", []):
            region_map_text, changed = update_fly_icon_style(region_map_text, style_plan)
            fly_changed |= changed
        if fly_changed:
            print(f"EDIT {REGION_MAP_C}")
            if not dry_run:
                write_text(region_map_c, region_map_text)

    tokens = []
    for map_plan in plan.get("maps", []):
        tokens.extend(
            token
            for token in map_plan.get("groupOldNames", [])
            if token and token not in {map_plan["newName"], map_plan.get("newDirName")}
        )
        tokens.extend(
            token
            for token in map_plan.get("oldIds", [map_plan["oldId"]])
            if token and token != map_plan["newId"]
        )
    for layout_plan in plan.get("layouts", []):
        tokens.extend(
            token
            for token in (layout_plan["oldId"], layout_plan.get("oldName", ""))
            if token and token not in {layout_plan["newId"], layout_plan.get("newName")}
        )
    for mapsec_plan in plan.get("mapsecs", []):
        if mapsec_plan["oldId"] != mapsec_plan["newId"]:
            tokens.append(mapsec_plan["oldId"])
    tokens.extend(plan.get("dropGroups", []))
    remaining = scan_text_refs(root, [token for token in tokens if token])
    if remaining:
        print("REVIEW remaining textual references:")
        for path in remaining:
            print(f"  {path}")


def command_audit(args: argparse.Namespace) -> int:
    root = Path(args.root).resolve()
    return print_diagnostics(collect_audit(root, args.target))


def command_plan(args: argparse.Namespace) -> int:
    root = Path(args.root).resolve()
    plan = make_plan(args)
    save_plan(plan, args.out)
    return 0


def make_temp_mapsec_plan_args(args: argparse.Namespace) -> argparse.Namespace:
    root = Path(args.root).resolve()
    map_ref = find_map_ref(root, args.map, args.match_by)
    current_mapsec = map_ref.data.get("region_map_section")
    if not isinstance(current_mapsec, str) or not current_mapsec.startswith("MAPSEC_"):
        raise RelinkError(f"Map {args.map!r} does not reference a MAPSEC_* region map section.")
    if current_mapsec in SPECIAL_MAPSECS:
        raise RelinkError(f"Map {args.map!r} references special mapsec {current_mapsec!r}; pass explicit plan options instead.")
    mapsec_entries = load_mapsec_entries(root)
    if current_mapsec not in mapsec_entries:
        raise RelinkError(f"Region map section {current_mapsec!r} was not found.")

    inferred_map = title_identifier_from_mapsec(current_mapsec)
    new_map = args.new_map_name or inferred_map
    if not new_map:
        raise RelinkError(f"Could not infer a production map name from {current_mapsec!r}; pass --new-map-name.")
    new_mapsec = args.new_mapsec or normalize_mapsec_id(current_mapsec)
    new_mapsec_name = args.new_mapsec_name or display_name_from_mapsec(new_mapsec)
    rename_mapsec = f"{current_mapsec}:{new_mapsec}" if current_mapsec != new_mapsec else None
    set_mapsec_name = None if rename_mapsec else f"{current_mapsec}:{new_mapsec_name}"
    to_group = args.to_group or f"gMapGroup_{title_identifier_from_mapsec(current_mapsec) or new_map}"

    return argparse.Namespace(
        root=args.root,
        map=f"{args.map}:{new_map}",
        match_by=args.match_by,
        new_map_dir=None,
        old_group_map_name=None,
        from_group=None,
        to_group=to_group,
        group=None,
        old_map_id=None,
        new_map_id=None,
        set_map_type=None,
        set_show_map_name=None,
        set_transition_setflag=None,
        new_mapsec=new_mapsec if not rename_mapsec else None,
        rename_mapsec=rename_mapsec,
        new_mapsec_name=new_mapsec_name if rename_mapsec else None,
        set_mapsec_name=set_mapsec_name,
        set_mapsec_bounds=None,
        set_region_map_cell=None,
        ensure_fly_location=None,
        ensure_fly_mapsec_type=None,
        set_fly_icon_style=None,
        ensure_mapsec_map=None,
        claim_unused_flag=None,
        set_layout_id=None,
        drop_group=None,
        rewrite_script_labels=not args.no_rewrite_script_labels,
        old_script_prefix=None,
        no_layout_rename=args.no_layout_rename,
        old_layout_id=None,
        new_layout_id=None,
        new_layout_name=None,
        set_layout_name=None,
        old_layout_dir=None,
        new_layout_dir=None,
        set_primary_tileset=None,
        set_secondary_tileset=None,
        out=args.out,
    )


def command_plan_temp_mapsec(args: argparse.Namespace) -> int:
    plan_args = make_temp_mapsec_plan_args(args)
    plan = make_plan(plan_args)
    if args.dry_run:
        root = Path(args.root).resolve()
        out = args.out
        if not out:
            out = str(Path(tempfile.gettempdir()) / f"{safe_temp_name(plan['maps'][0]['newName'])}_temp_mapsec_repair.json")
        save_plan(plan, out)
        apply_plan(root, plan, dry_run=True, allow_dirty=False, backup_root=None)
        print("Dry-run complete; no files changed.")
        return 0
    save_plan(plan, args.out)
    return 0


def command_apply(args: argparse.Namespace) -> int:
    root = Path(args.root).resolve()
    plan = load_plan(Path(args.plan))
    apply_plan(root, plan, args.dry_run, args.allow_dirty, args.backup_root)
    if args.dry_run:
        print("Dry-run complete; no files changed.")
    else:
        print("Apply complete.")
    return 0


def command_validate(args: argparse.Namespace) -> int:
    root = Path(args.root).resolve()
    return print_diagnostics(collect_audit(root, args.target))


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", default=".", help="Repository root. Defaults to current directory.")
    subparsers = parser.add_subparsers(dest="command", required=True)

    audit = subparsers.add_parser("audit", help="Audit map/layout linkage without modifying files.")
    audit.add_argument("--target", help="Also report files containing this token.")
    audit.set_defaults(func=command_audit)

    plan = subparsers.add_parser("plan", help="Create a reviewable relink plan.")
    plan.add_argument("--map", required=True, help="Map rename in OLD:NEW format, e.g. RougeCave_2:RougeCave_2F.")
    plan.add_argument("--match-by", choices=("dir", "name", "id"), default="dir", help="How to find OLD from --map. Defaults to directory name for repair-friendly plans.")
    plan.add_argument("--new-map-dir", help="Override the destination data/maps directory name. Defaults to NEW from --map.")
    plan.add_argument("--old-group-map-name", action="append", help="Additional old map name to remove from map_groups.json when repairing a typo. May be repeated.")
    plan.add_argument("--from-group", help="Only remove/rename the old map entry from this source map group.")
    plan.add_argument("--to-group", help="Move the renamed map entry into this target map group, creating it if missing.")
    plan.add_argument("--group", help="Preferred map group if the old map is not already grouped.")
    plan.add_argument("--old-map-id", help="Additional old MAP_* id to rewrite when repairing id mismatches.")
    plan.add_argument("--new-map-id", help="Override generated new MAP_* id.")
    plan.add_argument("--set-map-type", help="Set map.json map_type to a MAP_TYPE_* value.")
    plan.add_argument("--set-show-map-name", help="Set map.json show_map_name to true or false.")
    plan.add_argument("--set-transition-setflag", help="Ensure the map's MAP_SCRIPT_ON_TRANSITION sets this FLAG_* value.")
    plan.add_argument("--new-mapsec", help="Set region_map_section on the renamed map.")
    plan.add_argument("--rename-mapsec", help="Rename a region map section id in OLD:NEW format and rewrite map references.")
    plan.add_argument("--new-mapsec-name", help="Set the display name while using --rename-mapsec.")
    plan.add_argument("--set-mapsec-name", help="Set an existing region map section display name in MAPSEC_ID:NAME format.")
    plan.add_argument("--set-mapsec-bounds", action="append", help="Set mapsec town-map bounds in MAPSEC_ID:X:Y:WIDTH:HEIGHT format. May be repeated.")
    plan.add_argument("--set-region-map-cell", action="append", help="Set a town-map grid cell in REGION:X:Y:MAPSEC_ID format. REGION is hoenn, kanto, sevii123, sevii45, or sevii67. May be repeated.")
    plan.add_argument("--ensure-fly-location", action="append", help="Add or update a Fly destination in REGION:MAPSEC_ID:FLAG format. May be repeated.")
    plan.add_argument("--ensure-fly-mapsec-type", action="append", help="Ensure GetMapsecType treats MAPSEC_ID as a flag-gated Fly target, in MAPSEC_ID:FLAG format. May be repeated.")
    plan.add_argument("--set-fly-icon-style", action="append", help="Set Fly icon animation style in MAPSEC_ID:STYLE[:FLAG] format. STYLE is stock, palette-blink/blue-blink, or red-outline. May be repeated.")
    plan.add_argument("--ensure-mapsec-map", action="append", help="Ensure sMapHealLocations maps MAPSEC_ID to MAP_ID and optional HEAL_LOCATION_ID, in MAPSEC_ID:MAP_ID[:HEAL_LOCATION_ID] format. May be repeated.")
    plan.add_argument("--claim-unused-flag", action="append", help="Rename an existing unused FLAG_* define to a new FLAG_* define in OLD:NEW format. May be repeated.")
    plan.add_argument("--set-layout-id", help="Set map.json layout without renaming the layout entry. Useful when a map accidentally points at another map's layout.")
    plan.add_argument("--drop-group", action="append", help="Drop an empty bad map group from map_groups.json. May be repeated.")
    plan.add_argument("--rewrite-script-labels", action="store_true", help="Rewrite script label prefixes in the selected map's scripts.inc. Literal .string lines are not rewritten.")
    plan.add_argument("--old-script-prefix", action="append", help="Additional old script label prefix to rewrite when --rewrite-script-labels is set. May be repeated.")
    plan.add_argument("--no-layout-rename", action="store_true", help="Keep the existing layout id/name/path.")
    plan.add_argument("--old-layout-id", help="Override the old layout id when map.json points at the wrong layout.")
    plan.add_argument("--new-layout-id", help="Override generated new LAYOUT_* id.")
    plan.add_argument("--new-layout-name", help="Override generated new layout label.")
    plan.add_argument("--set-layout-name", help="Set the selected layout label without otherwise changing layout id/path. Pair with --no-layout-rename for metadata-only repair.")
    plan.add_argument("--old-layout-dir", help="Override old layout directory.")
    plan.add_argument("--new-layout-dir", help="Override new layout directory.")
    plan.add_argument("--set-primary-tileset", help="Set primary_tileset on the selected layout without otherwise changing the layout.")
    plan.add_argument("--set-secondary-tileset", help="Set secondary_tileset on the selected layout without otherwise changing the layout.")
    plan.add_argument("--out", help="Write plan JSON to this path instead of stdout.")
    plan.set_defaults(func=command_plan)

    temp_mapsec = subparsers.add_parser("plan-temp-mapsec", help="Create or dry-run a mapsec-derived repair plan for a temporary map.")
    temp_mapsec.add_argument("--map", required=True, help="Temporary map to repair, e.g. test1.")
    temp_mapsec.add_argument("--match-by", choices=("dir", "name", "id"), default="dir", help="How to find --map. Defaults to directory name.")
    temp_mapsec.add_argument("--new-map-name", help="Override the production map name inferred from region_map_section.")
    temp_mapsec.add_argument("--to-group", help="Override the target map group. Defaults to gMapGroup_<MapsecSuffix> and creates the group if missing.")
    temp_mapsec.add_argument("--new-mapsec", help="Override the normalized MAPSEC_* id inferred from the current region_map_section.")
    temp_mapsec.add_argument("--new-mapsec-name", help="Override the mapsec display name inferred from the normalized MAPSEC_* id.")
    temp_mapsec.add_argument("--no-layout-rename", action="store_true", help="Keep the existing layout id/name/path.")
    temp_mapsec.add_argument("--no-rewrite-script-labels", action="store_true", help="Do not rewrite temporary script label prefixes in scripts.inc.")
    temp_mapsec.add_argument("--dry-run", action="store_true", help="Write the inferred plan and immediately run apply --dry-run.")
    temp_mapsec.add_argument("--out", help="Write plan JSON to this path. With --dry-run, defaults to /tmp/<name>_temp_mapsec_repair.json.")
    temp_mapsec.set_defaults(func=command_plan_temp_mapsec)

    apply = subparsers.add_parser("apply", help="Apply or dry-run a relink plan.")
    apply.add_argument("plan", help="Path to plan JSON.")
    apply.add_argument("--dry-run", action="store_true", help="Print planned changes without modifying files.")
    apply.add_argument("--allow-dirty", action="store_true", help="Allow applying over dirty target files.")
    apply.add_argument("--backup-root", help="Directory for automatic .bak.tar archives. Defaults to .map_asset_relinker_backups.")
    apply.set_defaults(func=command_apply)

    validate = subparsers.add_parser("validate", help="Run post-apply consistency checks.")
    validate.add_argument("--target", help="Also report files containing this token.")
    validate.set_defaults(func=command_validate)
    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    try:
        return args.func(args)
    except RelinkError as err:
        print(f"ERROR: {err}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
