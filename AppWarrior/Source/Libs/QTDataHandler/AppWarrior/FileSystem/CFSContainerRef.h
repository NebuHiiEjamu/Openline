/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
// A reference to a file system item like a file, folder, whatever else

#ifndef _H_CFSContainerRef_
#define _H_CFSContainerRef_

#include "CFSRef.h"

#if PRAGMA_ONCE
	#pragma once
#endif
			   
HL_Begin_Namespace_BigRedH

class CFSContainerRef : public CFSRef
{
	public:
		virtual bool				IsContainer() const
										// throws nothing
										{ return true; }

		class Content : public std::list<CFSRef*>
		{
			public:
									Content();
										// throws ???
									~Content();
										// throws nothing
		};
		
		enum 						NotifyID
										{ 
											eLoaded = 'fDir' 
										  , eCreated= 'fCre'	
										};
		
		virtual void 				Create() = 0;
										// throws CFSException
		virtual void 				LoadContent() = 0;
										// throws CFSException
		virtual const Content&		GetContent() = 0;
										// throws CFSException
}; // class CFSContainerRef

HL_End_Namespace_BigRedH
#endif // _H_CFSContainerRef_