# Install script for directory: E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/sdk/driver

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
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/aud/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/capture/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/decompress/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/dma/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/dpu/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/encoder/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/exrtc/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/hdmirx/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/i2s/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/iic/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/intr/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/ir/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/isp/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/jpg/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/mmp/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/modulator/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/msc/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/nor/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/rtc/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/sd/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/spdif/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/spi/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/timer/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/tsi/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/tso/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/uart/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/usb/cmake_install.cmake")
  include("E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/watchdog/cmake_install.cmake")

endif()

