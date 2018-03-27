#IF(${SMEDIA_TARGET} STREQUAL example)
#    ADD_DEFINITIONS(
#        -DDISABLE_RTC
#    )
#ENDIF()

#ADD_DEFINITIONS(
#    -DDISABLE_RTC
#)

INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
)

SET(FAT_SRCS
    common/fat.c
    common/fat_lfn.c
    common/fat_m.c
    common/port_f.c
    common/common.c
    mmc/single/mmc_smedia.c
    #ftl/ftldrv.c
    #ftl/hlayer.c
    #ftl/ftl/mlayer.c
    #ftl/ftl/wear.c
    #ftl/ftl/phy/llayer.c
)

ADD_LIBRARY(fat STATIC
    ${FAT_SRCS}
)

TARGET_LINK_LIBRARIES(fat
    exrtc
)
