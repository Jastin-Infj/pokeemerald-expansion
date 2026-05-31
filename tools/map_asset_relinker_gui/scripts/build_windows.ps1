param(
    [switch]$CoreOnly
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$AppDir = Resolve-Path (Join-Path $ScriptDir "..")
$RepoRoot = Resolve-Path (Join-Path $AppDir "../..")
$CoreDir = Join-Path $RepoRoot "tools/map_asset_relinker_core"
$CoreExe = Join-Path $CoreDir "target/release/map-asset-relinker-core.exe"
$ReleaseDir = Join-Path $AppDir "src-tauri/target/release"
$GuiExe = Join-Path $ReleaseDir "map-asset-relinker-gui.exe"
$PortableDir = Join-Path $AppDir "dist-windows/portable"

cargo build --release --manifest-path (Join-Path $CoreDir "Cargo.toml")

if (!(Test-Path $CoreExe)) {
    throw "Expected executable was not created: $CoreExe"
}

Write-Host "Core executable:"
Write-Host "  $CoreExe"

if ($CoreOnly) {
    exit 0
}

Push-Location $AppDir
try {
    npm install
    npm run tauri build -- --bundles "nsis"
}
finally {
    Pop-Location
}

if (!(Test-Path $GuiExe)) {
    throw "Expected GUI executable was not created: $GuiExe"
}

if (Test-Path $PortableDir) {
    Remove-Item -Recurse -Force $PortableDir
}
New-Item -ItemType Directory -Force $PortableDir | Out-Null
Copy-Item $GuiExe (Join-Path $PortableDir "map-asset-relinker-gui.exe")
Copy-Item $CoreExe (Join-Path $PortableDir "map-asset-relinker-core.exe")
Set-Content -Encoding UTF8 -Path (Join-Path $PortableDir "README.txt") -Value @"
Map Asset Relinker portable Windows build

Run map-asset-relinker-gui.exe directly.

If Windows blocks the executable:
1. Right-click map-asset-relinker-gui.exe.
2. Open Properties.
3. Check Unblock if it is shown.
4. Press OK, then run the executable again.

The MSI bundle is intentionally not the primary distribution path for this tool.
"@

Write-Host ""
Write-Host "Native build output:"
Write-Host "  $CoreExe"

$Patterns = @("*.exe", "*.msi")
foreach ($Pattern in $Patterns) {
    Get-ChildItem -Path $ReleaseDir -Recurse -File -Filter $Pattern -ErrorAction SilentlyContinue |
        ForEach-Object { Write-Host ("  " + $_.FullName) }
}
Write-Host "Portable output:"
Get-ChildItem -Path $PortableDir -File |
    ForEach-Object { Write-Host ("  " + $_.FullName) }
