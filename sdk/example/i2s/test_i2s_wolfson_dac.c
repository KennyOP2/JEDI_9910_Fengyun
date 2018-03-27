/* sy.chuang, 2012-0910, ITE Tech. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mmp_i2s.h"

#define SRATE 48000
#define BIG_ENDIAN  0
#define SAMPLE_SIZE 16

#define USE_HDMIRX      0
#define INTERNAL_HDMIRX 0
#define I2S_SLAVE_MODE  0

#if (SRATE == 48000)
#include "dancing_48000.inc"
//#	include "LPCM_F32_BE_STEREO_48000.inc" //X
//#	include "LPCM_F32_LE_STEREO_48000.inc" //X
//#	include "LPCM_S16_BE_STEREO_48000.inc" //O
//#	include "LPCM_S16_LE_STEREO_48000.inc" //O
//#	include "LPCM_S24_BE_STEREO_48000.inc" //O, config to 'S32BE'
//#	include "LPCM_S24_LE_STEREO_48000.inc" //O, config to 'S32LE'
//#	include "LPCM_S32_BE_STEREO_48000.inc" //O
//#	include "LPCM_S32_LE_STEREO_48000.inc" //O
//#	include "LPCM__S8_EX_STEREO_48000.inc" //O, config to 'S16LE'
#elif (SRATE == 44100)
	#include "i_believe_44100.inc"
#elif (SRATE == 32000)
	#include "NotComingHome_32000.inc"
#elif (SRATE == 22050)
	#include "VK_22050.inc"
#elif (SRATE == 16000)
	#include "Nocturne_16000.inc"
#elif (SRATE == 11025)
	#include "maple_11025.inc"
#elif (SRATE == 8000)
	#include "NoNeedToKnowYou_8000.inc"
#else
	#error "INVALID SRATE !"
#endif

#define FC_MALLOC(ptr, size) \
do { \
	(ptr) = (void*)memalign(2048, (size)); \
	if((ptr) == NULL) SERR(); \
	if((unsigned)(ptr) % 2048) SERR(); \
} while(0)

#define SERR() do { printf("ERROR# %s:%d, %s\n", __FILE__, __LINE__, __func__); while(1); } while(0)

/* ========================================================================= */
/* ./sdk/src/mmp/host.c */
void HOST_ReadRegister(unsigned short destAddress, unsigned short* data);
void HOST_WriteRegister(unsigned short destAddress, unsigned short data);

/* wrapper */
static inline unsigned short reg_read16(unsigned short addr16)
{
	unsigned short data16;
	HOST_ReadRegister(addr16, &data16);
	return data16;
}
/* ========================================================================= */

static void dump_i2s_reg(void)
{
	unsigned short addr16;
	unsigned short data16;

	for(addr16=0x1600; addr16<=0x167E; addr16+=2)
	{
		data16 = reg_read16(addr16);
		printf("REG[0x%04x] -> 0x%04x\n", addr16, data16);
	}
}

void i2s_main(void)
{
	unsigned DA_r;
	unsigned DA_w;

	unsigned       buf_out_i2s_size;
	unsigned char *buf_out_i2s;

	/* prepare output DA buffer */
	{
		buf_out_i2s_size = AUDIOBINSize - (AUDIOBINSize % 64);
		FC_MALLOC(buf_out_i2s, buf_out_i2s_size);

		memcpy((void*)buf_out_i2s, (void*)AUDIOBIN, buf_out_i2s_size);

		printf("buf_out_i2s: 0x%08x\n", (unsigned)buf_out_i2s);
		printf("buf_out_i2s_size: 0x%08x = %u (bytes)\n", buf_out_i2s_size, buf_out_i2s_size);
	}

	/* init I2S */
	{
		STRC_I2S_SPEC spec;
		memset((void*)&spec, 0, sizeof(STRC_I2S_SPEC));

		spec.use_hdmirx         = USE_HDMIRX;
		spec.internal_hdmirx    = INTERNAL_HDMIRX;
		spec.slave_mode         = I2S_SLAVE_MODE;
		spec.channels           = 2;
		spec.sample_rate        = SRATE;
		spec.buffer_size        = buf_out_i2s_size;
		spec.is_big_endian      = BIG_ENDIAN;
		spec.sample_size        = SAMPLE_SIZE;

		spec.base_out_i2s_spdif     = buf_out_i2s;
		spec.postpone_audio_output  = 0;

		i2s_init_DAC(&spec);
	}
	I2S_DA32_SET_WP(buf_out_i2s_size);

	dump_i2s_reg();

	while(1)
	{
		DA_r = I2S_DA32_GET_RP();
		DA_w = I2S_DA32_GET_WP();

		printf("DA_r:0x%08x, DA_w:0x%08x\t\t\t\r", DA_r, DA_w);
	}
	printf("\n");

	printf("STOP HERE, %s:%d\n", __func__, __LINE__);
	while(1) { __asm__(""); }

	i2s_deinit_DAC();

	/* destroy output DA buffer */
	free(buf_out_i2s); buf_out_i2s = NULL;

	printf("STOP HERE, %s:%d\n", __FILE__, __LINE__);
	while(1) { MMP_USleep(500000); }
}

