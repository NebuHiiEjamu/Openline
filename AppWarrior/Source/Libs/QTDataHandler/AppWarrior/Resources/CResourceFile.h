/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#ifndef _H_CResourceFile_
#define _H_CResourceFile_

#include "CResourceException.h"
#include "CFlattenable.h"
#include "CFSLocalFileRef.h"

#if PRAGMA_ONCE
	#pragma once
#endif
			   
HL_Begin_Namespace_BigRedH

class CResourceList;

class CResourceFile
{
	public:
										CResourceFile
												( const CFSLocalFileRef &inRef );
											// throws ???
											
										~CResourceFile();	
											// throws nothing

		typedef FourCharCode			Type;
		static const Type				kResourceList;
		static const Type				kString;
		static const Type				kPNG;

		CRefObj<CFlattenable>&			GetResourceObject
												( Type inType
												, UInt32 inID );
											// throws CResourceException
											
											// conveniance functions
											// based on GetResourceObject
		CString 						GetResourceString( UInt32 inID );
											// throws CResourceException
		CRefObj<CFlattenable>&			GetResourceList( UInt32 inID );
											// throws CResourceException
	private:
		// for now the resources are stored in folders on disk
		CString						mFolderName;

		class ItemID
		{
			public:
										ItemID( Type inType, UInt32 inID)
											// throws nothing
											: mType(inType), mID(inID)  
											{}
				bool					IsLessThan(const ItemID& inID) const
											// throws nothing
											{
												if (mType == inID.mType)
													return mID < inID.mID;
												else	
													return mType < inID.mType;
											}
			private:
				Type 					mType;
				UInt32 					mID;
		};
		
		class Item : public CRefObj<CFlattenable>
		{
			public:
										Item( CResourceFile& inParent
											, Type inType
											, UInt32 inID );
											// throws CResourceException
				CFlattenable*			GetSibling()
											// throw nothing
											{ return mFlattenable.get(); }
				const ItemID&			GetID() const
											// throw nothing
											{ return mID; }					
			private:
				CResourceFile& 			mParent;
				ItemID 					mID;
				std::auto_ptr<CFlattenable>		
										mFlattenable;
		};

		struct ItemLess
		{
			bool						operator()( const ItemID * const & x
									  			  , const ItemID * const & y) 
									  			  const
											// throw nothing
											{ return x->IsLessThan(*y); }
		};
		
		class ObjPool : public std::map<const ItemID*,Item*,ItemLess>
		{
			public:
										~ObjPool();
											// throw nothing
		};

		ObjPool							mObjectPool;
		CMutex							mLock;

		std::auto_ptr<CFlattenable>		CreateResourceObject
												( Type inType
												, UInt32 inID );
											// throws CResourceException
		// Item needs access to CreateResourceObject
		friend class 					Item;
};

HL_End_Namespace_BigRedH
#endif // _H_CResourceFile_
