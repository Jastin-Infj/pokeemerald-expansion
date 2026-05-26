use map_asset_relinker_core::{scan_project as scan_project_core, ProjectSummary};
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
    let mut apply_args = vec![
        script_path.display().to_string(),
        "--root".to_string(),
        root.display().to_string(),
        "apply".to_string(),
    ];
    if dry_run {
        apply_args.push("--dry-run".to_string());
    } else {
        apply_args.push("--allow-dirty".to_string());
    }
    apply_args.push(plan_path.display().to_string());
    let apply = run_python_command(&python, &apply_args, &root)?;

    Ok(DryRunResult {
        plan_path: plan_path.display().to_string(),
        plan_stdout: plan.0,
        dry_run_stdout: apply.0,
        stderr: [plan.1, apply.1]
            .into_iter()
            .filter(|part| !part.is_empty())
            .collect::<Vec<_>>()
            .join("\n"),
        command: format!(
            "{}\n{}",
            command_line(&python, &plan_args),
            command_line(&python, &apply_args)
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
        .invoke_handler(tauri::generate_handler![
            scan_project,
            run_plan_dry_run,
            run_plan_apply
        ])
        .run(tauri::generate_context!())
        .expect("error while running Map Asset Relinker GUI");
}
