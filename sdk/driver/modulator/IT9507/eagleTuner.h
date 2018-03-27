#ifndef _EAGLETUNER_H_
#define _EAGLETUNER_H_


Dword EagleTuner_setIQCalibration(
	IN  Modulator*    modulator,
    IN  Dword         frequency	
);

Dword EagleTuner_calIQCalibrationValue(
	IN  Modulator*    modulator,
    IN  Dword         frequency,
	IN  Byte*		  val
);

Dword EagleTuner_setIQCalibrationEx(
	IN  Modulator*    modulator,
	IN  Dword         fIn,
	IN  IQtable       lowerfreq,
    IN  IQtable       upperfreq	
);
#endif
