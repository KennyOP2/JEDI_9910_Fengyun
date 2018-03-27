#ifndef USB_DEVICE_USER_H
#define USB_DEVICE_USER_H


#define Bulk_AP					0
#define Interrupt_AP			1   //Important:this setting only for cross-connection, NOT for PC test-bench
#define IsochronousIN_AP		2		
#define IsochronousOUT_AP		3	
#define OTG_AP_Satus			Bulk_AP

#define Bulk_FIFO_SingleDir		0	
#define Bulk_FIFO_BiDir			1
			
#define Bulk_Satus				Bulk_FIFO_BiDir


#endif

