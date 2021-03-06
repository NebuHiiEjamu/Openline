/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
// A reference to a UNC file (Win32 uses that)
// 		          \\computer\sharing\path\file
// or a local file (don't use relative paths)
//				  c:\absolutepath\file

#ifndef _H_CFSLocalFileRefWin_
#define _H_CFSLocalFileRefWin_

#include "CFSLocalFileRef.h"
#include "CString.h"

#if PRAGMA_ONCE
	#pragma once
#endif
			   
HL_Begin_Namespace_BigRedH

class CFSLocalFileRefWin : public CFSLocalFileRef
{

	public:
											CFSLocalFileRefWin
												(const CString& inFileName);
												// throws ???
											CFSLocalFileRefWin
												( const CString& inFileName
												, UInt32 inAttr);
												// throws ???
											CFSLocalFileRefWin
												(const CFSLocalFileRefWin& inOther);
												// throws ???

		virtual	std::auto_ptr<CFSRef> 		Clone() const;

		virtual const CString& 			GetName() const;
												// throws nothing
		virtual bool						IsEqual
												(const CFSRef& inOther) 
												const;
												// throws nothing
		virtual bool						IsLessThan
												(const CFSRef& inOther) 
												const;
												// throws nothing

		virtual void						LoadInputStream();
												// throws CFSException
		virtual std::auto_ptr<CInStream>&	GetInputStream();
												// throws CFSException
		virtual void						LoadOutputStream();
												// throws CFSException
		virtual std::auto_ptr<COutStream>&	GetOutputStream();
												// throws CFSException

		virtual void						LoadType();
												// throws CFSException
		virtual void 						SetFileType
												// throws CFSException
												(const CString& inMime);
		virtual CString					GetFileType();
												// throws CFSException

		virtual void 						LoadSize();
												// throws CFSException
		virtual UInt64 						GetSize();
												// throws CFSException

		virtual void						LoadRights();
												// throws CFSException
		virtual CFileRights&				GetRights();
												// throws CFSException

		virtual void 						Delete();
												// throws CFSException

		CString							GetFileExtension() const;
												// throws ???

	protected:
		bool								GetMimeFromRegistry();
												// throws ???
		void								InitRightsFrom(UInt32 inAttr);
												// throws nothing
	private:
		std::auto_ptr<CInStream>			mInStream;
		std::auto_ptr<COutStream>			mOutStream;
		
		CString							mFileName;
		CString								mMimeType;
		UInt64								mSize;
		CFileRights							mRights;

		bool								mSizeLoaded;
		bool								mTypeLoaded;
		bool								mRightsLoaded;
}; // class CFSLocalFileRefWin

HL_End_Namespace_BigRedH
#endif // _H_CFSLocalFileRefWin_
