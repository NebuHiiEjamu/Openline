/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "MoreTypes.h"
#include "UFileSys.h"

/*
 * Types
 */

typedef class TRezObj *TRez;

/*
 * Constants
 */

enum {
	resAttr_Changed		= 0x80000000,
	resAttr_Validated	= 0x40000000
};

enum {
	kDontBroadcast				= 1,
	kChangeIfExists				= 2
};

/*
 * Structures
 */

#pragma options align=packed
struct SRezListItem {
	Int32 id;
	Uint32 size;
	Uint32 attrib;
	Uint16 nameSize;
	Uint8 nameData[];
};
#pragma options align=reset

struct SRezStub {
	TRez rez;
	THdl hdl;
	Uint32 type;
	Int32 id;
};

/*
 * Resource Manager
 */

class URez
{
	public:
		// construction
		static TRez New(TIOReadProc inReadProc, TIOWriteProc inWriteProc, TIOGetSizeProc inGetSizeProc, TIOSetSizeProc inSetSizeProc, TIOControlProc inControlProc, void *inRef, bool inCreateNew = false);
		static TRez NewFromFile(TFSRefObj* inFile, bool inCreateNew = false);
		static void Dispose(TRez inRef);
		static void SetRef(TRez inRef, void *inRefVal);
		static bool IsChanged(TRez inRef);
		
		// add and remove items
		static Int32 AddItem(TRez inRef, Uint32 inType, Int32 inID, THdl inHdl, const Uint8 *inName = nil, Uint32 inAttrib = 0, Uint32 inOptions = 0);
		static void RemoveItem(TRez inRef, Uint32 inType, Int32 inID);
		static void RemoveAllItems(TRez inRef, Uint32 inType);
		static bool ItemExists(TRez inRef, Uint32 inType, Int32 inID);

		// load handle
		static THdl LoadItem(TRez inRef, Uint32 inType, Int32 inID, Uint32 inNilIfNotFound = false);
		static void ReleaseItem(TRez inRef, Uint32 inType, Int32 inID);
		static void ChangedItem(TRez inRef, Uint32 inType, Int32 inID);
		static void SetItemHandle(TRez inRef, Uint32 inType, Int32 inID, THdl inHdl);
		
		// read data
		static Uint32 ReadItem(TRez inRef, Uint32 inType, Int32 inID, Uint32 inOffset, void *outData, Uint32 inMaxSize);
		static Uint32 GetItemSize(TRez inRef, Uint32 inType, Int32 inID);
		 
		// item properties
		static Uint32 GetItemName(TRez inRef, Uint32 inType, Int32 inID, void *outData, Uint32 inMaxSize);
		static void SetItemName(TRez inRef, Uint32 inType, Int32 inID, const void *inData, Uint32 inDataSize);
		static Uint32 GetItemAttributes(TRez inRef, Uint32 inType, Int32 inID);
		static void SetItemAttributes(TRez inRef, Uint32 inType, Int32 inID, Uint32 inAttrib);
		static void ChangeItemAttributes(TRez inRef, Uint32 inType, Int32 inID, Uint32 inWhichAttr, Uint32 inNewAttr);
		static void SetItemID(TRez inRef, Uint32 inType, Int32 inID, Uint32 inNewType, Int32 inNewID);
		static void SetItemInfo(TRez inRef, Uint32 inType, Int32 inID, Uint32 inNewType, Int32 inNewID, Uint32 inAttrib, const Uint8 *inName);
		
		// get type and item listings
		static THdl GetTypeListing(TRez inRef, Uint32 *outCount = nil, Uint32 inOptions = 0);
		static THdl GetItemListing(TRez inRef, Uint32 inType, Uint32 *outCount = nil, Uint32 inOptions = 0);
		static bool GetItemListNext(THdl inItemList, Uint32& ioOffset, Int32 *outID, Uint32 *outSize = nil, Uint32 *outAttrib = nil, Uint8 *outName = nil);
		
		// search chain
		static void AddSearchChainStart(TRez inRef, Uint32 inOptions = 0);
		static void AddSearchChainEnd(TRez inRef, Uint32 inOptions = 0);
		static TRez SearchChain(Uint32 inType, Int32 inID, Uint32 inOptions = 0);
		static void AddProgramFileToSearchChain(const Uint8 *inName);

		// misc
		static void Save(TRez inRef, Uint32 inOptions = 0);
		static Int32 GetLowestUnusedID(TRez inRef, Uint32 inType, Int32 inStartFrom = 100);
		static THdl Reload(SRezStub& ioStub, TRez inRef, Uint32 inType, Int32 inID, Uint32 inNilIfNotFound = false);
		static THdl Reload(SRezStub& ioStub, Uint32 inType, Int32 inID);
		static void SetStubHdl(SRezStub& ioStub, THdl inHdl);
};

/*
 * Stack Classes
 */

class StRezLoader : private SRezStub
{
	public:
		StRezLoader()																		
		{	rez = nil; hdl = nil; type = 0; id = 0;																}

		StRezLoader(TRez inRef, Uint32 inType, Int32 inID, THdl& outHdl)					
		{	
			outHdl = hdl = URez::LoadItem(inRef, inType, inID, false); 
			rez = inRef; 
			type = inType; 
			id = inID;	
		}

		~StRezLoader()																		
		{	URez::ReleaseItem(rez, type, id);																	}

		TRez GetRez() const																	
		{	return rez;																							}
		THdl GetHdl() const																	
		{	return hdl;																							}
		Uint32 GetType() const																
		{	return type;																						}
		Uint32 GetID() const																
		{	return id;																							}

		THdl Reload(TRez inRef, Uint32 inType, Int32 inID, Uint32 inNilIfNotFound = false)	
		{	
			return URez::Reload(*this, inRef, inType, inID, inNilIfNotFound);
		}
		THdl Reload(Uint32 inType, Int32 inID)												
		{	
			return URez::Reload(*this, inType, inID); 
		}
		void SetHdl(THdl inHdl)																
		{	
			URez::SetStubHdl(*this, inHdl);	
		}
};

class StRezReleaser
{
	public:
		StRezReleaser(TRez inRef, Uint32 inType, Int32 inID) : mRez(inRef), mType(inType), mID(inID)	{											}
		StRezReleaser()																					{	URez::ReleaseItem(mRez, mType, mID);	}
	private:
		TRez mRez;
		Uint32 mType;
		Int32 mID;
};

/*
 * Error Codes
 */

enum {
	errorType_Rez						= 6,
	rezError_Unknown					= 100,
	rezError_NoSuchResource				= rezError_Unknown + 1,
	rezError_Corrupt					= rezError_Unknown + 2,
	rezError_ResourceInUse				= rezError_Unknown + 3,
	rezError_ResourceAlreadyExists		= rezError_Unknown + 4
};

/*
 * URez Object Interface
 */

class TRezObj
{
	public:
		void SetRef(void *inRef)																									{	URez::SetRef(this, inRef);														}
		bool IsChanged()																											{	return URez::IsChanged(this);													}
		
		Int32 AddItem(Uint32 inType, Int32 inID, THdl inHdl, const Uint8 *inName = nil, Uint32 inAttrib = 0, Uint32 inOptions = 0)	{	return URez::AddItem(this, inType, inID, inHdl, inName, inAttrib, inOptions);	}
		void RemoveItem(Uint32 inType, Int32 inID)																					{	URez::RemoveItem(this, inType, inID);											}
		void RemoveAllItems(Uint32 inType)																							{	URez::RemoveAllItems(this, inType);												}
		bool ItemExists(Uint32 inType, Int32 inID)																					{	return URez::ItemExists(this, inType, inID);									}

		THdl LoadItem(Uint32 inType, Int32 inID, bool inNilIfNotFound = false)														{	return URez::LoadItem(this, inType, inID, inNilIfNotFound);						}
		void ReleaseItem(Uint32 inType, Int32 inID)																					{	URez::ReleaseItem(this, inType, inID);											}
		void ChangedItem(Uint32 inType, Int32 inID)																					{	URez::ChangedItem(this, inType, inID);											}
		void SetItemHandle(Uint32 inType, Int32 inID, THdl inHdl)																	{	URez::SetItemHandle(this, inType, inID, inHdl);									}
		
		Uint32 ReadItem(Uint32 inType, Int32 inID, Uint32 inOffset, void *outData, Uint32 inMaxSize)								{	return URez::ReadItem(this, inType, inID, inOffset, outData, inMaxSize);		}
		Uint32 GetItemSize(Uint32 inType, Int32 inID)																				{	return URez::GetItemSize(this, inType, inID);									}
				
		Uint32 GetItemName(Uint32 inType, Int32 inID, void *outData, Uint32 inMaxSize)												{	return URez::GetItemName(this, inType, inID, outData, inMaxSize);				}
		void SetItemName(Uint32 inType, Int32 inID, const void *inData, Uint32 inDataSize)											{	URez::SetItemName(this, inType, inID, inData, inDataSize);						}
		Uint32 GetItemAttributes(Uint32 inType, Int32 inID)																			{	return URez::GetItemAttributes(this, inType, inID);								}
		void SetItemAttributes(Uint32 inType, Int32 inID, Uint32 inAttrib)															{	URez::SetItemAttributes(this, inType, inID, inAttrib);							}
		void ChangeItemAttributes(Uint32 inType, Int32 inID, Uint32 inWhichAttr, Uint32 inNewAttr)									{	URez::ChangeItemAttributes(this, inType, inID, inWhichAttr, inNewAttr);			}
		void SetItemID(Uint32 inType, Int32 inID, Uint32 inNewType, Int32 inNewID)													{	URez::SetItemID(this, inType, inID, inNewType, inNewID);						}
		void SetItemInfo(Uint32 inType, Int32 inID, Uint32 inNewType, Int32 inNewID, Uint32 inAttrib, const Uint8 *inName)			{	URez::SetItemInfo(this, inType, inID, inNewType, inNewID, inAttrib, inName);	}
		
		THdl GetTypeListing(Uint32 *outCount = nil)																					{	return URez::GetTypeListing(this, outCount);									}
		THdl GetItemListing(Uint32 inType, Uint32 *outCount = nil)																	{	return URez::GetItemListing(this, inType, outCount);							}
		
		void AddSearchChainStart()																									{	URez::AddSearchChainStart(this);												}
		void AddSearchChainEnd()																									{	URez::AddSearchChainEnd(this);													}

		void Save()																													{	URez::Save(this);																}
		Int32 GetLowestUnusedID(Uint32 inType, Int32 inStartFrom = 100)																{	return URez::GetLowestUnusedID(this, inType, inStartFrom);						}
		
		void operator delete(void *p)																								{	URez::Dispose((TRez)p);															}
	protected:
		TRezObj() {}				// force creation via URez
};




