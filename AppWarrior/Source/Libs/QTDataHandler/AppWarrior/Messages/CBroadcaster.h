// =====================================================================
//  CBroadcaster.h                       (C) Hotline Communications 1999
// =====================================================================
//
// A mix-in class that works with CListener class to implement
// dependencies. A Broadcaster sends messages to its Listeners.

#ifndef _H_CBroadcaster_
#define _H_CBroadcaster_

#if PRAGMA_ONCE
	#pragma once
#endif					    

#include "CMutex.h"

HL_Begin_Namespace_BigRedH

const unsigned long		msg_BL_BroadcasterDied = 'BDed';

class CListener;
class CMessage;


class	CBroadcaster 
{
	public:
								CBroadcaster();
									// throws ???
								CBroadcaster( const CBroadcaster &inOriginal );
									// throws ???
		virtual					~CBroadcaster();
									// throws nothing
		
			// ** Listener Management **
		void					AddListener( CListener &inListener );
									// throws CMessageException
		void					RemoveListener( CListener &inListener );
									// throws CMessageException
							
		bool					HasListener( const CListener &inListener );
									// throws nothing
		
			// ** Broadcasting State **
		void					StartBroadcasting()
									// throws nothing
									{ mIsBroadcasting = true; }
		void					StopBroadcasting()
									// throws nothing
									{ mIsBroadcasting = false; }
		bool					IsBroadcasting() const
									// throws nothing
									{ return mIsBroadcasting; }

								// Send the message
		void					BroadcastMessage( const CMessage &inMessage );
									// throws CMessageException

	protected:
		std::list<CListener*>	mListeners;
		CMutex					mLLock;
		bool					mIsBroadcasting;

			// ** Add safety to iterators that can cause deletions **
		std::set<CListener*>	mDeletionSet;
		UInt8					mDeletionCount;
		void					DeletionOpen()
									// throws nothing
									{ mDeletionCount++; }
		void					DeletionClose();
									// throws CMessageException

	public:
		// Helper stack based class to temp turn off broadcasting
		class StGag
		{
			public:
								StGag( CBroadcaster &inBroadcaster )
										// throws nothing
									: mBroadcaster( inBroadcaster )
										{
										mWasBroadcasting = mBroadcaster.IsBroadcasting();
										if( mWasBroadcasting )
											mBroadcaster.StopBroadcasting();
										}
								~StGag()
										// throws nothing
										{
										if( mWasBroadcasting )
											mBroadcaster.StartBroadcasting();
										}

			private:
				CBroadcaster	&mBroadcaster;
				bool			mWasBroadcasting;
		};
		// Helper stack based class manage deletions
		class StDeletion
		{
			public:
								StDeletion( CBroadcaster &inBroadcaster )
										// throws nothing
									: mBroadcaster( inBroadcaster )
										{ mBroadcaster.DeletionOpen(); }
								~StDeletion()
										// throws nothing
										{ try { mBroadcaster.DeletionClose(); } catch(...){} }

			private:
				CBroadcaster	&mBroadcaster;
		};
		friend class StDeletion;
};

HL_End_Namespace_BigRedH
#endif // _H_CBroadcaster_
