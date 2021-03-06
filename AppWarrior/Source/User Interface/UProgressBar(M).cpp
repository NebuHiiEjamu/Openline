#if MACINTOSH

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "UProgressBar.h"
#include <QuickDraw.h>

/*
 * Function Prototypes
 */

static void _DrawProgBar(Rect r, Uint32 val, Uint32 max);
static void _DrawProgBarBW(Rect r, Uint32 val, Uint32 max);
static void _CalcRgnProgBar(RgnHandle rgn, Rect r, Uint32 val, Uint32 max);

GrafPtr _ImageToGrafPtr(TImage inImage);

/* -------------------------------------------------------------------------- */

void UProgressBar_Draw(TImage inImage, const SRect& inBounds, const SProgressBarInfo& inInfo)
{
	const Rect bounds = { inBounds.top, inBounds.left, inBounds.bottom, inBounds.right };

	::SetPort(_ImageToGrafPtr(inImage));

	// if cannot draw via appearance manager, do it via internal funcs
	if (inInfo.options & 0x8000)	// if b/w
		_DrawProgBarBW(bounds, inInfo.val, inInfo.max);
	else
		_DrawProgBar(bounds, inInfo.val, inInfo.max);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

static void _CalcRgnProgBar(RgnHandle rgn, Rect r, Uint32 /* val */, Uint32 /* max */)
{
	if (!rgn) return;
	r.bottom = r.top + 13 < r.bottom ? r.top + 13 : r.bottom;
	RectRgn(rgn, &r);
}

const RGBColor blue6C = { 0x6666, 0x6666, 0xCCCC };
const RGBColor blue9F = { 0x9999, 0x9999, 0xFFFF };
const RGBColor blueCF = { 0xCCCC, 0xCCCC, 0xFFFF };
const RGBColor blue08 = { 0x0000, 0x0000, 0x8888 };
const RGBColor blue39 = { 0x3333, 0x3333, 0x9999 };

// width 100, height 13
static void _DrawProgBar(Rect r, Uint32 inVal, Uint32 max)
{
	Rect tr;
	
	if (inVal > max) inVal = max;
			
	// frame
	r.bottom = r.top + 13;
	tr = r;
	InsetRect(&tr, 1, 1);
	PenNormal();
	RGBForeColor((RGBColor *)&color_Black);
	FrameRect(&tr);
	
	// frame shading - top and left
	RGBForeColor((RGBColor *)&color_GrayA);
	MoveTo(r.left, r.top);
	LineTo(r.right-2, r.top);
	MoveTo(r.left, r.top);
	LineTo(r.left, r.bottom-2);
	
	// frame shading - bottom and right
	RGBForeColor((RGBColor *)&color_White);
	MoveTo(r.left+1, r.bottom-1);
	LineTo(r.right-1, r.bottom-1);
	MoveTo(r.right-1, r.top+1);
	LineTo(r.right-1, r.bottom-1);
	
	// frame shading - corners
	SetCPixel(r.left, r.bottom-1, (RGBColor *)&color_GrayD);
	SetCPixel(r.right-1, r.top, (RGBColor *)&color_GrayD);
	
	// calc val pix
	// this works, but can easily overflow and stuff
	//Uint32 val = r.left+2 + ((inVal * (r.right - r.left - 2)) / max);
	
	Uint32 val;
	if (max == 0)
		val = 0;
	else
		val = (((fast_float)inVal / (fast_float)max) * (r.right - r.left - 2));
	val += r.left + 2;

	if (r.right - val < 3) val = r.right-1;
	if (val - r.left < 4) val = r.left+1;
	
	if (val - r.left > 9)
	{
		// draw fill body
		RGBForeColor(&blue6C);
		MoveTo(r.left+4, r.top+2);
		LineTo(val-4, r.top+2);
		RGBForeColor(&blue9F);
		MoveTo(r.left+4, r.top+3);
		LineTo(val-4, r.top+3);
		RGBForeColor(&blueCF);
		MoveTo(r.left+4, r.top+4);
		LineTo(val-4, r.top+4);
		RGBForeColor((RGBColor *)&color_White);
		MoveTo(r.left+4, r.top+5);
		LineTo(val-4, r.top+5);
		RGBForeColor(&blueCF);
		MoveTo(r.left+4, r.top+6);
		LineTo(val-4, r.top+6);
		RGBForeColor(&blue9F);
		MoveTo(r.left+4, r.top+7);
		LineTo(val-4, r.top+7);
		RGBForeColor(&blue6C);
		MoveTo(r.left+4, r.top+8);
		LineTo(val-4, r.top+8);
		RGBForeColor(&blue39);
		MoveTo(r.left+4, r.top+9);
		LineTo(val-4, r.top+9);
		RGBForeColor(&blue08);
		MoveTo(r.left+4, r.top+10);
		LineTo(val-4, r.top+10);
	
		// draw fill start
		RGBForeColor(&blue6C);
		MoveTo(r.left+2, r.top+2);
		LineTo(r.left+2, r.bottom-3);
		SetCPixel(r.left+3, r.top+2, &blue9F);
		SetCPixel(r.left+3, r.top+3, &blueCF);
		SetCPixel(r.left+3, r.top+4, (RGBColor *)&color_White);
		SetCPixel(r.left+3, r.top+5, (RGBColor *)&color_White);
		SetCPixel(r.left+3, r.top+6, (RGBColor *)&color_White);
		SetCPixel(r.left+3, r.top+7, &blueCF);
		SetCPixel(r.left+3, r.top+8, &blue9F);
		SetCPixel(r.left+3, r.top+9, &blue6C);
		SetCPixel(r.left+3, r.top+10, &blue39);
		
		// draw fill end
		RGBForeColor((RGBColor *)&color_Black);
		MoveTo(val-1, r.top+2);
		LineTo(val-1, r.bottom-3);
		RGBForeColor(&blue08);
		MoveTo(val-2, r.top+2);
		LineTo(val-2, r.bottom-3);
		RGBForeColor(&blue39);
		MoveTo(val-3, r.top+3);
		LineTo(val-3, r.bottom-4);
		SetCPixel(val-3, r.top+2, &blue6C);
		SetCPixel(val-3, r.bottom-3, &blue08);
	}
	else if (val - r.left > 3)
	{
		tr.left = r.left+2;
		tr.top = r.top+2;
		tr.bottom = r.bottom-2;
		tr.right = val-1;
		RGBForeColor(&blue6C);
		PaintRect(&tr);
		
		RGBForeColor((RGBColor *)&color_Black);
		MoveTo(val-1, r.top+2);
		LineTo(val-1, r.bottom-3);
	}
	
	if (r.right - val > 5)
	{
		// draw bkgnd
		RGBForeColor((RGBColor *)&color_GrayB);
		tr.top = r.top+3;
		tr.bottom = r.bottom-3;
		tr.right = r.right-3;
		tr.left = val+2;
		PaintRect(&tr);
		
		// bkgnd right
		RGBForeColor((RGBColor *)&color_White);
		MoveTo(r.right-3, r.top+3);
		LineTo(r.right-3, r.bottom-4);
		SetCPixel(r.right-3, r.top+2, (RGBColor *)&color_GrayB);
		
		// bkgnd left
		RGBForeColor((RGBColor *)&color_Gray5);
		MoveTo(val, r.top+2);
		LineTo(val, r.bottom-3);
		RGBForeColor((RGBColor *)&color_Gray8);
		MoveTo(val+1, r.top+2);
		LineTo(val+1, r.bottom-3);
		RGBForeColor((RGBColor *)&color_Black);
		MoveTo(r.left+1, r.top+1);
		LineTo(r.left+1, r.bottom-2);
		
		// bkgnd top and bottom
		RGBForeColor((RGBColor *)&color_Gray8);
		MoveTo(val+2, r.top+2);
		LineTo(r.right-4, r.top+2);
		RGBForeColor((RGBColor *)&color_GrayD);
		MoveTo(val+2, r.bottom-3);
		LineTo(r.right-3, r.bottom-3);
	}
	else if (r.right - val > 0)
	{
		tr.top = r.top+2;
		tr.bottom = r.bottom-2;
		tr.left = val;
		tr.right = r.right-2;
		RGBForeColor((RGBColor *)&color_Gray8);
		PaintRect(&tr);
	}
}

static void _DrawProgBarBW(Rect r, Uint32 inVal, Uint32 max)
{
	RGBColor saveBack;
	Rect tr;
	
	if (inVal > max) inVal = max;
	
	// set white back
	GetBackColor(&saveBack);
	RGBBackColor((RGBColor *)&color_White);
	PenNormal();
	
	// frame
	r.bottom = r.top + 13;
	tr = r;
	InsetRect(&tr, 1, 1);
	PenNormal();
	RGBForeColor((RGBColor *)&color_Black);
	FrameRect(&tr);
	
	// calc val pix
	// this works, but can easily overflow and stuff
	//Uint32 val = r.left+2 + ((inVal * (r.right - r.left - 2)) / max);

	Uint32 val;
	if (max == 0)
		val = 0;
	else
		val = (((fast_float)inVal / (fast_float)max) * (r.right - r.left - 2));
	val += r.left + 2;

	// fill
	tr = r;
	InsetRect(&tr, 2, 2);
	tr.right = val;
	if (tr.right > r.right-2) tr.right = r.right-2;
	PaintRect(&tr);
	
	// bkgnd
	tr = r;
	InsetRect(&tr, 2, 2);
	tr.left = val;
	if (tr.left < r.left+2) tr.left = r.left+2;
	if (r.right - tr.left > 1)
	{
		RGBForeColor((RGBColor *)&color_White);
		PaintRect(&tr);
	}
	
	// restore back
	RGBBackColor(&saveBack);
}

#endif /* MACINTOSH */
