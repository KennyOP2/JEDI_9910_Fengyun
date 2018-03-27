/* sy.chuang, 2012-0925, ITE Tech. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
static inline unsigned reg_read32(unsigned addr32)
{
	unsigned data32;
	AHB_ReadRegister(addr32, &data32);
	return data32;
}
static inline void reg_write32(unsigned addr32, unsigned data32)
{
	AHB_WriteRegister(addr32, data32);
}
//static inline unsigned short reg_read16(unsigned short addr16)
//{
//	unsigned short data16;
//	HOST_ReadRegister(addr16, &data16);
//	return data16;
//}
//static inline void reg_write16(unsigned short addr16, unsigned short data16)
//{
//	HOST_WriteRegister(addr16, data16);
//}
static inline void i2s_delay_us(unsigned us)
{
	MMP_USleep(us);
}

/* ========================================================================= */
void i2s_main(void)
{
	/* config SPDIF register */
	{
		unsigned data32;

		data32 = reg_read32(0xDE900000|0x0000);
		data32 &= ~(7 << 12);
		data32 |=  (5 << 12);
		data32 &= ~(3 << 2);
		reg_write32(0xDE900000|0x0000, data32);

		data32 = reg_read32(0xDE900000|0x0004);
		data32 &= ~(0x1F << 16);
		data32 |=  (0x1F << 16);
		reg_write32(0xDE900000|0x0004, data32);

		data32 = reg_read32(0xDE000000|0x00D0);
		data32 |=  (1 << 7);
		reg_write32(0xDE000000|0x00D0, data32);

		data32 = reg_read32(0xDE900000|0x0008);
		data32 |=  (0xF << 0);
		reg_write32(0xDE900000|0x0008, data32);
	}

	{
		unsigned char *sp;
		unsigned char *sbuf;
		const unsigned sbufsize = ((3 << 20) - (512 << 10));

		unsigned data32;
		unsigned rx_fifo_valid_entry;
		
		unsigned sample_freq;
		unsigned sample_size;
		
		int first_flag = 1;

		FC_MALLOC(sbuf, sbufsize);
		memset((void*)sbuf, 0, sbufsize);
		sp = sbuf;

		printf("sbuf: 0x%08x\n", sbuf);
		printf("sbufsize: %u (Bytes)\n", sbufsize);

		/* get sample freq/size */
		{
			unsigned extra_bits;
			data32 = reg_read32(0xDE900000|0x0028);
			
			switch((data32 & 0xFF) >> 4)
			{
				case 0xF: { sample_freq = 44100; break; }
				case 0xD: { sample_freq = 48000; break; }
				case 0xC: { sample_freq = 32000; break; }
				default : { SERR(); break; }
			}
			
			if(data32 & 0x1) { extra_bits = 4; }
			else             { extra_bits = 0; }

			switch((data32 & 0xF) >> 1)
			{
				case 0x1: { sample_size = 16 + extra_bits; break; }
				case 0x2: { sample_size = 18 + extra_bits; break; }
				case 0x4: { sample_size = 19 + extra_bits; break; }
				case 0x5: { sample_size = 20 + extra_bits; break; }
				case 0x6: { sample_size = 17 + extra_bits; break; }
				default : { SERR(); break; }
			}
			
			printf("sample_freq: %u (Hz)\n", sample_freq);
			printf("sample_size: %u (bits)\n", sample_size);
		}

		while(1)
		{
			if((sp - sbuf) >= (sbufsize - (1 << 10))) { break; }

			data32 = reg_read32(0xDE900000|0x000C);
			rx_fifo_valid_entry = (data32 >> 4) & 0x7F;
			if(first_flag)
			{
				first_flag = 0;
				printf("rx_fifo_valid_entry: %u\n", rx_fifo_valid_entry);
			}
//			printf("rx_fifo_valid_entry: %u\n", rx_fifo_valid_entry);

			while(rx_fifo_valid_entry)
			{
				data32 = reg_read32(0xDE900000|0x0018);
//				printf("data32 = 0x%08x\n", data32);

				if(sample_size == 16)
				{
					*sp = (data32 >>  0) & 0xFF; sp++;
					*sp = (data32 >>  8) & 0xFF; sp++;
				}
				else /* sample_size = 17 ~ 24 */
				{
					*sp = (data32 >>  0) & 0xFF; sp++;
					*sp = (data32 >>  8) & 0xFF; sp++;
					*sp = (data32 >> 16) & 0xFF; sp++;
				}

				rx_fifo_valid_entry--;
			}
		}

		printf("STOP HERE, %s:%d\n", __FILE__, __LINE__);
		while(1) { MMP_USleep(500000); }

		free(sbuf); sbuf = NULL;
	}

	printf("STOP HERE, %s:%d\n", __FILE__, __LINE__);
	while(1) { MMP_USleep(500000); }
}

