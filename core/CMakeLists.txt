INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/driver/modulator
    ${PROJECT_SOURCE_DIR}/sdk/share/polarssl/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

IF(${SYSTEM_FAT_TYPE} STREQUAL HAVE_FAT)
    SET(SMEDIA_FAT_LIB
        fat
        msc
        #mspro
    )
    IF(${USB_STORAGE} STREQUAL ENABLE)
        SET(SMEDIA_FAT_LIB
            usb
        )
    ENDIF()
    IF(${SD_STORAGE} STREQUAL ENABLE)
        SET(SMEDIA_FAT_LIB
            sd
        )
    ENDIF()

    ADD_LIBRARY(core
        storage_mgr.c
    )
ENDIF()

IF(${COMPONENT_DEV} STREQUAL ENABLE)
    SET(COMPONENT_LIB
        cat9883
    )
ENDIF()

IF(${COMPOSITE_DEV} STREQUAL ENABLE)
    SET(COMPOSITE_LIB
        adv7180
    )
ENDIF()

IF(${HDMI_LOOPTHROUGH} STREQUAL ENABLE)
    SET(HDMITX_LIB
        hdmitx
    )
ENDIF()

IF(${MENCODER} STREQUAL ENABLE)
    ADD_SUBDIRECTORY(mencoder)
    SET(MENCODER_LIB
        lib_MEncoder
    )
    ADD_LIBRARY(core
        task_avi_mux.c
    )
ENDIF()

ADD_LIBRARY(core
    task_stream_mux.c
)

IF(${CONFIG_HAVE_NTFS})
    SET(DTV_LINK_LIBS
        ${DTV_LINK_LIBS}
        libsxa_blkdev_usb
        libsxa_ntfs3g
    )
ENDIF()

ADD_LIBRARY(core
    core_interface.c
    crc.c
    rom_parser.c
    mps_cmdq.c
    mps_control.c
    mps_system.c
    queue_mgr.c
    task_audio_in.c
    task_video_encoder.c
    ts_mux.c
    av_sync.c
    ts_security.c
    hdmi_loop_through.c
    msg_route.c
    psi_si_table_mgr.c
)

TARGET_LINK_LIBRARIES(core
    aud_plugin
    ${SMEDIA_FAT_LIB}
    ${COMPONENT_LIB}
    ${COMPOSITE_LIB}
    ${HDMITX_LIB}
    ${MENCODER_LIB}
    hdmirx
    pal
    mmp
    freertos
    encoder
    i2s
    iic
    polarssl
    timer
    nor
    exrtc
    spdif
    jpg
)
