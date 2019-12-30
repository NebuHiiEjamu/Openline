#if WIN32

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "UDebug.h"
#include "UText.h"
#include "stdarg.h"

/* -------------------------------------------------------------------------- */

void UDebug::Break()
{
	::DebugBreak();
}

void UDebug::Break(const Int8 inMsg[], ...)
{
	Int8 str[1024];

	va_list va;
	va_start(va, inMsg);
	str[UText::FormatArg(str, sizeof(str)-1, inMsg, va)] = 0;
	va_end(va);
	
	::OutputDebugString(str);
	::DebugBreak();
}

void UDebug::BreakAssembly()
{
	::DebugBreak();
}

void UDebug::BreakAssembly(const Int8 inMsg[], ...)
{
	Int8 str[1024];

	va_list va;
	va_start(va, inMsg);
	str[UText::FormatArg(str, sizeof(str)-1, inMsg, va)] = 0;
	va_end(va);
	
	::OutputDebugString(str);
	::DebugBreak();
}

void UDebug::BreakSource()
{
	::DebugBreak();
}

void UDebug::BreakSource(const Int8 inMsg[], ...)
{
	Int8 str[1024];

	va_list va;
	va_start(va, inMsg);
	str[UText::FormatArg(str, sizeof(str)-1, inMsg, va)] = 0;
	va_end(va);
	
	::OutputDebugString(str);
	::DebugBreak();
}

void UDebug::LogToDebugFile(const Int8 inMsg[], ...)
{
	Uint8 msg[2048];
	Int8 path[2048];
	Uint32 msgSize, pathSize;
	Uint8 *p;
	
	// get the debug message
	va_list va;
	va_start(va, inMsg);
	msgSize = UText::FormatArg(msg, sizeof(msg), inMsg, va);
	msg[msgSize++] = '\r';
	va_end(va);
	
	// make path to debuglog.txt file in same dir as EXE
	pathSize = ::GetModuleFileName(NULL, path, sizeof(path)-16);
	p = UMemory::SearchByteBackwards('\\', path, pathSize);
	if (p == nil) return;
	p++;
	UMemory::Copy(p, "debuglog.txt\0", 13);
	
	// write the message to the debuglog.txt file
	HANDLE h = ::CreateFileA(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h == INVALID_HANDLE_VALUE) return;
	if (::SetFilePointer(h, 0, NULL, FILE_END) != 0xFFFFFFFF)
		::WriteFile(h, msg, msgSize, &msgSize, NULL);
	::CloseHandle(h);
}

#if DEBUG
void DebugBreak(const Int8 inMsg[], ...)
{
	Int8 str[1024];

	va_list va;
	va_start(va, inMsg);
	str[UText::FormatArg(str, sizeof(str)-1, inMsg, va)] = 0;
	va_end(va);

	::MessageBox(NULL, str, "DEBUG", MB_TASKMODAL | MB_ICONWARNING);
}

void DebugLog(const Int8 inMsg[], ...)
{
	Int8 str[1024];

	va_list va;
	va_start(va, inMsg);
	str[UText::FormatArg(str, sizeof(str)-1, inMsg, va)] = 0;
	va_end(va);
	
	::OutputDebugString(str);
}
void DebugLogFile(const Int8 inMsg[], ...)
{
	Uint8 msg[2048];
	Int8 path[2048];
	Uint32 msgSize, pathSize;
	Uint8 *p;
	
	// get the debug message
	va_list va;
	va_start(va, inMsg);
	msgSize = UText::FormatArg(msg, sizeof(msg), inMsg, va);
	msg[msgSize++] = '\r';
	va_end(va);
	
	// make path to debuglog.txt file in same dir as EXE
	pathSize = ::GetModuleFileName(NULL, path, sizeof(path)-16);
	p = UMemory::SearchByteBackwards('\\', path, pathSize);
	if (p == nil) return;
	p++;
	UMemory::Copy(p, "debuglog.txt\0", 13);
	
	// write the message to the debuglog.txt file
	HANDLE h = ::CreateFileA(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h == INVALID_HANDLE_VALUE) return;
	if (::SetFilePointer(h, 0, NULL, FILE_END) != 0xFFFFFFFF)
		::WriteFile(h, msg, msgSize, &msgSize, NULL);
	::CloseHandle(h);
}
#endif






#endif
