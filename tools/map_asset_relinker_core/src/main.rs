use map_asset_relinker_core::scan_project;
use std::env;
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
        "help" | "--help" | "-h" => {
            print_help();
            Ok(())
        }
        _ => Err(format!("unknown command {command}; run with --help")),
    }
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
    println!("{json}");
    Ok(())
}

fn command_audit(args: Vec<String>) -> Result<(), String> {
    let (root, _pretty) = parse_common_args(args)?;
    let summary = scan_project(root)?;
    for warning in &summary.warnings {
        println!("WARNING: {warning}");
    }
    println!(
        "Audit complete: {} warning(s) across {} map(s).",
        summary.warning_count, summary.map_count
    );
    Ok(())
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
    println!(
        "map-asset-relinker-core\n\nUSAGE:\n  map-asset-relinker-core scan [--root PATH] [--pretty]\n  map-asset-relinker-core audit [--root PATH]\n\nThis is the Rust core migration path. The Python CLI remains the full plan/apply surface until Rust core reaches parity."
    );
}
