SET(CMAKE_C_FLAGS_DEBUG   "-O0 -g ${CMAKE_C_FLAGS_DEBUG} -O0 -g ")
SET(CMAKE_C_FLAGS_RELEASE "-O0 -g ${CMAKE_C_FLAGS_RELEASE} -O0 -g ")

include_directories(
    ${SMEDIA_INC_DIRS}
)

add_library(
    libsxa_blkdev_fat STATIC

    #=== source files =========================================#
    src/sxa_blkdev_fat.c

    #== header files (private) ================================#

    #== header files (public) =================================#
    #${PROJECT_SOURCE_DIR}/sdk/include/sxa/sxa.h
)
