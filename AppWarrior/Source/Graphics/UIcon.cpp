/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */


#include "UIcon.h"



void UIcon_Draw(TIcon inIcon, TImage inImage, const SRect& inBounds, Uint16 inAlign, Uint16 inTransform);

void UIcon_Draw(Int32 inID, TImage inImage, const SRect& inBounds, Uint16 inAlign, Uint16 inTransform);



void _SetVirtualOrigin(TImage inImage, const SPoint& inVirtualOrigin);

void _GetVirtualOrigin(TImage inImage, SPoint& outVirtualOrigin);

void _ResetVirtualOrigin(TImage inImage);



/* -------------------------------------------------------------------------- */



void UIcon::Draw(TIcon inIcon, TImage inImage, const SRect& inBounds, Uint16 inAlign, Uint16 inTransform)

{

	// get virtual origin

	SPoint stVirtualOrigin;

	_GetVirtualOrigin(inImage, stVirtualOrigin);



	if (stVirtualOrigin.IsNull())

		UIcon_Draw(inIcon, inImage, inBounds, inAlign, inTransform);

	else

	{

		// reset virtual origin

		_ResetVirtualOrigin(inImage);



		// recalculate rect

		SRect stBounds = inBounds;

		stBounds.MoveBack(stVirtualOrigin.x, stVirtualOrigin.y);

	

		// draw

		UIcon_Draw(inIcon, inImage, stBounds, inAlign, inTransform);



		// restore virtual origin

		_SetVirtualOrigin(inImage, stVirtualOrigin);

	}

}



void UIcon::Draw(Int32 inID, TImage inImage, const SRect& inBounds, Uint16 inAlign, Uint16 inTransform)

{

	// get virtual origin

	SPoint stVirtualOrigin;

	_GetVirtualOrigin(inImage, stVirtualOrigin);



	if (stVirtualOrigin.IsNull())

		UIcon_Draw(inID, inImage, inBounds, inAlign, inTransform);

	else

	{

		// reset virtual origin

		_ResetVirtualOrigin(inImage);



		// recalculate rect

		SRect stBounds = inBounds;

		stBounds.MoveBack(stVirtualOrigin.x, stVirtualOrigin.y);

	

		// draw

		UIcon_Draw(inID, inImage, stBounds, inAlign, inTransform);



		// restore virtual origin

		_SetVirtualOrigin(inImage, stVirtualOrigin);

	}

}



