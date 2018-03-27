# The set of languages for which implicit dependencies are needed:
set(CMAKE_DEPENDS_LANGUAGES
  "C"
  )
# The set of files for implicit dependencies of each language:
set(CMAKE_DEPENDS_CHECK_C
  "E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/sdk/driver/msc/mmp_msc.c" "E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/msc/CMakeFiles/msc.dir/mmp_msc.c.obj"
  "E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/sdk/driver/msc/scsi.c" "E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/msc/CMakeFiles/msc.dir/scsi.c.obj"
  "E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/sdk/driver/msc/transport.c" "E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/msc/CMakeFiles/msc.dir/transport.c.obj"
  "E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/sdk/driver/msc/usb.c" "E:/Grabber/SDK/TW/Fengyun/JEDI_9910_Fengyun/project/fengyun_fy283_bootloader/freertos/sdk/driver/msc/CMakeFiles/msc.dir/usb.c.obj"
  )
set(CMAKE_C_COMPILER_ID "GNU")

# Preprocessor definitions for this target.
set(CMAKE_TARGET_DEFINITIONS
  "BOOT_LOADER"
  "BUILD_NUMBER=3"
  "CFG_OSC_CLK=12000000"
  "CONFIG_HAVE_USBD=1"
  "CUSTOMER_CODE=9"
  "DTV_SD1_ENABLE"
  "DTV_USB_ENABLE"
  "ENABLE_CODECS_PLUGIN"
  "ENABLE_USB_DEVICE"
  "EXTERNAL_RTC"
  "HAVE_FAT"
  "IDLE_TASK"
  "IT9919_144TQFP"
  "MEMORY_SIZE=64M"
  "MM9910"
  "PROJECT_CODE=1"
  "REF_BOARD_AVSENDER"
  "RTOS_USE_ISR"
  "SDK_MAJOR_VERSION=0"
  "SDK_MINOR_VERSION=0"
  "__FREERTOS__"
  "__OR32__"
  )

# Targets to which this target links.
set(CMAKE_TARGET_LINKED_INFO_FILES
  )

# The include file search paths:
set(CMAKE_C_TARGET_INCLUDE_PATH
  "../../../freertos/include/or32"
  "../../../freertos/include/freertos"
  "../../../sdk/include"
  "../../../sdk/share/fat"
  "../../../sdk/share/fat/common"
  "../.repos"
  )
set(CMAKE_CXX_TARGET_INCLUDE_PATH ${CMAKE_C_TARGET_INCLUDE_PATH})
set(CMAKE_Fortran_TARGET_INCLUDE_PATH ${CMAKE_C_TARGET_INCLUDE_PATH})
set(CMAKE_ASM_TARGET_INCLUDE_PATH ${CMAKE_C_TARGET_INCLUDE_PATH})
