call %~dp0Scripts\config.cmd
call "%UNREALENGINE%\Engine\Build\BatchFiles\Build.bat" %V1PROJECT%Editor Win64 Development %PROJECT_FULLPATH% -projectfiles -project=%PROJECT_FULLPATH% -game