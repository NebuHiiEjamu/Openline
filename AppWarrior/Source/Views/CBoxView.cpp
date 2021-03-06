/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "CBoxView.h"

#pragma options align=packed

typedef struct {
	Int16 rsvd;
	Int16 pattern;
	Int16 titleStyle;
	Int16 style;
	Int16 script;
	Uint16 titleLen;
	Uint8 titleData[];
} SBoxView;

#pragma options align=reset

/* -------------------------------------------------------------------------- */

CBoxView::CBoxView(CViewHandler *inHandler, const SRect& inBounds)
	: CView(inHandler, inBounds)
{
	mStyle = boxStyle_Etched;
	mFont = nil;
	mPattern = nil;
	mTitle = nil;
}

CBoxView::CBoxView(CViewHandler *inHandler, const SRect& inBounds, Uint16 inStyle, const Uint8 *inTitle, TFontDesc inFont)
	: CView(inHandler, inBounds)
{
	mStyle = inStyle;
	mFont = inFont;
	mPattern = nil;
	mTitle = inTitle ? UMemory::NewHandle(inTitle+1, inTitle[0]) : nil;
}

CBoxView::~CBoxView()
{
	delete mFont;
	UMemory::Dispose(mTitle);
#if MACINTOSH
	UGraphics::DisposePattern(mPattern);
#endif
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void CBoxView::SetTitle(const void *inText, Uint32 inSize)
{
	if (mTitle == nil)
		mTitle = UMemory::NewHandle(inText, inSize);
	else
		UMemory::Set(mTitle, inText, inSize);
	
	Refresh();
}

// takes ownership of the TFontDesc
void CBoxView::SetFont(TFontDesc inFont)
{
	delete mFont;
	mFont = inFont;
	Refresh();
}

void CBoxView::SetStyle(Uint16 inStyle)
{
	if (mStyle != inStyle)
	{
		mStyle = inStyle;
		Refresh();
	}
}

void CBoxView::SetPattern(Int16 inID)
{
#if MACINTOSH
	mPattern = UGraphics::GetResPattern(inID);
#else
	#pragma unused(inID)
#endif
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void _FillRectWithPattern(TImage inImage, const SRect& inBounds, TPattern inPat);

void CBoxView::Draw(TImage inImage, const SRect& /* inUpdateRect */, Uint32 /* inDepth */)
{
	switch (mStyle)
	{
		case boxStyle_Sunken:
			UUserInterface::DrawSunkenBox(inImage, mBounds);
			break;
		case boxStyle_Raised:
			UUserInterface::DrawRaisedBox(inImage, mBounds);
			break;
		case boxStyle_Bar:
			UUserInterface::DrawBarBox(inImage, mBounds);
			break;
		case boxStyle_Pattern:
#if MACINTOSH
			_FillRectWithPattern(inImage, mBounds, mPattern);
#endif
			UUserInterface::DrawRaisedBox(inImage, mBounds);
			break;
			
		default:
			if (mTitle == nil)
			{
				UUserInterface::DrawEtchedBox(inImage, mBounds);
			}
			else
			{
				inImage->SetFont(mFont);
				void *p;
				StHandleLocker lock(mTitle, p);
				UUserInterface::DrawEtchedBox(inImage, mBounds, p, UMemory::GetSize(mTitle));
			}
			break;
	}
}

