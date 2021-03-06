#if WIN32

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "UCheckBox.h"

HDC _ImageToDC(TImage inImage);

enum {
	kCheckBoxSize	= 13
};

/* -------------------------------------------------------------------------- */

void UCheckBox_Draw(TImage inImage, const SRect& inBounds, const SCheckBoxInfo& inInfo)
{
	HDC dc = _ImageToDC(inImage);
	UINT state;
	COLORREF savedColor;
	RECT r;
	
	r.left = inBounds.left + 2;
	r.right = r.left + kCheckBoxSize;
	r.top = inBounds.top + ((inBounds.GetHeight() / 2) - (kCheckBoxSize / 2));
	r.bottom = r.top + kCheckBoxSize;
	
	state = (inInfo.style == 1) ? DFCS_BUTTONRADIO : DFCS_BUTTONCHECK;
	if (inInfo.mark == 1) state |= DFCS_CHECKED;
	
	::DrawFrameControl(dc, &r, DFC_BUTTON, state);
	
	if (inInfo.mark == 2)
	{
		HGDIOBJ savedPen = ::SelectObject(dc, ::CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_BTNTEXT)));
		::MoveToEx(dc, r.left + 4, r.top + kCheckBoxSize/2, NULL);
		::LineTo(dc, r.right - 4, r.top + kCheckBoxSize/2);
		::MoveToEx(dc, r.left + 4, r.top + kCheckBoxSize/2 + 1, NULL);
		::LineTo(dc, r.right - 4, r.top + kCheckBoxSize/2 + 1);
		::DeleteObject(::SelectObject(dc, savedPen));
	}
	
	if (inInfo.title)
	{
		r.left = inBounds.left + 2 + kCheckBoxSize + 4;
		r.top = inBounds.top;
		r.right = inBounds.right;
		r.bottom = inBounds.bottom;
		
		savedColor = ::SetTextColor(dc, ::GetSysColor(COLOR_BTNTEXT));
		::DrawTextA(dc, (char *)inInfo.title, inInfo.titleSize, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
		::SetTextColor(dc, savedColor);
	}
}

void UCheckBox_DrawFocused(TImage inImage, const SRect& inBounds, const SCheckBoxInfo& inInfo)
{
	HDC dc = _ImageToDC(inImage);
	UINT state;
	COLORREF savedColor;
	RECT r;
	
	r.left = inBounds.left + 2;
	r.right = r.left + kCheckBoxSize;
	r.top = inBounds.top + ((inBounds.GetHeight() / 2) - (kCheckBoxSize / 2));
	r.bottom = r.top + kCheckBoxSize;
	
	state = (inInfo.style == 1) ? DFCS_BUTTONRADIO : DFCS_BUTTONCHECK;
	if (inInfo.mark == 1) state |= DFCS_CHECKED;
	
	::DrawFrameControl(dc, &r, DFC_BUTTON, state);

	if (inInfo.mark == 2)
	{
		HGDIOBJ savedPen = ::SelectObject(dc, ::CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_BTNTEXT)));
		::MoveToEx(dc, r.left + 4, r.top + kCheckBoxSize/2, NULL);
		::LineTo(dc, r.right - 4, r.top + kCheckBoxSize/2);
		::MoveToEx(dc, r.left + 4, r.top + kCheckBoxSize/2 + 1, NULL);
		::LineTo(dc, r.right - 4, r.top + kCheckBoxSize/2 + 1);
		::DeleteObject(::SelectObject(dc, savedPen));
	}
	
	if (inInfo.title)
	{
		SIZE stTextSize;
		GetTextExtentPoint32(dc, (char *)inInfo.title, inInfo.titleSize, &stTextSize);
		
		RECT r;
		r.left = inBounds.left + 2 + kCheckBoxSize + 2;
		r.top = inBounds.top;
		r.right = r.left + stTextSize.cx + 6;
		r.bottom = inBounds.bottom;
		DrawFocusRect(dc, &r);

		r.left = inBounds.left + 2 + kCheckBoxSize + 4;
		r.top = inBounds.top;
		r.right = inBounds.right;
		r.bottom = inBounds.bottom;
		
		savedColor = ::SetTextColor(dc, ::GetSysColor(COLOR_BTNTEXT));
		::DrawTextA(dc, (char *)inInfo.title, inInfo.titleSize, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
		::SetTextColor(dc, savedColor);
	}	
}

void UCheckBox_DrawHilited(TImage inImage, const SRect& inBounds, const SCheckBoxInfo& inInfo)
{
	HDC dc = _ImageToDC(inImage);
	UINT state;
	COLORREF savedColor;
	RECT r;
	
	r.left = inBounds.left + 2;
	r.right = r.left + kCheckBoxSize;
	r.top = inBounds.top + ((inBounds.GetHeight() / 2) - (kCheckBoxSize / 2));
	r.bottom = r.top + kCheckBoxSize;
	
	state = (inInfo.style == 1) ? DFCS_BUTTONRADIO|DFCS_PUSHED : DFCS_BUTTONCHECK|DFCS_PUSHED;
	if (inInfo.mark == 1) state |= DFCS_CHECKED;
	
	::DrawFrameControl(dc, &r, DFC_BUTTON, state);

	if (inInfo.mark == 2)
	{
		HGDIOBJ savedPen = ::SelectObject(dc, ::CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_BTNTEXT)));
		::MoveToEx(dc, r.left + 4, r.top + kCheckBoxSize/2, NULL);
		::LineTo(dc, r.right - 4, r.top + kCheckBoxSize/2);
		::MoveToEx(dc, r.left + 4, r.top + kCheckBoxSize/2 + 1, NULL);
		::LineTo(dc, r.right - 4, r.top + kCheckBoxSize/2 + 1);
		::DeleteObject(::SelectObject(dc, savedPen));
	}

	if (inInfo.title)
	{
		SIZE stTextSize;
		GetTextExtentPoint32(dc, (char *)inInfo.title, inInfo.titleSize, &stTextSize);
	
		r.left = inBounds.left + 2 + kCheckBoxSize + 2;
		r.top = inBounds.top;
		r.right = r.left + stTextSize.cx + 6;
		r.bottom = inBounds.bottom;
		DrawFocusRect(dc, &r);

		r.left = inBounds.left + 2 + kCheckBoxSize + 4;
		r.top = inBounds.top;
		r.right = inBounds.right;
		r.bottom = inBounds.bottom;
		
		savedColor = ::SetTextColor(dc, ::GetSysColor(COLOR_BTNTEXT));
		::DrawTextA(dc, (char *)inInfo.title, inInfo.titleSize, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
		::SetTextColor(dc, savedColor);
	}
}

void UCheckBox_DrawDisabled(TImage inImage, const SRect& inBounds, const SCheckBoxInfo& inInfo)
{
	HDC dc = _ImageToDC(inImage);
	UINT state;
	COLORREF savedColor;
	RECT r;
	
	r.left = inBounds.left + 2;
	r.right = r.left + kCheckBoxSize;
	r.top = inBounds.top + ((inBounds.GetHeight() / 2) - (kCheckBoxSize / 2));
	r.bottom = r.top + kCheckBoxSize;
	
	state = (inInfo.style == 1) ? DFCS_BUTTONRADIO | DFCS_INACTIVE : DFCS_BUTTONCHECK | DFCS_INACTIVE;
	if (inInfo.mark == 1) state |= DFCS_CHECKED;
	
	::DrawFrameControl(dc, &r, DFC_BUTTON, state);
	
	if (inInfo.mark == 2)
	{
		HGDIOBJ savedPen = ::SelectObject(dc, ::CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DSHADOW)));
		::MoveToEx(dc, r.left + 4, r.top + kCheckBoxSize/2, NULL);
		::LineTo(dc, r.right - 4, r.top + kCheckBoxSize/2);
		::MoveToEx(dc, r.left + 4, r.top + kCheckBoxSize/2 + 1, NULL);
		::LineTo(dc, r.right - 4, r.top + kCheckBoxSize/2 + 1);
		::DeleteObject(::SelectObject(dc, savedPen));
	}

	if (inInfo.title)
	{
		r.left = inBounds.left + 2 + kCheckBoxSize + 4 + 1;
		r.top = inBounds.top + 1;
		r.right = inBounds.right + 1;
		r.bottom = inBounds.bottom + 1;
		
		savedColor = ::SetTextColor(dc, ::GetSysColor(COLOR_3DHILIGHT));
		::DrawTextA(dc, (char *)inInfo.title, inInfo.titleSize, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

		r.left--;
		r.top--;
		r.right--;
		r.bottom--;

		::SetTextColor(dc, ::GetSysColor(COLOR_3DSHADOW));
		::DrawTextA(dc, (char *)inInfo.title, inInfo.titleSize, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

		::SetTextColor(dc, savedColor);
	}
}

#endif /* WIN32 */
