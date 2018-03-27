
IF(${SMEDIA_PLATFORM} STREQUAL freertos)
    SET(DATACONV ${PROJECT_SOURCE_DIR}/tool/bin/dataconv.exe)
   
    #################################################################
    # Build CODEC as plug-ins
    #################################################################
    SET(ENABLE_CODECS_PLUGIN 1)
    ADD_DEFINITIONS(
        -DENABLE_CODECS_PLUGIN=1
        )
    SET(CODECS_LD "codecs.ld")
		SET(CMAKE_C_FLAGS_DEBUG     "-O0 -g3 -mhard-div -fno-short-enums")
		SET(CMAKE_C_FLAGS_RELEASE   "-O3 -g3 -DNDEBUG -mhard-div -fno-short-enums")
		SET(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g3 -mhard-div -fno-short-enums")
		SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -g3 -DNDEBUG -mhard-div -fno-short-enums")
    SET(CMAKE_EXE_LINKER_FLAGS "-nostartfiles -T${LIBRARY_OUTPUT_PATH}/${CODECS_LD} ${LIBRARY_OUTPUT_PATH}/codec_crt0.o")

    ADD_CUSTOM_COMMAND(
        OUTPUT ${LIBRARY_OUTPUT_PATH}/codec_crt0.o
        COMMAND ${CMAKE_C_COMPILER}
            ${CMAKE_CUSTOM_C_FLAGS}
            ${SMEDIA_DEFS}
            -I${PROJECT_SOURCE_DIR}/sdk/include
            -I${PROJECT_SOURCE_DIR}/freertos/include/or32
            -I${PROJECT_SOURCE_DIR}/freertos/include/freertos
            -c
            -o ${LIBRARY_OUTPUT_PATH}/codec_crt0.o
            ${CMAKE_CURRENT_SOURCE_DIR}/codec_crt0.c
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/codec_crt0.c
        )

    ADD_CUSTOM_TARGET(
        codec_crt0 ALL
        DEPENDS ${LIBRARY_OUTPUT_PATH}/codec_crt0.o
        )

    ADD_CUSTOM_COMMAND(
        OUTPUT ${LIBRARY_OUTPUT_PATH}/codecs.ld
        COMMAND ${CMAKE_C_PREPROCESSOR}
            ${CMAKE_CUSTOM_C_FLAGS}
            -I${PROJECT_SOURCE_DIR}/sdk/include/aud
            ${SMEDIA_DEFS}
            -E
            -P
            -o ${LIBRARY_OUTPUT_PATH}/codecs.ld
            ${CMAKE_CURRENT_SOURCE_DIR}/codecs.lds
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/codecs.lds
        )
    ADD_CUSTOM_TARGET(
        codecs.ld ALL
        DEPENDS ${LIBRARY_OUTPUT_PATH}/codecs.ld
        )

    ADD_SUBDIRECTORY(mp2encode)
    ADD_SUBDIRECTORY(aacencode)

ENDIF(${SMEDIA_PLATFORM} STREQUAL freertos)

