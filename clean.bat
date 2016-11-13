setlocal
set TOOLPATH=%~dp0..\Build

%TOOLPATH%\rm.exe -rf Binaries
%TOOLPATH%\rm.exe -rf Intermediate
%TOOLPATH%\rm.exe -rf Plugins\V1CoverSystemPlugin\Binaries
%TOOLPATH%\rm.exe -rf Plugins\V1CoverSystemPlugin\Intermediate
%TOOLPATH%\rm.exe -f Robogore.sln
%TOOLPATH%\rm.exe -f Robogore.VC.db