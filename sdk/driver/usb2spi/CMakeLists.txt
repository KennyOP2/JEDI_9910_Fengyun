INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    )

	SET(LINK_DIRS
        ${PROJECT_SOURCE_DIR}/sdk/include/usb2spi
        )
        
IF(${SMEDIA_PLATFORM} STREQUAL win32)
    ADD_LIBRARY(usb2spi
		dllmain.c
        usbspi.c
        spi.c
        bus.c
        )
    TARGET_LINK_LIBRARIES(usb2spi
        pal
        setupapi
        winmm
        ${LINK_DIRS}/FTCSPI.lib
        )
ENDIF(${SMEDIA_PLATFORM} STREQUAL win32)
