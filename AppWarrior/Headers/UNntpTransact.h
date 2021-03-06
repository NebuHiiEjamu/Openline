/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "UTransport.h"


/*
 * Constants
 */

enum {
	port_number_NNTP			= 119
};

// messages
enum {
	msg_NntpError				= 300,
	
	msg_NntpListGroups			= 301,
	msg_NntpSelectGroup			= 302,
	msg_NntpListArticles		= 303,
	msg_NntpSelectArticle		= 304,
	msg_NntpGetArticle			= 305,
	msg_NntpGetArticleHeader	= 306,
	msg_NntpGetArticleData		= 307,
	msg_NntpExistsArticle		= 308,
	msg_NntpPostArticle			= 309
};

// commands ID's
enum {
	kNntp_StartConnect			= 1,
	kNntp_UserLogin				= 2,
	kNntp_UserPassword			= 3,
	kNntp_Connected				= 4,

	kNntp_ListGroups			= 5,
	kNntp_SelectGroup			= 6,
	kNntp_ListArticles			= 7,
	kNntp_SelectArticle			= 8,
	kNntp_GetArticle			= 9,
	kNntp_GetArticleHeader		= 10,
	kNntp_GetArticleData		= 11,
	kNntp_ExistsArticle			= 12,
	kNntp_PostArticle			= 13
};

/*
 * Types
 */

typedef class TNntpTransactObj *TNntpTransact;

/*
 * Structures
 */

struct SNewsGroupInfo {
	Uint8 psGroupName[64];
	Uint32 nArticleCount;
	Uint32 nFirstArticleNumber;
	Uint32 nLastArticleNumber;
	bool bIsPostingAllowed;
};

struct SNewsArticleInfo {
	Uint32 nArticleNumber;
	Uint8 psArticleID[64];
	Uint8 psParentID[64];
	Uint8 psArticleName[64];
	SCalendarDate stArticleDate;
	Uint8 psPosterName[64];
	Uint8 psPosterAddr[64];
};

struct SArticleData {
	Int8 csFlavor[32];		// "text/plain"	for article text
	Uint8 psName[64];		// "\p.body" for article text
	Uint32 nDataSize;
	void *pData;
};

/*
 * UNntpTransact
 */
 
class UNntpTransact
{
	public:
		// new, dispose
		static TNntpTransact New();
		static void Dispose(TNntpTransact inTrn);
		
		// messaging
		static void SetMessageHandler(TNntpTransact inTrn, TMessageProc inProc, void *inContext = nil);
		static void PostMessage(TNntpTransact inTrn, Uint32 inMsg, const void *inData = nil, Uint32 inDataSize = 0, Int16 inPriority = priority_Normal);
		static void ReplaceMessage(TNntpTransact inTrn, Uint32 inMsg, const void *inData = nil, Uint32 inDataSize = 0, Int16 inPriority = priority_Normal);
		
		// transport
		static void MakeNewTransport(TNntpTransact inTrn);
		static TTransport GetTransport(TNntpTransact inTrn);

		// connecting and disconnecting
		static bool IsConnected(TNntpTransact inTrn);
		static bool StartConnect(TNntpTransact inTrn, const Uint8 *inServerAddr, const Uint8 *inLogin = nil, const Uint8 *inPassword = nil, Uint32 inMaxSecs = 0);
		static void Disconnect(TNntpTransact inTrn);
		static void StartDisconnect(TNntpTransact inTrn);
		static bool IsDisconnecting(TNntpTransact inTrn);
		
		// news server info
		static bool IsPostingAllowed(TNntpTransact inTrn);
		static Uint32 GetConnectedServer(TNntpTransact inTrn, Uint8 *outServerAddr, Uint32 inMaxSize);

		// command info	
		static Uint8 GetCommandID(TNntpTransact inTrn);			// kNntp_...
		static bool IsCommandExecuted(TNntpTransact inTrn);
		static bool IsCommandError(TNntpTransact inTrn);
		static Uint8 *GetCommandError(TNntpTransact inTrn);

		// list newsgroups (command ID: kNntp_ListGroups, generate msg_NntpListGroups message)
		static bool Command_ListGroups(TNntpTransact inTrn);
		static bool Command_ListGroups(TNntpTransact inTrn, const SCalendarDate& inSinceDate);

		static Uint32 GetGroupCount(TNntpTransact inTrn);
		static bool GetGroup(TNntpTransact inTrn, SNewsGroupInfo& outGroupInfo, Uint32 inIndex);
		static bool GetNextGroup(TNntpTransact inTrn, SNewsGroupInfo& outGroupInfo, Uint32& ioIndex);
		static bool GetPrevGroup(TNntpTransact inTrn, SNewsGroupInfo& outGroupInfo, Uint32& ioIndex);

		// select newsgroup (command ID: kNntp_SelectGroups, generate msg_NntpSelectGroup message)
		static bool Command_SelectGroup(TNntpTransact inTrn, const Uint8 *inGroupName, bool inIsPostingAllowed = true);

		static bool IsSelectedGroup(TNntpTransact inTrn);
		static bool IsGroupPostingAllowed(TNntpTransact inTrn);
		static bool GetSelectedGroup(TNntpTransact inTrn, SNewsGroupInfo& outGroupInfo);

		// list articles (command ID: kNntp_ListArticles, generate msg_NntpListArticles message)
		static bool Command_ListArticles(TNntpTransact inTrn);
		static bool Command_ListArticles(TNntpTransact inTrn, const SCalendarDate& inSinceDate); // this will list just ID's for new messages
		
		static Uint32 GetArticleCount(TNntpTransact inTrn);
		static bool GetArticle(TNntpTransact inTrn, SNewsArticleInfo& outArticleInfo, Uint32 inIndex);
		static bool GetNextArticle(TNntpTransact inTrn, SNewsArticleInfo& outArticleInfo, Uint32& ioIndex);
		static bool GetPrevArticle(TNntpTransact inTrn, SNewsArticleInfo& outArticleInfo, Uint32& ioIndex);

		// select article (command ID: kNntp_SelectArticles, generate msg_NntpSelectArticle message)
		static bool Command_SelectArticle(TNntpTransact inTrn, Uint32 inArticleID);
		static bool Command_SelectNextArticle(TNntpTransact inTrn);
		static bool Command_SelectPrevArticle(TNntpTransact inTrn);

		static bool IsSelectedArticle(TNntpTransact inTrn);
		static Uint32 GetSelectedArticleNumber(TNntpTransact inTrn);
		static Uint32 GetSelectedArticleID(TNntpTransact inTrn, Uint8 *outArticleID, Uint32 inMaxSize);
		
		// get article header and data (command ID: kNntp_GetArticle, generate msg_NntpGetArticle message)
		static bool Command_GetSelectedArticle(TNntpTransact inTrn);
		static bool Command_GetArticle(TNntpTransact inTrn, Uint32 inArticleNumber);
		static bool Command_GetArticle(TNntpTransact inTrn, const Uint8 *inArticleID);

		// get article header (command ID: kNntp_GetArticleHeader, generate msg_NntpGetArticleHeader message)
		static bool Command_GetSelectedArticleHeader(TNntpTransact inTrn);
		static bool Command_GetArticleHeader(TNntpTransact inTrn, Uint32 inArticleNumber);
		static bool Command_GetArticleHeader(TNntpTransact inTrn, const Uint8 *inArticleID);

		static bool IsCurrentArticle(TNntpTransact inTrn);
		static bool GetArticleInfo(TNntpTransact inTrn, SNewsArticleInfo& outArticleInfo);
	
		// get article data (command ID: kNntp_GetArticleData, generate msg_NntpGetArticleData message)
		static bool Command_GetSelectedArticleData(TNntpTransact inTrn);
		static bool Command_GetArticleData(TNntpTransact inTrn, Uint32 inArticleNumber);
		static bool Command_GetArticleData(TNntpTransact inTrn, const Uint8 *inArticleID);

		static bool IsArticleText(TNntpTransact inTrn);
		static Uint32 GetDataSize(TNntpTransact inTrn);
		static Uint32 GetDataCount(TNntpTransact inTrn);
		static SArticleData *GetData(TNntpTransact inTrn, Uint32 inIndex);
		static bool GetNextData(TNntpTransact inTrn, SArticleData*& outArticleData, Uint32& ioIndex);
		static bool GetPrevData(TNntpTransact inTrn, SArticleData*& outArticleData, Uint32& ioIndex);
		
		// test article existence (command ID: kNntp_ExistsArticle, generate msg_NntpExistsArticle message)
		static bool Command_ExistsArticle(TNntpTransact inTrn, const Uint8 *inArticleID);

		// post article (command ID: kNntp_PostArticle, generate msg_NntpPostArticle message)
		static bool Command_PostArticleStart(TNntpTransact inTrn);
		
		// must to supply ID's for all parents in follow-up order
		static bool AddParentID(TNntpTransact inTrn, const Uint8 *inParentID);
		static bool SetArticleID(TNntpTransact inTrn, const Uint8 *inArticleID);
		static bool SetOrganization(TNntpTransact inTrn, const Uint8 *inOrganization);
		static bool SetNewsReader(TNntpTransact inTrn, const Uint8 *inNewsReader);
		static bool SetPostDate(TNntpTransact inTrn, SCalendarDate& inPostDate);
		static bool SetPostInfo(TNntpTransact inTrn, const Uint8 *inArticleName, const Uint8 *inPosterName, const Uint8 *inPosterAddr = nil);
		static bool AddPostData(TNntpTransact inTrn, const Int8 *inFlavor, const Uint8 *inName, void *inData, Uint32 inDataSize);
		
		static bool Command_PostArticleFinish(TNntpTransact inTrn);
};


/*
 * Stack TNntpTransact
 */

class StNntpTransact
{
	public:
		StNntpTransact()								{	mRef = UNntpTransact::New();					}
		~StNntpTransact()								{	UNntpTransact::Dispose(mRef);					}
		operator TNntpTransact()						{	return mRef;									}
		TNntpTransact operator->() const				{	return mRef;									}
		bool IsValid()									{	return mRef != nil;								}
		bool IsInvalid()								{	return mRef == nil;								}

	private:
		TNntpTransact mRef;
};


/*
 * UNntpTransact Object Interface
 */

class TNntpTransactObj
{
	public:
		// messaging
		void SetMessageHandler(TMessageProc inProc, void *inContext = nil)									{	UNntpTransact::SetMessageHandler(this, inProc, inContext);													}
		void PostMessage(Uint32 inMsg, const void *inData = nil, Uint32 inDataSize = 0, Int16 inPriority = priority_Normal)		{	UNntpTransact::PostMessage(this, inMsg, inData, inDataSize, inPriority);				}
		void ReplaceMessage(Uint32 inMsg, const void *inData = nil, Uint32 inDataSize = 0, Int16 inPriority = priority_Normal)	{	UNntpTransact::ReplaceMessage(this, inMsg, inData, inDataSize, inPriority);				}

		// transport
		void MakeNewTransport() 																			{	UNntpTransact::MakeNewTransport(this);																		}
		TTransport GetTransport()																			{	return UNntpTransact::GetTransport(this);																	}

		// connecting and disconnecting		
		bool IsConnected()																					{	return UNntpTransact::IsConnected(this);																	}
		bool StartConnect(const Uint8 *inServerAddr, const Uint8 *inLogin = nil, const Uint8 *inPassword = nil, Uint32 inMaxSecs = 0)	{	return UNntpTransact::StartConnect(this, inServerAddr, inLogin, inPassword, inMaxSecs);		}
		void Disconnect()																					{	UNntpTransact::Disconnect(this);																			}
		void StartDisconnect()																				{	UNntpTransact::StartDisconnect(this);																		}
		bool IsDisconnecting()																				{	return UNntpTransact::IsDisconnecting(this);																}

		// news server info
		bool IsPostingAllowed()																				{	return UNntpTransact::IsPostingAllowed(this);																}
		Uint32 GetConnectedServer(Uint8 *outServerAddr, Uint32 inMaxSize)									{	return UNntpTransact::GetConnectedServer(this, outServerAddr, inMaxSize);									}

		// command info	
		Uint8 GetCommandID()																				{	return UNntpTransact::GetCommandID(this);																	}
		bool IsCommandExecuted()																			{	return UNntpTransact::IsCommandExecuted(this);																}
		bool IsCommandError()																				{	return UNntpTransact::IsCommandError(this);																	}
		Uint8 *GetCommandError()																			{	return UNntpTransact::GetCommandError(this);																}

		// list newsgroups
		bool Command_ListGroups()																			{	return UNntpTransact::Command_ListGroups(this);																}
		bool Command_ListGroups(const SCalendarDate& inSinceDate)											{	return UNntpTransact::Command_ListGroups(this, inSinceDate);												}

		Uint32 GetGroupCount()																				{	return UNntpTransact::GetGroupCount(this);																	}
		bool GetGroup(SNewsGroupInfo& outGroupInfo, Uint32 inIndex)											{	return UNntpTransact::GetGroup(this, outGroupInfo, inIndex);												}
		bool GetNextGroup(SNewsGroupInfo& outGroupInfo, Uint32& ioIndex)									{	return UNntpTransact::GetNextGroup(this, outGroupInfo, ioIndex);											}
		bool GetPrevGroup(SNewsGroupInfo& outGroupInfo, Uint32& ioIndex)									{	return UNntpTransact::GetPrevGroup(this, outGroupInfo, ioIndex);											}

		// select newsgroup
		bool Command_SelectGroup(const Uint8 *inGroupName, bool inIsPostingAllowed = true)					{	return UNntpTransact::Command_SelectGroup(this, inGroupName, inIsPostingAllowed);							}

		bool IsSelectedGroup()																				{	return UNntpTransact::IsSelectedGroup(this);																}
		bool IsGroupPostingAllowed()																		{	return UNntpTransact::IsGroupPostingAllowed(this);															}
		bool GetSelectedGroup(SNewsGroupInfo& outGroupInfo)													{	return UNntpTransact::GetSelectedGroup(this, outGroupInfo);													}

		// list articles
		bool Command_ListArticles()																			{	return UNntpTransact::Command_ListArticles(this);															}
		bool Command_ListArticles(const SCalendarDate& inSinceDate)											{	return UNntpTransact::Command_ListArticles(this, inSinceDate);												}
	
		Uint32 GetArticleCount()																			{	return UNntpTransact::GetArticleCount(this);																}
		bool GetArticle(SNewsArticleInfo& outArticleInfo, Uint32 inIndex)									{	return UNntpTransact::GetArticle(this, outArticleInfo, inIndex);											}
		bool GetNextArticle(SNewsArticleInfo& outArticleInfo, Uint32& ioIndex)								{	return UNntpTransact::GetNextArticle(this, outArticleInfo, ioIndex);										}
		bool GetPrevArticle(SNewsArticleInfo& outArticleInfo, Uint32& ioIndex)								{	return UNntpTransact::GetPrevArticle(this, outArticleInfo, ioIndex);										}

		// select article
		bool Command_SelectArticle(Uint32 inArticleID)														{	return UNntpTransact::Command_SelectArticle(this, inArticleID);												}
		bool Command_SelectNextArticle()																	{	return UNntpTransact::Command_SelectNextArticle(this);														}
		bool Command_SelectPrevArticle()																	{	return UNntpTransact::Command_SelectPrevArticle(this);														}

		bool IsSelectedArticle()																			{	return UNntpTransact::IsSelectedArticle(this);																}
		Uint32 GetSelectedArticleNumber()																	{	return UNntpTransact::GetSelectedArticleNumber(this);														}
		Uint32 GetSelectedArticleID(Uint8 *outArticleID, Uint32 inMaxSize)									{	return UNntpTransact::GetSelectedArticleID(this, outArticleID, inMaxSize);									}

		// get article header and data
		bool Command_GetSelectedArticle()																	{	return UNntpTransact::Command_GetSelectedArticle(this);														}
		bool Command_GetArticle(Uint32 inArticleNumber)														{	return UNntpTransact::Command_GetArticle(this, inArticleNumber);											}
		bool Command_GetArticle(const Uint8 *inArticleID)													{	return UNntpTransact::Command_GetArticle(this, inArticleID);												}

		// get article header
		bool Command_GetSelectedArticleHeader()																{	return UNntpTransact::Command_GetSelectedArticleHeader(this);												}
		bool Command_GetArticleHeader(Uint32 inArticleNumber)												{	return UNntpTransact::Command_GetArticleHeader(this, inArticleNumber);										}
		bool Command_GetArticleHeader(const Uint8 *inArticleID)												{	return UNntpTransact::Command_GetArticleHeader(this, inArticleID);											}

		bool IsCurrentArticle()																				{	return UNntpTransact::IsCurrentArticle(this);																}
		bool GetArticleInfo(SNewsArticleInfo& outArticleInfo)												{	return UNntpTransact::GetArticleInfo(this, outArticleInfo);													}

		// get article data
		bool Command_GetSelectedArticleData()																{	return UNntpTransact::Command_GetSelectedArticleData(this);													}
		bool Command_GetArticleData(Uint32 inArticleNumber)													{	return UNntpTransact::Command_GetArticleData(this, inArticleNumber);										}
		bool Command_GetArticleData(const Uint8 *inArticleID)												{	return UNntpTransact::Command_GetArticleData(this, inArticleID);											}
		
		bool IsArticleText()																				{	return UNntpTransact::IsArticleText(this);																	}
		Uint32 GetDataSize()																				{	return UNntpTransact::GetDataSize(this);																	}
		Uint32 GetDataCount()																				{	return UNntpTransact::GetDataCount(this);																	}
		SArticleData *GetData(Uint32 inIndex)																{	return UNntpTransact::GetData(this, inIndex);																}
		bool GetNextData(SArticleData*& outArticleData, Uint32& ioIndex)									{	return UNntpTransact::GetNextData(this, outArticleData, ioIndex);											}
		bool GetPrevData(SArticleData*& outArticleData, Uint32& ioIndex)									{	return UNntpTransact::GetPrevData(this, outArticleData, ioIndex);											}
	
		// test article existence
		bool Command_ExistsArticle(const Uint8 *inArticleID)												{	return UNntpTransact::Command_ExistsArticle(this, inArticleID);												}
		
		// post article
		bool Command_PostArticleStart()																		{	return UNntpTransact::Command_PostArticleStart(this);														}
		
		bool AddParentID(const Uint8 *inParentID)															{	return UNntpTransact::AddParentID(this, inParentID);														}
		bool SetArticleID(const Uint8 *inArticleID)															{	return UNntpTransact::SetArticleID(this, inArticleID);														}
		bool SetOrganization(const Uint8 *inOrganization)													{	return UNntpTransact::SetOrganization(this, inOrganization);												}
		bool SetNewsReader(const Uint8 *inNewsReader)														{	return UNntpTransact::SetNewsReader(this, inNewsReader);													}
		bool SetPostDate(SCalendarDate& inPostDate)															{	return UNntpTransact::SetPostDate(this, inPostDate);														}
		bool SetPostInfo(const Uint8 *inArticleName, const Uint8 *inPosterName, const Uint8 *inPosterAddr = nil)	{	return UNntpTransact::SetPostInfo(this, inArticleName, inPosterName, inPosterAddr);					}
		bool AddPostData(const Int8 *inFlavor, const Uint8 *inName, void *inData, Uint32 inDataSize)		{	return UNntpTransact::AddPostData(this, inFlavor, inName, inData, inDataSize);								}
		
		bool Command_PostArticleFinish()																	{	return UNntpTransact::Command_PostArticleFinish(this);														}

		void operator delete(void *p)																		{	UNntpTransact::Dispose((TNntpTransact)p);																	}
	protected:
		TNntpTransactObj() {}				// force creation via UNntpTransact
};

