@echo off
if not exist win32-config.cmd call win32-setting.cmd
call win32-config.cmd
call ..\version.cmd
mkdir win32
cd win32
set CMAKE_ROOT=..\..\..\tool
%CMAKE_ROOT%\bin\cmake.exe -DCMAKE_USE_RELATIVE_PATHS:BOOL=ON -G%SMEDIA_COMPILER% -DSDK_MAJOR_VERSION=%SDK_MAJOR_VERSION% -DSDK_MINOR_VERSION=%SDK_MINOR_VERSION% -DSMEDIA_PLATFORM=win32 -C..\..\..\win32\init.cmake ..\..\..
if errorlevel 1 pause
