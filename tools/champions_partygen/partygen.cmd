@echo off
setlocal
set SCRIPT_DIR=%~dp0
for %%I in ("%SCRIPT_DIR%..\..") do set REPO_ROOT=%%~fI
if "%~1"=="" (
    cargo run --manifest-path "%SCRIPT_DIR%Cargo.toml" -- --help
    exit /b %ERRORLEVEL%
)
cargo run --manifest-path "%SCRIPT_DIR%Cargo.toml" -- %* --rom-repo "%REPO_ROOT%"
exit /b %ERRORLEVEL%
