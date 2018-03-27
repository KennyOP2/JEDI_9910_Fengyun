/*
 * Copyright (c) 2010 ITE Technology Corp. All Rights Reserved.
 */
/** @stat
 * Task Statistics VCD Dump.
 *
 * To view the VCD file, please download Wave VCD Viewer from
 * http://www.iss-us.com/wavevcd/index.htm.
 *
 * Insert following function call to start trace the task context
 * switch event.
 *
 * PalTaskDumpOpen();
 *
 * And insert following code to the main task to dump the VCD file
 * when the trace complete.
 *
 * if (PalTaskDumpReady())
 *     PalTaskDumpWrite();
 *
 * @author Kuoping Hsu
 * @version 1.0
 */

#include "FreeRTOS.h"
#include "task.h"
#include "pal/pal.h"

#if ( configUSE_DUMP_FACILITY == 1 )

// Enable the define to measure memory bandwidth usage.
// This feature only avaliable on P680 IC.
#define MEMORY_BANDWIDTH_ANALYSIS
#define SAMPLE_PERIOD   2       // memory bandwidth sample period in milliseconds.

#ifdef BUILDING_BOOTLOADER

void portTASK_SWITCHED_IN(void) { }
void portTASK_SWITCHED_OUT(void) { }
void portTASK_DELAY(void) { }

#else // BUILDING_BOOTLOADER

#define MODE_MASK           (0xf0000000)
#define MODE_TASK_IN        (0x10000000)
#define MODE_TASK_OUT       (0x20000000)
#define MODE_TASK_DELAY     (0x30000000)
#define MODE_MEM_USAGE      (0x40000000)

extern signed portCHAR *    pxTCBName[];

/***************************************************************************
 *                              Private Variable
 ***************************************************************************/
struct _clock {
    unsigned sec;
    unsigned ms;
    unsigned us;
};

typedef struct _clock * _CLOCK_T;

static unsigned int * g_trace_buf  = (unsigned int*)0;
static unsigned int * g_buf_ptr    = 0;
static unsigned int   g_buf_idx    = 0;
static unsigned int   g_buf_size   = 0;
static unsigned int   g_enable     = 0;
static unsigned int   g_dump       = 0;
static MMP_WCHAR    * g_fname      = (MMP_WCHAR*)0;
static _CLOCK_T       g_start_time = (_CLOCK_T)0;

/***************************************************************************
 *                              Private Functions
 ***************************************************************************/
static _CLOCK_T
getClock(
    void)
{
    static struct _clock t; // Only for non-preemptive OS
    xTaskGetTime(&t.sec, &t.ms, &t.us);
    return &t;
}

static unsigned
getDuration(
    _CLOCK_T clock)
{
    struct _clock t;

    xTaskGetTime(&t.sec, &t.ms, &t.us);

    t.sec -= clock->sec;

    if (t.ms < clock->ms)
    {
        t.sec--;
        t.ms = 1000 + t.ms - clock->ms;
    }
    else
    {
        t.ms -= clock->ms;
    }

    if (t.us < clock->us)
    {
        t.ms--;
        t.us = 1000 + t.us - clock->us;
    }
    else
    {
        t.us -= clock->us;
    }

    return (t.sec * 1000 * 1000 + t.ms * 1000 + t.us);
}

static char *
dec2bin(int n)
{
    static char bin[33];
    int i;
    for(i=0; i<32; i++)
    {
        bin[i] = (n&0x80000000) ? '1' : '0';
        n <<= 1;
    }
    bin[32] = 0;
    return bin;
}

static
vcd_dump(void)
{
    #define TIMESCALE 1
    int i;
    int max_id = vGetMaxTaskID();
    PAL_FILE* fp;
    char buf[80];

    g_enable = 0;
    if (!g_fname)
    {
        printf("[RTOS][DUMP] No dump name is specified!!\n");
        goto end;
    }

    if ((fp = PalWFileOpen(g_fname, PAL_FILE_WBP, NULL)) == NULL)
    {
        printf("[RTOS][DUMP] Can not create dump file!!\n");
        goto end;
    }

    if (!g_buf_idx)
    {
        printf("[RTOS][DUMP] No data to dump!!\n");
        goto end;
    }

    //vTaskSuspendAll();
    printf("[RTOS][DUMP] Starting dump....\n");

    if (max_id > (int)('~' - '!' + 1))
    {
        max_id = (int)('~' - '!' + 1);
    }

    g_buf_ptr = g_trace_buf;

    snprintf(buf, sizeof(buf), "$version\n");                          PalFileWrite(buf, 1, strlen(buf), fp, NULL);
    snprintf(buf, sizeof(buf), "   OpenRTOS Context Switch dump.\n");  PalFileWrite(buf, 1, strlen(buf), fp, NULL);
    snprintf(buf, sizeof(buf), "$end\n");                              PalFileWrite(buf, 1, strlen(buf), fp, NULL);
    snprintf(buf, sizeof(buf), "$comment\n");                          PalFileWrite(buf, 1, strlen(buf), fp, NULL);
    snprintf(buf, sizeof(buf), "   ITE Tech. Corp. by Kuoping Hsu, Dec. 2010\n"); PalFileWrite(buf, 1, strlen(buf), fp, NULL);
    snprintf(buf, sizeof(buf), "$end\n");                              PalFileWrite(buf, 1, strlen(buf), fp, NULL);
    snprintf(buf, sizeof(buf), "$timescale %dus $end\n", TIMESCALE);   PalFileWrite(buf, 1, strlen(buf), fp, NULL);
    snprintf(buf, sizeof(buf), "$scope module task $end\n");           PalFileWrite(buf, 1, strlen(buf), fp, NULL);

    for(i=0; i<max_id; i++)
    {
        snprintf(buf, sizeof(buf), "$var wire 1 %c %s $end\n", '#'+i, pxTCBName[i]);
        PalFileWrite(buf, 1, strlen(buf), fp, NULL);
    }

    #if defined(MEMORY_BANDWIDTH_ANALYSIS)
    snprintf(buf, sizeof(buf), "$var wire 32 %c %s $end\n", '#'+i, "mem_usage");
    PalFileWrite(buf, 1, strlen(buf), fp, NULL);
    #endif // MEMORY_BANDWIDTH_ANALYSIS

    snprintf(buf, sizeof(buf), "$upscope $end\n");         PalFileWrite(buf, 1, strlen(buf), fp, NULL);
    snprintf(buf, sizeof(buf), "$enddefinitions $end\n");  PalFileWrite(buf, 1, strlen(buf), fp, NULL);
    snprintf(buf, sizeof(buf), "$dumpvars\n");             PalFileWrite(buf, 1, strlen(buf), fp, NULL);

    for(i=0; i<max_id; i++)
    {
        snprintf(buf, sizeof(buf), "x%c\n", '#'+i);
        PalFileWrite(buf, 1, strlen(buf), fp, NULL);
    }

    #if defined(MEMORY_BANDWIDTH_ANALYSIS)
    snprintf(buf, sizeof(buf), "bxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx %c\n", '#'+i);
    PalFileWrite(buf, 1, strlen(buf), fp, NULL);
    #endif // MEMORY_BANDWIDTH_ANALYSIS

    snprintf(buf, sizeof(buf), "$end\n");
    PalFileWrite(buf, 1, strlen(buf), fp, NULL);

    for(i=0; i<g_buf_idx*2; i+=2)
    {
        char signal = (char)('#'+(g_buf_ptr[i] & ~MODE_MASK));

        if (((g_buf_ptr[i] & MODE_MASK) != MODE_TASK_IN)    &&
            ((g_buf_ptr[i] & MODE_MASK) != MODE_TASK_OUT)   &&
            ((g_buf_ptr[i] & MODE_MASK) != MODE_TASK_DELAY) &&
            ((g_buf_ptr[i] & MODE_MASK) != MODE_MEM_USAGE))
        {
            printf("[RTOS][DUMP] unknown task stat 0x%08x.\n", g_buf_ptr[i]);
            continue;
        }

        snprintf(buf, sizeof(buf), "#%d\n", g_buf_ptr[i+1] / TIMESCALE);
        PalFileWrite(buf, 1, strlen(buf), fp, NULL);

        if ((g_buf_ptr[i] & MODE_MASK) == MODE_TASK_IN)
        {
            snprintf(buf, sizeof(buf), "1%c\n", signal);
            PalFileWrite(buf, 1, strlen(buf), fp, NULL);
        }
        else if ((g_buf_ptr[i] & MODE_MASK) == MODE_TASK_OUT)
        {
            snprintf(buf, sizeof(buf), "0%c\n", signal);
            PalFileWrite(buf, 1, strlen(buf), fp, NULL);
        }
        else if ((g_buf_ptr[i] & MODE_MASK) == MODE_TASK_DELAY)
        {
            snprintf(buf, sizeof(buf), "x%c\n", signal);
            PalFileWrite(buf, 1, strlen(buf), fp, NULL);
        }
        #if defined(MEMORY_BANDWIDTH_ANALYSIS)
        else if ((g_buf_ptr[i] & MODE_MASK) == MODE_MEM_USAGE)
        {
            snprintf(buf, sizeof(buf), "b%s %c\n", dec2bin((g_buf_ptr[i] & ~MODE_MASK)*1000/SAMPLE_PERIOD), (int)('#'+max_id));
            PalFileWrite(buf, 1, strlen(buf), fp, NULL);
        }
        #endif // MEMORY_BANDWIDTH_ANALYSIS
    }

    //xTaskResumeAll();
    printf("[RTOS][DUMP] Dump complete %d event....\n", g_buf_idx);

    /* stop trace after dump the trace */
end:
    if (fp)
    {
        PalFileClose(fp, NULL);
    }

    PalTaskDumpClose();
}

static void
mem_trace(void)
{
    #if defined(MEMORY_BANDWIDTH_ANALYSIS)
    static unsigned int * ptr = 0;

    if (g_buf_idx >= g_buf_size)
    {
        g_dump   = 1;
        g_enable = 0;
    }
    else
    {
        MMP_UINT16 regdata;
        HOST_ReadRegister(0x03e8, &regdata);
        if ((regdata & (1 << 12)) != 0)
        {
            MMP_UINT16 regdata1, regdata2;
            HOST_ReadRegister(0x03ea, &regdata1);
            HOST_ReadRegister(0x03ec, &regdata2);
            *ptr         = (regdata2 << 16) | regdata1 | MODE_MEM_USAGE;
            ptr          = g_buf_ptr;
            *g_buf_ptr++ = MODE_MEM_USAGE;
            *g_buf_ptr++ = getDuration(g_start_time);
            g_buf_idx++;

            HOST_WriteRegister(0x03e8, 0);
            HOST_WriteRegister(0x03e8, (1<<15));
        }
        else if ((regdata & (1 << 15)) == 0)
        {
            HOST_WriteRegister(0x03e8, (1<<15));
            HOST_WriteRegister(0x03e6, PalGetMemClock()*SAMPLE_PERIOD/65536/1000-1); // sample period

            ptr          = g_buf_ptr;
            *g_buf_ptr++ = MODE_MEM_USAGE;
            *g_buf_ptr++ = getDuration(g_start_time);
            g_buf_idx++;
        }
    }
    #endif // MEMORY_BANDWIDTH_ANALYSIS
}

/***************************************************************************
 *                              Public Functions
 ***************************************************************************/
void portTASK_SWITCHED_IN(void)
{
    if (g_trace_buf && g_enable)
    {
        if (g_buf_idx >= g_buf_size)
        {
            g_dump   = 1;
            g_enable = 0;
        }
        else
        {
            *g_buf_ptr++ = vGetCurrentTaskID() | MODE_TASK_IN;
            *g_buf_ptr++ = getDuration(g_start_time);
            g_buf_idx++;
        }

        mem_trace();
    }
}

void portTASK_SWITCHED_OUT(void)
{
    if (g_trace_buf && g_enable)
    {
        if (g_buf_idx >= g_buf_size)
        {
            g_dump   = 1;
            g_enable = 0;
        }
        else
        {
            *g_buf_ptr++ = vGetCurrentTaskID() | MODE_TASK_OUT;
            *g_buf_ptr++ = getDuration(g_start_time);
            g_buf_idx++;
        }
    }
}

void portTASK_DELAY(void)
{
    if (g_trace_buf && g_enable)
    {
        if (g_buf_idx >= g_buf_size)
        {
            g_dump   = 1;
            g_enable = 0;
        }
        else
        {
            *g_buf_ptr++ = vGetCurrentTaskID() | MODE_TASK_DELAY;
            *g_buf_ptr++ = getDuration(g_start_time);
            g_buf_idx++;
        }
    }
}

void PalTaskDumpOpen(MMP_WCHAR* filename, int size)
{
    g_buf_size   = size;
    g_buf_idx    = 0;
    g_start_time = getClock();
    g_fname      = filename;

    // Allocate Trace Buffer
    if (!g_trace_buf)
    {
        g_trace_buf = (unsigned int*)PalHeapAlloc(PAL_HEAP_DEFAULT, size*sizeof(unsigned int)*2);
    }

    if (g_trace_buf == (unsigned int*)0)
    {
        printf("[RTOS][DUMP] Can not creat trace buffer\n");
    }
    g_buf_ptr = g_trace_buf;
    g_enable  = 1;
    g_dump    = 0;
}

MMP_UINT32 PalTaskDumpReady(void)
{
    return (MMP_UINT32)g_dump;
}

void PalTaskDumpWrite(void)
{
    vcd_dump();
}

void PalTaskDumpClose(void)
{
    PalHeapFree(PAL_HEAP_DEFAULT, (void*)g_trace_buf);
    g_trace_buf = (unsigned int*)0;
    g_buf_idx   = 0;
    g_enable    = 0;
    g_dump      = 0;
    g_buf_ptr   = g_trace_buf;
}

#endif // BUILDING_BOOTLOADER
#endif // configUSE_DUMP_FACILITY
