/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"

typedef class TSoundOutputObj *TSoundOutput;

class USound
{
	public:
		static void Init();
		static void Beep();
		static void SetCanPlay(bool inCanPlay = true);
		static bool IsCanPlay();
		
		// play sounds
		static void Play(THdl inHdl, TMessageProc inMsgProc, void *inContext, Uint32 inOptions = 0);
		static void Play(TRez inRef, Int32 inID, Uint32 inOptions = 0);

		// portable sound format
		static THdl FlattenToHandle(Uint32 inChanCount, Uint32 inSampleRate, Uint32 inSampleSize, Uint32 inSampleCount, const void *inSampleData, Uint32 inSampleDataSize, Uint32 inCompType = 0, Uint32 inOptions = 0);

		// low-level sound output
		static TSoundOutput NewOutput(Uint32 inChanCount, Uint32 inSampleRate, Uint32 inSampleSize, Uint32 inCompType, Uint32 inSpecial, TMessageProc inProc, void *inContext, Uint32 inOptions = 0);
		static void FlushOutput(TSoundOutput inRef);
		static void DisposeOutput(TSoundOutput inRef);
		static void SetOutputVolume(TSoundOutput inRef, const Uint32 *inChans, Uint32 inChanCount, Uint32 inOptions = 0);
		static Uint32 GetOutputVolume(TSoundOutput inRef, Uint32 *outChans, Uint32 inMaxCount, Uint32 inOptions = 0);
		static void EnqueueOutput(TSoundOutput inRef, const void *inData, Uint32 inDataSize);
		static Uint8 *GetOutputScratch(TSoundOutput inRef);

		// ADPCM sound compression
		static void ADPCMInit(void *outState);
		static Uint32 ADPCMGetMaxCompressedSize(Uint32 inSampleCount);
		static Uint32 ADPCMGetMaxDecompressedSize(Uint32 inCompressedDataSize);
		static Uint32 ADPCMCompress16(const void *inData, Uint32 inDataSize, void *outData, void *ioState);
		static Uint32 ADPCMCompress8(const void *inData, Uint32 inDataSize, void *outData, void *ioState);
		static Uint32 ADPCMDecompress(const void *inData, Uint32 inDataSize, void *outData, void *ioState);
};

class TSoundOutputObj
{
	public:
		void SetVolume(const Uint32 *inChans, Uint32 inChanCount, Uint32 inOptions = 0)	{	USound::SetOutputVolume(this, inChans, inChanCount, inOptions);			}
		Uint32 GetVolume(Uint32 *outChans, Uint32 inMaxCount, Uint32 inOptions = 0)		{	return USound::GetOutputVolume(this, outChans, inMaxCount, inOptions);	}
		void Enqueue(const void *inData, Uint32 inDataSize)								{	USound::EnqueueOutput(this, inData, inDataSize);						}
		Uint8 *GetScratch()																{	return USound::GetOutputScratch(this);									}
		void Flush()																	{	USound::FlushOutput(this);												}
	
		void operator delete(void *p)													{	USound::DisposeOutput((TSoundOutput)p);									}
	protected:
		TSoundOutputObj() {}				// force creation via USound
};


