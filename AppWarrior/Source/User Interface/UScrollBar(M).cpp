#if MACINTOSH

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "UScrollBar.h"
#include <QuickDraw.h>

/*
 * Function Prototypes
 */

static void _SBDrawUp(const SRect& inRect);
static void _SBDrawDown(const SRect& inRect);
static void _SBDrawLeft(const SRect& inRect);
static void _SBDrawRight(const SRect& inRect);
static void _SBDrawHilitedUp(const SRect& inRect);
static void _SBDrawHilitedDown(const SRect& inRect);
static void _SBDrawHilitedLeft(const SRect& inRect);
static void _SBDrawHilitedRight(const SRect& inRect);
static void _SBDrawDisabledUp(const SRect& inRect);
static void _SBDrawDisabledDown(const SRect& inRect);
static void _SBDrawDisabledLeft(const SRect& inRect);
static void _SBDrawDisabledRight(const SRect& inRect);

static void _SBDrawVPage(const SRect& inRect);
static void _SBDrawHPage(const SRect& inRect);
static void _SBDrawHilitedVPage(const SRect& inRect);
static void _SBDrawHilitedHPage(const SRect& inRect);
static void _SBDrawDisabledVPage(const SRect& inRect);
static void _SBDrawDisabledHPage(const SRect& inRect);

static void _SBDrawVThumb(const SRect& inRect);
static void _SBDrawHThumb(const SRect& inRect);
static void _SBDrawHilitedVThumb(const SRect& inRect);
static void _SBDrawHilitedHThumb(const SRect& inRect);

GrafPtr _ImageToGrafPtr(TImage inImage);

/* -------------------------------------------------------------------------- */

void UScrollBar_Draw(TImage inImage, const SRect& inBounds, const SScrollBarInfo& inInfo, Uint32 inHilitedPart)
{
	Require(inImage);
	
	::SetPort(_ImageToGrafPtr(inImage));

#if 0	
	const Rect rct = {	inBounds.top, inBounds.left, inBounds.bottom, inBounds.right	};
	
	if(_SBDrawScrollBarAppearance(&rct, inInfo.val, inInfo.max, inHilitedPart))
		return;
#endif

	SRect r;
	bool isHoriz = inBounds.GetWidth() > inBounds.GetHeight();

	UScrollBar::GetPartRect(inBounds, inInfo, sbPart_Thumb, r);
	if (!r.IsEmpty())
	{
		if (inHilitedPart == sbPart_Thumb)
			isHoriz ? _SBDrawHilitedHThumb(r) : _SBDrawHilitedVThumb(r);
		else
			isHoriz ? _SBDrawHThumb(r) : _SBDrawVThumb(r);
	}
	
	UScrollBar::GetPartRect(inBounds, inInfo, sbPart_PageUp, r);
	if (!r.IsEmpty())
	{
		if (inHilitedPart == sbPart_PageUp)
			isHoriz ? _SBDrawHilitedHPage(r) : _SBDrawHilitedVPage(r);
		else
			isHoriz ? _SBDrawHPage(r) : _SBDrawVPage(r);
	}
	
	UScrollBar::GetPartRect(inBounds, inInfo, sbPart_PageDown, r);
	if (!r.IsEmpty())
	{
		if (inHilitedPart == sbPart_PageDown)
			isHoriz ? _SBDrawHilitedHPage(r) : _SBDrawHilitedVPage(r);
		else
			isHoriz ? _SBDrawHPage(r) : _SBDrawVPage(r);
	}
	
	UScrollBar::GetPartRect(inBounds, inInfo, sbPart_Up, r);
	if (!r.IsEmpty())
	{
		if (inHilitedPart == sbPart_Up)
			isHoriz ? _SBDrawHilitedLeft(r) : _SBDrawHilitedUp(r);
		else
			isHoriz ? _SBDrawLeft(r) : _SBDrawUp(r);
	}
	
	UScrollBar::GetPartRect(inBounds, inInfo, sbPart_Down, r);
	if (!r.IsEmpty())
	{
		if (inHilitedPart == sbPart_Down)
			isHoriz ? _SBDrawHilitedRight(r) : _SBDrawHilitedDown(r);
		else
			isHoriz ? _SBDrawRight(r) : _SBDrawDown(r);
	}
}

void UScrollBar_DrawDisabled(TImage inImage, const SRect& inBounds, const SScrollBarInfo& inInfo)
{
	Require(inImage);

#if 0
	const Rect rct = {	inBounds.top, inBounds.left, inBounds.bottom, inBounds.right	};
	
	if(_SBDrawScrollBarAppearanceDisabled(&rct))
		return;
#endif

	SRect r;
	bool isHoriz = inBounds.GetWidth() > inBounds.GetHeight();
	
	::SetPort(_ImageToGrafPtr(inImage));
		
	if (isHoriz)
	{
		r.left = inBounds.left + 15;
		r.right = inBounds.right - 15;
		r.top = inBounds.top;
		r.bottom = r.top + 16;
	}
	else
	{
		r.top = inBounds.top + 15;
		r.bottom = inBounds.bottom - 15;
		r.left = inBounds.left;
		r.right = r.left + 16;
	}

	if (!r.IsEmpty()) isHoriz ? _SBDrawDisabledHPage(r) : _SBDrawDisabledVPage(r);
	
	UScrollBar::GetPartRect(inBounds, inInfo, sbPart_Up, r);
	if (!r.IsEmpty()) isHoriz ? _SBDrawDisabledLeft(r) : _SBDrawDisabledUp(r);
	
	UScrollBar::GetPartRect(inBounds, inInfo, sbPart_Down, r);
	if (!r.IsEmpty()) isHoriz ? _SBDrawDisabledRight(r) : _SBDrawDisabledDown(r);
}

Uint32 UScrollBar::PointToPart(const SRect& inBounds, const SScrollBarInfo& inInfo, const SPoint& inPoint)
{
#if 0
	const Point pt = { inPoint.y, inPoint.x	};
	const Rect rct = {	inBounds.top, inBounds.left, inBounds.bottom, inBounds.right	};
	Uint32 outPart;
	if(_SBGetControlPartCode(&rct, inInfo.val, inInfo.max, pt, outPart))
		return outPart;
#endif	

	SRect r;
	
	GetPartRect(inBounds, inInfo, sbPart_Thumb, r);
	if (r.Contains(inPoint)) return sbPart_Thumb;
	
	GetPartRect(inBounds, inInfo, sbPart_PageUp, r);
	if (r.Contains(inPoint)) return sbPart_PageUp;
	
	GetPartRect(inBounds, inInfo, sbPart_PageDown, r);
	if (r.Contains(inPoint)) return sbPart_PageDown;
	
	GetPartRect(inBounds, inInfo, sbPart_Up, r);
	if (r.Contains(inPoint)) return sbPart_Up;
	
	GetPartRect(inBounds, inInfo, sbPart_Down, r);
	if (r.Contains(inPoint)) return sbPart_Down;
	
	return 0;
}

void UScrollBar::GetPartRect(const SRect& inBounds, const SScrollBarInfo& inInfo, Uint32 inPart, SRect& outRect)
{
	SRect r = inBounds;
	SRect thumb;
	
	if (r.GetWidth() > r.GetHeight())
	{
		r.bottom = r.top + 16;
		
		if (inPart == sbPart_Left)
		{
			r.right = r.left + 16;
		}
		else if (inPart == sbPart_Right)
		{
			r.left = r.right - 16;
		}
		else if (r.GetWidth() > 55)
		{
			// calculate horiz thumb rect
			thumb = r;
			thumb.left = r.left + 15;
			if (inInfo.max != 0)					// avert divide-by-zero processor exception
				thumb.left += ((r.right - r.left - 47) * inInfo.val) / inInfo.max;
			thumb.right = thumb.left+17;
			
			// bring thumb into range
			if (thumb.left < r.left+15)
			{
				thumb.left = r.left+15;
				thumb.right = thumb.left+17;
			}
			if (thumb.right > r.right-15)
			{
				thumb.right = r.right-15;
				thumb.left = thumb.right-17;
			}
			
			// now that we have the thumb, we can continue
			if (inPart == sbPart_PageLeft)
			{
				r.left += 15;
				r.right = thumb.left + 1;
			}
			else if (inPart == sbPart_PageRight)
			{
				r.left = thumb.right - 1;
				r.right = r.right - 15;
			}
			else if (inPart == sbPart_Thumb)
			{
				r = thumb;
			}
			else
				r.SetEmpty();
		}
		else
			r.SetEmpty();
	}
	else
	{
		r.right = r.left + 16;
		
		if (inPart == sbPart_Up)
		{
			r.bottom = r.top + 16;
		}
		else if (inPart == sbPart_Down)
		{
			r.top = r.bottom - 16;
		}
		else if (r.GetHeight() > 55)
		{
			// calculate vert thumb rect
			thumb = r;
			thumb.top = r.top + 15;
			if (inInfo.max != 0)					// avert divide-by-zero processor exception
				thumb.top += ((r.bottom - r.top - 47) * inInfo.val) / inInfo.max;
			thumb.bottom = thumb.top+17;

			// bring thumb into range
			if (thumb.top < r.top+15)
			{
				thumb.top = r.top+15;
				thumb.bottom = thumb.top+17;
			}
			if (thumb.bottom > r.bottom-15)
			{
				thumb.bottom = r.bottom-15;
				thumb.top = thumb.bottom-17;
			}
			
			// now that we have the thumb, we can continue
			if (inPart == sbPart_PageUp)
			{
				r.top += 15;
				r.bottom = thumb.top + 1;
			}
			else if (inPart == sbPart_PageDown)
			{
				r.top = thumb.bottom - 1;
				r.bottom = r.bottom - 15;
			}
			else if (inPart == sbPart_Thumb)
			{
				r = thumb;
			}
			else
				r.SetEmpty();
		}
		else
			r.SetEmpty();
	}
	
	outRect = r;
}

Int32 UScrollBar::GetThumbDelta(const SRect& inBounds, const SScrollBarInfo& inInfo, const SPoint& inMouseLoc)
{
	SRect thumbRect;
	Int32 thumbDelta;
	
	GetPartRect(inBounds, inInfo, sbPart_Thumb, thumbRect);
	
	if (inBounds.GetWidth() > inBounds.GetHeight())
		thumbDelta = (inMouseLoc.h - thumbRect.left) - 1;
	else
		thumbDelta = (inMouseLoc.v - thumbRect.top) - 1;
	
	return thumbDelta;
}

Uint32 UScrollBar::GetThumbValue(const SRect& inBounds, const SScrollBarInfo& inInfo, const SPoint& inMouseLoc, Int32 inThumbDelta)
{
	Int32 baseMin, baseMax, mouseVal;

	if (inBounds.GetWidth() > inBounds.GetHeight())
	{
		baseMin = inBounds.left + 15;
		baseMax = inBounds.right - 32;
		mouseVal = inMouseLoc.h - inThumbDelta;
	}
	else
	{
		baseMin = inBounds.top + 15;
		baseMax = inBounds.bottom - 32;
		mouseVal = inMouseLoc.v - inThumbDelta;
	}
	
	if (mouseVal < baseMin) mouseVal = baseMin;
	if (mouseVal > baseMax) mouseVal = baseMax;
	
	return (inInfo.max * (mouseVal - baseMin)) / (baseMax - baseMin);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

static void _SBDrawBox(const SRect& inRect, const SColor& fillCol, const SColor& frameCol, const SColor& topCol, const SColor& botCol, const SColor& cornerCol)
{
	Rect r;
	Int16 top, left, bottom, right;
	
	top = inRect.top;
	left = inRect.left;
	bottom = inRect.bottom;
	right = inRect.right;
	
	// frame
	PenNormal();
	r.top = inRect.top;
	r.left = inRect.left;
	r.bottom = inRect.bottom;
	r.right = inRect.right;
	RGBForeColor((RGBColor *)&frameCol);
	FrameRect(&r);

	// body
	r.top += 2;
	r.left += 2;
	r.bottom -= 2;
	r.right -= 2;
	RGBForeColor((RGBColor *)&fillCol);
	PaintRect(&r);

	// top shading
	RGBForeColor((RGBColor *)&topCol);
	MoveTo(left+1, top+1);
	LineTo(right-3, top+1);
	MoveTo(left+1, top+1);
	LineTo(left+1, bottom-3);
	
	// bottom shading
	RGBForeColor((RGBColor *)&botCol);
	MoveTo(left+2, bottom-2);
	LineTo(right-2, bottom-2);
	MoveTo(right-2, bottom-2);
	LineTo(right-2, top+2);
	
	// corners
	SetCPixel(right-2, top+1, (RGBColor *)&fillCol);
	SetCPixel(left+1, bottom-2, (RGBColor *)&fillCol);
	SetCPixel(left+1, top+1, (RGBColor *)&cornerCol);
}

static void _SBDrawDownArrow(const SRect& inRect, const SColor& inColor)
{
	Int16 h = inRect.left + ((inRect.GetWidth() - 7) / 2);
	Int16 v = inRect.top + ((inRect.GetHeight() - 4) / 2);
	
	RGBForeColor((RGBColor *)&inColor);
	
	MoveTo(h, v);
	LineTo(h+7, v);
	MoveTo(h+1, v+1);
	LineTo(h+6, v+1);
	MoveTo(h+2, v+2);
	LineTo(h+5, v+2);
	MoveTo(h+3, v+3);
	LineTo(h+4, v+3);
}

static void _SBDrawUpArrow(const SRect& inRect, const SColor& inColor)
{
	Int16 h = inRect.left + ((inRect.GetWidth() - 7) / 2);
	Int16 v = inRect.top + ((inRect.GetHeight() - 4) / 2);

	RGBForeColor((RGBColor *)&inColor);

	MoveTo(h+3, v);
	LineTo(h+4, v);
	MoveTo(h+2, v+1);
	LineTo(h+5, v+1);
	MoveTo(h+1, v+2);
	LineTo(h+6, v+2);
	MoveTo(h, v+3);
	LineTo(h+7, v+3);
}

static void _SBDrawLeftArrow(const SRect& inRect, const SColor& inColor)
{
	Int16 h = inRect.left + ((inRect.GetWidth() - 4) / 2);
	Int16 v = inRect.top + ((inRect.GetHeight() - 7) / 2);

	RGBForeColor((RGBColor *)&inColor);
	
	MoveTo(h+3, v);
	LineTo(h+3, v+7);
	
	MoveTo(h+2, v+1);
	LineTo(h+2, v+6);
	
	MoveTo(h+1, v+2);
	LineTo(h+1, v+5);
	
	MoveTo(h, v+3);
	LineTo(h, v+4);
}

static void _SBDrawRightArrow(const SRect& inRect, const SColor& inColor)
{
	Int16 h = inRect.left + ((inRect.GetWidth() - 4) / 2);
	Int16 v = inRect.top + ((inRect.GetHeight() - 7) / 2);

	RGBForeColor((RGBColor *)&inColor);

	MoveTo(h, v);
	LineTo(h, v+7);
	
	MoveTo(h+1, v+1);
	LineTo(h+1, v+6);
	
	MoveTo(h+2, v+2);
	LineTo(h+2, v+5);
	
	MoveTo(h+3, v+3);
	LineTo(h+3, v+4);
}

static void _SBDrawVBack(const SRect& inRect, const SColor& fillCol, const SColor& frameCol, const SColor& ramp1Col, const SColor& ramp2Col, const SColor& ramp3Col, const SColor& ramp4Col)
{
	Rect r;
	Int16 top, left, bottom, right;
	
	top = inRect.top + 1;
	left = inRect.left + 1;
	bottom = inRect.bottom - 1;
	right = inRect.right - 1;
	
	r.top = inRect.top;
	r.left = inRect.left;
	r.bottom = inRect.bottom;
	r.right = inRect.right;

	PenSize(1, 1);
	
	if (bottom <= top) return;
	if (bottom - top == 1)
	{
		RGBForeColor((RGBColor *)&ramp1Col);
		MoveTo(left, top);
		LineTo(right-1, top);
		RGBForeColor((RGBColor *)&frameCol);
		FrameRect(&r);
		return;
	}

	RGBForeColor((RGBColor *)&ramp1Col);
	MoveTo(left, top);
	LineTo(right-2, top);
	MoveTo(left, top);
	LineTo(left, bottom-1);
	
	RGBForeColor((RGBColor *)&ramp2Col);
	MoveTo(left+1, top+1);
	LineTo(right-3, top+1);
	MoveTo(left+1, top+1);
	LineTo(left+1, bottom-1);
	
	RGBForeColor((RGBColor *)&ramp4Col);
	MoveTo(right-1, top);
	LineTo(right-1, bottom-1);
	
	RGBForeColor((RGBColor *)&ramp3Col);
	MoveTo(right-2, top+1);
	LineTo(right-2, bottom-1);
	
	RGBForeColor((RGBColor *)&frameCol);
	FrameRect(&r);
	
	r.top += 3;
	r.left += 3;
	r.right -= 3;
	r.bottom -= 1;
	RGBForeColor((RGBColor *)&fillCol);
	PaintRect(&r);
}

static void _SBDrawHBack(const SRect& inRect, const SColor& fillCol, const SColor& frameCol, const SColor& ramp1Col, const SColor& ramp2Col, const SColor& ramp3Col, const SColor& ramp4Col)
{
	Rect r;
	Int16 top, left, bottom, right;
	
	top = inRect.top + 1;
	left = inRect.left + 1;
	bottom = inRect.bottom - 1;
	right = inRect.right - 1;
	
	r.top = inRect.top;
	r.left = inRect.left;
	r.bottom = inRect.bottom;
	r.right = inRect.right;

	PenSize(1, 1);

	if (right <= left) return;
	if (right - left == 1)
	{
		RGBForeColor((RGBColor *)&ramp1Col);
		MoveTo(left, top);
		LineTo(left, bottom-1);
		RGBForeColor((RGBColor *)&frameCol);
		FrameRect(&r);
		return;
	}

	RGBForeColor((RGBColor *)&ramp1Col);
	MoveTo(left, top);
	LineTo(right-1, top);
	MoveTo(left, top);
	LineTo(left, bottom-1);
	
	RGBForeColor((RGBColor *)&ramp2Col);
	MoveTo(left+1, top+1);
	LineTo(right-1, top+1);
	MoveTo(left+1, top+1);
	LineTo(left+1, bottom-1);
	
	RGBForeColor((RGBColor *)&ramp4Col);
	MoveTo(left, bottom-1);
	LineTo(right-1, bottom-1);
	
	RGBForeColor((RGBColor *)&ramp3Col);
	MoveTo(left+1, bottom-2);
	LineTo(right-1, bottom-2);
	
	RGBForeColor((RGBColor *)&frameCol);
	FrameRect(&r);
	
	r.top += 3;
	r.left += 3;
	r.bottom -= 3;
	r.right -= 1;
	RGBForeColor((RGBColor *)&fillCol);
	PaintRect(&r);
}

static void _SBDrawVThumb(const SRect& inRect, const SColor& fillCol, const SColor& frameCol, const SColor& boxLight, const SColor& boxDark, const SColor& boxLighter, const SColor& gripLight, const SColor& gripDark, const SColor& gripLighter)
{
#pragma unused(gripLighter)

	Int16 top, left, bottom, right;
	
	top = inRect.top;
	left = inRect.left;
	bottom = inRect.bottom;
	right = inRect.right;
	
	// frame and body
	_SBDrawBox(inRect, fillCol, frameCol, boxLight, boxDark, boxLighter);
	
	// grips (light)
	RGBForeColor((RGBColor *)&gripLight);
	MoveTo(left+4, top+4);
	LineTo(right-6, top+4);
	MoveTo(left+4, top+6);
	LineTo(right-6, top+6);
	MoveTo(left+4, top+8);
	LineTo(right-6, top+8);
	MoveTo(left+4, top+10);
	LineTo(right-6, top+10);
	
	// grips (dark)
	RGBForeColor((RGBColor *)&gripDark);
	MoveTo(left+5, top+5);
	LineTo(right-5, top+5);
	MoveTo(left+5, top+7);
	LineTo(right-5, top+7);
	MoveTo(left+5, top+9);
	LineTo(right-5, top+9);
	MoveTo(left+5, top+11);
	LineTo(right-5, top+11);
}

static void _SBDrawHThumb(const SRect& inRect, const SColor& fillCol, const SColor& frameCol, const SColor& boxLight, const SColor& boxDark, const SColor& boxLighter, const SColor& gripLight, const SColor& gripDark, const SColor& gripLighter)
{
	#pragma unused(gripLighter)

	Int16 top, left, bottom, right;
	
	top = inRect.top;
	left = inRect.left;
	bottom = inRect.bottom;
	right = inRect.right;
	
	// frame and body
	_SBDrawBox(inRect, fillCol, frameCol, boxLight, boxDark, boxLighter);
	
	// grips (light)
	RGBForeColor((RGBColor *)&gripLight);
	MoveTo(left+4, top+4);
	LineTo(left+4, bottom-6);
	MoveTo(left+6, top+4);
	LineTo(left+6, bottom-6);
	MoveTo(left+8, top+4);
	LineTo(left+8, bottom-6);
	MoveTo(left+10, top+4);
	LineTo(left+10, bottom-6);
	
	// grips (dark)
	RGBForeColor((RGBColor *)&gripDark);
	MoveTo(left+5, top+5);
	LineTo(left+5, bottom-5);
	MoveTo(left+7, top+5);
	LineTo(left+7, bottom-5);
	MoveTo(left+9, top+5);
	LineTo(left+9, bottom-5);
	MoveTo(left+11, top+5);
	LineTo(left+11, bottom-5);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

//								fill			frame			top				bottom			corner
#define _SBNormalBoxColors		color_GrayD,	color_Gray0,	color_GrayF,	color_GrayA,	color_GrayF
#define _SBHilitedBoxColors		color_Gray7,	color_Gray0,	color_Gray5,	color_Gray9,	color_Gray5
#define _SBDisabledBoxColors	color_GrayE,	color_Gray0,	color_GrayF,	color_GrayD,	color_GrayF

#define _SBNormalArrowColor		color_Gray0
#define _SBHilitedArrowColor	color_Gray0
#define _SBDisabledArrowColor	color_Gray7

static void _SBDrawUp(const SRect& inRect)
{
	_SBDrawBox(inRect, _SBNormalBoxColors);
	_SBDrawUpArrow(inRect, _SBNormalArrowColor);
}

static void _SBDrawDown(const SRect& inRect)
{
	_SBDrawBox(inRect, _SBNormalBoxColors);
	_SBDrawDownArrow(inRect, _SBNormalArrowColor);
}

static void _SBDrawLeft(const SRect& inRect)
{
	_SBDrawBox(inRect, _SBNormalBoxColors);
	_SBDrawLeftArrow(inRect, _SBNormalArrowColor);
}

static void _SBDrawRight(const SRect& inRect)
{
	_SBDrawBox(inRect, _SBNormalBoxColors);
	_SBDrawRightArrow(inRect, _SBNormalArrowColor);
}

static void _SBDrawHilitedUp(const SRect& inRect)
{
	_SBDrawBox(inRect, _SBHilitedBoxColors);
	_SBDrawUpArrow(inRect, _SBHilitedArrowColor);
}

static void _SBDrawHilitedDown(const SRect& inRect)
{
	_SBDrawBox(inRect, _SBHilitedBoxColors);
	_SBDrawDownArrow(inRect, _SBHilitedArrowColor);
}

static void _SBDrawHilitedLeft(const SRect& inRect)
{
	_SBDrawBox(inRect, _SBHilitedBoxColors);
	_SBDrawLeftArrow(inRect, _SBHilitedArrowColor);
}

static void _SBDrawHilitedRight(const SRect& inRect)
{
	_SBDrawBox(inRect, _SBHilitedBoxColors);
	_SBDrawRightArrow(inRect, _SBHilitedArrowColor);
}

static void _SBDrawDisabledUp(const SRect& inRect)
{
	_SBDrawBox(inRect, _SBDisabledBoxColors);
	_SBDrawUpArrow(inRect, _SBDisabledArrowColor);

	RGBForeColor((RGBColor *)&_SBDisabledArrowColor);
	MoveTo(inRect.left+1, inRect.bottom-1);
	LineTo(inRect.right-2, inRect.bottom-1);
}

static void _SBDrawDisabledDown(const SRect& inRect)
{
	_SBDrawBox(inRect, _SBDisabledBoxColors);
	_SBDrawDownArrow(inRect, _SBDisabledArrowColor);

	RGBForeColor((RGBColor *)&_SBDisabledArrowColor);
	MoveTo(inRect.left+1, inRect.top);
	LineTo(inRect.right-2, inRect.top);
}

static void _SBDrawDisabledLeft(const SRect& inRect)
{
	_SBDrawBox(inRect, _SBDisabledBoxColors);
	_SBDrawLeftArrow(inRect, _SBDisabledArrowColor);

	RGBForeColor((RGBColor *)&_SBDisabledArrowColor);
	MoveTo(inRect.right-1, inRect.top+1);
	LineTo(inRect.right-1, inRect.bottom-2);
}

static void _SBDrawDisabledRight(const SRect& inRect)
{
	_SBDrawBox(inRect, _SBDisabledBoxColors);
	_SBDrawRightArrow(inRect, _SBDisabledArrowColor);

	RGBForeColor((RGBColor *)&_SBDisabledArrowColor);
	MoveTo(inRect.left, inRect.top+1);
	LineTo(inRect.left, inRect.bottom-2);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

//								fill			frame			ramp1			ramp2			ramp3			ramp4
#define _SBNormalPageColors		color_GrayA,	color_Gray0,	color_Gray7,	color_Gray8,	color_GrayB,	color_GrayC
#define _SBHilitedPageColors	SColor(0x5556),	color_Gray0,	color_Gray8,	color_Gray7,	color_Gray4,	color_Gray3
// slightly changed hilited fill color to avoid conflict with Kaleidoscope (it changed into a progress bar)

static void _SBDrawVPage(const SRect& inRect)
{
	_SBDrawVBack(inRect, _SBNormalPageColors);
}

static void _SBDrawHPage(const SRect& inRect)
{
	_SBDrawHBack(inRect, _SBNormalPageColors);
}

static void _SBDrawHilitedVPage(const SRect& inRect)
{
	_SBDrawVBack(inRect, _SBHilitedPageColors);
}

static void _SBDrawHilitedHPage(const SRect& inRect)
{
	_SBDrawHBack(inRect, _SBHilitedPageColors);
}

static void _SBDrawDisabledVPage(const SRect& inRect)
{
	Rect r;
	
	PenNormal();
	
	r.top = inRect.top;
	r.left = inRect.left;
	r.bottom = inRect.bottom;
	r.right = inRect.right;
	
	RGBForeColor((RGBColor *)&color_Gray0);
	FrameRect(&r);

	InsetRect(&r, 1, 1);
	RGBForeColor((RGBColor *)&color_GrayE);
	PaintRect(&r);
}

static void _SBDrawDisabledHPage(const SRect& inRect)
{
	_SBDrawDisabledVPage(inRect);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

static const SColor _SBColor99F(0x9999,0x9999,0xFFFF);
static const SColor _SBColorCCF(0xCCCC,0xCCCC,0xFFFF);
static const SColor _SBColor66C(0x6666,0x6666,0xCCCC);
static const SColor _SBColor339(0x3333,0x3333,0x9999);
static const SColor _SBColor005(0x0000,0x0000,0x5555);

//								fill			frame			box light		box dark		box lighter		grip light		grip dark		grip lighter
#define _SBNormalThumbColors	_SBColor99F,	color_Gray0,		_SBColorCCF,	_SBColor66C,	color_GrayF,		_SBColorCCF,	_SBColor339,	color_GrayE
#define _SBHilitedThumbColors	_SBColor66C,	color_Gray0,		_SBColor99F,	_SBColor339,	_SBColorCCF,	_SBColor99F,	_SBColor005,	_SBColorCCF

static void _SBDrawVThumb(const SRect& inRect)
{
	_SBDrawVThumb(inRect, _SBNormalThumbColors);
}

static void _SBDrawHThumb(const SRect& inRect)
{
	_SBDrawHThumb(inRect, _SBNormalThumbColors);
}

static void _SBDrawHilitedVThumb(const SRect& inRect)
{
	_SBDrawVThumb(inRect, _SBHilitedThumbColors);
}

static void _SBDrawHilitedHThumb(const SRect& inRect)
{
	_SBDrawHThumb(inRect, _SBHilitedThumbColors);
}

#endif /* MACINTOSH */
