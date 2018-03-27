INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    )

SET(MP2_SRCS
    main.c
    mpegaudio.c
    mpegaudioenc.c
    resample2.c
)

IF(ENABLE_CODECS_PLUGIN)
    ADD_EXECUTABLE(mp2encode.plugin
        ${MP2_SRCS}
    )

    ADD_CUSTOM_COMMAND(
        TARGET mp2encode.plugin
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
                -O binary ${CMAKE_CURRENT_BINARY_DIR}/mp2encode.plugin ${LIBRARY_OUTPUT_PATH}/mp2encode.codecs
        COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/mp2encode.codecs
    )
ELSE(ENABLE_CODECS_PLUGIN)
    ADD_LIBRARY(mp2encode STATIC
        ${MP2_SRCS}
    )
ENDIF(ENABLE_CODECS_PLUGIN)

