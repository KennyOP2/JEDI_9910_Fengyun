if (DEFINED CFG_BUILD_CODECS)
    add_subdirectory(codecs)
endif()

add_subdirectory(driver)
add_subdirectory(share)