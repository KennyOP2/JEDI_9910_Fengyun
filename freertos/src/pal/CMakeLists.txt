IF(${CONFIG_HAVE_NTFS})
    SET(SMEDIA_INC_DIRS
        ${SMEDIA_INC_DIRS}
        ${PROJECT_SOURCE_DIR}/project/${PROJECT_TYPE}/.repos/ntfs-3g/src/port
    )
ENDIF()

INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
)

ADD_LIBRARY(pal STATIC
    config.h
    error.c
    event.c
    file.c
    heap.c
    init.c
    init.h
    msgq.c
    mutex.c
    print.c
    rand.c
    stat.c
    string.c
    thread.c
    timer.c
    sys.c
)
#ADD_LIBRARY(pal_bootloader STATIC
#    config.h
#    error.c
#    event.c
#    file.c
#    heap.c
#    init.c
#    init.h
#    msgq.c
#    mutex.c
#    print.c
#    rand.c
#    stat.c
#    string.c
#    thread.c
#    timer.c
#    sys.c
#    )
#SET_TARGET_PROPERTIES(pal_bootloader PROPERTIES COMPILE_FLAGS "-DBUILDING_BOOTLOADER")

IF(${SYSTEM_FAT_TYPE} STREQUAL HAVE_FAT)
    TARGET_LINK_LIBRARIES(pal
        fat
    )
ENDIF()