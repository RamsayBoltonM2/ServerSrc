#include "stdafx.h"
#include "../../libgame/include/grid.h"
#include "constants.h"
#include "utils.h"
#include "config.h"
#include "shop.h"
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
#include "db.h"
#include "desc_client.h"
#include "char.h"

/* ------------------------------------------------------------------------------------ */
CShop::CShop()
	: m_dwVnum(0), m_dwNPCVnum(0), m_pkPC(NULL)
#ifdef __SHOPEX_SYSTEM__
	,m_sPriceType(0), m_szShopName("")
#endif
{
	m_pGrid = M2_NEW CGrid(10, 10);
}

CShop::~CShop()
{
	TPacketGCShop pack;

	pack.header		= HEADER_GC_SHOP;
	pack.subheader	= SHOP_SUBHEADER_GC_END;
	pack.size		= sizeof(TPacketGCShop);

	Broadcast(&pack, sizeof(pack));

	GuestMapType::iterator it;

	it = m_map_guest.begin();

	while (it != m_map_guest.end())
	{
		LPCHARACTER ch = it->first;
		ch->SetShop(NULL);
		++it;
	}

	M2_DELETE(m_pGrid);
}
inline bool SHOP_SEARCH_MIN_PRICE_VAILD(long long item_price,long long price) {return (item_price <= price || price == 0);}
inline bool SHOP_SEARCH_MAX_PRICE_VAILD(long long item_price,long long price) {return (item_price >= price || price == 0);}
inline bool SHOP_SEARCH_MIN_LEVEL_VAILD(int min_level, int mlevel) {return (min_level <= mlevel || mlevel == 0);}
inline bool SHOP_SEARCH_MAX_LEVEL_VAILD(int max_level, int mlevel) {return (max_level >= mlevel || mlevel == 0);}
inline bool SHOP_SEARCH_MIN_REFINE_LEVEL(int min_refine, int mrefine) {return (min_refine <= mrefine || mrefine == 0);}
inline bool SHOP_SEARCH_MAX_REFINE_LEVEL(int max_refine, int mrefine) {return (max_refine >= mrefine || mrefine == 0);}
bool CShop::SearchItem(LPCHARACTER ch, const char* item_name, int iJob, int iType, int iSubType, int iMinLevel, int iMaxLevel, int iMinRefine, int iMaxRefine, int iMinPrice, int iMaxPrice, int iMinCheque, int iMaxCheque)
{
	bool ret = false;
	for (DWORD i = 0; i < m_itemVector.size() && i < SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		const SHOP_ITEM & item = m_itemVector[i];
		if (!item.pkItem)
			continue;
		
		if (item_name != " ")
		{
			if (!strstr(item.pkItem->GetName(), item_name))
				continue;
		}

		bool bufix = true;
		if (iType == 2 || iType == 3)
		{
			if (iJob == 0 && (IS_SET(item.pkItem->GetAntiFlag(), ITEM_ANTIFLAG_WARRIOR)))
				bufix = false;
			if (iJob == 1 && (IS_SET(item.pkItem->GetAntiFlag(), ITEM_ANTIFLAG_ASSASSIN)))
				bufix = false;		
			if (iJob == 2 && (IS_SET(item.pkItem->GetAntiFlag(), ITEM_ANTIFLAG_SURA)))
				bufix = false;	
			if (iJob == 3 && (IS_SET(item.pkItem->GetAntiFlag(), ITEM_ANTIFLAG_SHAMAN)))
				bufix = false;	
			if (iJob == 4 && (IS_SET(item.pkItem->GetAntiFlag(), ITEM_ANTIFLAG_WOLFMAN)))
				bufix = false;					
		}
		if (iType > 0)
		{
			if (item.pkItem->GetMaskType() != iType)
				continue;
		}

		if (item.pkItem->GetMaskSubType() != iSubType)
			continue;
		
		if (iMinLevel > 0)
		{
			if (!SHOP_SEARCH_MIN_LEVEL_VAILD(item.pkItem->GetLevelLimit(),iMinLevel))
				continue;
		}
		if (iMaxLevel != 120)
		{
			if (iMaxLevel < item.pkItem->GetLevelLimit())
				continue;
		}
		if (iMinRefine > 0)
		{
			if (!SHOP_SEARCH_MIN_REFINE_LEVEL(item.pkItem->GetRefineLevel(),iMinRefine))
				continue;
		}
		if (iMaxRefine != 9)
		{
			if (!SHOP_SEARCH_MAX_REFINE_LEVEL(item.pkItem->GetRefineLevel(),iMaxRefine))
				continue;
		}
		if (iMinPrice > 0)
		{
			if (!SHOP_SEARCH_MIN_PRICE_VAILD(item.price,iMinPrice))
				continue;
		}
		if (iMaxPrice > 0)
		{
			if (iMaxPrice < item.price)
				continue;
		}
		if (iMinCheque > 0)
		{
			if (iMinCheque > item.price_cheque)
				continue;
		}
		if (iMaxCheque > 0)
		{
			if (iMaxCheque < item.price_cheque)
				continue;
		}
		
		if (bufix == true)
		{
		
		ret = true;
		
		ch->ChatPacket(CHAT_TYPE_COMMAND,
			"GetSearchedItemData 1 %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",

		m_pkPC->GetName(), item.pkItem->GetVnum(), item.pkItem->GetCount(), item.pkItem->GetRefineLevel(), item.price, item.pkItem->GetLevelLimit(), item.pkItem->GetAttributeValue(6),

		item.pkItem->GetSocket(0), item.pkItem->GetSocket(1), item.pkItem->GetSocket(2), item.pkItem->GetSocket(3), item.pkItem->GetSocket(4), item.pkItem->GetSocket(5),

		item.pkItem->GetAttributeType(0), item.pkItem->GetAttributeValue(0), item.pkItem->GetAttributeType(1), item.pkItem->GetAttributeValue(1), item.pkItem->GetAttributeType(2), item.pkItem->GetAttributeValue(2),
		item.pkItem->GetAttributeType(3), item.pkItem->GetAttributeValue(3), item.pkItem->GetAttributeType(4), item.pkItem->GetAttributeValue(4), item.pkItem->GetAttributeType(5), item.pkItem->GetAttributeValue(5),
		item.pkItem->GetAttributeType(6), item.pkItem->GetAttributeValue(6),

		item.pkItem->GetAttributeType(7), item.pkItem->GetAttributeValue(7), item.pkItem->GetAttributeType(8), item.pkItem->GetAttributeValue(8), item.pkItem->GetAttributeType(9), item.pkItem->GetAttributeValue(9),
		item.pkItem->GetAttributeType(10), item.pkItem->GetAttributeValue(10), item.pkItem->GetAttributeType(11), item.pkItem->GetAttributeValue(11), item.pkItem->GetAttributeType(12), item.pkItem->GetAttributeValue(12),
		item.pkItem->GetAttributeType(13), item.pkItem->GetAttributeValue(13), item.pkItem->GetAttributeType(14), item.pkItem->GetAttributeValue(14),

		i, m_pkPC ? m_pkPC->GetVID() : 0, item.price_cheque, item.pkItem->GetTransmutation());
		}
	}
	
	if (ret == true)
		return true;
	else
		return false;
}
	

#ifdef __SHOPEX_SYSTEM__
bool CShop::Create(DWORD dwVnum, DWORD dwNPCVnum, TShopItemTable * pTable, short price_type, std::string shopname)
#else
bool CShop::Create(DWORD dwVnum, DWORD dwNPCVnum, TShopItemTable * pTable)
#endif
{
	/*
	   if (NULL == CMobManager::instance().Get(dwNPCVnum))
	   {
	   sys_err("No such a npc by vnum %d", dwNPCVnum);
	   return false;
	   }
	 */
	sys_log(0, "SHOP #%d (Shopkeeper %d)", dwVnum, dwNPCVnum);

	m_dwVnum = dwVnum;
	m_dwNPCVnum = dwNPCVnum;
#ifdef __SHOPEX_SYSTEM__
	m_sPriceType = price_type;
	m_szShopName = shopname;
#endif

	BYTE bItemCount;

	for (bItemCount = 0; bItemCount < SHOP_HOST_ITEM_MAX_NUM; ++bItemCount)
		if (0 == (pTable + bItemCount)->vnum)
			break;

	SetShopItems(pTable, bItemCount);
	return true;
}

void CShop::SetShopItemsPrivateShop(TShopItemTable2 * pTable, BYTE bItemCount)
{
	if (bItemCount > SHOP_HOST_ITEM_MAX_NUM)
		return;

	m_pGrid->Clear();

	m_itemVector.resize(SHOP_HOST_ITEM_MAX_NUM);
	memset(&m_itemVector[0], 0, sizeof(SHOP_ITEM) * m_itemVector.size());

	for (int i = 0; i < bItemCount; ++i)
	{
		LPITEM pkItem = NULL;
		const TItemTable * item_table;

		if (m_pkPC)
		{
			pkItem = m_pkPC->GetItem(pTable->pos);

			if (!pkItem)
			{
				sys_err("cannot find item on pos (%d, %d) (name: %s)", pTable->pos.window_type, pTable->pos.cell, m_pkPC->GetName());
				continue;
			}

			item_table = pkItem->GetProto();
		}
		else
		{
			if (!pTable->vnum)
				continue;

			item_table = ITEM_MANAGER::instance().GetTable(pTable->vnum);
		}

		if (!item_table)
		{
			sys_err("Shop: no item table by item vnum #%d", pTable->vnum);
			continue;
		}

		int iPos;

		if (IsPCShop())
		{
			sys_log(0, "MyShop: use position %d", pTable->display_pos);
			iPos = pTable->display_pos;
		}
		else
			iPos = m_pGrid->FindBlank(1, item_table->bSize);

		if (iPos < 0)
		{
			sys_err("not enough shop window");
			continue;
		}

		if (!m_pGrid->IsEmpty(iPos, 1, item_table->bSize))
		{
			if (IsPCShop())
			{
				sys_err("not empty position for pc shop %s[%d]", m_pkPC->GetName(), m_pkPC->GetPlayerID());
			}
			else
			{
				sys_err("not empty position for npc shop");
			}
			continue;
		}

		m_pGrid->Put(iPos, 1, item_table->bSize);

		SHOP_ITEM & item = m_itemVector[iPos];

		item.pkItem = pkItem;
		item.itemid = 0;

		if (item.pkItem)
		{
			item.vnum = pkItem->GetVnum();
			item.count = pkItem->GetCount(); // PC 샵의 경우 아이템 개수는 진짜 아이템의 개수여야 한다.
			item.price = pTable->price; // 가격도 사용자가 정한대로..
			item.price_cheque = pTable->price_cheque;
			item.itemid	= pkItem->GetID();
		}
		else
		{
			item.vnum = pTable->vnum;
			item.count = pTable->count;
			item.price = pTable->price;
		}

		char name[36];
		snprintf(name, sizeof(name), "%-20s(#%-5d) (x %d)", item_table->szName, (int) item.vnum, item.count);

		sys_log(0, "SHOP_ITEM: %-36s PRICE %-5d", name, item.price);
		/*if (IsPCShop())
			InsertInMysql(m_pkPC, pkItem, iPos, item.price);*/
		++pTable;
	}
}

void CShop::SetShopItems(TShopItemTable * pTable, BYTE bItemCount)
{
	if (bItemCount > SHOP_HOST_ITEM_MAX_NUM)
		return;

	m_pGrid->Clear();

	m_itemVector.resize(SHOP_HOST_ITEM_MAX_NUM);
	memset(&m_itemVector[0], 0, sizeof(SHOP_ITEM) * m_itemVector.size());

	for (int i = 0; i < bItemCount; ++i)
	{
		LPITEM pkItem = NULL;
		const TItemTable * item_table;

		if (m_pkPC)
		{
			pkItem = m_pkPC->GetItem(pTable->pos);

			if (!pkItem)
			{
				sys_err("cannot find item on pos (%d, %d) (name: %s)", pTable->pos.window_type, pTable->pos.cell, m_pkPC->GetName());
				continue;
			}

			item_table = pkItem->GetProto();
		}
		else
		{
			if (!pTable->vnum)
				continue;

			item_table = ITEM_MANAGER::instance().GetTable(pTable->vnum);
		}

		if (!item_table)
		{
			sys_err("Shop: no item table by item vnum #%d", pTable->vnum);
			continue;
		}

		int iPos;

		if (IsPCShop())
		{
			sys_log(0, "MyShop: use position %d", pTable->display_pos);
			iPos = pTable->display_pos;
		}
		else
			iPos = m_pGrid->FindBlank(1, item_table->bSize);

		if (iPos < 0)
		{
			sys_err("not enough shop window");
			continue;
		}

		if (!m_pGrid->IsEmpty(iPos, 1, item_table->bSize))
		{
			if (IsPCShop())
			{
				sys_err("not empty position for pc shop %s[%d]", m_pkPC->GetName(), m_pkPC->GetPlayerID());
			}
			else
			{
				sys_err("not empty position for npc shop");
			}
			continue;
		}

		m_pGrid->Put(iPos, 1, item_table->bSize);

		SHOP_ITEM & item = m_itemVector[iPos];

		item.pkItem = pkItem;
		item.itemid = 0;

		if (item.pkItem)
		{
			item.vnum = pkItem->GetVnum();
			item.count = pkItem->GetCount(); // PC 샵의 경우 아이템 개수는 진짜 아이템의 개수여야 한다.
			item.price = pTable->price; // 가격도 사용자가 정한대로..
			item.price_cheque = pTable->price_cheque;
			item.itemid	= pkItem->GetID();
		}
		else
		{
			item.vnum = pTable->vnum;
			item.count = pTable->count;
			item.price = pTable->price;
			item.alSockets[0] = pTable->alSockets[0];
			item.alSockets[1] = pTable->alSockets[1];
			item.alSockets[2] = pTable->alSockets[2];
			item.alSockets[3] = pTable->alSockets[3];
			item.aAttr[0].bType = pTable->aAttr[0].bType;
			item.aAttr[0].sValue = pTable->aAttr[0].sValue;
			item.aAttr[1].bType = pTable->aAttr[1].bType;
			item.aAttr[1].sValue = pTable->aAttr[1].sValue;
			item.aAttr[2].bType = pTable->aAttr[2].bType;
			item.aAttr[2].sValue = pTable->aAttr[2].sValue;
			item.aAttr[3].bType = pTable->aAttr[3].bType;
			item.aAttr[3].sValue = pTable->aAttr[3].sValue;
			item.aAttr[4].bType = pTable->aAttr[4].bType;
			item.aAttr[4].sValue = pTable->aAttr[4].sValue;
			item.aAttr[5].bType = pTable->aAttr[5].bType;
			item.aAttr[5].sValue = pTable->aAttr[5].sValue;
			item.aAttr[6].bType = pTable->aAttr[6].bType;
			item.aAttr[6].sValue = pTable->aAttr[6].sValue;
		}

		char name[36];
		snprintf(name, sizeof(name), "%-20s(#%-5d) (x %d)", item_table->szName, (int) item.vnum, item.count);

		sys_log(0, "SHOP_ITEM: %-36s PRICE %-5d", name, item.price);
		/*if (IsPCShop())
			InsertInMysql(m_pkPC, pkItem, iPos, item.price);*/
		++pTable;
	}
}

int CShop::Buy(LPCHARACTER ch, BYTE pos)
{
	if (pos >= m_itemVector.size())
	{
		sys_log(0, "Shop::Buy : invalid position %d : %s", pos, ch->GetName());
		return SHOP_SUBHEADER_GC_INVALID_POS;
	}

	sys_log(0, "Shop::Buy : name %s pos %d", ch->GetName(), pos);

	GuestMapType::iterator it = m_map_guest.find(ch);

	if (it == m_map_guest.end())
		return SHOP_SUBHEADER_GC_END;

	SHOP_ITEM& r_item = m_itemVector[pos];

	if (r_item.price < 0)
	{
		LogManager::instance().HackLog("SHOP_BUY_GOLD_OVERFLOW", ch);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;
	}

	LPITEM pkSelectedItem = ITEM_MANAGER::instance().Find(r_item.itemid);

#ifdef FULL_YANG
	long long dwPrice = r_item.price;
#else
	DWORD dwPrice = r_item.price;
#endif
	long long dwPriceCheque = r_item.price_cheque;
	DWORD dwTax = 0;


	int iVal = quest::CQuestManager::instance().GetEventFlag("personal_shop");

	if (0 < iVal)
	{
		if (iVal > 100)
		{
			iVal = 100;
		}

		dwTax = dwPrice * iVal / 100;
		dwPrice = dwPrice - dwTax;
	}
	else
	{
		iVal = 0;
		dwTax = 0;
	}

#ifdef __SHOPEX_SYSTEM__
	if (m_pkPC)
	{
		if (ch->GetGold() < (int) dwPrice)
		{
			sys_log(1, "Shop::Buy : Not enough money : %s has %d, price %d", ch->GetName(), ch->GetGold(), dwPrice);
			return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;
		}
		
		if (ch->GetCheque() < (int) dwPriceCheque)
		{
			sys_log(1, "Shop::Buy : Not enough money : %s has %d, price %d", ch->GetName(), ch->GetGold(), dwPrice);
			return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;
		}
	}
	else
	{
		if (m_sPriceType == 0)
		{
			if (ch->GetGold() < (int) dwPrice)
			{
				sys_log(1, "Shop::Buy : Not enough money : %s has %lld, price %lld", ch->GetName(), ch->GetGold(), dwPrice);
				return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;
			}			
		}
		else if (m_sPriceType == 1)
		{
			if (ch->GetDragonCoin() < (int) dwPrice)
			{
				sys_log(1, "Shop::Buy : Not enough dc : %s has %d, price %d", ch->GetName(), ch->GetDragonCoin(), dwPrice);
				return SHOP_SUBHEADER_GC_NOT_ENOUGH_DRAGON_COIN;
			}
		}
		else if (m_sPriceType == 2)
		{
			if (ch->GetDragonMark() < (int) dwPrice)
			{
				sys_log(1, "Shop::Buy : Not enough dm : %s has %d, price %d", ch->GetName(), ch->GetDragonMark(), dwPrice);
				return SHOP_SUBHEADER_GC_NOT_ENOUGH_DRAGON_MARK;
			}
		}
		else if (m_sPriceType == 3)
		{
			if (ch->GetAlignment() < (int) dwPrice*10)
			{
				sys_log(1, "Shop::Buy : Not enough alignment : %s has %d, price %d", ch->GetName(), ch->GetAlignment(), dwPrice*10);
				return SHOP_SUBHEADER_GC_NOT_ENOUGH_ALIGNMENT;
			}
		}
		else if (m_sPriceType == 4)
		{
			if (ch->GetWarPoint() < (int) dwPrice)
			{
				sys_log(1, "Shop::Buy : Not enough warpoint : %s has %d, price %d", ch->GetName(), ch->GetWarPoint(), dwPrice);
				return SHOP_SUBHEADER_GC_NOT_ENOUGH_WARPOINT;
			}
		}
		else if (m_sPriceType == 5)
		{
			if (ch->CountSpecifyItem(25105) < (int) dwPrice)
			{
				sys_log(1, "Shop::Buy : Not enough 10thgold : %s has %d, price %d", ch->GetName(), ch->CountSpecifyItem(25105), dwPrice);
				return SHOP_SUBHEADER_GC_NOT_ENOUGH_10TH_GOLD;
			}
		}
	}
#else
	if (ch->GetGold() < (int) dwPrice)
	{
		sys_log(1, "Shop::Buy : Not enough money : %s has %d, price %d", ch->GetName(), ch->GetGold(), dwPrice);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;
	}
#endif
	
	LPITEM item;

	if (m_pkPC)
		item = r_item.pkItem;
	else {
		item = ITEM_MANAGER::instance().CreateItem(r_item.vnum, r_item.count);
	}
	if (!item)
		return SHOP_SUBHEADER_GC_SOLD_OUT;
	
	int iEmptyPos;
	if (item->IsDragonSoul())
	{
		iEmptyPos = ch->GetEmptyDragonSoulInventory(item);
	}
#ifdef __SPECIAL_INVENTORY__
	else if (item->IsSkillBook())
	{
		iEmptyPos = ch->GetEmptySkillBookInventory(item->GetSize());
	}
	else if (item->IsUpgradeItem())
	{
		iEmptyPos = ch->GetEmptyUpgradeItemsInventory(item->GetSize());
	}
	else if (item->IsStone())
	{
		iEmptyPos = ch->GetEmptyStoneInventory(item->GetSize());
	}
#endif
	else
	{
		iEmptyPos = ch->GetEmptyInventory(item->GetSize());
	}

	if (iEmptyPos < 0)
	{
		if (m_pkPC)
		{
			sys_log(1, "Shop::Buy at PC Shop : Inventory full : %s size %d", ch->GetName(), item->GetSize());
			return SHOP_SUBHEADER_GC_INVENTORY_FULL;
		}
		else
		{
			sys_log(1, "Shop::Buy : Inventory full : %s size %d", ch->GetName(), item->GetSize());
			M2_DESTROY_ITEM(item);
			return SHOP_SUBHEADER_GC_INVENTORY_FULL;
		}
	}

#ifdef __SHOPEX_SYSTEM__
	if (m_pkPC)
	{
		ch->PointChange(POINT_GOLD, -dwPrice, false);
		ch->PointChange(POINT_CHEQUE, -dwPriceCheque, false);
	}
	else
	{
		if (m_sPriceType == 0)
		{
			ch->PointChange(POINT_GOLD, -dwPrice, false);
		}
		else if (m_sPriceType == 1)
		{
			ch->SetDragonCoin(ch->GetDragonCoin()-dwPrice);
			if (item->GetVnum() != 80014 || item->GetVnum() != 80015 || item->GetVnum() != 80016 || item->GetVnum() != 80017)
				ch->SetDragonMark(ch->GetDragonMark()+dwPrice);
		}
		else if (m_sPriceType == 2)
		{
			ch->SetDragonMark(ch->GetDragonMark()-dwPrice);
		}
		else if (m_sPriceType == 3)
		{
			ch->UpdateAlignment(-dwPrice*10);
		}
		else if (m_sPriceType == 4)
		{
			ch->SetWarPoint(ch->GetWarPoint()-dwPrice);
		}
		else if (m_sPriceType == 5)
		{
			ch->RemoveSpecifyItem(25105, dwPrice);
		}
	}
#else
	ch->PointChange(POINT_GOLD, -dwPrice, false);
#endif
	// ±?AO ˝?˝??? : ??±Y Aˇ?o
	if (m_pkPC)
	{
#ifdef __SECONDARY_SLOT_SYSTEM__
		m_pkPC->SyncNewQuickslot(QUICKSLOT_TYPE_ITEM, item->GetCell(), 1000);
#endif
		char buf[512];
		DWORD mpid = m_pkPC->GetPlayerID();
		if (item->GetVnum() >= 80003 && item->GetVnum() <= 80007)
		{
			snprintf(buf, sizeof(buf), "%s FROM: %u TO: %u PRICE: %u", item->GetName(), ch->GetPlayerID(), mpid, dwPrice);
			LogManager::instance().GoldBarLog(ch->GetPlayerID(), item->GetID(), SHOP_BUY, buf);
			LogManager::instance().GoldBarLog(mpid, item->GetID(), SHOP_SELL, buf);
		}
		LPITEM pkNewItem = ITEM_MANAGER::instance().CreateItem(r_item.vnum, r_item.count);

		if (!item)
			return SHOP_SUBHEADER_GC_SOLD_OUT;

		if (pkNewItem)
		{
			for (int i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
			{
				pkNewItem->SetSocket(i, item->GetSocket(i));
			}
			// 매직 아이템 설정
			item->CopyAttributeTo(pkNewItem);

			if (pkNewItem->IsDragonSoul())
				pkNewItem->AddToCharacter(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
#ifdef __SPECIAL_INVENTORY__
			else if (pkNewItem->IsSkillBook())
				pkNewItem->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
			else if (pkNewItem->IsUpgradeItem())
				pkNewItem->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
			else if (pkNewItem->IsStone())
				pkNewItem->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
#endif
			else
				pkNewItem->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
			pkNewItem->SetEvolution(item->GetEvolution());
			pkNewItem->SetTransmutation(item->GetTransmutation());
			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);
			ITEM_MANAGER::instance().FlushDelayedSave(pkNewItem);
			std::string szBuy("SHOP_BUY");
			std::string szSell("SHOP_SELL");
#ifdef FULL_YANG
			snprintf(buf, sizeof(buf), "%s %u(%s) %lld %u", pkNewItem->GetName(), mpid, m_pkPC->GetName(), dwPrice, pkNewItem->GetCount());
			LogManager::instance().ItemLog(ch, pkNewItem, szBuy.c_str(), buf);
			snprintf(buf, sizeof(buf), "%s %u(%s) %lld %u", pkNewItem->GetName(), ch->GetPlayerID(), ch->GetName(), dwPrice, pkNewItem->GetCount());
			LogManager::instance().ItemLog(m_pkPC, pkNewItem, szSell.c_str(), buf);
#else
			snprintf(buf, sizeof(buf), "%s %u(%s) %u %u", pkNewItem->GetName(), mpid, m_pkPC->GetName(), dwPrice, pkNewItem->GetCount());
			LogManager::instance().ItemLog(ch, pkNewItem, szBuy.c_str(), buf);
			snprintf(buf, sizeof(buf), "%s %u(%s) %u %u", pkNewItem->GetName(), ch->GetPlayerID(), ch->GetName(), dwPrice, pkNewItem->GetCount());
			LogManager::instance().ItemLog(m_pkPC, pkNewItem, szSell.c_str(), buf);
#endif



			r_item.pkItem = NULL;
			BroadcastUpdateItem(pos);
			//RemoveFromMysql(m_pkPC->GetPlayerID(), pos);
			{
#ifdef FULL_YANG_OWN
				m_pkPC->ChangeGold(dwPrice);
#else
				m_pkPC->PointChange(POINT_GOLD, dwPrice, false);
#endif
				m_pkPC->PointChange(POINT_CHEQUE, dwPriceCheque, false);
				if (iVal > 0)
				{
					m_pkPC->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("판매금액의 %d %% 가 세금으로 나가게됩니다"), iVal);
				}

			}
			item = pkNewItem;
		}
		else {
			return SHOP_SUBHEADER_GC_INVALID_POS;
		}
	}
	else
	{
		if (item->IsDragonSoul())
			item->AddToCharacter(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
#ifdef __SPECIAL_INVENTORY__
			else if (item->IsSkillBook())
				item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
			else if (item->IsUpgradeItem())
				item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
			else if (item->IsStone())
				item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
#endif
		else
			item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
		if (r_item.alSockets[0] > 0)
		{
			const TItemTable * table;
			table = ITEM_MANAGER::instance().GetTable(item->GetVnum());
			
			for (int i=0 ; i < ITEM_LIMIT_MAX_NUM ; i++)
			{
				if (LIMIT_REAL_TIME == table->aLimits->bType)
					item->SetSocket(0, time(0) + r_item.alSockets[0]);
				else
					item->SetSocket(0, r_item.alSockets[0]);
			}
		}
		if (r_item.alSockets[1] > 0)
			item->SetSocket(1, r_item.alSockets[1]);
		if (r_item.alSockets[2] > 0)
			item->SetSocket(2, r_item.alSockets[2]);
		if (r_item.alSockets[3] > 0)
			item->SetSocket(3, r_item.alSockets[3]);
		if (r_item.aAttr[0].bType > 0 && r_item.aAttr[0].sValue > 0)
			item->SetForceAttribute(0, r_item.aAttr[0].bType, r_item.aAttr[0].sValue);
		if (r_item.aAttr[1].bType > 0 && r_item.aAttr[1].sValue > 0)
			item->SetForceAttribute(1, r_item.aAttr[1].bType, r_item.aAttr[1].sValue);
		if (r_item.aAttr[2].bType > 0 && r_item.aAttr[2].sValue > 0)
			item->SetForceAttribute(2, r_item.aAttr[2].bType, r_item.aAttr[2].sValue);
		if (r_item.aAttr[3].bType > 0 && r_item.aAttr[3].sValue > 0)
			item->SetForceAttribute(3, r_item.aAttr[3].bType, r_item.aAttr[3].sValue);
		if (r_item.aAttr[4].bType > 0 && r_item.aAttr[4].sValue > 0)
			item->SetForceAttribute(4, r_item.aAttr[4].bType, r_item.aAttr[4].sValue);
		if (r_item.aAttr[5].bType > 0 && r_item.aAttr[5].sValue > 0)
			item->SetForceAttribute(5, r_item.aAttr[5].bType, r_item.aAttr[5].sValue);
		if (r_item.aAttr[6].bType > 0 && r_item.aAttr[6].sValue > 0)
			item->SetForceAttribute(6, r_item.aAttr[6].bType, r_item.aAttr[6].sValue);
		ITEM_MANAGER::instance().FlushDelayedSave(item);
		LogManager::instance().ItemLog(ch, item, "BUY", item->GetName());

		if (item->GetVnum() >= 80003 && item->GetVnum() <= 80007)
		{
			LogManager::instance().GoldBarLog(ch->GetPlayerID(), item->GetID(), PERSONAL_SHOP_BUY, "");
		}

		//DBManager::instance().SendMoneyLog(MONEY_LOG_SHOP, item->GetVnum(), -dwPrice);
	}

	if (item)
		sys_log(0, "SHOP: BUY: name %s %s(x %d):%u price %u", ch->GetName(), item->GetName(), item->GetCount(), item->GetID(), dwPrice);
	ch->Save();
	return (SHOP_SUBHEADER_GC_OK);
}

bool CShop::AddGuest(LPCHARACTER ch, DWORD owner_vid, bool bOtherEmpire)
{
	if (!ch)
		return false;

	if (ch->GetExchange())
		return false;

	if (ch->GetShop())
		return false;

	ch->SetShop(this);

	m_map_guest.insert(GuestMapType::value_type(ch, bOtherEmpire));

	TPacketGCShop pack;

	pack.header		= HEADER_GC_SHOP;
	pack.subheader	= SHOP_SUBHEADER_GC_START;

	TPacketGCShopStart pack2;

	memset(&pack2, 0, sizeof(pack2));
	pack2.owner_vid = owner_vid;
#ifdef __SHOPEX_SYSTEM__
	if (IsPCShop())
	{
		strlcpy(pack2.shop_name, m_pkPC->GetName(), SHOP_TAB_NAME_MAX);
		pack2.price_type = 0;
	}
	else
	{
		pack2.price_type = m_sPriceType;
		strlcpy(pack2.shop_name, m_szShopName.c_str(), SHOP_TAB_NAME_MAX);
	}
#endif

	for (DWORD i = 0; i < m_itemVector.size() && i < SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		const SHOP_ITEM & item = m_itemVector[i];

		//HIVALUE_ITEM_EVENT
		if (quest::CQuestManager::instance().GetEventFlag("hivalue_item_sell") == 0)
		{
			//축복의 구슬 && 만년한철 이벤트 
			if (item.vnum == 70024 || item.vnum == 70035)
			{				
				continue;
			}
		}
		//END_HIVALUE_ITEM_EVENT
		if (m_pkPC && !item.pkItem)
			continue;

		pack2.items[i].vnum = item.vnum;

		if (bOtherEmpire) // no empire price penalty for pc shop
			pack2.items[i].price = item.price * 3;
		else
			pack2.items[i].price = item.price;
		
		pack2.items[i].price_cheque = item.price_cheque;

		pack2.items[i].count = item.count;

		if (item.pkItem)
		{
			thecore_memcpy(pack2.items[i].alSockets, item.pkItem->GetSockets(), sizeof(pack2.items[i].alSockets));
			thecore_memcpy(pack2.items[i].aAttr, item.pkItem->GetAttributes(), sizeof(pack2.items[i].aAttr));
			pack2.items[i].evolution = item.pkItem->GetEvolution();
#ifdef __TRANSMUTATION_SYSTEM__
			pack2.items[i].transmutation = item.pkItem->GetTransmutation();
#endif
		}
#ifdef __SHOPEX_SYSTEM__
		else
		{
			thecore_memcpy(pack2.items[i].alSockets, item.alSockets, sizeof(pack2.items[i].alSockets));
			thecore_memcpy(pack2.items[i].aAttr, item.aAttr, sizeof(pack2.items[i].aAttr));
			pack2.items[i].iLimitValue = item.alSockets[0];
		}
#endif
	}

	pack.size = sizeof(pack) + sizeof(pack2);

	ch->GetDesc()->BufferedPacket(&pack, sizeof(TPacketGCShop));
	ch->GetDesc()->Packet(&pack2, sizeof(TPacketGCShopStart));
	return true;
}

void CShop::RemoveGuest(LPCHARACTER ch)
{
	if (ch->GetShop() != this)
		return;

	m_map_guest.erase(ch);
	ch->SetShop(NULL);

	TPacketGCShop pack;

	pack.header		= HEADER_GC_SHOP;
	pack.subheader	= SHOP_SUBHEADER_GC_END;
	pack.size		= sizeof(TPacketGCShop);

	ch->GetDesc()->Packet(&pack, sizeof(pack));
}

void CShop::Broadcast(const void * data, int bytes)
{
	sys_log(1, "Shop::Broadcast %p %d", data, bytes);

	GuestMapType::iterator it;

	it = m_map_guest.begin();

	while (it != m_map_guest.end())
	{
		LPCHARACTER ch = it->first;

		if (ch->GetDesc())
			ch->GetDesc()->Packet(data, bytes);

		++it;
	}
}

void CShop::BroadcastUpdateItem(BYTE pos)
{
	TPacketGCShop pack;
	TPacketGCShopUpdateItem pack2;

	TEMP_BUFFER	buf;

	pack.header		= HEADER_GC_SHOP;
	pack.subheader	= SHOP_SUBHEADER_GC_UPDATE_ITEM;
	pack.size		= sizeof(pack) + sizeof(pack2);

	pack2.pos		= pos;

	if (m_pkPC && !m_itemVector[pos].pkItem)
		pack2.item.vnum = 0;
	else
	{
		pack2.item.vnum	= m_itemVector[pos].vnum;
		if (m_itemVector[pos].pkItem)
		{
			thecore_memcpy(pack2.item.alSockets, m_itemVector[pos].pkItem->GetSockets(), sizeof(pack2.item.alSockets));
			thecore_memcpy(pack2.item.aAttr, m_itemVector[pos].pkItem->GetAttributes(), sizeof(pack2.item.aAttr));
			pack2.item.evolution = m_itemVector[pos].pkItem->GetEvolution();
		}
		else if (!m_pkPC && !IsPCShop())
#ifdef __SHOPEX_SYSTEM__
		{
			thecore_memcpy(pack2.item.alSockets, m_itemVector[pos].alSockets, sizeof(pack2.item.alSockets));
			thecore_memcpy(pack2.item.aAttr, m_itemVector[pos].aAttr, sizeof(pack2.item.aAttr));
			pack2.item.iLimitValue = m_itemVector[pos].alSockets[0];
		}
#else
		{
			memset(pack2.item.alSockets, 0, sizeof(pack2.item.alSockets));
			memset(pack2.item.aAttr, 0, sizeof(pack2.item.aAttr));
			pack2.item.evolution = 0;
		}
#endif	
	}

	pack2.item.price	= m_itemVector[pos].price;
	pack2.item.price_cheque = m_itemVector[pos].price_cheque;

	pack2.item.count	= m_itemVector[pos].count;

	buf.write(&pack, sizeof(pack));
	buf.write(&pack2, sizeof(pack2));

	Broadcast(buf.read_peek(), buf.size());
}

int CShop::GetNumberByVnum(DWORD dwVnum)
{
	int itemNumber = 0;

	for (DWORD i = 0; i < m_itemVector.size() && i < SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		const SHOP_ITEM & item = m_itemVector[i];

		if (item.vnum == dwVnum)
		{
			itemNumber += item.count;
		}
	}

	return itemNumber;
}

bool CShop::IsSellingItem(DWORD itemID)
{
	bool isSelling = false;

	for (DWORD i = 0; i < m_itemVector.size() && i < SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		if (m_itemVector[i].itemid == itemID)
		{
			isSelling = true;
			break;
		}
	}

	return isSelling;

}

int CShop::BuyFromGlass(LPCHARACTER ch_buyer, BYTE pos)
{
	sys_err("SHOP:0");
	if (pos >= m_itemVector.size())
	{
		sys_log(0, "Shop::BuyFromGlass : Invalid position %d - , Buyer: %s", pos, ch_buyer->GetName());
		return SHOP_SUBHEADER_GC_INVALID_POS;
	}
	
	sys_err("SHOP:1");

	sys_log(0, "Shop::BuyFromGlass : Buy event: Buyer %s pos %d", ch_buyer->GetName(), pos);


	SHOP_ITEM& r_item = m_itemVector[pos];
	if (r_item.price < 0)
	{
		LogManager::instance().HackLog("SHOP_BUY_GOLD_OVERFLOW", ch_buyer);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;
	}

	sys_err("SHOP:2");

	DWORD dwPrice = r_item.price;
	DWORD dwPriceCheque = r_item.price_cheque;
	if (ch_buyer->GetGold() < (int)dwPrice)
	{
		sys_log(1, "Shop::BuyFromGlass : Not enough money : %s has %d, price %d", ch_buyer->GetName(), ch_buyer->GetGold(), dwPrice);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;
	}

	if (ch_buyer->GetCheque() < (int)dwPriceCheque)
	{
		sys_log(1, "Shop::BuyFromGlass : Not enough money : %s has %d, price %d", ch_buyer->GetName(), ch_buyer->GetGold(), dwPrice);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;
	}	
	
	sys_err("SHOP:6");
	
	LPITEM item = r_item.pkItem;
	if (!item)
	{
		sys_log(1, "Shop::BuyFromGlass : Item not found Seller: %s Buyer: %s", m_pkPC->GetName(), ch_buyer->GetName());
		return SHOP_SUBHEADER_GC_SOLD_OUT;
	}

	sys_err("SHOP:7");

	int iEmptyPos;
	iEmptyPos = ch_buyer->GetEmptyInventory(item->GetSize());
	
	if (iEmptyPos < 0)
	{
		sys_log(1, "Shop::BuyFromGlass at PC Shop : Inventory full - Seller: %s Buyer: %s size %d", m_pkPC->GetName(), ch_buyer->GetName(), item->GetSize());
		return SHOP_SUBHEADER_GC_INVENTORY_FULL;
	}
	
	sys_err("SHOP:8");
	
	ch_buyer->PointChange(POINT_GOLD, dwPrice, false);
	ch_buyer->PointChange(POINT_CHEQUE, dwPriceCheque, false);
	

	{
		sys_err("SHOP:10");
#ifdef FULL_YANG_OWN
		m_pkPC->ChangeGold(dwPrice);
#else
		m_pkPC->PointChange(POINT_GOLD, dwPrice, false);
#endif
		m_pkPC->PointChange(POINT_CHEQUE, dwPriceCheque, false);
	}
	
	m_pkPC->SyncQuickslot(QUICKSLOT_TYPE_ITEM, item->GetCell(), 1000);

	char buf[512];

	if (item->GetVnum() >= 80003 && item->GetVnum() <= 80007)
	{
		snprintf(buf, sizeof(buf), "%s FROM: %u TO: %u PRICE: %u", item->GetName(), ch_buyer->GetPlayerID(), m_pkPC->GetPlayerID(), dwPrice);
		LogManager::instance().GoldBarLog(ch_buyer->GetPlayerID(), item->GetID(), SHOP_BUY, buf);
	}
	
	sys_err("SHOP:11");

	item->RemoveFromCharacter();
	item->AddToCharacter(ch_buyer, TItemPos(INVENTORY, iEmptyPos));
	ITEM_MANAGER::instance().FlushDelayedSave(item);

	snprintf(buf, sizeof(buf), "%s %u(%s) %u %u", item->GetName(), m_pkPC->GetPlayerID(), m_pkPC->GetName(), dwPrice, item->GetCount());
	LogManager::instance().ItemLog(ch_buyer, item, "SHOP_BUY_GLASS", buf);

	snprintf(buf, sizeof(buf), "%s %u(%s) %u %u", item->GetName(), ch_buyer->GetPlayerID(), ch_buyer->GetName(), dwPrice, item->GetCount());
	LogManager::instance().ItemLog(m_pkPC, item, "SHOP_SELL_GLASS", buf);

	//RemoveFromMysql(ch_seller->GetPlayerID(), pos);

	r_item.pkItem = NULL;
	BroadcastUpdateItem(pos);

	ch_buyer->ChatPacket(CHAT_TYPE_COMMAND, "RemoveItemFromList %s %d", m_pkPC->GetName(), pos);
	m_pkPC->PointChange(POINT_GOLD, dwPrice, false);

	if (item)
		sys_log(0, "SHOP BuyFromGlass: BUY: name %s %s(x %d):%u price %u", ch_buyer->GetName(), item->GetName(), item->GetCount(), item->GetID(), dwPrice);

	sys_err("SHOP:12");
	
	ch_buyer->Save();
	
	return (SHOP_SUBHEADER_GC_OK);
	sys_err("SHOP:13");
}

void CShop::InsertInMysql(LPCHARACTER lpSeller, LPITEM item, BYTE bPos, DWORD dwItemPrice)
{
	if (!item){ sys_err("Empty ITEM !"); return; }

	long socket1; long socket2; long socket3;
	socket1 = item->GetSocket(0); socket2 = item->GetSocket(1); socket3 = item->GetSocket(2);
	int socket4 = item->GetSocket(3);
	int socket5 = 0;
	int socket6 = 0;
	
	int transmutation = 0;
	int evolution = item->GetEvolution();

	BYTE attr1type; BYTE attr2type; BYTE attr3type; BYTE attr4type; BYTE attr5type; BYTE attr6type; BYTE attr7type;
	attr1type = item->GetAttributeType(0); attr2type = item->GetAttributeType(1); attr3type = item->GetAttributeType(2);
	attr4type = item->GetAttributeType(3); attr5type = item->GetAttributeType(4); attr6type = item->GetAttributeType(5);
	attr7type = item->GetAttributeType(6);

	int attr1value; int attr2value; int attr3value; int attr4value; int attr5value; int attr6value; int attr7value;
	attr1value = item->GetAttributeValue(0); attr2value = item->GetAttributeValue(1); attr3value = item->GetAttributeValue(2);
	attr4value = item->GetAttributeValue(3); attr5value = item->GetAttributeValue(4); attr6value = item->GetAttributeValue(5);
	attr7value = item->GetAttributeValue(6);
	
	int apply0type = item->GetAttributeType(7);
	int apply0value = item->GetAttributeValue(7);
	int apply1type = item->GetAttributeType(8);
	int apply1value = item->GetAttributeValue(8);
	int apply2type = item->GetAttributeType(9);
	int apply2value = item->GetAttributeValue(9);
	int apply3type = item->GetAttributeType(10);
	int apply3value = item->GetAttributeValue(10);
	int apply4type = item->GetAttributeType(11);
	int apply4value = item->GetAttributeValue(11);
	int apply5type = item->GetAttributeType(12);
	int apply5value = item->GetAttributeValue(12);
	int apply6type = item->GetAttributeType(13);
	int apply6value = item->GetAttributeValue(13);
	int apply7type = item->GetAttributeType(14);
	int apply7value = item->GetAttributeValue(14);
	
	const char* c_szSellerName = lpSeller ? lpSeller->GetName() : "Offline Shop"; DWORD dwSellerPID = lpSeller ? lpSeller->GetPlayerID() : m_pkPC->GetPlayerID();

	DWORD dwItemVnum = item->GetVnum(); const char* c_szItemName = item->GetName(); DWORD dwItemCount = item->GetCount();
	int iItemRefine = item->GetRefineLevel(); int iItemLevel; int iItemType = item->GetType();
	int iItemSubtype = item->GetSubType(); DWORD iItemAntiflag = item->GetAntiFlag();

	if (item->GetLimitType(0))
		iItemLevel = item->GetLimitValue(0);
	else
		iItemLevel = 0;

	std::unique_ptr<SQLMsg> pmsg(DBManager::instance().DirectQuery(
		"INSERT INTO shop_onsales_items%s(seller_pid, seller_name, item_name, item_vnum, item_count, item_refine, item_price, item_level, item_pos, item_type, item_subtype, item_antiflag, transmutation, socket0, socket1, socket2, socket3, socket4, socket5, attrtype0, attrvalue0, attrtype1, attrvalue1, attrtype2, attrvalue2, attrtype3, attrvalue3, attrtype4, attrvalue4, attrtype5, attrvalue5, attrtype6, attrvalue6, applytype0, applyvalue0, applytype1, applyvalue1, applytype2, applyvalue2, applytype3, applyvalue3, applytype4, applyvalue4, applytype5, applyvalue5, applytype6, applyvalue6, applytype7, applyvalue7, evolution) "
		"VALUES('%u', '%s', '%s', %u, %u, %d, %u, %d, %d, %d, %d, %u, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)",
		get_table_postfix(), dwSellerPID, c_szSellerName, c_szItemName, dwItemVnum, dwItemCount, iItemRefine, dwItemPrice, iItemLevel, bPos, iItemType, iItemSubtype, iItemAntiflag, transmutation, socket1, socket2, socket3, socket4, socket5, socket6, attr1type, attr1value, attr2type, attr2value, attr3type, attr3value, attr4type, attr4value, attr5type, attr5value, attr6type, attr6value, attr7type, attr7value, apply0type, apply0value, apply1type, apply1value, apply2type, apply2value, apply3type, apply3value, apply4type, apply4value, apply5type, apply5value, apply6type, apply6value, apply7type, apply7value, evolution
	));

	if (pmsg->uiSQLErrno != 0) {
		sys_err("Shop Item Insert Query Failed, Error code: %ld", pmsg->uiSQLErrno);
		return;
	}
}

void CShop::RemoveFromMysql(DWORD dwSellerPID, BYTE bPos){
	std::unique_ptr<SQLMsg> pmsg(DBManager::instance().DirectQuery("DELETE FROM shop_onsales_items WHERE item_pos='%d' AND seller_pid='%u'", bPos, dwSellerPID));

	if (pmsg->uiSQLErrno != 0) {
		sys_err("Shop Item Remove Query Failed, Error code: %ld", pmsg->uiSQLErrno);
		return;
	}
}

