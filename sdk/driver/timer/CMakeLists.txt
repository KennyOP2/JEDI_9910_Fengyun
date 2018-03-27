INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    )

IF(${SMEDIA_PLATFORM} STREQUAL win32)
    ADD_DEFINITIONS(
        -DTIMER_EXPORTS
        )

ENDIF(${SMEDIA_PLATFORM} STREQUAL win32)

ADD_LIBRARY(timer
    mmp_timer.c
    ${PROJECT_SOURCE_DIR}/sdk/include/timer/timer_reg.h
    ${PROJECT_SOURCE_DIR}/sdk/include/mmp_timer.h
    )

#IF(${SMEDIA_PLATFORM} STREQUAL win32)
    TARGET_LINK_LIBRARIES(timer
        mmp
        pal
        )

#ENDIF(${SMEDIA_PLATFORM} STREQUAL win32)
