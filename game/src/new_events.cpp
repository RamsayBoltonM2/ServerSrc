#include "stdafx.h"
#include "new_events.h"

#ifdef __NEW_EVENTS__
	#ifdef __KINGDOMS_WAR__
		#include "utils.h"
		#include "db.h"
		#include "log.h"
		#include "config.h"
		#include "char.h"
		#include "desc.h"
		#include "char_manager.h"
		#include "item_manager.h"
		#include "questmanager.h"
		#include "priv_manager.h"
		#include "sectree_manager.h"
		#include "start_position.h"
		#include <boost/lexical_cast.hpp>
		
		namespace KingdomsWar
		{
			char const * MSG[] = {
									"Razboiul dintre regate doar a inceput,",
									"Daca doriti sa participati la razboi vorbeste cu Administratorul Luptelor.",
									"Războiul dintre regate este pe sfarsite,",
									"Toti participantii vor fi teleportati in regat sau.",
									"Acest articol nu poate fi folosit aici.",
									"%s castiga razboiul,",
									"Toti participanti vor fi teleportati in regat sau.",
									"Cetateni %s isi pot retrage recompensa de depozit (Mall).",
									"Razboiul dintre regate se incheie fara nici un castigator",
									"Toti partecipantii vor fi teleportati in regatul sau.",
			};
			
			std::string EVENT_STAT = "kw_stat", EVENT_MIN_LV = "kw_lv_min", EVENT_MAX_LV = "kw_lv_max", REWARD_YANG = "kw_yang_count", REWARD_VNUM = "kw_item_vnum", REWARD_COUNT = "kw_item_count", REWARD_B_YANG = "kw_t_yang_count", REWARD_B_VNUM = "kw_t_item_vnum", REWARD_B_COUNT = "kw_t_item_count";
			
			BYTE IS_UNACCPETABLE_ITEM(DWORD dwVnum)
			{
				switch (dwVnum)
				{
					case 39011:
					case 39012:
					case 39013:
					case 71054:
						return 1;
				}
				
				return 0;
			};
		}
		
		struct FTeleportCity
		{
			void operator () (LPENTITY ent)
			{
				if (ent->IsType(ENTITY_CHARACTER))
				{
					LPCHARACTER pChar = (LPCHARACTER) ent;
					if ((pChar->IsPC()) && (pChar->GetGMLevel() == GM_PLAYER))
						pChar->WarpSet(EMPIRE_START_X(pChar->GetEmpire()), EMPIRE_START_Y(pChar->GetEmpire()));
				}
			}
		};
		
		struct FRefreshKills
		{
			int		m_iKills[EMPIRE_MAX_NUM - 1];
			
			void operator() (LPENTITY ent)
			{
				if (ent->IsType(ENTITY_CHARACTER))
				{
					LPCHARACTER pChar = (LPCHARACTER) ent;
					if (pChar->IsPC())
					{
						TPacketKingdomWar sPacket;
						sPacket.bHeader = HEADER_GC_KINGDOMSWAR;
						sPacket.bSubHeader = KINGDOMSWAR_SUBHEADER_GC_REFRESH;
						thecore_memcpy(sPacket.iKills, m_iKills, sizeof(sPacket.iKills));
						sPacket.iLimitKills = KingdomsWar::LIMIT_WIN_KILLS;
						sPacket.iDeads = 0;
						sPacket.iLimitDeads = 0;
						sPacket.dwTimeRemained = 0;
						pChar->GetDesc()->Packet(&sPacket, sizeof(TPacketKingdomWar));
					}
				}
			}
		};
		
		EVENTINFO(kingdoms_war_info)
		{
			BYTE bProtect;
		};
		
		EVENTFUNC(kingdoms_war_event)
		{
			kingdoms_war_info* pEventInfo = dynamic_cast<kingdoms_war_info*>(event->info);
			if (pEventInfo)
			{
				if (pEventInfo->bProtect == 1)
					CMgrKingdomsWar::instance().DeclareWinner();
			}
			
			return 0;
		}
		
		CMgrKingdomsWar::CMgrKingdomsWar()
		{
			bWinner = false;
			dwTimeRemained = 0;
			e_Limit = NULL;
			mPartecipants1.clear(); mPartecipants2.clear(); mPartecipants3.clear();
			memset(kills, 0, sizeof(kills));
			memset(&reward, 0, sizeof(reward));
		}
		
		CMgrKingdomsWar::~CMgrKingdomsWar()
		{
			if (e_Limit != NULL)
			{
				event_cancel(&e_Limit);
				e_Limit = NULL;
			}
			
			mPartecipants1.clear(); mPartecipants2.clear(); mPartecipants3.clear();
		}
		
		bool CMgrKingdomsWar::Start()
		{
			if (g_bChannel != KingdomsWar::MAP_CHANNEL)
				return false;
			
			if (quest::CQuestManager::instance().GetEventFlag(KingdomsWar::EVENT_STAT) != KingdomsWar::END)
				return false;
			
			mPartecipants1.clear(); mPartecipants2.clear(); mPartecipants3.clear();
			memset(kills, 0, sizeof(kills));
			memset(&reward, 0, sizeof(reward));
			
			TItemTable * pTable;
			int r_yang = quest::CQuestManager::instance().GetEventFlag(KingdomsWar::REWARD_YANG), b_yang = quest::CQuestManager::instance().GetEventFlag(KingdomsWar::REWARD_B_YANG); r_yang = (r_yang <= 0) ? 0 : r_yang; b_yang = (b_yang <= 0) ? 0 : b_yang;
			DWORD r_dwItemVnum = quest::CQuestManager::instance().GetEventFlag(KingdomsWar::REWARD_VNUM), b_dwItemVnum = quest::CQuestManager::instance().GetEventFlag(KingdomsWar::REWARD_B_VNUM); r_dwItemVnum = ((pTable = ITEM_MANAGER::instance().GetTable(r_dwItemVnum)) == NULL) ? 0 : r_dwItemVnum; b_dwItemVnum = ((pTable = ITEM_MANAGER::instance().GetTable(b_dwItemVnum)) == NULL) ? 0 : b_dwItemVnum;
			DWORD r_dwItemCount = quest::CQuestManager::instance().GetEventFlag(KingdomsWar::REWARD_COUNT), b_dwItemCount = quest::CQuestManager::instance().GetEventFlag(KingdomsWar::REWARD_B_COUNT); r_dwItemCount = (r_dwItemCount <= 0) ? 0 : r_dwItemCount; b_dwItemCount = (b_dwItemCount <= 0) ? 0 : b_dwItemCount;
			DWORD dwMinLevel = quest::CQuestManager::instance().GetEventFlag(KingdomsWar::EVENT_MIN_LV), dwMaxLevel = quest::CQuestManager::instance().GetEventFlag(KingdomsWar::EVENT_MAX_LV); dwMinLevel = (dwMinLevel <= 0) ? 0 : dwMinLevel; dwMaxLevel = (dwMaxLevel <= 0) ? 0 : dwMaxLevel;
			reward.r_yang = r_yang;
			reward.r_dwItemVnum = r_dwItemVnum;
			reward.r_dwItemCount = r_dwItemCount;
			reward.b_yang = b_yang;
			reward.b_dwItemVnum = b_dwItemVnum;
			reward.b_dwItemCount = b_dwItemCount;
			reward.dwMinLevel = dwMinLevel;
			reward.dwMaxLevel = dwMaxLevel;
			
			for (int i = 0; i < 2; ++i)
			{
				char buf[201] = {0};
				#ifdef __MULTI_LANGUAGE_SYSTEM__
				snprintf(buf, sizeof(buf), LC_TEXT(ENGLISH, KingdomsWar::MSG[i]));
				#else
				snprintf(buf, sizeof(buf), LC_TEXT(KingdomsWar::MSG[i]));
				#endif
				BroadcastNotice(buf);
			}
			
			quest::CQuestManager::instance().RequestSetEventFlag(KingdomsWar::EVENT_STAT, KingdomsWar::START);
			bWinner = false;
			
			if (e_Limit != NULL)
				event_cancel(&e_Limit);
			
			kingdoms_war_info* pEventInfo = AllocEventInfo<kingdoms_war_info>();
			pEventInfo->bProtect = 1;
			e_Limit = event_create(kingdoms_war_event, pEventInfo, PASSES_PER_SEC(KingdomsWar::MAX_TIME_LIMIT));
			
			LogManager::instance().KingdomsWarLog("EVENT_START");
			dwTimeRemained = get_global_time() + DWORD(KingdomsWar::MAX_TIME_LIMIT);
			return true;
		}
		
		bool CMgrKingdomsWar::Stop()
		{
			if (g_bChannel != KingdomsWar::MAP_CHANNEL)
				return false;
			
			if (quest::CQuestManager::instance().GetEventFlag(KingdomsWar::EVENT_STAT) != KingdomsWar::START)
				return false;
			
			if (e_Limit != NULL)
			{
				event_cancel(&e_Limit);
				e_Limit = NULL;
			}
			
			mPartecipants1.clear(); mPartecipants2.clear(); mPartecipants3.clear();
			memset(kills, 0, sizeof(kills));
			memset(&reward, 0, sizeof(reward));
			
			for (int i = 2; i < 4; ++i)
			{
				char buf[201] = {0};
				#ifdef __MULTI_LANGUAGE_SYSTEM__
				snprintf(buf, sizeof(buf), LC_TEXT(ENGLISH, KingdomsWar::MSG[i]));
				#else
				snprintf(buf, sizeof(buf), LC_TEXT(KingdomsWar::MSG[i]));
				#endif
				BroadcastNotice(buf);
			}
			
			quest::CQuestManager::instance().RequestSetEventFlag(KingdomsWar::EVENT_STAT, KingdomsWar::END);
			
			LPSECTREE_MAP pSectreeMap = SECTREE_MANAGER::instance().GetMap(KingdomsWar::MAP_INDEX);
			if (pSectreeMap != NULL)
			{
				FTeleportCity pChar;
				pSectreeMap->for_each(pChar);
			}
			
			bWinner = false;
			LogManager::instance().KingdomsWarLog("EVENT_END BY_GM");
			dwTimeRemained = 0;
			return true;
		}
		
		bool CMgrKingdomsWar::Config(DWORD dwMinLv, DWORD dwMaxLv, int r_iYang, DWORD r_dwItemVnum, DWORD r_dwItemCount, int t_iYang, DWORD t_dwItemVnum, DWORD t_dwItemCount)
		{
			if (g_bChannel != KingdomsWar::MAP_CHANNEL)
				return false;
			
			if (quest::CQuestManager::instance().GetEventFlag(KingdomsWar::EVENT_STAT) != KingdomsWar::END)
				return false;
			
			quest::CQuestManager::instance().RequestSetEventFlag(KingdomsWar::EVENT_MIN_LV, dwMinLv);
			quest::CQuestManager::instance().RequestSetEventFlag(KingdomsWar::EVENT_MAX_LV, dwMaxLv);
			quest::CQuestManager::instance().RequestSetEventFlag(KingdomsWar::REWARD_YANG, r_iYang);
			quest::CQuestManager::instance().RequestSetEventFlag(KingdomsWar::REWARD_VNUM, r_dwItemVnum);
			quest::CQuestManager::instance().RequestSetEventFlag(KingdomsWar::REWARD_COUNT, r_dwItemCount);
			quest::CQuestManager::instance().RequestSetEventFlag(KingdomsWar::REWARD_B_YANG, t_iYang);
			quest::CQuestManager::instance().RequestSetEventFlag(KingdomsWar::REWARD_B_VNUM, t_dwItemVnum);
			quest::CQuestManager::instance().RequestSetEventFlag(KingdomsWar::REWARD_B_COUNT, t_dwItemCount);
			return true;
		}
		
		void CMgrKingdomsWar::Register(LPCHARACTER pChar)
		{
			if ((g_bChannel != KingdomsWar::MAP_CHANNEL) || (!pChar) || (quest::CQuestManager::instance().GetEventFlag(KingdomsWar::EVENT_STAT) != KingdomsWar::START))
				return;
			
			const char* acc_name = NULL;
			LPDESC pDesc = pChar->GetDesc();
			if (pDesc)
				acc_name = pDesc->GetAccountTable().login;
			
			BYTE bEmpire = pChar->GetEmpire();
			DWORD dwPlayerID = pChar->GetPlayerID();
			switch (bEmpire)
			{
				case 1:
					{
						if (mPartecipants1.find(dwPlayerID) == mPartecipants1.end())
						{
							KingdomsWar::tPartecipants s;
							s.kill = 0;
							s.killed = 0;
							if (acc_name != NULL)
								s.account_name = acc_name;
							else
								s.account_name = "KW";
							
							mPartecipants1.insert(std::make_pair(dwPlayerID, s));
						}
					}
					break;
				case 2:
					{
						if (mPartecipants2.find(dwPlayerID) == mPartecipants2.end())
						{
							KingdomsWar::tPartecipants s;
							s.kill = 0;
							s.killed = 0;
							if (acc_name != NULL)
								s.account_name = acc_name;
							else
								s.account_name = "KW";
							
							mPartecipants2.insert(std::make_pair(dwPlayerID, s));
						}
					}
					break;
				case 3:
					{
						if (mPartecipants3.find(dwPlayerID) == mPartecipants3.end())
						{
							KingdomsWar::tPartecipants s;
							s.kill = 0;
							s.killed = 0;
							if (acc_name != NULL)
								s.account_name = acc_name;
							else
								s.account_name = "KW";
							
							mPartecipants3.insert(std::make_pair(dwPlayerID, s));
						}
					}
					break;
			}
			
			if (pDesc)
			{
				int iTimeRemained = int(DWORD(dwTimeRemained - get_global_time()) / 60);
				int iDeads = 0;
				if (bEmpire == 1)
				{
					itertype(mPartecipants1) iterFind = mPartecipants1.find(dwPlayerID);
					if (iterFind != mPartecipants1.end())
						iDeads = iterFind->second.killed;
				}
				else if (bEmpire == 2)
				{
					itertype(mPartecipants2) iterFind = mPartecipants2.find(dwPlayerID);
					if (iterFind != mPartecipants2.end())
						iDeads = iterFind->second.killed;
				}
				else if (bEmpire == 3)
				{
					itertype(mPartecipants3) iterFind = mPartecipants3.find(dwPlayerID);
					if (iterFind != mPartecipants3.end())
						iDeads = iterFind->second.killed;
				}
				
				TPacketKingdomWar sPacket;
				sPacket.bHeader = HEADER_GC_KINGDOMSWAR;
				sPacket.bSubHeader = KINGDOMSWAR_SUBHEADER_GC_OPEN;
				thecore_memcpy(sPacket.iKills, kills, sizeof(sPacket.iKills));
				sPacket.iLimitKills = KingdomsWar::LIMIT_WIN_KILLS;
				sPacket.iDeads = iDeads;
				sPacket.iLimitDeads = KingdomsWar::DEAD_LIMIT;
				sPacket.dwTimeRemained = iTimeRemained;
				pDesc->Packet(&sPacket, sizeof(TPacketKingdomWar));
			}
		}
		
		void CMgrKingdomsWar::Unregister(LPCHARACTER pChar)
		{
			if ((g_bChannel != KingdomsWar::MAP_CHANNEL) || (!pChar) || (quest::CQuestManager::instance().GetEventFlag(KingdomsWar::EVENT_STAT) != KingdomsWar::START))
				return;
			
			bool bRegistered = false;
			BYTE bEmpire = pChar->GetEmpire();
			DWORD dwPlayerID = pChar->GetPlayerID();
			switch (bEmpire)
			{
				case 1:
					{
						itertype(mPartecipants1) iterFind = mPartecipants1.find(dwPlayerID);
						if (iterFind != mPartecipants1.end())
						{
							int kills_by = iterFind->second.kill;
							if (kills_by > 0)
							{
								int iAmount = GetKillsScore(bEmpire) - kills_by;
								if (iAmount < 0)
									iAmount = 0;
								
								SetKillForce(bEmpire, iAmount);
								LPSECTREE_MAP pSectreeMap = SECTREE_MANAGER::instance().GetMap(KingdomsWar::MAP_INDEX);
								if (pSectreeMap != NULL)
								{
									FRefreshKills f;
									thecore_memcpy(f.m_iKills, kills, sizeof(f.m_iKills));
									pSectreeMap->for_each(f);
								}
							}
							
							mPartecipants1.erase(dwPlayerID);
							bRegistered = true;
						}
					}
					break;
				case 2:
					{
						itertype(mPartecipants2) iterFind = mPartecipants2.find(dwPlayerID);
						if (iterFind != mPartecipants2.end())
						{
							int kills_by = iterFind->second.kill;
							if (kills_by > 0)
							{
								int iAmount = GetKillsScore(bEmpire) - kills_by;
								if (iAmount < 0)
									iAmount = 0;
								
								SetKillForce(bEmpire, iAmount);
								LPSECTREE_MAP pSectreeMap = SECTREE_MANAGER::instance().GetMap(KingdomsWar::MAP_INDEX);
								if (pSectreeMap != NULL)
								{
									FRefreshKills f;
									thecore_memcpy(f.m_iKills, kills, sizeof(f.m_iKills));
									pSectreeMap->for_each(f);
								}
							}
							
							mPartecipants2.erase(dwPlayerID);
							bRegistered = true;
						}
					}
					break;
				case 3:
					{
						itertype(mPartecipants3) iterFind = mPartecipants3.find(dwPlayerID);
						if (iterFind != mPartecipants3.end())
						{
							int kills_by = iterFind->second.kill;
							if (kills_by > 0)
							{
								int iAmount = GetKillsScore(bEmpire) - kills_by;
								if (iAmount < 0)
									iAmount = 0;
								
								SetKillForce(bEmpire, iAmount);
								LPSECTREE_MAP pSectreeMap = SECTREE_MANAGER::instance().GetMap(KingdomsWar::MAP_INDEX);
								if (pSectreeMap != NULL)
								{
									FRefreshKills f;
									thecore_memcpy(f.m_iKills, kills, sizeof(f.m_iKills));
									pSectreeMap->for_each(f);
								}
							}
							
							mPartecipants3.erase(dwPlayerID);
							bRegistered = true;
						}
					}
					break;
			}
			
			if (bRegistered)
			{
				pChar->SetQuestFlag("kingdomswar_mgr.is_enough", 1);
				pChar->WarpSet(EMPIRE_START_X(bEmpire), EMPIRE_START_Y(bEmpire));
			}
		}
		
		void CMgrKingdomsWar::DeclareWinner()
		{
			if (e_Limit != NULL)
			{
				event_cancel(&e_Limit);
				e_Limit = NULL;
			}
			
			bWinner = true;
			BYTE bEmpire = 0;
			int k = 0;
			int k_kills[EMPIRE_MAX_NUM - 1] = {GetKillsScore(1), GetKillsScore(2), GetKillsScore(3)};
			k = (k_kills[0] > k_kills[1]) ? k_kills[0] : k_kills[1]; k = (k_kills[2] > k) ? k_kills[2] : k;
			if (k != 0)
			{
				for(int i = 0; i < EMPIRE_MAX_NUM; i++)
				{
					if (k_kills[i] == k)
					{
						bEmpire = i + 1;
						break;
					}
				}
			}
			
			if (bEmpire != 0)
			{
				int iTopKills = 0;
				DWORD dwTopPlayerID = 0;
				switch (bEmpire)
				{
					case 1:
						{
							itertype(mPartecipants1) iter = mPartecipants1.begin();
							for (; iter != mPartecipants1.end(); ++iter)
							{
								DWORD dwPlayerID = iter->first;
								int iKills = iter->second.kill;
								std::string acc_name = iter->second.account_name;
								if (reward.r_yang > 0)
									DBManager::instance().Query("INSERT INTO item_award (pid, login, vnum, socket0, given_time, why, mall) VALUES(%d, '%s', %d, %d, NOW(), '%s', 1);", dwPlayerID, acc_name.c_str(), ITEM_ELK_VNUM, reward.r_yang, "KINGDOMS_WAR YANG REWARD");
								
								if ((reward.r_dwItemVnum > 0) && (reward.r_dwItemCount))
									DBManager::instance().Query("INSERT INTO item_award (pid, login, vnum, count, given_time, why, mall) VALUES(%d, '%s', %d, %d, NOW(), '%s', 1);", dwPlayerID, acc_name.c_str(), reward.r_dwItemVnum, reward.r_dwItemCount, "KINGDOMS_WAR ITEM REWARD");
								
								if (iKills > iTopKills)
								{
									iTopKills = iKills;
									dwTopPlayerID = dwPlayerID;
								}
							}
							
							itertype(mPartecipants1) iterFind = mPartecipants1.find(dwTopPlayerID);
							if (iterFind != mPartecipants1.end())
							{
								std::string acc_name = iterFind->second.account_name;
								if (reward.b_yang > 0)
									DBManager::instance().Query("INSERT INTO item_award (pid, login, vnum, socket0, given_time, why, mall) VALUES(%d, '%s', %d, %d, NOW(), '%s', 1);", dwTopPlayerID, acc_name.c_str(), ITEM_ELK_VNUM, reward.b_yang, "KINGDOMS_WAR YANG REWARD - TOP KILLS");
								
								if ((reward.b_dwItemVnum > 0) && (reward.b_dwItemCount))
									DBManager::instance().Query("INSERT INTO item_award (pid, login, vnum, count, given_time, why, mall) VALUES(%d, '%s', %d, %d, NOW(), '%s', 1);", dwTopPlayerID, acc_name.c_str(), reward.b_dwItemVnum, reward.b_dwItemCount, "KINGDOMS_WAR ITEM REWARD - TOP KILLS");
							}
						}
						break;
					case 2:
						{
							itertype(mPartecipants2) iter = mPartecipants2.begin();
							for (; iter != mPartecipants2.end(); ++iter)
							{
								DWORD dwPlayerID = iter->first;
								int iKills = iter->second.kill;
								std::string acc_name = iter->second.account_name;
								if (reward.r_yang > 0)
									DBManager::instance().Query("INSERT INTO item_award (pid, login, vnum, socket0, given_time, why, mall) VALUES(%d, '%s', %d, %d, NOW(), '%s', 1);", dwPlayerID, acc_name.c_str(), ITEM_ELK_VNUM, reward.r_yang, "KINGDOMS_WAR YANG REWARD");
								
								if ((reward.r_dwItemVnum > 0) && (reward.r_dwItemCount))
									DBManager::instance().Query("INSERT INTO item_award (pid, login, vnum, count, given_time, why, mall) VALUES(%d, '%s', %d, %d, NOW(), '%s', 1);", dwPlayerID, acc_name.c_str(), reward.r_dwItemVnum, reward.r_dwItemCount, "KINGDOMS_WAR ITEM REWARD");
								
								if (iKills > iTopKills)
								{
									iTopKills = iKills;
									dwTopPlayerID = dwPlayerID;
								}
							}
							
							itertype(mPartecipants2) iterFind = mPartecipants2.find(dwTopPlayerID);
							if (iterFind != mPartecipants2.end())
							{
								std::string acc_name = iterFind->second.account_name;
								if (reward.b_yang > 0)
									DBManager::instance().Query("INSERT INTO item_award (pid, login, vnum, socket0, given_time, why, mall) VALUES(%d, '%s', %d, %d, NOW(), '%s', 1);", dwTopPlayerID, acc_name.c_str(), ITEM_ELK_VNUM, reward.b_yang, "KINGDOMS_WAR YANG REWARD - TOP KILLS");
								
								if ((reward.b_dwItemVnum > 0) && (reward.b_dwItemCount))
									DBManager::instance().Query("INSERT INTO item_award (pid, login, vnum, count, given_time, why, mall) VALUES(%d, '%s', %d, %d, NOW(), '%s', 1);", dwTopPlayerID, acc_name.c_str(), reward.b_dwItemVnum, reward.b_dwItemCount, "KINGDOMS_WAR ITEM REWARD - TOP KILLS");
							}
						}
						break;
					case 3:
						{
							itertype(mPartecipants3) iter = mPartecipants3.begin();
							for (; iter != mPartecipants3.end(); ++iter)
							{
								DWORD dwPlayerID = iter->first;
								int iKills = iter->second.kill;
								std::string acc_name = iter->second.account_name;
								if (reward.r_yang > 0)
									DBManager::instance().Query("INSERT INTO item_award (pid, login, vnum, socket0, given_time, why, mall) VALUES(%d, '%s', %d, %d, NOW(), '%s', 1);", dwPlayerID, acc_name.c_str(), ITEM_ELK_VNUM, reward.r_yang, "KINGDOMS_WAR YANG REWARD");
								
								if ((reward.r_dwItemVnum > 0) && (reward.r_dwItemCount))
									DBManager::instance().Query("INSERT INTO item_award (pid, login, vnum, count, given_time, why, mall) VALUES(%d, '%s', %d, %d, NOW(), '%s', 1);", dwPlayerID, acc_name.c_str(), reward.r_dwItemVnum, reward.r_dwItemCount, "KINGDOMS_WAR ITEM REWARD");
								
								if (iKills > iTopKills)
								{
									iTopKills = iKills;
									dwTopPlayerID = dwPlayerID;
								}
							}
							
							itertype(mPartecipants3) iterFind = mPartecipants3.find(dwTopPlayerID);
							if (iterFind != mPartecipants3.end())
							{
								std::string acc_name = iterFind->second.account_name;
								if (reward.b_yang > 0)
									DBManager::instance().Query("INSERT INTO item_award (pid, login, vnum, socket0, given_time, why, mall) VALUES(%d, '%s', %d, %d, NOW(), '%s', 1);", dwTopPlayerID, acc_name.c_str(), ITEM_ELK_VNUM, reward.b_yang, "KINGDOMS_WAR YANG REWARD - TOP KILLS");
								
								if ((reward.b_dwItemVnum > 0) && (reward.b_dwItemCount))
									DBManager::instance().Query("INSERT INTO item_award (pid, login, vnum, count, given_time, why, mall) VALUES(%d, '%s', %d, %d, NOW(), '%s', 1);", dwTopPlayerID, acc_name.c_str(), reward.b_dwItemVnum, reward.b_dwItemCount, "KINGDOMS_WAR ITEM REWARD - TOP KILLS");
							}
						}
						break;
				}
				
				for (int i = 5; i < 8; ++i)
				{
					char buf[201] = {0};
					#ifdef __MULTI_LANGUAGE_SYSTEM__
					if (i == 6)
						snprintf(buf, sizeof(buf), LC_TEXT(ENGLISH, KingdomsWar::MSG[i]));
					else
						snprintf(buf, sizeof(buf), LC_TEXT(ENGLISH, KingdomsWar::MSG[i]), LC_TEXT(ENGLISH, g_nation_name[bEmpire]));
					#else
					if (i == 6)
						snprintf(buf, sizeof(buf), LC_TEXT(KingdomsWar::MSG[i]));
					else
						snprintf(buf, sizeof(buf), LC_TEXT(KingdomsWar::MSG[i]), LC_TEXT(g_nation_name[bEmpire]));
					#endif
					BroadcastNotice(buf);
				}
				
				int iPrivTime = int(KingdomsWar::PRIV_TIME) * (60 * 60);
				if (iPrivTime > 0)
				{
					int rate[4] = {int(KingdomsWar::ITEM_DROP), int(KingdomsWar::GOLD_DROP), int(KingdomsWar::GOLD10_DROP), int(KingdomsWar::EXP)};
					for(int i = 0; i < 4; i++)
					{
						if (rate[i] > 0)
							CPrivManager::instance().RequestGiveEmpirePriv(bEmpire, int(i + 1), rate[i], iPrivTime);
					}
				}
				
				const char * cLog;
				std::string sLog = std::string("EVENT_END: WINNER IS KINGDOM[") + boost::lexical_cast<std::string>(DWORD(bEmpire)) + std::string("] TOP PLAYER IS [") + boost::lexical_cast<std::string>(dwTopPlayerID) + std::string("]");
				cLog = sLog.c_str();
				LogManager::instance().KingdomsWarLog(cLog);
			}
			else
			{
				for (int i = 8; i < 10; ++i)
				{
					char buf[201] = {0};
					#ifdef __MULTI_LANGUAGE_SYSTEM__
					snprintf(buf, sizeof(buf), LC_TEXT(ENGLISH, KingdomsWar::MSG[i]));
					#else
					snprintf(buf, sizeof(buf), LC_TEXT(KingdomsWar::MSG[i]));
					#endif
					BroadcastNotice(buf);
				}
				
				LogManager::instance().KingdomsWarLog("EVENT_END WITHOUT_WINNER");
			}
			
			mPartecipants1.clear(); mPartecipants2.clear(); mPartecipants3.clear();
			memset(kills, 0, sizeof(kills));
			memset(&reward, 0, sizeof(reward));
			
			quest::CQuestManager::instance().RequestSetEventFlag(KingdomsWar::EVENT_STAT, KingdomsWar::END);
			
			LPSECTREE_MAP pSectreeMap = SECTREE_MANAGER::instance().GetMap(KingdomsWar::MAP_INDEX);
			if (pSectreeMap != NULL)
			{
				FTeleportCity pChar;
				pSectreeMap->for_each(pChar);
			}
			
			dwTimeRemained = 0;
		}
		
		void CMgrKingdomsWar::OnKill(DWORD dwPlayerID1, BYTE bEmpire1, DWORD dwPlayerID2, BYTE bEmpire2)
		{
			if ((g_bChannel != KingdomsWar::MAP_CHANNEL) || (quest::CQuestManager::instance().GetEventFlag(KingdomsWar::EVENT_STAT) != KingdomsWar::START))
				return;
			
			if (((GetKillsScore(bEmpire1) + 1) >= int(KingdomsWar::LIMIT_WIN_KILLS)) && (!bWinner) && ((int(KingdomsWar::LIMIT_WIN_KILLS) > 0)))
			{
				DeclareWinner();
				return;
			}
			else
				SetKill(bEmpire1);
			
			switch (bEmpire1)
			{
				case 1:
					{
						itertype(mPartecipants1) iter = mPartecipants1.find(dwPlayerID1);
						if (iter != mPartecipants1.end())
							iter->second.kill += 1;
					}
					break;
				case 2:
					{
						itertype(mPartecipants2) iter = mPartecipants2.find(dwPlayerID1);
						if (iter != mPartecipants2.end())
							iter->second.kill += 1;
					}
					break;
				case 3:
					{
						itertype(mPartecipants3) iter = mPartecipants3.find(dwPlayerID1);
						if (iter != mPartecipants3.end())
							iter->second.kill += 1;
					}
					break;
			}
			
			LPCHARACTER pChar = CHARACTER_MANAGER::instance().FindByPID(dwPlayerID2);
			if (pChar)
			{
				switch (bEmpire2)
				{
					case 1:
						{
							itertype(mPartecipants1) iter = mPartecipants1.find(dwPlayerID2);
							if (iter != mPartecipants1.end())
							{
								iter->second.killed += 1;
								
								TPacketKingdomWar sPacket;
								sPacket.bHeader = HEADER_GC_KINGDOMSWAR;
								sPacket.bSubHeader = KINGDOMSWAR_SUBHEADER_GC_REFRESH;
								for (int i = 0; i < int(EMPIRE_MAX_NUM - 1); ++i)
									sPacket.iKills[i] = 0;
								sPacket.iLimitKills = 0;
								sPacket.iDeads = iter->second.killed;
								sPacket.iLimitDeads = int(KingdomsWar::DEAD_LIMIT);
								sPacket.dwTimeRemained = 0;
								pChar->GetDesc()->Packet(&sPacket, sizeof(TPacketKingdomWar));
								
								if (iter->second.killed >= int(KingdomsWar::DEAD_LIMIT))
								{
									pChar->SetQuestFlag("kingdomswar_mgr.is_enough", 1);
									pChar->WarpSet(EMPIRE_START_X(bEmpire2), EMPIRE_START_Y(bEmpire2));
								}
							}
						}
						break;
					case 2:
						{
							itertype(mPartecipants2) iter = mPartecipants2.find(dwPlayerID2);
							if (iter != mPartecipants2.end())
							{
								iter->second.killed += 1;
								
								TPacketKingdomWar sPacket;
								sPacket.bHeader = HEADER_GC_KINGDOMSWAR;
								sPacket.bSubHeader = KINGDOMSWAR_SUBHEADER_GC_REFRESH;
								for (int i = 0; i < int(EMPIRE_MAX_NUM - 1); ++i)
									sPacket.iKills[i] = 0;
								sPacket.iLimitKills = 0;
								sPacket.iDeads = iter->second.killed;
								sPacket.iLimitDeads = int(KingdomsWar::DEAD_LIMIT);
								sPacket.dwTimeRemained = 0;
								pChar->GetDesc()->Packet(&sPacket, sizeof(TPacketKingdomWar));
								
								if (iter->second.killed >= int(KingdomsWar::DEAD_LIMIT))
								{
									pChar->SetQuestFlag("kingdomswar_mgr.is_enough", 1);
									pChar->WarpSet(EMPIRE_START_X(bEmpire2), EMPIRE_START_Y(bEmpire2));
								}
							}
						}
						break;
					case 3:
						{
							itertype(mPartecipants3) iter = mPartecipants3.find(dwPlayerID2);
							if (iter != mPartecipants3.end())
							{
								iter->second.killed += 1;
								
								TPacketKingdomWar sPacket;
								sPacket.bHeader = HEADER_GC_KINGDOMSWAR;
								sPacket.bSubHeader = KINGDOMSWAR_SUBHEADER_GC_REFRESH;
								for (int i = 0; i < int(EMPIRE_MAX_NUM - 1); ++i)
									sPacket.iKills[i] = 0;
								sPacket.iLimitKills = 0;
								sPacket.iDeads = iter->second.killed;
								sPacket.iLimitDeads = int(KingdomsWar::DEAD_LIMIT);
								sPacket.dwTimeRemained = 0;
								pChar->GetDesc()->Packet(&sPacket, sizeof(TPacketKingdomWar));
								
								if (iter->second.killed >= int(KingdomsWar::DEAD_LIMIT))
								{
									pChar->SetQuestFlag("kingdomswar_mgr.is_enough", 1);
									pChar->WarpSet(EMPIRE_START_X(bEmpire2), EMPIRE_START_Y(bEmpire2));
								}
							}
						}
						break;
				}
			}
			
			LPSECTREE_MAP pSectreeMap = SECTREE_MANAGER::instance().GetMap(KingdomsWar::MAP_INDEX);
			if (pSectreeMap != NULL)
			{
				FRefreshKills f;
				thecore_memcpy(f.m_iKills, kills, sizeof(f.m_iKills));
				pSectreeMap->for_each(f);
			}
		}
	#endif
#endif
