#!/usr/bin/env sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
fixture="$script_dir/testdata/basic"
tmpdir=$(mktemp -d /tmp/maprelink-test.XXXXXX)
plan="$tmpdir/plan.json"
dry_run_log="$tmpdir/dry-run.log"
missing_group_tmpdir=$(mktemp -d /tmp/maprelink-new-group-test.XXXXXX)
missing_group_plan="$missing_group_tmpdir/plan.json"

assert_audit_fails() {
	root=$1
	expected=$2
	log="$root/audit.log"
	if python3 "$script_dir/map_relink.py" --root "$root" audit >"$log" 2>&1; then
		echo "expected audit to fail for $root" >&2
		cat "$log" >&2
		exit 1
	fi
	grep -F -q "$expected" "$log"
}

cp -R "$fixture/." "$tmpdir/"
cp -R "$fixture/." "$missing_group_tmpdir/"

python3 "$script_dir/map_relink.py" --root "$tmpdir" audit
python3 "$script_dir/map_relink.py" --root "$tmpdir" plan \
	--map OldCave_2:OldCave_2F \
	--from-group gMapGroup_Temp \
	--to-group gMapGroup_RougeCave \
	--out "$plan"
python3 -m json.tool "$plan" >/dev/null

grep -q '"newId": "MAP_OLD_CAVE_2F"' "$plan"
grep -q '"newId": "LAYOUT_OLD_CAVE_2F"' "$plan"
grep -q '"fromGroup": "gMapGroup_Temp"' "$plan"
grep -q '"toGroup": "gMapGroup_RougeCave"' "$plan"

python3 "$script_dir/map_relink.py" --root "$tmpdir" apply --dry-run "$plan" >"$dry_run_log"
grep -q 'MOVE data/maps/OldCave_2 -> data/maps/OldCave_2F' "$dry_run_log"
test -d "$tmpdir/data/maps/OldCave_2"
test ! -e "$tmpdir/data/maps/OldCave_2F"

python3 "$script_dir/map_relink.py" --root "$tmpdir" apply --allow-dirty "$plan"
python3 "$script_dir/map_relink.py" --root "$tmpdir" validate

test -d "$tmpdir/data/maps/OldCave_2F"
test ! -e "$tmpdir/data/maps/OldCave_2"
test -d "$tmpdir/data/layouts/OldCave_2F"
test ! -e "$tmpdir/data/layouts/OldCave_2"

python3 -m json.tool "$tmpdir/data/maps/OldCave_2F/map.json" >/dev/null
python3 -m json.tool "$tmpdir/data/maps/OldCave_Exit/map.json" >/dev/null
python3 -m json.tool "$tmpdir/data/maps/map_groups.json" >/dev/null
python3 -m json.tool "$tmpdir/data/layouts/layouts.json" >/dev/null

grep -q '"id": "MAP_OLD_CAVE_2F"' "$tmpdir/data/maps/OldCave_2F/map.json"
grep -q '"name": "OldCave_2F"' "$tmpdir/data/maps/OldCave_2F/map.json"
grep -q '"layout": "LAYOUT_OLD_CAVE_2F"' "$tmpdir/data/maps/OldCave_2F/map.json"
grep -q '"OldCave_2F"' "$tmpdir/data/maps/map_groups.json"
python3 -c 'import json, sys; data = json.load(open(sys.argv[1])); assert "OldCave_2" not in data["gMapGroup_Temp"]; assert "OldCave_2F" in data["gMapGroup_RougeCave"]' "$tmpdir/data/maps/map_groups.json"
grep -q '"id": "LAYOUT_OLD_CAVE_2F"' "$tmpdir/data/layouts/layouts.json"
grep -q 'data/layouts/OldCave_2F/map.bin' "$tmpdir/data/layouts/layouts.json"
grep -q 'data/maps/OldCave_2F/scripts.inc' "$tmpdir/data/event_scripts.s"
grep -q '"MAP_OLD_CAVE_2F"' "$tmpdir/data/maps/OldCave_Exit/map.json"
grep -q 'OldCave_2 should remain in dialogue' "$tmpdir/data/maps/OldCave_2F/scripts.inc"

python3 "$script_dir/map_relink.py" --root "$missing_group_tmpdir" plan \
	--map OldCave_2:OldCave_2F \
	--from-group gMapGroup_Temp \
	--to-group gMapGroup_NewArea \
	--out "$missing_group_plan"
python3 "$script_dir/map_relink.py" --root "$missing_group_tmpdir" apply --allow-dirty "$missing_group_plan" >/dev/null
python3 "$script_dir/map_relink.py" --root "$missing_group_tmpdir" validate
python3 -c 'import json, sys; data = json.load(open(sys.argv[1])); assert "gMapGroup_NewArea" in data["group_order"]; assert "OldCave_2F" in data["gMapGroup_NewArea"]; assert "OldCave_2" not in data["gMapGroup_Temp"]' "$missing_group_tmpdir/data/maps/map_groups.json"

broken_group_tmpdir=$(mktemp -d /tmp/maprelink-bad-group-test.XXXXXX)
cp -R "$fixture/." "$broken_group_tmpdir/"
python3 -c 'import json, sys; p = sys.argv[1]; data = json.load(open(p)); data["gMapGroup_Temp"][0] = "OldCave_Typo"; json.dump(data, open(p, "w"), indent=2); open(p, "a").write("\n")' "$broken_group_tmpdir/data/maps/map_groups.json"
assert_audit_fails "$broken_group_tmpdir" "map_groups.json lists 'OldCave_Typo'"

broken_map_name_tmpdir=$(mktemp -d /tmp/maprelink-bad-map-name-test.XXXXXX)
cp -R "$fixture/." "$broken_map_name_tmpdir/"
python3 -c 'import json, sys; p = sys.argv[1]; data = json.load(open(p)); data["name"] = "OldCaveTypo"; json.dump(data, open(p, "w"), indent=2); open(p, "a").write("\n")' "$broken_map_name_tmpdir/data/maps/OldCave_2/map.json"
assert_audit_fails "$broken_map_name_tmpdir" "name is 'OldCaveTypo', expected 'OldCave_2'"

broken_layout_tmpdir=$(mktemp -d /tmp/maprelink-bad-layout-test.XXXXXX)
cp -R "$fixture/." "$broken_layout_tmpdir/"
python3 -c 'import json, sys; p = sys.argv[1]; data = json.load(open(p)); data["layout"] = "LAYOUT_OLD_CAVE_TYPO"; json.dump(data, open(p, "w"), indent=2); open(p, "a").write("\n")' "$broken_layout_tmpdir/data/maps/OldCave_2/map.json"
assert_audit_fails "$broken_layout_tmpdir" "references missing layout 'LAYOUT_OLD_CAVE_TYPO'"

broken_mapsec_tmpdir=$(mktemp -d /tmp/maprelink-bad-mapsec-test.XXXXXX)
cp -R "$fixture/." "$broken_mapsec_tmpdir/"
python3 -c 'import json, sys; p = sys.argv[1]; data = json.load(open(p)); data["region_map_section"] = "MAPSEC_OLD_CAVE_TYPO"; json.dump(data, open(p, "w"), indent=2); open(p, "a").write("\n")' "$broken_mapsec_tmpdir/data/maps/OldCave_2/map.json"
assert_audit_fails "$broken_mapsec_tmpdir" "references missing region_map_section 'MAPSEC_OLD_CAVE_TYPO'"

broken_warp_tmpdir=$(mktemp -d /tmp/maprelink-bad-warp-test.XXXXXX)
cp -R "$fixture/." "$broken_warp_tmpdir/"
python3 -c 'import json, sys; p = sys.argv[1]; data = json.load(open(p)); data["warp_events"][0]["dest_map"] = "MAP_OLD_CAVE_TYPO"; json.dump(data, open(p, "w"), indent=2); open(p, "a").write("\n")' "$broken_warp_tmpdir/data/maps/OldCave_2/map.json"
assert_audit_fails "$broken_warp_tmpdir" "references missing map id 'MAP_OLD_CAVE_TYPO'"

echo "map_asset_relinker fixture test passed: $tmpdir $missing_group_tmpdir $broken_group_tmpdir $broken_map_name_tmpdir $broken_layout_tmpdir $broken_mapsec_tmpdir $broken_warp_tmpdir"
