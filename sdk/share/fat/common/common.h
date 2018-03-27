#ifndef _COMMON_H_
#define _COMMON_H_

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

#ifdef __cplusplus
extern "C" {
#endif

/* retry counter for read and write */
#define RDWR_RETRY	3

/* EXPORTS */

extern unsigned long drvbldnum;
extern FN_MUTEX_TYPE fat_gmutex;

extern int fn_setlasterror(F_MULTI *fm, int errorcode);
extern void fn_setlasterror_noret(F_MULTI *fm, int errorcode);
extern int fn_createdriver(F_MULTI *fm,F_DRIVER **driver,F_DRIVERINIT driver_init,unsigned long driver_param);
extern int fn_releasedriver(F_MULTI *fm,F_DRIVER *driver);
extern int fn_createpartition(F_MULTI *fm,F_DRIVER *driver,int parnum,const F_PARTITION *par);
extern int fn_initvolume(F_MULTI *fm,int drvnumber,F_DRIVERINIT driver_init,unsigned long driver_param);
extern int fn_initvolumepartition(F_MULTI *fm,int drvnumber,F_DRIVER *driver, int partition);
extern int fn_ftruncate(F_MULTI *fm,FN_FILE *filehandle,unsigned long length);
extern int fn_getpartition(F_DRIVER *driver,int parnum, F_PARTITION *par);

extern int _f_readsector(F_VOLUME *vi,void *data, unsigned long sector,int cnt);
extern int _f_writesector(F_VOLUME *vi,void *data, unsigned long sector, int cnt);
extern int _f_getclustervalue(F_VOLUME *vi,unsigned long cluster,unsigned long *pvalue);
extern void _f_clustertopos(const F_VOLUME *vi,unsigned long cluster, F_POS *pos);
extern unsigned long _f_getdecluster(const F_VOLUME *vi,F_DIRENTRY *de);
extern int _f_checkstatus(const F_VOLUME *vi,long *pstatus);
extern int _f_getvolume(F_MULTI *fm,int drivenum,F_VOLUME **pvi);
extern unsigned short _f_get16bitl(void *ptr);
extern unsigned long _f_get32bitl(void *ptr);
extern void _f_set16bitl(void *ptr,unsigned short num);
extern void _f_set32bitl(void *ptr,unsigned long num);
extern int _f_checklocked(long drvnum,const F_POS *pos);
extern int _f_checkreadlocked(F_VOLUME *vi,long drvnum,F_POS *pos,FN_FILEINT **fapp);
extern int _f_checkappendlocked(long drvnum,const F_POS *pos,FN_FILEINT *ofile);
extern void _f_initentry(F_DIRENTRY *de,const char *name,const char *ext);
extern int _f_alloccluster(F_VOLUME *vi,unsigned long *pcluster);
extern int _f_dobadblock(F_VOLUME *vi,FN_FILEINT *f);
extern int _f_setclustervalue(F_VOLUME *vi,unsigned long cluster, unsigned long data);
extern int _f_writefatsector(F_VOLUME *vi);
extern void _f_setdecluster(const F_VOLUME *vi,F_DIRENTRY *de,unsigned long cluster);
extern int _f_fseek(F_VOLUME *vi,F_MULTI *fm,FN_FILEINT *f,unsigned long offset);
extern int _f_getcurrsector(F_VOLUME *vi,FN_FILEINT *f,char *ptr,unsigned int *cnt);
extern int _f_removechain(F_VOLUME *vi,unsigned long cluster);
extern FN_FILEINT *_f_check_handle(FN_FILE *filehandle);
extern int _f_writedirsector(F_VOLUME *vi);
extern unsigned long _f_getmaxcluster(const F_VOLUME *vi);
extern int _f_writezeros(F_VOLUME *vi,F_MULTI *fm,FN_FILEINT *f,unsigned long num);
extern void _f_syncfiles(F_VOLUME *vi,F_MULTI *fm,const FN_FILEINT *file);
int _f_findopensize(unsigned long *ofsize,int drivenum,F_POS *pos);

#if F_MAXSEEKPOS
extern void _fn_removeseekpos(FN_FILEINT *f);
extern void _fn_updateseekpos(FN_FILEINT *f);
extern void _fn_initseekdivisor(FN_FILEINT *f,F_VOLUME *vi);
#endif

#ifdef FATCACHE_ENABLE
extern int _f_fatcache_flush (F_VOLUME *vi);
#endif

extern int _f_mutex_get(F_MULTI *fm,F_VOLUME *vi);
extern void _f_mutex_put(F_MULTI *fm);


#if (!FN_CAPI_USED)
#ifdef USE_MALLOC
extern F_MULTI *g_multi[FN_MAXTASK];
#else
extern F_MULTI g_multi[FN_MAXTASK];
#endif
#endif

extern int fnGetTask(F_MULTI **fm);

#ifdef INTERNAL_MEMFN
extern void *_f_memset(void *, int, unsigned long);
extern void *_f_memcpy(void *, void *, unsigned long);
#endif

#ifdef __cplusplus
}
#endif

/****************************************************************************
 *
 * end of common.h
 *
 ***************************************************************************/

#endif /* _COMMON_H_ */
