/* sy.chuang, 2012-0823, ITE Tech. */

#ifndef I2S_9910_REG_H
#define I2S_9910_REG_H

/* ************************************************************************************** */
#define REG_IIS_BASE_OUT1_LO    (0x1600)  /* RW: IIS Output Buffer Base1 Address LO           */
#define REG_IIS_BASE_OUT1_HI    (0x1602)  /* RW: IIS Output Buffer Base1 Address HI           */
#define REG_IIS_BASE_OUT2_LO    (0x1604)  /* RW: IIS Output Buffer Base2 Address LO           */
#define REG_IIS_BASE_OUT2_HI    (0x1606)  /* RW: IIS Output Buffer Base2 Address HI           */
#define REG_IIS_BASE_OUT3_LO    (0x1608)  /* RW: IIS Output Buffer Base3 Address LO           */
#define REG_IIS_BASE_OUT3_HI    (0x160A)  /* RW: IIS Output Buffer Base3 Address HI           */
#define REG_IIS_BASE_OUT4_LO    (0x160C)  /* RW: IIS Output Buffer Base4 Address LO           */
#define REG_IIS_BASE_OUT4_HI    (0x160E)  /* RW: IIS Output Buffer Base4 Address HI           */
#define REG_IIS_BASE_OUT5_LO    (0x1610)  /* RW: IIS Output Buffer Base5 Address LO           */
#define REG_IIS_BASE_OUT5_HI    (0x1612)  /* RW: IIS Output Buffer Base5 Address HI           */
#define REG_IIS_OUT_LEN_LO      (0x1614)  /* RW: IIS Output Buffer Length LO                  */
#define REG_IIS_OUT_LEN_HI      (0x1616)  /* RW: IIS Output Buffer Length HI                  */
#define REG_IIS_OUT_WRPTR_LO    (0x1618)  /* RW: IIS Output Buffer Write Pointer LO           */
#define REG_IIS_OUT_WRPTR_HI    (0x161A)  /* RW: IIS Output Buffer Write Pointer HI           */
#define REG_IIS_OUT_RDPTR_LO    (0x161C)  /* R : IIS Output Buffer Read Pointer LO            */
#define REG_IIS_OUT_RDPTR_HI    (0x161E)  /* R : IIS Output Buffer Read Pointer H             */
#define REG_IIS_BASE_OUT6_LO    (0x1620)  /* RW: IIS Output Buffer Base6 Address LO           */
#define REG_IIS_BASE_OUT6_HI    (0x1622)  /* RW: IIS Output Buffer Base6 Address HI           */
#define REG_IIS_BASE_IN1_LO     (0x1624)  /* RW: IIS Input Buffer Base1 Address LO            */
#define REG_IIS_BASE_IN1_HI     (0x1626)  /* RW: IIS Input Buffer Base1 Address HI            */
#define REG_IIS_BASE_IN2_LO     (0x1628)  /* RW: IIS Input Buffer Base2 Address LO            */
#define REG_IIS_BASE_IN2_HI     (0x162A)  /* RW: IIS Input Buffer Base2 Address HI            */
#define REG_IIS_BASE_IN3_LO     (0x162C)  /* RW: IIS Input Buffer Base3 Address LO            */
#define REG_IIS_BASE_IN3_HI     (0x162E)  /* RW: IIS Input Buffer Base3 Address HI            */
#define REG_IIS_BASE_IN4_LO     (0x1630)  /* RW: IIS Input Buffer Base4 Address LO            */
#define REG_IIS_BASE_IN4_HI     (0x1632)  /* RW: IIS Input Buffer Base4 Address HI            */
#define REG_IIS_BASE_IN5_LO     (0x1634)  /* RW: IIS Input Buffer Base5 Address LO            */
#define REG_IIS_BASE_IN5_HI     (0x1636)  /* RW: IIS Input Buffer Base5 Address HI            */
#define REG_IIS_IN_GAP_LO       (0x1638)  /* RW: IIS Input RdWrGap Threshold LO               */
#define REG_IIS_IN_GAP_HI       (0x163A)  /* RW: IIS Input RdWrGap Threshold HI               */
#define REG_IIS_IN_LEN_LO       (0x163C)  /* RW: IIS Input Buffer Length LO                   */
#define REG_IIS_IN_LEN_HI       (0x163E)  /* RW: IIS Input Buffer Length HI                   */
#define REG_IIS_ADC_SRATE_SET   (0x1640)  /* RW: ADC Sample Rate Ratio Setting                */
#define REG_IIS_DAC_SRATE_SET   (0x1642)  /* RW: DAC Sample Rate Ratio Setting                */
#define REG_IIS_CODEC_PCM_SET   (0x1644)  /* RW: ADC/DAC and PCM Setting                      */
#define REG_IIS_IN_RDPTR_LO     (0x1646)  /* RW: IIS Input Buffer Read Pointer LO             */
#define REG_IIS_IN_RDPTR_HI     (0x1648)  /* RW: IIS Input Buffer Read Pointer HI             */
#define REG_IIS_IN_WRPTR_LO     (0x164A)  /* R : IIS Input Buffer Write Pointer LO            */
#define REG_IIS_IN_WRPTR_HI     (0x164C)  /* R : IIS Input Buffer Write Pointer HI            */
#define REG_IIS_IN_CTRL1        (0x164E)  /* RW: IIS Input Control Setting1                   */
#define REG_IIS_IN_CTRL2        (0x1650)  /* RW: IIS Input Control Setting2                   */
#define REG_IIS_OUT_GAP_LO      (0x1652)  /* RW: IIS Output RdWrGap Threshold LO              */
#define REG_IIS_OUT_GAP_HI      (0x1654)  /* RW: IIS Output RdWrGap Threshold HI              */
#define REG_IIS_OUT_CTRL1       (0x1656)  /* RW: IIS Output Control Setting1                  */
#define REG_IIS_OUT_CTRL2       (0x1658)  /* RW: IIS Output Control Setting2                  */
#define REG_IIS_IN_STATUS1      (0x165A)  /* R : IIS Engine Input Status 1                    */
#define REG_IIS_IN_STATUS2      (0x165C)  /* R : IIS Engine Input Status 2                    */
#define REG_IIS_OUT_STATUS1     (0x165E)  /* R : IIS Engine Output Status 1                   */
#define REG_IIS_OUT_STATUS2     (0x1660)  /* R : IIS Engine Output Status 2                   */
#define REG_IIS_OUT_CROSS_LO    (0x1662)  /* RW: IIS Output Threshold Cross Low Word Control  */
#define REG_IIS_OUT_CROSS_HI    (0x1664)  /* RW: IIS Output Threshold Cross High Word Control */
#define REG_IIS_OUT_FADE        (0x1666)  /* RW: IIS Output Fade-in and Fade-out Control      */
#define REG_IIS_OUT_VOLUME      (0x1668)  /* RW: IIS Output Digital Volume Control            */
#define REG_IIS_IN_VOLUME       (0x166A)  /* RW: IIS Input Digital Volume Control             */
#define REG_IIS_SPDIF_VOL       (0x166C)  /* RW: SPDIF Volume Control                         */
#define REG_IIS_BIST            (0x166E)  /* RW: IIS BIST Control                             */
#define REG_IIS_DAC_CTRL        (0x1670)  /* RW: DAC Control                                  */
#define REG_IIS_AMP_CTRL        (0x1672)  /* RW: AMP Control                                  */
#define REG_IIS_AMP_VOL         (0x1674)  /* RW: AMP Volume Control                           */
#define REG_IIS_TW2866_SEQNUM   (0x1676)  /* RW: TW2866 sequence number                       */
#define REG_IIS_HDMITX_FT_R     (0x1678)  /* RW: HDMI TX FT sync word Right                   */
#define REG_IIS_HDMITX_FT_CTRL  (0x167A)  /* RW: HDMI TX FT Control                           */
#define REG_IIS_IOMUX_CTRL      (0x167C)  /* RW: Audio IO Mux Control                         */
#define REG_IIS_IO_SET          (0x167E)  /* RW: Audio IO Setting                             */
/* ************************************************************************************** */

#define REG_AUDIO_CLOCK_REG_3A  (0x003A)
#define REG_AUDIO_CLOCK_REG_3C  (0x003C)
#define REG_AUDIO_CLOCK_REG_3E  (0x003E)

#endif //I2S_9910_REG_H

