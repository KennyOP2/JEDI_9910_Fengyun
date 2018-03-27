INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    )

IF(${SMEDIA_PLATFORM} STREQUAL win32)
    ADD_DEFINITIONS(
        -DMSC_EXPORTS
        )

ENDIF(${SMEDIA_PLATFORM} STREQUAL win32)

ADD_LIBRARY(msc
    usb.c
    transport.c
    scsi.c
    mmp_msc.c
    ${PROJECT_SOURCE_DIR}/sdk/include/msc/config.h
    ${PROJECT_SOURCE_DIR}/sdk/include/msc/msc_error.h
    ${PROJECT_SOURCE_DIR}/sdk/include/msc/usb.h
    ${PROJECT_SOURCE_DIR}/sdk/include/msc/scsi.h
    ${PROJECT_SOURCE_DIR}/sdk/include/msc/protocol.h
    ${PROJECT_SOURCE_DIR}/sdk/include/msc/transport.h
    ${PROJECT_SOURCE_DIR}/sdk/include/mmp_msc.h
    )

IF(${SMEDIA_PLATFORM} STREQUAL win32)
    TARGET_LINK_LIBRARIES(msc
        mmp
        usb
        )

ENDIF(${SMEDIA_PLATFORM} STREQUAL win32)
