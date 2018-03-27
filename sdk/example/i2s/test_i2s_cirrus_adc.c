/* sy.chuang, 2012-0827, ITE Tech. */

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
void i2s_main(void)
{
	unsigned AD_r;
	unsigned AD_w;

	const unsigned buf_i2s_size = (2 << 20);
	unsigned char *buf_i2s;
	unsigned char *buf_in_hdmi[4];

	/* prepare I2S buffer */
	FC_MALLOC(buf_i2s, buf_i2s_size);
	FC_MALLOC(buf_in_hdmi[0], buf_i2s_size);
	FC_MALLOC(buf_in_hdmi[1], buf_i2s_size);
	FC_MALLOC(buf_in_hdmi[2], buf_i2s_size);
	FC_MALLOC(buf_in_hdmi[3], buf_i2s_size);

	printf("buf_i2s: 0x%08x\n", (unsigned)buf_i2s);
	printf("buf_i2s_size: 0x%08x = %u (bytes)\n", buf_i2s_size, buf_i2s_size);
	printf("buf_in_hdmi[0]: 0x%08x\n", (unsigned)buf_in_hdmi[0]);
	printf("buf_in_hdmi[1]: 0x%08x\n", (unsigned)buf_in_hdmi[1]);
	printf("buf_in_hdmi[2]: 0x%08x\n", (unsigned)buf_in_hdmi[2]);
	printf("buf_in_hdmi[3]: 0x%08x\n", (unsigned)buf_in_hdmi[3]);

	/* init I2S */
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

	while(1)
	{
		AD_r = I2S_AD32_GET_RP();
		AD_w = I2S_AD32_GET_WP();

		printf("AD_r:0x%08x, AD_w:0x%08x\t\t\t\r", AD_r, AD_w);

		if(AD_w >= (buf_i2s_size - (120 << 10))) { printf("\n"); break; }
	}

	/* TEST */
	{
		unsigned char *buf = buf_i2s;
		unsigned len = buf_i2s_size;
		unsigned i;

		printf("dump i2s input buffer ==>\n");
		for(i=0; i<(10 << 10); i++)
		{
			printf("%02x ", buf[i]);
			if(((i + 1) % 16) == 0) { printf("\n"); }
		}
		printf("\n<==\n");
	}

	printf("STOP HERE !\n");
	while(1) { __asm__(""); }

	i2s_deinit_ADC();

	/* destroy input AD buffer */
	free(buf_i2s); buf_i2s = NULL;
	free(buf_in_hdmi[0]); buf_in_hdmi[0] = NULL;
	free(buf_in_hdmi[1]); buf_in_hdmi[1] = NULL;
	free(buf_in_hdmi[2]); buf_in_hdmi[2] = NULL;
	free(buf_in_hdmi[3]); buf_in_hdmi[3] = NULL;

	printf("STOP HERE, %s:%d\n", __FILE__, __LINE__);
	while(1) { MMP_USleep(500000); }
}

