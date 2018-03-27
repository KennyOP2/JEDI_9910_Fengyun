INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    ${SMEDIA_INC_DIRS}/encoder
    )

IF(${SMEDIA_PLATFORM} STREQUAL win32)
    ADD_DEFINITIONS(
        -DENCODER_EXPORTS
        )

ENDIF(${SMEDIA_PLATFORM} STREQUAL win32)

ADD_LIBRARY(encoder
    mmp_encoder.c
    encoder_memory.c
    encoder_hardware.c
    encoder_bitstream.c
    )

TARGET_LINK_LIBRARIES(encoder
	mmp
	pal
	)
