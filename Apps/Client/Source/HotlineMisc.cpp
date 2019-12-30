/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "Hotline.h"


/* ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ */

#define MAX_CACHE_LIST_ITEMS	30

CMyCacheList::CMyCacheList()
{
}

CMyCacheList::~CMyCacheList()
{
	Clear();
}

bool CMyCacheList::AddFileList(const void *inPathData, Uint32 inPathSize, TFieldData inListData)
{
	return AddList(mFileList, inPathData, inPathSize, inListData);
}

bool CMyCacheList::AddBundleList(const void *inPathData, Uint32 inPathSize, TFieldData inListData)
{
	return AddList(mBundleList, inPathData, inPathSize, inListData);
}

bool CMyCacheList::AddArticleList(const void *inPathData, Uint32 inPathSize, TFieldData inListData)
{
	return AddList(mArticleList, inPathData, inPathSize, inListData);
}

TFieldData CMyCacheList::SearchFileList(const void *inPathData, Uint32 inPathSize)
{
	return SearchList(mFileList, inPathData, inPathSize);
}

TFieldData CMyCacheList::SearchBundleList(const void *inPathData, Uint32 inPathSize)
{
	return SearchList(mBundleList, inPathData, inPathSize);
}

TFieldData CMyCacheList::SearchArticleList(const void *inPathData, Uint32 inPathSize)
{
	return SearchList(mArticleList, inPathData, inPathSize);
}

bool CMyCacheList::ChangedFileList(const void *inPathData, Uint32 inPathSize)
{
	return ChangedList(mFileList, inPathData, inPathSize);
}

bool CMyCacheList::ChangedBundleList(const void *inPathData, Uint32 inPathSize)
{
	return ChangedList(mBundleList, inPathData, inPathSize);
}

bool CMyCacheList::ChangedArticleList(const void *inPathData, Uint32 inPathSize)
{
	return ChangedList(mArticleList, inPathData, inPathSize);
}

void CMyCacheList::Clear()
{
	ClearFileList();
	ClearBundleList();
	ClearArticleList();
}

void CMyCacheList::ClearFileList()
{
	ClearList(mFileList);
}

void CMyCacheList::ClearBundleList()
{
	ClearList(mBundleList);
}

void CMyCacheList::ClearArticleList()
{
	ClearList(mArticleList);
}

bool CMyCacheList::AddList(CPtrList<SMyCacheListInfo>& inCacheList, const void *inPathData, Uint32 inPathSize, TFieldData inListData)
{
	Uint32 i = 0;
	SMyCacheListInfo *pCacheListInfo;
	
	Uint32 nPathCheckSum = UMemory::Checksum(inPathData, inPathSize);
	
	while (inCacheList.GetNext(pCacheListInfo, i))
	{
		if (pCacheListInfo && pCacheListInfo->nPathCheckSum == nPathCheckSum && pCacheListInfo->nPathSize == inPathSize && 
			UMemory::Equal(pCacheListInfo->pPathData, inPathData, inPathSize))
		{
			pCacheListInfo->pListData->SetDataHandle(inListData->DetachDataHandle());
			pCacheListInfo->bChanged = false;
			
			return true;
		}
	}

	pCacheListInfo = nil;
	if (inCacheList.GetItemCount() < MAX_CACHE_LIST_ITEMS)
	{
		pCacheListInfo = new SMyCacheListInfo;
		pCacheListInfo->pPathData = nil;
	}
	else
	{
		pCacheListInfo = inCacheList.RemoveItem(1);		// remove old list
		
		if (pCacheListInfo && pCacheListInfo->pPathData)
		{
			UMemory::Dispose(pCacheListInfo->pPathData);
			pCacheListInfo->pPathData = nil;
		}
	}		

	if (!pCacheListInfo)
		return false;
	
	if (inPathData && inPathSize)
	{
		pCacheListInfo->pPathData = UMemory::New(inPathSize);
		UMemory::Copy(pCacheListInfo->pPathData, inPathData, inPathSize);
	}
	
	pCacheListInfo->nPathSize = inPathSize;
	pCacheListInfo->nPathCheckSum = nPathCheckSum;
	pCacheListInfo->pListData->SetDataHandle(inListData->DetachDataHandle());
	pCacheListInfo->bChanged = false;

	inCacheList.AddItem(pCacheListInfo);
		
	return true;
}

TFieldData CMyCacheList::SearchList(CPtrList<SMyCacheListInfo>& inCacheList, const void *inPathData, Uint32 inPathSize)
{
	Uint32 i = 0;
	SMyCacheListInfo *pCacheListInfo;
	
	Uint32 nPathCheckSum = UMemory::Checksum(inPathData, inPathSize);
	
	while (inCacheList.GetNext(pCacheListInfo, i))
	{
		if (pCacheListInfo && pCacheListInfo->nPathCheckSum == nPathCheckSum && pCacheListInfo->nPathSize == inPathSize && 
			UMemory::Equal(pCacheListInfo->pPathData, inPathData, inPathSize))
		{
			if (pCacheListInfo->bChanged)
				return nil;
			
			return pCacheListInfo->pListData;
		}
	}
	
	return nil;
}

bool CMyCacheList::ChangedList(CPtrList<SMyCacheListInfo>& inCacheList, const void *inPathData, Uint32 inPathSize)
{
	Uint32 i = 0;
	SMyCacheListInfo *pCacheListInfo;
	
	Uint32 nPathCheckSum = UMemory::Checksum(inPathData, inPathSize);
	
	while (inCacheList.GetNext(pCacheListInfo, i))
	{
		if (pCacheListInfo && pCacheListInfo->nPathCheckSum == nPathCheckSum && pCacheListInfo->nPathSize == inPathSize && 
			UMemory::Equal(pCacheListInfo->pPathData, inPathData, inPathSize))
		{
			pCacheListInfo->bChanged = true;
			return true;
		}
	}
	
	return false;
}

void CMyCacheList::ClearList(CPtrList<SMyCacheListInfo>& inCacheList)
{
	Uint32 i = 0;
	SMyCacheListInfo *pCacheListInfo;
	
	while (inCacheList.GetNext(pCacheListInfo, i))
	{
		UMemory::Dispose(pCacheListInfo->pPathData);
		delete pCacheListInfo;
	}

	inCacheList.Clear();
}

/* ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ */
#pragma mark -

enum {
	kHttp_IDVersion			= 2,
	kHttp_MaxIDs			= 250
};

CHttpIDList::CHttpIDList()
{
}

CHttpIDList::~CHttpIDList()
{
	Uint32 i = 0;
	Uint8 *psHttpID;
		
	while (mIDList.GetNext(psHttpID, i))
		UMemory::Dispose((TPtr)psHttpID);

	mIDList.Clear();	
}

bool CHttpIDList::AddID(const Uint8 *inHttpID)
{
	if (!inHttpID || !inHttpID[0])
		return false;
	
	const Uint8 *inNameEnd = UMemory::SearchByte(':', inHttpID + 1, inHttpID[0]);
	if (!inNameEnd)
		return false;

	Uint32 i = 0;
	Uint8 *psHttpID;
		
	while (mIDList.GetNext(psHttpID, i))
	{
		const Uint8 *pNameEnd = UMemory::SearchByte(':', psHttpID + 1, psHttpID[0]);
		if (!pNameEnd)
			continue;
			
		if (!UText::CompareInsensitive(inHttpID + 1, inNameEnd - inHttpID - 1, psHttpID + 1, pNameEnd - psHttpID - 1))
		{
			// remove ID
			mIDList.RemoveItem(psHttpID);
			UMemory::Dispose((TPtr)psHttpID);
			
			i--;
		}	
	}
	
	// check empty ID
	if (inNameEnd - inHttpID >= inHttpID[0])
		return true;
	
	// check max count
	if (mIDList.GetItemCount() >= kHttp_MaxIDs)
	{
		// remove old ID
		psHttpID = mIDList.RemoveItem(1);
		UMemory::Dispose((TPtr)psHttpID);
	}

	// set new ID
	psHttpID = (Uint8 *)UMemory::New(inHttpID, inHttpID[0] + 1);
		
	// add new ID
	mIDList.AddItem(psHttpID);
	
	return true;
}

void CHttpIDList::AddIDList(const CPtrList<Uint8>& inIDList)
{
	Uint32 i = 0;
	Uint8 *psHttpID;
		
	while (inIDList.GetNext(psHttpID, i))
		AddID(psHttpID);	
}

const CPtrList<Uint8>& CHttpIDList::GetIDList()
{
	return mIDList;
}

// inFileRef must be open
bool CHttpIDList::WriteData(TFSRefObj* inFileRef, Uint32 inOffset)
{
	if (!inFileRef)
		return false;

	Uint32 nOffset = inOffset;
	
	try
	{
		// write version number
		Uint16 nVersion = TB((Uint16)kHttp_IDVersion);
		nOffset += inFileRef->Write(nOffset, &nVersion, sizeof(Uint16));

		// write count
		Uint32 nIDCount = 0;
		nOffset += inFileRef->Write(nOffset, &nIDCount, sizeof(Uint32));
		
		Uint32 i = 0;	
		Uint8 *psHttpID;
		
		// write IDs
		while (mIDList.GetNext(psHttpID, i))
		{			
			if (psHttpID[0])
			{
				nOffset += inFileRef->Write(nOffset, psHttpID, psHttpID[0] + 1);			
				nIDCount++;
			}
		}
				
		if (nIDCount)
		{
			// write count
			nIDCount = TB(nIDCount);
			inFileRef->Write(inOffset + 2, &nIDCount, sizeof(Uint32));
		}
	}
	catch(...)
	{
		// nothing to do here
		throw;
	}
	
	return true;
}

// inFileRef must be open
bool CHttpIDList::ReadData(TFSRefObj* inFileRef, Uint32 inOffset)
{
	if (!inFileRef)
		return false;
	
	Uint8 *psHttpID = nil;
	Uint32 nOffset = inOffset;

	try
	{
		// read version number
		Uint16 nVersion;
		if (inFileRef->Read(nOffset, &nVersion, sizeof(Uint16)) != sizeof(Uint16))
			return false;
		
		nOffset += sizeof(Uint16);
		nVersion = TB(nVersion);
		if (nVersion != kHttp_IDVersion && nVersion != 1)
			return false;
				
		// read count
		Uint32 nIDCount;
		if (inFileRef->Read(nOffset, &nIDCount, sizeof(Uint32)) != sizeof(Uint32))
			return false;
		
		nOffset += sizeof(Uint32);
		nIDCount = TB(nIDCount);
		if (!nIDCount)
			return false;
		
		Uint8 nPsSize;
		if (nVersion == kHttp_IDVersion)
		{
			while (nIDCount--)
			{
				// read ID
				if (inFileRef->Read(nOffset, &nPsSize, 1) != 1)
					return false;
					
				nPsSize += 1;
				psHttpID = (Uint8 *)UMemory::New(nPsSize);

				if (inFileRef->Read(nOffset, psHttpID, nPsSize) != nPsSize)
				{
					UMemory::Dispose((TPtr)psHttpID);
					return false;
				}
				
				nOffset += nPsSize;
				
				// add new ID
				mIDList.AddItem(psHttpID);
				psHttpID = nil;
			}
		}
		else		// old version
		{
			while (nIDCount--)
			{
				// read data
				Uint32 nDataSize;
				if (inFileRef->Read(nOffset, &nDataSize, sizeof(Uint32)) != sizeof(Uint32))
					return false;
			
				nOffset += sizeof(Uint32);
				nDataSize = TB(nDataSize);

				void *pData = UMemory::New(nDataSize);	
				if (inFileRef->Read(nOffset, pData, nDataSize) != nDataSize)
				{
					UMemory::Dispose((TPtr)pData);
					return false;
				}
			
				nOffset += nDataSize;
		
				// get ID
				GetHttpIDFromData(pData, nDataSize);
				UMemory::Dispose((TPtr)pData);
								
				// skip host
				if (inFileRef->Read(nOffset, &nPsSize, 1) != 1)
					return false;
			
				nOffset += nPsSize + 1;

				// skip domain
				if (inFileRef->Read(nOffset, &nPsSize, 1) != 1)
					return false;
						
				nOffset += nPsSize + 1;
			}
		}
	}
	catch(...)
	{
		UMemory::Dispose((TPtr)psHttpID);
		throw;
	}	
	
	return true;
}

void CHttpIDList::GetHttpIDFromData(const void *inData, Uint32 inDataSize)
{
	if (!inData || !inDataSize)
		return;

	Uint8 psHttpID[256];
	
	const Uint8 *pDataEnd = (Uint8 *)inData + inDataSize;
	const Uint8 *pFieldBegin = (Uint8 *)inData;
	const Uint8 *pFieldEnd = nil;
			
	while (pFieldBegin < pDataEnd)
	{		
		pFieldEnd = UMemory::Search("\r\n", 2, pFieldBegin, pDataEnd - pFieldBegin);
		if (!pFieldEnd)
			break;

		Uint32 nFieldSize = pFieldEnd - pFieldBegin;
		if (nFieldSize > 255)
			nFieldSize = 255;
		
		// add ID
		psHttpID[0] = UMemory::Copy(psHttpID + 1, pFieldBegin, nFieldSize);
		AddID(psHttpID);

		pFieldBegin = pFieldEnd + 2;
	}			
}

/* ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ */
#pragma mark -

// takes ownership of inPathData
CMyPathData::CMyPathData(TPtr inPathData, Uint32 inPathSize, Uint32 inPathSum)
{
	mPathData = inPathData;
	mPathSize = inPathSize;
	mPathSum = inPathSum;
	
	if (inPathData && inPathSum == 0)
		mPathSum = UMemory::Checksum(mPathData, mPathSize);	
}

CMyPathData::~CMyPathData()
{
	if (mPathData)
		UMemory::Dispose(mPathData);
}

// takes ownership of inPathData
void CMyPathData::SetPathData(TPtr inPathData, Uint32 inPathSize, Uint32 inPathSum)
{
	UMemory::Dispose(mPathData);
	
	mPathData = inPathData;
	mPathSize = inPathSize;
	mPathSum = inPathSum;
	
	if (inPathData && inPathSum == 0)
		mPathSum = UMemory::Checksum(mPathData, mPathSize);
}

bool CMyPathData::IsPathEqual(const void *inData, Uint32 inDataSize)
{
	if (inDataSize != mPathSize) return false;
	return UMemory::Equal(mPathData, inData, mPathSize);
}

/* ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ */
#pragma mark -

CMySearchText::CMySearchText(CContainerView *inContainerView, const SRect& inBounds)
{
	mSearchTimer1 = nil;
	mSearchTimer2 = nil;
	
	// make search text
	CLabelView *lbl = new CLabelView(inContainerView, inBounds);
	lbl->SetFont(fd_Default9Bold);
	lbl->SetSizing(sizing_RightSticky);
	lbl->Show();
	searchText = lbl;	
}

CMySearchText::~CMySearchText()
{
	UTimer::Dispose(mSearchTimer1);
	UTimer::Dispose(mSearchTimer2);
}

bool CMySearchText::SearchText_KeyDown(const SKeyMsgData& inInfo)
{
	if (UText::IsPrinting(inInfo.keyChar))
	{
		Uint8 c = inInfo.keyChar;
		searchText->AppendText(&c, sizeof(c));
		
		if (!mSearchTimer1)
			mSearchTimer1 = UTimer::New(SearchTimer1Proc, this);
		
		mSearchTimer1->Start(1000, kOnceTimer);
		
		if (!mSearchTimer2)
			mSearchTimer2 = UTimer::New(SearchTimer2Proc, this);

		if (!mSearchTimer2->WasStarted())
			mSearchTimer2->Start(500, kRepeatingTimer);
		
		return true;
	}
	
	if (inInfo.keyChar == '\r' && searchText->GetTextSize())
	{
		mSearchTimer1->Stop();
		SearchTimer1Proc(this, nil, msg_Timer, nil, 0);
		
		return true;
	}
	
	return false;
}

void CMySearchText::SearchTimer1Proc(void *inContext, void *inObject, Uint32 inMsg, const void *inData, Uint32 inDataSize)
{
	#pragma unused(inObject, inData, inDataSize)
	
	if (inMsg == msg_Timer)
	{
		CMySearchText *pSearchText = (CMySearchText *)inContext;
		
		pSearchText->mSearchTimer2->Stop();
		
		Uint8 psText[256];
		psText[0] = pSearchText->searchText->GetText(psText + 1, sizeof(psText) - 1);
		if (psText[0])
		{
			pSearchText->searchText->SetText(nil, 0);
			pSearchText->SearchText(psText);
		}
	}
}

void CMySearchText::SearchTimer2Proc(void *inContext, void *inObject, Uint32 inMsg, const void *inData, Uint32 inDataSize)
{
	#pragma unused(inObject, inData, inDataSize)
	
	if (inMsg == msg_Timer)
	{
		CMySearchText *pSearchText = (CMySearchText *)inContext;
		
		Uint8 psText[256];
		psText[0] = pSearchText->searchText->GetText(psText + 1, sizeof(psText) - 1);
		if (psText[0])
			pSearchText->SearchText(psText);
	}
}

/* ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ */
#pragma mark -

void EventCallBack()
{
	static Uint8 nDepth = 0;
	
	if (nDepth < 5)
	{
		nDepth++;
		UApplication::Process();
		nDepth--;
	}		
}

void InitSubsTable(Uint8 *outTab)
{
	for (Uint32 i=0; i<256; i++)
		outTab[i] = i;
}

void BuildSubsTable(const Uint8 *inCharsToScram, Uint8 inNumA, Uint8 inNumB, Uint8 inNumC, Uint8 *ioTab)
{
	Uint8 unusedChars[256];
	Uint32 numA, numB, numC, maxTries, n, i;
	Uint8 *p, *ep, *sp, *tp;
	Uint8 c;
	
	UMemory::Copy(unusedChars, inCharsToScram, inCharsToScram[0]+1);
	
	numA = inNumA > 9 ? 9 : inNumA;
	numB = inNumB > 9 ? 9 : inNumB;
	numC = inNumC > 9 ? 9 : inNumC;
	
	p = sp = unusedChars + 1;
	ep = p + unusedChars[0] - 1;

	maxTries = 10 + numA + numB + numC;
	
	for (i=1; i<=inCharsToScram[0]; i++)
	{
		c = 0;
		
		n = maxTries;
		while (n--)
		{
			p += numA;
			if (p > ep) p = sp + numB;
			if (*p == 0) continue;
			p += (*p * numC) / 102;
			if (p > ep) p = sp + numC;
			
			if (*p)
			{
				c = *p;
				*p = 0;
				break;
			}
		}
		
		if (c == 0)
		{
			n = unusedChars[0];
			tp = sp;
			while (n--)
			{
				if (*tp)
				{
					c = *tp;
					*tp = 0;
					break;
				}
				tp++;
			}
		}
		
		ioTab[inCharsToScram[i]] = c;
	}
}

void SubsData(const Uint8 *inTab, void *ioData, Uint32 inDataSize)
{
	Uint8 *p = (Uint8 *)ioData;
	
	while (inDataSize--)
	{
		*p = inTab[*p];
		p++;
	}
}

void ReverseSubsTable(const Uint8 *inTab, Uint8 *outTab)
{
	Uint32 i;
	
	for (i=0; i<255; i++)
		outTab[inTab[i]] = i;
}

void GenerateRandomPassword(void *outData, Uint32 inSize)
{
	const Uint8 chars[] = "\pabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	Uint8 *p, *ep;
	
	if (inSize)
	{
		p = (Uint8 *)outData;
		ep = p + inSize;
		
		while (p != ep)
			*p++ = chars[UMath::GetRandom(1, chars[0])];
	}
}

