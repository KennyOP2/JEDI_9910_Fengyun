INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    .
)

IF(${SMEDIA_PLATFORM} STREQUAL win32)
    ADD_DEFINITIONS(
        -DModulator_EXPORTS
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

IF(${SENSOR_DEV} STREQUAL ENABLE)
    SET(SENSOR_LIB
	sensor
    )
ENDIF()

IF(${SMEDIA_PLATFORM} STREQUAL freertos)
    ADD_LIBRARY(cap
        mmp_capture.c
        capture_hw.c
        capture.c
        cap_util.c
        ${PROJECT_SOURCE_DIR}/sdk/include/capture/capture.h
    )

    TARGET_LINK_LIBRARIES(cap
        freertos
        isp
        mmp
        iic
        pal
        ${SENSOR_LIB}
    	hdmirx     
    	${COMPONENT_LIB}
    	${COMPOSITE_LIB}
    )
ENDIF()
