/*
 * (c) SMedia Tech. Corp. 2008
 *
 * Device Driver for Remote Controller
 *
 */

#include "ir/config.h"
#include "mmp_ir.h"
#include "mmp_dma.h"

#define HAVE_REPEAT_KEY//kenny

//=============================================================================
//                              Extern Reference
//=============================================================================
//TODO
//#ifdef __FREERTOS__
//extern MMP_UINT32 gpio_ir;
//#elif defined(__OPENRTOS__)
//extern MMP_UINT32 gpio_ir;
//#else
//MMP_UINT32 gpio_ir = 0;
//#endif

/* ------------------------------------------------------- *
* Capture Dirver Main Body
* ------------------------------------------------------- */
#define STATE_NUM (5)
#define MAX_VAL   ((1 << 10) - 1)

enum RCState {
    WAIT_START,
    WAIT_START_REPEAT_KEY,      // hiden state for repeat key
    WAIT_BIT,
    WAIT_BIT_ONE,               // hiden state for a "1" bit
    WAIT_END
};

static enum RCState    currState;
static int             receivedBitNum;
static unsigned long   receivedCode;            // LSB received code
static unsigned long   receivedCodeH;           // MSB received code if BIT_PER_KEY > 32
static int             threshold[STATE_NUM][2]; // min & max for a signal stste
static int             repeatKeyPress      = 0;
static int             repeatKeyHold       = 0;
static int             repeatKeyFast       = 0;
static int             repeatKeyCnt        = 0;

static int             duration[STATE_NUM] = {
    CONFIG_KEY_START,       // time period of normal start
    CONFIG_KEY_REPEAT,      // time period of repeat key start
    CONFIG_KEY_0,           // time period of bit0
    CONFIG_KEY_1,           // time period of bit1
    CONFIG_KEY_GAP          // time gap of each key stroke
};

static MMP_DMA_CONTEXT g_irTxDmaCtxt;
static MMP_DMA_CONTEXT g_irRxDmaCtxt;

static MMP_UINT32      irDmaReadAttrib[] =
{
    //TODO
    MMP_DMA_ATTRIB_DMA_TYPE, (MMP_UINT32)MMP_DMA_TYPE_IR_TO_MEM,
    MMP_DMA_ATTRIB_SRC_ADDR, 0xDEC0000C,                 //FOR WIN32
    MMP_DMA_ATTRIB_DST_ADDR, (MMP_UINT32)0,
    MMP_DMA_ATTRIB_TOTAL_SIZE_IN_BYTE, (MMP_UINT32)1024, //TODO
    MMP_DMA_ATTRIB_HW_HANDSHAKING, (MMP_UINT32)MMP_TRUE,
    MMP_DMA_ATTRIB_SRC_TX_WIDTH, 2,                      //TODO
    MMP_DMA_ATTRIB_SRC_BURST_SIZE, 1,
    MMP_DMA_ATTRIB_DST_TX_WIDTH, 2,
    MMP_DMA_ATTRIB_PRIORITY, 3,
    MMP_DMA_ATTRIB_NONE
};

static MMP_UINT32      irDmaWriteAttrib[] =
{
    MMP_DMA_ATTRIB_DMA_TYPE, (MMP_UINT32)MMP_DMA_TYPE_MEM_TO_IR,
    MMP_DMA_ATTRIB_SRC_ADDR, (MMP_UINT32)0,
    MMP_DMA_ATTRIB_DST_ADDR, 0xDEC00020,                 //FOR WIN32
    MMP_DMA_ATTRIB_TOTAL_SIZE_IN_BYTE, (MMP_UINT32)1024, //TODO
    MMP_DMA_ATTRIB_HW_HANDSHAKING, (MMP_UINT32)MMP_TRUE,
    MMP_DMA_ATTRIB_SRC_TX_WIDTH, 2,                      //TODO
    MMP_DMA_ATTRIB_SRC_BURST_SIZE, 1,
    MMP_DMA_ATTRIB_DST_TX_WIDTH, 2,
    MMP_DMA_ATTRIB_PRIORITY, 2,
    MMP_DMA_ATTRIB_NONE
};

//--------------------------
// Capture Init Pre Setting
//--------------------------
#define CAP_ENABLE         (0x1 << 0)
#define CAP_DISENABLE      (0xfffffffe)
#define CAP_DEBOUNCE       (0x1 << 1)
#define CAP_ENINT          (0x1 << 2)
#define CPA_PRMODE         (0x1 << 3)
#define CAP_FIFO_LEVEL     (0x1 << 4)    // D[5:4]
//#define CAP_MODE            ((0x1<<7)|(0x0<<6))    // D[7:6], Both edge
#define  CAP_MODE          ((0x0 << 6)) //kenny patch yongke ir
#define CAP_SIGINV         (0x1 << 8)
#define CAP_CLEAR          (0x1 << 9)
#define CAP_DISCLEAR       (0xfffffdff)
#ifdef __FREERTOS__
    #define CAP_OFFSETMODE (0x1 << 10)
    #define CAP_TO_NOSTALL (0x1 << 11)
#elif defined(__OPENRTOS__)
    #define CAP_OFFSETMODE (0x1 << 10)
    #define CAP_TO_NOSTALL (0x1 << 11)
#else
    #define CAP_OFFSETMODE (0x1 << 10)
    #define CAP_TO_NOSTALL (0x1 << 11)
#endif

#if defined(__OR32__)
    #define MULSHIFT(x, y, shift) ({  \
        register int ret;   \
        asm volatile ("l.mac %0,%1" : : "%r" (x), "r" (y));   \
        asm volatile ("l.macrc %0,%1" : "=r" (ret) : "i" (shift));    \
        ret;    \
    })
#else
static __inline int MULSHIFT(int x, int y, int shift)
{
    long long xext, yext;
    xext = (long long)x;  yext = (long long)y;
    xext = ((xext * yext) >> shift);
    return (int)xext;
}
#endif

#if !defined(ENABLE_TASK_LOG)
    #define SHOW_TASK_LOG()
#else
    #define SHOW_TASK_LOG()                                                                      \
    {                                                                                          \
        #define ROTATE_KEY  0xe01f6f82                                                         \
        #define POWER_KEY   0xed126f82                                                         \
        #define tskSIZE_OF_EACH_TRACE_LINE (2 * sizeof(int))                                     \
        if (completeACode) {                                                                   \
            #if (configGENERATE_RUN_TIME_STATS == 1)                                           \
                if (*code == ROTATE_KEY) {                                                         \
                    printf("Init Run Time Stats...\n");                                            \
                    vTaskInitRunTimeStats();                                                       \
                    completeACode = 0;                                                             \
                }                                                                                  \
            if (*code == POWER_KEY) {                                                          \
                vTaskGetRunTimeStats((char *)0);                                                \
                completeACode = 0;                                                             \
            }                                                                                  \
            #endif // (configGENERATE_RUN_TIME_STATS == 1)                                     \
                                                                                               \
            #if (configUSE_TRACE_FACILITY == 1)                                                \
            #define NTRACE (16384)                                                             \
            static char *pcBuffer = (char*)0;                                                  \
            if (*code == ROTATE_KEY) {                                                         \
                printf("Init Task Trace...\n");                                                \
                if (pcBuffer == (char*)0) {                                                    \
                    pcBuffer = PalHeapAlloc(0, NTRACE*tskSIZE_OF_EACH_TRACE_LINE);             \
                }                                                                              \
                vTaskStartTrace(pcBuffer, NTRACE*tskSIZE_OF_EACH_TRACE_LINE);                  \
                completeACode = 0;                                                             \
            }                                                                                  \
            if (*code == POWER_KEY) {                                                          \
                unsigned int ulTaskEventLog = ulTaskEndTrace() / tskSIZE_OF_EACH_TRACE_LINE;   \
                vTaskList((char*)0);                                                           \
                printf("Log %d tasks event\n\n", ulTaskEventLog);                              \
                if (pcBuffer != (char*)0) PalHeapFree(0, pcBuffer);                            \
                pcBuffer = (char*)0;                                                           \
                completeACode = 0;                                                             \
            }                                                                                  \
            #endif // (configUSE_TRACE_FACILITY == 1)                                          \
        }                                                                                      \
    }
#endif // !defined(ENABLE_TASK_LOG)

/* --------------------------------------------------------------- *
* Set Capture Interrupt
* Argument :
*           bDI : Data Ready Interrupt
*           bFI : FIFO Full Interrupt
*           bEI : FIFO Empty Interrupt
*           bOI : FIFO Overwrite Interrupt
*           bTi : Sample Counter Timeout Interrupt
* Return   :
*
* --------------------------------------------------------------- */
void CapSetINT(int bDI, int bFI, int bEI, int bOI, int TI)
{
    int iINT;
    iINT = (TI << 12) | (bOI << 11) | (bEI << 10) | (bFI << 9) | (bDI << 8);

    // Rx Set HW Register
    AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_CSR_REG, iINT);

    // Tx Set HW Register
    AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SSR_REG, iINT);
}

/* --------------------------------------------------------------- *
* Set Capture Control Registers
* Argument :
* Return   :
*
* --------------------------------------------------------------- */
void CapInit(MMP_UINT32 mode)
{
    int        iCtrlReg;
    int        iPreScalCnt;
    //    MMP_UINT32 ccrReg = 0;
    //    MMP_UINT32 cprReg = 0;
    MMP_UINT16 outdiv;
    MMP_UINT16 cfg;
    MMP_UINT32 busClk;
    //MMP_UINT32 crystal[] = {12000000, 16380000, 20480000, 26000000};
#ifdef WIN32
    //TODO
    busClk = 81000000;
#else
    HOST_ReadRegister(MMP_GENERAL_CONFIG_REG_00, &cfg);
    cfg    = (cfg >> 7) & 0x3;
    HOST_ReadRegister(MMP_APB_CLOCK_REG_1C, &outdiv);
    outdiv = (outdiv & MMP_APB_RAT_WCLK) + 4;

    //busClk = mode ? ((crystal[cfg]/outdiv) << 2) : ithGetBusClock();
    #if defined(__OPENRTOS__)
    busClk = mode ? ((12000000 / outdiv) << 2) : ithGetBusClock();
    #elif defined(__FREERTOS__)
    busClk = mode ? ((12000000 / outdiv) << 2) : or32_getBusCLK();
    #endif
#endif

    iCtrlReg    = CAP_MODE | CAP_OFFSETMODE | CAP_TO_NOSTALL | CAP_SIGINV;
    /*
       #ifdef __FREERTOS__
        iPreScalCnt = (int) MULSHIFT(SAMP_DUR, busClk, PRECISION) / 1000 - 1;
       #else
        iPreScalCnt = (int) MULSHIFT(SAMP_DUR, 80 * 1024 * 1024, PRECISION) / 1000 - 1;
        //iPreScalCnt = (int) MULSHIFT(SAMP_DUR, 300 * 1024 * 1024, PRECISION) / 1000 - 1; // for test
       #endif
     */
    //kenny patch yongke ir
    iPreScalCnt = (int) MULSHIFT(SAMP_DUR, busClk, PRECISION) / 1000 - 1;
    printf("iPreScalCnt = %d\n", iPreScalCnt);
    printf("busClk = %d\n", busClk);
    // Set HW Register
    AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_CCR_REG, iCtrlReg);
    AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_CPR_REG, iPreScalCnt);

    //Tx
    //AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SCR_REG, 0x1);
    AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SPR_REG, iPreScalCnt);
    AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SMR_REG, 0x00068839);//0x8AE 36k,0x839 38k

    //CapSetINT(0, 0, 0, 0, 0);
    CapSetINT(1, 1, 1, 1, 1); //intr
}

/* --------------------------------------------------------------- *
* Capture Get Signal
* Argument :
*
* Return   :
*           Sample Cnt Signal
* --------------------------------------------------------------- */
int CapGetSignal(void)
{
    int iStatReg;
    /*
       #ifdef __FREERTOS__
       MMP_UINT32 iCode;
       #else
       MMP_UINT16 iCode;
       #endif
     */
#ifdef WIN32
    MMP_UINT16 iCode;
#else
    MMP_UINT32 iCode;
#endif
    //    int iFIFOLen;
    MMP_UINT32 ctrlReg = 0;
    //	MMP_UINT32 iEmpty = 0;

    // Read HW Register
    AHB_ReadRegister(SIGNAL_CAP_BASE + CAP_CSR_REG, (MMP_UINT32 *) &iStatReg);
    AHB_ReadRegister(SIGNAL_CAP_BASE + CAP_CCR_REG, &ctrlReg);
    //printf("0x%x 0x%x\n", SIGNAL_CAP_BASE, iStatReg);
    if ((iStatReg & 0x04) == 0)
    {
        /*
           #ifdef __FREERTOS__
           AHB_ReadRegister(SIGNAL_CAP_BASE + CAP_CAP_REG, (MMP_UINT32*) &iCode);
           #else
           HOST_ReadRegister(SIGNAL_CAP_BASE + CAP_CAP_REG, &iCode);
           #endif
         */
#ifdef WIN32
        HOST_ReadRegister(SIGNAL_CAP_BASE + CAP_CAP_REG, &iCode);
#else
        AHB_ReadRegister(SIGNAL_CAP_BASE + CAP_CAP_REG, (MMP_UINT32 *) &iCode);
#endif
        //if (iCode>=0x121)
        //    iCode = 0x290;
        //else if (iCode>=0x7c)
        //    iCode = 0x87;
        //else if (iCode>=0x14)
        //    iCode = 0xb;
        //else if (iCode>=0xa)
        //    iCode = 0x16;

        //  printf("iCode == 0x%x\n", iCode);

        //set Tx
        //AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, iCode);

        LOG_DEBUG "(%d) iCode = %d\n", receivedBitNum, iCode LOG_END
    }
    else
    {
        iCode = 0x0;
    }

#ifndef WIN32
    if (iStatReg & (1 << 3)) // Overrun
    {
        printf("!!WARNING!! Remote IR code overrun, it will lost key!\n");
    }
#endif

    return (int) iCode;
}

static int getRCSignal(void)
{
    return CapGetSignal();
}

/* --------------------------------------------------------------- *
* Capture Fire
* Argument :
*           bFire : '1' Fire the capture; '0' Disable the capture
* Return   :
*
* --------------------------------------------------------------- */
void CapFire(int bFire)
{
    int iCtrlReg;

    // Read HW Register
    AHB_ReadRegister(SIGNAL_CAP_BASE + CAP_CCR_REG, (MMP_UINT32 *) &iCtrlReg);

    // Set HW Register
    if (bFire)
    {
        iCtrlReg = (iCtrlReg & CAP_DISCLEAR) | CAP_ENABLE | CAP_ENINT;
    }
    else
    {
        iCtrlReg = (iCtrlReg & CAP_DISENABLE) | CAP_CLEAR;
    }

    AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_CCR_REG, iCtrlReg);

    //Tx
    if (bFire)
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SCR_REG, CAP_ENABLE | (0x0 << 8) | CAP_ENINT);
    else
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SCR_REG, 0x200);
}

void initRemoteControl(MMP_UINT32 mode)
{
    int f1, f2;
    int i;

    CapInit(mode);

    // know the sampling duration
    // set the min & max for each state
    for (i = 0; i < STATE_NUM; i++)
    {
        // 0.92 & 1.08 is experienc value, duration range N is between 0.92*N and 1.08*N.
        f1 = ((duration[i]) << 1) / SAMP_DUR;
        f2 = MULSHIFT(f1, IR_TIME(0.92), PRECISION) >> 1;

        if (f2 >= MAX_VAL)
        {
            LOG_WARNING "Out of counter resolution!!\n" LOG_END
            //            asm volatile("l.trap 15");
        }

        threshold[i][0] = (f2 < 1) ? 1 : f2; // f1 * 0.92
        threshold[i][1] = (i == STATE_NUM - 1) ?
                          MAX_VAL :
                          ((MULSHIFT(f1, IR_TIME(1.08), PRECISION) + 1) >> 1); // f1 * 1.08 + 0.5

        printf("(%d), threshold[%d][0] = 0x%x, threshold[%d][1] = 0x%x\n",
               __LINE__, i, threshold[i][0], i, threshold[i][1]);
        LOG_DEBUG "threshold[%d][0] = 0x%d, threshold[%d][1] = 0x%x\n",
        i, threshold[i][0], i, threshold[i][1] LOG_END
    }
}

void resetRemoteControl(void)
{
    currState      = WAIT_START;
    receivedCode   = receivedCodeH = receivedBitNum = 0;
    repeatKeyPress = repeatKeyHold = 0;
    repeatKeyFast  = repeatKeyCnt = 0;

    CapFire(0);
    CapFire(1);
}

int getRemoteControlCode(unsigned long *code)
{
    int           completeACode, signal;
    unsigned long getBit;

    completeACode = 0;

    while (((signal = getRCSignal()) != 0) && !completeACode)
    {
        //printf("signal == %d\n", signal);
        switch (currState)
        {
        case WAIT_START:
            if ((signal >= threshold[WAIT_START][0]) &&
                (signal <= threshold[WAIT_START][1]))
            {
                currState = WAIT_BIT;
            }
            else if ((signal >= threshold[WAIT_START_REPEAT_KEY][0]) &&
                     (signal <= threshold[WAIT_START_REPEAT_KEY][1]))
            {
                *code = receivedCode;

#if defined(HAVE_REPEAT_KEY)
                if (1)   // (receivedCode) {
                {
                    if (repeatKeyPress >= REPEAT_THRESHOLD_BEGIN)
                    {
                        LOG_DEBUG "Hold\n" LOG_END
                        if ((repeatKeyFast == 0 && repeatKeyHold >= REPEAT_THRESHOLD_HOLD1) ||
                            (repeatKeyFast == 1 && repeatKeyHold >= REPEAT_THRESHOLD_HOLD2))
                        {
                            LOG_DEBUG "Send repeat key (%08x)\n", receivedCode LOG_END
                            completeACode = 1;
                            repeatKeyHold = 0;
                            if (repeatKeyCnt >= REPEAT_THRESHOLD_SPDUP)
                            {
                                repeatKeyFast = 1;
                            }
                            else
                            {
                                repeatKeyCnt++;
                            }
                        }
                        else
                        {
                            repeatKeyHold++;
                        }
                    }
                    else
                    {
                        repeatKeyPress++;
                    }
                }
#endif                 // HAVE_REPEAT_KEY

                currState = WAIT_END;
            }
            else                // error
            {
                currState = WAIT_START;
            }
            break;

        case WAIT_BIT:
            repeatKeyPress = repeatKeyHold = 0;
            repeatKeyFast  = repeatKeyCnt = 0;
            if ((signal >= threshold[WAIT_BIT][0]) &&
                (signal <= threshold[WAIT_BIT][1]))
            {
                // bit "0"
                getBit = 0;
            }
            else if ((signal >= threshold[WAIT_BIT_ONE][0]) &&
                     (signal <= threshold[WAIT_BIT_ONE][1]))
            {
                // bit "1"
#if CONFIG_LSB
                getBit = 0x80000000L;
#else
                getBit = 0x00000001L;
#endif
            }
            else                // error
            {
                receivedCode = receivedCodeH = receivedBitNum = 0;
                currState    = WAIT_START;
                continue;
            }

#if CONFIG_LSB
            receivedCodeH = (receivedCodeH >> 1) | ((receivedCode & 1) << 31);
            receivedCode  = (receivedCode >> 1) | getBit;
#else
            receivedCodeH = (receivedCodeH << 1) | (receivedCode & 0x80000000L);
            receivedCode  = (receivedCode << 1) | getBit;
#endif

            receivedBitNum++;
            if (receivedBitNum < BIT_PER_KEY)
                currState = WAIT_BIT;   // not yet complet a code
            else
            {
                completeACode = 1;
                *code         = receivedCode;
                currState     = WAIT_END;
            }
            break;

        case WAIT_END:
            if ((signal >= threshold[WAIT_END][0]) &&
                (signal <= threshold[WAIT_END][1]))
            {
                receivedBitNum = 0;
                completeACode  = 0;
                currState      = WAIT_START;
            }
            else                // error
            {   // receivedCode = receivedCodeH = 0;
                receivedBitNum = 0;
                repeatKeyPress = repeatKeyHold = 0;
                repeatKeyFast  = repeatKeyCnt = 0;
                completeACode  = 0;
                currState      = WAIT_START;
            }
            break;

        default:
            break;
        }
    }

    SHOW_TASK_LOG();

    if (completeACode)
    {
        return 1;
    }
    else
    {
        *code = 0;
        return 0;
    }
}

MMP_RESULT mmpIrInitialize(MMP_INT32 pllBypass)
{
    MMP_RESULT result    = MMP_SUCCESS;
    MMP_UINT32 padSelReg = 0;
    MMP_UINT32 pinDirReg = 0;

    LOG_ENTER "[mmpIrInitialize] Enter\n" LOG_END

    // Select DGPIO33(Rx) DGPIO32(Tx) pin
    AHB_WriteRegisterMask(GPIO_BASE + GPIO_PADSEL_REG_3, (1 << 1) | (1 << 3), (1 << 1) | (1 << 3));
    AHB_WriteRegisterMask(GPIO_BASE + GPIO_PADSEL_REG_3, (0 << 0) | (0 << 2), (1 << 0) | (1 << 2));

    initRemoteControl(pllBypass);
    resetRemoteControl();

#ifdef WIN32
    //dma
    result = mmpDmaCreateContext(&g_irTxDmaCtxt);
    if (result)
        printf("Create DMA Tx fail\n");

    result = mmpDmaCreateContext(&g_irRxDmaCtxt);
    if (result)
        printf("Create DMA Rx fail\n");
#endif

    LOG_LEAVE "[mmpIrInitialize] Leave\n" LOG_END

    return result;
}

MMP_RESULT mmpIrTerminate(void)
{
    LOG_ENTER "[mmpIrTerminate] Enter\n" LOG_END

    //    MMP_RESULT result = MMP_SUCCESS;

    LOG_LEAVE "[mmpIrTerminate] Leave\n" LOG_END

    return MMP_RESULT_SUCCESS;
}

MMP_UINT32 mmpIrGetKey(void)
{
    MMP_INT    keyPressed;
    MMP_UINT32 code = 0;
    //    MMP_UINT i = 0;

    LOG_ENTER "[mmpIrGetKey] Enter\n" LOG_END
    keyPressed = getRemoteControlCode(&code);
    //printf("key == 0x%x\n", code);
    LOG_LEAVE "[mmpIrGetKey] Leave\n" LOG_END

    if (keyPressed)
    {
        return code;
    }

    return 0;
}

MMP_UINT32 mmpIrGetKeyH(void)
{
    return receivedCodeH;
}

MMP_RESULT
mmpIrDmaRead(
    void *pdes,
    MMP_INT size)
{
    MMP_RESULT result = -1;
    //MMP_UINT32 temp;
    //MMP_UINT32 i;
    //MMP_UINT8 dataLen = 7;

    if (g_irRxDmaCtxt)
    {
        //TX channel
        irDmaReadAttrib[1]  = MMP_DMA_TYPE_IR_TO_MEM;
        irDmaReadAttrib[5]  = (MMP_UINT32)pdes;           /** dest address */
        irDmaReadAttrib[7]  = size;                       /** total size */
        irDmaReadAttrib[9]  = MMP_TRUE;                   /** handshaking */
        irDmaReadAttrib[11] = 2;                          /** source width */
        irDmaReadAttrib[13] = 1;                          /** burst size */
    }
    //g_SpiContext.tx_threshold = 4;
    //g_SpiContext.rx_threshold = 4; //need sync to burst size

    result = mmpDmaSetAttrib(g_irRxDmaCtxt, irDmaReadAttrib);
    if (result)
        goto end;

    result = mmpDmaFire(g_irRxDmaCtxt);
    if (result)
        goto end;

    //WriteReg(REG_SSP_CONTROL_1, g_SpiContext.def_Div | (dataLen << REG_SHIFT_SERIAL_DATA_LEN));

    ////SSP dma enable
    //WriteReg(REG_SSP_INTR_CONTROL, ((g_SpiContext.tx_threshold << REG_SHIFT_TX_THRESHOLD) |
    //                                (g_SpiContext.rx_threshold << REG_SHIFT_RX_THRESHOLD) |
    //                                REG_BIT_RX_DMA_EN));

    ////Fire SSP
    //WriteReg(REG_SSP_CONTROL_2, REG_BIT_SSP_EN | REG_BIT_TXDO_EN);
    ////dbg_msg(DBG_MSG_TYPE_SPI, "[SPI]%d ms \n", PalGetDuration(lastClock));

end:
    return result;
}

MMP_RESULT
mmpIrDmaWrite(
    MMP_UINT8 *psrc,
    MMP_INT size)
{
    MMP_RESULT result  = -1;
    MMP_UINT32 temp;
    MMP_UINT32 i;
    MMP_UINT8  dataLen = 7;

    if (g_irTxDmaCtxt)
    {
        //TX channel
        irDmaWriteAttrib[1]  = MMP_DMA_TYPE_MEM_TO_IR;
        irDmaWriteAttrib[3]  = (MMP_UINT32)psrc;           /** source address */
        irDmaWriteAttrib[7]  = size;                       /** total size */
        irDmaWriteAttrib[9]  = MMP_TRUE;                   /** handshaking */
        irDmaWriteAttrib[11] = 2;                          /** source width */
        irDmaWriteAttrib[13] = 1;                          /** burst size */
    }
    //g_SpiContext.tx_threshold = 4;
    //g_SpiContext.rx_threshold = 1;

    result = mmpDmaSetAttrib(g_irTxDmaCtxt, irDmaWriteAttrib);
    if (result)
        goto end;
    result = mmpDmaFire(g_irTxDmaCtxt);
    if (result)
        goto end;

    //WriteReg(REG_SSP_CONTROL_1, g_SpiContext.def_Div | (dataLen << REG_SHIFT_SERIAL_DATA_LEN));

    ////SSP dma enable
    //WriteReg(REG_SSP_INTR_CONTROL, ((g_SpiContext.tx_threshold << REG_SHIFT_TX_THRESHOLD) |
    //                                (g_SpiContext.rx_threshold << REG_SHIFT_RX_THRESHOLD) |
    //                                REG_BIT_TX_DMA_EN));

    ////Fire SSP
    //WriteReg(REG_SSP_CONTROL_2, REG_BIT_SSP_EN | REG_BIT_TXDO_EN);

end:
    return result;
}

MMP_RESULT
mmpIrDmaWaitIdle(
    void)
{
    MMP_RESULT result = -1;

    result = mmpDmaWaitIdle(g_irTxDmaCtxt);

    return result;
}
