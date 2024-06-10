//----------------------------------------------------------------------------------
//	FILE:			FlashingLeds-Settings.h
//
//	Description:    This file contains the definitions for this project, and is 
//					linked to both {ProjectName}-Main.c and {ProjectName}-ISR.asm .  
//
//	Type: 			Device Independent
//
//----------------------------------------------------------------------------------
//  Copyright Texas Instruments � 2010
//----------------------------------------------------------------------------------
//  Revision History:
//----------------------------------------------------------------------------------
//  Date	  | Description / Status
//----------------------------------------------------------------------------------
// 9 April 2010 - MB
//----------------------------------------------------------------------------------
#ifndef _PROJSETTINGS_H
#define _PROJSETTINGS_H

//**********************************************************************************
//  NOTE: WHEN CHANGING THIS FILE PLEASE REBUILD ALL
//**********************************************************************************

//==================================================================================
// Incremental Build options for System check-out
//==================================================================================
#define INCR_BUILD 1		//1 - Blink LD3 every 1s
							//2 - Blink LD3 every 2s
							//3 - 
							//4 - 
//==================================================================================
// System Settings
//----------------------------------------------------------------------------------
//Add any system specific setting below
#define EPWMn_DPL_ISR   1   // for EPWM triggered ISR set as 1

//==================================================================================
// Interrupt Framework options
//==================================================================================
// If using TI's Digital Power Library add interrupt settings here
#define CLAn_DPL_ISR	0	// for CLA Task n Triggered ISR set as 1

//----------------------------------------------------------------------------------
// If EPWM is triggering the CLAn_DPL__ISR = 1, then choose which Task is being triggered
// note the following code would clear the PWM interrupt,
// if an ADC interrupt is used to trigger the CLA leave the following all zeros
//----------------------------------------------------------------------------------
#define CLATASK1_EPWM1		1	// EPWM1 Interrupt provides CLA Task 1 trigger
#define CLATASK2_EPWM2		1	// EPWM2 Interrupt provides CLA Task 1 trigger
#define CLATASK3_EPWM3		0	// EPWM3 Interrupt provides CLA Task 1 trigger
#define CLATASK4_EPWM4		0	// EPWM4 Interrupt provides CLA Task 1 trigger
#define CLATASK5_EPWM5		0	// EPWM5 Interrupt provides CLA Task 1 trigger
#define CLATASK6_EPWM6		1	// EPWM6 Interrupt provides CLA Task 1 trigger
#define CLATASK7_EPWM7		1	// EPWM7 Interrupt provides CLA Task 1 trigger
// Task 8 is reserved for software based initialization when using DPlib
//----------------------------------------------------------------------------------
// If EPWMn_DPL_ISR = 1, then choose which module
//----------------------------------------------------------------------------------
#define EPWM1           1   // EPWM1 provides ISR trigger
#define EPWM2           1   // EPWM2 provides ISR trigger
#define EPWM3           0   // EPWM3 provides ISR trigger
#define EPWM4           0   // EPWM4 provides ISR trigger
#define EPWM5           0   // EPWM5 provides ISR trigger
#define EPWM6           1   // EPWM6 provides ISR trigger
#define EPWM7           1   // EPWM6 provides ISR trigger
#endif //_PROJSETTINGS_H

