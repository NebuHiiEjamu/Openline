/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "AW.h"
#include "CMessage.h"


HL_Begin_Namespace_BigRedH

// ---------------------------------------------------------------------
//  CMessage                                                   [private]
// ---------------------------------------------------------------------
// Constructor

CMessage::CMessage( UInt32 inID )
	: mID( inID )
{
}


// ---------------------------------------------------------------------
//  CMessage                                                    [public]
// ---------------------------------------------------------------------
// Copy Constructor

CMessage::CMessage( const CMessage &inOriginal )
{
	*this = inOriginal;
}


// ---------------------------------------------------------------------
//  ~CMessage                                          [public][virtual]
// ---------------------------------------------------------------------
// Destructor

CMessage::~CMessage()
{
}

HL_End_Namespace_BigRedH
