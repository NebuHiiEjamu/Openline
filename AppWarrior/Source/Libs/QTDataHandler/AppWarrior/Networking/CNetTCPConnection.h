// =====================================================================
//  CNetTCPConnection.h                  (C) Hotline Communications 2000
// =====================================================================
// Implements TCP connection operations for Windows & Linux
// UNIX will be tested soon :^))

#ifndef _H_CNetTCPConnection_
#define _H_CNetTCPConnection_

#if PRAGMA_ONCE
	#pragma once
#endif

#include "CNetConnection.h"

HL_Begin_Namespace_BigRedH

class CNetAddress;

class CNetTCPConnection : public CNetConnection
{
	public:
								CNetTCPConnection();
								// throws CMemoryException
								CNetTCPConnection( const CNetAddress& inLocalAddress );
								// throws CMemoryException
		virtual				~CNetTCPConnection();
	
		// ** Connection **
		virtual bool		Connect( const CNetAddress& inRemoteAddress );
								// throws CNetworkException
		virtual bool		IsConnected();
								// throws nothing
		virtual void		Disconnect();
								// throws nothing

		// ** Data Transfer **
		virtual UInt32		GetAvailable();
								// throws CNetworkException
		virtual UInt32		Read( UInt8 *inBuffer,
										UInt32 inMaxSize,
										UInt16 inTimeoutSec = 0 );
								// throws CNetworkException
		virtual UInt32		Write( UInt8 *inBuffer,
										 UInt32 inMaxSize,
										 UInt16 inTimeoutSec = 0 );
								// throws CNetworkException

	private:
		friend class CNetTCPListener;

		class CNetConnectionPS;
		CNetConnectionPS *mPlatform;
};

HL_End_Namespace_BigRedH

#endif // _H_CNetTCPConnection_