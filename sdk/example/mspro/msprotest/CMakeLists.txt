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

ADD_EXECUTABLE(msprotest
    main.c
    mspro.c
    test_f.c
    chkdsk.c
    ${PROJECT_SOURCE_DIR}/sdk/example/mspro/msprotest/config.h
    )

LINK_DIRECTORIES(
    ${PROJECT_BINARY_DIR}/pal
    ${PROJECT_BINARY_DIR}/fat
    ${PROJECT_BINARY_DIR}/mspro
    ${PROJECT_BINARY_DIR}/dma
    ${PROJECT_BINARY_DIR}/mmp
    ${EXAMPLE_LINK_DIRS}
    )

TARGET_LINK_LIBRARIES(msprotest
    pal
    ${SMEDIA_FAT_LIB}
    mspro
    dma
    mmp
    ${EXAMPLE_LINK_LIBS}
    )
    
IF(${SMEDIA_PLATFORM} STREQUAL freertos)
TARGET_LINK_LIBRARIES(msprotest
    intr
    )
ENDIF(${SMEDIA_PLATFORM} STREQUAL freertos)

IF(${SMEDIA_PLATFORM} STREQUAL freertos OR ${SMEDIA_PLATFORM} STREQUAL openrtos)
    ADD_CUSTOM_COMMAND(
        TARGET msprotest
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
            -O binary ${CMAKE_CURRENT_BINARY_DIR}/msprotest ${CMAKE_CURRENT_BINARY_DIR}/msprotest.bin
        )

ENDIF(${SMEDIA_PLATFORM} STREQUAL freertos OR ${SMEDIA_PLATFORM} STREQUAL openrtos)
