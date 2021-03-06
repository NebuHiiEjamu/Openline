#if WIN32

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "UClipboard.h"

static bool _CLIsOpen = false;

HGLOBAL _HdlToWinHdl(THdl inHdl, Uint32 *outSize);

Uint32 _MIMEToWinClipFormat(const void *inText, Uint32 inSize);

void _FailLastWinError(const Int8 *inFile, Uint32 inLine);
void _FailWinError(Int32 inWinError, const Int8 *inFile, Uint32 inLine);
#if DEBUG
	#define FailLastWinError()		_FailLastWinError(__FILE__, __LINE__)
	#define FailWinError(id)		_FailWinError(id, __FILE__, __LINE__)
#else
	#define FailLastWinError()		_FailLastWinError(nil, 0)
	#define FailWinError(id)		_FailWinError(id, nil, 0)
#endif

TImage _NewDCImage(HDC inDC, Uint32 inWidth, Uint32 inHeight);

/* -------------------------------------------------------------------------- */

// returns nil if no data
THdl UClipboard::GetData(const Int8 *inType)
{
	Uint32 format = _MIMEToWinClipFormat(inType, strlen(inType));
	THdl hdl = nil;

	if (format && ::OpenClipboard(NULL))
	{
		HANDLE h = ::GetClipboardData(format);
		if (h)
		{
			Uint32 s = ::GlobalSize(h);
			void *p = ::GlobalLock(h);
			try
			{
				if (format == CF_TEXT)
					s = strlen((char *)p);
				
				hdl = UMemory::NewHandle(p, s);
				
				// strip linefeed chars
				UMemory::SearchAndReplaceAll(hdl, 0, "\x0A", 1, nil, 0);
			}
			catch(...)
			{
				::GlobalUnlock(h);
				::CloseClipboard();
				UMemory::Dispose(hdl);
				throw;
			}
			::GlobalUnlock(h);
		}
		::CloseClipboard();
	}
	
	return hdl;
}

// if you just want one item on the clipboard, you don't have to call BeginSet/EndSet
void UClipboard::SetData(const Int8 *inType, const void *inData, Uint32 inDataSize)
{
	Uint32 err, format;
	Uint8 *p;
	HANDLE h;

	format = _MIMEToWinClipFormat(inType, strlen(inType));
	if (format == 0) return;

	if (format == CF_TEXT)
	{
		h = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, inDataSize+1+4);
		if (h == NULL) Fail(errorType_Memory, memError_NotEnough);
		
		p = (Uint8 *)::GlobalLock(h);
		*((Uint32 *)p)++ = inDataSize+1;
		::CopyMemory(p, inData, inDataSize);
		p[inDataSize] = 0;		// must be null-terminated
		::GlobalUnlock(h);

		// need to convert CR (AW) to CRLF (DOS/windoze)
		try
		{
			UMemory::SearchAndReplaceAll((THdl)h, 0, "\x0D", 1, "\x0D\x0A", 2);
			_HdlToWinHdl((THdl)h, nil);
		}
		catch(...)
		{
			::GlobalFree(h);
			throw;
		}
	}
	else
	{
		h = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, inDataSize);
		if (h == NULL) Fail(errorType_Memory, memError_NotEnough);
		
		p = (Uint8 *)::GlobalLock(h);
		::CopyMemory(p, inData, inDataSize);
		::GlobalUnlock(h);
	}
	
	if (!_CLIsOpen && !::OpenClipboard(NULL))
	{
		err = ::GetLastError();
		::GlobalFree(h);
		FailWinError(err);
	}
	
	if (!::EmptyClipboard())
	{
		err = ::GetLastError();
		::GlobalFree(h);
		FailWinError(err);
	}
	
	if (::SetClipboardData(format, h) == NULL)
	{
		err = ::GetLastError();
		::GlobalFree(h);
	}
	else
		err = 0;
	
	if (!_CLIsOpen) ::CloseClipboard();
	
	if (err) FailWinError(err);
}

void UClipboard::BeginSet()
{
	if (_CLIsOpen)
	{
		DebugBreak("UClipboard - BeginSet() already called");
		Fail(errorType_Misc, error_Protocol);
	}
	
	if (!::OpenClipboard(NULL)) FailLastWinError();
	_CLIsOpen = true;
}

void UClipboard::EndSet()
{
	if (_CLIsOpen)
	{
		_CLIsOpen = false;
		::CloseClipboard();
	}
}

// does not modify nor take ownership of <inImage>
void UClipboard::SetImageData(TImage inImage, const SRect *inRect, Uint32 /* inOptions */)
{
	#pragma unused(inRect)
	Require(inImage);
	Fail(errorType_Misc, error_Unimplemented);
}

TImage UClipboard::GetImageData(SRect *outRect, Uint32 /* inOptions */)
{
	TImage img = nil;
	HDC dc = NULL;
	HBITMAP bm = NULL;
	
	if (::OpenClipboard(NULL))
	{
		HANDLE h = (HBITMAP)::GetClipboardData(CF_DIB);
		if (h)
		{
			BITMAPINFO *bmi = (BITMAPINFO *)::GlobalLock(h);
			try
			{
				dc = ::CreateCompatibleDC(NULL);
				if (dc == NULL) Fail(errorType_Memory, memError_NotEnough);
				
				void *dibBits;
				bm = CreateDIBSection(dc, bmi, DIB_RGB_COLORS, &dibBits, NULL, 0);
				if (bm == NULL) FailLastWinError();
				GdiFlush();

				::CopyMemory(dibBits, BPTR(bmi) + bmi->bmiHeader.biSize + (bmi->bmiHeader.biClrUsed * sizeof(RGBQUAD)), bmi->bmiHeader.biSizeImage);

				::SelectObject(dc, bm);
				
				if (outRect) outRect->Set(0, 0, bmi->bmiHeader.biWidth, abs(bmi->bmiHeader.biHeight));
				
				img = _NewDCImage(dc, bmi->bmiHeader.biWidth, abs(bmi->bmiHeader.biHeight));
			}
			catch(...)
			{
				::GlobalUnlock(h);
				if (dc) DeleteDC(dc);
				if (bm) DeleteObject(bm);
				::CloseClipboard();
				throw;
			}
			::GlobalUnlock(h);
		}
		::CloseClipboard();
	}
	
	return img;
}

// does not modify nor take ownership of <inHdl>
void UClipboard::SetSoundData(THdl inHdl, Uint32 /* inOptions */)
{
	Require(inHdl);
	Fail(errorType_Misc, error_Unimplemented);
}

THdl UClipboard::GetSoundData(Uint32 /* inOptions */)
{
	Fail(errorType_Misc, error_Unimplemented);
	return nil;
}


/* -------------------------------------------------------------------------- */
#pragma mark -

Uint32 _MIMEToWinClipFormat(const void *inText, Uint32 inSize)
{
	if (inSize == 10 && UMemory::Equal(inText, "text/plain", 10))
		return CF_TEXT;
	
	return 0;
}



#endif /* WIN32 */
