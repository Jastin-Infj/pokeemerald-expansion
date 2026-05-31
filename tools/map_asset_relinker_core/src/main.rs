use map_asset_relinker_core::{
    apply_plan_dry_run, make_plan, scan_project, write_plan, PlanRequest,
};
use std::env;
use std::fs;
use std::io::{self, ErrorKind, Write};
use std::path::PathBuf;

fn main() {
    if let Err(err) = run() {
        eprintln!("error: {err}");
        std::process::exit(1);
    }
}

fn run() -> Result<(), String> {
    let mut args = env::args().skip(1);
    let command = args.next().unwrap_or_else(|| "help".to_string());
    match command.as_str() {
        "scan" => command_scan(args.collect()),
        "audit" => command_audit(args.collect()),
        "plan" => command_plan(args.collect()),
        "apply" => command_apply(args.collect()),
        "help" | "--help" | "-h" => {
            print_help();
            Ok(())
        }
        _ => Err(format!("unknown command {command}; run with --help")),
    }
}

fn command_apply(args: Vec<String>) -> Result<(), String> {
    let mut root = None;
    let mut dry_run = false;
    let mut plan_path = None;
    let mut iter = args.into_iter();
    while let Some(arg) = iter.next() {
        match arg.as_str() {
            "--root" => {
                root = Some(PathBuf::from(
                    iter.next()
                        .ok_or_else(|| "--root requires a path".to_string())?,
                ));
            }
            "--dry-run" => dry_run = true,
            "--help" | "-h" => {
                print_help();
                std::process::exit(0);
            }
            _ if arg.starts_with('-') => return Err(format!("unknown option {arg}")),
            _ => {
                if plan_path.is_some() {
                    return Err("apply accepts exactly one plan path".to_string());
                }
                plan_path = Some(PathBuf::from(arg));
            }
        }
    }
    if !dry_run {
        return Err(
            "Rust core apply currently supports --dry-run only; use the Python apply path for backup-backed writes."
                .to_string(),
        );
    }
    let plan_path = plan_path.ok_or_else(|| "apply requires a plan path".to_string())?;
    let root = map_asset_relinker_core::resolve_project_root(root)?;
    let plan_text = fs::read_to_string(&plan_path)
        .map_err(|err| format!("failed to read {}: {err}", plan_path.display()))?;
    let plan: serde_json::Value = serde_json::from_str(&plan_text)
        .map_err(|err| format!("invalid JSON in {}: {err}", plan_path.display()))?;
    write_stdout(&apply_plan_dry_run(&root, &plan)?)?;
    Ok(())
}

fn command_scan(args: Vec<String>) -> Result<(), String> {
    let (root, pretty) = parse_common_args(args)?;
    let summary = scan_project(root)?;
    let json = if pretty {
        serde_json::to_string_pretty(&summary)
    } else {
        serde_json::to_string(&summary)
    }
    .map_err(|err| format!("failed to encode scan summary: {err}"))?;
    write_stdout_line(&json)?;
    Ok(())
}

fn command_audit(args: Vec<String>) -> Result<(), String> {
    let (root, _pretty) = parse_common_args(args)?;
    let summary = scan_project(root)?;
    let mut output = String::new();
    for warning in &summary.warnings {
        output.push_str(&format!("WARNING: {warning}\n"));
    }
    output.push_str(&format!(
        "Audit complete: {} warning(s) across {} map(s).",
        summary.warning_count, summary.map_count
    ));
    output.push('\n');
    write_stdout(&output)?;
    Ok(())
}

fn command_plan(args: Vec<String>) -> Result<(), String> {
    let mut root = None;
    let mut map = None;
    let mut out = None;
    let mut from_group = None;
    let mut to_group = None;
    let mut rename_mapsec = None;
    let mut new_mapsec_name = None;
    let mut rename_layout = true;
    let mut rewrite_script_labels = false;
    let mut iter = args.into_iter();
    while let Some(arg) = iter.next() {
        match arg.as_str() {
            "--root" => {
                root = Some(PathBuf::from(
                    iter.next()
                        .ok_or_else(|| "--root requires a path".to_string())?,
                ));
            }
            "--map" => {
                map = Some(parse_pair(
                    &iter
                        .next()
                        .ok_or_else(|| "--map requires OLD:NEW".to_string())?,
                    "--map",
                )?);
            }
            "--out" => {
                out = Some(PathBuf::from(
                    iter.next()
                        .ok_or_else(|| "--out requires a path".to_string())?,
                ));
            }
            "--from-group" => {
                from_group = Some(
                    iter.next()
                        .ok_or_else(|| "--from-group requires a group".to_string())?,
                );
            }
            "--to-group" => {
                to_group = Some(
                    iter.next()
                        .ok_or_else(|| "--to-group requires a group".to_string())?,
                );
            }
            "--rename-mapsec" => {
                rename_mapsec = Some(parse_pair(
                    &iter
                        .next()
                        .ok_or_else(|| "--rename-mapsec requires OLD:NEW".to_string())?,
                    "--rename-mapsec",
                )?);
            }
            "--new-mapsec-name" => {
                new_mapsec_name = Some(
                    iter.next()
                        .ok_or_else(|| "--new-mapsec-name requires a name".to_string())?,
                );
            }
            "--no-layout-rename" => rename_layout = false,
            "--rewrite-script-labels" => rewrite_script_labels = true,
            "--help" | "-h" => {
                print_help();
                std::process::exit(0);
            }
            _ => return Err(format!("unknown option {arg}")),
        }
    }
    let Some((old_map, new_map)) = map else {
        return Err("--map OLD:NEW is required".to_string());
    };
    let root = map_asset_relinker_core::resolve_project_root(root)?;
    let request = PlanRequest {
        old_map,
        new_map,
        match_by: "dir".to_string(),
        from_group,
        to_group,
        group: None,
        rename_layout,
        rename_mapsec,
        new_mapsec_name,
        new_mapsec: None,
        set_layout_id: None,
        rewrite_script_labels,
    };
    let plan = make_plan(&root, &request)?;
    if let Some(out) = out {
        write_plan(&out, &plan)?;
        write_stdout_line(&format!("Wrote plan: {}", out.display()))?;
    } else {
        let json = serde_json::to_string_pretty(&plan)
            .map_err(|err| format!("failed to encode plan JSON: {err}"))?;
        write_stdout_line(&json)?;
    }
    Ok(())
}

fn parse_pair(raw: &str, option: &str) -> Result<(String, String), String> {
    let Some((left, right)) = raw.split_once(':') else {
        return Err(format!("{option} must be OLD:NEW"));
    };
    if left.is_empty() || right.is_empty() {
        return Err(format!("{option} must be OLD:NEW"));
    }
    Ok((left.to_string(), right.to_string()))
}

fn parse_common_args(args: Vec<String>) -> Result<(Option<PathBuf>, bool), String> {
    let mut root = None;
    let mut pretty = false;
    let mut iter = args.into_iter();
    while let Some(arg) = iter.next() {
        match arg.as_str() {
            "--root" => {
                let value = iter
                    .next()
                    .ok_or_else(|| "--root requires a path".to_string())?;
                root = Some(PathBuf::from(value));
            }
            "--pretty" => pretty = true,
            "--help" | "-h" => {
                print_help();
                std::process::exit(0);
            }
            _ => return Err(format!("unknown option {arg}")),
        }
    }
    Ok((root, pretty))
}

fn print_help() {
    let _ = write_stdout_line(
        "map-asset-relinker-core\n\nUSAGE:\n  map-asset-relinker-core scan [--root PATH] [--pretty]\n  map-asset-relinker-core audit [--root PATH]\n  map-asset-relinker-core plan --root PATH --map OLD:NEW [--to-group GROUP] [--rename-mapsec OLD:NEW] [--new-mapsec-name NAME] [--rewrite-script-labels] --out PATH\n  map-asset-relinker-core apply --root PATH --dry-run PLAN.json\n\nThis is the Rust core migration path. Real apply and backup-backed writes still use the Python path until Rust core reaches write parity.",
    );
}

fn write_stdout_line(text: &str) -> Result<(), String> {
    let mut line = String::from(text);
    line.push('\n');
    write_stdout(&line)
}

fn write_stdout(text: &str) -> Result<(), String> {
    let mut stdout = io::stdout();
    match stdout.write_all(text.as_bytes()) {
        Ok(()) => Ok(()),
        Err(err) if err.kind() == ErrorKind::BrokenPipe => std::process::exit(0),
        Err(err) => Err(format!("failed writing stdout: {err}")),
    }
}
