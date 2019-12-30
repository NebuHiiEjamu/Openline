// =====================================================================
//	CGenericObject.h                     (C) Hotline Communications 2000
// =====================================================================
// This is a CFlattenable that gets constructed when the mime type of the
// object to be constructed is not one of the known types.

#ifndef _H_CGenericObject_
#define _H_CGenericObject_

#include "CFlattenable.h"

#if PRAGMA_ONCE
	#pragma once
#endif
			   
HL_Begin_Namespace_BigRedH

class CGenericObject : public CFlattenable
{
	public:
							CGenericObject();
								// ???
		virtual				~CGenericObject();
								// throws nothing

	protected:
			//** CFlattenable overrides **
		virtual void 		AttachInStream(std::auto_ptr<CInStream>& inInStream);
								// throws CMessageException
		virtual void		ListenToMessage( const CMessage &inMessage );
								// throws CMessageException
	private:
		void				TryToReadSomeMore();
								// throws ???
};

HL_End_Namespace_BigRedH
#endif // _H_CGenericObject_
