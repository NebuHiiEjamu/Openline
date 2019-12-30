#if WIN32

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "UIconButton.h"

void _DrawButton(HDC inDC, const RECT *inRect, const void *inTitle, Uint32 inTitleSize, bool inEnabled, bool inFocused = false);
void _DrawButtonHilited(HDC inDC, const RECT *inRect, const void *inTitle, Uint32 inTitleSize, bool inFocused = false);
void _DrawIconButton(TImage inImage, const SRect& inBounds, const SIconButtonInfo& inInfo, bool inFocused = false);

HDC _ImageToDC(TImage inImage);

/* -------------------------------------------------------------------------- */

void UIconButton_Draw(TImage inImage, const SRect& inBounds, const SIconButtonInfo& inInfo)
{
	_DrawIconButton(inImage, inBounds, inInfo);
}

void UIconButton_DrawFocused(TImage inImage, const SRect& inBounds, const SIconButtonInfo& inInfo)
{
	_DrawIconButton(inImage, inBounds, inInfo, true);
}

void UIconButton_DrawHilited(TImage inImage, const SRect& inBounds, const SIconButtonInfo& inInfo)
{
	SIconButtonInfo info = inInfo;
	SRect buttonRect, textRect, iconRect;
	
	if (info.icon && info.iconLayer == 0)
		info.iconLayer = UPixmap::GetNearestLayer(info.icon, inImage, inBounds);
	
	UIconButton::CalcRects(inImage, inBounds, info, &buttonRect, &textRect, &iconRect);
		
	HDC dc = _ImageToDC(inImage);
	_DrawButtonHilited(dc, (RECT *)&buttonRect, nil, 0);
	
	if (info.title && info.titleSize)
	{
		textRect.Move(1, 1);
		COLORREF savedColor = ::SetTextColor(dc, ::GetSysColor(COLOR_BTNTEXT));
		::DrawTextA(dc, (char *)info.title, info.titleSize, (RECT *)&textRect, ((info.options & iconBtn_TitleLeft) ? DT_LEFT : DT_CENTER) | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
		::SetTextColor(dc, savedColor);
	}
	
	iconRect.Move(1, 1);
	UPixmap::Draw(info.icon, info.iconLayer, inImage, iconRect, align_Center, transform_Dark);
}

void UIconButton_DrawDisabled(TImage inImage, const SRect& inBounds, const SIconButtonInfo& inInfo)
{
	SIconButtonInfo info = inInfo;
	SRect buttonRect, textRect, iconRect;
	
	if (info.icon && info.iconLayer == 0)
		info.iconLayer = UPixmap::GetNearestLayer(info.icon, inImage, inBounds);
	
	UIconButton::CalcRects(inImage, inBounds, info, &buttonRect, &textRect, &iconRect);
	
	HDC dc = _ImageToDC(inImage);
	_DrawButton(dc, (RECT *)&buttonRect, nil, 0, false);
	
	if (info.title && info.titleSize)
	{
		buttonRect.left = textRect.left + 1;
		buttonRect.top = textRect.top + 1;
		buttonRect.right = textRect.right + 1;
		buttonRect.bottom = textRect.bottom + 1;
		
		COLORREF savedColor = ::SetTextColor(dc, ::GetSysColor(COLOR_3DHILIGHT));
		::DrawTextA(dc, (char *)info.title, info.titleSize, (RECT *)&buttonRect, ((info.options & iconBtn_TitleLeft) ? DT_LEFT : DT_CENTER) | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

		savedColor = ::SetTextColor(dc, ::GetSysColor(COLOR_3DSHADOW));
		::DrawTextA(dc, (char *)info.title, info.titleSize, (RECT *)&textRect, ((info.options & iconBtn_TitleLeft) ? DT_LEFT : DT_CENTER) | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

		::SetTextColor(dc, savedColor);
	}
	
	UPixmap::Draw(info.icon, info.iconLayer, inImage, iconRect, align_Center, transform_Light);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void _DrawIconButton(TImage inImage, const SRect& inBounds, const SIconButtonInfo& inInfo, bool inFocused)
{
	SIconButtonInfo info = inInfo;
	SRect buttonRect, textRect, iconRect;
	
	if (info.icon && info.iconLayer == 0)
		info.iconLayer = UPixmap::GetNearestLayer(info.icon, inImage, inBounds);
	
	UIconButton::CalcRects(inImage, inBounds, info, &buttonRect, &textRect, &iconRect);
	
	HDC dc = _ImageToDC(inImage);
	_DrawButton(dc, (RECT *)&buttonRect, nil, 0, true);
	
	if (info.title && info.titleSize)
	{
		COLORREF savedColor = ::SetTextColor(dc, ::GetSysColor(COLOR_BTNTEXT));
		::DrawTextA(dc, (char *)info.title, info.titleSize, (RECT *)&textRect, ((info.options & iconBtn_TitleLeft) ? DT_LEFT : DT_CENTER) | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
		::SetTextColor(dc, savedColor);
	}
	
	UPixmap::Draw(info.icon, info.iconLayer, inImage, iconRect, align_Center, transform_None);

	if (inFocused)
	{
		RECT r;
		r.top = buttonRect.top + 1;
		r.left = buttonRect.left + 1;
		r.bottom = buttonRect.bottom - 2;
		r.right = buttonRect.right - 2;
		
		DrawFocusRect(dc, &r);
	}
}

#endif /* WIN32 */

