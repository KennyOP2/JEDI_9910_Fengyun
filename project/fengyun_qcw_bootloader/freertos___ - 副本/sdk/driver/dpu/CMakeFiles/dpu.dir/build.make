# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.1

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/tool/bin/cmake.exe

# The command to remove a file.
RM = E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/tool/bin/cmake.exe -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos

# Include any dependencies generated for this target.
include sdk/driver/dpu/CMakeFiles/dpu.dir/depend.make

# Include the progress variables for this target.
include sdk/driver/dpu/CMakeFiles/dpu.dir/progress.make

# Include the compile flags for this target's objects.
include sdk/driver/dpu/CMakeFiles/dpu.dir/flags.make

sdk/driver/dpu/CMakeFiles/dpu.dir/mmp_dpu.c.obj: sdk/driver/dpu/CMakeFiles/dpu.dir/flags.make
sdk/driver/dpu/CMakeFiles/dpu.dir/mmp_dpu.c.obj: ../../../sdk/driver/dpu/mmp_dpu.c
	$(CMAKE_COMMAND) -E cmake_progress_report E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object sdk/driver/dpu/CMakeFiles/dpu.dir/mmp_dpu.c.obj"
	cd E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/dpu && C:/ITEGCC/bin/sm32-elf-gcc.exe  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/dpu.dir/mmp_dpu.c.obj   -c E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/sdk/driver/dpu/mmp_dpu.c

sdk/driver/dpu/CMakeFiles/dpu.dir/mmp_dpu.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/dpu.dir/mmp_dpu.c.i"
	cd E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/dpu && C:/ITEGCC/bin/sm32-elf-gcc.exe  $(C_DEFINES) $(C_FLAGS) -E E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/sdk/driver/dpu/mmp_dpu.c > CMakeFiles/dpu.dir/mmp_dpu.c.i

sdk/driver/dpu/CMakeFiles/dpu.dir/mmp_dpu.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/dpu.dir/mmp_dpu.c.s"
	cd E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/dpu && C:/ITEGCC/bin/sm32-elf-gcc.exe  $(C_DEFINES) $(C_FLAGS) -S E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/sdk/driver/dpu/mmp_dpu.c -o CMakeFiles/dpu.dir/mmp_dpu.c.s

sdk/driver/dpu/CMakeFiles/dpu.dir/mmp_dpu.c.obj.requires:
.PHONY : sdk/driver/dpu/CMakeFiles/dpu.dir/mmp_dpu.c.obj.requires

sdk/driver/dpu/CMakeFiles/dpu.dir/mmp_dpu.c.obj.provides: sdk/driver/dpu/CMakeFiles/dpu.dir/mmp_dpu.c.obj.requires
	$(MAKE) -f sdk/driver/dpu/CMakeFiles/dpu.dir/build.make sdk/driver/dpu/CMakeFiles/dpu.dir/mmp_dpu.c.obj.provides.build
.PHONY : sdk/driver/dpu/CMakeFiles/dpu.dir/mmp_dpu.c.obj.provides

sdk/driver/dpu/CMakeFiles/dpu.dir/mmp_dpu.c.obj.provides.build: sdk/driver/dpu/CMakeFiles/dpu.dir/mmp_dpu.c.obj

# Object files for target dpu
dpu_OBJECTS = \
"CMakeFiles/dpu.dir/mmp_dpu.c.obj"

# External object files for target dpu
dpu_EXTERNAL_OBJECTS =

lib/libdpu.a: sdk/driver/dpu/CMakeFiles/dpu.dir/mmp_dpu.c.obj
lib/libdpu.a: sdk/driver/dpu/CMakeFiles/dpu.dir/build.make
lib/libdpu.a: sdk/driver/dpu/CMakeFiles/dpu.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C static library ../../../lib/libdpu.a"
	cd E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/dpu && $(CMAKE_COMMAND) -P CMakeFiles/dpu.dir/cmake_clean_target.cmake
	cd E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/dpu && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/dpu.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
sdk/driver/dpu/CMakeFiles/dpu.dir/build: lib/libdpu.a
.PHONY : sdk/driver/dpu/CMakeFiles/dpu.dir/build

sdk/driver/dpu/CMakeFiles/dpu.dir/requires: sdk/driver/dpu/CMakeFiles/dpu.dir/mmp_dpu.c.obj.requires
.PHONY : sdk/driver/dpu/CMakeFiles/dpu.dir/requires

sdk/driver/dpu/CMakeFiles/dpu.dir/clean:
	cd E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/dpu && $(CMAKE_COMMAND) -P CMakeFiles/dpu.dir/cmake_clean.cmake
.PHONY : sdk/driver/dpu/CMakeFiles/dpu.dir/clean

sdk/driver/dpu/CMakeFiles/dpu.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/sdk/driver/dpu E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/dpu E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/dpu/CMakeFiles/dpu.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : sdk/driver/dpu/CMakeFiles/dpu.dir/depend

