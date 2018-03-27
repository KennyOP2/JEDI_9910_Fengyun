@echo off
call freertos-config.cmd
for /f "tokens=3 delims=() " %%a in ('find "CMAKE_FIND_ROOT_PATH " ..\..\freertos\toolchain.cmake') do set toolchain=%%a
set toolchain=%toolchain:/=\%
set CYGWIN=nodosfilewarning
cd ../../tool/bin
%toolchain%\bin\sm32-elf-addr2line -e ..\..\project\%PROJECT_TYPE%\freertos\project\%PROJECT_TYPE%\jedi 0x0013e598
pause