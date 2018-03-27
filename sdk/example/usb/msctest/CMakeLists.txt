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

ADD_EXECUTABLE(msctest
    main.c
    msc.c
    test_f.c
    chkdsk.c
    ${PROJECT_SOURCE_DIR}/sdk/example/usb/msctest/config.h
    )

TARGET_LINK_LIBRARIES(msctest
    pal
    fat
    msc
    usb
    mmp
    ${EXAMPLE_LINK_LIBS}
    )
    
IF(${SMEDIA_PLATFORM} STREQUAL freertos)
TARGET_LINK_LIBRARIES(msctest
    intr
    )
ENDIF(${SMEDIA_PLATFORM} STREQUAL freertos)

IF(${SMEDIA_PLATFORM} STREQUAL freertos OR ${SMEDIA_PLATFORM} STREQUAL openrtos)
    ADD_CUSTOM_COMMAND(
        TARGET msctest
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
            -O binary ${CMAKE_CURRENT_BINARY_DIR}/msctest ${CMAKE_CURRENT_BINARY_DIR}/msctest.bin
        )

ENDIF(${SMEDIA_PLATFORM} STREQUAL freertos OR ${SMEDIA_PLATFORM} STREQUAL openrtos)
