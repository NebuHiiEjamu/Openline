/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
/**********************************************************************
The flavors should really be MIME types like in UClipboard, not Uint32 codes.
***********************************************************************/


#if MACINTOSH

/*
Functions for working with Drag & Drop - the ability to "pick up" an object,
drag it somewhere else, and drop it.  The object is then moved or copied to its
new location.

A drag reference consists of a number of items that are being dragged.  Each
item comes with data in one or more "flavors".  For example, you might drag
three files, which would be three items, and each item might have a file system
flavor and a text flavor of the file name.
*/

#include "UDragAndDrop.h"
#include "UMemory.h"
#include "UError.h"

#include <Drag.h>
#include <Windows.h>
#include <QDOffscreen.h>

//#define kRootFolder GETLOST3453342
#include <Folders.h>
//#undef kRootFolder

#undef SetItem
#undef GetItem

#if TARGET_API_MAC_CARBON
	#define WEAK_IMPORT_TEST(sym)	(true)
#else
	#include <CodeFragments.h>
	#define WEAK_IMPORT_TEST(sym)	((sym) != (void *)kUnresolvedCFragSymbolAddress)
#endif

/*
 * Structures
 */

const Int32 kPrivateDataType = '%_pv';

struct SExtraInfo {
	void *sendProcRef;
};

struct SDragInfo
{
	DragReference drag;
	Uint16 dragAction;
};

//#define REF ((SDragInfo *)inRef)

/*
 * Function Prototypes
 */

static bool _DDGetPrivateData(TDrag inRef, SExtraInfo& outInfo);
static bool _DDGetPrivateData(TDrag inRef, Uint32 inOffset, void *outData, Uint32 inSize);
static void _DDSetPrivateData(TDrag inRef, const SExtraInfo& inInfo);
static void _DDMakePrivateData(TDrag inRef, SExtraInfo& outInfo);
static pascal OSErr _DDDragReceiveProc(WindowPtr inWin, void *inRef, DragReference inDrag);
static pascal OSErr _DDDragTrackingProc(DragTrackingMessage inMsg, WindowPtr inWin, void *inRef, DragReference inDrag);
static pascal OSErr _DDDragSendProc(FlavorType inType, void *inRef, ItemReference inItem, DragReference inDrag);
static bool _DDHasTranslucentDrag();

Uint16 _MacModsToStd(Uint16 mods);
TFSRefObj* _FSSpecToRef(const FSSpec& inSpec);
void _MIMEToMacTypeCode(const void *inText, Uint32 inSize, Uint32& outTypeCode, Uint32& outCreatorCode);
void _WNDispatchDrag(Uint32 inType, const SDragMsgData& inInfo, WindowPtr inWin);
Int16 _ConvertMacError(const SError& inErr);
GrafPtr _ImageToGrafPtr(TImage inImage);

/*
 * Global Variables
 */

static struct {
	Point dragLoc;
	Uint16 isInitted	: 1;
	Uint16 hasTransDrag	: 1;
	Uint16 isTracking	: 1;
	Uint16 dropAccepted	: 1;
	Uint16 isAvailable	: 1;
	Uint16 checkedAvail	: 1;
} _DDData = {{-30000,-30000},0,0,0,0,0,0};

#if TARGET_API_MAC_CARBON
	static DragTrackingHandlerUPP gTrackingUPP = ::NewDragTrackingHandlerUPP(_DDDragTrackingProc);
	static DragReceiveHandlerUPP gReceiveUPP =  ::NewDragReceiveHandlerUPP(_DDDragReceiveProc);
	static DragSendDataUPP gSendUPP = ::NewDragSendDataUPP(_DDDragSendProc);
#else
	static RoutineDescriptor _DDTrackingRD	= BUILD_ROUTINE_DESCRIPTOR(uppDragTrackingHandlerProcInfo, _DDDragTrackingProc);
	static RoutineDescriptor _DDReceiveRD	= BUILD_ROUTINE_DESCRIPTOR(uppDragReceiveHandlerProcInfo, _DDDragReceiveProc);
	static RoutineDescriptor _DDSendRD		= BUILD_ROUTINE_DESCRIPTOR(uppDragSendDataProcInfo, _DDDragSendProc);

	#define gTrackingUPP	((DragTrackingHandlerUPP)&_DDTrackingRD)
	#define gReceiveUPP		((DragReceiveHandlerUPP)&_DDReceiveRD)
	#define gSendUPP		((DragSendDataUPP)&_DDSendRD)
#endif

// mac error handling
void _FailMacError(Int16 inMacError, const Int8 *inFile, Uint32 inLine);
inline void _CheckMacError(Int16 inMacError, const Int8 *inFile, Uint32 inLine) { if (inMacError) _FailMacError(inMacError, inFile, inLine); }
#if DEBUG
	#define FailMacError(id)		_FailMacError(id, __FILE__, __LINE__)
	#define CheckMacError(id)		_CheckMacError(id, __FILE__, __LINE__)
#else
	#define FailMacError(id)		_FailMacError(id, nil, 0)
	#define CheckMacError(id)		_CheckMacError(id, nil, 0)
#endif

/* -------------------------------------------------------------------------- */

void UDragAndDrop::Init()
{
	if (!_DDData.isInitted)
	{
		// fail if drag-and-drop is not available
		if (!IsAvailable())
			Fail(errorType_DragAndDrop, dragError_NotAvailable);
	
		// check whether or not translucent dragging is supported
		_DDData.hasTransDrag = _DDHasTranslucentDrag();
		
		// install drag & drop handlers	
		::RemoveTrackingHandler(gTrackingUPP, nil);
		::RemoveReceiveHandler(gReceiveUPP, nil);
		CheckMacError(::InstallTrackingHandler(gTrackingUPP, nil, nil));
		CheckMacError(::InstallReceiveHandler(gReceiveUPP, nil, nil));
		
		_DDData.isInitted = true;
	}
}

bool UDragAndDrop::IsAvailable()
{
	if (!_DDData.checkedAvail)
	{
		Int32 response;
		_DDData.isAvailable = (Gestalt(gestaltDragMgrAttr, &response) == noErr) && (response & (1 << gestaltDragMgrPresent)) && WEAK_IMPORT_TEST(::NewDrag);
		_DDData.checkedAvail = true;
	}
	
	return _DDData.isAvailable;
}

TDrag UDragAndDrop::New()
{
	Init();
	
	SDragInfo *info = (SDragInfo *)UMemory::NewClear(sizeof(SDragInfo));
	
	if (info)
	{
		try
		{
			CheckMacError(::NewDrag(&info->drag));
		}
		catch(...)
		{
			delete info;
			throw;
		}
	}
	return (TDrag)info;
}

void UDragAndDrop::Dispose(TDrag inRef)
{
	if (inRef)
	{
		::DisposeDrag(((SDragInfo *)inRef)->drag);
		UMemory::Dispose((TPtr)inRef);
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

extern CGrafPtr _gStartupPort;
extern GDHandle _gStartupDevice;

// returns whether or not the drag was dropped
bool UDragAndDrop::Track(TDrag inRef, const SPoint& mouseLoc, TRegion rgn)
{
	Require(inRef);
	
	if (_DDData.isTracking)
	{
		DebugBreak("UDragAndDrop() - cannot track multiple drags!");
		Fail(errorType_Misc, error_Protocol);
	}

	EventRecord evt = { mouseDown, 0, 0, { mouseLoc.v, mouseLoc.h }, 0 };
	Int16 err = ::TrackDrag(((SDragInfo *)inRef)->drag, &evt, (RgnHandle)rgn);
	
	// work around bug (?) in toolbox
	SetGWorld(_gStartupPort, _gStartupDevice);
	
	if (err && err != userCanceledErr) FailMacError(err);
	return (err != userCanceledErr);
}

// call in reply to msg_DragDropped to indicate that the drag was accepted
void UDragAndDrop::AcceptDrop(TDrag /* inRef */)
{
	_DDData.dropAccepted = true;
}

void UDragAndDrop::RejectDrop(TDrag /* inRef */)
{
	_DDData.dropAccepted = false;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

// returns how many drag items are in the specified drag reference
Uint32 UDragAndDrop::GetItemCount(TDrag inRef)
{
	Uint16 itemCount;
	OSErr err = ::CountDragItems(((SDragInfo *)inRef)->drag, &itemCount);
	if (err == badDragRefErr)
		DebugBreak("badDragRefErr");
		
	CheckMacError(err);
	return itemCount;
}

Uint32 UDragAndDrop::GetFlavourCount(TDrag inRef, Uint32 inIndex)
{
	Uint16 itemCount;
	OSErr err = ::CountDragItemFlavors (((SDragInfo *)inRef)->drag, inIndex, &itemCount);
	if (err == badDragRefErr)
		DebugBreak("badDragRefErr");
		
	CheckMacError(err);
	return itemCount;
}

Uint32 UDragAndDrop::GetItem(TDrag inRef, Uint32 inIndex)
{
	Uint32 item;
	CheckMacError(::GetDragItemReferenceNumber(((SDragInfo *)inRef)->drag, inIndex, &item));
	return item;
}

void UDragAndDrop::SetItemBounds(TDrag inRef, Uint32 inItem, const SRect& inBounds)
{
	Rect macRect = { inBounds.top, inBounds.left, inBounds.bottom, inBounds.right };
	CheckMacError(::SetDragItemBounds(((SDragInfo *)inRef)->drag, inItem, &macRect));
}

void UDragAndDrop::GetItemBounds(TDrag inRef, Uint32 inItem, SRect& outBounds)
{
	Rect macRect;
	CheckMacError(::GetDragItemBounds(((SDragInfo *)inRef)->drag, inItem, &macRect));
	outBounds.Set(macRect.left, macRect.top, macRect.right, macRect.bottom);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

// returns true if the specified item comes in the specified flavor
bool UDragAndDrop::HasFlavor(TDrag inRef, Uint32 inItem, Uint32 inFlavor)
{
	Uint32 flags;
	return (::GetFlavorFlags(((SDragInfo *)inRef)->drag, inItem, inFlavor, &flags) == noErr);
}

// returns true if the specified drag reference has any items with the specified flavor
bool UDragAndDrop::HasFlavor(TDrag inRef, Uint32 inFlavor)
{
	Uint16 i, c;
	Uint32 item, flags;
	
	if (::CountDragItems(((SDragInfo *)inRef)->drag, &c) == noErr)
	{
		for (i=1; i<=c; i++)
		{
			if (::GetDragItemReferenceNumber(((SDragInfo *)inRef)->drag, i, &item) != noErr) break;
			
			if (::GetFlavorFlags(((SDragInfo *)inRef)->drag, item, inFlavor, &flags) == noErr)
				return true;
		}
	}
	
	return false;
}

void UDragAndDrop::AddFlavor(TDrag inRef, Uint32 inItem, Uint32 inFlavor, const void *inData, Uint32 inDataSize, Uint32 inFlags)
{
	CheckMacError(::AddDragItemFlavor(((SDragInfo *)inRef)->drag, inItem, inFlavor, (void *)inData, inDataSize, inFlags));
}

void UDragAndDrop::SetFlavorData(TDrag inRef, Uint32 inItem, Uint32 inFlavor, const void *inData, Uint32 inDataSize, Uint32 inOffset)
{
	CheckMacError(::SetDragItemFlavorData(((SDragInfo *)inRef)->drag, inItem, inFlavor, inData, inDataSize, inOffset));
}

void *_ImageToMacPict(TImage inImage, const SRect *inRect);

void UDragAndDrop::AddImageFlavor(TDrag inRef, Uint32 inItem, TImage inImage, const SRect *inRect, Uint32 inFlags)
{
	Handle h = (Handle)_ImageToMacPict(inImage, inRect);

	::HLock(h);
	
	OSErr err = ::AddDragItemFlavor(((SDragInfo *)inRef)->drag, inItem, 'PICT', *h, ::GetHandleSize(h), inFlags);
	
	DisposeHandle(h);
	
	CheckMacError(err);
}

void UDragAndDrop::AddTextFlavor(TDrag inRef, Uint32 inItem, const void *inText, Uint32 inTextSize, Uint32 /* inEncoding */, Uint32 inFlags)
{
	CheckMacError(::AddDragItemFlavor(((SDragInfo *)inRef)->drag, inItem, 'TEXT', (void *)inText, inTextSize, inFlags));
}

Uint32 UDragAndDrop::GetFlavorData(TDrag inRef, Uint32 inItem, Uint32 inFlavor, void *outData, Uint32 inMaxSize, Uint32 inOffset)
{
	OSErr err = ::GetFlavorData(((SDragInfo *)inRef)->drag, inItem, inFlavor, outData, (Int32 *)&inMaxSize, inOffset);

	if (err == badDragFlavorErr)	// if no such flavor
		return 0;					// then we didn't get any data
	else
		CheckMacError(err);
		
	return inMaxSize;
}

Uint32 UDragAndDrop::GetFlavorDataSize(TDrag inRef, Uint32 inItem, Uint32 inFlavor)
{
	Uint32 size;
	if (::GetFlavorDataSize(((SDragInfo *)inRef)->drag, inItem, inFlavor, (Int32 *)&size) != noErr)
		size = 0;
	return size;
}

Uint32 UDragAndDrop::GetFlavorFlags(TDrag inRef, Uint32 inItem, Uint32 inFlavor)
{
	Uint32 flags;
	CheckMacError(::GetFlavorFlags(((SDragInfo *)inRef)->drag, inItem, inFlavor, &flags));
	return flags;
}

Uint32 UDragAndDrop::GetFlavorCount(TDrag inRef, Uint32 inItem)
{
	Uint16 flavorCount;
	CheckMacError(::CountDragItemFlavors(((SDragInfo *)inRef)->drag, inItem, &flavorCount));
	return flavorCount;
}

Uint32 UDragAndDrop::GetFlavor(TDrag inRef, Uint32 inItem, Uint32 inIndex)
{
	Uint32 flavor;
	if (::GetFlavorType(((SDragInfo *)inRef)->drag, inItem, inIndex, &flavor) != noErr)
		flavor = 0;
	return flavor;
}

void UDragAndDrop::SetDragAction(TDrag inRef, Uint16 inAction)
{
	Require(inRef);
	((SDragInfo *)inRef)->dragAction = inAction;

}


Uint16 UDragAndDrop::GetDragAction(TDrag inRef)
{
	Require(inRef);
	return ((SDragInfo *)inRef)->dragAction;
}


/* -------------------------------------------------------------------------- */
#pragma mark -

// returns nil if no file sys flavor in the specified item
TFSRefObj* UDragAndDrop::GetFileSysFlavor(TDrag inRef, Uint32 inItem)
{
	FSSpec spec;
	Int32 s;
	OSErr err;
	
	s = sizeof(spec);
	err = ::GetFlavorData(((SDragInfo *)inRef)->drag, inItem, flavorTypeHFS, &spec, &s, OFFSET_OF(HFSFlavor, fileSpec));
	
	if (err)
	{
		if (err == badDragFlavorErr)
			return nil;
		else
			FailMacError(err);
	}
	
	if (s < 7) return nil;

	return _FSSpecToRef(spec);
}

Uint32 UDragAndDrop::GetFileSysFlavorTypeCode(TDrag inRef, Uint32 inItem)
{
	Uint32 code;
	Int32 s;
	OSErr err;
	
	s = sizeof(code);
	err = ::GetFlavorData(((SDragInfo *)inRef)->drag, inItem, flavorTypeHFS, &code, &s, 0);
	
	if (err || s != sizeof(code)) return 0;

	return code;
}

void UDragAndDrop::AddPromisedFileSysFlavor(TDrag inRef, Uint32 inItem, const Int8 *inFileType, Uint32 /* inFlags */)
{
	PromiseHFSFlavor phfs;
	
	if (inFileType)
		_MIMEToMacTypeCode(inFileType, strlen(inFileType), phfs.fileType, phfs.fileCreator);
	else
	{
		phfs.fileType = 'fold';
		phfs.fileCreator = 'MACS';
	}
	
	phfs.fdFlags = 0;
	phfs.promisedFlavor = 'fssP';
	
	CheckMacError(::AddDragItemFlavor(((SDragInfo *)inRef)->drag, inItem, flavorTypePromiseHFS, &phfs, sizeof(phfs), flavorNotSaved));
	CheckMacError(::AddDragItemFlavor(((SDragInfo *)inRef)->drag, inItem, 'fssP', nil, 0, flavorNotSaved));
}

// returns nil if the drag was not dropped or was not dropped into a folder
TFSRefObj* UDragAndDrop::GetDropFolder(TDrag inRef)
{
	AEDesc dropLocAlias = { typeNull, nil };
	AEDesc dropLocFSS = { typeNull, nil };
		
	if ((::GetDropLocation(((SDragInfo *)inRef)->drag, &dropLocAlias) == noErr) && (dropLocAlias.descriptorType == typeAlias) && (::AECoerceDesc(&dropLocAlias, typeFSS, &dropLocFSS) == noErr))
	{
		FSSpec fss;
	#if TARGET_API_MAC_CARBON
		::AEGetDescData(&dropLocFSS, &fss, sizeof(FSSpec));
	#else
		FSSpec *fssp = (FSSpec *)*dropLocFSS.dataHandle;
		::BlockMoveData(fssp, &fss, fssp->name[0] + 7);
	#endif
		
		if (dropLocAlias.dataHandle) ::AEDisposeDesc(&dropLocAlias);
		if (dropLocFSS.dataHandle) ::AEDisposeDesc(&dropLocFSS);

		return _FSSpecToRef(fss);
	}

	if (dropLocAlias.dataHandle) ::AEDisposeDesc(&dropLocAlias);
	if (dropLocFSS.dataHandle) ::AEDisposeDesc(&dropLocFSS);
	
	return nil;
}

// call this to notify the drag receiver that you have created the promised file
void UDragAndDrop::CreatedPromisedFile(TDrag inRef, Uint32 inItem, TFSRefObj* inFSRef)
{
	Require(inFSRef);
	CheckMacError(::SetDragItemFlavorData(((SDragInfo *)inRef)->drag, inItem, 'fssP', (FSSpec *)inFSRef, sizeof(FSSpec), 0));
}

/* -------------------------------------------------------------------------- */
#pragma mark -

Uint32 UDragAndDrop::GetAttributes(TDrag inRef)
{
	DragAttributes attr;
	CheckMacError(::GetDragAttributes(((SDragInfo *)inRef)->drag, &attr));
	return attr;
}

void UDragAndDrop::GetMouse(TDrag inRef, SPoint& mouse, SPoint& pinnedMouse)
{
	Point macMouse, macPinnedMouse;
	CheckMacError(::GetDragMouse(((SDragInfo *)inRef)->drag, &macMouse, &macPinnedMouse));
	
	mouse.h = macMouse.h;
	mouse.v = macMouse.v;
	pinnedMouse.h = macPinnedMouse.h;
	pinnedMouse.v = macPinnedMouse.v;
}

void UDragAndDrop::GetMouse(TDrag inRef, SPoint& mouse)
{
	Point macMouse;
	CheckMacError(::GetDragMouse(((SDragInfo *)inRef)->drag, &macMouse, nil));
	mouse.h = macMouse.h;
	mouse.v = macMouse.v;
}

void UDragAndDrop::SetMouse(TDrag inRef, const SPoint& pinnedMouse)
{
	Point pt = { pinnedMouse.v, pinnedMouse.h };
	CheckMacError(::SetDragMouse(((SDragInfo *)inRef)->drag, pt));
}

void UDragAndDrop::GetOrigin(TDrag inRef, SPoint& initialMouse)
{
	Point pt;
	CheckMacError(::GetDragOrigin(((SDragInfo *)inRef)->drag, &pt));
	initialMouse.h = pt.h;
	initialMouse.v = pt.v;
}

Int16 UDragAndDrop::GetModifiers(TDrag inRef)
{
	Int16 mods;
	CheckMacError(::GetDragModifiers(((SDragInfo *)inRef)->drag, &mods, nil, nil));
	return mods;
}

void UDragAndDrop::GetModifiers(TDrag inRef, Int16 *modifiers, Int16 *mouseDownModifiers, Int16 *mouseUpModifiers)
{
	CheckMacError(::GetDragModifiers(((SDragInfo *)inRef)->drag, modifiers, mouseDownModifiers, mouseUpModifiers));
}

bool UDragAndDrop::DropLocationIsTrash(TDrag inRef)
{
	OSErr			result;
	AEDesc			dropSpec;
	FSSpec			*theSpec;
	CInfoPBRec		thePB;
	Int16			trashVRefNum;
	Int32			trashDirID;
	AEDesc			dropLocation;
	
	if (::GetDropLocation(((SDragInfo *)inRef)->drag, &dropLocation) != noErr)
		return false;
	
	//
	//	Coerce the dropLocation descriptor to an FSSpec. If there's no dropLocation or
	//	it can't be coerced into an FSSpec, then it couldn't have been the Trash.
	//

	if ((dropLocation.descriptorType != typeNull) && (::AECoerceDesc(&dropLocation, typeFSS, &dropSpec) == noErr))
	{
	#if TARGET_API_MAC_CARBON
		FSSpec fss;
		::AEGetDescData(&dropSpec, &fss, sizeof(FSSpec));
		
		theSpec = &fss;
	#else
		HLock(dropSpec.dataHandle);
		theSpec = (FSSpec *) *dropSpec.dataHandle;
	#endif
	
		//
		//	Get the directory ID of the given dropLocation object.
		//

		thePB.dirInfo.ioCompletion = 0L;
		thePB.dirInfo.ioNamePtr = (StringPtr) &theSpec->name;
		thePB.dirInfo.ioVRefNum = theSpec->vRefNum;
		thePB.dirInfo.ioFDirIndex = 0;
		thePB.dirInfo.ioDrDirID = theSpec->parID;

		result = PBGetCatInfoSync(&thePB);

	#if !TARGET_API_MAC_CARBON
		HUnlock(dropSpec.dataHandle);
	#endif
	
		::AEDisposeDesc(&dropSpec);

		if (result != noErr)
			return(false);

		// if the result is not a directory, it must not be the trash
		if (!(thePB.dirInfo.ioFlAttrib & (1 << 4)))
			return(false);

		// get information about the trash folder
		FindFolder(theSpec->vRefNum, kTrashFolderType, kCreateFolder, &trashVRefNum, &trashDirID);

		// if the directory ID of the dropLocation object is the same as the directory ID
		// returned by FindFolder, then the drop must have occurred into the Trash.
		if (thePB.dirInfo.ioDrDirID == trashDirID)
			return(true);
	}

	return(false);
}

// does NOT take ownership of <inImage> nor <inRgn> - caller is responsible for disposing, and also must remain valid through life of <inRef> (unless a diff image/rgn is specified with another call to SetImage)
void UDragAndDrop::SetImage(TDrag inRef, TImage inImage, TRegion inRgn, const SPoint& inImageOffset, Uint32 inStyle)
{
	if (HasTranslucentDrag() && inImage)
	{
		Point pt = { inImageOffset.v, inImageOffset.h };
		Int16 err = ::SetDragImage(((SDragInfo *)inRef)->drag, ::GetGWorldPixMap((GWorldPtr)_ImageToGrafPtr(inImage)), (RgnHandle)inRgn, pt, (DragImageFlags)inStyle);
		
		if (err)
		{
			if (err != -1858 && err != -1859)	// we don't care if not supported or no suitable displays
				FailMacError(err);
		}
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void UDragAndDrop::SetSendHandler(TDrag inRef, TDragSendProc inProc, void *inSendRef)
{
	SExtraInfo info;
	
	_DDMakePrivateData(inRef, info);
	info.sendProcRef = inSendRef;
	_DDSetPrivateData(inRef, info);
	
	CheckMacError(::SetDragSendProc(((SDragInfo *)inRef)->drag, gSendUPP, inProc));
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void UDragAndDrop::DrawHilite(TImage inImage, const SRect& inRect, const SColor& inColor)
{
	/************** should this be in UUserInterface? ***********************/
	
	UGraphics::SetInkMode(inImage, mode_Copy);
	UGraphics::SetInkColor(inImage, inColor);
	UGraphics::SetPenSize(inImage, 2);
	UGraphics::FrameRect(inImage, inRect);
	UGraphics::SetPenSize(inImage, 1);

/*
	SColor saveHiliteColor;
	
	UGraphics::GetHiliteColor(inImage, saveHiliteColor);

	try
	{
		UGraphics::SetMode(inImage, grafMode_Hilite);
		UGraphics::SetHiliteColor(inImage, inColor);

		UGraphics::SetLineSize(inImage, 2, 2);
		UGraphics::DrawOutline(inImage, inRect);
		
		UGraphics::SetLineSize(inImage, 1, 1);
		UGraphics::SetMode(inImage, grafMode_Copy);
	}
	catch(...)
	{
		UGraphics::SetHiliteColor(inImage, saveHiliteColor);
		throw;
	}
	
	UGraphics::SetHiliteColor(inImage, saveHiliteColor);
*/
}

#if 0
void UDragAndDrop::DrawHilite(TImage inImage, TRegion inRgn, const SColor& inColor)
{
	SColor saveHiliteColor;
	
	UGraphics::GetHiliteColor(inImage, saveHiliteColor);

	try
	{
		UGraphics::SetInkMode(inImage, mode_Highlight);
		UGraphics::SetHiliteColor(inImage, inColor);

		UGraphics::SetStylePen(inImage, 2, 2);
		UGraphics::DrawOutline(inImage, inRgn);
		
		UGraphics::SetStylePen(inImage, 1, 1);
		UGraphics::SetInkMode(inImage, mode_Copy);
	}
	catch(...)
	{
		UGraphics::SetHiliteColor(inImage, saveHiliteColor);
		throw;
	}
	
	UGraphics::SetHiliteColor(inImage, saveHiliteColor);
}
#endif

void UDragAndDrop::DrawHilite(TImage inImage, const SRect& inRect)
{
	// SColor(0x6666,0x9999,0xCCCC)  light blue
	DrawHilite(inImage, inRect, SColor(0x9999,0x9999,0xCCCC));
}

void UDragAndDrop::DrawHilite(TImage inImage, TRegion inRgn)
{
	DrawHilite(inImage, inRgn, SColor(0x9999,0x9999,0xCCCC));
}

/* -------------------------------------------------------------------------- */
#pragma mark -

/*
 * HasTranslucentDrag() returns whether or not the current hardware/system configuration
 * supports translucent dragging (activated using SetImage()).  Calling SetImage() when
 * translucent dragging is not available will have no effect on the drag.  However, you
 * may want to use HasTranslucentDrag() to determine whether to generate the image in
 * the first place.
 */
bool UDragAndDrop::HasTranslucentDrag()
{
	Init();
	return _DDData.hasTransDrag;
}

bool UDragAndDrop::IsTracking()
{
	return _DDData.isTracking;
}

bool UDragAndDrop::Equals(TDrag inRef, TDrag inCompare)
{
	return ((SDragInfo *)inRef)->drag == ((SDragInfo *)inCompare)->drag;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

static bool _DDGetPrivateData(TDrag inRef, SExtraInfo& outInfo)
{
	Size s = sizeof(SExtraInfo);
	return (::GetFlavorData(((SDragInfo *)inRef)->drag, 1, kPrivateDataType, &outInfo, &s, 0) == 0 && s == sizeof(SExtraInfo));
}

static bool _DDGetPrivateData(TDrag inRef, Uint32 inOffset, void *outData, Uint32 inSize)
{
	Size s = inSize;
	return (::GetFlavorData(((SDragInfo *)inRef)->drag, 1, kPrivateDataType, outData, &s, inOffset) == 0 && s == inSize);
}

static void _DDSetPrivateData(TDrag inRef, const SExtraInfo& inInfo)
{
	// it's important to use flavorSenderOnly so that our private data is not available to other applications
	Int16 err = ::AddDragItemFlavor(((SDragInfo *)inRef)->drag, 1, kPrivateDataType, (void *)&inInfo, sizeof(SExtraInfo), flavorSenderOnly | flavorNotSaved);
	
	if (err)
	{
		if (err == duplicateFlavorErr)		// if private data has already been added, we need to replace it
			CheckMacError(::SetDragItemFlavorData(((SDragInfo *)inRef)->drag, 1, kPrivateDataType, &inInfo, sizeof(SExtraInfo), 0));
		else
			FailMacError(err);
	}
}

static void _DDMakePrivateData(TDrag inRef, SExtraInfo& outInfo)
{
	if (!_DDGetPrivateData(inRef, outInfo))
	{
		outInfo.sendProcRef = nil;
	}
}

static pascal OSErr _DDDragTrackingProc(DragTrackingMessage inMsg, WindowPtr inWin, void */*inRef*/, DragReference inDrag)
{
	const Uint32 dragMsgs[] = { msg_Draw, msg_DragBegin, msg_DragEnter, msg_DragMove, msg_DragLeave, msg_DragEnd, msg_DragDropped, 0 };
	_DDData.isTracking = true;
	OSErr result = 0;
	
	if (!inDrag || (inMsg != kDragTrackingEnterHandler && inMsg != kDragTrackingEnterWindow && inMsg != kDragTrackingInWindow && inMsg != kDragTrackingLeaveWindow && inMsg != kDragTrackingLeaveHandler))
		goto abort;
	
	// save graphics environment
	CGrafPtr savePort;
	GDHandle saveDev;
	::GetGWorld(&savePort, &saveDev);

	try
	{
		SDragMsgData info;
		Int16 type, macMods;
		Point macPt;
		
		::GetDragModifiers(inDrag, &macMods, nil, nil);
		::GetDragMouse(inDrag, &macPt, nil);

		if (inMsg == kDragTrackingEnterHandler)
			type = msg_DragBegin;
		else if (inMsg == kDragTrackingLeaveHandler)
			type = msg_DragEnd;
		else
		{
			type = msg_DragMove;
			
			// check if mouse hasn't moved
			if (macPt.h == _DDData.dragLoc.h && macPt.v == _DDData.dragLoc.v)
				goto abort;
		}
		
		_DDData.dragLoc = macPt;
		info.time = UDateTime::GetMilliseconds();
		info.loc.h = macPt.h;
		info.loc.v = macPt.v;
		info.mods = _MacModsToStd(macMods);
		info.button = 1;
		
		SDragInfo drag;
		drag.drag = inDrag;
		drag.dragAction = type == msg_DragEnd ? 7 : 0;
		
		// only valid for the duration of this function!!!
		info.drag = (TDrag)&drag;
	
		_WNDispatchDrag(type, info, inWin);
		
		// it is essential that we process ALL drag messages because inDrag is invalid when we return
		UApplication::ProcessOnly(dragMsgs);
	}
	catch (SError& err)
	{
		result = _ConvertMacError(err);
	}
	catch (...)
	{
		result = paramErr;
	}
	
	// restore graphics environment
	::SetGWorld(savePort, saveDev);
	
abort:
	_DDData.isTracking = false;
	return result;
}

static pascal OSErr _DDDragReceiveProc(WindowPtr inWin, void */*inRef*/, DragReference inDrag)
{
	OSErr result = 0;
	_DDData.isTracking = true;
	
	try
	{
		SDragMsgData info;
		Int16 macMods;
		Point macPt;
		
		::GetDragModifiers(inDrag, &macMods, nil, nil);
		::GetDragMouse(inDrag, &macPt, nil);
		
		info.time = UDateTime::GetMilliseconds();
		info.loc.h = macPt.h;
		info.loc.v = macPt.v;
		info.mods = _MacModsToStd(macMods);
		info.button = 1;

		SDragInfo drag;
		drag.drag = inDrag;
		drag.dragAction = 0;
		
		// only valid for the duration of this function!!!
		info.drag = (TDrag)&drag;
		
		_DDData.dropAccepted = false;
		_WNDispatchDrag(msg_DragDropped, info, inWin);
		
		// it is essential that we process ALL drag messages because inDrag is invalid when we return
		UApplication::ProcessOnly(msg_DragDropped);
		
		result = _DDData.dropAccepted ? 0 : dragNotAcceptedErr;
	}
	catch (SError& err)
	{
		result = _ConvertMacError(err);
	}
	catch (...)
	{
		result = paramErr;
	}
	
	_DDData.isTracking = false;
	return result;
}

static pascal OSErr _DDDragSendProc(FlavorType inFlavor, void *inRef, ItemReference inItem, DragReference inDrag)
{
	OSErr result = 0;
	SDragInfo drag;
	drag.drag = inDrag;
	drag.dragAction = 0;
	
	try
	{
		void *ref = nil;
		_DDGetPrivateData((TDrag)&drag, OFFSET_OF(SExtraInfo, sendProcRef), &ref, sizeof(void*));
		((TDragSendProc)inRef)((TDrag)&drag, inItem, inFlavor, ref);
	}
	catch(SError& err)
	{
		try {
			UApplication::Error(err);
		} catch(...) {}
		result = paramErr;
		
		//result = _ConvertMacError(err);
	}
	catch(...)
	{
		result = paramErr;
	}
	
	return result;
}

static bool _DDHasTranslucentDrag()
{
	long response;
	bool hasTransDrag = (Gestalt(gestaltDragMgrAttr, &response) == 0) && (response & 0x8);
	
#if !TARGET_API_MAC_CARBON
	/*
	 * The newer drag managers will report in gestalt that they do support
	 * translucent dragging when in fact they do not.  In these cases,
	 * SetDragImage() can be called, but will return an "unsupported" error
	 * code (#%$&*!).  So, we're going to call SetDragImage() on a dummy drag
	 * and see if it really is supported.
	 */
	if (hasTransDrag)
	{
		DragReference drag;
		if (::NewDrag(&drag) == 0)
		{
			Point pt = { 0, 0 };
			Int16 err = ::SetDragImage(drag, nil, nil, pt, 0);
			if (err == -1858 || err == -1859)	// not supported or no suitable displays
				hasTransDrag = false;
			
			::DisposeDrag(drag);
		}
	}
#endif

	return hasTransDrag;
}





#endif /* MACINTOSH */
