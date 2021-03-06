#if WIN32

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "USound.h"

struct SSoundOutput {
	SSoundOutput *next;
	HWAVEOUT wom;
	TMessageProc msgProc;
	void *msgProcContext;
	Uint8 scratch[32];
};

//#define SOUT	((SSoundOutput *)inRef)

extern HINSTANCE _gProgramInstance;
static ATOM _SNWaveOutAtom = 0;
static HWND _SNWaveOutWin = 0;
static SSoundOutput *_SNFirstSoundOutput = nil;

void _FailLastWinError(const Int8 *inFile, Uint32 inLine);
void _FailWinError(Int32 inWinError, const Int8 *inFile, Uint32 inLine);
#if DEBUG
	#define FailLastWinError()		_FailLastWinError(__FILE__, __LINE__)
	#define FailWinError(id)		_FailWinError(id, __FILE__, __LINE__)
#else
	#define FailLastWinError()		_FailLastWinError(nil, 0)
	#define FailWinError(id)		_FailWinError(id, nil, 0)
#endif

static LRESULT CALLBACK _SNWaveOutProc(HWND inWnd, UINT inMsg, WPARAM inWParam, LPARAM inLParam);

/* -------------------------------------------------------------------------- */

void USound::Init()
{

}

void USound::Beep()
{
	::MessageBeep(MB_OK);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

TSoundOutput USound::NewOutput(Uint32 inChanCount, Uint32 inSampleRate, Uint32 inSampleSize, Uint32 /* inCompType */, Uint32 /* inSpecial */, TMessageProc inProc, void *inContext, Uint32 /* inOptions */)
{
	HWAVEOUT wom = NULL;
	MMRESULT mmres;
	WAVEFORMATEX wfx;
	SSoundOutput *sout;

	Require(inChanCount && inSampleRate && inSampleSize);
	
	if (_SNWaveOutAtom == 0)
	{
		// fill in window class info
		WNDCLASSEX wndclass;
		wndclass.cbSize = sizeof(WNDCLASSEX);
		wndclass.style = 0;
		wndclass.lpfnWndProc = _SNWaveOutProc;
		wndclass.cbClsExtra = wndclass.cbWndExtra = 0;
		wndclass.hInstance = _gProgramInstance;
		wndclass.hIcon = NULL;
		wndclass.hCursor = NULL;
		wndclass.hbrBackground = NULL;
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = "USound-waveout";
		wndclass.hIconSm = NULL;
	
		// register window class
		_SNWaveOutAtom = RegisterClassEx(&wndclass);
		if (_SNWaveOutAtom == 0) FailLastWinError();
	}
	
	if (_SNWaveOutWin == 0)
	{
		_SNWaveOutWin = CreateWindow((char *)_SNWaveOutAtom, "", WS_OVERLAPPEDWINDOW, 0, 0, 32, 32, NULL, NULL, _gProgramInstance, NULL);
		if (_SNWaveOutWin == 0) FailLastWinError();
	}

	wfx.wFormatTag = 1;
    wfx.nChannels = inChanCount;
    wfx.nSamplesPerSec = inSampleRate;
    wfx.nAvgBytesPerSec = inChanCount * inSampleSize * inSampleRate;
    wfx.wBitsPerSample = inSampleSize * 8;
    wfx.nBlockAlign = inChanCount * inSampleSize;
    wfx.cbSize = 0;

	sout = (SSoundOutput *)UMemory::NewClear(sizeof(SSoundOutput));

	mmres = waveOutOpen(&wom, WAVE_MAPPER, &wfx, (DWORD)_SNWaveOutWin, 0, CALLBACK_WINDOW);
	if (mmres)
	{
		UMemory::Dispose((TPtr)sout);
		FailWinError(mmres);
	}
	
	sout->wom = wom;
	sout->msgProc = inProc;
	sout->msgProcContext = inContext;
	
	// add to TSoundOutput list
	sout->next = _SNFirstSoundOutput;
	_SNFirstSoundOutput = sout;

	return (TSoundOutput)sout;
}

// causes sound playback to be immediately stopped on the specified output, and all msg_ReleaseBuffer messages will be sent before this function returns
void USound::FlushOutput(TSoundOutput inRef)
{
	if (inRef)
	{
		waveOutReset(((SSoundOutput *)inRef)->wom);
		
		// make sure all the MM_WOM_DONE messages get processed before we return
		MSG msg;
		while (::PeekMessage(&msg, NULL, MM_WOM_DONE, MM_WOM_DONE, PM_REMOVE))
			::DispatchMessage(&msg);
	}
}

// WARNING! If there are still buffers being played, playback will be stopped immediately, but you will not receive any msg_ReleaseBuffer messages!  Use FlushOutput() if needed.
void USound::DisposeOutput(TSoundOutput inRef)
{
	if (inRef)
	{
		// remove from TSoundOutput list
		SSoundOutput *tm = _SNFirstSoundOutput;
		SSoundOutput *ptm = nil;
		while (tm)
		{
			if (tm == ((SSoundOutput *)inRef))
			{
				if (ptm)
					ptm->next = tm->next;
				else
					_SNFirstSoundOutput = tm->next;
				break;
			}
			ptm = tm;
			tm = tm->next;
		}

		// kill the waveout
		waveOutReset(((SSoundOutput *)inRef)->wom);	// note this will cause MM_WOM_DONE msgs to be posted, but because we removed inRef from the linked list, there won't be a problem
		waveOutClose(((SSoundOutput *)inRef)->wom);

		if (((SSoundOutput *)inRef)->msgProc)
			UApplication::FlushMessages(((SSoundOutput *)inRef)->msgProc, ((SSoundOutput *)inRef)->msgProcContext, inRef);
		
		UMemory::Dispose((TPtr)inRef);
	}
}

// for volumes, a value of 0xFFFF represents full volume, and a value of 0x0000 is silence
void USound::SetOutputVolume(TSoundOutput inRef, const Uint32 *inChans, Uint32 inChanCount, Uint32 /* inOptions */)
{
	Require(inRef);
	
	if (inChanCount)
	{
		DWORD vol = (Uint16)inChans[0];
		if (inChanCount > 1) vol |= inChans[1] << 16;
		
		MMRESULT mmres = waveOutSetVolume(((SSoundOutput *)inRef)->wom, vol);
		if (mmres) FailWinError(mmres);
	}
}

Uint32 USound::GetOutputVolume(TSoundOutput inRef, Uint32 *outChans, Uint32 inMaxCount, Uint32 /* inOptions */)
{
	Require(inRef);
	
	if (inMaxCount)
	{
		DWORD vol;
		
		MMRESULT mmres = waveOutGetVolume(((SSoundOutput *)inRef)->wom, &vol);
		if (mmres) FailWinError(mmres);
		
		outChans[0] = (Uint16)vol;
		
		if (inMaxCount > 1)
		{
			outChans[1] = vol >> 16;
			return 2;
		}
		
		return 1;
	}
	
	return 0;
}

/*
 * EnqueueOutput() adds a buffer to the output queue for the specified output.  The buffer must remain
 * valid until the sound output driver has finished playing it, at which point it will post a 
 * msg_ReleaseBuffer to the msg proc you specified when you called NewOutput().  The <inContext>
 * parameter for the message will be that which you specified for NewOutput().  The <inObject>
 * parameter will be <inRef>, and the data will consist of the <inData> ptr directly followed by 
 * <inDataSize>.  To avoid breaks in sound output, it is recommended that you enqueue two buffers
 * to begin with, and then with every msg_ReleaseBuffer you get, enqueue another buffer.  Buffers
 * should be at least (sampleRate * sampleSize * channelCount * 2) in size.  Also, you cannot split
 * samples across buffers - <inDataSize> must be a multiple of (sampleSize * channelCount).
 */
void USound::EnqueueOutput(TSoundOutput inRef, const void *inData, Uint32 inDataSize)
{
	Require(inRef && inData && inDataSize);

	WAVEHDR *whp = (WAVEHDR *)UMemory::NewClear(sizeof(WAVEHDR));
	
	whp->lpData = (char *)inData;
	whp->dwBufferLength = inDataSize;
	
	MMRESULT mmres = waveOutPrepareHeader(((SSoundOutput *)inRef)->wom, whp, sizeof(WAVEHDR));
	if (mmres)
	{
		UMemory::Dispose((TPtr)whp);
		FailWinError(mmres);
	}

	mmres = waveOutWrite(((SSoundOutput *)inRef)->wom, whp, sizeof(WAVEHDR));
	if (mmres)
	{
		waveOutUnprepareHeader(((SSoundOutput *)inRef)->wom, whp, sizeof(WAVEHDR));
		UMemory::Dispose((TPtr)whp);
		FailWinError(mmres);
	}
}

// returns ptr to a 32-byte area in <inRef> that you can use for your own purposes
Uint8 *USound::GetOutputScratch(TSoundOutput inRef)
{
	Require(inRef);
	return ((SSoundOutput *)inRef)->scratch;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

static SSoundOutput *_SNLookupWaveOut(HWAVEOUT inWOM)
{
	SSoundOutput *tm = _SNFirstSoundOutput;
	
	while (tm)
	{
		if (tm->wom == inWOM)
			return tm;

		tm = tm->next;
	}
	
	return nil;
}

static LRESULT CALLBACK _SNWaveOutProc(HWND inWnd, UINT inMsg, WPARAM inWParam, LPARAM inLParam)
{
	if (inMsg != MM_WOM_DONE)
		return DefWindowProc(inWnd, inMsg, inWParam, inLParam);
	
	try
	{
		Uint32 data[2];
		data[0] = (Uint32)((WAVEHDR *)inLParam)->lpData;
		data[1] = (Uint32)((WAVEHDR *)inLParam)->dwBufferLength;

		waveOutUnprepareHeader((HWAVEOUT)inWParam, (WAVEHDR *)inLParam, sizeof(WAVEHDR));

		UMemory::Dispose((TPtr)inLParam);
		
		// note that can still receive messages after USound::DisposeOutput (if in queue before disposed).  This is okay because _SNLookupWaveOut() won't find the TSoundOutput if it has been killed
		SSoundOutput *sout = _SNLookupWaveOut((HWAVEOUT)inWParam);
		if (sout && sout->msgProc)
			UApplication::SendMessage(msg_ReleaseBuffer, data, sizeof(data), priority_Normal, sout->msgProc, sout->msgProcContext, sout);
	}
	catch(...)
	{
		// DON'T throw exceptions!
	}
	
	return 0;
}







#endif
