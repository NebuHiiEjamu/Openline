/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "AW.h"
#include "UNetUtil.h"
#include <OpenTransport.h>


HL_Begin_Namespace_BigRedH

// ---------------------------------------------------------------------
//  StartNetworking                                     [public][static]
// ---------------------------------------------------------------------
// Start up OT

void
UNetUtil::StartNetworking()
{
#if TARGET_API_MAC_CARBON
	::InitOpenTransportInContext(kInitOTForApplicationMask, NULL);	//??
#else
	::InitOpenTransport();
#endif
}


// ---------------------------------------------------------------------
//  StopNetworking                                      [public][static]
// ---------------------------------------------------------------------
// Close OT

void
UNetUtil::StopNetworking()
{
#if TARGET_API_MAC_CARBON
	::CloseOpenTransportInContext(NULL);
#else
	::CloseOpenTransport();
#endif
}

HL_End_Namespace_BigRedH
