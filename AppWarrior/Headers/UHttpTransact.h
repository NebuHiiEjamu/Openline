/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "UTransport.h"


/*
 * Constants
 */

enum {
	protocol_HTTP_1_0			= 1,
	protocol_HTTP_1_1			= 2
};

enum {
	port_number_HTTP			= 80
};


/*
 * Types
 */

typedef class THttpTransactObj *THttpTransact;


/*
 * UHttpTransact
 */
 
class UHttpTransact
{
	public:
		// new, dispose
		static THttpTransact New();
		static void Dispose(THttpTransact inTrn);
		
		// messaging
		static void SetMessageHandler(THttpTransact inTrn, TMessageProc inProc, void *inContext = nil);
		static void PostMessage(THttpTransact inTrn, Uint32 inMsg, const void *inData = nil, Uint32 inDataSize = 0, Int16 inPriority = priority_Normal);
		static void ReplaceMessage(THttpTransact inTrn, Uint32 inMsg, const void *inData = nil, Uint32 inDataSize = 0, Int16 inPriority = priority_Normal);
		
		// transport
		static void MakeNewTransport(THttpTransact inTrn);
		static TTransport GetTransport(THttpTransact inTrn);

		// connecting and disconnecting
		static bool IsConnected(THttpTransact inTrn);
		static bool StartConnect(THttpTransact inTrn, const Uint8 *inAddress, Uint8 inProtocol, bool inKeepAlive = false, bool inGetHeader = false, Uint32 inMaxSecs = 0);
		static void Disconnect(THttpTransact inTrn);
		static void StartDisconnect(THttpTransact inTrn);
		static bool IsDisconnecting(THttpTransact inTrn);
		static bool IsComplete(THttpTransact inTrn);
		static bool IsError(THttpTransact inTrn);
		static bool IsNotModified(THttpTransact inTrn);

		// remote computer requests connection
		static void Listen(THttpTransact inTrn, const void *inAddr, Uint32 inAddrSize);
		static THttpTransact Accept(THttpTransact inTrn, void *outAddr = nil, Uint32 *ioAddrSize = nil);
		
		// set/get info
		static bool GetUrl(THttpTransact inTrn, Uint8 *outUrl, Uint32 inMaxSize);
		static bool GetHost(THttpTransact inTrn, Uint8 *outHost, Uint32 inMaxSize);
		static bool GetDomain(THttpTransact inTrn, Uint8 *outDomain, Uint32 inMaxSize);
		static bool GetLocation(THttpTransact inTrn, Uint8 *outLocation, Uint32 inMaxSize);
		static bool GetLicense(THttpTransact inTrn, Uint8 *outLicense, Uint32 inMaxSize);
		static void SetLastModified(THttpTransact inTrn, Uint8 *inLastModified, Uint32 inLength);
		static bool GetLastModified(THttpTransact inTrn, Uint8 *outLastModified, Uint32 inMaxSize);
		static void SetReferer(THttpTransact inTrn, const Uint8 *inReferer);
		static void SetUserAgentServer(THttpTransact inTrn, const Uint8 *inUserAgentServer);
		static void SetCustomField(THttpTransact inTrn, const Uint8 *inCustomField);
		static void SetHttpIDList(THttpTransact inTrn, const CPtrList<Uint8>& inHttpIDList);
		static const CPtrList<Uint8>& GetHttpIDList(THttpTransact inTrn);

		// send/receive data
		static bool SendHttpHeader(THttpTransact inTrn, const void *inData, Uint32 inDataSize, const Int8 *inType, Uint32 inTotalDataSize, bool inLastData = false);
		static bool SendHttpData(THttpTransact inTrn, const void *inData, Uint32 inDataSize, bool inLastData = false);
		static Uint32 GetUnsentSize(THttpTransact inTrn);
		static void *ReceiveHttpData(THttpTransact inTrn, Uint32& outDataSize, Int8 *outType);
		static void PurgeReceiveBuffer(THttpTransact inTrn);
		
		// add/get cookies
		static void *GetHttpCookie(const Uint8 *inHost, const Uint8 *inDomain, Uint32& outDataSize);
		static bool AddHttpCookie(const Uint8 *inHost, const Uint8 *inDomain, const void *inData, Uint32 inDataSize, const SCalendarDate& inExpiredDate);
		static void *GetExternalHttpCookie(const Uint8 *inHost, const Uint8 *inDomain, Uint32& outDataSize);
		static bool AddExternalHttpCookie(const Uint8 *inHost, const Uint8 *inDomain, const void *inData, Uint32 inDataSize, const SCalendarDate& inExpiredDate);

		// write/read cookies
		static bool WriteHttpCookies(TFSRefObj* inFileRef);
		static bool ReadHttpCookies(TFSRefObj* inFileRef);
};


/*
 * Stack THttpTransact
 */

class StHttpTransact
{
	public:
		StHttpTransact()								{	mRef = UHttpTransact::New();					}
		~StHttpTransact()								{	UHttpTransact::Dispose(mRef);					}
		operator THttpTransact()						{	return mRef;									}
		THttpTransact operator->() const				{	return mRef;									}
		bool IsValid()									{	return mRef != nil;								}
		bool IsInvalid()								{	return mRef == nil;								}

	private:
		THttpTransact mRef;
};


/*
 * UHttpTransact Object Interface
 */

class THttpTransactObj
{
	public:
		// messaging
		void SetMessageHandler(TMessageProc inProc, void *inContext = nil)
		{	UHttpTransact::SetMessageHandler(this, inProc, inContext);  }
		
		void PostMessage(Uint32 inMsg, const void *inData = nil, Uint32 inDataSize = 0, Int16 inPriority = priority_Normal)
		{	UHttpTransact::PostMessage(this, inMsg, inData, inDataSize, inPriority); }
		
		void ReplaceMessage(Uint32 inMsg, const void *inData = nil, Uint32 inDataSize = 0, Int16 inPriority = priority_Normal)
		{	UHttpTransact::ReplaceMessage(this, inMsg, inData, inDataSize, inPriority); }

		// transport
		void MakeNewTransport()
		{	UHttpTransact::MakeNewTransport(this); }
		TTransport GetTransport()
		{	return UHttpTransact::GetTransport(this); }

		// connecting and disconnecting		
		bool IsConnected()
		{	return UHttpTransact::IsConnected(this); }
		
		bool StartConnect(const Uint8 *inAddress, Uint8 inProtocol
						 , bool inKeepAlive = false
						 , bool inGetHeader = false
						 , Uint32 inMaxSecs = 0)				
		{	return UHttpTransact::StartConnect(this, inAddress, inProtocol, inKeepAlive, inGetHeader, inMaxSecs);		}
		
		void Disconnect()
		{	UHttpTransact::Disconnect(this); }
		void StartDisconnect()
		{	UHttpTransact::StartDisconnect(this); }
		bool IsDisconnecting()
		{	return UHttpTransact::IsDisconnecting(this); }
		bool IsComplete()
		{	return UHttpTransact::IsComplete(this);	}
		bool IsError()
		{	return UHttpTransact::IsError(this); }
		bool IsNotModified()
		{	return UHttpTransact::IsNotModified(this);	}

		// remote computer requests connection
		void Listen(const void *inAddr, Uint32 inAddrSize)
		{	UHttpTransact::Listen(this, inAddr, inAddrSize);}
		THttpTransact Accept(void *outAddr = nil, Uint32 *ioAddrSize = nil)
		{	return UHttpTransact::Accept(this, outAddr, ioAddrSize); }

		// set/get info
		bool GetUrl(Uint8 *outUrl, Uint32 inMaxSize)
		{	return UHttpTransact::GetUrl(this, outUrl, inMaxSize);																}
		bool GetHost(Uint8 *outHost, Uint32 inMaxSize)
		{	return UHttpTransact::GetHost(this, outHost, inMaxSize);															}
		bool GetDomain(Uint8 *outDomain, Uint32 inMaxSize)
		{	return UHttpTransact::GetDomain(this, outDomain, inMaxSize);														}
		bool GetLocation(Uint8 *outLocation, Uint32 inMaxSize)
		{	return UHttpTransact::GetLocation(this, outLocation, inMaxSize);													}
		bool GetLicense(Uint8 *outLicense, Uint32 inMaxSize)
		{	return UHttpTransact::GetLicense(this, outLicense, inMaxSize);														}
		void SetLastModified(Uint8 *inLastModified, Uint32 inLength)
		{	UHttpTransact::SetLastModified(this, inLastModified, inLength);														}
		bool GetLastModified(Uint8 *outLastModified, Uint32 inMaxSize)
		{	return UHttpTransact::GetLastModified(this, outLastModified, inMaxSize);											}
		void SetReferer(const Uint8 *inReferer)
		{	UHttpTransact::SetReferer(this, inReferer);																			}
		void SetUserAgentServer(const Uint8 *inUserAgentServer)
		{	UHttpTransact::SetUserAgentServer(this, inUserAgentServer);															}
		void SetCustomField(const Uint8 *inCustomField)
		{	UHttpTransact::SetCustomField(this, inCustomField);																	}
		void SetHttpIDList(const CPtrList<Uint8>& inHttpIDList)
		{	UHttpTransact::SetHttpIDList(this, inHttpIDList);																	}
		const CPtrList<Uint8>& GetHttpIDList()
		{	return UHttpTransact::GetHttpIDList(this);																			}

		// send/receive data
		bool SendHttpHeader(const void *inData, Uint32 inDataSize, const Int8 *inType, Uint32 inTotalDataSize, bool inLastData = false)	
		{	return UHttpTransact::SendHttpHeader(this, inData, inDataSize, inType, inTotalDataSize, inLastData);	}
		bool SendHttpData(const void *inData, Uint32 inDataSize, bool inLastData = false)
		{	return UHttpTransact::SendHttpData(this, inData, inDataSize, inLastData);				}
		Uint32 GetUnsentSize()
		{	return UHttpTransact::GetUnsentSize(this);																			}
		void *ReceiveHttpData(Uint32& outDataSize, Int8 *outType)
		{	return UHttpTransact::ReceiveHttpData(this, outDataSize, outType);													}
		void PurgeReceiveBuffer()
		{	UHttpTransact::PurgeReceiveBuffer(this);																			}
		
		void operator delete(void *p)
		{	UHttpTransact::Dispose((THttpTransact)p);																			}
	protected:
		THttpTransactObj() {}				// force creation via UHttpTransact
};


struct SHttpCookie {
	void *pData;
	Uint32 nDataSize;
	Uint8 *psHost;
	Uint8 *psDomain;
	SCalendarDate stExpiredDate;
};

class CCookieList
{
	public:
		CCookieList();
		virtual ~CCookieList();
		
		// add
		bool AddCookie(SHttpCookie *inHttpCookie);
		bool AddCookie(const Uint8 *inHost, const Uint8 *inDomain, const void *inData, Uint32 inDataSize, const SCalendarDate& inExpiredDate);
		
		// get
		void *GetCookie(const Uint8 *inHost, const Uint8 *inDomain, Uint32& outDataSize);
		bool GetNextCookie(SHttpCookie*& outHttpCookie, Uint32& ioIndex);
		
		// dispose
		static void DisposeCookie(SHttpCookie *inHttpCookie);

		// expired date
		static bool IsExpired(SCalendarDate& inExpiredDate);
		static bool ConvertExpiredDate(const Uint8 *inDate, SCalendarDate& outDate);

		// write/read
		bool WriteData(TFSRefObj* inFileRef);
		bool ReadData(TFSRefObj* inFileRef);
		
	private:
		CPtrList<SHttpCookie> mCookieList;
};


