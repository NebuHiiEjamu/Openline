// =====================================================================
//	CImageImp_W.h                        (C) Hotline Communications 1999
// =====================================================================
// The offscreen buffer and image holder (WIN32 implementation)

#ifndef _H_CImageImp_W_
#define _H_CImageImp_W_

#if PRAGMA_ONCE
	#pragma once
#endif			  

#ifndef _H_CImage_
	#include "CImage.h"
#endif

#ifndef _H_CGraphicsPort_
	#include "CGraphicsPort.h"
#endif

HL_Begin_Namespace_BigRedH

class CImage::Imp : public TRefCountingCreator<CGraphicsPort>
{
	public:
							Imp();		
							~Imp();		
		

		BITMAPINFOHEADER*	mInfo;
		UInt8*				mMemory;
		HBITMAP 			mHandleBMP;
		EImageDepth			mDepth;
		
		void 				CreateDIBSection(const CSize inSize, EImageDepth inBPP);
		
	protected:
		CGraphicsPort*		CreateProxy();
		
//		void 				SetAlpha(UInt8 inAlpha, const CRect inBounds);
}; // class CImage::Imp

HL_End_Namespace_BigRedH
#endif //_H_CImageImp_W_
