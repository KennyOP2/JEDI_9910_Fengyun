#ifndef USB_DEVICE_DEF_H
#define USB_DEVICE_DEF_H


//=============================================================================
//                              Constant Definition
//=============================================================================

// Descriptor Table uses the following parameters : fixed
#define DEVICE_LENGTH				0x12
#define CONFIG_LENGTH				0x09
#define INTERFACE_LENGTH			0x09
#define EP_LENGTH					0x07
#define DEVICE_QUALIFIER_LENGTH		0x0A
#define OTG_LENGTH                  0x03 //For OTG


//==================================
// Constant for Device Descriptor
//==================================
// Table 9-2. - Request type
#define REQUEST_TYPE_STANDARD               0x00
#define REQUEST_TYPE_CLASS                  0x20
#define REQUEST_TYPE_VENDOR                 0x40
#define REQUEST_TYPE_RESERVED               0x60
#define REQUEST_TYPE_MASK                   0x60

// Table 9-2. - Recipient
#define RECIPIENT_TYPE_DEVICE               0
#define RECIPIENT_TYPE_INTERFACE            1
#define RECIPIENT_TYPE_ENDPOINT             2
#define RECIPIENT_TYPE_OTHERS               3
#define RECIPIENT_TYPE_MASK                 0x1F

#define INDEX_DIRECTION_IN                  1
#define INDEX_DIRECTION_OUT                 0

// Table 9-4. - Standard Request code
#define REQUEST_CODE_GET_STATUS             0
#define REQUEST_CODE_CLEAR_FEATURE          1
#define REQUEST_CODE_RESERVED               2
#define REQUEST_CODE_SET_FEATURE            3
#define REQUEST_CODE_RESERVED1              4
#define REQUEST_CODE_SET_ADDRESS            5
#define REQUEST_CODE_GET_DESCRIPTOR         6
#define REQUEST_CODE_SET_DESCRIPTOR         7
#define REQUEST_CODE_GET_CONFIGURATION      8
#define REQUEST_CODE_SET_CONFIGURATION      9
#define REQUEST_CODE_GET_INTERFACE          10
#define REQUEST_CODE_SET_INTERFACE          11
#define REQUEST_CODE_SYNCH_FRAME            12

// Table 9-5. - Descriptor Types
#define DT_DEVICE							1
#define DT_CONFIGURATION					2
#define DT_STRING							3
#define DT_INTERFACE						4
#define DT_ENDPOINT							5
#define DT_DEVICE_QUALIFIER					6
#define DT_OTHER_SPEED_CONFIGURATION		7
#define DT_INTERFACE_POWER					8
#define DT_OTG				                9

// Table 9-6. - Feature Selectors
#define FEATURE_SEL_ENDPOINT_HALT           0
#define FEATURE_SEL_DEVICE_REMOTE_WAKEUP    1
#define FEATURE_SEL_DEVICE_TEST_MODE        2

// Table 9-7. - Test Mode Selectors
#define TEST_MODE_TEST_J                    0x1
#define TEST_MODE_TEST_K                    0x2
#define TEST_MODE_TEST_SE0_NAK              0x3
#define TEST_MODE_TEST_PACKET               0x4
#define TEST_MODE_TEST_FORCE_ENABLE         0x5

	
//==================================
// Constant for Configuration Descriptor
//==================================
//For OTG
#define OTG_SRP_SUPPORT			    0x01
#define OTG_HNP_SUPPORT			    0x02	
#define FS_OTG_SUPPORT              (OTG_SRP_SUPPORT | OTG_HNP_SUPPORT)   





#endif /* #ifndef  USB_DEVICE_DEF_H */

