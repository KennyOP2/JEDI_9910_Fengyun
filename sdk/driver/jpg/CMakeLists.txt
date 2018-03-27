INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    ${SMEDIA_INC_DIRS}/jpg
)

IF(${SMEDIA_PLATFORM} STREQUAL win32)
    ADD_DEFINITIONS(
        -DENCODER_EXPORTS
    )
ENDIF()

ADD_LIBRARY(jpg   
    jpg_common.c
    jpg_encode.c
    jpg_hw.c
    jpg_marker.c  
    mmp_jpg.c    
)

TARGET_LINK_LIBRARIES(jpg
    mmp
    pal
)
