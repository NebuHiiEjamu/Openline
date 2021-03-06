#if WIN32

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "UButton.h"

void _DrawButton(HDC inDC, const RECT *inRect, const void *inTitle, Uint32 inTitleSize, bool inEnabled, bool inFocused = false);
void _DrawButtonHilited(HDC inDC, const RECT *inRect, const void *inTitle, Uint32 inTitleSize, bool inFocused = false);
void _DrawButtonDefault(HDC inDC, const RECT *inRect, const void *inTitle, Uint32 inTitleSize, bool inFocused = false);

extern HBRUSH _3DFACE_BRUSH, _3DDKSHADOW_BRUSH;
extern HPEN _3DHILIGHT_PEN, _3DSHADOW_PEN, _3DDKSHADOW_PEN;

/* -------------------------------------------------------------------------- */

HDC _ImageToDC(TImage inImage);

void UButton_Draw(TImage inImage, const SRect& inBounds, const SButtonInfo& inInfo)
{
	HDC dc = _ImageToDC(inImage);
	
	if (inInfo.options & 1)		// if default
		_DrawButtonDefault(dc, (RECT *)&inBounds, inInfo.title, inInfo.titleSize);
	else
		_DrawButton(dc, (RECT *)&inBounds, inInfo.title, inInfo.titleSize, true);
}

void UButton_DrawFocused(TImage inImage, const SRect& inBounds, const SButtonInfo& inInfo)
{
	HDC dc = _ImageToDC(inImage);
	
	if (inInfo.options & 1)		// if default
		_DrawButtonDefault(dc, (RECT *)&inBounds, inInfo.title, inInfo.titleSize, true);
	else
		_DrawButton(dc, (RECT *)&inBounds, inInfo.title, inInfo.titleSize, true, true);
}

void UButton_DrawHilited(TImage inImage, const SRect& inBounds, const SButtonInfo& inInfo)
{
	_DrawButtonHilited(_ImageToDC(inImage), (RECT *)&inBounds, inInfo.title, inInfo.titleSize, true);
}

void UButton_DrawDisabled(TImage inImage, const SRect& inBounds, const SButtonInfo& inInfo)
{
	_DrawButton(_ImageToDC(inImage), (RECT *)&inBounds, inInfo.title, inInfo.titleSize, false, false);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void _DrawButton(HDC inDC, const RECT *inRect, const void *inTitle, Uint32 inTitleSize, bool inEnabled, bool inFocused)
{
	RECT r = *inRect;	
	
	::DrawFrameControl(inDC, &r, DFC_BUTTON, DFCS_BUTTONPUSH);
	
#if 0
	HPEN savedPen;
	
	SetROP2(inDC, R2_COPYPEN);
	
	// draw face
	r.top++;
	r.left++;
	r.bottom -= 2;
	r.right -= 2;
	FillRect(inDC, &r, _3DFACE_BRUSH);
	
	// draw light (top and left edges)
	r = *inRect;
	savedPen = SelectObject(inDC, _3DHILIGHT_PEN);
	MoveToEx(inDC, r.right-2, r.top, NULL);
	LineTo(inDC, r.left, r.top);
	LineTo(inDC, r.left, r.bottom-1);
	
	// draw dark (bottom and right edges)
	SelectObject(inDC, _3DSHADOW_PEN);
	MoveToEx(inDC, r.left+1, r.bottom-2, NULL);
	LineTo(inDC, r.right-2, r.bottom-2);
	LineTo(inDC, r.right-2, r.top);
	
	// draw extra dark (bottom and right edges)
	SelectObject(inDC, _3DDKSHADOW_PEN);
	MoveToEx(inDC, r.left, r.bottom-1, NULL);
	LineTo(inDC, r.right-1, r.bottom-1);
	LineTo(inDC, r.right-1, r.top-1);
	
	// restore pen
	SelectObject(inDC, savedPen);
#endif

	if (inEnabled && inFocused)
	{
		r.top += 3;
		r.left += 3;
		r.bottom -= 4;
		r.right -= 4;
		DrawFocusRect(inDC, &r);
	}

	// draw title
	if (inTitle)
	{
		r = *inRect;
		COLORREF savedColor;

		//SelectObject(inDC, GetStockObject(DEFAULT_GUI_FONT));
		//SetBkMode(inDC, TRANSPARENT);
		if (inEnabled)
		{
			savedColor = SetTextColor(inDC, GetSysColor(COLOR_BTNTEXT));
			DrawTextA(inDC, (char *)inTitle, inTitleSize, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);		
		}
		else
		{
			r.left++;
			r.top++;
			r.right++;
			r.bottom++;
			savedColor = SetTextColor(inDC, GetSysColor(COLOR_3DHILIGHT));
			DrawTextA(inDC, (char *)inTitle, inTitleSize, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

			SetTextColor(inDC, GetSysColor(COLOR_3DSHADOW));
			DrawTextA(inDC, (char *)inTitle, inTitleSize, (RECT *)inRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
		}
		
		// restore text color
		SetTextColor(inDC, savedColor);
	}
}

void _DrawButtonHilited(HDC inDC, const RECT *inRect, const void *inTitle, Uint32 inTitleSize, bool inFocused)
{
	RECT r = *inRect;
	
	::DrawFrameControl(inDC, &r, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED);
	
#if 0
	HPEN savedPen;

	SetROP2(inDC, R2_COPYPEN);

	// draw face
	r.top += 2;
	r.left += 2;
	r.bottom--;
	r.right--;
	FillRect(inDC, &r, _3DFACE_BRUSH);

	// draw extra dark (top and left edges)
	r = *inRect;
	savedPen = SelectObject(inDC, _3DDKSHADOW_PEN);
	MoveToEx(inDC, r.right-2, r.top, NULL);
	LineTo(inDC, r.left, r.top);
	LineTo(inDC, r.left, r.bottom-1);
	
	// draw dark (top and left edges)
	SelectObject(inDC, _3DSHADOW_PEN);
	MoveToEx(inDC, r.right-2, r.top+1, NULL);
	LineTo(inDC, r.left+1, r.top+1);
	LineTo(inDC, r.left+1, r.bottom-1);
	
	// draw light (bottom and right edges)
	SelectObject(inDC, _3DHILIGHT_PEN);
	MoveToEx(inDC, r.left, r.bottom-1, NULL);
	LineTo(inDC, r.right-1, r.bottom-1);
	LineTo(inDC, r.right-1, r.top-1);
	
	// restore pen
	SelectObject(inDC, savedPen);
#endif

	if (inFocused)
	{
		r.top += 3;
		r.left += 3;
		r.bottom -= 4;
		r.right -= 4;
		DrawFocusRect(inDC, &r);
	}

	// draw title
	if (inTitle)
	{
		r = *inRect;
		
		r.left++;
		r.top++;
		r.right++;
		r.bottom++;
		//SelectObject(inDC, GetStockObject(DEFAULT_GUI_FONT));
		//SetBkMode(inDC, TRANSPARENT);
		COLORREF savedColor = SetTextColor(inDC, GetSysColor(COLOR_BTNTEXT));
		DrawTextA(inDC, (char *)inTitle, inTitleSize, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
		SetTextColor(inDC, savedColor);
	}
}

void _DrawButtonDefault(HDC inDC, const RECT *inRect, const void *inTitle, Uint32 inTitleSize, bool inFocused)
{
	HPEN savedPen;
	COLORREF savedColor;
	RECT r;
	
	// draw face
	r = *inRect;
	r.top += 2;
	r.left += 2;
	r.bottom -= 3;
	r.right -= 3;
	FillRect(inDC, &r, _3DFACE_BRUSH);
	
	// draw extra dark border around button
	r = *inRect;
	FrameRect(inDC, &r, _3DDKSHADOW_BRUSH);
	
	// draw light (top and left edges)
	savedPen = (HPEN)SelectObject(inDC, _3DHILIGHT_PEN);
	MoveToEx(inDC, r.right-2, r.top+1, NULL);
	LineTo(inDC, r.left+1, r.top+1);
	LineTo(inDC, r.left+1, r.bottom-1);

	// draw dark (bottom and right edges)
	SelectObject(inDC, _3DSHADOW_PEN);
	MoveToEx(inDC, r.left+2, r.bottom-3, NULL);
	LineTo(inDC, r.right-3, r.bottom-3);
	LineTo(inDC, r.right-3, r.top+1);

	// draw extra dark (bottom and right edges)
	SelectObject(inDC, _3DDKSHADOW_PEN);
	MoveToEx(inDC, r.left+1, r.bottom-2, NULL);
	LineTo(inDC, r.right-2, r.bottom-2);
	LineTo(inDC, r.right-2, r.top);
	
	// restore pen
	SelectObject(inDC, savedPen);
	
	if (inFocused)
	{
		r.top += 3;
		r.left += 3;
		r.bottom -= 4;
		r.right -= 4;
		DrawFocusRect(inDC, &r);
	}
	
	// draw title
	if (inTitle)
	{
		r = *inRect;

		//SelectObject(inDC, GetStockObject(DEFAULT_GUI_FONT));
		//SetBkMode(inDC, TRANSPARENT);
		savedColor = SetTextColor(inDC, GetSysColor(COLOR_BTNTEXT));
		DrawTextA(inDC, (char *)inTitle, inTitleSize, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
		SetTextColor(inDC, savedColor);
	}
}




#endif /* WIN32 */
