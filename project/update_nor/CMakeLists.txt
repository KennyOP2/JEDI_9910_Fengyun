INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/core
)

IF(${SMEDIA_PLATFORM} STREQUAL freertos)
    ADD_EXECUTABLE(jedi
        main_freertos.c
    )
ENDIF()

ADD_EXECUTABLE(jedi
    main.c
)

TARGET_LINK_LIBRARIES(jedi
    nor
    spi
    intr
    dma
    pal
    mmp
    freertos
)

ADD_CUSTOM_COMMAND(
    OUTPUT ${LIBRARY_OUTPUT_PATH}/default.ld
    COMMAND ${CMAKE_C_PREPROCESSOR}
        -I${PROJECT_SOURCE_DIR}/sdk/include/aud
        ${JEDI_DEFS}
        -E
        -P
        -o ${LIBRARY_OUTPUT_PATH}/default.ld
        ${CMAKE_CURRENT_SOURCE_DIR}/../../freertos/src/startup/default.lds
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../../freertos/src/startup/default.lds
)
ADD_CUSTOM_TARGET(
    default.ld ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/default.ld
)
ADD_CUSTOM_COMMAND(
    TARGET jedi
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
        -O binary ${CMAKE_CURRENT_BINARY_DIR}/jedi ${CMAKE_CURRENT_BINARY_DIR}/jedi.bin
)
ADD_CUSTOM_COMMAND(
    TARGET jedi
    POST_BUILD
    COMMAND mkrom -z -b 512K
        ${PROJECT_SOURCE_DIR}/tool/target/${JEDI_ROM_INIT_SCRIPT} ${CMAKE_CURRENT_BINARY_DIR}/jedi.bin ${CMAKE_CURRENT_BINARY_DIR}/jedi.rom
    COMMAND mkrom -z -b 512K
        ${PROJECT_SOURCE_DIR}/tool/target/${JEDI_ROM_INIT_SCRIPT} ${CMAKE_CURRENT_BINARY_DIR}/jedi.bin ${CMAKE_CURRENT_BINARY_DIR}/kproc.sys
)
ADD_CUSTOM_COMMAND(
    TARGET jedi
    POST_BUILD
    COMMAND sm32-elf-readelf
        -a ${CMAKE_CURRENT_BINARY_DIR}/jedi > ${CMAKE_CURRENT_BINARY_DIR}/jedi.symbol
)
