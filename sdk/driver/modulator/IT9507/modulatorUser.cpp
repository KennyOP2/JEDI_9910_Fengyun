#include "pal/pal.h"
//#include "pthread.h"
#include "mmp_iic.h"
#include "modulatorUser.h"
#include "../usb_mod/usb_mod_transport.h"


Dword EagleUser_memoryCopy (
    IN  Modulator*    modulator,
    IN  void*           dest,
    IN  void*           src,
    IN  Dword           count
) {
    /*
     *  ToDo:  Add code here
     *
     *  //Pseudo code
     *  memcpy(dest, src, (size_t)count);
     *  return (0);
     */
    return (ModulatorError_NO_ERROR);
}

Dword EagleUser_delay (
    IN  Modulator*    modulator,
    IN  Dword           dwMs
) {
    /*
     *  ToDo:  Add code here
     *
     *  //Pseudo code
     *  delay(dwMs);
     *  return (0);
     */
    PalSleep(dwMs);
    return (ModulatorError_NO_ERROR);
}

Dword EagleUser_createCriticalSection (
    void
) {
    /*
     *  ToDo:  Add code here
     *
     *  //Pseudo code
     *  return (0);
     */
#if 0
    if (gAfaMutex)
        return ModulatorError_NO_ERROR;
 
    pthread_mutex_init(&gAfaMutex, NULL);

    if (0 == gAfaMutex)
        return ModulatorError_NULL_PTR;
#endif   
    return (ModulatorError_NO_ERROR);
}

Dword EagleUser_deleteCriticalSection (
    void
) {
    /*
     *  ToDo:  Add code here
     *
     *  //Pseudo code
     *  return (0);
     */
#if 0
    if (0 == gAfaMutex)
        return ModulatorError_NO_ERROR;
 
    pthread_mutex_destroy(&gAfaMutex);
    gAfaMutex = 0;
#endif    
    return (ModulatorError_NO_ERROR);
}

Dword EagleUser_enterCriticalSection (
    IN  Modulator*    modulator
) {
    /*
     *  ToDo:  Add code here
     *
     *  //Pseudo code
     *  return (0);
     */
    return (ModulatorError_NO_ERROR);
}


Dword EagleUser_leaveCriticalSection (
    IN  Modulator*    modulator
) {
    /*
     *  ToDo:  Add code here
     *
     *  //Pseudo code
     *  return (0);
     */
    return (ModulatorError_NO_ERROR);
}


Dword EagleUser_mpegConfig (
    IN  Modulator*    modulator
) {
    /*
     *  ToDo:  Add code here
     *
     */
    return (ModulatorError_NO_ERROR);
}


Dword EagleUser_busTx (
    IN  Modulator*    modulator,
    IN  Dword           bufferLength,
    IN  Byte*           buffer
) {
    /*
     *  ToDo:  Add code here
     *
     *  //Pseudo code
     *  short i;
     *
     *  start();
     *  write_i2c(uc2WireAddr);
     *  ack();
     *  for (i = 0; i < bufferLength; i++) {
     *      write_i2c(*(ucpBuffer + i));
     *      ack();
     *  }
     *  stop();
     *
     *  // If no error happened return 0, else return error code.
     *  return (0);
     */
    Dword error = ModulatorError_NO_ERROR;

    Modulator* eagle;
    eagle = (Modulator*) modulator;

    switch( eagle->busId )
    {       
        case Bus_I2C:
            error = mmpIicSendDataEx(IIC_MASTER_MODE, 
                                      (EagleUser_IIC_ADDRESS >> 1),
                                      (MMP_UINT8*) buffer, 
                                      (MMP_UINT16) bufferLength);
            if (0 != error)
            {
                mmpIicGenStop();
            }        
            break;
            
        case Bus_USB:
            error = usb_mod_CBI_sendcmd((struct usbtuner_data*) eagle->usb_info,
                                           (MMP_UINT8*) buffer, (MMP_UINT16) bufferLength);
                                           
            if( 0 != error )    printf(" err ! 0x%08x, %s()[#%d]\n", error, __FUNCTION__, __LINE__);                               
            break;
    }
    return error;
}


Dword EagleUser_busRx (
    IN  Modulator*    modulator,
    IN  Dword           bufferLength,
    OUT Byte*           buffer
) {
    /*
     *  ToDo:  Add code here
     *
     *  //Pseudo code
     *  short i;
     *
     *  start();
     *  write_i2c(uc2WireAddr | 0x01);
     *  ack();
     *  for (i = 0; i < bufferLength - 1; i++) {
     *      read_i2c(*(ucpBuffer + i));
     *      ack();
     *  }
     *  read_i2c(*(ucpBuffer + bufferLength - 1));
     *  nack();
     *  stop();
     *
     *  // If no error happened return 0, else return error code.
     *  return (0);
     */
    Dword error = ModulatorError_NO_ERROR; 
    MMP_UINT8* pDummyWrBuffer = 0;
    MMP_UINT16 dummyWrSize = 0;

    Modulator* eagle;
    eagle = (Modulator*) modulator;

    switch( eagle->busId )
    { 
        case Bus_I2C:
            error = mmpIicReceiveDataEx(IIC_MASTER_MODE, 
                                         (EagleUser_IIC_ADDRESS >> 1),
                                         pDummyWrBuffer, dummyWrSize,
                                         (MMP_UINT8*) buffer, (MMP_UINT16) bufferLength);

            if (0 != error)
            {
                mmpIicGenStop();
            }
            break;
            
        case Bus_USB:
            error = usb_mod_CBI_receivecmd((struct usbtuner_data*) eagle->usb_info,
                                              (MMP_UINT8*) buffer, (MMP_UINT16) bufferLength);

            if( 0 != error )   printf(" err ! 0x%08x, %s()[#%d]\n", error, __FUNCTION__, __LINE__);
            break;
    } 
    return error;
}

Dword EagleUser_busTxData (
    IN  Modulator*    modulator,
    IN  Dword           bufferLength,
    OUT Byte*           buffer
) {
    //return (ModulatorError_NO_ERROR);
    Dword error = ModulatorError_NO_ERROR; 
    
    Modulator* eagle;
    eagle = (Modulator*) modulator;

    error = usb_mod_CBI_transferData((struct usbtuner_data*) eagle->usb_info,
                                           (MMP_UINT8*) buffer, (MMP_UINT16) bufferLength);

    
    if (0 != error)    printf(" err ! 0x%08x, %s()[#%d]\n", error, __FUNCTION__, __LINE__);

    return error;
}

Dword EagleUser_setBus (
    IN  Modulator*    modulator,
    IN  Byte            busId,
    IN  Byte            i2cAddr
) {
    Dword error = ModulatorError_NO_ERROR;
    return(error);
}


 Dword EagleUser_Initialization  (
    IN  Modulator*    modulator
) {
    /*
     *  ToDo:  Add code here
     *
     *  // If no error happened return 0, else return error code.
     *  return (0);
     */
    Dword error = 0;
   
#if defined (IT9919_144TQFP)
    #if defined (REF_BOARD_CAMERA)
        error = IT9507_writeRegister (modulator, Processor_LINK, p_eagle_reg_top_gpioh8_o, 0); 
        if (error) goto exit;     
        error = IT9507_writeRegister (modulator, Processor_LINK, p_eagle_reg_top_gpioh8_en, 1);//U/V filter control pin
        if (error) goto exit;
        error = IT9507_writeRegister (modulator, Processor_LINK, p_eagle_reg_top_gpioh8_on, 1);
        if (error) goto exit;
    #endif
#endif 

#if defined (IT9919_144TQFP)
    #if (defined (REF_BOARD_CAMERA) || defined(REF_BOARD_AVSENDER))
        error = IT9507_writeRegister (modulator, Processor_LINK, p_eagle_reg_top_gpioh2_o, 0); //RF out power down
        if (error) goto exit;
        error = IT9507_writeRegister (modulator, Processor_LINK, p_eagle_reg_top_gpioh2_en, 1);
        if (error) goto exit;
        error = IT9507_writeRegister (modulator, Processor_LINK, p_eagle_reg_top_gpioh2_on, 1);
        if (error) goto exit; 
        
    #endif
#endif 

exit:
    return (error);

 }


Dword EagleUser_Finalize  (
    IN  Modulator*    modulator
) {
    /*
     *  ToDo:  Add code here
     *
     *  // If no error happened return 0, else return error code.
     *  return (0);
     */
    Dword error = 0;
    return (error);

 }
 

Dword EagleUser_acquireChannel (
    IN  Modulator*    modulator,
    IN  Word          bandwidth,
    IN  Dword         frequency
){

    Dword error = ModulatorError_NO_ERROR;
            
#if defined (IT9919_144TQFP)
    #if defined (REF_BOARD_CAMERA)                     
        if(frequency <= 230000) // <=230000KHz v-filter gpio set to Lo
        { 
            error = IT9507_writeRegister (modulator, Processor_LINK, p_eagle_reg_top_gpioh8_o, 0); 
            if (error) goto exit;        
        }
        else
        {
            error = IT9507_writeRegister (modulator, Processor_LINK, p_eagle_reg_top_gpioh8_o, 1); 
            if (error) goto exit;    
        }         
    #endif
#endif   

exit:
    return (error);
}

Dword EagleUser_setTxModeEnable (
    IN  Modulator*            modulator,
    IN  Byte                    enable  
) {
    Dword error = ModulatorError_NO_ERROR;

#if defined (IT9919_144TQFP)
    #if (defined (REF_BOARD_CAMERA) || defined(REF_BOARD_AVSENDER))  
    if(enable)
    {
        printf("RF power on\n");
        error = IT9507_writeRegister (modulator, Processor_LINK, p_eagle_reg_top_gpioh2_o, 1); //RF power up 
        if (error) goto exit;
    }
    else
    {
        printf("RF power off\n");
        error = IT9507_writeRegister (modulator, Processor_LINK, p_eagle_reg_top_gpioh2_o, 0); //RF power down 
        if (error) goto exit;        
    }   
    #endif
#endif  
    
exit :
    
    return (error);
}


Dword EagleUser_getChannelIndex (
    IN  Modulator*            modulator,
    IN  Byte*                    index  
) {
    Dword error = ModulatorError_NO_ERROR;
    return (error);
}

