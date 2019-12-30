/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "URegularTransport.h"

/*
 * Types
 */

typedef class TTransportObj *TTransport;

/*
 * Constants
 */

enum {
	// misc constants
	kInternetAddressType		= 2,
	kInternetNameAddressType	= 42,
	transport_TCPIP				= 1,
	transport_UDPIP				= 2,

	// messages
	msg_DataArrived				= 100,
	msg_DataTimeOut				= 101,
	msg_ReadyToSend				= 102,
	msg_ConnectionClosed		= 103,
	msg_ConnectionRequest		= 104,
	msg_ConnectionEstablished	= 105,
	msg_ConnectionRefused		= 106,
	
	// connect status
	kWaitingForConnection		= 1,
	kEstablishingConnection		= 2,
	kConnectionEstablished		= 3,
	
	// proxy servers
	proxy_HttpAddress			= 1,
	proxy_HttpSecureAddress		= 2,
	proxy_FtpAddress			= 3,
	proxy_GopherAddress			= 4,
	proxy_SocksAddress			= 5	
};

/*
 * Transport module
 */
 
class UTransport
{
	public:
		// initialize
		static bool IsAvailable();
		static bool HasTCP();
		static void Init();
		
		// new, dispose, properties
		static TTransport New(Uint32 inProtocol, bool inRegular = true, Uint32 inOptions = 0);
		static void Dispose(TTransport inTpt);
		static void SetDataMonitor(TTransport inTpt, TTransportMonitorProc inProc);
		static bool IsRegular(TTransport inTpt);
		
		// messaging
		static void SetMessageHandler(TTransport inTpt, TMessageProc inProc, void *inContext = nil);
		static void PostMessage(TTransport inTpt, Uint32 inMsg, const void *inData = nil, Uint32 inDataSize = 0, Int16 inPriority = priority_Normal);
		static void ReplaceMessage(TTransport inTpt, Uint32 inMsg, const void *inData = nil, Uint32 inDataSize = 0, Int16 inPriority = priority_Normal);

		// addresses
		static SIPAddress GetDefaultIPAddress();
		static void SetIPAddress(TTransport inTpt, SIPAddress inIPAddress);
		static SIPAddress GetIPAddress(TTransport inTpt);
		static Uint32 GetRemoteAddressText(TTransport inTpt, void *outAddr, Uint32 inMaxSize);
		static Uint32 GetLocalAddressText(TTransport inTpt, void *outAddr, Uint32 inMaxSize);
		static Uint32 CleanUpAddressText(Uint16 inType, const void *inAddr, Uint32 inAddrSize, void *outAddr, Uint32 inMaxSize);
		static Uint32 GetPortFromAddressText(Uint16 inType, const void *inAddr, Uint32 inAddrSize);
		static Uint32 GetLocalAddress(TTransport inTpt, void *outAddr, Uint32 inMaxSize);
		static Uint32 GetProxyServerAddress(Uint16 inType, void *outProxyAddr, Uint32 inMaxSize, const void *inBypassAddr = nil, Uint32 inBypassAddrSize = 0);
		static Uint32 IPAddressToText(SIPAddress inIPAddress, void *outAddr, Uint32 inMaxSize);
		static bool TextToIPAddress(Uint8 *inAddr, SIPAddress &outIPAddress);
		static bool GetEthernetAddress(SEthernetAddress& outEthernetAddr);

		// connecting and disconnecting
		static bool IsConnected(TTransport inTpt);
		static void StartConnect(TTransport inTpt, const void *inAddr, Uint32 inAddrSize, Uint32 inMaxSecs = 0);
		static void StartConnectThruFirewall(TTransport inTpt, const void *inAddr, Uint32 inAddrSize, const void *inFirewallAddr, Uint32 inFirewallAddrSize, Uint32 inMaxSecs = 0, Uint32 inOptions = 0);
		static Uint16 GetConnectStatus(TTransport inTpt);
		static void Disconnect(TTransport inTpt);
		static void StartDisconnect(TTransport inTpt);
		static bool IsDisconnecting(TTransport inTpt);
		static bool IsConnecting(TTransport inTpt);

		// incoming connection requests
		static void Listen(TTransport inTpt, const void *inAddr, Uint32 inAddrSize);
		static TTransport Accept(TTransport inTpt, void *outAddr = nil, Uint32 *ioAddrSize = nil);
		
		// buffers
		static void *NewBuffer(Uint32 inSize);
		static void DisposeBuffer(void *inBuf);
		static Uint32 GetBufferSize(void *inBuf);

		// send and receive data
		static void SendBuffer(TTransport inTpt, void *inBuf);
		static void Send(TTransport inTpt, const void *inData, Uint32 inDataSize);
		static bool ReceiveBuffer(TTransport inTpt, void*& outBuf, Uint32& outSize);
		static Uint32 Receive(TTransport inTpt, void *outData, Uint32 inMaxSize);
		static Uint32 GetReceiveSize(TTransport inTpt);
		static Uint32 GetUnsentSize(TTransport inTpt);
		static void NotifyReadyToReceive(TTransport inTpt, Uint32 inReceivedSize);
		static void NotifyReadyToSend(TTransport inTpt);
		static bool IsReadyToReceive(TTransport inTpt, Uint32 inReceivedSize);
		static bool IsReadyToSend(TTransport inTpt);
		
		// connectionless send/receive data
		static void SendUnit(TTransport inTpt, const void *inAddr, Uint32 inAddrSize, const void *inData, Uint32 inDataSize);
		static bool ReceiveUnit(TTransport inTpt, void *outAddr, Uint32& ioAddrSize, void *outData, Uint32& ioDataSize);

		// global protocol stuff
		static bool IsRegisteredProtocol(const Uint8 inProtocol[]);
		static bool IsRegisteredForProtocol(const Uint8 inProtocol[]);
		static bool RegisterProtocol(const Uint8 inProtocol[]);

		static bool LaunchURL(const void *inText, Uint32 inTextSize, Uint32 *ioSelStart = nil, Uint32 *ioSelEnd = nil);
		static bool LaunchURL(UInt8* pString) { return LaunchURL(1+pString, pString[0]); };
};

/*
 * Transport errors
 */

enum {
	errorType_Transport					= 8,
	transError_Unknown					= 100,
	transError_BadAddress				= transError_Unknown + 1,
	transError_ConnectionClosed			= transError_Unknown + 2,
	transError_ConnectionTimedOut		= transError_Unknown + 3,
	transError_ConnectionRefused		= transError_Unknown + 4,
	transError_HostNotResponding		= transError_Unknown + 5,
	transError_HostUnreachable			= transError_Unknown + 6,
	transError_InvalidConfig			= transError_Unknown + 7,
	transError_PendingAction			= transError_Unknown + 8,
	transError_NeedTCP					= transError_Unknown + 9,
	transError_NotAvailable				= transError_Unknown + 10,
	transError_DataTimedOut				= transError_Unknown + 11,
	transError_AddressInUse				= transError_Unknown + 12
};



/*
 * Stack TTransport
 */

class StTransport
{
	public:
		StTransport()									{	mRef = nil;										}
		void New(Uint32 inProtocol, bool inRegular = true, Uint32 inOptions = 0) 	{	mRef = UTransport::New(inProtocol, inRegular, inOptions);	}
		~StTransport()									{	try {	UTransport::Dispose(mRef);	} catch(...){ /* don't throw */	}					}
		operator TTransport()							{	return mRef;									}
		TTransport operator->() const					{	return mRef;									}
		bool IsValid()									{	return mRef != nil;								}
		bool IsInvalid()								{	return mRef == nil;								}

	private:
		TTransport mRef;
};


/*
 * UTransport Object Interface
 */

class TTransportObj
{
	public:
		void SetDataMonitor(TTransportMonitorProc inProc)													{	UTransport::SetDataMonitor(this, inProc);											}
		bool IsRegular()																					{	return UTransport::IsRegular(this);													}
		
		void SetMessageHandler(TMessageProc inProc, void *inContext = nil)									{	UTransport::SetMessageHandler(this, inProc, inContext);								}
		void PostMessage(Uint32 inMsg, const void *inData = nil, Uint32 inDataSize = 0, Int16 inPriority = priority_Normal)			{	UTransport::PostMessage(this, inMsg, inData, inDataSize, inPriority);		}
		void ReplaceMessage(Uint32 inMsg, const void *inData = nil, Uint32 inDataSize = 0, Int16 inPriority = priority_Normal)		{	UTransport::ReplaceMessage(this, inMsg, inData, inDataSize, inPriority);	}

		void SetIPAddress(SIPAddress inIPAddress)															{	UTransport::SetIPAddress(this, inIPAddress);										}
		SIPAddress GetIPAddress()																			{	return UTransport::GetIPAddress(this);												}
		Uint32 GetRemoteAddressText(void *outAddr, Uint32 inMaxSize)										{	return UTransport::GetRemoteAddressText(this, outAddr, inMaxSize);					}
		Uint32 GetLocalAddressText(void *outAddr, Uint32 inMaxSize)											{	return UTransport::GetLocalAddressText(this, outAddr, inMaxSize);					}
		Uint32 GetLocalAddress(void *outAddr, Uint32 inMaxSize)												{	return UTransport::GetLocalAddress(this, outAddr, inMaxSize);						}
		
		bool IsConnected()																					{	return UTransport::IsConnected(this);												}
		void StartConnect(const void *inAddr, Uint32 inAddrSize, Uint32 inMaxSecs = 0)						{	UTransport::StartConnect(this, inAddr, inAddrSize, inMaxSecs);						}
		void StartConnectThruFirewall(const void *inAddr, Uint32 inAddrSize, const void *inFirewallAddr, Uint32 inFirewallAddrSize, Uint32 inMaxSecs = 0, Uint32 inOptions = 0)	{	UTransport::StartConnectThruFirewall(this, inAddr, inAddrSize, inFirewallAddr, inFirewallAddrSize, inMaxSecs, inOptions);	}
		Uint16 GetConnectStatus()																			{	return UTransport::GetConnectStatus(this);											}
		void Disconnect()																					{	UTransport::Disconnect(this);														}
		void StartDisconnect()																				{	UTransport::StartDisconnect(this);													}
		bool IsDisconnecting()																				{	return UTransport::IsDisconnecting(this);											}
		bool IsConnecting()																					{	return UTransport::IsConnecting(this);												}

		void Listen(const void *inAddr, Uint32 inAddrSize)													{	UTransport::Listen(this, inAddr, inAddrSize);										}
		TTransport Accept(void *outAddr = nil, Uint32 *ioAddrSize = nil)									{	return UTransport::Accept(this, outAddr, ioAddrSize);								}
		
		void SendBuffer(void *inBuf)																		{	UTransport::SendBuffer(this, inBuf);												}
		void Send(const void *inData, Uint32 inDataSize)													{	UTransport::Send(this, inData, inDataSize);											}
		bool ReceiveBuffer(void*& outBuf, Uint32& outSize)													{	return UTransport::ReceiveBuffer(this, outBuf, outSize);							}
		Uint32 Receive(void *outData, Uint32 inMaxSize)														{	return UTransport::Receive(this, outData, inMaxSize);								}
		Uint32 GetReceiveSize()																				{	return UTransport::GetReceiveSize(this);											}
		Uint32 GetUnsentSize()																				{	return UTransport::GetUnsentSize(this);												}
		void NotifyReadyToReceive(Uint32 inReceivedSize)													{	UTransport::NotifyReadyToReceive(this, inReceivedSize);								}
		void NotifyReadyToSend()																			{	UTransport::NotifyReadyToSend(this);												}
		bool IsReadyToReceive(Uint32 inReceivedSize)														{	return UTransport::IsReadyToReceive(this, inReceivedSize);							}
		bool IsReadyToSend()																				{	return UTransport::IsReadyToSend(this);												}

		void SendUnit(const void *inAddr, Uint32 inAddrSize, const void *inData, Uint32 inDataSize)			{	UTransport::SendUnit(this, inAddr, inAddrSize, inData, inDataSize);					}
		bool ReceiveUnit(void *outAddr, Uint32& ioAddrSize, void *outData, Uint32& ioDataSize)				{	return UTransport::ReceiveUnit(this, outAddr, ioAddrSize, outData, ioDataSize);		}

		void operator delete(void *p)																		{	UTransport::Dispose((TTransport)p);													}
	protected:
		TTransportObj() {}				// force creation via UTransport
};

