//*****************************************************************************
// Name: MMP_FILE.h
//
// Description:
//     mmp file system API header file
//
// Author: teddy
// Version: 0.2
// Date:
//          v0.1    2004/07/07
//          V0.2    2004/07/21
//
// Copyright(c)2003-2004 Silicon Integrated Systems Corp. All rights reserved.
//*****************************************************************************

#ifndef __MMP_FILE_H
#define __MMP_FILE_H

#include "mmp_types.h"


#ifdef __cplusplus
extern "C"
{
#endif // End of #ifdef __cplusplus

#if defined(_WIN32)
    #if defined(FS_EXPORTS)
        #define FS_API __declspec(dllexport)
    #else
        #define FS_API __declspec(dllimport)
    #endif
#else
    #define FS_API extern
#endif


typedef void FILE_t;

#if defined(WIN32) //&& defined(WIN32_HD)
//  #include <stdio.h>
#endif// End of WIN32

#ifdef LINUX

#endif// End of LINUX

#ifdef WINCE

#endif// End of WINCE


#if defined(_WIN32) && !defined(_INC_STDIO)
#include <stddef.h>
//#define size_t    unsigned int
#define FILE        FILE_t
#define fopen       mfsFopen
#define fclose      mfsFclose
#define fseek       mfsFseek
#define fread       mfsFread
#define fwrite      mfsFwrite
#define ftell       mfsFtell
#define feof        mfsFeof
#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2
#endif

#if defined(NUCLEUS_PLUS) 

#include "stddef.h"
//#define size_t    unsigned int
//#define NULL (void *)0

#define FILE        FILE_t
#define fopen       mfsFopen
#define fclose      mfsFclose
#define fseek       mfsFseek
#define fread       mfsFread
#define fwrite      mfsFwrite
#define ftell       mfsFtell
#define feof        mfsFeof
#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2


#endif /*** END OF #if defined ***/

MMP_BOOL
MMP_fsinit(
    void);

FS_API MMP_BOOL 
mfsInitialize(
    void);
    
FS_API MMP_BOOL 
mfsTerminate(
    void);

FS_API FILE_t*
mfsFopen(
    const char *path, 
    const char *mode);

FS_API int 
mfsFclose(
    FILE_t *fp);

FS_API int 
mfsFseek(
    FILE_t *fp, 
    long int offset, 
    int whence);

FS_API size_t 
mfsFread(
    void *buf,
    size_t size, 
    size_t n, 
    FILE_t *fp);

FS_API size_t 
mfsFwrite(
    void *buf,
    size_t size, 
    size_t n, 
    FILE_t *fp);

FS_API int 
mfsFeof(
    FILE_t *fp);

FS_API long 
mfsFtell(
    FILE_t *fp);

FS_API MMP_BOOL 
mfsFormat(
    int mode);

FS_API MMP_ULONG
mfsGetTotalSpace( 
    void); 

FS_API MMP_ULONG 
mfsGetUsedSpace( 
    void); 

FS_API MMP_ULONG 
mfsGetFreeSpace( 
    void);

#ifdef __cplusplus
}
#endif // End of #ifdef __cplusplus
#endif
