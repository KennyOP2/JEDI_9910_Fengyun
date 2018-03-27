#ifndef USB_DEVICE_TABLE_H
#define USB_DEVICE_TABLE_H


#include "usb/device/usb_device_def.h"
#include "usb/device/usb_device_hs_pos.h"
#include "usb/device/usb_device_fs_pos.h"



#if (HS_MAX_CONFIG_TOTAL_LENGTH > FS_MAX_CONFIG_TOTAL_LENGTH)
	#define CONFIG_LENGTH_EX				(HS_MAX_CONFIG_TOTAL_LENGTH)
	#define OTHER_SPEED_CONFIG_LENGTH_EX	(HS_MAX_CONFIG_TOTAL_LENGTH)
#else
	#define CONFIG_LENGTH_EX				(FS_MAX_CONFIG_TOTAL_LENGTH)
	#define OTHER_SPEED_CONFIG_LENGTH_EX	(FS_MAX_CONFIG_TOTAL_LENGTH)
#endif


extern  MMP_UINT8 u8OTGHSDeviceDescriptor[DEVICE_LENGTH];
#if (HS_CONFIGURATION_NUMBER >= 1)
	extern  MMP_UINT8 u8HSConfigOTGDescriptor01[HS_C1_CONFIG_TOTAL_LENGTH];
#endif
#if (HS_CONFIGURATION_NUMBER >= 2)
	extern  MMP_UINT8 u8HSConfigOTGDescriptor02[HS_C2_CONFIG_TOTAL_LENGTH];
#endif

extern  MMP_UINT8 u8OTGFSDeviceDescriptor[DEVICE_LENGTH];
#if (FS_CONFIGURATION_NUMBER >= 1)
	extern  MMP_UINT8 u8FSConfigOTGDescriptor01[FS_C1_CONFIG_TOTAL_LENGTH];
#endif
#if (FS_CONFIGURATION_NUMBER >= 2)
	extern  MMP_UINT8 u8FSConfigOTGDescriptor02[FS_C2_CONFIG_TOTAL_LENGTH];
#endif



#endif  /* #ifndef USB_DEVICE_TABLE_H */
