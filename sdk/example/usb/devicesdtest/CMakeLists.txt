IF(${SMEDIA_PLATFORM} STREQUAL win32)
    SET(EXAMPLE_LINK_LIBS
        winmm
        )
ELSEIF(${SMEDIA_PLATFORM} STREQUAL freertos OR ${SMEDIA_PLATFORM} STREQUAL openrtos)
    SET(EXAMPLE_LINK_DIRS
        ${PROJECT_BINARY_DIR}/${SMEDIA_PLATFORM}
        )
    SET(EXAMPLE_LINK_LIBS
        ${SMEDIA_PLATFORM}
        )
ENDIF(${SMEDIA_PLATFORM} STREQUAL win32)

INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

ADD_EXECUTABLE(devicesdtest
    main.c
    ramdisk.c
    ramdisk.h
    ${PROJECT_SOURCE_DIR}/sdk/example/usb/devicesdtest/config.h
    )

TARGET_LINK_LIBRARIES(devicesdtest
    ${SMEDIA_FAT_LIB}
    pal
    usb
    dma
    sd
    mmp
    ${EXAMPLE_LINK_LIBS}
    )


IF(${SMEDIA_PLATFORM} STREQUAL freertos OR ${SMEDIA_PLATFORM} STREQUAL openrtos)
    ADD_CUSTOM_COMMAND(
        TARGET devicesdtest
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
            -O binary ${CMAKE_CURRENT_BINARY_DIR}/devicesdtest ${CMAKE_CURRENT_BINARY_DIR}/devicesdtest.bin
        )

ENDIF(${SMEDIA_PLATFORM} STREQUAL freertos OR ${SMEDIA_PLATFORM} STREQUAL openrtos)
