/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"

class UDigest
{
	public:
		// Encoder/Decoder in Base64 format
		static void *Base64_Encode(const void *inData, Uint32 inDataSize, Uint32& outDataSize);
		static void *Base64_Decode(const void *inData, Uint32 inDataSize, Uint32& outDataSize);

		// Encoder/Decoder in UU format
		static void *UU_Encode(const void *inData, Uint32 inDataSize, Uint32& outDataSize);
		static void *UU_Decode(const void *inData, Uint32 inDataSize, Uint32& outDataSize);
		
		// Encoder in MD5 format (outDataSize = 16 bytes)
		static void *MD5_Encode(const void *inData, Uint32 inDataSize, Uint32& outDataSize);
		// no decoder here
};
