#SET(CMAKE_C_FLAGS_DEBUG   "-O0 -g ${CMAKE_C_FLAGS_DEBUG} -O0 -g ")
#SET(CMAKE_C_FLAGS_RELEASE "-O0 -g ${CMAKE_C_FLAGS_RELEASE} -O0 -g ")

include_directories(
    ${SMEDIA_INC_DIRS}
    inc
    ${PROJECT_SOURCE_DIR}/sdk/include/sxa
)

ADD_DEFINITIONS(
#   -DCONFIG_SXA_DEBUG_PERFORMANCE=1
)

add_library(
    libsxa_blkdev_usb STATIC

    #=== source files =========================================#
    src/buffer.c
    src/usbdrv.c

    #== header files (private) ================================#

    #== header files (public) =================================#
    #${PROJECT_SOURCE_DIR}/sdk/include/sxa/sxa.h
)
