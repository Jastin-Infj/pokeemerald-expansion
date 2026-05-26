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
find "$tmpdir/.map_asset_relinker_backups" -name '*.bak.tar' -type f | grep -q .
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
python3 "$script_dir/map_relink.py" --root "$broken_group_tmpdir" plan \
	--map OldCave_2:OldCave_2F \
	--old-group-map-name OldCave_Typo \
	--from-group gMapGroup_Temp \
	--to-group gMapGroup_RougeCave \
	--out "$broken_group_tmpdir/repair.json"
python3 "$script_dir/map_relink.py" --root "$broken_group_tmpdir" apply --allow-dirty "$broken_group_tmpdir/repair.json" >/dev/null
python3 "$script_dir/map_relink.py" --root "$broken_group_tmpdir" validate

broken_map_name_tmpdir=$(mktemp -d /tmp/maprelink-bad-map-name-test.XXXXXX)
cp -R "$fixture/." "$broken_map_name_tmpdir/"
python3 -c 'import json, sys; p = sys.argv[1]; data = json.load(open(p)); data["name"] = "OldCaveTypo"; json.dump(data, open(p, "w"), indent=2); open(p, "a").write("\n")' "$broken_map_name_tmpdir/data/maps/OldCave_2/map.json"
assert_audit_fails "$broken_map_name_tmpdir" "name is 'OldCaveTypo', expected 'OldCave_2'"
python3 "$script_dir/map_relink.py" --root "$broken_map_name_tmpdir" plan \
	--map OldCave_2:OldCave_2F \
	--from-group gMapGroup_Temp \
	--to-group gMapGroup_RougeCave \
	--out "$broken_map_name_tmpdir/repair.json"
python3 "$script_dir/map_relink.py" --root "$broken_map_name_tmpdir" apply --allow-dirty "$broken_map_name_tmpdir/repair.json" >/dev/null
python3 "$script_dir/map_relink.py" --root "$broken_map_name_tmpdir" validate

broken_map_id_tmpdir=$(mktemp -d /tmp/maprelink-bad-map-id-test.XXXXXX)
cp -R "$fixture/." "$broken_map_id_tmpdir/"
python3 -c 'import json, sys; p = sys.argv[1]; data = json.load(open(p)); data["id"] = "MAP_OLD_CAVE_TYPO"; json.dump(data, open(p, "w"), indent=2); open(p, "a").write("\n")' "$broken_map_id_tmpdir/data/maps/OldCave_2/map.json"
assert_audit_fails "$broken_map_id_tmpdir" "references missing map id 'MAP_OLD_CAVE_2'"
python3 "$script_dir/map_relink.py" --root "$broken_map_id_tmpdir" plan \
	--map OldCave_2:OldCave_2F \
	--from-group gMapGroup_Temp \
	--to-group gMapGroup_RougeCave \
	--out "$broken_map_id_tmpdir/repair.json"
python3 "$script_dir/map_relink.py" --root "$broken_map_id_tmpdir" apply --allow-dirty "$broken_map_id_tmpdir/repair.json" >/dev/null
python3 "$script_dir/map_relink.py" --root "$broken_map_id_tmpdir" validate

broken_layout_tmpdir=$(mktemp -d /tmp/maprelink-bad-layout-test.XXXXXX)
cp -R "$fixture/." "$broken_layout_tmpdir/"
python3 -c 'import json, sys; p = sys.argv[1]; data = json.load(open(p)); data["layout"] = "LAYOUT_OLD_CAVE_TYPO"; json.dump(data, open(p, "w"), indent=2); open(p, "a").write("\n")' "$broken_layout_tmpdir/data/maps/OldCave_2/map.json"
assert_audit_fails "$broken_layout_tmpdir" "references missing layout 'LAYOUT_OLD_CAVE_TYPO'"
python3 "$script_dir/map_relink.py" --root "$broken_layout_tmpdir" plan \
	--map OldCave_2:OldCave_2F \
	--old-layout-id LAYOUT_OLD_CAVE_2 \
	--from-group gMapGroup_Temp \
	--to-group gMapGroup_RougeCave \
	--out "$broken_layout_tmpdir/repair.json"
python3 "$script_dir/map_relink.py" --root "$broken_layout_tmpdir" apply --allow-dirty "$broken_layout_tmpdir/repair.json" >/dev/null
python3 "$script_dir/map_relink.py" --root "$broken_layout_tmpdir" validate

foreign_layout_tmpdir=$(mktemp -d /tmp/maprelink-foreign-layout-test.XXXXXX)
cp -R "$fixture/." "$foreign_layout_tmpdir/"
python3 -c 'import json, sys; p = sys.argv[1]; data = json.load(open(p)); data["layout"] = "LAYOUT_OLD_CAVE_2"; json.dump(data, open(p, "w"), indent=2); open(p, "a").write("\n")' "$foreign_layout_tmpdir/data/maps/OldCave_Exit/map.json"
python3 "$script_dir/map_relink.py" --root "$foreign_layout_tmpdir" plan \
	--map OldCave_Exit:OldCave_Exit \
	--no-layout-rename \
	--set-layout-id LAYOUT_OLD_CAVE_EXIT \
	--out "$foreign_layout_tmpdir/repair.json"
python3 "$script_dir/map_relink.py" --root "$foreign_layout_tmpdir" apply --allow-dirty "$foreign_layout_tmpdir/repair.json" >/dev/null
python3 "$script_dir/map_relink.py" --root "$foreign_layout_tmpdir" validate
python3 -c 'import json, sys; data = json.load(open(sys.argv[1])); assert data["layout"] == "LAYOUT_OLD_CAVE_EXIT"' "$foreign_layout_tmpdir/data/maps/OldCave_Exit/map.json"

broken_mapsec_tmpdir=$(mktemp -d /tmp/maprelink-bad-mapsec-test.XXXXXX)
cp -R "$fixture/." "$broken_mapsec_tmpdir/"
python3 -c 'import json, sys; p = sys.argv[1]; data = json.load(open(p)); data["region_map_section"] = "MAPSEC_OLD_CAVE_TYPO"; json.dump(data, open(p, "w"), indent=2); open(p, "a").write("\n")' "$broken_mapsec_tmpdir/data/maps/OldCave_2/map.json"
assert_audit_fails "$broken_mapsec_tmpdir" "references missing region_map_section 'MAPSEC_OLD_CAVE_TYPO'"
python3 "$script_dir/map_relink.py" --root "$broken_mapsec_tmpdir" plan \
	--map OldCave_2:OldCave_2F \
	--new-mapsec MAPSEC_NONE \
	--from-group gMapGroup_Temp \
	--to-group gMapGroup_RougeCave \
	--out "$broken_mapsec_tmpdir/repair.json"
python3 "$script_dir/map_relink.py" --root "$broken_mapsec_tmpdir" apply --allow-dirty "$broken_mapsec_tmpdir/repair.json" >/dev/null
python3 "$script_dir/map_relink.py" --root "$broken_mapsec_tmpdir" validate

map_name_anchor_tmpdir=$(mktemp -d /tmp/maprelink-map-name-anchor-test.XXXXXX)
cp -R "$fixture/." "$map_name_anchor_tmpdir/"
mkdir -p "$map_name_anchor_tmpdir/src/data/region_map"
python3 -c 'import json, os, sys; p = sys.argv[1]; os.makedirs(os.path.dirname(p), exist_ok=True); json.dump({"map_sections": [{"id": "MAPSEC_OLDCAVE", "name": "OLDCAVE"}]}, open(p, "w"), indent=2); open(p, "a").write("\n")' "$map_name_anchor_tmpdir/src/data/region_map/region_map_sections.json"
python3 -c 'import json, sys; p = sys.argv[1]; data = json.load(open(p)); data["region_map_section"] = "MAPSEC_OLDCAVE"; json.dump(data, open(p, "w"), indent=2); open(p, "a").write("\n")' "$map_name_anchor_tmpdir/data/maps/OldCave_2/map.json"
python3 -c 'import json, sys; p = sys.argv[1]; data = json.load(open(p)); data["group_order"].append("gMapGroup_oldcave"); data["gMapGroup_oldcave"] = []; json.dump(data, open(p, "w"), indent=2); open(p, "a").write("\n")' "$map_name_anchor_tmpdir/data/maps/map_groups.json"
python3 "$script_dir/map_relink.py" --root "$map_name_anchor_tmpdir" plan \
	--map OldCave_2:OldCave_2 \
	--no-layout-rename \
	--to-group gMapGroup_OldCave \
	--rename-mapsec MAPSEC_OLDCAVE:MAPSEC_OLD_CAVE \
	--new-mapsec-name OLD-CAVE \
	--drop-group gMapGroup_oldcave \
	--out "$map_name_anchor_tmpdir/repair.json"
python3 "$script_dir/map_relink.py" --root "$map_name_anchor_tmpdir" apply --allow-dirty "$map_name_anchor_tmpdir/repair.json" >/dev/null
python3 "$script_dir/map_relink.py" --root "$map_name_anchor_tmpdir" validate
python3 -c 'import json, sys; data = json.load(open(sys.argv[1])); assert "OldCave_2" not in data["gMapGroup_Temp"]; assert "OldCave_2" in data["gMapGroup_OldCave"]; assert "gMapGroup_oldcave" not in data["group_order"]; assert "gMapGroup_oldcave" not in data' "$map_name_anchor_tmpdir/data/maps/map_groups.json"
python3 -c 'import json, sys; data = json.load(open(sys.argv[1])); assert data["region_map_section"] == "MAPSEC_OLD_CAVE"; assert data["layout"] == "LAYOUT_OLD_CAVE_2"' "$map_name_anchor_tmpdir/data/maps/OldCave_2/map.json"
python3 -c 'import json, sys; data = json.load(open(sys.argv[1])); sections = {s["id"]: s for s in data["map_sections"]}; assert "MAPSEC_OLDCAVE" not in sections; assert sections["MAPSEC_OLD_CAVE"]["name"] == "OLD-CAVE"' "$map_name_anchor_tmpdir/src/data/region_map/region_map_sections.json"

temp_mapsec_tmpdir=$(mktemp -d /tmp/maprelink-temp-mapsec-test.XXXXXX)
cp -R "$fixture/." "$temp_mapsec_tmpdir/"
mkdir -p "$temp_mapsec_tmpdir/src/data/region_map"
python3 -c 'import json, os, sys; p = sys.argv[1]; os.makedirs(os.path.dirname(p), exist_ok=True); json.dump({"map_sections": [{"id": "MAPSEC_Jongle", "name": "Jongle"}]}, open(p, "w"), indent=2); open(p, "a").write("\n")' "$temp_mapsec_tmpdir/src/data/region_map/region_map_sections.json"
python3 -c 'import json, sys; p = sys.argv[1]; data = json.load(open(p)); data["region_map_section"] = "MAPSEC_Jongle"; json.dump(data, open(p, "w"), indent=2); open(p, "a").write("\n")' "$temp_mapsec_tmpdir/data/maps/OldCave_2/map.json"
python3 "$script_dir/map_relink.py" --root "$temp_mapsec_tmpdir" plan-temp-mapsec \
	--map OldCave_2 \
	--dry-run \
	--out "$temp_mapsec_tmpdir/repair.json" | grep -q "EDIT data/maps/Jongle/scripts.inc"
python3 "$script_dir/map_relink.py" --root "$temp_mapsec_tmpdir" apply --allow-dirty "$temp_mapsec_tmpdir/repair.json" >/dev/null
python3 "$script_dir/map_relink.py" --root "$temp_mapsec_tmpdir" validate
python3 -c 'import json, sys; data = json.load(open(sys.argv[1])); assert "OldCave_2" not in data["gMapGroup_Temp"]; assert "Jongle" in data["gMapGroup_Jongle"]' "$temp_mapsec_tmpdir/data/maps/map_groups.json"
python3 -c 'import json, sys; data = json.load(open(sys.argv[1])); assert data["name"] == "Jongle"; assert data["region_map_section"] == "MAPSEC_JONGLE"; assert data["layout"] == "LAYOUT_JONGLE"' "$temp_mapsec_tmpdir/data/maps/Jongle/map.json"
python3 -c 'import json, sys; data = json.load(open(sys.argv[1])); sections = {s["id"]: s for s in data["map_sections"]}; assert "MAPSEC_Jongle" not in sections; assert sections["MAPSEC_JONGLE"]["name"] == "JONGLE"' "$temp_mapsec_tmpdir/src/data/region_map/region_map_sections.json"

tileset_fix_tmpdir=$(mktemp -d /tmp/maprelink-tileset-fix-test.XXXXXX)
cp -R "$fixture/." "$tileset_fix_tmpdir/"
python3 -c 'import json, sys; p = sys.argv[1]; data = json.load(open(p)); data["layouts"][0]["primary_tileset"] = "gTileset_Bad"; json.dump(data, open(p, "w"), indent=2); open(p, "a").write("\n")' "$tileset_fix_tmpdir/data/layouts/layouts.json"
python3 "$script_dir/map_relink.py" --root "$tileset_fix_tmpdir" plan \
	--map OldCave_2:OldCave_2 \
	--no-layout-rename \
	--set-primary-tileset gTileset_General \
	--out "$tileset_fix_tmpdir/repair.json"
python3 "$script_dir/map_relink.py" --root "$tileset_fix_tmpdir" apply --allow-dirty "$tileset_fix_tmpdir/repair.json" >/dev/null
python3 "$script_dir/map_relink.py" --root "$tileset_fix_tmpdir" validate
python3 -c 'import json, sys; data = json.load(open(sys.argv[1])); assert data["layouts"][0]["id"] == "LAYOUT_OLD_CAVE_2"; assert data["layouts"][0]["primary_tileset"] == "gTileset_General"' "$tileset_fix_tmpdir/data/layouts/layouts.json"

layout_name_fix_tmpdir=$(mktemp -d /tmp/maprelink-layout-name-fix-test.XXXXXX)
cp -R "$fixture/." "$layout_name_fix_tmpdir/"
python3 -c 'import json, sys; p = sys.argv[1]; data = json.load(open(p)); data["layouts"][0]["name"] = data["layouts"][1]["name"]; json.dump(data, open(p, "w"), indent=2); open(p, "a").write("\n")' "$layout_name_fix_tmpdir/data/layouts/layouts.json"
assert_audit_fails "$layout_name_fix_tmpdir" "layout name 'OldCave_Exit_Layout' appears multiple times"
python3 "$script_dir/map_relink.py" --root "$layout_name_fix_tmpdir" plan \
	--map OldCave_2:OldCave_2 \
	--no-layout-rename \
	--set-layout-name OldCave_2_Layout \
	--out "$layout_name_fix_tmpdir/repair.json"
python3 "$script_dir/map_relink.py" --root "$layout_name_fix_tmpdir" apply --allow-dirty "$layout_name_fix_tmpdir/repair.json" >/dev/null
python3 "$script_dir/map_relink.py" --root "$layout_name_fix_tmpdir" validate
python3 -c 'import json, sys; data = json.load(open(sys.argv[1])); assert data["layouts"][0]["id"] == "LAYOUT_OLD_CAVE_2"; assert data["layouts"][0]["name"] == "OldCave_2_Layout"' "$layout_name_fix_tmpdir/data/layouts/layouts.json"

duplicate_mapsec_tmpdir=$(mktemp -d /tmp/maprelink-duplicate-mapsec-test.XXXXXX)
cp -R "$fixture/." "$duplicate_mapsec_tmpdir/"
mkdir -p "$duplicate_mapsec_tmpdir/src/data/region_map"
python3 -c 'import json, os, sys; p = sys.argv[1]; os.makedirs(os.path.dirname(p), exist_ok=True); json.dump({"map_sections": [{"id": "MAPSEC_OLD_CAVE", "name": "OLD-CAVE"}, {"id": "MAPSEC_OLD_CAVE", "name": "OLD-CAVE-DUP"}]}, open(p, "w"), indent=2); open(p, "a").write("\n")' "$duplicate_mapsec_tmpdir/src/data/region_map/region_map_sections.json"
python3 -c 'import json, sys; p = sys.argv[1]; data = json.load(open(p)); data["region_map_section"] = "MAPSEC_OLD_CAVE"; json.dump(data, open(p, "w"), indent=2); open(p, "a").write("\n")' "$duplicate_mapsec_tmpdir/data/maps/OldCave_2/map.json"
assert_audit_fails "$duplicate_mapsec_tmpdir" "region map section id 'MAPSEC_OLD_CAVE' appears multiple times"
python3 "$script_dir/map_relink.py" --root "$duplicate_mapsec_tmpdir" plan \
	--map OldCave_2:OldCave_2 \
	--no-layout-rename \
	--rename-mapsec MAPSEC_OLD_CAVE:MAPSEC_OLD_CAVE_MAIN \
	--new-mapsec-name OLD-CAVE \
	--out "$duplicate_mapsec_tmpdir/repair.json"
python3 "$script_dir/map_relink.py" --root "$duplicate_mapsec_tmpdir" apply --allow-dirty "$duplicate_mapsec_tmpdir/repair.json" >/dev/null
python3 "$script_dir/map_relink.py" --root "$duplicate_mapsec_tmpdir" validate
python3 -c 'import json, sys; data = json.load(open(sys.argv[1])); ids = [s["id"] for s in data["map_sections"]]; assert ids.count("MAPSEC_OLD_CAVE") == 1; assert "MAPSEC_OLD_CAVE_MAIN" in ids' "$duplicate_mapsec_tmpdir/src/data/region_map/region_map_sections.json"

mapsec_name_fix_tmpdir=$(mktemp -d /tmp/maprelink-mapsec-name-fix-test.XXXXXX)
cp -R "$fixture/." "$mapsec_name_fix_tmpdir/"
mkdir -p "$mapsec_name_fix_tmpdir/src/data/region_map"
python3 -c 'import json, os, sys; p = sys.argv[1]; os.makedirs(os.path.dirname(p), exist_ok=True); json.dump({"map_sections": [{"id": "MAPSEC_OLD_CAVE", "name": "OLD WRONG"}]}, open(p, "w"), indent=2); open(p, "a").write("\n")' "$mapsec_name_fix_tmpdir/src/data/region_map/region_map_sections.json"
python3 "$script_dir/map_relink.py" --root "$mapsec_name_fix_tmpdir" plan \
	--map OldCave_2:OldCave_2 \
	--no-layout-rename \
	--set-mapsec-name MAPSEC_OLD_CAVE:OLD-CAVE \
	--out "$mapsec_name_fix_tmpdir/repair.json"
python3 "$script_dir/map_relink.py" --root "$mapsec_name_fix_tmpdir" apply --allow-dirty "$mapsec_name_fix_tmpdir/repair.json" >/dev/null
python3 "$script_dir/map_relink.py" --root "$mapsec_name_fix_tmpdir" validate
python3 -c 'import json, sys; data = json.load(open(sys.argv[1])); assert data["map_sections"][0]["id"] == "MAPSEC_OLD_CAVE"; assert data["map_sections"][0]["name"] == "OLD-CAVE"' "$mapsec_name_fix_tmpdir/src/data/region_map/region_map_sections.json"

region_map_tools_tmpdir=$(mktemp -d /tmp/maprelink-region-tools-test.XXXXXX)
cp -R "$fixture/." "$region_map_tools_tmpdir/"
mkdir -p "$region_map_tools_tmpdir/src/data/region_map"
mkdir -p "$region_map_tools_tmpdir/include/constants"
python3 -c 'import json, os, sys; p = sys.argv[1]; os.makedirs(os.path.dirname(p), exist_ok=True); json.dump({"map_sections": [{"id": "MAPSEC_OLD_CAVE", "name": "OLD-CAVE"}]}, open(p, "w"), indent=2); open(p, "a").write("\n")' "$region_map_tools_tmpdir/src/data/region_map/region_map_sections.json"
cat > "$region_map_tools_tmpdir/src/data/region_map/region_map_layout.h" <<'EOF'
static const mapsec_u8_t sRegionMap_MapSectionLayout[MAP_HEIGHT][MAP_WIDTH] = {
    {MAPSEC_NONE, MAPSEC_NONE, MAPSEC_NONE},
    {MAPSEC_NONE, MAPSEC_NONE, MAPSEC_NONE},
};
EOF
cat > "$region_map_tools_tmpdir/src/region_map.c" <<'EOF'
static const u8 sMapHealLocations[][3] =
{
    [MAPSEC_LITTLEROOT_TOWN] = {MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), HEAL_LOCATION_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F},
};

struct FlyLocation
{
    enum RegionMapType regionMapType;
    u16 flag;
    u16 mapsec;
};

static const struct FlyLocation sFlyLocations[] =
{
    {
        .regionMapType = REGION_MAP_HOENN,
        .mapsec = MAPSEC_LITTLEROOT_TOWN,
        .flag = FLAG_VISITED_LITTLEROOT_TOWN,
    },
};

static const mapsec_u16_t sPaletteBlinkFlyDestinations[] =
{
    MAPSEC_NONE,
};

static const mapsec_u16_t sRedOutlineFlyDestinations[][2] =
{
    {
        -1,
        MAPSEC_NONE
    },
};

static u8 GetMapsecType(mapsec_u16_t mapSecId)
{
    switch (mapSecId)
    {
    case MAPSEC_NONE:
        return MAPSECTYPE_NONE;
    default:
        return MAPSECTYPE_ROUTE;
    }
}

mapsec_u16_t GetRegionMapSecIdAt(u16 x, u16 y)
{
    return MAPSEC_NONE;
}
EOF
cat > "$region_map_tools_tmpdir/include/constants/flags.h" <<'EOF'
#define FLAG_UNUSED_0x881                           (SYSTEM_FLAGS + 0x21) // Unused Flag
EOF
python3 "$script_dir/map_relink.py" --root "$region_map_tools_tmpdir" plan \
	--map OldCave_2:OldCave_2 \
	--no-layout-rename \
	--new-mapsec MAPSEC_OLD_CAVE \
	--set-map-type MAP_TYPE_ROUTE \
	--set-show-map-name false \
	--set-transition-setflag FLAG_VISITED_OLD_CAVE \
	--set-mapsec-bounds MAPSEC_OLD_CAVE:1:1:2:1 \
	--set-region-map-cell hoenn:1:1:MAPSEC_OLD_CAVE \
	--ensure-mapsec-map MAPSEC_OLD_CAVE:MAP_OLD_CAVE_2:HEAL_LOCATION_NONE \
	--ensure-fly-location hoenn:MAPSEC_OLD_CAVE:FLAG_VISITED_OLD_CAVE \
	--ensure-fly-mapsec-type MAPSEC_OLD_CAVE:FLAG_VISITED_OLD_CAVE \
	--set-fly-icon-style MAPSEC_OLD_CAVE:palette-blink \
	--claim-unused-flag FLAG_UNUSED_0x881:FLAG_VISITED_OLD_CAVE \
	--out "$region_map_tools_tmpdir/repair.json"
python3 "$script_dir/map_relink.py" --root "$region_map_tools_tmpdir" apply --allow-dirty "$region_map_tools_tmpdir/repair.json" >/dev/null
python3 "$script_dir/map_relink.py" --root "$region_map_tools_tmpdir" validate
python3 -c 'import json, sys; data = json.load(open(sys.argv[1])); assert data["region_map_section"] == "MAPSEC_OLD_CAVE"; assert data["map_type"] == "MAP_TYPE_ROUTE"; assert data["show_map_name"] is False' "$region_map_tools_tmpdir/data/maps/OldCave_2/map.json"
python3 -c 'import json, sys; section = json.load(open(sys.argv[1]))["map_sections"][0]; assert section["x"] == 1 and section["y"] == 1 and section["width"] == 2 and section["height"] == 1' "$region_map_tools_tmpdir/src/data/region_map/region_map_sections.json"
grep -q '{MAPSEC_NONE, MAPSEC_OLD_CAVE, MAPSEC_NONE}' "$region_map_tools_tmpdir/src/data/region_map/region_map_layout.h"
grep -q '\[MAPSEC_OLD_CAVE\] = {MAP_GROUP(MAP_OLD_CAVE_2), MAP_NUM(MAP_OLD_CAVE_2), HEAL_LOCATION_NONE}' "$region_map_tools_tmpdir/src/region_map.c"
grep -q '.mapsec = MAPSEC_OLD_CAVE' "$region_map_tools_tmpdir/src/region_map.c"
grep -q '.flag = FLAG_VISITED_OLD_CAVE' "$region_map_tools_tmpdir/src/region_map.c"
grep -q 'case MAPSEC_OLD_CAVE:' "$region_map_tools_tmpdir/src/region_map.c"
grep -q 'FlagGet(FLAG_VISITED_OLD_CAVE)' "$region_map_tools_tmpdir/src/region_map.c"
grep -q '^    MAPSEC_OLD_CAVE,$' "$region_map_tools_tmpdir/src/region_map.c"
grep -q 'setflag FLAG_VISITED_OLD_CAVE' "$region_map_tools_tmpdir/data/maps/OldCave_2/scripts.inc"
grep -q 'FLAG_VISITED_OLD_CAVE' "$region_map_tools_tmpdir/include/constants/flags.h"
python3 "$script_dir/map_relink.py" --root "$region_map_tools_tmpdir" plan \
	--map OldCave_2:OldCave_2 \
	--no-layout-rename \
	--set-fly-icon-style MAPSEC_OLD_CAVE:blue-blink \
	--out "$region_map_tools_tmpdir/fly_icon_blue_blink.json"
python3 "$script_dir/map_relink.py" --root "$region_map_tools_tmpdir" apply --dry-run "$region_map_tools_tmpdir/fly_icon_blue_blink.json" | grep -q 'Dry-run complete; no files changed.'
python3 "$script_dir/map_relink.py" --root "$region_map_tools_tmpdir" plan \
	--map OldCave_2:OldCave_2 \
	--no-layout-rename \
	--set-fly-icon-style MAPSEC_OLD_CAVE:stock \
	--out "$region_map_tools_tmpdir/fly_icon_stock.json"
python3 "$script_dir/map_relink.py" --root "$region_map_tools_tmpdir" apply --allow-dirty "$region_map_tools_tmpdir/fly_icon_stock.json" >/dev/null
if grep -q '^    MAPSEC_OLD_CAVE,$' "$region_map_tools_tmpdir/src/region_map.c"; then
	echo "expected stock Fly icon style to remove palette-blink entry" >&2
	exit 1
fi
python3 "$script_dir/map_relink.py" --root "$region_map_tools_tmpdir" plan \
	--map OldCave_2:OldCave_2 \
	--no-layout-rename \
	--set-fly-icon-style MAPSEC_OLD_CAVE:red-outline \
	--out "$region_map_tools_tmpdir/fly_icon_red_outline.json"
python3 "$script_dir/map_relink.py" --root "$region_map_tools_tmpdir" apply --allow-dirty "$region_map_tools_tmpdir/fly_icon_red_outline.json" >/dev/null
grep -q 'FLAG_VISITED_OLD_CAVE,' "$region_map_tools_tmpdir/src/region_map.c"
grep -q '        MAPSEC_OLD_CAVE' "$region_map_tools_tmpdir/src/region_map.c"

broken_warp_tmpdir=$(mktemp -d /tmp/maprelink-bad-warp-test.XXXXXX)
cp -R "$fixture/." "$broken_warp_tmpdir/"
python3 -c 'import json, sys; p = sys.argv[1]; data = json.load(open(p)); data["warp_events"][0]["dest_map"] = "MAP_OLD_CAVE_TYPO"; json.dump(data, open(p, "w"), indent=2); open(p, "a").write("\n")' "$broken_warp_tmpdir/data/maps/OldCave_2/map.json"
assert_audit_fails "$broken_warp_tmpdir" "references missing map id 'MAP_OLD_CAVE_TYPO'"
python3 "$script_dir/map_relink.py" --root "$broken_warp_tmpdir" plan \
	--map OldCave_2:OldCave_2F \
	--old-map-id MAP_OLD_CAVE_TYPO \
	--from-group gMapGroup_Temp \
	--to-group gMapGroup_RougeCave \
	--out "$broken_warp_tmpdir/repair.json"
python3 "$script_dir/map_relink.py" --root "$broken_warp_tmpdir" apply --allow-dirty "$broken_warp_tmpdir/repair.json" >/dev/null
python3 "$script_dir/map_relink.py" --root "$broken_warp_tmpdir" validate

match_by_name_tmpdir=$(mktemp -d /tmp/maprelink-match-name-test.XXXXXX)
cp -R "$fixture/." "$match_by_name_tmpdir/"
mv "$match_by_name_tmpdir/data/maps/OldCave_2" "$match_by_name_tmpdir/data/maps/OldCaveDirTypo"
python3 -c 'import sys; p = sys.argv[1]; text = open(p).read().replace("data/maps/OldCave_2/scripts.inc", "data/maps/OldCaveDirTypo/scripts.inc"); open(p, "w").write(text)' "$match_by_name_tmpdir/data/event_scripts.s"
assert_audit_fails "$match_by_name_tmpdir" "name is 'OldCave_2', expected 'OldCaveDirTypo'"
python3 "$script_dir/map_relink.py" --root "$match_by_name_tmpdir" plan \
	--match-by name \
	--map OldCave_2:OldCave_2F \
	--from-group gMapGroup_Temp \
	--to-group gMapGroup_RougeCave \
	--out "$match_by_name_tmpdir/repair.json"
python3 "$script_dir/map_relink.py" --root "$match_by_name_tmpdir" apply --allow-dirty "$match_by_name_tmpdir/repair.json" >/dev/null
python3 "$script_dir/map_relink.py" --root "$match_by_name_tmpdir" validate

match_by_id_tmpdir=$(mktemp -d /tmp/maprelink-match-id-test.XXXXXX)
cp -R "$fixture/." "$match_by_id_tmpdir/"
mv "$match_by_id_tmpdir/data/maps/OldCave_2" "$match_by_id_tmpdir/data/maps/OldCaveDirTypo"
python3 -c 'import json, sys; p = sys.argv[1]; data = json.load(open(p)); data["name"] = "OldCaveNameTypo"; json.dump(data, open(p, "w"), indent=2); open(p, "a").write("\n")' "$match_by_id_tmpdir/data/maps/OldCaveDirTypo/map.json"
python3 -c 'import sys; p = sys.argv[1]; text = open(p).read().replace("data/maps/OldCave_2/scripts.inc", "data/maps/OldCaveDirTypo/scripts.inc"); open(p, "w").write(text)' "$match_by_id_tmpdir/data/event_scripts.s"
assert_audit_fails "$match_by_id_tmpdir" "name is 'OldCaveNameTypo', expected 'OldCaveDirTypo'"
python3 "$script_dir/map_relink.py" --root "$match_by_id_tmpdir" plan \
	--match-by id \
	--map MAP_OLD_CAVE_2:OldCave_2F \
	--old-group-map-name OldCave_2 \
	--from-group gMapGroup_Temp \
	--to-group gMapGroup_RougeCave \
	--out "$match_by_id_tmpdir/repair.json"
python3 "$script_dir/map_relink.py" --root "$match_by_id_tmpdir" apply --allow-dirty "$match_by_id_tmpdir/repair.json" >/dev/null
python3 "$script_dir/map_relink.py" --root "$match_by_id_tmpdir" validate

duplicate_script_tmpdir=$(mktemp -d /tmp/maprelink-duplicate-script-test.XXXXXX)
cp -R "$fixture/." "$duplicate_script_tmpdir/"
python3 -c 'import sys; p = sys.argv[1]; text = open(p).read(); text = text + "\t.include \"data/maps/OldCave_2/scripts.inc\"\n"; open(p, "w").write(text)' "$duplicate_script_tmpdir/data/event_scripts.s"
assert_audit_fails "$duplicate_script_tmpdir" "script include 'OldCave_2' appears multiple times"
python3 "$script_dir/map_relink.py" --root "$duplicate_script_tmpdir" plan \
	--map OldCave_2:OldCave_2 \
	--no-layout-rename \
	--out "$duplicate_script_tmpdir/repair.json"
python3 "$script_dir/map_relink.py" --root "$duplicate_script_tmpdir" apply --allow-dirty "$duplicate_script_tmpdir/repair.json" >/dev/null
python3 "$script_dir/map_relink.py" --root "$duplicate_script_tmpdir" validate
python3 -c 'import sys; text = open(sys.argv[1]).read(); assert text.count("data/maps/OldCave_2/scripts.inc") == 1' "$duplicate_script_tmpdir/data/event_scripts.s"

script_label_tmpdir=$(mktemp -d /tmp/maprelink-script-label-test.XXXXXX)
cp -R "$fixture/." "$script_label_tmpdir/"
python3 "$script_dir/map_relink.py" --root "$script_label_tmpdir" plan \
	--map OldCave_2:OldCave_2F \
	--from-group gMapGroup_Temp \
	--to-group gMapGroup_RougeCave \
	--rewrite-script-labels \
	--out "$script_label_tmpdir/repair.json"
python3 "$script_dir/map_relink.py" --root "$script_label_tmpdir" apply --allow-dirty "$script_label_tmpdir/repair.json" >/dev/null
python3 "$script_dir/map_relink.py" --root "$script_label_tmpdir" validate
grep -q 'OldCave_2F_MapScripts::' "$script_label_tmpdir/data/maps/OldCave_2F/scripts.inc"
grep -q 'OldCave_2F_EventScript_Guide::' "$script_label_tmpdir/data/maps/OldCave_2F/scripts.inc"
grep -q 'msgbox OldCave_2F_Text_Guide' "$script_label_tmpdir/data/maps/OldCave_2F/scripts.inc"
grep -q 'OldCave_2 should remain in dialogue' "$script_label_tmpdir/data/maps/OldCave_2F/scripts.inc"

echo "map_asset_relinker fixture test passed: $tmpdir $missing_group_tmpdir $broken_group_tmpdir $broken_map_name_tmpdir $broken_map_id_tmpdir $broken_layout_tmpdir $foreign_layout_tmpdir $broken_mapsec_tmpdir $map_name_anchor_tmpdir $temp_mapsec_tmpdir $tileset_fix_tmpdir $layout_name_fix_tmpdir $duplicate_mapsec_tmpdir $mapsec_name_fix_tmpdir $region_map_tools_tmpdir $broken_warp_tmpdir $match_by_name_tmpdir $match_by_id_tmpdir $duplicate_script_tmpdir $script_label_tmpdir"
