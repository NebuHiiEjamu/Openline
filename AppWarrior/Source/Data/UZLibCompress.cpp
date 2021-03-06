/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "UZLibCompress.h"
#include "zlib.h"

struct SZLibObj
{
	z_stream strm;
	
	// allow us to make a ratio for smarter allocation in Compress/Decompress
	Uint32 totalInData;
	Uint32 totalOutData;
	
	TPtr nextOut;
};


//#define REF	((SZLibObj *)inRef)

voidpf _ZLAlloc(voidpf opaque, uInt items, uInt size);
void _ZLDealloc(voidpf opaque, voidpf address);




TZCompress UZlibCompress::New(Int32 inLevel)
{
	if(inLevel < zCompressLevel_None || inLevel > zCompressLevel_Best)
		inLevel = zCompressLevel_Default;

	SZLibObj *obj = (SZLibObj *)UMemory::NewClear(sizeof(SZLibObj));

	obj->strm.zalloc = _ZLAlloc;
	obj->strm.zfree = _ZLDealloc;
	
	deflateInit(&obj->strm, inLevel);
	
	return (TZCompress)obj;
}



void UZlibCompress::Dispose(TZCompress inRef)
{
	Require(inRef);
	
	deflateEnd(&((SZLibObj *)inRef)->strm);
	
	UMemory::Dispose(((SZLibObj *)inRef)->nextOut);
	UMemory::Dispose((TPtr)inRef);
}

TPtr UZlibCompress::Compress(TZCompress inRef, const void *inData, Uint32 inDataSize, Uint32 &outSize, bool inFlush)
{
	Uint32 s = 1;
	if(!((SZLibObj *)inRef)->nextOut)
		((SZLibObj *)inRef)->nextOut = UMemory::New(s);
	
	TPtr p = ((SZLibObj *)inRef)->nextOut;
	
	((SZLibObj *)inRef)->strm.next_out = BPTR(p);
	((SZLibObj *)inRef)->strm.avail_out = s;
	((SZLibObj *)inRef)->strm.next_in = BPTR(inData);
	((SZLibObj *)inRef)->strm.avail_in = inDataSize;

	int flush = (inFlush ? Z_FULL_FLUSH : Z_NO_FLUSH);
	
	// process inData
	for(;;)
	{
		deflate(&((SZLibObj *)inRef)->strm, flush);

		if(((SZLibObj *)inRef)->strm.avail_in != 0 || ((SZLibObj *)inRef)->strm.avail_out == 0)
		{
			// reallocate ptr
			// find the size we need
			Uint32 usedSize = s - ((SZLibObj *)inRef)->strm.avail_out;
			s += (fast_float)((SZLibObj *)inRef)->totalOutData * (fast_float)(inDataSize - ((SZLibObj *)inRef)->strm.avail_in) / (fast_float)(((SZLibObj *)inRef)->totalInData + 1) + 1024;
			p = UMemory::Reallocate(p, s);
			((SZLibObj *)inRef)->nextOut = nil;
			((SZLibObj *)inRef)->strm.next_out = BPTR(p) + usedSize;
			((SZLibObj *)inRef)->strm.avail_out = s - usedSize;
			
		}
		else
			break;
	}
	
	// truncate the ptr to the exact data size
	if(((SZLibObj *)inRef)->nextOut)
	{
		// we didn't use our ptr
		// return 0
		s = 0;
		p = nil;
	}
	else if(((SZLibObj *)inRef)->strm.avail_out)
	{
		s -= ((SZLibObj *)inRef)->strm.avail_out;
		p = UMemory::Reallocate(p, s);
	}

	outSize = s;
	((SZLibObj *)inRef)->totalOutData += s;
	((SZLibObj *)inRef)->totalInData += inDataSize;
	return p;
}


Uint32 UZlibCompress::Process(TZCompress inRef, Uint8 *&ioRawData, Uint32 &ioRawSize, Uint8 *outCompressData, Uint32 inCompressMaxSize, bool inFinish)
{
	((SZLibObj *)inRef)->strm.next_out = outCompressData;
	((SZLibObj *)inRef)->strm.avail_out = inCompressMaxSize;
	((SZLibObj *)inRef)->strm.next_in = ioRawData;
	((SZLibObj *)inRef)->strm.avail_in = ioRawSize;

	deflate(&((SZLibObj *)inRef)->strm, inFinish ? Z_FULL_FLUSH : Z_NO_FLUSH);
	
	ioRawData += ioRawSize - ((SZLibObj *)inRef)->strm.avail_in;
	ioRawSize = ((SZLibObj *)inRef)->strm.avail_in;
	return inCompressMaxSize - ((SZLibObj *)inRef)->strm.avail_out;
}



#pragma mark -

TZDecompress UZlibDecompress::New()
{
	SZLibObj *obj = (SZLibObj *)UMemory::NewClear(sizeof(SZLibObj));
	
	obj->strm.zalloc = _ZLAlloc;
	obj->strm.zfree = _ZLDealloc;
	
	inflateInit(&obj->strm);
	
	return (TZDecompress)obj;
}


void UZlibDecompress::Dispose(TZDecompress inRef)
{
	Require(inRef);

	inflateEnd(&((SZLibObj *)inRef)->strm);
	
	UMemory::Dispose(((SZLibObj *)inRef)->nextOut);
	UMemory::Dispose((TPtr)inRef);
}


TPtr UZlibDecompress::Decompress(TZDecompress inRef, const void *inData, Uint32 inDataSize, Uint32 &outSize)
{
	Uint32 s = 1;
	if(!((SZLibObj *)inRef)->nextOut)
		((SZLibObj *)inRef)->nextOut = UMemory::New(s);
	
	TPtr p = ((SZLibObj *)inRef)->nextOut;
	
	((SZLibObj *)inRef)->strm.next_out = BPTR(p);
	((SZLibObj *)inRef)->strm.avail_out = s;
	((SZLibObj *)inRef)->strm.next_in = BPTR(inData);
	((SZLibObj *)inRef)->strm.avail_in = inDataSize;

	// process inData
	for(;;)
	{
		inflate(&((SZLibObj *)inRef)->strm, Z_NO_FLUSH);

		if(((SZLibObj *)inRef)->strm.avail_in != 0 || ((SZLibObj *)inRef)->strm.avail_out == 0)
		{
			// reallocate ptr
			// find the size we need
			Uint32 usedSize = s - ((SZLibObj *)inRef)->strm.avail_out;
			s += (fast_float)((SZLibObj *)inRef)->totalOutData * (fast_float)(inDataSize - ((SZLibObj *)inRef)->strm.avail_in) / (fast_float)(((SZLibObj *)inRef)->totalInData + 1) + 1024;
			p = UMemory::Reallocate(p, s);
			((SZLibObj *)inRef)->nextOut = nil;
			((SZLibObj *)inRef)->strm.next_out = BPTR(p) + usedSize;
			((SZLibObj *)inRef)->strm.avail_out = s - usedSize;
			
		}
		else
			break;
	}
	
	// truncate the ptr to the exact data size
	if(((SZLibObj *)inRef)->nextOut)
	{
		// we didn't use our ptr
		// return 0
		s = 0;
		p = nil;
	}
	else if(((SZLibObj *)inRef)->strm.avail_out)
	{
		s -= ((SZLibObj *)inRef)->strm.avail_out;
		p = UMemory::Reallocate(p, s);
	}

	outSize = s;
	((SZLibObj *)inRef)->totalOutData += s;
	((SZLibObj *)inRef)->totalInData += inDataSize;
	return p;
}


Uint32 UZlibDecompress::Process(TZDecompress inRef, Uint8 *&ioCompressedData, Uint32 &ioCompressedDataSize, Uint8 *outRawData, Uint32 inRawMaxSize)
{
	((SZLibObj *)inRef)->strm.next_out = outRawData;
	((SZLibObj *)inRef)->strm.avail_out = inRawMaxSize;
	((SZLibObj *)inRef)->strm.next_in = ioCompressedData;
	((SZLibObj *)inRef)->strm.avail_in = ioCompressedDataSize;

	inflate(&((SZLibObj *)inRef)->strm, Z_NO_FLUSH);
	
	ioCompressedData += ioCompressedDataSize - ((SZLibObj *)inRef)->strm.avail_in;
	ioCompressedDataSize = ((SZLibObj *)inRef)->strm.avail_in;
	return inRawMaxSize - ((SZLibObj *)inRef)->strm.avail_out;
}

#pragma mark -

#if MACINTOSH
// on the mac, use temp system memory from the system heap rather than the application heap
voidpf _ZLAlloc(voidpf opaque, uInt items, uInt size)
{
	#pragma unused(opaque)
	
	void *ptr = nil;
	
	Uint32 s = items * size;
	
	try
	{
		THdl hdl = UMemory::NewHandle(s + 4, 1);
		ptr = hdl->Lock();
		*((Uint32 *)ptr)++ = (Uint32)hdl;
	}
	catch(...)
	{
		return nil;
	}

	return ptr;
}

void _ZLDealloc(voidpf opaque, voidpf address)
{
	#pragma unused(opaque)
	
	Require(address);
	
	THdl h = (THdl)*((Uint32 *)(BPTR(address) - 4));
	h->Unlock();
	delete h;

}

#else

voidpf _ZLAlloc(voidpf opaque, uInt items, uInt size)
{
	#pragma unused(opaque)
	
	TPtr ptr = nil;
	
	
	Uint32 s = items * size;
	if(s == 0)
		s = 1;	// just in case it tries to allocate 0 bytes, it may still want a valid obj
	
	try
	{
		ptr = UMemory::New(s);
	}
	catch(...)
	{
		return nil;
	}

	return ptr;
}

void _ZLDealloc(voidpf opaque, voidpf address)
{
	#pragma unused(opaque)
	
	Require(address);
	
	UMemory::Dispose((TPtr)address);
}


#endif
