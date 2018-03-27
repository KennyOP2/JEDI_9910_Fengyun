# Install script for directory: E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/freertos

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/JEDI")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "RELEASE")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/freertos/src/newlib/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/freertos/src/or32/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/freertos/src/freertos/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/freertos/src/startup/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/freertos/src/pal/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/freertos/src/malloc/cmake_install.cmake")

endif()

