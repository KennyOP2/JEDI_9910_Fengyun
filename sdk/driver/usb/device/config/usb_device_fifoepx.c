/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */

#include "usb/device/usb_device_table.h"


/////////////////////////////////////////////////////
//      OTG_DEV_vOTGFIFO_EPxCfg_HS(void)
//      Description:
//          1. Configure the FIFO and EPx map
//      input: none
//      output: none
/////////////////////////////////////////////////////
void OTG_DEV_vOTGFIFO_EPxCfg_HS(void)
{
    MMP_UINT32 i;

    switch(u8OTGConfigValue)
    {
        #if (HS_CONFIGURATION_NUMBER >= 0X01)
        // Configuration 0X01
        case 0X01:
            switch(u8OTGInterfaceValue)
            {
                #if (HS_C1_INTERFACE_NUMBER >= 0x01)
                // Interface 0
                case 0:
                    switch(u8OTGInterfaceAlternateSetting)
                    {
                        #if (HS_C1_I0_ALT_NUMBER >= 0X01)
                        // AlternateSetting 0
                        case 0:
                            #if (HS_C1_I0_A0_EP_NUMBER >= 0X01)
                            //EP0X01
                            USB_DEVICE_SetEndpointMapReg(EP1, HS_C1_I0_A0_EP1_MAP);
                            USB_DEVICE_SetFifoMapReg(HS_C1_I0_A0_EP1_FIFO_START, HS_C1_I0_A0_EP1_FIFO_MAP);
                            USB_DEVICE_SetFifoConfigReg(HS_C1_I0_A0_EP1_FIFO_START, HS_C1_I0_A0_EP1_FIFO_CONFIG);
                            
                            for(i = HS_C1_I0_A0_EP1_FIFO_START + 1 ;
                                i < HS_C1_I0_A0_EP1_FIFO_START + HS_C1_I0_A0_EP1_FIFO_NO ;
                                i ++)
                            {
                                USB_DEVICE_SetFifoConfigReg(i, (HS_C1_I0_A0_EP1_FIFO_CONFIG & (~BIT7)) );
                            }
                            
                            USB_DEVICE_SetMaxPacketSizeReg(EP1, HS_C1_I0_A0_EP1_DIRECTION, (HS_C1_I0_A0_EP1_MAX_PACKET & 0x7ff) );
                            USB_DEVICE_SetTxNumHighBandwidthReg(EP1 , HS_C1_I0_A0_EP1_DIRECTION , HS_C1_I0_A0_EP1_MAX_PACKET);
                            #endif
                            #if (HS_C1_I0_A0_EP_NUMBER >= 0X02)
                            //EP0X02
                            USB_DEVICE_SetEndpointMapReg(EP2, HS_C1_I0_A0_EP2_MAP);
                            
                            #if((OTG_AP_Satus == Bulk_AP) && (Bulk_Satus == Bulk_FIFO_SingleDir))
                            USB_DEVICE_SetFifoMapReg(HS_C1_I0_A0_EP2_FIFO_START, HS_C1_I0_A0_EP2_FIFO_MAP);
                            #endif
                            
                            USB_DEVICE_SetFifoConfigReg(HS_C1_I0_A0_EP2_FIFO_START, HS_C1_I0_A0_EP2_FIFO_CONFIG);
                            
                            for(i = HS_C1_I0_A0_EP2_FIFO_START + 1 ;
                                i < HS_C1_I0_A0_EP2_FIFO_START + HS_C1_I0_A0_EP2_FIFO_NO ;
                                i ++)
                            {
                                USB_DEVICE_SetFifoConfigReg(i, (HS_C1_I0_A0_EP2_FIFO_CONFIG & (~BIT7)) );
                            }
                            
                            USB_DEVICE_SetMaxPacketSizeReg(EP2, HS_C1_I0_A0_EP2_DIRECTION, (HS_C1_I0_A0_EP2_MAX_PACKET & 0x7ff) );
                            USB_DEVICE_SetTxNumHighBandwidthReg(EP2 , HS_C1_I0_A0_EP2_DIRECTION , HS_C1_I0_A0_EP2_MAX_PACKET);
                            #endif
                            
                            #if (HS_C1_I0_A0_EP_NUMBER >= 0X03)
                            //EP0X03
                            USB_DEVICE_SetEndpointMapReg(EP3, HS_C1_I0_A0_EP3_MAP);                     
                            USB_DEVICE_SetFifoMapReg(HS_C1_I0_A0_EP3_FIFO_START, HS_C1_I0_A0_EP3_FIFO_MAP);
                            USB_DEVICE_SetFifoConfigReg(HS_C1_I0_A0_EP3_FIFO_START, HS_C1_I0_A0_EP3_FIFO_CONFIG);
                            
                            for(i = HS_C1_I0_A0_EP3_FIFO_START + 1 ;
                                i < HS_C1_I0_A0_EP3_FIFO_START + HS_C1_I0_A0_EP3_FIFO_NO ;
                                i ++)
                            {
                                USB_DEVICE_SetFifoConfigReg(i, (HS_C1_I0_A0_EP3_FIFO_CONFIG & (~BIT7)) );
                            }
                            
                            USB_DEVICE_SetMaxPacketSizeReg(EP3, HS_C1_I0_A0_EP3_DIRECTION, (HS_C1_I0_A0_EP3_MAX_PACKET & 0x7ff) );
                            USB_DEVICE_SetTxNumHighBandwidthReg(EP3, HS_C1_I0_A0_EP3_DIRECTION , HS_C1_I0_A0_EP3_MAX_PACKET);
                            #endif
                            
                            #if (HS_C1_I0_A0_EP_NUMBER >= 0X04)
                            //EP0X04
                            USB_DEVICE_SetEndpointMapReg(EP4, HS_C1_I0_A0_EP4_MAP);                         
                            USB_DEVICE_SetFifoMapReg(HS_C1_I0_A0_EP4_FIFO_START, HS_C1_I0_A0_EP4_FIFO_MAP);                     
                            USB_DEVICE_SetFifoConfigReg(HS_C1_I0_A0_EP4_FIFO_START, HS_C1_I0_A0_EP4_FIFO_CONFIG);
                            
                            for(i = HS_C1_I0_A0_EP4_FIFO_START + 1 ;
                                i < HS_C1_I0_A0_EP4_FIFO_START + HS_C1_I0_A0_EP4_FIFO_NO ;
                                i ++)
                            {
                                USB_DEVICE_SetFifoConfigReg(i, (HS_C1_I0_A0_EP4_FIFO_CONFIG & (~BIT7)) );
                            }
                            
                            USB_DEVICE_SetMaxPacketSizeReg(EP4, HS_C1_I0_A0_EP4_DIRECTION, (HS_C1_I0_A0_EP4_MAX_PACKET & 0x7ff) );
                            USB_DEVICE_SetTxNumHighBandwidthReg(EP4 , HS_C1_I0_A0_EP4_DIRECTION , HS_C1_I0_A0_EP4_MAX_PACKET);
                            #endif
                            break;
                        #endif
                        default:
                            break;
                    }
                    break;
                #endif
                default:
                    break;
            }
            break;
        #endif
        default:
            break;
    }
}

void OTG_DEV_vOTGFIFO_EPxCfg_FS(void)
{
    MMP_UINT32 i;
    switch(u8OTGConfigValue)
    {
        #if (FS_CONFIGURATION_NUMBER >= 0X01)
        // Configuration 0X01
        case 0X01:
            switch(u8OTGInterfaceValue)
            {
                #if (FS_C1_INTERFACE_NUMBER >= 0x01)
                // Interface 0
                case 0:
                    switch(u8OTGInterfaceAlternateSetting)
                    {
                        #if (FS_C1_I0_ALT_NUMBER >= 0X01)
                        // AlternateSetting 0
                        case 0:
                            #if (FS_C1_I0_A0_EP_NUMBER >= 0X01)
                            //EP0X01
                            USB_DEVICE_SetEndpointMapReg(EP1, FS_C1_I0_A0_EP1_MAP);
                            USB_DEVICE_SetFifoMapReg(FS_C1_I0_A0_EP1_FIFO_START, FS_C1_I0_A0_EP1_FIFO_MAP);
                            USB_DEVICE_SetFifoConfigReg(FS_C1_I0_A0_EP1_FIFO_START, FS_C1_I0_A0_EP1_FIFO_CONFIG);
                            
                            for(i = FS_C1_I0_A0_EP1_FIFO_START + 1 ;
                                i < FS_C1_I0_A0_EP1_FIFO_START + FS_C1_I0_A0_EP1_FIFO_NO ;
                                i ++)
                            {
                                USB_DEVICE_SetFifoConfigReg(i, (FS_C1_I0_A0_EP1_FIFO_CONFIG & (~BIT7)) );
                            }
                            
                            USB_DEVICE_SetMaxPacketSizeReg(EP1, FS_C1_I0_A0_EP1_DIRECTION, (FS_C1_I0_A0_EP1_MAX_PACKET & 0x7ff));
                            USB_DEVICE_SetTxNumHighBandwidthReg(EP1 , FS_C1_I0_A0_EP1_DIRECTION, FS_C1_I0_A0_EP1_MAX_PACKET);
                            #endif
                            #if (FS_C1_I0_A0_EP_NUMBER >= 0X02)
                            //EP0X02
                            USB_DEVICE_SetEndpointMapReg(EP2, FS_C1_I0_A0_EP2_MAP);
                            
                            #if((OTG_AP_Satus == Bulk_AP) && (Bulk_Satus == Bulk_FIFO_SingleDir))                           
                            USB_DEVICE_SetFifoMapReg(FS_C1_I0_A0_EP2_FIFO_START, FS_C1_I0_A0_EP2_FIFO_MAP);
                            #endif
                            
                            USB_DEVICE_SetFifoConfigReg(FS_C1_I0_A0_EP2_FIFO_START, FS_C1_I0_A0_EP2_FIFO_CONFIG);
                            
                            for(i = FS_C1_I0_A0_EP2_FIFO_START + 1 ;
                                i < FS_C1_I0_A0_EP2_FIFO_START + FS_C1_I0_A0_EP2_FIFO_NO ;
                                i ++)
                            {
                                USB_DEVICE_SetFifoConfigReg(i, (FS_C1_I0_A0_EP2_FIFO_CONFIG & (~BIT7)) );
                            }
                            
                            USB_DEVICE_SetMaxPacketSizeReg(EP2, FS_C1_I0_A0_EP2_DIRECTION, (FS_C1_I0_A0_EP2_MAX_PACKET & 0x7ff));
                            USB_DEVICE_SetTxNumHighBandwidthReg(EP2 , FS_C1_I0_A0_EP2_DIRECTION, FS_C1_I0_A0_EP2_MAX_PACKET);
                            #endif
                            #if (FS_C1_I0_A0_EP_NUMBER >= 0X03)
                            //EP0X03
                            USB_DEVICE_SetEndpointMapReg(EP3, FS_C1_I0_A0_EP3_MAP);
                            USB_DEVICE_SetFifoMapReg(FS_C1_I0_A0_EP3_FIFO_START, FS_C1_I0_A0_EP3_FIFO_MAP);
                            USB_DEVICE_SetFifoConfigReg(FS_C1_I0_A0_EP3_FIFO_START, FS_C1_I0_A0_EP3_FIFO_CONFIG);
                            
                            for(i = FS_C1_I0_A0_EP3_FIFO_START + 1 ;
                                i < FS_C1_I0_A0_EP3_FIFO_START + FS_C1_I0_A0_EP3_FIFO_NO ;
                                i ++)
                            {
                                USB_DEVICE_SetFifoConfigReg(i, (FS_C1_I0_A0_EP3_FIFO_CONFIG & (~BIT7)) );
                            }
                            
                            USB_DEVICE_SetMaxPacketSizeReg(EP3, FS_C1_I0_A0_EP3_DIRECTION, (FS_C1_I0_A0_EP3_MAX_PACKET & 0x7ff));
                            USB_DEVICE_SetTxNumHighBandwidthReg(EP3 , FS_C1_I0_A0_EP3_DIRECTION, FS_C1_I0_A0_EP3_MAX_PACKET);
                            #endif
                            #if (FS_C1_I0_A0_EP_NUMBER >= 0X04)
                            //EP0X01
                            USB_DEVICE_SetEndpointMapReg(EP4, FS_C1_I0_A0_EP4_MAP);
                            USB_DEVICE_SetFifoMapReg(FS_C1_I0_A0_EP4_FIFO_START, FS_C1_I0_A0_EP4_FIFO_MAP);
                            USB_DEVICE_SetFifoConfigReg(FS_C1_I0_A0_EP4_FIFO_START, FS_C1_I0_A0_EP4_FIFO_CONFIG);
                            
                            for(i = FS_C1_I0_A0_EP4_FIFO_START + 1 ;
                                i < FS_C1_I0_A0_EP4_FIFO_START + FS_C1_I0_A0_EP4_FIFO_NO ;
                                i ++)
                            {
                                USB_DEVICE_SetFifoConfigReg(i, (FS_C1_I0_A0_EP4_FIFO_CONFIG & (~BIT7)) );
                            }
                            
                            USB_DEVICE_SetMaxPacketSizeReg(EP4, FS_C1_I0_A0_EP4_DIRECTION, (FS_C1_I0_A0_EP4_MAX_PACKET & 0x7ff));
                            USB_DEVICE_SetTxNumHighBandwidthReg(EP4 , FS_C1_I0_A0_EP4_DIRECTION, FS_C1_I0_A0_EP4_MAX_PACKET);
                            #endif
                            break;
                        #endif
                        default:
                            break;
                    }
                    break;
                #endif
                default:
                    break;
            }
            break;
        #endif
        default:
            break;
    }
}

