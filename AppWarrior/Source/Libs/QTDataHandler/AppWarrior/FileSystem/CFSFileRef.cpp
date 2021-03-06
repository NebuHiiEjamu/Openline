/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
// The generic file reference helper functions

#include "AW.h"
#include "CFSFileRef.h"
#include "CMemoryObjectCache.h"

HL_Begin_Namespace_BigRedH

// ---------------------------------------------------------------------
//  LoadObject	                                       			[public]
// ---------------------------------------------------------------------
// ask the memory cache to load the object this FSRef referes to

void
CFSFileRef::LoadObject()
{
	try
	{
		CMemoryObjectCache::Instance().AddListener(*this);
		CMemoryObjectCache::Instance().LoadObject(*this);
	}
	catch (...) { RETHROW_FS_(eLoadObject,GetName()); }
}

// ---------------------------------------------------------------------
//  GetObject	                                       			[public]
// ---------------------------------------------------------------------
// ask the memory cache to return the object this FSRef referes to

CRefObj<CFlattenable>&
CFSFileRef::GetLoadedObject()
{
	try 
	{
		return CMemoryObjectCache::Instance().GetLoadedObject(*this);
	}
	catch (...) 
	{ 
		RETHROW_FS_(eLoadObject,GetName()); 
		return *(CRefObj<CFlattenable>*) nil; // stupid compiler
	}
}

// ---------------------------------------------------------------------
//  ListenToMessage                                    		 [protected]
// ---------------------------------------------------------------------
// listen to notifications from the memory cache

void
CFSFileRef::ListenToMessage( const CMessage &inMessage )
{
	try 
	{
		// is it the right type of message
		if (inMessage.GetID() == CMemoryObjectCache::eObject)
		{
			const CMemoryObjectCache::Message* msg = 
				static_cast <const CMemoryObjectCache::Message*> (&inMessage);
			// is the message from whom we expect it to be
			// the FSFileRef is owned by this message so we can use it's methods
			if ( msg->GetFSFileRef() == *this )
			{
				ASSERT(msg->VerifySource(CMemoryObjectCache::Instance()));
				CMemoryObjectCache::Instance().RemoveListener(*this);
				// resend the message to our own listener
				Notify(eObject,msg->GetSuccess());
			}
		}
	}
	catch (...) { RETHROW_MESSAGE_(eListenToMessage); }
		
}


HL_End_Namespace_BigRedH
