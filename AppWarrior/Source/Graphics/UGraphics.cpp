/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

/*
LINE DRAWING NOTES

- doesn't include ending point regardless of line size
- changing line size doesn't change length of line
- if line size is greater than 1, line is drawn centered around the points
- if line size is even, the line hangs more to the top/left
- lines are drawn squared off, ie lines are perfectly rectangular

If the lines aren't drawn like this... well they should be :)

For rectangles and ovals, the shape should fit entirely within the supplied
rect.  ie if the line size is thick, it should go inside the rect, not
centered around it.  If the width or height of the rect is smaller than
the line size, care must be taken to ensure that the shape does not draw
outside the rect.

For framed polygons, the lines should be centered around the points (it
doesn't make any sense otherwise).  For filled polygons, the shape should
go completely inside the points regardless of whether or not the line
size is thick (ie filled polygons draw as if the line size is 1).
*/

#include "UGraphics.h"

void UGraphics_DrawPixel(TImage inImage, const SPoint& inPixel);
void UGraphics_DrawLine(TImage inImage, const SLine& inLine, Uint32 inOptions);
void UGraphics_DrawLines(TImage inImage, const SLine *inLines, Uint32 inCount, Uint32 inOptions);
void UGraphics_DrawRect(TImage inImage, const SRect& inRect, Uint32 inFill);
void UGraphics_DrawOval(TImage inImage, const SRect& inRect, Uint32 inFill);
void UGraphics_DrawRoundRect(TImage inImage, const SRoundRect& inRect, Uint32 inFill);
void UGraphics_DrawPolygon(TImage inImage, const SPoint *inPointList, Uint32 inPointCount, Uint32 inFill, Uint32 inOptions);
void UGraphics_DrawRegion(TImage inImage, TRegion inRgn, Uint32 inFill);
void UGraphics_FillRect(TImage inImage, const SRect& inRect);
void UGraphics_FrameRect(TImage inImage, const SRect& inRect);
void UGraphics_SetPixel(TImage inImage, const SPoint& inPixel, const SColor& inColor);
void UGraphics_SetPixels(TImage inImage, const SPoint *inPointList, Uint32 inPointCount, const SColor& inColor);
void UGraphics_GetPixel(TImage inImage, const SPoint& inPixel, SColor& outColor);
void UGraphics_FloodFill(TImage inImage, const SPoint& inPt);

void UGraphics_DrawText(TImage inImage, const SRect& inBounds, const void *inText, Uint32 inTextSize, Uint32 inEncoding, Uint32 inAlignFlags);
void UGraphics_DrawTruncText(TImage inImage, const SRect& inBounds, const void *inText, Uint32 inTextSize, Uint32 inEncoding, Uint32 inAlignFlags);
void UGraphics_DrawTextBox(TImage inImage, const SRect& inBounds, const SRect& inUpdateRect, const void *inText, Uint32 inTextSize, Uint32 inEncoding, Uint32 inAlign);
Uint32 UGraphics_GetTextBoxHeight(TImage inImage, const SRect& inBounds, const void *inText, Uint32 inTextSize, Uint32 inEncoding, Uint32 inAlign);
void UGraphics_DrawTextLines(TImage inImage, const SRect& inBounds, const void *inText, Uint32 inEncoding, const Uint32 *inLineOffsets, Uint32 inStartLine, Uint32 inEndLine, Uint32 inLineHeight, Uint32 inAlign);

void UGraphics_CopyPixels(TImage inDest, const SPoint& inDestPt, TImage inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inOptions);
void UGraphics_CopyPixels(const SPixmap& inDest, const SPoint& inDestPt, TImage inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inOptions);
void UGraphics_CopyPixels(TImage inDest, const SPoint& inDestPt, const SPixmap& inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inOptions);
void UGraphics_CopyPixelsMasked(TImage inDest, const SPoint& inDestPt, TImage inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, TImage inTransMask, Uint32 inOptions);
void UGraphics_CopyPixelsTrans(TImage inDest, const SPoint& inDestPt, TImage inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inTransCol, Uint32 inOptions);
void UGraphics_CopyPixelsTrans(const SPixmap& inDest, const SPoint& inDestPt, TImage inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inTransCol, Uint32 inOptions);
void UGraphics_CopyPixelsTrans(TImage inDest, const SPoint& inDestPt, const SPixmap& inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inTransCol, Uint32 inOptions);
void UGraphics_StretchPixels(TImage inDest, const SRect& inDestRect, TImage inSource, const SRect& inSourceRect, Uint32 inOptions);

void _SetVirtualOrigin(TImage inImage, const SPoint& inVirtualOrigin);
void _GetVirtualOrigin(TImage inImage, SPoint& outVirtualOrigin);
void _ResetVirtualOrigin(TImage inImage);

/* -------------------------------------------------------------------------- */

void UGraphics::DrawPixel(TImage inImage, const SPoint& inPixel)
{
	// get virtual origin
	SPoint stVirtualOrigin;
	_GetVirtualOrigin(inImage, stVirtualOrigin);

	if (stVirtualOrigin.IsNull())
		UGraphics_DrawPixel(inImage, inPixel);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inImage);

		// recalculate point
		SPoint stPixel = inPixel;
		stPixel.MoveBack(stVirtualOrigin.x, stVirtualOrigin.y);
	
		// draw
		UGraphics_DrawPixel(inImage, stPixel);

		// restore virtual origin
		_SetVirtualOrigin(inImage, stVirtualOrigin);
	}
}

void UGraphics::DrawLine(TImage inImage, const SLine& inLine, Uint32 inOptions)
{
	// get virtual origin
	SPoint stVirtualOrigin;
	_GetVirtualOrigin(inImage, stVirtualOrigin);

	if (stVirtualOrigin.IsNull())
		UGraphics_DrawLine(inImage, inLine, inOptions);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inImage);

		// recalculate line
		SLine stLine = inLine;
		stLine.start.MoveBack(stVirtualOrigin.x, stVirtualOrigin.y);
		stLine.end.MoveBack(stVirtualOrigin.x, stVirtualOrigin.y);
	
		// draw
		UGraphics_DrawLine(inImage, stLine, inOptions);

		// restore virtual origin
		_SetVirtualOrigin(inImage, stVirtualOrigin);
	}
}

void UGraphics::DrawLines(TImage inImage, const SLine *inLines, Uint32 inCount, Uint32 inOptions)
{
	if (!inLines)
		return;

	// get virtual origin
	SPoint stVirtualOrigin;
	_GetVirtualOrigin(inImage, stVirtualOrigin);

	if (stVirtualOrigin.IsNull())
		UGraphics_DrawLines(inImage, inLines, inCount, inOptions);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inImage);
	
		// allocate memory
		SLine *pLines = (SLine *)UMemory::New(inLines, inCount * sizeof(SLine));

		// recalculate lines
		for (Uint32 i = 0; i < inCount; i++)
		{
			pLines[i].start.MoveBack(stVirtualOrigin.x, stVirtualOrigin.y);
			pLines[i].end.MoveBack(stVirtualOrigin.x, stVirtualOrigin.y);
		}
	
		// draw
		UGraphics_DrawLines(inImage, pLines, inCount, inOptions);
	
		// dispose memory
		UMemory::Dispose((TPtr)pLines);
	
		// restore virtual origin
		_SetVirtualOrigin(inImage, stVirtualOrigin);
	}
}

void UGraphics::DrawRect(TImage inImage, const SRect& inRect, Uint32 inFill)
{
	// get virtual origin
	SPoint stVirtualOrigin;
	_GetVirtualOrigin(inImage, stVirtualOrigin);

	if (stVirtualOrigin.IsNull())
		UGraphics_DrawRect(inImage, inRect, inFill);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inImage);

		// recalculate rect
		SRect stRect = inRect;
		stRect.MoveBack(stVirtualOrigin.x, stVirtualOrigin.y);
	
		// draw
		UGraphics_DrawRect(inImage, stRect, inFill);

		// restore virtual origin
		_SetVirtualOrigin(inImage, stVirtualOrigin);
	}
}

void UGraphics::DrawOval(TImage inImage, const SRect& inRect, Uint32 inFill)
{
	// get virtual origin
	SPoint stVirtualOrigin;
	_GetVirtualOrigin(inImage, stVirtualOrigin);

	if (stVirtualOrigin.IsNull())
		UGraphics_DrawOval(inImage, inRect, inFill);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inImage);

		// recalculate rect
		SRect stRect = inRect;
		stRect.MoveBack(stVirtualOrigin.x, stVirtualOrigin.y);
	
		// draw
		UGraphics_DrawOval(inImage, stRect, inFill);

		// restore virtual origin
		_SetVirtualOrigin(inImage, stVirtualOrigin);
	}
}

void UGraphics::DrawRoundRect(TImage inImage, const SRoundRect& inRect, Uint32 inFill)
{
	// get virtual origin
	SPoint stVirtualOrigin;
	_GetVirtualOrigin(inImage, stVirtualOrigin);

	if (stVirtualOrigin.IsNull())
		UGraphics_DrawRoundRect(inImage, inRect, inFill);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inImage);

		// recalculate rect
		SRoundRect stRect = inRect;
		stRect.MoveBack(stVirtualOrigin.x, stVirtualOrigin.y);
	
		// draw
		UGraphics_DrawRoundRect(inImage, stRect, inFill);

		// restore virtual origin
		_SetVirtualOrigin(inImage, stVirtualOrigin);
	}
}

void UGraphics::DrawPolygon(TImage inImage, const SPoint *inPointList, Uint32 inPointCount, Uint32 inFill, Uint32 inOptions)
{
	if (!inPointList)
		return;
	
	// get virtual origin
	SPoint stVirtualOrigin;
	_GetVirtualOrigin(inImage, stVirtualOrigin);

	if (stVirtualOrigin.IsNull())
		UGraphics_DrawPolygon(inImage, inPointList, inPointCount, inFill, inOptions);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inImage);
	
		// allocate memory
		SPoint *pPointList = (SPoint *)UMemory::New(inPointList, inPointCount * sizeof(SPoint));

		// recalculate points
		for (Uint32 i = 0; i < inPointCount; i++)
			pPointList[i].MoveBack(stVirtualOrigin.x, stVirtualOrigin.y);
	
		// draw
		UGraphics_DrawPolygon(inImage, pPointList, inPointCount, inFill, inOptions);
	
		// dispose memory
		UMemory::Dispose((TPtr)pPointList);
	
		// restore virtual origin
		_SetVirtualOrigin(inImage, stVirtualOrigin);
	}
}

void UGraphics::DrawRegion(TImage inImage, TRegion inRgn, Uint32 inFill)
{
	if (!inRgn)
		return;

	// get virtual origin
	SPoint stVirtualOrigin;
	_GetVirtualOrigin(inImage, stVirtualOrigin);

	if (stVirtualOrigin.IsNull())
		UGraphics_DrawRegion(inImage, inRgn, inFill);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inImage);

		// get bounds
		SRect stBounds;
		inRgn->GetBounds(stBounds);

		// recalculate bounds
		stBounds.MoveBack(stVirtualOrigin.x, stVirtualOrigin.y);
		inRgn->SetBounds(stBounds);
	
		// draw
		UGraphics_DrawRegion(inImage, inRgn, inFill);

		// restore bounds
		stBounds.Move(stVirtualOrigin.x, stVirtualOrigin.y);
		inRgn->SetBounds(stBounds);

		// restore virtual origin
		_SetVirtualOrigin(inImage, stVirtualOrigin);
	}
}

void UGraphics::FillRect(TImage inImage, const SRect& inRect)
{
	// get virtual origin
	SPoint stVirtualOrigin;
	_GetVirtualOrigin(inImage, stVirtualOrigin);

	if (stVirtualOrigin.IsNull())
		UGraphics_FillRect(inImage, inRect);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inImage);

		// recalculate rect
		SRect stRect = inRect;
		stRect.MoveBack(stVirtualOrigin.x, stVirtualOrigin.y);
	
		// draw
		UGraphics_FillRect(inImage, stRect);

		// restore virtual origin
		_SetVirtualOrigin(inImage, stVirtualOrigin);
	}
}

void UGraphics::FrameRect(TImage inImage, const SRect& inRect)
{
	// get virtual origin
	SPoint stVirtualOrigin;
	_GetVirtualOrigin(inImage, stVirtualOrigin);

	if (stVirtualOrigin.IsNull())
		UGraphics_FrameRect(inImage, inRect);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inImage);

		// recalculate rect
		SRect stRect = inRect;
		stRect.MoveBack(stVirtualOrigin.x, stVirtualOrigin.y);
	
		// draw
		UGraphics_FrameRect(inImage, stRect);

		// restore virtual origin
		_SetVirtualOrigin(inImage, stVirtualOrigin);
	}
}

void UGraphics::SetPixel(TImage inImage, const SPoint& inPixel, const SColor& inColor)
{
	// get virtual origin
	SPoint stVirtualOrigin;
	_GetVirtualOrigin(inImage, stVirtualOrigin);

	if (stVirtualOrigin.IsNull())
		UGraphics_SetPixel(inImage, inPixel, inColor);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inImage);

		// recalculate point
		SPoint stPixel = inPixel;
		stPixel.MoveBack(stVirtualOrigin.x, stVirtualOrigin.y);
	
		// get
		UGraphics_SetPixel(inImage, stPixel, inColor);

		// restore virtual origin
		_SetVirtualOrigin(inImage, stVirtualOrigin);
	}
}

void UGraphics::SetPixels(TImage inImage, const SPoint *inPointList, Uint32 inPointCount, const SColor& inColor)
{
	if (!inPointList)
		return;
	
	// get virtual origin
	SPoint stVirtualOrigin;
	_GetVirtualOrigin(inImage, stVirtualOrigin);

	if (stVirtualOrigin.IsNull())
		UGraphics_SetPixels(inImage, inPointList, inPointCount, inColor);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inImage);
	
		// allocate memory
		SPoint *pPointList = (SPoint *)UMemory::New(inPointList, inPointCount * sizeof(SPoint));

		// recalculate points
		for (Uint32 i = 0; i < inPointCount; i++)
			pPointList[i].MoveBack(stVirtualOrigin.x, stVirtualOrigin.y);
	
		// set
		UGraphics_SetPixels(inImage, pPointList, inPointCount, inColor);
	
		// dispose memory
		UMemory::Dispose((TPtr)pPointList);
	
		// restore virtual origin
		_SetVirtualOrigin(inImage, stVirtualOrigin);
	}
}

void UGraphics::GetPixel(TImage inImage, const SPoint& inPixel, SColor& outColor)
{
	// get virtual origin
	SPoint stVirtualOrigin;
	_GetVirtualOrigin(inImage, stVirtualOrigin);

	if (stVirtualOrigin.IsNull())
		UGraphics_GetPixel(inImage, inPixel, outColor);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inImage);

		// recalculate point
		SPoint stPixel = inPixel;
		stPixel.MoveBack(stVirtualOrigin.x, stVirtualOrigin.y);
	
		// get
		UGraphics_GetPixel(inImage, stPixel, outColor);

		// restore virtual origin
		_SetVirtualOrigin(inImage, stVirtualOrigin);
	}
}

void UGraphics::FloodFill(TImage inImage, const SPoint& inPt)
{
	// get virtual origin
	SPoint stVirtualOrigin;
	_GetVirtualOrigin(inImage, stVirtualOrigin);

	if (stVirtualOrigin.IsNull())
		UGraphics_FloodFill(inImage, inPt);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inImage);

		// recalculate point
		SPoint stPt = inPt;
		stPt.MoveBack(stVirtualOrigin.x, stVirtualOrigin.y);
	
		// draw
		UGraphics_FloodFill(inImage, stPt);

		// restore virtual origin
		_SetVirtualOrigin(inImage, stVirtualOrigin);
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void UGraphics::DrawText(TImage inImage, const SRect& inBounds, const void *inText, Uint32 inTextSize, Uint32 inEncoding, Uint32 inAlignFlags)
{
	if (!inText || !inTextSize)
		return;
	
	// get virtual origin
	SPoint stVirtualOrigin;
	_GetVirtualOrigin(inImage, stVirtualOrigin);

	if (stVirtualOrigin.IsNull())
		UGraphics_DrawText(inImage, inBounds, inText, inTextSize, inEncoding, inAlignFlags);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inImage);

		// recalculate rect
		SRect stBounds = inBounds;
		stBounds.MoveBack(stVirtualOrigin.x, stVirtualOrigin.y);
	
		// draw
		UGraphics_DrawText(inImage, stBounds, inText, inTextSize, inEncoding, inAlignFlags);

		// restore virtual origin
		_SetVirtualOrigin(inImage, stVirtualOrigin);
	}
}

void UGraphics::DrawTruncText(TImage inImage, const SRect& inBounds, const void *inText, Uint32 inTextSize, Uint32 inEncoding, Uint32 inAlignFlags)
{
	if (!inText || !inTextSize)
		return;

	// get virtual origin
	SPoint stVirtualOrigin;
	_GetVirtualOrigin(inImage, stVirtualOrigin);

	if (stVirtualOrigin.IsNull())
		UGraphics_DrawTruncText(inImage, inBounds, inText, inTextSize, inEncoding, inAlignFlags);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inImage);

		// recalculate rect
		SRect stBounds = inBounds;
		stBounds.MoveBack(stVirtualOrigin.x, stVirtualOrigin.y);
	
		// draw
		UGraphics_DrawTruncText(inImage, stBounds, inText, inTextSize, inEncoding, inAlignFlags);

		// restore virtual origin
		_SetVirtualOrigin(inImage, stVirtualOrigin);
	}
}

void UGraphics::DrawTextBox(TImage inImage, const SRect& inBounds, const SRect& inUpdateRect, const void *inText, Uint32 inTextSize, Uint32 inEncoding, Uint32 inAlign)
{
	if (!inText || !inTextSize)
		return;

	// get virtual origin
	SPoint stVirtualOrigin;
	_GetVirtualOrigin(inImage, stVirtualOrigin);

	if (stVirtualOrigin.IsNull())
		UGraphics_DrawTextBox(inImage, inBounds, inUpdateRect, inText, inTextSize, inEncoding, inAlign);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inImage);

		// recalculate rects
		SRect stBounds = inBounds;
		stBounds.MoveBack(stVirtualOrigin.x, stVirtualOrigin.y);

		SRect stUpdateRect = inUpdateRect;
		stUpdateRect.MoveBack(stVirtualOrigin.x, stVirtualOrigin.y);
	
		// draw
		UGraphics_DrawTextBox(inImage, stBounds, stUpdateRect, inText, inTextSize, inEncoding, inAlign);

		// restore virtual origin
		_SetVirtualOrigin(inImage, stVirtualOrigin);
	}
}

Uint32 UGraphics::GetTextBoxHeight(TImage inImage, const SRect& inBounds, const void *inText, Uint32 inTextSize, Uint32 inEncoding, Uint32 inAlign)
{
	if (!inText || !inTextSize)
		return 0;

	// get virtual origin
	SPoint stVirtualOrigin;
	_GetVirtualOrigin(inImage, stVirtualOrigin);

	if (stVirtualOrigin.IsNull())
		return UGraphics_GetTextBoxHeight(inImage, inBounds, inText, inTextSize, inEncoding, inAlign);

	// reset virtual origin
	_ResetVirtualOrigin(inImage);

	// recalculate rect
	SRect stBounds = inBounds;
	stBounds.MoveBack(stVirtualOrigin.x, stVirtualOrigin.y);
	
	// draw
	Uint32 nHeight = UGraphics_GetTextBoxHeight(inImage, stBounds, inText, inTextSize, inEncoding, inAlign);

	// restore virtual origin
	_SetVirtualOrigin(inImage, stVirtualOrigin);
	
	return nHeight;
}

void UGraphics::DrawTextLines(TImage inImage, const SRect& inBounds, const void *inText, Uint32 inEncoding, const Uint32 *inLineOffsets, Uint32 inStartLine, Uint32 inEndLine, Uint32 inLineHeight, Uint32 inAlign)
{
	if (!inText || inStartLine >= inEndLine)
		return;

	// get virtual origin
	SPoint stVirtualOrigin;
	_GetVirtualOrigin(inImage, stVirtualOrigin);

	if (stVirtualOrigin.IsNull())
		UGraphics_DrawTextLines(inImage, inBounds, inText, inEncoding, inLineOffsets, inStartLine, inEndLine, inLineHeight, inAlign);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inImage);

		// recalculate rect
		SRect stBounds = inBounds;
		stBounds.MoveBack(stVirtualOrigin.x, stVirtualOrigin.y);
	
		// draw
		UGraphics_DrawTextLines(inImage, stBounds, inText, inEncoding, inLineOffsets, inStartLine, inEndLine, inLineHeight, inAlign);

		// restore virtual origin
		_SetVirtualOrigin(inImage, stVirtualOrigin);
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void UGraphics::CopyPixels(TImage inDest, const SPoint& inDestPt, TImage inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inOptions)
{
	// get dest virtual origin
	SPoint stDestOrigin;
	_GetVirtualOrigin(inDest, stDestOrigin);

	// get source virtual origin
	SPoint stSourceOrigin;
	_GetVirtualOrigin(inSource, stSourceOrigin);

	if (stDestOrigin.IsNull() && stSourceOrigin.IsNull())
		UGraphics_CopyPixels(inDest, inDestPt, inSource, inSourcePt, inWidth, inHeight, inOptions);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inDest);
		_ResetVirtualOrigin(inSource);

		// recalculate dest point
		SPoint stDestPt = inDestPt;
		stDestPt.MoveBack(stDestOrigin.x, stDestOrigin.y);
	
		// recalculate source point
		SPoint stSourcePt = inSourcePt;
		stSourcePt.MoveBack(stSourceOrigin.x, stSourceOrigin.y);

		// copy pixels
		UGraphics_CopyPixels(inDest, stDestPt, inSource, stSourcePt, inWidth, inHeight, inOptions);

		// restore virtual origin
		_SetVirtualOrigin(inDest, stDestOrigin);
		_SetVirtualOrigin(inSource, stSourceOrigin);
	}
}

void UGraphics::CopyPixels(const SPixmap& inDest, const SPoint& inDestPt, TImage inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inOptions)
{
	// get source virtual origin
	SPoint stSourceOrigin;
	_GetVirtualOrigin(inSource, stSourceOrigin);

	if (stSourceOrigin.IsNull())
		UGraphics_CopyPixels(inDest, inDestPt, inSource, inSourcePt, inWidth, inHeight, inOptions);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inSource);
	
		// recalculate source point
		SPoint stSourcePt = inSourcePt;
		stSourcePt.MoveBack(stSourceOrigin.x, stSourceOrigin.y);

		// copy pixels
		UGraphics_CopyPixels(inDest, inDestPt, inSource, stSourcePt, inWidth, inHeight, inOptions);

		// restore virtual origin
		_SetVirtualOrigin(inSource, stSourceOrigin);
	}
}

void UGraphics::CopyPixels(TImage inDest, const SPoint& inDestPt, const SPixmap& inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inOptions)
{
	// get dest virtual origin
	SPoint stDestOrigin;
	_GetVirtualOrigin(inDest, stDestOrigin);

	if (stDestOrigin.IsNull())
		UGraphics_CopyPixels(inDest, inDestPt, inSource, inSourcePt, inWidth, inHeight, inOptions);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inDest);

		// recalculate dest point
		SPoint stDestPt = inDestPt;
		stDestPt.MoveBack(stDestOrigin.x, stDestOrigin.y);

		// copy pixels
		UGraphics_CopyPixels(inDest, stDestPt, inSource, inSourcePt, inWidth, inHeight, inOptions);

		// restore virtual origin
		_SetVirtualOrigin(inDest, stDestOrigin);
	}
}

void UGraphics::CopyPixelsMasked(TImage inDest, const SPoint& inDestPt, TImage inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, TImage inTransMask, Uint32 inOptions)
{
	// get dest virtual origin
	SPoint stDestOrigin;
	_GetVirtualOrigin(inDest, stDestOrigin);

	// get source virtual origin
	SPoint stSourceOrigin;
	_GetVirtualOrigin(inSource, stSourceOrigin);

	if (stDestOrigin.IsNull() && stSourceOrigin.IsNull())
		UGraphics_CopyPixelsMasked(inDest, inDestPt, inSource, inSourcePt, inWidth, inHeight, inTransMask, inOptions);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inDest);
		_ResetVirtualOrigin(inSource);

		// recalculate dest point
		SPoint stDestPt = inDestPt;
		stDestPt.MoveBack(stDestOrigin.x, stDestOrigin.y);
	
		// recalculate source point
		SPoint stSourcePt = inSourcePt;
		stSourcePt.MoveBack(stSourceOrigin.x, stSourceOrigin.y);

		// copy pixels
		UGraphics_CopyPixelsMasked(inDest, stDestPt, inSource, stSourcePt, inWidth, inHeight, inTransMask, inOptions);

		// restore virtual origin
		_SetVirtualOrigin(inDest, stDestOrigin);
		_SetVirtualOrigin(inSource, stSourceOrigin);
	}
}

void UGraphics::CopyPixelsTrans(TImage inDest, const SPoint& inDestPt, TImage inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inTransCol, Uint32 inOptions)
{
	// get dest virtual origin
	SPoint stDestOrigin;
	_GetVirtualOrigin(inDest, stDestOrigin);

	// get source virtual origin
	SPoint stSourceOrigin;
	_GetVirtualOrigin(inSource, stSourceOrigin);

	if (stDestOrigin.IsNull() && stSourceOrigin.IsNull())
		UGraphics_CopyPixelsTrans(inDest, inDestPt, inSource, inSourcePt, inWidth, inHeight, inTransCol, inOptions);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inDest);
		_ResetVirtualOrigin(inSource);

		// recalculate dest point
		SPoint stDestPt = inDestPt;
		stDestPt.MoveBack(stDestOrigin.x, stDestOrigin.y);
	
		// recalculate source point
		SPoint stSourcePt = inSourcePt;
		stSourcePt.MoveBack(stSourceOrigin.x, stSourceOrigin.y);

		// copy pixels
		UGraphics_CopyPixelsTrans(inDest, stDestPt, inSource, stSourcePt, inWidth, inHeight, inTransCol, inOptions);

		// restore virtual origin
		_SetVirtualOrigin(inDest, stDestOrigin);
		_SetVirtualOrigin(inSource, stSourceOrigin);
	}
}

void UGraphics::CopyPixelsTrans(const SPixmap& inDest, const SPoint& inDestPt, TImage inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inTransCol, Uint32 inOptions)
{
	// get source virtual origin
	SPoint stSourceOrigin;
	_GetVirtualOrigin(inSource, stSourceOrigin);

	if (stSourceOrigin.IsNull())
		UGraphics_CopyPixelsTrans(inDest, inDestPt, inSource, inSourcePt, inWidth, inHeight, inTransCol, inOptions);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inSource);
	
		// recalculate source point
		SPoint stSourcePt = inSourcePt;
		stSourcePt.MoveBack(stSourceOrigin.x, stSourceOrigin.y);

		// copy pixels
		UGraphics_CopyPixelsTrans(inDest, inDestPt, inSource, stSourcePt, inWidth, inHeight, inTransCol, inOptions);

		// restore virtual origin
		_SetVirtualOrigin(inSource, stSourceOrigin);
	}
}

void UGraphics::CopyPixelsTrans(TImage inDest, const SPoint& inDestPt, const SPixmap& inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inTransCol, Uint32 inOptions)
{
	// get dest virtual origin
	SPoint stDestOrigin;
	_GetVirtualOrigin(inDest, stDestOrigin);

	if (stDestOrigin.IsNull())
		UGraphics_CopyPixelsTrans(inDest, inDestPt, inSource, inSourcePt, inWidth, inHeight, inTransCol, inOptions);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inDest);

		// recalculate dest point
		SPoint stDestPt = inDestPt;
		stDestPt.MoveBack(stDestOrigin.x, stDestOrigin.y);

		// copy pixels
		UGraphics_CopyPixelsTrans(inDest, stDestPt, inSource, inSourcePt, inWidth, inHeight, inTransCol, inOptions);

		// restore virtual origin
		_SetVirtualOrigin(inDest, stDestOrigin);
	}
}

void UGraphics::StretchPixels(TImage inDest, const SRect& inDestRect, TImage inSource, const SRect& inSourceRect, Uint32 inOptions)
{
	// get dest virtual origin
	SPoint stDestOrigin;
	_GetVirtualOrigin(inDest, stDestOrigin);

	// get source virtual origin
	SPoint stSourceOrigin;
	_GetVirtualOrigin(inSource, stSourceOrigin);

	if (stDestOrigin.IsNull() && stSourceOrigin.IsNull())
		UGraphics_StretchPixels(inDest, inDestRect, inSource, inSourceRect, inOptions);
	else
	{
		// reset virtual origin
		_ResetVirtualOrigin(inDest);
		_ResetVirtualOrigin(inSource);

		// recalculate dest point
		SRect stDestRect = inDestRect;
		stDestRect.MoveBack(stDestOrigin.x, stDestOrigin.y);
	
		// recalculate source point
		SRect stSourceRect = inSourceRect;
		stSourceRect.MoveBack(stSourceOrigin.x, stSourceOrigin.y);

		// stretch pixels
		UGraphics_StretchPixels(inDest, stDestRect, inSource, stSourceRect, inOptions);

		// restore virtual origin
		_SetVirtualOrigin(inDest, stDestOrigin);
		_SetVirtualOrigin(inSource, stSourceOrigin);
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

Uint32 UGraphics::CalcBezierPoints(const SPoint *inControlPoints, Uint32 inControlCount, SPoint *outPoints, Uint32 inMaxCount)
{
	enum {
		maxControlPoints	= 64
	};
	
	Uint32 choose, steps, i;
	Int32 k, n;			// must be signed
	fast_float t, t1, tt, u, fx, fy;
	SFloatPoint carray[maxControlPoints];
	SFloatPoint barray[maxControlPoints];
	
	if (inControlCount > maxControlPoints)
	{
		DebugBreak("CalcBezierPoints - cannot have more than 64 control points");
		Fail(errorType_Misc, error_LimitReached);
	}
	
	/*
	 * Setup Bezier coefficient array once for control polygon
	 */

    n = inControlCount - 1;
    for (k = 0; k <= n; k++)
    {
		if (k == 0)
			choose = 1;
		else if (k == 1)
			choose = n;
		else
			choose = choose * (n-k+1)/k;

		carray[k].x = inControlPoints[k].x * choose;
		carray[k].y = inControlPoints[k].y * choose;
		//carray[k].z = inControlPoints[k].z * choose;		// 3D curves
	};

	/*
	 * Calculate points distributed along the curve
	 */
	
	steps = inMaxCount - 1;		// loose starting point

    for (i = 0; i <= steps; i++)
    {
    	t = (fast_float)i / steps;

		n = inControlCount - 1;
		u = t;
		
		barray[0].x = carray[0].x;
		barray[0].y = carray[0].y;
		//barray[0].z = carray[0].z;			// 3D curves
		
	    for (k = 1; k <= n; k++)
	    {
			barray[k].x = carray[k].x *u;
			barray[k].y = carray[k].y *u;
			//barray[k].z = carray[k].z *u;		// 3D curves
			u = u * t;
		};

		fx = barray[n].x;
		fy = barray[n].y;
		t1 = 1-t;
		tt = t1;
		
		for (k = n-1; k >= 0; k--)
		{
			fx += barray[k].x * tt;
			fy += barray[k].y * tt;
			//fz += barray[k].z *tt;			// 3D curves
			tt = tt * t1;
		}

		outPoints[i].x = fx;
		outPoints[i].y = fy;
	}
	
	return inMaxCount;
}

Uint32 UGraphics::EstimateBezierPointCount(const SPoint *inControlPoints, Uint32 inControlCount)
{
	enum {
		magicConstant	= 64	// make lower to generate more points
	};
	
	Uint32 i;
	Int32 x, y, smallestX, largestX, smallestY, largestY;
	
	if (inControlCount == 0) return 0;

	largestX = smallestX = inControlPoints[0].x;
	largestY = smallestY = inControlPoints[0].y;

	for (i=1; i!=inControlCount; i++)
	{
		x = inControlPoints[i].x;
		y = inControlPoints[i].y;
		
		if (x > largestX) largestX = x;
		if (x < smallestX) smallestX = x;
		
		if (y > largestY) largestY = y;
		if (y < smallestY) smallestY = y;
	}
	
	/*
	 * First we calculate the smallest rectangle that will enclose all
	 * of the points.  Then get the height and width of this rectangle.
	 * To calculate the number of steps, we use:
	 *
	 *       (numberOfControlPoints * (width + height)) / magicConstant
	 *
	 * The number of points includes the starting point, so add one to
	 * the number of steps.
	 */
	
	return ((inControlCount * ((largestX - smallestX) + (largestY - smallestY))) / magicConstant) + 1;
}

// writes 4 points to <outPts>
void UGraphics::GetLinePoly(const SPoint& inStart, const SPoint& inEnd, Uint32 inWidth, SPoint *outPts)
{
	ASSERT(inWidth > 1);
	
	Int32 dx = inEnd.x - inStart.x;
	Int32 dy = inEnd.y - inStart.y;
	
	Int32 topWidth = inWidth / 2;
	Int32 bottomWidth = (inWidth & 1) ? topWidth + 1 : topWidth;

	if (dx == 0)
	{
		// horizontal line
		outPts[0].x = outPts[3].x = inStart.x - topWidth;
		outPts[0].y = outPts[1].y = inStart.y;
		outPts[1].x = outPts[2].x = inStart.x + bottomWidth;
		outPts[2].y = outPts[3].y = inEnd.y;
	}
	else if (dy == 0)
	{
		// vertical line
		outPts[0].x = outPts[3].x = inStart.x;
		outPts[0].y = outPts[1].y = inStart.y - topWidth;
		outPts[1].x = outPts[2].x = inEnd.x;
		outPts[2].y = outPts[3].y = inStart.y + bottomWidth;
	}
	else 
	{
		fast_float length = UMath::SquareRoot( (dx * dx) + (dy * dy) );
		
		// make polygon of line
		SFloatPoint pts[4];
		pts[0].x = pts[3].x = length;
		pts[0].y = pts[1].y = bottomWidth;
		pts[1].x = pts[2].x = 0;
		pts[2].y = pts[3].y = -topWidth;
		
		// calculate angle of slope
		fast_float angle = UMath::ArcTangent((fast_float)dy / (fast_float)dx);
		if (dx < 0) angle += pi;

		// rotate the points to the correct slope
		SMatrix3 mat;
		mat.SetIdentity();
		mat.Rotate(angle);
		mat.TransformPoints(pts, 4);

		// output points and move to proper position
		outPts[0].x = RoundToLong(pts[0].x) + inStart.x;
		outPts[0].y = RoundToLong(pts[0].y) + inStart.y;
		outPts[1].x = RoundToLong(pts[1].x) + inStart.x;
		outPts[1].y = RoundToLong(pts[1].y) + inStart.y;
		outPts[2].x = RoundToLong(pts[2].x) + inStart.x;
		outPts[2].y = RoundToLong(pts[2].y) + inStart.y;
		outPts[3].x = RoundToLong(pts[3].x) + inStart.x;
		outPts[3].y = RoundToLong(pts[3].y) + inStart.y;
	}
}

/*
 * MinimizeColorTable() takes an array of 32-bit color values (<inCount> entries at <inTable>)
 * and writes the same array to <outTable> but omitting duplicate entries for colors that are
 * the same.  Up to 256 colors are written, so <outTable> must point to 1024 bytes of data.
 * If there are more than 256 unique colors in <inTable>, the function fails by returning
 * 0.  Otherwise, it returns the number of colors written to <outTable>.
 */
Uint32 UGraphics::MinimizeColorTable(const Uint32 *inTable, Uint32 inCount, Uint32 *outTable)
{
	Require(inCount);
	ASSERT(inTable && outTable);

	Uint32 c = 0;
	
	while (inCount--)
	{
		Uint32 v = *inTable++;
		Uint32 d = c;
		Uint32 *p = outTable;
		
		while (d--)
		{
			if (*p++ == v)
				goto nextCol;
		}
		
		if (c == 256) return 0;
		outTable[c++] = v;
		
		nextCol:;
	}
	
	return c;
}

// merges new colors into an existing color table (a new color is added to the table if it is not already in the table)
Uint32 UGraphics::MergeColorTable(Uint32 *ioColorTab, Uint32 inInitialCount, Uint32 inMaxColors, const Uint32 *inNewColors, Uint32 inNewColorCount)
{
	Uint32 *curDst, *endDst, *tmpDst;
	const Uint32 *curSrc, *endSrc;
	
	ASSERT(ioColorTab && inNewColors);
	
	if (inInitialCount >= inMaxColors) return inMaxColors;
	
	curDst = ioColorTab + inInitialCount;
	endDst = ioColorTab + inMaxColors;
	curSrc = inNewColors;
	endSrc = inNewColors + inNewColorCount;
	
	while (curSrc != endSrc && curDst != endDst)
	{
		Uint32 col = *curSrc++;
		
		tmpDst = ioColorTab;
		while (tmpDst != curDst)
		{
			if (*tmpDst++ == col)
				goto keepGoing;
		}
		
		*curDst++ = col;
	
		keepGoing:;
	}
	
	return curDst - ioColorTab;
}

/*
 * GetNearestColor() searches a color table for the color in that table that is closest
 * to a given color.   One Uint32 holds one color in the form RGBA (4 bytes).  <inColor>
 * is the color to find the nearest match in the color table at <inColorTab>, which has
 * <inColorCount> entries.  Returns the 0-based index into <inColorTab>.
 */
Uint32 UGraphics::GetNearestColor(Uint32 inColor, const Uint32 *inColorTab, Uint32 inColorCount, Uint32 /* inOptions */)
{
	Int32 deviation = max_Int32;
	
	// extract RGB from packed color, which is in RGBA format regardless of endianess
#if CONVERT_INTS
	Int32 red = inColor & 0xFF;
	Int32 green = (inColor >> 8) & 0xFF;
	Int32 blue = (inColor >> 16) & 0xFF;
#else
	Int32 red = (inColor >> 24) & 0xFF;
	Int32 green = (inColor >> 16) & 0xFF;
	Int32 blue = (inColor >> 8) & 0xFF;
#endif

	Uint8 *colorTab, *closestCol;
	colorTab = closestCol = (Uint8 *)inColorTab;
	
	while (inColorCount--)
	{
		Int32 l = ((red - colorTab[0]) * (red - colorTab[0])) + ((green - colorTab[1]) * (green - colorTab[1])) + ((blue - colorTab[2]) * (blue - colorTab[2]));

		if (l < deviation)
		{
			deviation = l;
			closestCol = colorTab;
			
			if (l == 0) break;
		}

		colorTab += 4;
	}

	return (Uint32 *)closestCol - inColorTab;
}

// returns true if found exact color
bool UGraphics::GetExactColor(Uint32 inColor, const Uint32 *inColorTab, Uint32 inColorCount, Uint32& outIndex)
{
	const Uint32 *col = inColorTab;
	
	while (inColorCount--)
	{
		if (*col++ == inColor)
		{
			outIndex = (col - inColorTab) - 1;
			return true;
		}
	}
	
	outIndex = 0;
	return false;
}

// returns true if found exact color (16 bit color table)
bool UGraphics::GetExactColor(Uint16 inColor, const Uint16 *inColorTab, Uint32 inColorCount, Uint32& outIndex)
{
	const Uint16 *col = inColorTab;
	
	while (inColorCount--)
	{
		if (*col++ == inColor)
		{
			outIndex = (col - inColorTab) - 1;
			return true;
		}
	}
	
	outIndex = 0;
	return false;
}

/*
ValidateCopyRects() validates the supplied points and width/height so that when you
copy pixels from the source to the dest, you won't copy outside of either image.
Returns true if there are pixels to copy, false if there is nothing to do.

What are inDstBnd and inSrcBnd?  For example, if you called UGraphics::FillRect() on
the dest image and you specified inDstBnd, the whole image would be filled.  You can
calculate the rect like this:

img->GetOrigin(org);
img->GetImageSize(w, h);
r.Set(-org.x, -org.y, -org.x + w, -org.y + h);
*/
bool UGraphics::ValidateCopyRects(const SRect& inDstBnd, const SRect& inSrcBnd, SPoint& ioDstPt, SPoint& ioSrcPt, Uint32& ioWidth, Uint32& ioHeight)
{
	// calculate rects to work with
	SRect srcRect(ioSrcPt.x, ioSrcPt.y, ioSrcPt.x + ioWidth, ioSrcPt.y + ioHeight);
	SRect dstRect(ioDstPt.x, ioDstPt.y, ioDstPt.x + ioWidth, ioDstPt.y + ioHeight);
	SRect srcBnd = inSrcBnd;
	SRect overlap;
	
	// translate the source rect and bounds to the same coordinate system as dstRect
	Int32 dx = dstRect.left - srcRect.left;
	Int32 dy = dstRect.top - srcRect.top;
	srcRect.Move(dx, dy);
	srcBnd.Move(dx, dy);
	
	// calculate the intersection of all four rectangles and bail out if no intersection
	srcRect.GetIntersection(srcBnd, overlap);
	overlap.GetIntersection(dstRect, overlap);
	if (!overlap.GetIntersection(inDstBnd, overlap)) return false;

	// area to copy begins at top/left of overlap
	ioDstPt = ioSrcPt = overlap.TopLeft();
	
	// translate source point back to original coordinate system
	ioSrcPt.MoveBack(dx, dy);

	// width/height of area to copy is width/height of overlap
	ioWidth = overlap.GetWidth();
	ioHeight = overlap.GetHeight();
	
	// all done, return true to do the copy
	return true;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

/*
Format of flattened FontDesc:

Uint16 format;		// always 'FD'
Uint16 version;		// currently 1
Uint32 size;
Uint32 effect;
Uint32 weight;
Uint32 shear;
Uint32 encoding;
Uint32 textAlign;
Uint32 customVal;
Uint32 rsvdA[4];	// should all be 0
SColor color;
Uint16 rsvdB;
SColor customColor;
Uint16 rsvdC;
Uint8 standardFont;	// 1-5 for standard fonts
pstring name;
pstring style;
*/
THdl UFontDesc::Flatten(const SFontDesc& inInfo)
{
	Uint32 s;
	const Uint8 *name, *style;
	const SColor *color, *customColor;
	Uint8 stdFont;
	THdl h;
	Uint32 *lp;
	Uint16 *wp;
	Uint8 *p;
	
	name = inInfo.name;
	style = inInfo.style;
	color = inInfo.color;
	customColor = inInfo.customColor;
	
	s = 64 + 3;
	if (style) s += style[0];
	
	if (name == kDefaultFont || name == nil)
		stdFont = 1;
	else if (name == kSystemFont)
		stdFont = 2;
	else if (name == kFixedFont)
		stdFont = 3;
	else if (name == kSansFont)
		stdFont = 4;
	else if (name == kSerifFont)
		stdFont = 5;
	else
	{
		stdFont = 0;
		s += name[0];
	}
	
	h = UMemory::NewHandle(s);
	p = UMemory::Lock(h);
	
	lp = (Uint32 *)p;
	lp[0] = TB((Uint32)0x46440001);
	lp[1] = TB(inInfo.size);
	lp[2] = TB(inInfo.effect);
	lp[3] = TB(inInfo.weight);
	lp[4] = TB(inInfo.shear);
	lp[5] = TB(inInfo.encoding);
	lp[6] = TB(inInfo.textAlign);
	lp[7] = TB(inInfo.customVal);
	lp[8] = 0;
	lp[9] = 0;
	lp[10] = 0;
	lp[11] = 0;

	if (color)
	{
		wp = (Uint16 *)(lp + 12);
		wp[0] = TB(color->red);
		wp[1] = TB(color->green);
		wp[2] = TB(color->blue);
		wp[3] = 0;
	}
	else
	{
		lp[12] = 0;
		lp[13] = 0;
	}

	if (customColor)
	{
		wp = (Uint16 *)(lp + 14);
		wp[0] = TB(customColor->red);
		wp[1] = TB(customColor->green);
		wp[2] = TB(customColor->blue);
		wp[3] = 0;
	}
	else
	{
		lp[14] = 0;
		lp[15] = 0;
	}

	p += 64;
	*p++ = stdFont;
	if (stdFont == 0)
		p += UMemory::Copy(p, name, name[0]+1);
	else
		*p++ = 0;

	if (style)
		p += UMemory::Copy(p, style, style[0]+1);
	else
		*p++ = 0;
	
	UMemory::Unlock(h);
	
	return h;
}
