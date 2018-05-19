#define _acce_cpp_
#include "stdafx.h"
#include "config.h"
#include "constants.h"
#include "utils.h"
#include "log.h"
#include "char.h"
#include "dev_log.h"
#include "locale_service.h"
#include "item.h"
#include "item_manager.h"
#include <stdlib.h>
#include <sstream>
#define RETURN_IF_ACCE_IS_NOT_OPENED(ch) if (!(ch)->IsAcceOpen()) return

extern int test_server;
static std::vector<ACCE_DATA*>	s_acce_proto;
static bool s_isInitializedAcceMaterialInformation = false;

enum EAcceResultCategory
{
	ACCE_CATEGORY_POTION,
	ACCE_CATEGORY_WEAPON,
	ACCE_CATEGORY_ARMOR,
	ACCE_CATEGORY_ACCESSORY,
	ACCE_CATEGORY_ETC,
};

typedef std::vector<ACCE_VALUE>	TAcceValueVector;

struct SAcceMaterialInfo
{
	SAcceMaterialInfo()
	{
		bHaveComplicateMaterial = false;
	};
	
	ACCE_VALUE			reward;
	TAcceValueVector	material;
	DWORD				gold;
	TAcceValueVector	complicateMaterial;
	
	std::string			infoText;
	bool				bHaveComplicateMaterial;
};

struct SItemNameAndLevel
{
	SItemNameAndLevel() { level = 0; }
	
	std::string		name;
	int				level;
};

typedef std::vector<SAcceMaterialInfo>								TAcceResultList;
typedef boost::unordered_map<DWORD, TAcceResultList>				TAcceMapByNPC;
typedef boost::unordered_map<DWORD, std::string>					TAcceResultInfoTextByNPC;

TAcceMapByNPC acce_info_map;
TAcceResultInfoTextByNPC acce_result_info_map_by_npc;

class CAcceMaterialInfoHelper
{
public:
public:
};

static bool FN_check_item_count(LPITEM *items, DWORD item_vnum, int need_count)
{
	int	count = 0;
	for (int i=0; i<ACCE_MAX_NUM; ++i)
	{
		if (NULL == items[i])
			continue;
		
		if (item_vnum==items[i]->GetVnum())
		{
			count += items[i]->GetCount();
		}
	}
	
	return (count>=need_count);
}

static void FN_remove_material(LPITEM *items, DWORD item_vnum, int need_count, int result)
{
	int		count = 0;
	LPITEM	item = NULL;
	
	item = items[1];
	if (item != NULL)
	{
		item->SetCount(0);
		items[1] = NULL;
	}
	
	if (result == 1)
	{
		item = items[0];
		if (item != NULL)
		{
			item->SetCount(0);
			items[0] = NULL;
		}
	}
}

static ACCE_DATA* FN_find_acce(LPITEM *items, WORD npc_vnum)
{
	DWORD	i, end_index;
	
	if (npc_vnum == 0)
		return NULL;
	
	end_index = s_acce_proto.size();
	for (i = 0; i < end_index; ++i)
	{
		if (s_acce_proto[i]->can_make_item(items, npc_vnum))
			return s_acce_proto[i];
	}
	
	return NULL;
}

static bool FN_check_valid_npc(WORD vnum)
{
	for (std::vector<ACCE_DATA*>::iterator iter = s_acce_proto.begin(); iter != s_acce_proto.end(); iter++)
	{
		if (std::find((*iter)->npc_vnum.begin(), (*iter)->npc_vnum.end(), vnum) != (*iter)->npc_vnum.end())
			return true;
	}
	
	return false;
}

static bool FN_check_acce_data(ACCE_DATA *acce_data)
{
	DWORD	i = 0;
	DWORD	end_index = 0;
	
	end_index = acce_data->npc_vnum.size();
	for (i=0; i<end_index; ++i)
	{
		if ( acce_data->npc_vnum[i] == 0 )
			return false;
	}
	
	end_index = acce_data->item.size();
	for (i=0; i<end_index; ++i)
	{
		if ( acce_data->item[i].vnum == 0 )
			return false;
		
		if ( acce_data->item[i].count == 0 )
			return false;
	}
	
	end_index = acce_data->reward.size();
	for (i=0; i<end_index; ++i)
	{
		if (acce_data->reward[i].vnum == 0)
			return false;
		
		if (acce_data->reward[i].count == 0)
			return false;
	}
	
	return true;
}

ACCE_DATA::ACCE_DATA()
{
	this->percent = 0;
	this->gold = 0;
	this->abs_chance_min = 0;
	this->abs_chance_max = 0;
}

bool ACCE_DATA::can_make_item(LPITEM *items, WORD npc_vnum)
{
	DWORD	i, end_index;
	DWORD	need_vnum;
	int		need_count;
	int		found_npc = false;
	
	end_index = this->npc_vnum.size();
	for (i=0; i<end_index; ++i)
	{
		if (npc_vnum == this->npc_vnum[i])
			found_npc = true;
	}
	
	if (false == found_npc)
		return false;
	
	end_index = this->item.size();
	for (i=0; i<end_index; ++i)
	{
		need_vnum = this->item[i].vnum;
		need_count = this->item[i].count;
		
		if (false == FN_check_item_count(items, need_vnum, need_count))
			return false;
	}
	
	return true;
}

ACCE_VALUE* ACCE_DATA::reward_value()
{
	int		end_index = 0;
	DWORD	reward_index = 0;
	
	end_index = this->reward.size();
	reward_index = number(0, end_index);
	reward_index = number(0, end_index-1);
	return &this->reward[reward_index];
}

void ACCE_DATA::remove_material(LPCHARACTER ch, int result)
{
	DWORD	i, end_index;
	DWORD	need_vnum;
	int		need_count;
	LPITEM	*items = ch->GetAcceItem();
	
	end_index = this->item.size();
	for (i = 0; i < ACCE_MAX_NUM; ++i)
	{
		need_vnum = this->item[i].vnum;
		need_count = this->item[i].count;
		
		FN_remove_material(items, need_vnum, need_count, result);
	}
}

void Acce_clean_item(LPCHARACTER ch)
{
	LPITEM	*acce_item;
	
	acce_item = ch->GetAcceItem();
	for (int i=0; i<ACCE_MAX_NUM; ++i)
	{
		if (acce_item[i] == NULL)
			continue;
		
		if (acce_item[i]->GetType() == ITEM_COSTUME && acce_item[i]->GetSubType() == COSTUME_ACCE && acce_item[i]->GetType() != ITEM_WEAPON && acce_item[i]->GetType() != ITEM_ARMOR)
			acce_item[i]->SetSocket(0, 0);
		
		acce_item[i] = NULL;
	}
}

void Acce_open(LPCHARACTER ch)
{
	if (false == s_isInitializedAcceMaterialInformation)
	{
		Acce_InformationInitialize();
	}
	
	if (NULL == ch)
		return;
	
	LPCHARACTER	npc;
	npc = ch->GetQuestNPC();
	if (NULL == npc)
	{
		if (test_server)
			dev_log(LOG_DEB0, "acce_npc is NULL");
		
		return;
	}
	
	if (FN_check_valid_npc(npc->GetRaceNum()) == false)
	{
		if ( test_server == true )
		{
			dev_log(LOG_DEB0, "Acce not valid NPC");
		}
		
		return;
	}
	
	if (ch->IsAcceOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Fereastra de rafinament a esarfelor este deja deschisa!"));
		return;
	}
	
	if ( ch->GetExchange() || ch->GetMyShop() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsAcceOpen() )
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래중(창고,교환,상점)에는 사용할 수 없습니다."));
		return;
	}
	
	long distance = DISTANCE_APPROX(ch->GetX() - npc->GetX(), ch->GetY() - npc->GetY());
	if (distance >= ACCE_MAX_DISTANCE)
	{
		sys_log(1, "Acce: TOO_FAR: %s distance %d", ch->GetName(), distance);
		return;
	}
	
	Acce_clean_item(ch);
	ch->SetAcceNpc(npc);
	ch->ChatPacket(CHAT_TYPE_COMMAND, "Acce open %d", npc->GetRaceNum());
}

void Acce_absorption_open(LPCHARACTER ch)
{
	if (false == s_isInitializedAcceMaterialInformation)
	{
		Acce_InformationInitialize();
	}
	
	if (NULL == ch)
		return;
	
	LPCHARACTER	npc;
	npc = ch->GetQuestNPC();
	if (NULL == npc)
	{
		if (test_server)
			dev_log(LOG_DEB0, "acce_npc is NULL");
		
		return;
	}
	
	if (FN_check_valid_npc(npc->GetRaceNum()) == false)
	{
		if ( test_server == true )
		{
			dev_log(LOG_DEB0, "Acce not valid NPC");
		}
		
		return;
	}
	
	if (ch->IsAcceOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Fereastra de rafinament a esarfelor este deja deschisa!"));
		return;
	}
	
	if ( ch->GetExchange() || ch->GetMyShop() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsAcceOpen() )
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래중(창고,교환,상점)에는 사용할 수 없습니다."));
		return;
	}
	
	long distance = DISTANCE_APPROX(ch->GetX() - npc->GetX(), ch->GetY() - npc->GetY());
	if (distance >= ACCE_MAX_DISTANCE)
	{
		sys_log(1, "Acce: TOO_FAR: %s distance %d", ch->GetName(), distance);
		return;
	}
	
	Acce_clean_item(ch);
	ch->SetAcceNpc(npc);
	ch->ChatPacket(CHAT_TYPE_COMMAND, "Acce open_absorption %d", npc->GetRaceNum());
}

void Acce_close(LPCHARACTER ch)
{
	RETURN_IF_ACCE_IS_NOT_OPENED(ch);
	Acce_clean_item(ch);
	ch->SetAcceNpc(NULL);
	ch->ChatPacket(CHAT_TYPE_COMMAND, "Acce close");
	dev_log(LOG_DEB0, "<Acce> close (%s)", ch->GetName());
}

void Acce_init()
{
	ACCE_DATA * p_acce = NULL;
	std::vector<ACCE_DATA*>::iterator iter;
	char file_name[256+1];
	
	snprintf(file_name, sizeof(file_name), "%s/acce.txt", LocaleService_GetBasePath().c_str());
	sys_log(0, "Acce_Init %s", file_name);
	for (iter = s_acce_proto.begin(); iter!=s_acce_proto.end(); iter++)
	{
		p_acce = *iter;
		M2_DELETE(p_acce);
	}
	
	s_acce_proto.clear();
	if (false == Acce_load(file_name))
		sys_err("Acce_Init failed");
}

bool Acce_load(const char *file)
{
	FILE	*fp;
	char	one_line[256];
	int		value1, value2;
	const char	*delim = " \t\r\n";
	char	*v, *token_string;
	ACCE_DATA	*acce_data = NULL;
	ACCE_VALUE	acce_value = {0, 0};
	
	if (0 == file || 0 == file[0])
		return false;
	
	if ((fp = fopen(file, "r")) == 0)
		return false;
	
	while (fgets(one_line, 256, fp))
	{
		value1 = value2 = 0;
		if (one_line[0] == '#')
			continue;
		
		token_string = strtok(one_line, delim);
		if (NULL == token_string)
			continue;
		
		if ((v = strtok(NULL, delim)))
			str_to_number(value1, v);
		
		if ((v = strtok(NULL, delim)))
			str_to_number(value2, v);
		
		TOKEN("section")
		{
			acce_data = M2_NEW ACCE_DATA;
		}
		else TOKEN("npc")
		{
			acce_data->npc_vnum.push_back((WORD)value1);
		}
		else TOKEN("item")
		{
			acce_value.vnum = value1;
			acce_value.count = value2;
			acce_data->item.push_back(acce_value);
		}
		else TOKEN("reward")
		{
			acce_value.vnum = value1;
			acce_value.count = value2;
			acce_data->reward.push_back(acce_value);
		}
		else TOKEN("abs_chance_min")
		{
			acce_data->abs_chance_min = value1;
		}
		else TOKEN("abs_chance_max")
		{
			acce_data->abs_chance_max = value1;
		}
		else TOKEN("percent")
		{
			acce_data->percent = value1;
		}
		else TOKEN("gold")
		{
			acce_data->gold = value1;
		}
		else TOKEN("end")
		{
			if (false == FN_check_acce_data(acce_data))
			{
				dev_log(LOG_DEB0, "something wrong");
				M2_DELETE(acce_data);
				continue;
			}
			
			s_acce_proto.push_back(acce_data);
		}
	}
	
	fclose(fp);
	return true;
}

static void FN_acce_print(ACCE_DATA *data, DWORD index)
{
	DWORD	i;
	dev_log(LOG_DEB0, "--------------------------------");
	dev_log(LOG_DEB0, "ACCE_DATA[%d]", index);
	for (i = 0; i < data->npc_vnum.size(); ++i)
	{
		dev_log(LOG_DEB0, "\tNPC_VNUM[%d] = %d", i, data->npc_vnum[i]);
	}
	for (i = 0; i < data->item.size(); ++i)
	{
		dev_log(LOG_DEB0, "\tITEM[%d]   = (%d, %d)", i, data->item[i].vnum, data->item[i].count);
	}
	for (i = 0; i < data->reward.size(); ++i)
	{
		dev_log(LOG_DEB0, "\tREWARD[%d] = (%d, %d)", i, data->reward[i].vnum, data->reward[i].count);
	}
	
	dev_log(LOG_DEB0, "\tPERCENT = %d", data->percent);
	dev_log(LOG_DEB0, "--------------------------------");
}

void Acce_print ()
{
	for (DWORD i=0; i<s_acce_proto.size(); ++i)
	{
		FN_acce_print(s_acce_proto[i], i);
	}
}

static bool FN_update_acce_status(LPCHARACTER ch)
{
	if (NULL == ch)
		return false;
	
	if (!ch->IsAcceOpen())
		return false;
	
	LPCHARACTER	npc = ch->GetQuestNPC();
	if (NULL == npc)
		return false;
	
	ACCE_DATA* Acce = FN_find_acce(ch->GetAcceItem(), npc->GetRaceNum());
	if (NULL == Acce)
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "Acce info 0 0 0 0 0");
		return false;
	}
	
	ch->ChatPacket(CHAT_TYPE_COMMAND, "Acce info %d %d %d %d %d", Acce->gold, 0, 0, Acce->reward[0], Acce->reward[1]);
	return true;
}

bool Acce_make(LPCHARACTER ch)
{
	LPCHARACTER	npc;
	int			percent_number = 0;
	ACCE_DATA	*acce_proto;
	LPITEM	*items;
	LPITEM	new_item;
	int	reward_vnum = 0;
	
	if (!(ch)->IsAcceOpen())
	{
		(ch)->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Fereastra de rafinament a esarfelor nu este deschisa!"));
		return false;
	}
	
	npc = ch->GetQuestNPC();
	if (NULL == npc)
	{
		return false;
	}
	
	items = ch->GetAcceItem();
	if (items[0] == NULL || items[1] == NULL || items[0]->GetValue(0) != items[1]->GetValue(0))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Drag the items into the window."));
		return false;
	}
#ifdef __GF_SEAL_ITEM_UPDATE__
	else if (items[0]->IsSealed() || items[1]->IsSealed())
	{
		(ch)->ChatPacket(CHAT_TYPE_INFO, "Voc?n? pode combinar itens bloqueados.");
		return false;
	}
#endif
	
	acce_proto = FN_find_acce(items, npc->GetRaceNum());
	if (NULL == acce_proto)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("제조 재료가 부족합니다"));
		return false;
	}
	
	if (ch->GetGold() < acce_proto->gold)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("돈이 부족하거나 아이템이 제자리에 없습니다."));
		return false;
	}
	
	reward_vnum = items[0]->GetValue(1);
	if (reward_vnum == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Drag the items into the window."));
		return false;
	}
	
	ACCE_VALUE	*reward_value = acce_proto->reward_value();
	if (0 < acce_proto->gold)
		ch->PointChange(POINT_GOLD, -(acce_proto->gold), false);
	
	percent_number = number(1, 100);
	if (percent_number <= acce_proto->percent)
	{
		acce_proto->remove_material(ch, 1);
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Felicitari,esarfa a fost creata!"));
		ch->ChatPacket(CHAT_TYPE_COMMAND, "acce success %d %d", reward_vnum, reward_value->count);
		ch->EffectPacket(SE_ACCE_SUCCEDED_1);
		int abs_chance = 0;
		if (acce_proto->abs_chance_min < 1 || acce_proto->abs_chance_max < 1)
		{
			abs_chance = 1;
		}
		else if (acce_proto->abs_chance_min > acce_proto->abs_chance_max)
		{
			abs_chance = 1;
		}
		else if (acce_proto->abs_chance_min == acce_proto->abs_chance_max)
		{
			abs_chance = acce_proto->abs_chance_min;
		}
		else
		{
			abs_chance = number(acce_proto->abs_chance_min, acce_proto->abs_chance_max);
		}
		
		new_item = ch->AutoGiveAcce(reward_vnum, reward_value->count, abs_chance);
		if (new_item)
			LogManager::instance().AcceLog(ch->GetPlayerID(), ch->GetX(), ch->GetY(), reward_vnum, new_item->GetID(), reward_value->count, abs_chance, 1);
		return true;
	}
	else
	{
		acce_proto->remove_material(ch, 0);
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ghinion,rafinarea esarfei a esuat!"));
		ch->ChatPacket(CHAT_TYPE_COMMAND, "Acce fail");
		LogManager::instance().AcceLog(ch->GetPlayerID(), ch->GetX(), ch->GetY(), reward_vnum, 0, 0, 0, 0);
		return false;
	}
	
	return false;
}

bool Acce_absorption_make(LPCHARACTER ch)
{
	LPCHARACTER	npc;
	LPITEM	*items;
	int	absorption_chance = 0;
	BYTE	type1 = 0, type2 = 0, type3 = 0, type4 = 0, type5 = 0, type6 = 0, type7 = 0, type8 = 0, type9 = 0, type10 = 0, type11 = 0, type12 = 0, type13 = 0, type14 = 0, type15 = 0;
	float	value1 = 0, value2 = 0, value3 = 0, value4 = 0, value5 = 0, value6 = 0, value7 = 0, value8 = 0, value9 = 0, value10 = 0, value11 = 0, value12 = 0, value13 = 0, value14 = 0, value15 = 0;
	
	if (!(ch)->IsAcceOpen())
	{
		(ch)->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Fereastra de rafinament a esarfelor nu este deschisa!"));
		return false;
	}
	
	npc = ch->GetQuestNPC();
	if (NULL == npc)
	{
		return false;
	}
	
	items = ch->GetAcceItem();
	if (items[0] == NULL || items[1] == NULL)
	{
		(ch)->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Pune esarfele in fereastra de rafinament!"));
		return false;
	}
	else if (items[1]->GetType() != ITEM_WEAPON && items[1]->GetType() != ITEM_ARMOR)
	{
		(ch)->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Poti folosii esarfa ca sa absorbi bonusurile doar de pe arme si armuri!"));
		return false;
	}
	else if (items[0]->GetAcceAttributeCount() > 0)
	{
		(ch)->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Trebuie sa golesti bonusurile existente de pe esarfa pentru a continua!"));
		return false;
	}
#ifdef __GF_SEAL_ITEM_UPDATE__
	else if (items[0]->IsSealed() || items[1]->IsSealed())
	{
		(ch)->ChatPacket(CHAT_TYPE_INFO, "Voc?n? pode absorver itens bloqueados.");
		return false;
	}
#endif
	
	absorption_chance = items[0]->GetSocket(1);
	type1 = items[1]->GetAttributeType(0);
	value1 = items[1]->GetAttributeValue(0) * absorption_chance / 100;
	type2 = items[1]->GetAttributeType(1);
	value2 = items[1]->GetAttributeValue(1) * absorption_chance / 100;
	type3 = items[1]->GetAttributeType(2);
	value3 = items[1]->GetAttributeValue(2) * absorption_chance / 100;
	type4 = items[1]->GetAttributeType(3);
	value4 = items[1]->GetAttributeValue(3) * absorption_chance / 100;
	type5 = items[1]->GetAttributeType(4);
	value5 = items[1]->GetAttributeValue(4) * absorption_chance / 100;
	type6 = items[1]->GetAttributeType(5);
	value6 = items[1]->GetAttributeValue(5) * absorption_chance / 100;
	type7 = items[1]->GetAttributeType(6);
	value7 = items[1]->GetAttributeValue(6) * absorption_chance / 100;
	
	if (items[1]->GetType() == ITEM_ARMOR)
	{
		type8 = 54;
		value8 = (items[1]->GetValue(1) + items[1]->GetValue(5)) * absorption_chance / 100;
	}
	else
	{
		type8 = 0;
		value8 = 0;
	}
	
	if (items[1]->GetType() == ITEM_WEAPON)
	{
		type9 = 53;
		value9 = (items[1]->GetValue(3) + items[1]->GetValue(5)) * absorption_chance / 100;
		type10 = 53;
		value10 = (items[1]->GetValue(4) + items[1]->GetValue(5)) * absorption_chance / 100;
		value11 = (items[1]->GetValue(1) + items[1]->GetValue(5)) * absorption_chance / 100;
		value12 = (items[1]->GetValue(2) + items[1]->GetValue(5)) * absorption_chance / 100;
		if (items[1]->GetValue(1) <= 0)
		{
			type11 = 0;
			value11 = 0;
		}
		else
		{
			type11 = 55;
		}
		
		if (items[1]->GetValue(2) <= 0)
		{
			type12 = 0;
			value12 = 0;
		}
		else
		{
			type12 = 55;
		}
	}
	else
	{
		type9 = 0;
		value9 = 0;
		type10 = 0;
		value10 = 0;
		type11 = 0;
		value11 = 0;
		type12 = 0;
		value12 = 0;
	}
	
	type13 = items[1]->GetNewAttributeType(0);
	value13 = items[1]->GetNewAttributeValue(0) * absorption_chance / 100;
	type14 = items[1]->GetNewAttributeType(1);
	value14 = items[1]->GetNewAttributeValue(1) * absorption_chance / 100;
	type15 = items[1]->GetNewAttributeType(2);
	value15 = items[1]->GetNewAttributeValue(2) * absorption_chance / 100;
	
	int value1_final = (value1 <= 0) ? (int)(1) : (int)(value1);
	int value2_final = (value2 <= 0) ? (int)(1) : (int)(value2);
	int value3_final = (value3 <= 0) ? (int)(1) : (int)(value3);
	int value4_final = (value4 <= 0) ? (int)(1) : (int)(value4);
	int value5_final = (value5 <= 0) ? (int)(1) : (int)(value5);
	int value6_final = (value6 <= 0) ? (int)(1) : (int)(value6);
	int value7_final = (value7 <= 0) ? (int)(1) : (int)(value7);
	int value8_final = (value8 <= 0) ? (int)(1) : (int)(value8);
	int value9_final = (value10 - value9 <= 0) ? (int)(1) : (int)(value10 - value9);
	int value10_final = (value10 <= 0) ? (int)(1) : (int)(value10);
	int value11_final = (value11 - value11 <= 0) ? (int)(1) : (int)(value12 - value11);
	int value12_final = (value12 <= 0) ? (int)(1) : (int)(value12);
	int value13_final = (value13 <= 0) ? (int)(1) : (int)(value13);
	int value14_final = (value14 <= 0) ? (int)(1) : (int)(value14);
	int value15_final = (value15 <= 0) ? (int)(1) : (int)(value15);
	
	items[0]->SetForceAttribute(0, type1, value1_final);
	items[0]->SetForceAttribute(1, type2, value2_final);
	items[0]->SetForceAttribute(2, type3, value3_final);
	items[0]->SetForceAttribute(3, type4, value4_final);
	items[0]->SetForceAttribute(4, type5, value5_final);
	items[0]->SetForceAttribute(5, type6, value6_final);
	items[0]->SetForceAttribute(6, type7, value7_final);
	items[0]->SetForceAttribute(7, type8, value8_final);
	items[0]->SetForceAttribute(8, type9, value9_final);
	items[0]->SetForceAttribute(9, type10, value10_final);
	items[0]->SetForceAttribute(10, type11, value11_final);
	items[0]->SetForceAttribute(11, type12, value12_final);
	items[0]->SetForceAttribute(12, type13, value13_final);
	items[0]->SetForceAttribute(13, type14, value14_final);
	items[0]->SetForceAttribute(14, type15, value15_final);
	
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Procesul de absorbtie a bonusurilor a avut succes!"));
	ch->ChatPacket(CHAT_TYPE_COMMAND, "acce success_absorption");
	
	items[1]->SetCount(0);
	items[1] = NULL;
	return true;
}

void Acce_show_list(LPCHARACTER ch)
{
	LPITEM	*acce_item;
	LPITEM	item;
	
	RETURN_IF_ACCE_IS_NOT_OPENED(ch);
	
	acce_item = ch->GetAcceItem();
	for (int i=0; i<ACCE_MAX_NUM; ++i)
	{
		item = acce_item[i];
		if (item == NULL)
			continue;
		
		ch->ChatPacket(CHAT_TYPE_INFO, "Acce[%d]: inventory[%d]: %s", i, item->GetCell(), item->GetName());
	}
}

void Acce_add_item(LPCHARACTER ch, int acce_index, int inven_index)
{
	LPITEM	item;
	LPITEM	*acce_item;
	
	RETURN_IF_ACCE_IS_NOT_OPENED(ch);
	
	if (inven_index < 0 || INVENTORY_MAX_NUM <= inven_index)
		return;
	
	if (acce_index<0 || ACCE_MAX_NUM <= acce_index)
		return;
	
	item = ch->GetInventoryItem(inven_index);
	if (item == NULL)
		return;
	
	if (item->GetType() == ITEM_COSTUME && item->GetSubType() == COSTUME_ACCE && item->GetType() != ITEM_WEAPON && item->GetType() != ITEM_ARMOR)
		item->SetSocket(0, 1);
	
	acce_item = ch->GetAcceItem();
	for (int i=0; i<ACCE_MAX_NUM; ++i)
	{
		if (item==acce_item[i])
		{
			acce_item[i] = NULL;
			break;
		}
	}
	
	acce_item[acce_index] = item;
	if (acce_index == 1)
	{
		if (item->GetType() == ITEM_WEAPON || item->GetType() == ITEM_ARMOR)
		{
			ch->ChatPacket(CHAT_TYPE_COMMAND, "AcceAbsMessage");
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_COMMAND, "AcceMessage");
		}
	}
	
	if (test_server)
		ch->ChatPacket(CHAT_TYPE_INFO, "Acce[%d]: inventory[%d]: %s added", acce_index, inven_index, item->GetName());
	
	if (acce_index == 0)
	{
		FN_update_acce_status(ch);
	}
	return;
}

void Acce_delete_item(LPCHARACTER ch, int acce_index)
{
	LPITEM	item;
	LPITEM	*acce_item;
	
	RETURN_IF_ACCE_IS_NOT_OPENED(ch);
	if (acce_index < 0 || ACCE_MAX_NUM <= acce_index)
		return;
	
	acce_item = ch->GetAcceItem();
	if (acce_item[acce_index] == NULL)
		return;
	
	item = acce_item[acce_index];
	if (!item)
		return;
	
	if (item->GetType() == ITEM_COSTUME && item->GetSubType() == COSTUME_ACCE && item->GetType() != ITEM_WEAPON && item->GetType() != ITEM_ARMOR)
		item->SetSocket(0, 0);
	
	acce_item[acce_index] = NULL;
	if (test_server)
		ch->ChatPacket(CHAT_TYPE_INFO, "Acce[%d]: Acce[%d]: %s deleted", acce_index, item->GetCell(), item->GetName());
	
	FN_update_acce_status(ch);
	return;
}

SItemNameAndLevel SplitItemNameAndLevelFromHName(const std::string& name)
{
	int level = 0;
	SItemNameAndLevel info;
	info.name = name;
	
	size_t pos = name.find("+");
	if (std::string::npos != pos)
	{
		const std::string levelStr = name.substr(pos + 1, name.size() - pos - 1);
		str_to_number(level, levelStr.c_str());

		info.name = name.substr(0, pos);
	}
	
	info.level = level;
	return info;
};

bool FIsEqualAcceValue(const ACCE_VALUE& a, const ACCE_VALUE& b)
{
	return (a.vnum == b.vnum) && (a.count == b.count);
}

bool FIsLessAcceValue(const ACCE_VALUE& a, const ACCE_VALUE& b)
{
	return a.vnum < b.vnum;
}

void Acce_MakeAcceInformationText()
{
	for (TAcceMapByNPC::iterator iter = acce_info_map.begin(); acce_info_map.end() != iter; ++iter)
	{
		TAcceResultList& resultList = iter->second;
		for (TAcceResultList::iterator resultIter = resultList.begin(); resultList.end() != resultIter; ++resultIter)
		{
			SAcceMaterialInfo& materialInfo = *resultIter;
			std::string& infoText = materialInfo.infoText;
			if (0 < materialInfo.complicateMaterial.size())
			{
				std::sort(materialInfo.complicateMaterial.begin(), materialInfo.complicateMaterial.end(), FIsLessAcceValue);
				std::sort(materialInfo.material.begin(), materialInfo.material.end(), FIsLessAcceValue);
				for (TAcceValueVector::iterator iter = materialInfo.complicateMaterial.begin(); materialInfo.complicateMaterial.end() != iter; ++iter)
				{
					for (TAcceValueVector::iterator targetIter = materialInfo.material.begin(); materialInfo.material.end() != targetIter; ++targetIter)
					{
						if (*targetIter == *iter)
						{
							targetIter = materialInfo.material.erase(targetIter);
						}
					}
				}
				
				for (TAcceValueVector::iterator iter = materialInfo.complicateMaterial.begin(); materialInfo.complicateMaterial.end() != iter; ++iter)
				{
					char tempBuffer[128];
					sprintf(tempBuffer, "%d,%d|", iter->vnum, iter->count);
					
					infoText += std::string(tempBuffer);
				}
				
				infoText.erase(infoText.size() - 1);
				if (0 < materialInfo.material.size())
					infoText.push_back('&');
			}
			
			for (TAcceValueVector::iterator iter = materialInfo.material.begin(); materialInfo.material.end() != iter; ++iter)
			{
				char tempBuffer[128];
				sprintf(tempBuffer, "%d,%d&", iter->vnum, iter->count);
				infoText += std::string(tempBuffer);
			}
			
			infoText.erase(infoText.size() - 1);
			if (0 < materialInfo.gold)
			{
				char temp[128];
				sprintf(temp, "%d", materialInfo.gold);
				infoText += std::string("/") + temp;
			}
		}
	}
}

bool Acce_InformationInitialize()
{
	for (int i = 0; i < s_acce_proto.size(); ++i)
	{
		ACCE_DATA* acceData = s_acce_proto[i];
		const std::vector<ACCE_VALUE>& rewards = acceData->reward;
		if (rewards.size() != 1)
		{
			sys_err("[AcceInfo] WARNING! Does not support multiple rewards (count: %d)", rewards.size());			
			continue;
		}
		
		const ACCE_VALUE& reward = rewards.at(0);
		const WORD& npcVNUM = acceData->npc_vnum.at(0);
		bool bComplicate = false;
		
		TAcceMapByNPC& acceMap = acce_info_map;
		TAcceResultList& resultList = acceMap[npcVNUM];
		SAcceMaterialInfo materialInfo;
		
		materialInfo.reward = reward;
		materialInfo.gold = acceData->gold;
		materialInfo.material = acceData->item;
		
		for (TAcceResultList::iterator iter = resultList.begin(); resultList.end() != iter; ++iter)
		{
			SAcceMaterialInfo& existInfo = *iter;
			if (reward.vnum == existInfo.reward.vnum)
			{
				for (TAcceValueVector::iterator existMaterialIter = existInfo.material.begin(); existInfo.material.end() != existMaterialIter; ++existMaterialIter)
				{
					TItemTable* existMaterialProto = ITEM_MANAGER::Instance().GetTable(existMaterialIter->vnum);
					if (NULL == existMaterialProto)
					{
						sys_err("There is no item(%u)", existMaterialIter->vnum);
						return false;
					}
					
					SItemNameAndLevel existItemInfo = SplitItemNameAndLevelFromHName(existMaterialProto->szName);
					if (0 < existItemInfo.level)
					{
						for (TAcceValueVector::iterator currentMaterialIter = materialInfo.material.begin(); materialInfo.material.end() != currentMaterialIter; ++currentMaterialIter)
						{
							TItemTable* currentMaterialProto = ITEM_MANAGER::Instance().GetTable(currentMaterialIter->vnum);
							SItemNameAndLevel currentItemInfo = SplitItemNameAndLevelFromHName(currentMaterialProto->szName);
							if (currentItemInfo.name == existItemInfo.name)
							{
								bComplicate = true;
								existInfo.complicateMaterial.push_back(*currentMaterialIter);
								if (std::find(existInfo.complicateMaterial.begin(), existInfo.complicateMaterial.end(), *existMaterialIter) == existInfo.complicateMaterial.end())
									existInfo.complicateMaterial.push_back(*existMaterialIter);
								
								break;
							}
						}
					}
				}
			}
		}
		
		if (false == bComplicate)
			resultList.push_back(materialInfo);
	}
	
	Acce_MakeAcceInformationText();
	s_isInitializedAcceMaterialInformation = true;
	return true;
}

void Acce_request_result_list(LPCHARACTER ch)
{
	RETURN_IF_ACCE_IS_NOT_OPENED(ch);
	LPCHARACTER	npc = ch->GetQuestNPC();
	if (NULL == npc)
		return;
	
	DWORD npcVNUM = npc->GetRaceNum();
	size_t resultCount = 0;
	std::string& resultText = acce_result_info_map_by_npc[npcVNUM];
	if (resultText.length() == 0)
	{
		resultText.clear();
		const TAcceResultList& resultList = acce_info_map[npcVNUM];
		for (TAcceResultList::const_iterator iter = resultList.begin(); resultList.end() != iter; ++iter)
		{
			const SAcceMaterialInfo& materialInfo = *iter;
			char temp[128];
			sprintf(temp, "%d,%d", materialInfo.reward.vnum, materialInfo.reward.count);
			
			resultText += std::string(temp) + "/";
		}
		
		if (resultCount == 0) {    return;}
		resultText.erase(resultText.size() - 1);
		if (resultCount == 0) 
		{
			return;
		}
		
		if (resultText.size() - 20 >= CHAT_MAX_LEN)
		{
			sys_err("[AcceInfo] Too long Acce result list text. (NPC: %d, length: %d)", npcVNUM, resultText.size());
			resultText.clear();
			resultCount = 0;
		}
	}
	
	ch->ChatPacket(CHAT_TYPE_COMMAND, "Acce r_list %d %d %s", npcVNUM, resultCount, resultText.c_str());
}

void Acce_request_material_info(LPCHARACTER ch, int requestStartIndex, int requestCount)
{
	RETURN_IF_ACCE_IS_NOT_OPENED(ch);
	LPCHARACTER	npc = ch->GetQuestNPC();
	if (NULL == npc)
		return;
	
	DWORD npcVNUM = npc->GetRaceNum();
	std::string materialInfoText = "";
	
	int index = 0;
	bool bCatchInfo = false;
	const TAcceResultList& resultList = acce_info_map[npcVNUM];
	
	for (TAcceResultList::const_iterator iter = resultList.begin(); resultList.end() != iter; ++iter)
	{
		const SAcceMaterialInfo& materialInfo = *iter;
		if (index++ == requestStartIndex)
		{
			bCatchInfo = true;
		}
		
		if (bCatchInfo)
		{
			materialInfoText += materialInfo.infoText + "@";
		}
		
		if (index >= requestStartIndex + requestCount)
			break;
	}
	
	if (!bCatchInfo || materialInfoText.size() == 0)
	{
		sys_err("[AcceInfo] Can't find matched material info (NPC: %d, index: %d, request count: %d)", npcVNUM, requestStartIndex, requestCount);
		return;
	}
	
	materialInfoText.erase(materialInfoText.size() - 1);
	if (materialInfoText.size() - 20 >= CHAT_MAX_LEN)
	{
		sys_err("[AcceInfo] Too long material info. (NPC: %d, requestStart: %d, requestCount: %d, length: %d)", npcVNUM, requestStartIndex, requestCount, materialInfoText.size());
	}
	
	ch->ChatPacket(CHAT_TYPE_COMMAND, "Acce m_info %d %d %s", requestStartIndex, requestCount, materialInfoText.c_str());
}

