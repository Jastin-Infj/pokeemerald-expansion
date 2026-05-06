use std::collections::{BTreeMap, BTreeSet};
use std::env;
use std::fmt;
use std::fs;
use std::path::{Path, PathBuf};

type Result<T> = std::result::Result<T, String>;

const DEFAULT_CATALOG: &str = "tools/champions_partygen/catalog";
const DEFAULT_TRAINERS_PARTY: &str = "src/data/trainers.party";
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
    let opts = Options::parse(args)?;
    match cmd.as_str() {
        "doctor" => cmd_doctor(&opts),
        "scan" => cmd_scan(&opts),
        "generate" => cmd_generate(&opts),
        "render-one" => cmd_render_one(&opts),
        "explain" => cmd_explain(&opts),
        "validate" => cmd_validate(&opts),
        "diff" => cmd_diff(&opts),
        "apply" => cmd_apply(&opts),
        _ => Err(format!("unknown command '{cmd}'")),
    }
}

fn print_help() {
    println!(
        "champions_partygen 0.1.0\n\
         \n\
         Commands:\n\
           doctor      Check repo, catalog, and trainerproc inputs\n\
           scan        Print source trainer and script usage counts\n\
           generate    Render generated .party fragment\n\
           render-one  Render one trainer from the catalog\n\
           explain     Explain selected sets for one trainer\n\
           validate    Validate a generated .party fragment\n\
           diff        Report trainer blocks changed by a fragment\n\
           apply       Replace matching trainer blocks in a .party file\n\
         \n\
         Common options:\n\
           --rom-repo PATH       Repo root (default: current directory)\n\
           --catalog PATH        Catalog dir (default: tools/champions_partygen/catalog)\n\
           --seed N              Deterministic generation seed\n\
           --out PATH            Output file for generate/apply\n\
           --input PATH          Generated .party fragment\n\
           --against PATH        Baseline .party for diff\n\
           --target PATH         .party file to apply into\n\
           --trainer TRAINER_ID  Trainer const for render-one/explain"
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
                other => return Err(format!("unknown option '{other}'")),
            }
            i += 1;
        }

        opts.catalog = resolve_path(&opts.rom_repo, &opts.catalog);
        Ok(opts)
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

fn cmd_doctor(opts: &Options) -> Result<()> {
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
    let report = validate_catalog(&catalog, &trainers, &constants);
    if report.has_errors() {
        return Err(report.to_string());
    }

    println!("repo: {}", repo.display());
    println!("catalog: {}", opts.catalog.display());
    println!("journey trainers: {}", catalog.journey.len());
    println!("blueprints: {}", catalog.blueprints.len());
    println!("sets: {}", catalog.sets.len());
    println!("source trainer blocks: {}", trainers.len());
    println!("doctor: ok");
    Ok(())
}

fn cmd_scan(opts: &Options) -> Result<()> {
    let party = read_to_string(opts.rom_repo.join(DEFAULT_TRAINERS_PARTY))?;
    let trainers = parse_party_blocks(&party);
    let script_usage = scan_script_usage(&opts.rom_repo)?;
    println!("trainer_blocks,{}", trainers.len());
    println!("script_trainer_refs,{}", script_usage.len());
    for (trainer, count) in script_usage.iter().take(30) {
        println!("{trainer},{count}");
    }
    Ok(())
}

fn cmd_generate(opts: &Options) -> Result<()> {
    let output = render_catalog(opts, opts.trainer.as_deref())?;
    if let Some(out) = &opts.out {
        write_file(out, &output)?;
    } else {
        print!("{output}");
    }
    Ok(())
}

fn cmd_render_one(opts: &Options) -> Result<()> {
    let trainer = opts
        .trainer
        .as_deref()
        .ok_or_else(|| "render-one requires --trainer".to_string())?;
    let output = render_catalog(opts, Some(trainer))?;
    print!("{output}");
    Ok(())
}

fn cmd_explain(opts: &Options) -> Result<()> {
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
    let mut rng = Rng::new(seed_for(opts.seed, &req.trainer_const));
    let selected = select_sets(blueprint, &catalog.sets, &mut rng)?;
    println!("trainer: {}", req.trainer_const);
    println!("blueprint: {}", req.blueprint_id);
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
    for set in selected {
        println!(
            "{} -> {} [{}] roles={} archetypes={}",
            set.id,
            set.species,
            set.tags.join(" / "),
            set.roles.join(" / "),
            set.archetypes.join(" / ")
        );
    }
    Ok(())
}

fn cmd_validate(opts: &Options) -> Result<()> {
    let input = opts
        .input
        .as_ref()
        .ok_or_else(|| "validate requires --input".to_string())?;
    let fragment = read_to_string(resolve_path(&opts.rom_repo, input))?;
    let constants = ConstantIndex::load(&opts.rom_repo)?;
    let report = validate_party_fragment(&fragment, &constants);
    if report.has_errors() {
        Err(report.to_string())
    } else {
        println!("validate: ok");
        Ok(())
    }
}

fn cmd_diff(opts: &Options) -> Result<()> {
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
    Ok(())
}

fn cmd_apply(opts: &Options) -> Result<()> {
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
    let target_path = resolve_path(&opts.rom_repo, &target);
    let source = read_to_string(&target_path)?;
    let result = apply_fragment(&source, &fragment)?;
    write_file(out, &result)?;
    println!("apply: wrote {}", out.display());
    Ok(())
}

fn render_catalog(opts: &Options, only_trainer: Option<&str>) -> Result<String> {
    let catalog = Catalog::load(&opts.catalog)?;
    let source_trainers =
        parse_party_blocks(&read_to_string(opts.rom_repo.join(DEFAULT_TRAINERS_PARTY))?);
    let constants = ConstantIndex::load(&opts.rom_repo)?;
    let report = validate_catalog(&catalog, &source_trainers, &constants);
    if report.has_errors() {
        return Err(report.to_string());
    }

    let mut out = String::new();
    out.push_str("/*\n");
    out.push_str("Generated by tools/champions_partygen.\n");
    out.push_str("Review with `partygen diff` before applying to src/data/trainers.party.\n");
    out.push_str("*/\n\n");

    for req in &catalog.journey {
        if let Some(trainer) = only_trainer {
            if trainer != req.trainer_const {
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
        let selected = select_sets(blueprint, &catalog.sets, &mut rng)?;
        render_trainer_block(&mut out, source, blueprint, &selected);
        out.push('\n');
    }

    if only_trainer.is_some() && out.matches("\n=== ").count() == 0 {
        return Err(format!(
            "{} is not in journey.json",
            only_trainer.unwrap_or_default()
        ));
    }
    Ok(out)
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

fn select_sets<'a>(blueprint: &Blueprint, sets: &'a [Set], rng: &mut Rng) -> Result<Vec<&'a Set>> {
    if blueprint.pool_size < blueprint.party_size {
        return Err(format!(
            "{} poolSize {} is smaller than partySize {}",
            blueprint.id, blueprint.pool_size, blueprint.party_size
        ));
    }
    let mut selected: Vec<&Set> = Vec::new();
    let mut used = BTreeSet::new();

    for req in &blueprint.required {
        let candidates: Vec<&Set> = sets
            .iter()
            .filter(|set| !used.contains(&set.id))
            .filter(|set| req.roles.iter().any(|role| set.roles.contains(role)))
            .filter(|set| match req.slot.as_str() {
                "lead" => set.tags.iter().any(|tag| tag == "Lead"),
                "ace" => set.tags.iter().any(|tag| tag == "Ace"),
                _ => true,
            })
            .collect();
        let picked = pick(candidates, rng).ok_or_else(|| {
            format!(
                "{} has no candidate for required slot '{}' roles {:?}",
                blueprint.id, req.slot, req.roles
            )
        })?;
        used.insert(picked.id.clone());
        selected.push(picked);
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
                .filter(|set| set.roles.contains(&pref.role))
                .collect();
            let Some(picked) = pick(candidates, rng) else {
                return Err(format!(
                    "{} has no candidate for preferred role {}",
                    blueprint.id, pref.role
                ));
            };
            used.insert(picked.id.clone());
            selected.push(picked);
        }
    }

    while selected.len() < blueprint.pool_size {
        let candidates: Vec<&Set> = sets.iter().filter(|set| !used.contains(&set.id)).collect();
        let Some(picked) = pick(candidates, rng) else {
            return Err(format!(
                "{} needs {} pool members but only {} unique sets are available",
                blueprint.id,
                blueprint.pool_size,
                selected.len()
            ));
        };
        used.insert(picked.id.clone());
        selected.push(picked);
    }

    Ok(selected)
}

fn pick<'a>(mut candidates: Vec<&'a Set>, rng: &mut Rng) -> Option<&'a Set> {
    candidates.sort_by(|a, b| a.id.cmp(&b.id));
    if candidates.is_empty() {
        None
    } else {
        let index = rng.next_usize(candidates.len());
        Some(candidates[index])
    }
}

fn validate_catalog(
    catalog: &Catalog,
    source_trainers: &BTreeMap<String, PartyBlock>,
    constants: &ConstantIndex,
) -> Report {
    let mut report = Report::default();
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
        constants.check("species", &set.species, &mut report);
        if let Some(ability) = &set.ability {
            constants.check("ability", ability, &mut report);
        }
        if let Some(item) = &set.item {
            constants.check("item", item, &mut report);
        }
        if let Some(nature) = &set.nature {
            constants.check("nature", nature, &mut report);
        }
        for mv in &set.moves {
            constants.check("move", mv, &mut report);
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
    }
    report
}

fn validate_party_fragment(fragment: &str, constants: &ConstantIndex) -> Report {
    let mut report = Report::default();
    let blocks = parse_party_blocks(fragment);
    if blocks.is_empty() {
        report.error("fragment contains no trainer blocks".to_string());
    }
    for (trainer, block) in blocks {
        constants.check("trainer", &trainer, &mut report);
        let mut party_size = None;
        let mut mon_count = 0usize;
        let mut current_moves = 0usize;
        for line in block.text.lines() {
            let trimmed = line.trim();
            if let Some(value) = trimmed.strip_prefix("Party Size:") {
                match value.trim().parse::<usize>() {
                    Ok(size) => party_size = Some(size),
                    Err(_) => report.error(format!("{trainer}: invalid Party Size")),
                }
            } else if let Some(tags) = trimmed.strip_prefix("Tags:") {
                for tag in tags.split('/') {
                    let tag = tag.trim();
                    if !tag.is_empty() && !VALID_POOL_TAGS.contains(&tag) {
                        report.error(format!("{trainer}: invalid tag '{tag}'"));
                    }
                }
            } else if trimmed.starts_with("- ") {
                current_moves += 1;
                constants.check("move", trimmed.trim_start_matches("- ").trim(), &mut report);
                if current_moves > 4 {
                    report.error(format!("{trainer}: mon has more than four moves"));
                }
            } else if is_mon_start(trimmed) {
                mon_count += 1;
                current_moves = 0;
                if let Some(species) = trimmed.split('@').next().map(str::trim) {
                    if species.starts_with("SPECIES_") {
                        constants.check("species", species, &mut report);
                    }
                }
                if let Some(item) = trimmed.split('@').nth(1).map(str::trim) {
                    constants.check("item", item, &mut report);
                }
            } else if let Some(value) = trimmed.strip_prefix("Ability:") {
                let ability = value.trim();
                if ability.starts_with("ABILITY_") {
                    constants.check("ability", ability, &mut report);
                }
            }
        }
        if let Some(size) = party_size {
            if size == 0 || size > 6 {
                report.error(format!("{trainer}: Party Size must be 1..6"));
            }
            if mon_count < size {
                report.error(format!(
                    "{trainer}: Party Size {size} exceeds mon count {mon_count}"
                ));
            }
            if mon_count > 255 {
                report.error(format!("{trainer}: pool exceeds 255 mons"));
            }
        }
    }
    report
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
}

impl fmt::Display for Report {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        for err in &self.errors {
            writeln!(f, "{err}")?;
        }
        Ok(())
    }
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
    tags: Vec<String>,
}

struct Blueprint {
    id: String,
    party_size: usize,
    pool_size: usize,
    ruleset_id: String,
    required: Vec<RequiredSlot>,
    preferred: Vec<PreferredRole>,
    constraints: Constraints,
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
}

struct Catalog {
    journey: Vec<JourneyTrainer>,
    blueprints: BTreeMap<String, Blueprint>,
    sets: Vec<Set>,
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
        Ok(Self {
            journey,
            blueprints,
            sets,
        })
    }
}

fn list_json_files(path: &Path) -> Result<Vec<PathBuf>> {
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
    for stage in json.obj()?.get_array("stages")? {
        for trainer in stage.obj()?.get_array("trainers")? {
            let obj = trainer.obj()?;
            result.push(JourneyTrainer {
                trainer_const: obj.get_string("trainerConst")?.to_string(),
                blueprint_id: obj.get_string("blueprintId")?.to_string(),
            });
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
                tags: obj.get_string_array_default("tags")?,
            })
        })
        .collect()
}

#[derive(Debug, Clone)]
enum Json {
    Null,
    Bool(()),
    Number(i64),
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
            Some(b'-') | Some(b'0'..=b'9') => self.parse_number().map(Json::Number),
            Some(b't') => {
                self.expect_bytes(b"true")?;
                Ok(Json::Bool(()))
            }
            Some(b'f') => {
                self.expect_bytes(b"false")?;
                Ok(Json::Bool(()))
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

    fn parse_number(&mut self) -> Result<i64> {
        let start = self.pos;
        self.consume(b'-');
        while matches!(self.peek(), Some(b'0'..=b'9')) {
            self.pos += 1;
        }
        std::str::from_utf8(&self.input[start..self.pos])
            .map_err(|e| e.to_string())?
            .parse()
            .map_err(|_| format!("invalid number at {start}"))
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
}
