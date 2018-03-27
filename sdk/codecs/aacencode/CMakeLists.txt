INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    )


SET(AAC_SRCS
    aac_rom.c
    aacenc.c
    aac-enc.c
    aacenc_core.c
    adj_thr.c
    band_nrg.c
    basicop2.c
    bit_cnt.c
    bitbuffer.c
    bitenc.c
    block_switch.c
    channel_map.c
    cmnMemory.c
    dyn_bits.c
    grp_data.c
    interface.c
    line_pe.c
    memalign.c
    ms_stereo.c
    oper_32b.c
    pre_echo_control.c
    psy_configuration.c
    psy_main.c
    qc_main.c
    quantize.c
    sf_estim.c
    spreading.c
    stat_bits.c
    tns.c
    transform.c
    #wavreader.c
    resample2.c
)

IF(ENABLE_CODECS_PLUGIN)
    ADD_EXECUTABLE(aacencode.plugin
        ${AAC_SRCS}
    )

    ADD_CUSTOM_COMMAND(
        TARGET aacencode.plugin
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
                -O binary ${CMAKE_CURRENT_BINARY_DIR}/aacencode.plugin ${LIBRARY_OUTPUT_PATH}/aacencode.codecs
        COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/aacencode.codecs
    )
ELSE(ENABLE_CODECS_PLUGIN)
    ADD_LIBRARY(aacencode STATIC
        ${AAC_SRCS}
    )
ENDIF(ENABLE_CODECS_PLUGIN)

