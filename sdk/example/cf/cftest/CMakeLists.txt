IF(${SMEDIA_PLATFORM} STREQUAL win32)
    SET(EXAMPLE_LINK_LIBS
        winmm
        )
ELSEIF(${SMEDIA_PLATFORM} STREQUAL freertos)
    SET(EXAMPLE_LINK_DIRS
        ${PROJECT_BINARY_DIR}/freertos
        )
    SET(EXAMPLE_LINK_LIBS
        freertos
        )
ELSEIF(${SMEDIA_PLATFORM} STREQUAL openrtos)
    SET(EXAMPLE_LINK_DIRS
        ${PROJECT_BINARY_DIR}/openrtos
        )
    SET(EXAMPLE_LINK_LIBS
        openrtos
        )
ENDIF(${SMEDIA_PLATFORM} STREQUAL win32)

INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

ADD_EXECUTABLE(cftest
    main.c
    cf.c
    test_f.c
    chkdsk.c
    ${PROJECT_SOURCE_DIR}/sdk/example/cf/cftest/config.h
    )

TARGET_LINK_LIBRARIES(cftest
    pal
    ${SMEDIA_FAT_LIB}
    cf
    dma
    mmp
    ${EXAMPLE_LINK_LIBS}
    )

TARGET_LINK_LIBRARIES(cftest
    dma
    mmp
    ${EXAMPLE_LINK_LIBS}
    )
    
IF(${SMEDIA_PLATFORM} STREQUAL freertos)
TARGET_LINK_LIBRARIES(cftest
    intr
    )
ENDIF(${SMEDIA_PLATFORM} STREQUAL freertos)

IF(${SMEDIA_PLATFORM} STREQUAL freertos OR ${SMEDIA_PLATFORM} STREQUAL openrtos)
    ADD_CUSTOM_COMMAND(
        TARGET cftest
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
            -O binary ${CMAKE_CURRENT_BINARY_DIR}/cftest ${CMAKE_CURRENT_BINARY_DIR}/cftest.bin
        )

ENDIF(${SMEDIA_PLATFORM} STREQUAL freertos OR ${SMEDIA_PLATFORM} STREQUAL openrtos)
