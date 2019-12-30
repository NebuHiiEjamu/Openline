#if MACINTOSH

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "UIconButton.h"
#include <QuickDraw.h>
#include <Icons.h>

/*
 * Function Prototypes
 */

static void _DrawIconBtn(TImage inImage, Rect r, const SIconButtonInfo& inInfo, bool drawTitle);
static void _DrawIconBtnHilited(TImage inImage, Rect r, const SIconButtonInfo& inInfo, bool drawTitle);
static void _DrawIconBtnDisabled(TImage inImage, Rect r, const SIconButtonInfo& inInfo, bool drawTitle);
static void _DrawIconBtnBW(TImage inImage, Rect r, const SIconButtonInfo& inInfo, bool drawTitle);
static void _DrawIconBtnHilitedBW(TImage inImage, Rect r, const SIconButtonInfo& inInfo, bool drawTitle);
static void _DrawIconBtnDisabledBW(TImage inImage, Rect r, const SIconButtonInfo& inInfo, bool drawTitle);
static void _CalcIconBtnRects(Rect DestRect, const SIconButtonInfo& inInfo, Rect *ButtonRect, Rect *TextRect, Rect *IconRect);

GrafPtr _ImageToGrafPtr(TImage inImage);

/* -------------------------------------------------------------------------- */

void UIconButton_Draw(TImage inImage, const SRect& inBounds, const SIconButtonInfo& inInfo)
{
	Rect r = { inBounds.top, inBounds.left, inBounds.bottom, inBounds.right };

	::SetPort(_ImageToGrafPtr(inImage));

	if (inInfo.options & 0x8000)	// if b/w
		_DrawIconBtnBW(inImage, r, inInfo, true);
	else
		_DrawIconBtn(inImage, r, inInfo, true);
}

void UIconButton_DrawFocused(TImage inImage, const SRect& inBounds, const SIconButtonInfo& inInfo)
{
	UIconButton_Draw(inImage, inBounds, inInfo);
}

void UIconButton_DrawHilited(TImage inImage, const SRect& inBounds, const SIconButtonInfo& inInfo)
{
	Rect r = { inBounds.top, inBounds.left, inBounds.bottom, inBounds.right };

	::SetPort(_ImageToGrafPtr(inImage));
	
	if (inInfo.options & 0x8000)	// if b/w
		_DrawIconBtnHilitedBW(inImage, r, inInfo, true);
	else
		_DrawIconBtnHilited(inImage, r, inInfo, true);
}

void UIconButton_DrawDisabled(TImage inImage, const SRect& inBounds, const SIconButtonInfo& inInfo)
{
	Rect r = { inBounds.top, inBounds.left, inBounds.bottom, inBounds.right };

	::SetPort(_ImageToGrafPtr(inImage));

	if (inInfo.options & 0x8000)	// if b/w
		_DrawIconBtnDisabledBW(inImage, r, inInfo, true);
	else
		_DrawIconBtnDisabled(inImage, r, inInfo, true);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

#if 0
void _CalcRegion(const SIconButtonInfo& inInfo, const SRect& inBounds, TRegion outRgn)
{
	Require(outRgn);
	Rect btnRect, textRect, iconRect, tr;
	
	Rect r = { inBounds.top, inBounds.left, inBounds.bottom, inBounds.right };
	_CalcIconBtnRects(r, inInfo, &btnRect, &textRect, &iconRect);
	
	OpenRgn();
	
	// button Rect
	tr = btnRect;
	if (tr.top < r.top)			tr.top = r.top;
	if (tr.left < r.left)		tr.left = r.left;
	if (tr.bottom > r.bottom)	tr.bottom = r.bottom;
	if (tr.right > r.right)		tr.right = r.right;
	FrameRoundRect(&tr, 4, 4);
	
	// text Rect
	if (!inInfo.titleInside && inInfo.title != 0 && inInfo.titleLen != 0)
	{
		tr = textRect;
		if (tr.top < r.top)			tr.top = r.top;
		if (tr.left < r.left)		tr.left = r.left;
		if (tr.bottom > r.bottom)	tr.bottom = r.bottom;
		if (tr.right > r.right)		tr.right = r.right;
		FrameRect(&tr);
	}
	
	CloseRgn((RgnHandle)outRgn);
}
#endif

void _DrawCenteredText(const void *inText, Uint32 inLength, Rect& r);
void _DrawVCenteredText(const void *inText, Uint32 inLength, Rect& r, Int16 offset);

static void _CalcIconBtnRects(Rect DestRect, const SIconButtonInfo& inInfo, Rect *ButtonRect, Rect *TextRect, Rect *IconRect)
{
	FontInfo fi;
	Int16 a, b, c;
	Rect r;
	GetFontInfo(&fi);

	if (inInfo.options & iconBtn_TitleLeft)
	{
		*ButtonRect = DestRect;
		
		r = *ButtonRect;
		r.top += 4;
		r.left += 6;
		r.bottom -= 4;
		r.right = r.left + inInfo.icon->GetWidth();
		*IconRect = r;
		
		r = *ButtonRect;
		r.left = IconRect->right + 4;
		*TextRect = r;
	}
	else if (!(inInfo.options & iconBtn_TitleOutside))
	{
		// calc button Rect
		*ButtonRect = DestRect;
		
		// calc text Rect
		if (inInfo.title == 0 || inInfo.titleSize == 0)
			TextRect->left = TextRect->right = TextRect->bottom = TextRect->top = 0;
		else
		{
			*TextRect = DestRect;
			TextRect->top = TextRect->bottom - (fi.ascent + fi.descent) - 4;
			TextRect->bottom -= 4;
		}
		
		// calc icon Rect
		r = *ButtonRect;
		r.top += 4;
		r.bottom -= (TextRect->bottom - TextRect->top) + 4;
		*IconRect = r;
	}
	else
	{
		// calc text Rect
		if (inInfo.title == 0 || inInfo.titleSize == 0)
			TextRect->left = TextRect->right = TextRect->bottom = TextRect->top = 0;
		else
		{
			*TextRect = DestRect;
			TextRect->top = TextRect->bottom - (fi.ascent + fi.descent);
		}
		
		// calc button Rect
		*ButtonRect = DestRect;
		ButtonRect->right = ButtonRect->left + 44;
		a = ButtonRect->right - ButtonRect->left;
		if (a >= (DestRect.right - DestRect.left))
			ButtonRect->right = DestRect.right;
		else
		{
			// center horiz
			b = ((DestRect.right - DestRect.left) - a) / 2;
			ButtonRect->left += b;
			ButtonRect->right = ButtonRect->left + a;
		}
		ButtonRect->bottom = ButtonRect->top + 44;
		c = TextRect->bottom - TextRect->top;
		b = ButtonRect->bottom - ButtonRect->top;
		if (b >= ((DestRect.bottom - DestRect.top) - c))
			ButtonRect->bottom = DestRect.bottom - c;
		else
		{
			// center vert
			a = ((DestRect.bottom - DestRect.top) - b - c) / 2;
			ButtonRect->top += a;
			ButtonRect->bottom = ButtonRect->top + b;
		}
		
		// calc icon Rect
		*IconRect = *ButtonRect;	// icon is drawn centered within bounds
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

// width 44, height 44
static void _DrawIconBtn(TImage inImage, Rect r, const SIconButtonInfo& inInfo, bool drawTitle)
{
	Rect btnRect, textRect, iconRect, tr;
	_CalcIconBtnRects(r, inInfo, &btnRect, &textRect, &iconRect);

	// frame and body
	PenNormal();
	RGBForeColor((RGBColor *)&color_Black);
	FrameRoundRect(&btnRect, 4, 4);
	tr = btnRect;
	InsetRect(&tr, 3, 3);
	RGBForeColor((RGBColor *)&color_GrayD);
	PaintRect(&tr);

	// top shading
	MoveTo(btnRect.left+2, btnRect.top+1);
	LineTo(btnRect.right-3, btnRect.top+1);
	RGBForeColor((RGBColor *)&color_White);
	MoveTo(btnRect.left+2, btnRect.top+2);
	LineTo(btnRect.right-4, btnRect.top+2);
	
	// left shading
	RGBForeColor((RGBColor *)&color_GrayD);
	MoveTo(btnRect.left+1, btnRect.top+2);
	LineTo(btnRect.left+1, btnRect.bottom-3);
	RGBForeColor((RGBColor *)&color_White);
	MoveTo(btnRect.left+2, btnRect.top+2);
	LineTo(btnRect.left+2, btnRect.bottom-4);

	// bottom shading
	RGBForeColor((RGBColor *)&color_Gray7);
	MoveTo(btnRect.left+2, btnRect.bottom-2);
	LineTo(btnRect.right-3, btnRect.bottom-2);
	RGBForeColor((RGBColor *)&color_GrayA);
	MoveTo(btnRect.left+3, btnRect.bottom-3);
	LineTo(btnRect.right-4, btnRect.bottom-3);
	
	// right shading
	RGBForeColor((RGBColor *)&color_Gray7);
	MoveTo(btnRect.right-2, btnRect.top+2);
	LineTo(btnRect.right-2, btnRect.bottom-3);
	RGBForeColor((RGBColor *)&color_GrayA);
	MoveTo(btnRect.right-3, btnRect.top+3);
	LineTo(btnRect.right-3, btnRect.bottom-4);
	
	// corners
	SetCPixel(btnRect.left+1, btnRect.top+1, (RGBColor *)&color_White);
	SetCPixel(btnRect.right-2, btnRect.top+1, (RGBColor *)&color_GrayC);
	SetCPixel(btnRect.right-3, btnRect.top+2, (RGBColor *)&color_GrayC);
	SetCPixel(btnRect.left+1, btnRect.bottom-2, (RGBColor *)&color_GrayC);
	SetCPixel(btnRect.left+2, btnRect.bottom-3, (RGBColor *)&color_GrayC);
	SetCPixel(btnRect.right-2, btnRect.bottom-2, (RGBColor *)&color_Gray4);
	SetCPixel(btnRect.right-3, btnRect.bottom-3, (RGBColor *)&color_Gray7);

	// draw icon
	SRect lr(iconRect.left, iconRect.top, iconRect.right, iconRect.bottom);
	inInfo.icon->Draw(inImage, lr, align_Center, transform_None);

	// draw title
	if (drawTitle)
	{
		RGBForeColor((RGBColor *)&color_Black);
		TextMode(srcOr);
		if (inInfo.options & iconBtn_TitleLeft)
			_DrawVCenteredText(inInfo.title, inInfo.titleSize, textRect, textRect.left);
		else
			_DrawCenteredText(inInfo.title, inInfo.titleSize, textRect);
	}
}

static void _DrawIconBtnHilited(TImage inImage, Rect r, const SIconButtonInfo& inInfo, bool drawTitle)
{
	Rect btnRect, textRect, iconRect, tr;
	_CalcIconBtnRects(r, inInfo, &btnRect, &textRect, &iconRect);

	// frame and body
	PenNormal();
	RGBForeColor((RGBColor *)&color_Black);
	FrameRoundRect(&btnRect, 4, 4);
	tr = btnRect;
	InsetRect(&tr, 3, 3);
	RGBForeColor((RGBColor *)&color_Gray5);
	PaintRect(&tr);

	// top shading
	RGBForeColor((RGBColor *)&color_Gray2);
	MoveTo(btnRect.left+2, btnRect.top+1);
	LineTo(btnRect.right-3, btnRect.top+1);
	RGBForeColor((RGBColor *)&color_Gray4);
	MoveTo(btnRect.left+2, btnRect.top+2);
	LineTo(btnRect.right-4, btnRect.top+2);
	
	// left shading
	RGBForeColor((RGBColor *)&color_Gray2);
	MoveTo(btnRect.left+1, btnRect.top+2);
	LineTo(btnRect.left+1, btnRect.bottom-3);
	RGBForeColor((RGBColor *)&color_Gray4);
	MoveTo(btnRect.left+2, btnRect.top+2);
	LineTo(btnRect.left+2, btnRect.bottom-4);

	// bottom shading
	RGBForeColor((RGBColor *)&color_Gray8);
	MoveTo(btnRect.left+2, btnRect.bottom-2);
	LineTo(btnRect.right-3, btnRect.bottom-2);
	RGBForeColor((RGBColor *)&color_Gray6);
	MoveTo(btnRect.left+3, btnRect.bottom-3);
	LineTo(btnRect.right-4, btnRect.bottom-3);
	
	// right shading
	RGBForeColor((RGBColor *)&color_Gray8);
	MoveTo(btnRect.right-2, btnRect.top+2);
	LineTo(btnRect.right-2, btnRect.bottom-3);
	RGBForeColor((RGBColor *)&color_Gray6);
	MoveTo(btnRect.right-3, btnRect.top+3);
	LineTo(btnRect.right-3, btnRect.bottom-4);
	
	// corners
	SetCPixel(btnRect.left+1, btnRect.top+1, (RGBColor *)&color_Gray1);
	SetCPixel(btnRect.right-2, btnRect.top+1, (RGBColor *)&color_Gray4);
	SetCPixel(btnRect.right-3, btnRect.top+2, (RGBColor *)&color_Gray5);
	SetCPixel(btnRect.left+1, btnRect.bottom-2, (RGBColor *)&color_Gray4);
	SetCPixel(btnRect.left+2, btnRect.bottom-3, (RGBColor *)&color_Gray5);
	SetCPixel(btnRect.right-2, btnRect.bottom-2, (RGBColor *)&color_GrayA);
	SetCPixel(btnRect.right-3, btnRect.bottom-3, (RGBColor *)&color_Gray8);

	// draw icon
	SRect lr(iconRect.left, iconRect.top, iconRect.right, iconRect.bottom);
	inInfo.icon->Draw(inImage, lr, align_Center, transform_Dark);
	
	// draw title
	if (drawTitle)
	{
		RGBForeColor((RGBColor *)&color_White);
		TextMode(srcOr);
		if (inInfo.options & iconBtn_TitleLeft)
			_DrawVCenteredText(inInfo.title, inInfo.titleSize, textRect, textRect.left);
		else
			_DrawCenteredText(inInfo.title, inInfo.titleSize, textRect);
	}
}

static void _DrawIconBtnDisabled(TImage inImage, Rect r, const SIconButtonInfo& inInfo, bool drawTitle)
{
	Rect btnRect, textRect, iconRect, tr;
	_CalcIconBtnRects(r, inInfo, &btnRect, &textRect, &iconRect);

	// frame and body
	PenNormal();
	RGBForeColor((RGBColor *)&color_Gray8);
	FrameRoundRect(&btnRect, 4, 4);
	tr = btnRect;
	InsetRect(&tr, 3, 3);
	RGBForeColor((RGBColor *)&color_GrayD);
	PaintRect(&tr);

	// top shading
	RGBForeColor((RGBColor *)&color_GrayD);
	MoveTo(btnRect.left+2, btnRect.top+1);
	LineTo(btnRect.right-3, btnRect.top+1);
	RGBForeColor((RGBColor *)&color_White);
	MoveTo(btnRect.left+2, btnRect.top+2);
	LineTo(btnRect.right-4, btnRect.top+2);
	
	// left shading
	RGBForeColor((RGBColor *)&color_GrayD);
	MoveTo(btnRect.left+1, btnRect.top+2);
	LineTo(btnRect.left+1, btnRect.bottom-3);
	RGBForeColor((RGBColor *)&color_White);
	MoveTo(btnRect.left+2, btnRect.top+2);
	LineTo(btnRect.left+2, btnRect.bottom-4);

	// bottom shading
	RGBForeColor((RGBColor *)&color_GrayA);
	MoveTo(btnRect.left+2, btnRect.bottom-2);
	LineTo(btnRect.right-3, btnRect.bottom-2);
	RGBForeColor((RGBColor *)&color_GrayB);
	MoveTo(btnRect.left+3, btnRect.bottom-3);
	LineTo(btnRect.right-4, btnRect.bottom-3);
	
	// right shading
	RGBForeColor((RGBColor *)&color_GrayA);
	MoveTo(btnRect.right-2, btnRect.top+2);
	LineTo(btnRect.right-2, btnRect.bottom-3);
	RGBForeColor((RGBColor *)&color_GrayB);
	MoveTo(btnRect.right-3, btnRect.top+3);
	LineTo(btnRect.right-3, btnRect.bottom-4);
	
	// corners
	SetCPixel(btnRect.left+1, btnRect.top+1, (RGBColor *)&color_White);
	SetCPixel(btnRect.right-2, btnRect.top+1, (RGBColor *)&color_GrayC);
	SetCPixel(btnRect.right-3, btnRect.top+2, (RGBColor *)&color_GrayC);
	SetCPixel(btnRect.left+1, btnRect.bottom-2, (RGBColor *)&color_GrayC);
	SetCPixel(btnRect.left+2, btnRect.bottom-3, (RGBColor *)&color_GrayC);
	SetCPixel(btnRect.right-2, btnRect.bottom-2, (RGBColor *)&color_Gray8);
	SetCPixel(btnRect.right-3, btnRect.bottom-3, (RGBColor *)&color_GrayA);

	// draw icon
	SRect lr(iconRect.left, iconRect.top, iconRect.right, iconRect.bottom);
	inInfo.icon->Draw(inImage, lr, align_Center, transform_Light);
	
	// draw title
	if (drawTitle)
	{
		if (inInfo.options & iconBtn_TitleOutside)
			RGBForeColor((RGBColor *)&color_Black);
		else
			RGBForeColor((RGBColor *)&color_Gray7);
		
		TextMode(srcOr);
		if (inInfo.options & iconBtn_TitleLeft)
			_DrawVCenteredText(inInfo.title, inInfo.titleSize, textRect, textRect.left);
		else
			_DrawCenteredText(inInfo.title, inInfo.titleSize, textRect);
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

static void _DrawIconBtnBW(TImage inImage, Rect r, const SIconButtonInfo& inInfo, bool drawTitle)
{
	const Pattern grayPat = { 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55 };
	
	Rect btnRect, textRect, iconRect, tr;
	RGBColor saveBack;
	_CalcIconBtnRects(r, inInfo, &btnRect, &textRect, &iconRect);

	// set white back
	GetBackColor(&saveBack);
	RGBBackColor((RGBColor *)&color_White);
	
	PenNormal();
	RGBForeColor((RGBColor *)&color_White);
	tr = btnRect;
	InsetRect(&tr, 1, 1);
	PaintRect(&tr);
	RGBForeColor((RGBColor *)&color_Black);
	
	PenPat(&grayPat);
	tr = btnRect;
	MoveTo(tr.right - 2, tr.top + 1);
	LineTo(tr.right - 2, tr.bottom - 2);
	MoveTo(tr.right - 3, tr.top + 2);
	LineTo(tr.right - 3, tr.bottom - 3);
	
	MoveTo(tr.left + 1, tr.bottom - 2);
	LineTo(tr.right - 2, tr.bottom - 2);
	MoveTo(tr.left + 2, tr.bottom - 3);
	LineTo(tr.right - 3, tr.bottom - 3);
	
	// frame
	PenNormal();
	RGBForeColor((RGBColor *)&color_Black);
	FrameRoundRect(&btnRect, 4, 4);
	
	// draw icon
	SRect lr(iconRect.left, iconRect.top, iconRect.right, iconRect.bottom);
	inInfo.icon->Draw(inImage, lr, align_Center, transform_None);
	
	// draw title
	if (drawTitle)
	{
		RGBForeColor((RGBColor *)&color_Black);
		TextMode(srcOr);
		if (inInfo.options & iconBtn_TitleLeft)
			_DrawVCenteredText(inInfo.title, inInfo.titleSize, textRect, textRect.left);
		else
			_DrawCenteredText(inInfo.title, inInfo.titleSize, textRect);
	}
	
	RGBBackColor(&saveBack);
}

static void _DrawIconBtnHilitedBW(TImage inImage, Rect r, const SIconButtonInfo& inInfo, bool drawTitle)
{
	const Pattern grayPat = { 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55 };
	
	Rect btnRect, textRect, iconRect, tr;
	RGBColor saveBack;
	_CalcIconBtnRects(r, inInfo, &btnRect, &textRect, &iconRect);

	// set white back
	GetBackColor(&saveBack);
	RGBBackColor((RGBColor *)&color_White);

	PenNormal();
	RGBForeColor((RGBColor *)&color_Black);
	PaintRoundRect(&btnRect, 4, 4);
		
	PenPat(&grayPat);
	tr = btnRect;
	MoveTo(tr.right - 2, tr.top + 1);
	LineTo(tr.right - 2, tr.bottom - 2);
	MoveTo(tr.right - 3, tr.top + 2);
	LineTo(tr.right - 3, tr.bottom - 3);
	
	MoveTo(tr.left + 1, tr.bottom - 2);
	LineTo(tr.right - 2, tr.bottom - 2);
	MoveTo(tr.left + 2, tr.bottom - 3);
	LineTo(tr.right - 3, tr.bottom - 3);
	PenNormal();
	
	// draw icon
	SRect lr(iconRect.left, iconRect.top, iconRect.right, iconRect.bottom);
	inInfo.icon->Draw(inImage, lr, align_Center, transform_Dark);
	
	if (drawTitle)
	{
		// draw title
		RGBForeColor((RGBColor *)&color_White);
		TextMode(srcOr);
		if (inInfo.options & iconBtn_TitleLeft)
			_DrawVCenteredText(inInfo.title, inInfo.titleSize, textRect, textRect.left);
		else
			_DrawCenteredText(inInfo.title, inInfo.titleSize, textRect);
	}
	
	RGBBackColor(&saveBack);
}

static void _DrawIconBtnDisabledBW(TImage inImage, Rect r, const SIconButtonInfo& inInfo, bool drawTitle)
{
	Rect btnRect, textRect, iconRect;
	RGBColor saveBack;
	_CalcIconBtnRects(r, inInfo, &btnRect, &textRect, &iconRect);

	// set white back
	GetBackColor(&saveBack);
	RGBBackColor((RGBColor *)&color_White);
	
	// body
	PenNormal();
	RGBForeColor((RGBColor *)&color_White);
	PaintRoundRect(&btnRect, 4, 4);
	
	// frame
	RGBForeColor((RGBColor *)&color_Black);
	FrameRoundRect(&btnRect, 4, 4);
	
	// draw icon
	SRect lr(iconRect.left, iconRect.top, iconRect.right, iconRect.bottom);
	inInfo.icon->Draw(inImage, lr, align_Center, transform_Light);
	
	// draw title
	if (drawTitle)
	{
		RGBForeColor((RGBColor *)&color_Black);
		TextMode(srcOr);
		if (inInfo.options & iconBtn_TitleLeft)
			_DrawVCenteredText(inInfo.title, inInfo.titleSize, textRect, textRect.left);
		else
			_DrawCenteredText(inInfo.title, inInfo.titleSize, textRect);
	}
	
	RGBBackColor(&saveBack);
}


#endif /* MACINTOSH */
