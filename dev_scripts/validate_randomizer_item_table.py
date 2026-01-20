#!/usr/bin/env python3
import argparse
import re
import sys
from pathlib import Path


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def parse_category_enum(text: str):
    match = re.search(r"enum\s+RandomizerItemCategory\s*\{([^}]+)\}", text, re.S)
    if not match:
        raise ValueError("RandomizerItemCategory enum not found.")
    body = match.group(1)
    names = []
    for line in body.splitlines():
        line = line.split("//", 1)[0]
        if not line.strip():
            continue
        for token in line.split(","):
            token = token.strip()
            if not token:
                continue
            name = token.split("=", 1)[0].strip()
            if name:
                names.append(name)
    if not names or names[-1] != "RANDOMIZER_ITEMCAT_COUNT":
        raise ValueError("RandomizerItemCategory must end with RANDOMIZER_ITEMCAT_COUNT.")
    return names[:-1]


def parse_bool(value: str):
    value = value.strip()
    if value in ("TRUE", "1"):
        return True
    if value in ("FALSE", "0"):
        return False
    raise ValueError(f"Unsupported boolean value: {value}")


def parse_int(value: str, constants=None):
    value = value.strip()
    try:
        return int(value, 0)
    except ValueError:
        if constants and value in constants:
            resolved = constants[value]
            if isinstance(resolved, int):
                return resolved
            return parse_int(str(resolved), constants)
        raise


def parse_constants(text: str):
    constants = {}
    define_pattern = re.compile(r"^\s*#define\s+([A-Z0-9_]+)\s+(.+)$")
    enum_pattern = re.compile(r"enum\s*(?:[A-Za-z0-9_]+\s*)?\{([^}]+)\}", re.S)

    for line in text.splitlines():
        line = line.split("//", 1)[0].split("/*", 1)[0].strip()
        if not line:
            continue
        match = define_pattern.match(line)
        if not match:
            continue
        name, value = match.groups()
        value = value.strip()
        if value.startswith("("):
            continue
        try:
            constants[name] = parse_int(value, constants)
        except ValueError:
            constants[name] = value

    for match in enum_pattern.finditer(text):
        body = match.group(1)
        current = 0
        for token in body.split(","):
            token = token.split("//", 1)[0].split("/*", 1)[0].strip()
            if not token:
                continue
            if "=" in token:
                name, value = (part.strip() for part in token.split("=", 1))
                current = parse_int(value, constants)
            else:
                name = token
            if name and name not in constants:
                constants[name] = current
            current += 1

    for _ in range(8):
        changed = False
        for name, value in list(constants.items()):
            if isinstance(value, str):
                try:
                    constants[name] = parse_int(value, constants)
                    changed = True
                except ValueError:
                    pass
        if not changed:
            break

    return constants


def parse_category_config(text: str, constants=None):
    config = {}
    pattern = re.compile(
        r"RANDOMIZER_ITEM_CATEGORY_CONFIG\(([^,]+),([^,]+),([^,]+),([^)]+)\)"
    )
    for line in text.splitlines():
        if line.lstrip().startswith("#define"):
            continue
        line = line.split("//", 1)[0]
        match = pattern.search(line)
        if not match:
            continue
        category, enabled, qty, weight = (part.strip() for part in match.groups())
        config[category] = {
            "enabled": parse_bool(enabled),
            "defaultQty": parse_int(qty, constants),
            "weightMul": parse_int(weight, constants),
        }
    return config


def parse_item_table(text: str, constants=None):
    entries = []
    pattern = re.compile(
        r"RANDOMIZER_ITEM_ENTRY\(([^,]+),([^,]+),([^,]+),([^)]+)\)"
    )
    for line in text.splitlines():
        if line.lstrip().startswith("#define"):
            continue
        line = line.split("//", 1)[0]
        match = pattern.search(line)
        if not match:
            continue
        item_id, weight, category, qty = (part.strip() for part in match.groups())
        entries.append(
            {
                "itemId": item_id,
                "weight": parse_int(weight, constants),
                "category": category,
                "qtyOverride": parse_int(qty, constants),
            }
        )
    return entries


def parse_item_pockets(text: str):
    pockets = {}
    current_item = None
    item_pattern = re.compile(r"\s*\[(ITEM_[A-Z0-9_]+)\]\s*=")
    pocket_pattern = re.compile(r"\.pocket\s*=\s*(POCKET_[A-Z0-9_]+)")
    for line in text.splitlines():
        item_match = item_pattern.match(line)
        if item_match:
            current_item = item_match.group(1)
        if current_item:
            pocket_match = pocket_pattern.search(line)
            if pocket_match:
                pockets[current_item] = pocket_match.group(1)
    return pockets


def parse_allow_empty(text: str) -> bool:
    match = re.search(r"#define\s+ALLOW_EMPTY_ITEM_POOL\s+(\S+)", text)
    if not match:
        raise ValueError("ALLOW_EMPTY_ITEM_POOL not found in randomizer config.")
    return parse_bool(match.group(1))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--randomizer-header", default="include/randomizer.h")
    parser.add_argument("--item-config", default="src/data/randomizer/item_category_config.h")
    parser.add_argument("--item-table", default="src/data/randomizer/item_table.h")
    parser.add_argument("--items-info", default="src/data/items.h")
    parser.add_argument("--randomizer-config", default="include/config/randomizer.h")
    parser.add_argument("--out", default=None)
    args = parser.parse_args()

    root = Path(__file__).resolve().parents[1]
    randomizer_header = root / args.randomizer_header
    item_config = root / args.item_config
    item_table = root / args.item_table
    items_info = root / args.items_info
    randomizer_config = root / args.randomizer_config

    errors = []

    categories = parse_category_enum(read_text(randomizer_header))
    item_config_text = read_text(item_config)
    item_table_text = read_text(item_table)
    config_constants = parse_constants(item_config_text)
    table_constants = parse_constants(item_table_text)
    config = parse_category_config(item_config_text, config_constants)
    entries = parse_item_table(item_table_text, table_constants)
    pockets = parse_item_pockets(read_text(items_info))
    allow_empty = parse_allow_empty(read_text(randomizer_config))

    missing_config = [name for name in categories if name not in config]
    if missing_config:
        errors.append(f"Missing category config entries: {', '.join(missing_config)}")

    seen = {}
    for entry in entries:
        item_id = entry["itemId"]
        weight = entry["weight"]
        qty_override = entry["qtyOverride"]
        category = entry["category"]

        if category not in categories:
            errors.append(f"Unknown category in item table: {category} ({item_id})")
            continue

        if weight == 0:
            errors.append(f"Weight is zero for item {item_id} ({category}).")

        if qty_override != 0 and qty_override < 1:
            errors.append(f"qtyOverride must be >= 1 for item {item_id} ({category}).")

        if item_id == "ITEM_NONE":
            errors.append("ITEM_NONE is not allowed in the randomizer item table.")

        if item_id not in pockets:
            if not (re.match(r"ITEM_TM\d{2,3}$", item_id) or re.match(r"ITEM_HM\d{2}$", item_id)):
                errors.append(f"Item not found in items data: {item_id}.")
        elif pockets[item_id] == "POCKET_KEY_ITEMS":
            errors.append(f"Key item is not allowed in item table: {item_id}.")

        if item_id in seen:
            seen[item_id].append(category)
        else:
            seen[item_id] = [category]

    duplicates = [item for item, cats in seen.items() if len(cats) > 1]
    if duplicates:
        errors.append(f"Duplicate item entries detected: {', '.join(sorted(duplicates))}")

    enabled_categories = [name for name in categories if config.get(name, {}).get("enabled")]
    disabled_categories = [name for name in categories if name not in enabled_categories]

    candidate_count = 0
    for entry in entries:
        category = entry["category"]
        if category not in config:
            continue
        cat_cfg = config[category]
        if not cat_cfg["enabled"] or cat_cfg["weightMul"] == 0:
            continue
        if entry["weight"] <= 0:
            continue
        candidate_count += 1

    if candidate_count == 0 and not allow_empty:
        errors.append(
            "Randomizer item pool is empty "
            f"(enabled categories={len(enabled_categories)}). "
            f"Disabled categories: {', '.join(disabled_categories)}"
        )

    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        return 1

    if args.out:
        out_path = root / args.out
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text("ok\n", encoding="utf-8")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
