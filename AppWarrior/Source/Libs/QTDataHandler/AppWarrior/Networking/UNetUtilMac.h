// =====================================================================
//  UNetUtilMac.h                        (C) Hotline Communications 1999
// =====================================================================
//
// Utility class to have various platform specific network calls

#ifndef _H_UNetUtilMac_
#define _H_UNetUtilMac_

#if PRAGMA_ONCE
	#pragma once
#endif

//#include <OpenTransport.h>
#include <OpenTptInternet.h>
#include "CNetAddress.h"

HL_Begin_Namespace_BigRedH

class UNetUtilMac
{
	public:
		static pascal void	YieldingNotifier( void* contextPtr, OTEventCode code, 
									   OTResult result, void* cookie );

		static CNetAddress	OTToNetAddress( InetAddress *inAddress );
		static InetAddress*	NetAddressToOT( const CNetAddress &inAddress );
		
	#if TARGET_API_MAC_CARBON
		static OTNotifyUPP sYieldingNotifierUPP;
	#endif
};

#if 0
	#pragma mark CThreadTimer
#endif


class CNetTimeoutMac 
{

	typedef struct 
	{
		CNetTimeoutMac			*mTimer;
		TMTask					mTask;
		QHdr					mSemQ;
		QElem					mSemEl;
	} SNetTimeoutTMTask;

	public:
				CNetTimeoutMac( TEndpoint *inEndpoint, UInt32 inMilliSecs );
				~CNetTimeoutMac();

		bool	HasFired() { return mFired; }

	private:
		static TimerUPP				sNetTimerUPP;	// UPP for time mgr
		
		SNetTimeoutTMTask			mTask;
		TEndpoint					*mEndpoint;
		bool						mFired;

		static pascal void			Execute( TMTaskPtr inTask );

		static SNetTimeoutTMTask*	GetTimeMgrPtr( TMTaskPtr inTaskPtr );
		void						InsertTimeTask();
		void						PrimeTimeTask( UInt32 inMilliSecs );
		void						RemoveTimeTask();

									CNetTimeoutMac();
};

HL_End_Namespace_BigRedH
#endif	// _H_UNetUtilMac_
