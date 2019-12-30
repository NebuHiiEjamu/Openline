/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#ifndef _H_CSyncTCPIOStream_
#define _H_CSyncTCPIOStream_

#include "CIOStream.h"
#include "CSyncTCPStreamBuffer.h"

#if PRAGMA_ONCE
	#pragma once
#endif
			   
HL_Begin_Namespace_BigRedH

class CSyncTCPIOStream : public CIOStream
{
	public:
					// ** Construction **
								CSyncTCPIOStream( UInt16 inTimeoutSec )
									// throws ???
									: CIOStream(mBuf), mBuf(inTimeoutSec)
									{}

					// ** Protocol **
		UInt16					GetTimeout() const
									// throws nothing
									{ return mBuf.GetTimeout(); }
		void					SetTimeout( UInt16 inTimeoutSec )
									// throws nothing
									{ mBuf.SetTimeout(inTimeoutSec); }

		void 					Open(const CString& inName)
									// throws CStreamException
									{ 
										CNetAddress a(inName, 0);
										mBuf.Open(a);
									}
		void 					Open( const CNetAddress& inRemoteAddress )
									// throws CStreamException
									{ 
										mBuf.Open(inRemoteAddress);
									}
	private:
		CSyncTCPStreamBuffer 	mBuf;
};

HL_End_Namespace_BigRedH

#endif // _H_CSyncTCPIOStream_
