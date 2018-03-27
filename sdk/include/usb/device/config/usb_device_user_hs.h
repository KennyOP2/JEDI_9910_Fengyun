#ifndef USB_DEVICE_USER_HS_H
#define USB_DEVICE_USER_HS_H

#include "usb/device/usb_register.h"
#include "usb/device/config/usb_device_user.h"


#if(OTG_AP_Satus == Bulk_AP)
	// device configuration:
	#define HS_bDeviceClass         0X00
	#define HS_bDeviceSubClass      0X00
	#define HS_bDeviceProtocol      0X00
	#define HS_iManufacturer        0X01
	#define HS_iProduct             0X02
	#define HS_iSerialNumber        0X03
	#define HS_CONFIGURATION_NUMBER 0X01
	
	#if (HS_CONFIGURATION_NUMBER >= 0X01)
		// Configuration 0X01
		#define HS_C1_INTERFACE_NUMBER  0X01
		#define HS_C1                   0X01
		#define HS_C1_iConfiguration    0x00
		#define HS_C1_bmAttribute       0XC0
		#define HS_C1_iMaxPower         0x32
	
		#if (HS_C1_INTERFACE_NUMBER >= 0x01)
			// Interface 0
			#define HS_C1_I0_ALT_NUMBER    0X01
			#if (HS_C1_I0_ALT_NUMBER >= 0X01) 
				// AlternateSetting 0X00
				#define HS_C1_I0_A0_bInterfaceNumber   0X00
				#define HS_C1_I0_A0_bAlternateSetting  0X00

				#if (Bulk_Satus == Bulk_FIFO_SingleDir)
				#define HS_C1_I0_A0_EP_NUMBER          0X02
				#elif(Bulk_Satus == Bulk_FIFO_BiDir)
				#define HS_C1_I0_A0_EP_NUMBER          0X02 //0X04 by powei
				#endif
				
				#define HS_C1_I0_A0_bInterfaceClass    0X08
				#define HS_C1_I0_A0_bInterfaceSubClass 0X06
				#define HS_C1_I0_A0_bInterfaceProtocol 0X50
				#define HS_C1_I0_A0_iInterface         0X00
	
				#if (HS_C1_I0_A0_EP_NUMBER >= 0X01)
					//EP0X01
					#define HS_C1_I0_A0_EP1_BLKSIZE    BLK512BYTE
					#define HS_C1_I0_A0_EP1_BLKNO      DOUBLE_BLK
					#define HS_C1_I0_A0_EP1_DIRECTION  DIRECTION_IN
					#define HS_C1_I0_A0_EP1_TYPE       TF_TYPE_BULK
					#define HS_C1_I0_A0_EP1_MAX_PACKET 0x0200
					#define HS_C1_I0_A0_EP1_bInterval  00
				#endif
				#if (HS_C1_I0_A0_EP_NUMBER >= 0X02)
					//EP0X02
					#define HS_C1_I0_A0_EP2_BLKSIZE    BLK512BYTE
					#define HS_C1_I0_A0_EP2_BLKNO      DOUBLE_BLK
					#define HS_C1_I0_A0_EP2_DIRECTION  DIRECTION_OUT
					#define HS_C1_I0_A0_EP2_TYPE       TF_TYPE_BULK
					#define HS_C1_I0_A0_EP2_MAX_PACKET 0x0200
					#define HS_C1_I0_A0_EP2_bInterval  00
				#endif
				#if (HS_C1_I0_A0_EP_NUMBER >= 0X03)
					//EP0X03
					#define HS_C1_I0_A0_EP3_BLKSIZE    BLK512BYTE
					#define HS_C1_I0_A0_EP3_BLKNO      SINGLE_BLK
					#define HS_C1_I0_A0_EP3_DIRECTION  DIRECTION_IN
					#define HS_C1_I0_A0_EP3_TYPE       TF_TYPE_INTERRUPT
					#define HS_C1_I0_A0_EP3_MAX_PACKET 0x040
					#define HS_C1_I0_A0_EP3_bInterval  01
				#endif
				#if (HS_C1_I0_A0_EP_NUMBER >= 0X04)
					//EP0X04
					#define HS_C1_I0_A0_EP4_BLKSIZE    BLK512BYTE
					#define HS_C1_I0_A0_EP4_BLKNO      SINGLE_BLK
					#define HS_C1_I0_A0_EP4_DIRECTION  DIRECTION_OUT
					#define HS_C1_I0_A0_EP4_TYPE       TF_TYPE_INTERRUPT
					#define HS_C1_I0_A0_EP4_MAX_PACKET 0x040
					#define HS_C1_I0_A0_EP4_bInterval  01
				#endif
			#endif
		#endif
	#endif
	
#elif(OTG_AP_Satus == Interrupt_AP)
	// device configuration:
	#define HS_bDeviceClass         0X00
	#define HS_bDeviceSubClass      0X00
	#define HS_bDeviceProtocol      0X00
	#define HS_iManufacturer        0X01
	#define HS_iProduct             0X02
	#define HS_iSerialNumber        0X03
	#define HS_CONFIGURATION_NUMBER 0X01
	
	#if (HS_CONFIGURATION_NUMBER >= 0X01)
		// Configuration 0X01
		#define HS_C1_INTERFACE_NUMBER  0X01
		#define HS_C1                   0X01
		#define HS_C1_iConfiguration    0X30
		#define HS_C1_bmAttribute       0XC0
		#define HS_C1_iMaxPower         0X32
	
		#if (HS_C1_INTERFACE_NUMBER >= 0x01)
			// Interface 0
			#define HS_C1_I0_ALT_NUMBER    0X01
			#if (HS_C1_I0_ALT_NUMBER >= 0X01) 
				// AlternateSetting 0X00
				#define HS_C1_I0_A0_bInterfaceNumber   0X00
				#define HS_C1_I0_A0_bAlternateSetting  0X00
				#define HS_C1_I0_A0_EP_NUMBER          0X02
				#define HS_C1_I0_A0_bInterfaceClass    0X00
				#define HS_C1_I0_A0_bInterfaceSubClass 0X00
				#define HS_C1_I0_A0_bInterfaceProtocol 0X00
				#define HS_C1_I0_A0_iInterface         0X40
	
				#if (HS_C1_I0_A0_EP_NUMBER >= 0X01)
					//EP0X01
					#define HS_C1_I0_A0_EP1_BLKSIZE    BLK1024BYTE
					#define HS_C1_I0_A0_EP1_BLKNO      SINGLE_BLK
					#define HS_C1_I0_A0_EP1_DIRECTION  DIRECTION_IN
					#define HS_C1_I0_A0_EP1_TYPE       TF_TYPE_INTERRUPT
					#define HS_C1_I0_A0_EP1_MAX_PACKET 0x0400
					#define HS_C1_I0_A0_EP1_bInterval  01
				#endif
				#if (HS_C1_I0_A0_EP_NUMBER >= 0X02)
					//EP0X02
					#define HS_C1_I0_A0_EP2_BLKSIZE    BLK1024BYTE
					#define HS_C1_I0_A0_EP2_BLKNO      SINGLE_BLK
					#define HS_C1_I0_A0_EP2_DIRECTION  DIRECTION_OUT
					#define HS_C1_I0_A0_EP2_TYPE       TF_TYPE_INTERRUPT
					#define HS_C1_I0_A0_EP2_MAX_PACKET 0x0400
					#define HS_C1_I0_A0_EP2_bInterval  01
				#endif
			#endif
		#endif
	#endif
	
#elif(OTG_AP_Satus == IsochronousIN_AP)
	// device configuration:
	#define HS_bDeviceClass         0X00
	#define HS_bDeviceSubClass      0X00
	#define HS_bDeviceProtocol      0X00
	#define HS_iManufacturer        0X01
	#define HS_iProduct             0X02
	#define HS_iSerialNumber        0X03
	#define HS_CONFIGURATION_NUMBER 0X01
	
	#if (HS_CONFIGURATION_NUMBER >= 0X01)
		// Configuration 0X01
		#define HS_C1_INTERFACE_NUMBER  0X01
		#define HS_C1                   0X01
		#define HS_C1_iConfiguration    0X30
		#define HS_C1_bmAttribute       0XC0
		#define HS_C1_iMaxPower         0X32
	
		#if (HS_C1_INTERFACE_NUMBER >= 0x01)
			// Interface 0
			#define HS_C1_I0_ALT_NUMBER    0X01
			#if (HS_C1_I0_ALT_NUMBER >= 0X01) 
				// AlternateSetting 0X00
				#define HS_C1_I0_A0_bInterfaceNumber   0X00
				#define HS_C1_I0_A0_bAlternateSetting  0X00
				#define HS_C1_I0_A0_EP_NUMBER          0X01
				#define HS_C1_I0_A0_bInterfaceClass    0X00
				#define HS_C1_I0_A0_bInterfaceSubClass 0X00
				#define HS_C1_I0_A0_bInterfaceProtocol 0X00
				#define HS_C1_I0_A0_iInterface         0X40
	
				#if (HS_C1_I0_A0_EP_NUMBER >= 0X01)
					//EP0X01
					#define HS_C1_I0_A0_EP1_BLKSIZE    BLK512BYTE//BLK1024BYTE
					#define HS_C1_I0_A0_EP1_BLKNO      DOUBLE_BLK
					#define HS_C1_I0_A0_EP1_DIRECTION  DIRECTION_IN
					#define HS_C1_I0_A0_EP1_TYPE       TF_TYPE_ISOCHRONOUS
					#define HS_C1_I0_A0_EP1_MAX_PACKET 128//0x1400//0x400
					#define HS_C1_I0_A0_EP1_bInterval  01
				#endif
			#endif
		#endif
	#endif

#elif(OTG_AP_Satus == IsochronousOUT_AP)
	// device configuration:
	#define HS_bDeviceClass         0X00
	#define HS_bDeviceSubClass      0X00
	#define HS_bDeviceProtocol      0X00
	#define HS_iManufacturer        0X01
	#define HS_iProduct             0X02
	#define HS_iSerialNumber        0X03
	#define HS_CONFIGURATION_NUMBER 0X01
	
	#if (HS_CONFIGURATION_NUMBER >= 0X01)
		// Configuration 0X01
		#define HS_C1_INTERFACE_NUMBER  0X01
		#define HS_C1                   0X01
		#define HS_C1_iConfiguration    0X30
		#define HS_C1_bmAttribute       0XC0
		#define HS_C1_iMaxPower         0X32
	
		#if (HS_C1_INTERFACE_NUMBER >= 0x01)
			// Interface 0
			#define HS_C1_I0_ALT_NUMBER    0X01
			#if (HS_C1_I0_ALT_NUMBER >= 0X01) 
				// AlternateSetting 0X00
				#define HS_C1_I0_A0_bInterfaceNumber   0X00
				#define HS_C1_I0_A0_bAlternateSetting  0X00
				#define HS_C1_I0_A0_EP_NUMBER          0X01
				#define HS_C1_I0_A0_bInterfaceClass    0X00
				#define HS_C1_I0_A0_bInterfaceSubClass 0X00
				#define HS_C1_I0_A0_bInterfaceProtocol 0X00
				#define HS_C1_I0_A0_iInterface         0X40
	
				#if (HS_C1_I0_A0_EP_NUMBER >= 0X01)
					//EP0X01
					#define HS_C1_I0_A0_EP1_BLKSIZE    BLK512BYTE
					#define HS_C1_I0_A0_EP1_BLKNO      TRIBLE_BLK
					#define HS_C1_I0_A0_EP1_DIRECTION  DIRECTION_OUT
					#define HS_C1_I0_A0_EP1_TYPE       TF_TYPE_ISOCHRONOUS
					#define HS_C1_I0_A0_EP1_MAX_PACKET 128//0x200
					#define HS_C1_I0_A0_EP1_bInterval  01
				#endif
			#endif
		#endif
	#endif
#endif


#endif
