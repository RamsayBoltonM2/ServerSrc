/*********************************************************************
* date        : 2016.05.03
* file        : reborn.cpp
* author      : VegaS
* version	  : 0.1.1
*/
#include "stdafx.h"
#include "constants.h"
#include "utils.h"
#include "desc.h"
#include "char.h"
#include "db.h"
#include "config.h"
#include "reborn.h"
#include "affect.h"
#include "start_position.h"
#include "item_manager.h"


#define __YMIR_UPDATE_WOLFMAN__
#define UNLIMITED 60*60*60*365

RebornManager::RebornManager()
{
}

RebornManager::~RebornManager()
{
}
/*********
* Table with bonus name affect ( if u want to change bonus you can find in enum EPointTypes on char.h other bonus type )
*/
int affect_name[] = {POINT_MAX_HP, POINT_ATTBONUS_HUMAN, POINT_CRITICAL_PCT, POINT_PENETRATE_PCT}; 

/*********
* Table with value bonus
* 1000 / 5 / 8 / 11 ==> POINT_MAX_HP / POINT_ATTBONUS_HUMAN / POINT_CRITICAL_PCT / POINT_PENETRATE_PCT
*/
int affect_value_A[] = {500, 2, 2, 2}; // Reborn I 
int affect_value_B[] = {1000, 3, 3, 3}; //  Reborn II
int affect_value_C[] = {1500, 5, 5, 5}; // Reborn III

/*********
* Settins for points/level
*/
int level_set = 90; //Level reset when you up in grade reborn
int level_max = 120; // Max level from your server to can activate reborn
int stat_points_max = 90; // Max pognts stat in character 

/*********
* Get type or make new grade reborn ( not change this )
*/
int type_level[] = {0, 1, 2, 3};

int dwRewardCoins[] = {
	50,
	60,
	90
};
DWORD dwCheckItem[] = {
	39922,
	59970,
	59971
};

/*********
* The time for waiting can disable / enable anonymous way
*/
int time_anonymous = 10;

/*********
* Get type affect on reborn loaded from affect.h ( not change this )
*/
int get_affect[] = {
							AFFECT_REBORN_1_A, AFFECT_REBORN_1_B, AFFECT_REBORN_1_C, AFFECT_REBORN_1_D, 
							AFFECT_REBORN_2_A, AFFECT_REBORN_2_B, AFFECT_REBORN_2_C, AFFECT_REBORN_2_D,
							AFFECT_REBORN_3_A, AFFECT_REBORN_3_B, AFFECT_REBORN_3_C, AFFECT_REBORN_3_D};

/*********
* Get status from anonymous mode for insert in mysql log_reborn what mode have actual
*/		
const char* name_anonymous[] = {"Activate", "Disable"};				
							
/*********
* Get race from insert in mysql log_rebron
*/													
const char* set_race = NULL;

/*********
* The name of each character races
*/
const char* name_race[] = {"Shaman", 
									"Sura", 
									"Ninja", 
								#ifdef __YMIR_UPDATE_WOLFMAN__
									"Warrior",
									"Wolfman"}; 
								#else
									"Warrior"}; 							
								#endif
								
/*********
* Global value for other function check
*/								
int global_value[] = {0, 1, 2, 3, 9, 10};	

/*********
* Translate small informations
*/	
#define EN  // Activate translate in language english

#ifdef EN
const char* translate[] = {"Debug: Your argument nameLog is invalid !", "<< Reborn >> You can not do that so fast, wait %u second!", "<< Reborn >> Already you own bonuses degree Reborn I !", "<< Reborn >> Already you own bonuses degree Reborn II !","<< Reborn >> Already you own bonuses degree Reborn III !","<< Reborn >> You already have the maximum degree of reborn!","<< Reborn >> You need level %u for reborn!",	"<< Reborn >> Anonymous mode has been '%s' !", "<< Reborn >> Only the administrator is allowed to use this command!"}; 
#endif	
#ifdef DE
const char* translate[] = {"Debug: Ihr Argument nameLog ist ungültig!", "<< Reborn >> Sie können nicht so schnell tun, warten %u eine Sekunde!", "<< Reborn >> Schon besitzen Sie Boni Grad Reborn I!", "<< Reborn >> Schon besitzen Sie Boni Grad Reborn II!","<< Reborn >> Schon besitzen Sie Boni Grad Reborn III!","<< Reborn >> Sie haben bereits die maximale Grad der neu geboren!", "<< Reborn >> Sie müssen level %u für neu geboren!", "<< Reborn >> Anonymous-Modus wurde '%s' !", "<< Reborn >> Nur der Administrator darf diesen Befehl zu verwenden!"}; 
#endif	
#ifdef IT
const char* translate[] = {"Debug: Il tuo argomento non è valido nameLog!", "<< Reborn >> Non si può fare che così in fretta, attendere %u di secondo!", "<< Reborn >> È già propri bonus grado Reborn I!", "<< Reborn >> È già propri bonus grado Reborn II!","<< Reborn >> È già propri bonus grado Reborn III!", "<< Reborn >> Hai già il massimo grado di rinascere!", "<< Reborn >> È necessario livello di %u per rinascere!", "<< Reborn >> Modalità Anonimo è stata '%s' !", "<< Reborn >> Solo l'amministratore è autorizzato ad utilizzare questo comando!"}; 
#endif	

void RebornManager::ResetCharacter(LPCHARACTER ch)
{
	if (NULL == ch)
		return;

	if (!ch->IsPC())
		return;
		
	BYTE skillGroup = ch->GetSkillGroup();
	TPlayerSkill* skills = ch->GetAllSkill();
	
	ch->ResetPoint(MINMAX(1, level_set, PLAYER_MAX_LEVEL_CONST));
	
	//ch->ClearSkill();
	//ch->ClearSubSkill();
	//ch->SetSkillGroup(0);

	ch->PointChange(POINT_STAT, ((MINMAX(global_value[1], level_set, stat_points_max) - ch->GetLevel()) * global_value[3]) + ch->GetPoint(POINT_LEVEL_STEP));
	ch->PointChange(POINT_LEVEL, level_set - ch->GetLevel());
	ch->SetRandomHP((level_set - global_value[1]) * number(JobInitialPoints[ch->GetJob()].hp_per_lv_begin, JobInitialPoints[ch->GetJob()].hp_per_lv_end));
	ch->SetRandomSP((level_set - global_value[1]) * number(JobInitialPoints[ch->GetJob()].sp_per_lv_begin, JobInitialPoints[ch->GetJob()].sp_per_lv_end));
	ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
	ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
	
	ch->SetSkillGroup(skillGroup);
	ch->SetAllSkill(skills);
	
	ch->ComputePoints();
	ch->PointsPacket();
	ch->SkillLevelPacket();
}

void RebornManager::SendLogs(LPCHARACTER ch, const char* nameLog)
{
	if (NULL == ch)
		return;

	if (!ch->IsPC())
		return;

	if (!*nameLog)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, translate[0]);
		return;
	}	
	
	if(ch->GetJob() == JOB_SHAMAN)
		set_race = name_race[0];
	else if(ch->GetJob() == JOB_SURA)
		set_race = name_race[1];
	else if(ch->GetJob() == JOB_ASSASSIN)
		set_race = name_race[2];
	else if(ch->GetJob() == JOB_WARRIOR)
		set_race = name_race[3];
#ifdef __YMIR_UPDATE_WOLFMAN__
	else if(ch->GetJob() == JOB_WOLFMAN)
		set_race = name_race[4];
#endif
	
	if (!strcmp(nameLog, "send_logs_1"))
	{	
		char szQuery[512];
		snprintf(szQuery, sizeof(szQuery), "INSERT INTO player.log_reborn (owner_id, type, name, time, x, y, mapIndex, channel, race) VALUES(%u, '%s', '%s', NOW(), %ld, %ld, %ld, %d, '%s')", 
																ch->GetPlayerID(), "Reborn I", ch->GetName(), ch->GetX(), ch->GetY(), ch->GetMapIndex(), g_bChannel, set_race, name_anonymous[1]);
		DBManager::Instance().DirectQuery(szQuery);
	}		
	
	else if (!strcmp(nameLog, "send_logs_2"))
	{		
		std::unique_ptr<SQLMsg> pMsg(DBManager::Instance().DirectQuery("UPDATE player.log_reborn SET type='%s', time=NOW(), x=%ld, y=%ld, mapIndex=%ld, channel=%d WHERE owner_id = %u", "Reborn II", 
																ch->GetX(), ch->GetY(), ch->GetMapIndex(), g_bChannel, ch->GetPlayerID()));	
	}
	
	else if (!strcmp(nameLog, "send_logs_3"))
	{		
		std::unique_ptr<SQLMsg> pMsg(DBManager::Instance().DirectQuery("UPDATE player.log_reborn SET type='%s', time=NOW(), x=%ld, y=%ld, mapIndex=%ld, channel=%d WHERE owner_id = %u", "Reborn III", 
																ch->GetX(), ch->GetY(), ch->GetMapIndex(), g_bChannel, ch->GetPlayerID()));	
	}
}

void RebornManager::SetAnonymous(LPCHARACTER ch, const char* valueAnonymous)
{
	int isAnonymous = ch->GetQuestFlag("reborn.valueAnonymous");
	int isGetLastTime = ch->GetQuestFlag("reborn.flood_database");
	
	if (NULL == ch)
		return;

	if (!ch->IsPC())
		return;

	if (!*valueAnonymous)
	{	
		return;
	}
	
	if (!strcmp(valueAnonymous, "anonymous"))
	{	
		if (isAnonymous == global_value[1]) 
		{
			if (get_global_time() < isGetLastTime + time_anonymous)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, translate[1], time_anonymous);
				return;
			}
				ch->ChatPacket(CHAT_TYPE_COMMAND, "SetReborn pro_1");
				ch->ChatPacket(CHAT_TYPE_NOTICE, translate[7], name_anonymous[0]);
				ch->ShowReborn(false);
				std::unique_ptr<SQLMsg> pMsg(DBManager::Instance().DirectQuery("UPDATE player.log_reborn SET mode_anonymous='%s' WHERE owner_id = %u", name_anonymous[0], ch->GetPlayerID()));		
				ch->SetQuestFlag("reborn.valueAnonymous", global_value[2]);
				ch->SetQuestFlag("reborn.flood_database", get_global_time());
		}
		
		else if (isAnonymous == global_value[2]) 
		{	
			if (get_global_time() < isGetLastTime + time_anonymous)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, translate[1], time_anonymous);
				return;
			}
				ch->ChatPacket(CHAT_TYPE_COMMAND, "SetReborn pro_0");
				ch->ChatPacket(CHAT_TYPE_NOTICE, translate[7], name_anonymous[1]);
				ch->ShowReborn(true);
				std::unique_ptr<SQLMsg> pMsg(DBManager::Instance().DirectQuery("UPDATE player.log_reborn SET mode_anonymous='%s' WHERE owner_id = %u", name_anonymous[1], ch->GetPlayerID()));				
				ch->SetQuestFlag("reborn.valueAnonymous", global_value[0]);
				ch->SetQuestFlag("reborn.flood_database", get_global_time());
		}
	}	
}

void RebornManager::SetAffect(LPCHARACTER ch, const char* valueAffect)
{
	if (NULL == ch)
		return;

	if (!ch->IsPC())
		return;
	
	if (!strcmp(valueAffect, "send_affect_1"))
	{
		if (!ch->FindAffect(get_affect[0]) && !ch->FindAffect(get_affect[1]) && !ch->FindAffect(get_affect[2]) && !ch->FindAffect(get_affect[3]))
		{
			ch->AddAffect(get_affect[0], affect_name[0], affect_value_A[0], 0, 60*60*60*365, 0, true);
			ch->AddAffect(get_affect[1], affect_name[1], affect_value_A[1], 0, 60*60*60*365, 0, true);
			ch->AddAffect(get_affect[2], affect_name[2], affect_value_A[2], 0, 60*60*60*365, 0, true);	
			ch->AddAffect(get_affect[3], affect_name[3], affect_value_A[3], 0, 60*60*60*365, 0, true);
		}
		else
			ch->ChatPacket(CHAT_TYPE_INFO, translate[2]);		
	}	
	
	else if (!strcmp(valueAffect, "send_affect_2"))
	{	
		if (!ch->FindAffect(get_affect[4]) && !ch->FindAffect(get_affect[5]) && !ch->FindAffect(get_affect[6]) && !ch->FindAffect(get_affect[7]))
		{
			ch->AddAffect(get_affect[4], affect_name[0], affect_value_B[0], 0, 60*60*60*365, 0, true);
			ch->AddAffect(get_affect[5], affect_name[1], affect_value_B[1], 0, 60*60*60*365, 0, true);
			ch->AddAffect(get_affect[6], affect_name[2], affect_value_B[2], 0, 60*60*60*365, 0, true);	
			ch->AddAffect(get_affect[7], affect_name[3], affect_value_B[3], 0, 60*60*60*365, 0, true);
		}
		else
			ch->ChatPacket(CHAT_TYPE_INFO, translate[3]);
	}
	
	else if (!strcmp(valueAffect, "send_affect_3"))
	{	
		if (!ch->FindAffect(get_affect[8]) && !ch->FindAffect(get_affect[9]) && !ch->FindAffect(get_affect[10]) && !ch->FindAffect(get_affect[11]))
		{
			ch->AddAffect(get_affect[8], affect_name[0], affect_value_C[0], 0, 60*60*60*365, 0, true);
			ch->AddAffect(get_affect[9], affect_name[1], affect_value_C[1], 0, 60*60*60*365, 0, true);
			ch->AddAffect(get_affect[10], affect_name[2], affect_value_C[2], 0, 60*60*60*365, 0, true);	
			ch->AddAffect(get_affect[11], affect_name[3], affect_value_C[3], 0, 60*60*60*365, 0, true);
		}
		else
			ch->ChatPacket(CHAT_TYPE_INFO, translate[4]);
	}	
	
	else if (!strcmp(valueAffect, "remove_affect_administrator"))
	{
		if (ch->GetGMLevel() == GM_IMPLEMENTOR)	
		{				
			ch->RemoveAffect(get_affect[0]);
			ch->RemoveAffect(get_affect[1]);	
			ch->RemoveAffect(get_affect[2]);	
			ch->RemoveAffect(get_affect[3]);	
			ch->RemoveAffect(get_affect[4]);	
			ch->RemoveAffect(get_affect[5]);	
			ch->RemoveAffect(get_affect[6]);	
			ch->RemoveAffect(get_affect[7]);	
			ch->RemoveAffect(get_affect[8]);	
			ch->RemoveAffect(get_affect[9]);
			ch->RemoveAffect(get_affect[10]);	
			ch->RemoveAffect(get_affect[11]);			
		}
		else
																	// If u want to get command from this you need to use in chat: /set_reborn remove_affect_administrator		
			ch->ChatPacket(CHAT_TYPE_INFO, translate[8]);	
	}
}

void RebornManager::RewardCoins(LPCHARACTER ch, int mode)
{
	char szQuery[CHAT_MAX_LEN + 1];
	snprintf(szQuery, sizeof(szQuery), "UPDATE account.account SET coins = coins + %u WHERE id = %u", dwRewardCoins[mode - 1], ch->GetDesc()->GetAccountTable().id);
	DBManager::instance().DirectQuery(szQuery);
	
	ch->ChatPacket(CHAT_TYPE_INFO, "<Info> Felicitari tocmai ai primit %d Monede Dragon.", dwRewardCoins[mode - 1]);
}

bool RebornManager::CheckItem(LPCHARACTER ch, const char* argument)
{
	if (!strcmp(argument, "reborn_A"))
	{
		return (ch->CountSpecifyItem(dwCheckItem[0]) > 0) ? true : false;
	}
	else if (!strcmp(argument, "reborn_B"))
	{
		return (ch->CountSpecifyItem(dwCheckItem[1]) > 0) ? true : false;
	}
	else if (!strcmp(argument, "reborn_C"))
	{
		return (ch->CountSpecifyItem(dwCheckItem[2]) > 0) ? true : false;
	}
}

void RebornManager::SetLevel(LPCHARACTER ch)
{
	if (NULL == ch)
		return;

	if (!ch->IsPC())
		return;
	
	if (ch->CountEmptyInventory() < 20)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Ai nevoie de minim %d spatii goale in inventar pentru a face posibila dezechiparea echipamentului.", 20);
		return;
	}
	
	if (ch->GetRealReborn() == type_level[3])
	{
		ch->ChatPacket(CHAT_TYPE_NOTICE, translate[5]);	
		return;	
	}	
	
	else if (ch->GetRealReborn() == type_level[0])
	{
		if (ch->GetLevel() == level_max)
		{
			if ((CheckItem(ch, "reborn_A")))
			{
				ResetCharacter(ch); 
				SetAffect(ch, "send_affect_1");
				SendLogs(ch, "send_logs_1");
			
				ch->UpdateReborn(type_level[1] - ch->GetRealReborn());
				RebornManager::instance().RewardCoins(ch, 1);
				ch->RemoveSpecifyItem(dwCheckItem[0], 1);
				ch->ChatPacket(CHAT_TYPE_COMMAND, "SetReborn arg1");
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
			}
			else
				ch->ChatPacket(CHAT_TYPE_INFO, "<Reborn> Ai nevoie de itemul [%s] pentru a putea face asta.", ITEM_MANAGER::instance().GetTable(dwCheckItem[0])->szLocaleName);
		}
		else
			ch->ChatPacket(CHAT_TYPE_INFO, translate[6], level_max);			
	}
	
	else if (ch->GetRealReborn() == type_level[1])
	{	
		if (ch->GetLevel() == level_max)
		{
			if ((CheckItem(ch, "reborn_B")))
			{
				ResetCharacter(ch); 
				SetAffect(ch, "send_affect_2");
				SendLogs(ch, "send_logs_2"); 
			
				ch->UpdateReborn(type_level[2] - ch->GetRealReborn());
				RebornManager::instance().RewardCoins(ch, 2);
				ch->RemoveSpecifyItem(dwCheckItem[1], 1);
				ch->ChatPacket(CHAT_TYPE_COMMAND, "SetReborn arg2");
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
			}
			else
				ch->ChatPacket(CHAT_TYPE_INFO, "<Reborn> Ai nevoie de itemul [%s] pentru a putea face asta.", ITEM_MANAGER::instance().GetTable(dwCheckItem[1])->szLocaleName);
		}
		else
			ch->ChatPacket(CHAT_TYPE_INFO, translate[6], level_max);			
	}
	
	else if (ch->GetRealReborn() == type_level[2])
	{	
		if (ch->GetLevel() == level_max)
		{
			if ((CheckItem(ch, "reborn_C")))
			{
				ResetCharacter(ch); 
				SetAffect(ch, "send_affect_3"); 
				SendLogs(ch, "send_logs_3"); 
			
				ch->UpdateReborn(type_level[3] - ch->GetRealReborn());
				RebornManager::instance().RewardCoins(ch, 3);
				ch->RemoveSpecifyItem(dwCheckItem[2], 1);
				ch->ChatPacket(CHAT_TYPE_COMMAND, "SetReborn arg3");
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
			}
			else
				ch->ChatPacket(CHAT_TYPE_INFO, "<Reborn> Ai nevoie de itemul [%s] pentru a putea face asta.", ITEM_MANAGER::instance().GetTable(dwCheckItem[2])->szLocaleName);
		}
		else
			ch->ChatPacket(CHAT_TYPE_INFO, translate[6], level_max);				
	}
}	