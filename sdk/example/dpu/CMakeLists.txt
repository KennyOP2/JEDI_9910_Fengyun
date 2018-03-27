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

ADD_EXECUTABLE(dputest
    main.c
    aes.c
    des.c
    sha2.c
    )

LINK_DIRECTORIES(
	${PROJECT_BINARY_DIR}/dpu
    ${PROJECT_BINARY_DIR}/pal
    ${PROJECT_BINARY_DIR}/mmp
    ${EXAMPLE_LINK_DIRS}
    )

IF(${SMEDIA_PLATFORM} STREQUAL openrtos)
TARGET_LINK_LIBRARIES(dputest
    dpu
    pal
    mmp
    ${EXAMPLE_LINK_LIBS}
    ith
    )
ELSEIF(${SMEDIA_PLATFORM} STREQUAL freertos)
	TARGET_LINK_LIBRARIES(dputest
	    dpu
	    pal
	    mmp
	    ${EXAMPLE_LINK_LIBS}
	    intr
	    )
ELSEIF(${SMEDIA_PLATFORM} STREQUAL win32)
	TARGET_LINK_LIBRARIES(dputest
	    dpu
	    pal
	    mmp
	    ${EXAMPLE_LINK_LIBS}
	    )
ENDIF(${SMEDIA_PLATFORM} STREQUAL openrtos)

IF(${SMEDIA_PLATFORM} STREQUAL freertos OR ${SMEDIA_PLATFORM} STREQUAL openrtos)
    ADD_CUSTOM_COMMAND(
        TARGET dputest
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
            -O binary ${CMAKE_CURRENT_BINARY_DIR}/dputest ${CMAKE_CURRENT_BINARY_DIR}/dputest.bin
        )

ENDIF(${SMEDIA_PLATFORM} STREQUAL freertos OR ${SMEDIA_PLATFORM} STREQUAL openrtos)
