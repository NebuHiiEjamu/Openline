/*	This class should probably not be derived from CContainerView.
**	It can be made more efficient if it is just a MultiContainerView interface.
*/

#include "CPaneView.h"

// orientations
// specifies where view2 is in relation to view1
enum
{
	or_Right	= 1,
	or_Left		= 2,
	or_Below	= 3,
	or_Above	= 4

};


CPaneView::CPaneView(CViewHandler *inHandler, const SRect& inBounds)
	:	CContainerView(inHandler, inBounds)
{
	mMoving = false;
	mOrientation = 0;
	mIgnoreSetBounds = false;
	
	// set all limits to 0
	ClearStruct(mLimit);
}

#pragma mark -

void CPaneView::SetViewLimits(CView *inView, Int32 inMinWidth, Int32 inMinHeight)
{
	Uint32 i = mViewList.GetItemIndex(inView);
	
	if(i == 1 || i == 2)
	{
		mLimit[--i].minWidth = inMinWidth;
		mLimit[i].minHeight = inMinHeight;
	}
}

bool CPaneView::SetBounds(const SRect& inBounds)
{

	CView *v1, *v2;
	v1 = mViewList.GetItem(1);
	v2 = mViewList.GetItem(2);
	SRect r1, r2;
	Int32 w, h;
	
	if(!v1 || !v2)
		return CContainerView::SetBounds(inBounds);

	mIgnoreSetBounds = true;

	if(CContainerView::SetBounds(inBounds))
	{
		// depending on the sizing_ of the views, their bounds may have changed
		v1->GetBounds(r1);
		v2->GetBounds(r2);

		// check that each view's bounds is legal based on its limits
		// if not, resize it
		// Note that if this pane is too small to accomodate both views, the first is shrunk
		// instead, this should cause a debug break 
		switch(mOrientation)
		{
			case or_Right:
				w = r1.GetWidth();
				if(w < mLimit[0].minWidth)
				{
					r1.right = r1.left + mLimit[0].minWidth;
					r2.left += mLimit[0].minWidth - w;
				}
				
				w = r2.GetWidth();
				if(w < mLimit[0].minWidth)
				{
					r2.left = r2.right - mLimit[1].minWidth;
					r1.right -= mLimit[1].minWidth - w;
				}
				break;
			
			case or_Left:
				w = r1.GetWidth();
				if(w < mLimit[0].minWidth)
				{
					r1.left = r1.right - mLimit[0].minWidth;
					r2.right -= mLimit[0].minWidth - w;
				}
				
				w = r2.GetWidth();
				if(w < mLimit[1].minWidth)
				{
					r2.right = r2.left + mLimit[1].minWidth;
					r1.left += mLimit[1].minWidth - w;
				}
				break;
			
			case or_Below:
				h = r1.GetHeight();
				if(h < mLimit[0].minHeight)
				{
					r1.bottom = r1.top + mLimit[0].minHeight;
					r2.top += mLimit[0].minHeight - h;
				}
				
				h = r2.GetHeight();
				if(h < mLimit[1].minHeight)
				{
					r2.top = r2.bottom - mLimit[1].minHeight;
					r1.bottom -= mLimit[1].minHeight - h;
				}
				break;
				
			case or_Above:
				h = r1.GetHeight();
				if(h < mLimit[0].minHeight)
				{
					r1.top = r1.bottom - mLimit[0].minHeight;
					r2.bottom -= mLimit[0].minHeight - h;
				}
				
				h = r2.GetHeight();
				if(h < mLimit[1].minHeight)
				{
					r2.bottom = r2.top + mLimit[1].minHeight;
					r1.top += mLimit[1].minHeight - h;
				}
				break;
				
			default:
				return true;	// should never get here
				break;
		};

		
		v1->SetBounds(r1);
		v2->SetBounds(r2);
		
		RecalcSlider();
		mIgnoreSetBounds = false;
		return true;
	}
	
	mIgnoreSetBounds = false;

	return false;
}

void CPaneView::SetPanePercent(Uint8 inPanePercent)
{
	if (!inPanePercent)
		return;
		
	CView *v1 = mViewList.GetItem(1);
	CView *v2 = mViewList.GetItem(2);
	if (!v1 || !v2)
		return;

	if (inPanePercent > 100)
		inPanePercent = 100;

	SRect stBounds;
	GetBounds(stBounds);

	SRect r1, r2;
	v1->GetBounds(r1);
	v2->GetBounds(r2);

	Int32 dx = 0;
	Int32 dy = 0;

	switch(mOrientation)
	{
		case or_Right:
			dx = ((double)inPanePercent * stBounds.GetWidth() / 100 + 0.5) - r1.GetWidth();
			if(dx < 0)
			{
				if(r1.GetWidth() + dx < mLimit[0].minWidth)
					dx = mLimit[0].minWidth - r1.GetWidth();
			}
			else if(dx)	// if it's positive
			{
				if(r2.GetWidth() - dx < mLimit[1].minWidth)
					dx = r2.GetWidth() - mLimit[1].minWidth;
			}
				
			r1.right += dx;
			r2.left += dx;
			break;
			
		case or_Left:
			dx = r1.GetWidth() - ((double)inPanePercent * stBounds.GetWidth() / 100 + 0.5);
			if(dx < 0)
			{
				if(r2.GetWidth() + dx < mLimit[1].minWidth)
					dx = mLimit[1].minWidth - r2.GetWidth();
			}
			else if(dx)
			{
				if(r1.GetWidth() - dx < mLimit[0].minWidth)
					dx = r1.GetWidth() - mLimit[0].minWidth;
			}
				
			r1.left += dx;
			r2.right += dx;
			break;
			
		case or_Below:
			dy = ((double)inPanePercent * stBounds.GetHeight() / 100 + 0.5) - r1.GetHeight();
			if(dy < 0)
			{
				if(r1.GetHeight() + dy < mLimit[0].minHeight)
					dy = mLimit[0].minHeight - r1.GetHeight();
			}
			else if(dy)
			{
				if(r2.GetHeight() - dy < mLimit[1].minHeight)
					dy =  r2.GetHeight() - mLimit[1].minHeight;
			}

			r1.bottom += dy;
			r2.top += dy;
			break;
				
		case or_Above:
			dy = r1.GetHeight() - ((double)inPanePercent * stBounds.GetHeight() / 100 + 0.5);
			if(dy < 0)
			{
				if(r2.GetHeight() + dy < mLimit[1].minHeight)
					dy =  mLimit[1].minHeight - r2.GetHeight();
			}
			else if(dy)
			{
				if(r1.GetHeight() - dy < mLimit[0].minHeight)
					dy = r1.GetHeight() - mLimit[0].minHeight;
			}

			r1.top += dy;
			r2.bottom += dy;
			break;
				
			default:
				return;	// should never get here
				break;
	};
		
	if(!dx && !dy)
		return;
		
	mSliderRect.Move(dx, dy);
		
	mIgnoreSetBounds = true;

	v1->SetBounds(r1);
	v2->SetBounds(r2);		
	
	mIgnoreSetBounds = false;
}

Uint8 CPaneView::GetPanePercent()
{
	CView *v1 = mViewList.GetItem(1);
	if (!v1)
		return 0;

	SRect stBounds;
	GetBounds(stBounds);

	SRect r1;
	v1->GetBounds(r1);

	Uint8 nPercent = 0;
	switch (mOrientation)
	{
		case or_Right:
		case or_Left:
			nPercent = (double)100 * r1.GetWidth() / stBounds.GetWidth() + 0.5;
			break;
			
		case or_Below:
		case or_Above:
			nPercent = (double)100 * r1.GetHeight() / stBounds.GetHeight() + 0.5;
			break;
	};

	return nPercent;
}

#pragma mark -

void CPaneView::MouseDown(const SMouseMsgData& inInfo)
{
	CView *v1, *v2;

	if (!mMoving && mSliderRect.Contains(inInfo.loc))
	{
		v1 = mViewList.GetItem(1);
		v2 = mViewList.GetItem(2);
		
		if (v1 && v2)
		{
			v1->GetBounds(mOldBounds[0]);
			v2->GetBounds(mOldBounds[1]);
			mOldSliderRect = mSliderRect;
			mDownPt = inInfo.loc;

			CaptureMouse();
			switch (mOrientation)
			{
				case or_Right:
				case or_Left:
					if (UMouse::GetImage() != mouseImage_SizeHeadWE)
						UMouse::SetImage(mouseImage_SizeHeadWE);
				break;
			
				case or_Below:
				case or_Above:
					if (UMouse::GetImage() != mouseImage_SizeHeadNS)
						UMouse::SetImage(mouseImage_SizeHeadNS);
				break;				
			};
			
			mMoving = true;
		}
	}

	if (!mMoving)
		CContainerView::MouseDown(inInfo);
}

void CPaneView::MouseMove(const SMouseMsgData& inInfo)
{
	CView *v1, *v2;
	SRect r1, r2, r1Old, r2Old;
	Int32 dx = 0;
	Int32 dy = 0;
	
	if (mMoving)
	{
		v1 = mViewList.GetItem(1);
		v2 = mViewList.GetItem(2);
		
		if (!v1 || !v2)
			return;
		
		v1->GetBounds(r1);
		v2->GetBounds(r2);
		
		r1Old = r1;
		r2Old = r2;
		
		switch (mOrientation)
		{
			case or_Right:
				dx = inInfo.loc.x - mDownPt.x;
				
				// ensure that the resizing results in legal sizes for each view
				if (dx < 0)
				{
					if (mOldBounds[0].GetWidth() + dx < mLimit[0].minWidth)
						dx = mLimit[0].minWidth - mOldBounds[0].GetWidth();
				}
				else if (dx)	// if it's positive
				{
					if (mOldBounds[1].GetWidth() - dx < mLimit[1].minWidth)
						dx = mOldBounds[1].GetWidth() - mLimit[1].minWidth;
				}
				
				r1.right = mOldBounds[0].right + dx;
				r2.left = mOldBounds[1].left + dx;
				break;
			
			case or_Left:
				dx = inInfo.loc.x - mDownPt.x;
				if (dx < 0)
				{
					if (mOldBounds[1].GetWidth() + dx < mLimit[1].minWidth)
						dx = mLimit[1].minWidth - mOldBounds[1].GetWidth();
				}
				else if (dx)
				{
					if (mOldBounds[0].GetWidth() - dx < mLimit[0].minWidth)
						dx = mOldBounds[0].GetWidth() - mLimit[0].minWidth;
				}
				
				r1.left = mOldBounds[0].left + dx;
				r2.right = mOldBounds[1].right + dx;
				break;
			
			case or_Below:
				dy = inInfo.loc.y - mDownPt.y;
				if (dy < 0)
				{
					if (mOldBounds[0].GetHeight() + dy < mLimit[0].minHeight)
						dy = mLimit[0].minHeight - mOldBounds[0].GetHeight();
				}
				else if (dy)
				{
					if (mOldBounds[1].GetHeight() - dy < mLimit[1].minHeight)
						dy =  mOldBounds[1].GetHeight() - mLimit[1].minHeight;
				}

				r1.bottom = mOldBounds[0].bottom + dy;
				r2.top = mOldBounds[1].top + dy;
				break;
				
			case or_Above:
				dy = inInfo.loc.y - mDownPt.y;
				if (dy < 0)
				{
					if (mOldBounds[1].GetHeight() + dy < mLimit[1].minHeight)
						dy =  mLimit[1].minHeight - mOldBounds[1].GetHeight();
				}
				else if (dy)
				{
					if (mOldBounds[0].GetHeight() - dy < mLimit[0].minHeight)
						dy = mOldBounds[0].GetHeight() - mLimit[0].minHeight;
				}

				r1.top = mOldBounds[0].top + dy;
				r2.bottom = mOldBounds[1].bottom + dy;
				break;
				
			default:
				return;	// should never get here
				break;
		}
		
		// see if we've moved the view - if not, return
		if (r1 == r1Old && r2 == r2Old)
			return;
		
		mSliderRect = mOldSliderRect;
		mSliderRect.Move(dx, dy);
		
		mIgnoreSetBounds = true;

		v1->SetBounds(r1);
		v2->SetBounds(r2);		
	
		mIgnoreSetBounds = false;
	}
	else if (!HandleGetCaptureMouseView())
	{
		if (mSliderRect.Contains(inInfo.loc))
		{
			switch (mOrientation)
			{
				case or_Right:
				case or_Left:
					if (UMouse::GetImage() != mouseImage_SizeHeadWE)
						UMouse::SetImage(mouseImage_SizeHeadWE);
				break;
			
				case or_Below:
				case or_Above:
					if (UMouse::GetImage() != mouseImage_SizeHeadNS)
						UMouse::SetImage(mouseImage_SizeHeadNS);
				break;				
			};
		}
		else
		{
			if (UMouse::GetImage() != mouseImage_Standard)
				UMouse::SetImage(mouseImage_Standard);

			CContainerView::MouseMove(inInfo);
		}
	}
	else
		CContainerView::MouseMove(inInfo);
}

void CPaneView::MouseUp(const SMouseMsgData& inInfo)
{
	if (mMoving)
	{
		ReleaseMouse();
		if (UMouse::GetImage() != mouseImage_Standard)
			UMouse::SetImage(mouseImage_Standard);

		mMoving = false;
	}
	else
		CContainerView::MouseUp(inInfo);
}

void CPaneView::MouseLeave(const SMouseMsgData& inInfo)
{
	if (UMouse::GetImage() != mouseImage_Standard)
		UMouse::SetImage(mouseImage_Standard);

	CContainerView::MouseLeave(inInfo);
}

#pragma mark -

void CPaneView::RecalcSlider()
{
	// determine the orientation of the two views
	// first, we must have both views installed
	SRect r1, r2;
	CView *v1, *v2;
	v1 = mViewList.GetItem(1);
	v2 = mViewList.GetItem(2);
	
	if(v1 && v2)
	{		
		v1->GetBounds(r1);
		v2->GetBounds(r2);
		
		if((abs(r1.top - r2.top) + abs(r1.bottom - r2.bottom)) < (abs(r1.left - r2.left) + abs(r1.right - r2.right)))
		{
			// vertical
			// determine which view sits where
			if(r1.left < r2.left && r1.right < r2.right)
			{
				// | v1 | v2 |
				mSliderRect.left = r1.right;
				mSliderRect.right = r2.left;
				
				mOrientation = or_Right;
			}
			else
			{
				// | v2 | v1 |
				mSliderRect.left = r2.right;
				mSliderRect.right = r1.left;
				
				mOrientation = or_Left;
			}
			
			mSliderRect.top = min(r1.top, r2.top);
			mSliderRect.bottom = max(r1.bottom, r2.bottom);			
		}
		else
		{
			// horizontal
			// determine which view sits where
			if(r1.top < r2.top && r1.bottom < r2.bottom)
			{
				// | v1 |
				// +----+
				// | v2 |
				mSliderRect.top = r1.bottom;
				mSliderRect.bottom = r2.top;
				
				mOrientation = or_Below;
			}
			else
			{
				// | v2 |
				// +----+
				// | v1 |
				mSliderRect.top = r2.bottom;
				mSliderRect.bottom = r1.top;
				
				mOrientation = or_Above;
			}
			
			mSliderRect.left = min(r1.left, r2.left);
			mSliderRect.right = max(r1.right, r2.right);
		}
	
		// now move the sliderRect into the view's coordinates
		mSliderRect.Move(mBounds.left, mBounds.top);
	}
}

void CPaneView::HandleInstall(CView *inView)
{
	if(mViewList.GetItemCount() >= 2)
	{
		// this container only accepts 2 views.
		Fail(errorType_Misc, error_Protocol);  
	}
	
	CContainerView::HandleInstall(inView);
	RecalcSlider();
}

void CPaneView::HandleSetBounds(CView *inView, const SRect& inBounds)
{
	CContainerView::HandleSetBounds(inView, inBounds);

	if(!mIgnoreSetBounds)
		RecalcSlider();
}

