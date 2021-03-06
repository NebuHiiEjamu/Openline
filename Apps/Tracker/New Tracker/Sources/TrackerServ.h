/*
 * TrackerServ.h
 * (c)1999 Hotline Communications
 */

#define NEW_TRACKER					0
#define INTERNAL_SERVER_PRIORITY	0


#pragma def_inherited on

#pragma mark ₯₯ Constants ₯₯

#pragma mark View IDs
enum {
	viewID_NewTracker			= 101,
	viewID_EditTracker			= 102,
	viewID_DeleteTracker		= 103,
	viewID_ShowLog				= 104,
	viewID_ShowStats			= 105,
	viewID_ShowServers			= 106,
	viewID_Quit					= 107,
	viewID_GlobalStats			= 108,
	viewID_TrackerList			= 109	
};

#pragma mark Tracker Options
enum {
	myOpt_ActiveTracker		= 0x01,
	myOpt_LogToFile			= 0x02,
	myOpt_LogServerConnect	= 0x04,
	myOpt_LogClientConnect	= 0x08,
	myOpt_LogTrackerStart	= 0x10,
	myOpt_LogTrackerStop	= 0x20
};

enum {
	kMyPrefsVersion		= 1,
	kMyPermBanVersion	= 1,
	kMyLoginVersion		= 1,

	kMaxLogins			= 500,
	kMaxPermBans		= 500,
	
	myWinVis_Toolbar	= 0x01,
	myWinVis_Log		= 0x01,
	myWinVis_Stats		= 0x02,
	myWinVis_Servers	= 0x04
};


/* ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ */
#pragma mark -
#pragma mark ₯₯ Structures ₯₯

#pragma mark SMyTrackerInfo
struct SMyTrackerInfo {
	Uint8 nActive;
	SIPAddress nTrackerIP;
};

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

#pragma mark SMyLogin
struct SMyLogin
{
	Uint8 psLogin[32];
	Uint8 psPassword[32];
};

#pragma mark SMyLoginInfo
#pragma options align=packed
struct SMyLoginInfo
{
	Uint8 nActive;
	Uint8 psLogin[32];
	Uint8 psPassword[32];
	Uint8 psDateTime[33];
	Uint8 nReserved[6];
};
#pragma options align=reset

#pragma mark SMyPermBan
struct SMyPermBan
{
	Uint32 nAddr;
};

#pragma mark SMyPermBanInfo
#pragma options align=packed
struct SMyPermBanInfo
{
	Uint8 nActive;
	Uint32 nAddr;
	Uint8 psDateTime[33];
	Uint16 nReserved;
	Uint8 psDescr[64];
};
#pragma options align=reset


/* ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ */
#pragma mark -
#pragma mark ₯₯ Views ₯₯

#pragma mark CMyTrackerListView
class CMyTrackerListView : public CGeneralCheckListView<SMyTrackerInfo>
{
	public:
		CMyTrackerListView(CViewHandler *inHandler, const SRect &inBounds);
	
		void AddTracker(const SIPAddress &inTrackerIP, bool inIsActive);
		void UpdateTracker(Uint32 inIndex, const SIPAddress &inTrackerIP, bool inIsActive);
		Uint32 GetSelectedTrackerIP(SIPAddress &outTrackerIP);
		
		virtual void SetItemSelect(Uint32 inIndex, bool inSelect);		
		virtual void CheckChanged(Uint32 inIndex, Uint8 inActive);
				
	protected:
		virtual void ItemDraw(Uint32 inItem, TImage inImage, const SRect& inBounds, const SRect& inUpdateRect, Uint32 inOptions = 0);
};

#pragma mark CMyClickLabelView 
class CMyClickLabelView : public CLabelView
{
	public:
		CMyClickLabelView(CViewHandler *inHandler, const SRect& inBounds);
		
		virtual void MouseUp(const SMouseMsgData& inInfo);
};

#pragma mark CMyLoginListView
class CMyLoginListView : public CGeneralCheckListView<SMyLoginInfo>
{
	public:
	#if NEW_TRACKER
		CMyLoginListView(CViewHandler *inHandler, const SRect &inBounds, CTextView *inLoginText, CTextView *inPassText);
	#else
		CMyLoginListView(CViewHandler *inHandler, const SRect &inBounds, CTextView *inPassText);
	#endif
	
		Uint32 AddLogin();
	#if NEW_TRACKER
		bool ModifyLogin(Uint32 inIndex, Uint8 *inLogin, Uint8 *inPass, bool inActive = false);
	#else
		bool ModifyLogin(Uint32 inIndex, Uint8 *inPass, bool inActive = false);
	#endif
		bool ModifySelectedLogin();
		bool DeleteSelectedLogin();
		
		virtual void SetItemSelect(Uint32 inItem, bool inSelect);		
				
	protected:
		virtual void ItemDraw(Uint32 inItem, TImage inImage, const SRect& inBounds, const SRect& inUpdateRect, Uint32 inOptions = 0);
		
	#if NEW_TRACKER
		CTextView *mLoginText;
	#endif
		CTextView *mPassText;
		
		Uint32 mLoginListIndex;
};

#pragma mark CMyPermBanListView
class CMyPermBanListView : public CGeneralCheckListView<SMyPermBanInfo>
{
	public:
		CMyPermBanListView(CViewHandler *inHandler, const SRect &inBounds, CTextView *inBanUrlText, CTextView *inBanDescrText);
	
		Uint32 AddPermBan();
		bool ModifyPermBan(Uint32 inIndex, Uint8 *inURL, Uint8 *inDescr, bool inActive = false);
		bool ModifySelectedPermBan();
		bool DeleteSelectedPermBan();
		
		virtual void SetItemSelect(Uint32 inItem, bool inSelect);		
				
	protected:
		virtual void ItemDraw(Uint32 inItem, TImage inImage, const SRect& inBounds, const SRect& inUpdateRect, Uint32 inOptions = 0);
		
		CTextView *mBanUrlText;
		CTextView *mBanDescrText;
		
		Uint32 mPermBanListIndex;
};


/* ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ */
#pragma mark -
#pragma mark ₯₯ Windows ₯₯

#pragma mark CMyToolbarWin
class CMyToolbarWin : public CWindow
{
	public:
		CMyToolbarWin();
		
		void AddTracker(const SIPAddress &inTrackerIP, bool inIsActive);
		void UpdateTracker(Uint32 inIndex, const SIPAddress &inTrackerIP, bool inIsActive);
		Uint32 GetSelectedTrackerIP(SIPAddress &outTrackerIP);
		void SelectFirstTracker();
		void DeleteSelectedTracker();
		void SetGlobalStats(Uint32 inServerCount, Uint32 inUserCount, Uint32 inListingCounter);
		
		bool IsCollapsed();
		void ToggleState();
				
	protected:
		struct {
			CMyTrackerListView *trackerList;
			CMyClickLabelView *globalStats;
		} mViews;
		
		Uint32 mTrueHeight;
};

#pragma mark CMyOptionsWin
class CMyOptionsWin : public CWindow
{
	public:
		CMyOptionsWin();
			
		void SetInfo(SIPAddress inIPAddress, Uint32 inMaxServersPerIP, Uint32 inOpts, CPtrList<SMyLoginInfo> *inLoginList, CPtrList<SMyPermBanInfo> *inPermBanList);
		void GetInfo(SIPAddress *outIPAddress, Uint32 *outMaxServersPerIP, Uint32 *outOpts, CPtrList<SMyLoginInfo> *outLoginList, CPtrList<SMyPermBanInfo> *outPermBanList);
		SIPAddress GetIPAddress();
		
		void AddLogin();
		void DeleteLogin();

		void AddPermBan();
		void DeletePermBan();

	protected:
		struct {
			CTextView *ip1Text, *ip2Text, *ip3Text, *ip4Text, *maxServersPerIP;
			CCheckBoxView *logToFile, *logServerConnect, *logClientConnect, *logTrackerStart, *logTrackerStop;
			
			CScrollerView *loginScroll;
		#if NEW_TRACKER
			CTextView *loginText;
		#endif
			CTextView *passText;
			CMyLoginListView *loginList;
			
			CScrollerView *permBanScroll;
			CTextView *banUrlText, *banDescrText;
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


/* ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ */
#pragma mark -
#pragma mark ₯₯ Application ₯₯

#pragma mark CMyTracker
class CMyTracker
{
	public:
		CMyTracker(TFSRefObj* inTrackerFolder);
		~CMyTracker();

		void StartTracker();
		void StopTracker(bool inLogStop = true);
		void DeleteTracker();
		
		static void MessageHandler(void *inContext, void *inObject, Uint32 inMsg, const void *inData, Uint32 inDataSize);

		bool IsActive();
		SIPAddress GetIPAddress();
		Uint32 GetServerCount();
		Uint32 GetUserCount();
		Uint32 GetListingCounter();

		bool EditTracker(bool inNewTracker);
		void ShowLogWin();
		void ShowStatsWin();
		void ShowServersWin();
		void BringToFrontWin();

		void RemoveOldServers();
		void ResetTrackerServ();
		
	protected:
		CMyTextWin *mLogWin;
		CMyStatsWin *mStatsWin;
		CMyTextWin *mServersWin;
		TFSRefObj* mTrackerFolder;

		Uint32 mOpts;
		SIPAddress mIPAddress;
		TTransport mRegTpt, mListenTpt, mListenHttpTpt;
		
		THdl mServerList;
		Uint32 mServerCount;
		CPtrList<SMyServerPerIP> mServerPerIPList;
		CPtrList<SMyClient> mClientList;
		CPtrList<SMyLogin> mLoginList;
		CPtrList<SMyPermBan> mPermBanList;
	
		Uint32 mMaxServersPerIP;
		Uint32 mUserCount, mListingCounter;

		void ProcessRegister();
		void ProcessClients();
		void RegisterServer(const SInternetAddress& inAddr, Uint16 inUserCount, Uint16 inFlags, Uint32 inPassID, const Uint8 *inName, const Uint8 *inDesc, const Uint8 *inPass);
		bool FindServerByName(Uint32 inCRC, Uint32& outIndex);
		bool FindServerByAddress(const SInternetAddress& inAddress, Uint32& outIndex);
		SMyServerInfo *NameToInfo(const Uint8 *inName);
		void SendServerList(TTransport inTpt);
		void SendLookup(TTransport inTpt, const Uint8 *inName);
		bool RemoveServer(Uint32 inIndex);

		void ClearServerList();
		void ClearClientList();

		void BuildServerPerIPList();
		bool FindServerByAddress(Uint32 inAddr, Uint32& outIndex);
		bool AddServerPerIP(Uint32 inAddr);
		bool RemoveServerPerIP(Uint32 inAddr);
		void ClearServerPerIPList();

		void UpdateTitles();
		void UpdateStatsWin();
		void UpdateServersWin();
		void UpdateFolderName();

		void Log(const Uint8 *inData, Uint32 inSize);
		void LogServerConnect(const Uint8 *inName, const SInternetAddress& inAddr, const Uint8 *inPass);
		void LogRejectServerConnect(const SInternetAddress& inAddr);
		void LogStartStop(bool inStartStop);
		void LogReset();
		void LogText(const Uint8 *inContextName, const Uint8 *inText);

		static SMyServerInfo *NewServerInfo(SMyServerInfo *ioInfo, const SInternetAddress& inAddress, Uint16 inUserCount, Uint16 inFlags, Uint32 inPassID, Uint32 inNameCRC, const Uint8 *inName, const Uint8 *inDesc, const Uint8 *inPass);
		static Uint32 GetServerInfoSize(const SMyServerInfo *inInfo);

		TFSRefObj* GetPrefsRef();
		bool WritePrefs();
		bool ReadPrefs(SRect *outWinRects, Uint16 *outWinVis);

		TFSRefObj* GetLoginRef();
		void MakeLoginBackup();
		void WriteLoginInfo(CPtrList<SMyLoginInfo> *inLoginList);
		void ReadLoginInfo(CPtrList<SMyLoginInfo> *outLoginList);
		void ReadLoginList();
		bool IsInvalidPassword(const Uint8 *inPassword);
		void ClearLoginList();

		TFSRefObj* GetPermBanRef();
		void MakePermBanBackup();
		void WritePermBanInfo(CPtrList<SMyPermBanInfo> *inPermBanList);
		void ReadPermBanInfo(CPtrList<SMyPermBanInfo> *outPermBanList);
		void ReadPermBanList();
		bool IsBannedAddress(const SInternetAddress& inAddress);
		void ClearPermBanList();
};

#pragma mark CMyApplication
class CMyApplication : public CApplication
{
	public:
		CMyApplication();
		~CMyApplication();
		void StartUp();
		
		virtual void KeyCommand(Uint32 inCmd, const SKeyMsgData& inInfo);
		virtual void WindowHit(CWindow *inWindow, const SHitMsgData& inInfo);
		virtual void Timer(TTimer inTimer);
		virtual void Error(const SError& inInfo);

		void SelectionChanged(SIPAddress inTrackerIP, bool inSelect);
		void CheckChanged(Uint32 inIndex, SIPAddress inTrackerIP, bool inActive);
		bool IsInTrackerList(SIPAddress inTrackerIP);
		
		void UpdateGlobalStats();
		void ResetErrorCount();

	protected:
		CMyToolbarWin *mToolbarWin;

		CPtrList<CMyTracker> mTrackerList;
		TTimer mRemoveOldTimer;
		
		Uint16 mErrorCount;
		Uint16 mResetCount;
		
		void ReadTrackers();
		bool AddTracker(CMyTracker *inTracker);
		
		void DoNewTracker();
		void DoEditTracker();
		void DoDeleteTracker();
		void DoShowTrackerLog();
		void DoShowTrackerStats();
		void DoShowTrackerServers();

		void RemoveOldServers();
		void ResetTrackerServ();

		TFSRefObj* GetPrefsRef();
		bool WritePrefs();
		bool ReadPrefs(SRect *outWinRects, Uint16 *outWinVis);
};

extern CMyApplication *gApp;
