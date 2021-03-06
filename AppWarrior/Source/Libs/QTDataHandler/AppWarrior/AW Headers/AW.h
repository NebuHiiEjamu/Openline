/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#ifndef _H_AW_
#define _H_AW_

#include "AWMacros.h"			// needed for BigRedH, ASSERT, REQUIRE

#define _MSL_NO_THROW_SPECS		// need for NewHandler, allows throwing
								// CMemoryException from new

#if PRAGMA_ONCE
	#pragma once
#endif

#if (defined(COMPILER_MSVC) && !defined(_CPPRTTI))
	#pragma message("RTTI is not enabled, please enable!")
#endif

#ifdef COMPILER_CW
	#pragma check_header_flags on
	#pragma cplusplus on
	#pragma warn_hidevirtual on
	#pragma warn_illpragma on
	//#pragma warn_notinlined on 
	//#pragma warn_possunwant on
	//#pragma warn_unusedarg on
	#pragma warn_unusedvar on
#endif	

#ifdef COMPILER_MSVC
	#pragma warning(disable: 4068)	// unknown pragma
	#pragma warning (disable: 4786)
#endif

#if 0
	#pragma mark stl
#endif

#include <list>
#include <map>
#include <string>
#include <queue>
#include <set>
#include <vector>
#include <memory>
#include <functional>
#include <algorithm>

#if 0
	#pragma mark CommonIncludes
#endif

#ifdef COMPILER_MSVC
#	include "VCSTD.h"
#endif

#include "AWTypes.h"        	   // needed everywhere 
#include "UAWError.h"        	   // needed for the AW error codes
#include "UDebug.h"				   // the debug utility class

#endif // _H_AW_