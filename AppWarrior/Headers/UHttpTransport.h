/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "URegularTransport.h"

/*
 * Types
 */

typedef class THttpTransportObj *THttpTransport;


/*
 * UHttpTransport
 */

class UHttpTransport
{
	public:	
		// initialize
		static void Init();
	
		// new, dispose, properties
		static THttpTransport New(Uint32 inProtocol, Uint32 inOptions = 0);
		static void Dispose(THttpTransport inTpt);
		static void SetDataMonitor(THttpTransport inTpt, TTransportMonitorProc inProc);
		
		// messaging
		static void SetMessageHandler(THttpTransport inTpt, TMessageProc inProc, void *inContext = nil, void *inObject = nil);
		static void PostMessage(THttpTransport inTpt, Uint32 inMsg, const void *inData = nil, Uint32 inDataSize = 0, Int16 inPriority = priority_Normal);
		static void ReplaceMessage(THttpTransport inTpt, Uint32 inMsg, const void *inData = nil, Uint32 inDataSize = 0, Int16 inPriority = priority_Normal);

		// addresses
		static void SetIPAddress(THttpTransport inTpt, SIPAddress inIPAddress);
		static SIPAddress GetIPAddress(THttpTransport inTpt);
		static Uint32 GetRemoteAddressText(THttpTransport inTpt, void *outAddr, Uint32 inMaxSize);
		static Uint32 GetLocalAddressText(THttpTransport inTpt, void *outAddr, Uint32 inMaxSize);
		static Uint32 GetLocalAddress(THttpTransport inTpt, void *outAddr, Uint32 inMaxSize);
	
		// connecting and disconnecting
		static bool IsConnected(THttpTransport inTpt);
		static void StartConnect(THttpTransport inTpt, const void *inAddr, Uint32 inAddrSize, Uint32 inMaxSecs = 0);
		static void StartConnectThruFirewall(THttpTransport inTpt, const void *inAddr, Uint32 inAddrSize, const void *inFirewallAddr, Uint32 inFirewallAddrSize, Uint32 inMaxSecs = 0, Uint32 inOptions = 0);
		static Uint16 GetConnectStatus(THttpTransport inTpt);
		static void Disconnect(THttpTransport inTpt);
		static void StartDisconnect(THttpTransport inTpt);
		static bool IsDisconnecting(THttpTransport inTpt);
		static bool IsConnecting(THttpTransport inTpt);

		// incoming connection requests
		static void Listen(THttpTransport inTpt, const void *inAddr, Uint32 inAddrSize);
		static THttpTransport Accept(THttpTransport inTpt, void *outAddr = nil, Uint32 *ioAddrSize = nil);
		
		// send and receive data
		static void SendBuffer(THttpTransport inTpt, void *inBuf);
		static void Send(THttpTransport inTpt, const void *inData, Uint32 inDataSize);
		static bool ReceiveBuffer(THttpTransport inTpt, void*& outBuf, Uint32& outSize);
		static Uint32 Receive(THttpTransport inTpt, void *outData, Uint32 inMaxSize);
		static Uint32 GetReceiveSize(THttpTransport inTpt);
		static Uint32 GetUnsentSize(THttpTransport inTpt);
		static void NotifyReadyToReceive(THttpTransport inTpt, Uint32 inReceivedSize);
		static void NotifyReadyToSend(THttpTransport inTpt);
		static bool IsReadyToReceive(THttpTransport inTpt, Uint32 inReceivedSize);
		static bool IsReadyToSend(THttpTransport inTpt);
		
		// connectionless send/receive data
		static void SendUnit(THttpTransport inTpt, const void *inAddr, Uint32 inAddrSize, const void *inData, Uint32 inDataSize);
		static bool ReceiveUnit(THttpTransport inTpt, void *outAddr, Uint32& ioAddrSize, void *outData, Uint32& ioDataSize);
};


/*
 * UHttpTransport Object Interface
 */

class THttpTransportObj
{
	public:
		void SetDataMonitor(TTransportMonitorProc inProc)													{	UHttpTransport::SetDataMonitor(this, inProc);											}
		
		void SetMessageHandler(TMessageProc inProc, void *inContext = nil, void *inObject = nil)			{	UHttpTransport::SetMessageHandler(this, inProc, inContext, inObject);					}
		void PostMessage(Uint32 inMsg, const void *inData = nil, Uint32 inDataSize = 0, Int16 inPriority = priority_Normal)			{	UHttpTransport::PostMessage(this, inMsg, inData, inDataSize, inPriority);		}
		void ReplaceMessage(Uint32 inMsg, const void *inData = nil, Uint32 inDataSize = 0, Int16 inPriority = priority_Normal)		{	UHttpTransport::ReplaceMessage(this, inMsg, inData, inDataSize, inPriority);	}

		void SetIPAddress(SIPAddress inIPAddress)															{	UHttpTransport::SetIPAddress(this, inIPAddress);										}
		SIPAddress GetIPAddress()																			{	return UHttpTransport::GetIPAddress(this);												}
		Uint32 GetRemoteAddressText(void *outAddr, Uint32 inMaxSize)										{	return UHttpTransport::GetRemoteAddressText(this, outAddr, inMaxSize);					}
		Uint32 GetLocalAddressText(void *outAddr, Uint32 inMaxSize)											{	return UHttpTransport::GetLocalAddressText(this, outAddr, inMaxSize);					}
		Uint32 GetLocalAddress(void *outAddr, Uint32 inMaxSize)												{	return UHttpTransport::GetLocalAddress(this, outAddr, inMaxSize);						}
		
		bool IsConnected()																					{	return UHttpTransport::IsConnected(this);												}
		void StartConnect(const void *inAddr, Uint32 inAddrSize, Uint32 inMaxSecs = 0)						{	UHttpTransport::StartConnect(this, inAddr, inAddrSize, inMaxSecs);						}
		void StartConnectThruFirewall(const void *inAddr, Uint32 inAddrSize, const void *inFirewallAddr, Uint32 inFirewallAddrSize, Uint32 inMaxSecs = 0, Uint32 inOptions = 0)	{	UHttpTransport::StartConnectThruFirewall(this, inAddr, inAddrSize, inFirewallAddr, inFirewallAddrSize, inMaxSecs, inOptions);	}
		Uint16 GetConnectStatus()																			{	return UHttpTransport::GetConnectStatus(this);											}
		void Disconnect()																					{	UHttpTransport::Disconnect(this);														}
		void StartDisconnect()																				{	UHttpTransport::StartDisconnect(this);													}
		bool IsDisconnecting()																				{	return UHttpTransport::IsDisconnecting(this);											}
		bool IsConnecting()																					{	return UHttpTransport::IsConnecting(this);												}

		void Listen(const void *inAddr, Uint32 inAddrSize)													{	UHttpTransport::Listen(this, inAddr, inAddrSize);										}
		THttpTransport Accept(void *outAddr = nil, Uint32 *ioAddrSize = nil)									{	return UHttpTransport::Accept(this, outAddr, ioAddrSize);							}
		
		void SendBuffer(void *inBuf)																		{	UHttpTransport::SendBuffer(this, inBuf);												}
		void Send(const void *inData, Uint32 inDataSize)													{	UHttpTransport::Send(this, inData, inDataSize);											}
		bool ReceiveBuffer(void*& outBuf, Uint32& outSize)													{	return UHttpTransport::ReceiveBuffer(this, outBuf, outSize);							}
		Uint32 Receive(void *outData, Uint32 inMaxSize)														{	return UHttpTransport::Receive(this, outData, inMaxSize);								}
		Uint32 GetReceiveSize()																				{	return UHttpTransport::GetReceiveSize(this);											}
		Uint32 GetUnsentSize()																				{	return UHttpTransport::GetUnsentSize(this);												}
		void NotifyReadyToReceive(Uint32 inReceivedSize)													{	UHttpTransport::NotifyReadyToReceive(this, inReceivedSize);								}
		void NotifyReadyToSend()																			{	UHttpTransport::NotifyReadyToSend(this);												}
		bool IsReadyToReceive(Uint32 inReceivedSize)														{	return UHttpTransport::IsReadyToReceive(this, inReceivedSize);							}
		bool IsReadyToSend()																				{	return UHttpTransport::IsReadyToSend(this);												}

		void SendUnit(const void *inAddr, Uint32 inAddrSize, const void *inData, Uint32 inDataSize)			{	UHttpTransport::SendUnit(this, inAddr, inAddrSize, inData, inDataSize);					}
		bool ReceiveUnit(void *outAddr, Uint32& ioAddrSize, void *outData, Uint32& ioDataSize)				{	return UHttpTransport::ReceiveUnit(this, outAddr, ioAddrSize, outData, ioDataSize);		}

		void operator delete(void *p)																		{	UHttpTransport::Dispose((THttpTransport)p);												}
	protected:
		THttpTransportObj() {}				// force creation via UHttpTransport
};

