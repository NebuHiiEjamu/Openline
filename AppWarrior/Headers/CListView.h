/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "CItemsView.h"


// abstract list view class
class CListView : public CSelectableItemsView
{	
	public:
		// construction
		CListView(CViewHandler *inHandler, const SRect& inBounds);
		
		// items
		virtual bool IsValidItem(Uint32 inItem) const;
		virtual bool GetNextItem(Uint32& ioItem) const;
		virtual bool GetPrevItem(Uint32& ioItem) const;

		// item selection
		virtual void SetItemSelect(Uint32 inItem, bool inSelect);
		virtual bool IsItemSelected(Uint32 inItem) const;
		virtual Uint32 GetFirstSelectedItem() const;
		virtual Uint32 GetLastSelectedItem() const;
		virtual void SetItemRangeSelect(Uint32 inFirstItem, Uint32 inLastItem, bool inSelect);

		// full size
		virtual void GetFullSize(Uint32& outWidth, Uint32& outHeight) const;
		virtual Uint32 GetFullHeight() const;
		virtual Uint32 GetFullWidth() const;

		// misc
		virtual void GetItemRect(Uint32 inItem, SRect& outRect) const;
		virtual void GetSelectedItemsRect(SRect& outRect) const;
		virtual Uint32 PointToItem(const SPoint& inPt) const;

		virtual void Draw(TImage inImage, const SRect& inUpdateRect, Uint32 inDepth);
		virtual void DragEnter(const SDragMsgData& inInfo);
		virtual void DragLeave(const SDragMsgData& inInfo);

	protected:
		CBoolArray mSelectData;
		Uint16 mCellHeight;
		
		// item changes
		virtual void ItemsInserted(Uint32 inAtItem, Uint32 inCount);
		virtual void ItemsRemoved(Uint32 inAtItem, Uint32 inCount);
		virtual void ItemsMoved(Uint32 inToItem, Uint32 inFromItem, Uint32 inCount);
		virtual void ItemsSwapped(Uint32 inItemA, Uint32 inItemB, Uint32 inCount);
};
