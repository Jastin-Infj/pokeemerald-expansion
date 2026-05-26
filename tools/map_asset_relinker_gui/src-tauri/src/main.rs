use map_asset_relinker_core::{
    apply_plan_dry_run, make_plan, resolve_project_root, scan_project as scan_project_core,
    write_plan, PlanRequest, ProjectSummary,
};
use serde::{Deserialize, Serialize};
use std::env;
use std::path::{Path, PathBuf};
use std::process::Command;

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

#[tauri::command]
fn scan_project(root: Option<String>) -> Result<ProjectSummary, String> {
    scan_project_core(root.map(PathBuf::from))
}

#[tauri::command]
fn run_plan_dry_run(options: PlanOptions) -> Result<DryRunResult, String> {
    run_plan_command(options, true)
}

#[tauri::command]
fn run_plan_apply(options: PlanOptions) -> Result<DryRunResult, String> {
    run_plan_command(options, false)
}

fn run_plan_command(options: PlanOptions, dry_run: bool) -> Result<DryRunResult, String> {
    let root = resolve_project_root(Some(PathBuf::from(options.root)))?;
    let safe_name = sanitize_for_file(if options.new_name.is_empty() {
        &options.old_name
    } else {
        &options.new_name
    });
    let plan_path = env::temp_dir().join(format!(
        "{safe_name}_relink_{}.json",
        chrono_like_timestamp()
    ));
    let should_rename_mapsec = !options.rename_mapsec_from.is_empty()
        && !options.rename_mapsec_to.is_empty()
        && options.rename_mapsec_from != options.rename_mapsec_to;
    let request = PlanRequest {
        old_map: options.old_name.clone(),
        new_map: options.new_name.clone(),
        match_by: "dir".to_string(),
        from_group: None,
        to_group: non_empty(options.target_group.clone()),
        group: None,
        rename_layout: options.rename_layout,
        rename_mapsec: should_rename_mapsec.then_some((
            options.rename_mapsec_from.clone(),
            options.rename_mapsec_to.clone(),
        )),
        new_mapsec_name: non_empty(options.new_mapsec_name.clone()),
        new_mapsec: None,
        set_layout_id: None,
        rewrite_script_labels: options.rewrite_script_labels,
    };
    let plan = make_plan(&root, &request)?;
    write_plan(&plan_path, &plan)?;
    let plan_stdout = format!("Wrote plan: {}\n", plan_path.display());

    let script_path = root.join("tools/map_asset_relinker/map_relink.py");
    let python = env::var("PYTHON").unwrap_or_else(|_| "python3".to_string());
    let mut stderr = String::new();
    let dry_run_stdout;
    let apply_command;
    if dry_run {
        dry_run_stdout = apply_plan_dry_run(&root, &plan)?;
        apply_command = core_apply_dry_run_command(&root, &plan_path);
    } else {
        let mut apply_args = vec![
            script_path.display().to_string(),
            "--root".to_string(),
            root.display().to_string(),
            "apply".to_string(),
        ];
        apply_args.push("--allow-dirty".to_string());
        apply_args.push(plan_path.display().to_string());
        let apply = run_python_command(&python, &apply_args, &root)?;
        dry_run_stdout = apply.0;
        stderr = apply.1;
        apply_command = command_line(&python, &apply_args);
    }

    Ok(DryRunResult {
        plan_path: plan_path.display().to_string(),
        plan_stdout,
        dry_run_stdout,
        stderr,
        command: format!(
            "{}\n{}",
            core_plan_command(&root, &request, &plan_path),
            apply_command,
        ),
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

fn non_empty(value: String) -> Option<String> {
    (!value.is_empty()).then_some(value)
}

fn core_plan_command(root: &Path, request: &PlanRequest, out: &Path) -> String {
    let mut args = vec![
        "tools/map_asset_relinker_core".to_string(),
        "plan".to_string(),
        "--root".to_string(),
        root.display().to_string(),
        "--map".to_string(),
        format!("{}:{}", request.old_map, request.new_map),
    ];
    if let Some(to_group) = &request.to_group {
        args.push("--to-group".to_string());
        args.push(to_group.clone());
    }
    if let Some((old_mapsec, new_mapsec)) = &request.rename_mapsec {
        args.push("--rename-mapsec".to_string());
        args.push(format!("{old_mapsec}:{new_mapsec}"));
    }
    if let Some(name) = &request.new_mapsec_name {
        args.push("--new-mapsec-name".to_string());
        args.push(name.clone());
    }
    if !request.rename_layout {
        args.push("--no-layout-rename".to_string());
    }
    if request.rewrite_script_labels {
        args.push("--rewrite-script-labels".to_string());
    }
    args.push("--out".to_string());
    args.push(out.display().to_string());
    args.into_iter()
        .map(|arg| shell_quote(&arg))
        .collect::<Vec<_>>()
        .join(" ")
}

fn core_apply_dry_run_command(root: &Path, plan_path: &Path) -> String {
    [
        "tools/map_asset_relinker_core".to_string(),
        "apply".to_string(),
        "--root".to_string(),
        root.display().to_string(),
        "--dry-run".to_string(),
        plan_path.display().to_string(),
    ]
    .into_iter()
    .map(|arg| shell_quote(&arg))
    .collect::<Vec<_>>()
    .join(" ")
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
        .invoke_handler(tauri::generate_handler![
            scan_project,
            run_plan_dry_run,
            run_plan_apply
        ])
        .run(tauri::generate_context!())
        .expect("error while running Map Asset Relinker GUI");
}
