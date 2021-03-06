/***************************************************************

CopyPixels() functions don't handle overlapping areas in the 
same pixmap.

CopyPixels() on direct to palette color copies (eg 32 to 8) could
be made MUCH faster by making a 5*5*5 bit entry table that acts like
a cache (so the same color is never looked up twice, and it also
reduces the max number of lookups to 32K, 15 bits).  This is MUCH
faster but the quality isn't always as good so provide option to
use the slow method as well.

Also the copies could probably be made faster by unrolling loops
and copying longs instead of individual bytes, words etc.

When working with 16-bit bit colors (eg in CopyPixels()), currently
the funcs produce the correct 16-bit value on a PPC processor, but 
not on Intel.  Need to change the bit shifts etc so that it always 
produces ARRRRRGGGGGBBBBB regardless of endianess.  eek that might 
be very messy/slow (?).  Maybe 16-bit will have to be incompatible 
across different processors.  First investigate if can keep it
portable without sacrificing too much speed.

**********************************************************************/



/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

/*

Format of the flattened pixmap:

Uint32 format;				// always AWPX
Uint16 version;				// currently 1
Uint16 rsvdA;				// reserved, set to 0
Uint32 rsvdB;				// reserved, set to 0
Uint16 rsvdC;				// reserved, set to 0
Uint16 layerCount;			// number of layers
struct {
	Uint32 width;			// width in pixels of image
	Uint32 height;			// height in pixels of image
	Uint32 rowBytes;		// number of bytes per pixel row
	Uint32 miscDataSize;	// number of bytes of misc data
	Uint32 colorDataOffset;	// offset from start of data area to color data
	Uint32 pixelDataOffset;	// offset from start of data area to pixel data
	Uint32 miscDataOffset;	// offset from start of data area to misc data
	Uint32 transColor;		// RGBA value of transparent color
	Uint16 rsvd;			// reserved, set to 0
	Uint16 colorCount;		// number of colors in color table
	Uint16 rsvdB;			// reserved, set to 0
	Uint16 rsvdC;			// reserved, set to 0
	Uint8 depth;			// number of bits per pixel, 1, 2, 4, 8, 16, 24, 32
	Uint8 pixPackType;		// pixel packing type
	Uint8 pixCompType;		// pixel compression type (0=none, 1=RLE)
	Uint8 pixLaceType;		// interlacing (0=none)
	Uint8 colorPackType; 	// color table packing (0=rgb, 1=rgba)
	Uint8 opts;				// bit 1 = use transparent color
	Uint8 rsvdD;			// reserved, set to 0
	Uint8 rsvdE;			// reserved, set to 0
} layer[layerCount];
Uint8 data[];

*/

#include "UPixmap.h"

#pragma options align=packed
struct SPMFlatHdr {
	Uint32 format;
	Uint16 version;
	Uint16 rsvdA;
	Uint32 rsvdB;
	Uint16 rsvdC;
	Uint16 layerCount;
};
#pragma options align=reset

#pragma options align=packed
struct SPMFlatLayerHdr {
	Uint32 width;
	Uint32 height;
	Uint32 rowBytes;
	Uint32 miscDataSize;
	Uint32 colorDataOffset;
	Uint32 pixelDataOffset;
	Uint32 miscDataOffset;
	Uint32 transColor;
	Uint16 rsvd;
	Uint16 colorCount;
	Uint16 rsvdB;
	Uint16 rsvdC;
	Uint8 depth;
	Uint8 pixPackType;
	Uint8 pixCompType;
	Uint8 pixLaceType;
	Uint8 colorPackType;
	Uint8 opts;
	Uint8 rsvdD;
	Uint8 rsvdE;
};
#pragma options align=reset

// incredibly cool tables for converting 5bit to 8bit without floating point calculations
Uint32 _gConv5to8TabR[32] = { 0x00000000, 0x08000000, 0x10000000, 0x19000000, 0x21000000, 0x29000000, 0x31000000, 0x3A000000, 0x42000000, 0x4A000000, 0x52000000, 0x5A000000, 0x63000000, 0x6B000000, 0x73000000, 0x7B000000, 0x84000000, 0x8C000000, 0x94000000, 0x9C000000, 0xA5000000, 0xAD000000, 0xB5000000, 0xBD000000, 0xC5000000, 0xCE000000, 0xD6000000, 0xDE000000, 0xE6000000, 0xEF000000, 0xF7000000, 0xFF000000 };
Uint32 _gConv5to8TabG[32] = { 0x00000000, 0x00080000, 0x00100000, 0x00190000, 0x00210000, 0x00290000, 0x00310000, 0x003A0000, 0x00420000, 0x004A0000, 0x00520000, 0x005A0000, 0x00630000, 0x006B0000, 0x00730000, 0x007B0000, 0x00840000, 0x008C0000, 0x00940000, 0x009C0000, 0x00A50000, 0x00AD0000, 0x00B50000, 0x00BD0000, 0x00C50000, 0x00CE0000, 0x00D60000, 0x00DE0000, 0x00E60000, 0x00EF0000, 0x00F70000, 0x00FF0000 };
Uint32 _gConv5to8TabB[32] = { 0x00000000, 0x00000800, 0x00001000, 0x00001900, 0x00002100, 0x00002900, 0x00003100, 0x00003A00, 0x00004200, 0x00004A00, 0x00005200, 0x00005A00, 0x00006300, 0x00006B00, 0x00007300, 0x00007B00, 0x00008400, 0x00008C00, 0x00009400, 0x00009C00, 0x0000A500, 0x0000AD00, 0x0000B500, 0x0000BD00, 0x0000C500, 0x0000CE00, 0x0000D600, 0x0000DE00, 0x0000E600, 0x0000EF00, 0x0000F700, 0x0000FF00 };

/* -------------------------------------------------------------------------- */

bool UPixmap::IsValid(THdl inHdl)
{
/****************************************************************************

It would be cool if there was a Uint32 of attributes stored with every THdl
(ooo and that + the existing lockcount would align to 8 bytes).

When the handle is created, the attributes are 0 by default.  When 
UPixmap::IsValid() is called, it can set a "validated" attribute so that
the next time it's called it can check the flag and not do the validation
all over again. Coooool!

*****************************************************************************/

	Require(inHdl);
	
	// check that got at least enough data for header
	Uint32 dataSize = inHdl->GetSize();
	if (dataSize < sizeof(SPMFlatHdr)) return false;
	
	// get ptr to data
	SPMFlatHdr *hdr;
	StHandleLocker lock(inHdl, (void*&)hdr);
	
	// check format and version specifiers
	if (hdr->format != TB((Uint32)'AWPX') || FB(hdr->version) != 1) goto invalid;
	
	// check got enough layer headers
	Uint32 layerCount = FB(hdr->layerCount);
	Uint32 totalHdrSize = sizeof(SPMFlatHdr) + (layerCount * sizeof(SPMFlatLayerHdr));
	if (dataSize < totalHdrSize) goto invalid;
	dataSize -= totalHdrSize;
	
	// validate each layer
	Uint32 offset = 0;
	SPMFlatLayerHdr *layer = (SPMFlatLayerHdr *)(hdr + 1);
	while (layerCount--)
	{
		// check for width/height being too big (don't want to overflow etc)
		Uint32 minRB = FB(layer->width);
		Uint32 height = FB(layer->height);
		if ((minRB & 0xE0000000) || (height & 0xE0000000)) goto invalid;
		
		// check the depth and rowBytes
		switch (layer->depth)
		{
			case 1:
				minRB = RoundUp8(minRB) >> 3;
				break;
			case 2:
				minRB = RoundUp4(minRB) >> 2;
				break;
			case 4:
				minRB = RoundUp2(minRB) >> 1;
				break;
			case 8:
				break;
			case 16:
				minRB <<= 1;
				break;
			case 24:
				minRB *= 3;
				break;
			case 32:
				minRB <<= 2;
				break;
			default:
				goto invalid;
				break;
		}
		Uint32 rb = FB(layer->rowBytes);
		if ((rb & 0xE0000000) || (rb < minRB)) goto invalid;
		
		// check color data offset and size
		Uint32 ofs = FB(layer->colorDataOffset);
		if (ofs < offset) goto invalid;
		offset = ofs + (FB(layer->colorCount) * sizeof(Uint32));
		if ((offset < ofs) || (offset > dataSize)) goto invalid;	// the (offset < ofs) is to check for overflow
		
		// check pixel data offset and size
		if (MulWillOverflow(rb, height)) goto invalid;
		ofs = FB(layer->pixelDataOffset);
		if (ofs < offset) goto invalid;
		offset = ofs + (rb * height);
		if ((offset < ofs) || (offset > dataSize)) goto invalid;	// the (offset < ofs) is to check for overflow
		
		// check misc data offset and size
		ofs = FB(layer->miscDataOffset);
		if (ofs < offset) goto invalid;
		offset = ofs + FB(layer->miscDataSize);
		if ((offset < ofs) || (offset > dataSize)) goto invalid;	// the (offset < ofs) is to check for overflow

		// next layer
		layer++;
	}
	
	// exit points
	return true;
invalid:
	return false;
}

Uint32 UPixmap::GetLayerCount(THdl inHdl)
{
	Require(inHdl);
	return FB(UMemory::ReadWord(inHdl, (UInt32)&(((SPMFlatHdr*)0)->layerCount) ));
}

// inIndex is 1-based
void UPixmap::GetLayerSize(THdl inHdl, Uint32 inIndex, Uint32& outWidth, Uint32& outHeight)
{
	Require(inHdl && inIndex);
	
	SPMFlatHdr *hdr;
	StHandleLocker lock(inHdl, (void*&)hdr);
	
	if (inIndex > FB(hdr->layerCount)) Fail(errorType_Misc, error_NoSuchItem);
	
	SPMFlatLayerHdr *layer = ((SPMFlatLayerHdr *)(hdr + 1)) + inIndex - 1;
	
	outWidth = FB(layer->width);
	outHeight = FB(layer->height);
}

// don't forget to lock the image! (if applicable)
void UPixmap::GetLayerPixels(THdl inHdl, Uint32 inIndex, TImage inDest, const SPoint& inDestPt, Uint32 inOptions)
{
	Require(inHdl && inIndex);
	
	SPMFlatHdr *hdr;
	StHandleLocker lock(inHdl, (void*&)hdr);
	
	Uint32 layerCount = FB(hdr->layerCount);
	if (inIndex > layerCount) Fail(errorType_Misc, error_NoSuchItem);
	
	SPMFlatLayerHdr *layer = ((SPMFlatLayerHdr *)(hdr + 1)) + inIndex - 1;
	
	Uint8 *dataArea = BPTR(hdr + 1) + (sizeof(SPMFlatLayerHdr) * layerCount);
	
	SPixmap pm;
	
	pm.width = FB(layer->width);
	pm.height = FB(layer->height);
	pm.depth = layer->depth;
	pm.rowBytes = FB(layer->rowBytes);
	pm.colorCount = FB(layer->colorCount);
	pm.misc = 0;
	pm.data = dataArea + FB(layer->pixelDataOffset);
	pm.colorTab = (Uint32 *)( dataArea + FB(layer->colorDataOffset) );
	
	UGraphics::CopyPixels(inDest, inDestPt, pm, kZeroPoint, pm.width, pm.height, inOptions);
}

// don't forget to lock the image! (if applicable)
void UPixmap::GetLayerPixels(THdl inHdl, Uint32 inIndex, const SPixmap& inDest, const SPoint& inDestPt, Uint32 inOptions)
{
	Require(inHdl && inIndex);
	
	SPMFlatHdr *hdr;
	StHandleLocker lock(inHdl, (void*&)hdr);
	
	Uint32 layerCount = FB(hdr->layerCount);
	if (inIndex > layerCount) Fail(errorType_Misc, error_NoSuchItem);
	
	SPMFlatLayerHdr *layer = ((SPMFlatLayerHdr *)(hdr + 1)) + inIndex - 1;
	
	Uint8 *dataArea = BPTR(hdr + 1) + (sizeof(SPMFlatLayerHdr) * layerCount);
	
	SPixmap pm;
	
	pm.width = FB(layer->width);
	pm.height = FB(layer->height);
	pm.depth = layer->depth;
	pm.rowBytes = FB(layer->rowBytes);
	pm.colorCount = FB(layer->colorCount);
	pm.misc = 0;
	pm.data = dataArea + FB(layer->pixelDataOffset);
	pm.colorTab = (Uint32 *)( dataArea + FB(layer->colorDataOffset) );
	
	CopyPixels(inDest, inDestPt, pm, kZeroPoint, pm.width, pm.height, inOptions);
}

bool UPixmap::GetLayerTransparentColor(THdl inHdl, Uint32 inIndex, Uint32& outColor)
{
	Require(inHdl && inIndex);
	
	SPMFlatHdr *hdr;
	StHandleLocker lock(inHdl, (void*&)hdr);
	
	if (inIndex > FB(hdr->layerCount)) Fail(errorType_Misc, error_NoSuchItem);
	
	SPMFlatLayerHdr *layer = ((SPMFlatLayerHdr *)(hdr + 1)) + inIndex - 1;
	
	outColor = layer->transColor;	// don't need to swap bytes (in RGBA order regardless of endianess)
	return (layer->opts & 1);
}

// returns 1-based index of layer that best matches inImage and inBounds (in terms of depth and size)
Uint32 UPixmap::GetNearestLayer(THdl inHdl, TImage /* inImage */, const SRect& inBounds)
{
	/******************************************************************
	TO DO on this function:
	Only inBounds is used to select the best layer.  The depth of
	inImage should also be used.
	********************************************************************/
	
	Require(inHdl);
	
	SPMFlatHdr *hdr;
	SPMFlatLayerHdr *curLayer, *foundLayer, *firstLayer;
	
	StHandleLocker lock(inHdl, (void*&)hdr);
	
	Uint32 layerCount = FB(hdr->layerCount);
	if (layerCount < 2) return layerCount;

	Uint32 bw = inBounds.GetWidth();
	Uint32 bh = inBounds.GetHeight();
	Uint32 maxDiff = max_Uint32;
	
	curLayer = foundLayer = firstLayer = (SPMFlatLayerHdr *)(hdr + 1);
	
	// search for the layer with the least difference in width/height
	while (layerCount--)
	{
		Uint32 curDiff = diff(FB(curLayer->width), bw) + diff(FB(curLayer->height), bh);
		
		if (curDiff < maxDiff)
		{
			maxDiff = curDiff;
			foundLayer = curLayer;
			
			if (curDiff == 0) break;	// exit loop immediately if found exact match
		}
		
		curLayer++;
	}
	
	return (foundLayer - firstLayer) + 1;
}

// if inIndex is 0, Draw() selects the pixmap that best matches inImage and inBounds (in terms of depth and size)
void UPixmap::Draw(THdl inHdl, Uint32 inIndex, TImage inImage, const SRect& inBounds, Uint16 inAlign, Uint16 inTransform)
{
	/*******************************
	TO DO on this function:  inTransform (for depths > 8)
	
	Also, when inIndex is 0, only inBounds is used to select the best layer.  The depth of inImage should
	also be used.
	**************************************/
	
	// if inHdl is nil, draw a crossed box instead (good for invalid/corrupt pixmaps)
	if (inHdl == nil)
	{
corrupt:
		SColor col;
		UUserInterface::GetSysColor(sysColor_Label, col);
		inImage->SetInkColor(col);
		
		inImage->SetPenSize(1);
		inImage->FrameRect(inBounds);
		
		inImage->DrawLine(*(SLine *)&inBounds);
		inImage->DrawLine(SLine(inBounds.right-1, inBounds.top, inBounds.left-1, inBounds.bottom));
		return;
	}
	
	SPMFlatHdr *hdr;
	SPMFlatLayerHdr *layer;
	StHandleLocker lock(inHdl, (void*&)hdr);
	Uint32 layerCount = FB(hdr->layerCount);
	Uint8 *dataArea = BPTR(hdr + 1) + (sizeof(SPMFlatLayerHdr) * layerCount);

	if (inIndex == 0)
	{
		if (layerCount == 1)
			layer = (SPMFlatLayerHdr *)(hdr + 1);	// first and only layer
		else if (layerCount == 0)
			goto corrupt;
		else
		{
			Uint32 bw = inBounds.GetWidth();
			Uint32 bh = inBounds.GetHeight();
			Uint32 maxDiff = max_Uint32;
			SPMFlatLayerHdr *curLayer = (SPMFlatLayerHdr *)(hdr + 1);
			layer = curLayer;
			
			// search for the layer with the least difference in width/height
			while (layerCount--)
			{
				Uint32 curDiff = diff(FB(curLayer->width), bw) + diff(FB(curLayer->height), bh);
				
				if (curDiff < maxDiff)
				{
					maxDiff = curDiff;
					layer = curLayer;
					
					if (curDiff == 0) break;	// exit loop immediately if found exact match
				}
				
				curLayer++;
			}
		}
	}
	else
	{
		if (inIndex > layerCount) goto corrupt;
		layer = ((SPMFlatLayerHdr *)(hdr + 1)) + inIndex - 1;
	}
		
	SPixmap pm;
	pm.width = FB(layer->width);
	pm.height = FB(layer->height);
	pm.depth = layer->depth;
	pm.rowBytes = FB(layer->rowBytes);
	pm.colorCount = FB(layer->colorCount);
	pm.misc = 0;
	pm.data = dataArea + FB(layer->pixelDataOffset);
	pm.colorTab = (Uint32 *)( dataArea + FB(layer->colorDataOffset) );
	
	SRect r;
	r.left = inBounds.left;
	r.top = inBounds.top;
	r.right = inBounds.left + pm.width;
	r.bottom = inBounds.top + pm.height;
	r.Align(inAlign, inBounds);
	
	
	Uint32 transCol = layer->transColor;
	
	Uint32 tmpTab[256];
	
	
	// do transform:
	if (pm.depth <= 8)	// if data is in color table
	{
		switch (inTransform)
		{
			case transform_Light:
			{
				Uint8 *in = (Uint8 *)pm.colorTab;
				Uint8 *out = (Uint8 *)tmpTab;
				Uint8 *end = (Uint8 *)(pm.colorTab + pm.colorCount);
				pm.colorTab = tmpTab;
				while (in != end)
				{
					*out++ = 128 + *in++ / 2;
					*out++ = 128 + *in++ / 2;
					*out++ = 128 + *in++ / 2;
					*out++ = 128 + *in++ / 2;
				}
				
				// if there's a transparent color, we need to modify it as well to work with our new color map
				out = (Uint8 *)&transCol;
				out[0] = 128 + out[0] / 2;
				out[1] = 128 + out[1] / 2;
				out[2] = 128 + out[2] / 2;
				out[3] = 128 + out[3] / 2;
			}
			break;
			
			
			case transform_Dark:
			{
				Uint8 *in = (Uint8 *)pm.colorTab;
				Uint8 *out = (Uint8 *)tmpTab;
				Uint8 *end = (Uint8 *)(pm.colorTab + pm.colorCount);
				pm.colorTab = tmpTab;
				while (in != end)
				{
					*out++ = *in++ / 2;
					*out++ = *in++ / 2;
					*out++ = *in++ / 2;
					*out++ = *in++ / 2;
				}
				
				// if there's a transparent color, we need to modify it as well to work with our new color map
				out = (Uint8 *)&transCol;
				out[0] = out[0] / 2;
				out[1] = out[1] / 2;
				out[2] = out[2] / 2;
				out[3] = out[3] / 2;
			}
			break;
		}
	}
	else
	{
		// unimplemented!	
	}
	
	try
	{
		if (layer->opts & 1)	// don't need to swap b/c only 1 byte, and don't swap transColor because always RGBA regardless of endianness
			UGraphics::CopyPixelsTrans(inImage, r.TopLeft(), pm, kZeroPoint, pm.width, pm.height, transCol);
		else
			UGraphics::CopyPixels(inImage, r.TopLeft(), pm, kZeroPoint, pm.width, pm.height);
	}
	catch(...)
	{
		goto corrupt;
	}
}

// don't forget to lock your TImages (that sucks, it would be nice if we could do it automatically)
THdl UPixmap::FlattenToHandle(const SExtPixmap *inLayers, Uint32 inCount)
{
	THdl h;
	Uint8 *p;
	Uint32 i, n, offset, dataOffset, colorDataSize, pixelDataSize;
	SPMFlatLayerHdr *layerHdr;
	SRect r(0,0,0,0);
	SPixmap dpm;
	Uint32 colorTab[257];

	Require(inLayers && inCount);
	ASSERT(inCount <= max_Uint16);
	
	offset = dataOffset = sizeof(SPMFlatHdr) + (sizeof(SPMFlatLayerHdr) * inCount);
	h = UMemory::NewHandleClear(offset);
	
	try
	{
		// write the header
		SPMFlatHdr *hdr = (SPMFlatHdr *)UMemory::Lock(h);
		hdr->format = TB((Uint32)'AWPX');
		hdr->version = TB((Uint16)1);
		hdr->layerCount = TB((Uint16)inCount);
		UMemory::Unlock(h);
		
		// write the layers
		for (i=0; i!=inCount; i++)
		{
			const SExtPixmap *layerInfo = inLayers + i;
			if (layerInfo->pixelDataSize == -1)	// if use TImage
			{
				TImage img = (TImage)layerInfo->pixelData;
				
				Uint32 wdt, hgt;
				UGraphics::GetImageSize(img, wdt, hgt);
				
				r.right = wdt;
				r.bottom = hgt;
				
				n = UGraphics::BuildColorTable(img, r, 1, colorTab, 257);
				
				dpm.width = r.right;
				dpm.height = r.bottom;
				dpm.colorCount = n;
				dpm.colorTab = colorTab;
				dpm.misc = 0;
				
				if (n <= 2)
				{
					dpm.depth = 1;
					dpm.rowBytes = RoundUp32(r.right) >> 3;
				}
				else if (n <= 4)
				{
					dpm.depth = 2;
					dpm.rowBytes = RoundUp16(r.right) >> 2;
				}
				else if (n <= 16)
				{
					dpm.depth = 4;
					dpm.rowBytes = RoundUp8(r.right) >> 1;
				}
				else if (n <= 256)
				{
					dpm.depth = 8;
					dpm.rowBytes = RoundUp4(r.right);
				}
				else
				{
					dpm.depth = 32;		// ****** this could be done better, eg 24 bit, or 16 bit
					dpm.rowBytes = r.right * sizeof(Uint32);
					dpm.colorCount = 0;
				}
				
				colorDataSize = dpm.colorCount * sizeof(Uint32);
				pixelDataSize = dpm.rowBytes * dpm.height;
				n = pixelDataSize + colorDataSize;
				
				UMemory::SetSize(h, offset + n);
				
				{
					StHandleLocker lock(h, (void*&)p);
					
					layerHdr = ((SPMFlatLayerHdr *)(p + sizeof(SPMFlatHdr))) + i;
					layerHdr->width = TB(dpm.width);
					layerHdr->height = TB(dpm.height);
					layerHdr->rowBytes = TB(dpm.rowBytes);
					layerHdr->colorDataOffset = TB((Uint32)(offset - dataOffset));
					layerHdr->pixelDataOffset = TB((Uint32)(offset - dataOffset + colorDataSize));
					layerHdr->miscDataOffset = TB((Uint32)(offset - dataOffset + colorDataSize + pixelDataSize));
					layerHdr->colorCount = TB((Uint16)dpm.colorCount);
					layerHdr->depth = dpm.depth;
					
					if (layerInfo->opts & 1)
					{
						layerHdr->opts = 1;
						layerHdr->transColor = layerInfo->transColor;	// don't need to swap bytes (in RGBA order regardless of endianess)
					}
					
					UMemory::Copy(p + offset, colorTab, colorDataSize);

					dpm.data = p + offset + colorDataSize;
					UGraphics::CopyPixels(dpm, kZeroPoint, img, kZeroPoint, dpm.width, dpm.height);
				}
				
				offset += n;
			}
			else
			{
				Fail(errorType_Misc, error_Unimplemented);
			}
		}
	}
	catch(...)
	{
		UMemory::Dispose(h);
		throw;
	}
	
	return h;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

// returns true if the specified SPixmap appears to be valid
bool UPixmap::IsValid(const SPixmap& inPM)
{
	Uint32 minRB = inPM.width;
	
	switch (inPM.depth)
	{
		case 1:
			minRB = RoundUp8(minRB) >> 3;
			break;
		case 2:
			minRB = RoundUp4(minRB) >> 2;
			break;
		case 4:
			minRB = RoundUp2(minRB) >> 1;
			break;
		case 8:
			break;
		case 16:
			minRB <<= 1;
			break;
		case 24:
			minRB *= 3;
			break;
		case 32:
			minRB <<= 2;
			break;
		default:
			return false;
			break;
	}
	
	return (inPM.rowBytes >= minRB) && (inPM.depth > 8 || inPM.colorCount != 0);
}

// note inHLine should not draw the ending points
// does nothing if the rect is invalid or empty
void UPixmap::ScanFilledRect(const SRect& inRect, THorizLineProc inHLine, void *inRef, Uint32 inColor)
{
	Int32 left = inRect.left;
	Int32 top = inRect.top;
	Int32 right = inRect.right;
	Int32 bottom = inRect.bottom;
	
	if (right > left && bottom > top)	// if valid and not empty
	{
		for (; top != bottom; top++)
			inHLine(inRef, left, top, right, inColor);
	}
}





#if 0


// coords don't include the last pt
typedef void (*THorizLineProc)(void *inRef, Int32 x1, Int32 y1, Int32 x2, Uint32 inColor);

void ScanFilledOval(const SRect& inRect, THorizLineProc inHLine, void *inRef, Uint32 inColor)
{
	Int32 x, y;
	Int32 a, b, c, d;
	Int32 da, db, dc, dd;
	Int32 na, nb, nc, nd;
	Int32 cx, cy, rx, ry;

	// calculate a filled ellipse of radius rx, ry around point cx, cy
	// coords passed to inHLine don't include ending point
	rx = inRect.GetWidth() >> 1;
	ry = inRect.GetHeight() >> 1;
	cx = inRect.left + rx;
	cy = inRect.top + ry;


	if (rx < 1)
		rx = 1;

	if (ry < 1) 
		ry = 1;

	if (rx > ry)
	{
		dc = -1;
		dd = max_Int32;
		x = 0;
		y = rx * 64;
		na = 0;
		nb = (y + 32) >> 6;
		nc = 0;
		nd = (nb * ry) / rx;

		do {
			a = na; 
			b = nb; 
			c = nc; 
			d = nd;

			x = x + (y / rx);
			y = y - (x / rx);
			na = (x + 32) >> 6; 
			nb = (y + 32) >> 6;
			nc = (na * ry) / rx; 
			nd = (nb * ry) / rx;

			if ((c > dc) && (c < dd))
			{
				inHLine(inRef, cx-b, cy+c, cx+b, inColor);
				if (c) inHLine(inRef, cx-b, cy-c, cx+b, inColor);
				dc = c;
			}

			if ((d < dd) && (d > dc))
			{ 
				inHLine(inRef, cx-a, cy+d, cx+a, inColor);
				inHLine(inRef, cx-a, cy-d, cx+a, inColor);
				dd = d;
			}

		} while(b > a);
	} 
	else
	{
		da = -1;
		db = max_Int32;
		x = 0;
		y = ry * 64;
		na = 0;
		nb = (y + 32) >> 6;
		nc = 0;
		nd = (nb * rx) / ry;

		do {
			a = na;
			b = nb;
			c = nc;
			d = nd;

			x = x + (y / ry); 
			y = y - (x / ry);
			na = (x + 32) >> 6; 
			nb = (y + 32) >> 6;
			nc = (na * rx) / ry; 
			nd = (nb * rx) / ry;

			if ((a > da) && (a < db))
			{
				inHLine(inRef, cx-d, cy+a, cx+d, inColor); 
				if (a) inHLine(inRef, cx-d, cy-a, cx+d, inColor);
				da = a;
			}

			if ((b < db) && (b > da))
			{ 
				inHLine(inRef, cx-c, cy+b, cx+c, inColor);
				inHLine(inRef, cx-c, cy-b, cx+c, inColor);
				db = b;
			}

		} while(b > a);
	}
}


#endif	// 0

/* -------------------------------------------------------------------------- */
#pragma mark -

/* an active edge */
struct _POLYGON_EDGE
{
	fast_float x, dx;		// floating point x position and gradient
	fast_float w;			// width of line segment
	Int32 top;				// top y position
	Int32 bottom;			// bottom y position
	_POLYGON_EDGE *prev;	// doubly linked list
	_POLYGON_EDGE *next;
};


#define _POLYGON_FIX_SHIFT     18


/* _poly_fill_edge_structure:
 *  Polygon helper function: initialises an edge structure for the 2d
 *  rasteriser.
 */
static void _poly_fill_edge_structure(_POLYGON_EDGE *edge, Int32 *i1, Int32 *i2)
{
	if (i2[1] < i1[1])
	{
		Int32 *it;

		it = i1;
		i1 = i2;
		i2 = it;
	}

	edge->top = i1[1];
	edge->bottom = i2[1] - 1;
	edge->dx = ((i2[0] - i1[0]) << _POLYGON_FIX_SHIFT) / (i2[1] - i1[1]);
	edge->x = (i1[0] << _POLYGON_FIX_SHIFT) + (1 << (_POLYGON_FIX_SHIFT-1)) - 1;
	edge->prev = NULL;
	edge->next = NULL;

	if (edge->dx < 0)
		edge->x += min(edge->dx + (1 << _POLYGON_FIX_SHIFT), (fast_float)0.0);

	edge->w = max(abs(edge->dx) - (1 << _POLYGON_FIX_SHIFT), (fast_float)0.0);
}

/* _poly_add_edge:
 *  Adds an edge structure to a linked list, returning the new head pointer.
 */
static _POLYGON_EDGE *_poly_add_edge(_POLYGON_EDGE *list, _POLYGON_EDGE *edge, Int32 sort_by_x)
{
	_POLYGON_EDGE *pos = list;
	_POLYGON_EDGE *prev = NULL;

	if (sort_by_x)
	{
		while (pos && ((pos->x + (pos->w + pos->dx) / 2) < (edge->x + (edge->w + edge->dx) / 2)))
		{
			prev = pos;
			pos = pos->next;
		}
	}
	else
	{
		while (pos && (pos->top < edge->top))
		{
			prev = pos;
			pos = pos->next;
		}
	}

	edge->next = pos;
	edge->prev = prev;

	if (pos)
		pos->prev = edge;

	if (prev)
	{
		prev->next = edge;
		return list;
	}
	
	return edge;
}


/* _poly_remove_edge:
 *  Removes an edge structure from a list, returning the new head pointer.
 */
static _POLYGON_EDGE *_poly_remove_edge(_POLYGON_EDGE *list, _POLYGON_EDGE *edge)
{
	if (edge->next) 
		edge->next->prev = edge->prev;

	if (edge->prev)
	{
		edge->prev->next = edge->next;
		return list;
	}

	return edge->next;
}

void UPixmap::ScanFilledPolygon(const SPoint *inPointList, Uint32 inPointCount, THorizLineProc inHLine, void *inRef, Uint32 inColor)
{
	Int32 c;
	Int32 top = max_Int32;
	Int32 bottom = min_Int32;
	Int32 *i1, *i2;
	_POLYGON_EDGE *edge, *next_edge;
	_POLYGON_EDGE *active_edges = NULL;
	_POLYGON_EDGE *inactive_edges = NULL;
	
	// sanity checks
	if (!inPointCount) return;
	
	/*
	 * Allocate space for the edge table from stack or heap
	 */

	Uint8 stack_scratch_mem[4096];
	TPtr heap_scratch_mem = nil;

	Uint32 size_scratch_mem = inPointCount * sizeof(_POLYGON_EDGE);
	
	if (size_scratch_mem > sizeof(stack_scratch_mem))		// if won't fit in stack memory
	{
		heap_scratch_mem = UMemory::New(size_scratch_mem);	// use heap memory instead
		edge = (_POLYGON_EDGE *)heap_scratch_mem;
	}
	else
		edge = (_POLYGON_EDGE *)stack_scratch_mem;
	
	scopekill(TPtrObj, heap_scratch_mem);
	
	/*
	 * Fill the edge table
	 */

	i1 = (Int32 *)inPointList;
	i2 = i1 + (inPointCount-1) * 2;

	for (c=0; c<inPointCount; c++)
	{
		if (i1[1] != i2[1])
		{
			_poly_fill_edge_structure(edge, i1, i2);

			if (edge->bottom >= edge->top)
			{
				if (edge->top < top)
					top = edge->top;

				if (edge->bottom > bottom)
					bottom = edge->bottom;

				inactive_edges = _poly_add_edge(inactive_edges, edge, false);
				edge++;
			}
		}
		i2 = i1;
		i1 += 2;
	}

	//if (bottom >= bmp->cb)	// cb is clip bottom (clipping rectangle)
	//	bottom = bmp->cb-1;

	/* for each scanline in the polygon... */
	for (c=top; c<=bottom; c++)
	{
		/* check for newly active edges */
		edge = inactive_edges;
		while (edge && (edge->top == c))
		{
			next_edge = edge->next;
			inactive_edges = _poly_remove_edge(inactive_edges, edge);
			active_edges = _poly_add_edge(active_edges, edge, true);
			edge = next_edge;
		}

		/* draw horizontal line segments */
		edge = active_edges;
		while (edge && edge->next)
		{
			inHLine(inRef, (Int32)edge->x >> _POLYGON_FIX_SHIFT, c, 
				(Int32)(edge->next->x + edge->next->w) >> _POLYGON_FIX_SHIFT, inColor);
			
			edge = edge->next->next;
		}

		/* update edges, sorting and removing dead ones */
		edge = active_edges;
		while (edge)
		{
			next_edge = edge->next;
			
			if (c >= edge->bottom)
			{
				active_edges = _poly_remove_edge(active_edges, edge);
			}
			else
			{
				edge->x += edge->dx;
				while (edge->prev && (edge->x + (edge->w / 2) < edge->prev->x + (edge->prev->w / 2)))
				{
					if (edge->next)
						edge->next->prev = edge->prev;
					edge->prev->next = edge->next;
					edge->next = edge->prev;
					edge->prev = edge->prev->prev;
					edge->next->prev = edge;
					if (edge->prev)
						edge->prev->next = edge;
					else
						active_edges = edge;
				}
			}
			
			edge = next_edge;
		}
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void _GRCopy32To32(void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount)
{
	Uint32 *sp = (Uint32 *)inSrc;
	Uint32 *dp = (Uint32 *)inDst;
	
	while (inRowCount--)
	{
		for (Uint32 i=0; i!=inWid; i++)
			dp[i] = sp[i];
		
		(Uint8 *)sp += inSrcRowBytes;
		(Uint8 *)dp += inDstRowBytes;
	}
}

// copy AW 32 (RGBA) to AW (also MAC) 16 (ARRRRRGGGGGBBBBB)
void _GRCopy32To16(void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount)
{	
	Uint32 *sp = (Uint32 *)inSrc;
	Uint16 *dp = (Uint16 *)inDst;
	
	while (inRowCount--)
	{
		for (Uint32 i=0; i!=inWid; i++)
			dp[i] = (((sp[i] >> 17) & 0x7C00) | ((sp[i] >> 14) & 0x03E0) | ((sp[i] >> 11) & 0x001F));		

		(Uint8 *)sp += inSrcRowBytes;
		(Uint8 *)dp += inDstRowBytes;
	}
}

// copy AW 24 (RGB) to AW (also MAC) 16 (ARRRRRGGGGGBBBBB)
void _GRCopy24To16(void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount)
{	
	Uint8 *sp = (Uint8 *)inSrc;
	Uint16 *dp = (Uint16 *)inDst;
	
	Int32 srcEnd = inSrcRowBytes - inWid * 3;
	
	while (inRowCount--)
	{
		for (Uint32 i=0; i!=inWid; i++)
		{
			dp[i] = ((sp[0] << 7) & 0x7C00) | ((sp[1] << 2) & 0x03E0) | (sp[2] >> 3);
			sp += 3;
		}

		sp += srcEnd;
		(Uint8 *)dp += inDstRowBytes;
	}
}


// copy AW 32 (RGBA) to any 8
void _GRCopy32To8(void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount, const Uint32 *inDstColors, Uint32 inDstColCount)
{
	Uint32 *sp = (Uint32 *)inSrc;
	Uint8 *dp = (Uint8 *)inDst;
	
	while (inRowCount--)
	{
		for (Uint32 i=0; i!=inWid; i++)
		{
			Uint32 v = sp[i];
			Uint32 d = inDstColCount;
			const Uint32 *p = inDstColors;
			
			while (d--)
			{
				if (*p++ == v)
				{
					dp[i] = p - inDstColors - 1;
					goto nextPix;
				}
			}

			dp[i] = UGraphics::GetNearestColor(v, inDstColors, inDstColCount);
			
			nextPix:;
		}
		
		(Uint8 *)sp += inSrcRowBytes;
		(Uint8 *)dp += inDstRowBytes;
	}
}

// copy AW (also MAC) 16 (ARRRRRGGGGGBBBBB) to AW 32 (RGBA)
void _GRCopy16To32(void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount)
{
	Uint16 *sp = (Uint16 *)inSrc;
	Uint32 *dp = (Uint32 *)inDst;
	
	while (inRowCount--)
	{
		for (Uint32 i=0; i!=inWid; i++)
			dp[i] = _gConv5to8TabR[(sp[i] >> 10) & 0x1F] | _gConv5to8TabG[(sp[i] >> 5) & 0x1F] | _gConv5to8TabB[sp[i] & 0x1F];
		
		(Uint8 *)sp += inSrcRowBytes;
		(Uint8 *)dp += inDstRowBytes;
	}
}

// copy any 16 to 16
void _GRCopy16To16(void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount)
{
	while (inRowCount--)
	{
		for (Uint32 i=0; i!=inWid; i++)
			((Uint16 *)inDst)[i] = ((Uint16 *)inSrc)[i];
		
		(Uint8 *)inSrc += inSrcRowBytes;
		(Uint8 *)inDst += inDstRowBytes;
	}
}

// copy AW (also MAC) 16 (ARRRRRGGGGGBBBBB) to any 8
void _GRCopy16To8(void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount, const Uint32 *inDstColors, Uint32 inDstColCount)
{
	Uint16 *sp = (Uint16 *)inSrc;
	Uint8 *dp = (Uint8 *)inDst;
	
	while (inRowCount--)
	{
		for (Uint32 i=0; i!=inWid; i++)
		{
			Uint32 v = _gConv5to8TabR[(sp[i] >> 10) & 0x1F] | _gConv5to8TabG[(sp[i] >> 5) & 0x1F] | _gConv5to8TabB[sp[i] & 0x1F];
			Uint32 d = inDstColCount;
			const Uint32 *p = inDstColors;
			
			while (d--)
			{
				if (*p++ == v)
				{
					dp[i] = p - inDstColors - 1;
					goto nextPix;
				}
			}

			dp[i] = UGraphics::GetNearestColor(v, inDstColors, inDstColCount);
			
			nextPix:;
		}
		
		(Uint8 *)sp += inSrcRowBytes;
		(Uint8 *)dp += inDstRowBytes;
	}
}

void _GRCopy8To32(void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount, const Uint32 *inSrcColors, Uint32 inSrcColorCount)
{
	Uint8 *sp = (Uint8 *)inSrc;
	Uint32 *dp = (Uint32 *)inDst;
	Uint32 i;
	
	Uint32 colorTabBuf[256];
	const Uint32 *colorTab;

	if (inSrcColorCount > 256) inSrcColorCount = 256;

	if (inSrcColorCount < 256)
	{
		// color table is too small, to avoid possible crashes from corrupt pixel data, build a full-size color table
		colorTab = colorTabBuf;
		
		for (i=0; i!=inSrcColorCount; i++)
			colorTabBuf[i] = inSrcColors[i];
		
		// clear remaining entries in color table because it's too short
		for (i=inSrcColorCount; i!=256; i++)
			colorTabBuf[i] = 0;
	}
	else
	{
		colorTab = inSrcColors;
	}
	
	while (inRowCount--)
	{
		for (i=0; i!=inWid; i++)
			dp[i] = colorTab[sp[i]];
		
		(Uint8 *)sp += inSrcRowBytes;
		(Uint8 *)dp += inDstRowBytes;
	}
}

void _GRCopy8To16(void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount, const Uint32 *inSrcColors, Uint32 inSrcColorCount)
{
	Uint8 *sp = (Uint8 *)inSrc;
	Uint16 *dp = (Uint16 *)inDst;
	Uint32 i;
	
	Uint16 colorTab[256];

	if (inSrcColorCount > 256) inSrcColorCount = 256;

	for (i=0; i!=inSrcColorCount; i++)
		colorTab[i] = (((inSrcColors[i] >> 17) & 0x7C00) | ((inSrcColors[i] >> 14) & 0x03E0) | ((inSrcColors[i] >> 11) & 0x001F));
	
	for (i=inSrcColorCount; i!=256; i++)		// clear any remaining entries in color table just in case color table is too short
		colorTab[i] = 0;

	while (inRowCount--)
	{
		for (i=0; i!=inWid; i++)
			dp[i] = colorTab[sp[i]];
		
		(Uint8 *)sp += inSrcRowBytes;
		(Uint8 *)dp += inDstRowBytes;
	}
}

void _GRCopy8To8(void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount, const Uint32 *inDstColors, Uint32 inDstColCount, const Uint32 *inSrcColors, Uint32 inSrcColCount)
{
	Uint8 *sp = (Uint8 *)inSrc;
	Uint8 *dp = (Uint8 *)inDst;
	Uint32 i;
	
	if (inDstColCount > 256) inDstColCount = 256;
	if (inSrcColCount > 256) inSrcColCount = 256;
	
	if (inDstColCount == inSrcColCount && UMemory::Equal(inDstColors, inSrcColors, inDstColCount * sizeof(Uint32)))	// if exactly the same color tab
	{
		while (inRowCount--)
		{
			for (i=0; i!=inWid; i++)
				dp[i] = sp[i];
			
			(Uint8 *)sp += inSrcRowBytes;
			(Uint8 *)dp += inDstRowBytes;
		}
	}
	else
	{
		Uint8 xlat[256];

		for (i=0; i!=inSrcColCount; i++)
			xlat[i] = UGraphics::GetNearestColor(inSrcColors[i], inDstColors, inDstColCount);
		
		while (inRowCount--)
		{
			for (i=0; i!=inWid; i++)
				dp[i] = xlat[sp[i]];
			
			(Uint8 *)sp += inSrcRowBytes;
			(Uint8 *)dp += inDstRowBytes;
		}
	}
}

void _GRTransCopy32To32(Uint32 inTransCol, void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount)
{
	Uint32 *sp = (Uint32 *)inSrc;
	Uint32 *dp = (Uint32 *)inDst;
	
	while (inRowCount--)
	{
		for (Uint32 i=0; i!=inWid; i++)
		{
			if (sp[i] != inTransCol)
				dp[i] = sp[i];
		}
		
		(Uint8 *)sp += inSrcRowBytes;
		(Uint8 *)dp += inDstRowBytes;
	}
}

// copy AW 32 (RGBA) to AW (also MAC) 16 (ARRRRRGGGGGBBBBB)
void _GRTransCopy32To16(Uint32 inTransCol, void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount)
{
	Uint32 *sp = (Uint32 *)inSrc;
	Uint16 *dp = (Uint16 *)inDst;
	
	while (inRowCount--)
	{
		for (Uint32 i=0; i!=inWid; i++)
		{
			if (sp[i] != inTransCol)
				dp[i] = (((sp[i] >> 17) & 0x7C00) | ((sp[i] >> 14) & 0x03E0) | ((sp[i] >> 11) & 0x001F));
		}
		
		(Uint8 *)sp += inSrcRowBytes;
		(Uint8 *)dp += inDstRowBytes;
	}
}

// copy AW 32 (RGBA) to any 8
void _GRTransCopy32To8(Uint32 inTransCol, void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount, const Uint32 *inDstColors, Uint32 inDstColCount)
{
	Uint32 *sp = (Uint32 *)inSrc;
	Uint8 *dp = (Uint8 *)inDst;
	
	while (inRowCount--)
	{
		for (Uint32 i=0; i!=inWid; i++)
		{
			Uint32 v = sp[i];
			if (v != inTransCol)
			{
				Uint32 d = inDstColCount;
				const Uint32 *p = inDstColors;
				
				while (d--)
				{
					if (*p++ == v)
					{
						dp[i] = p - inDstColors - 1;
						goto nextPix;
					}
				}

				dp[i] = UGraphics::GetNearestColor(v, inDstColors, inDstColCount);
			}
			
			nextPix:;
		}
		
		(Uint8 *)sp += inSrcRowBytes;
		(Uint8 *)dp += inDstRowBytes;
	}
}

// copy AW (also MAC) 16 (ARRRRRGGGGGBBBBB) to AW 32 (RGBA) with a mask
void _GRTransCopy16To32(const void *inMask, void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount)

{
	Uint16 *mask = (Uint16 *)inMask;
	Uint16 *sp = (Uint16 *)inSrc;
	Uint32 *dp = (Uint32 *)inDst;

	while (inRowCount--)
	{
		for (Uint32 i=0; i!=inWid; i++)
		{
			if (mask[i] == 0)
				dp[i] = _gConv5to8TabR[(sp[i] >> 10) & 0x1F] | _gConv5to8TabG[(sp[i] >> 5) & 0x1F] | _gConv5to8TabB[sp[i] & 0x1F];
		}

		(Uint8 *)mask += inSrcRowBytes;
		(Uint8 *)sp += inSrcRowBytes;
		(Uint8 *)dp += inDstRowBytes;
	}
}

// copy AW (also MAC) 16 (ARRRRRGGGGGBBBBB) to AW 32 (RGBA)
void _GRTransCopy16To32(Uint32 inTransCol, void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount)
{
	Uint16 *sp = (Uint16 *)inSrc;
	Uint32 *dp = (Uint32 *)inDst;
	
	Uint16 transCol = (((inTransCol >> 17) & 0x7C00) | ((inTransCol >> 14) & 0x03E0) | ((inTransCol >> 11) & 0x001F));
	
	while (inRowCount--)
	{
		for (Uint32 i=0; i!=inWid; i++)
		{
			if (sp[i] != transCol)
				dp[i] = _gConv5to8TabR[(sp[i] >> 10) & 0x1F] | _gConv5to8TabG[(sp[i] >> 5) & 0x1F] | _gConv5to8TabB[sp[i] & 0x1F];
		}
		
		(Uint8 *)sp += inSrcRowBytes;
		(Uint8 *)dp += inDstRowBytes;
	}
}


// copy any 16 to 16 with a mask

void _GRTransCopy16To16(const void *inMask, void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount)
{
	Uint16 *mask = (Uint16 *)inMask;
	Uint16 *sp = (Uint16 *)inSrc;
	Uint16 *dp = (Uint16 *)inDst;
		
	while (inRowCount--)
	{
		for (Uint32 i=0; i!=inWid; i++)
		{
			if (mask[i] == 0)
				dp[i] = sp[i];
		}

		(Uint8 *)mask += inSrcRowBytes;
		(Uint8 *)sp += inSrcRowBytes;
		(Uint8 *)dp += inDstRowBytes;
	}
}


// copy any 16 to 16
void _GRTransCopy16To16(Uint32 inTransCol, void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount)
{
	Uint16 *sp = (Uint16 *)inSrc;
	Uint16 *dp = (Uint16 *)inDst;
	
	Uint16 transCol = (((inTransCol >> 17) & 0x7C00) | ((inTransCol >> 14) & 0x03E0) | ((inTransCol >> 11) & 0x001F));
	
	while (inRowCount--)
	{
		for (Uint32 i=0; i!=inWid; i++)
		{
			if (sp[i] != transCol)
				dp[i] = sp[i];
		}
		
		(Uint8 *)sp += inSrcRowBytes;
		(Uint8 *)dp += inDstRowBytes;
	}
}

// copy AW (also MAC) 16 (ARRRRRGGGGGBBBBB) to any 8
void _GRTransCopy16To8(Uint32 inTransCol, void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount, const Uint32 *inDstColors, Uint32 inDstColCount)
{
	Uint16 *sp = (Uint16 *)inSrc;
	Uint8 *dp = (Uint8 *)inDst;
	
	Uint16 transCol = (((inTransCol >> 17) & 0x7C00) | ((inTransCol >> 14) & 0x03E0) | ((inTransCol >> 11) & 0x001F));
	
	while (inRowCount--)
	{
		for (Uint32 i=0; i!=inWid; i++)
		{
			if (sp[i] != transCol)
			{
				Uint32 v = _gConv5to8TabR[(sp[i] >> 10) & 0x1F] | _gConv5to8TabG[(sp[i] >> 5) & 0x1F] | _gConv5to8TabB[sp[i] & 0x1F];
				Uint32 d = inDstColCount;
				const Uint32 *p = inDstColors;
				
				while (d--)
				{
					if (*p++ == v)
					{
						dp[i] = p - inDstColors - 1;
						goto nextPix;
					}
				}

				dp[i] = UGraphics::GetNearestColor(v, inDstColors, inDstColCount);
			}
			
			nextPix:;
		}
		
		(Uint8 *)sp += inSrcRowBytes;
		(Uint8 *)dp += inDstRowBytes;
	}
}

void _GRTransCopy8To32(Uint32 inTransCol, void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount, const Uint32 *inSrcColors, Uint32 inSrcColorCount)
{
	Uint8 *sp = (Uint8 *)inSrc;
	Uint32 *dp = (Uint32 *)inDst;
	Uint32 i;
	
	Uint32 colorTabBuf[256];
	const Uint32 *colorTab;

	if (inSrcColorCount > 256) inSrcColorCount = 256;

	if (inSrcColorCount < 256)
	{
		colorTab = colorTabBuf;
		
		for (i=0; i!=inSrcColorCount; i++)
			colorTabBuf[i] = inSrcColors[i];
		
		// clear remaining entries in color table because it's too short
		for (i=inSrcColorCount; i!=256; i++)
			colorTabBuf[i] = 0;
	}
	else
	{
		colorTab = inSrcColors;
	}
	
	if (UGraphics::GetExactColor(inTransCol, inSrcColors, inSrcColorCount, i))
	{
		Uint8 transCol = i;

		while (inRowCount--)
		{
			for (i=0; i!=inWid; i++)
			{
				if (sp[i] != transCol)
					dp[i] = colorTab[sp[i]];
			}
			
			(Uint8 *)sp += inSrcRowBytes;
			(Uint8 *)dp += inDstRowBytes;
		}
	}
	else
	{
		while (inRowCount--)
		{
			for (i=0; i!=inWid; i++)
				dp[i] = colorTab[sp[i]];
			
			(Uint8 *)sp += inSrcRowBytes;
			(Uint8 *)dp += inDstRowBytes;
		}
	}
}

void _GRTransCopy8To16(Uint32 inTransCol, void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount, const Uint32 *inSrcColors, Uint32 inSrcColorCount)
{
	Uint8 *sp = (Uint8 *)inSrc;
	Uint16 *dp = (Uint16 *)inDst;
	Uint32 i;
	
	Uint16 colorTab[256];

	if (inSrcColorCount > 256) inSrcColorCount = 256;

	for (i=0; i!=inSrcColorCount; i++)
		colorTab[i] = (((inSrcColors[i] >> 17) & 0x7C00) | ((inSrcColors[i] >> 14) & 0x03E0) | ((inSrcColors[i] >> 11) & 0x001F));
	
	for (i=inSrcColorCount; i!=256; i++)		// clear any remaining entries in color table just in case color table is too short
		colorTab[i] = 0;

	if (UGraphics::GetExactColor(inTransCol, inSrcColors, inSrcColorCount, i))
	{
		Uint8 transCol = i;

		while (inRowCount--)
		{
			for (i=0; i!=inWid; i++)
			{
				if (sp[i] != transCol)
					dp[i] = colorTab[sp[i]];
			}
			
			(Uint8 *)sp += inSrcRowBytes;
			(Uint8 *)dp += inDstRowBytes;
		}
	}
	else
	{
		while (inRowCount--)
		{
			for (i=0; i!=inWid; i++)
				dp[i] = colorTab[sp[i]];
			
			(Uint8 *)sp += inSrcRowBytes;
			(Uint8 *)dp += inDstRowBytes;
		}
	}
}

void _GRTransCopy8To8(Uint32 inTransCol, void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount, const Uint32 *inDstColors, Uint32 inDstColCount, const Uint32 *inSrcColors, Uint32 inSrcColCount)
{
	Uint8 *sp = (Uint8 *)inSrc;
	Uint8 *dp = (Uint8 *)inDst;
	Uint32 i;
	Uint8 transCol;
	
	if (inDstColCount > 256) inDstColCount = 256;
	if (inSrcColCount > 256) inSrcColCount = 256;
	
	if (inDstColCount == inSrcColCount && UMemory::Equal(inDstColors, inSrcColors, inDstColCount * sizeof(Uint32)))	// if exactly the same color tab
	{
		if (UGraphics::GetExactColor(inTransCol, inSrcColors, inSrcColCount, i))
		{
			transCol = i;
			
			while (inRowCount--)
			{
				for (i=0; i!=inWid; i++)
				{
					if (sp[i] != transCol)
						dp[i] = sp[i];
				}
				
				(Uint8 *)sp += inSrcRowBytes;
				(Uint8 *)dp += inDstRowBytes;
			}
		}
		else
		{
			while (inRowCount--)
			{
				for (i=0; i!=inWid; i++)
					dp[i] = sp[i];
				
				(Uint8 *)sp += inSrcRowBytes;
				(Uint8 *)dp += inDstRowBytes;
			}
		}
	}
	else
	{
		Uint8 xlat[256];

		for (i=0; i!=inSrcColCount; i++)
			xlat[i] = UGraphics::GetNearestColor(inSrcColors[i], inDstColors, inDstColCount);
		
		if (UGraphics::GetExactColor(inTransCol, inSrcColors, inSrcColCount, i))
		{
			transCol = i;
			
			while (inRowCount--)
			{
				for (i=0; i!=inWid; i++)
				{
					if (sp[i] != transCol)
						dp[i] = xlat[sp[i]];
				}
				
				(Uint8 *)sp += inSrcRowBytes;
				(Uint8 *)dp += inDstRowBytes;
			}
		}
		else
		{
			while (inRowCount--)
			{
				for (i=0; i!=inWid; i++)
					dp[i] = xlat[sp[i]];
				
				(Uint8 *)sp += inSrcRowBytes;
				(Uint8 *)dp += inDstRowBytes;
			}
		}
	}
}

void _GRTrans32ToMask(Uint32 inTransCol, void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount)
{
	Uint32 *sp = (Uint32 *)inSrc;
	Uint8 *dp = (Uint8 *)inDst;

	UMemory::Fill(inDst, inRowCount * inDstRowBytes, 0);
	
	while (inRowCount--)
	{
		for (Uint32 i=0; i!=inWid; i++)
		{
			if (sp[i] != inTransCol)
				SetBit(dp, i);
		}
		
		(Uint8 *)sp += inSrcRowBytes;
		(Uint8 *)dp += inDstRowBytes;
	}
}

void _GRTrans16ToMask(Uint32 inTransCol, void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount)
{
	Uint16 *sp = (Uint16 *)inSrc;
	Uint8 *dp = (Uint8 *)inDst;

	Uint16 transCol = (((inTransCol >> 17) & 0x7C00) | ((inTransCol >> 14) & 0x03E0) | ((inTransCol >> 11) & 0x001F));

	UMemory::Fill(inDst, inRowCount * inDstRowBytes, 0);
	
	while (inRowCount--)
	{
		for (Uint32 i=0; i!=inWid; i++)
		{
			if (sp[i] != transCol)
				SetBit(dp, i);
		}
		
		(Uint8 *)sp += inSrcRowBytes;
		(Uint8 *)dp += inDstRowBytes;
	}
}

void _GRTrans8ToMask(Uint32 inTransCol, void *inDst, Uint32 inDstRowBytes, const void *inSrc, Uint32 inSrcRowBytes, Uint32 inWid, Uint32 inRowCount, const Uint32 *inSrcColors, Uint32 inSrcColorCount)
{
	Uint32 ti;

	if (UGraphics::GetExactColor(inTransCol, inSrcColors, inSrcColorCount, ti))
	{
		Uint8 *sp = (Uint8 *)inSrc;
		Uint8 *dp = (Uint8 *)inDst;
		Uint8 transCol = ti;
		
		UMemory::Fill(inDst, inRowCount * inDstRowBytes, 0);
		
		while (inRowCount--)
		{
			for (Uint32 i=0; i!=inWid; i++)
			{
				if (sp[i] != transCol)
					SetBit(dp, i);
			}
			
			(Uint8 *)sp += inSrcRowBytes;
			(Uint8 *)dp += inDstRowBytes;
		}
	}
	else
	{
		UMemory::Fill(inDst, inRowCount * inDstRowBytes, 0xFF);
	}
}

void UPixmap::CopyPixels(const SPixmap& inDest, const SPoint& inDestPt, const SPixmap& inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 /* inOptions */)
{
	// sanity checks on inSource
	if (!UPixmap::IsValid(inSource))
	{
		DebugBreak("UPixmap::CopyPixels - invalid source pixmap");
		Fail(errorType_Misc, error_Param);
	}
	
	// sanity checks on inDest
	if (!UPixmap::IsValid(inDest))
	{
		DebugBreak("UPixmap::CopyPixels - invalid destination pixmap");
		Fail(errorType_Misc, error_Param);
	}

	// validate the copy rects
	SPoint theDestPt = inDestPt;
	SPoint theSourcePt = inSourcePt;
	SRect srcBnd(0, 0, inSource.width, inSource.height);
	SRect dstBnd(0, 0, inDest.width, inDest.height);
	if (!UGraphics::ValidateCopyRects(dstBnd, srcBnd, theDestPt, theSourcePt, inWidth, inHeight))
		return;

	// switch on depth and perform appropriate copy
	switch (inSource.depth)
	{
		case 32:
		{
			switch (inDest.depth)
			{
				case 32:
					_GRCopy32To32(((Uint32 *)inDest.data) + theDestPt.x, inDest.rowBytes, ((Uint32 *)inSource.data) + theSourcePt.x, inSource.rowBytes, inWidth, inHeight);
					break;
				case 16:
					_GRCopy32To16(((Uint16 *)inDest.data) + theDestPt.x, inDest.rowBytes, ((Uint32 *)inSource.data) + theSourcePt.x, inSource.rowBytes, inWidth, inHeight);
					break;
				case 8:
					_GRCopy32To8(((Uint8 *)inDest.data) + theDestPt.x, inDest.rowBytes, ((Uint32 *)inSource.data) + theSourcePt.x, inSource.rowBytes, inWidth, inHeight, inDest.colorTab, inDest.colorCount);
					break;
				case 4:
				case 2:
				case 1:
					Fail(errorType_Misc, error_Unimplemented);
					break;
				default:
					Fail(errorType_Misc, error_Param);
					break;
			}
		}
		break;
		
		case 16:
		{
			switch (inDest.depth)
			{
				case 32:
					_GRCopy16To32(((Uint32 *)inDest.data) + theDestPt.x, inDest.rowBytes, ((Uint16 *)inSource.data) + theSourcePt.x, inSource.rowBytes, inWidth, inHeight);
					break;
				case 16:
					_GRCopy16To16(((Uint16 *)inDest.data) + theDestPt.x, inDest.rowBytes, ((Uint16 *)inSource.data) + theSourcePt.x, inSource.rowBytes, inWidth, inHeight);
					break;
				case 8:
					_GRCopy16To8(((Uint8 *)inDest.data) + theDestPt.x, inDest.rowBytes, ((Uint16 *)inSource.data) + theSourcePt.x, inSource.rowBytes, inWidth, inHeight, inDest.colorTab, inDest.colorCount);
					break;
				case 4:
				case 2:
				case 1:
					Fail(errorType_Misc, error_Unimplemented);
					break;
				default:
					Fail(errorType_Misc, error_Param);
					break;
			}
		}
		break;
		
		case 8:
		{
			switch (inDest.depth)
			{
				case 32:
					_GRCopy8To32(((Uint32 *)inDest.data) + theDestPt.x, inDest.rowBytes, ((Uint8 *)inSource.data) + theSourcePt.x, inSource.rowBytes, inWidth, inHeight, inSource.colorTab, inSource.colorCount);
					break;
				case 16:
					_GRCopy8To16(((Uint16 *)inDest.data) + theDestPt.x, inDest.rowBytes, ((Uint8 *)inSource.data) + theSourcePt.x, inSource.rowBytes, inWidth, inHeight, inSource.colorTab, inSource.colorCount);
					break;
				case 8:
					_GRCopy8To8(((Uint8 *)inDest.data) + theDestPt.x, inDest.rowBytes, ((Uint8 *)inSource.data) + theSourcePt.x, inSource.rowBytes, inWidth, inHeight, inDest.colorTab, inDest.colorCount, inSource.colorTab, inSource.colorCount);
					break;
				case 4:
				case 2:
				case 1:
					Fail(errorType_Misc, error_Unimplemented);
					break;
				default:
					Fail(errorType_Misc, error_Param);
					break;
			}
		}
		break;
		
		case 4:
		case 2:
		case 1:
			Fail(errorType_Misc, error_Unimplemented);
			break;
		default:
			Fail(errorType_Misc, error_Param);
			break;
	}
	
}

void UPixmap::CopyPixelsTrans(const SPixmap& inDest, const SPoint& inDestPt, const SPixmap& inSource, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inTransCol, Uint32 /* inOptions */)
{
	// sanity checks on inSource
	if (!UPixmap::IsValid(inSource))
	{
		DebugBreak("UPixmap::CopyPixels - invalid source pixmap");
		Fail(errorType_Misc, error_Param);
	}
	
	// sanity checks on inDest
	if (!UPixmap::IsValid(inDest))
	{
		DebugBreak("UPixmap::CopyPixels - invalid destination pixmap");
		Fail(errorType_Misc, error_Param);
	}

	// validate the copy rects
	SPoint theDestPt = inDestPt;
	SPoint theSourcePt = inSourcePt;
	SRect srcBnd(0, 0, inSource.width, inSource.height);
	SRect dstBnd(0, 0, inDest.width, inDest.height);
	if (!UGraphics::ValidateCopyRects(dstBnd, srcBnd, theDestPt, theSourcePt, inWidth, inHeight))
		return;

	// switch on depth and perform appropriate copy
	switch (inSource.depth)
	{
		case 32:
		{
			switch (inDest.depth)
			{
				case 32:
					_GRTransCopy32To32(inTransCol, ((Uint32 *)inDest.data) + theDestPt.x, inDest.rowBytes, ((Uint32 *)inSource.data) + theSourcePt.x, inSource.rowBytes, inWidth, inHeight);
					break;
				case 16:
					_GRTransCopy32To16(inTransCol, ((Uint16 *)inDest.data) + theDestPt.x, inDest.rowBytes, ((Uint32 *)inSource.data) + theSourcePt.x, inSource.rowBytes, inWidth, inHeight);
					break;
				case 8:
					_GRTransCopy32To8(inTransCol, ((Uint8 *)inDest.data) + theDestPt.x, inDest.rowBytes, ((Uint32 *)inSource.data) + theSourcePt.x, inSource.rowBytes, inWidth, inHeight, inDest.colorTab, inDest.colorCount);
					break;
				case 4:
				case 2:
				case 1:
					Fail(errorType_Misc, error_Unimplemented);
					break;
				default:
					Fail(errorType_Misc, error_Param);
					break;
			}
		}
		break;
		
		case 16:
		{
			switch (inDest.depth)
			{
				case 32:
					_GRTransCopy16To32(inTransCol, ((Uint32 *)inDest.data) + theDestPt.x, inDest.rowBytes, ((Uint16 *)inSource.data) + theSourcePt.x, inSource.rowBytes, inWidth, inHeight);
					break;
				case 16:
					_GRTransCopy16To16(inTransCol, ((Uint16 *)inDest.data) + theDestPt.x, inDest.rowBytes, ((Uint16 *)inSource.data) + theSourcePt.x, inSource.rowBytes, inWidth, inHeight);
					break;
				case 8:
					_GRTransCopy16To8(inTransCol, ((Uint8 *)inDest.data) + theDestPt.x, inDest.rowBytes, ((Uint16 *)inSource.data) + theSourcePt.x, inSource.rowBytes, inWidth, inHeight, inDest.colorTab, inDest.colorCount);
					break;
				case 4:
				case 2:
				case 1:
					Fail(errorType_Misc, error_Unimplemented);
					break;
				default:
					Fail(errorType_Misc, error_Param);
					break;
			}
		}
		break;
		
		case 8:
		{
			switch (inDest.depth)
			{
				case 32:
					_GRTransCopy8To32(inTransCol, ((Uint32 *)inDest.data) + theDestPt.x, inDest.rowBytes, ((Uint8 *)inSource.data) + theSourcePt.x, inSource.rowBytes, inWidth, inHeight, inSource.colorTab, inSource.colorCount);
					break;
				case 16:
					_GRTransCopy8To16(inTransCol, ((Uint16 *)inDest.data) + theDestPt.x, inDest.rowBytes, ((Uint8 *)inSource.data) + theSourcePt.x, inSource.rowBytes, inWidth, inHeight, inSource.colorTab, inSource.colorCount);
					break;
				case 8:
					_GRTransCopy8To8(inTransCol, ((Uint8 *)inDest.data) + theDestPt.x, inDest.rowBytes, ((Uint8 *)inSource.data) + theSourcePt.x, inSource.rowBytes, inWidth, inHeight, inDest.colorTab, inDest.colorCount, inSource.colorTab, inSource.colorCount);
					break;
				case 4:
				case 2:
				case 1:
					Fail(errorType_Misc, error_Unimplemented);
					break;
				default:
					Fail(errorType_Misc, error_Param);
					break;
			}
		}
		break;
		
		case 4:
		case 2:
		case 1:
			Fail(errorType_Misc, error_Unimplemented);
			break;
		default:
			Fail(errorType_Misc, error_Param);
			break;
	}
	
}

