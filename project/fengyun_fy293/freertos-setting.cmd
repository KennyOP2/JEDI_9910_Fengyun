@echo off

: set default values for unhandled settings
set CONFIG_HAVE_USBD=1
set CONFIG_HAVE_NTFS=1
set USB_DEVICE=ENABLE

:begin_pc_grabber
if NOT "%CONFIG_HAVE_USBD%"=="1" (
    set CONFIG_HAVE_USBD=0
    set CONFIG_USBD_HAVE_PCGRABBER=0
    goto end_pc_grabber
)

set /p CONFIG_USBD_HAVE_PCGRABBER=Enable PC Grabber Function (Y/N)? 
if /i "%CONFIG_USBD_HAVE_PCGRABBER%"=="N" (
    set CONFIG_USBD_HAVE_PCGRABBER=0
    goto end_pc_grabber
)
if /i "%CONFIG_USBD_HAVE_PCGRABBER%"=="Y" (
    set CONFIG_USBD_HAVE_PCGRABBER=1
    set CONFIG_HAVE_USBD=1
    set USB_DEVICE=ENABLE
    goto end_pc_grabber
)
goto begin_pc_grabber
:end_pc_grabber

:set_external_hdmirx
set /p EXTERNAL_HDMIRX=External HDMIRX (1:Disable, 2:Enable)?
if "%EXTERNAL_HDMIRX%"=="1" (
    set EXTERNAL_HDMIRX=DISABLE
    goto set_component_dev
)
if "%EXTERNAL_HDMIRX%"=="2" (
    set EXTERNAL_HDMIRX=ENABLE
    goto set_component_dev
)
goto set_external_hdmirx


:set_component_dev
set /p COMPONENT_DEV=Component Device(YPbPr/VGA) (1:Disable, 2:Enable)?
if "%COMPONENT_DEV%"=="1" (
    set COMPONENT_DEV=DISABLE
    goto set_composite_dev
)
if "%COMPONENT_DEV%"=="2" (
    set COMPONENT_DEV=ENABLE
    goto set_composite_dev
)
goto set_component_dev


:set_composite_dev
set /p COMPOSITE_DEV=Composite Device(CVBS) (1:Disable, 2:Enable)?
if "%COMPOSITE_DEV%"=="1" (
    set COMPOSITE_DEV=DISABLE
    goto set_hdmi_loopthrough
)
if "%COMPOSITE_DEV%"=="2" (
    set COMPOSITE_DEV=ENABLE
    goto set_hdmi_loopthrough
)
goto set_composite_dev


:set_hdmi_loopthrough
set /p HDMI_LOOPTHROUGH=HDMI Loopthrough (1:Disable, 2:Enable)?
if "%HDMI_LOOPTHROUGH%"=="1" (
    set HDMI_LOOPTHROUGH=DISABLE
    goto set_chip_version
)
if "%HDMI_LOOPTHROUGH%"=="2" (
    set HDMI_LOOPTHROUGH=ENABLE
    goto set_chip_version
)
goto set_hdmi_loopthrough


:set_chip_version
echo    1=IT9917 176TQFP
echo    2=IT9913 128LQFP
echo    3=IT9919 144TQFP
set /p CHIP_VERSION=Chip version (default=1)?
if "%CHIP_VERSION%"=="1" (
    set CHIP_VERSION=IT9917_176TQFP
    set BOARD_MODULE=EVB_BOARD
    goto final
)
if "%CHIP_VERSION%"=="2" (
    set CHIP_VERSION=IT9913_128LQFP
    set BOARD_MODULE=EVB_BOARD
    goto final
)
if "%CHIP_VERSION%"=="3" (
    set CHIP_VERSION=IT9919_144TQFP
    goto set_board_module
)
goto set_chip_version

:set_board_module
echo    1=EVB Board
echo    2=Reference Board (AVSender/Grabber)
echo    3=Reference Board (Camera)
set /p BOARD_MODULE=board module (default=1)?
if "%BOARD_MODULE%"=="1" (
    set BOARD_MODULE=EVB_BOARD
    goto set_hdcp_on
)
if "%BOARD_MODULE%"=="2" (
    set BOARD_MODULE=REF_BOARD_AVSENDER
    goto set_hdcp_on
)
if "%BOARD_MODULE%"=="3" (
    set BOARD_MODULE=REF_BOARD_CAMERA
    goto set_hdcp_on
)
goto set_board_module

:set_hdcp_on
set /p HDCP_ON=HDCP_ON (1:Disable, 2:Enable)?
if "%HDCP_ON%"=="1" (
    set HDCP_ON=DISABLE
    goto set_external_rtc
)
if "%HDCP_ON%"=="2" (
    set HDCP_ON=ENABLE
    goto set_external_rtc
)
goto set_hdcp_on

:set_external_rtc
set /p EXTERNAL_RTC=EXTERNAL_RTC (1:off, 2:on)?
if "%EXTERNAL_RTC%"=="1" (
    set EXTERNAL_RTC=DISABLE
    goto set_suport_mic_mixed
)
if "%EXTERNAL_RTC%"=="2" (
    set EXTERNAL_RTC=ENABLE
    goto set_suport_mic_mixed
)
goto set_external_rtc

:set_suport_mic_mixed
set /p SUPPORT_MIC_MIXED=SUPPORT_MIC_MIXED (1:off, 2:on)?
if "%SUPPORT_MIC_MIXED%"=="1" (
    set SUPPORT_MIC_MIXED=DISABLE
    goto set_use_wm8960_adc
)
if "%SUPPORT_MIC_MIXED%"=="2" (
    set SUPPORT_MIC_MIXED=ENABLE
    goto set_use_wm8960_adc
)
goto set_suport_mic_mixed

:set_use_wm8960_adc
set /p USE_WM8960_ADC=USE_WM8960_ADC (1:off, 2:on)?
if "%USE_WM8960_ADC%"=="1" (
    set USE_WM8960_ADC=DISABLE
    goto set_hdmitx_type
)
if "%USE_WM8960_ADC%"=="2" (
    set USE_WM8960_ADC=ENABLE
    goto set_hdmitx_type
)
goto set_use_wm8960_adc

:set_hdmitx_type
echo    1=IT6613
echo    2=IT66121
set /p HDMITX_TYPE=HDMITX_TYPE (default=1)?
if "%HDMITX_TYPE%"=="1" (
    set HDMITX_TYPE=IT6613
    goto final
)
if "%HDMITX_TYPE%"=="2" (
    set HDMITX_TYPE=IT66121
    goto final
)
goto set_hdmitx_type

:final
echo set CMAKE_BUILD_TYPE=RELEASE>> freertos-config.cmd
echo set PROJECT_TYPE=hw_grabber>> freertos-config.cmd
echo set SYSTEM_FAT_TYPE=HAVE_FAT>> freertos-config.cmd
echo set USB_DEVICE=%USB_DEVICE%>> freertos-config.cmd
echo set USB_STORAGE=ENABLE>> freertos-config.cmd
echo set SD_STORAGE=ENABLE>> freertos-config.cmd
echo set TSO_MODULE=DISABLE>> freertos-config.cmd
echo set EXTERNAL_HDMIRX=%EXTERNAL_HDMIRX%>> freertos-config.cmd
echo set COMPONENT_DEV=%COMPONENT_DEV%>> freertos-config.cmd
echo set COMPOSITE_DEV=%COMPOSITE_DEV%>> freertos-config.cmd
echo set HDMI_LOOPTHROUGH=%HDMI_LOOPTHROUGH%>> freertos-config.cmd
echo set MULTIPLE_INSTANCES=DISABLE>> freertos-config.cmd
echo set SENSOR_DEV=DISABLE>> freertos-config.cmd
echo set SENSOR_ID=NONE>> freertos-config.cmd
echo set CHIP_VERSION=%CHIP_VERSION%>> freertos-config.cmd
echo set BOARD_MODULE=%BOARD_MODULE%>> freertos-config.cmd
echo set HDCP_ON=%HDCP_ON%>> freertos-config.cmd
echo set CONFIG_USBD_HAVE_PCGRABBER=%CONFIG_USBD_HAVE_PCGRABBER% >> freertos-config.cmd
echo set CONFIG_HAVE_NTFS=%CONFIG_HAVE_NTFS% >> freertos-config.cmd
echo set CONFIG_HAVE_USBD=%CONFIG_HAVE_USBD% >> freertos-config.cmd
echo set EXTERNAL_RTC=%EXTERNAL_RTC% >> freertos-config.cmd
echo set SUPPORT_MIC_MIXED=%SUPPORT_MIC_MIXED% >> freertos-config.cmd
echo set USE_WM8960_ADC=%SUPPORT_MIC_MIXED% >> freertos-config.cmd
echo set MENCODER=ENABLE>>freertos-config.cmd
echo set HDMITX_TYPE=%HDMITX_TYPE%>>freertos-config.cmd