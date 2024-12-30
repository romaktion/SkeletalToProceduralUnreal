:: Copyright 2024 romaktion@gmail.com. All Rights Reserved.

@echo off

set ThisDir=%~dp0

set UEDir=%1
set UEDir=%UEDir:"=%

echo UEDir=%UEDir%

set DestDir=%ThisDir%
set UEUATDir=%UEDir%\Engine\Build\BatchFiles\RunUAT.bat
set UECmdDir=%UEDir%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe

call "%UEUATDir%" BuildPlugin -Plugin="%ThisDir%SkeletalToProcedural.uplugin" -Package="%DestDir%\SkeletalToProcedural" -Rocket
if %errorlevel% neq 0 pause
