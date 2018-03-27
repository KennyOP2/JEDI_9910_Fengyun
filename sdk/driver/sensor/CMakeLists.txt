INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    )

IF(${SMEDIA_PLATFORM} STREQUAL win32)
	ADD_DEFINITIONS(
	    -DSENSOR_EXPORTS
	    )

ENDIF(${SMEDIA_PLATFORM} STREQUAL win32)
  
  
  
#Sensor ID                     
IF(${SENSOR_ID} STREQUAL PIXELPLUS_PH1100K)   
    ADD_LIBRARY(sensor
        mmp_sensor.c   
        )
ELSEIF(${SENSOR_ID} STREQUAL IPASION_IP2986)
    ADD_LIBRARY(sensor
        ipasion/ipasion_driver.c
        ipasion/ipasion_io.c
        mmp_sensor.c    
        )
ELSEIF(${SENSOR_ID} STREQUAL APTINA_MT9M034)
    ADD_LIBRARY(sensor
        aptina/aptina_driver.c
        aptina/aptina_io.c
        mmp_sensor.c   
        )                            
ENDIF(${SENSOR_ID} STREQUAL PIXELPLUS_PH1100K)


TARGET_LINK_LIBRARIES(sensor
    mmp
    pal
    iic
    )