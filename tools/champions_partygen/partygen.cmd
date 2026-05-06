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
if /I "%CMD%"=="audit" goto two
if /I "%CMD%"=="logs" goto two
if /I "%CMD%"=="profile" goto two
cargo run --manifest-path "%SCRIPT_DIR%Cargo.toml" -- "%CMD%" --rom-repo "%REPO_ROOT%" %*
exit /b %ERRORLEVEL%
:two
if "%~1"=="" (
    cargo run --manifest-path "%SCRIPT_DIR%Cargo.toml" -- "%CMD%" --rom-repo "%REPO_ROOT%"
    exit /b %ERRORLEVEL%
)
set SUB=%~1
shift
cargo run --manifest-path "%SCRIPT_DIR%Cargo.toml" -- "%CMD%" "%SUB%" --rom-repo "%REPO_ROOT%" %*
exit /b %ERRORLEVEL%
