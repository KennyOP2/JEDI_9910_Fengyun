INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
)

IF(${SMEDIA_PLATFORM} STREQUAL win32)
    ADD_DEFINITIONS(
        -DHDMITX_EXPORTS
    )
ENDIF()

IF(${HDMITX_TYPE} STREQUAL IT66121)
    ADD_LIBRARY(hdmitx
        IT66121/hdmitx_drv.c
        IT66121/hdmitx_sys.c
    )
ELSE()
    ADD_LIBRARY(hdmitx
        IT6613/hdmitx_drv.c
        IT6613/hdmitx_sys.c
    )
ENDIF()

ADD_LIBRARY(hdmitx
    csc.c
    mmp_hdmitx.c
)

TARGET_LINK_LIBRARIES(hdmitx
    pal
    mmp
    iic
)
