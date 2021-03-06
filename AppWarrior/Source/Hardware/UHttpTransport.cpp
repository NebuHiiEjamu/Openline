/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
 
#include "UHttpTransport.h"


/*
 * Structures and Types
 */

#define LOOP_DATA_ARRIVED	0


enum
{
	http_Data			= 1,
	http_Padding		= 2,
	http_DisconnectData = 3
};

enum
{
	http_ReceivePadding	= 1,
	http_ReceiveHeader	= 2
};


#pragma options align=packed
struct SData
{
	Uint32 nDataCode;
	Uint32 nDataSize;
};
#pragma options align=reset

struct SSendData
{
	void *pSendData;
	Uint32 nDataSize;
	bool bDataBuffer;
};

struct SHttpTransport {
	TRegularTransport tptSend;
	TRegularTransport tptReceive;
	bool bSendPostHeader;
	bool bSendGetHeader;
	Uint8 nReceiveHeader;
	bool bIsConnectingSend;
	bool bIsConnectingReceive;
	bool bIsEstablished;
	bool bIsDisconnecting;
	TMessageProc msgProc;
	void *msgProcContext;
	void *msgObject;
	TTimer msgTimer;
	void *pAddress;
	Uint32 nAddressSize;
	Uint32 nProtocol;
	Uint32 nOptions;
	SGUID stConnectionGuid;
	SData stReceiveData;
	Uint32 nOffsetData;
	void *pReceivedData;
	Uint32 nReceivedOffset; 
	Uint32 nNotifyReceivedSize;
	CPtrList<SSendData> stSendDataList;
};

struct SConnectionInfo {
	TRegularTransport tptReg;
	SHttpTransport *pHttpTransport;
	SInternetAddress stAddress;
	SGUID stConnectionGuid;
	Uint32 nConnectionTime;
	Uint8 nReceiveHeader;
	bool bSendReceive;
	SData stReceiveData;
	Uint32 nOffsetData;
};

#define TPT					((SHttpTransport *)inTpt)

/*
 * Function Prototypes
 */

extern void _TRGetUnsentData(TRegularTransport inTpt, CPtrList<SSendData> *inSendDataList);

static void _HttpConnectionOpenMessageHandler(void *inContext, void *inObject, Uint32 inMsg, const void *inData, Uint32 inDataSize);
static void _HttpConnectionMessageHandler(void *inContext, void *inObject, Uint32 inMsg, const void *inData, Uint32 inDataSize);
static void _HttpConnectionTimerHandler(void *inContext, void *inObject, Uint32 inMsg, const void *inData, Uint32 inDataSize);
static void _HttpConnectionOpenCloseTimerHandler(void *inContext, void *inObject, Uint32 inMsg, const void *inData, Uint32 inDataSize);

static bool _CopyAddress(THttpTransport inTpt, const void *inAddress, Uint32 inAddressSize);
static bool _CheckAddress(THttpTransport inTpt, bool inSendReceive);
static bool _GetAddress(THttpTransport inTpt, Uint8 *outHost, Uint32 inHostSize, Uint8 *outAddress, Uint32 inAddressSize);
static bool _GetProxyAddresss(SInternetNameAddress& outAddress);
static bool _AcceptConnection(THttpTransport inTpt);
static bool _CheckConnectionEstabilished(THttpTransport inTpt);
static bool _SearchConnectionOpenPair(THttpTransport inTpt, TRegularTransport& outTptSend, TRegularTransport& outTptReceive, SGUID& outConnectionGuid, void *outAddr, Uint32 *ioAddrSize);
static bool _TryReconnect(THttpTransport inTpt);
static bool _WaitReconnect(THttpTransport inTpt);
static bool _SearchReconnect(SConnectionInfo *inConnectionInfo);
static bool _ReceiveDisconnectData(THttpTransport inTpt);
static void _Disconnect(THttpTransport inTpt);

#if LOOP_DATA_ARRIVED
static void _ProcessConnections();
static void _ProcessHttpData();
#endif

static bool _SendHttpPostHeader(THttpTransport inTpt);
static bool _SendHttpResponsePostHeader(TRegularTransport inTpt);
static bool _SendHttpGetHeader(THttpTransport inTpt);
static bool _SendHttpResponseGetHeader(TRegularTransport inTpt);
static bool _SendHttpPadding(TRegularTransport inTpt, Uint32 inDataSize = 0);
static void _SendDisconnectPost(TRegularTransport inTpt);
static void _SendDisconnectGet(TRegularTransport inTpt);

static bool _ReceiveHttpHeader(SConnectionInfo *inConnectionInfo);
static bool _ReceiveHttpHeader(THttpTransport inTpt);
static bool _ReceiveHttpData(THttpTransport inTpt);
static bool _ReceiveHttpPadding(SConnectionInfo *inConnectionInfo, Uint32 inReceiveSize);
static bool _ReceiveHttpPadding(THttpTransport inTpt, Uint32 inReceiveSize);

static bool _AddField_General(void **ioBuffer, Uint32& ioBufferSize, const Uint8 *inField, const Uint8 *inData, const Uint8 *inExtraData = nil);
static bool _AddField_End(void **ioBuffer, Uint32& ioBufferSize);
static bool _GetHeaderSize(void *inBuffer, Uint32 inBufferSize, Uint32& outHeaderSize);

static void _AddSendDataList(THttpTransport inTpt, Uint32 inDataSize);
static bool _ClearSendDataList(THttpTransport inTpt);
static Uint32 _GetUnsentSize(THttpTransport inTpt);

static void _CleanConnectionOpenList();
static void _ClearConnectionOpenList();
static void _CleanConnectionCloseList();
static void _ClearConnectionCloseList();

/*
 * Global Variables
 */

static TTimer _gConnectionOpenCloseTimer = nil;
static CPtrList<SConnectionInfo> _gConnectionOpenList;
static CPtrList<TRegularTransportObj> _gConnectionCloseList;
static CPtrList<SHttpTransport> _gHttpConnectionList;

static Uint32 _gPackageSize 			= 14336;	// 14k
static Uint32 _gPaddingTime1			= 2000;		// 2 secs
static Uint32 _gPaddingTime2			= 40000;	// 40 secs
static Uint32 _gConnectionTime			= 30000;	// 30 secs
static Uint32 _gConnectionOpenCloseTime	= 120000;	// 2 mins


/* -------------------------------------------------------------------------- */

static void _Deinit()
{
	UTimer::Dispose(_gConnectionOpenCloseTimer);

	_ClearConnectionOpenList();
	_gHttpConnectionList.Clear();
	_ClearConnectionCloseList();
}

void UHttpTransport::Init()
{
	_gConnectionOpenCloseTimer = UTimer::New(_HttpConnectionOpenCloseTimerHandler, nil);
	UTimer::Start(_gConnectionOpenCloseTimer, _gConnectionOpenCloseTime, kRepeatingTimer);	

	UProgramCleanup::InstallAppl(_Deinit);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

THttpTransport UHttpTransport::New(Uint32 inProtocol, Uint32 inOptions)
{
	SHttpTransport *tpt = (SHttpTransport *)UMemory::NewClear(sizeof(SHttpTransport));
		
	try
	{
		tpt->nProtocol = inProtocol;
		tpt->nOptions = inOptions;
		
		tpt->tptSend = URegularTransport::New(tpt->nProtocol, tpt->nOptions);
		URegularTransport::SetMessageHandler(tpt->tptSend, _HttpConnectionMessageHandler, tpt);
		tpt->bSendPostHeader = true;

		tpt->tptReceive = URegularTransport::New(tpt->nProtocol, tpt->nOptions);
		URegularTransport::SetMessageHandler(tpt->tptReceive, _HttpConnectionMessageHandler, tpt);
		tpt->bSendGetHeader = true;
		tpt->nReceiveHeader = http_ReceiveHeader;
		
		tpt->msgTimer = UTimer::New(_HttpConnectionTimerHandler, tpt);

		UGUID::Generate(tpt->stConnectionGuid);
		UGUID::Flatten(tpt->stConnectionGuid, &tpt->stConnectionGuid);
		
		_gHttpConnectionList.AddItem(tpt);
	}
	catch(...)
	{
		URegularTransport::Dispose(tpt->tptSend);
		URegularTransport::Dispose(tpt->tptReceive);
		UMemory::Dispose((TPtr)tpt);
		throw;
	}
	
	return (THttpTransport)tpt;
}

void UHttpTransport::Dispose(THttpTransport inTpt)
{
	if (inTpt)
	{
		_gHttpConnectionList.RemoveItem((SHttpTransport *)inTpt);
		UApplication::FlushMessages(_HttpConnectionMessageHandler, inTpt);
		
		if (TPT->msgProc)
			UApplication::FlushMessages(TPT->msgProc, TPT->msgProcContext, TPT->msgObject);
				
		UTimer::Dispose(TPT->msgTimer);

		if (TPT->tptSend)
		{
			if (!TPT->bIsDisconnecting && URegularTransport::IsConnected(TPT->tptSend))
				_SendDisconnectGet(TPT->tptSend);
			else
				URegularTransport::Dispose(TPT->tptSend);
			
			TPT->tptSend = nil;		// in _ClearSendDataList tptSent must be nil
		}
		
		if (TPT->tptReceive)
		{
			if (!TPT->bIsDisconnecting && URegularTransport::IsConnected(TPT->tptReceive))
				_SendDisconnectPost(TPT->tptReceive);
			else
				URegularTransport::Dispose(TPT->tptReceive);
		}
		
		if (TPT->pAddress)
			UMemory::Dispose((TPtr)TPT->pAddress);
		
		if (TPT->pReceivedData)
			UMemory::Dispose((TPtr)TPT->pReceivedData);
		
		_ClearSendDataList(inTpt);			
		UMemory::Dispose((TPtr)inTpt);
	}
}

void UHttpTransport::SetDataMonitor(THttpTransport inTpt, TTransportMonitorProc inProc)
{
	Require(inTpt);
	
	if (TPT->tptSend)
		URegularTransport::SetDataMonitor(TPT->tptSend, inProc);
	
	if (TPT->tptReceive)
		URegularTransport::SetDataMonitor(TPT->tptReceive, inProc);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void UHttpTransport::SetMessageHandler(THttpTransport inTpt, TMessageProc inProc, void *inContext, void *inObject)
{
	Require(inTpt);
	
	TPT->msgProc = inProc;
	TPT->msgProcContext = inContext;
	TPT->msgObject = inObject;
}

void UHttpTransport::PostMessage(THttpTransport inTpt, Uint32 inMsg, const void *inData, Uint32 inDataSize, Int16 inPriority)
{
	Require(inTpt);
	
	if (TPT->msgProc)
		UApplication::PostMessage(inMsg, inData, inDataSize, inPriority, TPT->msgProc, TPT->msgProcContext, TPT->msgObject);
}

void UHttpTransport::ReplaceMessage(THttpTransport inTpt, Uint32 inMsg, const void *inData, Uint32 inDataSize, Int16 inPriority)
{
	Require(inTpt);
	
	if (TPT->msgProc)
		UApplication::ReplaceMessage(inMsg, inData, inDataSize, inPriority, TPT->msgProc, TPT->msgProcContext, TPT->msgObject);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void UHttpTransport::SetIPAddress(THttpTransport inTpt, SIPAddress inIPAddress)
{
	Require(inTpt);

	if (TPT->tptSend)
		URegularTransport::SetIPAddress(TPT->tptSend, inIPAddress);
	
	if (TPT->tptReceive)
		URegularTransport::SetIPAddress(TPT->tptReceive, inIPAddress);
}

SIPAddress UHttpTransport::GetIPAddress(THttpTransport inTpt)
{
	Require(inTpt);

	if (TPT->tptReceive)
		return URegularTransport::GetIPAddress(TPT->tptReceive);
	else if (TPT->tptSend)
		return URegularTransport::GetIPAddress(TPT->tptSend);
		
	SIPAddress stIPAddress;
	stIPAddress.un_IP.stDW_IP.nDW_IP = 0;
	
	return stIPAddress;
}

Uint32 UHttpTransport::GetRemoteAddressText(THttpTransport inTpt, void *outAddr, Uint32 inMaxSize)
{
	Require(inTpt);

	if (TPT->tptReceive)
		return URegularTransport::GetRemoteAddressText(TPT->tptReceive, outAddr, inMaxSize);
	else if (TPT->tptSend)
		return URegularTransport::GetRemoteAddressText(TPT->tptSend, outAddr, inMaxSize);
		
	return 0;
}

Uint32 UHttpTransport::GetLocalAddressText(THttpTransport inTpt, void *outAddr, Uint32 inMaxSize)
{
	Require(inTpt);

	if (TPT->tptReceive)
		return URegularTransport::GetLocalAddressText(TPT->tptReceive, outAddr, inMaxSize);
	else if (TPT->tptSend)
		return URegularTransport::GetLocalAddressText(TPT->tptSend, outAddr, inMaxSize);
	
	return 0;
}

Uint32 UHttpTransport::GetLocalAddress(THttpTransport inTpt, void *outAddr, Uint32 inMaxSize)
{
	Require(inTpt);

	if (TPT->tptReceive)
		return URegularTransport::GetLocalAddress(TPT->tptReceive, outAddr, inMaxSize);
	else if (TPT->tptSend)
		return URegularTransport::GetLocalAddress(TPT->tptSend, outAddr, inMaxSize);
	
	return 0;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

bool UHttpTransport::IsConnected(THttpTransport inTpt)
{
	Require(inTpt);

	if (!TPT->bIsEstablished || TPT->bIsDisconnecting)
		return false;
		
	return true;
}

void UHttpTransport::StartConnectThruFirewall(THttpTransport inTpt, const void *inAddr, Uint32 inAddrSize, const void *inFirewallAddr, Uint32 inFirewallAddrSize, Uint32 inMaxSecs, Uint32 inOptions)
{
	Require(inTpt);
	
	if (!TPT->tptSend || !TPT->tptReceive)
		return;
	
	if (!_CopyAddress(inTpt, inAddr, inAddrSize))
		return;
		
	SInternetNameAddress stAddress;
	if (_GetProxyAddresss(stAddress))
	{
		// connect with proxy
		URegularTransport::StartConnectThruFirewall(TPT->tptSend, &stAddress, stAddress.name[0] + 5, inFirewallAddr, inFirewallAddrSize, inMaxSecs, inOptions);
		URegularTransport::StartConnectThruFirewall(TPT->tptReceive, &stAddress, stAddress.name[0] + 5, inFirewallAddr, inFirewallAddrSize, inMaxSecs, inOptions);
	}
	else
	{
		URegularTransport::StartConnectThruFirewall(TPT->tptSend, TPT->pAddress, TPT->nAddressSize, inFirewallAddr, inFirewallAddrSize, inMaxSecs, inOptions);
		URegularTransport::StartConnectThruFirewall(TPT->tptReceive, TPT->pAddress, TPT->nAddressSize, inFirewallAddr, inFirewallAddrSize, inMaxSecs, inOptions);
	}
	
	TPT->bIsConnectingSend = true;		
	TPT->bIsConnectingReceive = true;
	UTimer::Start(TPT->msgTimer, _gConnectionTime, kOnceTimer);
}

void UHttpTransport::StartConnect(THttpTransport inTpt, const void *inAddr, Uint32 inAddrSize, Uint32 inMaxSecs)
{
	Require(inTpt);

	if (!TPT->tptSend || !TPT->tptReceive)
		return;

	if (!_CopyAddress(inTpt, inAddr, inAddrSize))
		return;
	
	SInternetNameAddress stAddress;
	if (_GetProxyAddresss(stAddress))
	{
		// connect with proxy
		URegularTransport::StartConnect(TPT->tptSend, &stAddress, stAddress.name[0] + 5, inMaxSecs);
		URegularTransport::StartConnect(TPT->tptReceive, &stAddress, stAddress.name[0] + 5, inMaxSecs);
	}
	else
	{
		URegularTransport::StartConnect(TPT->tptSend, TPT->pAddress, TPT->nAddressSize, inMaxSecs);
		URegularTransport::StartConnect(TPT->tptReceive, TPT->pAddress, TPT->nAddressSize, inMaxSecs);
	}
	
	TPT->bIsConnectingSend = true;		
	TPT->bIsConnectingReceive = true;
	UTimer::Start(TPT->msgTimer, _gConnectionTime, kOnceTimer);
}

Uint16 UHttpTransport::GetConnectStatus(THttpTransport inTpt)
{
	Require(inTpt);

	if ((TPT->bIsConnectingSend || TPT->bIsConnectingReceive))
	{
		if (TPT->bIsDisconnecting)
			Fail(errorType_Transport, transError_DataTimedOut);
	
		return kEstablishingConnection;
	}
	
	if (!TPT->bIsEstablished)
		return kWaitingForConnection;
		
	return kConnectionEstablished;
}

void UHttpTransport::Disconnect(THttpTransport inTpt)
{
	Require(inTpt);

	if (TPT->bIsDisconnecting)
		return;
		
	TPT->bIsDisconnecting = true;
	
	if (TPT->tptSend && URegularTransport::IsConnected(TPT->tptSend))
	{
		_SendDisconnectGet(TPT->tptSend);
		TPT->tptSend = nil;
	}

	if (TPT->tptReceive && URegularTransport::IsConnected(TPT->tptReceive))
	{
		_SendDisconnectPost(TPT->tptReceive);
		TPT->tptReceive = nil;
	}

	UHttpTransport::PostMessage(inTpt, msg_ConnectionClosed, nil, 0, priority_Normal);
}

void UHttpTransport::StartDisconnect(THttpTransport inTpt)
{
	Require(inTpt);

	if (TPT->bIsDisconnecting)
		return;

	TPT->bIsDisconnecting = true;

	if (TPT->tptSend && URegularTransport::IsConnected(TPT->tptSend))
	{
		_SendDisconnectGet(TPT->tptSend);
		TPT->tptSend = nil;
	}

	if (TPT->tptReceive && URegularTransport::IsConnected(TPT->tptReceive))
	{
		_SendDisconnectPost(TPT->tptReceive);
		TPT->tptReceive = nil;
	}

	UHttpTransport::PostMessage(inTpt, msg_ConnectionClosed, nil, 0, priority_Normal);
}

bool UHttpTransport::IsDisconnecting(THttpTransport inTpt)
{
	Require(inTpt);

	return TPT->bIsDisconnecting;
}

bool UHttpTransport::IsConnecting(THttpTransport inTpt)
{
	Require(inTpt);

	if (!TPT->bIsEstablished)
		return true;
		
	return false;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void UHttpTransport::Listen(THttpTransport inTpt, const void *inAddr, Uint32 inAddrSize)
{
	Require(inTpt);

	if (TPT->tptReceive)
		URegularTransport::Listen(TPT->tptReceive, inAddr, inAddrSize);
}

THttpTransport UHttpTransport::Accept(THttpTransport inTpt, void *outAddr, Uint32 *ioAddrSize)
{
	Require(inTpt);
	
	SGUID stConnectionGuid;
	TRegularTransport tptSend, tptReceive;
	if (!_SearchConnectionOpenPair(inTpt, tptSend, tptReceive, stConnectionGuid, outAddr, ioAddrSize))
		return nil;
	
	SHttpTransport *tpt = (SHttpTransport *)UMemory::NewClear(sizeof(SHttpTransport));
		
	tpt->tptSend = tptSend;
	URegularTransport::SetMessageHandler(tpt->tptSend, _HttpConnectionMessageHandler, tpt);
	tpt->tptReceive = tptReceive;
	URegularTransport::SetMessageHandler(tpt->tptReceive, _HttpConnectionMessageHandler, tpt);
	
	tpt->msgTimer = UTimer::New(_HttpConnectionTimerHandler, tpt);
	UTimer::Start(tpt->msgTimer, _gPaddingTime2, kOnceTimer);

	tpt->stConnectionGuid = stConnectionGuid;
	tpt->bIsEstablished = true;

	_gHttpConnectionList.AddItem(tpt);
	UApplication::PostMessage(msg_DataArrived, nil, 0, priority_Normal, _HttpConnectionMessageHandler, tpt, tpt->tptReceive);

	return (THttpTransport)tpt;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void UHttpTransport::SendBuffer(THttpTransport inTpt, void *inBuf)
{
	Require(inTpt);
	
	if (!TPT->tptSend || !inBuf)
		return;

	Uint32 nBufSize = URegularTransport::GetBufferSize(inBuf);

	if (TPT->bIsConnectingSend)
	{
		_AddSendDataList(inTpt, nBufSize);

		SSendData *pSendData = (SSendData *)UMemory::New(sizeof(SSendData));
		
		pSendData->pSendData = inBuf;
		pSendData->nDataSize = nBufSize;
		pSendData->bDataBuffer = false;

		TPT->stSendDataList.AddItem(pSendData);
		return;
	}
	
	SData stData;
	stData.nDataCode = TB((Uint32)http_Data);
	stData.nDataSize = TB(nBufSize);

	URegularTransport::Send(TPT->tptSend, &stData, sizeof(SData));
	URegularTransport::SendBuffer(TPT->tptSend, inBuf);
	
	_SendHttpPadding(TPT->tptSend, nBufSize);
	UTimer::Start(TPT->msgTimer, _gPaddingTime1, kOnceTimer);
}

void UHttpTransport::Send(THttpTransport inTpt, const void *inData, Uint32 inDataSize)
{
	Require(inTpt);
	
	if (!TPT->tptSend || !inData || !inDataSize)
		return;
	
	if (TPT->bIsConnectingSend)
	{
		_AddSendDataList(inTpt, inDataSize);
		
		SSendData *pSendData = (SSendData *)UMemory::New(sizeof(SSendData));
		
		pSendData->pSendData = UMemory::New(inDataSize);
		if (!pSendData->pSendData)
		{
			UMemory::Dispose((TPtr)pSendData);
			return;
		}
		
		UMemory::Copy(pSendData->pSendData, inData, inDataSize);
		pSendData->nDataSize = inDataSize;
		pSendData->bDataBuffer = true;

		TPT->stSendDataList.AddItem(pSendData);
		return;
	}

	SData stData;
	stData.nDataCode = TB((Uint32)http_Data);
	stData.nDataSize = TB(inDataSize);
	
	URegularTransport::Send(TPT->tptSend, &stData, sizeof(SData));
	URegularTransport::Send(TPT->tptSend, inData, inDataSize);
	
	_SendHttpPadding(TPT->tptSend, inDataSize);		
	UTimer::Start(TPT->msgTimer, _gPaddingTime1, kOnceTimer);
}

bool UHttpTransport::ReceiveBuffer(THttpTransport inTpt, void*& outBuf, Uint32& outSize)
{
	Require(inTpt);
	
	outBuf = nil;
	outSize = 0;

	if (TPT->stReceiveData.nDataCode == http_DisconnectData)
	{
		outSize = TPT->stReceiveData.nDataSize - TPT->nReceivedOffset;
			
		outBuf = URegularTransport::NewBuffer(outSize);
		if (!outBuf)
			return false;
		
		UMemory::Copy(outBuf, (Uint8 *)TPT->pReceivedData + TPT->nReceivedOffset, outSize);
		
		UMemory::Dispose((TPtr)TPT->pReceivedData);
		TPT->pReceivedData = nil;
		
		TPT->stReceiveData.nDataCode = 0;
		TPT->stReceiveData.nDataSize = 0;
		TPT->nReceivedOffset = 0; 
				
		return true;
	}

	if (!TPT->tptReceive || TPT->nReceiveHeader || TPT->bIsConnectingReceive)
		return false;

	if ((!TPT->stReceiveData.nDataSize || TPT->stReceiveData.nDataCode != http_Data) && !_ReceiveHttpData(inTpt))
		return false;

	Uint32 nReceiveSize = URegularTransport::GetReceiveSize(TPT->tptReceive);
	if (!nReceiveSize)
		return false;

	if (nReceiveSize > TPT->stReceiveData.nDataSize)
		nReceiveSize = TPT->stReceiveData.nDataSize;

	outBuf = URegularTransport::NewBuffer(nReceiveSize);
	if (!outBuf)
		return false;
	
	outSize = URegularTransport::Receive(TPT->tptReceive, outBuf, nReceiveSize);
	TPT->stReceiveData.nDataSize -= outSize;

	UApplication::PostMessage(msg_DataArrived, nil, 0, priority_Normal, _HttpConnectionMessageHandler, inTpt, TPT->tptReceive);
	return true;
}

Uint32 UHttpTransport::Receive(THttpTransport inTpt, void *outData, Uint32 inMaxSize)
{
	Require(inTpt);
	
	if (TPT->stReceiveData.nDataCode == http_DisconnectData)
	{
		Uint32 nDataSize = TPT->stReceiveData.nDataSize - TPT->nReceivedOffset;
		if (nDataSize > inMaxSize)
			nDataSize = inMaxSize;
			
		UMemory::Copy(outData, (Uint8 *)TPT->pReceivedData + TPT->nReceivedOffset, nDataSize);
		TPT->nReceivedOffset += nDataSize;
		
		if (TPT->nReceivedOffset == TPT->stReceiveData.nDataSize)
		{
			UMemory::Dispose((TPtr)TPT->pReceivedData);
			TPT->pReceivedData = nil;
			
			TPT->stReceiveData.nDataCode = 0;
			TPT->stReceiveData.nDataSize = 0;
			TPT->nReceivedOffset = 0; 
		}
		else
			UHttpTransport::PostMessage(inTpt, msg_DataArrived, nil, 0, priority_Normal);			
		
		return nDataSize;
	}

	if (!TPT->tptReceive || TPT->nReceiveHeader || TPT->bIsConnectingReceive)
		return 0;

	if ((!TPT->stReceiveData.nDataSize || TPT->stReceiveData.nDataCode != http_Data) && !_ReceiveHttpData(inTpt))
		return 0;
	
	Uint32 nReceiveSize = URegularTransport::GetReceiveSize(TPT->tptReceive);
	if (!nReceiveSize)
		return 0;
	
	if (nReceiveSize > TPT->stReceiveData.nDataSize)
		nReceiveSize = TPT->stReceiveData.nDataSize;
	
	if (nReceiveSize > inMaxSize)
		nReceiveSize = inMaxSize;
		
    nReceiveSize = URegularTransport::Receive(TPT->tptReceive, outData, nReceiveSize);
	TPT->stReceiveData.nDataSize -= nReceiveSize;

	UApplication::PostMessage(msg_DataArrived, nil, 0, priority_Normal, _HttpConnectionMessageHandler, inTpt, TPT->tptReceive);
	return nReceiveSize;
}

Uint32 UHttpTransport::GetReceiveSize(THttpTransport inTpt)
{
	Require(inTpt);

	if (TPT->stReceiveData.nDataCode == http_DisconnectData)
		return TPT->stReceiveData.nDataSize - TPT->nReceivedOffset;

	if (!TPT->tptReceive || TPT->nReceiveHeader || TPT->bIsConnectingReceive)
		return 0;

	if (!TPT->stReceiveData.nDataSize || TPT->stReceiveData.nDataCode != http_Data)
	{
		if (_ReceiveHttpData(inTpt))
			UHttpTransport::PostMessage(inTpt, msg_DataArrived, nil, 0, priority_Normal);
		else
			return 0;
	}

	Uint32 nReceiveSize =  URegularTransport::GetReceiveSize(TPT->tptReceive);
	return (nReceiveSize > TPT->stReceiveData.nDataSize ? TPT->stReceiveData.nDataSize : nReceiveSize);
}

Uint32 UHttpTransport::GetUnsentSize(THttpTransport inTpt)
{
	Require(inTpt);

	if (TPT->bIsConnectingSend)
		return _GetUnsentSize(inTpt);

	if (TPT->tptSend)
		return URegularTransport::GetUnsentSize(TPT->tptSend);
		
	return 0;
}

void UHttpTransport::NotifyReadyToReceive(THttpTransport inTpt, Uint32 inReceivedSize)
{
	Require(inTpt);
	
	TPT->nNotifyReceivedSize = 0;
	
	if (!TPT->tptReceive)
		return;
	
	if (!inReceivedSize)
	{
		URegularTransport::NotifyReadyToReceive(TPT->tptReceive, inReceivedSize);
		return;
	}
	
	if (TPT->stReceiveData.nDataCode == http_DisconnectData)
	{
		if (inReceivedSize <= TPT->stReceiveData.nDataSize)
			UHttpTransport::PostMessage(inTpt, msg_DataArrived, nil, 0, priority_Normal);
		
		return;		
	}

	if (TPT->stReceiveData.nDataCode != http_Data || !TPT->stReceiveData.nDataSize)
	{
		TPT->nNotifyReceivedSize = inReceivedSize;
		return;
	}
	
	if (inReceivedSize <= TPT->stReceiveData.nDataSize)
		URegularTransport::NotifyReadyToReceive(TPT->tptReceive, inReceivedSize);
}

void UHttpTransport::NotifyReadyToSend(THttpTransport inTpt)
{
	Require(inTpt);

	if (!TPT->tptSend || TPT->bIsConnectingSend)
		return;

	URegularTransport::NotifyReadyToSend(TPT->tptSend);
}

bool UHttpTransport::IsReadyToReceive(THttpTransport inTpt, Uint32 inReceivedSize)
{
	Require(inTpt);
	
	TPT->nNotifyReceivedSize = 0;
	
	if (!TPT->tptReceive)
		return false;
	
	if (!inReceivedSize)
		return URegularTransport::IsReadyToReceive(TPT->tptReceive, inReceivedSize);

	if (TPT->stReceiveData.nDataCode == http_DisconnectData)
	{
		if (inReceivedSize <= TPT->stReceiveData.nDataSize)
			return true;
		
		return false;
	}

	if (TPT->stReceiveData.nDataCode != http_Data || !TPT->stReceiveData.nDataSize)
	{
		TPT->nNotifyReceivedSize = inReceivedSize;
		return false;
	}

	if (inReceivedSize <= TPT->stReceiveData.nDataSize)
		return URegularTransport::IsReadyToReceive(TPT->tptReceive, inReceivedSize);
		
	return false;
}

bool UHttpTransport::IsReadyToSend(THttpTransport inTpt)
{
	Require(inTpt);

	if (!TPT->tptSend || TPT->bIsConnectingSend)
		return false;

	return URegularTransport::IsReadyToSend(TPT->tptSend);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void UHttpTransport::SendUnit(THttpTransport inTpt, const void *inAddr, Uint32 inAddrSize, const void *inData, Uint32 inDataSize)
{
	Require(inTpt);

	if (TPT->tptSend)
		URegularTransport::SendUnit(TPT->tptSend, inAddr, inAddrSize, inData, inDataSize);
}

bool UHttpTransport::ReceiveUnit(THttpTransport inTpt, void *outAddr, Uint32& ioAddrSize, void *outData, Uint32& ioDataSize)
{
	Require(inTpt);

	if (TPT->tptReceive)
		return URegularTransport::ReceiveUnit(TPT->tptReceive, outAddr, ioAddrSize, outData, ioDataSize);
		
	return false;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

static void _HttpConnectionOpenMessageHandler(void *inContext, void *inObject, Uint32 inMsg, const void *inData, Uint32 inDataSize)
{
	#pragma unused(inContext, inObject, inData, inDataSize)
	SConnectionInfo *pConnectionInfo = (SConnectionInfo *)inContext;
	
	switch (inMsg)
	{
		case msg_ConnectionEstablished:
		case msg_ConnectionRefused:
			break;
			
		case msg_DataArrived:
		#if LOOP_DATA_ARRIVED
			_ProcessConnections();
			break;
		#else
			if (pConnectionInfo->nReceiveHeader && _ReceiveHttpHeader(pConnectionInfo))
			{
				pConnectionInfo->nReceiveHeader = 0;
				
				if (_SearchReconnect(pConnectionInfo))
				{
					_gConnectionOpenList.RemoveItem(pConnectionInfo);
					UMemory::Dispose((TPtr)pConnectionInfo);

					return;
				}
				
				UHttpTransport::PostMessage((THttpTransport)pConnectionInfo->pHttpTransport, msg_ConnectionRequest, nil, 0, priority_Normal);
			}
			break;
		#endif
	
		case msg_ConnectionClosed:		
		case msg_DataTimeOut:
		case msg_ReadyToSend:
		case msg_ConnectionRequest:
			break;
    };
}

static void _HttpConnectionMessageHandler(void *inContext, void *inObject, Uint32 inMsg, const void *inData, Uint32 inDataSize)
{
	#pragma unused(inObject)

	SHttpTransport *pHttpTransport = (SHttpTransport *)inContext;
	
	switch (inMsg)
	{
		case msg_ConnectionEstablished:
			if (pHttpTransport->bSendPostHeader && URegularTransport::IsConnected(pHttpTransport->tptSend))
			{
				pHttpTransport->bSendPostHeader = false;
				_SendHttpPostHeader((THttpTransport)pHttpTransport);
				
				_CheckConnectionEstabilished((THttpTransport)pHttpTransport);
			}
			else if (pHttpTransport->bSendGetHeader && URegularTransport::IsConnected(pHttpTransport->tptReceive))
			{			
				pHttpTransport->bSendGetHeader = false;
				_SendHttpGetHeader((THttpTransport)pHttpTransport);	
			}
			return; // don't post msg_ConnectionEstablished message
		
		case msg_ConnectionRefused:
			break;
			
		case msg_DataArrived:
		#if LOOP_DATA_ARRIVED
			_ProcessHttpData();
			return;	// don't post msg_DataArrived message
		#else
			if (pHttpTransport->nReceiveHeader)
			{
				if (_ReceiveHttpHeader((THttpTransport)pHttpTransport))
				{
					pHttpTransport->nReceiveHeader = 0;
					_CheckConnectionEstabilished((THttpTransport)pHttpTransport);
				}
			
				return; // don't post msg_DataArrived message	
			}
			
			if (!_ReceiveHttpData((THttpTransport)pHttpTransport))
				return; // don't post msg_DataArrived message	
				
			break;
		#endif
		
		case msg_ConnectionClosed:
			if (!pHttpTransport->bIsDisconnecting)
			{
				if (pHttpTransport->nProtocol)
				{
					if (!_TryReconnect((THttpTransport)pHttpTransport))
						UHttpTransport::Disconnect((THttpTransport)pHttpTransport);
				}
				else
				{
					if (!_WaitReconnect((THttpTransport)pHttpTransport))
						UHttpTransport::Disconnect((THttpTransport)pHttpTransport);
				}
			}
			return; 	// don't post msg_ConnectionClosed message
			
		case msg_DataTimeOut:
		case msg_ReadyToSend:
			break;
			
		case msg_ConnectionRequest:
			_AcceptConnection((THttpTransport)pHttpTransport);
			return;	// don't post msg_ConnectionRequest message
    };

	UHttpTransport::PostMessage((THttpTransport)pHttpTransport, inMsg, inData, inDataSize, priority_Normal);
}

static void _HttpConnectionTimerHandler(void *inContext, void *inObject, Uint32 inMsg, const void *inData, Uint32 inDataSize)
{
	#pragma unused(inObject, inData, inDataSize)

	if (inMsg == msg_Timer)
	{
		SHttpTransport *pHttpTransport = (SHttpTransport *)inContext;

		if (pHttpTransport->bIsConnectingSend || pHttpTransport->bIsConnectingReceive)
		{
			UHttpTransport::Disconnect((THttpTransport)pHttpTransport);
			return;
		}
	
		if (pHttpTransport->tptSend && URegularTransport::IsConnected(pHttpTransport->tptSend))
		{
			_SendHttpPadding(pHttpTransport->tptSend);
			UTimer::Start(pHttpTransport->msgTimer, _gPaddingTime2, kOnceTimer);
		}
	}
}

static void _HttpConnectionOpenCloseTimerHandler(void *inContext, void *inObject, Uint32 inMsg, const void *inData, Uint32 inDataSize)
{
	#pragma unused(inContext, inObject, inData, inDataSize)
	
	if (inMsg == msg_Timer)
	{
		_CleanConnectionOpenList();
		_CleanConnectionCloseList();
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

static bool _CopyAddress(THttpTransport inTpt, const void *inAddress, Uint32 inAddressSize)
{
	if (TPT->pAddress)
	{
		UMemory::Dispose((TPtr)TPT->pAddress);
		TPT->pAddress = nil;
		TPT->nAddressSize = 0;
	}

	if (!inAddress || inAddressSize < 6)
		return false;
	
	Uint16 nType = *(Uint16 *)inAddress;
	if (nType != kInternetAddressType && nType != kInternetNameAddressType)
		return false;
	
	TPT->nAddressSize = inAddressSize;
	TPT->pAddress = UMemory::New(TPT->nAddressSize);
	
	if (!TPT->pAddress)
	{
		TPT->nAddressSize = 0;
		return false;
	}

	UMemory::Copy(TPT->pAddress, inAddress, TPT->nAddressSize);
	return true;
}

static bool _CheckAddress(THttpTransport inTpt, bool inSendReceive)
{
	if (!TPT->pAddress || TPT->nAddressSize < 6)
		return false;
	
	Uint16 nType = *(Uint16 *)TPT->pAddress;
	if (nType != kInternetAddressType && nType != kInternetNameAddressType)
		return false;

	if ((nType == kInternetAddressType && (((SInternetAddress *)TPT->pAddress)->host[0] == 127 && ((SInternetAddress *)TPT->pAddress)->host[1] == 0 && ((SInternetAddress *)TPT->pAddress)->host[2] == 0 && ((SInternetAddress *)TPT->pAddress)->host[3] == 1)) ||
	    (nType == kInternetNameAddressType && !UMemory::Compare(((SInternetNameAddress *)TPT->pAddress)->name + 1, "127.0.0.1", ((SInternetNameAddress *)TPT->pAddress)->name[0])))
	{
		Uint32 nPort;
		if (nType == kInternetAddressType)
			nPort = ((SInternetAddress *)TPT->pAddress)->port;
		else
			nPort = ((SInternetNameAddress *)TPT->pAddress)->port;

		UMemory::Dispose((TPtr)TPT->pAddress);

		TPT->nAddressSize = 16;
		TPT->pAddress = UMemory::New(TPT->nAddressSize);
	
		if (!TPT->pAddress)
		{
			TPT->nAddressSize = 0;
			return false;
		}
		
		TRegularTransport tptReg;
		if (inSendReceive)
			tptReg = TPT->tptSend;
		else
			tptReg = TPT->tptReceive;
				
		TPT->nAddressSize = URegularTransport::GetLocalAddress(tptReg, TPT->pAddress, TPT->nAddressSize);
		((SInternetAddress *)TPT->pAddress)->port = nPort;
	}

	return true;
}

static bool _GetAddress(THttpTransport inTpt, Uint8 *outHost, Uint32 inHostSize, Uint8 *outAddress, Uint32 inAddressSize)
{
	if (!TPT->pAddress || TPT->nAddressSize < 6)
		return false;

	Uint16 nType = *(Uint16 *)TPT->pAddress;
	if (nType != kInternetAddressType && nType != kInternetNameAddressType)
		return false;
	
	if (nType == kInternetAddressType)
	{
		if (TPT->nAddressSize < sizeof(SInternetAddress))
			return false;
		
		const SInternetAddress& stAddress = *(SInternetAddress *)TPT->pAddress;
		
		if (stAddress.port)
			outHost[0] = UText::Format(outHost + 1, inHostSize - 1, "%d.%d.%d.%d:%hu", stAddress.host[0], stAddress.host[1], stAddress.host[2], stAddress.host[3], stAddress.port);
		else
			outHost[0] = UText::Format(outHost + 1, inHostSize - 1, "%d.%d.%d.%d", stAddress.host[0], stAddress.host[1], stAddress.host[2], stAddress.host[3]);
	}
	else if (nType == kInternetNameAddressType)
	{
		if (TPT->nAddressSize > sizeof(SInternetNameAddress))
			return false;
				
		const SInternetNameAddress& stAddress = *(SInternetNameAddress *)TPT->pAddress;
		Uint8 *pPortNum = UMemory::SearchByte(':', stAddress.name + 1, stAddress.name[0]);

		if (stAddress.port && !pPortNum)
			outHost[0] = UText::Format(outHost + 1, inHostSize - 1, "%#s:%hu", stAddress.name, stAddress.port);
		else
			outHost[0] = UMemory::Copy(outHost + 1, stAddress.name + 1, stAddress.name[0] > inHostSize ? inHostSize : stAddress.name[0]);
	}
	
	Uint8 bufConnectionGuid[37];
	bufConnectionGuid[0] = UGUID::ToText(TPT->stConnectionGuid, bufConnectionGuid + 1, sizeof(bufConnectionGuid) - 1);
	outAddress[0] = UText::Format(outAddress + 1, inAddressSize - 1, "http://%#s/%#s", outHost, bufConnectionGuid);

	return true;
}

static bool _GetProxyAddresss(SInternetNameAddress& outAddress)
{
	Uint8 psProxyAddr[256];
	psProxyAddr[0] = URegularTransport::GetProxyServerAddress(proxy_HttpAddress, psProxyAddr + 1, sizeof(psProxyAddr) - 1);
	if (!psProxyAddr[0])
		return false;
		
	Uint32 nProxyPort = port_number_HTTP;
	Uint32 nTempProxyPort = UTransport::GetPortFromAddressText(kInternetNameAddressType, psProxyAddr + 1, psProxyAddr[0]);
		
	if (nTempProxyPort)
	{
		nProxyPort = nTempProxyPort;
			
		Uint8 *pProxyPort = UMemory::SearchByte(':', psProxyAddr + 1, psProxyAddr[0]);
		if (pProxyPort) 
			psProxyAddr[0] -= psProxyAddr + psProxyAddr[0] + 1 - pProxyPort;
	}
	
	outAddress.type = kInternetNameAddressType;
	outAddress.port = nProxyPort;
	UMemory::Copy(outAddress.name, psProxyAddr, psProxyAddr[0] + 1);
	
	return true;
}

static bool _AcceptConnection(THttpTransport inTpt)
{
	if (!TPT->tptReceive)
		return false;
	
	SInternetAddress stAddress;
	Uint32 nAddressSize = sizeof(SInternetAddress);

	TRegularTransport tptReg = URegularTransport::Accept(TPT->tptReceive, &stAddress, &nAddressSize);
	if (!tptReg)
		return false;
		
	SConnectionInfo *pConnectionInfo = (SConnectionInfo *)UMemory::NewClear(sizeof(SConnectionInfo));
	
	pConnectionInfo->tptReg = tptReg;
	URegularTransport::SetMessageHandler(pConnectionInfo->tptReg, _HttpConnectionOpenMessageHandler, pConnectionInfo);

	pConnectionInfo->pHttpTransport = TPT;
	pConnectionInfo->stAddress = stAddress;
	pConnectionInfo->nConnectionTime = UDateTime::GetMilliseconds();
	pConnectionInfo->nReceiveHeader = http_ReceiveHeader;
			
	_gConnectionOpenList.AddItem(pConnectionInfo);

	return true;
}

static bool _CheckConnectionEstabilished(THttpTransport inTpt)
{
	if (!TPT->bSendPostHeader && !TPT->bSendGetHeader && !TPT->nReceiveHeader)
	{				
		TPT->bIsConnectingSend = false;
		TPT->bIsConnectingReceive = false;

		if (!TPT->bIsEstablished)
		{
			TPT->bIsEstablished = true;			 
			UHttpTransport::PostMessage(inTpt, msg_ConnectionEstablished, nil, 0, priority_Normal);
		}
				
		if (_ClearSendDataList(inTpt))
			UTimer::Start(TPT->msgTimer, _gPaddingTime1, kOnceTimer);
		else
			UTimer::Start(TPT->msgTimer, _gPaddingTime2, kOnceTimer);

		UApplication::PostMessage(msg_DataArrived, nil, 0, priority_Normal, _HttpConnectionMessageHandler, inTpt, TPT->tptReceive);
		return true;
	}
	
	return false;
}

static bool _SearchConnectionOpenPair(THttpTransport inTpt, TRegularTransport& outTptSend, TRegularTransport& outTptReceive, SGUID& outConnectionGuid,  void *outAddr, Uint32 *ioAddrSize)
{
	Uint32 i = 0;
	SConnectionInfo *pConnectionSend;
	
	while (_gConnectionOpenList.GetNext(pConnectionSend, i))
	{
		if (pConnectionSend->pHttpTransport != TPT || pConnectionSend->nReceiveHeader || !pConnectionSend->bSendReceive)
			continue;
		
		Uint32 j = 0;
		SConnectionInfo *pConnectionReceive;
	
		while (_gConnectionOpenList.GetNext(pConnectionReceive, j))
		{
			if (i == j || pConnectionReceive->pHttpTransport != TPT || pConnectionReceive->nReceiveHeader || pConnectionReceive->bSendReceive)
				continue;
				
			if (pConnectionSend->stConnectionGuid == pConnectionReceive->stConnectionGuid)
			{
				outTptSend = pConnectionSend->tptReg;
				outTptReceive = pConnectionReceive->tptReg;
				outConnectionGuid = pConnectionReceive->stConnectionGuid;
				
				if (outAddr && ioAddrSize && *ioAddrSize >= sizeof(SInternetAddress))
				{
					*ioAddrSize = sizeof(SInternetAddress);
					UMemory::Copy(outAddr, &pConnectionSend->stAddress, *ioAddrSize);
				}
	
				UApplication::FlushMessages(_HttpConnectionOpenMessageHandler, pConnectionSend);
				_gConnectionOpenList.RemoveItem(pConnectionSend);
				UMemory::Dispose((TPtr)pConnectionSend);
	
				UApplication::FlushMessages(_HttpConnectionOpenMessageHandler, pConnectionReceive);
				_gConnectionOpenList.RemoveItem(pConnectionReceive);
				UMemory::Dispose((TPtr)pConnectionReceive);

				return true;
			}
		}
	}
	
	return false;
}

static bool _TryReconnect(THttpTransport inTpt)
{
	if (!TPT->bIsConnectingSend && TPT->tptSend && !URegularTransport::IsConnected(TPT->tptSend))
	{
		UApplication::FlushMessages(_HttpConnectionTimerHandler, inTpt, TPT->msgTimer);
		UTimer::Start(TPT->msgTimer, _gConnectionTime, kOnceTimer);
		TPT->bIsConnectingSend = true;		

		_TRGetUnsentData(TPT->tptSend, &TPT->stSendDataList);
		URegularTransport::Dispose(TPT->tptSend);
	
		TPT->tptSend = URegularTransport::New(TPT->nProtocol, TPT->nOptions);
		URegularTransport::SetMessageHandler(TPT->tptSend, _HttpConnectionMessageHandler, TPT);
		TPT->bSendPostHeader = true;

		SInternetNameAddress stAddress;
		if (_GetProxyAddresss(stAddress))
			URegularTransport::StartConnect(TPT->tptSend, &stAddress, stAddress.name[0] + 5);
		else
			URegularTransport::StartConnect(TPT->tptSend, TPT->pAddress, TPT->nAddressSize);
		
		return true;
	}

	if (!TPT->bIsConnectingReceive && TPT->tptReceive && !URegularTransport::IsConnected(TPT->tptReceive))
	{
		if (!_ReceiveDisconnectData(inTpt))
			return false;

		UApplication::FlushMessages(_HttpConnectionTimerHandler, inTpt, TPT->msgTimer);
		UTimer::Start(TPT->msgTimer, _gConnectionTime, kOnceTimer);
		TPT->bIsConnectingReceive = true;

		URegularTransport::Dispose(TPT->tptReceive);

		TPT->tptReceive = URegularTransport::New(TPT->nProtocol, TPT->nOptions);
		URegularTransport::SetMessageHandler(TPT->tptReceive, _HttpConnectionMessageHandler, TPT);
		TPT->bSendGetHeader = true;
		TPT->nReceiveHeader = http_ReceiveHeader;
		
		SInternetNameAddress stAddress;
		if (_GetProxyAddresss(stAddress))
			URegularTransport::StartConnect(TPT->tptReceive, &stAddress, stAddress.name[0] + 5);
		else
			URegularTransport::StartConnect(TPT->tptReceive, TPT->pAddress, TPT->nAddressSize);
			
		return true;
	}
	
	return false;
}

static bool _WaitReconnect(THttpTransport inTpt)
{
	if (!TPT->bIsConnectingSend && TPT->tptSend && !URegularTransport::IsConnected(TPT->tptSend))
	{		
		_TRGetUnsentData(TPT->tptSend, &TPT->stSendDataList);

		UApplication::FlushMessages(_HttpConnectionTimerHandler, inTpt, TPT->msgTimer);
		UTimer::Start(TPT->msgTimer, _gConnectionTime, kOnceTimer);
		TPT->bIsConnectingSend = true;
		
		return true;
	}

	if (!TPT->bIsConnectingReceive && TPT->tptReceive && !URegularTransport::IsConnected(TPT->tptReceive))
	{
		if (!_ReceiveDisconnectData(inTpt))
			return false;

		UApplication::FlushMessages(_HttpConnectionTimerHandler, inTpt, TPT->msgTimer);
		UTimer::Start(TPT->msgTimer, _gConnectionTime, kOnceTimer);
		TPT->bIsConnectingReceive = true;
		
		return true;
	}
	
	return false;
}

static bool _SearchReconnect(SConnectionInfo *inConnectionInfo)
{
	Uint32 i = 0;
	SHttpTransport *pHttpTransport;
	
	while (_gHttpConnectionList.GetNext(pHttpTransport, i))
	{
		if (pHttpTransport->stConnectionGuid == inConnectionInfo->stConnectionGuid)
		{
			UApplication::FlushMessages(_HttpConnectionOpenMessageHandler, inConnectionInfo);
			
			TRegularTransport tptReg = inConnectionInfo->tptReg;
			URegularTransport::SetMessageHandler(tptReg, _HttpConnectionMessageHandler, pHttpTransport);
			
			if (inConnectionInfo->bSendReceive)
			{
				_TRGetUnsentData(pHttpTransport->tptSend, &pHttpTransport->stSendDataList);
				URegularTransport::Dispose(pHttpTransport->tptSend);
				
				pHttpTransport->tptSend = tptReg;
				pHttpTransport->bIsConnectingSend = false;
				
				if (_ClearSendDataList((THttpTransport)pHttpTransport))
					UTimer::Start(pHttpTransport->msgTimer, _gPaddingTime1, kOnceTimer);
				else
					UTimer::Start(pHttpTransport->msgTimer, _gPaddingTime2, kOnceTimer);
			}
			else
			{
				URegularTransport::Dispose(pHttpTransport->tptReceive);
				
				pHttpTransport->tptReceive = tptReg;
				pHttpTransport->bIsConnectingReceive = false;

				UApplication::PostMessage(msg_DataArrived, nil, 0, priority_Normal, _HttpConnectionMessageHandler, pHttpTransport, pHttpTransport->tptReceive);
			}

			return true;
		}
	}
	
	return false;
}

static bool _ReceiveDisconnectData(THttpTransport inTpt)
{
	SData stDisconnectData;
	
	if (TPT->stReceiveData.nDataCode == http_DisconnectData)
	{
		stDisconnectData.nDataCode = 0;
		stDisconnectData.nDataSize = 0;
	}
	else
	{
		stDisconnectData = TPT->stReceiveData;

		TPT->stReceiveData.nDataCode = 0;
		TPT->stReceiveData.nDataSize = 0;
	}

	void *pBuffer;
	Uint32 nBufferSize;	
	Uint32 nBufferOffset = 0;
	
	if (!URegularTransport::ReceiveBuffer(TPT->tptReceive, pBuffer, nBufferSize))
	{
		if (TPT->stReceiveData.nDataCode == http_Data && TPT->stReceiveData.nDataSize)
			return false;
		
		return true;
	}
	
	bool bRet = false;
		
	while (true)
	{
		if (stDisconnectData.nDataSize > nBufferSize - nBufferOffset)
		{
			if (stDisconnectData.nDataCode == http_Padding)
				bRet = true;
	
			break;
		}
		
		if (stDisconnectData.nDataSize)
		{
			if (stDisconnectData.nDataCode == http_Data)
			{
				Uint32 nDataOffset = TPT->stReceiveData.nDataSize;
				TPT->stReceiveData.nDataSize += stDisconnectData.nDataSize;
				TPT->stReceiveData.nDataCode = http_DisconnectData;
				
				if (TPT->pReceivedData)
					TPT->pReceivedData = UMemory::Reallocate((TPtr)TPT->pReceivedData, TPT->stReceiveData.nDataSize);
				else
					TPT->pReceivedData = UMemory::New(TPT->stReceiveData.nDataSize);
				
				if (!TPT->pReceivedData)
				{
					TPT->stReceiveData.nDataCode = 0;
					TPT->stReceiveData.nDataSize = 0;
					TPT->nReceivedOffset = 0;

					break;
				}
				
				UMemory::Copy((Uint8 *)TPT->pReceivedData + nDataOffset, (Uint8 *)pBuffer + nBufferOffset, stDisconnectData.nDataSize);
			}
			
			nBufferOffset += stDisconnectData.nDataSize;
		}

		if (nBufferSize - nBufferOffset < sizeof(SData))
		{
			bRet = true;
			break;
		}

		stDisconnectData = *(SData *)((Uint8 *)pBuffer + nBufferOffset);
		nBufferOffset += sizeof(SData);

		stDisconnectData.nDataCode = TB(stDisconnectData.nDataCode);
		stDisconnectData.nDataSize = TB(stDisconnectData.nDataSize);
			
		if (stDisconnectData.nDataCode != http_Data && stDisconnectData.nDataCode != http_Padding)
			break;
	}
	
	URegularTransport::DisposeBuffer(pBuffer);
	return bRet;
}

static void _Disconnect(THttpTransport inTpt)
{
	if (TPT->bIsDisconnecting)
		return;
	
	TPT->bIsDisconnecting = true;
		
	if (TPT->tptSend && URegularTransport::IsConnected(TPT->tptSend))
		URegularTransport::Disconnect(TPT->tptSend);
			
	if (TPT->tptReceive && URegularTransport::IsConnected(TPT->tptReceive))
	{
		_SendDisconnectPost(TPT->tptReceive);
		TPT->tptReceive = nil;
	}
			
	UHttpTransport::PostMessage(inTpt, msg_ConnectionClosed, nil, 0, priority_Normal);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

#if LOOP_DATA_ARRIVED
static void _ProcessConnections()
{
	Uint32 i = 0;
	SConnectionInfo *pConnectionInfo;
	
	while (gConnectionOpenList.GetNext(pConnectionInfo, i))
	{
		if (!pConnectionInfo->nReceiveHeader)
			continue;
			
		if (_ReceiveHttpHeader(pConnectionInfo))
		{
		 	pConnectionInfo->nReceiveHeader = 0;
		 	
		 	if (_SearchReconnect(pConnectionInfo))
		 	{
				gConnectionOpenList.RemoveItem(pConnectionInfo);
				UMemory::Dispose((TPtr)pConnectionInfo);
				
				i--;
				continue;
		 	}

			UHttpTransport::PostMessage((THttpTransport)pConnectionInfo->pHttpTransport, msg_ConnectionRequest, nil, 0, priority_Normal);
		}
	}
}

static void _ProcessHttpData()
{
	Uint32 i = 0;
	SHttpTransport *pHttpTransport;
	
	while (gHttpConnectionList.GetNext(pHttpTransport, i))
	{
		if (pHttpTransport->nReceiveHeader)
		{
			if (_ReceiveHttpHeader((THttpTransport)pHttpTransport))
			{
				pHttpTransport->nReceiveHeader = 0;
				_CheckConnectionEstabilished((THttpTransport)pHttpTransport);
			}
			
			continue;
		}
		
		if (_ReceiveHttpData((THttpTransport)pHttpTransport))
			UHttpTransport::PostMessage((THttpTransport)pHttpTransport, msg_DataArrived, nil, 0, priority_Normal);
	}
}
#endif

/* -------------------------------------------------------------------------- */
#pragma mark -

static bool _SendHttpPostHeader(THttpTransport inTpt)
{
	if (!_CheckAddress(inTpt, true))
		return false;

	Uint8 psHost[256];
	Uint8 psAddress[256];
	if (!_GetAddress(inTpt, psHost, sizeof(psHost), psAddress, sizeof(psAddress)))
		return false;

	void *pBuffer = nil;
	Uint32 nBufferSize = 0;

	if (!_AddField_General(&pBuffer, nBufferSize, "\pPOST ", psAddress, "\p HTTP/1.0") || !_AddField_General(&pBuffer, nBufferSize, "\pProxy-Connection: ", "\pKeep-Alive") ||
	    !_AddField_General(&pBuffer, nBufferSize, "\pPragma: ", "\pno-cache") || !_AddField_General(&pBuffer, nBufferSize, "\pHost: ", psHost) ||
		!_AddField_General(&pBuffer, nBufferSize, "\pContent-Length: ", "\p999999999") || !_AddField_General(&pBuffer, nBufferSize, "\pContent-Type: ", "\photline/protocol") ||
	    !_AddField_End(&pBuffer, nBufferSize))
	{
		UMemory::Dispose((TPtr)pBuffer);
		return false;
	}
		
	URegularTransport::Send(TPT->tptSend, pBuffer, nBufferSize);
	UMemory::Dispose((TPtr)pBuffer);

	_SendHttpPadding(TPT->tptSend, nBufferSize - sizeof(SData));
	_SendHttpPadding(TPT->tptSend);
	_SendHttpPadding(TPT->tptSend);
	_SendHttpPadding(TPT->tptSend);

	return true;
}

static bool _SendHttpResponsePostHeader(TRegularTransport inTpt)
{
	void *pBuffer = nil;
	Uint32 nBufferSize = 0;
		
	if (!_AddField_General(&pBuffer, nBufferSize, "\pHTTP/1.0 ", "\p302 Found") || !_AddField_General(&pBuffer, nBufferSize, "\pConnection: ", "\pclose") ||
	    !_AddField_General(&pBuffer, nBufferSize, "\pContent-Length: ", "\p8") || !_AddField_General(&pBuffer, nBufferSize, "\pContent-Type: ", "\photline/protocol") ||
	    !_AddField_End(&pBuffer, nBufferSize))
	{
		UMemory::Dispose((TPtr)pBuffer);
		return false;
	}
	
	nBufferSize += 8;
	pBuffer = UMemory::Reallocate((TPtr)pBuffer, nBufferSize);
	if (!pBuffer)
		return false;
		
	UMemory::Clear((Uint8 *)pBuffer + nBufferSize - 8, 8);
	
	URegularTransport::Send(inTpt, pBuffer, nBufferSize);
	UMemory::Dispose((TPtr)pBuffer);

	return true;
}

static bool _SendHttpGetHeader(THttpTransport inTpt)
{	
	if (!_CheckAddress(inTpt, false))
		return false;
	
	Uint8 psHost[256];
	Uint8 psAddress[256];
	if (!_GetAddress(inTpt, psHost, sizeof(psHost), psAddress, sizeof(psAddress)))
		return false;
	
	void *pBuffer = nil;
	Uint32 nBufferSize = 0;
	
	if (!_AddField_General(&pBuffer, nBufferSize, "\pGET ", psAddress, "\p HTTP/1.0") || !_AddField_General(&pBuffer, nBufferSize, "\pProxy-Connection: ", "\pKeep-Alive") ||
		!_AddField_General(&pBuffer, nBufferSize, "\pPragma: ", "\pno-cache") || !_AddField_General(&pBuffer, nBufferSize, "\pHost: ", psHost) ||
		!_AddField_General(&pBuffer, nBufferSize, "\pAccept: ", "\photline/protocol") || !_AddField_End(&pBuffer, nBufferSize))
	{
		UMemory::Dispose((TPtr)pBuffer);
		return false;
	}
		
	URegularTransport::Send(TPT->tptReceive, pBuffer, nBufferSize);
	UMemory::Dispose((TPtr)pBuffer);
	
	return true;
}

static bool _SendHttpResponseGetHeader(TRegularTransport inTpt)
{
	void *pBuffer = nil;
	Uint32 nBufferSize = 0;
	
	if (!_AddField_General(&pBuffer, nBufferSize, "\pHTTP/1.0 ", "\p200 OK") || !_AddField_General(&pBuffer, nBufferSize, "\pProxy-Connection: ", "\pKeep-Alive") ||
	    !_AddField_General(&pBuffer, nBufferSize, "\pContent-Length: ", "\p999999999") || !_AddField_General(&pBuffer, nBufferSize, "\pContent-Type: ", "\photline/protocol") ||
	    !_AddField_End(&pBuffer, nBufferSize))
	{
		UMemory::Dispose((TPtr)pBuffer);
		return false;
	}
		
	URegularTransport::Send(inTpt, pBuffer, nBufferSize);
	UMemory::Dispose((TPtr)pBuffer);

	_SendHttpPadding(inTpt, nBufferSize - sizeof(SData));
	_SendHttpPadding(inTpt);
	_SendHttpPadding(inTpt);

	return true;
}

static bool _SendHttpPadding(TRegularTransport inTpt, Uint32 inDataSize)
{
	if (inDataSize >= _gPackageSize)
		return true;
	
	Uint32 nPaddingSize = _gPackageSize - inDataSize;		
	if (inDataSize)
	{
		if (nPaddingSize <= sizeof(SData)) 
			return true;

		nPaddingSize -= sizeof(SData);
	}
	
	if (nPaddingSize <= sizeof(SData)) 
		return true;

	void *pPadding = UMemory::New(nPaddingSize);
	if (!pPadding)
		return false;

	((SData *)pPadding)->nDataCode = TB((Uint32)http_Padding);
	((SData *)pPadding)->nDataSize = TB(nPaddingSize - sizeof(SData));
	
	URegularTransport::Send(inTpt, pPadding, nPaddingSize);
	UMemory::Dispose((TPtr)pPadding);
	
	return true;
}

static void _SendDisconnectPost(TRegularTransport inTpt)
{
	URegularTransport::SetMessageHandler(inTpt, nil);
	_SendHttpResponsePostHeader(inTpt);

	_gConnectionCloseList.AddItem(inTpt);
	URegularTransport::StartDisconnect(inTpt);
}

static void _SendDisconnectGet(TRegularTransport inTpt)
{
	SData stData;
	stData.nDataCode = 0;
	stData.nDataSize = 0;
	
	URegularTransport::SetMessageHandler(inTpt, nil);
	URegularTransport::Send(inTpt, &stData, sizeof(SData));

	_gConnectionCloseList.AddItem(inTpt);
	URegularTransport::StartDisconnect(inTpt);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

static bool _ReceiveHttpHeader(SConnectionInfo *inConnectionInfo)
{
	Uint32 nReceiveSize = URegularTransport::GetReceiveSize(inConnectionInfo->tptReg);
	if (!nReceiveSize)
		return false;
	
	if (inConnectionInfo->nReceiveHeader == http_ReceiveHeader)
	{
		SGUID stNilGuid;
		ClearStruct(stNilGuid);
				
		bool bConnectionType = false;
		if (inConnectionInfo->stConnectionGuid == stNilGuid)
			bConnectionType = true;
		
		if (bConnectionType && nReceiveSize < 42)
		{
			URegularTransport::NotifyReadyToReceive(inConnectionInfo->tptReg, 42);
			return false;
		}
	
		if (nReceiveSize > _gPackageSize/2)			
			nReceiveSize = _gPackageSize/2;
	
		void *pBuffer = UMemory::NewClear(nReceiveSize);
		if (!pBuffer)
			return false;

		nReceiveSize = URegularTransport::Receive(inConnectionInfo->tptReg, pBuffer, nReceiveSize);
		if (!nReceiveSize)
		{	
			UMemory::Dispose((TPtr)pBuffer);
			return false;
		}
		
		if (bConnectionType)
		{
			if (!UMemory::Compare(pBuffer, "GET", 3))
				inConnectionInfo->bSendReceive = true;
			else if (!UMemory::Compare(pBuffer, "POST", 4))
				inConnectionInfo->bSendReceive = false;
			else
			{
				UMemory::Dispose((TPtr)pBuffer);
				return false;
			}
			
			Uint8 *pGuid = UMemory::Search("HTTP/1.0", 8, pBuffer, nReceiveSize);
			if (!pGuid || pGuid - pBuffer < 37)
			{
				UMemory::Dispose((TPtr)pBuffer);
				return false;
			}
		
			pGuid -= 37;
			UGUID::FromText(inConnectionInfo->stConnectionGuid, pGuid, 36);
			UGUID::Unflatten(inConnectionInfo->stConnectionGuid, &inConnectionInfo->stConnectionGuid);
		}

		Uint32 nHeaderSize;
		if (!_GetHeaderSize(pBuffer, nReceiveSize, nHeaderSize))
		{
			UMemory::Dispose((TPtr)pBuffer);
			return false;
		}
		
		nReceiveSize -= nHeaderSize;
		inConnectionInfo->nReceiveHeader = http_ReceivePadding;
		
		if (inConnectionInfo->bSendReceive)
		{
			UMemory::Dispose((TPtr)pBuffer);
			_SendHttpResponseGetHeader(inConnectionInfo->tptReg);
			
			return true;
		}
			
		if (!nReceiveSize)
		{
			UMemory::Dispose((TPtr)pBuffer);
			
			URegularTransport::NotifyReadyToReceive(inConnectionInfo->tptReg, sizeof(SData));
			return false;
		}
								
		Uint32 nReceiveData = nReceiveSize > sizeof(SData) ? sizeof(SData) : nReceiveSize;
		UMemory::Copy(&inConnectionInfo->stReceiveData, (Uint8 *)pBuffer + nHeaderSize, nReceiveData);
		UMemory::Dispose((TPtr)pBuffer);
					
		inConnectionInfo->nOffsetData = nReceiveData;
		if (inConnectionInfo->nOffsetData != sizeof(SData))
			return false;
				
		inConnectionInfo->stReceiveData.nDataCode = TB(inConnectionInfo->stReceiveData.nDataCode);
		inConnectionInfo->stReceiveData.nDataSize = TB(inConnectionInfo->stReceiveData.nDataSize);
		inConnectionInfo->nOffsetData = 0;

		if (inConnectionInfo->stReceiveData.nDataCode != http_Padding)
			return false;
			
		nReceiveSize -=	nReceiveData;
		if (nReceiveSize < inConnectionInfo->stReceiveData.nDataSize)
		{
			inConnectionInfo->stReceiveData.nDataSize -= nReceiveSize;
			return _ReceiveHttpHeader(inConnectionInfo);
		}
		
		inConnectionInfo->stReceiveData.nDataSize = 0;
		return true;
	}

	if (inConnectionInfo->nReceiveHeader == http_ReceivePadding)
	{
		if (inConnectionInfo->bSendReceive)
			return true;
		
		if (inConnectionInfo->stReceiveData.nDataSize && !inConnectionInfo->nOffsetData)
			return _ReceiveHttpPadding(inConnectionInfo, nReceiveSize);
	
		if (nReceiveSize < sizeof(SData) - inConnectionInfo->nOffsetData)
		{
			URegularTransport::NotifyReadyToReceive(inConnectionInfo->tptReg, sizeof(SData) - inConnectionInfo->nOffsetData);
			return false;
		}
			
		Uint32 nReceiveData = URegularTransport::Receive(inConnectionInfo->tptReg, (Uint8 *)&inConnectionInfo->stReceiveData + inConnectionInfo->nOffsetData, sizeof(SData) - inConnectionInfo->nOffsetData);
		nReceiveSize -= nReceiveData;
	
		inConnectionInfo->nOffsetData += nReceiveData;
		if (inConnectionInfo->nOffsetData != sizeof(SData))
			return false;
		
		inConnectionInfo->stReceiveData.nDataCode = TB(inConnectionInfo->stReceiveData.nDataCode);
		inConnectionInfo->stReceiveData.nDataSize = TB(inConnectionInfo->stReceiveData.nDataSize);
		inConnectionInfo->nOffsetData = 0;

		if (inConnectionInfo->stReceiveData.nDataCode != http_Padding)
			return false;
			
		return _ReceiveHttpPadding(inConnectionInfo, nReceiveSize);
	}
	
	return true;
}

static bool _ReceiveHttpHeader(THttpTransport inTpt)
{
	Uint32 nReceiveSize = URegularTransport::GetReceiveSize(TPT->tptReceive);
	if (!nReceiveSize)
		return false;
		
	if (TPT->nReceiveHeader == http_ReceiveHeader)
	{
		if (nReceiveSize > _gPackageSize/2)			
			nReceiveSize = _gPackageSize/2;
	
		void *pBuffer = UMemory::NewClear(nReceiveSize);
		if (!pBuffer)
			return false;
				
		nReceiveSize = URegularTransport::Receive(TPT->tptReceive, pBuffer, nReceiveSize);
		if (!nReceiveSize)
		{
			UMemory::Dispose((TPtr)pBuffer);
			return false;
		}
	
		Uint32 nHeaderSize = 0;
		if (!_GetHeaderSize(pBuffer, nReceiveSize, nHeaderSize))
		{
			UMemory::Dispose((TPtr)pBuffer);
			return false;
		}
		
		nReceiveSize -= nHeaderSize;
		TPT->nReceiveHeader = http_ReceivePadding;
		
		if (!nReceiveSize)
		{
			UMemory::Dispose((TPtr)pBuffer);
			
			URegularTransport::NotifyReadyToReceive(TPT->tptReceive, sizeof(SData));
			return false;
		}

		Uint32 nReceiveData = nReceiveSize > sizeof(SData) ? sizeof(SData) : nReceiveSize;
		UMemory::Copy(&TPT->stReceiveData, (Uint8 *)pBuffer + nHeaderSize, nReceiveData);
		UMemory::Dispose((TPtr)pBuffer);
					
		TPT->nOffsetData = nReceiveData;
		if (TPT->nOffsetData != sizeof(SData))
			return false;

		TPT->stReceiveData.nDataCode = TB(TPT->stReceiveData.nDataCode);
		TPT->stReceiveData.nDataSize = TB(TPT->stReceiveData.nDataSize);
		TPT->nOffsetData = 0;

		if (TPT->stReceiveData.nDataCode != http_Padding)
			return false;

		nReceiveSize -=	nReceiveData;		
		if (nReceiveSize < TPT->stReceiveData.nDataSize)
		{
			TPT->stReceiveData.nDataSize -= nReceiveSize;
			return _ReceiveHttpHeader(inTpt);
		}
		
		TPT->stReceiveData.nDataSize = 0;
		return true;
	}
	
	if (TPT->nReceiveHeader == http_ReceivePadding)
	{
		if (TPT->stReceiveData.nDataSize && !TPT->nOffsetData)
			return _ReceiveHttpPadding(inTpt, nReceiveSize);
	
		if (nReceiveSize < sizeof(SData) - TPT->nOffsetData)
		{
			URegularTransport::NotifyReadyToReceive(TPT->tptReceive, sizeof(SData) - TPT->nOffsetData);
			return false;
		}
		
		Uint32 nReceiveData = URegularTransport::Receive(TPT->tptReceive, (Uint8 *)&TPT->stReceiveData + TPT->nOffsetData, sizeof(SData) - TPT->nOffsetData);
		nReceiveSize -= nReceiveData;
	
		TPT->nOffsetData += nReceiveData;
		if (TPT->nOffsetData != sizeof(SData))
			return false;
				
		TPT->stReceiveData.nDataCode = TB(TPT->stReceiveData.nDataCode);
		TPT->stReceiveData.nDataSize = TB(TPT->stReceiveData.nDataSize);
		TPT->nOffsetData = 0;

		if (TPT->stReceiveData.nDataCode != http_Padding)
			return false;
			
		return _ReceiveHttpPadding(inTpt, nReceiveSize);
	}
	
	return true;
}

static bool _ReceiveHttpData(THttpTransport inTpt)
{
	Uint32 nReceiveSize = URegularTransport::GetReceiveSize(TPT->tptReceive);
	if (!nReceiveSize)
		return false;
	
	if (TPT->stReceiveData.nDataSize && !TPT->nOffsetData)
	{
		if (TPT->stReceiveData.nDataCode == http_Data)
			return true;
		
		if (_ReceiveHttpPadding(inTpt, nReceiveSize))
			return _ReceiveHttpData(inTpt);
			
		return false;
	}
	
	if (nReceiveSize < sizeof(SData) - TPT->nOffsetData)
	{
		URegularTransport::NotifyReadyToReceive(TPT->tptReceive, sizeof(SData) - TPT->nOffsetData);
		return false;
	}
	
	Uint32 nReceiveData = URegularTransport::Receive(TPT->tptReceive, (Uint8 *)&TPT->stReceiveData + TPT->nOffsetData, sizeof(SData) - TPT->nOffsetData);
	nReceiveSize -= nReceiveData;
	
	TPT->nOffsetData += nReceiveData;
	if (TPT->nOffsetData != sizeof(SData))
		return false;
	
	TPT->stReceiveData.nDataCode = TB(TPT->stReceiveData.nDataCode);
	TPT->stReceiveData.nDataSize = TB(TPT->stReceiveData.nDataSize);
	TPT->nOffsetData = 0;

	if (TPT->stReceiveData.nDataCode == http_Data)
	{
		if (TPT->nNotifyReceivedSize)
		{
			if (TPT->nNotifyReceivedSize <= TPT->stReceiveData.nDataSize)
				URegularTransport::NotifyReadyToReceive(TPT->tptReceive, TPT->nNotifyReceivedSize);
	
			TPT->nNotifyReceivedSize = 0;
		}
		
		return true;
	}
	
	if (TPT->stReceiveData.nDataCode == http_Padding)
	{
		if (_ReceiveHttpPadding(inTpt, nReceiveSize))
			return _ReceiveHttpData(inTpt);
			
		return false;
	}

	_Disconnect(inTpt);
	return false;
}

static bool _ReceiveHttpPadding(SConnectionInfo *inConnectionInfo, Uint32 inReceiveSize)
{
	Uint32 nReceiveSize = inReceiveSize > inConnectionInfo->stReceiveData.nDataSize ? inConnectionInfo->stReceiveData.nDataSize : inReceiveSize;
	if (!nReceiveSize)
		return false;
		
	void *pPadding = UMemory::NewClear(nReceiveSize);
	if (!pPadding)
		return false;
	
	nReceiveSize = URegularTransport::Receive(inConnectionInfo->tptReg, pPadding, nReceiveSize);
	inConnectionInfo->stReceiveData.nDataSize -= nReceiveSize;
	
	UMemory::Dispose((TPtr)pPadding);
	
	if (inConnectionInfo->stReceiveData.nDataSize)
		return false;

	return true;
}

static bool _ReceiveHttpPadding(THttpTransport inTpt, Uint32 inReceiveSize)
{
	Uint32 nReceiveSize = inReceiveSize > TPT->stReceiveData.nDataSize ? TPT->stReceiveData.nDataSize : inReceiveSize;
	if (!nReceiveSize)
		return false;
		
	void *pPadding = UMemory::NewClear(nReceiveSize);
	if (!pPadding)
		return false;
	
	nReceiveSize = URegularTransport::Receive(TPT->tptReceive, pPadding, nReceiveSize);
	TPT->stReceiveData.nDataSize -= nReceiveSize;
	
	UMemory::Dispose((TPtr)pPadding);
	
	if (TPT->stReceiveData.nDataSize)
		return false;

	return true;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

static bool _AddField_General(void **ioBuffer, Uint32& ioBufferSize, const Uint8 *inField, const Uint8 *inData, const Uint8 *inExtraData)
{
	Uint32 nOffset = ioBufferSize;
	ioBufferSize += inField[0] + inData[0] + 2;

	if (inExtraData)
		ioBufferSize += inExtraData[0];
	
	if (*ioBuffer == nil)
		*ioBuffer = UMemory::NewClear(ioBufferSize);
	else
		*ioBuffer = UMemory::Reallocate((TPtr)*ioBuffer, ioBufferSize);
		
	if (!*ioBuffer)
		return false;
		
	nOffset += UMemory::Copy((Uint8 *)*ioBuffer + nOffset, inField + 1, inField[0]);
	nOffset += UMemory::Copy((Uint8 *)*ioBuffer + nOffset, inData + 1, inData[0]);
	
	if (inExtraData)
		nOffset += UMemory::Copy((Uint8 *)*ioBuffer + nOffset, inExtraData + 1, inExtraData[0]);
	
	*((Uint8 *)*ioBuffer + nOffset) = '\r';
	*((Uint8 *)*ioBuffer + nOffset + 1) = '\n';

	return true;
}

static bool _AddField_End(void **ioBuffer, Uint32& ioBufferSize)
{
	if (!*ioBuffer || !ioBufferSize)
		return false;
		
	ioBufferSize += 2;
	*ioBuffer = UMemory::Reallocate((TPtr)*ioBuffer, ioBufferSize);
	
	if (!*ioBuffer)
		return false;

	*((Uint8 *)*ioBuffer + ioBufferSize -2) = '\r';
	*((Uint8 *)*ioBuffer + ioBufferSize -1) = '\n';
	
	return true;
}

static bool _GetHeaderSize(void *inBuffer, Uint32 inBufferSize, Uint32& outHeaderSize)
{
	outHeaderSize = 0;
		
	while (inBufferSize > outHeaderSize)
	{
		Uint32 nDataSize = inBufferSize - outHeaderSize;
		Uint8 *pFieldBegin = (Uint8 *)inBuffer + outHeaderSize;
		Uint8 *pFieldEnd = UMemory::Search("\r\n", 2, pFieldBegin, (nDataSize > 256 ? 256 : nDataSize));

		if (!pFieldEnd || pFieldEnd - pFieldBegin + 4 > nDataSize)
		{
			outHeaderSize = inBufferSize;
			return false;
		}
		
		pFieldEnd += 2;
		outHeaderSize += pFieldEnd - pFieldBegin;
	
		if (!UMemory::Compare(pFieldEnd, "\r\n", 2))
		{		
			outHeaderSize += 2;
			return true;
		}	
	}
	
	return false;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

static void _AddSendDataList(THttpTransport inTpt, Uint32 inDataSize)
{
	SData *pData = (SData *)UMemory::New(sizeof(SData));
	
	pData->nDataCode = TB((Uint32)http_Data);
	pData->nDataSize = TB(inDataSize);

	SSendData *pSendData = (SSendData *)UMemory::New(sizeof(SSendData));

	pSendData->pSendData = pData;
	pSendData->nDataSize = sizeof(SData);
	pSendData->bDataBuffer = true;

	TPT->stSendDataList.AddItem(pSendData);
}

static bool _ClearSendDataList(THttpTransport inTpt)
{
	bool bRet = false;
	bool bSendData = false;
	
	if (TPT->tptSend && URegularTransport::IsConnected(TPT->tptSend))
		bSendData = true;

	Uint32 i = 0;
	SSendData *pSendData;

	while (TPT->stSendDataList.GetNext(pSendData, i))
	{
		if (pSendData->bDataBuffer)
		{
			if (bSendData)
			{
				bRet = true;
				UHttpTransport::Send(inTpt, pSendData->pSendData, pSendData->nDataSize);
			}
			
			UMemory::Dispose((TPtr)pSendData->pSendData);
		}
		else if (bSendData)
		{
			bRet = true;
			UHttpTransport::SendBuffer(inTpt, pSendData->pSendData);
		}
		else
		{
			URegularTransport::DisposeBuffer(pSendData->pSendData);
		}
		
		UMemory::Dispose((TPtr)pSendData);
	}
	
	TPT->stSendDataList.Clear();
	
	return bRet;
}

static Uint32 _GetUnsentSize(THttpTransport inTpt)
{
	Uint32 nUnsentSize = 0;
	
	Uint32 i = 0;
	SSendData *pSendData;

	while (TPT->stSendDataList.GetNext(pSendData, i))
	{
		nUnsentSize += pSendData->nDataSize;
	}

	return nUnsentSize;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

static void _CleanConnectionOpenList()
{
	Uint32 nCurrentTime = UDateTime::GetMilliseconds();

	Uint32 i = 0;
	SConnectionInfo *pConnectionInfo;
	
	while (_gConnectionOpenList.GetNext(pConnectionInfo, i))
	{
		if (nCurrentTime - pConnectionInfo->nConnectionTime > _gConnectionTime)
		{
			_gConnectionOpenList.RemoveItem(pConnectionInfo);
			
			URegularTransport::Dispose(pConnectionInfo->tptReg);
			UMemory::Dispose((TPtr)pConnectionInfo);

			i--;
		}
	}
}

static void _ClearConnectionOpenList()
{
	Uint32 i = 0;
	SConnectionInfo *pConnectionInfo;
	
	while (_gConnectionOpenList.GetNext(pConnectionInfo, i))
	{
		URegularTransport::Dispose(pConnectionInfo->tptReg);
		UMemory::Dispose((TPtr)pConnectionInfo);
	}
	
	_gConnectionOpenList.Clear();
}

static void _CleanConnectionCloseList()
{
	Uint32 i = 0;
	TRegularTransport pRegularTransport;
	
	while (_gConnectionCloseList.GetNext(pRegularTransport, i))
	{
		if (!URegularTransport::IsConnected(pRegularTransport))
		{
			_gConnectionCloseList.RemoveItem(pRegularTransport);
			URegularTransport::Dispose(pRegularTransport);

			i--;
		}
	}	
}

static void _ClearConnectionCloseList()
{
	Uint32 nStartTime = UDateTime::GetMilliseconds();

	Uint32 i = 0;
	TRegularTransport pRegularTransport;
	
	while (_gConnectionCloseList.GetNext(pRegularTransport, i))
	{
		while (URegularTransport::GetUnsentSize(pRegularTransport) && UDateTime::GetMilliseconds() - nStartTime < 2000) // 2 sec
			UApplication::Process();	
		
		URegularTransport::Dispose(pRegularTransport);
	}

	_gConnectionCloseList.Clear();
}
