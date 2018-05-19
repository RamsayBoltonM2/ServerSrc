#ifndef __INC__METIN_II_VEGAS_BIOLOG__
#define __INC__METIN_II_VEGAS_BIOLOG__
/*********************************************************************
* title_name		: Professional Biolog System
* date_created		: 2016.08.07
* filename			: biolog.h
* author			: VegaS
*/
#pragma once
class BiologManager : public singleton<BiologManager>
{
	public:
		BiologManager();
~BiologManager();
		
	bool	SendUpdate_Binary(LPCHARACTER ch);
	bool	SendButton(LPCHARACTER ch);	
	bool	Send_FinishedRefresh(LPCHARACTER ch);	
	void	SendBonusType(LPCHARACTER ch, int argument);	
	void	RestartTime(LPCHARACTER ch);
	void	SendWindow_SelectType(LPCHARACTER ch, int iReward);		
	void	SelectBonusType(LPCHARACTER ch, const char* iReward);		
};
#endif