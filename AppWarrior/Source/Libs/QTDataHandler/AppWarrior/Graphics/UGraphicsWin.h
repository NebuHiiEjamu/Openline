// =====================================================================
//	UGraphicsWin.h                       (C) Hotline Communications 1999
// =====================================================================
// Utility class for windows specific conversions.
// Also includes template wrappers for low-level drawing routines.

#ifndef _H_UGraphicsWin_
#define _H_UGraphicsWin_

#include "CColor.h"
#include "CRect.h"

HL_Begin_Namespace_BigRedH

class UGraphicsWin {
	public:
		static inline void
						CColorToCOLORREF (
							const CColor& inColor
							,COLORREF* outColor);
							// throws nothing
							
		static inline void
						COLORREFToCColor (
							const COLORREF& inColor
							,CColor* outColor);
							// throws nothing
					
			// Converts a CRect into a Win32 Rect
		static inline void 		CRectToRect (
									const CRect& inRect,
									::RECT* outRect);
									// throws nothing
									
		// Converts a Rect into a CRect
		static inline void 		RectToCRect (
									const ::RECT& inRect,
									CRect* outRect);
									// throws nothing
		
			// Inserts the correct template parameters for you.
		static void BlendBitmap	(
						unsigned char* inSrcRows
						,long inSrcRowBytes
						,unsigned char* inDestRows
						,long inDestRowBytes
						,unsigned char* inMaskRows
						,long inMaskRowBytes
						,unsigned char* outRows
						,long inOutRowBytes
						,unsigned long inRowPixels
						,unsigned long inRowCount
						,int inBPPA
						,int inBPPB
						,int inBPPOut);
						// throws nothing
		
		static void	InlineBlendBitmap	 (	
						unsigned char* inSrcRowsA
						,long inSrcRowBytesA
						,unsigned char* inSrcRowsB
						,long  inSrcRowBytesB
						,unsigned char* outRows
						,long  inOutRowBytes 
						,unsigned long  inRowPixels
						,unsigned long  inRowCount
						,int inBPPA
						,int inBPPB
						,int inBPPOut);
						// throws nothing
		
		static void BlendConstantAlphaBitmap (	
						unsigned char* inSrcRows
						,long inSrcRowBytes
						,unsigned char* inDestRows
						,long  inDestRowBytes
						,unsigned char* outRows
						,long  inOutRowBytes 
						,unsigned long  inRowPixels
						,unsigned long  inRowCount
						,unsigned char inAlpha
						,int inBPPA
						,int inBPPB
						,int inBPPOut);
						// throws nothing
						
		static void ColorKeyBitmap		 (
						unsigned char* inSrcRowsA
						,long inSrcRowBytesA
						,unsigned char* inSrcRowsB
						,long  inSrcRowBytesB
						,unsigned char* outRows
						,long  inOutRowBytes 
						,unsigned long  inRowPixels
						,unsigned long  inRowCount
						,const CColor& inColorKey
						,int inBPPA
						,int inBPPB
						,int inBPPOut);
						// throws nothing

};

// ---------------------------------------------------------------------
//  CColorToCOLORREF                            [public][static][inline]
// ---------------------------------------------------------------------
// Converts a CColor to a COLORREF
inline void
UGraphicsWin::CColorToCOLORREF (const CColor& inColor, COLORREF* outColor) 
{
	*outColor = static_cast<COLORREF>(inColor.GetBlue()) << 16 |
				static_cast<COLORREF>(inColor.GetGreen()) << 8 |
				static_cast<COLORREF>(inColor.GetRed());
}

// ---------------------------------------------------------------------
//  COLORREFToCColor                            [public][static][inline]
// ---------------------------------------------------------------------
// Converts a COLORREF to a CColor
inline void
UGraphicsWin::COLORREFToCColor (const COLORREF& inColor, CColor* outColor) 
{
	outColor->SetBlue(static_cast<ColorChannel>((inColor >> 16) & 0xFF));
	outColor->SetGreen(static_cast<ColorChannel>((inColor >> 8) & 0xFF));
	outColor->SetRed(static_cast<ColorChannel>(inColor & 0xFF));
}

// ---------------------------------------------------------------------
//  CRectToRect                                 [public][static][inline]
// ---------------------------------------------------------------------
//
// Converts a CRect into a Win32 Rect

inline void
UGraphicsWin::CRectToRect (const CRect& inRect, ::RECT* outRect) 
{
	outRect->top = inRect.GetTop();
	outRect->left = inRect.GetLeft();
	outRect->bottom = inRect.GetBottom();
	outRect->right = inRect.GetRight();
}


// ---------------------------------------------------------------------
//  RectToCRect                                 [public][static][inline]
// ---------------------------------------------------------------------
//
// Converts a Win32 Rect to a CRect

inline void 
UGraphicsWin::RectToCRect (const ::RECT& inRect, CRect* outRect) 
{
	(*outRect) = CRect(inRect.top, inRect.left
					,inRect.bottom, inRect.right);
}

HL_End_Namespace_BigRedH

#endif // _H_UGraphicsWin_