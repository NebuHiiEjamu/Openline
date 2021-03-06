/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
/*

-- Misc Notes --

Hertz (Hz) is the measurement used for the rate at which a sound is played.  It 
is the number of samples per second, which is stored as a Uint32.  A kilohertz
(kHz) is 1000 hertz.

Some popular sampling rates are:

11025	Quarter CD sampling rate
22050	Half CD sampling rate
44100	CD sampling rate

A single sample is either 8 or 16 bits - a signed integer (in portable format,
not host format!).  Note that because sound is really vibration, it is the 
change in samples that produces an audible sound.  The reason samples are 
signed is because the speaker cone hardware has polarity - a negative and a 
positive side.

When a sound has multiple channels (eg stereo is two channels), the channels 
are stored in an interleaved arrangement, eg LRLRLR not LLLRRR.  This is so 
that the samples that are played simultaneously are stored next to each other.

ADPCM sound compression gives exactly 4:1 compression on 16-bit sound, and 2:1
compression on 8-bit sound (since the compressed size is the same for both,
you would only ever use 8-bit if you don't have 16-bit samples).

-- Sound Format --

Format of the flattened sound:

Uint32 format;				// always AWSN
Uint16 version;				// currently 1
Uint8 chanCount;			// number of channels, usually 1 (mono) or 2 (stereo)
Uint8 sampleSize;			// in bytes, 1 or 2 (8 or 16 bit)
Uint32 sampleRate;			// samples per second (hertz)
Uint32 sampleCount;			// number of samples
Uint32 rsvdA;				// reserved, should be 0
Uint32 rsvdB;				// reserved, should be 0
Uint32 compType;			// compression type, 0=none, 'IPCM'=Intel/DVI ADPCM
Uint32 dataSize;			// number of bytes in following array
Uint8 sampleData[dataSize];	// signed samples or compressed data

*/

#include "USound.h"

struct _adpcm_state {
    int valprev;	/* Previous output value */
    int index;		/* Index into stepsize table */
};

/* Intel ADPCM step variation table */
static int _adpcm_indexTable[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8,
};

static int _adpcm_stepsizeTable[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

#pragma options align=packed
struct SAWSoundHdr {
	Uint32 format;				// always AWSN
	Uint16 version;				// currently 1
	Uint8 chanCount;			// number of channels, usually 1 (mono) or 2 (stereo)
	Uint8 sampleSize;			// in bytes, 1 or 2 (8 or 16 bit)
	Uint32 sampleRate;			// samples per second (hertz)
	Uint32 sampleCount;			// number of samples
	Uint32 rsvdA;				// reserved, should be 0
	Uint32 rsvdB;				// reserved, should be 0
	Uint32 compType;			// compression type, 0=none, 'IPCM'=Intel/DVI ADPCM
	Uint32 dataSize;			// number of bytes in following array
	Uint8 sampleData[];			// signed samples or compressed data
};
#pragma options align=reset

Uint8 _gDisableSound = false;

/* -------------------------------------------------------------------------- */

static void _SNPlayHdlMsgHandler(void *inContext, void *inObject, Uint32 inMsg, const void */* inData */, Uint32 /* inDataSize */)
{
	if (inMsg == msg_ReleaseBuffer)	// if sound has finished playing
	{
		// unlock the sound handle (we locked it before we started playing it)
		UMemory::Unlock((THdl)inContext);
		
		// extract the msg proc that we'll be sending our msg to
		Uint32 *lp = (Uint32 *)USound::GetOutputScratch((TSoundOutput)inObject);
		TMessageProc msgProc = (TMessageProc)( lp[0] );
		void *msgContext = (void *)( lp[1] );

		// the sound has finished so we're done with the output, kill it
		USound::DisposeOutput((TSoundOutput)inObject);
		
		// send the completion message
		if (msgProc) UApplication::SendMessage(msg_ReleaseBuffer, nil, 0, priority_Normal, msgProc, msgContext, inContext);
	}
}

static void _SNPlayRezMsgHandler(void *inContext, void *inObject, Uint32 inMsg, const void */* inData */, Uint32 /* inDataSize */)
{
	if (inMsg == msg_ReleaseBuffer)	// if sound has finished playing
	{
		// unlock the sound handle (we locked it before we started playing it)
		UMemory::Unlock((THdl)inContext);
		
		// extract the msg proc that we'll be sending our msg to
		Uint32 *lp = (Uint32 *)USound::GetOutputScratch((TSoundOutput)inObject);
		
		// release the resource
		URez::ReleaseItem((TRez)lp[0], 'SOUN', (Int32)lp[1]);
		
		// the sound has finished so we're done with the output, kill it
		USound::DisposeOutput((TSoundOutput)inObject);
	}
}

/*
 * USound::Play() plays the sound <inHdl> asyncronously, and <inHdl> must remain valid until it 
 * is finished playing.  If <inMsgProc> is not nil, a msg_ReleaseBuffer will be posted to it
 * when the sound is finished.  USound::Play() locks <inHdl> while it is playing and unlocks
 * it before the msg_ReleaseBuffer is posted.
 */
void USound::Play(THdl inHdl, TMessageProc inMsgProc, void *inContext, Uint32 /* inOptions */)
{
	Require(inHdl);
	
	// check that there is enough data for at least the header
	Uint32 hdlSize = inHdl->GetSize();
	if (hdlSize < sizeof(SAWSoundHdr)) Fail(errorType_Misc, error_Corrupt);
	
	// get ptr to sound header
	SAWSoundHdr *hdr = (SAWSoundHdr *)UMemory::Lock(inHdl);
	TSoundOutput sout = nil;
	
	try
	{
		// check format and version
		if (hdr->format != TB((Uint32)'AWSN')) Fail(errorType_Misc, error_FormatUnknown);
		if (hdr->version != TB((Uint16)1)) Fail(errorType_Misc, error_VersionUnknown);
		if (hdr->compType != 0) Fail(errorType_Misc, error_FormatUnknown);

		// check that there is as much data as the sound claims
		Uint32 dataSize = FB(hdr->dataSize);
		if (sizeof(SAWSoundHdr) + dataSize > hdlSize) Fail(errorType_Misc, error_Corrupt);
		
		// check that the amount of data supplied for the samples is actually enough
		Uint32 chanCount = FB(hdr->chanCount);
		Uint32 sampleSize = FB(hdr->sampleSize);
		if ((chanCount * sampleSize * FB(hdr->sampleCount)) > dataSize) Fail(errorType_Misc, error_Corrupt);
			
		// create new sound output
		sout = USound::NewOutput(chanCount, FB(hdr->sampleRate), sampleSize, 0, 0, _SNPlayHdlMsgHandler, inHdl);
	
		// store msg proc that we'll be posting our message to
		Uint32 *lp = (Uint32 *)USound::GetOutputScratch(sout);
		lp[0] = (Uint32)inMsgProc;
		lp[1] = (Uint32)inContext;
	
		// play the sound, and we'll next get control in _SNPlayHdlMsgHandler once the sound has finished playing
		USound::EnqueueOutput(sout, hdr->sampleData, dataSize);
	}
	catch(...)
	{
		// clean up
		USound::DisposeOutput(sout);
		UMemory::Unlock(inHdl);
		throw;
	}
}

// if <inRef> is nil, searchs global resource chain.  Does nothing if can't find or play the sound
#if !MACINTOSH
void USound::Play(TRez inRef, Int32 inID, Uint32 /* inOptions */)
{
	if (_gDisableSound || inID == 0) return;

	TRez rz = inRef ? inRef : URez::SearchChain('SOUN', inID);
	if (rz == nil) return;
	
	THdl h = rz->LoadItem('SOUN', inID, true);
	if (h == nil) return;
	
	// check that there is enough data for at least the header
	Uint32 hdlSize = h->GetSize();
	if (hdlSize < sizeof(SAWSoundHdr))
	{
		DebugBreak("USound::Play - corrupt sound handle");
		rz->ReleaseItem('SOUN', inID);
		return;
	}
	
	// get ptr to sound header
	SAWSoundHdr *hdr = (SAWSoundHdr *)UMemory::Lock(h);
	TSoundOutput sout = nil;
	
	try
	{
		// check format and version
		if (hdr->format != TB((Uint32)'AWSN')) Fail(errorType_Misc, error_FormatUnknown);
		if (hdr->version != TB((Uint16)1)) Fail(errorType_Misc, error_VersionUnknown);
		if (hdr->compType != 0) Fail(errorType_Misc, error_FormatUnknown);

		// check that there is as much data as the sound claims
		Uint32 dataSize = FB(hdr->dataSize);
		if (sizeof(SAWSoundHdr) + dataSize > hdlSize) Fail(errorType_Misc, error_Corrupt);
		
		// check that the amount of data supplied for the samples is actually enough
		Uint32 chanCount = FB(hdr->chanCount);
		Uint32 sampleSize = FB(hdr->sampleSize);
		if ((chanCount * sampleSize * FB(hdr->sampleCount)) > dataSize) Fail(errorType_Misc, error_Corrupt);
			
		// create new sound output
		sout = USound::NewOutput(chanCount, FB(hdr->sampleRate), sampleSize, 0, 0, _SNPlayRezMsgHandler, h);
	
		// store msg proc that we'll be posting our message to
		Uint32 *lp = (Uint32 *)USound::GetOutputScratch(sout);
#if DEBUG
		if (!lp)
			DebugBreak("GetOutputScratch Failed");
#endif
		lp[0] = (Uint32)rz;
		lp[1] = (Uint32)inID;
	
		// play the sound, and we'll next get control in _SNPlayHdlMsgHandler once the sound has finished playing
		USound::EnqueueOutput(sout, hdr->sampleData, dataSize);
	}
	catch(...)
	{
		// clean up
		USound::DisposeOutput(sout);
		UMemory::Unlock(h);
		rz->ReleaseItem('SOUN', inID);
		// don't throw
	}
}
#endif

// samples in inSampleData are assumed to be in portable format, not host format!
THdl USound::FlattenToHandle(Uint32 inChanCount, Uint32 inSampleRate, Uint32 inSampleSize, Uint32 inSampleCount, const void *inSampleData, Uint32 inSampleDataSize, Uint32 inCompType, Uint32 /* inOptions */)
{
	enum {
		headerSize	= 32
	};
	
	Require(inChanCount && (inChanCount <= max_Uint8) && inSampleRate && inSampleSize && (inSampleSize <= max_Uint8));
	
	THdl h = UMemory::NewHandle(headerSize + inSampleDataSize);
	
	try
	{
		SAWSoundHdr *hdr;
		StHandleLocker lock(h, (void*&)hdr);
		
		hdr->format = TB((Uint32)'AWSN');
		hdr->version = TB((Uint16)1);
		hdr->chanCount = TB((Uint8)inChanCount);
		hdr->sampleSize = TB((Uint8)inSampleSize);
		hdr->sampleRate = TB((Uint32)inSampleRate);
		hdr->sampleCount = TB((Uint32)inSampleCount);
		hdr->rsvdA = hdr->rsvdB = 0;
		hdr->compType = TB((Uint32)inCompType);
		hdr->dataSize = TB((Uint32)inSampleDataSize);	// normally (inChanCount * inSampleSize * inSampleCount)

		UMemory::Copy(hdr->sampleData, inSampleData, inSampleDataSize);
	}
	catch(...)
	{
		UMemory::Dispose(h);
		throw;
	}
	
	return h;
}

void USound::SetCanPlay(bool inCanPlay)
{
	_gDisableSound = !inCanPlay;
}

bool USound::IsCanPlay()
{
	return !_gDisableSound;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

// outState must point to 16 bytes
void USound::ADPCMInit(void *outState)
{
	((_adpcm_state *)outState)->valprev = ((_adpcm_state *)outState)->index = 0;
}

Uint32 USound::ADPCMGetMaxCompressedSize(Uint32 inSampleCount)
{
	// there are two samples per byte, so round up to multiple of 2, then divide by two to get number of bytes
	return RoundUp2(inSampleCount) >> 1;
}

Uint32 USound::ADPCMGetMaxDecompressedSize(Uint32 inCompressedDataSize)
{
	// there are two compressed samples per byte, and it decompresses to 2-byte samples, so that's 4x the size
	return inCompressedDataSize << 2;
}

/*
 * Intel/DVI ADPCM compressor.  <ioState> must point to 16 bytes originally initialized
 * via ADPCMInit().  <inData> must point to inDataSize/2 Int16 samples in portable format
 * (NOT host format!), and the compressed output is in portable format as well. Returns
 * number of bytes written to <outData> (size of compressed data).
 */
Uint32 USound::ADPCMCompress16(const void *inData, Uint32 inDataSize, void *outData, void *ioState)
{
    Int16 *inp;			/* Input buffer pointer */
    Int8 *outp;			/* output buffer pointer */
    int val;			/* Current input sample value */
    int sign;			/* Current adpcm sign bit */
    int delta;			/* Current adpcm output value */
    int tdiff;			/* Difference between val and valprev */
    int step;			/* Stepsize */
    int valpred;		/* Predicted output value */
    int vpdiff;			/* Current change to valpred */
    int index;			/* Current step change index */
    int outputbuffer;	/* place to keep previous 4-bit value */
    int bufferstep;		/* toggle between outputbuffer/output */

    outp = (Int8 *)outData;
    inp = (Int16 *)inData;

    valpred = ((_adpcm_state *)ioState)->valprev;
    index = ((_adpcm_state *)ioState)->index;
    step = _adpcm_stepsizeTable[index];
    
    bufferstep = 1;
    
    inDataSize >>= 1;	// inDataSize/2 is number of samples (16-bit so two bytes per sample)

    while (inDataSize--)
    {
		val = FB( *inp++ );

		/* Step 1 - compute difference with previous value */
		tdiff = val - valpred;
		sign = (tdiff < 0) ? 8 : 0;
		if ( sign ) tdiff = (-tdiff);

		/* Step 2 - Divide and clamp */
		/* Note:
		** This code *approximately* computes:
		**    delta = tdiff*4/step;
		**    vpdiff = (delta+0.5)*step/4;
		** but in shift step bits are dropped. The net result of this is
		** that even if you have fast mul/div hardware you cannot put it to
		** good use since the fixup would be too expensive.
		*/
		delta = 0;
		vpdiff = (step >> 3);
		
		if ( tdiff >= step )
		{
		    delta = 4;
		    tdiff -= step;
		    vpdiff += step;
		}
		step >>= 1;
		if ( tdiff >= step  )
		{
		    delta |= 2;
		    tdiff -= step;
		    vpdiff += step;
		}
		step >>= 1;
		if ( tdiff >= step )
		{
		    delta |= 1;
		    vpdiff += step;
		}

		/* Step 3 - Update previous value */
		if ( sign )
		  valpred -= vpdiff;
		else
		  valpred += vpdiff;

		/* Step 4 - Clamp previous value to 16 bits */
		if ( valpred > 32767 )
		  valpred = 32767;
		else if ( valpred < -32768 )
		  valpred = -32768;

		/* Step 5 - Assemble value, update index and step values */
		delta |= sign;
		
		index += _adpcm_indexTable[delta];
		if ( index < 0 ) index = 0;
		if ( index > 88 ) index = 88;
		step = _adpcm_stepsizeTable[index];

		/* Step 6 - Output value */
		if (bufferstep)
		    outputbuffer = (delta << 4) & 0xF0;
		else
		    *outp++ = (delta & 0x0F) | outputbuffer;
		
		bufferstep = !bufferstep;
    }

    /* Output last step, if needed */
    if (!bufferstep)
      *outp++ = outputbuffer;
    
    ((_adpcm_state *)ioState)->valprev = valpred;
    ((_adpcm_state *)ioState)->index = index;
    
    return outp - (Int8 *)outData;
}

/*
 * Intel/DVI ADPCM compressor.  <ioState> must point to 16 bytes originally initialized
 * via ADPCMInit().  <inData> must point to inDataSize Int8 samples in portable format
 * (NOT host format!), and the compressed output is in portable format as well. Returns
 * number of bytes written to <outData> (size of compressed data).
 */
Uint32 USound::ADPCMCompress8(const void *inData, Uint32 inDataSize, void *outData, void *ioState)
{
	// this table is for mapping 8-bit numbers to 16-bit signed
	const int scale8to16tab[] = {
		-32768, -32511, -32254, -31997, -31740, -31483, -31226, -30969, -30712, -30455, -30198, -29941, -29684, -29427, 
		-29170, -28913, -28656, -28399, -28142, -27885, -27628, -27371, -27114, -26857, -26600, -26343, -26086, -25829, 
		-25572, -25315, -25058, -24801, -24544, -24287, -24030, -23773, -23516, -23259, -23002, -22745, -22488, -22231, 
		-21974, -21717, -21460, -21203, -20946, -20689, -20432, -20175, -19918, -19661, -19404, -19147, -18890, -18633, 
		-18376, -18119, -17862, -17605, -17348, -17091, -16834, -16577, -16320, -16063, -15806, -15549, -15292, -15035, 
		-14778, -14521, -14264, -14007, -13750, -13493, -13236, -12979, -12722, -12465, -12208, -11951, -11694, -11437, 
		-11180, -10923, -10666, -10409, -10152, -9895, -9638, -9381, -9124, -8867, -8610, -8353, -8096, -7839, -7582, 
		-7325, -7068, -6811, -6554, -6297, -6040, -5783, -5526, -5269, -5012, -4755, -4498, -4241, -3984, -3727, -3470, 
		-3213, -2956, -2699, -2442, -2185, -1928, -1671, -1414, -1157, -900, -643, -386, -129, 128, 385, 642, 899, 1156, 
		1413, 1670, 1927, 2184, 2441, 2698, 2955, 3212, 3469, 3726, 3983, 4240, 4497, 4754, 5011, 5268, 5525, 5782, 6039, 
		6296, 6553, 6810, 7067, 7324, 7581, 7838, 8095, 8352, 8609, 8866, 9123, 9380, 9637, 9894, 10151, 10408, 10665, 
		10922, 11179, 11436, 11693, 11950, 12207, 12464, 12721, 12978, 13235, 13492, 13749, 14006, 14263, 14520, 14777, 
		15034, 15291, 15548, 15805, 16062, 16319, 16576, 16833, 17090, 17347, 17604, 17861, 18118, 18375, 18632, 18889, 
		19146, 19403, 19660, 19917, 20174, 20431, 20688, 20945, 21202, 21459, 21716, 21973, 22230, 22487, 22744, 23001, 
		23258, 23515, 23772, 24029, 24286, 24543, 24800, 25057, 25314, 25571, 25828, 26085, 26342, 26599, 26856, 27113, 
		27370, 27627, 27884, 28141, 28398, 28655, 28912, 29169, 29426, 29683, 29940, 30197, 30454, 30711, 30968, 31225, 
		31482, 31739, 31996, 32253, 32510, 32767
	};
	
    Int8 *inp;			/* Input buffer pointer */
    Int8 *outp;			/* output buffer pointer */
    int val;			/* Current input sample value */
    int sign;			/* Current adpcm sign bit */
    int delta;			/* Current adpcm output value */
    int tdiff;			/* Difference between val and valprev */
    int step;			/* Stepsize */
    int valpred;		/* Predicted output value */
    int vpdiff;			/* Current change to valpred */
    int index;			/* Current step change index */
    int outputbuffer;	/* place to keep previous 4-bit value */
    int bufferstep;		/* toggle between outputbuffer/output */

    outp = (Int8 *)outData;
    inp = (Int8 *)inData;

    valpred = ((_adpcm_state *)ioState)->valprev;
    index = ((_adpcm_state *)ioState)->index;
    step = _adpcm_stepsizeTable[index];
    
    bufferstep = 1;
    
	const int *scaleMidPt = scale8to16tab + 128;	// the input bytes are signed, so we'll put the ptr in the middle of the table so min_Int8 gets the first entry
    
    inDataSize >>= 1;	// inDataSize/2 is number of samples (16-bit so two bytes per sample)

    while (inDataSize--)
    {
		val = scaleMidPt[ FB( *inp++ ) ];

		/* Step 1 - compute difference with previous value */
		tdiff = val - valpred;
		sign = (tdiff < 0) ? 8 : 0;
		if ( sign ) tdiff = (-tdiff);

		/* Step 2 - Divide and clamp */
		/* Note:
		** This code *approximately* computes:
		**    delta = tdiff*4/step;
		**    vpdiff = (delta+0.5)*step/4;
		** but in shift step bits are dropped. The net result of this is
		** that even if you have fast mul/div hardware you cannot put it to
		** good use since the fixup would be too expensive.
		*/
		delta = 0;
		vpdiff = (step >> 3);
		
		if ( tdiff >= step )
		{
		    delta = 4;
		    tdiff -= step;
		    vpdiff += step;
		}
		step >>= 1;
		if ( tdiff >= step  )
		{
		    delta |= 2;
		    tdiff -= step;
		    vpdiff += step;
		}
		step >>= 1;
		if ( tdiff >= step )
		{
		    delta |= 1;
		    vpdiff += step;
		}

		/* Step 3 - Update previous value */
		if ( sign )
		  valpred -= vpdiff;
		else
		  valpred += vpdiff;

		/* Step 4 - Clamp previous value to 16 bits */
		if ( valpred > 32767 )
		  valpred = 32767;
		else if ( valpred < -32768 )
		  valpred = -32768;

		/* Step 5 - Assemble value, update index and step values */
		delta |= sign;
		
		index += _adpcm_indexTable[delta];
		if ( index < 0 ) index = 0;
		if ( index > 88 ) index = 88;
		step = _adpcm_stepsizeTable[index];

		/* Step 6 - Output value */
		if (bufferstep)
		    outputbuffer = (delta << 4) & 0xF0;
		else
		    *outp++ = (delta & 0x0F) | outputbuffer;
		
		bufferstep = !bufferstep;
    }

    /* Output last step, if needed */
    if (!bufferstep)
      *outp++ = outputbuffer;
    
    ((_adpcm_state *)ioState)->valprev = valpred;
    ((_adpcm_state *)ioState)->index = index;

    return outp - (Int8 *)outData;
}

/*
 * Intel/DVI ADPCM decompressor.  <ioState> must point to 16 bytes originally initialized
 * via ADPCMInit().  <inData> must point to output from a previous ADPCMCompress call,
 * and the decompressed output is in portable format - not host format! It's also 16-bit
 * samples even if the data was originally produced from 8-bit samples.  Returns number 
 * of bytes written to <outData> (size of decompressed data).
 */
Uint32 USound::ADPCMDecompress(const void *inData, Uint32 inDataSize, void *outData, void *ioState)
{
    Int8 *inp;			/* Input buffer pointer */
    Int16 *outp;		/* output buffer pointer */
    int sign;			/* Current adpcm sign bit */
    int delta;			/* Current adpcm output value */
    int step;			/* Stepsize */
    int valpred;		/* Predicted value */
    int vpdiff;			/* Current change to valpred */
    int index;			/* Current step change index */
    int inputbuffer;	/* place to keep next 4-bit value */
    int bufferstep;		/* toggle between inputbuffer/input */

    outp = (Int16 *)outData;
    inp = (Int8 *)inData;

    valpred = ((_adpcm_state *)ioState)->valprev;
    index = ((_adpcm_state *)ioState)->index;
    step = _adpcm_stepsizeTable[index];

    bufferstep = 0;
    
    inDataSize <<= 1;	// inDataSize*2 is number of samples since there are two samples per byte in the compressed data
    
    while (inDataSize--)
	{
		/* Step 1 - get the delta value */
		if (bufferstep)
			delta = inputbuffer & 0xF;
		else
		{
			inputbuffer = *inp++;
			delta = (inputbuffer >> 4) & 0xF;
		}
		bufferstep = !bufferstep;

		/* Step 2 - Find new index value (for later) */
		index += _adpcm_indexTable[delta];
		if ( index < 0 ) index = 0;
		if ( index > 88 ) index = 88;

		/* Step 3 - Separate sign and magnitude */
		sign = delta & 8;
		delta = delta & 7;

		/* Step 4 - Compute difference and new predicted value */
		/*
		** Computes 'vpdiff = (delta+0.5)*step/4', but see comment
		** in ADPCMCompress.
		*/
		vpdiff = step >> 3;
		if ( delta & 4 ) vpdiff += step;
		if ( delta & 2 ) vpdiff += step>>1;
		if ( delta & 1 ) vpdiff += step>>2;

		if ( sign )
		  valpred -= vpdiff;
		else
		  valpred += vpdiff;

		/* Step 5 - clamp output value */
		if ( valpred > 32767 )
			valpred = 32767;
		else if ( valpred < -32768 )
			valpred = -32768;

		/* Step 6 - Update step value */
		step = _adpcm_stepsizeTable[index];

		/* Step 7 - Output value */
		*outp++ = TB((Int16)valpred);
    }

    ((_adpcm_state *)ioState)->valprev = valpred;
    ((_adpcm_state *)ioState)->index = index;
    
    return outp - (Int16 *)outData;
}


