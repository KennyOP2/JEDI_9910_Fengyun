INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
)

IF(${SMEDIA_PLATFORM} STREQUAL win32)
    ADD_DEFINITIONS(
        -DHDMIRX_EXPORTS
    )
ENDIF()

IF(${EXTERNAL_HDMIRX} STREQUAL ENABLE)

    ADD_LIBRARY(hdmirx
        it6604/csc.c
        it6604/io.c
        it6604/timerprocess.c
        it6604/it6604.c
        it6604/mmp_it6604.c
        mmp_hdmirx.c
    )

ELSE()

    ADD_LIBRARY(hdmirx
        it6607/csc.c
        it6607/io.c
        it6607/timerprocess.c
        it6607/it6607.c
        it6607/mmp_it6607.c
        mmp_hdmirx.c
    )
ENDIF()

TARGET_LINK_LIBRARIES(hdmirx
    mmp
    pal
    iic
)