#ifndef __EAGLEUSER_H__
#define __EAGLEUSER_H__


//#include <stdio.h>
#include "IT9507.h"
#include "modulatorType.h"
#ifdef __cplusplus
extern "C" {
#endif

#define EagleUser_MAX_PKT_SIZE               255


#define EagleUser_RETRY_MAX_LIMIT            10


/** Define I2C master speed, the default value 0x07 means 366KHz (1000000000 / (24.4 * 16 * EagleUser_I2C_SPEED)). */
#define EagleUser_IIC_SPEED              0x07

/** Define I2C address of secondary chip when Diversity mode or PIP mode is active. */
#define EagleUser_IIC_ADDRESS            0x38
#define EagleUser_SlaveIIC_ADDRESS       0x3A
#define EagleUser_DEVICETYPE			 0

/** Define USB frame size */
#define EagleUser_USB20_MAX_PACKET_SIZE      512
#define EagleUser_USB20_FRAME_SIZE           (188 * 348)
#define EagleUser_USB20_FRAME_SIZE_DW        (EagleUser_USB20_FRAME_SIZE / 4)
#define EagleUser_USB11_MAX_PACKET_SIZE      64
#define EagleUser_USB11_FRAME_SIZE           (188 * 21)
#define EagleUser_USB11_FRAME_SIZE_DW        (EagleUser_USB11_FRAME_SIZE / 4)
#define EagleUser_MAXFRAMESIZE			63

/**
 * Memory copy Function
 */
Dword EagleUser_memoryCopy (
    IN  Modulator*    modulator,
    IN  void*           dest,
    IN  void*           src,
    IN  Dword           count
);


/**
 * Delay Function
 */
Dword EagleUser_delay (
    IN  Modulator*    modulator,
    IN  Dword           dwMs
);

/**
 * Create critical section
 */
Dword EagleUser_createCriticalSection (
    void
);

/**
 * Delete critical section
 */
Dword EagleUser_deleteCriticalSection (
    void
);

/**
 * Enter critical section
 */
Dword EagleUser_enterCriticalSection (
    IN  Modulator*    modulator
);


/**
 * Leave critical section
 */
Dword EagleUser_leaveCriticalSection (
    IN  Modulator*    modulator
);


/**
 * Config MPEG2 interface
 */
Dword EagleUser_mpegConfig (
    IN  Modulator*    modulator
);


/**
 * Write data via "Control Bus"
 * I2C mode : uc2WireAddr mean modulator chip address, the default value is 0x38
 * USB mode : uc2WireAddr is useless, don't have to send this data
 */
Dword EagleUser_busTx (
    IN  Modulator*    modulator,
    IN  Dword           bufferLength,
    IN  Byte*           buffer
);


/**
 * Read data via "Control Bus"
 * I2C mode : uc2WireAddr mean modulator chip address, the default value is 0x38
 * USB mode : uc2WireAddr is useless, don't have to send this data
 */
Dword EagleUser_busRx (
    IN  Modulator*    modulator,
    IN  Dword           bufferLength,
    OUT Byte*           buffer
);

/**
 * Read data via "Data Bus"
 * I2C mode : uc2WireAddr mean modulator chip address, the default value is 0x38
 * USB mode : uc2WireAddr is useless, don't have to send this data
 */
Dword EagleUser_busTxData (
    IN  Modulator*    modulator,
    IN  Dword           bufferLength,
    OUT Byte*           buffer
);

Dword EagleUser_setBus (
	IN  Modulator*	modulator,
	IN  Byte			busId,
	IN  Byte            i2cAddr
);

Dword EagleUser_Initialization  (
    IN  Modulator*    modulator
); 

Dword EagleUser_Finalize  (
    IN  Modulator*    modulator
);

Dword EagleUser_acquireChannel (
	IN  Modulator*    modulator,
	IN  Word          bandwidth,
	IN  Dword         frequency
);

Dword EagleUser_setTxModeEnable (
	IN  Modulator*            modulator,
	IN  Byte                    enable	
);

Dword EagleUser_getChannelIndex (
	IN  Modulator*            modulator,
	IN  Byte*                    index	
);

#ifdef __cplusplus
}
#endif

#endif
