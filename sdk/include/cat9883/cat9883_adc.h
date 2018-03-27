///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <ADC.h>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2011/08/12
//   @fileversion: CAT9883_V43
//******************************************/

#define Reg_Pair_initial(x)     Load_Reg_ini(x, (sizeof(x)/sizeof(x[0])))

MMP_UINT16 CAT9883_InWidth;
MMP_UINT16 CAT9883_InHeight;
MMP_UINT16 CAT9883_InFrameRate;
MMP_UINT16 CAT9883_InIsInterlace;
MMP_UINT16 CAT9883_InIsTVMode;


MMP_BOOL Mode_stable(MMP_BOOL isFrameRateCheck);

void ADC_Mode_Change();

void CAT9883CInitial();
void CAT9883Cmodechange();

void ModeDetectingprocess();
void AutoColorCalibration();

MMP_UINT16 Hsync_counter();
MMP_UINT16 Vsync_timer();
MMP_UINT16 Get_mode_number(MMP_BOOL isFrameRateCheck);
void Set_mode(MMP_UINT16);
MMP_BOOL Is_TV_mode();

MMP_UINT16 TV_mode(MMP_BOOL isFrameRateCheck);
MMP_UINT16 PC_mode();

MMP_UINT16 Frame_rate();

void Set_CAT9883_Tri_State_Enable();
void Set_CAT9883_Tri_State_Disable();

void CAT9883_PowerDown(
    MMP_BOOL enable);

MMP_BOOL CAT9883_IsNTSCorPAL(
    void);

