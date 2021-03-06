#if MACINTOSH

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "URegularTransport.h"
#include "UProgramCleanup.h"

// trick <OpenTransport.h> into thinking we included <stddef.h>
#define __STDDEF__
typedef unsigned long size_t;

#include <OpenTransport.h>
#include <OpenTptInternet.h>
#include <OpenTptClient.h>
#include <Gestalt.h>
#include <Processes.h>
#include <Threads.h>
#include <Timer.h>
#include <InternetConfig.h>


#if !TARGET_API_MAC_CARBON
	#pragma import on
	extern "C" OSStatus OTSetMemoryLimits(size_t size, size_t max);
	#pragma import off
#endif

#ifndef FLUSHR
#define FLUSHR			0x1 			/* Flush the read queue */
#define FLUSHW			0x2 			/* Flush the write queue */
#define FLUSHRW 		(FLUSHW|FLUSHR) /* Flush both */
#define FLUSHBAND		0x40			/* Flush a particular band */
#define I_FLUSH			16645
#endif

/*
 * Structures and Types
 */

#pragma options align=packed
struct SSendBuf {
	SSendBuf *next;
	Handle h;
	Uint32 offset;		// for when part of the buffer was sent straight away
	Uint32 dataSize;
	Uint8 data[];
};
#pragma options align=reset

struct SFirewallInfo {
	TBind ret;
	InetAddress ipaddr;			// address of the host to connect to via firewall
	Uint8 firewallAddr[256];	// in OT format
	Uint32 firewallAddrSize;
	Uint32 timeout;				// in milliseconds
	Uint16 port;
};

struct SReceivedData
{
	void *pData;
	Uint32 nDataSize;
	Uint32 nReceivedSize;
	Uint32 nReceivedOffset;
};

struct SRegularTransport {
	SRegularTransport *next;
	EndpointRef ep;
	long connectTimer;
	Uint32 configProtocol, configOptions;
	TTransportMonitorProc dataMonitorProc;
	TMessageProc msgProc;
	void *msgProcContext;
	void *msgObject;
	OTResult bindResult, acceptResult, connectResult, getProtAddrResult;
	InetHost remoteIPAddress;
	OTSequence connectSeqVal;
	SSendBuf *sendQueue;
	SIPAddress localIPAddress;
	SFirewallInfo *firewallInfo;
	SReceivedData receivedData;
	Uint32 notifyReceivedSize;
	Uint8 isBound;
	Uint8 postFlags;
	Uint8 flags;
};

// bit numbers for flags
enum {
	// post flags
	flag_PostConnectRequest		= 0,
	flag_PostDataArrived		= 1,
	flag_PostConnectionClosed	= 2,
	flag_PostConnectEstab		= 3,
	flag_PostConnectRefuse		= 4,
	flag_PostReadyToSend		= 5,

	// misc flags
	flag_CheckReadyToSend		= 0,
	flag_OrdDisconWhenNoOutData	= 1,
	flag_CheckFirewallReply		= 2,
	flag_StartConnectCalled		= 3,
	flag_IsConnected			= 4,	// IsConnected status according to user of URegularTransport (not whether really connected)
	
	// global flags
	flag_Process				= 0
};

#define	kNotComplete	max_Int32

#define TPT		((SRegularTransport *)inTpt)

/*
 * Macros
 */

#if DEBUG
	#define FailOTError(id)		_FailOTError(id, __FILE__, __LINE__)
	#define CheckOTError(id)	_CheckOTError(id, __FILE__, __LINE__)
#else
	#define FailOTError(id)		_FailOTError(id)
	#define CheckOTError(id)	_CheckOTError(id)
#endif

/*
 * Function Prototypes
 */

static EndpointRef _TRCreateEndpoint(TRegularTransport inTpt, bool inTilisten = false);
static OTConfiguration *_TRCreateConfig(Uint32 inProtocol, Uint32 inOptions, bool inTilisten = false);
static void _TRBindIPAddress(TRegularTransport inTpt, Uint16 inPort = 0, Uint16 inGlen = 0);
static void _TRBind(SRegularTransport *inTpt, TBind *inReqAddr, TBind *inRetAddr);
static bool _TRNotifyReadyToReceive(TRegularTransport inTpt, Uint32 inReceivedSize, bool inPostMessage = true);
static pascal void _TREventNotifyCallback(void *inContext, OTEventCode inEventCode, OTResult inResult, void *inParam);
static void _TRSendQueued(TRegularTransport inTpt);
static Uint32 _TRAddressToOT(const void *inAddr, Uint32 inAddrSize, void *outAddr, Uint32 inMaxSize);
static Uint32 _TROTToAddress(const void *inAddr, Uint32 inAddrSize, void *outAddr, Uint32 inMaxSize);
static void _TRProcess();
static pascal void _TRConnectTimer(void *inArg);
static void _TRDisposeUnsent(TRegularTransport inTpt);
static void _ConvertOTError(Int16 inOTError, Int16& outType, Int16& outID);
static void _FailOTError(Int16 inOTError, const Int8 *inFile, Uint32 inLine);
static void _FailOTError(Int16 inMacError);
inline void _CheckOTError(Int16 inOTError, const Int8 *inFile, Uint32 inLine);
inline void _CheckOTError(Int16 inOTError);

bool GetEthernetAddr(SEthernetAddress& outEthernetAddr);
void _ConvertMacError(Int16 inMacError, Int16& outType, Int16& outID);
void _WakeupApp();

#if !TARGET_API_MAC_CARBON
	enum {
		uppExitToShellProcInfo = kPascalStackBased
	};

	static pascal void _TRExitToShell();

	typedef UniversalProcPtr ExitToShellUPP;
	static ExitToShellUPP _APSaveExitToShell = nil;

	#define CallExitToShellProc(proc)	::CallUniversalProc((UniversalProcPtr) (proc), uppExitToShellProcInfo)
	static RoutineDescriptor _TRExitToShellRD = BUILD_ROUTINE_DESCRIPTOR(uppExitToShellProcInfo, _TRExitToShell);
#endif

#if TARGET_API_MAC_CARBON
	static OTNotifyUPP 	_TREventNotifyCallbackUPP = ::NewOTNotifyUPP(_TREventNotifyCallback);
	static OTProcessUPP _TRConnectTimerUPP = ::NewOTProcessUPP(_TRConnectTimer);
#else
	#define _TREventNotifyCallbackUPP	_TREventNotifyCallback
	#define _TRConnectTimerUPP			_TRConnectTimer
#endif

/*
 * Global Variables
 */

static struct {
	OTLIFO killBufList;
	Int32 gestaltResult, otVersion;
	SRegularTransport *firsTRegularTransporter;
	Uint16 isInitted			: 1;
	Uint16 gestaltInitted		: 1;
	Uint8 flags;
} _TRData = {{0},0,0,0,0,0,0};

extern void (*_APTransportProcess)();

Uint32 _TRSendBufSize = 51200;	// 50K


/* -------------------------------------------------------------------------- */

bool URegularTransport::IsAvailable()
{
	// get gestalt info on OT
	if (!_TRData.gestaltInitted)
	{
		::Gestalt('otan', &_TRData.gestaltResult);
		::Gestalt('otvr', &_TRData.otVersion);
		
		_TRData.gestaltInitted = true;
	}
	
	// need OT 1.1.1 or better
	return (_TRData.gestaltResult & gestaltOpenTptPresentMask) && (_TRData.otVersion >= 0x01110000);
}

bool URegularTransport::HasTCP()
{
	if (!IsAvailable())
		return false;
	
	return (_TRData.gestaltResult & gestaltOpenTptTCPPresentMask) != 0;
}

/*
 * _Deinit() deinitializes URegularTransport.  _Deinit() is called automatically on
 * program exit if you called Init().
 */
static void _Deinit()
{
	if (_TRData.isInitted)
	{
		// kill all transporters
		while (_TRData.firsTRegularTransporter)
			URegularTransport::Dispose((TRegularTransport)_TRData.firsTRegularTransporter);
		
		_APTransportProcess = nil;
		
		// close open transport
		_TRData.isInitted = false;
	
	#if TARGET_API_MAC_CARBON
		::CloseOpenTransportInContext(NULL);
	#else
		::CloseOpenTransport();
	#endif
	}
}

/*
 * IMPORTANT: If using URegularTransport with threads, UThread::Init() must be called
 * BEFORE URegularTransport::Init().
 */
void URegularTransport::Init()
{	
	if (!_TRData.isInitted)
	{
		// ensure that OT is available
		if (!IsAvailable())
			Fail(errorType_Transport, transError_NotAvailable);
				
		// initialize OT
	#if TARGET_API_MAC_CARBON
		CheckOTError(::InitOpenTransportInContext(kInitOTForApplicationMask, NULL));	//??
	#else
		CheckOTError(::InitOpenTransport());
	#endif
	
	#if !TARGET_API_MAC_CARBON
		if (OTSetMemoryLimits != nil)
			OTSetMemoryLimits(131072, 0);
	#endif
	
		try
		{
			// make Deinit() get called automatically at program exit
			UProgramCleanup::InstallSystem(_Deinit);
		}
		catch(...)
		{
			// deinit OT
		#if TARGET_API_MAC_CARBON
			::CloseOpenTransportInContext(NULL);
		#else
			::CloseOpenTransport();
		#endif
		
			throw;
		}
		
		_APTransportProcess = _TRProcess;
			
		// Carbon does not support Path Manager
	#if !TARGET_API_MAC_CARBON
		if (_APSaveExitToShell == nil)
		{
			// patch ExitToShell
			_APSaveExitToShell = (ExitToShellUPP)::NGetTrapAddress(_ExitToShell, (_ExitToShell & 0x0800) ? ToolTrap : OSTrap);
			::NSetTrapAddress((UniversalProcPtr)&_TRExitToShellRD, _ExitToShell, (_ExitToShell & 0x0800) ? ToolTrap : OSTrap);
		}
	#endif
		
		// URegularTransport has successfully initialized
		_TRData.isInitted = true;		
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

TRegularTransport URegularTransport::New(Uint32 inProtocol, Uint32 inOptions)
{
	SRegularTransport *tpt = nil;
	
	Init();
	
	try
	{
		// allocate memory for TRegularTransport
		tpt = (SRegularTransport *)UMemory::NewClear(sizeof(SRegularTransport));
		tpt->configProtocol = inProtocol;
		tpt->configOptions = inOptions;
		
		// create endpoint
		tpt->ep = _TRCreateEndpoint((TRegularTransport)tpt);
	}
	catch(...)
	{
		// clean up
		UMemory::Dispose((TPtr)tpt);
		throw;
	}
	
	// add to transporter list
	tpt->next = _TRData.firsTRegularTransporter;
	_TRData.firsTRegularTransporter = tpt;

	// return transporter reference
	return (TRegularTransport)tpt;
}

// also closes connection if necessary
void URegularTransport::Dispose(TRegularTransport inTpt)
{
	if (inTpt)
	{
		// remove from transporter list
		SRegularTransport *tm = _TRData.firsTRegularTransporter;
		SRegularTransport *ptm = nil;
		
		while (tm)
		{
			if (tm == TPT)
			{
				if (ptm)
					ptm->next = tm->next;
				else
					_TRData.firsTRegularTransporter = tm->next;
				break;
			}
			
			ptm = tm;
			tm = tm->next;
		}
		
		// kill any messages for this transporter
		if (TPT->msgProc)
			UApplication::FlushMessages(TPT->msgProc, TPT->msgProcContext, TPT->msgObject);

		// kill timer
		if (TPT->connectTimer)
			::OTDestroyTimerTask(TPT->connectTimer);
		
		// dispose received data
		if (TPT->receivedData.pData)
			UMemory::Dispose((TPtr)TPT->receivedData.pData);
		
		// dispose unsent data
		_TRDisposeUnsent(inTpt);
		
		// flush outgoing data
		EndpointRef ep = TPT->ep;
		::OTIoctl(ep, I_FLUSH, (void *)FLUSHRW);

		// disconnect and close provider
		::OTSndDisconnect(ep, nil);
		::OTUnbind(ep);
		::OTCloseProvider(ep);		// will not cancel the OTSndDisconnect()

		// release memory
		UMemory::Dispose((TPtr)TPT->firewallInfo);
		UMemory::Dispose((TPtr)inTpt);
	}
}

void URegularTransport::SetDataMonitor(TRegularTransport inTpt, TTransportMonitorProc inProc)
{
	Require(inTpt);
	TPT->dataMonitorProc = inProc;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void URegularTransport::SetMessageHandler(TRegularTransport inTpt, TMessageProc inProc, void *inContext, void *inObject)
{
	Require(inTpt);
	
	TPT->msgProc = inProc;
	TPT->msgProcContext = inContext;
	TPT->msgObject = inObject;
}

void URegularTransport::PostMessage(TRegularTransport inTpt, Uint32 inMsg, const void *inData, Uint32 inDataSize, Int16 inPriority)
{
	Require(inTpt);
	if (TPT->msgProc)
		UApplication::PostMessage(inMsg, inData, inDataSize, inPriority, TPT->msgProc, TPT->msgProcContext, TPT->msgObject);
}

void URegularTransport::ReplaceMessage(TRegularTransport inTpt, Uint32 inMsg, const void *inData, Uint32 inDataSize, Int16 inPriority)
{
	Require(inTpt);
	if (TPT->msgProc)
		UApplication::ReplaceMessage(inMsg, inData, inDataSize, inPriority, TPT->msgProc, TPT->msgProcContext, TPT->msgObject);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

SIPAddress URegularTransport::GetDefaultIPAddress()
{
	SIPAddress defaultIPAddress;
	defaultIPAddress.un_IP.stDW_IP.nDW_IP = 0;

	InetInterfaceInfo info;
	CheckOTError(::OTInetGetInterfaceInfo(&info, kDefaultInetInterface));
	
	Uint8 psAddress[32];
	::OTInetHostToString(info.fAddress, (char *)psAddress + 1);

	Uint32 nSize = 0;
	Uint8 *pAddress = psAddress + 1;
	while (*pAddress++) ++nSize;
	psAddress[0] = nSize;

	UTransport::TextToIPAddress(psAddress, defaultIPAddress);

	return defaultIPAddress;
}

void URegularTransport::SetIPAddress(TRegularTransport inTpt, SIPAddress inIPAddress)
{
	Require(inTpt);

	if (TPT->localIPAddress.un_IP.stDW_IP.nDW_IP != inIPAddress.un_IP.stDW_IP.nDW_IP)
	{
		TPT->localIPAddress = inIPAddress;
		TPT->isBound = false;
	}
}

SIPAddress URegularTransport::GetIPAddress(TRegularTransport inTpt)
{
	Require(inTpt);
	
	// check the saved address
	if (!TPT->localIPAddress.IsNull())
		return TPT->localIPAddress;

	// get the address
	Uint8 psAddress[32];
	psAddress[0] = GetLocalAddressText(inTpt, psAddress + 1, sizeof(psAddress) - 1);
	
	SIPAddress localIPAddress;
	UTransport::TextToIPAddress(psAddress, localIPAddress);

	// save the address
	if (TPT->isBound)
		TPT->localIPAddress.un_IP.stDW_IP.nDW_IP = localIPAddress.un_IP.stDW_IP.nDW_IP;
	
	return localIPAddress;
}

Uint32 URegularTransport::GetRemoteAddressText(TRegularTransport inTpt, void *outAddr, Uint32 inMaxSize)
{
	Require(inTpt && outAddr && inMaxSize > 15);
	
	// check if we don't have the remote address and thus need to look it up
	if (TPT->remoteIPAddress == 0 && ::OTAtomicTestBit(&TPT->flags, flag_StartConnectCalled) && (TPT->getProtAddrResult != kNotComplete))
	{
		Uint8 buf[256];
		TBind peerAddr = { { sizeof(buf), 0, buf }, 0 };

		// we're going to attempt to lookup the peer's address, which has to be done asyncronously
		TPT->getProtAddrResult = kNotComplete;
		if (::OTGetProtAddress(TPT->ep, nil, &peerAddr))
			TPT->getProtAddrResult = 0;		// an error occured, abort the operation
		else
		{
			// wait until the GetProtAddress operation completes
			while (TPT->getProtAddrResult == kNotComplete) {}
			
			// remember the address (if we got it that is...)
			if ((peerAddr.addr.len >= sizeof(InetAddress)) && (*(OTAddressType *)buf == AF_INET))
				TPT->remoteIPAddress = ((InetAddress *)buf)->fHost;
		}
	}
	
	// output the address text
	if (TPT->remoteIPAddress)
	{
		::OTInetHostToString(TPT->remoteIPAddress, (char *)outAddr);
		
		Uint32 nSize = 0;
		Uint8 *pAddr = (Uint8 *)outAddr;
		while (*pAddr++) ++nSize;
		
		return nSize;
	}
	
	// couldn't get the address
	return 0;
}

Uint32 URegularTransport::GetLocalAddressText(TRegularTransport inTpt, void *outAddr, Uint32 inMaxSize)
{
	Require(inTpt && inMaxSize > 15);

	// check the saved address
	if (TPT->isBound && !TPT->localIPAddress.IsNull())
		return UText::Format(outAddr, inMaxSize, "%d.%d.%d.%d", TPT->localIPAddress.un_IP.stB_IP.nB_IP1, TPT->localIPAddress.un_IP.stB_IP.nB_IP2, TPT->localIPAddress.un_IP.stB_IP.nB_IP3, TPT->localIPAddress.un_IP.stB_IP.nB_IP4);

	Uint8 buf[256];
	((InetAddress *)buf)->fHost = 0;
	
	// check if the endpoint is connected
	if (::OTGetEndpointState(TPT->ep) == T_DATAXFER)
	{
		TBind localAddr = { { sizeof(buf), 0, buf }, 0 };

		// this shouldn't ever happen but just in case...
		if (TPT->getProtAddrResult == kNotComplete)
		{
			DebugBreak("URegularTransport - already getting local address!");
			Fail(errorType_Misc, error_Protocol);
		}

		TPT->getProtAddrResult = kNotComplete;
		OSStatus err = ::OTGetProtAddress(TPT->ep, &localAddr, nil);
		if (err)
		{
			// an error occured, abort the operation
			TPT->getProtAddrResult = 0;
			FailOTError(err);
		}
		else
		{
			// wait until the GetProtAddress operation completes
			while (TPT->getProtAddrResult == kNotComplete) {}
		}
	}
		
	// check the address
	if (((InetAddress *)buf)->fHost)
	{
		::OTInetHostToString(((InetAddress *)buf)->fHost, (char *)outAddr);
		
		// save the address
		if (TPT->isBound)
			TPT->localIPAddress.un_IP.stDW_IP.nDW_IP = ((InetAddress *)buf)->fHost;
	}
	else
	{
		// grab the default address
		InetInterfaceInfo info;
		CheckOTError(::OTInetGetInterfaceInfo(&info, kDefaultInetInterface));
	
		::OTInetHostToString(info.fAddress, (char *)outAddr);

		// save the address
		if (TPT->isBound)
			TPT->localIPAddress.un_IP.stDW_IP.nDW_IP = info.fAddress;
	}
	
	Uint32 nSize = 0;
	Uint8 *pAddr = (Uint8 *)outAddr;
	while (*pAddr++) ++nSize;
	
	return nSize;
}

Uint32 URegularTransport::GetLocalAddress(TRegularTransport inTpt, void *outAddr, Uint32 inMaxSize)
{
	Require(inTpt && inMaxSize > 15);

	Uint8 buf[256];
	((InetAddress *)buf)->fPort = 0;
	((InetAddress *)buf)->fHost = 0;
	
	// check if the endpoint is connected
	if (::OTGetEndpointState(TPT->ep) == T_DATAXFER)
	{
		TBind localAddr = { { sizeof(buf), 0, buf }, 0 };

		// this shouldn't ever happen but just in case...
		if (TPT->getProtAddrResult == kNotComplete)
		{
			DebugBreak("URegularTransport - already getting local address!");
			Fail(errorType_Misc, error_Protocol);
		}

		TPT->getProtAddrResult = kNotComplete;
		OSStatus err = ::OTGetProtAddress(TPT->ep, &localAddr, nil);
		if (err)
		{
			// an error occured, abort the operation
			TPT->getProtAddrResult = 0;
			FailOTError(err);
		}
		else
		{
			// wait until the GetProtAddress operation completes
			while (TPT->getProtAddrResult == kNotComplete) {}
		}
	}
	
	SInternetAddress *oa = (SInternetAddress *)outAddr;
	
	oa->type = kInternetAddressType;
	oa->port = ((InetAddress *)buf)->fPort;
	*(Uint32 *)oa->host = ((InetAddress *)buf)->fHost;
	((Uint32 *)oa->host)[1] = 0;
	
	// check the address
	if (*(Uint32 *)oa->host == 0)
	{
		if (TPT->isBound && !TPT->localIPAddress.IsNull())
			*(Uint32 *)oa->host = TPT->localIPAddress.un_IP.stDW_IP.nDW_IP;
		else
		{
			// grab the default address
			InetInterfaceInfo info;
			if (::OTInetGetInterfaceInfo(&info, kDefaultInetInterface) == 0)
				*(Uint32 *)oa->host = info.fAddress;
		}
	}
	
	// save the address
	if (TPT->isBound && TPT->localIPAddress.IsNull())
		TPT->localIPAddress.un_IP.stDW_IP.nDW_IP = *(Uint32 *)oa->host;

	return sizeof(SInternetAddress);
}

Uint32 URegularTransport::GetProxyServerAddress(Uint16 inType, void *outProxyAddr, Uint32 inMaxSize, const void *inBypassAddr, Uint32 inBypassAddrSize)
{
	Uint8 usekey[256];
	Uint8 key[256];
	
	switch (inType)
	{
		case proxy_HttpAddress: 		
		case proxy_HttpSecureAddress: 	pstrcpy(usekey, kICUseHTTPProxy); pstrcpy(key, kICHTTPProxyHost);break;
		case proxy_FtpAddress: 			pstrcpy(usekey, kICUseFTPProxy); pstrcpy(key, kICFTPProxyHost);break;
		case proxy_GopherAddress: 		pstrcpy(usekey, kICUseGopherProxy); pstrcpy(key, kICGopherProxy);break;
		case proxy_SocksAddress: 		pstrcpy(usekey, kICUseSocks); pstrcpy(key, kICSocksHost);break;
		default: return 0;
	};

	ProcessInfoRec info;
	ProcessSerialNumber psn; 
	psn.highLongOfPSN = psn.lowLongOfPSN = 0;
	FourCharCode signature = '\?\?\?\?';
	
	// find out the signature of the current process
	if (GetCurrentProcess(&psn) == noErr)
	{
		info.processInfoLength = sizeof(info);
		info.processName = nil;
		info.processAppSpec = nil;
		if (GetProcessInformation(&psn, &info) == noErr)
			signature = info.processSignature;
	}

	ICAttr attr;
	ICInstance inst;
	Uint8 bufProxyInfo[512];
	Int32 nProxySize = sizeof(bufProxyInfo);
	
	if (ICStart(&inst, signature) == noErr)
	{
	#if !TARGET_API_MAC_CARBON
		if (ICFindConfigFile(inst, 0, nil) == noErr)
	#endif
		{
			if (ICBegin(inst, icReadOnlyPerm) == noErr)
			{
				Boolean bUseProxy;
				Int32 nUseProxySize = sizeof(Boolean);				
				if (ICGetPref(inst, usekey, &attr, (Int8 *)&bUseProxy, &nUseProxySize) != icPrefNotFoundErr && !bUseProxy)
				{
					ICEnd(inst);
					ICStop(inst);
					return 0;
				}
				
				if (ICGetPref(inst, kICNoProxyDomains, &attr, (Int8 *)bufProxyInfo, &nProxySize) != icPrefNotFoundErr && nProxySize > 2)
				{				
					Uint16 nBypassCount = *((Uint16 *)bufProxyInfo);
					Uint8 *pBypassOffset = bufProxyInfo + 2;
	
					while (nBypassCount--)
					{			
						if (pBypassOffset[0] <= inBypassAddrSize && !UMemory::Compare(inBypassAddr, pBypassOffset + 1, pBypassOffset[0]))
						{
							ICEnd(inst);
							ICStop(inst);
							return 0;
						}
						
						pBypassOffset += pBypassOffset[0] + 1;
						
						if (pBypassOffset >= bufProxyInfo + nProxySize)
							break;
					}
				}
								
				nProxySize = sizeof(bufProxyInfo);
				
				if (ICGetPref(inst, key, &attr, (Int8 *)bufProxyInfo, &nProxySize) != icPrefNotFoundErr && bufProxyInfo[0])
				{
					ICEnd(inst);
					ICStop(inst);
											
					Uint32 nAddrSize = bufProxyInfo[0] > inMaxSize ? inMaxSize : bufProxyInfo[0];
					UMemory::Copy(outProxyAddr, bufProxyInfo + 1, nAddrSize);

					return nAddrSize;
				}
			
				ICEnd(inst);
			}			
		}
		
		ICStop(inst);
	}
	
	return 0;
}

bool URegularTransport::GetEthernetAddress(SEthernetAddress& outEthernetAddr)
{
	if (GetEthernetAddr(outEthernetAddr))
		return true;
	
	outEthernetAddr.SetNull();
	return false;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

bool URegularTransport::IsConnected(TRegularTransport inTpt)
{
	Require(inTpt);
	
	return ::OTAtomicTestBit(&TPT->flags, flag_IsConnected);
}

void URegularTransport::StartConnectThruFirewall(TRegularTransport inTpt, const void *inAddr, Uint32 inAddrSize, const void *inFirewallAddr, Uint32 inFirewallAddrSize, Uint32 inMaxSecs, Uint32 /* inOptions */)
{
	SFirewallInfo *info;
	TBind req;
	Uint8 buf[512];
	
	// sanity checks
	Require(inTpt);
	if (::OTAtomicTestBit(&TPT->flags, flag_StartConnectCalled))
	{
		DebugBreak("URegularTransport - a TRegularTransport can only be connected once");
		Fail(errorType_Misc, error_Protocol);
	}
	
	// bind endpoint if necessary
	if (!TPT->isBound)
		_TRBindIPAddress(inTpt);
	
	// allocate memory to hold firewall info
	info = (SFirewallInfo *)UMemory::New(sizeof(SFirewallInfo));
	
	// fill in the firewall info
	try
	{
		info->timeout = inMaxSecs ? inMaxSecs * 1000 : 30000;
		info->firewallAddrSize = _TRAddressToOT(inFirewallAddr, inFirewallAddrSize, info->firewallAddr, 256);
		
		req.addr.maxlen = req.addr.len = _TRAddressToOT(inAddr, inAddrSize, buf, sizeof(buf));
		req.addr.buf = buf;
		req.qlen = 0;
		
		// determine port number
		if (*(Uint16 *)buf == AF_DNS)
		{
			// extract port number from the DNS string
			Uint8 *p = buf + req.addr.len;
			Uint32 n = req.addr.len - 2;
			for (n++; --n && *--p != ':';) {}
			p++;
			n = (buf + req.addr.len) - p;
			
			Uint16 num = 0;
			while (n--)
			{
				num *= 10;
				num += *p++ - '0';
			}
			info->port = num;
		}
		else
			info->port = ((Uint16 *)buf)[1];
		
		info->ret.addr.maxlen = sizeof(InetAddress);
		info->ret.addr.len = 0;
		info->ret.addr.buf = (Uint8 *)&info->ipaddr;
		info->ret.qlen = 0;
		
		// ret parameter must remain valid until we get the completion message
		TPT->connectResult = kNotComplete;
		TPT->firewallInfo = info;				// must set before calling OTResolveAddress
		CheckOTError(OTResolveAddress(TPT->ep, &req, &info->ret, 10));
	}
	catch(...)
	{
		TPT->firewallInfo = nil;
		UMemory::Dispose((TPtr)info);
		throw;
	}
	
	::OTAtomicSetBit(&TPT->flags, flag_StartConnectCalled);
}

void URegularTransport::StartConnect(TRegularTransport inTpt, const void *inAddr, Uint32 inAddrSize, Uint32 inMaxSecs)
{
	Uint8 otaddr[512];
	Uint32 otaddrSize;
	TCall call;
	OSStatus err;
	
	// sanity checks
	Require(inTpt);
	if (::OTAtomicTestBit(&TPT->flags, flag_StartConnectCalled))
	{
		DebugBreak("URegularTransport - a TRegularTransport can only be connected once");
		Fail(errorType_Misc, error_Protocol);
	}

	// don't even THINK about connecting if there's less than 8K available
	if (!UMemory::IsAvailable(8192))
		Fail(errorType_Memory, memError_NotEnough);
	
	// make the connect timer if necessary
	if (TPT->connectTimer == 0)
	{
	#if TARGET_API_MAC_CARBON
		TPT->connectTimer = ::OTCreateTimerTaskInContext(_TRConnectTimerUPP, inTpt, NULL);
	#else	
		TPT->connectTimer = ::OTCreateTimerTask(_TRConnectTimerUPP, inTpt);
	#endif
		
		if (TPT->connectTimer == 0) 
			Fail(errorType_Memory, memError_NotEnough);
	}
	
	// convert address to OT format
	otaddrSize = _TRAddressToOT(inAddr, inAddrSize, otaddr, sizeof(otaddr));
	
	// remember what we're connecting to (except if it's not an IP, eg a DNS name)
	if ((otaddrSize >= sizeof(InetAddress)) && (*(OTAddressType *)otaddr == AF_INET))
		TPT->remoteIPAddress = ((InetAddress *)otaddr)->fHost;
	
	// bind endpoint if necessary
	if (!TPT->isBound)
		_TRBindIPAddress(inTpt);
	
	// specify connect options
	ClearStruct(call);
	call.addr.maxlen = otaddrSize;
	call.addr.len = otaddrSize;
	call.addr.buf = otaddr;
	
	// send connect request
	TPT->connectResult = kNotComplete;
	err = OTConnect(TPT->ep, &call, nil);
	if (err && err != kOTNoDataErr)
	{
		TPT->connectResult = 0;
		FailOTError(err);
	}
	TPT->connectSeqVal = call.sequence;
	
	// install connect timer
	::OTScheduleTimerTask(TPT->connectTimer, inMaxSecs ? inMaxSecs * 1000 : 30000);

	::OTAtomicSetBit(&TPT->flags, flag_StartConnectCalled);
}

// returns true if the connection has been established, or false if still waiting
Uint16 URegularTransport::GetConnectStatus(TRegularTransport inTpt)
{
	Require(inTpt);
	
	if (TPT->connectResult != kNotComplete)
	{
		if (TPT->connectTimer)
		{
			::OTDestroyTimerTask(TPT->connectTimer);
			TPT->connectTimer = 0;
		}
		
		UMemory::Dispose((TPtr)TPT->firewallInfo);
		TPT->firewallInfo = nil;
		
		if (TPT->connectResult != 0)
		{
			if (TPT->connectResult == TNODATA || TPT->connectResult == kOTNoDataErr)
				TPT->connectResult = kOTBadAddressErr;
			
			FailOTError(TPT->connectResult);
		}
		
		return kConnectionEstablished;
	}
	
	// no reply from remote host yet
	return kWaitingForConnection;
}

/*
 * Disconnect() sends an abortive disconnect, which is effective
 * immediately.  Any unsent data will NOT get sent.  Note that
 * once the TRegularTransport is disconnected, you CAN'T use StartConnect()
 * on it.  A TRegularTransport can only be connected ONCE.
 */
void URegularTransport::Disconnect(TRegularTransport inTpt)
{
	if (inTpt)
	{
		if (TPT->connectTimer)
		{
			OTDestroyTimerTask(TPT->connectTimer);
			TPT->connectTimer = 0;
		}
		
		// cancel StartConnect()
		if (TPT->connectResult == kNotComplete && TPT->connectSeqVal)
		{
			TCall call = { 0 };
			call.sequence = TPT->connectSeqVal;
			
			OTSndDisconnect(TPT->ep, &call);
			TPT->connectResult = 0;
		}
		
		_TRDisposeUnsent(inTpt);
		OTIoctl(TPT->ep, I_FLUSH, (void *)FLUSHRW);
		OTSndDisconnect(TPT->ep, nil);
		
		OTAtomicClearBit(&TPT->flags, flag_IsConnected);
	}
}

/*
 * StartDisconnect() starts an orderly disconnect.  Any unsent
 * data will be sent before the connection closes.  You cannot
 * send any more data after this call, but you CAN receive data.
 * A msg_ConnectionClosed will be sent once the disconnect
 * is complete.  IsConnected() returns TRUE up until this time.
 * You can receive data even after the connection has closed.
 */
void URegularTransport::StartDisconnect(TRegularTransport inTpt)
{
	if (inTpt)
	{
		EndpointRef ep = TPT->ep;
		Boolean doLeave = OTEnterNotifier(ep);
	
		if (TPT->sendQueue == nil)				// if no outgoing data
			OTSndOrderlyDisconnect(ep);			// then we can disconnect now
		else
			OTAtomicSetBit(&TPT->flags, flag_OrdDisconWhenNoOutData);

		if (doLeave) OTLeaveNotifier(ep);
	}
}

/*
 * IsDisconnecting() returns TRUE if the connection is in the
 * process of being closed (either because StartDisconnect() has
 * been called, or the remote host has requested a disconnect).
 * When TRUE, you should NOT send any more data.  When all unsent
 * data has been actually sent, the connection will close with a
 * msg_ConnectionClosed.  You can continue to receive data even
 * after the connection has closed.
 */
bool URegularTransport::IsDisconnecting(TRegularTransport inTpt)
{
	Require(inTpt);
	return (OTAtomicTestBit(&TPT->flags, flag_OrdDisconWhenNoOutData) || (OTGetEndpointState(TPT->ep) == T_OUTREL));
}

// returns true if the specified TRegularTransport is in the process of connection (returns false if connected or disconnected)
bool URegularTransport::IsConnecting(TRegularTransport inTpt)
{
	return inTpt && (TPT->connectResult == kNotComplete);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

// for TCP, use SInternetAddress and set host to all 0
void URegularTransport::Listen(TRegularTransport inTpt, const void *inAddr, Uint32 inAddrSize)
{
	Require(inTpt);
	
	if ((inAddrSize < sizeof(SInternetAddress)) || (*(Uint16 *)inAddr != kInternetAddressType))
		Fail(errorType_Transport, transError_BadAddress);

	if (TPT->configProtocol == transport_TCPIP)
	{
		// disconnect and close provider
		OTSndDisconnect(TPT->ep, nil);
		OTUnbind(TPT->ep);
		OTCloseProvider(TPT->ep);

		// create new endpoint
		TPT->ep = _TRCreateEndpoint(inTpt, true);
	}

	_TRBindIPAddress(inTpt, ((Uint16 *)inAddr)[1], 16);	
}

// returns NIL if no connection to accept
TRegularTransport URegularTransport::Accept(TRegularTransport inTpt, void *outAddr, Uint32 *ioAddrSize)
{
	EndpointRef ep;
	OSStatus err;
	TCall call;
	Uint8 buf[512];

	// sanity checks
	Require(inTpt);
	
	// if we're already accepting a connection, bail out
	if (TPT->acceptResult == kNotComplete)
		return nil;

	// don't need to post any connection-request messages because we're gonna accept them now
	OTAtomicClearBit(&TPT->postFlags, flag_PostConnectRequest);
	
	// setup structure to get info on the incoming connection
	call.addr.maxlen = sizeof(buf);
	call.addr.len = 0;
	call.addr.buf = buf;
	call.opt.maxlen = call.opt.len = 0;
	call.opt.buf = nil;
	call.udata.maxlen = call.udata.len = 0;
	call.udata.buf = nil;
	call.sequence = 0;

	// get info on the incoming connection
	ep = TPT->ep;
	if (OTListen(ep, &call) != noErr)
		return nil;
	
	// if necessary, copy the remote address out
	if (outAddr && ioAddrSize)
		*ioAddrSize = _TROTToAddress(call.addr.buf, call.addr.len, outAddr, *ioAddrSize);
	
	// create a new endpoint to accept the connection on
	SRegularTransport *acceptTpt = (SRegularTransport *)New(TPT->configProtocol, TPT->configOptions);
	
	// remember the address of the remote host
	if ((call.addr.len >= sizeof(InetAddress)) && (*(OTAddressType *)call.addr.buf == AF_INET))
		acceptTpt->remoteIPAddress = ((InetAddress *)call.addr.buf)->fHost;
	
	// start the process of accepting the connection
	TPT->acceptResult = kNotComplete;
	err = OTAccept(ep, acceptTpt->ep, &call);

	// wait until the accept is complete
	if (err)
	{		
		TPT->acceptResult = 0;
		Dispose((TRegularTransport)acceptTpt);
		return nil;
	}
	else
	{
		while (TPT->acceptResult == kNotComplete) {}
		
		if (TPT->acceptResult != noErr)
		{
			Dispose((TRegularTransport)acceptTpt);
			return nil;
		}
	}

	OTAtomicSetBit(&acceptTpt->flags, flag_StartConnectCalled);
	OTAtomicSetBit(&acceptTpt->flags, flag_IsConnected);
	
	// successfully accepted connection
	return (TRegularTransport)acceptTpt;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void *URegularTransport::NewBuffer(Uint32 inSize)
{
	Handle h;
	OSErr err;
	SSendBuf *buf;
	
	Require(inSize);
	
	h = ::TempNewHandle(sizeof(SSendBuf) + inSize, &err);
	if (h == nil) Fail(errorType_Memory, memError_NotEnough);

	::HLockHi(h);
	buf = (SSendBuf *)*h;
	
	buf->next = nil;
	buf->h = h;
	buf->offset = 0;
	buf->dataSize = inSize;

	return BPTR(buf) + sizeof(SSendBuf);
}

void URegularTransport::DisposeBuffer(void *inBuf)
{
	if (inBuf)
	{
		Handle h = ((SSendBuf *)(BPTR(inBuf) - sizeof(SSendBuf)))->h;
		DisposeHandle(h);
	}
}

Uint32 URegularTransport::GetBufferSize(void *inBuf)
{
	if (inBuf) return ((SSendBuf *)(BPTR(inBuf) - sizeof(SSendBuf)))->dataSize;
	return 0;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void URegularTransport::SendBuffer(TRegularTransport inTpt, void *inBuf)
{
	SSendBuf *buf, *link;
	OTResult result;
	
	// sanity checks
	Require(inTpt && inBuf);
	if (OTAtomicTestBit(&TPT->flags, flag_OrdDisconWhenNoOutData))
	{
		DebugBreak("URegularTransport - cannot send more data after StartDisconnect()");
		Fail(errorType_Misc, error_Protocol);
	}

	// get ptr to buf header	
	buf = (SSendBuf *)(BPTR(inBuf) - sizeof(SSendBuf));
	
	// call data-monitor proc
	if (TPT->dataMonitorProc)
	{
		try {
			TPT->dataMonitorProc(inTpt, buf->data, buf->dataSize, true);
		} catch(...) {}
	}

	// prevent notifier from being called
	EndpointRef ep = TPT->ep;
	Boolean doLeave = OTEnterNotifier(ep);		// so glad they added this function
	
	// if there's a queue, enqueue the new data, otherwise try sending straight away
	if (TPT->sendQueue)
	{
		// add to end of queue
		link = TPT->sendQueue;
		while (link->next)
			link = link->next;
	
		link->next = buf;
	}
	else	// no queue, try sending now, then enqueue what doesn't get sent
	{
		result = OTSnd(ep, buf->data, buf->dataSize, 0);
		if (result < 0) result = 0;		// if error, no data got sent
		
		if (result < buf->dataSize)		// if only part of the data got sent, enqueue the rest
		{
			buf->offset = result;
			TPT->sendQueue = buf;
		}
		else	// all of it got sent, can kill the buffer
			DisposeHandle(buf->h);
	}
	
	// okay, notifier can be called now
	if (doLeave) OTLeaveNotifier(ep);
}

void URegularTransport::Send(TRegularTransport inTpt, const void *inData, Uint32 inDataSize)
{
	SSendBuf *buf, *link;
	OTResult result;
	Handle h;
	OSErr err;
	
	if (inDataSize)
	{
		// sanity checks
		Require(inTpt);
		if (OTAtomicTestBit(&TPT->flags, flag_OrdDisconWhenNoOutData))
		{
			DebugBreak("URegularTransport - cannot send more data after StartDisconnect()");
			Fail(errorType_Misc, error_Protocol);
		}

		// prevent notifier from being called
		EndpointRef ep = TPT->ep;
		Boolean doLeave = OTEnterNotifier(ep);		// if you think it's bad now, you should have seen what it was like before they added this function...

		// if there's a queue, enqueue the new data, otherwise try sending straight away
		if (TPT->sendQueue)
		{
			h = ::TempNewHandle(sizeof(SSendBuf) + inDataSize, &err);
			if (h == nil)
			{
				if (doLeave) OTLeaveNotifier(ep);
				Fail(errorType_Memory, memError_NotEnough);
			}

			::HLockHi(h);
			buf = (SSendBuf *)*h;
			
			buf->next = nil;
			buf->h = h;
			buf->offset = 0;
			buf->dataSize = inDataSize;
			::BlockMoveData(inData, buf->data, inDataSize);
			
			// add to end of queue
			link = TPT->sendQueue;
			while (link->next)
				link = link->next;
			link->next = buf;
		}
		else	// no queue, try sending now, then enqueue what doesn't get sent
		{
			result = OTSnd(ep, (void *)inData, inDataSize, 0);
			if (result < 0) result = 0;		// if error, no data got sent
			
			if (result < inDataSize)		// if only part of the data got sent, enqueue the rest
			{
				Uint32 bytesLeft = inDataSize - result;
				
				h = ::TempNewHandle(sizeof(SSendBuf) + bytesLeft, &err);
				if (h == nil)
				{
					if (doLeave) OTLeaveNotifier(ep);
					Fail(errorType_Memory, memError_NotEnough);
				}
				
				::HLockHi(h);
				buf = (SSendBuf *)*h;
				
				buf->next = nil;
				buf->h = h;
				buf->offset = 0;
				buf->dataSize = bytesLeft;
				::BlockMoveData(BPTR(inData) + result, buf->data, bytesLeft);
				
				TPT->sendQueue = buf;
			}
		}
		
		// okay, notifier can be called now
		if (doLeave) OTLeaveNotifier(ep);
		
		// call the data monitor proc
		if (TPT->dataMonitorProc)
		{
			try {
				TPT->dataMonitorProc(inTpt, inData, inDataSize, true);
			} catch(...) {}
		}
	}
}

bool URegularTransport::ReceiveBuffer(TRegularTransport inTpt, void*& outBuf, Uint32& outSize)
{
	outBuf = nil;
	outSize = 0;
	
	if (inTpt == nil) 
		return false;				
		
	Uint32 nSize = 0;
	Uint32 nReceivedSize = TPT->receivedData.nReceivedSize - TPT->receivedData.nReceivedOffset;
	
	if (OTCountDataBytes(TPT->ep, &nSize) != 0)
	{
		if (nReceivedSize)
		{
			outBuf = URegularTransport::NewBuffer(nReceivedSize);
			if (!outBuf)
				return false;
			
			outSize = nReceivedSize;
			UMemory::Copy(outBuf, (Uint8 *)TPT->receivedData.pData + TPT->receivedData.nReceivedOffset, nReceivedSize);
								
			UMemory::Dispose((TPtr)TPT->receivedData.pData);
			ClearStruct(TPT->receivedData);

			if (TPT->notifyReceivedSize)
				TPT->notifyReceivedSize = 0;

			return true;
		}
		
		return false;
	}
			
	if (nReceivedSize + nSize)
	{
		void *pBuf = URegularTransport::NewBuffer(nReceivedSize + nSize);
		if (!pBuf)
			return false;
			
		if (nReceivedSize)
		{
			UMemory::Copy(pBuf, (Uint8 *)TPT->receivedData.pData + TPT->receivedData.nReceivedOffset, nReceivedSize);
								
			UMemory::Dispose((TPtr)TPT->receivedData.pData);
			ClearStruct(TPT->receivedData);			
		}

		OTFlags nFlags;
		OTResult nResult = 0;
		
		if (nSize)
			nResult = OTRcv(TPT->ep, (Uint8 *)pBuf + nReceivedSize, nSize, &nFlags);
			
		if (nReceivedSize || nResult > 0)
		{
			outBuf = pBuf;
			outSize = nReceivedSize;
			if (nResult > 0)
				outSize += nResult;
						
			if (TPT->notifyReceivedSize)
				TPT->notifyReceivedSize = 0;
	
			// call the data monitor proc
			if (TPT->dataMonitorProc)
			{
				try {
					TPT->dataMonitorProc(inTpt, outBuf, outSize, false);
				} catch(...) {}
			}
				
			/*
			 * New data may have arrived after the OTCountDataBytes but before
			 * the OTRcv, in which case a T_DATA event will not be generated
			 * because the original T_DATA hasn't been cleared yet (OTRcv hasn't
			 * been called).  We check for this situation here.
			 */
			if ((OTCountDataBytes(TPT->ep, &nSize) == 0) && nSize)
			{
				OTAtomicSetBit(&TPT->postFlags, flag_PostDataArrived);
				OTAtomicSetBit(&_TRData.flags, flag_Process);
				_WakeupApp();
			}

			return true;
		}
		
		URegularTransport::DisposeBuffer(pBuf);
	}
			
	return false;
}

/*
 * IMPORTANT: It is possible for Receive() to return 0 even after GetReceiveSize()
 * has indicated that data is pending.
 */
Uint32 URegularTransport::Receive(TRegularTransport inTpt, void *outData, Uint32 inMaxSize)
{
	if (inTpt == nil) 
		return 0;
	
	Uint32 nReceivedSize = TPT->receivedData.nReceivedSize - TPT->receivedData.nReceivedOffset;
	if (nReceivedSize > inMaxSize)
		nReceivedSize = inMaxSize;
	
	if (nReceivedSize)
	{
		UMemory::Copy(outData, (Uint8 *)TPT->receivedData.pData + TPT->receivedData.nReceivedOffset, nReceivedSize);
		TPT->receivedData.nReceivedOffset += nReceivedSize;
		inMaxSize -= nReceivedSize;
		
		if (TPT->receivedData.nReceivedSize == TPT->receivedData.nReceivedOffset)
		{
			UMemory::Dispose((TPtr)TPT->receivedData.pData);
			ClearStruct(TPT->receivedData);			
		}
	}

	OTFlags nFlags;
	OTResult nResult = 0;
		
	if (inMaxSize)
		nResult = OTRcv(TPT->ep, (Uint8 *)outData + nReceivedSize, inMaxSize, &nFlags);
		
	if (nReceivedSize || nResult > 0)
	{
		Uint32 nDataSize = nReceivedSize;
		if (nResult > 0)
			nDataSize += nResult;

		if (TPT->notifyReceivedSize)
			TPT->notifyReceivedSize = 0;

		// call the data monitor proc
		if (TPT->dataMonitorProc)
		{
			try {
				TPT->dataMonitorProc(inTpt, outData, nDataSize, false);
			} catch(...) {}
		}
		
		Uint32 nSize;
		if ((OTCountDataBytes(TPT->ep, &nSize) == 0) && nSize)
		{
			OTAtomicSetBit(&TPT->postFlags, flag_PostDataArrived);
			OTAtomicSetBit(&_TRData.flags, flag_Process);
			_WakeupApp();
		}
		
		return nDataSize;
	}
	
	return 0;
}

Uint32 URegularTransport::GetReceiveSize(TRegularTransport inTpt)
{
	if (inTpt == nil) 
		return 0;
	
	Uint32 nReceivedSize = TPT->receivedData.nReceivedSize - TPT->receivedData.nReceivedOffset;

	size_t nSize;
	if (OTCountDataBytes(TPT->ep, &nSize) != 0)
		return nReceivedSize;
		
	return (nReceivedSize + nSize);
}

// returns number of bytes waiting to be sent
Uint32 URegularTransport::GetUnsentSize(TRegularTransport inTpt)
{
	if (inTpt == nil) return 0;
	
	// prevent notifier from being called (so it doesn't mess with the queue behind our backs)
	Boolean doLeave = OTEnterNotifier(TPT->ep);
	
	SSendBuf *buf = TPT->sendQueue;
	Uint32 s = 0;
	
	while (buf)
	{
		s += buf->dataSize - buf->offset;
		buf = buf->next;
	}
	
	// okay, notifier can be called now
	if (doLeave) OTLeaveNotifier(TPT->ep);

	return s;
}

void URegularTransport::NotifyReadyToReceive(TRegularTransport inTpt, Uint32 inReceivedSize)
{
	Require(inTpt);
	
	TPT->notifyReceivedSize = inReceivedSize;
	if (!TPT->notifyReceivedSize)
		return;

	Uint32 nReceivedSize;
	if (OTCountDataBytes(TPT->ep, &nReceivedSize) != 0 || !nReceivedSize)
		return;

	_TRNotifyReadyToReceive(inTpt, nReceivedSize);	
}

void URegularTransport::NotifyReadyToSend(TRegularTransport inTpt)
{
	Require(inTpt);

	if (TPT->msgProc)
	{
		bool postReady;
		
		Boolean doLeave = OTEnterNotifier(TPT->ep);

		if (TPT->sendQueue == nil || TPT->sendQueue->next == nil)	// if already ready-to-send
		{
			OTAtomicClearBit(&TPT->flags, flag_CheckReadyToSend);
			postReady = true;
		}
		else
		{
			OTAtomicSetBit(&TPT->flags, flag_CheckReadyToSend);
			postReady = false;
		}
		
		if (doLeave) OTLeaveNotifier(TPT->ep);

		if (postReady)
			URegularTransport::PostMessage(inTpt, msg_ReadyToSend, nil, 0, priority_Normal);
	}
}

bool URegularTransport::IsReadyToReceive(TRegularTransport inTpt, Uint32 inReceivedSize)
{
	Require(inTpt);
	
	TPT->notifyReceivedSize = inReceivedSize;
	if (!TPT->notifyReceivedSize)
		return true;

	Uint32 nReceivedSize = TPT->receivedData.nReceivedSize - TPT->receivedData.nReceivedOffset;
	if (nReceivedSize >= inReceivedSize)
		return true;

	if (OTCountDataBytes(TPT->ep, &nReceivedSize) != 0 || !nReceivedSize)
		return false;

	return _TRNotifyReadyToReceive(inTpt, nReceivedSize, false);
}

bool URegularTransport::IsReadyToSend(TRegularTransport inTpt)
{
	if (inTpt == nil) return false;

	// prevent notifier from being called (so it doesn't mess with the queue behind our backs)
	Boolean doLeave = OTEnterNotifier(TPT->ep);

	// we're ready-to-send if there are less than 2 outstanding buffers
	bool isReady = (TPT->sendQueue == nil || TPT->sendQueue->next == nil);
	
	// okay, notifier can be called now
	if (doLeave) OTLeaveNotifier(TPT->ep);

	return isReady;	
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void URegularTransport::SendUnit(TRegularTransport inTpt, const void *inAddr, Uint32 inAddrSize, const void *inData, Uint32 inDataSize)
{
	Require(inTpt && inAddrSize);
	if (inDataSize == 0) return;
	
	// bind endpoint if necessary
	if (!TPT->isBound)
		_TRBindIPAddress(inTpt);

	TUnitData ud;
	Uint8 addr[512];
	OSStatus err;
	
	ud.addr.maxlen = ud.addr.len = _TRAddressToOT(inAddr, inAddrSize, addr, sizeof(addr));
	ud.addr.buf = addr;
	
	ud.opt.maxlen = ud.opt.len = 0;
	ud.opt.buf = nil;
	ud.udata.maxlen = ud.udata.len = inDataSize;
	ud.udata.buf = (Uint8 *)inData;

	err = OTSndUData(TPT->ep, &ud);
	
	if (err == kOTOutStateErr && OTGetEndpointState(TPT->ep) == T_UNBND)
	{
		_TRBindIPAddress(inTpt);			
		OTSndUData(TPT->ep, &ud);
	}
}

/*
 * ReceiveUnit() receives a unit of data (datagram).  Up to <ioDataSize> bytes of data
 * are copied to <outData>, and on exit, <ioDataSize> is set to the actual number of bytes
 * received.  Up to <ioAddrSize> bytes of the senders address are copied to <outAddr>, and
 * on exit, <ioAddrSize> is set to the actual number of bytes copied to <outAddr>.
 * Returns true if a unit was received.  If there is more data than will fit in the supplied
 * buffer, the extra data is lost (another call to ReceiveUnit() will receive the next unit,
 * not the rest of the data).
 */
bool URegularTransport::ReceiveUnit(TRegularTransport inTpt, void *outAddr, Uint32& ioAddrSize, void *outData, Uint32& ioDataSize)
{
	TUnitData ud;
	OTFlags flags;
	Uint8 addr[512];
	
	Require(inTpt);

	ud.addr.maxlen = sizeof(addr);
	ud.addr.len = 0;
	ud.addr.buf = addr;
	ud.opt.maxlen = ud.opt.len = 0;
	ud.opt.buf = nil;
	ud.udata.maxlen = ioDataSize;
	ud.udata.len = 0;
	ud.udata.buf = (Uint8 *)outData;
	
	if (OTRcvUData(TPT->ep, &ud, &flags) == noErr)
	{
		ioAddrSize = _TROTToAddress(ud.addr.buf, ud.addr.len, outAddr, ioAddrSize);
		ioDataSize = ud.udata.len;
		
		if (flags == T_MORE)	// if didn't get all of the datagram, we're gonna just loose the rest
		{
			ud.addr.maxlen = ud.addr.len = 0;
			ud.addr.buf = nil;
			ud.opt.maxlen = ud.opt.len = 0;
			ud.opt.buf = nil;
			ud.udata.maxlen = sizeof(addr);
			ud.udata.len = 0;
			ud.udata.buf = addr;
			
			while (OTRcvUData(TPT->ep, &ud, &flags) == noErr && flags == T_MORE) {}
		}
		
		return true;
	}
	
	ioDataSize = ioAddrSize = 0;
	return false;
}

#pragma mark -

// read the signature and name of the current application
// read the protocol key value
// returns true if the specified protocol is registered
bool URegularTransport::IsRegisteredProtocol(const Uint8 inProtocol[])
{
	Int8 buf[4];
	Uint8 key[256];
	pstrcpy(key, kICHelper);
	UMemory::AppendPString(key, inProtocol);
	
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
	
	Int32 size = sizeof(buf);
	ICAttr attr;
	
	// check if the protocol is registered
	if (ICStart(&inst, signature) == noErr)
	{
	#if !TARGET_API_MAC_CARBON
		if (ICFindConfigFile(inst, 0, nil) == noErr)
	#endif
		{
			if (ICBegin(inst, icReadOnlyPerm) == noErr)
			{
				if (ICGetPref(inst, key, &attr, buf, &size) != icPrefNotFoundErr)
				{
					ICEnd(inst);
					ICStop(inst);
					return true;
				}
				ICEnd(inst);
			}
		}
		ICStop(inst);
	}

	return false;
}

// read the signature and name of the current application
// compare the signature with the protocol key value
// returns true if the current application is registered to handle the specified protocol
bool URegularTransport::IsRegisteredForProtocol(const Uint8 inProtocol[])
{
	Int8 buf[260];
	Uint8 key[256];
	Uint8 name[256];
	pstrcpy(key, kICHelper);
	UMemory::AppendPString(key, inProtocol);
	
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
		info.processName = name;
		info.processAppSpec = nil;
		if (GetProcessInformation(&psn, &info) == noErr)
			signature = info.processSignature;
	}
	else
		return false;
	
	Int32 size = sizeof(buf);
	ICAttr attr;
	
	// check if the protocol is registered
	if (ICStart(&inst, signature) == noErr)
	{
	#if !TARGET_API_MAC_CARBON
		if (ICFindConfigFile(inst, 0, nil) == noErr)
	#endif
		{
			if (ICBegin(inst, icReadOnlyPerm) == noErr)
			{
				if (ICGetPref(inst, key, &attr, buf, &size) != icPrefNotFoundErr)
				{
					if (*((Uint32 *)buf) == signature && !UText::CompareInsensitive(buf + 5, buf[4], name + 1, name[0]))
					{
						ICEnd(inst);
						ICStop(inst);
						return true;
					}
				}
				ICEnd(inst);
			}
		}
		ICStop(inst);
	}

	return false;
}

// read the signature and name of the current application
// using this information create a protocol key
// returns true if the current application was successfully registered to handle the specified protocol
bool URegularTransport::RegisterProtocol(const Uint8 inProtocol[])
{
	Uint8 buf[260];
	Uint8 key[256];
	pstrcpy(key, kICHelper);
	UMemory::AppendPString(key, inProtocol);
	
	ICInstance inst;
	FourCharCode signature;
	ProcessSerialNumber psn;
	ProcessInfoRec info;
	
	// find out the signature and the name of the current process
	psn.highLongOfPSN = psn.lowLongOfPSN = 0;
	if (GetCurrentProcess(&psn) == noErr)
	{
		info.processInfoLength = sizeof(info);
		info.processName = buf + 4;
		info.processAppSpec = nil;
		if (GetProcessInformation(&psn, &info) == noErr)
			signature = info.processSignature;
	}
	else
		return false;
	
	*((Uint32 *)buf) = signature;
		
	// register the protocol
	if (ICStart(&inst, signature) == noErr)
	{
	#if !TARGET_API_MAC_CARBON
		if (ICFindConfigFile(inst, 0, nil) == noErr)
	#endif
		{
			if (ICBegin(inst, icReadWritePerm) == noErr)
			{
				if (ICSetPref(inst, key, kICAttrNoChange, (Int8 *)buf, 4 + 1 + buf[4]) == noErr)
				{
					ICStop(inst);
					ICEnd(inst);
					return true;
				}
			}
			ICEnd(inst);
		}
		ICStop(inst);
	}

	return false;
}

// returns false if unable to launch
bool URegularTransport::LaunchURL(const void *inText, Uint32 inTextSize, Uint32 *ioSelStart, Uint32 *ioSelEnd)
{
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
	
	if (ICStart(&inst, signature) == noErr)
	{
	#if !TARGET_API_MAC_CARBON
		if (ICFindConfigFile(inst, 0, nil) == noErr)
	#endif
		{
			long st = 0;
			long en = inTextSize;
			OSErr err = ICLaunchURL(inst, "\p", 
									(Ptr)inText, inTextSize, 
									ioSelStart ? (long *)ioSelStart : &st, 
									ioSelEnd ? (long *)ioSelEnd : &en);
		    // OSStatus err =
			ICStop(inst);
			return err == noErr;
		}
		
		ICStop(inst);
	}
	
	// unable to launch
	return false;
}


/* -------------------------------------------------------------------------- */
#pragma mark -

static EndpointRef _TRCreateEndpoint(TRegularTransport inTpt, bool inTilisten)
{
	EndpointRef ep = nil;
	OSStatus err;
	TOptMgmt req;
	Uint8 buf[128];
	Uint32 *p;
	
	try
	{		
		// open endpoint
	#if TARGET_API_MAC_CARBON
		ep = ::OTOpenEndpointInContext(_TRCreateConfig(TPT->configProtocol, TPT->configOptions, inTilisten), 0, nil, &err, NULL);
	#else
		ep = ::OTOpenEndpoint(_TRCreateConfig(TPT->configProtocol, TPT->configOptions, inTilisten), 0, nil, &err);
	#endif
		
		CheckOTError(err);	
			
		CheckOTError(::OTInstallNotifier(ep, _TREventNotifyCallbackUPP, inTpt));

		// set standard options
		if (TPT->configProtocol == transport_TCPIP)		// UDP/IP doesn't support these
		{
			req.opt.buf = buf;
			req.flags = T_NEGOTIATE;

			p = (Uint32 *)buf;
			p[0] = kOTFourByteOptionSize;
			p[1] = XTI_GENERIC;
			p[2] = XTI_SNDBUF;
			p[3] = 0;
			p[4] = _TRSendBufSize;
			p[5] = kOTFourByteOptionSize;
			p[6] = XTI_GENERIC;
			p[7] = XTI_RCVBUF;
			p[8] = 0;
			p[9] = 131072;						// 128K
			p[10] = kOTOptionHeaderSize + sizeof(t_linger);
			p[11] = XTI_GENERIC;
			p[12] = XTI_LINGER;
			p[13] = 0;
			p[14] = true;
			p[15] = 200;						// 200 seconds
			
			req.opt.len = req.opt.maxlen = 64;
			CheckOTError(OTOptionManagement(ep, &req, nil));
			
			p[0] = kOTOptionHeaderSize + sizeof(t_kpalive);
			p[1] = INET_TCP;		// b/c different level, can't do all in the one option mgmnt call (f-ing OT!!!)
			p[2] = TCP_KEEPALIVE;
			p[3] = 0;
			p[4] = true;
			p[5] = 10;							// check connection every 10 minutes
			
			req.opt.len = req.opt.maxlen = 24;
			CheckOTError(OTOptionManagement(ep, &req, nil));
		}
		
		// make endpoint asynchronous
		CheckOTError(OTSetAsynchronous(ep));
	}
	catch(...)
	{
		// clean up
		if (ep) OTCloseProvider(ep);
		throw;
	}
	
	return ep;
}

static OTConfiguration *_TRCreateConfig(Uint32 inProtocol, Uint32 inOptions, bool inTilisten)
{
	#pragma unused(inOptions)
	
	const Int8 *path;
	OTConfiguration *config;
	
	switch (inProtocol)
	{
		case transport_TCPIP:
			if (inTilisten)
				path = "tilisten,tcp";
			else
				path = "tcp";
			break;
		
		case transport_UDPIP:
			path = "udp";
			break;
		
		default:
			Fail(errorType_Transport, transError_InvalidConfig);
			break;
	}
	
	config = OTCreateConfiguration(path);
	
	if (config == kOTInvalidConfigurationPtr)
		Fail(errorType_Transport, transError_InvalidConfig);
	else if (config == kOTNoMemoryConfigurationPtr)
		Fail(errorType_Memory, memError_NotEnough);
		
	return config;
}

static void _TRBindIPAddress(TRegularTransport inTpt, Uint16 inPort, Uint16 inGlen)
{
	InetAddress stOtAddr;
	ClearStruct(stOtAddr);
		
	stOtAddr.fAddressType = AF_INET;
	stOtAddr.fPort = inPort;
	stOtAddr.fHost = TPT->localIPAddress.un_IP.stDW_IP.nDW_IP;

	// convert address to OT format
	TBind reqAddr;
			
	// specify address for OT
	reqAddr.addr.maxlen = sizeof(InetAddress);
	reqAddr.addr.len = sizeof(InetAddress);
	reqAddr.addr.buf = (Uint8 *)&stOtAddr;
	
	// specify number of outstanding connection requests
	reqAddr.qlen = inGlen;
	
	// perform the bind
	_TRBind(TPT, &reqAddr, nil);	
}

static void _TRBind(SRegularTransport *inTpt, TBind *inReqAddr, TBind *inRetAddr)
{
	OSStatus err;
	
	if (inTpt->bindResult == kNotComplete)
	{
		DebugBreak("URegularTransport - transporter is already being bound!");
		Fail(errorType_Misc, error_Protocol);
	}
	
	inTpt->bindResult = kNotComplete;
	err = OTBind(inTpt->ep, inReqAddr, inRetAddr);
	if (err)
	{
		inTpt->bindResult = 0;
		FailOTError(err);
	}
	
	while (inTpt->bindResult == kNotComplete) {}
	
	if (inTpt->bindResult == kOTNoAddressErr)
		Fail(errorType_Transport, transError_AddressInUse);
	
	CheckOTError(inTpt->bindResult);
	inTpt->isBound = true;
}

static bool _TRNotifyReadyToReceive(TRegularTransport inTpt, Uint32 inReceivedSize, bool inPostMessage)
{
	if (TPT->receivedData.nReceivedSize - TPT->receivedData.nReceivedOffset >= TPT->notifyReceivedSize)
	{
		TPT->notifyReceivedSize = 0;
		if (inPostMessage)
			URegularTransport::PostMessage(inTpt, msg_DataArrived, nil, 0, priority_Normal);
		
		return true;
	}
		
	Uint32 nReceivedSize = TPT->notifyReceivedSize - (TPT->receivedData.nReceivedSize - TPT->receivedData.nReceivedOffset);
	if (nReceivedSize > inReceivedSize)
		nReceivedSize = inReceivedSize;
		
	if (TPT->receivedData.nDataSize < TPT->receivedData.nReceivedSize + nReceivedSize)
	{
		TPT->receivedData.nDataSize = TPT->receivedData.nReceivedSize + nReceivedSize;
		
		if (TPT->receivedData.pData)
			TPT->receivedData.pData = UMemory::Reallocate((TPtr)TPT->receivedData.pData, TPT->receivedData.nDataSize);
		else
			TPT->receivedData.pData = UMemory::New(TPT->receivedData.nDataSize);
	}
	
	OTFlags nFlags;				
	TPT->receivedData.nReceivedSize += OTRcv(TPT->ep, (Uint8 *)TPT->receivedData.pData + TPT->receivedData.nReceivedSize, nReceivedSize, &nFlags);

	if (TPT->receivedData.nReceivedSize - TPT->receivedData.nReceivedOffset >= TPT->notifyReceivedSize)
	{
		TPT->notifyReceivedSize = 0;
		if (inPostMessage)
			URegularTransport::PostMessage(inTpt, msg_DataArrived, nil, 0, priority_Normal);
			
		return true;
	}
	
	return false;
}

/*
EVENT NOTIFIER FUNCTION

Called at deferred task time.  Same restrictions as interrupts.  Cannot move memory, use
unlocked handles etc.

Must be reentrant in certain cases:

However, Open Transport does guarantee that if you are processing one of the primary notifications
(T_LISTEN, T_CONNECT, T_DATA, T_EXDATA, T_DISCONNECT, T_UDERR, T_ORDREL, T_REQUEST, T_REPLY,
T_PASSCON) or one of the completion notifications (those notifications ending with the word
COMPLETE), you will not be reentered.  Other events (most notably, kOTProviderWillClose and
T_MEMORYRELEASED) will cause your notifier to be reentered.  Beginning with version 1.1.1, a new
set of APIs (OTEnterNotifier and OTLeaveNotifier) allow your foreground code to limit interruption 
by notification events (see a description of these functions for more information).

Note:  OTAtomicSetBit returns previous value.  OTAtomicAdd32 returns new value.
*/
static pascal void _TREventNotifyCallback(void *inContext, OTEventCode inEventCode, OTResult inResult, void *inParam)
{
	SRegularTransport *tpt = (SRegularTransport *)inContext;
	OTResult err;
	TUDErr uderr;
	Uint8 buf[16];
	
	switch (inEventCode)
	{
		/*
		 * Data has arrived.  A note about how T_DATA events are generated:
		 * In looking at the source code, there is an internal state flag associated
		 * with the TNativeEndpoint object, which is checked as each incoming packet
		 * is processed.  If the flag is cleared, then the T_DATA event is generated.
		 * This flag is only cleared on calls to OTRcv and OTRcvUData when there is
		 * no data.
		 */
		case T_DATA:
		case T_EXDATA:
			if (OTAtomicTestBit(&tpt->flags, flag_CheckFirewallReply))
			{
				OTFlags flags;
				if (OTRcv(tpt->ep, buf, 8, &flags) == 8)
				{
					OTAtomicClearBit(&tpt->flags, flag_CheckFirewallReply);
					if ((buf[0] == 4 || buf[0] == 0) && buf[1] == 90)	// if connect successful
					{
						tpt->connectResult = 0;
						OTAtomicSetBit(&tpt->flags, flag_IsConnected);
						OTAtomicSetBit(&tpt->postFlags, flag_PostConnectEstab);
					}
					else	// connect failed
					{
						tpt->connectResult = kECONNREFUSEDErr;
						OTAtomicSetBit(&tpt->postFlags, flag_PostConnectRefuse);
					}
					OTAtomicSetBit(&_TRData.flags, flag_Process);
					_WakeupApp();
				}
			}
			else
			{
				OTAtomicSetBit(&tpt->postFlags, flag_PostDataArrived);
				OTAtomicSetBit(&_TRData.flags, flag_Process);
				_WakeupApp();
			}
			break;
			
		// flow-control restrictions lifted
		case T_GODATA:
		case T_GOEXDATA:
			OTLook(tpt->ep);		// eat this event (clear/unblock)
			_TRSendQueued((TRegularTransport)tpt);
			break;

		// connection established
		case T_CONNECT:
			err = OTRcvConnect(tpt->ep, nil);
			if (err == 0)
			{
				if (tpt->firewallInfo == nil)	// if not using a firewall
				{
					tpt->connectResult = 0;
					OTAtomicSetBit(&tpt->flags, flag_IsConnected);
					OTAtomicSetBit(&tpt->postFlags, flag_PostConnectEstab);
				}
				else	// using a firewall
				{
					// make packet of info to tell firewall where to connect to on our behalf
					buf[0] = 4;
					buf[1] = 1;
					*(Uint16 *)(buf+2) = tpt->firewallInfo->port;
					*(Uint32 *)(buf+4) = tpt->firewallInfo->ipaddr.fHost;
					buf[8] = 'x';
					buf[9] = 0;

					err = OTSnd(tpt->ep, buf, 10, 0);
					if (err == 10)	// if all sent
						OTAtomicSetBit(&tpt->flags, flag_CheckFirewallReply);
					else
					{
						if (err < 0) tpt->connectResult = err;
						else tpt->connectResult = kOTFlowErr;
						OTAtomicSetBit(&tpt->postFlags, flag_PostConnectRefuse);
					}
				}
			}
			else
			{
				if (tpt->connectResult == kEHOSTDOWNErr)	// if connect timer went off
					err = kEHOSTDOWNErr;
				
				OTAtomicSetBit(&tpt->postFlags, flag_PostConnectRefuse);
				tpt->connectResult = err;
			}
			OTAtomicSetBit(&_TRData.flags, flag_Process);
			_WakeupApp();
			break;
		
		// connection closed
		case T_DISCONNECT:
			if (inParam || tpt->connectResult == kNotComplete)
			{
				// rejected connection request
				TDiscon discon;
				ClearStruct(discon);
				err = OTRcvDisconnect(tpt->ep, &discon);
				if (err == 0) err = discon.reason;
				if (err == 0) err = kECONNREFUSEDErr;
				tpt->connectResult = err;
				OTAtomicClearBit(&tpt->flags, flag_IsConnected);
				OTAtomicSetBit(&tpt->postFlags, flag_PostConnectRefuse);
				OTAtomicSetBit(&_TRData.flags, flag_Process);
				_WakeupApp();
			}
			else
			{
				// remote host closed connection
				if (OTAtomicClearBit(&tpt->flags, flag_CheckFirewallReply))
				{
					// handle the situation where the connection to the firewall closes before getting the reply
					tpt->connectResult = kECONNREFUSEDErr;
					OTAtomicSetBit(&tpt->postFlags, flag_PostConnectRefuse);
				}
				else if (OTAtomicClearBit(&tpt->flags, flag_IsConnected))
					OTAtomicSetBit(&tpt->postFlags, flag_PostConnectionClosed);
				
				OTRcvDisconnect(tpt->ep, nil);
				OTAtomicSetBit(&_TRData.flags, flag_Process);
				_WakeupApp();
			}
			break;

		// disconnect complete
		case T_DISCONNECTCOMPLETE:
			OTAtomicClearBit(&tpt->flags, flag_IsConnected);
			OTAtomicSetBit(&tpt->postFlags, flag_PostConnectionClosed);
			OTAtomicSetBit(&_TRData.flags, flag_Process);
			_WakeupApp();
			break;
			
		// orderly disconnect
		case T_ORDREL:
			OTAtomicClearBit(&tpt->flags, flag_IsConnected);
			OTAtomicSetBit(&tpt->postFlags, flag_PostConnectionClosed);
			OTRcvOrderlyDisconnect(tpt->ep);
			OTSndOrderlyDisconnect(tpt->ep);
			OTAtomicSetBit(&_TRData.flags, flag_Process);
			_WakeupApp();
			break;

		// remote computer is requesting connection
		case T_LISTEN:
			OTAtomicSetBit(&tpt->postFlags, flag_PostConnectRequest);
			OTAtomicSetBit(&_TRData.flags, flag_Process);
			_WakeupApp();
			break;
		
		// bind operation completed
		case T_BINDCOMPLETE:
			tpt->bindResult = inResult;
			_WakeupApp();
			break;
		
		// connection-request-acceptance operation completed
		case T_ACCEPTCOMPLETE:
			tpt->acceptResult = inResult;
			_WakeupApp();
			break;
		
		// provider is closing
		case kOTProviderWillClose:
			OTAtomicClearBit(&tpt->flags, flag_IsConnected);
			OTAtomicSetBit(&tpt->postFlags, flag_PostConnectionClosed);
			OTAtomicSetBit(&_TRData.flags, flag_Process);
			_WakeupApp();
			break;
			
		// unit data error
		case T_UDERR:
			uderr.addr.maxlen = uderr.addr.len = uderr.opt.maxlen = uderr.opt.len = 0;
			uderr.addr.buf = uderr.opt.buf = nil;
			uderr.error = 0;
			OTRcvUDErr(tpt->ep, &uderr);
			break;
		
		// flush command issued via OTIoctl
		case kStreamIoctlEvent:
			OTSndDisconnect(tpt->ep, nil);
			break;
		
		// finished resolving an address (for firewall)
		case T_RESOLVEADDRCOMPLETE:
			if (inResult != noErr)
			{
				tpt->connectResult = inResult;
				OTAtomicSetBit(&tpt->postFlags, flag_PostConnectRefuse);
				OTAtomicSetBit(&_TRData.flags, flag_Process);
				_WakeupApp();
			}
			else
			{
				TCall call = { 0 };
				call.addr.maxlen = call.addr.len = tpt->firewallInfo->firewallAddrSize;
				call.addr.buf = tpt->firewallInfo->firewallAddr;
				
				err = OTConnect(tpt->ep, &call, nil);
				if (err && err != kOTNoDataErr)
				{
					tpt->connectResult = err;
					OTAtomicSetBit(&tpt->postFlags, flag_PostConnectRefuse);
					OTAtomicSetBit(&_TRData.flags, flag_Process);
					_WakeupApp();
				}
				
				tpt->connectSeqVal = call.sequence;
			}
			break;
		
		case T_GETPROTADDRCOMPLETE:
			tpt->getProtAddrResult = inResult;
			_WakeupApp();
			break;
	}
}

static void _TRSendQueued(TRegularTransport inTpt)
{
	EndpointRef ep = TPT->ep;
	Boolean doLeave = OTEnterNotifier(ep);

	SSendBuf *buf = TPT->sendQueue;
	SSendBuf *nextbuf;
	OTResult result;
	Uint32 s;
	bool wakeApp = false;
	
	while (buf)
	{
		s = buf->dataSize - buf->offset;
		result = OTSnd(ep, buf->data + buf->offset, s, 0);
				
		if (result <= 0) break;	// if error or no data sent, just fall out of the loop
		
		if (result < s)	// if only part of the data was sent
		{
			buf->offset += result;
			break;
		}
		
		// all of this buffer was sent, so we can dispose and unlink it
		nextbuf = buf->next;
		OTLIFOEnqueue(&_TRData.killBufList, (OTLink *)buf);
		wakeApp = true;
		TPT->sendQueue = nextbuf;
		
		// move to next buffer
		buf = nextbuf;
	}

	if (TPT->sendQueue == nil && OTAtomicClearBit(&TPT->flags, flag_OrdDisconWhenNoOutData))
		OTSndOrderlyDisconnect(ep);
	else if ((TPT->sendQueue == nil || TPT->sendQueue->next == nil) && OTAtomicClearBit(&TPT->flags, flag_CheckReadyToSend))
	{
		OTAtomicSetBit(&TPT->postFlags, flag_PostReadyToSend);
		wakeApp = true;
	}
	
	if (wakeApp)
	{
		OTAtomicSetBit(&_TRData.flags, flag_Process);
		_WakeupApp();
	}
	
	if (doLeave) OTLeaveNotifier(ep);
}

static Uint32 _TRAddressToOT(const void *inAddr, Uint32 inAddrSize, void *outAddr, Uint32 inMaxSize)
{
	if (inAddrSize < 2)
		Fail(errorType_Transport, transError_BadAddress);
	
	Uint16 type = *(Uint16 *)inAddr;
	
	if (type == kInternetAddressType)
	{
		if (inAddrSize < sizeof(SInternetAddress))
			Fail(errorType_Transport, transError_BadAddress);
		if (sizeof(InetAddress) > inMaxSize)
			Fail(errorType_Transport, transError_Unknown);

		const SInternetAddress& addr = *(SInternetAddress *)inAddr;
		InetAddress otaddr;
		
		otaddr.fAddressType = AF_INET;
		otaddr.fPort = addr.port;
		otaddr.fHost = *(Uint32 *)addr.host;
		
		((Uint32 *)otaddr.fUnused)[0] = 0;
		((Uint32 *)otaddr.fUnused)[1] = 0;
		
		*(InetAddress *)outAddr = otaddr;
		return sizeof(InetAddress);
	}
	else if (type == kInternetNameAddressType)
	{
		if (inAddrSize > sizeof(SInternetNameAddress))
			Fail(errorType_Transport, transError_BadAddress);
		
		Uint8 buf[64];
		Uint8 *p, *ep, *np;
		Uint32 s, ms, n;
		
		const SInternetNameAddress& addr = *(SInternetNameAddress *)inAddr;
		DNSAddress& otaddr = *(DNSAddress *)outAddr;
		
		otaddr.fAddressType = AF_DNS;
		
		// fail if zero length
		np = BPTR(addr.name);
		if (np[0] == 0)
			Fail(errorType_Transport, transError_BadAddress);
		
		// remove spaces at start
		p = np+1;
		ep = p + np[0];
		n = 0;
		while (p != ep)
		{
			if (UText::IsSpace(*p)) n++;
			else break;
			p++;
		}
		if (n)
		{
			np[0] -= n;
			if (np[0] == 0) Fail(errorType_Transport, transError_BadAddress);
			BlockMoveData(np+1+n, np+1, np[0]);
		}
		
		// remove spaces at end
		ep = np;
		p = ep + np[0];
		n = 0;
		while (p != ep)
		{
			if (UText::IsSpace(*p)) n++;
			else break;
			p--;
		}
		np[0] -= n;
		if (np[0] == 0) Fail(errorType_Transport, transError_BadAddress);
		
		// make sure size doesn't include terminating nulls
		ms = np[0];
		s = 0;
		p = (Uint8 *)np+1;
		while (*p++)
		{
			if (s == ms) break;
			++s;
		}
		
		// copy name out
		BlockMoveData(addr.name+1, otaddr.fName, s);

		// use correct port
		if (addr.port == 0)
		{
			// check that name contains port
			p = (Uint8 *)otaddr.fName;
			ep = p + s;
			while (p < ep) if (*p++ == ':') goto portIsGood;
			Fail(errorType_Transport, transError_BadAddress);
		}
		else
		{
			// convert port num to text
			n = addr.port;
			np = &buf[sizeof(buf)];
			for (; n; n /= 10) *--np = n % 10 + '0';

			// determine where to write port
			p = (Uint8 *)otaddr.fName;
			ep = p + s;
			while (p < ep) if (*p++ == ':') { p--; break; }
			
			// append port to name
			*p++ = ':';
			ep = &buf[sizeof(buf)];
			while (np < ep) *p++ = *np++;
			
			// determine new size
			s = p - (Uint8 *)otaddr.fName;
		}
		portIsGood:
		
		// return size of address
		return s + 2;
	}
	else
		Fail(errorType_Transport, transError_BadAddress);
	
	return 0;
}

static Uint32 _TROTToAddress(const void *inAddr, Uint32 inAddrSize, void *outAddr, Uint32 inMaxSize)
{
	Uint32 s;
	Uint16 type;
	
	if (inAddrSize < 6)
		Fail(errorType_Transport, transError_BadAddress);
	
	type = *(Uint16 *)inAddr;
	
	if (type == AF_INET)
	{
		InetAddress& otaddr = *(InetAddress *)inAddr;
		SInternetAddress addr;
		addr.type = kInternetAddressType;
		addr.port = otaddr.fPort;
		((Uint32 *)addr.host)[0] = otaddr.fHost;
		((Uint32 *)addr.host)[1] = 0;
		
		s = sizeof(SInternetAddress);
		if (s > inMaxSize) s = inMaxSize;
		BlockMoveData(&addr, outAddr, s);
		return s;
	}
	else if (type == AF_DNS)
	{
		DNSAddress& otaddr = *(DNSAddress *)inAddr;
		SInternetNameAddress addr;
		
		addr.type = kInternetNameAddressType;
		addr.port = 0;
		
		s = inAddrSize - 2;
		addr.name[0] = s > 255 ? 255 : s;
		BlockMoveData(otaddr.fName, addr.name+1, addr.name[0]);
		
		s = 5 + addr.name[0];
		if (s > inMaxSize) s = inMaxSize;
		BlockMoveData(&addr, outAddr, s);
		return s;
	}
	else
		Fail(errorType_Transport, transError_BadAddress);
	
	return 0;
}

static void _TRProcess()
{	
	if (!OTAtomicClearBit(&_TRData.flags, flag_Process))
		return;
	
	SRegularTransport *tpt = _TRData.firsTRegularTransporter;
	SSendBuf *buf, *nextbuf;
	Handle h;

	while (tpt)
	{
		if ((tpt->postFlags || tpt->flags) && tpt->msgProc)
		{
			// PostConnectionClosed
			if (OTAtomicClearBit(&tpt->postFlags, flag_PostConnectionClosed))
			{
				OTAtomicClearBit(&tpt->postFlags, flag_PostDataArrived);		// closed, so ignore new data
				URegularTransport::PostMessage((TRegularTransport)tpt, msg_ConnectionClosed, nil, 0, priority_Normal);
			}
			
			// PostConnectEstab - note MUST be checked before PostDataArrived (so ConnectEstab is posted before DataArrived)
			if (OTAtomicClearBit(&tpt->postFlags, flag_PostConnectEstab))
				URegularTransport::PostMessage((TRegularTransport)tpt, msg_ConnectionEstablished, nil, 0, priority_Normal);

			// PostDataArrived
			if (OTAtomicClearBit(&tpt->postFlags, flag_PostDataArrived))
			{
				size_t nReceivedSize;
				if (OTCountDataBytes(tpt->ep, &nReceivedSize) == 0 && nReceivedSize)
				{								
					if (tpt->notifyReceivedSize)
						_TRNotifyReadyToReceive((TRegularTransport)tpt, nReceivedSize);
					else							
						URegularTransport::PostMessage((TRegularTransport)tpt, msg_DataArrived, nil, 0, priority_Normal);
				}
			}
			
			// PostConnectRequest
			if (OTAtomicClearBit(&tpt->postFlags, flag_PostConnectRequest))
				URegularTransport::PostMessage((TRegularTransport)tpt, msg_ConnectionRequest, nil, 0, priority_Normal);
			
			// PostConnectRefuse
			if (OTAtomicClearBit(&tpt->postFlags, flag_PostConnectRefuse))
				URegularTransport::PostMessage((TRegularTransport)tpt, msg_ConnectionRefused, nil, 0, priority_Normal);
			
			// PostReadyToSend
			if (OTAtomicClearBit(&tpt->postFlags, flag_PostReadyToSend))
				URegularTransport::PostMessage((TRegularTransport)tpt, msg_ReadyToSend, nil, 0, priority_Normal);
		}
		
		// next transport
		tpt = tpt->next;
	}
	
	// dispose everything in kill list
	buf = (SSendBuf *)OTLIFOStealList(&_TRData.killBufList);
	while (buf)
	{
		nextbuf = buf->next;
		h = buf->h;
		DisposeHandle(h);
		buf = nextbuf;
	}	
}

#if !TARGET_API_MAC_CARBON
// If InitOpenTransport() has been called and CloseOpenTransport() is not called
// when the application terminates, Open Transport usually crashes the computer
// later on, particularly when trying to restart from the Finder.  This ExitToShell()
// patch will ensure that CloseOpenTransport() is always called.
static pascal void _TRExitToShell()
{		
	// kill timers
	SRegularTransport *tpt = _TRData.firsTRegularTransporter;
	while (tpt)
	{
		if (tpt->connectTimer) 
			::OTDestroyTimerTask(tpt->connectTimer);
			
		::OTCloseProvider(tpt->ep);
		tpt = tpt->next;
	}
		
	// deinit open transport
	if (_TRData.isInitted)
	{
		_TRData.isInitted = false;
		::CloseOpenTransport();
	}
		
	CallExitToShellProc(_APSaveExitToShell);
}
#endif

static pascal void _TRConnectTimer(void *inArg)
{
	// don't need to and must not call OTEnterInterrupt() in here (it's only for non-OT-scheduled tasks)
	
	SRegularTransport *tpt = (SRegularTransport *)inArg;
	
	if (tpt->connectResult == kNotComplete)
	{
		OTSndDisconnect(tpt->ep, nil);
		
		tpt->connectResult = kEHOSTDOWNErr;
		OTAtomicSetBit(&tpt->postFlags, flag_PostConnectRefuse);
		
		_WakeupApp();
	}
}

struct SSendData
{
	void *pSendData;
	Uint32 nDataSize;
	bool bDataBuffer;
};

void _TRGetUnsentData(TRegularTransport inTpt, CPtrList<SSendData> *inSendDataList)
{
	SSendBuf *pNextBuf;
	SSendBuf *pSendBuf = TPT->sendQueue;
	
	while (pSendBuf)
	{
		if (pSendBuf->offset)
		{
			pNextBuf = pSendBuf->next;
			UMemory::Dispose((TPtr)pSendBuf);
			pSendBuf = pNextBuf;
			
			continue;
		}
		
		SSendData *pSendData = (SSendData *)UMemory::New(sizeof(SSendData));
		
		pSendData->pSendData = pSendBuf->data;
		pSendData->nDataSize = pSendBuf->dataSize;
		pSendData->bDataBuffer = false;

		inSendDataList->AddItem(pSendData);
		
		pNextBuf = pSendBuf->next;
		pSendBuf->next = nil;
		pSendBuf = pNextBuf;
	}

	TPT->sendQueue = nil;
}

static void _TRDisposeUnsent(TRegularTransport inTpt)
{
	SSendBuf *buf, *nextbuf;
	
	Boolean doLeave = OTEnterNotifier(TPT->ep);
	
	buf = TPT->sendQueue;
	TPT->sendQueue = nil;
	
	if (doLeave) OTLeaveNotifier(TPT->ep);

	while (buf)
	{
		nextbuf = buf->next;
				
		DisposeHandle(buf->h);
		buf = nextbuf;
	}
}

typedef struct {
	Int16 mac, type, id;
} SErrorEntry;

static const SErrorEntry gOTErrorTable[] = {
	{ -3285,	errorType_Transport,	transError_ConnectionClosed			},
	{ -3264,	errorType_Transport,	transError_HostUnreachable			},
	{ -3263,	errorType_Transport,	transError_HostNotResponding		},
	{ -3260,	errorType_Transport,	transError_ConnectionRefused		},
	{ -3259,	errorType_Transport,	transError_ConnectionTimedOut		},
	{ -3253,	errorType_Transport,	transError_ConnectionClosed			},
	{ -3252,	errorType_Transport,	transError_ConnectionClosed			},
	{ -3251,	errorType_Transport,	transError_ConnectionClosed			},
	{ -3211,	errorType_Memory,		memError_NotEnough					},
	{ -3170,	errorType_Transport,	transError_BadAddress				},
	{ -3165,	errorType_Misc,			error_Param							},
	{ -3160,	errorType_Misc,			error_Param							},
	{ -3159,	errorType_Misc,			error_Param							},
	{ -3158,	errorType_Transport,	transError_PendingAction			},
	{ -3156,	errorType_Misc,			error_Param							},
	{ -3155,	errorType_Misc,			error_Protocol						},
	{ -3154,	errorType_Misc,			error_Param							},
	{ -3153,	errorType_Misc,			error_Param							},
	{ -3151,	errorType_Misc,			error_Param							},
	{ -3150,	errorType_Transport,	transError_BadAddress				},
	{ 21,		errorType_Transport,	transError_BadAddress				},
	{ 50,		errorType_Transport,	transError_HostUnreachable			},
	{ 51,		errorType_Transport,	transError_HostUnreachable			},
	{ 52,		errorType_Transport,	transError_ConnectionClosed			},
	{ 53,		errorType_Transport,	transError_ConnectionClosed			},
	{ 54,		errorType_Transport,	transError_ConnectionClosed			},
	{ 55,		errorType_Memory,		memError_NotEnough					},
	{ 60,		errorType_Transport,	transError_ConnectionTimedOut		},
	{ 61,		errorType_Transport,	transError_ConnectionRefused		},
	{ 64,		errorType_Transport,	transError_HostNotResponding		},
	{ 65,		errorType_Transport,	transError_HostUnreachable			}
};

static void _ConvertOTError(Int16 inOTError, Int16& outType, Int16& outID)
{
	Int32 lowBound	= 1;
	Int32 highBound	= sizeof(gOTErrorTable) / sizeof(SErrorEntry);
	Int32 i;
	const SErrorEntry *table = gOTErrorTable;
	const SErrorEntry *entry;
	
	do
	{
		i = (lowBound + highBound) >> 1;		// same as:  (lowBound + highBound) / 2
		entry = table + i - 1;
		
		if (inOTError == entry->mac)
		{
			outType = entry->type;
			outID = entry->id;
			return;
		}
		else if (inOTError < entry->mac)
			highBound = i - 1;
		else
			lowBound = i + 1;
	
	} while (lowBound <= highBound);
	
	// not found, try standard mac errors
	_ConvertMacError(inOTError, outType, outID);
	
	if (outType == errorType_Misc && outID == error_Unknown)	// if not found
	{
		// unknown transport error
		outType = errorType_Transport;
		outID = transError_Unknown;
	}
}

static void _FailOTError(Int16 inOTError, const Int8 *inFile, Uint32 inLine)
{
	SError err;
	
	err.file = inFile;
	err.line = inLine;
	err.fatal = err.silent = false;
	err.special = inOTError;
	
	_ConvertOTError(inOTError, err.type, err.id);
	
	throw err;
}

static void _FailOTError(Int16 inOTError)
{
	SError err;
	
	err.file = nil;
	err.line = 0;
	err.fatal = err.silent = false;
	err.special = inOTError;
	
	_ConvertOTError(inOTError, err.type, err.id);
	
	throw err;
}

inline void _CheckOTError(Int16 inOTError, const Int8 *inFile, Uint32 inLine)
{
	if (inOTError) _FailOTError(inOTError, inFile, inLine);
}

inline void _CheckOTError(Int16 inOTError)
{
	if (inOTError) _FailOTError(inOTError);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

//static pascal OSErr _GURLHandler(const AppleEvent *inEvent, AppleEvent *inReply, long inRefCon);
static pascal OSErr _GURLHandler(const AppleEvent *inEvent, AppleEvent */* inReply */, long /* inRefCon */)
{
	OSErr err;
	DescType typeCode;
	Uint8 buf[1024];
	Size actualSize;
	
	err = AEGetKeyPtr(inEvent, keyDirectObject, typeWildCard, &typeCode, buf, sizeof(buf), &actualSize);
	if (err) return err;
	
	try {
		UApplication::PostMessage(msg_OpenURL, buf, actualSize);
	} catch(...) {}
	
	return noErr;
}


void _InstallURLHandler()
{	
#if TARGET_API_MAC_CARBON
	static AEEventHandlerUPP _GURLHandlerUPP = ::NewAEEventHandlerUPP(_GURLHandler);
	::AEInstallEventHandler(kInternetEventClass, kAEGetURL, _GURLHandlerUPP, 0, false);
#else
	static RoutineDescriptor _GURLHandlerRD = BUILD_ROUTINE_DESCRIPTOR(uppAEEventHandlerProcInfo, _GURLHandler);
	::AEInstallEventHandler(kInternetEventClass, kAEGetURL, &_GURLHandlerRD, 0, false);
#endif
}

#endif /* MACINTOSH */

