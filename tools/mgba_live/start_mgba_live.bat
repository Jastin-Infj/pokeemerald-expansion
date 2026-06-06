@echo off
setlocal EnableExtensions

if "%~1"=="-h" goto usage
if "%~1"=="--help" goto usage

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..\..") do set "PROJECT_ROOT=%%~fI"

if "%~1"=="" (
  set "SESSION=manual-mgba"
) else (
  set "SESSION=%~1"
)

if "%~2"=="" (
  if "%FPS_TARGET%"=="" (
    set "FPS=120"
  ) else (
    set "FPS=%FPS_TARGET%"
  )
) else (
  set "FPS=%~2"
)

if "%~3"=="" (
  if "%ROM%"=="" (
    set "ROM_PATH=%PROJECT_ROOT%\pokeemerald.gba"
  ) else (
    set "ROM_PATH=%ROM%"
  )
) else (
  set "ROM_PATH=%~3"
)

if "%READY_TIMEOUT%"=="" (
  set "READY_TIMEOUT=20"
)
if "%VIDEO_SYNC%"=="" (
  set "VIDEO_SYNC=1"
)
if "%BATTLE_ACTION_LOG_AUTOSAVE%"=="" (
  set "BATTLE_ACTION_LOG_AUTOSAVE=1"
)
if "%MGBA_LIVE_CLI%"=="" (
  set "MGBA_LIVE_CLI=mgba-live-cli"
)

if not exist "%ROM_PATH%" (
  echo ROM not found: %ROM_PATH%
  exit /b 2
)

set "SCRIPT_ARG="
if not "%BATTLE_ACTION_LOG_AUTOSAVE%"=="0" (
  set "AUTOSAVE_SCRIPT=%SCRIPT_DIR%battle_action_log_autosave.lua"
  if "%POKEEMERALD_ROOT%"=="" set "POKEEMERALD_ROOT=%PROJECT_ROOT%"
  if "%BATTLE_ACTION_LOG_EXPORTER%"=="" set "BATTLE_ACTION_LOG_EXPORTER=%SCRIPT_DIR%battle_action_log_export.lua"
  if "%BATTLE_ACTION_LOG_OUT%"=="" set "BATTLE_ACTION_LOG_OUT=%TEMP%\%SESSION%-battle-action-log-autosave.json"
  if "%BATTLE_ACTION_LOG_AUTOSAVE_INTERVAL%"=="" set "BATTLE_ACTION_LOG_AUTOSAVE_INTERVAL=120"
  set "SCRIPT_ARG=--script %AUTOSAVE_SCRIPT%"
)

if "%VIDEO_SYNC%"=="0" (
  "%MGBA_LIVE_CLI%" start --rom "%ROM_PATH%" --session-id "%SESSION%" --fps-target "%FPS%" --ready-timeout "%READY_TIMEOUT%" %SCRIPT_ARG%
) else (
  "%MGBA_LIVE_CLI%" start --rom "%ROM_PATH%" --session-id "%SESSION%" --fps-target "%FPS%" --ready-timeout "%READY_TIMEOUT%" --config videoSync=1 %SCRIPT_ARG%
)
exit /b %ERRORLEVEL%

:usage
echo Usage:
echo   tools\mgba_live\start_mgba_live.bat [SESSION] [FPS] [ROM]
echo.
echo Environment:
echo   MGBA_LIVE_CLI  mgba-live-cli.exe path or command name
echo   FPS_TARGET     FPS target, default 120
echo   ROM            ROM path, default .\pokeemerald.gba
echo   READY_TIMEOUT  bridge ready timeout in seconds, default 20
echo   VIDEO_SYNC     set to 0 to skip videoSync=1, default 1
echo   BATTLE_ACTION_LOG_AUTOSAVE
echo                  set to 0 to skip battle action log autosave, default 1
echo   BATTLE_ACTION_LOG_OUT
echo                  autosave output, default %%TEMP%%\SESSION-battle-action-log-autosave.json
echo   BATTLE_ACTION_LOG_AUTOSAVE_INTERVAL
echo                  autosave interval in frames, default 120
echo.
echo Examples:
echo   tools\mgba_live\start_mgba_live.bat
echo   tools\mgba_live\start_mgba_live.bat manual-ai-log 120
exit /b 2
