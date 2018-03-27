/////////////////////////////////////////////////////////////////
//                      Include File
/////////////////////////////////////////////////////////////////
#include <stdio.h>

#include <string.h>

#include "mpegaudio.h"

#if defined(ENABLE_CODECS_PLUGIN)
# include "plugin.h"
#endif

#ifdef MP2_ENCODE_PERFORMANCE_TEST_BY_TICK
#include "ticktimer.h"
#endif
#include "resample.h"

#define BIT_RATE 192000
/////////////////////////////////////////////////////////////////
//                      Local Variable
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
//                      Global Variable
/////////////////////////////////////////////////////////////////

MpegAudioContext gMpegAudioContext;

char gFrame[MPA_MAX_CODED_FRAME_SIZE];
char streamBuf[READBUF_SIZE]; // wave input buffer
char gOutBuf[OUTBUF_SIZE]; // encode mp3 buffer 
unsigned char paramBuf[MPA_FRAME_SIZE*2*2+2];  // temp buffer for wav data
int gnFrames;

char gOutFrameBuf[OUTFRAME_SIZE]; //prepare output mp3 buffer 0~3: time stamp, 4~7: data size, 8~end: data

unsigned int gTimeStamp;  // output data ,time stamp
unsigned int gDataSize; // ouput data,encoded size
unsigned int gAppendFrame; // output data,encoded frames
// microsecond 10(-6)
unsigned int TimeStamp48000[10]={24000,48000,72000,96000,120000,144000,168000,192000,216000,240000};
unsigned int TimeStamp44100[10]={26122,52245,78637,104490,130612,156734,182857,208980,235102,261224};
unsigned int TimeStamp32000[10]={36000,72000,108000,144000,180000,216000,252000,288000,324000,360000};
/////////////////////////////////////////////////////////////////
//                      Global Function
/////////////////////////////////////////////////////////////////
#define getUsedLen(rdptr, wrptr, len) (((wrptr) >= (rdptr)) ? ((wrptr) - (rdptr)) : ((len) - ((rdptr) - (wrptr))))

static AUDIO_ENCODE_PARAM  gMp2AudioEncode;
static AVResampleContext avResampleContext;
#ifdef MP2_ENCODE_PERFORMANCE_TEST_BY_TICK
static  int  gnNewTicks,gnTotalTicks;
static  int  gnTotalTime;
#endif

#ifdef ITE_RISC
__inline short CLIPTOSHORT(int x)
{
    int sign;

    /* clip to [-32768, 32767] */
    sign = x >> 31;
    if (sign != (x >> 15))
        x = sign ^ ((1 << 15) - 1);

    return (short)x;
}

__inline short MixerResult(int a, int b)
{
    #define _INT16_MIN (-32700)
    #define _INT16_MAX (32700)
    int c = 0;
    if (a < 0 && b < 0)
    {
        return (short) ((a + b) - ((a*b)/_INT16_MIN));
    }
    else if (a > 0 && b > 0)
    {
        return (short) ((a+b) - ((a*b) /_INT16_MAX));
    }
    else
    {
        return (short) (a+b);
    }
}

static __inline int MULSHIFT32_ADD(int a, int b, int c, int d)
{
    int result;
    asm volatile ("l.mac %0, %1" : : "r"(a), "r"(b));
    asm volatile ("l.mac %0, %1" : : "r"(c), "r"(d));
    asm volatile ("l.macrc %0, 32" : "=r" (result));
    return result;
}

static __inline int MULSHIFT32_SUB(int a, int b, int c, int d)
{
    int result;
    asm volatile ("l.mac %0, %1" : : "r"(a), "r"(b));
    asm volatile ("l.msb %0, %1" : : "r"(c), "r"(d));
    asm volatile ("l.macrc %0, 32" : "=r"(result));
    return result;
}

static __inline short MULSHIFT_div4(int a, int b)
{
    int result;
    asm volatile ("l.mac %0, %1" : : "r"(a), "r"(b));
    asm volatile ("l.macrc %0, 2" : "=r" (result));
    return (short)result;
}

static __inline short MULSHIFT_div8(int a, int b)
{
    int result;
    asm volatile ("l.mac %0, %1" : : "r"(a), "r"(b));
    asm volatile ("l.macrc %0, 3" : "=r" (result));
    return (short)result;
}

static __inline short MULSHIFT_div16(int a, int b)
{
    int result;
    asm volatile ("l.mac %0, %1" : : "r"(a), "r"(b));
    asm volatile ("l.macrc %0, 4" : "=r" (result));
    return (short)result;
}

static __inline short MULSHIFT_div32(int a, int b)
{
    int result;
    asm volatile ("l.mac %0, %1" : : "r"(a), "r"(b));
    asm volatile ("l.macrc %0, 5" : "=r" (result));
    return (short)result;
}


void ControlVolume(short* pBuffer, int nSamples)
{
    int i;
    short *buf = (short *)pBuffer;          
    short nTemp;
    unsigned int value = getAudioVolume();
    switch (value) 
    {
        case 0:     // mute
             for(i = 0; i < nSamples; i++){
                 buf[i] = 0;
             }
             break;

        case 1:     // 1/32
             for(i = 0; i < nSamples; i++) {
                 buf[i] = buf[i]>>5;
             }
             break;

        case 2:     // 2/32
             for(i = 0; i < nSamples; i++) {
                 buf[i] = buf[i]>>4;
             }
             break;

        case 3:     // 6/32
             for(i = 0; i < nSamples; i++) {
                 buf[i] = MULSHIFT_div32((int)buf[i],6);
             }
             break;

        case 4:     // 16/32
             for(i = 0; i < nSamples; i++) {
                 buf[i] = buf[i]>>1;
             }
             break;

        case 5:     // default
             break;

        case 6:     // 17/16
             for(i = 0; i < nSamples; i++) {
                 buf[i] = CLIPTOSHORT(MULSHIFT_div16((int)buf[i],17));
             }
             break;

        case 7:     // 9/8
             for(i = 0; i < nSamples; i++) {
                 buf[i] = CLIPTOSHORT(MULSHIFT_div8((int)buf[i],9));
             }
             break;

        case 8:     // 37/32
             for(i = 0; i < nSamples; i++) {
                 buf[i] = CLIPTOSHORT(MULSHIFT_div32((int)buf[i],37));
             }
             break;

        case 9:     // 39/32
             for(i = 0; i < nSamples; i++) {
                 buf[i] = CLIPTOSHORT(MULSHIFT_div32((int)buf[i],39));
             }
             break;

        case 10:    // 10/8
             for(i = 0; i < nSamples; i++) {
                 buf[i] = CLIPTOSHORT(MULSHIFT_div8((int)buf[i],10));
             }
             break;

        case 11:    // 41/32
             for(i = 0; i < nSamples; i++) {
                 buf[i] = CLIPTOSHORT(MULSHIFT_div32((int)buf[i],41));
             }
             break;

        case 12:    // 11/8
             for(i = 0; i < nSamples; i++) {
                 buf[i] = CLIPTOSHORT(MULSHIFT_div8((int)buf[i],11));
             }
             break;

        case 13:   // 45/32
             for(i = 0; i < nSamples; i++) {
                 buf[i] = CLIPTOSHORT(MULSHIFT_div32((int)buf[i],45));
             }
             break;

        case 14:    // 47/32
             for(i = 0; i < nSamples; i++) {
                 buf[i] = CLIPTOSHORT(MULSHIFT_div32((int)buf[i],47));
             }
             break;

        case 15:   // 49/32
             for(i = 0; i < nSamples; i++) {
                 buf[i] = CLIPTOSHORT(MULSHIFT_div32((int)buf[i],49));
             }
             break;

        case 16:   // 13/8
             for(i = 0; i < nSamples; i++) {
                 buf[i] = CLIPTOSHORT(MULSHIFT_div8((int)buf[i],13));
             }
             break;

        case 17:    // 55/32
             for(i = 0; i < nSamples; i++) {
                 buf[i] = CLIPTOSHORT(MULSHIFT_div32((int)buf[i],55));
             }
             break;

        case 18:    // 29/16
             for(i = 0; i < nSamples; i++) {
                 buf[i] = CLIPTOSHORT(MULSHIFT_div16((int)buf[i],29));
             }
             break;

        case 19:   // 31/16
             for(i = 0; i < nSamples; i++) {
                 buf[i] = CLIPTOSHORT(MULSHIFT_div16((int)buf[i],31));
             }
             break;

        case 20:   // 2
             for(i = 0; i <nSamples; i++) {
                buf[i] = CLIPTOSHORT((buf[i] << 1));
             }
             break;

        //case 21:  // 4
        //    for(i = 0; i <nSamples; i++) {
        //       buf[i] = CLIPTOSHORT((buf[i] << 2));
        //    }
        //    break;

        default:
             break;
    }
}



void ChangeEndian(char* ptr,int size);
static int FillReadBuffer();
static int FillReadBufferOtherRisc();


static void FillWriteBuffer(int nMp3Bytes);
int getResampleEncodeBuffer(ResampleAudioContext *audioContext,short* writeptr,int nSize);
int getResampleAvailableLength(ResampleAudioContext *audioContext);
int resample(AVResampleContext *avResampleContext,ResampleAudioContext *audioContext, short *writeptr);

void resampleInit(ResampleAudioContext *audioContext){
    int nSize;

    //memset(audioContext,0,sizeof(ResampleAudioContext));
    if (gMp2AudioEncode.nSampleRate == 32000){
        nSize = gMp2AudioEncode.nChannels*2*768;//4
    }
    else if (gMp2AudioEncode.nSampleRate == 44100){
        nSize = gMp2AudioEncode.nChannels*2*1060;
        audioContext->nUseTempBuffer = 1;
    }
    audioContext->nInSampleRate = gMp2AudioEncode.nSampleRate;
    audioContext->nOutSampleRate = gMp2AudioEncode.nOutSampleRate;
    audioContext->nInSize = nSize;
    audioContext->nInChannels = gMp2AudioEncode.nChannels;
    audioContext->nTempBufferLength = TEMP_BUFFER_SIZE;
    audioContext->nTempBufferRdPtr = 0;
    audioContext->nTempBufferWrPtr = 0;
    printf("[Mp2 Enc]resampleInit %d %d %d %d %d\n",audioContext->nInSampleRate,audioContext->nOutSampleRate,audioContext->nInSize,audioContext->nInChannels,audioContext->nUseTempBuffer);
}

int getResampleEncodeBuffer(ResampleAudioContext *audioContext,short* writeptr,int nSize){
    // copy to encode buffer
    if (audioContext->nTempBufferRdPtr+nSize < audioContext->nTempBufferLength){
        memcpy(writeptr,&audioContext->nTempBuffer[audioContext->nTempBufferRdPtr],nSize*2/* short*/);
        audioContext->nTempBufferRdPtr += nSize;
    } else if (audioContext->nTempBufferRdPtr+nSize == audioContext->nTempBufferLength) {
        memcpy(writeptr,&audioContext->nTempBuffer[audioContext->nTempBufferRdPtr],(audioContext->nTempBufferLength-audioContext->nTempBufferRdPtr)*2/* short*/);
        // reset rd ptr
        audioContext->nTempBufferRdPtr = 0;
    } else {
        memcpy(writeptr,&audioContext->nTempBuffer[audioContext->nTempBufferRdPtr],(audioContext->nTempBufferLength-audioContext->nTempBufferRdPtr)*2/* short*/);
        memcpy(&writeptr[audioContext->nTempBufferLength-audioContext->nTempBufferRdPtr],audioContext->nTempBuffer,(nSize-(audioContext->nTempBufferLength-audioContext->nTempBufferRdPtr))*2/* short*/);
        audioContext->nTempBufferRdPtr = nSize-(audioContext->nTempBufferLength-audioContext->nTempBufferRdPtr);
        // reset rd wr ptr
        memcpy(audioContext->nTempBuffer,&audioContext->nTempBuffer[audioContext->nTempBufferRdPtr],(audioContext->nTempBufferWrPtr-audioContext->nTempBufferRdPtr)*2/* short*/);
        audioContext->nTempBufferWrPtr = audioContext->nTempBufferWrPtr - (audioContext->nTempBufferWrPtr-audioContext->nTempBufferRdPtr);
        audioContext->nTempBufferRdPtr = 0;
    }
}

int getResampleAvailableLength(ResampleAudioContext *audioContext){
    if (audioContext->nTempBufferWrPtr>=audioContext->nTempBufferRdPtr){
        return (audioContext->nTempBufferWrPtr-audioContext->nTempBufferRdPtr);
    } else {
        //printf("getResampleAvailableLength wr %d > rd %d ptr \n",audioContext->nTempBufferWrPtr,audioContext->nTempBufferRdPtr);
        return audioContext->nTempBufferLength-(audioContext->nTempBufferRdPtr-audioContext->nTempBufferWrPtr);
    }
}

int resample(AVResampleContext *avResampleContext,ResampleAudioContext *audioContext, short *writeptr)
{
    int nTemp,i,j;
    int consumed,lenout,nReSampleOutputSize;
    float ratio = (float)audioContext->nOutSampleRate/(float)audioContext->nInSampleRate;

    for (i=0;i<audioContext->nInChannels;i++) {
        int is_last = i + 1 == audioContext->nInChannels;

        lenout= (audioContext->nInSize/audioContext->nInChannels) *ratio + 16;  
        nTemp = (audioContext->nInSize/audioContext->nInChannels)/2+audioContext->nKeep[i];
        nReSampleOutputSize = av_resample(avResampleContext, audioContext->reSamplePcmOutput[i], audioContext->reSamplePcmInput[i],
            &consumed, nTemp, lenout, is_last);
        audioContext->nKeep[i] = nTemp - consumed;
        //printf("resample nReSampleOutputSize %d consumed %d   %d %d %d %d %d \n",nReSampleOutputSize,consumed,audioContext->reSamplePcmOutput[i][1],audioContext->reSamplePcmOutput[i][2],audioContext->reSamplePcmOutput[i][3],audioContext->reSamplePcmOutput[i][4],audioContext->reSamplePcmOutput[i][5]);
    } 
    
    nTemp = audioContext->nKeep[0];
    for (i=0;i<nTemp;i++) {
        for (j=0;j<audioContext->nInChannels;j++)
            audioContext->reSamplePcmInput[j][i] = audioContext->reSamplePcmInput[j][consumed + i];
    }
        
    if (audioContext->nUseTempBuffer == 0){
        if (audioContext->nInChannels==1) {
            for (i = 0; i < nReSampleOutputSize; i++) 
            {  // interleave channels
                *writeptr++ = audioContext->reSamplePcmOutput[0][i];
                *writeptr++ = audioContext->reSamplePcmOutput[0][i];
            }
        } else if (audioContext->nInChannels==2) {
            for (i = 0; i < nReSampleOutputSize; i++) 
            {  // interleave channels
                *writeptr++ = audioContext->reSamplePcmOutput[0][i];
                *writeptr++ = audioContext->reSamplePcmOutput[1][i];
            }
        }
    } else {
        // copy to temp buffer
        if (audioContext->nInChannels==1) {
            for (i = 0; i < nReSampleOutputSize; i++) 
            {  // interleave channels
                if (audioContext->nTempBufferWrPtr >= audioContext->nTempBufferLength){
                    audioContext->nTempBufferWrPtr = 0;
                }
                audioContext->nTempBuffer[audioContext->nTempBufferWrPtr++] = audioContext->reSamplePcmOutput[0][i];
                audioContext->nTempBuffer[audioContext->nTempBufferWrPtr++] = audioContext->reSamplePcmOutput[0][i];
            }
            // copy to encode buffer
            getResampleEncodeBuffer(audioContext,writeptr,2*MAX_FRAMESIZE);

        } else if (audioContext->nInChannels==2) {
            if (nReSampleOutputSize<MAX_FRAMESIZE){
                printf("resample use temp buffers nReSampleOutputSize %d < %d \n",nReSampleOutputSize,MAX_FRAMESIZE);
                nReSampleOutputSize = MAX_FRAMESIZE;
            }
            for (i = 0; i < nReSampleOutputSize; i++) 
            {  // interleave channels
                if (audioContext->nTempBufferWrPtr >= audioContext->nTempBufferLength){
                    audioContext->nTempBufferWrPtr = 0;
                }
                audioContext->nTempBuffer[audioContext->nTempBufferWrPtr++] = audioContext->reSamplePcmOutput[0][i];
                audioContext->nTempBuffer[audioContext->nTempBufferWrPtr++] = audioContext->reSamplePcmOutput[1][i];
            }
            // copy to encode buffer
            getResampleEncodeBuffer(audioContext,(short*)writeptr,audioContext->nInChannels*MAX_FRAMESIZE);
        }
    }
    return nReSampleOutputSize*2*sizeof(short);
}

void Get_Encode_Parameter(MpegAudioContext *s)
{
    int mode,sampleRate;

#if 0
    s->nb_channels = getMP2EncCh()+1;

    mode = getMP2EncSampleRate();
    if (mode == 0)
        sampleRate = 48000;
    else if (mode == 1)
        sampleRate = 44100;
    else if (mode == 2)
        sampleRate = 32000;
    
    s->sample_rate = sampleRate;
#else
    s->nb_channels = gMp2AudioEncode.nChannels;
    if (gMp2AudioEncode.nSampleRate != gMp2AudioEncode.nOutSampleRate){
        s->sample_rate = gMp2AudioEncode.nOutSampleRate;
    } else {
        s->sample_rate = gMp2AudioEncode.nSampleRate;
    }

    s->bit_rate = (int)gMp2AudioEncode.nBitrate;
#endif    
//    s->bit_rate = BIT_RATE;    
    //printf("[Mp2 Enc] ch %d sampleRate %d bitrate %d \n",s->nb_channels,s->sample_rate,s->bit_rate);
}
static __inline unsigned int setStreamRdPtr(unsigned int wrPtr) 
{
    MMIO_Write(DrvDecode_RdPtr, wrPtr);
    return 0;
}

static __inline unsigned int setStreamRdPtrOtherRisc(unsigned int wrPtr) 
{
    MMIO_Write(DrvDecode_RdPtr, wrPtr);
    return 0;
}


static __inline unsigned int setStreamWrPtr(unsigned int wrPtr) 
{
    MMIO_Write(DrvDecode_WrPtr, wrPtr);
    return 0;
}
static __inline unsigned int getStreamWrPtr() 
{
    unsigned int wrPtr;
    wrPtr = MMIO_Read(DrvDecode_WrPtr);

    return wrPtr;
}

__inline unsigned int getStreamRdPtr() 
{
    unsigned int rdPtr;
    rdPtr = MMIO_Read(DrvDecode_RdPtr);

#if defined(__OR32__) && !defined(__FREERTOS__)
    if (0xffff == rdPtr) asm volatile("l.trap 15");
#endif
    return rdPtr;
}

static __inline unsigned int getStreamWrPtrOtherRisc() 
{
    unsigned int wrPtr;
    wrPtr = MMIO_Read(DrvDecode_WrPtr);

    return wrPtr;
}

__inline unsigned int getStreamRdPtrOtherRisc() 
{
    unsigned int rdPtr;
    rdPtr = MMIO_Read(DrvDecode_RdPtr);

#if defined(__OR32__) && !defined(__FREERTOS__)
    if (0xffff == rdPtr) asm volatile("l.trap 15");
#endif
    return rdPtr;
}

__inline unsigned int getOutBufRdPtr() 
{
    unsigned int rdPtr;
    rdPtr = MMIO_Read(DrvEncode_RdPtr);
    return rdPtr;
}

__inline unsigned int setOutBufRdPtr(unsigned int wrPtr) 
{
    MMIO_Write(DrvDecode_RdPtr, wrPtr);   
    return 0;
}

__inline unsigned int getOutBufWrPtr() 
{
    unsigned int wrPtr;
    wrPtr = MMIO_Read(DrvEncode_WrPtr);
    return wrPtr;
}

__inline unsigned int setOutBufWrPtr(unsigned int wrPtr) 
{
    MMIO_Write(DrvEncode_WrPtr, wrPtr);   
    return 0;
}

static void occupyEncodeBuffer()
{
#if 0
    unsigned int nWriteProtect=0;
    unsigned int nProtect=0;
    do{
        nWriteProtect = getAudioReadBufferStatus();
        nProtect++;
        PalSleep(1);
    }while (nWriteProtect==1);

    if (nProtect>1)
        printf("[Mp2 Enc] occupyEncodeBuffer %d \n",nProtect);

    occupyAudioReadBuffer();
#endif    
}

static void releaseEncodeBuffer()
{
   // releaseAudioReadBuffer();
}

static void occupyPCMBuffer()
{
#if 0
    unsigned int nWriteProtect=0;
    unsigned int nProtect=0;

    do{
        nWriteProtect = getAudioWriteBufferStatus();
        nProtect++;
        PalSleep(1);
    }while (nWriteProtect==1);

    if (nProtect>1)
        printf("[Mp2 Enc] occupyPCMBuffer nProtect %d \n",nProtect);

    occupyAudioWriteBuffer();
#endif    
}

static void releasePCMBuffer()
{
    // releaseAudioWriteBuffer();
}

static int GetAvaliableReadBufferSize() 
{
    int len = 0;
    unsigned int wavReadIdx,wavWriteIdx;
    wavReadIdx = getStreamRdPtr();
    // Wait Read Buffer avaliable
    wavWriteIdx = getStreamWrPtr();
    len = getUsedLen(wavReadIdx, wavWriteIdx, READBUF_SIZE);

    //printf("[Mp2 Enc] wavReadIdx %d wavWriteIdx %d len %d \n",wavReadIdx,wavWriteIdx,len);
    return len;
}

static int GetAvaliableReadBufferSizeOtherRisc() 
{
    int len = 0;
    unsigned int wavReadIdx,wavWriteIdx;
    wavReadIdx = getStreamRdPtrOtherRisc();
    // Wait Read Buffer avaliable
    wavWriteIdx = getStreamWrPtrOtherRisc();
    len = getUsedLen(wavReadIdx, wavWriteIdx, gMp2AudioEncode.nBufferLength);

    //printf("[Mp2 Enc] wavReadIdx %d wavWriteIdx %d len %d \n",wavReadIdx,wavWriteIdx,len);
    return len;
}

static int GetAvaliableReadBufferSize_I2S() 
{
    int len = 0;
    unsigned int wavReadIdx,wavWriteIdx;
    wavReadIdx = I2S_AD32_GET_RP();
    // Wait Read Buffer avaliable
    wavWriteIdx = I2S_AD32_GET_WP();
    len = getUsedLen(wavReadIdx, wavWriteIdx, gMp2AudioEncode.nBufferLength);

    //printf("[Mp2 Enc] wavReadIdx %d wavWriteIdx %d len %d \n",wavReadIdx,wavWriteIdx,len);
    return len;
}

static int GetAvaliableReadBufferSize_Mixer() 
{
    int len = 0;
    unsigned int mixReadIdx,mixWriteIdx;
    mixReadIdx = getMixerReadPorinter();
    // Wait Read Buffer avaliable
    mixWriteIdx = getMixerWritePorinter();
    len = getUsedLen(mixReadIdx, mixWriteIdx, gMp2AudioEncode.nMixBufferLength);

    //printf("[Mp2 Enc] mixReadIdx %d mixWriteIdx %d len %d \n",mixReadIdx,mixWriteIdx,len);
    return len;
}

/**************************************************************************************
 * Function     : FillReadBuffer
 *
 * Description  : Update the read pointer of WAVE Buffer and return the valid data length
 *                of input buffer.
 *
 * Inputs       : nReadBytes: number of bytes will read
 *
 * Global var   : wavWriteIdx: write pointer of WAVE buffer
 *                wavReadIdx : read pointer of WAVE buffer
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         : The WAVE buffer is circular buffer.
 *
 **************************************************************************************/
static int FillReadBuffer() 
{
    int len = 0;
    unsigned int wavReadIdx,wavWriteIdx;
    int nTemp,bytesLeft;

    // Updates the read buffer and returns the avaliable size
    // of input buffer. Wait a minimun FRAME_SIZE length.        
    nTemp = MPA_FRAME_SIZE*2*gMpegAudioContext.nb_channels;        

    // buffer reserve from Mp2 Encoder
    if (gMp2AudioEncode.nInputBufferType==0)
    {
    do
    {
        bytesLeft = GetAvaliableReadBufferSize();
#ifdef ITE_RISC
        PalSleep(1);
#endif
    }while (bytesLeft<=nTemp && !isSTOP());
#if defined(__FREERTOS__)
            dc_invalidate(); // Flush Data Cache
#endif         

    wavReadIdx = getStreamRdPtr();
        if (nTemp+wavReadIdx<=READBUF_SIZE) {
        memcpy(&paramBuf[0],&streamBuf[wavReadIdx],nTemp);
        } else {
        printf("[Mp2 Enc] memcpy \n");
        memcpy(&paramBuf[0],&streamBuf[wavReadIdx],READBUF_SIZE-wavReadIdx);
        memcpy(&paramBuf[READBUF_SIZE-wavReadIdx],&streamBuf[0],nTemp-(READBUF_SIZE-wavReadIdx));
    }
    //printf("[Mp2 Enc]  wavReadIdx  %d %d\n",wavReadIdx,nRead++);

    occupyPCMBuffer();
    //wavReadIdx = getStreamRdPtr();
    // Update Read Buffer
        if (nTemp > 0) {
        wavReadIdx = wavReadIdx + nTemp;
        if (wavReadIdx >= READBUF_SIZE) {
            wavReadIdx -= READBUF_SIZE;
        }
        setStreamRdPtr(wavReadIdx);
    }
#if defined(__FREERTOS__)
    dc_invalidate(); // Flush Data Cache
#endif

    releasePCMBuffer();
    }

    return len;
}

/**************************************************************************************
 * Function     : FillReadBufferResample
 *
 * Description  : Update the read pointer of WAVE Buffer and return the valid data length
 *                of input buffer.
 *
 * Inputs       : nReadBytes: number of bytes will read
 *
 * Global var   : wavWriteIdx: write pointer of WAVE buffer
 *                wavReadIdx : read pointer of WAVE buffer
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         : The WAVE buffer is circular buffer.
 *
 **************************************************************************************/
static int FillReadBufferResample(ResampleAudioContext *audioContext) 
{
    int len = 0;
    unsigned int wavReadIdx,wavWriteIdx;
    int nTmp;    
    int nTemp,bytesLeft;
    short* readBuffer;
    int i,j;

    // Updates the read buffer and returns the avaliable size
    // of input buffer. Wait a minimun FRAME_SIZE length.        
    //nTemp = MPA_FRAME_SIZE*2*gMpegAudioContext.nb_channels;        
    nTemp = audioContext->nInSize*(gMp2AudioEncode.nSampleSize/16);

    // check resample temp buffers
    if (audioContext->nUseTempBuffer == 1){
        nTmp = getResampleAvailableLength(audioContext);
        //printf("getResampleAvailableLength %d  %d  \n",nTemp,2*MAX_FRAMESIZE);
        if (nTmp>=audioContext->nInChannels*MAX_FRAMESIZE){
            //printf("[Mp2 Enc]getResampleAvailableLength %d > %d  \n",nTmp,audioContext->nInChannels*MAX_FRAMESIZE);
            getResampleEncodeBuffer(audioContext,(short*)paramBuf,audioContext->nInChannels*MAX_FRAMESIZE);
            return 0;
        }
    }

    // buffer reserve from Mp2 Encoder
    if (gMp2AudioEncode.nInputBufferType==0)
    {
        do
        {
            bytesLeft = GetAvaliableReadBufferSize();
#ifdef ITE_RISC
            PalSleep(1);
#endif
        }while (bytesLeft<=nTemp && !isSTOP());
#if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
#endif         

        wavReadIdx = getStreamRdPtr();
        if (nTemp+wavReadIdx<=READBUF_SIZE) {
            //memcpy(&paramBuf[0],&streamBuf[wavReadIdx],nTemp);
            // read data
            readBuffer = (short*)&streamBuf[wavReadIdx];
            for(i = 0,j=0; i < nTemp/2; i+=2,j++){
               audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
               audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
            }
        } else {
            //printf("[Mp2 Enc] memcpy \n");
            // read data
            readBuffer = (short*)&streamBuf[wavReadIdx];
            for(i = 0,j=0; i < (READBUF_SIZE-wavReadIdx)/2; i+=2,j++){
               audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
               audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
            }
            // read data
            readBuffer = (short*)&streamBuf[0];
            for(i = 0 ; i < (nTemp-(READBUF_SIZE-wavReadIdx))/2; i+=2,j++){
               audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
               audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
            }
        }        
        ChangeEndian((char*)&audioContext->reSamplePcmInput[0][audioContext->nKeep[0]], nTemp/2);
        ChangeEndian((char*)&audioContext->reSamplePcmInput[1][audioContext->nKeep[1]], nTemp/2);        
        // resample
        nTmp = resample(&avResampleContext,audioContext,(short*)paramBuf);

    }
    //printf("[Mp2 Enc]  wavReadIdx  %d %d\n",wavReadIdx,nRead++);

    occupyPCMBuffer();

    // Update Read Buffer
    if (nTemp > 0) {
        wavReadIdx = wavReadIdx + nTemp;
        if (wavReadIdx >= READBUF_SIZE) {
            wavReadIdx -= READBUF_SIZE;
        }
        setStreamRdPtr(wavReadIdx);
    }
#if defined(__FREERTOS__)
    dc_invalidate(); // Flush Data Cache
#endif

    releasePCMBuffer();
    
    return len;
}

/**************************************************************************************
 * Function     : FillReadBufferOtherRisc
 *
 * Description  : Update the read pointer of WAVE Buffer and return the valid data length
 *                of input buffer.
 *
 * Inputs       : nReadBytes: number of bytes will read
 *
 * Global var   : wavWriteIdx: write pointer of WAVE buffer
 *                wavReadIdx : read pointer of WAVE buffer
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         : The WAVE buffer is circular buffer.
 *
 **************************************************************************************/
static int FillReadBufferOtherRisc() 
{
    int len = 0;
    unsigned int wavReadIdx,wavWriteIdx;
    int nTemp,bytesLeft;
    int i,j;
    short* buf;// = (char *)&pcmWriteBuf[pcmWriteIdx];
    int* in;//  = (char *)pcmbuf;

    // Updates the read buffer and returns the avaliable size
    // of input buffer. Wait a minimun FRAME_SIZE length.        
    nTemp = MPA_FRAME_SIZE*2*gMpegAudioContext.nb_channels*(gMp2AudioEncode.nSampleSize/16);

    // buffer reserve from Other Risc AP
    if (gMp2AudioEncode.nInputBufferType==1)
    {
        do
        {
            bytesLeft = GetAvaliableReadBufferSizeOtherRisc();
#ifdef ITE_RISC
            PalSleep(1);
#endif
        }while (bytesLeft<=nTemp && !isSTOP());
#if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
#endif
        // read data into encode buffer
        wavReadIdx = getStreamRdPtrOtherRisc();
        if (gMp2AudioEncode.nSampleSize==16){
            if (nTemp+wavReadIdx<=gMp2AudioEncode.nBufferLength) {
                memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],nTemp);
            } else {
                printf("[Mp2 Enc] 16 memcpy \n");
                memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],gMp2AudioEncode.nBufferLength-wavReadIdx);
                memcpy(&paramBuf[gMp2AudioEncode.nBufferLength-wavReadIdx],&gMp2AudioEncode.pInputBuffer[0],nTemp-(gMp2AudioEncode.nBufferLength-wavReadIdx));
            }
        } else if (gMp2AudioEncode.nSampleSize==32){

            if (nTemp+wavReadIdx<=gMp2AudioEncode.nBufferLength) {
                buf = (short*)paramBuf;
                in = (int*) &gMp2AudioEncode.pInputBuffer[wavReadIdx];
                for (i= 0;i<MPA_FRAME_SIZE*gMpegAudioContext.nb_channels;i++){
                    buf[i] = (short)in[i];
                }
            } else {
                printf("[Mp2 Enc] 32 memcpy\n");
                buf = (short*)paramBuf;
                in = (int*) &gMp2AudioEncode.pInputBuffer[wavReadIdx];

                for (i= 0;i<(gMp2AudioEncode.nBufferLength-wavReadIdx)/4;i++){
                    buf[i] = (short)in[i];
                }

                in = (int*) &gMp2AudioEncode.pInputBuffer[0];
                for (j= 0;i<MPA_FRAME_SIZE*gMpegAudioContext.nb_channels;i++,j++){
                    buf[i] = (short)in[j];
                }

            }

        }
        //printf("[Mp2 Enc]  wavReadIdx  %d %d\n",wavReadIdx,nRead++);

        occupyPCMBuffer();        
        //wavReadIdx = getStreamRdPtr();
        // Update Read Buffer
        if (nTemp > 0) {
            wavReadIdx = wavReadIdx + nTemp;
            if (wavReadIdx >= gMp2AudioEncode.nBufferLength) {
                wavReadIdx -= gMp2AudioEncode.nBufferLength;
            }
            setStreamRdPtrOtherRisc(wavReadIdx);
        }
#if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
#endif

        releasePCMBuffer();
    }
    
    return len;
}


/**************************************************************************************
 * Function     : FillReadBuffer_I2S
 *
 * Description  : Update the read pointer of WAVE Buffer and return the valid data length
 *                of input buffer.
 *
 * Inputs       : nReadBytes: number of bytes will read
 *
 * Global var   : wavWriteIdx: write pointer of WAVE buffer
 *                wavReadIdx : read pointer of WAVE buffer
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         : The WAVE buffer is circular buffer.
 *
 **************************************************************************************/
static int FillReadBuffer_I2S() 
{
    int len = 0;
    unsigned int wavReadIdx,wavWriteIdx;
    unsigned int mixReadIdx,mixWriteIdx;    
    int nTemp,bytesLeft;
    int i,j;
    short* buf;// = (char *)&pcmWriteBuf[pcmWriteIdx];
    short* pMixbuf;
    int* in;//  = (char *)pcmbuf;
    int bNoI2S = 0;
    int bNoMixer = 0;

    // Updates the read buffer and returns the avaliable size
    // of input buffer. Wait a minimun FRAME_SIZE length.        
    nTemp = MPA_FRAME_SIZE*2*gMpegAudioContext.nb_channels*(gMp2AudioEncode.nSampleSize/16);

    // buffer reserve from I2S
    if (gMp2AudioEncode.nInputBufferType==2 && gMp2AudioEncode.nEnableMixer ==0)
    {
        do
        {
            bytesLeft = GetAvaliableReadBufferSize_I2S();
#ifdef ITE_RISC
            PalSleep(1);
#endif
        }while (bytesLeft<=nTemp && !isSTOP());
#if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
#endif
        // read data into encode buffer
        wavReadIdx = I2S_AD32_GET_RP();
        if (gMp2AudioEncode.nSampleSize==16){
            if (nTemp+wavReadIdx<=gMp2AudioEncode.nBufferLength) {
                memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],nTemp);
            } else {
                //printf("[Mp2 Enc] 16 memcpy \n");
                memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],gMp2AudioEncode.nBufferLength-wavReadIdx);
                memcpy(&paramBuf[gMp2AudioEncode.nBufferLength-wavReadIdx],&gMp2AudioEncode.pInputBuffer[0],nTemp-(gMp2AudioEncode.nBufferLength-wavReadIdx));
            }
        } else if (gMp2AudioEncode.nSampleSize==32){

            if (nTemp+wavReadIdx<=gMp2AudioEncode.nBufferLength) {
                buf = (short*)paramBuf;
                in = (int*) &gMp2AudioEncode.pInputBuffer[wavReadIdx];
                for (i= 0;i<MPA_FRAME_SIZE*gMpegAudioContext.nb_channels;i++){
                    buf[i] = (short)in[i];
                }
            } else {
               // printf("[Mp2 Enc] 32 memcpy\n");
                buf = (short*)paramBuf;
                in = (int*) &gMp2AudioEncode.pInputBuffer[wavReadIdx];

                for (i= 0;i<(gMp2AudioEncode.nBufferLength-wavReadIdx)/4;i++){
                    buf[i] = (short)in[i];
                }

                in = (int*) &gMp2AudioEncode.pInputBuffer[0];
                for (j= 0;i<MPA_FRAME_SIZE*gMpegAudioContext.nb_channels;i++,j++){
                    buf[i] = (short)in[j];
                }
            }
        }
        //printf("[Mp2 Enc]  wavReadIdx  %d %d\n",wavReadIdx,nRead++);
        occupyPCMBuffer();        
        //wavReadIdx = getStreamRdPtr();
        // Update Read Buffer
        if (nTemp > 0) {
            wavReadIdx = wavReadIdx + nTemp;
            if (wavReadIdx >= gMp2AudioEncode.nBufferLength) {
                wavReadIdx -= gMp2AudioEncode.nBufferLength;
            }
            I2S_AD32_SET_RP(wavReadIdx);
        }
#if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
#endif

        releasePCMBuffer();
    } else if (gMp2AudioEncode.nInputBufferType==2 && gMp2AudioEncode.nEnableMixer ==1) {
        // wait buffer available
        do
        {
            bytesLeft = GetAvaliableReadBufferSize_Mixer();
#ifdef ITE_RISC
            PalSleep(1);
#else
            for (i = 0; i < 1024; i++) asm("");
#endif
        }while (bytesLeft<=nTemp && !isSTOP());

        bytesLeft = GetAvaliableReadBufferSize_I2S();
        if (bytesLeft < nTemp)
        {
            bNoI2S = 1;
        }
        else
        {
            bNoI2S = 0;
        }

#if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
#endif

        if (bNoI2S && bNoMixer)
        {
            return 0;
        }

        if (bNoI2S)
        {
            // mix data
            mixReadIdx = getMixerReadPorinter();
            buf = (short*)paramBuf;
            if (gMp2AudioEncode.nSampleSize==16){
                if (nTemp+mixReadIdx<=gMp2AudioEncode.nMixBufferLength) {
                    memcpy(buf, &gMp2AudioEncode.pMixBuffer[mixReadIdx], nTemp);
                } else {
                    pMixbuf = (short*) &gMp2AudioEncode.pMixBuffer[mixReadIdx];
                    memcpy(buf, pMixbuf,gMp2AudioEncode.nMixBufferLength-mixReadIdx);
                    pMixbuf= (short*) &gMp2AudioEncode.pMixBuffer[0];
                    memcpy(&buf[(gMp2AudioEncode.nMixBufferLength-mixReadIdx)/2], pMixbuf, (nTemp - (gMp2AudioEncode.nMixBufferLength-mixReadIdx)));
                }
            }
        }
        else if (bNoMixer)
        {
            // read data into encode buffer
            wavReadIdx = I2S_AD32_GET_RP();
            buf = (short*)paramBuf;
            if (gMp2AudioEncode.nSampleSize==16){
                if (nTemp+wavReadIdx<=gMp2AudioEncode.nBufferLength) {
                    memcpy(buf,&gMp2AudioEncode.pInputBuffer[wavReadIdx],nTemp);
                } else {
                    //printf("[Mp2 Enc] 16 memcpy \n");
                    memcpy(buf,&gMp2AudioEncode.pInputBuffer[wavReadIdx],gMp2AudioEncode.nBufferLength-wavReadIdx);
                    memcpy(&buf[(gMp2AudioEncode.nBufferLength-wavReadIdx)/2],&gMp2AudioEncode.pInputBuffer[0],nTemp-(gMp2AudioEncode.nBufferLength-wavReadIdx));
                }
            }
            ControlVolume(buf, nTemp);            
        }
        else
        {
            // read data into encode buffer
            wavReadIdx = I2S_AD32_GET_RP();
            if (gMp2AudioEncode.nSampleSize==16){
                if (nTemp+wavReadIdx<=gMp2AudioEncode.nBufferLength) {
                    memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],nTemp);
                } else {
                    //printf("[Mp2 Enc] 16 memcpy \n");
                    memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],gMp2AudioEncode.nBufferLength-wavReadIdx);
                    memcpy(&paramBuf[gMp2AudioEncode.nBufferLength-wavReadIdx],&gMp2AudioEncode.pInputBuffer[0],nTemp-(gMp2AudioEncode.nBufferLength-wavReadIdx));
                }
            } else if (gMp2AudioEncode.nSampleSize==32){
                if (nTemp+wavReadIdx<=gMp2AudioEncode.nBufferLength) {
                    buf = (short*)paramBuf;
                    in = (int*) &gMp2AudioEncode.pInputBuffer[wavReadIdx];
                    for (i= 0;i<MPA_FRAME_SIZE*gMpegAudioContext.nb_channels;i++){
                        buf[i] = (short)in[i];
                    }
                } else {
                   // printf("[Mp2 Enc] 32 memcpy\n");
                    buf = (short*)paramBuf;
                    in = (int*) &gMp2AudioEncode.pInputBuffer[wavReadIdx];
                    for (i= 0;i<(gMp2AudioEncode.nBufferLength-wavReadIdx)/4;i++){
                        buf[i] = (short)in[i];
                    }
                    in = (int*) &gMp2AudioEncode.pInputBuffer[0];
                    for (j= 0;i<MPA_FRAME_SIZE*gMpegAudioContext.nb_channels;i++,j++){
                        buf[i] = (short)in[j];
                    }
                }
            }
            buf =  (short*)paramBuf;
            ControlVolume(buf, nTemp);
            // mix data
            mixReadIdx = getMixerReadPorinter();
            if (gMp2AudioEncode.nSampleSize==16){
                if (nTemp+mixReadIdx<=gMp2AudioEncode.nMixBufferLength) {
                    //memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],nTemp);
                    buf = (short*)paramBuf;
                    pMixbuf = (short*) &gMp2AudioEncode.pMixBuffer[mixReadIdx];
                    for (i= 0;i<MPA_FRAME_SIZE *gMpegAudioContext.nb_channels;i++){
                        buf[i] = MixerResult(buf[i] , pMixbuf[i]);
                    }

                } else {
                    buf = (short*)paramBuf;
                    pMixbuf = (short*) &gMp2AudioEncode.pMixBuffer[mixReadIdx];
                    for (i= 0;i<(gMp2AudioEncode.nMixBufferLength-mixReadIdx)/2;i++){
                        buf[i] = MixerResult(buf[i] , pMixbuf[i]);
                    }
                    pMixbuf= (short*) &gMp2AudioEncode.pMixBuffer[0];
                    for (j= 0;j<MPA_FRAME_SIZE*gMpegAudioContext.nb_channels-((gMp2AudioEncode.nMixBufferLength-mixReadIdx)/2);i++,j++){
                        buf[i] = MixerResult(buf[i] , pMixbuf[j]);                    
                    }

                }
            }
        }
        
        // Update Read Buffer
        //if (nTemp > 0) {
        if (!bNoI2S)
        {
            wavReadIdx = wavReadIdx + nTemp;
            if (wavReadIdx >= gMp2AudioEncode.nBufferLength) {
                wavReadIdx -= gMp2AudioEncode.nBufferLength;
            }
            I2S_AD32_SET_RP(wavReadIdx);
        }

        if (!bNoMixer)
        {
            mixReadIdx = mixReadIdx + nTemp;
            if (mixReadIdx >= gMp2AudioEncode.nMixBufferLength) {
                mixReadIdx -= gMp2AudioEncode.nMixBufferLength;
            }
            //printf("[Mp2 Enc] setMixerReadPorinter %d \n",mixReadIdx);
            setMixerReadPorinter(mixReadIdx);
        }
        //}
#if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
#endif

    }
    
    return len;
}

/**************************************************************************************
 * Function     : FillReadBuffer_I2S_Resample
 *
 * Description  : Update the read pointer of WAVE Buffer and return the valid data length
 *                of input buffer.
 *
 * Inputs       : nReadBytes: number of bytes will read
 *
 * Global var   : wavWriteIdx: write pointer of WAVE buffer
 *                wavReadIdx : read pointer of WAVE buffer
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         : The WAVE buffer is circular buffer.
 *
 **************************************************************************************/
static int FillReadBuffer_I2S_Resample(ResampleAudioContext *audioContext) 
{
    int len = 0;
    unsigned int wavReadIdx,wavWriteIdx;
    int nTmp;
    int nTemp,bytesLeft;
    int i,j;
    short* buf;// = (char *)&pcmWriteBuf[pcmWriteIdx];
    int* in;//  = (char *)pcmbuf;
    short* readBuffer;

    // Updates the read buffer and returns the avaliable size
    // of input buffer. Wait a minimun FRAME_SIZE length.        
    //nTemp = MPA_FRAME_SIZE*2*gMpegAudioContext.nb_channels*(gMp2AudioEncode.nSampleSize/16);
    nTemp = audioContext->nInSize*(gMp2AudioEncode.nSampleSize/16);

    // check resample temp buffers
    if (audioContext->nUseTempBuffer == 1){
        nTmp = getResampleAvailableLength(audioContext);
        //printf("getResampleAvailableLength %d  %d  \n",nTemp,2*MAX_FRAMESIZE);
        if (nTmp>=audioContext->nInChannels*MAX_FRAMESIZE){
            printf("getResampleAvailableLength %d > %d  \n",nTmp,audioContext->nInChannels*MAX_FRAMESIZE);
            getResampleEncodeBuffer(audioContext,(short*)paramBuf,audioContext->nInChannels*MAX_FRAMESIZE);
            return 0;
        }
    }

    // buffer reserve from I2S
    if (gMp2AudioEncode.nInputBufferType==2)
    {
        do
        {
            bytesLeft = GetAvaliableReadBufferSize_I2S();
#ifdef ITE_RISC
            PalSleep(1);
#endif
        }while (bytesLeft<=nTemp && !isSTOP());
#if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
#endif
        // read data into encode buffer
        wavReadIdx = I2S_AD32_GET_RP();
        if (gMp2AudioEncode.nSampleSize==16){
            if (nTemp+wavReadIdx<=gMp2AudioEncode.nBufferLength) {
                //memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],nTemp);
                // read data
                readBuffer = (short*)&gMp2AudioEncode.pInputBuffer[wavReadIdx];
                for(i = 0,j=0; i < nTemp/2; i+=2,j++){
                   audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
                   audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
                }                
            } else {
                //printf("[Mp2 Enc] 16 memcpy \n");
                //memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],gMp2AudioEncode.nBufferLength-wavReadIdx);
                //memcpy(&paramBuf[gMp2AudioEncode.nBufferLength-wavReadIdx],&gMp2AudioEncode.pInputBuffer[0],nTemp-(gMp2AudioEncode.nBufferLength-wavReadIdx));
                // read data
                readBuffer = (short*)&gMp2AudioEncode.pInputBuffer[wavReadIdx];
                for(i = 0,j=0; i < (gMp2AudioEncode.nBufferLength-wavReadIdx)/2; i+=2,j++){
                   audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
                   audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
                }                
                // read data
                readBuffer = (short*)&gMp2AudioEncode.pInputBuffer[0];
                for(i = 0 ; i < (nTemp-(gMp2AudioEncode.nBufferLength-wavReadIdx))/2; i+=2,j++){
                   audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
                   audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
                }                
                
            }

    #if 0//def MP2_ENCODE_PERFORMANCE_TEST_BY_TICK
            start_timer();
    #endif
           
            // resample
            nTmp = resample(&avResampleContext,audioContext,(short*)paramBuf);

    #if 0 //def MP2_ENCODE_PERFORMANCE_TEST_BY_TICK
            gnNewTicks = get_timer();            
            gnTotalTicks += gnNewTicks;
            if (gnFrames % 50 == 0 && gnFrames>0) {
                gnTotalTime += (gnTotalTicks/(PalGetSysClock()/1000));
                printf("[MP2 Enc] resample (%d~%d) total %d (ms) average %d (ms) nFrames %d\n",(gnFrames+1-50),(gnFrames+1),(gnTotalTicks/(PalGetSysClock()/1000)),((gnTotalTicks/(PalGetSysClock()/1000))/50),gnFrames+1);
                gnTotalTicks=0;
            }
    #endif
            
        } else if (gMp2AudioEncode.nSampleSize==32){
            printf("[Mp2 Enc] resample not support 32 bits sample size\n");
#if 0
            if (nTemp+wavReadIdx<=gMp2AudioEncode.nBufferLength) {
                buf = (short*)paramBuf;
                in = (int*) &gMp2AudioEncode.pInputBuffer[wavReadIdx];
                for (i= 0;i<MPA_FRAME_SIZE*gMpegAudioContext.nb_channels;i++){
                    buf[i] = (short)in[i];
                }
            } else {
               // printf("[Mp2 Enc] 32 memcpy\n");
                buf = (short*)paramBuf;
                in = (int*) &gMp2AudioEncode.pInputBuffer[wavReadIdx];

                for (i= 0;i<(gMp2AudioEncode.nBufferLength-wavReadIdx)/4;i++){
                    buf[i] = (short)in[i];
                }

                in = (int*) &gMp2AudioEncode.pInputBuffer[0];
                for (j= 0;i<MPA_FRAME_SIZE*gMpegAudioContext.nb_channels;i++,j++){
                    buf[i] = (short)in[j];
                }

            }
#endif
        }
        //printf("[Mp2 Enc]  wavReadIdx  %d %d\n",wavReadIdx,nRead++);

        occupyPCMBuffer();        
        //wavReadIdx = getStreamRdPtr();
        // Update Read Buffer
        if (nTemp > 0) {
            wavReadIdx = wavReadIdx + nTemp;
            if (wavReadIdx >= gMp2AudioEncode.nBufferLength) {
                wavReadIdx -= gMp2AudioEncode.nBufferLength;
            }
            I2S_AD32_SET_RP(wavReadIdx);
        }
#if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
#endif

        releasePCMBuffer();
    }
    
    return len;
}

/**************************************************************************************
 * Function     : FillReadBuffer_I2S_Resample_Mixer
 *
 * Description  : Update the read pointer of WAVE Buffer and return the valid data length
 *                of input buffer.
 *
 * Inputs       : nReadBytes: number of bytes will read
 *
 * Global var   : wavWriteIdx: write pointer of WAVE buffer
 *                wavReadIdx : read pointer of WAVE buffer
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         : The WAVE buffer is circular buffer.
 *
 **************************************************************************************/
static int FillReadBuffer_I2S_Resample_Mixer(ResampleAudioContext *audioContext) 
{
    int len = 0;
    unsigned int wavReadIdx,wavWriteIdx;
    unsigned int mixReadIdx,mixWriteIdx;    
   
    int nTmp;
    int nTemp,bytesLeft;
    int i,j;
    short* buf;// = (char *)&pcmWriteBuf[pcmWriteIdx];
    short* pMixbuf;
    
    int* in;//  = (char *)pcmbuf;
    short* readBuffer;
    int bNoI2S = 0;
    int bNoMixer = 0;
    int prevUsedSize = 0;
    int waitCount = 0;
    int mixerSampleLen = MPA_FRAME_SIZE * 2 * gMpegAudioContext.nb_channels * (gMp2AudioEncode.nSampleSize/16);
    // Updates the read buffer and returns the avaliable size
    // of input buffer. Wait a minimun FRAME_SIZE length.        
    //nTemp = MPA_FRAME_SIZE*2*gMpegAudioContext.nb_channels*(gMp2AudioEncode.nSampleSize/16);
    nTemp = audioContext->nInSize*(gMp2AudioEncode.nSampleSize/16);

    // check resample temp buffers
    if (audioContext->nUseTempBuffer == 1){
        nTmp = getResampleAvailableLength(audioContext);
        //printf("getResampleAvailableLength %d  %d  \n",nTemp,2*MAX_FRAMESIZE);
        if (nTmp>=audioContext->nInChannels*MAX_FRAMESIZE){
            printf("getResampleAvailableLength %d > %d  \n",nTmp,audioContext->nInChannels*MAX_FRAMESIZE);
            getResampleEncodeBuffer(audioContext,(short*)paramBuf,audioContext->nInChannels*MAX_FRAMESIZE);
            return 0;
        }
    }
    // buffer reserve from I2S
    if (gMp2AudioEncode.nInputBufferType==2)
    {
        do
        {
            bytesLeft = GetAvaliableReadBufferSize_Mixer();
#ifdef ITE_RISC
            PalSleep(1);
#else
            for (i = 0; i < 512; i++) asm("");
#endif
        }while (bytesLeft<= mixerSampleLen && !isSTOP());       

        bytesLeft = GetAvaliableReadBufferSize_I2S();
        if (bytesLeft < nTemp)
        {
            bNoI2S = 1;
        }
        else
        {
            bNoI2S = 0;
        }

#if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
#endif

        if (bNoI2S && bNoMixer)
        {
            return 0;
        }

        if (bNoI2S)
        {
            // mix data
            mixReadIdx = getMixerReadPorinter();
            buf = (short*)paramBuf;
            if (gMp2AudioEncode.nSampleSize==16){
                if (mixerSampleLen+mixReadIdx<=gMp2AudioEncode.nMixBufferLength) {
                    memcpy(buf, &gMp2AudioEncode.pMixBuffer[mixReadIdx], mixerSampleLen);
                } else {
                    pMixbuf = (short*) &gMp2AudioEncode.pMixBuffer[mixReadIdx];
                    memcpy(buf, pMixbuf,gMp2AudioEncode.nMixBufferLength-mixReadIdx);
                    pMixbuf= (short*) &gMp2AudioEncode.pMixBuffer[0];
                    memcpy(&buf[(gMp2AudioEncode.nMixBufferLength-mixReadIdx)/2], pMixbuf, (mixerSampleLen - (gMp2AudioEncode.nMixBufferLength-mixReadIdx)));
                }
            }
        }
        else if (bNoMixer)
        {
            if (gMp2AudioEncode.nSampleSize==16){
                if (nTemp+wavReadIdx<=gMp2AudioEncode.nBufferLength) {
                    //memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],nTemp);
                    // read data
                    readBuffer = (short*)&gMp2AudioEncode.pInputBuffer[wavReadIdx];

                    for(i = 0,j=0; i < nTemp/2; i+=2,j++){
                       audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
                       audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
                    }
                } else {
                    //printf("[Mp2 Enc] 16 memcpy \n");
                    //memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],gMp2AudioEncode.nBufferLength-wavReadIdx);
                    //memcpy(&paramBuf[gMp2AudioEncode.nBufferLength-wavReadIdx],&gMp2AudioEncode.pInputBuffer[0],nTemp-(gMp2AudioEncode.nBufferLength-wavReadIdx));
                    // read data
                    readBuffer = (short*)&gMp2AudioEncode.pInputBuffer[wavReadIdx];
                    for(i = 0,j=0; i < (gMp2AudioEncode.nBufferLength-wavReadIdx)/2; i+=2,j++){
                       audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
                       audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
                    }
                    // read data
                    readBuffer = (short*)&gMp2AudioEncode.pInputBuffer[0];
                    for(i = 0 ; i < (nTemp-(gMp2AudioEncode.nBufferLength-wavReadIdx))/2; i+=2,j++){
                       audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
                       audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
                    }
                }
            }
            // resample
            nTmp = resample(&avResampleContext,audioContext,(short*)paramBuf);
            ControlVolume((short*) paramBuf, MPA_FRAME_SIZE * gMpegAudioContext.nb_channels);            
        }
        else
        {
            // read data into encode buffer
            wavReadIdx = I2S_AD32_GET_RP();
            if (gMp2AudioEncode.nSampleSize==16){
#if 1
                if (nTemp+wavReadIdx<=gMp2AudioEncode.nBufferLength) {
                    //memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],nTemp);
                    // read data
                    readBuffer = (short*)&gMp2AudioEncode.pInputBuffer[wavReadIdx];
                    for(i = 0,j=0; i < nTemp/2; i+=2,j++){
                       audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
                       audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
                    }
                } else {
                    //printf("[Mp2 Enc] 16 memcpy \n");
                    //memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],gMp2AudioEncode.nBufferLength-wavReadIdx);
                    //memcpy(&paramBuf[gMp2AudioEncode.nBufferLength-wavReadIdx],&gMp2AudioEncode.pInputBuffer[0],nTemp-(gMp2AudioEncode.nBufferLength-wavReadIdx));
                    // read data
                    readBuffer = (short*)&gMp2AudioEncode.pInputBuffer[wavReadIdx];
                    for(i = 0,j=0; i < (gMp2AudioEncode.nBufferLength-wavReadIdx)/2; i+=2,j++){
                       audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
                       audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
                    }
                    // read data
                    readBuffer = (short*)&gMp2AudioEncode.pInputBuffer[0];
                    for(i = 0 ; i < (nTemp-(gMp2AudioEncode.nBufferLength-wavReadIdx))/2; i+=2,j++){
                       audioContext->reSamplePcmInput[0][audioContext->nKeep[0]+j] = readBuffer[i];
                       audioContext->reSamplePcmInput[1][audioContext->nKeep[1]+j] = readBuffer[i+1];
                    }
                }
                // resample
                nTmp = resample(&avResampleContext,audioContext,(short*)paramBuf);
                ControlVolume((short*) paramBuf, MPA_FRAME_SIZE * gMpegAudioContext.nb_channels);
#endif
                // after resample then mix data
                mixReadIdx = getMixerReadPorinter();
#if 1
                if (mixerSampleLen+mixReadIdx<=gMp2AudioEncode.nMixBufferLength) {
                    //memcpy(&paramBuf[0],&gMp2AudioEncode.pInputBuffer[wavReadIdx],nTemp);
                    pMixbuf = (short*) &gMp2AudioEncode.pMixBuffer[mixReadIdx];
                    buf = (short*) paramBuf;
                    for(i = 0; i < mixerSampleLen/2; i++){
                        buf[i] = CLIPTOSHORT(pMixbuf[i] + buf[i]);
                    }
                } else {
                    pMixbuf = (short*) &gMp2AudioEncode.pMixBuffer[mixReadIdx];
                    buf = (short*) paramBuf;
                    for(i = 0; i < (gMp2AudioEncode.nMixBufferLength - mixReadIdx) / 2; i++)
                    {
                        buf[i] = CLIPTOSHORT(pMixbuf[i] + buf[i]);
                    }
                    pMixbuf = (short*) &gMp2AudioEncode.pMixBuffer[0];
                    buf = (short*) &paramBuf[gMp2AudioEncode.nMixBufferLength - mixReadIdx];
                    
                    for (i = 0; i < (mixerSampleLen - (gMp2AudioEncode.nMixBufferLength - mixReadIdx)) / 2; i++)
                    {
                        buf[i] = CLIPTOSHORT(pMixbuf[i] + buf[i]);
                    }
                }
#endif
        #if 0//def MP2_ENCODE_PERFORMANCE_TEST_BY_TICK
                start_timer();
        #endif

        #if 0 //def MP2_ENCODE_PERFORMANCE_TEST_BY_TICK
                gnNewTicks = get_timer();            
                gnTotalTicks += gnNewTicks;
                if (gnFrames % 50 == 0 && gnFrames>0) {
                    gnTotalTime += (gnTotalTicks/(PalGetSysClock()/1000));
                    printf("[MP2 Enc] resample (%d~%d) total %d (ms) average %d (ms) nFrames %d\n",(gnFrames+1-50),(gnFrames+1),(gnTotalTicks/(PalGetSysClock()/1000)),((gnTotalTicks/(PalGetSysClock()/1000))/50),gnFrames+1);
                    gnTotalTicks=0;
                }
        #endif
                
            } else if (gMp2AudioEncode.nSampleSize==32){
                printf("[Mp2 Enc] resample not support 32 bits sample size\n");
    #if 0
                if (nTemp+wavReadIdx<=gMp2AudioEncode.nBufferLength) {
                    buf = (short*)paramBuf;
                    in = (int*) &gMp2AudioEncode.pInputBuffer[wavReadIdx];
                    for (i= 0;i<MPA_FRAME_SIZE*gMpegAudioContext.nb_channels;i++){
                        buf[i] = (short)in[i];
                    }
                } else {
                   // printf("[Mp2 Enc] 32 memcpy\n");
                    buf = (short*)paramBuf;
                    in = (int*) &gMp2AudioEncode.pInputBuffer[wavReadIdx];

                    for (i= 0;i<(gMp2AudioEncode.nBufferLength-wavReadIdx)/4;i++){
                        buf[i] = (short)in[i];
                    }

                    in = (int*) &gMp2AudioEncode.pInputBuffer[0];
                    for (j= 0;i<MPA_FRAME_SIZE*gMpegAudioContext.nb_channels;i++,j++){
                        buf[i] = (short)in[j];
                    }

                }
    #endif
            }
        }
        //printf("[Mp2 Enc]  wavReadIdx  %d %d\n",wavReadIdx,nRead++);

        occupyPCMBuffer();
        //wavReadIdx = getStreamRdPtr();
        // Update Read Buffer
        //if (nTemp > 0) {
        if (!bNoI2S)
        {
            wavReadIdx = wavReadIdx + nTemp;
            if (wavReadIdx >= gMp2AudioEncode.nBufferLength) {
                wavReadIdx -= gMp2AudioEncode.nBufferLength;
            }
            I2S_AD32_SET_RP(wavReadIdx);
        }
        
        if (!bNoMixer)
        {
            mixReadIdx = mixReadIdx + mixerSampleLen;
            if (mixReadIdx >= gMp2AudioEncode.nMixBufferLength) {
                mixReadIdx -= gMp2AudioEncode.nMixBufferLength;
            }
            //printf("[Mp2 Enc] setMixerReadPorinter %d \n",mixReadIdx);
            setMixerReadPorinter(mixReadIdx);
        }
        //}
#if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
#endif

        releasePCMBuffer();
    }
    
    return len;
}


// return 1:write output buffer,return 0: not write output buffer
static int OutputWriteBuffer(int nMp3Bytes)
{

#ifdef OUTPUT_ONE_FRAME
    // get time stamp;
    if (gMpegAudioContext.sample_rate==48000)
        gTimeStamp = TimeStamp48000[0];
    else if (gMpegAudioContext.sample_rate==44100)
        gTimeStamp = TimeStamp44100[0];
    else if (gMpegAudioContext.sample_rate==32000)
        gTimeStamp = TimeStamp32000[0];

    gDataSize=nMp3Bytes;
    memcpy(&gOutFrameBuf[0],&gTimeStamp,sizeof(int));
    memcpy(&gOutFrameBuf[4],&gDataSize,sizeof(int));
    if (nMp3Bytes<=OUTFRAME_SIZE)
        memcpy(&gOutFrameBuf[8],&gFrame[0],nMp3Bytes);
    
    if (nMp3Bytes==0 || nMp3Bytes >OUTFRAME_SIZE)
        printf("[Mp2 Enc] OutputWriteBuffer %d %d %d\n",gTimeStamp,gDataSize,nMp3Bytes);        

    return 1;

#else
    if ( (gAppendFrame+1)>=10 || (gDataSize+nMp3Bytes)>=OUTFRAME_SIZE-8){
        // get time stamp;
        if (gMpegAudioContext.sample_rate==48000)
            gTimeStamp = TimeStamp48000[gAppendFrame-1];
        else if (gMpegAudioContext.sample_rate==44100)
            gTimeStamp = TimeStamp44100[gAppendFrame-1];
        else if (gMpegAudioContext.sample_rate==32000)
            gTimeStamp = TimeStamp32000[gAppendFrame-1];

        memcpy(&gOutFrameBuf[0],&gTimeStamp,sizeof(int));
        memcpy(&gOutFrameBuf[4],&gDataSize,sizeof(int));            

       // printf("[Mp2 Enc] OutputWriteBuffer %d %d %d %d\n",gTimeStamp,gDataSize,gAppendFrame,nMp3Bytes);        

        gAppendFrame=0;
        gDataSize=0;

        return 1;
    } else {
        return 0;
    }
#endif
}


static int AppendWriteBuffer(int nMp3Bytes)
{
    memcpy(&gOutFrameBuf[8+gDataSize],&gFrame[0],nMp3Bytes);
    gAppendFrame++;
    gDataSize+=nMp3Bytes;

}


/**************************************************************************************
 * Function     : FillWriteBuffer
 *
 * Description  : Wait the avaliable length of the output buffer bigger than on
 *                frame of audio data.
 *
 * Inputs       :
 *
 * Global Var   : pcmWriteIdx: write index of output buffer.
 *                pcmReadIdx : read index of output buffer.
 *
 * Outputs      :
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         : Output buffer is a circular queue.
 *
 **************************************************************************************/
static void FillWriteBuffer(int nMp3Bytes) 
{
    int len;
    int nOutReadIdx,nOutWriteIdx;

    occupyEncodeBuffer();
    
    nOutWriteIdx = getOutBufWrPtr(); 
    // Update Write Buffer
#if 1
    if (OutputWriteBuffer(nMp3Bytes) > 0) {
        if (nOutWriteIdx+OUTFRAME_SIZE > OUTBUF_SIZE) {
            // it will not happen
            memcpy(&gOutBuf[nOutWriteIdx],&gOutFrameBuf[0],OUTBUF_SIZE-nOutWriteIdx);
            memcpy(&gOutBuf[0],&gOutFrameBuf[OUTBUF_SIZE-nOutWriteIdx],OUTFRAME_SIZE-(OUTBUF_SIZE-nOutWriteIdx));
            nOutWriteIdx = 0;
        } else {
            memcpy(&gOutBuf[nOutWriteIdx],&gOutFrameBuf[0],OUTFRAME_SIZE);
            nOutWriteIdx = nOutWriteIdx + OUTFRAME_SIZE;
            if (nOutWriteIdx>=OUTBUF_SIZE)
                nOutWriteIdx=0;
        }
        //printf("[Mp2 Enc] set outBuffer %d \n",nOutWriteIdx);
        setOutBufWrPtr(nOutWriteIdx);
    }
    #ifndef OUTPUT_ONE_FRAME    
    AppendWriteBuffer(nMp3Bytes);
    #endif
#else
    if (nMp3Bytes > 0) {
    
        if (nOutWriteIdx+nMp3Bytes >= OUTBUF_SIZE) {
            memcpy(&gOutBuf[nOutWriteIdx],&gFrame[0],OUTBUF_SIZE-nOutWriteIdx);
            memcpy(&gOutBuf[0],&gFrame[OUTBUF_SIZE-nOutWriteIdx],nMp3Bytes-(OUTBUF_SIZE-nOutWriteIdx));
            nOutWriteIdx = nMp3Bytes-(OUTBUF_SIZE-nOutWriteIdx);
        } else {
            memcpy(&gOutBuf[nOutWriteIdx],&gFrame[0],nMp3Bytes);
            nOutWriteIdx = nOutWriteIdx + nMp3Bytes;            
        }
        setOutBufWrPtr(nOutWriteIdx);
    }
#endif

#if defined(__FREERTOS__)
        dc_invalidate(); // Flush Data Cache
#endif
    releaseEncodeBuffer();

    // Wait output buffer avaliable
    do {
        nOutReadIdx = getOutBufRdPtr();    
        if (nOutReadIdx <= nOutWriteIdx) {
            len = OUTBUF_SIZE - (nOutWriteIdx - nOutReadIdx);
        } else {
            len = nOutReadIdx - nOutWriteIdx;
        }

        if ((len-2) < OUTFRAME_SIZE && !isSTOP()) {
        #if defined(__FREERTOS__)
            PalSleep(2);
        #endif
        } else {
            break;
        }
    } while(1 && !isSTOP());

    // PRINTF("pcmWriteIdx(%d) pcmReadIdx(%d) len(%d) nPCMBytes(%d)\n", pcmWriteIdx, pcmReadIdx, len, nPCMBytes);
}

static __inline void checkControl(void) {
    static int curPause = 0;
    static int prePause = 0;

   /* do {
        eofReached  = isEOF() || isSTOP();
        curPause = isPAUSE();
        if (!curPause) {  // Un-pause
            if (prePause) pauseDAC(0);
            break;
        } else { // Pause
            if (!prePause && curPause) {
                pauseDAC(1);
            }
            #if defined(__FREERTOS__)
            //taskYIELD();
            PalSleep(1);
            #else
            or32_delay(1); // delay 1ms
            #endif
        }
        prePause = curPause;
    } while(!eofReached);*/

    prePause = curPause;
}

void ClearRdBuffer(void) {

    //SetFrameNo(0);
    MMIO_Write(DrvDecode_WrPtr, 0);
    MMIO_Write(DrvDecode_RdPtr, 0);
    MMIO_Write(DrvEncode_WrPtr, 0);
    MMIO_Write(DrvEncode_RdPtr, 0);

    if (isSTOP()) {
        MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) & ~DrvDecode_STOP);
    }
    
    #if defined(__OR32__)
        dc_invalidate(); // Flush DC Cache
    #endif
    releasePCMBuffer();
    releaseEncodeBuffer();
}

void MP2Encode_GetBufInfo(unsigned* inBuf, unsigned* inLen,unsigned* outEnBuf, unsigned* outEnLen,unsigned* pAudio)
{
#if defined(__FREERTOS__)
    dc_invalidate(); // Flush Data Cache
#endif         

    *inBuf = (unsigned)streamBuf;
    *inLen = sizeof(streamBuf);
    *outEnBuf =  (unsigned)gOutBuf;
    *outEnLen =  sizeof(gOutBuf);

    *pAudio = (unsigned)&gMp2AudioEncode;
}

void ChangeEndian(char* ptr,int size)
{
    int i;
    char *buf = (char *)ptr;
    char in;
    for(i=0; i<size; i+=2)
    {
        in = buf[i];
        buf[i]   = buf[i+1];
        buf[i+1] = in;
    }
}

void ChangeEndian4(char* ptr,int size)
{
    int i;
    char *buf = (char *)ptr;
    char in , in2;
    for(i=0; i<size; i+=4)
    {
        in = buf[i];
        in2 = buf[i+1];
        buf[i]   = buf[i+3];
        buf[i+1] = buf[i+2];
        buf[i+2] =in2;
        buf[i+3] = in;
    }
}

#endif // #ifdef ITE_RISC

# if defined(__FREERTOS__) && !defined(ENABLE_CODECS_PLUGIN)
portTASK_FUNCTION(mp2encode_task, params)
# else
int main(int argc, char **argv)
# endif
{
    int size;
    int bytesLeft;
    unsigned int wavReadIdx;
    int i;
#ifdef MP2_ENCODE_PERFORMANCE_TEST_BY_TICK
    int  nNewTicks,nTotalTicks;
    int  nTotalTime;
#endif
    // resample 
    ResampleAudioContext resampleAudioContext = { 0 };
    int nNeedResample = 0;
    int nTemp;
    int nSize;
    
    while (1)
    {
        gTimeStamp = 0;
        gDataSize = 0;
        gAppendFrame = 0;

        memset(gOutFrameBuf,0,sizeof(gOutFrameBuf));

        do {
            PalSleep(1);
        } while (MMIO_Read(AUDIO_DECODER_START_FALG) != 2);

        if (&gMp2AudioEncode){
            printf("[Mp2 Enc] start , input buffer type %d nSampleSize %d start pointer 0x%x \n",gMp2AudioEncode.nInputBufferType,gMp2AudioEncode.nSampleSize,I2S_AD32_GET_RP());
        } else {
            printf("[Mp2 Enc] start , input buffer null pointer \n");
        }
    
        if (gMp2AudioEncode.nInputBufferType==0 || gMp2AudioEncode.nInputBufferType==1){
            setStreamRdPtr(0);
            setStreamWrPtr(0);
        }
        setOutBufRdPtr(0);
        setOutBufWrPtr(0);
        
        Get_Encode_Parameter(&gMpegAudioContext);
#ifdef MP2_ENABLE_INTERNAL_SD
        gMpegAudioContext.samples_buf[0] = (short*)INTERNAL_SD;
        gMpegAudioContext.samples_buf[1] = (short*)&gMpegAudioContext.samples_buf_temp[1];
#else
        gMpegAudioContext.samples_buf[0] = (short*)&gMpegAudioContext.samples_buf_temp[0];
        gMpegAudioContext.samples_buf[1] = (short*)&gMpegAudioContext.samples_buf_temp[1];        
#endif
        MPA_encode_init(&gMpegAudioContext);
        //printf("[Mp2 Enc] 0x%x 0x%x \n",gMpegAudioContext.samples_buf[0],gMpegAudioContext.samples_buf[1]);
        
#ifdef MP2_ENCODE_PERFORMANCE_TEST_BY_TICK
         nNewTicks = 0;
         nTotalTicks=0;
         nTotalTime=0;

         gnNewTicks = 0;
         gnTotalTicks=0;
         gnTotalTime=0;         
#endif        

        gnFrames = 0;
// resample setting
        if (gMp2AudioEncode.nSampleRate != gMp2AudioEncode.nOutSampleRate) {            
            printf("[Mp2 Enc] resampleInit \n");
            resampleInit(&resampleAudioContext);
            av_resample_init(&avResampleContext,gMp2AudioEncode.nOutSampleRate, gMp2AudioEncode.nSampleRate, RESAMPLE_FILTER_LENGTH, 10,0, 0.8);
            printf("[Mp2 Enc] av_resample_init \n");            
            nNeedResample = 1;
        } else {
            nNeedResample = 0;
        }
// 
        if (gMp2AudioEncode.nEnableMixer==1){
            setMixerReadPorinter(getMixerWritePorinter());
            printf("[Mp2 Enc] enable mixer ,Mixer buffer length %d \n",gMp2AudioEncode.nMixBufferLength);
        }
        
        for(;;) // forever loop
        {
            int exitflag1 = 0;
            int i,j;
            int frameSize;
            Get_Encode_Parameter(&gMpegAudioContext);

            if (gMp2AudioEncode.nSampleRate != gMp2AudioEncode.nOutSampleRate) {
                if(nNeedResample == 0) {            
                    printf("[MP2 Enc] 2 resampleInit, nSample: %u, nOut: %u \n", gMp2AudioEncode.nSampleRate, gMp2AudioEncode.nOutSampleRate);
                    resampleInit(&resampleAudioContext);
                    av_resample_init(&avResampleContext,gMp2AudioEncode.nOutSampleRate, gMp2AudioEncode.nSampleRate, RESAMPLE_FILTER_LENGTH, 10,0, 0.8);
                    printf("[MP2 Enc] 2 av_resample_init \n");            
                    nNeedResample = 1;
                }
            } else {
                if (nNeedResample) {
                    printf("From resampling to not resample: %u\n", gMp2AudioEncode.nSampleRate);
                    nNeedResample = 0;
                }
            }
            
            // read input data
            if (gMp2AudioEncode.nInputBufferType==0){
                if (nNeedResample==1) {
                    FillReadBufferResample(&resampleAudioContext);                    
                } else {
                    FillReadBuffer();
                }
            } else if (gMp2AudioEncode.nInputBufferType==1){
                FillReadBufferOtherRisc();
            } else if (gMp2AudioEncode.nInputBufferType==2){
                if (nNeedResample==1) {
                    if (gMp2AudioEncode.nEnableMixer==1){
                        // resample and mixer
                        FillReadBuffer_I2S_Resample_Mixer(&resampleAudioContext);
                    } else {
                        FillReadBuffer_I2S_Resample(&resampleAudioContext);
                    }
                } else {
                    FillReadBuffer_I2S();
                }
            }

#ifdef ITE_RISC
        if (gMp2AudioEncode.nInputBufferType!=2)
            if (nNeedResample==0)
                ChangeEndian4(&paramBuf[0],MPA_FRAME_SIZE*2*gMpegAudioContext.nb_channels);
    #ifdef MP2_ENCODE_PERFORMANCE_TEST_BY_TICK
            start_timer();
    #endif
#endif
            // mp2 encode
            frameSize = MPA_encode_frame(&gMpegAudioContext,&gFrame[0],0,&paramBuf[0]);
        
#ifdef ITE_RISC
    #ifdef MP2_ENCODE_PERFORMANCE_TEST_BY_TICK
            nNewTicks = get_timer();            
            nTotalTicks += nNewTicks;
            if (gnFrames % 50 == 0 && gnFrames>0) {
                nTotalTime += (nTotalTicks/(PalGetSysClock()/1000));
                printf("[MP2 Enc] (%d~%d) total %d (ms) average %d (ms) nFrames %d system clock %d\n",(gnFrames+1-50),(gnFrames+1),(nTotalTicks/(PalGetSysClock()/1000)),((nTotalTicks/(PalGetSysClock()/1000))/50),gnFrames+1,PalGetSysClock());
                nTotalTicks=0;
            }
    #endif
            ChangeEndian4(&gFrame[0],frameSize);
#endif
            gnFrames++;
            
#if defined(WIN32) || defined(__CYGWIN__)
            fwrite(&gFrame[0], 1, frameSize, fmp3);
#endif

            FillWriteBuffer(frameSize);
           
            if (isSTOP()) {
                break;
            }

        }

#if defined(WIN32)    
        fclose(fmp3);
#endif

       printf("[Mp2 Enc] ClearRdBuffer gnFrames%d\n",gnFrames);

        ClearRdBuffer();
        gnFrames = 0;
        MMIO_Write(AUDIO_DECODER_START_FALG,0);
    }
}

