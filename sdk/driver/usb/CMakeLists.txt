INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/${SMEDIA_PLATFORM}/include/freertos
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/.repos
)

IF(${SMEDIA_PLATFORM} STREQUAL win32)
    ADD_DEFINITIONS(
        -DUSBEX_EXPORTS
        -DOTG_EXPORTS
    )

    ADD_LIBRARY(usb
        mmp_usbex.c
        usb/hcd.c
        usb/usb.c
        usb/urbbuf.c
        usb/hcd/ehci_hcd.c
        usb/otg/otg.c
        ${PROJECT_SOURCE_DIR}/sdk/include/mmp_usbex.h
        ${PROJECT_SOURCE_DIR}/sdk/include/usb/config.h
        ${PROJECT_SOURCE_DIR}/sdk/include/usb/usb/usb.h
        ${PROJECT_SOURCE_DIR}/sdk/include/usb/usb/hcd.h
        ${PROJECT_SOURCE_DIR}/sdk/include/usb/usb/list.h
        ${PROJECT_SOURCE_DIR}/sdk/include/usb/usb/urbbuf.h
        ${PROJECT_SOURCE_DIR}/sdk/include/usb/usb/usbex_error.h
        ${PROJECT_SOURCE_DIR}/sdk/include/usb/usb/hcd/ehci.h
        ${PROJECT_SOURCE_DIR}/sdk/include/usb/usb/hcd/ehci_vmem.h
        ${PROJECT_SOURCE_DIR}/sdk/include/usb/usb/otg/otg.h
        device/usb_device.c
    )
       
    TARGET_LINK_LIBRARIES(usb
        mmp
        winmm
    )

ELSEIF(${SMEDIA_PLATFORM} STREQUAL freertos OR ${SMEDIA_PLATFORM} STREQUAL openrtos)
    set(it_usbd_files
        device/usb_device.c
    )
    if(${CONFIG_HAVE_USBD})
        set(it_usbd_files
            ${it_usbd_files}
            ${PROJECT_SOURCE_DIR}/.repos/usbd/src/it_usbd_device.cpp
            ${PROJECT_SOURCE_DIR}/.repos/usbd/src/it_usbd_descriptors.cpp
            ${PROJECT_SOURCE_DIR}/.repos/usbd/src/it_usbd_property.cpp
            ${PROJECT_SOURCE_DIR}/.repos/usbd/src/it_usbd_debug.cpp
        )
    endif()

    ADD_LIBRARY(usb
        mmp_usbex.c
        usb/hcd.c
        usb/usb.c
        usb/urbbuf.c
        usb/hcd/ehci_hcd.c
        usb/otg/otg.c
        ${it_usbd_files}
    )
ENDIF()
