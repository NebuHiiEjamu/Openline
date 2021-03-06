/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "AW.h"
#include "CStreamBuffer.h"
#include "CMessage.h"
#include "CAsyncStreamBuffer.h"

HL_Begin_Namespace_BigRedH

// ---------------------------------------------------------------------
//  PutWithEndianOrdering                              			[public]
// ---------------------------------------------------------------------
// helper for shift operations, puts a block of data to the stream
// fails if the operation cannot be completed at once
// takes care of the byte ordering issues

void
CStreamBuffer::PutWithEndianOrdering(const void* inBuf, UInt32 inSize)
{
	UInt8 reserve[1024];
	UInt8* local;
	if (IsOrderReversed())
	{
		local = reserve;
		ArrangeOrder(local, (const UInt8*)inBuf, inSize);
	}
	else
		local = (UInt8*) inBuf;
		
	PutAll(local,inSize);
}


// ---------------------------------------------------------------------
//  GetWithEndianOrdering                              			[public]
// ---------------------------------------------------------------------
// helper for shift operations, gets a block of data from the stream
// fails if the operation cannot be completed at once
// takes care of the byte ordering issues

void
CStreamBuffer::GetWithEndianOrdering(void* inBuf, UInt32 inSize)
{
	GetAll(inBuf, inSize);
	ArrangeOrder((UInt8*)inBuf, inSize);
}


// ---------------------------------------------------------------------
//  Put			                                     			[public]
// ---------------------------------------------------------------------
// helper for shift operations, puts a block of data to the stream
// fails if the operation cannot be completed at once

void
CStreamBuffer::PutAll(const void* inBuf, UInt32 inSize)
{
	if (inSize != Write(inBuf,inSize))
		THROW_UNKNOWN_STREAM_(ePut);
}

// ---------------------------------------------------------------------
//  Get			                                     			[public]
// ---------------------------------------------------------------------
// helper for shift operations, gets a block of data from the stream
// fails if the operation cannot be completed at once

void
CStreamBuffer::GetAll(void* inBuf, UInt32 inSize)
{
	if (inSize != Read(inBuf,inSize))
		THROW_UNKNOWN_STREAM_(eGet);
}

// ---------------------------------------------------------------------
//  IsSourceFor	                                     			[public]
// ---------------------------------------------------------------------
// helper for finding out if this stream is the source of the notification

bool
CStreamBuffer::IsSourceFor(const CMessage& inMsg)
{
	try
	{
		ASSERT(inMsg.GetID()==CAsyncStreamBuffer::eDataRcv
			|| inMsg.GetID()==CAsyncStreamBuffer::eDataSnt 
			|| inMsg.GetID()==CAsyncStreamBuffer::eClosed	);
		const CAsyncStreamBuffer::Message* msg = 
			static_cast<const CAsyncStreamBuffer::Message*> (&inMsg);
		CStreamBuffer* src = &msg->GetSource();
		return src == this;
	}	
	catch (...) 
	{ 
		RETHROW_STREAM_(eIsSource); 
		return false; // stupid compiler
	}
}

HL_End_Namespace_BigRedH
