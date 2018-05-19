#include "stdafx.h"
#include "desc.h"
#include "utils.h"
#include "char.h"
#include "affect.h"
#include <string>
#include <algorithm>
#include <boost/algorithm/string/replace.hpp>
#include "biolog.h"
#include "biologDefines.h"

const char* pTableStringGlobal[5][10] =
{  
	{"biolog_lv30.collect_count_actual", "biolog_lv40.collect_count_actual", "biolog_lv50.collect_count_actual", "biolog_lv60.collect_count_actual", "biolog_lv70.collect_count_actual", "biolog_lv80.collect_count_actual", "biolog_lv85.collect_count_actual", "biolog_lv90.collect_count_actual", "biolog_lv92.collect_count_actual", "biolog_lv94.collect_count_actual"},{"biolog_lv30.left_time", "biolog_lv40.left_time", "biolog_lv50.left_time", "biolog_lv60.left_time", "biolog_lv70.left_time", "biolog_lv80.left_time", "biolog_lv85.left_time", "biolog_lv90.left_time", "biolog_lv92.left_time", "biolog_lv94.left_time"},{"biolog_lv30.duration_send", "biolog_lv40.duration_send", "biolog_lv50.duration_send", "biolog_lv60.duration_send", "biolog_lv70.duration_send", "biolog_lv80.duration_send", "biolog_lv85.duration_send", "biolog_lv90.duration_send", "biolog_lv92.duration_send", "biolog_lv94.duration_send"},{"biolog_lv30.completed", "biolog_lv40.completed", "biolog_lv50.completed", "biolog_lv60.completed", "biolog_lv70.completed", "biolog_lv80.completed", "biolog_lv85.completed", "biolog_lv90.completed", "biolog_lv92.completed", "biolog_lv94.completed"}		
};			

const char* pxCountTable[] =
{
	pTableStringGlobal[0][0], pTableStringGlobal[0][1], pTableStringGlobal[0][2], pTableStringGlobal[0][3], pTableStringGlobal[0][4], pTableStringGlobal[0][5], pTableStringGlobal[0][6], pTableStringGlobal[0][7], pTableStringGlobal[0][8], pTableStringGlobal[0][9]
};		

int pTableAffect[] =
{
	AFF_BIO_1, AFF_BIO_2, AFF_BIO_3, AFF_BIO_4, AFF_BIO_5, AFF_BIO_6, AFF_BIO_7, AFF_BIO_8, AFF_BIO_9, AFF_BIO_10, AFF_BIO_11, AFF_BIO_12, AFF_BIO_13, AFF_BIO_14, AFF_BIO_15, AFF_BIO_16, AFF_BIO_17, AFF_BIO_18, AFF_BIO_19, AFF_BIO_20
};


BiologManager::BiologManager()
{	
}

BiologManager::~BiologManager()
{
}
void BiologManager::RestartTime(LPCHARACTER ch)
{
	#define TIME_FOR_WAIT_TO_USE_AGAIN	6*60*60 // You can use one item once every 6 hours, that means 4 items per day.

	char szBuf[CHAT_MAX_LEN + 1];
	snprintf(szBuf, sizeof(szBuf), "BIOLOG.ITEM_RESTART_TIME");

	int curTime = ch->GetQuestFlag(szBuf);

	if (curTime && get_global_time() < curTime)
	{
		int c = curTime - get_global_time();
		int mSec = c % 60;
			c/=60;
		int mMin = c % 60;
			c/=60;
		int mHours = c % 24;

		ch->ChatPacket(CHAT_TYPE_INFO, "<Biyolog> Biyologa zaman ayirmak icin [%d H] [%d M] [%d S] beklemek zorundasiniz.", mHours, mMin, mSec);
		return;
	}

	for (int i=0; i<=9; i++){
		for (int j=1; j<=2; j++)
			ch->SetBiologState(pTableStringGlobal[j][i], 0);
	}

	ch->ChatPacket(CHAT_TYPE_INFO, "<Biolog> Zaman analizi basariyla kaldirildi, simdi analiz icin yeni bir nesne gonderebilirsiniz.");
	ch->SetQuestFlag(szBuf, get_global_time() + TIME_FOR_WAIT_TO_USE_AGAIN);
}
/*********************************************************************
* Initialization
*/
bool BiologManager::Send_FinishedRefresh(LPCHARACTER ch)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "BINARY_Biolog_Update %d %d %d %d", 0, 0, 0, 0);
}
/*********************************************************************
* Sending message as it is, failure, success and completed.
*/							
void SendMessage(LPCHARACTER ch, const char* pType)
{
	std::string szText("");

	if (!strcmp(pType, "Basarili"))
		szText = pTable_Translate[0];
	
	if (!strcmp(pType, "Basarisiz"))
		szText = pTable_Translate[1];
			
	if (!strcmp(pType, "Tamamlanan"))
		szText = pTable_Translate[2];
	
	boost::algorithm::replace_all(szText, " ", "$");

	char buf[512+1];
	snprintf(buf, sizeof(buf), "BINARY_Biolog_SendMessage %s", szText.c_str());
	ch->ChatPacket(CHAT_TYPE_COMMAND, buf);
}
/*********************************************************************
* Send messages to the client when you finish a degree biologist with arguments reward
*/
void SendMessagePopUp(LPCHARACTER ch, const char* iRewardType, int iRewardItem, const char* iBonusName_1, int iBonusValue_1, const char* iBonusName_2, int iBonusValue_2)
{
	std::string pRewardType = iRewardType;		boost::algorithm::replace_all(pRewardType, " ", "$");
	std::string pBonusName_1 = iBonusName_1;	boost::algorithm::replace_all(pBonusName_1, " ", "$");
	std::string pBonusName_2 = iBonusName_2;	boost::algorithm::replace_all(pBonusName_2, " ", "$");

	char buf[2048];
	snprintf(buf, sizeof(buf), "BINARY_Biolog_PopUp %s %d %s %d %s %d", pRewardType.c_str(), (int)iRewardItem, pBonusName_1.c_str(), (int)iBonusValue_1, pBonusName_2.c_str(), (int)iBonusValue_2);
	ch->ChatPacket(CHAT_TYPE_COMMAND, buf);
}
/*********************************************************************
* Send bonuses for biolog 30-90
*/
void BiologManager::SendBonusType(LPCHARACTER ch, int argument)
{
	switch (argument)
	{
		case 0:
			SendMessagePopUp(ch, pPopUp[argument][0], (int)pTableRewardItem[argument], pPopUp[argument][1], (int)tableBonus[argument][1], "$", (int)0);
			ch->AutoGiveItem(pTableRewardItem[argument]);	
			ch->AddAffect(pTableAffect[0], tableBonus[argument][0], tableBonus[argument][1], 0, unlimited, 0, true);
			break;
		case 1:
			SendMessagePopUp(ch, pPopUp[argument][0], (int)pTableRewardItem[argument], pPopUp[argument][1], (int)tableBonus[argument][1], "$", (int)0);
			ch->AutoGiveItem(pTableRewardItem[argument]);	
			ch->AddAffect(pTableAffect[1], tableBonus[argument][0], tableBonus[argument][1], 0, unlimited, 0, true);
			break;
		case 2:
			SendMessagePopUp(ch, pPopUp[argument][0], (int)pTableRewardItem[argument], pPopUp[argument][1], (int)tableBonus[argument][1], "$", (int)0);
			ch->AutoGiveItem(pTableRewardItem[argument]);	
			ch->AddAffect(pTableAffect[2], tableBonus[argument][0], tableBonus[argument][1], 0, unlimited, 0, true);
			break;
		case 3:
			SendMessagePopUp(ch, pPopUp[argument][0], (int)pTableRewardItem[argument], pPopUp[argument][1], (int)tableBonus[argument][1], "$", (int)0);
			ch->AutoGiveItem(pTableRewardItem[argument]);	
			ch->AddAffect(pTableAffect[3], tableBonus[argument][0], tableBonus[argument][1], 0, unlimited, 0, true);
			break;
		case 4:
			SendMessagePopUp(ch, pPopUp[argument][0], (int)pTableRewardItem[argument], pPopUp[argument][1], (int)tableBonus[argument][1], pPopUp[argument][2], (int)tableBonus[argument][3]);		
			ch->AutoGiveItem(pTableRewardItem[argument]);	
			ch->AddAffect(pTableAffect[4], tableBonus[argument][0], tableBonus[argument][1], 0, unlimited, 0, true);
			ch->AddAffect(pTableAffect[5], tableBonus[argument][2], tableBonus[argument][3], 0, unlimited, 0, true);
			break;
		case 5:
			SendMessagePopUp(ch, pPopUp[argument][0], (int)pTableRewardItem[argument], pPopUp[argument][1], (int)tableBonus[argument][1], pPopUp[argument][2], (int)tableBonus[argument][3]);		
			ch->AutoGiveItem(pTableRewardItem[argument]);	
			ch->AddAffect(pTableAffect[6], tableBonus[argument][0], tableBonus[argument][1], 0, unlimited, 0, true);
			ch->AddAffect(pTableAffect[7], tableBonus[argument][2], tableBonus[argument][3], 0, unlimited, 0, true);
			break;
		case 6:
			SendMessagePopUp(ch, pPopUp[argument][0], (int)pTableRewardItem[argument], pPopUp[argument][1], (int)tableBonus[argument][1], "$", (int)0); 
			ch->AutoGiveItem(pTableRewardItem[argument]);								
			ch->AddAffect(pTableAffect[8], tableBonus[argument][0], tableBonus[argument][1], 0, unlimited, 0, true);
			ch->AddAffect(pTableAffect[9], tableBonus[argument][2], tableBonus[argument][3], 0, unlimited, 0, true);
			ch->AddAffect(pTableAffect[10], tableBonus[argument][4], tableBonus[argument][5], 0, unlimited, 0, true);
			ch->AddAffect(pTableAffect[11], tableBonus[argument][6], tableBonus[argument][7], 0, unlimited, 0, true);
	#ifdef __YMIR_UPDATE_WOLFMAN__
			ch->AddAffect(pTableAffect[12], tableBonus[argument][8], tableBonus[argument][9], 0, unlimited, 0, true);
	#endif
			break;
		case 7:
			SendMessagePopUp(ch, pPopUp[argument][0], (int)pTableRewardItem[argument], pPopUp[argument][1], (int)tableBonus[argument][1], "$", (int)0);		
			ch->AutoGiveItem(pTableRewardItem[argument]);									
			ch->AddAffect(pTableAffect[13], tableBonus[argument][0], tableBonus[argument][1], 0, unlimited, 0, true);
			ch->AddAffect(pTableAffect[14], tableBonus[argument][2], tableBonus[argument][3], 0, unlimited, 0, true);
			ch->AddAffect(pTableAffect[15], tableBonus[argument][4], tableBonus[argument][5], 0, unlimited, 0, true);
			ch->AddAffect(pTableAffect[16], tableBonus[argument][6], tableBonus[argument][7], 0, unlimited, 0, true);
	#ifdef __YMIR_UPDATE_WOLFMAN__
			ch->AddAffect(pTableAffect[17], tableBonus[argument][8], tableBonus[argument][9], 0, unlimited, 0, true);
	#endif
			break;	
		}
}
/*********************************************************************
* Receiving bonuses when you select the box when finished 92-94 biologist.
*/	
void BiologManager::SelectBonusType(LPCHARACTER ch, const char* iReward)
{
	int pSelectReward[] =
	{
		(ch->GetQuestFlag("biolog_lv92.reward")), (ch->GetQuestFlag("biolog_lv92.reward"))
	};
	
	if (pSelectReward[0] != 1 && ch->GetBiologState(pTableStringGlobal[3][8]) == 1)
	{	
		if (!strcmp(iReward, "92_reward_1")){	
			ch->AutoGiveItem(pTableRewardItem[8]);
			ch->AddAffect(pTableAffect[18], tableBonus[8][0], tableBonus[8][1], 0, unlimited, 0, true);
			SendMessagePopUp(ch, pPopUp[8][0], (int)pTableRewardItem[8], pPopUp[8][1], (int)tableBonus[8][1], "$", (int)0);	}
				
		if (!strcmp(iReward, "92_reward_2")){	
			ch->AutoGiveItem(pTableRewardItem[8]);
			ch->AddAffect(pTableAffect[18], tableBonus[8][2], tableBonus[8][3], 0, unlimited, 0, true);
			SendMessagePopUp(ch, pPopUp[8][0], (int)pTableRewardItem[8], pPopUp[8][2], (int)tableBonus[8][3], "$", (int)0);	}
					
		if (!strcmp(iReward, "92_reward_3")){	
			ch->AutoGiveItem(pTableRewardItem[8]);
			ch->AddAffect(pTableAffect[18], tableBonus[8][4], tableBonus[8][5], 0, unlimited, 0, true);
			SendMessagePopUp(ch, pPopUp[8][0], (int)pTableRewardItem[8], pPopUp[8][3], (int)tableBonus[8][5], "$", (int)0);	
		}
		ch->SetBiologState("biolog_lv92.used_window", 1);	
	}	
		 
	if (pSelectReward[1] != 1 && ch->GetBiologState(pTableStringGlobal[3][9]) == 1)
	{						
		if (!strcmp(iReward, "94_reward_1")){	
			ch->AutoGiveItem(pTableRewardItem[9]);
			ch->AddAffect(pTableAffect[19], tableBonus[9][0], tableBonus[9][1], 0, unlimited, 0, true);
			SendMessagePopUp(ch, pPopUp[9][0], (int)pTableRewardItem[9], pPopUp[9][1], (int)tableBonus[9][1], "$", (int)0);	}
				
		if (!strcmp(iReward, "94_reward_2")){	
			ch->AutoGiveItem(pTableRewardItem[9]);
			ch->AddAffect(pTableAffect[19], tableBonus[9][2], tableBonus[9][3], 0, unlimited, 0, true);
			SendMessagePopUp(ch, pPopUp[9][0], (int)pTableRewardItem[9], pPopUp[9][2], (int)tableBonus[9][3], "$", (int)0); }

		if (!strcmp(iReward, "94_reward_3")){
			ch->AutoGiveItem(pTableRewardItem[9]);
			ch->AddAffect(pTableAffect[19], tableBonus[9][4], tableBonus[9][5], 0, unlimited, 0, true);
			SendMessagePopUp(ch, pPopUp[9][0], (int)pTableRewardItem[9], pPopUp[9][3], (int)tableBonus[9][5], "$", (int)0);
		}
		ch->SetBiologState("biolog_lv94.used_window", 1);	
	}
}
/*********************************************************************
* Open window to select reward biologist 92-94
*/	
void BiologManager::SendWindow_SelectType(LPCHARACTER ch, int iType)
{	
	int pGetCompleted[] = {(ch->GetQuestFlag("biolog_lv92.used_window")), (ch->GetQuestFlag("biolog_lv94.used_window"))};
	
	switch (iType)
	{
		case 1:
			if (pGetCompleted[0] != 1)	
			{	
				std::string arg1 = (const char*) pPopUp[8][0];		boost::algorithm::replace_all(arg1, " ", "$");
				std::string arg2 = (const char*) pPopUp[8][1];		boost::algorithm::replace_all(arg2, " ", "$");
				std::string arg3 = (const char*) pPopUp[8][2];		boost::algorithm::replace_all(arg3, " ", "$");	
				std::string arg4 = (const char*) pPopUp[8][3];		boost::algorithm::replace_all(arg4, " ", "$");		
				
				char buf[512+1];
				snprintf(buf, sizeof(buf), "BINARY_Biolog_SelectReward %d %s %s %d %s %d %s %d", (int)1, arg1.c_str(), arg2.c_str(), (int)tableBonus[8][1], arg3.c_str(), (int)tableBonus[8][3], arg4.c_str(), (int)tableBonus[8][5]);
				ch->ChatPacket(CHAT_TYPE_COMMAND, buf);	
			}
		break;
		case 2:	
			if (pGetCompleted[1] != 1)
			{	
				std::string arg1 = (const char*) pPopUp[9][0];		boost::algorithm::replace_all(arg1, " ", "$");
				std::string arg2 = (const char*) pPopUp[9][1];		boost::algorithm::replace_all(arg2, " ", "$");
				std::string arg3 = (const char*) pPopUp[9][2];		boost::algorithm::replace_all(arg3, " ", "$");
				std::string arg4 = (const char*) pPopUp[9][3];		boost::algorithm::replace_all(arg4, " ", "$");
				
				char buf[512+1];
				snprintf(buf, sizeof(buf), "BINARY_Biolog_SelectReward %d %s %s %d %s %d %s %d", (int)2, arg1.c_str(), arg2.c_str(), (int)tableBonus[8][1], arg3.c_str(), (int)tableBonus[8][3], arg4.c_str(), (int)tableBonus[8][5]);
				ch->ChatPacket(CHAT_TYPE_COMMAND, buf);		
			}
		break;
	}	
}
/*********************************************************************
* Function to display the send button count.
*/	
bool SendBiolog_Data(LPCHARACTER ch, int argument)
{
	int pTableString[3][10] =
	{
		{(ch->GetBiologState(pTableStringGlobal[0][0])), (ch->GetBiologState(pTableStringGlobal[0][1])), (ch->GetBiologState(pTableStringGlobal[0][2])),(ch->GetBiologState(pTableStringGlobal[0][3])), (ch->GetBiologState(pTableStringGlobal[0][4])), (ch->GetBiologState(pTableStringGlobal[0][5])),(ch->GetBiologState(pTableStringGlobal[0][6])), (ch->GetBiologState(pTableStringGlobal[0][7])),(ch->GetBiologState(pTableStringGlobal[0][8])), (ch->GetBiologState(pTableStringGlobal[0][9]))}, {(ch->GetBiologState(pTableStringGlobal[1][0])), (ch->GetBiologState(pTableStringGlobal[1][1])), (ch->GetBiologState(pTableStringGlobal[1][2])),(ch->GetBiologState(pTableStringGlobal[1][3])), (ch->GetBiologState(pTableStringGlobal[1][4])), (ch->GetBiologState(pTableStringGlobal[1][5])),(ch->GetBiologState(pTableStringGlobal[1][6])), (ch->GetBiologState(pTableStringGlobal[1][7])),(ch->GetBiologState(pTableStringGlobal[1][8])), (ch->GetBiologState(pTableStringGlobal[1][9]))}, {(ch->GetBiologState(pTableStringGlobal[2][0])), (ch->GetBiologState(pTableStringGlobal[2][1])), (ch->GetBiologState(pTableStringGlobal[2][2])),(ch->GetBiologState(pTableStringGlobal[2][3])), (ch->GetBiologState(pTableStringGlobal[2][4])), (ch->GetBiologState(pTableStringGlobal[2][5])),(ch->GetBiologState(pTableStringGlobal[2][6])), (ch->GetBiologState(pTableStringGlobal[2][7])),(ch->GetBiologState(pTableStringGlobal[2][8])), (ch->GetBiologState(pTableStringGlobal[2][9]))}
	};							

	int iPass_percent = pTablePercentage[1];
	int iRandom = number(1, 100);	
	
	if (ch->GetLevel() < pTableLevel[argument])
	{
		ch->ChatPacket(CHAT_TYPE_NOTICE, pTable_Translate[5], pTableLevel[argument]);
		return false;		
	}	
	
	if (init_biologTime() < pTableString[2][argument] + pTableTime[argument])
	{
		int amount = pTableString[1][argument] - init_biologTime();
		
		int secs=amount%60;
			amount/=60;
		int mins=amount%60;
			amount/=60;
		int hours=amount%24;
		int days=amount/24;

		ch->ChatPacket(CHAT_TYPE_NOTICE, pTable_Translate[3], days, hours, mins, secs);
		return false;
	}

	if (ch->CountSpecifyItem(pTableVnum[argument]) > 0)
	{	
		if (ch->CountSpecifyItem(pTablePercentage[0]) > 0)
		{
			iPass_percent = pTablePercentage[2];
			ch->RemoveSpecifyItem(pTablePercentage[0], 1);
		}
			
		if (iRandom <= iPass_percent)
		{
			if (pTableString[0][argument] < pTableCountMax[argument] - 1)
			{
				SendMessage(ch, "Basarili");
				ch->SetBiologState(pTableStringGlobal[0][argument], pTableString[0][argument] + 1);
				ch->SetBiologState(pTableStringGlobal[1][argument], init_biologTime() + pTableTime[argument]);
				ch->SetBiologState(pTableStringGlobal[2][argument], init_biologTime());
				ch->RemoveSpecifyItem(pTableVnum[argument], 1);
				BiologManager::instance().SendUpdate_Binary(ch);
				return false;
			}
				if (argument <= 7)
				{
					SendMessage(ch, "Tamamlanan"); }
				
				ch->SetBiologState(pTableStringGlobal[0][argument], pTableString[0][argument] + 1);
				ch->SetBiologState(pTableStringGlobal[1][argument], 0);
				ch->SetBiologState(pTableStringGlobal[2][argument], 0);
				ch->SetBiologState(pTableStringGlobal[3][argument], 1);
				ch->RemoveSpecifyItem(pTableVnum[argument], 1);		
				BiologManager::instance().SendUpdate_Binary(ch);
				
				if (argument <= 7)
				{
					BiologManager::instance().SendBonusType(ch, argument); }
				
				if (argument == 8)
				{
					BiologManager::instance().SendWindow_SelectType(ch, 1); }
					
				if (argument == 9)
				{
					BiologManager::instance().SendWindow_SelectType(ch, 2);
					BiologManager::instance().Send_FinishedRefresh(ch); }
				
				return false;
		}
			SendMessage(ch, "Basarisiz");
			ch->RemoveSpecifyItem(pTableVnum[argument], 1);
			return false;
	}
	else
		ch->ChatPacket(CHAT_TYPE_NOTICE, pTable_Translate[4]);
}	
/*********************************************************************
* Function to display the send button count. - first check after send in SendBiolog_Data
*/	
bool BiologManager::SendButton(LPCHARACTER ch)
{
	int pTableString[3][10] =
	{
		{(ch->GetBiologState(pTableStringGlobal[0][0])), (ch->GetBiologState(pTableStringGlobal[0][1])), (ch->GetBiologState(pTableStringGlobal[0][2])),(ch->GetBiologState(pTableStringGlobal[0][3])), (ch->GetBiologState(pTableStringGlobal[0][4])), (ch->GetBiologState(pTableStringGlobal[0][5])),(ch->GetBiologState(pTableStringGlobal[0][6])), (ch->GetBiologState(pTableStringGlobal[0][7])),(ch->GetBiologState(pTableStringGlobal[0][8])), (ch->GetBiologState(pTableStringGlobal[0][9]))}, {(ch->GetBiologState(pTableStringGlobal[1][0])), (ch->GetBiologState(pTableStringGlobal[1][1])), (ch->GetBiologState(pTableStringGlobal[1][2])),(ch->GetBiologState(pTableStringGlobal[1][3])), (ch->GetBiologState(pTableStringGlobal[1][4])), (ch->GetBiologState(pTableStringGlobal[1][5])),(ch->GetBiologState(pTableStringGlobal[1][6])), (ch->GetBiologState(pTableStringGlobal[1][7])),(ch->GetBiologState(pTableStringGlobal[1][8])), (ch->GetBiologState(pTableStringGlobal[1][9]))}, {(ch->GetBiologState(pTableStringGlobal[2][0])), (ch->GetBiologState(pTableStringGlobal[2][1])), (ch->GetBiologState(pTableStringGlobal[2][2])),(ch->GetBiologState(pTableStringGlobal[2][3])), (ch->GetBiologState(pTableStringGlobal[2][4])), (ch->GetBiologState(pTableStringGlobal[2][5])),(ch->GetBiologState(pTableStringGlobal[2][6])), (ch->GetBiologState(pTableStringGlobal[2][7])),(ch->GetBiologState(pTableStringGlobal[2][8])), (ch->GetBiologState(pTableStringGlobal[2][9]))}
	};	
	
	if (ch->GetBiologState(pTableStringGlobal[3][9]) == 1)
	{ 
		ch->ChatPacket(CHAT_TYPE_NOTICE, pTable_Translate[6]);
		return false;
	}

	if (pTableString[0][0] < pTableCountMax[0])
	{
		SendBiolog_Data(ch, 0);
		return false;
	}		
		
	for (int btn = 1; btn < 10; btn++)
	{	
		if (pTableString[0][btn] < pTableCountMax[btn])
		{
			SendBiolog_Data(ch, btn);
			break;
		}	
	}	
}
/*********************************************************************
* Update in binary data.
*/	
bool BiologManager::SendUpdate_Binary(LPCHARACTER ch) 
{				
	int pTableString[3][10] =
	{
		{(ch->GetBiologState(pTableStringGlobal[0][0])), (ch->GetBiologState(pTableStringGlobal[0][1])), (ch->GetBiologState(pTableStringGlobal[0][2])),(ch->GetBiologState(pTableStringGlobal[0][3])), (ch->GetBiologState(pTableStringGlobal[0][4])), (ch->GetBiologState(pTableStringGlobal[0][5])),(ch->GetBiologState(pTableStringGlobal[0][6])), (ch->GetBiologState(pTableStringGlobal[0][7])),(ch->GetBiologState(pTableStringGlobal[0][8])), (ch->GetBiologState(pTableStringGlobal[0][9]))}, {(ch->GetBiologState(pTableStringGlobal[1][0])), (ch->GetBiologState(pTableStringGlobal[1][1])), (ch->GetBiologState(pTableStringGlobal[1][2])),(ch->GetBiologState(pTableStringGlobal[1][3])), (ch->GetBiologState(pTableStringGlobal[1][4])), (ch->GetBiologState(pTableStringGlobal[1][5])),(ch->GetBiologState(pTableStringGlobal[1][6])), (ch->GetBiologState(pTableStringGlobal[1][7])),(ch->GetBiologState(pTableStringGlobal[1][8])), (ch->GetBiologState(pTableStringGlobal[1][9]))}, {(ch->GetBiologState(pTableStringGlobal[2][0])), (ch->GetBiologState(pTableStringGlobal[2][1])), (ch->GetBiologState(pTableStringGlobal[2][2])),(ch->GetBiologState(pTableStringGlobal[2][3])), (ch->GetBiologState(pTableStringGlobal[2][4])), (ch->GetBiologState(pTableStringGlobal[2][5])),(ch->GetBiologState(pTableStringGlobal[2][6])), (ch->GetBiologState(pTableStringGlobal[2][7])),(ch->GetBiologState(pTableStringGlobal[2][8])), (ch->GetBiologState(pTableStringGlobal[2][9]))}
	};

	for (int count = 0; count < _countof(pxCountTable); count++)
	{
		if (ch->GetBiologState(pxCountTable[count]) < 1)
		{
			BiologManager::instance().Send_FinishedRefresh(ch);
		}
	}

	for (int grade = 0; grade < 9; grade++)
	{	
		if (ch->GetBiologState(pTableStringGlobal[3][0]) != 1)
		{
			ch->ChatPacket(CHAT_TYPE_COMMAND, "BINARY_Biolog_Update %d %d %d %d", pTableString[1][0] - init_biologTime(), pTableString[0][0], pTableCountMax[0], pTableVnum[0]);
		}
		
		if (ch->GetBiologState(pTableStringGlobal[3][grade]) == 1)
		{
			ch->ChatPacket(CHAT_TYPE_COMMAND, "BINARY_Biolog_Update %d %d %d %d", pTableString[1][grade+1] - init_biologTime(), pTableString[0][grade+1], pTableCountMax[grade+1], pTableVnum[grade+1]);
		}		
	}	
}

