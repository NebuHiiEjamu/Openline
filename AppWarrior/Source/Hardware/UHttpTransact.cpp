/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "UHttpTransact.h"

/*
 * Constants
 */

enum {
	kHttp_CookieVersion		= 1,
	kHttp_MaxCookies		= 250
};


/*
 * Structures and Types
 */
 
struct SHttpTransact {
	TTransport tpt;
	TMessageProc msgProc;
	void *msgProcContext;
#if WIN32
	TTimer closeTimer;
#endif

	void *pBuffer;
	Uint32 nBufferSize;
	Uint32 nHeaderSize;
	Uint32 nDataSize;
	Int8 csType[32];
				
	Uint8 nProtocol;
	Uint8 psHost[256];
	Uint8 psDomain[256];
	Uint8 psLocation[256];
	Uint8 psLicense[256];
	
	Uint8 psLastModified[64];
	Uint32 nLength;
	
	Uint8 psReferer[64];
	Uint8 psUserAgentServer[128];
	Uint8 psCustomField[256];
	CPtrList<Uint8> lHttpIDList;
	
	Uint16 isChunked		: 1;
	Uint16 isComplete		: 1;
	Uint16 isError			: 1;
	Uint16 isNotModified	: 1;
	Uint16 isGetHeader		: 1;
	Uint16 isHeaderEnd		: 1;
};

#define TRN		((SHttpTransact *)inTrn)

/*
 * Function Prototypes
 */

static void _HttpMessageHandler(void *inContext, void *inObject, Uint32 inMsg, const void *inData, Uint32 inDataSize);
#if WIN32
static void _HttpConnectionCloseTimerHandler(void *inContext, void *inObject, Uint32 inMsg, const void *inData, Uint32 inDataSize);
#endif

static void _StartHttpConnect(THttpTransact inTrn, Uint16 inPort, Uint8 *inAddress, Uint32 inMaxSecs);
static bool _CheckAddress(THttpTransact inTrn, const Uint8 *inAddress, bool& outSameHost);
static const Uint8 *_SearchDomain(const Uint8 *inAddress, Uint32 inAddressSize);
static bool _IsUrlInvalidChar(Uint8 inChar);
static bool _IsUrlParamDelimiterChar(Uint8 inChar);
static bool _SkipRequestFields(THttpTransact inTrn);
static void *_ComposeRequestData(THttpTransact inTrn, const void *inData, Uint32 inDataSize);
static void *_ComposeRequestChunks(THttpTransact inTrn, const void *inData, Uint32 inDataSize, bool inLastData = false);
static void _AddResponseHttpID(THttpTransact inTrn, const void *inData, Uint32 inDataSize);
static bool _SkipResponseFields(THttpTransact inTrn);
static bool _SkipResponseChunks(THttpTransact inTrn);

static bool _AddResponseField_Http(THttpTransact inTrn);
static bool _AddResponseField_Server(THttpTransact inTrn, const Uint8 *inServer);
static bool _AddResponseField_ContentLength(THttpTransact inTrn, Uint32 inContentLength);
static bool _AddResponseField_Connection(THttpTransact inTrn);
static bool _AddResponseField_TransferEncoding(THttpTransact inTrn);
static bool _AddResponseField_ContentType(THttpTransact inTrn);

static bool _AddRequestField_Get(THttpTransact inTrn, const Uint8 *inDomain, bool inGetHeader);
static bool _AddRequestField_IfModifiedSince(THttpTransact inTrn, const Uint8 *inLastModified, Uint32 inLength);
static bool _AddRequestField_Referer(THttpTransact inTrn, const Uint8 *inReferer);
static bool _AddRequestField_Connection(THttpTransact inTrn, bool inProxyConnection);
static bool _AddRequestField_UserAgent(THttpTransact inTrn, const Uint8 *inUserAgent);
static bool _AddRequestField_Pragma(THttpTransact inTrn, const Uint8 *inPragma = nil);
static bool _AddRequestField_Host(THttpTransact inTrn, const Uint8 *inHost);
static bool _AddRequestField_Accept(THttpTransact inTrn, const Uint8 *inAccept = nil);
static bool _AddRequestField_AcceptLanguage(THttpTransact inTrn, const Uint8 *inAcceptLanguage = nil);
static bool _AddRequestField_AcceptCharset(THttpTransact inTrn, const Uint8 *inAcceptCharset = nil);
static bool _AddRequestField_HttpCookie(THttpTransact inTrn);
static bool _AddRequestField_HttpID(THttpTransact inTrn);
static bool _AddRequestField_CustomField(THttpTransact inTrn, const Uint8 *inCustomField);

static bool _AddField_General(THttpTransact inTrn, const Uint8* inField, const Uint8 *inData, const Uint8 *inExtraData = nil);
static bool _AddField_End(THttpTransact inTrn);

static void _CleanDomain(Uint8 *inDomain);
static void _ClearHttpTransport(THttpTransact inTrn);
static void _ClearHttpBuffer(THttpTransact inTrn);
static void _ClearHttpMisc(THttpTransact inTrn);
static void _ClearHttpIDList(THttpTransact inTrn);

/*
 * Global Variables
 */

static CCookieList _gHttpCookieList;

/* -------------------------------------------------------------------------- */

THttpTransact UHttpTransact::New()
{
	SHttpTransact *trn = (SHttpTransact *)UMemory::NewClear(sizeof(SHttpTransact));
	trn->nProtocol = protocol_HTTP_1_0;
	
	return (THttpTransact)trn;
}

void UHttpTransact::Dispose(THttpTransact inTrn)
{
	if (inTrn)
	{
		if (TRN->msgProc)
			UApplication::FlushMessages(TRN->msgProc, TRN->msgProcContext, inTrn);
		
	#if WIN32
		if (TRN->closeTimer)
			UTimer::Dispose(TRN->closeTimer);
	#endif
		
		if (TRN->pBuffer)
			UMemory::Dispose((TPtr)TRN->pBuffer);
		
		_ClearHttpIDList(inTrn);
		
		if (TRN->tpt)
			UTransport::Dispose(TRN->tpt);
		
		UMemory::Dispose((TPtr)inTrn);
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void UHttpTransact::SetMessageHandler(THttpTransact inTrn, TMessageProc inProc, void *inContext)
{
	Require(inTrn);
	
	TRN->msgProc = inProc;
	TRN->msgProcContext = inContext;
}

void UHttpTransact::PostMessage(THttpTransact inTrn, Uint32 inMsg, const void *inData, Uint32 inDataSize, Int16 inPriority)
{
	Require(inTrn);
	
	if (TRN->msgProc)
		UApplication::PostMessage(inMsg, inData, inDataSize, inPriority, TRN->msgProc, TRN->msgProcContext, inTrn);
}

void UHttpTransact::ReplaceMessage(THttpTransact inTrn, Uint32 inMsg, const void *inData, Uint32 inDataSize, Int16 inPriority)
{
	Require(inTrn);
	
	if (TRN->msgProc)
		UApplication::ReplaceMessage(inMsg, inData, inDataSize, inPriority, TRN->msgProc, TRN->msgProcContext, inTrn);
}


/* -------------------------------------------------------------------------- */
#pragma mark -

void UHttpTransact::MakeNewTransport(THttpTransact inTrn)
{
	Require(inTrn);
	
	_ClearHttpTransport(inTrn);
    
    try
    {
    	TRN->tpt = UTransport::New(transport_TCPIP);
    	UTransport::SetMessageHandler(TRN->tpt, _HttpMessageHandler, inTrn);
    }
    catch(...)
    {
    	UTransport::Dispose(TRN->tpt);
	  	TRN->tpt = nil;
    }
}

TTransport UHttpTransact::GetTransport(THttpTransact inTrn)
{
	Require(inTrn);
	
	return TRN->tpt;
}


/* -------------------------------------------------------------------------- */
#pragma mark -

bool UHttpTransact::IsConnected(THttpTransact inTrn)
{
	Require(inTrn);
	
	if (TRN->tpt)
		return UTransport::IsConnected(TRN->tpt);
		
	return false;
}

bool UHttpTransact::StartConnect(THttpTransact inTrn, const Uint8 *inAddress, Uint8 inProtocol, bool inKeepAlive, bool inGetHeader, Uint32 inMaxSecs)
{
	_ClearHttpBuffer(inTrn);
	
	TRN->isChunked = false;
	TRN->isComplete = false;
	TRN->isError = false;
	TRN->isNotModified = false;
	TRN->isGetHeader = inGetHeader;

	if (inProtocol == protocol_HTTP_1_0 || inProtocol == protocol_HTTP_1_1)
		TRN->nProtocol = inProtocol;

	if (!TRN->msgProc)
		return false;

	bool bSameHost;
	if (!_CheckAddress(inTrn, inAddress, bSameHost))
		return false;
	
	bool bProxyConnection = false;
	
	Uint8 psProxyAddr[256];
	psProxyAddr[0] = UTransport::GetProxyServerAddress(proxy_HttpAddress, psProxyAddr + 1, sizeof(psProxyAddr) - 1, TRN->psHost + 1, TRN->psHost[0]);
	Uint32 nProxyPort = port_number_HTTP;
	
	if (psProxyAddr[0])
	{
		bProxyConnection = true;
		Uint32 nTempProxyPort = UTransport::GetPortFromAddressText(kInternetNameAddressType, psProxyAddr + 1, psProxyAddr[0]);
		
		if (nTempProxyPort)
		{
			nProxyPort = nTempProxyPort;
			
			Uint8 *pProxyPort = UMemory::SearchByte(':', psProxyAddr + 1, psProxyAddr[0]);
			if (pProxyPort) 
				psProxyAddr[0] -= psProxyAddr + psProxyAddr[0] + 1 - pProxyPort;
		}
	}
	
	Uint8 psCompleteAddress[256];
	if (bProxyConnection)
	{
		psCompleteAddress[0] = UMemory::Copy(psCompleteAddress + 1, "http://", 7);
		psCompleteAddress[0] += UMemory::Copy(psCompleteAddress + psCompleteAddress[0] + 1, TRN->psHost + 1, psCompleteAddress[0] + TRN->psHost[0] > 255 ? 255 - psCompleteAddress[0] : TRN->psHost[0]);
		psCompleteAddress[0] += UMemory::Copy(psCompleteAddress + psCompleteAddress[0] + 1, TRN->psDomain + 1, psCompleteAddress[0] + TRN->psDomain[0] > 255 ? 255 - psCompleteAddress[0] : TRN->psDomain[0]);	
	}
		
	if ((bProxyConnection && !_AddRequestField_Get(inTrn, psCompleteAddress, inGetHeader)) || (!bProxyConnection && !_AddRequestField_Get(inTrn, TRN->psDomain, inGetHeader)) ||
		!_AddRequestField_IfModifiedSince(inTrn, TRN->psLastModified, TRN->nLength) || !_AddRequestField_Referer(inTrn, TRN->psReferer) || 
		(inKeepAlive && !_AddRequestField_Connection(inTrn, bProxyConnection)) || !_AddRequestField_UserAgent(inTrn, TRN->psUserAgentServer) ||
		!_AddRequestField_Pragma(inTrn) || !_AddRequestField_Host(inTrn, TRN->psHost) || !_AddRequestField_Accept(inTrn) || 
		!_AddRequestField_AcceptLanguage(inTrn) || !_AddRequestField_AcceptCharset(inTrn) || !_AddRequestField_HttpCookie(inTrn) ||
		!_AddRequestField_HttpID(inTrn) || !_AddRequestField_CustomField(inTrn, TRN->psCustomField) || !_AddField_End(inTrn))
	{
		_ClearHttpBuffer(inTrn);
		_ClearHttpMisc(inTrn);
		
		return false;
	}

	_ClearHttpMisc(inTrn);

	if (!TRN->tpt || !TRN->tpt->IsConnected() || !bSameHost)
	{
		if (psProxyAddr[0])
			_StartHttpConnect(inTrn, nProxyPort, psProxyAddr, inMaxSecs);
		else
			_StartHttpConnect(inTrn, port_number_HTTP, TRN->psHost, inMaxSecs);
	}
	else
		UApplication::PostMessage(msg_ConnectionEstablished, nil, 0, priority_Normal, _HttpMessageHandler, inTrn, nil);

	return true;
}

void UHttpTransact::Disconnect(THttpTransact inTrn)
{
	Require(inTrn);
	
	if (TRN->tpt)
		UTransport::Disconnect(TRN->tpt);
}

void UHttpTransact::StartDisconnect(THttpTransact inTrn)
{
	Require(inTrn);

	if (TRN->tpt)
		UTransport::StartDisconnect(TRN->tpt);
}

bool UHttpTransact::IsDisconnecting(THttpTransact inTrn)
{
	Require(inTrn);
	
	if (TRN->tpt)
		return UTransport::IsDisconnecting(TRN->tpt);
		
	return false;
}

bool UHttpTransact::IsComplete(THttpTransact inTrn)
{
	Require(inTrn);

	return TRN->isComplete;
}

bool UHttpTransact::IsError(THttpTransact inTrn)
{
	Require(inTrn);

	return TRN->isError;
}

bool UHttpTransact::IsNotModified(THttpTransact inTrn)
{
	Require(inTrn);

	return TRN->isNotModified;
}


/* -------------------------------------------------------------------------- */
#pragma mark -

void UHttpTransact::Listen(THttpTransact inTrn, const void *inAddr, Uint32 inAddrSize)
{
	Require(inTrn);
	
	UHttpTransact::MakeNewTransport(inTrn);	
	UTransport::Listen(TRN->tpt, inAddr, inAddrSize);
}

THttpTransact UHttpTransact::Accept(THttpTransact inTrn, void *outAddr, Uint32 *ioAddrSize)
{
	Require(inTrn);
	
	TTransport tpt = UTransport::Accept(TRN->tpt, outAddr, ioAddrSize);
	if (tpt == nil) return nil;
	
	SHttpTransact *trn = nil;

	try
	{
		trn = (SHttpTransact *)UMemory::NewClear(sizeof(SHttpTransact));
		trn->tpt = tpt;
		UTransport::SetMessageHandler(trn->tpt, _HttpMessageHandler, trn);
		trn->nProtocol = protocol_HTTP_1_0;
	}
	catch(...)
	{
		UTransport::Dispose(tpt);
		UMemory::Dispose((TPtr)trn);
		throw;
	}
	
	return (THttpTransact)trn;
}


/* -------------------------------------------------------------------------- */
#pragma mark -

bool UHttpTransact::GetUrl(THttpTransact inTrn, Uint8 *outUrl, Uint32 inMaxSize)
{
	Require(inTrn);
	
	if (!TRN->psHost[0] || !outUrl || !inMaxSize)
		return false;
	
	outUrl[0] = UText::Format(outUrl + 1, inMaxSize - 1, "http://%#s%#s", TRN->psHost, TRN->psDomain);
	return true;
}

bool UHttpTransact::GetHost(THttpTransact inTrn, Uint8 *outHost, Uint32 inMaxSize)
{
	Require(inTrn);
	
	if (!TRN->psHost[0] || !outHost || !inMaxSize)
		return false;
		
	outHost[0] = UMemory::Copy(outHost + 1, TRN->psHost + 1, TRN->psHost[0] > inMaxSize - 1 ? inMaxSize - 1 : TRN->psHost[0]);
	return true;
}

bool UHttpTransact::GetDomain(THttpTransact inTrn, Uint8 *outDomain, Uint32 inMaxSize)
{
	Require(inTrn);
	
	if (!TRN->psDomain[0] || !outDomain || !inMaxSize)
		return false;
		
	outDomain[0] = UMemory::Copy(outDomain + 1, TRN->psDomain + 1, TRN->psDomain[0] > inMaxSize - 1 ? inMaxSize - 1 : TRN->psDomain[0]);
	return true;
}

bool UHttpTransact::GetLocation(THttpTransact inTrn, Uint8 *outLocation, Uint32 inMaxSize)
{
	Require(inTrn);
	
	if (!TRN->psLocation[0] || !outLocation || !inMaxSize)
		return false;
		
	outLocation[0] = UMemory::Copy(outLocation + 1, TRN->psLocation + 1, TRN->psLocation[0] > inMaxSize - 1 ? inMaxSize - 1 : TRN->psLocation[0]);
	return true;
}

bool UHttpTransact::GetLicense(THttpTransact inTrn, Uint8 *outLicense, Uint32 inMaxSize)
{
	Require(inTrn);
	
	if (!TRN->psLicense[0] || !outLicense || !inMaxSize)
		return false;
		
	outLicense[0] = UMemory::Copy(outLicense + 1, TRN->psLicense + 1, TRN->psLicense[0] > inMaxSize - 1 ? inMaxSize - 1 : TRN->psLicense[0]);
	return true;
}

void UHttpTransact::SetLastModified(THttpTransact inTrn, Uint8 *inLastModified, Uint32 inLength)
{
	Require(inTrn);
	
	TRN->psLastModified[0] = 0;
	TRN->nLength = 0;
	
	if (!inLastModified || !inLastModified[0])
		return;
	
	TRN->psLastModified[0] = UMemory::Copy(TRN->psLastModified + 1, inLastModified + 1, inLastModified[0] > sizeof(TRN->psLastModified) - 1 ? sizeof(TRN->psLastModified) - 1 : inLastModified[0]);
	TRN->nLength = inLength;
}

bool UHttpTransact::GetLastModified(THttpTransact inTrn, Uint8 *outLastModified, Uint32 inMaxSize)
{
	Require(inTrn);
	
	if (!TRN->psLastModified[0] || !outLastModified || !inMaxSize)
		return false;
		
	outLastModified[0] = UMemory::Copy(outLastModified + 1, TRN->psLastModified + 1, TRN->psLastModified[0] > inMaxSize - 1 ? inMaxSize - 1 : TRN->psLastModified[0]);
	return true;
}

void UHttpTransact::SetReferer(THttpTransact inTrn, const Uint8 *inReferer)
{
	Require(inTrn);
	
	TRN->psReferer[0] = 0;

	if (!inReferer || !inReferer[0])
		return;
	
	TRN->psReferer[0] = UMemory::Copy(TRN->psReferer + 1, inReferer + 1, inReferer[0] > sizeof(TRN->psReferer) - 1 ? sizeof(TRN->psReferer) - 1 : inReferer[0]);
}

void UHttpTransact::SetUserAgentServer(THttpTransact inTrn, const Uint8 *inUserAgentServer)
{
	Require(inTrn);
	
	TRN->psUserAgentServer[0] = 0;
	
	if (!inUserAgentServer || !inUserAgentServer[0])
		return;
	
	TRN->psUserAgentServer[0] = UMemory::Copy(TRN->psUserAgentServer + 1, inUserAgentServer + 1, inUserAgentServer[0] > sizeof(TRN->psUserAgentServer) - 1 ? sizeof(TRN->psUserAgentServer) - 1 : inUserAgentServer[0]);
}

void UHttpTransact::SetCustomField(THttpTransact inTrn, const Uint8 *inCustomField)
{
	Require(inTrn);
	
	TRN->psCustomField[0] = 0;
	
	if (!inCustomField || !inCustomField[0])
		return;
	
	TRN->psCustomField[0] = UMemory::Copy(TRN->psCustomField + 1, inCustomField + 1, inCustomField[0] > sizeof(TRN->psCustomField) - 1 ? sizeof(TRN->psCustomField) - 1 : inCustomField[0]);
}

void UHttpTransact::SetHttpIDList(THttpTransact inTrn, const CPtrList<Uint8>& inHttpIDList)
{
	Require(inTrn);

	_ClearHttpIDList(inTrn);

	Uint32 i = 0;
	Uint8 *psHttpID;
	
	while (inHttpIDList.GetNext(psHttpID, i))
	{
		Uint8 *psNewHttpID = (Uint8 *)UMemory::New(psHttpID, psHttpID[0] + 1);
		TRN->lHttpIDList.AddItem(psNewHttpID);
	}
}

const CPtrList<Uint8>& UHttpTransact::GetHttpIDList(THttpTransact inTrn)
{
	Require(inTrn);

	return TRN->lHttpIDList;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

// inLastData is ignored if we use protocol_HTTP_1_0
// inTotalDataSize is ignored if we use protocol_HTTP_1_1
bool UHttpTransact::SendHttpHeader(THttpTransact inTrn, const void *inData, Uint32 inDataSize, const Int8 *inType, Uint32 inTotalDataSize, bool inLastData)
{
	Require(inTrn);
	_ClearHttpBuffer(inTrn);
	
	if (!TRN->tpt || !inData || !inDataSize)
		return false;
			
	Uint32 nTypeSize = UMemory::Copy(TRN->csType, inType, strlen(inType) > sizeof(TRN->csType) - 1 ? sizeof(TRN->csType) - 1 : strlen(inType));
	*(TRN->csType + nTypeSize) = 0;
	
	if (!_AddResponseField_Http(inTrn) || !_AddResponseField_Server(inTrn, TRN->psUserAgentServer) || 
	    (TRN->nProtocol == protocol_HTTP_1_0 && !_AddResponseField_ContentLength(inTrn, inTotalDataSize)) ||
		!_AddResponseField_Connection(inTrn) || (TRN->nProtocol == protocol_HTTP_1_1 && !_AddResponseField_TransferEncoding(inTrn)) ||
		!_AddResponseField_ContentType(inTrn) ||  (TRN->nProtocol == protocol_HTTP_1_0 && !_AddField_End(inTrn)))
	{
		_ClearHttpBuffer(inTrn);
		return false;
	}

	void *pTransportBuf = nil;
	if (TRN->nProtocol == protocol_HTTP_1_0)
		pTransportBuf = _ComposeRequestData(inTrn, inData, inDataSize);
	else if (TRN->nProtocol == protocol_HTTP_1_1)
		pTransportBuf = _ComposeRequestChunks(inTrn, inData, inDataSize, inLastData);

	if (!pTransportBuf)
		return false;
	
	try
	{		
		UTransport::SendBuffer(TRN->tpt, pTransportBuf);
	}
	catch(...)
	{
		_ClearHttpBuffer(inTrn);
		UTransport::DisposeBuffer(pTransportBuf);
		throw;
	}

	_ClearHttpBuffer(inTrn);
	return true;
}

// inLastData is ignored if we use protocol_HTTP_1_0
bool UHttpTransact::SendHttpData(THttpTransact inTrn, const void *inData, Uint32 inDataSize, bool inLastData)
{
	Require(inTrn);

	if (!TRN->tpt || !inData || !inDataSize)
		return false;
	
	_ClearHttpBuffer(inTrn);
	
	void *pTransportBuf = nil;
	if (TRN->nProtocol == protocol_HTTP_1_0)
		pTransportBuf = _ComposeRequestData(inTrn, inData, inDataSize);
	else if (TRN->nProtocol == protocol_HTTP_1_1)
		pTransportBuf = _ComposeRequestChunks(inTrn, inData, inDataSize, inLastData);

	if (!pTransportBuf)
		return false;

	try
	{		
		UTransport::SendBuffer(TRN->tpt, pTransportBuf);
	}
	catch(...)
	{
		UTransport::DisposeBuffer(pTransportBuf);
		throw;
	}
	
	return true;
}

Uint32 UHttpTransact::GetUnsentSize(THttpTransact inTrn)
{
	Require(inTrn);
	
	return UTransport::GetUnsentSize(TRN->tpt);
}

// outType must have 32 chars
void *UHttpTransact::ReceiveHttpData(THttpTransact inTrn, Uint32& outDataSize, Int8 *outType)
{	
	Require(inTrn);
	
	if (!TRN->pBuffer || !TRN->isHeaderEnd)
	{
		outDataSize = 0;
		return nil;
	}
	
	outDataSize = TRN->nBufferSize - TRN->nHeaderSize;
	
	if (outType)
		UMemory::Copy(outType, TRN->csType, strlen(TRN->csType) + 1);
	
	return (Uint8*)TRN->pBuffer + TRN->nHeaderSize;
}

void UHttpTransact::PurgeReceiveBuffer(THttpTransact inTrn)
{
	if (!TRN->isHeaderEnd)
		return;
		
	UMemory::Dispose((TPtr)TRN->pBuffer);
	TRN->pBuffer = nil;
	TRN->nBufferSize = 0;
	TRN->nHeaderSize = 0;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

// inExpiredDate must be in local time
bool UHttpTransact::AddHttpCookie(const Uint8 *inHost, const Uint8 *inDomain, const void *inData, Uint32 inDataSize, const SCalendarDate& inExpiredDate)
{
	return _gHttpCookieList.AddCookie(inHost, inDomain, inData, inDataSize,	inExpiredDate);
}

void *UHttpTransact::GetHttpCookie(const Uint8 *inHost, const Uint8 *inDomain, Uint32& outDataSize)
{
	return _gHttpCookieList.GetCookie(inHost, inDomain, outDataSize);
}

bool UHttpTransact::WriteHttpCookies(TFSRefObj* inFileRef)
{
	return _gHttpCookieList.WriteData(inFileRef);
}

bool UHttpTransact::ReadHttpCookies(TFSRefObj* inFileRef)
{
	return _gHttpCookieList.ReadData(inFileRef);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

static void _HttpMessageHandler(void *inContext, void */*inObject*/, Uint32 inMsg, const void *inData, Uint32 inDataSize)
{
	SHttpTransact *pHttpTransact = (SHttpTransact *)inContext;
	
	switch (inMsg)
	{
		case msg_ConnectionEstablished:
			if (pHttpTransact->pBuffer && pHttpTransact->tpt && pHttpTransact->tpt->IsConnected())
			{
				pHttpTransact->tpt->Send(pHttpTransact->pBuffer, pHttpTransact->nBufferSize);
				_ClearHttpBuffer((THttpTransact)pHttpTransact);
			}
			break;

		case msg_ConnectionRefused:
			_ClearHttpTransport((THttpTransact)pHttpTransact);
			_ClearHttpBuffer((THttpTransact)pHttpTransact);
			break;

		case msg_DataArrived:
		case msg_ConnectionClosed:		
			if (pHttpTransact->tpt && !pHttpTransact->isComplete)
			{
				Uint32 nRecSize = pHttpTransact->tpt->GetReceiveSize();
		    	if (!nRecSize)
		    	{
    				if (!pHttpTransact->tpt->IsConnected())
    				{
    				#if WIN32	// sometimes on Win98 msg_ConnectionClosed arrives before msg_DataArrived
    					if (pHttpTransact->nHeaderSize)
		    				pHttpTransact->isComplete = true;
		    			else if (!pHttpTransact->closeTimer)
		    			{
		    				pHttpTransact->closeTimer = UTimer::New(_HttpConnectionCloseTimerHandler, pHttpTransact);
							UTimer::Start(pHttpTransact->closeTimer, 5000, kOnceTimer);	// wait 5 more secs for data
		    				return; 													// don't post the message
		    			}
		    		#else
			    		pHttpTransact->isComplete = true;
		    		#endif
		    		}
		    	
		    		goto checkIsComplete;
				}
			
				try
				{
					if (!pHttpTransact->pBuffer)
						pHttpTransact->pBuffer = UMemory::NewClear(nRecSize);
					else
						pHttpTransact->pBuffer = UMemory::Reallocate((TPtr)pHttpTransact->pBuffer, pHttpTransact->nBufferSize + nRecSize);
				} 
				catch(...)
				{
					_ClearHttpTransport((THttpTransact)pHttpTransact);
					_ClearHttpBuffer((THttpTransact)pHttpTransact);
					pHttpTransact->isComplete = true;
				
					goto checkIsComplete;
    			}
    		
    			pHttpTransact->nBufferSize += pHttpTransact->tpt->Receive((Uint8 *)pHttpTransact->pBuffer + pHttpTransact->nBufferSize, nRecSize);
    		
	    		_SkipRequestFields((THttpTransact)pHttpTransact);
	    		_SkipResponseFields((THttpTransact)pHttpTransact);
    		
    			if (pHttpTransact->isChunked)
    			{
   					if (_SkipResponseChunks((THttpTransact)pHttpTransact))
	   					pHttpTransact->isComplete = true;
    			}
    			else if (pHttpTransact->nDataSize && pHttpTransact->nBufferSize >= pHttpTransact->nHeaderSize + pHttpTransact->nDataSize)
    				pHttpTransact->isComplete = true;
    			
    			if (!pHttpTransact->isComplete && !pHttpTransact->tpt->IsConnected())
    				pHttpTransact->isComplete = true;
    				
checkIsComplete:
    			if (pHttpTransact->isComplete)
    			{    			
				#if WIN32
					if (pHttpTransact->closeTimer)
					{
						UTimer::Dispose(pHttpTransact->closeTimer);
						pHttpTransact->closeTimer = nil;
					}
				#endif
    				
    				UHttpTransact::PostMessage((THttpTransact)pHttpTransact, msg_ConnectionClosed, nil, 0, priority_Normal);
					return;
    			}
    		}		
    		break;
    		
//		case msg_DataTimeOut:
//		case msg_ReadyToSend:
//		case msg_ConnectionRequest:
    };

	UHttpTransact::PostMessage((THttpTransact)pHttpTransact, inMsg, inData, inDataSize, priority_Normal);
}

#if WIN32
static void _HttpConnectionCloseTimerHandler(void *inContext, void *inObject, Uint32 inMsg, const void *inData, Uint32 inDataSize)
{
	#pragma unused(inObject, inData, inDataSize)

	if (inMsg == msg_Timer)
	{
		SHttpTransact *pHttpTransact = (SHttpTransact *)inContext;

		pHttpTransact->isComplete = true;
		UHttpTransact::PostMessage((THttpTransact)pHttpTransact, msg_ConnectionClosed, nil, 0, priority_Normal);
	}
}
#endif

/* -------------------------------------------------------------------------- */
#pragma mark -

static void _StartHttpConnect(THttpTransact inTrn, Uint16 inPort, Uint8 *inAddress, Uint32 inMaxSecs)
{
	Require(inTrn);

	SInternetNameAddress stAddress;
	stAddress.type = kInternetNameAddressType;
	stAddress.port = inPort;
	UMemory::Copy(stAddress.name, inAddress, inAddress[0] + 1);

	// a TTransport can only be connected ONCE so we must make a new one
	UHttpTransact::MakeNewTransport(inTrn);
	UTransport::StartConnect(TRN->tpt, &stAddress, stAddress.name[0] + 5, inMaxSecs);
}

static bool _CheckAddress(THttpTransact inTrn, const Uint8 *inAddress, bool& outSameHost)
{
	outSameHost = false;
	Require(inTrn);

	const Uint8 *pNewDomain = nil;
	Uint8 pOldDomain[256];
	pOldDomain[0] = 0;

	if (!TRN->psHost[0])
	{		
		if (inAddress[0] <= 7 || UText::CompareInsensitive(inAddress + 1, "http://", 7))
			return false;

		pNewDomain = _SearchDomain(inAddress + 8, inAddress[0] - 7);
				
		if (!pNewDomain || *pNewDomain != '/')
		{
			pOldDomain[0] = 1;
			pOldDomain[1] = '/';
		}

		if (!pNewDomain)
			pNewDomain = inAddress + inAddress[0] + 1;
	}
	else
	{
		if (inAddress[0] > 7 && !UText::CompareInsensitive(inAddress + 1, "http://", 7))
		{
			pNewDomain = _SearchDomain(inAddress + 8, inAddress[0] - 7);
				
			if (!pNewDomain || *pNewDomain != '/')
			{
				pOldDomain[0] = 1;
				pOldDomain[1] = '/';
			}

			if (!pNewDomain)
				pNewDomain = inAddress + inAddress[0] + 1;

			Uint8 oldHost[256];
			UMemory::Copy(oldHost, TRN->psHost , TRN->psHost[0] + 1);
			UText::MakeLowercase(oldHost + 1, oldHost[0]);

			Uint8 newHost[256];
			newHost[0] = UMemory::Copy(newHost + 1, inAddress + 8, pNewDomain - inAddress - 8);
			UText::MakeLowercase(newHost + 1, newHost[0]);
		
			if(!UMemory::Compare(oldHost + 1, oldHost[0], newHost + 1, newHost[0]))
				outSameHost = true;	
		}
		
		if (!pNewDomain)
		{
			// search domain end
			Uint8 nDomainCount = 0;
			if (TRN->psDomain[0])
			{
				do
				{
					Uint8 nChar = TRN->psDomain[nDomainCount + 1];
					if (_IsUrlInvalidChar(nChar) || _IsUrlParamDelimiterChar(nChar))
						break;
						
				} while (++nDomainCount < TRN->psDomain[0]);
			}
						
			// search address end
			Uint8 nAddressCount = 0;
			if (inAddress[0])
			{
				do
				{
					Uint8 nChar = inAddress[nAddressCount + 1];
					if (_IsUrlInvalidChar(nChar) || _IsUrlParamDelimiterChar(nChar))
						break;
						
				} while (++nAddressCount < inAddress[0]);
			}
			
			// search first domain delimiter
			if (nDomainCount)
				while (TRN->psDomain[nDomainCount--] != '/' && nDomainCount != 0){}
			
			while (nDomainCount && nAddressCount)
			{
				// search next address delimiter
				while (inAddress[nAddressCount--] != '/' && nAddressCount != 0){}

				// search next domain delimiter
				if (inAddress[nAddressCount + 1] == '/')
					while (TRN->psDomain[nDomainCount--] != '/' && nDomainCount != 0){}
			}
			
			if (nDomainCount)
				pOldDomain[0] = UMemory::Copy(pOldDomain + 1, TRN->psDomain + 1, nDomainCount); 

			if (!UMemory::Compare(inAddress + 1, "../", 3))
				pNewDomain = inAddress + 3;
			else
			{
				pNewDomain = inAddress + 1;

			 	if (UMemory::Compare(inAddress + 1, "/", 1))
				{
					pOldDomain[pOldDomain[0] + 1] = '/';
					pOldDomain[0] += 1;
				}
			}
				
			outSameHost = true;
		}
	}
	
	if (!pNewDomain)
		return false;
			
	if (!outSameHost)
		TRN->psHost[0] = UMemory::Copy(TRN->psHost + 1, inAddress + 8 , pNewDomain - inAddress - 8);

	TRN->psDomain[0] = 0;
	if (pOldDomain[0])
		UMemory::Copy(TRN->psDomain, pOldDomain, pOldDomain[0] + 1);
	
	TRN->psDomain[0] += UMemory::Copy(TRN->psDomain + TRN->psDomain[0] + 1, pNewDomain, inAddress[0] - (pNewDomain - inAddress - 1));

	return true;
}

static const Uint8 *_SearchDomain(const Uint8 *inAddress, Uint32 inAddressSize)
{
	Uint32 nCount = 0;
	while (nCount < inAddressSize)
	{
		Uint8 nChar = *(inAddress + nCount);
		if (_IsUrlInvalidChar(nChar) || _IsUrlParamDelimiterChar(nChar) || nChar == '/')
			return (inAddress + nCount);
		
		nCount++;
	}
	
	return nil;	
}

static bool _IsUrlInvalidChar(Uint8 inChar) 
{
	if (inChar == ' ' || inChar == '\t' || inChar == '\n' || inChar == '\r')
		return true;

	return false;
}

static bool _IsUrlParamDelimiterChar(Uint8 inChar) 
{
	if (inChar == '?' || inChar == '+' || inChar == '&')
		return true;

	return false;
}

static bool _SkipRequestFields(THttpTransact inTrn)
{
	Require(inTrn);

	if (TRN->isHeaderEnd || !TRN->pBuffer || !UText::CompareInsensitive(TRN->pBuffer, "HTTP/", 5))
		return false;

	if (UText::CompareInsensitive(TRN->pBuffer, "GET ", 4))
	{
		TRN->isHeaderEnd = true;
		TRN->isComplete = true;
		return false;
	}

	while (!TRN->isHeaderEnd)
	{
		Uint32 nReceivedDataSize = TRN->nBufferSize - TRN->nHeaderSize;
		Uint8 *pFieldBegin = (Uint8 *)TRN->pBuffer + TRN->nHeaderSize;
		Uint8 *pFieldEnd = UMemory::Search("\r\n", 2, pFieldBegin, (nReceivedDataSize > 256 ? 256 : nReceivedDataSize));

		if (!pFieldEnd)
			return false;
		
		Uint16 nFieldLength = pFieldEnd - pFieldBegin;
	
		if (nFieldLength > 11 && !UText::CompareInsensitive(pFieldBegin, "GET ", 4))
		{
			Uint8 *pDomainBegin = pFieldBegin + 4;
			if (*pDomainBegin == '/')
				pDomainBegin++;
				
			Uint8 *pDomainEnd = UText::SearchInsensitive(" HTTP", 5, pDomainBegin, pFieldEnd - pDomainBegin);
				
			if (pDomainEnd)
			{
				if (pDomainBegin != pDomainEnd)
				{
					TRN->psDomain[0] = UMemory::Copy(TRN->psDomain + 1, pDomainBegin, pDomainEnd - pDomainBegin);
					_CleanDomain(TRN->psDomain);
				}

				if (!UText::CompareInsensitive(pDomainEnd + 1, "HTTP/1.0", 8))
					TRN->nProtocol = protocol_HTTP_1_0;
				else if (!UText::CompareInsensitive(pDomainEnd + 1, "HTTP/1.1", 8))
					TRN->nProtocol = protocol_HTTP_1_1;
			}			
		}
		else if (nFieldLength > 6 && !UText::CompareInsensitive(pFieldBegin, "Host: ", 6))
		{
			TRN->psHost[0] = UMemory::Copy(TRN->psHost + 1, pFieldBegin + 6, nFieldLength - 6 > 255 ? 255 : nFieldLength - 6);
		}
	
		pFieldEnd += 2;
		TRN->nHeaderSize += pFieldEnd - pFieldBegin;
	
		if (!UMemory::Compare(pFieldEnd, "\r\n", 2))
		{		
			TRN->isHeaderEnd = true;
			TRN->isComplete = true;
		}	
	}
	
	return true;
}

static void *_ComposeRequestData(THttpTransact inTrn, const void *inData, Uint32 inDataSize)
{	
	Require(inTrn);
	
	if (!inData || !inDataSize)
		return nil;		

	void *pTransportBuf = UTransport::NewBuffer(TRN->nBufferSize + inDataSize);

	if (!pTransportBuf)
		return nil;

	if (TRN->pBuffer && TRN->nBufferSize)
		UMemory::Copy(pTransportBuf, TRN->pBuffer, TRN->nBufferSize);

	UMemory::Copy((Uint8 *)pTransportBuf + TRN->nBufferSize, inData, inDataSize);

	return pTransportBuf;
}

static void *_ComposeRequestChunks(THttpTransact inTrn, const void *inData, Uint32 inDataSize, bool inLastData)
{	
	Require(inTrn);
	
	if (!inData || !inDataSize)
		return nil;
		
	void *pBuffer = nil;
	Uint32 nBufferSize = 0;

	if (TRN->pBuffer && TRN->nBufferSize)
	{
		try
		{
			pBuffer = UMemory::New(TRN->nBufferSize);
		}
		catch(...)
		{
			// don't throw
			return nil;
		}
				
		UMemory::Copy(pBuffer, TRN->pBuffer, TRN->nBufferSize);
		nBufferSize = TRN->nBufferSize;
	}
		
	Uint32 nMaxChunkSize = 32768;	// 32K

	Uint32 nOffset = 0;
	do
	{
		Uint32 nChunkSize;
		if (nOffset + nMaxChunkSize > inDataSize)
			nChunkSize = inDataSize - nOffset;
		else
			nChunkSize = nMaxChunkSize;
	
		Uint8 psChunkSize[10];
		psChunkSize[0] = UText::Format(psChunkSize + 1, sizeof(psChunkSize) - 1, "\r\n%lX\r\n", nChunkSize);

		try
		{
			if (!pBuffer)
				pBuffer = UMemory::New(psChunkSize[0] + nChunkSize);
			else
				pBuffer = UMemory::Reallocate((TPtr)pBuffer, nBufferSize + psChunkSize[0] + nChunkSize);
		}
		catch(...)
		{	
			// don't throw
			return nil; 				
		}
		
		UMemory::Copy((Uint8 *)pBuffer + nBufferSize, psChunkSize + 1, psChunkSize[0]);
		UMemory::Copy((Uint8 *)pBuffer + nBufferSize + psChunkSize[0], (Uint8 *)inData + nOffset, nChunkSize);
		
		nOffset += nChunkSize;
		nBufferSize += psChunkSize[0] + nChunkSize;
		
	} while (nOffset < inDataSize);
					
	if (inLastData)
	{
		Uint8 psLastChunk[8];
		psLastChunk[0] = UMemory::Copy(psLastChunk + 1, "\r\n0\r\n\r\n", 7);
	
		pBuffer = UMemory::Reallocate((TPtr)pBuffer, nBufferSize + psLastChunk[0]);
			
		if (!pBuffer)
			return nil; 				

		UMemory::Copy((Uint8 *)pBuffer + nBufferSize, psLastChunk + 1, psLastChunk[0]);
		nBufferSize += psLastChunk[0];
	}

	void *pTransportBuf = UTransport::NewBuffer(nBufferSize);

	if (!pTransportBuf)
	{
		UMemory::Dispose((TPtr)pBuffer);
		return nil;
	}

	UMemory::Copy(pTransportBuf, pBuffer, nBufferSize);
	UMemory::Dispose((TPtr)pBuffer);

	return pTransportBuf;
}

static void _AddResponseHttpID(THttpTransact inTrn, const void *inData, Uint32 inDataSize)
{
	Require(inTrn);

	Uint32 nHttpIDSize = inDataSize > 255 ? 255 : inDataSize;
	
	Uint8 *psHttpID = (Uint8 *)UMemory::New(nHttpIDSize + 1);
	psHttpID[0] = UMemory::Copy(psHttpID + 1, inData, nHttpIDSize);
	
	TRN->lHttpIDList.AddItem(psHttpID);
}

static bool _SkipResponseFields(THttpTransact inTrn)
{
	Require(inTrn);

	if (TRN->isHeaderEnd || !TRN->pBuffer || !UText::CompareInsensitive(TRN->pBuffer, "GET ", 4))
		return false;

	if (UText::CompareInsensitive(TRN->pBuffer, "HTTP/", 5))
	{
		TRN->isHeaderEnd = true;
		if (TRN->isGetHeader)
			TRN->isComplete = true;

		return false;
	}

	if (UMemory::Compare((Uint8 *)TRN->pBuffer + 9, "2", 1))
	{	
		TRN->isError = true;
		
		if (!UMemory::Compare((Uint8 *)TRN->pBuffer + 9, "304", 3))
		{
			TRN->isComplete = true;
			TRN->isNotModified = true;
		}
	}

	while (!TRN->isHeaderEnd)
	{
		Uint32 nReceivedDataSize = TRN->nBufferSize - TRN->nHeaderSize;
		Uint8 *pFieldBegin = (Uint8 *)TRN->pBuffer + TRN->nHeaderSize;
		Uint8 *pFieldEnd = UMemory::Search("\r\n", 2, pFieldBegin, (nReceivedDataSize > 256 ? 256 : nReceivedDataSize));

		if (!pFieldEnd)
			return false;
		
		Uint16 nFieldLength = pFieldEnd - pFieldBegin;
		Uint8 nFieldNameLength = 0;		// sometimes we don't have a space after the field name
			
		if (!TRN->isChunked && nFieldLength > 15 && !UText::CompareInsensitive(pFieldBegin, "Content-Length:", 15))
		{
			nFieldNameLength = 15;
			if (*(pFieldBegin + nFieldNameLength) == ' ')
				nFieldNameLength++;

			TRN->nDataSize = UText::TextToInteger(pFieldBegin + nFieldNameLength, nFieldLength - nFieldNameLength);
			if (!TRN->nDataSize)
				TRN->isComplete = true;
		}
		else if (nFieldLength > 18 && !UText::CompareInsensitive(pFieldBegin, "Transfer-Encoding:", 18))
		{
			nFieldNameLength = 18;
			if (*(pFieldBegin + nFieldNameLength) == ' ')
				nFieldNameLength++;

			if (nFieldLength - nFieldNameLength >= 7 && !UText::CompareInsensitive(pFieldBegin + nFieldNameLength, "chunked", 7))
			{
				TRN->isChunked = true;
				TRN->nDataSize = 0;
			}
		}
		else if (nFieldLength > 9 && !UText::CompareInsensitive(pFieldBegin, "Location:", 9))
		{
			nFieldNameLength = 9;
			if (*(pFieldBegin + nFieldNameLength) == ' ')
				nFieldNameLength++;
			
			TRN->psLocation[0] = UMemory::Copy(TRN->psLocation + 1, pFieldBegin + nFieldNameLength, nFieldLength - nFieldNameLength > 255 ? 255 : nFieldLength - nFieldNameLength);
		}
		else if (nFieldLength > 14 && !UText::CompareInsensitive(pFieldBegin, "Last-Modified:", 14))
		{
			nFieldNameLength = 14;
			if (*(pFieldBegin + nFieldNameLength) == ' ')
				nFieldNameLength++;
	
			TRN->psLastModified[0] = UMemory::Copy(TRN->psLastModified + 1, pFieldBegin + nFieldNameLength, nFieldLength - nFieldNameLength > sizeof(TRN->psLastModified) - 1 ? sizeof(TRN->psLastModified) - 1 : nFieldLength - nFieldNameLength);
		}
		else if (*((Uint8 *)TRN->pBuffer + 9) == '2' && nFieldLength > 13 && !UText::CompareInsensitive(pFieldBegin, "Content-Type:", 13))
		{
			nFieldNameLength = 13;
			if (*(pFieldBegin + nFieldNameLength) == ' ')
				nFieldNameLength++;

			Uint32 nTypeSize = UMemory::Copy(TRN->csType, pFieldBegin + nFieldNameLength, nFieldLength - nFieldNameLength > sizeof(TRN->csType) - 1 ? sizeof(TRN->csType) - 1 : nFieldLength - nFieldNameLength);
			*(TRN->csType + nTypeSize) = 0;			
		}
		else if (nFieldLength > 11 && !UText::CompareInsensitive(pFieldBegin, "Set-Cookie:", 11))
		{
			nFieldNameLength = 11;
			if (*(pFieldBegin + nFieldNameLength) == ' ')
				nFieldNameLength++;

			Uint8 *pDataBegin = pFieldBegin + nFieldNameLength;
			Uint8 *pDataEnd = UMemory::SearchByte(';', pDataBegin, nFieldLength - nFieldNameLength);
			
			if (!pDataEnd) 
				pDataEnd = pFieldBegin + nFieldLength;

			if (pDataBegin != pDataEnd)
			{
				SHttpCookie *pHttpCookie = (SHttpCookie *)UMemory::NewClear(sizeof(SHttpCookie));
				
				pHttpCookie->nDataSize = pDataEnd - pDataBegin;
				pHttpCookie->pData = UMemory::New(pHttpCookie->nDataSize);
				UMemory::Copy(pHttpCookie->pData, pDataBegin, pHttpCookie->nDataSize);
				
				Uint8 *pSecureBegin = pDataEnd;
				Uint8 *pExpiresBegin = UText::SearchInsensitive("Expires=", 8, pDataEnd, nFieldLength - (pDataEnd - pFieldBegin));

				if (pExpiresBegin)
				{
					pExpiresBegin += 8;
					Uint8 *pExpiresEnd = UMemory::SearchByte(';', pExpiresBegin, nFieldLength - (pExpiresBegin - pFieldBegin));

					if (!pExpiresEnd)
						pExpiresEnd = pFieldBegin + nFieldLength;

					Uint8 psExpiredDate[64];
					psExpiredDate[0] = UMemory::Copy(psExpiredDate + 1, pExpiresBegin, pExpiresEnd - pExpiresBegin > sizeof(psExpiredDate) - 1 ? sizeof(psExpiredDate) - 1 : pExpiresEnd - pExpiresBegin);
					_gHttpCookieList.ConvertExpiredDate(psExpiredDate, pHttpCookie->stExpiredDate);
					
					if (pExpiresEnd > pSecureBegin)
						pSecureBegin = pExpiresEnd;
				}

				Uint8 *pHostBegin = UText::SearchInsensitive("Domain=", 7, pDataEnd, nFieldLength - (pDataEnd - pFieldBegin));

				if (pHostBegin)
				{
					pHostBegin += 7;
					Uint8 *pHostEnd = UMemory::SearchByte(';', pHostBegin, nFieldLength - (pHostBegin - pFieldBegin));
	
					if (!pHostEnd)
						pHostEnd = pFieldBegin + nFieldLength;

					Uint32 nHostSize = pHostEnd - pHostBegin > 255 ? 255 : pHostEnd - pHostBegin;
					pHttpCookie->psHost = (Uint8 *)UMemory::New(nHostSize + 1);
					pHttpCookie->psHost[0] = UMemory::Copy(pHttpCookie->psHost + 1, pHostBegin, nHostSize);
					
					if (pHostEnd > pSecureBegin)
						pSecureBegin = pHostEnd;
				}
				else
				{
					pHttpCookie->psHost = (Uint8 *)UMemory::New(TRN->psHost[0] + 1);
					UMemory::Copy(pHttpCookie->psHost, TRN->psHost, TRN->psHost[0] + 1);
				}
	
				Uint8 *pDomainBegin = UText::SearchInsensitive("Path=", 5, pDataEnd, nFieldLength - (pDataEnd - pFieldBegin));
	
				if (pDomainBegin)
				{
					pDomainBegin += 5;
					Uint8 *pDomainEnd = UMemory::SearchByte(';', pDomainBegin, nFieldLength - (pDomainBegin - pFieldBegin));

					if (!pDomainEnd)
						pDomainEnd = pFieldBegin + nFieldLength;

					Uint32 nDomainSize = pDomainEnd - pDomainBegin > 255 ? 255 : pDomainEnd - pDomainBegin;
					pHttpCookie->psDomain = (Uint8 *)UMemory::New(nDomainSize + 1);
					pHttpCookie->psDomain[0] = UMemory::Copy(pHttpCookie->psDomain + 1, pDomainBegin, nDomainSize);
						
					if (pDomainEnd > pSecureBegin)
						pSecureBegin = pDomainEnd;						
				}
				else
				{
					pHttpCookie->psDomain = (Uint8 *)UMemory::New(TRN->psDomain[0] + 1);
					UMemory::Copy(pHttpCookie->psDomain, TRN->psDomain, TRN->psDomain[0] + 1);
				}

				if (pSecureBegin != pFieldBegin + nFieldLength && UText::SearchInsensitive("secure", 6, pSecureBegin, nFieldLength - (pSecureBegin - pFieldBegin)))
					_gHttpCookieList.DisposeCookie(pHttpCookie);	// we don't support HTTPS (HTTP over SSL) so ignore secure cookies
				else			
					_gHttpCookieList.AddCookie(pHttpCookie);
			}
		}
		else if (nFieldLength > 16 && !UText::CompareInsensitive(pFieldBegin, "HL-License-Resp:", 16))
		{
			nFieldNameLength = 16;
			if (*(pFieldBegin + nFieldNameLength) == ' ')
				nFieldNameLength++;
	
			TRN->psLicense[0] = UMemory::Copy(TRN->psLicense + 1, pFieldBegin + nFieldNameLength, nFieldLength - nFieldNameLength > 255 ? 255 : nFieldLength - nFieldNameLength);
		}
		else if (nFieldLength > 3 && !UText::CompareInsensitive(pFieldBegin, "HL-", 3))
			_AddResponseHttpID(inTrn, pFieldBegin, nFieldLength);
		
		pFieldEnd += 2;
		TRN->nHeaderSize += pFieldEnd - pFieldBegin;
	
		if (!UMemory::Compare(pFieldEnd, "\r\n", 2))
		{
			if (!TRN->isChunked)
				TRN->nHeaderSize += 2;
		
			TRN->isHeaderEnd = true;
			if (TRN->isGetHeader)
				TRN->isComplete = true;
		}	
	}
	
	return true;
}

static bool _SkipResponseChunks(THttpTransact inTrn)
{
	Require(inTrn);

	while (TRN->nBufferSize >= TRN->nHeaderSize + TRN->nDataSize)
	{
		Uint8 *pFieldBegin = (Uint8*)TRN->pBuffer + TRN->nHeaderSize + TRN->nDataSize;
		if (UMemory::Compare(pFieldBegin, "\r\n", 2))
			return false;
	
		pFieldBegin += 2;
		Uint32 nRemainDataSize = (Uint8 *)TRN->pBuffer + TRN->nBufferSize - pFieldBegin;
		if (nRemainDataSize >= 5 && !UMemory::Compare(pFieldBegin, "0\r\n\r\n", 5))
		{
			TRN->nBufferSize = TRN->nHeaderSize + TRN->nDataSize;
			return true;
		}
	
		Uint8 *pFieldEnd = UMemory::Search("\r\n", 2, pFieldBegin, (nRemainDataSize > 30 ? 30 : nRemainDataSize));		
		if (!pFieldEnd)
			return false;
		
		Uint16 nFieldLength = pFieldEnd - pFieldBegin;

		Uint16 nLength = nFieldLength;
		while (nLength > 1 && pFieldBegin[nLength-1] == ' ')
			nLength--;

		if (nLength > 4)
			nLength = 4;
			
		Uint8 hex[4] = {'0', '0', '0', '0'};
		UMemory::Copy(hex + 4 - nLength, pFieldBegin, nLength);
		UText::MakeUppercase(hex + 4 - nLength, nLength);
		
		Uint16 nNewDataSize = 0;
		UMemory::HexToData(hex, 4, &nNewDataSize, 2);

		#if CONVERT_INTS
		nNewDataSize = swap_int(nNewDataSize); 
		#endif

		Uint32 nOldDataSize = TRN->nDataSize;
		TRN->nDataSize += nNewDataSize;
		
		Uint8 *pReplaceData = (Uint8 *)TRN->pBuffer + TRN->nHeaderSize + nOldDataSize;
		Uint8 *pMoveData = pReplaceData + nFieldLength + 4;
		UMemory::Copy(pReplaceData, pMoveData, (Uint8 *)TRN->pBuffer + TRN->nBufferSize - pMoveData);
		TRN->nBufferSize -= nFieldLength + 4;
	}
	 
	return false;
}


/* -------------------------------------------------------------------------- */
#pragma mark -

static bool _AddResponseField_Http(THttpTransact inTrn)
{
	Uint8 *pProtocol;
	if (TRN->nProtocol == protocol_HTTP_1_1)
		pProtocol = "\pHTTP/1.1 ";
	else
		pProtocol = "\pHTTP/1.0 ";
	
	return _AddField_General(inTrn, pProtocol, "\p200 OK");
}

static bool _AddResponseField_Server(THttpTransact inTrn, const Uint8 *inServer)
{
	if (!inServer && !inServer[0])
		return true;

	return _AddField_General(inTrn, "\pServer: ", inServer);
}

static bool _AddResponseField_ContentLength(THttpTransact inTrn, Uint32 inContentLength)
{	
	Uint8 psContentLength[32];
	psContentLength[0] = UText::IntegerToText(psContentLength + 1, sizeof(psContentLength) - 1, inContentLength);
	
	return _AddField_General(inTrn, "\pContent-Length: ", psContentLength);
}

static bool _AddResponseField_Connection(THttpTransact inTrn)
{
	return _AddField_General(inTrn, "\pConnection: ", "\pKeep-Alive");
}

static bool _AddResponseField_TransferEncoding(THttpTransact inTrn)
{
	return _AddField_General(inTrn, "\pTransfer-Encoding: ", "\pchunked");
}

static bool _AddResponseField_ContentType(THttpTransact inTrn)
{
	Uint8 psType[32];
	psType[0] = UMemory::Copy(psType + 1, TRN->csType, strlen(TRN->csType));
		
	return _AddField_General(inTrn, "\pContent-Type: ", psType);
}


/* -------------------------------------------------------------------------- */
#pragma mark -

static bool _AddRequestField_Get(THttpTransact inTrn, const Uint8 *inDomain, bool inGetHeader)
{
	if (!inDomain)
		return false;
		
	const Uint8 *psProtocol;
	if (TRN->nProtocol == protocol_HTTP_1_1)
		psProtocol = "\p HTTP/1.1";
	else
		psProtocol = "\p HTTP/1.0";
	
	const Uint8 *psCommand;
	if (inGetHeader)
		psCommand = "\pHEAD ";
	else
		psCommand = "\pGET ";

	const Uint8 *psDomain;
	if (!inDomain[0])
		psDomain = "\p/";
	else
		psDomain = inDomain;
	
	return _AddField_General(inTrn, psCommand, psDomain, psProtocol);
}

static bool _AddRequestField_IfModifiedSince(THttpTransact inTrn, const Uint8 *inLastModified, Uint32 inLength)
{	
	if (!inLastModified || !inLastModified[0] || !inLength)
		return true;
		
	Uint8 psLength[32];
	psLength[0] = UText::Format(psLength + 1, sizeof(psLength) - 1, "; length=%hu", inLength);
	
	return _AddField_General(inTrn, "\pIf-Modified-Since: ", inLastModified, psLength);
}

static bool _AddRequestField_Referer(THttpTransact inTrn, const Uint8 *inReferer)
{
	if (!inReferer || !inReferer[0])
		return true;
		
	return _AddField_General(inTrn, "\pReferer: ", inReferer);
}

static bool _AddRequestField_Connection(THttpTransact inTrn, bool inProxyConnection)
{
	if (inProxyConnection)
		return _AddField_General(inTrn, "\pProxy-Connection: ", "\pKeep-Alive");

	return _AddField_General(inTrn, "\pConnection: ", "\pKeep-Alive");
}

static bool _AddRequestField_UserAgent(THttpTransact inTrn, const Uint8 *inUserAgent)
{
	if (!inUserAgent && !inUserAgent[0])
		return true;
		
	return _AddField_General(inTrn, "\pUser-Agent: ", inUserAgent);
}

static bool _AddRequestField_Pragma(THttpTransact inTrn, const Uint8 *inPragma)
{
	if (inPragma)
		return _AddField_General(inTrn, "\pPragma: ", inPragma);
	else
		return _AddField_General(inTrn, "\pPragma: ", "\pno-cache");
}

static bool _AddRequestField_Host(THttpTransact inTrn, const Uint8 *inHost)
{
	if (!inHost)
		return false;
		
	return _AddField_General(inTrn, "\pHost: ", inHost);
}

static bool _AddRequestField_Accept(THttpTransact inTrn, const Uint8 *inAccept)
{
	if (inAccept)
		return _AddField_General(inTrn, "\pAccept: ", inAccept);
	else
		return _AddField_General(inTrn, "\pAccept: ", "\ptext/*, image/*, */*");
}

static bool _AddRequestField_AcceptLanguage(THttpTransact inTrn, const Uint8 *inAcceptLanguage)
{
	if (inAcceptLanguage)
		return _AddField_General(inTrn, "\pAccept-Language: ", inAcceptLanguage);
	else
		return _AddField_General(inTrn, "\pAccept-Language: ", "\pen");
}

static bool _AddRequestField_AcceptCharset(THttpTransact inTrn, const Uint8 *inAcceptCharset)
{
	if (inAcceptCharset)
		return _AddField_General(inTrn, "\pAccept-Charset: ", inAcceptCharset);
	else
		return _AddField_General(inTrn, "\pAccept-Charset: ", "\piso-8859-1,*,utf-8");
}

static bool _AddRequestField_HttpCookie(THttpTransact inTrn)
{
	Uint32 i = 0;	
	SHttpCookie *pHttpCookie = nil;
		
	CPtrList<SHttpCookie> sendCookieList;
	
	while (_gHttpCookieList.GetNextCookie(pHttpCookie, i))
	{
		if (TRN->psHost[0] >= pHttpCookie->psHost[0] && !UText::CompareInsensitive(TRN->psHost + 1 + TRN->psHost[0] - pHttpCookie->psHost[0], pHttpCookie->psHost + 1, pHttpCookie->psHost[0]) && 	// tail matching
		    TRN->psDomain[0] >= pHttpCookie->psDomain[0] && !UText::CompareInsensitive(TRN->psDomain + 1, pHttpCookie->psDomain + 1, pHttpCookie->psDomain[0]))										// head matching
		    sendCookieList.AddItem(pHttpCookie);
	}
		
	if (sendCookieList.GetItemCount())
	{
		Uint32 nOffset = TRN->nBufferSize;
		TRN->nBufferSize += 8;

		i = 0;
		while (sendCookieList.GetNext(pHttpCookie, i))
			TRN->nBufferSize += pHttpCookie->nDataSize + 2;

		try
		{
			if (TRN->pBuffer == nil)
				TRN->pBuffer = UMemory::NewClear(TRN->nBufferSize);
			else
				TRN->pBuffer = UMemory::Reallocate((TPtr)TRN->pBuffer, TRN->nBufferSize);
		}
		catch(...)
		{
			TRN->nBufferSize = 0;
			return false;
		}

		nOffset += UMemory::Copy((Uint8 *)TRN->pBuffer + nOffset, "Cookie: ", 8);

		while (sendCookieList.GetItemCount())
		{
			Uint8 nDomainSize = 0;
			SHttpCookie *pDomainCookie = nil;

			i = 0;	
			while (sendCookieList.GetNext(pHttpCookie, i))
			{
				// cookies with more specific domain (path) must be send first
				if (pHttpCookie->psDomain[0] >= nDomainSize)
				{
					nDomainSize = pHttpCookie->psDomain[0];
					pDomainCookie = pHttpCookie;
				}
			}
		
			if (!pDomainCookie)
				break;
			
			// add cookie and separator
			nOffset += UMemory::Copy((Uint8 *)TRN->pBuffer + nOffset, pDomainCookie->pData, pDomainCookie->nDataSize);
			nOffset += UMemory::Copy((Uint8 *)TRN->pBuffer + nOffset, "; ", 2);
						
			sendCookieList.RemoveItem(pDomainCookie);
		}

		// add end of field
		nOffset -= 2;
		*((Uint8 *)TRN->pBuffer + nOffset) = '\r';
		*((Uint8 *)TRN->pBuffer + nOffset + 1) = '\n';		
	}
	
	return true;
}

static bool _AddRequestField_HttpID(THttpTransact inTrn)
{
	Uint32 nHttpIDSize = 0;
	
	Uint32 i = 0;
	Uint8 *psHttpID;
	
	while (TRN->lHttpIDList.GetNext(psHttpID, i))
	{
		if (psHttpID[0])
			nHttpIDSize += psHttpID[0] + 2;
	}
		
	if (nHttpIDSize)
	{
		Uint32 nOffset = TRN->nBufferSize;
		TRN->nBufferSize += nHttpIDSize;

		try
		{
			if (TRN->pBuffer == nil)
				TRN->pBuffer = UMemory::NewClear(TRN->nBufferSize);
			else
				TRN->pBuffer = UMemory::Reallocate((TPtr)TRN->pBuffer, TRN->nBufferSize);
		}
		catch(...)
		{
			TRN->nBufferSize = 0;
			return false;
		}
				
		// add IDs
		i = 0;
		while (TRN->lHttpIDList.GetNext(psHttpID, i))
		{
			if (psHttpID[0])
			{
				// set data
				nOffset += UMemory::Copy((Uint8 *)TRN->pBuffer + nOffset, psHttpID + 1, psHttpID[0]);

				// set end of field
				*((Uint8 *)TRN->pBuffer + nOffset) = '\r';
				*((Uint8 *)TRN->pBuffer + nOffset + 1) = '\n';
				nOffset += 2;
			}
		}
	}

	return true;
}

static bool _AddRequestField_CustomField(THttpTransact inTrn, const Uint8 *inCustomField)
{
	if (!inCustomField && !inCustomField[0])
		return true;
		
	return _AddField_General(inTrn, "\p", inCustomField);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

static bool _AddField_General(THttpTransact inTrn, const Uint8 *inField, const Uint8 *inData, const Uint8 *inExtraData)
{
	Uint32 nOffset = TRN->nBufferSize;
	TRN->nBufferSize += inField[0] + inData[0] + 2;

	if (inExtraData)
		TRN->nBufferSize += inExtraData[0];
	
	try
	{
		if (TRN->pBuffer == nil)
			TRN->pBuffer = UMemory::NewClear(TRN->nBufferSize);
		else
			TRN->pBuffer = UMemory::Reallocate((TPtr)TRN->pBuffer, TRN->nBufferSize);
	}
	catch(...)
	{	
		TRN->nBufferSize = 0;
		return false;
	}
		
	nOffset += UMemory::Copy((Uint8 *)TRN->pBuffer + nOffset, inField + 1, inField[0]);
	nOffset += UMemory::Copy((Uint8 *)TRN->pBuffer + nOffset, inData + 1, inData[0]);
	
	if (inExtraData)
		nOffset += UMemory::Copy((Uint8 *)TRN->pBuffer + nOffset, inExtraData + 1, inExtraData[0]);
	
	*((Uint8 *)TRN->pBuffer + nOffset) = '\r';
	*((Uint8 *)TRN->pBuffer + nOffset + 1) = '\n';

	return true;
}

static bool _AddField_End(THttpTransact inTrn)
{
	if (!TRN->pBuffer)
		return false;
		
	TRN->nBufferSize += 2;
	try
	{
		TRN->pBuffer = UMemory::Reallocate((TPtr)TRN->pBuffer, TRN->nBufferSize);
	}
	catch(...)
	{
		TRN->nBufferSize = 0;
		return false;
	}
	
	*((Uint8 *)TRN->pBuffer + TRN->nBufferSize -2) = '\r';
	*((Uint8 *)TRN->pBuffer + TRN->nBufferSize -1) = '\n';
	
	return true;
}


/* -------------------------------------------------------------------------- */
#pragma mark -

static void _CleanDomain(Uint8 *inDomain)
{
	Uint8 *pReplace;
	
	do
	{
		pReplace = UMemory::Search("%20", 3, inDomain + 1, inDomain[0]);
		
		if (pReplace)
		{
			*pReplace = ' '; pReplace++;
			UMemory::Copy(pReplace, pReplace + 2, inDomain[0] - (pReplace - inDomain + 1));
			inDomain[0] -= 2;
		}
	
	} while (pReplace);
}

static void _ClearHttpTransport(THttpTransact inTrn)
{
	if (TRN->tpt)
	{		
		UTransport::Dispose(TRN->tpt);
		TRN->tpt = nil;
	}
}

static void _ClearHttpBuffer(THttpTransact inTrn)
{
	if (TRN->pBuffer)
	{
		UMemory::Dispose((TPtr)TRN->pBuffer);
		TRN->pBuffer = nil;
	}	
	
	TRN->nBufferSize = 0;
	
	TRN->nHeaderSize = 0;
	TRN->isHeaderEnd = false;
	
	TRN->nDataSize = 0;
	TRN->csType[0] = 0;
}

static void _ClearHttpMisc(THttpTransact inTrn)
{
	TRN->psLocation[0] = 0;
	TRN->psLicense[0] = 0;
	
	TRN->psLastModified[0] = 0;
	TRN->nLength = 0;
	
	TRN->psCustomField[0] = 0;
	_ClearHttpIDList(inTrn);
}

void _ClearHttpIDList(THttpTransact inTrn)
{
	Uint32 i = 0;
	Uint8 *psHttpID;
	
	while (TRN->lHttpIDList.GetNext(psHttpID, i))
		UMemory::Dispose((TPtr)psHttpID);
		
	TRN->lHttpIDList.Clear();
}


/* -------------------------------------------------------------------------- */
#pragma mark -

CCookieList::CCookieList()
{
}

CCookieList::~CCookieList()
{
	Uint32 i = 0;
	SHttpCookie *pHttpCookie;
		
	while (mCookieList.GetNext(pHttpCookie, i))
		DisposeCookie(pHttpCookie);

	mCookieList.Clear();	
}

bool CCookieList::AddCookie(SHttpCookie *inHttpCookie)
{
	if (!inHttpCookie)
		return false;
		
	const Uint8 *inNameEnd = UMemory::SearchByte('=', inHttpCookie->pData, inHttpCookie->nDataSize);
	if (!inNameEnd)
	{
		DisposeCookie(inHttpCookie);
		return false;
	}

	Uint32 i = 0;	
	SHttpCookie *pHttpCookie;
		
	while (mCookieList.GetNext(pHttpCookie, i))
	{
		if (!UText::CompareInsensitive(inHttpCookie->psHost + 1, inHttpCookie->psHost[0], pHttpCookie->psHost + 1, pHttpCookie->psHost[0]) &&
			!UText::CompareInsensitive(inHttpCookie->psDomain + 1, inHttpCookie->psDomain[0], pHttpCookie->psDomain + 1, pHttpCookie->psDomain[0]))
		{
			const Uint8 *pNameEnd = UMemory::SearchByte('=', pHttpCookie->pData, pHttpCookie->nDataSize);
			if (!pNameEnd)
				continue;
			
			if (!UText::CompareInsensitive(inHttpCookie->pData, inNameEnd - inHttpCookie->pData, pHttpCookie->pData, pNameEnd - pHttpCookie->pData))
			{
				// invert data size
				Uint32 nDataSize = pHttpCookie->nDataSize;
				pHttpCookie->nDataSize = inHttpCookie->nDataSize;
				inHttpCookie->nDataSize = nDataSize;
				
				// invert data
				void *pData = pHttpCookie->pData;
				pHttpCookie->pData = inHttpCookie->pData;
				inHttpCookie->pData = pData;
				
				// set expired date
				pHttpCookie->stExpiredDate = inHttpCookie->stExpiredDate;
			
				DisposeCookie(inHttpCookie);
				return true;
			}
		}
	}
	
	// check max count
	if (mCookieList.GetItemCount() >= kHttp_MaxCookies)
	{
		pHttpCookie = mCookieList.RemoveItem(1);	// remove old cookie
		DisposeCookie(pHttpCookie);
	}
	
	// add new cookie
	mCookieList.AddItem(inHttpCookie);
	
	return true;
}

// inExpiredDate must be in local time
bool CCookieList::AddCookie(const Uint8 *inHost, const Uint8 *inDomain, const void *inData, Uint32 inDataSize, const SCalendarDate& inExpiredDate)
{
	if (!inHost || !inDomain || !inData || !inDataSize)
		return false;

	SHttpCookie *pHttpCookie = (SHttpCookie *)UMemory::NewClear(sizeof(SHttpCookie));
		
	pHttpCookie->pData = UMemory::New(inData, inDataSize);
	pHttpCookie->nDataSize = inDataSize;

	pHttpCookie->psHost = (Uint8 *)UMemory::New(inHost, inHost[0] + 1);
	pHttpCookie->psDomain = (Uint8 *)UMemory::New(inDomain, inDomain[0] + 1);
	pHttpCookie->stExpiredDate = inExpiredDate;
	
	return AddCookie(pHttpCookie);
}

void *CCookieList::GetCookie(const Uint8 *inHost, const Uint8 *inDomain, Uint32& outDataSize)
{
	outDataSize = 0;
	if (!inHost || !inDomain)
		return nil;

	CPtrList<SHttpCookie> lCookieList;

	Uint32 i = 0;	
	SHttpCookie *pHttpCookie;

	while (GetNextCookie(pHttpCookie, i))
	{
		if (inHost[0] >= pHttpCookie->psHost[0] && !UText::CompareInsensitive(inHost + 1 + inHost[0] - pHttpCookie->psHost[0], pHttpCookie->psHost + 1, pHttpCookie->psHost[0]) && 		// tail matching
		    inDomain[0] >= pHttpCookie->psDomain[0] && !UText::CompareInsensitive(inDomain + 1, pHttpCookie->psDomain + 1, pHttpCookie->psDomain[0]))									// head matching
		    lCookieList.AddItem(pHttpCookie);
	}
		
	void *pData = nil;
	Uint32 nDataSize = 0;

	if (lCookieList.GetItemCount())
	{
		i = 0;
		while (lCookieList.GetNext(pHttpCookie, i))
			nDataSize += pHttpCookie->nDataSize + 2;

		nDataSize -= 2;
		try
		{
			pData = UMemory::NewClear(nDataSize);
		}
		catch(...)
		{
			// don't throw
			return nil;
		}

		Uint32 nOffset = 0;
		while (lCookieList.GetItemCount())
		{
			Uint8 nDomainSize = 0;
			SHttpCookie *pDomainCookie;

			i = 0;	
			while (lCookieList.GetNext(pHttpCookie, i))
			{
				if (pHttpCookie->psDomain[0] > nDomainSize)	// cookie with more specific domain (path) must be send first
				{
					nDomainSize = pHttpCookie->psDomain[0];
					pDomainCookie = pHttpCookie;
				}
			}
		
			nOffset += UMemory::Copy((Uint8 *)pData + nOffset, pDomainCookie->pData, pDomainCookie->nDataSize);
			if (nOffset <= nDataSize)
				nOffset += UMemory::Copy((Uint8 *)pData + nOffset, "; ", 2);
			
			lCookieList.RemoveItem(pDomainCookie);
		}
	}
		
	outDataSize = nDataSize;
	return pData;
}

bool CCookieList::GetNextCookie(SHttpCookie*& outHttpCookie, Uint32& ioIndex)
{
	while (true)
	{
		if (!mCookieList.GetNext(outHttpCookie, ioIndex))
			return false;

		if (IsExpired(outHttpCookie->stExpiredDate))
		{
			mCookieList.RemoveItem(ioIndex);
			ioIndex--;
						
			DisposeCookie(outHttpCookie);
			continue;
		}
		
		return true;
	}
}

void CCookieList::DisposeCookie(SHttpCookie *inHttpCookie)
{
	if (!inHttpCookie)
		return;
	
	UMemory::Dispose((TPtr)inHttpCookie->pData);
	UMemory::Dispose((TPtr)inHttpCookie->psHost);
	UMemory::Dispose((TPtr)inHttpCookie->psDomain);
	
	UMemory::Dispose((TPtr)inHttpCookie);
}

bool CCookieList::IsExpired(SCalendarDate& inExpiredDate)
{
	// if expired date is not specified, the cookie will expires when the user's session ends
	if (!inExpiredDate.year)
		return false;
	
	SCalendarDate stCurrentDate;
	UDateTime::GetCalendarDate(calendar_Gregorian, stCurrentDate);
			
	return stCurrentDate >= inExpiredDate;
}

// Wdy, DD-Mon-YYYY HH:MM:SS GMT (sometimes Wdy, DD Mon YYYY HH:MM:SS GMT)
bool CCookieList::ConvertExpiredDate(const Uint8 *inDate, SCalendarDate& outDate)
{
	ClearStruct(outDate);

	if (!inDate || !inDate[0])
		return false;
	
	const Uint8 *pDateEnd = inDate + inDate[0] + 1;

	const Uint8 *pWeekDayBegin = inDate + 1;
	const Uint8 *pWeekDayEnd = UMemory::Search(", ", 2, pWeekDayBegin, pDateEnd - pWeekDayBegin);
	if (!pWeekDayEnd)
		return false;

	const Uint8 *pWeekDayList1[7] = {"\pMonday", "\pTuesday", "\pWednesday", "\pThursday", "\pFriday", "\pSaturday", "\pSunday"};
	const Int8 *pWeekDayList2[7] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

	Uint8 nWeekDay = 0;
	while (nWeekDay < 7)
	{
		if (!UText::CompareInsensitive(pWeekDayBegin, pWeekDayEnd - pWeekDayBegin, pWeekDayList1[nWeekDay] + 1, pWeekDayList1[nWeekDay][0]) ||
			!UText::CompareInsensitive(pWeekDayBegin, pWeekDayEnd - pWeekDayBegin, pWeekDayList2[nWeekDay], 3))
			break;
			
		nWeekDay++;
	}
	
	if (nWeekDay >= 7)
		return false;
	
	nWeekDay++;

	const Uint8 *pDayBegin = pWeekDayEnd + 2;
	const Uint8 *pDayEnd = UMemory::SearchByte('-', pDayBegin, pDateEnd - pDayBegin);
	if (!pDayEnd)
	{
		pDayEnd = UMemory::SearchByte(' ', pDayBegin, pDateEnd - pDayBegin);
		if (!pDayEnd)
			return false;
	}
	
	Int8 nDay = UText::TextToInteger(pDayBegin, pDayEnd - pDayBegin, 0, 10);
	if (nDay < 1 || nDay > 31)
		return false;

	const Uint8 *pMonthBegin = pDayEnd + 1;
	const Uint8 *pMonthEnd = UMemory::SearchByte('-', pMonthBegin, pDateEnd - pMonthBegin);
	if (!pMonthEnd)
	{
		pMonthEnd = UMemory::SearchByte(' ', pMonthBegin, pDateEnd - pMonthBegin);
		if (!pMonthEnd)
			return false;
	}
	
	const Int8 *pMonthList[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

	Uint8 nMonth = 0;
	while (nMonth < 12)
	{
		if (!UText::CompareInsensitive(pMonthBegin, pMonthList[nMonth], 3))
			break;
			
		nMonth++;
	}
	
	if (nMonth >= 12)
		return false;
	
	nMonth++;
		
	const Uint8 *pYearBegin = pMonthEnd + 1;
	const Uint8 *pYearEnd = UMemory::SearchByte(' ', pYearBegin, pDateEnd - pYearBegin);
	if (!pYearEnd)
		return false;

	Int16 nYear = UText::TextToInteger(pYearBegin, pYearEnd - pYearBegin);
	if (nYear <= 0)
		return false;

	const Uint8 *pHourBegin = pYearEnd + 1;
	const Uint8 *pHourEnd = UMemory::SearchByte(':', pHourBegin, pDateEnd - pHourBegin);
	if (!pHourEnd)
		return false;

	Int16 nHour = UText::TextToInteger(pHourBegin, pHourEnd - pHourBegin, 0, 10);
	if (nHour < 0 || nHour > 23)
		return false;

	const Uint8 *pMinuteBegin = pHourEnd + 1;
	const Uint8 *pMinuteEnd = UMemory::SearchByte(':', pMinuteBegin, pDateEnd - pMinuteBegin);
	if (!pMinuteEnd)
		return false;

	Int16 nMinute = UText::TextToInteger(pMinuteBegin, pMinuteEnd - pMinuteBegin, 0, 10);
	if (nMinute < 0 || nMinute > 59)
		return false;

	const Uint8 *pSecondBegin = pMinuteEnd + 1;
	const Uint8 *pSecondEnd = UText::SearchInsensitive(" GMT", 4, pSecondBegin, pDateEnd - pSecondBegin);
	if (!pSecondEnd)
		return false;

	Int16 nSecond = UText::TextToInteger(pSecondBegin, pSecondEnd - pSecondBegin, 0, 10);
	if (nSecond < 0 || nSecond > 59)
		return false;

	outDate.year = nYear;
	outDate.month = nMonth;
	outDate.day = nDay;
	outDate.hour = nHour;
	outDate.minute = nMinute;
	outDate.second = nSecond;
	outDate.weekDay = nWeekDay;

	// convert from GMT to local time
	outDate += UDateTime::GetGMTDelta();

	return true;
}

// does NOT take ownership of inFileRef
bool CCookieList::WriteData(TFSRefObj* inFileRef)
{
	if (!inFileRef)
		return false;

	bool bIsFolder = false;
	if (inFileRef->Exists(&bIsFolder))
	{
		if (bIsFolder)
			inFileRef->DeleteFolder();
		else
			inFileRef->DeleteFile();
	}

	Uint32 nCookieCount = mCookieList.GetItemCount();
	if (!nCookieCount)
		return false;

	inFileRef->CreateFileAndOpen('ckie', 'HTLC', kAlwaysOpenFile, perm_ReadWrite);

	try
	{
		// write version number
		Uint16 nCookieVersion = TB((Uint16)kHttp_CookieVersion);
		Uint32 nOffset = inFileRef->Write(0, &nCookieVersion, sizeof(Uint16));

		// write count
		nCookieCount = 0;
		nOffset += inFileRef->Write(nOffset, &nCookieCount, sizeof(Uint32));
		
		Uint32 i = 0;	
		SHttpCookie *pHttpCookie;
		
		while (mCookieList.GetNext(pHttpCookie, i))
		{
			// if expired date is not specified, the cookie expires when the user's session ends
			if (!pHttpCookie->stExpiredDate.IsValid())
				continue;
			
			// write data
			Uint32 nDataSize = TB(pHttpCookie->nDataSize);
			nOffset += inFileRef->Write(nOffset, &nDataSize, sizeof(Uint32));
			nOffset += inFileRef->Write(nOffset, pHttpCookie->pData, pHttpCookie->nDataSize);
			
			// write host/domain
			nOffset += inFileRef->Write(nOffset, pHttpCookie->psHost, pHttpCookie->psHost[0] + 1);
			nOffset += inFileRef->Write(nOffset, pHttpCookie->psDomain, pHttpCookie->psDomain[0] + 1);

			// write expired date
			SCalendarDate stExpiredDate = {TB(pHttpCookie->stExpiredDate.year), TB(pHttpCookie->stExpiredDate.month), TB(pHttpCookie->stExpiredDate.day), TB(pHttpCookie->stExpiredDate.hour), TB(pHttpCookie->stExpiredDate.minute), TB(pHttpCookie->stExpiredDate.second), TB(pHttpCookie->stExpiredDate.weekDay), TB(pHttpCookie->stExpiredDate.val)};
			nOffset += inFileRef->Write(nOffset, &stExpiredDate, sizeof(SCalendarDate));
			
			nCookieCount++;
		}
		
		if (!nCookieCount)		
		{
			inFileRef->Close();
			inFileRef->DeleteFile();
			return false;
		}
		
		// write count
		nCookieCount = TB(nCookieCount);
		inFileRef->Write(2, &nCookieCount, sizeof(Uint32));
	}
	catch(...)
	{
		inFileRef->Close();
		inFileRef->DeleteFile();
		throw;
	}
	
	inFileRef->Close();
	return true;
}

// does NOT take ownership of inFileRef
bool CCookieList::ReadData(TFSRefObj* inFileRef)
{
	if (!inFileRef)
		return false;
	
	bool bIsFolder = false;
	if (!inFileRef->Exists(&bIsFolder) || bIsFolder)
		return false;
	
	inFileRef->Open(perm_Read);
	
	void *pData = nil;
	SHttpCookie *pHttpCookie = nil;
	Uint32 nOffset = 0;

	try
	{
		// read version number
		Uint16 nVersion;
		if (inFileRef->Read(nOffset, &nVersion, sizeof(Uint16)) != sizeof(Uint16))
		{
			inFileRef->Close();
			inFileRef->DeleteFile();
			return false;
		}
		
		nOffset += sizeof(Uint16);
		nVersion = TB(nVersion);
		if (nVersion != kHttp_CookieVersion)
		{
			inFileRef->Close();
			return false;
		}
				
		// read count
		Uint32 nCookieCount;
		if (inFileRef->Read(nOffset, &nCookieCount, sizeof(Uint32)) != sizeof(Uint32))
		{
			inFileRef->Close();
			inFileRef->DeleteFile();
			return false;
		}
		
		nOffset += sizeof(Uint32);
		nCookieCount = TB(nCookieCount);	
		if (!nCookieCount)
		{
			inFileRef->Close();
			inFileRef->DeleteFile();
			return false;
		}
		
		Uint8 nPsSize;
		Uint8 psHost[256];
		Uint8 psDomain[256];
		
		while (nCookieCount--)
		{
			// read data
			Uint32 nDataSize;
			if (inFileRef->Read(nOffset, &nDataSize, sizeof(Uint32)) != sizeof(Uint32))
			{
				inFileRef->Close();
				return false;
			}
			
			nOffset += sizeof(Uint32);
			nDataSize = TB(nDataSize);

			pData = UMemory::New(nDataSize);
			if (inFileRef->Read(nOffset, pData, nDataSize) != nDataSize)
			{
				UMemory::Dispose((TPtr)pData);
				inFileRef->Close();			
				return false;
			}
			
			nOffset += nDataSize;
		
			// read host
			if (inFileRef->Read(nOffset, &nPsSize, 1) != 1)
			{
				UMemory::Dispose((TPtr)pData);
				inFileRef->Close();			
				return false;
			}
			
			nPsSize += 1;
			if (inFileRef->Read(nOffset, psHost, nPsSize) != nPsSize)
			{
				UMemory::Dispose((TPtr)pData);
				inFileRef->Close();			
				return false;
			}
			
			nOffset += nPsSize;

			// read domain
			if (inFileRef->Read(nOffset, &nPsSize, 1) != 1)
			{
				UMemory::Dispose((TPtr)pData);
				inFileRef->Close();			
				return false;
			}
			
			nPsSize += 1;
			if (inFileRef->Read(nOffset, psDomain, nPsSize) != nPsSize)
			{
				UMemory::Dispose((TPtr)pData);
				inFileRef->Close();			
				return false;
			}
			
			nOffset += nPsSize;

			// read expired date
			SCalendarDate stExpiredDate;
			if (inFileRef->Read(nOffset, &stExpiredDate, sizeof(SCalendarDate)) != sizeof(SCalendarDate))
			{
				UMemory::Dispose((TPtr)pData);
				inFileRef->Close();
				return false;
			}
			
			nOffset += sizeof(SCalendarDate);

			// set cookie
			pHttpCookie = (SHttpCookie *)UMemory::NewClear(sizeof(SHttpCookie));
				
			// set data
			pHttpCookie->pData = pData;
			pHttpCookie->nDataSize = nDataSize;
			pData = nil;

			// set host/domain
			pHttpCookie->psHost = (Uint8 *)UMemory::New(psHost, psHost[0] + 1);
			pHttpCookie->psDomain = (Uint8 *)UMemory::New(psDomain, psDomain[0] + 1);

			// set expired date
			pHttpCookie->stExpiredDate.year = TB(stExpiredDate.year);
			pHttpCookie->stExpiredDate.month = TB(stExpiredDate.month);
			pHttpCookie->stExpiredDate.day = TB(stExpiredDate.day);
			pHttpCookie->stExpiredDate.hour = TB(stExpiredDate.hour);
			pHttpCookie->stExpiredDate.minute = TB(stExpiredDate.minute);
			pHttpCookie->stExpiredDate.second = TB(stExpiredDate.second);
			pHttpCookie->stExpiredDate.weekDay = TB(stExpiredDate.weekDay);
			pHttpCookie->stExpiredDate.val = TB(stExpiredDate.val);

			mCookieList.AddItem(pHttpCookie);
			pHttpCookie = nil;
		}
	}
	catch(...)
	{
		inFileRef->Close();
		inFileRef->DeleteFile();
		
		UMemory::Dispose((TPtr)pData);
		DisposeCookie(pHttpCookie);
		throw;
	}	
	
	inFileRef->Close();
	return true;
}

