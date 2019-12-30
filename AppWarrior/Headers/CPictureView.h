/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "CView.h"

class CPictureView : public CView
{
	public:
		// construction
		CPictureView(CViewHandler *inHandler, const SRect& inBounds);
		CPictureView(CViewHandler *inHandler, const SRect& inBounds, Int32 inPictID);

		// properties
#if MACINTOSH
		virtual void SetPicture(Int16 inID);
		Int16 GetPicture() const;
#else
		void SetPicture(Int32 inID)				{	mRez.Reload('PIXM', inID); Refresh();				}
		void SetPicture(TRez inRez, Int32 inID)	{	mRez.Reload(inRez, 'PIXM', inID, true); Refresh();	}
		void SetPicture(THdl inHdl)				{	mRez.SetHdl(inHdl); Refresh();						}
#endif
		
		virtual void Draw(TImage inImage, const SRect& inUpdateRect, Uint32 inDepth);

	protected:
#if MACINTOSH
		Int16 mPictureID;
#else
		StRezLoader mRez;
#endif
};

#include "CWindow.h"

CWindow *NewPictureWindow(Int16 inID, Int16 inLayer = windowLayer_Modal, bool inInset = false);
