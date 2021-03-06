/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"

enum {
	kReplaceIfExists	= 1		// for AddItem()
};

typedef class TIDVarArrayObj *TIDVarArray;

class UIDVarArray
{
	public:
		// new and dispose
		static TIDVarArray New();
		static void Dispose(TIDVarArray inRef);
		
		// add and remove items
		static void AddItem(TIDVarArray inRef, Uint32 inID, const void *inData, Uint32 inDataSize, Uint32 inOptions = 0);
		static void RemoveItem(TIDVarArray inRef, Uint32 inID);
		static Uint32 GetItemCount(TIDVarArray inRef);
		static bool ItemExists(TIDVarArray inRef, Uint32 inID);
		
		// set and get items
		static void SetItem(TIDVarArray inRef, Uint32 inID, const void *inData, Uint32 inDataSize);
		static Uint32 GetItem(TIDVarArray inRef, Uint32 inID, void *outData, Uint32 inMaxSize);
		static Uint32 GetItemSize(TIDVarArray inRef, Uint32 inID);
		static Uint32 ReadItem(TIDVarArray inRef, Uint32 inID, Uint32 inOffset, void *outData, Uint32 inMaxSize);
		static Uint32 WriteItem(TIDVarArray inRef, Uint32 inID, Uint32 inOffset, const void *inData, Uint32 inDataSize);
		
		// direct access to item data
		static Uint8 *GetItemPtr(TIDVarArray inRef, Uint32 inID, Uint32 *outSize = nil);
		static void ReleaseItemPtr(TIDVarArray inRef, Uint32 inID);

		// flattening
		static THdl FlattenToHandle(TIDVarArray inRef);
		static TIDVarArray Unflatten(const void *inData, Uint32 inDataSize);
		static Uint32 GetItem(const void *inData, Uint32 inDataSize, Uint32 inID, void *outData, Uint32 inMaxSize);
};

/*
 * Helpers
 */

class StIDVarArrayPtr
{
	public:
		StIDVarArrayPtr(TIDVarArray inRef, Uint32 inID, void*& outPtr, Uint32 *outSize = nil)	: mRef(inRef), mID(inID)	{	outPtr = UIDVarArray::GetItemPtr(inRef, inID, outSize);	}
		~StIDVarArrayPtr()																									{	UIDVarArray::ReleaseItemPtr(mRef, mID);					}
	
	protected:
		TIDVarArray mRef;
		Uint32 mID;
};

/*
 * UIDVarArray Object Interface
 */

class TIDVarArrayObj
{
	public:
		void AddItem(Uint32 inID, const void *inData, Uint32 inDataSize, Uint32 inOptions = 0)		{	UIDVarArray::AddItem(this, inID, inData, inDataSize, inOptions);			}
		void RemoveItem(Uint32 inID)																{	UIDVarArray::RemoveItem(this, inID);										}
		Uint32 GetItemCount()																		{	return UIDVarArray::GetItemCount(this);										}
		bool ItemExists(Uint32 inID)																{	return UIDVarArray::ItemExists(this, inID);									}
		
		void SetItem(Uint32 inID, const void *inData, Uint32 inDataSize)							{	UIDVarArray::SetItem(this, inID, inData, inDataSize);						}
		Uint32 GetItem(Uint32 inID, void *outData, Uint32 inMaxSize)								{	return UIDVarArray::GetItem(this, inID, outData, inMaxSize);				}
		Uint32 GetItemSize(Uint32 inID)																{	return UIDVarArray::GetItemSize(this, inID);								}
		Uint32 ReadItem(Uint32 inID, Uint32 inOffset, void *outData, Uint32 inMaxSize)				{	return UIDVarArray::ReadItem(this, inID, inOffset, outData, inMaxSize);		}
		Uint32 WriteItem(Uint32 inID, Uint32 inOffset, const void *inData, Uint32 inDataSize)		{	return UIDVarArray::WriteItem(this, inID, inOffset, inData, inDataSize);	}
		
		Uint8 *GetItemPtr(Uint32 inID, Uint32 *outSize = nil)										{	return UIDVarArray::GetItemPtr(this, inID, outSize);						}
		void ReleaseItemPtr(Uint32 inID)															{	UIDVarArray::ReleaseItemPtr(this, inID);									}

		THdl FlattenToHandle()																		{	return UIDVarArray::FlattenToHandle(this);									}
		
		void operator delete(void *p)																{	UIDVarArray::Dispose((TIDVarArray)p);										}
	protected:
		TIDVarArrayObj() {}				// force creation via UIDVarArray
};


