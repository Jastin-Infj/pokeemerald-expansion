use map_asset_relinker_core::{
    apply_plan_dry_run, make_plan, resolve_project_root, scan_project as scan_project_core,
    write_plan, PlanRequest, ProjectSummary,
};
use serde::{Deserialize, Serialize};
use std::env;
use std::fs::{self, File, OpenOptions};
use std::io::{BufRead, BufReader, ErrorKind, Write};
use std::path::{Path, PathBuf};
use std::process::Command;
use std::sync::mpsc;
use std::time::Duration;
use tauri_plugin_dialog::DialogExt;

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
    plan_path: Option<String>,
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

#[derive(Debug, Deserialize)]
#[serde(rename_all = "camelCase")]
struct DiagnosticEvent {
    event: String,
    details: Option<serde_json::Value>,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct DiagnosticLogSnapshot {
    path: String,
    lines: Vec<String>,
}

#[tauri::command]
fn scan_project(root: Option<String>) -> Result<ProjectSummary, String> {
    scan_project_core(root.map(PathBuf::from))
}

#[tauri::command]
fn write_diagnostic_event(event: DiagnosticEvent) -> Result<String, String> {
    let path = diagnostic_log_path();
    if let Some(parent) = path.parent() {
        fs::create_dir_all(parent)
            .map_err(|err| format!("failed to create diagnostics directory: {err}"))?;
    }
    let record = serde_json::json!({
        "unixMs": chrono_like_timestamp(),
        "pid": std::process::id(),
        "version": env!("CARGO_PKG_VERSION"),
        "event": event.event,
        "details": event.details.unwrap_or_else(|| serde_json::json!({})),
    });
    let mut file = OpenOptions::new()
        .create(true)
        .append(true)
        .open(&path)
        .map_err(|err| format!("failed to open diagnostics log: {err}"))?;
    writeln!(file, "{record}").map_err(|err| format!("failed to write diagnostics log: {err}"))?;
    Ok(path.display().to_string())
}

#[tauri::command]
fn read_diagnostic_log(line_count: Option<usize>) -> Result<DiagnosticLogSnapshot, String> {
    let path = diagnostic_log_path();
    let limit = line_count.unwrap_or(120).clamp(1, 500);
    let lines = match File::open(&path) {
        Ok(file) => {
            let all_lines = BufReader::new(file)
                .lines()
                .collect::<Result<Vec<_>, _>>()
                .map_err(|err| format!("failed to read diagnostics log: {err}"))?;
            let start = all_lines.len().saturating_sub(limit);
            all_lines[start..].to_vec()
        }
        Err(err) if err.kind() == std::io::ErrorKind::NotFound => Vec::new(),
        Err(err) => return Err(format!("failed to open diagnostics log: {err}")),
    };
    Ok(DiagnosticLogSnapshot {
        path: path.display().to_string(),
        lines,
    })
}

#[tauri::command]
async fn choose_project_root(
    window: tauri::Window,
    current_root: Option<String>,
) -> Result<Option<String>, String> {
    let mut dialog = window
        .dialog()
        .file()
        .set_title("Select pokeemerald-expansion Project");
    #[cfg(any(windows, target_os = "macos"))]
    {
        dialog = dialog.set_parent(&window);
    }
    let starting_directory = current_root
        .as_deref()
        .map(str::trim)
        .filter(|root| !root.is_empty())
        .and_then(existing_directory_for_dialog);
    if let Some(starting_directory) = &starting_directory {
        dialog = dialog.set_directory(starting_directory);
    }
    let (sender, receiver) = mpsc::channel();
    dialog.pick_folder(move |folder| {
        let result = match folder {
            Some(folder) => folder
                .into_path()
                .map(|path| Some(path.display().to_string()))
                .map_err(|err| format!("failed to read selected folder path: {err}")),
            None => Ok(None),
        };
        let _ = sender.send(result);
    });
    tauri::async_runtime::spawn_blocking(move || {
        receiver
            .recv_timeout(Duration::from_secs(180))
            .map_err(|err| {
                let directory_note = starting_directory
                    .as_ref()
                    .map(|path| format!(" from {}", path.display()))
                    .unwrap_or_default();
                format!("folder picker did not return{directory_note}: {err}")
            })?
    })
    .await
    .map_err(|err| format!("folder picker task failed: {err}"))?
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
    let plan_path = if !dry_run {
        options
            .plan_path
            .as_deref()
            .filter(|path| !path.trim().is_empty())
            .map(PathBuf::from)
    } else {
        None
    }
    .unwrap_or_else(|| {
        env::temp_dir().join(format!(
            "{safe_name}_relink_{}.json",
            chrono_like_timestamp()
        ))
    });
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
    let using_reviewed_plan = !dry_run
        && options
            .plan_path
            .as_deref()
            .is_some_and(|path| !path.trim().is_empty());
    let plan_stdout;
    let plan;
    if using_reviewed_plan {
        plan_stdout = format!("Using reviewed plan: {}\n", plan_path.display());
        plan = serde_json::Value::Null;
    } else {
        plan = make_plan(&root, &request)?;
        write_plan(&plan_path, &plan)?;
        plan_stdout = format!("Wrote plan: {}\n", plan_path.display());
    }

    let script_path = root.join("tools/map_asset_relinker/map_relink.py");
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
        let apply = run_python_command(&apply_args, &root)?;
        dry_run_stdout = apply.0;
        stderr = apply.1;
        apply_command = apply.2;
    }

    Ok(DryRunResult {
        plan_path: plan_path.display().to_string(),
        plan_stdout,
        dry_run_stdout,
        stderr,
        command: format!(
            "{}\n{}",
            if using_reviewed_plan {
                format!(
                    "# reviewed plan: {}",
                    shell_quote(&plan_path.display().to_string())
                )
            } else {
                core_plan_command(&root, &request, &plan_path)
            },
            apply_command,
        ),
    })
}

fn run_python_command(args: &[String], cwd: &Path) -> Result<(String, String, String), String> {
    let mut launch_errors = Vec::new();
    for (program, prefix_args) in python_command_candidates() {
        let mut command_args = prefix_args;
        command_args.extend_from_slice(args);
        let command_text = command_line(&program, &command_args);
        let output = match Command::new(&program)
            .args(&command_args)
            .current_dir(cwd)
            .output()
        {
            Ok(output) => output,
            Err(err) if err.kind() == ErrorKind::NotFound => {
                launch_errors.push(format!("{program}: {err}"));
                continue;
            }
            Err(err) => return Err(format!("failed to run {command_text}: {err}")),
        };
        let stdout = String::from_utf8_lossy(&output.stdout).to_string();
        let stderr = String::from_utf8_lossy(&output.stderr).to_string();
        if !output.status.success() {
            return Err(format!(
                "{} failed with {}\n{}\n{}",
                command_text, output.status, stdout, stderr
            ));
        }
        return Ok((stdout, stderr, command_text));
    }
    Err(format!(
        "failed to run Python. Tried: {}",
        launch_errors.join("; ")
    ))
}

fn python_command_candidates() -> Vec<(String, Vec<String>)> {
    if let Ok(python) = env::var("PYTHON") {
        return vec![(python, Vec::new())];
    }
    if cfg!(windows) {
        vec![
            ("py".to_string(), vec!["-3".to_string()]),
            ("python".to_string(), Vec::new()),
            ("python3".to_string(), Vec::new()),
        ]
    } else {
        vec![
            ("python3".to_string(), Vec::new()),
            ("python".to_string(), Vec::new()),
        ]
    }
}

fn non_empty(value: String) -> Option<String> {
    (!value.is_empty()).then_some(value)
}

fn existing_directory_for_dialog(root: &str) -> Option<PathBuf> {
    let path = PathBuf::from(root);
    if path.is_dir() {
        return Some(path);
    }
    path.parent()
        .filter(|parent| parent.is_dir())
        .map(Path::to_path_buf)
}

fn diagnostic_log_path() -> PathBuf {
    diagnostic_base_dir()
        .join("Map Asset Relinker")
        .join("logs")
        .join("diagnostics.jsonl")
}

#[cfg(windows)]
fn diagnostic_base_dir() -> PathBuf {
    env::var_os("APPDATA")
        .map(PathBuf::from)
        .unwrap_or_else(env::temp_dir)
}

#[cfg(target_os = "macos")]
fn diagnostic_base_dir() -> PathBuf {
    env::var_os("HOME")
        .map(PathBuf::from)
        .map(|home| home.join("Library").join("Application Support"))
        .unwrap_or_else(env::temp_dir)
}

#[cfg(all(unix, not(target_os = "macos")))]
fn diagnostic_base_dir() -> PathBuf {
    env::var_os("XDG_DATA_HOME")
        .map(PathBuf::from)
        .or_else(|| {
            env::var_os("HOME")
                .map(PathBuf::from)
                .map(|home| home.join(".local").join("share"))
        })
        .unwrap_or_else(env::temp_dir)
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
        .plugin(tauri_plugin_dialog::init())
        .invoke_handler(tauri::generate_handler![
            scan_project,
            write_diagnostic_event,
            read_diagnostic_log,
            choose_project_root,
            run_plan_dry_run,
            run_plan_apply
        ])
        .run(tauri::generate_context!())
        .expect("error while running Map Asset Relinker GUI");
}
