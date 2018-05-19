/*
	* Filename : offline_shop.cpp
	* Version : 0.1
	* Description : --
*/

#include "stdafx.h"
#include "../../libgame/include/grid.h"
#include "constants.h"
#include "utils.h"
#include "config.h"
#include "desc.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "item.h"
#include "item_manager.h"
#include "buffer_manager.h"
#include "packet.h"
#include "log.h"
#include "db.h"
#include "questmanager.h"
#include "monarch.h"
#include "mob_manager.h"
#include "locale_service.h"
#include "offline_shop.h"
#include "p2p.h"
#include "offlineshop_manager.h"
#include "desc_client.h"
#include "target.h"

COfflineShop::COfflineShop() : 
	m_pkOfflineShopNPC(NULL), 
	m_dwDisplayedCount(0),
	m_llMapIndex(0), 
	m_iTime(0), 
	llMoney(0), 
	dwBar1(0), 
	dwBar2(0), 
	dwBar3(0), 
	dwSoulStone(0),
	dwDragonScale(0),
	dwDragonClaw(0)
#ifdef __CHEQUE_SYSTEM__
	,dwCheque(0)
#endif
{
	m_pGrid = M2_NEW CGrid(5, 9);
}

COfflineShop::~COfflineShop()
{
	TPacketGCShop pack;
	pack.header = HEADER_GC_OFFLINE_SHOP;
	pack.subheader = SHOP_SUBHEADER_GC_END;
	pack.size = sizeof(TPacketGCShop);

	Broadcast(&pack, sizeof(pack));

	for (GuestMapType::iterator it = m_map_guest.begin(); it != m_map_guest.end(); ++it)
	{
		LPCHARACTER ch = it->first;
		ch->SetOfflineShop(NULL);
	}

	M2_DELETE(m_pGrid);
}

void COfflineShop::SetOfflineShopNPC(LPCHARACTER npc)
{
	m_pkOfflineShopNPC = npc;
}

void COfflineShop::CreateTable(DWORD dwOwnerID)
{
	MYSQL_ROW row;
	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT * FROM %soffline_shop_item WHERE owner_id = %u", get_table_postfix(), dwOwnerID));
	
	m_itemVector.resize(OFFLINE_SHOP_HOST_ITEM_MAX_NUM);
	memset(&m_itemVector[0], 0, sizeof(OFFLINE_SHOP_ITEM) * m_itemVector.size());
	
	if (pMsg->Get()->uiNumRows != 0)
	{
		while (NULL != (row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
		{
			const TItemTable * item_table;
		
			DWORD vnum = 0;
			str_to_number(vnum, row[4]);
			BYTE pos = 0;
			str_to_number(pos, row[2]);
		
			item_table = ITEM_MANAGER::instance().GetTable(vnum);
			
			if (!item_table)
			{
				sys_err("OfflineShop: no item table by item vnum #%d", vnum);
				continue;
			}
						
			OFFLINE_SHOP_ITEM & item = m_itemVector[pos];
			
			str_to_number(item.id, row[0]);
			str_to_number(item.owner_id, row[1]);
			str_to_number(item.pos, row[2]);
			str_to_number(item.count, row[3]);
			str_to_number(item.vnum, row[4]);
			str_to_number(item.alSockets[0], row[5]);
			str_to_number(item.alSockets[1], row[6]);
			str_to_number(item.alSockets[2], row[7]);
			str_to_number(item.alSockets[3], row[8]);
			str_to_number(item.aAttr[0].bType, row[9]);
			str_to_number(item.aAttr[0].sValue, row[10]);
			str_to_number(item.aAttr[1].bType, row[11]);
			str_to_number(item.aAttr[1].sValue, row[12]);
			str_to_number(item.aAttr[2].bType, row[13]);
			str_to_number(item.aAttr[2].sValue, row[14]);
			str_to_number(item.aAttr[3].bType, row[15]);
			str_to_number(item.aAttr[3].sValue, row[16]);
			str_to_number(item.aAttr[4].bType, row[17]);
			str_to_number(item.aAttr[4].sValue, row[18]);
			str_to_number(item.aAttr[5].bType, row[19]);
			str_to_number(item.aAttr[5].sValue, row[20]);
			str_to_number(item.aAttr[6].bType, row[21]);
			str_to_number(item.aAttr[6].sValue, row[22]);
			str_to_number(item.aAttr[7].bType, row[23]);
			str_to_number(item.aAttr[7].sValue, row[24]);
			str_to_number(item.aAttr[8].bType, row[25]);
			str_to_number(item.aAttr[8].sValue, row[26]);
			str_to_number(item.aAttr[9].bType, row[27]);
			str_to_number(item.aAttr[9].sValue, row[28]);
			str_to_number(item.aAttr[10].bType, row[29]);
			str_to_number(item.aAttr[10].sValue, row[30]);
			str_to_number(item.aAttr[11].bType, row[31]);
			str_to_number(item.aAttr[11].sValue, row[32]);
			str_to_number(item.aAttr[12].bType, row[33]);
			str_to_number(item.aAttr[12].sValue, row[34]);
			str_to_number(item.aAttr[13].bType, row[35]);
			str_to_number(item.aAttr[13].sValue, row[36]);
			str_to_number(item.aAttr[14].bType, row[37]);
			str_to_number(item.aAttr[14].sValue, row[38]);
			str_to_number(item.price, row[39]);
			str_to_number(item.price2, row[40]);
			str_to_number(item.price_type, row[41]);
			str_to_number(item.status, row[42]);
			strlcpy(item.szBuyerName, row[43], sizeof(item.szBuyerName));
#ifdef __TRANSMUTATION_SYSTEM__
			str_to_number(item.transmutation, row[44]);
#endif
			strlcpy(item.szName, row[45], sizeof(item.szName));
			str_to_number(item.refine_level, row[46]);
			DWORD vid = GetOfflineShopNPC()->GetVID();
			item.shop_id = vid;
			str_to_number(item.evolution, row[48]);
		}
	}
}

void COfflineShop::SetOfflineShopBankValues(DWORD dwOwnerPID)
{
#ifdef __CHEQUE_SYSTEM__
	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT shop_money,shop_bar1,shop_bar2,shop_bar3,shop_soul_stone,shop_dragon_scale,shop_dragon_claw,shop_cheque FROM player.player WHERE id = %u", dwOwnerPID));
#else
	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT shop_money,shop_bar1,shop_bar2,shop_bar3,shop_soul_stone,shop_dragon_scale,shop_dragon_claw FROM player.player WHERE id = %u", dwOwnerPID));
#endif
	MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
	if (pMsg->Get()->uiNumRows == 0)
	{
		SetOfflineShopGold(0);
		SetOfflineShopBar1(0);
		SetOfflineShopBar2(0);
		SetOfflineShopBar3(0);
		SetOfflineShopSoulStone(0);
		SetOfflineShopDragonScale(0);
		SetOfflineShopDragonClaw(0);
#ifdef __CHEQUE_SYSTEM__
		SetOfflineShopCheque(0);
#endif
	}
	else
	{
		long long dwMoney = 0;
		DWORD dwBar1 = 0;
		DWORD dwBar2 = 0;
		DWORD dwBar3 = 0;
		DWORD dwSoulStone = 0;
		DWORD dwDragonScale = 0;
		DWORD dwDragonClaw = 0;
#ifdef __CHEQUE_SYSTEM__
		DWORD dwCheque = 0;
#endif
		str_to_number(dwMoney, row[0]);
		str_to_number(dwBar1, row[1]);
		str_to_number(dwBar2, row[2]);
		str_to_number(dwBar3, row[3]);
		str_to_number(dwSoulStone, row[4]);
		str_to_number(dwDragonScale, row[5]);
		str_to_number(dwDragonClaw, row[6]);
#ifdef __CHEQUE_SYSTEM__
		str_to_number(dwCheque, row[7]);
#endif
		SetOfflineShopGold(dwMoney);
		SetOfflineShopBar1(dwBar1);
		SetOfflineShopBar2(dwBar2);
		SetOfflineShopBar3(dwBar3);
		SetOfflineShopSoulStone(dwSoulStone);
		SetOfflineShopDragonScale(dwDragonScale);
		SetOfflineShopDragonClaw(dwDragonClaw);
#ifdef __CHEQUE_SYSTEM__
		SetOfflineShopCheque(dwCheque);
#endif
	}
}

void COfflineShop::SetOfflineShopItems(DWORD dwOwnerPID, TOfflineShopItemTable * pTable, BYTE bItemCount)
{
	if (bItemCount > OFFLINE_SHOP_HOST_ITEM_MAX_NUM)
		return;
	
	m_pGrid->Clear();
	
	m_itemVector.resize(OFFLINE_SHOP_HOST_ITEM_MAX_NUM);
	memset(&m_itemVector[0], 0, sizeof(OFFLINE_SHOP_ITEM) * m_itemVector.size());
	
	for (int i = 0; i < bItemCount; ++i)
	{
		const TItemTable * item_table;
		
		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(dwOwnerPID);
		LPITEM pkItem = ch->GetItem(pTable->pos);
		
		if (!pkItem)
			continue;
		
		if (!pkItem->GetVnum())
			continue;
		
		item_table = ITEM_MANAGER::instance().GetTable(pkItem->GetVnum());
		
		if (!item_table)
		{
			sys_err("OfflineShop: no item table by item vnum #%d", pkItem->GetVnum());
			continue;
		}

		OFFLINE_SHOP_ITEM & item = m_itemVector[pTable->display_pos];
		
		item.id = pkItem->GetID();
		item.owner_id = ch->GetPlayerID();
		item.pos = pTable->display_pos;
		item.count = pkItem->GetCount();
		item.price = pTable->price;
		item.price2 = pTable->price2;
		item.price_type = pTable->price_type;
		item.vnum = pkItem->GetVnum();
		item.status = 0;
		for (int i=0; i < ITEM_SOCKET_MAX_NUM; ++i)
			item.alSockets[i] = pkItem->GetSocket(i);
		for (int i=0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
		{
			item.aAttr[i].bType = pkItem->GetAttributeType(i);
			item.aAttr[i].sValue = pkItem->GetAttributeValue(i);
		}
		
#ifdef __TRANSMUTATION_SYSTEM__
		item.transmutation = pkItem->GetTransmutation();
#endif

#ifdef __PRIVATESHOP_SEARCH_SYSTEM__
		strlcpy(item.szName, pkItem->GetName(), sizeof(item.szName));
		item.refine_level = pkItem->GetRefineLevel();
		item.shop_id = GetOfflineShopNPC()->GetVID();
#endif
		
		item.evolution = pkItem->GetEvolution();
		
		char szColumns[QUERY_MAX_LEN], szValues[QUERY_MAX_LEN];

		int iLen = snprintf(szColumns, sizeof(szColumns), "id,owner_id,pos,count,price,price2,price_type,vnum");
		int iUpdateLen = snprintf(szValues, sizeof(szValues), "%u,%u,%d,%u,%d,%d,%d,%u", pkItem->GetID(), ch->GetPlayerID(), pTable->display_pos, pkItem->GetCount(), pTable->price, pTable->price2, pTable->price_type, pkItem->GetVnum());

		iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ",socket0,socket1,socket2,socket3");
		iUpdateLen += snprintf(szValues + iUpdateLen, sizeof(szValues) - iUpdateLen, ",%ld,%ld,%ld,%ld", pkItem->GetSocket(0), pkItem->GetSocket(1), pkItem->GetSocket(2), pkItem->GetSocket(3));

		iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ", attrtype0, attrvalue0, attrtype1, attrvalue1, attrtype2, attrvalue2, attrtype3, attrvalue3, attrtype4, attrvalue4, attrtype5, attrvalue5, attrtype6, attrvalue6, applytype0, applyvalue0, applytype1, applyvalue1, applytype2, applyvalue2, applytype3, applyvalue3, applytype4, applyvalue4, applytype5, applyvalue5, applytype6, applyvalue6, applytype7, applyvalue7");
		iUpdateLen += snprintf(szValues + iUpdateLen, sizeof(szValues) - iUpdateLen, ",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
			pkItem->GetAttributeType(0), pkItem->GetAttributeValue(0),
			pkItem->GetAttributeType(1), pkItem->GetAttributeValue(1),
			pkItem->GetAttributeType(2), pkItem->GetAttributeValue(2),
			pkItem->GetAttributeType(3), pkItem->GetAttributeValue(3),
			pkItem->GetAttributeType(4), pkItem->GetAttributeValue(4),
			pkItem->GetAttributeType(5), pkItem->GetAttributeValue(5),
			pkItem->GetAttributeType(6), pkItem->GetAttributeValue(6),
			pkItem->GetAttributeType(7), pkItem->GetAttributeValue(7),
			pkItem->GetAttributeType(8), pkItem->GetAttributeValue(8),
			pkItem->GetAttributeType(9), pkItem->GetAttributeValue(9),
			pkItem->GetAttributeType(10), pkItem->GetAttributeValue(10),
			pkItem->GetAttributeType(11), pkItem->GetAttributeValue(11),
			pkItem->GetAttributeType(12), pkItem->GetAttributeValue(12),
			pkItem->GetAttributeType(13), pkItem->GetAttributeValue(13),
			pkItem->GetAttributeType(14), pkItem->GetAttributeValue(14));
			
#ifdef __TRANSMUTATION_SYSTEM__
		iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ",transmutation");
		iUpdateLen += snprintf(szValues + iUpdateLen, sizeof(szValues) - iUpdateLen, ",%d", pkItem->GetTransmutation());
#endif

#ifdef __PRIVATESHOP_SEARCH_SYSTEM__
		iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ",item_name,refine_level,shop_id");
		iUpdateLen += snprintf(szValues + iUpdateLen, sizeof(szValues) - iUpdateLen, ",'%s',%d,%d", ch->GetName(), pkItem->GetRefineLevel(), (DWORD) GetOfflineShopNPC()->GetVID());
#endif

		iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ",evolution");
		iUpdateLen += snprintf(szValues + iUpdateLen, sizeof(szValues) - iUpdateLen, ",%d", pkItem->GetEvolution());

		char szInsertQuery[QUERY_MAX_LEN];
		snprintf(szInsertQuery, sizeof(szInsertQuery), "INSERT INTO %soffline_shop_item (%s) VALUES (%s)", get_table_postfix(), szColumns, szValues);
		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery(szInsertQuery));
		ITEM_MANAGER::instance().RemoveItem(pkItem);

		char name[36];
		snprintf(name, sizeof(name), "%-20s(#%-5d) (x %d)", item_table->szName, (int) item.vnum, item.count);

		sys_log(0, "OFFLINE_SHOP_ITEM: %-36s PRICE %-5d", name, item.price);
		++pTable;		
	}
}

void COfflineShop::AddItem(LPCHARACTER ch, LPITEM pkItem, BYTE bPos, int iPrice, int iPrice2, short sPrice_Type)
{
	OFFLINE_SHOP_ITEM & item = m_itemVector[bPos];
		
	item.id = pkItem->GetID();
	item.owner_id = ch->GetPlayerID();
	item.pos = bPos;
	item.count = pkItem->GetCount();
	item.price = iPrice;
	item.price2 = iPrice2;
	item.price_type = sPrice_Type;
	item.vnum = pkItem->GetVnum();
	item.status = 0;
	thecore_memcpy(item.alSockets, pkItem->GetSockets(), sizeof(item.alSockets));
	thecore_memcpy(item.aAttr, pkItem->GetAttributes(), sizeof(item.aAttr));
#ifdef __TRANSMUTATION_SYSTEM__
	item.transmutation = pkItem->GetTransmutation();
#endif
#ifdef __PRIVATESHOP_SEARCH_SYSTEM__
	strlcpy(item.szName, pkItem->GetName(), sizeof(item.szName));
	item.refine_level = pkItem->GetRefineLevel();
	item.shop_id = GetOfflineShopNPC()->GetVID();
#endif
	item.evolution = pkItem->GetEvolution();
	
	char szColumns[QUERY_MAX_LEN], szValues[QUERY_MAX_LEN];

	int iLen = snprintf(szColumns, sizeof(szColumns), "id,owner_id,pos,count,price,price2,price_type,vnum");
	int iUpdateLen = snprintf(szValues, sizeof(szValues), "%u,%u,%d,%u,%d,%d,%d,%u", pkItem->GetID(), ch->GetPlayerID(), bPos, pkItem->GetCount(), iPrice, iPrice2, sPrice_Type, pkItem->GetVnum());

	iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ",socket0,socket1,socket2,socket3");
	iUpdateLen += snprintf(szValues + iUpdateLen, sizeof(szValues) - iUpdateLen, ",%ld,%ld,%ld,%ld", pkItem->GetSocket(0), pkItem->GetSocket(1), pkItem->GetSocket(2), pkItem->GetSocket(3));

	iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ", attrtype0, attrvalue0, attrtype1, attrvalue1, attrtype2, attrvalue2, attrtype3, attrvalue3, attrtype4, attrvalue4, attrtype5, attrvalue5, attrtype6, attrvalue6, applytype0, applyvalue0, applytype1, applyvalue1, applytype2, applyvalue2, applytype3, applyvalue3, applytype4, applyvalue4, applytype5, applyvalue5, applytype6, applyvalue6, applytype7, applyvalue7");
	iUpdateLen += snprintf(szValues + iUpdateLen, sizeof(szValues) - iUpdateLen, ",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
		pkItem->GetAttributeType(0), pkItem->GetAttributeValue(0),
		pkItem->GetAttributeType(1), pkItem->GetAttributeValue(1),
		pkItem->GetAttributeType(2), pkItem->GetAttributeValue(2),
		pkItem->GetAttributeType(3), pkItem->GetAttributeValue(3),
		pkItem->GetAttributeType(4), pkItem->GetAttributeValue(4),
		pkItem->GetAttributeType(5), pkItem->GetAttributeValue(5),
		pkItem->GetAttributeType(6), pkItem->GetAttributeValue(6),
		pkItem->GetAttributeType(7), pkItem->GetAttributeValue(7),
		pkItem->GetAttributeType(8), pkItem->GetAttributeValue(8),
		pkItem->GetAttributeType(9), pkItem->GetAttributeValue(9),
		pkItem->GetAttributeType(10), pkItem->GetAttributeValue(10),
		pkItem->GetAttributeType(11), pkItem->GetAttributeValue(11),
		pkItem->GetAttributeType(12), pkItem->GetAttributeValue(12),
		pkItem->GetAttributeType(13), pkItem->GetAttributeValue(13),
		pkItem->GetAttributeType(14), pkItem->GetAttributeValue(14));
		
#ifdef __TRANSMUTATION_SYSTEM__
		iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ",transmutation");
		iUpdateLen += snprintf(szValues + iUpdateLen, sizeof(szValues) - iUpdateLen, ",%d", pkItem->GetTransmutation());
#endif

#ifdef __PRIVATESHOP_SEARCH_SYSTEM__
		iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ",item_name,refine_level,shop_id");
		iUpdateLen += snprintf(szValues + iUpdateLen, sizeof(szValues) - iUpdateLen, ",'%s',%d,%d", ch->GetName(), pkItem->GetRefineLevel(), (DWORD) GetOfflineShopNPC()->GetVID());
#endif

		iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ",evolution");
		iUpdateLen += snprintf(szValues + iUpdateLen, sizeof(szValues) - iUpdateLen, ",%d", pkItem->GetEvolution());

	char szInsertQuery[QUERY_MAX_LEN];
	snprintf(szInsertQuery, sizeof(szInsertQuery), "INSERT INTO %soffline_shop_item (%s) VALUES (%s)", get_table_postfix(), szColumns, szValues);
	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery(szInsertQuery));
	// ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("OFFLINE_SHOP_ITEM_ADDED"), pkItem->GetName());
	pkItem->RemoveFromCharacter();
	ITEM_MANAGER::instance().FlushDelayedSave(pkItem);
	Refresh(ch);
}

void COfflineShop::RemoveItem(LPCHARACTER ch, BYTE bPos)
{
	OFFLINE_SHOP_ITEM & item = m_itemVector[bPos];
	TPlayerItem item2;
	
	if (item.status == 2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("OFFLINE_SHOP_ITEM_SATILDI"));
		return;
	}
	
	item2.id = item.id;
	item2.vnum = item.vnum;
	item2.count = item.count;
	item2.alSockets[0] = item.alSockets[0];
	item2.alSockets[1] = item.alSockets[1];
	item2.alSockets[2] = item.alSockets[2];
	item2.alSockets[3] = item.alSockets[3];
	item2.alSockets[4] = item.alSockets[4];
	item2.alSockets[5] = item.alSockets[5];
	for (int j = 0; j < ITEM_ATTRIBUTE_MAX_NUM; j++)
	{
		item2.aAttr[j].bType = item.aAttr[j].bType;
		item2.aAttr[j].sValue = item.aAttr[j].sValue;
	}
#ifdef __TRANSMUTATION_SYSTEM__
	item2.transmutation = item.transmutation;
#endif
	item2.evolution = item.evolution;
	
	LPITEM pItem = ITEM_MANAGER::instance().CreateItem(item2.vnum, item2.count);
	if (!pItem)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("OFFLINE_SHOP_ERROR"));
		return;
	}
	
	//pItem->SetID(item2.id);
	
	pItem->SetAttributes(item2.aAttr);
	pItem->SetSockets(item2.alSockets);
#ifdef __TRANSMUTATION_SYSTEM__
	pItem->SetTransmutation(item2.transmutation);
#endif
	pItem->SetEvolution(item2.evolution);

	int iEmptyPos = 0;
	if (pItem->IsDragonSoul())
		iEmptyPos = ch->GetEmptyDragonSoulInventory(pItem);
#ifdef __SPECIAL_INVENTORY__
	else if (pItem->IsSkillBook())
		iEmptyPos = ch->GetEmptySkillBookInventory(pItem->GetSize());
	else if (pItem->IsUpgradeItem())
		iEmptyPos = ch->GetEmptyUpgradeItemsInventory(pItem->GetSize());
#endif
	else
		iEmptyPos = ch->GetEmptyInventory(pItem->GetSize());

	if (iEmptyPos < 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("OFFLINE_SHOP_NOUT_ENOUGH_SPACE"));
		return;
	}
	
	if (pItem->IsDragonSoul())
		pItem->AddToCharacter(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
#ifdef __SPECIAL_INVENTORY__
	else if (pItem->IsSkillBook())
		pItem->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
	else if (pItem->IsUpgradeItem())
		pItem->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
#endif
	else
		pItem->AddToCharacter(ch, TItemPos(INVENTORY,iEmptyPos));
	
	ITEM_MANAGER::instance().FlushDelayedSave(pItem);

	DBManager::instance().DirectQuery("DELETE FROM %soffline_shop_item WHERE owner_id = %u and pos = %d", get_table_postfix(), ch->GetPlayerID(), bPos);
	memset(&item, 0, sizeof(item));

	LogManager::instance().ItemLog(ch, pItem, "DELETE OFFLINE SHOP ITEM", "");
	// ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("OFFLINE_SHOP_ITEM_SOLD"), pItem->GetName());
	Refresh(ch);
}

bool COfflineShop::AddGuest(LPCHARACTER ch, LPCHARACTER npc)
{
	if (!ch || ch->GetExchange() || ch->GetShop() || ch->GetMyShop() || ch->GetOfflineShop())
		return false;

	ch->SetOfflineShop(this);
	m_map_guest.insert(GuestMapType::value_type(ch, false));

	TPacketGCShop pack;
	pack.header = HEADER_GC_OFFLINE_SHOP;
	pack.subheader = SHOP_SUBHEADER_GC_START;

	TPacketGCOfflineShopStart pack2;
	memset(&pack2, 0, sizeof(pack2));
	pack2.owner_vid = npc->GetVID();
	
	++m_dwDisplayedCount;
	
	pack2.m_dwDisplayedCount = m_dwDisplayedCount;
	
	if (GetLeftItemCount() == 0)
	{
		DBManager::instance().DirectQuery("DELETE FROM player.offline_shop_npc WHERE owner_id = %u", npc->GetOfflineShopRealOwner());
		DBManager::instance().DirectQuery("UPDATE %soffline_shop_item SET status = 1 WHERE owner_id = %u and status = 0", get_table_postfix(), npc->GetOfflineShopRealOwner());
		DBManager::instance().DirectQuery("DELETE FROM %soffline_shop_item WHERE owner_id = %u and status = 2", get_table_postfix(), npc->GetOfflineShopRealOwner());
		ch->SetOfflineShop(NULL);
		ch->SetOfflineShopOwner(NULL);
		M2_DESTROY_CHARACTER(npc);
		return false;
	}
	
	for (DWORD i = 0; i < m_itemVector.size() && i < OFFLINE_SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		const OFFLINE_SHOP_ITEM & item = m_itemVector[i];
		if (item.vnum == 0)
			continue;
		
		pack2.items[item.pos].count = item.count;
		pack2.items[item.pos].vnum = item.vnum;
		pack2.items[item.pos].price = item.price;
		pack2.items[item.pos].price2 = item.price2;
		pack2.items[item.pos].price_type = item.price_type;
		pack2.items[item.pos].status = item.status;
		strlcpy(pack2.items[item.pos].szBuyerName, item.szBuyerName, sizeof(pack2.items[item.pos].szBuyerName));
		pack2.items[item.pos].evolution = item.evolution;
		
		DWORD alSockets[ITEM_SOCKET_MAX_NUM];
		for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
			alSockets[i] = item.alSockets[i];
		TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
		
		for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
		{
			aAttr[i].bType = item.aAttr[i].bType;
			aAttr[i].sValue = item.aAttr[i].sValue;
		}
		
#ifdef __TRANSMUTATION_SYSTEM__
		pack2.items[item.pos].transmutation = item.transmutation;
#endif
		
		thecore_memcpy(pack2.items[item.pos].alSockets, alSockets, sizeof(pack2.items[item.pos].alSockets));
		thecore_memcpy(pack2.items[item.pos].aAttr, aAttr, sizeof(pack2.items[item.pos].aAttr));
	}

	pack.size = sizeof(pack) + sizeof(pack2);
	ch->GetDesc()->BufferedPacket(&pack, sizeof(TPacketGCShop));
	ch->GetDesc()->Packet(&pack2, sizeof(TPacketGCOfflineShopStart));
	
	return true;
}

void COfflineShop::SetGuestMap(LPCHARACTER ch)
{
	GuestMapType::iterator it = m_map_guest.find(ch);
	if (it != m_map_guest.end())
		return;
	m_map_guest.insert(GuestMapType::value_type(ch, false));
}

void COfflineShop::RemoveGuestMap(LPCHARACTER ch)
{
	if (ch->GetOfflineShop() != this)
		return;
	
	m_map_guest.erase(ch);
}

void COfflineShop::RemoveGuest(LPCHARACTER ch)
{
	if (ch->GetOfflineShop() != this)
		return;

	m_map_guest.erase(ch);
	ch->SetOfflineShop(NULL);

	TPacketGCShop pack;
	pack.header = HEADER_GC_OFFLINE_SHOP;
	pack.subheader = SHOP_SUBHEADER_GC_END;
	pack.size = sizeof(TPacketGCShop);

	ch->GetDesc()->Packet(&pack, sizeof(pack));
}

void COfflineShop::RemoveAllGuest()
{
	TPacketGCShop pack;
	pack.header = HEADER_GC_OFFLINE_SHOP;
	pack.subheader = SHOP_SUBHEADER_GC_END;
	pack.size = sizeof(TPacketGCShop);

	Broadcast(&pack, sizeof(pack));

	for (GuestMapType::iterator it = m_map_guest.begin(); it != m_map_guest.end(); ++it)
	{
		LPCHARACTER ch = it->first;
		ch->SetOfflineShop(NULL);
	}
}

void COfflineShop::Destroy(LPCHARACTER npc)
{
	DBManager::instance().DirectQuery("DELETE FROM %soffline_shop_item WHERE owner_id = %u and status = 2", get_table_postfix(), npc->GetOfflineShopRealOwner());
	DBManager::instance().DirectQuery("DELETE FROM %soffline_shop_npc WHERE owner_id = %u", get_table_postfix(), npc->GetOfflineShopRealOwner());
	DBManager::instance().DirectQuery("UPDATE %soffline_shop_item SET status = 1 WHERE owner_id = %u and status = 0", get_table_postfix(), npc->GetOfflineShopRealOwner());
	RemoveAllGuest();
	M2_DESTROY_CHARACTER(npc);
}

int COfflineShop::Buy(LPCHARACTER ch, BYTE bPos)
{
	if (ch->GetOfflineShopOwner()->GetOfflineShopRealOwner() == ch->GetPlayerID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("OFFLINE_SHOP_NOT_BUY_YOUR_SHOP"));
		return SHOP_SUBHEADER_GC_OK;
	}

	if (bPos >= OFFLINE_SHOP_HOST_ITEM_MAX_NUM)
	{
		sys_log(0, "OfflineShop::Buy : invalid position %d : %s", bPos, ch->GetName());
		return SHOP_SUBHEADER_GC_INVALID_POS;
	}

	sys_log(0, "OfflineShop::Buy : name %s pos %d", ch->GetName(), bPos);
	
	GuestMapType::iterator it = m_map_guest.find(ch);
	if (it == m_map_guest.end())
		return SHOP_SUBHEADER_GC_END;
	
	OFFLINE_SHOP_ITEM& r_item = m_itemVector[bPos];
	DWORD dwID = r_item.id;
	DWORD dwPrice = r_item.price;
	DWORD dwPrice2 = r_item.price2;
	short sPriceType = r_item.price_type;
	DWORD dwItemVnum = r_item.vnum;
	BYTE bCount = r_item.count;
	DWORD alSockets[ITEM_SOCKET_MAX_NUM];	
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
#ifdef __TRANSMUTATION_SYSTEM__
	DWORD dwTransmutation = r_item.transmutation;
#endif
	DWORD dwEvolution = r_item.evolution;
	
	for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		alSockets[i] = r_item.alSockets[i];
	
	for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
	{
		aAttr[i].bType = r_item.aAttr[i].bType;
		aAttr[i].sValue = r_item.aAttr[i].sValue;
	}

	if (r_item.status != 0)
		return SHOP_SUBHEADER_GC_SOLD_OUT;

	if (dwPrice > 0)
	{
		if (ch->GetGold() < static_cast<DWORD>(dwPrice))
		{
			sys_log(1, "Shop::Buy : Not enough money : %s has %d, price %d", ch->GetName(), ch->GetGold(), dwPrice);
			return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;
		}
	}
	
	if (sPriceType == 1 && dwPrice2 > 0 && dwPrice2 > ch->CountSpecifyItem(OFFLINE_SHOP_PRICE_GOLD_BAR_1))
	{
		sys_log(1, "Shop::Buy : Not enough bar : %s has %d, price %d", ch->GetName(), ch->CountSpecifyItem(OFFLINE_SHOP_PRICE_GOLD_BAR_1), dwPrice2);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_BAR_1;
	}
	
	if (sPriceType == 2 && dwPrice2 > 0 && dwPrice2 > ch->CountSpecifyItem(OFFLINE_SHOP_PRICE_GOLD_BAR_2))
	{
		sys_log(1, "Shop::Buy : Not enough bar : %s has %d, price %d", ch->GetName(), ch->CountSpecifyItem(OFFLINE_SHOP_PRICE_GOLD_BAR_2), dwPrice2);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_BAR_2;
	}

	if (sPriceType == 3 && dwPrice2 > 0 && dwPrice2 > ch->CountSpecifyItem(OFFLINE_SHOP_PRICE_GOLD_BAR_3))
	{
		sys_log(1, "Shop::Buy : Not enough bar : %s has %d, price %d", ch->GetName(), ch->CountSpecifyItem(OFFLINE_SHOP_PRICE_GOLD_BAR_3), dwPrice2);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_BAR_3;
	}	
	
	if (sPriceType == 4 && dwPrice2 > 0 && dwPrice2 > ch->CountSpecifyItem(OFFLINE_SHOP_PRICE_SOUL_STONE))
	{
		sys_log(1, "Shop::Buy : Not enough soul stone : %s has %d, price %d", ch->GetName(), ch->CountSpecifyItem(OFFLINE_SHOP_PRICE_SOUL_STONE), dwPrice2);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_SOUL_STONE;
	}	
	
	if (sPriceType == 5 && dwPrice2 > 0 && dwPrice2 > ch->CountSpecifyItem(OFFLINE_SHOP_PRICE_DRAGON_SCALE))
	{
		sys_log(1, "Shop::Buy : Not enough dragon scale : %s has %d, price %d", ch->GetName(), ch->CountSpecifyItem(OFFLINE_SHOP_PRICE_DRAGON_SCALE), dwPrice2);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_DRAGON_SCALE;
	}	
	
	if (sPriceType == 6 && dwPrice2 > 0 && dwPrice2 > ch->CountSpecifyItem(OFFLINE_SHOP_PRICE_DRAGON_CLAW))
	{
		sys_log(1, "Shop::Buy : Not enough dragon claw : %s has %d, price %d", ch->GetName(), ch->CountSpecifyItem(OFFLINE_SHOP_PRICE_DRAGON_CLAW), dwPrice2);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_DRAGON_CLAW;
	}
	
#ifdef __CHEQUE_SYSTEM__
	if (sPriceType == 7 && dwPrice2 > 0 && dwPrice2 > ch->GetCheque())
	{
		sys_log(1, "Shop::Buy : Not enough dragon CHEQUE : %s has %d, price %d", ch->GetName(), ch->GetCheque(), dwPrice2);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_CHEQUE;
	}
#endif
	
	LPITEM pItem = ITEM_MANAGER::Instance().CreateItem(dwItemVnum, bCount);
	if (!pItem)
		return SHOP_SUBHEADER_GC_SOLD_OUT;

	//pItem->SetID(dwID);
	pItem->SetAttributes(aAttr);
	for (BYTE i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		pItem->SetSocket(i, alSockets[i]);
#ifdef __TRANSMUTATION_SYSTEM__
	pItem->SetTransmutation(dwTransmutation);
#endif
	pItem->SetEvolution(dwEvolution);
	
	int iEmptyPos = 0;
	if (pItem->IsDragonSoul())
		iEmptyPos = ch->GetEmptyDragonSoulInventory(pItem);
#ifdef __SPECIAL_INVENTORY__
	else if (pItem->IsSkillBook())
		iEmptyPos = ch->GetEmptySkillBookInventory(pItem->GetSize());
	else if (pItem->IsUpgradeItem())
		iEmptyPos = ch->GetEmptyUpgradeItemsInventory(pItem->GetSize());
#endif
	else
		iEmptyPos = ch->GetEmptyInventory(pItem->GetSize());

	if (iEmptyPos < 0)
		return SHOP_SUBHEADER_GC_INVENTORY_FULL;

	if (pItem->IsDragonSoul())
		pItem->AddToCharacter(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
#ifdef __SPECIAL_INVENTORY__
	else if (pItem->IsSkillBook())
		pItem->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
	else if (pItem->IsUpgradeItem())
		pItem->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
#endif
	else
		pItem->AddToCharacter(ch, TItemPos(INVENTORY,iEmptyPos));

	if (pItem)
		sys_log(0, "OFFLINE_SHOP: BUY: name %s %s(x %u):%u price %d", ch->GetName(), pItem->GetName(), pItem->GetCount(), pItem->GetID(), dwPrice);
	
	if (dwPrice > 0)
	{
		DBManager::instance().DirectQuery("UPDATE player.player SET shop_money = shop_money + %d WHERE id = %u", dwPrice, ch->GetOfflineShopOwner()->GetOfflineShopRealOwner());
		SetOfflineShopGold(GetOfflineShopGold()+dwPrice);
	}
	
	if (dwPrice2 > 0)
	{
		if (sPriceType == 1)
		{
			DBManager::instance().DirectQuery("UPDATE player.player SET shop_bar1 = shop_bar1 + %d WHERE id = %u", dwPrice2, ch->GetOfflineShopOwner()->GetOfflineShopRealOwner());
			SetOfflineShopBar1(GetOfflineShopBar1()+dwPrice2);
		}
		else if (sPriceType == 2)
		{
			DBManager::instance().DirectQuery("UPDATE player.player SET shop_bar2 = shop_bar2 + %d WHERE id = %u", dwPrice2, ch->GetOfflineShopOwner()->GetOfflineShopRealOwner());
			SetOfflineShopBar2(GetOfflineShopBar2()+dwPrice2);
		}
		else if (sPriceType == 3)
		{
			DBManager::instance().DirectQuery("UPDATE player.player SET shop_bar3 = shop_bar3 + %d WHERE id = %u", dwPrice2, ch->GetOfflineShopOwner()->GetOfflineShopRealOwner());
			SetOfflineShopBar3(GetOfflineShopBar3()+dwPrice2);
		}
		else if (sPriceType == 4)
		{
			DBManager::instance().DirectQuery("UPDATE player.player SET shop_soul_stone = shop_soul_stone + %d WHERE id = %u", dwPrice2, ch->GetOfflineShopOwner()->GetOfflineShopRealOwner());
			SetOfflineShopSoulStone(GetOfflineShopSoulStone()+dwPrice2);
		}
		else if (sPriceType == 5)
		{
			DBManager::instance().DirectQuery("UPDATE player.player SET shop_dragon_scale = shop_dragon_scale + %d WHERE id = %u", dwPrice2, ch->GetOfflineShopOwner()->GetOfflineShopRealOwner());
			SetOfflineShopDragonScale(GetOfflineShopDragonScale()+dwPrice2);
		}
		else if (sPriceType == 6)
		{
			DBManager::instance().DirectQuery("UPDATE player.player SET shop_dragon_claw = shop_dragon_claw + %d WHERE id = %u", dwPrice2, ch->GetOfflineShopOwner()->GetOfflineShopRealOwner());
			SetOfflineShopDragonClaw(GetOfflineShopDragonClaw()+dwPrice2);
		}
#ifdef __CHEQUE_SYSTEM__
		else if (sPriceType == 7)
		{
			DBManager::instance().DirectQuery("UPDATE player.player SET shop_cheque = shop_cheque + %d WHERE id = %u", dwPrice2, ch->GetOfflineShopOwner()->GetOfflineShopRealOwner());
			SetOfflineShopCheque(GetOfflineShopCheque()+dwPrice2);
		}
#endif
	}
	
	DBManager::instance().DirectQuery("UPDATE %soffline_shop_item SET status = 2 WHERE pos = %d and owner_id = %u", get_table_postfix(), bPos, ch->GetOfflineShopOwner()->GetOfflineShopRealOwner());
	DBManager::instance().DirectQuery("UPDATE %soffline_shop_item SET buyer_name='%s' WHERE pos = %d and owner_id = %u", get_table_postfix(), ch->GetName(), bPos, ch->GetOfflineShopOwner()->GetOfflineShopRealOwner());
	r_item.status = 2;
	strlcpy(r_item.szBuyerName, ch->GetName(), sizeof(r_item.szBuyerName));
	ITEM_MANAGER::instance().FlushDelayedSave(pItem);
	BroadcastUpdateItem(bPos, ch->GetOfflineShopOwner()->GetOfflineShopRealOwner(), false);
	
	if (dwPrice > 0)
		ch->PointChange(POINT_GOLD, -dwPrice, false);
	
	if (dwPrice2 > 0)
	{
		if (sPriceType == 1)
			ch->RemoveSpecifyItem(OFFLINE_SHOP_PRICE_GOLD_BAR_1, dwPrice2);
		else if (sPriceType == 2)
			ch->RemoveSpecifyItem(OFFLINE_SHOP_PRICE_GOLD_BAR_2, dwPrice2);
		else if (sPriceType == 3)
			ch->RemoveSpecifyItem(OFFLINE_SHOP_PRICE_GOLD_BAR_3, dwPrice2);
		else if (sPriceType == 4)
			ch->RemoveSpecifyItem(OFFLINE_SHOP_PRICE_SOUL_STONE, dwPrice2);
		else if (sPriceType == 5)
			ch->RemoveSpecifyItem(OFFLINE_SHOP_PRICE_DRAGON_SCALE, dwPrice2);
		else if (sPriceType == 6)
			ch->RemoveSpecifyItem(OFFLINE_SHOP_PRICE_DRAGON_CLAW, dwPrice2);
#ifdef __CHEQUE_SYSTEM__
		else if (sPriceType == 7)
			ch->PointChange(POINT_CHEQUE, -dwPrice2, false);
#endif
	}

	if (pItem->GetVnum() >= 55701 && pItem->GetVnum() <= 55706) {
		DBManager::instance().DirectQuery("UPDATE new_petsystem SET id =%d WHERE id = %d", pItem->GetID(), pItem->GetAttributeValue(14));
	}
	
	ch->Save();
	LogManager::instance().ItemLog(ch, pItem, "BUY ITEM FROM OFFLINE SHOP", "");
	
	BYTE bLeftItemCount = GetLeftItemCount();
	return (SHOP_SUBHEADER_GC_OK);
}

void COfflineShop::BroadcastUpdateItem(BYTE bPos, DWORD dwPID, bool bDestroy)
{
	TPacketGCShop pack;
	TPacketGCOfflineShopUpdateItem pack2;
	TEMP_BUFFER buf;
	pack.header = HEADER_GC_OFFLINE_SHOP;
	pack.subheader = SHOP_SUBHEADER_GC_UPDATE_ITEM;
	pack.size = sizeof(pack) + sizeof(pack2);
	pack2.pos = bPos;

	if (bDestroy)
	{
		pack2.item.vnum = 0;
		pack2.item.count = 0;
		pack2.item.price = 0;
		pack2.item.price2 = 0;
		pack2.item.price_type = 0;
		pack2.item.status = 0;
		memset(pack2.item.alSockets, 0, sizeof(pack2.item.alSockets));
		memset(pack2.item.aAttr, 0, sizeof(pack2.item.aAttr));
#ifdef __TRANSMUTATION_SYSTEM__
		pack2.item.transmutation = 0;
#endif
		pack2.item.evolution = 0;
	}
	else
	{
		pack2.item.count = m_itemVector[bPos].count;
		pack2.item.vnum = m_itemVector[bPos].vnum;
		pack2.item.price = m_itemVector[bPos].price;
		pack2.item.price2 = m_itemVector[bPos].price2;
		pack2.item.price_type = m_itemVector[bPos].price_type;
		pack2.item.status = m_itemVector[bPos].status;
		strlcpy(pack2.item.szBuyerName, m_itemVector[bPos].szBuyerName, sizeof(pack2.item.szBuyerName));
		thecore_memcpy(pack2.item.alSockets, m_itemVector[bPos].alSockets, sizeof(pack2.item.alSockets));
		thecore_memcpy(pack2.item.aAttr, m_itemVector[bPos].aAttr, sizeof(pack2.item.aAttr));
#ifdef __TRANSMUTATION_SYSTEM__
		pack2.item.transmutation = m_itemVector[bPos].transmutation;
#endif
		pack2.item.evolution = m_itemVector[bPos].evolution;
	}

	buf.write(&pack, sizeof(pack));
	buf.write(&pack2, sizeof(pack2));
	Broadcast(buf.read_peek(), buf.size());
}

void COfflineShop::BroadcastUpdatePrice(BYTE bPos, DWORD dwPrice)
{
	TPacketGCShop pack;
	TPacketGCShopUpdatePrice pack2;

	TEMP_BUFFER buf;
	
	pack.header = HEADER_GC_OFFLINE_SHOP;
	pack.subheader = SHOP_SUBHEADER_GC_UPDATE_PRICE;
	pack.size = sizeof(pack) + sizeof(pack2);

	pack2.bPos = bPos;
	pack2.iPrice = dwPrice;

	buf.write(&pack, sizeof(pack));
	buf.write(&pack2, sizeof(pack2));

	Broadcast(buf.read_peek(), buf.size());
}

void COfflineShop::Refresh(LPCHARACTER ch)
{
	TPacketGCShop pack;
	pack.header = HEADER_GC_OFFLINE_SHOP;
	pack.subheader = SHOP_SUBHEADER_GC_UPDATE_ITEM2;

	TPacketGCOfflineShopStart pack2;
	memset(&pack2, 0, sizeof(pack2));
	pack2.owner_vid = 0;
	pack2.m_dwDisplayedCount = m_dwDisplayedCount;

	for (DWORD i = 0; i < m_itemVector.size() && i < OFFLINE_SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		const OFFLINE_SHOP_ITEM & item = m_itemVector[i];
		if (item.vnum == 0)
			continue;
		
		pack2.items[item.pos].count = item.count;
		pack2.items[item.pos].vnum = item.vnum;
		pack2.items[item.pos].price = item.price;
		pack2.items[item.pos].price2 = item.price2;
		pack2.items[item.pos].price_type = item.price_type;
		pack2.items[item.pos].status = item.status;
		strlcpy(pack2.items[item.pos].szBuyerName, item.szBuyerName, sizeof(pack2.items[item.pos].szBuyerName));
		thecore_memcpy(pack2.items[item.pos].alSockets, item.alSockets, sizeof(pack2.items[item.pos].alSockets));
		thecore_memcpy(pack2.items[item.pos].aAttr, item.aAttr, sizeof(pack2.items[item.pos].aAttr));
#ifdef __TRANSMUTATION_SYSTEM__
		pack2.items[item.pos].transmutation = item.transmutation;
#endif
		pack2.items[item.pos].evolution = item.evolution;
	}

	pack.size = sizeof(pack) + sizeof(pack2);
	ch->GetDesc()->BufferedPacket(&pack, sizeof(TPacketGCShop));
	ch->GetDesc()->Packet(&pack2, sizeof(TPacketGCOfflineShopStart));
}

bool COfflineShop::RemoveItem(DWORD dwVID, BYTE bPos)
{
	DBManager::instance().Query("DELETE FROM %soffline_shop_item WHERE owner_id = %u and pos = %d", get_table_postfix(), dwVID, bPos);
	return true;
}

BYTE COfflineShop::GetLeftItemCount()
{
	BYTE bCount = 0;
	for (int i = 0; i < OFFLINE_SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		const OFFLINE_SHOP_ITEM & item = m_itemVector[i];
		if (item.vnum == 0)
			continue;
		if (item.status == 0)
			bCount++;
	}

	return bCount;
}

void COfflineShop::Broadcast(const void * data, int bytes)
{
	sys_log(1, "OfflineShop::Broadcast %p %d", data, bytes);

	for (GuestMapType::iterator it = m_map_guest.begin(); it != m_map_guest.end(); ++it)
	{
		LPCHARACTER ch = it->first;
		if (ch->GetDesc())
			ch->GetDesc()->Packet(data, bytes);
	}
}
