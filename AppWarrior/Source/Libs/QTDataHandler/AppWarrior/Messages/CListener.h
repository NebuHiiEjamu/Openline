// =====================================================================
//	CListener.h                          (C) Hotline Communications 1999
// =====================================================================
//
// An abstract mix-in class that works with CBroadcaster class to
// implement dependencies. A Listener receives messages from its 
// Broadcasters.

#ifndef _H_CListener_
#define _H_CListener_

#include "CMessage.h"

#if PRAGMA_ONCE
	#pragma once
#endif			    

HL_Begin_Namespace_BigRedH
class CMessageWrangler;

class CListener
{
		friend class CBroadcaster;
	public:
						CListener();
							// throws CMessageException, CMemoryException
						CListener( CMessageWrangler *inWrangler );
							// throws CMemoryException
						CListener( const CListener &inOriginal );
							// throws CMessageException, CMemoryException
		virtual			~CListener();
							// throws nothing
		
						// Are we listening to this broadcaster?
		bool			HasBroadcaster( const CBroadcaster &inBroadcaster );
							// throws nothing
		
			// ** Wrangler Management **
		void			SetWrangler( CMessageWrangler *inWrangler )
							// throws nothing
							{ mWrangler = inWrangler; }

			// ** Listening State **
		void			StartListening()
							// throws nothing
							{ mIsListening = true; }
		void			StopListening()
							// throws nothing
							{ mIsListening = false; }
		bool			IsListening() const
							// throws nothing
							{ return mIsListening; }
							
						// Receive the message
		void			Listen( const CMessage &inMessage );
							// throws CMessageException

						// Handle the message
		virtual void	ListenToMessage( const CMessage &inMessage ) = 0;
							// Pure Virtual. Concrete subclasses must override
							// throws CMessageException, ???

	protected:
		std::list<CBroadcaster*>	mBroadcasters;
		bool						mIsListening;
		CMessageWrangler			*mWrangler;

			// ** Broadcaster Management **
		void			AddBroadcaster( CBroadcaster &inBroadcaster );
							// throws CMessageException
		void			RemoveBroadcaster( CBroadcaster &inBroadcaster );
							// throws CMessageException

	public:
		// Helper stack based class to temp turn off listening
		class StEarplug
		{
			public:
								StEarplug( CListener &inListener )
									// throws nothing
									: mListener( inListener )
										{
										mWasListening = mListener.IsListening();
										if( mWasListening )
											mListener.StopListening();
										}
								~StEarplug()
									// throws nothing
										{
										if( mWasListening )
											mListener.StartListening();
										}

			private:
				CListener		&mListener;
				bool			mWasListening;
		};
};



HL_End_Namespace_BigRedH
#endif // _H_CListener_
