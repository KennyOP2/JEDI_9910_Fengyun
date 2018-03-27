/*
 * Copyright (c) 2004 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * Platform definitions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef PAL_DEF_H
#define PAL_DEF_H

#include "mmp_types.h"

/** Unicode definition */
#define PAL_UNICODE

#ifdef PAL_UNICODE
    typedef MMP_WCHAR   PAL_TCHAR;   /**< 16-bit UNICODE character type */
#else
    typedef MMP_CHAR    PAL_TCHAR;   /**< 8-bit ANSI character type */
#endif // PAL_UNICODE

/** String definition */
#ifdef PAL_UNICODE
    #define PAL_T(x) L ## x /**< Unicode string type */
#else
    #define PAL_T(x) x      /**< ANSI string type */
#endif // PAL_UNICODE

#endif /* PAL_DEF_H */

