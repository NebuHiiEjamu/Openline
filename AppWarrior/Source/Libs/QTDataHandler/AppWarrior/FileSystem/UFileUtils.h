/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
// A utility class for getting various information about the current
// file system

#ifndef _H_UFileUtils_
#define _H_UFileUtils_

#if PRAGMA_ONCE
	#pragma once
#endif

HL_Begin_Namespace_BigRedH

class UFileUtils {
	public:
										// Returns the pathname to the
										// temporary folder on the current platform
		static CString			GetTemporaryFolder();
										// throws CFSException

};

HL_End_Namespace_BigRedH
#endif // _H_UFileUtils_
