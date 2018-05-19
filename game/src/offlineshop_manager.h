#pragma once
class COfflineShopManager : public singleton<COfflineShopManager>
{
	public:
		typedef std::map<DWORD, COfflineShop *> TShopMap;
		typedef std::map<DWORD, DWORD> TOfflineShopMap;
	public:
		COfflineShopManager();
		~COfflineShopManager();

		bool StartShopping(LPCHARACTER pkChr, LPCHARACTER pkChrShopKeeper);
		void StopShopping(LPCHARACTER ch);

		void Buy(LPCHARACTER ch, BYTE bPos);

		LPOFFLINESHOP	CreateOfflineShop(LPCHARACTER npc, DWORD dwOwnerPID, TOfflineShopItemTable * pTable = NULL, short bItemCount = 0, long lMapIndex = 0, int iTime = 0, const char * szSign = "\0");
		LPOFFLINESHOP	FindOfflineShop(DWORD dwVID);
		void			ChangeOfflineShopTime(LPCHARACTER ch, int bTime);
		void			DestroyOfflineShop(LPCHARACTER ch, DWORD dwVID, bool bDestroyAll = false);
		
		void			AddItem(LPCHARACTER ch, BYTE bDisplayPos, TItemPos bPos, int iPrice, int iPrice2, short sPriceType);
		void			RemoveItem(LPCHARACTER ch, BYTE bPos);
		void			Refresh(LPCHARACTER ch);
        void            RefreshMoney(LPCHARACTER ch);

		DWORD			FindMyOfflineShop(DWORD dwPID);
		void			WithdrawMoney(LPCHARACTER ch, DWORD llRequiredMoney);
		void			WithdrawBar1(LPCHARACTER ch, DWORD dwRequiredBar);
		void			WithdrawBar2(LPCHARACTER ch, DWORD dwRequiredBar);
		void			WithdrawBar3(LPCHARACTER ch, DWORD dwRequiredBar);
		void			WithdrawSoulStone(LPCHARACTER ch, DWORD dwRequiredSoulStone);
		void			WithdrawDragonScale(LPCHARACTER ch, DWORD dwRequiredDragonScale);
		void			WithdrawDragonClaw(LPCHARACTER ch, DWORD dwRequiredDragonClaw);
#ifdef __CHEQUE_SYSTEM__
		void			WithdrawCheque(LPCHARACTER ch, DWORD dwRequiredCheque);
#endif
		BYTE			LeftItemCount(LPCHARACTER ch);
		int				GetShopCountChannel() const { return m_Map_pkOfflineShopByNPC2.size(); }

		void			RefreshUnsoldItems(LPCHARACTER ch);
		void			TakeItem(LPCHARACTER ch, BYTE bPos);
		bool			HasOfflineShop(LPCHARACTER ch);
		long			GetMapIndex(LPCHARACTER ch);
		int				GetLeftTime(LPCHARACTER ch);
		LPOFFLINESHOP	FindShop(DWORD dwVID);
		bool			AddGuest(LPCHARACTER ch, DWORD dwVID);
		LPCHARACTER		GetOfflineShopNPC(DWORD dwVID);
		
	private:
		TOfflineShopMap	m_Map_pkOfflineShopByNPC2;
		TShopMap		m_map_pkOfflineShopByNPC;
};

