#if WIN32

#define DISABLE_DD	0


/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
#include "UDragAndDrop.h"

 
IID IID_AWSDragRefPtr = { /* cc8821a0-ae61-11d2-a7b7-00050281476a */
    0xcc8821a0,
    0xae61,
    0x11d2,
    {0xa7, 0xb7, 0x00, 0x05, 0x02, 0x81, 0x47, 0x6a}
  };

IID IID_AW_DDDataObject = { /* 8ec8e0c0-afb0-11d2-a7b7-00050281476a */
    0x8ec8e0c0,
    0xafb0,
    0x11d2,
    {0xa7, 0xb7, 0x00, 0x05, 0x02, 0x81, 0x47, 0x6a}
  };

typedef struct _DROPFILES { 
    DWORD pFiles; // offset of file list 
    POINT pt;     // drop point (coordinates depend on fNC) 
    BOOL fNC;     // nonclient area flag. If this member is TRUE, pt specifies the screen coordinates of a point in a window�s nonclient area. If it is FALSE, pt specifies the client coordinates of a point in the client area.
    BOOL fWide;   // TRUE if file contains wide characters, FALSE otherwise 
} DROPFILES, FAR * LPDROPFILES;


struct SDDFlav
{
	SDDFlav *next;
	CLIPFORMAT flavor;
	
	union
	{
		struct
		{
			void *data;
			Uint32 dataSize;
		} data;
		
		HGLOBAL hglob;
		Int8 *path;			// null-terminated DOS style path
	};
};

struct SDragRef
{
	SDDFlav *topFlav;		// used if I initiate create it.  if it exists, it's my TDrag if nil, and if This is not my drag, we need to build the list.

	IDataObject *dataObj;	// nil if this is my drag - since I can't take ownership of these
	bool dropAccepted;
	Uint16 dragAction;
	
	TDragSendProc pProc;
	void *pSendRef;
};

struct FlavMap
{
	Uint32 awFlav;
	CLIPFORMAT winFlav;
	DWORD tymed;
};


class FAR _DDDataObject : public IDataObject
{
	public:
	    _DDDataObject(SDDFlav **inTopFlav);     
	   
	   /* IUnknown methods */
	    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
	    STDMETHOD_(ULONG, AddRef)(void);
	    STDMETHOD_(ULONG, Release)(void);
	
	    /* IDataObject methods */    
	    STDMETHOD(GetData)(LPFORMATETC pformatetcIn,  LPSTGMEDIUM pmedium );
	    STDMETHOD(GetDataHere)(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium );
	    STDMETHOD(QueryGetData)(LPFORMATETC pformatetc );
	    STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC pformatetc, LPFORMATETC pformatetcOut);
	    STDMETHOD(SetData)(LPFORMATETC pformatetc, STGMEDIUM FAR * pmedium,
	                       BOOL fRelease);
	    STDMETHOD(EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC FAR* ppenumFormatEtc);
	    STDMETHOD(DAdvise)(FORMATETC FAR* pFormatetc, DWORD advf, 
	                       LPADVISESINK pAdvSink, DWORD FAR* pdwConnection);
	    STDMETHOD(DUnadvise)(DWORD dwConnection);
	    STDMETHOD(EnumDAdvise)(LPENUMSTATDATA FAR* ppenumAdvise);
	    
	private:
		ULONG mRefs;   
		SDDFlav **mTopFlav;
};

class FAR _DDDropSource : public IDropSource
{
	public:    
	    _DDDropSource();
	
	    /* IUnknown methods */
	    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
	    STDMETHOD_(ULONG, AddRef)(void);
	    STDMETHOD_(ULONG, Release)(void);
	
	    /* IDropSource methods */
	    STDMETHOD(QueryContinueDrag)(BOOL fEscapePressed, DWORD grfKeyState);
	    STDMETHOD(GiveFeedback)(DWORD dwEffect);
	 
	private:
	    ULONG m_refs;     
};  

class FAR _DDDropTarget : public IDropTarget
{
	public:    
	    _DDDropTarget(void *inRef);

	    /* IUnknown methods */
	    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
	    STDMETHOD_(ULONG, AddRef)(void);
	    STDMETHOD_(ULONG, Release)(void);

	    /* IDropTarget methods */
	    STDMETHOD(DragEnter)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
	    STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
	    STDMETHOD(DragLeave)();
	    STDMETHOD(Drop)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect); 
	    
	    /* Utility function to read type of drag from key state*/
	    STDMETHOD_(BOOL, DispatchDrag)(Uint32 inMsg, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
	 
	private:
	    ULONG mRefs;  
	    void *mRef;
	    SDragRef *mDrag;
	    
	    POINTL mLastPt;
	    DWORD mLastEffect, mLastKeyState;
};

class _DDFormatEtcEnum : public IEnumFORMATETC
{
	public:
		_DDFormatEtcEnum(ULONG inCount, FORMATETC *inFmts);

		// IUnknown Functions
		STDMETHOD(QueryInterface)(
		    REFIID iid,
		//Specifies the requested interface's IID
		    void ** ppvObject
		//Receives an indirect pointer to the object
		   );
		   
		STDMETHOD_(ULONG, AddRef)(void);

		STDMETHOD_(ULONG, Release)(void);

		// Enumeration functions
		//HRESULT Next(ULONG celt, FORMATETC * rgelt, ULONG * pceltFetched);
		STDMETHOD(Next)(ULONG celt, FORMATETC * rgelt, ULONG * pceltFetched);

		STDMETHOD(Skip)(ULONG celt);
		STDMETHOD(Reset)(void);
		STDMETHOD(Clone)(IEnumFORMATETC ** ppenum);
		
	protected:
		FORMATETC *mFmts;
		FORMATETC *mCurrentRef;
		ULONG mFmtCount;
		
		ULONG mRefs;
};


enum
{
	flavor_WPath						= 'path'
};

//#define REF ((SDragRef *)inRef)

void _WNDispatchDrag(TWindow inWin, TDrag inDrag, Uint32 inMsg, DWORD grfKeyState, POINTL pt);

const FlavMap *_DDLookupAWFlav(Uint32 inFlav);
const FlavMap *_DDLookupWinFlav(CLIPFORMAT inFlav);
SDDFlav *_DDBuildFlavList(TDrag inRef);

extern const char _UTCharMap_AWToPC[];
//extern const char _UTCharMap_PCToAW[];

const FlavMap _DDFlavMap[] = { {'TEXT', CF_TEXT, TYMED_HGLOBAL}, {'PICT', CF_BITMAP, TYMED_MFPICT}, {'FILE', flavor_WFile, TYMED_HGLOBAL}, {'PATH', flavor_WPath, TYMED_FILE} };
#define _DDFlavMapSize sizeof(_DDFlavMap) / sizeof(FlavMap)

struct DDGlobals
{	
	bool isInited;
	SDragRef *activeDrag;

	bool isHotlineDrag;
	bool isHotlineDrop;
	bool wasDropped;
	bool wasStarted;
	bool wasError;

} _gDDData = {false, nil, false, false, false, false, false};


/* -------------------------------------------------------------------------- */

void UDragAndDrop::Init()
{
	if (!_gDDData.isInited)
		_gDDData.isInited = true;
}

bool UDragAndDrop::IsAvailable()
{
#if DISABLE_DD
	return false;
#endif

	return true;
}

TDrag UDragAndDrop::New()
{
#if DISABLE_DD
	return nil;
#endif
		
	SDragRef *ref = nil;
	try
	{
		ref = (SDragRef *)UMemory::NewClear(sizeof(SDragRef));
	}
	catch(...)
	{
		delete ref;
		ref = nil;
		throw;
	}
		
	return (TDrag)ref;
}

void UDragAndDrop::Dispose(TDrag inRef)
{
#if DISABLE_DD
	return;
#endif

	if (inRef)
	{
		SDDFlav *flav = ((SDragRef *)inRef)->topFlav;
		
		while (flav)
		{
			SDDFlav *next = flav->next;
			
			if (flav->flavor == (Uint16)flavor_WFile)
			{
				if (flav->hglob)
					::DragFinish((HDROP)flav->hglob);
			}
			else if (flav->flavor == (Uint16)flavor_WPath)
			{
				if (flav->path)
					UMemory::Dispose((TPtr)flav->path);
			}
			else if (flav->hglob)
			{
				::GlobalFree(flav->hglob);
			}
			
			delete flav;
			flav = next;
		}
		
		UMemory::Dispose((TPtr)((SDragRef *)inRef));
		
		// TO DO  are there any restrictions on deleting this object?  do I need to post some kind of msg if something's using them?
		// what about if there are still ptrs to it?  If this is an issue, I can alwyas set a flag "deleted" and store a list
		// of all TDrags and then when Ref is decremented, I check the flag and if it's 0, kill it then...
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void _WNPostMouseUp();

bool UDragAndDrop::Track(TDrag inRef, const SPoint& inMouseLoc, TRegion inRgn)
{
#if DISABLE_DD
	return false;
#endif
	// maybe use CreateCursor to create a cursor from inRgn
	// GetSystemMetrics to see if a large cursor is supported, and if so, create one here and store it as the global cursor
	// should basically be 11111 for AND and 1010101 for XOR?  (where we want the lines)
	// draw the region on a 1-bit?  wait, we have HRGN...now, that takes care of URegion, but how do I convert this into something usable for d&d?
	// also, URegion on mac (and pc) wouldn't take care of pc cursor - ie, 
	
	#pragma unused(inRef, inMouseLoc, inRgn)
		
	_WNPostMouseUp();	// windows doesn't post a mouse-up after DoDragDrop returns (when the mouse is released).
		
	DWORD dwResult = 0;
	if (_gDDData.isInited)
	{
		_gDDData.activeDrag = ((SDragRef *)inRef);
		_gDDData.isHotlineDrag = true;
		_gDDData.isHotlineDrop = true; 
		_gDDData.wasDropped = false; 
		_gDDData.wasStarted = false;
		_gDDData.wasError = false;

		_DDDataObject *pDataObj = new _DDDataObject(&((SDragRef *)inRef)->topFlav);
		_DDDropSource *pDropSrc = new _DDDropSource();
				
		DWORD dwEffect; 
		dwResult = ::DoDragDrop(pDataObj, pDropSrc, DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK | DROPEFFECT_SCROLL, &dwEffect);

	   	pDropSrc->Release();
	   	pDataObj->Release();
	   	
		_gDDData.activeDrag = nil;
		_gDDData.isHotlineDrag = false;
		_gDDData.isHotlineDrop = false; 
		_gDDData.wasDropped = false; 
		_gDDData.wasStarted = false;
		_gDDData.wasError = false;
	}
	
	return (dwResult == DRAGDROP_S_DROP);
}

void UDragAndDrop::SetDragAction(TDrag inRef, Uint16 inAction)
{
#if DISABLE_DD
	return;
#endif

	if (inRef)
	{
		((SDragRef *)inRef)->dragAction = inAction;
	}
}

Uint16 UDragAndDrop::GetDragAction(TDrag inRef)
{
	if (((SDragRef *)inRef))
		return ((SDragRef *)inRef)->dragAction;

	return dragAction_None;
}


void UDragAndDrop::AcceptDrop(TDrag inRef)
{
#if DISABLE_DD
	return;
#endif

	if (((SDragRef *)inRef)->dataObj)
		((SDragRef *)inRef)->dropAccepted = true;
}

void UDragAndDrop::RejectDrop(TDrag inRef)
{
#if DISABLE_DD
	return;
#endif

	if (((SDragRef *)inRef)->dataObj)
		((SDragRef *)inRef)->dropAccepted = false;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

// it seems windows only supports a single item being dragged (although with diff flavours)
// custom implementations (such as multi-files) seem to be a struct of files
// need to look into this cuz it'd be nice to translate this to multi-itmes (break it down)
// to locate the multi-file flavour, start a drag into explorer and see if it requests a specific flavour
// if so, use this and accept it from explorer and examine the struct
// in any case, not "urgent" as need to get it all working first

// wait - figure out GetCanonicalFormatEtc - might be able to do multi-items
// but, items aren't separated as item1= {flav1, flav2, flav3} item2={flav2, flav3}, item3={flav2, flav1};
// its more like flavour1 {a, b, c}  flav2{a} flav3{a, b} which might actually make more sense for UD&D
//assuming of course the above is true.

// so we have a getFlavCount(inFlavour);
// and GetFlavourData(Uint32 ioIndex);	// start as 0 and it'll increment while there are more items of this flavour

Uint32 UDragAndDrop::GetItemCount(TDrag inRef)
{
#if DISABLE_DD
	return 0;
#endif
	
	if (_gDDData.isHotlineDrag)
		return 1;

	Uint32 nNrDropFile = inRef->GetFlavorDataSize(1, flavor_WFile);
	if (nNrDropFile) return nNrDropFile;

	return 1;	// windows only supports 1 item, but several flavours
}

Uint32 UDragAndDrop::GetFlavourCount(TDrag inRef, Uint32 /*inIndex*/)
{
	// must enumerate the list
	if (((SDragRef *)inRef)->dataObj)
	{
		// this is an external drag
		if (!((SDragRef *)inRef)->topFlav)
			((SDragRef *)inRef)->topFlav = _DDBuildFlavList(inRef);		// note!! this does not actually fill in the data structs of the list with data
	}

	Uint32 nFlavCount = 0;
	SDDFlav *flav = ((SDragRef *)inRef)->topFlav;
	
	while (flav)
	{
		nFlavCount++;
		flav = flav->next;
	}
		
	return nFlavCount;	
}

Uint32 UDragAndDrop::GetItem(TDrag inRef, Uint32 inIndex)
{
	#pragma unused(inRef, inIndex)
	return inIndex;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

bool UDragAndDrop::HasFlavor(TDrag inRef, Uint32 inItem, Uint32 inFlavor)
{
#if DISABLE_DD
	return false;
#endif

	return (inItem == 1) ? HasFlavor(inRef, inFlavor) : false;
}

bool UDragAndDrop::HasFlavor(TDrag inRef, Uint32 inFlavor)
{
#if DISABLE_DD
	return false;
#endif

	if (inRef)
	{
		const FlavMap *flav = _DDLookupAWFlav(inFlavor);
				
		ASSERT(((SDragRef *)inRef)->dataObj);
		FORMATETC format;
		format.cfFormat = flav ? flav->winFlav : inFlavor;
		format.ptd = NULL;
		format.dwAspect = DVASPECT_CONTENT;
		format.lindex = -1;
		format.tymed = flav ? flav->tymed : TYMED_HGLOBAL;
		return ((SDragRef *)inRef)->dataObj->QueryGetData(&format) == S_OK;
	}
		
	return false;
}

void UDragAndDrop::AddFlavor(TDrag inRef, Uint32 inItem, Uint32 inFlavor, const void *inData, Uint32 inDataSize, Uint32 inFlags)
{
#if DISABLE_DD
	return;
#endif
	#pragma unused(inItem, inFlags)

	// add to our linked list
	// but wait, what happens if I try to call this on a TDrag from elsewhere - eg, a file from the finder
	// and my d&d func gets a TDrag, can I add a flav to this?  prolly not
	// fail w/ something or other.

	// in any case, switch through inFlavour and see if it's something we know - if it is, call the specific func?
	// see if anythign defined in win32api about custom flavors
	
	if (((SDragRef *)inRef)->dataObj)	// this is a windows d&d - I can't do jack with it
		return;

	FORMATETC fmtetc;
	fmtetc.ptd = NULL;
	fmtetc.dwAspect = DVASPECT_CONTENT;
	fmtetc.lindex = -1;

	const FlavMap *flavMap = _DDLookupAWFlav(inFlavor);
		
	if (flavMap)
	{
		fmtetc.cfFormat = flavMap->winFlav;
		fmtetc.tymed = flavMap->tymed;
	}
	else
	{
		fmtetc.cfFormat = inFlavor;
		fmtetc.tymed = TYMED_HGLOBAL;
	}

	STGMEDIUM medium;
	medium.tymed = fmtetc.tymed;
	medium.pUnkForRelease = NULL;

	switch (fmtetc.tymed)
	{
		case TYMED_HGLOBAL:
			if (fmtetc.cfFormat == (Uint16)flavor_WFile)
			{
				medium.hGlobal = (HGLOBAL)inData;
			}
			else
			{
				medium.hGlobal = ::GlobalAlloc(GMEM_FIXED, inDataSize + 4);
				*((Uint32 *)medium.hGlobal) = UMemory::Copy(BPTR(medium.hGlobal) + 4, inData, inDataSize);
			}
			break;
				
		case TYMED_FILE:
			if (fmtetc.cfFormat == (Uint16)flavor_WPath)
			{
				medium.lpszFileName = (LPWSTR)UMemory::New(inDataSize + 1);
				medium.lpszFileName[UMemory::Copy(medium.lpszFileName, inData, inDataSize)] = 0;
			}	
			break;		
	};
	
	_DDDataObject *obj = new _DDDataObject(&((SDragRef *)inRef)->topFlav);
	obj->SetData(&fmtetc, &medium, false);
	obj->Release();
	
	// should I allocate this using windows memory or a TPtr?
	// could use a STGMEDIUM...might be easiest...or wait - since I have to copy to a destination anyway, could just use a TPtr
	// need a way to signify data type (or could have a union and use whichever based on type)
	// like TImage, TFSRefObj*, TPtr....but, this func only adds raw data, so we can assume it'll be HGlobal?  that way we just pass the HGlobal when asked	
}


HGLOBAL _HdlToWinHdl(THdl inHdl, Uint32 *outSize);

void UDragAndDrop::AddTextFlavor(TDrag inRef, Uint32 inItem, const void *inText, Uint32 inTextSize, Uint32 inEncoding, Uint32 inFlags)
{
#if DISABLE_DD
	return;
#endif

	#pragma unused(inItem, inEncoding, inFlags)
	// to do - replace CR with CRLF
	
	if (((SDragRef *)inRef)->dataObj)	// this is a windows d&d - I can't do jack with it
		return;

	STGMEDIUM medium;
	medium.hGlobal = NULL;
	
	try
	{
		FORMATETC fmtetc;
		fmtetc.cfFormat = CF_TEXT;
		fmtetc.ptd = NULL;
		fmtetc.dwAspect = DVASPECT_CONTENT;
		fmtetc.lindex = -1;
		fmtetc.tymed = TYMED_HGLOBAL;
		
		medium.tymed = fmtetc.tymed;
		medium.pUnkForRelease = NULL;
			
		// what's a good growsize?
		Uint32 growSize = (inTextSize >> 8) + 5;
	
		// needs to be movable so we can do the search and replace
		Uint32 outSize = inTextSize + growSize;
		medium.hGlobal = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, outSize);
		if (medium.hGlobal == NULL) Fail(errorType_Memory, memError_NotEnough);

#if oldCode
		Uint8 *start = (Uint8 *)::GlobalLock(medium.hGlobal);
		*((Uint32 *)start)++ = inTextSize+1;
		::CopyMemory(start, inText, inTextSize);
		start[inTextSize] = NULL;		// must be null-terminated		
#endif
		
		Uint8 *outStart = (Uint8 *)::GlobalLock(medium.hGlobal);
		Uint8 *out = outStart;
		Uint8 *lastOut = outStart + outSize;
		
		const Uint8 *p = (Uint8 *)inText;
		const Uint8 *q = p + inTextSize;
		
		Uint32 totalSize = inTextSize;
		
		// convert all CRs to CRLFs and map all the characters to standard pc ones		
		while (p != q)
		{
			if (out == lastOut)
			{
reallocate:
				Uint32 outs = out - outStart;
				
				::GlobalUnlock(medium.hGlobal);
				
				outSize += growSize;
				medium.hGlobal = ::GlobalReAlloc(medium.hGlobal, outSize, NULL);
				if (medium.hGlobal == NULL) Fail(errorType_Memory, memError_NotEnough);
				outStart = (Uint8 *)::GlobalLock(medium.hGlobal);
				out = outStart + outs;
				lastOut = outStart + outSize;
			}
			
			if (*p == 13)	// CR
			{
				if (out == lastOut - 1)
					goto reallocate;
					
				*out++ = 13;
				*out++ = 10;
				p++;
			}
			else
				*out++ = _UTCharMap_AWToPC[*p++];
		}
		
		// need to ensure we have enough room for the trailing NULL
		if (out == lastOut)
		{
			Uint32 outs = out - outStart;
			::GlobalUnlock(medium.hGlobal);
			
			medium.hGlobal = ::GlobalReAlloc(medium.hGlobal, ++outSize, NULL);
			if (medium.hGlobal == NULL) Fail(errorType_Memory, memError_NotEnough);
			
			outStart = (Uint8 *)::GlobalLock(medium.hGlobal);
			out = outStart + outs;
		}
		
		*out++ = NULL;	
		::GlobalUnlock(medium.hGlobal);
		
#if oldCode		
		// need to convert CR (AW) to CRLF (DOS/windoze)
		UMemory::SearchAndReplaceAll((THdl)medium.hGlobal, 0, "\x0D", 1, "\x0D\x0A", 2);
		_HdlToWinHdl((THdl)medium.hGlobal, nil);
#endif

		_DDDataObject *obj = new _DDDataObject(&((SDragRef *)inRef)->topFlav);
		obj->SetData(&fmtetc, &medium, false);
		obj->Release();
	}
	catch(...)
	{
		if (medium.hGlobal)
			::GlobalFree(medium.hGlobal);
		
		DebugBreak("UDragAndDrop::AddFlavor failed");
		return;
		// don't throw - just don't drag
	}
}

Uint32 UDragAndDrop::GetFlavorData(TDrag inRef, Uint32 inItem, Uint32 inFlavor, void *outData, Uint32 inMaxSize, Uint32 inOffset)
{
#if DISABLE_DD
	return 0;
#endif
	
	if (((SDragRef *)inRef)->dataObj)
	{
		// this is a win drag
		const FlavMap *flav = _DDLookupAWFlav(inFlavor);
		if (flav && flav->tymed != TYMED_HGLOBAL && flav->tymed != TYMED_FILE)
		{
			// this a pict or some other yuckie - call the proper func!
			return 0;
		}
		
		FORMATETC fmtetc;
		fmtetc.cfFormat = flav ? flav->winFlav : inFlavor;
		fmtetc.ptd = NULL;
		fmtetc.dwAspect = DVASPECT_CONTENT;
		fmtetc.lindex = -1;
		fmtetc.tymed = flav ? flav->tymed : TYMED_HGLOBAL;

		// User has dropped on us. Get the CF_TEXT data from drag source
		STGMEDIUM medium;
		HRESULT hr = ((SDragRef *)inRef)->dataObj->GetData(&fmtetc, &medium);
		
		if (FAILED(hr))
			return 0;
                
        Uint32 nSize = 0;
        if (fmtetc.tymed == TYMED_HGLOBAL)
		{
        	if (fmtetc.cfFormat == (Uint16)flavor_WFile)
			{
				nSize = ::DragQueryFile((HDROP)medium.hGlobal, inItem, (LPTSTR)outData, inMaxSize); // return file path
			}
			else
			{
        		// Display the data and release it.
				LPSTR pszText = (LPSTR)GlobalLock(medium.hGlobal);
				Int8 *pText = pszText + inOffset;
				
				// if this is text, we should convert CRLF to CR
				if (fmtetc.cfFormat == CF_TEXT)
				{
					Uint32 textLen = strlen(pText);
			
					Uint8 *ptr = (Uint8 *)pText;
					Uint8 *q = ptr + textLen;
					Uint8 *o = BPTR(outData);
			
					while (q != ptr)
					{
						if (*ptr == 10)		// we can just strip out all LFs (don't need to specifically look for CRLF) since text shouldn't have any
							ptr++;
						else
							*o++ = *ptr++;	//_UTCharMap_PCToAW[*ptr++];
					}

					nSize = o - BPTR(outData);
				}
				else
				{
					nSize = min(inMaxSize, GlobalSize(medium.hGlobal) - inOffset - 4, *((Uint32 *)pszText) - inOffset);
					if (nSize) nSize = UMemory::Copy(outData, pText + 4, nSize);
				}
		
				GlobalUnlock(medium.hGlobal);
			}
		}	
        else if (fmtetc.tymed == TYMED_FILE)
        {
        	nSize = min(inMaxSize, strlen((Int8 *)medium.lpszFileName) - inOffset);
			if (nSize) nSize = UMemory::Copy(outData, medium.lpszFileName + inOffset, nSize);
        }

        ReleaseStgMedium(&medium);
		return nSize;
	}
	else	// an aw drag
	{
		return 0;
	}
}

Uint32 UDragAndDrop::GetFlavorDataSize(TDrag inRef, Uint32 /*inItem*/, Uint32 inFlavor)
{
#if DISABLE_DD
	return 0;
#endif

	// might be difficult to implement - unless the hglobal is already allocated, we can get it and get size
	if (((SDragRef *)inRef)->dataObj)
	{
		// this is a win drag
		const FlavMap *flav = _DDLookupAWFlav(inFlavor);
		if (flav && flav->tymed != TYMED_HGLOBAL && flav->tymed != TYMED_FILE)
		{
			// this is a pict or some other yuckie - call the proper func!
			return 0;
		}
		
		FORMATETC fmtetc;
		fmtetc.cfFormat = flav ? flav->winFlav : inFlavor;
		fmtetc.ptd = NULL;
		fmtetc.dwAspect = DVASPECT_CONTENT;
		fmtetc.lindex = -1;
		fmtetc.tymed = flav ? flav->tymed : TYMED_HGLOBAL;

		// User has dropped on us. Get the CF_TEXT data from drag source
		STGMEDIUM medium;
		HRESULT hr = ((SDragRef *)inRef)->dataObj->GetData(&fmtetc, &medium);
		
		if (FAILED(hr))
			return 0;
        
		Uint32 nSize = 0;
        if (fmtetc.tymed == TYMED_HGLOBAL)
		{
			if (fmtetc.cfFormat == (Uint16)flavor_WFile)
			{
				nSize = ::DragQueryFile((HDROP)medium.hGlobal, max_Uint32, nil, 0); // return number of files
			}
			else if (fmtetc.cfFormat == CF_TEXT)	// if text, we must ignore the LFs
			{
				LPSTR pszText = (LPSTR)::GlobalLock(medium.hGlobal);
			
				Uint32 nMaxSize = ::GlobalSize(medium.hGlobal);
        		Uint8 *pText = BPTR(pszText);
        	
        		while (nMaxSize--)
        		{
        			if (*pText == NULL)
        				break;
        		
        			if (*pText++ != 10)	// linefeed
        				nSize++;
        		}
        	
        		::GlobalUnlock(medium.hGlobal);
			}
			else
			{
				nSize = ::GlobalSize(medium.hGlobal);
				
				if (nSize < 4)
				{
					nSize = 0;
				}
				else
				{
					nSize = min(*((Uint32 *)(LPSTR)::GlobalLock(medium.hGlobal)), nSize);
					::GlobalUnlock(medium.hGlobal);
				}
			}
		}
        else if (fmtetc.tymed == TYMED_FILE)
        {
        	nSize = strlen((Int8 *)medium.lpszFileName);
        }
		
        ReleaseStgMedium(&medium);
		return nSize;
	}
	else	// an aw drag
	{
		return 0;	
	}
}

Uint32 UDragAndDrop::GetFlavor(TDrag inRef, Uint32 /*inItem*/, Uint32 inIndex)
{
#if DISABLE_DD
	return 0;
#endif

	// must enumerate the list
	if (((SDragRef *)inRef)->dataObj)
	{
		// this is an external drag
		if (!((SDragRef *)inRef)->topFlav)
			((SDragRef *)inRef)->topFlav = _DDBuildFlavList(inRef);		// note!! this does not actually fill in the data structs of the list with data
	}

	if (((SDragRef *)inRef)->topFlav)
	{
		SDDFlav *flav = ((SDragRef *)inRef)->topFlav;
		while (--inIndex) // index is 1-based
		{
			if (!flav)
				return nil;
		
			flav = flav->next;
		}
		
		return flav->flavor;
	}

	return 0;
}


extern TFSRefObj* _FSNewRefFromWinPath(const void *inPath, Uint32 inPathSize);

TFSRefObj* UDragAndDrop::GetFileSysFlavor(TDrag inRef, Uint32 inItem)
{
#if DISABLE_DD
	return nil;
#endif

	if (_gDDData.isHotlineDrag)
		return nil;
	
	Uint8 bufPath[2048];
	Uint32 nPathSize = inRef->GetFlavorData(inItem - 1, 'FILE', bufPath, sizeof(bufPath));
	if (nPathSize) return _FSNewRefFromWinPath(bufPath, nPathSize);
	
	return nil;
}

void UDragAndDrop::AddPromisedFileSysFlavor(TDrag inRef, Uint32 inItem, const Int8 *inFileType, Uint32 inFlags)
{
	#pragma unused(inFlags)
	
	Uint8 bufTempPath[2046];
	Uint32 nPathSize = UFileSys::GetTempPath(bufTempPath, sizeof(bufTempPath));
	nPathSize += UMemory::Copy(bufTempPath + nPathSize, inFileType, inItem);
	
	DROPFILES stDropFile;
	stDropFile.pFiles = sizeof(DROPFILES);
	stDropFile.pt.x = 0; 
	stDropFile.pt.y = 0; 
	stDropFile.fNC = FALSE;
	stDropFile.fWide = FALSE;
	
	HGLOBAL hDrop = ::GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(DROPFILES) + nPathSize + 2);

	Uint8 *pDrop = (Uint8 *)GlobalLock(hDrop);
	UMemory::Copy(pDrop, &stDropFile, sizeof(DROPFILES));
	UMemory::Copy(pDrop + sizeof(DROPFILES), bufTempPath, nPathSize);
	GlobalUnlock(hDrop);
	
	AddFlavor(inRef, 1, 'FILE', hDrop, 0);
}

TFSRefObj* UDragAndDrop::GetDropFolder(TDrag inRef)
{
	#pragma unused(inRef)

	Uint8 bufTempPath[256];
	bufTempPath[0] = UFileSys::GetTempPath(bufTempPath + 1, sizeof(bufTempPath) - 1);

	return _FSNewRefFromWinPath(bufTempPath + 1, bufTempPath[0]);
}

void UDragAndDrop::CreatedPromisedFile(TDrag inRef, Uint32 inItem, TFSRefObj* inFSRef)
{
	#pragma unused(inRef, inItem, inFSRef)
}

bool UDragAndDrop::DropLocationIsTrash(TDrag inRef)
{
	#pragma unused(inRef)	
	return false;
}

void UDragAndDrop::SetSendHandler(TDrag inRef, TDragSendProc inProc, void *inSendRef)
{
	((SDragRef *)inRef)->pProc = inProc; 
	((SDragRef *)inRef)->pSendRef = inSendRef;
}

void UDragAndDrop::DrawHilite(TImage inImage, const SRect& inRect, const SColor& inColor)
{
	UGraphics::SetInkMode(inImage, mode_Copy);
	UGraphics::SetInkColor(inImage, inColor);
	UGraphics::SetPenSize(inImage, 2);
	UGraphics::FrameRect(inImage, inRect);
	UGraphics::SetPenSize(inImage, 1);
}

void UDragAndDrop::DrawHilite(TImage inImage, const SRect& inRect)
{
	DrawHilite(inImage, inRect, SColor(0x6666,0x9999,0xCCCC));
}

void UDragAndDrop::DrawHilite(TImage inImage, TRegion inRgn)
{
	DrawHilite(inImage, inRgn, SColor(0x6666,0x9999,0xCCCC));
}

/* -------------------------------------------------------------------------- */
#pragma mark -

bool UDragAndDrop::HasTranslucentDrag()
{
	return false;
}

bool UDragAndDrop::IsTracking()
{
	return _gDDData.wasStarted;
}

bool UDragAndDrop::Equals(TDrag inRef, TDrag inCompare)
{
	if (inRef == inCompare)
		return true;
	
	if (!(inRef && inCompare))
		return false;
	
	// if the two have diff refs, one must be an in drag, the other is my own out, otherwise false
	
	SDragRef *myDrag, *winDrag;	
	
	if ((SDragRef *)inRef == _gDDData.activeDrag)
	{
		myDrag = (SDragRef *)inRef;
		winDrag = (SDragRef *)inCompare;
	}
	else if ((SDragRef *)inCompare == _gDDData.activeDrag)
	{
		winDrag = (SDragRef *)inRef;
		myDrag = (SDragRef *)inCompare;
	}
	else
		return false;
			
	if (!winDrag->dataObj)	// if it's a win drag, it must have a data obj
		return false;
		
	// now must determine if they're equal - note this isn't legal windows behavior
	// I don't add a ref to the SDragRef, but that's OK since I don't use the ptr for anythign but to compare
	SDragRef *drag;
	if (winDrag->dataObj->QueryInterface(IID_AWSDragRefPtr, (void**)&drag) == S_FALSE)
		return false;

	return drag == myDrag;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

const FlavMap *_DDLookupAWFlav(Uint32 inFlav)
{
	Uint32 i = _DDFlavMapSize;
	
	while(i--)
	{
		if (inFlav == _DDFlavMap[i].awFlav)
			return &_DDFlavMap[i];
	}
	
	return NULL;
}

const FlavMap *_DDLookupWinFlav(CLIPFORMAT inFlav)
{
	Uint32 i = _DDFlavMapSize;
	
	while(i--)
	{
		if (inFlav == _DDFlavMap[i].winFlav)
			return &_DDFlavMap[i];
	}
	
	return nil;
}


void _DDRegisterWinForDrop(HWND inWin, void *inRef)
{
	_DDDropTarget *tgt;
	try
	{
		tgt = new _DDDropTarget(inRef);
		::RegisterDragDrop(inWin, tgt);
	}
	catch(...)
	{
		delete tgt;
	}
}


SDDFlav *_DDBuildFlavList(TDrag inRef)
{
	SDDFlav *topFlav = nil;
	SDDFlav **prevNext = &topFlav;

	if (((SDragRef *)inRef)->dataObj)
	{
		IEnumFORMATETC *fmts;
		if (((SDragRef *)inRef)->dataObj->EnumFormatEtc(DATADIR_GET, &fmts) != S_OK)
			return nil;		
		
		FORMATETC fmt;
		while(fmts->Next(1, &fmt, NULL) == S_OK)
		{
			SDDFlav *flav = (SDDFlav *)UMemory::NewClear(sizeof(SDDFlav));
			flav->flavor = fmt.cfFormat;
				
			*prevNext = flav;
			prevNext = &flav->next;
		}
		
		fmts->Release();
	}
	
	return topFlav;
}


#pragma mark -
   
//---------------------------------------------------------------------
//                    _DDDataObject Constructor
//---------------------------------------------------------------------        

_DDDataObject::_DDDataObject(SDDFlav **inTopFlav)
{
	mRefs = 1;    
	mTopFlav = inTopFlav;
}   

//---------------------------------------------------------------------
//                    IUnknown Methods
//---------------------------------------------------------------------

STDMETHODIMP _DDDataObject::QueryInterface(REFIID iid, void FAR* FAR* ppv) 
{
	if(iid == IID_IUnknown || iid == IID_IDataObject || iid == IID_AW_DDDataObject)
	{
		*ppv = this;
		AddRef();
		return NOERROR;
	}
    else if (iid == IID_AWSDragRefPtr)
    {
    	if (_gDDData.activeDrag)
    	{
    		*ppv = _gDDData.activeDrag;	// this isn't perfect since I don't know if this is mine...but it doesn't matter.
			return NOERROR;
    	}
    }
    
    *ppv = NULL;
    return ResultFromScode(E_NOINTERFACE);
}

ULONG _DDDataObject::AddRef(void)
{
    return ++mRefs;
}

ULONG _DDDataObject::Release(void)
{
    if (--mRefs == 0)
    {
      delete this;
      return 0;
    }

    return mRefs;
}  

//---------------------------------------------------------------------
//                    IDataObject Methods    
//  
// The following methods are NOT supported for data transfer using the clipboard or drag-drop: 
//
//      IDataObject::DAdvise    -- return OLE_E_ADVISENOTSUPPORTED
//                 ::DUnadvise
//                 ::EnumDAdvise
//      IDataObject::GetCanonicalFormatEtc -- return E_NOTIMPL
//                   (NOTE: must set pformatetcOut->ptd = NULL)
//---------------------------------------------------------------------  

STDMETHODIMP _DDDataObject::GetData(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium) 
{   
	SDDFlav *flav = *mTopFlav;
		
	while (flav)
	{
		if (flav->flavor == pformatetc->cfFormat)
		{
			// now check what flav is used
			switch (pformatetc->cfFormat)
			{
				case CF_TEXT:
					if (pformatetc->tymed & TYMED_HGLOBAL)
					{      
				        pmedium->tymed = TYMED_HGLOBAL;
				        pmedium->hGlobal = flav->hglob; 
				 		AddRef();
				 		pmedium->pUnkForRelease = this;
				        return ResultFromScode(S_OK);
					}
					break;

				case (Uint16)flavor_WFile:
					if (pformatetc->tymed & TYMED_HGLOBAL)
					{
				        if (!_gDDData.isHotlineDrop && 
				        	_gDDData.wasDropped && 
				        	!_gDDData.wasStarted && 
				        	_gDDData.activeDrag->pProc && 
				        	_gDDData.activeDrag->pSendRef)
				        {
							_gDDData.wasStarted = true;
							_gDDData.activeDrag->dataObj = this;
								
							try
							{
								bool bDropEnd = false;
								_gDDData.activeDrag->pProc(
											(TDrag)_gDDData.activeDrag, 
											1, 0, 
											_gDDData.activeDrag->pSendRef, 
											&bDropEnd, 
											&_gDDData.wasError);
								
								while ( !UApplication::IsQuit() && 
										!bDropEnd &&
										!_gDDData.wasError) 
									UApplication::ProcessAndSleep();
							}
							catch(...)
							{
								// don't throw
								_gDDData.wasError = true;
							}
						}
				        
				        if (_gDDData.wasError)
  							return ResultFromScode(E_UNEXPECTED);
				        
				        pmedium->tymed = TYMED_HGLOBAL;
				        pmedium->hGlobal = flav->hglob; 
				 		AddRef();
				 		pmedium->pUnkForRelease = this;
				        return ResultFromScode(S_OK);
					}
					break;
								
				case (Uint16)flavor_WPath:
					if (pformatetc->tymed & TYMED_FILE)
					{
				        pmedium->tymed = TYMED_FILE;
				        pmedium->lpszFileName = (LPWSTR)flav->path; 
				 		AddRef();
				 		pmedium->pUnkForRelease = this;
				        return ResultFromScode(S_OK);
					}
					break;
					
				default:
					if (pformatetc->tymed & TYMED_HGLOBAL)
					{
				        pmedium->tymed = TYMED_HGLOBAL;
				        pmedium->hGlobal = flav->hglob; 
				 		AddRef();
				 		pmedium->pUnkForRelease = this;
				        return ResultFromScode(S_OK);
					}
					break;
			};
		}
				
		flav = flav->next;
	}	
       
    return ResultFromScode(DATA_E_FORMATETC);
}
   
STDMETHODIMP _DDDataObject::GetDataHere(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium)
{
	SDDFlav *flav = *mTopFlav;
		
	while (flav)
	{
		if (flav->flavor == pformatetc->cfFormat)
		{
			// now check what flav is used
			switch (pformatetc->cfFormat)
			{
				case CF_TEXT:
					if (pformatetc->tymed & TYMED_HGLOBAL && pmedium->tymed & TYMED_HGLOBAL && GlobalSize(pmedium->hGlobal) >= GlobalSize(flav->hglob))
					{
				        UMemory::Copy(GlobalLock(pmedium->hGlobal), GlobalLock(flav->hglob), GlobalSize(flav->hglob));
				        GlobalUnlock(pmedium->hGlobal); GlobalUnlock(flav->hglob);
				        return ResultFromScode(S_OK);
					}
					break;

				case (Uint16)flavor_WFile:
					if (pformatetc->tymed & TYMED_HGLOBAL && pmedium->tymed & TYMED_HGLOBAL && GlobalSize(pmedium->hGlobal) >= GlobalSize(flav->hglob))
					{
				        if (!_gDDData.isHotlineDrop && _gDDData.wasDropped && !_gDDData.wasStarted && _gDDData.activeDrag->pProc && _gDDData.activeDrag->pSendRef)
				        {
							_gDDData.wasStarted = true;
							_gDDData.activeDrag->dataObj = this;
								
							try
							{
								bool bDropEnd = false;
								_gDDData.activeDrag->pProc((TDrag)_gDDData.activeDrag, 1, 0, _gDDData.activeDrag->pSendRef, &bDropEnd, &_gDDData.wasError);
								
								while (!UApplication::IsQuit() && !bDropEnd) 
									UApplication::ProcessAndSleep();
							}
							catch(...)
							{
								// don't throw
								_gDDData.wasError = true;
							}
						}

				        if (_gDDData.wasError)
  							return ResultFromScode(E_UNEXPECTED);

				        UMemory::Copy(GlobalLock(pmedium->hGlobal), GlobalLock(flav->hglob), GlobalSize(flav->hglob));
				        GlobalUnlock(pmedium->hGlobal); GlobalUnlock(flav->hglob);
				        return ResultFromScode(S_OK);
					}
					break;
								
				case (Uint16)flavor_WPath:
					if (pformatetc->tymed & TYMED_FILE && pmedium->tymed & TYMED_FILE && pmedium->lpszFileName)
					{
				        UMemory::Copy(pmedium->lpszFileName, flav->path, strlen(flav->path));
				        return ResultFromScode(S_OK);
					}
					break;
					
				default:
					if (pformatetc->tymed & TYMED_HGLOBAL && pmedium->tymed & TYMED_HGLOBAL && GlobalSize(pmedium->hGlobal) >= GlobalSize(flav->hglob))
					{
				        UMemory::Copy(GlobalLock(pmedium->hGlobal), GlobalLock(flav->hglob), GlobalSize(flav->hglob));
				        GlobalUnlock(pmedium->hGlobal); GlobalUnlock(flav->hglob);
				        return ResultFromScode(S_OK);
					}
					break;
			};
		}
				
		flav = flav->next;
	}	
       
    return ResultFromScode(DATA_E_FORMATETC);
}     

STDMETHODIMP _DDDataObject::QueryGetData(LPFORMATETC pformatetc) 
{   
	SDDFlav *flav = *mTopFlav;
	
	while (flav)
	{
		if (flav->flavor == pformatetc->cfFormat)
		{		
			// now check what flav is used
			switch (pformatetc->cfFormat)
			{
				case CF_TEXT:
					if (pformatetc->tymed & TYMED_HGLOBAL && pformatetc->dwAspect == DVASPECT_CONTENT)
    					return ResultFromScode(S_OK); 

					break;
						
				case (Uint16)flavor_WFile:
					if (pformatetc->tymed & TYMED_HGLOBAL && pformatetc->dwAspect == DVASPECT_CONTENT)
       					return ResultFromScode(S_OK); 

					break;
			
				case (Uint16)flavor_WPath:
					if (pformatetc->tymed & TYMED_FILE && pformatetc->dwAspect == DVASPECT_CONTENT)
  						return ResultFromScode(S_OK); 

					break;

				default:
					if (pformatetc->tymed & TYMED_HGLOBAL && pformatetc->dwAspect == DVASPECT_CONTENT)
   						return ResultFromScode(S_OK); 

					break;
			}		
		}
				
		flav = flav->next;
	}
		
	return ResultFromScode(S_FALSE);
}

STDMETHODIMP _DDDataObject::GetCanonicalFormatEtc(LPFORMATETC pformatetc, LPFORMATETC pformatetcOut)
{ 
    #pragma unused(pformatetc)
    pformatetcOut->ptd = NULL; 
   	USound::Beep();
    
    return ResultFromScode(E_NOTIMPL);
}        

STDMETHODIMP _DDDataObject::SetData(LPFORMATETC pformatetc, STGMEDIUM *pmedium, BOOL fRelease)
{   
	if (pformatetc->tymed != TYMED_HGLOBAL && pformatetc->tymed != TYMED_FILE)
		return ResultFromScode(DV_E_TYMED);	
	
	SDDFlav *flav = (SDDFlav *)UMemory::NewClear(sizeof(SDDFlav));
	flav->flavor = pformatetc->cfFormat;
	
	switch (pformatetc->cfFormat)
	{
		case CF_TEXT:
		case (Uint16)flavor_WFile:
			if (pformatetc->tymed & TYMED_HGLOBAL)
			{
		      	if (fRelease)
		      	{
		        	DWORD dwSize = GlobalSize(pmedium->hGlobal);
		        	flav->hglob = ::GlobalAlloc(GMEM_FIXED, dwSize);
		        	UMemory::Copy(GlobalLock(flav->hglob), GlobalLock(pmedium->hGlobal), dwSize);
		        	GlobalUnlock(flav->hglob); GlobalUnlock(pmedium->hGlobal);
		        }
		        else
		        {
		        	flav->hglob = pmedium->hGlobal;
		        }
		    }

			break;
							
		case (Uint16)flavor_WPath:
			if (pformatetc->tymed & TYMED_FILE)
			{
				if (fRelease)
				{
					DWORD dwSize = strlen((Int8 *)pmedium->lpszFileName);
					flav->path = (Int8 *)UMemory::New(dwSize);
					UMemory::Copy(flav->path, pmedium->lpszFileName, dwSize);
				}
				else
				{
					flav->path = (Int8*)pmedium->lpszFileName;
				}
			}

			break;
				
		default:
			if (pformatetc->tymed & TYMED_HGLOBAL)
			{
		      	if (fRelease)
		      	{
		        	DWORD dwSize = GlobalSize(pmedium->hGlobal);
		        	flav->hglob = ::GlobalAlloc(GMEM_FIXED, dwSize);
		        	UMemory::Copy(GlobalLock(flav->hglob), GlobalLock(pmedium->hGlobal), dwSize);
		        	GlobalUnlock(flav->hglob); GlobalUnlock(pmedium->hGlobal);
		        }
		        else
		        {
		        	flav->hglob = pmedium->hGlobal;
		        }
		    }

			break;
	};
	
	// flav needs to be added to the end
	SDDFlav **flavPtr = mTopFlav;
	while(*flavPtr)
		flavPtr = &(*flavPtr)->next;
		
	*flavPtr = flav;
					
    return ResultFromScode(S_OK);
}


/*
STDAPI_(LPENUMFORMATETC)OleStdEnumFmtEtc_Create(ULONG nCount, LPFORMATETC lpEtc);
*/
  
STDMETHODIMP _DDDataObject::EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC FAR* ppenumFormatEtc)
{ 
#if 0
    // A standard implementation is provided by OleStdEnumFmtEtc_Create
    // which can be found in \ole2\samp\ole2ui\enumfetc.c in the OLE 2 SDK.
    // This code from ole2ui is copied to the enumfetc.c file in this sample.
    
    SCODE sc = S_OK;
    FORMATETC fmtetc;
    
    fmtetc.cfFormat = CF_TEXT;
    fmtetc.dwAspect = DVASPECT_CONTENT;
    fmtetc.tymed = TYMED_HGLOBAL;
    fmtetc.ptd = NULL;
    fmtetc.lindex = -1;

    if (dwDirection == DATADIR_GET){
        *ppenumFormatEtc = ::OleStdEnumFmtEtc_Create(1, &fmtetc);
        if (*ppenumFormatEtc == NULL)
            sc = E_OUTOFMEMORY;

    } else if (dwDirection == DATADIR_SET){
        // A data transfer object that is used to transfer data
        //    (either via the clipboard or drag/drop does NOT
        //    accept SetData on ANY format.
        sc = E_NOTIMPL;
        goto error;
    } else {
        sc = E_INVALIDARG;
        goto error;
    }

error:
    return ResultFromScode(sc);
#endif 

    SCODE sc = S_OK;

	if (dwDirection == DATADIR_GET)
	{
//		if (_gDDData.activeDrag)
//		{
			SDDFlav *flav = *mTopFlav;
			Uint32 count = 0;
			while (flav)
			{
				count++;
				flav = flav->next;
			}
			
			
			FORMATETC *fmts = (FORMATETC *)UMemory::NewClear(sizeof(FORMATETC) * count);
		
			flav = *mTopFlav;
			
			FORMATETC *fmt = fmts;
			
			while (flav)
			{
				fmt->cfFormat = flav->flavor;
			    fmt->dwAspect = DVASPECT_CONTENT;
			    const FlavMap *fm = _DDLookupWinFlav(flav->flavor);
			    fmt->tymed = fm ? fm->tymed : TYMED_HGLOBAL;
			    fmt->ptd = NULL;
			    fmt->lindex = -1;

				fmt++;
				flav = flav->next;
			}
			
			try
			{
				*ppenumFormatEtc = new _DDFormatEtcEnum(count, fmts);
			}
			catch(...)
			{
				sc = E_OUTOFMEMORY;	
			}
			delete fmts;
//		}
	}
	else if (dwDirection == DATADIR_SET)
	{
        // A data transfer object that is used to transfer data
        // (either via the clipboard or drag/drop does NOT
        // accept SetData on ANY format.
        sc = E_NOTIMPL;
        goto error;
    }
    else
    {
        sc = E_INVALIDARG;
        goto error;
    }

error:
    return ResultFromScode(sc);
}

STDMETHODIMP _DDDataObject::DAdvise(FORMATETC FAR* pFormatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD FAR* pdwConnection)
{ 
	#pragma unused(pFormatetc, advf, pAdvSink, pdwConnection)
	return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}
   

STDMETHODIMP _DDDataObject::DUnadvise(DWORD dwConnection)
{ 
    #pragma unused(dwConnection)
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

STDMETHODIMP _DDDataObject::EnumDAdvise(LPENUMSTATDATA FAR* ppenumAdvise)
{ 
   	#pragma unused(ppenumAdvise)
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}


#pragma mark -

//---------------------------------------------------------------------
//                    _DDDropSource Constructor
//---------------------------------------------------------------------        

_DDDropSource::_DDDropSource()
{
    m_refs = 1;  
}

//---------------------------------------------------------------------
//                    IUnknown Methods
//---------------------------------------------------------------------

HRESULT _DDDropSource::QueryInterface(REFIID riid, void FAR* FAR* ppvObj)
{
	if (riid == IID_IUnknown || riid == IID_IDataObject)
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}

	*ppvObj = NULL;
	
	return ResultFromScode(E_NOINTERFACE);
}

ULONG _DDDropSource::AddRef()
{
    return ++m_refs;
}

ULONG _DDDropSource::Release()
{
    if(--m_refs == 0)
    {
      delete this;
      return 0;
    }
    
    return m_refs;
}

//---------------------------------------------------------------------
//                    IDropSource Methods    
//---------------------------------------------------------------------  

HRESULT _DDDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
	if (fEscapePressed)
		return DRAGDROP_S_CANCEL;
		
	if (grfKeyState & MK_LBUTTON)
		return S_OK;

	_gDDData.wasDropped = true;
		
	return DRAGDROP_S_DROP;
}

LONG _DDDropSource::GiveFeedback(DWORD dwEffect)
{
	#pragma unused(dwEffect)
    return ResultFromScode(DRAGDROP_S_USEDEFAULTCURSORS);
}
                                          

#pragma mark -

_DDFormatEtcEnum::_DDFormatEtcEnum(ULONG inCount, FORMATETC *inFmts)
{
	mFmts = nil;
	mFmtCount = inCount;
	mRefs = 1;
	if (inCount)
	{
		try
		{
			mFmts = (FORMATETC *)UMemory::New(inFmts, sizeof(FORMATETC) * inCount);
			mCurrentRef = mFmts;
		}
		catch(...)
		{
			mFmts = nil;
			mFmtCount = 0;
			throw;
		}
	}
}

HRESULT _DDFormatEtcEnum::QueryInterface(REFIID iid, void ** ppvObject)
{
	if(iid == IID_IUnknown || iid == IID_IEnumFORMATETC)
	{
		*ppvObject = this;
		AddRef();
		return NOERROR;
	}

	*ppvObject = NULL;
	return ResultFromScode(E_NOINTERFACE);
}

ULONG _DDFormatEtcEnum::AddRef(void)
{
	return ++mRefs;
}

ULONG _DDFormatEtcEnum::Release(void)
{
	if (--mRefs == 0)
	{
		delete this;
		return 0;
	}

	return mRefs;
}

HRESULT _DDFormatEtcEnum::Next(ULONG celt, FORMATETC * rgelt, ULONG * pceltFetched)
{
	HRESULT result = S_OK;
	
	ULONG remaining = (mFmtCount - (mCurrentRef - mFmts));
	
	if (remaining < celt)
	{
		if (remaining == 0)
		{
			if (pceltFetched)
				*pceltFetched = 0;
			return S_FALSE;
		}
		
		result = S_FALSE;
		celt = remaining;
	}
	
	
	ULONG count = UMemory::Copy(rgelt, mCurrentRef, sizeof(FORMATETC) * celt) / sizeof(FORMATETC);
	
	if (pceltFetched)
		*pceltFetched = count;
		
	mCurrentRef += count;
	
	return result;
}

HRESULT _DDFormatEtcEnum::Skip(ULONG celt)
{
	ULONG remaining = (mFmtCount - (mCurrentRef - mFmts));
	
	if (remaining < celt)
	{
		mCurrentRef += remaining;
		return S_FALSE;
	}

	mCurrentRef += celt;
	return S_OK;
}

HRESULT _DDFormatEtcEnum::Reset(void)
{
	mCurrentRef = mFmts;
	return S_OK;
}

HRESULT _DDFormatEtcEnum::Clone(IEnumFORMATETC ** ppenum)
{
	try
	{
		*ppenum = new _DDFormatEtcEnum(mFmtCount, mFmts);
	}
	catch(...)
	{
		*ppenum = nil;
		throw;
	}
	
	return S_OK;
}


#pragma mark -

//---------------------------------------------------------------------
//                    _DDDropTarget Constructor
//---------------------------------------------------------------------        

_DDDropTarget::_DDDropTarget(void *inRef)
{
	mRefs = 1; 
	mRef = inRef;
	mDrag = nil;
	mLastPt.x = 0;
	mLastPt.y = 0;
	mLastEffect = DROPEFFECT_NONE;
	mLastKeyState = 0;
}   

//---------------------------------------------------------------------
//                    IUnknown Methods
//---------------------------------------------------------------------

STDMETHODIMP _DDDropTarget::QueryInterface(REFIID iid, void FAR* FAR* ppv) 
{
    if(iid == IID_IUnknown || iid == IID_IDropTarget)
    {
      *ppv = this;
      AddRef();
      return NOERROR;
    }

    *ppv = NULL;
    return ResultFromScode(E_NOINTERFACE);
}


STDMETHODIMP_(ULONG) _DDDropTarget::AddRef(void)
{
    return ++mRefs;
}


STDMETHODIMP_(ULONG) _DDDropTarget::Release(void)
{
    if(--mRefs == 0)
    {
      delete this;
      return 0;
    }

    return mRefs;
}  

//---------------------------------------------------------------------
//                    IDropTarget Methods
//---------------------------------------------------------------------  

STDMETHODIMP _DDDropTarget::DragEnter(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{  
	// I could get an enumeratin of formats from the source and thus the HasFlav will work
	// this obj could store all this stuff, or we just store globally

	_gDDData.isHotlineDrop = true;
	
	try
	{
		mDrag = (SDragRef *)UMemory::NewClear(sizeof(SDragRef));
		
		mDrag->dataObj = pDataObj;
		pDataObj->AddRef();
		
		mLastPt = pt;

		// do I want to get an enum of formats here and build a linked list?
		DispatchDrag(msg_DragEnter, grfKeyState, pt, pdwEffect);
	}
	catch(...)
	{
		mDrag = nil;
		// don't throw since a drag failing isn't fatal
	}

	return NOERROR;
}

STDMETHODIMP _DDDropTarget::DragOver(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
	// is this the right place to do this?
	// I shoudl also check the modifiers...

	if (pt.x != mLastPt.x || pt.y != mLastPt.y || grfKeyState != mLastKeyState)
	{
		mLastPt = pt;
		DispatchDrag(msg_DragMove, grfKeyState, pt, pdwEffect);
		mLastEffect = *pdwEffect;
		mLastKeyState = grfKeyState;
	}
	else
		*pdwEffect = mLastEffect;
    	    
    return NOERROR;
}

STDMETHODIMP _DDDropTarget::DragLeave()
{   
    _WNDispatchDrag((TWindow)mRef, (TDrag)mDrag, msg_DragLeave, nil, mLastPt);

    mDrag->dataObj->Release();
    delete mDrag;
    mDrag = nil;

	_gDDData.isHotlineDrop = false;

    return NOERROR;
}

STDMETHODIMP _DDDropTarget::Drop(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)  
{   
	(pDataObj);
    ASSERT(pDataObj == mDrag->dataObj);
    mLastPt = pt;

 	DispatchDrag(msg_DragDropped, grfKeyState, pt, pdwEffect);
    _WNDispatchDrag((TWindow)mRef, (TDrag)mDrag, msg_DragLeave, grfKeyState, pt);
	
    mDrag->dataObj->Release();

    if (mDrag->dropAccepted)
	    *pdwEffect = DROPEFFECT_COPY;	// only copy is supported at this pt.
    else
		*pdwEffect = DROPEFFECT_NONE;
		
    delete mDrag;
    mDrag = nil;

	_gDDData.isHotlineDrop = false;
	
	return NOERROR;
}   

/* OleStdGetDropEffect
** -------------------
**
** Convert a keyboard state into a DROPEFFECT.
**
** returns the DROPEFFECT value derived from the key state.
**    the following is the standard interpretation:
**          no modifier -- DROPEFFECT_COPY
**          CTRL        -- DROPEFFECT_MOVE
**          CTRL-SHIFT  -- DROPEFFECT_LINK
**
**    Default Drop: this depends on the type of the target application.
**    this is re-interpretable by each target application. a typical
**    interpretation is if the drag is local to the same document
**    (which is source of the drag) then a MOVE operation is
**    performed. if the drag is not local, then a COPY operation is
**    performed.
*/

#define OleStdGetDropEffect(grfKeyState) ( (grfKeyState & MK_CONTROL) ?     \
        ( (grfKeyState & MK_SHIFT) ? DROPEFFECT_LINK : DROPEFFECT_MOVE ) : DROPEFFECT_COPY )


//---------------------------------------------------------------------
// _DDDropTarget::QueryDrop: Given key state, determines the type of 
// acceptable drag and returns the a dwEffect. 
//---------------------------------------------------------------------   
STDMETHODIMP_(BOOL) _DDDropTarget::DispatchDrag(Uint32 inMsg, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{  
    _WNDispatchDrag((TWindow)mRef, (TDrag)mDrag, inMsg, grfKeyState, pt);

	switch(mDrag->dragAction)
	{
		case dragAction_None:
			*pdwEffect = DROPEFFECT_NONE;
			return FALSE;
			break;
		case dragAction_Move:
			*pdwEffect = DROPEFFECT_MOVE;
			break;
		case dragAction_Link:
			*pdwEffect = DROPEFFECT_LINK;
			break;
		default:	// dragAction_Copy
			*pdwEffect = DROPEFFECT_COPY;
			break;
	
	}

	return TRUE;
}   


#endif // WIN32
