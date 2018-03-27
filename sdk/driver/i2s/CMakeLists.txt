INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
)

ADD_LIBRARY(i2s
    i2s_9910.c
    codec_wm8960.c
    transceiver_wm8804.c

    # inlcude file
    ../../include/i2s/transceiver_wm8804.h
    ../../include/i2s/codec_wm8960.h
    ../../include/mmp_i2s.h
    ../../include/i2s/i2s_9910_reg.h
)

TARGET_LINK_LIBRARIES(i2s
    core
)