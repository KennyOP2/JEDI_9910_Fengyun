/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *
 *	Description: USB high & full speed descriptor table
 *
 *		Device Descriptor (High Speed)
 *		|
 *		-- 1th CONFIGURATION Descriptor
 *		|	|
 *		|	-- 0th INTERFACE
 *		|	|	|
 *		|	|	-- 0th Alternate setting Descriptor
 *		|	|	|	|
 *		|	|	|	-- 1th EP descriptor
 *		|	|	|	|
 *		|	|	|	-- 2th EP descriptor
 *		|	|	|	|
 *		|	|	|	-- 3th EP descriptor
 *		|	|	|
 *		|	|	-- 1th Alternate setting Descriptor
 *		|	|
 *		|	-- 1th INTERFACE
 *		|		|
 *		|		.
 *		|		.
 *		|
 *		-- 2th CONFIGURATION Descriptor
 *			|
 *			.
 *			.
 *
 */

//=============================================================================
//                              Include Files
//=============================================================================

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Private Structure Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
// High speed Configuration
MMP_UINT8 u8OTGHSDeviceDescriptor[DEVICE_LENGTH] =
{
	//	DEVICE descriptor
	DEVICE_LENGTH,					// bLength
	DT_DEVICE,						// bDescriptorType
	mLowByte(HS_USB_SPEC_VER),			// bcdUSB
	mHighByte(HS_USB_SPEC_VER),
	HS_bDeviceClass,			    // bDeviceClass
	HS_bDeviceSubClass,			    // bDeviceSubClass
	HS_bDeviceProtocol,			    // bDeviceProtocol
	EP0MAXPACKETSIZE,				// bMaxPacketSize0
	mLowByte(HS_VENDOR_ID),			// idVendor
	mHighByte(HS_VENDOR_ID),
	mLowByte(HS_PRODUCT_ID),			// idProduct
	mHighByte(HS_PRODUCT_ID),
	mLowByte(HS_DEVICE_RELEASE_NO),	// bcdDeviceReleaseNumber
	mHighByte(HS_DEVICE_RELEASE_NO),
	HS_iManufacturer,			    // iManufacturer
	HS_iProduct,				    // iProduct
	HS_iSerialNumber, 			    // iSerialNumber
	HS_CONFIGURATION_NUMBER			// bNumConfigurations
};

// High speed Configuration
#if (HS_CONFIGURATION_NUMBER >= 1)
// Configuration 1
MMP_UINT8 u8HSConfigOTGDescriptor01[HS_C1_CONFIG_TOTAL_LENGTH] =
{
	//	CONFIGURATION descriptor
	CONFIG_LENGTH,					// bLength
	DT_CONFIGURATION,				// bDescriptorType CONFIGURATION
	mLowByte(HS_C1_CONFIG_TOTAL_LENGTH),	// wTotalLength, include all descriptors
	mHighByte(HS_C1_CONFIG_TOTAL_LENGTH),
	HS_C1_INTERFACE_NUMBER,			// bNumInterface
	HS_C1,							// bConfigurationValue
	HS_C1_iConfiguration,			// iConfiguration
	HS_C1_bmAttribute,				// bmAttribute
									// D7: Reserved(set to one), D6: Self-powered, D5: Remote Wakeup, D4..0: Reserved(reset to zero)
	HS_C1_iMaxPower,				// iMaxPower (2mA / units)

	#if (HS_C1_INTERFACE_NUMBER >= 1)
		// Interface 0
		#if (HS_C1_I0_ALT_NUMBER >= 1)
			// Alternate Setting 0
			INTERFACE_LENGTH,				// bLength
			DT_INTERFACE,					// bDescriptorType INTERFACE
			HS_C1_I0_A0_bInterfaceNumber,      // bInterfaceNumber
			HS_C1_I0_A0_bAlternateSetting,	    // bAlternateSetting
			HS_C1_I0_A0_EP_NUMBER,			// bNumEndpoints(excluding endpoint zero)
			HS_C1_I0_A0_bInterfaceClass,	// bInterfaceClass
			HS_C1_I0_A0_bInterfaceSubClass,// bInterfaceSubClass
			HS_C1_I0_A0_bInterfaceProtocol,// bInterfaceProtocol
			HS_C1_I0_A0_iInterface,		// iInterface

			#if (HS_C1_I0_A0_EP_NUMBER >= 1)
				// EP1
				EP_LENGTH,						// bLength
				DT_ENDPOINT,					// bDescriptorType ENDPOINT
				(((1 - HS_C1_I0_A0_EP1_DIRECTION) << 7) | EP1),	// bEndpointAddress
									// D7: Direction, 1=IN, 0=OUT
									// D6..4: Reserved(reset to zero), D3..0: The endpointer number
				HS_C1_I0_A0_EP1_TYPE,			// bmAttributes
									// D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
									// if not an isochronous endpoint, D7..2 are Reserved
				mLowByte(HS_C1_I0_A0_EP1_MAX_PACKET),	// wMaxPacketSize
				mHighByte(HS_C1_I0_A0_EP1_MAX_PACKET),
				HS_C1_I0_A0_EP1_bInterval,			// Interval for polling endpoint for data transfers.
			#endif
			#if (HS_C1_I0_A0_EP_NUMBER >= 2)
				// EP2
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A0_EP2_DIRECTION) << 7) | EP2),
				HS_C1_I0_A0_EP2_TYPE,	
				mLowByte(HS_C1_I0_A0_EP2_MAX_PACKET),
				mHighByte(HS_C1_I0_A0_EP2_MAX_PACKET),
				HS_C1_I0_A0_EP2_bInterval,
			#endif
			#if (HS_C1_I0_A0_EP_NUMBER >= 3)
				// EP3
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A0_EP3_DIRECTION) << 7) | EP3),
				HS_C1_I0_A0_EP3_TYPE,	
				mLowByte(HS_C1_I0_A0_EP3_MAX_PACKET),
				mHighByte(HS_C1_I0_A0_EP3_MAX_PACKET),
				HS_C1_I0_A0_EP3_bInterval,
			#endif
			#if (HS_C1_I0_A0_EP_NUMBER >= 4)
				// EP4
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A0_EP4_DIRECTION) << 7) | EP4),
				HS_C1_I0_A0_EP4_TYPE,	
				mLowByte(HS_C1_I0_A0_EP4_MAX_PACKET),
				mHighByte(HS_C1_I0_A0_EP4_MAX_PACKET),
				HS_C1_I0_A0_EP4_bInterval,
			#endif
			#if (HS_C1_I0_A0_EP_NUMBER >= 5)
				// EP5
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A0_EP5_DIRECTION) << 7) | EP5),
				HS_C1_I0_A0_EP5_TYPE,	
				mLowByte(HS_C1_I0_A0_EP5_MAX_PACKET),
				mHighByte(HS_C1_I0_A0_EP5_MAX_PACKET),
				HS_C1_I0_A0_EP5_bInterval,
			#endif
			#if (HS_C1_I0_A0_EP_NUMBER >= 6)
				// EP6
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A0_EP6_DIRECTION) << 7) | EP6),
				HS_C1_I0_A0_EP6_TYPE,	
				mLowByte(HS_C1_I0_A0_EP6_MAX_PACKET),
				mHighByte(HS_C1_I0_A0_EP6_MAX_PACKET),
				HS_C1_I0_A0_EP6_bInterval,
			#endif
			#if (HS_C1_I0_A0_EP_NUMBER >= 7)
				// EP7
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A0_EP7_DIRECTION) << 7) | EP7),
				HS_C1_I0_A0_EP7_TYPE,	
				mLowByte(HS_C1_I0_A0_EP7_MAX_PACKET),
				mHighByte(HS_C1_I0_A0_EP7_MAX_PACKET),
				HS_C1_I0_A0_EP7_bInterval,
			#endif
			#if (HS_C1_I0_A0_EP_NUMBER >= 8)
				// EP8
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A0_EP8_DIRECTION) << 7) | EP8),
				HS_C1_I0_A0_EP8_TYPE,	
				mLowByte(HS_C1_I0_A0_EP8_MAX_PACKET),
				mHighByte(HS_C1_I0_A0_EP8_MAX_PACKET),
				HS_C1_I0_A0_EP8_bInterval,
			#endif
			#if (HS_C1_I0_A0_EP_NUMBER >= 9)
				// EP9
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A0_EP9_DIRECTION) << 7) | EP9),
				HS_C1_I0_A0_EP9_TYPE,	
				mLowByte(HS_C1_I0_A0_EP9_MAX_PACKET),
				mHighByte(HS_C1_I0_A0_EP9_MAX_PACKET),
				HS_C1_I0_A0_EP9_bInterval,
			#endif
			#if (HS_C1_I0_A0_EP_NUMBER >= 10)
				// EP10
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A0_EP10_DIRECTION) << 7) | EP10),
				HS_C1_I0_A0_EP10_TYPE,	
				mLowByte(HS_C1_I0_A0_EP10_MAX_PACKET),
				mHighByte(HS_C1_I0_A0_EP10_MAX_PACKET),
				HS_C1_I0_A0_EP10_bInterval,
			#endif
			#if (HS_C1_I0_A0_EP_NUMBER >= 11)
				// EP11
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A0_EP11_DIRECTION) << 7) | EP11),
				HS_C1_I0_A0_EP11_TYPE,	
				mLowByte(HS_C1_I0_A0_EP11_MAX_PACKET),
				mHighByte(HS_C1_I0_A0_EP11_MAX_PACKET),
				HS_C1_I0_A0_EP11_bInterval,
			#endif
			#if (HS_C1_I0_A0_EP_NUMBER >= 12)
				// EP12
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A0_EP12_DIRECTION) << 7) | EP12),
				HS_C1_I0_A0_EP12_TYPE,	
				mLowByte(HS_C1_I0_A0_EP12_MAX_PACKET),
				mHighByte(HS_C1_I0_A0_EP12_MAX_PACKET),
				HS_C1_I0_A0_EP12_bInterval,
			#endif
			#if (HS_C1_I0_A0_EP_NUMBER >= 13)
				// EP13
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A0_EP13_DIRECTION) << 7) | EP13),
				HS_C1_I0_A0_EP13_TYPE,	
				mLowByte(HS_C1_I0_A0_EP13_MAX_PACKET),
				mHighByte(HS_C1_I0_A0_EP13_MAX_PACKET),
				HS_C1_I0_A0_EP13_bInterval,
			#endif
			#if (HS_C1_I0_A0_EP_NUMBER >= 14)
				// EP14
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A0_EP14_DIRECTION) << 7) | EP14),
				HS_C1_I0_A0_EP14_TYPE,	
				mLowByte(HS_C1_I0_A0_EP14_MAX_PACKET),
				mHighByte(HS_C1_I0_A0_EP14_MAX_PACKET),
				HS_C1_I0_A0_EP14_bInterval,
			#endif
			#if (HS_C1_I0_A0_EP_NUMBER >= 15)
				// EP15
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A0_EP15_DIRECTION) << 7) | EP15),
				HS_C1_I0_A0_EP15_TYPE,	
				mLowByte(HS_C1_I0_A0_EP15_MAX_PACKET),
				mHighByte(HS_C1_I0_A0_EP15_MAX_PACKET),
				HS_C1_I0_A0_EP15_bInterval,
			#endif
		#endif
		#if (HS_C1_I0_ALT_NUMBER >= 2)
			// Alternate Setting 0
			INTERFACE_LENGTH,				// bLength
			DT_INTERFACE,					// bDescriptorType INTERFACE
			HS_C1_I0_A1_bInterfaceNumber,   // bInterfaceNumber
			HS_C1_I0_A1_bAlternateSetting,	// bAlternateSetting
			HS_C1_I0_A1_EP_NUMBER,			// bNumEndpoints(excluding endpoint zero)
			HS_C1_I0_A1_bInterfaceClass,	// bInterfaceClass
			HS_C1_I0_A1_bInterfaceSubClass,// bInterfaceSubClass
			HS_C1_I0_A1_bInterfaceProtocol,// bInterfaceProtocol
			HS_C1_I0_A1_iInterface,		// iInterface

			#if (HS_C1_I0_A1_EP_NUMBER >= 1)
				// EP1
				EP_LENGTH,						// bLength
				DT_ENDPOINT,					// bDescriptorType ENDPOINT
				(((1 - HS_C1_I0_A1_EP1_DIRECTION) << 7) | EP1),	// bEndpointAddress
									// D7: Direction, 1=IN, 0=OUT
									// D6..4: Reserved(reset to zero), D3..0: The endpointer number
				HS_C1_I0_A1_EP1_TYPE,			// bmAttributes
									// D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
									// if not an isochronous endpoint, D7..2 are Reserved
				mLowByte(HS_C1_I0_A1_EP1_MAX_PACKET),	// wMaxPacketSize
				mHighByte(HS_C1_I0_A1_EP1_MAX_PACKET),
				HS_C1_I0_A1_EP1_bInterval,			// Interval for polling endpoint for data transfers.
			#endif
			#if (HS_C1_I0_A1_EP_NUMBER >= 2)
				// EP2
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A1_EP2_DIRECTION) << 7) | EP2),
				HS_C1_I0_A1_EP2_TYPE,	
				mLowByte(HS_C1_I0_A1_EP2_MAX_PACKET),
				mHighByte(HS_C1_I0_A1_EP2_MAX_PACKET),
				HS_C1_I0_A1_EP2_bInterval,
			#endif
			#if (HS_C1_I0_A1_EP_NUMBER >= 3)
				// EP3
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A1_EP3_DIRECTION) << 7) | EP3),
				HS_C1_I0_A1_EP3_TYPE,	
				mLowByte(HS_C1_I0_A1_EP3_MAX_PACKET),
				mHighByte(HS_C1_I0_A1_EP3_MAX_PACKET),
				HS_C1_I0_A1_EP3_bInterval,
			#endif
			#if (HS_C1_I0_A1_EP_NUMBER >= 4)
				// EP4
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A1_EP4_DIRECTION) << 7) | EP4),
				HS_C1_I0_A1_EP4_TYPE,	
				mLowByte(HS_C1_I0_A1_EP4_MAX_PACKET),
				mHighByte(HS_C1_I0_A1_EP4_MAX_PACKET),
				HS_C1_I0_A1_EP4_bInterval,
			#endif
			#if (HS_C1_I0_A1_EP_NUMBER >= 5)
				// EP5
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A1_EP5_DIRECTION) << 7) | EP5),
				HS_C1_I0_A1_EP5_TYPE,	
				mLowByte(HS_C1_I0_A1_EP5_MAX_PACKET),
				mHighByte(HS_C1_I0_A1_EP5_MAX_PACKET),
				HS_C1_I0_A1_EP5_bInterval,
			#endif
			#if (HS_C1_I0_A1_EP_NUMBER >= 6)
				// EP6
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A1_EP6_DIRECTION) << 7) | EP6),
				HS_C1_I0_A1_EP6_TYPE,	
				mLowByte(HS_C1_I0_A1_EP6_MAX_PACKET),
				mHighByte(HS_C1_I0_A1_EP6_MAX_PACKET),
				HS_C1_I0_A1_EP6_bInterval,
			#endif
			#if (HS_C1_I0_A1_EP_NUMBER >= 7)
				// EP7
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A1_EP7_DIRECTION) << 7) | EP7),
				HS_C1_I0_A1_EP7_TYPE,	
				mLowByte(HS_C1_I0_A1_EP7_MAX_PACKET),
				mHighByte(HS_C1_I0_A1_EP7_MAX_PACKET),
				HS_C1_I0_A1_EP7_bInterval,
			#endif
			#if (HS_C1_I0_A1_EP_NUMBER >= 8)
				// EP8
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A1_EP8_DIRECTION) << 7) | EP8),
				HS_C1_I0_A1_EP8_TYPE,	
				mLowByte(HS_C1_I0_A1_EP8_MAX_PACKET),
				mHighByte(HS_C1_I0_A1_EP8_MAX_PACKET),
				HS_C1_I0_A1_EP8_bInterval,
			#endif
			#if (HS_C1_I0_A1_EP_NUMBER >= 9)
				// EP9
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A1_EP9_DIRECTION) << 7) | EP9),
				HS_C1_I0_A1_EP9_TYPE,	
				mLowByte(HS_C1_I0_A1_EP9_MAX_PACKET),
				mHighByte(HS_C1_I0_A1_EP9_MAX_PACKET),
				HS_C1_I0_A1_EP9_bInterval,
			#endif
			#if (HS_C1_I0_A1_EP_NUMBER >= 10)
				// EP10
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A1_EP10_DIRECTION) << 7) | EP10),
				HS_C1_I0_A1_EP10_TYPE,	
				mLowByte(HS_C1_I0_A1_EP10_MAX_PACKET),
				mHighByte(HS_C1_I0_A1_EP10_MAX_PACKET),
				HS_C1_I0_A1_EP10_bInterval,
			#endif
			#if (HS_C1_I0_A1_EP_NUMBER >= 11)
				// EP11
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A1_EP11_DIRECTION) << 7) | EP11),
				HS_C1_I0_A1_EP11_TYPE,	
				mLowByte(HS_C1_I0_A1_EP11_MAX_PACKET),
				mHighByte(HS_C1_I0_A1_EP11_MAX_PACKET),
				HS_C1_I0_A1_EP11_bInterval,
			#endif
			#if (HS_C1_I0_A1_EP_NUMBER >= 12)
				// EP12
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A1_EP12_DIRECTION) << 7) | EP12),
				HS_C1_I0_A1_EP12_TYPE,	
				mLowByte(HS_C1_I0_A1_EP12_MAX_PACKET),
				mHighByte(HS_C1_I0_A1_EP12_MAX_PACKET),
				HS_C1_I0_A1_EP12_bInterval,
			#endif
			#if (HS_C1_I0_A1_EP_NUMBER >= 13)
				// EP13
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A1_EP13_DIRECTION) << 7) | EP13),
				HS_C1_I0_A1_EP13_TYPE,	
				mLowByte(HS_C1_I0_A1_EP13_MAX_PACKET),
				mHighByte(HS_C1_I0_A1_EP13_MAX_PACKET),
				HS_C1_I0_A1_EP13_bInterval,
			#endif
			#if (HS_C1_I0_A1_EP_NUMBER >= 14)
				// EP14
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A1_EP14_DIRECTION) << 7) | EP14),
				HS_C1_I0_A1_EP14_TYPE,	
				mLowByte(HS_C1_I0_A1_EP14_MAX_PACKET),
				mHighByte(HS_C1_I0_A1_EP14_MAX_PACKET),
				HS_C1_I0_A1_EP14_bInterval,
			#endif
			#if (HS_C1_I0_A1_EP_NUMBER >= 15)
				// EP15
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I0_A1_EP15_DIRECTION) << 7) | EP15),
				HS_C1_I0_A1_EP15_TYPE,	
				mLowByte(HS_C1_I0_A1_EP15_MAX_PACKET),
				mHighByte(HS_C1_I0_A1_EP15_MAX_PACKET),
				HS_C1_I0_A1_EP15_bInterval,
			#endif
		#endif
	#endif

	#if (HS_C1_INTERFACE_NUMBER >= 2)
		// Interface 1
		#if (HS_C1_I1_ALT_NUMBER >= 1)
			// Alternate Setting 0
			INTERFACE_LENGTH,			// bLength
			DT_INTERFACE,				// bDescriptorType INTERFACE
			HS_C1_I1_A0_bInterfaceNumber,      // bInterfaceNumber
			HS_C1_I1_A0_bAlternateSetting,	    // bAlternateSetting
			HS_C1_I1_A0_EP_NUMBER,		// bNumEndpoints(excluding endpoint zero)
			HS_C1_I1_A0_bInterfaceClass,	// bInterfaceClass
			HS_C1_I1_A0_bInterfaceSubClass,// bInterfaceSubClass
			HS_C1_I1_A0_bInterfaceProtocol,// bInterfaceProtocol
			HS_C1_I1_A0_iInterface,		// iInterface

			#if (HS_C1_I1_A0_EP_NUMBER >= 1)
				// EP1
				EP_LENGTH,					// bLength
				DT_ENDPOINT,				// bDescriptorType ENDPOINT
				(((1 - HS_C1_I1_A0_EP1_DIRECTION) << 7) | EP1),	// bEndpointAddress
								// D7: Direction, 1=IN, 0=OUT
								// D6..4: Reserved(reset to zero), D3..0: The endpointer number
				HS_C1_I1_A0_EP1_TYPE,		// bmAttributes
								// D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
								// if not an isochronous endpoint, D7..2 are Reserved
				mLowByte(HS_C1_I1_A0_EP1_MAX_PACKET),	// wMaxPacketSize
				mHighByte(HS_C1_I1_A0_EP1_MAX_PACKET),
				HS_C1_I1_A0_EP1_bInterval,			// Interval for polling endpoint for data transfers.
			#endif
			#if (HS_C1_I1_A0_EP_NUMBER >= 2)
				// EP2
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A0_EP2_DIRECTION) << 7) | EP2),
				HS_C1_I1_A0_EP2_TYPE,	
				mLowByte(HS_C1_I1_A0_EP2_MAX_PACKET),
				mHighByte(HS_C1_I1_A0_EP2_MAX_PACKET),
				HS_C1_I1_A0_EP2_bInterval,
			#endif
			#if (HS_C1_I1_A0_EP_NUMBER >= 3)
				// EP3
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A0_EP3_DIRECTION) << 7) | EP3),
				HS_C1_I1_A0_EP3_TYPE,	
				mLowByte(HS_C1_I1_A0_EP3_MAX_PACKET),
				mHighByte(HS_C1_I1_A0_EP3_MAX_PACKET),
				HS_C1_I1_A0_EP3_bInterval,
			#endif
			#if (HS_C1_I1_A0_EP_NUMBER >= 4)
				// EP4
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A0_EP4_DIRECTION) << 7) | EP4),
				HS_C1_I1_A0_EP4_TYPE,	
				mLowByte(HS_C1_I1_A0_EP4_MAX_PACKET),
				mHighByte(HS_C1_I1_A0_EP4_MAX_PACKET),
				HS_C1_I1_A0_EP4_bInterval,
			#endif
			#if (HS_C1_I1_A0_EP_NUMBER >= 5)
				// EP5
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A0_EP5_DIRECTION) << 7) | EP5),
				HS_C1_I1_A0_EP5_TYPE,	
				mLowByte(HS_C1_I1_A0_EP5_MAX_PACKET),
				mHighByte(HS_C1_I1_A0_EP5_MAX_PACKET),
				HS_C1_I1_A0_EP5_bInterval,
			#endif
			#if (HS_C1_I1_A0_EP_NUMBER >= 6)
				// EP6
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A0_EP6_DIRECTION) << 7) | EP6),
				HS_C1_I1_A0_EP6_TYPE,	
				mLowByte(HS_C1_I1_A0_EP6_MAX_PACKET),
				mHighByte(HS_C1_I1_A0_EP6_MAX_PACKET),
				HS_C1_I1_A0_EP6_bInterval,
			#endif
			#if (HS_C1_I1_A0_EP_NUMBER >= 7)
				// EP7
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A0_EP7_DIRECTION) << 7) | EP7),
				HS_C1_I1_A0_EP7_TYPE,	
				mLowByte(HS_C1_I1_A0_EP7_MAX_PACKET),
				mHighByte(HS_C1_I1_A0_EP7_MAX_PACKET),
				HS_C1_I1_A0_EP7_bInterval,
			#endif
			#if (HS_C1_I1_A0_EP_NUMBER >= 8)
				// EP8
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A0_EP8_DIRECTION) << 7) | EP8),
				HS_C1_I1_A0_EP8_TYPE,	
				mLowByte(HS_C1_I1_A0_EP8_MAX_PACKET),
				mHighByte(HS_C1_I1_A0_EP8_MAX_PACKET),
				HS_C1_I1_A0_EP8_bInterval,
			#endif
			#if (HS_C1_I1_A0_EP_NUMBER >= 9)
				// EP9
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A0_EP9_DIRECTION) << 7) | EP9),
				HS_C1_I1_A0_EP9_TYPE,	
				mLowByte(HS_C1_I1_A0_EP9_MAX_PACKET),
				mHighByte(HS_C1_I1_A0_EP9_MAX_PACKET),
				HS_C1_I1_A0_EP9_bInterval,
			#endif
			#if (HS_C1_I1_A0_EP_NUMBER >= 10)
				// EP10
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A0_EP10_DIRECTION) << 7) | EP10),
				HS_C1_I1_A0_EP10_TYPE,	
				mLowByte(HS_C1_I1_A0_EP10_MAX_PACKET),
				mHighByte(HS_C1_I1_A0_EP10_MAX_PACKET),
				HS_C1_I1_A0_EP10_bInterval,
			#endif
			#if (HS_C1_I1_A0_EP_NUMBER >= 11)
				// EP11
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A0_EP11_DIRECTION) << 7) | EP11),
				HS_C1_I1_A0_EP11_TYPE,	
				mLowByte(HS_C1_I1_A0_EP11_MAX_PACKET),
				mHighByte(HS_C1_I1_A0_EP11_MAX_PACKET),
				HS_C1_I1_A0_EP11_bInterval,
			#endif
			#if (HS_C1_I1_A0_EP_NUMBER >= 12)
				// EP12
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A0_EP12_DIRECTION) << 7) | EP12),
				HS_C1_I1_A0_EP12_TYPE,	
				mLowByte(HS_C1_I1_A0_EP12_MAX_PACKET),
				mHighByte(HS_C1_I1_A0_EP12_MAX_PACKET),
				HS_C1_I1_A0_EP12_bInterval,
			#endif
			#if (HS_C1_I1_A0_EP_NUMBER >= 13)
				// EP13
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A0_EP13_DIRECTION) << 7) | EP13),
				HS_C1_I1_A0_EP13_TYPE,	
				mLowByte(HS_C1_I1_A0_EP13_MAX_PACKET),
				mHighByte(HS_C1_I1_A0_EP13_MAX_PACKET),
				HS_C1_I1_A0_EP13_bInterval,
			#endif
			#if (HS_C1_I1_A0_EP_NUMBER >= 14)
				// EP14
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A0_EP14_DIRECTION) << 7) | EP14),
				HS_C1_I1_A0_EP14_TYPE,	
				mLowByte(HS_C1_I1_A0_EP14_MAX_PACKET),
				mHighByte(HS_C1_I1_A0_EP14_MAX_PACKET),
				HS_C1_I1_A0_EP14_bInterval,
			#endif
			#if (HS_C1_I1_A0_EP_NUMBER >= 15)
				// EP15
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A0_EP15_DIRECTION) << 7) | EP15),
				HS_C1_I1_A0_EP15_TYPE,	
				mLowByte(HS_C1_I1_A0_EP15_MAX_PACKET),
				mHighByte(HS_C1_I1_A0_EP15_MAX_PACKET),
				HS_C1_I1_A0_EP15_bInterval,
			#endif
		#endif

		#if (HS_C1_I1_ALT_NUMBER >= 2)
			// Alternate Setting 0
			INTERFACE_LENGTH,			// bLength
			DT_INTERFACE,				// bDescriptorType INTERFACE
			HS_C1_I1_A1_bInterfaceNumber,      // bInterfaceNumber
			HS_C1_I1_A1_bAlternateSetting,	    // bAlternateSetting
			HS_C1_I1_A1_EP_NUMBER,		// bNumEndpoints(excluding endpoint zero)
			HS_C1_I1_A1_bInterfaceClass,	// bInterfaceClass
			HS_C1_I1_A1_bInterfaceSubClass,// bInterfaceSubClass
			HS_C1_I1_A1_bInterfaceProtocol,// bInterfaceProtocol
			HS_C1_I1_A1_iInterface,		// iInterface

			#if (HS_C1_I1_A1_EP_NUMBER >= 1)
				// EP1
				EP_LENGTH,					// bLength
				DT_ENDPOINT,				// bDescriptorType ENDPOINT
				(((1 - HS_C1_I1_A1_EP1_DIRECTION) << 7) | EP1),	// bEndpointAddress
								// D7: Direction, 1=IN, 0=OUT
								// D6..4: Reserved(reset to zero), D3..0: The endpointer number
				HS_C1_I1_A1_EP1_TYPE,		// bmAttributes
								// D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
								// if not an isochronous endpoint, D7..2 are Reserved
				mLowByte(HS_C1_I1_A1_EP1_MAX_PACKET),	// wMaxPacketSize
				mHighByte(HS_C1_I1_A1_EP1_MAX_PACKET),
				HS_C1_I1_A1_EP1_bInterval,			// Interval for polling endpoint for data transfers.
			#endif
			#if (HS_C1_I1_A1_EP_NUMBER >= 2)
				// EP2
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A1_EP2_DIRECTION) << 7) | EP2),
				HS_C1_I1_A1_EP2_TYPE,	
				mLowByte(HS_C1_I1_A1_EP2_MAX_PACKET),
				mHighByte(HS_C1_I1_A1_EP2_MAX_PACKET),
				HS_C1_I1_A1_EP2_bInterval,
			#endif
			#if (HS_C1_I1_A1_EP_NUMBER >= 3)
				// EP3
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A1_EP3_DIRECTION) << 7) | EP3),
				HS_C1_I1_A1_EP3_TYPE,	
				mLowByte(HS_C1_I1_A1_EP3_MAX_PACKET),
				mHighByte(HS_C1_I1_A1_EP3_MAX_PACKET),
				HS_C1_I1_A1_EP3_bInterval,
			#endif
			#if (HS_C1_I1_A1_EP_NUMBER >= 4)
				// EP4
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A1_EP4_DIRECTION) << 7) | EP4),
				HS_C1_I1_A1_EP4_TYPE,	
				mLowByte(HS_C1_I1_A1_EP4_MAX_PACKET),
				mHighByte(HS_C1_I1_A1_EP4_MAX_PACKET),
				HS_C1_I1_A1_EP4_bInterval,
			#endif
			#if (HS_C1_I1_A1_EP_NUMBER >= 5)
				// EP5
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A1_EP5_DIRECTION) << 7) | EP5),
				HS_C1_I1_A1_EP5_TYPE,	
				mLowByte(HS_C1_I1_A1_EP5_MAX_PACKET),
				mHighByte(HS_C1_I1_A1_EP5_MAX_PACKET),
				HS_C1_I1_A1_EP5_bInterval,
			#endif
			#if (HS_C1_I1_A1_EP_NUMBER >= 6)
				// EP6
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A1_EP6_DIRECTION) << 7) | EP6),
				HS_C1_I1_A1_EP6_TYPE,	
				mLowByte(HS_C1_I1_A1_EP6_MAX_PACKET),
				mHighByte(HS_C1_I1_A1_EP6_MAX_PACKET),
				HS_C1_I1_A1_EP6_bInterval,
			#endif
			#if (HS_C1_I1_A1_EP_NUMBER >= 7)
				// EP7
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A1_EP7_DIRECTION) << 7) | EP7),
				HS_C1_I1_A1_EP7_TYPE,	
				mLowByte(HS_C1_I1_A1_EP7_MAX_PACKET),
				mHighByte(HS_C1_I1_A1_EP7_MAX_PACKET),
				HS_C1_I1_A1_EP7_bInterval,
			#endif
			#if (HS_C1_I1_A1_EP_NUMBER >= 8)
				// EP8
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A1_EP8_DIRECTION) << 7) | EP8),
				HS_C1_I1_A1_EP8_TYPE,	
				mLowByte(HS_C1_I1_A1_EP8_MAX_PACKET),
				mHighByte(HS_C1_I1_A1_EP8_MAX_PACKET),
				HS_C1_I1_A1_EP8_bInterval,
			#endif
			#if (HS_C1_I1_A1_EP_NUMBER >= 9)
				// EP9
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A1_EP9_DIRECTION) << 7) | EP9),
				HS_C1_I1_A1_EP9_TYPE,	
				mLowByte(HS_C1_I1_A1_EP9_MAX_PACKET),
				mHighByte(HS_C1_I1_A1_EP9_MAX_PACKET),
				HS_C1_I1_A1_EP9_bInterval,
			#endif
			#if (HS_C1_I1_A1_EP_NUMBER >= 10)
				// EP10
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A1_EP10_DIRECTION) << 7) | EP10),
				HS_C1_I1_A1_EP10_TYPE,	
				mLowByte(HS_C1_I1_A1_EP10_MAX_PACKET),
				mHighByte(HS_C1_I1_A1_EP10_MAX_PACKET),
				HS_C1_I1_A1_EP10_bInterval,
			#endif
			#if (HS_C1_I1_A1_EP_NUMBER >= 11)
				// EP11
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A1_EP11_DIRECTION) << 7) | EP11),
				HS_C1_I1_A1_EP11_TYPE,	
				mLowByte(HS_C1_I1_A1_EP11_MAX_PACKET),
				mHighByte(HS_C1_I1_A1_EP11_MAX_PACKET),
				HS_C1_I1_A1_EP11_bInterval,
			#endif
			#if (HS_C1_I1_A1_EP_NUMBER >= 12)
				// EP12
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A1_EP12_DIRECTION) << 7) | EP12),
				HS_C1_I1_A1_EP12_TYPE,	
				mLowByte(HS_C1_I1_A1_EP12_MAX_PACKET),
				mHighByte(HS_C1_I1_A1_EP12_MAX_PACKET),
				HS_C1_I1_A1_EP12_bInterval,
			#endif
			#if (HS_C1_I1_A1_EP_NUMBER >= 13)
				// EP13
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A1_EP13_DIRECTION) << 7) | EP13),
				HS_C1_I1_A1_EP13_TYPE,	
				mLowByte(HS_C1_I1_A1_EP13_MAX_PACKET),
				mHighByte(HS_C1_I1_A1_EP13_MAX_PACKET),
				HS_C1_I1_A1_EP13_bInterval,
			#endif
			#if (HS_C1_I1_A1_EP_NUMBER >= 14)
				// EP14
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A1_EP14_DIRECTION) << 7) | EP14),
				HS_C1_I1_A1_EP14_TYPE,	
				mLowByte(HS_C1_I1_A1_EP14_MAX_PACKET),
				mHighByte(HS_C1_I1_A1_EP14_MAX_PACKET),
				HS_C1_I1_A1_EP14_bInterval,
			#endif
			#if (HS_C1_I1_A1_EP_NUMBER >= 15)
				// EP15
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C1_I1_A1_EP15_DIRECTION) << 7) | EP15),
				HS_C1_I1_A1_EP15_TYPE,	
				mLowByte(HS_C1_I1_A1_EP15_MAX_PACKET),
				mHighByte(HS_C1_I1_A1_EP15_MAX_PACKET),
				HS_C1_I1_A1_EP15_bInterval,
			#endif
		#endif

	#endif
/*
	OTG_LENGTH,               
	DT_OTG,                   
	FS_OTG_SUPPORT,   */
};
#endif

#if (HS_CONFIGURATION_NUMBER >= 2)
// Configuration 2
MMP_UINT8 u8HSConfigOTGDescriptor02[HS_C2_CONFIG_TOTAL_LENGTH] =
{
	//	CONFIGURATION descriptor
	CONFIG_LENGTH,					// bLength
	DT_CONFIGURATION,				// bDescriptorType CONFIGURATION
	mLowByte(HS_C2_CONFIG_TOTAL_LENGTH),	// wTotalLength, include all descriptors
	mHighByte(HS_C2_CONFIG_TOTAL_LENGTH),
	HS_C2_INTERFACE_NUMBER,			// bNumInterface
	HS_C2,							// bConfigurationValue
	HS_C2_iConfiguration,			// iConfiguration
	HS_C2_bmAttribute,				// bmAttribute
									// D7: Reserved(set to one), D6: Self-powered, D5: Remote Wakeup, D4..0: Reserved(reset to zero)
	HS_C2_iMaxPower,				// iMaxPower (2mA units)

	#if (HS_C2_INTERFACE_NUMBER >= 1)
		// Interface 0
		#if (HS_C2_I0_ALT_NUMBER >= 1)
			// Alternate Setting 0
			INTERFACE_LENGTH,				// bLength
			DT_INTERFACE,					// bDescriptorType INTERFACE
			HS_C2_I0_A0_bInterfaceNumber,      // bInterfaceNumber
			HS_C2_I0_A0_bAlternateSetting,	    // bAlternateSetting
			HS_C2_I0_A0_EP_NUMBER,			// bNumEndpoints(excluding endpoint zero)
			HS_C2_I0_A0_bInterfaceClass,	// bInterfaceClass
			HS_C2_I0_A0_bInterfaceSubClass,// bInterfaceSubClass
			HS_C2_I0_A0_bInterfaceProtocol,// bInterfaceProtocol
			HS_C2_I0_A0_iInterface,		// iInterface

			#if (HS_C2_I0_A0_EP_NUMBER >= 1)
				// EP1
				EP_LENGTH,						// bLength
				DT_ENDPOINT,					// bDescriptorType ENDPOINT
				(((1 - HS_C2_I0_A0_EP1_DIRECTION) << 7) | EP1),	// bEndpointAddress
									// D7: Direction, 1=IN, 0=OUT
									// D6..4: Reserved(reset to zero), D3..0: The endpointer number
				HS_C2_I0_A0_EP1_TYPE,			// bmAttributes
									// D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
									// if not an isochronous endpoint, D7..2 are Reserved
				mLowByte(HS_C2_I0_A0_EP1_MAX_PACKET),	// wMaxPacketSize
				mHighByte(HS_C2_I0_A0_EP1_MAX_PACKET),
				HS_C2_I0_A0_EP1_bInterval,							// Interval for polling endpoint for data transfers.
			#endif
			#if (HS_C2_I0_A0_EP_NUMBER >= 2)
				// EP2
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A0_EP2_DIRECTION) << 7) | EP2),
				HS_C2_I0_A0_EP2_TYPE,	
				mLowByte(HS_C2_I0_A0_EP2_MAX_PACKET),
				mHighByte(HS_C2_I0_A0_EP2_MAX_PACKET),
				HS_C2_I0_A0_EP2_bInterval,
			#endif
			#if (HS_C2_I0_A0_EP_NUMBER >= 3)
				// EP3
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A0_EP3_DIRECTION) << 7) | EP3),
				HS_C2_I0_A0_EP3_TYPE,	
				mLowByte(HS_C2_I0_A0_EP3_MAX_PACKET),
				mHighByte(HS_C2_I0_A0_EP3_MAX_PACKET),
				HS_C2_I0_A0_EP3_bInterval,
			#endif
			#if (HS_C2_I0_A0_EP_NUMBER >= 4)
				// EP4
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A0_EP4_DIRECTION) << 7) | EP4),
				HS_C2_I0_A0_EP4_TYPE,	
				mLowByte(HS_C2_I0_A0_EP4_MAX_PACKET),
				mHighByte(HS_C2_I0_A0_EP4_MAX_PACKET),
				HS_C2_I0_A0_EP4_bInterval,
			#endif
			#if (HS_C2_I0_A0_EP_NUMBER >= 5)
				// EP5
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A0_EP5_DIRECTION) << 7) | EP5),
				HS_C2_I0_A0_EP5_TYPE,	
				mLowByte(HS_C2_I0_A0_EP5_MAX_PACKET),
				mHighByte(HS_C2_I0_A0_EP5_MAX_PACKET),
				HS_C2_I0_A0_EP5_bInterval,
			#endif
			#if (HS_C2_I0_A0_EP_NUMBER >= 6)
				// EP6
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A0_EP6_DIRECTION) << 7) | EP6),
				HS_C2_I0_A0_EP6_TYPE,	
				mLowByte(HS_C2_I0_A0_EP6_MAX_PACKET),
				mHighByte(HS_C2_I0_A0_EP6_MAX_PACKET),
				HS_C2_I0_A0_EP6_bInterval,
			#endif
			#if (HS_C2_I0_A0_EP_NUMBER >= 7)
				// EP7
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A0_EP7_DIRECTION) << 7) | EP7),
				HS_C2_I0_A0_EP7_TYPE,	
				mLowByte(HS_C2_I0_A0_EP7_MAX_PACKET),
				mHighByte(HS_C2_I0_A0_EP7_MAX_PACKET),
				HS_C2_I0_A0_EP7_bInterval,
			#endif
			#if (HS_C2_I0_A0_EP_NUMBER >= 8)
				// EP8
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A0_EP8_DIRECTION) << 7) | EP8),
				HS_C2_I0_A0_EP8_TYPE,	
				mLowByte(HS_C2_I0_A0_EP8_MAX_PACKET),
				mHighByte(HS_C2_I0_A0_EP8_MAX_PACKET),
				HS_C2_I0_A0_EP8_bInterval,
			#endif
			#if (HS_C2_I0_A0_EP_NUMBER >= 9)
				// EP9
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A0_EP9_DIRECTION) << 7) | EP9),
				HS_C2_I0_A0_EP9_TYPE,	
				mLowByte(HS_C2_I0_A0_EP9_MAX_PACKET),
				mHighByte(HS_C2_I0_A0_EP9_MAX_PACKET),
				HS_C2_I0_A0_EP9_bInterval,
			#endif
			#if (HS_C2_I0_A0_EP_NUMBER >= 10)
				// EP10
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A0_EP10_DIRECTION) << 7) | EP10),
				HS_C2_I0_A0_EP10_TYPE,	
				mLowByte(HS_C2_I0_A0_EP10_MAX_PACKET),
				mHighByte(HS_C2_I0_A0_EP10_MAX_PACKET),
				HS_C2_I0_A0_EP10_bInterval,
			#endif
			#if (HS_C2_I0_A0_EP_NUMBER >= 11)
				// EP11
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A0_EP11_DIRECTION) << 7) | EP11),
				HS_C2_I0_A0_EP11_TYPE,	
				mLowByte(HS_C2_I0_A0_EP11_MAX_PACKET),
				mHighByte(HS_C2_I0_A0_EP11_MAX_PACKET),
				HS_C2_I0_A0_EP11_bInterval,
			#endif
			#if (HS_C2_I0_A0_EP_NUMBER >= 12)
				// EP12
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A0_EP12_DIRECTION) << 7) | EP12),
				HS_C2_I0_A0_EP12_TYPE,	
				mLowByte(HS_C2_I0_A0_EP12_MAX_PACKET),
				mHighByte(HS_C2_I0_A0_EP12_MAX_PACKET),
				HS_C2_I0_A0_EP12_bInterval,
			#endif
			#if (HS_C2_I0_A0_EP_NUMBER >= 13)
				// EP13
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A0_EP13_DIRECTION) << 7) | EP13),
				HS_C2_I0_A0_EP13_TYPE,	
				mLowByte(HS_C2_I0_A0_EP13_MAX_PACKET),
				mHighByte(HS_C2_I0_A0_EP13_MAX_PACKET),
				HS_C2_I0_A0_EP13_bInterval,
			#endif
			#if (HS_C2_I0_A0_EP_NUMBER >= 14)
				// EP14
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A0_EP14_DIRECTION) << 7) | EP14),
				HS_C2_I0_A0_EP14_TYPE,	
				mLowByte(HS_C2_I0_A0_EP14_MAX_PACKET),
				mHighByte(HS_C2_I0_A0_EP14_MAX_PACKET),
				HS_C2_I0_A0_EP14_bInterval,
			#endif
			#if (HS_C2_I0_A0_EP_NUMBER >= 15)
				// EP15
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A0_EP15_DIRECTION) << 7) | EP15),
				HS_C2_I0_A0_EP15_TYPE,	
				mLowByte(HS_C2_I0_A0_EP15_MAX_PACKET),
				mHighByte(HS_C2_I0_A0_EP15_MAX_PACKET),
				HS_C2_I0_A0_EP15_bInterval,
			#endif
		#endif

		#if (HS_C2_I0_ALT_NUMBER >= 2)  
			// Alternate Setting 0
			INTERFACE_LENGTH,				// bLength
			DT_INTERFACE,					// bDescriptorType INTERFACE
			HS_C2_I0_A1_bInterfaceNumber,      // bInterfaceNumber
			HS_C2_I0_A1_bAlternateSetting,	    // bAlternateSetting
			HS_C2_I0_A1_EP_NUMBER,			// bNumEndpoints(excluding endpoint zero)
			HS_C2_I0_A1_bInterfaceClass,	// bInterfaceClass
			HS_C2_I0_A1_bInterfaceSubClass,// bInterfaceSubClass
			HS_C2_I0_A1_bInterfaceProtocol,// bInterfaceProtocol
			HS_C2_I0_A1_iInterface,		// iInterface

			#if (HS_C2_I0_A1_EP_NUMBER >= 1)
				// EP1
				EP_LENGTH,						// bLength
				DT_ENDPOINT,					// bDescriptorType ENDPOINT
				(((1 - HS_C2_I0_A1_EP1_DIRECTION) << 7) | EP1),	// bEndpointAddress
									// D7: Direction, 1=IN, 0=OUT
									// D6..4: Reserved(reset to zero), D3..0: The endpointer number
				HS_C2_I0_A1_EP1_TYPE,			// bmAttributes
									// D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
									// if not an isochronous endpoint, D7..2 are Reserved
				mLowByte(HS_C2_I0_A1_EP1_MAX_PACKET),	// wMaxPacketSize
				mHighByte(HS_C2_I0_A1_EP1_MAX_PACKET),
				HS_C2_I0_A1_EP1_bInterval,							// Interval for polling endpoint for data transfers.
			#endif
			#if (HS_C2_I0_A1_EP_NUMBER >= 2)
				// EP2
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A1_EP2_DIRECTION) << 7) | EP2),
				HS_C2_I0_A1_EP2_TYPE,	
				mLowByte(HS_C2_I0_A1_EP2_MAX_PACKET),
				mHighByte(HS_C2_I0_A1_EP2_MAX_PACKET),
				HS_C2_I0_A1_EP2_bInterval,
			#endif
			#if (HS_C2_I0_A1_EP_NUMBER >= 3)
				// EP3
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A1_EP3_DIRECTION) << 7) | EP3),
				HS_C2_I0_A1_EP3_TYPE,	
				mLowByte(HS_C2_I0_A1_EP3_MAX_PACKET),
				mHighByte(HS_C2_I0_A1_EP3_MAX_PACKET),
				HS_C2_I0_A1_EP3_bInterval,
			#endif
			#if (HS_C2_I0_A1_EP_NUMBER >= 4)
				// EP4
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A1_EP4_DIRECTION) << 7) | EP4),
				HS_C2_I0_A1_EP4_TYPE,	
				mLowByte(HS_C2_I0_A1_EP4_MAX_PACKET),
				mHighByte(HS_C2_I0_A1_EP4_MAX_PACKET),
				HS_C2_I0_A1_EP4_bInterval,
			#endif
			#if (HS_C2_I0_A1_EP_NUMBER >= 5)
				// EP5
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A1_EP5_DIRECTION) << 7) | EP5),
				HS_C2_I0_A1_EP5_TYPE,	
				mLowByte(HS_C2_I0_A1_EP5_MAX_PACKET),
				mHighByte(HS_C2_I0_A1_EP5_MAX_PACKET),
				HS_C2_I0_A1_EP5_bInterval,
			#endif
			#if (HS_C2_I0_A1_EP_NUMBER >= 6)
				// EP6
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A1_EP6_DIRECTION) << 7) | EP6),
				HS_C2_I0_A1_EP6_TYPE,	
				mLowByte(HS_C2_I0_A1_EP6_MAX_PACKET),
				mHighByte(HS_C2_I0_A1_EP6_MAX_PACKET),
				HS_C2_I0_A1_EP6_bInterval,
			#endif
			#if (HS_C2_I0_A1_EP_NUMBER >= 7)
				// EP7
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A1_EP7_DIRECTION) << 7) | EP7),
				HS_C2_I0_A1_EP7_TYPE,	
				mLowByte(HS_C2_I0_A1_EP7_MAX_PACKET),
				mHighByte(HS_C2_I0_A1_EP7_MAX_PACKET),
				HS_C2_I0_A1_EP7_bInterval,
			#endif
			#if (HS_C2_I0_A1_EP_NUMBER >= 8)
				// EP8
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A1_EP8_DIRECTION) << 7) | EP8),
				HS_C2_I0_A1_EP8_TYPE,	
				mLowByte(HS_C2_I0_A1_EP8_MAX_PACKET),
				mHighByte(HS_C2_I0_A1_EP8_MAX_PACKET),
				HS_C2_I0_A1_EP8_bInterval,
			#endif
			#if (HS_C2_I0_A1_EP_NUMBER >= 9)
				// EP9
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A1_EP9_DIRECTION) << 7) | EP9),
				HS_C2_I0_A1_EP9_TYPE,	
				mLowByte(HS_C2_I0_A1_EP9_MAX_PACKET),
				mHighByte(HS_C2_I0_A1_EP9_MAX_PACKET),
				HS_C2_I0_A1_EP9_bInterval,
			#endif
			#if (HS_C2_I0_A1_EP_NUMBER >= 10)
				// EP10
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A1_EP10_DIRECTION) << 7) | EP10),
				HS_C2_I0_A1_EP10_TYPE,	
				mLowByte(HS_C2_I0_A1_EP10_MAX_PACKET),
				mHighByte(HS_C2_I0_A1_EP10_MAX_PACKET),
				HS_C2_I0_A1_EP10_bInterval,
			#endif
			#if (HS_C2_I0_A1_EP_NUMBER >= 11)
				// EP11
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A1_EP11_DIRECTION) << 7) | EP11),
				HS_C2_I0_A1_EP11_TYPE,	
				mLowByte(HS_C2_I0_A1_EP11_MAX_PACKET),
				mHighByte(HS_C2_I0_A1_EP11_MAX_PACKET),
				HS_C2_I0_A1_EP11_bInterval,
			#endif
			#if (HS_C2_I0_A1_EP_NUMBER >= 12)
				// EP12
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A1_EP12_DIRECTION) << 7) | EP12),
				HS_C2_I0_A1_EP12_TYPE,	
				mLowByte(HS_C2_I0_A1_EP12_MAX_PACKET),
				mHighByte(HS_C2_I0_A1_EP12_MAX_PACKET),
				HS_C2_I0_A1_EP12_bInterval,
			#endif
			#if (HS_C2_I0_A1_EP_NUMBER >= 13)
				// EP13
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A1_EP13_DIRECTION) << 7) | EP13),
				HS_C2_I0_A1_EP13_TYPE,	
				mLowByte(HS_C2_I0_A1_EP13_MAX_PACKET),
				mHighByte(HS_C2_I0_A1_EP13_MAX_PACKET),
				HS_C2_I0_A1_EP13_bInterval,
			#endif
			#if (HS_C2_I0_A1_EP_NUMBER >= 14)
				// EP14
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A1_EP14_DIRECTION) << 7) | EP14),
				HS_C2_I0_A1_EP14_TYPE,	
				mLowByte(HS_C2_I0_A1_EP14_MAX_PACKET),
				mHighByte(HS_C2_I0_A1_EP14_MAX_PACKET),
				HS_C2_I0_A1_EP14_bInterval,
			#endif
			#if (HS_C2_I0_A1_EP_NUMBER >= 15)
				// EP15
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I0_A1_EP15_DIRECTION) << 7) | EP15),
				HS_C2_I0_A1_EP15_TYPE,	
				mLowByte(HS_C2_I0_A1_EP15_MAX_PACKET),
				mHighByte(HS_C2_I0_A1_EP15_MAX_PACKET),
				HS_C2_I0_A1_EP15_bInterval,
			#endif
		#endif

	#endif

	#if (HS_C2_INTERFACE_NUMBER >= 2)
		// Interface 1
		#if (HS_C2_I1_ALT_NUMBER >= 1)
			// Alternate Setting 0
			INTERFACE_LENGTH,			// bLength
			DT_INTERFACE,				// bDescriptorType INTERFACE
			HS_C2_I1_A0_bInterfaceNumber,      // bInterfaceNumber
			HS_C2_I1_A0_bAlternateSetting,	    // bAlternateSetting
			HS_C2_I1_A0_EP_NUMBER,		// bNumEndpoints(excluding endpoint zero)
			HS_C2_I1_A0_bInterfaceClass,	// bInterfaceClass
			HS_C2_I1_A0_bInterfaceSubClass,// bInterfaceSubClass
			HS_C2_I1_A0_bInterfaceProtocol,// bInterfaceProtocol
			HS_C2_I1_A0_iInterface,		// iInterface

			#if (HS_C2_I1_A0_EP_NUMBER >= 1)
				// EP1
				EP_LENGTH,					// bLength
				DT_ENDPOINT,				// bDescriptorType ENDPOINT
				(((1 - HS_C2_I1_A0_EP1_DIRECTION) << 7) | EP1),	// bEndpointAddress
								// D7: Direction, 1=IN, 0=OUT
								// D6..4: Reserved(reset to zero), D3..0: The endpointer number
				HS_C2_I1_A0_EP1_TYPE,		// bmAttributes
								// D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
								// if not an isochronous endpoint, D7..2 are Reserved
				mLowByte(HS_C2_I1_A0_EP1_MAX_PACKET),	// wMaxPacketSize
				mHighByte(HS_C2_I1_A0_EP1_MAX_PACKET),
				HS_C2_I1_A0_EP1_bInterval,			// Interval for polling endpoint for data transfers.
			#endif
			#if (HS_C2_I1_A0_EP_NUMBER >= 2)
				// EP2
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A0_EP2_DIRECTION) << 7) | EP2),
				HS_C2_I1_A0_EP2_TYPE,	
				mLowByte(HS_C2_I1_A0_EP2_MAX_PACKET),
				mHighByte(HS_C2_I1_A0_EP2_MAX_PACKET),
				HS_C2_I1_A0_EP2_bInterval,
			#endif
			#if (HS_C2_I1_A0_EP_NUMBER >= 3)
				// EP3
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A0_EP3_DIRECTION) << 7) | EP3),
				HS_C2_I1_A0_EP3_TYPE,	
				mLowByte(HS_C2_I1_A0_EP3_MAX_PACKET),
				mHighByte(HS_C2_I1_A0_EP3_MAX_PACKET),
				HS_C2_I1_A0_EP3_bInterval,
			#endif
			#if (HS_C2_I1_A0_EP_NUMBER >= 4)
				// EP4
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A0_EP4_DIRECTION) << 7) | EP4),
				HS_C2_I1_A0_EP4_TYPE,	
				mLowByte(HS_C2_I1_A0_EP4_MAX_PACKET),
				mHighByte(HS_C2_I1_A0_EP4_MAX_PACKET),
				HS_C2_I1_A0_EP4_bInterval,
			#endif
			#if (HS_C2_I1_A0_EP_NUMBER >= 5)
				// EP5
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A0_EP5_DIRECTION) << 7) | EP5),
				HS_C2_I1_A0_EP5_TYPE,	
				mLowByte(HS_C2_I1_A0_EP5_MAX_PACKET),
				mHighByte(HS_C2_I1_A0_EP5_MAX_PACKET),
				HS_C2_I1_A0_EP5_bInterval,
			#endif
			#if (HS_C2_I1_A0_EP_NUMBER >= 6)
				// EP6
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A0_EP6_DIRECTION) << 7) | EP6),
				HS_C2_I1_A0_EP6_TYPE,	
				mLowByte(HS_C2_I1_A0_EP6_MAX_PACKET),
				mHighByte(HS_C2_I1_A0_EP6_MAX_PACKET),
				HS_C2_I1_A0_EP6_bInterval,
			#endif
			#if (HS_C2_I1_A0_EP_NUMBER >= 7)
				// EP7
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A0_EP7_DIRECTION) << 7) | EP7),
				HS_C2_I1_A0_EP7_TYPE,	
				mLowByte(HS_C2_I1_A0_EP7_MAX_PACKET),
				mHighByte(HS_C2_I1_A0_EP7_MAX_PACKET),
				HS_C2_I1_A0_EP7_bInterval,
			#endif
			#if (HS_C2_I1_A0_EP_NUMBER >= 8)
				// EP8
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A0_EP8_DIRECTION) << 7) | EP8),
				HS_C2_I1_A0_EP8_TYPE,	
				mLowByte(HS_C2_I1_A0_EP8_MAX_PACKET),
				mHighByte(HS_C2_I1_A0_EP8_MAX_PACKET),
				HS_C2_I1_A0_EP8_bInterval,
			#endif
			#if (HS_C2_I1_A0_EP_NUMBER >= 9)
				// EP9
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A0_EP9_DIRECTION) << 7) | EP9),
				HS_C2_I1_A0_EP9_TYPE,	
				mLowByte(HS_C2_I1_A0_EP9_MAX_PACKET),
				mHighByte(HS_C2_I1_A0_EP9_MAX_PACKET),
				HS_C2_I1_A0_EP9_bInterval,
			#endif
			#if (HS_C2_I1_A0_EP_NUMBER >= 10
				// EP10
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A0_EP10_DIRECTION) << 7) | EP10),
				HS_C2_I1_A0_EP10_TYPE,	
				mLowByte(HS_C2_I1_A0_EP10_MAX_PACKET),
				mHighByte(HS_C2_I1_A0_EP10_MAX_PACKET),
				HS_C2_I1_A0_EP10_bInterval,
			#endif
			#if (HS_C2_I1_A0_EP_NUMBER >= 11)
				// EP11
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A0_EP11_DIRECTION) << 7) | EP11),
				HS_C2_I1_A0_EP11_TYPE,	
				mLowByte(HS_C2_I1_A0_EP11_MAX_PACKET),
				mHighByte(HS_C2_I1_A0_EP11_MAX_PACKET),
				HS_C2_I1_A0_EP11_bInterval,
			#endif
			#if (HS_C2_I1_A0_EP_NUMBER >= 12)
				// EP12
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A0_EP12_DIRECTION) << 7) | EP12),
				HS_C2_I1_A0_EP12_TYPE,	
				mLowByte(HS_C2_I1_A0_EP12_MAX_PACKET),
				mHighByte(HS_C2_I1_A0_EP12_MAX_PACKET),
				HS_C2_I1_A0_EP12_bInterval,
			#endif
			#if (HS_C2_I1_A0_EP_NUMBER >= 13)
				// EP13
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A0_EP13_DIRECTION) << 7) | EP13),
				HS_C2_I1_A0_EP13_TYPE,	
				mLowByte(HS_C2_I1_A0_EP13_MAX_PACKET),
				mHighByte(HS_C2_I1_A0_EP13_MAX_PACKET),
				HS_C2_I1_A0_EP13_bInterval,
			#endif
			#if (HS_C2_I1_A0_EP_NUMBER >= 14)
				// EP14
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A0_EP14_DIRECTION) << 7) | EP14),
				HS_C2_I1_A0_EP14_TYPE,	
				mLowByte(HS_C2_I1_A0_EP14_MAX_PACKET),
				mHighByte(HS_C2_I1_A0_EP14_MAX_PACKET),
				HS_C2_I1_A0_EP14_bInterval,
			#endif
			#if (HS_C2_I1_A0_EP_NUMBER >= 15)
				// EP15
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A0_EP15_DIRECTION) << 7) | EP15),
				HS_C2_I1_A0_EP15_TYPE,	
				mLowByte(HS_C2_I1_A0_EP15_MAX_PACKET),
				mHighByte(HS_C2_I1_A0_EP15_MAX_PACKET),
				HS_C2_I1_A0_EP15_bInterval,
			#endif
		#endif

		#if (HS_C2_I1_ALT_NUMBER >= 2)
			// Alternate Setting 0
			INTERFACE_LENGTH,			// bLength
			DT_INTERFACE,				// bDescriptorType INTERFACE
			HS_C2_I1_A1_bInterfaceNumber,      // bInterfaceNumber
			HS_C2_I1_A1_bAlternateSetting,	    // bAlternateSetting
			HS_C2_I1_A1_EP_NUMBER,		// bNumEndpoints(excluding endpoint zero)
			HS_C2_I1_A1_bInterfaceClass,	// bInterfaceClass
			HS_C2_I1_A1_bInterfaceSubClass,// bInterfaceSubClass
			HS_C2_I1_A1_bInterfaceProtocol,// bInterfaceProtocol
			HS_C2_I1_A1_iInterface,		// iInterface

			#if (HS_C2_I1_A1_EP_NUMBER >= 1)
				// EP1
				EP_LENGTH,					// bLength
				DT_ENDPOINT,				// bDescriptorType ENDPOINT
				(((1 - HS_C2_I1_A1_EP1_DIRECTION) << 7) | EP1),	// bEndpointAddress
								// D7: Direction, 1=IN, 0=OUT
								// D6..4: Reserved(reset to zero), D3..0: The endpointer number
				HS_C2_I1_A1_EP1_TYPE,		// bmAttributes
								// D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
								// if not an isochronous endpoint, D7..2 are Reserved
				mLowByte(HS_C2_I1_A1_EP1_MAX_PACKET),	// wMaxPacketSize
				mHighByte(HS_C2_I1_A1_EP1_MAX_PACKET),
				HS_C2_I1_A1_EP1_bInterval,			// Interval for polling endpoint for data transfers.
			#endif
			#if (HS_C2_I1_A1_EP_NUMBER >= 2)
				// EP2
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A1_EP2_DIRECTION) << 7) | EP2),
				HS_C2_I1_A1_EP2_TYPE,	
				mLowByte(HS_C2_I1_A1_EP2_MAX_PACKET),
				mHighByte(HS_C2_I1_A1_EP2_MAX_PACKET),
				HS_C2_I1_A1_EP2_bInterval,
			#endif
			#if (HS_C2_I1_A1_EP_NUMBER >= 3)
				// EP3
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A1_EP3_DIRECTION) << 7) | EP3),
				HS_C2_I1_A1_EP3_TYPE,	
				mLowByte(HS_C2_I1_A1_EP3_MAX_PACKET),
				mHighByte(HS_C2_I1_A1_EP3_MAX_PACKET),
				HS_C2_I1_A1_EP3_bInterval,
			#endif
			#if (HS_C2_I1_A1_EP_NUMBER >= 4)
				// EP4
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A1_EP4_DIRECTION) << 7) | EP4),
				HS_C2_I1_A1_EP4_TYPE,	
				mLowByte(HS_C2_I1_A1_EP4_MAX_PACKET),
				mHighByte(HS_C2_I1_A1_EP4_MAX_PACKET),
				HS_C2_I1_A1_EP4_bInterval,
			#endif
			#if (HS_C2_I1_A1_EP_NUMBER >= 5)
				// EP5
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A1_EP5_DIRECTION) << 7) | EP5),
				HS_C2_I1_A1_EP5_TYPE,	
				mLowByte(HS_C2_I1_A1_EP5_MAX_PACKET),
				mHighByte(HS_C2_I1_A1_EP5_MAX_PACKET),
				HS_C2_I1_A1_EP5_bInterval,
			#endif
			#if (HS_C2_I1_A1_EP_NUMBER >= 6)
				// EP6
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A1_EP6_DIRECTION) << 7) | EP6),
				HS_C2_I1_A1_EP6_TYPE,	
				mLowByte(HS_C2_I1_A1_EP6_MAX_PACKET),
				mHighByte(HS_C2_I1_A1_EP6_MAX_PACKET),
				HS_C2_I1_A1_EP6_bInterval,
			#endif
			#if (HS_C2_I1_A1_EP_NUMBER >= 7)
				// EP7
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A1_EP7_DIRECTION) << 7) | EP7),
				HS_C2_I1_A1_EP7_TYPE,	
				mLowByte(HS_C2_I1_A1_EP7_MAX_PACKET),
				mHighByte(HS_C2_I1_A1_EP7_MAX_PACKET),
				HS_C2_I1_A1_EP7_bInterval,
			#endif
			#if (HS_C2_I1_A1_EP_NUMBER >= 8)
				// EP8
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A1_EP8_DIRECTION) << 7) | EP8),
				HS_C2_I1_A1_EP8_TYPE,	
				mLowByte(HS_C2_I1_A1_EP8_MAX_PACKET),
				mHighByte(HS_C2_I1_A1_EP8_MAX_PACKET),
				HS_C2_I1_A1_EP8_bInterval,
			#endif
			#if (HS_C2_I1_A1_EP_NUMBER >= 9)
				// EP9
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A1_EP9_DIRECTION) << 7) | EP9),
				HS_C2_I1_A1_EP9_TYPE,	
				mLowByte(HS_C2_I1_A1_EP9_MAX_PACKET),
				mHighByte(HS_C2_I1_A1_EP9_MAX_PACKET),
				HS_C2_I1_A1_EP9_bInterval,
			#endif
			#if (HS_C2_I1_A1_EP_NUMBER >= 10
				// EP10
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A1_EP10_DIRECTION) << 7) | EP10),
				HS_C2_I1_A1_EP10_TYPE,	
				mLowByte(HS_C2_I1_A1_EP10_MAX_PACKET),
				mHighByte(HS_C2_I1_A1_EP10_MAX_PACKET),
				HS_C2_I1_A1_EP10_bInterval,
			#endif
			#if (HS_C2_I1_A1_EP_NUMBER >= 11)
				// EP11
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A1_EP11_DIRECTION) << 7) | EP11),
				HS_C2_I1_A1_EP11_TYPE,	
				mLowByte(HS_C2_I1_A1_EP11_MAX_PACKET),
				mHighByte(HS_C2_I1_A1_EP11_MAX_PACKET),
				HS_C2_I1_A1_EP11_bInterval,
			#endif
			#if (HS_C2_I1_A1_EP_NUMBER >= 12)
				// EP12
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A1_EP12_DIRECTION) << 7) | EP12),
				HS_C2_I1_A1_EP12_TYPE,	
				mLowByte(HS_C2_I1_A1_EP12_MAX_PACKET),
				mHighByte(HS_C2_I1_A1_EP12_MAX_PACKET),
				HS_C2_I1_A1_EP12_bInterval,
			#endif
			#if (HS_C2_I1_A1_EP_NUMBER >= 13)
				// EP13
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A1_EP13_DIRECTION) << 7) | EP13),
				HS_C2_I1_A1_EP13_TYPE,	
				mLowByte(HS_C2_I1_A1_EP13_MAX_PACKET),
				mHighByte(HS_C2_I1_A1_EP13_MAX_PACKET),
				HS_C2_I1_A1_EP13_bInterval,
			#endif
			#if (HS_C2_I1_A1_EP_NUMBER >= 14)
				// EP14
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A1_EP14_DIRECTION) << 7) | EP14),
				HS_C2_I1_A1_EP14_TYPE,	
				mLowByte(HS_C2_I1_A1_EP14_MAX_PACKET),
				mHighByte(HS_C2_I1_A1_EP14_MAX_PACKET),
				HS_C2_I1_A1_EP14_bInterval,
			#endif
			#if (HS_C2_I1_A1_EP_NUMBER >= 15)
				// EP15
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - HS_C2_I1_A1_EP15_DIRECTION) << 7) | EP15),
				HS_C2_I1_A1_EP15_TYPE,	
				mLowByte(HS_C2_I1_A1_EP15_MAX_PACKET),
				mHighByte(HS_C2_I1_A1_EP15_MAX_PACKET),
				HS_C2_I1_A1_EP15_bInterval,
			#endif
		#endif

	#endif
};
#endif


// Full speed Configuration
MMP_UINT8 u8OTGFSDeviceDescriptor[DEVICE_LENGTH] =
{
	//	DEVICE descriptor : from 0
	DEVICE_LENGTH,					// bLength
	DT_DEVICE,						// bDescriptorType
	mLowByte(FS_USB_SPEC_VER),			// bcdUSB
	mHighByte(FS_USB_SPEC_VER),
	FS_bDeviceClass,			    // bDeviceClass
	FS_bDeviceSubClass,			    // bDeviceSubClass
	FS_bDeviceProtocol,			    // bDeviceProtocol
	EP0MAXPACKETSIZE,				// bMaxPacketSize0
	mLowByte(FS_VENDOR_ID),			// idVendor
	mHighByte(FS_VENDOR_ID),
	mLowByte(FS_PRODUCT_ID),			// idProduct
	mHighByte(FS_PRODUCT_ID),
	mLowByte(FS_DEVICE_RELEASE_NO),	// bcdDeviceReleaseNumber
	mHighByte(FS_DEVICE_RELEASE_NO),
	FS_iManufacturer,			    // iManufacturer
	FS_iProduct,				    // iProduct
	FS_iSerialNumber, 			    // iSerialNumber
	FS_CONFIGURATION_NUMBER			// bNumConfigurations
};

#if (FS_CONFIGURATION_NUMBER >= 1)
// Configuration 1
MMP_UINT8 u8FSConfigOTGDescriptor01[FS_C1_CONFIG_TOTAL_LENGTH] =
{
	//	CONFIGURATION descriptor
	CONFIG_LENGTH,					// bLength
	DT_CONFIGURATION,				// bDescriptorType CONFIGURATION
	mLowByte(FS_C1_CONFIG_TOTAL_LENGTH),	// wTotalLength, include all descriptors
	mHighByte(FS_C1_CONFIG_TOTAL_LENGTH),
	FS_C1_INTERFACE_NUMBER,			// bNumInterface
	FS_C1,							// bConfigurationValue
	FS_C1_iConfiguration,			// iConfiguration
	FS_C1_bmAttribute,				// bmAttribute
									// D7: Reserved(set to one), D6: Self-powered, D5: Remote Wakeup, D4..0: Reserved(reset to zero)
	FS_C1_iMaxPower,				// iMaxPower (2mA units)

	#if (FS_C1_INTERFACE_NUMBER >= 1)
		// Interface 0
		#if (FS_C1_I0_ALT_NUMBER >= 1)
			// Alternate Setting 0
			INTERFACE_LENGTH,				// bLength
			DT_INTERFACE,					// bDescriptorType INTERFACE
			FS_C1_I0_A0_bInterfaceNumber,   // bInterfaceNumber
			FS_C1_I0_A0_bAlternateSetting,	// bAlternateSetting
			FS_C1_I0_A0_EP_NUMBER,			// bNumEndpoints(excluding endpoint zero)
			FS_C1_I0_A0_bInterfaceClass,	// bInterfaceClass
			FS_C1_I0_A0_bInterfaceSubClass, // bInterfaceSubClass
			FS_C1_I0_A0_bInterfaceProtocol, // bInterfaceProtocol
			FS_C1_I0_A0_iInterface,		    // iInterface

			#if (FS_C1_I0_A0_EP_NUMBER >= 1)
				// EP1
				EP_LENGTH,						                // bLength
				DT_ENDPOINT,					                // bDescriptorType ENDPOINT
				(((1 - FS_C1_I0_A0_EP1_DIRECTION) << 7) | EP1),	// bEndpointAddress
									                            // D7: Direction, 1=IN, 0=OUT
									                            // D6..4: Reserved(reset to zero), D3..0: The endpointer number
				FS_C1_I0_A0_EP1_TYPE,			                // bmAttributes
									                            // D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
									                            // if not an isochronous endpoint, D7..2 are Reserved
				mLowByte(FS_C1_I0_A0_EP1_MAX_PACKET),	        // wMaxPacketSize
				mHighByte(FS_C1_I0_A0_EP1_MAX_PACKET),
				FS_C1_I0_A0_EP1_bInterval,						// Interval for polling endpoint for data transfers.
			#endif
			#if (FS_C1_I0_A0_EP_NUMBER >= 2)
				// EP2
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A0_EP2_DIRECTION) << 7) | EP2),
				FS_C1_I0_A0_EP2_TYPE,	
				mLowByte(FS_C1_I0_A0_EP2_MAX_PACKET),
				mHighByte(FS_C1_I0_A0_EP2_MAX_PACKET),
				FS_C1_I0_A0_EP2_bInterval,
			#endif
			#if (FS_C1_I0_A0_EP_NUMBER >= 3)
				// EP3
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A0_EP3_DIRECTION) << 7) | EP3),
				FS_C1_I0_A0_EP3_TYPE,	
				mLowByte(FS_C1_I0_A0_EP3_MAX_PACKET),
				mHighByte(FS_C1_I0_A0_EP3_MAX_PACKET),
				FS_C1_I0_A0_EP3_bInterval,
			#endif
			#if (FS_C1_I0_A0_EP_NUMBER >= 4)
				// EP4
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A0_EP4_DIRECTION) << 7) | EP4),
				FS_C1_I0_A0_EP4_TYPE,	
				mLowByte(FS_C1_I0_A0_EP4_MAX_PACKET),
				mHighByte(FS_C1_I0_A0_EP4_MAX_PACKET),
				FS_C1_I0_A0_EP4_bInterval,
			#endif
			#if (FS_C1_I0_A0_EP_NUMBER >= 5)
				// EP5
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A0_EP5_DIRECTION) << 7) | EP5),
				FS_C1_I0_A0_EP5_TYPE,	
				mLowByte(FS_C1_I0_A0_EP5_MAX_PACKET),
				mHighByte(FS_C1_I0_A0_EP5_MAX_PACKET),
				FS_C1_I0_A0_EP5_bInterval,
			#endif
			#if (FS_C1_I0_A0_EP_NUMBER >= 6)
				// EP6
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A0_EP6_DIRECTION) << 7) | EP6),
				FS_C1_I0_A0_EP6_TYPE,	
				mLowByte(FS_C1_I0_A0_EP6_MAX_PACKET),
				mHighByte(FS_C1_I0_A0_EP6_MAX_PACKET),
				FS_C1_I0_A0_EP6_bInterval,
			#endif
			#if (FS_C1_I0_A0_EP_NUMBER >= 7)
				// EP7
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A0_EP7_DIRECTION) << 7) | EP7),
				FS_C1_I0_A0_EP7_TYPE,	
				mLowByte(FS_C1_I0_A0_EP7_MAX_PACKET),
				mHighByte(FS_C1_I0_A0_EP7_MAX_PACKET),
				FS_C1_I0_A0_EP7_bInterval,
			#endif
			#if (FS_C1_I0_A0_EP_NUMBER >= 8)
				// EP8
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A0_EP8_DIRECTION) << 7) | EP8),
				FS_C1_I0_A0_EP8_TYPE,	
				mLowByte(FS_C1_I0_A0_EP8_MAX_PACKET),
				mHighByte(FS_C1_I0_A0_EP8_MAX_PACKET),
				FS_C1_I0_A0_EP8_bInterval,
			#endif
			#if (FS_C1_I0_A0_EP_NUMBER >= 9)
				// EP9
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A0_EP9_DIRECTION) << 7) | EP9),
				FS_C1_I0_A0_EP9_TYPE,	
				mLowByte(FS_C1_I0_A0_EP9_MAX_PACKET),
				mHighByte(FS_C1_I0_A0_EP9_MAX_PACKET),
				FS_C1_I0_A0_EP9_bInterval,
			#endif
			#if (FS_C1_I0_A0_EP_NUMBER >= 10)
				// EP10
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A0_EP10_DIRECTION) << 7) | EP10),
				FS_C1_I0_A0_EP10_TYPE,	
				mLowByte(FS_C1_I0_A0_EP10_MAX_PACKET),
				mHighByte(FS_C1_I0_A0_EP10_MAX_PACKET),
				FS_C1_I0_A0_EP10_bInterval,
			#endif
			#if (FS_C1_I0_A0_EP_NUMBER >= 11)
				// EP11
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A0_EP11_DIRECTION) << 7) | EP11),
				FS_C1_I0_A0_EP11_TYPE,	
				mLowByte(FS_C1_I0_A0_EP11_MAX_PACKET),
				mHighByte(FS_C1_I0_A0_EP11_MAX_PACKET),
				FS_C1_I0_A0_EP11_bInterval,
			#endif
			#if (FS_C1_I0_A0_EP_NUMBER >= 12)
				// EP12
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A0_EP12_DIRECTION) << 7) | EP12),
				FS_C1_I0_A0_EP12_TYPE,	
				mLowByte(FS_C1_I0_A0_EP12_MAX_PACKET),
				mHighByte(FS_C1_I0_A0_EP12_MAX_PACKET),
				FS_C1_I0_A0_EP12_bInterval,
			#endif
			#if (FS_C1_I0_A0_EP_NUMBER >= 13)
				// EP13
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A0_EP13_DIRECTION) << 7) | EP13),
				FS_C1_I0_A0_EP13_TYPE,	
				mLowByte(FS_C1_I0_A0_EP13_MAX_PACKET),
				mHighByte(FS_C1_I0_A0_EP13_MAX_PACKET),
				FS_C1_I0_A0_EP13_bInterval,
			#endif
			#if (FS_C1_I0_A0_EP_NUMBER >= 14)
				// EP14
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A0_EP14_DIRECTION) << 7) | EP14),
				FS_C1_I0_A0_EP14_TYPE,	
				mLowByte(FS_C1_I0_A0_EP14_MAX_PACKET),
				mHighByte(FS_C1_I0_A0_EP14_MAX_PACKET),
				FS_C1_I0_A0_EP14_bInterval,
			#endif
			#if (FS_C1_I0_A0_EP_NUMBER >= 15)
				// EP15
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A0_EP15_DIRECTION) << 7) | EP15),
				FS_C1_I0_A0_EP15_TYPE,	
				mLowByte(FS_C1_I0_A0_EP15_MAX_PACKET),
				mHighByte(FS_C1_I0_A0_EP15_MAX_PACKET),
				FS_C1_I0_A0_EP15_bInterval,
			#endif
		#endif

		#if (FS_C1_I0_ALT_NUMBER >= 2)
			// Alternate Setting 0
			INTERFACE_LENGTH,				// bLength
			DT_INTERFACE,					// bDescriptorType INTERFACE
			FS_C1_I0_A1_bInterfaceNumber,      // bInterfaceNumber
			FS_C1_I0_A1_bAlternateSetting,	    // bAlternateSetting
			FS_C1_I0_A1_EP_NUMBER,			// bNumEndpoints(excluding endpoint zero)
			FS_C1_I0_A1_bInterfaceClass,	// bInterfaceClass
			FS_C1_I0_A1_bInterfaceSubClass,// bInterfaceSubClass
			FS_C1_I0_A1_bInterfaceProtocol,// bInterfaceProtocol
			FS_C1_I0_A1_iInterface,		// iInterface

			#if (FS_C1_I0_A1_EP_NUMBER >= 1)
				// EP1
				EP_LENGTH,						// bLength
				DT_ENDPOINT,					// bDescriptorType ENDPOINT
				(((1 - FS_C1_I0_A1_EP1_DIRECTION) << 7) | EP1),	// bEndpointAddress
									// D7: Direction, 1=IN, 0=OUT
									// D6..4: Reserved(reset to zero), D3..0: The endpointer number
				FS_C1_I0_A1_EP1_TYPE,			// bmAttributes
									// D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
									// if not an isochronous endpoint, D7..2 are Reserved
				mLowByte(FS_C1_I0_A1_EP1_MAX_PACKET),	// wMaxPacketSize
				mHighByte(FS_C1_I0_A1_EP1_MAX_PACKET),
				FS_C1_I0_A1_EP1_bInterval,							// Interval for polling endpoint for data transfers.
			#endif
			#if (FS_C1_I0_A1_EP_NUMBER >= 2)
				// EP2
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A1_EP2_DIRECTION) << 7) | EP2),
				FS_C1_I0_A1_EP2_TYPE,	
				mLowByte(FS_C1_I0_A1_EP2_MAX_PACKET),
				mHighByte(FS_C1_I0_A1_EP2_MAX_PACKET),
				FS_C1_I0_A1_EP2_bInterval,
			#endif
			#if (FS_C1_I0_A1_EP_NUMBER >= 3)
				// EP3
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A1_EP3_DIRECTION) << 7) | EP3),
				FS_C1_I0_A1_EP3_TYPE,	
				mLowByte(FS_C1_I0_A1_EP3_MAX_PACKET),
				mHighByte(FS_C1_I0_A1_EP3_MAX_PACKET),
				FS_C1_I0_A1_EP3_bInterval,
			#endif
			#if (FS_C1_I0_A1_EP_NUMBER >= 4)
				// EP4
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A1_EP4_DIRECTION) << 7) | EP4),
				FS_C1_I0_A1_EP4_TYPE,	
				mLowByte(FS_C1_I0_A1_EP4_MAX_PACKET),
				mHighByte(FS_C1_I0_A1_EP4_MAX_PACKET),
				FS_C1_I0_A1_EP4_bInterval,
			#endif
			#if (FS_C1_I0_A1_EP_NUMBER >= 5)
				// EP5
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A1_EP5_DIRECTION) << 7) | EP5),
				FS_C1_I0_A1_EP5_TYPE,	
				mLowByte(FS_C1_I0_A1_EP5_MAX_PACKET),
				mHighByte(FS_C1_I0_A1_EP5_MAX_PACKET),
				FS_C1_I0_A1_EP5_bInterval,
			#endif
			#if (FS_C1_I0_A1_EP_NUMBER >= 6)
				// EP6
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A1_EP6_DIRECTION) << 7) | EP6),
				FS_C1_I0_A1_EP6_TYPE,	
				mLowByte(FS_C1_I0_A1_EP6_MAX_PACKET),
				mHighByte(FS_C1_I0_A1_EP6_MAX_PACKET),
				FS_C1_I0_A1_EP6_bInterval,
			#endif
			#if (FS_C1_I0_A1_EP_NUMBER >= 7)
				// EP7
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A1_EP7_DIRECTION) << 7) | EP7),
				FS_C1_I0_A1_EP7_TYPE,	
				mLowByte(FS_C1_I0_A1_EP7_MAX_PACKET),
				mHighByte(FS_C1_I0_A1_EP7_MAX_PACKET),
				FS_C1_I0_A1_EP7_bInterval,
			#endif
			#if (FS_C1_I0_A1_EP_NUMBER >= 8)
				// EP8
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A1_EP8_DIRECTION) << 7) | EP8),
				FS_C1_I0_A1_EP8_TYPE,	
				mLowByte(FS_C1_I0_A1_EP8_MAX_PACKET),
				mHighByte(FS_C1_I0_A1_EP8_MAX_PACKET),
				FS_C1_I0_A1_EP8_bInterval,
			#endif
			#if (FS_C1_I0_A1_EP_NUMBER >= 9)
				// EP9
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A1_EP9_DIRECTION) << 7) | EP9),
				FS_C1_I0_A1_EP9_TYPE,	
				mLowByte(FS_C1_I0_A1_EP9_MAX_PACKET),
				mHighByte(FS_C1_I0_A1_EP9_MAX_PACKET),
				FS_C1_I0_A1_EP9_bInterval,
			#endif
			#if (FS_C1_I0_A1_EP_NUMBER >= 10)
				// EP10
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A1_EP10_DIRECTION) << 7) | EP10),
				FS_C1_I0_A1_EP10_TYPE,	
				mLowByte(FS_C1_I0_A1_EP10_MAX_PACKET),
				mHighByte(FS_C1_I0_A1_EP10_MAX_PACKET),
				FS_C1_I0_A1_EP10_bInterval,
			#endif
			#if (FS_C1_I0_A1_EP_NUMBER >= 11)
				// EP11
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A1_EP11_DIRECTION) << 7) | EP11),
				FS_C1_I0_A1_EP11_TYPE,	
				mLowByte(FS_C1_I0_A1_EP11_MAX_PACKET),
				mHighByte(FS_C1_I0_A1_EP11_MAX_PACKET),
				FS_C1_I0_A1_EP11_bInterval,
			#endif
			#if (FS_C1_I0_A1_EP_NUMBER >= 12)
				// EP12
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A1_EP12_DIRECTION) << 7) | EP12),
				FS_C1_I0_A1_EP12_TYPE,	
				mLowByte(FS_C1_I0_A1_EP12_MAX_PACKET),
				mHighByte(FS_C1_I0_A1_EP12_MAX_PACKET),
				FS_C1_I0_A1_EP12_bInterval,
			#endif
			#if (FS_C1_I0_A1_EP_NUMBER >= 13)
				// EP13
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A1_EP13_DIRECTION) << 7) | EP13),
				FS_C1_I0_A1_EP13_TYPE,	
				mLowByte(FS_C1_I0_A1_EP13_MAX_PACKET),
				mHighByte(FS_C1_I0_A1_EP13_MAX_PACKET),
				FS_C1_I0_A1_EP13_bInterval,
			#endif
			#if (FS_C1_I0_A1_EP_NUMBER >= 14)
				// EP14
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A1_EP14_DIRECTION) << 7) | EP14),
				FS_C1_I0_A1_EP14_TYPE,	
				mLowByte(FS_C1_I0_A1_EP14_MAX_PACKET),
				mHighByte(FS_C1_I0_A1_EP14_MAX_PACKET),
				FS_C1_I0_A1_EP14_bInterval,
			#endif
			#if (FS_C1_I0_A1_EP_NUMBER >= 15)
				// EP15
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I0_A1_EP15_DIRECTION) << 7) | EP15),
				FS_C1_I0_A1_EP15_TYPE,	
				mLowByte(FS_C1_I0_A1_EP15_MAX_PACKET),
				mHighByte(FS_C1_I0_A1_EP15_MAX_PACKET),
				FS_C1_I0_A1_EP15_bInterval,
			#endif
		#endif
	#endif

	#if (FS_C1_INTERFACE_NUMBER >= 2)
		// Interface 1
		#if (FS_C1_I1_ALT_NUMBER >= 1)
			// Alternate Setting 0
			INTERFACE_LENGTH,			        // bLength
			DT_INTERFACE,				        // bDescriptorType INTERFACE
			FS_C1_I1_A0_bInterfaceNumber,       // bInterfaceNumber
			FS_C1_I1_A0_bAlternateSetting,	    // bAlternateSetting
			FS_C1_I1_A0_EP_NUMBER,		        // bNumEndpoints(excluding endpoint zero)
			FS_C1_I1_A0_bInterfaceClass,	    // bInterfaceClass
			FS_C1_I1_A0_bInterfaceSubClass,     // bInterfaceSubClass
			FS_C1_I1_A0_bInterfaceProtocol,     // bInterfaceProtocol
			FS_C1_I1_A0_iInterface,		        // iInterface


			#if (FS_C1_I1_A0_EP_NUMBER >= 1)
				// EP1
				EP_LENGTH,					// bLength
				DT_ENDPOINT,				// bDescriptorType ENDPOINT
				(((1 - FS_C1_I1_A0_EP1_DIRECTION) << 7) | EP1),	// bEndpointAddress
								// D7: Direction, 1=IN, 0=OUT
								// D6..4: Reserved(reset to zero), D3..0: The endpointer number
				FS_C1_I1_A0_EP1_TYPE,		// bmAttributes
								// D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
								// if not an isochronous endpoint, D7..2 are Reserved
				mLowByte(FS_C1_I1_A0_EP1_MAX_PACKET),				// wMaxPacketSize
				mHighByte(FS_C1_I1_A0_EP1_MAX_PACKET),
				FS_C1_I1_A0_EP1_bInterval,						// Interval for polling endpoint for data transfers.
			#endif
			#if (FS_C1_I1_A0_EP_NUMBER >= 2)
				// EP2
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A0_EP2_DIRECTION) << 7) | EP2),
				FS_C1_I1_A0_EP2_TYPE,	
				mLowByte(FS_C1_I1_A0_EP2_MAX_PACKET),
				mHighByte(FS_C1_I1_A0_EP2_MAX_PACKET),
				FS_C1_I1_A0_EP2_bInterval,
			#endif
			#if (FS_C1_I1_A0_EP_NUMBER >= 3)
				// EP3
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A0_EP3_DIRECTION) << 7) | EP3),
				FS_C1_I1_A0_EP3_TYPE,	
				mLowByte(FS_C1_I1_A0_EP3_MAX_PACKET),
				mHighByte(FS_C1_I1_A0_EP3_MAX_PACKET),
				FS_C1_I1_A0_EP3_bInterval,
			#endif
			#if (FS_C1_I1_A0_EP_NUMBER >= 4)
				// EP4
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A0_EP4_DIRECTION) << 7) | EP4),
				FS_C1_I1_A0_EP4_TYPE,	
				mLowByte(FS_C1_I1_A0_EP4_MAX_PACKET),
				mHighByte(FS_C1_I1_A0_EP4_MAX_PACKET),
				FS_C1_I1_A0_EP4_bInterval,
			#endif
			#if (FS_C1_I1_A0_EP_NUMBER >= 5)
				// EP5
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A0_EP5_DIRECTION) << 7) | EP5),
				FS_C1_I1_A0_EP5_TYPE,	
				mLowByte(FS_C1_I1_A0_EP5_MAX_PACKET),
				mHighByte(FS_C1_I1_A0_EP5_MAX_PACKET),
				FS_C1_I1_A0_EP5_bInterval,
			#endif
			#if (FS_C1_I1_A0_EP_NUMBER >= 6)
				// EP6
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A0_EP6_DIRECTION) << 7) | EP6),
				FS_C1_I1_A0_EP6_TYPE,	
				mLowByte(FS_C1_I1_A0_EP6_MAX_PACKET),
				mHighByte(FS_C1_I1_A0_EP6_MAX_PACKET),
				FS_C1_I1_A0_EP6_bInterval,
			#endif
			#if (FS_C1_I1_A0_EP_NUMBER >= 7)
				// EP7
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A0_EP7_DIRECTION) << 7) | EP7),
				FS_C1_I1_A0_EP7_TYPE,	
				mLowByte(FS_C1_I1_A0_EP7_MAX_PACKET),
				mHighByte(FS_C1_I1_A0_EP7_MAX_PACKET),
				FS_C1_I1_A0_EP7_bInterval,
			#endif
			#if (FS_C1_I1_A0_EP_NUMBER >= 8)
				// EP8
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A0_EP8_DIRECTION) << 7) | EP8),
				FS_C1_I1_A0_EP8_TYPE,	
				mLowByte(FS_C1_I1_A0_EP8_MAX_PACKET),
				mHighByte(FS_C1_I1_A0_EP8_MAX_PACKET),
				FS_C1_I1_A0_EP8_bInterval,
			#endif
			#if (FS_C1_I1_A0_EP_NUMBER >= 9)
				// EP9
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A0_EP9_DIRECTION) << 7) | EP9),
				FS_C1_I1_A0_EP9_TYPE,	
				mLowByte(FS_C1_I1_A0_EP9_MAX_PACKET),
				mHighByte(FS_C1_I1_A0_EP9_MAX_PACKET),
				FS_C1_I1_A0_EP9_bInterval,
			#endif
			#if (FS_C1_I1_A0_EP_NUMBER >= 10)
				// EP10
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A0_EP10_DIRECTION) << 7) | EP10),
				FS_C1_I1_A0_EP10_TYPE,	
				mLowByte(FS_C1_I1_A0_EP10_MAX_PACKET),
				mHighByte(FS_C1_I1_A0_EP10_MAX_PACKET),
				FS_C1_I1_A0_EP10_bInterval,
			#endif
			#if (FS_C1_I1_A0_EP_NUMBER >= 11)
				// EP11
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A0_EP11_DIRECTION) << 7) | EP11),
				FS_C1_I1_A0_EP11_TYPE,	
				mLowByte(FS_C1_I1_A0_EP11_MAX_PACKET),
				mHighByte(FS_C1_I1_A0_EP11_MAX_PACKET),
				FS_C1_I1_A0_EP11_bInterval,
			#endif
			#if (FS_C1_I1_A0_EP_NUMBER >= 12)
				// EP12
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A0_EP12_DIRECTION) << 7) | EP12),
				FS_C1_I1_A0_EP12_TYPE,	
				mLowByte(FS_C1_I1_A0_EP12_MAX_PACKET),
				mHighByte(FS_C1_I1_A0_EP12_MAX_PACKET),
				FS_C1_I1_A0_EP12_bInterval,
			#endif
			#if (FS_C1_I1_A0_EP_NUMBER >= 13)
				// EP13
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A0_EP13_DIRECTION) << 7) | EP13),
				FS_C1_I1_A0_EP13_TYPE,	
				mLowByte(FS_C1_I1_A0_EP13_MAX_PACKET),
				mHighByte(FS_C1_I1_A0_EP13_MAX_PACKET),
				FS_C1_I1_A0_EP13_bInterval,
			#endif
			#if (FS_C1_I1_A0_EP_NUMBER >= 14)
				// EP14
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A0_EP14_DIRECTION) << 7) | EP14),
				FS_C1_I1_A0_EP14_TYPE,	
				mLowByte(FS_C1_I1_A0_EP14_MAX_PACKET),
				mHighByte(FS_C1_I1_A0_EP14_MAX_PACKET),
				FS_C1_I1_A0_EP14_bInterval,
			#endif
			#if (FS_C1_I1_A0_EP_NUMBER >= 15)
				// EP15
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A0_EP15_DIRECTION) << 7) | EP15),
				FS_C1_I1_A0_EP15_TYPE,	
				mLowByte(FS_C1_I1_A0_EP15_MAX_PACKET),
				mHighByte(FS_C1_I1_A0_EP15_MAX_PACKET),
				FS_C1_I1_A0_EP15_bInterval,
			#endif
		#endif

		#if (FS_C1_I1_ALT_NUMBER >= 2)
			// Alternate Setting 0
			INTERFACE_LENGTH,			        // bLength
			DT_INTERFACE,				        // bDescriptorType INTERFACE
			FS_C1_I1_A1_bInterfaceNumber,       // bInterfaceNumber
			FS_C1_I1_A1_bAlternateSetting,	    // bAlternateSetting
			FS_C1_I1_A1_EP_NUMBER,		        // bNumEndpoints(excluding endpoint zero)
			FS_C1_I1_A1_bInterfaceClass,    	// bInterfaceClass
			FS_C1_I1_A1_bInterfaceSubClass,     // bInterfaceSubClass
			FS_C1_I1_A1_bInterfaceProtocol,     // bInterfaceProtocol
			FS_C1_I1_A1_iInterface,		        // iInterface

			#if (FS_C1_I1_A1_EP_NUMBER >= 1)
				// EP1
				EP_LENGTH,					// bLength
				DT_ENDPOINT,				// bDescriptorType ENDPOINT
				(((1 - FS_C1_I1_A1_EP1_DIRECTION) << 7) | EP1),	// bEndpointAddress
								// D7: Direction, 1=IN, 0=OUT
								// D6..4: Reserved(reset to zero), D3..0: The endpointer number
				FS_C1_I1_A1_EP1_TYPE,		// bmAttributes
								// D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
								// if not an isochronous endpoint, D7..2 are Reserved
				mLowByte(FS_C1_I1_A1_EP1_MAX_PACKET),				// wMaxPacketSize
				mHighByte(FS_C1_I1_A1_EP1_MAX_PACKET),
				FS_C1_I1_A1_EP1_bInterval,						// Interval for polling endpoint for data transfers.
			#endif
			#if (FS_C1_I1_A1_EP_NUMBER >= 2)
				// EP2
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A1_EP2_DIRECTION) << 7) | EP2),
				FS_C1_I1_A1_EP2_TYPE,	
				mLowByte(FS_C1_I1_A1_EP2_MAX_PACKET),
				mHighByte(FS_C1_I1_A1_EP2_MAX_PACKET),
				FS_C1_I1_A1_EP2_bInterval,
			#endif
			#if (FS_C1_I1_A1_EP_NUMBER >= 3)
				// EP3
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A1_EP3_DIRECTION) << 7) | EP3),
				FS_C1_I1_A1_EP3_TYPE,	
				mLowByte(FS_C1_I1_A1_EP3_MAX_PACKET),
				mHighByte(FS_C1_I1_A1_EP3_MAX_PACKET),
				FS_C1_I1_A1_EP3_bInterval,
			#endif
			#if (FS_C1_I1_A1_EP_NUMBER >= 4)
				// EP4
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A1_EP4_DIRECTION) << 7) | EP4),
				FS_C1_I1_A1_EP4_TYPE,	
				mLowByte(FS_C1_I1_A1_EP4_MAX_PACKET),
				mHighByte(FS_C1_I1_A1_EP4_MAX_PACKET),
				FS_C1_I1_A1_EP4_bInterval,
			#endif
			#if (FS_C1_I1_A1_EP_NUMBER >= 5)
				// EP5
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A1_EP5_DIRECTION) << 7) | EP5),
				FS_C1_I1_A1_EP5_TYPE,	
				mLowByte(FS_C1_I1_A1_EP5_MAX_PACKET),
				mHighByte(FS_C1_I1_A1_EP5_MAX_PACKET),
				FS_C1_I1_A1_EP5_bInterval,
			#endif
			#if (FS_C1_I1_A1_EP_NUMBER >= 6)
				// EP6
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A1_EP6_DIRECTION) << 7) | EP6),
				FS_C1_I1_A1_EP6_TYPE,	
				mLowByte(FS_C1_I1_A1_EP6_MAX_PACKET),
				mHighByte(FS_C1_I1_A1_EP6_MAX_PACKET),
				FS_C1_I1_A1_EP6_bInterval,
			#endif
			#if (FS_C1_I1_A1_EP_NUMBER >= 7)
				// EP7
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A1_EP7_DIRECTION) << 7) | EP7),
				FS_C1_I1_A1_EP7_TYPE,	
				mLowByte(FS_C1_I1_A1_EP7_MAX_PACKET),
				mHighByte(FS_C1_I1_A1_EP7_MAX_PACKET),
				FS_C1_I1_A1_EP7_bInterval,
			#endif
			#if (FS_C1_I1_A1_EP_NUMBER >= 8)
				// EP8
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A1_EP8_DIRECTION) << 7) | EP8),
				FS_C1_I1_A1_EP8_TYPE,	
				mLowByte(FS_C1_I1_A1_EP8_MAX_PACKET),
				mHighByte(FS_C1_I1_A1_EP8_MAX_PACKET),
				FS_C1_I1_A1_EP8_bInterval,
			#endif
			#if (FS_C1_I1_A1_EP_NUMBER >= 9)
				// EP9
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A1_EP9_DIRECTION) << 7) | EP9),
				FS_C1_I1_A1_EP9_TYPE,	
				mLowByte(FS_C1_I1_A1_EP9_MAX_PACKET),
				mHighByte(FS_C1_I1_A1_EP9_MAX_PACKET),
				FS_C1_I1_A1_EP9_bInterval,
			#endif
			#if (FS_C1_I1_A1_EP_NUMBER >= 10)
				// EP10
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A1_EP10_DIRECTION) << 7) | EP10),
				FS_C1_I1_A1_EP10_TYPE,	
				mLowByte(FS_C1_I1_A1_EP10_MAX_PACKET),
				mHighByte(FS_C1_I1_A1_EP10_MAX_PACKET),
				FS_C1_I1_A1_EP10_bInterval,
			#endif
			#if (FS_C1_I1_A1_EP_NUMBER >= 11)
				// EP11
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A1_EP11_DIRECTION) << 7) | EP11),
				FS_C1_I1_A1_EP11_TYPE,	
				mLowByte(FS_C1_I1_A1_EP11_MAX_PACKET),
				mHighByte(FS_C1_I1_A1_EP11_MAX_PACKET),
				FS_C1_I1_A1_EP11_bInterval,
			#endif
			#if (FS_C1_I1_A1_EP_NUMBER >= 12)
				// EP12
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A1_EP12_DIRECTION) << 7) | EP12),
				FS_C1_I1_A1_EP12_TYPE,	
				mLowByte(FS_C1_I1_A1_EP12_MAX_PACKET),
				mHighByte(FS_C1_I1_A1_EP12_MAX_PACKET),
				FS_C1_I1_A1_EP12_bInterval,
			#endif
			#if (FS_C1_I1_A1_EP_NUMBER >= 13)
				// EP13
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A1_EP13_DIRECTION) << 7) | EP13),
				FS_C1_I1_A1_EP13_TYPE,	
				mLowByte(FS_C1_I1_A1_EP13_MAX_PACKET),
				mHighByte(FS_C1_I1_A1_EP13_MAX_PACKET),
				FS_C1_I1_A1_EP13_bInterval,
			#endif
			#if (FS_C1_I1_A1_EP_NUMBER >= 14)
				// EP14
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A1_EP14_DIRECTION) << 7) | EP14),
				FS_C1_I1_A1_EP14_TYPE,	
				mLowByte(FS_C1_I1_A1_EP14_MAX_PACKET),
				mHighByte(FS_C1_I1_A1_EP14_MAX_PACKET),
				FS_C1_I1_A1_EP14_bInterval,
			#endif
			#if (FS_C1_I1_A1_EP_NUMBER >= 15)
				// EP15
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C1_I1_A1_EP15_DIRECTION) << 7) | EP15),
				FS_C1_I1_A1_EP15_TYPE,	
				mLowByte(FS_C1_I1_A1_EP15_MAX_PACKET),
				mHighByte(FS_C1_I1_A1_EP15_MAX_PACKET),
				FS_C1_I1_A1_EP15_bInterval,
			#endif
		#endif

	#endif
/*
	OTG_LENGTH,               
	DT_OTG,                   
	FS_OTG_SUPPORT,   
*/
};
#endif

#if (FS_CONFIGURATION_NUMBER >= 2)
// Configuration 2
MMP_UINT8 u8FSConfigOTGDescriptor02[FS_C2_CONFIG_TOTAL_LENGTH] =
{
	//	CONFIGURATION descriptor
	CONFIG_LENGTH,					// bLength
	DT_CONFIGURATION,				// bDescriptorType CONFIGURATION
	mLowByte(FS_C2_CONFIG_TOTAL_LENGTH),	// wTotalLength, include all descriptors
	mHighByte(FS_C2_CONFIG_TOTAL_LENGTH),
	FS_C2_INTERFACE_NUMBER,			// bNumInterface
	FS_C2,							// bConfigurationValue
	FS_C2_iConfiguration,			// iConfiguration
	FS_C2_bmAttribute,				// bmAttribute
									// D7: Reserved(set to one), D6: Self-powered, D5: Remote Wakeup, D4..0: Reserved(reset to zero)
	FS_C2_iMaxPower,				// iMaxPower (2mA units)

	#if (FS_C2_INTERFACE_NUMBER >= 1)
		// Interface 0
		#if (FS_C2_I0_ALT_NUMBER >= 1)
			// Alternate Setting 0
			INTERFACE_LENGTH,				// bLength
			DT_INTERFACE,					// bDescriptorType INTERFACE
			FS_C2_I0_A0_bInterfaceNumber,      // bInterfaceNumber
			FS_C2_I0_A0_bAlternateSetting,	    // bAlternateSetting
			FS_C2_I0_A0_EP_NUMBER,			// bNumEndpoints(excluding endpoint zero)
			FS_C2_I0_A0_bInterfaceClass,	// bInterfaceClass
			FS_C2_I0_A0_bInterfaceSubClass,// bInterfaceSubClass
			FS_C2_I0_A0_bInterfaceProtocol,// bInterfaceProtocol
			FS_C2_I0_A0_iInterface,		// iInterface

			#if (FS_C2_I0_A0_EP_NUMBER >= 1)
				// EP1
				EP_LENGTH,						// bLength
				DT_ENDPOINT,					// bDescriptorType ENDPOINT
				(((1 - FS_C2_I0_A0_EP1_DIRECTION) << 7) | EP1),	// bEndpointAddress
									// D7: Direction, 1=IN, 0=OUT
									// D6..4: Reserved(reset to zero), D3..0: The endpointer number
				FS_C2_I0_A0_EP1_TYPE,			// bmAttributes
									// D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
									// if not an isochronous endpoint, D7..2 are Reserved
				mLowByte(FS_C2_I0_A0_EP1_MAX_PACKET),	// wMaxPacketSize
				mHighByte(FS_C2_I0_A0_EP1_MAX_PACKET),
				FS_C2_I0_A0_EP1_bInterval,							// Interval for polling endpoint for data transfers.
			#endif
			#if (FS_C2_I0_A0_EP_NUMBER >= 2)
				// EP2
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A0_EP2_DIRECTION) << 7) | EP2),
				FS_C2_I0_A0_EP2_TYPE,	
				mLowByte(FS_C2_I0_A0_EP2_MAX_PACKET),
				mHighByte(FS_C2_I0_A0_EP2_MAX_PACKET),
				FS_C2_I0_A0_EP2_bInterval,
			#endif
			#if (FS_C2_I0_A0_EP_NUMBER >= 3)
				// EP3
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A0_EP3_DIRECTION) << 7) | EP3),
				FS_C2_I0_A0_EP3_TYPE,	
				mLowByte(FS_C2_I0_A0_EP3_MAX_PACKET),
				mHighByte(FS_C2_I0_A0_EP3_MAX_PACKET),
				FS_C2_I0_A0_EP3_bInterval,
			#endif
			#if (FS_C2_I0_A0_EP_NUMBER >= 4)
				// EP4
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A0_EP3_DIRECTION) << 7) | EP4),
				FS_C2_I0_A0_EP3_TYPE,	
				mLowByte(FS_C2_I0_A0_EP3_MAX_PACKET),
				mHighByte(FS_C2_I0_A0_EP3_MAX_PACKET),
				FS_C2_I0_A0_EP3_bInterval,
			#endif
			#if (FS_C2_I0_A0_EP_NUMBER >= 5)
				// EP5
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A0_EP5_DIRECTION) << 7) | EP5),
				FS_C2_I0_A0_EP5_TYPE,	
				mLowByte(FS_C2_I0_A0_EP5_MAX_PACKET),
				mHighByte(FS_C2_I0_A0_EP5_MAX_PACKET),
				FS_C2_I0_A0_EP5_bInterval,
			#endif
			#if (FS_C2_I0_A0_EP_NUMBER >= 6)
				// EP6
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A0_EP6_DIRECTION) << 7) | EP6),
				FS_C2_I0_A0_EP6_TYPE,	
				mLowByte(FS_C2_I0_A0_EP6_MAX_PACKET),
				mHighByte(FS_C2_I0_A0_EP6_MAX_PACKET),
				FS_C2_I0_A0_EP6_bInterval,
			#endif
			#if (FS_C2_I0_A0_EP_NUMBER >= 7)
				// EP7
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A0_EP7_DIRECTION) << 7) | EP7),
				FS_C2_I0_A0_EP7_TYPE,	
				mLowByte(FS_C2_I0_A0_EP7_MAX_PACKET),
				mHighByte(FS_C2_I0_A0_EP7_MAX_PACKET),
				FS_C2_I0_A0_EP7_bInterval,
			#endif
			#if (FS_C2_I0_A0_EP_NUMBER >= 8)
				// EP8
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A0_EP8_DIRECTION) << 7) | EP8),
				FS_C2_I0_A0_EP8_TYPE,	
				mLowByte(FS_C2_I0_A0_EP8_MAX_PACKET),
				mHighByte(FS_C2_I0_A0_EP8_MAX_PACKET),
				FS_C2_I0_A0_EP8_bInterval,
			#endif
			#if (FS_C2_I0_A0_EP_NUMBER >= 9)
				// EP9
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A0_EP9_DIRECTION) << 7) | EP9),
				FS_C2_I0_A0_EP9_TYPE,	
				mLowByte(FS_C2_I0_A0_EP9_MAX_PACKET),
				mHighByte(FS_C2_I0_A0_EP9_MAX_PACKET),
				FS_C2_I0_A0_EP9_bInterval,
			#endif
			#if (FS_C2_I0_A0_EP_NUMBER >= 10)
				// EP10
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A0_EP10_DIRECTION) << 7) | EP10),
				FS_C2_I0_A0_EP10_TYPE,	
				mLowByte(FS_C2_I0_A0_EP10_MAX_PACKET),
				mHighByte(FS_C2_I0_A0_EP10_MAX_PACKET),
				FS_C2_I0_A0_EP10_bInterval,
			#endif
			#if (FS_C2_I0_A0_EP_NUMBER >= 11)
				// EP11
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A0_EP11_DIRECTION) << 7) | EP11),
				FS_C2_I0_A0_EP11_TYPE,	
				mLowByte(FS_C2_I0_A0_EP11_MAX_PACKET),
				mHighByte(FS_C2_I0_A0_EP11_MAX_PACKET),
				FS_C2_I0_A0_EP11_bInterval,
			#endif
			#if (FS_C2_I0_A0_EP_NUMBER >= 12)
				// EP12
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A0_EP12_DIRECTION) << 7) | EP12),
				FS_C2_I0_A0_EP12_TYPE,	
				mLowByte(FS_C2_I0_A0_EP12_MAX_PACKET),
				mHighByte(FS_C2_I0_A0_EP12_MAX_PACKET),
				FS_C2_I0_A0_EP12_bInterval,
			#endif
			#if (FS_C2_I0_A0_EP_NUMBER >= 13)
				// EP13
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A0_EP13_DIRECTION) << 7) | EP13),
				FS_C2_I0_A0_EP13_TYPE,	
				mLowByte(FS_C2_I0_A0_EP13_MAX_PACKET),
				mHighByte(FS_C2_I0_A0_EP13_MAX_PACKET),
				FS_C2_I0_A0_EP13_bInterval,
			#endif
			#if (FS_C2_I0_A0_EP_NUMBER >= 14)
				// EP14
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A0_EP14_DIRECTION) << 7) | EP14),
				FS_C2_I0_A0_EP14_TYPE,	
				mLowByte(FS_C2_I0_A0_EP14_MAX_PACKET),
				mHighByte(FS_C2_I0_A0_EP14_MAX_PACKET),
				FS_C2_I0_A0_EP14_bInterval,
			#endif
			#if (FS_C2_I0_A0_EP_NUMBER >= 15)
				// EP15
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A0_EP15_DIRECTION) << 7) | EP15),
				FS_C2_I0_A0_EP15_TYPE,	
				mLowByte(FS_C2_I0_A0_EP15_MAX_PACKET),
				mHighByte(FS_C2_I0_A0_EP15_MAX_PACKET),
				FS_C2_I0_A0_EP15_bInterval,
			#endif
		#endif

		#if (FS_C2_I0_ALT_NUMBER >= 2)
			// Alternate Setting 0
			INTERFACE_LENGTH,				// bLength
			DT_INTERFACE,					// bDescriptorType INTERFACE
			FS_C2_I0_A1_bInterfaceNumber,      // bInterfaceNumber
			FS_C2_I0_A1_bAlternateSetting,	    // bAlternateSetting
			FS_C2_I0_A1_EP_NUMBER,			// bNumEndpoints(excluding endpoint zero)
			FS_C2_I0_A1_bInterfaceClass,	// bInterfaceClass
			FS_C2_I0_A1_bInterfaceSubClass,// bInterfaceSubClass
			FS_C2_I0_A1_bInterfaceProtocol,// bInterfaceProtocol
			FS_C2_I0_A1_iInterface,		// iInterface

			#if (FS_C2_I0_A1_EP_NUMBER >= 1)
				// EP1
				EP_LENGTH,						// bLength
				DT_ENDPOINT,					// bDescriptorType ENDPOINT
				(((1 - FS_C2_I0_A1_EP1_DIRECTION) << 7) | EP1),	// bEndpointAddress
									// D7: Direction, 1=IN, 0=OUT
									// D6..4: Reserved(reset to zero), D3..0: The endpointer number
				FS_C2_I0_A1_EP1_TYPE,			// bmAttributes
									// D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
									// if not an isochronous endpoint, D7..2 are Reserved
				mLowByte(FS_C2_I0_A1_EP1_MAX_PACKET),	// wMaxPacketSize
				mHighByte(FS_C2_I0_A1_EP1_MAX_PACKET),
				FS_C2_I0_A1_EP1_bInterval,							// Interval for polling endpoint for data transfers.
			#endif
			#if (FS_C2_I0_A1_EP_NUMBER >= 2)
				// EP2
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A1_EP2_DIRECTION) << 7) | EP2),
				FS_C2_I0_A1_EP2_TYPE,	
				mLowByte(FS_C2_I0_A1_EP2_MAX_PACKET),
				mHighByte(FS_C2_I0_A1_EP2_MAX_PACKET),
				FS_C2_I0_A1_EP2_bInterval,
			#endif
			#if (FS_C2_I0_A1_EP_NUMBER >= 3)
				// EP3
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A1_EP3_DIRECTION) << 7) | EP3),
				FS_C2_I0_A1_EP3_TYPE,	
				mLowByte(FS_C2_I0_A1_EP3_MAX_PACKET),
				mHighByte(FS_C2_I0_A1_EP3_MAX_PACKET),
				FS_C2_I0_A1_EP3_bInterval,
			#endif
			#if (FS_C2_I0_A1_EP_NUMBER >= 4)
				// EP4
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A1_EP3_DIRECTION) << 7) | EP4),
				FS_C2_I0_A1_EP3_TYPE,	
				mLowByte(FS_C2_I0_A1_EP3_MAX_PACKET),
				mHighByte(FS_C2_I0_A1_EP3_MAX_PACKET),
				FS_C2_I0_A1_EP3_bInterval,
			#endif
			#if (FS_C2_I0_A1_EP_NUMBER >= 5)
				// EP5
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A1_EP5_DIRECTION) << 7) | EP5),
				FS_C2_I0_A1_EP5_TYPE,	
				mLowByte(FS_C2_I0_A1_EP5_MAX_PACKET),
				mHighByte(FS_C2_I0_A1_EP5_MAX_PACKET),
				FS_C2_I0_A1_EP5_bInterval,
			#endif
			#if (FS_C2_I0_A1_EP_NUMBER >= 6)
				// EP6
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A1_EP6_DIRECTION) << 7) | EP6),
				FS_C2_I0_A1_EP6_TYPE,	
				mLowByte(FS_C2_I0_A1_EP6_MAX_PACKET),
				mHighByte(FS_C2_I0_A1_EP6_MAX_PACKET),
				FS_C2_I0_A1_EP6_bInterval,
			#endif
			#if (FS_C2_I0_A1_EP_NUMBER >= 7)
				// EP7
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A1_EP7_DIRECTION) << 7) | EP7),
				FS_C2_I0_A1_EP7_TYPE,	
				mLowByte(FS_C2_I0_A1_EP7_MAX_PACKET),
				mHighByte(FS_C2_I0_A1_EP7_MAX_PACKET),
				FS_C2_I0_A1_EP7_bInterval,
			#endif
			#if (FS_C2_I0_A1_EP_NUMBER >= 8)
				// EP8
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A1_EP8_DIRECTION) << 7) | EP8),
				FS_C2_I0_A1_EP8_TYPE,	
				mLowByte(FS_C2_I0_A1_EP8_MAX_PACKET),
				mHighByte(FS_C2_I0_A1_EP8_MAX_PACKET),
				FS_C2_I0_A1_EP8_bInterval,
			#endif
			#if (FS_C2_I0_A1_EP_NUMBER >= 9)
				// EP9
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A1_EP9_DIRECTION) << 7) | EP9),
				FS_C2_I0_A1_EP9_TYPE,	
				mLowByte(FS_C2_I0_A1_EP9_MAX_PACKET),
				mHighByte(FS_C2_I0_A1_EP9_MAX_PACKET),
				FS_C2_I0_A1_EP9_bInterval,
			#endif
			#if (FS_C2_I0_A1_EP_NUMBER >= 10)
				// EP10
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A1_EP10_DIRECTION) << 7) | EP10),
				FS_C2_I0_A1_EP10_TYPE,	
				mLowByte(FS_C2_I0_A1_EP10_MAX_PACKET),
				mHighByte(FS_C2_I0_A1_EP10_MAX_PACKET),
				FS_C2_I0_A1_EP10_bInterval,
			#endif
			#if (FS_C2_I0_A1_EP_NUMBER >= 11)
				// EP11
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A1_EP11_DIRECTION) << 7) | EP11),
				FS_C2_I0_A1_EP11_TYPE,	
				mLowByte(FS_C2_I0_A1_EP11_MAX_PACKET),
				mHighByte(FS_C2_I0_A1_EP11_MAX_PACKET),
				FS_C2_I0_A1_EP11_bInterval,
			#endif
			#if (FS_C2_I0_A1_EP_NUMBER >= 12)
				// EP12
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A1_EP12_DIRECTION) << 7) | EP12),
				FS_C2_I0_A1_EP12_TYPE,	
				mLowByte(FS_C2_I0_A1_EP12_MAX_PACKET),
				mHighByte(FS_C2_I0_A1_EP12_MAX_PACKET),
				FS_C2_I0_A1_EP12_bInterval,
			#endif
			#if (FS_C2_I0_A1_EP_NUMBER >= 13)
				// EP13
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A1_EP13_DIRECTION) << 7) | EP13),
				FS_C2_I0_A1_EP13_TYPE,	
				mLowByte(FS_C2_I0_A1_EP13_MAX_PACKET),
				mHighByte(FS_C2_I0_A1_EP13_MAX_PACKET),
				FS_C2_I0_A1_EP13_bInterval,
			#endif
			#if (FS_C2_I0_A1_EP_NUMBER >= 14)
				// EP14
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A1_EP14_DIRECTION) << 7) | EP14),
				FS_C2_I0_A1_EP14_TYPE,	
				mLowByte(FS_C2_I0_A1_EP14_MAX_PACKET),
				mHighByte(FS_C2_I0_A1_EP14_MAX_PACKET),
				FS_C2_I0_A1_EP14_bInterval,
			#endif
			#if (FS_C2_I0_A1_EP_NUMBER >= 15)
				// EP15
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I0_A1_EP15_DIRECTION) << 7) | EP15),
				FS_C2_I0_A1_EP15_TYPE,	
				mLowByte(FS_C2_I0_A1_EP15_MAX_PACKET),
				mHighByte(FS_C2_I0_A1_EP15_MAX_PACKET),
				FS_C2_I0_A1_EP15_bInterval,
			#endif
		#endif

	#endif

	#if (FS_C2_INTERFACE_NUMBER >= 2)
		// Interface 1
		#if (FS_C2_I1_ALT_NUMBER >= 1)
			// Alternate Setting 0
			INTERFACE_LENGTH,			// bLength
			DT_INTERFACE,				// bDescriptorType INTERFACE
			FS_C2_I1_A0_bInterfaceNumber,      // bInterfaceNumber
			FS_C2_I1_A0_bAlternateSetting,	    // bAlternateSetting
			FS_C2_I1_A0_EP_NUMBER,		// bNumEndpoints(excluding endpoint zero)
			FS_C2_I1_A0_bInterfaceClass,	// bInterfaceClass
			FS_C2_I1_A0_bInterfaceSubClass,// bInterfaceSubClass
			FS_C2_I1_A0_bInterfaceProtocol,// bInterfaceProtocol
			FS_C2_I1_A0_iInterface,		// iInterface

			#if (FS_C2_I1_A0_EP_NUMBER >= 1)
				// EP1
				EP_LENGTH,					// bLength
				DT_ENDPOINT,				// bDescriptorType ENDPOINT
				(((1 - FS_C2_I1_A0_EP1_DIRECTION) << 7) | EP1),	// bEndpointAddress
								// D7: Direction, 1=IN, 0=OUT
								// D6..4: Reserved(reset to zero), D3..0: The endpointer number
				FS_C2_I1_A0_EP1_TYPE,		// bmAttributes
								// D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
								// if not an isochronous endpoint, D7..2 are Reserved
				mLowByte(FS_C2_I1_A0_EP1_MAX_PACKET),	// wMaxPacketSize
				mHighByte(FS_C2_I1_A0_EP1_MAX_PACKET),
				FS_C2_I1_A0_EP1_bInterval,						// Interval for polling endpoint for data transfers.
			#endif
			#if (FS_C2_I1_A0_EP_NUMBER >= 2)
				// EP2
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A0_EP2_DIRECTION) << 7) | EP2),
				FS_C2_I1_A0_EP2_TYPE,	
				mLowByte(FS_C2_I1_A0_EP2_MAX_PACKET),
				mHighByte(FS_C2_I1_A0_EP2_MAX_PACKET),
				FS_C2_I1_A0_EP2_bInterval,
			#endif
			#if (FS_C2_I1_A0_EP_NUMBER >= 3)
				// EP3
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A0_EP3_DIRECTION) << 7) | EP3),
				FS_C2_I1_A0_EP3_TYPE,	
				mLowByte(FS_C2_I1_A0_EP3_MAX_PACKET),
				mHighByte(FS_C2_I1_A0_EP3_MAX_PACKET),
				FS_C2_I1_A0_EP3_bInterval,
			#endif
			#if (FS_C2_I1_A0_EP_NUMBER >= 4)
				// EP4
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A0_EP4_DIRECTION) << 7) | EP4),
				FS_C2_I1_A0_EP4_TYPE,	
				mLowByte(FS_C2_I1_A0_EP4_MAX_PACKET),
				mHighByte(FS_C2_I1_A0_EP4_MAX_PACKET),
				FS_C2_I1_A0_EP4_bInterval,
			#endif
			#if (FS_C2_I1_A0_EP_NUMBER >= 5)
				// EP5
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A0_EP5_DIRECTION) << 7) | EP5),
				FS_C2_I1_A0_EP5_TYPE,	
				mLowByte(FS_C2_I1_A0_EP5_MAX_PACKET),
				mHighByte(FS_C2_I1_A0_EP5_MAX_PACKET),
				FS_C2_I1_A0_EP5_bInterval,
			#endif
			#if (FS_C2_I1_A0_EP_NUMBER >= 6)
				// EP6
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A0_EP6_DIRECTION) << 7) | EP6),
				FS_C2_I1_A0_EP6_TYPE,	
				mLowByte(FS_C2_I1_A0_EP6_MAX_PACKET),
				mHighByte(FS_C2_I1_A0_EP6_MAX_PACKET),
				FS_C2_I1_A0_EP6_bInterval,
			#endif
			#if (FS_C2_I1_A0_EP_NUMBER >= 7)
				// EP7
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A0_EP7_DIRECTION) << 7) | EP7),
				FS_C2_I1_A0_EP7_TYPE,	
				mLowByte(FS_C2_I1_A0_EP7_MAX_PACKET),
				mHighByte(FS_C2_I1_A0_EP7_MAX_PACKET),
				FS_C2_I1_A0_EP7_bInterval,
			#endif
			#if (FS_C2_I1_A0_EP_NUMBER >= 8)
				// EP8
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A0_EP8_DIRECTION) << 7) | EP8),
				FS_C2_I1_A0_EP8_TYPE,	
				mLowByte(FS_C2_I1_A0_EP8_MAX_PACKET),
				mHighByte(FS_C2_I1_A0_EP8_MAX_PACKET),
				FS_C2_I1_A0_EP8_bInterval,
			#endif
			#if (FS_C2_I1_A0_EP_NUMBER >= 9)
				// EP9
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A0_EP9_DIRECTION) << 7) | EP9),
				FS_C2_I1_A0_EP9_TYPE,	
				mLowByte(FS_C2_I1_A0_EP9_MAX_PACKET),
				mHighByte(FS_C2_I1_A0_EP9_MAX_PACKET),
				FS_C2_I1_A0_EP9_bInterval,
			#endif
			#if (FS_C2_I1_A0_EP_NUMBER >= 10)
				// EP10
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A0_EP10_DIRECTION) << 7) | EP10),
				FS_C2_I1_A0_EP10_TYPE,	
				mLowByte(FS_C2_I1_A0_EP10_MAX_PACKET),
				mHighByte(FS_C2_I1_A0_EP10_MAX_PACKET),
				FS_C2_I1_A0_EP10_bInterval,
			#endif
			#if (FS_C2_I1_A0_EP_NUMBER >= 11)
				// EP11
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A0_EP11_DIRECTION) << 7) | EP11),
				FS_C2_I1_A0_EP11_TYPE,	
				mLowByte(FS_C2_I1_A0_EP11_MAX_PACKET),
				mHighByte(FS_C2_I1_A0_EP11_MAX_PACKET),
				FS_C2_I1_A0_EP11_bInterval,
			#endif
			#if (FS_C2_I1_A0_EP_NUMBER >= 12)
				// EP12
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A0_EP12_DIRECTION) << 7) | EP12),
				FS_C2_I1_A0_EP12_TYPE,	
				mLowByte(FS_C2_I1_A0_EP12_MAX_PACKET),
				mHighByte(FS_C2_I1_A0_EP12_MAX_PACKET),
				FS_C2_I1_A0_EP12_bInterval,
			#endif
			#if (FS_C2_I1_A0_EP_NUMBER >= 13)
				// EP13
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A0_EP13_DIRECTION) << 7) | EP13),
				FS_C2_I1_A0_EP13_TYPE,	
				mLowByte(FS_C2_I1_A0_EP13_MAX_PACKET),
				mHighByte(FS_C2_I1_A0_EP13_MAX_PACKET),
				FS_C2_I1_A0_EP13_bInterval,
			#endif
			#if (FS_C2_I1_A0_EP_NUMBER >= 14)
				// EP14
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A0_EP14_DIRECTION) << 7) | EP14),
				FS_C2_I1_A0_EP14_TYPE,	
				mLowByte(FS_C2_I1_A0_EP14_MAX_PACKET),
				mHighByte(FS_C2_I1_A0_EP14_MAX_PACKET),
				FS_C2_I1_A0_EP14_bInterval,
			#endif
			#if (FS_C2_I1_A0_EP_NUMBER >= 15)
				// EP15
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A0_EP15_DIRECTION) << 7) | EP15),
				FS_C2_I1_A0_EP15_TYPE,	
				mLowByte(FS_C2_I1_A0_EP15_MAX_PACKET),
				mHighByte(FS_C2_I1_A0_EP15_MAX_PACKET),
				FS_C2_I1_A0_EP15_bInterval,
			#endif
		#endif

		#if (FS_C2_I1_ALT_NUMBER >= 2)
			// Alternate Setting 0
			INTERFACE_LENGTH,			// bLength
			DT_INTERFACE,				// bDescriptorType INTERFACE
			FS_C2_I1_A1_bInterfaceNumber,      // bInterfaceNumber
			FS_C2_I1_A1_bAlternateSetting,	    // bAlternateSetting
			FS_C2_I1_A1_EP_NUMBER,		// bNumEndpoints(excluding endpoint zero)
			FS_C2_I1_A1_bInterfaceClass,	// bInterfaceClass
			FS_C2_I1_A1_bInterfaceSubClass,// bInterfaceSubClass
			FS_C2_I1_A1_bInterfaceProtocol,// bInterfaceProtocol
			FS_C2_I1_A1_iInterface,		// iInterface

			#if (FS_C2_I1_A1_EP_NUMBER >= 1)
				// EP1
				EP_LENGTH,					// bLength
				DT_ENDPOINT,				// bDescriptorType ENDPOINT
				(((1 - FS_C2_I1_A1_EP1_DIRECTION) << 7) | EP1),	// bEndpointAddress
								// D7: Direction, 1=IN, 0=OUT
								// D6..4: Reserved(reset to zero), D3..0: The endpointer number
				FS_C2_I1_A1_EP1_TYPE,		// bmAttributes
								// D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
								// if not an isochronous endpoint, D7..2 are Reserved
				mLowByte(FS_C2_I1_A1_EP1_MAX_PACKET),	// wMaxPacketSize
				mHighByte(FS_C2_I1_A1_EP1_MAX_PACKET),
				FS_C2_I1_A1_EP1_bInterval,						// Interval for polling endpoint for data transfers.
			#endif
			#if (FS_C2_I1_A1_EP_NUMBER >= 2)
				// EP2
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A1_EP2_DIRECTION) << 7) | EP2),
				FS_C2_I1_A1_EP2_TYPE,	
				mLowByte(FS_C2_I1_A1_EP2_MAX_PACKET),
				mHighByte(FS_C2_I1_A1_EP2_MAX_PACKET),
				FS_C2_I1_A1_EP2_bInterval,
			#endif
			#if (FS_C2_I1_A1_EP_NUMBER >= 3)
				// EP3
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A1_EP3_DIRECTION) << 7) | EP3),
				FS_C2_I1_A1_EP3_TYPE,	
				mLowByte(FS_C2_I1_A1_EP3_MAX_PACKET),
				mHighByte(FS_C2_I1_A1_EP3_MAX_PACKET),
				FS_C2_I1_A1_EP3_bInterval,
			#endif
			#if (FS_C2_I1_A1_EP_NUMBER >= 4)
				// EP4
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A1_EP4_DIRECTION) << 7) | EP4),
				FS_C2_I1_A1_EP4_TYPE,	
				mLowByte(FS_C2_I1_A1_EP4_MAX_PACKET),
				mHighByte(FS_C2_I1_A1_EP4_MAX_PACKET),
				FS_C2_I1_A1_EP4_bInterval,
			#endif
			#if (FS_C2_I1_A1_EP_NUMBER >= 5)
				// EP5
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A1_EP5_DIRECTION) << 7) | EP5),
				FS_C2_I1_A1_EP5_TYPE,	
				mLowByte(FS_C2_I1_A1_EP5_MAX_PACKET),
				mHighByte(FS_C2_I1_A1_EP5_MAX_PACKET),
				FS_C2_I1_A1_EP5_bInterval,
			#endif
			#if (FS_C2_I1_A1_EP_NUMBER >= 6)
				// EP6
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A1_EP6_DIRECTION) << 7) | EP6),
				FS_C2_I1_A1_EP6_TYPE,	
				mLowByte(FS_C2_I1_A1_EP6_MAX_PACKET),
				mHighByte(FS_C2_I1_A1_EP6_MAX_PACKET),
				FS_C2_I1_A1_EP6_bInterval,
			#endif
			#if (FS_C2_I1_A1_EP_NUMBER >= 7)
				// EP7
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A1_EP7_DIRECTION) << 7) | EP7),
				FS_C2_I1_A1_EP7_TYPE,	
				mLowByte(FS_C2_I1_A1_EP7_MAX_PACKET),
				mHighByte(FS_C2_I1_A1_EP7_MAX_PACKET),
				FS_C2_I1_A1_EP7_bInterval,
			#endif
			#if (FS_C2_I1_A1_EP_NUMBER >= 8)
				// EP8
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A1_EP8_DIRECTION) << 7) | EP8),
				FS_C2_I1_A1_EP8_TYPE,	
				mLowByte(FS_C2_I1_A1_EP8_MAX_PACKET),
				mHighByte(FS_C2_I1_A1_EP8_MAX_PACKET),
				FS_C2_I1_A1_EP8_bInterval,
			#endif
			#if (FS_C2_I1_A1_EP_NUMBER >= 9)
				// EP9
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A1_EP9_DIRECTION) << 7) | EP9),
				FS_C2_I1_A1_EP9_TYPE,	
				mLowByte(FS_C2_I1_A1_EP9_MAX_PACKET),
				mHighByte(FS_C2_I1_A1_EP9_MAX_PACKET),
				FS_C2_I1_A1_EP9_bInterval,
			#endif
			#if (FS_C2_I1_A1_EP_NUMBER >= 10)
				// EP10
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A1_EP10_DIRECTION) << 7) | EP10),
				FS_C2_I1_A1_EP10_TYPE,	
				mLowByte(FS_C2_I1_A1_EP10_MAX_PACKET),
				mHighByte(FS_C2_I1_A1_EP10_MAX_PACKET),
				FS_C2_I1_A1_EP10_bInterval,
			#endif
			#if (FS_C2_I1_A1_EP_NUMBER >= 11)
				// EP11
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A1_EP11_DIRECTION) << 7) | EP11),
				FS_C2_I1_A1_EP11_TYPE,	
				mLowByte(FS_C2_I1_A1_EP11_MAX_PACKET),
				mHighByte(FS_C2_I1_A1_EP11_MAX_PACKET),
				FS_C2_I1_A1_EP11_bInterval,
			#endif
			#if (FS_C2_I1_A1_EP_NUMBER >= 12)
				// EP12
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A1_EP12_DIRECTION) << 7) | EP12),
				FS_C2_I1_A1_EP12_TYPE,	
				mLowByte(FS_C2_I1_A1_EP12_MAX_PACKET),
				mHighByte(FS_C2_I1_A1_EP12_MAX_PACKET),
				FS_C2_I1_A1_EP12_bInterval,
			#endif
			#if (FS_C2_I1_A1_EP_NUMBER >= 13)
				// EP13
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A1_EP13_DIRECTION) << 7) | EP13),
				FS_C2_I1_A1_EP13_TYPE,	
				mLowByte(FS_C2_I1_A1_EP13_MAX_PACKET),
				mHighByte(FS_C2_I1_A1_EP13_MAX_PACKET),
				FS_C2_I1_A1_EP13_bInterval,
			#endif
			#if (FS_C2_I1_A1_EP_NUMBER >= 14)
				// EP14
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A1_EP14_DIRECTION) << 7) | EP14),
				FS_C2_I1_A1_EP14_TYPE,	
				mLowByte(FS_C2_I1_A1_EP14_MAX_PACKET),
				mHighByte(FS_C2_I1_A1_EP14_MAX_PACKET),
				FS_C2_I1_A1_EP14_bInterval,
			#endif
			#if (FS_C2_I1_A1_EP_NUMBER >= 15)
				// EP15
				EP_LENGTH,
				DT_ENDPOINT,
				(((1 - FS_C2_I1_A1_EP15_DIRECTION) << 7) | EP15),
				FS_C2_I1_A1_EP15_TYPE,	
				mLowByte(FS_C2_I1_A1_EP15_MAX_PACKET),
				mHighByte(FS_C2_I1_A1_EP15_MAX_PACKET),
				FS_C2_I1_A1_EP15_bInterval,
			#endif
		#endif

	#endif
};
#endif


MMP_UINT8 u8OTGDeviceDescriptorEX[DEVICE_LENGTH];
MMP_UINT8 u8ConfigOTGDescriptorEX[CONFIG_LENGTH_EX];
MMP_UINT8 u8OtherSpeedConfigOTGDescriptorEX[OTHER_SPEED_CONFIG_LENGTH_EX];
MMP_UINT8 u8OTGDeviceQualifierDescriptorEX[DEVICE_QUALIFIER_LENGTH];

