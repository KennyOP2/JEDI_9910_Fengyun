ADD_CUSTOM_COMMAND(
    OUTPUT ${LIBRARY_OUTPUT_PATH}/or32_crt0.o
    COMMAND ${CMAKE_C_COMPILER}
        ${CMAKE_CUSTOM_C_FLAGS}
        -I${PROJECT_SOURCE_DIR}/${SMEDIA_PLATFORM}/include/or32
        ${SMEDIA_DEFS}
        -c
        -o ${LIBRARY_OUTPUT_PATH}/or32_crt0.o
        ${CMAKE_CURRENT_SOURCE_DIR}/or32_crt0.S
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/or32_crt0.S
    )
ADD_CUSTOM_TARGET(
    or32_crt0 ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/or32_crt0.o
    )

ADD_CUSTOM_COMMAND(
    OUTPUT ${LIBRARY_OUTPUT_PATH}/default.ld
    COMMAND ${CMAKE_C_PREPROCESSOR}
        ${CMAKE_CUSTOM_C_FLAGS}
        -I${PROJECT_SOURCE_DIR}/sdk/include/aud
        ${SMEDIA_DEFS}
        -E
        -P
        -o ${LIBRARY_OUTPUT_PATH}/default.ld
        ${CMAKE_CURRENT_SOURCE_DIR}/default.lds
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/default.lds
    )
ADD_CUSTOM_TARGET(
    default.ld ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/default.ld
    )

