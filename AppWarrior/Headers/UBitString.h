/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"

class UBitString
{
	public:
		static bool Get(const void *inData, Uint32 inBit);
		static void Set(void *ioData, Uint32 inBit, bool inValue);
		static void Set(void *ioData, Uint32 inIndex, Uint32 inCount, bool inValue);
		static void Set(void *ioData, Uint32 inBit);
		static void Clear(void *ioData, Uint32 inBit);
		static void Copy(void *ioData, Uint32 inTo, Uint32 inFrom, Uint32 inSize);
		
		static bool GetFirstSet(const void *inData, Uint32 inMaxBits, Uint32& outIndex);
		static bool GetLastSet(const void *inData, Uint32 inMaxBits, Uint32& outIndex);
		static bool GetNextSet(const void *inData, Uint32 inMaxBits, Uint32& ioIndex);
};

#define UBits UBitString



