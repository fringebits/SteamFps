@echo off
call %~dp0Scripts\config.cmd
"%UNREALENGINE%\Engine\Binaries\Win64\UE4Editor.exe" %PROJECT_FULLPATH% %* 
