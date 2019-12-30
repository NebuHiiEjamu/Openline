// =====================================================================
//	CFontWin.h                           (C) Hotline Communications 1999
// =====================================================================
//
// Manages font info for drawing text on Win32

#ifndef _H_CFontWin_
#define _H_CFontWin_

#include "CFont.h"

HL_Begin_Namespace_BigRedH

class CFontWin : public CFont
{
	public:
						CFontWin();
							// throws CGraphicsException
							
		virtual			~CFontWin();
		virtual CFont*	Clone() const;
		
		virtual const CFontMetrics&
						GetMetrics() const;
		virtual CRect 	MeasureText(const CString& inText
									, const TProxy<CGraphicsPort>& inPort) const;

		virtual void 	SetColor (const CColor& inColor);					
		virtual void	SetSize (int inSize);
		virtual void 	SetItalic (bool inState);
		virtual void	SetBold (bool inState);
		virtual void 	SetUnderline(bool inState);
		
		class StFontScoper {
			public:
				StFontScoper (HDC inDC, const CFont& inFont);
				~StFontScoper ();
				
			private:
				StFontScoper (const StFontScoper&);
				StFontScoper& operator= (const StFontScoper&);
				
				HDC mDC;
				HGDIOBJ mObj;
				int mOldBkMode;
				COLORREF mOldTextColor;
		};

		
	private:
		::HFONT			MakeFont() const;
		
		mutable CFontMetrics
						mMetrics;

						CFontWin(const CFontWin&);
		friend class StFontScoper;
};

HL_End_Namespace_BigRedH

#endif