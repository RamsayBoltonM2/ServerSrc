/*********************************************************************
* date        : 2016.09.21
* file        : bank.cpp
* author      : VegaS
* version	  : 0.0.4
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
#include "bank.h"
#include <string>

using namespace std;

BankManager::BankManager()
{
}
BankManager::~BankManager()
{
}

#define ENABLE_BANK_LAST_TIME

static const char* locale_string_bank[] = /* From here you can translate */
{
	"<Bank> Unele caractere nu sunt permise.",
	"<Bank> Parola este incorecta , trebuie sa ai 4 caractere.",
	"<Bank> Contul %s deja exista !",
	"<Bank> Contul %s a fost creat cu succes ! Parola: %s.",
	
	"<Bank> Contul %s care incerci sa ii trimiti o suma de yang nu exista",
	"<Bank> Aceasta suma este prea putina.",
	"<Bank> Nu esti detinatorul acestor bani din banca.",
	"<Bank> Info. tinta  trimis cu succes : [Nume cont: %s , Yang: %d]",

	"<Bank> Suma introdusa este prea mica.",
	"<Bank> Ai prea putini bani.",
	"<Bank> Suma introdusa pentru retragere este mai mare decat suma existenta.",
	"<Bank> Nu detii acesti bani in banca",
	"<Bank> Nu potii detine aceasta suma in inventar.",

	"<Bank> Parola este incorecta , trebuie sa ai 4 caractere.",
	"<Bank> Parola  repetata este incorecta , trebuie sa ai 4 caractere.",
	"<Bank> Parola nu este la fel ca cea repetata.",
	"<Bank> Parola repetata nu este identica cu prima parola.",
	"<Bank> Deja detii aceasta parola , te rog alege o alta parola.",
	"<Bank> Parola dvs. a fost schimbata cu succes ! Noua parola este: %s",

	"<Bank> Nu poti face aceasta actiune atat de rapid, trebuie sa astepti %d secunda(e) !"
};

bool ReturnCommand(LPCHARACTER ch, int mode, const char* row)
{
	if (mode == 1)
		ch->ChatPacket(CHAT_TYPE_COMMAND, "BINARY_Account_Bank close 0");
	else if (mode == 2)
		ch->ChatPacket(CHAT_TYPE_COMMAND, "BINARY_Account_Bank open 0");
	else if (mode == 3)
		ch->ChatPacket(CHAT_TYPE_COMMAND, "BINARY_Account_Bank reload %s", row);
	else if (mode == 4)	
		ch->ChatPacket(CHAT_TYPE_COMMAND, "BINARY_Account_Bank wrong_id 0");
	else if (mode == 5)		
		ch->ChatPacket(CHAT_TYPE_COMMAND, "BINARY_Account_Bank wrong_password 0");
}

#ifdef ENABLE_BANK_LAST_TIME
#define stringLastTime "bank.used_mode_"
bool GetTime(LPCHARACTER ch, int mode)
{
	string module;
	ostringstream str;
				
	str << stringLastTime << mode;
	module = str.str();
	
	int get_last_time = ch->GetQuestFlag(module);

	if (get_last_time)
	{
		if (get_global_time() < get_last_time)
		{
			int amount = get_last_time - get_global_time();
			int seconds = amount % 60;
			ch->ChatPacket(CHAT_TYPE_INFO, locale_string_bank[19], seconds);
			return false;
		}
	}
	return true;
}

void SetTime(LPCHARACTER ch, int mode)
{
	for (int increment = 0; increment <= 6; increment++)
	{
		if (increment == mode)
		{
			string module;
			ostringstream str;
				
			str << stringLastTime << mode;
			module = str.str();
			
			ch->SetQuestFlag(module, get_global_time() + 15);
		}
	}	
}
#endif
bool CheckExploit(LPCHARACTER ch, const char* mex1, const char* mex2, const char* mex3, const char* mex4)
{
    static char ret[] = {'?', '*', '$', '!', '/', '>', '<', '|', ';', ':', '}', '{', '[', ']', '%', '#', '@', '^', '&', 'DROP', 'TRUNCATE'};
	static std::string str1 = "", str2 = "", str3 = "", str4 = "";

	str1 = mex1; str2 = mex2; str3 = mex3; str4 = mex4;
	
	for (int i = 0; i < _countof(ret); i++)
	{
		if (str1.find(ret[i]) != std::string::npos || str2.find(ret[i]) != std::string::npos || str3.find(ret[i]) != std::string::npos || str4.find(ret[i]) != std::string::npos)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, locale_string_bank[0]);
			return false;
        }
	}

	if (strlen(mex1) > 12 || strlen(mex2) > 4)
		return false;

	return true;
}

bool CheckAccountBank(LPCHARACTER ch, const char* mAccount, const char* mPassword)
{
	SQLMsg *selectID = DBManager::instance().DirectQuery("SELECT user_name FROM account.bank_user WHERE user_name = '%s'", mAccount);
	SQLMsg *selectPassword = DBManager::instance().DirectQuery("SELECT user_password FROM account.bank_user WHERE user_name = '%s' AND user_password = '%s'", mAccount, mPassword);	
	
	if (selectID->Get()->uiNumRows == 0)
	{
		ReturnCommand(ch, 4, "");
		return false;
	}
	
	if (selectPassword->Get()->uiNumRows == 0)
	{
		ReturnCommand(ch, 5, "");
		return false;
	}
	
	if (selectID->Get() && selectPassword->Get())
		return true;
}

bool Send_Logs(LPCHARACTER ch, const char* mAccount)
{
#define MAX_LOGS 1000
	TPacketCGBank p;
	p.header = HEADER_GC_BANK;

	SQLMsg *selectLogs = DBManager::instance().DirectQuery("SELECT * FROM account.bank_log WHERE user_name = '%s' ORDER BY user_datetime DESC LIMIT 1000", mAccount);
	MYSQL_ROW res;
	int tmp = 0;
	
	if(selectLogs->uiSQLErrno != 0)
		return false;

	while ((res = mysql_fetch_row(selectLogs->Get()->pSQLResult)))
	{
		p.logs[tmp] = TAccountBank();
		str_to_number(p.logs[tmp].user_action, res[1]);
		str_to_number(p.logs[tmp].user_money, res[2]);
		strncpy(p.logs[tmp].user_datetime, res[3], sizeof(p.logs[tmp].user_datetime) - 1);
		strncpy(p.logs[tmp].user_ip, res[4], sizeof(p.logs[tmp].user_ip));
		strncpy(p.logs[tmp].user_recvmoney, res[5], sizeof(p.logs[tmp].user_recvmoney));
		tmp++;
	}
	if(selectLogs->Get()->uiNumRows < MAX_LOGS)
	{
		while (tmp<MAX_LOGS)
		{
			p.logs[tmp] = TAccountBank();
			p.logs[tmp].user_action = 999;
			p.logs[tmp].user_money = 0;
			strncpy(p.logs[tmp].user_datetime, "0000-00-00 00:00:00", sizeof(p.logs[tmp].user_datetime) - 1);
			strncpy(p.logs[tmp].user_ip, "-", sizeof(p.logs[tmp].user_ip));
			strncpy(p.logs[tmp].user_recvmoney, "-", sizeof(p.logs[tmp].user_recvmoney));
			tmp++;
		}
	}
	ch->GetDesc()->Packet(&p, sizeof(p));
}

bool BankManager::Send_Create(LPCHARACTER ch, const char* mNewAccount, const char* mNewPassword)
{
	if (!ch->GetDesc())
		return false;
	
	if ((CheckExploit(ch, mNewAccount, mNewPassword, "", "")) == false)
		return false;
	
	if (!mNewPassword)
		return false;
	
	if (strlen(mNewPassword) != 4)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, locale_string_bank[1]);
		return false;
	}

	SQLMsg *createdID = DBManager::instance().DirectQuery("SELECT user_name FROM account.bank_user WHERE user_name = '%s'", mNewAccount);
	
	if (createdID->Get()->uiNumRows > 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, locale_string_bank[2], mNewAccount);
		return false;
	}
#ifdef ENABLE_BANK_LAST_TIME	
	if ((GetTime(ch, 1)) == false)
		return false;
#endif
	char sqQuery[QUERY_MAX_LEN + 1];
	snprintf(sqQuery, sizeof(sqQuery), "INSERT INTO account.bank_user (user_name, user_gold, user_password) VALUES('%s', '0', '%s')", mNewAccount, mNewPassword);
	DBManager::Instance().DirectQuery(sqQuery);

	ch->ChatPacket(CHAT_TYPE_INFO, locale_string_bank[3], mNewAccount, mNewPassword);
#ifdef ENABLE_BANK_LAST_TIME
	SetTime(ch, 1);	
#endif
}

bool BankManager::Send_Open(LPCHARACTER ch, const char* mAccount, const char* mPassword)
{
	if (!ch->GetDesc())
		return false;
	
	if ((CheckExploit(ch, mAccount, mPassword, "", "")) == false)
		return false;
	
	if ((CheckAccountBank(ch, mAccount, mPassword)) == false)
		return false;
#ifdef ENABLE_BANK_LAST_TIME
	if ((GetTime(ch, 2)) == false)
		return false;
#endif
	SQLMsg *selectMoney = DBManager::instance().DirectQuery("SELECT user_gold FROM account.bank_user WHERE user_name = '%s' AND user_password = '%s'", mAccount, mPassword);
	MYSQL_RES *result = selectMoney->Get()->pSQLResult;
	MYSQL_ROW row = mysql_fetch_row(result);

	if (selectMoney->uiSQLErrno != 0 || !result || !selectMoney->Get() || selectMoney->Get()->uiNumRows < 0)
		return false;

	Send_Logs(ch, mAccount);
	ReturnCommand(ch, 3, row[0]);
	ReturnCommand(ch, 2, "");
#ifdef ENABLE_BANK_LAST_TIME
	SetTime(ch, 2);
#endif
	return false;
}

bool BankManager::Send_AddMoney(LPCHARACTER ch, const char* mAccount, const char* mPassword, const char* mAmount)
{
	if ((CheckExploit(ch, mAccount, mPassword, mAmount, "")) == false)
		return false;

	if ((CheckAccountBank(ch, mAccount, mPassword)) == false)
		return false;
#ifdef ENABLE_BANK_LAST_TIME
	if ((GetTime(ch, 3)) == false)
		return false;
#endif
	int moneyRequest = 0;
	str_to_number(moneyRequest, mAmount);
	
	if (moneyRequest < 1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, locale_string_bank[8]);
		return false;
	}
	
	if (ch->GetGold() - moneyRequest < 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, locale_string_bank[9]);
		return false;
	}

	char buf[CHAT_MAX_LEN + 1];
	snprintf(buf, sizeof(buf), "UPDATE account.bank_user SET user_gold = user_gold + '%d' WHERE user_name = '%s' AND user_password = '%s'", moneyRequest, mAccount, mPassword);
	std::unique_ptr<SQLMsg> update(DBManager::instance().DirectQuery(buf));

	ch->PointChange(POINT_GOLD, - moneyRequest, true); 
	
	SQLMsg *selectMoney = DBManager::instance().DirectQuery("SELECT user_gold FROM account.bank_user WHERE user_name = '%s' AND user_password = '%s'", mAccount, mPassword);
	MYSQL_RES *result = selectMoney->Get()->pSQLResult;
	MYSQL_ROW row = mysql_fetch_row(result);

	char sqQuery[QUERY_MAX_LEN + 1];
	snprintf(sqQuery, sizeof(sqQuery), "INSERT INTO account.bank_log (user_name, user_action, user_money, user_datetime, user_ip) VALUES('%s', '1', '%d', NOW(), '%s')", mAccount, moneyRequest, ch->GetDesc()->GetHostName());
	DBManager::Instance().DirectQuery(sqQuery);	
	
	Send_Logs(ch, mAccount);	
	ReturnCommand(ch, 3, row[0]);
#ifdef ENABLE_BANK_LAST_TIME
	SetTime(ch, 3);	
#endif
}

bool BankManager::Send_WithdrawMoney(LPCHARACTER ch, const char* mAccount, const char* mPassword, const char* mAmount)
{	
	if ((CheckExploit(ch, mAccount, mPassword, mAmount, "")) == false)
		return false;

	if ((CheckAccountBank(ch, mAccount, mPassword)) == false)
		return false;
#ifdef ENABLE_BANK_LAST_TIME
	if ((GetTime(ch, 4)) == false)
		return false;
#endif
	SQLMsg *selectPassword = DBManager::instance().DirectQuery("SELECT user_gold FROM account.bank_user WHERE user_name = '%s' AND user_password = '%s'", mAccount, mPassword);	
	
	MYSQL_RES *result = selectPassword->Get()->pSQLResult;
	MYSQL_ROW line = mysql_fetch_row(result);
	
	long long moneyRequest = 0;
	str_to_number(moneyRequest, mAmount);
	
	if (moneyRequest < 1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, locale_string_bank[10]);
		return false;
	}
	
	long long moneyBank = 0;
	str_to_number(moneyBank, line[0]);
	
	if (moneyRequest > moneyBank)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, locale_string_bank[11]);
		return false;
	}
	
	if (static_cast<long long>(ch->GetGold()) + moneyRequest > GOLD_MAX - 1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, locale_string_bank[12]);
		return false;
	}

	char buf[CHAT_MAX_LEN + 1];
	snprintf(buf, sizeof(buf), "UPDATE account.bank_user SET user_gold = user_gold - '%lld' WHERE user_name = '%s' AND user_password = '%s'", moneyRequest, mAccount, mPassword);
	std::unique_ptr<SQLMsg> pDuration(DBManager::instance().DirectQuery(buf));		
	
	ch->PointChange(POINT_GOLD, moneyRequest, true); 

	{
		SQLMsg *selectMoney = DBManager::instance().DirectQuery("SELECT user_gold FROM account.bank_user WHERE user_name = '%s' AND user_password = '%s'", mAccount, mPassword);
		MYSQL_RES *result = selectMoney->Get()->pSQLResult;
		MYSQL_ROW row = mysql_fetch_row(result);

		char sqQuery[QUERY_MAX_LEN + 1];
		snprintf(sqQuery, sizeof(sqQuery), "INSERT INTO account.bank_log (user_name, user_action, user_money, user_datetime, user_ip) VALUES('%s', '2', '%lld', NOW(), '%s')", mAccount, moneyRequest, ch->GetDesc()->GetHostName());
		DBManager::Instance().DirectQuery(sqQuery);	
			
		Send_Logs(ch, mAccount);
		ReturnCommand(ch, 3, row[0]);
#ifdef ENABLE_BANK_LAST_TIME
		SetTime(ch, 4);	
#endif
	}
}

bool BankManager::Send_Money(LPCHARACTER ch, const char* mAccount, const char* mPassword, const char* mTargetAccount, const char* mAmount)
{
	if ((CheckExploit(ch, mAccount, mPassword, mTargetAccount, mAmount)) == false)
		return false;

	if ((CheckAccountBank(ch, mAccount, mPassword)) == false)
		return false;
#ifdef ENABLE_BANK_LAST_TIME
	if ((GetTime(ch, 5)) == false)
		return false;
#endif
	SQLMsg *targetID = DBManager::instance().DirectQuery("SELECT user_name FROM account.bank_user WHERE user_name = '%s'", mTargetAccount);
	
	if (targetID->Get()->uiNumRows == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, locale_string_bank[4], mTargetAccount);
		return false;
	}

	int moneyRequest = 0;
	str_to_number(moneyRequest, mAmount);
	
	if (moneyRequest <= 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, locale_string_bank[5]);
		return false;
	}

	if (*mTargetAccount == *mAccount)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "<Bank> Kendine para gonderemezsin.");
		return false;
	}
	
	SQLMsg *selectMoney = DBManager::instance().DirectQuery("SELECT user_gold FROM account.bank_user WHERE user_name = '%s' AND user_password = '%s'", mAccount, mPassword);	
		
	MYSQL_RES *res = selectMoney->Get()->pSQLResult;
	MYSQL_ROW row = mysql_fetch_row(res);
		
	long long moneyBank = 0;
	str_to_number(moneyBank, row[0]);
		
	if (moneyRequest > moneyBank)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, locale_string_bank[6]);
		return false;
	}

	char buf[CHAT_MAX_LEN + 1];
	snprintf(buf, sizeof(buf), "UPDATE account.bank_user SET user_gold = user_gold - '%d' WHERE user_name = '%s'", moneyRequest, mAccount);
	std::unique_ptr<SQLMsg> ret(DBManager::instance().DirectQuery(buf));
		
	char targetBuf[CHAT_MAX_LEN + 1];
	snprintf(targetBuf, sizeof(targetBuf), "UPDATE account.bank_user SET user_gold = user_gold + '%d' WHERE user_name = '%s'", moneyRequest, mTargetAccount);
	std::unique_ptr<SQLMsg> mex(DBManager::instance().DirectQuery(targetBuf));
		
	char sqQuery[QUERY_MAX_LEN + 1];
	snprintf(sqQuery, sizeof(sqQuery), "INSERT INTO account.bank_log (user_name, user_action, user_money, user_datetime, user_ip, user_recvmoney) VALUES('%s', '3', '%d', NOW(), '%s', '%s')", mAccount, moneyRequest, ch->GetDesc()->GetHostName(), mTargetAccount);
	DBManager::Instance().DirectQuery(sqQuery);
		
	char sqQueryTarget[QUERY_MAX_LEN + 1];
	snprintf(sqQueryTarget, sizeof(sqQueryTarget), "INSERT INTO account.bank_log (user_name, user_action, user_money, user_datetime, user_recvmoney) VALUES('%s', '4', '%d', NOW(), '%s')", mTargetAccount, moneyRequest, mAccount);
	DBManager::Instance().DirectQuery(sqQueryTarget);

	Send_Logs(ch, mAccount);
	ch->ChatPacket(CHAT_TYPE_INFO, locale_string_bank[7], mTargetAccount, moneyRequest);
	ReturnCommand(ch, 3, row[0]);
#ifdef ENABLE_BANK_LAST_TIME
	SetTime(ch, 5);	
#endif
}

bool BankManager::Send_ChangePassword(LPCHARACTER ch, const char* mAccount, const char* mPassword, const char* mNewPassword, const char* mNewPasswordRepeat)
{
	if ((CheckExploit(ch, mAccount, mPassword, mNewPassword, mNewPasswordRepeat)) == false)
		return false;
	
	if ((CheckAccountBank(ch, mAccount, mPassword)) == false)
		return false;
	
	if (!*mNewPassword && !*mNewPasswordRepeat)
		return false;
	
	if (strlen(mNewPassword) != 4)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, locale_string_bank[13]);
		return false;
	}
	
	if (strlen(mNewPasswordRepeat) != 4)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, locale_string_bank[14]);
		return false;
	}
	
	if (*mNewPassword != *mNewPasswordRepeat)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, locale_string_bank[15]);
		return false;
	}
	
	if (*mNewPasswordRepeat != *mNewPassword)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, locale_string_bank[16]);
		return false;
	}	
#ifdef ENABLE_BANK_LAST_TIME
	if ((GetTime(ch, 6)) == false)
		return false;
#endif
	SQLMsg *selectPassword = DBManager::instance().DirectQuery("SELECT user_password FROM account.bank_user WHERE user_name = '%s' AND user_password = '%s'", mAccount, mPassword);
	MYSQL_RES *res = selectPassword->Get()->pSQLResult;
	MYSQL_ROW row = mysql_fetch_row(res);
	
	const char* resAcc = row[0];
	
	if (*mNewPassword == *resAcc || *mNewPasswordRepeat == *resAcc)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, locale_string_bank[17]);
		return false;
	}		
	
	char buf[CHAT_MAX_LEN + 1];
	snprintf(buf, sizeof(buf), "UPDATE account.bank_user SET user_password = '%s' WHERE user_name = '%s' AND user_password = '%s'", mNewPassword, mAccount, mPassword);
	std::unique_ptr<SQLMsg> pDuration(DBManager::instance().DirectQuery(buf));
	
	char sqQuery[QUERY_MAX_LEN + 1];
	snprintf(sqQuery, sizeof(sqQuery), "INSERT INTO account.bank_log (user_name, user_action, user_datetime, user_money, user_ip) VALUES('%s', '5', NOW(), '53', '%s')", mAccount, ch->GetDesc()->GetHostName());
	DBManager::Instance().DirectQuery(sqQuery);	

	ch->ChatPacket(CHAT_TYPE_INFO, locale_string_bank[18], mNewPassword);
	ReturnCommand(ch, 1, "");
#ifdef ENABLE_BANK_LAST_TIME
	SetTime(ch, 6);	
#endif
}
