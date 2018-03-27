#ifndef CODEC_WM8960_H
#define CODEC_WM8960_H

void itp_codec_creat_semaphore(void);
void itp_codec_delete_semaphore(void);
void itp_codec_param_init(void);
void itp_codec_lock_dev(void);
void itp_codec_unlock_dev(void);
void itp_codec_set_linein(unsigned int enmicin, unsigned int enlinein2, unsigned int enlinein3);

//===========================================================================
//                          Audio Output
//===========================================================================
/**
 * Initialize the audio DAC
 */
void itp_codec_playback_init(void);

/**
 * De-initialize the audio DAC
 */
void itp_codec_playback_deinit(void);
void itp_codec_playback_set_direct_vol(unsigned target_vol);
void itp_codec_playback_amp_volume_down(void);
void itp_codec_playback_amp_volume_up(void);
void itp_codec_playback_get_currvol(unsigned *currvol);
void itp_codec_playback_get_vol_range(unsigned *max, unsigned *regular_0db, unsigned *min);
void itp_codec_playback_mute(void);
void itp_codec_playback_unmute(void);

//===========================================================================
//                          Audio Input
//===========================================================================
/**
 * Initialize the audio ADC
 */
void itp_codec_rec_init();

/**
 * De-initialize the audio ADC
 */
void itp_codec_rec_deinit(void);

/**
 * Set audio input volume
 *
 * @param[in] target_vol the desired volume value (range: 0x0[-17.25 dB] ~
 *       0x3F[+30 dB])
 */
void itp_codec_rec_set_direct_vol(unsigned target_vol);

/**
 * Get current audio input volume
 *
 * @param[out] currvol current audio input volume
 */
void itp_codec_rec_get_currvol(unsigned *currvol);

/**
 * Get the supported audio input volume range
 *
 * @param[out] max         max audio input volume
 * @param[out] regular_0db original audio intput volume.
 * @param[out] min         min audio input volume
 */
void itp_codec_rec_get_vol_range(unsigned *max, unsigned *regular_0db, unsigned *min);

/**
 * Mute the audio intput volume
 */
void itp_codec_rec_mute(void);

/**
 * Unmute the audio intput volume
 */
void itp_codec_rec_unmute(void);

#endif //CODEC_WM8960_H
