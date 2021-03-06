#if WIN32

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "UMouse.h"


extern HINSTANCE _gProgramInstance;

static HCURSOR _gArrowCursor = nil;
static HCURSOR _gIBeamCursor = nil;
static HCURSOR _gCrossCursor = nil;
static HCURSOR _gWaitCursor = nil;
static HCURSOR _gSizeCursor = nil;
static HCURSOR _gSizeWECursor = nil;
static HCURSOR _gSizeNSCursor = nil;
static HCURSOR _gSizeNWSECursor = nil;
static HCURSOR _gSizeNESWCursor = nil;
static HCURSOR _gNoCursor = nil;
static Int32 _gCurCursorID = 0;

/* -------------------------------------------------------------------------- */

void UMouse::SetImage(Int32 inID)
{
	HCURSOR hCursor = NULL;
	
	if (_gCurCursorID == inID) return;
	
	switch (inID)
	{
		case mouseImage_Standard:
		case mouseImage_StandardAdd:
		case mouseImage_StandardLink:
		case mouseImage_StandardMenu:
			if (_gArrowCursor == nil) _gArrowCursor = ::LoadCursor(NULL, IDC_ARROW);
			hCursor = _gArrowCursor;
			break;
		case mouseImage_Text:
			if (_gIBeamCursor == nil) _gIBeamCursor = ::LoadCursor(NULL, IDC_IBEAM);
			hCursor = _gIBeamCursor;
			break;
		case mouseImage_Cross:
		case mouseImage_FatCross:
			if (_gCrossCursor == nil) _gCrossCursor = ::LoadCursor(NULL, IDC_CROSS);
			hCursor = _gCrossCursor;
			break;
		case mouseImage_Wait:
			if (_gWaitCursor == nil) _gWaitCursor = ::LoadCursor(NULL, IDC_WAIT);
			hCursor = _gWaitCursor;
			break;
		case mouseImage_No:
			if (_gNoCursor == nil) _gNoCursor = ::LoadCursor(NULL, IDC_NO);
			hCursor = _gNoCursor;
			break;
		case mouseImage_Size:
			if (_gSizeCursor == nil) _gSizeCursor = ::LoadCursor(NULL, IDC_SIZE);
			hCursor = _gSizeCursor;
			break;
		case mouseImage_SizeWE:
			if (_gSizeWECursor == nil) _gSizeWECursor = ::LoadCursor(NULL, IDC_SIZEWE);
			hCursor = _gSizeWECursor;
			break;
		case mouseImage_SizeNS:
			if (_gSizeNSCursor == nil) _gSizeNSCursor = ::LoadCursor(NULL, IDC_SIZENS);
			hCursor = _gSizeNSCursor;
			break;
		case mouseImage_SizeNWSE:
			if (_gSizeNWSECursor == nil) _gSizeNWSECursor = ::LoadCursor(NULL, IDC_SIZENWSE);
			hCursor = _gSizeNWSECursor;
			break;
		case mouseImage_SizeNESW:
			if (_gSizeNESWCursor == nil) _gSizeNESWCursor = ::LoadCursor(NULL, IDC_SIZENESW);
			hCursor = _gSizeNESWCursor;
			break;
		default:
			hCursor = ::LoadCursor(_gProgramInstance, MAKEINTRESOURCE(inID));
			break;
	}
	
	if (hCursor)
	{
		::SetCursor(hCursor);
		_gCurCursorID = inID;
	}
}

Int32 UMouse::GetImage()
{
	return _gCurCursorID;
}

/*
 * ResetImage() sets the image of the mouse/cursor/pointer to its standard
 * image (typically an arrow).
 */
void UMouse::ResetImage()
{
	if (_gArrowCursor == nil) _gArrowCursor = ::LoadCursor(NULL, IDC_ARROW);
	::SetCursor(_gArrowCursor);
	_gCurCursorID = 0;
}

void UMouse::Hide()
{
	::ShowCursor(FALSE);
}

void UMouse::Show()
{
	::ShowCursor(TRUE);
}

/*
 * Obscure() temporarily hides the cursor - the cursor is redisplayed the next
 * time the user moves the mouse.
 */
void UMouse::Obscure()
{
	// don't support this
}

/*
 * GetLocation() sets <outLoc> to the location of the cursor in global
 * (screen) coordinates.
 */
void UMouse::GetLocation(SPoint& outLoc)
{
	::GetCursorPos((POINT *)&outLoc);
}

/*
 * SetLocation() sets the location of the cursor to <inLoc>, which should
 * be in global (screen) coordinates.
 */
void UMouse::SetLocation(const SPoint& inLoc)
{
	::SetCursorPos(inLoc.x, inLoc.y);
}

/*
 * IsDown() returns whether or not the specified button is currently
 * being held down by the user.
 */
bool UMouse::IsDown(Uint16 inButton)
{
	switch (inButton)
	{
		// logical buttons
		case mouseButton_Default:
		case mouseButton_Left:
			return ::GetAsyncKeyState(::GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON) != 0;
		case mouseButton_Right:
			return ::GetAsyncKeyState(::GetSystemMetrics(SM_SWAPBUTTON) ? VK_LBUTTON : VK_RBUTTON) != 0;
		case mouseButton_Middle:
			return ::GetAsyncKeyState(VK_MBUTTON) != 0;
		
		// physical buttons
		case mouseButton_First:
			return ::GetAsyncKeyState(VK_LBUTTON) != 0;
		case mouseButton_Second:
			return ::GetAsyncKeyState((::GetSystemMetrics(SM_CMOUSEBUTTONS) == 2) ? VK_RBUTTON : VK_MBUTTON) != 0;
		case mouseButton_Third:
			if (::GetSystemMetrics(SM_CMOUSEBUTTONS) >= 3)
				return ::GetAsyncKeyState(VK_RBUTTON) != 0;
			break;
	}
	
	return false;
}

/*
 * IsStillDown() returns whether or not the specified button has been
 * continuously pressed since the most recent mouse down message.
 */
bool UMouse::IsStillDown(Uint16 inButton)
{
	switch (inButton)
	{
		case mouseButton_Default:
		case mouseButton_Left:
			return ::GetKeyState(VK_LBUTTON) != 0;
		case mouseButton_Right:
			return ::GetKeyState(VK_RBUTTON) != 0;
		case mouseButton_Middle:
			return ::GetKeyState(VK_MBUTTON) != 0;
	}
	
	return false;
}

/*
 * To determine whether a sequence of mouse events constitutes a double click, measure the
 * elapsed time (in milliseconds) between a mouse-up and a mouse-down.  If the time between
 * the two mouse events is less than the value returned by GetDoubleClickTime(), the two
 * mouse events can then be interpreted as a double click.
 */
Uint32 UMouse::GetDoubleClickTime()
{
	// don't know how to get the value from the Mouse control panel, so default to 500
	//return 500;
	
	return ::GetDoubleClickTime();
}

/*
 * WaitMouseDownMove() waits for either the mouse to move from the location
 * <inInitialLoc> or for the mouse button to be released.  If the mouse moves
 * away from the location before the mouse button is released, WaitMouseDownMove()
 * returns true.  If the mouse button is released before the mouse moved from the
 * location, WaitMouseDownMove() returns false.
 */
bool UMouse::WaitMouseDownMove(const SPoint& inInitialLoc, Uint16 inButton)
{
	POINT pt;

	while (UMouse::IsStillDown(inButton))
	{
		::GetCursorPos(&pt);
		
		if (((abs(inInitialLoc.x - pt.x) > 3) || (abs(inInitialLoc.y - pt.y) > 3) ))
			return true;
	}
	
	return false;
}






#endif
