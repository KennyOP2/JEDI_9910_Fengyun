set _PROJECT_TYPE_=hw_grabber
set _WORKSPACE=%cd%

cd %_WORKSPACE%\project\%_PROJECT_TYPE_%
call freertos.cmd
@echo on
cd %_WORKSPACE%\core
del *.c /s
del *.cpp /s
rem cd %_WORKSPACE%\freertos
rem del *.c /s
rem del *.cpp /s
cd %_WORKSPACE%\sdk\share
del *.c /s
del *.cpp /s
cd %_WORKSPACE%\sdk\src
del *.c /s
del *.cpp /s
cd %_WORKSPACE%
rem echo. > %_WORKSPACE%\freertos\CMakeLists.txt
echo. > %_WORKSPACE%\sdk\CMakeLists.txt
ren CMakeLists.txt CMakeLists.txt.bak
ren CMakeLists_sdk.txt CMakeLists.txt
del %_WORKSPACE%\project\%_PROJECT_TYPE_%\freertos\*.* /q
del %_WORKSPACE%\project\update_nor\freertos\*.* /q
del %_WORKSPACE%\project\bootloader\freertos\*.* /q
del %_WORKSPACE%\project\hw_grabber\.repos\ntfs-3g\*.c /s/q
del %_WORKSPACE%\project\update_nor\.repos\ntfs-3g\*.c /s/q
del %_WORKSPACE%\project\bootloader\.repos\ntfs-3g\*.c /s/q
RMDIR %_WORKSPACE%\project\%_PROJECT_TYPE_%\freertos\CMakeFiles /s/q
RMDIR %_WORKSPACE%\project\%_PROJECT_TYPE_%\freertos\core /s/q
RMDIR %_WORKSPACE%\project\%_PROJECT_TYPE_%\freertos\freertos /s/q
RMDIR %_WORKSPACE%\project\%_PROJECT_TYPE_%\freertos\project /s/q
RMDIR %_WORKSPACE%\project\%_PROJECT_TYPE_%\freertos\sdk /s/q
RMDIR %_WORKSPACE%\project\update_nor\freertos\CMakeFiles /s/q
RMDIR %_WORKSPACE%\project\update_nor\freertos\core /s/q
RMDIR %_WORKSPACE%\project\update_nor\freertos\freertos /s/q
RMDIR %_WORKSPACE%\project\update_nor\freertos\project /s/q
RMDIR %_WORKSPACE%\project\update_nor\freertos\sdk /s/q
RMDIR %_WORKSPACE%\project\bootloader\freertos\CMakeFiles /s/q
RMDIR %_WORKSPACE%\project\bootloader\freertos\core /s/q
RMDIR %_WORKSPACE%\project\bootloader\freertos\freertos /s/q
RMDIR %_WORKSPACE%\project\bootloader\freertos\project\CMakeFiles /s/q
RMDIR %_WORKSPACE%\project\bootloader\freertos\project\bootloader\CMakeFiles /s/q
RMDIR %_WORKSPACE%\project\bootloader\freertos\sdk /s/q
cd %_WORKSPACE%\project\%_PROJECT_TYPE_%
call freertos.cmd
@echo on
cd ..\
del %_WORKSPACE%\project\%_PROJECT_TYPE_%\freertos\*.* /q
del %_WORKSPACE%\project\update_nor\freertos\*.* /q
del %_WORKSPACE%\project\bootloader\freertos\*.* /q
RMDIR %_WORKSPACE%\project\%_PROJECT_TYPE_%\freertos\CMakeFiles /s/q
RMDIR %_WORKSPACE%\project\%_PROJECT_TYPE_%\freertos\freertos /s/q
RMDIR %_WORKSPACE%\project\%_PROJECT_TYPE_%\freertos\project /s/q
RMDIR %_WORKSPACE%\project\%_PROJECT_TYPE_%\freertos\sdk /s/q
RMDIR %_WORKSPACE%\project\update_nor\freertos\CMakeFiles /s/q
RMDIR %_WORKSPACE%\project\update_nor\freertos\freertos /s/q
RMDIR %_WORKSPACE%\project\update_nor\freertos\project /s/q
RMDIR %_WORKSPACE%\project\update_nor\freertos\sdk /s/q
pause