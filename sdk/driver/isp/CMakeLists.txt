INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    )

IF(${SMEDIA_PLATFORM} STREQUAL win32)
    ADD_DEFINITIONS(
        -DISP_EXPORTS
        )

ENDIF(${SMEDIA_PLATFORM} STREQUAL win32)


ADD_LIBRARY(isp  
    isp.c
    isp_hw.c
    isp_util.c
    mmp_isp.c     
    
    # inlcude file
    ../../include/isp/isp.h
    ../../include/isp/isp_config.h
    ../../include/isp/isp_error.h
    ../../include/isp/isp_hw.h  
    ../../include/isp/isp_reg.h
    ../../include/isp/isp_types.h
    ../../include/isp/isp_util.h   
    ../../include/mmp_isp.h 
    )                        

IF(${SMEDIA_PLATFORM} STREQUAL win32)
    ADD_LIBRARY(isp 
        dllmain.c 
        )
        
    TARGET_LINK_LIBRARIES(isp
        pal
        mmp
        )
ENDIF(${SMEDIA_PLATFORM} STREQUAL win32)

IF(${SMEDIA_PLATFORM} STREQUAL freertos)
    TARGET_LINK_LIBRARIES(isp
        pal
        mmp
    )
ENDIF(${SMEDIA_PLATFORM} STREQUAL freertos)
