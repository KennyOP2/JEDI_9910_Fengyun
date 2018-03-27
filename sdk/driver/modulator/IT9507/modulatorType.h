#ifndef __MODULATOR_TYPE_H__
#define __MODULATOR_TYPE_H__


#define EagleUser_INTERNAL	 1

#ifdef WIN32


#endif

#if 0 //def __GNUC__

/* *

 * for Linux 

 */

/* define NULL, true, flase */

#include <linux/stddef.h>

/* define bool */

#include <linux/types.h>

/* include Linux delay */

#include <linux/delay.h>

#endif
#define IN
#define OUT
#define INOUT
#define Bus_I2C             1
#define Bus_USB             2
#define IT9507Cmd_buildCommand(command, processor)  (command + (Word) (processor << 12))
#define Eagle_MAX_BIT               8
#define IQ_TABLE_NROW 91
#define Command_REG_DEMOD_READ          0x0000
#define Command_REG_DEMOD_WRITE         0x0001
#define Command_REG_EEPROM_READ         0x0004
#define Command_REG_EEPROM_WRITE        0x0005
#define Command_IR_GET                  0x0018
#define Command_QUERYINFO               0x0022
#define Command_BOOT                    0x0023
#define Command_REBOOT                  0x0023
#define Command_SCATTER_WRITE           0x0029
#define Command_GENERIC_READ            0x002A
#define Command_GENERIC_WRITE           0x002B


#ifndef UNDER_CE
#endif

#ifdef __cplusplus
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT __declspec(dllexport)
#endif

/**
 * The type of handle.
 */
typedef void* Handle;


/**
 * The type defination of 8-bits unsigned type.
 */
typedef unsigned char Byte;


/**
 * The type defination of 16-bits unsigned type.
 */
typedef unsigned short Word;


/**
 * The type defination of 32-bits unsigned type.
 */
typedef unsigned long Dword;


/**
 * The type defination of 16-bits signed type.
 */
typedef short Short;


/**
 * The type defination of 32-bits signed type.
 */
typedef long Long;


/**
 * The type defination of ValueSet.
 */
typedef struct {
    Dword frequency;      /**  */
    int dAmp;			  /**  */
	int dPhi;
} IQtable;

/**
 * The type defination of Bool
 */

typedef enum {
    False = 0,
    True = 1
} Bool
#ifdef WIN32
#ifndef __cplusplus
,bool
#endif
#endif
;

//typedef enum {False=0, True} bool;


typedef struct {
    Byte segmentType;           /** 0:Firmware download 1:Rom copy 2:Direct command */
    Dword segmentLength;
} Segment;


/**
 * The type defination of Bandwidth.
 */
typedef enum {
    Bandwidth_6M = 0,           /** Signal bandwidth is 6MHz */
    Bandwidth_7M,               /** Signal bandwidth is 7MHz */
    Bandwidth_8M,               /** Signal bandwidth is 8MHz */
    Bandwidth_5M                /** Signal bandwidth is 5MHz */
} Bandwidth;

/**
 * The type defination of TransmissionMode.
 */
typedef enum {
    TransmissionMode_2K = 0,    /** OFDM frame consists of 2048 different carriers (2K FFT mode) */
    TransmissionMode_8K = 1,    /** OFDM frame consists of 8192 different carriers (8K FFT mode) */
    TransmissionMode_4K = 2     /** OFDM frame consists of 4096 different carriers (4K FFT mode) */
} TransmissionModes;


/**
 * The type defination of Interval.
 */
typedef enum {
    Interval_1_OVER_32 = 0,     /** Guard interval is 1/32 of symbol length */
    Interval_1_OVER_16,         /** Guard interval is 1/16 of symbol length */
    Interval_1_OVER_8,          /** Guard interval is 1/8 of symbol length  */
    Interval_1_OVER_4           /** Guard interval is 1/4 of symbol length  */
} Interval;


/**
 * The type defination of Priority.
 */
typedef enum {
    Priority_HIGH = 0,          /** DVB-T and DVB-H - identifies high-priority stream */
    Priority_LOW                /** DVB-T and DVB-H - identifies low-priority stream  */
} Priority;


/**
 * The type defination of CodeRate.
 */
typedef enum {
    CodeRate_1_OVER_2 = 0,      /** Signal uses FEC coding ratio of 1/2 */
    CodeRate_2_OVER_3,          /** Signal uses FEC coding ratio of 2/3 */
    CodeRate_3_OVER_4,          /** Signal uses FEC coding ratio of 3/4 */
    CodeRate_5_OVER_6,          /** Signal uses FEC coding ratio of 5/6 */
    CodeRate_7_OVER_8,          /** Signal uses FEC coding ratio of 7/8 */
    CodeRate_NONE               /** None, NXT doesn't have this one     */
} CodeRate;


/**
 * TPS Hierarchy and Alpha value.
 */
typedef enum {
    Hierarchy_NONE = 0,         /** Signal is non-hierarchical        */
    Hierarchy_ALPHA_1,          /** Signalling format uses alpha of 1 */
    Hierarchy_ALPHA_2,          /** Signalling format uses alpha of 2 */
    Hierarchy_ALPHA_4           /** Signalling format uses alpha of 4 */
} Hierarchy;

/**
 * The type defination of IpVersion.
 */
typedef enum {
    IpVersion_IPV4 = 0,         /** The IP version if IPv4 */
    IpVersion_IPV6 = 1          /** The IP version if IPv6 */
} IpVersion;

/**
 * The type defination of Target.
 */
typedef enum {
    SectionType_MPE = 0,        /** Stands for MPE data.                                         */
    SectionType_SIPSI,          /** Stands for SI/PSI table, but don't have to specify table ID. */
    SectionType_TABLE           /** Stands for SI/PSI table.                                     */
} SectionType;


/**
 * The type defination of FrameRow.
 */
typedef enum {
    FrameRow_256 = 0,           /** There should be 256 rows for each column in MPE-FEC frame.  */
    FrameRow_512,               /** There should be 512 rows for each column in MPE-FEC frame.  */
    FrameRow_768,               /** There should be 768 rows for each column in MPE-FEC frame.  */
    FrameRow_1024               /** There should be 1024 rows for each column in MPE-FEC frame. */
} FrameRow;


/**
 * The type defination of Pid.
 *
 * In DVB-T mode, only value is valid. In DVB-H mode,
 * as sectionType = SectionType_SIPSI: only value is valid.
 * as sectionType = SectionType_TABLE: both value and table is valid.
 * as sectionType = SectionType_MPE: except table all other fields is valid.
 */
typedef struct {
    Byte table;                 /** The table ID. Which is used to filter specific SI/PSI table.                                  */
    Byte duration;              /** The maximum burst duration. It can be specify to 0xFF if user don't know the exact value.     */
    FrameRow frameRow;          /** The frame row of MPE-FEC. It means the exact number of rows for each column in MPE-FEC frame. */
    SectionType sectionType;    /** The section type of pid. See the defination of SectionType.                                   */
    Priority priority;          /** The priority of MPE data. Only valid when sectionType is set to SectionType_MPE.              */
    IpVersion version;          /** The IP version of MPE data. Only valid when sectionType is set to SectionType_MPE.            */
    Bool cache;                 /** True: MPE data will be cached in device's buffer. Fasle: MPE will be transfer to host.        */
    Word value;                 /** The 13 bits Packet ID.                                                                        */
} Pid;


/**
 * The type defination of ValueSet.
 */
typedef struct {
    Dword address;      /** The address of target register */
    Byte value;         /** The value of target register   */
} ValueSet;

/**
 * The type defination of StreamType.
 */
typedef enum {
	INTERFACE_UNUSE = 0, 
    PARALLEL_TS_INPUT=4,		/** input via paralle interface */
    SERIAL_TS_INPUT,			/** input via serial interface  */
} TsInterface;

/**
 * The type defination of StreamType.
 */
typedef enum {
    Architecture_NONE = 0,      /** Inavalid (Null) Architecture.                                    */
    Architecture_DCA,           /** Diversity combine architecture. Only valid when chip number > 1. */
    Architecture_PIP            /** Picture in picture. Only valid when chip number > 1.             */
} Architecture;


/**
 * The type defination of Processor.
 */
typedef enum {
    Processor_LINK = 0,
    Processor_OFDM = 8
} Processor;



/**
 * The type defination of Constellation.
 */
typedef enum {

    Constellation_QPSK = 0,     /** Signal uses QPSK constellation  */
    Constellation_16QAM,        /** Signal uses 16QAM constellation */
    Constellation_64QAM         /** Signal uses 64QAM constellation */
} Constellation;

/**
 * The defination of ChannelInformation.
 */
typedef struct {
    Dword frequency;                    /** Channel frequency in KHz.                                */
    TransmissionModes transmissionMode; /** Number of carriers used for OFDM signal                  */
    Constellation constellation;        /** Constellation scheme (FFT mode) in use                   */
    Interval interval;                  /** Fraction of symbol length used as guard (Guard Interval) */
    Priority priority;                  /** The priority of stream                                   */
    CodeRate highCodeRate;              /** FEC coding ratio of high-priority stream                 */
    CodeRate lowCodeRate;               /** FEC coding ratio of low-priority stream                  */
    Hierarchy hierarchy;                /** Hierarchy levels of OFDM signal                          */
    Bandwidth bandwidth;
} ChannelModulation;

typedef struct _TPS{

    Byte highCodeRate;
    Byte lowCodeRate;
    Byte transmissionMode;
    Byte constellation; 
    Byte interval;
    Word cellid;

} TPS, *pTPS;

/**
 * The data structure of Eagle
 */
typedef struct {
    /** Basic structure */
    Handle userData;
    Byte busId;
	Byte i2cAddr;
    Byte* firmwareCodes;
    Segment* firmwareSegments;
    Byte*  firmwarePartitions;
    Word* scriptSets;
    ValueSet* scripts;
    TsInterface tsInterfaceType;
    Word bandwidth;
    Dword frequency;    
    Bool booted;
	Byte slaveIICAddr;  
	ChannelModulation channelModulation;
	Word c1DefaultValue;
	Word c2DefaultValue;
	Word c3DefaultValue;
	IQtable *ptrIQtableEx;
	int	outputGain;	
    Word* usb_info;
} Modulator;


extern const Byte Eagle_bitMask[8];
#define REG_MASK(pos, len)                (Eagle_bitMask[len-1] << pos)
#define REG_CLEAR(temp, pos, len)         (temp & (~REG_MASK(pos, len)))
#define REG_CREATE(val, temp, pos, len)   ((val << pos) | (REG_CLEAR(temp, pos, len)))
#define REG_GET(value, pos, len)          ((value & REG_MASK(pos, len)) >> pos)
#define LOWBYTE(w)      ((Byte)((w) & 0xff))
#define HIGHBYTE(w)     ((Byte)((w >> 8) & 0xff))
#define OMEGA_NORMAL                    0x00
#define OMEGA_LNA_Config_1              0x01
#define OMEGA_LNA_Config_2              0x02
#define OMEGA_LNA_Config_3              0x03
#endif
