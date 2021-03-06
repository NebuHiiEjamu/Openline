/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
// base class for synchronous streams

#ifndef _H_CSyncStreamBuffer_
#define _H_CSyncStreamBuffer_

#include "CStreamBuffer.h"

#if PRAGMA_ONCE
	#pragma once
#endif
			   
HL_Begin_Namespace_BigRedH

class CSyncStreamBuffer : public CStreamBuffer
{
	public:
								// always fail
		virtual void		AddListener( CListener &inListener )
								// throws nothing
								{ ASSERT(false); } 

								// you always can write, it just 
								// happens to block when you do
		virtual bool		CanWriteSomeMore()	
								// throws nothing
								{ return true; }

								// always fail, you cannot expect 
								// a certain amount of bytes from a blocking call
		virtual void 		SetExpect(UInt32 inSize)   
								// throws nothing
								{ ASSERT(false); } 
};

HL_End_Namespace_BigRedH
#endif // _H_CSyncStreamBuffer_
