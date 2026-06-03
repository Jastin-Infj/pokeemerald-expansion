#!/usr/bin/env sh
set -u

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../.." && pwd)

out_root="$repo_root/logs/validation"
run_id=$(date +%Y%m%d-%H%M%S)
run_make_all=1
run_make_debug=1
run_make_check=1
run_diff_check=1
review_mode=none
review_base=master
review_commit=
focused_tests=
isolated_codex_home=0
summarize_only=

usage() {
    cat <<'EOF'
Usage: tools/validation/run_review_checks.sh [options]

Runs local validation into timestamped log files. Codex review is opt-in.

Options:
  --out DIR                  Log directory root. Default: logs/validation
  --run-id NAME              Run directory name. Default: YYYYMMDD-HHMMSS
  --skip-diff-check          Skip git diff --check
  --skip-all                 Skip rtk make -j16 -O all
  --skip-debug               Skip rtk make -j16 -O debug
  --skip-check               Skip rtk make -j16 -O check
  --skip-make                Skip all make steps
  --with-codex-review        Also run rtk codex review --uncommitted
  --review-mode MODE         uncommitted, base, commit, or none. Default: none
  --base BRANCH              Base branch for --review-mode base. Default: master
  --commit SHA               Commit SHA for --review-mode commit
  --tests PATTERN            Also run focused make check with TESTS=PATTERN
  --isolated-codex-home      Run codex review with HOME/CODEX_HOME inside log dir
  --summarize-only DIR       Regenerate ACTIONABLE_FINDINGS.md for an existing log dir
  -h, --help                 Show this help

Examples:
  tools/validation/run_review_checks.sh
  tools/validation/run_review_checks.sh --with-codex-review
  tools/validation/run_review_checks.sh --review-mode uncommitted
  tools/validation/run_review_checks.sh --review-mode base --base master
  tools/validation/run_review_checks.sh --review-mode commit --commit abc1234
  tools/validation/run_review_checks.sh --summarize-only logs/validation/20260601-120000
EOF
}

while [ "$#" -gt 0 ]; do
    case "$1" in
        --out)
            out_root=$2
            shift 2
            ;;
        --run-id)
            run_id=$2
            shift 2
            ;;
        --skip-diff-check)
            run_diff_check=0
            shift
            ;;
        --skip-all)
            run_make_all=0
            shift
            ;;
        --skip-debug)
            run_make_debug=0
            shift
            ;;
        --skip-check)
            run_make_check=0
            shift
            ;;
        --skip-make)
            run_make_all=0
            run_make_debug=0
            run_make_check=0
            shift
            ;;
        --with-codex-review)
            review_mode=uncommitted
            shift
            ;;
        --review-mode)
            review_mode=$2
            shift 2
            ;;
        --base)
            review_base=$2
            shift 2
            ;;
        --commit)
            review_commit=$2
            shift 2
            ;;
        --tests)
            focused_tests=$2
            shift 2
            ;;
        --isolated-codex-home)
            isolated_codex_home=1
            shift
            ;;
        --summarize-only)
            summarize_only=$2
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "unknown option: $1" >&2
            usage >&2
            exit 2
            ;;
    esac
done

case "$review_mode" in
    uncommitted|base|commit|none)
        ;;
    *)
        echo "invalid --review-mode: $review_mode" >&2
        exit 2
        ;;
esac

if [ "$review_mode" = "commit" ] && [ -z "$review_commit" ]; then
    echo "--review-mode commit requires --commit SHA" >&2
    exit 2
fi

if [ -n "$summarize_only" ]; then
    case "$summarize_only" in
        /*)
            out_dir=$summarize_only
            ;;
        *)
            out_dir="$repo_root/$summarize_only"
            ;;
    esac
    run_id=$(basename "$out_dir")
    if [ ! -d "$out_dir" ]; then
        echo "--summarize-only directory does not exist: $out_dir" >&2
        exit 2
    fi
else
    out_dir="$out_root/$run_id"
    mkdir -p "$out_dir"
fi
summary="$out_dir/SUMMARY.md"
failures="$out_dir/failures.txt"
findings="$out_dir/ACTIONABLE_FINDINGS.md"
if [ -z "$summarize_only" ] || [ ! -f "$failures" ]; then
    : > "$failures"
fi

run_step() {
    name=$1
    shift
    log="$out_dir/$name.log"

    {
        printf '## %s\n' "$name"
        printf 'cwd: %s\n' "$repo_root"
        printf 'command:'
        for arg in "$@"; do
            printf ' %s' "$arg"
        done
        printf '\n\n'
    } > "$log"

    printf '[run] %s\n' "$name"
    if (cd "$repo_root" && "$@") >> "$log" 2>&1; then
        printf '[pass] %s\n' "$name"
        return 0
    else
        status=$?
    fi

    printf '[fail] %s (exit %s)\n' "$name" "$status"
    printf '%s exit %s\n' "$name" "$status" >> "$failures"
    return 0
}

run_step_shell() {
    name=$1
    shell_cmd=$2
    log="$out_dir/$name.log"

    {
        printf '## %s\n' "$name"
        printf 'cwd: %s\n' "$repo_root"
        printf 'command: %s\n\n' "$shell_cmd"
    } > "$log"

    printf '[run] %s\n' "$name"
    if (cd "$repo_root" && sh -c "$shell_cmd") >> "$log" 2>&1; then
        printf '[pass] %s\n' "$name"
        return 0
    else
        status=$?
    fi

    printf '[fail] %s (exit %s)\n' "$name" "$status"
    printf '%s exit %s\n' "$name" "$status" >> "$failures"
    return 0
}

run_codex_review() {
    case "$review_mode" in
        none)
            return 0
            ;;
        uncommitted)
            review_cmd='rtk codex review --uncommitted'
            ;;
        base)
            review_cmd="rtk codex review --base '$review_base'"
            ;;
        commit)
            review_cmd="rtk codex review --commit '$review_commit'"
            ;;
    esac

    if [ "$isolated_codex_home" -eq 1 ]; then
        mkdir -p "$out_dir/codex_home" "$out_dir/runtime"
        run_step_shell "codex-review-$review_mode" "HOME='$out_dir/codex_home' CODEX_HOME='$out_dir/codex_home' XDG_RUNTIME_DIR='$out_dir/runtime' $review_cmd"
    else
        run_step_shell "codex-review-$review_mode" "$review_cmd"
    fi
}

write_actionable_findings() {
    {
        printf '# Actionable Findings %s\n\n' "$run_id"
        printf 'This file is the first thing to read. Full logs remain in the same directory.\n\n'

        printf '## Failed Steps\n\n'
        if [ -s "$failures" ]; then
            sed 's/^/- /' "$failures"
        else
            printf 'None.\n'
        fi

        printf '\n## Codex Review Priorities\n\n'
        found_review=0
        for log in "$out_dir"/codex-review-*.log; do
            [ -e "$log" ] || continue
            matches=$(grep -n -E '^[[:space:]]*- \[[Pp][0-3]\]|^Review was interrupted|^ERROR: (Reconnecting|stream disconnected)|^[0-9T:.-]+Z ERROR|invalid_grant' "$log" || true)
            if [ -n "$matches" ]; then
                found_review=1
                printf '### `%s`\n\n' "$(basename "$log")"
                printf '%s\n\n' "$matches" | sed 's/^/- /'
            fi
        done
        if [ "$found_review" -eq 0 ]; then
            printf 'None found in review logs.\n'
        fi

        printf '\n## Failed Log Tails\n\n'
        if [ -s "$failures" ]; then
            while IFS= read -r failure; do
                step=${failure% exit *}
                log="$out_dir/$step.log"
                [ -f "$log" ] || continue
                printf '### `%s`\n\n' "$(basename "$log")"
                printf '```text\n'
                tail -120 "$log"
                printf '\n```\n\n'
            done < "$failures"
        else
            printf 'None.\n'
        fi

        printf '\n## Suspicious Markers In Passing Logs\n\n'
        found_marker=0
        for log in "$out_dir"/*.log; do
            [ -e "$log" ] || continue
            case "$(basename "$log")" in
                codex-review-*.log)
                    continue
                    ;;
            esac
            matches=$(grep -n -E 'No tests found|KNOWN_FAILING|EXPECTED_FAIL|warning:|WARN|ERROR|FAILED|CRASH' "$log" | grep -v 'Review was interrupted' || true)
            if [ -n "$matches" ]; then
                found_marker=1
                printf '### `%s`\n\n' "$(basename "$log")"
                printf '%s\n\n' "$matches" | head -80 | sed 's/^/- /'
            fi
        done
        if [ "$found_marker" -eq 0 ]; then
            printf 'None.\n'
        fi
    } > "$findings"
}

write_summary() {
    {
        printf '# Validation Run %s\n\n' "$run_id"
        printf '%s\n' "- Repository: \`$repo_root\`"
        printf '%s\n' "- Output directory: \`$out_dir\`"
        printf '%s\n' "- Review mode: \`$review_mode\`"
        if [ "$review_mode" = "base" ]; then
            printf '%s\n' "- Review base: \`$review_base\`"
        fi
        if [ "$review_mode" = "commit" ]; then
            printf '%s\n' "- Review commit: \`$review_commit\`"
        fi
        if [ -n "$focused_tests" ]; then
            printf '%s\n' "- Focused tests: \`$focused_tests\`"
        fi
        if [ -n "$summarize_only" ]; then
            printf '%s\n' "- Mode: summarize existing logs only"
        fi
        printf '\n## Logs\n\n'
        printf '%s\n' "- \`$(basename "$findings")\`"
        for log in "$out_dir"/*.log; do
            [ -e "$log" ] || continue
            printf '%s\n' "- \`$(basename "$log")\`"
        done
        printf '\n## Failures\n\n'
        if [ -s "$failures" ]; then
            sed 's/^/- /' "$failures"
        else
            printf 'None.\n'
        fi
    } > "$summary"
}

finish_run() {
    write_actionable_findings
    write_summary

    printf '\nsummary: %s\n' "$summary"
    printf 'actionable: %s\n' "$findings"
    if [ -s "$failures" ]; then
        exit 1
    fi
    exit 0
}

if [ -n "$summarize_only" ]; then
    finish_run
fi

run_step "git-status" rtk git status --short

if [ "$run_diff_check" -eq 1 ]; then
    run_step "git-diff-check" rtk git diff --check
fi

if [ "$run_make_all" -eq 1 ]; then
    run_step "make-all" rtk make -j16 -O all
fi

if [ "$run_make_debug" -eq 1 ]; then
    run_step "make-debug" rtk make -j16 -O debug
fi

if [ "$run_make_check" -eq 1 ]; then
    run_step "make-check" rtk make -j16 -O check
fi

if [ -n "$focused_tests" ]; then
    run_step_shell "make-check-focused" "TESTS='$focused_tests' rtk make -j16 -O check"
fi

run_codex_review

finish_run
