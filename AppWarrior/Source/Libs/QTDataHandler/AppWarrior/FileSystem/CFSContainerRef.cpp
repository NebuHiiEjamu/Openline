/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
// base class for all container FSrefs

#include "AW.h"
#include "CFSContainerRef.h"

HL_Begin_Namespace_BigRedH

// ---------------------------------------------------------------------
//  CFSContainerRef	                                        	[public]
// ---------------------------------------------------------------------
// constructor

CFSContainerRef::Content::Content()
{
}

// ---------------------------------------------------------------------
//  ~CFSContainerRef	                               [public][virtual]
// ---------------------------------------------------------------------
// destructor

CFSContainerRef::Content::~Content()
{ 
	try
	{
		while(!empty()) 
		{
			try // don't allow anything be thrown out of the destructor
			{ 
				delete front(); 
				pop_front(); 
			} 
			catch (...)
			{
				UDebug::Message("failed to delete a Content item in " __FILE__ "\n");
			}
		}
	} 
	catch (...)
	{
		UDebug::Message("failed to destruct Content in " __FILE__ "\n");
	}
}

HL_End_Namespace_BigRedH
