#!/usr/bin/env python3
"""Audit and relink Porymap-style map/layout assets.

This tool intentionally works on repo source files, not on Porymap internals.
It updates structured map JSON, layout JSON, map group JSON, exact script
include paths, and map references in warps/connections.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable


MAPS_DIR = Path("data/maps")
LAYOUTS_DIR = Path("data/layouts")
MAP_GROUPS = MAPS_DIR / "map_groups.json"
LAYOUTS_JSON = LAYOUTS_DIR / "layouts.json"
EVENT_SCRIPTS = Path("data/event_scripts.s")
REGION_MAP_SECTIONS_JSON = Path("src/data/region_map/region_map_sections.json")
SPECIAL_MAP_IDS = {"MAP_DYNAMIC", "MAP_NONE"}
SPECIAL_MAPSECS = {"MAPSEC_DYNAMIC", "MAPSEC_NONE"}

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


def parse_pair(raw: str, label: str) -> tuple[str, str]:
    if ":" not in raw:
        raise RelinkError(f"{label} must use OLD:NEW format.")
    old, new = raw.split(":", 1)
    if not old or not new:
        raise RelinkError(f"{label} must use non-empty OLD:NEW values.")
    return old, new


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
        seen_groups.setdefault(group, []).append(f"group_order[{index}]")
        maps = groups_data.get(group)
        if not isinstance(maps, list):
            diagnostics.append(Diagnostic("error", f"map_groups.json lists group {group!r}, but that field is missing or not a list."))
            continue
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
        if "LAYOUT_" in name or name.endswith("_Layout_Layout"):
            diagnostics.append(Diagnostic("warning", f"layout {layout_id!r} has suspicious label name {name!r}."))

    for layout_id, count in sorted(layout_id_counts.items()):
        if count > 1:
            diagnostics.append(Diagnostic("error", f"layout id {layout_id!r} appears {count} times."))

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
        included_maps = set(re.findall(r'\.include\s+"data/maps/([^"]+)/scripts\.inc"', text))
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


def find_map_ref(root: Path, old_name: str) -> MapRef:
    maps = build_map_index(root)
    if old_name not in maps:
        raise RelinkError(f"Map {old_name!r} was not found in {MAPS_DIR}.")
    return maps[old_name]


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
    map_ref = find_map_ref(root, old_map)
    old_map_id = map_ref.data.get("id")
    new_map_id = args.new_map_id or f"MAP_{camel_to_upper_snake(new_map)}"
    old_layout_id = map_ref.data.get("layout")
    layout_ref = find_layout_ref(root, old_layout_id)

    layout_plan = None
    if not args.no_layout_rename:
        new_layout_id = args.new_layout_id or f"LAYOUT_{camel_to_upper_snake(new_map)}"
        old_layout_name = layout_ref.data.get("name")
        new_layout_name = args.new_layout_name or f"{new_map}_Layout"
        old_layout_dir = args.old_layout_dir or default_layout_dir(layout_ref.data)
        new_layout_dir = args.new_layout_dir or f"{LAYOUTS_DIR.as_posix()}/{new_map}"
        layout_plan = {
            "oldId": old_layout_id,
            "newId": new_layout_id,
            "oldName": old_layout_name,
            "newName": new_layout_name,
            "oldDir": old_layout_dir,
            "newDir": new_layout_dir,
        }

    groups = map_ref.groups
    from_group = args.from_group
    to_group = args.to_group
    if from_group and from_group not in groups:
        raise RelinkError(f"Map {old_map!r} is not listed in source group {from_group!r}.")
    group = to_group or args.group or from_group or (groups[0] if groups else None)
    plan: dict[str, Any] = {
        "version": 1,
        "maps": [
            {
                "oldName": old_map,
                "newName": new_map,
                "oldId": old_map_id,
                "newId": new_map_id,
                "group": group,
                "fromGroup": from_group,
                "toGroup": to_group,
            }
        ],
        "layouts": [layout_plan] if layout_plan else [],
        "options": {
            "rewriteScriptLabels": False,
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
    for map_plan in plan.get("maps", []):
        old_name = map_plan["oldName"]
        new_name = map_plan["newName"]
        paths.append(repo_path(root, MAPS_DIR / old_name / "map.json"))
        paths.append(repo_path(root, MAPS_DIR / new_name / "map.json"))
        paths.append(repo_path(root, MAPS_DIR / old_name / "scripts.inc"))
        paths.append(repo_path(root, MAPS_DIR / new_name / "scripts.inc"))
    return paths


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


def remove_map_from_group(maps: list[Any], map_name: str) -> tuple[bool, int | None]:
    changed = False
    first_index: int | None = None
    index = 0
    while index < len(maps):
        if maps[index] == map_name:
            if first_index is None:
                first_index = index
            del maps[index]
            changed = True
            continue
        index += 1
    return changed, first_index


def update_map_groups(
    groups_data: dict[str, Any],
    old_name: str,
    new_name: str,
    from_group: str | None,
    to_group: str | None,
    preferred_group: str | None,
) -> bool:
    changed = False
    found = False
    if to_group:
        insert_index: int | None = None
        for group in groups_data.get("group_order", []):
            if from_group and group != from_group:
                continue
            maps = groups_data.get(group, [])
            removed, first_index = remove_map_from_group(maps, old_name)
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
        return changed

    for group in groups_data.get("group_order", []):
        if from_group and group != from_group:
            continue
        maps = groups_data.get(group, [])
        for index, map_name in enumerate(maps):
            if map_name == old_name:
                maps[index] = new_name
                found = True
                changed = True
    if not found and preferred_group:
        maps = ensure_map_group(groups_data, preferred_group)
        if new_name not in maps:
            maps.append(new_name)
            changed = True
    return changed


def update_map_json(data: dict[str, Any], old_id: str, new_id: str, new_name: str, new_layout: str | None) -> bool:
    changed = False
    if data.get("id") == old_id:
        data["id"] = new_id
        changed = True
    if data.get("name") != new_name:
        data["name"] = new_name
        changed = True
    if new_layout and data.get("layout") != new_layout:
        data["layout"] = new_layout
        changed = True
    return changed


def update_map_refs(data: dict[str, Any], old_id: str, new_id: str) -> bool:
    changed = False
    connections = data.get("connections")
    if isinstance(connections, list):
        for connection in connections:
            if isinstance(connection, dict) and connection.get("map") == old_id:
                connection["map"] = new_id
                changed = True
    warps = data.get("warp_events")
    if isinstance(warps, list):
        for warp in warps:
            if isinstance(warp, dict) and warp.get("dest_map") == old_id:
                warp["dest_map"] = new_id
                changed = True
    return changed


def update_layouts_json(layouts_data: dict[str, Any], layout_plan: dict[str, Any]) -> bool:
    changed = False
    old_id = layout_plan["oldId"]
    for layout in layouts_data.get("layouts", []):
        if not isinstance(layout, dict) or layout.get("id") != old_id:
            continue
        layout["id"] = layout_plan["newId"]
        if layout_plan.get("newName"):
            layout["name"] = layout_plan["newName"]
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


def update_event_scripts(text: str, old_name: str, new_name: str) -> tuple[str, bool]:
    old = f'.include "data/maps/{old_name}/scripts.inc"'
    new = f'.include "data/maps/{new_name}/scripts.inc"'
    if old not in text:
        return text, False
    return text.replace(old, new, 1), True


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
    src = repo_path(root, src_rel)
    dst = repo_path(root, dst_rel)
    if src == dst:
        return
    print(f"MOVE {src_rel} -> {dst_rel}")
    if dry_run:
        return
    if not src.exists():
        raise RelinkError(f"Cannot move missing path {src_rel}.")
    if dst.exists():
        raise RelinkError(f"Destination already exists: {dst_rel}.")
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.move(str(src), str(dst))


def apply_plan(root: Path, plan: dict[str, Any], dry_run: bool, allow_dirty: bool) -> None:
    if not allow_dirty and not dry_run:
        check_dirty(root, target_paths_from_plan(root, plan))

    for path in target_paths_from_plan(root, plan):
        ensure_not_generated(path, root)

    layout_by_old_id = {layout["oldId"]: layout for layout in plan.get("layouts", [])}

    # Moves first, so written JSON lands in final paths.
    for layout in plan.get("layouts", []):
        old_dir = layout.get("oldDir")
        new_dir = layout.get("newDir")
        if old_dir and new_dir:
            move_path(root, old_dir, new_dir, dry_run)

    for map_plan in plan.get("maps", []):
        old_name = map_plan["oldName"]
        new_name = map_plan["newName"]
        move_path(root, (MAPS_DIR / old_name).as_posix(), (MAPS_DIR / new_name).as_posix(), dry_run)

    groups_path = repo_path(root, MAP_GROUPS)
    groups_data = load_json(groups_path)
    groups_changed = False
    for map_plan in plan.get("maps", []):
        groups_changed |= update_map_groups(
            groups_data,
            map_plan["oldName"],
            map_plan["newName"],
            map_plan.get("fromGroup"),
            map_plan.get("toGroup"),
            map_plan.get("group"),
        )
    if groups_changed:
        print(f"EDIT {MAP_GROUPS}")
        if not dry_run:
            write_json(groups_path, groups_data)

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
        old_name = map_plan["oldName"]
        new_name = map_plan["newName"]
        old_id = map_plan["oldId"]
        new_id = map_plan["newId"]
        map_json_path = repo_path(root, MAPS_DIR / (new_name if not dry_run else old_name) / "map.json")
        map_data = load_json(map_json_path)
        new_layout = None
        old_layout = map_data.get("layout")
        if old_layout in layout_by_old_id:
            new_layout = layout_by_old_id[old_layout]["newId"]
        if update_map_json(map_data, old_id, new_id, new_name, new_layout):
            print(f"EDIT {MAPS_DIR / new_name / 'map.json'}")
            if not dry_run:
                write_json(repo_path(root, MAPS_DIR / new_name / "map.json"), map_data)

        event_scripts = repo_path(root, EVENT_SCRIPTS)
        if event_scripts.exists():
            new_text, changed = update_event_scripts(read_text(event_scripts), old_name, new_name)
            if changed:
                print(f"EDIT {EVENT_SCRIPTS}")
                if not dry_run:
                    write_text(event_scripts, new_text)

        for map_json in iter_all_map_json(root):
            if dry_run and map_json.parent.name == old_name:
                continue
            data = load_json(map_json)
            if update_map_refs(data, old_id, new_id):
                print(f"EDIT {rel(map_json, root)}")
                if not dry_run:
                    write_json(map_json, data)

    tokens = []
    for map_plan in plan.get("maps", []):
        tokens.extend([map_plan["oldName"], map_plan["oldId"]])
    for layout_plan in plan.get("layouts", []):
        tokens.extend([layout_plan["oldId"], layout_plan.get("oldName", "")])
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


def command_apply(args: argparse.Namespace) -> int:
    root = Path(args.root).resolve()
    plan = load_plan(Path(args.plan))
    apply_plan(root, plan, args.dry_run, args.allow_dirty)
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
    plan.add_argument("--from-group", help="Only remove/rename the old map entry from this source map group.")
    plan.add_argument("--to-group", help="Move the renamed map entry into this target map group, creating it if missing.")
    plan.add_argument("--group", help="Preferred map group if the old map is not already grouped.")
    plan.add_argument("--new-map-id", help="Override generated new MAP_* id.")
    plan.add_argument("--no-layout-rename", action="store_true", help="Keep the existing layout id/name/path.")
    plan.add_argument("--new-layout-id", help="Override generated new LAYOUT_* id.")
    plan.add_argument("--new-layout-name", help="Override generated new layout label.")
    plan.add_argument("--old-layout-dir", help="Override old layout directory.")
    plan.add_argument("--new-layout-dir", help="Override new layout directory.")
    plan.add_argument("--out", help="Write plan JSON to this path instead of stdout.")
    plan.set_defaults(func=command_plan)

    apply = subparsers.add_parser("apply", help="Apply or dry-run a relink plan.")
    apply.add_argument("plan", help="Path to plan JSON.")
    apply.add_argument("--dry-run", action="store_true", help="Print planned changes without modifying files.")
    apply.add_argument("--allow-dirty", action="store_true", help="Allow applying over dirty target files.")
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
