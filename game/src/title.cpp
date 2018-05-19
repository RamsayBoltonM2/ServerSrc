/*********************************************************************
* date        : 2016.07.16
* file        : title.cpp
* author      : VegaS
* description : 
*/
#include "stdafx.h"
#include "constants.h"
#include "utils.h"
#include "desc.h"
#include "char.h"
#include "db.h"
#include "config.h"
#include "title.h"
#include "affect.h"
#include "item.h"
#include "questmanager.h"
/*********************************************************************
*/
TitleManager::TitleManager()
{
}

TitleManager::~TitleManager()
{
}
/*****************************
* Set requirements gold(yang) for title
*/
#define need_gold_1		500000
#define need_gold_2		800000
#define need_gold_3		1500000
#define need_gold_4		1800000
#define need_gold_5		2500000
#define need_gold_6		3000000
#define need_gold_7		4000000
#define need_gold_8		5000000
#define need_gold_9		10000000
#define need_gold_10	15000000
#define need_gold_11	25000000
#define need_gold_12	50000000
#define need_gold_13	75000000
#define need_gold_14	100000000
#define need_gold_15	150000000
#define need_gold_16	300000000

/*****************************
* Set requirements level for title
*/
#define need_level_1	7
#define need_level_2	14
#define need_level_3	21
#define need_level_4	28
#define need_level_5	35
#define need_level_6	42
#define need_level_7	49
#define need_level_8	56
#define need_level_9	63
#define need_level_10	70
#define need_level_11	77
#define need_level_12	84	
#define need_level_13	91
#define need_level_14	98
#define need_level_15	100
#define need_level_16	103	

/*****************************
* Set requirements level for playTime on character
*/
#define need_minutes_1	5
#define need_minutes_2	15
#define need_minutes_3	40
#define need_minutes_4	50													
#define need_minutes_5	60
#define need_minutes_6	75
#define need_minutes_7	90
#define need_minutes_8	110
#define need_minutes_9	130
#define need_minutes_10	150
#define need_minutes_11	170
#define need_minutes_12	200
#define need_minutes_13	500
#define need_minutes_14	700
#define need_minutes_15	1200
#define need_minutes_16	1500	
#include <boost/unordered_map.hpp>
#include "../../common/stl.h"
/****************************************************************************************************************************************************************************************
* Get enable options
*/
#define ENABLE_CHECK_TIME_POTIONS // If you want to make enable check for buy potions with time
#define ENABLE_GIVE_JCOINS // Enable to give you jcoins loadead from account.account(jcoins [row]) example: if u buy a potion yeelow cost 50coins, when you buy give make: -50 coins and give +50 jcoins

int sTitle[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19}; // Do not change anything here

int x[] = {0, 0, 0}; // Do not change anything here
/****************
* Get type affect on title premium loaded from affect.h ( not change this )
*/	
int get_affect_premium[] = {PRESTIGE_AFFECT_1, PRESTIGE_AFFECT_2, PRESTIGE_AFFECT_3};

/****************
* Settings how hours you need to wait for can buy again potion on shopping-area 
*/	
int sPotionTime[] = {12, 12, 12};
/****************
* ItemVnum for potions to check if u have for attached title premium 1/2/3 
*/	
int sPotionVnum[] = {55001, 55002, 55003};
/****************
* Price potions in coins value loaded from account.account for potion yellow/pink/white
*/	
int sPotionPrice[] = {50, 100, 150};

int	sTimeDuratingBonus[] = {60*60*24*365};	
/****************
* Table with bonus name affect ( if u want to change bonus you can find in enum EPointTypes on char.h other bonus type )
* If u want you can put automatically value from bonus like - POINTS_ATTBONUS_MONSTER = 63 and , 20 is value for bonus monster apply
*/	
int pTitleBonus_1[] = {POINT_MAX_HP, 3000};
int pTitleBonus_2[] = {POINT_ATTBONUS_MONSTER, 20};
int pTitleBonus_3[] = {POINT_CRITICAL_PCT, 15};

const char* sPotionFlag[] = {"title.getTimePotions_1", "title.getTimePotions_2", "title.getTimePotions_3"}; // Do not change anything here

/****************
* Settings for translate
*/	
const char* requirements[] = {
									"[Title System] You must have at least %u yang for this title!", 
									"[Title System] You must have a minimum level of %u for this title!",
									"[Title System] You must have minutes played on character: %u"};							
const char* title_translate[] = {
									"[Title System] You need to wait %u hour for can buy again a potion!",
									"[Title System] You do not have enough coins in your account dragon! Please charge it!",
									"[Title System] The bonus premium received the title disappeared !",
									"[Title System] Your title has been removed!, received x1 Yellow Potion Enchanted!",
									"[Title System] Your title has been removed!, received x1 Pink Potion Enchanted!",							
									"[Title System] Your title has been removed!, received x1 White Potion Enchanted!",
									"[Title System] You do not have a premium title for transforms!",
									"[Title System] You have a title premium attached, you cannot make that!",
									"[Title System] Already you have this title!",
									"[Title System] You must have a x1 Yellow Potion Enchanted!",
									"[Title System] You must have a x1 Pink Potion Enchanted!",
									"[Title System] You must have a x1 White Potion Enchanted!",
									"[Title System] Your title has been removed !"};						

bool TitleManager::TransformTitle(LPCHARACTER ch)
{
	if (NULL == ch)
		return false;

	if (!ch->IsPC())
		return false;
	
	switch (ch->GetRealTitle())
	{
		case 17:	
			if (ch->FindAffect(get_affect_premium[0]))
			{
				ch->ChatPacket(CHAT_TYPE_NOTICE, title_translate[2]);		
				ch->ChatPacket(CHAT_TYPE_NOTICE, title_translate[3]);
				ch->RemoveAffect(get_affect_premium[0]);	
				ch->UpdateTitle(- ch->GetRealTitle());
				ch->AutoGiveItem(sPotionVnum[0]);	
			}
			else if (!ch->FindAffect(get_affect_premium[0])){ 
				return false; ch->ChatPacket(CHAT_TYPE_NOTICE, title_translate[6]);}
				break;
		case 18:	
			if (ch->FindAffect(get_affect_premium[1]))
			{
				ch->ChatPacket(CHAT_TYPE_NOTICE, title_translate[2]);		
				ch->ChatPacket(CHAT_TYPE_NOTICE, title_translate[4]);
				ch->RemoveAffect(get_affect_premium[1]);	
				ch->UpdateTitle(- ch->GetRealTitle());
				ch->AutoGiveItem(sPotionVnum[1]);	
			}
			else if (!ch->FindAffect(get_affect_premium[0])){ 
				return false; ch->ChatPacket(CHAT_TYPE_NOTICE, title_translate[6]);}
				break;
		case 19:	
			if (ch->FindAffect(get_affect_premium[2]))
			{
				ch->ChatPacket(CHAT_TYPE_NOTICE, title_translate[2]);		
				ch->ChatPacket(CHAT_TYPE_NOTICE, title_translate[5]);
				ch->RemoveAffect(get_affect_premium[2]);	
				ch->UpdateTitle(- ch->GetRealTitle());
				ch->AutoGiveItem(sPotionVnum[2]);	
			}
			else if (!ch->FindAffect(get_affect_premium[0])){ 
				return false; ch->ChatPacket(CHAT_TYPE_NOTICE, title_translate[6]);}
				break;
		}		
}		
									
void TitleManager::BuyPotion(LPCHARACTER ch, const char* mPotion)
{
	if (NULL == ch)
		return;

	if (!ch->IsPC())
		return;

	if (!*mPotion)
	{	
		return;
	}
/****************
* Argument first
*/
	if (!strcmp(mPotion, "buy_potion_1")) 
	{	
		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT coins FROM account.account WHERE id = %u", ch->GetDesc()->GetAccountTable().id));
		quest::PC* pPC = quest::CQuestManager::instance().GetPC(ch->GetPlayerID());	
		if (pMsg->Get()->uiNumRows == x[0])	return;
			MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
			str_to_number(x[0], row[0]);
	#ifdef ENABLE_CHECK_TIME_POTIONS		
		int szTime = pPC->GetFlag(sPotionFlag[0]);
		if (get_global_time() - szTime < sPotionTime[0] * 60 * 60)	
		{	
			ch->ChatPacket(CHAT_TYPE_NOTICE, title_translate[0], sPotionTime[2]);	return;	
		}		
	#endif	
		if (x[0] < sPotionPrice[0])	{
			ch->ChatPacket(CHAT_TYPE_NOTICE, title_translate[1]);	return;	}
		else
			ch->AutoGiveItem(sPotionVnum[0]);
			DBManager::instance().DirectQuery("UPDATE account.account SET coins = coins - %u WHERE id = %u", sPotionPrice[0], ch->GetDesc()->GetAccountTable().id);
		#ifdef ENABLE_CHECK_TIME_POTIONS		
			pPC->SetFlag(sPotionFlag[0], get_global_time());
		#endif		
		
		#ifdef ENABLE_GIVE_JCOINS
			DBManager::instance().DirectQuery("UPDATE account.account SET jcoins = jcoins + %u WHERE id = %u", sPotionPrice[0], ch->GetDesc()->GetAccountTable().id);	
		#endif			
		}
//}		
/****************
* Argument two
*/
	if (!strcmp(mPotion, "buy_potion_2"))
	{
		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT coins FROM account.account WHERE id = %u", ch->GetDesc()->GetAccountTable().id));
		quest::PC* pPC = quest::CQuestManager::instance().GetPC(ch->GetPlayerID());	
		if (pMsg->Get()->uiNumRows == x[1])	return;
			MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
			str_to_number(x[1], row[0]);
	#ifdef ENABLE_CHECK_TIME_POTIONS		
		int szTime = pPC->GetFlag(sPotionFlag[0]);
		if (get_global_time() - szTime < sPotionTime[0] * 60 * 60)	
		{	
			ch->ChatPacket(CHAT_TYPE_NOTICE, title_translate[0], sPotionTime[2]);	return;	
		}		
	#endif	
		if (x[1] < sPotionPrice[1])	{
			ch->ChatPacket(CHAT_TYPE_NOTICE, title_translate[1]);	return;	}
		else
			ch->AutoGiveItem(sPotionVnum[1]);
			DBManager::instance().DirectQuery("UPDATE account.account SET coins = coins - %u WHERE id = %u", sPotionPrice[1], ch->GetDesc()->GetAccountTable().id);
		#ifdef ENABLE_CHECK_TIME_POTIONS		
			pPC->SetFlag(sPotionFlag[1], get_global_time());
		#endif		
		
		#ifdef ENABLE_GIVE_JCOINS
			DBManager::instance().DirectQuery("UPDATE account.account SET jcoins = jcoins + %u WHERE id = %u", sPotionPrice[1], ch->GetDesc()->GetAccountTable().id);	
		#endif		
	}	
//}	
/****************
* Argument three
*/	
	if (!strcmp(mPotion, "buy_potion_3"))
	{
		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT coins FROM account.account WHERE id = %u", ch->GetDesc()->GetAccountTable().id));
		quest::PC* pPC = quest::CQuestManager::instance().GetPC(ch->GetPlayerID());	
		if (pMsg->Get()->uiNumRows == x[2])	return;
			MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
			str_to_number(x[2], row[0]);
	#ifdef ENABLE_CHECK_TIME_POTIONS		
		int szTime = pPC->GetFlag(sPotionFlag[0]);
		if (get_global_time() - szTime < sPotionTime[0] * 60 * 60)	
		{	
			ch->ChatPacket(CHAT_TYPE_NOTICE, title_translate[0], sPotionTime[2]);	return;	
		}		
	#endif	
		if (x[2] < sPotionPrice[2])	{
			ch->ChatPacket(CHAT_TYPE_NOTICE, title_translate[1]);	return;	}
		else
			ch->AutoGiveItem(sPotionVnum[2]);
			DBManager::instance().DirectQuery("UPDATE account.account SET coins = coins - %u WHERE id = %u", sPotionPrice[2], ch->GetDesc()->GetAccountTable().id);
		#ifdef ENABLE_CHECK_TIME_POTIONS		
			pPC->SetFlag(sPotionFlag[0], get_global_time());
		#endif		
		
		#ifdef ENABLE_GIVE_JCOINS
			DBManager::instance().DirectQuery("UPDATE account.account SET jcoins = jcoins + %u WHERE id = %u", sPotionPrice[2], ch->GetDesc()->GetAccountTable().id);	
		#endif	
	}
}	

static bool GetPlayTime(LPCHARACTER ch, int sTime)
{
	if (NULL == ch)
		return false;

	if (!ch->IsPC())
		return false;
	
	if (ch->GetRealPoint(POINT_PLAYTIME) < sTime)
	{
		switch (sTime)
		{
			case need_minutes_1:		case need_minutes_2:	case need_minutes_3:	case need_minutes_4:
			case need_minutes_5:		case need_minutes_6:	case need_minutes_7:	case need_minutes_8:		
			case need_minutes_9:		case need_minutes_10:	case need_minutes_11:	case need_minutes_12:		
			case need_minutes_13:		case need_minutes_14:	case need_minutes_15:	case need_minutes_16:
				ch->ChatPacket(CHAT_TYPE_NOTICE, requirements[2], sTime);
				return false;	
		}
	}
		return true;
}	
	
static bool GetLevel(LPCHARACTER ch, int sLevel)
{
	if (NULL == ch)
		return false;

	if (!ch->IsPC())
		return false;
	
	if (ch->GetLevel() < sLevel)
	{
		switch (sLevel)
		{
			case need_level_1:		case need_level_2:	case need_level_3:	case need_level_4:
			case need_level_5:		case need_level_6:	case need_level_7:	case need_level_8:		
			case need_level_9:		case need_level_10:	case need_level_11:	case need_level_12:		
			case need_level_13:		case need_level_14:	case need_level_15:	case need_level_16:
				ch->ChatPacket(CHAT_TYPE_NOTICE, requirements[1], sLevel);
				return false;	
		}
	}
		return true;
}

static bool GetMoney(LPCHARACTER ch, int sGold)
{
	if (NULL == ch)
		return false;

	if (!ch->IsPC())
		return false;
	
	if (ch->GetGold() < sGold)
	{
		switch (sGold)
		{
			case need_gold_1:	case need_gold_2:	case need_gold_3:	case need_gold_4:
			case need_gold_5:	case need_gold_6:	case need_gold_7:	case need_gold_8:
			case need_gold_9:	case need_gold_10:	case need_gold_11:	case need_gold_12:
			case need_gold_13:	case need_gold_14:	case need_gold_15:	case need_gold_16:
				ch->ChatPacket(CHAT_TYPE_NOTICE, requirements[0], sGold);
				return false;	
		}
	}
		return true;
}

static bool GetTitlePremium(LPCHARACTER ch)
{
	if (NULL == ch)
		return false;

	if (!ch->IsPC())
		return false;
	
	switch (ch->GetRealTitle())
	{
		case 17:	
		case 18:
		case 19:
			ch->ChatPacket(CHAT_TYPE_NOTICE, title_translate[7]);	
			return false;
	}
		return true;	
}

static bool GetTitleActual(LPCHARACTER ch, int arg1)
{
	if (NULL == ch)
		return false;

	if (!ch->IsPC())
		return false;
	
	if (ch->GetRealTitle() == arg1)
	{		
		switch (arg1)
		{
			case 1:		case 2:		case 3:		case 4:	
			case 5:		case 6:		case 7:		case 8:	
			case 9:		case 10:	case 11:	case 12:	
			case 13:	case 14:	case 15:	case 16:	
			case 17:	case 18:	case 19:			
				ch->ChatPacket(CHAT_TYPE_NOTICE, title_translate[8]);
				return false;
		}
	}
		return true;	
}

bool TitleManager::UpdateTitle(LPCHARACTER ch, int changeTitle, int changeMoney)
{
	if (changeTitle > 0 && changeMoney > 0)
	{
		ch->UpdateTitle(changeTitle - ch->GetTitle());
		ch->PointChange(POINT_GOLD, - changeMoney);	
	}
	else if (changeTitle > 0 && changeMoney < 1)
	{
		ch->UpdateTitle(changeTitle - ch->GetTitle());
	}	
}

void TitleManager::SetAffect(LPCHARACTER ch, const char* valueAffect)
{
	if (NULL == ch)
		return;

	if (!ch->IsPC())
		return;
/****************
* [Premium Title I]
*/				
	if (!strcmp(valueAffect, "send_premium_1"))
	{
		if (ch->CountSpecifyItem(sPotionVnum[0]))
		{	
			ch->AddAffect(get_affect_premium[0], pTitleBonus_1[0], pTitleBonus_1[1], 0, sTimeDuratingBonus[0], 0, true);
			ch->RemoveSpecifyItem(sPotionVnum[0], 1);	
			UpdateTitle(ch, sTitle[17], 0);	
		}		
		else	
			ch->ChatPacket(CHAT_TYPE_NOTICE, title_translate[9]);
			return;	
	}	
/****************
* [Premium Title II]
*/		
	if (!strcmp(valueAffect, "send_premium_2"))
	{
		if (ch->CountSpecifyItem(sPotionVnum[1]))
		{		
			ch->AddAffect(get_affect_premium[1], pTitleBonus_2[0], pTitleBonus_2[1], 0, sTimeDuratingBonus[0], 0, true);
			ch->RemoveSpecifyItem(sPotionVnum[1], 1);	
			UpdateTitle(ch, sTitle[18], 0);
		}		
		else	
			ch->ChatPacket(CHAT_TYPE_NOTICE, title_translate[10]);
			return;	
	}	
/****************
* [Premium Title III]
*/		
	if (!strcmp(valueAffect, "send_premium_3"))
	{
		if (ch->CountSpecifyItem(sPotionVnum[2]))
		{		
				ch->AddAffect(get_affect_premium[2], pTitleBonus_3[0], pTitleBonus_3[1], 0, sTimeDuratingBonus[0], 0, true);
				ch->RemoveSpecifyItem(sPotionVnum[2], 1);	
				UpdateTitle(ch, sTitle[19], 0);
		}		
		else	
			ch->ChatPacket(CHAT_TYPE_NOTICE, title_translate[11]);
			return;	
	}	
}	
bool TitleManager::SetTitle(LPCHARACTER ch, const char* pTitle)
{
	if (NULL == ch)
		return false;

	if (!ch->IsPC())
		return false;

	if (!*pTitle)
	{	
		return false;
	}
/****************
* Title  free
*/
	if (!strcmp(pTitle, "disable")	&& (GetTitleActual(ch, sTitle[17]) == true) && (GetTitleActual(ch, sTitle[18]) == true) && (GetTitleActual(ch, sTitle[19]) == true))	
	{		
		ch->UpdateTitle(- ch->GetTitle());	
		ch->ChatPacket(CHAT_TYPE_NOTICE, title_translate[12]);
	}
	
	if (!strcmp(pTitle, "title1") && (GetLevel(ch, need_level_1) == true) && (GetPlayTime(ch, need_minutes_1) == true) && (GetMoney(ch, need_gold_1) == true) && (GetTitlePremium(ch) == true) && (GetTitleActual(ch, sTitle[1]) == true))	
	{	
			UpdateTitle(ch, sTitle[1], need_gold_1);
	}
	
	if (!strcmp(pTitle, "title2") && (GetLevel(ch, need_level_2) == true) && (GetPlayTime(ch, need_minutes_2) == true) && (GetMoney(ch, need_gold_2) == true) && (GetTitlePremium(ch) == true) && (GetTitleActual(ch, sTitle[2]) == true))	
	{
			UpdateTitle(ch, sTitle[2], need_gold_2);
	}	
	
	if (!strcmp(pTitle, "title3") && (GetLevel(ch, need_level_3) == true) && (GetPlayTime(ch, need_minutes_3) == true) && (GetMoney(ch, need_gold_3) == true) && (GetTitlePremium(ch) == true) && (GetTitleActual(ch, sTitle[3]) == true))	
	{		
			UpdateTitle(ch, sTitle[3], need_gold_3);
	}	
	
	if (!strcmp(pTitle, "title4") && (GetLevel(ch, need_level_4) == true) && (GetPlayTime(ch, need_minutes_4) == true) && (GetMoney(ch, need_gold_4) == true) && (GetTitlePremium(ch) == true) && (GetTitleActual(ch, sTitle[4]) == true))	
	{
			UpdateTitle(ch, sTitle[4], need_gold_4);
	}	
	
	if (!strcmp(pTitle, "title5") && (GetLevel(ch, need_level_5) == true) && (GetPlayTime(ch, need_minutes_5) == true) && (GetMoney(ch, need_gold_5) == true) && (GetTitlePremium(ch) == true) && (GetTitleActual(ch, sTitle[5]) == true))	
	{
			UpdateTitle(ch, sTitle[5], need_gold_5);
	}	
	
	if (!strcmp(pTitle, "title6") && (GetLevel(ch, need_level_6) == true) && (GetPlayTime(ch, need_minutes_6) == true) && (GetMoney(ch, need_gold_6) == true) && (GetTitlePremium(ch) == true) && (GetTitleActual(ch, sTitle[6]) == true))	
	{
			UpdateTitle(ch, sTitle[6], need_gold_6);
	}
	
	if (!strcmp(pTitle, "title7") && (GetLevel(ch, need_level_7) == true) && (GetPlayTime(ch, need_minutes_7) == true) && (GetMoney(ch, need_gold_7) == true) && (GetTitlePremium(ch) == true) && (GetTitleActual(ch, sTitle[7]) == true))	
	{
			UpdateTitle(ch, sTitle[7], need_gold_7);
	}	
	
	if (!strcmp(pTitle, "title8") && (GetLevel(ch, need_level_8) == true) && (GetPlayTime(ch, need_minutes_8) == true) && (GetMoney(ch, need_gold_8) == true) && (GetTitlePremium(ch) == true) && (GetTitleActual(ch, sTitle[8]) == true))	
	{
			UpdateTitle(ch, sTitle[8], need_gold_8);
	}
	
	if (!strcmp(pTitle, "title9") && (GetLevel(ch, need_level_9) == true) && (GetPlayTime(ch, need_minutes_9) == true) && (GetMoney(ch, need_gold_9) == true) && (GetTitlePremium(ch) == true) && (GetTitleActual(ch, sTitle[9]) == true))	
	{
			UpdateTitle(ch, sTitle[9], need_gold_9);
	}	
	
	if (!strcmp(pTitle, "title10") && (GetLevel(ch, need_level_10) == true) && (GetPlayTime(ch, need_minutes_10) == true) && (GetMoney(ch, need_gold_10) == true) && (GetTitlePremium(ch) == true) && (GetTitleActual(ch, sTitle[10]) == true))	
	{
			UpdateTitle(ch, sTitle[10], need_gold_10);
	}
	
	if (!strcmp(pTitle, "title11") && (GetLevel(ch, need_level_11) == true) && (GetPlayTime(ch, need_minutes_11) == true) && (GetMoney(ch, need_gold_11) == true) && (GetTitlePremium(ch) == true) && (GetTitleActual(ch, sTitle[11]) == true))	
	{
			UpdateTitle(ch, sTitle[11], need_gold_11);
	}	
	
	if (!strcmp(pTitle, "title12") && (GetLevel(ch, need_level_12) == true) && (GetPlayTime(ch, need_minutes_12) == true) && (GetMoney(ch, need_gold_12) == true) && (GetTitlePremium(ch) == true) && (GetTitleActual(ch, sTitle[12]) == true))	
	{
			UpdateTitle(ch, sTitle[12], need_gold_12);
	}
	
	if (!strcmp(pTitle, "title13") && (GetLevel(ch, need_level_13) == true) && (GetPlayTime(ch, need_minutes_13) == true) && (GetMoney(ch, need_gold_13) == true) && (GetTitlePremium(ch) == true) && (GetTitleActual(ch, sTitle[13]) == true))	
	{
			UpdateTitle(ch, sTitle[13], need_gold_13);
	}	
	
	if (!strcmp(pTitle, "title14") && (GetLevel(ch, need_level_14) == true) && (GetPlayTime(ch, need_minutes_14) == true) && (GetMoney(ch, need_gold_14) == true) && (GetTitlePremium(ch) == true) && (GetTitleActual(ch, sTitle[14]) == true))	
	{
			UpdateTitle(ch, sTitle[14], need_gold_14);
	}	
	
	if (!strcmp(pTitle, "title15") && (GetLevel(ch, need_level_15) == true) && (GetPlayTime(ch, need_minutes_15) == true) && (GetMoney(ch, need_gold_15) == true) && (GetTitlePremium(ch) == true) && (GetTitleActual(ch, sTitle[15]) == true))	
	{
			UpdateTitle(ch, sTitle[15], need_gold_15);
	}	
	
	if (!strcmp(pTitle, "title16") && (GetLevel(ch, need_level_16) == true) && (GetPlayTime(ch, need_minutes_16) == true) && (GetMoney(ch, need_gold_16) == true) && (GetTitlePremium(ch) == true) && (GetTitleActual(ch, sTitle[16]) == true))	
	{
			UpdateTitle(ch, sTitle[16], need_gold_16);
	}
/****************
* Title  premium
*/
	if (!strcmp(pTitle, "title17") && (GetTitlePremium(ch) == true))
	{
			SetAffect(ch, "send_premium_1");
	}	
	
	if (!strcmp(pTitle, "title18") && (GetTitlePremium(ch) == true))	
	{
			SetAffect(ch, "send_premium_2");
	}	
	
	if (!strcmp(pTitle, "title19") && (GetTitlePremium(ch) == true))	
	{
			SetAffect(ch, "send_premium_3");
	}		
}