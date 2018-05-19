/*********************************************************************
* date        : 2016.11.15
* file        : ticket.cpp
* author      : VegaS
* version	  : 0.0.5
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
#include <sstream>
#include "ticket.h"
#include <string>
#include <algorithm>
#include <boost/algorithm/string/replace.hpp>

using namespace std;
using namespace boost;
using namespace algorithm;

#define WAIT_TIMER_FOR_OPEN		10		// You can open ticket window once every 10 second.
#define WAIT_TIMER_FOR_CREATE	1*60*60	// You can create a ticket once every 1 hour.
#define WAIT_TIMER_FOR_REPLY	10*60	// You can reply to a ticket once every 10 minutes.
		
CTicketSystem::CTicketSystem()
{}
CTicketSystem::~CTicketSystem()
{}

const char* LC_TRANSLATE(const char* front_inserter)
{
	typedef map<const char *, const char *> TMapTranslate;
	TMapTranslate LC_TRANSLATE;

	LC_TRANSLATE["USER_ERROR_ACCOUNT_IS_BANNED"]				= "Sistemi raporlamak icin kullanamazsiniz. Engelleme nedeni: %s";
	LC_TRANSLATE["USER_ERROR_LAST_TIME"]						= "Beklemelisin [%d gun] [%d saat] [%d dakika] [%d saniye].";
	LC_TRANSLATE["USER_ERROR_OWNER"]							= "Bunu yapamazsin.";
	LC_TRANSLATE["USER_ERROR_TICKET_ID"]						= "Rapor: %s, yok, onlara cevap veremezsiniz.";
	LC_TRANSLATE["USER_ERROR_STATUS_NOT_OPENED"]				= "Rapor: %s kapali!";
	
	LC_TRANSLATE["STAFF_ERROR_OPEN_SORT"]						= "Bilinmeyen hata.";
	LC_TRANSLATE["STAFF_ERROR_LOGS_ARGUMENTS_LOW"]				= "B³¹d! Gönderilen paket 0'dan küçük.";
	LC_TRANSLATE["STAFF_ERROR_LOGS_ARGUMENTS_HIGH"]				= "B³¹d! Paketin gönderilmesi ile ilgili sorun. %d | %d, maksimum: %d";
	
	LC_TRANSLATE["STAFF_ERROR_LOGS_LOADED_MYSQL"]				= "Rapor gonderemezsiniz.";	
	LC_TRANSLATE["STAFF_ERROR_CREATE"]							= "Bunu yapamazsin.";
	LC_TRANSLATE["STAFF_ERROR_BAN_WITH_REASON"]					= "Oyuncuyu engelleme sebebini belirtmelisiniz.";	
	LC_TRANSLATE["STAFF_ERROR_SEARCH_USER"]						= "Oyuncu bulunamadi.";
	LC_TRANSLATE["STAFF_ERROR_CANNOT_BAN_SELF"]					= "Kendini engelleyemezsin.";
	LC_TRANSLATE["STAFF_ERROR_USER_ALREADY_IS_BANNED"]			= "Oyuncu zaten engellendi.";
	LC_TRANSLATE["STAFF_ERROR_CANNOT_UNBAN_SELF"]				= "Kendinin kilidini açamazsin.";	
	LC_TRANSLATE["STAFF_ERROR_USER_IS_NOT_BANNED"]				= "Bu hesap engellenmiyor!";		
	LC_TRANSLATE["STAFF_ERROR_CHANGE_STATUS_NOID"]				= "Rapor: %s, yok, durumu degistiremezsin.";		
	LC_TRANSLATE["STAFF_ERROR_CHANGE_STATUS_ALREADY_IS_OPENED"]	= "Rapor: %s, zaten acik";	
	LC_TRANSLATE["STAFF_ERROR_CHANGE_STATUS_ALREADY_IS_CLOSED"]	= "Rapor: %s, zaten kapali.";

	LC_TRANSLATE["STAFF_SUCCES_UNBANNED_USER"]					= "Gracz %s kilidi acildi.";		
	LC_TRANSLATE["STAFF_SUCCES_BANNED_USER"]					= "Gracz %s o engellendi. Engelleme nedeni: %s.";		
	LC_TRANSLATE["STAFF_SUCCES_CHANGE_STATUS_TO_OPEN"]			= "Rapor: %s acildi.";
	LC_TRANSLATE["STAFF_SUCCES_CHANGE_STATUS_TO_CLOSED"]		= "Rapor: %s kapaliydi.";

	return LC_TRANSLATE[front_inserter];
}

static char ar_Elements[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
int ar_SizeElements = sizeof(ar_Elements) - 1;

char CTicketRandom()
{
    return ar_Elements[rand() % ar_SizeElements];
}

bool CTicketSystem::IsDenied(LPCHARACTER ch, char* argument)
{
    static char dwListDenied[] = {
		// '?',
		// '*',
		//'$',
		//'!',
		// '/',
		//'>',
		//'<',
		// '|',
		// ';',
		//':',
		//'}',
		//'{',
		//'[',
		//']',
		'%',
		'#',
		'@',
		'^',
		'&',
		'DROP',
		'drop',
		// '"',
		// '\'',
		// '\\',
		'TRUNCATE'
	};
	
	std::string strLine = argument;
	
	for (int i = 0; i < _countof(dwListDenied); i++)
	{
		if (strLine.find(dwListDenied[i]) != std::string::npos)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "Mesajda ozel karakterler kullanamazsiniz.");
			return true;
        }
	}

	return false;
}

bool CTicketSystem::GetIsMemberStaff(LPCHARACTER ch)
{
	if (ch->GetGMLevel() > GM_PLAYER)
		return true;

	return false;
}

int CTicketSystem::GetFilter(LPCHARACTER ch)
{
	int iResultIdx = ch->GetQuestFlag(RECV_MODE_SORT);
	return (iResultIdx);
}

bool CTicketSystem::GetOwner(LPCHARACTER ch, char* strID)
{
	char szQuery[QUERY_MAX_LEN + 1];
	snprintf(szQuery, sizeof(szQuery), "SELECT ticked_id FROM ticket.list WHERE ticked_id = '%s' and char_name = '%s'", strID, ch->GetName());
	unique_ptr<SQLMsg> dwExtractID(DBManager::instance().DirectQuery(szQuery));

	return (dwExtractID->Get()->uiNumRows > 0 || (CTicketSystem::instance().GetIsMemberStaff(ch))) ? true : false;
}

bool CTicketSystem::GetIsOpened(char* strID)
{
	char szQuery[QUERY_MAX_LEN + 1];
	snprintf(szQuery, sizeof(szQuery), "SELECT status FROM ticket.list WHERE ticked_id = '%s'", strID);
	unique_ptr<SQLMsg> dwExtractStatus(DBManager::instance().DirectQuery(szQuery));
	MYSQL_ROW row = mysql_fetch_row(dwExtractStatus->Get()->pSQLResult);
	
	int result = 0;
	str_to_number(result, row[0]);
	
	return (result == STATUS_OPENED) ? true : false;
}

bool CTicketSystem::GetExistID(char* strID)
{
	char szQuery[QUERY_MAX_LEN + 1];
	snprintf(szQuery, sizeof(szQuery), "SELECT ticked_id FROM ticket.list WHERE ticked_id = '%s'", strID);
	unique_ptr<SQLMsg> dwExtractID(DBManager::instance().DirectQuery(szQuery));
	
	return (dwExtractID->Get()->uiNumRows > 0) ? true : false;
}

bool CTicketSystem::GetAccountBanned(LPCHARACTER ch)
{
	char szQuery[QUERY_MAX_LEN + 1];
	snprintf(szQuery, sizeof(szQuery), "SELECT account_id,reason FROM ticket.user_restricted WHERE account_id = '%d'", ch->GetDesc()->GetAccountTable().id);
	unique_ptr<SQLMsg> dwExtractID(DBManager::instance().DirectQuery(szQuery));
	MYSQL_ROW row = mysql_fetch_row(dwExtractID->Get()->pSQLResult);
	
	if (dwExtractID->Get()->uiNumRows > 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("USER_ERROR_ACCOUNT_IS_BANNED"), row[1]);
		return true;
	}

	return false;
}

bool CTicketSystem::GetLastTimeUsed(LPCHARACTER ch, int iGetMode)
{
	string strGetFlag;
	ostringstream str;

	str << RECV_MODE_TIME << iGetMode;
	strGetFlag = str.str();
	
	int vecTime = ch->GetQuestFlag(strGetFlag);

	if (vecTime && get_global_time() < vecTime)
	{	
		int total = vecTime - get_global_time();
		int sec = total % 60;
			total /= 60;
		int mins = total % 60;
			total /= 60;
		int hours = total % 24;
		int days = total / 24;
		
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("USER_ERROR_LAST_TIME"), days, hours, mins, sec);
		return false;
	}

	return true;
}

void CTicketSystem::SetLastTimeUsed(LPCHARACTER ch, int iRegMode)
{
	int iDam = 0;
	for (int row = 1; row <= 3; row++)
	{
		if (row == iRegMode && !CTicketSystem::instance().GetIsMemberStaff(ch))
		{
			string strSaveFlag;
			ostringstream str;

			str << RECV_MODE_TIME << iRegMode;
			strSaveFlag = str.str();

			if (iRegMode == 1)
				iDam = WAIT_TIMER_FOR_OPEN;
			if (iRegMode == 2)
				iDam = WAIT_TIMER_FOR_CREATE;
			if (iRegMode == 3)
				iDam = WAIT_TIMER_FOR_REPLY;

			ch->SetQuestFlag(strSaveFlag, get_global_time() + iDam);
		}
	}
}

void TicketReplaceOld(string& line, string& oldString, string& newString)
{
	const size_t oldSize = oldString.length();

	if (oldSize > line.length())
		return;

	const size_t newSize = newString.length();
	
	for(size_t pos = 0; ; pos += newSize)
	{
		pos = line.find(oldString, pos);
		
		if (pos == string::npos)
			return;
		
		if (oldSize == newSize)
		{
			line.replace(pos, oldSize, newString);
		}
		else
		{
			line.erase(pos, oldSize);
			line.insert(pos, newString);
		}
	}
}

bool CTicketSystem::SendTicketLogs(LPCHARACTER ch, int dwModeLogs, const char* szTicketID, int iStartPage, int iSortMode)
{
	switch(dwModeLogs)
	{
		case LOGS_GENERAL:
		{
			TPacketTicketLogsData dwPacket;
			dwPacket.header = HEADER_GC_TICKET_LOGS;

			SQLMsg *dwExtractList = DBManager::instance().DirectQuery("SELECT * FROM ticket.list WHERE char_name = '%s' ORDER BY create_date DESC LIMIT %d", ch->GetName(), MAX_LOGS_GENERAL);
			MYSQL_ROW iCurRow;
			int idx = 0;
			
			if(dwExtractList->uiSQLErrno != 0)
				return false;

			while ((iCurRow = mysql_fetch_row(dwExtractList->Get()->pSQLResult)))
			{
				dwPacket.logs[idx] = TPacketTicketLogs();
				strncpy(dwPacket.logs[idx].ticked_id, iCurRow[1], sizeof(dwPacket.logs[idx].ticked_id));
				strncpy(dwPacket.logs[idx].title, iCurRow[3], sizeof(dwPacket.logs[idx].title));
				strncpy(dwPacket.logs[idx].content, iCurRow[4], sizeof(dwPacket.logs[idx].content));
				str_to_number(dwPacket.logs[idx].priority, iCurRow[5]);
				strncpy(dwPacket.logs[idx].create_date, iCurRow[6], sizeof(dwPacket.logs[idx].create_date) - 1);
				str_to_number(dwPacket.logs[idx].status, iCurRow[7]);
				idx++;
			}

			if(dwExtractList->Get()->uiNumRows < MAX_LOGS_GENERAL)
			{
				while (idx < MAX_LOGS_GENERAL)
				{
					dwPacket.logs[idx] = TPacketTicketLogs();
					strncpy(dwPacket.logs[idx].ticked_id, RECV_UNKNOWN, sizeof(dwPacket.logs[idx].ticked_id));
					strncpy(dwPacket.logs[idx].title, RECV_UNKNOWN, sizeof(dwPacket.logs[idx].title));
					strncpy(dwPacket.logs[idx].content, RECV_UNKNOWN, sizeof(dwPacket.logs[idx].content));			
					dwPacket.logs[idx].priority = 999;			
					strncpy(dwPacket.logs[idx].create_date, RECV_UNKNOWN, sizeof(dwPacket.logs[idx].create_date) - 1);			
					dwPacket.logs[idx].status = 999;				
					idx++;
				}
			}
			ch->GetDesc()->Packet(&dwPacket, sizeof(dwPacket));
		}
		break;
		
		case LOGS_FROM_REPLY:
		{
			TPacketTicketLogsDataReply dwPacket;
			dwPacket.header = HEADER_GC_TICKET_LOGS_REPLY;

			SQLMsg *dwExtractList = DBManager::instance().DirectQuery("SELECT * FROM ticket.reply WHERE ticked_id = '%s' ORDER BY reply_date DESC LIMIT %d", szTicketID, MAX_LOGS_FROM_REPLY);
			MYSQL_ROW iCurRow;
			int idx = 0;
			
			if(dwExtractList->uiSQLErrno != 0)
				return false;

			while ((iCurRow = mysql_fetch_row(dwExtractList->Get()->pSQLResult)))
			{
				dwPacket.logs[idx] = TPacketTicketLogsReply();
				strncpy(dwPacket.logs[idx].reply_from, iCurRow[2], sizeof(dwPacket.logs[idx].reply_from));
				strncpy(dwPacket.logs[idx].reply_content, iCurRow[3], sizeof(dwPacket.logs[idx].reply_content));
				strncpy(dwPacket.logs[idx].reply_date, iCurRow[4], sizeof(dwPacket.logs[idx].reply_date) - 1);
				idx++;
			}

			if(dwExtractList->Get()->uiNumRows < MAX_LOGS_FROM_REPLY)
			{
				while (idx < MAX_LOGS_FROM_REPLY)
				{
					dwPacket.logs[idx] = TPacketTicketLogsReply();
					strncpy(dwPacket.logs[idx].reply_from, RECV_NULL, sizeof(dwPacket.logs[idx].reply_from));
					strncpy(dwPacket.logs[idx].reply_content, RECV_UNKNOWN, sizeof(dwPacket.logs[idx].reply_content));		
					strncpy(dwPacket.logs[idx].reply_date, RECV_UNKNOWN, sizeof(dwPacket.logs[idx].reply_date) - 1);					
					idx++;
				}
			}
			ch->GetDesc()->Packet(&dwPacket, sizeof(dwPacket));
		}
		break;
		
		case LOGS_ADMIN:
		{
			char szQuery[QUERY_MAX_LEN + 1];
			int iStartIdx = 0;
			int iEndIdx = 0;
			
			for (int iCurIndex = 1; iCurIndex <= MAX_PAGE_ADMIN; iCurIndex++)
			{
				if (iCurIndex == iStartPage)
				{
					if (iStartPage == 1)
					{
						iStartIdx = 0;
						iEndIdx = MAX_LOGS_PER_PAGE;
						break;
					}
					else
					{
						iStartIdx = MAX_LOGS_PER_PAGE * (iCurIndex - 1);
						iEndIdx = MAX_LOGS_PER_PAGE * (iCurIndex - 1) + MAX_LOGS_PER_PAGE;
						break;
					}
				}
			}

			unique_ptr<SQLMsg> dwExtract(DBManager::instance().DirectQuery("SELECT COUNT(*) FROM ticket.list"));
			MYSQL_ROW maxRow = mysql_fetch_row(dwExtract->Get()->pSQLResult);
			
			int	iMaxRows = 0;
			str_to_number(iMaxRows, maxRow[0]);
			
			if (iStartIdx < 0 || iEndIdx < 0)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_LOGS_ARGUMENTS_LOW"));
				return false;
			}
			
			if (iEndIdx > (iMaxRows + MAX_LOGS_PER_PAGE))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_LOGS_ARGUMENTS_HIGH"), iStartIdx, iEndIdx, iMaxRows);
				return false;
			}
			
			typedef map<int, string> TMapString;
			typedef map<int, int> TMapInt;
			TMapString ticked_id, char_name, title, content, create_date;
			TMapInt priority, status;

			if (iSortMode == 1)
				snprintf(szQuery, sizeof(szQuery), "SELECT * FROM ticket.list ORDER BY create_date DESC LIMIT %d, %d", iStartIdx, iEndIdx);
			else if (iSortMode == 2)
				snprintf(szQuery, sizeof(szQuery), "SELECT * FROM ticket.list WHERE status = %d ORDER BY create_date DESC LIMIT %d, %d", STATUS_OPENED, iStartIdx, iEndIdx);
			else if (iSortMode == 3)
				snprintf(szQuery, sizeof(szQuery), "SELECT * FROM ticket.list WHERE status = %d ORDER BY create_date DESC LIMIT %d, %d", STATUS_CLOSED, iStartIdx, iEndIdx);
			else if (iSortMode == 4)
				snprintf(szQuery, sizeof(szQuery), "SELECT * FROM ticket.list WHERE priority > 1 ORDER BY create_date DESC LIMIT %d, %d", iStartIdx, iEndIdx);

			SQLMsg *dwExtractList = DBManager::instance().DirectQuery(szQuery);

			MYSQL_ROW iCurRow;
			int idx = 0;
			int dwPriority = 0, dwStatus = 0;

			if(dwExtractList->uiSQLErrno != 0)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_LOGS_LOADED_MYSQL"));
				return false;
			}
					
			while ((iCurRow = mysql_fetch_row(dwExtractList->Get()->pSQLResult)))
			{
				const char* ar_DataRows[8] =
				{
					iCurRow[0],
					iCurRow[1],
					iCurRow[2],
					iCurRow[3],
					iCurRow[4],
					iCurRow[5],
					iCurRow[6],
					iCurRow[7]
				};				
						
				ticked_id[idx] = ar_DataRows[1];
				char_name[idx] = ar_DataRows[2];
				title[idx] = ar_DataRows[3];
				content[idx] = ar_DataRows[4];
				create_date[idx] = ar_DataRows[6];

				str_to_number(dwPriority, ar_DataRows[5]);
				str_to_number(dwStatus, ar_DataRows[7]);

				priority[idx] = dwPriority, status[idx] = dwStatus;
				idx++;
			}
					
			if(dwExtractList->Get()->uiNumRows < MAX_LOGS_PER_PAGE)
			{
				while (idx < MAX_LOGS_PER_PAGE)
				{
					ticked_id[idx] = RECV_NULL, char_name[idx] = RECV_NULL, title[idx] = RECV_NULL, content[idx] = RECV_NULL, create_date[idx] = RECV_NULL;
					priority[idx] = 0, status[idx] = 0;
					idx++;
				}
			}

			int index = 0;
			while (index < MAX_LOGS_PER_PAGE)
			{
				algorithm::replace_all(ticked_id[index], " ", "#");
				algorithm::replace_all(char_name[index], " ", "#");
				algorithm::replace_all(title[index], " ", "#");
				algorithm::replace_all(content[index], " ", "#");
				algorithm::replace_all(create_date[index], " ", "#");
						
				char szBuf[QUERY_MAX_LEN + 1];
				snprintf(szBuf, sizeof(szBuf), "ticket team_logs %d %s %s %s %s %d %s %d", index, ticked_id[index].c_str(), char_name[index].c_str(), title[index].c_str(), content[index].c_str(), priority[index], create_date[index].c_str(), status[index]);
				ch->ChatPacket(CHAT_TYPE_COMMAND, szBuf);
				index++;
			}
		}
		break;

		default:
			break;
	}
}

void CTicketSystem::ChangePage(LPCHARACTER ch, const char* c_pData)
{
	TPacketCGTicketAdminChangePage * p = (TPacketCGTicketAdminChangePage *)c_pData;
	
	if (!ch)
		return;
	
	if (p->iStartPage < 0 || p->iStartPage > MAX_PAGE_ADMIN)
		return;
	
	if (!CTicketSystem::instance().GetIsMemberStaff(ch))
		return;

	CTicketSystem::instance().SendTicketLogs(ch, LOGS_ADMIN, RECV_UNKNOWN, p->iStartPage, CTicketSystem::instance().GetFilter(ch));
}

void CTicketSystem::Open(LPCHARACTER ch, const char* c_pData)
{
	TPacketCGTicketOpen * p = (TPacketCGTicketOpen *)c_pData;
	
	if (!ch)
		return;
	
	if (!(CTicketSystem::instance().GetLastTimeUsed(ch, 1)))	
		return;
	
	if ((CTicketSystem::instance().GetAccountBanned(ch)))
		return;
	
	if (p->action == 2 && strlen(p->ticked_id) <= 0)
		return;
	
	switch(p->action)
	{
		case PAGE_SORT_ADMIN:
		{
			if (p->mode > 0 && CTicketSystem::instance().GetIsMemberStaff(ch))
			{
				ch->SetQuestFlag(RECV_MODE_SORT, p->mode);
				CTicketSystem::instance().SendTicketLogs(ch, LOGS_ADMIN, RECV_UNKNOWN, 1, p->mode);
				ch->ChatPacket(CHAT_TYPE_COMMAND, "ticket elevate o %d", 1);
			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_OPEN_SORT"));
			}
		}
		break;
		
		case PAGE_HOME:
		{
			if (CTicketSystem::instance().GetIsMemberStaff(ch))
			{
				ch->ChatPacket(CHAT_TYPE_COMMAND, "ticket elevate r %d", 1);
			}
			
			else if (!CTicketSystem::instance().GetIsMemberStaff(ch))
			{
				CTicketSystem::instance().SendTicketLogs(ch, LOGS_GENERAL, RECV_UNKNOWN, 0, 0);
				ch->ChatPacket(CHAT_TYPE_COMMAND, "ticket elevate o %d", 0);
			}
		}
		break;

		case PAGE_REPLY:
		{
			// if (CTicketSystem::instance().IsDenied(ch, p->ticked_id))
				// return;
			
			if (!CTicketSystem::instance().GetExistID(p->ticked_id))
				return;
			
			CTicketSystem::instance().SendTicketLogs(ch, LOGS_FROM_REPLY, p->ticked_id, 0, 0);
		}
		break;

		default:
			break;
	}
}

void CTicketSystem::Create(LPCHARACTER ch, const char* c_pData)
{
	TPacketCGTicketCreate * p = (TPacketCGTicketCreate *)c_pData;
	
	if (!ch)
		return;
	
	if (strlen(p->title) <= 0 || strlen(p->content) <= 0 || strlen(p->content) > MAX_LEN_CONTENT || strlen(p->title) > MAX_LEN_TITLE)
		return;
	
	if (CTicketSystem::instance().GetIsMemberStaff(ch))
	{
		ch->ChatPacket(CHAT_TYPE_NOTICE, LC_TRANSLATE("STAFF_ERROR_CREATE"));
		return;
	}
	
	if (!(CTicketSystem::instance().GetLastTimeUsed(ch, 2)))	
		return;
	
	if (CTicketSystem::instance().GetAccountBanned(ch))
		return;
	
	if (CTicketSystem::instance().IsDenied(ch, p->title) || CTicketSystem::instance().IsDenied(ch, p->content))
		return;

	string strID;

    srand(time(0));
    for(unsigned int i = 0; i < MAX_LEN_TICKET_ID; ++i) {
		strID += CTicketRandom();
	}

	unique_ptr<SQLMsg> dwExtract(DBManager::instance().DirectQuery("SELECT ticked_id FROM ticket.list WHERE ticked_id = '%s'", strID.c_str()));

	while (dwExtract->Get()->uiNumRows > 0)
	{
		srand(time(0));
		for(unsigned int i = 0; i < MAX_LEN_TICKET_ID; ++i) {
			strID += CTicketRandom();
		}
	}
	
	char szQuery[QUERY_MAX_LEN + 1];
	snprintf(szQuery, sizeof(szQuery), "INSERT INTO ticket.list (ticked_id,char_name,title,content,priority,create_date,status) VALUES('%s','%s','%s','%s','%d',NOW(),'%d')", strID.c_str(), ch->GetName(), p->title, p->content, p->priority, STATUS_OPENED);
	DBManager::Instance().DirectQuery(szQuery);
	
	CTicketSystem::instance().SetLastTimeUsed(ch, 2);
	CTicketSystem::instance().SendTicketLogs(ch, LOGS_GENERAL, RECV_UNKNOWN, 0, 0);		
}

void CTicketSystem::Reply(LPCHARACTER ch, const char* c_pData)
{
	TPacketCGTicketReply * p = (TPacketCGTicketReply *)c_pData;
	
	if (!ch)
		return;
	
	if (strlen(p->ticked_id) <= 0 || strlen(p->reply) <= 0 || strlen(p->reply) > MAX_LEN_CONTENT_REPLY)
		return;
	
	if (!CTicketSystem::instance().GetLastTimeUsed(ch, 3))	
		return;
	
	if ((CTicketSystem::instance().GetAccountBanned(ch)))
		return;
	
	if (/*CTicketSystem::instance().IsDenied(ch, p->ticked_id) || */CTicketSystem::instance().IsDenied(ch, p->reply))
		return;

	string strReply = p->reply;
	
	if (!CTicketSystem::instance().GetOwner(ch, p->ticked_id))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("USER_ERROR_OWNER"));
		return;
	}

	if (!CTicketSystem::instance().GetExistID(p->ticked_id))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("USER_ERROR_TICKET_ID"), p->ticked_id);
		return;
	}
	
	if (!CTicketSystem::instance().GetIsOpened(p->ticked_id))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("USER_ERROR_STATUS_NOT_OPENED"), p->ticked_id);
		return;
	}
	
	char szQuery[QUERY_MAX_LEN + 1];
	snprintf(szQuery, sizeof(szQuery), "INSERT INTO ticket.reply (ticked_id,reply_from,reply_content,reply_date) VALUES('%s','%s','%s',NOW())", p->ticked_id, ch->GetName(), strReply.c_str());
	DBManager::Instance().DirectQuery(szQuery);
	
	CTicketSystem::instance().SendTicketLogs(ch, LOGS_FROM_REPLY, p->ticked_id, 0, 0);
	CTicketSystem::instance().SetLastTimeUsed(ch, 3);
}

void CTicketSystem::Action(LPCHARACTER ch, const char* c_pData)
{
	TPacketCGTicketAdmin * p = (TPacketCGTicketAdmin *)c_pData;
	
	if (strlen(p->ticked_id) > MAX_LEN_TICKET_ID || strlen(p->char_name) > MAX_LEN_CHAR_NAME || strlen(p->reason) > MAX_LEN_REASON)
		return;
	
	if (!CTicketSystem::instance().GetIsMemberStaff(ch))
		return;

	switch(p->action)
	{
		case ACTION_BAN:
		{
			if (CTicketSystem::instance().IsDenied(ch, p->char_name) || CTicketSystem::instance().IsDenied(ch, p->reason))
				return;
			
			if (strlen(p->reason) <= 0)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_BAN_WITH_REASON"));		
				return;
			}
				
			unique_ptr<SQLMsg> dwExtractTarget(DBManager::instance().DirectQuery("SELECT name FROM player.player WHERE name = '%s'", p->char_name));
				
			if (dwExtractTarget->uiSQLErrno != 0 || !dwExtractTarget->Get()->uiNumRows)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_SEARCH_USER"));
				return;
			}
				
			if (!strcmp(p->char_name, ch->GetName()))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_CANNOT_BAN_SELF"));
				return;
			}
				
			unique_ptr<SQLMsg> dwExtractName(DBManager::instance().DirectQuery("SELECT account_id FROM player.player WHERE name = '%s'", p->char_name));
			MYSQL_ROW row = mysql_fetch_row(dwExtractName->Get()->pSQLResult);
				
			int account_id = 0;
			str_to_number(account_id, row[0]);

			unique_ptr<SQLMsg> dwExtractID(DBManager::instance().DirectQuery("SELECT account_id FROM ticket.user_restricted WHERE account_id = '%d'", account_id));
				
			if (dwExtractID->Get()->uiNumRows > 0)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_USER_ALREADY_IS_BANNED"));
				return;
			}

			char szQuery[QUERY_MAX_LEN + 1];
			snprintf(szQuery, sizeof(szQuery), "INSERT INTO ticket.user_restricted (account_id, reason) VALUES('%d', '%s')", account_id, p->reason);
			DBManager::Instance().DirectQuery(szQuery);
				
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_SUCCES_BANNED_USER"), p->char_name, p->reason);
		}
		break;
		
		case ACTION_UNBAN:
		{
			if (CTicketSystem::instance().IsDenied(ch, p->char_name))
				return;
				
			unique_ptr<SQLMsg> dwExtractTarget(DBManager::instance().DirectQuery("SELECT name FROM player.player WHERE name = '%s'", p->char_name));
				
			if (dwExtractTarget->uiSQLErrno != 0 || !dwExtractTarget->Get()->uiNumRows)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_SEARCH_USER"));
				return;
			}
				
			if (!strcmp(p->char_name, ch->GetName()))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_CANNOT_UNBAN_SELF"));
				return;
			}
				
			unique_ptr<SQLMsg> dwExtractName(DBManager::instance().DirectQuery("SELECT account_id FROM player.player WHERE name = '%s'", p->char_name));
			MYSQL_ROW row = mysql_fetch_row(dwExtractName->Get()->pSQLResult);
				
			int account_id = 0;
			str_to_number(account_id, row[0]);

			unique_ptr<SQLMsg> dwExtractID(DBManager::instance().DirectQuery("SELECT account_id FROM ticket.user_restricted WHERE account_id = '%d'", account_id));
			
			if (dwExtractID->uiSQLErrno != 0 || !dwExtractID->Get()->uiNumRows)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_USER_IS_NOT_BANNED"));	
				return;
			}
				
			if (dwExtractID->Get()->uiNumRows > 0)
			{
				char szQuery[QUERY_MAX_LEN];
				snprintf(szQuery, sizeof(szQuery), "DELETE FROM ticket.user_restricted WHERE account_id = '%d'", account_id);
				unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery(szQuery));	
			}
				
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_SUCCES_UNBANNED_USER"), p->char_name);
		}
		break;
		
		case ACTION_STATUS_OPEN:
		{
			// if (CTicketSystem::instance().IsDenied(ch, p->ticked_id))
				// return;

			if (!CTicketSystem::instance().GetExistID(p->ticked_id))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_CHANGE_STATUS_NOID"), p->ticked_id);
				return;
			}
			
			if (CTicketSystem::instance().GetIsOpened(p->ticked_id))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_CHANGE_STATUS_ALREADY_IS_OPENED"), p->ticked_id);
				return;				
			}
			
			char szQuery[CHAT_MAX_LEN * 2 + 1];
			snprintf(szQuery, sizeof(szQuery), "UPDATE ticket.list SET status = '%d' WHERE ticked_id = '%s'", STATUS_OPENED, p->ticked_id);
			unique_ptr<SQLMsg> update(DBManager::instance().DirectQuery(szQuery));
			
			CTicketSystem::instance().SendTicketLogs(ch, LOGS_ADMIN, RECV_UNKNOWN, 0, CTicketSystem::instance().GetFilter(ch));
			
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_SUCCES_CHANGE_STATUS_TO_OPEN"), p->ticked_id);
		}
		break;
		
		case ACTION_STATUS_CLOSED:
		{
			// if (CTicketSystem::instance().IsDenied(ch, p->ticked_id))
				// return;

			if (!CTicketSystem::instance().GetExistID(p->ticked_id))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_CHANGE_STATUS_NOID"), p->ticked_id);
				return;
			}
			
			if (!CTicketSystem::instance().GetIsOpened(p->ticked_id))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_CHANGE_STATUS_ALREADY_IS_CLOSED"), p->ticked_id);
				return;				
			}
			
			char szQuery[CHAT_MAX_LEN * 2 + 1];
			snprintf(szQuery, sizeof(szQuery), "UPDATE ticket.list SET status = '%d' WHERE ticked_id = '%s'", STATUS_CLOSED, p->ticked_id);
			unique_ptr<SQLMsg> update(DBManager::instance().DirectQuery(szQuery));
			
			CTicketSystem::instance().SendTicketLogs(ch, LOGS_ADMIN, RECV_UNKNOWN, 0, CTicketSystem::instance().GetFilter(ch));
			
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_SUCCES_CHANGE_STATUS_TO_CLOSED"), p->ticked_id);
		}
		break;
		default:
			break;
	}
}