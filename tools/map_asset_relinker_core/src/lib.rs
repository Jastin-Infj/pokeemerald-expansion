use serde::Serialize;
use serde_json::{json, Value};
use std::collections::{BTreeMap, BTreeSet};
use std::env;
use std::fs;
use std::path::{Path, PathBuf};

const MAPS_DIR: &str = "data/maps";
const LAYOUTS_DIR: &str = "data/layouts";
const MAP_GROUPS: &str = "data/maps/map_groups.json";
const LAYOUTS_JSON: &str = "data/layouts/layouts.json";
const REGION_MAP_SECTIONS_JSON: &str = "src/data/region_map/region_map_sections.json";

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct ProjectSummary {
    pub root: String,
    pub map_count: usize,
    pub group_count: usize,
    pub layout_count: usize,
    pub mapsec_count: usize,
    pub warning_count: usize,
    pub maps: Vec<MapSummary>,
    pub warnings: Vec<String>,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct MapSummary {
    pub directory_name: String,
    pub name: String,
    pub id: String,
    pub layout: String,
    pub layout_name: Option<String>,
    pub mapsec: String,
    pub mapsec_name: Option<String>,
    pub mapsec_position: Option<String>,
    pub map_type: String,
    pub show_map_name: bool,
    pub group: Option<String>,
    pub group_count: usize,
    pub issues: Vec<String>,
}

#[derive(Debug, Clone)]
struct LayoutInfo {
    name: Option<String>,
    border_path: Option<String>,
    blockdata_path: Option<String>,
}

#[derive(Debug, Clone)]
struct MapsecInfo {
    name: Option<String>,
    x: Option<i64>,
    y: Option<i64>,
    width: Option<i64>,
    height: Option<i64>,
}

#[derive(Debug, Clone)]
pub struct PlanRequest {
    pub old_map: String,
    pub new_map: String,
    pub match_by: String,
    pub from_group: Option<String>,
    pub to_group: Option<String>,
    pub group: Option<String>,
    pub rename_layout: bool,
    pub rename_mapsec: Option<(String, String)>,
    pub new_mapsec_name: Option<String>,
    pub new_mapsec: Option<String>,
    pub set_layout_id: Option<String>,
    pub rewrite_script_labels: bool,
}

#[derive(Debug)]
struct MapRef {
    path: PathBuf,
    data: Value,
    groups: Vec<String>,
}

#[derive(Debug)]
struct LayoutRef {
    data: Value,
}

pub fn make_plan(root: &Path, request: &PlanRequest) -> Result<Value, String> {
    let map_ref = find_map_ref(root, &request.old_map, &request.match_by)?;
    let old_dir_name = map_ref
        .path
        .parent()
        .and_then(Path::file_name)
        .map(|name| name.to_string_lossy().to_string())
        .ok_or_else(|| {
            format!(
                "failed to derive map directory for {}",
                map_ref.path.display()
            )
        })?;
    let new_dir_name = request.new_map.clone();
    let old_map_id = string_field(&map_ref.data, "id");
    let mut old_map_ids = BTreeSet::new();
    if let Some(id) = old_map_id.as_deref().filter(|id| !id.is_empty()) {
        old_map_ids.insert(id.to_string());
    }
    if request.match_by != "id" {
        old_map_ids.insert(format!("MAP_{}", camel_to_upper_snake(&request.old_map)));
    } else {
        old_map_ids.insert(request.old_map.clone());
    }
    let old_map_ids = old_map_ids.into_iter().collect::<Vec<_>>();
    let new_map_id = format!("MAP_{}", camel_to_upper_snake(&request.new_map));

    let mut mapsec_plans = Vec::new();
    let mut new_mapsec = request.new_mapsec.clone();
    if let Some((old_mapsec, renamed_mapsec)) = &request.rename_mapsec {
        let mapsec_entries = read_mapsecs(&read_json(&root.join(REGION_MAP_SECTIONS_JSON))?);
        let Some(entry) = mapsec_entries.get(old_mapsec) else {
            return Err(format!("Region map section {old_mapsec:?} was not found."));
        };
        if mapsec_entries.contains_key(renamed_mapsec) && renamed_mapsec != old_mapsec {
            return Err(format!(
                "Region map section {renamed_mapsec:?} already exists."
            ));
        }
        if let Some(explicit_new) = &request.new_mapsec {
            if explicit_new != renamed_mapsec {
                return Err(
                    "--new-mapsec must match the NEW side of rename_mapsec when both are provided."
                        .to_string(),
                );
            }
        }
        new_mapsec = Some(renamed_mapsec.clone());
        mapsec_plans.push(json!({
            "oldId": old_mapsec,
            "newId": renamed_mapsec,
            "oldName": entry.name.clone(),
            "newName": request.new_mapsec_name.clone(),
        }));
    } else if let Some(new_mapsec_id) = &new_mapsec {
        let mapsec_entries = read_mapsecs(&read_json(&root.join(REGION_MAP_SECTIONS_JSON))?);
        if !mapsec_entries.contains_key(new_mapsec_id) {
            return Err(format!(
                "Region map section {new_mapsec_id:?} was not found."
            ));
        }
    }

    let layout_plan = if request.rename_layout {
        let old_layout_id = string_field(&map_ref.data, "layout")
            .ok_or_else(|| format!("{} has no layout field", map_ref.path.display()))?;
        let layout_ref = find_layout_ref(root, &old_layout_id)?;
        let old_layout_name = string_field(&layout_ref.data, "name");
        Some(json!({
            "oldId": old_layout_id,
            "newId": format!("LAYOUT_{}", camel_to_upper_snake(&request.new_map)),
            "oldName": old_layout_name,
            "newName": format!("{}_Layout", request.new_map),
            "oldDir": default_layout_dir(&layout_ref.data),
            "newDir": format!("{LAYOUTS_DIR}/{}", request.new_map),
            "primaryTileset": Value::Null,
            "secondaryTileset": Value::Null,
        }))
    } else {
        None
    };

    if let Some(layout_id) = &request.set_layout_id {
        find_layout_ref(root, layout_id)?;
    }

    let mut old_group_names = BTreeSet::new();
    old_group_names.insert(old_dir_name.clone());
    old_group_names.insert(request.old_map.clone());
    if let Some(json_name) = string_field(&map_ref.data, "name").filter(|name| !name.is_empty()) {
        old_group_names.insert(json_name);
    }
    let old_group_names = old_group_names.into_iter().collect::<Vec<_>>();
    let mut groups = map_ref.groups.clone();
    for group_name in map_groups_for_names(root, &old_group_names)? {
        if !groups.contains(&group_name) {
            groups.push(group_name);
        }
    }
    groups.sort();
    if let Some(from_group) = &request.from_group {
        if !groups.contains(from_group) {
            return Err(format!(
                "Map {:?} is not listed in source group {:?}.",
                request.old_map, from_group
            ));
        }
    }
    let group = request
        .to_group
        .clone()
        .or_else(|| request.group.clone())
        .or_else(|| request.from_group.clone())
        .or_else(|| groups.first().cloned());

    let script_old_prefixes = if request.rewrite_script_labels {
        let mut prefixes = BTreeSet::new();
        prefixes.insert(request.old_map.clone());
        prefixes.insert(old_dir_name.clone());
        prefixes.into_iter().collect::<Vec<_>>()
    } else {
        Vec::new()
    };

    let new_layout_id = request.set_layout_id.clone().or_else(|| {
        layout_plan
            .as_ref()
            .and_then(|plan| string_field(plan, "newId"))
    });

    Ok(json!({
        "version": 1,
        "maps": [{
            "oldName": request.old_map,
            "newName": request.new_map,
            "oldDirName": old_dir_name,
            "newDirName": new_dir_name,
            "oldId": old_map_id,
            "oldIds": old_map_ids,
            "newId": new_map_id,
            "newMapsec": new_mapsec,
            "newLayoutId": new_layout_id,
            "mapType": Value::Null,
            "showMapName": Value::Null,
            "transitionSetFlag": Value::Null,
            "group": group,
            "fromGroup": request.from_group,
            "toGroup": request.to_group,
            "groupOldNames": old_group_names,
            "matchBy": request.match_by,
            "scriptOldPrefixes": script_old_prefixes,
        }],
        "layouts": layout_plan.into_iter().collect::<Vec<_>>(),
        "mapsecs": mapsec_plans,
        "regionMapCells": [],
        "flyLocations": [],
        "flyMapsecTypes": [],
        "flyIconStyles": [],
        "mapsecMaps": [],
        "flagClaims": [],
        "dropGroups": [],
        "options": {
            "rewriteScriptLabels": request.rewrite_script_labels,
        },
    }))
}

pub fn write_plan(path: &Path, plan: &Value) -> Result<(), String> {
    let text = serde_json::to_string_pretty(plan)
        .map_err(|err| format!("failed to encode plan JSON: {err}"))?;
    fs::write(path, format!("{text}\n"))
        .map_err(|err| format!("failed to write {}: {err}", path.display()))
}

pub fn scan_project(root: Option<PathBuf>) -> Result<ProjectSummary, String> {
    let root = resolve_project_root(root)?;
    let map_groups = read_json(&root.join("data/maps/map_groups.json"))?;
    let layouts_json = read_json(&root.join("data/layouts/layouts.json"))?;
    let mapsecs_json = read_json(&root.join("src/data/region_map/region_map_sections.json"))?;

    let group_order = map_groups
        .get("group_order")
        .and_then(Value::as_array)
        .ok_or_else(|| "data/maps/map_groups.json is missing group_order".to_string())?;

    let mut map_to_groups: BTreeMap<String, Vec<String>> = BTreeMap::new();
    for group in group_order.iter().filter_map(Value::as_str) {
        if let Some(entries) = map_groups.get(group).and_then(Value::as_array) {
            for entry in entries.iter().filter_map(Value::as_str) {
                map_to_groups
                    .entry(entry.to_string())
                    .or_default()
                    .push(group.to_string());
            }
        }
    }

    let layouts = read_layouts(&layouts_json);
    let mapsecs = read_mapsecs(&mapsecs_json);
    let mut warnings = Vec::new();
    let mut maps = Vec::new();
    let mut seen_ids: BTreeMap<String, Vec<String>> = BTreeMap::new();
    let mut seen_names = BTreeSet::new();

    let maps_dir = root.join("data/maps");
    for entry in fs::read_dir(&maps_dir)
        .map_err(|err| format!("failed to read {}: {err}", maps_dir.display()))?
    {
        let entry = entry.map_err(|err| format!("failed to read map entry: {err}"))?;
        let file_type = entry
            .file_type()
            .map_err(|err| format!("failed to read file type for {:?}: {err}", entry.path()))?;
        if !file_type.is_dir() {
            continue;
        }

        let directory_name = entry.file_name().to_string_lossy().to_string();
        let map_json_path = entry.path().join("map.json");
        if !map_json_path.exists() {
            warnings.push(format!("{directory_name}: missing map.json"));
            continue;
        }

        let map_json = read_json(&map_json_path)?;
        let name = string_field(&map_json, "name").unwrap_or_else(|| directory_name.clone());
        let id = string_field(&map_json, "id").unwrap_or_default();
        let layout = string_field(&map_json, "layout").unwrap_or_default();
        let mapsec = string_field(&map_json, "region_map_section").unwrap_or_default();
        let map_type = string_field(&map_json, "map_type").unwrap_or_default();
        let show_map_name = map_json
            .get("show_map_name")
            .and_then(Value::as_bool)
            .unwrap_or(false);

        let groups = map_to_groups.get(&name).cloned().unwrap_or_default();
        let layout_info = layouts.get(&layout);
        let mapsec_info = mapsecs.get(&mapsec);
        let mut issues = Vec::new();

        if directory_name != name {
            issues.push(format!(
                "directory name is {directory_name}, map.json name is {name}"
            ));
        }
        if id.is_empty() {
            issues.push("map id is empty".to_string());
        }
        if looks_temporary_map_name(&name) {
            issues.push(format!(
                "map name {name} looks temporary; pick a production map name before committing"
            ));
        }
        if layout.is_empty() {
            issues.push("layout is empty".to_string());
        } else if layout_info.is_none() {
            issues.push(format!("layout {layout} is missing from layouts.json"));
        }
        if mapsec.is_empty() {
            issues.push("region_map_section is empty".to_string());
        } else if mapsec != "MAPSEC_NONE" && mapsec_info.is_none() {
            issues.push(format!("{mapsec} is missing from region_map_sections.json"));
        }
        if !mapsec.is_empty() && mapsec != "MAPSEC_NONE" && !is_uppercase_mapsec_id(&mapsec) {
            issues.push(format!(
                "{mapsec} has suspicious naming; expected uppercase MAPSEC_*"
            ));
        }
        match groups.len() {
            0 => issues.push("map is not listed in any map group".to_string()),
            1 => {}
            count => issues.push(format!("map is listed in {count} map groups")),
        }

        if !id.is_empty() {
            seen_ids.entry(id.clone()).or_default().push(name.clone());
        }
        if !seen_names.insert(name.clone()) {
            issues.push(format!("duplicate map name {name}"));
        }

        if let Some(layout_info) = layout_info {
            check_layout_path(
                &root,
                &mut issues,
                &layout,
                "border",
                &layout_info.border_path,
            );
            check_layout_path(
                &root,
                &mut issues,
                &layout,
                "blockdata",
                &layout_info.blockdata_path,
            );
        }

        let mapsec_position = mapsec_info.and_then(|entry| {
            Some(format!(
                "{},{} {}x{}",
                entry.x?, entry.y?, entry.width?, entry.height?
            ))
        });

        warnings.extend(issues.iter().map(|issue| format!("{name}: {issue}")));
        maps.push(MapSummary {
            directory_name,
            name,
            id,
            layout,
            layout_name: layout_info.and_then(|entry| entry.name.clone()),
            mapsec,
            mapsec_name: mapsec_info.and_then(|entry| entry.name.clone()),
            mapsec_position,
            map_type,
            show_map_name,
            group: groups.first().cloned(),
            group_count: groups.len(),
            issues,
        });
    }

    for (id, names) in seen_ids {
        if names.len() > 1 {
            warnings.push(format!("duplicate map id {id}: {}", names.join(", ")));
        }
    }

    maps.sort_by(|left, right| left.name.cmp(&right.name));
    warnings.sort();

    Ok(ProjectSummary {
        root: root.display().to_string(),
        map_count: maps.len(),
        group_count: group_order.len(),
        layout_count: layouts.len(),
        mapsec_count: mapsecs.len(),
        warning_count: warnings.len(),
        maps,
        warnings,
    })
}

pub fn resolve_project_root(root: Option<PathBuf>) -> Result<PathBuf, String> {
    if let Some(root) = root {
        validate_project_root(&root)?;
        return Ok(root);
    }

    let mut candidate =
        env::current_dir().map_err(|err| format!("failed to read current directory: {err}"))?;
    loop {
        if validate_project_root(&candidate).is_ok() {
            return Ok(candidate);
        }
        if !candidate.pop() {
            break;
        }
    }

    Err("could not locate repo root; choose a folder containing data/maps/map_groups.json".into())
}

fn find_map_ref(root: &Path, old_value: &str, match_by: &str) -> Result<MapRef, String> {
    let mut matches = Vec::new();
    let maps_dir = root.join(MAPS_DIR);
    for entry in fs::read_dir(&maps_dir)
        .map_err(|err| format!("failed to read {}: {err}", maps_dir.display()))?
    {
        let entry = entry.map_err(|err| format!("failed to read map entry: {err}"))?;
        let file_type = entry
            .file_type()
            .map_err(|err| format!("failed to read file type for {:?}: {err}", entry.path()))?;
        if !file_type.is_dir() {
            continue;
        }
        let map_json_path = entry.path().join("map.json");
        if !map_json_path.is_file() {
            continue;
        }
        let data = read_json(&map_json_path)?;
        let directory_name = entry.file_name().to_string_lossy().to_string();
        let json_name = string_field(&data, "name");
        let map_id = string_field(&data, "id");
        let matched = match match_by {
            "dir" => directory_name == old_value,
            "name" => json_name.as_deref() == Some(old_value),
            "id" => map_id.as_deref() == Some(old_value),
            _ => return Err(format!("unknown match mode {match_by:?}")),
        };
        if matched {
            let mut group_names = BTreeSet::new();
            group_names.insert(directory_name);
            if let Some(name) = json_name {
                group_names.insert(name);
            }
            let group_names = group_names.into_iter().collect::<Vec<_>>();
            let groups = map_groups_for_names(root, &group_names)?;
            matches.push(MapRef {
                path: map_json_path,
                data,
                groups,
            });
        }
    }
    match matches.len() {
        0 => Err(format!(
            "Map {old_value:?} was not found by {match_by} in {MAPS_DIR}."
        )),
        1 => Ok(matches.remove(0)),
        _ => {
            let locations = matches
                .iter()
                .map(|map_ref| display_path(root, &map_ref.path))
                .collect::<Vec<_>>()
                .join(", ");
            Err(format!(
                "Map {old_value:?} matched multiple maps by {match_by}: {locations}."
            ))
        }
    }
}

fn map_groups_for_names(root: &Path, names: &[String]) -> Result<Vec<String>, String> {
    let groups_data = read_json(&root.join(MAP_GROUPS))?;
    let Some(group_order) = groups_data.get("group_order").and_then(Value::as_array) else {
        return Ok(Vec::new());
    };
    let names = names.iter().cloned().collect::<BTreeSet<_>>();
    let mut groups = Vec::new();
    for group in group_order.iter().filter_map(Value::as_str) {
        let Some(entries) = groups_data.get(group).and_then(Value::as_array) else {
            continue;
        };
        if entries
            .iter()
            .filter_map(Value::as_str)
            .any(|map_name| names.contains(map_name))
        {
            groups.push(group.to_string());
        }
    }
    Ok(groups)
}

fn find_layout_ref(root: &Path, layout_id: &str) -> Result<LayoutRef, String> {
    let layouts_json = read_json(&root.join(LAYOUTS_JSON))?;
    let Some(entries) = layouts_json.get("layouts").and_then(Value::as_array) else {
        return Err(format!("{LAYOUTS_JSON} is missing layouts."));
    };
    for entry in entries {
        if string_field(entry, "id").as_deref() == Some(layout_id) {
            return Ok(LayoutRef {
                data: entry.clone(),
            });
        }
    }
    Err(format!(
        "Layout {layout_id:?} was not found in {LAYOUTS_JSON}."
    ))
}

fn default_layout_dir(layout: &Value) -> Option<String> {
    let border = string_field(layout, "border_filepath")?;
    let block = string_field(layout, "blockdata_filepath")?;
    let border_dir = Path::new(&border).parent()?.to_string_lossy().to_string();
    let block_dir = Path::new(&block).parent()?.to_string_lossy().to_string();
    (border_dir == block_dir).then_some(border_dir)
}

fn validate_project_root(root: &Path) -> Result<(), String> {
    let required = root.join("data/maps/map_groups.json");
    if required.exists() {
        Ok(())
    } else {
        Err(format!("{} does not exist", required.display()))
    }
}

fn read_json(path: &Path) -> Result<Value, String> {
    let text = fs::read_to_string(path)
        .map_err(|err| format!("failed to read {}: {err}", path.display()))?;
    serde_json::from_str(&text).map_err(|err| format!("invalid JSON in {}: {err}", path.display()))
}

fn read_layouts(layouts_json: &Value) -> BTreeMap<String, LayoutInfo> {
    let mut layouts = BTreeMap::new();
    if let Some(entries) = layouts_json.get("layouts").and_then(Value::as_array) {
        for entry in entries {
            if let Some(id) = string_field(entry, "id") {
                layouts.insert(
                    id,
                    LayoutInfo {
                        name: string_field(entry, "name"),
                        border_path: string_field(entry, "border_filepath"),
                        blockdata_path: string_field(entry, "blockdata_filepath"),
                    },
                );
            }
        }
    }
    layouts
}

fn read_mapsecs(mapsecs_json: &Value) -> BTreeMap<String, MapsecInfo> {
    let mut mapsecs = BTreeMap::new();
    if let Some(entries) = mapsecs_json.get("map_sections").and_then(Value::as_array) {
        for entry in entries {
            if let Some(id) = string_field(entry, "id") {
                mapsecs.insert(
                    id,
                    MapsecInfo {
                        name: string_field(entry, "name"),
                        x: entry.get("x").and_then(Value::as_i64),
                        y: entry.get("y").and_then(Value::as_i64),
                        width: entry.get("width").and_then(Value::as_i64),
                        height: entry.get("height").and_then(Value::as_i64),
                    },
                );
            }
        }
    }
    mapsecs
}

fn string_field(value: &Value, key: &str) -> Option<String> {
    value
        .get(key)
        .and_then(Value::as_str)
        .map(ToString::to_string)
}

fn display_path(root: &Path, path: &Path) -> String {
    path.strip_prefix(root)
        .unwrap_or(path)
        .to_string_lossy()
        .replace('\\', "/")
}

fn camel_to_upper_snake(name: &str) -> String {
    name.split('_')
        .filter(|token| !token.is_empty())
        .map(split_camel_token)
        .collect::<Vec<_>>()
        .join("_")
        .to_ascii_uppercase()
}

fn split_camel_token(token: &str) -> String {
    let chars = token.chars().collect::<Vec<_>>();
    let mut out = String::new();
    for (index, ch) in chars.iter().enumerate() {
        if index > 0 && ch.is_ascii_uppercase() && chars[index - 1].is_ascii_lowercase() {
            out.push('_');
        }
        out.push(*ch);
    }
    out
}

fn check_layout_path(
    root: &Path,
    issues: &mut Vec<String>,
    layout: &str,
    label: &str,
    path: &Option<String>,
) {
    let Some(path) = path else {
        issues.push(format!("{layout} has no {label} filepath"));
        return;
    };
    if !root.join(path).exists() {
        issues.push(format!("{layout} {label} file is missing: {path}"));
    }
}

fn looks_temporary_map_name(name: &str) -> bool {
    let lower = name.to_ascii_lowercase();
    temporary_suffix(&lower, "test")
        || temporary_suffix(&lower, "temp")
        || temporary_suffix(&lower, "temporary")
}

fn temporary_suffix(value: &str, prefix: &str) -> bool {
    value.strip_prefix(prefix).is_some_and(|suffix| {
        let suffix = suffix.trim_start_matches(['_', '-']);
        suffix.chars().all(|ch| ch.is_ascii_digit())
    })
}

fn is_uppercase_mapsec_id(mapsec: &str) -> bool {
    mapsec.strip_prefix("MAPSEC_").is_some_and(|suffix| {
        !suffix.is_empty()
            && suffix
                .chars()
                .all(|ch| ch.is_ascii_uppercase() || ch.is_ascii_digit() || ch == '_')
    })
}
