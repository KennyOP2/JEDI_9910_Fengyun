INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    .
)

IF(${SMEDIA_PLATFORM} STREQUAL win32)
    ADD_DEFINITIONS(
        -DModulator_EXPORTS
    )
ENDIF()

IF(${SMEDIA_PLATFORM} STREQUAL freertos)
    ADD_LIBRARY(eagle_modulator
        #IT9507/eagle.cpp
        #IT9507/eagle.h
        IT9507/modulatorUser.cpp
        #IT9507/WinIo.h
        #IT9507/i2cimpl.cpp
        #IT9507/i2cimpl.h
        #IT9507/IQ_fixed_table.h
        IT9507/IT9507.cpp
        IT9507/IT9507.h
        IT9507/eagleTuner.cpp
        IT9507/eagleTuner.h
        #IT9507/modulatorCmd.cpp
        #IT9507/modulatorCmd.h
        IT9507/modulatorError.h
        IT9507/modulatorFirmware.h
        IT9507/modulatorRegister.h
        IT9507/modulatorType.h
        IT9507/modulatorUser.h
        IT9507/modulatorVariable.h
        IT9507/modulatorVersion.h
        #IT9507/usb2impl.cpp
        #IT9507/usb2impl.h
        usb_mod/usb_mod.c
        usb_mod/usb_mod.h
        usb_mod/usb_mod_transport.c
        usb_mod/usb_mod_transport.h
        mod_ctrl.h
        mod_ctrl.c
    )

    TARGET_LINK_LIBRARIES(eagle_modulator
        mmp
        usb
        iic
        pal
    )
ENDIF()
