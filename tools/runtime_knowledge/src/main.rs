use std::collections::{BTreeMap, BTreeSet, HashMap};
use std::env;
use std::fs;
use std::io;
use std::path::{Path, PathBuf};

const MOVE_BOOL_TO_FLAG: &[(&str, &str)] = &[
    ("makesContact", "AI_MOVE_KNOWLEDGE_CONTACT"),
    ("soundMove", "AI_MOVE_KNOWLEDGE_SOUND"),
    ("ballisticMove", "AI_MOVE_KNOWLEDGE_BALLISTIC"),
    ("powderMove", "AI_MOVE_KNOWLEDGE_POWDER"),
    ("slicingMove", "AI_MOVE_KNOWLEDGE_SLICING"),
    ("punchingMove", "AI_MOVE_KNOWLEDGE_PUNCHING"),
    ("bitingMove", "AI_MOVE_KNOWLEDGE_BITING"),
    ("pulseMove", "AI_MOVE_KNOWLEDGE_PULSE"),
    ("danceMove", "AI_MOVE_KNOWLEDGE_DANCE"),
    ("windMove", "AI_MOVE_KNOWLEDGE_WIND"),
    ("healingMove", "AI_MOVE_KNOWLEDGE_HEALING"),
    ("magicCoatAffected", "AI_MOVE_KNOWLEDGE_MAGIC_COAT"),
    ("snatchAffected", "AI_MOVE_KNOWLEDGE_SNATCH"),
];

const MOVE_EXTRA_FIELDS: &[&str] = &[
    "ignoresProtect",
    "ignoresSubstitute",
    "metronomeBanned",
    "mirrorMoveBanned",
    "sketchBanned",
    "assistBanned",
    "copycatBanned",
    "sleepTalkBanned",
    "encoreBanned",
    "multiHit",
    "twoTurnAttack",
];

const MOVE_CORE_FIELDS: &[&str] = &[
    "effect", "power", "type", "accuracy", "pp", "target", "priority", "category",
];

const MOVE_ABILITY_CONTROL_EFFECTS: &[&str] = &[
    "EFFECT_GASTRO_ACID",
    "EFFECT_OVERWRITE_ABILITY",
    "EFFECT_ROLE_PLAY",
    "EFFECT_SKILL_SWAP",
    "EFFECT_ENTRAINMENT",
    "EFFECT_DOODLE",
];

const MOVE_DENIAL_EFFECTS: &[&str] = &[
    "EFFECT_ATTRACT",
    "EFFECT_DISABLE",
    "EFFECT_ENCORE",
    "EFFECT_HEAL_BLOCK",
    "EFFECT_IMPRISON",
    "EFFECT_TAUNT",
    "EFFECT_TORMENT",
];

const MOVE_COMBO_STATE_EFFECTS: &[&str] = &[
    "EFFECT_BIDE",
    "EFFECT_CHARGE",
    "EFFECT_DEFENSE_CURL",
    "EFFECT_FOCUS_ENERGY",
    "EFFECT_LASER_FOCUS",
    "EFFECT_LUCKY_CHANT",
    "EFFECT_MINIMIZE",
    "EFFECT_ROLLOUT",
    "EFFECT_STOCKPILE",
];

const SOURCE_FILES: &[&str] = &[
    "include/battle_ai_util.h",
    "include/constants/abilities.h",
    "include/constants/hold_effects.h",
    "src/battle_ai_util.c",
    "src/data/items.h",
    "src/data/moves_info.h",
    "src/item.c",
];

#[derive(Clone)]
struct FlagDefine {
    tag: String,
}

#[derive(Clone)]
struct Constant {
    id: String,
    ordinal: usize,
    value: usize,
    raw_value: Option<String>,
}

struct MoveEntry {
    id: String,
    name: Option<String>,
    fields: BTreeMap<String, String>,
    runtime_flags: BTreeMap<String, String>,
    conditional_runtime_flags: BTreeMap<String, String>,
    ai_knowledge_flags: Vec<String>,
    tags: Vec<String>,
}

struct KnowledgeConstant {
    constant: Constant,
    ai_knowledge_flags: Vec<String>,
    tags: Vec<String>,
}

struct ItemEntry {
    id: String,
    name: Option<String>,
    hold_effect: String,
    hold_effect_param: Option<String>,
    ai_knowledge_flags: Vec<String>,
    tags: Vec<String>,
}

struct GimmickPolicy {
    id: &'static str,
    default_priority: usize,
    resource_policy: &'static str,
    reason_tags: &'static [&'static str],
}

struct Catalogs {
    moves: Vec<MoveEntry>,
    abilities: Vec<KnowledgeConstant>,
    hold_effects: Vec<KnowledgeConstant>,
    items: Vec<ItemEntry>,
    gimmicks: Vec<GimmickPolicy>,
    summary: Summary,
}

struct Summary {
    catalog_version: usize,
    counts: BTreeMap<String, usize>,
    flag_counts: BTreeMap<String, usize>,
    source_files: Vec<String>,
}

struct Args {
    root: PathBuf,
    out: PathBuf,
    strict: bool,
}

fn main() {
    if let Err(err) = run() {
        eprintln!("runtime_knowledge: {err}");
        std::process::exit(1);
    }
}

fn run() -> Result<(), Box<dyn std::error::Error>> {
    let args = parse_args()?;
    let catalogs = build_catalog(&args.root)?;
    if args.strict {
        require_catalogs(&catalogs)?;
    }

    fs::create_dir_all(&args.out)?;
    write_json(args.out.join("moves.json"), json_moves(&catalogs.moves))?;
    write_json(
        args.out.join("abilities.json"),
        json_knowledge_constants(&catalogs.abilities),
    )?;
    write_json(
        args.out.join("hold_effects.json"),
        json_knowledge_constants(&catalogs.hold_effects),
    )?;
    write_json(args.out.join("items.json"), json_items(&catalogs.items))?;
    write_json(
        args.out.join("gimmicks.json"),
        json_gimmicks(&catalogs.gimmicks),
    )?;
    write_json(
        args.out.join("summary.json"),
        json_summary(&catalogs.summary),
    )?;
    Ok(())
}

fn parse_args() -> Result<Args, String> {
    let mut root = env::current_dir().map_err(|err| err.to_string())?;
    let mut out = None;
    let mut strict = true;
    let mut iter = env::args().skip(1);

    while let Some(arg) = iter.next() {
        match arg.as_str() {
            "--root" => {
                let value = iter.next().ok_or("--root requires a path")?;
                root = PathBuf::from(value);
            }
            "--out" => {
                let value = iter.next().ok_or("--out requires a path")?;
                out = Some(PathBuf::from(value));
            }
            "--pretty" => {}
            "--no-strict" => strict = false,
            "-h" | "--help" => {
                print_help();
                std::process::exit(0);
            }
            _ => return Err(format!("unknown argument: {arg}")),
        }
    }

    Ok(Args {
        root: root
            .canonicalize()
            .map_err(|err| format!("invalid --root: {err}"))?,
        out: out.ok_or("--out is required")?,
        strict,
    })
}

fn print_help() {
    println!("Build Smart Gimmick AI runtime knowledge catalogs.");
    println!();
    println!("Usage:");
    println!("  runtime_knowledge --out /tmp/runtime_knowledge_catalog [--root PATH] [--pretty] [--no-strict]");
}

fn build_catalog(root: &Path) -> Result<Catalogs, Box<dyn std::error::Error>> {
    let header_text = read_text(root, "include/battle_ai_util.h")?;
    let flag_defines = parse_flag_defines(&header_text);
    let (hold_effects, hold_effect_flags) = parse_hold_effects(root, &flag_defines)?;
    let moves = parse_moves(root, &flag_defines)?;
    let abilities = parse_abilities(root, &flag_defines)?;
    let items = parse_items(root, &hold_effect_flags, &flag_defines)?;
    let gimmicks = gimmick_policies();

    let mut counts = BTreeMap::new();
    counts.insert("abilities".to_string(), abilities.len());
    counts.insert("gimmicks".to_string(), gimmicks.len());
    counts.insert("hold_effects".to_string(), hold_effects.len());
    counts.insert("items".to_string(), items.len());
    counts.insert("moves".to_string(), moves.len());

    let mut flag_counts = BTreeMap::new();
    flag_counts.insert(
        "ability".to_string(),
        flag_defines
            .keys()
            .filter(|name| name.starts_with("AI_ABILITY_KNOWLEDGE_") && !name.ends_with("_NONE"))
            .count(),
    );
    flag_counts.insert(
        "hold_effect".to_string(),
        flag_defines
            .keys()
            .filter(|name| {
                name.starts_with("AI_HOLD_EFFECT_KNOWLEDGE_") && !name.ends_with("_NONE")
            })
            .count(),
    );
    flag_counts.insert(
        "move".to_string(),
        flag_defines
            .keys()
            .filter(|name| name.starts_with("AI_MOVE_KNOWLEDGE_") && !name.ends_with("_NONE"))
            .count(),
    );

    Ok(Catalogs {
        moves,
        abilities,
        hold_effects,
        items,
        gimmicks,
        summary: Summary {
            catalog_version: 1,
            counts,
            flag_counts,
            source_files: SOURCE_FILES
                .iter()
                .map(|path| (*path).to_string())
                .collect(),
        },
    })
}

fn read_text(root: &Path, rel_path: &str) -> io::Result<String> {
    fs::read_to_string(root.join(rel_path))
}

fn strip_c_comments(text: &str) -> String {
    let bytes = text.as_bytes();
    let mut out = String::with_capacity(text.len());
    let mut i = 0;
    while i < bytes.len() {
        if i + 1 < bytes.len() && bytes[i] == b'/' && bytes[i + 1] == b'/' {
            while i < bytes.len() && bytes[i] != b'\n' {
                i += 1;
            }
            if i < bytes.len() {
                out.push('\n');
                i += 1;
            }
        } else if i + 1 < bytes.len() && bytes[i] == b'/' && bytes[i + 1] == b'*' {
            i += 2;
            while i + 1 < bytes.len() && !(bytes[i] == b'*' && bytes[i + 1] == b'/') {
                if bytes[i] == b'\n' {
                    out.push('\n');
                }
                i += 1;
            }
            i = (i + 2).min(bytes.len());
        } else {
            out.push(bytes[i] as char);
            i += 1;
        }
    }
    out
}

fn clean_expr(expr: &str) -> String {
    expr.trim()
        .trim_end_matches(',')
        .split_whitespace()
        .collect::<Vec<_>>()
        .join(" ")
}

fn is_truthy_expr(expr: Option<&str>) -> bool {
    match expr.map(clean_expr) {
        Some(value) => value != "0" && value != "FALSE" && value != "false",
        None => false,
    }
}

fn flag_tag(flag_name: &str) -> String {
    if flag_name.ends_with("_NONE") {
        return "none".to_string();
    }
    flag_name
        .split_once("_KNOWLEDGE_")
        .map(|(_, tag)| tag.to_ascii_lowercase())
        .unwrap_or_else(|| flag_name.to_ascii_lowercase())
}

fn parse_flag_defines(text: &str) -> HashMap<String, FlagDefine> {
    let mut flags = HashMap::new();
    for line in text.lines() {
        let trimmed = line.trim();
        if !trimmed.starts_with("#define AI_") || !trimmed.contains("_KNOWLEDGE_") {
            continue;
        }
        let mut parts = trimmed.split_whitespace();
        let _define = parts.next();
        let Some(name) = parts.next() else {
            continue;
        };
        flags.insert(
            name.to_string(),
            FlagDefine {
                tag: flag_tag(name),
            },
        );
    }
    flags
}

fn parse_enum_constants(text: &str, prefix: &str) -> Vec<Constant> {
    let stripped = strip_c_comments(text);
    let body = enum_body_for_prefix(&stripped, prefix);
    let mut constants = Vec::new();
    let mut next_value = 0usize;

    for line in body.lines() {
        let cleaned = clean_expr(line);
        if cleaned.is_empty() || !cleaned.starts_with(&format!("{prefix}_")) {
            continue;
        }
        let without_comma = cleaned.trim_end_matches(',');
        let (id, raw_value) = match without_comma.split_once('=') {
            Some((left, right)) => (left.trim().to_string(), Some(clean_expr(right))),
            None => {
                let token = without_comma
                    .split(|ch: char| !(ch.is_ascii_alphanumeric() || ch == '_'))
                    .next()
                    .unwrap_or("")
                    .to_string();
                (token, None)
            }
        };
        if id.is_empty() || id.ends_with("_COUNT") {
            continue;
        }
        let numeric_value = raw_value
            .as_ref()
            .and_then(|value| value.parse::<usize>().ok());
        if let Some(value) = numeric_value {
            next_value = value;
        }
        constants.push(Constant {
            id,
            ordinal: constants.len(),
            value: numeric_value.unwrap_or(next_value),
            raw_value,
        });
        next_value += 1;
    }

    constants
}

fn enum_body_for_prefix<'a>(text: &'a str, prefix: &str) -> &'a str {
    let Some(first_constant) = text.find(&format!("{prefix}_")) else {
        return text;
    };
    let Some(brace_start) = text[..first_constant].rfind('{') else {
        return text;
    };
    find_balanced_body(text, brace_start).unwrap_or(text)
}

fn find_balanced_body(text: &str, brace_start: usize) -> Option<&str> {
    let mut depth = 0usize;
    for (pos, byte) in text.bytes().enumerate().skip(brace_start) {
        match byte {
            b'{' => depth += 1,
            b'}' => {
                depth = depth.saturating_sub(1);
                if depth == 0 {
                    return Some(&text[brace_start + 1..pos]);
                }
            }
            _ => {}
        }
    }
    None
}

fn collect_indexed_blocks(text: &str, prefix: &str) -> Vec<(String, String)> {
    let mut starts: Vec<(usize, String)> = Vec::new();
    let needle = format!("[{prefix}_");
    let mut pos = 0usize;
    for line in text.split_inclusive('\n') {
        let trimmed = line.trim_start();
        if trimmed.starts_with(&needle) {
            if let Some(end) = trimmed.find(']') {
                let id = trimmed[1..end].to_string();
                starts.push((pos, id));
            }
        }
        pos += line.len();
    }

    let mut blocks = Vec::new();
    for index in 0..starts.len() {
        let start = starts[index].0;
        let end = starts
            .get(index + 1)
            .map(|(pos, _)| *pos)
            .unwrap_or(text.len());
        blocks.push((starts[index].1.clone(), text[start..end].to_string()));
    }
    blocks
}

fn parse_assignment(block: &str, field: &str) -> Option<String> {
    let needle = format!(".{field}");
    for line in block.lines() {
        let trimmed = line.trim();
        if !trimmed.starts_with(&needle) {
            continue;
        }
        let Some((_, value)) = trimmed.split_once('=') else {
            continue;
        };
        return Some(clean_expr(value));
    }
    None
}

fn parse_compound_string(block: &str, field: &str) -> Option<String> {
    let needle = format!(".{field}");
    let start = block.find(&needle)?;
    let after_field = &block[start..];
    let quote_start = after_field.find('"')?;
    let mut out = String::new();
    let mut escaped = false;
    for ch in after_field[quote_start + 1..].chars() {
        if escaped {
            out.push(ch);
            escaped = false;
        } else if ch == '\\' {
            escaped = true;
        } else if ch == '"' {
            return Some(out);
        } else {
            out.push(ch);
        }
    }
    None
}

fn extract_function_body<'a>(text: &'a str, function_name: &str) -> &'a str {
    let Some(start) = text.find(&format!("{function_name}(")) else {
        return "";
    };
    let Some(brace_offset) = text[start..].find('{') else {
        return "";
    };
    let brace_start = start + brace_offset;
    find_balanced_body(text, brace_start).unwrap_or("")
}

fn parse_switch_flag_map(
    function_body: &str,
    subject_prefix: &str,
    flag_prefix: &str,
) -> HashMap<String, Vec<String>> {
    let body = strip_c_comments(function_body);
    let mut mapping: HashMap<String, BTreeSet<String>> = HashMap::new();
    let mut pending_subjects: Vec<String> = Vec::new();

    for line in body.lines() {
        let trimmed = line.trim();
        if let Some(subject) = parse_case_subject(trimmed, subject_prefix) {
            pending_subjects.push(subject);
            continue;
        }
        if trimmed.starts_with("default:") {
            pending_subjects.clear();
            continue;
        }
        if let Some(flag) = parse_flag_assignment(trimmed, flag_prefix) {
            if !pending_subjects.is_empty() {
                for subject in &pending_subjects {
                    mapping
                        .entry(subject.clone())
                        .or_default()
                        .insert(flag.clone());
                }
            }
            continue;
        }
        if trimmed == "break;" {
            pending_subjects.clear();
        }
    }

    mapping
        .into_iter()
        .map(|(key, values)| (key, values.into_iter().collect()))
        .collect()
}

fn parse_case_subject(line: &str, subject_prefix: &str) -> Option<String> {
    let rest = line.strip_prefix("case ")?;
    let token = rest
        .split(|ch: char| !(ch.is_ascii_alphanumeric() || ch == '_'))
        .next()?;
    if token.starts_with(&format!("{subject_prefix}_")) {
        Some(token.to_string())
    } else {
        None
    }
}

fn parse_flag_assignment(line: &str, flag_prefix: &str) -> Option<String> {
    let (_, rest) = line.split_once("flags |=")?;
    let token = rest
        .trim()
        .split(|ch: char| !(ch.is_ascii_alphanumeric() || ch == '_'))
        .next()?;
    if token.starts_with(&format!("{flag_prefix}_")) {
        Some(token.to_string())
    } else {
        None
    }
}

fn parse_choice_hold_effects(text: &str) -> BTreeSet<String> {
    let body = extract_function_body(text, "IsHoldEffectChoice");
    extract_tokens_with_prefix(body, "HOLD_EFFECT")
}

fn extract_tokens_with_prefix(text: &str, prefix: &str) -> BTreeSet<String> {
    let token_prefix = format!("{prefix}_");
    let mut tokens = BTreeSet::new();
    for token in text.split(|ch: char| !(ch.is_ascii_alphanumeric() || ch == '_')) {
        if token.starts_with(&token_prefix) {
            tokens.insert(token.to_string());
        }
    }
    tokens
}

fn parse_moves(
    root: &Path,
    flag_defines: &HashMap<String, FlagDefine>,
) -> io::Result<Vec<MoveEntry>> {
    let moves_text = read_text(root, "src/data/moves_info.h")?;
    let ability_control: BTreeSet<&str> = MOVE_ABILITY_CONTROL_EFFECTS.iter().copied().collect();
    let denial: BTreeSet<&str> = MOVE_DENIAL_EFFECTS.iter().copied().collect();
    let combo_state: BTreeSet<&str> = MOVE_COMBO_STATE_EFFECTS.iter().copied().collect();
    let mut moves = Vec::new();

    for (move_id, block) in collect_indexed_blocks(&moves_text, "MOVE") {
        let mut fields = BTreeMap::new();
        let mut runtime_flags = BTreeMap::new();
        let mut conditional_runtime_flags = BTreeMap::new();
        let mut knowledge_flags = BTreeSet::new();

        for field in MOVE_CORE_FIELDS {
            if let Some(value) = parse_assignment(&block, field) {
                fields.insert((*field).to_string(), value);
            }
        }

        for (field, ai_flag) in MOVE_BOOL_TO_FLAG {
            let expr = parse_assignment(&block, field);
            if is_truthy_expr(expr.as_deref()) {
                let value = expr.unwrap();
                runtime_flags.insert((*field).to_string(), value.clone());
                knowledge_flags.insert((*ai_flag).to_string());
                if value != "TRUE" {
                    conditional_runtime_flags.insert((*field).to_string(), value);
                }
            }
        }

        for field in MOVE_EXTRA_FIELDS {
            let expr = parse_assignment(&block, field);
            if is_truthy_expr(expr.as_deref()) {
                let value = expr.unwrap();
                runtime_flags.insert((*field).to_string(), value.clone());
                if value != "TRUE" {
                    conditional_runtime_flags.insert((*field).to_string(), value);
                }
            }
        }

        if let Some(effect) = fields.get("effect") {
            if effect == "EFFECT_MAGIC_COAT" {
                knowledge_flags.insert("AI_MOVE_KNOWLEDGE_MAGIC_COAT".to_string());
            }
            if effect == "EFFECT_SNATCH" {
                knowledge_flags.insert("AI_MOVE_KNOWLEDGE_SNATCH".to_string());
            }
            if ability_control.contains(effect.as_str()) {
                knowledge_flags.insert("AI_MOVE_KNOWLEDGE_ABILITY_CONTROL".to_string());
            }
            if denial.contains(effect.as_str()) {
                knowledge_flags.insert("AI_MOVE_KNOWLEDGE_MOVE_DENIAL".to_string());
            }
            if combo_state.contains(effect.as_str()) {
                knowledge_flags.insert("AI_MOVE_KNOWLEDGE_COMBO_STATE".to_string());
            }
        }

        let ai_knowledge_flags: Vec<String> = knowledge_flags.into_iter().collect();
        let tags = tags_for(&ai_knowledge_flags, flag_defines);
        moves.push(MoveEntry {
            id: move_id,
            name: parse_compound_string(&block, "name"),
            fields,
            runtime_flags,
            conditional_runtime_flags,
            ai_knowledge_flags,
            tags,
        });
    }

    Ok(moves)
}

fn parse_abilities(
    root: &Path,
    flag_defines: &HashMap<String, FlagDefine>,
) -> io::Result<Vec<KnowledgeConstant>> {
    let constants = parse_enum_constants(
        &read_text(root, "include/constants/abilities.h")?,
        "ABILITY",
    );
    let ai_text = read_text(root, "src/battle_ai_util.c")?;
    let flags_by_ability = parse_switch_flag_map(
        extract_function_body(&ai_text, "AI_GetAbilityKnowledgeFlags"),
        "ABILITY",
        "AI_ABILITY_KNOWLEDGE",
    );
    Ok(constants
        .into_iter()
        .map(|constant| {
            let ai_knowledge_flags = flags_by_ability
                .get(&constant.id)
                .cloned()
                .unwrap_or_default();
            let tags = tags_for(&ai_knowledge_flags, flag_defines);
            KnowledgeConstant {
                constant,
                ai_knowledge_flags,
                tags,
            }
        })
        .collect())
}

fn parse_hold_effects(
    root: &Path,
    flag_defines: &HashMap<String, FlagDefine>,
) -> io::Result<(Vec<KnowledgeConstant>, HashMap<String, Vec<String>>)> {
    let constants = parse_enum_constants(
        &read_text(root, "include/constants/hold_effects.h")?,
        "HOLD_EFFECT",
    );
    let ai_text = read_text(root, "src/battle_ai_util.c")?;
    let item_text = read_text(root, "src/item.c")?;
    let mut flag_sets: HashMap<String, BTreeSet<String>> = parse_switch_flag_map(
        extract_function_body(&ai_text, "AI_GetHoldEffectKnowledgeFlags"),
        "HOLD_EFFECT",
        "AI_HOLD_EFFECT_KNOWLEDGE",
    )
    .into_iter()
    .map(|(key, values)| (key, values.into_iter().collect()))
    .collect();

    for hold_effect in parse_choice_hold_effects(&item_text) {
        flag_sets
            .entry(hold_effect)
            .or_default()
            .insert("AI_HOLD_EFFECT_KNOWLEDGE_CHOICE_LOCK".to_string());
    }

    let flag_map: HashMap<String, Vec<String>> = flag_sets
        .into_iter()
        .map(|(key, values)| (key, values.into_iter().collect()))
        .collect();

    let entries = constants
        .into_iter()
        .map(|constant| {
            let ai_knowledge_flags = flag_map.get(&constant.id).cloned().unwrap_or_default();
            let tags = tags_for(&ai_knowledge_flags, flag_defines);
            KnowledgeConstant {
                constant,
                ai_knowledge_flags,
                tags,
            }
        })
        .collect();

    Ok((entries, flag_map))
}

fn parse_items(
    root: &Path,
    hold_effect_flags: &HashMap<String, Vec<String>>,
    flag_defines: &HashMap<String, FlagDefine>,
) -> io::Result<Vec<ItemEntry>> {
    let items_text = read_text(root, "src/data/items.h")?;
    let mut items = Vec::new();

    for (item_id, block) in collect_indexed_blocks(&items_text, "ITEM") {
        let hold_effect = parse_assignment(&block, "holdEffect")
            .unwrap_or_else(|| "HOLD_EFFECT_NONE".to_string());
        let ai_knowledge_flags = hold_effect_flags
            .get(&hold_effect)
            .cloned()
            .unwrap_or_default();
        let tags = tags_for(&ai_knowledge_flags, flag_defines);
        items.push(ItemEntry {
            id: item_id,
            name: parse_compound_string(&block, "name"),
            hold_effect,
            hold_effect_param: parse_assignment(&block, "holdEffectParam"),
            ai_knowledge_flags,
            tags,
        });
    }

    Ok(items)
}

fn tags_for(flags: &[String], flag_defines: &HashMap<String, FlagDefine>) -> Vec<String> {
    let mut tags = BTreeSet::new();
    for flag in flags {
        if let Some(flag_define) = flag_defines.get(flag) {
            tags.insert(flag_define.tag.clone());
        }
    }
    tags.into_iter().collect()
}

fn gimmick_policies() -> Vec<GimmickPolicy> {
    vec![
        GimmickPolicy {
            id: "dynamax",
            default_priority: 1,
            resource_policy: "highest_value_when_hp_or_max_move_board_control_changes_next_2_to_3_turns",
            reason_tags: &["max_move_board_control", "max_move_defensive_timing", "preserve_dynamax"],
        },
        GimmickPolicy {
            id: "terastal",
            default_priority: 2,
            resource_policy: "prefer_when_type_flip_creates_survival_or_stable_sweep_line",
            reason_tags: &["defensive_type_flip", "offensive_stab_gain", "preserve_tera"],
        },
        GimmickPolicy {
            id: "z_move",
            default_priority: 3,
            resource_policy: "prefer_for_ko_conversion_trap_break_accuracy_stabilization_or_status_z_payoff",
            reason_tags: &["ko_conversion", "low_accuracy_stabilized", "status_z_payoff"],
        },
        GimmickPolicy {
            id: "mega",
            default_priority: 4,
            resource_policy: "usually_use_when_post_form_value_is_better_but_preserve_pre_mega_ability_when_needed",
            reason_tags: &["mega_output_gain", "preserve_pre_mega_ability"],
        },
    ]
}

fn require_catalogs(catalogs: &Catalogs) -> Result<(), String> {
    let required = [
        ("moves", catalogs.moves.len(), 100usize),
        ("abilities", catalogs.abilities.len(), 100usize),
        ("hold_effects", catalogs.hold_effects.len(), 50usize),
        ("items", catalogs.items.len(), 100usize),
        ("gimmicks", catalogs.gimmicks.len(), 4usize),
    ];
    for (name, count, minimum) in required {
        if count < minimum {
            return Err(format!(
                "{name} catalog too small: got {count}, expected at least {minimum}"
            ));
        }
    }
    Ok(())
}

fn write_json(path: PathBuf, content: String) -> io::Result<()> {
    fs::write(path, content)
}

fn json_escape(value: &str) -> String {
    let mut out = String::with_capacity(value.len() + 2);
    out.push('"');
    for ch in value.chars() {
        match ch {
            '"' => out.push_str("\\\""),
            '\\' => out.push_str("\\\\"),
            '\n' => out.push_str("\\n"),
            '\r' => out.push_str("\\r"),
            '\t' => out.push_str("\\t"),
            ch if ch.is_control() => out.push_str(&format!("\\u{:04x}", ch as u32)),
            ch => out.push(ch),
        }
    }
    out.push('"');
    out
}

fn json_array_strings(values: &[String], indent: usize) -> String {
    if values.is_empty() {
        return "[]".to_string();
    }
    let pad = "  ".repeat(indent);
    let inner_pad = "  ".repeat(indent + 1);
    let mut out = String::from("[\n");
    for (index, value) in values.iter().enumerate() {
        out.push_str(&inner_pad);
        out.push_str(&json_escape(value));
        if index + 1 != values.len() {
            out.push(',');
        }
        out.push('\n');
    }
    out.push_str(&pad);
    out.push(']');
    out
}

fn json_array_static_strings(values: &[&str], indent: usize) -> String {
    let owned = values
        .iter()
        .map(|value| (*value).to_string())
        .collect::<Vec<_>>();
    json_array_strings(&owned, indent)
}

fn json_map_strings(values: &BTreeMap<String, String>, indent: usize) -> String {
    if values.is_empty() {
        return "{}".to_string();
    }
    let pad = "  ".repeat(indent);
    let inner_pad = "  ".repeat(indent + 1);
    let mut out = String::from("{\n");
    for (index, (key, value)) in values.iter().enumerate() {
        out.push_str(&inner_pad);
        out.push_str(&json_escape(key));
        out.push_str(": ");
        out.push_str(&json_escape(value));
        if index + 1 != values.len() {
            out.push(',');
        }
        out.push('\n');
    }
    out.push_str(&pad);
    out.push('}');
    out
}

fn json_map_usize(values: &BTreeMap<String, usize>, indent: usize) -> String {
    if values.is_empty() {
        return "{}".to_string();
    }
    let pad = "  ".repeat(indent);
    let inner_pad = "  ".repeat(indent + 1);
    let mut out = String::from("{\n");
    for (index, (key, value)) in values.iter().enumerate() {
        out.push_str(&inner_pad);
        out.push_str(&json_escape(key));
        out.push_str(": ");
        out.push_str(&value.to_string());
        if index + 1 != values.len() {
            out.push(',');
        }
        out.push('\n');
    }
    out.push_str(&pad);
    out.push('}');
    out
}

fn json_option_string(value: &Option<String>) -> String {
    value
        .as_ref()
        .map(|value| json_escape(value))
        .unwrap_or_else(|| "null".to_string())
}

fn json_objects<T>(values: &[T], indent: usize, render: fn(&T, usize) -> String) -> String {
    if values.is_empty() {
        return "[]\n".to_string();
    }
    let pad = "  ".repeat(indent);
    let inner_pad = "  ".repeat(indent + 1);
    let mut out = String::from("[\n");
    for (index, value) in values.iter().enumerate() {
        let rendered = render(value, indent + 1);
        for (line_index, line) in rendered.lines().enumerate() {
            if line_index == 0 {
                out.push_str(&inner_pad);
                out.push_str(line);
            } else {
                out.push('\n');
                out.push_str(line);
            }
        }
        if index + 1 != values.len() {
            out.push(',');
        }
        out.push('\n');
    }
    out.push_str(&pad);
    out.push_str("]\n");
    out
}

fn json_moves(moves: &[MoveEntry]) -> String {
    json_objects(moves, 0, json_move)
}

fn json_move(value: &MoveEntry, indent: usize) -> String {
    object_with_fields(
        vec![
            (
                "ai_knowledge_flags",
                json_array_strings(&value.ai_knowledge_flags, indent + 1),
            ),
            (
                "conditional_runtime_flags",
                json_map_strings(&value.conditional_runtime_flags, indent + 1),
            ),
            ("fields", json_map_strings(&value.fields, indent + 1)),
            ("id", json_escape(&value.id)),
            ("name", json_option_string(&value.name)),
            (
                "runtime_flags",
                json_map_strings(&value.runtime_flags, indent + 1),
            ),
            ("tags", json_array_strings(&value.tags, indent + 1)),
        ],
        indent,
    )
}

fn json_knowledge_constants(values: &[KnowledgeConstant]) -> String {
    json_objects(values, 0, json_knowledge_constant)
}

fn json_knowledge_constant(value: &KnowledgeConstant, indent: usize) -> String {
    object_with_fields(
        vec![
            (
                "ai_knowledge_flags",
                json_array_strings(&value.ai_knowledge_flags, indent + 1),
            ),
            ("id", json_escape(&value.constant.id)),
            ("ordinal", value.constant.ordinal.to_string()),
            ("raw_value", json_option_string(&value.constant.raw_value)),
            ("tags", json_array_strings(&value.tags, indent + 1)),
            ("value", value.constant.value.to_string()),
        ],
        indent,
    )
}

fn json_items(items: &[ItemEntry]) -> String {
    json_objects(items, 0, json_item)
}

fn json_item(value: &ItemEntry, indent: usize) -> String {
    object_with_fields(
        vec![
            (
                "ai_knowledge_flags",
                json_array_strings(&value.ai_knowledge_flags, indent + 1),
            ),
            ("hold_effect", json_escape(&value.hold_effect)),
            (
                "hold_effect_param",
                json_option_string(&value.hold_effect_param),
            ),
            ("id", json_escape(&value.id)),
            ("name", json_option_string(&value.name)),
            ("tags", json_array_strings(&value.tags, indent + 1)),
        ],
        indent,
    )
}

fn json_gimmicks(gimmicks: &[GimmickPolicy]) -> String {
    json_objects(gimmicks, 0, json_gimmick)
}

fn json_gimmick(value: &GimmickPolicy, indent: usize) -> String {
    object_with_fields(
        vec![
            ("default_priority", value.default_priority.to_string()),
            ("id", json_escape(value.id)),
            (
                "reason_tags",
                json_array_static_strings(value.reason_tags, indent + 1),
            ),
            ("resource_policy", json_escape(value.resource_policy)),
        ],
        indent,
    )
}

fn json_summary(summary: &Summary) -> String {
    object_with_fields(
        vec![
            ("catalog_version", summary.catalog_version.to_string()),
            ("counts", json_map_usize(&summary.counts, 1)),
            ("flag_counts", json_map_usize(&summary.flag_counts, 1)),
            ("source_files", json_array_strings(&summary.source_files, 1)),
        ],
        0,
    ) + "\n"
}

fn object_with_fields(fields: Vec<(&str, String)>, indent: usize) -> String {
    let pad = "  ".repeat(indent);
    let inner_pad = "  ".repeat(indent + 1);
    let mut out = String::from("{\n");
    let len = fields.len();
    for (index, (key, value)) in fields.into_iter().enumerate() {
        out.push_str(&inner_pad);
        out.push_str(&json_escape(key));
        out.push_str(": ");
        out.push_str(&value);
        if index + 1 != len {
            out.push(',');
        }
        out.push('\n');
    }
    out.push_str(&pad);
    out.push('}');
    out
}
