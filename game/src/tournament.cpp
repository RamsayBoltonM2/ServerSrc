/*********************************************************************
* title_name		: Tournament PvP
* date_created		: 2017.02.12 / 01:41
* filename			: tournament.cpp
* author			: VegaS
* version_actual	: Version 0.0.3
* Created for		: Terabithia.pl
*/

#include "stdafx.h"
#include "desc.h"
#include "utils.h"
#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "affect.h"
#include "start_position.h"
#include "p2p.h"
#include "db.h"
#include "dungeon.h"
#include "castle.h"
#include <string>
#include <boost/algorithm/string/replace.hpp>
#include "desc_manager.h"
#include "buffer_manager.h"
#include "dev_log.h"
#include <fstream>
#include <algorithm>
#include <iostream>
#include "constants.h"
#include "questmanager.h"
#include "desc_client.h"
#include "sectree_manager.h"
#include "regen.h"
#include "item.h"
#include <boost/format.hpp>
#include "item_manager.h"
#include "tournament.h"

#define insert_winners(fmt, ...) RegisterWinners(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#define FILENAME_STATUS			"tournament_status.txt"
#define FILENAME_LOG_WINNER		"tournament_logs_winners.txt"
#define FILENAME_BLOCK_ITEMS	"tournament_forbidden_items.txt"
#define MYSQL_DATABASE_RANKING	"player.tournament_ranking"

std::map<DWORD, DWORD> m_map_category_low;	
std::map<DWORD, DWORD> m_map_category_medium;	
std::map<DWORD, DWORD> m_map_category_hard;
static LPEVENT running_event = NULL;

static const char* LC_TRANSLATE(const char* key) /* Configurable */
{
	typedef std::map<const char *, const char *> TMapTranslate;
	TMapTranslate LC_TRANSLATE;
	
	LC_TRANSLATE["TOURNAMENT_CATEGORY_LOW"]								= "Acemi";	
	LC_TRANSLATE["TOURNAMENT_CATEGORY_MEDIUM"]							= "Deneyimli";	
	LC_TRANSLATE["TOURNAMENT_CATEGORY_HARD"]							= "Uzman";

	LC_TRANSLATE["TOURNAMENT_TEAM_MEMBER_RED"]							= "Kýrmýzý";		
	LC_TRANSLATE["TOURNAMENT_TEAM_MEMBER_BLUE"]							= "Mavi";	
	
	LC_TRANSLATE["TOURNAMENT_ANNOUNCEMENT_BEFORE_START"]				= "[Turnuva] %s kategori turnuvasý %d dakika içerisinde baþlayacak.";

	LC_TRANSLATE["TOURNAMENT_ANNOUNCEMENT_START_LINE_1"]				= "[Turnuva] %s kategori turnuvasý baþladý, iki tarafada bol þans! ";
	LC_TRANSLATE["TOURNAMENT_ANNOUNCEMENT_START_LINE_2"]				= "[Turnuva] Turnuva %d dakika boyunca devam edecek. ";
	LC_TRANSLATE["TOURNAMENT_ANNOUNCEMENT_START_LINE_3"]				= "[Turnuva] Turnuvayý izlemek isteyen oyuncular görev penceresini kullanabilir. ";

	LC_TRANSLATE["TOURNAMENT_ANNOUNCEMENT_NOT_STARTED_LINE_1"]			= "[Turnuva] %s kategori için oyuncu sayýsý yeterli deðil, turnuva baþlamayacak. ";
	LC_TRANSLATE["TOURNAMENT_ANNOUNCEMENT_NOT_STARTED_LINE_2"]			= "[Turnuva] %s'de %s kategori turnuvasý baþlayacak. ";

	LC_TRANSLATE["TOURNAMENT_ANNOUNCEMENT_FINISHED_LINE_1"]				= "[Turnuva] %s kategori turnuvasý sona erdi. ";
	LC_TRANSLATE["TOURNAMENT_ANNOUNCEMENT_FINISHED_LINE_2"]				= "[Turnuva] %s'de %s kategori turnuvasý baþlayacak. ";

	LC_TRANSLATE["TOURNAMENT_ANNOUNCEMENT_WINNER_SAME"]					= "[Turnuva] Turnuva berabere bitti, iki takýmda ayný sayýda skor aldý. ";
	LC_TRANSLATE["TOURNAMENT_ANNOUNCEMENT_WINNER_NOT_EXIST"]			= "[Turnuva] Ýki tarafta turnuvayý kazanamadý, çünkü turnuva alanýnda kimse kalmadý. ";
	LC_TRANSLATE["TOURNAMENT_ANNOUNCEMENT_WINNER_SUCCES"]				= "[Turnuva] %s takým turnuvayayý kazandý, turnuva alanýnda %d üyesi hayatta kalmayý baþardý. ";

	LC_TRANSLATE["TOURNAMENT_MEMBER_REMAINING_LIFE"]					= "[Turnuva] %d yaþam hakkýn kaldý. ";
	LC_TRANSLATE["TOURNAMENT_MEMBER_FINISHED_LIFE_LINE_1"]				= "[Turnuva] Hiç yaþam hakkýn kalmadý. ";
	LC_TRANSLATE["TOURNAMENT_MEMBER_FINISHED_LIFE_LINE_2"]				= "[Turnuva] Turnuvadan elendin, bidahaki sefere görüþmek üzere. ";
	LC_TRANSLATE["TOURNAMENT_MEMBER_DIVIDED"]							= "[Turnuva] %s takýma kayýt edildin. ";

	LC_TRANSLATE["TOURNAMENT_MEMBER_BLOCK_DUEL"]						= "[Turnuva] Burada düello isteðinde bulunamazsýn. ";
	LC_TRANSLATE["TOURNAMENT_MEMBER_BLOCK_PARTY"]						= "[Turnuva] Burada grup daveti gönderemezsin. ";
	LC_TRANSLATE["TOURNAMENT_MEMBER_BLOCK_RING_MARRIAGE"]				= "[Turnuva] Burada nikah yüzüðünü kullanamazsýn. ";
	LC_TRANSLATE["TOURNAMENT_MEMBER_BLOCK_POLY"]						= "[Turnuva] Burada dönüþemezsin. ";
	LC_TRANSLATE["TOURNAMENT_MEMBER_BLOCK_CHANGE_PKMODE"]				= "[Turnuva] Burada PK modunu deðiþemezsin. ";
	LC_TRANSLATE["TOURNAMENT_MEMBER_BLOCK_MOUNT"]						= "[Turnuva] Burada bineðini çaðýramazsýn. ";
	LC_TRANSLATE["TOURNAMENT_MEMBER_BLOCK_HORSE"]						= "[Turnuva] Burada atýný çaðýramazsýn. ";
	LC_TRANSLATE["TOURNAMENT_MEMBER_BLOCK_EXIT_OBSERVER_MODE_LINE_1"]	= "[Turnuva] Burada izleyici modunu deðiþtiremezsin. ";
	LC_TRANSLATE["TOURNAMENT_MEMBER_BLOCK_EXIT_OBSERVER_MODE_LINE_2"]	= "[Turnuva] Turnuva alanýnda ayrýlmak için görev penceresini kullan. ";

	LC_TRANSLATE["TOURNAMENT_MEMBER_OPEN_REWARD_WRONG_SIZE"]			= "[Turnuva] Bu nesneyi kullanmak için envanterinde en az [%d] depolama alaný olmalýdýr. ";
	LC_TRANSLATE["TOURNAMENT_MEMBER_CANNOT_USE_ITEM"]					= "[Turnuva] Nesne: %s, kýsýtlanmýþ. ";
	LC_TRANSLATE["TOURNAMENT_INSERT_LOG_WINNERS"]						= "Ýsim: %s | Level: %d | IP: %s | Yaþam hakký: %d | Takým: %s";	

#ifdef ENABLE_EXTRA_LIVES_TOURNAMENT
	LC_TRANSLATE["TOURNAMENT_MEMBER_ADD_EXTRA_LIVES_ERROR_LINE_1"]		= "[Turnuva] Burada bu nesneyi kullanamazsýn. ";
	LC_TRANSLATE["TOURNAMENT_MEMBER_ADD_EXTRA_LIVES_ERROR_LINE_2"]		= "[Turnuva] Bunu son zamanlarda zaten yaptýn, bu kadar acele etme. Ekstra yaþam haklarýný bitirmen gerekiyor. ";
	LC_TRANSLATE["TOURNAMENT_MEMBER_ADD_EXTRA_LIVES_ERROR_LINE_3"]		= "[Turnuva] Kalan yaþam hakký: %d / %d. ";
	LC_TRANSLATE["TOURNAMENT_MEMBER_ADD_EXTRA_LIVES_SUCCES"]			= "[Turnuva] Tebrikler turnuvaya +%d yaþam hakký eklendi. Kullanýlabilir %d. ";
#endif
	
	return LC_TRANSLATE[key];
}

const int TOURNAMENT_INFO_TIMER[TOURNAMENT_MAX_CATEGORY][4] = /* Configurable */
{
	/*
		*	Example: { 14, 00,	14, 30 }
				* Row 1 (14):  Hour when start
				* Row 2 (00):  Min when start
				* Row 3 (14):  Hour when finish
				* Row 4 (30):  Min when finish

		*	Not put duration start to finish more 59min because coded is seted for max 59min calculation to show.
		*	If you want to put to start on 14:30, so time to finish need to be max. 14:59. no put more then 59 because nothing will happens.
		*	No put timer on minute like: 14:06, 16:09, need to be all from 10+ like: 14:10, 14:11, 14:12 etc. but not 01/02/03/04/05/06/07/08/09, is not so important not will be problem or something, but just will appear on announcement something like: 14:2 , not 14:02.
	*/
	{ 12, 30,	13, 00 }, // TOURNAMENT_CATEGORY_LOW
	{ 20, 30,	21, 00 }, // TOURNAMENT_CATEGORY_MEDIUM
	{ 21, 45,	22, 15 }  // TOURNAMENT_CATEGORY_HARD
};

long g_position_tournament[2][2] = /* Configurable */
{
	{ 5014000,	5011900 },	// TEAM_MEMBERS_A
	{ 5008300,	5013000 },	// TEAM_MEMBERS_B
};

long g_observers_position_tournament[3][2] = /* Configurable */
{
	{ 5015200,	5009500 },	// Random 1
	{ 5015400,	5013100 },	// Random 2
	{ 5015000,	5016500 }	// Random 3
};

bool CTournamentPvP::file_is_empty(std::ifstream& file)
{
	return (file.peek() == std::ifstream::traits_type::eof() == true || !file || !file.is_open()) ? true : false;
}

int CTournamentPvP::GetStatus()
{
	int key = 0;
	char szFileName[256];
	snprintf(szFileName, sizeof(szFileName), "%s/%s", LocaleService_GetBasePath().c_str(), FILENAME_STATUS);

	std::ifstream file(szFileName);
		
	if (!file.is_open())
	{
		sys_err("Error %s", szFileName);
		return false;
	}

	file >> key;
	file.close();

	return key;
}

void CTournamentPvP::WriteStatus(int key)
{
	char szFileName[256];
	snprintf(szFileName, sizeof(szFileName), "%s/%s", LocaleService_GetBasePath().c_str(), FILENAME_STATUS);

	FILE* file = NULL;	
	file = fopen(szFileName, "w+");

	if (!file)
	{
		sys_err("Error %s", szFileName);
		return;
	}
	
	fprintf(file, "");
	
	fprintf(file, "%d", key);	
	fclose(file);
}

void RegisterWinners(const char *func, int line, const char *format, ...)
{
	va_list kwargs;
	time_t vKey = time(0);  
	char *time_s = asctime(localtime(&vKey));

	FILE* file = NULL;
	char szBuf[1024 + 2];
	int length;
	
	char szFileName[256];
	snprintf(szFileName, sizeof(szFileName), "%s/%s", LocaleService_GetBasePath().c_str(), FILENAME_LOG_WINNER);
	file = fopen(szFileName, "a+");

	if (!file)
	{
		sys_err("Error %s", szFileName);
		return;
	}

	time_s[strlen(time_s) - 1] = '\0';
	length = snprintf(szBuf, 1024, "Date: %-15.15s | ", time_s + 4);
	szBuf[1025] = '\0';

	if (length < 1024)
	{
		va_start(kwargs, format);
		vsnprintf(szBuf + length, 1024 - length, format, kwargs);
		va_end(kwargs);
	}

	strcat(szBuf, "\n");

	fputs(szBuf, file);
	fflush(file);

	fputs(szBuf, stdout);
	fflush(stdout);
}

void CTournamentPvP::ReadFileItems()
{
	char szFileName[256];
	snprintf(szFileName, sizeof(szFileName), "%s/%s", LocaleService_GetBasePath().c_str(), FILENAME_BLOCK_ITEMS);
	std::ifstream file(szFileName);

	if (!file.is_open())
	{
		sys_err("Error %s", szFileName);
		return;
	}

	std::string line;
	while (getline(file, line))
	{
		if (line.empty())
			continue;
		
		DWORD iVnum = atoi(line.c_str());
		m_listForbidden.push_back(iVnum);
	}
}
	
bool CTournamentPvP::GetIsPermission_Day()
{
	time_t cur_Time = time(NULL);
	struct tm vKey = *localtime(&cur_Time);

	int adwListAccesDays[] =
	{
		/*
			Explanation: 
				Add in list days what you want to run tournament.
				Day of the month (1 - 31).
		*/
		3,
		6,
		9,
		12,
		18,
		19,
		21,
		25,
		28
	};
	
	for (int key = 0; key < _countof(adwListAccesDays); key++)
	{
		if (vKey.tm_mday == adwListAccesDays[key])
			return true;
	}
	
	return false;
}

void CTournamentPvP::PrepareAnnouncement()
{
	if (!CTournamentPvP::instance().GetIsPermission_Day())
		return;

	time_t cur_Time = time(NULL);
	struct tm vKey = *localtime(&cur_Time);
	unsigned int categoryIndex = 0;

	const int c_pszArray[6] =
	{
		TOURNAMENT_PASSER_NOTICE - TOURNAMENT_PASSER_NOTICE,
		TOURNAMENT_PASSER_NOTICE * 1,
		TOURNAMENT_PASSER_NOTICE * 2,
		TOURNAMENT_PASSER_NOTICE * 3,
		TOURNAMENT_PASSER_NOTICE * 4,
		TOURNAMENT_PASSER_NOTICE * 5
		/*
			* 1 hour before the tournament start will notice in game on every 10m.
			* eg. Category Low - Start on 14:00
			* Will start to notice from 13:00 like this:
				13:00 - <Tournament PvP> Category low start in 60m.
					...after 10m again:
				13:10 - <Tournament PvP> Category low start in 50m.
					...after 10m again:
				13:20 - <Tournament PvP> Category low start in 40m.
			* You can set other timer from tournament.h -> TOURNAMENT_PASSER_NOTICE = 10
			* Recommended as between 10 minutes default for not make so much spam chat.
		*/
	};
	
	for (unsigned int keyIndex = 1; keyIndex < TOURNAMENT_MAX_CATEGORY + 1; keyIndex ++)
	{
		if (vKey.tm_hour == TOURNAMENT_INFO_TIMER[keyIndex - 1][0] - 1)
		{
			categoryIndex = keyIndex;
			break;
		}
	}

	if (categoryIndex != 0 && vKey.tm_hour == TOURNAMENT_INFO_TIMER[categoryIndex - 1][0] - 1)
	{
		for (unsigned int keyIndex = 0; keyIndex < _countof(c_pszArray); keyIndex ++)
		{
			if (vKey.tm_min == c_pszArray[keyIndex] && vKey.tm_sec == 0)
			{
				CTournamentPvP::instance().SendNoticeLine(LC_TRANSLATE("TOURNAMENT_ANNOUNCEMENT_NOT_STARTED_LINE_2"), CTournamentPvP::instance().ConvertTimeToString(TOURNAMENT_INFO_START, categoryIndex).c_str(), CTournamentPvP::instance().ConvertCategoryToString(categoryIndex).c_str());
				break;
			}
		}
	}
}

bool CTournamentPvP::AnalyzeTimer(int key, int categoryIndex)
{
	time_t cur_Time = time(NULL);
	struct tm vKey = *localtime(&cur_Time);

	if (!CTournamentPvP::instance().GetIsPermission_Day())
		return false;

	switch (key)
	{
		case TOURNAMENT_CAN_START:
			return (vKey.tm_hour == TOURNAMENT_INFO_TIMER[categoryIndex - 1][0] && vKey.tm_min == TOURNAMENT_INFO_TIMER[categoryIndex - 1][1] && vKey.tm_sec == 0);

		case TOURNAMENT_PROCESSING:
			return (vKey.tm_hour == TOURNAMENT_INFO_TIMER[categoryIndex - 1][2] && vKey.tm_min == TOURNAMENT_INFO_TIMER[categoryIndex - 1][3] && vKey.tm_sec == 0);
	}
}

struct FRefreshWindow
{
	void operator() (LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = static_cast<LPCHARACTER>(ent);
			if (ch->IsPC() && !ch->IsObserverMode())
			{
				TPacketGCTournamentAdd p;
				p.header = HEADER_GC_TOURNAMENT_ADD;
				p.membersOnline_A = CTournamentPvP::instance().GetMembersTeamA();
				p.membersOnline_B = CTournamentPvP::instance().GetMembersTeamB();
				p.membersDead_A = (CTournamentPvP::instance().GetMembersTeamA() == TOURNAMENT_MAX_PLAYERS / 2) ? 0 : TOURNAMENT_MAX_PLAYERS / 2 - CTournamentPvP::instance().GetMembersTeamA();
				p.membersDead_B = (CTournamentPvP::instance().GetMembersTeamB() == TOURNAMENT_MAX_PLAYERS / 2) ? 0 : TOURNAMENT_MAX_PLAYERS / 2 - CTournamentPvP::instance().GetMembersTeamB();
				p.memberLives = CTournamentPvP::instance().GetMyLives(ch);
				p.dwTimeRemained = CTournamentPvP::instance().GetCurrentTimer();
				ch->GetDesc()->Packet(&p, sizeof(TPacketGCTournamentAdd));
			}
		}
	}
};

struct FWarpToHome
{
	void operator() (LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = static_cast<LPCHARACTER>(ent);

			if (ch->IsPC())
				ch->GoHome();
		}
	}
};

EVENTINFO(TournamentPvPInfoData)
{
	CTournamentPvP *pTournament;

	TournamentPvPInfoData()
	: pTournament(0)
	{
	}
};

bool CTournamentPvP::IsTournamentMap(LPCHARACTER ch, int key)
{
	if (ch->GetMapIndex() == TOURNAMENT_MAP_INDEX)
	{
		switch(key)
		{
			case TOURNAMENT_BLOCK_DUEL:
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("TOURNAMENT_MEMBER_BLOCK_DUEL")); 
				return true;

			case TOURNAMENT_BLOCK_PARTY:
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("TOURNAMENT_MEMBER_BLOCK_PARTY"));
				return true;

			case TOURNAMENT_BLOCK_RING_MARRIAGE:
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("TOURNAMENT_MEMBER_BLOCK_RING_MARRIAGE"));
				return true;

			case TOURNAMENT_BLOCK_POLY:
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("TOURNAMENT_MEMBER_BLOCK_POLY")); 
				return true;

			case TOURNAMENT_BLOCK_CHANGE_PKMODE:
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("TOURNAMENT_MEMBER_BLOCK_CHANGE_PKMODE"));
				return true;

			case TOURNAMENT_BLOCK_MOUNT:
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("TOURNAMENT_MEMBER_BLOCK_MOUNT"));
				return true;			
			
			case TOURNAMENT_BLOCK_HORSE:
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("TOURNAMENT_MEMBER_BLOCK_HORSE"));
				return true;

			case TOURNAMENT_BLOCK_EXIT_OBSERVER_MODE:
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("TOURNAMENT_MEMBER_BLOCK_EXIT_OBSERVER_MODE_LINE_1"));		
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("TOURNAMENT_MEMBER_BLOCK_EXIT_OBSERVER_MODE_LINE_2"));
				return true;

			default:
				return false;
		}
	}
}

int CTournamentPvP::GetDurationMinutes(int key)
{
	return TOURNAMENT_INFO_TIMER[key - 1][3] - TOURNAMENT_INFO_TIMER[key - 1][1];
}

std::string CTournamentPvP::ConvertTimeToString(int typeInfo, int key)
{
	switch (typeInfo)
	{
		case TOURNAMENT_INFO_START:
			return str(boost::format("%02i:%02i") % TOURNAMENT_INFO_TIMER[key - 1][0] % TOURNAMENT_INFO_TIMER[key - 1][1]);	

		case TOURNAMENT_INFO_FINISH:
			return str(boost::format("%02i:%02i") % TOURNAMENT_INFO_TIMER[key - 1][2] % TOURNAMENT_INFO_TIMER[key - 1][3]);
	}
}

std::string CTournamentPvP::ConvertCategoryToString(int categoryIndex)
{
	switch (categoryIndex)
	{
		case TOURNAMENT_CATEGORY_LOW:
			return LC_TRANSLATE("TOURNAMENT_CATEGORY_LOW");
		case TOURNAMENT_CATEGORY_MEDIUM:
			return LC_TRANSLATE("TOURNAMENT_CATEGORY_MEDIUM");
		case TOURNAMENT_CATEGORY_HARD:
			return LC_TRANSLATE("TOURNAMENT_CATEGORY_HARD");		
	}
}

std::string CTournamentPvP::ConvertTeamToString(DWORD idxTeam)
{
	switch (idxTeam)
	{
		case TEAM_MEMBERS_A:
			return LC_TRANSLATE("TOURNAMENT_TEAM_MEMBER_RED");
		case TEAM_MEMBERS_B:
			return LC_TRANSLATE("TOURNAMENT_TEAM_MEMBER_BLUE");
	}
}

std::map<DWORD, DWORD> CTournamentPvP::ConvertCategoryToMap(int categoryIndex)
{
    switch(categoryIndex)
    {
        case TOURNAMENT_CATEGORY_LOW:
            return m_map_category_low;
        case TOURNAMENT_CATEGORY_MEDIUM:
            return m_map_category_medium;
        case TOURNAMENT_CATEGORY_HARD:
            return m_map_category_hard;
    }
}

int CTournamentPvP::GetAttackMode(int indexTeam)
{
	switch (indexTeam)
	{
		case TEAM_MEMBERS_A:
			return PK_MODE_TEAM_A;
		case TEAM_MEMBERS_B:
			return PK_MODE_TEAM_B;
	}
}

int CTournamentPvP::ResizeCategoryIndex(int indexType)
{
	switch (indexType)
	{
		case TOURNAMENT_CATEGORY_LOW:
			return TOURNAMENT_CATEGORY_MEDIUM;
		case TOURNAMENT_CATEGORY_MEDIUM:
			return TOURNAMENT_CATEGORY_HARD;
		case TOURNAMENT_CATEGORY_HARD:
			return TOURNAMENT_CATEGORY_LOW;			
	}
}

void RefreshWindow()
{
	LPSECTREE_MAP pSectreeMap = SECTREE_MANAGER::instance().GetMap(TOURNAMENT_MAP_INDEX);
	if (pSectreeMap != NULL)
	{
		FRefreshWindow f;
		pSectreeMap->for_each(f);
	}
}

void CTournamentPvP::LoadingState(int indexType, int key)
{
	switch (indexType)
	{
		case TOURNAMENT_STATE_STARTED: 
		{
			CTournamentPvP::instance().SetCurrentTime(CTournamentPvP::instance().GetDurationMinutes(key) * 60);
			CTournamentPvP::instance().SendNoticeLine(LC_TRANSLATE("TOURNAMENT_ANNOUNCEMENT_START_LINE_1"), CTournamentPvP::instance().ConvertCategoryToString(key).c_str());
			CTournamentPvP::instance().SendNoticeLine(LC_TRANSLATE("TOURNAMENT_ANNOUNCEMENT_START_LINE_2"), CTournamentPvP::instance().GetDurationMinutes(key));
			CTournamentPvP::instance().SendNoticeLine(LC_TRANSLATE("TOURNAMENT_ANNOUNCEMENT_START_LINE_3"));
			CTournamentPvP::instance().TransferByCategory(key);
			RefreshWindow();
		}
		break;
			
		case TOURNAMENT_STATE_NOT_STARTED:
		{
			CTournamentPvP::instance().SendNoticeLine(LC_TRANSLATE("TOURNAMENT_ANNOUNCEMENT_NOT_STARTED_LINE_1"), CTournamentPvP::instance().ConvertCategoryToString(key).c_str());
			CTournamentPvP::instance().SendNoticeLine(LC_TRANSLATE("TOURNAMENT_ANNOUNCEMENT_NOT_STARTED_LINE_2"), CTournamentPvP::instance().ConvertTimeToString(TOURNAMENT_INFO_START, CTournamentPvP::instance().ResizeCategoryIndex(key)).c_str(), CTournamentPvP::instance().ConvertCategoryToString(CTournamentPvP::instance().ResizeCategoryIndex(key)).c_str());
		}
		break;
			
		case TOURNAMENT_STATE_FINISHED:
		{
			CTournamentPvP::instance().SendNoticeLine(LC_TRANSLATE("TOURNAMENT_ANNOUNCEMENT_FINISHED_LINE_1"), CTournamentPvP::instance().ConvertCategoryToString(key).c_str());
			CTournamentPvP::instance().SendNoticeLine(LC_TRANSLATE("TOURNAMENT_ANNOUNCEMENT_FINISHED_LINE_2"), CTournamentPvP::instance().ConvertTimeToString(TOURNAMENT_INFO_START, CTournamentPvP::instance().ResizeCategoryIndex(key)).c_str(), CTournamentPvP::instance().ConvertCategoryToString(CTournamentPvP::instance().ResizeCategoryIndex(key)).c_str());	
			CTournamentPvP::instance().GetTeamWinner();
		}
		break;			
	}
}

EVENTFUNC(tournament_timer)
{
	if (event == NULL)
		return 0;

	if (event->info == NULL)
		return 0;

	TournamentPvPInfoData* info = dynamic_cast<TournamentPvPInfoData*>(event->info);

	if (info == NULL)
		return 0;
	
	CTournamentPvP* pInstance = info->pTournament;

	if (pInstance == NULL)
		return 0;
	
	static int vKey = 0;
	
	for (int categoryIndex = 1; categoryIndex < TOURNAMENT_MAX_CATEGORY + 1; categoryIndex ++)
	{
		if (CTournamentPvP::instance().AnalyzeTimer(TOURNAMENT_CAN_START, categoryIndex) == true)
		{
			if (vKey == TOURNAMENT_CAN_START)
			{
				if (pInstance->CheckingStart(pInstance->ConvertCategoryToMap(categoryIndex)) == true)
				{
					pInstance->LoadingState(TOURNAMENT_STATE_STARTED, categoryIndex);
					vKey++;
					pInstance->WriteStatus(1);
					return PASSES_PER_SEC(1);	
				}
				else if (pInstance->CheckingStart(pInstance->ConvertCategoryToMap(categoryIndex)) == false)
				{
					pInstance->LoadingState(TOURNAMENT_STATE_NOT_STARTED, categoryIndex);
					vKey = TOURNAMENT_CAN_START;
					return PASSES_PER_SEC(1);
				}
			}
		}
			
		else if (CTournamentPvP::instance().AnalyzeTimer(TOURNAMENT_PROCESSING, categoryIndex) == true)
		{
			if (vKey == TOURNAMENT_PROCESSING)
			{
				pInstance->LoadingState(TOURNAMENT_STATE_FINISHED, categoryIndex);
				vKey = TOURNAMENT_CAN_START;
				pInstance->WriteStatus(0);
				return PASSES_PER_SEC(1);
			}
		}
	}

	CTournamentPvP::instance().PrepareAnnouncement();
	return PASSES_PER_SEC(1);
}

bool CTournamentPvP::Initialize()
{
	ReadFileItems();
	
	if (running_event != NULL)
	{
		event_cancel(&running_event);
		running_event = NULL;
	}
	
	ClearRegisters();
	ClearSTDMap();
	WriteStatus(0);
	SetCurrentTime(0);
	
	TournamentPvPInfoData* info = AllocEventInfo<TournamentPvPInfoData>();
	info->pTournament = this;

	running_event = event_create(tournament_timer, info, PASSES_PER_SEC(30));
	return true;
}

void CTournamentPvP::Destroy()
{
	ClearSTDMap();
	ClearRegisters();
	WriteStatus(0);
	SetCurrentTime(0);
	
	if (running_event != NULL)
	{
		event_cancel(&running_event);
		running_event = NULL;
	}
}

void CTournamentPvP::ClearSTDMap()
{
	m_map_team_a.clear();
	m_map_team_b.clear();
	m_map_lives.clear();
}

void CTournamentPvP::ClearRegisters()
{
	m_map_category_low.clear();
	m_map_category_medium.clear();
	m_map_category_hard.clear();	
}

bool CTournamentPvP::CanUseItem(LPCHARACTER ch, LPITEM item)
{
	if (!ch || !item)
		return false;

	switch (item->GetVnum())
	{
#ifdef ENABLE_EXTRA_LIVES_TOURNAMENT	
		case TOURNAMENT_ITEM_EXTRA_LIVES:
		{
			if (ch->GetMapIndex() == TOURNAMENT_MAP_INDEX)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("TOURNAMENT_MEMBER_ADD_EXTRA_LIVES_ERROR_LINE_1"));
				return false;
			}
			
			if (CTournamentPvP::instance().GetExistExtraLives(ch))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("TOURNAMENT_MEMBER_ADD_EXTRA_LIVES_ERROR_LINE_2"));
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("TOURNAMENT_MEMBER_ADD_EXTRA_LIVES_ERROR_LINE_3"), (TOURNAMENT_CAN_USED_MAX_EXTRA_LIVES - CTournamentPvP::instance().GetUsedCountExtraLives(ch)), TOURNAMENT_CAN_USED_MAX_EXTRA_LIVES);
				return false;
			}
			
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("TOURNAMENT_MEMBER_ADD_EXTRA_LIVES_SUCCES"), (TOURNAMENT_EXTRA_LIVES - TOURNAMENT_MAX_LIVES), TOURNAMENT_CAN_USED_MAX_EXTRA_LIVES);
			ch->SetQuestFlag(FLAG_EXTRA_LIVES, 1);
			ch->RemoveSpecifyItem(TOURNAMENT_ITEM_BOX, 1);
		}
		break;
#endif

		case TOURNAMENT_ITEM_BOX:
		{
			const DWORD MAX_LIST_REWARD = 8; /* Change here how many rows have m_dwListRewards. */
			DWORD m_dwListRewards[MAX_LIST_REWARD][2] = /* Configurable */
			{
				{	80014,	1	},
				{	14209,	1	},
				{	16209,	1	},
				{	17109,	1	},
				{	149,	1	},
				{	159,	1	},
				{	20259,	1	},
				{	299,	1	}
			};
			
			if (ch->CountEmptyInventory() < MAX_LIST_REWARD)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("TOURNAMENT_MEMBER_OPEN_REWARD_WRONG_SIZE"), MAX_LIST_REWARD);
				return false;
			}

			for (int i=0; i< _countof(m_dwListRewards); i++)
			{
				ch->AutoGiveItem(m_dwListRewards[i][0], m_dwListRewards[i][1]);
			}
			ch->RemoveSpecifyItem(TOURNAMENT_ITEM_BOX, 1);
		}
		break;
		default:
			return true;
	}
}

void CTournamentPvP::GiveReward(LPCHARACTER ch)
{
	ch->AutoGiveItem(TOURNAMENT_ITEM_BOX, 1);
}

void CTournamentPvP::SendNoticeLine(const char * format, ...)
{
	if (!format)
		return;

	char chatbuf[CHAT_MAX_LEN + 1];
	va_list args;

	va_start(args, format);
	vsnprintf(chatbuf, sizeof(chatbuf), format, args);
	va_end(args);

	/*TPacketGGNotice p;
	p.bHeader = HEADER_GG_NOTICE;
	p.lSize = strlen(chatbuf) + 1;

	TEMP_BUFFER buf;
	buf.write(&p, sizeof(p));
	buf.write(chatbuf, p.lSize);

	P2P_MANAGER::instance().Send(buf.read_peek(), buf.size());*/
	
	SendNotice(chatbuf);
}

#ifdef ENABLE_KILL_COUNTS_FOR_EACH_PLAYER	
void CTournamentPvP::InsertPlayerKillLogs(LPCHARACTER ch)
{
	int points = ch->GetQuestFlag(FLAG_KILL_COUNT);
	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT pid FROM %s WHERE pid = '%d'", MYSQL_DATABASE_RANKING, ch->GetPlayerID()));
	
	if (pMsg->Get()->uiNumRows > 0)
	{
		DBManager::instance().DirectQuery("UPDATE %s SET total_points = '%d' WHERE pid = '%d'", MYSQL_DATABASE_RANKING, points, ch->GetPlayerID());
	}
	else
	{
		DBManager::Instance().DirectQuery("INSERT INTO %s (pid, name, total_points) VALUES('%d', '%s', '%d')", MYSQL_DATABASE_RANKING, ch->GetPlayerID(), ch->GetName(), points);	
	}
}
#endif

void CTournamentPvP::GetTeamWinner()
{
	if (CTournamentPvP::instance().GetMembersTeamA() == CTournamentPvP::instance().GetMembersTeamB())
	{
		SendNoticeLine(LC_TRANSLATE("TOURNAMENT_ANNOUNCEMENT_WINNER_SAME"));	
		DestroyAll();
		return;
	}
	
	if (CTournamentPvP::instance().GetMembersTeamA() == TOURNAMENT_NO_MEMBERS && CTournamentPvP::instance().GetMembersTeamB() == TOURNAMENT_NO_MEMBERS)
	{
		SendNoticeLine(LC_TRANSLATE("TOURNAMENT_ANNOUNCEMENT_WINNER_NOT_EXIST"));	
		DestroyAll();
		return;
	}	

	int idxTeamWinner = (CTournamentPvP::instance().GetMembersTeamA() > CTournamentPvP::instance().GetMembersTeamB()) ? TEAM_MEMBERS_A : TEAM_MEMBERS_B;
	int idxSize = (CTournamentPvP::instance().GetMembersTeamA() > CTournamentPvP::instance().GetMembersTeamB()) ? CTournamentPvP::instance().GetMembersTeamA() : CTournamentPvP::instance().GetMembersTeamB();
	
	switch (idxTeamWinner)
	{
		case TEAM_MEMBERS_A:
		{
			itertype(m_map_team_a) it = m_map_team_a.begin();
			LPCHARACTER ch = NULL;
			for (; it != m_map_team_a.end(); ++it)
			{
				ch = CHARACTER_MANAGER::instance().FindByPID(it->second);
				if (ch)
				{
					insert_winners(LC_TRANSLATE("TOURNAMENT_INSERT_LOG_WINNERS"), ch->GetName(), ch->GetLevel(), ch->GetDesc()->GetHostName(), CTournamentPvP::instance().GetMyLives(ch), CTournamentPvP::instance().ConvertTeamToString(idxTeamWinner).c_str());
					GiveReward(ch);
				}
			}
		}
		break;
		
		case TEAM_MEMBERS_B:
		{
			itertype(m_map_team_b) it = m_map_team_b.begin();
			LPCHARACTER ch = NULL;
			for (; it != m_map_team_b.end(); ++it)
			{
				ch = CHARACTER_MANAGER::instance().FindByPID(it->second);
				if (ch)
				{
					insert_winners(LC_TRANSLATE("TOURNAMENT_INSERT_LOG_WINNERS"), ch->GetName(), ch->GetLevel(), ch->GetDesc()->GetHostName(), CTournamentPvP::instance().GetMyLives(ch), CTournamentPvP::instance().ConvertTeamToString(idxTeamWinner).c_str());
					GiveReward(ch);
				}
			}
		}
		break;
	}

	SendNoticeLine(LC_TRANSLATE("TOURNAMENT_ANNOUNCEMENT_WINNER_SUCCES"), CTournamentPvP::instance().ConvertTeamToString(idxTeamWinner).c_str(), idxSize);
	DestroyAll();
}

bool CTournamentPvP::IsLimitedItem(LPCHARACTER ch, DWORD dwVnum)
{
	if (m_listForbidden.empty())
		return false;
	
	if (ch->GetMapIndex() != TOURNAMENT_MAP_INDEX)
		return false;
	
	if (std::find(m_listForbidden.begin(), m_listForbidden.end(), dwVnum) != m_listForbidden.end())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("TOURNAMENT_MEMBER_CANNOT_USE_ITEM"), ITEM_MANAGER::instance().GetTable(dwVnum)->szLocaleName);
		return true;
	}
	
	return false;
}

bool CTournamentPvP::IsRegister(LPCHARACTER ch, std::map<DWORD, DWORD> m_map_global)
{
	itertype(m_map_global) it = m_map_global.find(ch->GetPlayerID());
	return (it != m_map_global.end());
}

int CTournamentPvP::GetIndexTeam(LPCHARACTER ch)
{
	if (IsRegister(ch, m_map_team_a))
		return TEAM_MEMBERS_A;
		
	if (IsRegister(ch, m_map_team_b))
		return TEAM_MEMBERS_B;
		
	return 0;
}

void CTournamentPvP::Register(LPCHARACTER ch, DWORD dwCategory)
{
	switch (dwCategory)
	{
		case TOURNAMENT_CATEGORY_LOW:
			m_map_category_low.insert(std::make_pair(ch->GetPlayerID(), ch->GetPlayerID()));
			break;
		case TOURNAMENT_CATEGORY_MEDIUM:
			m_map_category_medium.insert(std::make_pair(ch->GetPlayerID(), ch->GetPlayerID()));
			break;
		case TOURNAMENT_CATEGORY_HARD:
			m_map_category_hard.insert(std::make_pair(ch->GetPlayerID(), ch->GetPlayerID()));
			break;
	}
}

void CTournamentPvP::Respawn(LPCHARACTER ch)
{
	int teamIndex = CTournamentPvP::instance().GetIndexTeam(ch);

	if (teamIndex != 0)
	{
		ch->Show(TOURNAMENT_MAP_INDEX, g_position_tournament[teamIndex - 1][0], g_position_tournament[teamIndex - 1][1]);
		ch->Stop();
	}
}

#ifdef ENABLE_EXTRA_LIVES_TOURNAMENT
bool CTournamentPvP::GetExistExtraLives(LPCHARACTER ch)
{
	return (ch->GetQuestFlag(FLAG_EXTRA_LIVES) > 0);
}

int CTournamentPvP::GetUsedCountExtraLives(LPCHARACTER ch)
{
	return ch->GetQuestFlag(FLAG_USED_COUNT_EXTRA_LIVES);
}

void CTournamentPvP::SetUsedCountExtraLives(LPCHARACTER ch, int val)
{
	ch->SetQuestFlag(FLAG_USED_COUNT_EXTRA_LIVES, val);
}
#endif

void CTournamentPvP::AppendLives(LPCHARACTER ch)
{
#ifdef ENABLE_EXTRA_LIVES_TOURNAMENT
	if (CTournamentPvP::instance().GetExistExtraLives(ch))
	{
		m_map_lives.insert(std::make_pair(ch->GetPlayerID(), TOURNAMENT_EXTRA_LIVES));
	}
	else
	{
		m_map_lives.insert(std::make_pair(ch->GetPlayerID(), TOURNAMENT_MAX_LIVES));
	}
#else
	m_map_lives.insert(std::make_pair(ch->GetPlayerID(), TOURNAMENT_MAX_LIVES));
#endif
}

void CTournamentPvP::DestroyAll()
{
	LPSECTREE_MAP sectree = SECTREE_MANAGER::instance().GetMap(TOURNAMENT_MAP_INDEX);

	if ( sectree != NULL )
	{
		struct FWarpToHome f;
		sectree->for_each( f );
	}
	
	ClearSTDMap();
}

void CTournamentPvP::OnDisconnect(LPCHARACTER ch)
{
	if (ch->GetMapIndex() == TOURNAMENT_MAP_INDEX)
	{
		m_map_lives.erase(ch->GetPlayerID());
		m_map_team_a.erase(ch->GetPlayerID());
		m_map_team_b.erase(ch->GetPlayerID());
		ch->GoHome();
		RefreshWindow();
	}
}

void CTournamentPvP::OnLogin(LPCHARACTER ch)
{
	if (ch->GetMapIndex() == TOURNAMENT_MAP_INDEX)
		ch->SetObserverMode(true);
}

bool CTournamentPvP::RemoveLives(LPCHARACTER pkDead)	
{
    itertype(m_map_lives) it = m_map_lives.find(pkDead->GetPlayerID());

	if (it == m_map_lives.end())
		return false;
	
	if (it->second == TOURNAMENT_LAST_LIFE)
	{
		pkDead->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("TOURNAMENT_MEMBER_FINISHED_LIFE_LINE_1"));
		pkDead->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("TOURNAMENT_MEMBER_FINISHED_LIFE_LINE_2"));	
		pkDead->GoHome();
		return false;
	}

	it->second -= 1;
	m_map_lives[pkDead->GetPlayerID()] = it->second;
	RefreshWindow();
	return true;
}

void CTournamentPvP::OnKill(LPCHARACTER pkKiller, LPCHARACTER pkDead)
{
	if (!pkKiller->IsPC())
		return;
	
	if (!pkDead->IsPC())
		return;
		
	if (pkKiller->GetMapIndex() != TOURNAMENT_MAP_INDEX)
		return;
	
	if (pkKiller->GetPKMode() == pkDead->GetPKMode())
		return;

#ifdef ENABLE_KILL_COUNTS_FOR_EACH_PLAYER	
	int KillCount = pkKiller->GetQuestFlag(FLAG_KILL_COUNT);
	KillCount += 1;
	pkKiller->SetQuestFlag(FLAG_KILL_COUNT, KillCount);

	CTournamentPvP::instance().InsertPlayerKillLogs(pkKiller);
#endif

	if (CTournamentPvP::instance().RemoveLives(pkDead))
	{
		pkDead->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("TOURNAMENT_MEMBER_REMAINING_LIFE"), m_map_lives[pkDead->GetPlayerID()]);
		CTournamentPvP::instance().Respawn(pkDead);
	}
}

void CTournamentPvP::PushBack(std::map<DWORD, DWORD> m_map_global, std::vector<DWORD>* m_vec)
{
	itertype(m_map_global) iter = m_map_global.begin();
	
	for (; iter != m_map_global.end(); ++iter)
	{
		m_vec->push_back(iter->second);
	}
}

bool CTournamentPvP::CheckingStart(std::map<DWORD, DWORD> m_map_global)
{
	itertype(m_map_global) it = m_map_global.begin();
	LPCHARACTER ch = NULL;
	int iterator = 0;
	
	for (; it != m_map_global.end(); ++it)
	{
		ch = CHARACTER_MANAGER::instance().FindByPID(it->second);
		
		if (ch != NULL && ch->GetMapIndex() == TOURNAMENT_MAP_INDEX)
			iterator++;
	}
	return (iterator >= TOURNAMENT_MAX_PLAYERS) ? true : false;
}

bool CTournamentPvP::TransferByCategory(DWORD index)
{
	std::vector<DWORD> m_vec_character, m_vec_cache;
	DWORD dwPID;
	LPCHARACTER ch = NULL;
	
	ClearSTDMap();
	
	switch (index)
	{
		case TOURNAMENT_CATEGORY_LOW:
			CTournamentPvP::instance().PushBack(m_map_category_low, &m_vec_character);
			break;
		case TOURNAMENT_CATEGORY_MEDIUM:
			CTournamentPvP::instance().PushBack(m_map_category_medium, &m_vec_character);
			break;
		case TOURNAMENT_CATEGORY_HARD:
			CTournamentPvP::instance().PushBack(m_map_category_hard, &m_vec_character);
			break;
	}

	srand(time(0));

	while (CTournamentPvP::instance().GetMembersTeamA() < TOURNAMENT_MAX_PLAYERS / 2)
	{
		dwPID = m_vec_character[rand() % m_vec_character.size()];
		while (std::find(m_vec_cache.begin(), m_vec_cache.end(), dwPID) != m_vec_cache.end())
		{
			dwPID = m_vec_character[rand() % m_vec_character.size()];
		}
		
		ch = CHARACTER_MANAGER::instance().FindByPID(dwPID);

		if (ch != NULL)
		{
			m_map_team_a.insert(std::make_pair(dwPID, dwPID));
			m_vec_cache.push_back(dwPID);
			
			switch (index)
			{
				case TOURNAMENT_CATEGORY_LOW:
					m_map_category_low.erase(dwPID);
					break;
				case TOURNAMENT_CATEGORY_MEDIUM:
					m_map_category_medium.erase(dwPID);
					break;
				case TOURNAMENT_CATEGORY_HARD:
					m_map_category_hard.erase(dwPID);
					break;
			}
		}
	}
	
	srand(time(0));
	
	while (CTournamentPvP::instance().GetMembersTeamB() < TOURNAMENT_MAX_PLAYERS / 2)
	{
		dwPID = m_vec_character[ rand() % m_vec_character.size()];
		while (std::find(m_vec_cache.begin(), m_vec_cache.end(), dwPID) != m_vec_cache.end())
		{
			dwPID = m_vec_character[rand() % m_vec_character.size()];
		}

		ch = CHARACTER_MANAGER::instance().FindByPID(dwPID);

		if (ch != NULL)
		{
			m_map_team_b.insert(std::make_pair(dwPID, dwPID));
			m_vec_cache.push_back(dwPID);
			
			switch (index)
			{
				case TOURNAMENT_CATEGORY_LOW:
					m_map_category_low.erase(dwPID);
					break;
				case TOURNAMENT_CATEGORY_MEDIUM:
					m_map_category_medium.erase(dwPID);
					break;
				case TOURNAMENT_CATEGORY_HARD:
					m_map_category_hard.erase(dwPID);
					break;
			}
		}
	}

	CTournamentPvP::instance().TeleportMembers(m_map_team_a, TEAM_MEMBERS_A);
	CTournamentPvP::instance().TeleportMembers(m_map_team_b, TEAM_MEMBERS_B);
}

void CTournamentPvP::TeleportMembers(std::map<DWORD, DWORD> m_map_global, DWORD index)
{
	itertype(m_map_global) it = m_map_global.begin();
	LPCHARACTER ch = NULL;

	for (; it != m_map_global.end(); ++it)
	{
		ch = CHARACTER_MANAGER::instance().FindByPID(it->second);

		if (ch != NULL)
		{
			if (ch->IsObserverMode())
			{
				ch->SetObserverMode(false);
			}
			
			CTournamentPvP::instance().AppendLives(ch);
			ch->Show(TOURNAMENT_MAP_INDEX, g_position_tournament[index - 1][0], g_position_tournament[index - 1][1]);
			ch->Stop();
			ch->SetPKMode(CTournamentPvP::instance().GetAttackMode(index));
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("TOURNAMENT_MEMBER_DIVIDED"), CTournamentPvP::instance().ConvertTeamToString(index).c_str());

#ifdef ENABLE_EXTRA_LIVES_TOURNAMENT			
			int usedCount = CTournamentPvP::instance().GetUsedCountExtraLives(ch);

			if (usedCount == TOURNAMENT_CAN_USED_MAX_EXTRA_LIVES - 1)
			{
				CTournamentPvP::instance().SetUsedCountExtraLives(ch, 0);
				ch->SetQuestFlag(FLAG_EXTRA_LIVES, 0);
			}
			else
			{
				CTournamentPvP::instance().SetUsedCountExtraLives(ch, usedCount + 1);
			}
#endif
		}
	}
}

void CTournamentPvP::Warp(LPCHARACTER ch)
{
	int random = number(0, 2);
	ch->WarpSet(g_observers_position_tournament[random][0], g_observers_position_tournament[random][1]);
}

int CTournamentPvP::GetParticipantsLow()
{
	return m_map_category_low.size();
}	
int CTournamentPvP::GetParticipantsMedium()
{
	return m_map_category_medium.size();
}	
int CTournamentPvP::GetParticipantsHard()
{
	return m_map_category_hard.size();
}

namespace quest
{
	int tournament_is_map(lua_State* L)
	{
		LPCHARACTER pkChar = CQuestManager::instance().GetCurrentCharacterPtr();
		if (pkChar && pkChar->GetMapIndex() == TOURNAMENT_MAP_INDEX)
			lua_pushboolean(L, true);
		else
			lua_pushboolean(L, false);
		return 1;
	}
	
	int tournament_register(lua_State* L)
	{
		LPCHARACTER pkChar = CQuestManager::instance().GetCurrentCharacterPtr();
		
		DWORD teamIdx = (DWORD)lua_tonumber(L, 1);
		CTournamentPvP::instance().Register(pkChar, teamIdx);
		return 1;
	}
	
	int tournament_warp(lua_State* L)
	{
		LPCHARACTER pkChar = CQuestManager::instance().GetCurrentCharacterPtr();
		if (pkChar)
			CTournamentPvP::instance().Warp(pkChar);
		return 1;
	}
	
	int tournament_delete_register(lua_State* L)
	{
		LPCHARACTER pkChar = CQuestManager::instance().GetCurrentCharacterPtr();
		if (pkChar)
		{
			if (CTournamentPvP::instance().IsRegister(pkChar, m_map_category_low))
				m_map_category_low.erase(pkChar->GetPlayerID());
			
			else if (CTournamentPvP::instance().IsRegister(pkChar, m_map_category_medium))
				m_map_category_medium.erase(pkChar->GetPlayerID());			
			
			else if (CTournamentPvP::instance().IsRegister(pkChar, m_map_category_hard))
				m_map_category_hard.erase(pkChar->GetPlayerID());			
		}
		return 1;
	}
	
	int tournament_info_timer(lua_State* L)
	{
		int typeInfo = (int)lua_tonumber(L, 1);
		int typeCat = (int)lua_tonumber(L, 2);
		
		lua_pushstring(L, CTournamentPvP::instance().ConvertTimeToString(typeInfo, typeCat).c_str());
		return 1;
	}
	
	int tournament_is_running(lua_State* L)
	{
		if (CTournamentPvP::instance().GetStatus() == TOURNAMENT_STARTED)
		{
			lua_pushboolean(L, true);
		}
		else {
			lua_pushboolean(L, false);
		}
		return 1;
	}	
	
	int tournament_observer(lua_State* L)
	{
		LPCHARACTER pkChar = CQuestManager::instance().GetCurrentCharacterPtr();
		pkChar->SetQuestFlag(FLAG_OBSERVER, 1);
		CTournamentPvP::instance().Warp(pkChar);
		return 1;
	}	
	
	int tournament_get_participants(lua_State* L)
	{
		int categoryIndex = (int)lua_tonumber(L, 1);
		
		int m_counter[TOURNAMENT_MAX_CATEGORY] =
		{
			CTournamentPvP::instance().GetParticipantsLow(),
			CTournamentPvP::instance().GetParticipantsMedium(),
			CTournamentPvP::instance().GetParticipantsHard()
		};
		
		lua_pushnumber(L, m_counter[categoryIndex - 1]);
		return 1;
	}
	
	int tournament_get_is_register(lua_State* L)
	{
		LPCHARACTER pkChar = CQuestManager::instance().GetCurrentCharacterPtr();
		
		if (CTournamentPvP::instance().IsRegister(pkChar, m_map_category_low) || CTournamentPvP::instance().IsRegister(pkChar, m_map_category_medium) || CTournamentPvP::instance().IsRegister(pkChar, m_map_category_hard)) {
			lua_pushboolean(L, true);
		}
		else {
			lua_pushboolean(L, false);
		}
		return 1;
	}
	
	int tournament_info_current_timer(lua_State* L)
	{
		time_t currentTime;
		struct tm *localTime;
		time(&currentTime);
		localTime = localtime(&currentTime);
		lua_pushstring(L, asctime(localTime));
		return 1;
	}

	void RegisterTournamentPvPFunctionTable()
	{
		luaL_reg tournament_functions[] =
		{
			{	"get_is_register",	tournament_get_is_register	},
			{	"participants",		tournament_get_participants	},
			{	"is_map",			tournament_is_map			},
			{	"register",			tournament_register			},
			{	"warp",				tournament_warp				},
			{	"is_running",		tournament_is_running		},
			{	"observer",			tournament_observer			},
			{	"info_timer",		tournament_info_timer		},
			{	"info_current_timer", tournament_info_current_timer },
			{	"delete_register",	tournament_delete_register	},
			{	NULL,		NULL					}
		};
		
		CQuestManager::instance().AddLuaFunctionTable("tournament", tournament_functions);
	}
}