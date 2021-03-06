#if MACINTOSH

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "UPopupButton.h"

#include <QuickDraw.h>
#include <TextUtils.h>

/*
 * Function Prototypes
 */

static void _DrawPopup(Rect r, const void *title, Uint32 titleLen);
static void _DrawPopupDisabled(Rect r, const void *title, Uint32 titleLen);
static void _DrawPopupHilited(Rect r, const void *title, Uint32 titleLen);
static void _DrawPopupBW(Rect r, const void *title, Uint32 titleLen);
static void _DrawPopupDisabledBW(Rect r, const void *title, Uint32 titleLen);
static void _DrawPopupHilitedBW(Rect r, const void *title, Uint32 titleLen);

GrafPtr _ImageToGrafPtr(TImage inImage);

/* -------------------------------------------------------------------------- */

void UPopupButton::Draw(TImage inImage, const SRect& inBounds, const SPopupButtonInfo& inInfo)
{
	Rect r = { inBounds.top, inBounds.left, inBounds.bottom, inBounds.right };
	
	::SetPort(_ImageToGrafPtr(inImage));

	if (inInfo.options & 0x8000)	// if b/w
		_DrawPopupBW(r, inInfo.title, inInfo.titleSize);
	else
		_DrawPopup(r, inInfo.title, inInfo.titleSize);
}

void UPopupButton::DrawFocused(TImage inImage, const SRect& inBounds, const SPopupButtonInfo& inInfo)
{
	DrawHilited(inImage, inBounds, inInfo);
}

void UPopupButton::DrawHilited(TImage inImage, const SRect& inBounds, const SPopupButtonInfo& inInfo)
{
	Rect r = { inBounds.top, inBounds.left, inBounds.bottom, inBounds.right };

	::SetPort(_ImageToGrafPtr(inImage));
	
	if (inInfo.options & 0x8000)	// if b/w
		_DrawPopupHilitedBW(r, inInfo.title, inInfo.titleSize);
	else
		_DrawPopupHilited(r, inInfo.title, inInfo.titleSize);
}

void UPopupButton::DrawDisabled(TImage inImage, const SRect& inBounds, const SPopupButtonInfo& inInfo)
{
	Rect r = { inBounds.top, inBounds.left, inBounds.bottom, inBounds.right };

	::SetPort(_ImageToGrafPtr(inImage));
	
	if (inInfo.options & 0x8000)	// if b/w
		_DrawPopupDisabledBW(r, inInfo.title, inInfo.titleSize);
	else
		_DrawPopupDisabled(r, inInfo.title, inInfo.titleSize);
}

// returns true if you should draw the title
bool UPopupButton::GetTitleRect(const SRect& inBounds, SRect& outRect)
{
	outRect.top = inBounds.top + 2;
	outRect.bottom = inBounds.bottom - 2;
	outRect.left = inBounds.left + 2;
	outRect.right = inBounds.right - 22;
	
	return (outRect.GetHeight() >= 8) && (outRect.GetWidth() >= 16);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

#include <Script.h>

void _DrawVCenteredText(const void *inText, Uint32 inLength, Rect& r, Int16 offset);

static void _DrawTitle(const void *title, Uint32 titleLen, const Rect *inBounds, RGBColor *inColor)
{
	if (title)
	{
		Int8 buf[256];
		Int16 length;
		Rect ir = *inBounds;
	
		if (titleLen > sizeof(buf)) titleLen = sizeof(buf);
		BlockMoveData(title, buf, titleLen);
		
		length = titleLen;
		TruncText(ir.right - (ir.left+8), buf, &length, smTruncEnd);
		
		RGBForeColor(inColor);
		ir.bottom--;
		_DrawVCenteredText(buf, length, ir, ir.left+8);
	}
}


static void _DrawTri(Rect r)
{
	Int16 h, v;
	h = r.left + (((r.right - r.left)-8)/2) - 1;
	v = r.top + (((r.bottom - r.top)-5)/2);

	MoveTo(h, v);
	LineTo(h+8, v);
	MoveTo(h+1, v+1);
	LineTo(h+7, v+1);
	MoveTo(h+2, v+2);
	LineTo(h+6, v+2);
	MoveTo(h+3, v+3);
	LineTo(h+5, v+3);
	MoveTo(h+4, v+4);
	LineTo(h+4, v+4);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

static void _DrawNTPopup(Rect r)
{
	Rect tr;
	
	// frame
	PenNormal();
	RGBForeColor((RGBColor *)&color_Black);
	FrameRoundRect(&r, 8, 8);
	
	// body
	tr = r;
	InsetRect(&tr, 3, 3);
	RGBForeColor((RGBColor *)&color_GrayD);
	PaintRect(&tr);
	
	// top shading
	RGBForeColor((RGBColor *)&color_GrayD);
	MoveTo(r.left+2, r.top+1);
	LineTo(r.right-3, r.top+1);
	RGBForeColor((RGBColor *)&color_White);
	MoveTo(r.left+2, r.top+2);
	LineTo(r.right-4, r.top+2);
	SetCPixel(r.right-3, r.top+2, (RGBColor *)&color_GrayD);
	SetCPixel(r.right-2, r.top+2, (RGBColor *)&color_GrayA);
	
	// left shading
	RGBForeColor((RGBColor *)&color_GrayD);
	MoveTo(r.left+1, r.top+2);
	LineTo(r.left+1, r.bottom-3);
	RGBForeColor((RGBColor *)&color_White);
	MoveTo(r.left+2, r.top+2);
	LineTo(r.left+2, r.bottom-4);
	
	// bottom shading
	RGBForeColor((RGBColor *)&color_Gray8);
	MoveTo(r.left+2, r.bottom-2);
	LineTo(r.right-3, r.bottom-2);
	RGBForeColor((RGBColor *)&color_GrayA);
	MoveTo(r.left+3, r.bottom-3);
	LineTo(r.right-4, r.bottom-3);
	SetCPixel(r.left+2, r.bottom-3, (RGBColor *)&color_GrayD);

	// right shading
	RGBForeColor((RGBColor *)&color_Gray8);
	MoveTo(r.right-2, r.top+3);
	LineTo(r.right-2, r.bottom-3);
	RGBForeColor((RGBColor *)&color_GrayA);
	MoveTo(r.right-3, r.top+3);
	LineTo(r.right-3, r.bottom-4);
	SetCPixel(r.right-3, r.bottom-3, (RGBColor *)&color_Gray8);

	// triangle
	RGBForeColor((RGBColor *)&color_Black);
	_DrawTri(r);
}

// width 100, height 19
static void _DrawPopup(Rect r, const void *title, Uint32 titleLen)
{
	Rect br, ir, tr;
	
	//RGBForeColor(&color_BLUE);
	//PaintRect(&r);
	
	// calc rects
	br = r;
	br.left = br.right-22;
	if (br.left < r.left) br.left = r.left;
	ir = r;
	ir.right = br.left;
	
	// check if no title area
	if (ir.right - ir.left < 11)
	{
		_DrawNTPopup(r);
		return;
	}
	
	// frame
	PenNormal();
	RGBForeColor((RGBColor *)&color_Black);
	FrameRoundRect(&r, 8, 8);
	
	// title area
	if (ir.left != ir.right)
	{
		// body
		tr.top = ir.top+2;
		tr.left = ir.left+2;
		tr.bottom = ir.bottom-2;
		tr.right = ir.right-1;
		RGBForeColor((RGBColor *)&color_GrayD);
		PaintRect(&tr);
	
		// shading
		RGBForeColor((RGBColor *)&color_White);
		MoveTo(ir.left+2, ir.top+1);
		LineTo(ir.right-2, ir.top+1);
		MoveTo(ir.left+1, ir.top+2);
		LineTo(ir.left+1, ir.bottom-3);
		RGBForeColor((RGBColor *)&color_GrayA);
		MoveTo(ir.left+2, ir.bottom-2);
		LineTo(ir.right-1, ir.bottom-2);
		MoveTo(ir.right-1, ir.top+2);
		LineTo(ir.right-1, ir.bottom-3);
		SetCPixel(ir.right-1, ir.top+1, (RGBColor *)&color_GrayD);
		
		// title
		TextMode(srcOr);
		_DrawTitle(title, titleLen, &ir, (RGBColor *)&color_Black);
	}
	
	// triangle area
	{
		// body
		tr.top = br.top+3;
		tr.left = br.left+2;
		tr.bottom = br.bottom-3;
		tr.right = br.right-3;
		RGBForeColor((RGBColor *)&color_GrayD);
		PaintRect(&tr);
	
		// top shading
		RGBForeColor((RGBColor *)&color_GrayD);
		MoveTo(br.left+1, br.top+1);
		LineTo(br.right-3, br.top+1);
		RGBForeColor((RGBColor *)&color_White);
		MoveTo(br.left+1, br.top+2);
		LineTo(br.right-4, br.top+2);
		
		// left shading
		RGBForeColor((RGBColor *)&color_GrayD);
		MoveTo(br.left, br.top+2);
		LineTo(br.left, br.bottom-3);
		RGBForeColor((RGBColor *)&color_White);
		MoveTo(br.left+1, br.top+2);
		LineTo(br.left+1, br.bottom-4);
		
		// bottom shading
		RGBForeColor((RGBColor *)&color_Gray8);
		MoveTo(br.left+1, br.bottom-2);
		LineTo(br.right-3, br.bottom-2);
		RGBForeColor((RGBColor *)&color_GrayA);
		MoveTo(br.left+2, br.bottom-3);
		LineTo(br.right-4, br.bottom-3);
		
		// right shading
		RGBForeColor((RGBColor *)&color_Gray8);
		MoveTo(br.right-2, br.top+3);
		LineTo(br.right-2, br.bottom-3);
		RGBForeColor((RGBColor *)&color_GrayA);
		MoveTo(br.right-3, br.top+3);
		LineTo(br.right-3, br.bottom-4);
		
		// corners
		if (ir.left != ir.right)
		{
			SetCPixel(br.left, br.top+1, (RGBColor *)&color_GrayD);
			SetCPixel(br.left, br.bottom-2, (RGBColor *)&color_GrayD);
		}
		SetCPixel(br.left+1, br.bottom-3, (RGBColor *)&color_GrayD);
		SetCPixel(br.right-3, br.bottom-3, (RGBColor *)&color_Gray8);
		SetCPixel(br.right-3, br.top+2, (RGBColor *)&color_GrayD);
		SetCPixel(br.right-2, br.top+2, (RGBColor *)&color_GrayA);
		
		// triangle
		RGBForeColor((RGBColor *)&color_Black);
		_DrawTri(br);
	}
}

static void _DrawNTPopupDisabled(Rect r)
{
	Rect tr;
	
	// frame
	PenNormal();
	RGBForeColor((RGBColor *)&color_Gray8);
	FrameRoundRect(&r, 8, 8);
	
	// body
	tr = r;
	InsetRect(&tr, 3, 3);
	RGBForeColor((RGBColor *)&color_GrayD);
	PaintRect(&tr);
	
	// top shading
	RGBForeColor((RGBColor *)&color_GrayD);
	MoveTo(r.left+2, r.top+1);
	LineTo(r.right-3, r.top+1);
	RGBForeColor((RGBColor *)&color_White);
	MoveTo(r.left+2, r.top+2);
	LineTo(r.right-4, r.top+2);
	SetCPixel(r.right-3, r.top+2, (RGBColor *)&color_GrayD);
	SetCPixel(r.right-2, r.top+2, (RGBColor *)&color_GrayB);
	
	// left shading
	RGBForeColor((RGBColor *)&color_GrayD);
	MoveTo(r.left+1, r.top+2);
	LineTo(r.left+1, r.bottom-3);
	RGBForeColor((RGBColor *)&color_White);
	MoveTo(r.left+2, r.top+2);
	LineTo(r.left+2, r.bottom-4);
	
	// bottom shading
	RGBForeColor((RGBColor *)&color_GrayA);
	MoveTo(r.left+2, r.bottom-2);
	LineTo(r.right-3, r.bottom-2);
	RGBForeColor((RGBColor *)&color_GrayB);
	MoveTo(r.left+3, r.bottom-3);
	LineTo(r.right-4, r.bottom-3);
	SetCPixel(r.left+2, r.bottom-3, (RGBColor *)&color_GrayD);

	// right shading
	RGBForeColor((RGBColor *)&color_GrayA);
	MoveTo(r.right-2, r.top+3);
	LineTo(r.right-2, r.bottom-3);
	RGBForeColor((RGBColor *)&color_GrayB);
	MoveTo(r.right-3, r.top+3);
	LineTo(r.right-3, r.bottom-4);
	SetCPixel(r.right-3, r.bottom-3, (RGBColor *)&color_GrayA);

	// triangle
	RGBForeColor((RGBColor *)&color_Gray8);
	_DrawTri(r);
}

static void _DrawPopupDisabled(Rect r, const void *title, Uint32 titleLen)
{
	Rect br, ir, tr;
	
	//RGBForeColor(&color_BLUE);
	//PaintRect(&r);
	
	// calc rects
	br = r;
	br.left = br.right-22;
	if (br.left < r.left) br.left = r.left;
	ir = r;
	ir.right = br.left;
	
	// check if no title area
	if (ir.right - ir.left < 11)
	{
		_DrawNTPopupDisabled(r);
		return;
	}
	
	// frame
	PenNormal();
	RGBForeColor((RGBColor *)&color_Gray8);
	FrameRoundRect(&r, 8, 8);
	
	// title area
	if (ir.left != ir.right)
	{
		// body
		tr.top = ir.top+2;
		tr.left = ir.left+2;
		tr.bottom = ir.bottom-2;
		tr.right = ir.right-1;
		RGBForeColor((RGBColor *)&color_GrayD);
		PaintRect(&tr);
	
		// shading
		RGBForeColor((RGBColor *)&color_White);
		MoveTo(ir.left+2, ir.top+1);
		LineTo(ir.right-2, ir.top+1);
		MoveTo(ir.left+1, ir.top+2);
		LineTo(ir.left+1, ir.bottom-3);
		RGBForeColor((RGBColor *)&color_GrayB);
		MoveTo(ir.left+2, ir.bottom-2);
		LineTo(ir.right-1, ir.bottom-2);
		MoveTo(ir.right-1, ir.top+2);
		LineTo(ir.right-1, ir.bottom-3);
		SetCPixel(ir.right-1, ir.top+1, (RGBColor *)&color_GrayB);
		
		// title
		TextMode(srcOr);
		_DrawTitle(title, titleLen, &ir, (RGBColor *)&color_Gray6);
	}
	
	// triangle area
	{
		// body
		tr.top = br.top+3;
		tr.left = br.left+2;
		tr.bottom = br.bottom-3;
		tr.right = br.right-3;
		RGBForeColor((RGBColor *)&color_GrayD);
		PaintRect(&tr);
	
		// top shading
		RGBForeColor((RGBColor *)&color_GrayD);
		MoveTo(br.left+1, br.top+1);
		LineTo(br.right-3, br.top+1);
		RGBForeColor((RGBColor *)&color_White);
		MoveTo(br.left+1, br.top+2);
		LineTo(br.right-4, br.top+2);
		
		// left shading
		RGBForeColor((RGBColor *)&color_GrayD);
		MoveTo(br.left, br.top+2);
		LineTo(br.left, br.bottom-3);
		RGBForeColor((RGBColor *)&color_White);
		MoveTo(br.left+1, br.top+2);
		LineTo(br.left+1, br.bottom-4);
		
		// bottom shading
		RGBForeColor((RGBColor *)&color_GrayA);
		MoveTo(br.left+1, br.bottom-2);
		LineTo(br.right-3, br.bottom-2);
		RGBForeColor((RGBColor *)&color_GrayB);
		MoveTo(br.left+2, br.bottom-3);
		LineTo(br.right-4, br.bottom-3);
		
		// right shading
		RGBForeColor((RGBColor *)&color_GrayA);
		MoveTo(br.right-2, br.top+3);
		LineTo(br.right-2, br.bottom-3);
		RGBForeColor((RGBColor *)&color_GrayB);
		MoveTo(br.right-3, br.top+3);
		LineTo(br.right-3, br.bottom-4);
		
		// corners
		if (ir.left != ir.right)
		{
			SetCPixel(br.left, br.top+1, (RGBColor *)&color_GrayD);
			SetCPixel(br.left, br.bottom-2, (RGBColor *)&color_GrayD);
		}
		SetCPixel(br.left+1, br.bottom-3, (RGBColor *)&color_GrayD);
		SetCPixel(br.right-3, br.bottom-3, (RGBColor *)&color_GrayA);
		SetCPixel(br.right-3, br.top+2, (RGBColor *)&color_GrayD);
		SetCPixel(br.right-2, br.top+2, (RGBColor *)&color_GrayB);
		
		// triangle
		RGBForeColor((RGBColor *)&color_Gray8);
		_DrawTri(br);
	}
}

static void _DrawNTPopupHilited(Rect r)
{
	Rect tr;
	
	// frame
	PenNormal();
	RGBForeColor((RGBColor *)&color_Black);
	FrameRoundRect(&r, 8, 8);
	
	// body
	tr = r;
	InsetRect(&tr, 3, 3);
	RGBForeColor((RGBColor *)&color_Gray5);
	PaintRect(&tr);
	
	// top shading
	RGBForeColor((RGBColor *)&color_Gray2);
	MoveTo(r.left+2, r.top+1);
	LineTo(r.right-3, r.top+1);
	RGBForeColor((RGBColor *)&color_Gray4);
	MoveTo(r.left+2, r.top+2);
	LineTo(r.right-4, r.top+2);
	SetCPixel(r.right-3, r.top+2, (RGBColor *)&color_Gray5);
	SetCPixel(r.right-2, r.top+2, (RGBColor *)&color_Gray7);
	
	// left shading
	RGBForeColor((RGBColor *)&color_Gray2);
	MoveTo(r.left+1, r.top+2);
	LineTo(r.left+1, r.bottom-3);
	RGBForeColor((RGBColor *)&color_Gray4);
	MoveTo(r.left+2, r.top+2);
	LineTo(r.left+2, r.bottom-4);
	SetCPixel(r.top+2, r.top+2, (RGBColor *)&color_Gray2);
	SetCPixel(r.top+3, r.top+3, (RGBColor *)&color_Gray4);
	
	// bottom shading
	RGBForeColor((RGBColor *)&color_Gray7);
	MoveTo(r.left+2, r.bottom-2);
	LineTo(r.right-3, r.bottom-2);
	RGBForeColor((RGBColor *)&color_Gray6);
	MoveTo(r.left+3, r.bottom-3);
	LineTo(r.right-4, r.bottom-3);
	SetCPixel(r.left+2, r.bottom-3, (RGBColor *)&color_Gray5);

	// right shading
	RGBForeColor((RGBColor *)&color_Gray7);
	MoveTo(r.right-2, r.top+3);
	LineTo(r.right-2, r.bottom-3);
	RGBForeColor((RGBColor *)&color_Gray6);
	MoveTo(r.right-3, r.top+3);
	LineTo(r.right-3, r.bottom-4);
	SetCPixel(r.right-3, r.bottom-3, (RGBColor *)&color_Gray7);
	SetCPixel(r.right-4, r.bottom-4, (RGBColor *)&color_Gray6);

	// triangle
	RGBForeColor((RGBColor *)&color_White);
	_DrawTri(r);
}

static void _DrawPopupHilited(Rect r, const void *title, Uint32 titleLen)
{
	Rect br, ir, tr;
		
	// calc rects
	br = r;
	br.left = br.right-22;
	if (br.left < r.left) br.left = r.left;
	ir = r;
	ir.right = br.left;
	
	// check if no title area
	if (ir.right - ir.left < 11)
	{
		_DrawNTPopupHilited(r);
		return;
	}
	
	// frame
	PenNormal();
	RGBForeColor((RGBColor *)&color_Black);
	FrameRoundRect(&r, 8, 8);
	
	// title area
	if (ir.left != ir.right)
	{
		// body
		tr.top = ir.top+2;
		tr.left = ir.left+2;
		tr.bottom = ir.bottom-2;
		tr.right = ir.right-1;
		RGBForeColor((RGBColor *)&color_Gray5);
		PaintRect(&tr);
	
		// shading
		RGBForeColor((RGBColor *)&color_Gray4);
		MoveTo(ir.left+2, ir.top+1);
		LineTo(ir.right-2, ir.top+1);
		MoveTo(ir.left+1, ir.top+2);
		LineTo(ir.left+1, ir.bottom-3);
		RGBForeColor((RGBColor *)&color_Gray6);
		MoveTo(ir.left+2, ir.bottom-2);
		LineTo(ir.right-1, ir.bottom-2);
		MoveTo(ir.right-1, ir.top+2);
		LineTo(ir.right-1, ir.bottom-3);
		SetCPixel(ir.right-1, ir.top+1, (RGBColor *)&color_Gray5);
		SetCPixel(ir.left+2, ir.top+2, (RGBColor *)&color_Gray4);
		
		// title
		TextMode(srcOr);
		_DrawTitle(title, titleLen, &ir, (RGBColor *)&color_White);
	}
	
	// triangle area
	{
		// body
		tr.top = br.top+3;
		tr.left = br.left+2;
		tr.bottom = br.bottom-3;
		tr.right = br.right-3;
		RGBForeColor((RGBColor *)&color_Gray5);
		PaintRect(&tr);
	
		// top shading
		RGBForeColor((RGBColor *)&color_Gray2);
		MoveTo(br.left+1, br.top+1);
		LineTo(br.right-3, br.top+1);
		RGBForeColor((RGBColor *)&color_Gray4);
		MoveTo(br.left+1, br.top+2);
		LineTo(br.right-4, br.top+2);
		
		// left shading
		RGBForeColor((RGBColor *)&color_Gray5);
		MoveTo(br.left, br.top+2);
		LineTo(br.left, br.bottom-3);
		RGBForeColor((RGBColor *)&color_Gray4);
		MoveTo(br.left+1, br.top+2);
		LineTo(br.left+1, br.bottom-4);
		
		// bottom shading
		RGBForeColor((RGBColor *)&color_Gray7);
		MoveTo(br.left+1, br.bottom-2);
		LineTo(br.right-3, br.bottom-2);
		RGBForeColor((RGBColor *)&color_Gray6);
		MoveTo(br.left+2, br.bottom-3);
		LineTo(br.right-4, br.bottom-3);
		
		// right shading
		RGBForeColor((RGBColor *)&color_Gray7);
		MoveTo(br.right-2, br.top+3);
		LineTo(br.right-2, br.bottom-3);
		RGBForeColor((RGBColor *)&color_Gray6);
		MoveTo(br.right-3, br.top+3);
		LineTo(br.right-3, br.bottom-4);
		
		// corners
		if (ir.left != ir.right)
		{
			SetCPixel(br.left, br.top+1, (RGBColor *)&color_Gray5);
			SetCPixel(br.left, br.bottom-2, (RGBColor *)&color_Gray5);
		}
		SetCPixel(br.left+1, br.bottom-3, (RGBColor *)&color_Gray5);
		SetCPixel(br.right-3, br.bottom-3, (RGBColor *)&color_Gray7);
		SetCPixel(br.right-4, br.bottom-4, (RGBColor *)&color_Gray6);
		SetCPixel(br.right-3, br.top+2, (RGBColor *)&color_Gray5);
		SetCPixel(br.right-2, br.top+2, (RGBColor *)&color_Gray6);
		
		// triangle
		RGBForeColor((RGBColor *)&color_White);
		_DrawTri(br);
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

static void _DrawNTPopupBW(Rect r)
{
	// set white back
	RGBColor saveBack;
	GetBackColor(&saveBack);
	RGBBackColor((RGBColor *)&color_White);
	
	// frame and body
	PenNormal();
	RGBForeColor((RGBColor *)&color_White);
	PaintRoundRect(&r, 8, 8);
	RGBForeColor((RGBColor *)&color_Black);
	FrameRoundRect(&r, 8, 8);
		
	// triangle
	RGBForeColor((RGBColor *)&color_Black);
	_DrawTri(r);
	
	// restore back color
	RGBBackColor(&saveBack);
}

static void _DrawPopupBW(Rect r, const void *title, Uint32 titleLen)
{
	Rect br, ir, tr;
	RGBColor saveBack;
	
	// set white back
	GetBackColor(&saveBack);
	RGBBackColor((RGBColor *)&color_White);
			
	// calc rects
	br = r;
	br.left = br.right-22;
	if (br.left < r.left) br.left = r.left;
	ir = r;
	ir.right = br.left;
	
	// check if no title area
	if (ir.right - ir.left < 11)
	{
		_DrawNTPopupBW(r);
		RGBBackColor(&saveBack);
		return;
	}
	
	// frame and body
	PenNormal();
	RGBForeColor((RGBColor *)&color_White);
	PaintRoundRect(&r, 8, 8);
	RGBForeColor((RGBColor *)&color_Black);
	FrameRoundRect(&r, 8, 8);
	
	// title area
	if (ir.left != ir.right)
	{
		// body
		tr.top = ir.top+2;
		tr.left = ir.left+2;
		tr.bottom = ir.bottom-2;
		tr.right = ir.right-1;
			
		// title
		TextMode(srcOr);
		_DrawTitle(title, titleLen, &ir, (RGBColor *)&color_Black);
	}
	
	// triangle area
	{
		// body
		tr.top = br.top+3;
		tr.left = br.left+2;
		tr.bottom = br.bottom-3;
		tr.right = br.right-3;
			
		// triangle
		RGBForeColor((RGBColor *)&color_Black);
		_DrawTri(br);
	}
	
	// restore back color
	RGBBackColor(&saveBack);
}

static void _DrawNTPopupDisabledBW(Rect r)
{
	// set white back
	RGBColor saveBack;
	GetBackColor(&saveBack);
	RGBBackColor((RGBColor *)&color_White);
	
	// frame and body
	PenNormal();
	RGBForeColor((RGBColor *)&color_White);
	PaintRoundRect(&r, 8, 8);
	RGBForeColor((RGBColor *)&color_Black);
	FrameRoundRect(&r, 8, 8);
		
	// triangle
	RGBForeColor((RGBColor *)&color_Black);
	PenPat((PatPtr)&pattern_Gray);
	_DrawTri(r);
	
	// restore back color
	PenNormal();
	RGBBackColor(&saveBack);
}

static void _DrawPopupDisabledBW(Rect r, const void *title, Uint32 titleLen)
{
	Rect br, ir, tr;
	RGBColor saveBack;
	
	// set white back
	GetBackColor(&saveBack);
	RGBBackColor((RGBColor *)&color_White);
	
	// calc rects
	br = r;
	br.left = br.right-22;
	if (br.left < r.left) br.left = r.left;
	ir = r;
	ir.right = br.left;
	
	// check if no title area
	if (ir.right - ir.left < 11)
	{
		_DrawNTPopupDisabledBW(r);
		RGBBackColor(&saveBack);
		return;
	}
	
	// frame and body
	PenNormal();
	RGBForeColor((RGBColor *)&color_White);
	PaintRoundRect(&r, 8, 8);
	RGBForeColor((RGBColor *)&color_Black);
	FrameRoundRect(&r, 8, 8);
	
	// title area
	if (ir.left != ir.right)
	{
		// body
		tr.top = ir.top+2;
		tr.left = ir.left+2;
		tr.bottom = ir.bottom-2;
		tr.right = ir.right-1;
			
		// title
		TextMode(grayishTextOr);
		_DrawTitle(title, titleLen, &ir, (RGBColor *)&color_Black);
		TextMode(srcOr);
	}
	
	// triangle area
	{
		// body
		tr.top = br.top+3;
		tr.left = br.left+2;
		tr.bottom = br.bottom-3;
		tr.right = br.right-3;
			
		// triangle
		RGBForeColor((RGBColor *)&color_Black);
		_DrawTri(br);
	}
	
	// restore back color
	RGBBackColor(&saveBack);
}

static void _DrawNTPopupHilitedBW(Rect r)
{
	// set white back
	RGBColor saveBack;
	GetBackColor(&saveBack);
	RGBBackColor((RGBColor *)&color_White);
	
	// frame and body
	PenNormal();
	RGBForeColor((RGBColor *)&color_Black);
	PaintRoundRect(&r, 8, 8);
		
	// triangle
	RGBForeColor((RGBColor *)&color_White);
	_DrawTri(r);
	
	// restore back color
	RGBBackColor(&saveBack);
}

static void _DrawPopupHilitedBW(Rect r, const void *title, Uint32 titleLen)
{
	Rect br, ir, tr;
	RGBColor saveBack;
	
	// set white back
	GetBackColor(&saveBack);
	RGBBackColor((RGBColor *)&color_White);
			
	// calc rects
	br = r;
	br.left = br.right-22;
	if (br.left < r.left) br.left = r.left;
	ir = r;
	ir.right = br.left;
	
	// check if no title area
	if (ir.right - ir.left < 11)
	{
		_DrawNTPopupHilitedBW(r);
		RGBBackColor(&saveBack);
		return;
	}
	
	// frame and body
	PenNormal();
	RGBForeColor((RGBColor *)&color_Black);
	PaintRoundRect(&r, 8, 8);
	
	// title area
	if (ir.left != ir.right)
	{
		// body
		tr.top = ir.top+2;
		tr.left = ir.left+2;
		tr.bottom = ir.bottom-2;
		tr.right = ir.right-1;
			
		// title
		TextMode(srcOr);
		_DrawTitle(title, titleLen, &ir, (RGBColor *)&color_White);
	}
	
	// triangle area
	{
		// body
		tr.top = br.top+3;
		tr.left = br.left+2;
		tr.bottom = br.bottom-3;
		tr.right = br.right-3;
			
		// triangle
		RGBForeColor((RGBColor *)&color_White);
		_DrawTri(br);
	}
	
	// restore back color
	RGBBackColor(&saveBack);
}




#endif /* MACINTOSH */
