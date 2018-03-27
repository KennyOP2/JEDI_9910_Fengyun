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

ADD_EXECUTABLE(nortest
    normain.c
#    test/test_f.c
#    test/testdrv_f.c
#    test/testport_f.c
#    test/chkdsk.c
#   ../../../../dpf/mmc_smedia.c
    )

LINK_DIRECTORIES(
    ${PROJECT_BINARY_DIR}/pal
    ${PROJECT_BINARY_DIR}/fat
#    ${PROJECT_BINARY_DIR}/mmc
#    ${PROJECT_BINARY_DIR}/rtc
    ${PROJECT_BINARY_DIR}/mmp
    ${PROJECT_BINARY_DIR}/dma
    ${PROJECT_BINARY_DIR}/nor
	${PROJECT_BINARY_DIR}/spi
    ${EXAMPLE_LINK_DIRS}
    )

TARGET_LINK_LIBRARIES(nortest
#    pal
#    ${SMEDIA_FAT_LIB}
#    mmc
    nor
    pal
    ${SMEDIA_FAT_LIB}
    mmp
#    rtc
    dma
	spi
    ${EXAMPLE_LINK_LIBS}
    )
    
IF(${SMEDIA_PLATFORM} STREQUAL freertos)   
TARGET_LINK_LIBRARIES(nortest            
    intr                                   
    )                                      
ENDIF(${SMEDIA_PLATFORM} STREQUAL freertos)

IF(${SMEDIA_PLATFORM} STREQUAL freertos OR ${SMEDIA_PLATFORM} STREQUAL openrtos)
    ADD_CUSTOM_COMMAND(
        TARGET nortest
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
            -O binary ${CMAKE_CURRENT_BINARY_DIR}/nortest ${CMAKE_CURRENT_BINARY_DIR}/nortest.bin
        )

ENDIF(${SMEDIA_PLATFORM} STREQUAL freertos OR ${SMEDIA_PLATFORM} STREQUAL openrtos)
