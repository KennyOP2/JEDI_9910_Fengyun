/***************************************************************************
 * Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
 *
 * @file
 * Codecs Startup Code
 *
 * @author Kuoping Hsu
 * @version 1.0
 *
 ***************************************************************************/
#include "aud/codecs.h"
#include "mmio.h"

char __tm_info[] = "Audio Codec Plug-Ins (c) 2008 SMedia Tech. Corp.";

// Put the *ci to the data section instead of bss section. Cause the
// bss section will clear in the codec_start function, it will
// reset the *ci to NULL.
struct _codec_api *ci __attribute__ ((section (".data"))) = NULL;

extern unsigned char codec_start_addr[];
extern unsigned char codec_end_addr[];
extern unsigned char codec_bss_start[];
extern unsigned char codec_bss_end[];

int codec_start(struct _codec_api *api);
extern int codec_info();

const struct _codec_header __header __attribute__ ((section (".codecs_header"))) = {
    CODEC_MAGIC, TARGET_ID, CODEC_API_VERSION,
    (unsigned char*)codec_start_addr, (unsigned char*)codec_end_addr,
    codec_start, codec_info
};

// startup entry
int codec_start(struct _codec_api *api)
{
    int i = 0;
    int *ptr;

    // Make sure the ci is not in .bss section. The .bss section will
    // clear later.
    ci = api;

    // Clear BSS section of CODEC. Be carefull, it will clear all of
    // global variable which un-initialized or initialize with zero's
    // variable.
    ptr = (int*)codec_bss_start;
    do {
        *ptr++ = 0;
    } while((int)ptr <= (int)codec_bss_end);

    MMIO_Write(AUDIO_DECODER_START_FALG, 1);       

    main();

    // Never return by codec main functions.
    taskSOFTWARE_BREAKPOINT();
    while(1) ;

    return 0;
}

void MMIO_Write(unsigned short addr, unsigned short data)
{
    *(volatile unsigned short *) (MMIO_ADDR + addr) = data;
}

unsigned int MMIO_Read(unsigned short addr)
{
    return *(volatile unsigned short *) (MMIO_ADDR + addr);
}

// dummy __main function, do not remove it.
void __main(void) { /* dummy */ }

