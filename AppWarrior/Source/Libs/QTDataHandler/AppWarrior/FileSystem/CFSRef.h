/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
// A reference to a file system item like a file, folder, whatever else

#ifndef _H_CFSRef_
#define _H_CFSRef_

#include "CFSException.h"
#include "CBroadcaster.h"
#include "CMessage.h"
#include "CFileRights.h"

#if PRAGMA_ONCE
	#pragma once
#endif
			   
HL_Begin_Namespace_BigRedH

class CFSRef : public CBroadcaster
{
	public:
		virtual 				 		~CFSRef() 
											// throws nothing
											{}

		virtual	std::auto_ptr<CFSRef>	Clone() const = 0;
											// throws CFSException
			//** the name as it is on the local platform **
		virtual const CString& 		GetName() const = 0;
											// throws nothing
		virtual bool			 		IsContainer() const = 0;
											// throws nothing

			// ** check if it's about the same file **
		virtual bool					IsEqual(const CFSRef& inOther) 
											const = 0;	
											// throws nothing
		virtual bool					IsLessThan(const CFSRef& inOther) 
											const = 0;	
											// throws nothing
				bool 					operator == (const CFSRef& inOther) 
											const
											// throws nothing
											{ return IsEqual(inOther); }
				bool 					operator != (const CFSRef& inOther) 
											const
											// throws nothing
											{ return !IsEqual(inOther); }
				bool 					operator <  (const CFSRef& inOther) 
											const
											// throws nothing
											{ return IsLessThan(inOther); }
				bool 					operator <= (const CFSRef& inOther) 
											const
											// throws nothing
											{ return IsLessThan(inOther) 
												  || IsEqual(inOther); }
				bool 					operator >  (const CFSRef& inOther) 
											const
											// throws nothing
											{ return inOther.IsLessThan(*this); }
				bool 					operator >= (const CFSRef& inOther) 
											const
											// throws nothing
											{ return inOther.IsLessThan(*this) 
												  || IsEqual(inOther); }

		virtual void					LoadRights()=0;
											// throws CFSException
		virtual CFileRights&			GetRights()=0;
											// throws CFSException
		virtual void					Delete()=0;
											// throws CFSException

//		virtual void					CopyTo(CFSRef& inOther)=0;
//		virtual void					MoveTo(CFSRef& inOther)=0;
//		virtual void					Rename(CFSRef& inOther)=0;
//		virtual void					MoveToTrash(CFSRef& inOther)=0;
//		virtual void					SetComment()=0;

			// ** ID's and Msg types for notifications **
		enum 							NotifyID
											{
												eRights	= 'fsRg'
											  ,	eWRights= 'fsRw'
											  , eDeleted= 'fDel'
											  , eCopied	= 'fCpy'
											  , eMoved	= 'fMov'
											  , eRenamed= 'fRen'
											  , eTrashed= 'fTsh'
											};
		typedef TObjectMessageS<bool, CFSRef&>	Message; 

	protected:
			// ** helpers for the derived classes **
		void 							Notify( UInt32 inNotifyID
											  , bool inSuccess);
											// throws CMessageException
}; // class CFSRef 

HL_End_Namespace_BigRedH
#endif // _H_CFSRef_