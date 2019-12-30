/*
 	File:		CMCalibrator.h
 
 	Contains:	ColorSync Calibration API
 
 	Version:	Technology:	ColorSync 2.5
 				Release:	QuickTime 4.0
 
 	Copyright:	(c) 1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		For bug reports, consult the following page on
 				the World Wide Web:
 
 					http://developer.apple.com/bugreporter/
 
*/
#ifndef __CMCALIBRATOR__
#define __CMCALIBRATOR__

#ifndef __CMAPPLICATION__
#include <CMApplication.h>
#endif
#ifndef __DISPLAYS__
#include <Displays.h>
#endif
#ifndef __ERRORS__
#include <Errors.h>
#endif



#if PRAGMA_ONCE
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_IMPORT
#pragma import on
#endif

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

typedef CALLBACK_API( void , CalibrateEventProcPtr )(EventRecord *event);
typedef STACK_UPP_TYPE(CalibrateEventProcPtr) 					CalibrateEventUPP;

struct CalibratorInfo {
	AVIDType 						displayID;
	CMProfileLocation 				profileLocation;
	CalibrateEventUPP 				eventProc;
	UInt32 							reserved;
	UInt32 							flags;
	Boolean 						isGood;
	SInt8 							byteFiller;
};
typedef struct CalibratorInfo			CalibratorInfo;
typedef CALLBACK_API( Boolean , CanCalibrateProcPtr )(AVIDType displayID);
typedef CALLBACK_API( OSErr , CalibrateProcPtr )(CalibratorInfo *theInfo);
typedef STACK_UPP_TYPE(CanCalibrateProcPtr) 					CanCalibrateUPP;
typedef STACK_UPP_TYPE(CalibrateProcPtr) 						CalibrateUPP;
enum { uppCalibrateEventProcInfo = 0x000000C0 }; 				/* pascal no_return_value Func(4_bytes) */
enum { uppCanCalibrateProcInfo = 0x000000D0 }; 					/* pascal 1_byte Func(4_bytes) */
enum { uppCalibrateProcInfo = 0x000000E0 }; 					/* pascal 2_bytes Func(4_bytes) */
#if MIXEDMODE_CALLS_ARE_FUNCTIONS
EXTERN_API(CalibrateEventUPP)
NewCalibrateEventProc		   (CalibrateEventProcPtr	userRoutine);
EXTERN_API(CanCalibrateUPP)
NewCanCalibrateProc			   (CanCalibrateProcPtr		userRoutine);
EXTERN_API(CalibrateUPP)
NewCalibrateProc			   (CalibrateProcPtr		userRoutine);
EXTERN_API(void)
CallCalibrateEventProc		   (CalibrateEventUPP		userRoutine,
								EventRecord *			event);
EXTERN_API(Boolean)
CallCanCalibrateProc		   (CanCalibrateUPP			userRoutine,
								AVIDType				displayID);
EXTERN_API(OSErr)
CallCalibrateProc			   (CalibrateUPP			userRoutine,
								CalibratorInfo *		theInfo);
#else
#define NewCalibrateEventProc(userRoutine) 						(CalibrateEventUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppCalibrateEventProcInfo, GetCurrentArchitecture())
#define NewCanCalibrateProc(userRoutine) 						(CanCalibrateUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppCanCalibrateProcInfo, GetCurrentArchitecture())
#define NewCalibrateProc(userRoutine) 							(CalibrateUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppCalibrateProcInfo, GetCurrentArchitecture())
#define CallCalibrateEventProc(userRoutine, event) 				CALL_ONE_PARAMETER_UPP((userRoutine), uppCalibrateEventProcInfo, (event))
#define CallCanCalibrateProc(userRoutine, displayID) 			CALL_ONE_PARAMETER_UPP((userRoutine), uppCanCalibrateProcInfo, (displayID))
#define CallCalibrateProc(userRoutine, theInfo) 				CALL_ONE_PARAMETER_UPP((userRoutine), uppCalibrateProcInfo, (theInfo))
#endif

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

#ifdef PRAGMA_IMPORT_OFF
#pragma import off
#elif PRAGMA_IMPORT
#pragma import reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CMCALIBRATOR__ */

