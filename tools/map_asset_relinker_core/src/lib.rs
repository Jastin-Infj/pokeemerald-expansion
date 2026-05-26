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
const EVENT_SCRIPTS: &str = "data/event_scripts.s";
const REGION_MAP_SECTIONS_JSON: &str = "src/data/region_map/region_map_sections.json";
const REGION_MAP_C: &str = "src/region_map.c";
const FLAGS_H: &str = "include/constants/flags.h";
const REGION_MAP_LAYOUT_FILES: &[(&str, &str)] = &[
    ("hoenn", "src/data/region_map/region_map_layout.h"),
    ("kanto", "src/data/region_map/region_map_layout_kanto.h"),
    (
        "sevii123",
        "src/data/region_map/region_map_layout_sevii123.h",
    ),
    ("sevii45", "src/data/region_map/region_map_layout_sevii45.h"),
    ("sevii67", "src/data/region_map/region_map_layout_sevii67.h"),
];
const GENERATED_OUTPUTS: &[&str] = &[
    "include/constants/map_groups.h",
    "include/constants/layouts.h",
    "include/constants/map_event_ids.h",
    "data/maps/groups.inc",
    "data/maps/headers.inc",
    "data/maps/events.inc",
    "data/maps/connections.inc",
    "data/layouts/layouts.inc",
    "data/layouts/layouts_table.inc",
    "src/data/map_group_count.h",
];

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

pub fn apply_plan_dry_run(root: &Path, plan: &Value) -> Result<String, String> {
    validate_project_root(root)?;
    let mut output = Vec::new();
    let mut printed_edits = BTreeSet::new();
    let layout_by_old_id = layout_plans(plan)
        .into_iter()
        .filter_map(|layout| Some((plan_string(layout, "oldId")?, layout)))
        .collect::<BTreeMap<_, _>>();

    for layout in layout_plans(plan) {
        let old_dir = plan_string(layout, "oldDir");
        let new_dir = plan_string(layout, "newDir");
        if let (Some(old_dir), Some(new_dir)) = (old_dir, new_dir) {
            if old_dir != new_dir {
                output.push(format!("MOVE {old_dir} -> {new_dir}"));
            }
        }
    }

    for map_plan in map_plans(plan) {
        let old_dir = map_old_dir(map_plan)?;
        let new_dir = map_new_dir(map_plan)?;
        if old_dir != new_dir {
            output.push(format!(
                "MOVE {}/{} -> {}/{}",
                MAPS_DIR, old_dir, MAPS_DIR, new_dir
            ));
        }
    }

    let groups_path = root.join(MAP_GROUPS);
    let mut groups_data = read_json(&groups_path)?;
    let mut groups_changed = false;
    for map_plan in map_plans(plan) {
        groups_changed |= update_map_groups_value(
            &mut groups_data,
            &map_old_group_names(map_plan)?,
            &plan_string_required(map_plan, "newName")?,
            plan_string(map_plan, "fromGroup").as_deref(),
            plan_string(map_plan, "toGroup").as_deref(),
            plan_string(map_plan, "group").as_deref(),
        )?;
    }
    groups_changed |=
        drop_empty_map_groups_value(&mut groups_data, &string_vec(plan, "dropGroups"))?;
    if groups_changed {
        print_edit(&mut output, &mut printed_edits, MAP_GROUPS);
    }

    let mapsec_renames = mapsec_plans(plan)
        .into_iter()
        .filter_map(|mapsec| Some((plan_string(mapsec, "oldId")?, plan_string(mapsec, "newId")?)))
        .collect::<Vec<_>>();
    if !mapsec_plans(plan).is_empty() {
        let mut mapsecs_data = read_json(&root.join(REGION_MAP_SECTIONS_JSON))?;
        let mut changed = false;
        for mapsec_plan in mapsec_plans(plan) {
            changed |= update_mapsecs_json_value(&mut mapsecs_data, mapsec_plan)?;
        }
        if changed {
            print_edit(&mut output, &mut printed_edits, REGION_MAP_SECTIONS_JSON);
        }
    }

    if !layout_plans(plan).is_empty() {
        let mut layouts_data = read_json(&root.join(LAYOUTS_JSON))?;
        let mut changed = false;
        for layout_plan in layout_plans(plan) {
            changed |= update_layouts_json_value(&mut layouts_data, layout_plan)?;
        }
        if changed {
            print_edit(&mut output, &mut printed_edits, LAYOUTS_JSON);
        }
    }

    for map_plan in map_plans(plan) {
        let old_dir = map_old_dir(map_plan)?;
        let new_dir = map_new_dir(map_plan)?;
        let old_ids = map_old_ids(map_plan)?;
        let new_id = plan_string_required(map_plan, "newId")?;
        let new_name = plan_string_required(map_plan, "newName")?;
        let mut map_data = read_json(&root.join(MAPS_DIR).join(&old_dir).join("map.json"))?;
        let mut new_layout = plan_string(map_plan, "newLayoutId");
        if new_layout.is_none() {
            if let Some(old_layout) = string_field(&map_data, "layout") {
                new_layout = layout_by_old_id
                    .get(&old_layout)
                    .and_then(|layout| plan_string(layout, "newId"));
            }
        }
        let map_changed = update_map_json_value(
            &mut map_data,
            &old_ids,
            &new_id,
            &new_name,
            new_layout.as_deref(),
            plan_string(map_plan, "newMapsec").as_deref(),
            plan_string(map_plan, "mapType").as_deref(),
            plan_bool(map_plan, "showMapName"),
        );
        if map_changed {
            print_edit(
                &mut output,
                &mut printed_edits,
                &format!("{MAPS_DIR}/{new_dir}/map.json"),
            );
        }

        let script_path = root.join(MAPS_DIR).join(&old_dir).join("scripts.inc");
        let mut script_text = if script_path.exists() {
            Some(read_text(&script_path)?)
        } else {
            None
        };
        let mut script_changed = false;
        if plan
            .get("options")
            .and_then(|options| options.get("rewriteScriptLabels"))
            .and_then(Value::as_bool)
            .unwrap_or(false)
        {
            if let Some(text) = script_text.take() {
                let prefixes = {
                    let configured = string_vec(map_plan, "scriptOldPrefixes");
                    if configured.is_empty() {
                        vec![old_dir.clone(), plan_string_required(map_plan, "oldName")?]
                    } else {
                        configured
                    }
                };
                let (updated, changed) =
                    update_script_label_prefixes_text(&text, &prefixes, &new_name);
                script_changed |= changed;
                script_text = Some(updated);
            }
        }
        if let Some(flag) = plan_string(map_plan, "transitionSetFlag") {
            if let Some(text) = script_text.as_deref() {
                script_changed |= transition_setflag_would_change(text, &new_name, &flag)?;
            }
        }
        if script_changed {
            print_edit(
                &mut output,
                &mut printed_edits,
                &format!("{MAPS_DIR}/{new_dir}/scripts.inc"),
            );
        }

        let event_scripts = root.join(EVENT_SCRIPTS);
        if event_scripts.exists() {
            let text = read_text(&event_scripts)?;
            let (_updated, changed) = update_event_scripts_text(&text, &old_dir, &new_dir);
            if changed {
                print_edit(&mut output, &mut printed_edits, EVENT_SCRIPTS);
            }
        }

        for map_json in iter_all_map_json(root)? {
            if map_json
                .parent()
                .and_then(Path::file_name)
                .map(|name| name == old_dir.as_str())
                .unwrap_or(false)
            {
                continue;
            }
            let mut data = read_json(&map_json)?;
            let mut refs_changed = update_map_refs_value(&mut data, &old_ids, &new_id);
            for (old_mapsec, new_mapsec) in &mapsec_renames {
                refs_changed |= update_mapsec_refs_value(&mut data, old_mapsec, new_mapsec);
            }
            if refs_changed {
                print_edit(
                    &mut output,
                    &mut printed_edits,
                    &display_path(root, &map_json),
                );
            }
        }
    }

    let mut region_layout_changed = BTreeSet::new();
    for cell_plan in plan_array(plan, "regionMapCells") {
        let region = plan_string_required(cell_plan, "region")?;
        let layout_path = region_map_layout_path(&region)
            .ok_or_else(|| format!("unknown region map layout region {region:?}"))?;
        if region_layout_changed.contains(layout_path) {
            continue;
        }
        let text = read_text(&root.join(layout_path))?;
        if region_map_layout_would_change(&text, cell_plan)? {
            region_layout_changed.insert(layout_path.to_string());
        }
    }
    for path in region_layout_changed {
        print_edit(&mut output, &mut printed_edits, &path);
    }

    if !plan_array(plan, "flagClaims").is_empty() {
        let text = read_text(&root.join(FLAGS_H))?;
        let mut changed = false;
        for claim in plan_array(plan, "flagClaims") {
            changed |= flag_claim_would_change(&text, claim)?;
        }
        if changed {
            print_edit(&mut output, &mut printed_edits, FLAGS_H);
        }
    }

    if !plan_array(plan, "flyLocations").is_empty()
        || !plan_array(plan, "flyMapsecTypes").is_empty()
        || !plan_array(plan, "flyIconStyles").is_empty()
        || !plan_array(plan, "mapsecMaps").is_empty()
    {
        let region_map_path = root.join(REGION_MAP_C);
        if region_map_path.exists() {
            print_edit(&mut output, &mut printed_edits, REGION_MAP_C);
        }
    }

    let tokens = remaining_ref_tokens(plan);
    let remaining = scan_text_refs(root, &tokens)?;
    if !remaining.is_empty() {
        output.push("REVIEW remaining textual references:".to_string());
        for path in remaining {
            output.push(format!("  {path}"));
        }
    }
    output.push("Dry-run complete; no files changed.".to_string());
    Ok(format!("{}\n", output.join("\n")))
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

fn read_text(path: &Path) -> Result<String, String> {
    fs::read_to_string(path).map_err(|err| format!("failed to read {}: {err}", path.display()))
}

fn plan_array<'a>(value: &'a Value, key: &str) -> Vec<&'a Value> {
    value
        .get(key)
        .and_then(Value::as_array)
        .map(|items| items.iter().collect())
        .unwrap_or_default()
}

fn map_plans(plan: &Value) -> Vec<&Value> {
    plan_array(plan, "maps")
}

fn layout_plans(plan: &Value) -> Vec<&Value> {
    plan_array(plan, "layouts")
}

fn mapsec_plans(plan: &Value) -> Vec<&Value> {
    plan_array(plan, "mapsecs")
}

fn plan_string(value: &Value, key: &str) -> Option<String> {
    value
        .get(key)
        .and_then(Value::as_str)
        .filter(|value| !value.is_empty())
        .map(ToString::to_string)
}

fn plan_string_required(value: &Value, key: &str) -> Result<String, String> {
    plan_string(value, key).ok_or_else(|| format!("plan field {key:?} must be a non-empty string"))
}

fn plan_bool(value: &Value, key: &str) -> Option<bool> {
    value.get(key).and_then(Value::as_bool)
}

fn string_vec(value: &Value, key: &str) -> Vec<String> {
    value
        .get(key)
        .and_then(Value::as_array)
        .map(|items| {
            items
                .iter()
                .filter_map(Value::as_str)
                .filter(|item| !item.is_empty())
                .map(ToString::to_string)
                .collect()
        })
        .unwrap_or_default()
}

fn map_old_dir(map_plan: &Value) -> Result<String, String> {
    plan_string(map_plan, "oldDirName")
        .or_else(|| plan_string(map_plan, "oldName"))
        .ok_or_else(|| "map plan must include oldDirName or oldName".to_string())
}

fn map_new_dir(map_plan: &Value) -> Result<String, String> {
    plan_string(map_plan, "newDirName")
        .or_else(|| plan_string(map_plan, "newName"))
        .ok_or_else(|| "map plan must include newDirName or newName".to_string())
}

fn map_old_group_names(map_plan: &Value) -> Result<Vec<String>, String> {
    let names = string_vec(map_plan, "groupOldNames");
    if names.is_empty() {
        Ok(vec![plan_string_required(map_plan, "oldName")?])
    } else {
        Ok(names)
    }
}

fn map_old_ids(map_plan: &Value) -> Result<BTreeSet<String>, String> {
    let mut old_ids = string_vec(map_plan, "oldIds")
        .into_iter()
        .collect::<BTreeSet<_>>();
    if old_ids.is_empty() {
        if let Some(old_id) = plan_string(map_plan, "oldId") {
            old_ids.insert(old_id);
        }
    }
    Ok(old_ids)
}

fn print_edit(output: &mut Vec<String>, printed: &mut BTreeSet<String>, path: &str) {
    if printed.insert(path.to_string()) {
        output.push(format!("EDIT {path}"));
    }
}

fn value_object_mut(value: &mut Value) -> Result<&mut serde_json::Map<String, Value>, String> {
    value
        .as_object_mut()
        .ok_or_else(|| "expected JSON object".to_string())
}

fn value_array_mut<'a>(
    value: &'a mut Value,
    key: &str,
) -> Result<Option<&'a mut Vec<Value>>, String> {
    let Some(field) = value_object_mut(value)?.get_mut(key) else {
        return Ok(None);
    };
    field
        .as_array_mut()
        .map(Some)
        .ok_or_else(|| format!("JSON field {key:?} must be an array"))
}

fn group_order_strings(groups_data: &Value) -> Result<Vec<String>, String> {
    groups_data
        .get("group_order")
        .and_then(Value::as_array)
        .ok_or_else(|| "map_groups.json field 'group_order' must be a list.".to_string())
        .map(|items| {
            items
                .iter()
                .filter_map(Value::as_str)
                .map(ToString::to_string)
                .collect()
        })
}

fn ensure_map_group_array<'a>(
    groups_data: &'a mut Value,
    group: &str,
) -> Result<&'a mut Vec<Value>, String> {
    let object = value_object_mut(groups_data)?;
    {
        let order = object
            .entry("group_order")
            .or_insert_with(|| json!([]))
            .as_array_mut()
            .ok_or_else(|| "map_groups.json field 'group_order' must be a list.".to_string())?;
        if !order.iter().any(|entry| entry.as_str() == Some(group)) {
            order.push(Value::String(group.to_string()));
        }
    }
    object
        .entry(group.to_string())
        .or_insert_with(|| json!([]))
        .as_array_mut()
        .ok_or_else(|| format!("map_groups.json field {group:?} must be a list."))
}

fn remove_map_names(maps: &mut Vec<Value>, old_names: &BTreeSet<String>) -> (bool, Option<usize>) {
    let mut changed = false;
    let mut first_index = None;
    let mut index = 0;
    while index < maps.len() {
        let should_remove = maps[index]
            .as_str()
            .map(|map_name| old_names.contains(map_name))
            .unwrap_or(false);
        if should_remove {
            first_index.get_or_insert(index);
            maps.remove(index);
            changed = true;
        } else {
            index += 1;
        }
    }
    (changed, first_index)
}

fn update_map_groups_value(
    groups_data: &mut Value,
    old_names: &[String],
    new_name: &str,
    from_group: Option<&str>,
    to_group: Option<&str>,
    preferred_group: Option<&str>,
) -> Result<bool, String> {
    let before = groups_data.clone();
    let old_names = old_names.iter().cloned().collect::<BTreeSet<_>>();
    if let Some(to_group) = to_group {
        let mut insert_index = None;
        for group in group_order_strings(groups_data)? {
            if from_group.is_some_and(|from_group| from_group != group) {
                continue;
            }
            if let Some(maps) = value_array_mut(groups_data, &group)? {
                let (removed, first_index) = remove_map_names(maps, &old_names);
                if removed && group == to_group && insert_index.is_none() {
                    insert_index = first_index;
                }
            }
        }
        let target_maps = ensure_map_group_array(groups_data, to_group)?;
        if !target_maps
            .iter()
            .any(|entry| entry.as_str() == Some(new_name))
        {
            let value = Value::String(new_name.to_string());
            if let Some(index) = insert_index.filter(|index| *index <= target_maps.len()) {
                target_maps.insert(index, value);
            } else {
                target_maps.push(value);
            }
        }
        return Ok(*groups_data != before);
    }

    let mut found = false;
    for group in group_order_strings(groups_data)? {
        if from_group.is_some_and(|from_group| from_group != group) {
            continue;
        }
        if let Some(maps) = value_array_mut(groups_data, &group)? {
            for entry in maps {
                if entry
                    .as_str()
                    .map(|map_name| old_names.contains(map_name))
                    .unwrap_or(false)
                {
                    found = true;
                    if entry.as_str() != Some(new_name) {
                        *entry = Value::String(new_name.to_string());
                    }
                }
            }
        }
    }
    if !found {
        if let Some(group) = preferred_group {
            let maps = ensure_map_group_array(groups_data, group)?;
            if !maps.iter().any(|entry| entry.as_str() == Some(new_name)) {
                maps.push(Value::String(new_name.to_string()));
            }
        }
    }
    Ok(*groups_data != before)
}

fn drop_empty_map_groups_value(groups_data: &mut Value, groups: &[String]) -> Result<bool, String> {
    let before = groups_data.clone();
    for group in groups.iter().filter(|group| !group.is_empty()) {
        if let Some(maps) = groups_data.get(group).and_then(Value::as_array) {
            if !maps.is_empty() {
                return Err(format!("Refusing to drop non-empty map group {group:?}."));
            }
        }
        if let Some(order) = value_array_mut(groups_data, "group_order")? {
            order.retain(|entry| entry.as_str() != Some(group));
        }
        if let Some(object) = groups_data.as_object_mut() {
            object.remove(group);
        }
    }
    Ok(*groups_data != before)
}

fn set_string_field(object: &mut serde_json::Map<String, Value>, key: &str, value: &str) -> bool {
    if object.get(key).and_then(Value::as_str) == Some(value) {
        return false;
    }
    object.insert(key.to_string(), Value::String(value.to_string()));
    true
}

fn set_bool_field(object: &mut serde_json::Map<String, Value>, key: &str, value: bool) -> bool {
    if object.get(key).and_then(Value::as_bool) == Some(value) {
        return false;
    }
    object.insert(key.to_string(), Value::Bool(value));
    true
}

fn update_map_json_value(
    data: &mut Value,
    old_ids: &BTreeSet<String>,
    new_id: &str,
    new_name: &str,
    new_layout: Option<&str>,
    new_mapsec: Option<&str>,
    map_type: Option<&str>,
    show_map_name: Option<bool>,
) -> bool {
    let Some(object) = data.as_object_mut() else {
        return false;
    };
    let mut changed = false;
    if object
        .get("id")
        .and_then(Value::as_str)
        .map(|id| old_ids.contains(id) && id != new_id)
        .unwrap_or(false)
    {
        object.insert("id".to_string(), Value::String(new_id.to_string()));
        changed = true;
    }
    changed |= set_string_field(object, "name", new_name);
    if let Some(new_layout) = new_layout {
        changed |= set_string_field(object, "layout", new_layout);
    }
    if let Some(new_mapsec) = new_mapsec {
        changed |= set_string_field(object, "region_map_section", new_mapsec);
    }
    if let Some(map_type) = map_type {
        changed |= set_string_field(object, "map_type", map_type);
    }
    if let Some(show_map_name) = show_map_name {
        changed |= set_bool_field(object, "show_map_name", show_map_name);
    }
    changed
}

fn update_map_refs_value(data: &mut Value, old_ids: &BTreeSet<String>, new_id: &str) -> bool {
    let mut changed = false;
    if let Some(connections) = data.get_mut("connections").and_then(Value::as_array_mut) {
        for connection in connections.iter_mut().filter_map(Value::as_object_mut) {
            if connection
                .get("map")
                .and_then(Value::as_str)
                .map(|map| old_ids.contains(map) && map != new_id)
                .unwrap_or(false)
            {
                connection.insert("map".to_string(), Value::String(new_id.to_string()));
                changed = true;
            }
        }
    }
    if let Some(warps) = data.get_mut("warp_events").and_then(Value::as_array_mut) {
        for warp in warps.iter_mut().filter_map(Value::as_object_mut) {
            if warp
                .get("dest_map")
                .and_then(Value::as_str)
                .map(|dest_map| old_ids.contains(dest_map) && dest_map != new_id)
                .unwrap_or(false)
            {
                warp.insert("dest_map".to_string(), Value::String(new_id.to_string()));
                changed = true;
            }
        }
    }
    changed
}

fn update_mapsec_refs_value(data: &mut Value, old_id: &str, new_id: &str) -> bool {
    let Some(object) = data.as_object_mut() else {
        return false;
    };
    if object.get("region_map_section").and_then(Value::as_str) == Some(old_id) && old_id != new_id
    {
        object.insert(
            "region_map_section".to_string(),
            Value::String(new_id.to_string()),
        );
        true
    } else {
        false
    }
}

fn update_mapsecs_json_value(data: &mut Value, mapsec_plan: &Value) -> Result<bool, String> {
    let before = data.clone();
    let old_id = plan_string_required(mapsec_plan, "oldId")?;
    let new_id = plan_string_required(mapsec_plan, "newId")?;
    let Some(sections) = data.get_mut("map_sections").and_then(Value::as_array_mut) else {
        return Ok(false);
    };
    for section in sections.iter_mut().filter_map(Value::as_object_mut) {
        if section.get("id").and_then(Value::as_str) != Some(old_id.as_str()) {
            continue;
        }
        section.insert("id".to_string(), Value::String(new_id));
        if let Some(new_name) = plan_string(mapsec_plan, "newName") {
            section.insert("name".to_string(), Value::String(new_name));
        }
        if let Some(bounds) = mapsec_plan.get("bounds").and_then(Value::as_object) {
            for field in ["x", "y", "width", "height"] {
                if let Some(value) = bounds.get(field) {
                    section.insert(field.to_string(), value.clone());
                }
            }
        }
        break;
    }
    Ok(*data != before)
}

fn update_layouts_json_value(data: &mut Value, layout_plan: &Value) -> Result<bool, String> {
    let before = data.clone();
    let old_id = plan_string_required(layout_plan, "oldId")?;
    let new_id = plan_string_required(layout_plan, "newId")?;
    let Some(layouts) = data.get_mut("layouts").and_then(Value::as_array_mut) else {
        return Ok(false);
    };
    for layout in layouts.iter_mut().filter_map(Value::as_object_mut) {
        if layout.get("id").and_then(Value::as_str) != Some(old_id.as_str()) {
            continue;
        }
        layout.insert("id".to_string(), Value::String(new_id));
        if let Some(name) = plan_string(layout_plan, "newName") {
            layout.insert("name".to_string(), Value::String(name));
        }
        if let Some(primary) = plan_string(layout_plan, "primaryTileset") {
            layout.insert("primary_tileset".to_string(), Value::String(primary));
        }
        if let Some(secondary) = plan_string(layout_plan, "secondaryTileset") {
            layout.insert("secondary_tileset".to_string(), Value::String(secondary));
        }
        if let (Some(old_dir), Some(new_dir)) = (
            plan_string(layout_plan, "oldDir"),
            plan_string(layout_plan, "newDir"),
        ) {
            for field in ["border_filepath", "blockdata_filepath"] {
                if let Some(value) = layout.get(field).and_then(Value::as_str) {
                    if let Some(updated) = replace_path_prefix(value, &old_dir, &new_dir) {
                        layout.insert(field.to_string(), Value::String(updated));
                    }
                }
            }
        }
        break;
    }
    Ok(*data != before)
}

fn replace_path_prefix(value: &str, old_dir: &str, new_dir: &str) -> Option<String> {
    let old_dir = old_dir.trim_end_matches('/');
    let new_dir = new_dir.trim_end_matches('/');
    let prefix = format!("{old_dir}/");
    value
        .starts_with(&prefix)
        .then(|| format!("{new_dir}{}", &value[old_dir.len()..]))
}

fn update_event_scripts_text(text: &str, old_name: &str, new_name: &str) -> (String, bool) {
    let old = format!(".include \"data/maps/{old_name}/scripts.inc\"");
    let new = format!(".include \"data/maps/{new_name}/scripts.inc\"");
    let mut changed = false;
    let mut text = text.to_string();
    if old != new && text.contains(&old) {
        text = text.replace(&old, &new);
        changed = true;
    }
    let mut seen = false;
    let mut normalized = String::new();
    for line in text.split_inclusive('\n') {
        if line.trim() == new {
            if seen {
                changed = true;
                continue;
            }
            seen = true;
        }
        normalized.push_str(line);
    }
    (normalized, changed)
}

fn update_script_label_prefixes_text(
    text: &str,
    old_prefixes: &[String],
    new_prefix: &str,
) -> (String, bool) {
    let mut text = text.to_string();
    let mut changed = false;
    let mut prefixes = old_prefixes
        .iter()
        .filter(|prefix| !prefix.is_empty() && prefix.as_str() != new_prefix)
        .cloned()
        .collect::<BTreeSet<_>>()
        .into_iter()
        .collect::<Vec<_>>();
    prefixes.sort_by_key(|prefix| std::cmp::Reverse(prefix.len()));
    for prefix in prefixes {
        let mut updated = String::new();
        for line in text.split_inclusive('\n') {
            if line.trim_start().starts_with(".string") {
                updated.push_str(line);
                continue;
            }
            let line_updated = replace_label_prefix(line, &prefix, new_prefix);
            if line_updated != line {
                changed = true;
            }
            updated.push_str(&line_updated);
        }
        text = updated;
    }
    (text, changed)
}

fn replace_label_prefix(line: &str, old_prefix: &str, new_prefix: &str) -> String {
    let mut result = String::new();
    let mut cursor = 0;
    while let Some(relative) = line[cursor..].find(old_prefix) {
        let start = cursor + relative;
        let end = start + old_prefix.len();
        let bytes = line.as_bytes();
        let before_ok = start == 0 || !is_identifier_byte(bytes[start - 1]);
        let after_ok = bytes.get(end) == Some(&b'_');
        if before_ok && after_ok {
            result.push_str(&line[cursor..start]);
            result.push_str(new_prefix);
            cursor = end;
        } else {
            result.push_str(&line[cursor..end]);
            cursor = end;
        }
    }
    result.push_str(&line[cursor..]);
    result
}

fn is_identifier_byte(byte: u8) -> bool {
    byte.is_ascii_alphanumeric() || byte == b'_'
}

fn transition_setflag_would_change(text: &str, map_name: &str, flag: &str) -> Result<bool, String> {
    if text.contains(&format!("setflag {flag}")) {
        return Ok(false);
    }
    let header = format!("{map_name}_MapScripts::");
    let Some(header_index) = text.find(&header) else {
        return Err(format!("Could not find map script header {header}."));
    };
    if text[header_index..].find("\t.byte 0").is_none() {
        return Err(format!(
            "Could not find map script terminator for {map_name}."
        ));
    }
    Ok(true)
}

fn region_map_layout_path(region: &str) -> Option<&'static str> {
    REGION_MAP_LAYOUT_FILES
        .iter()
        .find_map(|(candidate, path)| (*candidate == region).then_some(*path))
}

fn region_map_layout_would_change(text: &str, cell_plan: &Value) -> Result<bool, String> {
    let target_x = cell_plan
        .get("x")
        .and_then(Value::as_i64)
        .ok_or_else(|| "regionMapCells entry requires x".to_string())?;
    let target_y = cell_plan
        .get("y")
        .and_then(Value::as_i64)
        .ok_or_else(|| "regionMapCells entry requires y".to_string())?;
    let mapsec = plan_string_required(cell_plan, "mapsec")?;
    if target_x < 0 || target_y < 0 {
        return Err("--set-region-map-cell coordinates must be non-negative.".to_string());
    }
    let mut row_index = 0i64;
    for line in text.lines() {
        let trimmed = line.trim();
        if !trimmed.starts_with('{') || !trimmed.contains('}') {
            continue;
        }
        let Some(start) = trimmed.find('{') else {
            continue;
        };
        let Some(end) = trimmed.rfind('}') else {
            continue;
        };
        if row_index == target_y {
            let cells = trimmed[start + 1..end]
                .split(',')
                .map(str::trim)
                .collect::<Vec<_>>();
            let x = target_x as usize;
            if x >= cells.len() {
                return Err(format!(
                    "Region map x={target_x} is outside row width {}.",
                    cells.len()
                ));
            }
            return Ok(cells[x] != mapsec);
        }
        row_index += 1;
    }
    Err(format!(
        "Region map y={target_y} is outside row count {row_index}."
    ))
}

fn flag_claim_would_change(text: &str, claim: &Value) -> Result<bool, String> {
    let old_flag = plan_string_required(claim, "oldFlag")?;
    let new_flag = plan_string_required(claim, "newFlag")?;
    let old_define = format!("#define {old_flag}");
    if text.lines().any(|line| line.starts_with(&old_define)) {
        return Ok(true);
    }
    let new_define = format!("#define {new_flag}");
    if text.lines().any(|line| line.starts_with(&new_define)) {
        return Ok(false);
    }
    Err(format!(
        "Could not find unused flag {old_flag} in include/constants/flags.h."
    ))
}

fn iter_all_map_json(root: &Path) -> Result<Vec<PathBuf>, String> {
    let maps_dir = root.join(MAPS_DIR);
    let mut paths = Vec::new();
    for entry in fs::read_dir(&maps_dir)
        .map_err(|err| format!("failed to read {}: {err}", maps_dir.display()))?
    {
        let entry = entry.map_err(|err| format!("failed to read map entry: {err}"))?;
        if entry
            .file_type()
            .map_err(|err| format!("failed to read file type for {:?}: {err}", entry.path()))?
            .is_dir()
        {
            let map_json = entry.path().join("map.json");
            if map_json.is_file() {
                paths.push(map_json);
            }
        }
    }
    paths.sort();
    Ok(paths)
}

fn remaining_ref_tokens(plan: &Value) -> Vec<String> {
    let mut tokens = BTreeSet::new();
    for map_plan in map_plans(plan) {
        let new_name = plan_string(map_plan, "newName");
        let new_dir = plan_string(map_plan, "newDirName");
        for token in string_vec(map_plan, "groupOldNames") {
            if Some(token.as_str()) != new_name.as_deref()
                && Some(token.as_str()) != new_dir.as_deref()
            {
                tokens.insert(token);
            }
        }
        let new_id = plan_string(map_plan, "newId");
        for token in string_vec(map_plan, "oldIds") {
            if Some(token.as_str()) != new_id.as_deref() {
                tokens.insert(token);
            }
        }
    }
    for layout_plan in layout_plans(plan) {
        let new_id = plan_string(layout_plan, "newId");
        let new_name = plan_string(layout_plan, "newName");
        for token in [
            plan_string(layout_plan, "oldId"),
            plan_string(layout_plan, "oldName"),
        ]
        .into_iter()
        .flatten()
        {
            if Some(token.as_str()) != new_id.as_deref()
                && Some(token.as_str()) != new_name.as_deref()
            {
                tokens.insert(token);
            }
        }
    }
    for mapsec_plan in mapsec_plans(plan) {
        let old_id = plan_string(mapsec_plan, "oldId");
        let new_id = plan_string(mapsec_plan, "newId");
        if old_id != new_id {
            if let Some(old_id) = old_id {
                tokens.insert(old_id);
            }
        }
    }
    tokens.extend(string_vec(plan, "dropGroups"));
    tokens
        .into_iter()
        .filter(|token| !token.is_empty())
        .collect()
}

fn scan_text_refs(root: &Path, tokens: &[String]) -> Result<Vec<String>, String> {
    if tokens.is_empty() {
        return Ok(Vec::new());
    }
    let mut files = Vec::new();
    collect_search_files(&root.join("data/maps"), &mut files)?;
    collect_search_files(&root.join("data/layouts"), &mut files)?;
    collect_search_files(&root.join(EVENT_SCRIPTS), &mut files)?;

    let mut matches = BTreeSet::new();
    for path in files {
        if is_generated_source(root, &path) {
            continue;
        }
        let Some(extension) = path.extension().and_then(|extension| extension.to_str()) else {
            continue;
        };
        if !matches!(extension, "json" | "inc" | "s") {
            continue;
        }
        let Ok(text) = fs::read_to_string(&path) else {
            continue;
        };
        if tokens.iter().any(|token| has_token_reference(&text, token)) {
            matches.insert(display_path(root, &path));
        }
    }
    Ok(matches.into_iter().collect())
}

fn collect_search_files(path: &Path, files: &mut Vec<PathBuf>) -> Result<(), String> {
    if !path.exists() {
        return Ok(());
    }
    if path.is_file() {
        files.push(path.to_path_buf());
        return Ok(());
    }
    for entry in
        fs::read_dir(path).map_err(|err| format!("failed to read {}: {err}", path.display()))?
    {
        let entry = entry.map_err(|err| format!("failed to read directory entry: {err}"))?;
        let path = entry.path();
        if path.is_dir() {
            collect_search_files(&path, files)?;
        } else if path.is_file() {
            files.push(path);
        }
    }
    Ok(())
}

fn is_generated_source(root: &Path, path: &Path) -> bool {
    let relative = display_path(root, path);
    if GENERATED_OUTPUTS.contains(&relative.as_str()) {
        return true;
    }
    let relative_path = Path::new(&relative);
    relative_path.starts_with("data/maps")
        && matches!(
            relative_path.file_name().and_then(|name| name.to_str()),
            Some("header.inc" | "events.inc" | "connections.inc")
        )
}

fn has_token_reference(text: &str, token: &str) -> bool {
    let mut cursor = 0;
    while let Some(relative) = text[cursor..].find(token) {
        let start = cursor + relative;
        let end = start + token.len();
        let bytes = text.as_bytes();
        let before_ok = start == 0 || !bytes[start - 1].is_ascii_alphanumeric();
        let after_ok = bytes
            .get(end)
            .map(|byte| !byte.is_ascii_alphanumeric())
            .unwrap_or(true);
        if before_ok && after_ok {
            return true;
        }
        cursor = end;
    }
    false
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
