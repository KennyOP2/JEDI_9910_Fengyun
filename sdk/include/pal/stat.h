/*
 * Copyright (c) 2010 ITE Technology Corp. All Rights Reserved.
 */
/** @stat
 * Task Statistics VCD Dump.
 *
 * To view the VCD file, please download Wave VCD Viewer from
 * http://www.iss-us.com/wavevcd/index.htm.
 *
 * Insert following function call to start trace the task context
 * switch event.
 *
 * PalTaskDumpOpen();
 *
 * And insert following code to the main task to dump the VCD file
 * when the trace complete.
 *
 * if (PalTaskDumpReady())
 *     PalTaskDumpWrite();
 *
 * @author Kuoping Hsu
 * @version 1.0
 */
#ifndef PAL_STAT_H
#define PAL_STAT_H

#include "pal/def.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Open Task Dump file
 */
void PalTaskDumpOpen( MMP_WCHAR * filename, int size );

/*
 * Return Task Dump Status
 */
MMP_UINT32 PalTaskDumpReady( void );

/*
 * Write Task Dump file
 */
void PalTaskDumpWrite( void );

/*
 * Close Task Dump file
 */
void PalTaskDumpClose( void );

#ifdef __cplusplus
}
#endif

#endif /* PAL_PRINT_H */

