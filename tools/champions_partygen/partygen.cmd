@echo off
setlocal
set SCRIPT_DIR=%~dp0
for %%I in ("%SCRIPT_DIR%..\..") do set REPO_ROOT=%%~fI
if "%~1"=="" (
    cargo run --manifest-path "%SCRIPT_DIR%Cargo.toml" -- --help
    exit /b %ERRORLEVEL%
)
set CMD=%~1
shift
cargo run --manifest-path "%SCRIPT_DIR%Cargo.toml" -- "%CMD%" --rom-repo "%REPO_ROOT%" %*
