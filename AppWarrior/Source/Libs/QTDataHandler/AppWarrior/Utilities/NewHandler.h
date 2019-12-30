// =====================================================================
//  NewHandler.h                         (C) Hotline Communications 1999
// =====================================================================
//
// Used by new to handle allocations not completed. We use to throw a
// CMemory execption instead of the standard bad_alloc

#ifndef _H_NewHandler_
#define _H_NewHandler_

#include "AW.h"

#if PRAGMA_ONCE
	#pragma once
#endif

HL_Begin_Namespace_BigRedH

void	NH_SetupNewHandler();
void	NH_ResetNewHandler();

#if defined(COMPILER_MSVC)
int		NH_NewHandler(size_t);
#else
void	NH_NewHandler();
#endif
			// throws CMemoryException

HL_End_Namespace_BigRedH

#endif	// _H_NewHandler_
