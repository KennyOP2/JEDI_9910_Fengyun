INCLUDE(CMakeForceCompiler)

# this one is important
SET(CMAKE_SYSTEM_NAME Generic)

# specify the cross compiler
CMAKE_FORCE_C_COMPILER(sm32-elf-gcc GNU 4)
CMAKE_FORCE_CXX_COMPILER(sm32-elf-g++ GNU)

# specify the preprocessor
SET(CMAKE_C_PREPROCESSOR sm32-elf-cpp)

# where is the target environment
SET(CMAKE_FIND_ROOT_PATH  C:/ITEGCC)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
