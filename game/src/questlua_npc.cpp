#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "questmanager.h"
#include "char.h"
#include "party.h"
#include "xmas_event.h"
#include "char_manager.h"
#include "shop_manager.h"
#include "guild.h"
#include "desc.h"

namespace quest
{
	//
	// "npc" lua functions
	//
	int npc_open_shop(lua_State * L)
	{
		int iShopVnum = 0;

		if (lua_gettop(L) == 1)
		{
			if (lua_isnumber(L, 1))
				iShopVnum = (int) lua_tonumber(L, 1);
		}

		if (CQuestManager::instance().GetCurrentNPCCharacterPtr())
			CShopManager::instance().StartShopping(CQuestManager::instance().GetCurrentCharacterPtr(), CQuestManager::instance().GetCurrentNPCCharacterPtr(), iShopVnum);
		return 0;
	}

	int npc_get_vid_hp(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();

		DWORD vid = (DWORD) lua_tonumber(L, 1);
		LPCHARACTER targetChar = CHARACTER_MANAGER::instance().Find(vid);

		if (targetChar)
			lua_pushnumber(L, targetChar->GetHP());
		else
			lua_pushnumber(L, 0);


		return 1;
	}
	
	int npc_get_vid_max_hp(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();

		DWORD vid = (DWORD) lua_tonumber(L, 1);
		LPCHARACTER targetChar = CHARACTER_MANAGER::instance().Find(vid);

		if (targetChar)
			lua_pushnumber(L, targetChar->GetMaxHP());
		else
			lua_pushnumber(L, 0);


		return 1;
	}
	
	int npc_get_vid_sp(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();

		DWORD vid = (DWORD) lua_tonumber(L, 1);
		LPCHARACTER targetChar = CHARACTER_MANAGER::instance().Find(vid);

		if (targetChar)
			lua_pushnumber(L, targetChar->GetSP());
		else
			lua_pushnumber(L, 0);


		return 1;
	}
	
	int npc_get_vid_max_sp(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();

		DWORD vid = (DWORD) lua_tonumber(L, 1);
		LPCHARACTER targetChar = CHARACTER_MANAGER::instance().Find(vid);

		if (targetChar)
			lua_pushnumber(L, targetChar->GetMaxSP());
		else
			lua_pushnumber(L, 0);


		return 1;
	}

	int npc_is_pc(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();
		if (npc && npc->IsPC())
			lua_pushboolean(L, 1);
		else
			lua_pushboolean(L, 0);
		return 1;
	}

	int npc_get_empire(lua_State * L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();
		if (npc)
			lua_pushnumber(L, npc->GetEmpire());
		else
			lua_pushnumber(L, 0);
		return 1;
	}

	int npc_get_race(lua_State * L)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentNPCRace());
		return 1;
	}

	int npc_get_guild(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();
		CGuild* pGuild = NULL;
		if (npc)
			pGuild = npc->GetGuild();

		lua_pushnumber(L, pGuild ? pGuild->GetID() : 0);
		return 1;
	}

	int npc_get_remain_skill_book_count(lua_State* L)
	{
		LPCHARACTER npc = CQuestManager::instance().GetCurrentNPCCharacterPtr();
		if (!npc || npc->IsPC() || npc->GetRaceNum() != xmas::MOB_SANTA_VNUM)
		{
			lua_pushnumber(L, 0);
			return 1;
		}
		lua_pushnumber(L, MAX(0, npc->GetPoint(POINT_ATT_GRADE_BONUS)));
		return 1;
	}

	int npc_dec_remain_skill_book_count(lua_State* L)
	{
		LPCHARACTER npc = CQuestManager::instance().GetCurrentNPCCharacterPtr();
		if (!npc || npc->IsPC() || npc->GetRaceNum() != xmas::MOB_SANTA_VNUM)
		{
			return 0;
		}
		npc->SetPoint(POINT_ATT_GRADE_BONUS, MAX(0, npc->GetPoint(POINT_ATT_GRADE_BONUS)-1));
		return 0;
	}

	int npc_get_remain_hairdye_count(lua_State* L)
	{
		LPCHARACTER npc = CQuestManager::instance().GetCurrentNPCCharacterPtr();
		if (!npc || npc->IsPC() || npc->GetRaceNum() != xmas::MOB_SANTA_VNUM)
		{
			lua_pushnumber(L, 0);
			return 1;
		}
		lua_pushnumber(L, MAX(0, npc->GetPoint(POINT_DEF_GRADE_BONUS)));
		return 1;
	}

	int npc_dec_remain_hairdye_count(lua_State* L)
	{
		LPCHARACTER npc = CQuestManager::instance().GetCurrentNPCCharacterPtr();
		if (!npc || npc->IsPC() || npc->GetRaceNum() != xmas::MOB_SANTA_VNUM)
		{
			return 0;
		}
		npc->SetPoint(POINT_DEF_GRADE_BONUS, MAX(0, npc->GetPoint(POINT_DEF_GRADE_BONUS)-1));
		return 0;
	}

	int npc_is_quest(lua_State * L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();

		if (npc)
		{
			const std::string & r_st = q.GetCurrentQuestName();

			if (q.GetQuestIndexByName(r_st) == npc->GetQuestBy())
			{
				lua_pushboolean(L, 1);
				return 1;
			}
		}

		lua_pushboolean(L, 0);
		return 1;
	}

	int npc_kill(lua_State * L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();

		ch->SetQuestNPCID(0);
		if (npc)
		{
			npc->Dead();
		}
		return 0;
	}

	int npc_purge(lua_State * L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();

		ch->SetQuestNPCID(0);
		if (npc)
		{
			M2_DESTROY_CHARACTER(npc);
		}
		return 0;
	}

	int npc_is_near(lua_State * L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();

		lua_Number dist = 10;

		if (lua_isnumber(L, 1))
			dist = lua_tonumber(L, 1);

		if (ch == NULL || npc == NULL)
		{
			lua_pushboolean(L, false);
		}
		else
		{
			lua_pushboolean(L, DISTANCE_APPROX(ch->GetX() - npc->GetX(), ch->GetY() - npc->GetY()) < dist*100);
		}

		return 1;
	}

	int npc_is_near_vid(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid vid");
			lua_pushboolean(L, 0);
			return 1;
		}

		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = CHARACTER_MANAGER::instance().Find((DWORD)lua_tonumber(L, 1));
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();

		lua_Number dist = 10;

		if (lua_isnumber(L, 2))
			dist = lua_tonumber(L, 2);

		if (ch == NULL || npc == NULL)
		{
			lua_pushboolean(L, false);
		}
		else
		{
			lua_pushboolean(L, DISTANCE_APPROX(ch->GetX() - npc->GetX(), ch->GetY() - npc->GetY()) < dist*100);
		}

		return 1;
	}

	int npc_unlock(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();

		if ( npc != NULL )
		{
			if (npc->IsPC())
				return 0;

			if (npc->GetQuestNPCID() == ch->GetPlayerID())
			{
				npc->SetQuestNPCID(0);
			}
		}
		return 0;
	}

	int npc_lock(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();

		if (!npc || npc->IsPC())
		{
			lua_pushboolean(L, TRUE);
			return 1;
		}

		if (npc->GetQuestNPCID() == 0 || npc->GetQuestNPCID() == ch->GetPlayerID())
		{
			npc->SetQuestNPCID(ch->GetPlayerID());
			lua_pushboolean(L, TRUE);
		}
		else
		{
			lua_pushboolean(L, FALSE);
		}

		return 1;
	}

	int npc_get_leader_vid(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();

		LPPARTY party = npc->GetParty();
		LPCHARACTER leader = party->GetLeader();

		if (leader)
			lua_pushnumber(L, leader->GetVID());
		else
			lua_pushnumber(L, 0);


		return 1;
	}

	int npc_get_vid(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();
		
		lua_pushnumber(L, npc->GetVID());


		return 1;
	}

	int npc_get_vid_attack_mul(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();

		lua_Number vid = lua_tonumber(L, 1);
		LPCHARACTER targetChar = CHARACTER_MANAGER::instance().Find(vid);

		if (targetChar)
			lua_pushnumber(L, targetChar->GetAttMul());
		else
			lua_pushnumber(L, 0);


		return 1;
	}

	int npc_set_vid_attack_mul(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();

		lua_Number vid = lua_tonumber(L, 1);
		lua_Number attack_mul = lua_tonumber(L, 2);

		LPCHARACTER targetChar = CHARACTER_MANAGER::instance().Find(vid);

		if (targetChar)
			targetChar->SetAttMul(attack_mul);

		return 0;
	}

	int npc_get_vid_damage_mul(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();

		lua_Number vid = lua_tonumber(L, 1);
		LPCHARACTER targetChar = CHARACTER_MANAGER::instance().Find(vid);

		if (targetChar)
			lua_pushnumber(L, targetChar->GetDamMul());
		else
			lua_pushnumber(L, 0);


		return 1;
	}

	int npc_set_vid_damage_mul(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();

		lua_Number vid = lua_tonumber(L, 1);
		lua_Number damage_mul = lua_tonumber(L, 2);

		LPCHARACTER targetChar = CHARACTER_MANAGER::instance().Find(vid);

		if (targetChar)
			targetChar->SetDamMul(damage_mul);

		return 0;
	}
	//New quest functions
	int npc_get_level(lua_State* L)
    {
		if (CQuestManager::instance().GetCurrentNPCCharacterPtr() != NULL)
			lua_pushnumber(L, CQuestManager::instance().GetCurrentNPCCharacterPtr()->GetLevel());
        return 1;
    }
 
    int npc_get_name(lua_State* L)
    {
        lua_pushstring(L, CQuestManager::instance().GetCurrentNPCCharacterPtr()->GetName());
        return 1;
    }
 
    int npc_get_rank(lua_State* L)
    {
        lua_pushnumber(L, CQuestManager::instance().GetCurrentNPCCharacterPtr()->GetMobRank());
        return 1;
    }
 
    int npc_get_type(lua_State* L)
    {
        lua_pushnumber(L, CQuestManager::instance().GetCurrentNPCCharacterPtr()->GetMobTable().bType);
        return 1;
    }
 
    int npc_is_metin(lua_State* L)
    {
        lua_pushboolean(L, CQuestManager::instance().GetCurrentNPCCharacterPtr()->IsStone());
        return 1;
    }
 
    int npc_is_boss(lua_State* L)
    {
        lua_pushboolean(L, CQuestManager::instance().GetCurrentNPCCharacterPtr()->GetMobRank() == MOB_RANK_BOSS);
        return 1;
    }
 
    int npc_show_effect_on_target(lua_State* L)
    {
        CQuestManager& q = CQuestManager::instance();
        LPCHARACTER ch = q.GetCurrentCharacterPtr();
        LPCHARACTER tch = q.GetCurrentNPCCharacterPtr();
 
        if (!tch || ch->GetVID() == tch->GetVID())
            return 0;
 
        if (lua_isstring(L, 1))
            tch->SpecificEffectPacket(lua_tostring(L, 2));
 
        return 0;
    }
 
    int npc_get_ip(lua_State* L)
    {
        LPCHARACTER npc = CQuestManager::instance().GetCurrentNPCCharacterPtr();
        if (npc && npc->IsPC())
            lua_pushstring(L, npc->GetDesc()->GetHostName());
        else
            lua_pushstring(L, "");
        return 1;
    }
 
    int npc_get_client_version(lua_State* L)
    {
        LPCHARACTER npc = CQuestManager::instance().GetCurrentNPCCharacterPtr();
        if (npc && npc->IsPC())
            lua_pushstring(L, npc->GetDesc()->GetClientVersion());
        else
            lua_pushstring(L, "");
        return 1;
    }
 
    int npc_get_job(lua_State* L)
    {
        LPCHARACTER npc = CQuestManager::instance().GetCurrentNPCCharacterPtr();
        if (npc && npc->IsPC())
            lua_pushnumber(L, npc->GetJob());
        else
            lua_pushnumber(L, -1);
        return 1;
    }
 
    int npc_get_pid(lua_State* L)
    {
        LPCHARACTER npc = CQuestManager::instance().GetCurrentNPCCharacterPtr();
        lua_pushnumber(L, npc->GetPlayerID());
        return 1;
    }
 
    int npc_get_exp(lua_State* L)
    {
        lua_pushnumber(L, CQuestManager::instance().GetCurrentNPCCharacterPtr()->GetMobTable().dwExp);
        return 1;
    }
	//new quest functions end
	
	void RegisterNPCFunctionTable()
	{
		luaL_reg npc_functions[] = 
		{
			{ "getrace",			npc_get_race			},
			{ "get_race",			npc_get_race			},
			{ "open_shop",			npc_open_shop			},
			{ "get_empire",			npc_get_empire			},
			{ "is_pc",				npc_is_pc			},
			{ "is_quest",			npc_is_quest			},
			{ "kill",				npc_kill			},
			{ "purge",				npc_purge			},
			{ "is_near",			npc_is_near			},
			{ "is_near_vid",			npc_is_near_vid			},
			{ "lock",				npc_lock			},
			{ "unlock",				npc_unlock			},
			{ "get_guild",			npc_get_guild			},
			{ "get_leader_vid",		npc_get_leader_vid	},
			{ "get_vid",			npc_get_vid	},
			{ "get_vid_attack_mul",		npc_get_vid_attack_mul	},
			{ "set_vid_attack_mul",		npc_set_vid_attack_mul	},
			{ "get_vid_damage_mul",		npc_get_vid_damage_mul	},
			{ "set_vid_damage_mul",		npc_set_vid_damage_mul	},
			{ "get_level",                  npc_get_level                   },
            { "get_name",                   npc_get_name                    },
            { "get_type",                   npc_get_type                    },
            { "get_rank",                   npc_get_rank                    },
            { "is_metin",                   npc_is_metin                    },
            { "is_boss",                    npc_is_boss                 },
            { "show_effect_on_target",          npc_show_effect_on_target           },
            { "get_ip",                 npc_get_ip                  },
            { "get_client_version",             npc_get_client_version              },
            { "get_job",                    npc_get_job                 },
            { "get_pid",                    npc_get_pid                 },
            { "get_exp",                    npc_get_exp                 },
			{ "get_vid_max_hp",		npc_get_vid_max_hp	},
			{ "get_vid_hp",		npc_get_vid_hp	},
			{ "get_vid_max_sp",		npc_get_vid_max_sp	},
			{ "get_vid_sp",		npc_get_vid_sp	},

			// X-mas santa special
			{ "get_remain_skill_book_count",	npc_get_remain_skill_book_count },
			{ "dec_remain_skill_book_count",	npc_dec_remain_skill_book_count },
			{ "get_remain_hairdye_count",	npc_get_remain_hairdye_count	},
			{ "dec_remain_hairdye_count",	npc_dec_remain_hairdye_count	},
			{ NULL,				NULL			    	}
		};

		CQuestManager::instance().AddLuaFunctionTable("npc", npc_functions);
	}
};
