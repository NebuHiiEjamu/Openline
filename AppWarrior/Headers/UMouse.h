/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "GrafTypes.h"

// standard mouse images
enum {
	mouseImage_Standard		= 0,	// usually arrow
	mouseImage_Text			= 1,	// text I-beam
	mouseImage_Cross		= 2,	// crosshair
	mouseImage_FatCross		= 3,	// fat cross
	mouseImage_Wait			= 4,	// watch or hourglass
	mouseImage_No			= 5,	// slashed circle
	mouseImage_Size			= 6,	// four pointed arrow
	mouseImage_SizeWE		= 7,	// double pointed arrow west and east
	mouseImage_SizeNS		= 8,	// double pointed arrow north and south
	mouseImage_SizeNWSE		= 9,	// double pointed arrow northwest and southeast
	mouseImage_SizeNESW		= 10,	// double pointed arrow northeast and southwest
	mouseImage_Hand			= 11,	// hand for moving
	mouseImage_HandGrab		= 12,	// hand grabbing screen
	mouseImage_StandardAdd	= 13,	// arrow with little plus sign
	mouseImage_StandardLink	= 14,	// arrow with little arrow
	mouseImage_StandardMenu	= 15,	// arrow with little menu
	mouseImage_HandPoint	= 16,	// hand pointing
	mouseImage_SizeHeadWE	= 17,	// double pointed arrow west and east for resizing headings
	mouseImage_SizeHeadNS	= 18,	// double pointed arrow north and south for resizing headings

	mouseImage_Default		= mouseImage_Standard,
	mouseImage_Arrow		= mouseImage_Standard,
	mouseImage_SizeHoriz	= mouseImage_SizeWE,
	mouseImage_SizeVert		= mouseImage_SizeNS
}; 

// mouse buttons
enum {
	// physical buttons
	mouseButton_First		= 1,
	mouseButton_Second		= 2,
	mouseButton_Third		= 3,
	
	// logical buttons
	mouseButton_Default		= 0,
	mouseButton_Left		= 0xA0,
	mouseButton_Right		= 0xB0,
	mouseButton_Middle		= 0xC0
};

class UMouse
{
	public:
		// image
		static void SetImage(Int32 inID);
		static Int32 GetImage();
		static void ResetImage();
		
		// visibility
		static void Hide();
		static void Show();
		static void Obscure();
		
		// location
		static void GetLocation(SPoint& outLoc);
		static void SetLocation(const SPoint& inLoc);
		
		// buttons
		static bool IsDown(Uint16 inButton = 0);
		static bool IsStillDown(Uint16 inButton = 0);
		static Uint32 GetDoubleClickTime();
		
		// misc
		static bool WaitMouseDownMove(const SPoint& inInitialLoc, Uint16 inButton = 0);
};

class StMouseImage
{
	public:
		StMouseImage()				{	mSaveImage = UMouse::GetImage();							}
		StMouseImage(Int32 inID)	{	mSaveImage = UMouse::GetImage(); UMouse::SetImage(inID);	}
		~StMouseImage()				{	UMouse::SetImage(mSaveImage); 								}
		
	private:
		Int32 mSaveImage;
};


