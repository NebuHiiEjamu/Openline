#if WIN32

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include <commctrl.h>

#include "UGraphics.h"

struct SImage {
	HDC dc;
	COLORREF penColor;
	Uint32 penWidth, fontSize, fontEffect;
	HBRUSH brush;
	HBITMAP bitmap;
	Uint32 width, height;
	SPoint virtualOrigin;
};

//#define REF		((SImage *)inImage)
//#define REFDC	(((SImage *)inImage)->dc)

#define RGBColor(c)		PALETTERGB((c).red >> 8, (c).green >> 8, (c).blue >> 8)

HBRUSH _NULL_BRUSH = 0;
HPEN _NULL_PEN = 0;
HBRUSH _3DFACE_BRUSH = 0;
HBRUSH _3DDKSHADOW_BRUSH = 0;
HPEN _3DHILIGHT_PEN = 0;
HPEN _3DSHADOW_PEN = 0;
HPEN _3DDKSHADOW_PEN = 0;
HBRUSH _ACTIVECAPTION_BRUSH = 0;
HPEN _ACTIVECAPTION_PEN = 0;
HBRUSH _3DSHADOW_BRUSH = 0;
HBRUSH _HALFTONE_BRUSH = 0;
HPALETTE _MAC_PALETTE = 0;

static SImage _gDummyImage;

struct SFontDescObj {
	LOGFONT lf;
	Uint32 size, effect, align, customVal;
	SColor color, customColor;
	Uint8 locked;
};

static SFontDescObj _gDefaultFontDesc;

#define FD		((SFontDescObj *)inFD)

static Uint32 _FontNameToWinName(const Uint8 *inName, Int8 *outName, Uint32 inBufferSize);

void _FailLastWinError(const Int8 *inFile, Uint32 inLine);
void _FailWinError(Int32 inWinError, const Int8 *inFile, Uint32 inLine);
#if DEBUG
	#define FailLastWinError()		_FailLastWinError(__FILE__, __LINE__)
	#define FailWinError(id)		_FailWinError(id, __FILE__, __LINE__)
#else
	#define FailLastWinError()		_FailLastWinError(nil, 0)
	#define FailWinError(id)		_FailWinError(id, nil, 0)
#endif

static const char *_MAC_LOGPALETTE_DATA = "\x00\x03\x00\x01"     /* "\x00\x03\xEC\x00"*/
"\xFF\xFF\xFF\x00\xFF\xFF\xCC\x00\xFF\xFF\x99\x00\xFF\xFF\x66\x00\xFF\xFF\x33\x00\xFF\xFF\x00\x00\xFF\xCC\xFF\x00\xFF\xCC\xCC\x00\xFF\xCC\x99\x00\xFF\xCC"
"\x66\x00\xFF\xCC\x33\x00\xFF\xCC\x00\x00\xFF\x99\xFF\x00\xFF\x99\xCC\x00\xFF\x99\x99\x00\xFF\x99\x66\x00\xFF\x99\x33\x00\xFF\x99\x00\x00\xFF\x66\xFF\x00"
"\xFF\x66\xCC\x00\xFF\x66\x99\x00\xFF\x66\x66\x00\xFF\x66\x33\x00\xFF\x66\x00\x00\xFF\x33\xFF\x00\xFF\x33\xCC\x00\xFF\x33\x99\x00\xFF\x33\x66\x00\xFF\x33"
"\x33\x00\xFF\x33\x00\x00\xFF\x00\xFF\x00\xFF\x00\xCC\x00\xFF\x00\x99\x00\xFF\x00\x66\x00\xFF\x00\x33\x00\xFF\x00\x00\x00\xCC\xFF\xFF\x00\xCC\xFF\xCC\x00"
"\xCC\xFF\x99\x00\xCC\xFF\x66\x00\xCC\xFF\x33\x00\xCC\xFF\x00\x00\xCC\xCC\xFF\x00\xCC\xCC\xCC\x00\xCC\xCC\x99\x00\xCC\xCC\x66\x00\xCC\xCC\x33\x00\xCC\xCC"
"\x00\x00\xCC\x99\xFF\x00\xCC\x99\xCC\x00\xCC\x99\x99\x00\xCC\x99\x66\x00\xCC\x99\x33\x00\xCC\x99\x00\x00\xCC\x66\xFF\x00\xCC\x66\xCC\x00\xCC\x66\x99\x00"
"\xCC\x66\x66\x00\xCC\x66\x33\x00\xCC\x66\x00\x00\xCC\x33\xFF\x00\xCC\x33\xCC\x00\xCC\x33\x99\x00\xCC\x33\x66\x00\xCC\x33\x33\x00\xCC\x33\x00\x00\xCC\x00"
"\xFF\x00\xCC\x00\xCC\x00\xCC\x00\x99\x00\xCC\x00\x66\x00\xCC\x00\x33\x00\xCC\x00\x00\x00\x99\xFF\xFF\x00\x99\xFF\xCC\x00\x99\xFF\x99\x00\x99\xFF\x66\x00"
"\x99\xFF\x33\x00\x99\xFF\x00\x00\x99\xCC\xFF\x00\x99\xCC\xCC\x00\x99\xCC\x99\x00\x99\xCC\x66\x00\x99\xCC\x33\x00\x99\xCC\x00\x00\x99\x99\xFF\x00\x99\x99"
"\xCC\x00\x99\x99\x99\x00\x99\x99\x66\x00\x99\x99\x33\x00\x99\x99\x00\x00\x99\x66\xFF\x00\x99\x66\xCC\x00\x99\x66\x99\x00\x99\x66\x66\x00\x99\x66\x33\x00"
"\x99\x66\x00\x00\x99\x33\xFF\x00\x99\x33\xCC\x00\x99\x33\x99\x00\x99\x33\x66\x00\x99\x33\x33\x00\x99\x33\x00\x00\x99\x00\xFF\x00\x99\x00\xCC\x00\x99\x00"
"\x99\x00\x99\x00\x66\x00\x99\x00\x33\x00\x99\x00\x00\x00\x66\xFF\xFF\x00\x66\xFF\xCC\x00\x66\xFF\x99\x00\x66\xFF\x66\x00\x66\xFF\x33\x00\x66\xFF\x00\x00"
"\x66\xCC\xFF\x00\x66\xCC\xCC\x00\x66\xCC\x99\x00\x66\xCC\x66\x00\x66\xCC\x33\x00\x66\xCC\x00\x00\x66\x99\xFF\x00\x66\x99\xCC\x00\x66\x99\x99\x00\x66\x99"
"\x66\x00\x66\x99\x33\x00\x66\x99\x00\x00\x66\x66\xFF\x00\x66\x66\xCC\x00\x66\x66\x99\x00\x66\x66\x66\x00\x66\x66\x33\x00\x66\x66\x00\x00\x66\x33\xFF\x00"
"\x66\x33\xCC\x00\x66\x33\x99\x00\x66\x33\x66\x00\x66\x33\x33\x00\x66\x33\x00\x00\x66\x00\xFF\x00\x66\x00\xCC\x00\x66\x00\x99\x00\x66\x00\x66\x00\x66\x00"
"\x33\x00\x66\x00\x00\x00\x33\xFF\xFF\x00\x33\xFF\xCC\x00\x33\xFF\x99\x00\x33\xFF\x66\x00\x33\xFF\x33\x00\x33\xFF\x00\x00\x33\xCC\xFF\x00\x33\xCC\xCC\x00"
"\x33\xCC\x99\x00\x33\xCC\x66\x00\x33\xCC\x33\x00\x33\xCC\x00\x00\x33\x99\xFF\x00\x33\x99\xCC\x00\x33\x99\x99\x00\x33\x99\x66\x00\x33\x99\x33\x00\x33\x99"
"\x00\x00\x33\x66\xFF\x00\x33\x66\xCC\x00\x33\x66\x99\x00\x33\x66\x66\x00\x33\x66\x33\x00\x33\x66\x00\x00\x33\x33\xFF\x00\x33\x33\xCC\x00\x33\x33\x99\x00"
"\x33\x33\x66\x00\x33\x33\x33\x00\x33\x33\x00\x00\x33\x00\xFF\x00\x33\x00\xCC\x00\x33\x00\x99\x00\x33\x00\x66\x00\x33\x00\x33\x00\x33\x00\x00\x00\x00\xFF"
"\xFF\x00\x00\xFF\xCC\x00\x00\xFF\x99\x00\x00\xFF\x66\x00\x00\xFF\x33\x00\x00\xFF\x00\x00\x00\xCC\xFF\x00\x00\xCC\xCC\x00\x00\xCC\x99\x00\x00\xCC\x66\x00"
"\x00\xCC\x33\x00\x00\xCC\x00\x00\x00\x99\xFF\x00\x00\x99\xCC\x00\x00\x99\x99\x00\x00\x99\x66\x00\x00\x99\x33\x00\x00\x99\x00\x00\x00\x66\xFF\x00\x00\x66"
"\xCC\x00\x00\x66\x99\x00\x00\x66\x66\x00\x00\x66\x33\x00\x00\x66\x00\x00\x00\x33\xFF\x00\x00\x33\xCC\x00\x00\x33\x99\x00\x00\x33\x66\x00\x00\x33\x33\x00"
"\x00\x33\x00\x00\x00\x00\xFF\x00\x00\x00\xCC\x00\x00\x00\x99\x00\x00\x00\x66\x00\x00\x00\x33\x00\xEE\x00\x00\x00\xDD\x00\x00\x00\xBB\x00\x00\x00\xAA\x00"
"\x00\x00\x88\x00\x00\x00\x77\x00\x00\x00\x55\x00\x00\x00\x44\x00\x00\x00\x22\x00\x00\x00\x11\x00\x00\x00\x00\xEE\x00\x00\x00\xDD\x00\x00\x00\xBB\x00\x00"
"\x00\xAA\x00\x00\x00\x88\x00\x00\x00\x77\x00\x00\x00\x55\x00\x00\x00\x44\x00\x00\x00\x22\x00\x00\x00\x11\x00\x00\x00\x00\xEE\x00\x00\x00\xDD\x00\x00\x00"
"\xBB\x00\x00\x00\xAA\x00\x00\x00\x88\x00\x00\x00\x77\x00\x00\x00\x55\x00\x00\x00\x44\x00\x00\x00\x22\x00\x00\x00\x11\x00\xEE\xEE\xEE\x00\xDD\xDD\xDD\x00"
"\xBB\xBB\xBB\x00\xAA\xAA\xAA\x00\x88\x88\x88\x00\x77\x77\x77\x00\x55\x55\x55\x00\x44\x44\x44\x00\x22\x22\x22\x00\x11\x11\x11\x00\x00\x00\x00\x00";


// character mapping table

extern const char _UTCharMap_AWToPC[];

void _GRTrans32ToMask(Uint32 inTransCol, void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount);
void _GRTrans8ToMask(Uint32 inTransCol, void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount, const Uint32 *inSrcColors, Uint32 inSrcColorCount);

void _GRTransCopy32To32(Uint32 inTransCol, void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount);
void _GRTransCopy32To8(Uint32 inTransCol, void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount, const Uint32 *inDstColors, Uint32 inDstColCount);
void _GRTransCopy8To32(Uint32 inTransCol, void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount, const Uint32 *inSrcColors, Uint32 inSrcColorCount);
void _GRTransCopy8To8(Uint32 inTransCol, void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount, const Uint32 *inDstColors, Uint32 inDstColCount, const Uint32 *inSrcColors, Uint32 inSrcColCount);

/* -------------------------------------------------------------------------- */

void UGraphics::Init()
{
	if (_NULL_BRUSH == 0)
	{
		// **** need to trap WM_SYSCOLORCHANGE and recreate brushes (?) and pens
		_NULL_BRUSH = (HBRUSH)::GetStockObject(NULL_BRUSH);
		_NULL_PEN = (HPEN)::GetStockObject(NULL_PEN);
		_3DFACE_BRUSH = ::GetSysColorBrush(COLOR_3DFACE);
		_3DDKSHADOW_BRUSH = ::GetSysColorBrush(COLOR_3DDKSHADOW);
		_ACTIVECAPTION_BRUSH = ::GetSysColorBrush(COLOR_ACTIVECAPTION);
		_3DHILIGHT_PEN = ::CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DHILIGHT));
		_3DSHADOW_PEN = ::CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DSHADOW));
		_3DDKSHADOW_PEN = ::CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DDKSHADOW));
		_ACTIVECAPTION_PEN = ::CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_ACTIVECAPTION));
		_3DSHADOW_BRUSH = ::GetSysColorBrush(COLOR_3DSHADOW);

		// create halftone brush
		WORD grayPattern[8];
		for (int i = 0; i < 8; i++)
			grayPattern[i] = (WORD)(0x5555 << (i & 1));
		HBITMAP grayBitmap = CreateBitmap(8, 8, 1, 1, &grayPattern);
		if (grayBitmap != NULL)
		{
			_HALFTONE_BRUSH = ::CreatePatternBrush(grayBitmap);
			DeleteObject(grayBitmap);
		}
				
		// create dummy image
		ClearStruct(_gDummyImage);
		_gDummyImage.dc = ::CreateIC("DISPLAY", NULL, NULL, NULL);
		if (_gDummyImage.dc == 0) Fail(errorType_Memory, memError_NotEnough);

		// all red palette for debugging
		/*
		LOGPALETTE *lgp = (LOGPALETTE *)_MAC_LOGPALETTE_DATA;
		for (Uint32 i=0; i!=256; i++)
		{
			lgp->palPalEntry[i].peRed = i;
			lgp->palPalEntry[i].peGreen = 0;
			lgp->palPalEntry[i].peBlue = 0;
		}
		*/

		// create mac palette - much nicer than the crappy windoze default
		//_MAC_PALETTE = ::CreatePalette((LOGPALETTE *)_MAC_LOGPALETTE_DATA);
		//if (_MAC_PALETTE == NULL) Fail(errorType_Memory, memError_NotEnough);
		
		// build a color table consisting of the windoze system colors and as many mac colors as will fit
		Uint8 palBuf[2048];
		LOGPALETTE *logPal = (LOGPALETTE *)palBuf;
		logPal->palVersion = 0x300;
		logPal->palNumEntries = ::GetSystemPaletteEntries(_gDummyImage.dc, 0, 20, logPal->palPalEntry);
		logPal->palNumEntries = UGraphics::MergeColorTable((Uint32 *)logPal->palPalEntry, logPal->palNumEntries, 256, ((Uint32 *)_MAC_LOGPALETTE_DATA)+1, 256);

		// create the mac palette
		_MAC_PALETTE = ::CreatePalette(logPal);
		if (_MAC_PALETTE == NULL) Fail(errorType_Memory, memError_NotEnough);		
		
		// default font is 'MS Sans Serif' Bold 8
		ClearStruct(_gDefaultFontDesc);
		_gDefaultFontDesc.lf.lfHeight = -8;
		_gDefaultFontDesc.lf.lfWeight = FW_NORMAL;
		_gDefaultFontDesc.size = 8;
		_gDefaultFontDesc.effect = fontEffect_Bold;
		_gDefaultFontDesc.align = textAlign_Left;
		_gDefaultFontDesc.locked = true;
		::CopyMemory(_gDefaultFontDesc.lf.lfFaceName, "MS Sans Serif", 13);
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

/*
 * SetOrigin() makes the specified point the origin (0,0).  You can
 * think of it as adding the point to the coordinates of all
 * subsequent drawing commands.
 */
void UGraphics::SetOrigin(TImage inImage, const SPoint& inOrigin)
{
	ASSERT(inImage);
	HDC dc = (((SImage *)inImage)->dc);
	POINT pt;
	
	::GetViewportOrgEx(dc, &pt);

	::SetViewportOrgEx(dc, inOrigin.x, inOrigin.y, NULL);
	
	// make clip rgn stick to the coordinate system
	::OffsetClipRgn(dc, inOrigin.x - pt.x, inOrigin.y - pt.y);
}

void UGraphics::GetOrigin(TImage inImage, SPoint& outOrigin)
{
	ASSERT(inImage);
	::GetViewportOrgEx((((SImage *)inImage)->dc), (LPPOINT)&outOrigin);
}

void UGraphics::AddOrigin(TImage inImage, Int32 inHorizDelta, Int32 inVertDelta)
{
	ASSERT(inImage);
	HDC dc = (((SImage *)inImage)->dc);

	::OffsetViewportOrgEx(dc, inHorizDelta, inVertDelta, NULL);
	
	// make clip rgn stick to the coordinate system
	::OffsetClipRgn(dc, inHorizDelta, inVertDelta);
}

void UGraphics::ResetOrigin(TImage inImage)
{
	ASSERT(inImage);
	HDC dc = (((SImage *)inImage)->dc);
	POINT pt;
	
	::GetViewportOrgEx(dc, &pt);
	::OffsetClipRgn(dc, -pt.x, -pt.y);

	::SetViewportOrgEx(dc, 0, 0, NULL);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void UGraphics::SetClip(TImage inImage, TRegion inClip)
{
	Require(inImage && inClip);
	
	HDC dc = (((SImage *)inImage)->dc);
	RECT r;
	
	::GetRgnBox((HRGN)inClip, &r);
	
	if (r.left == min_Int16 && r.top == min_Int16 && r.right == max_Int16 && r.bottom == max_Int16)
	{
		// remove clipping
		::SelectClipRgn(dc, NULL);
	}
	else
	{
		::SelectClipRgn(dc, (HRGN)inClip);

		// windoze clip rgn ignores the origin, so fix it here
		POINT pt;
		::GetViewportOrgEx(dc, &pt);
		if (pt.x || pt.y) ::OffsetClipRgn(dc, pt.x, pt.y);
	}
}

void UGraphics::SetClip(TImage inImage, const SRect& inClip)
{
	SRect r;
	POINT pt;
	HRGN rgn;
	HDC dc;
	
	ASSERT(inImage);
	dc = (((SImage *)inImage)->dc);
	::GetViewportOrgEx(dc, &pt);

	// windoze clip rgn ignores the origin, so fix it here
	r = inClip;
	r.Move(pt.x, pt.y);
	
	rgn = ::CreateRectRgnIndirect((RECT *)&r);
	if (rgn)
	{
		::SelectClipRgn(dc, rgn);
		::DeleteObject(rgn);
	}
}

void UGraphics::GetClip(TImage inImage, TRegion outClip)
{
	Require(inImage && outClip);
	
	HDC dc = (((SImage *)inImage)->dc);
	
	if (::GetClipRgn(dc, (HRGN)outClip) == 1)	// if succeeded and there is a clipping region
	{
		// undo the offseting that was necessary because windoze clip rgn ignores the origin
		POINT pt;
		::GetViewportOrgEx(dc, &pt);
		::OffsetRgn((HRGN)outClip, -pt.x, -pt.y);
	}
	else
	{
		// no clipping region or failed
		URegion::SetRect(outClip, SRect(min_Int16, min_Int16, max_Int16, max_Int16));
	}
}

void UGraphics::GetClip(TImage inImage, SRect& outClip)
{
	ASSERT(inImage);
	HDC dc = (((SImage *)inImage)->dc);
	POINT pt;

	if (::GetClipBox(dc, (LPRECT)&outClip) == ERROR)	// if error
		outClip.SetEmpty();
	else
	{
		// undo the offseting that was necessary because windoze clip rgn ignores the origin
		::GetViewportOrgEx(dc, &pt);
		outClip.Move(-pt.x, -pt.y);
	}
}

void UGraphics::SetNoClip(TImage inImage)
{
	ASSERT(inImage);
	::SelectClipRgn((((SImage *)inImage)->dc), NULL);
}

void UGraphics::IntersectClip(TImage inImage, const SRect& inClip)
{
	ASSERT(inImage);
	::IntersectClipRect((((SImage *)inImage)->dc), inClip.left, inClip.top, inClip.right, inClip.bottom);
	
	// apparently it is not necessary to adjust the rect for IntersectClipRect()
	/*
	SRect r;
	POINT pt;
	HDC dc;
	
	dc = (((SImage *)inImage)->dc);
	::GetViewportOrgEx(dc, &pt);

	// windoze clip rgn ignores the origin, so fix it here
	r = inClip;
	r.Move(pt.x, pt.y);

	::IntersectClipRect(dc, r.left, r.top, r.right, r.bottom);*/
}

void UGraphics::IntersectClip(TImage inImage, TRegion inClip)
{
	Require(inImage && inClip);
	HDC dc = (((SImage *)inImage)->dc);
	POINT pt;
	HRGN rgn;
	
	::GetViewportOrgEx(dc, &pt);
	
	if (pt.x || pt.y)	// if need to take into account origin
	{
		rgn = ::CreateRectRgn(0,0,1,1);
		if (rgn == nil) return;
		
		if (::CombineRgn(rgn, (HRGN)inClip, NULL, RGN_COPY) == ERROR)
		{
			::DeleteObject(rgn);
			return;
		}
		
		::OffsetRgn(rgn, pt.x, pt.y);
		
		::ExtSelectClipRgn(dc, rgn, RGN_AND);
		
		::DeleteObject(rgn);
	}
	else
	{
		::ExtSelectClipRgn(dc, (HRGN)inClip, RGN_AND);
	}
}

// set the clip to the intersection of inRgn and inRect
void UGraphics::IntersectClip(TImage inImage, TRegion inRgn, const SRect& inRect)
{
	Require(inImage && inRgn);
	HDC dc = (((SImage *)inImage)->dc);
	POINT pt;
	
	HRGN tempRgn = nil;
	HRGN rectRgn = nil;
	
	tempRgn = ::CreateRectRgn(0,0,1,1);
	if (tempRgn == nil) goto abort;

	rectRgn = ::CreateRectRgnIndirect((RECT *)&inRect);
	if (rectRgn == nil) goto abort;
	
	if (::CombineRgn(tempRgn, (HRGN)inRgn, rectRgn, RGN_AND) == ERROR) goto abort;

	::GetViewportOrgEx(dc, &pt);
	if (pt.x || pt.y) ::OffsetRgn(tempRgn, pt.x, pt.y);

	::SelectClipRgn(dc, tempRgn);
	
abort:
	if (tempRgn) ::DeleteObject(tempRgn);
	if (rectRgn) ::DeleteObject(rectRgn);
}

void UGraphics::MoveClip(TImage inImage, Int32 inHorizDelta, Int32 inVertDelta)
{
	ASSERT(inImage);
	if (inHorizDelta || inVertDelta)
		::OffsetClipRgn((((SImage *)inImage)->dc), inHorizDelta, inVertDelta);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void UGraphics::Reset(TImage inImage)
{
	ASSERT(inImage);
	HDC dc = (((SImage *)inImage)->dc);
	
	DeleteObject(SelectObject(dc, CreatePen(PS_SOLID, 1, 0)));
	((SImage *)inImage)->penColor = 0;
	((SImage *)inImage)->penWidth = 1;

	HBRUSH brush = CreateSolidBrush(0);
	DeleteObject(SelectObject(dc, brush));
	((SImage *)inImage)->brush = brush;
	
	SetROP2(dc, R2_COPYPEN);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void UGraphics::ResetPen(TImage inImage)
{
	ASSERT(inImage);
	DeleteObject(SelectObject((((SImage *)inImage)->dc), CreatePen(PS_SOLID, 1, ((SImage *)inImage)->penColor)));
	((SImage *)inImage)->penWidth = 1;
}

void UGraphics::SetPenSize(TImage inImage, Uint32 inSize)
{
	ASSERT(inImage);
	DeleteObject(SelectObject((((SImage *)inImage)->dc), CreatePen(PS_SOLID, inSize, ((SImage *)inImage)->penColor)));
	((SImage *)inImage)->penWidth = inSize;
}

Uint32 UGraphics::GetPenSize(TImage inImage)
{
	ASSERT(inImage);
	return ((SImage *)inImage)->penWidth;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void UGraphics::ResetInk(TImage inImage)
{
	ASSERT(inImage);
	HDC dc = (((SImage *)inImage)->dc);
	
	DeleteObject(SelectObject(dc, CreatePen(PS_SOLID, ((SImage *)inImage)->penWidth, 0)));
	((SImage *)inImage)->penColor = 0;
	
	SetROP2(dc, R2_COPYPEN);
}

void UGraphics::SetInkAttributes(TImage /* inImage */, Uint32 /* inAttrib */)
{
	// don't support ink attributes
}

void UGraphics::SetInkColor(TImage inImage, const SColor& inColor)
{
	ASSERT(inImage);
	COLORREF color = RGBColor(inColor);
	HDC dc = (((SImage *)inImage)->dc);
	
	// disable dithering for 16-bit color (24-bit won't dither, and we're using PALETTERGB for the other depths)
	if (::GetDeviceCaps(dc, BITSPIXEL) == 16)
		color = ::GetNearestColor(dc, color);
	
	// pen is for lines
	DeleteObject(SelectObject(dc, CreatePen(PS_SOLID, ((SImage *)inImage)->penWidth, color)));
	((SImage *)inImage)->penColor = color;

	// brush is for fill
	HBRUSH brush = CreateSolidBrush(color);
	DeleteObject(SelectObject(dc, brush));
	((SImage *)inImage)->brush = brush;
	
	// text color
	::SetTextColor(dc, color);
}

void UGraphics::SetInkMode(TImage inImage, Uint32 inTransferMode, const SColor */* inOperand */)
{
	Int32 mode;
	
	ASSERT(inImage);

	switch (inTransferMode)
	{
		case mode_Copy:
			mode = R2_COPYPEN;
			break;
		case mode_Or:
		case mode_RampOr:
			mode = R2_MERGEPEN;
			break;
		case mode_Xor:
		case mode_RampXor:
			mode = SRCINVERT;	// R2_XORPEN doesn't seem to work
			break;
		case mode_And:
		case mode_RampAnd:
			mode = R2_MASKPEN;
			break;
		default:
			mode = R2_COPYPEN;
			break;
	}
	
	SetROP2((((SImage *)inImage)->dc), mode);
}

Uint32 UGraphics::GetInkAttributes(TImage /* inImage */)
{
	// don't support ink attributes
	return 0;
}

void UGraphics::GetInkColor(TImage inImage, SColor& outColor)
{
	COLORREF c = ((SImage *)inImage)->penColor;

	outColor.Set(GetRValue(c) * 257, GetGValue(c) * 257, GetBValue(c) * 257);
}

void UGraphics::GetInkMode(TImage inImage, Uint32& outTransferMode, SColor& outOperand)
{
	outOperand.Set(0);
	outTransferMode = GetInkMode(inImage);
}

Uint32 UGraphics::GetInkMode(TImage inImage)
{
	Uint32 mode;
	
	ASSERT(inImage);

	switch (GetROP2((((SImage *)inImage)->dc)))
	{
		case R2_COPYPEN:
			mode = mode_Copy;
			break;
		case R2_MERGEPEN:
			mode = mode_Or;
			break;
		case SRCINVERT:
			mode = mode_Xor;
			break;
		case R2_MASKPEN:
			mode = mode_And;
			break;
		default:
			mode = mode_Copy;
			break;
	}
	
	return mode;
}

void UGraphics::GetNearestColor(TImage inImage, SColor& ioColor)
{
	ASSERT(inImage);

	COLORREF c = ::GetNearestColor((((SImage *)inImage)->dc), RGBColor(ioColor));
	ioColor.Set(GetRValue(c) * 257, GetGValue(c) * 257, GetBValue(c) * 257);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void UGraphics_DrawPixel(TImage inImage, const SPoint& inPixel)
{
	ASSERT(inImage);
	
	::SetPixelV((((SImage *)inImage)->dc), inPixel.x, inPixel.y, ((SImage *)inImage)->penColor);
}

// see notes in UGraphics.cp regarding how lines are drawn
void UGraphics_DrawLine(TImage inImage, const SLine& inLine, Uint32 inOptions)
{
	ASSERT(inImage);
	
	HDC dc = (((SImage *)inImage)->dc);
	Int32 pn = ((SImage *)inImage)->penWidth;
	
	if (inOptions & 1)	// if draw rounded overlapping corners
	{
		// in addition to this, 1pt lines (only) need to be extended 1 pixel but dunno how to do that
		if ((pn > 5) && (pn & 1))		// if need to fix up stupid windoze line centering
		{
			::MoveToEx(dc, inLine.start.x+1, inLine.start.y+1, NULL);
			::LineTo(dc, inLine.end.x+1, inLine.end.y+1);
		}
		else
		{
			::MoveToEx(dc, inLine.start.x, inLine.start.y, NULL);
			::LineTo(dc, inLine.end.x, inLine.end.y);
		}
	}
	else if (pn == 1)
	{
		::MoveToEx(dc, inLine.start.x, inLine.start.y, NULL);
		::LineTo(dc, inLine.end.x, inLine.end.y);
	}
	else if (pn != 0)
	{
		SPoint pts[4];
		UGraphics::GetLinePoly(inLine.start, inLine.end, pn, pts);

		if (::BeginPath(dc))
		{
			::Polyline(dc, (POINT *)pts, 4);
			::EndPath(dc);
			::FillPath(dc);
			::AbortPath(dc);
		}
	}
}

// see notes in UGraphics.cp regarding how lines are drawn
void UGraphics_DrawLines(TImage inImage, const SLine *inLines, Uint32 inCount, Uint32 inOptions)
{
	if (!inLines)
		return;
	
	ASSERT(inImage);
	
	HDC dc = (((SImage *)inImage)->dc);
	Int32 pn = ((SImage *)inImage)->penWidth;

	if (inOptions & 1)	// if draw rounded overlapping corners
	{
		// in addition to this, 1pt lines (only) need to be extended 1 pixel but dunno how to do that
		if ((pn > 5) && (pn & 1))		// if need to fix up stupid windoze line centering
		{
			while (inCount--)
			{
				::MoveToEx(dc, inLines->start.x+1, inLines->start.y+1, NULL);
				::LineTo(dc, inLines->end.x+1, inLines->end.y+1);
				inLines++;
			}
		}
		else
		{
			while (inCount--)
			{
				::MoveToEx(dc, inLines->start.x, inLines->start.y, NULL);
				::LineTo(dc, inLines->end.x, inLines->end.y);
				inLines++;
			}
		}
	}
	else if (pn == 1)
	{
		while (inCount--)
		{
			::MoveToEx(dc, inLines->start.x, inLines->start.y, NULL);
			::LineTo(dc, inLines->end.x, inLines->end.y);
			inLines++;
		}
	}
	else if (pn != 0)
	{
		SPoint pts[4];
		
		while (inCount--)
		{
			UGraphics::GetLinePoly(inLines->start, inLines->end, pn, pts);

			if (::BeginPath(dc))
			{
				::Polyline(dc, (POINT *)pts, 4);
				::EndPath(dc);
				::FillPath(dc);
				::AbortPath(dc);
			}
			
			inLines++;
		}
	}
}

void UGraphics_DrawRect(TImage inImage, const SRect& inRect, Uint32 inFill)
{
	ASSERT(inImage && inRect.IsValid());

	HDC dc = (((SImage *)inImage)->dc);
	Uint32 pn = ((SImage *)inImage)->penWidth;

	switch (inFill)
	{
		case fill_None:
			break;
		
		case fill_OpenFrame:
		case fill_ClosedFrame:
			if (pn == 1)
				::FrameRect(dc, (RECT *)&inRect, ((SImage *)inImage)->brush);	// this can only draw 1pt lines
			else
			{
				if (inRect.GetWidth() <= pn || inRect.GetHeight() <= pn)
					::FillRect(dc, (RECT *)&inRect, ((SImage *)inImage)->brush);
				else
				{
					// calculate the coords necessary to trick shitty windoze into drawing the rectangle entirely inside the rect
					Uint32 pnh = pn >> 1;
					Int32 left = inRect.left + pnh;
					Int32 top = inRect.top + pnh;
					Int32 right = inRect.right - pnh;
					Int32 bottom = inRect.bottom - pnh;
					if ((pn & 1) == 0)
					{
						right++;
						bottom++;
					}
					
					// frame the rect with thick line
					HGDIOBJ savedObj = ::SelectObject(dc, _NULL_BRUSH);
					::Rectangle(dc, left, top, right, bottom);
					::SelectObject(dc, savedObj);
				}
			}
			break;
		
		default:
			// we purposely use this func which ignores the line size because we want the rect just simply filled
			::FillRect(dc, (RECT *)&inRect, ((SImage *)inImage)->brush);
			break;
	}
}

void UGraphics_DrawOval(TImage inImage, const SRect& inRect, Uint32 inFill)
{
	ASSERT(inImage && inRect.IsValid());
		
	HGDIOBJ savedObj;
	HDC dc = (((SImage *)inImage)->dc);
	
	if (inFill == fill_OpenFrame || inFill == fill_ClosedFrame)
	{
		Uint32 pn = ((SImage *)inImage)->penWidth;
		
		if (pn == 1)
		{
			savedObj = ::SelectObject(dc, _NULL_BRUSH);
			::Ellipse(dc, inRect.left, inRect.top, inRect.right, inRect.bottom);
			::SelectObject(dc, savedObj);
		}
		else
		{
			Int32 left = inRect.left;
			Int32 top = inRect.top;
			Int32 right = inRect.right;
			Int32 bottom = inRect.bottom;
						
			if ((right - left) <= pn || (bottom - top) <= pn)
			{
				// rect is too small, draw solid instead (to avoid drawing outside of the rect)
				savedObj = ::SelectObject(dc, _NULL_PEN);
				::Ellipse(dc, left, top, right, bottom);
				::SelectObject(dc, savedObj);
			}
			else
			{
				if ((pn & 1) == 0)
				{
					right++;
					bottom++;
				}

				pn >>= 1;
				left += pn;
				top += pn;
				right -= pn;
				bottom -= pn;
				
				savedObj = ::SelectObject(dc, _NULL_BRUSH);
				::Ellipse(dc, left, top, right, bottom);
				::SelectObject(dc, savedObj);
			}
		}
	}
	else if (inFill != fill_None)
	{
		savedObj = ::SelectObject(dc, _NULL_PEN);
		::Ellipse(dc, inRect.left, inRect.top, inRect.right, inRect.bottom);
		::SelectObject(dc, savedObj);
	}
}

void UGraphics_DrawRoundRect(TImage inImage, const SRoundRect& inRect, Uint32 inFill)
{
	ASSERT(inImage);

	HGDIOBJ savedObj;
	HDC dc = (((SImage *)inImage)->dc);

	switch (inFill)
	{
		case fill_None:
			break;
		
		case fill_OpenFrame:
		case fill_ClosedFrame:
			savedObj = ::SelectObject(dc, _NULL_BRUSH);
			::RoundRect(dc, inRect.left, inRect.top, inRect.right, inRect.bottom, inRect.ovalWidth, inRect.ovalHeight);
			::SelectObject(dc, savedObj);
			break;
		
		default:
			savedObj = ::SelectObject(dc, _NULL_PEN);
			::RoundRect(dc, inRect.left, inRect.top, inRect.right, inRect.bottom, inRect.ovalWidth, inRect.ovalHeight);
			::SelectObject(dc, savedObj);
			break;
	}
}

void UGraphics_DrawPolygon(TImage inImage, const SPoint *inPointList, Uint32 inPointCount, Uint32 inFill, Uint32 /* inOptions */)
{
	if (!inPointList)
		return;

	ASSERT(inImage);

	if (inPointCount > 1)
	{
		HDC dc = (((SImage *)inImage)->dc);

		if (inFill == fill_OpenFrame || inFill == fill_ClosedFrame)
		{
			if ((((SImage *)inImage)->penWidth > 5) && (((SImage *)inImage)->penWidth & 1))	// if need to fix up stupid windoze line centering
			{
				const SPoint *firstPt = inPointList;
				
				::MoveToEx(dc, inPointList->x+1, inPointList->y+1, NULL);
			
				inPointCount--;
				inPointList++;
				
				while (inPointCount--)
				{
					::LineTo(dc, inPointList->x+1, inPointList->y+1);
					inPointList++;
				}

				if (inFill == fill_ClosedFrame)
					::LineTo(dc, firstPt->x+1, firstPt->y+1);
			}
			else
			{
				::Polyline(dc, (POINT *)inPointList, inPointCount);
				
				if (inFill == fill_ClosedFrame)
				{
					::MoveToEx(dc, inPointList[inPointCount-1].x, inPointList[inPointCount-1].y, NULL);
					::LineTo(dc, inPointList[0].x, inPointList[0].y);
				}
			}
		}
		else if (inFill != fill_None)
		{
			// draw filled
			if (::BeginPath(dc))
			{
				::Polyline(dc, (POINT *)inPointList, inPointCount);
				::EndPath(dc);
				::FillPath(dc);
				::AbortPath(dc);
			}
		}
	}
}

void UGraphics_DrawRegion(TImage inImage, TRegion inRgn, Uint32 inFill)
{
	if (!inRgn)
		return;
	
	ASSERT(inImage);
		
	switch (inFill)
	{
		case fill_None:
			break;
			
		case fill_OpenFrame:
		case fill_ClosedFrame:
			::FrameRgn((((SImage *)inImage)->dc), (HRGN)inRgn, ((SImage *)inImage)->brush, ((SImage *)inImage)->penWidth, ((SImage *)inImage)->penWidth);
			break;
			
		default:
			::PaintRgn((((SImage *)inImage)->dc), (HRGN)inRgn);
			break;
	}
}

void UGraphics_FillRect(TImage inImage, const SRect& inRect)
{
	ASSERT(inImage && inRect.IsValid());

	// we purposely use this func which ignores the line size because we want the rect just simply filled
	::FillRect((((SImage *)inImage)->dc), (RECT *)&inRect, ((SImage *)inImage)->brush);
}

void UGraphics_FrameRect(TImage inImage, const SRect& inRect)
{
	ASSERT(inImage && inRect.IsValid());
	
	HDC dc = (((SImage *)inImage)->dc);
	Uint32 pn = ((SImage *)inImage)->penWidth;

	if (pn == 1)
		::FrameRect(dc, (RECT *)&inRect, ((SImage *)inImage)->brush);	// this can only draw 1pt lines
	else
	{
		if (inRect.GetWidth() <= pn || inRect.GetHeight() <= pn)
			::FillRect(dc, (RECT *)&inRect, ((SImage *)inImage)->brush);
		else
		{
			// calculate the coords necessary to trick shitty windoze into drawing the rectangle entirely inside the rect
			Uint32 pnh = pn >> 1;
			Int32 left = inRect.left + pnh;
			Int32 top = inRect.top + pnh;
			Int32 right = inRect.right - pnh;
			Int32 bottom = inRect.bottom - pnh;
			if ((pn & 1) == 0)
			{
				right++;
				bottom++;
			}
			
			// frame the rect with thick line
			HGDIOBJ savedObj = ::SelectObject(dc, _NULL_BRUSH);
			::Rectangle(dc, left, top, right, bottom);
			::SelectObject(dc, savedObj);
		}
	}
}

void UGraphics_SetPixel(TImage inImage, const SPoint& inPixel, const SColor& inColor)
{
	ASSERT(inImage);
	
	::SetPixelV((((SImage *)inImage)->dc), inPixel.x, inPixel.y, RGBColor(inColor));
}

void UGraphics_SetPixels(TImage inImage, const SPoint *inPointList, Uint32 inPointCount, const SColor& inColor)
{
	if (!inPointList)
		return;

	ASSERT(inImage);
	
	HDC dc = (((SImage *)inImage)->dc);
	COLORREF col = RGBColor(inColor);
	
	while (inPointCount--)
	{
		::SetPixelV((((SImage *)inImage)->dc), inPointList->x, inPointList->y, col);
		inPointList++;
	}
}

void UGraphics_GetPixel(TImage inImage, const SPoint& inPixel, SColor& outColor)
{
	ASSERT(inImage);
	
	COLORREF c = ::GetPixel((((SImage *)inImage)->dc), inPixel.x, inPixel.y);
	outColor.Set(GetRValue(c) * 257, GetGValue(c) * 257, GetBValue(c) * 257);
}

void UGraphics_FloodFill(TImage inImage, const SPoint& inPt)
{
	ASSERT(inImage);
	
	HDC dc = (((SImage *)inImage)->dc);
	COLORREF col = ::GetPixel(dc, inPt.x, inPt.y);
	
	if (::GetDeviceCaps(dc, RASTERCAPS) & RC_PALETTE)
		col = PALETTEINDEX(::GetNearestPaletteIndex(_MAC_PALETTE, col));

	::ExtFloodFill(dc, inPt.x, inPt.y, col, FLOODFILLSURFACE);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void UGraphics::ResetFont(TImage inImage)
{
	ASSERT(inImage);
	::DeleteObject(::SelectObject((((SImage *)inImage)->dc), ::GetStockObject(SYSTEM_FONT)));
	((SImage *)inImage)->fontSize = ((SImage *)inImage)->fontEffect = 0;
}

void UGraphics::SetFontName(TImage inImage, const Uint8 *inName, const Uint8 *inStyle)
{
	ASSERT(inImage);
	UGraphics::SetFont(inImage, inName, inStyle, ((SImage *)inImage)->fontSize, ((SImage *)inImage)->fontEffect);
}

void UGraphics::SetFontSize(TImage inImage, Uint32 inSize)
{
	HDC dc;
	Uint32 effect;
	LOGFONT lf;
	
	ASSERT(inImage);
	dc = (((SImage *)inImage)->dc);
	effect = ((SImage *)inImage)->fontEffect;
	
	::GetTextFace(dc, LF_FACESIZE, lf.lfFaceName);

	lf.lfHeight = -inSize;
	lf.lfWidth = lf.lfEscapement = lf.lfOrientation = 0;
	lf.lfWeight = (effect & fontEffect_Bold) ? FW_BOLD : 0;
	lf.lfItalic = (effect & fontEffect_Italic) ? TRUE : FALSE;
	lf.lfUnderline = (effect & fontEffect_Underline) ? TRUE : FALSE;
	lf.lfStrikeOut = (effect & fontEffect_StrikeOut) ? TRUE : FALSE;
	lf.lfCharSet = lf.lfOutPrecision = lf.lfClipPrecision = lf.lfQuality = lf.lfPitchAndFamily = 0;
	
	::DeleteObject(::SelectObject(dc, ::CreateFontIndirect(&lf)));

	((SImage *)inImage)->fontSize = inSize;
}

Uint32 UGraphics::GetFontSize(TImage inImage)
{
	Require(inImage);
	return ((SImage *)inImage)->fontSize;
}

void UGraphics::SetFontEffect(TImage inImage, Uint32 inFlags)
{
	ASSERT(inImage);
	HDC dc = (((SImage *)inImage)->dc);
	LOGFONT lf;
		
	::GetTextFace(dc, LF_FACESIZE, lf.lfFaceName);

	lf.lfHeight = -((SImage *)inImage)->fontSize;
	lf.lfWidth = lf.lfEscapement = lf.lfOrientation = 0;
	lf.lfWeight = (inFlags & fontEffect_Bold) ? FW_BOLD : 0;
	lf.lfItalic = (inFlags & fontEffect_Italic) ? TRUE : FALSE;
	lf.lfUnderline = (inFlags & fontEffect_Underline) ? TRUE : FALSE;
	lf.lfStrikeOut = (inFlags & fontEffect_StrikeOut) ? TRUE : FALSE;
	lf.lfCharSet = lf.lfOutPrecision = lf.lfClipPrecision = lf.lfQuality = lf.lfPitchAndFamily = 0;
	
	::DeleteObject(::SelectObject(dc, ::CreateFontIndirect(&lf)));

	((SImage *)inImage)->fontEffect = inFlags;
}

Uint32 UGraphics::GetFontEffect(TImage inImage)
{
	Require(inImage);
	return ((SImage *)inImage)->fontEffect;
}

void UGraphics::SetFont(TImage inImage, const Uint8 *inName, const Uint8 */* inStyle */, Uint32 inSize, Uint32 inEffect)
{
	HDC dc;
	LOGFONT lf;
	
	ASSERT(inImage);
	dc = (((SImage *)inImage)->dc);

	lf.lfHeight = -inSize;
	lf.lfWidth = lf.lfEscapement = lf.lfOrientation = 0;
	lf.lfWeight = (inEffect & fontEffect_Bold) ? FW_BOLD : 0;
	lf.lfItalic = (inEffect & fontEffect_Italic) ? TRUE : FALSE;
	lf.lfUnderline = (inEffect & fontEffect_Underline) ? TRUE : FALSE;
	lf.lfStrikeOut = (inEffect & fontEffect_StrikeOut) ? TRUE : FALSE;
	lf.lfCharSet = lf.lfOutPrecision = lf.lfClipPrecision = lf.lfQuality = lf.lfPitchAndFamily = 0;
	
	_FontNameToWinName(inName, lf.lfFaceName, LF_FACESIZE);
		
	::DeleteObject(::SelectObject(dc, ::CreateFontIndirect(&lf)));

	((SImage *)inImage)->fontSize = inSize;
	((SImage *)inImage)->fontEffect = inEffect;
}

void UGraphics::SetFont(TImage inImage, const SFontDesc& inInfo)
{
	HDC dc;
	LOGFONT lf;
	Uint32 effect;
	
	ASSERT(inImage);
	dc = (((SImage *)inImage)->dc);
	effect = inInfo.effect;

	lf.lfHeight = -inInfo.size;
	lf.lfWidth = lf.lfEscapement = lf.lfOrientation = 0;
	lf.lfWeight = (effect & fontEffect_Bold) ? FW_BOLD : 0;
	lf.lfItalic = (effect & fontEffect_Italic) ? TRUE : FALSE;
	lf.lfUnderline = (effect & fontEffect_Underline) ? TRUE : FALSE;
	lf.lfStrikeOut = (effect & fontEffect_StrikeOut) ? TRUE : FALSE;
	lf.lfCharSet = lf.lfOutPrecision = lf.lfClipPrecision = lf.lfQuality = lf.lfPitchAndFamily = 0;

	_FontNameToWinName(inInfo.name, lf.lfFaceName, LF_FACESIZE);
	
	::DeleteObject(::SelectObject(dc, ::CreateFontIndirect(&lf)));

	((SImage *)inImage)->fontSize = inInfo.size;
	((SImage *)inImage)->fontEffect = effect;
}

void UGraphics::SetFont(TImage inImage, TFontDesc inFD)
{
	ASSERT(inImage);
	if (inFD)
	{
		::DeleteObject(::SelectObject((((SImage *)inImage)->dc), ::CreateFontIndirect(&FD->lf)));
		SetInkColor(inImage, FD->color);

		((SImage *)inImage)->fontSize = FD->size;
		((SImage *)inImage)->fontEffect = FD->effect;
	}
	else
	{
		::DeleteObject(::SelectObject((((SImage *)inImage)->dc), ::CreateFontIndirect(&_gDefaultFontDesc.lf)));
		SetInkColor(inImage, _gDefaultFontDesc.color);

		((SImage *)inImage)->fontSize = _gDefaultFontDesc.size;
		((SImage *)inImage)->fontEffect = _gDefaultFontDesc.effect;
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void UGraphics::GetFontMetrics(TImage inImage, SFontMetrics& outInfo)
{
	TEXTMETRIC tm;
	
	ASSERT(inImage);
	::GetTextMetrics((((SImage *)inImage)->dc), &tm);
	
	outInfo.ascent = tm.tmAscent;
	outInfo.descent = tm.tmDescent;
	outInfo.lineSpace = tm.tmAscent + tm.tmDescent + tm.tmExternalLeading;
	outInfo.internal = tm.tmInternalLeading;
	outInfo.rsvd[0] = outInfo.rsvd[1] = outInfo.rsvd[2] = outInfo.rsvd[3] = 0;
}

Uint32 UGraphics::GetFontHeight(TImage inImage)
{
	TEXTMETRIC tm;
	
	ASSERT(inImage);
	::GetTextMetrics((((SImage *)inImage)->dc), &tm);

	return tm.tmHeight;
}

Uint32 UGraphics::GetFontLineHeight(TImage inImage)
{
	TEXTMETRIC tm;
	
	ASSERT(inImage);
	::GetTextMetrics((((SImage *)inImage)->dc), &tm);

	return tm.tmAscent + tm.tmDescent + tm.tmExternalLeading;
}

Uint32 UGraphics::GetCharWidth(TImage inImage, Uint16 inChar, Uint32 /* inEncoding */)
{
	int buf[2];
	
	ASSERT(inImage);
	
	// the second call to GetCharWidth() is necessary because GetCharWidth32() can return RROR_CALL_NOT_IMPLEMENTED
	if (!::GetCharWidth32((((SImage *)inImage)->dc), inChar, inChar, buf) && !::GetCharWidth((((SImage *)inImage)->dc), inChar, inChar, buf))
		buf[0] = 0;
	
	return buf[0];
}

Uint32 UGraphics::GetTextWidth(TImage inImage, const void *inText, Uint32 inTextSize, Uint32 /* inEncoding */)
{
	SIZE s;
	
	ASSERT(inImage);
	::GetTextExtentPoint32((((SImage *)inImage)->dc), (char *)inText, inTextSize, &s);

	return s.cx;
}

static Uint32 _GRCalcMaxFit(HDC inDC, const void *inText, Uint32 inTextSize, Uint32 inMaxWidth)
{
	// if no text or zero width, then no chars will fit
	if (!inText || !inTextSize || !inMaxWidth) 
		return 0;
	
	/*
	Windoze is really slow at calculating the max fit, so we do it ourselves.  There are two
	ways to do it using windoze.  Like this:
	
	::GetTextExtentExPoint(inDC, (char *)inText, inTextSize, inMaxWidth, &nMaxFit, NULL, &siz);
	
	Or like this:

	GCP_RESULTS info = { sizeof(GCP_RESULTS),0,0,0,0,0,0,inTextSize,0 };
	::GetCharacterPlacement(inDC, (char *)inText, inTextSize, inMaxWidth, &info, GCP_MAXEXTENT);
	info.nMaxFit
	
	On WinNT, either way is incredibly slow.  Completely unacceptably slow.  On 95, the 
	GetTextExtentExPoint is slow but bearable.  GetCharacterPlacement is okay.
	
	But the fastest way to do it on either is the following.
	*/

	TEXTMETRIC tm;
	SIZE siz;
	Uint32 count;
	
	// get metrics of the current font
	::GetTextMetrics(inDC, &tm);

	// guess the max fit
	count = inMaxWidth / tm.tmAveCharWidth;
	if (count > inTextSize) count = inTextSize;
	
	// see how wide our guess is
	GetTextExtentPoint32(inDC, (char *)inText, count, &siz);
	
	if (siz.cx == inMaxWidth)
	{
		// we exactly guessed how many chars would fit, wow!
		return count;
	}
	else if (siz.cx > inMaxWidth)
	{
		// the guess is wider than the max, so keep removing one character until it fits
		for(;;)
		{
			if (count == 0) return 0;
			count--;
			
			GetTextExtentPoint32(inDC, (char *)inText, count, &siz);
			
			if (siz.cx <= inMaxWidth) return count;
		}
	}
	else
	{
		// the guess is narrower than the max, so keep adding one more character until it won't fit
		for(;;)
		{
			count++;
			if (count > inTextSize) return inTextSize;
			
			GetTextExtentPoint32(inDC, (char *)inText, count, &siz);
			
			if (siz.cx > inMaxWidth) return count-1;
		}
	}
	
	// should never get here
	return 0;
}

// returns offset into <inText> of the line break (same as size of the line)
// spaces and carriage returns cause breaks AFTER the space (ie spaces and CRs at end of line)
Uint32 UGraphics::GetTextLineBreak(TImage inImage, const void *inText, Uint32 inTextSize, Uint32 inMaxWidth)
{
	if (!inText || !inTextSize) 
		return 0;
	
	Uint8 *p;
	Uint32 n;
	
	// determine max chars for this line
	Uint32 nMaxFit = _GRCalcMaxFit((((SImage *)inImage)->dc), (char *)inText, inTextSize, inMaxWidth);
	
	// break at least 1 character
	if (nMaxFit == 0) return 1;
	
	// if return before max, break after that return
	p = (Uint8 *)inText;
	n = nMaxFit;
	while (n--)
	{
		if (*p++ == '\r')
			return p - (Uint8 *)inText;
	}
	
	// check if all of the text fits
	if (nMaxFit >= inTextSize) return inTextSize;
	
	// break after space nearest the end
	p = (Uint8 *)inText + nMaxFit;
	n = nMaxFit;
	while (n--)
	{
		if (*--p == ' ')
			return (p+1) - (Uint8 *)inText;
	}
	
	// no space, break at the max
	return nMaxFit;
}

// returns offset of character that <inWidth> is closest to
// offset might equal inTextSize, meaning end of text
Uint32 UGraphics::WidthToChar(TImage inImage, const void *inText, Uint32 inTextSize, Uint32 inWidth, bool *outLeftSide)
{
	if (!inText || !inTextSize)
	{
		if (outLeftSide) *outLeftSide = 1;
		return 0;
	}
	
	GCP_RESULTS info = { sizeof(GCP_RESULTS),0,0,0,0,0,0,inTextSize,0 };
	SIZE extent;
	int charWidth[2];
	Uint8 c;
	
	ASSERT(inImage);
	HDC dc = (((SImage *)inImage)->dc);
	
	// special case for less than 1 char widths (GetCharacterPlacement stuffs)
	c = ((Uint8 *)inText)[0];
	if (!::GetCharWidth32(dc, c, c, charWidth) && !::GetCharWidth(dc, c, c, charWidth))	// the second call to GetCharWidth() is necessary to work around bug in win95
	{
#if DEBUG
		DWORD err = ::GetLastError();
		DebugBreak("WidthToChar - error getting char width:  %lu", err);
#endif
		return 0;
	}
	if (inWidth <= charWidth[0])
	{
		if (inWidth >= (charWidth[0] / 2))
		{
			if (outLeftSide) *outLeftSide = 0;
			return 1;
		}
		else
		{
			if (outLeftSide) *outLeftSide = 1;
			return 0;
		}
	}

	// determine max chars that will fit in the width
	::GetCharacterPlacement(dc, (char *)inText, inTextSize, inWidth, &info, GCP_MAXEXTENT);
	
	// if all chars fit, then position must be at end
	if (info.nMaxFit >= inTextSize)
	{
		if (outLeftSide) *outLeftSide = 0;
		return inTextSize;
	}
	
	// get width of text that fits, and width of character that inWidth is in
	::GetTextExtentPoint32(dc, (char *)inText, info.nMaxFit, &extent);
	c = ((Uint8 *)inText)[info.nMaxFit];
	if (!::GetCharWidth32(dc, c, c, charWidth) && !::GetCharWidth(dc, c, c, charWidth))	// the second call to GetCharWidth() is necessary to work around bug in win95
		charWidth[0] = 0;
		
	// if inWidth is past the half-way mark, then position should be on the right
	if (inWidth >= (extent.cx + (charWidth[0] / 2)))
	{
		if (outLeftSide) *outLeftSide = 0;
		return info.nMaxFit + 1;
	}
	
	// info.nMaxFit is the always-left position
	if (outLeftSide) *outLeftSide = 1;
	return info.nMaxFit;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void UGraphics_DrawText(TImage inImage, const SRect& inBounds, const void *inText, Uint32 inTextSize, Uint32 /* inEncoding */, Uint32 inAlignFlags)
{
	if (!inText || !inTextSize)
		return;

	ASSERT(inImage);

	Uint32 flags = DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE;
	
	if (inAlignFlags & align_Right)
		flags |= DT_RIGHT;
	else if (inAlignFlags & align_CenterHoriz)
		flags |= DT_CENTER;
	else
		flags |= DT_LEFT;
		
	if (inAlignFlags & align_Bottom)
		flags |= DT_BOTTOM;
	else if (inAlignFlags & align_CenterVert)
		flags |= DT_VCENTER;
	else
		flags |= DT_TOP;
	
	StPtr mapped(inTextSize + 1);	// +1 only so if we have size = 0 we have no errors
	
	char *p = mapped;
	Uint8 *q = (Uint8 *)inText;
	Uint32 s = inTextSize;
	
	while (s--)
	{
		*p++ = _UTCharMap_AWToPC[*q++];
	}
		
	::DrawTextA((((SImage *)inImage)->dc), (char *)mapped, inTextSize, (LPRECT)&inBounds, flags);
}

void UGraphics_DrawTruncText(TImage inImage, const SRect& inBounds, const void *inText, Uint32 inTextSize, Uint32 /* inEncoding */, Uint32 inAlignFlags)
{
	if (!inText || !inTextSize)
		return;

	ASSERT(inImage);

	Uint32 flags = DT_NOPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS;
	
	if (inAlignFlags & align_Right)
		flags |= DT_RIGHT;
	else if (inAlignFlags & align_CenterHoriz)
		flags |= DT_CENTER;
	else
		flags |= DT_LEFT;
		
	if (inAlignFlags & align_Bottom)
		flags |= DT_BOTTOM;
	else if (inAlignFlags & align_CenterVert)
		flags |= DT_VCENTER;
	else
		flags |= DT_TOP;
	
	StPtr mapped(inTextSize + 1);
	
	char *p = mapped;
	Uint8 *q = (Uint8 *)inText;
	Uint32 s = inTextSize;
	
	while (s--)
	{
		*p++ = _UTCharMap_AWToPC[*q++];
	}
	
	::DrawTextEx((((SImage *)inImage)->dc), (char *)mapped, inTextSize, (LPRECT)&inBounds, flags, NULL);
}

void UGraphics_DrawTextBox(TImage inImage, const SRect& inBounds, const SRect& /* inUpdateRect */, const void *inText, Uint32 inTextSize, Uint32 /* inEncoding */, Uint32 inAlign)
{
	if (!inText || !inTextSize)
		return;

	ASSERT(inImage);

	Uint32 flags = DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK;
	
	if (inAlign == textAlign_Right)
		flags |= DT_RIGHT;
	else if (inAlign == textAlign_Center)
		flags |= DT_CENTER;
	else
		flags |= DT_LEFT;
	
	StPtr mapped(inTextSize + 1);
	
	char *p = mapped;
	Uint8 *q = (Uint8 *)inText;
	Uint32 s = inTextSize;
	
	while (s--)
	{
		*p++ = _UTCharMap_AWToPC[*q++];
	}
	
	::DrawTextA((((SImage *)inImage)->dc), (char *)mapped, inTextSize, (LPRECT)&inBounds, flags);
}

Uint32 UGraphics_GetTextBoxHeight(TImage inImage, const SRect& inBounds, const void *inText, Uint32 inTextSize, Uint32 /* inEncoding */, Uint32 inAlign)
{
	if (!inText || !inTextSize)
		return 0;

	ASSERT(inImage);

	Uint32 flags = DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK | DT_CALCRECT;
	RECT r = *(RECT *)&inBounds;
	
	if (inAlign == textAlign_Right)
		flags |= DT_RIGHT;
	else if (inAlign == textAlign_Center)
		flags |= DT_CENTER;
	else
		flags |= DT_LEFT;
	
	return ::DrawTextA((((SImage *)inImage)->dc), (char *)inText, inTextSize, &r, flags);
}

// drawing includes inStartLine (0-based), does NOT include inEndLine
void UGraphics_DrawTextLines(TImage inImage, const SRect& inBounds, const void *inText, Uint32 /* inEncoding */, const Uint32 *inLineOffsets, Uint32 inStartLine, Uint32 inEndLine, Uint32 inLineHeight, Uint32 inAlign)
{
	if (!inText || inStartLine >= inEndLine)
		return;

	ASSERT(inImage);

	char convTxt[4096];	// won't have more than 4k per line
	HDC dc = (((SImage *)inImage)->dc);
	Int32 curY, boundsLeft, boundsRight, boundsMiddle;
	Uint8 *linePtr;
	Uint32 lineBytes, i;

	i = inStartLine;
	boundsLeft = inBounds.left;
	boundsRight = inBounds.right;
	boundsMiddle = (inBounds.left + inBounds.right) / 2;
	curY = inBounds.top + (i * inLineHeight);
	
	/*****************************
	Might be faster to use PolyTextOut().
	First check if the pdx parameter can be NULL.
	***********************************/
	
	if (inAlign == textAlign_Right)
	{
		::SetTextAlign(dc, TA_TOP | TA_RIGHT);
		
		while (i != inEndLine)
		{
			linePtr = (Uint8 *)inText + inLineOffsets[i];
			lineBytes = inLineOffsets[i+1] - inLineOffsets[i];
			
			char *p = convTxt;
			Uint8 *q = (Uint8 *)linePtr;
			Uint32 s = lineBytes;
			
			while (s--)
			{
				*p++ = _UTCharMap_AWToPC[*q++];
			}
			::TextOut(dc, boundsRight, curY, (char *)linePtr, UText::GetVisibleLength(linePtr, lineBytes));
			
			curY += inLineHeight;
			i++;
		}
	}
	else if (inAlign == textAlign_Center)
	{
		::SetTextAlign(dc, TA_TOP | TA_CENTER);

		while (i != inEndLine)
		{
			linePtr = (Uint8 *)inText + inLineOffsets[i];
			lineBytes = inLineOffsets[i+1] - inLineOffsets[i];
			
			char *p = convTxt;
			Uint8 *q = (Uint8 *)linePtr;
			Uint32 s = lineBytes;
			
			while (s--)
			{
				*p++ = _UTCharMap_AWToPC[*q++];
			}
			::TextOut(dc, boundsMiddle, curY, (char *)linePtr, UText::GetVisibleLength(linePtr, lineBytes));

			curY += inLineHeight;
			i++;
		}
	}
	else
	{
		::SetTextAlign(dc, TA_TOP | TA_LEFT);
		
		while (i != inEndLine)
		{
			linePtr = (Uint8 *)inText + inLineOffsets[i];
			lineBytes = inLineOffsets[i+1] - inLineOffsets[i];
			
			// don't draw carriage return character at end (it shows up as black square - stupid huh?)
			if (lineBytes && linePtr[lineBytes-1] == '\r')
				lineBytes--;
			
			char *p = convTxt;
			Uint8 *q = (Uint8 *)linePtr;
			Uint32 s = lineBytes;
			
			while (s--)
			{
				*p++ = _UTCharMap_AWToPC[*q++];
			}
			::TextOut(dc, boundsLeft, curY, convTxt, lineBytes);

			curY += inLineHeight;
			i++;
		}
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

TImage UGraphics::NewCompatibleImage(Uint32 inWidth, Uint32 inHeight)
{
	HDC onscreenDC = NULL;
	HDC offscreenDC = NULL;
	HBITMAP offscreenBM = NULL;
	SImage *img = NULL;
	
	try
	{
		// get on-screen DC
		onscreenDC = _gDummyImage.dc;

		// create off-screen DC
		offscreenDC = ::CreateCompatibleDC(onscreenDC);
		if (offscreenDC == NULL) Fail(errorType_Memory, memError_NotEnough);
		
		// select nicer color palette
		if (::SelectPalette(offscreenDC, _MAC_PALETTE, false) == NULL)
			FailLastWinError();
		if (::RealizePalette(offscreenDC) == GDI_ERROR)
			FailLastWinError();
		
		// create bitmap for off-screen DC
		offscreenBM = ::CreateCompatibleBitmap(onscreenDC, inWidth, inHeight);
		if (offscreenBM == NULL) Fail(errorType_Memory, memError_NotEnough);
		::SelectObject(offscreenDC, offscreenBM);

		// create UGraphics-compatible TImage to use with off-screen DC
		::SetBkMode(offscreenDC, TRANSPARENT);
		img = (SImage *)UMemory::NewClear(sizeof(SImage));
		img->dc = offscreenDC;
		img->bitmap = offscreenBM;
		img->width = inWidth;
		img->height = inHeight;
	}
	catch(...)
	{
		// clean up
		if (offscreenDC) ::DeleteDC(offscreenDC);
		if (offscreenBM) ::DeleteObject(offscreenBM);
		UMemory::Dispose((TPtr)img);
		throw;
	}
	
	// successfully created offscreen image
	return (TImage)img;
}

void UGraphics::DisposeImage(TImage inImage)
{
	if (inImage)
	{
		if (((SImage *)inImage)->bitmap == NULL)
		{
			DebugBreak("UGraphics - specified image should not be disposed");
			return;
		}
		
		HDC offscreenDC = (((SImage *)inImage)->dc);
		
		// delete objects
		::DeleteObject(::SelectObject(offscreenDC, ::GetStockObject(SYSTEM_FONT)));
		::DeleteObject(::SelectObject(offscreenDC, _NULL_BRUSH));
		::DeleteObject(::SelectObject(offscreenDC, _NULL_PEN));
		
		::DeleteDC(offscreenDC);
		::DeleteObject(((SImage *)inImage)->bitmap);

		UMemory::Dispose((TPtr)inImage);
	}
}

void UGraphics::LockImage(TImage /* inImage */)
{
	// windoze bitmaps don't need to be locked
}

void UGraphics::UnlockImage(TImage /* inImage */)
{
	// windoze bitmaps don't need to be locked
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void UGraphics_CopyPixels(TImage inDest, const SPoint& inDestPt, TImage inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 /* inOptions */)
{
	SPoint theDestPt, theSourcePt, org;
	SRect dstBnd, srcBnd;
	
	Require(inDest && inSource);
	
	HDC dstDC = ((SImage *)inDest)->dc;
	HDC srcDC = ((SImage *)inSource)->dc;
	
	theDestPt = inDestPt;
	theSourcePt = inSourcePt;
	
	::GetViewportOrgEx(dstDC, (LPPOINT)&org);
	dstBnd.Set(-org.x, -org.y, -org.x + ((SImage *)inDest)->width, -org.y + ((SImage *)inDest)->height);
	
	::GetViewportOrgEx(srcDC, (LPPOINT)&org);
	srcBnd.Set(-org.x, -org.y, -org.x + ((SImage *)inSource)->width, -org.y + ((SImage *)inSource)->height);
	
	if (UGraphics::ValidateCopyRects(dstBnd, srcBnd, theDestPt, theSourcePt, inWidth, inHeight))
		::BitBlt(dstDC, theDestPt.x, theDestPt.y, inWidth, inHeight, srcDC, theSourcePt.x, theSourcePt.y, SRCCOPY);
}

void UGraphics_CopyPixels(const SPixmap& inDest, const SPoint& inDestPt, TImage inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 /* inOptions */)
{
	#pragma unused(inDest, inDestPt, inSource, inSourcePt, inWidth, inHeight)
	
	Fail(errorType_Misc, error_Unimplemented);
	
	/*
	Maybe the way to do this efficiently is to go
	
	CreateDIBSection
	and specify the memory in inDest
	
	then use BitBlt to copy from inSource to the DIB section (which is inDest)
	
	perfect!
	
	****** actually this prolly won't work because CreateDIBSection can't be told to use inDest (I think)
	yeah that's right, so have to copy inSource into a temporary DIB section
	*/
}

void UGraphics_CopyPixels(TImage inDest, const SPoint& inDestPt, const SPixmap& inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 /* inOptions */)
{
	Uint8 buf[2048];
	BITMAPINFO *bmi = (BITMAPINFO *)buf;
	Uint32 colorCount;
	
	// sanity checks on inDest
	Require(inDest);
	
	// sanity checks on inSource
	if (!UPixmap::IsValid(inSource))
	{
		DebugBreak("UGraphics::CopyPixels - invalid source pixmap");
		Fail(errorType_Misc, error_Param);
	}
	
	// validate the copy rects
	SPoint theDestPt, theSourcePt, org;
	SRect dstBnd, srcBnd;
	HDC dstDC = ((SImage *)inDest)->dc;
	theDestPt = inDestPt;
	theSourcePt = inSourcePt;
	::GetViewportOrgEx(dstDC, (LPPOINT)&org);
	dstBnd.Set(-org.x, -org.y, -org.x + ((SImage *)inDest)->width, -org.y + ((SImage *)inDest)->height);
	srcBnd.Set(0, 0, inSource.width, inSource.height);
	if (!UGraphics::ValidateCopyRects(dstBnd, srcBnd, theDestPt, theSourcePt, inWidth, inHeight))
		return;
	
	// check depth
	switch (inSource.depth)
	{
		case 1:
		case 2:
		case 4:
		case 8:
			// ignore any extra (unused) colors
			colorCount = inSource.colorCount;
			if (colorCount > 256) colorCount = 256;
			
			// fill in struct of info about source pixmap
			bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi->bmiHeader.biWidth = inWidth;
			bmi->bmiHeader.biHeight = -inHeight;		// the - (negative) is necessary otherwise the image appears upside down
			bmi->bmiHeader.biPlanes = 1; 
			bmi->bmiHeader.biBitCount = inSource.depth;
			bmi->bmiHeader.biCompression = BI_RGB;
			bmi->bmiHeader.biSizeImage = 0; 
			bmi->bmiHeader.biXPelsPerMeter = 0; 
			bmi->bmiHeader.biYPelsPerMeter = 0; 
			bmi->bmiHeader.biClrUsed = colorCount; 
			bmi->bmiHeader.biClrImportant = 0; 
			
			// convert AW RGBA colors to icky windoze BGRA format
			Uint8 *bmicol = (Uint8 *)bmi->bmiColors;
			Uint8 *awcol = (Uint8 *)inSource.colorTab;
			while (colorCount--)
			{
				bmicol[0] = awcol[2];
				bmicol[1] = awcol[1];
				bmicol[2] = awcol[0];
				bmicol[3] = awcol[3];
				bmicol += 4;
				awcol += 4;
			}
			
			// perform the copy
			if (inSource.rowBytes != RoundUp4(inSource.rowBytes))
			{
				if (inSource.depth == 8)
				{
					Uint8 *ip = BPTR(inSource.data) + (theSourcePt.y * inSource.rowBytes);
					Uint32 indent = theSourcePt.x > 0 ? theSourcePt.x : 0;
									
					Uint32 theRowBytes = inWidth + theSourcePt.x;
					Uint32 rowWidth;
					Uint32 tail;
					
					if (theRowBytes < inSource.rowBytes)
					{
						rowWidth = inWidth;
						tail = inSource.rowBytes - theRowBytes;
					}
					else
					{
						rowWidth = inSource.rowBytes - theSourcePt.x;
						tail = 0;
					}
					
					Uint32 lines = inHeight;
					
					Uint32 outRowBytes = rowWidth;
					Uint32 roundUpSize = RoundUp4(theRowBytes);
					
					StPtr data(lines * roundUpSize);
					
					Uint8 *op = (Uint8 *)data;
					
					Uint32 remainder = roundUpSize - outRowBytes;
					
					tail += rowWidth;
					while (lines--)
					{
						ip += indent;
						op += UMemory::Copy(op, ip, rowWidth) + remainder;
						ip += tail;
					}
					
					::SetDIBitsToDevice(dstDC, theDestPt.x, theDestPt.y, inWidth, inHeight, 0, 0, 0, inHeight, data, bmi, DIB_RGB_COLORS);
						
				}
				else
				{
					DebugBreak("UGraphics::CopyPixels - this version can only handle 4-byte aligned rowBytes");
					Fail(errorType_Misc, error_Unimplemented);
				}
			}
			else
			{
				::SetDIBitsToDevice(dstDC, theDestPt.x, theDestPt.y, inWidth, inHeight, theSourcePt.x, 0, 0, inHeight, BPTR(inSource.data) + (theSourcePt.y * inSource.rowBytes), bmi, DIB_RGB_COLORS);
			}
			break;
		
		case 16:
			{
				// fill in struct of info about source pixmap
				bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				bmi->bmiHeader.biWidth = inWidth;
				bmi->bmiHeader.biHeight = -inHeight;		// the - (negative) is necessary otherwise the image appears upside down
				bmi->bmiHeader.biPlanes = 1; 
				bmi->bmiHeader.biBitCount = inSource.depth;
				bmi->bmiHeader.biCompression = BI_RGB;
				bmi->bmiHeader.biSizeImage = 0; 
				bmi->bmiHeader.biXPelsPerMeter = 0; 
				bmi->bmiHeader.biYPelsPerMeter = 0; 
				bmi->bmiHeader.biClrUsed = inSource.colorCount; 
				bmi->bmiHeader.biClrImportant = 0; 
				*((Uint32*)(bmi->bmiColors)) = nil;
				// do 1 scan line at a time?  naw, let's build it all
				Uint16 *ip = (Uint16 *)(BPTR(inSource.data) + (theSourcePt.y * inSource.rowBytes));
				
				// only output to the rect specified
				
				Uint32 indent = theSourcePt.x > 0 ? theSourcePt.x * sizeof(Uint16) : 0;
								
				Uint32 theRowBytes = (inWidth + theSourcePt.x) * sizeof(Uint16);
				Uint32 rowWidth;
				Uint32 tail;
				
				if (theRowBytes < inSource.rowBytes)
				{
					rowWidth = inWidth;
					tail = inSource.rowBytes - theRowBytes;
				}
				else
				{
					rowWidth = inSource.rowBytes / sizeof(Uint16) - theSourcePt.x;
					tail = 0;
				}
	
				Uint32 lines = inHeight;
				
				Uint32 outRowBytes = rowWidth * sizeof(Uint16);
				Uint32 roundUpSize = RoundUp4(theRowBytes);
				
				StPtr data(lines * roundUpSize);
				
				Uint16 *op = (Uint16 *)BPTR(data);
				
				if (outRowBytes == roundUpSize)
				{
					while (lines--)
					{
						(Uint8 *)ip += indent;
						Uint32 w = rowWidth;
						
						while(w--)
						{
							//*op++ = ((*ip << 10) & 0x7C00) | ((*ip << 1)  & 0x03E0) | ((*ip >> 10) & 0x001F);
							*op++ = *ip << 1 | *ip >> 15;
							ip++;
						}
						
						(Uint8 *)ip += tail;
					}
				}
				else
				{
					Uint32 remainder = roundUpSize - outRowBytes;
					while (lines--)
					{
						(Uint8 *)ip += indent;
						Uint32 w = rowWidth;
						
						while(w--)
						{
							//*op++ = ((*ip << 10) & 0x7C00) | ((*ip << 1)  & 0x03E0) | ((*ip >> 10) & 0x001F);
							*op++ = *ip << 1 | *ip >> 15;
							ip++;
						}
						
						(Uint8 *)ip += tail;
						(Uint8 *)op += remainder;
					}
				}
				::SetDIBitsToDevice(dstDC, theDestPt.x, theDestPt.y, inWidth, inHeight, 0, 0, 0, inHeight, data, bmi, DIB_RGB_COLORS);
			}
			break;
			
			
			
		case 24:
			{
				// fill in struct of info about source pixmap
				bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				bmi->bmiHeader.biWidth = inWidth;
				bmi->bmiHeader.biHeight = -inHeight;		// the - (negative) is necessary otherwise the image appears upside down
				bmi->bmiHeader.biPlanes = 1; 
				bmi->bmiHeader.biBitCount = inSource.depth;
				bmi->bmiHeader.biCompression = BI_RGB;
				bmi->bmiHeader.biSizeImage = 0; 
				bmi->bmiHeader.biXPelsPerMeter = 0; 
				bmi->bmiHeader.biYPelsPerMeter = 0; 
				bmi->bmiHeader.biClrUsed = inSource.colorCount; 
				bmi->bmiHeader.biClrImportant = 0; 

				// do 1 scan line at a time?  naw, let's build it all
				Uint8 *ip = BPTR(inSource.data) + (theSourcePt.y * inSource.rowBytes);
				
				// only output to the rect specified
				
				Uint32 indent = theSourcePt.x > 0 ? theSourcePt.x * 3 : 0;
								
				Uint32 theRowBytes = (inWidth + theSourcePt.x) * 3;
				Uint32 rowWidth;
				Uint32 tail;
				
				if (theRowBytes < inSource.rowBytes)
				{
					rowWidth = inWidth;
					tail = inSource.rowBytes - theRowBytes;
				}
				else
				{
					rowWidth = inSource.rowBytes / 3 - theSourcePt.x;
					tail = 0;
				}
	
				Uint32 lines = inHeight;
				
				Uint32 outRowBytes = rowWidth * 3;
				Uint32 roundUpSize = RoundUp4(theRowBytes);
				
				StPtr data(lines * roundUpSize);
				
				Uint8 *op = (Uint8 *)data;
				
				if (outRowBytes == roundUpSize)
				{
					while (lines--)
					{
						ip += indent;
						Uint32 w = rowWidth;
						
						while(w--)
						{
							*op++ = ip[2];	// G
							*op++ = ip[1];	// B
							*op++ = ip[0];	// R
							ip += 3;
						}
						
						ip += tail;
					}
				}
				else
				{
					Uint32 remainder = roundUpSize - outRowBytes;
					while (lines--)
					{
						ip += indent;
						Uint32 w = rowWidth;
						
						while(w--)
						{
							*op++ = ip[2];	// G
							*op++ = ip[1];	// B
							*op++ = ip[0];	// R
							ip += 3;
						}
						
						ip += tail;
						op += remainder;
					}
				}
				::SetDIBitsToDevice(dstDC, theDestPt.x, theDestPt.y, inWidth, inHeight, 0, 0, 0, inHeight, data, bmi, DIB_RGB_COLORS);
			}
			break;
		
		case 32:
			{
				// fill in struct of info about source pixmap
				bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				bmi->bmiHeader.biWidth = inWidth;
				bmi->bmiHeader.biHeight = -inHeight;		// the - (negative) is necessary otherwise the image appears upside down
				bmi->bmiHeader.biPlanes = 1; 
				bmi->bmiHeader.biBitCount = inSource.depth;
				bmi->bmiHeader.biCompression = BI_RGB;
				bmi->bmiHeader.biSizeImage = 0; 
				bmi->bmiHeader.biXPelsPerMeter = 0; 
				bmi->bmiHeader.biYPelsPerMeter = 0; 
				bmi->bmiHeader.biClrUsed = inSource.colorCount; 
				bmi->bmiHeader.biClrImportant = 0; 

				// do 1 scan line at a time?  naw, let's build it all
				Uint32 pixels = inHeight * inSource.rowBytes;
				Uint8 *ip = BPTR(inSource.data) + (theSourcePt.y * inSource.rowBytes);
				
				// only output to the rect specified
				
				inSourcePt.x;
				Uint32 indent = theSourcePt.x > 0 ? theSourcePt.x * sizeof(Uint32) : 0;
								
				Uint32 theRowBytes = (inWidth + theSourcePt.x) * sizeof(Uint32);
				Uint32 rowWidth;
				Uint32 tail;
				
				if (theRowBytes < inSource.rowBytes)
				{
					rowWidth = inWidth;
					tail = inSource.rowBytes - theRowBytes;
				}
				else
				{
					rowWidth = inSource.rowBytes / sizeof(Uint32) - theSourcePt.x;
					tail = 0;
				}
	
				Uint32 lines = inHeight;
				
				StPtr data(lines * rowWidth * sizeof(Uint32));
				
				Uint8 *op = (Uint8 *)data;
				
				while (lines--)
				{
					ip += indent;
					Uint32 w = rowWidth;
					
					while(w--)
					{
						*op++ = ip[2];	// B
						*op++ = ip[1];	// G
						*op++ = ip[0];	// R
						*op++ = ip[3];	// A
						ip += 4;
					}
					
					ip += tail;
				}
								
				::SetDIBitsToDevice(dstDC, theDestPt.x, theDestPt.y, inWidth, inHeight, 0, 0, 0, inHeight, data, bmi, DIB_RGB_COLORS);
			}
			break;
	
		default:
			Fail(errorType_Misc, error_Param);
			break;
	}
	
}

inline Uint32 _GRSwapRGBAandBGRA(Uint32 inVal)
{
	Uint8 *p = (Uint8 *)&inVal;
	
    Uint8 tmp = p[0];
    p[0] = p[2];
    p[2] = tmp;
	
	return inVal;
}

// given an area of a DC, makes a mask the same size where 1 pixels in the mask are opaque (non-transparent) pixels; returns nil if fails
static HBITMAP _GRMakeTransMask(HDC inDC, const SPoint& inPt, Uint32 inWidth, Uint32 inHeight, Uint32 inTransCol)
{
	HBITMAP srcBitmap, mskBitmap, tmpBitmap, dudBitmap;
	void *srcBitmapBits, *mskBitmapBits;
	HDC tmpDC;
	Uint8 mskBMIBuf[64];
	Uint8 srcBMIBuf[2048];
	BITMAPINFO *mskBMI = (BITMAPINFO *)mskBMIBuf;
	BITMAPINFO *srcBMI = (BITMAPINFO *)srcBMIBuf;

	// initialize ptrs
	srcBitmap = mskBitmap = tmpBitmap = dudBitmap = NULL;
	tmpDC = NULL;
	
	// create dud bitmap
	dudBitmap = ::CreateBitmap(0, 0, 1, 1, NULL);
	if (dudBitmap == NULL) return nil;
	
	// get source bitmap
	srcBitmap = (HBITMAP)::SelectObject(inDC, dudBitmap);
	if (srcBitmap == NULL) goto bail;
	
	// determine depth we're gonna copy the source bits into
	srcBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	srcBMI->bmiHeader.biBitCount = 0;
	srcBMI->bmiHeader.biClrImportant = 0;
	if (::GetDIBits(inDC, srcBitmap, 0, 0, NULL, srcBMI, DIB_RGB_COLORS) == 0) goto bail;
	srcBMI->bmiHeader.biBitCount = (srcBMI->bmiHeader.biBitCount <= 8) ? 8 : 32;
	
	// if applicable, grab the color table that the source image is using
	if (srcBMI->bmiHeader.biBitCount == 8)
	{
		// don't check the result here because it appears to fail but works anyway
		::GetDIBits(inDC, srcBitmap, 0, 0, NULL, srcBMI, DIB_RGB_COLORS);
	}
	else
	{
		srcBMI->bmiHeader.biClrUsed = srcBMI->bmiHeader.biClrImportant = 0;
	}
	
	// release the source bitmap
	::SelectObject(inDC, srcBitmap);
	srcBitmap = NULL;
	::DeleteObject(dudBitmap);
	dudBitmap = NULL;

	// create a temporary bitmap to copy the source bits into
	srcBMI->bmiHeader.biWidth = inWidth;
	srcBMI->bmiHeader.biHeight = -inHeight;
	srcBMI->bmiHeader.biPlanes = 1;
	srcBMI->bmiHeader.biCompression = BI_RGB;
	srcBMI->bmiHeader.biSizeImage = 0;
	srcBMI->bmiHeader.biXPelsPerMeter = srcBMI->bmiHeader.biYPelsPerMeter = 0;
	tmpBitmap = ::CreateDIBSection(inDC, srcBMI, DIB_RGB_COLORS, &srcBitmapBits, NULL, 0);
	if (tmpBitmap == NULL) goto bail;
	
	// create a temporary DC to select our temporary bitmap into so we can use BitBlt (f-ing piece of shit windoze!)
	tmpDC = ::CreateCompatibleDC(_gDummyImage.dc);
	if (tmpDC == NULL) goto bail;
	
	// select temp bitmap into temp DC, copy source bits into temp DC, kill temp DC
	::SelectObject(tmpDC, tmpBitmap);
	::BitBlt(tmpDC, 0, 0, inWidth, inHeight, inDC, inPt.x, inPt.y, SRCCOPY);
	::DeleteDC(tmpDC);
	tmpDC = NULL;

	// create a 1-bit bitmap that we'll generate the mask into
	mskBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	mskBMI->bmiHeader.biWidth = inWidth;
	mskBMI->bmiHeader.biHeight = -inHeight;
	mskBMI->bmiHeader.biPlanes = 1;
	mskBMI->bmiHeader.biBitCount = 1;
	mskBMI->bmiHeader.biCompression = BI_RGB;
	mskBMI->bmiHeader.biSizeImage = 0;
	mskBMI->bmiHeader.biXPelsPerMeter = mskBMI->bmiHeader.biYPelsPerMeter = 0;
	mskBMI->bmiHeader.biClrUsed = mskBMI->bmiHeader.biClrImportant = 0;
	mskBMI->bmiColors[0].rgbBlue = mskBMI->bmiColors[0].rgbGreen = mskBMI->bmiColors[0].rgbRed = mskBMI->bmiColors[0].rgbReserved = 0;
	mskBMI->bmiColors[1].rgbBlue = mskBMI->bmiColors[1].rgbGreen = mskBMI->bmiColors[1].rgbRed = 0xFF;
	mskBMI->bmiColors[1].rgbReserved = 0;
	mskBitmap = ::CreateDIBSection(NULL, mskBMI, DIB_RGB_COLORS, &mskBitmapBits, NULL, 0);
	if (mskBitmap == NULL) goto bail;
	
	// convert inTransCol (which is in RGBA format regardless of endianness) to icky windoze BGRA
	inTransCol = _GRSwapRGBAandBGRA(inTransCol);
	
	// generate the mask
	if (srcBMI->bmiHeader.biBitCount == 8)
	{
		Uint32 colorCount = srcBMI->bmiHeader.biClrUsed;
		if (colorCount == 0 || colorCount > 256) colorCount = 256;
		
		_GRTrans8ToMask(inTransCol, mskBitmapBits, RoundUp4(inWidth >> 3), srcBitmapBits, RoundUp4(inWidth), inWidth, inHeight, (Uint32 *)srcBMI->bmiColors, colorCount);
	}
	else
	{
		_GRTrans32ToMask(inTransCol, mskBitmapBits, RoundUp4(inWidth >> 3), srcBitmapBits, inWidth << 2, inWidth, inHeight);
	}
	::DeleteObject(tmpBitmap);
	
	// all done (gawd that was a frigging nightmare)
	return mskBitmap;
	
	// clean up and bail out
	bail:
	if (srcBitmap) ::SelectObject(inDC, srcBitmap);
	if (dudBitmap) ::DeleteObject(dudBitmap);
	if (mskBitmap) ::DeleteObject(mskBitmap);
	if (tmpBitmap) ::DeleteObject(tmpBitmap);
	if (tmpDC) ::DeleteDC(tmpDC);
	return nil;
}

static void _GRConvertBGRAandRGBA(void *ioData, Uint32 inCount)
{
	Uint8 *p = (Uint8 *)ioData;
	
	while (inCount--)
	{
	    Uint8 tmp = p[0];
	    p[0] = p[2];
	    p[2] = tmp;
		p += 4;
	}
}

static void _GRCopyRGBAandBGRA(const void *inSrc, void *outDst, Uint32 inCount)
{
	Uint8 *sp = (Uint8 *)inSrc;
	Uint8 *dp = (Uint8 *)outDst;
	
	while (inCount--)
	{
		dp[0] = sp[2];
		dp[1] = sp[1];
		dp[2] = sp[0];
		dp[3] = sp[3];
		sp += 4;
		dp += 4;
	}
}

void UGraphics_CopyPixelsMasked(TImage inDest, const SPoint& inDestPt, TImage inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, TImage inTransMask, Uint32 /*inOptions*/)
{
	Require(inDest && inSource && inTransMask);

	if (((SImage *)inSource)->width != ((SImage *)inTransMask)->width || ((SImage *)inSource)->height != ((SImage *)inTransMask)->height)
		return;
	
	HDC srcDC = ((SImage *)inSource)->dc;
	HDC dstDC = ((SImage *)inDest)->dc;
	HDC maskDC = ((SImage *)inTransMask)->dc;

	// validate the copy rects
	SPoint org;
	SRect dstBnd, srcBnd;
	SPoint theDestPt = inDestPt;
	SPoint theSourcePt = inSourcePt;
	::GetViewportOrgEx(dstDC, (LPPOINT)&org);
	dstBnd.Set(-org.x, -org.y, -org.x + ((SImage *)inDest)->width, -org.y + ((SImage *)inDest)->height);
	::GetViewportOrgEx(srcDC, (LPPOINT)&org);
	srcBnd.Set(-org.x, -org.y, -org.x + ((SImage *)inSource)->width, -org.y + ((SImage *)inSource)->height);
	if (!UGraphics::ValidateCopyRects(dstBnd, srcBnd, theDestPt, theSourcePt, inWidth, inHeight))
		return;
	
	HDC tmpDC = ::CreateCompatibleDC(srcDC);
	if (tmpDC == NULL)
		return;
		
	HBITMAP tmpBMP = ::CreateCompatibleBitmap(srcDC, inWidth, inHeight);
	if (tmpBMP == NULL)
	{
		::DeleteDC(tmpDC);
		return;	
	}
	
	HBITMAP tmpMASK = ::CreateCompatibleBitmap(maskDC, inWidth, inHeight);
	if (tmpMASK == NULL)
	{
		::DeleteDC(tmpDC);
		::DeleteObject(tmpBMP);
		return;
	}

	// copy source bitmap into temporary source bitmap
	HGDIOBJ hObj = ::SelectObject(tmpDC, tmpBMP);
	::BitBlt(tmpDC, 0, 0, inWidth, inHeight, srcDC, theSourcePt.x, theSourcePt.y, SRCCOPY);
	::SelectObject(tmpDC, tmpMASK);
	::BitBlt(tmpDC, 0, 0, inWidth, inHeight, maskDC, theSourcePt.x, theSourcePt.y, SRCCOPY);
	::SelectObject(tmpDC, hObj);
	::DeleteDC(tmpDC);
	
	HIMAGELIST hImageList = ImageList_Create(inWidth, inHeight, ILC_COLORDDB | ILC_MASK, 1, 1);
	if (hImageList == NULL)
	{
		::DeleteObject(tmpBMP);
		::DeleteObject(tmpMASK);
		return;
	}
   
	int nIndex = ImageList_Add(hImageList, tmpBMP, tmpMASK);
	::DeleteObject(tmpBMP);
	::DeleteObject(tmpMASK);
	
	if (nIndex == -1) 
		return;

	ImageList_Draw(hImageList, nIndex, dstDC, theDestPt.x, theDestPt.y, ILD_TRANSPARENT);

	ImageList_RemoveAll(hImageList);
	ImageList_Destroy(hImageList);
}

void UGraphics_CopyPixelsTrans(TImage inDest, const SPoint& inDestPt, TImage inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inTransCol, Uint32 /*inOptions*/)
{
	Require(inDest && inSource);

	HDC srcDC = ((SImage *)inSource)->dc;
	HDC dstDC = ((SImage *)inDest)->dc;

	// validate the copy rects
	SPoint org;
	SRect dstBnd, srcBnd;
	SPoint theDestPt = inDestPt;
	SPoint theSourcePt = inSourcePt;
	::GetViewportOrgEx(dstDC, (LPPOINT)&org);
	dstBnd.Set(-org.x, -org.y, -org.x + ((SImage *)inDest)->width, -org.y + ((SImage *)inDest)->height);
	::GetViewportOrgEx(srcDC, (LPPOINT)&org);
	srcBnd.Set(-org.x, -org.y, -org.x + ((SImage *)inSource)->width, -org.y + ((SImage *)inSource)->height);
	if (!UGraphics::ValidateCopyRects(dstBnd, srcBnd, theDestPt, theSourcePt, inWidth, inHeight))
		return;
	
	HDC tmpDC = ::CreateCompatibleDC(srcDC);
	if (tmpDC == NULL)
		return;
		
	HBITMAP tmpBMP = ::CreateCompatibleBitmap(srcDC, inWidth, inHeight);
	if (tmpBMP == NULL)
	{
		::DeleteDC(tmpDC);
		return;	
	}
	
	// copy source bitmap into temporary source bitmap
	HGDIOBJ hObj = ::SelectObject(tmpDC, tmpBMP);
	::BitBlt(tmpDC, 0, 0, inWidth, inHeight, srcDC, theSourcePt.x, theSourcePt.y, SRCCOPY);
	::SelectObject(tmpDC, hObj);
	::DeleteDC(tmpDC);
	
	HIMAGELIST hImageList = ImageList_Create(inWidth, inHeight, ILC_COLORDDB | ILC_MASK, 1, 1);
	if (hImageList == NULL)
	{
		::DeleteObject(tmpBMP);
		return;
	}
   
	int nIndex = ImageList_AddMasked(hImageList, tmpBMP, inTransCol);
	::DeleteObject(tmpBMP);
	
	if (nIndex == -1) 
		return;

	ImageList_Draw(hImageList, nIndex, dstDC, theDestPt.x, theDestPt.y, ILD_TRANSPARENT);

	ImageList_RemoveAll(hImageList);
	ImageList_Destroy(hImageList);
}
   
/*
void UGraphics_CopyPixelsTrans(TImage inDest, const SPoint& inDestPt, TImage inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inTransCol, Uint32 inOptions)
{
	HBITMAP dudBitmap, srcBitmap, dstBitmap, srcDIB, dstDIB;
	void *srcBitmapBits, *dstBitmapBits;
	HDC tmpDC, srcDC, dstDC;
	Uint32 srcColorCount, dstColorCount;
	SRect dstBnd, srcBnd;
	SPoint theDestPt, theSourcePt, org;
	Uint8 dstBMIBuf[2048];
	Uint8 srcBMIBuf[2048];
	BITMAPINFO *dstBMI = (BITMAPINFO *)dstBMIBuf;
	BITMAPINFO *srcBMI = (BITMAPINFO *)srcBMIBuf;

	// sanity checks
	Require(inDest && inSource);
	
	// initialize values
	dudBitmap = srcBitmap = dstBitmap = srcDIB = dstDIB = NULL;
	tmpDC = NULL;
	srcDC = ((SImage *)inSource)->dc;
	dstDC = ((SImage *)inDest)->dc;
	
	// validate the copy rects
	theDestPt = inDestPt;
	theSourcePt = inSourcePt;
	::GetViewportOrgEx(dstDC, (LPPOINT)&org);
	dstBnd.Set(-org.x, -org.y, -org.x + ((SImage *)inDest)->width, -org.y + ((SImage *)inDest)->height);
	::GetViewportOrgEx(srcDC, (LPPOINT)&org);
	srcBnd.Set(-org.x, -org.y, -org.x + ((SImage *)inSource)->width, -org.y + ((SImage *)inSource)->height);
	if (!ValidateCopyRects(dstBnd, srcBnd, theDestPt, theSourcePt, inWidth, inHeight))
		return;

	// create dud bitmap
	dudBitmap = ::CreateBitmap(0, 0, 1, 1, NULL);
	if (dudBitmap == NULL) return;
	
	// get source bitmap
	srcBitmap = (HBITMAP)::SelectObject(srcDC, dudBitmap);
	if (srcBitmap == NULL) goto bail;
	
	// determine depth we're gonna copy the source bits into
	srcBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	srcBMI->bmiHeader.biBitCount = 0;
	srcBMI->bmiHeader.biClrImportant = 0;
	if (::GetDIBits(srcDC, srcBitmap, 0, 0, NULL, srcBMI, DIB_RGB_COLORS) == 0) goto bail;
	srcBMI->bmiHeader.biBitCount = (srcBMI->bmiHeader.biBitCount <= 8) ? 8 : 32;

	// if applicable, grab the color table that the source image is using
	if (srcBMI->bmiHeader.biBitCount == 8)
	{
		// don't check the result here because it appears to fail but works anyway
		::GetDIBits(srcDC, srcBitmap, 0, 0, NULL, srcBMI, DIB_RGB_COLORS);
	}
	else
	{
		srcBMI->bmiHeader.biClrUsed = srcBMI->bmiHeader.biClrImportant = 0;
	}
	
	// release the source bitmap
	::SelectObject(srcDC, srcBitmap);
	srcBitmap = NULL;
	
	// get dest bitmap
	dstBitmap = (HBITMAP)::SelectObject(dstDC, dudBitmap);
	if (dstBitmap == NULL) goto bail;
	
	// determine depth we're gonna copy the dest bits into
	dstBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	dstBMI->bmiHeader.biBitCount = 0;
	dstBMI->bmiHeader.biClrImportant = 0;
	if (::GetDIBits(dstDC, dstBitmap, 0, 0, NULL, dstBMI, DIB_RGB_COLORS) == 0) goto bail;
	dstBMI->bmiHeader.biBitCount = (dstBMI->bmiHeader.biBitCount <= 8) ? 8 : 32;

	// if applicable, grab the color table that the dest image is using
	if (dstBMI->bmiHeader.biBitCount == 8)
	{
		// don't check the result here because it appears to fail but works anyway
		::GetDIBits(dstDC, dstBitmap, 0, 0, NULL, dstBMI, DIB_RGB_COLORS);
	}
	else
	{
		dstBMI->bmiHeader.biClrUsed = dstBMI->bmiHeader.biClrImportant = 0;
	}
	
	// release the dest bitmap
	::SelectObject(dstDC, dstBitmap);
	dstBitmap = NULL;

	// don't need dud bitmap anymore
	::DeleteObject(dudBitmap);
	dudBitmap = NULL;

	// create the temporary source bitmap to copy the source bits into
	srcBMI->bmiHeader.biWidth = inWidth;
	srcBMI->bmiHeader.biHeight = -inHeight;
	srcBMI->bmiHeader.biPlanes = 1;
	srcBMI->bmiHeader.biCompression = BI_RGB;
	srcBMI->bmiHeader.biSizeImage = 0;
	srcBMI->bmiHeader.biXPelsPerMeter = srcBMI->bmiHeader.biYPelsPerMeter = 0;
	srcDIB = ::CreateDIBSection(srcDC, srcBMI, DIB_RGB_COLORS, &srcBitmapBits, NULL, 0);
	if (srcDIB == NULL) goto bail;
	
	// create the temporary dest bitmap to copy the dest bits into
	dstBMI->bmiHeader.biWidth = inWidth;
	dstBMI->bmiHeader.biHeight = -inHeight;
	dstBMI->bmiHeader.biPlanes = 1;
	dstBMI->bmiHeader.biCompression = BI_RGB;
	dstBMI->bmiHeader.biSizeImage = 0;
	dstBMI->bmiHeader.biXPelsPerMeter = dstBMI->bmiHeader.biYPelsPerMeter = 0;
	dstDIB = ::CreateDIBSection(dstDC, dstBMI, DIB_RGB_COLORS, &dstBitmapBits, NULL, 0);
	if (dstDIB == NULL) goto bail;
	
	// create a temporary DC to select our temporary bitmaps into so we can use BitBlt (may you rot in hell, billy!)
	tmpDC = ::CreateCompatibleDC(_gDummyImage.dc);
	if (tmpDC == NULL) goto bail;
	
	// copy source bitmap into temporary source bitmap
	::SelectObject(tmpDC, srcDIB);
	::BitBlt(tmpDC, 0, 0, inWidth, inHeight, srcDC, theSourcePt.x, theSourcePt.y, SRCCOPY);
	
	// copy dest bitmap into temporary dest bitmap
	::SelectObject(tmpDC, dstDIB);
	::BitBlt(tmpDC, 0, 0, inWidth, inHeight, dstDC, theDestPt.x, theDestPt.y, SRCCOPY);
	
	// convert inTransCol (which is in RGBA format regardless of endianness) to icky windoze BGRA
	inTransCol = _GRSwapRGBAandBGRA(inTransCol);

	// now that we've finally got access to the source and dest pixel data, we can do our Turbo-Studly(TM) transparent copy
	if (srcBMI->bmiHeader.biBitCount == 8)
	{
		srcColorCount = srcBMI->bmiHeader.biClrUsed;
		if (srcColorCount == 0 || srcColorCount > 256) srcColorCount = 256;

		if (dstBMI->bmiHeader.biBitCount == 8)
		{
			dstColorCount = dstBMI->bmiHeader.biClrUsed;
			if (dstColorCount == 0 || dstColorCount > 256) dstColorCount = 256;
			_GRTransCopy8To8(inTransCol, dstBitmapBits, RoundUp4(inWidth), srcBitmapBits, RoundUp4(inWidth), inWidth, inHeight, (Uint32 *)dstBMI->bmiColors, dstColorCount, (Uint32 *)srcBMI->bmiColors, srcColorCount);
		}
		else
		{
			_GRTransCopy8To32(inTransCol, dstBitmapBits, inWidth << 2, srcBitmapBits, RoundUp4(inWidth), inWidth, inHeight, (Uint32 *)srcBMI->bmiColors, srcColorCount);
		}
	}
	else
	{
		if (dstBMI->bmiHeader.biBitCount == 8)
		{
			dstColorCount = dstBMI->bmiHeader.biClrUsed;
			if (dstColorCount == 0 || dstColorCount > 256) dstColorCount = 256;
			_GRTransCopy32To8(inTransCol, dstBitmapBits, RoundUp4(inWidth), srcBitmapBits, inWidth << 2, inWidth, inHeight, (Uint32 *)dstBMI->bmiColors, dstColorCount);
		}
		else
		{
			_GRTransCopy32To32(inTransCol, dstBitmapBits, inWidth << 2, srcBitmapBits, inWidth << 2, inWidth, inHeight);
		}
	}
	
	// unfortunately, our transparent copy was only between the temporary bitmaps, so now copy to the real dest
	::BitBlt(dstDC, theDestPt.x, theDestPt.y, inWidth, inHeight, tmpDC, 0, 0, SRCCOPY);

	// deleted all the temporary shit we created
	::DeleteDC(tmpDC);
	::DeleteObject(srcDIB);
	::DeleteObject(dstDIB);
	
	// all done (and with only three copies more than necessary grrr)
	return;

	// clean up and bail out
	bail:
	if (srcBitmap) ::SelectObject(srcDC, srcBitmap);
	if (dstBitmap) ::SelectObject(dstDC, dstBitmap);
	if (dudBitmap) ::DeleteObject(dudBitmap);
	if (srcDIB) ::DeleteObject(srcDIB);
	if (dstDIB) ::DeleteObject(dstDIB);
	if (tmpDC) ::DeleteDC(tmpDC);
}
*/

void UGraphics_CopyPixelsTrans(const SPixmap& inDest, const SPoint& inDestPt, TImage inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inTransCol, Uint32 /* inOptions */)
{
	#pragma unused(inDest, inDestPt, inSource, inSourcePt, inWidth, inHeight, inTransCol)
	
	Fail(errorType_Misc, error_Unimplemented);
}

void UGraphics_CopyPixelsTrans(TImage inDest, const SPoint& inDestPt, const SPixmap& inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inTransCol, Uint32 /* inOptions */)
{
	HBITMAP dudBitmap, dstBitmap, dstDIB;
	void *dstBitmapBits;
	HDC tmpDC, dstDC;
	Uint32 dstColorCount;
	SRect dstBnd, srcBnd;
	SPoint theDestPt, theSourcePt, org;
	Uint8 dstBMIBuf[2048];
	BITMAPINFO *dstBMI = (BITMAPINFO *)dstBMIBuf;

	// sanity checks on inDest
	Require(inDest);
	
	// sanity checks on inSource
	if (!UPixmap::IsValid(inSource))
	{
		DebugBreak("UGraphics::CopyPixels - invalid source pixmap");
		Fail(errorType_Misc, error_Param);
	}
	
	// initialize values
	dudBitmap = dstBitmap = dstDIB = NULL;
	tmpDC = NULL;
	dstDC = ((SImage *)inDest)->dc;
	
	// validate the copy rects
	theDestPt = inDestPt;
	theSourcePt = inSourcePt;
	::GetViewportOrgEx(dstDC, (LPPOINT)&org);
	dstBnd.Set(-org.x, -org.y, -org.x + ((SImage *)inDest)->width, -org.y + ((SImage *)inDest)->height);
	srcBnd.Set(0, 0, inSource.width, inSource.height);
	if (!UGraphics::ValidateCopyRects(dstBnd, srcBnd, theDestPt, theSourcePt, inWidth, inHeight))
		return;

	// create dud bitmap
	dudBitmap = ::CreateBitmap(0, 0, 1, 1, NULL);
	if (dudBitmap == NULL) return;
	
	// get dest bitmap
	dstBitmap = (HBITMAP)::SelectObject(dstDC, dudBitmap);
	if (dstBitmap == NULL) goto bail;

	// determine depth we're gonna copy the dest bits into
	dstBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	dstBMI->bmiHeader.biBitCount = 0;
	dstBMI->bmiHeader.biClrImportant = 0;
	if (::GetDIBits(dstDC, dstBitmap, 0, 0, NULL, dstBMI, DIB_RGB_COLORS) == 0) goto bail;
	dstBMI->bmiHeader.biBitCount = (dstBMI->bmiHeader.biBitCount <= 8) ? 8 : 32;
	
	// if applicable, grab the color table that the dest image is using
	if (dstBMI->bmiHeader.biBitCount == 8)
	{
		// don't check the result here because it appears to fail but works anyway
		::GetDIBits(dstDC, dstBitmap, 0, 0, NULL, dstBMI, DIB_RGB_COLORS);
	}
	else
	{
		dstBMI->bmiHeader.biClrUsed = dstBMI->bmiHeader.biClrImportant = 0;
	}

	// release the dest bitmap
	::SelectObject(dstDC, dstBitmap);
	dstBitmap = NULL;

	// don't need dud bitmap anymore
	::DeleteObject(dudBitmap);
	dudBitmap = NULL;
	
	// create the temporary dest bitmap to copy the dest bits into
	dstBMI->bmiHeader.biWidth = inWidth;
	dstBMI->bmiHeader.biHeight = -inHeight;
	dstBMI->bmiHeader.biPlanes = 1;
	dstBMI->bmiHeader.biCompression = BI_RGB;
	dstBMI->bmiHeader.biSizeImage = 0;
	dstBMI->bmiHeader.biXPelsPerMeter = dstBMI->bmiHeader.biYPelsPerMeter = 0;
	dstDIB = ::CreateDIBSection(dstDC, dstBMI, DIB_RGB_COLORS, &dstBitmapBits, NULL, 0);
	if (dstDIB == NULL) goto bail;
	
	// create a temporary DC to select our temporary bitmaps into so we can use BitBlt (may you rot in hell, billy!)
	tmpDC = ::CreateCompatibleDC(dstDC);
	if (tmpDC == NULL) goto bail;

	// copy dest bitmap into temporary dest bitmap
	::SelectObject(tmpDC, dstDIB);
	::BitBlt(tmpDC, 0, 0, inWidth, inHeight, dstDC, theDestPt.x, theDestPt.y, SRCCOPY);
	
	// now that we've finally got access to the source and dest pixel data, we can do our Turbo-Studly(TM) transparent copy
	Uint8 *srcData = BPTR(inSource.data) + (inSource.rowBytes * theSourcePt.y) + theSourcePt.x;
	switch (inSource.depth)
	{
		case 1:
		case 2:
		case 4:
		case 16:
		case 24:
		case 32:
			DebugBreak("UGraphics::CopyPixelsTrans - only 8-bit source implemented");
			goto bail;
			break;
		case 8:
			if (dstBMI->bmiHeader.biBitCount == 8)
			{
				dstColorCount = dstBMI->bmiHeader.biClrUsed;
				if (dstColorCount == 0 || dstColorCount > 256) dstColorCount = 256;
				
				_GRConvertBGRAandRGBA(dstBMI->bmiColors, dstColorCount);
				_GRTransCopy8To8(inTransCol, dstBitmapBits, RoundUp4(inWidth), srcData, inSource.rowBytes, inWidth, inHeight, (Uint32 *)dstBMI->bmiColors, dstColorCount, inSource.colorTab, inSource.colorCount);
			}
			else
			{
				_GRCopyRGBAandBGRA(inSource.colorTab, dstBMI->bmiColors, inSource.colorCount);
				_GRTransCopy8To32(_GRSwapRGBAandBGRA(inTransCol), dstBitmapBits, inWidth << 2, srcData, inSource.rowBytes, inWidth, inHeight, (Uint32 *)dstBMI->bmiColors, inSource.colorCount);
			}
			break;
		default:
			goto bail;
			break;
	}

	// unfortunately, our transparent copy was only between the temporary bitmaps, so now copy to the real dest
	::BitBlt(dstDC, theDestPt.x, theDestPt.y, inWidth, inHeight, tmpDC, 0, 0, SRCCOPY);

	// deleted all the temporary shit we created
	::DeleteDC(tmpDC);
	::DeleteObject(dstDIB);
	
	// all done (and with only three copies more than necessary grrr)
	return;

	// clean up and bail out
	bail:
	if (dstBitmap) ::SelectObject(dstDC, dstBitmap);
	if (dudBitmap) ::DeleteObject(dudBitmap);
	if (dstDIB) ::DeleteObject(dstDIB);
	if (tmpDC) ::DeleteDC(tmpDC);
}

void UGraphics_StretchPixels(TImage inDest, const SRect& inDestRect, TImage inSource, const SRect& inSourceRect, Uint32 /* inOptions */)
{
	Require(inDest && inSource);
	
	ASSERT(inDestRect.IsValid() && inSourceRect.IsValid());
	
	/*****************************************************************
	
	This should probably use a variation of ValidateCopyRects()
	(one that works with src and dst having different widths/heights)
	
	******************************************************************/
		
	//BOOL result =
	::StretchBlt(((SImage *)inDest)->dc, inDestRect.left, inDestRect.top, inDestRect.GetWidth(), inDestRect.GetHeight(), ((SImage *)inSource)->dc, inSourceRect.left, inSourceRect.top, inSourceRect.GetWidth(), inSourceRect.GetHeight(), SRCCOPY);
	
	/*if (!result)
	{
		DWORD err = ::GetLastError();
		
		DebugBreak("%lu", err);
	}*/
}

void UGraphics::GetImageSize(TImage inImage, Uint32& outWidth, Uint32& outHeight)
{
	// note that GetViewportExtEx() does NOT give you the size of the image
	// and using GetCurrentObject() and GetBitmapDimensionEx() doesn't work either (up yours billy!)
	
	Require(inImage);
	outWidth = ((SImage *)inImage)->width;
	outHeight = ((SImage *)inImage)->height;
}

Uint32 UGraphics::BuildColorTable(TImage inImage, const SRect& inRect, Uint32 inMethod, Uint32 *outColors, Uint32 inMaxColors)
{
	#pragma unused(inImage, inRect, inMethod, outColors, inMaxColors)
	Fail(errorType_Misc, error_Unimplemented);
	return 0;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void UGraphics::GetRawPixels(TImage inImage, const SRect& inRect, void *outData, Uint32 /* inOptions */)
{
	#pragma unused(inRect)

	Require(inImage && outData);
	
	Fail(errorType_Misc, error_Unimplemented);
}

void UGraphics::SetRawPixels(TImage inImage, const SRect& inRect, const void *inData, Uint32 /* inOptions */)
{
	#pragma unused(inRect)

	Require(inImage && inData);
	
	Fail(errorType_Misc, error_Unimplemented);
}

/* -------------------------------------------------------------------------- */
#pragma mark -
bool _StandardDialogBox(Uint8 inActive = 0);

TImage UGraphics::GetDummyImage()
{
	return (TImage)&_gDummyImage;
}

bool UGraphics::UserSelectColor(SColor& ioColor, const Uint8 */* inPrompt */, Uint32 /* inOptions */)
{
	if (_StandardDialogBox()) // if a dialog box is already opened
		return false;

	static DWORD customColors[16];
	CHOOSECOLOR info;
	
	info.lStructSize = sizeof(info);
	info.hwndOwner = NULL;
	info.hInstance = NULL;
	info.rgbResult = RGB(ioColor.red >> 8, ioColor.green >> 8, ioColor.blue >> 8);
	info.lpCustColors = customColors;
	info.Flags = CC_RGBINIT;
	info.lCustData = 0;
	info.lpfnHook = NULL;
	info.lpTemplateName = NULL;
	
	_StandardDialogBox(1);
	if (!::ChooseColor(&info))
	{
		_StandardDialogBox(2);
		return false;
	}	

	_StandardDialogBox(2);

	ioColor.Set(GetRValue(info.rgbResult) * 257, GetGValue(info.rgbResult) * 257, GetBValue(info.rgbResult) * 257);
	return true;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

HDC _ImageToDC(TImage inImage)
{
	return (((SImage *)inImage)->dc);
}

void _SetImageDC(TImage inImage, HDC inDC)
{
	::SetBkMode(inDC, TRANSPARENT);

	(((SImage *)inImage)->dc) = inDC;
}

TImage _NewDCImage(HDC inDC, Uint32 inWidth, Uint32 inHeight)
{
	::SetBkMode(inDC, TRANSPARENT);
	
	SImage *img = (SImage *)UMemory::NewClear(sizeof(SImage));
	
	img->dc = inDC;
	img->width = inWidth;
	img->height = inHeight;
	
	UGraphics::Reset((TImage)img);
	UGraphics::ResetFont((TImage)img);
	
	return (TImage)img;
}

void _NewBitmapImage(TImage inImage)
{
	((SImage *)inImage)->width = ::GetDeviceCaps((((SImage *)inImage)->dc), HORZRES);
	((SImage *)inImage)->height = ::GetDeviceCaps((((SImage *)inImage)->dc), VERTRES);

	// for Letter width:4887, height=6390 - not enough memory to create an bitmap so big
//	HBITMAP hBitmap = ::CreateCompatibleBitmap((((SImage *)inImage)->dc), ((SImage *)inImage)->width, ((SImage *)inImage)->height); // ???
	HBITMAP hBitmap = ::CreateCompatibleBitmap((((SImage *)inImage)->dc), 0, 0); 
	if (hBitmap == NULL) Fail(errorType_Memory, memError_NotEnough);
	::SelectObject((((SImage *)inImage)->dc), hBitmap);

	if (((SImage *)inImage)->bitmap)
		::DeleteObject(((SImage *)inImage)->bitmap);
	
	((SImage *)inImage)->bitmap = hBitmap;
}

// does not dispose the actual DC or anything
void _DisposeDCImage(TImage inImage)
{
	if (inImage)
	{
		HDC dc = (((SImage *)inImage)->dc);
		
		// delete objects
		::DeleteObject(::SelectObject(dc, ::GetStockObject(SYSTEM_FONT)));
		::DeleteObject(::SelectObject(dc, _NULL_BRUSH));
		::DeleteObject(::SelectObject(dc, _NULL_PEN));
	
		UMemory::Dispose((TPtr)inImage);
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void _SetVirtualOrigin(TImage inImage, const SPoint& inVirtualOrigin)
{
	ASSERT(inImage);

	((SImage *)inImage)->virtualOrigin = inVirtualOrigin;
}

void _GetVirtualOrigin(TImage inImage, SPoint& outVirtualOrigin)
{
	ASSERT(inImage);

	outVirtualOrigin = ((SImage *)inImage)->virtualOrigin;
}

void _AddVirtualOrigin(TImage inImage, const SPoint& inVirtualOrigin)
{
	ASSERT(inImage);

	((SImage *)inImage)->virtualOrigin.x += inVirtualOrigin.x;
	((SImage *)inImage)->virtualOrigin.y += inVirtualOrigin.y;
}

void _ResetVirtualOrigin(TImage inImage)
{
	ASSERT(inImage);

	((SImage *)inImage)->virtualOrigin.x = 0;
	((SImage *)inImage)->virtualOrigin.y = 0;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

static int CALLBACK _FontExistsEnumProc(ENUMLOGFONT FAR*, NEWTEXTMETRIC FAR*, int, LPARAM inLParam)
{
	*(Uint32 *)inLParam = 1;
	return 0;
}

bool _FontExists(LPCTSTR inFamily)
{
	Uint32 exists = 0;
	
	EnumFontFamilies(_gDummyImage.dc, inFamily, (FONTENUMPROC)_FontExistsEnumProc, (LPARAM)&exists);
	
	return exists;
}

static Uint32 _FontNameToWinName(const Uint8 *inName, Int8 *outName, Uint32 inBufferSize)
{
	if (inName == kDefaultFont || inName == nil)
		inName = "\pMS Sans Serif";
	else if (inName == kSystemFont)
		inName = "\pMS Sans Serif";
	else if (inName == kFixedFont)
		//inName = _FontExists("Monaco") ? "\pMonaco" : "\pFixedsys";
		//inName = "\pFixedsys";
		inName = "\pCourier New";
	else if (inName == kSansFont)
		inName = "\pArial";
	else if (inName == kSerifFont)
		inName = "\pTimes New Roman";
	
	Uint32 s = inName[0];
	inBufferSize--;
	if (s > inBufferSize) s = inBufferSize;
	::CopyMemory(outName, inName+1, s);
	outName[s] = 0;
	
	return s;
}

TFontDesc UFontDesc::New(const SFontDesc& inInfo)
{
	SFontDescObj *fd = (SFontDescObj *)UMemory::NewClear(sizeof(SFontDescObj));
	
	fd->lf.lfHeight = -inInfo.size;
	
	Uint32 effect = inInfo.effect;
	if (effect & fontEffect_Bold) fd->lf.lfWeight = FW_BOLD;
	if (effect & fontEffect_Italic) fd->lf.lfItalic = TRUE;
	if (effect & fontEffect_Underline) fd->lf.lfUnderline = TRUE;
	if (effect & fontEffect_StrikeOut) fd->lf.lfStrikeOut = TRUE;
	
	_FontNameToWinName(inInfo.name, fd->lf.lfFaceName, LF_FACESIZE);
	
	fd->size = inInfo.size;
	fd->effect = effect;
	fd->align = inInfo.textAlign;
	fd->customVal = inInfo.customVal;
	
	if (inInfo.color) fd->color = *inInfo.color;
	if (inInfo.customColor) fd->customColor = *inInfo.customColor;
	
	return (TFontDesc)fd;
}

TFontDesc UFontDesc::New(const Uint8 *inName, const Uint8 */* inStyle */, Uint32 inSize, Uint32 inEffect)
{
	SFontDescObj *fd = (SFontDescObj *)UMemory::NewClear(sizeof(SFontDescObj));
	
	fd->lf.lfHeight = -inSize;
	
	if (inEffect & fontEffect_Bold) fd->lf.lfWeight = FW_BOLD;
	if (inEffect & fontEffect_Italic) fd->lf.lfItalic = TRUE;
	if (inEffect & fontEffect_Underline) fd->lf.lfUnderline = TRUE;
	if (inEffect & fontEffect_StrikeOut) fd->lf.lfStrikeOut = TRUE;
	
	_FontNameToWinName(inName, fd->lf.lfFaceName, LF_FACESIZE);
	
	fd->size = inSize;
	fd->effect = inEffect;
	fd->align = textAlign_Left;
	
	return (TFontDesc)fd;
}

void UFontDesc::Dispose(TFontDesc inFD)
{
	if (inFD && !FD->locked)
		UMemory::Dispose((TPtr)inFD);
}

TFontDesc UFontDesc::Clone(TFontDesc inFD)
{
	Require(inFD);
	return (TFontDesc)UMemory::New(inFD, sizeof(SFontDescObj));
}

// can't dispose or modify if locked
void UFontDesc::SetLock(TFontDesc inFD, bool inLock)
{
	Require(inFD);
	FD->locked = (inLock != 0);
}

bool UFontDesc::IsLocked(TFontDesc inFD)
{
	return inFD && FD->locked;
}

void UFontDesc::SetDefault(const SFontDesc& inInfo)
{
	SFontDescObj *fd = (SFontDescObj *)UFontDesc::New(inInfo);
	_gDefaultFontDesc = *fd;
	UMemory::Dispose((TPtr)fd);
}

void UFontDesc::SetFontName(TFontDesc inFD, const Uint8 *inName, const Uint8 */* inStyle */)
{
	Require(inFD);
	if (FD->locked) Fail(errorType_Misc, error_Locked);
	_FontNameToWinName(inName, FD->lf.lfFaceName, LF_FACESIZE);
}

// returns true if changed
bool UFontDesc::SetFontSize(TFontDesc inFD, Uint32 inSize)
{
	Require(inFD);
	if (FD->locked) Fail(errorType_Misc, error_Locked);

	if (inSize != FD->size)
	{
		FD->lf.lfHeight = -inSize;
		FD->size = inSize;
		return true;
	}
	
	return false;
}

Uint32 UFontDesc::GetFontSize(TFontDesc inFD)
{
	return inFD ? FD->size : _gDefaultFontDesc.size;
}

void UFontDesc::SetFontEffect(TFontDesc inFD, Uint32 inFlags)
{
	Require(inFD);
	if (FD->locked) Fail(errorType_Misc, error_Locked);
	
	FD->lf.lfWeight = (inFlags & fontEffect_Bold) ? FW_BOLD : 0;
	FD->lf.lfItalic = (inFlags & fontEffect_Italic) ? TRUE : FALSE;
	FD->lf.lfUnderline = (inFlags & fontEffect_Underline) ? TRUE : FALSE;
	FD->lf.lfStrikeOut = (inFlags & fontEffect_StrikeOut) ? TRUE : FALSE;
	
	FD->effect = inFlags;
}

Uint32 UFontDesc::GetFontEffect(TFontDesc inFD)
{
	return inFD ? FD->effect : _gDefaultFontDesc.effect;
}

void UFontDesc::SetAlign(TFontDesc inFD, Uint32 inAlign)
{
	Require(inFD);
	if (FD->locked) Fail(errorType_Misc, error_Locked);
	FD->align = inAlign;
}

Uint32 UFontDesc::GetAlign(TFontDesc inFD)
{
	return inFD ? FD->align : _gDefaultFontDesc.align;
}

void UFontDesc::SetColor(TFontDesc inFD, const SColor& inColor)
{
	Require(inFD);
	if (FD->locked) Fail(errorType_Misc, error_Locked);
	FD->color = inColor;
}

void UFontDesc::GetColor(TFontDesc inFD, SColor& outColor)
{
	outColor = inFD ? FD->color : _gDefaultFontDesc.color;
}

void UFontDesc::SetCustomColor(TFontDesc inFD, const SColor& inColor)
{
	Require(inFD);
	if (FD->locked) Fail(errorType_Misc, error_Locked);
	FD->customColor = inColor;
}

void UFontDesc::GetCustomColor(TFontDesc inFD, SColor& outColor)
{
	outColor = inFD ? FD->customColor : _gDefaultFontDesc.customColor;
}

void UFontDesc::SetCustomValue(TFontDesc inFD, Uint32 inVal)
{
	Require(inFD);
	if (FD->locked) Fail(errorType_Misc, error_Locked);
	FD->customVal = inVal;
}

Uint32 UFontDesc::GetCustomValue(TFontDesc inFD)
{
	return inFD ? FD->customVal : _gDefaultFontDesc.customVal;
}

TFontDesc UFontDesc::Unflatten(const void *inData, Uint32 inDataSize)
{
	Uint32 *lp;
	Uint8 *name;
	Uint32 effect;
	Uint16 stdFont;

	if (inDataSize < 67) Fail(errorType_Misc, error_Corrupt);
	
	if (*(Uint16 *)inData != 'FD') Fail(errorType_Misc, error_FormatUnknown);
	if (FB(*(Uint16 *)(BPTR(inData)+2)) != 1) Fail(errorType_Misc, error_VersionUnknown);
	
	SFontDescObj *fd = (SFontDescObj *)UMemory::NewClear(sizeof(SFontDescObj));
	
	try
	{
		lp = (Uint32 *)inData;
		
		fd->size = FB(lp[1]);
		fd->effect = effect = FB(lp[2]);
		fd->align = FB(lp[6]);
		fd->customVal = FB(lp[7]);

		fd->color = *(SColor *)(lp+12);
		fd->customColor = *(SColor *)(lp+14);
		
		stdFont = *((Uint8 *)inData + 64);
		name = BPTR(inData) + 65;
	
		if (stdFont)
		{
			switch (stdFont)
			{
				case 1:
					name = kDefaultFont;
					break;
				case 2:
					name = kSystemFont;
					break;
				case 3:
					name = kFixedFont;
					break;
				case 4:
					name = kSansFont;
					break;
				case 5:
					name = kSerifFont;
					break;
				default:
					name = kDefaultFont;
					break;
			}
		}
		
		fd->lf.lfHeight = -fd->size;
		
		if (effect & fontEffect_Bold) fd->lf.lfWeight = FW_BOLD;
		if (effect & fontEffect_Italic) fd->lf.lfItalic = TRUE;
		if (effect & fontEffect_Underline) fd->lf.lfUnderline = TRUE;
		if (effect & fontEffect_StrikeOut) fd->lf.lfStrikeOut = TRUE;
		
		_FontNameToWinName(name, fd->lf.lfFaceName, LF_FACESIZE);
	}
	catch(...)
	{
		UMemory::Dispose((TPtr)fd);
		throw;
	}
	
	return (TFontDesc)fd;
}

static int CALLBACK _MyEnumFontFamProc(ENUMLOGFONT *inLF, NEWTEXTMETRIC */* inTM */, int /* inType */, LPARAM inRef)
{
	char *faceName = inLF->elfLogFont.lfFaceName;
	TEnumFontNamesProc proc = (TEnumFontNamesProc)*(Uint32 *)inRef;
	Uint8 str[256];
	
	str[0] = strlen(faceName);
	::CopyMemory(str+1, faceName, str[0]);
	
	try {
		proc(str, 0, 0, (void *)(((Uint32 *)inRef)[1]));
	} catch(...) {}		// don't throw exceptions over win callbacks
	
	return 1;
}

void UFontDesc::EnumFontNames(TEnumFontNamesProc inProc, void *inRef)
{
	Uint32 data[2] = { (Uint32)inProc, (Uint32)inRef };
		
	::EnumFontFamilies(_gDummyImage.dc, NULL, (FONTENUMPROC)_MyEnumFontFamProc, (LPARAM)data);
}




#endif
