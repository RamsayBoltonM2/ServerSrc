#define OXEVENT_MAP_INDEX 113

#ifdef __EXTENDED_OXEVENT_SYSTEM__

	#define PASSWORD_MANAGER "metin2team" // Change here password for login in event manager.

	enum ETypeInfoOxEvent
	{
		OXEVENT_TOP_LIMIT = 5,
		OXEVENT_FIRST_ANSWER = 1,
		OXEVENT_NO_DATA = 999,
		OXEVENT_OPEN_LOGIN = 0,
		OXEVENT_OPEN_EVENT = 1,
		OXEVENT_CLOSE_GATES = 2,
		OXEVENT_CLOSE_EVENT = 3,
		OXEVENT_REWARD_PLAYERS = 4,
		OXEVENT_ASK_QUESTION = 5,
		OXEVENT_FORCE_CLOSE_EVENT = 6,
	};
#endif

struct tag_Quiz
{
	char level;
	char Quiz[256];
	bool answer;
};

enum OXEventStatus
{
	OXEVENT_FINISH = 0, // OX이벤트가 완전히 끝난 상태
	OXEVENT_OPEN = 1,	// OX이벤트가 시작됨. 을두지(20012)를 통해서 입장가능
	OXEVENT_CLOSE = 2,	// OX이벤트의 참가가 끝남. 을두지(20012)를 통한 입장이 차단됨
	OXEVENT_QUIZ = 3,	// 퀴즈를 출제함.

	OXEVENT_ERR = 0xff
};

class COXEventManager : public singleton<COXEventManager>
{
	private :
		std::map<DWORD, DWORD> m_map_char;
		std::map<DWORD, DWORD> m_map_attender;
		std::map<DWORD, DWORD> m_map_miss;
#ifdef __EXTENDED_OXEVENT_SYSTEM__
		std::map<DWORD, DWORD> m_map_answers;
#endif

		std::vector<std::vector<tag_Quiz> > m_vec_quiz;
#ifdef __OXEVENT_QUIZ_UPDATE__
		DWORD iCounterQuiz;
#endif
		LPEVENT m_timedEvent;
#ifdef __EXTENDED_OXEVENT_SYSTEM__
		LPEVENT m_counterEvent;
#endif

	protected :
		bool CheckAnswer();

		bool EnterAudience(LPCHARACTER pChar);
		bool EnterAttender(LPCHARACTER pChar);
		std::set<std::string> m_list_iplist;

	public :
		bool Initialize();
		void Destroy();

		OXEventStatus GetStatus();
		void SetStatus(OXEventStatus status);

		bool LoadQuizScript(const char* szFileName);

		void CheckIpAdr(DWORD pidm);
		void RemoveFromIpList(const char* gelenip);

		bool Enter(LPCHARACTER pChar);

		bool CloseEvent();

		void ClearQuiz();
		bool AddQuiz(unsigned char level, const char* pszQuestion, bool answer);
		bool ShowQuizList(LPCHARACTER pChar);
#ifdef __EXTENDED_OXEVENT_SYSTEM__
		DWORD GetObserverCount() { return (m_map_char.size() - m_map_attender.size() > 0) ? m_map_char.size() - m_map_attender.size() : 0; }
		std::string GetGuildWinner(LPCHARACTER pkChar);

		bool IsExistVnum(DWORD dwItemVnum);
		bool IsExistCount(BYTE count);
		bool IsExistKeyAnswers(LPCHARACTER pChar, int * keyAnswers);
		//bool CheckPassword(LPCHARACTER pChar, const char* c_szPassword);
		
		bool IsLastManStanding();
		void InsertAnswers(LPCHARACTER pChar);
		void TruncateAnswers(LPCHARACTER pChar);
		void RegisterWinner();
		void InitializePacket(LPCHARACTER pChar);
		
		void RefreshWinners();
		void OpenGui(LPCHARACTER pChar);
		void Disconnect(LPCHARACTER pChar);
		
		void GetErrByStatus(LPCHARACTER pChar, int key);

		void NoticeAll(std::string msg);
		void Manager(LPCHARACTER ch, const char* c_pData);
#endif

		bool Quiz(unsigned char level, int timelimit);
		bool GiveItemToAttender(DWORD dwItemVnum, BYTE count);

		bool CheckAnswer(bool answer);
		void WarpToAudience();

		bool LogWinner();

		DWORD GetAttenderCount() { return m_map_attender.size(); }
};

