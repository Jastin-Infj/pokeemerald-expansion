param(
    [switch]$CoreOnly
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$AppDir = Resolve-Path (Join-Path $ScriptDir "..")
$RepoRoot = Resolve-Path (Join-Path $AppDir "../..")
$CoreDir = Join-Path $RepoRoot "tools/map_asset_relinker_core"
$CoreExe = Join-Path $CoreDir "target/release/map-asset-relinker-core.exe"

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
    npm run tauri build
}
finally {
    Pop-Location
}

Write-Host ""
Write-Host "Native build output:"
Write-Host "  $CoreExe"

$ReleaseDir = Join-Path $AppDir "src-tauri/target/release"
$Patterns = @("*.exe", "*.msi", "*.AppImage", "*.deb", "*.rpm")
foreach ($Pattern in $Patterns) {
    Get-ChildItem -Path $ReleaseDir -Recurse -File -Filter $Pattern -ErrorAction SilentlyContinue |
        ForEach-Object { Write-Host ("  " + $_.FullName) }
}
