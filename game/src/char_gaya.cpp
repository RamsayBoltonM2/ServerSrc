#include "stdafx.h"
#include "constants.h"
#include "utils.h"
#include "config.h"
#include "char.h"
#include "item.h"
#include "item_manager.h"
#include "locale_service.h"
#include "questmanager.h"

const char* txt_gaya[] = {
	"[Gaya] Yeterli gaya yok.",
	"[Gaya] Hata.",
	"[Gaya] %d adet %s gerekli.",
	"[Gaya] Yeterli yang yok.",
	"[Gaya] Gaya miktari: %d.",
	"[Gaya] Bileme islemi basarili oldu.",
	"[Gaya] Bileme islemi basarisiz oldu."
};

void CHARACTER::lOAD_GAYA()
{
	FILE 	*fp;
	char	one_line[256];
	int		value1, value2, value3;
	const char	*delim = " \t\r\n";
	char	*v, *token_string;
	char file_name[256+1];

	Gaya_Load_Values gaya_values = {0,0,0};

	snprintf(file_name, sizeof(file_name), "%s/gaya.txt", LocaleService_GetBasePath().c_str());
	fp = fopen(file_name, "r");


	while (fgets(one_line, 256, fp))
	{
		value1 = value2 = value3 = 0;

		if (one_line[0] == '#')
			continue;
		
		token_string = strtok(one_line, delim);
		if (NULL == token_string)
			continue;
		
		if ((v = strtok(NULL, delim)))
			str_to_number(value1, v);
		
		if ((v = strtok(NULL, delim)))
			str_to_number(value2, v);

		if ((v = strtok(NULL, delim)))
			str_to_number(value3, v);


		TOKEN("ITEM")
		{
			gaya_values.items = value1;
			gaya_values.gaya  = value2;
			gaya_values.count = value3;
			load_gaya_items.push_back(gaya_values);
		}
		else TOKEN("GLIMMERSTONE"){load_gaya_values.glimmerstone = value1;}
		else TOKEN("GAYA_EXPANSION"){load_gaya_values.gaya_expansion = value1;}
		else TOKEN("GAYA_REFRESH"){load_gaya_values.gaya_refresh = value1;}
		else TOKEN("GLIMMERSTONE_COUNT"){load_gaya_values.glimmerstone_count = value1;}
		else TOKEN("GRADE_STONE"){load_gaya_values.grade_stone = value1;}
		else TOKEN("GIVE_GAYA"){load_gaya_values.give_gaya = value1;}
		else TOKEN("PROB_GAYA"){load_gaya_values.prob_gaya = value1;}
		else TOKEN("COST_GAYA_YANG"){load_gaya_values.cost_gaya_yang = value1;}
		else TOKEN("GAYA_EXPANSION_COUNT"){load_gaya_values.gaya_expansion_count = value1;}
		else TOKEN("GAYA_REFRESH_COUNT"){load_gaya_values.gaya_refresh_count = value1;}
	}
	fclose(fp);
}

bool CHARACTER::CheckItemsFull()
{
	FILE 	*fp;
	char file_name[256+1];
	snprintf(file_name, sizeof(file_name), "%s/gaya/%s_gaya_system.txt", LocaleService_GetBasePath().c_str(),GetName());
	if ((fp = fopen(file_name, "r")) == 0)
	{return false;}
	else{return true;}
}

void CHARACTER::ClearGayaMarket()
{
	info_items.clear();
	info_slots.clear();
}

void CHARACTER::InfoGayaMarker()
{
	ClearGayaMarket();
	UpdateItemsGayaMarker0();
	ChatPacket(CHAT_TYPE_COMMAND, "GayaMarketClear");
	for (int i=0; i<info_items.size(); ++i){
		ChatPacket(CHAT_TYPE_COMMAND, "GayaMarketItems %d %d %d",info_items[i].value_1,info_items[i].value_2,info_items[i].value_3);
	}
	if (info_slots.empty())
		return;

	ChatPacket(CHAT_TYPE_COMMAND, "GayaMarketSlotsDesblock %d %d %d %d %d %d",info_slots[0].value_1,info_slots[0].value_2,info_slots[0].value_3,info_slots[0].value_4,info_slots[0].value_5,info_slots[0].value_6);
	ChatPacket(CHAT_TYPE_COMMAND, "GayaMarketTime %d",GetGayaState("system_gaya.gaya_time_world_4")); 
}

bool CHARACTER::CheckSlotGayaMarket(int slot)
{
	if(slot == 3){if(info_slots[0].value_1 == 0){return false;}else{return true;}}
	else if(slot == 4){if(info_slots[0].value_2 == 0){return false;}else{return true;}}
	else if(slot == 5){if(info_slots[0].value_3 == 0){return false;}else{return true;}}
	else if(slot == 6){if(info_slots[0].value_4 == 0){return false;}else{return true;}}
	else if(slot == 7){if(info_slots[0].value_5 == 0){return false;}else{return true;}}
	else if(slot == 8){if(info_slots[0].value_6 == 0){return false;}else{return true;}}
}

void CHARACTER::BuyItemsGayaMarket(int slot)
{	
	if(GetGaya() >=  info_items[slot].value_2){
		PointChange(POINT_GAYA, -info_items[slot].value_2);
		AutoGiveItem(info_items[slot].value_1,info_items[slot].value_3);
	}else{
		ChatPacket(CHAT_TYPE_INFO,txt_gaya[0]);
		return;
	}
}

void CHARACTER::RefreshItemsGayaMarket()
{
	FILE 	*fileID;
	char file_name[256+1];

	snprintf(file_name, sizeof(file_name), "%s/gaya/%s_gaya_system.txt", LocaleService_GetBasePath().c_str(),GetName());
	fileID = fopen(file_name, "w");
	
	if (NULL == fileID)
		return;

	for (int i = 0; i<9; ++i){
		int number = number(1, load_gaya_items.size()-1);
		fprintf(fileID,"Item	%d	%d	%d\n",load_gaya_items[number].items,load_gaya_items[number].gaya,load_gaya_items[number].count);
	}
	
	if (info_slots.empty())
		return;

	fprintf(fileID, "Slots_Desblock	%d	%d	%d	%d	%d	%d\n",info_slots[0].value_1,info_slots[0].value_2,info_slots[0].value_3,info_slots[0].value_4,info_slots[0].value_5,info_slots[0].value_6);
	fclose(fileID);
}

void CHARACTER::UpdateSlotGayaMarket(int slot)
{
	FILE 	*fileID;
	char file_name[256+1];
	snprintf(file_name, sizeof(file_name), "%s/gaya/%s_gaya_system.txt", LocaleService_GetBasePath().c_str(),GetName());
	fileID = fopen(file_name, "w");
	for (int i = 0; i<9; ++i){
		fprintf(fileID,"Item	%d	%d	%d\n",info_items[i].value_1,info_items[i].value_2,info_items[i].value_3);
	}

	if(slot == 3){info_slots[0].value_1 = 1;}
	else if(slot == 4){info_slots[0].value_2 = 1;}
	else if(slot == 5){info_slots[0].value_3 = 1;}
	else if(slot == 6){info_slots[0].value_4 = 1;}
	else if(slot == 7){info_slots[0].value_5 = 1;}
	else if(slot == 8){info_slots[0].value_6 = 1;}

	fprintf(fileID, "Slots_Desblock	%d	%d	%d	%d	%d	%d\n",info_slots[0].value_1,info_slots[0].value_2,info_slots[0].value_3,info_slots[0].value_4,info_slots[0].value_5,info_slots[0].value_6);
	fclose(fileID);
	InfoGayaMarker();
}

void CHARACTER::UpdateItemsGayaMarker0()
{
	FILE 	*fp;
	char	one_line[256];
	int		value1, value2, value3, value4, value5, value6;
	const char	*delim = " \t\r\n";
	char	*v, *token_string;
	char file_name[256+1];

	Gaya_Shop_Values market_gaya_values_0 = {0,0,0};
	Gaya_Shop_Values market_gaya_values_1 = {0,0,0,0,0,0};	

	snprintf(file_name, sizeof(file_name), "%s/gaya/%s_gaya_system.txt", LocaleService_GetBasePath().c_str(),GetName());
	fp = fopen(file_name, "r");


	while (fgets(one_line, 256, fp))
	{
		value1 = value2 = value3 = value4 = value5 = value6 = 0;

		if (one_line[0] == '#')
			continue;
		
		token_string = strtok(one_line, delim);
		if (NULL == token_string)
			continue;
		
		if ((v = strtok(NULL, delim)))
			str_to_number(value1, v);
		
		if ((v = strtok(NULL, delim)))
			str_to_number(value2, v);

		if ((v = strtok(NULL, delim)))
			str_to_number(value3, v);

		if ((v = strtok(NULL, delim)))
			str_to_number(value4, v);

		if ((v = strtok(NULL, delim)))
			str_to_number(value5, v);

		if ((v = strtok(NULL, delim)))
			str_to_number(value6, v);


		TOKEN("Item")
		{
			market_gaya_values_0.value_1 = value1;
			market_gaya_values_0.value_2 = value2;
			market_gaya_values_0.value_3 = value3;
			info_items.push_back(market_gaya_values_0);
		}
		else TOKEN("Slots_Desblock")
		{
			market_gaya_values_1.value_1 = value1;
			market_gaya_values_1.value_2 = value2;
			market_gaya_values_1.value_3 = value3;
			market_gaya_values_1.value_4 = value4;
			market_gaya_values_1.value_5 = value5;
			market_gaya_values_1.value_6 = value6;
			info_slots.push_back(market_gaya_values_1);
		}
	}
	fclose(fp);
}

void CHARACTER::UpdateItemsGayaMarker()
{
	FILE	*fileID;
	char	one_line[256];
	char file_name[256+1];
	int 	slot_block = 0;
	snprintf(file_name, sizeof(file_name), "%s/gaya/%s_gaya_system.txt", LocaleService_GetBasePath().c_str(),GetName());
	fileID = fopen(file_name, "a");

	//ChatPacket(CHAT_TYPE_COMMAND, "GayaMarketClear");

	for (int i = 0; i<9; ++i){
		int number = number(1, load_gaya_items.size()-1);
		fprintf(fileID,"Item	%d	%d	%d\n",load_gaya_items[number].items,load_gaya_items[number].gaya,load_gaya_items[number].count);
		//ChatPacket(CHAT_TYPE_COMMAND, "GayaMarketItems %d %d %d",load_gaya_items[number].items,load_gaya_items[number].gaya,load_gaya_items[number].count);
	}
	fprintf(fileID, "Slots_Desblock	%d	%d	%d	%d	%d	%d\n", 0,0,0,0,0,0);
	fclose(fileID);
	//ChatPacket(CHAT_TYPE_COMMAND, "GayaMarketSlotsDesblock %d %d %d %d %d %d",0,0,0,0,0,0);
}

void CHARACTER::CraftGayaItems(int slot)
{
		LPITEM	item = GetItem(TItemPos(INVENTORY, slot));

		int ID_Glimmerstone = load_gaya_values.glimmerstone;
		int Count_Glimmerstone = load_gaya_values.glimmerstone_count;
		int Grade_Stone = load_gaya_values.grade_stone;
		int Point_Gaya = load_gaya_values.give_gaya;
		int Random_Point_Gaya = number(1, 100);
		int Prob_Gaya = load_gaya_values.prob_gaya;
		int Cost_Gaya_Yang = load_gaya_values.cost_gaya_yang;

		LPITEM  item_glimmerstone = ITEM_MANAGER::instance().CreateItem(ID_Glimmerstone, Count_Glimmerstone, 0, true);

		if(NULL == item){ //Null item
			return;
		}

		if (item->GetType() != ITEM_METIN || item->GetRefineLevel() > Grade_Stone){ // Item diferente a stone o diferente nivel
			ChatPacket(CHAT_TYPE_INFO, txt_gaya[1]);
			return;
		}

		if (CountSpecifyItem(ID_Glimmerstone) < Count_Glimmerstone){ //Check count Glimmerstone max 10
			ChatPacket(CHAT_TYPE_INFO, txt_gaya[2],Count_Glimmerstone,item_glimmerstone->GetName());
			return;
		}

		if(GetGold() < Cost_Gaya_Yang){
			ChatPacket(CHAT_TYPE_INFO, txt_gaya[3]);
			return;
		}

		if (Random_Point_Gaya <= Prob_Gaya){

			PointChange(POINT_GAYA, Point_Gaya);
			ChatPacket(CHAT_TYPE_INFO, txt_gaya[4],Point_Gaya);
			ChatPacket(CHAT_TYPE_INFO, txt_gaya[5]);

		}else{
			ChatPacket(CHAT_TYPE_INFO, txt_gaya[6]);
		}

		ChatPacket(CHAT_TYPE_COMMAND, "GayaCheck");
		RemoveSpecifyItem(ID_Glimmerstone, Count_Glimmerstone);
		PointChange(POINT_GOLD, -Cost_Gaya_Yang);
		item->SetCount(item->GetCount()-1);
}

void CHARACTER::MarketGayaItems(int slot)
{
	if(CheckItemsFull() == false)
	{
		ChatPacket(CHAT_TYPE_INFO, txt_gaya[1]);
		return;
	}
	if (slot > 8){
		ChatPacket(CHAT_TYPE_INFO, txt_gaya[1]);
		return;
	}
	int ID_GayaMarketExpansion = load_gaya_values.gaya_expansion;
	LPITEM  item_gayarexpansion = ITEM_MANAGER::instance().CreateItem(ID_GayaMarketExpansion, load_gaya_values.gaya_expansion_count, 0, true);

	if (slot >= 3){
		if(CheckSlotGayaMarket(slot) == false){
			if(CountSpecifyItem(ID_GayaMarketExpansion) >= load_gaya_values.gaya_expansion_count){
				RemoveSpecifyItem(ID_GayaMarketExpansion, load_gaya_values.gaya_expansion_count);
				UpdateSlotGayaMarket(slot);
				return;
			}else{
				ChatPacket(CHAT_TYPE_INFO, txt_gaya[2],load_gaya_values.gaya_expansion_count,item_gayarexpansion->GetName());
				return;
			}
		}else{
			BuyItemsGayaMarket(slot);
			return;
		}
	}else{
		BuyItemsGayaMarket(slot);
		return;
	}
	return;
}

void CHARACTER::RefreshGayaItems()
{
	if(CheckItemsFull() == false){
		ChatPacket(CHAT_TYPE_INFO, txt_gaya[1]);
		return;
	}

	int ID_GayaMarketRefresh = load_gaya_values.gaya_refresh;
	LPITEM  item_gayarefresh = ITEM_MANAGER::instance().CreateItem(ID_GayaMarketRefresh, load_gaya_values.gaya_refresh_count, 0, true);

	if (!item_gayarefresh) {
		return;
	}

	if(CountSpecifyItem(ID_GayaMarketRefresh) < load_gaya_values.gaya_refresh_count)
	{
		ChatPacket(CHAT_TYPE_INFO, txt_gaya[2],load_gaya_values.gaya_refresh_count,item_gayarefresh->GetName());
		return;
	}
		
	RemoveSpecifyItem(ID_GayaMarketRefresh, load_gaya_values.gaya_refresh_count);
	RefreshItemsGayaMarket();
	InfoGayaMarker();
}

int CHARACTER::GetGayaState(const std::string& state) const
{
	quest::CQuestManager& q = quest::CQuestManager::instance();
	quest::PC* pPC = q.GetPC(GetPlayerID());
	return pPC->GetFlag(state);
}

void CHARACTER::SetGayaState(const std::string& state, int szValue)
{
	quest::CQuestManager& q = quest::CQuestManager::instance();
	quest::PC* pPC = q.GetPC(GetPlayerID());
	pPC->SetFlag(state, szValue);
}

EVENTFUNC(check_time_market_event)
{

	char_event_info* info = dynamic_cast<char_event_info*>( event->info );
	if ( info == NULL )
	{
		sys_err( "check_time_market_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER	ch = info->ch;

	if (NULL == ch || ch->IsNPC())
		return 0;

	if (ch->GetGayaState("system_gaya.gaya_time_world_4") - init_gayaTime() <= 0)
	{
		ch->SetGayaState("system_gaya.gaya_time_world_4", init_gayaTime() +  (60*60*5));
		ch->RefreshItemsGayaMarket();
		ch->InfoGayaMarker();
	}
	
	return PASSES_PER_SEC(2);

}
void CHARACTER::StartCheckTimeMarket()
{
	if (GayaUpdateTime)
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();

	info->ch = this;

	GayaUpdateTime = event_create(check_time_market_event, info, PASSES_PER_SEC(5));	// 1ºÐ
}