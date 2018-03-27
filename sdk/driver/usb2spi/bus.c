/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file xcpu_io.c
 *
 * @author I-Chun Lai
 * @version 0.1
 */

#include "usb2spi\bus.h"

#define CHIP_A1     0
#define PRO_A1      1


#if (!defined(CASTOR_BUS_TYPE)) || (CASTOR_BUS_TYPE > 0)

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

#define ENDIAN  (0)     // 0: auto, 1: little, 2: big

#define SECTION_SIZE        (0x10000)
#define MAX_TX_SIZE         (0x10000/3*2)
//#define MAX_PREREAD_COUNT   (4096)
#define MAX_PREREAD_COUNT   (0xF000)

#define MEM_ADDRESS_HI      0x206
#define	PREREAD_ADDRESS_LO  0x208
#define	PREREAD_ADDRESS_HI  0x20A
#define	PREREAD_LENGTH      0x20C
#define	PREREAD_FIRE        0x20E
//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
static MMP_UINT16 gNonAck = 1;
//=============================================================================
//                              Private Function Declaration
//=============================================================================

static MMP_INLINE MMP_UINT16
_UINT16LE_To_UINT16BE(
    MMP_UINT16 int16le)
{
    return ((int16le & 0x00FF) << 8)
         + ((int16le & 0xFF00) >> 8);
}

static MMP_INLINE MMP_UINT16
_UINT16BE_To_UINT16LE(
    MMP_UINT16 int16be)
{
    return ((int16be & 0x00FF) << 8)
         + ((int16be & 0xFF00) >> 8);
}

static MMP_INLINE MMP_UINT32
_UINT32LE_To_UINT32BE(
    MMP_UINT32 int32le)
{
    return ((int32le & 0x000000FFL) << 24)
         + ((int32le & 0x0000FF00L) <<  8)
         + ((int32le & 0x00FF0000L) >>  8)
         + ((int32le & 0xFF000000L) >> 24);
}

static MMP_INLINE MMP_UINT32
_UINT32BE_To_UINT32LE(
    MMP_UINT32 int32be)
{
    return ((int32be & 0x000000FFL) << 24)
         + ((int32be & 0x0000FF00L) <<  8)
         + ((int32be & 0x00FF0000L) >>  8)
         + ((int32be & 0xFF000000L) >> 24);
}

#if (ENDIAN == 0)
MMP_BOOL
_IsBigEndian(
    void);
#endif

//=============================================================================
//                              Public Function Definition
//=============================================================================

#define MAX_READ_DATA_BYTES_BUFFER_SIZE 65536    // 64k bytes
typedef MMP_UINT8 ReadDataByteBuffer[MAX_READ_DATA_BYTES_BUFFER_SIZE];
typedef ReadDataByteBuffer *PReadDataByteBuffer;

#define MAX_WRITE_CONTROL_BYTES_BUFFER_SIZE 256    // 256 bytes
typedef MMP_UINT8 WriteControlByteBuffer[MAX_WRITE_CONTROL_BYTES_BUFFER_SIZE];
typedef WriteControlByteBuffer *PWriteControlByteBuffer;

#define MAX_WRITE_DATA_BYTES_BUFFER_SIZE 65536    // 64k bytes
typedef MMP_UINT8 WriteDataByteBuffer[MAX_WRITE_DATA_BYTES_BUFFER_SIZE];
typedef WriteDataByteBuffer *PWriteDataByteBuffer;

MMP_UINT32 SpiRead(MMP_UINT32 wrLen, PWriteControlByteBuffer pwrBuf, MMP_UINT32 rdLen, PReadDataByteBuffer prdBuf);
MMP_UINT32 SpiRead2(MMP_UINT32 wrLen, MMP_UINT32 wrLenBits, PWriteControlByteBuffer pwrBuf, MMP_UINT32 rdLen, PReadDataByteBuffer prdBuf);
MMP_UINT32 SpiWrite(MMP_UINT32 ctrlLen, PWriteControlByteBuffer pCtrlBuf, MMP_UINT32 dataLen, PWriteDataByteBuffer pDataBuf);
int SpiSetCs(int high);

WriteControlByteBuffer wrBuf;
ReadDataByteBuffer rdBuf;
WriteDataByteBuffer wrDataBuf;


MMP_UINT16
xCpuIO_SaveSpi()
{
    MMP_UINT32 error = 0;

    MMP_UINT32 wrLen;
    MMP_UINT32 rdLen;

retry:
    wrLen = 3;
    rdLen = 3;

    wrBuf[0] = 2;
    wrBuf[1] = 4;
    wrBuf[2] = 0;

    SpiSetCs(0);
    error = SpiRead2(wrLen, 23, &wrBuf, rdLen, &rdBuf);
    SpiSetCs(1);

    if (error)
    {
        PalPrintf("%s : %s:SpiRead2 Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);

#if (1)
        while(1)
        {
            PalSleep(1000);
        }
#endif
    }
    else
    {
     
        if ((rdBuf[0] != 0xFF) || (rdBuf[1] != 0x01) || (rdBuf[2] != 0x00))
        {
            PalPrintf("%s : %s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
            PalPrintf("\nAck Error : 0x%x\n", rdBuf[0]);
            PalSleep(1000);
            goto retry;

        }
        else
        {        
            PalPrintf("SPI has been saved!!\n");
        }
        
    }

    return 0;
}

MMP_UINT16
xCpuIO_ReadRegister(
    MMP_UINT16 addr)
{
    MMP_UINT16 value;
    MMP_UINT32 error;
    MMP_UINT32 wrLen;
    MMP_UINT32 rdLen;


    addr /= 2;

retry:
    if(gNonAck)
    {
        wrLen = 3;
        rdLen = 2;

        wrBuf[0] = 1;
        wrBuf[1] = (MMP_UINT8)(addr & 0x00FF);
        wrBuf[2] = (MMP_UINT8)((addr & 0xFF00) >> 8);

        SpiSetCs(0);
        error = SpiRead(wrLen, &wrBuf, rdLen, &rdBuf);
        SpiSetCs(1);

        if (error != 0)
        {
            PalPrintf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
            value = 0;
            PalPrintf("\nAck Error : 0x%x\n", rdBuf[0]);
        }
        else
        {
            value = (MMP_UINT16)((rdBuf[1] << 8) + rdBuf[0]);
        }
    }
    else
    {
        wrLen = 3;
        rdLen = 3;
    
        wrBuf[0] = 1;
        wrBuf[1] = (MMP_UINT8)(addr & 0x00FF);
        wrBuf[2] = (MMP_UINT8)((addr & 0xFF00) >> 8);
    
        SpiSetCs(0);
        error = SpiRead(wrLen, &wrBuf, rdLen, &rdBuf);
        SpiSetCs(1);
    
        if ((rdBuf[0] != 0xFF) || (error != 0))
        {
            PalPrintf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
            value = 0;
            PalPrintf("\nAck Error : 0x%x\n", rdBuf[0]);
            xCpuIO_SaveSpi();
            
            goto retry;
        }
        else
        {
            value = (MMP_UINT16)((rdBuf[2] << 8) + rdBuf[1]);
        }
    }


    return value;
}

void
xCpuIO_WriteRegister(
    MMP_UINT16 addr,
    MMP_UINT16 data)
{
    MMP_UINT32 wrLen;
    MMP_UINT32 rdLen;
    MMP_UINT32 error;

    addr /= 2;

retry:
    if(gNonAck)
    {
        wrLen = 3;
        rdLen = 2;

        wrBuf[0] = 0;
        wrBuf[1] = (MMP_UINT8)(addr & 0x00FF);
        wrBuf[2] = (MMP_UINT8)((addr & 0xFF00) >> 8);
        rdBuf[0] = (MMP_UINT8)(data & 0x00FF);
        rdBuf[1] = (MMP_UINT8)((data & 0xFF00) >> 8);
    
        error = SpiWrite(wrLen, &wrBuf, rdLen, &rdBuf);
      
        if ((addr == 0x08) && (data == 0x1000))
            return;
      
        if (error != 0)
        {
            PalPrintf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
            PalPrintf("\nAck Error : 0x%x\n", rdBuf[0]);
        }
    }
    else
    {
        wrLen = 5;
        rdLen = 1;
    
        wrBuf[0] = 0;
        wrBuf[1] = (MMP_UINT8)(addr & 0x00FF);
        wrBuf[2] = (MMP_UINT8)((addr & 0xFF00) >> 8);
        wrBuf[3] = (MMP_UINT8)(data & 0x00FF);
        wrBuf[4] = (MMP_UINT8)((data & 0xFF00) >> 8);
    
        SpiSetCs(0);
        error = SpiRead(wrLen, &wrBuf, rdLen, &rdBuf);
        SpiSetCs(1);
      
        if ((addr == 0x08) && (data == 0x1000))
            return;
      
    
        if ((rdBuf[0] != 0xFF) || (error != 0))
        {
            PalPrintf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
            PalPrintf("\nAck Error : 0x%x\n", rdBuf[0]);
            xCpuIO_SaveSpi();
            goto retry;
        }
    }
}

void
xCpuIO_ReadMemory(
    MMP_UINT32 destAddress,
    MMP_UINT32 srcAddress,
    MMP_UINT32 sizeInByte)
{
    MMP_UINT section = srcAddress / SECTION_SIZE;
    MMP_UINT preSize = 0;
    MMP_UINT data;
    MMP_UINT8* pdest = (MMP_UINT8*)destAddress;

    MMP_UINT32 wrLen;
    MMP_UINT32 rdLen;

    MMP_UINT32 i;
    MMP_UINT32 error;
  	MMP_UINT32 oddFlag = 0;

    if(gNonAck)
    {
        //if(sizeInByte <= 2)
        if(0)
		{

			data = (srcAddress / SECTION_SIZE);
			xCpuIO_WriteRegister(MEM_ADDRESS_HI, (MMP_UINT16)data);
			
			srcAddress >>= 1;
			wrLen = 3;
			rdLen = sizeInByte;

			wrBuf[0] = 1;
			wrBuf[1] = (MMP_UINT8)(srcAddress & 0x00FF);
			wrBuf[2] = (MMP_UINT8)((srcAddress & 0xFF00) >> 8);
			wrBuf[2] |= 0x80;

			error = SpiRead(wrLen, &wrBuf, rdLen, &rdBuf);

			if (error != 0)
			{
				PalPrintf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
				PalPrintf("\nAck Error : 0x%x\n", rdBuf[0]);
			}

			for (i = 0; i < sizeInByte; i++)
			{
				*(pdest + i) = rdBuf[i];
			}

		}
		else
		{    
		    
            while (sizeInByte)
            {
                preSize = (sizeInByte > MAX_PREREAD_COUNT)
                        ? MAX_PREREAD_COUNT
                        : sizeInByte;
                            
                if ( preSize&1)
    			{
                    preSize +=1;
    				oddFlag = 1;
    			}

                data = srcAddress % SECTION_SIZE;
				xCpuIO_WriteRegister(PREREAD_ADDRESS_LO, (MMP_UINT16)data);

				data = srcAddress / SECTION_SIZE;
				xCpuIO_WriteRegister(PREREAD_ADDRESS_HI, (MMP_UINT16)data);

				data = (preSize / 2) - 1;
				xCpuIO_WriteRegister(PREREAD_LENGTH, (MMP_UINT16)data);
    		
				data = 0x8007;
				xCpuIO_WriteRegister(PREREAD_FIRE, (MMP_UINT16)data);

                wrLen = 3;
                rdLen = preSize;


                wrBuf[0] = 1;
                wrBuf[1] = (MMP_UINT8)(0 & 0x00FF);
                wrBuf[2] = (MMP_UINT8)((0 & 0xFF00) >> 8);
                wrBuf[2] |= 0x80;
            
                error = SpiRead(wrLen, &wrBuf, rdLen, &rdBuf);
    
                if (error != 0)
                {
                    PalPrintf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
                    PalPrintf("\nAck Error : 0x%x\n", rdBuf[0]);
                }
                
                if (oddFlag)
    			{
                    preSize -=1;
    				oddFlag = 0;
    			}

                for (i = 0; i < preSize; i++)
                {
                    *(pdest + i) = rdBuf[i];
                }

                sizeInByte -= preSize;
                srcAddress += preSize;
                pdest += preSize;
            }
        }
    }
    else
    {
        while (sizeInByte)
        {
retry:
            preSize = (sizeInByte > MAX_PREREAD_COUNT)
                    ? MAX_PREREAD_COUNT
                    : sizeInByte;
    
            data = srcAddress % SECTION_SIZE;
			xCpuIO_WriteRegister(PREREAD_ADDRESS_LO, (MMP_UINT16)data);

			data = srcAddress / SECTION_SIZE;
			xCpuIO_WriteRegister(PREREAD_ADDRESS_HI, (MMP_UINT16)data);

			data = (preSize / 2) - 1;
			xCpuIO_WriteRegister(PREREAD_LENGTH, (MMP_UINT16)data);
    
			data = 0x8007;
			xCpuIO_WriteRegister(PREREAD_FIRE, (MMP_UINT16)data);
    		
    
            wrLen = 3;
            rdLen = preSize/2*3;
    
    
            wrBuf[0] = 1;
            wrBuf[1] = (MMP_UINT8)(0 & 0x00FF);
            wrBuf[2] = (MMP_UINT8)((0 & 0xFF00) >> 8);
            wrBuf[2] |= 0x80;
        
            SpiSetCs(0);
            error = SpiRead(wrLen, &wrBuf, rdLen, &rdBuf);
            SpiSetCs(1);
    
            //printf("%s: %d\n", __FUNCTION__, __LINE__);
    
            if ((rdBuf[0] != 0xFF) || (error != 0))
            {
                PalPrintf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
                PalPrintf("\nAck Error : 0x%x\n", rdBuf[0]);
                xCpuIO_SaveSpi();
                goto retry;
            }
    
            for (i = 0; i < preSize/2; i++)
            {
                if (rdBuf[i*3] != 0xFF)
                {
                    goto retry;
                }
                *(pdest + i*2)     = rdBuf[i*3 + 1];
                *(pdest + i*2 + 1) = rdBuf[i*3 + 2];
            }
    
            sizeInByte -= preSize;
            srcAddress += preSize;
            pdest += preSize;
        }
    }
}


void
xCpuIO_SecWriteMemory(
    MMP_UINT16 SecNum,
    MMP_UINT16 SecOffset,
    MMP_UINT8 *pDataBuffer,
    MMP_UINT32 DataLen)
{
    
    MMP_UINT32 ulRemainLen;
    MMP_UINT16 usSecOffset;
    MMP_UINT16 usTmpSecOffset;
    MMP_UINT32 usDataLen;
    MMP_UINT32 wrLen;
    MMP_UINT32 rdLen;
    MMP_UINT32 wrDataLen;
    MMP_UINT32 i;
    MMP_UINT8 *ptr;
    MMP_UINT32 error;

    if (DataLen > 0x10000)
    {
        PalPrintf("Top Half: Data length is too large\n");
        return;
    }
    
    if(gNonAck)
    {
        xCpuIO_WriteRegister(0x206, SecNum);
    
        ptr = pDataBuffer;
        
        ulRemainLen = DataLen;
        usSecOffset = SecOffset;
        usDataLen = (MMP_UINT16)((ulRemainLen > (MMP_UINT32)(0xFF00 - usSecOffset))?
                    (MMP_UINT32)(0xFF00 - usSecOffset) : ulRemainLen);
        
        if(usDataLen == 0)
        {
                        
            usDataLen = (ulRemainLen) ? ((MMP_UINT16)ulRemainLen) : (0xFF00 - usSecOffset);
            if(usDataLen == 0)
                return;
        }

        wrLen = 3;
        wrDataLen = usDataLen;    

        for (i = 0; i < (usDataLen); i += 2)
        {
            wrDataBuf[i] = *ptr++;
            wrDataBuf[i + 1] = *ptr++;
        }
    
        wrBuf[0] = 0;
        usTmpSecOffset = (usSecOffset / 2);
          
        wrBuf[1] = (MMP_UINT8)(usTmpSecOffset & 0x00FF);
        wrBuf[2] = (MMP_UINT8)((usTmpSecOffset & 0xFF00) >> 8);
        wrBuf[2] |= 0x80;
    
        error = SpiWrite(wrLen, &wrBuf, wrDataLen, &wrDataBuf);
        if (error)
        {
            PalPrintf("\n%s:SpiWrite Error! Error: 0x%08x\n", __FUNCTION__, error);
        }

    
        ulRemainLen -= (MMP_UINT32)usDataLen;

        if (ulRemainLen)
        {
            usDataLen = (MMP_UINT16)ulRemainLen;    
            usSecOffset = 0xFF00;
            
            wrLen = 3;
            wrDataLen = usDataLen;

            if (usDataLen > 0x100)
            {
                PalPrintf("Buttom Half: Data Length is too large\n");
            }

            xCpuIO_WriteRegister(0x206, SecNum);

            for (i = 0; i < (usDataLen); i += 2)
            {
                wrDataBuf[i] = *ptr++;
                wrDataBuf[i + 1] = *ptr++;
            }

            wrBuf[0] = 0;
            usTmpSecOffset = (usSecOffset / 2);
           
            wrBuf[1] = (MMP_UINT8)(usTmpSecOffset & 0x00FF);
            wrBuf[2] = (MMP_UINT8)((usTmpSecOffset & 0xFF00) >> 8);
            wrBuf[2] |= 0x80;

            error = SpiWrite(wrLen, &wrBuf, wrDataLen, &wrDataBuf);
            if (error)
            {
                PalPrintf("\n%s:SpiWrite Error! Error: 0x%08x\n", __FUNCTION__, error);
            }
        }
    }
    else
    {
    
        if (DataLen < 160)
        {
            xCpuIO_WriteRegister(0x206, SecNum);
    
            ptr = pDataBuffer;
            
            ulRemainLen = DataLen;
            usSecOffset = SecOffset;
            
retry:
            usDataLen = (MMP_UINT16)DataLen;
            
            wrLen = 3 + (usDataLen/2*3 - 1);
            rdLen = 1;        
            
            wrBuf[0] = 0;
    
            usTmpSecOffset = (usSecOffset / 2);
            
    
            wrBuf[1] = (MMP_UINT8)(usTmpSecOffset & 0x00FF);
            wrBuf[2] = (MMP_UINT8)((usTmpSecOffset & 0xFF00) >> 8);
            wrBuf[2] |= 0x80;
    
            for (i = 0; i < (usDataLen/2); i++)
            {
                wrBuf[3 + i*3]     = *ptr++;
                wrBuf[3 + i*3 + 1] = *ptr++;
                wrBuf[3 + i*3 + 2] = 0;
            }
            
            error = SpiRead(wrLen, &wrBuf, rdLen, &rdBuf);
            if (error)
            {
                PalPrintf("\n%s:SpiWrite Error! Error: 0x%08x\n", __FUNCTION__, error);
            }
    
            if (rdBuf[0] != 0xFF)
            {
                PalPrintf("%s: ACK Error\n", __FUNCTION__);
                PalPrintf("\nAck Error : 0x%x\n", rdBuf[0]);
                xCpuIO_SaveSpi();
                goto retry;
            }
        }
        else
        {
    
            xCpuIO_WriteRegister(0x206, SecNum);
            
            ptr = pDataBuffer;
            
            ulRemainLen = DataLen;
            usSecOffset = SecOffset;
            usDataLen = (MMP_UINT16)((ulRemainLen > (MMP_UINT32)(0xFF00 - usSecOffset))?
                        (MMP_UINT32)(0xFF00 - usSecOffset) : ulRemainLen);
    
            if(usDataLen == 0)
            {
                            
                usDataLen = (ulRemainLen) ? ((MMP_UINT16)ulRemainLen) : (0xFF00 - usSecOffset);
                if(usDataLen == 0)
                    return;
            }
            
            wrLen = 3;
            wrDataLen = usDataLen/2*3;    
    
            for (i = 0; i < (usDataLen/2); i++)
            {
                wrDataBuf[i*3] = *ptr++;
                wrDataBuf[i*3 + 1] = *ptr++;
                wrDataBuf[i*3 + 2] = 0;
            }
            
            wrBuf[0] = 0;
            usTmpSecOffset = (usSecOffset / 2);
          
    
            wrBuf[1] = (MMP_UINT8)(usTmpSecOffset & 0x00FF);
            wrBuf[2] = (MMP_UINT8)((usTmpSecOffset & 0xFF00) >> 8);
            wrBuf[2] |= 0x80;
            
            error = SpiWrite(wrLen, &wrBuf, wrDataLen, &wrDataBuf);
            if (error)
            {
                PalPrintf("\n%s:SpiWrite Error! Error: 0x%08x\n", __FUNCTION__, error);
            }
    
            
            ulRemainLen -= (MMP_UINT32)usDataLen;
    
            if (ulRemainLen)
            {
                usDataLen = (MMP_UINT16)ulRemainLen;    
                usSecOffset = 0xFF00;
                
                wrLen = 3;
                wrDataLen = usDataLen/2*3;
    
                if (usDataLen > 0x100)
                {
                    PalPrintf("Buttom Half: Data Length is too large\n");
                }
    
                xCpuIO_WriteRegister(0x206, SecNum);
    
                for (i = 0; i < (usDataLen/2); i++)
                {
                    wrDataBuf[i*3] = *ptr++;
                    wrDataBuf[i*3 + 1] = *ptr++;
                    wrDataBuf[i*3 + 2] = 0;
                }
    
                wrBuf[0] = 0;
                usTmpSecOffset = (usSecOffset / 2);
                 
    
                wrBuf[1] = (MMP_UINT8)(usTmpSecOffset & 0x00FF);
                wrBuf[2] = (MMP_UINT8)((usTmpSecOffset & 0xFF00) >> 8);
                wrBuf[2] |= 0x80;
    
                error = SpiWrite(wrLen, &wrBuf, wrDataLen, &wrDataBuf);
                if (error)
                {
                    PalPrintf("\n%s:SpiWrite Error! Error: 0x%08x\n", __FUNCTION__, error);
                }
            }
        }
    }
}

void
xCpuIO_WriteMemory(
    MMP_UINT32 destAddress,
    MMP_UINT32 srcAddress,
    MMP_UINT32 sizeInByte)
{
#if (1)
    MMP_UINT8 *ptr;


    MMP_UINT16 usSecNum;
    MMP_UINT16 usSecOffset;
    MMP_UINT32 ulWriteLen;
    MMP_UINT32 ulRemainLen;

    //if ((sizeInByte % 2) == 1)
    //{
    //    PalPrintf("write mem length is odd\n");
    //    return;
    //}
    sizeInByte = ((sizeInByte + 3) >> 2) << 2;

    usSecNum    = (MMP_UINT16)(destAddress / SECTION_SIZE);
    usSecOffset = (MMP_UINT16)(destAddress % SECTION_SIZE);
    ulRemainLen = sizeInByte;
    ulWriteLen  = (ulRemainLen > (SECTION_SIZE - (MMP_UINT32)usSecOffset))? (SECTION_SIZE - (MMP_UINT32)usSecOffset) : ulRemainLen;

    ptr = (MMP_UINT8 *)srcAddress;

    while(ulRemainLen > 0)
    {
        xCpuIO_SecWriteMemory(usSecNum, usSecOffset, ptr, ulWriteLen);

        usSecNum++;
        usSecOffset = 0;
        ptr += ulWriteLen;
        ulRemainLen -= ulWriteLen;
        ulWriteLen = ((ulRemainLen > 0x10000)? 0x10000:ulRemainLen);
    }


#else
    MMP_UINT32 wrLen;
    MMP_UINT32 wrDataLen;
    MMP_UINT32 rdLen;
    WriteControlByteBuffer wrBuf;
    WriteDataByteBuffer wrDataBuf;
    ReadDataByteBuffer rdBuf;
    MMP_UINT32 size;
    MMP_UINT32 addr;
    MMP_UINT8 *ptr;
    MMP_UINT32 i;
    MMP_UINT32 j;
    MMP_UINT32 seq = 0;
    MMP_UINT32 offset;

    MMP_UINT32 n;
    MMP_UINT32 r;

    MMP_UINT16 section = (MMP_UINT16)(destAddress / SECTION_SIZE);
    xCpuIO_WriteRegister(0x206, section);

    offset = destAddress % SECTION_SIZE;

    wrLen = 5;
    rdLen = 1;

    wrBuf[0] = 0;
    wrBuf[1] = (MMP_UINT8)(offset & 0x00FF);
    wrBuf[2] = (MMP_UINT8)((offset & 0xFF00) >> 8);
    wrBuf[2] |= 0x80;
    wrBuf[3] = *(MMP_UINT8 *)srcAddress;
    wrBuf[4] = *(MMP_UINT8 *)(srcAddress + 1);
    
    SpiSetCs(0);
    
    Spi(wrLen, &wrBuf, rdLen, &rdBuf);
    if (rdBuf[0] != 0xFF)
    {
        PalPrintf("\n%s:Ack Error!\n", __FUNCTION__);

#if (1)
        while(1)
        {
            PalSleep(1000);
        }
#endif
        return;
    }
    
    size = sizeInByte - 2;
    addr = srcAddress + 2;
    
    ptr = (MMP_UINT8 *)addr;
    n = size / 40002;
    r = size % 40002;
    wrLen = 3;
    wrDataLen = 20000*3;

    for (i = 0; i < n; i++)
    {

        wrBuf[0] = *ptr++;
        wrBuf[1] = *ptr++;
        wrBuf[2] = 0;
        
        for (j = 0; j < 20000; j++)
        {
            wrDataBuf[j*3]      = *ptr++;
            wrDataBuf[j*3 + 1]  = *ptr++;
            wrDataBuf[j*3 + 2]  = 0;
        }

        SpiWrite(wrLen, &wrBuf, wrDataLen, &wrDataBuf);
    }

    if (r > 0)
    {
        wrLen = 3;
        wrDataLen = ((r-2)/2*3);
        
        wrBuf[0] = *ptr++;
        wrBuf[1] = *ptr++;
        wrBuf[2] = 0;

        for (j = 0; j < (r-2)/2; j++)
        {
            wrDataBuf[j*3]      = *ptr++;
            wrDataBuf[j*3 + 1]  = *ptr++;
            //wrBuf[j*3]      = seq++;
            //wrBuf[j*3 + 1]  = seq++;
            wrDataBuf[j*3 + 2]  = 0;
        }

        SpiWrite(wrLen, &wrBuf, wrDataLen, &wrDataBuf);
    }

    SpiSetCs(1);
#endif
}

void
xCpuIO_ReadMemoryUInt16(
    MMP_UINT32 destAddress,
    MMP_UINT32 srcAddress,
    MMP_UINT32 sizeInByte)
{
    xCpuIO_ReadMemory(destAddress, srcAddress, sizeInByte);

#if (ENDIAN == 0)
    if (!_IsBigEndian())
#endif
#if (ENDIAN == 0 || ENDIAN == 1)
    {
        MMP_UINT    i;
        MMP_UINT16* ptr;

        ptr = (MMP_UINT16*)destAddress;
        for (i = 0; i < sizeInByte / sizeof(MMP_UINT16); ++i)
        {
            *(ptr + i) = _UINT16BE_To_UINT16LE(*(ptr + i));
        }
    }
#endif
}

void
xCpuIO_WriteMemoryUInt16(
    MMP_UINT32 destAddress,
    MMP_UINT32 srcAddress,
    MMP_UINT32 sizeInByte)
{
#if (ENDIAN == 0 || ENDIAN == 1)
    MMP_UINT    i;
    MMP_UINT16* ptr;
#endif
#if (ENDIAN == 0)
    MMP_BOOL    isLittleEndian = !_IsBigEndian();

    if (isLittleEndian)
#endif
#if (ENDIAN == 0 || ENDIAN == 1)
    {
        ptr = (MMP_UINT16*)srcAddress;
        for (i = 0; i < sizeInByte / sizeof(MMP_UINT16); ++i)
        {
            *(ptr + i) = _UINT16LE_To_UINT16BE(*(ptr + i));
        }
    }
#endif

    xCpuIO_WriteMemory(destAddress, srcAddress, sizeInByte);

#if (ENDIAN == 0)
    if (isLittleEndian)
#endif
#if (ENDIAN == 0 || ENDIAN == 1)
    {
        ptr = (MMP_UINT16*)srcAddress;
        for (i = 0; i < sizeInByte / sizeof(MMP_UINT16); ++i)
        {
            *(ptr + i) = _UINT16BE_To_UINT16LE(*(ptr + i));
        }
    }
#endif
}

void
xCpuIO_ReadMemoryUInt32(
    MMP_UINT32 destAddress,
    MMP_UINT32 srcAddress,
    MMP_UINT32 sizeInByte)
{
    xCpuIO_ReadMemory(destAddress, srcAddress, sizeInByte);

#if (ENDIAN == 0)
    if (!_IsBigEndian())
#endif
#if (ENDIAN == 0 || ENDIAN == 1)
    {
        MMP_UINT    i;
        MMP_UINT32* ptr;

        ptr = (MMP_UINT32*)destAddress;
        for (i = 0; i < sizeInByte / sizeof(MMP_UINT32); ++i)
        {
            *(ptr + i) = _UINT32BE_To_UINT32LE(*(ptr + i));
        }
    }
#endif
}

void
xCpuIO_WriteMemoryUInt32(
    MMP_UINT32 destAddress,
    MMP_UINT32 srcAddress,
    MMP_UINT32 sizeInByte)
{
#if (ENDIAN == 0 || ENDIAN == 1)
    MMP_UINT    i;
    MMP_UINT32* ptr;
#endif
#if (ENDIAN == 0)
    MMP_BOOL    isLittleEndian = !_IsBigEndian();

    if (isLittleEndian)
#endif
#if (ENDIAN == 0 || ENDIAN == 1)
    {
        ptr = (MMP_UINT32*)srcAddress;
        for (i = 0; i < sizeInByte / sizeof(MMP_UINT32); ++i)
        {
            *(ptr + i) = _UINT32LE_To_UINT32BE(*(ptr + i));
        }
    }
#endif

    xCpuIO_WriteMemory(
        (MMP_UINT32)destAddress,
        (MMP_UINT32)srcAddress,
        sizeInByte);

#if (ENDIAN == 0)
    if (isLittleEndian)
#endif
#if (ENDIAN == 0 || ENDIAN == 1)
    {
        ptr = (MMP_UINT32*)srcAddress;
        for (i = 0; i < sizeInByte / sizeof(MMP_UINT32); ++i)
        {
            *(ptr + i) = _UINT32BE_To_UINT32LE(*(ptr + i));
        }
    }
#endif
}

//=============================================================================
//                              Private Function Definition
//=============================================================================

#if (ENDIAN == 0)
MMP_BOOL
_IsBigEndian(
    void)
{
    union
    {
        MMP_UINT32 k;
        MMP_UINT8  c[4];
    } u;

    u.k = 0xFF000000;

    if (0xFF == u.c[0])
    {
        return MMP_TRUE;
    }
    return MMP_FALSE;
}
#endif

#endif // #if (!defined(XCPUIO_BUS_TYPE)) || (XCPUIO_BUS_TYPE > 0)
