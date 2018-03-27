/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * SMedia IR Driver API header file.
 *
 * @author James Lin
 */

#ifndef MMP_IR_H
#define MMP_IR_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
	#if defined(IR_EXPORTS)
		#define IR_API __declspec(dllexport)
	#else
		#define IR_API __declspec(dllimport)
	#endif
#else
	#define IR_API extern
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "mmp_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Enumeration Type Definition
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

/** @defgroup group11 SMedia IR Driver API
 *  The IR module API.
 *  @{
 */

//=============================================================================
//                             IR Mode Functions
//=============================================================================
/**
 * Initialize IR module.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
IR_API MMP_RESULT
mmpIrInitialize(
    MMP_INT32 pllBypass);

/**
 * Terminate IR module.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
IR_API MMP_RESULT
mmpIrTerminate(
    void);

/**
 * Get the keycode. -1 indicates no key pressed.
 */
IR_API MMP_UINT32
mmpIrGetKey(
    void);

/**
 * Get the MSB keycode if the keycode is large than 32 bits.
 */
IR_API MMP_UINT32
mmpIrGetKeyH(
    void);

//@}
#ifdef __cplusplus
}
#endif

#endif
