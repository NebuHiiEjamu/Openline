/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "CListView.h"
#include "CTabbedItemsView.h"


// tabbed list view class
class CTabbedListView : public CListView, public CTabbedItemsView
{	
	public:
		// construction
		CTabbedListView(CViewHandler *inHandler, const SRect& inBounds, bool inShowTabLine = false);

		// properties
		virtual bool SetBounds(const SRect& inBounds);

		// mouse events
		virtual void MouseDown(const SMouseMsgData& inInfo);
		virtual void MouseUp(const SMouseMsgData& inInfo);
		virtual void MouseMove(const SMouseMsgData& inInfo);
		virtual void MouseLeave(const SMouseMsgData& inInfo);

	protected:
		virtual void ItemDraw(Uint32 inListIndex, TImage inImage, const SRect& inBounds, const SRect& inUpdateRect, Uint32 inOptions = 0);
		virtual void ItemDraw(Uint32 inListIndex, TImage inImage, const SRect& inBounds, const CPtrList<SRect>& inTabRectList, Uint32 inOptions = 0);

		// mouse events
		virtual void ItemMouseDown(Uint32 inListIndex, const SMouseMsgData& inInfo);
		virtual void ItemMouseUp(Uint32 inListIndex, const SMouseMsgData& inInfo);
		virtual void ItemMouseEnter(Uint32 inListIndex, const SMouseMsgData& inInfo);
		virtual void ItemMouseMove(Uint32 inListIndex, const SMouseMsgData& inInfo);
		virtual void ItemMouseLeave(Uint32 inListIndex, const SMouseMsgData& inInfo);

		// misc
		virtual void RefreshTabView();
		virtual void GetTabViewBounds(SRect& outBounds);
		virtual void GetTabViewScrollPos(Uint32& outHorizPos, Uint32& outVertPos);
		virtual Uint32 GetTabHeight() const;
};
