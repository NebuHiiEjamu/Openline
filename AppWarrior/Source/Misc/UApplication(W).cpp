#if WIN32

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "UApplication.h"

void _FailLastWinError(const Int8 *inFile, Uint32 inLine);
void _FailWinError(Int32 inWinError, const Int8 *inFile, Uint32 inLine);
#if DEBUG
	#define FailLastWinError()		_FailLastWinError(__FILE__, __LINE__)
	#define FailWinError(id)		_FailWinError(id, __FILE__, __LINE__)
#else
	#define FailLastWinError()		_FailLastWinError(nil, 0)
	#define FailWinError(id)		_FailWinError(id, nil, 0)
#endif

static TMessageSys _APMsgSys = nil;
static bool _APQuitFlag = false;
static bool _APSleepFlag = false;

static ATOM _APDummyClassAtom = 0;
static HWND _APDummyWin = 0;

extern HINSTANCE _gProgramInstance;
extern Int8 *_gCmdLineStr;
extern Uint32 _gCmdLineStrSize;
extern Uint32 _gCmdLineStrOffset;

TFSRefObj* _FSNewRefFromWinPath(const void *inPath, Uint32 inPathSize);
TFSRefObj* _FSGetModuleRef();

static void _APProcess(MSG *inMsg);
	
static LRESULT CALLBACK _APDummyProc(HWND inWnd, UINT inMsg, WPARAM inWParam, LPARAM inLParam);


/* -------------------------------------------------------------------------- */

void UApplication::Init()
{
	if (UService::IsService())
	{
		WNDCLASSEX wndclass;

		if (_APDummyClassAtom == 0)
		{
			// fill in window class info
			wndclass.cbSize = sizeof(WNDCLASSEX);
			wndclass.style = 0;
			wndclass.lpfnWndProc = _APDummyProc;
			wndclass.cbClsExtra = wndclass.cbWndExtra = 0;
			wndclass.hInstance = _gProgramInstance;
			wndclass.hIcon = NULL;
			wndclass.hCursor = NULL;
			wndclass.hbrBackground = NULL;
			wndclass.lpszMenuName = NULL;
			wndclass.lpszClassName = "UApplication-dummy";
			wndclass.hIconSm = NULL;
	
			// register window class
			_APDummyClassAtom = RegisterClassEx(&wndclass);
			if (_APDummyClassAtom == 0) FailLastWinError();
		}
	
		if (_APDummyWin == 0)
		{
			_APDummyWin = CreateWindow((char *)_APDummyClassAtom, "", WS_OVERLAPPEDWINDOW, 0, 0, 32, 32, NULL, NULL, _gProgramInstance, NULL);
			if (_APDummyWin == 0) FailLastWinError();
		}
	}

	if (_APMsgSys == nil)
	{
		_APMsgSys = UMessageSys::New();
		
		// check for files/docs/protocols to open
		if (_gCmdLineStr) UApplication::PostMessage(msg_OpenDocuments);
	}
}



// processes all messages and returns straight away
void UApplication::Process()
{
	MSG msg;
	
	while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		_APProcess(&msg);
	while (_APMsgSys->Execute(nil)) {}
}

// processes all messages and doesn't return until there are new messages
void UApplication::ProcessAndSleep()
{
	MSG msg;
	
	while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		_APProcess(&msg);

	while (_APMsgSys->Execute(nil)) {}

	_APSleepFlag = true;
	
	if (::GetMessageA(&msg, NULL, 0, 0) == TRUE)
		_APProcess(&msg);
	_APSleepFlag = false;
}

// processes all messages of types in the specified list (zero-terminated) and returns straight away
void UApplication::ProcessOnly(const Uint32 inMsgList[])
{
	MSG msg;
	
	while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		_APProcess(&msg);
	
	while (_APMsgSys->Execute(inMsgList)) {}
}

void UApplication::ProcessOnly(Uint32 inMsg)
{
	const Uint32 msgList[2] = { inMsg, 0 };
	ProcessOnly(msgList);
}

#pragma mark -

// continually processes messages and doesn't return until Quit() is called
void UApplication::Run()
{
	while (!_APQuitFlag)
	{
		Process();
		if (_APQuitFlag) break;
		ProcessAndSleep();
	}
}

// causes Run() to return when convenient
void UApplication::Quit()
{
	UProgramCleanup::CleanupAppl();
	_APQuitFlag = true;	
}

// terminates the program immediately (does not return)
void UApplication::Abort()
{
	::ExitProcess(0);
}

void UApplication::Error(const SError& inError)
{
	/*
	 * IMPORTANT: This function may get called while a drag operation is running.
	 * Thus it's important that it be compatible with the drag operation.  For
	 * example, going into a modal loop to display an error box in the middle of
	 * a drag is not very nice.  Thus, Error() posts an error message.  While the
	 * drag is running, only drag messages get processed.  Thus, any errors
	 * that occured during the drag will nicely show their error boxes when the
	 * drag has completed.
	 */
	
	if (inError.fatal)
		throw inError;

	try
	{
		PostMessage(msg_Error, &inError, sizeof(inError), priority_Draw-1);
	}
	catch(...)
	{
		// don't throw because we've probably been called from another "catch" handler
	}
}

bool UApplication::IsQuit()
{
	return _APQuitFlag;
}

#pragma mark -

void UApplication::SetMessageHandler(TMessageProc inProc, void *inContext)
{
	_APMsgSys->SetDefaultProc(inProc, inContext);
}

void UApplication::SendMessage(Uint32 inMsg, const void *inData, Uint32 inDataSize, Int16 /* inPriority */, TMessageProc inProc, void *inContext, void *inObject)
{
	try
	{
		if (inProc == nil)
			_APMsgSys->GetDefaultProc(inProc, inContext);

		inProc(inContext, inObject, inMsg, inData, inDataSize);
	}
	catch(SError& err)
	{
		if (inMsg != msg_Error)	// avoid infinite loop of never-ending error messages
		{
			try { _APMsgSys->Post(msg_Error, &err, sizeof(err), priority_Draw-1); } catch(...) {}
		}
		// don't throw
	}
	catch(...)
	{
		// don't throw
	}
}

extern HWND _TRSockWin;

void UApplication::PostMessage(Uint32 inMsg, const void *inData, Uint32 inDataSize, Int16 inPriority, TMessageProc inProc, void *inContext, void *inObject)
{
	_APMsgSys->Post(inMsg, inData, inDataSize, inPriority, inProc, inContext, inObject);
	
	// ::GetMessageA will return only when is a message in the queue, so we post a dummy message
	if (_APSleepFlag)
		::PostMessageA(_APDummyWin, 0, 0, 0);	
}

void UApplication::ReplaceMessage(Uint32 inMsg, const void *inData, Uint32 inDataSize, Int16 inPriority, TMessageProc inProc, void *inContext, void *inObject)
{
	_APMsgSys->Replace(inMsg, inData, inDataSize, inPriority, inProc, inContext, inObject);
}

void UApplication::FlushMessages(TMessageProc inProc, void *inContext, void *inObject)
{
	_APMsgSys->Flush(inProc, inContext, inObject);
}

bool UApplication::PeekMessage(Uint32 inMsg, void *outData, Uint32& ioDataSize, TMessageProc inProc, void *inContext, void *inObject)
{
	return _APMsgSys->Peek(inMsg, outData, ioDataSize, inProc, inContext, inObject);
}

#pragma mark -

void UApplication::SetCanOpenDocuments(bool /* inEnable */)
{
	// nothing to do (?)
	// ******** get rid of this func ?  don't think need it anymore
}

bool _GetURLAddress(const void *inText, Uint32 inTextSize, Uint32& outStart, Uint32& outSize);

// returns nil if no document
TFSRefObj* UApplication::GetDocumentToOpen()
{
	goFromStart:
	
	if (_gCmdLineStr)	// if got some docs to open from the command line (when app was started)
	{
		ASSERT(_gCmdLineStrOffset < _gCmdLineStrSize);		
		// get ptr and size of data remaining to process in command line string
		Uint8 *line = BPTR(_gCmdLineStr) + _gCmdLineStrOffset;
		Uint32 lineSize = _gCmdLineStrSize - _gCmdLineStrOffset;
		
		// if there is just one file, windoze sometimes uses the long file name which can include spaces, but if there are multiple files, space is used as a separator!! argh!
		if (_gCmdLineStrOffset == 0 && (UMemory::SearchByte(':', line, lineSize) == UMemory::SearchByteBackwards(':', line, lineSize)))
		{
			// remove any quote at the start
			if (lineSize && *line == '\"')
			{
				line++;
				lineSize--;
			}
			
			// remove any quote at the end
			if (lineSize && line[lineSize-1] == '\"')
				lineSize--;

			// check if this is a protocol.  if not, it's a file
			TFSRefObj* fp = nil;
			Uint32 start, size;
			if (_GetURLAddress(line, lineSize, start, size))
			{
				// this is a url - post a msg with it
				UApplication::PostMessage(msg_OpenURL, line, lineSize);
			}
			else
			{
				// there is only one colon, so only one file
				fp = _FSNewRefFromWinPath(line, lineSize);
			}
			
			// there is only one colon, so only one file
			::GlobalFree((HGLOBAL)_gCmdLineStr);
			_gCmdLineStr = NULL;
			_gCmdLineStrSize = _gCmdLineStrOffset = 0;
			return fp;
		}

		// determine size of first path in string (command line string is paths separated by spaces)
		Uint8 *p = UMemory::SearchByte(' ', line, lineSize);
		Uint32 s = p ? p - line : lineSize;

		// check if this is a protocol.  if not, it's a file
		// if it's a protocol, post a msg with the info, increment by s + 1 and goFromStart:
		TFSRefObj* fp = nil;
		Uint32 start, size;
		if (_GetURLAddress(line, s, start, size))
		{
			// this is a url - post a msg with it
			UApplication::PostMessage(msg_OpenURL, line, s);
		}
		else
		{
			// create FS ref from the path
			fp = _FSNewRefFromWinPath(line, s);
		}
		// offset tells us where we're up to in the command line string
		_gCmdLineStrOffset += s + 1;
		
		// check if no more paths in command line string
		if (_gCmdLineStrOffset >= _gCmdLineStrSize)
		{
			// no more paths, free memory command line string was using
			::GlobalFree((HGLOBAL)_gCmdLineStr);
			_gCmdLineStr = NULL;
			_gCmdLineStrSize = _gCmdLineStrOffset = 0;
		}
		
		// all done - if we had a file, return it, else we had a url and we should search for more files
		if (fp)
			return fp;
		else
			goto goFromStart;
	}

	// no documents to open at this time
	return nil;
}

extern void _ShowWantsAttention();

void UApplication::ShowWantsAttention()
{
	_ShowWantsAttention();
}

TFSRefObj* UApplication::GetAppRef()
{
	return _FSGetModuleRef();
}


#pragma mark -

extern bool _ReadTempMessageFromRegistry(Int8 *outMessage, DWORD inMaxSize);

bool _DispatchThreadMessage(Uint32 inMsg)
{
#if 0
	switch (inMsg)
	{
		case msg_ExternalCmdLine:
			Int8 bufCmdLine[256];
			if (_ReadTempMessageFromRegistry(bufCmdLine, sizeof(bufCmdLine)))
			{	
				if (_gCmdLineStr)
					::GlobalFree((HGLOBAL)_gCmdLineStr);

				Uint32 nCmdLineSize = strlen(bufCmdLine);
				_gCmdLineStr = (Int8 *)::GlobalAlloc(GMEM_FIXED, nCmdLineSize + 1);
	
				if (_gCmdLineStr)
				{
					::CopyMemory(_gCmdLineStr, bufCmdLine, nCmdLineSize + 1);
					_gCmdLineStrSize = nCmdLineSize;
			
					UApplication::PostMessage(msg_OpenDocuments);
				}
			}
			
			return true;
	};

#else
#pragma unused(inMsg)
#endif	
	return false;
}

static void _APProcess(MSG *inMsg)
{
	if (inMsg->hwnd || !_DispatchThreadMessage(inMsg->message))
		::DispatchMessage(inMsg);
}


static LRESULT CALLBACK _APDummyProc(HWND inWnd, UINT inMsg, WPARAM inWParam, LPARAM inLParam)
{
	return DefWindowProc(inWnd, inMsg, inWParam, inLParam);
}

#endif /* WIN32 */
