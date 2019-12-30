/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */


#define USE_LOG_WIN					0
#define INTERNAL_SERVER_PRIORITY	0

#pragma def_inherited on


#pragma mark ₯₯ Constants ₯₯

#pragma mark View IDs
enum {
	viewID_Options			= 101
};

/* ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ */
#pragma mark -
#pragma mark ₯₯ Structures ₯₯

#pragma mark SMyServerInfo
#pragma options align=packed
struct SMyServerInfo {
	Uint32 nameCRC;
	Uint32 timeStamp;
	Uint32 address;
	Uint32 passID;
	Uint16 port;
	Uint16 userCount;
	Uint16 flags;
	Uint8 data[];
};

#pragma options align=reset
#pragma mark SMyServerPerIP
struct SMyServerPerIP {
	Uint32 address;
	Uint32 count;
};

#pragma mark SMyClient
struct SMyClient {
	TTransport tpt;
	Uint32 isEstablished	: 1;
};

#pragma mark SMyLoginInfo
struct SMyLoginInfo
{
	Uint8 psPassword[32];
};

#pragma mark SMyPermBanInfo
struct SMyPermBanInfo
{
	Uint32 nAddr;
};

/* ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ */
#pragma mark -
#pragma mark ₯₯ Views ₯₯

#pragma mark CMyLoginListView
class CMyLoginListView : public CGeneralListView<SMyLoginInfo>
{
	public:
		CMyLoginListView(CViewHandler *inHandler, const SRect &inBounds, CTextView *inPassText);

		Uint32 AddLogin();
		bool ModifyLogin(Uint32 inIndex, Uint8 *inPass);
		bool ModifySelectedLogin();
		bool DeleteSelectedLogin();

		virtual void SetItemSelect(Uint32 inItem, bool inSelect);		

	protected:
		virtual void ItemDraw(Uint32 inItem, TImage inImage, const SRect& inBounds, const SRect& inUpdateRect, Uint32 inOptions = 0);

		CTextView *mPassText;
		Uint32 mLoginListIndex;
};

#pragma mark CMyPermBanListView
class CMyPermBanListView : public CGeneralListView<SMyPermBanInfo>
{
	public:
		CMyPermBanListView(CViewHandler *inHandler, const SRect &inBounds, CTextView *inBanUrlText);

		Uint32 AddPermBan();
		bool ModifyPermBan(Uint32 inIndex, Uint8 *inURL);
		bool ModifySelectedPermBan();
		bool DeleteSelectedPermBan();

		virtual void SetItemSelect(Uint32 inItem, bool inSelect);		

	protected:
		virtual void ItemDraw(Uint32 inItem, TImage inImage, const SRect& inBounds, const SRect& inUpdateRect, Uint32 inOptions = 0);
		
		CTextView *mBanUrlText;
		Uint32 mPermBanListIndex;
};

/* ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ */
#pragma mark -
#pragma mark ₯₯ Windows ₯₯

#pragma mark CMyOptionsWin
class CMyOptionsWin : public CWindow
{
	public:
		CMyOptionsWin();

		void SetInfo(SIPAddress inIPAddress, Uint32 inMaxServersPerIP, CPtrList<SMyLoginInfo> *inLoginList, CPtrList<SMyPermBanInfo> *inPermBanList);
		void GetInfo(SIPAddress *outIPAddress, Uint32 *outMaxServersPerIP, CPtrList<SMyLoginInfo> *outLoginList, CPtrList<SMyPermBanInfo> *outPermBanList);
		SIPAddress GetIPAddress();

		void AddLogin();
		void DeleteLogin();

		void AddPermBan();
		void DeletePermBan();

	protected:
		struct {
			CTextView *ip1Text, *ip2Text, *ip3Text, *ip4Text, *maxServersPerIP;

			CScrollerView *loginScroll;
			CTextView *passText;
			CMyLoginListView *loginList;

			CScrollerView *permBanScroll;
			CTextView *banUrlText;
			CMyPermBanListView *permBanList;
		} mViews;

		CContainerView *MakeGeneralTab();
		CContainerView *MakePasswordTab();
		CContainerView *MakeBanTab();
};

#pragma mark CMyStatsWin
class CMyStatsWin : public CWindow
{
	public:
		CMyStatsWin();

		void SetServerCount(Uint32 inNum);
		void SetUserCount(Uint32 inNum);
		void SetListingCounter(Uint32 inNum);

	protected:
		struct {
			CLabelView *servCountText, *userCountText, *listCountText;
		} mViews;

		Uint32 mServerCount, mUserCount, mListingCounter;
};

#if USE_LOG_WIN
#pragma mark CMyTextWin
class CMyTextWin : public CWindow
{
	public:
		CMyTextWin();

		CTextView *GetTextView()			{	return mTextView;					}
		void ScrollToBottom()				{	mScrollerView->ScrollToBottom();	}

	protected:
		CTextView *mTextView;
		CScrollerView *mScrollerView;
};
#endif

/* ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ */
#pragma mark -
#pragma mark ₯₯ Application ₯₯

#pragma mark CMyApplication
class CMyApplication : public CApplication
{
	public:
		CMyApplication();
		virtual ~CMyApplication();

		void StartUp();

	#if USE_LOG_WIN
		void Log(const Uint8 *inName, const SInternetAddress& inAddr, const Uint8 *inPasswd);
	#endif
		virtual void WindowHit(CWindow *inWindow, const SHitMsgData& inInfo);
		virtual void Error(const SError& inInfo);
		virtual void HandleMessage(void *inObject, Uint32 inMsg, const void *inData, Uint32 inDataSize);
		virtual void Timer(TTimer inTimer);

	protected:
		Uint16 mErrorCount;
		Uint16 mResetCount;

		CMyStatsWin *mStatsWin;
	#if USE_LOG_WIN
		CMyTextWin *mLogWin;
	#endif

		SIPAddress mIPAddress;
		TTransport mRegTpt, mListenTpt, mListenHttpTpt;
		TTimer mRemoveOldTimer;

		THdl mServerList;
		Uint32 mServerCount;
		CPtrList<SMyServerPerIP> mServerPerIPList;
		CPtrList<SMyClient> mClientList;

		TPtr mBanList, mPassList;
		Uint32 mBanCount, mPassCount;

		Uint32 mMaxServersPerIP;
		Uint32 mListingCounter, mUserCount;

		static SMyServerInfo *NewServerInfo(SMyServerInfo *ioInfo, const SInternetAddress& inAddr, Uint16 inUserCount, Uint16 inFlags, Uint32 inPassID, Uint32 inNameCRC, const Uint8 *inName, const Uint8 *inDesc);
		static Uint32 GetServerInfoSize(const SMyServerInfo *inInfo);

		void DoOptions();
		void ResetServer();
		void UpdateDisplay();
		void ProcessRegister();
		void ProcessClients();
		void RegisterServer(const SInternetAddress& inAddr, Uint16 inUserCount, Uint16 inFlags, Uint32 inPassID, const Uint8 *inName, const Uint8 *inDesc, const Uint8 *inPassword);
		bool FindServerByName(Uint32 inCRC, Uint32& outIndex);
		bool FindServerByAddress(const SInternetAddress& inAddr, Uint32& outIndex);
		void RemoveOldServers();
		bool RemoveServer(Uint32 inIndex);
		SMyServerInfo *NameToInfo(const Uint8 inName[]);
		void SendServerList(TTransport inTpt);
		void SendLookup(TTransport inTpt, const Uint8 inName[]);

		void ClearServerList();
		void ClearClientList();

		void ReadPasswords();
		bool IsInvalidPassword(const Uint8 *inPass);
		void WriteLoginInfo(CPtrList<SMyLoginInfo> *inLoginList);
		void ReadLoginInfo(CPtrList<SMyLoginInfo> *outLoginList);

		void ReadBans();
		bool IsBannedAddress(const SInternetAddress& inAddr);
		void WritePermBanInfo(CPtrList<SMyPermBanInfo> *inPermBanList);
		void ReadPermBanInfo(CPtrList<SMyPermBanInfo> *outPermBanList);

		void WriteIPAddress();
		void ReadIPAddress();
		void WriteMaxServersPerIP();
		void ReadMaxServersPerIP();

		void BuildServerPerIPList();
		bool FindServerByAddress(Uint32 inAddr, Uint32& outIndex);
		bool AddServerPerIP(Uint32 inAddr);
		bool RemoveServerPerIP(Uint32 inAddr);
		void ClearServerPerIPList();
};

