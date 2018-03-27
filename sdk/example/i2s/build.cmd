@echo off
cd ..\..\..\build
if not exist freertos-config.cmd call freertos-setting.cmd
call freertos-config.cmd
call version.cmd
@echo -- SDK Version: %SDK_MAJOR_VERSION%.%SDK_MINOR_VERSION%
for /f %%a in ('cd') do set CMAKE_ROOT=%%a\..\tool
for /f "tokens=3 delims=() " %%a in ('find "CMAKE_FIND_ROOT_PATH " ..\freertos\toolchain.cmake') do set toolchain=%%a
set toolchain=%toolchain:/=\%
set PATH=%CMAKE_ROOT%\bin;%toolchain%\bin;%PATH%
if not exist freertos mkdir freertos
cd freertos
if exist app\jedi del app\jedi
if exist sdk\example\i2s\test_i2s     del sdk\example\i2s\test_i2s
if exist sdk\example\i2s\test_i2s.bin del sdk\example\i2s\test_i2s.bin
cmake.exe -G"Unix Makefiles" -DCONFIG_HAVE_USBD=1 -DUSB_DEVICE=%USB_DEVICE% -DUSB_STORAGE=%USB_STORAGE% -DSD_STORAGE=%SD_STORAGE% -DSDK_MAJOR_VERSION=%SDK_MAJOR_VERSION% -DSDK_MINOR_VERSION=%SDK_MINOR_VERSION% -DSMEDIA_PLATFORM=freertos -DSYSTEM_FAT_TYPE=%SYSTEM_FAT_TYPE% -DCMAKE_BUILD_TYPE=%CMAKE_BUILD_TYPE% -DCMAKE_TOOLCHAIN_FILE=..\..\freertos\toolchain.cmake ..\..
rem make VERBOSE=1
set CYGWIN=nodosfilewarning
make
cd ../
call copy_audio_codec.cmd
@if not defined NO_PAUSE pause
