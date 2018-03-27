INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
)

IF(${SMEDIA_PLATFORM} STREQUAL win32)
    ADD_DEFINITIONS(
        -DDMA_EXPORTS
    )
ENDIF()

ADD_LIBRARY(dma
    mmp_dma.c
    dma.c
    dma_hw.c
    ${PROJECT_SOURCE_DIR}/sdk/include/dma/dma_reg.h
    ${PROJECT_SOURCE_DIR}/sdk/include/dma/dma_hw.h
    ${PROJECT_SOURCE_DIR}/sdk/include/dma/dma.h
    ${PROJECT_SOURCE_DIR}/sdk/include/dma/dma_error.h
    ${PROJECT_SOURCE_DIR}/sdk/include/dma/config.h
    ${PROJECT_SOURCE_DIR}/sdk/include/mmp_dma.h
)

IF(${SMEDIA_PLATFORM} STREQUAL win32)
    TARGET_LINK_LIBRARIES(dma
        mmp
    )
ELSEIF(${SMEDIA_PLATFORM} STREQUAL freertos)
    TARGET_LINK_LIBRARIES(dma
        intr
    )
ENDIF()
