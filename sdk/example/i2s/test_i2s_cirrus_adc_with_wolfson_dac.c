/* sy.chuang, 2012-0911, ITE Tech. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mmp_i2s.h"

#define SRATE       48000

#define USE_HDMIRX      1
#define INTERNAL_HDMIRX 1
#define I2S_SLAVE_MODE  1

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

/* ========================================================================= */
void i2s_main(void)
{
	unsigned AD_w;
	unsigned DA_r;

	const unsigned buf_i2s_size = (512 << 10);
	unsigned char *buf_i2s;
	unsigned char *buf_in_hdmi[4];

	/* prepare I2S buffer */
	FC_MALLOC(buf_i2s, buf_i2s_size);
	FC_MALLOC(buf_in_hdmi[0], buf_i2s_size);
	FC_MALLOC(buf_in_hdmi[1], buf_i2s_size);
	FC_MALLOC(buf_in_hdmi[2], buf_i2s_size);
	FC_MALLOC(buf_in_hdmi[3], buf_i2s_size);

	/* init I2S DAC */
	{
		STRC_I2S_SPEC spec;
		memset((void*)&spec, 0, sizeof(STRC_I2S_SPEC));

		spec.use_hdmirx         = USE_HDMIRX;
		spec.internal_hdmirx    = INTERNAL_HDMIRX;
		spec.slave_mode         = I2S_SLAVE_MODE;
		spec.channels           = 2;
		spec.sample_rate        = SRATE;
		spec.buffer_size        = buf_i2s_size;
		spec.is_big_endian      = 0;
		spec.sample_size        = 16;

		spec.base_out_i2s_spdif     = buf_i2s; // buf_in_hdmi[1];
		spec.postpone_audio_output  = 0;

		i2s_init_DAC(&spec);
	}

	/* init I2S ADC */
	{
		STRC_I2S_SPEC spec;
		memset((void*)&spec, 0, sizeof(STRC_I2S_SPEC));

		spec.use_hdmirx         = USE_HDMIRX;
		spec.internal_hdmirx    = INTERNAL_HDMIRX;
		spec.slave_mode         = I2S_SLAVE_MODE;
		spec.channels           = 2;
		spec.sample_rate        = SRATE;
		spec.buffer_size        = buf_i2s_size;
		spec.is_big_endian      = 0;
		spec.sample_size        = 16;

		spec.base_in_hdmi[0]    = buf_in_hdmi[0];
		spec.base_in_hdmi[1]    = buf_in_hdmi[1];
		spec.base_in_hdmi[2]    = buf_in_hdmi[2];
		spec.base_in_hdmi[3]    = buf_in_hdmi[3];
		spec.base_in_i2s        = buf_i2s;
		spec.from_LineIN        = 1;
		spec.from_MIC_IN        = 1;
		spec.record_mode        = 1;

		i2s_init_ADC(&spec);
	}

	dump_i2s_reg();

	while(1)
	{
		AD_w = I2S_AD32_GET_WP();
		I2S_DA32_SET_WP(AD_w);
		DA_r = I2S_DA32_GET_RP();
		I2S_AD32_SET_RP(DA_r);
		printf("AD_w:0x%08x, DA_r:0x%08x\t\t\t\r", AD_w, DA_r);
	}

	printf("STOP HERE, %s:%d\n", __func__, __LINE__);
	while(1) { __asm__(""); }

	i2s_deinit_DAC();

	/* destroy I2S buffer */
	free(buf_i2s); buf_i2s = NULL;
	free(buf_in_hdmi[0]); buf_in_hdmi[0] = NULL;
	free(buf_in_hdmi[1]); buf_in_hdmi[1] = NULL;
	free(buf_in_hdmi[2]); buf_in_hdmi[2] = NULL;
	free(buf_in_hdmi[3]); buf_in_hdmi[3] = NULL;

	printf("STOP HERE, %s:%d\n", __FILE__, __LINE__);
	while(1) { MMP_USleep(500000); }
}

