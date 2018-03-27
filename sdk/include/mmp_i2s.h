/* sy.chuang, 2012-0823, ITE Tech. */

/* for 9910 */

#ifndef MMP_I2S_H
#define MMP_I2S_H

#ifdef __cplusplus
extern "C" {
#endif
//=============================================================================
//                              Constant Definition
//=============================================================================
typedef enum CODEC_INPUT_TAG
{
    CODEC_INPUT_ONLY_MIC = 0,
    CODEC_INPUT_ONLY_LINE,
    CODEC_INPUT_MIC_LINE,
    CODEC_INPUT_NONE,
} CODEC_INPUT;

typedef enum CODEC_MIC_BOOST_TAG
{
    CODEC_MIC_BOOST_ADD_0DB  = 0,
    CODEC_MIC_BOOST_ADD_13DB = 1,
    CODEC_MIC_BOOST_ADD_20DB = 2,
    CODEC_MIC_BOOST_ADD_29DB = 3,
} CODEC_MIC_BOOST;

/* ************************************************************************** */
typedef struct
{
    /* input/ouput common */
    unsigned      use_hdmirx;      /* ex: 0:(no use hdmirx), 1:(use hdmirx) */
    unsigned      internal_hdmirx; /* ex: 0:(ext. hdmirx, or 'NoUSE hdmirx'), 1:(internal hdmirx) */
    unsigned      use_hdmitx;      /* ex: 0:(no use hdmitx), 1:(use hdmitx) */
    unsigned      slave_mode;      /* ex: 0:(i2s master), 1:(i2s slave) */
    unsigned      channels;        /* ex: 1:(mono), 2:(stereo) */
    unsigned      sample_rate;     /* ex: 44100/48000 Hz */
    unsigned      buffer_size;
    unsigned      is_big_endian;
    unsigned      sample_size; /* ex: 8/16/24/32 bits */

    /* for input use */
    unsigned char *base_in_hdmi[4];
    unsigned char *base_in_i2s;
    unsigned      from_LineIN;
    unsigned      from_MIC_IN;
    unsigned      digital_IN;  //for dexatek
    unsigned      record_mode; /* 0: hardware start via capture hardware, 1: hardware start via software set */

    /* for output use */
    unsigned char *base_out_i2s_spdif;
    unsigned      postpone_audio_output; /* manually enable audio output */
} STRC_I2S_SPEC;

/* ************************************************************************** */
/* DA */
unsigned I2S_DA32_GET_RP(void);
unsigned I2S_DA32_GET_WP(void);
void I2S_DA32_SET_WP(unsigned WP32);

/* AD */
unsigned I2S_AD32_GET_RP(void);
unsigned I2S_AD32_GET_WP(void);
void I2S_AD32_SET_RP(unsigned RP32);

/********************** export APIs **********************/
/* DA */
void i2s_volume_up(void);
void i2s_volume_down(void);
void i2s_pause_DAC(int pause);
void i2s_deinit_DAC(void);
void i2s_init_DAC(STRC_I2S_SPEC *i2s_spec);
void i2s_mute_DAC(int mute);
int i2s_set_direct_volstep(unsigned volstep);
unsigned i2s_get_current_volstep(void);
void i2s_get_volstep_range(unsigned *max, unsigned *normal, unsigned *min);

/* AD */
void i2s_pause_ADC(int pause);
void i2s_deinit_ADC(MMP_BOOL bDeInitDevice);
void i2s_init_ADC(STRC_I2S_SPEC *i2s_spec, MMP_BOOL bInitDevice);
int i2s_ADC_set_direct_volstep(unsigned volstep);
unsigned i2s_ADC_get_current_volstep(void);
void i2s_ADC_get_volstep_range(unsigned *max, unsigned *normal, unsigned *min);
void i2s_mute_ADC(int mute);
/* ************************************************************************** */

void codec_initialize(
    void);

void codec_terminate(
    void);

void codec_set_device(
    CODEC_INPUT  inType,
    unsigned int isAnalogAudioDev);

void codec_set_mic_boost(
    CODEC_MIC_BOOST boostgain);

void codec_set_mic_mute(
    void);

void codec_set_mic_unmute(
    void);

/**
 * Set the microphone volume
 *
 * @param[in] vol the volume value
 */
void codec_set_mic_vol(
    unsigned int vol);

void codec_set_line_boost(
    unsigned int line,
    unsigned int boostgain);

#ifdef __cplusplus
}
#endif

#endif //MMP_I2S_H