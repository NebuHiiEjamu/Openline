/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
 
#include <InternetConfig.h>
#include "UHttpTransact.h"


static bool _IsDefaultBrowser_InternetExplorer();
static bool _IsDefaultBrowser_NetscapeNavigator();
static bool _IsDefaultBrowser(const Uint8 *inBrowserName);

static void *_GetExternalCookie_InternetExplorer(const Uint8 *inHost, const Uint8 *inDomain, Uint32& outDataSize);
static void *_GetExternalCookie_NetscapeNavigator(const Uint8 *inHost, const Uint8 *inDomain, Uint32& outDataSize);
bool _AddExternalCookie_InternetExplorer(const Uint8 *inHost, const Uint8 *inDomain, const void *inData, Uint32 inDataSize, const SCalendarDate& inExpiredDate);
bool _AddExternalCookie_NetscapeNavigator(const Uint8 *inHost, const Uint8 *inDomain, const void *inData, Uint32 inDataSize, const SCalendarDate& inExpiredDate);

static void *_GetCookie__InternetExplorer(const Uint8 *inData, Uint32 inDataSize, const Uint8 *inHost, const Uint8 *inDomain, Uint32& outDataSize);
static Uint32 _ReadNextCookie_InternetExplorer(const Uint8 *inData, Uint32 inDataSize, Uint8 *outHost, Uint8 *outDomain, Uint8 *outCookieData, Uint32& outCookieSize, Uint32 inMaxSize);
Uint32 _SetCookie__InternetExplorer(const Uint8 *inHost, const Uint8 *inDomain, const void *inData, Uint32 inDataSize, const SCalendarDate& inExpiredDate, Uint8 *outCookieData, Uint32 inMaxSize);

static TFSRefObj* _GetCookiesRef_NetscapeNavigator();
static Uint32 _ReadNextCookie_NetscapeNavigator(const Uint8 *inData, Uint32 inDataSize, Uint8 *outHost, Uint8 *outDomain, Uint8 *outCookieData, Uint32& outCookieSize, Uint32 inMaxSize);


void *UHttpTransact::GetExternalHttpCookie(const Uint8 *inHost, const Uint8 *inDomain, Uint32& outDataSize)
{
	outDataSize = 0;
	if (!inHost || !inDomain)
		return nil;

	if (_IsDefaultBrowser_InternetExplorer())
		return _GetExternalCookie_InternetExplorer(inHost, inDomain, outDataSize);
	else if (_IsDefaultBrowser_NetscapeNavigator())
		return _GetExternalCookie_NetscapeNavigator(inHost, inDomain, outDataSize);

	return nil;
}

// inExpiredDate must be in local time
bool UHttpTransact::AddExternalHttpCookie(const Uint8 *inHost, const Uint8 *inDomain, const void *inData, Uint32 inDataSize, const SCalendarDate& inExpiredDate)
{
	if (_IsDefaultBrowser_InternetExplorer())
		return _AddExternalCookie_InternetExplorer(inHost, inDomain, inData, inDataSize, inExpiredDate);
	else if (_IsDefaultBrowser_NetscapeNavigator())
		return _AddExternalCookie_NetscapeNavigator(inHost, inDomain, inData, inDataSize, inExpiredDate);

	return false;
}

#pragma mark -

static bool _IsDefaultBrowser_InternetExplorer()
{
	return _IsDefaultBrowser("\pInternet Explorer");
}

static bool _IsDefaultBrowser_NetscapeNavigator()
{
	return _IsDefaultBrowser("\pNetscape Communicator");
}

static bool _IsDefaultBrowser(const Uint8 *inBrowserName)
{
	if (!inBrowserName || !inBrowserName[0])
		return false;

	ProcessInfoRec info;
	ProcessSerialNumber psn;
	FourCharCode signature = '\?\?\?\?';	

	// find out the signature of the current process
	psn.highLongOfPSN = psn.lowLongOfPSN = 0;
	if (GetCurrentProcess(&psn) == noErr)
	{
		info.processInfoLength = sizeof(info);
		info.processName = nil;
		info.processAppSpec = nil;
		if (GetProcessInformation(&psn, &info) == noErr)
			signature = info.processSignature;
	}
		
	ICInstance inst;
	if (ICStart(&inst, signature) == noErr)
	{
	#if !TARGET_API_MAC_CARBON
		if (ICFindConfigFile(inst, 0, nil) == noErr)
	#endif
		{
			if (ICBegin(inst, icReadOnlyPerm) == noErr)
			{
				ICMapEntry stMapEntry;
				if (ICMapFilename(inst, "\p.html", &stMapEntry) != icPrefNotFoundErr)
				{
					ICEnd(inst);
					ICStop(inst);
					
					if (stMapEntry.creatorAppName[0] < inBrowserName[0] || UText::CompareInsensitive(stMapEntry.creatorAppName + 1, inBrowserName + 1, inBrowserName[0]))
						return false;
					
					return true;
				}
				ICEnd(inst);
			}
		}
		ICStop(inst);
	}
	
	return false;
}

#pragma mark -

static void *_GetExternalCookie_InternetExplorer(const Uint8 *inHost, const Uint8 *inDomain, Uint32& outDataSize)
{
	Uint8 key[256];
	pstrcpy(key, "\pHTTP Cookies");

	ICInstance inst;
	FourCharCode signature;
	ProcessSerialNumber psn;
	ProcessInfoRec info;
	
	// find out the signature of the current process
	signature = '\?\?\?\?';
	psn.highLongOfPSN = psn.lowLongOfPSN = 0;
	if (GetCurrentProcess(&psn) == noErr)
	{
		info.processInfoLength = sizeof(info);
		info.processName = nil;
		info.processAppSpec = nil;
		if (GetProcessInformation(&psn, &info) == noErr)
			signature = info.processSignature;
	}
	
	ICAttr attr;
	Int32 nListSize = 0;
	
	if (ICStart(&inst, signature) == noErr)
	{
	#if !TARGET_API_MAC_CARBON
		if (ICFindConfigFile(inst, 0, nil) == noErr)
	#endif
		{
			if (ICBegin(inst, icReadOnlyPerm) == noErr)
			{
				if (ICGetPref(inst, key, &attr, nil, &nListSize) != icPrefNotFoundErr)
				{
					void *pCookieList;
					try
					{
						pCookieList = UMemory::New(nListSize);
					}
					catch(...)
					{
						ICEnd(inst);
						ICStop(inst);
						throw;
					}
					
					void *pData = nil;
					Uint32 nDataSize = 0;

					if (ICGetPref(inst, key, &attr, (Int8 *)pCookieList, &nListSize) != icPrefNotFoundErr)
						pData = _GetCookie__InternetExplorer((Uint8 *)pCookieList, nListSize, inHost, inDomain, nDataSize);
					
					UMemory::Dispose((TPtr)pCookieList);
					
					ICEnd(inst);
					ICStop(inst);

					outDataSize = nDataSize;
					return pData;
				}
				ICEnd(inst);
			}
		}
		ICStop(inst);
	}
	
	return nil;
}

static void *_GetExternalCookie_NetscapeNavigator(const Uint8 *inHost, const Uint8 *inDomain, Uint32& outDataSize)
{
	TFSRefObj* pCookiesRef = _GetCookiesRef_NetscapeNavigator();
	if (!pCookiesRef)
		return nil;

	scopekill(TFSRefObj, pCookiesRef);
	StFileOpener pCookiesOpener(pCookiesRef, perm_Read);

	void *pData = nil;
	Uint32 nDataSize = 0;

	Uint8 bufReadData[2048];
	Uint32 nReadOffset;
	Uint32 nReadSize;
	
	Uint32 nFileOffset = 0;
	Uint8 psHost[256], psDomain[256], bufCookieData[1024];
	
	while (true)
	{
		nReadOffset = 0;
		nReadSize = pCookiesRef->Read(nFileOffset, bufReadData, sizeof(bufReadData));
		if (!nReadSize)
			break;
			
		while (true)
		{
			Uint32 nCookieSize;
			Uint32 nLineLength = _ReadNextCookie_NetscapeNavigator(bufReadData + nReadOffset, nReadSize - nReadOffset, psHost, psDomain, bufCookieData, nCookieSize, sizeof(bufCookieData));
			if (!nLineLength)
				break;
			
			nReadOffset += nLineLength;

			if (nCookieSize)
			{
				if (inHost[0] >= psHost[0] && !UText::CompareInsensitive(inHost + 1 + inHost[0] - psHost[0], psHost + 1, psHost[0]) && 	// tail matching
		    		inDomain[0] >= psDomain[0] && !UText::CompareInsensitive(inDomain + 1, psDomain + 1, psDomain[0]))					// head matching
		    	{
		    		if (pData)
						pData = UMemory::Reallocate((TPtr)pData, nDataSize + nCookieSize + 2);
					else
						pData = UMemory::NewClear(nCookieSize);
						
					if (!pData)
						return nil;
		    		
					if (nDataSize)
						nDataSize += UMemory::Copy((Uint8 *)pData + nDataSize, "; ", 2);

		    		nDataSize += UMemory::Copy((Uint8 *)pData + nDataSize, bufCookieData, nCookieSize);
		    	}
		    }
		}
		
		if (!nReadOffset)
			break;
			
		nFileOffset += nReadOffset;
	}
	
	outDataSize = nDataSize;
	return pData;
}

// inExpiredDate must be in local time
bool _AddExternalCookie_InternetExplorer(const Uint8 *inHost, const Uint8 *inDomain, const void *inData, Uint32 inDataSize, const SCalendarDate& inExpiredDate)
{
	// if the cookie expires when the user's session ends don't write in the Internet Config file
	if (!inExpiredDate.IsValid())
		return false;

	Uint8 key[256];
	pstrcpy(key, "\pHTTP Cookies");

	ICInstance inst;
	FourCharCode signature;
	ProcessSerialNumber psn;
	ProcessInfoRec info;
	
	// find out the signature of the current process
	signature = '\?\?\?\?';
	psn.highLongOfPSN = psn.lowLongOfPSN = 0;
	if (GetCurrentProcess(&psn) == noErr)
	{
		info.processInfoLength = sizeof(info);
		info.processName = nil;
		info.processAppSpec = nil;
		if (GetProcessInformation(&psn, &info) == noErr)
			signature = info.processSignature;
	}
	
	ICAttr attr;
	Int32 nListSize = 0;
	
	if (ICStart(&inst, signature) == noErr)
	{
	#if !TARGET_API_MAC_CARBON
		if (ICFindConfigFile(inst, 0, nil) == noErr)
	#endif
		{
			if (ICBegin(inst, icReadWritePerm) == noErr)
			{
				if (ICGetPref(inst, key, &attr, nil, &nListSize) != icPrefNotFoundErr)
				{
					void *pCookieList;
					try
					{
						pCookieList = UMemory::New(nListSize);
					}
					catch(...)
					{
						ICEnd(inst);
						ICStop(inst);
						throw;
					}
					
					bool bRet = false;
					if (ICGetPref(inst, key, &attr, (Int8 *)pCookieList, &nListSize) != icPrefNotFoundErr)
					{
						Uint8 bufCookieData[2048];
						Uint32 nCookieSize = _SetCookie__InternetExplorer(inHost, inDomain, inData, inDataSize, inExpiredDate, bufCookieData, sizeof(bufCookieData));
										
						if (nCookieSize)
						{
							try
							{
								pCookieList = UMemory::Reallocate((TPtr)pCookieList, nListSize + nCookieSize);
							}
							catch(...)
							{
								ICEnd(inst);
								ICStop(inst);
								throw;
							}
														
							nListSize += UMemory::Copy((Uint8 *)pCookieList + nListSize, bufCookieData, nCookieSize);
							if (ICSetPref(inst, key, kICAttrNoChange, (Int8 *)pCookieList, nListSize) != icPermErr)
								bRet = true;
						}
					}
					
					UMemory::Dispose((TPtr)pCookieList);
					
					ICEnd(inst);
					ICStop(inst);

					return bRet;
				}
				ICEnd(inst);
			}
		}
		ICStop(inst);
	}
	
	return false;

}

// inExpiredDate must be in local time
bool _AddExternalCookie_NetscapeNavigator(const Uint8 *inHost, const Uint8 *inDomain, const void *inData, Uint32 inDataSize, const SCalendarDate& inExpiredDate)
{
	// if the cookie expires when the user's session ends don't write in the file
	if (!inExpiredDate.IsValid())
		return true;

	Uint8 *pValue = UMemory::SearchByte('=', inData, inDataSize);
	if (!pValue)
		return false;

	TFSRefObj* pCookiesRef = _GetCookiesRef_NetscapeNavigator();
	if (!pCookiesRef)
		return false;

	scopekill(TFSRefObj, pCookiesRef);
	
	// convert from local time to GMT
	SDateTimeStamp stExpiredDate = inExpiredDate - UDateTime::GetGMTDelta();	
	stExpiredDate.ConvertYear(1970);	// calculate seconds elapsed from midnight, January 1, 1970

	Uint8 psExpiredDate[16];
	psExpiredDate[0] = UText::Format(psExpiredDate + 1, sizeof(psExpiredDate) - 1, "%lu", stExpiredDate.seconds);
	
	Uint32 nOffset = pCookiesRef->GetSize();
	StFileOpener pCookiesOpener(pCookiesRef, perm_ReadWrite);

	// format shall be: "host \t isDomain \t domain \t isSecure \t expiresDate \t name \t value \r"
	nOffset += pCookiesRef->Write(nOffset, inHost + 1, inHost[0]);					// write host
	nOffset += pCookiesRef->Write(nOffset, "\tTRUE\t", 6);							// write isDomain flag
	nOffset += pCookiesRef->Write(nOffset, inDomain + 1, inDomain[0]);				// write domain
	nOffset += pCookiesRef->Write(nOffset, "\tFALSE\t", 7);							// write isSecure flag
	nOffset += pCookiesRef->Write(nOffset, psExpiredDate + 1, psExpiredDate[0]);	// write expiresDate
	nOffset += pCookiesRef->Write(nOffset, "\t", 1);								// write tab
	nOffset += pCookiesRef->Write(nOffset, inData, pValue - inData);				// write cookie name
	nOffset += pCookiesRef->Write(nOffset, "\t", 1);								// write tab
	pValue++;																		// skip '='
	nOffset += pCookiesRef->Write(nOffset, pValue, inDataSize - (pValue - inData));	// write cookie value
	pCookiesRef->Write(nOffset, "\r", 1);											// write end of line

	return true;	
}

#pragma mark -

static void *_GetCookie__InternetExplorer(const Uint8 *inData, Uint32 inDataSize, const Uint8 *inHost, const Uint8 *inDomain, Uint32& outDataSize)
{
	void *pData = nil;
	Uint32 nDataSize = 0;

	Uint32 nOffset = 0;
	Uint8 psHost[256], psDomain[256], bufCookieData[1024];
	
	while (nOffset < inDataSize)
	{
		Uint32 nCookieSize;
		Uint32 nLineLength = _ReadNextCookie_InternetExplorer(inData + nOffset, inDataSize - nOffset, psHost, psDomain, bufCookieData, nCookieSize, sizeof(bufCookieData));
		if (!nLineLength)
			break;		
		
		nOffset += nLineLength;
			
		if (nCookieSize)
		{
			if (inHost[0] >= psHost[0] && !UText::CompareInsensitive(inHost + 1 + inHost[0] - psHost[0], psHost + 1, psHost[0]) && 	// tail matching
		    	inDomain[0] >= psDomain[0] && !UText::CompareInsensitive(inDomain + 1, psDomain + 1, psDomain[0]))					// head matching
		    {
		    	if (pData)
					pData = UMemory::Reallocate((TPtr)pData, nDataSize + nCookieSize + 2);
				else
					pData = UMemory::NewClear(nCookieSize);
						
				if (!pData)
					return nil;
		    		
				if (nDataSize)
					nDataSize += UMemory::Copy((Uint8 *)pData + nDataSize, "; ", 2);

		    	nDataSize += UMemory::Copy((Uint8 *)pData + nDataSize, bufCookieData, nCookieSize);
		    }
		}
	}
	
	outDataSize = nDataSize;
	return pData;

}

// format shall be: "name value host domain isSecure expiresDate isEnabled\r\n"
static Uint32 _ReadNextCookie_InternetExplorer(const Uint8 *inData, Uint32 inDataSize, Uint8 *outHost, Uint8 *outDomain, Uint8 *outCookieData, Uint32& outCookieSize, Uint32 inMaxSize)
{
	outCookieSize = 0;
	if (!inData || !inDataSize)
		return 0;
		
	// the end of line is "\r\n"
	Uint8 *pLineEnd = UMemory::Search("\r\n", 2, inData, inDataSize);
	if (!pLineEnd)
		return 0;

	Uint32 nLineLength = pLineEnd + 2 - inData;

	// search cookie name
	Uint8 *pNameEnd = UMemory::SearchByte(' ', inData, pLineEnd - inData);
	if (!pNameEnd)
		return 0;

	outCookieSize = UMemory::Copy(outCookieData, inData, pNameEnd - inData > inMaxSize - 1 ? inMaxSize - 1 : pNameEnd - inData) + 1;
	*(outCookieData + outCookieSize - 1) = '=';
	
	// search cookie value
	Uint8 *pValueBegin = pNameEnd + 1;
	Uint8 *pValueEnd = UMemory::SearchByte(' ', pValueBegin, pLineEnd - pValueBegin);
	if (!pValueEnd)
		return 0;

	outCookieSize += UMemory::Copy(outCookieData + outCookieSize, pValueBegin, pValueEnd - pValueBegin > inMaxSize - outCookieSize ? inMaxSize - outCookieSize : pValueEnd - pValueBegin);

	// search host
	Uint8 *pHostBegin = pValueEnd + 1;
	Uint8 *pHostEnd = UMemory::SearchByte(' ', pHostBegin, pLineEnd - pHostBegin);
	if (!pHostEnd)
		return 0;
	
	outHost[0] = UMemory::Copy(outHost + 1, pHostBegin, pHostEnd - pHostBegin > 255 ? 255 : pHostEnd - pHostBegin);
	
	// search domain
	Uint8 *pDomainBegin = pHostEnd + 1;
	Uint8 *pDomainEnd = UMemory::SearchByte(' ', pDomainBegin, pLineEnd - pDomainBegin);
	if (!pDomainEnd)
		return 0;

	outDomain[0] = UMemory::Copy(outDomain + 1, pDomainBegin, pDomainEnd - pDomainBegin > 255 ? 255 : pDomainEnd - pDomainBegin);
	
	// skip isSecure
	Uint8 *pFieldBegin = pDomainEnd + 1;
	Uint8 *pFieldEnd = UMemory::SearchByte(' ', pFieldBegin, pLineEnd - pFieldBegin);
	if (!pFieldEnd)
		return 0;

	// check expiresDate
	Uint8 *pDateBegin = pFieldEnd + 1;
	Uint8 *pDateEnd = UMemory::SearchByte(' ', pDateBegin, pLineEnd - pDateBegin);
	if (!pDateEnd)
		return 0;
	
	SDateTimeStamp stExpiredDate;
	stExpiredDate.year = 1904;	//  seconds elapsed from midnight, January 1, 1904
	stExpiredDate.seconds = UText::TextToUInteger(pDateBegin, pDateEnd - pDateBegin);
	stExpiredDate.msecs = 0;
	
	// convert from GMT to local time
	stExpiredDate += UDateTime::GetGMTDelta();

	SDateTimeStamp stCurrentDate;
	UDateTime::GetDateTimeStamp(stCurrentDate);
	
	// check if this cookie has expired
	if (stCurrentDate >= stExpiredDate)
		outCookieSize = 0;

	return nLineLength;
}

// format shall be: "name value host domain isSecure expiresDate isEnabled\r\n"
Uint32 _SetCookie__InternetExplorer(const Uint8 *inHost, const Uint8 *inDomain, const void *inData, Uint32 inDataSize, const SCalendarDate& inExpiredDate, Uint8 *outCookieData, Uint32 inMaxSize)
{
	Uint32 nCookieSize = UMemory::Copy(outCookieData, inData, inDataSize > inMaxSize ? inMaxSize : inDataSize);
	Uint8 *pEqual = UMemory::SearchByte('=', outCookieData, nCookieSize);
	if (!pEqual)
		return 0;
	
	*pEqual = ' ';	// replace equal with space
	
	// convert from local time to GMT
	SDateTimeStamp stExpiredDate = inExpiredDate - UDateTime::GetGMTDelta();	
	stExpiredDate.ConvertYear(1904);	// calculate seconds elapsed from midnight, January 1, 1904

	Uint8 psExpiredDate[16];
	psExpiredDate[0] = UText::Format(psExpiredDate + 1, sizeof(psExpiredDate) - 1, "%lu", stExpiredDate.seconds);

	nCookieSize += UText::Format(outCookieData + nCookieSize, inMaxSize - nCookieSize, " %#s %#s 0 %#s 1\r\n", inHost, inDomain, psExpiredDate);

	return nCookieSize;
}

static TFSRefObj* _GetCookiesRef_NetscapeNavigator()
{
	// make Netscape folder ref
	TFSRefObj* pNetscapeRef = UFileSys::New(kPrefsFolder, nil, "\pNetscape Users");
	scopekill(TFSRefObj, pNetscapeRef);
		
	bool bIsFolder;
	if (!pNetscapeRef->Exists(&bIsFolder) || !bIsFolder)
		return nil;
		
	Uint32 nOffset = 0;
	Uint8 psUserName[256];
	Uint32 nType, nFlags;
	THdl hList = pNetscapeRef->GetListing();
	TFSRefObj* pCookiesRef = nil;

	try
	{
		while(UFS::GetListNext(hList, nOffset, psUserName, &nType, nil, nil, nil, nil, &nFlags))
		{
			if (nFlags & 1) // if invisible
				continue;
	
			// make user folder ref
			TFSRefObj* pUserRef = UFileSys::New(pNetscapeRef, nil, psUserName);
			scopekill(TFSRefObj, pUserRef);

			if (!pUserRef->Exists(&bIsFolder) || !bIsFolder)
				continue;

			// make file ref
			pCookiesRef = UFileSys::New(pUserRef, nil, "\pMagicCookie");
			if (!pCookiesRef->Exists(&bIsFolder) || bIsFolder)
			{
				UFileSys::Dispose(pCookiesRef);
				pCookiesRef = nil;
				
				continue;
			}
			
			break;
		}
	}
	catch(...)
	{
		UMemory::Dispose(hList);
		throw;
	}

	UMemory::Dispose(hList);
	
	return pCookiesRef;
}

// format shall be: "host \t isDomain \t domain \t isSecure \t expiresDate \t name \t value \r"
static Uint32 _ReadNextCookie_NetscapeNavigator(const Uint8 *inData, Uint32 inDataSize, Uint8 *outHost, Uint8 *outDomain, Uint8 *outCookieData, Uint32& outCookieSize, Uint32 inMaxSize)
{
	outCookieSize = 0;
	if (!inData || !inDataSize)
		return 0;
		
	// the end of line in "MagicCookie" is "\r"
	Uint8 *pLineEnd = UMemory::SearchByte('\r', inData, inDataSize);
	if (!pLineEnd)
		return 0;

	Uint32 nLineLength = pLineEnd + 1 - inData;
	if (*inData == '#' || nLineLength <= 1)			// comment line or empty line
		return nLineLength;

	// search host
	Uint8 *pHostEnd = UMemory::SearchByte('\t', inData, pLineEnd - inData);
	if (!pHostEnd)
		return 0;
	
	outHost[0] = UMemory::Copy(outHost + 1, inData, pHostEnd - inData > 255 ? 255 : pHostEnd - inData);
	
	// skip isDomain
	Uint8 *pFieldBegin = pHostEnd + 1;
	Uint8 *pFieldEnd = UMemory::SearchByte('\t', pFieldBegin, pLineEnd - pFieldBegin);
	if (!pFieldEnd)
		return 0;
	
	// search domain
	Uint8 *pDomainBegin = pFieldEnd + 1;
	Uint8 *pDomainEnd = UMemory::SearchByte('\t', pDomainBegin, pLineEnd - pDomainBegin);
	if (!pDomainEnd)
		return 0;

	outDomain[0] = UMemory::Copy(outDomain + 1, pDomainBegin, pDomainEnd - pDomainBegin > 255 ? 255 : pDomainEnd - pDomainBegin);
	
	// skip isSecure
	pFieldBegin = pDomainEnd + 1;
	pFieldEnd = UMemory::SearchByte('\t', pFieldBegin, pLineEnd - pFieldBegin);
	if (!pFieldEnd)
		return 0;
		
	// check expiresDate
	Uint8 *pDateBegin = pFieldEnd + 1;
	Uint8 *pDateEnd = UMemory::SearchByte('\t', pDateBegin, pLineEnd - pDateBegin);
	if (!pDateEnd)
		return 0;

	SDateTimeStamp stExpiredDate;
	stExpiredDate.year = 1970;	//  seconds elapsed from midnight, January 1, 1970
	stExpiredDate.seconds = UText::TextToUInteger(pDateBegin, pDateEnd - pDateBegin);
	stExpiredDate.msecs = 0;
	
	// convert from GMT to local time
	stExpiredDate += UDateTime::GetGMTDelta();

	SDateTimeStamp stCurrentDate;
	UDateTime::GetDateTimeStamp(stCurrentDate);
	
	// check if this cookie has expired
	if (stCurrentDate >= stExpiredDate)
		return nLineLength;

	// search cookie name
	Uint8 *pNameBegin = pDateEnd + 1;
	Uint8 *pNameEnd = UMemory::SearchByte('\t', pNameBegin, pLineEnd - pNameBegin);
	if (!pNameEnd)
		return 0;

	outCookieSize = UMemory::Copy(outCookieData, pNameBegin, pNameEnd - pNameBegin > inMaxSize - 1 ? inMaxSize - 1 : pNameEnd - pNameBegin) + 1;
	*(outCookieData + outCookieSize - 1) = '=';
	
	// search cookie value
	Uint8 *pValueBegin = pNameEnd + 1;
	outCookieSize += UMemory::Copy(outCookieData + outCookieSize, pValueBegin, pLineEnd - pValueBegin > inMaxSize - outCookieSize ? inMaxSize - outCookieSize : pLineEnd - pValueBegin);

	return nLineLength;
}
