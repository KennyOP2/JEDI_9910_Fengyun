#ifndef _DEFS_H_
#define _DEFS_H_

/****************************************************************************
 *
 *            Copyright (c) 2003 by HCC Embedded 
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
 * Budapest 1132
 * Victor Hugo Utca 11-15
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

#define F_FILE FN_FILE
#define F_FIND FN_FIND
#define F_SPACE FN_SPACE
#define F_MAXPATH FN_MAXPATH
#define F_SEEK_SET FN_SEEK_SET
#define F_SEEK_END FN_SEEK_END
#define F_SEEK_CUR FN_SEEK_CUR

#define f_init fn_init
#define f_getversion fm_getversion
#define f_createdriver(driver,driver_init,driver_param) fm_createdriver(driver,driver_init,driver_param)
#define f_releasedriver(driver)	fn_releasedriver(driver)
#define f_createpartition(driver,parnum,par) fm_createpartition(driver,parnum,par)
#define f_getpartition(driver,parnum,par) fm_getpartition(driver,parnum,par)
#define f_initvolume(drvnumber,driver_init,driver_param) fm_initvolume(drvnumber,driver_init,driver_param)
#define f_initvolumepartition(drvnumber,driver,partition) fm_initvolumepartition(drvnumber,driver,partition)

#define f_delvolume(drvnumber) fm_delvolume(drvnumber)
#define f_get_volume_count() fm_get_volume_count()
#define f_get_volume_list(buf) fm_get_volume_list(buf)
#define f_checkvolume(drvnumber) fm_checkvolume(drvnumber)
#define f_format(drivenum,fattype) fm_format(drivenum,fattype)
#define f_getcwd(buffer,maxlen) fm_getcwd(buffer,maxlen)
#define f_getdcwd(drivenum,buffer,maxlen) fm_getdcwd(drivenum,buffer,maxlen)
#define f_chdrive(drivenum) fm_chdrive(drivenum)
#define f_getdrive fm_getdrive
#define f_getfreespace(drivenum,pspace) fm_getfreespace(drivenum,pspace)

#define f_chdir(dirname) fm_chdir(dirname)
#define f_mkdir(dirname) fm_mkdir(dirname)
#define f_rmdir(dirname) fm_rmdir(dirname)

#define f_findfirst(filename,find) fm_findfirst(filename,find)
#define f_findnext(find) fm_findnext(find)
#define f_rename(filename,newname) fm_rename(filename,newname)
#define f_move(filename,newname) fm_move(filename,newname)
#define f_filelength(filename) fm_filelength(filename)

#define f_close(filehandle) fm_close(filehandle)
#define f_flush(filehandle) fm_flush(filehandle)
#define f_open(filename,mode) fm_open(filename,mode)
#define f_truncate(filename,length) fm_truncate(filename,length)
#define f_ftruncate(filehandle,length) fm_ftruncate(filehandle,length)

#define f_read(buf,size,size_st,filehandle) fm_read(buf,size,size_st,filehandle)
#define f_write(buf,size,size_st,filehandle) fm_write(buf,size,size_st,filehandle)

#define f_seek(filehandle,offset,whence) fm_seek(filehandle,offset,whence)
#define f_seteof(filehandle) fm_seteof(filehandle)

#define f_tell(filehandle) fm_tell(filehandle)
#define f_getc(filehandle) fm_getc(filehandle)
#define f_putc(ch,filehandle) fm_putc(ch,filehandle)
#define f_rewind(filehandle) fm_rewind(filehandle)
#define f_eof(filehandle) fm_eof(filehandle)

#define f_stat(filename,stat) fm_stat(filename,stat)
#define f_gettimedate(filename,pctime,pcdate) fm_gettimedate(filename,pctime,pcdate)
#define f_settimedate(filename,ctime,cdate) fm_settimedate(filename,ctime,cdate)
#define f_delete(filename) fm_delete(filename)

#define f_getattr(filename,attr) fm_getattr(filename,attr)
#define f_setattr(filename,attr) fm_setattr(filename,attr)

#define f_getlabel(drivenum,label,len) fm_getlabel(drivenum,label,len)
#define f_setlabel(drivenum,label) fm_setlabel(drivenum,label)

#define f_get_oem(drivenum,str,maxlen) fm_get_oem(drivenum,str,maxlen)

extern void f_releaseFS(long);

#ifdef HCC_UNICODE
#define f_wgetcwd(buffer,maxlen) fm_wgetcwd(buffer,maxlen)
#define f_wgetdcwd(drivenum,buffer,maxlen) fm_wgetdcwd(drivenum,buffer,maxlen)
#define f_wchdir(dirname) fm_wchdir(dirname)
#define f_wmkdir(dirname) fm_wmkdir(dirname)
#define f_wrmdir(dirname) fm_wrmdir(dirname)
#define f_wfindfirst(filename,find) fm_wfindfirst(filename,find)
#define f_wfindnext(find) fm_wfindnext(find)
#define f_wrename(filename,newname) fm_wrename(filename,newname)
#define f_wmove(filename,newname) fm_wmove(filename,newname)
#define f_wfilelength(filename) fm_wfilelength(filename)
#define f_wopen(filename,mode) fm_wopen(filename,mode)
#define f_wtruncate(filename,length) fm_wtruncate(filename,length)
#define f_wstat(filename,stat) fm_wstat(filename,stat)
#define f_wgettimedate(filename,pctime,pcdate) fm_wgettimedate(filename,pctime,pcdate)
#define f_wsettimedate(filename,ctime,cdate) fm_wsettimedate(filename,ctime,cdate)
#define f_wdelete(filename) fm_wdelete(filename)
#define f_wgetattr(filename,attr) fm_wgetattr(filename,attr)
#define f_wsetattr(filename,attr) fm_wsetattr(filename,attr)
#endif


#ifdef __cplusplus
}
#endif

/****************************************************************************
 *
 * end of defs.h
 *
 ***************************************************************************/

#endif /* _DEFS_H_ */

