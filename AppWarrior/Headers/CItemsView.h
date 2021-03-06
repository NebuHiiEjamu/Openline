/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "CView.h"
#include "CBoolArray.h"
#include "CDragAndDroppable.h"

// abstract items view class
class CItemsView : public CView
{
	public:
		// construction
		CItemsView(CViewHandler *inHandler, const SRect& inBounds);
		
		// items
		virtual bool IsValidItem(Uint32 inItem) const = 0;
		virtual bool GetNextItem(Uint32& ioItem) const = 0;
		virtual bool GetPrevItem(Uint32& ioItem) const = 0;
		virtual Uint32 GetItemCount() const = 0;
		void DrawOneItem(Uint32 inItem, TImage inImage, const SRect& inBounds, const SRect& inUpdateRect, Uint32 inOptions = 0)	{	ItemDraw(inItem, inImage, inBounds, inUpdateRect, inOptions);	}

		// misc
		virtual void GetItemRect(Uint32 inItem, SRect& outRect) const = 0;
		virtual void GetItemRegion(Uint32 inItem, TRegion outRgn) const;
		virtual Uint32 PointToItem(const SPoint& inPt) const;
		void RefreshItem(Uint32 inItem);
		void MakeItemVisible(Uint32 inItem, Uint16 inAlign = align_Inside);
		
		// full size
		virtual void GetFullSize(Uint32& outWidth, Uint32& outHeight) const;
		virtual Uint32 GetFullHeight() const;
		virtual Uint32 GetFullWidth() const;
		
		// misc
		virtual void Draw(TImage inImage, const SRect& inUpdateRect, Uint32 inDepth);
		virtual void MouseDown(const SMouseMsgData& inInfo);
		virtual void MouseUp(const SMouseMsgData& inInfo);
		virtual void MouseEnter(const SMouseMsgData& inInfo);
		virtual void MouseMove(const SMouseMsgData& inInfo);
		virtual void MouseLeave(const SMouseMsgData& inInfo);

	protected:
		Uint32 mMouseDownItem, mMouseWithinItem;

		// item events
		virtual void ItemDraw(Uint32 inItem, TImage inImage, const SRect& inBounds, const SRect& inUpdateRect, Uint32 inOptions = 0);
		virtual void ItemMouseDown(Uint32 inItem, const SMouseMsgData& inInfo);
		virtual void ItemMouseUp(Uint32 inItem, const SMouseMsgData& inInfo);
		virtual void ItemMouseEnter(Uint32 inItem, const SMouseMsgData& inInfo);
		virtual void ItemMouseMove(Uint32 inItem, const SMouseMsgData& inInfo);
		virtual void ItemMouseLeave(Uint32 inItem, const SMouseMsgData& inInfo);
		
		// item changes
		virtual void ItemsInserted(Uint32 inAtItem, Uint32 inCount);
		virtual void ItemsRemoved(Uint32 inAtItem, Uint32 inCount);
		virtual void ItemsMoved(Uint32 inToItem, Uint32 inFromItem, Uint32 inCount);
		virtual void ItemsSwapped(Uint32 inItemA, Uint32 inItemB, Uint32 inCount);
		
		// tab height
		virtual Uint32 GetTabHeight() const		{	return 0;		}
};

// list selection behaviour
enum {
	itemBehav_DeselectOthers		= 0x001,
	itemBehav_ToggleSelect			= 0x002,
	itemBehav_ShiftExtend			= 0x004,
	itemBehav_ShiftToggle			= 0x008,
	itemBehav_CommandToggle			= 0x010,
	itemBehav_DoubleClickAction		= 0x020,
	itemBehav_DragDeselectOthers	= 0x040,
	itemBehav_DragArrange			= 0x080,
	itemBehav_DragAction			= 0x100,
	itemBehav_MouseUpAction			= 0x200,
	itemBehav_ChangeAction			= 0x400,
	itemBehav_MouseMoveSelect		= 0x800,
	itemBehav_ControlToggle			= 0x1000,
	itemBehav_SelectOnlyOne			= itemBehav_DeselectOthers + itemBehav_DragDeselectOthers
};

// item draw options
enum {
	kDrawItemForButton				= 1,
	kDrawItemHilitedForButton		= 2,
	kDrawItemDisabledForButton		= 3,
	kDrawItemForDrag				= 4
};

// abstract selectable items view class
class CSelectableItemsView : public CItemsView, public CDragAndDroppable
{	
	public:
		// construction
		CSelectableItemsView(CViewHandler *inHandler, const SRect& inBounds);
	
		// item selection
		virtual void SetItemSelect(Uint32 inItem, bool inSelect) = 0;
		virtual bool IsItemSelected(Uint32 inItem) const = 0;
		void SelectItem(Uint32 inItem)								{	SetItemSelect(inItem, true);		}
		void DeselectItem(Uint32 inItem)							{	SetItemSelect(inItem, false);		}
		virtual Uint32 GetFirstSelectedItem() const;
		virtual Uint32 GetLastSelectedItem() const;
		virtual void SetItemRangeSelect(Uint32 inFirstItem, Uint32 inLastItem, bool inSelect);
		void DeselectAll();
		
		// behaviour
		virtual void SetBehaviour(Uint16 inBehav);
		Uint16 GetBehaviour() const;
		
		// misc
		virtual bool GetNextSelectedItem(Uint32& ioItem) const;
		virtual void GetSelectedItemsRegion(TRegion outRgn) const;
		virtual void GetSelectedItemsRect(SRect& outRect) const;
		virtual TImage GetSelectedItemsImage(SPoint& outScreenDelta) const;
//		virtual bool CanAcceptDrop(TDrag inDrag) const;
		virtual bool ChangeState(Uint16 inState);
	
		// internal functions
		virtual bool KeyDown(const SKeyMsgData& inInfo);
		virtual void KeyUp(const SKeyMsgData& inInfo);
		virtual bool KeyRepeat(const SKeyMsgData& inInfo);
		virtual bool Drop(const SDragMsgData& inInfo);

	protected:
		Uint16 mBehaviour;
		Uint32 mTrackedItem, mDragItem;
		SPoint mDragMouseLoc;
		Uint16 mRefreshSelectedOnSetActive	: 1;
		
		// item events
		virtual void ItemMouseDown(Uint32 inItem, const SMouseMsgData& inInfo);
		virtual void ItemMouseUp(Uint32 inItem, const SMouseMsgData& inInfo);
		virtual void ItemMouseEnter(Uint32 inItem, const SMouseMsgData& inInfo);
		virtual void ItemMouseMove(Uint32 inItem, const SMouseMsgData& inInfo);
		virtual void ItemMouseLeave(Uint32 inItem, const SMouseMsgData& inInfo);
		virtual void ItemDrag(Uint32 inItem, const SMouseMsgData& inInfo);
};

