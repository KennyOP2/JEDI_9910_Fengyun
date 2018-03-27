/****************************************************************************
 *
 *            Copyright (c) 2003-2007 by HCC Embedded
 *
 * This software is copyrighted by and is the sole property of
 * HCC.  All rights, title, ownership, or other interests
 * in the software remain the property of HCC.  This
 * software may only be used in accordance with the corresponding
 * license agreement.  Any unauthorized use, duplication, transmission,
 * distribution, or disclosure of this software is expressly forbidden.
 *
 * This Copyright notice may not be removed or modified without prior
 * written consent of HCC.
 *
 * HCC reserves the right to modify this software without notice.
 *
 * HCC Embedded
 * Budapest 1133
 * Vaci ut 110
 * Hungary
 *
 * Tel:  +36 (1) 450 1302
 * Fax:  +36 (1) 450 1303
 * http: www.hcc-embedded.com
 * email: info@hcc-embedded.com
 *
 ***************************************************************************/

/****************************************************************************
 *
 * Includes
 *
 ***************************************************************************/

#include "test_f.h"		 /* include for test externs */
#include <stdio.h>		 /* include for printing */

//#include "testdrv_f.h" 	 /* include for testdrv.h can be changed for any driver */
//#include "../ftl/src/ftldrv.h"
//E:\DPF_230_NFXD_20081211\DPF_230_NFXD_20081211\sdk\share\src\fat\src\ftl\src
//#include "D:/DPF_230_XD/sdk/share/src/fat/src/ftl/src/ftldrv.h"
//#include "../../sdk/include/pal/timer.h"
//#include "../../sdk/include/nor/nor.h"
#include "../../sdk/include/mmp_nor.h"
/****************************************************************************
 *
 * _f_poweron
 *
 * This function is called to simulate power on
 * Function which should call f_initvolume for the drive to be
 * tested - which must be drive 0 ("A").
 *
 * RETURNS
 *
 * 0 if success or error code
 *
 ***************************************************************************/

int _f_poweron()
{
	int ret;

	ret=f_init();
	if (ret) return ret;

	ret=f_enterFS();
	if (ret) return ret;

	//ret=f_initvolume(0,f_testramdrvinit,0);
	//ret = f_initvolume(0, ftl_initfunc, 0);//f_xdinit
    //ret = f_initvolume(0, f_xdinit, 0);
ret = f_initvolume(0, NorInitial, 0);

	if (ret) return ret;

	return f_chdrive(0);
}

/****************************************************************************
 *
 * _f_poweroff
 *
 * This function is called to simulate power off
 *
 * RETURNS
 *
 * 0 if success or error code
 *
 ***************************************************************************/

int _f_poweroff()
{
	return f_delvolume(0);
}

/****************************************************************************
 *
 * _f_dump
 *
 * Simple dump (print) string
 *
 * INPUTS
 *
 * s - string to print
 *
 ***************************************************************************/

void _f_dump(char *s)
{
	(void)printf ("%s\n",s);
}

/****************************************************************************
 *
 * _f_result
 *
 * Called when a test line is failed to print error
 *
 * INPUTS
 *
 * line - test.c line where the test failed
 *        if line parameter is -1 then result contains the number of errors
 * result - test result to print
 *
 * RETURNS
 *
 * Always return with 1
 *
 ***************************************************************************/

int _f_result(int line,long result)
{
	if (line!=-1)
	{
		(void)printf ("ERROR: test.c at line %d result %d\n",line,result);
	}
	else
	{
		(void)printf ("ERROR: found %d error(s)\n",result);
	}
	return 1;
}

/****************************************************************************
 *
 * end of testport_f.c
 *
 ***************************************************************************/

