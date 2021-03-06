/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"


#pragma mark CGeneralTabbedListView
template <class T> class CGeneralTabbedListView : public CTabbedListView
{
	public:
		CGeneralTabbedListView(CViewHandler *inHandler, const SRect &inBounds, bool inShowTabLine = false);
		virtual ~CGeneralTabbedListView();
	
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


template<class T> CGeneralTabbedListView<T>::CGeneralTabbedListView(CViewHandler *inHandler, const SRect &inBounds, bool inShowTabLine)
	: CTabbedListView(inHandler, inBounds, inShowTabLine)
{
	mCellHeight = 19;
	mRefreshSelectedOnSetActive = true;
	
	mBehaviour = itemBehav_SelectOnlyOne | itemBehav_DoubleClickAction;
}

template<class T> CGeneralTabbedListView<T>::~CGeneralTabbedListView()
{
	ClearList();
}

template<class T> void CGeneralTabbedListView<T>::SetItemsFromList(CPtrList<T> *inGeneralList)
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

template<class T> void CGeneralTabbedListView<T>::GetItemsFromList(CPtrList<T> *outGeneralList)
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

template<class T> Uint32 CGeneralTabbedListView<T>::AddItem(T *inItem, bool inSelect)
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

template<class T> bool CGeneralTabbedListView<T>::InsertItem(Uint32 inIndex, T *inItem, bool inSelect)
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

template<class T> bool CGeneralTabbedListView<T>::ModifyItem(Uint32 inIndex, T *inItem)
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

template<class T> bool CGeneralTabbedListView<T>::DeleteItem(Uint32 inIndex)
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

template<class T> void CGeneralTabbedListView<T>::ClearList(bool inDisposeItems)
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

template<class T> T *CGeneralTabbedListView<T>::GetItem(Uint32 inIndex)
{
	if (inIndex < 1 || inIndex > mGeneralList.GetItemCount())
		return nil;

	return mGeneralList.GetItem(inIndex);
}

template<class T> Uint32 CGeneralTabbedListView<T>::GetSelectedItem(T **outItem)
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

template<class T> Uint32 CGeneralTabbedListView<T>::DeleteSelectedItem()
{
	Uint32 nSelectedItem = GetFirstSelectedItem();
	if (nSelectedItem)
		DeleteItem(nSelectedItem);
		
	return nSelectedItem;
}

template<class T> Uint32 CGeneralTabbedListView<T>::RefreshSelectedItem()
{
	Uint32 nSelectedItem = GetFirstSelectedItem();
	if (nSelectedItem)
		RefreshItem(nSelectedItem);

	return nSelectedItem;
}

template<class T> void CGeneralTabbedListView<T>::GetFullSize(Uint32& outWidth, Uint32& outHeight) const
{
	CTabbedListView::GetFullSize(outWidth, outHeight);

	if (GetItemCount())
		outHeight++;
}

template<class T> Uint32 CGeneralTabbedListView<T>::GetFullHeight() const
{
	Uint32 nFullHeight = CTabbedListView::GetFullHeight();
	
	if (GetItemCount())
		nFullHeight++;
		
	return nFullHeight;
}

template<class T> void  CGeneralTabbedListView<T>::Draw(TImage inImage, const SRect& inUpdateRect, Uint32 inDepth)
{
	UGraphics::SetFont(inImage, kDefaultFont, nil, 9);

#if MACINTOSH
	inImage->SetInkMode(mode_Or);
#endif

	CTabbedListView::Draw(inImage, inUpdateRect, inDepth);
}

template<class T> void  CGeneralTabbedListView<T>::ItemDraw(Uint32 inItem, TImage inImage, const SRect& inBounds, const SRect& inUpdateRect, Uint32 inOptions)
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

	CTabbedListView::ItemDraw(inItem, inImage, inBounds, inUpdateRect, inOptions);
}
