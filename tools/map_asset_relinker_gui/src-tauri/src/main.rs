use serde::{Deserialize, Serialize};
use serde_json::Value;
use std::collections::{BTreeMap, BTreeSet};
use std::env;
use std::fs;
use std::path::{Path, PathBuf};
use std::process::Command;

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct ProjectSummary {
    root: String,
    map_count: usize,
    group_count: usize,
    layout_count: usize,
    mapsec_count: usize,
    warning_count: usize,
    maps: Vec<MapSummary>,
    warnings: Vec<String>,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct MapSummary {
    directory_name: String,
    name: String,
    id: String,
    layout: String,
    layout_name: Option<String>,
    mapsec: String,
    mapsec_name: Option<String>,
    mapsec_position: Option<String>,
    map_type: String,
    show_map_name: bool,
    group: Option<String>,
    group_count: usize,
    issues: Vec<String>,
}

#[derive(Debug, Deserialize)]
#[serde(rename_all = "camelCase")]
struct PlanOptions {
    root: String,
    old_name: String,
    new_name: String,
    target_group: String,
    rename_mapsec_from: String,
    rename_mapsec_to: String,
    new_mapsec_name: String,
    rename_layout: bool,
    rewrite_script_labels: bool,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct DryRunResult {
    plan_path: String,
    plan_stdout: String,
    dry_run_stdout: String,
    stderr: String,
    command: String,
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

#[tauri::command]
fn scan_project(root: Option<String>) -> Result<ProjectSummary, String> {
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

#[tauri::command]
fn run_plan_dry_run(options: PlanOptions) -> Result<DryRunResult, String> {
    let root = resolve_project_root(Some(options.root))?;
    let safe_name = sanitize_for_file(if options.new_name.is_empty() {
        &options.old_name
    } else {
        &options.new_name
    });
    let plan_path = env::temp_dir().join(format!(
        "{safe_name}_relink_{}.json",
        chrono_like_timestamp()
    ));
    let script_path = root.join("tools/map_asset_relinker/map_relink.py");
    let python = env::var("PYTHON").unwrap_or_else(|_| "python3".to_string());

    let mut plan_args = vec![
        script_path.display().to_string(),
        "--root".to_string(),
        root.display().to_string(),
        "plan".to_string(),
        "--map".to_string(),
        format!("{}:{}", options.old_name, options.new_name),
    ];
    if !options.target_group.is_empty() {
        plan_args.push("--to-group".to_string());
        plan_args.push(options.target_group.clone());
    }
    let should_rename_mapsec = !options.rename_mapsec_from.is_empty()
        && !options.rename_mapsec_to.is_empty()
        && options.rename_mapsec_from != options.rename_mapsec_to;
    if should_rename_mapsec {
        plan_args.push("--rename-mapsec".to_string());
        plan_args.push(format!(
            "{}:{}",
            options.rename_mapsec_from, options.rename_mapsec_to
        ));
    }
    if should_rename_mapsec && !options.new_mapsec_name.is_empty() {
        plan_args.push("--new-mapsec-name".to_string());
        plan_args.push(options.new_mapsec_name.clone());
    }
    if !options.rename_layout {
        plan_args.push("--no-layout-rename".to_string());
    }
    if options.rewrite_script_labels {
        plan_args.push("--rewrite-script-labels".to_string());
    }
    plan_args.push("--out".to_string());
    plan_args.push(plan_path.display().to_string());

    let plan = run_python_command(&python, &plan_args, &root)?;
    let dry_run_args = vec![
        script_path.display().to_string(),
        "--root".to_string(),
        root.display().to_string(),
        "apply".to_string(),
        "--dry-run".to_string(),
        plan_path.display().to_string(),
    ];
    let dry_run = run_python_command(&python, &dry_run_args, &root)?;

    Ok(DryRunResult {
        plan_path: plan_path.display().to_string(),
        plan_stdout: plan.0,
        dry_run_stdout: dry_run.0,
        stderr: [plan.1, dry_run.1]
            .into_iter()
            .filter(|part| !part.is_empty())
            .collect::<Vec<_>>()
            .join("\n"),
        command: format!(
            "{}\n{}",
            command_line(&python, &plan_args),
            command_line(&python, &dry_run_args)
        ),
    })
}

fn resolve_project_root(root: Option<String>) -> Result<PathBuf, String> {
    if let Some(root) = root {
        let root = PathBuf::from(root);
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

fn run_python_command(
    python: &str,
    args: &[String],
    cwd: &Path,
) -> Result<(String, String), String> {
    let output = Command::new(python)
        .args(args)
        .current_dir(cwd)
        .output()
        .map_err(|err| format!("failed to run {python}: {err}"))?;
    let stdout = String::from_utf8_lossy(&output.stdout).to_string();
    let stderr = String::from_utf8_lossy(&output.stderr).to_string();
    if !output.status.success() {
        return Err(format!(
            "{} failed with {}\n{}\n{}",
            command_line(python, args),
            output.status,
            stdout,
            stderr
        ));
    }
    Ok((stdout, stderr))
}

fn sanitize_for_file(value: &str) -> String {
    let sanitized = value
        .chars()
        .map(|ch| {
            if ch.is_ascii_alphanumeric() || ch == '_' {
                ch.to_ascii_lowercase()
            } else {
                '_'
            }
        })
        .collect::<String>();
    if sanitized.is_empty() {
        "map".to_string()
    } else {
        sanitized
    }
}

fn chrono_like_timestamp() -> u128 {
    std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .map(|duration| duration.as_millis())
        .unwrap_or(0)
}

fn command_line(program: &str, args: &[String]) -> String {
    std::iter::once(program.to_string())
        .chain(args.iter().cloned())
        .map(|arg| shell_quote(&arg))
        .collect::<Vec<_>>()
        .join(" ")
}

fn shell_quote(value: &str) -> String {
    if value
        .chars()
        .all(|ch| ch.is_ascii_alphanumeric() || "_./:=-".contains(ch))
    {
        value.to_string()
    } else {
        format!("'{}'", value.replace('\'', "'\\''"))
    }
}

fn main() {
    tauri::Builder::default()
        .invoke_handler(tauri::generate_handler![scan_project, run_plan_dry_run])
        .run(tauri::generate_context!())
        .expect("error while running Map Asset Relinker GUI");
}
