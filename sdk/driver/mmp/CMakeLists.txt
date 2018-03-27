INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    )

IF(${SMEDIA_PLATFORM} STREQUAL win32)
    ADD_LIBRARY(mmp
        host_clock.c
        )
ENDIF(${SMEDIA_PLATFORM} STREQUAL win32)

ADD_LIBRARY(mmp
    ahb.c
    clock.c
    gpio.c
    host.c
    mem.c
    sys.c
    )

IF(${SMEDIA_PLATFORM} STREQUAL win32)
    TARGET_LINK_LIBRARIES(mmp
        setupapi
        usb2spi
        )
ENDIF(${SMEDIA_PLATFORM} STREQUAL win32)
