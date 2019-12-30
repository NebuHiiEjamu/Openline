/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "CLinkedList.h"

/* -------------------------------------------------------------------------- */

// add the link to the list at the front
void CLinkedList::AddFirst(CLink *ioLink)
{
	if (ioLink)
	{
		ioLink->mNext = mHead;
		mHead = ioLink;
	}
}

// add the link to the list at the end
void CLinkedList::AddLast(CLink *ioLink)
{
	if (ioLink)
	{
		ioLink->mNext = nil;
		
		CLink *p = mHead;
		if (p)
		{
			while (p->mNext)
				p = p->mNext;
			
			p->mNext = ioLink;
		}
		else
		{
			mHead = ioLink;
		}
	}
}

// remove the first link from the list
CLink *CLinkedList::RemoveFirst()
{
	CLink *p = mHead;
	if (p)
	{
		mHead = p->mNext;
		p->mNext = nil;
	}
	return p;
}

// remove the last link from the list
CLink *CLinkedList::RemoveLast()
{
	CLink *p = mHead;
	if (p)
	{
		while (p->mNext && p->mNext->mNext)
			p = p->mNext;
		
		CLink *tmp = p;
		p = p->mNext;
		
		if (p == nil)
		{
			mHead = nil;
			tmp->mNext = nil;
			return tmp;
		}
		 
		tmp->mNext = nil;
		
		p->mNext = nil;
	}
	return p;
}

// return the last link from the list
CLink *CLinkedList::GetLast() const
{
	CLink *p = mHead;
	if (p)
	{
		while (p->mNext)
			p = p->mNext;
	}
	return p;
}

// return true if the link is present in the list
bool CLinkedList::IsInList(const CLink *inLink) const
{
	CLink *p = mHead;
	
	while (p)
	{
		if (p == inLink) return true;
		p = p->mNext;
	}
	
	return false;
}

// remove the specified link from the list, returning true if it was found
bool CLinkedList::RemoveLink(CLink *ioLink)
{
	CLink *link = mHead;
	CLink *prev = nil;
	
	while (link)
	{
		if (link == ioLink)
		{
			if (prev)
				prev->mNext = link->mNext;
			else
				mHead = link->mNext;
			
			link->mNext = nil;
			return true;
		}
		
		prev = link;
		link = link->mNext;
	}
	
	return false;
}

// return the <inIndex>th link in the list (0-based)
CLink *CLinkedList::GetIndexedLink(Uint32 inIndex) const
{
	CLink *p = mHead;
	Uint32 i = 0;
	
	while (p)
	{
		if (i == inIndex) return p;
		
		p = p->mNext;
		i++;
	}
	
	return nil;
}

Uint32 CLinkedList::GetCount() const
{
	CLink *p = mHead;
	Uint32 n = 0;
	
	while (p)
	{
		p = p->mNext;
		n++;
	}
	
	return n;
}









