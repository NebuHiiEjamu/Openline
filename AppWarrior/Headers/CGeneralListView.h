/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"


#pragma mark CGeneralListView
template <class T> class CGeneralListView : public CListView
{
	public:
		CGeneralListView(CViewHandler *inHandler, const SRect &inBounds);
		virtual ~CGeneralListView();
	
		void SetItemsFromList(CPtrList<T> *inGeneralList);
		void GetItemsFromList(CPtrList<T> *outGeneralList);
				
		Uint32 AddItem(T *inItem, bool inSelect = true);
		bool InsertItem(Uint32 inIndex, T *inItem, bool inSelect = true);
		bool ModifyItem(Uint32 inIndex, T *inItem);
		bool DeleteItem(Uint32 inIndex);
 		void ClearList(bool inDisposeItems = true);
  
		CPtrList<T> *GetList()							{	return &mGeneralList;						}
		T *GetItem(Uint32 inIndex);
		Uint32 GetSelectedItem(T **outItem);
		Uint32 DeleteSelectedItem();
		Uint32 RefreshSelectedItem();

		virtual Uint32 GetItemCount() const				{	return mGeneralList.GetItemCount();			}
		virtual void GetFullSize(Uint32& outWidth, Uint32& outHeight) const;
		virtual Uint32 GetFullHeight() const;
		
		virtual void Draw(TImage inImage, const SRect& inUpdateRect, Uint32 inDepth);
				
	protected:
		virtual void ItemDraw(Uint32 inItem, TImage inImage, const SRect& inBounds, const SRect& inUpdateRect, Uint32 inOptions = 0);
				
		CPtrList<T> mGeneralList;
};


template<class T> CGeneralListView<T>::CGeneralListView(CViewHandler *inHandler, const SRect &inBounds)
	: CListView(inHandler, inBounds)
{
	mCellHeight = 19;
	mRefreshSelectedOnSetActive = true;
	
	mBehaviour = itemBehav_SelectOnlyOne | itemBehav_DoubleClickAction;
}

template<class T> CGeneralListView<T>::~CGeneralListView()
{
	ClearList();
}

template<class T> void CGeneralListView<T>::SetItemsFromList(CPtrList<T> *inGeneralList)
{
	if (!inGeneralList)
		return;
	
	Uint32 nIndex = mGeneralList.GetItemCount();
	
	T *pItem;
	Uint32 i = 0;
	
	while (inGeneralList->GetNext(pItem, i))
	{
		T *pNewItem = (T *)UMemory::New(sizeof(T));
		UMemory::Copy(pNewItem, pItem, sizeof(T));
		
		mGeneralList.AddItem(pNewItem);
	}
	
	Uint32 nItemCount = inGeneralList->GetItemCount();
	
	if (nItemCount)
	{
		ItemsInserted(nIndex, nItemCount);
		SelectItem(1);
	}
}

template<class T> void CGeneralListView<T>::GetItemsFromList(CPtrList<T> *outGeneralList)
{
	if (!outGeneralList)
		return;
	
	T *pItem = outGeneralList->GetItem(1);
	while (pItem)
	{
		outGeneralList->RemoveItem(pItem);
		UMemory::Dispose((TPtr)pItem);
		
		pItem = outGeneralList->GetItem(1);
	}

	Uint32 i = 0;
	while (mGeneralList.GetNext(pItem, i))
	{
		T *pNewItem = (T *)UMemory::New(sizeof(T));
		UMemory::Copy(pNewItem, pItem, sizeof(T));
		
		outGeneralList->AddItem(pNewItem);
	}
}

template<class T> Uint32 CGeneralListView<T>::AddItem(T *inItem, bool inSelect)
{
	if (!inItem)
		return 0;
	
	Uint32 nIndex = mGeneralList.AddItem(inItem);
	ItemsInserted(nIndex, 1);
	
	if (inSelect)
	{
		DeselectAll();
		SelectItem(nIndex);
		MakeItemVisible(nIndex);
	}
	
	return nIndex;
}

template<class T> bool CGeneralListView<T>::InsertItem(Uint32 inIndex, T *inItem, bool inSelect)
{
	if (inIndex < 1 || inIndex > mGeneralList.GetItemCount() || !inItem)
		return false;
	
	mGeneralList.InsertItem(inIndex, inItem);
	ItemsInserted(inIndex, 1);
	
	if (inSelect)
	{
		DeselectAll();
		SelectItem(inIndex);
		MakeItemVisible(inIndex);
	}
	
	return true;
}

template<class T> bool CGeneralListView<T>::ModifyItem(Uint32 inIndex, T *inItem)
{
	if (inIndex < 1 || inIndex > mGeneralList.GetItemCount() || !inItem)
		return false;

	T *pItem = mGeneralList.GetItem(inIndex);
	if (!pItem)
		return false;

	mGeneralList.RemoveItem(pItem);
	UMemory::Dispose((TPtr)pItem);

	mGeneralList.InsertItem(inIndex, inItem);
	RefreshItem(inIndex);
	
	return true;
}

template<class T> bool CGeneralListView<T>::DeleteItem(Uint32 inIndex)
{
	if (inIndex < 1 || inIndex > mGeneralList.GetItemCount())
		return false;

	T *pItem = mGeneralList.GetItem(inIndex);
	if (!pItem)
		return false;

	mGeneralList.RemoveItem(pItem);
	UMemory::Dispose((TPtr)pItem);
	
	ItemsRemoved(inIndex, 1);
	DeselectAll();
	
	Uint32 nItemCount = mGeneralList.GetItemCount();
	if (inIndex > nItemCount)
		inIndex = nItemCount;

	if (inIndex)
	{
		SelectItem(inIndex);
		MakeItemVisible(inIndex);
	}
		
	return true;
}

template<class T> void CGeneralListView<T>::ClearList(bool inDisposeItems)
{
	if (inDisposeItems)
	{
		Uint32 i = 0;
		T *pItem;
	
		while (mGeneralList.GetNext(pItem, i))
			UMemory::Dispose((TPtr)pItem);
	}

	Uint32 nItemCount = mGeneralList.GetItemCount();
	mGeneralList.Clear();
	
	if (nItemCount)
		ItemsRemoved(1, nItemCount);
}

template<class T> T *CGeneralListView<T>::GetItem(Uint32 inIndex)
{
	if (inIndex < 1 || inIndex > mGeneralList.GetItemCount())
		return nil;

	return mGeneralList.GetItem(inIndex);
}

template<class T> Uint32 CGeneralListView<T>::GetSelectedItem(T **outItem)
{
	Uint32 nSelectedItem = GetFirstSelectedItem();
	if (!nSelectedItem)
	{
		*outItem = nil;
		return 0;
	}
		
	*outItem = GetItem(nSelectedItem);
	if (!*outItem)
		return 0;

	return nSelectedItem;
}

template<class T> Uint32 CGeneralListView<T>::DeleteSelectedItem()
{
	Uint32 nSelectedItem = GetFirstSelectedItem();
	if (nSelectedItem)
		DeleteItem(nSelectedItem);
		
	return nSelectedItem;
}

template<class T> Uint32 CGeneralListView<T>::RefreshSelectedItem()
{
	Uint32 nSelectedItem = GetFirstSelectedItem();
	if (nSelectedItem)
		RefreshItem(nSelectedItem);

	return nSelectedItem;
}

template<class T> void CGeneralListView<T>::GetFullSize(Uint32& outWidth, Uint32& outHeight) const
{
	CListView::GetFullSize(outWidth, outHeight);

	if (GetItemCount())
		outHeight++;
}

template<class T> Uint32 CGeneralListView<T>::GetFullHeight() const
{
	Uint32 nFullHeight = CListView::GetFullHeight();
	
	if (GetItemCount())
		nFullHeight++;
		
	return nFullHeight;
}

template<class T> void  CGeneralListView<T>::Draw(TImage inImage, const SRect& inUpdateRect, Uint32 inDepth)
{
	UGraphics::SetFont(inImage, kDefaultFont, nil, 9);

#if MACINTOSH
	inImage->SetInkMode(mode_Or);
#endif

	CListView::Draw(inImage, inUpdateRect, inDepth);
}

template<class T> void  CGeneralListView<T>::ItemDraw(Uint32 inItem, TImage inImage, const SRect& inBounds, const SRect& inUpdateRect, Uint32 inOptions)
{
	if (inItem > mGeneralList.GetItemCount())
		return;

	SRect r;
	SColor stHiliteCol;
	bool bIsActive = IsFocus() && mIsEnabled;

	if (mSelectData.GetItem(inItem))
	{
		if (inOptions == kDrawItemForDrag)
			r = inBounds;
		else
			r.Set(inBounds.left + 1, inBounds.top + 1, inBounds.right - 1, inBounds.bottom);

		UUserInterface::GetHighlightColor(&stHiliteCol);
		inImage->SetInkColor(stHiliteCol);

		if (bIsActive)
			inImage->FillRect(r);
		else
			inImage->FrameRect(r);
	}

	// draw light lines around this item
	if (inOptions != kDrawItemForDrag)
	{
		UUserInterface::GetSysColor(sysColor_Light, stHiliteCol);
		inImage->SetInkColor(stHiliteCol);
		r = inBounds;
		r.bottom++;
		inImage->FrameRect(r);
	}

	CListView::ItemDraw(inItem, inImage, inBounds, inUpdateRect, inOptions);
}


/* ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ */
#pragma mark -

#pragma mark CGeneralCheckListView
template<class T> class CGeneralCheckListView : public CGeneralListView<T>
{
	public:
		CGeneralCheckListView(CViewHandler *inHandler, const SRect &inBounds);
		
		virtual Uint32 GetActiveCount();

		// mouse events
		virtual void MouseDown(const SMouseMsgData& inInfo);

		// key events
		virtual bool KeyDown(const SKeyMsgData& inInfo);

		// check events
		virtual void CheckChanged(Uint32 inIndex, Uint8 inActive);

	protected:
		virtual void ItemDraw(Uint32 inItem, TImage inImage, const SRect& inBounds, const SRect& inUpdateRect, Uint32 inOptions = 0);
				
		bool CheckAll(SPoint inPosition);
		bool CheckSelected(SPoint inPosition);
		void MarkSelected(Uint32 nSelectedItem);
};


template<class T> CGeneralCheckListView<T>::CGeneralCheckListView(CViewHandler *inHandler, const SRect &inBounds)
	: CGeneralListView(inHandler, inBounds)
{
}

template<class T> Uint32 CGeneralCheckListView<T>::GetActiveCount()
{
	Uint32 nActiveCount = 0;
	Uint32 nItemCount = mGeneralList.GetItemCount();

	for (Uint32 i = 1; i<=nItemCount; i++)
	{
		T *pItem = GetItem(i);
			
		if (pItem && pItem->nActive == 1)
			nActiveCount++;
	}

	return nActiveCount;
}

template<class T> void CGeneralCheckListView<T>::MouseDown(const SMouseMsgData& inInfo)
{
	if (!PointToItem(inInfo.loc)) 
		return;
	
	if ((inInfo.mods & modKey_Option) && CheckAll(inInfo.loc))
		return;
	
	if (inInfo.mods & kIsDoubleClick && CheckSelected(inInfo.loc))
		return;
	
	CGeneralListView::MouseDown(inInfo);
	
	CheckSelected(inInfo.loc);
}

template<class T> bool CGeneralCheckListView<T>::KeyDown(const SKeyMsgData& inInfo)
{
	bool bRet1 = false;
	if (inInfo.keyCode == key_Space)
	{
		bRet1 = true;
		Uint32 nSelectedItem = GetFirstSelectedItem();

		if (nSelectedItem)
			MarkSelected(nSelectedItem);
	}
	
	bool bRet2 = CGeneralListView::KeyDown(inInfo);

	return (bRet1 || bRet2);
}

// derived classes can provide implementation
template<class T> void CGeneralCheckListView<T>::CheckChanged(Uint32, Uint8){}

template<class T> void CGeneralCheckListView<T>::ItemDraw(Uint32 inItem, TImage inImage, const SRect& inBounds, const SRect& inUpdateRect, Uint32 inOptions)
{
	if (inItem > mGeneralList.GetItemCount())
		return;
	
	CGeneralListView::ItemDraw(inItem, inImage, inBounds, inUpdateRect, inOptions);
	
	T *pItem = GetItem(inItem);
	if(!pItem)
		return;
	
	// checkbox
	SCheckBoxInfo stCheckBox = (SCheckBoxInfo) { 0, pItem->nActive ? 1 : 0, nil, 0, 0, 0 };

	SRect rBounds = inBounds;
	rBounds.top += 3;
	rBounds.left += 3;
	rBounds.bottom -= 2;
	rBounds.right = rBounds.left + 16;

	UCheckBox::Draw(inImage, rBounds, stCheckBox);
}

template<class T> bool CGeneralCheckListView<T>::CheckAll(SPoint inPosition)
{
	if (inPosition.x >= 20)
		return false;

	Uint32 nSelectedItem = GetFirstSelectedItem();
	if (!nSelectedItem)
		return false;

	T *pItem = GetItem(nSelectedItem);
	if (!pItem)
		return false;
		
	Uint8 nDecision = !pItem->nActive ? 1 : 0;
	Uint32 nItemCount = mGeneralList.GetItemCount();

	for (Uint32 i = 1; i<=nItemCount; i++)
	{
		pItem = GetItem(i);
			
		if (pItem)
		{	
			pItem->nActive = nDecision;

			RefreshItem(i);			
			CheckChanged(i, pItem->nActive);
		}
	}
	
	return true;
}

template<class T> bool CGeneralCheckListView<T>::CheckSelected(SPoint inPosition)
{
	if (inPosition.x >= 20)
		return false;

	Uint32 nSelectedItem = GetFirstSelectedItem();
	if (!nSelectedItem)
		return false;

	if (mGeneralList.GetItemCount() != nSelectedItem)
	{
		MarkSelected(nSelectedItem);
		return true;
	}
		
	SRect theRect;
	GetSelectedItemsRect(theRect);
			
	if(theRect.Contains(inPosition))
	{
		MarkSelected(nSelectedItem);
		return true;
	}
	
	return false;
}

template<class T> void CGeneralCheckListView<T>::MarkSelected(Uint32 inSelectedItem)
{
	T *pItem = GetItem(inSelectedItem);
	if(!pItem)
		return;
	
	if (pItem->nActive)
		pItem->nActive = 0;
	else
		pItem->nActive = 1;
		
	RefreshItem(inSelectedItem);
	CheckChanged(inSelectedItem, pItem->nActive);
}

