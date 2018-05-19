#ifndef __INC_METIN_II_GAME_SHOP_MANAGER_H__
#define __INC_METIN_II_GAME_SHOP_MANAGER_H__

class CShop;
typedef class CShop * LPSHOP;

class CShopManager : public singleton<CShopManager>
{
public:
	typedef std::map<DWORD, CShop *> TShopMap;

public:
	CShopManager();
	virtual ~CShopManager();

	bool	Initialize(TShopTable * table, int size);
	void	Destroy();

	LPSHOP	Get(DWORD dwVnum);
	LPSHOP	GetByNPCVnum(DWORD dwVnum);

	bool	StartShopping(LPCHARACTER pkChr, LPCHARACTER pkShopKeeper, int iShopVnum = 0);
	bool	NPCAC(LPCHARACTER pkChr, LPCHARACTER pkShopKeeper, int iShopVnum = 0);
	void	StopShopping(LPCHARACTER ch);

	void	Buy(LPCHARACTER ch, BYTE pos);
	void	Sell(LPCHARACTER ch, UINT bCell, BYTE bCount=0);

	LPSHOP	CreatePCShop(LPCHARACTER ch, TShopItemTable2 * pTable, BYTE bItemCount);
	LPSHOP	FindPCShop(DWORD dwVID);
	void	DestroyPCShop(LPCHARACTER ch);
	void	BuyFromGlass(DWORD dwVID, LPCHARACTER ch_buyer, BYTE pos, int iBuyType);
	void	FindFromGlass(LPCHARACTER ch_seller, LPCHARACTER ch_buyer);
	void	RemoveAllItemsFromMysql(DWORD dwSellerPID);
	bool	AddGuest(LPCHARACTER ch, DWORD dwVID);
#ifdef OFFLINE_SHOP
public:
	bool	StartOfflineShop(DWORD id, bool onboot = false);
	bool	CreateOfflineShop(LPCHARACTER owner, const char *szSign,const std::vector<TShopItemTable2*> pTable, DWORD days);
	LPSHOP	CreateNPCShop(LPCHARACTER ch, std::vector<TShopItemTable2 *> map_shop);
	DWORD	GetShopCount() { return m_map_pkShop.size(); }
#endif

private:
	TShopMap	m_map_pkShop;
	TShopMap	m_map_pkShopByNPCVnum;
	TShopMap	m_map_pkShopByPC;

	bool	ReadShopTableEx(const char* stFileName);
};

#endif
