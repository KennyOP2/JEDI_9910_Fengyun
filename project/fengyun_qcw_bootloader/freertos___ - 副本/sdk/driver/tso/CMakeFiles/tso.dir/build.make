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
include sdk/driver/tso/CMakeFiles/tso.dir/depend.make

# Include the progress variables for this target.
include sdk/driver/tso/CMakeFiles/tso.dir/progress.make

# Include the compile flags for this target's objects.
include sdk/driver/tso/CMakeFiles/tso.dir/flags.make

sdk/driver/tso/CMakeFiles/tso.dir/tso.c.obj: sdk/driver/tso/CMakeFiles/tso.dir/flags.make
sdk/driver/tso/CMakeFiles/tso.dir/tso.c.obj: ../../../sdk/driver/tso/tso.c
	$(CMAKE_COMMAND) -E cmake_progress_report E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object sdk/driver/tso/CMakeFiles/tso.dir/tso.c.obj"
	cd E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/tso && C:/ITEGCC/bin/sm32-elf-gcc.exe  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/tso.dir/tso.c.obj   -c E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/sdk/driver/tso/tso.c

sdk/driver/tso/CMakeFiles/tso.dir/tso.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/tso.dir/tso.c.i"
	cd E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/tso && C:/ITEGCC/bin/sm32-elf-gcc.exe  $(C_DEFINES) $(C_FLAGS) -E E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/sdk/driver/tso/tso.c > CMakeFiles/tso.dir/tso.c.i

sdk/driver/tso/CMakeFiles/tso.dir/tso.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/tso.dir/tso.c.s"
	cd E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/tso && C:/ITEGCC/bin/sm32-elf-gcc.exe  $(C_DEFINES) $(C_FLAGS) -S E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/sdk/driver/tso/tso.c -o CMakeFiles/tso.dir/tso.c.s

sdk/driver/tso/CMakeFiles/tso.dir/tso.c.obj.requires:
.PHONY : sdk/driver/tso/CMakeFiles/tso.dir/tso.c.obj.requires

sdk/driver/tso/CMakeFiles/tso.dir/tso.c.obj.provides: sdk/driver/tso/CMakeFiles/tso.dir/tso.c.obj.requires
	$(MAKE) -f sdk/driver/tso/CMakeFiles/tso.dir/build.make sdk/driver/tso/CMakeFiles/tso.dir/tso.c.obj.provides.build
.PHONY : sdk/driver/tso/CMakeFiles/tso.dir/tso.c.obj.provides

sdk/driver/tso/CMakeFiles/tso.dir/tso.c.obj.provides.build: sdk/driver/tso/CMakeFiles/tso.dir/tso.c.obj

# Object files for target tso
tso_OBJECTS = \
"CMakeFiles/tso.dir/tso.c.obj"

# External object files for target tso
tso_EXTERNAL_OBJECTS =

lib/libtso.a: sdk/driver/tso/CMakeFiles/tso.dir/tso.c.obj
lib/libtso.a: sdk/driver/tso/CMakeFiles/tso.dir/build.make
lib/libtso.a: sdk/driver/tso/CMakeFiles/tso.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C static library ../../../lib/libtso.a"
	cd E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/tso && $(CMAKE_COMMAND) -P CMakeFiles/tso.dir/cmake_clean_target.cmake
	cd E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/tso && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/tso.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
sdk/driver/tso/CMakeFiles/tso.dir/build: lib/libtso.a
.PHONY : sdk/driver/tso/CMakeFiles/tso.dir/build

sdk/driver/tso/CMakeFiles/tso.dir/requires: sdk/driver/tso/CMakeFiles/tso.dir/tso.c.obj.requires
.PHONY : sdk/driver/tso/CMakeFiles/tso.dir/requires

sdk/driver/tso/CMakeFiles/tso.dir/clean:
	cd E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/tso && $(CMAKE_COMMAND) -P CMakeFiles/tso.dir/cmake_clean.cmake
.PHONY : sdk/driver/tso/CMakeFiles/tso.dir/clean

sdk/driver/tso/CMakeFiles/tso.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/sdk/driver/tso E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/tso E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/tso/CMakeFiles/tso.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : sdk/driver/tso/CMakeFiles/tso.dir/depend

