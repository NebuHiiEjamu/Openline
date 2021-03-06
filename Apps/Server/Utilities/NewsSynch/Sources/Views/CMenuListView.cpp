/*****************************************************

- extract code out of this into a UMenuData (class for storing menu items)
- or would it draw the whole menu too?   what about UFixedSizeList ?  UVarSizeList ?  UTable ?

- if UMenuData, have func to return the THdl and func to get offsets to menuitems
  (for fast access when drawing etc)

alternatively, and maybe simpler, have a UMenu that does everything including
storage of the menu items and drawing.  This is like UEditText - UEditText
both stores and draws the text.  Makes sense.

Note:  for each menu item, store name and also a data string.  Data string
can come in very handy.

********************************************************/


/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "CMenuListView.h"
#include "UMemory.h"

/*
 * Structures
 */

#pragma options align=packed

typedef struct {
	Char16 markChar;
	Int16 fontID;
	Uint8 fontSize, fontStyle;
	Int32 iconID;
	Int32 hierID;
	Uint32 cmdNum;
	Uint32 userRef;
	Uint32 rsvd;
	Uint16 shortcut;
	Uint16 isTextStyle	: 1;
	Uint16 useFont		: 1;
	Uint16 isKeycode	: 1;
	Uint16 commandKey	: 1;
	Uint16 optionKey	: 1;
	Uint16 shiftKey		: 1;
	Uint16 controlKey	: 1;
	Uint16 isDisabled	: 1;
	Uint16 wasDisabled	: 1;
	Uint16 isTicked		: 1;
	Int16 script;
	Uint16 titleLen;
	Uint8 titleData[];
} SMenuItem;

struct SMenuListView {
	Int16 helpID;
	Int16 textStyleID;
	Uint32 rsvd[4];
	Uint16 itemCount;		// includes menu title item
	Uint8 itemData[];		// first item is menu title
};

#pragma options align=reset

/* -------------------------------------------------------------------------- */

CMenuListView::CMenuListView(CViewHandler *inHandler, const SRect& inBounds)
	: CListView(inHandler, inBounds)
{
	mData = nil;
	mItemOffsets = nil;
	
	try
	{
		mData = UMemory::NewHandleClear(sizeof(SMenuListView));
		mItemOffsets = UMemory::NewHandle(sizeof(Uint32));
		
		*(Uint32 *)UMemory::Lock(mItemOffsets) = sizeof(SMenuListView);
		UMemory::Unlock(mItemOffsets);
	}
	catch(...)
	{
		UMemory::Dispose((THdl)mData);
		UMemory::Dispose((THdl)mItemOffsets);
		throw;
	}
	
	mCellHeight = 18;
}

CMenuListView::~CMenuListView()
{
	UMemory::Dispose(mData);
	UMemory::Dispose(mItemOffsets);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

Uint32 CMenuListView::InsertItems(Uint32 inIndex, Uint32 inCount, const void *inText, Uint32 inSize)
{
	Uint32 itemCount;
	{
		SMenuListView *tempData;
		StHandleLocker tempLock(mData, (void*&)tempData);
		itemCount = tempData->itemCount;
	}

	if (inIndex > itemCount)
		inIndex = itemCount + 1;
	else if (inIndex < 1)
		inIndex = 1;
	
	if (inCount > 0)
	{
		Uint32 dataSize = sizeof(SMenuItem) + inSize;
		Uint32 lastIndex = inIndex + inCount - 1;
		Uint32 itemsSize = inCount * dataSize;
		Uint32 i;

		UMemory::SetSize(mItemOffsets, (itemCount + inCount + 1) * sizeof(Uint32));
		
		Uint32 *offsetTab;
		StHandleLocker lockOffsets(mItemOffsets, (void*&)offsetTab);
		
		Uint32 itemOffset = offsetTab[inIndex-1];
		UMemory::Insert((THdl)mData, itemOffset, nil, itemsSize);
		
		SMenuListView *menuData;
		StHandleLocker locker(mData, (void*&)menuData);
		UMemory::Clear(BPTR(menuData) + itemOffset, itemsSize);
		
		SMenuItem *mp = (SMenuItem *)(BPTR(menuData) + itemOffset);
		i = inCount;
		while(i--)
		{
			mp->script = 0;
			mp->titleLen = inSize;
			UMemory::Copy(mp->titleData, inText, inSize);
			mp = (SMenuItem *)(BPTR(mp) + dataSize);
		}
		
		menuData->itemCount += inCount;
		
		Uint32 *itemOffsetP = offsetTab + (inIndex-1);
		Uint32 *endOffset = offsetTab + menuData->itemCount + 1;
		mp = (SMenuItem *)(BPTR(menuData) + itemOffset);
				
		if (itemOffsetP < endOffset)
		{
			for(;;)
			{
				*itemOffsetP++ = BPTR(mp) - BPTR(menuData);
				if (itemOffsetP >= endOffset) break;
				
				mp = (SMenuItem *)(BPTR(mp) + sizeof(SMenuItem) + mp->titleLen);
			}
		}
				
		ItemsInserted(inIndex, inCount);
	}
	
	return inIndex;
}

Uint32 CMenuListView::InsertItem(Uint32 inAtItem, const void *inText, Uint32 inSize)
{
	return InsertItems(inAtItem, 1, inText, inSize);
}

Uint32 CMenuListView::AddItem(const void *inText, Uint32 inSize)
{
	return InsertItems(max_Uint32, 1, inText, inSize);
}

Uint32 CMenuListView::AddItem(const Uint8 inTitle[])
{
	return InsertItems(max_Uint32, 1, inTitle+1, inTitle[0]);
}

void CMenuListView::RemoveItems(Uint32 inIndex, Uint32 inCount)
{
	Uint32 itemCount;
	{
		SMenuListView *tempData;
		StHandleLocker tempLock(mData, (void*&)tempData);
		itemCount = tempData->itemCount;
	}
	
	Require(inIndex > 0 && inIndex <= itemCount);
	if ((inIndex-1) + inCount > itemCount)
		inCount = itemCount - (inIndex-1);
	if (inCount == 0) return;

	{
		Uint32 *sizeData;
		StHandleLocker lockOffsets(mItemOffsets, (void*&)sizeData);
		
		Uint32 s = sizeData[(inIndex + inCount) - 1] - sizeData[inIndex - 1];
		
		UMemory::Remove(mData, sizeData[inIndex-1], s);
		
		if (inIndex + inCount <= itemCount)
		{
			// **** this can be made more efficient
			for (Uint32 i = inIndex + inCount; i <= itemCount; i++)
				sizeData[i - inCount] = sizeData[i] - s;
		}
	}
	
	// ******* aren't we supposed to subtract something from the itemCount??
	// geez this sucks, rewrite into a UMenuData or something
	// (class for storing menu items)
	// have func to return the THdl and func to get offsets to menuitems
	// (for fast access when drawing etc)
	
	UMemory::SetSize(mItemOffsets, (itemCount + 1) * sizeof(Uint32));
	
	ItemsRemoved(inIndex, inCount);
}

void CMenuListView::RemoveItem(Uint32 inItem)
{
	RemoveItems(inItem, 1);
}

Uint32 CMenuListView::GetItemCount() const
{
	SMenuListView *tempData;
	StHandleLocker tempLock(mData, (void*&)tempData);
	return tempData->itemCount;
}

void CMenuListView::Clear()
{
	Uint32 nItemCount = CMenuListView::GetItemCount();
	
	UMemory::Dispose(mData);
	mData = nil;
	
	UMemory::Dispose(mItemOffsets);
	mItemOffsets = nil;

	try
	{
		mData = UMemory::NewHandleClear(sizeof(SMenuListView));
		mItemOffsets = UMemory::NewHandle(sizeof(Uint32));
		
		*(Uint32 *)UMemory::Lock(mItemOffsets) = sizeof(SMenuListView);
		UMemory::Unlock(mItemOffsets);
	}
	catch(...)
	{
		UMemory::Dispose((THdl)mData);
		UMemory::Dispose((THdl)mItemOffsets);
		throw;
	}

	ItemsRemoved(1, nItemCount);
}

Uint32 CMenuListView::InsertDivider(Uint32 inAtItem)
{
	return InsertItems(inAtItem, 1, "-", 1);
}

Uint32 CMenuListView::AddDivider()
{
	return InsertItems(max_Uint32, 1, "-", 1);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void CMenuListView::AddFontNameProc(const Uint8 *inName, Uint32 /* inEncoding */, Uint32 /* inFlags */, void *inRef)
{
	((CMenuListView *)inRef)->AddItem(inName);
}

void CMenuListView::AppendFontNames()
{
	UFontDesc::EnumFontNames(AddFontNameProc, this);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void CMenuListView::SetItemMark(Uint32 inItem, Char16 inMarkChar)
{
	SMenuListView *menuData;
	StHandleLocker lock(mData, (void*&)menuData);
	Uint32 *itemOffsets;
	StHandleLocker lockOffsets(mItemOffsets, (void*&)itemOffsets);
	
	Require(inItem != 0 && inItem <= menuData->itemCount);
	SMenuItem *mp = (SMenuItem *)(BPTR(menuData) + itemOffsets[inItem-1]);
	
	if (mp->markChar != inMarkChar)
	{
		mp->markChar = inMarkChar;
		RefreshItem(inItem);
	}
}

void CMenuListView::SetItemIcon(Uint32 inItem, Int16 inIconID)
{
	SMenuListView *menuData;
	StHandleLocker lock(mData, (void*&)menuData);
	Uint32 *itemOffsets;
	StHandleLocker lockOffsets(mItemOffsets, (void*&)itemOffsets);

	Require(inItem != 0 && inItem <= menuData->itemCount);
	SMenuItem *mp = (SMenuItem *)(BPTR(menuData) + itemOffsets[inItem-1]);
	
	if ((Int16)mp->iconID != inIconID)
	{
		mp->iconID = inIconID;
		RefreshItem(inItem);
	}
}

void CMenuListView::SetItemCommand(Uint32 inItem, Uint32 inCmdNum)
{
	SMenuListView *menuData;
	StHandleLocker lock(mData, (void*&)menuData);
	Uint32 *itemOffsets;
	StHandleLocker lockOffsets(mItemOffsets, (void*&)itemOffsets);

	Require(inItem != 0 && inItem <= menuData->itemCount);
	SMenuItem *mp = (SMenuItem *)(BPTR(menuData) + itemOffsets[inItem-1]);
	mp->cmdNum = inCmdNum;
}

void CMenuListView::SetItemRef(Uint32 inItem, Uint32 inRef)
{
	SMenuListView *menuData;
	StHandleLocker lock(mData, (void*&)menuData);
	Uint32 *itemOffsets;
	StHandleLocker lockOffsets(mItemOffsets, (void*&)itemOffsets);

	Require(inItem != 0 && inItem <= menuData->itemCount);
	SMenuItem *mp = (SMenuItem *)(BPTR(menuData) + itemOffsets[inItem-1]);
	mp->userRef = inRef;
}

void CMenuListView::SetItemEnable(Uint32 inItem, bool inEnable)
{
	SMenuListView *menuData;
	StHandleLocker lock(mData, (void*&)menuData);
	Uint32 *itemOffsets;
	StHandleLocker lockOffsets(mItemOffsets, (void*&)itemOffsets);

	Require(inItem != 0 && inItem <= menuData->itemCount);
	SMenuItem *mp = (SMenuItem *)(BPTR(menuData) + itemOffsets[inItem-1]);
	
	if (mp->isDisabled != !inEnable)
	{
		mp->isDisabled = !inEnable;
		RefreshItem(inItem);
	}
}

void CMenuListView::SetItemShortcut(Uint32 inItem, Uint16 inShortcut, bool inKeyCode)
{
	SMenuListView *menuData;
	StHandleLocker lock(mData, (void*&)menuData);
	Uint32 *itemOffsets;
	StHandleLocker lockOffsets(mItemOffsets, (void*&)itemOffsets);

	Require(inItem != 0 && inItem <= menuData->itemCount);
	SMenuItem *mp = (SMenuItem *)(BPTR(menuData) + itemOffsets[inItem-1]);
	
	mp->shortcut = inShortcut;
	mp->isKeycode = (inKeyCode != 0);
	mp->commandKey = true;
	mp->optionKey = mp->shiftKey = mp->controlKey = false;
	
	RefreshItem(inItem);
}

void CMenuListView::SetItemTick(Uint32 inItem, bool inTicked)
{
	SMenuListView *menuData;
	StHandleLocker lock(mData, (void*&)menuData);
	Uint32 *itemOffsets;
	StHandleLocker lockOffsets(mItemOffsets, (void*&)itemOffsets);

	Require(inItem != 0 && inItem <= menuData->itemCount);
	SMenuItem *mp = (SMenuItem *)(BPTR(menuData) + itemOffsets[inItem-1]);
	
	if (mp->isTicked != (inTicked!=0))
	{
		mp->isTicked = (inTicked!=0);
		RefreshItem(inItem);
	}
}

Char16 CMenuListView::GetItemMark(Uint32 inItem) const
{
	SMenuListView *menuData;
	StHandleLocker lock(mData, (void*&)menuData);
	Uint32 *itemOffsets;
	StHandleLocker lockOffsets(mItemOffsets, (void*&)itemOffsets);

	Require(inItem != 0 && inItem <= menuData->itemCount);
	SMenuItem *mp = (SMenuItem *)(BPTR(menuData) + itemOffsets[inItem-1]);
	return mp->markChar;
}

Int16 CMenuListView::GetItemIcon(Uint32 inItem) const
{
	SMenuListView *menuData;
	StHandleLocker lock(mData, (void*&)menuData);
	Uint32 *itemOffsets;
	StHandleLocker lockOffsets(mItemOffsets, (void*&)itemOffsets);

	Require(inItem != 0 && inItem <= menuData->itemCount);
	SMenuItem *mp = (SMenuItem *)(BPTR(menuData) + itemOffsets[inItem-1]);
	return mp->iconID;
}

Uint32 CMenuListView::GetItemCommand(Uint32 inItem) const
{
	SMenuListView *menuData;
	StHandleLocker lock(mData, (void*&)menuData);
	Uint32 *itemOffsets;
	StHandleLocker lockOffsets(mItemOffsets, (void*&)itemOffsets);

	Require(inItem != 0 && inItem <= menuData->itemCount);
	SMenuItem *mp = (SMenuItem *)(BPTR(menuData) + itemOffsets[inItem-1]);
	return mp->cmdNum;
}

Uint32 CMenuListView::GetItemRef(Uint32 inItem) const
{
	SMenuListView *menuData;
	StHandleLocker lock(mData, (void*&)menuData);
	Uint32 *itemOffsets;
	StHandleLocker lockOffsets(mItemOffsets, (void*&)itemOffsets);

	Require(inItem != 0 && inItem <= menuData->itemCount);
	SMenuItem *mp = (SMenuItem *)(BPTR(menuData) + itemOffsets[inItem-1]);
	return mp->userRef;
}

bool CMenuListView::IsItemEnabled(Uint32 inItem) const
{
	SMenuListView *menuData;
	StHandleLocker lock(mData, (void*&)menuData);
	Uint32 *itemOffsets;
	StHandleLocker lockOffsets(mItemOffsets, (void*&)itemOffsets);

	Require(inItem != 0 && inItem <= menuData->itemCount);
	SMenuItem *mp = (SMenuItem *)(BPTR(menuData) + itemOffsets[inItem-1]);
	return !mp->isDisabled;
}

bool CMenuListView::IsItemTicked(Uint32 inItem) const
{
	SMenuListView *menuData;
	StHandleLocker lock(mData, (void*&)menuData);
	Uint32 *itemOffsets;
	StHandleLocker lockOffsets(mItemOffsets, (void*&)itemOffsets);

	Require(inItem != 0 && inItem <= menuData->itemCount);
	SMenuItem *mp = (SMenuItem *)(BPTR(menuData) + itemOffsets[inItem-1]);
	return mp->isTicked;
}

void CMenuListView::SetItemFont(Uint32 inItem, Int16 inFontID, Uint16 inFontSize, Uint16 inFontStyle)
{
	SMenuListView *menuData;
	StHandleLocker lock(mData, (void*&)menuData);
	Uint32 *itemOffsets;
	StHandleLocker lockOffsets(mItemOffsets, (void*&)itemOffsets);

	Require(inItem != 0 && inItem <= menuData->itemCount);
	SMenuItem *mp = (SMenuItem *)(BPTR(menuData) + itemOffsets[inItem-1]);
	
	mp->fontID = inFontID;
	mp->fontSize = inFontSize;
	mp->fontStyle = inFontStyle;
	mp->useFont = true;
	mp->isTextStyle = false;
	
	RefreshItem(inItem);
}

void CMenuListView::SetItemTextStyle(Uint32 inItem, Int16 inID)
{
	SMenuListView *menuData;
	StHandleLocker lock(mData, (void*&)menuData);
	Uint32 *itemOffsets;
	StHandleLocker lockOffsets(mItemOffsets, (void*&)itemOffsets);

	Require(inItem != 0 && inItem <= menuData->itemCount);
	SMenuItem *mp = (SMenuItem *)(BPTR(menuData) + itemOffsets[inItem-1]);
	
	mp->fontID = inID;
	mp->useFont = true;
	mp->isTextStyle = true;
	
	RefreshItem(inItem);
}

Int16 CMenuListView::GetItemTextStyle(Uint32 inItem) const
{
	SMenuListView *menuData;
	StHandleLocker lock(mData, (void*&)menuData);
	Uint32 *itemOffsets;
	StHandleLocker lockOffsets(mItemOffsets, (void*&)itemOffsets);

	Require(inItem != 0 && inItem <= menuData->itemCount);
	SMenuItem *mp = (SMenuItem *)(BPTR(menuData) + itemOffsets[inItem-1]);
	
	if (mp->isTextStyle)
		return mp->fontID;
	
	return 0;
}

void CMenuListView::SetItemTitle(Uint32 inItem, const void *inText, Uint32 inSize)
{
	#pragma unused(inItem, inText, inSize)
	DebugBreak("CMenuListView - SetItemTitle() is not yet implemented");
	
	// *** to update offsets, simply recalc from item after inItem to end
	// *** (grab sizes from mData)
}

Uint32 CMenuListView::GetItemTitle(Uint32 inItem, void *outText, Uint32 inMaxSize) const
{
	SMenuListView *menuData;
	StHandleLocker lock(mData, (void*&)menuData);
	Uint32 *itemOffsets;
	StHandleLocker lockOffsets(mItemOffsets, (void*&)itemOffsets);

	Require(inItem != 0 && inItem <= menuData->itemCount);
	
	SMenuItem *mp = (SMenuItem *)(BPTR(menuData) + itemOffsets[inItem-1]);
		
	Uint32 s = mp->titleLen;
	if (s > inMaxSize) s = inMaxSize;
	
	UMemory::Copy(outText, mp->titleData, s);
	
	return s;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

Uint32 CMenuListView::GetFullWidth() const
{
	Uint32 i, n;
	SMenuItem *mp;
	Uint32 width, maxWidth;
	TImage img = UGraphics::GetDummyImage();
	
	void *menuData;
	StHandleLocker locker(mData, menuData);
	n = GetItemCount();
	maxWidth = 0;
	
	//UTextStyle::SetImageStyle(img, (**mData).textStyleID);
	
	Uint32 *itemOffsets;
	StHandleLocker lockOffsets(mItemOffsets, (void*&)itemOffsets);

	for (i=1; i<=n; i++)
	{
		mp = (SMenuItem *)(BPTR(menuData) + itemOffsets[i-1]);
		
		width = (mp->iconID ? 36 : 16) + UGraphics::GetTextWidth(img, mp->titleData, mp->titleLen) + 10;
		
		if (width > maxWidth)
			maxWidth = width;
	}
	
	return maxWidth;
}

void CMenuListView::Draw(TImage inImage, const SRect& inUpdateRect, Uint32 inDepth)
{
	UGraphics::Reset(inImage);
	//UTextStyle::SetImageStyle(inImage, (**mData).textStyleID);
	inImage->SetFont((TFontDesc)nil);
	CListView::Draw(inImage, inUpdateRect, inDepth);
}

// ****** this should become a utility func in UUserInterface
void CMenuListView::ItemDraw(Uint32 inItem, TImage inImage, const SRect& inBounds, const SRect& /* inUpdateRect */, Uint32 inOptions)
{
	SRect r;
	Uint8 c;
	
	SMenuListView *menuData;
	StHandleLocker locker(mData, (void*&)menuData);
	Uint32 *itemOffsets;
	StHandleLocker lockOffsets(mItemOffsets, (void*&)itemOffsets);

	SMenuItem *mp = (SMenuItem *)(BPTR(menuData) + itemOffsets[inItem-1]);
	
	bool isTop = (inItem == 1);
	bool isBottom = (inItem == menuData->itemCount);
	bool isDivider = (mp->titleLen == 1 && mp->titleData[0] == '-');
	bool isSelected = isDivider ? false : mSelectData.GetItem(inItem);
		
	SColor fillColor, lightColor, darkColor, titleColor;
	
	if (inOptions == kDrawItemForButton)
	{
		inImage->SetFont(nil);
#if MACINTOSH
		titleColor.Set(0x0000);
#else
		UUserInterface::GetSysColor(sysColor_Label, titleColor);
#endif
	}
	else if (inOptions == kDrawItemHilitedForButton)
	{
		inImage->SetFont(nil);
#if MACINTOSH
		titleColor.Set(0xFFFF);
#else
		UUserInterface::GetSysColor(sysColor_InverseHighlight, titleColor);
#endif
	}
	else if (inOptions == kDrawItemDisabledForButton)
	{
		inImage->SetFont(nil);
		titleColor.Set(0x7777);
	}
	else
	{
		// determine colors
		if (isSelected)
		{
#if MACINTOSH
			fillColor.Set(0x3333,0x3333,0x9999);
			titleColor.Set(0xFFFF,0xFFFF,0xFFFF);
			lightColor.Set(0x6666,0x6666,0xCCCC);
			darkColor.Set(0x0000,0x0000,0x8888);
#else
			UUserInterface::GetSysColor(sysColor_Highlight, fillColor);
			UUserInterface::GetSysColor(sysColor_InverseHighlight, titleColor);
#endif
		}
		else
		{
#if MACINTOSH
			fillColor.Set(0xDDDD);
			lightColor.Set(0xFFFF);
			darkColor.Set(0x9999);
			titleColor.Set(0x0000);
#else
			UUserInterface::GetSysColor(sysColor_Background, fillColor);
			UUserInterface::GetSysColor(sysColor_Label, titleColor);
#endif
		}
#if !WIN32 && !MACINTOSH
		lightColor = fillColor;
		lightColor.Lighten(0x3333);
		darkColor = fillColor;
		darkColor.Dark(0x3333);
#endif

		// draw fill
#if WIN32
		if (isSelected)
		{
			inImage->SetInkColor(fillColor);
			inImage->FillRect(inBounds);
		}
#else
		r.top = inBounds.top;
		r.bottom = inBounds.bottom;
		r.left = inBounds.left + 1;
		r.right = inBounds.right - 1;
		inImage->SetInkColor(fillColor);
		inImage->FillRect(r);
#endif
		
#if !WIN32
		// draw left and right shading
		r = inBounds;
		inImage->SetInkColor(lightColor);
		inImage->DrawLine(SLine(r.left,r.top,r.left,r.bottom));
		inImage->SetInkColor(darkColor);
		inImage->DrawLine(SLine(r.right-1,r.top,r.right-1,r.bottom));

		// draw top shading
		if (isTop)
		{
			inImage->SetInkColor(lightColor);
			inImage->DrawLine(SLine(r.left,r.top,r.right-1,r.top));
			
			inImage->SetPixel(SPoint(r.right-1,r.top), fillColor);
		}

		// draw bottom shading
		if (isBottom)
		{
			inImage->SetInkColor(darkColor);
			inImage->DrawLine(SLine(r.left+1,r.bottom-1,r.right-1,r.bottom-1));
			
			inImage->SetPixel(SPoint(r.left,r.bottom-1), fillColor);
		}
#endif
	}
	
	// draw title or divider
	if (isDivider)
	{
		// draw divider
		r.top = inBounds.top + (inBounds.GetHeight() / 2);
		r.left = inBounds.left;
		r.right = inBounds.right;
		inImage->SetInkColor(color_Gray8);
		inImage->DrawLine(SLine(r.left,r.top,r.right,r.top));
		inImage->SetInkColor(color_White);
		inImage->DrawLine(SLine(r.left,r.top+1,r.right,r.top+1));
	}
	else
	{
		// draw icon
		if (mp->iconID)
		{
			r.top = inBounds.top + 1;
			r.left = inBounds.left + 16;
			r.bottom = r.top + 16;
			r.right = r.left + 16;
			UIcon::Draw(mp->iconID, inImage, r, align_Center, transform_None);
		}

		// draw title
		r.top = inBounds.top;
		r.bottom = inBounds.bottom;
		r.left = inBounds.left + (mp->iconID ? 36 : 16);
		r.right = inBounds.right;
		inImage->SetInkColor(titleColor);
		UGraphics::DrawText(inImage, r, mp->titleData, mp->titleLen, 0, align_Left + align_CenterVert);
		
		// draw tick
		if (mp->isTicked)
		{
			r.left = r.right = inBounds.left + 3;
			//inImage->SetColor(SColor(0x0000,0x9999,0x0000));
			if (UGraphics::GetFontSize(inImage) <= 10)
			{
				// draw bullet
				c = 0xA5;
				UGraphics::DrawText(inImage, r, &c, sizeof(c), 0, align_Left + align_CenterVert);
			}
			else
			{
				// draw tick
				//c = 0x12;
				//Int16 saveFont = UGraphics::GetFont(inImage);
				//UGraphics::SetFont(inImage, 0);
				//UGraphics::DrawText(inImage, r, &c, sizeof(c));
				//UGraphics::SetFont(inImage, saveFont);
				
				// **** draw directly, don't use font
			}
		}
	}
}





