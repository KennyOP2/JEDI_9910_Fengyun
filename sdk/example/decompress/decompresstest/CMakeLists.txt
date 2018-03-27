IF(${SMEDIA_PLATFORM} STREQUAL win32)
    SET(EXAMPLE_LINK_LIBS
        winmm
        )
ELSEIF(${SMEDIA_PLATFORM} STREQUAL freertos OR ${SMEDIA_PLATFORM} STREQUAL openrtos)
    SET(EXAMPLE_LINK_DIRS
        ${PROJECT_BINARY_DIR}/${SMEDIA_PLATFORM}
        )
    SET(EXAMPLE_LINK_LIBS
        ${SMEDIA_PLATFORM}
#        nandboot
        )
ENDIF(${SMEDIA_PLATFORM} STREQUAL win32)

INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

ADD_EXECUTABLE(decompresstest
    main.c
    )

LINK_DIRECTORIES(
    ${PROJECT_BINARY_DIR}/decompress
    ${PROJECT_BINARY_DIR}/pal    
    ${PROJECT_BINARY_DIR}/mmp  
    ${PROJECT_BINARY_DIR}/ucl
    ${EXAMPLE_LINK_DIRS}
    )

TARGET_LINK_LIBRARIES(decompresstest
    decompress   
    pal      
    mmp
    ucl
    ${EXAMPLE_LINK_LIBS}
    #ith
    )

IF(${SMEDIA_PLATFORM} STREQUAL openrtos)
	TARGET_LINK_LIBRARIES(decompresstest
	    decompress
	    pal
	    mmp
	    ucl
	    ${EXAMPLE_LINK_LIBS}
	    ith
	    )
ELSEIF(${SMEDIA_PLATFORM} STREQUAL freertos)
	TARGET_LINK_LIBRARIES(decompresstest
	    decompress
	    pal
	    mmp
	    ucl
	    ${EXAMPLE_LINK_LIBS}
	    intr
	    )
ELSEIF(${SMEDIA_PLATFORM} STREQUAL win32)
	TARGET_LINK_LIBRARIES(decompresstest
	    decompress
	    ucl	    
	    pal
	    mmp
    ${EXAMPLE_LINK_LIBS}
    )
ENDIF(${SMEDIA_PLATFORM} STREQUAL openrtos)

IF(${SMEDIA_PLATFORM} STREQUAL freertos OR ${SMEDIA_PLATFORM} STREQUAL openrtos)
    ADD_CUSTOM_COMMAND(
        TARGET decompresstest
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
            -O binary ${CMAKE_CURRENT_BINARY_DIR}/decompresstest ${CMAKE_CURRENT_BINARY_DIR}/decompresstest.bin
        )

ENDIF(${SMEDIA_PLATFORM} STREQUAL freertos OR ${SMEDIA_PLATFORM} STREQUAL openrtos)
