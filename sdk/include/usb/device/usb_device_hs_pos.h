#ifndef USB_DEVICE_HS_POS_H
#define USB_DEVICE_HS_POS_H

#include "usb/device/config/usb_device_user_hs.h"

// Processing
#if (HS_CONFIGURATION_NUMBER >= 1)
	// Configuration 1
	#if (HS_C1_INTERFACE_NUMBER >= 1)
		// Interface 0
		#if (HS_C1_I0_ALT_NUMBER >= 1)
			// AlternateSetting 0
			#define HS_C1_I0_A0_EP_LENGTH			(EP_LENGTH * HS_C1_I0_A0_EP_NUMBER)
			#if (HS_C1_I0_A0_EP_NUMBER >= 1)
				// EP1
				#define HS_C1_I0_A0_EP1_FIFO_START		FIFO0
				#define HS_C1_I0_A0_EP1_FIFO_NO		(HS_C1_I0_A0_EP1_BLKNO * HS_C1_I0_A0_EP1_BLKSIZE)
				#define HS_C1_I0_A0_EP1_FIFO_CONFIG	(USB_DEVICE_MSK_FIFO_ENABLE | ((HS_C1_I0_A0_EP1_BLKSIZE - 1) << 4) | ((HS_C1_I0_A0_EP1_BLKNO - 1) << 2) | HS_C1_I0_A0_EP1_TYPE)

				#if(((Bulk_Satus == Bulk_FIFO_SingleDir) && (OTG_AP_Satus == Bulk_AP)) || (OTG_AP_Satus != Bulk_AP))
				#define HS_C1_I0_A0_EP1_FIFO_MAP		(((1 - HS_C1_I0_A0_EP1_DIRECTION) << 4) | EP1)
				#elif(Bulk_Satus == Bulk_FIFO_BiDir)
				#define HS_C1_I0_A0_EP1_FIFO_MAP		((2 << 4) | EP1)
				#endif
				
				#define HS_C1_I0_A0_EP1_MAP			(HS_C1_I0_A0_EP1_FIFO_START |	(HS_C1_I0_A0_EP1_FIFO_START << 4)	| (MASK_F0 >> (4*HS_C1_I0_A0_EP1_DIRECTION)))
			#endif
			#if (HS_C1_I0_A0_EP_NUMBER >= 2)
				// EP2
				#if(((Bulk_Satus == Bulk_FIFO_SingleDir) && (OTG_AP_Satus == Bulk_AP)) || (OTG_AP_Satus != Bulk_AP))
				#define HS_C1_I0_A0_EP2_FIFO_START		(HS_C1_I0_A0_EP1_FIFO_START + HS_C1_I0_A0_EP1_FIFO_NO)
				#elif(Bulk_Satus == Bulk_FIFO_BiDir)
				#define HS_C1_I0_A0_EP2_FIFO_START		FIFO0
				#endif

				#define HS_C1_I0_A0_EP2_FIFO_NO		(HS_C1_I0_A0_EP2_BLKNO * HS_C1_I0_A0_EP2_BLKSIZE)
				#define HS_C1_I0_A0_EP2_FIFO_CONFIG	(USB_DEVICE_MSK_FIFO_ENABLE | ((HS_C1_I0_A0_EP2_BLKSIZE - 1) << 4) | ((HS_C1_I0_A0_EP2_BLKNO - 1) << 2) | HS_C1_I0_A0_EP2_TYPE)
				#define HS_C1_I0_A0_EP2_FIFO_MAP		(((1 - HS_C1_I0_A0_EP2_DIRECTION) << 4) | EP2)
				#define HS_C1_I0_A0_EP2_MAP			(HS_C1_I0_A0_EP2_FIFO_START |	(HS_C1_I0_A0_EP2_FIFO_START << 4)	| (MASK_F0 >> (4*HS_C1_I0_A0_EP2_DIRECTION)))
			#endif
			#if (HS_C1_I0_A0_EP_NUMBER >= 3)
				// EP3	(For Bulk Bi-direction test Interrupt IN Endpoint)
				#define HS_C1_I0_A0_EP3_FIFO_START		FIFO2
				#define HS_C1_I0_A0_EP3_FIFO_NO		(HS_C1_I0_A0_EP3_BLKNO * HS_C1_I0_A0_EP3_BLKSIZE)
				#define HS_C1_I0_A0_EP3_FIFO_CONFIG	(USB_DEVICE_MSK_FIFO_ENABLE | ((HS_C1_I0_A0_EP3_BLKSIZE - 1) << 4) | ((HS_C1_I0_A0_EP3_BLKNO - 1) << 2) | HS_C1_I0_A0_EP3_TYPE)
				#define HS_C1_I0_A0_EP3_FIFO_MAP		(((1 - HS_C1_I0_A0_EP3_DIRECTION) << 4) | EP3)
				#define HS_C1_I0_A0_EP3_MAP			(HS_C1_I0_A0_EP3_FIFO_START |	(HS_C1_I0_A0_EP3_FIFO_START << 4)	| (MASK_F0 >> (4*HS_C1_I0_A0_EP3_DIRECTION)))
			#endif
			#if (HS_C1_I0_A0_EP_NUMBER >= 4)
				// EP4	(For Bulk Bi-direction test Interrupt OUT Endpoint)
				#define HS_C1_I0_A0_EP4_FIFO_START		(HS_C1_I0_A0_EP3_FIFO_START + HS_C1_I0_A0_EP3_FIFO_NO)
				#define HS_C1_I0_A0_EP4_FIFO_NO		(HS_C1_I0_A0_EP4_BLKNO * HS_C1_I0_A0_EP4_BLKSIZE)
				#define HS_C1_I0_A0_EP4_FIFO_CONFIG	(USB_DEVICE_MSK_FIFO_ENABLE | ((HS_C1_I0_A0_EP4_BLKSIZE - 1) << 4) | ((HS_C1_I0_A0_EP4_BLKNO - 1) << 2) | HS_C1_I0_A0_EP4_TYPE)
				#define HS_C1_I0_A0_EP4_FIFO_MAP		(((1 - HS_C1_I0_A0_EP4_DIRECTION) << 4) | EP4)
				#define HS_C1_I0_A0_EP4_MAP			(HS_C1_I0_A0_EP4_FIFO_START |	(HS_C1_I0_A0_EP4_FIFO_START << 4)	| (MASK_F0 >> (4*HS_C1_I0_A0_EP4_DIRECTION)))
			#endif
		#endif
		
		#if (HS_C1_I0_ALT_NUMBER == 1)
			#define HS_C1_I0_ALT_LENGTH				(HS_C1_I0_A0_EP_LENGTH)
		#elif (HS_C1_I0_ALT_NUMBER == 2)
			#define HS_C1_I0_ALT_LENGTH				(HS_C1_I0_A0_EP_LENGTH + HS_C1_I0_A1_EP_LENGTH)
		#endif
	#endif

	#if (HS_C1_INTERFACE_NUMBER == 1)
		#define HS_C1_INTERFACE_LENGTH				(HS_C1_I0_ALT_LENGTH)
	#elif (HS_C1_INTERFACE_NUMBER == 2)
		#define HS_C1_INTERFACE_LENGTH				(HS_C1_I0_ALT_LENGTH + HS_C1_I1_ALT_LENGTH)
	#endif
#endif

	// Configuration Descriptor length in ROM code
#if (HS_CONFIGURATION_NUMBER >= 1)
    #define HS_C1_CONFIG_TOTAL_LENGTH				(CONFIG_LENGTH + INTERFACE_LENGTH +  HS_C1_INTERFACE_LENGTH)
#endif
#if (HS_CONFIGURATION_NUMBER >= 2)
	#define HS_C2_CONFIG_TOTAL_LENGTH				(CONFIG_LENGTH + INTERFACE_LENGTH +  HS_C2_INTERFACE_LENGTH)
#endif

	// Configuration Descriptor length in external SRAM
#if (HS_CONFIGURATION_NUMBER == 1)
	#define HS_MAX_CONFIG_TOTAL_LENGTH				(HS_C1_CONFIG_TOTAL_LENGTH)
#elif (HS_CONFIGURATION_NUMBER == 2)
	#if (HS_C1_CONFIG_TOTAL_LENGTH > HS_C2_CONFIG_TOTAL_LENGTH)
		#define HS_MAX_CONFIG_TOTAL_LENGTH			(HS_C1_CONFIG_TOTAL_LENGTH)
	#else
		#define HS_MAX_CONFIG_TOTAL_LENGTH			(HS_C2_CONFIG_TOTAL_LENGTH)
	#endif
#endif



#endif
