#ifndef USB_DEVICE_FS_POS_H
#define USB_DEVICE_FS_POS_H

#include "usb/device/config/usb_device_user_fs.h"


// Processing
#if (FS_CONFIGURATION_NUMBER >= 1)
	// Configuration 1
	#if (FS_C1_INTERFACE_NUMBER >= 1)
		// Interface 0
		#if (FS_C1_I0_ALT_NUMBER >= 1)
			// AlternateSetting 0
			#define FS_C1_I0_A0_EP_LENGTH			(EP_LENGTH * FS_C1_I0_A0_EP_NUMBER)
			#if (FS_C1_I0_A0_EP_NUMBER >= 1)
				// EP1
				#define FS_C1_I0_A0_EP1_FIFO_START		FIFO0
				#define FS_C1_I0_A0_EP1_FIFO_NO		(FS_C1_I0_A0_EP1_BLKNO * FS_C1_I0_A0_EP1_BLKSIZE)
				#define FS_C1_I0_A0_EP1_FIFO_CONFIG	(USB_DEVICE_MSK_FIFO_ENABLE | ((FS_C1_I0_A0_EP1_BLKSIZE - 1) << 4) | ((FS_C1_I0_A0_EP1_BLKNO - 1) << 2) | FS_C1_I0_A0_EP1_TYPE)
				
				#if(((Bulk_Satus == Bulk_FIFO_SingleDir) && (OTG_AP_Satus == Bulk_AP)) || (OTG_AP_Satus != Bulk_AP))
				#define FS_C1_I0_A0_EP1_FIFO_MAP		(((1 - FS_C1_I0_A0_EP1_DIRECTION) << 4) | EP1)
				#elif(Bulk_Satus == Bulk_FIFO_BiDir)
				#define FS_C1_I0_A0_EP1_FIFO_MAP		((2 << 4) | EP1)				
				#endif
				
				#define FS_C1_I0_A0_EP1_MAP			(FS_C1_I0_A0_EP1_FIFO_START |	(FS_C1_I0_A0_EP1_FIFO_START << 4)	| (MASK_F0 >> (4*FS_C1_I0_A0_EP1_DIRECTION)))
			#endif
			#if (FS_C1_I0_A0_EP_NUMBER >= 2)
				// EP2
				#if(((Bulk_Satus == Bulk_FIFO_SingleDir) && (OTG_AP_Satus == Bulk_AP)) || (OTG_AP_Satus != Bulk_AP))
				#define FS_C1_I0_A0_EP2_FIFO_START		(FS_C1_I0_A0_EP1_FIFO_START + FS_C1_I0_A0_EP1_FIFO_NO)
				#elif(Bulk_Satus == Bulk_FIFO_BiDir)
				#define FS_C1_I0_A0_EP2_FIFO_START		FIFO0
				#endif

				#define FS_C1_I0_A0_EP2_FIFO_NO		(FS_C1_I0_A0_EP2_BLKNO * FS_C1_I0_A0_EP2_BLKSIZE)
				#define FS_C1_I0_A0_EP2_FIFO_CONFIG	(USB_DEVICE_MSK_FIFO_ENABLE | ((FS_C1_I0_A0_EP2_BLKSIZE - 1) << 4) | ((FS_C1_I0_A0_EP2_BLKNO - 1) << 2) | FS_C1_I0_A0_EP2_TYPE)
				#define FS_C1_I0_A0_EP2_FIFO_MAP		(((1 - FS_C1_I0_A0_EP2_DIRECTION) << 4) | EP2)
				#define FS_C1_I0_A0_EP2_MAP			(FS_C1_I0_A0_EP2_FIFO_START |	(FS_C1_I0_A0_EP2_FIFO_START << 4)	| (MASK_F0 >> (4*FS_C1_I0_A0_EP2_DIRECTION)))
			#endif
			#if (FS_C1_I0_A0_EP_NUMBER >= 3)
				// EP3	(For Bulk Bi-direction test Interrupt IN Endpoint)
				#define FS_C1_I0_A0_EP3_FIFO_START		FIFO2
				#define FS_C1_I0_A0_EP3_FIFO_NO		(FS_C1_I0_A0_EP3_BLKNO * FS_C1_I0_A0_EP3_BLKSIZE)
				#define FS_C1_I0_A0_EP3_FIFO_CONFIG	(USB_DEVICE_MSK_FIFO_ENABLE | ((FS_C1_I0_A0_EP3_BLKSIZE - 1) << 4) | ((FS_C1_I0_A0_EP3_BLKNO - 1) << 2) | FS_C1_I0_A0_EP3_TYPE)
				#define FS_C1_I0_A0_EP3_FIFO_MAP		(((1 - FS_C1_I0_A0_EP3_DIRECTION) << 4) | EP3)
				#define FS_C1_I0_A0_EP3_MAP			(FS_C1_I0_A0_EP3_FIFO_START |	(FS_C1_I0_A0_EP3_FIFO_START << 4)	| (MASK_F0 >> (4*FS_C1_I0_A0_EP3_DIRECTION)))
			#endif
			#if (FS_C1_I0_A0_EP_NUMBER >= 4)
				// EP4	(For Bulk Bi-direction test Interrupt OUT Endpoint)
				#define FS_C1_I0_A0_EP4_FIFO_START		(FS_C1_I0_A0_EP3_FIFO_START + FS_C1_I0_A0_EP3_FIFO_NO)
				#define FS_C1_I0_A0_EP4_FIFO_NO		(FS_C1_I0_A0_EP4_BLKNO * FS_C1_I0_A0_EP4_BLKSIZE)
				#define FS_C1_I0_A0_EP4_FIFO_CONFIG	(USB_DEVICE_MSK_FIFO_ENABLE | ((FS_C1_I0_A0_EP4_BLKSIZE - 1) << 4) | ((FS_C1_I0_A0_EP4_BLKNO - 1) << 2) | FS_C1_I0_A0_EP4_TYPE)
				#define FS_C1_I0_A0_EP4_FIFO_MAP		(((1 - FS_C1_I0_A0_EP4_DIRECTION) << 4) | EP4)
				#define FS_C1_I0_A0_EP4_MAP			(FS_C1_I0_A0_EP4_FIFO_START |	(FS_C1_I0_A0_EP4_FIFO_START << 4)	| (MASK_F0 >> (4*FS_C1_I0_A0_EP4_DIRECTION)))
			#endif					
		#endif
		
		#if (FS_C1_I0_ALT_NUMBER == 1)
			#define FS_C1_I0_ALT_LENGTH				(FS_C1_I0_A0_EP_LENGTH)
		#elif (FS_C1_I0_ALT_NUMBER == 2)
			#define FS_C1_I0_ALT_LENGTH				(FS_C1_I0_A0_EP_LENGTH + FS_C1_I0_A1_EP_LENGTH)
		#endif
	#endif

	#if (FS_C1_INTERFACE_NUMBER == 1)
		#define FS_C1_INTERFACE_LENGTH				(FS_C1_I0_ALT_LENGTH)
	#elif (FS_C1_INTERFACE_NUMBER == 2)
		#define FS_C1_INTERFACE_LENGTH				(FS_C1_I0_ALT_LENGTH + HS_FS_C1_I1_ALT_LENGTH)
	#endif
#endif


// Configuration Descriptor length in ROM code
#if (FS_CONFIGURATION_NUMBER >= 1)
	#define FS_C1_CONFIG_TOTAL_LENGTH				(CONFIG_LENGTH + INTERFACE_LENGTH +  FS_C1_INTERFACE_LENGTH)
#endif
#if (FS_CONFIGURATION_NUMBER >= 2)
	#define FS_C2_CONFIG_TOTAL_LENGTH				(CONFIG_LENGTH + INTERFACE_LENGTH +  FS_C2_INTERFACE_LENGTH)
#endif

// Configuration Descriptor length in external SRAM
#if (FS_CONFIGURATION_NUMBER == 1)
	#define FS_MAX_CONFIG_TOTAL_LENGTH				(FS_C1_CONFIG_TOTAL_LENGTH)
#elif (FS_CONFIGURATION_NUMBER == 2)
	#if (FS_C1_CONFIG_TOTAL_LENGTH > FS_C2_CONFIG_TOTAL_LENGTH)
		#define FS_MAX_CONFIG_TOTAL_LENGTH			(FS_C1_CONFIG_TOTAL_LENGTH)
	#else
		#define FS_MAX_CONFIG_TOTAL_LENGTH			(FS_C2_CONFIG_TOTAL_LENGTH)
	#endif
#endif



#endif
