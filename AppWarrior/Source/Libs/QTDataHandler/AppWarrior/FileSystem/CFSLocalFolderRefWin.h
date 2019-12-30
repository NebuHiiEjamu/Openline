/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
// A reference to a local file system folder

#ifndef _H_CFSLocalFolderRefWin_
#define _H_CFSLocalFolderRefWin_

#include "CFSLocalFolderRef.h"

#if PRAGMA_ONCE
	#pragma once
#endif
			   
HL_Begin_Namespace_BigRedH

class CFSLocalFolderRefWin : public CFSLocalFolderRef
{
	public:
									CFSLocalFolderRefWin
										// throws ???
										(const CString& inFolderName);
									CFSLocalFolderRefWin
										// throws ???
										( const CString& inFolderName
										, UInt32 inAttr);
									CFSLocalFolderRefWin
										// throws ???
										(const CFSLocalFolderRefWin& inOther);

		virtual	std::auto_ptr<CFSRef> Clone() const;
										// throws CFSException

		virtual const CString& 	GetName() const;
										// throws nothing

		virtual bool				IsEqual(const CFSRef& inOther) const;
										// throws nothing
		virtual bool				IsLessThan(const CFSRef& inOther) const;
										// throws nothing

		virtual void				LoadRights();
										// throws CFSException
		virtual CFileRights&		GetRights();
										// throws CFSException
//		virtual void				SaveRights();

		virtual void 				Create();
										// throws CFSException
		virtual void 				Delete();
										// throws CFSException
		
		virtual void 				LoadContent();
										// throws CFSException
		virtual const Content&		GetContent();
										// throws CFSException
	protected:
		void						InitRightsFrom(UInt32 inAttr);
										// throws CException
	private:
		Content						mContent;
		CString					mFolderName;
		CFileRights					mRights;

		bool						mContentLoaded;
		bool						mRightsLoaded;
}; // class CFSLocalFolderRefWin 

HL_End_Namespace_BigRedH
#endif // _H_CFSLocalFolderRefWin_