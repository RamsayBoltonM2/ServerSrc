#include "stdafx.h"
#undef sys_err
#ifndef __WIN32__
	#define sys_err(fmt, args...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
	#define sys_err(fmt, ...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

#ifdef __NEW_EVENTS__
#include "new_events.h"
#include "char.h"
#include "questlua.h"
#include "questmanager.h"

namespace quest
{
	#ifdef __KINGDOMS_WAR__
		int kingdoms_war_start(lua_State* L)
		{
			lua_pushboolean(L, CMgrKingdomsWar::instance().Start());
			return 1;
		}
		
		int kingdoms_war_stop(lua_State* L)
		{
			lua_pushboolean(L, CMgrKingdomsWar::instance().Stop());
			return 1;
		}
		
		int kingdoms_get_map_ch(lua_State* L)
		{
			lua_pushnumber(L, KingdomsWar::MAP_CHANNEL);
			return 1;
		}
		
		int kingdoms_get_map_index(lua_State* L)
		{
			lua_pushnumber(L, KingdomsWar::MAP_INDEX);
			return 1;
		}
		
		int kingdoms_get_stat_flagname(lua_State* L)
		{
			std::string l = KingdomsWar::EVENT_STAT;
			lua_pushstring(L, l.c_str());
			return 1;
		}
		
		int kingdoms_get_stat_flag_end(lua_State* L)
		{
			lua_pushnumber(L, KingdomsWar::END);
			return 1;
		}
		
		int kingdoms_register(lua_State* L)
		{
			CMgrKingdomsWar::instance().Register(CQuestManager::instance().GetCurrentCharacterPtr());
			return 0;
		}
		
		int kingdoms_unregister(lua_State* L)
		{
			CMgrKingdomsWar::instance().Unregister(CQuestManager::instance().GetCurrentCharacterPtr());
			return 0;
		}
		
		int kingdoms_get_minlv_flagname(lua_State* L)
		{
			std::string l = KingdomsWar::EVENT_MIN_LV;
			lua_pushstring(L, l.c_str());
			return 1;
		}
		
		int kingdoms_get_maxlv_flagname(lua_State* L)
		{
			std::string l = KingdomsWar::EVENT_MAX_LV;
			lua_pushstring(L, l.c_str());
			return 1;
		}
		
		int kingdoms_config(lua_State* L)
		{
			if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 5) || !lua_isnumber(L, 6) || !lua_isnumber(L, 7) || !lua_isnumber(L, 8))
			{
				sys_err("Invalid argument.");
				lua_pushboolean(L, false);
				return 1;
			}
			
			lua_pushboolean(L, CMgrKingdomsWar::instance().Config(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5), lua_tonumber(L, 6), lua_tonumber(L, 7), lua_tonumber(L, 8)));
			return 1;
		}
		
		int kingdoms_get_ryang_flagname(lua_State* L)
		{
			std::string l = KingdomsWar::REWARD_YANG;
			lua_pushstring(L, l.c_str());
			return 1;
		}
		
		int kingdoms_get_ritem_flagname(lua_State* L)
		{
			std::string l = KingdomsWar::REWARD_VNUM;
			lua_pushstring(L, l.c_str());
			return 1;
		}
		
		int kingdoms_get_rcount_flagname(lua_State* L)
		{
			std::string l = KingdomsWar::REWARD_COUNT;
			lua_pushstring(L, l.c_str());
			return 1;
		}
		
		int kingdoms_get_tyang_flagname(lua_State* L)
		{
			std::string l = KingdomsWar::REWARD_B_YANG;
			lua_pushstring(L, l.c_str());
			return 1;
		}
		
		int kingdoms_get_titem_flagname(lua_State* L)
		{
			std::string l = KingdomsWar::REWARD_B_VNUM;
			lua_pushstring(L, l.c_str());
			return 1;
		}
		
		int kingdoms_get_tcount_flagname(lua_State* L)
		{
			std::string l = KingdomsWar::REWARD_B_COUNT;
			lua_pushstring(L, l.c_str());
			return 1;
		}
		
		int kingdoms_get_priv_reward(lua_State* L)
		{
			lua_pushnumber(L, KingdomsWar::ITEM_DROP);
			lua_pushnumber(L, KingdomsWar::GOLD_DROP);
			lua_pushnumber(L, KingdomsWar::GOLD10_DROP);
			lua_pushnumber(L, KingdomsWar::EXP);
			lua_pushnumber(L, KingdomsWar::PRIV_TIME);
			return 5;
		}
		
		int kingdoms_get_limits(lua_State* L)
		{
			int iTime = int(KingdomsWar::MAX_TIME_LIMIT) / 60;
			lua_pushnumber(L, iTime);
			lua_pushnumber(L, KingdomsWar::LIMIT_WIN_KILLS);
			lua_pushnumber(L, KingdomsWar::DEAD_LIMIT);
			return 3;
		}
		
		void RegisterKingdomsWarFunctionTable()
		{
			luaL_reg functions[] =
			{
				{"start", kingdoms_war_start},
				{"stop", kingdoms_war_stop},
				{"get_map_ch", kingdoms_get_map_ch},
				{"get_map_index", kingdoms_get_map_index},
				{"get_stat_flagname", kingdoms_get_stat_flagname},
				{"get_stat_flag_end", kingdoms_get_stat_flag_end},
				{"register", kingdoms_register},
				{"unregister", kingdoms_unregister},
				{"get_minlv_flagname", kingdoms_get_minlv_flagname},
				{"get_maxlv_flagname", kingdoms_get_maxlv_flagname},
				{"config", kingdoms_config},
				{"get_ryang_flagname", kingdoms_get_ryang_flagname},
				{"get_ritem_flagname", kingdoms_get_ritem_flagname},
				{"get_rcount_flagname", kingdoms_get_rcount_flagname},
				{"get_tyang_flagname", kingdoms_get_tyang_flagname},
				{"get_titem_flagname", kingdoms_get_titem_flagname},
				{"get_tcount_flagname", kingdoms_get_tcount_flagname},
				{"get_priv_reward", kingdoms_get_priv_reward},
				{"get_limits", kingdoms_get_limits},
				{NULL, NULL}
			};
			
			CQuestManager::instance().AddLuaFunctionTable("kingdoms_war", functions);
		}
	#endif
}
#endif
