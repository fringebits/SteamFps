@echo off
call %~dp0Scripts\config.cmd

if "%1"=="client" goto RunClient
if "%1"=="server" goto RunServer
if "%1"=="test" goto RunTests
if "%1"=="kill" goto KillAll
if "%1"=="logs" goto RunLogs

echo "Usage: run (edit|client|server) [options]"

echo "Running editor by default."

goto RunEditor

:: SHIFT doesn't work like it should. Must reference all arguments specifically

:: Unreal command line arguments
:: https://docs.unrealengine.com/latest/INT/Programming/Basics/CommandLineArguments/

:RunEditor
@echo on
start "" "%UNREALENGINE%\Engine\Binaries\Win64\UE4Editor.exe" %PROJECT_FULLPATH% %*
goto Done

:RunClient
start "" "%UNREALENGINE%\Engine\Binaries\Win64\UE4Editor.exe" %PROJECT_FULLPATH% %DEFAULT_MAP% -game -log %2 %3 %4 %5 %6 %7 %8 %9
goto Done

:RunServer
start "" "%UNREALENGINE%\Engine\Binaries\Win64\UE4Editor.exe" %PROJECT_FULLPATH% %DEFAULT_MAP% -server -log %2 %3 %4 %5 %6 %7 %8 %9
goto Done

:RunTests
start "" "%UNREALENGINE%\Engine\Binaries\Win64\UE4Editor.exe" %PROJECT_FULLPATH% -execcmds="Automation RunTests SteamFps.Smoke; Quit" -game -log %2 %3 %4 %5 %6 %7 %8 %9
goto Done

:RunLogs
:: Server log
start "" "%USERPROFILE%\Toolbox\apps\TextAnalysisTool\TextAnalysisTool.NET.exe" Saved\Logs\SteamFps.log
:: Client log
start "" "%USERPROFILE%\Toolbox\apps\TextAnalysisTool\TextAnalysisTool.NET.exe" Saved\Logs\SteamFps_2.log
goto Done

:KillAll
taskkill /F /IM %PROJECT_NAME%.exe
taskkill /F /IM %PROJECT_NAME%Server.exe
taskkill /F /IM UE4Editor.exe
goto Done

:Done
@echo off
echo bye.