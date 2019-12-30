/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
// the way a CMemoryStreamBuffer is used

#ifndef _H_CPipe_
#define _H_CPipe_

#include "CIOStream.h"
#include "CMemoryStreamBuffer.h"

#if PRAGMA_ONCE
	#pragma once
#endif
			   
HL_Begin_Namespace_BigRedH

class CPipe : public CIOStream
{
	public:
								CPipe()
									// throws CStreamException
									: CIOStream(mBuf)
									{}
		void					Open()
									// throws CStreamException
									{ mBuf.Open(); }
	private:
		CMemoryStreamBuffer 	mBuf;
}; // class CPipe

HL_End_Namespace_BigRedH
#endif // _H_CPipe_
