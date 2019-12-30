#if WIN32

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "UIcon.h"

/* -------------------------------------------------------------------------- */

void UIcon::Init()
{

}

void UIcon::Purge()
{

}

TIcon UIcon::Load(Int32 inID)
{
	StRezLoader *rz = new StRezLoader;
	THdl h;
	
	try
	{
		h = rz->Reload('ICON', inID);
	}
	catch(...)
	{
		delete rz;
		throw;
	}
	
	if (h == nil)
	{
		delete rz;
		return nil;
	}
	
	return (TIcon)rz;
}

void UIcon::Release(TIcon inIcon)
{
	if (inIcon) 
		delete (StRezLoader *)inIcon;
}

void UIcon_Draw(TIcon inIcon, TImage inImage, const SRect& inBounds, Uint16 inAlign, Uint16 inTransform)
{
	if (inIcon) 
		UPixmap::Draw(((StRezLoader *)inIcon)->GetHdl(), 0, inImage, inBounds, inAlign, inTransform);
}

void UIcon_Draw(Int32 inID, TImage inImage, const SRect& inBounds, Uint16 inAlign, Uint16 inTransform)
{
	TRez rz = URez::SearchChain('ICON', inID);
	
	if (rz)
	{
		THdl h = rz->LoadItem('ICON', inID, true);
		
		if (h)
		{
			StRezReleaser rel(rz, 'ICON', inID);
			UPixmap::Draw(h, 0, inImage, inBounds, inAlign, inTransform);
		}
	}
}

Uint16 UIcon::GetHeight(TIcon inIcon)
{
	//UPixmap::GetLayerSize(((StRezLoader *)inIcon)->GetHdl(), Uint32 inIndex, Uint32& outWidth, Uint32& outHeight);
	
	#pragma unused(inIcon)
	return 16;
}

Uint16 UIcon::GetWidth(TIcon inIcon)
{
	#pragma unused(inIcon)
	return 16;
}

Int32 UIcon::GetID(TIcon inIcon)
{
	if (inIcon == nil) return 0;
	return ((StRezLoader *)inIcon)->GetID();
}

#endif
