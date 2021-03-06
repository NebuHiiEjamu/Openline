/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "GrafTypes.h"
#include "URegion.h"
#include "UGraphics.h"

enum {
	flavorFlag_SenderOnly			= 0x00000001L,			// flavor is available to sender only
	flavorFlag_SenderTranslated		= 0x00000002L,			// flavor is translated by sender
	flavorFlag_NotSaved				= 0x00000004L,			// flavor should not be saved
	flavorFlag_SystemTranslated		= 0x00000100L			// flavor is translated by system
};

enum {
	dragFlag_LeftSenderWindow		= 0x00000001L,			// drag has left the source window since Track()
	dragFlag_InsideSenderApp		= 0x00000002L,			// drag is occurring within the sender application
	dragFlag_InsideSenderWindow		= 0x00000004L			// drag is occurring within the sender window
};

enum {
	dragImageStyle_Standard			= 0,
	dragImageStyle_Dark				= 1,
	dragImageStyle_Darker			= 2,
	dragImageStyle_Opaque			= 3,
	dragImageStyle_RegionAndImage	= 16
};

// a mouse enter must SetDragAction
enum
{
	dragAction_None		= 0,
	dragAction_Copy		= 1,
	dragAction_Move		= 2,
	dragAction_Link		= 3
};

enum {
	flavor_File			= 'hfs '
#if WIN32
	, flavor_WFile		= 15	
#endif
};

typedef class TDragObj *TDrag;
//typedef class TFSRefObj *TFSRefObj*;

// outDropEnd is set true by the proc to show that the processing is complete
// it is used generally for files but must be set by any proc
typedef void (*TDragSendProc)(TDrag inDrag, Uint32 inItem, Uint32 inFlavor, void *inRef, bool *outDropEnd = nil, bool *outDropError = nil);

struct SDragMsgData {
	Uint32 time;
	SPoint loc;
	Uint16 button;
	Uint16 mods;
	TDrag drag;
};

class UDragAndDrop
{
	public:
		// create and dispose drag references
		static void Init();
		static bool IsAvailable();
		static TDrag New();
		static void Dispose(TDrag inRef);
		
		// tracking
		static bool Track(TDrag inRef, const SPoint& inMouseLoc, TRegion inRgn);
		static void AcceptDrop(TDrag inRef);
		static void RejectDrop(TDrag inRef);
		
		// items
		static Uint32 GetItemCount(TDrag inRef);
		static Uint32 GetFlavourCount(TDrag inRef, Uint32 inIndex);
		static Uint32 GetItem(TDrag inRef, Uint32 inIndex);
		static void SetItemBounds(TDrag inRef, Uint32 inItem, const SRect& inBounds);
		static void GetItemBounds(TDrag inRef, Uint32 inItem, SRect& outBounds);

		// add flavors
		static bool HasFlavor(TDrag inRef, Uint32 inItem, Uint32 inFlavor);
		static bool HasFlavor(TDrag inRef, Uint32 inFlavor);
		static void AddFlavor(TDrag inRef, Uint32 inItem, Uint32 inFlavor, const void *inData, Uint32 inDataSize, Uint32 inFlags = 0);
		static void SetFlavorData(TDrag inRef, Uint32 inItem, Uint32 inFlavor, const void *inData, Uint32 inDataSize, Uint32 inOffset = 0);
		static void AddImageFlavor(TDrag inRef, Uint32 inItem, TImage inImage, const SRect *inRect = nil, Uint32 inFlags = 0);
		static void AddTextFlavor(TDrag inRef, Uint32 inItem, const void *inText, Uint32 inTextSize, Uint32 inEncoding = 0, Uint32 inFlags = 0);

		// get flavors
		static Uint32 GetFlavorData(TDrag inRef, Uint32 inItem, Uint32 inFlavor, void *outData, Uint32 inMaxSize, Uint32 inOffset = 0);
		static Uint32 GetFlavorDataSize(TDrag inRef, Uint32 inItem, Uint32 inFlavor);
		static Uint32 GetFlavorFlags(TDrag inRef, Uint32 inItem, Uint32 inFlavor);
		static Uint32 GetFlavorCount(TDrag inRef, Uint32 inItem);
		static Uint32 GetFlavor(TDrag inRef, Uint32 inItem, Uint32 inIndex);
		
		// file system flavors
		static TFSRefObj* GetFileSysFlavor(TDrag inRef, Uint32 inItem);
		static Uint32 GetFileSysFlavorTypeCode(TDrag inRef, Uint32 inItem);
		static void AddPromisedFileSysFlavor(TDrag inRef, Uint32 inItem, const Int8 *inFileType, Uint32 inFlags = 0);
		static TFSRefObj* GetDropFolder(TDrag inRef);
		static void CreatedPromisedFile(TDrag inRef, Uint32 inItem, TFSRefObj* inFSRef);

		// properties
		static Uint32 GetAttributes(TDrag inRef);
		static void GetMouse(TDrag inRef, SPoint& outMouse, SPoint& outPinnedMouse);
		static void GetMouse(TDrag inRef, SPoint& outMouse);
		static void SetMouse(TDrag inRef, const SPoint& inPinnedMouse);
		static void GetOrigin(TDrag inRef, SPoint& outInitialMouse);
		static Int16 GetModifiers(TDrag inRef);
		static void GetModifiers(TDrag inRef, Int16 *outModifiers, Int16 *outMouseDownModifiers = nil, Int16 *outMouseUpModifiers = nil);
		static bool DropLocationIsTrash(TDrag inRef);
		static void SetImage(TDrag inRef, TImage inImage, TRegion inRgn, const SPoint& inImageOffset, Uint32 inStyle = dragImageStyle_Standard);
		static void SetDragAction(TDrag inRef, Uint16 inAction);
		static Uint16 GetDragAction(TDrag inRef);
		
		// callback procs
		static void SetSendHandler(TDrag inRef, TDragSendProc inProc, void *inSendRef = nil);
		
		// hiliting
		static void DrawHilite(TImage inImage, const SRect& inRect, const SColor& inColor);
		static void DrawHilite(TImage inImage, TRegion inRgn, const SColor& inColor);
		static void DrawHilite(TImage inImage, const SRect& inRect);
		static void DrawHilite(TImage inImage, TRegion inRgn);
		
		// misc
		static bool HasTranslucentDrag();
		static bool IsTracking();
		
		static bool Equals(TDrag inRef, TDrag inCompare);
};

// drag-and-drop errors
enum {
	errorType_DragAndDrop				= 19,
	dragError_Unknown					= 100,
	dragError_NotAvailable				= dragError_Unknown + 1
};

// object interface
class TDragObj
{
	public:
		bool Track(const SPoint& inMouseLoc, TRegion inRgn)																	{	return UDragAndDrop::Track(this, inMouseLoc, inRgn);										}
		void AcceptDrop()																									{	UDragAndDrop::AcceptDrop(this);																}
		void RejectDrop()																									{	UDragAndDrop::RejectDrop(this);																}
		
		Uint32 GetItemCount()																								{	return UDragAndDrop::GetItemCount(this);													}
		Uint32 GetFlavourCount(Uint32 inIndex)																				{	return UDragAndDrop::GetFlavourCount(this, inIndex);										}
		Uint32 GetItem(Uint32 inIndex)																						{	return UDragAndDrop::GetItem(this, inIndex);												}
		void SetItemBounds(Uint32 inItem, const SRect& inBounds)															{	UDragAndDrop::SetItemBounds(this, inItem, inBounds);										}
		void GetItemBounds(Uint32 inItem, SRect& outBounds)																	{	UDragAndDrop::GetItemBounds(this, inItem, outBounds);										}

		bool HasFlavor(Uint32 inItem, Uint32 inFlavor)																		{	return UDragAndDrop::HasFlavor(this, inItem, inFlavor);										}
		bool HasFlavor(Uint32 inFlavor)																						{	return UDragAndDrop::HasFlavor(this, inFlavor);												}
		void AddFlavor(Uint32 inItem, Uint32 inFlavor, const void *inData, Uint32 inDataSize, Uint32 inFlags = 0)			{	UDragAndDrop::AddFlavor(this, inItem, inFlavor, inData, inDataSize, inFlags);				}
		void SetFlavorData(Uint32 inItem, Uint32 inFlavor, const void *inData, Uint32 inDataSize, Uint32 inOffset = 0)		{	UDragAndDrop::SetFlavorData(this, inItem, inFlavor, inData, inDataSize, inOffset);			}
		void AddImageFlavor(Uint32 inItem, TImage inImage, const SRect *inRect = nil, Uint32 inFlags = 0)					{	UDragAndDrop::AddImageFlavor(this, inItem, inImage, inRect, inFlags);						}
		void AddTextFlavor(Uint32 inItem, const void *inText, Uint32 inTextSize, Uint32 inEncoding = 0, Uint32 inFlags = 0)	{	UDragAndDrop::AddTextFlavor(this, inItem, inText, inTextSize, inEncoding, inFlags);			}

		Uint32 GetFlavorData(Uint32 inItem, Uint32 inFlavor, void *outData, Uint32 inMaxSize, Uint32 inOffset = 0)			{	return UDragAndDrop::GetFlavorData(this, inItem, inFlavor, outData, inMaxSize, inOffset);	}
		Uint32 GetFlavorDataSize(Uint32 inItem, Uint32 inFlavor)															{	return UDragAndDrop::GetFlavorDataSize(this, inItem, inFlavor);								}
		Uint32 GetFlavorFlags(Uint32 inItem, Uint32 inFlavor)																{	return UDragAndDrop::GetFlavorFlags(this, inItem, inFlavor);								}
		Uint32 GetFlavorCount(Uint32 inItem)																				{	return UDragAndDrop::GetFlavorCount(this, inItem);											}
		Uint32 GetFlavor(Uint32 inItem, Uint32 inIndex)																		{	return UDragAndDrop::GetFlavor(this, inItem, inIndex);										}

		TFSRefObj* GetFileSysFlavor(Uint32 inItem)																				{	return UDragAndDrop::GetFileSysFlavor(this, inItem);										}
		Uint32 GetFileSysFlavorTypeCode(Uint32 inItem)																		{	return UDragAndDrop::GetFileSysFlavorTypeCode(this, inItem);								}
		void AddPromisedFileSysFlavor(Uint32 inItem, const Int8 *inFileType, Uint32 inFlags = 0)							{	UDragAndDrop::AddPromisedFileSysFlavor(this, inItem, inFileType, inFlags);					}
		TFSRefObj* GetDropFolder()																								{	return UDragAndDrop::GetDropFolder(this);													}
		void CreatedPromisedFile(Uint32 inItem, TFSRefObj* inFSRef)																{	UDragAndDrop::CreatedPromisedFile(this, inItem, inFSRef);									}

		Uint32 GetAttributes()																								{	return UDragAndDrop::GetAttributes(this);													}
		void GetMouse(SPoint& outMouse, SPoint& outPinnedMouse)																{	UDragAndDrop::GetMouse(this, outMouse, outPinnedMouse);										}
		void GetMouse(SPoint& outMouse)																						{	UDragAndDrop::GetMouse(this, outMouse);														}
		void SetMouse(const SPoint& inPinnedMouse)																			{	UDragAndDrop::SetMouse(this, inPinnedMouse);												}
		void GetOrigin(SPoint& outInitialMouse)																				{	UDragAndDrop::GetOrigin(this, outInitialMouse);												}
		Int16 GetModifiers()																								{	return UDragAndDrop::GetModifiers(this);													}
		void GetModifiers(Int16 *outModifiers, Int16 *outMouseDownModifiers = nil, Int16 *outMouseUpModifiers = nil)		{	UDragAndDrop::GetModifiers(this, outModifiers, outMouseDownModifiers, outMouseUpModifiers);	}
		bool DropLocationIsTrash()																							{	return UDragAndDrop::DropLocationIsTrash(this);												}
		void SetImage(TImage inImage, TRegion inRgn, const SPoint& inImageOffset, Uint32 inStyle = dragImageStyle_Standard)	{	UDragAndDrop::SetImage(this, inImage, inRgn, inImageOffset, inStyle);						}
		void SetDragAction(Uint16 inAction)																					{	UDragAndDrop::SetDragAction(this, inAction);												}
		Uint16 GetDragAction()																								{	return UDragAndDrop::GetDragAction(this);													}
		
		void SetSendHandler(TDragSendProc inProc, void *inRef = nil)														{	UDragAndDrop::SetSendHandler(this, inProc, inRef);											}

		bool Equals(TDrag inCompare)																						{	return UDragAndDrop::Equals(this, inCompare);												}

		void operator delete(void *p)																						{	UDragAndDrop::Dispose((TDrag)p);															}
	protected:
		TDragObj() {}		// force creation via UDragAndDrop
};

class StDrag
{
	public:
		StDrag()						{	mRef = UDragAndDrop::New();		}
		~StDrag()						{	UDragAndDrop::Dispose(mRef);	}
		operator TDrag()				{	return mRef;					}
		TDragObj *operator->() const	{	return mRef;					}

	private:
		TDrag mRef;
};

