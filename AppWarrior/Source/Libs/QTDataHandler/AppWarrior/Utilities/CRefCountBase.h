// ===========================================================================
//  CRefCountBase.h 	                       (c) 2000 Hotline Communications
// ===========================================================================
#ifndef _H_CRefCountBase_
#define _H_CRefCountBase_

#include "CListener.h"
#include "CPersistenceException.h"

#if PRAGMA_ONCE
	#pragma once
#endif
			   
HL_Begin_Namespace_BigRedH

// ===========================================================================
//  CRefCountBase                              (c) 2000 Hotline Communications
// ===========================================================================

class CRefCountBase	
{
	public:
		virtual			~CRefCountBase()
							// throws nothing
							{}
		virtual void 	AddRef() = 0;
							// throws ???
		virtual void	Release()= 0; 
							// throws ???
};

// ===========================================================================
//  COwnedRefCount                             (c) 2000 Hotline Communications
// ===========================================================================

template <class Owner>
class TOwnedRefCount : public CRefCountBase	
{				
	public:
						TOwnedRefCount() 
							// throws nothing
							: mRefCount(0)
							, mOwner(nil)
							{}
		virtual			~TOwnedRefCount()
							// throws nothing
							{ ASSERT(mRefCount==0);	}

		void			SetOwner(Owner& inOwner)
							// throws nothing
							{ mOwner = &inOwner; }				

		UInt32			GetRefCount() const
							// throws nothing
							{ return mRefCount; }

		virtual void 	AddRef() 
							// throws nothing
							{ mRefCount++; }

		virtual void	Release();
							// throws nothing
	private:
		UInt32 		mRefCount;	
		Owner*		mOwner;
};

// ---------------------------------------------------------------------
//  Release			                                        	[public]
// ---------------------------------------------------------------------
// Decrements the reference count. When the count reaches 0 notify the 
// owner that this object is no longer referenced.

template <class Owner>
void	
TOwnedRefCount<Owner>::Release() 
{ 
	if (--mRefCount == 0) 
	{
		if (mOwner != nil)
		{
			mOwner->ObjectNoLongerReferenced(*this); 
		}	
		else // it's not assigned to any owner yet ...
		{ 
			delete this; // acts as a stack class
		} 
	}	
}



// ===========================================================================
//  TRefCount 		                           (c) 2000 Hotline Communications
// ===========================================================================
// reference counted object pointer
// points to a CRefCountBase that can be somehow casted to a T

template <class T>
class TRefCount
{
	public:
					TRefCount()
						// throws nothing
						: mRef(nil)
						, mQuick(nil)
						{}

					TRefCount(CRefCountBase& inObject)
						// throws nothing
						: mRef(&inObject)
						, mQuick(nil)
						{ AddRef(); }

					TRefCount(CRefCountBase* inObject)
						// throws nothing
						: mRef(inObject)
						, mQuick(nil)
						{ AddRef(); }

					TRefCount(const TRefCount& inOther)
						// throws nothing
						: mRef(inOther.mRef)
						, mQuick(nil)
						{ AddRef(); }

					~TRefCount()
						// throws nothing
						{ try { Release(); } catch (...) {} }

		TRefCount&	operator = (CRefCountBase& inObject)
						// throws nothing
						{	// avoid self assignement to release the reference
							if (&inObject != mRef)
							{   // release the old mRef and replace with the new one
								Release(); 
								mRef = &inObject;
								AddRef();
							}	 
							return *this;
						}

		TRefCount&	operator = (const TRefCount& inOther)
						// throws nothing
						{	// avoid self assignement to release the reference
							if (inOther.mRef != mRef)
							{   // release the old mRef and replace with the new one
								Release(); 
								mRef = inOther.mRef;
								AddRef();
							}	 
							return *this;
						}
						
		T* 			operator -> () 
						// throws nothing
						{ ASSERT(mQuick!=nil); return mQuick; }

					operator T& () const
						// throws nothing
						{ ASSERT(mQuick!=nil); return *mQuick; }

					operator T* () const
						// throws nothing
						{ ASSERT(mQuick!=nil); return mQuick; }

		T*			GetPtr() const				
						// throws nothing
						{ ASSERT(mQuick!=nil); return mQuick; }

		bool		operator ! () const
						// throws nothing
						{ return mQuick!=nil; }			

		bool		IsValid()
						// throws ???
						{ return (mQuick!=nil)&&(mQuick->IsValid()); }				
						
		void		Release()
						// throws nothing
						{
							if (mRef != nil)
							{
								CRefCountBase* tmp = mRef; // don't remove this
								mRef   = nil;		 // you want mRef to
								mQuick = nil;		 // be nil at the time
								tmp->Release(); 	 // Release is called	
							}	
						}						
	protected:

		void 		AddRef()
						// throws nothing
						{ 
							if (mRef != nil)
							{
								mRef->AddRef(); 
								mQuick = dynamic_cast <T*> (mRef);
								// we should have the object here
								ASSERT(mQuick != nil);

								if (mQuick == nil)
									THROW_UNKNOWN_PERSISTENCE_(eCreateObject);
							}	
						}
	private:
		CRefCountBase*	mRef;
		T*				mQuick; // mQuick = dynamic_cast <T*> (mRef)
};


// ===========================================================================
//  TListenerRef		                       (c) 2000 Hotline Communications
// ===========================================================================
// a TRef that can listen to object creation messages
class CFSFileRef;

template <class T, class Parent>
class TListenerRefCount : public TRefCount<T>
					    , public CListener
{
	public:
					TListenerRefCount(Parent& inParent)
						// throws nothing
						: mParent(inParent)	
						{}
					TListenerRefCount(Parent& inParent, T& inObject)
						// throws nothing
						: TRefCount(inObject)
						, mParent(inParent)	
						{}
					TListenerRefCount(TListenerRefCount& inOther)
						// throws nothing
						: TRefCount(inOther)
						, mParent(inOther.mParent)	
						{}

		void 		LoadFrom(CFSFileRef& inRef)
						// throws ???
						{
							mRef = std::auto_ptr<CFSFileRef> (
									static_cast<CFSFileRef*> (
									 inRef.Clone().release() ));
							mRef->AddListener(*this);
							mRef->LoadObject();
						}
	protected: 
						// CListener::ListenToMessage override
		void		ListenToMessage( const CMessage &inMessage );
						// throws CMessageException
	private:
		Parent&						mParent;
		std::auto_ptr<CFSFileRef>	mRef;
};


// ---------------------------------------------------------------------
//  ListenToMessage												[public]
// ---------------------------------------------------------------------
// listens to object created notification from CFSFileRef

template <class T, class Parent>
void
TListenerRefCount<T,Parent>::ListenToMessage( const CMessage &inMessage )
{
	try
	{
		if (inMessage.GetID() == CFSFileRef::eObject)
		{
			const CFSFileRef::Message* msg 
					= static_cast< const CFSFileRef::Message* > (&inMessage);
			if (mRef.IsSourceFor(*msg))
			{
				mRef->RemoveListener(*this);
				if (msg->GetMsg()) // successfull load
				{
					SetRef(mRef->GetLoadedObject());
					mParent.ObjectAvailableNotification(*this);
				}
			}	
		}	
	}
	catch (...) { RETHROW_MESSAGE_(eListenToMessage); }
}				

HL_End_Namespace_BigRedH
#endif // _H_CRefCountBase_
