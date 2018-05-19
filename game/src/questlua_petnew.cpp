#include "stdafx.h"

#include "questlua.h"
#include "questmanager.h"
#include "horsename_manager.h"
#include "char.h"
#include "affect.h"
#include "config.h"
#include "utils.h"
#include "db.h"

#include "New_PetSystem.h"

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

extern int(*check_name) (const char * str);

namespace quest
{

#ifdef __GROWTH_PET_SYSTEM__
	// syntax in LUA: pet.summon(mob_vnum, pet's name, (bool)run to me from far away)
	int newpet_summon(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CNewPetSystem* petSystem = ch->GetNewPetSystem();
		LPITEM pItem = CQuestManager::instance().GetCurrentItem();
		if (!ch || !petSystem || !pItem)
		{
			lua_pushnumber(L, 0);
			return 1;
		}
#ifdef __ENABLE_PVP_ADVANCED__
		if ((ch->GetDuel("BlockPet")))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("duel_block_function"));
			lua_pushnumber (L, 0);
			return 1;
		}
#endif
		if (0 == petSystem)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		// 소환수의 vnum
		DWORD mobVnum = lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0;

		// 소환수의 이름		

		const char* petName = lua_isstring(L, 2) ? lua_tostring(L, 2) : 0;

		// 소환하면 멀리서부터 달려오는지 여부
		bool bFromFar = lua_isboolean(L, 3) ? lua_toboolean(L, 3) : false;

		CNewPetActor* pet = petSystem->Summon(mobVnum, pItem, petName, bFromFar);

		//ch->ChatPacket(CHAT_TYPE_INFO, "Pet %s Level %d Currentexp: %d NextExp: %d", pet->GetCharacter()->GetName(), pet->GetCharacter()->GetLevel(),pet->GetCharacter()->GetExp(), pet->GetCharacter()->PetGetNextExp());


		if (pet != NULL)
			lua_pushnumber(L, pet->GetVID());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	// syntax: pet.unsummon(mob_vnum)
	int newpet_unsummon(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CNewPetSystem* petSystem = ch->GetNewPetSystem();

		if (0 == petSystem)
			return 0;

		// 소환수의 vnum
		DWORD mobVnum = lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0;

		petSystem->Unsummon(mobVnum);
		return 1;
	}

	// syntax: pet.unsummon(mob_vnum)
	int newpet_count_summoned(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CNewPetSystem* petSystem = ch->GetNewPetSystem();

		lua_Number count = 0;

		if (0 != petSystem)
			count = (lua_Number)petSystem->CountSummoned();

		lua_pushnumber(L, count);

		return 1;
	}

	// syntax: pet.is_summon(mob_vnum)
	int newpet_is_summon(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CNewPetSystem* petSystem = ch->GetNewPetSystem();

		if (0 == petSystem)
			return 0;

		// 소환수의 vnum
		DWORD mobVnum = lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0;

		CNewPetActor* petActor = petSystem->GetByVnum(mobVnum);

		if (NULL == petActor)
			lua_pushboolean(L, false);
		else
			lua_pushboolean(L, petActor->IsSummoned());

		return 1;
	}

	int newpet_increaseskill(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CNewPetSystem* petSystem = ch->GetNewPetSystem();

		if (0 == petSystem)
			return 0;

		// 소환수의 vnum
		DWORD skill = lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0;

		bool petActor = petSystem->IncreasePetSkill(skill);

		if (NULL == petActor)
			lua_pushboolean(L, false);
		else
			lua_pushboolean(L, petActor);
		return 1;

	}

	int newpet_resetskill(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CNewPetSystem* petSystem = ch->GetNewPetSystem();

		if (0 == petSystem)
			return 0;

		// 소환수의 vnum
		DWORD skill = lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0;

		bool petActor = petSystem->ResetSkill(skill);

		if (NULL == petActor)
			lua_pushboolean(L, false);
		else
			lua_pushboolean(L, petActor);
		return 1;

	}

	int newpet_increaseevolution(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CNewPetSystem* petSystem = ch->GetNewPetSystem();

		if (0 == petSystem)
			return 0;

		// 소환수의 vnum		

		bool petActor = petSystem->IncreasePetEvolution();

		if (NULL == petActor)
			lua_pushboolean(L, false);
		else
			lua_pushboolean(L, petActor);
		return 1;

	}

	int newpet_get_level(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CNewPetSystem* petSystem = ch->GetNewPetSystem();

		if (0 == petSystem) {
			lua_pushnumber(L, -1);
			return 0;
		}
		int pet_level = petSystem->GetLevel();

		if (NULL == pet_level)
			lua_pushnumber(L, -1);
		else
			lua_pushnumber(L, pet_level);

		return 1;

	}

	int newpet_get_evo(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CNewPetSystem* petSystem = ch->GetNewPetSystem();

		if (0 == petSystem) {
			lua_pushnumber(L, -1);
			return 0;
		}
		int pet_evo = petSystem->GetEvolution();

		if (NULL == pet_evo)
			lua_pushnumber(L, -1);
		else
			lua_pushnumber(L, pet_evo);

		return 1;

	}

	int newpet_restore_pet(lua_State* L)
	{

		DWORD id = lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0;
		if (id == 0){
			lua_pushboolean(L, false);
			return 0;
		}

		char szQuery1[1024];
		snprintf(szQuery1, sizeof(szQuery1), "SELECT duration,tduration FROM new_petsystem WHERE id = %lu ", id);
		std::unique_ptr<SQLMsg> pmsg2(DBManager::instance().DirectQuery(szQuery1));
		if (pmsg2->Get()->uiNumRows > 0) {
			MYSQL_ROW row = mysql_fetch_row(pmsg2->Get()->pSQLResult);
			if (atoi(row[0]) <= 0){
				DBManager::instance().DirectQuery("UPDATE new_petsystem SET duration=%d WHERE id = %lu ", atoi(row[1]), id);
				lua_pushboolean(L, true);
			}
			else{
				lua_pushboolean(L, false);
			}
		}
		else{
			lua_pushboolean(L, false);
		}

		return 1;
	}

	int newpet_spawn_effect(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CNewPetSystem* petSystem = ch->GetNewPetSystem();

		if (0 == petSystem)
			return 0;

		DWORD mobVnum = lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0;

		CNewPetActor* petActor = petSystem->GetByVnum(mobVnum);
		if (NULL == petActor)
			return 0;
		LPCHARACTER pet_ch = petActor->GetCharacter();
		if (NULL == pet_ch)
			return 0;

		if (lua_isstring(L, 2))
		{
			pet_ch->SpecificEffectPacket(lua_tostring(L, 2));
		}
		return 0;
	}

	int newpet_eggrequest(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();		
		int evid = lua_isnumber(L, 0) ? lua_tonumber(L, 0) : 0;
		ch->SetEggVid(evid);
		return 1;
	}

	void RegisterNewPetFunctionTable()
	{
		luaL_reg pet_functions[] =
		{
			{ "EggRequest",		newpet_eggrequest},
			{ "summon",			newpet_summon },
			{ "unsummon",		newpet_unsummon },
			{ "is_summon",		newpet_is_summon },
			{ "count_summoned",	newpet_count_summoned },
			{ "spawn_effect",	newpet_spawn_effect },
			{ "increaseskill",	newpet_increaseskill},
			{ "increaseevo",	newpet_increaseevolution},
			{ "getlevel",		newpet_get_level },
			{ "getevo",			newpet_get_evo },
			{ "restorepet",		newpet_restore_pet},	
			{ "resetskill",		newpet_resetskill},
			{ NULL,				NULL }
		};

		CQuestManager::instance().AddLuaFunctionTable("newpet", pet_functions);
	}
#endif

}