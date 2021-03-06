/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "UDragAndDrop.h"

class CDragAndDroppable;

class CDragAndDropHandler
{
	public:
		virtual void HandleDrag(CDragAndDroppable *inDD, const SMouseMsgData& inInfo);
		virtual void HandleSetDragAction(CDragAndDroppable *inDD, const SDragMsgData& inInfo);
		virtual bool HandleDrop(CDragAndDroppable *inDD, const SDragMsgData& inInfo);
};

class CDragAndDroppable
{
	public:
		CDragAndDroppable();
		CDragAndDroppable(CDragAndDropHandler *inHandler);
		
		virtual void SetDragAndDropHandler(CDragAndDropHandler *inHandler);
		CDragAndDropHandler *GetDragAndDropHandler() const;
		
	protected:
		CDragAndDropHandler *mDragAndDropHandler;
};


