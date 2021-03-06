#if MACINTOSH

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "UIcon.h"

#include <Icons.h>

struct SIcon {
	Int16 id;
	Uint16 useCount;
	CIconHandle h;
};

static struct {
	SIcon **list;	// TPtr containing list of SIcon*
	Uint32 count, allocCount, releaseCount;
	Uint32 lastSearchIndex;
	Uint16 lastSearchID;
} _ICData = {0,0,0,0,0,0};

#define REF		((SIcon *)inIcon)

static bool _ICSearch(Uint16 inID, Uint32& outIndex);
static Uint32 _ICAdd(SIcon *inIcon);
static void _ICRemove(SIcon *inIcon);

GrafPtr _ImageToGrafPtr(TImage inImage);

/* -------------------------------------------------------------------------- */

void UIcon::Init()
{
	if (_ICData.list == nil)
	{
		_ICData.list = (SIcon **)UMemory::New(128);
		_ICData.allocCount = 32;
	}
}

void UIcon::Purge()
{
	//DebugBreak("UIcon - purging");
	if (_ICData.count == 0) return;
	
	SIcon **ep = _ICData.list - 1;
	SIcon **p = ep + _ICData.count;
	SIcon *icon;
	
	_ICData.lastSearchID = 0;
	
	while (p != ep)
	{
		icon = *p;
		if (icon->useCount == 0)
		{
			UMemory::Copy(p, p+1, ((_ICData.list + _ICData.count) - p - 1) * sizeof(Uint32));
			_ICData.count--;
			
			::DisposeCIcon(icon->h);
			UMemory::Dispose((TPtr)icon);
		}

		p--;
	}
}

// returns NIL if not found
TIcon UIcon::Load(Int32 inID)
{
	if (inID == 0) return nil;
	Init();
	
	Uint32 i;
	SIcon *icon = nil;
	CIconHandle h;
	
	if (_ICSearch(inID, i))
	{
		icon = _ICData.list[i];
		icon->useCount++;
	}
	else
	{
		h = ::GetCIcon(inID);
		if (h)
		{
			try
			{
				icon = (SIcon *)UMemory::New(sizeof(SIcon));
				icon->id = inID;
				icon->useCount = 1;
				icon->h = h;
				_ICAdd(icon);
			}
			catch(...)
			{
				::DisposeCIcon(h);
				UMemory::Dispose((TPtr)icon);
				throw;
			}
		}
	}

	return (TIcon)icon;
}

void UIcon::Release(TIcon inIcon)
{
	if (inIcon)
	{
		if (REF->useCount == 0)
		{
			DebugBreak("UIcon - attempt to release already released icon!");
			return;
		}
		
		REF->useCount--;
		
		if (REF->useCount == 0)
		{
			_ICData.releaseCount++;
			
			if (_ICData.releaseCount > 64)
			{
				_ICData.releaseCount = 0;
				Purge();
			}
		}
	}
}

void UIcon_Draw(TIcon inIcon, TImage inImage, const SRect& inBounds, Uint16 inAlign, Uint16 inTransform)
{
	/*
	 * PlotCIconHandle() uses CopyMask(), which doesn't go through the standard bottlenecks.
	 * This means it won't get recorded in pictures or printed.  However, there is an low-memory
	 * global that indicates that CopyBits() should be used (which does go through the
	 * bottlenecks):
	 *
	 *		*((Int16 *)0x0948) = 0;			// use CopyBits()
	 *		*((Int16 *)0x0948) = -1;		// use CopyMask()
	 */

	if (inIcon && inImage)
	{
		CIconHandle hIcon = REF->h;
		Rect r;
		SRect lr;
		Int16 trans;
		
		// determine rect to draw icon into
		r = (**hIcon).iconPMap.bounds;
		lr.top = inBounds.top;
		lr.left = inBounds.left;
		lr.bottom = lr.top + (r.bottom - r.top);
		lr.right = lr.left + (r.right - r.left);
		lr.Align(inAlign, inBounds);
		r.top = lr.top;
		r.left = lr.left;
		r.bottom = lr.bottom;
		r.right = lr.right;
		
		// determine transform
		switch (inTransform)
		{
			case transform_Dark:
				trans = kTransformSelected;
				break;
			case transform_Light:
				trans = kTransformDisabled;
				break;
			default:
				trans = kTransformNone;
				break;
		}

		// plot the icon
	#if TARGET_API_MAC_CARBON
		if (::GetQDGlobalsThePort() != _ImageToGrafPtr(inImage))
	#else
		if (qd.thePort != _ImageToGrafPtr(inImage)) 
	#endif
			::SetPort(_ImageToGrafPtr(inImage));
	
		PlotCIconHandle(&r, kAlignNone, trans, hIcon);
	}
}

void UIcon_Draw(Int32 inID, TImage inImage, const SRect& inBounds, Uint16 inAlign, Uint16 inTransform)
{
	if (inID)
	{
		try
		{
			TIcon icon = UIcon::Load(inID);
			UIcon_Draw(icon, inImage, inBounds, inAlign, inTransform);
			UIcon::Release(icon);
		}
		catch(...) {}
	}
}

Uint16 UIcon::GetHeight(TIcon inIcon)
{
	if (inIcon)
	{
		Rect& r = (**REF->h).iconPMap.bounds;
		return r.bottom - r.top;
	}

	return 0;
}

Uint16 UIcon::GetWidth(TIcon inIcon)
{
	if (inIcon)
	{
		Rect& r = (**REF->h).iconPMap.bounds;
		return r.right - r.left;
	}

	return 0;
}

Int32 UIcon::GetID(TIcon inIcon)
{
	if (inIcon) 
		return REF->id;

	return 0;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

// outIndex is 0-based
static bool _ICSearch(Uint16 inID, Uint32& outIndex)
{	
	if (_ICData.lastSearchID == inID)
	{
		outIndex = _ICData.lastSearchIndex;
		return true;
	}
	
	if (_ICData.count == 0)
	{
		outIndex = 0;
		return false;
	}

	SIcon **lookupTab = _ICData.list;
	Uint32 l, r, i;
	Uint16 id;

	id = lookupTab[0]->id;
	
	if (inID == id)
	{
		outIndex = 0;
		goto found;
	}
	else if (inID < id)
	{
		outIndex = 0;
		return false;
	}
	
	l = 1;
	r = _ICData.count - 1;
	
	if (l > r)
	{
		outIndex = 1;
		return false;
	}
	
	while (l <= r)
	{
		i = (l + r) >> 1;
		
		id = lookupTab[i]->id;

		if (inID == id)
		{
			outIndex = i;
			goto found;
		}
		else if (inID < id)
			r = i - 1;
		else
			l = i + 1;
	}
	
	if (inID > id)
		i++;
	
	outIndex = i;
	return false;
	
found:
	_ICData.lastSearchID = inID;
	_ICData.lastSearchIndex = outIndex;
	return true;
}

static Uint32 _ICAdd(SIcon *inIcon)
{
	Uint32 i;
	SIcon **p;
	
	if (_ICSearch(inIcon->id, i))
	{
		DebugBreak("UIcon - icon already registered!");
		Fail(errorType_Misc, error_Unknown);
	}
	
	if (_ICData.count >= _ICData.allocCount)
	{
		_ICData.list = (SIcon **)UMemory::Reallocate((TPtr)_ICData.list, (_ICData.allocCount+128) * sizeof(Uint32));
		_ICData.allocCount += 128;
	}

	p = _ICData.list + i;
	UMemory::Copy(p+1, p, (_ICData.count - i) * sizeof(Uint32));
	
	*p = inIcon;
	_ICData.count++;
	
	_ICData.lastSearchID = inIcon->id;
	_ICData.lastSearchIndex = i;

	return i;
}

static void _ICRemove(SIcon *inIcon)
{
	Uint32 i;
	SIcon **p;
	
	if (_ICSearch(inIcon->id, i))
	{
		p = _ICData.list + i;
		UMemory::Copy(p, p+1, (_ICData.count - i - 1) * sizeof(Uint32));
		_ICData.count--;
		
		if (_ICData.lastSearchIndex >= i)
			_ICData.lastSearchID = 0;
	}
}

#endif /* MACINTOSH */
