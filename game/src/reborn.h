#ifndef __INC_METIN_II_GAME_REBORN_SYSTEM_H__
#define __INC_METIN_II_GAME_REBORN_SYSTEM_H__
/*********************************************************************
* date        : 2016.05.03
* file        : reborn.h
* author      : VegaS
* version	  : 0.1.1
*/
#pragma once

class RebornManager : public singleton<RebornManager>
{
	public:
		RebornManager();
		~RebornManager();
	
	void	ResetCharacter(LPCHARACTER ch);	
	void	SetLevel(LPCHARACTER ch);
	
	bool	CheckItem(LPCHARACTER ch, const char* argument);
	
	void 	RewardCoins(LPCHARACTER ch, int mode);
	void	SetAffect(LPCHARACTER ch, const char* valueAffect);	
	void	SetAnonymous(LPCHARACTER ch, const char* valueAnonymous);
	void	SendLogs(LPCHARACTER ch, const char* nameLog);	
};

#endif