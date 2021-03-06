/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
#include "Hotline.h"


CChatLog::CChatLog()
: mRef(0), mEnabled(false), mPos(0)
{
	

}

CChatLog::~CChatLog()
{
	Enable(false);
}

void CChatLog::Enable(bool on)
{
	if (mEnabled == on)
		return;
	mEnabled = on;
	if (mEnabled)
		OpenLog();
	else
		CloseLog();
}

void CChatLog::OpenLog()
{
	if (mRef != 0)
		return;

	const Uint8 kFolderName[] = "\pData";
	TFSRefObj* folder = UFS::New(kProgramFolder, nil, kFolderName);
	scopekill(TFSRefObj, folder);
	
	if (!folder->Exists())
		folder->CreateFolder();
	
	const Uint8 kFileName[] = "\pChatLog.xml";
	mRef = UFS::New(folder, nil, kFileName);
	if (!mRef->Exists()) 
		mRef->CreateFile('TEXT', 'ttxt');

	mRef->Open(perm_ReadWrite);
	mPos = mRef->GetSize();

	#if MACINTOSH
	const Uint8 header[] = "\p<?xml version=\"1.0\"?>\r<ChatLog>\r</ChatLog>";
	#elif WIN32
	const Uint8 header[] = "\p<?xml version=\"1.0\"?>\r\n<ChatLog>\r\n</ChatLog>";
	#endif

	if (mPos <= header[0])
		mPos = mRef->Write(mPos, header+1, header[0]);
   // we need to overwrite </ChatLog> at the end of the file
	mPos -= 10;
}

void CChatLog::CloseLog()
{
	if (mRef == 0)
		return;

	const Uint8 header[] = "\p</ChatLog>";
	mPos += mRef->Write(mPos, header+1, header[0]);
    mRef->SetSize(mPos);
    mRef->Close();
	UFileSys::Dispose(mRef);
	mRef = 0;
}

void CChatLog::AppendLog(const UInt8* type, const UInt8* user, const UInt8 *msg, UInt32 msgZ)
{
	if (mRef == 0)
		return;
	if (mEnabled == false)
		return;
	if (msg == 0 || msgZ == 0)
		return;
	// skip front whitespace
	while(msgZ>0 &&(*msg==' '||*msg=='\r'||*msg=='\n')) 
		++msg,--msgZ;
	// if we don't have user name try to grab one from the message
	UInt8 usrNameBuf[64];
	if (user == 0 || user[0] == 0)
	{
	    UInt8 *p = (UInt8*)msg, *pe = p+min(msgZ,64UL), *u = usrNameBuf+1;
	    for(; p<pe && *p != ':'; ++p) { *u++ = *p; }
	    if (msgZ > 0 && *p == ':' && p > msg)
	    {
	    	usrNameBuf[0] = u-1 - usrNameBuf;
	    	user = usrNameBuf;
	    	msgZ -= p+1 - msg;
	    	msg = p+1;
			// skip front whitespace
			while(msgZ>0 &&(*msg==' '||*msg=='\r'||*msg=='\n')) 
				++msg,--msgZ;
	    }
	}	
	const Uint8 LT[] = "&lt;";
	const Uint8 GT[] = "&gt;";
	const Uint8 AMP[] = "&amp;";
	#if MACINTOSH
	const Uint8 BR[] = "<BR/>\r";
	#elif WIN32
	const Uint8 BR[] = "<BR/>\r\n";
	#endif

    UInt8 msgbuf[1024]; // we hope for no overflow , but we don't crash if we get one
    for(UInt8 *b=(UInt8*)msg, *e=b+msgZ, *d=msgbuf, *f=d+sizeof(msgbuf)-64; b<e && d<f; ++b)
    {
    	switch(*b)
    	{
    	case '<': d += UMemory::Copy(d, LT, sizeof(LT)-1); break;
    	case '>': d += UMemory::Copy(d, GT, sizeof(GT)-1); break;
    	case '&': d += UMemory::Copy(d, AMP, sizeof(AMP)-1); break;
    	case '\r':  d += UMemory::Copy(d, BR, sizeof(BR)-1); break;
    	case '\n':  break; // ignore
    	default:  *d++ = *b; break;
    	}
		*d = 0;
    }

	Uint8 timeStamp[64];
	timeStamp[0] = UDateTime::DateToText(timeStamp+1, sizeof(timeStamp)-1, 
										 kShortDateText + kTimeWithSecsText); //kShortDateFullYearText
	#if MACINTOSH
	const char fmt[] = "<%#s at='%#s' from='%#s'>\r%s\r</%#s>\r\r</ChatLog>";
	#elif WIN32
	const char fmt[] = "<%#s at='%#s' from='%#s'>\r\n%s\r\n</%#s>\r\n\r\n</ChatLog>";
	#endif

    UInt8 text[1024]; // we hope for no overflow , but we don't crash if we get one
	UInt32 textZ = UText::Format(text, sizeof(text), fmt, 
						type, timeStamp, user, msgbuf, type );
	mPos+= mRef->Write(mPos, text, textZ);
    mPos-= 10; // we overwrite </ChatLog> every time we append to the file
    mRef->Flush();
}

