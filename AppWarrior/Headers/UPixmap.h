/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */


#pragma once
#include "typedefs.h"
#include "UGraphics.h"

/*
 * Types
 */

typedef void (*THorizLineProc)(void *inRef, Int32 x1, Int32 y1, Int32 x2, Uint32 inColor);
typedef void (*TVertLineProc)(void *inRef, Int32 x1, Int32 y1, Int32 y2, Uint32 inColor);

/*
 * Structs
 */

// simple pixmap
struct SPixmap {
	Uint32 width;			// width in pixels of image
	Uint32 height;			// height in pixels of image
	Uint32 depth;			// number of bits per pixel, 1, 2, 4, 8, 16, 24, 32
	Uint32 rowBytes;		// number of bytes per pixel row
	Uint32 colorCount;		// number of colors in color table
	Uint32 misc;			// reserved
	void *data;				// ptr to pixel data
	Uint32 *colorTab;		// ptr to color data
};

// extended pixmap
struct SExtPixmap {
	Uint32 width;			// width in pixels of image
	Uint32 height;			// height in pixels of image
	Uint32 rowBytes;		// number of bytes per pixel row
	Uint32 colorCount;		// number of colors in color table
	Uint32 transColor;		// RGBA value of transparent color
	Uint32 rsvdA;			// reserved, set to 0
	Uint32 rsvdB;			// reserved, set to 0
	Uint32 rsvdC;			// reserved, set to 0
	Uint8 depth;			// number of bits per pixel, 1, 2, 4, 8, 16, 24, 32
	Uint8 pixPackType;		// pixel packing type
	Uint8 pixCompType;		// pixel compression type (0=none, 1=RLE)
	Uint8 pixLaceType;		// interlacing (0=none)
	Uint8 colorPackType; 	// color table packing (0=rgb, 1=rgba)
	Uint8 opts;				// bit 1 = use transparent color
	Uint8 rsvdD;			// reserved, set to 0
	Uint8 rsvdE;			// reserved, set to 0
	void *pixelData;		// ptr to pixel data
	Uint32 pixelDataSize;	// size in bytes of pixel data
	void *colorData;		// ptr to color data
	Uint32 colorDataSize;	// size in bytes of color data
	void *miscData;			// ptr to misc data
	Uint32 miscDataSize;	// size in bytes of misc data
};

/*
 * Pixmap Class
 */

class UPixmap
{
	public:
		static bool IsValid(THdl inHdl);
		static bool IsValid(const SPixmap& inPM);
		
		static Uint32 GetLayerCount(THdl inHdl);
		static void GetLayerSize(THdl inHdl, Uint32 inIndex, Uint32& outWidth, Uint32& outHeight);
		static void GetLayerPixels(THdl inHdl, Uint32 inIndex, TImage inDest, const SPoint& inDestPt, Uint32 inOptions = 0);
		static void GetLayerPixels(THdl inHdl, Uint32 inIndex, const SPixmap& inDest, const SPoint& inDestPt, Uint32 inOptions = 0);
		static bool GetLayerTransparentColor(THdl inHdl, Uint32 inIndex, Uint32& outColor);

		static void Draw(THdl inHdl, Uint32 inIndex, TImage inImage, const SRect& inBounds, Uint16 inAlign = align_Center, Uint16 inTransform = transform_None);
		static Uint32 GetNearestLayer(THdl inHdl, TImage inImage, const SRect& inBounds);

		static THdl FlattenToHandle(const SExtPixmap *inLayers, Uint32 inCount);

		static void ScanFilledRect(const SRect& inRect, THorizLineProc inHLine, void *inRef, Uint32 inColor);
		static void ScanFilledPolygon(const SPoint *inPointList, Uint32 inPointCount, THorizLineProc inHLine, void *inRef, Uint32 inColor);

		static void CopyPixels(const SPixmap& inDest, const SPoint& inDestPt, const SPixmap& inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inOptions = 0);
		static void CopyPixelsTrans(const SPixmap& inDest, const SPoint& inDestPt, const SPixmap& inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inTransCol, Uint32 inOptions = 0);
};




