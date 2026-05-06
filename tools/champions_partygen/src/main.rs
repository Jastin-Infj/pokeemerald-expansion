use std::collections::{BTreeMap, BTreeSet};
use std::env;
use std::fmt;
use std::fs;
use std::path::{Path, PathBuf};
use std::time::{SystemTime, UNIX_EPOCH};

type Result<T> = std::result::Result<T, String>;

const DEFAULT_CATALOG: &str = "tools/champions_partygen/catalog";
const DEFAULT_TRAINERS_PARTY: &str = "src/data/trainers.party";
const DEFAULT_AUDIT_DIR: &str = "tools/champions_partygen/local/audit";
const DEFAULT_LOGS_NORMALIZED_DIR: &str = "tools/champions_partygen/local/logs/normalized";
const DEFAULT_PROFILE_PATH: &str = "tools/champions_partygen/local/profiles/active.json";
const DEFAULT_MINIMUM_ADAPTATION_RUNS: usize = 25;
const DEFAULT_WEAKNESS_BONUS: f64 = 1.5;
const DEFAULT_EXPLORATION_RATE: f64 = 0.2;
const DEFAULT_XTR_SET_REUSE_THRESHOLD: usize = 3;
const DEFAULT_XTR_ADJACENT_OVERLAP_RATIO: f64 = 0.5;
const VALID_POOL_TAGS: &[&str] = &[
    "Lead",
    "Ace",
    "Weather Setter",
    "Weather Abuser",
    "Support",
    "Tag 5",
    "Tag 6",
    "Tag 7",
];
const VALID_RANKS: &[&str] = &["early", "mid", "late", "champion"];
const VALID_MODES: &[&str] = &["single", "double"];
const DEFAULT_SHAREABLE_ITEMS: &[&str] = &["ITEM_LUM_BERRY", "ITEM_LEFTOVERS", "ITEM_SITRUS_BERRY"];
const DEFAULT_ITEM_DUP_LIMIT: usize = 1;
const SCHEMA_VERSION: u32 = 1;

fn main() {
    if let Err(err) = run() {
        eprintln!("error: {err}");
        std::process::exit(1);
    }
}

fn run() -> Result<()> {
    let mut args: Vec<String> = env::args().skip(1).collect();
    if args.is_empty() || args[0] == "-h" || args[0] == "--help" {
        print_help();
        return Ok(());
    }

    let cmd = args.remove(0);
    // Two-token subcommands: "audit show", "audit list", "logs normalize",
    // "profile build", "profile show", "profile diff".
    let sub = match cmd.as_str() {
        "audit" | "logs" | "profile" => {
            if args.is_empty() {
                return Err(format!("{cmd} requires a subcommand"));
            }
            Some(args.remove(0))
        }
        _ => None,
    };

    let opts = Options::parse(args)?;
    let exit_code = match (cmd.as_str(), sub.as_deref()) {
        ("doctor", _) => cmd_doctor(&opts)?,
        ("scan", _) => cmd_scan(&opts)?,
        ("generate", _) => cmd_generate(&opts)?,
        ("render-one", _) => cmd_render_one(&opts)?,
        ("explain", _) => cmd_explain(&opts)?,
        ("validate", _) => cmd_validate(&opts)?,
        ("lint", _) => cmd_lint(&opts)?,
        ("diff", _) => cmd_diff(&opts)?,
        ("apply", _) => cmd_apply(&opts)?,
        ("audit", Some("show")) => cmd_audit_show(&opts)?,
        ("audit", Some("list")) => cmd_audit_list(&opts)?,
        ("logs", Some("normalize")) => cmd_logs_normalize(&opts)?,
        ("profile", Some("build")) => cmd_profile_build(&opts)?,
        ("profile", Some("show")) => cmd_profile_show(&opts)?,
        ("profile", Some("diff")) => cmd_profile_diff(&opts)?,
        (other, Some(sub)) => return Err(format!("unknown subcommand '{other} {sub}'")),
        (other, None) => return Err(format!("unknown command '{other}'")),
    };

    if exit_code != 0 {
        std::process::exit(exit_code);
    }
    Ok(())
}

fn print_help() {
    println!(
        "champions_partygen 0.2.0\n\
         \n\
         Commands:\n\
           doctor                Check repo, catalog, and trainerproc inputs\n\
           scan                  Print source trainer and script usage counts\n\
           generate              Render generated .party fragment + audit log\n\
           render-one            Render one trainer from the catalog\n\
           explain               Explain selected sets for one trainer\n\
           validate              Validate a generated .party fragment\n\
           lint                  Alias for `validate --strict`\n\
           diff                  Report trainer blocks changed by a fragment\n\
           apply                 Replace matching trainer blocks in a .party file\n\
           audit show            Print an audit log human-readably\n\
           audit list            List available audit log run ids\n\
           logs normalize        Convert raw mGBA log to JSONL\n\
           profile build         Build player_profile.json from JSONL\n\
           profile show          Print a profile summary\n\
           profile diff          Compare two profile snapshots\n\
         \n\
         Common options:\n\
           --rom-repo PATH       Repo root (default: current directory)\n\
           --catalog PATH        Catalog dir (default: tools/champions_partygen/catalog)\n\
           --seed N              Deterministic generation seed\n\
           --out PATH            Output file for generate / apply / logs / profile\n\
           --input PATH          Generated .party fragment / raw log / JSONL\n\
           --against PATH        Baseline .party for diff\n\
           --target PATH         .party file to apply into\n\
           --trainer TRAINER_ID  Trainer const for render-one/explain\n\
           --strict              Promote lint warnings to errors\n\
           --lint-only           Run lint without writing .party output\n\
           --audit-out PATH      Override audit log path\n\
           --audit-dir PATH      Override audit log directory\n\
           --profile PATH        Apply catalog-pinned player profile weights\n\
           --run RUN_ID          Audit log run id for `audit show`\n\
           --before PATH         Profile snapshot for `profile diff`\n\
           --after PATH          Profile snapshot for `profile diff`"
    );
}

#[derive(Default)]
struct Options {
    rom_repo: PathBuf,
    catalog: PathBuf,
    seed: u64,
    out: Option<PathBuf>,
    input: Option<PathBuf>,
    against: Option<PathBuf>,
    target: Option<PathBuf>,
    trainer: Option<String>,
    strict: bool,
    lint_only: bool,
    audit_out: Option<PathBuf>,
    audit_dir: Option<PathBuf>,
    profile: Option<PathBuf>,
    run_id: Option<String>,
    before: Option<PathBuf>,
    after: Option<PathBuf>,
}

impl Options {
    fn parse(args: Vec<String>) -> Result<Self> {
        let cwd = env::current_dir().map_err(|e| format!("current_dir: {e}"))?;
        let mut opts = Options {
            rom_repo: cwd,
            catalog: PathBuf::from(DEFAULT_CATALOG),
            seed: 0,
            ..Default::default()
        };

        let mut i = 0;
        while i < args.len() {
            match args[i].as_str() {
                "--rom-repo" => {
                    i += 1;
                    opts.rom_repo = PathBuf::from(value_arg(&args, i, "--rom-repo")?);
                }
                "--catalog" => {
                    i += 1;
                    opts.catalog = PathBuf::from(value_arg(&args, i, "--catalog")?);
                }
                "--seed" => {
                    i += 1;
                    opts.seed = value_arg(&args, i, "--seed")?
                        .parse()
                        .map_err(|_| "--seed must be an unsigned integer".to_string())?;
                }
                "--out" => {
                    i += 1;
                    opts.out = Some(PathBuf::from(value_arg(&args, i, "--out")?));
                }
                "--input" => {
                    i += 1;
                    opts.input = Some(PathBuf::from(value_arg(&args, i, "--input")?));
                }
                "--against" => {
                    i += 1;
                    opts.against = Some(PathBuf::from(value_arg(&args, i, "--against")?));
                }
                "--target" => {
                    i += 1;
                    opts.target = Some(PathBuf::from(value_arg(&args, i, "--target")?));
                }
                "--trainer" => {
                    i += 1;
                    opts.trainer = Some(value_arg(&args, i, "--trainer")?.to_string());
                }
                "--strict" => opts.strict = true,
                "--lint-only" => opts.lint_only = true,
                "--audit-out" => {
                    i += 1;
                    opts.audit_out = Some(PathBuf::from(value_arg(&args, i, "--audit-out")?));
                }
                "--audit-dir" => {
                    i += 1;
                    opts.audit_dir = Some(PathBuf::from(value_arg(&args, i, "--audit-dir")?));
                }
                "--profile" => {
                    i += 1;
                    opts.profile = Some(PathBuf::from(value_arg(&args, i, "--profile")?));
                }
                "--run" => {
                    i += 1;
                    opts.run_id = Some(value_arg(&args, i, "--run")?.to_string());
                }
                "--before" => {
                    i += 1;
                    opts.before = Some(PathBuf::from(value_arg(&args, i, "--before")?));
                }
                "--after" => {
                    i += 1;
                    opts.after = Some(PathBuf::from(value_arg(&args, i, "--after")?));
                }
                other => return Err(format!("unknown option '{other}'")),
            }
            i += 1;
        }

        opts.catalog = resolve_path(&opts.rom_repo, &opts.catalog);
        Ok(opts)
    }

    fn audit_dir(&self) -> PathBuf {
        match &self.audit_dir {
            Some(p) => resolve_path(&self.rom_repo, p),
            None => self.rom_repo.join(DEFAULT_AUDIT_DIR),
        }
    }
}

fn value_arg<'a>(args: &'a [String], i: usize, flag: &str) -> Result<&'a str> {
    args.get(i)
        .map(|s| s.as_str())
        .ok_or_else(|| format!("{flag} requires a value"))
}

fn resolve_path(root: &Path, path: &Path) -> PathBuf {
    if path.is_absolute() {
        path.to_path_buf()
    } else {
        root.join(path)
    }
}

fn cmd_doctor(opts: &Options) -> Result<i32> {
    let repo = &opts.rom_repo;
    let checks = [
        repo.join(DEFAULT_TRAINERS_PARTY),
        repo.join("include/constants/opponents.h"),
        repo.join("include/constants/species.h"),
        repo.join("include/constants/moves.h"),
        repo.join("include/constants/items.h"),
        repo.join("include/constants/abilities.h"),
        repo.join("tools/trainerproc/main.c"),
        opts.catalog.join("journey.json"),
    ];

    for path in checks {
        if !path.exists() {
            return Err(format!("missing required path {}", path.display()));
        }
    }

    let catalog = Catalog::load(&opts.catalog)?;
    let trainers = parse_party_blocks(&read_to_string(repo.join(DEFAULT_TRAINERS_PARTY))?);
    let constants = ConstantIndex::load(repo)?;
    let mut report = Report::default();
    validate_catalog(&catalog, &trainers, &constants, &mut report);
    if report.has_errors() {
        return Err(report.error_summary());
    }

    println!("repo: {}", repo.display());
    println!("catalog: {}", opts.catalog.display());
    println!("journey trainers: {}", catalog.journey.len());
    println!("blueprints: {}", catalog.blueprints.len());
    println!("sets: {}", catalog.sets.len());
    println!("source trainer blocks: {}", trainers.len());
    println!("doctor: ok");
    Ok(0)
}

fn cmd_scan(opts: &Options) -> Result<i32> {
    let party = read_to_string(opts.rom_repo.join(DEFAULT_TRAINERS_PARTY))?;
    let trainers = parse_party_blocks(&party);
    let script_usage = scan_script_usage(&opts.rom_repo)?;
    println!("trainer_blocks,{}", trainers.len());
    println!("script_trainer_refs,{}", script_usage.len());
    for (trainer, count) in script_usage.iter().take(30) {
        println!("{trainer},{count}");
    }
    Ok(0)
}

fn cmd_generate(opts: &Options) -> Result<i32> {
    let catalog = Catalog::load(&opts.catalog)?;
    let catalog_version = hash_catalog(&opts.catalog).unwrap_or_else(|_| "unavailable".to_string());
    let profile_state = load_profile_state(opts, &catalog, &catalog_version)?;
    if let Some(profile) = &profile_state {
        eprintln!("profile: {}", profile.status);
        for warning in &profile.warnings {
            eprintln!("profile: {warning}");
        }
    }
    let source_trainers =
        parse_party_blocks(&read_to_string(opts.rom_repo.join(DEFAULT_TRAINERS_PARTY))?);
    let constants = ConstantIndex::load(&opts.rom_repo)?;
    let mut report = Report::default();
    validate_catalog(&catalog, &source_trainers, &constants, &mut report);
    if report.has_errors() {
        return Err(report.error_summary());
    }

    let only_trainer = opts.trainer.as_deref();
    let mut output = String::new();
    output.push_str("/*\n");
    output.push_str("Generated by tools/champions_partygen.\n");
    output.push_str("Review with `partygen diff` before applying to src/data/trainers.party.\n");
    output.push_str("*/\n\n");

    let mut trainer_audits: Vec<TrainerAudit> = Vec::new();

    for req in &catalog.journey {
        if let Some(t) = only_trainer {
            if t != req.trainer_const {
                continue;
            }
        }
        let source = source_trainers
            .get(&req.trainer_const)
            .ok_or_else(|| format!("missing source trainer block {}", req.trainer_const))?;
        let blueprint = catalog
            .blueprints
            .get(&req.blueprint_id)
            .ok_or_else(|| format!("missing blueprint {}", req.blueprint_id))?;
        let mut rng = Rng::new(seed_for(opts.seed, &req.trainer_const));
        let selection = select_sets(
            blueprint,
            &catalog.sets,
            profile_state
                .as_ref()
                .filter(|p| p.enabled)
                .map(|p| &p.profile),
            &mut rng,
        )?;
        let mut audit = TrainerAudit {
            trainer_const: req.trainer_const.clone(),
            tags: req.tags.clone(),
            stage_id: req.stage_id.clone(),
            journey_index: req.journey_index,
            stage_index: req.stage_index,
            trainer_index: req.trainer_index,
            blueprint_id: blueprint.id.clone(),
            mode: blueprint.mode.clone(),
            rank: blueprint.rank.clone(),
            lints: Vec::new(),
            selections: selection
                .picks
                .iter()
                .map(|p| SelectionRecord {
                    slot: p.slot.clone(),
                    set_id: p.set_id.clone(),
                    rule: p.rule.clone(),
                    weight: p.weight,
                })
                .collect(),
        };
        run_trainer_lints(
            blueprint,
            source,
            &selection.sets,
            &catalog.lint,
            &mut audit.lints,
        );
        let mut block = String::new();
        render_trainer_block(&mut block, source, blueprint, &selection.sets);
        let frag_lints = lint_generated_fragment(&block, &constants);
        for issue in frag_lints {
            audit.lints.push(issue);
        }
        output.push_str(&block);
        output.push('\n');
        trainer_audits.push(audit);
    }

    run_cross_trainer_lints(&mut trainer_audits);

    if only_trainer.is_some() && trainer_audits.is_empty() {
        return Err(format!(
            "{} is not in journey.json",
            only_trainer.unwrap_or_default()
        ));
    }

    let audit = AuditLog {
        schema_version: SCHEMA_VERSION,
        run_id: build_run_id(opts.seed),
        seed: opts.seed,
        catalog_path: opts.catalog.display().to_string(),
        catalog_hash: catalog_version,
        profile_path: profile_state.as_ref().map(|p| p.path.clone()),
        profile_status: profile_state.as_ref().map(|p| p.status.clone()),
        profile_catalog_version: profile_state
            .as_ref()
            .and_then(|p| p.profile.catalog_version.clone()),
        profile_warnings: profile_state
            .as_ref()
            .map(|p| p.warnings.clone())
            .unwrap_or_default(),
        trainers: trainer_audits,
    };

    let audit_path = match &opts.audit_out {
        Some(p) => resolve_path(&opts.rom_repo, p),
        None => {
            let dir = opts.audit_dir();
            dir.join(format!("{}.json", audit.run_id))
        }
    };
    write_file(&audit_path, &audit_to_json(&audit))?;
    eprintln!("audit: wrote {}", audit_path.display());

    let (errors, warnings, notes) = audit.severity_counts();
    eprintln!(
        "lint: {} error(s), {} warning(s), {} note(s)",
        errors, warnings, notes
    );

    if errors > 0 || (opts.strict && warnings > 0) {
        eprintln!("{}", audit.lint_summary());
        return Ok(2);
    }

    if !opts.lint_only {
        if let Some(out_path) = &opts.out {
            let resolved = resolve_path(&opts.rom_repo, out_path);
            write_file(&resolved, &output)?;
        } else {
            print!("{output}");
        }
    } else if let Some(out_path) = &opts.out {
        eprintln!(
            "lint-only: skipping write to {}",
            resolve_path(&opts.rom_repo, out_path).display()
        );
    }

    Ok(0)
}

fn cmd_render_one(opts: &Options) -> Result<i32> {
    let trainer = opts
        .trainer
        .as_deref()
        .ok_or_else(|| "render-one requires --trainer".to_string())?;
    let mut sub_opts = Options {
        rom_repo: opts.rom_repo.clone(),
        catalog: opts.catalog.clone(),
        seed: opts.seed,
        out: None,
        trainer: Some(trainer.to_string()),
        ..Default::default()
    };
    sub_opts.lint_only = true;
    sub_opts.audit_out = opts.audit_out.clone();
    sub_opts.audit_dir = opts.audit_dir.clone();
    cmd_generate(&sub_opts)
}

fn cmd_explain(opts: &Options) -> Result<i32> {
    let trainer = opts
        .trainer
        .as_deref()
        .ok_or_else(|| "explain requires --trainer".to_string())?;
    let catalog = Catalog::load(&opts.catalog)?;
    let source_trainers =
        parse_party_blocks(&read_to_string(opts.rom_repo.join(DEFAULT_TRAINERS_PARTY))?);
    let req = catalog
        .journey
        .iter()
        .find(|t| t.trainer_const == trainer)
        .ok_or_else(|| format!("{trainer} is not in journey.json"))?;
    let blueprint = catalog
        .blueprints
        .get(&req.blueprint_id)
        .ok_or_else(|| format!("missing blueprint {}", req.blueprint_id))?;
    let catalog_version = hash_catalog(&opts.catalog).unwrap_or_else(|_| "unavailable".to_string());
    let profile_state = load_profile_state(opts, &catalog, &catalog_version)?;
    let mut rng = Rng::new(seed_for(opts.seed, &req.trainer_const));
    let selection = select_sets(
        blueprint,
        &catalog.sets,
        profile_state
            .as_ref()
            .filter(|p| p.enabled)
            .map(|p| &p.profile),
        &mut rng,
    )?;
    println!("trainer: {}", req.trainer_const);
    println!("blueprint: {}", req.blueprint_id);
    println!("mode: {}", blueprint.mode);
    println!("rank: {}", blueprint.rank);
    println!("party_size: {}", blueprint.party_size);
    println!("pool_size: {}", blueprint.pool_size);
    println!(
        "source_header: {}",
        if source_trainers.contains_key(&req.trainer_const) {
            "preserved"
        } else {
            "missing"
        }
    );
    if let Some(profile) = &profile_state {
        println!("profile: {} ({})", profile.path, profile.status);
        for warning in &profile.warnings {
            println!("profile_warning: {warning}");
        }
    }
    for (set, pick) in selection.sets.iter().zip(selection.picks.iter()) {
        println!(
            "{} -> {} [{}] roles={} archetypes={} via={} (slot={}, weight={:.2})",
            set.id,
            set.species,
            set.tags.join(" / "),
            set.roles.join(" / "),
            set.archetypes.join(" / "),
            pick.rule,
            pick.slot,
            pick.weight,
        );
    }
    Ok(0)
}

fn cmd_validate(opts: &Options) -> Result<i32> {
    let input = opts
        .input
        .as_ref()
        .ok_or_else(|| "validate requires --input".to_string())?;
    let fragment = read_to_string(resolve_path(&opts.rom_repo, input))?;
    let constants = ConstantIndex::load(&opts.rom_repo)?;
    let issues = validate_party_fragment(&fragment, &constants);
    let errors = issues
        .iter()
        .filter(|i| matches!(i.severity, Severity::Error))
        .count();
    let warnings = issues
        .iter()
        .filter(|i| matches!(i.severity, Severity::Warning))
        .count();
    let notes = issues
        .iter()
        .filter(|i| matches!(i.severity, Severity::Note))
        .count();
    if errors > 0 || (opts.strict && warnings > 0) {
        for issue in &issues {
            eprintln!("{}", issue);
        }
        return Ok(2);
    }
    for issue in &issues {
        if !matches!(issue.severity, Severity::Note) {
            eprintln!("{}", issue);
        }
    }
    println!(
        "validate: ok ({} error(s), {} warning(s), {} note(s))",
        errors, warnings, notes
    );
    Ok(0)
}

fn cmd_lint(opts: &Options) -> Result<i32> {
    let mut o = Options {
        rom_repo: opts.rom_repo.clone(),
        catalog: opts.catalog.clone(),
        seed: opts.seed,
        input: opts.input.clone(),
        ..Default::default()
    };
    o.strict = true;
    cmd_validate(&o)
}

fn cmd_diff(opts: &Options) -> Result<i32> {
    let input = opts
        .input
        .as_ref()
        .ok_or_else(|| "diff requires --input".to_string())?;
    let against = opts
        .against
        .as_ref()
        .cloned()
        .unwrap_or_else(|| PathBuf::from(DEFAULT_TRAINERS_PARTY));
    let fragment = read_to_string(resolve_path(&opts.rom_repo, input))?;
    let baseline = read_to_string(resolve_path(&opts.rom_repo, &against))?;
    let generated = parse_party_blocks(&fragment);
    let source = parse_party_blocks(&baseline);
    for (trainer, new_block) in &generated {
        match source.get(trainer) {
            Some(old_block)
                if normalize_block(&old_block.text) == normalize_block(&new_block.text) =>
            {
                println!("{trainer}: unchanged");
            }
            Some(old_block) => {
                let old_mons = count_mons(&old_block.text);
                let new_mons = count_mons(&new_block.text);
                println!("{trainer}: changed old_mons={old_mons} new_mons={new_mons}");
            }
            None => println!("{trainer}: new block"),
        }
    }
    Ok(0)
}

fn cmd_apply(opts: &Options) -> Result<i32> {
    let input = opts
        .input
        .as_ref()
        .ok_or_else(|| "apply requires --input".to_string())?;
    let target = opts
        .target
        .as_ref()
        .cloned()
        .unwrap_or_else(|| PathBuf::from(DEFAULT_TRAINERS_PARTY));
    let out = opts
        .out
        .as_ref()
        .ok_or_else(|| "apply requires --out; overwrite deliberately after review".to_string())?;
    let fragment = read_to_string(resolve_path(&opts.rom_repo, input))?;
    let constants = ConstantIndex::load(&opts.rom_repo)?;
    let issues = validate_party_fragment(&fragment, &constants);
    let errors = issues
        .iter()
        .filter(|i| matches!(i.severity, Severity::Error))
        .count();
    if errors > 0 {
        for issue in &issues {
            eprintln!("{}", issue);
        }
        return Err("refusing to apply: lint errors detected".to_string());
    }
    let target_path = resolve_path(&opts.rom_repo, &target);
    let source = read_to_string(&target_path)?;
    let result = apply_fragment(&source, &fragment)?;
    let resolved_out = resolve_path(&opts.rom_repo, out);
    write_file(&resolved_out, &result)?;
    println!("apply: wrote {}", resolved_out.display());
    Ok(0)
}

fn cmd_audit_show(opts: &Options) -> Result<i32> {
    let path = audit_path_for_show(opts)?;
    let text = read_to_string(&path)?;
    let json = JsonParser::new(&text)
        .parse()
        .map_err(|e| format!("{}: {e}", path.display()))?;
    let obj = json.obj()?;
    println!("run: {}", obj.get_string("runId")?);
    println!("seed: {}", obj.get_i64_default("seed", 0)?);
    println!("catalog: {}", obj.get_string("catalogPath")?);
    println!("catalog_hash: {}", obj.get_string("catalogHash")?);
    if let Some(Json::String(p)) = obj.get_optional("profilePath") {
        println!("profile: {p}");
    }
    if let Some(Json::String(s)) = obj.get_optional("profileStatus") {
        println!("profile_status: {s}");
    }
    if let Some(Json::String(v)) = obj.get_optional("profileCatalogVersion") {
        println!("profile_catalog_version: {v}");
    }
    let profile_warnings = obj.get_string_array_default("profileWarnings")?;
    if !profile_warnings.is_empty() {
        println!("profile_warnings:");
        for warning in profile_warnings {
            println!("  {warning}");
        }
    }
    for trainer in obj.get_array("trainers")? {
        let to = trainer.obj()?;
        println!();
        println!("== {} ==", to.get_string("trainerConst")?);
        let tags = to.get_string_array_default("tags")?;
        if !tags.is_empty() {
            println!("tags: {}", tags.join(", "));
        }
        let stage_id = to.get_string_default("stageId", "")?;
        if !stage_id.is_empty() {
            println!(
                "stage: {} journey_index={}",
                stage_id,
                to.get_i64_default("journeyIndex", -1)?
            );
        }
        println!(
            "blueprint: {} mode={} rank={}",
            to.get_string("blueprintId")?,
            to.get_string("mode")?,
            to.get_string("rank")?,
        );
        let lints = to.get_array_default("lints")?;
        if lints.is_empty() {
            println!("lints: clean");
        } else {
            for lint in lints {
                let lo = lint.obj()?;
                println!(
                    "  [{}] {}: {}",
                    lo.get_string("severity")?,
                    lo.get_string("id")?,
                    lo.get_string("msg")?
                );
            }
        }
        let selections = to.get_array_default("selections")?;
        for sel in selections {
            let so = sel.obj()?;
            println!(
                "  pick {} -> {} via {} (weight {:.2})",
                so.get_string("slot")?,
                so.get_string("setId")?,
                so.get_string("rule")?,
                so.get_f64_default("weight", 1.0)?,
            );
        }
    }
    Ok(0)
}

fn audit_path_for_show(opts: &Options) -> Result<PathBuf> {
    if let Some(input) = &opts.input {
        return Ok(resolve_path(&opts.rom_repo, input));
    }
    let run = opts
        .run_id
        .as_deref()
        .ok_or_else(|| "audit show requires --run RUN_ID or --input PATH".to_string())?;
    let dir = opts.audit_dir();
    Ok(dir.join(format!("{run}.json")))
}

fn cmd_audit_list(opts: &Options) -> Result<i32> {
    let dir = opts.audit_dir();
    if !dir.exists() {
        println!("(no audit dir at {})", dir.display());
        return Ok(0);
    }
    let mut entries: Vec<String> = Vec::new();
    for entry in fs::read_dir(&dir).map_err(|e| format!("read_dir {}: {e}", dir.display()))? {
        let entry = entry.map_err(|e| format!("read_dir {}: {e}", dir.display()))?;
        let p = entry.path();
        if p.extension().and_then(|e| e.to_str()) != Some("json") {
            continue;
        }
        if let Some(stem) = p.file_stem().and_then(|s| s.to_str()) {
            entries.push(stem.to_string());
        }
    }
    entries.sort();
    for run in entries {
        println!("{run}");
    }
    Ok(0)
}

fn cmd_logs_normalize(opts: &Options) -> Result<i32> {
    let input = opts
        .input
        .as_ref()
        .ok_or_else(|| "logs normalize requires --input".to_string())?;
    let resolved_in = resolve_path(&opts.rom_repo, input);
    let raw_files = collect_raw_log_files(&resolved_in)?;
    let mut rows: Vec<NormalizedBattle> = Vec::new();
    let mut errors_by_file: BTreeMap<String, Vec<String>> = BTreeMap::new();
    for file in &raw_files {
        let text = read_to_string(file)?;
        let (mut parsed, errs) = parse_raw_log(&text, file);
        rows.append(&mut parsed);
        if !errs.is_empty() {
            errors_by_file.insert(file.display().to_string(), errs);
        }
    }
    rows.sort_by(|a, b| {
        a.run_id
            .cmp(&b.run_id)
            .then_with(|| a.battle_id.cmp(&b.battle_id))
    });
    let out_path = match &opts.out {
        Some(p) => resolve_path(&opts.rom_repo, p),
        None => {
            let dir = opts.rom_repo.join(DEFAULT_LOGS_NORMALIZED_DIR);
            dir.join(format!("{}.jsonl", build_run_id(opts.seed)))
        }
    };
    let mut buf = String::new();
    for row in &rows {
        buf.push_str(&normalized_battle_to_jsonl(row));
        buf.push('\n');
    }
    write_file(&out_path, &buf)?;
    println!(
        "logs normalize: wrote {} battle row(s) to {}",
        rows.len(),
        out_path.display()
    );
    if !errors_by_file.is_empty() {
        for (file, errs) in &errors_by_file {
            for e in errs {
                eprintln!("{file}: {e}");
            }
        }
    }
    Ok(0)
}

fn cmd_profile_build(opts: &Options) -> Result<i32> {
    let input = opts
        .input
        .as_ref()
        .ok_or_else(|| "profile build requires --input (JSONL file or directory)".to_string())?;
    let resolved_in = resolve_path(&opts.rom_repo, input);
    let jsonl_files = collect_jsonl_files(&resolved_in)?;
    let mut battles: Vec<NormalizedBattle> = Vec::new();
    for file in &jsonl_files {
        let text = read_to_string(file)?;
        for (lineno, line) in text.lines().enumerate() {
            if line.trim().is_empty() {
                continue;
            }
            let json = JsonParser::new(line)
                .parse()
                .map_err(|e| format!("{}:{}: {e}", file.display(), lineno + 1))?;
            let row = parse_normalized_battle(&json)?;
            battles.push(row);
        }
    }
    let profile = aggregate_profile(&battles, Some(hash_catalog(&opts.catalog)?));
    let out_path = match &opts.out {
        Some(p) => resolve_path(&opts.rom_repo, p),
        None => opts.rom_repo.join(DEFAULT_PROFILE_PATH),
    };
    write_file(&out_path, &profile_to_json(&profile))?;
    println!(
        "profile build: aggregated {} battle(s) -> {}",
        battles.len(),
        out_path.display()
    );
    Ok(0)
}

fn cmd_profile_show(opts: &Options) -> Result<i32> {
    let input = opts
        .input
        .as_ref()
        .ok_or_else(|| "profile show requires --input".to_string())?;
    let resolved = resolve_path(&opts.rom_repo, input);
    let text = read_to_string(&resolved)?;
    let json = JsonParser::new(&text)
        .parse()
        .map_err(|e| format!("{}: {e}", resolved.display()))?;
    let profile = parse_profile(&json)?;
    println!("schema_version: {}", profile.schema_version);
    if let Some(catalog_version) = &profile.catalog_version {
        println!("catalog_version: {}", catalog_version);
    }
    println!("generated_at: {}", profile.generated_at);
    println!("total_battles: {}", profile.total_battles);
    println!("oldest_battle_at: {}", profile.oldest_battle_at);
    println!("newest_battle_at: {}", profile.newest_battle_at);
    println!("win_rate: {:.3}", profile.win_rate);
    println!("preferred_tempo: {}", profile.preferred_tempo);
    println!("switch_habit: {:.2}", profile.switch_habit);
    println!("item_reliance: {:.2}", profile.item_reliance);
    if profile.archetype_win_rate.is_empty() {
        println!("archetype_win_rate: (none)");
    } else {
        println!("archetype_win_rate:");
        for (k, v) in &profile.archetype_win_rate {
            println!("  {}: {:.3}", k, v);
        }
    }
    if profile.repeated_weaknesses.is_empty() {
        println!("repeated_weaknesses: (none)");
    } else {
        println!(
            "repeated_weaknesses: {}",
            profile.repeated_weaknesses.join(", ")
        );
    }
    Ok(0)
}

fn cmd_profile_diff(opts: &Options) -> Result<i32> {
    let before = opts
        .before
        .as_ref()
        .ok_or_else(|| "profile diff requires --before".to_string())?;
    let after = opts
        .after
        .as_ref()
        .ok_or_else(|| "profile diff requires --after".to_string())?;
    let b = parse_profile(
        &JsonParser::new(&read_to_string(resolve_path(&opts.rom_repo, before))?)
            .parse()
            .map_err(|e| format!("before: {e}"))?,
    )?;
    let a = parse_profile(
        &JsonParser::new(&read_to_string(resolve_path(&opts.rom_repo, after))?)
            .parse()
            .map_err(|e| format!("after: {e}"))?,
    )?;
    println!(
        "total_battles: {} -> {} (delta {})",
        b.total_battles,
        a.total_battles,
        a.total_battles as i64 - b.total_battles as i64
    );
    println!(
        "win_rate: {:.3} -> {:.3} (delta {:+.3})",
        b.win_rate,
        a.win_rate,
        a.win_rate - b.win_rate
    );
    if b.catalog_version != a.catalog_version {
        println!(
            "catalog_version: {} -> {}",
            b.catalog_version.as_deref().unwrap_or("(missing)"),
            a.catalog_version.as_deref().unwrap_or("(missing)")
        );
    }
    let keys: BTreeSet<&String> = b
        .archetype_win_rate
        .keys()
        .chain(a.archetype_win_rate.keys())
        .collect();
    for k in keys {
        let bv = b.archetype_win_rate.get(k).copied().unwrap_or(0.0);
        let av = a.archetype_win_rate.get(k).copied().unwrap_or(0.0);
        println!(
            "archetype {}: {:.3} -> {:.3} (delta {:+.3})",
            k,
            bv,
            av,
            av - bv
        );
    }
    Ok(0)
}

struct Selection<'a> {
    sets: Vec<&'a Set>,
    picks: Vec<PickRecord>,
}

#[derive(Clone)]
struct PickRecord {
    slot: String,
    set_id: String,
    rule: String,
    weight: f64,
}

fn render_trainer_block(
    out: &mut String,
    source: &PartyBlock,
    blueprint: &Blueprint,
    sets: &[&Set],
) {
    out.push_str(&format!("=== {} ===\n", source.name));
    for line in &source.header_lines {
        let key = line.split(':').next().unwrap_or("").trim();
        if matches!(
            key,
            "Party Size" | "Pool Rules" | "Pool Pick Functions" | "Pool Prune" | "Copy Pool"
        ) {
            continue;
        }
        out.push_str(line);
        out.push('\n');
    }
    out.push_str(&format!("Party Size: {}\n", blueprint.party_size));
    out.push_str(&format!(
        "Pool Rules: {}\n\n",
        pool_ruleset_to_party_name(&blueprint.ruleset_id)
    ));

    for set in sets {
        render_set(out, set);
        out.push('\n');
    }
}

fn render_set(out: &mut String, set: &Set) {
    out.push_str(&set.species);
    if let Some(item) = &set.item {
        if item != "ITEM_NONE" {
            out.push_str(" @ ");
            out.push_str(item);
        }
    }
    out.push('\n');
    if let Some(ability) = &set.ability {
        out.push_str("Ability: ");
        out.push_str(ability);
        out.push('\n');
    }
    out.push_str(&format!("Level: {}\n", set.level));
    out.push_str(&format!("IVs: {}\n", stat_line(&set.ivs)));
    out.push_str(&format!("EVs: {}\n", stat_line(&set.evs)));
    if let Some(nature) = &set.nature {
        out.push_str("Nature: ");
        out.push_str(&constant_to_title(nature, "NATURE_"));
        out.push('\n');
    }
    if !set.tags.is_empty() {
        out.push_str("Tags: ");
        out.push_str(&set.tags.join(" / "));
        out.push('\n');
    }
    for mv in &set.moves {
        out.push_str("- ");
        out.push_str(mv);
        out.push('\n');
    }
}

fn pool_ruleset_to_party_name(ruleset: &str) -> String {
    let raw = ruleset.strip_prefix("POOL_RULESET_").unwrap_or(ruleset);
    raw.split('_')
        .map(|part| {
            let mut chars = part.chars();
            match chars.next() {
                Some(first) => {
                    first.to_ascii_uppercase().to_string() + &chars.as_str().to_ascii_lowercase()
                }
                None => String::new(),
            }
        })
        .collect::<Vec<_>>()
        .join(" ")
}

fn stat_line(value: &str) -> String {
    let labels = ["HP", "Atk", "Def", "SpA", "SpD", "Spe"];
    let nums: Vec<&str> = value.split('/').collect();
    labels
        .iter()
        .enumerate()
        .map(|(i, label)| format!("{} {}", nums.get(i).copied().unwrap_or("0"), label))
        .collect::<Vec<_>>()
        .join(" / ")
}

fn constant_to_title(value: &str, prefix: &str) -> String {
    let raw = value.strip_prefix(prefix).unwrap_or(value);
    raw.split('_')
        .map(|part| {
            let lower = part.to_ascii_lowercase();
            let mut chars = lower.chars();
            match chars.next() {
                Some(first) => first.to_ascii_uppercase().to_string() + chars.as_str(),
                None => String::new(),
            }
        })
        .collect::<Vec<_>>()
        .join(" ")
}

fn select_sets<'a>(
    blueprint: &Blueprint,
    sets: &'a [Set],
    profile: Option<&Profile>,
    rng: &mut Rng,
) -> Result<Selection<'a>> {
    if blueprint.pool_size < blueprint.party_size {
        return Err(format!(
            "{} poolSize {} is smaller than partySize {}",
            blueprint.id, blueprint.pool_size, blueprint.party_size
        ));
    }
    let mut selected: Vec<&Set> = Vec::new();
    let mut picks: Vec<PickRecord> = Vec::new();
    let mut used = BTreeSet::new();

    for req in &blueprint.required {
        let candidates: Vec<&Set> = sets
            .iter()
            .filter(|set| !used.contains(&set.id))
            .filter(|set| set_allowed_for_blueprint(blueprint, set))
            .filter(|set| req.roles.iter().any(|role| set.roles.contains(role)))
            .filter(|set| match req.slot.as_str() {
                "lead" => set.tags.iter().any(|tag| tag == "Lead"),
                "ace" => set.tags.iter().any(|tag| tag == "Ace"),
                _ => true,
            })
            .collect();
        let picked = pick_weighted(candidates, profile, rng).ok_or_else(|| {
            format!(
                "{} has no candidate for required slot '{}' roles {:?}",
                blueprint.id, req.slot, req.roles
            )
        })?;
        used.insert(picked.set.id.clone());
        selected.push(picked.set);
        picks.push(PickRecord {
            slot: req.slot.clone(),
            set_id: picked.set.id.clone(),
            rule: format!("required:{:?}", req.roles),
            weight: picked.weight,
        });
    }

    for pref in &blueprint.preferred {
        let current = selected
            .iter()
            .filter(|set| set.roles.contains(&pref.role))
            .count();
        let needed = pref.min.saturating_sub(current);
        for _ in 0..needed {
            if selected.len() >= blueprint.pool_size {
                break;
            }
            let candidates: Vec<&Set> = sets
                .iter()
                .filter(|set| !used.contains(&set.id))
                .filter(|set| set_allowed_for_blueprint(blueprint, set))
                .filter(|set| set.roles.contains(&pref.role))
                .collect();
            let Some(picked) = pick_weighted(candidates, profile, rng) else {
                return Err(format!(
                    "{} has no candidate for preferred role {}",
                    blueprint.id, pref.role
                ));
            };
            used.insert(picked.set.id.clone());
            selected.push(picked.set);
            picks.push(PickRecord {
                slot: "fill".to_string(),
                set_id: picked.set.id.clone(),
                rule: format!("preferred:{}", pref.role),
                weight: picked.weight,
            });
        }
    }

    while selected.len() < blueprint.pool_size {
        let candidates: Vec<&Set> = sets
            .iter()
            .filter(|set| !used.contains(&set.id))
            .filter(|set| set_allowed_for_blueprint(blueprint, set))
            .collect();
        let Some(picked) = pick_weighted(candidates, profile, rng) else {
            return Err(format!(
                "{} needs {} pool members but only {} unique sets are available",
                blueprint.id,
                blueprint.pool_size,
                selected.len()
            ));
        };
        used.insert(picked.set.id.clone());
        selected.push(picked.set);
        picks.push(PickRecord {
            slot: "fill".to_string(),
            set_id: picked.set.id.clone(),
            rule: "fill".to_string(),
            weight: picked.weight,
        });
    }

    Ok(Selection {
        sets: selected,
        picks,
    })
}

fn rank_compatible(blueprint: &Blueprint, set: &Set) -> bool {
    let bp = rank_to_index(&blueprint.rank);
    let lo = rank_to_index(&set.min_rank);
    let hi = rank_to_index(&set.max_rank);
    bp >= lo && bp <= hi
}

fn set_allowed_for_blueprint(blueprint: &Blueprint, set: &Set) -> bool {
    if !rank_compatible(blueprint, set) {
        return false;
    }
    if blueprint.set_groups.is_empty() {
        return true;
    }
    blueprint
        .set_groups
        .iter()
        .any(|group| set.groups.contains(group))
}

fn rank_to_index(rank: &str) -> i32 {
    let normalized = rank.strip_prefix("rank.").unwrap_or(rank);
    match normalized {
        "early" => 0,
        "mid" => 1,
        "late" => 2,
        "champion" => 3,
        _ => 1,
    }
}

struct WeightedPick<'a> {
    set: &'a Set,
    weight: f64,
}

fn pick_weighted<'a>(
    mut candidates: Vec<&'a Set>,
    profile: Option<&Profile>,
    rng: &mut Rng,
) -> Option<WeightedPick<'a>> {
    candidates.sort_by(|a, b| a.id.cmp(&b.id));
    if candidates.is_empty() {
        return None;
    }

    let Some(profile) = profile else {
        let index = rng.next_usize(candidates.len());
        return Some(WeightedPick {
            set: candidates[index],
            weight: 1.0,
        });
    };

    let weights: Vec<f64> = candidates
        .iter()
        .map(|set| profile_weight(set, profile))
        .collect();
    let first_weight = weights[0];
    if weights
        .iter()
        .all(|weight| (*weight - first_weight).abs() < 1e-9)
    {
        let index = rng.next_usize(candidates.len());
        return Some(WeightedPick {
            set: candidates[index],
            weight: first_weight,
        });
    }

    if rng.next_f64() < DEFAULT_EXPLORATION_RATE {
        let index = rng.next_usize(candidates.len());
        return Some(WeightedPick {
            set: candidates[index],
            weight: 1.0,
        });
    }

    let total: f64 = weights.iter().sum();
    if total <= 0.0 || !total.is_finite() {
        let index = rng.next_usize(candidates.len());
        return Some(WeightedPick {
            set: candidates[index],
            weight: 1.0,
        });
    }

    let mut target = rng.next_f64() * total;
    for (set, weight) in candidates.iter().zip(weights.iter()) {
        if target < *weight {
            return Some(WeightedPick {
                set,
                weight: *weight,
            });
        }
        target -= *weight;
    }
    let last = candidates[candidates.len() - 1];
    Some(WeightedPick {
        set: last,
        weight: profile_weight(last, profile),
    })
}

fn profile_weight(set: &Set, profile: &Profile) -> f64 {
    if set
        .archetypes
        .iter()
        .any(|a| profile.repeated_weaknesses.contains(a))
    {
        DEFAULT_WEAKNESS_BONUS
    } else {
        1.0
    }
}

fn validate_catalog(
    catalog: &Catalog,
    source_trainers: &BTreeMap<String, PartyBlock>,
    constants: &ConstantIndex,
    report: &mut Report,
) {
    for req in &catalog.journey {
        if !source_trainers.contains_key(&req.trainer_const) {
            report.error(format!("missing trainer block {}", req.trainer_const));
        }
        if !constants.trainers.contains(&req.trainer_const) {
            report.error(format!("missing trainer constant {}", req.trainer_const));
        }
        if !catalog.blueprints.contains_key(&req.blueprint_id) {
            report.error(format!("missing blueprint {}", req.blueprint_id));
        }
    }
    for blueprint in catalog.blueprints.values() {
        if blueprint.party_size == 0 || blueprint.party_size > 6 {
            report.error(format!("{} partySize must be 1..6", blueprint.id));
        }
        if blueprint.pool_size < blueprint.party_size {
            report.error(format!("{} poolSize must be >= partySize", blueprint.id));
        }
        if !VALID_MODES.contains(&blueprint.mode.as_str()) {
            report.error(format!(
                "{} mode '{}' must be one of {:?}",
                blueprint.id, blueprint.mode, VALID_MODES
            ));
        }
        let normalized_rank = blueprint
            .rank
            .strip_prefix("rank.")
            .unwrap_or(&blueprint.rank);
        if !VALID_RANKS.contains(&normalized_rank) {
            report.error(format!(
                "{} rank '{}' must be one of {:?}",
                blueprint.id, blueprint.rank, VALID_RANKS
            ));
        }
        if !blueprint.set_groups.is_empty() {
            let candidates = catalog
                .sets
                .iter()
                .filter(|set| set_allowed_for_blueprint(blueprint, set))
                .count();
            if candidates < blueprint.pool_size {
                report.error(format!(
                    "{} setGroups {:?} expose only {} compatible set(s), below poolSize {}",
                    blueprint.id, blueprint.set_groups, candidates, blueprint.pool_size
                ));
            }
        }
        for pref in &blueprint.preferred {
            if pref.max < pref.min {
                report.error(format!(
                    "{} preferred role {} has max below min",
                    blueprint.id, pref.role
                ));
            }
        }
        if blueprint.constraints.min_local_pool_size > blueprint.pool_size {
            report.error(format!(
                "{} minLocalPoolSize exceeds poolSize",
                blueprint.id
            ));
        }
        if blueprint.constraints.max_local_pool_size != 0
            && blueprint.pool_size > blueprint.constraints.max_local_pool_size
        {
            report.error(format!(
                "{} poolSize exceeds maxLocalPoolSize",
                blueprint.id
            ));
        }
    }
    for set in &catalog.sets {
        constants.check("species", &set.species, report);
        if let Some(ability) = &set.ability {
            constants.check("ability", ability, report);
        }
        if let Some(item) = &set.item {
            constants.check("item", item, report);
        }
        if let Some(nature) = &set.nature {
            constants.check("nature", nature, report);
        }
        for mv in &set.moves {
            constants.check("move", mv, report);
        }
        if set.moves.len() > 4 {
            report.error(format!("{} has more than four moves", set.id));
        }
        for tag in &set.tags {
            if !VALID_POOL_TAGS.contains(&tag.as_str()) {
                report.error(format!("{} has invalid pool tag '{}'", set.id, tag));
            }
        }
        if parse_stats(&set.ivs).is_none() {
            report.error(format!("{} has invalid ivs '{}'", set.id, set.ivs));
        }
        if parse_stats(&set.evs).is_none() {
            report.error(format!("{} has invalid evs '{}'", set.id, set.evs));
        }
        let min_idx = rank_to_index(&set.min_rank);
        let max_idx = rank_to_index(&set.max_rank);
        if max_idx < min_idx {
            report.error(format!(
                "{} maxRank '{}' is below minRank '{}'",
                set.id, set.max_rank, set.min_rank
            ));
        }
    }
}

fn validate_party_fragment(fragment: &str, constants: &ConstantIndex) -> Vec<LintIssue> {
    let mut out: Vec<LintIssue> = Vec::new();
    let blocks = parse_party_blocks(fragment);
    if blocks.is_empty() {
        out.push(LintIssue::error_(
            "FRG001",
            None,
            "fragment contains no trainer blocks",
        ));
    }
    for (trainer, block) in blocks {
        if !constants.trainers.contains(&trainer) {
            out.push(LintIssue::error_(
                "FRG002",
                Some(&trainer),
                &format!("unknown trainer constant {trainer}"),
            ));
        }
        let mut party_size = None;
        let mut mon_count = 0usize;
        let mut current_moves = 0usize;
        for line in block.text.lines() {
            let trimmed = line.trim();
            if let Some(value) = trimmed.strip_prefix("Party Size:") {
                match value.trim().parse::<usize>() {
                    Ok(size) => party_size = Some(size),
                    Err(_) => out.push(LintIssue::error_(
                        "FRG003",
                        Some(&trainer),
                        "invalid Party Size",
                    )),
                }
            } else if let Some(tags) = trimmed.strip_prefix("Tags:") {
                for tag in tags.split('/') {
                    let tag = tag.trim();
                    if !tag.is_empty() && !VALID_POOL_TAGS.contains(&tag) {
                        out.push(LintIssue::error_(
                            "FRG004",
                            Some(&trainer),
                            &format!("invalid tag '{tag}'"),
                        ));
                    }
                }
            } else if trimmed.starts_with("- ") {
                current_moves += 1;
                let mv = trimmed.trim_start_matches("- ").trim();
                if !constants.moves.contains(mv) {
                    out.push(LintIssue::error_(
                        "FRG005",
                        Some(&trainer),
                        &format!("unknown move {mv}"),
                    ));
                }
                if current_moves > 4 {
                    out.push(LintIssue::error_(
                        "FRG006",
                        Some(&trainer),
                        "mon has more than four moves",
                    ));
                }
            } else if is_mon_start(trimmed) {
                mon_count += 1;
                current_moves = 0;
                if let Some(species) = trimmed.split('@').next().map(str::trim) {
                    if species.starts_with("SPECIES_") && !constants.species.contains(species) {
                        out.push(LintIssue::error_(
                            "FRG007",
                            Some(&trainer),
                            &format!("unknown species {species}"),
                        ));
                    }
                }
                if let Some(item) = trimmed.split('@').nth(1).map(str::trim) {
                    if !constants.items.contains(item) {
                        out.push(LintIssue::error_(
                            "FRG008",
                            Some(&trainer),
                            &format!("unknown item {item}"),
                        ));
                    }
                }
            } else if let Some(value) = trimmed.strip_prefix("Ability:") {
                let ability = value.trim();
                if ability.starts_with("ABILITY_") && !constants.abilities.contains(ability) {
                    out.push(LintIssue::error_(
                        "FRG009",
                        Some(&trainer),
                        &format!("unknown ability {ability}"),
                    ));
                }
            }
        }
        if let Some(size) = party_size {
            if size == 0 || size > 6 {
                out.push(LintIssue::error_(
                    "FRG010",
                    Some(&trainer),
                    "Party Size must be 1..6",
                ));
            }
            if mon_count < size {
                out.push(LintIssue::error_(
                    "FRG011",
                    Some(&trainer),
                    &format!("Party Size {size} exceeds mon count {mon_count}"),
                ));
            }
            if mon_count > 255 {
                out.push(LintIssue::error_(
                    "FRG012",
                    Some(&trainer),
                    "pool exceeds 255 mons",
                ));
            }
        }
    }
    out
}

fn parse_stats(value: &str) -> Option<[u16; 6]> {
    let parts: Vec<&str> = value.split('/').collect();
    if parts.len() != 6 {
        return None;
    }
    let mut result = [0; 6];
    for (i, part) in parts.iter().enumerate() {
        result[i] = part.parse().ok()?;
    }
    Some(result)
}

#[derive(Default)]
struct Report {
    errors: Vec<String>,
}

impl Report {
    fn error(&mut self, msg: String) {
        self.errors.push(msg);
    }

    fn has_errors(&self) -> bool {
        !self.errors.is_empty()
    }

    fn error_summary(&self) -> String {
        self.errors.join("\n")
    }
}

impl fmt::Display for Report {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        for err in &self.errors {
            writeln!(f, "{err}")?;
        }
        Ok(())
    }
}

#[derive(Clone, Debug)]
enum Severity {
    Error,
    Warning,
    Note,
}

impl Severity {
    fn as_str(&self) -> &'static str {
        match self {
            Severity::Error => "error",
            Severity::Warning => "warning",
            Severity::Note => "note",
        }
    }
}

#[derive(Clone, Debug)]
struct LintIssue {
    id: String,
    severity: Severity,
    trainer: Option<String>,
    msg: String,
}

impl LintIssue {
    fn new(id: &str, severity: Severity, trainer: Option<&str>, msg: &str) -> Self {
        Self {
            id: id.to_string(),
            severity,
            trainer: trainer.map(str::to_string),
            msg: msg.to_string(),
        }
    }
    fn error_(id: &str, trainer: Option<&str>, msg: &str) -> Self {
        Self::new(id, Severity::Error, trainer, msg)
    }
    fn warning(id: &str, trainer: Option<&str>, msg: &str) -> Self {
        Self::new(id, Severity::Warning, trainer, msg)
    }
    fn note(id: &str, trainer: Option<&str>, msg: &str) -> Self {
        Self::new(id, Severity::Note, trainer, msg)
    }
}

impl fmt::Display for LintIssue {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match &self.trainer {
            Some(t) => write!(
                f,
                "[{}] {} {}: {}",
                self.severity.as_str(),
                self.id,
                t,
                self.msg
            ),
            None => write!(f, "[{}] {}: {}", self.severity.as_str(), self.id, self.msg),
        }
    }
}

fn run_trainer_lints(
    blueprint: &Blueprint,
    source: &PartyBlock,
    sets: &[&Set],
    lint_cfg: &LintConfig,
    out: &mut Vec<LintIssue>,
) {
    let header_double = source_double_battle(source);
    let bp_mode_double = blueprint.mode == "double";
    if let Some(double) = header_double {
        if double != bp_mode_double {
            out.push(LintIssue::error_(
                "DBL001",
                Some(&source.name),
                &format!(
                    "trainer header Double Battle={} but blueprint mode='{}'",
                    if double { "Yes" } else { "No" },
                    blueprint.mode
                ),
            ));
        }
    }

    if bp_mode_double && blueprint.party_size < 2 {
        out.push(LintIssue::error_(
            "DBL002",
            Some(&source.name),
            "doubles blueprint requires partySize >= 2",
        ));
    }

    if bp_mode_double && blueprint.require_spread_move {
        let any_spread = sets.iter().any(|s| {
            s.doubles_spread_move || s.moves.iter().any(|m| lint_cfg.spread_moves.contains(m))
        });
        if !any_spread {
            out.push(LintIssue::error_(
                "DBL003",
                Some(&source.name),
                "doubles pool has zero spread moves",
            ));
        }
    }

    if bp_mode_double {
        let lead_count = sets
            .iter()
            .filter(|s| s.tags.iter().any(|t| t == "Lead"))
            .count();
        if lead_count < 2 {
            out.push(LintIssue::warning(
                "DBL004",
                Some(&source.name),
                &format!("doubles pool has {lead_count} Lead-tagged set(s); doubles usually wants at least 2"),
            ));
        }
        let ace_count = sets
            .iter()
            .filter(|s| s.tags.iter().any(|t| t == "Ace"))
            .count();
        if ace_count > 1 {
            out.push(LintIssue::warning(
                "DBL005",
                Some(&source.name),
                &format!("doubles pool has {ace_count} Ace-tagged sets; doubles win-conditions usually share reward"),
            ));
        }
    }

    let lead_count = sets
        .iter()
        .filter(|s| s.tags.iter().any(|t| t == "Lead"))
        .count();
    let ace_count = sets
        .iter()
        .filter(|s| s.tags.iter().any(|t| t == "Ace"))
        .count();
    let needs_lead = blueprint.required.iter().any(|r| r.slot == "lead");
    let needs_ace = blueprint.required.iter().any(|r| r.slot == "ace");
    if needs_lead && lead_count == 0 {
        out.push(LintIssue::error_(
            "SLT001",
            Some(&source.name),
            "blueprint requires Lead but pool has zero Lead-tagged sets",
        ));
    }
    if needs_ace && ace_count == 0 {
        out.push(LintIssue::error_(
            "SLT002",
            Some(&source.name),
            "blueprint requires Ace but pool has zero Ace-tagged sets",
        ));
    }

    let mut species_seen: BTreeMap<&str, usize> = BTreeMap::new();
    for s in sets {
        *species_seen.entry(s.species.as_str()).or_default() += 1;
    }
    if !blueprint.allow_species_duplicate {
        for (species, count) in species_seen {
            if count > 1 {
                out.push(LintIssue::warning(
                    "SLT004",
                    Some(&source.name),
                    &format!("species {species} appears {count} times; set allowSpeciesDuplicate=true to silence"),
                ));
            }
        }
    }

    let mut item_seen: BTreeMap<&str, usize> = BTreeMap::new();
    for s in sets {
        if let Some(item) = &s.item {
            if item == "ITEM_NONE" {
                continue;
            }
            *item_seen.entry(item.as_str()).or_default() += 1;
        }
    }
    let dup_limit = lint_cfg.item_duplication_limit.max(1);
    for (item, count) in item_seen {
        let limit = if lint_cfg.shareable_items.contains(item) {
            dup_limit + 1
        } else {
            dup_limit
        };
        if count > limit {
            out.push(LintIssue::warning(
                "ITM001",
                Some(&source.name),
                &format!("item {item} appears {count} times (limit {limit})"),
            ));
        }
    }
    for s in sets {
        if let Some(item) = &s.item {
            if lint_cfg.blocked_items.contains(item.as_str()) {
                out.push(LintIssue::note(
                    "ITM003",
                    Some(&source.name),
                    &format!("item {item} is on the blocklist"),
                ));
            }
        }
    }

    let has_setter = sets.iter().any(|s| set_has_tag(s, "Weather Setter"));
    let has_abuser = sets.iter().any(|s| set_has_tag(s, "Weather Abuser"));
    if has_setter && !has_abuser {
        out.push(LintIssue::error_(
            "WTH001",
            Some(&source.name),
            "pool has Weather Setter but zero Weather Abuser",
        ));
    }
    if has_abuser && !has_setter {
        out.push(LintIssue::error_(
            "WTH002",
            Some(&source.name),
            "pool has Weather Abuser but zero Weather Setter",
        ));
    }
    run_battlefield_pair_lints(source, sets, lint_cfg, out);

    let offensive_types = sets
        .iter()
        .flat_map(|s| s.archetypes.iter())
        .filter(|a| a.starts_with("type."))
        .collect::<BTreeSet<_>>();
    if offensive_types.len() == 1 {
        out.push(LintIssue::warning(
            "CVR001",
            Some(&source.name),
            &format!(
                "pool offensive coverage uses only one declared type tag ({})",
                offensive_types.iter().next().unwrap()
            ),
        ));
    }

    if blueprint.party_size == 0 {
        out.push(LintIssue::error_(
            "RNK000",
            Some(&source.name),
            "blueprint partySize is zero",
        ));
    }
}

fn run_battlefield_pair_lints(
    source: &PartyBlock,
    sets: &[&Set],
    lint_cfg: &LintConfig,
    out: &mut Vec<LintIssue>,
) {
    for pair in &lint_cfg.battlefield_pairs {
        let has_setter = sets
            .iter()
            .any(|s| set_matches_battlefield_side(s, pair, true));
        let has_abuser = sets
            .iter()
            .any(|s| set_matches_battlefield_side(s, pair, false));
        let (setter_id, abuser_id) = if pair.kind == "weather" {
            ("WTH001", "WTH002")
        } else {
            ("BFL001", "BFL002")
        };
        if has_setter && !has_abuser {
            out.push(LintIssue::error_(
                setter_id,
                Some(&source.name),
                &format!(
                    "pool has {} setter but zero matching abuser ({})",
                    pair.label, pair.id
                ),
            ));
        }
        if has_abuser && !has_setter {
            out.push(LintIssue::error_(
                abuser_id,
                Some(&source.name),
                &format!(
                    "pool has {} abuser but zero matching setter ({})",
                    pair.label, pair.id
                ),
            ));
        }
    }
}

fn set_matches_battlefield_side(set: &Set, pair: &BattlefieldPair, setter: bool) -> bool {
    let tags = if setter {
        &pair.setter_tags
    } else {
        &pair.abuser_tags
    };
    if tags.iter().any(|tag| set_has_tag(set, tag)) {
        return true;
    }

    let moves = if setter {
        &pair.setter_moves
    } else {
        &pair.abuser_moves
    };
    if pair.detect_moves && moves.iter().any(|m| set.moves.contains(m)) {
        return true;
    }

    let abilities = if setter {
        &pair.setter_abilities
    } else {
        &pair.abuser_abilities
    };
    pair.detect_abilities
        && set
            .ability
            .as_ref()
            .is_some_and(|ability| abilities.contains(ability))
}

fn set_has_tag(set: &Set, tag: &str) -> bool {
    set.tags.iter().any(|t| t == tag) || set.lint_tags.iter().any(|t| t == tag)
}

fn lint_generated_fragment(block: &str, constants: &ConstantIndex) -> Vec<LintIssue> {
    let mut issues = validate_party_fragment(block, constants);
    issues.retain(|i| {
        // strip FRG001 (no blocks) since we always generate exactly one block here
        i.id != "FRG001"
    });
    issues
}

fn run_cross_trainer_lints(audits: &mut [TrainerAudit]) {
    let mut set_usage: BTreeMap<String, Vec<usize>> = BTreeMap::new();
    for (idx, audit) in audits.iter().enumerate() {
        for set_id in selection_set_ids(audit) {
            set_usage.entry(set_id).or_default().push(idx);
        }
    }
    for (set_id, users) in set_usage {
        let count = users.len();
        if count >= DEFAULT_XTR_SET_REUSE_THRESHOLD {
            for idx in users {
                let trainer = audits[idx].trainer_const.clone();
                audits[idx].lints.push(LintIssue::warning(
                    "XTR001",
                    Some(&trainer),
                    &format!(
                        "set {set_id} appears in {count} generated pools (threshold {})",
                        DEFAULT_XTR_SET_REUSE_THRESHOLD
                    ),
                ));
            }
        }
    }

    for i in 1..audits.len() {
        let prev_stage = audits[i - 1].stage_id.clone();
        let curr_stage = audits[i].stage_id.clone();
        if prev_stage != curr_stage {
            continue;
        }
        if audits[i - 1].journey_index + 1 != audits[i].journey_index {
            continue;
        }
        let prev_sets = selection_set_ids(&audits[i - 1]);
        let curr_sets = selection_set_ids(&audits[i]);
        let smaller = prev_sets.len().min(curr_sets.len());
        if smaller == 0 {
            continue;
        }
        let shared = prev_sets.intersection(&curr_sets).count();
        let ratio = shared as f64 / smaller as f64;
        if ratio > DEFAULT_XTR_ADJACENT_OVERLAP_RATIO {
            let prev_trainer = audits[i - 1].trainer_const.clone();
            let curr_trainer = audits[i].trainer_const.clone();
            let msg = format!(
                "adjacent stage trainers {prev_trainer} and {curr_trainer} share {shared}/{smaller} pool sets"
            );
            audits[i - 1]
                .lints
                .push(LintIssue::warning("XTR002", Some(&prev_trainer), &msg));
            audits[i]
                .lints
                .push(LintIssue::warning("XTR002", Some(&curr_trainer), &msg));
        }
    }
}

fn selection_set_ids(audit: &TrainerAudit) -> BTreeSet<String> {
    audit.selections.iter().map(|s| s.set_id.clone()).collect()
}

fn source_double_battle(source: &PartyBlock) -> Option<bool> {
    for line in &source.header_lines {
        let trimmed = line.trim();
        if let Some(rest) = trimmed.strip_prefix("Double Battle:") {
            let v = rest.trim().to_ascii_lowercase();
            if v == "yes" {
                return Some(true);
            }
            if v == "no" {
                return Some(false);
            }
        }
    }
    None
}

#[derive(Clone)]
struct Set {
    id: String,
    species: String,
    ability: Option<String>,
    moves: Vec<String>,
    item: Option<String>,
    ivs: String,
    evs: String,
    nature: Option<String>,
    level: u16,
    roles: Vec<String>,
    archetypes: Vec<String>,
    groups: Vec<String>,
    lint_tags: Vec<String>,
    tags: Vec<String>,
    min_rank: String,
    max_rank: String,
    doubles_spread_move: bool,
}

struct Blueprint {
    id: String,
    party_size: usize,
    pool_size: usize,
    ruleset_id: String,
    required: Vec<RequiredSlot>,
    preferred: Vec<PreferredRole>,
    constraints: Constraints,
    mode: String,
    rank: String,
    set_groups: Vec<String>,
    require_spread_move: bool,
    allow_species_duplicate: bool,
}

struct RequiredSlot {
    slot: String,
    roles: Vec<String>,
}

struct PreferredRole {
    role: String,
    min: usize,
    max: usize,
}

#[derive(Default)]
struct Constraints {
    min_local_pool_size: usize,
    max_local_pool_size: usize,
}

struct JourneyTrainer {
    trainer_const: String,
    blueprint_id: String,
    tags: Vec<String>,
    stage_id: String,
    journey_index: usize,
    stage_index: usize,
    trainer_index: usize,
}

struct Catalog {
    journey: Vec<JourneyTrainer>,
    blueprints: BTreeMap<String, Blueprint>,
    sets: Vec<Set>,
    lint: LintConfig,
}

#[derive(Default)]
struct LintConfig {
    spread_moves: BTreeSet<String>,
    blocked_items: BTreeSet<String>,
    shareable_items: BTreeSet<String>,
    item_duplication_limit: usize,
    battlefield_pairs: Vec<BattlefieldPair>,
}

struct BattlefieldPair {
    id: String,
    label: String,
    kind: String,
    setter_tags: Vec<String>,
    abuser_tags: Vec<String>,
    setter_moves: Vec<String>,
    setter_abilities: Vec<String>,
    abuser_moves: Vec<String>,
    abuser_abilities: Vec<String>,
    detect_moves: bool,
    detect_abilities: bool,
}

impl Catalog {
    fn load(path: &Path) -> Result<Self> {
        let journey = parse_journey(&read_json(path.join("journey.json"))?)?;
        let mut blueprints = BTreeMap::new();
        for file in list_json_files(&path.join("blueprints"))? {
            let blueprint = parse_blueprint(&read_json(file)?)?;
            blueprints.insert(blueprint.id.clone(), blueprint);
        }
        let mut sets = Vec::new();
        for file in list_json_files(&path.join("sets"))? {
            sets.extend(parse_sets(&read_json(file)?)?);
        }
        let lint = load_lint_config(path)?;
        Ok(Self {
            journey,
            blueprints,
            sets,
            lint,
        })
    }
}

fn load_lint_config(catalog: &Path) -> Result<LintConfig> {
    let mut cfg = LintConfig {
        item_duplication_limit: DEFAULT_ITEM_DUP_LIMIT,
        shareable_items: DEFAULT_SHAREABLE_ITEMS
            .iter()
            .map(|s| s.to_string())
            .collect(),
        ..Default::default()
    };
    let lint_dir = catalog.join("lint");
    if !lint_dir.exists() {
        return Ok(cfg);
    }
    let spread = lint_dir.join("spread_moves.json");
    if spread.exists() {
        let json = read_versioned_json(&spread)?;
        for v in json.obj()?.get_array_default("spreadMoves")? {
            if let Json::String(s) = v {
                cfg.spread_moves.insert(s);
            }
        }
    }
    let weather = lint_dir.join("weather_pairs.json");
    if weather.exists() {
        let _ = read_versioned_json(&weather)?;
    }
    let battlefield = lint_dir.join("battlefield_pairs.json");
    if battlefield.exists() {
        let json = read_versioned_json(&battlefield)?;
        for v in json.obj()?.get_array_default("pairs")? {
            let obj = v.obj()?;
            let id = obj.get_string("id")?.to_string();
            cfg.battlefield_pairs.push(BattlefieldPair {
                id: id.clone(),
                label: obj.get_optional_string("label")?.unwrap_or(id),
                kind: obj
                    .get_optional_string("kind")?
                    .unwrap_or_else(|| "battlefield".to_string()),
                setter_tags: obj.get_string_array_default("setterTags")?,
                abuser_tags: obj.get_string_array_default("abuserTags")?,
                setter_moves: obj.get_string_array_default("setterMoves")?,
                setter_abilities: obj.get_string_array_default("setterAbilities")?,
                abuser_moves: obj.get_string_array_default("abuserMoves")?,
                abuser_abilities: obj.get_string_array_default("abuserAbilities")?,
                detect_moves: obj.get_bool_default("detectMoves", false)?,
                detect_abilities: obj.get_bool_default("detectAbilities", false)?,
            });
        }
    }
    let block = lint_dir.join("items_blocklist.json");
    if block.exists() {
        let json = read_versioned_json(&block)?;
        for v in json.obj()?.get_array_default("blocked")? {
            if let Json::String(s) = v {
                cfg.blocked_items.insert(s);
            }
        }
        for v in json.obj()?.get_array_default("shareable")? {
            if let Json::String(s) = v {
                cfg.shareable_items.insert(s);
            }
        }
    }
    Ok(cfg)
}

fn read_versioned_json(path: &Path) -> Result<Json> {
    let json = read_json(path.to_path_buf())?;
    let version = json
        .obj()?
        .get_usize_default("schemaVersion", SCHEMA_VERSION as usize)?;
    if version as u32 != SCHEMA_VERSION {
        return Err(format!(
            "{}: unsupported schemaVersion {}; expected {}",
            path.display(),
            version,
            SCHEMA_VERSION
        ));
    }
    Ok(json)
}

fn list_json_files(path: &Path) -> Result<Vec<PathBuf>> {
    if !path.exists() {
        return Ok(Vec::new());
    }
    let mut files = Vec::new();
    for entry in fs::read_dir(path).map_err(|e| format!("read_dir {}: {e}", path.display()))? {
        let entry = entry.map_err(|e| format!("read_dir {}: {e}", path.display()))?;
        let p = entry.path();
        if p.extension().and_then(|e| e.to_str()) == Some("json") {
            files.push(p);
        }
    }
    files.sort();
    Ok(files)
}

fn read_json(path: PathBuf) -> Result<Json> {
    JsonParser::new(&read_to_string(&path)?)
        .parse()
        .map_err(|e| format!("{}: {e}", path.display()))
}

fn parse_journey(json: &Json) -> Result<Vec<JourneyTrainer>> {
    let mut result = Vec::new();
    let mut journey_index = 0usize;
    for (stage_index, stage) in json.obj()?.get_array("stages")?.iter().enumerate() {
        let stage_obj = stage.obj()?;
        let stage_id = stage_obj.get_string("stageId")?.to_string();
        for (trainer_index, trainer) in stage_obj.get_array("trainers")?.iter().enumerate() {
            let obj = trainer.obj()?;
            result.push(JourneyTrainer {
                trainer_const: obj.get_string("trainerConst")?.to_string(),
                blueprint_id: obj.get_string("blueprintId")?.to_string(),
                tags: obj.get_string_array_default("tags")?,
                stage_id: stage_id.clone(),
                journey_index,
                stage_index,
                trainer_index,
            });
            journey_index += 1;
        }
    }
    Ok(result)
}

fn parse_blueprint(json: &Json) -> Result<Blueprint> {
    let obj = json.obj()?;
    let constraints = match obj.get_optional("constraints") {
        Some(value) => {
            let c = value.obj()?;
            Constraints {
                min_local_pool_size: c.get_usize_default("minLocalPoolSize", 0)?,
                max_local_pool_size: c.get_usize_default("maxLocalPoolSize", 0)?,
            }
        }
        None => Constraints::default(),
    };
    let required = obj
        .get_array_default("required")?
        .iter()
        .map(|entry| {
            let obj = entry.obj()?;
            Ok(RequiredSlot {
                slot: obj.get_string("slot")?.to_string(),
                roles: obj.get_string_array("roles")?,
            })
        })
        .collect::<Result<Vec<_>>>()?;
    let preferred = obj
        .get_array_default("preferred")?
        .iter()
        .map(|entry| {
            let obj = entry.obj()?;
            Ok(PreferredRole {
                role: obj.get_string("role")?.to_string(),
                min: obj.get_usize_default("min", 0)?,
                max: obj.get_usize_default("max", usize::MAX)?,
            })
        })
        .collect::<Result<Vec<_>>>()?;
    Ok(Blueprint {
        id: obj.get_string("blueprintId")?.to_string(),
        party_size: obj.get_usize("partySize")?,
        pool_size: obj.get_usize("poolSize")?,
        ruleset_id: obj
            .get_string_default("rulesetId", "POOL_RULESET_BASIC")?
            .to_string(),
        required,
        preferred,
        constraints,
        mode: obj.get_string_default("mode", "single")?.to_string(),
        rank: obj.get_string_default("rank", "mid")?.to_string(),
        set_groups: obj.get_string_array_default("setGroups")?,
        require_spread_move: obj.get_bool_default("requireSpreadMove", true)?,
        allow_species_duplicate: obj.get_bool_default("allowSpeciesDuplicate", false)?,
    })
}

fn parse_sets(json: &Json) -> Result<Vec<Set>> {
    json.obj()?
        .get_array("sets")?
        .iter()
        .map(|entry| {
            let obj = entry.obj()?;
            Ok(Set {
                id: obj.get_string("id")?.to_string(),
                species: obj.get_string("species")?.to_string(),
                ability: obj.get_optional_string("ability")?,
                moves: obj.get_string_array("moves")?,
                item: obj.get_optional_string("item")?,
                ivs: obj
                    .get_string_default("ivs", "31/31/31/31/31/31")?
                    .to_string(),
                evs: obj.get_string_default("evs", "0/0/0/0/0/0")?.to_string(),
                nature: obj.get_optional_string("nature")?,
                level: obj.get_usize_default("level", 50)? as u16,
                roles: obj.get_string_array_default("roles")?,
                archetypes: obj.get_string_array_default("archetypes")?,
                groups: obj.get_string_array_default("groups")?,
                lint_tags: obj.get_string_array_default("lintTags")?,
                tags: obj.get_string_array_default("tags")?,
                min_rank: obj.get_string_default("minRank", "early")?.to_string(),
                max_rank: obj.get_string_default("maxRank", "champion")?.to_string(),
                doubles_spread_move: obj.get_bool_default("doublesSpreadMove", false)?,
            })
        })
        .collect()
}

#[derive(Debug, Clone)]
enum Json {
    Null,
    Bool(bool),
    Number(i64),
    Float(f64),
    String(String),
    Array(Vec<Json>),
    Object(BTreeMap<String, Json>),
}

impl Json {
    fn obj(&self) -> Result<&BTreeMap<String, Json>> {
        match self {
            Json::Object(obj) => Ok(obj),
            _ => Err("expected object".to_string()),
        }
    }

    fn as_f64(&self) -> Option<f64> {
        match self {
            Json::Number(n) => Some(*n as f64),
            Json::Float(f) => Some(*f),
            _ => None,
        }
    }

    fn as_i64(&self) -> Option<i64> {
        match self {
            Json::Number(n) => Some(*n),
            Json::Float(f) => Some(*f as i64),
            _ => None,
        }
    }
}

trait JsonObjectExt {
    fn get_optional(&self, key: &str) -> Option<&Json>;
    fn get_string(&self, key: &str) -> Result<&str>;
    fn get_string_default<'a>(&'a self, key: &str, default: &'a str) -> Result<&'a str>;
    fn get_optional_string(&self, key: &str) -> Result<Option<String>>;
    fn get_array(&self, key: &str) -> Result<&Vec<Json>>;
    fn get_array_default(&self, key: &str) -> Result<Vec<Json>>;
    fn get_string_array(&self, key: &str) -> Result<Vec<String>>;
    fn get_string_array_default(&self, key: &str) -> Result<Vec<String>>;
    fn get_usize(&self, key: &str) -> Result<usize>;
    fn get_usize_default(&self, key: &str, default: usize) -> Result<usize>;
    fn get_i64_default(&self, key: &str, default: i64) -> Result<i64>;
    fn get_f64_default(&self, key: &str, default: f64) -> Result<f64>;
    fn get_bool_default(&self, key: &str, default: bool) -> Result<bool>;
}

impl JsonObjectExt for BTreeMap<String, Json> {
    fn get_optional(&self, key: &str) -> Option<&Json> {
        self.get(key)
    }

    fn get_string(&self, key: &str) -> Result<&str> {
        match self.get(key) {
            Some(Json::String(value)) => Ok(value),
            Some(_) => Err(format!("{key}: expected string")),
            None => Err(format!("{key}: missing")),
        }
    }

    fn get_string_default<'a>(&'a self, key: &str, default: &'a str) -> Result<&'a str> {
        match self.get(key) {
            Some(Json::String(value)) => Ok(value),
            Some(_) => Err(format!("{key}: expected string")),
            None => Ok(default),
        }
    }

    fn get_optional_string(&self, key: &str) -> Result<Option<String>> {
        match self.get(key) {
            Some(Json::String(value)) => Ok(Some(value.clone())),
            Some(Json::Null) | None => Ok(None),
            Some(_) => Err(format!("{key}: expected string")),
        }
    }

    fn get_array(&self, key: &str) -> Result<&Vec<Json>> {
        match self.get(key) {
            Some(Json::Array(value)) => Ok(value),
            Some(_) => Err(format!("{key}: expected array")),
            None => Err(format!("{key}: missing")),
        }
    }

    fn get_array_default(&self, key: &str) -> Result<Vec<Json>> {
        match self.get(key) {
            Some(Json::Array(value)) => Ok(value.clone()),
            Some(_) => Err(format!("{key}: expected array")),
            None => Ok(Vec::new()),
        }
    }

    fn get_string_array(&self, key: &str) -> Result<Vec<String>> {
        self.get_array(key)?
            .iter()
            .map(|entry| match entry {
                Json::String(value) => Ok(value.clone()),
                _ => Err(format!("{key}: expected string array")),
            })
            .collect()
    }

    fn get_string_array_default(&self, key: &str) -> Result<Vec<String>> {
        match self.get(key) {
            Some(_) => self.get_string_array(key),
            None => Ok(Vec::new()),
        }
    }

    fn get_usize(&self, key: &str) -> Result<usize> {
        match self.get(key) {
            Some(Json::Number(value)) if *value >= 0 => Ok(*value as usize),
            Some(_) => Err(format!("{key}: expected positive number")),
            None => Err(format!("{key}: missing")),
        }
    }

    fn get_usize_default(&self, key: &str, default: usize) -> Result<usize> {
        match self.get(key) {
            Some(_) => self.get_usize(key),
            None => Ok(default),
        }
    }

    fn get_i64_default(&self, key: &str, default: i64) -> Result<i64> {
        match self.get(key) {
            Some(v) => v.as_i64().ok_or_else(|| format!("{key}: expected number")),
            None => Ok(default),
        }
    }

    fn get_f64_default(&self, key: &str, default: f64) -> Result<f64> {
        match self.get(key) {
            Some(v) => v.as_f64().ok_or_else(|| format!("{key}: expected number")),
            None => Ok(default),
        }
    }

    fn get_bool_default(&self, key: &str, default: bool) -> Result<bool> {
        match self.get(key) {
            Some(Json::Bool(b)) => Ok(*b),
            Some(_) => Err(format!("{key}: expected boolean")),
            None => Ok(default),
        }
    }
}

struct JsonParser<'a> {
    input: &'a [u8],
    pos: usize,
}

impl<'a> JsonParser<'a> {
    fn new(input: &'a str) -> Self {
        Self {
            input: input.as_bytes(),
            pos: 0,
        }
    }

    fn parse(mut self) -> Result<Json> {
        let value = self.parse_value()?;
        self.skip_ws();
        if self.pos != self.input.len() {
            Err(format!("unexpected trailing byte at {}", self.pos))
        } else {
            Ok(value)
        }
    }

    fn parse_value(&mut self) -> Result<Json> {
        self.skip_ws();
        match self.peek() {
            Some(b'{') => self.parse_object(),
            Some(b'[') => self.parse_array(),
            Some(b'"') => self.parse_string().map(Json::String),
            Some(b'-') | Some(b'0'..=b'9') => self.parse_number(),
            Some(b't') => {
                self.expect_bytes(b"true")?;
                Ok(Json::Bool(true))
            }
            Some(b'f') => {
                self.expect_bytes(b"false")?;
                Ok(Json::Bool(false))
            }
            Some(b'n') => {
                self.expect_bytes(b"null")?;
                Ok(Json::Null)
            }
            Some(byte) => Err(format!(
                "unexpected byte '{}' at {}",
                byte as char, self.pos
            )),
            None => Err("unexpected end of JSON".to_string()),
        }
    }

    fn parse_object(&mut self) -> Result<Json> {
        self.expect(b'{')?;
        let mut obj = BTreeMap::new();
        loop {
            self.skip_ws();
            if self.consume(b'}') {
                break;
            }
            let key = self.parse_string()?;
            self.skip_ws();
            self.expect(b':')?;
            let value = self.parse_value()?;
            obj.insert(key, value);
            self.skip_ws();
            if self.consume(b'}') {
                break;
            }
            self.expect(b',')?;
        }
        Ok(Json::Object(obj))
    }

    fn parse_array(&mut self) -> Result<Json> {
        self.expect(b'[')?;
        let mut arr = Vec::new();
        loop {
            self.skip_ws();
            if self.consume(b']') {
                break;
            }
            arr.push(self.parse_value()?);
            self.skip_ws();
            if self.consume(b']') {
                break;
            }
            self.expect(b',')?;
        }
        Ok(Json::Array(arr))
    }

    fn parse_string(&mut self) -> Result<String> {
        self.expect(b'"')?;
        let mut out = String::new();
        while let Some(byte) = self.peek() {
            self.pos += 1;
            match byte {
                b'"' => return Ok(out),
                b'\\' => {
                    let esc = self
                        .peek()
                        .ok_or_else(|| "unterminated escape".to_string())?;
                    self.pos += 1;
                    match esc {
                        b'"' => out.push('"'),
                        b'\\' => out.push('\\'),
                        b'/' => out.push('/'),
                        b'b' => out.push('\u{0008}'),
                        b'f' => out.push('\u{000c}'),
                        b'n' => out.push('\n'),
                        b'r' => out.push('\r'),
                        b't' => out.push('\t'),
                        b'u' => {
                            let code = self.parse_hex4()?;
                            let ch = char::from_u32(code)
                                .ok_or_else(|| format!("invalid unicode escape {code:x}"))?;
                            out.push(ch);
                        }
                        _ => return Err(format!("invalid escape '{}'", esc as char)),
                    }
                }
                _ => out.push(byte as char),
            }
        }
        Err("unterminated string".to_string())
    }

    fn parse_number(&mut self) -> Result<Json> {
        let start = self.pos;
        self.consume(b'-');
        while matches!(self.peek(), Some(b'0'..=b'9')) {
            self.pos += 1;
        }
        let mut is_float = false;
        if self.consume(b'.') {
            is_float = true;
            while matches!(self.peek(), Some(b'0'..=b'9')) {
                self.pos += 1;
            }
        }
        if matches!(self.peek(), Some(b'e' | b'E')) {
            is_float = true;
            self.pos += 1;
            if matches!(self.peek(), Some(b'+' | b'-')) {
                self.pos += 1;
            }
            while matches!(self.peek(), Some(b'0'..=b'9')) {
                self.pos += 1;
            }
        }
        let text = std::str::from_utf8(&self.input[start..self.pos]).map_err(|e| e.to_string())?;
        if is_float {
            text.parse::<f64>()
                .map(Json::Float)
                .map_err(|_| format!("invalid number at {start}"))
        } else {
            text.parse::<i64>()
                .map(Json::Number)
                .map_err(|_| format!("invalid number at {start}"))
        }
    }

    fn parse_hex4(&mut self) -> Result<u32> {
        if self.pos + 4 > self.input.len() {
            return Err("short unicode escape".to_string());
        }
        let mut code = 0u32;
        for _ in 0..4 {
            code <<= 4;
            code += match self.input[self.pos] {
                b'0'..=b'9' => (self.input[self.pos] - b'0') as u32,
                b'a'..=b'f' => (self.input[self.pos] - b'a' + 10) as u32,
                b'A'..=b'F' => (self.input[self.pos] - b'A' + 10) as u32,
                _ => return Err("invalid unicode escape".to_string()),
            };
            self.pos += 1;
        }
        Ok(code)
    }

    fn skip_ws(&mut self) {
        while matches!(self.peek(), Some(b' ' | b'\n' | b'\r' | b'\t')) {
            self.pos += 1;
        }
    }

    fn expect(&mut self, byte: u8) -> Result<()> {
        if self.consume(byte) {
            Ok(())
        } else {
            Err(format!("expected '{}' at {}", byte as char, self.pos))
        }
    }

    fn expect_bytes(&mut self, bytes: &[u8]) -> Result<()> {
        for byte in bytes {
            self.expect(*byte)?;
        }
        Ok(())
    }

    fn consume(&mut self, byte: u8) -> bool {
        if self.peek() == Some(byte) {
            self.pos += 1;
            true
        } else {
            false
        }
    }

    fn peek(&self) -> Option<u8> {
        self.input.get(self.pos).copied()
    }
}

#[derive(Clone)]
struct PartyBlock {
    name: String,
    text: String,
    header_lines: Vec<String>,
}

fn parse_party_blocks(text: &str) -> BTreeMap<String, PartyBlock> {
    let mut starts = Vec::new();
    let mut offset = 0usize;
    for line in text.split_inclusive('\n') {
        let trimmed = line.trim();
        if trimmed.starts_with("=== ") && trimmed.ends_with(" ===") {
            let name = trimmed
                .trim_start_matches("=== ")
                .trim_end_matches(" ===")
                .to_string();
            starts.push((offset, name));
        }
        offset += line.len();
    }

    let mut blocks = BTreeMap::new();
    for i in 0..starts.len() {
        let start = starts[i].0;
        let end = starts.get(i + 1).map(|next| next.0).unwrap_or(text.len());
        let name = starts[i].1.clone();
        let block_text = text[start..end].trim_end().to_string();
        let header_lines = extract_header_lines(&block_text);
        blocks.insert(
            name.clone(),
            PartyBlock {
                name,
                text: block_text,
                header_lines,
            },
        );
    }
    blocks
}

fn extract_header_lines(block: &str) -> Vec<String> {
    let mut lines = block.lines();
    lines.next();
    let mut header = Vec::new();
    for line in lines {
        if line.trim().is_empty() {
            break;
        }
        header.push(line.to_string());
    }
    header
}

fn apply_fragment(source: &str, fragment: &str) -> Result<String> {
    let generated = parse_party_blocks(fragment);
    if generated.is_empty() {
        return Err("input fragment has no trainer blocks".to_string());
    }
    let mut out = String::new();
    let mut starts = Vec::new();
    let mut offset = 0usize;
    for line in source.split_inclusive('\n') {
        let trimmed = line.trim();
        if trimmed.starts_with("=== ") && trimmed.ends_with(" ===") {
            let name = trimmed
                .trim_start_matches("=== ")
                .trim_end_matches(" ===")
                .to_string();
            starts.push((offset, name));
        }
        offset += line.len();
    }
    let mut replaced = BTreeSet::new();
    let mut cursor = 0usize;
    for i in 0..starts.len() {
        let start = starts[i].0;
        let end = starts.get(i + 1).map(|next| next.0).unwrap_or(source.len());
        out.push_str(&source[cursor..start]);
        if let Some(block) = generated.get(&starts[i].1) {
            out.push_str(block.text.trim_end());
            out.push_str("\n\n");
            replaced.insert(starts[i].1.clone());
        } else {
            out.push_str(source[start..end].trim_end());
            out.push_str("\n\n");
        }
        cursor = end;
    }
    out.push_str(source[cursor..].trim_start_matches('\n'));
    for name in generated.keys() {
        if !replaced.contains(name) {
            return Err(format!(
                "{name} does not exist in target; MVP only replaces blocks"
            ));
        }
    }
    Ok(out.trim_end().to_string() + "\n")
}

fn normalize_block(text: &str) -> String {
    text.lines()
        .map(str::trim_end)
        .collect::<Vec<_>>()
        .join("\n")
}

fn count_mons(block: &str) -> usize {
    block
        .lines()
        .filter(|line| is_mon_start(line.trim()))
        .count()
}

fn is_mon_start(line: &str) -> bool {
    if line.is_empty()
        || line.starts_with("===")
        || line.starts_with("/*")
        || line.starts_with('*')
        || line.contains(':')
        || line.starts_with('-')
    {
        return false;
    }
    true
}

struct ConstantIndex {
    trainers: BTreeSet<String>,
    species: BTreeSet<String>,
    moves: BTreeSet<String>,
    items: BTreeSet<String>,
    abilities: BTreeSet<String>,
    natures: BTreeSet<String>,
}

impl ConstantIndex {
    fn load(repo: &Path) -> Result<Self> {
        Ok(Self {
            trainers: collect_prefixed(repo, "include/constants/opponents.h", "TRAINER_")?,
            species: collect_prefixed(repo, "include/constants/species.h", "SPECIES_")?,
            moves: collect_prefixed(repo, "include/constants/moves.h", "MOVE_")?,
            items: collect_prefixed(repo, "include/constants/items.h", "ITEM_")?,
            abilities: collect_prefixed(repo, "include/constants/abilities.h", "ABILITY_")?,
            natures: collect_prefixed(repo, "include/constants/pokemon.h", "NATURE_")
                .or_else(|_| collect_prefixed(repo, "include/constants/battle.h", "NATURE_"))
                .unwrap_or_else(|_| default_natures()),
        })
    }

    fn check(&self, kind: &str, value: &str, report: &mut Report) {
        let ok = match kind {
            "trainer" => self.trainers.contains(value),
            "species" => self.species.contains(value),
            "move" => self.moves.contains(value),
            "item" => self.items.contains(value),
            "ability" => self.abilities.contains(value),
            "nature" => self.natures.contains(value),
            _ => true,
        };
        if !ok {
            report.error(format!("unknown {kind} constant {value}"));
        }
    }
}

fn collect_prefixed(repo: &Path, relative: &str, prefix: &str) -> Result<BTreeSet<String>> {
    let text = read_to_string(repo.join(relative))?;
    let mut values = BTreeSet::new();
    for token in split_tokens(&text) {
        if token.starts_with(prefix) {
            values.insert(token.to_string());
        }
    }
    Ok(values)
}

fn split_tokens(text: &str) -> impl Iterator<Item = &str> {
    text.split(|c: char| !c.is_ascii_alphanumeric() && c != '_')
        .filter(|s| !s.is_empty())
}

fn default_natures() -> BTreeSet<String> {
    [
        "NATURE_HARDY",
        "NATURE_LONELY",
        "NATURE_BRAVE",
        "NATURE_ADAMANT",
        "NATURE_NAUGHTY",
        "NATURE_BOLD",
        "NATURE_DOCILE",
        "NATURE_RELAXED",
        "NATURE_IMPISH",
        "NATURE_LAX",
        "NATURE_TIMID",
        "NATURE_HASTY",
        "NATURE_SERIOUS",
        "NATURE_JOLLY",
        "NATURE_NAIVE",
        "NATURE_MODEST",
        "NATURE_MILD",
        "NATURE_QUIET",
        "NATURE_BASHFUL",
        "NATURE_RASH",
        "NATURE_CALM",
        "NATURE_GENTLE",
        "NATURE_SASSY",
        "NATURE_CAREFUL",
        "NATURE_QUIRKY",
    ]
    .into_iter()
    .map(String::from)
    .collect()
}

fn scan_script_usage(repo: &Path) -> Result<BTreeMap<String, usize>> {
    let mut usage = BTreeMap::new();
    let roots = [repo.join("data/maps"), repo.join("data/scripts")];
    for root in roots {
        scan_script_dir(&root, &mut usage)?;
    }
    Ok(usage)
}

fn scan_script_dir(path: &Path, usage: &mut BTreeMap<String, usize>) -> Result<()> {
    if !path.exists() {
        return Ok(());
    }
    for entry in fs::read_dir(path).map_err(|e| format!("read_dir {}: {e}", path.display()))? {
        let entry = entry.map_err(|e| format!("read_dir {}: {e}", path.display()))?;
        let path = entry.path();
        if path.is_dir() {
            scan_script_dir(&path, usage)?;
        } else if path.extension().and_then(|e| e.to_str()) == Some("inc") {
            let text = read_to_string(&path)?;
            for token in split_tokens(&text) {
                if token.starts_with("TRAINER_") {
                    *usage.entry(token.to_string()).or_default() += 1;
                }
            }
        }
    }
    Ok(())
}

struct Rng(u64);

impl Rng {
    fn new(seed: u64) -> Self {
        Self(seed ^ 0x9e37_79b9_7f4a_7c15)
    }

    fn next_u64(&mut self) -> u64 {
        self.0 = self
            .0
            .wrapping_mul(6364136223846793005)
            .wrapping_add(1442695040888963407);
        self.0
    }

    fn next_usize(&mut self, max: usize) -> usize {
        (self.next_u64() as usize) % max
    }

    fn next_f64(&mut self) -> f64 {
        let value = self.next_u64() >> 11;
        value as f64 / ((1u64 << 53) as f64)
    }
}

fn seed_for(seed: u64, trainer: &str) -> u64 {
    let mut hash = 1469598103934665603u64 ^ seed;
    for byte in trainer.as_bytes() {
        hash ^= *byte as u64;
        hash = hash.wrapping_mul(1099511628211);
    }
    hash
}

fn read_to_string<P: AsRef<Path>>(path: P) -> Result<String> {
    fs::read_to_string(path.as_ref()).map_err(|e| format!("read {}: {e}", path.as_ref().display()))
}

fn write_file(path: &Path, contents: &str) -> Result<()> {
    if let Some(parent) = path.parent() {
        if !parent.as_os_str().is_empty() {
            fs::create_dir_all(parent)
                .map_err(|e| format!("create_dir_all {}: {e}", parent.display()))?;
        }
    }
    fs::write(path, contents).map_err(|e| format!("write {}: {e}", path.display()))
}

// ---------- Audit log ----------

struct AuditLog {
    schema_version: u32,
    run_id: String,
    seed: u64,
    catalog_path: String,
    catalog_hash: String,
    profile_path: Option<String>,
    profile_status: Option<String>,
    profile_catalog_version: Option<String>,
    profile_warnings: Vec<String>,
    trainers: Vec<TrainerAudit>,
}

struct TrainerAudit {
    trainer_const: String,
    tags: Vec<String>,
    stage_id: String,
    journey_index: usize,
    stage_index: usize,
    trainer_index: usize,
    blueprint_id: String,
    mode: String,
    rank: String,
    lints: Vec<LintIssue>,
    selections: Vec<SelectionRecord>,
}

struct SelectionRecord {
    slot: String,
    set_id: String,
    rule: String,
    weight: f64,
}

impl AuditLog {
    fn severity_counts(&self) -> (usize, usize, usize) {
        let mut e = 0;
        let mut w = 0;
        let mut n = 0;
        for t in &self.trainers {
            for l in &t.lints {
                match l.severity {
                    Severity::Error => e += 1,
                    Severity::Warning => w += 1,
                    Severity::Note => n += 1,
                }
            }
        }
        (e, w, n)
    }
    fn lint_summary(&self) -> String {
        let mut out = String::new();
        for t in &self.trainers {
            for l in &t.lints {
                if matches!(l.severity, Severity::Error | Severity::Warning) {
                    out.push_str(&format!("{}\n", l));
                }
            }
        }
        out
    }
}

fn audit_to_json(log: &AuditLog) -> String {
    let mut w = JsonWriter::new();
    w.begin_object();
    w.kv_number("schemaVersion", log.schema_version as i64);
    w.kv_string("runId", &log.run_id);
    w.kv_number("seed", log.seed as i64);
    w.kv_string("catalogPath", &log.catalog_path);
    w.kv_string("catalogHash", &log.catalog_hash);
    match &log.profile_path {
        Some(p) => w.kv_string("profilePath", p),
        None => w.kv_null("profilePath"),
    }
    match &log.profile_status {
        Some(s) => w.kv_string("profileStatus", s),
        None => w.kv_null("profileStatus"),
    }
    match &log.profile_catalog_version {
        Some(v) => w.kv_string("profileCatalogVersion", v),
        None => w.kv_null("profileCatalogVersion"),
    }
    write_string_array(&mut w, "profileWarnings", &log.profile_warnings);
    w.key("trainers");
    w.begin_array();
    for t in &log.trainers {
        w.begin_object();
        w.kv_string("trainerConst", &t.trainer_const);
        write_string_array(&mut w, "tags", &t.tags);
        w.kv_string("stageId", &t.stage_id);
        w.kv_number("journeyIndex", t.journey_index as i64);
        w.kv_number("stageIndex", t.stage_index as i64);
        w.kv_number("trainerIndex", t.trainer_index as i64);
        w.kv_string("blueprintId", &t.blueprint_id);
        w.kv_string("mode", &t.mode);
        w.kv_string("rank", &t.rank);
        w.key("lints");
        w.begin_array();
        for l in &t.lints {
            w.begin_object();
            w.kv_string("id", &l.id);
            w.kv_string("severity", l.severity.as_str());
            match &l.trainer {
                Some(s) => w.kv_string("trainer", s),
                None => w.kv_null("trainer"),
            }
            w.kv_string("msg", &l.msg);
            w.end_object();
        }
        w.end_array();
        w.key("selections");
        w.begin_array();
        for s in &t.selections {
            w.begin_object();
            w.kv_string("slot", &s.slot);
            w.kv_string("setId", &s.set_id);
            w.kv_string("rule", &s.rule);
            w.kv_float("weight", s.weight);
            w.end_object();
        }
        w.end_array();
        w.end_object();
    }
    w.end_array();
    w.end_object();
    w.into_string()
}

fn build_run_id(seed: u64) -> String {
    let now = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap_or_default();
    let secs = now.as_secs();
    let nanos = now.subsec_nanos();
    format!(
        "{}-{:08x}-{:x}",
        utc_compact(secs),
        seed as u32 ^ nanos,
        nanos & 0xffff
    )
}

fn utc_compact(secs: u64) -> String {
    // YYYYMMDDTHHMMSSZ-style timestamp without depending on chrono.
    let (y, mo, d, h, mi, s) = unix_to_ymdhms(secs);
    format!("{:04}{:02}{:02}T{:02}{:02}{:02}Z", y, mo, d, h, mi, s)
}

fn unix_to_ymdhms(mut secs: u64) -> (u64, u64, u64, u64, u64, u64) {
    let s = secs % 60;
    secs /= 60;
    let mi = secs % 60;
    secs /= 60;
    let h = secs % 24;
    secs /= 24;
    let mut days = secs as i64;
    let mut y: i64 = 1970;
    loop {
        let yd = if is_leap(y) { 366 } else { 365 };
        if days < yd {
            break;
        }
        days -= yd;
        y += 1;
    }
    let months = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
    let mut mo = 0usize;
    while mo < 12 {
        let md = if mo == 1 && is_leap(y) {
            29
        } else {
            months[mo]
        } as i64;
        if days < md {
            break;
        }
        days -= md;
        mo += 1;
    }
    (y as u64, (mo + 1) as u64, (days + 1) as u64, h, mi, s)
}

fn is_leap(y: i64) -> bool {
    (y % 4 == 0 && y % 100 != 0) || y % 400 == 0
}

fn hash_catalog(path: &Path) -> Result<String> {
    let mut hasher = FnvHasher::new();
    hash_dir(path, path, &mut hasher)?;
    Ok(format!("fnv64:{:016x}", hasher.0))
}

fn hash_dir(root: &Path, path: &Path, hasher: &mut FnvHasher) -> Result<()> {
    if !path.exists() {
        return Ok(());
    }
    let mut entries: Vec<PathBuf> = fs::read_dir(path)
        .map_err(|e| format!("read_dir {}: {e}", path.display()))?
        .map(|e| e.map(|e| e.path()))
        .collect::<std::result::Result<Vec<_>, _>>()
        .map_err(|e| e.to_string())?;
    entries.sort();
    for p in entries {
        if p.is_dir() {
            hash_dir(root, &p, hasher)?;
        } else if p.extension().and_then(|e| e.to_str()) == Some("json") {
            let text = read_to_string(&p)?;
            let rel = p.strip_prefix(root).unwrap_or(&p);
            hasher.update(rel.display().to_string().as_bytes());
            hasher.update(b"\0");
            hasher.update(text.as_bytes());
            hasher.update(b"\n");
        }
    }
    Ok(())
}

struct FnvHasher(u64);

impl FnvHasher {
    fn new() -> Self {
        Self(0xcbf29ce484222325)
    }
    fn update(&mut self, bytes: &[u8]) {
        for b in bytes {
            self.0 ^= *b as u64;
            self.0 = self.0.wrapping_mul(0x100000001b3);
        }
    }
}

// ---------- JSON Writer ----------

struct JsonWriter {
    buf: String,
    indent: usize,
    needs_comma: Vec<bool>,
    after_key: bool,
}

impl JsonWriter {
    fn new() -> Self {
        Self {
            buf: String::new(),
            indent: 0,
            needs_comma: Vec::new(),
            after_key: false,
        }
    }

    fn into_string(self) -> String {
        self.buf
    }

    fn write_indent(&mut self) {
        for _ in 0..self.indent {
            self.buf.push_str("  ");
        }
    }

    fn comma_if_needed(&mut self) {
        if let Some(needs) = self.needs_comma.last_mut() {
            if *needs {
                self.buf.push(',');
                self.buf.push('\n');
            }
            *needs = true;
        }
    }

    fn begin_value(&mut self) {
        if self.after_key {
            self.after_key = false;
            return;
        }
        self.comma_if_needed();
        if !self.needs_comma.is_empty() {
            self.write_indent();
        }
    }

    fn begin_object(&mut self) {
        self.begin_value();
        self.buf.push_str("{\n");
        self.indent += 1;
        self.needs_comma.push(false);
    }

    fn end_object(&mut self) {
        self.needs_comma.pop();
        self.buf.push('\n');
        self.indent -= 1;
        self.write_indent();
        self.buf.push('}');
        if self.needs_comma.is_empty() {
            self.buf.push('\n');
        }
    }

    fn begin_array(&mut self) {
        self.begin_value();
        self.buf.push_str("[\n");
        self.indent += 1;
        self.needs_comma.push(false);
    }

    fn end_array(&mut self) {
        self.needs_comma.pop();
        self.buf.push('\n');
        self.indent -= 1;
        self.write_indent();
        self.buf.push(']');
    }

    fn key(&mut self, k: &str) {
        self.comma_if_needed();
        self.write_indent();
        self.buf.push('"');
        self.buf.push_str(&escape_json(k));
        self.buf.push_str("\": ");
        self.after_key = true;
    }

    fn kv_string(&mut self, k: &str, v: &str) {
        self.key(k);
        self.after_key = false;
        self.buf.push('"');
        self.buf.push_str(&escape_json(v));
        self.buf.push('"');
    }

    fn kv_number(&mut self, k: &str, v: i64) {
        self.key(k);
        self.after_key = false;
        self.buf.push_str(&v.to_string());
    }

    fn kv_float(&mut self, k: &str, v: f64) {
        self.key(k);
        self.after_key = false;
        if v.fract() == 0.0 && v.is_finite() {
            self.buf.push_str(&format!("{:.1}", v));
        } else {
            self.buf.push_str(&format!("{}", v));
        }
    }

    fn kv_null(&mut self, k: &str) {
        self.key(k);
        self.after_key = false;
        self.buf.push_str("null");
    }

    fn raw_value(&mut self, v: &str) {
        self.begin_value();
        self.buf.push_str(v);
    }
}

fn escape_json(s: &str) -> String {
    let mut out = String::with_capacity(s.len());
    for c in s.chars() {
        match c {
            '"' => out.push_str("\\\""),
            '\\' => out.push_str("\\\\"),
            '\n' => out.push_str("\\n"),
            '\r' => out.push_str("\\r"),
            '\t' => out.push_str("\\t"),
            c if (c as u32) < 0x20 => out.push_str(&format!("\\u{:04x}", c as u32)),
            c => out.push(c),
        }
    }
    out
}

// ---------- Raw log → normalized JSONL ----------

#[derive(Clone, Debug)]
struct NormalizedBattle {
    schema_version: u32,
    run_id: String,
    battle_id: i64,
    trainer_const: String,
    format: String,
    outcome: String,
    turns: i64,
    player_party: Vec<NormalizedMon>,
    player_faint_order: Vec<String>,
    opponent_faint_order: Vec<String>,
    switches_by_player: i64,
    switches_by_opponent: i64,
    items_used_by_player: Vec<String>,
    key_moves: Vec<String>,
    warnings: Vec<String>,
}

#[derive(Clone, Debug, Default)]
struct NormalizedMon {
    species: String,
    level: i64,
    item: Option<String>,
    moves: Vec<String>,
}

fn collect_raw_log_files(path: &Path) -> Result<Vec<PathBuf>> {
    if !path.exists() {
        return Err(format!("missing input path {}", path.display()));
    }
    if path.is_file() {
        return Ok(vec![path.to_path_buf()]);
    }
    let mut out = Vec::new();
    walk_files(path, &mut out, "log")?;
    out.sort();
    Ok(out)
}

fn collect_jsonl_files(path: &Path) -> Result<Vec<PathBuf>> {
    if !path.exists() {
        return Err(format!("missing input path {}", path.display()));
    }
    if path.is_file() {
        return Ok(vec![path.to_path_buf()]);
    }
    let mut out = Vec::new();
    walk_files(path, &mut out, "jsonl")?;
    out.sort();
    Ok(out)
}

fn walk_files(path: &Path, out: &mut Vec<PathBuf>, ext: &str) -> Result<()> {
    for entry in fs::read_dir(path).map_err(|e| format!("read_dir {}: {e}", path.display()))? {
        let entry = entry.map_err(|e| format!("read_dir {}: {e}", path.display()))?;
        let p = entry.path();
        if p.is_dir() {
            walk_files(&p, out, ext)?;
        } else if p.extension().and_then(|e| e.to_str()) == Some(ext) {
            out.push(p);
        }
    }
    Ok(())
}

fn parse_raw_log(text: &str, source: &Path) -> (Vec<NormalizedBattle>, Vec<String>) {
    let mut rows: Vec<NormalizedBattle> = Vec::new();
    let mut errors: Vec<String> = Vec::new();
    let run_id = source
        .file_stem()
        .and_then(|s| s.to_str())
        .unwrap_or("run")
        .to_string();
    let mut current: Option<NormalizedBattle> = None;
    for (lineno, line) in text.lines().enumerate() {
        if line.trim().is_empty() {
            continue;
        }
        let mut parts = line.split('\t');
        let _ts = match parts.next() {
            Some(t) => t,
            None => {
                errors.push(format!("line {}: missing timestamp", lineno + 1));
                continue;
            }
        };
        let event = match parts.next() {
            Some(e) => e,
            None => {
                errors.push(format!("line {}: missing event", lineno + 1));
                continue;
            }
        };
        let mut kv: BTreeMap<String, String> = BTreeMap::new();
        for token in parts {
            if let Some((k, v)) = token.split_once('=') {
                kv.insert(k.to_string(), v.to_string());
            }
        }
        match event {
            "session_start" => {}
            "battle_start" => {
                if let Some(prev) = current.take() {
                    rows.push(prev);
                }
                let battle_id = kv
                    .get("battleId")
                    .and_then(|v| v.parse().ok())
                    .unwrap_or_default();
                current = Some(NormalizedBattle {
                    schema_version: SCHEMA_VERSION,
                    run_id: run_id.clone(),
                    battle_id,
                    trainer_const: kv
                        .get("trainerConst")
                        .cloned()
                        .unwrap_or_else(|| "TRAINER_NONE".to_string()),
                    format: kv
                        .get("format")
                        .cloned()
                        .unwrap_or_else(|| "single".to_string()),
                    outcome: "incomplete".to_string(),
                    turns: 0,
                    player_party: Vec::new(),
                    player_faint_order: Vec::new(),
                    opponent_faint_order: Vec::new(),
                    switches_by_player: 0,
                    switches_by_opponent: 0,
                    items_used_by_player: Vec::new(),
                    key_moves: Vec::new(),
                    warnings: Vec::new(),
                });
            }
            "turn_start" => {
                if let Some(b) = current.as_mut() {
                    if let Some(turn) = kv.get("turn").and_then(|v| v.parse().ok()) {
                        b.turns = turn;
                    }
                }
            }
            "move_used" => {
                if let Some(b) = current.as_mut() {
                    if kv.get("side").map(String::as_str) == Some("player") {
                        if let Some(mv) = kv.get("move") {
                            if matches!(
                                mv.as_str(),
                                "MOVE_DRAGON_DANCE"
                                    | "MOVE_NASTY_PLOT"
                                    | "MOVE_SWORDS_DANCE"
                                    | "MOVE_PROTECT"
                                    | "MOVE_TAUNT"
                                    | "MOVE_TRICK_ROOM"
                            ) {
                                b.key_moves.push(mv.clone());
                            }
                        }
                    }
                }
            }
            "switch" => {
                if let Some(b) = current.as_mut() {
                    match kv.get("side").map(String::as_str) {
                        Some("player") => b.switches_by_player += 1,
                        Some("opponent") => b.switches_by_opponent += 1,
                        _ => {}
                    }
                }
            }
            "item_used" => {
                if let Some(b) = current.as_mut() {
                    if kv.get("side").map(String::as_str) == Some("player") {
                        if let Some(item) = kv.get("item") {
                            b.items_used_by_player.push(item.clone());
                        }
                    }
                }
            }
            "faint" => {
                if let Some(b) = current.as_mut() {
                    let species = kv
                        .get("species")
                        .cloned()
                        .unwrap_or_else(|| "SPECIES_NONE".to_string());
                    match kv.get("side").map(String::as_str) {
                        Some("player") => b.player_faint_order.push(species),
                        Some("opponent") => b.opponent_faint_order.push(species),
                        _ => {}
                    }
                }
            }
            "battle_end" => {
                if let Some(b) = current.as_mut() {
                    b.outcome = kv
                        .get("outcome")
                        .cloned()
                        .unwrap_or_else(|| "incomplete".to_string());
                    if let Some(turns) = kv.get("turns").and_then(|v| v.parse().ok()) {
                        b.turns = turns;
                    }
                }
                if let Some(prev) = current.take() {
                    rows.push(prev);
                }
            }
            other => {
                errors.push(format!("line {}: unknown event '{other}'", lineno + 1));
            }
        }
    }
    if let Some(prev) = current.take() {
        rows.push(prev);
    }
    (rows, errors)
}

fn normalized_battle_to_jsonl(row: &NormalizedBattle) -> String {
    let mut w = JsonWriter::new();
    w.begin_object();
    w.kv_number("schemaVersion", row.schema_version as i64);
    w.kv_string("runId", &row.run_id);
    w.kv_number("battleId", row.battle_id);
    w.kv_string("trainerConst", &row.trainer_const);
    w.kv_string("format", &row.format);
    w.kv_string("outcome", &row.outcome);
    w.kv_number("turns", row.turns);
    w.key("playerParty");
    w.begin_array();
    for mon in &row.player_party {
        w.begin_object();
        w.kv_string("species", &mon.species);
        w.kv_number("level", mon.level);
        match &mon.item {
            Some(s) => w.kv_string("item", s),
            None => w.kv_null("item"),
        }
        w.key("moves");
        w.begin_array();
        for m in &mon.moves {
            w.raw_value(&format!("\"{}\"", escape_json(m)));
        }
        w.end_array();
        w.end_object();
    }
    w.end_array();
    write_string_array(&mut w, "playerFaintOrder", &row.player_faint_order);
    write_string_array(&mut w, "opponentFaintOrder", &row.opponent_faint_order);
    w.kv_number("switchesByPlayer", row.switches_by_player);
    w.kv_number("switchesByOpponent", row.switches_by_opponent);
    write_string_array(&mut w, "itemsUsedByPlayer", &row.items_used_by_player);
    write_string_array(&mut w, "keyMoves", &row.key_moves);
    write_string_array(&mut w, "warnings", &row.warnings);
    w.end_object();
    let out = w.into_string();
    out.replace('\n', "")
}

fn write_string_array(w: &mut JsonWriter, key: &str, items: &[String]) {
    w.key(key);
    w.begin_array();
    for s in items {
        w.raw_value(&format!("\"{}\"", escape_json(s)));
    }
    w.end_array();
}

fn parse_normalized_battle(json: &Json) -> Result<NormalizedBattle> {
    let obj = json.obj()?;
    Ok(NormalizedBattle {
        schema_version: obj.get_usize_default("schemaVersion", SCHEMA_VERSION as usize)? as u32,
        run_id: obj.get_string_default("runId", "run")?.to_string(),
        battle_id: obj.get_i64_default("battleId", 0)?,
        trainer_const: obj
            .get_string_default("trainerConst", "TRAINER_NONE")?
            .to_string(),
        format: obj.get_string_default("format", "single")?.to_string(),
        outcome: obj.get_string_default("outcome", "incomplete")?.to_string(),
        turns: obj.get_i64_default("turns", 0)?,
        player_party: Vec::new(),
        player_faint_order: obj.get_string_array_default("playerFaintOrder")?,
        opponent_faint_order: obj.get_string_array_default("opponentFaintOrder")?,
        switches_by_player: obj.get_i64_default("switchesByPlayer", 0)?,
        switches_by_opponent: obj.get_i64_default("switchesByOpponent", 0)?,
        items_used_by_player: obj.get_string_array_default("itemsUsedByPlayer")?,
        key_moves: obj.get_string_array_default("keyMoves")?,
        warnings: obj.get_string_array_default("warnings")?,
    })
}

// ---------- Profile aggregation ----------

#[derive(Default)]
struct Profile {
    schema_version: u32,
    catalog_version: Option<String>,
    generated_at: String,
    total_battles: usize,
    oldest_battle_at: String,
    newest_battle_at: String,
    win_rate: f64,
    archetype_win_rate: BTreeMap<String, f64>,
    repeated_weaknesses: Vec<String>,
    preferred_tempo: i64,
    switch_habit: f64,
    item_reliance: f64,
}

struct ProfileState {
    path: String,
    profile: Profile,
    enabled: bool,
    status: String,
    warnings: Vec<String>,
}

fn load_profile_state(
    opts: &Options,
    catalog: &Catalog,
    catalog_version: &str,
) -> Result<Option<ProfileState>> {
    let Some(profile_path) = &opts.profile else {
        return Ok(None);
    };
    let resolved = resolve_path(&opts.rom_repo, profile_path);
    let json = JsonParser::new(&read_to_string(&resolved)?)
        .parse()
        .map_err(|e| format!("{}: {e}", resolved.display()))?;
    let profile = parse_profile(&json)?;

    let mut warnings = Vec::new();
    let catalog_archetypes: BTreeSet<String> = catalog
        .sets
        .iter()
        .flat_map(|set| set.archetypes.iter().cloned())
        .collect();
    let unknown_weaknesses: Vec<String> = profile
        .repeated_weaknesses
        .iter()
        .filter(|tag| !catalog_archetypes.contains(tag.as_str()))
        .cloned()
        .collect();
    if !unknown_weaknesses.is_empty() {
        warnings.push(format!(
            "profile repeatedWeaknesses not present in catalog archetypes: {}",
            unknown_weaknesses.join(", ")
        ));
    }

    let (enabled, status) = if profile.total_battles < DEFAULT_MINIMUM_ADAPTATION_RUNS {
        (
            false,
            format!(
                "under_minimum_samples({}<{}); weights skipped",
                profile.total_battles, DEFAULT_MINIMUM_ADAPTATION_RUNS
            ),
        )
    } else {
        match &profile.catalog_version {
            Some(version) if version == catalog_version => (
                true,
                format!(
                    "enabled; repeatedWeaknesses use {:.2}x weight with {:.2} exploration",
                    DEFAULT_WEAKNESS_BONUS, DEFAULT_EXPLORATION_RATE
                ),
            ),
            Some(version) => (
                false,
                format!(
                    "catalog_version_mismatch(profile={}, catalog={}); weights skipped",
                    version, catalog_version
                ),
            ),
            None => (
                false,
                "catalog_version_missing; weights skipped".to_string(),
            ),
        }
    };

    Ok(Some(ProfileState {
        path: resolved.display().to_string(),
        profile,
        enabled,
        status,
        warnings,
    }))
}

fn aggregate_profile(battles: &[NormalizedBattle], catalog_version: Option<String>) -> Profile {
    let mut p = Profile {
        schema_version: SCHEMA_VERSION,
        catalog_version,
        generated_at: utc_compact_iso_now(),
        ..Default::default()
    };
    let completed: Vec<&NormalizedBattle> = battles
        .iter()
        .filter(|battle| battle.outcome != "incomplete")
        .collect();
    p.total_battles = completed.len();
    if completed.is_empty() {
        return p;
    }
    p.oldest_battle_at = completed.first().unwrap().run_id.clone();
    p.newest_battle_at = completed.last().unwrap().run_id.clone();
    let wins = completed.iter().filter(|b| b.outcome == "win").count();
    p.win_rate = wins as f64 / completed.len() as f64;
    let mut win_turns: Vec<i64> = battles
        .iter()
        .filter(|b| b.outcome == "win")
        .map(|b| b.turns)
        .collect();
    win_turns.sort();
    if !win_turns.is_empty() {
        p.preferred_tempo = win_turns[win_turns.len() / 2];
    }
    let mut switches: Vec<i64> = completed.iter().map(|b| b.switches_by_player).collect();
    switches.sort();
    if !switches.is_empty() {
        p.switch_habit = switches[switches.len() / 2] as f64;
    }
    let item_total: i64 = completed
        .iter()
        .map(|b| b.items_used_by_player.len() as i64)
        .sum();
    p.item_reliance = item_total as f64 / completed.len() as f64;
    let mut by_arch: BTreeMap<String, (usize, usize)> = BTreeMap::new();
    for b in completed {
        let key = b.trainer_const.clone();
        let entry = by_arch.entry(key).or_default();
        entry.0 += 1;
        if b.outcome == "win" {
            entry.1 += 1;
        }
    }
    for (k, (total, wins)) in &by_arch {
        let rate = if *total == 0 {
            0.0
        } else {
            *wins as f64 / *total as f64
        };
        p.archetype_win_rate.insert(k.clone(), rate);
        if *total >= 5 && rate < 0.5 {
            p.repeated_weaknesses.push(k.clone());
        }
    }
    p
}

fn utc_compact_iso_now() -> String {
    let now = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap_or_default();
    let secs = now.as_secs();
    let (y, mo, d, h, mi, s) = unix_to_ymdhms(secs);
    format!("{:04}-{:02}-{:02}T{:02}:{:02}:{:02}Z", y, mo, d, h, mi, s)
}

fn profile_to_json(p: &Profile) -> String {
    let mut w = JsonWriter::new();
    w.begin_object();
    w.kv_number("schemaVersion", p.schema_version as i64);
    match &p.catalog_version {
        Some(v) => w.kv_string("catalogVersion", v),
        None => w.kv_null("catalogVersion"),
    }
    w.kv_string("generatedAt", &p.generated_at);
    w.key("confidence");
    w.begin_object();
    w.kv_number("totalBattles", p.total_battles as i64);
    w.kv_string("oldestBattleAt", &p.oldest_battle_at);
    w.kv_string("newestBattleAt", &p.newest_battle_at);
    w.end_object();
    w.kv_float("winRate", p.win_rate);
    w.key("archetypeWinRate");
    w.begin_object();
    for (k, v) in &p.archetype_win_rate {
        w.kv_float(k, *v);
    }
    w.end_object();
    write_string_array(&mut w, "repeatedWeaknesses", &p.repeated_weaknesses);
    w.kv_number("preferredTempo", p.preferred_tempo);
    w.kv_float("switchHabit", p.switch_habit);
    w.kv_float("itemReliance", p.item_reliance);
    w.end_object();
    w.into_string()
}

fn parse_profile(json: &Json) -> Result<Profile> {
    let obj = json.obj()?;
    let confidence = obj
        .get_optional("confidence")
        .map(|v| v.obj())
        .transpose()?;
    let mut total = 0usize;
    let mut oldest = String::new();
    let mut newest = String::new();
    if let Some(c) = confidence {
        total = c.get_usize_default("totalBattles", 0)?;
        oldest = c.get_string_default("oldestBattleAt", "")?.to_string();
        newest = c.get_string_default("newestBattleAt", "")?.to_string();
    }
    let mut arch = BTreeMap::new();
    if let Some(Json::Object(map)) = obj.get_optional("archetypeWinRate") {
        for (k, v) in map {
            if let Some(f) = v.as_f64() {
                arch.insert(k.clone(), f);
            }
        }
    }
    Ok(Profile {
        schema_version: obj.get_usize_default("schemaVersion", SCHEMA_VERSION as usize)? as u32,
        catalog_version: obj.get_optional_string("catalogVersion")?,
        generated_at: obj.get_string_default("generatedAt", "")?.to_string(),
        total_battles: total,
        oldest_battle_at: oldest,
        newest_battle_at: newest,
        win_rate: obj.get_f64_default("winRate", 0.0)?,
        archetype_win_rate: arch,
        repeated_weaknesses: obj.get_string_array_default("repeatedWeaknesses")?,
        preferred_tempo: obj.get_i64_default("preferredTempo", 0)?,
        switch_habit: obj.get_f64_default("switchHabit", 0.0)?,
        item_reliance: obj.get_f64_default("itemReliance", 0.0)?,
    })
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn parses_json_object() {
        let json = JsonParser::new(r#"{"a": ["b", 2], "c": true}"#)
            .parse()
            .unwrap();
        assert_eq!(json.obj().unwrap().get_array("a").unwrap().len(), 2);
    }

    #[test]
    fn parses_json_floats() {
        let json = JsonParser::new(r#"{"x": 0.72, "y": -1.5e2}"#)
            .parse()
            .unwrap();
        let obj = json.obj().unwrap();
        assert!((obj.get_f64_default("x", 0.0).unwrap() - 0.72).abs() < 1e-9);
        assert!((obj.get_f64_default("y", 0.0).unwrap() + 150.0).abs() < 1e-9);
    }

    #[test]
    fn parses_party_blocks() {
        let blocks = parse_party_blocks(
            "=== TRAINER_A ===\nName: A\n\nSPECIES_GEODUDE\n\n=== TRAINER_B ===\nName: B\n",
        );
        assert_eq!(blocks.len(), 2);
        assert_eq!(blocks["TRAINER_A"].header_lines[0], "Name: A");
    }

    #[test]
    fn replaces_existing_block() {
        let source = "pre\n=== TRAINER_A ===\nName: A\n\nOld\n\n=== TRAINER_B ===\nName: B\n\n";
        let fragment = "=== TRAINER_A ===\nName: A\n\nNew\n";
        let out = apply_fragment(source, fragment).unwrap();
        assert!(out.contains("New"));
        assert!(!out.contains("Old"));
        assert!(out.contains("TRAINER_B"));
    }

    #[test]
    fn detects_double_battle_header() {
        let blocks = parse_party_blocks(
            "=== TRAINER_X ===\nName: X\nDouble Battle: Yes\n\nSPECIES_GEODUDE\n",
        );
        let pb = blocks.get("TRAINER_X").unwrap();
        assert_eq!(source_double_battle(pb), Some(true));
    }

    #[test]
    fn raw_log_parses_simple_session() {
        let text = "2026-05-06T10:00:00Z\tsession_start\tschemaVersion=1\n\
                    2026-05-06T10:00:05Z\tbattle_start\tbattleId=1\ttrainerConst=TRAINER_X\tformat=single\n\
                    2026-05-06T10:00:10Z\tturn_start\tturn=1\n\
                    2026-05-06T10:00:11Z\tmove_used\tside=player\tslot=0\tmove=MOVE_DRAGON_DANCE\n\
                    2026-05-06T10:00:12Z\tswitch\tside=player\tfrom=A\tto=B\n\
                    2026-05-06T10:00:23Z\tbattle_end\toutcome=win\tturns=4\n";
        let (rows, errs) = parse_raw_log(text, Path::new("test.log"));
        assert!(errs.is_empty(), "unexpected errors: {errs:?}");
        assert_eq!(rows.len(), 1);
        assert_eq!(rows[0].outcome, "win");
        assert_eq!(rows[0].turns, 4);
        assert_eq!(rows[0].switches_by_player, 1);
        assert_eq!(rows[0].key_moves, vec!["MOVE_DRAGON_DANCE".to_string()]);
    }

    #[test]
    fn aggregates_simple_profile() {
        let battles = vec![
            NormalizedBattle {
                schema_version: SCHEMA_VERSION,
                run_id: "r1".to_string(),
                battle_id: 1,
                trainer_const: "TRAINER_X".to_string(),
                format: "single".to_string(),
                outcome: "win".to_string(),
                turns: 8,
                player_party: vec![],
                player_faint_order: vec![],
                opponent_faint_order: vec![],
                switches_by_player: 1,
                switches_by_opponent: 0,
                items_used_by_player: vec!["ITEM_X".into()],
                key_moves: vec![],
                warnings: vec![],
            },
            NormalizedBattle {
                schema_version: SCHEMA_VERSION,
                run_id: "r1".to_string(),
                battle_id: 2,
                trainer_const: "TRAINER_X".to_string(),
                format: "single".to_string(),
                outcome: "loss".to_string(),
                turns: 5,
                player_party: vec![],
                player_faint_order: vec![],
                opponent_faint_order: vec![],
                switches_by_player: 2,
                switches_by_opponent: 0,
                items_used_by_player: vec![],
                key_moves: vec![],
                warnings: vec![],
            },
        ];
        let p = aggregate_profile(&battles, Some("fnv64:test".to_string()));
        assert_eq!(p.total_battles, 2);
        assert_eq!(p.catalog_version.as_deref(), Some("fnv64:test"));
        assert!((p.win_rate - 0.5).abs() < 1e-9);
        assert!(p.archetype_win_rate.contains_key("TRAINER_X"));
    }

    #[test]
    fn lint_detects_weather_pair_imbalance() {
        let blueprint = Blueprint {
            id: "b1".to_string(),
            party_size: 3,
            pool_size: 3,
            ruleset_id: "POOL_RULESET_BASIC".to_string(),
            required: vec![],
            preferred: vec![],
            constraints: Constraints::default(),
            mode: "single".to_string(),
            rank: "champion".to_string(),
            set_groups: vec![],
            require_spread_move: true,
            allow_species_duplicate: false,
        };
        let setter = Set {
            id: "s.setter".to_string(),
            species: "SPECIES_TYRANITAR".to_string(),
            ability: None,
            moves: vec![],
            item: None,
            ivs: "31/31/31/31/31/31".to_string(),
            evs: "0/0/0/0/0/0".to_string(),
            nature: None,
            level: 50,
            roles: vec![],
            archetypes: vec![],
            groups: vec![],
            lint_tags: vec![],
            tags: vec!["Weather Setter".to_string()],
            min_rank: "early".to_string(),
            max_rank: "champion".to_string(),
            doubles_spread_move: false,
        };
        let source = PartyBlock {
            name: "TRAINER_X".to_string(),
            text: "=== TRAINER_X ===\nName: X\n".to_string(),
            header_lines: vec!["Name: X".to_string()],
        };
        let cfg = LintConfig::default();
        let mut issues = Vec::new();
        run_trainer_lints(&blueprint, &source, &[&setter], &cfg, &mut issues);
        assert!(issues.iter().any(|i| i.id == "WTH001"));
    }

    #[test]
    fn lint_detects_battlefield_pair_imbalance() {
        let blueprint = Blueprint {
            id: "b1".to_string(),
            party_size: 3,
            pool_size: 3,
            ruleset_id: "POOL_RULESET_BASIC".to_string(),
            required: vec![],
            preferred: vec![],
            constraints: Constraints::default(),
            mode: "single".to_string(),
            rank: "champion".to_string(),
            set_groups: vec![],
            require_spread_move: true,
            allow_species_duplicate: false,
        };
        let setter = Set {
            id: "s.setter".to_string(),
            species: "SPECIES_PINCURCHIN".to_string(),
            ability: None,
            moves: vec![],
            item: None,
            ivs: "31/31/31/31/31/31".to_string(),
            evs: "0/0/0/0/0/0".to_string(),
            nature: None,
            level: 50,
            roles: vec![],
            archetypes: vec![],
            groups: vec![],
            lint_tags: vec!["Terrain Setter: Electric".to_string()],
            tags: vec![],
            min_rank: "early".to_string(),
            max_rank: "champion".to_string(),
            doubles_spread_move: false,
        };
        let source = PartyBlock {
            name: "TRAINER_X".to_string(),
            text: "=== TRAINER_X ===\nName: X\n".to_string(),
            header_lines: vec!["Name: X".to_string()],
        };
        let cfg = LintConfig {
            battlefield_pairs: vec![BattlefieldPair {
                id: "terrain.electric".to_string(),
                label: "electric terrain".to_string(),
                kind: "terrain".to_string(),
                setter_tags: vec!["Terrain Setter: Electric".to_string()],
                abuser_tags: vec!["Terrain Abuser: Electric".to_string()],
                setter_moves: vec![],
                setter_abilities: vec![],
                abuser_moves: vec![],
                abuser_abilities: vec![],
                detect_moves: false,
                detect_abilities: false,
            }],
            ..LintConfig::default()
        };
        let mut issues = Vec::new();
        run_trainer_lints(&blueprint, &source, &[&setter], &cfg, &mut issues);
        assert!(issues.iter().any(|i| i.id == "BFL001"));
    }

    #[test]
    fn set_groups_keep_blueprint_pools_separate() {
        let blueprint = Blueprint {
            id: "b.grouped".to_string(),
            party_size: 1,
            pool_size: 1,
            ruleset_id: "POOL_RULESET_BASIC".to_string(),
            required: vec![],
            preferred: vec![],
            constraints: Constraints::default(),
            mode: "single".to_string(),
            rank: "champion".to_string(),
            set_groups: vec!["pool.allowed".to_string()],
            require_spread_move: false,
            allow_species_duplicate: false,
        };
        let blocked = Set {
            id: "s.blocked".to_string(),
            species: "SPECIES_ABSOL".to_string(),
            ability: None,
            moves: vec![],
            item: None,
            ivs: "31/31/31/31/31/31".to_string(),
            evs: "0/0/0/0/0/0".to_string(),
            nature: None,
            level: 50,
            roles: vec![],
            archetypes: vec![],
            groups: vec!["pool.other".to_string()],
            lint_tags: vec![],
            tags: vec![],
            min_rank: "early".to_string(),
            max_rank: "champion".to_string(),
            doubles_spread_move: false,
        };
        let allowed = Set {
            id: "s.allowed".to_string(),
            species: "SPECIES_METANG".to_string(),
            ability: None,
            moves: vec![],
            item: None,
            ivs: "31/31/31/31/31/31".to_string(),
            evs: "0/0/0/0/0/0".to_string(),
            nature: None,
            level: 50,
            roles: vec![],
            archetypes: vec![],
            groups: vec!["pool.allowed".to_string()],
            lint_tags: vec![],
            tags: vec![],
            min_rank: "early".to_string(),
            max_rank: "champion".to_string(),
            doubles_spread_move: false,
        };
        let sets = vec![blocked, allowed];
        let mut rng = Rng::new(1);
        let selection = select_sets(&blueprint, &sets, None, &mut rng).unwrap();
        assert_eq!(selection.sets[0].id, "s.allowed");
    }

    #[test]
    fn lint_detects_doubles_mode_mismatch() {
        let blueprint = Blueprint {
            id: "b1".to_string(),
            party_size: 4,
            pool_size: 4,
            ruleset_id: "POOL_RULESET_DOUBLES".to_string(),
            required: vec![],
            preferred: vec![],
            constraints: Constraints::default(),
            mode: "double".to_string(),
            rank: "champion".to_string(),
            set_groups: vec![],
            require_spread_move: false,
            allow_species_duplicate: false,
        };
        let source = PartyBlock {
            name: "TRAINER_X".to_string(),
            text: "=== TRAINER_X ===\nName: X\nDouble Battle: No\n".to_string(),
            header_lines: vec!["Name: X".to_string(), "Double Battle: No".to_string()],
        };
        let cfg = LintConfig::default();
        let mut issues = Vec::new();
        run_trainer_lints(&blueprint, &source, &[], &cfg, &mut issues);
        assert!(issues.iter().any(|i| i.id == "DBL001"));
    }
}
