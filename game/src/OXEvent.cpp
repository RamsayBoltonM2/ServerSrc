#include "stdafx.h"
#include "constants.h"
#include "config.h"
#include "questmanager.h"
#include "start_position.h"
#include "packet.h"
#include "buffer_manager.h"
#include "log.h"
#include "char.h"
#include "char_manager.h"
#include "OXEvent.h"
#include "desc.h"
#ifdef __OXEVENT_QUIZ_UPDATE__
	#include <boost/format.hpp>
#endif
#ifdef __EXTENDED_OXEVENT_SYSTEM__
//@version 0.0.1
	#include "db.h"
	#include "p2p.h"
	#include "guild.h"
	#include "item_manager.h"
	#include "item.h"
#endif

bool COXEventManager::Initialize()
{
	m_timedEvent = NULL;
#ifdef __EXTENDED_OXEVENT_SYSTEM__
	m_counterEvent = NULL;
	m_map_answers.clear();
#endif
	m_map_char.clear();
	m_map_attender.clear();
	m_vec_quiz.clear();
	m_list_iplist.clear();

	SetStatus(OXEVENT_FINISH);

	return true;
}

void COXEventManager::Destroy()
{
	CloseEvent();
#ifdef __EXTENDED_OXEVENT_SYSTEM__
	m_map_answers.clear();
#endif

	m_map_char.clear();
	m_map_attender.clear();
	m_vec_quiz.clear();
	m_list_iplist.clear();

	SetStatus(OXEVENT_FINISH);
}

OXEventStatus COXEventManager::GetStatus()
{
	BYTE ret = quest::CQuestManager::instance().GetEventFlag("oxevent_status");

	switch (ret)
	{
		case 0 :
			return OXEVENT_FINISH;

		case 1 :
			return OXEVENT_OPEN;

		case 2 :
			return OXEVENT_CLOSE;

		case 3 :
			return OXEVENT_QUIZ;

		default :
			return OXEVENT_ERR;
	}

	return OXEVENT_ERR;
}

void COXEventManager::SetStatus(OXEventStatus status)
{
	BYTE val = 0;

	switch (status)
	{
		case OXEVENT_OPEN :
			val = 1;
			break;

		case OXEVENT_CLOSE :
			val = 2;
			break;

		case OXEVENT_QUIZ :
			val = 3;
			break;

		case OXEVENT_FINISH :
#ifdef __OXEVENT_QUIZ_UPDATE__
		iCounterQuiz = 0;
#endif
		case OXEVENT_ERR :
		default :
			val = 0;
			break;
	}
	quest::CQuestManager::instance().RequestSetEventFlag("oxevent_status", val);
}

bool COXEventManager::Enter(LPCHARACTER pkChar)
{
	if (GetStatus() == OXEVENT_FINISH)
	{
		sys_log(0, "OXEVENT : map finished. but char enter. %s", pkChar->GetName());
		return false;
	}

	PIXEL_POSITION pos = pkChar->GetXYZ();

	if (pos.x == 896500 && pos.y == 24600)
	{
		return EnterAttender(pkChar);
	}
	else if (pos.x == 896300 && pos.y == 28900)
	{
		return EnterAudience(pkChar);
	}
	else
	{
		sys_log(0, "OXEVENT : wrong pos enter %d %d", pos.x, pos.y);
		return false;
	}

	return false;
}

void COXEventManager::RemoveFromIpList(const char* gelenip){
   std::string silinecekip = gelenip;
   m_list_iplist.erase(silinecekip);
}

void COXEventManager::CheckIpAdr(DWORD pidm){
   LPCHARACTER pkMyChar = CHARACTER_MANAGER::instance().FindByPID(pidm);

   char pkChrIP[250];
   snprintf(pkChrIP, sizeof(pkChrIP), "%s", pkMyChar->GetDesc()->GetHostName());

   for (itertype(m_list_iplist) it = m_list_iplist.begin(); it != m_list_iplist.end(); ++it)
   {
      const std::string& loopdaki_ip = *it;
      if (loopdaki_ip.empty()) return;

      char listdekiIp[250];
      snprintf(listdekiIp, sizeof(listdekiIp), "%s", loopdaki_ip.c_str());

      if (!strcmp(listdekiIp, pkChrIP)) {
         pkMyChar->GetDesc()->DelayedDisconnect(5);
         pkMyChar->ChatPacket(CHAT_TYPE_INFO, "Multi IP detected");
      }
   }
}

bool COXEventManager::EnterAttender(LPCHARACTER pkChar)
{
	DWORD pid = pkChar->GetPlayerID();

	m_map_char.insert(std::make_pair(pid, pid));
	m_map_attender.insert(std::make_pair(pid, pid));
	CheckIpAdr(pid);
	m_list_iplist.insert(pkChar->GetDesc()->GetHostName());

	return true;
}

bool COXEventManager::EnterAudience(LPCHARACTER pkChar)
{
	DWORD pid = pkChar->GetPlayerID();

	m_map_char.insert(std::make_pair(pid, pid));

	return true;
}

bool COXEventManager::AddQuiz(unsigned char level, const char* pszQuestion, bool answer)
{
	if (m_vec_quiz.size() < (size_t) level + 1)
		m_vec_quiz.resize(level + 1);

	struct tag_Quiz tmpQuiz;

	tmpQuiz.level = level;
	strlcpy(tmpQuiz.Quiz, pszQuestion, sizeof(tmpQuiz.Quiz));
	tmpQuiz.answer = answer;

	m_vec_quiz[level].push_back(tmpQuiz);
	return true;
}

bool COXEventManager::ShowQuizList(LPCHARACTER pkChar)
{
	int c = 0;

	for (size_t i = 0; i < m_vec_quiz.size(); ++i)
	{
		for (size_t j = 0; j < m_vec_quiz[i].size(); ++j, ++c)
		{
			pkChar->ChatPacket(CHAT_TYPE_INFO, "%d %s %s", m_vec_quiz[i][j].level, m_vec_quiz[i][j].Quiz, m_vec_quiz[i][j].answer ? LC_TEXT("참") : LC_TEXT("거짓"));
		}
	}

	pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("총 퀴즈 수: %d"), c);
	return true;
}

void COXEventManager::ClearQuiz()
{
	for (unsigned int i = 0; i < m_vec_quiz.size(); ++i)
	{
		m_vec_quiz[i].clear();
	}

	m_vec_quiz.clear();
}

EVENTINFO(OXEventInfoData)
{
	bool answer;

	OXEventInfoData()
	: answer( false )
	{
	}
};

EVENTFUNC(oxevent_timer)
{
	static BYTE flag = 0;
	OXEventInfoData* info = dynamic_cast<OXEventInfoData*>(event->info);

	if ( info == NULL )
	{
		sys_err( "oxevent_timer> <Factor> Null pointer" );
		return 0;
	}

	switch (flag)
	{
		case 0:
			SendNoticeMap(LC_TEXT("10초뒤 판정하겠습니다."), OXEVENT_MAP_INDEX, true);
			flag++;
			return PASSES_PER_SEC(10);

		case 1:
			SendNoticeMap(LC_TEXT("정답은"), OXEVENT_MAP_INDEX, true);

			if (info->answer == true)
			{
				COXEventManager::instance().CheckAnswer(true);
				SendNoticeMap(LC_TEXT("O 입니다"), OXEVENT_MAP_INDEX, true);
			}
			else
			{
				COXEventManager::instance().CheckAnswer(false);
				SendNoticeMap(LC_TEXT("X 입니다"), OXEVENT_MAP_INDEX, true);
			}

			if (LC_IsJapan())
			{
				SendNoticeMap("듩댾궑궫뺴갲귩둖궸댷벍궠궧귏궥갃", OXEVENT_MAP_INDEX, true);
			}
			else
			{
				SendNoticeMap(LC_TEXT("5초 뒤 틀리신 분들을 바깥으로 이동 시키겠습니다."), OXEVENT_MAP_INDEX, true);
			}

			flag++;
			return PASSES_PER_SEC(5);

		case 2:
			COXEventManager::instance().WarpToAudience();
			COXEventManager::instance().SetStatus(OXEVENT_CLOSE);
			SendNoticeMap(LC_TEXT("다음 문제 준비해주세요."), OXEVENT_MAP_INDEX, true);
			flag = 0;
			break;
	}
	return 0;
}

bool COXEventManager::Quiz(unsigned char level, int timelimit)
{
	if (m_vec_quiz.size() == 0) return false;
	if (level > m_vec_quiz.size()) level = m_vec_quiz.size() - 1;
	if (m_vec_quiz[level].size() <= 0) return false;

	if (timelimit < 0) timelimit = 30;

	int idx = number(0, m_vec_quiz[level].size()-1);
#ifdef __OXEVENT_QUIZ_UPDATE__
	iCounterQuiz += 1;
	std::string textCounterQuiz = str(boost::format("Numarul intrebari: %d.") % iCounterQuiz);
	SendNoticeMap(textCounterQuiz.c_str(), OXEVENT_MAP_INDEX, true);
#endif
	SendNoticeMap(LC_TEXT("문제 입니다."), OXEVENT_MAP_INDEX, true);
	SendNoticeMap(m_vec_quiz[level][idx].Quiz, OXEVENT_MAP_INDEX, true);
	SendNoticeMap(LC_TEXT("맞으면 O, 틀리면 X로 이동해주세요"), OXEVENT_MAP_INDEX, true);

	if (m_timedEvent != NULL) {
		event_cancel(&m_timedEvent);
	}

	OXEventInfoData* answer = AllocEventInfo<OXEventInfoData>();

	answer->answer = m_vec_quiz[level][idx].answer;

	timelimit -= 15;
	m_timedEvent = event_create(oxevent_timer, answer, PASSES_PER_SEC(timelimit));

	SetStatus(OXEVENT_QUIZ);

	m_vec_quiz[level].erase(m_vec_quiz[level].begin()+idx);
	return true;
}

#ifdef __EXTENDED_OXEVENT_SYSTEM__
bool COXEventManager::IsLastManStanding()
{
	return (m_map_attender.size() == 1) ? true : false;
}

bool COXEventManager::IsExistKeyAnswers(LPCHARACTER pkChar, int * keyAnswers)
{
    itertype(m_map_answers) it = m_map_answers.find(pkChar->GetPlayerID());

	if (it == m_map_answers.end())
		return false;
	
	*keyAnswers = it->second;
}

void COXEventManager::TruncateAnswers(LPCHARACTER pkChar)
{
	m_map_answers.erase(pkChar->GetPlayerID());
}

void COXEventManager::InsertAnswers(LPCHARACTER pkChar)
{
	int keyAnswers = 0;
	int sumKeys = 0;

	if (!COXEventManager::instance().IsExistKeyAnswers(pkChar, &keyAnswers))
	{
		m_map_answers.insert(std::make_pair(pkChar->GetPlayerID(), OXEVENT_FIRST_ANSWER));
	}

	sumKeys = keyAnswers += 1;
	m_map_answers[pkChar->GetPlayerID()] = sumKeys;
	pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("oxevent_correct_answers"), sumKeys);
}

std::string COXEventManager::GetGuildWinner(LPCHARACTER pkChar)
{
	CGuild * m_name = pkChar->GetGuild();
	return (m_name) ? m_name->GetName() : "EMPTY";
}

bool COXEventManager::IsExistVnum(DWORD dwItemVnum)
{
	return (ITEM_MANAGER::instance().GetTable(dwItemVnum)) ? true : false;	
}

bool COXEventManager::IsExistCount(BYTE count)
{
	return (count > 0 && count <= ITEM_MAX_COUNT) ? true : false;	
}

void COXEventManager::RefreshWinners()
{
	itertype(m_map_char) iter = m_map_char.begin();

	LPCHARACTER pkChar = NULL;
	for (; iter != m_map_char.end(); ++iter)
	{
		pkChar = CHARACTER_MANAGER::instance().FindByPID(iter->second);

		if (pkChar != NULL)
			COXEventManager::instance().InitializePacket(pkChar);
	}	
}

void COXEventManager::RegisterWinner()
{
	if (COXEventManager::instance().IsLastManStanding())
	{
		itertype(m_map_attender) iter = m_map_attender.begin();

		for (; iter != m_map_attender.end(); ++iter)
		{
			LPCHARACTER pkChar = CHARACTER_MANAGER::instance().FindByPID(iter->first);

			if (pkChar)
			{
				char szBuf[CHAT_MAX_LEN + 1];
				snprintf(szBuf, sizeof(szBuf), LC_TEXT("oxevent_announcement_winner"), pkChar->GetName());
				COXEventManager::instance().NoticeAll(szBuf);
				
				char szQuery[QUERY_MAX_LEN + 1];
				snprintf(szQuery, sizeof(szQuery), "INSERT INTO player.oxevent (name, level, empire, job, guild, date, correct_answers) VALUES('%s','%d','%d','%d','%s',NOW(),'%d')", pkChar->GetName(), pkChar->GetLevel(), pkChar->GetEmpire(), pkChar->GetJob(), COXEventManager::Instance().GetGuildWinner(pkChar).c_str(), m_map_answers[pkChar->GetPlayerID()]);
				DBManager::Instance().DirectQuery(szQuery);
				
				COXEventManager::instance().RefreshWinners();
			}
		}
	}
}

void COXEventManager::InitializePacket(LPCHARACTER pkChar)
{
	TPacketCGOxEventData p;
	p.header = HEADER_GC_OXEVENT;

	SQLMsg *extractList = DBManager::instance().DirectQuery("SELECT * FROM player.oxevent ORDER BY date DESC LIMIT %d", OXEVENT_TOP_LIMIT);
	MYSQL_ROW row;
	int i = 0;
			
	if(extractList->uiSQLErrno != 0)
		return;

	while ((row = mysql_fetch_row(extractList->Get()->pSQLResult)))
	{
		p.tempInfo[i] = TPacketCGOxEvent();
		strncpy(p.tempInfo[i].name, row[1], sizeof(p.tempInfo[i].name));
		str_to_number(p.tempInfo[i].level, row[2]);
		strncpy(p.tempInfo[i].guild, row[3], sizeof(p.tempInfo[i].guild));		
		str_to_number(p.tempInfo[i].empire, row[4]);
		str_to_number(p.tempInfo[i].job, row[5]);
		strncpy(p.tempInfo[i].date, row[6], sizeof(p.tempInfo[i].date));	
		str_to_number(p.tempInfo[i].correct_answers, row[7]);			
		i++;
	}

	if(extractList->Get()->uiNumRows < OXEVENT_TOP_LIMIT)
	{
		while (i < OXEVENT_TOP_LIMIT)
		{
			p.tempInfo[i] = TPacketCGOxEvent();
			strncpy(p.tempInfo[i].name, "", sizeof(p.tempInfo[i].name));
			p.tempInfo[i].level = OXEVENT_NO_DATA;
			strncpy(p.tempInfo[i].guild, "", sizeof(p.tempInfo[i].guild));
			p.tempInfo[i].empire = OXEVENT_NO_DATA;
			p.tempInfo[i].job = OXEVENT_NO_DATA;
			strncpy(p.tempInfo[i].date, "", sizeof(p.tempInfo[i].date));			
			p.tempInfo[i].correct_answers = OXEVENT_NO_DATA;
			i++;
		}
	}
	pkChar->GetDesc()->Packet(&p, sizeof(p));
}

void COXEventManager::NoticeAll(std::string msg)
{
	TPacketGGNotice p;
	p.bHeader = HEADER_GG_NOTICE;
	p.lSize = strlen(msg.c_str()) + 1;

	TEMP_BUFFER buf;
	buf.write(&p, sizeof(p));
	buf.write(msg.c_str(), p.lSize);

	P2P_MANAGER::instance().Send(buf.read_peek(), buf.size());

	SendNotice(msg.c_str());
}

void COXEventManager::GetErrByStatus(LPCHARACTER staff, int key)
{
	switch(key)
	{
		case OXEVENT_FINISH:
			staff->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("oxevent_err_status_a")); 
			break;
		case OXEVENT_OPEN:
			staff->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("oxevent_err_status_b")); 
			break;
		case OXEVENT_CLOSE:
			staff->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("oxevent_err_status_c")); 
			break;
		case OXEVENT_QUIZ:
			staff->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("oxevent_err_status_d")); 
			break;
		default:
			break;
	}
}

EVENTINFO(end_oxevent_info)
{
	int empty;

	end_oxevent_info() 
	: empty( 0 )
	{
	}
};

EVENTFUNC(end_oxevent)
{
	COXEventManager::instance().CloseEvent();
	return 0;
}

EVENTINFO(TOxEventCounterInfo)
{
    DynamicCharacterPtr pkChar;
    TOxEventCounterInfo() : pkChar(){}
};

void COXEventManager::Disconnect(LPCHARACTER pkChar)
{
	m_map_answers.erase(pkChar->GetPlayerID());
	m_map_char.erase(pkChar->GetPlayerID());
	m_map_attender.erase(pkChar->GetPlayerID());
}

EVENTFUNC(start_counter_oxevent)
{   
    TOxEventCounterInfo* info = dynamic_cast<TOxEventCounterInfo*>(event->info);
    
    if (info == NULL)
        return 0;
    
    LPCHARACTER pkChar = info->pkChar;
    
    if (pkChar == NULL)
        return 0;

	pkChar->ChatPacket(CHAT_TYPE_COMMAND, "oxevent_manager refresh %d %d", COXEventManager::instance().GetAttenderCount(), COXEventManager::instance().GetObserverCount());
	return PASSES_PER_SEC(3);
}
	
void COXEventManager::OpenGui(LPCHARACTER pkChar)
{
	if (m_counterEvent != NULL)
	{
		event_cancel(&m_counterEvent);
	}

	TOxEventCounterInfo* info = AllocEventInfo<TOxEventCounterInfo>();
	info->pkChar = pkChar;

	m_counterEvent = event_create(start_counter_oxevent, info, PASSES_PER_SEC(1));	
	pkChar->ChatPacket(CHAT_TYPE_COMMAND, "oxevent_manager open");
}

/*
bool COXEventManager::CheckPassword(LPCHARACTER pkChar, const char* c_szPassword)
{
}
*/

void COXEventManager::Manager(LPCHARACTER pkChar, const char* c_pData)
{
	TPacketCGOxEventManager * p = (TPacketCGOxEventManager *)c_pData;
	
	OXEventStatus resultKey = COXEventManager::instance().GetStatus();
	
	if (!pkChar)
		return;
	
	if (pkChar->GetMapIndex() != OXEVENT_MAP_INDEX)
		return;
	
	if (pkChar->GetGMLevel() != GM_IMPLEMENTOR)
		return;
	
	if (p->type > OXEVENT_FORCE_CLOSE_EVENT)
		return;
	
	switch(p->type)
	{
		case OXEVENT_OPEN_LOGIN:
		{
			if (strlen(p->password) > 12)
			{
				pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("oxevent_wrong_password_length"));
				return;
			}

			if (strcmp(p->password, PASSWORD_MANAGER))
			{
				pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("oxevent_wrong_password"));
				return;		
			}

			COXEventManager::instance().OpenGui(pkChar);
		}
		break;
			
		case OXEVENT_OPEN_EVENT:
		{
			if (resultKey == OXEVENT_FINISH)
			{
				COXEventManager::instance().ClearQuiz();
				
				char szFile[256];
				snprintf(szFile, sizeof(szFile), "%s/oxquiz.lua", LocaleService_GetBasePath().c_str());
				int result = lua_dofile(quest::CQuestManager::instance().GetLuaState(), szFile);
				
				if (result != 0)
				{
					pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("oxevent_error_file_quiz"));	
					return;
				}
				else
				{
					pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("oxevent_succes_file_quiz"));	
					pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("oxevent_succes_start"));	

					COXEventManager::instance().SetStatus(OXEVENT_OPEN);
					COXEventManager::instance().NoticeAll(LC_TEXT("oxevent_start_message_1"));
					COXEventManager::instance().NoticeAll(LC_TEXT("oxevent_start_message_2"));
				}
			}
			else
				COXEventManager::instance().GetErrByStatus(pkChar, resultKey);
		}
		break;
		
		case OXEVENT_CLOSE_GATES:
		{
			if (resultKey == OXEVENT_OPEN)
			{
				COXEventManager::instance().SetStatus(OXEVENT_CLOSE);
				COXEventManager::instance().NoticeAll(LC_TEXT("oxevent_close_gates"));
			}
			else
				COXEventManager::instance().GetErrByStatus(pkChar, resultKey);	
		}
		break;
		
		case OXEVENT_CLOSE_EVENT:
		{
			if (resultKey == OXEVENT_CLOSE)
			{
				COXEventManager::instance().RegisterWinner();
				COXEventManager::instance().InitializePacket(pkChar);
				COXEventManager::instance().SetStatus(OXEVENT_FINISH);

				end_oxevent_info* info = AllocEventInfo<end_oxevent_info>();
				event_create(end_oxevent, info, PASSES_PER_SEC(15));
				
				COXEventManager::instance().NoticeAll(LC_TEXT("oxevent_finish"));
			}
			else
				COXEventManager::instance().GetErrByStatus(pkChar, resultKey);	
		}
		break;
		
		case OXEVENT_ASK_QUESTION:
		{
			if (resultKey == OXEVENT_CLOSE)
			{
				bool ret = COXEventManager::instance().Quiz(1, 30);
				
				if (ret == false)
				{
					pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("oxevent_fail_question"));	
					return;					
				}
				else
				{
					pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("oxevent_succes_question"));	
				}
			}
			else
				COXEventManager::instance().GetErrByStatus(pkChar, resultKey);	
		}
		break;
		
		case OXEVENT_REWARD_PLAYERS:
		{
			if (resultKey == OXEVENT_CLOSE)
			{
				if (!COXEventManager::instance().IsExistCount(p->count))
				{
					pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("oxevent_fail_reward_count"));
					return;
				}
				
				if (COXEventManager::instance().IsExistVnum(p->vnum))
				{
					TItemTable * item = ITEM_MANAGER::instance().GetTable(p->vnum);
					
					DWORD count;

					char hyperlinks[512 + 1];
					snprintf(hyperlinks, sizeof(hyperlinks), "item:%x:%x:%x:%x:%x", p->vnum, item->dwFlags, 0, 0, 0);
					
					if (item->dwFlags == ITEM_FLAG_STACKABLE)
						count = p->count;
					else
						count = 1;

					char szBuf[512 + 1];
					snprintf(szBuf, sizeof(szBuf), "%s %d x |cffffc700|H%s|h[%s]|h|r", LC_TEXT("oxevent_reward_item_chat"), count, hyperlinks, item->szLocaleName);
					
					COXEventManager::instance().NoticeAll(szBuf);
					COXEventManager::instance().GiveItemToAttender(p->vnum, count);
				}
				else
					pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("oxevent_fail_reward_item"));	
			}
			else
				COXEventManager::instance().GetErrByStatus(pkChar, resultKey);	
		}
		break;
		
		case OXEVENT_FORCE_CLOSE_EVENT:
		{
			if (resultKey != OXEVENT_FINISH)
			{
				COXEventManager::instance().CloseEvent();
				COXEventManager::instance().SetStatus(OXEVENT_FINISH);
				
				COXEventManager::instance().NoticeAll(LC_TEXT("oxevent_force_close"));
			}
			else
				COXEventManager::instance().GetErrByStatus(pkChar, OXEVENT_FINISH);	
		}
		break;		

		default:
			break;
	}
}
#endif

bool COXEventManager::CheckAnswer(bool answer)
{
	if (m_map_attender.size() <= 0) return true;

	itertype(m_map_attender) iter = m_map_attender.begin();
	itertype(m_map_attender) iter_tmp;

	m_map_miss.clear();

	int rect[4];
	if (answer != true)
	{
		rect[0] = 892600;
		rect[1] = 22900;
		rect[2] = 896300;
		rect[3] = 26400;
	}
	else
	{
		rect[0] = 896600;
		rect[1] = 22900;
		rect[2] = 900300;
		rect[3] = 26400;
	}

	LPCHARACTER pkChar = NULL;
	PIXEL_POSITION pos;
	for (; iter != m_map_attender.end();)
	{
		pkChar = CHARACTER_MANAGER::instance().FindByPID(iter->second);
		if (pkChar != NULL)
		{
			pos = pkChar->GetXYZ();

			if (pos.x < rect[0] || pos.x > rect[2] || pos.y < rect[1] || pos.y > rect[3])
			{
#ifdef __EXTENDED_OXEVENT_SYSTEM__
				COXEventManager::instance().TruncateAnswers(pkChar);
#endif
				pkChar->EffectPacket(SE_FAIL);
				iter_tmp = iter;
				iter++;
				m_map_attender.erase(iter_tmp);
				m_map_miss.insert(std::make_pair(pkChar->GetPlayerID(), pkChar->GetPlayerID()));
			}
			else
			{
				pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("정답입니다!"));
#ifdef __EXTENDED_OXEVENT_SYSTEM__
				COXEventManager::instance().InsertAnswers(pkChar);
#endif
				// pkChar->CreateFly(number(FLY_FIREWORK1, FLY_FIREWORK6), pkChar);
				char chatbuf[256];
				int len = snprintf(chatbuf, sizeof(chatbuf),
						"%s %u %u", number(0, 1) == 1 ? "cheer1" : "cheer2", (DWORD)pkChar->GetVID(), 0);

				// 리턴값이 sizeof(chatbuf) 이상일 경우 truncate되었다는 뜻..
				if (len < 0 || len >= (int) sizeof(chatbuf))
					len = sizeof(chatbuf) - 1;

				// \0 문자 포함
				++len;

				TPacketGCChat pack_chat;
				pack_chat.header = HEADER_GC_CHAT;
				pack_chat.size = sizeof(TPacketGCChat) + len;
				pack_chat.type = CHAT_TYPE_COMMAND;
				pack_chat.id = 0;

				TEMP_BUFFER buf;
				buf.write(&pack_chat, sizeof(TPacketGCChat));
				buf.write(chatbuf, len);

				pkChar->PacketAround(buf.read_peek(), buf.size());
				pkChar->EffectPacket(SE_SUCCESS);

				++iter;
			}
		}
		else
		{
			itertype(m_map_char) err = m_map_char.find(iter->first);
			if (err != m_map_char.end()) m_map_char.erase(err);

			itertype(m_map_miss) err2 = m_map_miss.find(iter->first);
			if (err2 != m_map_miss.end()) m_map_miss.erase(err2);

			iter_tmp = iter;
			++iter;
			m_map_attender.erase(iter_tmp);
		}
	}
	return true;
}

void COXEventManager::WarpToAudience()
{
	if (m_map_miss.size() <= 0) return;

	itertype(m_map_miss) iter = m_map_miss.begin();
	LPCHARACTER pkChar = NULL;

	for (; iter != m_map_miss.end(); ++iter)
	{
		pkChar = CHARACTER_MANAGER::instance().FindByPID(iter->second);

		if (pkChar != NULL)
		{
			switch ( number(0, 3))
			{
				case 0 : pkChar->Show(OXEVENT_MAP_INDEX, 896300, 28900); break;
				case 1 : pkChar->Show(OXEVENT_MAP_INDEX, 890900, 28100); break;
				case 2 : pkChar->Show(OXEVENT_MAP_INDEX, 896600, 20500); break;
				case 3 : pkChar->Show(OXEVENT_MAP_INDEX, 902500, 28100); break;
				default : pkChar->Show(OXEVENT_MAP_INDEX, 896300, 28900); break;
			}
		}
	}

	m_map_miss.clear();
}

bool COXEventManager::CloseEvent()
{
	if (m_timedEvent != NULL) {
		event_cancel(&m_timedEvent);
	}

	itertype(m_map_char) iter = m_map_char.begin();

	LPCHARACTER pkChar = NULL;
	for (; iter != m_map_char.end(); ++iter)
	{
		pkChar = CHARACTER_MANAGER::instance().FindByPID(iter->second);

		if (pkChar != NULL)
			pkChar->WarpSet(EMPIRE_START_X(pkChar->GetEmpire()), EMPIRE_START_Y(pkChar->GetEmpire()));
	}

	m_map_char.clear();

	return true;
}

bool COXEventManager::LogWinner()
{
	itertype(m_map_attender) iter = m_map_attender.begin();

	for (; iter != m_map_attender.end(); ++iter)
	{
		LPCHARACTER pkChar = CHARACTER_MANAGER::instance().FindByPID(iter->second);

		if (pkChar)
			LogManager::instance().CharLog(pkChar, 0, "OXEVENT", "LastManStanding");
	}

	return true;
}

bool COXEventManager::GiveItemToAttender(DWORD dwItemVnum, BYTE count)
{
	itertype(m_map_attender) iter = m_map_attender.begin();

	for (; iter != m_map_attender.end(); ++iter)
	{
		LPCHARACTER pkChar = CHARACTER_MANAGER::instance().FindByPID(iter->second);

		if (pkChar)
		{
			pkChar->AutoGiveItem(dwItemVnum, count);
			LogManager::instance().ItemLog(pkChar->GetPlayerID(), 0, count, dwItemVnum, "OXEVENT_REWARD", "", pkChar->GetDesc()->GetHostName(), dwItemVnum);
		}
	}

	return true;
}

