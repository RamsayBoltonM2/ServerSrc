#pragma once

enum
{
	OFFLINE_SHOP_MAX_DISTANCE = 1500,
};

class COfflineShop
{
	public:
		typedef struct offline_shop_item
		{
			DWORD	id;
			DWORD	owner_id;
			BYTE	pos;
			BYTE	count;			
			DWORD		price;
			DWORD		price2;
			short		price_type;
			DWORD		vnum;	
			long	alSockets[ITEM_SOCKET_MAX_NUM];
			TPlayerItemAttribute	aAttr[ITEM_ATTRIBUTE_MAX_NUM];
			BYTE	status;
			char szBuyerName[CHARACTER_NAME_MAX_LEN + 1];
#ifdef __TRANSMUTATION_SYSTEM__
			DWORD	transmutation;
#endif	
			DWORD	evolution;
			char szName[ITEM_NAME_MAX_LEN + 1];
			BYTE	refine_level;
			DWORD	shop_id;
			offline_shop_item()
			{
				id = 0;
				owner_id = 0;
				pos = 0;
				count = 0;
				price = 0;
				price2 = 0;
				price_type = 0;
				vnum = 0;
				alSockets[0] = 0;
				alSockets[1] = 0;
				alSockets[2] = 0;
				alSockets[3] = 0;
				alSockets[4] = 0;
				alSockets[5] = 0;
				aAttr[0].bType = 0;
				aAttr[0].sValue = 0;
				aAttr[1].bType = 0;
				aAttr[1].sValue = 0;
				aAttr[2].bType = 0;
				aAttr[2].sValue = 0;
				aAttr[3].bType = 0;
				aAttr[3].sValue = 0;
				aAttr[4].bType = 0;
				aAttr[4].sValue = 0;
				aAttr[5].bType = 0;
				aAttr[5].sValue = 0;
				aAttr[6].bType = 0;
				aAttr[6].sValue = 0;
				aAttr[7].bType = 0;
				aAttr[7].sValue = 0;
				aAttr[8].bType = 0;
				aAttr[8].sValue = 0;
				aAttr[9].bType = 0;
				aAttr[9].sValue = 0;
				aAttr[10].bType = 0;
				aAttr[10].sValue = 0;
				aAttr[11].bType = 0;
				aAttr[11].sValue = 0;
				aAttr[12].bType = 0;
				aAttr[12].sValue = 0;
				aAttr[13].bType = 0;
				aAttr[13].sValue = 0;	
				aAttr[14].bType = 0;
				aAttr[14].sValue = 0;				
				status = 0;
				szBuyerName[CHARACTER_NAME_MAX_LEN + 1] = 0;
#ifdef __TRANSMUTATION_SYSTEM__
				transmutation = 0;
#endif
				evolution = 0;
				szName[ITEM_NAME_MAX_LEN + 1] = 0;
				refine_level = 0;
				shop_id = 0;
			}
		} OFFLINE_SHOP_ITEM;
	
		COfflineShop();
		~COfflineShop();

		virtual void	SetOfflineShopNPC(LPCHARACTER npc);
		virtual bool	IsOfflineShopNPC(){ return m_pkOfflineShopNPC ? true : false; }
		LPCHARACTER		GetOfflineShopNPC() { return m_pkOfflineShopNPC; }
	
		void			CreateTable(DWORD dwOwnerPID);
		void			SetOfflineShopItems(DWORD dwOwnerPID, TOfflineShopItemTable * pTable, BYTE bItemCount);
		void			AddItem(LPCHARACTER ch, LPITEM pkItem, BYTE bPos, int iPrice, int iPrice2, short sPrice_Type);
		void			RemoveItem(LPCHARACTER ch, BYTE bPos);
		
		virtual bool	AddGuest(LPCHARACTER ch, LPCHARACTER npc);
		
		void			RemoveGuest(LPCHARACTER ch);
		void			RemoveAllGuest();
		void			Destroy(LPCHARACTER npc);

		virtual int		Buy(LPCHARACTER ch, BYTE bPos);
		
		void			BroadcastUpdateItem(BYTE bPos, DWORD dwPID, bool bDestroy = false);
		void			BroadcastUpdatePrice(BYTE bPos, DWORD dwPrice);
		void			Refresh(LPCHARACTER ch);

		bool			RemoveItem(DWORD dwVID, BYTE bPos);
		BYTE			GetLeftItemCount();
		void			SetOfflineShopGold(long long val) { llMoney = val; }
		long long		GetOfflineShopGold() const { return llMoney; }
		void			SetOfflineShopBar1(DWORD val) { dwBar1 = val; }
		DWORD			GetOfflineShopBar1() const { return dwBar1; }
		void			SetOfflineShopBar2(DWORD val) { dwBar2 = val; }
		DWORD			GetOfflineShopBar2() const { return dwBar2; }
		void			SetOfflineShopBar3(DWORD val) { dwBar3 = val; }
		DWORD			GetOfflineShopBar3() const { return dwBar3; }
		void			SetOfflineShopSoulStone(DWORD val) { dwSoulStone = val; }
		DWORD			GetOfflineShopSoulStone() const { return dwSoulStone; }		
		void			SetOfflineShopDragonScale(DWORD val) { dwDragonScale = val; }
		DWORD			GetOfflineShopDragonScale() const { return dwDragonScale; }
		void			SetOfflineShopDragonClaw(DWORD val) { dwDragonClaw = val; }
		DWORD			GetOfflineShopDragonClaw() const { return dwDragonClaw; }
#ifdef __CHEQUE_SYSTEM__
		void			SetOfflineShopCheque(DWORD val) { dwCheque = val; }
		DWORD			GetOfflineShopCheque() const { return dwCheque; }
#endif
		void			SetOfflineShopMapIndex(long idx) { m_llMapIndex = idx; }
		long			GetOfflineShopMapIndex() const { return m_llMapIndex; }
		void			SetOfflineShopTime(int iTime) { m_iTime = iTime; }
		int				GetOfflineShopTime() const { return m_iTime; }
		void			SetOfflineShopBankValues(DWORD dwOwnerPID);
		
		std::string shopSign;
		const char *		GetShopSign() { return shopSign.c_str(); };
		void				SetShopSign(const char * c) { shopSign = c; };
		
		std::vector<OFFLINE_SHOP_ITEM>	GetItemVector() { return m_itemVector; }
		
		void				SetGuestMap(LPCHARACTER ch);
		void				RemoveGuestMap(LPCHARACTER ch);
		
	protected:
		void			Broadcast(const void * data, int bytes);

	private:
		// Grid
		CGrid *				m_pGrid;
		long long			llMoney;
		DWORD				dwBar1;
		DWORD				dwBar2;
		DWORD				dwBar3;
		DWORD				dwSoulStone;
		DWORD				dwDragonScale;
		DWORD				dwDragonClaw;
#ifdef __CHEQUE_SYSTEM__
		DWORD				dwCheque;
#endif
		long				m_llMapIndex;
		int					m_iTime;

		// Guest Map
		typedef TR1_NS::unordered_map<LPCHARACTER, bool> GuestMapType;
		GuestMapType m_map_guest;
		// End Of Guest Map
		
		std::vector<OFFLINE_SHOP_ITEM>		m_itemVector;

		LPCHARACTER m_pkOfflineShopNPC;
		DWORD	m_dwDisplayedCount;
};

