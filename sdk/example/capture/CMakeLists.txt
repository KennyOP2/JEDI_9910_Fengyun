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
ENDIF()

INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/sdk/driver/capture
)

ADD_EXECUTABLE(capture_test
    main.c
)

LINK_DIRECTORIES(
    ${PROJECT_BINARY_DIR}/cap
    ${PROJECT_BINARY_DIR}/pal
    ${PROJECT_BINARY_DIR}/mmp
    ${EXAMPLE_LINK_DIRS}
)

TARGET_LINK_LIBRARIES(capture_test
    cap
    pal
    ${SMEDIA_FAT_LIB}
    mmp
    iic
    it6607
    isp
    #hdmirx
    ${EXAMPLE_LINK_LIBS}
)

IF(${SMEDIA_PLATFORM} STREQUAL openrtos)
    TARGET_LINK_LIBRARIES(capture_test
        ith
        ${EXAMPLE_LINK_LIBS}
    )
ELSEIF(${SMEDIA_PLATFORM} STREQUAL freertos)
    TARGET_LINK_LIBRARIES(capture_test
        intr
        ${EXAMPLE_LINK_LIBS}
    )
ENDIF()

IF(${SMEDIA_PLATFORM} STREQUAL freertos OR ${SMEDIA_PLATFORM} STREQUAL openrtos)
    ADD_CUSTOM_COMMAND(
        TARGET capture_test
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
            -O binary ${CMAKE_CURRENT_BINARY_DIR}/capture_test ${CMAKE_CURRENT_BINARY_DIR}/capture_test.bin
    )
ENDIF()
