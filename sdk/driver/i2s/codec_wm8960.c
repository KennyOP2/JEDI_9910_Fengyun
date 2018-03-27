/* sy.chuang, 2012-0622, ITE Tech. */
#include "pal/pal.h"
#include "mmp_types.h"
#include "sys/sys.h"

#include "host/ahb.h"
#include "host/host.h"
#include "host/gpio.h"
#include "mmp_iic.h"

#define WM8960_I2CADR (0x34 >> 1)
#define GPIO_BASEADDR (0xDE000000)

//#define CONFIG_WOLFSON_MASTER

/* FIXME: need more verification */
//#ifdef CFG_CHIP_REV_A0
#define WM8960_BEFORE_ACCESS_IIC 0
#define WM8960_AFTER_ACCESS_IIC  0
//#else /* if (defined CFG_CHIP_REV_A1) */
//	#define WM8960_BEFORE_ACCESS_IIC    3
//	#define WM8960_AFTER_ACCESS_IIC     3
//#endif

//#define WM8960_DEBUG_I2C_COMMON_WRITE
//#define WM8960_DEBUG_PROGRAM_REGVALUE

/* ************************************************************************** */
/* wrapper */
static inline void i2s_delay_us(unsigned us)
{
    //	printf("delay_us(%u)\n", us);
    MMP_USleep(us);
}

/* ************************************************************************** */
#define MAX_OUT1_VOLUME          0x7F   /* +6 dB */
#define ZERO_DB_OUT1_VOLUME      0x79   /*  0 dB */
#define MIN_OUT1_VOLUME          0x30   /* -73dB */

#define MAX_INPUT_PGA_VOLUME     0x3F   /* +30   dB */
#define ZERO_DB_INPUT_PGA_VOLUME 0x17   /* +0    dB */
#define MIN_INPUT_PGA_VOLUME     0x00   /* -17.25dB */

typedef struct WM8960_PARAM_TAG
{
    unsigned     curr_out1_volume;
    unsigned     curr_input_pga_volume;

    unsigned int wm8960_DA_running;
    unsigned int wm8960_AD_running;

    unsigned int micin;
    unsigned int linein2;
    unsigned int linein3;

    unsigned int micBoostGain;   //0:+0db, 1:+13db, 2:+20db, 3:+29db
    unsigned int line2BoostGain; //000 = Mute 001 = -12dB ...3dB steps up to 111 = +6dB
    unsigned int line3BoostGain; //000 = Mute 001 = -12dB ...3dB steps up to 111 = +6dB

    unsigned int runDAC;
    unsigned int registerVol;
} WM8960_PARAM;

//for record
WM8960_PARAM        gtCodec0 = {ZERO_DB_OUT1_VOLUME, ZERO_DB_INPUT_PGA_VOLUME, 0, 0,   0, 0, 0,   0, 5, 5, 0, 0};
WM8960_PARAM        *gtCodec            = &gtCodec0;
static void         *gtCodecModuleMutex = MMP_NULL;

static unsigned int Adcdatstate         = 0;//kenny patch 20140428
void itp_codec_rec_mute(void);
void itp_codec_rec_unmute(void);

/* ************************************************************************** */
static void I2C_common_write_byte(unsigned char RegAddr, unsigned char d)
{
    int flag;

    mmpIicLockModule();

    if (0 != (flag = mmpIicSendData(IIC_MASTER_MODE, WM8960_I2CADR, RegAddr, &d, 1)))
    {
        printf("WM8960# IIC Write Fail!\n");
        mmpIicGenStop();
    }

    mmpIicReleaseModule();
}

static void wm8960_write_reg(unsigned char reg_addr, unsigned short value)
{
    unsigned char A;
    unsigned char B;

#ifdef WM8960_DEBUG_PROGRAM_REGVALUE
    printf("WM8960# reg[0x%02x] = 0x%04x\n", reg_addr, value);
#endif

    A = ((reg_addr & 0x7F) << 1) | ((value >> 8) & 0x1);
    B = (value & 0xFF);

    I2C_common_write_byte(A, B);
}

void mic_input_control(void)
{
    unsigned short data16;

    //mic setting
    data16 = ((0x1 & 0x1) << 8) |     //differential inputs
             ((0x0 & 0x1) << 7) |
             ((0x0 & 0x1) << 6) |
             ((gtCodec->micBoostGain & 0x3) << 4) |
             ((gtCodec->micin & 0x1) << 3);

    wm8960_write_reg(32, data16);     /* enable VMID to PGA, and to boost mixer */
    wm8960_write_reg(33, data16);     /* enable VMID to PGA, and to boost mixer */
}

void line2_line3_input_control(void)
{
    unsigned short data16;
    unsigned int   line3BoostGain, line2BoostGain;

    //line2, line3 setting
    if (gtCodec->linein2 == 0)
        line2BoostGain = 0; //MUTE
    else
        line2BoostGain = gtCodec->line2BoostGain;

    if (gtCodec->linein3 == 0)
        line3BoostGain = 0; //MUTE
    else
        line3BoostGain = gtCodec->line3BoostGain;

    data16 = ((line3BoostGain & 0x7) << 4) |
             ((line2BoostGain & 0x7) << 1);

    wm8960_write_reg(43, data16);     /* LIN3/LIN2 Boost */
    wm8960_write_reg(44, data16);     /* RIN3/LIN2 Boost */
}

static void init_wm8960_common(void)
{
    printf("WM8960# %s\n", __func__);
    if (gtCodec->wm8960_DA_running)
    {
        printf("WM8960# DAC is running, skip re-init process !\n");
        return;
    }
    if (gtCodec->wm8960_AD_running)
    {
        printf("WM8960# ADC is running, skip re-init process !\n");
        return;
    }

    /* programming WM8960 */
    {
        unsigned short data16;
        unsigned int   line2BoostGain, line3BoostGain;

        //kenny patch 20140428 wm8960_write_reg(15, 0x000); /* reset WM8960 */

        /* AD */
        wm8960_write_reg( 9, 0x040);         /* audio interface: ADCLRC/GPIO1 select GPIO1 */

        wm8960_write_reg(47, 0x03C);         /* enable left/right input PGA */
        //kenny patch 20140428
        if (gtCodec->runDAC)
        {
            if (Adcdatstate == 0)
            {
                Adcdatstate = 1;
                wm8960_write_reg(24, 0x008);                 /* Tristates ADCDATand switches ADCLRC, DACLRC and BCLK to inputs. */
                printf("ADCDAT Tristate!!!\r\n");
            }
            else
            {
                printf("ACDAT Tristate now,don't set again!!!\r\n");
            }
        }
        else
        {
            Adcdatstate = 0;
            wm8960_write_reg(24, 0x000);
        }
        //mic setting
        mic_input_control();
        //wm8960_write_reg(17, 0x19B); /* Automatic Level Control  */
        /* restore previous rec volume */
        data16               = (1 << 8) | (gtCodec->curr_input_pga_volume & 0x3F);
        wm8960_write_reg( 0, data16); /* left input PGA disable analogue mute, 0dB */
        wm8960_write_reg( 1, data16); /* right input PGA disable analogue mute, 0dB */
        gtCodec->registerVol = (data16 & 0x3F);

        //line2, line3 setting
        line2_line3_input_control();

        wm8960_write_reg(21, 0x1C3);         /* Left ADC Vol = 0dB */
        wm8960_write_reg(22, 0x1C3);         /* Right ADC Vol = 0dB */

        /* DA */
#ifdef CONFIG_WOLFSON_MASTER
        wm8960_write_reg( 7, 0x04E);         /* audio interface: data word length: 32 bits, I2S format */
        wm8960_write_reg( 8, 0x1C4);
#else
        wm8960_write_reg( 7, 0x00E);         /* audio interface: data word length: 32 bits, I2S format */
#endif

        /* anti-pop */
        wm8960_write_reg(28, 0x094);         /* Enable POBCTRL, SOFT_ST and BUFDCOPEN */
        wm8960_write_reg(26, 0x060);         /* Enable LOUT1 and ROUT1 */
        i2s_delay_us(50000);
        wm8960_write_reg(25, 0x080);         /* VMID=50K, Enable VREF, AINL, AINR, ADCL and ADCR */
        i2s_delay_us(100000);
        wm8960_write_reg(25, 0x0FE);         /* VMID=50K, Enable VREF, AINL, AINR, ADCL, ADCR and MICB */

        //ADC Data Output Select
        //wm8960_write_reg(23, 0x1C4); /* [3:2]left data = left ADC; right data = left ADC */

        //if(gtCodec->micin) {
        //	wm8960_write_reg(25, 0x0FE); /* VMID=50K, Enable VREF, AINL, AINR, ADCL, ADCR and MICB */
        //}
        //else {
        //	wm8960_write_reg(25, 0x0CE); /* VMID=50K, Enable VREF, ADCL and ADCR */
        //}

        wm8960_write_reg(28, 0x010);         /* Disable POBCTRL and SOFT_ST. BUFDCOPEN remain enabled */

        //DAC setting
        wm8960_write_reg(26, 0x1E0);         /* Enable DACL, DACR. LOUT1 and ROUT1 remain enabled */
        wm8960_write_reg(34, 0x170);         /* Left DAC to left output mixer enabled (LD2LO) */
        wm8960_write_reg(37, 0x170);         /* Right DAC to right output mixer enabled (RD2RO) */

        /* restore previous playback volume */
        data16 = (1 << 8) | (gtCodec->curr_out1_volume & 0x7F);
        wm8960_write_reg( 2, data16);        /* LOUT1 Vol = 0dB */
        wm8960_write_reg( 3, data16);        /* ROUT1 Vol = 0dB */

        wm8960_write_reg( 5, 0x000);         /* Unmute DAC digital soft mute */
    }
}

static void deinit_wm8960_common(void)
{
    printf("WM8960# %s\n", __func__);

    if (gtCodec->wm8960_DA_running)
    {
        printf("WM8960# DAC is running, skip deinit !\n");
        return;
    }
    if (gtCodec->wm8960_AD_running)
    {
        printf("WM8960# ADC is running, skip deinit !\n");
        return;
    }

    wm8960_write_reg( 5, 0x008);     //DAC Digital Soft Mute = Mute
    wm8960_write_reg(28, 0x090);     //Enable POBCTRL and BUFDCOPEN
    wm8960_write_reg(25, 0x000);     //Disable VMID and VREF
    wm8960_write_reg(28, 0x190);     //Enable VMIDTOG to discharge VMID capacitor
    i2s_delay_us(800000);
    wm8960_write_reg(26, 0x000);     //Disable DACL, DACR, LOUT1, ROUT1
    //kenny patch 20140428 wm8960_write_reg(15, 0x000); //Reset Device (default registers)
}

/* ************************************************************************** */
void itp_codec_creat_semaphore(void)
{
    if (gtCodecModuleMutex == MMP_NULL)
        gtCodecModuleMutex = SYS_CreateSemaphore(1, "CODEC_MODULE");
}

void itp_codec_delete_semaphore(void)
{
    if (gtCodecModuleMutex)
    {
        SYS_DeleteSemaphore(gtCodecModuleMutex);
        gtCodecModuleMutex = MMP_NULL;
    }
}

void itp_codec_lock_dev(void)
{
    if (gtCodecModuleMutex)
        SYS_WaitSemaphore(gtCodecModuleMutex);
}

void itp_codec_unlock_dev(void)
{
    if (gtCodecModuleMutex)
        SYS_ReleaseSemaphore(gtCodecModuleMutex);
}

void itp_codec_param_init(void)
{
#ifdef USE_WM8960_ADC
    itp_codec_lock_dev();
    //kenny patch 20140428 wm8960_write_reg(15, 0x000); /* reset WM8960 */
    itp_codec_unlock_dev();
#endif
    gtCodec->curr_out1_volume      = ZERO_DB_OUT1_VOLUME;
    gtCodec->curr_input_pga_volume = ZERO_DB_INPUT_PGA_VOLUME;
    gtCodec->wm8960_DA_running     = 0;
    gtCodec->wm8960_AD_running     = 0;
    gtCodec->micin                 = 0;
    gtCodec->linein2               = 0;
    gtCodec->linein3               = 0;
    gtCodec->micBoostGain          = 0;
    gtCodec->line2BoostGain        = 5;
    gtCodec->line3BoostGain        = 5;
    gtCodec->runDAC                = 0;
}

void itp_codec_set_linein(
    unsigned int enmicin,
    unsigned int enlinein2,
    unsigned int enlinein3)
{
    unsigned short data16;
    unsigned int   micBoostGain, line3BoostGain, line2BoostGain;

    if (gtCodec->micin != enmicin)
    {
        itp_codec_rec_mute();
        gtCodec->micin = enmicin;
        //mic setting
        mic_input_control();
        itp_codec_rec_unmute();
    }

    if ((gtCodec->linein2 != enlinein2) || (gtCodec->linein3 != enlinein3))
    {
        itp_codec_rec_mute();
        gtCodec->linein2 = enlinein2;
        gtCodec->linein3 = enlinein3;
        //line2 line3 setting
        line2_line3_input_control();
        itp_codec_rec_unmute();
    }
}

void itp_codec_set_boost_gain(
    unsigned int line,
    unsigned int boostgain)
{
    switch (line)
    {
    case 0:
        if (gtCodec->micBoostGain != boostgain)
        {
            itp_codec_rec_mute();
            gtCodec->micBoostGain = boostgain;
            mic_input_control();
            itp_codec_rec_unmute();
        }
        break;

    case 1:
        if (gtCodec->line2BoostGain != boostgain)
        {
            itp_codec_rec_mute();
            gtCodec->line2BoostGain = boostgain;
            line2_line3_input_control();
            itp_codec_rec_unmute();
        }
        break;

    case 2:
        if (gtCodec->line3BoostGain != boostgain)
        {
            itp_codec_rec_mute();
            gtCodec->line3BoostGain = boostgain;
            line2_line3_input_control();
            itp_codec_rec_unmute();
        }
        break;
    }
}

//===========================================================================
//                          Audio Output
//===========================================================================
/**
 * Initialize the audio DAC
 */
void itp_codec_playback_init(void)
{
    printf("WM8960# %s\n", __func__);

    gtCodec->micin             = 0;
    gtCodec->linein2           = 0;
    gtCodec->linein3           = 0;
    gtCodec->runDAC            = 1;

    init_wm8960_common();
    gtCodec->wm8960_DA_running = 1;
}

/**
 * De-initialize the audio DAC
 */
void itp_codec_playback_deinit(void)
{
    printf("WM8960# %s\n", __func__);

    gtCodec->wm8960_DA_running = 0;     /* put before deinit_wm8960_common() */
    deinit_wm8960_common();
}

void itp_codec_playback_set_direct_vol(unsigned target_vol)
{
    int            direction;
    unsigned short data16;

    if (target_vol > MAX_OUT1_VOLUME)
    {
        printf("WM8960 ERROR# invalid target volume step: 0x%08x\n", target_vol);
        return;
    }

    if (target_vol == gtCodec->curr_out1_volume)
    {
        return;
    }
    else if (target_vol > gtCodec->curr_out1_volume)
    {
        direction = 1;
    }                                                                       /* + */
    else
    {
        direction = 0;
    }                                                              /* - */

    while (gtCodec->curr_out1_volume != target_vol)
    {
        if (direction == 1)
        {
            gtCodec->curr_out1_volume++;
        }
        else
        {
            gtCodec->curr_out1_volume--;
        }

        if (gtCodec->wm8960_DA_running)
        {
            data16 = (1 << 8) | gtCodec->curr_out1_volume;
            wm8960_write_reg(2, data16);
            wm8960_write_reg(3, data16);
        }

        i2s_delay_us(1000); /* FIXME: dummy loop */;
    }
}

void itp_codec_playback_amp_volume_down(void)
{
    if (gtCodec->curr_out1_volume <= MIN_OUT1_VOLUME)
        return;

    switch (gtCodec->curr_out1_volume)
    {
    case 0x7D ... 0x7F: { itp_codec_playback_set_direct_vol(0x7C); break; }             /* +3 dB */
    case 0x7A ... 0x7C: { itp_codec_playback_set_direct_vol(0x79); break; }             /*  0 dB */
    case 0x77 ... 0x79: { itp_codec_playback_set_direct_vol(0x76); break; }             /* -3 dB */
    case 0x74 ... 0x76: { itp_codec_playback_set_direct_vol(0x73); break; }             /* -6 dB */
    case 0x70 ... 0x73: { itp_codec_playback_set_direct_vol(0x6F); break; }             /* -10dB */
    case 0x5C ... 0x6F: { itp_codec_playback_set_direct_vol(0x5B); break; }             /* -30dB */
    case 0x31 ... 0x5B: { itp_codec_playback_set_direct_vol(0x30); break; }             /* -73dB */
    }
}

void itp_codec_playback_amp_volume_up(void)
{
    if (gtCodec->curr_out1_volume >= MAX_OUT1_VOLUME)
        return;

    switch (gtCodec->curr_out1_volume)
    {
    case 0x7C ... 0x7E: { itp_codec_playback_set_direct_vol(0x7F); break; }             /* +6 dB */
    case 0x79 ... 0x7B: { itp_codec_playback_set_direct_vol(0x7C); break; }             /* +3 dB */
    case 0x76 ... 0x78: { itp_codec_playback_set_direct_vol(0x79); break; }             /*  0 dB */
    case 0x73 ... 0x75: { itp_codec_playback_set_direct_vol(0x76); break; }             /* -3 dB */
    case 0x6F ... 0x72: { itp_codec_playback_set_direct_vol(0x73); break; }             /* -6 dB */
    case 0x5B ... 0x6E: { itp_codec_playback_set_direct_vol(0x6F); break; }             /* -10dB */
    case 0x30 ... 0x5A: { itp_codec_playback_set_direct_vol(0x5B); break; }             /* -30dB */
    }
}

void itp_codec_playback_get_currvol(unsigned *currvol)
{
    *currvol = gtCodec->curr_out1_volume;
}

void itp_codec_playback_get_vol_range(unsigned *max, unsigned *regular_0db, unsigned *min)
{
    *max         = MAX_OUT1_VOLUME;
    *regular_0db = ZERO_DB_OUT1_VOLUME;
    *min         = MIN_OUT1_VOLUME;
}

void itp_codec_playback_mute(void)
{}

void itp_codec_playback_unmute(void)
{}

//===========================================================================
//                          Audio Input
//===========================================================================
/**
 * Initialize the audio ADC
 */
void itp_codec_rec_init()
{
    printf("WM8960# %s\n", __func__);

    //todo: add mutex protection
    gtCodec->runDAC            = 0;
    init_wm8960_common();
    gtCodec->wm8960_AD_running = 1;
}

/**
 * De-initialize the audio ADC
 */
void itp_codec_rec_deinit(void)
{
    printf("WM8960# %s\n", __func__);

    //todo: add mutex protection
    gtCodec->wm8960_AD_running = 0;     /* put before deinit_wm8960_common() */
    deinit_wm8960_common();
}

/**
 * Set audio input volume
 *
 * @param[in] target_vol the desired volume value (range: 0x0[-17.25 dB] ~
 *       0x3F[+30 dB])
 */
void itp_codec_rec_set_direct_vol(unsigned target_vol)
{
    int            direction;
    unsigned short data16;

    if (target_vol > MAX_INPUT_PGA_VOLUME)
    {
        printf("WM8960 ERROR# invalid target volume step: 0x%08x\n", target_vol);
        target_vol = MAX_INPUT_PGA_VOLUME;  // iclai (2015-08-15): fix the logic error
    }

    if (target_vol == gtCodec->curr_input_pga_volume)
    {
        return;
    }
    else if (target_vol > gtCodec->curr_input_pga_volume)
    {
        direction = 1;
    }                                                                            /* + */
    else
    {
        direction = 0;
    }                                                                            /* - */

    while (gtCodec->curr_input_pga_volume != target_vol)
    {
        if (direction == 1)
        {
            gtCodec->curr_input_pga_volume++;
        }
        else
        {
            gtCodec->curr_input_pga_volume--;
        }

        data16               = (1 << 8) | (gtCodec->curr_input_pga_volume & 0x3F);
        wm8960_write_reg( 0, data16);
        wm8960_write_reg( 1, data16);
        gtCodec->registerVol = (data16 & 0x3F);
        i2s_delay_us(1000); /* FIXME: dummy loop */;
    }
}

/**
 * Get current audio input volume
 *
 * @param[out] currvol current audio input volume
 */
void itp_codec_rec_get_currvol(unsigned *currvol)
{
    *currvol = gtCodec->curr_input_pga_volume;
}

/**
 * Get the supported audio input volume range
 *
 * @param[out] max         max audio input volume
 * @param[out] regular_0db original audio intput volume.
 * @param[out] min         min audio input volume
 */
void itp_codec_rec_get_vol_range(unsigned *max, unsigned *regular_0db, unsigned *min)
{
    *max         = MAX_INPUT_PGA_VOLUME;
    *regular_0db = ZERO_DB_INPUT_PGA_VOLUME;
    *min         = MIN_INPUT_PGA_VOLUME;
}

/**
 * Mute the audio intput volume
 */
void itp_codec_rec_mute(void)
{
    unsigned short data16;
    unsigned       inpvol = gtCodec->curr_input_pga_volume;

    while (MIN_INPUT_PGA_VOLUME != inpvol)
    {
        inpvol--;
        data16               = (1 << 8) | (inpvol & 0x3F);
        wm8960_write_reg( 0, data16);
        wm8960_write_reg( 1, data16);
        gtCodec->registerVol = (data16 & 0x3F);
        i2s_delay_us(1000); /* FIXME: dummy loop */;
    }
}

/**
 * Unmute the audio intput volume
 */
void itp_codec_rec_unmute(void)
{
    unsigned short data16;
    unsigned       inpvol = MIN_INPUT_PGA_VOLUME;

    while (inpvol != gtCodec->curr_input_pga_volume)
    {
        inpvol++;
        data16               = (1 << 8) | (inpvol & 0x3F);
        wm8960_write_reg( 0, data16);
        wm8960_write_reg( 1, data16);
        gtCodec->registerVol = (data16 & 0x3F);
        i2s_delay_us(1000); /* FIXME: dummy loop */;
    }
}