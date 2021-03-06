/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#ifndef _H_CSyncTCPStreamBuffer_
#define _H_CSyncTCPStreamBuffer_

#if PRAGMA_ONCE
	#pragma once
#endif

#include "CSyncStreamBuffer.h"
			   
HL_Begin_Namespace_BigRedH

class CSyncTCPStreamBuffer : public CSyncStreamBuffer
{
	private:

		enum EConst
		{
			eDefTimeout = 5 // 5 secs
		};

	public:
			// ** Construction **
							CSyncTCPStreamBuffer( UInt16 inTimeout = eDefTimeout)
								// throws ???
								: mTimeout( inTimeout )
								{}
							CSyncTCPStreamBuffer( const CNetAddress& inLocalAddress,
												  const CNetAddress& inRemoteAddress,
												  UInt16 inTimeout = eDefTimeout )
								// throws ???
								: mConnection(inLocalAddress), mTimeout(inTimeout)
								{ Open(inRemoteAddress); }
		virtual				~CSyncTCPStreamBuffer();
								// throws nothing
		
			// ** Protocol **
		bool				IsOpen()
								// throws CStreamException
								{ return mConnection.IsConnected(); }

		bool				IsEOF()
								// throws CStreamException
								{ return false; }

		UInt16				GetTimeout() const
								// throws nothing
								{ return mTimeout; }
		void				SetTimeout( UInt16 inTimeout )
								// throws nothing
								{ mTimeout = inTimeout; }

		void 				Open( const CNetAddress& inRemoteAddress );
								// throws CStreamException
		void				Close();
								// throws CStreamException

		UInt32				GetAvailable();
								// throws CStreamException
		UInt32				Read(void* inBuf, UInt32 inSize);
								// throws CStreamException
		UInt32 				Write(const void* inBuf, UInt32 inSize);
								// throws CStreamException
	private:
		CNetTCPConnection	mConnection;
		UInt16				mTimeout;
};


HL_End_Namespace_BigRedH
#endif // _H_CSyncTCPStreamBuffer_
