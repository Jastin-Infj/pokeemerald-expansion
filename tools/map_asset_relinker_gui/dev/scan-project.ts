import fs from "node:fs";
import path from "node:path";

type JsonValue =
  | null
  | boolean
  | number
  | string
  | JsonValue[]
  | { [key: string]: JsonValue };

type JsonObject = { [key: string]: JsonValue };

type LayoutInfo = {
  name: string | null;
  borderPath: string | null;
  blockdataPath: string | null;
};

type MapsecInfo = {
  name: string | null;
  x: number | null;
  y: number | null;
  width: number | null;
  height: number | null;
};

export type MapSummary = {
  directoryName: string;
  name: string;
  id: string;
  layout: string;
  layoutName: string | null;
  mapsec: string;
  mapsecName: string | null;
  mapsecPosition: string | null;
  mapType: string;
  showMapName: boolean;
  group: string | null;
  groupCount: number;
  issues: string[];
};

export type ProjectSummary = {
  root: string;
  mapCount: number;
  groupCount: number;
  layoutCount: number;
  mapsecCount: number;
  warningCount: number;
  maps: MapSummary[];
  warnings: string[];
};

export function scanProjectFromNode(root?: string | null): ProjectSummary {
  const projectRoot = resolveProjectRoot(root);
  const mapGroups = readJson(path.join(projectRoot, "data/maps/map_groups.json"));
  const layoutsJson = readJson(path.join(projectRoot, "data/layouts/layouts.json"));
  const mapsecsJson = readJson(
    path.join(projectRoot, "src/data/region_map/region_map_sections.json"),
  );

  const groupOrder = asArray(mapGroups.group_order);
  const mapToGroups = new Map<string, string[]>();

  for (const group of groupOrder.map(asString).filter(Boolean)) {
    const entries = asArray(mapGroups[group]);
    for (const entry of entries.map(asString).filter(Boolean)) {
      const groups = mapToGroups.get(entry) ?? [];
      groups.push(group);
      mapToGroups.set(entry, groups);
    }
  }

  const layouts = readLayouts(layoutsJson);
  const mapsecs = readMapsecs(mapsecsJson);
  const warnings: string[] = [];
  const maps: MapSummary[] = [];
  const seenIds = new Map<string, string[]>();
  const seenNames = new Set<string>();

  const mapsDir = path.join(projectRoot, "data/maps");
  for (const directoryName of fs.readdirSync(mapsDir).sort()) {
    const mapDir = path.join(mapsDir, directoryName);
    if (!fs.statSync(mapDir).isDirectory()) {
      continue;
    }

    const mapJsonPath = path.join(mapDir, "map.json");
    if (!fs.existsSync(mapJsonPath)) {
      warnings.push(`${directoryName}: missing map.json`);
      continue;
    }

    const mapJson = readJson(mapJsonPath);
    const name = stringField(mapJson, "name") ?? directoryName;
    const id = stringField(mapJson, "id") ?? "";
    const layout = stringField(mapJson, "layout") ?? "";
    const mapsec = stringField(mapJson, "region_map_section") ?? "";
    const mapType = stringField(mapJson, "map_type") ?? "";
    const showMapName = booleanField(mapJson, "show_map_name") ?? false;
    const groups = mapToGroups.get(name) ?? [];
    const layoutInfo = layouts.get(layout) ?? null;
    const mapsecInfo = mapsecs.get(mapsec) ?? null;
    const issues: string[] = [];

    if (directoryName !== name) {
      issues.push(`directory name is ${directoryName}, map.json name is ${name}`);
    }
    if (!id) {
      issues.push("map id is empty");
    }
    if (!layout) {
      issues.push("layout is empty");
    } else if (!layoutInfo) {
      issues.push(`layout ${layout} is missing from layouts.json`);
    }
    if (!mapsec) {
      issues.push("region_map_section is empty");
    } else if (mapsec !== "MAPSEC_NONE" && !mapsecInfo) {
      issues.push(`${mapsec} is missing from region_map_sections.json`);
    }
    if (groups.length === 0) {
      issues.push("map is not listed in any map group");
    } else if (groups.length > 1) {
      issues.push(`map is listed in ${groups.length} map groups`);
    }

    if (id) {
      const names = seenIds.get(id) ?? [];
      names.push(name);
      seenIds.set(id, names);
    }
    if (seenNames.has(name)) {
      issues.push(`duplicate map name ${name}`);
    }
    seenNames.add(name);

    if (layoutInfo) {
      checkLayoutPath(projectRoot, issues, layout, "border", layoutInfo.borderPath);
      checkLayoutPath(
        projectRoot,
        issues,
        layout,
        "blockdata",
        layoutInfo.blockdataPath,
      );
    }

    const mapsecPosition =
      mapsecInfo?.x != null &&
      mapsecInfo.y != null &&
      mapsecInfo.width != null &&
      mapsecInfo.height != null
        ? `${mapsecInfo.x},${mapsecInfo.y} ${mapsecInfo.width}x${mapsecInfo.height}`
        : null;

    warnings.push(...issues.map((issue) => `${name}: ${issue}`));
    maps.push({
      directoryName,
      name,
      id,
      layout,
      layoutName: layoutInfo?.name ?? null,
      mapsec,
      mapsecName: mapsecInfo?.name ?? null,
      mapsecPosition,
      mapType,
      showMapName,
      group: groups[0] ?? null,
      groupCount: groups.length,
      issues,
    });
  }

  for (const [id, names] of seenIds.entries()) {
    if (names.length > 1) {
      warnings.push(`duplicate map id ${id}: ${names.join(", ")}`);
    }
  }

  maps.sort((left, right) => left.name.localeCompare(right.name));
  warnings.sort();

  return {
    root: projectRoot,
    mapCount: maps.length,
    groupCount: groupOrder.length,
    layoutCount: layouts.size,
    mapsecCount: mapsecs.size,
    warningCount: warnings.length,
    maps,
    warnings,
  };
}

function resolveProjectRoot(root?: string | null): string {
  if (root) {
    const resolved = path.resolve(root);
    validateProjectRoot(resolved);
    return resolved;
  }

  let candidate = process.cwd();
  while (true) {
    if (fs.existsSync(path.join(candidate, "data/maps/map_groups.json"))) {
      return candidate;
    }
    const parent = path.dirname(candidate);
    if (parent === candidate) {
      break;
    }
    candidate = parent;
  }

  throw new Error("could not locate repo root; choose a folder containing data/maps/map_groups.json");
}

function validateProjectRoot(root: string): void {
  const required = path.join(root, "data/maps/map_groups.json");
  if (!fs.existsSync(required)) {
    throw new Error(`${required} does not exist`);
  }
}

function readJson(filePath: string): JsonObject {
  return JSON.parse(fs.readFileSync(filePath, "utf8")) as JsonObject;
}

function readLayouts(layoutsJson: JsonObject): Map<string, LayoutInfo> {
  const layouts = new Map<string, LayoutInfo>();
  for (const entry of asArray(layoutsJson.layouts).filter(isObject)) {
    const id = stringField(entry, "id");
    if (!id) {
      continue;
    }
    layouts.set(id, {
      name: stringField(entry, "name"),
      borderPath: stringField(entry, "border_filepath"),
      blockdataPath: stringField(entry, "blockdata_filepath"),
    });
  }
  return layouts;
}

function readMapsecs(mapsecsJson: JsonObject): Map<string, MapsecInfo> {
  const mapsecs = new Map<string, MapsecInfo>();
  for (const entry of asArray(mapsecsJson.map_sections).filter(isObject)) {
    const id = stringField(entry, "id");
    if (!id) {
      continue;
    }
    mapsecs.set(id, {
      name: stringField(entry, "name"),
      x: numberField(entry, "x"),
      y: numberField(entry, "y"),
      width: numberField(entry, "width"),
      height: numberField(entry, "height"),
    });
  }
  return mapsecs;
}

function checkLayoutPath(
  root: string,
  issues: string[],
  layout: string,
  label: string,
  layoutPath: string | null,
): void {
  if (!layoutPath) {
    issues.push(`${layout} has no ${label} filepath`);
    return;
  }
  if (!fs.existsSync(path.join(root, layoutPath))) {
    issues.push(`${layout} ${label} file is missing: ${layoutPath}`);
  }
}

function asArray(value: JsonValue | undefined): JsonValue[] {
  return Array.isArray(value) ? value : [];
}

function asString(value: JsonValue): string {
  return typeof value === "string" ? value : "";
}

function isObject(value: JsonValue): value is JsonObject {
  return Boolean(value) && typeof value === "object" && !Array.isArray(value);
}

function stringField(value: JsonObject, key: string): string | null {
  const field = value[key];
  return typeof field === "string" ? field : null;
}

function numberField(value: JsonObject, key: string): number | null {
  const field = value[key];
  return typeof field === "number" ? field : null;
}

function booleanField(value: JsonObject, key: string): boolean | null {
  const field = value[key];
  return typeof field === "boolean" ? field : null;
}
