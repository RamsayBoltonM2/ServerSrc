#define _item_combination_cpp_
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
// #define RETURN_IF_COMB_IS_NOT_OPENED(ch) if (!(ch)->IsCombOpen()) return

extern int test_server;

void Comb_open_costume(LPCHARACTER ch)
{
	if (NULL == ch)
		return;

	LPCHARACTER	npc = ch->GetQuestNPC();
	if (NULL == npc)
	{
		if (test_server)
			dev_log(LOG_DEB0, "combination_npc is NULL");
		
		return;
	}

	if (ch->IsCombOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Kombinasyon penceresi zaten acik.");
		return;
	}

	if (ch->GetExchange() || ch->GetMyShop() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsAcceOpen() || ch->IsCombOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래중(창고,교환,상점)에는 사용할 수 없습니다."));
		return;
	}

	long distance = DISTANCE_APPROX(ch->GetX() - npc->GetX(), ch->GetY() - npc->GetY());
	if (distance >= COMB_MAX_DISTANCE)
	{
		sys_log(1, "Combination: TOO_FAR: %s distance %d", ch->GetName(), distance);
		return;
	}

	Comb_clean_item(ch);
	ch->SetCombNpc(npc);
	ch->ChatPacket(CHAT_TYPE_COMMAND, "combination open_costume %d", npc->GetRaceNum());
}

void Comb_open_skillbook(LPCHARACTER ch)
{
	if (NULL == ch)
		return;

	LPCHARACTER	npc = ch->GetQuestNPC();
	if (NULL == npc)
	{
		if (test_server)
			dev_log(LOG_DEB0, "combination_npc is NULL");
		
		return;
	}

	if (ch->IsCombOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Kombinasyon penceresi zaten acik.");
		return;
	}

	if (ch->GetExchange() || ch->GetMyShop() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsAcceOpen() || ch->IsCombOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래중(창고,교환,상점)에는 사용할 수 없습니다."));
		return;
	}

	long distance = DISTANCE_APPROX(ch->GetX() - npc->GetX(), ch->GetY() - npc->GetY());
	if (distance >= COMB_MAX_DISTANCE)
	{
		sys_log(1, "Combination: TOO_FAR: %s distance %d", ch->GetName(), distance);
		return;
	}

	Comb_clean_item(ch);
	ch->SetCombNpc(npc);
	ch->ChatPacket(CHAT_TYPE_COMMAND, "combination open_skillbook %d", npc->GetRaceNum());
}

void Comb_add_item(LPCHARACTER ch, int combination_index, int inven_index)
{
	LPITEM	item;
	LPITEM	*combination_item;

	// RETURN_IF_COMB_IS_NOT_OPENED(ch);

	if (inven_index < 0 || INVENTORY_MAX_NUM <= inven_index)
		return;

	if (combination_index < 0 || COMB_MAX_NUM <= combination_index)
		return;

	item = ch->GetInventoryItem(inven_index);
	if (item == NULL)
		return;

	combination_item = ch->GetCombItem();
	for (int i = 0; i < COMB_MAX_NUM; ++i)
	{
		if (item == combination_item[i])
		{
			combination_item[i] = NULL;
			break;
		}
	}

	combination_item[combination_index] = item;

	if (test_server)
		ch->ChatPacket(CHAT_TYPE_INFO, "combination[%d]: inventory[%d]: %s added", combination_index, inven_index, item->GetName());

	return;
}

void Comb_add_item2(LPCHARACTER ch, int combination_index, int inven_index)
{
	LPITEM	item;
	LPITEM	*combination_item;

	// RETURN_IF_COMB_IS_NOT_OPENED(ch);

	if (inven_index < 0 || INVENTORY_MAX_NUM <= inven_index)
		return;

	if (combination_index < 0 || COMB_MAX_NUM <= combination_index)
		return;

	item = ch->GetInventoryItem(inven_index);
	if (item == NULL)
		return;

	combination_item = ch->GetCombItem();
	for (int i = 0; i < COMB_MAX_NUM; ++i)
	{
		if (item == combination_item[i])
		{
			combination_item[i] = NULL;
			break;
		}
	}

	combination_item[combination_index] = item;

	if (test_server)
		ch->ChatPacket(CHAT_TYPE_INFO, "combination[%d]: inventory[%d]: %s added", combination_index, inven_index, item->GetName());

	ch->ChatPacket(CHAT_TYPE_INFO, "combination[%d]: inventory[%d]: %s added", combination_index, inven_index, item->GetName());

	return;
}

void Comb_del_item(LPCHARACTER ch, int combination_index)
{
	LPITEM	item;
	LPITEM	*combination_item;

	// RETURN_IF_COMB_IS_NOT_OPENED(ch);
	if (combination_index < 0 || COMB_MAX_NUM <= combination_index)
		return;

	combination_item = ch->GetCombItem();
	if (combination_item[combination_index] == NULL)
		return;

	item = combination_item[combination_index];

	combination_item[combination_index] = NULL;
	if (test_server)
		ch->ChatPacket(CHAT_TYPE_INFO, "combination[%d]: combination[%d]: %s deleted", combination_index, item->GetCell(), item->GetName());

	return;
}

bool Comb_make_cotume(LPCHARACTER ch)
{
	LPCHARACTER	npc;
	LPITEM	*items;

	if (!(ch)->IsCombOpen())
	{
		(ch)->ChatPacket(CHAT_TYPE_INFO, "Kombinasyon penceresi acik degil.");
		return false;
	}

	npc = ch->GetQuestNPC();
	if (NULL == npc)
	{
		return false;
	}

	items = ch->GetCombItem();
	if (items[0] == NULL || items[1] == NULL || items[2] == NULL)
	{
		(ch)->ChatPacket(CHAT_TYPE_INFO, "Kombinasyon penceresine ogeleri surukleyin.");
		return false;
	}
	else if (items[2] == NULL)
	{
		(ch)->ChatPacket(CHAT_TYPE_INFO, "Bonuslari degistirmek icin ikinci kostum gereklidir.");
		return false;
	}
	else if (items[1]->GetType() != ITEM_COSTUME && items[1]->GetSubType() != COSTUME_BODY || 
			items[1]->GetType() != ITEM_COSTUME && items[1]->GetSubType() != COSTUME_HAIR || 
			items[1]->GetType() != ITEM_COSTUME && items[1]->GetSubType() != COSTUME_WEAPON_SWORD || 
			items[1]->GetType() != ITEM_COSTUME && items[1]->GetSubType() != COSTUME_WEAPON_DAGGER || 
			items[1]->GetType() != ITEM_COSTUME && items[1]->GetSubType() != COSTUME_WEAPON_BOW || 
			items[1]->GetType() != ITEM_COSTUME && items[1]->GetSubType() != COSTUME_WEAPON_TWO_HANDED || 
			items[1]->GetType() != ITEM_COSTUME && items[1]->GetSubType() != COSTUME_WEAPON_BELL || 
			items[1]->GetType() != ITEM_COSTUME && items[1]->GetSubType() != COSTUME_WEAPON_FAN || 
			items[2]->GetType() != ITEM_COSTUME && items[2]->GetSubType() != COSTUME_BODY || 
			items[2]->GetType() != ITEM_COSTUME && items[2]->GetSubType() != COSTUME_HAIR || 
			items[2]->GetType() != ITEM_COSTUME && items[2]->GetSubType() != COSTUME_WEAPON_SWORD || 
			items[2]->GetType() != ITEM_COSTUME && items[2]->GetSubType() != COSTUME_WEAPON_DAGGER || 
			items[2]->GetType() != ITEM_COSTUME && items[2]->GetSubType() != COSTUME_WEAPON_BOW || 
			items[2]->GetType() != ITEM_COSTUME && items[2]->GetSubType() != COSTUME_WEAPON_TWO_HANDED || 
			items[2]->GetType() != ITEM_COSTUME && items[2]->GetSubType() != COSTUME_WEAPON_BELL || 
			items[2]->GetType() != ITEM_COSTUME && items[2]->GetSubType() != COSTUME_WEAPON_FAN)
	{
		(ch)->ChatPacket(CHAT_TYPE_INFO, "Sadece kostum efsunlarini degistirebilirsiniz.");
		return false;
	}
	else if (items[1]->IsEquipped() || items[2]->IsEquipped())
	{
		(ch)->ChatPacket(CHAT_TYPE_INFO, "Bu islemi uygulayamam.");
		return false;
	}
#ifdef __GF_SEAL_ITEM_UPDATE__
	else if (items[0]->IsBind() || items[0]->IsUntilBind() || items[1]->IsBind() || items[1]->IsUntilBind() || items[2]->IsBind() || items[2]->IsUntilBind())
	{
		(ch)->ChatPacket(CHAT_TYPE_INFO, "Kilitli itemi degistiremezsin.");
		return false;
	}
#endif
	else if (items[2]->GetAttributeCount() < 1)
	{
		(ch)->ChatPacket(CHAT_TYPE_INFO, "ikinci kostum bonusunuz yok.");
		return false;
	}

	items[1]->ClearAttribute();
	for (int i = 0; i < items[2]->GetAttributeCount(); i++){
		items[1]->SetForceAttribute(i, items[2]->GetAttributeType(i), items[2]->GetAttributeValue(i));
	}

	ch->ChatPacket(CHAT_TYPE_INFO, "Kombinasyon basarili!");
	ch->ChatPacket(CHAT_TYPE_COMMAND, "combination success_costume");

	items[0]->RemoveFromCharacter();
	items[2]->RemoveFromCharacter();

	items[2]->SetCount(0);
	items[2] = NULL;
	return true;
}

bool Comb_make_skillbook(LPCHARACTER ch)
{
	LPCHARACTER	npc;
	LPITEM	*items;
	int	i_number = 0;

	if (!(ch)->IsCombOpen())
	{
		(ch)->ChatPacket(CHAT_TYPE_INFO, "Kombinasyon penceresi acik degil.");
		return false;
	}

	npc = ch->GetQuestNPC();
	if (NULL == npc)
	{
		return false;
	}

	items = ch->GetCombItem();

	if (ch->GetGold() < 1000000)
	{
		(ch)->ChatPacket(CHAT_TYPE_INFO, "Kitap degisimi yapmak icin yerli yang yok.");
		return false;
	}
	
	else if (ch->GetSkillGroup() == 0)
	{
		(ch)->ChatPacket(CHAT_TYPE_INFO, "Kitap degisimi yapabilmen icin becerileri olusturmalisin.");
		return false;
	}

	for (int i = 0; i < COMB_MAX_NUM; ++i)
	{
		if (items[i] == NULL)
		{
			(ch)->ChatPacket(CHAT_TYPE_INFO, "10 Kitap karsiliginda degistirebilirim.");
			return false;
		}
		else if (items[i]->GetVnum() != 50300)
		{
			(ch)->ChatPacket(CHAT_TYPE_INFO, "Sadece bk kitaplari degistirilir.");
			return false;
		}
#ifdef __GF_SEAL_ITEM_UPDATE__
		else if (items[i]->IsBind() || items[i]->IsUntilBind())
		{
			(ch)->ChatPacket(CHAT_TYPE_INFO, "Kilitli iteme bu islemi yapamazsin.");
			return false;
		}
#endif
	}

	if (ch->GetJob() == JOB_WARRIOR)
	{
		if (ch->GetSkillGroup() == 1)
		{
			i_number = number(1,6);
		}
		else if (ch->GetSkillGroup() == 2)
		{
			i_number = number(16,21);
		}
	}
	else if (ch->GetJob() == JOB_ASSASSIN)
	{
		if (ch->GetSkillGroup() == 1)
		{
			i_number = number(31,36);
		}
		else if (ch->GetSkillGroup() == 2)
		{
			i_number = number(46,51);
		}
	}
	else if (ch->GetJob() == JOB_SURA)
	{
		if (ch->GetSkillGroup() == 1)
		{
			i_number = number(61,66);
		}
		else if (ch->GetSkillGroup() == 2)
		{
			i_number = number(76,81);
		}
	}
	else if (ch->GetJob() == JOB_SHAMAN)
	{
		if (ch->GetSkillGroup() == 1)
		{
			i_number = number(91,96);
		}
		else if (ch->GetSkillGroup() == 2)
		{
			i_number = number(106,111);
		}
	}
	else if (ch->GetJob() == JOB_WOLFMAN)
	{
		if (ch->GetSkillGroup() == 1)
		{
			i_number = number(170,175);
		}
	}

	items[0]->RemoveFromCharacter();
	items[1]->RemoveFromCharacter();
	items[2]->RemoveFromCharacter();
	items[3]->RemoveFromCharacter();
	items[4]->RemoveFromCharacter();
	items[5]->RemoveFromCharacter();
	items[6]->RemoveFromCharacter();
	items[7]->RemoveFromCharacter();
	items[8]->RemoveFromCharacter();
	items[9]->RemoveFromCharacter();
	ch->PointChange(POINT_GOLD, -1000000, false);
	ch->AutoGiveItem(50300)->SetSocket(0, i_number);

	ch->ChatPacket(CHAT_TYPE_INFO, "Kombinasyon basarili!");
	ch->ChatPacket(CHAT_TYPE_COMMAND, "combination success_skillbook");

	return true;
}

void Comb_close(LPCHARACTER ch)
{
	// RETURN_IF_COMB_IS_NOT_OPENED(ch);
	Comb_clean_item(ch);
	ch->SetCombNpc(NULL);
	ch->ChatPacket(CHAT_TYPE_COMMAND, "combination close");
	dev_log(LOG_DEB0, "<combination> close (%s)", ch->GetName());
}

void Comb_clean_item(LPCHARACTER ch)
{
	LPITEM	*combination_item;

	combination_item = ch->GetCombItem();
	for (int i = 0; i < COMB_MAX_NUM; ++i)
	{
		if (combination_item[i] == NULL)
			continue;

		combination_item[i] = NULL;
	}
}

