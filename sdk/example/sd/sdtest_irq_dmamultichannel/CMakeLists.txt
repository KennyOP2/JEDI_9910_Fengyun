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

ADD_EXECUTABLE(sdtest_irq_dmamultichannel
    main.c
    sd.c
    test_f.c
    chkdsk.c
    ${PROJECT_SOURCE_DIR}/sdk/example/sd/sdtest_irq_dmamultichannel/config.h
    )

LINK_DIRECTORIES(
    ${PROJECT_BINARY_DIR}/pal
    ${PROJECT_BINARY_DIR}/fat
    ${PROJECT_BINARY_DIR}/sd
    ${PROJECT_BINARY_DIR}/dma
    ${PROJECT_BINARY_DIR}/mmp
    ${EXAMPLE_LINK_DIRS}
    )

TARGET_LINK_LIBRARIES(sdtest_irq_dmamultichannel
    pal
    fat
    sd
    dma
    mmp
    ${EXAMPLE_LINK_LIBS}
    )
    
IF(${SMEDIA_PLATFORM} STREQUAL freertos)
TARGET_LINK_LIBRARIES(sdtest_irq_dmamultichannel
    intr
    )
ENDIF(${SMEDIA_PLATFORM} STREQUAL freertos)

IF(${SMEDIA_PLATFORM} STREQUAL freertos OR ${SMEDIA_PLATFORM} STREQUAL openrtos)
    ADD_CUSTOM_COMMAND(
        TARGET sdtest_irq_dmamultichannel
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
            -O binary ${CMAKE_CURRENT_BINARY_DIR}/sdtest_irq_dmamultichannel ${CMAKE_CURRENT_BINARY_DIR}/sdtest_irq_dmamultichannel.bin
        )

ENDIF(${SMEDIA_PLATFORM} STREQUAL freertos OR ${SMEDIA_PLATFORM} STREQUAL openrtos)
