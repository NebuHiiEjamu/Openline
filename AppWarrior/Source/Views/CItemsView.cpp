/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
/****************************************************************

see notes at stop of CMenuListView.cp for U classes

UList - same height & width items, top to bottom, selectable,
drawitem calls supplied proc.  For selection, can store one
byte of flags, one of which is selected.

UHierList - var height items, disclosure triangles, selectable

UVarHeightList - same width, diff height items, top to bottom,
selectable, stores height in an array of offsets

UTable - grid, each item referenced by x,y.  Variable column
and row sizes essential.  Discontiguous selection? Use bitmap?
Must be able to be used as the basis for a spreadsheet.
Does not include data storage.

UTableData - stores var size amount of data for 2D table
(all in-memory)

Except for UTableData, these classes do NOT store any data
for items.  Caller is responsible for that, and can use
do things like use UTableData, or store ptrs to objects
with virtual Draw function (make a C class for that, CCell).

UVarArray - manages an in-memory array of variable size items.
Uses offset table for speedy lookup.  Data stored in handle.
Need to be able to get ptr to data!!  eg when drawing


******************************************************************/

/****************************************************************

CFlexiListView
CFlexiListItem

CFlexiListView (name?) is a list style items view (derived from
CSelectableItemsView or CListView or CMultiHeightListView or
whatever).

It uses item objects (CFlexiListItem) to draw each item.  Each
object has a height, and a virtual Draw() function.  A CPtrList
is used to store the object ptrs.

This makes it very flexible for a list with different types of
items.  Also, it's easy to store stuff since each object is 
allocated on the heap as normal.

************** call it CCell !!!! 

CCell and CCellListView ?

Sure makes it damn easy to make a list, and a very flexible one
too. 

*****************************************************************/




/*
-- CSelectableItemsView Behaviour --

By default, a click or drag will select the item the click was in (leaving the
selection of other items untouched).  If it is already selected, it will stay
selected.

The following constants can be combined to modify behaviour:

itemBehav_DeselectOthers		Click deselects all items except the clicked item
itemBehav_ToggleSelect			Click toggles the selection of the clicked item
itemBehav_ShiftExtend			Shift-click extends the selection contiguously
itemBehav_ShiftToggle			Shift-click toggles the selection of the clicked item
itemBehav_CommandToggle			Command-click toggles the selection of the clicked item
itemBehav_ControlToggle			Ctrl-click toggles the selection of the clicked item
itemBehav_DoubleClickAction		Double-click can perform any action you like
itemBehav_DragDeselectOthers	Dragging deselects all items except the current item
itemBehav_DragArrange			Dragging allows the order of items to be rearranged
itemBehav_DragAction			Dragging becomes Drag & Drop items

For example, lists in the Finder are:
    
itemBehav_DeselectOthers + itemBehav_ShiftToggle + itemBehav_DragAction + itemBehav_DoubleClickAction

The standard open/save file lists are:

itemBehav_DeselectOthers + itemBehav_DragDeselectOthers + itemBehav_DoubleClickAction
*/

#include "CItemsView.h"

/* -------------------------------------------------------------------------- */

CItemsView::CItemsView(CViewHandler *inHandler, const SRect& inBounds)
	: CView(inHandler, inBounds)
{
	mMouseDownItem = mMouseWithinItem = 0;
}

void CItemsView::GetItemRegion(Uint32 inItem, TRegion outRgn) const
{
	SRect r;
	GetItemRect(inItem, r);
	URegion::SetRect(outRgn, r);
}

/*
 * PointToItem() returns which item the specified point is within
 * (or 0 if none).  It calls GetItemRect() for each item until
 * the point is within an item rect.  Derived classes will probably
 * want to override this behaviour to provide a more efficient
 * implementation.
 */
Uint32 CItemsView::PointToItem(const SPoint& inPt) const
{
	Uint32 i = 0;
	SRect r;
	
	while (GetNextItem(i))
	{
		GetItemRect(i, r);
		
		if (r.Contains(inPt))
			return i;
	}
	
	return 0;
}

void CItemsView::RefreshItem(Uint32 inItem)
{
	SRect r;
	GetItemRect(inItem, r);
	Refresh(r);
}

void CItemsView::MakeItemVisible(Uint32 inItem, Uint16 inAlign)
{
	if (IsValidItem(inItem))
	{
		SRect r;
		GetItemRect(inItem, r);
		
		Uint32 nTabHeight = GetTabHeight();
		if (nTabHeight && r.top >= nTabHeight)
			r.top -= nTabHeight;
		
		MakeRectVisible(r, inAlign);
	}
}

void CItemsView::GetFullSize(Uint32& outWidth, Uint32& outHeight) const
{
	Uint32 i, right, bottom;
	SRect r;
	
	i = 0;
	right = mBounds.right;
	bottom = mBounds.bottom;
	
	while (GetNextItem(i))
	{
		GetItemRect(i, r);
		
		if (r.right > right)
			right = r.right;
		
		if (r.bottom > bottom)
			bottom = r.bottom;
	}
	
	outHeight = bottom - mBounds.top;
	outWidth = right - mBounds.left;
}

Uint32 CItemsView::GetFullHeight() const
{
	Uint32 i, bottom;
	SRect r;
	
	i = 0;
	bottom = mBounds.bottom;
	
	while (GetNextItem(i))
	{
		GetItemRect(i, r);
		
		if (r.bottom > bottom)
			bottom = r.bottom;
	}
	
	return bottom - mBounds.top;
}

Uint32 CItemsView::GetFullWidth() const
{
	Uint32 i, right;
	SRect r;
	
	i = 0;
	right = mBounds.right;
	
	while (GetNextItem(i))
	{
		GetItemRect(i, r);
		
		if (r.right > right)
			right = r.right;
	}
	
	return right - mBounds.left;
}

void CItemsView::Draw(TImage inImage, const SRect& inUpdateRect, Uint32 /* inDepth */)
{
	Uint32 i = 0;
	SRect r;
	
	while (GetNextItem(i))
	{
		GetItemRect(i, r);
		
		if (r.Intersects(inUpdateRect))
			ItemDraw(i, inImage, r, inUpdateRect);
	}
}

void CItemsView::MouseDown(const SMouseMsgData& inInfo)
{
	inherited::MouseDown(inInfo);
	
	Uint32 foundItem, i;

	// can't handle multiple mouse downs
	if (mMouseDownItem)
		return;
	
	// find which item the mouse is in
	foundItem = PointToItem(inInfo.loc);
	
	// if item mouse is in has changed, update mouse within item
	if (foundItem != mMouseWithinItem)
	{
		// leave current
		i = mMouseWithinItem;
		if (i)
		{
			mMouseWithinItem = 0;
			ItemMouseLeave(i, inInfo);
		}
	
		// enter new
		if (foundItem)
		{
			ItemMouseEnter(foundItem, inInfo);
			mMouseWithinItem = foundItem;
		}
	}
	
	// give mouse down to the item
	if (foundItem)
	{
		mMouseDownItem = foundItem;
		ItemMouseDown(foundItem, inInfo);
	}
}

void CItemsView::MouseUp(const SMouseMsgData& inInfo)
{
	inherited::MouseUp(inInfo);
	
	Uint32 i = mMouseDownItem;
	
	// give mouse up to the item we gave the mouse down to
	if (i)
	{
		mMouseDownItem = 0;
		ItemMouseUp(i, inInfo);
	}
}

void CItemsView::MouseEnter(const SMouseMsgData& inInfo)
{
	inherited::MouseEnter(inInfo);
	
	// in this case, mouse enter is the same as move
	CItemsView::MouseMove(inInfo);					// don't use the virtual dispatch
}

void CItemsView::MouseMove(const SMouseMsgData& inInfo)
{
	inherited::MouseMove(inInfo);
	
	Uint32 foundItem, i;

	// find which item the mouse is in
	foundItem = PointToItem(inInfo.loc);
	
	// if item mouse is in has changed, update mouse within item
	if (foundItem == mMouseWithinItem)
	{
		if (foundItem) 
			ItemMouseMove(foundItem, inInfo);
	}
	else
	{
		// leave current
		i = mMouseWithinItem;
		if (i)
		{
			mMouseWithinItem = 0;
			ItemMouseLeave(i, inInfo);
		}
	
		// enter new
		if (foundItem)
		{
			ItemMouseEnter(foundItem, inInfo);
			mMouseWithinItem = foundItem;
		}
	}
}

void CItemsView::MouseLeave(const SMouseMsgData& inInfo)
{
	inherited::MouseLeave(inInfo);
	
	Uint32 i = mMouseWithinItem;
	
	// if currently in an item
	if (i)
	{
		// leave that item
		mMouseWithinItem = 0;
		ItemMouseLeave(i, inInfo);
	}
}

// derived classes can provide implementation
void CItemsView::ItemDraw(Uint32, TImage, const SRect&, const SRect&, Uint32) {}
void CItemsView::ItemMouseDown(Uint32, const SMouseMsgData&) {}
void CItemsView::ItemMouseUp(Uint32, const SMouseMsgData&) {}
void CItemsView::ItemMouseEnter(Uint32, const SMouseMsgData&) {}
void CItemsView::ItemMouseMove(Uint32, const SMouseMsgData&) {}
void CItemsView::ItemMouseLeave(Uint32, const SMouseMsgData&) {}

void CItemsView::ItemsInserted(Uint32, Uint32)
{
	if ((mSizing & sizing_FullHeight) && (mSizing & sizing_FullWidth))
		SetFullSize();
	else if (mSizing & sizing_FullHeight)
		SetFullHeight();
	else if (mSizing & sizing_FullWidth)
		SetFullWidth();
}

void CItemsView::ItemsRemoved(Uint32, Uint32)
{
	if ((mSizing & sizing_FullHeight) && (mSizing & sizing_FullWidth))
		SetFullSize();
	else if (mSizing & sizing_FullHeight)
		SetFullHeight();
	else if (mSizing & sizing_FullWidth)
		SetFullWidth();
}

void CItemsView::ItemsMoved(Uint32, Uint32, Uint32) {}
void CItemsView::ItemsSwapped(Uint32, Uint32, Uint32) {}


/* -------------------------------------------------------------------------- */
#pragma mark -

#pragma options align=packed

typedef struct {
	Uint16 behaviour;
	Uint16 rsvd;
} SSelectableItemsView;

#pragma options align=reset

CSelectableItemsView::CSelectableItemsView(CViewHandler *inHandler, const SRect& inBounds)
	: CItemsView(inHandler, inBounds)
{
	mBehaviour = 0;
	mTrackedItem = mDragItem = 0;
	mRefreshSelectedOnSetActive = false;
}

Uint32 CSelectableItemsView::GetFirstSelectedItem() const
{
	Uint32 i = 0;
	
	if (GetNextSelectedItem(i))
		return i;
	
	return 0;
}

Uint32 CSelectableItemsView::GetLastSelectedItem() const
{
	Uint32 i = 0;
	
	while (GetPrevItem(i))
	{
		if (IsItemSelected(i))
			return i;
	}
	
	return 0;
}

void CSelectableItemsView::SetItemRangeSelect(Uint32 inFirstItem, Uint32 inLastItem, bool inSelect)
{
	do {
		SetItemSelect(inFirstItem, inSelect);
	} while (GetNextItem(inFirstItem) && inFirstItem != inLastItem);
}

void CSelectableItemsView::DeselectAll()
{
	Uint32 first, last;
	first = last = 0;
	if (GetNextItem(first) && GetPrevItem(last))
		SetItemRangeSelect(first, last, false);
}

void CSelectableItemsView::SetBehaviour(Uint16 inBehav)
{
	mBehaviour = inBehav;
}

Uint16 CSelectableItemsView::GetBehaviour() const
{
	return mBehaviour;
}

void CSelectableItemsView::GetSelectedItemsRegion(TRegion outRgn) const
{
	Uint32 i = 0;
	StRegion rgn;
	
	URegion::SetEmpty(outRgn);
	
	while (GetNextSelectedItem(i))
	{
		GetItemRegion(i, rgn);
		URegion::Add(outRgn, rgn);
	}
}

// get the smallest rectangle that encloses all selected items
void CSelectableItemsView::GetSelectedItemsRect(SRect& outRect) const
{
	SRect selRect(kZeroRect);
	SRect r;
	Uint32 i = 0;

	while (GetNextSelectedItem(i))
	{
		GetItemRect(i, r);
		selRect |= r;
	}
	
	outRect = selRect;
}

TImage CSelectableItemsView::GetSelectedItemsImage(SPoint& outScreenDelta) const
{
#if MACINTOSH

	TImage img;
	SRect localBounds, globalBounds, r;
	Int32 hd, vd;
	Uint32 i = 0;
	Uint16 depth;
	
	GetSelectedItemsRect(localBounds);
	if (localBounds.IsEmpty()) return nil;
	GetScreenDelta(hd, vd);
	globalBounds = localBounds;
	globalBounds.Move(hd, vd);
	
	img = UGraphics::NewImage(globalBounds);
	
	try
	{
		UGraphics::LockImage(img);
		depth = UGraphics::GetDepth(img);

		r = globalBounds;
		r.Move(-r.left, -r.top);
		UGraphics::SetInkColor(img, color_White);
		UGraphics::FillRect(img, r);

		while (GetNextSelectedItem(i))
		{
			GetItemRect(i, r);
			r.Move(-localBounds.left, -localBounds.top);
			((CSelectableItemsView *)this)->ItemDraw(i, img, r, r, kDrawItemForDrag);
		}
		
		UGraphics::UnlockImage(img);
	}
	catch(...)
	{
		UGraphics::DisposeImage(img);
		throw;
	}
	
	outScreenDelta.h = globalBounds.left;
	outScreenDelta.v = globalBounds.top;
	return img;

#else
	
	#pragma unused(outScreenDelta)
	Fail(errorType_Misc, error_Unimplemented);
	return nil;
	
#endif
}

bool CSelectableItemsView::GetNextSelectedItem(Uint32& ioItem) const
{
	while (GetNextItem(ioItem))
	{
		if (IsItemSelected(ioItem))
			return true;
	}
	return false;
}

/*
bool CSelectableItemsView::CanAcceptDrop(TDrag inDrag) const
{
	if (mDragAndDropHandler)
		return mDragAndDropHandler->HandleCanAcceptDrop((CSelectableItemsView *)this, inDrag);
	return false;
}
*/
bool CSelectableItemsView::KeyDown(const SKeyMsgData& inInfo)
{
	Char16 keyChar = inInfo.keyChar;
	Uint16 keyCode = inInfo.keyCode;
	
	if (keyCode == key_Left || keyCode == key_Up || keyCode == key_Right || keyCode == key_Down || keyCode == key_Home || keyCode == key_End)
	{
		Uint32 i, first, last;

		if (keyCode == key_Left || keyCode == key_Up)				// left or up arrow keys
		{
			// determine which item to select
			if (inInfo.mods & modKey_Command)
			{
				// first item
				i = 0;
				if (!GetNextItem(i)) return true;
			}
			else
			{
				i = GetFirstSelectedItem();
				
				if (i == 0)
				{
					// first item
					if (!GetNextItem(i)) 
						return true;
				}
				else
				{
					// previous item
					if (!GetPrevItem(i))
						return true;
				}
			}
		}
		else if (keyCode == key_Right || keyCode == key_Down)				// right or down arrow keys
		{
			// determine which item to select
			if (inInfo.mods & modKey_Command)
			{
				// last item
				i = 0;
				if (!GetPrevItem(i)) return true;
			}
			else
			{
				i = GetFirstSelectedItem();
				
				if (i == 0)
				{
					// first item
					if (!GetNextItem(i)) 
						return true;
				}
				else
				{
					// next item
					if (!GetNextItem(i))
						return true;
				}
			}
		}
		else if (keyCode == key_Home)
		{
			// first item
			i = 0;
			if (!GetNextItem(i)) 
				return true;
		}
		else if (keyCode == key_End)
		{
			// last item
			i = 0;
			if (!GetPrevItem(i)) 
				return true;
		}
		
		// deselect all items and select that item
		first = last = 0;
		if (GetNextItem(first) && GetPrevItem(last))
			SetItemRangeSelect(first, last, false);
		
		SetItemSelect(i, true);
		MakeItemVisible(i, align_InsideHoriz + align_InsideVert);
		
		// notify change
		if (mBehaviour & itemBehav_ChangeAction) Hit(hitType_Change);
		
		return true;
	}
	
	if (keyChar == '\r' || keyCode == key_nEnter) 				// return or enter key
	{
		if (mBehaviour & itemBehav_DoubleClickAction)
			Hit(hitType_Standard);
			
		return true;
	}
	
	return false;
}

void CSelectableItemsView::KeyUp(const SKeyMsgData& /* inInfo */)
{
	
}

bool CSelectableItemsView::KeyRepeat(const SKeyMsgData& inInfo)
{
	return CSelectableItemsView::KeyDown(inInfo);
}

bool CSelectableItemsView::Drop(const SDragMsgData& inInfo)
{
	if (mDragAndDropHandler)
		return mDragAndDropHandler->HandleDrop(this, inInfo);
	return false;
}

bool CSelectableItemsView::ChangeState(Uint16 inState)
{
	if (mState != inState)	// if change
	{
		mState = inState;
		
		if (mRefreshSelectedOnSetActive && inState != viewState_Hidden)
		{
			SRect r;
			GetSelectedItemsRect(r);
			Refresh(r);
		}
		
		return true;
	}
	
	return false;
}

void CSelectableItemsView::ItemMouseDown(Uint32 inItem, const SMouseMsgData& inInfo)
{
	Uint32 first, last;
		
	// perform the correct kind of behaviour
	if (inInfo.mods & modKey_Shift)							// if the shift key is down
	{
		if (mBehaviour & itemBehav_ShiftToggle)				// if toggling selection
			SetItemSelect(inItem, !IsItemSelected(inItem));
		else if (mBehaviour & itemBehav_ShiftExtend)		// if extending selection
		{
			first = GetFirstSelectedItem();
			last = GetLastSelectedItem();
			if (first == 0)									// if no selected items
				SetItemSelect(inItem, true);				// just select hit item
			else if (inItem >= last)	
				SetItemRangeSelect(first, inItem, true);
			else if (inItem < first)
				SetItemRangeSelect(inItem, last, true);
			else
			{
				SetItemRangeSelect(first, inItem, true);
				last = 0;
				if (GetNextItem(inItem) && GetPrevItem(last))
					SetItemRangeSelect(inItem, last, false);
			}
		}
		else goto ignoreModifiers;
		
		if (mBehaviour & itemBehav_ChangeAction) Hit(hitType_Change);
	}
	else if (inInfo.mods & modKey_Command)					// if the command key is down
	{
		if (mBehaviour & itemBehav_CommandToggle)			// if toggling selection
			SetItemSelect(inItem, !IsItemSelected(inItem));
		else goto ignoreModifiers;
		
		if (mBehaviour & itemBehav_ChangeAction) Hit(hitType_Change);
	}
	else if (inInfo.mods & modKey_Control)					// if the control key is down
	{
		if (mBehaviour & itemBehav_ControlToggle)			// if toggling selection
		{
			first = GetFirstSelectedItem();
			last = GetLastSelectedItem();

			if (inItem != first || inItem != last)
				SetItemSelect(inItem, !IsItemSelected(inItem));
		}
		else goto ignoreModifiers;
		
		if (mBehaviour & itemBehav_ChangeAction) Hit(hitType_Change);
	}	
	else if (inInfo.mods & kIsDoubleClick)		// if this click makes it a double
	{
		if (mBehaviour & itemBehav_DoubleClickAction)
			Hit(hitType_Standard, 0, inInfo.mods);
		else goto ignoreModifiers;
	}
	else
	{
		ignoreModifiers:
		
		if (mBehaviour & itemBehav_ToggleSelect)
		{
			SetItemSelect(inItem, !IsItemSelected(inItem));
			
			/*
			 * Should really set all the items the mouse hits to what we set
			 * the above.  So if the user clicked on an unselected item, they
			 * could drag around to select a whole batch (or if the item was
			 * selected, then drag to deselect a whole batch).
			 */
		}
		else if (mBehaviour & itemBehav_DragArrange)
		{
			DebugBreak("CSelectableItemsView - itemBehav_DragArrange is not yet implemented");
		}
		else if (mBehaviour & itemBehav_DragAction)
		{
			if (!IsItemSelected(inItem))		// if not selected
			{
				if (mBehaviour & itemBehav_DeselectOthers)
				{
					// deselect all
					first = last = 0;
					if (GetNextItem(first) && GetPrevItem(last))
					{
						if (inItem > first)
							SetItemRangeSelect(first, inItem - 1, false);
					
						if (inItem < last)
							SetItemRangeSelect(inItem + 1, last, false);
					}
				}
					
				SetItemSelect(inItem, true);
			}
			
			mDragMouseLoc = inInfo.loc;
			mDragItem = inItem;
		}
		else		// select any item the mouse hits
		{
			if (mBehaviour & itemBehav_DeselectOthers)
			{
				// deselect all items
				first = last = 0;
				if (GetNextItem(first) && GetPrevItem(last))
				{
					if (inItem > first)
						SetItemRangeSelect(first, inItem - 1, false);
					
					if (inItem < last)
						SetItemRangeSelect(inItem + 1, last, false);
				}
			}
			
			// select the item that was hit
			SetItemSelect(inItem, true);
			
			if (mBehaviour & itemBehav_ChangeAction) Hit(hitType_Change);
			
			mTrackedItem = inItem;
		}
	}
}

void CSelectableItemsView::ItemMouseUp(Uint32 /* inItem */, const SMouseMsgData& /* inInfo */)
{
	if ((mBehaviour & itemBehav_MouseUpAction) && mTrackedItem)
		Hit(hitType_Standard, mTrackedItem);
	
	mTrackedItem = 0;
	mDragItem = 0;
}

void CSelectableItemsView::ItemMouseEnter(Uint32 inItem, const SMouseMsgData& inInfo)
{
	CSelectableItemsView::ItemMouseMove(inItem, inInfo);
}

void CSelectableItemsView::ItemMouseMove(Uint32 inItem, const SMouseMsgData& inInfo)
{
	if (IsAnyMouseBtnDown())
	{
		/*
		 * The mouse is down.
		 */
		
		if (mTrackedItem)
		{
			// check if the item we're tracking has been deleted or something
			if (!IsValidItem(mTrackedItem))
			{
				mTrackedItem = 0;
				return;
			}
			
			if (inItem && inItem != mTrackedItem)
			{
				// deselect old
				if (mBehaviour & itemBehav_DragDeselectOthers && mTrackedItem)
					SetItemSelect(mTrackedItem, false);
				
				// select new
				SetItemSelect(inItem, true);
				
				// new item is now old
				mTrackedItem = inItem;
			}
		}
		else if (mDragItem)
		{
			if (mBehaviour & itemBehav_DragAction)
			{
				SMouseMsgData info = inInfo;
				Uint32 i = mDragItem;
				mDragItem = 0;
				info.loc = mDragMouseLoc;
				ItemDrag(i, info);
			}
		}
	}
	else
	{
		/*
		 * The mouse is up.
		 */

		if (mBehaviour & itemBehav_MouseMoveSelect)
		{
			if (mBehaviour & itemBehav_DragDeselectOthers)
			{
				// deselect all
				Uint32 first = 0;
				Uint32 last = 0;
				if (GetNextItem(first) && GetPrevItem(last))
					SetItemRangeSelect(first, last, false);
			}
			
			// select new
			SetItemSelect(inItem, true);
		}
	}
}

void CSelectableItemsView::ItemMouseLeave(Uint32 /* inItem */, const SMouseMsgData& /* inInfo */)
{
	// nothing to do
}

// item is item mouse is on, but all selected items should be dragged
void CSelectableItemsView::ItemDrag(Uint32 /* inItem */, const SMouseMsgData& inInfo)
{
	if (mDragAndDropHandler)
		mDragAndDropHandler->HandleDrag(this, inInfo);
}

