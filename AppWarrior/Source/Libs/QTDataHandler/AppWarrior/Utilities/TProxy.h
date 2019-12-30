// =====================================================================
//  TProxy.h                             (C) Hotline Communications 1999
// =====================================================================
// A design for Proxies where creation, copying and
// destruction must be tracked by the Creator of the Proxy.
// This allows for RefCounting, and potentially any number of other
// creational strategies to be implemented with a proper proxy already
// in place.

#ifndef _H_TProxy_
#define _H_TProxy_

// Forward declaration of TCreator, so the TProxy can refer to it.
template <class T> class TCreator;

#if 0
#pragma mark -
#pragma mark TProxy<T> Declaration
#endif

// This is the Proxy the outside world gets to hang on to.
// It would be bad to allow unrestricted access to the wrapper pointers,
// because the public could make a copy of the pointer, giving it a different
// scope than the Proxy, which defeats the purpose of this class.
template <class T>
class TProxy {
	public:
						TProxy (const TProxy& inOther);
		const TProxy& 	operator= (const TProxy& inOther);
	
						~TProxy ();

		T* 				operator-> () 	{ return mPtr; }
		const T*		operator-> () const { return mPtr; }
		
		// Dangerous, but essential for upcasting...
		// DO NOT, under ANY circumstances DELETE the pointer returned here!
		T*				GetPtr () 		{ return mPtr; }
		const T*		GetPtr () const { return mPtr; }
		
	private:
						TProxy (	TCreator<T>* inOwner, 
									T* inObject);

		TCreator<T>* mOwner;
		T* mPtr;

	friend class TCreator<T>;
};

#if 0
#pragma mark -
#pragma mark TCreator<T> Declaration
#endif

// This is the TCreator interface.  Mostly it is notifications called
// on various events by the TProxy, but the public calls GetProxy
// to actually get a TProxy in the first place.  The return-by-val
// semantics generate some redundant messages, but these are the
// semantics we want for Proxies, so we must keep the notification
// functions tight for the typical case.

template <class T>
class TCreator {
	public:
		// Called when a proxy is requested by the public.
		virtual TProxy<T>	GetProxy() = 0;

	protected:
		// Actually builds the TProxy object
		TProxy<T> 			MakeProxy(T* inObject) 
			{	return TProxy<T>(this, inObject);	}
	
		// Called by the proxy when a TProxy is copied.
		virtual void 		ProxyCopied(T* inObject) = 0;

		// Called by the proxy when a TProxy is deleted.
		virtual void 		ProxyDeleted(T* inObject) = 0;

	// Friendship so the proxy can call the notifiers, which aren't public
	friend class TProxy<T>;
};

#if 0
#pragma mark -
#pragma mark TEmptyCreator<T> Declaration
#endif

// This is a dummy creator, for the case where you MUST use this template,
// but don't want it to do anything.
// Done this way, so the overhead in the unusual case of having no creator
// is handled by the unusual case, rather than null owner pointer checks
// in TProxy, which would hit every use; hard because they're passed by value
// all over the place, so leaner is better on TProxy, at the expense of having
// this silly dummy object.

template <class T>
class TEmptyCreator : public TCreator<T> {
	public:
							TEmptyCreator(T* inPointer);
		virtual TProxy<T>	GetProxy();
	
	protected:
		virtual T*			CreateProxy() { return mPointer; }
		virtual void		ProxyCopied(T* inObject) 	{}
		virtual void		ProxyDeleted(T* inObject) 	{}
		
	private:
		T*					mPointer;
};

#if 0
#pragma mark -
#pragma mark TRefCountingCreator<T> Declaration
#endif

// A sample TCreator; this one does RefCounting.
// Implemented for CGraphicsPort, but useful elsewhere.
template <class T>
class TRefCountingCreator : public TCreator<T> {
	public:
		virtual TProxy<T> 		GetProxy();

	protected:	
								TRefCountingCreator();

		virtual T* 				CreateProxy () = 0;
		virtual void			ProxyCopied(T* inObject);
		virtual void			ProxyDeleted(T* inObject);
		// This is what's called when the proxied object is actually deleted
		virtual void			DeleteProxy(T* inObject);
	
	private:
		T* mObject;
		int mRefCount;
};

// =====================================================================
//	TProxy<T>
// =====================================================================

#if 0
#pragma mark -
#pragma mark -
#pragma mark TProxy<T> Implementation
#pragma mark -
#endif

// ---------------------------------------------------------------------
//  TProxy   		                                           [private]
// ---------------------------------------------------------------------
// Create a TProxy object, associated with a given Creator.

template <class T>
TProxy<T>::TProxy (TCreator<T>* inOwner, T* inObject) : mOwner(inOwner), mPtr(inObject)
{}


// ---------------------------------------------------------------------
//  TProxy   		                                            [public]
// ---------------------------------------------------------------------
// Copy constructor, that informs the Creator about such copies.

template <class T>
TProxy<T>::TProxy (const TProxy& inOther) 
{
	mOwner = inOther.mOwner;
	mPtr = inOther.mPtr;
	mOwner->ProxyCopied(mPtr);
}


// ---------------------------------------------------------------------
//  operator=  		                                            [public]
// ---------------------------------------------------------------------
// Assignment.  Assures that copy message is sent first, so the object
// isn't accidentally destroyed on self-assigns.
// No simple self-assign check, because it's somewhat non-trivial.

template <class T>	
const TProxy<T>& 
TProxy<T>::operator= (const TProxy<T>& inOther) {
	TCreator<T>* oldOwner = mOwner;
	T* oldPtr = mPtr;
	mOwner = inOther.mOwner;
	mPtr = inOther.mPtr;
	mOwner->ProxyCopied(mPtr);
	oldOwner->ProxyDeleted(oldPtr);
}


// ---------------------------------------------------------------------
//  ~TProxy   		                                            [public]
// ---------------------------------------------------------------------
// Destroy a TProxy object, informing the Creator about this.

template <class T>
TProxy<T>::~TProxy () {
	mOwner->ProxyDeleted(mPtr);
}

// =====================================================================
//	TEmptyCreator<T>
// =====================================================================

// Implementation of TEmptyCreator
#if 0
#pragma mark -
#pragma mark TEmptyCreator<T> Implementation
#endif

// ---------------------------------------------------------------------
//  TEmptyCreator		                                     [protected]
// ---------------------------------------------------------------------
// Create a dummy Creator

template <class T>
TEmptyCreator<T>::TEmptyCreator (T* inPointer)
: mPointer(inPointer)
{}


// ---------------------------------------------------------------------
//  GetProxy 			                               [virtual][public]
// ---------------------------------------------------------------------
// Pass-through to CreateObject

template <class T>
TProxy<T>
TEmptyCreator<T>::GetProxy()
{
	return MakeProxy(CreateProxy());
}

// =====================================================================
//	TRefCountingCreator<T>
// =====================================================================
#if 0
#pragma mark -
#pragma mark TRefCountingCreator<T> Implementation
#pragma mark -
#endif

// ---------------------------------------------------------------------
//  TRefCountingCreator                                      [protected]
// ---------------------------------------------------------------------
// Create a TRefCountingCreator object.

template <class T>
TRefCountingCreator<T>::TRefCountingCreator()
: TCreator<T>(), mObject(0), mRefCount(0)
{}


// ---------------------------------------------------------------------
//  GetProxy   		                               [virtual][public]
// ---------------------------------------------------------------------
// This is what the public calls to get an object from a Creator.
// If the refCount is not 0, another proxy is made to point to the same
// object.  Note that implementations of TRefCountingCreator need to
// define CreateObject, which actually creates the refCounted object.

template <class T>
TProxy<T>
TRefCountingCreator<T>::GetProxy ()
{
	if (mRefCount == 0) 
		mObject = CreateProxy();
	
	++mRefCount;
	return MakeProxy(mObject);
}


// ---------------------------------------------------------------------
//  ProxyCopied		                                [virtual][protected]
// ---------------------------------------------------------------------
// Notification that a proxy was copied; increments the RefCount.

template <class T>
void
TRefCountingCreator<T>::ProxyCopied(T*) {
	++mRefCount;
}


// ---------------------------------------------------------------------
//  ProxyDeleted	                                [virtual][protected]
// ---------------------------------------------------------------------
// Notification that a proxy was deleted; decrements the RefCount.
// If the refCount hits 0, DeleteObject is called.

template <class T>
void
TRefCountingCreator<T>::ProxyDeleted(T*) {
	--mRefCount;
		
	if (mRefCount <= 0)
		DeleteProxy(mObject);
}

	
// ---------------------------------------------------------------------
//  DeleteProxy	                                [virtual][protected]
// ---------------------------------------------------------------------
// This is the function called when the RefCounted object should
// actually be deleted.  This default implementation simply does a delete.
	
template <class T>
void
TRefCountingCreator<T>::DeleteProxy (T*) {
	delete mObject;
	mObject = 0;
}

#endif // _H_TProxy_