/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "MoreTypes.h"
#include "UError.h"
#include "UMessageSys.h"

class UApplication
{
	public:
		static void Init();

		static void Process();
		static void ProcessAndSleep();
		static void ProcessOnly(const Uint32 inMsgList[]);
		static void ProcessOnly(Uint32 inMsg);
		
		static void Run();
		static void Quit();
		static void Abort();
		static void Error(const SError& inError);
		static bool IsQuit();

		static void SetMessageHandler(TMessageProc inProc, void *inContext = nil);
		static void SendMessage(Uint32 inMsg, const void *inData = nil, Uint32 inDataSize = 0, Int16 inPriority = priority_Normal, TMessageProc inProc = nil, void *inContext = nil, void *inObject = nil);
		static void PostMessage(Uint32 inMsg, const void *inData = nil, Uint32 inDataSize = 0, Int16 inPriority = priority_Normal, TMessageProc inProc = nil, void *inContext = nil, void *inObject = nil);
		static void ReplaceMessage(Uint32 inMsg, const void *inData = nil, Uint32 inDataSize = 0, Int16 inPriority = priority_Normal, TMessageProc inProc = nil, void *inContext = nil, void *inObject = nil);
		static void FlushMessages(TMessageProc inProc, void *inContext, void *inObject = nil);
		static bool PeekMessage(Uint32 inMsg, void *outData, Uint32& ioDataSize, TMessageProc inProc, void *inContext, void *inObject = nil);
		
		static void SetCanOpenDocuments(bool inEnable);
		static TFSRefObj* GetDocumentToOpen();
		
		static void ShowWantsAttention();
		
		static TFSRefObj* GetAppRef();
		
};




