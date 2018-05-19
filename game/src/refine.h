/*********************************************************************
* date        : 2018.02.25
* file        : refine.h
* author      : RAMSAY BOLTON
* version	  : 0.0.1
*/
#ifndef __INC_REFINE_H
#define __INC_REFINE_H

#include "constants.h"

#define PADDER_INCREASE_VALUE "REFINE.INCREASE_PERCENTAGE_"
#define PADDER_MODE "REFINE.MODE_%d"
#define PADDER_MIN 0
#define PADDER_SAVE 1

enum
{
	BLACKSMITH_MOB = 20016,
	ALCHEMIST_MOB = 20001,

	BLACKSMITH_WEAPON_MOB = 20044,
	BLACKSMITH_ARMOR_MOB = 20045,
	BLACKSMITH_ACCESSORY_MOB = 20046,

	DEVILTOWER_BLACKSMITH_WEAPON_MOB = 20074,
	DEVILTOWER_BLACKSMITH_ARMOR_MOB = 20075,
	DEVILTOWER_BLACKSMITH_ACCESSORY_MOB = 20076,

	BLACKSMITH2_MOB	= 20091,
};

#ifdef __EXTENDED_REFINE_WINDOW_SYSTEM__
	enum
	{
		REFINE_PERCENTAGE_LOW = 5,
		REFINE_PERCENTAGE_MEDIUM = 10,
		REFINE_PERCENTAGE_EXTRA = 15,

		REFINE_VNUM_POTION_LOW = 56001,
		REFINE_VNUM_POTION_MEDIUM = 56002,
		REFINE_VNUM_POTION_EXTRA = 56003,
		
	};
#endif

class CRefineManager : public singleton<CRefineManager>
{
	typedef std::map<DWORD, TRefineTable> TRefineRecipeMap;
	public:
	CRefineManager();
	virtual ~CRefineManager();

	bool	Initialize(TRefineTable * table, int size);
	const TRefineTable* GetRefineRecipe(DWORD id);

#ifdef __EXTENDED_REFINE_WINDOW_SYSTEM__
	bool	GetPercentage(LPCHARACTER ch, BYTE lLow, BYTE lMedium, BYTE lExtra, BYTE total, LPITEM item);
	void	Increase(LPCHARACTER ch, BYTE lLow, BYTE lMedium, BYTE lExtra);
	void	Reset(LPCHARACTER ch);
	int		Result(LPCHARACTER ch);
#endif
	
	private:
		TRefineRecipeMap    m_map_RefineRecipe;

};
#endif