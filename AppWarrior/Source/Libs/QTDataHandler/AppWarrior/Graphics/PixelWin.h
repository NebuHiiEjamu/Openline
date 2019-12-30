// =====================================================================
//	PixelWin.h			                 (C) Hotline Communications 1999
// =====================================================================
//
// Pixel format definitions for Win32.
// Note that on Win32 the bitmaps are upside-down
// i.e. Offset 0 is the start of the bottom scan line.
// This won't change the pixel cracking classes,
// but it's still good to know.

#ifndef _H_PixelWin_
#define _H_PixelWin_

#include "CColor.h"

HL_Begin_Namespace_BigRedH

// =====================================================================
//	PixelFormat32Win_
// =====================================================================
// The PixelFormat object for 32 bit pixels, on windows.
class PixelFormat32Win_ {
	public:
		static inline int GetPixelBytes()
			{ return 4; }
		
		// Break a packed pixel into 8bpp channels		
		static inline CColor
						CrackPixel (unsigned char* inPixel)
		{
			return CColor(inPixel[2], inPixel[1], inPixel[0]);
		}
		
		// CrackPixel, but alpha aware
		static inline CColor
						CrackPixelAlpha (unsigned char* inPixel)
		{
			return CColor(inPixel[2], inPixel[1], inPixel[0], inPixel[3]);			
		}
				
		// Builds a packed pixel from 8 bit channels
		static inline void 	MakePixel (	unsigned char* outPixel
										,const CColor& inPixel)
		{
			outPixel[2] = inPixel.GetRed();
			outPixel[1] = inPixel.GetGreen();
			outPixel[0] = inPixel.GetBlue();
		}
		
		// MakePixel, but alpha aware
		static inline void MakePixelAlpha ( unsigned char* outPixel
											,const CColor& inPixel)
		{
			MakePixel(outPixel, inPixel);
			outPixel[3] = inPixel.GetAlpha();
		}
};
extern const PixelFormat32Win_ PixelFormat32Win;

// =====================================================================
//	PixelFormat16Win
// =====================================================================
// The PixelFormat object for 16 bit pixels, on windows.
class PixelFormat16Win_ {
	public:
		static inline int GetPixelBytes()
			{ return 2; }
			
		static inline CColor 
						CrackPixel 	(unsigned char* inPixel)
		{
			unsigned short inPixelBits = *reinterpret_cast<unsigned short*>(inPixel);
			unsigned char outRed, outGreen, outBlue;
			// Wierd form; makes low 3 bits (conventionally empty) = high 3 bits
			// This is so that 0 maps to 0, but PixMax maps to PixMax
			outRed = static_cast<unsigned char>((inPixelBits & 0x1F) << 3);
			outRed = static_cast<unsigned char>(outRed | ((outRed & 0xE0) >> 5));
			
			
			outGreen = static_cast<unsigned char>(((inPixelBits >> 5) & 0x1F) << 3);
			outGreen = static_cast<unsigned char>(outGreen | ((outGreen & 0xE0) >> 5));
			
			outBlue = static_cast<unsigned char>(((inPixelBits >> 10) & 0x1F) << 3);
			outBlue = static_cast<unsigned char>(outBlue | ((outBlue & 0xE0) >> 5));

			return CColor(outRed, outGreen, outBlue);
		}
		
		static inline CColor 
						CrackPixelAlpha (unsigned char* inPixel)
		{
			unsigned short inPixelBits = *reinterpret_cast<unsigned short*>(inPixel);
			unsigned char outRed, outGreen, outBlue, outAlpha;
			// Wierd form; makes low 3 bits (conventionally empty) = high 3 bits
			// This is so that 0 maps to 0, but PixMax maps to PixMax
			outRed = static_cast<unsigned char>((inPixelBits & 0x1F) << 3);
			outRed = static_cast<unsigned char>(outRed | ((outRed & 0xE0) >> 5));
			
			
			outGreen = static_cast<unsigned char>(((inPixelBits >> 5) & 0x1F) << 3);
			outGreen = static_cast<unsigned char>(outGreen | ((outGreen & 0xE0) >> 5));
			
			outBlue = static_cast<unsigned char>(((inPixelBits >> 10) & 0x1F) << 3);
			outBlue = static_cast<unsigned char>(outBlue | ((outBlue & 0xE0) >> 5));

			outAlpha = static_cast<unsigned char>((inPixelBits >> 15)?0xFF:0);
			
			return CColor(outRed, outGreen, outBlue, outAlpha);
		}

		static inline void MakePixel (	unsigned char* outPixel
										,const CColor& inPixel)
		{
			*reinterpret_cast<unsigned short*>(outPixel) =
					static_cast<unsigned short>
					((static_cast<unsigned short>(inPixel.GetRed()) >> 3)
					| (((static_cast<unsigned short>(inPixel.GetGreen()) >> 3) & 0x1F) << 5)
					| (((static_cast<unsigned short>(inPixel.GetBlue()) >> 3) & 0x1F) << 10));
		}
		
		static inline void MakePixelAlpha (	unsigned char* outPixel
											,const CColor& inPixel)
		{
			*reinterpret_cast<unsigned short*>(outPixel) =
					static_cast<unsigned short>
					( (((static_cast<unsigned short>((inPixel.GetAlpha() >= 0x80)?1:0))) << 15)
					| (((static_cast<unsigned short>(inPixel.GetRed()) >> 3)))
					| (((static_cast<unsigned short>(inPixel.GetGreen()) >> 3) & 0x1F) << 5)
					| (((static_cast<unsigned short>(inPixel.GetBlue()) >> 3) & 0x1F) << 10));
		}


};
extern const PixelFormat16Win_ PixelFormat16Win;

HL_End_Namespace_BigRedH

#endif // _H_PixelWin_