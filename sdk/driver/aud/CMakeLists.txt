INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    ${LIBRARY_OUTPUT_PATH}
    )

IF(${SMEDIA_PLATFORM} STREQUAL win32)
    ADD_DEFINITIONS(
        -DAUD_EXPORTS
        -DHAVE_MP3
        -DHAVE_AMR
        -DHAVE_AAC
        -DHAVE_WAV
        -DHAVE_AC3
        )

    ADD_LIBRARY(aud
        audio.c
        )

    TARGET_LINK_LIBRARIES(aud
        mmp
        wm8978codec
        iic
        pal
        winmm
        )
ELSE(${SMEDIA_PLATFORM} STREQUAL win32)
    ADD_LIBRARY(aud
        audio.c
        )

    ADD_LIBRARY(aud_plugin
        audio.c
        codecs.c
        )

    SET_TARGET_PROPERTIES(aud_plugin PROPERTIES COMPILE_FLAGS "-DENABLE_CODECS_PLUGIN")
ENDIF(${SMEDIA_PLATFORM} STREQUAL win32)

