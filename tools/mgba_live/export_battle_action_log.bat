@echo off
setlocal EnableExtensions

if "%~1"=="-h" goto usage
if "%~1"=="--help" goto usage

if "%~1"=="" (
  set "ACTIVE_SESSION_FILE=%USERPROFILE%\.mgba-live-mcp\runtime\active_session"
  if not exist "%ACTIVE_SESSION_FILE%" goto usage
  set /p SESSION=<"%ACTIVE_SESSION_FILE%"
) else (
  set "SESSION=%~1"
)
if "%SESSION%"=="" (
  echo No mGBA Live session id was provided and active_session is empty.
  exit /b 2
)

if "%~2"=="" (
  set "OUT_JSON=%TEMP%\pokeemerald-battle-action-log.json"
) else (
  set "OUT_JSON=%~2"
)

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..\..") do set "PROJECT_ROOT=%%~fI"
set "LUA_SCRIPT=%SCRIPT_DIR%battle_action_log_export.lua"

set "PROJECT_ROOT=%PROJECT_ROOT:\=/%"
set "OUT_JSON=%OUT_JSON:\=/%"
set "LUA_SCRIPT=%LUA_SCRIPT:\=/%"

if "%MGBA_LIVE_CLI%"=="" (
  set "MGBA_LIVE_CLI=mgba-live-cli"
)
if "%TIMEOUT%"=="" (
  set "TIMEOUT=8"
)

set "CODE=_G.BATTLE_ACTION_LOG_ROOT=[=[%PROJECT_ROOT%]=]; _G.BATTLE_ACTION_LOG_OUT=[=[%OUT_JSON%]=]; return dofile([=[%LUA_SCRIPT%]=])"

"%MGBA_LIVE_CLI%" run-lua --session "%SESSION%" --code "%CODE%" --timeout "%TIMEOUT%"
exit /b %ERRORLEVEL%

:usage
echo Usage:
echo   tools\mgba_live\export_battle_action_log.bat [SESSION] [OUT_JSON]
echo.
echo Environment:
echo   MGBA_LIVE_CLI  mgba-live-cli.exe path or command name
echo   TIMEOUT        run-lua timeout in seconds, default 8
echo.
echo Example:
echo   tools\mgba_live\export_battle_action_log.bat
echo   tools\mgba_live\export_battle_action_log.bat smart-gimmick-dmax-double-log-20260606 %%TEMP%%\battle-action-log.json
exit /b 2
