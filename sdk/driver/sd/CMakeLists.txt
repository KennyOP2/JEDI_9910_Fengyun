INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/include/sd
    )

IF(${SMEDIA_PLATFORM} STREQUAL win32)
    ADD_DEFINITIONS(
        -DSD_EXPORTS
        )

ENDIF(${SMEDIA_PLATFORM} STREQUAL win32)

ADD_LIBRARY(sd
    mmp_sd.c
    sd_hw.c
    sd.c
    mmc.c
    common.c
    ${PROJECT_SOURCE_DIR}/sdk/include/sd/config.h
    ${PROJECT_SOURCE_DIR}/sdk/include/sd/sd_error.h
    ${PROJECT_SOURCE_DIR}/sdk/include/sd/sd_reg.h
    ${PROJECT_SOURCE_DIR}/sdk/include/sd/sd_hw.h
    ${PROJECT_SOURCE_DIR}/sdk/include/sd/sd.h
    ${PROJECT_SOURCE_DIR}/sdk/include/mmp_sd.h
    )

TARGET_LINK_LIBRARIES(sd
    mmp
    dma
    )

IF(${SMEDIA_PLATFORM} STREQUAL win32)
    TARGET_LINK_LIBRARIES(sd
        winmm
        )
ENDIF(${SMEDIA_PLATFORM} STREQUAL win32)
