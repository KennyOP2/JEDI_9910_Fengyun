/*
 * Copyright (c) 2005 SMedia technology Corp. All Rights Reserved.
 */
/** @file
 * SMedia IIC Driver API header file.
 *
 * @author Alex.C Hsieh
 * @version 0.9
 */
#ifndef MMP_IIC_H
#define MMP_IIC_H

#include "mmp_types.h"
#include "mmp.h"

/**
 * DLL export API declaration for Win32.
 */
#if defined(WIN32)
#if defined(IIC_EXPORTS)
#define IIC_API __declspec(dllexport)
#else
#define IIC_API __declspec(dllimport)
#endif
#else
#define IIC_API extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*Error Code*/
#define I2C_ERROR_CODE_BASE            0x012C0000

#define I2C_NON_ACK						(I2C_ERROR_CODE_BASE + 1u)
#define I2C_ARBITRATION_LOSS			(I2C_ERROR_CODE_BASE + 2u)
#define I2C_MODE_TRANSMIT_ERROR			(I2C_ERROR_CODE_BASE + 3u)
#define I2C_MODE_RECEIVE_ERROR			(I2C_ERROR_CODE_BASE + 4u)
#define I2C_WAIT_TRANSMIT_TIME_OUT		(I2C_ERROR_CODE_BASE + 5u)
#define I2C_INVALID_ACK                 (I2C_ERROR_CODE_BASE + 6u)

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef MMP_LONG MMP_IIC_HANDLE;

typedef enum IIC_HW_CONFIG_MODE_TAG
{
    //GPIO_MODE,
    CONTROLLER_MODE
}IIC_HW_CONFIG_MODE;

typedef enum OP_MODE_TAG
{
    IIC_SLAVE_MODE,
    IIC_MASTER_MODE
}IIC_OP_MODE;



//=============================================================================
//                              Function Declaration
//=============================================================================

/** @defgroup group9 SMedia IIC Driver API
 *  The supported API for IIC.
 *  @{
 */

/**
 * Initialize IIC
 *
 * @hDevice     A handle to IIC device
 * @gpio_group  GPIO group ID that connect to IIC device.
 * @sclk_pin    GPIO pin number which connect to IIC sclk pin.
 * @data_pin    GPIO pin number which connect to IIC data pin.
 * @delay       specify delay time (in ms) for each GPIO operation
 * @bModuleLock Whether provide the lock module feature for AP layer.
 * @return      MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR 
 *              otherwise.
 *
 * @remark Application must call this API first when it want to use IIC API.
 * Before mmpIicTerminate() this API will be used once only.
 */
IIC_API MMP_RESULT
mmpIicInitialize(
    IIC_HW_CONFIG_MODE mode,
    MMP_IIC_HANDLE *hDevice,
    MMP_GPIO_GROUP gpio_group,
    MMP_GPIO_PIN   sclk_pin,
    MMP_GPIO_PIN   data_pin,
    MMP_ULONG      delay,
    MMP_BOOL       bModuleLock);

/**
 * Terminate IIC
 *
 * @hDevice     A handle to IIC device
 *
 * @return      MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR 
 *              otherwise.
 */
IIC_API MMP_RESULT
mmpIicTerminate(
    MMP_IIC_HANDLE hDevice);

/**
 * Recieve device data packet throught IIC
 *
 * @hDevice     A handle to IIC device
 * @data        Recieved device data
 * @return      MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR 
 *              otherwise.
 */
IIC_API MMP_RESULT
mmpIicRecieve(
    MMP_IIC_HANDLE hDevice,
    MMP_UINT       *data);

/**
 * Send device data packet throught IIC
 *
 * @hDevice     A handle to IIC device
 * @data        Data send to device
 * @return      MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR 
 *              otherwise.
 */
IIC_API MMP_RESULT
mmpIicSend(
    MMP_IIC_HANDLE hDevice,
    MMP_UINT       data);

/**
 * IIC start condition
 *
 * @hDevice     A handle to IIC device
 * @return      MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR 
 *              otherwise.
 */
IIC_API MMP_RESULT
mmpIicStart(
    MMP_IIC_HANDLE hDevice);

/**
 * IIC stop condition
 *
 * @hDevice     A handle to IIC device
 * @return      MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR 
 *              otherwise.
 */
IIC_API MMP_RESULT
mmpIicStop(
    MMP_IIC_HANDLE hDevice);

/**
 * Send data to I2C slave device.
 *
 * @param mode		    master/salve mode.So far only support master mode.
 * @param slaveAddr     slave address.
 * @param regAddr		starting reigster address.
 * @param pbuffer       output buffer pointer.
 * @param size			number of bytes to Send.Maximum size is 254.
 * @return MMP_RESULT_SUCCESS if succeed, otherwise error codes 
 */
MMP_API MMP_RESULT
mmpIicSendData(
    IIC_OP_MODE mode,
    MMP_UINT8 slaveAddr,
	MMP_UINT8 regAddr,
    MMP_UINT8 *pbuffer,
    MMP_UINT16 size);

/**
 * Send data to I2C slave device.
 *
 * @param mode		    master/salve mode.So far only support master mode.
 * @param addr          slave address.
 * @param pbuffer       output buffer pointer.
 * @param size			number of bytes to Send.Maximum size is 254.
 * @return MMP_RESULT_SUCCESS if succeed, otherwise error codes 
 */
MMP_API MMP_RESULT
mmpIicSendDataEx(
    IIC_OP_MODE mode,
    MMP_UINT8 slaveAddr,
    MMP_UINT8 *pbuffer,
    MMP_UINT16 size);

/**
 * Receive data to I2C slave device.
 *
 * @param mode		    master/salve mode.So far only support master mode.
 * @param slaveAddr		slave address.
 * @param regAddr		starting reigster address.
 * @param pbuffer		input buffer pointer.
 * @param size			number of bytes to receive.Maximum size is 254.
 * @return MMP_RESULT_SUCCESS if succeed, otherwise error codes 
 */
MMP_API MMP_RESULT
mmpIicReceiveData(
    IIC_OP_MODE mode,
    MMP_UINT8 slaveAddr,
	MMP_UINT8 regAddr,
    MMP_UINT8 *pbuffer,
    MMP_UINT16 size);

/**
 * Receive data to I2C slave device.
 *
 * @param mode		    master/salve mode.So far only support master mode.
 * @param slaveAddr		slave address.
 * @param pwbuffer		output buffer pointer.
 * @param wsize		    number of bytes to send.Maximum size is 254.
 * @param prbuffer		input buffer pointer.
 * @param rsize			number of bytes to receive.Maximum size is 254.
 * @return MMP_RESULT_SUCCESS if succeed, otherwise error codes 
 */
MMP_API MMP_RESULT
mmpIicReceiveDataEx(
    IIC_OP_MODE mode,
    MMP_UINT8 slaveAddr,
    MMP_UINT8 *pwbuffer,
    MMP_UINT16 wsize,   
    MMP_UINT8 *prbuffer,
    MMP_UINT16 rsize);

/**
 * Change I2c transferring clock .
 *
 * @param clock      desire clock rate.
 * @return actual clock rate.  
 */
MMP_API MMP_UINT32
mmpIicSetClockRate(
    MMP_UINT32 clock);

/**
 * Get current clock .
 *
 * @return current clock rate.  
 */
MMP_API MMP_UINT32
mmpIicGetClockRate(
    void);      

/**
 * Gen stop indication command .
 *
 * @return MMP_RESULT_SUCCESS if succeed, otherwise error codes 
 */
MMP_API MMP_RESULT
mmpIicGenStop(
    void);

/**
 * Lock whole IIC module to prevent re-entry issue.
 *
 * @return none 
 */
MMP_API void
mmpIicLockModule(
    void);

/**
 * Release the locked IIC module for other task usage.
 *
 * @return none 
 */
MMP_API void
mmpIicReleaseModule(
    void);

MMP_API MMP_UINT32
mmpIicSlaveRead(
	MMP_UINT8* inutBuffer,
	MMP_UINT32 inputBufferLength);

MMP_API MMP_RESULT
mmpIicSlaveWrite(
	MMP_UINT8* outputBuffer,
	MMP_UINT32 outputBufferLength);
    
///////////////////////////////////////////////////////////////////////////////
//@}

#ifdef __cplusplus
}
#endif

#endif // MMP_IIC_H
